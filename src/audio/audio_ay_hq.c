/* ZEsarUX AY HQ prototype. GPL-3.0-or-later. */
#include "audio_ay_hq.h"
#include "ay_hq.h"
#include "ay38912.h"
#include "audio.h"
#include "cpu.h"
#include "screen.h"
#include "core_spectrum.h"
#include "debug.h"
#include "tape.h"
#include "gs.h"
#include "hilow_datadrive.h"
#include "if1.h"
#include "zeng_online_client.h"

int audio_ay_hq_requested=0;
static struct ay_hq synth;
static void (*output)(int16_t, int16_t);
static int initialized, last_tstate, last_machine=-1;
static int early_tick;
static unsigned early_ticks_used;

int audio_ay_hq_active(void)
{
    return output && MACHINE_IS_SPECTRUM_16_48_128_P2_P2A_P3 &&
           cpu_turbo_speed==1 && !machine_60hz.v &&
           !core_spectrum_uses_reduced.v && total_ay_chips==1 && !audio_nagra_effect.v &&
           !audio_resample_1bit.v &&
           !audiodac_enabled.v && !gs_enabled.v && !hilow_enabled.v && !if1_enabled.v &&
           !realtape_playing.v && !audio_is_recording_input && !audio_tone_generator &&
           !audio_menu_tone_generator_active.v && !zeng_online_connected.v;
}

static void emit(void *context GCC_UNUSED, int16_t left, int16_t right)
{
    output(left,right);
}

void audio_ay_hq_start(void (*sink)(int16_t, int16_t))
{
    output=sink;
    initialized=0;
    early_tick=0; early_ticks_used=0;
}

void audio_ay_hq_stop(void)
{
    if (output && MACHINE_IS_PENTAGON) debug_printf(VERBOSE_INFO,"AY HQ Pentagon: %u early frame deadlines retained",early_ticks_used);
    output=0;
    initialized=0;
    early_tick=0;
}

void audio_ay_hq_reset(void)
{
    initialized=0;
    early_tick=0;
}

static void configure(void)
{
    /* Pentagon conserva sus 71680 T por frame y el ritmo de 50 Hz del emulador.
       Su AY nominal usa 1.75 MHz; no cambiar el reloj del generador antiguo. */
    unsigned clock=MACHINE_IS_PENTAGON ? 1750000 : ay_chip_frequency;
    ay_hq_init(&synth,screen_testados_total*50,clock,emit,0);
    for (int reg=0; reg<16; reg++) ay_hq_write(&synth,reg,ay_3_8912_registros[0][reg]);
    last_tstate=t_estados;
    last_machine=current_machine_type;
    initialized=1;
    debug_printf(VERBOSE_INFO,"AY HQ: 44100 Hz, signed 16-bit, CPU %u Hz, AY %u Hz",synth.cpu_hz,synth.clock_hz);
}

void audio_ay_hq_sync(void)
{
    if (!audio_ay_hq_active()) { initialized=0; return; }
    if (!initialized || last_machine!=current_machine_type) configure();
    /* Las escrituras directas de snapshots se reflejan sin reiniciar R13 cada linea. */
    for (int reg=0; reg<16; reg++)
        if (synth.regs[reg]!=ay_3_8912_registros[0][reg]) ay_hq_write(&synth,reg,ay_3_8912_registros[0][reg]);
    int pan[3]={1,1,1};
    switch (ay3_stereo_mode) {
        case 1: pan[0]=0; pan[1]=2; break;
        case 2: pan[0]=0; pan[2]=2; break;
        case 3: pan[1]=0; pan[2]=2; break;
        case 4: pan[0]=2; pan[2]=0; break;
        case 5: pan[0]=ay3_custom_stereo_A; pan[1]=ay3_custom_stereo_B; pan[2]=ay3_custom_stereo_C; break;
    }
    for (int ch=0; ch<3; ch++) {
        synth.pan[ch][0]=ay_chip_present.v && pan[ch]!=2;
        synth.pan[ch][1]=ay_chip_present.v && pan[ch]!=0;
    }
    synth.gain=7000.0*audiovolume/100;
    synth.mixer_mask=ay_filtros[0];
    synth.speech=ay_speech_enabled.v;
    synth.envelopes=ay_envelopes_enabled.v;
    if (t_estados>=last_tstate) ay_hq_advance(&synth,t_estados-last_tstate);
    last_tstate=t_estados;
}

void audio_ay_hq_end_frame(void)
{
    audio_ay_hq_sync();
    if (initialized) last_tstate-=screen_testados_total;
}

void audio_ay_hq_write(int reg, int value)
{
    audio_ay_hq_sync();
    if (initialized) ay_hq_write(&synth,reg,value);
}

void audio_ay_hq_aux(int left, int right)
{
    if (initialized) { synth.aux[0]=left*256.0; synth.aux[1]=right*256.0; }
}

/* Conservar un tick que llega durante el calculo del frame. El core limpia
   interrupcion_timer_generada al terminar cada opcode; sin este credito,
   el final del frame puede esperar otro tick y perder 20 ms de audio. */
void audio_ay_hq_timer_tick(void)
{
    if (audio_ay_hq_active() && MACHINE_IS_PENTAGON) {
        if (!esperando_tiempo_final_t_estados.v) early_tick=1;
    } else early_tick=0;
}

int audio_ay_hq_take_early_tick(void)
{
    int ready=early_tick && audio_ay_hq_active() && MACHINE_IS_PENTAGON;
    early_tick=0;
    if (ready) early_ticks_used++;
    return ready;
}
