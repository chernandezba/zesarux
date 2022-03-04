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

#ifndef SCRCURSES_H
#define SCRCURSES_H

#include "cpu.h"

extern int scrcurses_init (void);
extern void scrcurses_end(void);
extern void scrcurses_refresca_pantalla(void);
extern void scrcurses_refresca_pantalla_solo_driver(void);
extern z80_byte scrcurses_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrcurses_actualiza_tablas_teclado(void);
extern void scrcurses_debug_registers(void);
extern void scrcurses_messages_debug(char *s);
extern void scrcurses_inicializa_colores(void);

extern void scrcurses_z88_draw_lower_screen(void);
extern int scrcurses_return_gunstick_view_white(void);
extern void scrcurses_fade_color(int color);


#endif
