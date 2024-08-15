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

#ifndef CPC_H
#define CPC_H


#include "cpu.h"

#define CPC_MAX_ADDITIONAL_ROMS 8

//Para roms aditionales
struct s_cpc_additional_rom {
    z80_byte bank_number; //el que le quiera asignar el usuario
    int enabled;
};

extern struct s_cpc_additional_rom cpc_additional_roms[];

extern z80_byte cpc_gate_registers[];

extern z80_byte *cpc_rom_mem_table[];

extern z80_byte *cpc_ram_mem_table[];

extern z80_byte *cpc_memory_paged_read[];
extern z80_byte *cpc_memory_paged_write[];

extern void cpc_out_port_df(z80_byte value);
extern void cpc_out_port_fa7e(z80_byte value);
extern z80_byte cpc_port_df;

#define CPC_MEMORY_TYPE_ROM 0
#define CPC_MEMORY_TYPE_RAM 1

extern z80_byte debug_cpc_type_memory_paged_read[];
extern z80_byte debug_cpc_paginas_memoria_mapeadas_read[];
extern void init_cpc_line_display_table(void);

//Hacer que estos valores de border sean multiples de 8
#define CPC_LEFT_BORDER_NO_ZOOM 80
//#define CPC_LEFT_BORDER_NO_ZOOM 192


#define CPC_TOP_BORDER_NO_ZOOM 72



#define CPC_DISPLAY_WIDTH 640
#define CPC_DISPLAY_HEIGHT 400

#define CPC_TOTAL_SCANLINES ((CPC_DISPLAY_HEIGHT+CPC_TOP_BORDER_NO_ZOOM*2)/2)

#define CPC_LEFT_BORDER CPC_LEFT_BORDER_NO_ZOOM*zoom_x
#define CPC_TOP_BORDER CPC_TOP_BORDER_NO_ZOOM*zoom_y

/*

http://www.cpcwiki.eu/index.php/Video_modes

Mode 0: 160×200 pixels with 16 colors (4 bpp)
Mode 1: 320×200 pixels with 4 colors (2 bpp)
Mode 2: 640×200 pixels with 2 colors (1 bpp)
Mode 3: 160×200 pixels with 4 colors (2bpp) (this is not an official mode, but rather a side-effect of the hardware)
The Video modes are known to display pixels with different sizes.

Basically, the Amstrad CPC Video works like a CGA video card from a PC. But extra features like a 16 colours mode exist.

The dimensions in pixels given could be raised with clever use of FullScreen Trick (often dubbed erronuously as "overscan mode".)

This then allows with a video memory of 24 KB (approximately) to displays on the standard screen up to :

Full screen Mode 0: 192×272 pixels with 16 colors (4 bpp)
Full screen Mode 1: 384×272 pixels with 4 colors (2 bpp)
Full screen Mode 2: 768×272 pixels with 2 colors (1 bpp)

En modo no rainbow, border superior e inferior suman 272-200=72 pixeles. O sea 72/2=36 cada uno. Pero como hacemos x2, son de 72 cada uno
Borde izquierdo y derecho son de 768-640=128 total, o sea , 64 pixeles cada uno

*/

extern z80_int cpc_line_display_table[];


extern int cpc_rgb_table[];

extern z80_byte cpc_palette_table[];

extern z80_byte cpc_keyboard_table[];

extern z80_byte cpc_border_color;

extern void cpc_out_port_gate(z80_byte value);

extern void cpc_out_ppi(z80_byte puerto_h,z80_byte value);

extern z80_byte cpc_in_ppi(z80_byte puerto_h);

extern void cpc_set_memory_pages();

extern void cpc_init_memory_tables();

extern void cpc_out_port_crtc(z80_int puerto,z80_byte value);

extern z80_byte cpc_crtc_registers[];

extern z80_byte cpc_ppi_ports[];

extern z80_byte cpc_crtc_last_selected_register;

extern z80_byte cpc_scanline_counter;

extern z80_bit cpc_forzar_modo_video;
extern z80_byte cpc_forzar_modo_video_modo;

extern void cpc_splash_videomode_change(void);

//extern z80_bit cpc_send_double_vsync;

extern z80_bit cpc_debug_borders;

extern void cpc_handle_vsync_state(void);

extern z80_bit cpc_crt_pending_interrupt;

extern int cpc_pending_last_drawn_lines_black_counter;

extern void cpc_reset(void);

extern int cpc_crtc_get_height_character(void);

extern void screen_store_scanline_rainbow_cpc_border_and_display(void);

extern int cpc_crtc_get_total_pixels_horizontal(void);
extern int cpc_crtc_get_total_pixels_vertical(void);
extern int cpc_crtc_get_total_horizontal(void);
extern int cpc_crtc_get_total_vertical(void);

extern int cpc_crtc_get_total_right_border(void);
extern int cpc_crtc_get_total_left_border(void);
extern int cpc_crtc_get_top_border_height(void);
extern int cpc_crtc_get_bottom_border_height(void);
extern int cpc_crtc_get_total_hsync_width(void);
extern int cpc_crtc_get_total_vsync_height_crtc(void);

extern z80_int cpc_ctrc_get_offset_videoram(void);
extern z80_byte cpc_ctrc_get_video_page(void);
extern z80_int cpc_incrementa_puntero_videoram(z80_int direccion_pixel);

//extern z80_bit cpc_endframe_workaround;

extern int cpc_crtc_contador_scanline;

extern const char *cpc_video_modes_strings[];

extern int cpc_get_crtc_final_display_zone(void);

extern void cpc_if_autoenable_realvideo(void);

extern void cpc_if_autoenable_realvideo_on_changemodes(void);
extern void cpc_if_autoenable_realvideo_on_changeborder(void);

#endif
