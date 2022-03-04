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

#ifndef PRINTERS_H
#define PRINTERS_H

#include "cpu.h"

extern z80_bit zxprinter_enabled;
extern z80_bit zxprinter_motor;
extern z80_bit zxprinter_power;
extern int zxprinter_speed;
extern int zxprinter_x;

extern z80_byte zxprinter_get_port(void);

extern z80_byte zxprinter_get_port(void);
extern void zxprinter_write_port(z80_byte value);
extern char *zxprinter_ocr_filename;
extern char *zxprinter_bitmap_filename;
extern int zxprinter_file_bitmap_init(void);
extern int zxprinter_file_ocr_init(void);
extern void eject_zxprinter_bitmap_file(void);
extern void close_zxprinter_bitmap_file(void);
extern void close_zxprinter_ocr_file(void);

extern int printing_counter;
extern void draw_print_text(void);
extern void delete_print_text(void);



#endif
