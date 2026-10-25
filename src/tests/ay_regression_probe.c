/* Sonda de pruebas compartida por upstream y la rama AY; no va al ejecutable normal. */
#include "cpu.h"
#include "operaciones.h"
#include "mem128.h"
#include "screen.h"
#include "audio.h"
#include "ay38912.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef AUDIT_WITH_HQ
#include "audio_ay_hq.h"
#endif
extern void core_spectrum_fin_frame_pantalla(void);
static FILE *trace;
static unsigned frames,limit;
static uint64_t ay_hash=UINT64_C(14695981039346656037),pcm_hash=UINT64_C(14695981039346656037);
static uint64_t ay_count,pcm_count;
static uint64_t hash(uint64_t h,const void *data,size_t n)
{
    const unsigned char *p=data;
    while (n--) { h^=*p++; h*=UINT64_C(1099511628211); }
    return h;
}
void ay_audit_write(unsigned port,unsigned value)
{
    unsigned event[]={frames,(unsigned)t_estados,reg_pc,port,value};
    ay_hash=hash(ay_hash,event,sizeof(event)); ay_count++;
}
void ay_audit_pcm(char left,char right)
{
    unsigned char sample[]={(unsigned char)left,(unsigned char)right};
    pcm_hash=hash(pcm_hash,sample,sizeof(sample)); pcm_count++;
}
__attribute__((constructor)) static void init(void)
{
    const char *path=getenv("AY_AUDIT_TRACE");
    if (!path) return;
    trace=fopen(path,"w"); assert(trace);
    limit=(unsigned)strtoul(getenv("AY_AUDIT_FRAMES"),NULL,10); assert(limit);
}
void __cyg_profile_func_enter(void *function,void *caller)
{ (void)function; (void)caller; }
void __cyg_profile_func_exit(void *function,void *caller)
{
    (void)caller;
    if (!trace || function!=(void *)core_spectrum_fin_frame_pantalla) return;
#ifdef AUDIT_WITH_HQ
    assert(!audio_ay_hq_requested && !audio_ay_hq_active());
#endif
    unsigned regs[]={reg_pc,reg_sp,reg_a,Z80_FLAGS,reg_bc,reg_de,reg_hl,reg_ix,reg_iy,
        reg_i,reg_r,reg_r_bit7,reg_a_shadow,Z80_FLAGS_SHADOW,reg_b_shadow,reg_c_shadow,
        reg_d_shadow,reg_e_shadow,reg_h_shadow,reg_l_shadow,iff1.v,iff2.v,im_mode,
        (unsigned)t_estados,puerto_32765,puerto_8189};
    uint64_t memory=UINT64_C(14695981039346656037);
    if (MACHINE_IS_SPECTRUM_16_48) {
        for (unsigned address=16384;address<65536;address++) {
            unsigned char value=peek_byte_no_time(address);
            memory=hash(memory,&value,1);
        }
    } else {
        for (int bank=0;bank<8;bank++) memory=hash(memory,ram_mem_table[bank],16384);
    }
    fprintf(trace,"%u cpu=%016llx ram=%016llx video=%016llx ay=%016llx writes=%llu pcm=%016llx samples=%llu regs=%016llx\n",
        ++frames,(unsigned long long)hash(UINT64_C(14695981039346656037),regs,sizeof(regs)),
        (unsigned long long)memory,(unsigned long long)hash(UINT64_C(14695981039346656037),rainbow_buffer,
        (size_t)rainbow_buffer_current_width*rainbow_buffer_current_height*sizeof(*rainbow_buffer)),
        (unsigned long long)ay_hash,(unsigned long long)ay_count,(unsigned long long)pcm_hash,(unsigned long long)pcm_count,
        (unsigned long long)hash(UINT64_C(14695981039346656037),ay_3_8912_registros,MAX_AY_CHIPS*16));
    if (!getenv("AY_AUDIT_REALTIME")) {
        /* Acelerar solamente la espera de pared, después de capturar el estado. */
        esperando_tiempo_final_t_estados.v=0; interrupcion_timer_generada.v=1;
    }
    if (frames==limit) { fclose(trace); _Exit(0); }
}
