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

#ifndef DIDAKTIKGAMA_H
#define DIDAKTIKGAMA_H

#include "cpu.h"

#define DIDAKTIKGAMA_ROM_SIZE 16
#define DIDAKTIKGAMA_RAM_SIZE 80

//las dos rams de 32kb
#define DIDAKTIKGAMA_TOTAL_RAM_PAGES 2


extern void didaktikgama_malloc_mem_machine(void);


extern void didaktikgama_mem_page_ram(void);


extern z80_byte *didaktikgama_ram_mem_table[];
extern z80_byte *didaktikgama_memory_paged[];
extern void didaktikgama_set_normal_pages(void);
extern void didaktikgama_reset(void);

extern void poke_byte_no_time_didaktikgama(z80_int dir,z80_byte valor);
extern void poke_byte_didaktikgama(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_no_time_didaktikgama(z80_int dir);
extern z80_byte peek_byte_didaktikgama(z80_int dir);

extern void didaktikgama_out_port(z80_int puerto, z80_byte value);
extern z80_byte didaktikgama_in_port(z80_int puerto);


#endif
