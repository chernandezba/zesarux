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

#include "cpu.h"
#include "chloe.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "timex.h"
#include "prism.h"


//Direcciones donde estan cada pagina de rom. 2 paginas de 16 kb
z80_byte *chloe_rom_mem_table[2];

//Direcciones donde estan cada pagina de ram home, en paginas de 16 kb
z80_byte *chloe_home_ram_mem_table[8];

//Direcciones donde estan cada pagina de ram ex, en paginas de 8 kb
z80_byte *chloe_ex_ram_mem_table[8];

//Direcciones donde estan cada pagina de ram dock, en paginas de 8 kb
z80_byte *chloe_dock_ram_mem_table[8];

//Direcciones actuales mapeadas, bloques de 8 kb
z80_byte *chloe_memory_paged[8];


//Tipos de memoria mapeadas
//0=rom
//1=home
//2=dock
//3=ex
//Constantes definidas en CHLOE_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX
z80_byte chloe_type_memory_paged[8];


//Paginas mapeadas en cada zona de RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
z80_byte debug_chloe_paginas_memoria_mapeadas[8];


void chloe_set_memory_pages(void)
{
	//timex_port_f4;
	//Cada bit determina si se mapea home(0) o dock/ex
	//timex_port_ff
	//Bit 7:    Selects which bank the horizontal MMU should use. 0=DOCK, 1=EX-ROM.

	/*
z80_byte puerto_32765=0;

El conmutador de hardware esta en la direccion de E/S 7FFDh (32765). El campo del bit
para esta direccion es el siguiente:
DO a D2         Seleccion de RAM
D3              Seleccion de pantalla
D4                      Seleccion de ROM
D5       Inhabilitacion de la paginacion
	*/


	//Paginas de ram entre 0 y 7
	//Paginas 6 y 7 vienen determinadas por byte de paginacion de 128 puerto 32765
	//En chloe 140 se, paginas son como en spectrum 128: rom0/1 ram 5 ram 2 ram_puerto_32765
	//En chloe 280 se, paginas son segun puertos de paginacion timex y ultima pagina de ram

	//Asignamos primero como en chloe 140 se. Luego para el 280se, cambiamos segun bits de paginacion timex
	z80_byte rom_entra=(puerto_32765 >>4)&1;

	z80_byte *puntero_rom=chloe_rom_mem_table[rom_entra];

	chloe_memory_paged[0]=puntero_rom;
	chloe_memory_paged[1]=&puntero_rom[8192];

	chloe_type_memory_paged[0]=CHLOE_MEMORY_TYPE_ROM;
	chloe_type_memory_paged[1]=CHLOE_MEMORY_TYPE_ROM;

	debug_chloe_paginas_memoria_mapeadas[0]=rom_entra;
	debug_chloe_paginas_memoria_mapeadas[1]=rom_entra;


	//Pagina 5
	z80_byte *puntero_ram5=chloe_home_ram_mem_table[5];	
	chloe_memory_paged[2]=puntero_ram5;
	chloe_memory_paged[3]=&puntero_ram5[8192];
        chloe_type_memory_paged[2]=CHLOE_MEMORY_TYPE_HOME;
        chloe_type_memory_paged[3]=CHLOE_MEMORY_TYPE_HOME;
	debug_chloe_paginas_memoria_mapeadas[2]=5;
	debug_chloe_paginas_memoria_mapeadas[3]=5;

	//Pagina 2
	z80_byte *puntero_ram2=chloe_home_ram_mem_table[2];	
	chloe_memory_paged[4]=puntero_ram2;
	chloe_memory_paged[5]=&puntero_ram2[8192];
        chloe_type_memory_paged[4]=CHLOE_MEMORY_TYPE_HOME;
        chloe_type_memory_paged[5]=CHLOE_MEMORY_TYPE_HOME;
	debug_chloe_paginas_memoria_mapeadas[4]=2;
	debug_chloe_paginas_memoria_mapeadas[5]=2;

	//Pagina segun puerto 32765
	z80_byte ram_entra=(puerto_32765)&7;
        z80_byte *puntero_ram_entra=chloe_home_ram_mem_table[ram_entra];
        chloe_memory_paged[6]=puntero_ram_entra;
        chloe_memory_paged[7]=&puntero_ram_entra[8192];
        chloe_type_memory_paged[6]=CHLOE_MEMORY_TYPE_HOME;
        chloe_type_memory_paged[7]=CHLOE_MEMORY_TYPE_HOME;
	debug_chloe_paginas_memoria_mapeadas[6]=ram_entra;
	debug_chloe_paginas_memoria_mapeadas[7]=ram_entra;

	//TODO. Estamos usando tablas de paginacion segun segmentos de 16kb
	//si en la ram alta metemos por ejemplo una pagina de ex y el ultimo trozo de 8 kb es la ram 5 de home,
	//todo el bloque de 16b tendra la misma contencion que pagina 5
	//Habria que considerar bloques de contend de 8 kb
	contend_pages_actual[3]=contend_pages_chloe[ram_entra];


	//Aplicamos segun paginacion CHLOE 280SE, y MMU de Chloe en zxuno
	if (MACHINE_IS_CHLOE_280SE || MACHINE_IS_ZXUNO) {
		int bloque_ram;
		z80_byte mascara_puerto_f4=1;

		for (bloque_ram=0;bloque_ram<8;bloque_ram++) {
			//Ver cada bit del puerto f4
			if (timex_port_f4 & mascara_puerto_f4) {
				//Se pagina ex o dock
			        //timex_port_ff
			        //Bit 7:    Selects which bank the horizontal MMU should use. 0=DOCK, 1=EX-ROM.
				z80_byte *puntero_memoria;
				if (timex_port_ff&128) {
					//EX
					puntero_memoria=chloe_ex_ram_mem_table[bloque_ram];
					chloe_type_memory_paged[bloque_ram]=CHLOE_MEMORY_TYPE_EX;
				}
				else {
					//DOCK
					puntero_memoria=chloe_dock_ram_mem_table[bloque_ram];
					chloe_type_memory_paged[bloque_ram]=CHLOE_MEMORY_TYPE_DOCK;
				}
				chloe_memory_paged[bloque_ram]=puntero_memoria;
				debug_chloe_paginas_memoria_mapeadas[bloque_ram]=bloque_ram;


				if (bloque_ram==6 || bloque_ram==7) {
				        //TODO. Estamos usando tablas de paginacion segun segmentos de 16kb
				        //si en la ram alta metemos por ejemplo una pagina de ex y el ultimo trozo de 8 kb es la ram 5 de home,
				        //todo el bloque de 16b tendra la misma contencion que pagina 5
				        //Habria que considerar bloques de contend de 8 kb
					//Entra ex o dock, por tanto no hay contend
				        contend_pages_actual[3]=0;
				}

			}

			mascara_puerto_f4=mascara_puerto_f4<<1;
		}
	}

	//printf ("fin chloe_set_memory_pages\n");

}


void chloe_init_memory_tables(void)
{
	debug_printf (VERBOSE_DEBUG,"Initializing Chloe memory pages");

	/*
//Direcciones donde estan cada pagina de rom. 2 paginas de 16 kb
//z80_byte *chloe_rom_mem_table[2];

//Direcciones donde estan cada pagina de ram home
//z80_byte *chloe_home_ram_mem_table[8];

//Direcciones donde estan cada pagina de ram ex
//z80_byte *chloe_ex_ram_mem_table[8];

//Direcciones donde estan cada pagina de ram dock
//z80_byte *chloe_dock_ram_mem_table[8];

//Direcciones actuales mapeadas, bloques de 8 kb
	*/

	//memoria_spectrum
	//32 kb rom
	//128kb home
	//64 kb ex
	//64 kb dock

	z80_byte *puntero;
	puntero=memoria_spectrum;

	chloe_rom_mem_table[0]=puntero;
	chloe_rom_mem_table[1]=&puntero[16384];
	puntero +=32768;

	int i;
	for (i=0;i<8;i++) {
		chloe_home_ram_mem_table[i]=puntero;
		puntero +=16384;
	}

	for (i=0;i<8;i++) {
		chloe_ex_ram_mem_table[i]=puntero;
		puntero +=8192;
	}

	for (i=0;i<8;i++) {
		chloe_dock_ram_mem_table[i]=puntero;
		puntero +=8192;
	}

	//Tablas contend
	contend_pages_actual[0]=0;
	contend_pages_actual[1]=contend_pages_chloe[5];
	contend_pages_actual[2]=contend_pages_chloe[2];
	contend_pages_actual[3]=contend_pages_chloe[0];



}


void chloe_out_ula2(z80_byte value)
{

        z80_byte ula2_register=(value>>4)&15;

        value=value&15;

	//Solo registro 0 para control del turbo
	if (ula2_register==0) {
		prism_ula2_registers[ula2_register]=value;
                prism_set_emulator_setting_cpuspeed();
	}
}

