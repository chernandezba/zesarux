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

#ifndef CHROME_H
#define CHROME_H

#include "cpu.h"

extern void chrome_set_memory_pages(void);

extern void chrome_init_memory_tables(void);

extern z80_byte *chrome_rom_mem_table[4];

extern z80_byte *chrome_ram_mem_table[10];

extern z80_byte *chrome_memory_paged[];

extern int si_chrome_features_enabled(void);

extern int chrome_ram89_at_00(void);

#endif
