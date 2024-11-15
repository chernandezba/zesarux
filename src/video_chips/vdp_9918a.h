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

#ifndef VDP_9918A_H
#define VDP_9918A_H

#include "cpu.h"

extern z80_byte vdp_9918a_registers[];






extern z80_byte vdp_9918a_status_register;

extern char *get_vdp_9918_string_video_mode(void);

extern z80_int vdp_9918a_get_sprite_attribute_table(void);

extern z80_byte vdp_9918a_get_video_mode(void);

extern void vdp_9918a_out_vram_data(z80_byte *vram_memory,z80_byte value);

extern z80_int vdp_9918a_get_pattern_name_table(void);

extern z80_byte vdp_9918a_in_vram_data(z80_byte *vram_memory);

extern z80_byte vdp_9918a_get_border_color(void);

extern z80_byte vdp_9918a_get_foreground_color(void);

extern int vdp_9918a_get_sprite_size(void);

extern int vdp_9918a_get_sprite_double(void);

extern z80_int vdp_9918a_get_pattern_color_table(void);

extern z80_int vdp_9918a_get_pattern_base_address(void);

extern z80_int vdp_9918a_get_sprite_pattern_table(void);

extern z80_byte vdp_9918a_in_vdp_status(void);

extern void vdp_9918a_out_command_status(z80_byte value);

extern void screen_store_scanline_rainbow_vdp_9918a_border_and_display(z80_int *scanline_buffer,z80_byte *vram_memory);

extern int vdp_9918a_get_tile_heigth(void);

extern int vdp_9918a_get_tile_width(void);

extern void vdp_9918a_render_ula_no_rainbow(z80_byte *vram);

extern void vdp_9918a_render_sprites_no_rainbow(z80_byte *vram);

extern void vdp_9918a_refresca_border(void);

extern void vdp_9918a_render_rainbow_display_line(int scanline,z80_int *scanline_buffer,z80_byte *vram);


extern void vdp_9918a_render_rainbow_sprites_line(int scanline,z80_int *scanline_buffer,z80_byte *vram);

extern z80_byte vdp_9918a_read_vram_byte(z80_byte *vram,z80_int address);

extern void vdp_9918a_reset(void);

extern void vdp9918a_put_sprite_pixel(z80_int *destino,z80_int color);

extern z80_byte vdp_9918a_last_command_status_bytes_counter;
extern z80_int vdp_9918a_last_vram_position;
extern z80_byte vdp_9918a_last_command_status_bytes[];
extern z80_byte vdp_9918a_last_vram_bytes[];

extern z80_bit vdp_9918a_force_disable_layer_ula;
extern z80_bit vdp_9918a_force_disable_layer_sprites;
extern z80_bit vdp_9918a_force_disable_layer_border;


//Forzar a dibujar capa con color fijo, para debug
extern z80_bit vdp_9918a_reveal_layer_ula;
extern z80_bit vdp_9918a_reveal_layer_sprites;

extern z80_int vdp_9918a_buffer_render_sprites[];

extern void vdp_9918a_scr_refresca_pantalla_y_border_rainbow(void);


#define VDP_9918A_LEFT_BORDER_NO_ZOOM 48
#define VDP_9918A_RIGHT_BORDER_NO_ZOOM 48

#define VDP_9918A_TOP_BORDER_NO_ZOOM 56
#define VDP_9918A_BOTTOM_BORDER_NO_ZOOM 56

#define VDP_9918A_ANCHO_PANTALLA 256
#define VDP_9918A_ALTO_PANTALLA 192


#define VDP_9918A_TOP_BORDER VDP_9918A_TOP_BORDER_NO_ZOOM*zoom_y

#define VDP_9918A_LEFT_BORDER VDP_9918A_LEFT_BORDER_NO_ZOOM*zoom_x


#define VDP_9918A_BOTTOM_BORDER BOTTOM_BORDER_NO_ZOOM*zoom_y


#define VDP_9918A_MAX_SPRITES 32

#define VDP_9918A_TOTAL_REGISTERS 16




#define VDP_9918A_MAX_SPRITES_PER_LINE 5

#endif
