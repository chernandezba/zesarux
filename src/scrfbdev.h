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

#ifndef SCRFBDEV_H
#define SCRFBDEV_H

#include "cpu.h"

extern int scrfbdev_init (void);
extern void scrfbdev_end(void);
extern void scrfbdev_refresca_pantalla(void);
extern void scrfbdev_refresca_pantalla_solo_driver(void);
extern z80_byte scrfbdev_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrfbdev_actualiza_tablas_teclado(void);
extern void scrfbdev_debug_registers(void);
extern void scrfbdev_messages_debug(char *s);

extern int fbdev_no_uses_tty;
extern int fbdev_no_uses_ttyraw;
extern int fbdev_use_all_virtual_res;
extern int fbdev_decimal_full_scale_fbdev;
extern int fbdev_margin_width;
extern int fbdev_margin_height;



#endif
