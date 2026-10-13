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
#include "scorpion.h"
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

ZS-256	Paginación, ROM, monitor NMI y temporización Scorpion
ZS-256 Turbo+	CPU Turbo, Diferencias de temporización, puertos y lógica
Scorpion GMX	Nueva paginación, ROM ampliada y vídeo adicional

*/

/*

Scorpion ZS 256:

RAM 256 KB

| Puerto | Bits | Función |
|---|---|---|
| `7FFDh` | D0–D2 | Seleccionan una página de 0 a 7. |
| `1FFDh` | D4 | Añade 8 al número de página cuando vale 1. |


ROM 64 KB

| Página | Desplazamiento en la ROM | Contenido | Selección |
|---|---:|---|---|
| 0 | `0000h` | BASIC 128 | `1FFDh.D1=0`, `7FFDh.D4=0` |
| 1 | `4000h` | BASIC 48 | `1FFDh.D1=0`, `7FFDh.D4=1` |
| 2 | `8000h` | Monitor de servicio | `1FFDh.D1=1` |
| 3 | `C000h` | TR-DOS | Se activa mediante la lógica de TR-DOS, no mediante la selección BASIC de `7FFDh`. |

*/


//Donde esta cada pagina de ram
z80_byte *scorpion_ram_mem_table[SCORPIONGMX_TOTAL_RAM_PAGES];

//Donde esta cada pagina de rom
z80_byte *scorpion_rom_mem_table[SCORPIONGMX_TOTAL_ROM_PAGES];

//Direcciones actuales mapeadas
z80_byte *scorpion_memory_paged[4];
static z80_bit scorpion_trdos_active={0};

z80_byte scorpion_read_ay_port_a(z80_byte ay_value)
{
    // En Turbo+ las entradas 0-3 y 5 leen D0-D4 de 7FFD; la entrada 4 lee D4 de 1FFD.
    return (ay_value & 0xc0) | (puerto_32765 & 0x0f) |
           (puerto_8189 & 0x10) | ((puerto_32765 & 0x10) << 1);
}

void scorpion_init_memory_tables(void)
{
    //Primero rom y luego ram

    int i;
    z80_byte *puntero=memoria_spectrum;

    for (i=0;i<SCORPIONGMX_TOTAL_ROM_PAGES;i++) {
        scorpion_rom_mem_table[i]=puntero;
        puntero +=16384;
    }

    for (i=0;i<SCORPIONGMX_TOTAL_RAM_PAGES;i++) {
        scorpion_ram_mem_table[i]=puntero;
        puntero +=16384;
    }


}

void scorpion_set_normal_pages(void)
{
    scorpion_trdos_active.v=0;
    scorpion_memory_paged[0]=scorpion_rom_mem_table[0];
    scorpion_memory_paged[1]=scorpion_ram_mem_table[5];
    scorpion_memory_paged[2]=scorpion_ram_mem_table[2];
    scorpion_memory_paged[3]=scorpion_ram_mem_table[0];


    //TODO
    if (MACHINE_IS_SCORPION_GMX) {
    }

}

void scorpion_malloc_mem_machine(void)
{
    //64 KB ROM, 256 KB RAM en scorpion 256
    //512 KB ROM, 2048 KB RAM en scorpion gmx
    //asignamos el maximo

    int total_ram=(SCORPIONGMX_ROM_SIZE+SCORPIONGMX_RAM_SIZE)*1024;

    //malloc_machine((SCORPION_ROM_SIZE+SCORPION_RAM_SIZE)*1024);
    //random_ram(memoria_spectrum+SCORPION_ROM_SIZE*1024,SCORPION_RAM_SIZE*1024);

    malloc_machine(total_ram);
    //random_ram(memoria_spectrum+SCORPION_ROM_SIZE*1024,SCORPION_RAM_SIZE*1024);

    scorpion_init_memory_tables();
    scorpion_set_normal_pages();
}

void scorpion_mem_page_ram(void)
{

    z80_byte page_entra=puerto_32765 & 7;

//| `7FFDh` | D0–D2 | Seleccionan una página de 0 a 7. |
//| `1FFDh` | D4 | Añade 8 al número de página cuando vale 1. |

    if (puerto_8189 & 16) page_entra +=8;

    //TODO
    if (MACHINE_IS_SCORPION_GMX) {
    }

    scorpion_memory_paged[3]=scorpion_ram_mem_table[page_entra];
}

void scorpion_mem_page_rom(void)
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
    if ((puerto_8189 & 2)==0) {
        page_entra=(puerto_32765 & 16) >> 4;
    }

    else {
        page_entra=2;
    }

    //TODO
    if (MACHINE_IS_SCORPION_GMX) {
    }

    if (scorpion_trdos_active.v) scorpion_memory_paged[0]=scorpion_rom_mem_table[3];
    else if (puerto_8189 & 1) scorpion_memory_paged[0]=scorpion_ram_mem_table[0];
    else scorpion_memory_paged[0]=scorpion_rom_mem_table[page_entra];
}

void scorpion_trdos_update(z80_int dir)
{
    //TR-DOS se activa al ejecutar en 3D00h-3DFFh desde BASIC 48.
    //Se desactiva al ejecutar fuera de la zona de ROM.
    if (scorpion_trdos_active.v) {
        if (dir>=0x4000) {
            scorpion_trdos_active.v=0;
            scorpion_mem_page_rom();
        }
    }
    else if (dir>=0x3D00 && dir<=0x3DFF &&
             (puerto_8189 & 3)==0 && (puerto_32765 & 16)) {
        scorpion_trdos_active.v=1;
        scorpion_mem_page_rom();
    }
}

int scorpion_trdos_is_active(void)
{
    return scorpion_trdos_active.v;
}

void scorpion_nmi(void)
{
    // MAGIC selecciona el monitor antes de leer la instrucción en 0066h.
    scorpion_trdos_active.v=0;
    puerto_8189=(puerto_8189 & ~1) | 2;
    scorpion_mem_page_rom();
}

void scorpion_write_port_1ffd(z80_byte value)
{
    puerto_8189=value;

    //asignar ram
    scorpion_mem_page_ram();

    //asignar rom
    scorpion_mem_page_rom();
}



void scorpion_set_cpu_speed(int turbo)
{

    cpu_turbo_speed=turbo;
    cpu_set_turbo_speed();
}

z80_byte fetch_opcode_scorpion(void)
{

    scorpion_trdos_update(reg_pc);
    if (scorpion_trdos_is_active()) betadisk_handle_trdos_traps();

#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif
	return peek_byte_no_time (reg_pc);
}

void poke_byte_no_time_scorpion(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (dir>16383 || (puerto_8189 & 1)) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
        dir = dir & 16383;
        puntero=scorpion_memory_paged[segmento]+dir;

        *puntero=valor;
    }
}


void poke_byte_scorpion(z80_int dir,z80_byte valor)
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



    if (dir>16383 || (puerto_8189 & 1)) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
		dir = dir & 16383;
		puntero=scorpion_memory_paged[segmento]+dir;

		//printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
		*puntero=valor;
	}
}




z80_byte peek_byte_no_time_scorpion(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

	dir = dir & 16383;
	puntero=scorpion_memory_paged[segmento]+dir;

	return *puntero;
}


z80_byte peek_byte_scorpion(z80_int dir)
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
	puntero=scorpion_memory_paged[segmento]+dir;

	return *puntero;
}




