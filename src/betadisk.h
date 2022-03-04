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

#ifndef BETADISK_H
#define BETADISK_H

#include "cpu.h"

#define BETADISK_SIZE 16384

#define BETADISK_ROM_FILENAME "trdos.rom"

extern z80_bit betadisk_enabled;
extern z80_bit betadisk_active;

extern z80_bit betadisk_allow_boot_48k;

extern void betadisk_enable(void);
extern void betadisk_disable(void);

extern void betadisk_reset(void);


extern z80_byte *betadisk_memory_pointer;
extern int betadisk_check_if_rom_area(z80_int dir);

extern char trd_file_name[];

extern z80_bit trd_enabled;

#define TRD_FILE_SIZE (640*1024)

extern void trd_enable(void);

extern void trd_disable(void);

extern void trd_flush_contents_to_disk(void);

#define TRD_SYM_trdos_variable_sector_rw_flag 23758
#define TRD_SYM_trdos_variable_current_sector 23796
#define TRD_SYM_trdos_variable_current_track 23797

extern z80_bit trd_write_protection;

extern z80_bit trd_persistent_writes;

extern void trd_insert_disk(char *nombre);

#endif
