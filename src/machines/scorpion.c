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
#include <time.h>

#include "cpu.h"
#include "scorpion.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "zxvision.h"
#include "screen.h"
#include "ula.h"
#include "operaciones.h"


/*

RAM 256 KB

| Puerto | Bits | Función |
|---|---|---|
| `7FFDh` | D0–D2 | Seleccionan una página de 0 a 7. |
| `1FFDh` | D4 | Añade 8 al número de página cuando vale 1. |


ROM 64 KB

| Página | Desplazamiento en la ROM | Contenido | Selección |
|---|---:|---|---|
| 0 | `0000h` | BASIC 128 | `1FFDh.D1=0`, `7FFDh.D4=0` |
| 1 | `4000h` | BASIC 48 | `1FFDh.D1=0`, `7FFDh.D4=1` |
| 2 | `8000h` | Monitor de servicio | `1FFDh.D1=1` |
| 3 | `C000h` | TR-DOS | Se activa mediante la lógica de TR-DOS, no mediante la selección BASIC de `7FFDh`. |

*/