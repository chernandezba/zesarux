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

#ifndef SCORPION_H
#define SCORPION_H

#include "cpu.h"

#define SCORPION_ROM_SIZE 64
#define SCORPION_RAM_SIZE 256
#define SCORPION_TOTAL_RAM_PAGES (SCORPION_RAM_SIZE/16)
#define SCORPION_TOTAL_ROM_PAGES (SCORPION_ROM_SIZE/16)

extern void scorpion_malloc_mem_machine(void);

extern void scorpion_mem_page_rom(void);
extern void scorpion_mem_page_ram(void);
extern void scorpion_write_port_1ffd(z80_byte value);

extern z80_byte *scorpion_ram_mem_table[];
extern z80_byte *scorpion_memory_paged[];

#endif
