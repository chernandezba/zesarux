/* ZEsarUX AY HQ prototype. GPL-3.0-or-later. */
#ifndef ZESARUX_AUDIO_AY_HQ_H
#define ZESARUX_AUDIO_AY_HQ_H
#include <stdint.h>
extern int audio_ay_hq_requested;
void audio_ay_hq_start(void (*sink)(int16_t, int16_t));
void audio_ay_hq_stop(void);
int audio_ay_hq_active(void);
void audio_ay_hq_sync(void);
void audio_ay_hq_end_frame(void);
void audio_ay_hq_reset(void);
void audio_ay_hq_write(int reg, int value);
void audio_ay_hq_aux(int left, int right);
void audio_ay_hq_timer_tick(void);
int audio_ay_hq_take_early_tick(void);
#endif
