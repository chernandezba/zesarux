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

#ifndef TSCONF_H
#define TSCONF_H

#include "cpu.h"


#define TSCONF_LEFT_BORDER_NO_ZOOM 0
#define TSCONF_TOP_BORDER_NO_ZOOM 0

#define TSCONF_LEFT_BORDER TSCONF_LEFT_BORDER_NO_ZOOM*zoom_x
#define TSCONF_TOP_BORDER TSCONF_TOP_BORDER_NO_ZOOM*zoom_y

#define TSCONF_DISPLAY_WIDTH 720
#define TSCONF_DISPLAY_HEIGHT 576

#define TSCONF_FMAPS_SIZE 1024
#define TSCONF_ROM_PAGES 32
#define TSCONF_RAM_PAGES 256

#define TSCONF_MAX_SPRITES 85

extern z80_byte tsconf_last_port_eff7;
extern z80_byte tsconf_last_port_dff7;
//extern z80_byte tsconf_nvram[];

extern void tsconf_write_af_port(z80_byte puerto_h,z80_byte value);
extern z80_byte tsconf_get_af_port(z80_byte index);
extern void tsconf_init_memory_tables(void);
extern void tsconf_set_memory_pages(void);
extern z80_byte *tsconf_memory_paged[];
extern z80_byte *tsconf_rom_mem_table[];
extern z80_byte *tsconf_ram_mem_table[];
extern z80_byte tsconf_af_ports[];

extern z80_byte tsconf_fmaps[];

extern int temp_tsconf_in_system_rom_flag;

extern void tsconf_reset_cpu(void);
extern void tsconf_hard_reset(void);

extern z80_byte tsconf_get_text_font_page(void);

extern int tsconf_current_pixel_width;
extern int tsconf_current_pixel_height;
extern int tsconf_current_border_width;
extern int tsconf_current_border_height;

extern z80_byte tsconf_get_video_mode_display(void);

extern z80_byte tsconf_get_memconfig(void);

extern z80_byte tsconf_get_vram_page(void);

extern z80_int tsconf_return_cram_color(z80_byte color);

extern z80_byte tsconf_return_cram_palette_offset(void);

extern void tsconf_set_sizes_display(void);

extern z80_byte tsconf_get_rom_bank(void);

extern z80_bit tsconf_vdac_with_pwm;

extern z80_byte tsconf_palette_depth;

extern z80_byte tsconf_rgb_5_to_8(z80_byte color);

extern void screen_store_scanline_rainbow_solo_display_tsconf(void);

//extern z80_bit tsconf_si_render_spritetile_rapido;

extern z80_bit tsconf_force_disable_layer_ula;
extern z80_bit tsconf_force_disable_layer_tiles_zero;
extern z80_bit tsconf_force_disable_layer_tiles_one;
extern z80_bit tsconf_force_disable_layer_sprites_zero;
extern z80_bit tsconf_force_disable_layer_sprites_one;
extern z80_bit tsconf_force_disable_layer_sprites_two;
extern z80_bit tsconf_force_disable_layer_border;

//extern z80_bit tsconf_fired_frame_interrupt;

extern int debug_tsconf_dma_source;
extern int debug_tsconf_dma_destination;
extern int debug_tsconf_dma_burst_length;
extern int debug_tsconf_dma_burst_number;
extern z80_byte debug_tsconf_dma_s_align;
extern z80_byte debug_tsconf_dma_d_align;
extern z80_byte debug_tsconf_dma_addr_align_size;
extern z80_byte debug_tsconf_dma_ddev;
extern z80_byte debug_tsconf_dma_rw;

extern char *tsconf_dma_types[];

extern z80_bit tsconf_dma_disabled;


extern int tsconf_if_ula_enabled(void);
extern int tsconf_if_sprites_enabled(void);
extern int tsconf_if_tiles_zero_enabled(void);
extern int tsconf_if_tiles_one_enabled(void);
extern void tsconf_get_current_video_mode(char *s);
extern int tsconf_return_tilegraphicspage(z80_byte layer);
extern int tsconf_return_spritesgraphicspage(void);
extern int tsconf_return_tilemappage(void);

extern void tsconf_write_fmaps(int tsconf_fmaps_offset,z80_byte valor);


struct s_tsconf_debug_sprite {

		    int x;
            int y;

			z80_byte xs;
            z80_byte ys;



	  		z80_byte xf;
			z80_byte yf;             

            z80_byte act;

            z80_byte leap;

			z80_byte tnum_x;
    		z80_byte tnum_y;

		    z80_byte spal;

};

extern void tsconf_get_debug_sprite(int sprite,struct s_tsconf_debug_sprite *dest);

extern int tsconf_last_frame_y;

extern z80_byte tsconf_vector_fired_interrupt;

extern int tsconf_handle_frame_interrupts_prev_horiz;

extern void tsconf_handle_frame_interrupts(void);

extern void tsconf_handle_line_interrupts(void);

extern void tsconf_set_default_basic_palette(void);

extern void tsconf_set_emulador_settings(void);

extern z80_bit tsconf_reveal_layer_ula;

extern z80_bit tsconf_reveal_layer_sprites_zero;
extern z80_bit tsconf_reveal_layer_sprites_one;
extern z80_bit tsconf_reveal_layer_sprites_two;

extern z80_bit tsconf_reveal_layer_tiles_zero;
extern z80_bit tsconf_reveal_layer_tiles_one;

#define TSCONF_ZIFI_COMMAND_REG		    0xC7EF
#define TSCONF_ZIFI_ERROR_REG			0xC7EF
#define TSCONF_ZIFI_DATA_REG			0xBFEF
#define TSCONF_ZIFI_INPUT_FIFO_STATUS	0xC0EF
#define TSCONF_ZIFI_OUTPUT_FIFO_STATUS	0xC1EF


extern z80_byte tsconf_zifi_read_data_reg(void);
extern void tsconf_zifi_write_data_reg(z80_byte value);
extern z80_byte tsconf_zifi_read_error_reg(void);
extern void tsconf_zifi_write_command_reg(z80_byte value);
extern z80_byte tsconf_zifi_read_input_fifo_status(void);
extern z80_byte tsconf_zifi_read_output_fifo_status(void);
extern z80_byte tsconf_read_port_57(void);

#endif
