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

#ifndef ENHANCED_ZX81_READ_H
#define ENHANCED_ZX81_READ_H



typedef unsigned char z80_byte;

#define ENHANCED_GLOBAL_INFO_LAST_BYTES_LENGTH 10

extern int enh_zx81_lee_datos(z80_byte *enhanced_memoria,int tamanyo_memoria,z80_byte *destino_p81,
    z80_byte amplitud_media, int debug_print,int *longitud_nombre,void (*fun_print)(char *),int *cancel_process);
extern z80_byte return_zx81_char(z80_byte codigo);

struct s_enh_zx81_lee_global_info {
    int enh_global_input_position;
    int enh_global_output_position;
    z80_byte enh_global_last_byte_read;
    z80_byte enh_global_partial_byte_read;
    int enh_global_last_bit_read;
    int enh_global_bit_position_in_byte;
    int enh_global_pulses_of_a_bit;
    z80_byte enh_global_last_bytes[ENHANCED_GLOBAL_INFO_LAST_BYTES_LENGTH];
};

extern void enh_zx81_lee_get_global_info(struct s_enh_zx81_lee_global_info *i);

#endif
