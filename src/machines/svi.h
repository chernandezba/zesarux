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

#ifndef SVI_H
#define SVI_H

#include "cpu.h"

extern z80_byte *svi_vram_memory;

extern z80_byte svi_ppi_register_a;
extern z80_byte svi_ppi_register_b;
extern z80_byte svi_ppi_register_c;

extern z80_bit svi_sound_cassette_out;

extern z80_byte svi_keyboard_table[];
extern z80_byte svi_read_vram_byte(z80_int address);

//extern int svi_memory_slots[4][4];

extern z80_byte *svi_return_segment_address(z80_int direccion,int *tipo);

extern void screen_store_scanline_rainbow_svi_border_and_display(void);

extern int da_amplitud_speaker_svi(void);

#define SVI_SLOT_MEMORY_TYPE_ROM 0
#define SVI_SLOT_MEMORY_TYPE_RAM 1
#define SVI_SLOT_MEMORY_TYPE_EMPTY 2

extern void svi_insert_rom_cartridge(char *filename);
extern void svi_empty_romcartridge_space(void);
extern void svi_out_port_vdp_data(z80_byte value);
extern void svi_out_port_vdp_command_status(z80_byte value);
extern void svi_out_port_psg(z80_byte puerto_l,z80_byte value);
extern void svi_out_port_ppi(z80_byte puerto_l,z80_byte value);
extern z80_byte svi_in_port_vdp_data(void);
extern z80_byte svi_in_port_vdp_status(void);
extern z80_byte svi_in_port_ppi(z80_byte puerto_l);
extern void svi_reset(void);
extern void svi_alloc_vram_memory(void);
extern void svi_init_memory_tables(void);
extern void scr_refresca_pantalla_y_border_svi(void);

extern char *svi_get_string_memory_type(int tipo);


extern int svi_cas_load_detect(void);
extern void svi_cas_load(void);

extern void svi_get_string_memory_slot(char *buffer_mem_type,char *long_buffer_memory_type, z80_byte slot,z80_byte segment);

extern z80_bit svi_cartridge_inserted;

extern z80_byte svi_read_psg(void);

#endif
