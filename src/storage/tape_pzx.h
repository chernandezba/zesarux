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

#ifndef TAPE_PZX_H
#define TAPE_PZX_H

#include "cpu.h"
#include "zvfs.h"

extern int tape_block_pzx_open(void);
extern int tape_block_pzx_read(void *dir,int longitud);
extern int tape_block_pzx_readlength(void);
extern int tape_block_pzx_seek(int longitud,int direccion);
extern int tape_block_pzx_feof(void);
extern void tape_block_pzx_rewindbegin(void);

extern int tape_out_block_pzx_open(void);
extern int tape_out_block_pzx_close(void);
extern int tape_block_pzx_save(void *dir,int longitud);
extern void tape_block_pzx_begin_save(int longitud,z80_byte flag);
extern void tape_write_pzx_header_ptr(FILE *ptr_archivo, int in_fatfs, FIL *fil_tzxfile);
extern void tape_block_pzx_begin_save_ptr(FILE *ptr_archivo,int longitud,z80_byte flag,int in_fatfs, FIL *fil_tzxfile);
extern int tape_pzx_see_if_standard_tape(void);


#endif
