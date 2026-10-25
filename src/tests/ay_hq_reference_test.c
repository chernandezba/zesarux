/* Comparación con Ayumi sin modificar; sólo se enlaza en las pruebas. */
#include "ay_hq.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "reference/ayumi/ayumi.c"

static struct ay_hq hq;
static struct ayumi ref;
static unsigned char regs[16];
static unsigned phase;
static uint64_t checked_ticks;
static void write_reg(unsigned reg,unsigned value)
{
    regs[reg]=(unsigned char)value;
    ay_hq_write(&hq,reg,value);
    if (reg<6) {
        int ch=reg/2;ayumi_set_tone(&ref,ch,regs[ch*2]|(regs[ch*2+1]<<8));
    }
    if (reg==6) ayumi_set_noise(&ref,value);
    if (reg>=7 && reg<=10) {
        for (int ch=0;ch<3;ch++) {
            ayumi_set_mixer(&ref,ch,(regs[7]>>ch)&1,(regs[7]>>(ch+3))&1,regs[8+ch]&16);
            ayumi_set_volume(&ref,ch,regs[8+ch]);
        }
    }
    if (reg==11 || reg==12) ayumi_set_envelope(&ref,regs[11]|(regs[12]<<8));
    if (reg==13) ayumi_set_envelope_shape(&ref,value);
}
static void init(void)
{
    ay_hq_init(&hq,3546800,1773400,NULL,NULL);hq.gain=1;
    assert(ayumi_configure(&ref,0,1773400,44100));
    memset(regs,0,sizeof(regs));phase=0;
    for (int ch=0;ch<3;ch++) ref.channels[ch].pan_left=ref.channels[ch].pan_right=1;
    for (unsigned reg=0;reg<16;reg++) write_reg(reg,0);
}
static void advance(unsigned cycles)
{
    /* Escrituras entre ticks: ambos ven la misma fase del reloj maestro. */
    while (cycles--) {
        ay_hq_advance(&hq,1);
        if (++phase==16) {
            phase=0;update_mixer(&ref);checked_ticks++;
            assert(hq.noise==(unsigned)ref.noise);
            for (int ch=0;ch<3;ch++) {
                assert(hq.tone[ch]==ref.channels[ch].tone);
                assert(hq.tone_count[ch]==(unsigned)ref.channels[ch].tone_counter);
            }
            assert(fabs(hq.history[0][hq.head]-ref.left)<1e-12);
            assert(fabs(hq.history[1][hq.head]-ref.right)<1e-12);
        }
    }
}
static unsigned rng=0x718b4c31;
static unsigned random_value(void)
{ rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng; }
static double pcm[44100];
static unsigned pcm_count;
static void capture(void *context,int16_t left,int16_t right)
{ (void)context;assert(left==right);assert(pcm_count<44100);pcm[pcm_count++]=left; }
static void waveform(int period,int volume)
{
    ay_hq_init(&hq,3546800,1773400,capture,NULL);pcm_count=0;
    ay_hq_write(&hq,0,period&255);ay_hq_write(&hq,1,period>>8);
    ay_hq_write(&hq,7,0x3e);ay_hq_write(&hq,8,volume);
    ay_hq_advance(&hq,hq.cpu_hz/2);assert(pcm_count==22050);
    assert(ayumi_configure(&ref,0,1773400,44100));
    ayumi_set_tone(&ref,0,period);ayumi_set_volume(&ref,0,volume);
    for (int ch=0;ch<3;ch++) ayumi_set_mixer(&ref,ch,ch!=0,1,0);
    ayumi_set_pan(&ref,0,0,0);
    double reference[22050];
    for (int i=0;i<22050;i++) { ayumi_process(&ref);ayumi_remove_dc(&ref);reference[i]=ref.left; }
    double best=-1;
    for (int lag=-100;lag<=100;lag++) {
        double x=0,y=0,xx=0,yy=0,xy=0;int n=0;
        for (int i=4410;i<21000;i++) {
            double a=pcm[i],b=reference[i+lag];
            x+=a;y+=b;xx+=a*a;yy+=b*b;xy+=a*b;n++;
        }
        double correlation=(xy-x*y/n)/sqrt((xx-x*x/n)*(yy-y*y/n));
        if (correlation>best) best=correlation;
    }
    printf("Ayumi PCM: period=%d volume=%d best integer-lag correlation=%.6f\n",period,volume,best);
    assert(best>0.98);
}
int main(void)
{
    for (unsigned shape=0;shape<16;shape++) {
        for (unsigned period=0;period<4;period++) {
            init();write_reg(7,0x3f);write_reg(8,16);write_reg(11,period);write_reg(13,shape);
            advance(16*256*4);
            advance(7);write_reg(13,shape);advance(16*256);
        }
    }
    init();
    for (unsigned i=0;i<20000;i++) {
        unsigned reg=random_value()%14,value=random_value()&255;
        write_reg(reg,value);advance(1+random_value()%127);
    }
    /* Todos los estados del mezclador y niveles fijos, incluidos cero y uno. */
    for (unsigned mixer=0;mixer<64;mixer++) {
        init();write_reg(7,mixer);write_reg(0,7);write_reg(2,13);write_reg(4,23);write_reg(6,3);
        for (unsigned volume=0;volume<16;volume++) {
            for (int ch=0;ch<3;ch++) write_reg(8+ch,volume);
            advance(16*128);
        }
    }
    waveform(125,15);waveform(125,1);waveform(1000,15);
    printf("Ayumi reference passed: %llu exact pre-filter chip ticks\n",(unsigned long long)checked_ticks);
}
