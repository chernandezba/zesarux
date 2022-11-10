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

#ifndef DSK_H
#define DSK_H

#include "cpu.h"

//Ejemplo metal action:  214784 14 sep  2000 Metal Action 1 - Side A.dsk
#define DSK_MAX_BUFFER_DISCO 214784



extern void dsk_insert_disk(char *nombre);

extern char dskplusthree_file_name[];

extern void dskplusthree_flush_contents_to_disk(void);

extern z80_bit dskplusthree_emulation;


extern z80_bit dskplusthree_write_protection;

extern z80_bit dskplusthree_persistent_writes;


extern void dskplusthree_disable(void);
extern void dskplusthree_enable(void);

extern void plus3dsk_put_byte_disk(int offset,z80_byte value);
extern z80_byte plus3dsk_get_byte_disk(int offset);

extern z80_byte p3dsk_buffer_disco[];

extern void dsk_show_activity(void);

extern int dsk_get_sector(int pista,int parametro_r);

extern void dsk_get_chrn(int pista,int sector,int *parametro_c,int *parametro_h,int *parametro_r,int *parametro_n);

#endif
