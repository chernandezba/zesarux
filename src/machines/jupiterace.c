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

#include <stdio.h>
#include <string.h>

#include "jupiterace.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "audio.h"
#include "core_ace.h"
#include "operaciones.h"
#include "zxvision.h"

int amplitud_speaker_actual_ace=AMPLITUD_BEEPER;

z80_bit bit_salida_sonido_ace;

int da_amplitud_speaker_ace(void)
{
                                if (bit_salida_sonido_ace.v) return amplitud_speaker_actual_ace;
                                else return -amplitud_speaker_actual_ace;
}


//Establece tamanyo ram Valor entre 3 y 51
void set_ace_ramtop(z80_byte valor)
{
    if (valor<3 || valor>51) {
            cpu_panic("Cannot set ACE RAM");
    }

    ramtop_ace=16383+1024*(valor-3);

    //printf("ramtop ace: %d\n",ramtop_ace);
}

int get_ram_ace(void)
{
  return ((ramtop_ace-16383)/1024)+3;
}
