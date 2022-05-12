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

#ifndef HILOW_H
#define HILOW_H

#include "cpu.h"

#define HILOW_ROM_FILE_NAME "hilow.rom"


#define HILOW_ROM_SIZE 8192

#define HILOW_RAM_SIZE 2048
//#define HILOW_RAM_SIZE 8192

#define HILOW_SECTOR_SIZE 2048

//8 KB rom, 2 kb ram
//Creo que son 8 kb ram...
#define HILOW_MEM_SIZE (HILOW_ROM_SIZE+HILOW_RAM_SIZE)

extern z80_byte *hilow_memory_pointer;
 
//extern void hilow_press_button(void);
extern void hilow_enable(void);
extern void hilow_disable(void);
extern void hilow_reset(void);

extern z80_bit hilow_enabled;

extern z80_bit hilow_mapped_rom;
extern z80_bit hilow_mapped_ram;

extern z80_byte hilow_read_port_ff(z80_int puerto);
extern void hilow_write_port_ff(z80_int port,z80_byte value);

extern z80_bit hilow_cinta_insertada;
extern z80_bit hilow_tapa_abierta;

#endif
