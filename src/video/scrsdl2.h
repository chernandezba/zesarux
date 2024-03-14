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

#ifndef SCRSDL2_H
#define SCRSDL2_H

#include "cpu.h"


extern void scrsdl_refresca_pantalla(void);
extern void scrsdl_refresca_pantalla_solo_driver(void);
extern int scrsdl_init (void);
extern void scrsdl_end(void);
extern z80_byte scrsdl_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrsdl_actualiza_tablas_teclado(void);
extern void scrsdl_debug_registers(void);
extern void scrsdl_messages_debug(char *s);


#endif
