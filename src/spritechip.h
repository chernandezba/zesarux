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

#ifndef SPRITECHIP_H
#define SPRITECHIP_H

#include "cpu.h"

#define SPRITECHIP_COMMAND_PORT 0x03F1
#define SPRITECHIP_DATA_PORT 0x04F1

extern z80_bit spritechip_enabled;

extern void spritechip_enable(void);
extern void spritechip_disable(void);

extern void spritechip_write(z80_int port, z80_byte value);
extern z80_byte spritechip_read(z80_int port);

extern void spritechip_do_overlay(void);

extern void sprite_chip_scroll_horizontal_izq(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno,z80_byte cuantos_pixeles);
extern void sprite_chip_scroll_horizontal_der(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno,z80_byte cuantos_pixeles);
extern void sprite_chip_scroll_vertical_arr(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno,z80_byte cuantos_pixeles);
extern void sprite_chip_scroll_vertical_aba(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno,z80_byte cuantos_pixeles);

extern void sprite_chip_scroll_vertical_arr_rotar(z80_int x,z80_int y,z80_int ancho,z80_int alto,z80_byte bit_relleno,z80_byte cuantos_pixeles);
extern void sprite_chip_scroll_vertical_arr_rotar_metebuffer(z80_int x,z80_int y,z80_int ancho,z80_int alto,z80_byte bit_relleno,z80_byte cuantos_pixeles);

extern void spritechip_do_scroll(void);


#endif
