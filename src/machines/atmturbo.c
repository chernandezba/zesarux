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
#include "atmturbo.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "zxvision.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"
#include "start.h"
#include "betadisk.h"



/*

ATM Turbo 1:

RAM 512 KB

En la ATM Turbo 1 v4.10–4.50, la RAM se pagina con dos puertos:
- #7FFD (32765): los bits 0–2 eligen una de las 8 páginas de 16 KB dentro del banco.
- #FDFD (65021): los bits 0–1 eligen uno de los 4 bancos de RAM.
Así, en modo Spectrum, el número de página visible en #C000–#FFFF es (#FDFD & 3) × 8 + (#7FFD & 7):
páginas 0–31, que suman 512 KB. #4000–#7FFF suele tener la página 5 y #8000–#BFFF la 2. El bit 5 de #7FFD bloquea nuevas escrituras a ese puerto. Manual técnico de ATM Turbo 1, placas v4.10–4.50


ROM 64 KB

| Página | Contenido | Selección |
|---|---|---|
| ROM 0 | BIOS de CP/M | Modo CP/M sistema |
| ROM 1 | TR-DOS | Al entrar en TR-DOS |
| ROM 2 | BASIC 128 | Bit 4 de `#7FFD` = 1 |
| ROM 3 | BASIC 48 | Bit 4 de `#7FFD` = 0 |

El bit 2 de #FDFD selecciona otro banco de 64 KB destinado a ROM disk;
no selecciona una de esas cuatro páginas. La documentación describe también que una lectura de #FB
activa la ROM BIOS de CP/M y una lectura de #7B vuelve a la ROM Spectrum.

*/


//Donde esta cada pagina de ram
z80_byte *atmturbo_ram_mem_table[ATMTURBO_TOTAL_RAM_PAGES];

//Donde esta cada pagina de rom
z80_byte *atmturbo_rom_mem_table[ATMTURBO_TOTAL_ROM_PAGES];

//Direcciones actuales mapeadas
z80_byte *atmturbo_memory_paged[4];

z80_byte puerto_fdfd;

int atmturbo_rombios=0;


void atmturbo_init_memory_tables(void)
{
    //Primero rom y luego ram

    int i;
    z80_byte *puntero=memoria_spectrum;

    for (i=0;i<ATMTURBO_TOTAL_ROM_PAGES;i++) {
        atmturbo_rom_mem_table[i]=puntero;
        puntero +=16384;
    }

    for (i=0;i<ATMTURBO_TOTAL_RAM_PAGES;i++) {
        atmturbo_ram_mem_table[i]=puntero;
        puntero +=16384;
    }


}

void atmturbo_set_normal_pages(void)
{

    atmturbo_memory_paged[0]=atmturbo_rom_mem_table[0];
    atmturbo_memory_paged[1]=atmturbo_ram_mem_table[5];
    atmturbo_memory_paged[2]=atmturbo_ram_mem_table[2];
    atmturbo_memory_paged[3]=atmturbo_ram_mem_table[0];
    debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM;
    debug_paginas_memoria_mapeadas[1]=5;
    debug_paginas_memoria_mapeadas[2]=2;
    debug_paginas_memoria_mapeadas[3]=0;

}


void atmturbo_malloc_mem_machine(void)
{


    int total_ram=(ATMTURBO_ROM_SIZE+ATMTURBO_RAM_SIZE)*1024;

    malloc_machine(total_ram);


    atmturbo_init_memory_tables();
    atmturbo_set_normal_pages();
}

void atmturbo_mem_page_ram(void)
{

    z80_byte page_entra=puerto_32765 & 7;
    z80_byte page_fdfd=puerto_fdfd &3;

    int pagina=(page_fdfd)*8 + page_entra;


    atmturbo_memory_paged[3]=atmturbo_ram_mem_table[pagina];
    debug_paginas_memoria_mapeadas[3]=page_entra;
}

void atmturbo_mem_page_rom(void)
{

    z80_byte page_entra;

    /*

| Página | Desplazamiento en la ROM | Contenido | Selección |
|---|---:|---|---|
| 0 | `0000h` | BASIC 128 | `1FFDh.D1=0`, `7FFDh.D4=0` |
| 1 | `4000h` | BASIC 48 | `1FFDh.D1=0`, `7FFDh.D4=1` |
| 2 | `8000h` | Monitor de servicio | `1FFDh.D1=1` |
| 3 | `C000h` | TR-DOS | Se activa mediante la lógica de TR-DOS, no mediante la selección BASIC de `7FFDh`. |
    */

    page_entra=(puerto_32765 & 16) >> 4;

    if (atmturbo_rombios==0) page_entra |=2;

    atmturbo_memory_paged[0]=atmturbo_rom_mem_table[page_entra];
    debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+page_entra;

}

void atmturbo_out_port_32765(z80_byte value)
{
    puerto_32765=value;

    atmturbo_mem_page_ram();
    atmturbo_mem_page_rom();
}

void atmturbo_out_port_fdfd(z80_byte value)
{
    puerto_fdfd=value;

    atmturbo_mem_page_ram();
    atmturbo_mem_page_rom();
}

void atmturbo_reset(void)
{
    atmturbo_rombios=1;

    atmturbo_mem_page_rom();
    atmturbo_mem_page_ram();
}



void poke_byte_no_time_atmturbo(z80_int dir,z80_byte valor)
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
        puntero=atmturbo_memory_paged[segmento]+dir;

        *puntero=valor;
    }
}


void poke_byte_atmturbo(z80_int dir,z80_byte valor)
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
		puntero=atmturbo_memory_paged[segmento]+dir;

		//printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
		*puntero=valor;
	}
}




z80_byte peek_byte_no_time_atmturbo(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

	dir = dir & 16383;
	puntero=atmturbo_memory_paged[segmento]+dir;

	return *puntero;
}


z80_byte peek_byte_atmturbo(z80_int dir)
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
	puntero=atmturbo_memory_paged[segmento]+dir;

	return *puntero;
}


z80_byte atmturbo_read_port_fb(void)
{
    atmturbo_rombios=1;

    atmturbo_mem_page_rom();

    return 255;

}

z80_byte atmturbo_read_port_7b(void)
{
    atmturbo_rombios=0;

    atmturbo_mem_page_rom();

    return 255;

}

