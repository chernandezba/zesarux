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

#ifndef SAMRAM_H
#define SAMRAM_H

#include "cpu.h"

#define SAMRAM_SIZE 65536

extern z80_bit samram_enabled;


extern void samram_write_port(z80_byte value);
extern void samram_enable(void);
extern void samram_disable(void);

extern char samram_rom_file_name[];

//extern void samram_press_button(void);

extern void samram_nmi(void);

extern z80_byte *samram_memory_pointer;

extern int samram_tap_save_detect(void);

extern void samram_opcode_edf9(void);
extern void samram_opcode_edfa(void);
extern void samram_opcode_edfb(void);
extern void samram_opcode_edfe(void);

#endif
