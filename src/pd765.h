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

#ifndef PD765_H
#define PD765_H

#include "cpu.h"

extern z80_bit pd765_enabled;
extern void pd765_enable(void);
extern void pd765_disable(void);

extern void pd765_motor_on(void);
extern void pd765_motor_off(void);
extern void pd765_write_command(z80_byte value);
extern z80_byte pd765_read_command(void);
extern z80_byte pd765_read_status_register(void);

extern void traps_plus3dos(void);

extern z80_bit plus3dos_traps;

extern char dskplusthree_file_name[];

extern z80_bit dskplusthree_emulation;

extern void dskplusthree_disable(void);
extern void dskplusthree_enable(void);

extern z80_bit dskplusthree_write_protection;

extern z80_bit dskplusthree_persistent_writes;

extern void dskplusthree_flush_contents_to_disk(void);

extern void dsk_insert_disk(char *nombre);

#endif
