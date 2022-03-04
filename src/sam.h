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

#ifndef SAM_H
#define SAM_H


#include "cpu.h"

extern z80_byte sam_vmpr;
extern z80_byte sam_hmpr;
extern z80_byte sam_lmpr;
extern z80_byte sam_border;

extern void sam_set_memory_pages(void);

extern z80_byte *sam_memory_paged[];

extern z80_byte sam_memory_paged_type[];

extern z80_byte *sam_rom_memory[];

extern z80_byte *sam_ram_memory[];

//Hacer que estos valores de border sean multiples de 8
#define SAM_LEFT_BORDER_NO_ZOOM 48
#define SAM_TOP_BORDER_NO_ZOOM 24

#define SAM_LEFT_BORDER SAM_LEFT_BORDER_NO_ZOOM*zoom_x
#define SAM_TOP_BORDER SAM_TOP_BORDER_NO_ZOOM*zoom_y

//#define SAM_LEFT_BORDER 0
//#define SAM_TOP_BORDER 0
#define SAM_DISPLAY_WIDTH 512
#define SAM_DISPLAY_HEIGHT 384

extern z80_byte sam_memoria_total_mascara;

extern z80_byte sam_palette[];

extern z80_byte debug_sam_paginas_memoria_mapeadas[];

extern void sam_init_memory_tables(void);

extern z80_byte puerto_65534;

extern z80_byte puerto_teclado_sam_fef9;
extern z80_byte puerto_teclado_sam_fdf9;
extern z80_byte puerto_teclado_sam_fbf9;
extern z80_byte puerto_teclado_sam_f7f9;

extern z80_byte puerto_teclado_sam_eff9;
extern z80_byte puerto_teclado_sam_dff9;
extern z80_byte puerto_teclado_sam_bff9;
extern z80_byte puerto_teclado_sam_7ff9;

extern void sam_splash_videomode_change(void);

#endif
