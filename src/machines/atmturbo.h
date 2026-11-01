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

#ifndef ATMTURBO_H
#define ATMTURBO_H

#include "cpu.h"


#define ATMTURBO_ROM_SIZE 64
#define ATMTURBO_RAM_SIZE 512
#define ATMTURBO_TOTAL_RAM_PAGES (ATMTURBO_RAM_SIZE/16)
#define ATMTURBO_TOTAL_ROM_PAGES (ATMTURBO_ROM_SIZE/16)

extern z80_byte *atmturbo_ram_mem_table[];

extern z80_byte *atmturbo_rom_mem_table[];


extern void poke_byte_atmturbo(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_atmturbo(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_atmturbo(z80_int dir);
extern z80_byte peek_byte_no_time_atmturbo(z80_int dir);

extern void atmturbo_reset(void);

extern void atmturbo_malloc_mem_machine(void);

#endif
