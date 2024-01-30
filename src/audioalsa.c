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
#include "ay38912.h"
#include "utils.h"

//temp
#include "timer.h"


#ifdef USE_PTHREADS
#include <pthread.h>

     pthread_t thread1_alsa;


#endif



void *audioalsa_enviar_audio(void *nada);
void audioalsa_send_frame(char *buffer);


//buffer temporal de envio. suficiente para que quepa
char buf_enviar[AUDIO_BUFFER_SIZE*10];


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


char fifo_alsa_buffer[MAX_FIFO_ALSA_BUFFER_SIZE];



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
                return (fifo_alsa_buffer_size-fifo_alsa_read_position)+fifo_alsa_write_position;
        }
}

void audioalsa_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=fifo_alsa_buffer_size;
  *current_size=fifo_alsa_return_size();
}

//retornar siguiente valor para indice. normalmente +1 a no ser que se de la vuelta
int fifo_alsa_next_index(int v)
{
        v=v+1;
        if (v==fifo_alsa_buffer_size) v=0;

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
	debug_printf (VERBOSE_INFO,"Init Alsa Audio Driver (mono) - using pthreads. Using alsaperiodsize=%d bytes, fifoalsabuffersize=%d bytes, MAX_FIFO_ALSA_BUFFER_SIZE=%d bytes, %d Hz",alsa_periodsize,fifo_alsa_buffer_size,MAX_FIFO_ALSA_BUFFER_SIZE,FRECUENCIA_SONIDO);
#else
	debug_printf (VERBOSE_INFO,"Init Alsa Audio Driver (mono) - not using pthreads. Using alsaperiodsize=%d bytes, %d Hz",AUDIO_BUFFER_SIZE*2, FRECUENCIA_SONIDO);
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
    //if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE) < 0) {
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
    if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 1) < 0) {
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

	//temp cambio
	//bufsize=0;


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


	//temp error
	//printf ("temp error alsa\n");
	//return 1;



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

			//tamanyo antes
			//printf ("enviar. antes. tamanyo fifo: %d read %d write %d\n",fifo_alsa_return_size(),fifo_alsa_read_position,fifo_alsa_write_position);
		//if (fifo_alsa_return_size()>=AUDIO_BUFFER_SIZE) {
			new_audioalsa_enviar_audio_envio();


		//No enviar sonido si audio no activo
                /*while (audio_playing.v==0) {
                        //1 ms
                        usleep(1000);
                }
		*/


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

char audioalsa_buffer_mono[AUDIO_BUFFER_SIZE];

void audioalsa_convert_mono(char *origen)
{
	int valor_sonido_int;
	char canal_izquierdo, canal_derecho;
	char canal_mezclado;

	char *destino;
	destino=audioalsa_buffer_mono;

	int i;

	for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
		canal_izquierdo=*origen;
		origen++;

		canal_derecho=*origen;
		origen++;

		valor_sonido_int=canal_izquierdo+canal_derecho;
		valor_sonido_int/=2;

		canal_mezclado=valor_sonido_int;

		*destino=canal_mezclado;
		destino++;
	}
}


void audioalsa_send_frame(char *buffer)
{
	//Convertimos a buffer mono
        audioalsa_convert_mono(buffer);
	return new_audioalsa_send_frame(audioalsa_buffer_mono);

	//return new_audioalsa_send_frame(buffer);
}



//Rutinas midi

//Para ver dispositivos midi:
//cat /proc/asound/seq/clients

//Para iniciar timidity en modo secuenciador midi externo:
//timidity -iA -Os -B2,8 -EFreverb=0

#define ZESARUX_MID_PPQ 96


//Guarda el estado actual del midi
struct s_zesarux_mid_alsa_audio_info {

	/* Parametros utilitzados en la libreria ALSA  */
	int port;
	snd_seq_t *handle;
	int cua;
	int client;

	//Cliente midi
	int midi_client,midi_port;

	//utilizado en la subscripcion y demas...
	snd_seq_addr_t sender, dest;
	snd_seq_port_subscribe_t *subs;




};

//struct s_zesarux_mid_alsa_audio_info zesarux_mid_alsa_audio_info;


//Guarda el estado actual del midi
struct s_zesarux_mid_alsa_audio_info zesarux_mid_alsa_audio_info;





//Crear un cliente alsa sequencer
snd_seq_t *alsa_mid_audio_open_client()
{
	int err;
	err = snd_seq_open(&zesarux_mid_alsa_audio_info.handle, "default", SND_SEQ_OPEN_OUTPUT, 0);
	if (err < 0)
		return NULL;
	snd_seq_set_client_name(zesarux_mid_alsa_audio_info.handle, "ZEsarUX MIDI Client");

	zesarux_mid_alsa_audio_info.client=snd_seq_client_id(zesarux_mid_alsa_audio_info.handle);

	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_audio_open_client: Client: %d",zesarux_mid_alsa_audio_info.client);

	return zesarux_mid_alsa_audio_info.handle;
}

// crear un nuevo port sequencer. Retorna el port id
int alsa_mid_audio_new_port
(snd_seq_t *handle)
{
	return snd_seq_create_simple_port(handle, "Midi Out port",
                        				SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
												SND_SEQ_PORT_TYPE_APPLICATION);
}

//Inicializar el tempo de la cola
void alsa_mid_alsa_mid_audio_set_tempo_init
(void)
{
	snd_seq_queue_tempo_t *tempo;


	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_alsa_mid_audio_set_tempo_init");


	snd_seq_queue_tempo_alloca(&tempo);
	snd_seq_queue_tempo_set_tempo(tempo, 1000000); // 60 BPM
	snd_seq_queue_tempo_set_ppq(tempo, ZESARUX_MID_PPQ); // 48 PPQ
	//els ppq, con esta funcion, solo se pueden seleccionar antes de activar la cola

	snd_seq_set_queue_tempo(zesarux_mid_alsa_audio_info.handle, zesarux_mid_alsa_audio_info.cua, tempo);
}

//Cambio del tempo de la cola
void alsa_mid_audio_set_tempo
(void)
{
//TEMPO=(60*1000000)/BPM
//(BPM/60)*PPQ=ticks/temps

	unsigned int tempo;
	//int p;
	unsigned int bpm;


	//forzado
	bpm=120;

	tempo=(60*1000000)/bpm;


	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_audio_set_tempo: Setting tempo=%u",tempo);


	snd_seq_change_queue_tempo(zesarux_mid_alsa_audio_info.handle,zesarux_mid_alsa_audio_info.cua,tempo,NULL);
}



//Crear una nueva cola sequencer y retornar el id
int alsa_mid_audio_new_queue
(snd_seq_t *handle)
{
	return snd_seq_alloc_named_queue(handle, "ZEsarUX alsa queue");
}

//Se suscribe al puerto midi indicado. Retorna <0 en caso de error
int alsa_mid_subscribe_midi_port(int midi_client, int midi_port)
{

	int err;

	zesarux_mid_alsa_audio_info.dest.client = midi_client;
	zesarux_mid_alsa_audio_info.dest.port = midi_port;
	zesarux_mid_alsa_audio_info.sender.client = zesarux_mid_alsa_audio_info.client;
	zesarux_mid_alsa_audio_info.sender.port = zesarux_mid_alsa_audio_info.port;

	snd_seq_port_subscribe_alloca(&zesarux_mid_alsa_audio_info.subs);
	snd_seq_port_subscribe_set_sender(zesarux_mid_alsa_audio_info.subs, &zesarux_mid_alsa_audio_info.sender);
	snd_seq_port_subscribe_set_dest(zesarux_mid_alsa_audio_info.subs, &zesarux_mid_alsa_audio_info.dest);

	snd_seq_port_subscribe_set_time_update(zesarux_mid_alsa_audio_info.subs, 1);
	snd_seq_port_subscribe_set_time_real(zesarux_mid_alsa_audio_info.subs, 1);

	err=snd_seq_subscribe_port(zesarux_mid_alsa_audio_info.handle, zesarux_mid_alsa_audio_info.subs);
	if (err<0) return err;

	return 0;

}

//Se desuscribe al puerto midi indicado. Retorna <0 en caso de error
int alsa_mid_unsubscribe_midi_port(void)
{

	int err;


	snd_seq_port_subscribe_alloca(&zesarux_mid_alsa_audio_info.subs);
	snd_seq_port_subscribe_set_sender(zesarux_mid_alsa_audio_info.subs, &zesarux_mid_alsa_audio_info.sender);
	snd_seq_port_subscribe_set_dest(zesarux_mid_alsa_audio_info.subs, &zesarux_mid_alsa_audio_info.dest);

	snd_seq_port_subscribe_set_time_update(zesarux_mid_alsa_audio_info.subs, 1);
	snd_seq_port_subscribe_set_time_real(zesarux_mid_alsa_audio_info.subs, 1);

	err=snd_seq_unsubscribe_port(zesarux_mid_alsa_audio_info.handle, zesarux_mid_alsa_audio_info.subs);
	if (err<0) {
		debug_printf (VERBOSE_ERR,"Error unsubscribing midi alsa port");
		return err;
	}

	return 0;

}


snd_rawmidi_t *alsa_raw_handle_out = 0;

//char *alsa_mid_device_out="hw:1,0";

//char alsa_mid_device_out[MAX_ALSA_MID_DEVICE_OUT]="hw:0,0";


void alsa_midi_output_reset(void)
{

	z80_byte reset[] = {0xFF, 0, 0,0};

	if (audio_midi_raw_mode) {
		//TODO: Esto no parece hacer nada. El reset de midi solo parece funcionar en windows
		snd_rawmidi_write(alsa_raw_handle_out,reset,4);
	}

}

//enviar nota midi raw inmediatamente, esto viene de las funciones aymidi_rs232_, solo valido cuando hay modo raw
int alsa_midi_raw(z80_byte value)
{

	if (!audio_midi_raw_mode) return 0;

	//printf ("Sending alsa_midi_raw value %02XH\n",value);

	snd_rawmidi_write(alsa_raw_handle_out,&value,1);


	return 0;

}










//Hacer note on de una nota inmediatamente
int alsa_note_on_noraw(unsigned char channel, unsigned char note,unsigned char velocity)
{

	debug_printf (VERBOSE_PARANOID,"noteon event channel %d note %d velocity %d",channel,note,velocity);

	snd_seq_event_t ev;

	snd_seq_ev_clear(&ev);

	snd_seq_ev_set_source(&ev, zesarux_mid_alsa_audio_info.port);
	snd_seq_ev_set_subs(&ev);

	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_noteon(&ev, channel, note, velocity);
	return (snd_seq_event_output(zesarux_mid_alsa_audio_info.handle, &ev));

}


//Hacer note on de una nota inmediatamente en modo raw
int alsa_note_on_raw(unsigned char channel, unsigned char note,unsigned char velocity)
{

  debug_printf (VERBOSE_PARANOID,"noteon event channel %d note %d velocity %d",channel,note,velocity);

  z80_byte noteon[] = {0x90, note, velocity};

  snd_rawmidi_write(alsa_raw_handle_out,noteon,3);


  return 0;
}



int alsa_note_on(unsigned char channel, unsigned char note,unsigned char velocity)
{
	if (audio_midi_raw_mode) return alsa_note_on_raw(channel, note, velocity);
	else return alsa_note_on_noraw(channel, note, velocity);

}

int alsa_change_instrument_raw(unsigned char channel,unsigned char instrument)
{

  debug_printf (VERBOSE_PARANOID,"change instrument event channel %d instrument %d",channel,instrument);

    //El mensaje seria 0xC0 + canal


    z80_byte instrumentchange[] = {0xC0+channel, instrument & 127};

    snd_rawmidi_write(alsa_raw_handle_out,instrumentchange,2);



  return 0;
}

int alsa_change_instrument_noraw(unsigned char channel,unsigned char instrument)
{

	debug_printf (VERBOSE_PARANOID,"change instrument event channel %d instrument %d",channel,instrument);

	snd_seq_event_t ev;

	snd_seq_ev_clear(&ev);

	snd_seq_ev_set_source(&ev, zesarux_mid_alsa_audio_info.port);
	snd_seq_ev_set_subs(&ev);

	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_pgmchange(&ev, channel, instrument);
	return (snd_seq_event_output(zesarux_mid_alsa_audio_info.handle, &ev));

    return 0;
}

int alsa_change_instrument(unsigned char instrument)
{

    int i;
    for (i=0;i<16;i++) {

	    if (audio_midi_raw_mode) alsa_change_instrument_raw(i,instrument);
	    else alsa_change_instrument_noraw(i,instrument);
    }

    return 0;

}


//Hacer note off de una nota inmediatamente
int alsa_note_off_noraw(unsigned char channel, unsigned char note,unsigned char velocity)
{

	debug_printf (VERBOSE_PARANOID,"noteoff event channel %d note %d velocity %d",channel,note,velocity);
	snd_seq_event_t ev;

	snd_seq_ev_clear(&ev);


	snd_seq_ev_set_source(&ev, zesarux_mid_alsa_audio_info.port);
	snd_seq_ev_set_subs(&ev);

	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_noteoff(&ev, channel, note, velocity);
	return (snd_seq_event_output(zesarux_mid_alsa_audio_info.handle, &ev));

}

int alsa_note_off_raw(unsigned char channel, unsigned char note,unsigned char velocity)
{

  debug_printf (VERBOSE_PARANOID,"noteoff event channel %d note %d velocity %d",channel,note,velocity);

  z80_byte noteoff[] = {0x80, note, velocity};

  snd_rawmidi_write(alsa_raw_handle_out,noteoff,3);


  return 0;
}

int alsa_note_off(unsigned char channel, unsigned char note,unsigned char velocity)
{
	if (audio_midi_raw_mode) return alsa_note_off_raw(channel, note, velocity);
	else return alsa_note_off_noraw(channel, note, velocity);
}



//Inicializar el sistema ALSA en modo raw
int alsa_mid_initialize_audio_raw(void)
{

		int err = snd_rawmidi_open(NULL,&alsa_raw_handle_out,audio_raw_midi_device_out,SND_RAWMIDI_NONBLOCK);
        if (err) {
            debug_printf (VERBOSE_ERR,"snd_rawmidi_open %s failed: %d",audio_raw_midi_device_out,err);
			return 1;
        }

	return 0;
}




//Inicializar el sistema ALSA
int alsa_mid_initialize_audio(void)
{
	//Creamos el cliente

	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_initialize_audio: Creating client");

	if (alsa_mid_audio_open_client()==NULL) {
		debug_printf (VERBOSE_ERR,"Error creating alsa client");
		return 1;
	}

	//Creamos el puerto

	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_initialize_audio: Creating port");

	zesarux_mid_alsa_audio_info.port=alsa_mid_audio_new_port(zesarux_mid_alsa_audio_info.handle);

	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_initialize_audio: Port: %d",zesarux_mid_alsa_audio_info.port);

	if (zesarux_mid_alsa_audio_info.port<0) {
		debug_printf (VERBOSE_ERR,"Error creating port");
		return 1;
	}

	//Creamos la cola

	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_initialize_audio: Creating queue");

	zesarux_mid_alsa_audio_info.cua=alsa_mid_audio_new_queue(zesarux_mid_alsa_audio_info.handle);
	if (zesarux_mid_alsa_audio_info.cua<0) {
		debug_printf (VERBOSE_ERR,"Error creating queue");
		return 1;
	}

	//Escogemos el tempo. Realmente hace falta dado que usamos notas directas??
	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_initialize_audio: Choosing tempo");
	alsa_mid_alsa_mid_audio_set_tempo_init();

	//Nos suscribimos al puerto de midi

	debug_printf (VERBOSE_DEBUG,"midi_alsa::alsa_mid_initialize_audio: Subscribing midi client=%d port=%d",
				zesarux_mid_alsa_audio_info.midi_client,zesarux_mid_alsa_audio_info.midi_port);

	if (alsa_mid_subscribe_midi_port(zesarux_mid_alsa_audio_info.midi_client,zesarux_mid_alsa_audio_info.midi_port)<0) {

		debug_printf (VERBOSE_ERR,"Error subscribing midi client=%d port=%d",
				zesarux_mid_alsa_audio_info.midi_client,zesarux_mid_alsa_audio_info.midi_port);
		return 1;

	}
	//subscriure_port_midi_equivalent(midi_client,midi_port);

	//Iniciamos la cola. Dado que lanzamos notas directas esto no hace falta
	//printf ("midi_alsa::alsa_mid_initialize_audio: Iniciem la cua\n");
	/*if (alsa_mid_audio_initialize_queue()<0) {
		printf ("Error iniciant la cua\n");
		exit (1);
	}*/

	return 0;
}




//Utilizado para calcular el valor del volumen
long alsa_mid_percent_to_alsa(int val, long pmin, long pmax)
{
  return pmin + ((pmax - pmin) * val + 50) / 100;
}


//Establecer el volumen del master
void alsa_mid_set_volume_master(int percent GCC_UNUSED)
{
	snd_mixer_t *mixer;
	//snd_mixer_elem_t *elem;    //utilitzat pel master volume
	snd_mixer_selem_id_t *id;
	//long pmin,pmax;

	snd_mixer_selem_id_alloca(&id);

	snd_mixer_open(&mixer, 0);         //Obrir el mesclador
	snd_mixer_attach(mixer, "default");//Utilitzar el mesclador per defecte
	snd_mixer_selem_register(mixer, NULL, NULL);
	snd_mixer_load(mixer);             //Carregar el mesclador

	snd_mixer_selem_id_set_name(id, "Master");

	//elem = snd_mixer_find_selem(mixer, id);

	//Parece que da error al ejecutar esto
	//snd_mixer_selem_get_playback_volume_range(elem,&pmin,&pmax);

	//Parece que da error al ejecutar esto
	//snd_mixer_selem_set_playback_volume_all(elem, alsa_mid_percent_to_alsa(percent,pmin,pmax));

	snd_mixer_close(mixer);

}


//Inicializar la musica
void alsa_mid_initialize_volume(void)
{

        alsa_mid_set_volume_master(alsa_midi_volume);
}







//int alsa_midi_client=0;
//int alsa_midi_port=0;
int alsa_midi_volume=100;



int alsa_mid_initialize_all_noraw(void)
//Devuelve 1 si error
{

        //Inicializar sistema ALSA midi
        zesarux_mid_alsa_audio_info.midi_client=audio_midi_client;
        zesarux_mid_alsa_audio_info.midi_port=audio_midi_port;

    if (alsa_mid_initialize_audio() ) return 1;

	alsa_mid_initialize_volume();

	//if (alsa_mid_initialize_audio_raw() ) return 1;

	return 0;



}


int alsa_mid_initialize_all_raw(void)
//Devuelve 1 si error
{

    if (alsa_mid_initialize_audio_raw() ) return 1;

	return 0;



}


int alsa_mid_initialize_all(void)
{
	if (audio_midi_raw_mode) return alsa_mid_initialize_all_raw();
	else return alsa_mid_initialize_all_noraw();
}



void alsa_mid_finish_all_noraw(void)
{
	alsa_mid_unsubscribe_midi_port();
}

void alsa_mid_finish_all_raw(void)
{


		int err=snd_rawmidi_close(alsa_raw_handle_out);

	if (err<0) {
		debug_printf (VERBOSE_ERR,"Error closing midi alsa raw port");

	}

}

void alsa_mid_finish_all(void)
{
	if (audio_midi_raw_mode) alsa_mid_finish_all_raw();
	else alsa_mid_finish_all_noraw();
}


void alsa_midi_output_flush_output_noraw(void)
{
	snd_seq_drain_output(zesarux_mid_alsa_audio_info.handle);
}


void alsa_midi_output_flush_output_raw(void)
{
	//snd_seq_drain_output(zesarux_mid_alsa_audio_info.handle);
}


void alsa_midi_output_flush_output(void)
{
	if (audio_midi_raw_mode) alsa_midi_output_flush_output_raw();
	else alsa_midi_output_flush_output_noraw();
}


#ifdef USE_PTHREADS

int audioalsa_can_record_input(void)
{

    return 1;

}


pthread_t thread_alsa_capture;

  int capture_buffer_frames = AUDIO_RECORD_BUFFER_SIZE; //128;
  snd_pcm_t *capture_handle;
  char *capture_buffer;

  int alsa_avisado_fifo_llena=0;


//stereo y 16 bits
#define ALSA_CAPTURE_BUFFER (AUDIO_RECORD_BUFFER_SIZE*2*2)

//TEMPORAL
char temppp_buffer[AUDIO_RECORD_BUFFER_SIZE];



struct timeval alsa_tiempo_antes,alsa_tiempo_despues;

long alsa_tiempo_difftime;


unsigned int actual_rate;

void *audioalsa_capture_thread_function(void *nada)
{

//int err;
int leidos;

	while (1) {

        timer_stats_current_time(&alsa_tiempo_antes);

        printf("antes readi\n");
        //Esta funcion es bloqueante y se espera a que acabe
        leidos = snd_pcm_readi (capture_handle, capture_buffer, capture_buffer_frames);
        printf("despues readi. %d\n",leidos);

        if (leidos <0) {
            printf("err: %d\n",leidos);

        fprintf (stderr, "read from audio interface failed. err: %d (%s)\n",
                leidos, snd_strerror (leidos));

                           leidos=0;
                           usleep(1000);
        }

    else {


        //Convertir a signed 8, y a mno
        int i;
        //int destino=0;
        int escritos=0;

        int contador_rate=0;

        for (i=0;i<leidos;) {
            int offset=i*4;
            int canal_izquierdo=capture_buffer[offset]+256*capture_buffer[offset+1];
            int canal_derecho=capture_buffer[offset+2]+256*capture_buffer[offset+3];

            int sumado=(canal_izquierdo+canal_derecho)/2;
            //pasar a 8 bits
            sumado /=256;
            char final=sumado;
            temppp_buffer[escritos]=final;

            //Cutre resample
            //En PC la frecuencia que me retorna no es la deseada, hago un resample
            //descartando valores, suponiendo que frecuencia de sampleado que me da es mayor a la que pido

            if (actual_rate>AUDIO_RECORD_FREQUENCY) {
                contador_rate +=AUDIO_RECORD_FREQUENCY;
                if (contador_rate>=actual_rate) {
                    contador_rate -=actual_rate;
                    escritos++;
                    //printf("incrementar\n");
                }
                else {
                    //printf("no incrementar\n");
                }
                i++;
            }

            else if (AUDIO_RECORD_FREQUENCY>actual_rate) {
                contador_rate +=actual_rate;
                if (contador_rate>=AUDIO_RECORD_FREQUENCY) {
                    contador_rate -=AUDIO_RECORD_FREQUENCY;
                    i++;
                    //printf("incrementar\n");
                }
                else {
                    //printf("no incrementar\n");
                }
                escritos++;
            }

            else {
                escritos++;
                i++;
            }




        }

        int valores_escribir=leidos;

        if (audiorecord_input_fifo_write(temppp_buffer,escritos) && !alsa_avisado_fifo_llena) {
            int miliseconds_lost=(1000*leidos)/AUDIO_RECORD_FREQUENCY;
            debug_printf(VERBOSE_ERR,"External Audio Source buffer is full, a section of %d ms has been lost. "
                "I recommend you to disable and enable External Audio Source in order to empty the input buffer",
                miliseconds_lost);
            alsa_avisado_fifo_llena=1;
        }


    }


        alsa_tiempo_difftime=timer_stats_diference_time(&alsa_tiempo_antes,&alsa_tiempo_despues);
        //fprintf(stdout, "read  done\n");

        long esperado_microseconds=(1000000L*capture_buffer_frames)/AUDIO_RECORD_FREQUENCY;

        printf("tiempo: %ld esperado: %ld frames: %d leidos: %d  (%d %d %d)\n",alsa_tiempo_difftime,esperado_microseconds,
            leidos,leidos,1000000,leidos,AUDIO_RECORD_FREQUENCY);
        printf("%ld\n",esperado_microseconds);
        //printf("long %d long long %d\n",sizeof(long),sizeof(long long));

        long diferencia_a_final=esperado_microseconds-alsa_tiempo_difftime;
        if (diferencia_a_final>0) {
            printf("Falta %ld microsegundos\n",diferencia_a_final);
            //usleep(diferencia_a_final/2);
        }



	}

	//para que no se queje el compilador de variable no usada
	nada=0;
	nada++;


}


void audioalsa_start_record_input_create_thread(void)
{
    if (pthread_create( &thread_alsa_capture, NULL, &audioalsa_capture_thread_function, NULL) ) {
        cpu_panic("Can not create audioalsa pthread");
    }

}

#else

int audioalsa_can_record_input(void)
{
    return 0;

}


void audioalsa_start_record_input_create_thread(void)
{
    //Nada
}

#endif

void audioalsa_start_record_input(void)
{

    int i;
    int err;

    unsigned int rate = AUDIO_RECORD_FREQUENCY;


    snd_pcm_hw_params_t *hw_params;

    //Deberia ser signed 8 bits (SND_PCM_FORMAT_S8) pero no parece gustarle
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;


    //char *capture_device="hw:0";


    if ((err = snd_pcm_open (&capture_handle, alsa_capture_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot open audio device %s (%s)",alsa_capture_device,snd_strerror (err));
        return;
    }


    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot allocate hardware parameter structure (%s)",snd_strerror (err));
        return;
    }


    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot initialize hardware parameter structure (%s)",snd_strerror (err));
        return;
    }


    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot set access type (%s)",snd_strerror (err));
        return;
    }


    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot set sample format (%s)",snd_strerror (err));
        return;
    }


    actual_rate = rate;


    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &actual_rate, 0)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot set sample rate (%s)",snd_strerror (err));
        return;
    }

    if (rate != actual_rate) {
        debug_printf(VERBOSE_WARN,"Alsa: the rate %d Hz is not supported by your hardware. Using %d Hz instead and software resample",
            rate, actual_rate);
    }


    //Deberia ser 1 canal pero tampoco le gusta
    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot set channel count (%s)\n",snd_strerror (err));
        return;
    }


    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot set parameters (%s)",snd_strerror (err));
        return;
    }


    snd_pcm_hw_params_free (hw_params);


    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        debug_printf(VERBOSE_ERR,"Alsa: cannot prepare audio interface for use (%s)\n",snd_strerror (err));
        return;
    }


    //capture_buffer = malloc(capture_buffer_frames * snd_pcm_format_width(format) / 8 *2);

    //16 bits, stereo
    capture_buffer = util_malloc(ALSA_CAPTURE_BUFFER,"Cannot allocate memory for audio capture");

    fprintf(stdout, "buffer allocated\n");


    audioalsa_start_record_input_create_thread();






    audio_is_recording_input=1;

}


void audioalsa_stop_record_input(void)
{
//TODO cancel pthread

    free(capture_buffer);
    audio_is_recording_input=0;


}