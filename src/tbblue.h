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

#ifndef TBBLUE_H
#define TBBLUE_H

#include "cpu.h"

#define TBBLUE_CORE_VERSION_MAJOR     1 
#define TBBLUE_CORE_VERSION_MINOR     10
#define TBBLUE_CORE_VERSION_SUBMINOR  47

extern z80_byte *tbblue_ram_memory_pages[];

extern z80_byte *tbblue_rom_memory_pages[];

extern z80_byte *tbblue_memory_paged[];

//extern z80_byte tbblue_config1;
//extern z80_byte tbblue_config2;
//extern z80_byte tbblue_port_24df;

extern void tbblue_out_port(z80_int port,z80_byte value);

extern void tbblue_set_memory_pages(void);

extern void tbblue_init_memory_tables(void);

extern void tbblue_hard_reset(void);

extern void tbblue_reset(void);

extern z80_byte *tbblue_fpga_rom;

extern z80_bit tbblue_low_segment_writable;

extern z80_bit tbblue_bootrom;

//extern z80_byte tbblue_read_port_24d5(void);

//extern z80_byte tbblue_read_port_24d5_index;

extern void tbblue_set_timing_48k(void);

//extern void tbblue_set_emulator_setting_timing(void);

/*
243B - Set register #
253B - Set/read value
*/

#define TBBLUE_REGISTER_PORT 0x243b
#define TBBLUE_VALUE_PORT 0x253b
#define TBBLUE_LAYER2_PORT 0x123B

#define TBBLUE_SPRITE_INDEX_PORT 0x303B
#define TBBLUE_SPRITE_PALETTE_PORT 0x53
#define TBBLUE_SPRITE_PATTERN_PORT 0x5b
#define TBBLUE_SPRITE_SPRITE_PORT 0x57

#define TBBLUE_UART_RX_PORT 0x133b

#define TBBLUE_SECOND_KEMPSTON_PORT 0x37

#define MAX_SPRITES_PER_LINE 12

#define TBBLUE_SPRITE_BORDER 32

#define MAX_X_SPRITE_LINE (TBBLUE_SPRITE_BORDER+256+TBBLUE_SPRITE_BORDER)


#define TBBLUE_COPPER_MEMORY 2048

extern z80_byte tbblue_copper_memory[];

extern void tbblue_set_value_port_position(z80_byte index_position,z80_byte value);


extern z80_byte tbblue_registers[];

extern z80_byte tbblue_last_register;

extern z80_byte tbblue_get_value_port(void);
extern void tbblue_set_register_port(z80_byte value);
extern void tbblue_set_value_port(z80_byte value);
extern z80_byte tbblue_get_value_port_register(z80_byte registro);

extern void tbsprite_do_overlay(void);

#define TBBLUE_MAX_PATTERNS 64
#define TBBLUE_SPRITE_SIZE 256

#define TBBLUE_MAX_SPRITES 64
//#define TBBLUE_TRANSPARENT_COLOR_INDEX 0xE3
//#define TBBLUE_TRANSPARENT_COLOR 0x1C6

#define TBBLUE_DEFAULT_TRANSPARENT 0xE3
#define TBBLUE_TRANSPARENT_REGISTER (tbblue_registers[20])
#define TBBLUE_TRANSPARENT_REGISTER_9 (TBBLUE_TRANSPARENT_REGISTER<<1)

#define TBBLUE_SPRITE_HEIGHT 16
#define TBBLUE_SPRITE_WIDTH 16

//extern z80_byte tbsprite_patterns[TBBLUE_MAX_PATTERNS][256];
extern z80_byte tbsprite_new_patterns[TBBLUE_MAX_PATTERNS*256];


//extern z80_int tbsprite_palette[];
extern z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];


extern z80_int tbblue_palette_ula_first[];
extern z80_int tbblue_palette_ula_second[];
extern z80_int tbblue_palette_layer2_first[];
extern z80_int tbblue_palette_layer2_second[];
extern z80_int tbblue_palette_sprite_first[];
extern z80_int tbblue_palette_sprite_second[];

extern void tbblue_out_port_sprite_index(z80_byte value);
//extern void tbblue_out_sprite_palette(z80_byte value);
extern void tbblue_out_sprite_pattern(z80_byte value);
extern void tbblue_out_sprite_sprite(z80_byte value);
extern z80_byte tbblue_get_port_sprite_index(void);

extern z80_int tbblue_get_palette_active_layer2(z80_byte index);

extern z80_int tbblue_get_palette_active_ula(z80_byte index);

extern z80_byte tbblue_port_123b;
extern int tbblue_write_on_layer2(void);
extern int tbblue_get_offset_start_layer2(void);
extern z80_byte tbblue_get_port_layer2_value(void);
extern void tbblue_out_port_layer2_value(z80_byte value);
extern int tbblue_is_active_layer2(void);

#define TBBLUE_MACHINE_TYPE ((tbblue_registers[3])&3)

#define TBBLUE_MACHINE_48K (TBBLUE_MACHINE_TYPE==1)
#define TBBLUE_MACHINE_128_P2 (TBBLUE_MACHINE_TYPE==2)
#define TBBLUE_MACHINE_P2A (TBBLUE_MACHINE_TYPE==3)

extern void screen_store_scanline_rainbow_solo_display_tbblue(void);

extern int tbblue_get_current_ram(void);

extern z80_byte tbblue_extra_512kb_blocks;

extern z80_byte return_tbblue_mmu_segment(z80_int dir);

extern int tbblue_is_writable_segment_mmu_rom_space(z80_int dir);


extern z80_byte tbsprite_pattern_get_value_index(z80_byte sprite,z80_byte index_in_sprite);

extern void tbsprite_pattern_put_value_index(z80_byte sprite,z80_byte index_in_sprite,z80_byte value);


extern z80_byte clip_window_layer2[];
extern z80_byte clip_window_sprites[];
extern z80_byte clip_window_ula[];
extern z80_byte tbblue_get_clip_window_layer2_index(void);
extern z80_byte tbblue_get_clip_window_sprites_index(void);
extern z80_byte tbblue_get_clip_window_ula_index(void);

extern z80_bit tbblue_fast_boot_mode;


extern z80_byte tbblue_copper_get_control_bits(void);

extern z80_int tbblue_copper_pc;


#define TBBLUE_RCCH_COPPER_STOP             0x00
#define TBBLUE_RCCH_COPPER_RUN_LOOP_RESET   0x40
#define TBBLUE_RCCH_COPPER_RUN_LOOP         0x80
#define TBBLUE_RCCH_COPPER_RUN_VBI          0xc0



extern int tbblue_get_current_raster_horiz_position(void);
extern int tbblue_get_current_raster_position(void);
extern int tbblue_copper_wait_cond_fired(void);
extern void tbblue_copper_handle_next_opcode(void);
extern void tbblue_copper_handle_vsync(void);

extern z80_bit tbblue_deny_turbo_rom;


extern z80_bit tbblue_force_disable_layer_ula;
extern z80_bit tbblue_force_disable_layer_sprites;
extern z80_bit tbblue_force_disable_layer_layer_two;

extern int tbblue_if_sprites_enabled(void);

extern char *tbblue_get_string_layer_prio(int layer,z80_byte prio);

extern void tbblue_get_string_palette_format(char *texto);

extern int tbblue_get_offset_start_layer2_reg(z80_byte register_value);

extern z80_byte tbblue_get_layers_priorities(void);

extern z80_byte tbblue_get_register_port(void);

extern void get_pixel_color_tbblue(z80_byte attribute,z80_int *tinta_orig, z80_int *papel_orig);

#endif
