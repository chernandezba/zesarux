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
#include <stdlib.h>

#include "cpu.h"
#include "tsconf.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "menu.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"


//Comun zxevo, tanto para baseconf como tsconf
z80_byte zxevo_nvram[256];

//Control de acceso a celdas nvram
//z80_byte zxevo_last_port_eff7;

//celda nvram seleccionada
z80_byte zxevo_last_port_dff7;

