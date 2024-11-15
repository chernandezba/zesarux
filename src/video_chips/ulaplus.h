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

#ifndef ULAPLUS_H
#define ULAPLUS_H

#include "cpu.h"


extern void init_ulaplus_table(void);

extern z80_bit ulaplus_presente;

extern z80_bit ulaplus_enabled;

extern z80_byte ulaplus_last_send_BF3B;
extern z80_byte ulaplus_last_send_FF3B;

extern void ulaplus_change_palette_colour(z80_byte index,z80_byte color);

extern void disable_ulaplus(void);
extern void enable_ulaplus(void);

extern z80_byte ulaplus_mode;
extern z80_byte ulaplus_extended_mode;

extern z80_byte ulaplus_palette_table[];
extern int ulaplus_rgb_table[];


extern void ulaplus_set_mode(z80_byte value);

extern void ulaplus_set_extended_mode(z80_byte value);

extern z80_byte ulaplus_return_port_ff3b(void);

extern void ulaplus_write_port(z80_int puerto,z80_byte value);


#endif
