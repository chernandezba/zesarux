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

#ifndef TAPE_TAP_H
#define TAPE_TAP_H

#include "cpu.h"


extern int tape_block_tap_open(void);
extern int tape_block_tap_read(void *dir,int longitud);
extern int tape_block_tap_readlength(void);
extern int tape_block_tap_seek(int longitud,int direccion);
extern int tape_block_tap_feof(void);
extern void tape_block_tap_rewindbegin(void);

extern int tape_out_block_tap_open(void);
extern int tape_out_block_tap_close(void);
extern int tape_block_tap_save(void *dir,int longitud);
extern void tape_block_tap_begin_save(int longitud,z80_byte flag);





#endif
