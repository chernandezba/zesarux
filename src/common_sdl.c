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

z80_bit audiosdl_inicializado={0};
z80_bit scrsdl_inicializado={0};

int commonsdl_init(void)
{

	//se debe poner a 1 (audiosdl_inicializado.v o scrsdl_inicializado.v) justo despues de llamar aqui

	//Ya hay algun driver inicializado. Salir sin hacer nada
	if (audiosdl_inicializado.v || scrsdl_inicializado.v) return 0;

	debug_printf (VERBOSE_DEBUG,"Calling SDL_Init");

        if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0) {
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
