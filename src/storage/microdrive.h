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

#ifndef MICRODRIVE_H
#define MICRODRIVE_H

#include "cpu.h"


#define MDR_BYTES_PER_SECTOR 543
#define MDR_MAX_FILE_SIZE 137923

extern z80_byte mdr_next_byte(void);

extern void mdr_next_sector(void);

extern char microdrive_file_name[];

extern z80_bit microdrive_enabled;

extern void microdrive_insert(void);

extern void microdrive_footer_operating(void);

extern void microdrive_eject(void);
extern z80_byte microdrive_status_ef(void);

extern int mdr_current_sector;
extern int mdr_current_offset_in_sector;

extern z80_bit microdrive_write_protect;

extern void mdr_write_byte(z80_byte valor);

extern void microdrive_flush_to_disk(void);

extern void microdrive_write_port_ef(z80_byte value);

extern int microdrive_formateando;

extern int escrito_byte_info_una_vez;

extern z80_bit microdrive_persistent_writes;

#define MICRODRIVE_STATUS_BIT_GAP  4
#define MICRODRIVE_STATUS_BIT_SYNC 2

//Si vale 1, es que no esta protegido
#define MICRODRIVE_STATUS_BIT_NOT_WRITE_PROTECT 1

#define MAX_MICRODRIVES 8

struct s_microdrive_status {
    int motor_on;
};

#endif
