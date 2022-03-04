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

#ifndef MENU_DEBUG_CPU_H
#define MENU_DEBUG_CPU_H

#include "cpu.h"

extern void menu_debug_registers(MENU_ITEM_PARAMETERS);
extern void menu_debug_registers_view_adventure(MENU_ITEM_PARAMETERS);
extern void menu_watches(MENU_ITEM_PARAMETERS);
extern void menu_debug_registers_run_cpu_opcode(void);
extern void menu_debug_textadventure_map_connections(MENU_ITEM_PARAMETERS);

extern int map_adventure_offset_x;
extern int map_adventure_offset_y;

#endif