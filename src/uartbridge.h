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

#ifndef UARTBRIDGE_H
#define UARTBRIDGE_H

#include "cpu.h"
#include "chardevice.h"

extern int uartbridge_handler;
extern z80_bit uartbridge_enabled;
extern char uartbridge_name[];
extern void uartbridge_enable(void);
extern void uartbridge_disable(void);

extern z80_byte uartbridge_readdata(void);
extern void uartbridge_writedata(z80_byte value);
extern int uartbridge_available(void);

extern enum chardevice_speed uartbridge_speed;

#endif
