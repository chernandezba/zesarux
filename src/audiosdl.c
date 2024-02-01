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

#ifdef MINGW
    #include <windows.h>
    #include <mmsystem.h>
#endif

#include <string.h>
#include <stdio.h>
#if defined(__APPLE__)
        #include <SDL.h>
#else
	#include <SDL/SDL.h>
#endif



#include "compileoptions.h"
#include "common_sdl.h"
#include "audiosdl.h"
#include "audio.h"
#include "cpu.h"
#include "debug.h"
#include "settings.h"
#include "timer.h"


//char *buffer_actual;


void audiosdl_fifo_sdl_write(char *origen,int longitud);

void audiosdl_callback(void *udata, Uint8 *stream, int len);

//En versiones anteriores venia por defecto a 1024
int audiosdl_samples=DEFAULT_AUDIOSDL_SAMPLES;

int audiosdl_init(void)
{


	//audio_driver_accepts_stereo.v=1;

	//buffer_actual=audio_buffer_one;


	debug_printf (VERBOSE_INFO,"Init SDL Audio Driver, %d Hz, sample size: %d",FRECUENCIA_SONIDO,audiosdl_samples);


    	if( commonsdl_init() != 0 ) {
		debug_printf (VERBOSE_ERR,"audio sdl driver. Error initializing driver");
                return 1;
	}

	audiosdl_inicializado.v=1;


	SDL_AudioSpec wanted;

	/* Set the audio format */
	wanted.freq = FRECUENCIA_SONIDO;
	wanted.format = AUDIO_S8;
	wanted.channels = 2;    /* 1 = mono, 2 = stereo */
	wanted.samples = audiosdl_samples;  /* 1024 -> Good low-latency value for callback */
	wanted.callback = audiosdl_callback;
	wanted.userdata = NULL;

	/* Open the audio device, forcing the desired format */
	if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
		debug_printf( VERBOSE_ERR, "Could not open sdl audio: %s", SDL_GetError());
	        return 1;
	}

	//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
	audio_set_driver_name("sdl");

	return 0;

}


void audiosdl_send_frame(char *buffer)
{

	if (audio_playing.v==0) audio_playing.v=1;

	//buffer_actual=buffer;

	audiosdl_fifo_sdl_write(buffer,AUDIO_BUFFER_SIZE);
	SDL_PauseAudio(0);


}

int audiosdl_thread_finish(void)
{

	return 0;

}

void audiosdl_end(void)
{
        debug_printf (VERBOSE_INFO,"Ending SDL audio driver");
	audiosdl_thread_finish();

	SDL_CloseAudio();

        audiosdl_inicializado.v=0;
        commonsdl_end();

}



int audiosdl_fifo_sdl_write_position=0;
int audiosdl_fifo_sdl_read_position=0;

void audiosdl_empty_buffer(void)
{
  debug_printf(VERBOSE_DEBUG,"Emptying audio buffer");
  audiosdl_fifo_sdl_write_position=0;
}

//nuestra FIFO
//#define audiosdl_return_fifo_buffer_size() (AUDIO_BUFFER_SIZE*4)
//#define audiosdl_return_fifo_buffer_size() (AUDIO_BUFFER_SIZE*2)


//Tamanyo de fifo. Es un multiplicador de AUDIO_BUFFER_SIZE
int audiosdl_fifo_buffer_size_multiplier=2;
int audiosdl_return_fifo_buffer_size(void)
{
  return AUDIO_BUFFER_SIZE*audiosdl_fifo_buffer_size_multiplier*2; //*2 porque es stereo
}


//nuestra FIFO. De tamayo maximo. Por defecto es x2 y llega hasta xMAX_AUDIOCOREAUDIO_FIFO_MULTIPLIER
char audiosdl_fifo_sdl_buffer[AUDIO_BUFFER_SIZE*MAX_AUDIOSDL_FIFO_MULTIPLIER*2]; //*2 porque es stereo


//retorna numero de elementos en la fifo
int audiosdl_fifo_sdl_return_size(void)
{
	//si write es mayor o igual (caso normal)
	if (audiosdl_fifo_sdl_write_position>=audiosdl_fifo_sdl_read_position) return audiosdl_fifo_sdl_write_position-audiosdl_fifo_sdl_read_position;

	else {
		//write es menor, cosa que quiere decir que hemos dado la vuelta
		return (audiosdl_return_fifo_buffer_size()-audiosdl_fifo_sdl_read_position)+audiosdl_fifo_sdl_write_position;
	}
}

void audiosdl_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=audiosdl_return_fifo_buffer_size();
  *current_size=audiosdl_fifo_sdl_return_size();
}

//retornar siguiente valor para indice. normalmente +1 a no ser que se de la vuelta
int audiosdl_fifo_sdl_next_index(int v)
{
	v=v+1;
	if (v==audiosdl_return_fifo_buffer_size()) v=0;

	return v;
}

//escribir datos en la fifo
void audiosdl_fifo_sdl_write(char *origen,int longitud)
{
	for (;longitud>0;longitud--) {

		//ver si la escritura alcanza la lectura. en ese caso, error
		if (audiosdl_fifo_sdl_next_index(audiosdl_fifo_sdl_write_position)==audiosdl_fifo_sdl_read_position) {
			debug_printf (VERBOSE_DEBUG,"audiosdl FIFO full");
      //Si se llena fifo, resetearla a 0 para corregir latencia
      if (audio_noreset_audiobuffer_full.v==0) audiosdl_empty_buffer();
			return;
		}

		//Canal izquierdo
		audiosdl_fifo_sdl_buffer[audiosdl_fifo_sdl_write_position]=*origen++;
		audiosdl_fifo_sdl_write_position=audiosdl_fifo_sdl_next_index(audiosdl_fifo_sdl_write_position);

		//Canal derecho
		audiosdl_fifo_sdl_buffer[audiosdl_fifo_sdl_write_position]=*origen++;
		audiosdl_fifo_sdl_write_position=audiosdl_fifo_sdl_next_index(audiosdl_fifo_sdl_write_position);
	}
}


//leer datos de la fifo
void audiosdl_fifo_sdl_read(char *destino,int longitud)
{
	for (;longitud>0;longitud--) {



                if (audiosdl_fifo_sdl_return_size()==0) {
                        debug_printf (VERBOSE_INFO,"audiosdl FIFO empty");
                        return;
                }




                //ver si la lectura alcanza la escritura. en ese caso, error
                //if (audiosdl_fifo_sdl_next_index(audiosdl_fifo_sdl_read_position)==audiosdl_fifo_sdl_write_position) {
                //        debug_printf (VERBOSE_DEBUG,"FIFO vacia");
                //        return;
                //}



                *destino++=audiosdl_fifo_sdl_buffer[audiosdl_fifo_sdl_read_position];
                audiosdl_fifo_sdl_read_position=audiosdl_fifo_sdl_next_index(audiosdl_fifo_sdl_read_position);
        }
}


//puntero del buffer
//el tema es que nuestro buffer tiene un tamanyo y mac os x usa otro de diferente longitud
//si nos pide menos audio del que tenemos, a la siguiente vez se le enviara lo que quede del buffer

int contador_buffer_sonido=0;

char temporary_audiosdl_fifo_sdl_buffer[AUDIO_BUFFER_SIZE*MAX_AUDIOSDL_FIFO_MULTIPLIER*2];

//ver http://www.libsdl.org/release/SDL-1.2.15/docs/html/guideaudioexamples.html




void audiosdl_callback(void *udata, Uint8 *stream, int len)
{

	//printf ("audiosdl_callback\n");

	//para que no se queje el compilador de no usado
	udata++;

	//si esta el sonido desactivado, enviamos silencio
	if (audio_playing.v==0) {
		char *puntero_salida;
		puntero_salida = temporary_audiosdl_fifo_sdl_buffer;
		int longitud=len;
		while (longitud>0) {
			*puntero_salida=0;
			puntero_salida++;
			longitud--;
		}
		//printf ("audio_playing.v=0 en audiosdl\n");
	}

	else {

        //Nuevo Callback. Ver las notas sobre el nuevo callback justo despues de esta funcion
        if (audiosdl_use_new_callback.v) {

            int indice=0;


            int tamanyo_fifo=audiosdl_fifo_sdl_return_size();

            int leer=len;

            //printf ("audiosdl_callback. longitud pedida: %d AUDIO_BUFFER_SIZE: %d\n",len,AUDIO_BUFFER_SIZE);
            if (leer>tamanyo_fifo) {
                //debug_printf (VERBOSE_DEBUG,"FIFO is not big enough. Length asked: %d audiosdl_fifo_sdl_return_size: %d",leer,tamanyo_fifo );

                //Si se pide mas audio del que tenemos, dejamos nuestro buffer anterior (llegará a SDL_MixAudio) y no metemos audio nuevo, esto hace que se note menos el corte
                //Esto podria parecer algo sin sentido pero no es asi, cuando sucede esto, es mejor retornar el mismo buffer que teniamos,
                //en vez de hacer un llenado parcial, porque en este caso se notarian los clicks, tanto en Windows como en Linux
            }


            else {

                //printf ("audiosdl_callback. enviando sonido\n");
                if (leer) {
                    audiosdl_fifo_sdl_read(&temporary_audiosdl_fifo_sdl_buffer[indice],leer);
                }

            }
        }

        //Viejo Callback
        else {
            //printf ("audiosdl_callback. longitud pedida: %d AUDIO_BUFFER_SIZE: %d\n",len,AUDIO_BUFFER_SIZE);
            if (len>audiosdl_fifo_sdl_return_size()) {
                //debug_printf (VERBOSE_DEBUG,"FIFO is not big enough. Length asked: %d audiosdl_fifo_sdl_return_size: %d",len,audiosdl_fifo_sdl_return_size() );
                //Volvemos sin hacer SDL_MixAudio

                return ;
            }


            else {
                //printf ("audiosdl_callback. enviando sonido\n");
                audiosdl_fifo_sdl_read(temporary_audiosdl_fifo_sdl_buffer,len);
            }
        }

	}

	SDL_MixAudio(stream, (Uint8 *) temporary_audiosdl_fifo_sdl_buffer, len, SDL_MIX_MAXVOLUME);


}

/*
Notas sobre el nuevo callback:

Este callback soluciona los problemas de "clicks" en windows (y en algunos linux, sobretodo en maquina virtual)

el problema con el sonido en windows tiene su origen en el windows propiamente y también en la manera como estaba enviando yo el sonido
lo resumo de manera rápida:
-en el driver de audio sdl (el que uso en windows, y que también puedes usar en linux) es el propio driver el que llama a una función, que defino de mi código de ZEsarUX, cuando "necesita" enviar mas sonido
o sea, te dice "oye necesito 10ms de sonido, dámelos"
ese sonido yo lo tengo pre-generado en otro sitio, desde el bucle de emulación de la cpu
y aquí pueden suceder dos cosas:
1) que ese buffer que yo tengo pre-generado tenga ya al menos esos 10ms de sonido, por tanto le paso el trozo que me pide. hasta aqui ok
2) que ese buffer que yo tengo pre-generado no tenga aun 10ms de sonido, que haya menos. hasta ahora lo que yo hacia era: no te doy nada de sonido. con lo que el driver estaba enviando silencio (los clicks)
ahora el caso 2) con el nuevo callback hace otra cosa: te envío el mismo sonido que te había enviado antes

por tanto, eso se puede llegar  a apreciar con musica funcionando, no hay clicks pero te puede sonar raro a veces. es verdad que esos fragmentos son muy cortos (menos de 10ms realmente) y cuesta apreciarlos

ese caso 2 lo estaba gestionando mal, pero también es verdad que en linux (en maquina fisica) no me solia suceder (aunque si en maquina virtual)

por qué no sucedia en linux? porque linux gestiona mucho mejor las temporalizaciones que en windows, por tanto, el linux físico cuando me pedia ese trocito de sonido yo ya lo tenia generado, porque todo va mas "a su tiempo justo"

pero windows parece que no, cuando el driver de sonido me pedia sonid, mi bucle de emulacion aún no había llegado a ese punto (por error en las temporalizaciones de windows) y de ahí que se manifestase ese problema en el audio (el underrun se llama)

viendo código de otros emuladores que usan sdl vi varias maneras de gestionar eso

1) retornar solo la cantidad de sonido que llevo generado. eso provoca clicks

2) retornar la cantidad generada +silencio para rellenar: eso provoca clicks

3) retornar lo mismo que habias enviado antes: eso no genera clicks. Asunto resuelto :)
Esto lo vi en el emulador Xpeccy, archivo Xpeccy/src/xcore/sound.cpp
*/


#ifdef MINGW

//Realmente esto no es SDL, es windows, temporalmente lo pongo aqui y ya lo movere...

int audiosdl_can_record_input(void)
{
    return 1;
}





pthread_t thread_windows_capture;


  int windows_avisado_fifo_llena=0;


//stereo y 16 bits
#define WINDOWS_CAPTURE_BUFFER (AUDIO_RECORD_BUFFER_SIZE*2*2)





struct timeval windows_tiempo_antes,windows_tiempo_despues;

long windows_tiempo_difftime;



int audiowindows_record_must_finish=0;

char buffer_audiowindows_captura_temporal[AUDIO_RECORD_BUFFER_SIZE];

int audiowindows_capture_thread_running=0;


 const int NUMPTS = AUDIO_RECORD_BUFFER_SIZE; //44100 *1 ; // 10;   // 10 seconds
 int sampleRate = AUDIO_RECORD_FREQUENCY;
                             // for 8-bit capture, you'd use 'unsigned char' or 'BYTE' 8-bit types

 short int waveIn[AUDIO_RECORD_BUFFER_SIZE];   // 'short int' is a 16-bit type; I request 16-bit samples below
 HWAVEIN      hWaveIn;
 WAVEHDR      WaveInHdr;
 MMRESULT result;

void *audiowindows_capture_thread_function(void *nada)
{

    audiowindows_capture_thread_running=1;

    int err;

	while (!audiowindows_record_must_finish) {

        timer_stats_current_time(&windows_tiempo_antes);

        printf("antes pa_simple_read\n");


 // Commence sampling input
 result = waveInStart(hWaveIn);
 if (result)
 {
  printf("Failed to start recording\n");
  return;
 }


 // Wait until finished recording
 do {} while (waveInUnprepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR))==WAVERR_STILLPLAYING);



        //Esta funcion es bloqueante y se espera a que acabe
        if (0) {
        //if (pa_simple_read (audiowindows_record_s,buffer_audiowindows_captura_temporal,AUDIO_RECORD_BUFFER_SIZE,&err) <0) {


            fprintf (stderr, "read from audio interface failed. err: %d\n",
                err);


                    usleep(1000);
        }

        else {

            //Convertir unsigned en signed
            int i;
            for (i=0;i<AUDIO_RECORD_BUFFER_SIZE;i++) {
                z80_byte valor=(z80_byte) waveIn[i]; //buffer_audiowindows_captura_temporal[i];
                int valor_signo=valor; //-128;
                buffer_audiowindows_captura_temporal[i]=valor_signo;
            }


            if (audiorecord_input_fifo_write(buffer_audiowindows_captura_temporal,AUDIO_RECORD_BUFFER_SIZE) && !windows_avisado_fifo_llena) {
                int miliseconds_lost=(1000*AUDIO_RECORD_BUFFER_SIZE)/AUDIO_RECORD_FREQUENCY;
                debug_printf(VERBOSE_ERR,"External Audio Source buffer is full, a section of %d ms has been lost. "
                    "I recommend you to disable and enable External Audio Source in order to empty the input buffer",
                    miliseconds_lost);
                windows_avisado_fifo_llena=1;
            }


        }


        windows_tiempo_difftime=timer_stats_diference_time(&windows_tiempo_antes,&windows_tiempo_despues);
        //fprintf(stdout, "read  done\n");

        long esperado_microseconds=(1000000L*AUDIO_RECORD_BUFFER_SIZE)/AUDIO_RECORD_FREQUENCY;

        printf("tiempo: %ld esperado: %ld\n",windows_tiempo_difftime,esperado_microseconds);

        //printf("long %d long long %d\n",sizeof(long),sizeof(long long));

        long diferencia_a_final=esperado_microseconds-windows_tiempo_difftime;
        printf("Diferencia %ld microsegundos\n",diferencia_a_final);
        if (diferencia_a_final>0) {
            printf("Falta %ld microsegundos\n",diferencia_a_final);
            //usleep(diferencia_a_final/2);
        }

	}

	//para que no se queje el compilador de variable no usada
	nada=0;
	nada++;

    printf("finished audio windows record\n");


    audiowindows_capture_thread_running=0;


    return NULL;


}


void audiowindows_start_record_input_create_thread(void)
{

    audiowindows_record_must_finish=0;

    if (pthread_create( &thread_windows_capture, NULL, &audiowindows_capture_thread_function, NULL) ) {
        cpu_panic("Can not create audiowindows pthread");
    }

}

pa_buffer_attr audiowindows_record_attributes;

void audiosdl_start_record_input(void)
{
        //Vaciar posible sonido que haya antes del buffer, por si el usuario ha desactivado y activado varias veces
        audiorecord_input_empty_buffer_with_lock();
printf("Start audiowindows record\n");

         // Specify recording parameters
 WAVEFORMATEX pFormat;
 pFormat.wFormatTag=WAVE_FORMAT_PCM;     // simple, uncompressed format
 pFormat.nChannels=1;                    //  1=mono, 2=stereo
 pFormat.nSamplesPerSec=sampleRate;      // 44100
 pFormat.nAvgBytesPerSec=sampleRate*2;   // = nSamplesPerSec * n.Channels * wBitsPerSample/8
 pFormat.nBlockAlign=2;                  // = n.Channels * wBitsPerSample/8
 pFormat.wBitsPerSample=16;              //  16 for high quality, 8 for telephone-grade
 pFormat.cbSize=0;

 result = waveInOpen(&hWaveIn, WAVE_MAPPER,&pFormat,
            0L, 0L, WAVE_FORMAT_DIRECT);
 if (result)
 {
  char fault[256];
  waveInGetErrorText(result, fault, 256);
  printf("Failed to open waveform input device\n");
  return;
 }

 // Set up and prepare header for input
 WaveInHdr.lpData = (LPSTR)waveIn;
 WaveInHdr.dwBufferLength = NUMPTS*2;
 WaveInHdr.dwBytesRecorded=0;
 WaveInHdr.dwUser = 0L;
 WaveInHdr.dwFlags = 0L;
 WaveInHdr.dwLoops = 0L;
 waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

 // Insert a wave input buffer
 result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
 if (result)
 {
  printf("Failed to read block from device\n");
  return;
 }

    audiowindows_start_record_input_create_thread();






    audio_is_recording_input=1;

    printf("Finish initializing audiowindows record\n");

}


void audiosdl_stop_record_input(void)
{

    if (audio_is_recording_input) {
        audiowindows_record_must_finish=1;


        while (audiowindows_capture_thread_running) {
            timer_sleep(100);
        }

        audio_is_recording_input=0;

         waveInClose(hWaveIn);
    }


}



#else



int audiosdl_can_record_input(void)
{
    return 0;
}


void audiosdl_start_record_input(void)
{

    //Nada

}


void audiosdl_stop_record_input(void)
{

    //Nada

}

#endif