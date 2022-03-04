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

#ifndef DS1307_H
#define DS1307_H

#include "cpu.h"

#define DS1307_PORT_CLOCK 0x103b
#define DS1307_PORT_DATA  0x113b

extern z80_byte ds1307_get_port_clock(void);
extern z80_byte ds1307_get_port_data(void);
extern void ds1307_write_port_data(z80_byte value);
extern void ds1307_write_port_clock(z80_byte value);
extern void ds1307_reset(void);

extern void tbblue_trap_return_rtc(void);

#endif
