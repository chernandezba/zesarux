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

#ifndef CORE_SPECTRUM_H
#define CORE_SPECTRUM_H

#include "cpu.h"

extern void cpu_core_loop_spectrum(void);
extern z80_byte byte_leido_core_spectrum;
extern void core_spectrum_store_rainbow_current_atributes(void);
extern int si_siguiente_sonido(void);
extern void t_scanline_next_fullborder(void);
extern void interrupcion_si_despues_lda_ir(void);

#endif
