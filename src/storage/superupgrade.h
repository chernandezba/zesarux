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

#ifndef SUPERUPGRADE_H
#define SUPERUPGRADE_H

#include "cpu.h"

#define SUPERUPGRADE_RAM_SIZE (512*1024)
#define SUPERUPGRADE_ROM_SIZE (512*1024)

extern z80_bit superupgrade_enabled;

extern void superupgrade_enable(int hard_reset);
extern void superupgrade_disable(void);

extern char superupgrade_rom_file_name[];

extern void superupgrade_set_memory_pages(void);

extern z80_byte *superupgrade_ram_memory_table[];

extern void superupgrade_hard_reset(void);

extern int superupgrade_supported_machine(void);

extern void superupgrade_write_7ffd(z80_byte value);
extern void superupgrade_write_1ffd(z80_byte value);
extern void superupgrade_write_43b(z80_byte value);

extern z80_bit superupgrade_flash_write_protected;

extern z80_byte superupgrade_get_rom_bank(void);

//extern int superupgrade_flash_operating_counter;

extern void delete_superupgrade_flash_text(void);
extern void superupgrade_flush_flash_to_disk(void);

extern z80_byte superupgrade_puerto_43b;

extern int si_superupgrade_muestra_rom_interna(void);

extern z80_byte *superupgrade_rom_memory_pointer;
extern z80_byte *superupgrade_ram_memory_pointer;


#endif
