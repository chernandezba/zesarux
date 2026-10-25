/* ZEsarUX AY HQ prototype. GPL-3.0-or-later. */
#include "ay_hq.h"
#include <math.h>
#include <string.h>

/* Curva AY de Ayumi (Peter Sovietov, MIT; licenses/ay_hq_ayumi.txt).
   Evita cuantizar los niveles bajos a 8 bits. */
static const double dac[16] = {
    0, .00999465934234, .0144502937362, .0210574502174,
    .0307011520562, .0455481803616, .0644998855573, .107362478065,
    .126588845655, .20498970016, .292210269322, .372838941024,
    .492530708782, .635324635691, .805584802014, 1
};
#define PI 3.14159265358979323846

void ay_hq_write(struct ay_hq *a, unsigned reg, unsigned value)
{
    if (reg >= 16) return;
    a->regs[reg] = value;
    if (reg == 13) {
        a->attack = (value & 4) ? 31 : 0;
        a->hold = (value & 8) ? (value & 1) : 1;
        a->alternate = (value & 8) ? (value & 2) : a->attack;
        a->holding = 0;
        a->envelope_step = 31;
        a->envelope_count = 0;
    }
}

void ay_hq_init(struct ay_hq *a, unsigned cpu_hz, unsigned clock_hz,
                void (*emit)(void *, int16_t, int16_t), void *context)
{
    memset(a, 0, sizeof(*a));
    a->cpu_hz = cpu_hz;
    a->clock_hz = clock_hz;
    a->noise = 1;
    a->gain = 7000;
    a->speech = a->envelopes = 1;
    a->emit = emit;
    a->context = context;
    for (int ch=0; ch<3; ch++) a->pan[ch][0] = a->pan[ch][1] = 1;
    ay_hq_write(a, 13, 0);
    /* FIR polifasico Blackman, corte a 19 kHz, antes de bajar a 44.1 kHz. */
    double fc = 19000.0 / (clock_hz / 8.0);
    for (int p=0; p<AY_HQ_PHASES; p++) {
        double sum=0;
        for (int k=0; k<AY_HQ_TAPS; k++) {
            double x=k + (double)p/AY_HQ_PHASES - (AY_HQ_TAPS-1)/2.0;
            double sinc=fabs(x)<1e-12 ? 2*fc : sin(2*PI*fc*x)/(PI*x);
            double w=.42-.5*cos(2*PI*k/(AY_HQ_TAPS-1))+.08*cos(4*PI*k/(AY_HQ_TAPS-1));
            sum += a->kernel[p][k] = sinc*w;
        }
        for (int k=0; k<AY_HQ_TAPS; k++) a->kernel[p][k] /= sum;
    }
}

static unsigned period(const struct ay_hq *a, unsigned reg, unsigned mask)
{
    unsigned n=a->regs[reg] | ((a->regs[reg+1]&mask)<<8);
    return n ? n : 1;
}

static void chip_tick(struct ay_hq *a)
{
    unsigned np=a->regs[6]&31;
    if (++a->noise_count >= 2*(np ? np : 1)) {
        a->noise_count=0;
        a->noise=(a->noise>>1) | (((a->noise^(a->noise>>3))&1)<<16);
    }
    if (!a->holding && ++a->envelope_count >= period(a,11,255)) {
        a->envelope_count=0;
        if (--a->envelope_step < 0) {
            if (a->alternate) a->attack ^= 31;
            if (a->hold) {
                a->holding=1;
                a->envelope_step=0;
            } else a->envelope_step=31;
        }
    }
    double output[2]={a->aux[0],a->aux[1]};
    for (int ch=0; ch<3; ch++) {
        if (++a->tone_count[ch] >= period(a,ch*2,15)) {
            a->tone_count[ch]=0;
            a->tone[ch] ^= 1;
        }
        unsigned mixer=a->regs[7] | a->mixer_mask;
        int gate=(a->tone[ch] | ((mixer>>ch)&1)) &
                 ((a->noise&1) | ((mixer>>(ch+3))&1));
        if (!a->speech && (mixer & (9u<<ch))==(9u<<ch)) gate=0;
        unsigned v=a->regs[8+ch];
        unsigned level=(v&16) ? (a->envelopes ? ((a->envelope_step^a->attack)>>1) : 15) : (v&15);
        double sample=gate ? dac[level] : 0;
        for (int side=0; side<2; side++) output[side] += sample*a->pan[ch][side]*a->gain;
    }
    a->head=(a->head+AY_HQ_TAPS-1)%AY_HQ_TAPS;
    for (int side=0; side<2; side++) a->history[side][a->head]=output[side];
}

static void output_sample(struct ay_hq *a)
{
    unsigned phase=(a->chip_phase*AY_HQ_PHASES)/((uint64_t)a->cpu_hz*8);
    int16_t out[2];
    for (int side=0; side<2; side++) {
        double x=0;
        unsigned h=a->head;
        for (int k=0; k<AY_HQ_TAPS; k++) {
            x += a->history[side][h]*a->kernel[phase][k];
            if (++h==AY_HQ_TAPS) h=0;
        }
        /* Eliminar continua (20 Hz) despues del FIR. */
        double y=x-a->dc_x[side]+.997154544*a->dc_y[side];
        a->dc_x[side]=x;
        a->dc_y[side]=y;
        if (y>32767) y=32767;
        if (y< -32768) y= -32768;
        out[side]=(int16_t)lrint(y);
    }
    a->samples++;
    if (a->emit) a->emit(a->context,out[0],out[1]);
}

void ay_hq_advance(struct ay_hq *a, unsigned cycles)
{
    /* Acumuladores racionales: sin redondear periodos ni acumular deriva. */
    while (cycles--) {
        a->chip_phase += a->clock_hz;
        if (a->chip_phase >= (uint64_t)a->cpu_hz*8) {
            a->chip_phase -= (uint64_t)a->cpu_hz*8;
            chip_tick(a);
        }
        a->sample_phase += AY_HQ_RATE;
        if (a->sample_phase >= a->cpu_hz) {
            a->sample_phase -= a->cpu_hz;
            output_sample(a);
        }
    }
}
