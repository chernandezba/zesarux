/* Prueba del reloj y las escrituras entre el core Spectrum y AY HQ. GPL-3.0-or-later. */
#include "cpu.h"
#include "audio.h"
#include "ay38912.h"
#include "audio_ay_hq.h"
#include "ay_hq.h"
#include "screen.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

z80_byte current_machine_type=MACHINE_ID_SPECTRUM_48;
int t_estados, screen_testados_total=69888, cpu_turbo_speed=1;
z80_bit machine_60hz, core_spectrum_uses_reduced, esperando_tiempo_final_t_estados;
z80_bit ay_chip_present={1}, ay_speech_enabled={1}, ay_envelopes_enabled={1};
int ay_chip_frequency=1773400, total_ay_chips=1, audiovolume=100;
int ay3_stereo_mode, ay3_custom_stereo_A, ay3_custom_stereo_B, ay3_custom_stereo_C;
z80_byte ay_3_8912_registros[MAX_AY_CHIPS][16], ay_filtros[MAX_AY_CHIPS];
z80_bit audio_resample_1bit, audio_nagra_effect, audiodac_enabled, gs_enabled, hilow_enabled, if1_enabled;
z80_bit realtape_playing, audio_menu_tone_generator_active, zeng_online_connected;
int audio_is_recording_input, audio_tone_generator;
void debug_printf(int level, const char *format, ...) { (void)level; (void)format; }
static struct ay_hq reference;
static uint64_t actual_hash, expected_hash, actual_count, expected_count;
static double left_energy,right_energy;
static int16_t captured[4096][2];
static void output(int16_t l,int16_t r)
{
    actual_hash=actual_hash*131+(uint16_t)l;
    actual_hash=actual_hash*131+(uint16_t)r;
    if (actual_count<4096) { captured[actual_count][0]=l; captured[actual_count][1]=r; }
    left_energy+=(double)l*l; right_energy+=(double)r*r;
    actual_count++;
}
static void expected(void *context,int16_t l,int16_t r)
{
    (void)context;
    expected_hash=expected_hash*131+(uint16_t)l;
    expected_hash=expected_hash*131+(uint16_t)r;
    expected_count++;
}
static void check_machine(z80_byte machine,int frame_t,unsigned clock)
{
    current_machine_type=machine; screen_testados_total=frame_t; t_estados=0;
    actual_hash=expected_hash=actual_count=expected_count=0;
    memset(ay_3_8912_registros,0,sizeof(ay_3_8912_registros));
    uint8_t *regs=ay_3_8912_registros[0];
    regs[0]=125; regs[7]=0x3e; regs[8]=16; regs[11]=100; regs[13]=10;
    ay_hq_init(&reference,screen_testados_total*50,clock,expected,NULL);
    for (int i=0; i<16; i++) ay_hq_write(&reference,i,regs[i]);
    audio_ay_hq_start(output);
    assert(audio_ay_hq_active());
    audio_ay_hq_sync();
    t_estados=123;
    audio_ay_hq_write(13,10);
    ay_hq_advance(&reference,123); ay_hq_write(&reference,13,10);
    t_estados=228;
    audio_ay_hq_sync(); ay_hq_advance(&reference,105);
    t_estados=frame_t+4;
    audio_ay_hq_end_frame(); ay_hq_advance(&reference,frame_t+4-228);
    t_estados-=screen_testados_total;
    t_estados=20;
    audio_ay_hq_write(8,7); regs[8]=7;
    ay_hq_advance(&reference,16); ay_hq_write(&reference,8,7);
    t_estados=screen_testados_total;
    audio_ay_hq_end_frame(); ay_hq_advance(&reference,screen_testados_total-20);
    assert(actual_count==1764 && actual_count==expected_count);
    assert(actual_hash==expected_hash);
    cpu_turbo_speed=2; assert(!audio_ay_hq_active()); cpu_turbo_speed=1;
    total_ay_chips=2; assert(!audio_ay_hq_active()); total_ay_chips=1;
    core_spectrum_uses_reduced.v=1; assert(!audio_ay_hq_active()); core_spectrum_uses_reduced.v=0;
    esperando_tiempo_final_t_estados.v=0;
    audio_ay_hq_timer_tick();
    audio_ay_hq_timer_tick();
    assert(audio_ay_hq_take_early_tick()==(machine==MACHINE_ID_PENTAGON));
    assert(!audio_ay_hq_take_early_tick());
    esperando_tiempo_final_t_estados.v=1;
    audio_ay_hq_timer_tick();
    assert(!audio_ay_hq_take_early_tick());
    esperando_tiempo_final_t_estados.v=0;
    audio_ay_hq_stop(); assert(!audio_ay_hq_active());
}
/* Controles del adaptador: comprobar la salida observable, no sólo elegibilidad. */
static void prepare(int channel,int mode,int volume)
{
    current_machine_type=MACHINE_ID_SPECTRUM_128;screen_testados_total=70908;t_estados=0;
    memset(ay_3_8912_registros,0,sizeof(ay_3_8912_registros));
    memset(ay_filtros,0,sizeof(ay_filtros));
    ay_3_8912_registros[0][channel*2]=125;
    ay_3_8912_registros[0][7]=0x3f ^ (1<<channel);
    ay_3_8912_registros[0][8+channel]=15;
    ay3_stereo_mode=mode;audiovolume=volume;
    ay_chip_present.v=ay_speech_enabled.v=ay_envelopes_enabled.v=1;
    actual_count=actual_hash=0;left_energy=right_energy=0;
    audio_ay_hq_start(output);audio_ay_hq_sync();
}
static void two_frames(void)
{
    for (int i=0;i<2;i++) {
        t_estados=screen_testados_total;audio_ay_hq_end_frame();t_estados=0;
    }
    assert(actual_count==1764);
}
static void controls(void)
{
    const int sides[5][3]={{1,1,1},{0,2,1},{0,1,2},{1,0,2},{2,1,0}};
    for (int mode=0;mode<6;mode++) for (int ch=0;ch<3;ch++) {
        ay3_custom_stereo_A=2;ay3_custom_stereo_B=0;ay3_custom_stereo_C=1;
        int side=mode==5 ? (int[]){2,0,1}[ch] : sides[mode][ch];
        prepare(ch,mode,100);two_frames();
        assert((left_energy>0)==(side!=2));
        assert((right_energy>0)==(side!=0));
        if (side==1) assert(left_energy==right_energy);
    }
    prepare(0,0,100);two_frames();
    int16_t full[1764][2];memcpy(full,captured,sizeof(full));
    prepare(0,0,50);two_frames();
    for (int i=0;i<1764;i++) assert(abs(2*(int)captured[i][0]-full[i][0])<=1);
    prepare(0,0,0);two_frames();assert(left_energy==0 && right_energy==0);
    prepare(0,0,100);ay_chip_present.v=0;two_frames();assert(left_energy==0 && right_energy==0);
    prepare(0,0,100);ay_filtros[0]=0x3f;audio_ay_hq_reset();audio_ay_hq_sync();
    /* El filtro de canales sigue el mezclador existente: desactiva las puertas. */
    two_frames();assert(left_energy==right_energy);
    prepare(0,0,100);ay_3_8912_registros[0][8]=0;audio_ay_hq_reset();audio_ay_hq_sync();
    audio_ay_hq_aux(10,-10);two_frames();assert(left_energy>0 && left_energy==right_energy);
    for (int i=0;i<1764;i++) assert(captured[i][0]==-captured[i][1]);
    /* Reiniciar y volver a cargar los registros debe reproducir el mismo inicio. */
    prepare(0,0,100);two_frames();uint64_t fresh=actual_hash;
    audio_ay_hq_reset();t_estados=0;actual_count=actual_hash=0;
    audio_ay_hq_sync();two_frames();assert(actual_hash==fresh);
    /* Sin avance del tiempo (pausa), no se generan muestras adicionales. */
    uint64_t paused=actual_count;
    for (int i=0;i<100;i++) audio_ay_hq_sync();
    assert(actual_count==paused);
    /* Restauración de snapshot después de desactivar temporalmente HQ. */
    cpu_turbo_speed=2;audio_ay_hq_sync();assert(!audio_ay_hq_active());
    t_estados=700;audio_ay_hq_sync();assert(actual_count==paused);cpu_turbo_speed=1;
    t_estados=0;audio_ay_hq_reset();actual_count=actual_hash=0;audio_ay_hq_sync();
    two_frames();assert(actual_hash==fresh);
    audio_ay_hq_stop();
}
static void fallback_matrix(void)
{
    prepare(0,0,100);
    z80_bit *flags[]={&machine_60hz,&core_spectrum_uses_reduced,&audio_nagra_effect,
        &audio_resample_1bit,&audiodac_enabled,&gs_enabled,&hilow_enabled,&if1_enabled,
        &realtape_playing,&audio_menu_tone_generator_active,&zeng_online_connected};
    for (unsigned i=0;i<sizeof(flags)/sizeof(*flags);i++) {
        flags[i]->v=1;assert(!audio_ay_hq_active());audio_ay_hq_sync();
        flags[i]->v=0;assert(audio_ay_hq_active());audio_ay_hq_sync();
    }
    audio_is_recording_input=1;assert(!audio_ay_hq_active());audio_is_recording_input=0;
    audio_tone_generator=1;assert(!audio_ay_hq_active());audio_tone_generator=0;
    current_machine_type=MACHINE_ID_TBBLUE;assert(!audio_ay_hq_active());
    current_machine_type=MACHINE_ID_SPECTRUM_128;assert(audio_ay_hq_active());
    audio_ay_hq_stop();assert(!audio_ay_hq_active());
}
int main(void)
{
    check_machine(MACHINE_ID_SPECTRUM_48,69888,1773400);
    check_machine(MACHINE_ID_SPECTRUM_128,70908,1773400);
    check_machine(MACHINE_ID_PENTAGON,71680,1750000);
    check_machine(MACHINE_ID_SPECTRUM_P2,70908,1773400);
    check_machine(MACHINE_ID_SPECTRUM_P2A_40,70908,1773400);
    check_machine(MACHINE_ID_SPECTRUM_P3_40,70908,1773400);
    controls();fallback_matrix();
    puts("AY HQ Spectrum/Pentagon integration tests passed (six models, timestamps, R13, frame overshoot, stereo, volume, aux, reset, pause, fallback)");
    return 0;
}
