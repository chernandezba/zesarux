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

z80_byte puerto_fdfd;

/* CPSYS cambia al leer #FB/#7B; no forma parte del registro #7FFD. */
static int atmturbo_cpsys;
static int atmturbo_rom_enabled=1;
static int atmturbo_trdos_active;
static int atmturbo_video_mode=3;
static int atmturbo_border_colour;
static z80_byte atmturbo_border_left_next[ATM_TURBO_DISPLAY_WIDTH+2*ATM_TURBO_LEFT_BORDER_NO_ZOOM];
static int atmturbo_border_left_next_valid;
static int atmturbo_palette[16];
static int atmturbo_fdc_selected;
static unsigned int atmturbo2_mmu[8];
static z80_int atmturbo2_port_77;
static int atmturbo2_segment_ram[4];
static z80_byte atmturbo2_text_font[2048];

static void atmturbo2_refresh_memory(void);

int atmturbo2_shadow_ports_enabled(void)
{
    return MACHINE_IS_ATM_TURBO2 &&
        (atmturbo_trdos_active || !(atmturbo2_port_77 & 0x200));
}

void atmturbo2_update_timing(void)
{
    /* La PLM CP6N.SRC genera VS en V=312 con RG0=1 y en V=316
       con RG0=0. Añadimos el tiempo al final para conservar la
       posición de inicio del área activa en ambos modos. */
    screen_invisible_borde_superior=8;
    screen_total_borde_inferior=(atmturbo_video_mode&1) ? 56 : 60;
    screen_set_video_params_indices();
}

void atmturbo2_out_port_77(z80_int port,z80_byte value)
{
    atmturbo2_port_77=port;
    atmturbo_video_mode=value&7;
    atmturbo2_update_timing();
    atmturbo2_refresh_memory();
    modificado_border.v=1;
}

void atmturbo2_out_port_fe(z80_int port,z80_byte value)
{
    atmturbo_border_colour=(value&7)|((port&8) ? 0 : 8);
}

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

int atmturbo2_palette_port_enabled(void)
{
    return atmturbo2_shadow_ports_enabled() && !(atmturbo2_port_77&0x4000);
}

void atmturbo2_out_port_f7(z80_int port,z80_byte value)
{
    int index=((puerto_32765 & 16) ? 4 : 0) | ((port>>14)&3);
    atmturbo2_mmu[index]=((((value&0xc0)<<2)|(value&0x3f))^0x33f);
    atmturbo2_refresh_memory();
}

static void atmturbo2_refresh_memory(void)
{
    int segment;
    for (segment=0;segment<4;segment++) {
        unsigned int entry=atmturbo2_mmu[((puerto_32765&16) ? 4 : 0)+segment];
        unsigned int page=entry&0x3f;
        int ram=1;

        if (!(atmturbo2_port_77&0x100)) {
            page=3;
            ram=0;
        }
        else switch (entry&0x300) {
        case 0x000:
            page=(page&~7)|(puerto_32765&7);
            break;
        case 0x100:
            page=(page&~1)|((atmturbo_trdos_active ||
                             !(atmturbo2_port_77&0x200)) ? 1 : 0);
            ram=0;
            break;
        case 0x200:
            break;
        case 0x300:
            ram=0;
            break;
        }

        if (ram) {
            page%=ATM_TURBO_TOTAL_RAM_PAGES;
            atmturbo_memory_paged[segment]=atmturbo_ram_mem_table[page];
            debug_paginas_memoria_mapeadas[segment]=page;
        }
        else {
            page%=ATM_TURBO_TOTAL_ROM_PAGES;
            atmturbo_memory_paged[segment]=atmturbo_rom_mem_table[page];
            debug_paginas_memoria_mapeadas[segment]=DEBUG_PAGINA_MAP_ES_ROM+page;
        }
        atmturbo2_segment_ram[segment]=ram;
    }
}

void atmturbo_mem_page_rom(void);

int atmturbo_trdos_is_active(void)
{
    return atmturbo_trdos_active && reg_pc<0x4000;
}

int atmturbo_fdc_is_active(z80_int port,int reading)
{
    if (MACHINE_IS_ATM_TURBO2)
        return atmturbo2_shadow_ports_enabled();
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
    int palette_colour=SPECTRA_INDEX_FIRST_COLOR+green*16+red*4+blue;
    atmturbo_palette[atmturbo_border_colour]=palette_colour;
    modificado_border.v=1;
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
    if (MACHINE_IS_ATM_TURBO2) {
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
    if (MACHINE_IS_ATM_TURBO2) {
        FILE *font_file;
        open_sharedfile("sgen.rom",&font_file);
        if (!font_file || fread(atmturbo2_text_font,1,sizeof(atmturbo2_text_font),font_file)!=sizeof(atmturbo2_text_font))
            cpu_panic("Cannot load ATM Turbo 2 character ROM");
        fclose(font_file);
    }
}

void atmturbo_mem_page_ram(void)
{
    if (MACHINE_IS_ATM_TURBO2) {
        atmturbo2_refresh_memory();
        return;
    }

    z80_byte page_entra=puerto_32765 & 7;
    z80_byte page_fdfd=puerto_fdfd &3;

    int pagina=(page_fdfd)*8 + page_entra;


    atmturbo_memory_paged[3]=atmturbo_ram_mem_table[pagina];
    debug_paginas_memoria_mapeadas[3]=page_entra;
}

void atmturbo_mem_page_rom(void)
{
    if (MACHINE_IS_ATM_TURBO2) {
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
    atmturbo_border_left_next_valid=0;
    if (MACHINE_IS_ATM_TURBO2) {
        puerto_32765=0;
        atmturbo2_port_77=0;
        for (i=0;i<8;i++) atmturbo2_mmu[i]=0;
        atmturbo2_update_timing();
    }
    modificado_border.v=1;
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

z80_byte atmturbo_get_border_colour(void)
{
    return atmturbo_border_colour;
}

static int atmturbo_z_signal_low(int t)
{
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
    return low;
}

z80_byte atmturbo_read_port_fe(z80_byte value)
{
    return (value & 0x7f) | (atmturbo_z_signal_low(t_estados) ? 0 : 0x80);
}

z80_byte atmturbo2_read_port_fe(z80_byte value)
{
    /* La INT de ATM2 cae 3565 T después del origen del barrido.
       Reutilizamos la señal Z de ATM1, desplazada a ese origen. */
    int low=atmturbo_z_signal_low(t_estados+3565);
    return (value & 0xdf) | (low ? 0 : 0x20);
}



void poke_byte_no_time_atmturbo(z80_int dir,z80_byte valor)
{
    int segmento;
    z80_byte *puntero;

    segmento=dir / 16384;

    if (MACHINE_IS_ATM_TURBO2 ? atmturbo2_segment_ram[segmento] :
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



    if (MACHINE_IS_ATM_TURBO2 ? atmturbo2_segment_ram[segmento] :
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

/* Lee un píxel del modo de texto de 80 columnas de ATM2. */
static int atmturbo2_get_text_colour(int x,int y)
{
    int page=(puerto_32765&8) ? 7 : 5;
    int column=x>>3;
    int address=0x1c0+(y>>3)*64+(column>>1);
    z80_byte character,attribute;
    int ink,paper;

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

    for (x=0;x<total_width;x++) {
        state=post_retrace_start+(x*(line_states-post_retrace_start))/total_width;
        if (state>=line_states) state=line_states-1;
        atmturbo_border_left_next[x]=state_colour[state];
    }
    atmturbo_border_left_next_valid=1;
}

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
            (MACHINE_IS_ATM_TURBO2 && atmturbo_video_mode==6 ?
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
                int colour=(MACHINE_IS_ATM_TURBO2 && atmturbo_video_mode==6) ?
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
