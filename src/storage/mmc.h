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
#include "zesarux.h"

#define MMC_MAX_CARDS 2


extern void mmc_cs(z80_byte value);

extern z80_bit mmc_mirror_second_card;



extern void mmc_flush_flash_to_disk(void);

extern z80_byte mmc_read(void);

extern void mmc_write(z80_byte value);


extern void zxmmc_write_port(z80_byte puerto_l,z80_byte value);

extern z80_byte zxmmc_read_port(z80_byte puerto_l);
extern z80_byte mmc_last_port_value_1f;







extern void mmc_enable(int tarjeta);
extern void mmc_disable(int tarjeta);

extern void mmc_reset(int card);

extern int mmc_read_file_to_memory(int tarjeta);


extern char mmc_file_name[MMC_MAX_CARDS][PATH_MAX];

extern z80_bit mmc_write_protection[];

extern z80_bit mmc_persistent_writes[];


extern z80_byte mmc_last_command[];

extern z80_byte mmc_r1[];

extern char mmc_filemap_name[MMC_MAX_CARDS][PATH_MAX];
extern int mmc_filemap_from_esxdos[];

extern z80_bit mmc_sdhc_addressing[];

extern z80_bit mmc_enabled[];

#endif
