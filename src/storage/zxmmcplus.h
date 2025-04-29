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

#ifndef ZXMMCPLUS_H
#define ZXMMCPLUS_H

#include "cpu.h"

#define ZXMMCPLUS_FLASHROM_SIZE (512*1024)
#define ZXMMCPLUS_RAM_SIZE (512*1024)

#define ZXMMCPLUS_FLASHROM_FILE_NAME "zxmmcplus.flash"

extern int zxmmcplus_flashrom_must_flush_to_disk;
extern int zxmmcplus_load_flash(void);
extern char zxmmcplus_flash_name[];
extern z80_bit zxmmcplus_flashrom_write_protect;
extern z80_bit zxmmcplus_flashrom_persistent_writes;
extern z80_bit zxmmcplus_enabled;
extern void zxmmcplus_flashrom_flush_contents_to_disk(void);
extern void zxmmcplus_mmc_cs(z80_byte value);
extern void zxmmcplus_reset(void);
extern z80_byte zxmmcplus_read_port(void);
extern void zxmmcplus_write_port(z80_byte value);
extern void zxmmcplus_disable(void);
extern void zxmmcplus_enable(void);
extern void zxmmcplus_sync_bit_7ffd(void);
extern z80_byte *zxmmcplus_memory_pointer;
extern int zxmmcplus_paged_on_read(void);
extern int zxmmcplus_rom_selected(void);
extern int zxmmcplus_get_page_number(void);
extern void zxmmcplus_clear_ram(void);

#endif
