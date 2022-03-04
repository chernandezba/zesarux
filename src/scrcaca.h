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

#ifndef SCRCACA_H
#define SCRCACA_H

#include "cpu.h"

extern int scrcaca_init (void);
extern void scrcaca_end(void);
extern void scrcaca_refresca_pantalla(void);
extern void scrcaca_refresca_pantalla_solo_driver(void);
extern z80_byte scrcaca_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrcaca_actualiza_tablas_teclado(void);
extern void scrcaca_debug_registers(void);
extern int scrcaca_fast;
extern void scrcaca_messages_debug(char *s);


#endif
