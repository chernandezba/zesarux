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

    printf("Called Timer callback. interval called=%d return interval=%d\n",interval,return_intervalo);

    return return_intervalo;
}

int commonsdl_init_timer(void)
{
    SDL_TimerID timerID = SDL_AddTimer( timer_sleep_machine/1000, commonsdl_timer_callback, NULL );
    if (timerID==NULL) return 0;

    else return 1;
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
