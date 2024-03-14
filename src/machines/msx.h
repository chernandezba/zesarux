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

#ifndef MSX_H
#define MSX_H

#include "cpu.h"

extern z80_byte *msx_vram_memory;

extern z80_byte msx_ppi_register_a;
extern z80_byte msx_ppi_register_b;
extern z80_byte msx_ppi_register_c;

extern z80_byte msx_keyboard_table[];
extern z80_byte msx_read_vram_byte(z80_int address);

extern int msx_memory_slots[4][4];

extern z80_byte *msx_return_segment_address(z80_int direccion,int *tipo);

extern void screen_store_scanline_rainbow_msx_border_and_display(void);

extern int da_amplitud_speaker_msx(void);

#define MSX_SLOT_MEMORY_TYPE_ROM 0
#define MSX_SLOT_MEMORY_TYPE_RAM 1
#define MSX_SLOT_MEMORY_TYPE_EMPTY 2

extern void msx_insert_rom_cartridge(char *filename);
extern void msx_empty_romcartridge_space(void);
extern void msx_out_port_vdp_data(z80_byte value);
extern void msx_out_port_vdp_command_status(z80_byte value);
extern void msx_out_port_psg(z80_byte puerto_l,z80_byte value);
extern void msx_out_port_ppi(z80_byte puerto_l,z80_byte value);
extern z80_byte msx_in_port_vdp_data(void);
extern z80_byte msx_in_port_vdp_status(void);
extern z80_byte msx_in_port_ppi(z80_byte puerto_l);
extern void msx_reset(void);
extern void msx_alloc_vram_memory(void);
extern void msx_init_memory_tables(void);
extern void scr_refresca_pantalla_y_border_msx(void);

extern char *msx_get_string_memory_type(int tipo);

extern int tape_block_cas_open(void);

extern z80_byte msx_cabecera_firma[];

extern int msx_cas_load_detect(void);
extern void msx_cas_load(void);

extern z80_bit msx_cartridge_inserted;
extern z80_byte msx_read_psg(void);

#endif
