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

#include <string.h>
#include <stdio.h>
#if defined(__APPLE__)
        #include <SDL.h>
#else
	#include <SDL/SDL.h>
#endif



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


//Callback para windows
//Tecnicamente este deberia ser para todos pero esta visto que en Linux produce clicks


#ifdef MINGW

void audiosdl_callback(void *udata, Uint8 *stream, int total_len)
{

    int len=total_len;

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

        int indice=0;

        int warned_fifo=0;

        //Para dar un limite en espera del bucle while, por si acaso no se quede dentro siempre
        int max_wait=1000;

        //Esperamos a llenar todo el sonido que nos solicitan
        //Puede suceder que el callback nos pida mas sonido del que tenemos; como el buffer lo llenamos desde otro thread,
        //al final acabara llenandose
        //Esto corrige los clicks famosos que se escuchaban en Windows
        while (len>0 && audio_playing.v && max_wait>0) {

            int tamanyo_fifo=audiosdl_fifo_sdl_return_size();

            int leer=len;

		    //printf ("audiosdl_callback. longitud pedida: %d AUDIO_BUFFER_SIZE: %d\n",len,AUDIO_BUFFER_SIZE);
	    	if (leer>tamanyo_fifo) {
                //Aviso solo la primera vez
                if (!warned_fifo) {
                    debug_printf (VERBOSE_DEBUG,"FIFO is not big enough. Length asked: %d audiosdl_fifo_sdl_return_size: %d",leer,tamanyo_fifo );
                    warned_fifo=1;
                }

                //le damos un tiempo para ver si llena la fifo
                timer_sleep(1);


                leer=tamanyo_fifo;
            }
            else {
                //printf("FIFO OK\n");
            }
	
		
			//printf ("audiosdl_callback. enviando sonido\n");
            if (leer) {
			    audiosdl_fifo_sdl_read(&temporary_audiosdl_fifo_sdl_buffer[indice],leer);
            }
		

            indice+=leer;
            len -=leer;

            max_wait--;
        }

	}

	SDL_MixAudio(stream, (Uint8 *) temporary_audiosdl_fifo_sdl_buffer, total_len, SDL_MIX_MAXVOLUME);


}


#else

//Callback para el resto
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

		//printf ("audiosdl_callback. longitud pedida: %d AUDIO_BUFFER_SIZE: %d\n",len,AUDIO_BUFFER_SIZE);
		if (len>audiosdl_fifo_sdl_return_size()) {
			//debug_printf (VERBOSE_DEBUG,"FIFO is not big enough. Length asked: %d audiosdl_fifo_sdl_return_size: %d",len,audiosdl_fifo_sdl_return_size() );
			//esto puede pasar con el detector de silencio

			//retornar solo lo que tenemos
			//audiosdl_fifo_sdl_read(out,audiosdl_fifo_sdl_return_size() );

			return ;
		}


		else {
			//printf ("audiosdl_callback. enviando sonido\n");
			audiosdl_fifo_sdl_read(temporary_audiosdl_fifo_sdl_buffer,len);
		}

	}

	SDL_MixAudio(stream, (Uint8 *) temporary_audiosdl_fifo_sdl_buffer, len, SDL_MIX_MAXVOLUME);


}

#endif
