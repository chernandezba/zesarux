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
#include "vg93.h"
#include "ide.h"
#include "utils.h"



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
z80_byte *atmturbo_ram_mem_table[ATM_TURBO_TOTAL_RAM_PAGES];

//Donde esta cada pagina de rom
z80_byte *atmturbo_rom_mem_table[ATM_TURBO_TOTAL_ROM_PAGES];

//Direcciones actuales mapeadas
z80_byte *atmturbo_memory_paged[4];

/* Último valor escrito en #FDFD para seleccionar el banco de RAM de ATM1. */
z80_byte puerto_fdfd;

/* Activa la ROM de CP/M de ATM1 tras leer #FB y la desactiva al leer #7B. */
static int atmturbo_cpsys;
/* ATM1 muestra ROM en #0000; al desactivarse se muestra RAM en ese segmento. */
static int atmturbo_rom_enabled=1;
/* Indica que la ROM de TR-DOS está paginada temporalmente. */
static int atmturbo_trdos_active;
/* Código del modo de vídeo seleccionado: 3 es el modo Spectrum. */
static int atmturbo_video_mode=3;
/* Índice de la paleta usado para el color del borde. */
static int atmturbo_border_colour;
/* Colores al final de la línea anterior que llegan al borde izquierdo. */
static z80_byte atmturbo_border_left_next[ATM_TURBO_DISPLAY_WIDTH+2*ATM_TURBO_LEFT_BORDER_NO_ZOOM];
/* Indica si el búfer del borde izquierdo ya contiene una línea válida. */
static int atmturbo_border_left_next_valid;
/* Traduce los 16 índices de color de ATM a colores del emulador. */
static int atmturbo_palette[16];
/* Recuerda si ATM1 ya ha seleccionado el controlador de disquete. */
static int atmturbo_fdc_selected;
/* Ocho entradas de paginado de ATM2: cuatro por cada valor de #7FFD.D4. */
static unsigned int atmturbo2_mmu[8];
/* Dirección del último OUT a #77; sus bits altos controlan PEN, CP/M y PEN2. */
static z80_int atmturbo2_port_77;
/* Indica para cada segmento de 16 KB si la página visible es RAM. */
static int atmturbo2_segment_ram[4];
/* Glifos de 8 bytes cargados de sgen.rom para el modo de texto de ATM2. */
static z80_byte atmturbo2_text_font[2048];
/* Último estado entregado de cada fila para detectar pulsaciones y liberaciones XT. */
static z80_byte atmturbo2_xt_rows[8];
/* Código de pulsación de cada tecla, conservado para generar su liberación. */
static z80_byte atmturbo2_xt_codes[8][5];
/* Código XT pendiente, con sus bits invertidos de orden como en el receptor serie. */
static z80_byte atmturbo2_xt_data;
/* Lecturas pendientes del código: la BIOS lee el receptor dos veces por evento. */
static int atmturbo2_xt_reads;
/* Modo del controlador 8031: matriz, código CP/M, CP/M con flags o XT directo. */
static z80_byte atmturbo2plus_keyboard_mode;
/* Fase de recepción por #FE: 0 normal, 1 comando, 2 argumento. */
static int atmturbo2plus_keyboard_phase;
/* Comando que espera un argumento después de la secuencia #55 por #FE. */
static z80_byte atmturbo2plus_keyboard_command;
/* Código CP/M pendiente de entrega y último código leído del controlador. */
static z80_byte atmturbo2plus_keyboard_code,atmturbo2plus_keyboard_last;
/* Flags del 8031: Shift, Control y selección del alfabeto ruso. */
static z80_byte atmturbo2plus_keyboard_flags;
/* Bit 6 de #77: permite seleccionar la matriz sin intervención del 8031. */
static int atmturbo2plus_keyboard_disabled;
/* D5 de #77 permite o bloquea las interrupciones periódicas en ATM2+. */
static int atmturbo2plus_int_enabled;
/* Diferencia respecto al reloj del anfitrión, modificable por comandos del 8031. */
static time_t atmturbo2plus_clock_offset;
/* Byte alto retenido por el interfaz IDE de 16 bits de ATM2/2+. */
static z80_byte atmturbo2_ide_read_high,atmturbo2_ide_write_high;

static void atmturbo2_refresh_memory(void);

z80_bit ide_atm_enabled={0};

/* Consulta la matriz y retiene un cambio hasta que CP/M recoge su código XT. */
z80_byte atmturbo2_read_keyboard_status(void)
{
    /* El menú bloquea tanto la consulta de la matriz como los eventos pendientes. */
    if (zxvision_key_not_sent_emulated_mach()) return 0x7f;

    /* Códigos de exploración XT, ordenados por fila y bit de la matriz Spectrum.
       Caps Shift representa Shift izquierdo y Symbol Shift representa Control. */
    static const z80_byte codes[8][5]={
        {0x2a,0x2c,0x2d,0x2e,0x2f},
        {0x1e,0x1f,0x20,0x21,0x22},
        {0x10,0x11,0x12,0x13,0x14},
        {0x02,0x03,0x04,0x05,0x06},
        {0x0b,0x0a,0x09,0x08,0x07},
        {0x19,0x18,0x17,0x16,0x15},
        {0x1c,0x26,0x25,0x24,0x23},
        {0x39,0x1d,0x32,0x31,0x30}
    };
    z80_byte rows[8]={puerto_65278,puerto_65022,puerto_64510,puerto_63486,
                      puerto_61438,puerto_57342,puerto_49150,puerto_32766};
    int colon=!(rows[7]&2) && !(rows[0]&2);
    int row,bit,phase;
    if (atmturbo2_xt_reads) return 0xff;
    /* Symbol Shift+Z produce dos puntos: Shift y punto y coma del teclado XT.
       En esta combinación Symbol Shift no debe generar Control. */
    if (colon) {
        rows[0]&=~1;
        rows[7]|=2;
    }
    /* Entrega los modificadores antes que las letras de una misma consulta. */
    for (phase=0;phase<2;phase++) {
        for (row=0;row<8;row++) {
            for (bit=0;bit<5;bit++) {
                int modifier=(row==0 && bit==0) || (row==7 && bit==1);
                if (modifier!=(phase==0)) continue;
                int mask=1<<bit;
                if ((rows[row]^atmturbo2_xt_rows[row])&mask) {
                    z80_byte code;
                    if (rows[row]&mask) code=atmturbo2_xt_codes[row][bit]|0x80;
                    else {
                        code=(colon && row==0 && bit==1) ? 0x27 : codes[row][bit];
                        atmturbo2_xt_codes[row][bit]=code;
                    }
                    atmturbo2_xt_rows[row]^=mask;
                    atmturbo2_xt_data=0;
                    for (int i=0;i<8;i++) {
                        atmturbo2_xt_data=(atmturbo2_xt_data<<1)|(code&1);
                        code>>=1;
                    }
                    atmturbo2_xt_reads=2;
                    return 0xff;
                }
            }
        }
    }
    return 0x7f;
}

/* Mantiene el código serie estable durante las dos lecturas de la BIOS. */
z80_byte atmturbo2_read_keyboard_data(void)
{
    /* Mientras el menú captura el teclado, no entrega ni consume el código XT. */
    if (zxvision_key_not_sent_emulated_mach()) return 0xff;

    if (atmturbo2_xt_reads) atmturbo2_xt_reads--;
    return atmturbo2_xt_data;
}

/* Convierte los eventos XT de la matriz en códigos y modificadores del 8031. */
static void atmturbo2plus_poll_keyboard(void)
{
    /* Tabla CP/M del controlador XT: las letras se entregan en mayúsculas. */
    static const z80_byte characters[0x3a]={
        [0x01]=27,[0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',
        [0x07]='6',[0x08]='7',[0x09]='8',[0x0a]='9',[0x0b]='0',
        [0x0c]='-',[0x0d]='=',[0x0e]=8,[0x0f]=9,
        [0x10]='Q',[0x11]='W',[0x12]='E',[0x13]='R',[0x14]='T',
        [0x15]='Y',[0x16]='U',[0x17]='I',[0x18]='O',[0x19]='P',
        [0x1a]='[',[0x1b]=']',[0x1c]=13,
        [0x1e]='A',[0x1f]='S',[0x20]='D',[0x21]='F',[0x22]='G',
        [0x23]='H',[0x24]='J',[0x25]='K',[0x26]='L',[0x27]=';',[0x28]='\'',
        [0x29]='`',[0x2b]='\\',[0x2c]='Z',[0x2d]='X',[0x2e]='C',
        [0x2f]='V',[0x30]='B',[0x31]='N',[0x32]='M',[0x33]=',',
        [0x34]='.',[0x35]='/',[0x37]='*',[0x39]=' '
    };
    if (zxvision_key_not_sent_emulated_mach() || atmturbo2plus_keyboard_code) return;
    while (atmturbo2_read_keyboard_status()&0x80) {
        z80_byte serial=atmturbo2_read_keyboard_data();
        atmturbo2_read_keyboard_data();
        z80_byte scan=0;
        for (int i=0;i<8;i++) {
            scan=(scan<<1)|(serial&1);
            serial>>=1;
        }
        int released=scan&0x80;
        int key=scan&0x7f;
        int flag=(key==0x2a || key==0x36) ? 1 : (key==0x1d ? 2 : 0);
        if (flag) {
            if (released) atmturbo2plus_keyboard_flags&=~flag;
            else atmturbo2plus_keyboard_flags|=flag;
        }
        if (atmturbo2plus_keyboard_mode==3) atmturbo2plus_keyboard_code=scan;
        else if (!released && key<(int)sizeof(characters))
            atmturbo2plus_keyboard_code=characters[key];
        if (atmturbo2plus_keyboard_code) return;
    }
}

/* Extrae un código del 8031 y conserva el último para el comando #49. */
static z80_byte atmturbo2plus_get_keyboard_code(void)
{
    z80_byte code=atmturbo2plus_keyboard_code;
    atmturbo2plus_keyboard_code=0;
    if (code) atmturbo2plus_keyboard_last=code;
    return code;
}

/* Lee o ajusta los campos del reloj que expone el controlador de teclado. */
static z80_byte atmturbo2plus_clock(int command,int selector,int value,int writing)
{
    time_t now=time(NULL);
    time_t current=now+atmturbo2plus_clock_offset;
    struct tm *local=localtime(&current);
    if (!local || selector>2) return 0xff;
    struct tm date=*local;
    int *field;
    if (command==0x10 || command==0x11)
        field=selector==0 ? &date.tm_sec : (selector==1 ? &date.tm_min : &date.tm_hour);
    else field=selector==0 ? &date.tm_mday : (selector==1 ? &date.tm_mon : &date.tm_year);
    /* tm_mon comienza en cero; el protocolo devuelve meses de 1 a 12. */
    int month=command>=0x12 && selector==1;
    if (!writing) return *field+month;
    *field=value-month;
    date.tm_isdst=-1;
    time_t adjusted=mktime(&date);
    if (adjusted!=(time_t)-1) atmturbo2plus_clock_offset=adjusted-now;
    return 0xff;
}

/* Emula el protocolo 8031 por lecturas #FE: #55, comando y argumento opcional. */
z80_byte atmturbo2plus_read_port_fe(z80_int port,z80_byte value)
{
    int high=port>>8;
    int selector=high>>6;
    if (atmturbo2plus_keyboard_disabled) return value;
    if (zxvision_key_not_sent_emulated_mach()) {
        if (!atmturbo2plus_keyboard_phase && atmturbo2plus_keyboard_mode) return 0;
    }
    else atmturbo2plus_poll_keyboard();

    if (atmturbo2plus_keyboard_phase==2) {
        atmturbo2plus_keyboard_phase=0;
        int command=atmturbo2plus_keyboard_command&0x3f;
        if (command==8) atmturbo2plus_keyboard_mode=high&3;
        if (command==0x11 || command==0x13)
            return atmturbo2plus_clock(command,atmturbo2plus_keyboard_command>>6,high,1);
        return 0xff;
    }
    if (atmturbo2plus_keyboard_phase==1) {
        /* Versión XT 1.06 documentada: bytes 6, 0, 1, 0. */
        static const z80_byte version[4]={6,0,1,0};
        atmturbo2plus_keyboard_phase=0;
        atmturbo2plus_keyboard_command=high;
        switch (high&0x3f) {
            case 1: return version[selector];
            case 7:
                atmturbo2plus_keyboard_code=atmturbo2plus_keyboard_last=0;
                atmturbo2plus_keyboard_flags=0;
                return 0xff;
            case 8: case 0x11: case 0x13: case 0x14: case 0x15:
                atmturbo2plus_keyboard_phase=2;
                return 0xff;
            case 9:
                if (selector==0) return atmturbo2plus_get_keyboard_code();
                if (selector==1) return atmturbo2plus_keyboard_last;
                return selector==2 ? atmturbo2plus_keyboard_flags : 0;
            case 0x0a: atmturbo2plus_keyboard_flags|=0x80; return 0xff;
            case 0x0b: atmturbo2plus_keyboard_flags&=0x7f; return 0xff;
            case 0x0d: reset_cpu(); return 0xff;
            case 0x10: case 0x12: return atmturbo2plus_clock(high&0x3f,selector,0,0);
            /* Sin comunicación RS-232 pendiente ni señales de módem activas. */
            case 0x16: case 0x17: return 0;
            default: return 0xff;
        }
    }
    if (high==0x55) {
        atmturbo2plus_keyboard_phase=1;
        return 0xaa;
    }
    if (!atmturbo2plus_keyboard_mode) return value;
    if (atmturbo2plus_keyboard_mode==2 && selector)
        return selector==1 ? atmturbo2plus_keyboard_flags : 0;
    return atmturbo2plus_get_keyboard_code();
}

/* Permite acceder a los puertos ATM2 cuando está activo DOS o CP/M. */
int atmturbo2_shadow_ports_enabled(void)
{
    return MACHINE_IS_ATM_TURBO2_FAMILY &&
        (atmturbo_trdos_active || !(atmturbo2_port_77 & 0x200));
}

/* Lee IDE en los puertos de sombra: A5-A7 eligen registro y A8 el byte alto. */
z80_byte atmturbo2_ide_read_port(z80_int port)
{
    if (!ide_enabled.v || !ide_atm_enabled.v) return 0xff;
    if (port&0x100) return atmturbo2_ide_read_high;
    int reg=(port>>5)&7;
    z80_byte value=ide_read_command_block_register(reg);
    if (!reg) atmturbo2_ide_read_high=ide_read_command_block_register(0);
    return value;
}

/* Escribe IDE reteniendo primero el byte alto de cada palabra de datos. */
void atmturbo2_ide_write_port(z80_int port,z80_byte value)
{
    if (!ide_enabled.v || !ide_atm_enabled.v) return;
    if (port&0x100) {
        atmturbo2_ide_write_high=value;
        return;
    }
    int reg=(port>>5)&7;
    ide_write_command_block_register(reg,value);
    if (!reg) ide_write_command_block_register(0,atmturbo2_ide_write_high);
}

/* Ajusta la duración del cuadro de ATM2 según el bit RG0 del modo de vídeo. */
void atmturbo2_update_timing(void)
{
    /* La PLM CP6N.SRC genera VS en V=312 con RG0=1 y en V=316
       con RG0=0. Añadimos el tiempo al final para conservar la
       posición de inicio del área activa en ambos modos. */
    screen_invisible_borde_superior=8;
    screen_total_borde_inferior=(atmturbo_video_mode&1) ? 56 : 60;
    screen_set_video_params_indices();
}

/* Aplica el bit de turbo de #77 mediante el cambio de velocidad común de CPU. */
static void atmturbo2_set_cpu_speed(z80_byte value)
{
    int speed=(value&8) ? 2 : 1;
    if (cpu_turbo_speed==speed) return;
    cpu_turbo_speed=speed;
    cpu_set_turbo_speed();
}

/* Devuelve el estado de la puerta de interrupciones de cuadro de ATM2+. */
int atmturbo2plus_interrupts_enabled(void)
{
    return atmturbo2plus_int_enabled;
}

/* Guarda los controles de #77 y aplica el vídeo, el turbo y el mapa de memoria. */
void atmturbo2_out_port_77(z80_int port,z80_byte value)
{
    atmturbo2_port_77=port;
    if (MACHINE_IS_ATM_TURBO2_PLUS) {
        atmturbo2plus_keyboard_disabled=(value&0x40)!=0;
        atmturbo2plus_int_enabled=(value&0x20)!=0;
        if (!atmturbo2plus_int_enabled) interrupcion_maskable_generada.v=0;
    }
    atmturbo_video_mode=value&7;
    atmturbo2_update_timing();
    atmturbo2_set_cpu_speed(value);
    atmturbo2_refresh_memory();
    modificado_border.v=1;
}

/* Selecciona el índice de paleta del borde mediante los datos y A3 de #FE. */
void atmturbo2_out_port_fe(z80_int port,z80_byte value)
{
    atmturbo_border_colour=(value&7)|((port&8) ? 0 : 8);
}

/* Cambia el color del índice seleccionado con los bits RGB de #FF. */
void atmturbo2_out_port_ff(z80_byte value)
{
    int colour=value^0xff;
    int red=((colour&2) ? 2 : 0)|((colour&0x40) ? 1 : 0);
    int green=((colour&0x10) ? 2 : 0)|((colour&0x80) ? 1 : 0);
    int blue=((colour&1) ? 2 : 0)|((colour&0x20) ? 1 : 0);
    atmturbo_palette[atmturbo_border_colour]=
        SPECTRA_INDEX_FIRST_COLOR+green*16+red*4+blue;
    modificado_border.v=1;
}

/* Decide si #FF escribe la paleta en vez de acceder al controlador de disco. */
int atmturbo2_palette_port_enabled(void)
{
    return atmturbo2_shadow_ports_enabled() && !(atmturbo2_port_77&0x4000);
}

/* Programa una entrada de la MMU ATM2 elegida por A14-A15 y #7FFD.D4. */
void atmturbo2_out_port_f7(z80_int port,z80_byte value)
{
    int index=((puerto_32765 & 16) ? 4 : 0) | ((port>>14)&3);
    atmturbo2_mmu[index]=((((value&0xc0)<<2)|(value&0x3f))^0x33f);
    atmturbo2_refresh_memory();
}

/* Resuelve las cuatro entradas activas de la MMU en páginas de ROM o RAM. */
static void atmturbo2_refresh_memory(void)
{
    int ram_pages=MACHINE_IS_ATM_TURBO2_PLUS ? ATM_TURBO2_PLUS_TOTAL_RAM_PAGES : ATM_TURBO2_TOTAL_RAM_PAGES;
    int rom_pages=MACHINE_IS_ATM_TURBO2_PLUS ? ATM_TURBO2_PLUS_TOTAL_ROM_PAGES : ATM_TURBO2_TOTAL_ROM_PAGES;
    int segment;
    for (segment=0;segment<4;segment++) {
        unsigned int entry=atmturbo2_mmu[((puerto_32765&16) ? 4 : 0)+segment];
        unsigned int page=entry&0x3f;
        int ram=1;

        if (!(atmturbo2_port_77&0x100)) {
            /* PEN desactivado presenta la última página de la ROM instalada. */
            page=rom_pages-1;
            ram=0;
        }
        else switch (entry&0x300) {
        case 0x000:
            /* Página RAM cuyo trío inferior viene de #7FFD. */
            page=(page&~7)|(puerto_32765&7);
            break;
        case 0x100:
            /* Página ROM cuyo bit inferior sigue a DOS o CP/M. */
            page=(page&~1)|((atmturbo_trdos_active ||
                             !(atmturbo2_port_77&0x200)) ? 1 : 0);
            ram=0;
            break;
        case 0x200:
            /* Página RAM fija indicada por la entrada de la MMU. */
            break;
        case 0x300:
            /* Página ROM fija indicada por la entrada de la MMU. */
            ram=0;
            break;
        }

        if (ram) {
            page%=ram_pages;
            atmturbo_memory_paged[segment]=atmturbo_ram_mem_table[page];
            debug_paginas_memoria_mapeadas[segment]=page;
        }
        else {
            page%=rom_pages;
            atmturbo_memory_paged[segment]=atmturbo_rom_mem_table[page];
            debug_paginas_memoria_mapeadas[segment]=DEBUG_PAGINA_MAP_ES_ROM+page;
        }
        atmturbo2_segment_ram[segment]=ram;
    }
}

void atmturbo_mem_page_rom(void);

/* Comprueba si la CPU está ejecutando la ROM de TR-DOS en #0000-#3FFF. */
int atmturbo_trdos_is_active(void)
{
    return atmturbo_trdos_active && reg_pc<0x4000;
}

/* Determina si un acceso a los puertos de Beta Disk llega al VG93. */
int atmturbo_fdc_is_active(z80_int port,int reading)
{
    if (MACHINE_IS_ATM_TURBO2_FAMILY)
        return atmturbo2_shadow_ports_enabled();
    if (atmturbo_trdos_is_active()) return 1;
    if (!atmturbo_cpsys) return 0;
    /* La BIOS sondea #FF antes de seleccionar el disquete. */
    return !reading || (port & 0xff)!=0xff || atmturbo_fdc_selected;
}

/* Actualiza un color de la paleta de ATM1 a partir del valor de #7DFD. */
void atmturbo_out_port_7dfd(z80_byte value)
{
    int color=value ^ 0xff;
    int red=((color & 2) ? 2 : 0) | ((color & 0x10) ? 1 : 0);
    int green=((color & 4) ? 2 : 0) | ((color & 0x20) ? 1 : 0);
    int blue=((color & 1) ? 2 : 0) | ((color & 8) ? 1 : 0);
    int palette_colour=SPECTRA_INDEX_FIRST_COLOR+green*16+red*4+blue;
    atmturbo_palette[atmturbo_border_colour]=palette_colour;
    modificado_border.v=1;
}

/* Lee el VG93 si está habilitado o devuelve un estado de unidad no preparada. */
z80_byte atmturbo_fdc_read_port(z80_int port)
{
    if (trd_enabled.v) return vg93_read_port(port);

    /* Sin controlador habilitado, informa fin de comando y unidad no preparada. */
    if ((port & 0xff)==0x1f || (port & 0xff)==0xff) return 0x80;
    return 0xff;
}

/* Marca el acceso al VG93 y le envía la escritura si está habilitado. */
void atmturbo_fdc_write_port(z80_int port,z80_byte value)
{
    atmturbo_fdc_selected=1;
    if (trd_enabled.v) vg93_write_port(port,value);
}

/* Detecta la entrada y salida automáticas de TR-DOS durante el fetch. */
z80_byte fetch_opcode_atmturbo(void)
{
    if (MACHINE_IS_ATM_TURBO2_FAMILY) {
        /* El fetch en #3Dxx activa TR-DOS y salir de la ROM lo desactiva. */
        if (atmturbo_trdos_active && reg_pc>=0x4000) {
            atmturbo_trdos_active=0;
            atmturbo2_refresh_memory();
        }
        else if (!atmturbo_trdos_active && (puerto_32765&16) &&
                 !atmturbo2_segment_ram[0] &&
                 (reg_pc&0xff00)==0x3d00) {
            atmturbo_trdos_active=1;
            atmturbo2_refresh_memory();
        }
        if (atmturbo_trdos_active) betadisk_handle_trdos_traps();
        return fetch_opcode_spectrum();
    }
    if (atmturbo_trdos_active && reg_pc>=0x4000) {
        atmturbo_trdos_active=0;
        atmturbo_mem_page_rom();
    }
    else if (!atmturbo_trdos_active && !atmturbo_cpsys && atmturbo_rom_enabled &&
             (puerto_32765 & 16) && (reg_pc & 0xff00)==0x3d00) {
        atmturbo_trdos_active=1;
        atmturbo_mem_page_rom();
    }
    if (atmturbo_trdos_active) betadisk_handle_trdos_traps();
    return fetch_opcode_spectrum();
}


/* Divide la memoria de la máquina en páginas físicas de ROM y RAM de 16 KB. */
void atmturbo_init_memory_tables(void)
{
    //Primero rom y luego ram

    int i;
    z80_byte *puntero=memoria_spectrum;

    for (i=0;i<ATM_TURBO_TOTAL_ROM_PAGES;i++) {
        atmturbo_rom_mem_table[i]=puntero;
        puntero +=16384;
    }

    for (i=0;i<ATM_TURBO_TOTAL_RAM_PAGES;i++) {
        atmturbo_ram_mem_table[i]=puntero;
        puntero +=16384;
    }


}

/* Instala el mapa inicial de segmentos antes de aplicar los puertos ATM. */
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


/* Reserva las páginas ATM y carga la ROM de caracteres usada por ATM2. */
void atmturbo_malloc_mem_machine(void)
{


    int total_ram=(ATM_TURBO_MAX_ROM_SIZE+ATM_TURBO_MAX_RAM_SIZE)*1024;

    malloc_machine(total_ram);


    atmturbo_init_memory_tables();
    atmturbo_set_normal_pages();
    if (MACHINE_IS_ATM_TURBO2_FAMILY) {
        FILE *font_file;
        open_sharedfile("sgen.rom",&font_file);
        if (!font_file || fread(atmturbo2_text_font,1,sizeof(atmturbo2_text_font),font_file)!=sizeof(atmturbo2_text_font))
            cpu_panic("Cannot load ATM Turbo 2 character ROM");
        fclose(font_file);
    }
}

/* Actualiza la RAM visible: #C000 en ATM1 o los cuatro segmentos en ATM2. */
void atmturbo_mem_page_ram(void)
{
    if (MACHINE_IS_ATM_TURBO2_FAMILY) {
        atmturbo2_refresh_memory();
        return;
    }

    z80_byte page_entra=puerto_32765 & 7;
    z80_byte page_fdfd=puerto_fdfd &3;

    int pagina=(page_fdfd)*8 + page_entra;


    atmturbo_memory_paged[3]=atmturbo_ram_mem_table[pagina];
    debug_paginas_memoria_mapeadas[3]=page_entra;
}

/* Selecciona BIOS, TR-DOS o BASIC de ATM1; ATM2 usa su MMU completa. */
void atmturbo_mem_page_rom(void)
{
    if (MACHINE_IS_ATM_TURBO2_FAMILY) {
        atmturbo2_refresh_memory();
        return;
    }

    z80_byte page_entra;

    /* La imagen ATM1 contiene BIOS, TR-DOS, BASIC 128 y BASIC 48. */
    if (atmturbo_trdos_active) page_entra=1;
    else if (atmturbo_cpsys) page_entra=0;
    else page_entra=(puerto_32765 & 16) ? 3 : 2;

    if (atmturbo_rom_enabled) {
        atmturbo_memory_paged[0]=atmturbo_rom_mem_table[page_entra];
        debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+page_entra;
    }
    else {
        atmturbo_memory_paged[0]=atmturbo_ram_mem_table[0];
        debug_paginas_memoria_mapeadas[0]=0;
    }

}

/* Aplica #7FFD y respeta el bloqueo de paginado de su bit 5. */
void atmturbo_out_port_32765(z80_byte value)
{
    if (puerto_32765 & 32) return;

    puerto_32765=value;

    if (MACHINE_IS_ATM_TURBO1 && (value & 32)) {
        atmturbo_cpsys=0;
        atmturbo_fdc_selected=0;
    }

    atmturbo_mem_page_ram();
    atmturbo_mem_page_rom();
}

/* Selecciona el banco de RAM adicional de ATM1 mediante #FDFD. */
void atmturbo_out_port_fdfd(z80_byte value)
{
    puerto_fdfd=value;

    atmturbo_mem_page_ram();
    atmturbo_mem_page_rom();
}

/* Restablece la paleta, los modos, los puertos y el mapa de arranque ATM. */
void atmturbo_reset(void)
{
    int i;
    for (i=0;i<16;i++) atmturbo_palette[i]=i;
    atmturbo_trdos_active=0;
    atmturbo_fdc_selected=0;
    atmturbo_cpsys=1;
    atmturbo_rom_enabled=1;
    atmturbo_video_mode=3;
    atmturbo_border_colour=0;
    atmturbo_border_left_next_valid=0;
    if (MACHINE_IS_ATM_TURBO2_FAMILY) {
        atmturbo2plus_keyboard_mode=atmturbo2plus_keyboard_phase=0;
        atmturbo2plus_keyboard_code=atmturbo2plus_keyboard_last=0;
        atmturbo2plus_keyboard_flags=0;
        atmturbo2plus_keyboard_disabled=0;
        atmturbo2plus_int_enabled=0;
        atmturbo2_ide_read_high=atmturbo2_ide_write_high=0xff;
        for (i=0;i<8;i++) atmturbo2_xt_rows[i]=0xff;
        atmturbo2_xt_data=0xff;
        atmturbo2_xt_reads=0;
        puerto_32765=0;
        atmturbo2_port_77=0;
        for (i=0;i<8;i++) atmturbo2_mmu[i]=0;
        atmturbo2_update_timing();
        atmturbo2_set_cpu_speed(0);
    }
    modificado_border.v=1;
    vg93_reset();

    atmturbo_memory_paged[1]=atmturbo_ram_mem_table[5];
    debug_paginas_memoria_mapeadas[1]=5;

    atmturbo_mem_page_rom();
    atmturbo_mem_page_ram();
}

/* ATM1 selecciona borde, modo gráfico y presencia de ROM con #FE. */
void atmturbo_out_port_fe(z80_int port,z80_byte value)
{
    /* El índice de paleta lo seleccionan D0-D2 y A3 de #FE. */
    atmturbo_border_colour=(value&7) | ((port&8) ? 0 : 8);
    atmturbo_rom_enabled=(port & 0x80)!=0;
    atmturbo_video_mode=(port >> 5) & 3;
    atmturbo_memory_paged[1]=atmturbo_ram_mem_table[atmturbo_rom_enabled ? 5 : 4];
    debug_paginas_memoria_mapeadas[1]=atmturbo_rom_enabled ? 5 : 4;
    atmturbo_mem_page_rom();
}

/* Devuelve el índice de paleta que usa actualmente el borde. */
z80_byte atmturbo_get_border_colour(void)
{
    return atmturbo_border_colour;
}

/*
   Señal Z del hardware ATM (distinta del flag Z del registro F del Z80).

   La lógica programable de vídeo combina bits de los contadores horizontal
   y vertical, el modo de pantalla y otras señales del circuito para generar
   un nivel eléctrico que cambia en determinados momentos del barrido.
   CP45.SRC contiene las ecuaciones de Z; CP6N.SRC define las señales internas
   Z1 y Z2. Estos fuentes de la PLM están en codex_additional/atm-plms/PRO.

   La CPU consulta este nivel al leer #FE: D7 en ATM1 y D5 en ATM2. Cada IN
   obtiene el nivel de ese instante; varias lecturas separadas por instrucciones
   forman una secuencia que depende del barrido y de la velocidad de ejecución.

   La ROM 1.06 de ATM2 utiliza esa secuencia para descifrar el código de CP/M
   almacenado en la propia ROM antes de ejecutarlo desde RAM:
   - Se sincroniza con la interrupción de vídeo.
   - Espera mediante bucles de instrucciones y muestrea Z repetidamente.
   - Acumula los bits en HL y usa ese valor como clave del descifrado.
   En el arranque comprobado, la clave correcta es #3F6D. Cuando Z permanecía
   siempre alta, obtenía #FFFF y dejaba código incorrecto en RAM, provocando
   un cuelgue o un retorno al menú. Esto actúa como una protección ligada al
   hardware y sus tiempos: emular solo las instrucciones Z80 no es suficiente.

   Aquí aproximamos Z mediante intervalos de T-estados ajustados a las rutinas
   comprobadas de las ROM; no reproducimos íntegramente las ecuaciones de la
   PLM. Las ventanas adicionales de ATM2 están en atmturbo2_read_port_fe().
   Esta función indica si el nivel es bajo para el instante y cuadro recibidos.
*/
static int atmturbo_z_signal_low(int t,int frame_states)
{
    int low;
    if (frame_states<80000) {
        low=(t>=7200 && t<7240) ||
            (t>=7284 && t<7324) ||
            (t>=7326 && t<7366);
    }
    else {
        low=(t>=21514 && t<21554) ||
            (t>=21703 && t<21783) ||
            (t>=21808 && t<21848);
    }
    return low;
}

/* Inserta la señal Z de ATM1 en D7 de la lectura de teclado #FE. */
z80_byte atmturbo_read_port_fe(z80_byte value)
{
    return (value & 0x7f) |
           (atmturbo_z_signal_low(t_estados,screen_testados_total) ? 0 : 0x80);
}

/* Inserta la señal Z de ATM2 en D5 de la lectura de teclado #FE. */
z80_byte atmturbo2_read_port_fe(z80_byte value)
{
    /* La INT de ATM2 cae 3565 T después del origen del barrido.
       Reutilizamos la señal Z de ATM1, desplazada a ese origen y expresada
       en ciclos de 3,5 MHz para que el turbo no desplace los pulsos. */
    int t=t_estados/cpu_turbo_speed+3565;
    int low=atmturbo_z_signal_low(t,screen_testados_total/cpu_turbo_speed);

    /* La aproximación de pulsos anterior solo cubre la protección Spectrum.
       Con RG0=0, Z2 también cambia durante las líneas 48-51. Estos pulsos
       permiten que la ROM 1.06 descifre el CCP/BDOS de CP/M: con Z siempre
       alta obtiene FFFF y ejecuta datos sin descifrar al terminar el arranque.
       Sigue siendo una aproximación temporal, no la lógica completa de CP6N. */
    if (!(atmturbo_video_mode&1)) {
        low |= (t>=10816 && t<10912) ||
               (t>=11232 && t<11264) ||
               (t>=11392 && t<11424) ||
               (t>=11552 && t<11584);
    }
    return (value & 0xdf) | (low ? 0 : 0x20);
}



/* Escribe en la página RAM visible sin añadir los T-estados del acceso. */
void poke_byte_no_time_atmturbo(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (MACHINE_IS_ATM_TURBO2_FAMILY ? atmturbo2_segment_ram[segmento] :
        (dir>16383 || !atmturbo_rom_enabled)) {
		poke_byte_spectrum_sync_rainbow(dir);
#ifdef EMULATE_VISUALMEM

        set_visualmembuffer(dir);

#endif
        dir = dir & 16383;
        puntero=atmturbo_memory_paged[segmento]+dir;

        *puntero=valor;
    }
}


/* Escribe en la página RAM visible contando el acceso de la CPU. */
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



    if (MACHINE_IS_ATM_TURBO2_FAMILY ? atmturbo2_segment_ram[segmento] :
        (dir>16383 || !atmturbo_rom_enabled)) {
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




/* Lee la página activa sin consumir T-estados de la CPU. */
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


/* Lee la página activa y añade los T-estados de memoria. */
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


/* La lectura de #FB activa la BIOS CP/M de ATM1 y abandona TR-DOS. */
z80_byte atmturbo_read_port_fb(void)
{
    if (!atmturbo_cpsys) atmturbo_fdc_selected=0;
    atmturbo_trdos_active=0;
    atmturbo_cpsys=1;

    atmturbo_mem_page_rom();

    return 255;

}

/* La lectura de #7B vuelve de CP/M a las ROM Spectrum de ATM1. */
z80_byte atmturbo_read_port_7b(void)
{
    atmturbo_trdos_active=0;
    atmturbo_cpsys=0;
    atmturbo_fdc_selected=0;

    atmturbo_mem_page_rom();

    return 255;

}



/* Lee un píxel de los modos gráficos 320x200 o 640x200 de ATM1 y ATM2. */
static int atmturbo_get_graphics_colour(int x,int y,int mode)
{
    int base=(puerto_32765 & 8) ? 7 : 5;
    z80_byte *pixels=atmturbo_ram_mem_table[base];
    z80_byte *attributes=atmturbo_ram_mem_table[base-4];
    int offset=y*40+(mode==0 ? x/8 : x/16);
    int colour;

    if (mode==0) {
        /* 320x200 reparte parejas de píxeles entre dos páginas y mitades. */
        int part=(x & 7)/2;
        z80_byte value=atmturbo_ram_mem_table[(part & 1) ? base : base-4]
                                             [offset+((part & 2) ? 8192 : 0)];
        colour=(x & 1) ? ((value & 0x80)>>4)|((value>>3)&7)
                       : ((value & 0x40)>>3)|(value&7);
    }
    else {
        /* 640x200 usa bytes de píxeles y atributos en páginas separadas. */
        int byte_offset=offset+((x & 8) ? 8192 : 0);
        z80_byte value=pixels[byte_offset];
        z80_byte attribute=attributes[byte_offset];
        colour=(value & (0x80>>(x & 7)))
               ? ((attribute & 0x40)>>3)|(attribute&7)
               : ((attribute & 0x80)>>4)|((attribute>>3)&7);
    }

    return atmturbo_palette[colour];
}

/* Lee un píxel del modo de texto de 80 columnas de ATM2. */
static int atmturbo2_get_text_colour(int x,int y)
{
    int page=(puerto_32765&8) ? 7 : 5;
    int column=x>>3;
    int address=0x1c0+(y>>3)*64+(column>>1);
    z80_byte character,attribute;
    int ink,paper;

    /* Las columnas pares e impares intercambian página y mitad de 8 KB. */
    if (column&1) {
        character=atmturbo_ram_mem_table[page][address+0x2000];
        attribute=atmturbo_ram_mem_table[page-4][address+1];
    }
    else {
        character=atmturbo_ram_mem_table[page][address];
        attribute=atmturbo_ram_mem_table[page-4][address+0x2000];
    }
    ink=(attribute&7)|((attribute&0x40)>>3);
    paper=((attribute>>3)&7)|((attribute&0x80)>>4);
    return atmturbo_palette[(atmturbo2_text_font[character*8+(y&7)] &
                            (0x80>>(x&7))) ? ink : paper];
}

/* Obtiene el píxel del modo Spectrum para la línea de vídeo actual. */
static int atmturbo_get_standard_colour(int x,int y)
{
    z80_byte *screen=get_base_mem_pantalla();
    int column=x>>3;
    int attribute=screen[6144+(y>>3)*32+column];
    int pixels=screen[screen_addr_table[y<<5]+column];
    int ink,paper;

    if (scr_refresca_sin_colores.v) attribute=56;
    else if (scr_refresca_show_attribute_grid.v)
        attribute=56+((((y>>3)+column)&1) ? 64 : 0);

    ink=(attribute&7)+((attribute&64) ? 8 : 0);
    paper=((attribute>>3)&7)+((attribute&64) ? 8 : 0);
    if ((attribute&128) && estado_parpadeo.v) {
        int aux=ink;
        ink=paper;
        paper=aux;
    }
    return atmturbo_palette[(pixels&(0x80>>(x&7))) ? ink : paper];
}

/* Dibuja el borde de una línea en vídeo real y conserva su color final. */
void atmturbo_store_scanline_rainbow_border(void)
{
    z80_byte state_colour[MAX_STATES_LINE];
    z80_byte colour=screen_border_last_color;
    int standard=(atmturbo_video_mode==3);
    int source_height=standard ? 192 : 200;
    int top_border=(288-source_height)/2;
    int visible_y=t_scanline_draw-(screen_indice_inicio_pant-top_border);
    int active_y=t_scanline_draw-screen_indice_inicio_pant;
    int total_width=get_total_ancho_rainbow();
    int left_width=standard ? 104 : 40;
    int right_start=total_width-left_width;
    int line_start=t_scanline*screen_testados_linea;
    int pre_retrace=screen_testados_indice_borde_derecho*cpu_turbo_speed;
    int post_retrace_start=screen_testados_linea-
                           (screen_total_borde_izquierdo/2)*cpu_turbo_speed;
    int line_states=screen_testados_linea;
    int x,state,duplicate;

    if (pre_retrace>line_states) pre_retrace=line_states;
    if (post_retrace_start<pre_retrace) post_retrace_start=pre_retrace;
    if (post_retrace_start>=line_states) post_retrace_start=line_states-1;

    for (state=0;state<line_states;state++) {
        z80_byte change=fullbuffer_border[line_start+state];
        if (change!=255) colour=change;
        state_colour[state]=colour;
    }
    screen_border_last_color=colour;

    /* El borde visible usa el color retenido de la línea anterior a la izquierda. */
    if (visible_y>=0 && visible_y<288) {
        int output_y=visible_y*2;
        int central_line=(active_y>=0 && active_y<source_height);
        for (x=0;x<total_width;x++) {
            z80_byte raw_colour;
            if (central_line && x>=left_width && x<right_start) continue;
            if (x<left_width) {
                raw_colour=atmturbo_border_left_next_valid ?
                    atmturbo_border_left_next[(x*total_width)/left_width] :
                    state_colour[0];
            }
            else {
                int position=x-left_width;
                int length=total_width-left_width;
                state=(position*pre_retrace)/length;
                if (state>=pre_retrace) state=pre_retrace-1;
                raw_colour=state_colour[state];
            }
            for (duplicate=0;duplicate<2;duplicate++)
                rainbow_buffer[(output_y+duplicate)*total_width+x]=
                    atmturbo_palette[scr_refresca_sin_colores.v ? 7 : (raw_colour&15)];
        }
    }

    /* Conserva los colores del final del barrido para la línea siguiente. */
    for (x=0;x<total_width;x++) {
        state=post_retrace_start+(x*(line_states-post_retrace_start))/total_width;
        if (state>=line_states) state=line_states-1;
        atmturbo_border_left_next[x]=state_colour[state];
    }
    atmturbo_border_left_next_valid=1;
}

/* Dibuja una línea activa en vídeo real según el modo gráfico o de texto. */
void atmturbo_store_scanline_rainbow_display(void)
{
    int standard=(atmturbo_video_mode==3);
    int width=standard ? 256 : (atmturbo_video_mode==0 ? 320 : 640);
    int height=standard ? 192 : 200;
    int active_y=t_scanline_draw-screen_indice_inicio_pant;
    int total_width=get_total_ancho_rainbow();
    int scale_x=(width==640) ? 1 : 2;
    int offset_x=(total_width-width*scale_x)/2;
    int output_y=(border_enabled.v ? ((288-height)/2)*2 :
                  (ATM_TURBO_DISPLAY_HEIGHT-height*2)/2)+active_y*2;
    int x,duplicate;

    if (active_y<0 || active_y>=height) return;

    if (!border_enabled.v && active_y==0) {
        int position;
        int border_colour=atmturbo_palette[atmturbo_border_colour];
        for (position=0;position<total_width*ATM_TURBO_DISPLAY_HEIGHT;position++)
            rainbow_buffer[position]=border_colour;
    }

    for (x=0;x<width;x++) {
        int colour=standard ? atmturbo_get_standard_colour(x,active_y) :
            (MACHINE_IS_ATM_TURBO2_FAMILY && atmturbo_video_mode==6 ?
             atmturbo2_get_text_colour(x,active_y) :
             (MACHINE_IS_ATM_TURBO1 && atmturbo_video_mode==2 ? 0 :
              atmturbo_get_graphics_colour(x,active_y,atmturbo_video_mode)));
        int output_x=offset_x+x*scale_x;
        for (duplicate=0;duplicate<2;duplicate++) {
            z80_int *pixel=&rainbow_buffer[(output_y+duplicate)*total_width+output_x];
            pixel[0]=colour;
            if (scale_x==2) pixel[1]=colour;
        }
    }
}

/* Dibuja un píxel lógico con escala entera dentro del área de 640x400. */
static void atmturbo_putpixel_scaled(int x,int y,int colour,
                                    int scale_x,int scale_y,int offset_x,int offset_y)
{
    int dx,dy;
    int output_x=offset_x+x*scale_x;
    int output_y=offset_y+y*scale_y;

    for (dy=0;dy<scale_y;dy++)
        for (dx=0;dx<scale_x;dx++)
            scr_putpixel_zoom(output_x+dx,output_y+dy,colour);
}

/* Dibuja el modo Spectrum en 512x384, centrado dentro de 640x400. */
static void atmturbo_refresca_pantalla_standard(void)
{
    z80_byte *screen=get_base_mem_pantalla();
    int x,y;

    for (y=0;y<192;y++) {
        for (x=0;x<256;x++) {
            int column=x>>3;
            int attribute=screen[6144+(y>>3)*32+column];
            int pixels=screen[screen_addr_table[y<<5]+column];
            int ink,paper;

            if (scr_refresca_sin_colores.v) attribute=56;
            else if (scr_refresca_show_attribute_grid.v)
                attribute=56+((((y>>3)+column)&1) ? 64 : 0);

            ink=(attribute&7)+((attribute&64) ? 8 : 0);
            paper=((attribute>>3)&7)+((attribute&64) ? 8 : 0);
            if ((attribute&128) && estado_parpadeo.v) {
                int aux=ink;
                ink=paper;
                paper=aux;
            }

            atmturbo_putpixel_scaled(x,y,
                atmturbo_palette[(pixels&(0x80>>(x&7))) ? ink : paper],
                2,2,64,8);
        }
    }
}

/* Rellena el borde exterior y el espacio libre del modo estándar. */
static void atmturbo_refresca_border(void)
{
    int standard=(atmturbo_video_mode==3);
    int outer_left=ATM_TURBO_LEFT_BORDER_NO_ZOOM*border_enabled.v;
    int outer_top=ATM_TURBO_TOP_BORDER_NO_ZOOM*border_enabled.v;
    int total_width=ATM_TURBO_DISPLAY_WIDTH+
        (ATM_TURBO_LEFT_BORDER_NO_ZOOM+ATM_TURBO_RIGHT_BORDER_NO_ZOOM)*border_enabled.v;
    int total_height=ATM_TURBO_DISPLAY_HEIGHT+
        (ATM_TURBO_TOP_BORDER_NO_ZOOM+ATM_TURBO_BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;
    int left=outer_left+(standard ? 64 : 0);
    int top=outer_top+(standard ? 8 : 0);
    int right=left+(standard ? 512 : 640);
    int bottom=top+(standard ? 384 : 400);
    int colour=atmturbo_palette[scr_refresca_sin_colores.v ? 7 : atmturbo_border_colour];
    int x,y,dx,dy;

    for (y=0;y<total_height;y++) {
        for (x=0;x<total_width;x++) {
            if (x>=left && x<right && y>=top && y<bottom) continue;
            for (dy=0;dy<zoom_y;dy++)
                for (dx=0;dx<zoom_x;dx++)
                    scr_putpixel(x*zoom_x+dx,y*zoom_y+dy,colour);
        }
    }
}

/* Refresca la pantalla ATM y redibuja el borde normal cuando ha cambiado. */
void atmturbo_refresca_pantalla(void)
{
    if (rainbow_enabled.v) {
        scr_refresca_pantalla_rainbow_comun();
    }

    else {

        if (atmturbo_video_mode==3) {
            atmturbo_refresca_pantalla_standard();
        }
        else {
            int x,y;
            int width=(atmturbo_video_mode==0) ? 320 : 640;
            int scale_x=(width==320) ? 2 : 1;

            for (y=0;y<200;y++) {
                for (x=0;x<width;x++) {
                int colour=(MACHINE_IS_ATM_TURBO2_FAMILY && atmturbo_video_mode==6) ?
                    atmturbo2_get_text_colour(x,y) :
                    (MACHINE_IS_ATM_TURBO1 && atmturbo_video_mode==2 ? 0 :
                     atmturbo_get_graphics_colour(x,y,atmturbo_video_mode));
                    atmturbo_putpixel_scaled(x,y,colour,scale_x,2,0,0);
                }
            }
        }

        if (modificado_border.v) {
            atmturbo_refresca_border();
            modificado_border.v=0;
        }
    }
}
