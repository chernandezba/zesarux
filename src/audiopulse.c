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
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>


#include "audiopulse.h"
#include "cpu.h"
#include "debug.h"
#include "audio.h"
#include "compileoptions.h"
#include "utils.h"
#include "settings.h"

//temp
#include "timer.h"

#ifdef USE_PTHREADS
#include <pthread.h>

     pthread_t thread1_pulse;


#endif

//buffer de destino para pulse audio debe ser unsigned
unsigned char unsigned_audio_buffer[AUDIO_BUFFER_SIZE*2]; //*2 porque es estereo


#ifndef USE_PTHREADS

//
//
//Inicio Rutinas sin pthreads
//
//

pa_simple *audiopulse_s;
pa_sample_spec audiopulse_ss;




int audiopulse_init(void)
{

        debug_printf (VERBOSE_INFO,"Init Pulse Audio Driver - not using pthreads, %d Hz",FRECUENCIA_SONIDO);

	//audio_driver_accepts_stereo.v=1;


        audiopulse_ss.format = PA_SAMPLE_U8;
        audiopulse_ss.channels = 2;
        audiopulse_ss.rate = FRECUENCIA_SONIDO;


        audiopulse_s = pa_simple_new(NULL, // Use the default server.
                "ZEsarUX", // Our application's name.
                PA_STREAM_PLAYBACK,
                NULL, // Use the default device.
                "Emulation sound", // Description of our stream.
                &audiopulse_ss, // Our sample format.
                NULL, // Use default channel map
                NULL, // Use default buffering attributes.
                NULL // Ignore error code.
        );

        if (audiopulse_s==NULL) {
                debug_printf (VERBOSE_ERR,"Error initializing Pulse Audio Driver");
                return 1;
        }

	//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
	audio_set_driver_name("pulse");

        return 0;
}



void audiopulse_send_frame(char *buffer)
{

        int error;

	//printf ("temp envio sonido\n");


	convert_signed_unsigned(buffer,unsigned_audio_buffer,AUDIO_BUFFER_SIZE*2);  //*2 porque es estereo

        pa_simple_write (audiopulse_s,unsigned_audio_buffer,AUDIO_BUFFER_SIZE*2,&error); //*2 porque es estereo



}

int audiopulse_thread_finish(void)
{
	debug_printf (VERBOSE_DEBUG,"Ending audio pthread");
	return 0;
}



void audiopulse_end(void)
{
        debug_printf (VERBOSE_INFO,"Ending pulse audio driver");
	audio_playing.v=0;

	pa_simple_free(audiopulse_s);

}


void audiopulse_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=AUDIO_BUFFER_SIZE;

  //realmente no usa un buffer fifo, esto puede ser engañoso porque siempre dice que está lleno el buffer
  //pero mejor asi que no diga que siempre esta vacio
  //total esto solo se usa para estadisticas en Core Statistics
  *current_size=AUDIO_BUFFER_SIZE;
}



int audiopulse_can_record_input(void)
{
    return 0;

}


void audiopulse_start_record_input(void)
{

    //Nada

}


void audiopulse_stop_record_input(void)
{

    //Nada

}


#else

//
//
//Inicio Rutinas con pthreads
//
//

pa_simple *audiopulse_s;
pa_sample_spec audiopulse_ss;



int audiopulse_init(void)
{

	//audio_driver_accepts_stereo.v=1;


	//Esto ocurre cuando los dos valen 4 y entonces la fifo siempre dice que esta llena
	if (pulse_periodsize==fifo_pulse_buffer_size) fifo_pulse_buffer_size=AUDIO_BUFFER_SIZE*5;

	debug_printf (VERBOSE_INFO,"Init Pulse Audio Driver - using pthreads. Using pulseperiodsize=%d bytes, fifopulsebuffersize=%d bytes, %d Hz",pulse_periodsize,fifo_pulse_buffer_size,FRECUENCIA_SONIDO);


	audiopulse_ss.format = PA_SAMPLE_U8;
	audiopulse_ss.channels = 2;
	audiopulse_ss.rate = FRECUENCIA_SONIDO;


	audiopulse_s = pa_simple_new(NULL, // Use the default server.
		"ZEsarUX", // Our application's name.
		PA_STREAM_PLAYBACK,
		NULL, // Use the default device.
		"Music", // Description of our stream.
		&audiopulse_ss, // Our sample format.
		NULL, // Use default channel map
		NULL, // Use default buffering attributes.
		NULL // Ignore error code.
	);

	if (audiopulse_s==NULL) {
		debug_printf (VERBOSE_ERR,"Error initializing Pulse Audio Driver");
		return 1;
	}

	//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
	audio_set_driver_name("pulse");

        return 0;
}




//buffer temporal de envio. suficiente para que quepa
char buf_enviar_pulse[AUDIO_BUFFER_SIZE*10*2]; //*2 porque es estereo


int fifo_pulse_write_position=0;
int fifo_pulse_read_position=0;


void audiopulse_empty_buffer(void)
{
  debug_printf(VERBOSE_DEBUG,"Emptying audio buffer");
  fifo_pulse_write_position=0;
}

//nuestra FIFO_PULSE
#define MAX_FIFO_PULSE_BUFFER_SIZE (AUDIO_BUFFER_SIZE*10)

//Desde 4 hasta 10
int fifo_pulse_buffer_size=AUDIO_BUFFER_SIZE*10;

//1-4.
//Cuando habia sonido mono,por defecto estaba a 1
//Con stereo, esta a 2
int pulse_periodsize=AUDIO_BUFFER_SIZE*2;


char fifo_pulse_buffer[MAX_FIFO_PULSE_BUFFER_SIZE*2]; //*2 porque es estereo

int audiopulse_return_fifo_buffer_size(void)
{
  return fifo_pulse_buffer_size*2; //*2 porque es stereo
}

//retorna numero de elementos en la fifo_pulse
int fifo_pulse_return_size(void)
{
        //si write es mayor o igual (caso normal)
        if (fifo_pulse_write_position>=fifo_pulse_read_position) {

		//printf ("write es mayor o igual: write: %d read: %d\n",fifo_pulse_write_position,fifo_pulse_read_position);
		return fifo_pulse_write_position-fifo_pulse_read_position;
	}

        else {
                //write es menor, cosa que quiere decir que hemos dado la vuelta
                return (audiopulse_return_fifo_buffer_size()-fifo_pulse_read_position)+fifo_pulse_write_position;
        }
}

void audiopulse_get_buffer_info (int *buffer_size,int *current_size)
{
  *buffer_size=audiopulse_return_fifo_buffer_size();
  *current_size=fifo_pulse_return_size();
}

//retornar siguiente valor para indice. normalmente +1 a no ser que se de la vuelta
int fifo_pulse_next_index(int v)
{
        v=v+1;
        if (v==audiopulse_return_fifo_buffer_size()) v=0;

        return v;
}

//escribir datos en la fifo_pulse
void fifo_pulse_write(unsigned char *origen,int longitud)
{
        for (;longitud>0;longitud--) {

                //ver si la escritura alcanza la lectura. en ese caso, error
                if (fifo_pulse_next_index(fifo_pulse_write_position)==fifo_pulse_read_position) {
                        debug_printf (VERBOSE_DEBUG,"FIFO_PULSE full");

                        //Si se llena fifo, resetearla a 0 para corregir latencia
                        if (audio_noreset_audiobuffer_full.v==0) audiopulse_empty_buffer();

			//temp resetear fifo
			//fifo_pulse_write_position=0;
			//fifo_pulse_read_position=0;


                        return;
                }

		//Canal izquierdo
                fifo_pulse_buffer[fifo_pulse_write_position]=*origen++;
                fifo_pulse_write_position=fifo_pulse_next_index(fifo_pulse_write_position);

		//Canal derecho
                fifo_pulse_buffer[fifo_pulse_write_position]=*origen++;
                fifo_pulse_write_position=fifo_pulse_next_index(fifo_pulse_write_position);
	}
}


//leer datos de la fifo_pulse
void fifo_pulse_read(char *destino,int longitud)
{
        for (;longitud>0;longitud--) {

		if (fifo_pulse_return_size()==0) {
                        debug_printf (VERBOSE_DEBUG,"FIFO_PULSE vacia");
                        return;
                }


                //ver si la lectura alcanza la escritura. en ese caso, error
                //if (fifo_pulse_next_index(fifo_pulse_read_position)==fifo_pulse_write_position) {
                //        debug_printf (VERBOSE_INFO,"FIFO_PULSE vacia");
                //        return;
                //}

                *destino++=fifo_pulse_buffer[fifo_pulse_read_position];
                fifo_pulse_read_position=fifo_pulse_next_index(fifo_pulse_read_position);
        }
}


int audiopulse_thread_finish(void)
{

	if (thread1_pulse!=0) {
        	debug_printf (VERBOSE_DEBUG,"Ending audiopulse thread");
		int s=pthread_cancel(thread1_pulse);
		if (s != 0) debug_printf (VERBOSE_DEBUG,"Error cancelling pthread pulse");

		thread1_pulse=0;

	}

	//Pausa de 0.1 segundo
	usleep(100000);


	return 0;

}

void audiopulse_end(void)
{
        debug_printf (VERBOSE_INFO,"Ending pulse audio driver");
        audiopulse_thread_finish();
	audio_playing.v=0;

	//pa_simple_free(audiopulse_s);

}



void audiopulse_enviar_audio_envio(void)
{

	int error;



		if (fifo_pulse_return_size()>=pulse_periodsize) {


			//Si hay detectado silencio, la fifo estara vacia y por tanto ya no entrara aqui y no enviara sonido


			//manera normal usando funciones de fifo

			//printf ("temp envio sonido\n");

			fifo_pulse_read(buf_enviar_pulse,pulse_periodsize);
			pa_simple_write (audiopulse_s,buf_enviar_pulse,pulse_periodsize,&error);

                        //Siguiente fragmento de audio. Es mejor hacerlo aqui que no esperar
                        //Esto da sonido correcto. Porque? No estoy seguro del todo...
			fifo_pulse_read(buf_enviar_pulse,pulse_periodsize);
			pa_simple_write (audiopulse_s,buf_enviar_pulse,pulse_periodsize,&error);

			//printf ("sonido despues\n");
			//Cuando se vuelve de multitarea, parece que el primer pa_simple_write bloquea y se queda esperando
			//y no se llega a este "sonido despues"
			//para ello, lo que hacemos, es llamar a init en send_frame


			interrupt_finish_sound.v=1;

		}

		else {
			//printf ("temp en usleep de envio audio\n");
                        usleep(1000);
		}
}



char *buffer_playback_pulse;
//int pthread_enviar_sonido_pulse=0;
int frames_sonido_enviados_pulse=0;

void *audiopulse_enviar_audio(void *nada)
{


	while (1) {

			//tamanyo antes
			//printf ("enviar. antes. tamanyo fifo: %d read %d write %d\n",fifo_pulse_return_size(),fifo_pulse_read_position,fifo_pulse_write_position);
			audiopulse_enviar_audio_envio();

	}

	//para que no se queje el compilador de variable no usada
	nada=0;
	nada++;


}



pthread_t thread1_pulse=0;

void audiopulse_send_frame(char *buffer)
{

        //pthread_enviar_sonido_pulse=1;
        if (audio_playing.v==0) {
                //Volvemos a activar pthread
                buffer_playback_pulse=buffer;
                audio_playing.v=1;

        }

        if (thread1_pulse==0) {
                buffer_playback_pulse=buffer;

		debug_printf(VERBOSE_DEBUG,"Creating audiopulse pthread");


                if (pthread_create( &thread1_pulse, NULL, &audiopulse_enviar_audio, NULL) ) {
                        cpu_panic("Can not create audiopulse pthread");
                }
        }

                        //tamanyo antes
                        //printf ("write. antes. tamanyo fifo: %d read %d write %d\n",fifo_pulse_return_size(),fifo_pulse_read_position,fifo_pulse_write_position);


	convert_signed_unsigned(buffer,unsigned_audio_buffer,AUDIO_BUFFER_SIZE*2); //*2 porque es estereo

	fifo_pulse_write(unsigned_audio_buffer,AUDIO_BUFFER_SIZE);

                        //tamanyo despues
                        //printf ("write. despues. tamanyo fifo: %d read %d write %d\n",fifo_pulse_return_size(),fifo_pulse_read_position,fifo_pulse_write_position);
}








pthread_t thread_pulse_capture;


  int pulse_avisado_fifo_llena=0;


//stereo y 16 bits
#define PULSE_CAPTURE_BUFFER (AUDIO_RECORD_BUFFER_SIZE*2*2)





struct timeval pulse_tiempo_antes,pulse_tiempo_despues;

long pulse_tiempo_difftime;


pa_simple *audiopulse_record_s;
pa_sample_spec audiopulse_record_ss;

int audiopulse_record_must_finish=0;

char buffer_audiopulse_captura_temporal[AUDIO_RECORD_BUFFER_SIZE];

int audio_capture_thread_running=0;

void *audiopulse_capture_thread_function(void *nada)
{

    audio_capture_thread_running=1;

    int err;

	while (!audiopulse_record_must_finish) {

        timer_stats_current_time(&pulse_tiempo_antes);

        printf("antes pa_simple_read\n");


        //Esta funcion es bloqueante y se espera a que acabe
        if (pa_simple_read (audiopulse_record_s,buffer_audiopulse_captura_temporal,AUDIO_RECORD_BUFFER_SIZE,&err) <0) {


            fprintf (stderr, "read from audio interface failed. err: %d\n",
                err);


                    usleep(1000);
        }

        else {

            //Convertir unsigned en signed
            int i;
            for (i=0;i<AUDIO_RECORD_BUFFER_SIZE;i++) {
                z80_byte valor=(z80_byte) buffer_audiopulse_captura_temporal[i];
                int valor_signo=valor-128;
                buffer_audiopulse_captura_temporal[i]=valor_signo;
            }


            if (audiorecord_input_fifo_write(buffer_audiopulse_captura_temporal,AUDIO_RECORD_BUFFER_SIZE) && !pulse_avisado_fifo_llena) {
                int miliseconds_lost=(1000*AUDIO_RECORD_BUFFER_SIZE)/AUDIO_RECORD_FREQUENCY;
                debug_printf(VERBOSE_ERR,"External Audio Source buffer is full, a section of %d ms has been lost. "
                    "I recommend you to disable and enable External Audio Source in order to empty the input buffer",
                    miliseconds_lost);
                pulse_avisado_fifo_llena=1;
            }


        }


        pulse_tiempo_difftime=timer_stats_diference_time(&pulse_tiempo_antes,&pulse_tiempo_despues);
        //fprintf(stdout, "read  done\n");

        long esperado_microseconds=(1000000L*AUDIO_RECORD_BUFFER_SIZE)/AUDIO_RECORD_FREQUENCY;

        printf("tiempo: %ld esperado: %ld\n",pulse_tiempo_difftime,esperado_microseconds);

        //printf("long %d long long %d\n",sizeof(long),sizeof(long long));

        long diferencia_a_final=esperado_microseconds-pulse_tiempo_difftime;
        printf("Diferencia %ld microsegundos\n",diferencia_a_final);
        if (diferencia_a_final>0) {
            printf("Falta %ld microsegundos\n",diferencia_a_final);
            //usleep(diferencia_a_final/2);
        }

	}

	//para que no se queje el compilador de variable no usada
	nada=0;
	nada++;

    printf("finished audio pulse record\n");


    audio_capture_thread_running=0;


    return NULL;


}


void audiopulse_start_record_input_create_thread(void)
{

    audiopulse_record_must_finish=0;

    if (pthread_create( &thread_pulse_capture, NULL, &audiopulse_capture_thread_function, NULL) ) {
        cpu_panic("Can not create audiopulse pthread");
    }

}

pa_buffer_attr audiopulse_record_attributes;

void audiopulse_start_record_input(void)
{
        //Vaciar posible sonido que haya antes del buffer, por si el usuario ha desactivado y activado varias veces
        audiorecord_input_empty_buffer_with_lock();
printf("Start audiopulse record\n");

        audiopulse_record_ss.format = PA_SAMPLE_U8;
        audiopulse_record_ss.channels = 1;
        audiopulse_record_ss.rate = AUDIO_RECORD_FREQUENCY;

        audiopulse_record_attributes.maxlength=-1;
        //Si dejasemos esto por defecto, enviaria los datos a rafagas de 2s, cosa que funcionaria,
        //pero por ejemplo en la ventana de external audio source, la forma de onda va y viene
        //mejor bajarlo al trozo que queremos realmente leer
        audiopulse_record_attributes.fragsize=AUDIO_RECORD_BUFFER_SIZE;


        audiopulse_record_s = pa_simple_new(NULL, // Use the default server.
                "ZEsarUX", // Our application's name.
                PA_STREAM_RECORD,
                NULL, // Use the default device.
                "Record", // Description of our stream.
                &audiopulse_record_ss, // Our sample format.
                NULL, // Use default channel map
                &audiopulse_record_attributes,
                NULL // Ignore error code.
        );

        if (audiopulse_record_s==NULL) {
                debug_printf (VERBOSE_ERR,"Error initializing Pulse Audio Recording");
                return;
        }

    audiopulse_start_record_input_create_thread();






    audio_is_recording_input=1;

    printf("Finish initializing audiopulse record\n");

}


void audiopulse_stop_record_input(void)
{

    audiopulse_record_must_finish=1;


    while (audio_capture_thread_running) {
        timer_sleep(100);
    }

    audio_is_recording_input=0;

    pa_simple_free(audiopulse_record_s);


}

int audiopulse_can_record_input(void)
{
    return 1;

}

#endif