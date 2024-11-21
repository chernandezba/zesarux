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

#ifndef LEC_H
#define LEC_H

#include "cpu.h"

#define LEC_MAX_RAM_BLOCKS 16
#define LEC_MAX_RAM_SIZE (LEC_MAX_RAM_BLOCKS*32768)

extern z80_bit lec_enabled;
extern void lec_out_port(z80_byte value);
extern void lec_reset(void);

extern void lec_disable(void);
extern void lec_enable(void);

extern int lec_memory_type;
extern int lec_all_ram(void);
extern z80_byte *lec_memory_pages[];

extern int lec_get_total_memory_size(void);
extern z80_byte *lec_ram_memory_pointer;
extern void lec_set_memory_pages(void);

#endif
