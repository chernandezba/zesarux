/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>

#include <AudioUnit/AudioComponent.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/AudioHardware.h>

#include <CoreMIDI/CoreMIDI.h>

//Para funciones de lectura de sonido por fuente externa
#include <AudioToolbox/AudioToolbox.h>


#include "audio.h"
#include "audiocoreaudio.h"
#include "cpu.h"
#include "debug.h"
#include "settings.h"


//saudiocoreaudio_fifo_t sound_fifo;

char *buffer_actual;




/* info about the format used for writing to output unit */
static AudioStreamBasicDescription deviceFormat;

/* converts from Fuse format (signed 16 bit ints) to CoreAudio format (floats)
 */
static AudioUnit gOutputUnit;

/* Records sound writer status information */
static int audio_output_started;

void audiocoreaudio_fifo_write(char *origen,int longitud);

//Tamanyo de fifo. Es un multiplicador de AUDIO_BUFFER_SIZE
int audiocoreaudio_fifo_buffer_size_multiplier=2;
int audiocoreaudio_return_fifo_buffer_size(void)
{
  return AUDIO_BUFFER_SIZE*audiocoreaudio_fifo_buffer_size_multiplier*2; //*2 porque es stereo
}


//nuestra FIFO. De tamayo maximo. Por defecto es x2 y llega hasta xMAX_AUDIOCOREAUDIO_FIFO_MULTIPLIER
char audiocoreaudio_fifo_buffer[AUDIO_BUFFER_SIZE*MAX_AUDIOCOREAUDIO_FIFO_MULTIPLIER*2]; //*2 porque es stereo

static
OSStatus coreaudiowrite( void *inRefCon,
                         AudioUnitRenderActionFlags *ioActionFlags,
                         const AudioTimeStamp *inTimeStamp,
                         UInt32 inBusNumber,
                         UInt32 inNumberFrames,
                         AudioBufferList *ioData );


/* get the default output device for the HAL */
int get_default_output_device(AudioDeviceID* device)
{
  OSStatus err = kAudioHardwareNoError;
  UInt32 count;

  AudioObjectPropertyAddress property_address = {
    kAudioHardwarePropertyDefaultOutputDevice,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
  };

  /* get the default output device for the HAL */
  count = sizeof( *device );
  err = AudioObjectGetPropertyData( kAudioObjectSystemObject, &property_address,
                                    0, NULL, &count, device);
  if ( err != kAudioHardwareNoError && device != kAudioObjectUnknown ) {
    debug_printf( VERBOSE_ERR,
              "get kAudioHardwarePropertyDefaultOutputDevice error %ld",
              (long)err );
    return 1;
  }

  return 0;
}


/* get the nominal sample rate used by the supplied device */
static int
get_default_sample_rate( AudioDeviceID device, Float64 *rate )
{
  OSStatus err = kAudioHardwareNoError;
  UInt32 count;

  AudioObjectPropertyAddress property_address = {
    kAudioDevicePropertyNominalSampleRate,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
  };

  /* get the default output device for the HAL */
  count = sizeof( *rate );
  err = AudioObjectGetPropertyData( device, &property_address, 0, NULL, &count,
                                    rate);
  if ( err != kAudioHardwareNoError ) {
    debug_printf( VERBOSE_ERR,
              "get kAudioDevicePropertyNominalSampleRate error %ld",
              (long)err );
    return 1;
  }

  return 0;
}



int audiocoreaudio_init(void)
{

	//audio_driver_accepts_stereo.v=1;


	debug_printf (VERBOSE_INFO,"Init CoreAudio Driver, %d Hz",FRECUENCIA_SONIDO);

	buffer_actual=audio_buffer_one;

int *freqptr;
int freqqqq;
freqptr=&freqqqq;

  OSStatus err = kAudioHardwareNoError;
  AudioDeviceID device = kAudioObjectUnknown; /* the default device */
  //int error;
  float hz;
  int sound_framesiz;

  if( get_default_output_device(&device) ) return 1;
  if( get_default_sample_rate( device, &deviceFormat.mSampleRate ) ) return 1;

  *freqptr = deviceFormat.mSampleRate;

  deviceFormat.mFormatID =  kAudioFormatLinearPCM;
  deviceFormat.mFormatFlags =  kLinearPCMFormatFlagIsSignedInteger
#ifdef WORDS_BIGENDIAN
                    | kLinearPCMFormatFlagIsBigEndian
#endif      /* #ifdef WORDS_BIGENDIAN */
                    | kLinearPCMFormatFlagIsPacked;

//inDesc.mSampleRate=rate;

//parece que esto no hace nada
deviceFormat.mSampleRate=FRECUENCIA_SONIDO;


char *stereoptr;
char pepe;
stereoptr=&pepe;

*stereoptr=0x0;

  deviceFormat.mBytesPerPacket = *stereoptr ? 4 : 2;
  deviceFormat.mBytesPerPacket=2;  //1=mono, 2=stereo

  deviceFormat.mFramesPerPacket = 1;

  deviceFormat.mBytesPerFrame = *stereoptr ? 4 : 2;
  deviceFormat.mBytesPerFrame = 2; //1=mono, 2=stereo

  //deviceFormat.mBitsPerChannel = 16;
  deviceFormat.mBitsPerChannel = 8;

  deviceFormat.mChannelsPerFrame = *stereoptr ? 2 : 1;
  deviceFormat.mChannelsPerFrame = 2; //1=mono, 2=stereo

  /* Open the default output unit */
  AudioComponentDescription desc;
  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_DefaultOutput;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;

  AudioComponent comp = AudioComponentFindNext( NULL, &desc );
  if( comp == NULL ) {
    debug_printf( VERBOSE_ERR, "AudioComponentFindNext" );
    return 1;
  }

  err = AudioComponentInstanceNew( comp, &gOutputUnit );
  if( comp == NULL ) {
    debug_printf( VERBOSE_ERR, "AudioComponentInstanceNew=%ld", (long)err );
    return 1;
  }

  /* Set up a callback function to generate output to the output unit */
  AURenderCallbackStruct input;
  input.inputProc = coreaudiowrite;
  input.inputProcRefCon = NULL;

  err = AudioUnitSetProperty( gOutputUnit,
                              kAudioUnitProperty_SetRenderCallback,
                              kAudioUnitScope_Input,
                              0,
                              &input,
                              sizeof( input ) );
  if( err ) {
    debug_printf( VERBOSE_ERR, "AudioUnitSetProperty-CB=%ld", (long)err );
    return 1;
  }

  err = AudioUnitSetProperty( gOutputUnit,
                              kAudioUnitProperty_StreamFormat,
                              kAudioUnitScope_Input,
                              0,
                              &deviceFormat,
                              sizeof( AudioStreamBasicDescription ) );
  if( err ) {
    debug_printf( VERBOSE_ERR, "AudioUnitSetProperty-SF=%4.4s, %ld", (char*)&err,
              (long)err );
    return 1;
  }

  err = AudioUnitInitialize( gOutputUnit );
  if( err ) {
    debug_printf( VERBOSE_ERR, "AudioUnitInitialize=%ld", (long)err );
    return 1;
  }

  /* Adjust relative processor speed to deal with adjusting sound generation
     frequency against emulation speed (more flexible than adjusting generated
     sample rate) */
  //hz = (float)sound_get_effective_processor_speed() /
    //          machine_current->timings.tstates_per_frame;


  /* Amount of audio data we will accumulate before yielding back to the OS.
     Not much point having more than 100Hz playback, we probably get
     downgraded by the OS as being a hog too (unlimited Hz limits playback
     speed to about 2000% on my Mac, 100Hz allows up to 5000% for me) */


	hz=FRECUENCIA_SONIDO;

  sound_framesiz = deviceFormat.mSampleRate / hz;

  /* wait to run sound until we have some sound to play */
  audio_output_started = 0;




	//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
	audio_set_driver_name("coreaudio");




  return 0;
}


void sound_lowlevel_frame( char *data, int len );


//
//Inicio funciones de lectura de sonido por fuente externa
//



//https://developer.apple.com/forums/thread/70916
//https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioQueueProgrammingGuide/AQRecord/RecordingAudio.html


static const int kNumberBuffers = 1;

int recording_num_buffer=0;
int recording_num_buffer_lectura=0;

struct AQRecorderState {
    AudioStreamBasicDescription  mDataFormat;                   // 2
    AudioQueueRef                mQueue;                        // 3
    AudioQueueBufferRef          mBuffers[kNumberBuffers];      // 4
    AudioFileID                  mAudioFile;                    // 5
    UInt32                       bufferByteSize;                // 6
    SInt64                       mCurrentPacket;                // 7
    bool                         mIsRunning;                    // 8
};

/*
                         AudioBufferList *ioData )
{
  int len = deviceFormat.mBytesPerFrame * inNumberFrames;
  uint8_t* out = ioData->mBuffers[0].mData;
*/

int saltado_interrupcion_audiocoreaudio=0;

char buffer_input_stereo[2][AUDIO_BUFFER_SIZE*2];

void audiocoreaudio_start_recording_oneshoot(void);

struct AQRecorderState S;

void HandleInputBuffer (
                               void                                *aqData,
                               AudioQueueRef                       inAQ,
                               AudioQueueBufferRef                 inBuffer,
                               const AudioTimeStamp                *inStartTime,
                               UInt32                              inNumPackets,
                               const AudioStreamPacketDescription  *inPacketDesc
) {

    struct AQRecorderState *pAqData = (struct AQRecorderState *) aqData;

    if (inNumPackets == 0 && pAqData->mDataFormat.mBytesPerPacket != 0) {
        inNumPackets = inBuffer->mAudioDataByteSize / pAqData->mDataFormat.mBytesPerPacket;
    }
    //printf("%d\n", *(char*)inBuffer->mAudioData);
    //if (pAqData->mIsRunning == 0)
    //    return;

    AudioQueueEnqueueBuffer(pAqData->mQueue, inBuffer, 0, NULL);
    saltado_interrupcion_audiocoreaudio=1;
    printf("End input. %d\n",inNumPackets);

    //recording_num_buffer ^=1;

    //Y vuelta a reproducir
    //audiocoreaudio_start_recording_oneshoot();

        //convertir a stereo
        char *buffer_in=S.mBuffers[0]->mAudioData;
        int i;
        for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
            printf("%d ",buffer_in[i]);
            buffer_input_stereo[recording_num_buffer_lectura][i*2]=buffer_in[i];
            buffer_input_stereo[recording_num_buffer_lectura][i*2+1]=buffer_in[i];

        }
        printf("\n");
}

/*void envia_sonido(char *buffer,int longitud)
{
    int i;
        for (i=0;i<longitud ;i++) {
        printf("%d ",buffer[i]);

        int indice_destino=i*2;

        if (indice_destino<AUDIO_BUFFER_SIZE*2-2) {
            audio_buffer[indice_destino]=buffer[i];
            audio_buffer[indice_destino+1]=buffer[i];
        }
    }
}*/



int audiocoreaudio_esta_grabando=0;

void audiocoreaudio_start_recording_oneshoot_continue(void)
{

OSStatus r = 0;



    S.mCurrentPacket = 0;
    S.mIsRunning = true;

    r = AudioQueueStart(S.mQueue, NULL);
    int tiempo_sleep=1000000/(15600/S.bufferByteSize);
    //printf("tiempo_sleep: %d\n",tiempo_sleep);

    audiocoreaudio_esta_grabando=1;
    //usleep(tiempo_sleep);

    //printf("After reading\n");
    //PRINT_R;

    //r = AudioQueueStop(S.mQueue, true);
    //S.mIsRunning = false;

    //PRINT_R;

    //r = AudioQueueDispose(S.mQueue, true);
    //AudioQueueBuffer *buffer=&S.mQueue;
    //AudioQueueBuffer *buffer=S.mBuffers[0];

    //char *buffer=S.mBuffers[0]->mAudioData;


    //envia_sonido(buffer,S.bufferByteSize);


    //printf("\n");

    //recording_num_buffer ^=1;

}

void audiocoreaudio_start_recording_oneshoot(void)
{


#define PRINT_R do{\
printf("%d: r = %d\n",__LINE__, r);\
}while(0)

    AudioStreamBasicDescription *fmt = &S.mDataFormat;

    fmt->mFormatID         = kAudioFormatLinearPCM;
    fmt->mSampleRate       = 15600;
    fmt->mChannelsPerFrame = 1;
    fmt->mBitsPerChannel   = 8;
    fmt->mBytesPerFrame    = fmt->mChannelsPerFrame * sizeof (char);
    fmt->mFramesPerPacket  = 1;
    fmt->mBytesPerPacket   = fmt->mBytesPerFrame * fmt->mFramesPerPacket;
    fmt->mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger;



/*
if( get_default_output_device(&device) ) return 1;
  if( get_default_sample_rate( device, &deviceFormat.mSampleRate ) ) return 1;

  *freqptr = deviceFormat.mSampleRate;

  deviceFormat.mFormatID =  kAudioFormatLinearPCM;
  deviceFormat.mFormatFlags =  kLinearPCMFormatFlagIsSignedInteger
#ifdef WORDS_BIGENDIAN
                    | kLinearPCMFormatFlagIsBigEndian

                    | kLinearPCMFormatFlagIsPacked;

//inDe

*stereoptr=0x0;

  deviceFormat.mBytesPerPacket = *stereoptr ? 4 : 2;
  deviceFormat.mBytesPerPacket=2;  //1=mono, 2=stereo

  deviceFormat.mFramesPerPacket = 1;

  deviceFormat.mBytesPerFrame = *stereoptr ? 4 : 2;
  deviceFormat.mBytesPerFrame = 2; //1=mono, 2=stereo

  //deviceFormat.mBitsPerChannel = 16;
  deviceFormat.mBitsPerChannel = 8;

  deviceFormat.mChannelsPerFrame = *stereoptr ? 2 : 1;
  deviceFormat.mChannelsPerFrame = 2; //1=mono, 2=stereo

  AudioComponentDescription desc;
  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_DefaultOutput;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;

*/



    OSStatus r = 0;

    r = AudioQueueNewInput(&S.mDataFormat, HandleInputBuffer, &S, NULL, kCFRunLoopCommonModes, 0, &S.mQueue);

    //PRINT_R;

    UInt32 dataFormatSize = sizeof (S.mDataFormat);

    r = AudioQueueGetProperty (
                           S.mQueue,
                           kAudioConverterCurrentInputStreamDescription,
                           &S.mDataFormat,
                           &dataFormatSize
                           );

    S.bufferByteSize = AUDIO_BUFFER_SIZE;

    int i;
    //for (i = 0; i < kNumberBuffers; ++i) {
        r = AudioQueueAllocateBuffer(S.mQueue, S.bufferByteSize, &S.mBuffers[recording_num_buffer]);
        //PRINT_R;
        r = AudioQueueEnqueueBuffer(S.mQueue, S.mBuffers[recording_num_buffer], 0, NULL);
        //PRINT_R;
    //}

    audiocoreaudio_start_recording_oneshoot_continue();

}






void encolar_sonido_output_coreaudio(char *buffer_send_frame)
{

    //hasta que no haya grabado una vez, y por tanto los buffers de input esten asignados, no lanzar
    if (!audiocoreaudio_esta_grabando) {
        printf("Aun no estan asignados los buffers de input\n");
        audiocoreaudio_start_recording_oneshoot();
        //sleep(1);
    }

    else {

        int buffer_lectura=recording_num_buffer ^1; //_lectura;



        char *buffer_in=S.mBuffers[buffer_lectura]->mAudioData;

        //convertir a stereo
        int i;
        for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
            //invento feo para alterar el buffer de origen y poder "ver" el sonido en ventana view waveform
            //QUITAR ESTO DE LA VERSION FINAL!!!
            buffer_send_frame[i*2]=buffer_input_stereo[recording_num_buffer_lectura][i*2];
            buffer_send_frame[i*2+1]=buffer_input_stereo[recording_num_buffer_lectura][i*2+1];
        }

        audiocoreaudio_fifo_write(buffer_input_stereo[recording_num_buffer_lectura],AUDIO_BUFFER_SIZE);
        //envia_sonido(buffer_in,S.bufferByteSize);

        recording_num_buffer_lectura ^=1;
        audiocoreaudio_start_recording_oneshoot();
    }


    //Y volver a capturar sonido
    //audiocoreaudio_start_recording_oneshoot();
}

int audiocoreaudio_check_interrupt(void)
{
    if (!audiocoreaudio_esta_grabando) return 1;

    if (saltado_interrupcion_audiocoreaudio)
    {
        saltado_interrupcion_audiocoreaudio=0;
        return 1;
    }
printf("no timer\n");
    return 0;
}

//
//FIN funciones de lectura de sonido por fuente externa
//


void audiocoreaudio_send_frame(char *buffer)
{

if (audio_playing.v==0) audio_playing.v=1;

//sound_lowlevel_frame(buffer,AUDIO_BUFFER_SIZE);
buffer_actual=buffer;


  if( !audio_output_started ) {
    /* Start the rendering
       The DefaultOutputUnit will do any format conversions to the format of the
       default device */
    OSStatus err ;

	err= AudioOutputUnitStart( gOutputUnit );
    if( err ) {
      debug_printf( VERBOSE_ERR, "AudioOutputUnitStart=%ld", (long)err );
      return;
    }

    audio_output_started = 1;
  }

audiocoreaudio_fifo_write(buffer,AUDIO_BUFFER_SIZE);

    //Pruebas a escuchar sonido por line in. Comentar el audiocoreaudio_fifo_write anterior para que funcione
    //encolar_sonido_output_coreaudio(buffer);

}

int audiocoreaudio_thread_finish(void)
{

return 0;
}

void audiocoreaudio_end(void)
{
        debug_printf (VERBOSE_INFO,"Ending coreaudio audio driver");
	audiocoreaudio_thread_finish();


	struct AURenderCallbackStruct callback;

	/* stop processing the audio unit */
	if (AudioOutputUnitStop(gOutputUnit) != noErr) {
		debug_printf (VERBOSE_ERR,"audiocoreaudio_end AudioOutputUnitStop failed");
		return;
	}

	/* Remove the input callback */
	callback.inputProc = 0;
	callback.inputProcRefCon = 0;
	if (AudioUnitSetProperty(gOutputUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback)) != noErr) {
		debug_printf (VERBOSE_ERR,"audiocoreaudio_end AudioUnitSetProperty failed");
		return;
	}

	/*if (CloseComponent(gOutputUnit) != noErr) {
		debug_printf (VERBOSE_ERR,"audiocoreaudio_end CloseComponent failed");
		return;
	}*/

}

int audiocoreaudio_fifo_write_position=0;
int audiocoreaudio_fifo_read_position=0;

void audiocoreaudio_empty_buffer(void)
{
  debug_printf(VERBOSE_DEBUG,"Emptying audio buffer");
  audiocoreaudio_fifo_write_position=0;
}



//retorna numero de elementos en la fifo
int audiocoreaudio_fifo_return_size(void)
{
	//si write es mayor o igual (caso normal)
	if (audiocoreaudio_fifo_write_position>=audiocoreaudio_fifo_read_position) return audiocoreaudio_fifo_write_position-audiocoreaudio_fifo_read_position;

	else {
		//write es menor, cosa que quiere decir que hemos dado la vuelta
		return (audiocoreaudio_return_fifo_buffer_size()-audiocoreaudio_fifo_read_position)+audiocoreaudio_fifo_write_position;
	}
}

void audiocoreaudio_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=audiocoreaudio_return_fifo_buffer_size();
  *current_size=audiocoreaudio_fifo_return_size();
}

//retornar siguiente valor para indice. normalmente +1 a no ser que se de la vuelta
int audiocoreaudio_fifo_next_index(int v)
{
	v=v+1;
	if (v==audiocoreaudio_return_fifo_buffer_size()) v=0;

	return v;
}

//escribir datos en la fifo
void audiocoreaudio_fifo_write(char *origen,int longitud)
{
	for (;longitud>0;longitud--) {

		//ver si la escritura alcanza la lectura. en ese caso, error
		if (audiocoreaudio_fifo_next_index(audiocoreaudio_fifo_write_position)==audiocoreaudio_fifo_read_position) {
			debug_printf (VERBOSE_DEBUG,"audiocoreaudio FIFO full");
      //Si se llena fifo, resetearla a 0 para corregir latencia
      if (audio_noreset_audiobuffer_full.v==0) audiocoreaudio_empty_buffer();
			return;
		}

    //Canal izquierdo
		audiocoreaudio_fifo_buffer[audiocoreaudio_fifo_write_position]=*origen++;
		audiocoreaudio_fifo_write_position=audiocoreaudio_fifo_next_index(audiocoreaudio_fifo_write_position);

    //Canal derecho
		audiocoreaudio_fifo_buffer[audiocoreaudio_fifo_write_position]=*origen++;
		audiocoreaudio_fifo_write_position=audiocoreaudio_fifo_next_index(audiocoreaudio_fifo_write_position);
	}
}


//leer datos de la fifo
//void audiocoreaudio_fifo_read(char *destino,int longitud)
void audiocoreaudio_fifo_read(uint8_t *destino,int longitud)
{
	for (;longitud>0;longitud--) {



                if (audiocoreaudio_fifo_return_size()==0) {
                        debug_printf (VERBOSE_INFO,"audiocoreaudio FIFO empty");
                        return;
                }


                //ver si la lectura alcanza la escritura. en ese caso, error
                //if (audiocoreaudio_fifo_next_index(audiocoreaudio_fifo_read_position)==audiocoreaudio_fifo_write_position) {
                //        debug_printf (VERBOSE_DEBUG,"FIFO vacia");
                //        return;
                //}

                //Canal izquierdo

                //La lectura nos viene del driver coreaudio, nos pregunta por bytes, independientemente del canal izquierdo o derecho
                *destino++=audiocoreaudio_fifo_buffer[audiocoreaudio_fifo_read_position];
                audiocoreaudio_fifo_read_position=audiocoreaudio_fifo_next_index(audiocoreaudio_fifo_read_position);

                //Canal derecho
                //*destino++=audiocoreaudio_fifo_buffer[audiocoreaudio_fifo_read_position];
                //audiocoreaudio_fifo_read_position=audiocoreaudio_fifo_next_index(audiocoreaudio_fifo_read_position);
        }
}


//puntero del buffer
//el tema es que nuestro buffer tiene un tamanyo y mac os x usa otro de diferente longitud
//si nos pide menos audio del que tenemos, a la siguiente vez se le enviara lo que quede del buffer


OSStatus coreaudiowrite( void *inRefCon GCC_UNUSED,
                         AudioUnitRenderActionFlags *ioActionFlags GCC_UNUSED,
                         const AudioTimeStamp *inTimeStamp GCC_UNUSED,
                         UInt32 inBusNumber GCC_UNUSED,
                         UInt32 inNumberFrames,
                         AudioBufferList *ioData )
{
  int len = deviceFormat.mBytesPerFrame * inNumberFrames;
  uint8_t* out = ioData->mBuffers[0].mData;



	//si esta el sonido desactivado, enviamos silencio
	if (audio_playing.v==0) {
		uint8_t *puntero_salida;
		puntero_salida = out;
		while (len>0) {
			*puntero_salida=0;
			puntero_salida++;
			len--;
		}
		//printf ("audio_playing.v=0 en audiocoreaudio\n");
	}

	else {

		//printf ("coreaudiowrite. longitud pedida: %d AUDIO_BUFFER_SIZE: %d\n",len,AUDIO_BUFFER_SIZE);
		if (len>audiocoreaudio_fifo_return_size()) {
			//debug_printf (VERBOSE_DEBUG,"FIFO is not big enough. Length asked: %d audiocoreaudio_fifo_return_size: %d",len,audiocoreaudio_fifo_return_size() );
			//esto puede pasar con el detector de silencio

			//retornar solo lo que tenemos
			//audiocoreaudio_fifo_read(out,audiocoreaudio_fifo_return_size() );

			return noErr;
		}


		else {
			audiocoreaudio_fifo_read(out,len);
		}

	}



return noErr;

}



// Midi code derived from work of Craig Stuart Sapp:
//
// Programmer:	Craig Stuart Sapp
// Date:	Mon Jun  8 14:54:42 PDT 2009
//
// Derived from "Audio and MIDI on Mac OS X" Preliminary Documentation,
// May 2001 Apple Computer, Inc. found in PDF form on the developer.apple.com
// website, as well as using links at the bottom of the file.
//




void playPacketListOnAllDevices   (MIDIPortRef     midiout,  const MIDIPacketList* pktlist);


MIDIClientRef coreaudio_midi_midiclient;
MIDIPortRef   coreaudio_midi_midiout;

MIDIPacketList *coreaudio_midi_packetlist=NULL;
MIDIPacket *coreaudio_midi_currentpacket=NULL;

//Mas que suficiente para almacenar 3 notas*3 canales
//de todas maneras, si no cabe al agregar, hace flush y reintenta
#define COREAUDIO_MIDI_BUFFER_SIZE 16384
z80_byte coreaudio_midi_buffer[COREAUDIO_MIDI_BUFFER_SIZE];

void coreaudio_mid_add_note(z80_byte *note,int messagesize)
{

  //Ver si hay espacio suficiente. Si no, flush

   MIDITimeStamp timestamp = 0;   // 0 will mean play now.

    coreaudio_midi_currentpacket = MIDIPacketListAdd(coreaudio_midi_packetlist, COREAUDIO_MIDI_BUFFER_SIZE,
         coreaudio_midi_currentpacket, timestamp, messagesize, note);

    if (coreaudio_midi_currentpacket==NULL) {
      debug_printf (VERBOSE_DEBUG,"Coreaudio midi queue was full. Flush and retry");
      //Hacemos flush y reintentamos
      coreaudio_midi_output_flush_output();
      coreaudio_midi_currentpacket = MIDIPacketListAdd(coreaudio_midi_packetlist, COREAUDIO_MIDI_BUFFER_SIZE,
         coreaudio_midi_currentpacket, timestamp, messagesize, note);
    }
}

void coreaudio_mid_raw_send(z80_byte value)
{
    z80_byte rawpacket[] = {value};

  coreaudio_mid_add_note(rawpacket,1);
}

void coreaudio_mid_initialize_queue(void)
{
   coreaudio_midi_packetlist = (MIDIPacketList*)coreaudio_midi_buffer;
   coreaudio_midi_currentpacket = MIDIPacketListInit(coreaudio_midi_packetlist);
}

void coreaudio_midi_output_flush_output(void)
{

   // send the MIDI data
   //Por si acaso comprobamos que no sea NULL
  if (coreaudio_midi_packetlist!=NULL) playPacketListOnAllDevices(coreaudio_midi_midiout, coreaudio_midi_packetlist);

  coreaudio_mid_initialize_queue();

}

void coreaudio_midi_output_reset(void)
{

//printf ("reset\n");

//TODO: Esto no parece hacer nada. El reset de midi solo parece funcionar en windows



    z80_byte resetcommand[] = {0xFF,0,0,0};
     coreaudio_mid_add_note(resetcommand,4);
/*

    int channel;

    for (channel=0;channel<16;channel++) {

  printf ("Sending reset\n");

  //All Sound Off=120
  //z80_byte resetcommand[] = {176, 121, 0};
  z80_byte resetcommand[] = {176, 120, 0};

  resetcommand[0] &=0xF0;
  resetcommand[0] |=channel;

  coreaudio_mid_add_note(resetcommand,3);

  z80_byte notesoffcommand[] = {176, 123, 0};

  notesoffcommand[0] &=0xF0;
  notesoffcommand[0] |=channel;

  coreaudio_mid_add_note(notesoffcommand,3);

    }
    */

}

int coreaudio_mid_initialize_all(void)
{
   // Prepare MIDI Interface Client/Port for writing MIDI data:


   OSStatus status;
   status = MIDIClientCreate(CFSTR("ZEsarUX"), NULL, NULL, &coreaudio_midi_midiclient);
   if (status) {
       debug_printf(VERBOSE_ERR,"Error trying to create MIDI Client structure: %d", status);
       //printf("%s\n", GetMacOSStatusErrorString(status));
       return 1;
   }
   status = MIDIOutputPortCreate(coreaudio_midi_midiclient, CFSTR("ZEsarUX output"), &coreaudio_midi_midiout);
   if (status) {
       debug_printf(VERBOSE_ERR,"Error trying to create MIDI output port: %d", status);
       //printf("%s\n", GetMacOSStatusErrorString(status));
       return 1;
   }


  coreaudio_mid_initialize_queue();


  return 0;
}


void coreaudio_mid_finish_all(void)
{
  OSStatus status;

    status = MIDIPortDispose(coreaudio_midi_midiout);
    if (status) {
      debug_printf(VERBOSE_ERR,"Error trying to close MIDI output port");
      return;
   }
   //coreaudio_midi_midiout = NULL;
  status = MIDIClientDispose(coreaudio_midi_midiclient);
   if (status) {
      debug_printf(VERBOSE_ERR,"Error trying to close MIDI client: %d");
   }
   //coreaudio_midi_midiclient = NULL;

   return;
}



//Hacer note on de una nota inmediatamente
int coreaudio_note_on(unsigned char channel, unsigned char note,unsigned char velocity)
{

  debug_printf (VERBOSE_PARANOID,"noteon event channel %d note %d velocity %d",channel,note,velocity);

  z80_byte noteon[] = {0x90, note, velocity};

  coreaudio_mid_add_note(noteon,3);

  return 0;
}

int coreaudio_note_off(unsigned char channel, unsigned char note,unsigned char velocity)
{

  debug_printf (VERBOSE_PARANOID,"noteoff event channel %d note %d velocity %d",channel,note,velocity);

  z80_byte noteoff[] = {0x80, note, velocity};

  coreaudio_mid_add_note(noteoff,3);


  return 0;
}

int coreaudio_change_instrument(unsigned char instrument)
{

  debug_printf (VERBOSE_PARANOID,"change instrument event instrument %d",instrument);

    //El mensaje seria 0xC0 + canal

    int i;
    for (i=0;i<16;i++) {

        z80_byte instrumentchange[] = {0xC0+i, instrument & 127};

        coreaudio_mid_add_note(instrumentchange,2);
    }


  return 0;
}


/////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// playPacketOnAllDevices -- play the list of MIDI packets
//    on all MIDI output devices which the computer knows about.
//    (Send the MIDI message(s) to all MIDI out ports).
//

void playPacketListOnAllDevices(MIDIPortRef midiout,const MIDIPacketList* pktlist)
{
   // send MIDI message to all MIDI output devices connected to computer:
   ItemCount nDests = MIDIGetNumberOfDestinations();
   ItemCount iDest;
   OSStatus status;
   MIDIEndpointRef dest;
   for(iDest=0; iDest<nDests; iDest++) {
      dest = MIDIGetDestination(iDest);
      status = MIDISend(midiout, dest, pktlist);
      if (status) {
          debug_printf(VERBOSE_DEBUG,"coreaudio_midi: Problem sending MIDI data");
          //printf("%s\n", GetMacOSStatusErrorString(status));
          //exit(status);
      }
   }
}


/////////////////////////////////////////////////////////////////////////
//
// NOTES
//
/*

Crapple functions used in this program:

/// TYPEDEFS //////////////////////////////////////////////////////////////

UInt16    => unsigned short int
UInt32    => unsigned long int
UInt64    => unsigned long long int
Byte      => char
ByteCount => int
OSStatus  => int
Boolean   => char

MIDITimeStamp => UInt64
   A host clock time representing the time of an event, as returned by
   mach_absolute_time() or UpTime().  Since MIDI applications will tend to
   do a fair amount of math with the times of events, it is more
   convenient to use a UInt64 than an AbsoluteTime.  See CoreAudio/HostTime.h

struct MIDIPacket { MIDITimeStamp timeStamp; UInt16 length; Byte data[256]; };
      timeStamp = The time at which the events occurred (if receiving MIDI),
                  or the time at which the events are to be played (if sending
                  MIDI).  Zero means "now" when sending MIDI data.  The time
                  stamp applies to the first MIDI byte in the packet.
      length    = The number of valid MIDI bytes which follow in data[].
                  It may be larger than 256 bytes if the packet is dynamically
                  allocated.
      data      = A variable-length stream of MIDI messages. Running status
                  is not allowed.  In the case of system-exclusive messages,
                  a packet may only contain a single message, or portion
                  of one, with no other MIDI events.  The MIDI messages in
                  the packet must always be complete, except for
                  system-exclusive messages.  data[] is declared to be 256
                  bytes in length so clients don't have to create custom data
                  structures in simple situations.

struct MIDIPacketList { UInt32  numPackets; MIDIPacket packet[1]; };
      numPackets = The number of MIDIPackets in the list.
      packet     = An open-ended array of variable-length MIDIPackets.
   The timestamps in the packet list must be in ascending order.
   Note that the packets in the list, while defined as an array, may
   *not* be accessed as an array, since they are vairable-length.  To
   iterate through the packets in a packet list, use a loop such as:
      MIDIPacket *packet = &packetList->packet[0];
      for (int i=0; i<packetList->numPackets; i++) {
         ...
         packet = MIDIPacketNext(packet);
      }

struct MIDISysexSendRequest {
   MIDIEndpointRef destination;
   const Byte     *data;
   UInt32          bytesToSend;
   Boolean         complete;
   Byte            reserved[3];  // to fill up 4-byte boundary, I suppose.
   MIDICompetionProc completionProc;
   void *completionRefCon;
};
      destination      = The endpoint to which the event is to be sent.
      data             = Initially, a pointer to the sys-ex event to be
                         sent.  MIDISendSysex will advance this pointer
                         as bytes are sent.
      bytesToSend      = Initially, the number of bytes to be sent.
                         MIDISendSysex will decrement this counter
                         as bytes are sent.
      complete         = The client may set this to true at any time
                         to abort transmission.  The implementation
                         sets this to true when all bytes have been sent.
      completionProc   = Called when all bytes ahve been sent, or after the
                         client has set complete to true.
      completionRefCon = Passed as a refCon to completionProc.
   This data structure represents a request to send a single system-exclusive
   MIDI event to a MIDI destination asynchronously.


/// MIDI OPENING/CLOSING FUNCTIONS ////////////////////////////////////////

OSStatus MIDIClientCreate(CFStringRef name, MIDINotifyProc notifyProc,
                           void* notifyRefCon, MIDIClientRef* outClient);
      name         = The client's name.
      notifyProc   = An optional (may be NULL) callback function through
                     which the client will receive notifications of changes
                     to the system.
      notifyRefCon = An optional (may be NULL) refCon passed back to
                     notifyRefCon.
      outClient    = On successful return, points to the newly-created
                     MIDIClientRef.
   All clients must be created and disposed on the same thread.
   Note that notifyProc will always be called on the run loop
   which was current when MIDIClientCreate was first called.


OSStatus MIDIOutputPortCreate(MidiClientRef midiclient, CFStringRef portName,
                              MIDIPortRef *outPort);
      client   = The client to own the newly-created port.
      portName = The name of the port.
      outPort  = On successful return, points to the newly-created MIDIPort.
   Creates an output port through which the client may send outgoing
   MIDI messages to any MIDI destination.  Output ports provied a
   mechanism for MIDI merging.  CoreMIDI assumes that each output port
   will be responsible for sending only a single MIDI stream to each
   destination, although a single port may address all of the destinations
   in the system.  Multiple output ports are only necessary when an
   application is capable of directing multiple simultaneous MIDI streams
   to the same destination.


OSStatus MIDIPortDispose(MIDIPortRef port);
      port = The port to dispose.
   It is not usually necessary to call this function.  When an application's
   MIDIClient's are automatically disposed at termination, or explicitly,
   via MIDIClientDispose, the client's ports are automatically disposed at
   that time.

OSS MIDIClientDispose(MIDIClientRef client);
      client = The client to dispose
   Not an essential function to call; CoreMIDI framework will
   automatically dispose all MIDIClients when an application terminates.


/// MIDI PACKET FUNCTIONS /////////////////////////////////////////////////

MIDIPacket* MIDIPacketListInit(MIDIPacketList* packetList);
      packetList = The packet list to be initialized.
   Returns a pointer to the first MIDIPacket in the packet list.

MIDIPacket* MIDIPacketListAdd(MIDIPacketList* packetList, ByteCount listSize,
                                   MIDIPacket* curPacket, MIDITimeStamp time,
                                   ByteCount nData, const Byte* data);
      packetList = The packet list to which the event is to be added.
      listSize   = The size, in bytes, of the packet list.
      curPacket  = A packet pointer returned by a previous call to
                   MIDIPacketListInit() or MIDIPacketListAdd() for this
                   packet list.
      time       = The new event's time (when to play the MIDI event
                   when this is output data).
      nData      = The length of the new event, in bytes.
      data       = The new event.  May be a single MIDI event, or a
                   partial sys-ex event.  Running status is *not*
                   permitted.
   Returns null if there was not room in the packet for the event; otherwise,
   returns a packet point which should be passed as CurPacket in a
   subsequent call to this function.  The maximum size of a packet list is
   65536 bytes.  Large sysex messages must be sent in smaller packet
   lists.



/// MIDI SEND FUNCTIONS ///////////////////////////////////////////////////

OSStatus MIDISend(MIDIPortRef port, MIDIEndpointRef dest,
                                              const MIDIPacketList *packetList);
      port       = The output port through which the MIDI is to be sent.
      dest       = The destination to receive the events.
      packetList = The MIDI events to be sent.
   Events with future timestamps are scheduled for future delivery.
   CoreMIDI performs and needed MIDI merging.



/// OTHER USEFUL FUNCTIONS ////////////////////////////////////////////////

OSStatus MIDIRestart(void);
   This function forces CoreMIDI to ask its drivers to rescan for hardware.
   (OSX10.1 and later).

OSStatus MIDISendSysex(MIDISysexSendRequest* request);
      request = contains the destination, and a pointer to the MIDI data
                to be sent.
   request->data must point to a single complete or partial MIDI
   system-exclusive message.


/// LINKS AND MISC. NOTES /////////////////////////////////////////////////

MIDIServices.h Documentation:
   http://developer.apple.com/DOCUMENTATION/MusicAudio/Reference/CACoreMIDIRef/MIDIServices/CompositePage.html

CoreMIDI Documentation:
   https://developer.apple.com/documentation/MusicAudio/Reference/CACoreMIDIRef/MIDIServices/index.html

OSStatus Information:
   https://developer.apple.com/documentation/Carbon/Reference/ErrorHandler/Reference/reference.html#//apple_ref/doc/uid/TP40000867-CH201-DontLinkElementID_2

// Example MIDI I/O program: http://bl0rg.net/~manuel/midi-merge.c
// MIDIOutputPortCreate: http://xmidi.com/docs/coremidi34.html
// MIDI echo example: http://www.allegro.cc/forums/thread/598206
// MidiPacket *MIDIPacketNext(MidiPacket *pkt);
// MidiPacket *MIDIPacketListInit(MidiPacketList *pktlist);
// MIDIPacket *MIDIPacketListAdd(MIDIPacketList *pktlist, ByteCount
//    listSize, MidiPacket *curPacket, MIDITimeStamp time, ByteCount nData,
//    Byte *data);
// MIDISend(MIDIPortRef port, MIDIEndpointRef dest, const MIDIPacketList*pktlist);
// OSStatus string: http://lists.apple.com/archives/QuickTime-API/2006/Oct/msg00092.html


*/
