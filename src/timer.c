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

/*
   Timing functions
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#ifndef MINGW
	#include <unistd.h>
#endif


#include "timer.h"
#include "cpu.h"
#include "start.h"
#include "audio.h"
#include "audio_ayplayer.h"
#include "debug.h"
#include "zxvision.h"
#include "tape.h"
#include "screen.h"
#include "compileoptions.h"
#include "zx8081.h"
#include "joystick.h"
#include "utils.h"
#include "printers.h"
#include "z88.h"
#include "zxuno.h"
#include "textspeech.h"
#include "mmc.h"
#include "ide.h"
#include "zxpand.h"
#include "superupgrade.h"
#include "snap.h"
#include "snap_rzx.h"
#include "ql.h"
#include "esxdos_handler.h"
#include "betadisk.h"
#include "pd765.h"
#include "settings.h"
#include "ql_i8049.h"
#include "cpc.h"
#include "snap_ram.h"
#include "hilow_datadrive.h"
#include "dsk.h"
#include "menu_items.h"
#include "menu_items_storage.h"
#include "pcw.h"
#include "utils_text_adventure.h"
#include "zeng_online.h"
#include "zeng_online_client.h"
#include "zxvision_topbar.h"
#include "microdrive.h"
#include "zxmmcplus.h"

#include "autoselectoptions.h"




#ifdef USE_PTHREADS

#include <pthread.h>

pthread_t thread_usleep_timer;

#endif


#ifdef COMPILE_SDL

	#ifdef COMPILE_SDL2
		#include "common_sdl2.h"
	#else
		#include "common_sdl.h"
	#endif
#endif

#ifdef USE_COCOA
    #include "scrcocoa.h"
#endif




//El TIMER_END siempre se mantiene como delimitador de final
struct s_zesarux_timer available_timers[TIMER_LIST_MAX_SIZE]={
    { TIMER_END,"end",NULL,NULL}
};


enum timer_type timer_selected=TIMER_UNASSIGNED;


char timer_user_preferred[TIMER_MAX_NAME]="";


void timer_set_preferred_by_name(char *name)
{
   strcpy(timer_user_preferred,name);
   //printf("Timer set: %s\n",name);
}




void timer_debug_print_timer_list(struct s_zesarux_timer *lista)
{
    int i;

    for (i=0;i<TIMER_LIST_MAX_SIZE;i++) {
        debug_printf(VERBOSE_DEBUG,"Timer %d Name: [%s] ID timer %d",i,lista[i].name,lista[i].timer);
        //printf("Timer %d Name: [%s] Value %d\n",i,lista[i].name,lista[i].timer);
        if (lista[i].timer==TIMER_END) break;
    }

}

int timer_find(struct s_zesarux_timer *timer_list,enum timer_type timer_to_find)
{
    int i;

    for (i=0;i<TIMER_LIST_MAX_SIZE;i++) {
        //printf("%d %d %d\n",i,timer_list[i].timer,timer_to_find);
        if (timer_list[i].timer==timer_to_find) return i;

        //Si llegamos al fin. Nota: se puede haber buscado el TIMER_END y en ese caso saldria por la condicion anterior
        if (timer_list[i].timer==TIMER_END) return -1;
    }

    return -1;

}

int timer_find_by_name(struct s_zesarux_timer *timer_list,char *timer_to_find)
{
    int i;

    for (i=0;i<TIMER_LIST_MAX_SIZE;i++) {
        if (!strcmp(timer_list[i].name,timer_to_find)) return i;
        if (timer_list[i].timer==TIMER_END) return -1;
    }

    return -1;

}


int timer_list_is_full(struct s_zesarux_timer *timer_list)
{
    //Si el ultimo es end, no hay mas sitio
    int end_pos=timer_find(timer_list,TIMER_END);
    //printf("end_pos: %d\n",end_pos);
    if (end_pos==TIMER_LIST_MAX_SIZE-1) {
        return 1;
    }

    else return 0;
}

//Agrega un temporizador al principio de la list
//Nota: siempre le paso el puntero al array asi puedo tener diferentes arrays, uno para los timers normales y el otro para codetests
void timer_add_timer_to_top(struct s_zesarux_timer *timer_list,enum timer_type timer_to_add,char *name,int (*start)(void),void (*stop)(void))
{
    debug_printf(VERBOSE_DEBUG,"Add timer %s to top",name);

    //Ver si hay sitio para meter otro
    if (timer_list_is_full(timer_list)) {
        debug_printf(VERBOSE_DEBUG,"Can not add timer %s to top. List is full",name);
        //printf("Can not add timer to top. List is full\n");
        return;
    }

    //No agregar si ya esta
    int pos_exists=timer_find(timer_list,timer_to_add);
    if (pos_exists>=0) {
        debug_printf(VERBOSE_DEBUG,"Timer %s already exists on position %d",name,pos_exists);
        return;
    }


    //Mover todos hacia abajo
    int i;

    for (i=TIMER_LIST_MAX_SIZE-1;i>=1;i--) {
       timer_list[i].timer=timer_list[i-1].timer;
       strcpy(timer_list[i].name,timer_list[i-1].name);
       timer_list[i].start=timer_list[i-1].start;
       timer_list[i].stop=timer_list[i-1].stop;
    }

    //Agregar al primero
    timer_list[0].timer=timer_to_add;
    strcpy(timer_list[0].name,name);
    timer_list[0].start=start;
    timer_list[0].stop=stop;

}

//Agrega un temporizador al final de la lista
void timer_add_timer_to_bottom(struct s_zesarux_timer *timer_list,enum timer_type timer_to_add,char *name,int (*start)(void),void (*stop)(void))
{
    debug_printf(VERBOSE_DEBUG,"Add timer %s to bottom",name);

    //Ver si hay sitio para meter otro
    if (timer_list_is_full(timer_list)) {
        debug_printf(VERBOSE_DEBUG,"Can not add timer %s to bottom. List is full",name);
        //printf("Can not add timer to bottom. List is full\n");
        return;
    }

    //No agregar si ya esta
    int pos_exists=timer_find(timer_list,timer_to_add);
    if (pos_exists>=0) {
        debug_printf(VERBOSE_DEBUG,"Timer %s already exists on position %d",name,pos_exists);
        return;
    }

    //Mover todos hacia abajo
    int i;

    for (i=0;i<TIMER_LIST_MAX_SIZE-1;i++) {
       if (timer_list[i].timer==TIMER_END) break;
    }


    timer_list[i].timer=timer_to_add;
    strcpy(timer_list[i].name,name);
    timer_list[i].start=start;
    timer_list[i].stop=stop;


    timer_list[i+1].timer=TIMER_END;
    strcpy(timer_list[i+1].name,"end");
    timer_list[i+1].start=NULL;
    timer_list[i+1].stop=NULL;

}



//Quito un temporizador de la lista
void timer_remove_timer(struct s_zesarux_timer *timer_list,enum timer_type timer_to_remove)
{

    debug_printf(VERBOSE_DEBUG,"Remove timer id %d",timer_to_remove);

    //Primero buscarlo
    int i;

    for (i=0;i<TIMER_LIST_MAX_SIZE;i++) {
        if (timer_list[i].timer==timer_to_remove) break;
    }

    if (i==TIMER_LIST_MAX_SIZE) {
        debug_printf(VERBOSE_DEBUG,"Timer to remove %d not found",timer_to_remove);
        return;
    }

    //Si el timer activo es el que quitamos, tenemos que parar ese timer
    if (timer_to_remove==timer_selected) {
        debug_printf(VERBOSE_INFO,"Stopping current timer as it has been deleted");
        timer_list[i].stop();
    }

    //Mover todos hacia el que queremos borrar

    for (;i<TIMER_LIST_MAX_SIZE-1;i++) {
       timer_list[i].timer=timer_list[i+1].timer;
       strcpy(timer_list[i].name,timer_list[i+1].name);
       timer_list[i].start=timer_list[i+1].start;
       timer_list[i].stop=timer_list[i+1].stop;
    }

    //Si el timer activo es el que quitamos, tenemos que inicializar el timer disponible
    if (timer_to_remove==timer_selected) {
        debug_printf(VERBOSE_DEBUG,"Starting available timer as the current timer has been deleted");
        start_timer();
    }

}


void stop_current_timer(void)
{


    debug_printf(VERBOSE_DEBUG,"Stop current timer ID timer %d",timer_selected);

    //debug_exec_show_backtrace();

    if (timer_selected==TIMER_UNASSIGNED) {
        debug_printf(VERBOSE_DEBUG,"Timer has not been assigned. Return without stopping any timer");
        return;
    }

    int pos=timer_find(available_timers,timer_selected);

    if (pos>=0) {

        void (*stop_function)(void);
        stop_function=available_timers[pos].stop;

        if (stop_function==NULL) {
            debug_printf(VERBOSE_DEBUG,"Timer stop function is NULL");
        }
        else {
            stop_function();
        }


    }
    else {
        debug_printf(VERBOSE_DEBUG,"Can not stop current timer as it can not be found");
    }


}


//Pausa de microsegundos
//Por alguna razón que desconozco, en Mac usando el driver de audio null, driver de video cocoa y cuando la aplicación no tiene el foco,
//a veces este usleep tarda muchisimo, del orden de 10 segundos o asi
//de tal manera que ZEsarUX se queda congelado
//Bugs de usleep en Mac que comentan que a veces tarda mas, que podrian tener relación:
//https://github.com/emscripten-core/emscripten/issues/16499
//https://savannah.gnu.org/bugs/?46845
void timer_usleep(int usec)
{
	usleep (usec);
}


//Pausa de milisegundos
void timer_sleep(int milisec)
{
    //usleep(milisec*1000);
    timer_usleep(milisec*1000);
}

int timer_init_date(void)
{
    debug_printf(VERBOSE_INFO,"Initializing timer Date");
    //Siempre inicializa
    return 1;
}

void timer_stop_date(void)
{
    //No hace nada realmente
}

//Interrupcion de timer generada por timers que se lanzan desde thread, como por ejemplo usleep o sdl
int timer_pthread_generada=0;

void timer_trigger_interrupt(void)
{
    timer_pthread_generada=1;
}


void *usleep_timer_function(void *nada)
{
	while (1) {
		timer_usleep(timer_sleep_machine);

		//printf ("tick timer thread %d\n",contador_segundo);
		timer_trigger_interrupt();

#ifdef USE_PTHREADS
        pthread_testcancel();
#endif

	}

	//para que no se queje el compilador de variable no usada
	nada=0;
	nada++;


	return NULL;
}




int timer_init_usleep(void)
{
    //printf("timer_init_usleep\n");
    debug_printf(VERBOSE_INFO,"Initializing timer Usleep for %d microsec",timer_sleep_machine);
    //printf("----Llamado a timer_init_usleep\n");
    //debug_exec_show_backtrace();


#ifdef USE_PTHREADS
    if (pthread_create( &thread_usleep_timer, NULL, &usleep_timer_function, NULL) ) {
        //Error al lanzar el thread
        return 0;
    }
    else {
        //Ok inicializado
        //y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	    pthread_detach(thread_usleep_timer);

        return 1;
    }
#endif

    //No inicializa
    return 0;
}

void timer_stop_usleep(void)
{
    debug_printf(VERBOSE_INFO,"Stopping timer Usleep");

#ifdef USE_PTHREADS
    //printf("Stopping timer Usleep %p\n",thread_usleep_timer);
    int returncode=pthread_cancel(thread_usleep_timer);
    if (returncode) {
        debug_printf(VERBOSE_INFO,"Error canceling timer usleep err=%d",returncode);
    }

#endif
}




int start_timer_specified(struct s_zesarux_timer *t)
{



    int (*start_function)(void);
    start_function=t->start;

    if (start_function==NULL) {
        debug_printf(VERBOSE_DEBUG,"Timer start function is NULL");
        return 0;
    }

    int return_init=start_function();

    return return_init;
}


//IMPORTANTE: Todas las llamadas a start_timer deben ir precedidas de un stop_current_timer, para evitar
//arrancar dos timers a la vez
//Adicional: aunque ahora los timers van vinculados a un driver de video (sdl) o sistema (mac-cocoa), podria
//haber otros timers vinculados a una señal hardware, como un cristal de cuarzo que genera 50 hz o cosas asi
/*

Antes de implementar este nuevo sistema de timers, funcionaba así:

- Si no habia soporte de threads en compilación, se utilizaba timer Date
- Si habia soporte de threads en compilación, se utilizaba:
  - En Mac, timer Date (porque no es preciso el timer Sleep en este caso, y va todo algo mas rápido)
  - En Windows, timer Date (porque generaba algunos "clicks" en el audio)
  - En el resto, timer Sleep
*/
void start_timer(void)
{
    //debug_printf(VERBOSE_INFO,"Start Timer");

    //Si el usuario tiene un timer favorito

    debug_printf(VERBOSE_DEBUG,"Start timer. Available timers:");
    timer_debug_print_timer_list(available_timers);

    if (timer_user_preferred[0]) {

        debug_printf(VERBOSE_DEBUG,"Trying preferred timer %s initialization",timer_user_preferred);

        int pos=timer_find_by_name(available_timers,timer_user_preferred);

        if (pos>=0) {

            //char timer_name[TIMER_MAX_NAME];

            //strcpy(timer_name,available_timers[pos].name);

            //debug_printf(VERBOSE_INFO,"Trying %s preferred timer initialization",timer_name);

            //printf("Trying %s preferred timer initialization\n",timer_name);

            timer_selected=available_timers[pos].timer;

            //printf("timer selected: %d\n",timer_selected);


            if (!start_timer_specified(&available_timers[pos])) {
                debug_printf(VERBOSE_DEBUG,"Preferred timer by the user failed initialization. Trying all available");
                //printf("Preferred timer by the user failed initialization. Trying all available\n");
            }
            else {
                return;
            }
        }

        else {
            debug_printf(VERBOSE_DEBUG,"Preferred timer %s not found",timer_user_preferred);
        }
    }


    //Ir recorriendo del primero al ultimo y quedarse con el primero que de ok al inicializar
    debug_printf(VERBOSE_DEBUG,"Try available timers in order");
    //printf("Try available timers in order\n");


    int i;

    int return_init=0;

    for (i=0;i<TIMER_LIST_MAX_SIZE && available_timers[i].timer!=TIMER_END;i++) {
        timer_selected=available_timers[i].timer;

        char timer_name[TIMER_MAX_NAME];

        strcpy(timer_name,available_timers[i].name);

        debug_printf(VERBOSE_DEBUG,"Trying %s timer initialization",timer_name);

        //printf("Trying %s timer initialization\n",timer_name);

        //Las funciones init de cada timer retornan 0 si error. No 0 si ok
        //return_init=init_timer_selected(timer_selected);

        return_init=start_timer_specified(&available_timers[i]);



        if (return_init) {
            //printf("Inicializado %s timer\n",timer_name);
            return;
        }

        else {
            debug_printf(VERBOSE_DEBUG,"Failed timer initialization. Trying next");
        }

    }

    //Llegado aqui han fallado todos los timers. Hacermos fallback al timer date que deberia funcionar siempre
    //y que deberia haber estado en la lista de disponibles, pero por si acaso
    debug_printf(VERBOSE_DEBUG,"All timers failed initialization. Fallback to date timer");
    timer_selected=TIMER_DATE;


}


void timer_change(char *t)
{
    //Parar el timer sin activar otro tendria consecuencias catastróficas ya que nadie estaria activando
    //como timer y cualquier evento del menu o de la emulación que dependa de tiempo no funcionaria (probablemente se colgaria)
    //Por tanto debe activarse otro timer a continuación
    stop_current_timer();
    strcpy(timer_user_preferred,t);
    start_timer();
}

void timer_restart(void)
{
    stop_current_timer();
    start_timer();
}

void timer_add_timer_usleep_to_top(void)
{
    timer_add_timer_to_top(available_timers,TIMER_USLEEP,"usleep",timer_init_usleep,timer_stop_usleep);
}

void timer_add_timer_usleep_to_bottom(void)
{
    timer_add_timer_to_bottom(available_timers,TIMER_USLEEP,"usleep",timer_init_usleep,timer_stop_usleep);
}

void init_timer(void)
{

    debug_printf(VERBOSE_DEBUG,"Preparing available timers. Initials:");
    timer_debug_print_timer_list(available_timers);

    //Primero quitar o poner timers segun drivers y segun sistema operativo

    //Date siempre disponible
    timer_add_timer_to_top(available_timers,TIMER_DATE,"date",timer_init_date,timer_stop_date);

#ifdef USE_PTHREADS
    timer_add_timer_usleep_to_top();
#endif


    debug_printf(VERBOSE_DEBUG,"Available timers after adding some:");
    timer_debug_print_timer_list(available_timers);



#ifdef MINGW
    //Parece que en Windows el timer usleep no funciona bien... lo metemos al final de la lista de prioridades
    timer_remove_timer(available_timers,TIMER_USLEEP);

#ifdef USE_PTHREADS
    timer_add_timer_usleep_to_bottom();
#endif

#endif


#if defined(__APPLE__)
    //Parece que en Mac el timer usleep no funciona bien... lo metemos al final de la lista de prioridades
    timer_remove_timer(available_timers,TIMER_USLEEP);
    //printf("Quitar usleep\n");

#ifdef USE_PTHREADS
    timer_add_timer_usleep_to_bottom();
    //printf("Poner usleep al fondo\n");
#endif

#endif


    debug_printf(VERBOSE_DEBUG,"Available timers:");
    //printf("Available timers at final of init_timer:\n");
    timer_debug_print_timer_list(available_timers);


}



//tiempo en microsegundos que dura cada interrupcion de timer
int timer_sleep_machine=20000;

//valor original antes de aplicar cpu speed
int original_timer_sleep_machine=20000;


//Va desde 0 a 1000. Si llega a 1000 se pone a 0. Se incrementa en 20 cada 20 ms
int contador_segundo=0;

//Mismo que contador_segundo pero no se resetea a 0. Se usa en draw overlays, es mejor que comparar contra contador_segundo cuando
//se redibuja por ejemplo dos veces por segundo, en que muchas veces coincide el contador anterior con el actual y refresca
//menos de lo que deberia
int contador_segundo_infinito;

//Va desde 0 a 20000. Cuando llega a 20000 se pone a 0. Indica cuando se ha pasado 20 ms
int contador_20ms=0;




int conta_envio_audio=0;


//Contador que se decrementa(si esta activo) cada 1/50 s, e indica cuando se debe liberar una tecla pulsada desde el menu de On Screen Keyboard
int timer_on_screen_key=0;

//Parecido pero para adventure keyboard
int timer_on_screen_adv_key=0;


//lo que dura un frame en microsegundos  (20 ms = 200000 milisec)
//#define FRAME_MICROSECONDS (1000000/50)



//Estadisticas de diferentes partes del emulador

// Tiempo que se tarda en ejecutar todas las instrucciones de un frame completo de pantalla
//Tiempos antes y despues
struct timeval core_cpu_timer_frame_antes,core_cpu_timer_frame_despues;
//Ultimo intervalo de tiempo
long core_cpu_timer_frame_difftime;
//Media de todos los intervalos
long core_cpu_timer_frame_media=0;

// Tiempo que se tarda en refrescar la pantalla
//Tiempos antes y despues
struct timeval core_cpu_timer_refresca_pantalla_antes,core_cpu_timer_refresca_pantalla_despues;
//Ultimo intervalo de tiempo
long core_cpu_timer_refresca_pantalla_difftime;
//Media de todos los intervalos
long core_cpu_timer_refresca_pantalla_media=0;


//Tiempo entre cada refresco de pantalla. Idealmente: 20 ms
//Tiempos antes y despues
struct timeval core_cpu_timer_each_frame_antes,core_cpu_timer_each_frame_despues;
//Ultimo intervalo de tiempo
long core_cpu_timer_each_frame_difftime;
//Media de todos los intervalos
long core_cpu_timer_each_frame_media=0;


//Tiempo de renderizado de la funcion de overlay
struct timeval core_render_menu_overlay_antes,core_render_menu_overlay_despues;
//Ultimo intervalo de tiempo
long core_render_menu_overlay_difftime;
//Media de todos los intervalos
long core_render_menu_overlay_media=0;

//Tiempo en descomprimir un snapshot zip de ZENG Online. En microsegundos
struct timeval zeng_online_uncompress_time_antes,zeng_online_uncompress_time_despues;
//Ultimo intervalo de tiempo
long zeng_online_uncompress_difftime;
//Media de todos los intervalos
long zeng_online_uncompress_media=0;

//Tiempo en comprimir un snapshot zip de ZENG Online. En microsegundos
struct timeval zeng_online_compress_time_antes,zeng_online_compress_time_despues;
//Ultimo intervalo de tiempo
long zeng_online_compress_difftime;
//Media de todos los intervalos
long zeng_online_compress_media=0;


void timer_stats_current_time(struct timeval *tiempo)
{
	gettimeofday(tiempo, NULL);
}


//Diferencia de dos tiempos pero calculando el tiempo_despues segun tiempo actual
//En microsegundos
long timer_stats_diference_time(struct timeval *tiempo_antes, struct timeval *tiempo_despues)
{
	timer_stats_current_time(tiempo_despues);
	long difftime_seconds, difftime_useconds;

	difftime_seconds = tiempo_despues->tv_sec  - tiempo_antes->tv_sec;
	difftime_useconds = tiempo_despues->tv_usec  - tiempo_antes->tv_usec;

	long difftime = ((difftime_seconds) * 1000000 + difftime_useconds);

	return difftime;
}

long timer_get_current_seconds(void)
{
    struct timeval struct_tiempo_ahora;

    timer_stats_current_time(&struct_tiempo_ahora);

    return struct_tiempo_ahora.tv_sec;
}









z80_bit top_speed_timer={0};
int top_speed_real_frames=0;

z80_bit interrupcion_fifty_generada={0};

int timer_condicion_top_speed(void)
{
        if (menu_abierto==1) return 0;
        if (top_speed_timer.v) return 1;
        return 0;
}






void timer_pause_waiting_end_frame(void)
{

	//printf ("esperando final frame\n");

	//lo ideal seria hacer un "halt" de la cpu, para esperar a la siguiente interrupcion, pero
	//ahora mismo las interrupciones se generan mediante un "timer" del spectrum, por tanto,
	//no hay interrupcion posible

	//hacemos una pequenya pausa para liberar la cpu un poco

	//manera normal de hacer siempre sleep
	//pese a las pruebas realizadas en benchmark, de esperar un tiempo proporcional al que falta para final de ciclo,
	//acaba dando el mismo uso de cpu que este: esperar 1 ms


	if (MACHINE_IS_Z88) {
		//Como un frame de Z88 dura 1/4 del spectrum (5 ms) esta pausa tambien es 1/4

		//11% cpu. teclas responde casi instantaneo
		timer_usleep(1000/4);

	}


	else {
#ifdef EMULATE_RASPBERRY
		//En raspberry el timer es mejor que sea menor (una xxx parte del normal)
		timer_usleep(1000/16);

#else

    //Otras esperas para final de frame, por ejemplo para FiwixOS, esto mejora la velocidad
    #ifdef SHORT_SLEEP_TIMER
        //printf("Using short timer\n");
        timer_usleep(1000/16);
    #else

		//Timer normal
		timer_sleep(1);
    #endif

#endif
	}


}


//necesario llamar aqui cuando no hay pthreads y hay paro de emulacion durante 1 segundo o mas...
void timer_reset(void)
{
	gettimeofday(&z80_interrupts_timer_antes, NULL);
}


long z80_timer_acumulado=0;

//Devuelve 1 si hay interrupcion
int timer_check_interrupt_thread(void)
{
	if (timer_pthread_generada) {
		top_speed_real_frames++;
		timer_pthread_generada=0;

		if (MACHINE_IS_Z88) {
			conta_envio_audio++;
			if (conta_envio_audio>=4) {
				envio_audio();
				//printf ("enviamos audio\n");
				conta_envio_audio=0;

			}
		}

		else envio_audio();

		//printf ("int pthread\n");
		return 1;
	}

	return 0;
}



//Devuelve 1 si hay interrupcion
int timer_check_interrupt_no_thread(void)
{


	int timer_warn_high_interrupt_time,timer_warn_low_interrupt_time,timer_interrupt_time;


	timer_interrupt_time=timer_sleep_machine;
	timer_warn_low_interrupt_time=timer_interrupt_time-timer_interrupt_time/2;


	if (MACHINE_IS_Z88) {
		timer_warn_high_interrupt_time=timer_interrupt_time+timer_interrupt_time*12;
	}

	else {
		timer_warn_high_interrupt_time=timer_interrupt_time+timer_interrupt_time*3;
	}


	gettimeofday(&z80_interrupts_timer_ahora, NULL);


		z80_timer_seconds  = z80_interrupts_timer_ahora.tv_sec  - z80_interrupts_timer_antes.tv_sec;
		z80_timer_useconds = z80_interrupts_timer_ahora.tv_usec - z80_interrupts_timer_antes.tv_usec;

		z80_timer_difftime = ((z80_timer_seconds) * 1000000 + z80_timer_useconds);


		//compensar el tiempo que nos hemos pasado antes (positivo o negativo)
		z80_timer_difftime +=z80_timer_acumulado;


		if (z80_timer_difftime>=timer_interrupt_time)  {
            //Ver que tambien el timer de lectura de record input haya llegado
            //if (timer_wait_record_input_interrupt() ) {

			top_speed_real_frames++;

			z80_timer_acumulado=z80_timer_difftime-timer_interrupt_time;

			timer_reset();


                	if (MACHINE_IS_Z88) {
                        	conta_envio_audio++;
	                        if (conta_envio_audio>=4) {
        	                        envio_audio();
                	                conta_envio_audio=0;
                        	}
	                }

        	        else envio_audio();


			if (z80_timer_difftime>timer_warn_high_interrupt_time) {
				debug_printf(VERBOSE_INFO,"z80 interrupt (timer) time more than %d micros : %d",timer_warn_high_interrupt_time,z80_timer_difftime);

				//parar temporalmente el thread de sonido para que se vuelva a resincronizar todo
                                audio_playing.v=0;

				//ajustar contador de tiempo. Si por ejemplo se ha suspendido el proceso con ctrl+z y
				//se vuelve aqui, este acumulado es muy grande y no baja nunca a 0, y por tanto deja de emular correctamente
				z80_timer_acumulado=0;


			}
			else if (z80_timer_difftime<timer_warn_low_interrupt_time) {
				debug_printf(VERBOSE_INFO,"z80 interrupt (sound) time less than %d micros : %d",timer_warn_low_interrupt_time,z80_timer_difftime);
				//parar temporalmente el thread de sonido para que se vuelva a resincronizar todo
                                audio_playing.v=0;
			}

			return 1;
		//}

        }
	return 0;
}

struct timeval antes, ahora;


int get_timer_check_interrupt(void)
{
    int si_saltado_interrupcion;

    //Timer de tipo Date
    if (timer_selected==TIMER_DATE) {
        si_saltado_interrupcion=timer_check_interrupt_no_thread();
    }


    //Timer que salta con hilo. TIMER_USLEEP, TIMER_SDL, TIMER_MAC
    else {
	    si_saltado_interrupcion=timer_check_interrupt_thread();
    }


    return si_saltado_interrupcion;

}

//Ver si hay que generar interrupcion 1/50
//bucle principal de ejecucion de la cpu de spectrum
void timer_check_interrupt(void)
{
    //printf("timer_check_interrupt\n");
	int si_saltado_interrupcion=get_timer_check_interrupt();


	if (timer_condicion_top_speed() ) {
		interrupcion_timer_generada.v=1;
	}

    if (si_saltado_interrupcion ) {


        //printf ("despues timer_check_interrupt_thread. framedrop_total %d\n",framedrop_total);



        interrupcion_timer_generada.v=1;
        interrupcion_fifty_generada.v=1;

        contador_20ms +=timer_sleep_machine;

        //printf ("contador_20ms: %d\n",contador_20ms);

        if (contador_20ms<20000) return;

        //Cosas que suceden cada 20 ms aproximadamente
        /*digo aproximadamente porque:
        En spectrum, con cpu 100%, se incrementa en intervalos de 20000, y por tanto, cada vez viene aqui
        En Z88, con cpu 100%, se incrementa en intervalos de 5000, y aqui se entrara 1 de cada 4 veces
        Con cpu no 100%, puede que no sea exacto, y este contador por ejemplo salte a 23000 y luego se resetee a 0,
        "pierdiendose" esos 3000 us extra. Como lo que hay aqui debajo no tiene que ser completamente exacto,
        no importa mucho
        Esto afecta bastante cuando cpu speed < 100% . en ese caso, los sleeps son mayores de 20000, es cuando se
        pierde ese extra. Entonces con cpu speed < 100%, esto se llama menos veces que cada 20 ms
        */


        //printf ("20ms. contador_segundo: %d\n",contador_segundo);

        contador_20ms=0;

        //han pasado 20 ms
        contador_segundo +=20;

        contador_segundo_infinito +=20;

        //emulacion refresco memoria. Decrementar contador, de alguna manera "diciendo" que registro R funciona bien
        if (machine_emulate_memory_refresh) {
                //Si maximo es 1000, esto se resetea en 20 segundos aprox
                machine_emulate_memory_refresh_counter -=MAX_EMULATE_MEMORY_REFRESH_COUNTER/20/50;
                if (machine_emulate_memory_refresh_counter<0) machine_emulate_memory_refresh_counter=0;

                //Hacer debug de esto de vez en cuando
                if (machine_emulate_memory_refresh_counter!=0) {
                        //cada segundo
                        if (contador_segundo>=1000) {
                                machine_emulate_memory_refresh_debug_counter();
                        }
                }
        }



        //temp forzar framedrop
        //esperando_tiempo_final_t_estados.v=0;


        //if (framedrop_total>30) printf ("--------------framedrop total %d\n",framedrop_total);

        //if (esperando_tiempo_final_t_estados.v==0 && framedrop_total<40) {

            //Antes el 48 era 40
        if (esperando_tiempo_final_t_estados.v==0 && framedrop_total<48) {

            //normal
            framescreen_saltar++;

            //printf ("Interrupcion con framedrop\n");
        }
        else {
            framescreen_saltar=0;
            //printf ("Interrupcion sin framedrop\n");
        }

        //On Screen keyboard
        if (timer_on_screen_key) {
            timer_on_screen_key--;
            //Si llega a 0, liberar tecla
            if (timer_on_screen_key==0) {
                debug_printf (VERBOSE_DEBUG,"Releasing all keys so one was pressed from OSD keyboard");
                reset_keyboard_ports();

                //Si hay que volver a menu
                if (menu_button_osdkeyboard_return.v) {
                    menu_button_osdkeyboard_return.v=0;
                    menu_button_osdkeyboard.v=1;
                    menu_set_menu_abierto(1);
                }
            }

        }

        if (timer_osd_keyboard_menu) {
            timer_osd_keyboard_menu--;
        }

        if (cpc_pending_last_drawn_lines_black_counter) {
            cpc_pending_last_drawn_lines_black_counter--;
        }


        if (timer_on_screen_adv_key) {
            timer_on_screen_adv_key--;

            //Si llega a 25, es ese medio segundo sin pulsar tecla
            if (timer_on_screen_adv_key==adventure_keyboard_key_length/2) {
                reset_keyboard_ports();
            }

                            //Si llega a 0, volver a menu
            if (timer_on_screen_adv_key==0) {
                //Hay que volver a menu
                menu_button_osd_adv_keyboard_return.v=1;
                                    menu_set_menu_abierto(1);
            }
        }

        //joystick autofire
        if (joystick_autofire_frequency!=0) {
            joystick_autofire_counter++;
            if (joystick_autofire_counter>=joystick_autofire_frequency) {
                joystick_autofire_counter=0;

                //si estamos en menu, no tocar disparador automatico
                if (!menu_abierto) puerto_especial_joystick ^=16;
            }
        }

        //joystick auto left right
        if (joystick_autoleftright_enabled) {
            joystick_autoleftright_counter++;
            if (joystick_autoleftright_counter>=joystick_autoleftright_frequency) {
                joystick_autoleftright_counter=0;

                joystick_autoleftright_status++;
                if (joystick_autoleftright_status>1) joystick_autoleftright_status=0;

                //si estamos en menu, no tocar estado joystick
                if (!menu_abierto) {
                    if (joystick_autoleftright_status==0) {
                        puerto_especial_joystick=1;
                        menu_footer_activity("LEFT ");
                    }
                    if (joystick_autoleftright_status==1) {
                        puerto_especial_joystick=2;
                        menu_footer_activity("RIGHT");
                    }
                }
            }
        }

        if (z88_pendiente_cerrar_tapa_timer) {
            z88_pendiente_cerrar_tapa_timer--;
            if (z88_pendiente_cerrar_tapa_timer==0) z88_close_flap_ahora();
        }

        //Input file keyboard
        if (input_file_keyboard_is_playing() ) {
            input_file_keyboard_delay_counter++;
            if (input_file_keyboard_delay_counter>=input_file_keyboard_delay) {
                input_file_keyboard_delay_counter=0;
                input_file_keyboard_pending_next.v=1;

                input_file_keyboard_is_pause.v ^=1;

            }
        }

        if (ql_mantenido_pulsada_tecla) {
            if (ql_mantenido_pulsada_tecla_timer<50) ql_mantenido_pulsada_tecla_timer++;
        }

        //Temporizador de cancion ay playing
        ay_player_playing_timer();

        menu_window_splash_counter_ms +=20;

        //decrementar contador pausa cinta
        if (tape_pause!=0) tape_pause--;

        //contador de doble click de raton
        menu_mouse_left_double_click_counter++;

        if (menu_contador_teclas_repeticion) {
            menu_contador_teclas_repeticion--;
        }

        else {
            //es 0. hay repeticion
            if (menu_segundo_contador_teclas_repeticion) {
                                menu_segundo_contador_teclas_repeticion--;
            }
        }

        //temporizador cuando se esta limitando el uso de tecla de abrir menu
        if (menu_limit_menu_open.v) {
            util_if_open_just_menu_counter++;
            //Si ha pasado 2 segundos entre la primera pulsacion y ahora, resetear todo.
            //Si no hiciera esto, y por ejemplo pulsase dos veces en un momento, sin mas,
            //luego al pulsar 3 seguidas, no se abriria el menu, porque contaria 2 primeras fuera de tiempo (mas de un segundo) con diferencia de la primera,
            //y luego las 2 siguientes. Por lo que en ese supuesto caso tendria que pulsar 3x2=6 veces para poder abrir el menu, y aun asi, tendria siempre dos mas
            //fuera de secuencia que me perjudicaria
            if (util_if_open_just_menu_times) {
                int diferencia=util_if_open_just_menu_counter-util_if_open_just_menu_initial_counter;
                if (diferencia>50*2) {
                    debug_printf(VERBOSE_DEBUG,"Timeout pressing menu key. Reset to 0 times");
                    util_if_open_just_menu_times=0;
                }
            }
        }


        //temporizador para movimiento de mouse en menu
        menu_mouse_frame_counter++;

        //Si hay texto ahi acumulado pero no se ha recibido salto de linea, al cabo de un segundo, saltar
        if (textspeech_filter_program!=NULL) {
	        scrtextspeech_filter_counter++;
            //printf ("scrtextspeech_filter_counter: %d\n",scrtextspeech_filter_counter);

            //al cabo de X segundos

            if (textspeech_timeout_no_enter>0) {

                    if (scrtextspeech_filter_counter>=50*textspeech_timeout_no_enter && index_buffer_speech!=0) {
                            debug_printf (VERBOSE_DEBUG,"Forcing sending filter text although there is no carriage return");
                            textspeech_add_speech_fifo();
                    }

            }
	    }


        if (textspeech_filter_program!=NULL) {

            //Si hay pendiente speech
            //Si hay finalizado el proceso hijo
            //printf ("esperar\n");
            if (textspeech_finalizado_hijo_speech() ) {

                //recoger texto de salida
                textspeech_get_stdout_childs();

                //printf ("desde timer\n");
                scrtextspeech_filter_run_pending();
            }

            //if (textspeech_get_stdout_childs()) {
                //printf("processed stdout from timer\n");
            //}
        }

        //Ocultación/activación de botón switch zx desktop
        zxdesktop_switchdesktop_timer_event();

        //Ocultación/activación topbar
        topbar_timer_event();

        pd765_handle_speed_motor();

        hilow_timer_events();

        //para que la rotacion sea constante en dicha ventana
        //si hiciera la gestion de rotacion desde esa ventana sucederia
        //que si hay framedrop, rotaria mas lento
        menu_visualfloppy_increment_rotation();

        realtape_visual_cassete_timer();

        timer_watermark_device_activity_decrement_counter();

        textadv_location_timer_event();

        gamelife_timer_counter_event();

        //Cosas que suceden cada 1 segundo
        if (contador_segundo>=1000) {

            //printf ("segundo\n");

            contador_segundo=0;


            //Estadisticas de vsync por second
            last_vsync_per_second=vsync_per_second;
            vsync_per_second=0;

            //Temporizador para tooltip
            if (tooltip_enabled.v) menu_tooltip_counter++;

            //Temporizador para ventanas splash
            menu_window_splash_counter++;

            pd765_read_stats_update();
            pd765_write_stats_update();

            //resetear texto splash
            reset_welcome_message();



            //temporizador de carga de cinta para escribir texto loading en pantalla
            /*if (tape_loading_counter) {
                tape_loading_counter--;
                if (tape_loading_counter==0) {
                    delete_tape_text();
                }
            }*/




            //temporizador de impresion para escribir generico footer en pantalla
            if (generic_footertext_operating_counter) {
                    generic_footertext_operating_counter--;
                    if (generic_footertext_operating_counter==0) {
                            delete_generic_footertext();
                    }
            }

            //temporizacion para z88 que indica actividad de escritura en slot 3
            if (z88_footer_timer_slot3_activity_indicator) {
                z88_footer_timer_slot3_activity_indicator--;
                if (z88_footer_timer_slot3_activity_indicator==0) {
                    z88_reset_slot3_activity_indicator();
                }
            }



            //Temporizador para decir si se ha detectado real joystick
            //dado que si no hay joystick, por defecto está habilitado el bit de joystick presente, pero luego
            //el realjoystick_null_main lo pondrá a 0, desactivandolo
            //si llamamos a menu_tell_if_realjoystick_detected justo al arrancar ZEsarUX, no da tiempo a ejecutar
            //realjoystick_null_main, por tanto el joystick aun seguira presente al inicio, y se dirá erroneamente que esta presente,
            //cuando no lo esta
            if (menu_tell_if_realjoystick_detected_counter>0) {
                menu_tell_if_realjoystick_detected_counter--;
                if (menu_tell_if_realjoystick_detected_counter==0) menu_tell_if_realjoystick_detected();
            }




            //temporizador de Tape/snap options set en pantalla, primer mensaje
            if (tape_options_set_first_message_counter) {
                tape_options_set_first_message_counter--;
                if (tape_options_set_first_message_counter==0) {
                    delete_tape_options_set_first_message();
                }
            }

            //temporizador de Tape/snap options set en pantalla, segundo mensaje
            if (tape_options_set_second_message_counter) {
                    tape_options_set_second_message_counter--;
                    if (tape_options_set_second_message_counter==0) {
                            delete_tape_options_set_second_message();
                    }
            }

            //temporizador despues de pulsar rewind. Al cabo de 5 segundos se resetea la posicion
            snapshot_in_ram_rewind_timer();


            autosave_snapshot_at_fixed_interval();

            //escritura de contenido de EPROM/FLASH de Z88 a disco
            z88_flush_eprom_or_flash_to_disk();

            //escritura de contenido de SPI Flash de zxuno a disco
            zxuno_flush_flash_to_disk();


            //escritura de contenido de MMC a disco
            mmc_flush_flash_to_disk();

            //escritura de contenido de IDE a disco
            ide_flush_flash_to_disk();

            //escritura de contenido de TRD a disco
            trd_flush_contents_to_disk();

            //escritura de contenido de DSK a disco
            dskplusthree_flush_contents_to_disk();

            //escritura de contenido de HiLow a disco
            hilow_flush_contents_to_disk();


            //escritura de contenido de microdrive a disco
            microdrive_flush_to_disk();

            rzx_print_footer();

            realtape_print_footer();

            external_audio_source_print_footer();

            cf2_floppy_icon_activity();

            menu_footer_f5_menu_timer();

            if (betadisk_simulated_motor>0) betadisk_simulated_motor--;

            if (MACHINE_IS_PCW) pcw_boot_timer_handle();

            timer_zeng_online_server();

            zec_veces_audio_no_recibido_timer();

            timer_device_activity_change_watermark_colors();

            timer_get_cpu_use();


            //escritura de contenido de flash de superupgrade a disco
            superupgrade_flush_flash_to_disk();


            //salir del emulador despues de x segundos
            if (exit_emulator_after_seconds) {
                exit_emulator_after_seconds_counter++;
                if (exit_emulator_after_seconds_counter>=exit_emulator_after_seconds) {
                    debug_printf(VERBOSE_INFO,"Exiting emulator after %d seconds",exit_emulator_after_seconds);
                    end_emulator_autosave_snapshot();
                }
            }


        }



    }


}


int timer_get_uptime_seconds(void)
{

	struct timeval ahora;

	long z80_uptime_difftime, z80_uptime_seconds, z80_uptime_useconds;

	long z80_uptime_total_seconds;

	gettimeofday(&ahora, NULL);


				z80_uptime_seconds  = ahora.tv_sec  - zesarux_start_time.tv_sec;
				z80_uptime_useconds = ahora.tv_usec - zesarux_start_time.tv_usec;

				z80_uptime_difftime = ((z80_uptime_seconds) * 1000000 + z80_uptime_useconds);

		//printf ("useconds: %ld\n",z80_uptime_difftime);

	z80_uptime_total_seconds=z80_uptime_difftime/1000000;

	//printf ("seconds: %ld\n",z80_uptime_total_seconds);

	//Si por algo el valor es negativo (porque no haya segundos iniciales por ejemplo), retornar 0
	if (z80_uptime_total_seconds<0) z80_uptime_total_seconds=0;

	return z80_uptime_total_seconds;

}


void timer_get_texto_time(struct timeval *tv, char *texto)
{


	struct tm* ptm;


	/* Obtain the time of day, and convert it to a tm struct. */
	//gettimeofday (&tv, NULL);

	//convert it to a tm struc
	ptm = localtime (&tv->tv_sec);
	/* Format the date and time, down to a single second. */
	char time_string[40];

	strftime (texto, sizeof(time_string), "%H:%M:%S", ptm);

}


long timer_get_elapsed_seconds_since_first_version(void)
{

        struct timeval ahora;

        long z80_total_seconds;



	//24th September 2013
	long first_version=1379973600;

        gettimeofday(&ahora, NULL);


        z80_total_seconds  = ahora.tv_sec  - first_version;

	//printf ("segundos desde creacion: %ld\n",z80_uptime_seconds);

	return z80_total_seconds;

}


//Devolver tiempo invertido en ZEsarUX. De media 2 horas por dia
int timer_get_worked_time(void)
{
	long total_segundos=timer_get_elapsed_seconds_since_first_version();

	//Pasar a dias
	long total_dias=total_segundos/60/60/24;

	//Dos horas al dia
	int total_tiempo_invertido=total_dias*2;

	return total_tiempo_invertido;

}

void timer_toggle_top_speed_timer(void)
{
	top_speed_timer.v ^=1;
}



void timer_get_elapsed_core_frame_pre(void)
{
	//Para calcular lo que se tarda en ejecutar todo un frame
	timer_stats_current_time(&core_cpu_timer_frame_antes);
}



void timer_get_elapsed_core_frame_post(void)
{
	core_cpu_timer_frame_difftime=timer_stats_diference_time(&core_cpu_timer_frame_antes,&core_cpu_timer_frame_despues);

	//printf ("tiempo transcurrido: %ld microsec\n",tiempo_timer_difftime);
	//media de tiempo
	core_cpu_timer_frame_media=(core_cpu_timer_frame_media+core_cpu_timer_frame_difftime)/2;
	//printf ("tiempo medio transcurrido: %ld microsec\n",core_cpu_timer_frame_media);
}
