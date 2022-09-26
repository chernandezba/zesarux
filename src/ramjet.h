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

#ifndef RAMJET_H
#define RAMJET_H

#include "cpu.h"

#define RAMJET_ROM "ramjet2.rom"

#define RAMJET_ROM_SIZE 16384


extern z80_bit ramjet_enabled;


extern void ramjet_reset(void);
extern void ramjet_enable(void);
extern void ramjet_disable(void);
extern void ramjet_nmi(void);

extern z80_byte *ramjet_memory_pointer;

extern z80_bit ramjet_mapped_rom_memory;

extern void ramjet_write_port(z80_int puerto,z80_byte value);

#endif
