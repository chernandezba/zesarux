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

#ifndef MHPOKEADOR_H
#define MHPOKEADOR_H

#include "cpu.h"

#define MHPOKEADOR_ROM_POKEADOR "mhpokeador.rom"
#define MHPOKEADOR_ROM_TRANSFER "mhpokeador-transfer.rom"

//Diferentes firmwares a cargar
#define MHPOKEADOR_TIPO_ROM_POKEADOR 0
#define MHPOKEADOR_TIPO_ROM_TRANSFER 1


#define MHPOKEADOR_RAM_SIZE 2048

#define MHPOKEADOR_ROM_SIZE 1024


extern z80_bit mhpokeador_enabled;


extern void mhpokeador_reset(void);
extern void mhpokeador_enable(void);
extern void mhpokeador_disable(void);
extern void mhpokeador_nmi(void);

extern z80_byte *mhpokeador_memory_pointer;



extern char mhpokeador_rom_filename[];

#endif
