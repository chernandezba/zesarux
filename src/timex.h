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

#ifndef TIMEX_H
#define TIMEX_H

#include "cpu.h"

extern z80_byte timex_port_f4;
extern z80_byte timex_port_ff;
extern z80_bit timex_video_emulation;


extern z80_byte *timex_rom_mem_table[];

extern z80_byte *timex_home_ram_mem_table[];

extern z80_byte *timex_ex_rom_mem_table[];

extern z80_byte *timex_dock_rom_mem_table[];

extern z80_byte *timex_memory_paged[];

extern void timex_set_memory_pages(void);

extern void timex_init_memory_tables(void);

extern z80_byte timex_type_memory_paged[];

//Tipos de memoria mapeadas
//0=rom
//1=home
//2=dock
//3=ex
//Constantes definidas en TIMEX_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX

#define TIMEX_MEMORY_TYPE_ROM  0
#define TIMEX_MEMORY_TYPE_HOME 1
#define TIMEX_MEMORY_TYPE_DOCK 2
#define TIMEX_MEMORY_TYPE_EX   3

extern z80_byte debug_timex_paginas_memoria_mapeadas[];

extern void timex_empty_dock_space(void);

extern void timex_insert_dck_cartridge(char *filename);

extern z80_bit timex_cartridge_inserted;


extern int get_timex_ink_mode6_color(void);
extern int get_timex_border_mode6_color(void);

extern int timex_si_modo_512(void);

extern z80_bit timex_mode_512192_real;

extern int timex_si_modo_512_y_zoom_par(void);

extern int timex_si_modo_shadow(void);
extern int timex_si_modo_8x1(void);

extern int get_timex_paper_mode6_color(void);

extern int timex_ugly_hack_enabled;
extern int timex_ugly_hack_last_hires;

extern void set_timex_port_ff(z80_byte value);


#endif
