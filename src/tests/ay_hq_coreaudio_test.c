/* Prueba de la cola y el callback reales, sin abrir dispositivo ni ventana. */
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include "../audio/audiocoreaudio.c"

z80_bit audio_playing;
int ending_emulator_flag;
void debug_printf(int level,const char *format,...) { (void)level;(void)format; }
static void reset_fifo(void)
{
    hq_write=hq_read=0;hq_overruns=hq_underruns=0;
    hq_frames=hq_device_frames=hq_missing_frames=hq_discarded_frames=0;
    hq_mode=1;audio_playing.v=1;ending_emulator_flag=0;
    deviceFormat.mBytesPerFrame=4;
}
static int16_t output_buffer[HQ_FIFO_FRAMES*2+8];
static void callback(unsigned frames)
{
    AudioBufferList buffers={0};buffers.mNumberBuffers=1;
    buffers.mBuffers[0].mNumberChannels=2;
    buffers.mBuffers[0].mData=output_buffer;
    buffers.mBuffers[0].mDataByteSize=frames*4;
    assert(coreaudiowrite(NULL,NULL,NULL,0,frames,&buffers)==noErr);
}
static void *producer(void *context)
{
    (void)context;
    for (unsigned i=0;i<200000;i++) {
        while (__atomic_load_n(&hq_write,__ATOMIC_RELAXED)-__atomic_load_n(&hq_read,__ATOMIC_ACQUIRE)==HQ_FIFO_FRAMES) sched_yield();
        hq_push((int16_t)i,(int16_t)~i);
    }
    return NULL;
}
static void *consumer(void *context)
{
    (void)context;
    for (unsigned i=0;i<200000;i++) {
        while (__atomic_load_n(&hq_write,__ATOMIC_ACQUIRE)==__atomic_load_n(&hq_read,__ATOMIC_RELAXED)) sched_yield();
        int16_t pair[2];hq_read_samples(pair,1);
        assert(pair[0]==(int16_t)i && pair[1]==(int16_t)~i);
    }
    return NULL;
}
int main(void)
{
    reset_fifo();
    for (unsigned i=0;i<HQ_FIFO_FRAMES;i++) hq_push((int16_t)i,-(int16_t)i);
    hq_push(123,456);assert(hq_overruns==1 && hq_read==0 && hq_write==HQ_FIFO_FRAMES);
    callback(HQ_FIFO_FRAMES);
    for (unsigned i=0;i<HQ_FIFO_FRAMES;i++) assert(output_buffer[i*2]==(int16_t)i && output_buffer[i*2+1]==-(int16_t)i);
    callback(4);for (int i=0;i<8;i++) assert(output_buffer[i]==0);
    assert(hq_underruns==1 && hq_missing_frames==4);
    hq_push(10,20);audio_playing.v=0;callback(4);
    for (int i=0;i<8;i++) assert(output_buffer[i]==0);
    assert(hq_read==hq_write && hq_discarded_frames==1);
    audio_playing.v=1;hq_push(30,40);callback(1);assert(output_buffer[0]==30 && output_buffer[1]==40);
    hq_push(50,60);ending_emulator_flag=1;callback(1);
    assert(output_buffer[0]==0 && output_buffer[1]==0 && hq_read==hq_write);
    reset_fifo();hq_write=hq_read=UINT_MAX-8;
    for (int i=0;i<32;i++) hq_push(i,-i);
    callback(32);for (int i=0;i<32;i++) assert(output_buffer[i*2]==i && output_buffer[i*2+1]==-i);
    reset_fifo();pthread_t p,c;
    assert(!pthread_create(&p,NULL,producer,NULL));assert(!pthread_create(&c,NULL,consumer,NULL));
    assert(!pthread_join(p,NULL));assert(!pthread_join(c,NULL));
    assert(!hq_overruns && !hq_underruns && hq_read==hq_write);
    puts("CoreAudio HQ: actual FIFO/callback passed (overflow, underrun silence, pause flush, resume, shutdown, counter wrap, 200000 concurrent frames)");
}
