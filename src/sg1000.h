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

#ifndef SG1000_H
#define SG1000_H

#include "cpu.h"

extern z80_byte *sg1000_vram_memory;


extern z80_byte sg1000_read_vram_byte(z80_int address);

extern z80_bit sg1000_cartridge_inserted;


extern z80_byte *sg1000_return_segment_address(z80_int direccion,int *tipo);



#define SG1000_SLOT_MEMORY_TYPE_ROM 0
#define SG1000_SLOT_MEMORY_TYPE_RAM 1
#define SG1000_SLOT_MEMORY_TYPE_EMPTY 2

extern void sg1000_insert_rom_cartridge(char *filename);
extern void sg1000_empty_romcartridge_space(void);
extern void sg1000_out_port_vdp_data(z80_byte value);
extern void sg1000_out_port_vdp_command_status(z80_byte value);


extern z80_byte sg1000_in_port_vdp_data(void);
extern z80_byte sg1000_in_port_vdp_status(void);

extern void sg1000_reset(void);
extern void sg1000_alloc_vram_memory(void);
extern void sg1000_init_memory_tables(void);
extern void scr_refresca_pantalla_y_border_sg1000(void);

extern char *sg1000_get_string_memory_type(int tipo);

extern void screen_store_scanline_rainbow_sg1000_border_and_display(void);


extern z80_byte sg1000_get_joypad_a(void);
extern z80_byte sg1000_get_joypad_b(void);

#endif
