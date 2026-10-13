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
/* Último valor escrito en 7EFD (la lectura de este puerto devuelve otro estado):
 * D0 EWR: permite escribir en la memoria Flash.
 * D1 Vpp: activa la tensión de programación de la Flash.
 * D2 MagEnable: 0 permite la tecla Magic; 1 bloquea su NMI.
 * D3 Graphic: 0 usa vídeo Spectrum; 1 usa el modo GMX de 640x200.
 * D4 A16, D5 A17, D6 A18: eligen uno de ocho bloques de 64 KiB de Flash.
 * D7 Normal/Turbo: 0 velocidad normal; 1 velocidad doble.
 */
static z80_byte scorpion_gmx_port_7efd;
/* Último valor escrito en DFFD, extensión de RAM de tipo Profi:
 * D0 SM0, D1 SM1, D2 SM2: bits 4, 5 y 6 del banco de 16 KiB en C000h.
 * Los bits 0-2 proceden de 7FFD y el bit 3 de 1FFD.D4.
 * D3-D7: sin función documentada para GMX.
 */
static z80_byte scorpion_gmx_port_dffd;
/* Último valor escrito en 78FD (segunda ventana de RAM, 8000h-BFFFh):
 * D0 SW0, D2 SW2, D3 SW3: bits 0, 2 y 3 del banco de 16 KiB.
 * D1 SW1: bit 1 del banco, invertido físicamente; al reset vale 0
 *          y queda seleccionada la página 2.
 * D4 SW4, D5 SW5, D6 SW6: bits 4, 5 y 6 del banco.
 * D7: sin función documentada en escritura; en lectura refleja el
 *     bit 1 del color del borde.
 */
static z80_byte scorpion_gmx_port_78fd;
/* Último valor escrito en el puerto global xx00:
 * D0 CONF0, D1 CONF1, D2 CONF2: variante de configuración FLEX8000.
 * D3 RECONF: solicita cargar esa configuración y reiniciar GMX.
 * D4 FIXROM: fija A16-A18 de la Flash ante escrituras a 7EFD y reset.
 * D5 BLKEXT: desactiva las ampliaciones GMX.
 * D6 BLKEX2: junto con D5 desactiva los puertos xxFD y cambia la
 *             decodificación de 7FFD al modo compatible.
 * D7 ST/PENT: 0 temporización Scorpion; 1 temporización Pentagon.
 * El reset no borra este registro.
 */
static z80_byte scorpion_gmx_port_00;
/* Registro interno de lectura serie por 78FD: tras RECONF contiene
 * 88h OR CONF0..CONF2. Cada lectura entrega el bit 0 y lo desplaza.
 */
static z80_byte scorpion_gmx_magic_shift;

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


    if (MACHINE_IS_SCORPION_GMX) {
        scorpion_gmx_port_7efd=0;
        scorpion_gmx_port_dffd=0;
        scorpion_gmx_port_78fd=0;
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

    if (MACHINE_IS_SCORPION_GMX) {
        page_entra |= (scorpion_gmx_port_dffd & 7) << 4;
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

    if (MACHINE_IS_SCORPION_GMX) {
        page_entra |= (scorpion_gmx_port_7efd & 0x70) >> 2;
    }

    if (scorpion_trdos_active.v || (MACHINE_IS_SCORPION_GMX && (puerto_8189 & 4)))
        scorpion_memory_paged[0]=scorpion_rom_mem_table[(page_entra & ~3) | 3];
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
    return scorpion_trdos_active.v || (MACHINE_IS_SCORPION_GMX && (puerto_8189 & 4));
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

/*
 * Actualiza los registros de paginado y configuración de GMX.
 * Entrada: port es la dirección del puerto; value es el byte escrito.
 * Salida: ninguna. Puede cambiar el mapa de memoria o reiniciar la CPU.
 */
void scorpion_gmx_write_port(z80_int port,z80_byte value)
{
    if ((port & 0xff)==0) {
        scorpion_gmx_port_00=value;
        if (value & 8) {
            // La reconfiguración conserva este puerto y reinicia el procesador.
            scorpion_gmx_magic_shift=0x88 | (value & 7);
            if (!(value & 0x10)) reset_cpu();
        }
        return;
    }

    switch (port & 0xff00) {
        case 0xdf00:
            scorpion_gmx_port_dffd=value;
            scorpion_mem_page_ram();
            break;
        case 0x7800:
            scorpion_gmx_port_78fd=value;
            scorpion_memory_paged[2]=scorpion_ram_mem_table[(value ^ 2) & 0x7f];
            break;
        case 0x7e00:
            scorpion_gmx_port_7efd=(scorpion_gmx_port_00 & 0x10) ?
                ((value & 0x8f) | (scorpion_gmx_port_7efd & 0x70)) : value;
            scorpion_mem_page_rom();
            scorpion_set_cpu_speed((value & 0x80) ? 2 : 1);
            break;
    }
}

/*
 * Lee los registros de estado de paginado de GMX.
 * Entrada: port es la dirección del puerto solicitado.
 * Salida: byte de estado; 78FD desplaza un bit de la configuración latente.
 */
z80_byte scorpion_gmx_read_port(z80_int port)
{
    switch (port & 0xff00) {
        case 0x7a00:
            return (puerto_32765 & 7) | ((puerto_8189 & 0x10) >> 1) |
                   ((scorpion_gmx_port_dffd & 7) << 4);
        case 0x7800:
        {
            z80_byte result=(scorpion_gmx_port_78fd & 0x7f) |
                            (scorpion_gmx_magic_shift & 1);
            scorpion_gmx_magic_shift >>= 1;
            return result;
        }
        case 0x7e00:
            return ((puerto_32765 & 0x20) >> 5) |
                   ((puerto_32765 & 8) >> 2) |
                   ((scorpion_gmx_port_7efd & 0x80) >> 5) |
                   (scorpion_gmx_port_7efd & 8) |
                   ((scorpion_gmx_port_00 & 0x80) >> 3) |
                   (scorpion_gmx_port_00 & 0x20) |
                   ((puerto_8189 & 1) << 6);
    }
    return 0xff;
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




