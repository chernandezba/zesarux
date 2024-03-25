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

#ifndef HILOW_BARBANEGRA_H
#define HILOW_BARBANEGRA_H

#include "cpu.h"

#define HILOW_BARBANEGRA_ROM_FILE_NAME "hilow_barbanegra.rom"


#define HILOW_BARBANEGRA_ROM_SIZE 8192

#define HILOW_BARBANEGRA_RAM_SIZE 2048


#define HILOW_BARBANEGRA_MEM_SIZE (HILOW_BARBANEGRA_ROM_SIZE+HILOW_BARBANEGRA_RAM_SIZE)

extern z80_bit hilow_bbn_enabled;
extern z80_bit hilow_bbn_mapped_memory;

extern void hilow_bbn_reset(void);
extern void hilow_bbn_write_port_fd(z80_int port,z80_byte value);
extern void hilow_bbn_enable(void);
extern void hilow_bbn_disable(void);
extern void hilow_bbn_nmi(void);

extern z80_byte *hilow_bbn_memory_pointer;

#endif
