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

#ifndef TRANSTAPE_H
#define TRANSTAPE_H

#include "cpu.h"

#define TRANSTAPE_ROM_FILE_NAME "transtape3.rom"


#define TRANSTAPE_ROM_SIZE 16384

#define TRANSTAPE_RAM_SIZE 2048


#define TRANSTAPE_MEM_SIZE (TRANSTAPE_ROM_SIZE+TRANSTAPE_RAM_SIZE)

extern z80_bit transtape_enabled;


extern void transtape_reset(void);
extern void transtape_enable(void);
extern void transtape_disable(void);
extern void transtape_nmi(void);

extern z80_byte *transtape_memory_pointer;

extern void transtape_write_port(z80_byte puerto_l,z80_byte value);

extern z80_int conmutadores_load_save_turbo;

#endif
