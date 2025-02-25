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

#if defined(__APPLE__)
        #include <SDL.h>
#else
	#include <SDL/SDL.h>
#endif


#include "common_sdl.h"
#include "cpu.h"
#include "debug.h"
#include "timer.h"

z80_bit audiosdl_inicializado={0};
z80_bit scrsdl_inicializado={0};

Uint32 commonsdl_timer_callback( Uint32 interval, void* param )
{

    timer_trigger_interrupt();

    //Retornar mismo intervalo para decir que queremos generar la interrupcion de nuevo

    //return interval;
    int return_intervalo=timer_sleep_machine/1000;

    //en condiciones normales, la mayoria tiene interrupción cada 20ms (20000 microsec) y Z88 cada 5 ms
    //Si en cambio alteramos cpu speed, este valor se altera y podria llegar a ser 0
    //Retornar 0 desde este callback significaria no volver a llamar al callback, y para evitar eso, retornamos 1
    if (return_intervalo==0) return_intervalo=1;

    //printf("Called Timer callback. interval called=%d return interval=%d\n",interval,return_intervalo);


    //SDL no permite timer < 10 ms
    //Aqui se saltaria en caso de que se tenga una maquina con timer >10ms, por ejemplo Spectrum,
    //y se cambie a otra maquina con timer <10ms (5ms, Z88) o tambien subiendo el cpu speed > 200%
    //En este caso desactivamos este timer de SDL y cualquier otro timer basado en threads, por lo que usara el de date
    //Nota: Pero en linux se supone que deberia ir bien usar un timer de threads con tiempos <10ms
    //lo que deberiamos hacer es denegar el timer de sdl, pero en windows si que deberia saltar a usar un timer de no threads
    if (timer_sleep_machine<10000) {
        debug_printf(VERBOSE_INFO,"SDL callback pretends to call at %d microsec but minimum is 10000. Set a non-sdl timer",timer_sleep_machine);
        start_timer();
        //Devolvemos 0 para desactivar el timer de sdl
        return 0;
    }


    return return_intervalo;
}

SDL_TimerID timerID;

//Retorna 0 si error. No 0 si ok
int commonsdl_init_timer_continue(void)
{
    int interval_ms=timer_sleep_machine/1000;

    timerID = SDL_AddTimer( interval_ms, commonsdl_timer_callback, NULL );
    if (timerID==NULL) {
        //Error
        return 0;
    }

    else {
        return 1;
    }
}

int commonsdl_init_timer(void)
{

    debug_printf(VERBOSE_INFO,"Initializing timer SDL for %d ms",timer_sleep_machine/10000);

    //SDL no permite timer < 10 ms
    if (timer_sleep_machine<10000) {
        debug_printf(VERBOSE_INFO,"SDL callback pretends to call at %d microsec but minimum is 10000. Can't set SDL timer",timer_sleep_machine);
        return 0;
    }


    int retorno=commonsdl_init_timer_continue();
    if (!retorno) {
        debug_printf(VERBOSE_INFO,"Error starting SDL timer");
        return 0;
    }
    else {
        //Ok inicializado
        return 1;
    }



}

void commonsdl_stop_timer(void)
{
    debug_printf(VERBOSE_INFO,"Stopping timer SDL");
    if (timerID!=NULL) {
        SDL_RemoveTimer(timerID);
    }
}

int commonsdl_init(void)
{

	//se debe poner a 1 (audiosdl_inicializado.v o scrsdl_inicializado.v) justo despues de llamar aqui

	//Ya hay algun driver inicializado. Salir sin hacer nada
	if (audiosdl_inicializado.v || scrsdl_inicializado.v) return 0;

	debug_printf (VERBOSE_DEBUG,"Calling SDL_Init");

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)<0) {
		debug_printf (VERBOSE_INFO,"Error SDL message: %s",SDL_GetError() );
		return 1;
	}

	return 0;

}

void commonsdl_end(void)
{
	//se debe poner a 0 (audiosdl_inicializado.v o scrsdl_inicializado.v) justo antes de llamar aqui

	//Aun hay algun driver funcionando. Volvemos sin hacer nada
	if (audiosdl_inicializado.v || scrsdl_inicializado.v) return;

	debug_printf (VERBOSE_DEBUG,"Calling SDL_Quit");

	SDL_Quit();
}
