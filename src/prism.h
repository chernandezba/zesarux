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

#ifndef PRISM_H
#define PRISM_H

#include "cpu.h"
#include "screen.h"

extern z80_byte *prism_rom_mem_table[];

extern z80_byte *prism_ram_mem_table[];

extern z80_byte *prism_memory_paged[];

extern void prism_set_memory_pages(void);

extern void prism_init_memory_tables(void);

extern z80_byte prism_type_memory_paged[];

//Tipos de memoria mapeadas
//0=rom
//1=home
//2=dock
//3=ex
//Constantes definidas en PRISM_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX

#define PRISM_MEMORY_TYPE_ROM  0
#define PRISM_MEMORY_TYPE_HOME 1
#define PRISM_MEMORY_TYPE_DOCK 2
#define PRISM_MEMORY_TYPE_EX   3

extern z80_byte debug_prism_paginas_memoria_mapeadas[];

extern z80_byte prism_rom_page;

extern z80_bit prism_failsafe_mode;

#define PRISM_FAILSAFE_ROM_NAME "prism_failsafe.rom"

extern void prism_load_failsafe_rom(void);

extern void prism_malloc_vram(void);

extern z80_byte *prism_vram_mem_table[];

extern void prism_out_ula2(z80_byte value);

extern z80_byte prism_ula2_registers[];

extern void hard_reset_cpu_prism(void);



//Hacer que estos valores de border sean multiples de 8
#define PRISM_LEFT_BORDER_NO_ZOOM 64
#define PRISM_TOP_BORDER_NO_ZOOM 48

#define PRISM_LEFT_BORDER PRISM_LEFT_BORDER_NO_ZOOM*zoom_x
#define PRISM_TOP_BORDER PRISM_TOP_BORDER_NO_ZOOM*zoom_y

#define PRISM_DISPLAY_WIDTH 512
#define PRISM_DISPLAY_HEIGHT 384


//extern z80_byte prism_ula2_border_colour;

extern z80_byte prism_last_ae3b;

extern void prism_out_9e3b(z80_byte value);

extern z80_byte get_prism_ula2_border_colour(void);

extern z80_byte prism_ae3b_registers[];

extern void init_prism_palettes(void);

extern z80_int prism_palette_two[];
extern z80_int prism_palette_zero[];


extern z80_byte prism_ula2_palette_control_colour;
extern z80_byte prism_ula2_palette_control_index;
extern z80_byte prism_ula2_palette_control_rgb[];


#define MAX_PRISM_BORDER_BUFFER (525*MAX_STATES_LINE)
extern z80_int prism_ula2_border_colour_buffer[];

//Total en un frame actual
//#define MAX_CURRENT_STATES_LINE (256*cpu_turbo_speed)
//esto es equivalente a CURRENT_FULLBORDER_ARRAY_LENGTH, de hecho valen lo mismo,
//no se porque defini dos variables por separado cuando realmente valen lo mismo ....
#define CURRENT_PRISM_BORDER_BUFFER (525*MAX_CURRENT_STATES_LINE)

extern void init_prism_palette_two(void);

extern z80_byte prism_retorna_ram_entra(void);

extern void prism_set_emulator_setting_cpuspeed(void);



//screendatadecoding
/*
0000 – SPECTRUM ATTR: D0-D2 ink, D3-D5, paper, D6 bright, D7 Flash

0001 – 16+16 Colour: ATTR: D0-D2 ink, D3-D5, paper, D6 ink bright, D7 paper bright

0010 - 32 Colour ATTR: D0-D2 ink, D3-D5 paper, D6-D7 "CLUT". In the standard palette, colours 0-15 are the same as the normal Spectrum palette and colours 16-31 are darker versions of the no
rmal spectrum palette)

0011 - RESERVED

0100 - 256 Colour mode 1 - D0-D7 = ink colour. Paper colour is determined by ULA2 BORDER (IO 0x9E3B)
*/

#define GET_PIXEL_COLOR_PRISM \
            switch (screendatadecoding) { \
            \
            case 0: \
            default: \
                        ink=attribute &7; \
                        paper=(attribute>>3) &7; \
                        bright=(attribute)&64; \
                        flash=(attribute)&128; \
                        if (flash) { \
                                if (estado_parpadeo.v) { \
                                        aux=paper; \
                                        paper=ink; \
                                        ink=aux; \
                                } \
                        } \
                        \
                        if (bright) {   \
                                paper+=8; \
                                ink+=8; \
                        } \
            break; \
            \
            case 1: \
            ink=attribute &7; \
                        paper=(attribute>>3) &7; \
                        bright=(attribute)&64; \
                        bright2=(attribute)&128; \
            if (bright) {   \
                                ink+=8; \
                        } \
            if (bright2) {   \
                                paper+=8; \
                        } \
                        break; \
            \
            case 2: \
            ink=attribute &7; \
                        paper=(attribute>>3) &7; \
                        clut=((attribute)&(64+128))>>6; \
            ink +=8*clut; \
            paper +=8*clut; \
            break; \
            \
            case 4: \
            ink=attribute; \
            paper=prism_ula2_border_colour; \
            break; \
            } \




#endif
