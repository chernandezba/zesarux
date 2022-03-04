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

#ifndef MULTIFACE_H
#define MULTIFACE_H

#include "cpu.h"

//ROM 8kb, RAM 8kb
#define MULTIFACE_SIZE (16*1024)

#define MULTIFACE_TOTAL_TYPES 3
#define MULTIFACE_TYPE_ONE 0
#define MULTIFACE_TYPE_128 1
#define MULTIFACE_TYPE_THREE 2

//#define MULTIFACE_PORT_ONE_IN    0x9f
//#define MULTIFACE_PORT_ONE_OUT   0x1f
//#define MULTIFACE_PORT_128_IN    0xbf
//#define MULTIFACE_PORT_128_OUT   0x3f
//#define MULTIFACE_PORT_THREE_IN  0x3f
//#define MULTIFACE_PORT_THREE_OUT 0xbf

extern z80_byte multiface_type;

extern char *multiface_types_string[];

extern z80_bit multiface_enabled;

extern void multiface_enable(void);
extern void multiface_disable(void);

extern char multiface_rom_file_name[];

extern void multiface_map_memory(void);
extern void multiface_unmap_memory(void);

//extern z80_byte multiface_current_port_in;
//extern z80_byte multiface_current_port_out;

extern int multiface_lockout;

extern z80_bit multiface_switched_on;

extern z80_byte *multiface_memory_pointer;


#endif
