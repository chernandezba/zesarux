/* Sonda exclusiva de pruebas; no se enlaza con el ejecutable normal. GPL-3.0-or-later. */
#include "cpu.h"
#include "mem128.h"
#include "screen.h"
#include "audio_ay_hq.h"
#include "core_spectrum.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern void core_spectrum_fin_frame_pantalla(void);
static FILE *trace;
static unsigned frames,limit;
static unsigned long long samples;
static int peak,ready;
static void sink(int16_t l,int16_t r)
{
    samples++;
    if (abs((int)l)>peak) peak=abs((int)l);
    if (abs((int)r)>peak) peak=abs((int)r);
}
static uint64_t hash(const void *data,size_t n)
{
    const unsigned char *p=data;
    uint64_t h=UINT64_C(14695981039346656037);
    while (n--) { h^=*p++; h*=UINT64_C(1099511628211); }
    return h;
}
__attribute__((constructor)) static void init(void)
{
    const char *path=getenv("AY_DEMO_TRACE");
    if (!path) return;
    trace=fopen(path,"w");
    if (!trace) abort();
    limit=(unsigned)strtoul(getenv("AY_DEMO_FRAMES") ? getenv("AY_DEMO_FRAMES") : "16000",0,10);
    if (getenv("AY_DEMO_HQ")) audio_ay_hq_start(sink);
    ready=1;
}
void __cyg_profile_func_enter(void *function,void *caller)
{
    (void)function; (void)caller;
}
void __cyg_profile_func_exit(void *function,void *caller)
{
    (void)caller;
    if (!ready || function!=(void *)core_spectrum_fin_frame_pantalla) return;
    unsigned regs[]={reg_pc,reg_sp,reg_a,Z80_FLAGS,reg_bc,reg_de,reg_hl,reg_ix,reg_iy,
        reg_i,reg_r,reg_r_bit7,reg_a_shadow,Z80_FLAGS_SHADOW,reg_b_shadow,reg_c_shadow,
        reg_d_shadow,reg_e_shadow,reg_h_shadow,reg_l_shadow,(unsigned)t_estados};
    fprintf(trace,"%u %016llx",++frames,(unsigned long long)hash(regs,sizeof(regs)));
    for (int bank=0;bank<8;bank++) fprintf(trace," %016llx",(unsigned long long)hash(ram_mem_table[bank],16384));
    fprintf(trace," %016llx",(unsigned long long)hash(rainbow_buffer,
        (size_t)rainbow_buffer_current_width*rainbow_buffer_current_height*sizeof(*rainbow_buffer)));
    /* Variables del kernel #8000: el mapa exacto se toma del .sym en el analizador. */
    fprintf(trace," ");
    for (int i=0;i<0x1400;i++) fprintf(trace,"%02x",ram_mem_table[2][i]);
    fprintf(trace," PC=%04x SP=%04x I=%02x\n",reg_pc,reg_sp,reg_i);
    /* Solo quitar la espera de pared: no tocar T-estados ni interrupciones Z80. */
    esperando_tiempo_final_t_estados.v=0;
    interrupcion_timer_generada.v=1;
    if (frames>=limit) {
        fprintf(stderr,"AY_DEMO_TRACE: %u frames; HQ active=%d, %llu samples, peak=%d\n",frames,audio_ay_hq_active(),samples,peak);
        fclose(trace);
        _Exit(0);
    }
}
