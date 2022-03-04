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


#include <alsa/asoundlib.h>


#include "audioalsa.h"
#include "cpu.h"
#include "audio.h"
#include "compileoptions.h"
#include "debug.h"
#include "settings.h"




#ifdef USE_PTHREADS
#include <pthread.h>

     pthread_t thread1_alsa;


#endif



void *audioalsa_enviar_audio(void *nada);
void audioalsa_send_frame(char *buffer);


//buffer temporal de envio. suficiente para que quepa
char buf_enviar[AUDIO_BUFFER_SIZE*10*2]; //*2 porque es estereo


void audioalsa_callback(snd_async_handler_t *pcm_callback);

/* Handle for the PCM device */
snd_pcm_t *pcm_handle;

int fifo_alsa_write_position=0;
int fifo_alsa_read_position=0;

void audioalsa_empty_buffer(void)
{
  debug_printf(VERBOSE_DEBUG,"Emptying audio buffer");
  fifo_alsa_write_position=0;
}

//nuestra FIFO_ALSA
#define MAX_FIFO_ALSA_BUFFER_SIZE (AUDIO_BUFFER_SIZE*30)


//Desde 4 hasta 10
int fifo_alsa_buffer_size=AUDIO_BUFFER_SIZE*4;

//*2 o *4
int alsa_periodsize=AUDIO_BUFFER_SIZE*2;


char fifo_alsa_buffer[MAX_FIFO_ALSA_BUFFER_SIZE*2]; //*2 porque es estereo


int audioalsa_return_fifo_buffer_size(void)
{
  return fifo_alsa_buffer_size*2; //*2 porque es stereo
}

//retorna numero de elementos en la fifo_alsa
int fifo_alsa_return_size(void)
{
        //si write es mayor o igual (caso normal)
        if (fifo_alsa_write_position>=fifo_alsa_read_position) {

		//printf ("write es mayor o igual: write: %d read: %d\n",fifo_alsa_write_position,fifo_alsa_read_position);
		return fifo_alsa_write_position-fifo_alsa_read_position;
	}

        else {
                //write es menor, cosa que quiere decir que hemos dado la vuelta
                return (audioalsa_return_fifo_buffer_size()-fifo_alsa_read_position)+fifo_alsa_write_position;
        }
}

void audioalsa_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=audioalsa_return_fifo_buffer_size();
  *current_size=fifo_alsa_return_size();
}

//retornar siguiente valor para indice. normalmente +1 a no ser que se de la vuelta
int fifo_alsa_next_index(int v)
{
        v=v+1;
        if (v==audioalsa_return_fifo_buffer_size()) v=0;

        return v;
}

//escribir datos en la fifo_alsa
void fifo_alsa_write(char *origen,int longitud)
{
        for (;longitud>0;longitud--) {

                //ver si la escritura alcanza la lectura. en ese caso, error
                if (fifo_alsa_next_index(fifo_alsa_write_position)==fifo_alsa_read_position) {
                        debug_printf (VERBOSE_DEBUG,"FIFO_ALSA full");

                        //Si se llena fifo, resetearla a 0 para corregir latencia
                        if (audio_noreset_audiobuffer_full.v==0) audioalsa_empty_buffer();

                        return;
                }

		//Canal izquierdo
                fifo_alsa_buffer[fifo_alsa_write_position]=*origen++;
                fifo_alsa_write_position=fifo_alsa_next_index(fifo_alsa_write_position);

		//Canal derecho
                fifo_alsa_buffer[fifo_alsa_write_position]=*origen++;
                fifo_alsa_write_position=fifo_alsa_next_index(fifo_alsa_write_position);

	}
}


//leer datos de la fifo_alsa
void fifo_alsa_read(char *destino,int longitud)
{
        for (;longitud>0;longitud--) {

		if (fifo_alsa_return_size()==0) {
                        debug_printf (VERBOSE_DEBUG,"FIFO_ALSA vacia");
                        return;
                }


                //ver si la lectura alcanza la escritura. en ese caso, error
                //if (fifo_alsa_next_index(fifo_alsa_read_position)==fifo_alsa_write_position) {
                //        debug_printf (VERBOSE_INFO,"FIFO_ALSA vacia");
                //        return;
                //}

                *destino++=fifo_alsa_buffer[fifo_alsa_read_position];
                fifo_alsa_read_position=fifo_alsa_next_index(fifo_alsa_read_position);
        }
}


//int ptr_audioalsa;


    /* Playback stream */
    snd_pcm_stream_t stream;

    /* This structure contains information about    */
    /* the hardware and can be used to specify the  */
    /* configuration to be used for the PCM stream. */
    snd_pcm_hw_params_t *hwparams;

    /* Name of the PCM device, like plughw:0,0          */
    /* The first number is the number of the soundcard, */
    /* the second number is the number of the device.   */
    char *pcm_name;

//unsigned int requested, ioctl_format, ioctl_channels, ioctl_rate;

    snd_pcm_uframes_t periodsize ;



int audioalsa_init(void)
{

	//audio_driver_accepts_stereo.v=1;

#ifdef USE_PTHREADS
	debug_printf (VERBOSE_INFO,"Init Alsa Audio Driver - using pthreads. Using alsaperiodsize=%d bytes, fifoalsabuffersize=%d bytes, MAX_FIFO_ALSA_BUFFER_SIZE=%d bytes, %d Hz",alsa_periodsize,fifo_alsa_buffer_size,MAX_FIFO_ALSA_BUFFER_SIZE,FRECUENCIA_SONIDO);
#else
	debug_printf (VERBOSE_INFO,"Init Alsa Audio Driver - not using pthreads. Using alsaperiodsize=%d bytes, %d Hz",AUDIO_BUFFER_SIZE*2, FRECUENCIA_SONIDO);
#endif


	 stream = SND_PCM_STREAM_PLAYBACK;
	    /* Init pcm_name. Of course, later you */
    /* will make this configurable ;-)     */
    pcm_name = strdup("plughw:0,0");


    /* Allocate the snd_pcm_hw_params_t structure on the stack. */
    snd_pcm_hw_params_alloca(&hwparams);


    /* Open PCM. The last parameter of this function is the mode. */
    /* If this is set to 0, the standard mode is used. Possible   */
    /* other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.       */
    /* If SND_PCM_NONBLOCK is used, read / write access to the    */
    /* PCM device will return immediately. If SND_PCM_ASYNC is    */
    /* specified, SIGIO will be emitted whenever a period has     */
    /* been completely processed by the soundcard.                */

    if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0) {
      debug_printf(VERBOSE_ERR, "Error opening PCM device %s", pcm_name);
      return 1;
    }

/* Init hwparams with full configuration space */
    if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
      debug_printf(VERBOSE_ERR, "Can not configure this PCM device.");
      snd_pcm_close( pcm_handle );
      return 1;
    }


    unsigned int rate = FRECUENCIA_SONIDO; /* Sample rate */
    //int rate = 44100; /* Sample rate */
    unsigned int exact_rate;   /* Sample rate returned by */
                      /* snd_pcm_hw_params_set_rate_near */
    int dir;          /* exact_rate == rate --> dir = 0 */
                      /* exact_rate < rate  --> dir = -1 */
                      /* exact_rate > rate  --> dir = 1 */
    int periods = 2;       /* Number of periods */
    periodsize = AUDIO_BUFFER_SIZE*2; /* Periodsize (bytes) */
    //periodsize = 8192; /* Periodsize (bytes) */
    //periodsize = 8192;

	//valor normal, el de siempre, con rutinas vieja_
	//periodsize = AUDIO_BUFFER_SIZE*2;


	//pruebas con rutinas new_

//estos valores los he deducido probando, para que no se corte sonido en ninguna de las dos posibilidades
#ifdef USE_PTHREADS
	periodsize = alsa_periodsize;
#else
	periodsize = AUDIO_BUFFER_SIZE*2;
#endif

	//periodsize *=2; //*2 porque es stereo


/* Set access type. This can be either    */
    /* SND_PCM_ACCESS_RW_INTERLEAVED or       */
    /* SND_PCM_ACCESS_RW_NONINTERLEAVED.      */
    /* There are also access types for MMAPed */
    /* access, but this is beyond the scope   */
    /* of this introduction.                  */
    if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
      debug_printf(VERBOSE_ERR, "Error setting access.");
      snd_pcm_close( pcm_handle );
      return 1;
    }

    /* Set sample format */
    if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S8) < 0) {
      debug_printf(VERBOSE_ERR, "Error setting format.");
      snd_pcm_close( pcm_handle );
      return 1;
    }

    /* Set sample rate. If the exact rate is not supported */
    /* by the hardware, use nearest possible rate.         */
    exact_rate = rate;
    if (snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &exact_rate, 0) < 0) {
      debug_printf(VERBOSE_ERR, "Error setting rate.");
      snd_pcm_close( pcm_handle );
      return 1;
    }
    if (rate != exact_rate) {
      debug_printf(VERBOSE_ERR, "The rate %d Hz is not supported by your hardware. ==> Using %d Hz instead.", rate, exact_rate);
    }

    /* Set number of channels */
    if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2) < 0) {
      debug_printf(VERBOSE_ERR, "Error setting channels.");
      snd_pcm_close( pcm_handle );
      return 1;
    }

    /* Set number of periods. Periods used to be called fragments. */
    if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0) < 0) {
      debug_printf(VERBOSE_ERR, "Error setting periods.");
      snd_pcm_close( pcm_handle );
      return 1;
    }


 snd_pcm_uframes_t frames;
  int err;

if ((err = snd_pcm_hw_params_get_period_size_min(hwparams, &frames, &dir)) < 0) {
    debug_printf(VERBOSE_ERR, "cannot  get min period size  (%s)",
	     snd_strerror (err));
      snd_pcm_close( pcm_handle );
    return 1;
  }
  debug_printf(VERBOSE_DEBUG,"min period size in frames %d", frames);

  if ((err = snd_pcm_hw_params_get_period_size_max(hwparams, &frames, &dir)) < 0) {
    debug_printf(VERBOSE_ERR,"cannot  get max period size (%s)",
	     snd_strerror (err));
      snd_pcm_close( pcm_handle );
    return 1;
  }
  debug_printf(VERBOSE_DEBUG,"max period size in frames %d", frames);


snd_pcm_uframes_t buffer_size_min;

  if ((err = snd_pcm_hw_params_get_buffer_size_min(hwparams, &buffer_size_min)) < 0) {
    debug_printf(VERBOSE_ERR,"cannot  get min buffer size (%s)",
             snd_strerror (err));
      snd_pcm_close( pcm_handle );
    return 1;
  }
  debug_printf(VERBOSE_DEBUG,"min buffer size %d", buffer_size_min);

snd_pcm_uframes_t buffer_size_max;

  if ((err = snd_pcm_hw_params_get_buffer_size_max(hwparams, &buffer_size_max)) < 0) {
    debug_printf(VERBOSE_ERR,"cannot  get max buffer size (%s)",
             snd_strerror (err));
      snd_pcm_close( pcm_handle );
    return 1;
  }
  debug_printf(VERBOSE_DEBUG,"max buffer size %d", buffer_size_max);




    /* Set buffer size (in frames). The resulting latency is given by */
    /* latency = periodsize * periods / (rate * bytes_per_frame)     */

        unsigned int bufsize=(periodsize * periods)>>2;
        debug_printf(VERBOSE_DEBUG,"Intended buffer size %d",bufsize);



	//Si el bufsize que pretendemos es muy pequenyo, lo cambiamos
	//Esto solo sucede, de momento, en raspberry
	//En el resto de pc-linux no sucede
        if (bufsize<buffer_size_min) {
		//con esto dara buffer underrun
		//bufsize=buffer_size_min;

		bufsize=buffer_size_min*2;

		//con esto en raspberry necesita un buffer de audio de 256 kb
        	//bufsize=buffer_size_max;

		//recalcular periodsize. teniamos que:
		//unsigned int bufsize=(periodsize * periods)>>2;
		periodsize=(bufsize*4)/periods;

		//recalcular fifo_alsa_buffer_size
		fifo_alsa_buffer_size=periodsize*2;


		if (fifo_alsa_buffer_size>MAX_FIFO_ALSA_BUFFER_SIZE) {
			debug_printf (VERBOSE_ERR,"Resulting fifo_alsa_buffer_size: %d exceeds maximum: %d",fifo_alsa_buffer_size,MAX_FIFO_ALSA_BUFFER_SIZE);
			return 1;
		}

		//esta alsa_periodsize no se vuelve a usar, solo la reasigno para que quede mas claro el proceso
		alsa_periodsize=periodsize;

		debug_printf (VERBOSE_INFO,"Reasign alsaperiodsize=%d bytes, fifoalsabuffersize=%d bytes",alsa_periodsize,fifo_alsa_buffer_size);

	}


        debug_printf(VERBOSE_INFO,"Trying buffer size %d",bufsize);



    /* Set buffer size (in frames). The resulting latency is given by */
    /* latency = periodsize * periods / (rate * bytes_per_frame)     */
    //if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, (periodsize * periods)>>2) < 0) {
    if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, bufsize) < 0) {
      debug_printf(VERBOSE_ERR, "Error setting buffersize.");
      snd_pcm_close( pcm_handle );
      return 1;
    }


    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
      debug_printf(VERBOSE_ERR, "Error setting HW params.");
      snd_pcm_close( pcm_handle );
      return 1;
    }

	//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
  audio_set_driver_name("alsa");



		return 0;

}


int audioalsa_thread_finish(void)
{

#ifdef USE_PTHREADS
        if (thread1_alsa!=0) {
                debug_printf (VERBOSE_DEBUG,"Ending audioalsa thread");
                int s=pthread_cancel(thread1_alsa);
		if (s != 0) debug_printf (VERBOSE_DEBUG,"Error cancelling pthread alsa");

                thread1_alsa=0;
        }

	//Pausa de 0.1 segundo
	usleep(100000);

#endif

return 0;

}

void audioalsa_end(void)
{
        debug_printf (VERBOSE_INFO,"Ending alsa audio driver");
        audioalsa_thread_finish();
	audio_playing.v=0;
	snd_pcm_close( pcm_handle );

}





void new_audioalsa_enviar_audio_envio(void)
{

	int ret;

        int frames = periodsize >> 2;
        frames=frames*2;

	int len;

		len=frames;

		//len *=2; //porque es stereo


		if (fifo_alsa_return_size()>=len) {

			//Si hay detectado silencio, la fifo estara vacia y por tanto ya no entrara aqui y no enviara sonido

			//printf ("temp envio sonido\n");

			//manera normal usando funciones de fifo
			fifo_alsa_read(buf_enviar,len); 
			ret = snd_pcm_writei( pcm_handle, buf_enviar, len );

			//Siguiente fragmento de audio. Es mejor hacerlo aqui que no esperar
			//Esto da sonido correcto. Porque? No estoy seguro del todo...

			if (ret==len) {
				fifo_alsa_read(buf_enviar,len); 
				//printf ("enviar audio alsa len: %d\n",len);
				ret = snd_pcm_writei( pcm_handle, buf_enviar, len );
			}


			//tamanyo despues
			//printf ("enviar. despues. tamanyo fifo: %d read %d write %d\n",fifo_alsa_return_size(),fifo_alsa_read_position,fifo_alsa_write_position);

			if (ret!=len ) {
                    		if( ret < 0 ) {
					//printf ("ret<0 ret=%d\n",ret);
		                      snd_pcm_prepare( pcm_handle );
                		      debug_printf(VERBOSE_DEBUG, "Alsa Buffer Underrun");

				}
			}

			else {

                	//no esperamos , enviamos siguiente sonido avisando de interrupcion a cpu
	                interrupt_finish_sound.v=1;

			}

		}

		else {
#ifdef EMULATE_RASPBERRY
			usleep(1000/16);
#else
			usleep(1000);
#endif


			//printf ("temp en usleep de envio audio\n");

		}
}


#ifdef USE_PTHREADS

char *buffer_playback_alsa;
//int pthread_enviar_sonido_alsa=0;
int frames_sonido_enviados_alsa=0;

void *new_audioalsa_enviar_audio(void *nada)
{


	while (1) {

			new_audioalsa_enviar_audio_envio();

	}

	//para que no se queje el compilador de variable no usada
	nada=0;
	nada++;


}



pthread_t thread1_alsa=0;

void new_audioalsa_send_frame(char *buffer)
{

        //pthread_enviar_sonido_alsa=1;
        if (audio_playing.v==0) {
                //Volvemos a activar pthread
                buffer_playback_alsa=buffer;
                audio_playing.v=1;
        }

        if (thread1_alsa==0) {
                buffer_playback_alsa=buffer;

                if (pthread_create( &thread1_alsa, NULL, &audioalsa_enviar_audio, NULL) ) {
                        cpu_panic("Can not create audioalsa pthread");
                }
        }

                        //tamanyo antes
                        //printf ("write. antes. tamanyo fifo: %d read %d write %d\n",fifo_alsa_return_size(),fifo_alsa_read_position,fifo_alsa_write_position);

	fifo_alsa_write(buffer,AUDIO_BUFFER_SIZE);
                        //tamanyo despues
                        //printf ("write. despues. tamanyo fifo: %d read %d write %d\n",fifo_alsa_return_size(),fifo_alsa_read_position,fifo_alsa_write_position);
}


#else


//sin pthreads. Esta funcion es igual que la vieja. No usa fifo

void new_audioalsa_send_frame(char *buffer)
{

	char *buffer_playback_alsa;

	buffer_playback_alsa=buffer;


	//Enviar sonido

	int frames = periodsize >> 2;
	frames=frames*2;


	//After the PCM device is configured, we can start writing PCM data to it. The first write access will start the PCM playback. For interleaved write access, we use the function

    /* Write num_frames frames from buffer data to    */
    /* the PCM device pointed to by pcm_handle.       */
    /* Returns the number of frames actually written. */
	int len=frames;
	int ret;

	//len *=2; //porque es stereo

	//printf ("temp envio sonido\n");

	while( ( ret = snd_pcm_writei( pcm_handle, buffer_playback_alsa, len ) ) != len ) {
	    if( ret < 0 ) {
	      snd_pcm_prepare( pcm_handle );
	      debug_printf(VERBOSE_DEBUG, "Alsa Buffer Underrun");
	    } else {
	        len -= ret;
	    }
	  }


//Fin Enviar sonido

}



//sin pthreads




#endif




//Funciones de envio . Nuevas. Con fifo


#ifdef USE_PTHREADS
void *audioalsa_enviar_audio(void *nada)
{
        return new_audioalsa_enviar_audio(nada);
}
#endif

void audioalsa_send_frame(char *buffer)
{
        return new_audioalsa_send_frame(buffer);
}
