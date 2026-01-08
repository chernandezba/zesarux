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

#ifndef TV_H
#define TV_H

#include "cpu.h"

//VSYNC pulse tiene que ser al menos 400us
//VSYNC dura 6.25 lineas:
//64 us-> una linea -> 207 t_estados
//400 us-> 6.25 lineas -> 1293 t_estados
//tomamos 1100 t_estados como buenos

//#define MINIMO_DURACION_VSYNC 1100

//Normal. Para manic miner, minimo 800
//#define DEFAULT_MINIMO_DURACION_VSYNC 800

/*Nuevo valor estandard, en teoria son
A true VSync basically has a pulse of 2.5 scan lines (2.5 * 64us = 160us = 517.5 T-states at 3.25MHz).
Either side are scan lines containing pre and post equalizing pulses, but these can be ignored.
A VSync of 160us worked for my analogue TV using the RF connection. Since the ZX81 generates the pulse in software,
it can produce any length VSync it wants. It is then a matter of whether the TV is tolerant enough to accept it.
*/

//Con 518 ok. Pero HERO.81 requiere 160
#define DEFAULT_MINIMO_DURACION_VSYNC 160
#define PERMITIDO_MINIMO_DURACION_VSYNC 100
#define PERMITIDO_MAXIMO_DURACION_VSYNC 2000

extern void tv_time_event(int delta);

extern void tv_enable_hsync(void);
extern void tv_disable_hsync(void);
extern void tv_enable_vsync(void);
extern void tv_disable_vsync(void);

extern int tv_get_x(void);
extern int tv_get_y(void);
extern int tv_get_time(void);
extern int tv_get_vsync_signal(void);
extern int tv_get_hsync_signal(void);


extern int tv_max_line_period;
extern int tv_max_lines;

#endif
