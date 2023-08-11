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
#include <stdlib.h>
#include <dirent.h>

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "spritefinder.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "screen.h"


z80_bit spritefinder_enabled={0};





int spritefinder_nested_id_poke_byte;
int spritefinder_nested_id_poke_byte_no_time;
int spritefinder_nested_id_peek_byte;
int spritefinder_nested_id_peek_byte_no_time;


//Ultimas coordenadas detectadas del scanline anterior
int lastx=-1;
int lasty=-1;

//Indica cuantos scanlines consecutivos hemos leido
int scanlinesleidos=0;

z80_int spritefinder_posible_dir=0;


/*

Notas: busca rutina que escribe al menos dos scanlines consecutivos
Esto detecta muchos juegos cuando escriben cualquier cosa, otros no (aquellos que usan doble buffer posiblemente)
Ejemplo que van: sir fred, kings valley
Ejemplo que no va: paperboy y otros

Se podria intentar hacer que detectase el sprite del personaje como aquello que se sobreescribe al pulsar teclas (cuando se mueve)
Problema: hay juegos que escriben continuamente el personaje (como kings valley), otros (como sirfred) serviria

Entonces el problema se limita a ver que zona de pantalla se altera al pulsar una tecla (idealmente el personaje se mueve)
Esto necesitaria mucha memoria de buffer, o no?
De momento puede servir simplemente para saber desde qué rutina se está escribiendo en pantalla

*/

//Obtener coordenada x,y de una direccion de pantalla dada
//x entre 0..255 (aunque sera multiple de 8)
//y entre 0..191
void spritefinder_get_xy(z80_int dir,int *xdest,int *ydest)
{
	util_spectrumscreen_get_xy(dir,xdest,ydest);
	return;

	//De momento para ir rapido, buscamos direccion en array de scanline
	//screen_addr_table

	dir -=16384;

	int indice=0;
	int x,y;
	for (y=0;y<192;y++) {
		for (x=0;x<32;x++) {

			if (dir==screen_addr_table[indice]) {
				*xdest=x*8;
				*ydest=y;
				return;
			}

			indice++;
		}
	}

}

z80_byte temp_conta;

void spritefinder_handle_writing(z80_int dir,z80_byte valor)
{
	//Si se escribe en pantalla. De momento ignorar rutinas con doble buffer
	if (dir>=16384 && dir<=22527) {

		if (spritefinder_posible_dir==reg_pc) {
			//Alterar destino para ver si lo detecta bien
			debug_nested_poke_byte_no_time_call_previous(spritefinder_nested_id_poke_byte_no_time,dir,valor^temp_conta);
			temp_conta++;
		}

		//printf ("Writing on screen. Dir=%d value=%d\n",dir,valor);

		int x,y;

		//Obtener coordenada x,y para una direccion
		spritefinder_get_xy(dir,&x,&y);

		//printf ("x=%d y=%d\n",x,y);

		if (scanlinesleidos==0) {
			lastx=x;
			lasty=y;
			scanlinesleidos=1;
		}

		else if (scanlinesleidos==1) {
			//Ver si scanline actual es y+1 y la coordenada x es x, o x-8 o x+8
			//Eso seria para sprites de 16 pixeles de ancho
			//Damos hasta sprites de 24 (3 bytes)
			if (y==lasty+1) {
				if (x==lastx || x==lastx-8 || x==lastx-16 || x==lasty+8 || x==lasty+16) {
					scanlinesleidos=2;
				}
			}

			if (scanlinesleidos==2) {
				debug_printf (VERBOSE_DEBUG,"Detected possible scanline routine at address: %d",reg_pc);
				spritefinder_posible_dir=reg_pc;
			}

			scanlinesleidos=0; //Reiniciamos proceso

		}
	}
}


z80_byte spritefinder_poke_byte(z80_int dir,z80_byte valor)
{

	//spritefinder_original_poke_byte(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_call_previous(spritefinder_nested_id_poke_byte,dir,valor);

        spritefinder_handle_writing(dir,valor);


        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte spritefinder_poke_byte_no_time(z80_int dir,z80_byte valor)
{
        //spritefinder_original_poke_byte_no_time(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_no_time_call_previous(spritefinder_nested_id_poke_byte_no_time,dir,valor);

        spritefinder_handle_writing(dir,valor);


        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte spritefinder_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(spritefinder_nested_id_peek_byte,dir);

	return valor_leido;
}

z80_byte spritefinder_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(spritefinder_nested_id_peek_byte_no_time,dir);


	return valor_leido;
}



//Establecer rutinas propias
void spritefinder_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting spritefinder poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	spritefinder_nested_id_poke_byte=debug_nested_poke_byte_add(spritefinder_poke_byte,"Spritefinder poke_byte");
	spritefinder_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(spritefinder_poke_byte_no_time,"Spritefinder poke_byte_no_time");
	spritefinder_nested_id_peek_byte=debug_nested_peek_byte_add(spritefinder_peek_byte,"Spritefinder peek_byte");
	spritefinder_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(spritefinder_peek_byte_no_time,"Spritefinder peek_byte_no_time");

}

//Restaurar rutinas de spritefinder
void spritefinder_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before spritefinder");


	debug_nested_poke_byte_del(spritefinder_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(spritefinder_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(spritefinder_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(spritefinder_nested_id_peek_byte_no_time);
}





void spritefinder_enable(void)
{

  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable spritefinder on non Spectrum machine");
    return;
  }

	if (spritefinder_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"Already enabled");
		return;
	}


	spritefinder_set_peek_poke_functions();

	spritefinder_enabled.v=1;





}

void spritefinder_disable(void)
{
	if (spritefinder_enabled.v==0) return;

	spritefinder_restore_peek_poke_functions();


	spritefinder_enabled.v=0;
}



