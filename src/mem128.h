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

#ifndef MEM128_H
#define MEM128_H

#include "cpu.h"

extern z80_byte *ram_mem_table[];
extern z80_byte *rom_mem_table[];
extern z80_byte *memory_paged[];
extern z80_byte *get_base_mem_pantalla(void);

#define DEBUG_PAGINA_MAP_ES_ROM 32768
#define DEBUG_PAGINA_MAP_MASK 32767
extern z80_int debug_paginas_memoria_mapeadas[];

extern int mem128_multiplicador;

extern z80_byte puerto_32765;
extern z80_byte puerto_8189;
extern void mem_page_ram_rom(void);
extern void mem_set_normal_pages_p2a(void);
extern void mem_set_normal_pages_128k(void);
extern void mem_page_ram_p2a(void);
extern void mem_page_rom_p2a(void);
extern void mem_page_ram_128k(void);
extern void mem_page_rom_128k(void);

extern int get_actual_rom_p2a(void);
extern int get_actual_rom_128k(void);
extern void mem128_p2a_write_page_port(z80_int puerto, z80_byte value);

extern z80_byte *get_base_mem_pantalla_attributes(void);

extern void mem_init_memory_tables_128k(void);
extern void mem_init_memory_tables_p2a(void);

extern void mem_set_multiplicador_128(z80_byte valor);
extern int mem_paging_is_enabled(void);

#endif
