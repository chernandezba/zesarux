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
z80_byte *atmturbo_ram_mem_table[ATMTURBO_TOTAL_RAM_PAGES];

//Donde esta cada pagina de rom
z80_byte *atmturbo_rom_mem_table[ATMTURBO_TOTAL_ROM_PAGES];

//Direcciones actuales mapeadas
z80_byte *atmturbo_memory_paged[4];

z80_byte puerto_fdfd;

/* CPSYS cambia al leer #FB/#7B; no forma parte del registro #7FFD. */
static int atmturbo_cpsys;
static int atmturbo_rom_enabled=1;
static int atmturbo_trdos_active;
static int atmturbo_video_mode=3;
static int atmturbo_palette[16];

void atmturbo_mem_page_rom(void);

int atmturbo_trdos_is_active(void)
{
    return atmturbo_trdos_active && reg_pc<0x4000;
}

void atmturbo_out_port_7dfd(z80_int port,z80_byte value)
{
    int index=(out_254 & 7) | ((port & 8) ? 0 : 8);
    int color=value ^ 0xff;
    int red=((color & 2) ? 2 : 0) | ((color & 0x40) ? 1 : 0);
    int green=((color & 0x10) ? 2 : 0) | ((color & 0x80) ? 1 : 0);
    int blue=((color & 1) ? 2 : 0) | ((color & 0x20) ? 1 : 0);
    atmturbo_palette[index]=SPECTRA_INDEX_FIRST_COLOR+green*16+red*4+blue;
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

    if (value & 32) atmturbo_cpsys=0;

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
    atmturbo_cpsys=1;
    atmturbo_rom_enabled=1;
    atmturbo_video_mode=3;
    vg93_reset();

    atmturbo_memory_paged[1]=atmturbo_ram_mem_table[5];
    debug_paginas_memoria_mapeadas[1]=5;

    atmturbo_mem_page_rom();
    atmturbo_mem_page_ram();
}

void atmturbo_out_port_fe(z80_int port)
{
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
    atmturbo_trdos_active=0;
    atmturbo_cpsys=1;

    atmturbo_mem_page_rom();

    return 255;

}

z80_byte atmturbo_read_port_7b(void)
{
    atmturbo_trdos_active=0;
    atmturbo_cpsys=0;

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

/* Promedia el área completa de origen cubierta por un píxel de salida. */
static int atmturbo_get_scaled_colour(int x,int y,int mode)
{
    int width=(mode==0) ? 320 : 640;
    int left=x*width;
    int right=(x+1)*width;
    int top=y*200;
    int bottom=(y+1)*200;
    int red=0,green=0,blue=0;
    int source_y,source_x;
    int area=width*200;

    for (source_y=top/192;source_y<=(bottom-1)/192;source_y++) {
        int y0=source_y*192;
        int y1=y0+192;
        int overlap_y=(bottom<y1 ? bottom : y1)-(top>y0 ? top : y0);

        for (source_x=left/256;source_x<=(right-1)/256;source_x++) {
            int x0=source_x*256;
            int x1=x0+256;
            int overlap_x=(right<x1 ? right : x1)-(left>x0 ? left : x0);
            int weight=overlap_x*overlap_y;
            int colour=atmturbo_get_graphics_colour(source_x,source_y,mode);
            int rgb=spectrum_colortable_normal[colour];
            red+=((rgb>>16)&255)*weight;
            green+=((rgb>>8)&255)*weight;
            blue+=(rgb&255)*weight;
        }
    }

    red=(red+area/2)/area;
    green=(green+area/2)/area;
    blue=(blue+area/2)/area;
    return RGB9_INDEX_FIRST_COLOR+(((red*7+127)/255)<<6)+
           (((green*7+127)/255)<<3)+((blue*7+127)/255);
}

void atmturbo_refresca_pantalla(void)
{

    if (atmturbo_video_mode==3) {

                        //modo clasico. sin rainbow
                if (rainbow_enabled.v==0) {
                        if (border_enabled.v) {
                                //ver si hay que refrescar border
                                if (modificado_border.v)
                                {
                                        scr_refresca_border();
                                        modificado_border.v=0;
                                }

                        }

                        scr_refresca_pantalla_comun();
                }

                else {
                //modo rainbow - real video
                        scr_refresca_pantalla_rainbow_comun_spectrum();
                }


    }

    else {

        int x,y;

        if (border_enabled.v && modificado_border.v) {
            scr_refresca_border();
            modificado_border.v=0;
        }

        for (y=0;y<192;y++) {
            for (x=0;x<256;x++) {
                int colour=0;
                if (atmturbo_video_mode!=2) {
                    colour=atmturbo_get_scaled_colour(x,y,atmturbo_video_mode);
                }
                scr_putpixel_zoom(x,y,colour);
            }
        }

    }

}
