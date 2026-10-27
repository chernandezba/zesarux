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

#ifndef IDE_H
#define IDE_H

#include "cpu.h"

extern z80_bit ide_enabled;
extern void ide_cs(z80_byte value);

extern void ide_enable(void);
extern void ide_disable(void);

extern void ide_flush_flash_to_disk(void);

extern char ide_file_name[];

extern z80_byte ide_read(void);

extern void ide_write(z80_byte value);


extern void ide_write_command_block_register(z80_byte ide_register,z80_byte value);
extern z80_byte ide_read_command_block_register(z80_byte ide_register);

extern void ide_reset(void);

extern z80_bit eight_bit_simple_ide_enabled;

extern z80_byte eight_bit_simple_ide_read(z80_byte port);
extern void eight_bit_simple_ide_write(z80_byte port,z80_byte value);

extern void eight_bit_simple_ide_enable(void);
extern void eight_bit_simple_ide_disable(void);

extern int ide_read_file_to_memory(void);

extern z80_bit ide_write_protection;

extern z80_bit ide_persistent_writes;

extern z80_byte ide_get_data_register(void);

extern z80_byte ide_get_error_register(void);

extern z80_byte ide_register_sector_count;
extern z80_byte ide_register_sector_number;
extern z80_byte ide_register_cylinder_low;
extern z80_byte ide_register_cylinder_high;
extern z80_byte ide_register_drive_head;
extern z80_byte ide_status_register;

extern z80_bit ide_smuc_enabled;
extern int ide_smuc_dma_selected;
extern int ide_smuc_read(z80_int port,z80_byte *value);
extern int ide_smuc_write(z80_int port,z80_byte value);
extern void ide_smuc_reset(void);

extern z80_byte ide_smuc_control;
extern z80_byte ide_smuc_config;
extern z80_byte ide_smuc_data_high;
extern int ide_smuc_identify_first_word;
extern int ide_smuc_eeprom_initialized;
extern int ide_smuc_eeprom_state;
extern int ide_smuc_eeprom_address;
extern int ide_smuc_eeprom_input;
extern int ide_smuc_eeprom_input_bits;
extern int ide_smuc_eeprom_output;
extern int ide_smuc_eeprom_output_bits;
extern int ide_smuc_eeprom_release;
extern z80_byte ide_smuc_eeprom_bus;

#endif
