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

#ifndef TAPE_SMP_H
#define TAPE_SMP_H

#include "cpu.h"


extern int tape_block_smp_open(void);
extern int tape_block_smp_read(void *dir,int longitud);
extern int tape_block_smp_readlength(void);
extern int tape_block_smp_seek(int longitud,int direccion);

extern void snap_load_zx80_zx81_load_smp(void);
extern int main_leezx81(char *archivo_destino, char *texto_info_output,int si_load);
extern z80_byte *spec_smp_memory;
extern int tape_guessing_parameters;

extern int main_spec_rwaatap(long *array_block_positions,int max_array_block_positions,int *codigo_retorno,int longitud_archivo_smp);
extern int spec_smp_read_index_tap;
extern int spec_smp_write_index_tap;
extern int spec_smp_total_read;
extern int lee_smp_ya_convertido;

extern FILE *ptr_mycinta_smp;

extern char *main_spec_rwaatap_pointer_print;
extern int main_spec_rwaatap_pointer_print_max;

extern void main_leezx81_init_semaphore(void);

#endif
