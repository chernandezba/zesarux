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

#ifndef DEBUG_TIMINGS_H
#define DEBUG_TIMINGS_H

#include "cpu.h"

#define SPECIAL_TIMING_VALUE_DDFD_INEXISTENT 221

//7 maximo pero 1 mas para tiempos de ddfd de opcodes inexistentes, que se generan con un valor mas al principio
#define MAX_TIEMPOS_OPCODES 8

//Tablas de tiempos de instrucciones
struct s_opcodes_times {
    //Usado en todos opcodes. Y Usado en opcodes con condición, cuando no se cumple condición.
    int times_condition_not_triggered[MAX_TIEMPOS_OPCODES];

    //Usado en opcodes con condición, cuando se cumple condición
    int times_condition_triggered[MAX_TIEMPOS_OPCODES];
};

extern struct s_opcodes_times *debug_get_timing_opcode(z80_byte byte1,z80_byte byte2,z80_byte byte4);

#endif
