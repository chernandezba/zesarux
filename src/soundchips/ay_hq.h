/* ZEsarUX AY HQ prototype. GPL-3.0-or-later. */
#ifndef ZESARUX_AY_HQ_H
#define ZESARUX_AY_HQ_H
#include <stdint.h>
#define AY_HQ_RATE 44100
#define AY_HQ_TAPS 192
#define AY_HQ_PHASES 256
struct ay_hq {
    uint8_t regs[16];
    unsigned cpu_hz, clock_hz;
    uint64_t chip_phase, sample_phase, samples;
    unsigned tone_count[3], noise_count, envelope_count, noise;
    unsigned mixer_mask;
    int speech, envelopes;
    int tone[3], envelope_step, attack, hold, alternate, holding;
    double pan[3][2], gain, aux[2];
    double history[2][AY_HQ_TAPS], kernel[AY_HQ_PHASES][AY_HQ_TAPS];
    double dc_x[2], dc_y[2];
    unsigned head;
    void (*emit)(void *, int16_t, int16_t);
    void *context;
};
void ay_hq_init(struct ay_hq *, unsigned cpu_hz, unsigned clock_hz,
                void (*emit)(void *, int16_t, int16_t), void *context);
void ay_hq_write(struct ay_hq *, unsigned reg, unsigned value);
void ay_hq_advance(struct ay_hq *, unsigned cycles);
#endif
