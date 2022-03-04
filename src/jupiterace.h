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

#ifndef JUPITERACE_H
#define JUPITERACE_H

#include "cpu.h"

extern int da_amplitud_speaker_ace(void);

extern int amplitud_speaker_actual_ace;

extern z80_bit bit_salida_sonido_ace;

extern void set_ace_ramtop(z80_byte valor);
extern int get_ram_ace(void);


#endif
