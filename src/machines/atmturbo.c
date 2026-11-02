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
#include "vg93.h"



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

z80_byte puerto_fdfd;

/* CPSYS cambia al leer #FB/#7B; no forma parte del registro #7FFD. */
static int atmturbo_cpsys;
static int atmturbo_rom_enabled=1;
static int atmturbo_trdos_active;
static int atmturbo_video_mode=3;
static int atmturbo_border_colour;
static int atmturbo_palette[16];
static int atmturbo_fdc_selected;

void atmturbo_mem_page_rom(void);

int atmturbo_trdos_is_active(void)
{
    return atmturbo_trdos_active && reg_pc<0x4000;
}

int atmturbo_fdc_is_active(z80_int port,int reading)
{
    if (atmturbo_trdos_is_active()) return 1;
    if (!atmturbo_cpsys) return 0;
    /* La BIOS sondea #FF antes de seleccionar el disquete. */
    return !reading || (port & 0xff)!=0xff || atmturbo_fdc_selected;
}

void atmturbo_out_port_7dfd(z80_byte value)
{
    int color=value ^ 0xff;
    int red=((color & 2) ? 2 : 0) | ((color & 0x10) ? 1 : 0);
    int green=((color & 4) ? 2 : 0) | ((color & 0x20) ? 1 : 0);
    int blue=((color & 1) ? 2 : 0) | ((color & 8) ? 1 : 0);
    atmturbo_palette[atmturbo_border_colour]=
        SPECTRA_INDEX_FIRST_COLOR+green*16+red*4+blue;
}

z80_byte atmturbo_fdc_read_port(z80_int port)
{
    if (trd_enabled.v) return vg93_read_port(port);

    /* Sin disquete, el controlador informa fin de comando y unidad no preparada. */
    if ((port & 0xff)==0x1f || (port & 0xff)==0xff) return 0x80;
    return 0xff;
}

void atmturbo_fdc_write_port(z80_int port,z80_byte value)
{
    atmturbo_fdc_selected=1;
    if (trd_enabled.v) vg93_write_port(port,value);
}

z80_byte fetch_opcode_atmturbo(void)
{
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


    int total_ram=(ATM_TURBO_ROM_SIZE+ATM_TURBO_RAM_SIZE)*1024;

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

void atmturbo_out_port_32765(z80_byte value)
{
    if (puerto_32765 & 32) return;

    puerto_32765=value;

    if (value & 32) {
        atmturbo_cpsys=0;
        atmturbo_fdc_selected=0;
    }

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
    int i;
    for (i=0;i<16;i++) atmturbo_palette[i]=i;
    atmturbo_trdos_active=0;
    atmturbo_fdc_selected=0;
    atmturbo_cpsys=1;
    atmturbo_rom_enabled=1;
    atmturbo_video_mode=3;
    atmturbo_border_colour=0;
    vg93_reset();

    atmturbo_memory_paged[1]=atmturbo_ram_mem_table[5];
    debug_paginas_memoria_mapeadas[1]=5;

    atmturbo_mem_page_rom();
    atmturbo_mem_page_ram();
}

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

z80_byte atmturbo_read_port_fe(z80_byte value)
{
    int t=t_estados;
    int low;
    if (screen_testados_total<80000) {
        low=(t>=7200 && t<7240) ||
            (t>=7284 && t<7324) ||
            (t>=7326 && t<7366);
    }
    else {
        low=(t>=21514 && t<21554) ||
            (t>=21703 && t<21783) ||
            (t>=21808 && t<21848);
    }
    return (value & 0x7f) | (low ? 0 : 0x80);
}



void poke_byte_no_time_atmturbo(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (dir>16383 || !atmturbo_rom_enabled) {
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



    if (dir>16383 || !atmturbo_rom_enabled) {
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
    if (!atmturbo_cpsys) atmturbo_fdc_selected=0;
    atmturbo_trdos_active=0;
    atmturbo_cpsys=1;

    atmturbo_mem_page_rom();

    return 255;

}

z80_byte atmturbo_read_port_7b(void)
{
    atmturbo_trdos_active=0;
    atmturbo_cpsys=0;
    atmturbo_fdc_selected=0;

    atmturbo_mem_page_rom();

    return 255;

}



/* Obtiene un color del modo gráfico de ATM1 en sus coordenadas originales. */
static int atmturbo_get_graphics_colour(int x,int y,int mode)
{
    int base=(puerto_32765 & 8) ? 7 : 5;
    z80_byte *pixels=atmturbo_ram_mem_table[base];
    z80_byte *attributes=atmturbo_ram_mem_table[base-4];
    int offset=y*40+(mode==0 ? x/8 : x/16);
    int colour;

    if (mode==0) {
        int part=(x & 7)/2;
        z80_byte value=atmturbo_ram_mem_table[(part & 1) ? base : base-4]
                                             [offset+((part & 2) ? 8192 : 0)];
        colour=(x & 1) ? ((value & 0x80)>>4)|((value>>3)&7)
                       : ((value & 0x40)>>3)|(value&7);
    }
    else {
        int byte_offset=offset+((x & 8) ? 8192 : 0);
        z80_byte value=pixels[byte_offset];
        z80_byte attribute=attributes[byte_offset];
        colour=(value & (0x80>>(x & 7)))
               ? ((attribute & 0x40)>>3)|(attribute&7)
               : ((attribute & 0x80)>>4)|((attribute>>3)&7);
    }

    return atmturbo_palette[colour];
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

void atmturbo_refresca_pantalla(void)
{
    if (atmturbo_video_mode==3) {
        atmturbo_refresca_pantalla_standard();
    }
    else {
        int x,y;
        int width=(atmturbo_video_mode==0) ? 320 : 640;
        int scale_x=(width==320) ? 2 : 1;

        for (y=0;y<200;y++) {
            for (x=0;x<width;x++) {
                int colour=(atmturbo_video_mode==2) ? 0 :
                    atmturbo_get_graphics_colour(x,y,atmturbo_video_mode);
                atmturbo_putpixel_scaled(x,y,colour,scale_x,2,0,0);
            }
        }
    }
    atmturbo_refresca_border();
    modificado_border.v=0;
}
