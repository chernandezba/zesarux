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

#ifndef DANDANATOR_H
#define DANDANATOR_H

#include "cpu.h"

#define DANDANATOR_SIZE (512*1024)

extern z80_bit dandanator_enabled;

extern void dandanator_enable(void);
extern void dandanator_disable(void);

extern char dandanator_rom_file_name[];

extern z80_bit dandanator_switched_on;
//extern z80_bit dandanator_accepting_commands;
extern z80_byte dandanator_active_bank;
extern void dandanator_press_button(void);

extern z80_byte *dandanator_memory_pointer;

extern z80_bit dandanator_cpc_received_preffix;

#endif
