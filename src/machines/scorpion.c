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
#include "ide.h"
#include "mmc.h"

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

// Indica que MAGIC ya ha esperado una lectura M1 de una instrucción desde RAM.
static int scorpion_magic_ram_fetch_seen;


/* Habilita la interfaz Z-Controller; sin ella la tarjeta SD figura ausente. */
z80_bit zcontroller_enabled={0};

/* Último valor escrito en 7EFD (la lectura de este puerto devuelve otro estado):
 * D0 EWR: permite escribir en la memoria Flash.
 * D1 Vpp: activa la tensión de programación de la Flash.
 * D2 MagEnable: 0 permite la tecla Magic; 1 bloquea su NMI.
 * D3 Graphic: 0 usa vídeo Spectrum; 1 usa el modo GMX de 640x200.
 * D4 A16, D5 A17, D6 A18: eligen uno de ocho bloques de 64 KiB de Flash.
 * D7 Normal/Turbo: 0 velocidad normal; 1 velocidad doble.
 */
z80_byte scorpion_gmx_port_7efd;

/* Último valor escrito en DFFD, extensión de RAM de tipo Profi:
 * D0 SM0, D1 SM1, D2 SM2: bits 4, 5 y 6 del banco de 16 KiB en C000h.
 * Los bits 0-2 proceden de 7FFD y el bit 3 de 1FFD.D4.
 * D3-D7: sin función documentada para GMX.
 */
z80_byte scorpion_gmx_port_dffd;

/* Último valor escrito en 78FD (segunda ventana de RAM, 8000h-BFFFh):
 * D0 SW0, D2 SW2, D3 SW3: bits 0, 2 y 3 del banco de 16 KiB.
 * D1 SW1: bit 1 del banco, invertido físicamente; al reset vale 0
 *          y queda seleccionada la página 2.
 * D4 SW4, D5 SW5, D6 SW6: bits 4, 5 y 6 del banco.
 * D7: sin función documentada en escritura; en lectura refleja el
 *     bit 1 del color del borde.
 */
z80_byte scorpion_gmx_port_78fd;

/* Desplazamiento vertical del vídeo ampliado: 7AFD aporta D4-D7 del
 * byte bajo y 7CFD aporta D0-D5 del byte alto. D0-D3 del byte bajo
 * siempre valen cero. El resultado es un desplazamiento de 14 bits.
 */
static z80_byte scorpion_gmx_scroll_low;

static z80_byte scorpion_gmx_scroll_high;

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
z80_byte scorpion_gmx_port_00;

/* Configuración FLEX8000 activa; RECONF de xx00 carga el esquema elegido.
 * El esquema 4 corresponde al Pentagon128 de la ROM GMX.
 */
z80_byte scorpion_gmx_active_scheme;

/* Registro interno de lectura serie por 78FD: tras RECONF contiene
 * 88h OR CONF0..CONF2. Cada lectura entrega el bit 0 y lo desplaza.
 */
z80_byte scorpion_gmx_magic_shift;



/* Z-Controller de KOE, interfaz SD externa usada por ProfROM:
 * 77h.D0 alimenta la tarjeta y D1 deselecciona su CS cuando vale 1.
 * La lectura de 77h devuelve D0=1 si no hay tarjeta y D1=1 si
 * está protegida contra escritura. 57h intercambia bytes por SPI.
 */
z80_byte scorpion_gmx_zcontroller_control;



/* Inicializa el esquema y el registro serie al encender GMX de nuevo.
 * Entrada: ninguna. Salida: ninguna; no se usa para el botón RESET.
 */
void scorpion_gmx_power_on(void)
{
    scorpion_gmx_port_00=0;
    scorpion_gmx_active_scheme=0;
    scorpion_gmx_magic_shift=0;
}

/* Consulta si GMX ha cargado el esquema de hardware Pentagon.
 * Entrada: ninguna. Salida: 1 para el esquema 4; 0 para los demás.
 */
int scorpion_gmx_is_pentagon_scheme(void)
{
    return MACHINE_IS_SCORPION_GMX && scorpion_gmx_active_scheme==4;
}

/* Consulta la temporización efectiva de GMX.
 * Entrada: ninguna. Salida: 1 para Pentagon; 0 para Scorpion.
 * El esquema 4 usa siempre 320 líneas e INT Pentagon, aunque xx00.D7
 * no esté activado por separado.
 */
int scorpion_gmx_uses_pentagon_timing(void)
{
    return scorpion_gmx_is_pentagon_scheme() || (scorpion_gmx_port_00 & 0x80)!=0;
}

z80_byte scorpion_read_ay_port_a(z80_byte ay_value)
{
    // En Turbo+ las entradas 0-3 y 5 leen D0-D4 de 7FFD; la entrada 4 lee D4 de 1FFD.
    return (ay_value & 0xc0) | (puerto_32765 & 0x0f) |
           (puerto_8189 & 0x10) | ((puerto_32765 & 0x10) << 1);
}


/* Lee los puertos SD del Z-Controller de GMX.
 * Entrada: puerto Z80 y puntero al resultado. Salida: 1 si se atendió
 * el puerto; 0 si no pertenece al Z-Controller.
 */
int scorpion_gmx_zcontroller_read(z80_int port,z80_byte *value)
{
    if (!MACHINE_IS_SCORPION_GMX || scorpion_gmx_is_pentagon_scheme()) return 0;

    if ((port & 0xff)==0x77) {
        *value=(zcontroller_enabled.v && mmc_enabled[0].v ? 0 : 1) |
               (zcontroller_enabled.v && mmc_write_protection[0].v ? 2 : 0);
        return 1;
    }
    if ((port & 0xff)==0x57) {
        *value=(zcontroller_enabled.v && (scorpion_gmx_zcontroller_control & 3)==1) ? mmc_read() : 0xff;
        return 1;
    }
    return 0;
}

/* Escribe los puertos SD del Z-Controller de GMX.
 * Entrada: puerto Z80 y dato. Salida: 1 si se atendió el puerto;
 * 0 si no pertenece al Z-Controller.
 */
int scorpion_gmx_zcontroller_write(z80_int port,z80_byte value)
{
    if (!MACHINE_IS_SCORPION_GMX || scorpion_gmx_is_pentagon_scheme()) return 0;

    if (!zcontroller_enabled.v) {
        /* La interfaz ausente no transmite órdenes a la tarjeta. */
        if ((port & 0xff)==0x77 || (port & 0xff)==0x57) return 1;
        return 0;
    }

    if ((port & 0xff)==0x77) {
        int was_selected=(scorpion_gmx_zcontroller_control & 3)==1;
        int is_selected=(value & 3)==1;
        scorpion_gmx_zcontroller_control=value;
        if (was_selected!=is_selected) mmc_cs(is_selected ? 2 : 3);
        return 1;
    }
    if ((port & 0xff)==0x57) {
        if ((scorpion_gmx_zcontroller_control & 3)==1) mmc_write(value);
        return 1;
    }
    return 0;
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
    scorpion_magic_ram_fetch_seen=0;
    scorpion_memory_paged[0]=scorpion_rom_mem_table[0];
    scorpion_memory_paged[1]=scorpion_ram_mem_table[5];
    scorpion_memory_paged[2]=scorpion_ram_mem_table[2];
    scorpion_memory_paged[3]=scorpion_ram_mem_table[0];
    debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM;
    debug_paginas_memoria_mapeadas[1]=5;
    debug_paginas_memoria_mapeadas[2]=2;
    debug_paginas_memoria_mapeadas[3]=0;


    if (MACHINE_IS_SCORPION_GMX) {
        ide_smuc_reset();
        scorpion_gmx_port_7efd=0;
        scorpion_gmx_zcontroller_control=0;
        scorpion_gmx_port_dffd=0;
        scorpion_gmx_port_78fd=0;
        scorpion_gmx_scroll_low=0;
        scorpion_gmx_scroll_high=0;
        if (scorpion_gmx_is_pentagon_scheme()) scorpion_mem_page_rom();
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
    z80_byte page_8000;

//| `7FFDh` | D0–D2 | Seleccionan una página de 0 a 7. |
//| `1FFDh` | D4 | Añade 8 al número de página cuando vale 1. |

    if (!scorpion_gmx_is_pentagon_scheme() && (puerto_8189 & 16)) page_entra +=8;

    if (MACHINE_IS_SCORPION_GMX && !scorpion_gmx_is_pentagon_scheme()) {
        page_entra |= (scorpion_gmx_port_dffd & 7) << 4;
        // 78FD selecciona de forma independiente la página de 8000h-BFFFh.
        page_8000=(scorpion_gmx_port_78fd ^ 2) & 0x7f;
        scorpion_memory_paged[2]=scorpion_ram_mem_table[page_8000];
        debug_paginas_memoria_mapeadas[2]=page_8000;
    }

    scorpion_memory_paged[3]=scorpion_ram_mem_table[page_entra];
    debug_paginas_memoria_mapeadas[3]=page_entra;
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

    if (MACHINE_IS_SCORPION_GMX && scorpion_gmx_is_pentagon_scheme()) {
        page_entra+=4;
    }
    else if (MACHINE_IS_SCORPION_GMX) {
        page_entra |= (scorpion_gmx_port_7efd & 0x70) >> 2;
    }

    if (scorpion_trdos_active.v || (MACHINE_IS_SCORPION_GMX &&
        !scorpion_gmx_is_pentagon_scheme() && (puerto_8189 & 4))) {
        page_entra=(page_entra & ~3) | 3;
        scorpion_memory_paged[0]=scorpion_rom_mem_table[page_entra];
        debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+page_entra;
    }
    else if (!scorpion_gmx_is_pentagon_scheme() && (puerto_8189 & 1)) {
        scorpion_memory_paged[0]=scorpion_ram_mem_table[0];
        debug_paginas_memoria_mapeadas[0]=0;
    }
    else {
        scorpion_memory_paged[0]=scorpion_rom_mem_table[page_entra];
        debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+page_entra;
    }
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
    return scorpion_trdos_active.v || (MACHINE_IS_SCORPION_GMX &&
        !scorpion_gmx_is_pentagon_scheme() && (puerto_8189 & 4));
}

/* Indica si el joystick Kempston puede responder a un puerto en GMX.
 * Entrada: dirección del puerto. Salida: 1 si el joystick puede responder;
 * 0 si TR-DOS está activo o la ROM de servicio consulta el puerto 1Fh.
 */
int scorpion_gmx_kempston_port_enabled(z80_int port)
{
    int page;

    if (scorpion_trdos_is_active()) return 0;
    if ((port & 0xff)!=0x1f) return 1;

    for (page=0;page<SCORPIONGMX_TOTAL_ROM_PAGES;page++) {
        if (scorpion_memory_paged[0]==scorpion_rom_mem_table[page]) {
            return (page & 3)!=2;
        }
    }

    return 1;
}

void scorpion_nmi(void)
{
    if (MACHINE_IS_SCORPION_GMX && scorpion_gmx_is_pentagon_scheme()) {
        // MAGIC del Pentagon entra en la ROM TR-DOS, no en el monitor.
        scorpion_trdos_active.v=1;
        scorpion_mem_page_rom();
        return;
    }
    // MAGIC selecciona el monitor antes de leer la instrucción en 0066h.
    scorpion_magic_ram_fetch_seen=0;
    // En GMX, MGEN bloquea nuevas pulsaciones y permite paginar con 7FFD.D5 activo.
    if (MACHINE_IS_SCORPION_GMX) scorpion_gmx_port_7efd |= 0x04;
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
        int timing_anterior=scorpion_gmx_uses_pentagon_timing();
        scorpion_gmx_port_00=value;
        if (value & 8) {
            // RECONF carga el esquema antes de actualizar la temporización.
            scorpion_gmx_magic_shift=0x88 | (value & 7);
            scorpion_gmx_active_scheme=value & 7;
        }
        if (timing_anterior!=scorpion_gmx_uses_pentagon_timing())
            scorpion_set_cpu_speed(cpu_turbo_speed);
        if (value & 8) {
            // La reconfiguración conserva este puerto y reinicia el procesador.
            if (!(value & 0x10)) reset_cpu();
        }
        return;
    }

    // La configuración Pentagon desconecta los puertos ampliados GMX.
    if (scorpion_gmx_is_pentagon_scheme()) return;

    switch (port & 0xff00) {
        case 0xdf00:
            scorpion_gmx_port_dffd=value;
            scorpion_mem_page_ram();
            break;
        case 0x7800:
            scorpion_gmx_port_78fd=value;
            scorpion_mem_page_ram();
            break;
        case 0x7a00:
            scorpion_gmx_scroll_low=value & 0xf0;
            break;
        case 0x7c00:
            scorpion_gmx_scroll_high=value & 0x3f;
            break;
        case 0x7e00:
        {
            z80_byte video_anterior=scorpion_gmx_port_7efd & 8;
            scorpion_gmx_port_7efd=(scorpion_gmx_port_00 & 0x10) ?
                ((value & 0x8f) | (scorpion_gmx_port_7efd & 0x70)) : value;
            if (video_anterior != (scorpion_gmx_port_7efd & 8)) {

                /*
                printf("Scorpion GMX: video %s (7EFDh=%02Xh, PC=%04Xh)\n",
                       (scorpion_gmx_port_7efd & 8) ? "640x200" : "256x192",

                */

            char buffer_mensaje[256];
            sprintf(buffer_mensaje,"Setting video mode %s",(scorpion_gmx_port_7efd & 8) ? "640x200" : "256x192");
            screen_print_splash_text_center_no_if_previous(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);

            }
            scorpion_mem_page_rom();
            scorpion_set_cpu_speed((value & 0x80) ? 2 : 1);
            break;
        }
    }
}

/*
 * Lee los registros de estado de paginado de GMX.
 * Entrada: port es la dirección del puerto solicitado.
 * Salida: byte de estado; 78FD desplaza un bit de la configuración latente.
 */
z80_byte scorpion_gmx_read_port(z80_int port)
{
    if (scorpion_gmx_is_pentagon_scheme()) return 0xff;
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

    /* El cambio de velocidad recalcula los parámetros de máquina y vuelve
       a fijar el pulso en 36 T; a 7 MHz debe durar 72 T de CPU. */
    if (MACHINE_IS_SCORPION_ZS_256_TURBO_PLUS || MACHINE_IS_SCORPION_GMX)
        cpu_duracion_pulso_interrupcion=36*turbo;
}

/*
 * Simula seleccionar la velocidad normal con el control físico de GMX.
 * Entrada: ninguna.
 * Salida: ninguna; deja la CPU a x1 y actualiza el estado de 7EFD.D7.
 */
void scorpion_gmx_set_normal_speed(void)
{
    if (!MACHINE_IS_SCORPION_GMX) return;

    scorpion_gmx_port_7efd &= ~0x80;
    scorpion_set_cpu_speed(1);
}

/*
 * Simula seleccionar la velocidad turbo con el control físico de GMX.
 * Entrada: ninguna.
 * Salida: ninguna; deja la CPU a x2 y actualiza el estado de 7EFD.D7.
 */
void scorpion_gmx_set_turbo_speed(void)
{
    if (!MACHINE_IS_SCORPION_GMX) return;

    scorpion_gmx_port_7efd |= 0x80;
    scorpion_set_cpu_speed(2);
}

z80_byte fetch_opcode_scorpion(void)
{

    scorpion_trdos_update(reg_pc);
    if (scorpion_trdos_is_active()) betadisk_handle_trdos_traps();

    // MAGIC se enclava hasta el primer ciclo M1 que lee una instrucción de RAM.
    if (!interrupcion_non_maskable_generada.v) scorpion_magic_ram_fetch_seen=0;
    else if (reg_pc>=0x4000 || scorpion_memory_paged[0]==scorpion_ram_mem_table[0]) {
        scorpion_magic_ram_fetch_seen=1;
    }

#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif
	return peek_byte_no_time (reg_pc);
}

/* Devuelve 1 cuando la NMI pendiente ya ha visto una lectura de instrucción
   desde RAM; no recibe parámetros y devuelve 0 mientras MAGIC espera en ROM. */
int scorpion_nmi_ram_fetch_seen(void)
{
    return scorpion_magic_ram_fetch_seen;
}

/* Indica si el botón MAGIC puede generar una NMI en GMX.
   No recibe parámetros; devuelve 1 si 7EFDh.D2 está a cero, o 0 si está bloqueado. */
int scorpion_gmx_magic_enabled(void)
{
    if (scorpion_gmx_is_pentagon_scheme()) return 1;
    return (scorpion_gmx_port_7efd & 0x04)==0;
}

/* Indica si GMX acepta escrituras de paginación en 7FFDh.
   No recibe parámetros; devuelve 1 si 7FFD.D5 no bloquea el puerto o
   si 7EFD.D2 (MGEN) anula ese bloqueo. */
int scorpion_gmx_7ffd_paging_enabled(void)
{
    if (scorpion_gmx_is_pentagon_scheme()) return !(puerto_32765 & 0x20);
    return !(puerto_32765 & 0x20) || (scorpion_gmx_port_7efd & 0x04);
}

void poke_byte_no_time_scorpion(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (dir>16383 || (!scorpion_gmx_is_pentagon_scheme() && (puerto_8189 & 1))) {
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



    if (dir>16383 || (!scorpion_gmx_is_pentagon_scheme() && (puerto_8189 & 1))) {
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



// Dibuja colour en (x,y), con escalas scale_x/scale_y y offsets offset_x/offset_y; sin retorno.
static void scorpiongmx_putpixel_scaled(int x,int y,z80_int colour,
                                    int scale_x,int scale_y,
                                    int offset_x,int offset_y)
{
    int dx,dy;
    int output_x=offset_x+x*scale_x;
    int output_y=offset_y+y*scale_y;

    for (dy=0;dy<scale_y;dy++) {
        for (dx=0;dx<scale_x;dx++) {
            scr_putpixel_zoom(output_x+dx,output_y+dy,colour);
        }
    }
}

//Refresco pantalla sin rainbow
// Dibuja la pantalla ZX estándar sin rainbow; sin parámetros ni retorno.
void scorpiongmx_refresca_pantalla_no_rainbow_standard_48k(void)
{
    int x,y,bit;
    z80_int direccion,dir_atributo;
    z80_byte byte_leido;
    int color=0;
    int fila;
    //int zx,zy;

    z80_byte attribute,ink,paper,bright,flash,aux;



    z80_byte *screen=get_base_mem_pantalla();

    //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
    z80_byte x_hi;

    for (y=0;y<192;y++) {

        direccion=screen_addr_table[(y<<5)];


        fila=y/8;
        dir_atributo=6144+(fila*32);
        for (x=0,x_hi=0;x<32;x++,x_hi +=8) {



            byte_leido=screen[direccion];
            attribute=screen[dir_atributo];



            if (scr_refresca_sin_colores.v) {
                attribute=56;


            }

            else {
                if (scr_refresca_show_attribute_grid.v) {
                    int cuad=(fila+x)%2;
                    if (cuad) attribute=56+64;
                    else attribute=56;
                }
            }


            ink=attribute &7;
            paper=(attribute>>3) &7;
            bright=(attribute) &64;
            flash=(attribute)&128;
            if (flash) {
                //intercambiar si conviene
                if (estado_parpadeo.v) {
                        aux=paper;
                        paper=ink;
                        ink=aux;
                }
            }

            if (bright) {
                ink +=8;
                paper +=8;
            }

            for (bit=0;bit<8;bit++) {

                color= ( byte_leido & 128 ? ink : paper );


                scorpiongmx_putpixel_scaled(x_hi+bit,y,
                                         color,
                                         2,2,64,8);

                byte_leido=byte_leido<<1;
            }



            direccion++;
            dir_atributo++;
        }

    }

}




/* Dibuja el modo GMX de 640x200 desde sus páginas físicas de píxeles y
 * atributos. Entrada: ninguna. Salida: ninguna; actualiza la imagen visible.
 */
static void scorpiongmx_refresca_pantalla_no_rainbow_640x200(void)
{
    int x,y,bit;
    unsigned int scroll=((unsigned int)scorpion_gmx_scroll_high << 8) |
                        scorpion_gmx_scroll_low;
    int pagina=(puerto_32765 & 8) ? 0x3b : 0x39;
    z80_byte *pixels=scorpion_ram_mem_table[pagina];
    z80_byte *attributes=scorpion_ram_mem_table[pagina+0x40];

    for (y=0;y<200;y++) {
        unsigned int direccion=(scroll+y*80)%16000;

        for (x=0;x<80;x++) {
            z80_byte pixel=pixels[direccion];
            z80_byte attribute=attributes[direccion];
            z80_byte ink,paper;

            if (scr_refresca_sin_colores.v) attribute=56;
            else if (scr_refresca_show_attribute_grid.v)
                attribute=((y/8+x)&1) ? 120 : 56;

            ink=(attribute & 7) | ((attribute & 0x40) >> 3);
            paper=((attribute >> 3) & 7) | ((attribute & 0x40) >> 3);

            if ((attribute & 0x80) && estado_parpadeo.v) {
                z80_byte aux=ink;
                ink=paper;
                paper=aux;
            }

            for (bit=0;bit<8;bit++) {
                scorpiongmx_putpixel_scaled(x*8+bit,y,
                                            (pixel & 0x80) ? ink : paper,
                                            1,2,0,0);
                pixel <<= 1;
            }

            direccion++;
            if (direccion==16000) direccion=0;
        }
    }
}


int if_scorpiongmx_mode_640_200(void)
{
    if (scorpion_gmx_port_7efd & 8) return 1;
    else return 0;
}

/* Devuelve el desplazamiento vertical visible en líneas completas.
 * Entrada: ninguna. Salida: líneas de 0 a 199; si el desplazamiento
 * no es múltiplo de 80 bytes, se descarta la fracción de línea.
 */
int scorpion_gmx_get_scroll_lines(void)
{
    unsigned int offset=((unsigned int)scorpion_gmx_scroll_high << 8) |
                        scorpion_gmx_scroll_low;
    return (offset % 16000) / 80;
}

void scorpiongmx_refresca_pantalla_no_rainbow(void)
{
    if (scorpion_gmx_port_7efd & 8)
        scorpiongmx_refresca_pantalla_no_rainbow_640x200();
    else
        scorpiongmx_refresca_pantalla_no_rainbow_standard_48k();
}

// Dibuja colour en (x,y) usando el zoom global; sin retorno.
static void scorpiongmx_putpixel_absolute(int x,int y,z80_int colour)
{
    int dx,dy;
    int px=x*zoom_x;
    int py=y*zoom_y;

    for (dy=0;dy<zoom_y;dy++)
        for (dx=0;dx<zoom_x;dx++)
            scr_putpixel(px+dx,py+dy,colour);
}

// Escribe en offset_x/offset_y y width/height la geometría del modo actual; sin retorno.
static void scorpiongmx_get_mode_geometry(int *offset_x,int *offset_y,
                                       int *width,int *height)
{
    if (scorpion_gmx_port_7efd & 8) {
        *width=SCORPIONGMX_DISPLAY_WIDTH;
        *height=SCORPIONGMX_DISPLAY_HEIGHT;
        *offset_x=0;
        *offset_y=0;
        return;
    }

    // La pantalla Spectrum 256x192 se dibuja a escala 2 dentro de 640x400.
    *width=512;
    *height=384;
    *offset_x=(SCORPIONGMX_DISPLAY_WIDTH-*width)/2;
    *offset_y=(SCORPIONGMX_DISPLAY_HEIGHT-*height)/2;
}

// Dibuja el borde del modo GMX actual; sin parámetros ni retorno.
static void screen_scorpiongmx_refresca_border(void)
{
    int x,y,offset_x,offset_y,width,height;
    int left,top,right,bottom;
    int total_width,total_height;
    z80_int colour=out_254 & 7;

    scorpiongmx_get_mode_geometry(&offset_x,&offset_y,&width,&height);
    total_width=SCORPIONGMX_DISPLAY_WIDTH+
            (SCORPIONGMX_LEFT_BORDER_NO_ZOOM+SCORPIONGMX_RIGHT_BORDER_NO_ZOOM)*border_enabled.v;
    total_height=SCORPIONGMX_DISPLAY_HEIGHT+
            (SCORPIONGMX_TOP_BORDER_NO_ZOOM+SCORPIONGMX_BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;

    left=SCORPIONGMX_LEFT_BORDER_NO_ZOOM*border_enabled.v+offset_x;
    top=SCORPIONGMX_TOP_BORDER_NO_ZOOM*border_enabled.v+offset_y;
    right=left+width;
    bottom=top+height;

    for (y=0;y<top;y++)
        for (x=0;x<total_width;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);

    for (y=bottom;y<total_height;y++)
        for (x=0;x<total_width;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);

    for (y=top;y<bottom;y++) {
        for (x=0;x<left;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);
        for (x=right;x<total_width;x++)
                scorpiongmx_putpixel_absolute(x,y,colour);
        }
}

/* Registra la línea de imagen GMX con la geometría de su modo actual.
 * Entrada: estado de vídeo y scanline actual. Salida: píxeles en rainbow_buffer.
 */
void scorpiongmx_store_scanline_rainbow_display(void)
{
    int source_y=t_scanline_draw-screen_indice_inicio_pant;
    int wide=(scorpion_gmx_port_7efd & 8)!=0;
    int source_height=wide ? 200 : 192;
    int source_width=wide ? 640 : 256;
    int scale_x=wide ? 1 : 2;
    int width=get_total_ancho_rainbow();
    int offset_x=(width-source_width*scale_x)/2;
    int top=(border_enabled.v ? SCORPIONGMX_TOP_BORDER_NO_ZOOM : 0)+
            (SCORPIONGMX_DISPLAY_HEIGHT-source_height*2)/2;
    int first_row,last_row,x,row,bit;
    z80_byte *pixels=NULL,*attributes=NULL;
    unsigned int scroll=0;

    if (source_y<0 || source_y>=source_height) return;

    /* El modo ancho usa ocho scanlines más de la zona que en modo ZX
       corresponde al borde inferior, sin variar la temporización. */
    first_row=source_y;
    last_row=source_y+1;

    if (!border_enabled.v && source_y==0) {
        int position;
        z80_int colour=out_254 & 7;
        for (position=0;position<width*SCORPIONGMX_DISPLAY_HEIGHT;position++)
            rainbow_buffer[position]=colour;
    }

    if (wide) {
        int page=(puerto_32765 & 8) ? 0x3b : 0x39;
        pixels=scorpion_ram_mem_table[page];
        attributes=scorpion_ram_mem_table[page+0x40];
        scroll=((unsigned int)scorpion_gmx_scroll_high << 8) |
               scorpion_gmx_scroll_low;
    }

    for (row=first_row;row<last_row;row++) {
        int output_y=top+row*2;
        if (output_y<0 || output_y+1>=get_total_alto_rainbow()) continue;

        for (x=0;x<source_width/8;x++) {
            z80_byte pixel,attribute,ink,paper;

            if (wide) {
                unsigned int address=(scroll+row*80+x)%16000;
                pixel=pixels[address];
                attribute=attributes[address];
            }
            else {
                pixel=scanline_buffer[x*2];
                attribute=scanline_buffer[x*2+1];
            }

            if (scr_refresca_sin_colores.v) attribute=56;
            else if (scr_refresca_show_attribute_grid.v)
                attribute=((row/8+x)&1) ? 120 : 56;

            ink=(attribute & 7) | ((attribute & 0x40) >> 3);
            paper=((attribute >> 3) & 7) | ((attribute & 0x40) >> 3);
            if ((attribute & 0x80) && estado_parpadeo.v) {
                z80_byte aux=ink;
                ink=paper;
                paper=aux;
            }

            for (bit=0;bit<8;bit++) {
                int output_x=offset_x+(x*8+bit)*scale_x;
                z80_int colour=(pixel & 0x80) ? ink : paper;
                int dy,dx;
                for (dy=0;dy<2;dy++)
                    for (dx=0;dx<scale_x;dx++)
                        rainbow_buffer[(output_y+dy)*width+output_x+dx]=colour;
                pixel<<=1;
            }
        }
    }
}

/* Guarda el borde GMX respetando los cambios de color de este scanline.
 * Entrada: fullbuffer_border y scanline actual. Salida: borde en rainbow_buffer.
 */
void scorpiongmx_store_scanline_rainbow_border(void)
{
    z80_byte state_colour[MAX_STATES_LINE];
    static z80_byte left_next[SCORPIONGMX_DISPLAY_WIDTH+
                              SCORPIONGMX_LEFT_BORDER_NO_ZOOM*2];
    static int left_next_valid=0;
    z80_byte colour=screen_border_last_color;
    int wide=(scorpion_gmx_port_7efd & 8)!=0;
    int source_height=wide ? 200 : 192;
    int top_border=(288-source_height)/2;
    int visible_y=t_scanline_draw-(screen_indice_inicio_pant-top_border);
    int width=get_total_ancho_rainbow();
    int left=wide ? SCORPIONGMX_LEFT_BORDER_NO_ZOOM :
                    SCORPIONGMX_LEFT_BORDER_NO_ZOOM+64;
    int right=width-left;
    int line_start=t_scanline*screen_testados_linea;
    int pre_retrace=screen_testados_indice_borde_derecho*cpu_turbo_speed;
    int post_retrace=screen_testados_linea-
                     (screen_total_borde_izquierdo/2)*cpu_turbo_speed;
    int state,x,dy;

    if (pre_retrace>screen_testados_linea) pre_retrace=screen_testados_linea;
    if (post_retrace<pre_retrace) post_retrace=pre_retrace;
    if (post_retrace>=screen_testados_linea)
        post_retrace=screen_testados_linea-1;

    for (state=0;state<screen_testados_linea;state++) {
        z80_byte change=fullbuffer_border[line_start+state];
        if (change!=255) colour=change;
        state_colour[state]=colour;
    }
    screen_border_last_color=colour;

    if (visible_y>=0 && visible_y<288) {
        int output_y=visible_y*2;
        int active=visible_y>=top_border && visible_y<top_border+source_height;
        for (x=0;x<width;x++) {
            z80_byte raw;
            if (active && x>=left && x<right) continue;
            if (x<left && left_next_valid)
                raw=left_next[(x*width)/left];
            else {
                state=((x<left ? 0 : x-left)*pre_retrace)/
                      (x<left ? 1 : width-left);
                if (state>=pre_retrace) state=pre_retrace-1;
                raw=state_colour[state];
            }
            for (dy=0;dy<2;dy++)
                rainbow_buffer[(output_y+dy)*width+x]=raw & 7;
        }
    }

    for (x=0;x<width;x++) {
        state=post_retrace+
              (x*(screen_testados_linea-post_retrace))/width;
        if (state>=screen_testados_linea) state=screen_testados_linea-1;
        left_next[x]=state_colour[state];
    }
    left_next_valid=1;
}

// Refresca pantalla y borde usando rainbow o dibujo completo; sin parámetros ni retorno.
void scorpiongmx_refresca_pantalla(void)
{
    if (rainbow_enabled.v) {
        scr_refresca_pantalla_rainbow_comun();
    }
    else {
        scorpiongmx_refresca_pantalla_no_rainbow();
        screen_scorpiongmx_refresca_border();
        modificado_border.v=0;
    }
}
