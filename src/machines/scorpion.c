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


/*

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
z80_byte *scorpion_ram_mem_table[SCORPION_TOTAL_RAM_PAGES];

//Donde esta cada pagina de rom
z80_byte *scorpion_rom_mem_table[SCORPION_TOTAL_ROM_PAGES];

//Direcciones actuales mapeadas
z80_byte *scorpion_memory_paged[4];
static z80_bit scorpion_trdos_active={0};

void scorpion_init_memory_tables(void)
{
    //Primero rom y luego ram

    int i;
    z80_byte *puntero=memoria_spectrum;

    for (i=0;i<SCORPION_TOTAL_ROM_PAGES;i++) {
        scorpion_rom_mem_table[i]=puntero;
        puntero +=16384;
    }

    for (i=0;i<SCORPION_TOTAL_RAM_PAGES;i++) {
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

}

void scorpion_malloc_mem_machine(void)
{
    //64 KB ROM, 256 KB RAM

    malloc_machine((SCORPION_ROM_SIZE+SCORPION_RAM_SIZE)*1024);
    random_ram(memoria_spectrum+SCORPION_ROM_SIZE*1024,SCORPION_RAM_SIZE*1024);

    scorpion_init_memory_tables();
    scorpion_set_normal_pages();
}

void scorpion_mem_page_ram(void)
{

    z80_byte page_entra=puerto_32765 & 7;

//| `7FFDh` | D0–D2 | Seleccionan una página de 0 a 7. |
//| `1FFDh` | D4 | Añade 8 al número de página cuando vale 1. |

    if (puerto_8189 & 16) page_entra +=8;

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

void scorpion_write_port_1ffd(z80_byte value)
{
    puerto_8189=value;

    //asignar ram
    scorpion_mem_page_ram();

    //asignar rom
    scorpion_mem_page_rom();
}
