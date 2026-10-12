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
#include <time.h>

#include "cpu.h"
#include "didaktikgama.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "zxvision.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"
#include "start.h"




//Donde esta cada pagina de ram
z80_byte *didaktikgama_ram_mem_table[DIDAKTIKGAMA_TOTAL_RAM_PAGES];


//Direcciones actuales mapeadas
z80_byte *didaktikgama_memory_paged[4];
static z80_bit didaktikgama_trdos_active={0};

z80_byte didaktikgama_ram_paged=0;

void didaktikgama_init_memory_tables(void)
{
    //Primero rom y luego ram

    int i;
    z80_byte *puntero=memoria_spectrum;

    //saltar la rom
    puntero +=16384;

    //saltamos la ram 16384-32767
    puntero +=16384;


    for (i=0;i<DIDAKTIKGAMA_TOTAL_RAM_PAGES;i++) {
        didaktikgama_ram_mem_table[i]=puntero;
        puntero +=32768;
    }


}

void didaktikgama_set_normal_pages(void)
{
    didaktikgama_trdos_active.v=0;
    didaktikgama_memory_paged[0]=memoria_spectrum;
    didaktikgama_memory_paged[1]=&memoria_spectrum[16384];
    didaktikgama_memory_paged[2]=didaktikgama_ram_mem_table[0];
    didaktikgama_memory_paged[3]=didaktikgama_ram_mem_table[0]+16384;

}

void didaktikgama_malloc_mem_machine(void)
{
    //16 KB ROM, 80 KB RAM

    malloc_machine((16+DIDAKTIKGAMA_RAM_SIZE)*1024);
    random_ram(memoria_spectrum+DIDAKTIKGAMA_ROM_SIZE*1024,DIDAKTIKGAMA_RAM_SIZE*1024);

    didaktikgama_init_memory_tables();
    didaktikgama_set_normal_pages();
}

void didaktikgama_mem_page_ram(void)
{

    z80_byte page_entra=didaktikgama_ram_paged;

    didaktikgama_memory_paged[2]=didaktikgama_ram_mem_table[page_entra];
    didaktikgama_memory_paged[3]=didaktikgama_ram_mem_table[page_entra]+16384;

}

void didaktikgama_reset(void)
{
    //TODO reset puertos

    didaktikgama_ram_paged=0;
    didaktikgama_set_normal_pages();

}





void poke_byte_no_time_didaktikgama(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (dir>16383) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
        dir = dir & 16383;
        puntero=didaktikgama_memory_paged[segmento]+dir;

        *puntero=valor;
    }
}


void poke_byte_didaktikgama(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

#ifdef EMULATE_CONTEND
    if (contend_pages_actual[segmento]) {
        //printf ("t_estados: %d\n",t_estados);
        t_estados += contend_table[ t_estados ];
    }
#endif
    t_estados += 3;



    if (dir>16383) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
		dir = dir & 16383;
		puntero=didaktikgama_memory_paged[segmento]+dir;

//		printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
		*puntero=valor;
	}
}


z80_byte peek_byte_no_time_didaktikgama(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

	dir = dir & 16383;
	puntero=didaktikgama_memory_paged[segmento]+dir;

	return *puntero;
}


z80_byte peek_byte_didaktikgama(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

#ifdef EMULATE_CONTEND
    if (contend_pages_actual[segmento]) {
            t_estados += contend_table[ t_estados ];
    }
#endif
    t_estados += 3;



	dir = dir & 16383;
	puntero=didaktikgama_memory_paged[segmento]+dir;

	return *puntero;
}