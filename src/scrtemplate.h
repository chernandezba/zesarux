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

/*

This is a template for a video driver. It can be used as an example for a graphical video driver,
like xwindows or fbdev. Some other drivers, like curses or cacalib works in a different way

Replace: 
VIDEONAME_CAP with the video driver name in capital letters, like "XWINDOWS"
videoname with the video driver name in lowercase letters, like "xwindows"

*/

#ifndef SCRVIDEONAME_CAP_H
#define SCRVIDEONAME_CAP_H

#include "cpu.h"


extern void scrvideoname_refresca_pantalla(void);
extern void scrvideoname_refresca_pantalla_solo_driver(void);
extern int scrvideoname_init (void);
extern void scrvideoname_end(void);
extern z80_byte scrvideoname_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrvideoname_actualiza_tablas_teclado(void);
extern void scrvideoname_debug_registers(void);
extern void scrvideoname_messages_debug(char *s);


#endif
