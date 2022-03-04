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

#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H

#include "cpu.h"

extern void debugger_disassemble( char *buffer, size_t buflen, size_t *length, unsigned int address );
extern void debugger_disassemble_array (char *buffer, size_t buflen, size_t *length, unsigned int address );
extern z80_byte disassemble_array[];

#define DISASSEMBLE_ARRAY_LENGTH 10

#endif
