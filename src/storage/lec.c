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

#include "lec.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"



z80_bit lec_enabled={0};


z80_byte *lec_ram_memory_pointer;


//Bloques de 32kb
//En lec80, dos bloques
//En lec272, 8 bloques
//En lec528, 16 bloques
z80_byte *lec_ram_memory_table[16];


//Para segmento 0000 y 8000h
z80_byte *lec_memory_page2[4];

z80_byte lec_port_fd=0;

int lec_nested_id_peek_byte;
int lec_nested_id_peek_byte_no_time;
int lec_nested_id_poke_byte;
int lec_nested_id_poke_byte_no_time;




void lec_common_poke(z80_int dir,z80_byte valor)
{


	int segmento;
	z80_byte *puntero;
        if (dir>16383) {
                segmento=dir / 16384;
                dir = dir & 16383;
                puntero=lec_memory_paged[segmento]+dir;

                *puntero=valor;
        }
}


z80_byte lec_poke_byte_no_time(z80_int dir,z80_byte valor)
{

  lec_common_poke(dir,valor);
	debug_nested_poke_byte_no_time_call_previous(lec_nested_id_poke_byte_no_time,dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}




z80_byte lec_poke_byte(z80_int dir,z80_byte valor)
{

  lec_common_poke(dir,valor);
	debug_nested_poke_byte_call_previous(lec_nested_id_poke_byte,dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}


z80_byte lec_peek_byte_no_time(z80_int dir_orig,z80_byte value GCC_UNUSED)
{

      //Si se muestra ROM interna en vez de pagina de lec
      if (dir_orig<16384 && si_lec_muestra_rom_interna() ) {
        return debug_nested_peek_byte_no_time_call_previous(lec_nested_id_peek_byte_no_time,dir_orig);
      }

        int segmento;
        z80_byte *puntero;
	z80_int dir=dir_orig;
        segmento=dir / 16384;

        dir = dir & 16383;
        puntero=lec_memory_paged[segmento]+dir;

	//Aunque no usamos el valor de peek, llamamos para realizar contienda, llamar a otras funciones anidadas, etc
	//lec_original_peek_byte_no_time(dir_orig);
	debug_nested_peek_byte_no_time_call_previous(lec_nested_id_peek_byte_no_time,dir);

        return *puntero;

}

z80_byte lec_peek_byte(z80_int dir_orig,z80_byte value GCC_UNUSED)
{

        //Si se muestra ROM interna en vez de pagina de lec
        if (dir_orig<16384 && si_lec_muestra_rom_interna() ) {
          return debug_nested_peek_byte_call_previous(lec_nested_id_peek_byte,dir_orig);
        }

        int segmento;
        z80_byte *puntero;
	z80_int dir=dir_orig;
        segmento=dir / 16384;

        dir = dir & 16383;
        puntero=lec_memory_paged[segmento]+dir;

	//Aunque no usamos el valor de peek, llamamos para realizar contienda, llamar a otras funciones anidadas, etc
	//lec_original_peek_byte(dir_orig);
	debug_nested_peek_byte_call_previous(lec_nested_id_peek_byte,dir);

        return *puntero;

}




//Establecer rutinas propias
void lec_set_peek_poke_functions(void)
{
        debug_printf (VERBOSE_DEBUG,"Setting lec poke / peek functions");


        lec_nested_id_poke_byte=debug_nested_poke_byte_add(lec_poke_byte,"lec poke_byte");
        lec_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(lec_poke_byte_no_time,"lec poke_byte_no_time");
        lec_nested_id_peek_byte=debug_nested_peek_byte_add(lec_peek_byte,"lec peek_byte");
        lec_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(lec_peek_byte_no_time,"lec peek_byte_no_time");

}

//Restaurar rutinas de lec
void lec_restore_peek_poke_functions(void)
{
        debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before lec");

        debug_nested_poke_byte_del(lec_nested_id_poke_byte);
        debug_nested_poke_byte_no_time_del(lec_nested_id_poke_byte_no_time);
        debug_nested_peek_byte_del(lec_nested_id_peek_byte);
        debug_nested_peek_byte_no_time_del(lec_nested_id_peek_byte_no_time);

}



void lec_alloc_memory(void)
{
    int size=LEC_MAX_RAM_SIZE:

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for lec emulation",size/1024);

    lec_ram_memory_pointer=util_malloc(size,"No enough memory for lec emulation");



}



void lec_init_memory_tables(void)
{

	int pagina;

	for (pagina=0;pagina<32;pagina++) {
		lec_rom_memory_table[pagina]=&lec_rom_memory_pointer[16384*pagina];
		lec_ram_memory_table[pagina]=&lec_ram_memory_pointer[16384*pagina];
	}
}



z80_byte lec_get_ram_bank(void)
{
	z80_byte banco;

	//Maquinas de 128k solo soporta 128kb de RAM
	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) banco=(puerto_32765&7);

	else banco=(puerto_32765&7)+((puerto_32765>>3)&24);

	return banco;
}

void lec_set_memory_pages(void)
{
	z80_byte rom_page=lec_get_rom_bank();
	z80_byte ram_page=lec_get_ram_bank();

	lec_memory_paged[0]=lec_rom_memory_table[rom_page];

	lec_memory_paged[1]=lec_ram_memory_table[5];
	lec_memory_paged[2]=lec_ram_memory_table[2];
	lec_memory_paged[3]=lec_ram_memory_table[ram_page];





}

void lec_reset(void)
{
    /*
	lec_puerto_43b=0;
  lec_flash_write_protected.v=1;
  lec_write_status=0;
  lec_write_buffer_index=0;
  lec_pending_protect_flash.v=0;
  */
 lec_port_fd=0;

	lec_set_memory_pages();
}



void lec_enable(void)
{
  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable lec on non Spectrum machine");
    return;
  }

	if (lec_enabled.v) return;



	lec_alloc_memory();
	lec_init_memory_tables();
	lec_set_memory_pages();

	lec_set_peek_poke_functions();

	lec_enabled.v=1;

}

void lec_disable(void)
{
	if (lec_enabled.v==0) return;

	lec_restore_peek_poke_functions();

	free(lec_ram_memory_pointer);

	lec_enabled.v=0;



}

