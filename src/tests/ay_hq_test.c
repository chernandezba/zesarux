/* Pruebas funcionales del generador AY HQ. GPL-3.0-or-later. */
#include "ay_hq.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct ay_hq a,b;
static double energy;
static unsigned count;
static FILE *pcm;
static void capture(void *context, int16_t l, int16_t r)
{
    (void)context;
    if (count++>4410) energy+=(double)l*l;
    assert(l==r);
    if (pcm) { fwrite(&l,2,1,pcm); fwrite(&r,2,1,pcm); }
}
static void init(struct ay_hq *s)
{
    ay_hq_init(s,3546800,1773400,capture,NULL);
    ay_hq_write(s,7,0x3e);
    ay_hq_write(s,8,15);
}
static double tone(int period, int volume)
{
    init(&a);
    ay_hq_write(&a,0,period&255);
    ay_hq_write(&a,1,period>>8);
    ay_hq_write(&a,8,volume);
    count=0; energy=0;
    ay_hq_advance(&a,a.cpu_hz);
    assert(a.samples==44100);
    return sqrt(energy/(44100-4411));
}
int main(int argc, char **argv)
{
    init(&a);
    ay_hq_write(&a,0,123);
    ay_hq_advance(&a,16*122);
    assert(a.tone[0]==0);
    ay_hq_advance(&a,16);
    assert(a.tone[0]==1);
    assert(a.tone_count[0]==0);
    ay_hq_write(&a,0,0); ay_hq_write(&a,1,0);
    ay_hq_advance(&a,16);
    assert(a.tone[0]==0);
    /* Los 16 envelopes: ataque, continuacion, alternancia y retencion. */
    for (int shape=0; shape<16; shape++) {
        init(&a);
        ay_hq_write(&a,11,1); ay_hq_write(&a,12,0);
        ay_hq_write(&a,13,shape);
        assert((a.envelope_step^a.attack)==((shape&4)?0:31));
        ay_hq_advance(&a,16*31);
        assert((a.envelope_step^a.attack)==((shape&4)?31:0));
        ay_hq_advance(&a,16);
        int expected;
        if (!(shape&8)) expected=0;
        else if (shape&1) expected=((shape&4)!=0)^((shape&2)!=0) ? 31 : 0;
        else expected=((shape&4)!=0)^((shape&2)!=0) ? 0 : 31;
        assert((a.envelope_step^a.attack)==expected);
        ay_hq_write(&a,13,shape);
        assert(a.envelope_count==0 && a.envelope_step==31 && !a.holding);
    }
    /* Secuencia del LFSR de 17 bits; periodo cero equivale a uno. */
    init(&a); ay_hq_write(&a,6,0);
    unsigned noise=1;
    for (int i=0; i<100; i++) {
        noise=(noise>>1)|(((noise^(noise>>3))&1)<<16);
        ay_hq_advance(&a,32);
        assert(a.noise==noise);
    }
    /* Mismas escrituras en los mismos T-estados, particiones diferentes. */
    init(&a); init(&b);
    ay_hq_write(&a,0,79); ay_hq_write(&b,0,79);
    ay_hq_advance(&a,12345);
    for (int i=0; i<12345; i++) ay_hq_advance(&b,1);
    ay_hq_write(&a,8,3); ay_hq_write(&b,8,3);
    ay_hq_advance(&a,56789);
    for (int i=0; i<56789; i+=1) ay_hq_advance(&b,1);
    assert(a.chip_phase==b.chip_phase && a.sample_phase==b.sample_phase);
    assert(!memcmp(a.history,b.history,sizeof(a.history)));
    assert(!memcmp(a.dc_y,b.dc_y,sizeof(a.dc_y)));
    /* No hay deriva al atravesar frames de 48K y de 128K. */
    const unsigned frames[]={69888,70908,71680};
    for (int m=0; m<3; m++) {
        ay_hq_init(&a,frames[m]*50,m==2 ? 1750000 : 1773400,NULL,NULL);
        for (int f=0; f<500; f++) {
            ay_hq_advance(&a,frames[m]);
            assert(a.samples==(unsigned)(f+1)*882);
        }
    }
    double audible=tone(8,15);
    double low=tone(125,1);
    double alias=tone(2,15);
    printf("13.85 kHz tone RMS: %.2f; volume 1 RMS: %.2f; ultrasonic alias RMS: %.2f\n",audible,low,alias);
    assert(audible>1000 && low>10 && alias<10);
    if (argc==2) {
        pcm=fopen(argv[1],"wb"); assert(pcm);
        /* Escalera de volumen, tonos altos y ruido con envolvente. */
        init(&a); ay_hq_write(&a,0,125);
        for (int v=1; v<16; v++) { ay_hq_write(&a,8,v); ay_hq_advance(&a,a.cpu_hz/10); }
        const int periods[]={64,32,16,8,4,2,1};
        for (unsigned i=0;i<sizeof(periods)/sizeof(*periods);i++) {
            ay_hq_write(&a,0,periods[i]); ay_hq_advance(&a,a.cpu_hz/4);
        }
        ay_hq_write(&a,7,0x37); ay_hq_write(&a,6,7);
        ay_hq_write(&a,8,16); ay_hq_write(&a,11,200); ay_hq_write(&a,13,10);
        ay_hq_advance(&a,a.cpu_hz*2);
        fclose(pcm);
    }
    puts("AY HQ tests passed");
    return 0;
}
