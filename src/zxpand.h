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

#ifndef ZXPAND_H
#define ZXPAND_H

#include "cpu.h"

#define ZXPAND_FR_OK 0 
#define ZXPAND_FR_DISK_ERR 1 
#define ZXPAND_FR_INT_ERR 2 
#define ZXPAND_FR_NOT_READY 3 
#define ZXPAND_FR_NO_FILE 4 
#define ZXPAND_FR_NO_PATH 5 
#define ZXPAND_FR_INVALID_NAME 6 
#define ZXPAND_FR_DENIED 7 
#define ZXPAND_FR_EXIST 8 
#define ZXPAND_FR_INVALID_OBJECT 9 
#define ZXPAND_FR_WRITE_PROTECTED 10 
#define ZXPAND_FR_INVALID_DRIVE 11 
#define ZXPAND_FR_NOT_ENABLED 12 
#define ZXPAND_FR_NO_FILESYSTEM 13 
#define ZXPAND_FR_MKFS_ABORTED 14 
#define ZXPAND_FR_TIMEOUT 15 

extern z80_bit zxpand_enabled;

extern void zxpand_enable(void);
extern void zxpand_disable(void);

extern z80_byte zxpand_read(z80_byte puerto_h);
extern void zxpand_write(z80_byte puerto_h,z80_byte value);

extern z80_byte *zxpand_memory_pointer;
extern z80_bit zxpand_overlay_rom;

//extern int zxpand_operating_counter;

extern z80_bit zxpand_overlay_rom;

extern void delete_zxpand_text(void);

extern char zxpand_root_dir[];

extern char zxpand_cwd[];

extern z80_bit dragons_lair_hack;

#endif
