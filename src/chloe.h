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

#ifndef CHLOE_H
#define CHLOE_H

#include "cpu.h"

extern z80_byte *chloe_rom_mem_table[];

extern z80_byte *chloe_home_ram_mem_table[];

extern z80_byte *chloe_ex_ram_mem_table[];

extern z80_byte *chloe_dock_ram_mem_table[];

extern z80_byte *chloe_memory_paged[];

extern void chloe_set_memory_pages(void);

extern void chloe_init_memory_tables(void);

extern z80_byte chloe_type_memory_paged[];

//Tipos de memoria mapeadas
//0=rom
//1=home
//2=dock
//3=ex
//Constantes definidas en CHLOE_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX

#define CHLOE_MEMORY_TYPE_ROM  0
#define CHLOE_MEMORY_TYPE_HOME 1
#define CHLOE_MEMORY_TYPE_DOCK 2
#define CHLOE_MEMORY_TYPE_EX   3

extern z80_byte debug_chloe_paginas_memoria_mapeadas[];

extern void chloe_out_ula2(z80_byte value);


#endif
