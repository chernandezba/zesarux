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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//#include <time.h>
#include <sys/time.h>



#include <sys/soundcard.h>


#include "audiodsp.h"
#include "cpu.h"
#include "audio.h"
#include "compileoptions.h"
#include "debug.h"
#include "settings.h"

#ifdef USE_PTHREADS
#include <pthread.h>

     pthread_t thread1;

#endif

int ptr_audiodsp;

unsigned int requested, ioctl_format, ioctl_channels, ioctl_rate;

#define BASE_SOUND_FRAG_PWR     6

int audiodsp_init(void)
{

	//audio_driver_accepts_stereo.v=1;


#ifdef USE_PTHREADS
        debug_printf (VERBOSE_INFO,"Init DSP Audio Driver - using phtreads, %d Hz",FRECUENCIA_SONIDO);
#else
        debug_printf (VERBOSE_INFO,"Init DSP Audio Driver - not using phtreads, %d Hz",FRECUENCIA_SONIDO);
#endif



#ifdef USE_PTHREADS
                ptr_audiodsp=open("/dev/dsp",O_WRONLY);
#else
		//Hacer que no bloquee la operacion fread
                ptr_audiodsp=open("/dev/dsp",O_WRONLY|O_NONBLOCK);
#endif
                if (ptr_audiodsp==-1)
                {
                        debug_printf(VERBOSE_INFO,"Unable to open /dev/dsp : %s ",strerror(errno));
			return 1;
                }


#ifdef USE_PTHREADS
#else

int flags;
if((flags=fcntl(ptr_audiodsp,F_GETFL))==-1)
  {
  debug_printf(VERBOSE_ERR,"couldn't get flags from sound device: %s",strerror(errno));
  return 1;
  }
flags &= ~O_NONBLOCK;
if(fcntl(ptr_audiodsp,F_SETFL,flags)==-1)
  {
  debug_printf(VERBOSE_ERR,"couldn't set sound device non-blocking: %s",strerror(errno));
  return 1;
  }

#endif

		ioctl_format=AFMT_S8;
		ioctl_channels=2; //2 porque es stereo
		//312 lineas(AUDIO_BUFFER_SIZE) por 50 Hz
		ioctl_rate=FRECUENCIA_SONIDO;


		if (ioctl(ptr_audiodsp,SNDCTL_DSP_SETFMT,&ioctl_format) == -1) {		
			debug_printf(VERBOSE_ERR,"Format selection failed : %s ",strerror(errno));
			return 1;
		}

		if (ioctl(ptr_audiodsp,SNDCTL_DSP_CHANNELS,&ioctl_channels) == -1) {		
			debug_printf(VERBOSE_ERR,"Channels selection failed : %s ",strerror(errno));
			return 1;
		}

		if (ioctl(ptr_audiodsp,SNDCTL_DSP_SPEED,&ioctl_rate) == -1) {		
			debug_printf(VERBOSE_ERR,"Rate selection failed : %s ",strerror(errno));
			return 1;
		}


		//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
		audio_set_driver_name("dsp");

		return 0;

}



int audiodsp_thread_finish(void)
{

#ifdef USE_PTHREADS
	if (thread1!=0) {
        	debug_printf (VERBOSE_DEBUG,"Ending audiodsp thread");
		int s=pthread_cancel(thread1);
		if (s != 0) debug_printf (VERBOSE_DEBUG,"Error cancelling pthread dsp");

		thread1=0;
	}

	//Pausa de 0.1 segundo
	 usleep(100000);
	
#endif

return 0;

}

void audiodsp_end(void)
{
	debug_printf (VERBOSE_INFO,"Ending dsp audio driver");
	audiodsp_thread_finish();
	audio_playing.v=0;

	close(ptr_audiodsp);
}


#ifdef USE_PTHREADS


struct timeval audiodsp_pthread_timer_antes, audiodsp_pthread_timer_ahora;



//Para calcular tiempos funciones. Iniciar contador antes
void audiodsp_pthread_tiempo_inicial(void)
{

        gettimeofday(&audiodsp_pthread_timer_antes, NULL);

}

//Para calcular tiempos funciones. Contar contador despues e imprimir tiempo por pantalla
int audiodsp_pthread_tiempo_final(void)
{

        long audiodsp_pthread_timer_mtime, audiodsp_pthread_timer_seconds, audiodsp_pthread_timer_useconds;    

        gettimeofday(&audiodsp_pthread_timer_ahora, NULL);

        audiodsp_pthread_timer_seconds  = audiodsp_pthread_timer_ahora.tv_sec  - audiodsp_pthread_timer_antes.tv_sec;
        audiodsp_pthread_timer_useconds = audiodsp_pthread_timer_ahora.tv_usec - audiodsp_pthread_timer_antes.tv_usec;

        audiodsp_pthread_timer_mtime = ((audiodsp_pthread_timer_seconds) * 1000 + audiodsp_pthread_timer_useconds/1000.0) + 0.5;

        //printf("Elapsed time: %ld milliseconds\n\r", audiodsp_pthread_timer_mtime);

	return audiodsp_pthread_timer_mtime;
}




char *buffer_playback_dsp;
//int pthread_enviar_sonido_dsp=0;
int frames_sonido_enviados_dsp=0;

void *audiodsp_enviar_audio(void *nada)
{
	long tiempo_pasado;
	//printf ("Antes escrito buffer=%p\n",buffer_playback_dsp);

	while (1) {

		//printf ("audio playing: %d\n",audio_playing.v);


		//Parece que las funciones de dsp de escritura a veces retornan sin esperar a escribir todo...
		//esperamos a que pase el tiempo que dura un frame de sonido
		do {
			tiempo_pasado=audiodsp_pthread_tiempo_final();
			//printf ("Tiempo pasado: %ld\n",tiempo_pasado);

                        //1 ms
                        usleep(1000);

		} while (tiempo_pasado<AUDIO_MS_DURACION);

		//Establecer el buffer de reproduccion
		buffer_playback_dsp=audio_buffer_playback;

                //enviamos siguiente sonido avisando de interrupcion a cpu
                interrupt_finish_sound.v=1;

		//printf ("Tiempo pasado: %ld Tiempo minimo: %d\n",tiempo_pasado,AUDIO_MS_DURACION);

		//pthread_enviar_sonido_dsp=0;

		audiodsp_pthread_tiempo_inicial();
		int len=AUDIO_BUFFER_SIZE;

		len *=2; //*2 porque es stereo

		int ret;
		int ofs=0;
		while(len)
		{


			ret=write(ptr_audiodsp,buffer_playback_dsp+ofs,len);
			//printf ("Escritos: %d \n",ret);
			if(ret>0)
				ofs+=ret,len-=ret;
		}

                //printf ("Despues escrito\n");


                //conmutamos nuestro playback buffer

/*
                if (buffer_playback_dsp==audio_buffer_one) buffer_playback_dsp=audio_buffer_two;
                else buffer_playback_dsp=audio_buffer_one;
*/

                //debug_printf(VERBOSE_DEBUG,"Frames sonido enviados: %d\n",++frames_sonido_enviados_dsp);

		//Si esta el sonido parado o hay deteccion de silencio
		//en otros drivers no detectamos silencio directamente,
		//sino que detectan si la fifo tiene datos o no. cuando hay silencio, la fifo se vacia,
		//y por tanto no tiene datos. En este driver dsp no trabajamos con fifo,
		//por eso miramos aqui si esta en silencio o no
                while (audio_playing.v==0 || silence_detection_counter==SILENCE_DETECTION_MAX) {
                        //1 ms
                        usleep(1000);
			//printf ("temp en usleep de envio audio\n");
                }

	}


	//Para evitar warnings al compilar de unused parameter ‘nada’ [-Wunused-parameter]
	nada=0;
	nada++;


	return NULL;
} 

pthread_t thread1=0;

void audiodsp_send_frame(char *buffer)
{


	//pthread_enviar_sonido_dsp=1;
        if (audio_playing.v==0) {
                //Volvemos a activar pthread
                buffer_playback_dsp=buffer;
                audio_playing.v=1;
        }


	if (thread1==0) {
	buffer_playback_dsp=buffer;
	audiodsp_pthread_tiempo_inicial();
     if (pthread_create( &thread1, NULL, &audiodsp_enviar_audio, NULL) ) {
                cpu_panic("Can not create audiodsp pthread");
        }      
	}


	
}
#else 
void audiodsp_send_frame(char *buffer)
{
		//printf ("temp envio sonido\n");

                write(ptr_audiodsp,buffer,AUDIO_BUFFER_SIZE*2); //*2 porque es stereo


}
#endif




void audiodsp_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=AUDIO_BUFFER_SIZE*2; //*2 porque es stereo
  *current_size=0;
}

