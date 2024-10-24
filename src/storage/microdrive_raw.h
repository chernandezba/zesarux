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

#ifndef MICRODRIVE_RAW_H
#define MICRODRIVE_RAW_H

#include "cpu.h"

#define MICRODRIVE_RAW_HEADER_SIZE 256

#define MICRODRIVE_RAW_SIGNATURE "RAWMDV"

//789 bytes por sector es lo que me salen mis calculos en formato raw, formateado desde la rom de interface1
//El tamaño comun sera eso multiplicado por 254 sectores
#define MICRODRIVE_RAW_COMMON_SECTOR_SIZE 789
#define MICRODRIVE_RAW_COMMON_SIZE (MICRODRIVE_RAW_COMMON_SECTOR_SIZE*254)

#define MICRODRIVE_RAW_INFO_BYTE_MASK_BAD_POSITION  0x0200

#define MICRODRIVE_RAW_INFO_BYTE_MASK_DATA          0x0100

//extern int microdrive_is_raw;

extern int microdrive_raw_pending_read_port;

extern z80_int microdrive_raw_last_read_byte;

extern int microdrive_raw_pending_status_port;

extern int estado_wait_por_puerto_tipo;

extern z80_byte microdrive_raw_status_ef(void);

extern void mdr_raw_write_byte(z80_byte value);

extern z80_byte microdrive_raw_read_port_e7(void);

extern z80_byte microdrive_raw_read_port_ef(void);

extern void microdrive_raw_insert(int microdrive_seleccionado);

extern int microdrive_current_is_raw(void);

extern void microdrive_raw_flush_to_disk_one(int microdrive_seleccionado);

extern void microdrive_raw_create_header(char *destino);

extern void microdrive_raw_move(void);

extern void microdrive_raw_mark_bad_position(int microdrive_seleccionado,int position);

extern void microdrive_raw_unmark_bad_position(int microdrive_seleccionado,int position);

extern void microdrive_raw_insert(int microdrive_seleccionado);

extern void microdrive_raw_flush_to_disk_one(int microdrive_seleccionado);

extern void microdrive_raw_full_erase(int microdrive_seleccionado);


#endif