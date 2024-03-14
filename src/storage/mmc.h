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

#ifndef MMC_H
#define MMC_H

#include "cpu.h"

extern z80_bit mmc_enabled;
extern void mmc_cs(z80_byte value);

extern void mmc_enable(void);
extern void mmc_disable(void);

extern void mmc_flush_flash_to_disk(void);

extern char mmc_file_name[];

extern z80_byte mmc_read(void);

extern void mmc_write(z80_byte value);

extern void mmc_reset(void);

extern int mmc_read_file_to_memory(void);

extern z80_bit mmc_write_protection;

extern z80_bit mmc_persistent_writes;


extern z80_byte mmc_last_command;

extern z80_byte mmc_r1;

#endif
