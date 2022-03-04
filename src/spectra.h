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

#ifndef SPECTRA_H
#define SPECTRA_H

#include "cpu.h"

extern z80_bit spectra_enabled;
extern void spectra_enable(void);
extern void spectra_disable(void);
extern z80_byte spectra_read(void);
extern void spectra_write(z80_byte value);
extern z80_byte *spectra_ram;
extern void spectra_set_poke(void);
extern z80_byte spectra_display_mode_register;


#endif
