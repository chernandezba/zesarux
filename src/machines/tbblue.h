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


#define TBBLUE_CORE_DEFAULT_VERSION_MAJOR     3
#define TBBLUE_CORE_DEFAULT_VERSION_MINOR     2
#define TBBLUE_CORE_DEFAULT_VERSION_SUBMINOR  1

//borde izquierdo + pantalla + borde derecho, multiplicado por 2
#define TBBLUE_LAYERS_PIXEL_WIDTH ((48+256+48)*2)

//De momento esto solo se usa en carga/grabacion snapshots zsf
//#define TBBLUE_TOTAL_RAM_SIZE 2048

#define TBBLUE_MAX_SRAM_8KB_BLOCKS 224

#define TBBLUE_TOTAL_RAM (0x040000+8192*TBBLUE_MAX_SRAM_8KB_BLOCKS)

#define TBBLUE_TOTAL_RAM_SIZE (TBBLUE_TOTAL_RAM/1024)

#define TBBLUE_FPGA_ROM_SIZE 8

//Los 8kb de rom del final estan repetidos
#define TBBLUE_TOTAL_MEMORY_USED (TBBLUE_TOTAL_RAM_SIZE+TBBLUE_FPGA_ROM_SIZE*2)

extern z80_byte *tbblue_ram_memory_pages[];

extern z80_byte *tbblue_rom_memory_pages[];

extern z80_byte *tbblue_memory_paged[];

//extern z80_byte tbblue_config1;
//extern z80_byte tbblue_config2;
//extern z80_byte tbblue_port_24df;

extern int tbblue_use_rtc_traps;

extern int tbblue_already_autoenabled_rainbow;

extern void tbblue_out_port(z80_int port,z80_byte value);

extern z80_byte tbblue_core_current_version_major;
extern z80_byte tbblue_core_current_version_minor;
extern z80_byte tbblue_core_current_version_subminor;

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

extern int tbblue_get_altrom_offset_dir(int altrom,z80_int dir);
extern int tbblue_get_altrom(void);

extern void tbblue_set_emulator_setting_timing(void);
extern void tbblue_set_emulator_setting_reg_8(void);
extern void tbblue_set_emulator_setting_divmmc(void);

extern void tbblue_set_ram_blocks(int memoria_kb);

extern char *tbblue_get_layer2_mode_name(void);

#define TBBLUE_REGISTER_PORT 0x243b
#define TBBLUE_VALUE_PORT 0x253b
#define TBBLUE_LAYER2_PORT 0x123B

#define TBBLUE_SPRITE_INDEX_PORT 0x303B
#define TBBLUE_SPRITE_PALETTE_PORT 0x53
#define TBBLUE_SPRITE_PATTERN_PORT 0x5b
#define TBBLUE_SPRITE_SPRITE_PORT 0x57

#define TBBLUE_LAYER2_PRIORITY 0x8000


#define TBBLUE_UART_TX_PORT 0x133b
//Tambien byte de estado en lectura

#define TBBLUE_UART_RX_PORT 0x143b

#define TBBLUE_UART_STATUS_DATA_READY 1
#define TBBLUE_UART_STATUS_BUSY 2
#define TBBLUE_UART_STATUS_FIFO_FULL 4



#define TBBLUE_SECOND_KEMPSTON_PORT 0x37




#define TBBLUE_COPPER_MEMORY 2048

extern z80_byte tbblue_copper_memory[];

extern void tbblue_set_value_port_position(z80_byte index_position,z80_byte value);

extern z80_int tbblue_copper_get_pc(void);

extern z80_byte tbblue_registers[];

extern z80_byte tbblue_last_register;

extern z80_byte tbblue_get_value_port(void);
extern void tbblue_set_register_port(z80_byte value);
extern void tbblue_set_value_port(z80_byte value);
extern z80_byte tbblue_get_value_port_register(z80_byte registro);

extern void tbsprite_do_overlay(void);


#define MAX_SPRITES_PER_LINE 100

#define TBBLUE_SPRITE_BORDER 32
#define TBBLUE_TILES_BORDER 32

//Lineas de border superior e inferior ocupados por layer 2 en resoluciones 1 y 2
#define TBBLUE_LAYER2_12_BORDER 32

#define MAX_X_SPRITE_LINE (TBBLUE_SPRITE_BORDER+256+TBBLUE_SPRITE_BORDER)



#define TBBLUE_MAX_PATTERNS 64
#define TBBLUE_SPRITE_8BPP_SIZE 256
#define TBBLUE_SPRITE_4BPP_SIZE 128

#define TBBLUE_SPRITE_ARRAY_PATTERN_SIZE (TBBLUE_MAX_PATTERNS*TBBLUE_SPRITE_8BPP_SIZE)

#define TBBLUE_MAX_SPRITES 128
//#define TBBLUE_TRANSPARENT_COLOR_INDEX 0xE3
//#define TBBLUE_TRANSPARENT_COLOR 0x1C6
#define TBBLUE_SPRITE_ATTRIBUTE_SIZE 5

#define TBBLUE_DEFAULT_TRANSPARENT 0xE3
#define TBBLUE_TRANSPARENT_REGISTER (tbblue_registers[20])
#define TBBLUE_TRANSPARENT_REGISTER_9 (TBBLUE_TRANSPARENT_REGISTER<<1)

#define TBBLUE_SPRITE_HEIGHT 16
#define TBBLUE_SPRITE_WIDTH 16

#define TBBLUE_TILE_HEIGHT 8
#define TBBLUE_TILE_WIDTH 8

//extern z80_byte tbsprite_patterns[TBBLUE_MAX_PATTERNS][256];
extern z80_byte tbsprite_new_patterns[TBBLUE_MAX_PATTERNS*256];


//extern z80_int tbsprite_palette[];
extern z80_byte tbsprite_new_sprites[TBBLUE_MAX_SPRITES][TBBLUE_SPRITE_ATTRIBUTE_SIZE];


extern z80_int tbblue_palette_ula_first[];
extern z80_int tbblue_palette_ula_second[];
extern z80_int tbblue_palette_layer2_first[];
extern z80_int tbblue_palette_layer2_second[];
extern z80_int tbblue_palette_sprite_first[];
extern z80_int tbblue_palette_sprite_second[];
extern z80_int tbblue_palette_tilemap_first[];
extern z80_int tbblue_palette_tilemap_second[];

extern void tbblue_out_port_sprite_index(z80_byte value);
//extern void tbblue_out_sprite_palette(z80_byte value);
extern void tbblue_out_sprite_pattern(z80_byte value);
extern void tbblue_out_sprite_sprite(z80_byte value);
extern z80_byte tbblue_get_port_sprite_index(void);

extern z80_int tbblue_get_palette_active_layer2(z80_byte index);

extern z80_int tbblue_get_palette_active_ula(z80_byte index);
extern z80_int tbblue_get_border_color(z80_int color);

extern z80_byte tbblue_port_123b;
extern z80_byte tbblue_port_123b_second_byte;
extern int tbblue_write_on_layer2(void);
extern int tbblue_read_on_layer2(void);
extern int tbblue_layer2_size_mapped(void);
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


extern z80_byte tbsprite_pattern_get_value_index_8bpp(z80_byte sprite,z80_byte index_in_sprite);
extern z80_byte tbsprite_pattern_get_value_index_4bpp(z80_byte sprite,z80_byte index_in_sprite);

extern void tbsprite_pattern_put_value_index_8bpp(z80_byte sprite,z80_byte index_in_sprite,z80_byte value);

extern int tbsprite_last_visible_sprite;

extern void tbblue_write_sprite_value(z80_byte indice,z80_byte subindice,z80_byte value);

extern z80_bit tbblue_disable_optimized_sprites;


#define TBBLUE_CLIP_WINDOW_LAYER2   0
#define TBBLUE_CLIP_WINDOW_SPRITES  1
#define TBBLUE_CLIP_WINDOW_ULA      2
#define TBBLUE_CLIP_WINDOW_TILEMAP  3
extern z80_byte tbblue_clip_windows[4][4];

extern z80_byte tbblue_get_clip_window_layer2_index(void);
extern z80_byte tbblue_get_clip_window_sprites_index(void);
extern z80_byte tbblue_get_clip_window_ula_index(void);
extern z80_byte tbblue_get_clip_window_tilemap_index(void);

extern z80_bit tbblue_fast_boot_mode;


extern z80_byte tbblue_copper_get_control_bits(void);

extern z80_int tbblue_copper_pc;


#define TBBLUE_RCCH_COPPER_STOP             0x00
#define TBBLUE_RCCH_COPPER_RUN_LOOP_RESET   0x40
#define TBBLUE_RCCH_COPPER_RUN_LOOP         0x80
#define TBBLUE_RCCH_COPPER_RUN_VBI          0xc0


//Hacer que estos valores de border sean multiples de 8
#define TBBLUE_LEFT_BORDER_NO_ZOOM (48*2)
#define TBBLUE_TOP_BORDER_NO_ZOOM (56*2)
#define TBBLUE_BOTTOM_BORDER_NO_ZOOM (56*2)

#define TBBLUE_LEFT_BORDER TBBLUE_LEFT_BORDER_NO_ZOOM*zoom_x
#define TBBLUE_TOP_BORDER TBBLUE_TOP_BORDER_NO_ZOOM*zoom_y
#define TBBLUE_BOTTOM_BORDER TBBLUE_BOTTOM_BORDER_NO_ZOOM*zoom_y

#define TBBLUE_DISPLAY_WIDTH 512
#define TBBLUE_DISPLAY_HEIGHT 384

extern z80_bit tbblue_reveal_layer_ula;
extern z80_bit tbblue_reveal_layer_tiles;
extern z80_bit tbblue_reveal_layer_layer2;
extern z80_bit tbblue_reveal_layer_sprites;


extern int tbblue_get_current_raster_horiz_position(void);
extern int tbblue_get_current_raster_position(void);
extern int tbblue_copper_wait_cond_fired(void);
extern void tbblue_copper_handle_next_opcode(void);
extern void tbblue_copper_handle_vsync(void);

extern z80_bit tbblue_deny_turbo_rom;
extern int tbblue_deny_turbo_rom_max_allowed;
extern z80_bit tbblue_deny_turbo_everywhere;
extern int tbblue_deny_turbo_everywhere_max_allowed;
extern void tbblue_set_emulator_setting_turbo(void);


extern z80_bit tbblue_force_disable_layer_ula;
extern z80_bit tbblue_force_disable_layer_tilemap;
extern z80_bit tbblue_force_disable_layer_sprites;
extern z80_bit tbblue_force_disable_layer_layer_two;

extern z80_bit tbblue_allow_layer2_priority_bit;

extern z80_bit tbblue_force_disable_cooper;

extern int tbblue_if_sprites_enabled(void);
extern int tbblue_if_ula_is_enabled(void);

extern char *tbblue_get_string_layer_prio(int layer,z80_byte prio);

extern void tbblue_get_string_palette_format(char *texto);

extern int tbblue_pendiente_retn_stackless;
extern void tbblue_handle_nmi(void);

//extern int tbblue_prueba_dentro_nmi;

extern int tbblue_get_offset_start_layer2_reg(z80_byte register_value);
extern int tbblue_get_offset_start_tilemap(void);
extern int tbblue_get_offset_start_tiledef(void);
extern z80_byte tbblue_get_ram_page_tilemap(void);
extern z80_byte tbblue_get_ram_page_tiledef(void);

extern int tbblue_get_tilemap_width(void);
extern int tbblue_if_tilemap_enabled(void);

extern z80_byte tbblue_get_layers_priorities(void);

extern z80_byte tbblue_get_register_port(void);

extern void get_pixel_color_tbblue(z80_byte attribute,z80_int *tinta_orig, z80_int *papel_orig);

extern void screen_tbblue_refresca_no_rainbow(void);

extern void screen_tbblue_refresca_rainbow(void);

extern z80_byte tbblue_machine_id;

extern z80_byte tbblue_board_id;

struct s_tbblue_machine_id_definition {
    z80_byte id;
    char nombre[32];
};

extern struct s_tbblue_machine_id_definition tbblue_machine_id_list[];

extern z80_byte *get_lores_pointer(int y);

extern void tbblue_out_port_32765(z80_byte value);
extern void tbblue_out_port_8189(z80_byte value);

extern z80_byte tbblue_uartbridge_readdata(void);
extern void tbblue_uartbridge_writedata(z80_byte value);

extern z80_byte tbblue_uartbridge_readstatus(void);
extern int tbblue_tiles_are_monocrome(void);

extern z80_byte tbblue_get_diviface_enabled(void);

extern int debug_tbblue_sprite_visibility[];

extern void tbblue_retn(void);

extern void tbblue_dac_mix(void);

extern void tbblue_out_port_dac(z80_byte puerto_l,z80_byte value);

extern z80_byte tbblue_dac_a;
extern z80_byte tbblue_dac_b;
extern z80_byte tbblue_dac_c;
extern z80_byte tbblue_dac_d;

#endif
