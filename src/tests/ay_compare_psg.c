/* Comparacion reproducible: el AY original y AY HQ reciben el mismo PSG. GPL-3.0-or-later. */
#include "cpu.h"
#include "ay38912.h"
#include "ay_hq.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

z80_byte current_machine_type=MACHINE_ID_SPECTRUM_128;
int cpu_turbo_speed=1;
unsigned int debug_t_estados_parcial;
void debug_printf(int level,const char *format,...) { (void)level; (void)format; }
void reset_silence_detection_counter(void) {}
void audio_ay_hq_reset(void) {}
void audio_ay_hq_write(int reg,int value) { (void)reg; (void)value; }
void audio_midi_output_raw(z80_byte value) { (void)value; }
void cpu_panic(char *message) { fprintf(stderr,"%s\n",message); exit(1); }

static struct ay_hq synth;
static FILE *old_pcm,*new_pcm;
static unsigned frames;
static void emit(void *context,int16_t l,int16_t r)
{
    (void)context;
    fwrite(&l,2,1,new_pcm); fwrite(&r,2,1,new_pcm);
}
static void frame(void)
{
    for (int s=0;s<312;s++) {
        int16_t l=(int8_t)da_output_ay_izquierdo()*256;
        int16_t r=(int8_t)da_output_ay_derecho()*256;
        fwrite(&l,2,1,old_pcm); fwrite(&r,2,1,old_pcm);
        ay_chip_siguiente_ciclo();
    }
    ay_hq_advance(&synth,70908);
    frames++;
}
int main(int argc,char **argv)
{
    if (argc!=4) { fprintf(stderr,"usage: ay_compare_psg input.psg old.pcm new.pcm\n"); return 1; }
    FILE *f=fopen(argv[1],"rb");
    old_pcm=fopen(argv[2],"wb"); new_pcm=fopen(argv[3],"wb");
    if (!f || !old_pcm || !new_pcm) return 1;
    char header[16];
    if (fread(header,1,16,f)!=16 || header[0]!='P' || header[1]!='S' || header[2]!='G') return 1;
    init_chip_ay(); ay_init_filters();
    ay_chip_present.v=ay_speech_enabled.v=ay_envelopes_enabled.v=1;
    ay3_stereo_mode=1;
    ay_hq_init(&synth,70908*50,1773400,emit,NULL);
    synth.pan[0][1]=0; synth.pan[1][0]=0;
    for (int reg=0;reg<16;reg++) {
        out_port_ay(65533,reg); out_port_ay(49149,0);
        ay_hq_write(&synth,reg,0);
    }
    int started=0,cmd;
    while ((cmd=fgetc(f))!=EOF && cmd!=0xfd) {
        if (cmd==0xff) { if (started) frame(); started=1; }
        else if (cmd==0xfe) {
            int n=fgetc(f); if (n<0) return 1;
            for (int i=0;i<n*4;i++) frame();
            started=0;
        } else if (cmd<16) {
            int value=fgetc(f); if (value<0) return 1;
            out_port_ay(65533,cmd); out_port_ay(49149,value);
            ay_hq_write(&synth,cmd,value);
        } else { fprintf(stderr,"invalid PSG command %d\n",cmd); return 1; }
    }
    if (started) frame();
    fclose(f); fclose(old_pcm); fclose(new_pcm);
    printf("%u frames, %.2f seconds, %llu HQ samples\n",frames,frames/50.0,(unsigned long long)synth.samples);
    return 0;
}
