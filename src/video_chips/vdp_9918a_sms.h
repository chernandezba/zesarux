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

#ifndef VDP_9918A_SMS_H
#define VDP_9918A_SMS_H

#include "cpu.h"

#define VDP_9918A_SMS_MODE4_MAX_SPRITES 64

#define VDP_9918A_SMS_MAX_SPRITES_PER_LINE 8

#define VDP_9918A_SMS_MODE4_MAPPED_PALETTE_COLOURS 32

extern int sms_writing_cram;
//extern int sms_pending_line_interrupt;
//extern z80_byte vdp_9918a_sms_raster_line_counter;
extern z80_bit sms_disable_raster_interrupt;
extern z80_bit sms_only_one_raster_int_frame;
extern z80_byte index_sms_escritura_cram;
extern z80_byte vdp_9918a_sms_cram[];
extern void vdp_9918a_sms_reset(void);
extern int vdp_9918a_si_sms_video_mode4(void);
extern void vdp_9918a_render_sprites_sms_video_mode4_no_rainbow(z80_byte *vram);
extern void vdp_9918a_render_ula_no_rainbow_sms(z80_byte *vram,int render_tiles_foreground,int reveal,int forzada_negro);
extern z80_int vdp_9918a_sms_get_pattern_name_table(void);

extern z80_byte vdp_9918a_sms_get_scroll_horizontal(void);
extern z80_byte vdp_9918a_sms_get_scroll_vertical(void);

extern int vdp_9918a_sms_get_sprite_height(void);
extern z80_int vdp_9918a_get_sprite_pattern_table_sms_mode4(void);

extern z80_bit vdp_9918a_sms_force_show_column_zero;
extern z80_bit vdp_9918a_sms_lock_scroll_horizontal;
extern z80_bit vdp_9918a_sms_lock_scroll_vertical;
extern void vdp_9918a_render_rainbow_display_line_sms(int scanline,z80_int *scanline_buffer,z80_int *scanline_buffer_foreground,z80_byte *vram);
extern void screen_store_scanline_rainbow_solo_display_vdp_9918a_sms_3layer(z80_int *scanline_buffer,z80_byte *vram_memory_pointer,int y_display);

extern z80_bit vdp_9918a_force_disable_layer_tile_bg;
extern z80_bit vdp_9918a_force_disable_layer_tile_fg;
extern z80_bit vdp_9918a_reveal_layer_tile_fg;
extern z80_bit vdp_9918a_reveal_layer_tile_bg;

extern z80_bit vdp_9918a_force_bg_tiles;
extern int vdp_9918a_sms_get_cram_color(int index);

extern z80_byte sms_next_scroll_vertical_value;
extern const char *s_vdp_9918a_video_mode_sms_4;
extern void vdp9918a_sms_set_scroll_vertical(z80_byte valor);
extern void vdp_9918a_sms_set_writing_cram(z80_byte valor);
extern int vdp_9918a_sms_get_final_color_border(void);

extern void vdp_9918a_sms_raster_line_reset(void);
extern z80_byte vdp_9918a_sms_raster_line_counter;
extern z80_bit sms_wonderboy_scroll_hack;

extern void vdp_9918a_sms_handle_raster_interrupt(void);
extern z80_byte vdp_9918a_sms_pre_write_reg(z80_byte vdp_register,z80_byte next_value);

#endif
