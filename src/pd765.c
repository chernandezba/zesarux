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
#include <dirent.h>
#include <unistd.h>
#include <string.h>


#include "pd765.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "dsk.h"

z80_bit pd765_enabled={0};



z80_bit pd765_enabled;
void pd765_enable(void)
{

}

void pd765_disable(void)
{

}

void pd765_motor_on(void)
{

}
void pd765_motor_off(void)
{

}
void pd765_write_command(z80_byte value)
{

}
z80_byte pd765_read_command(void)
{
    return 255;
}
z80_byte pd765_read_status_register(void)
{
    return 255;
}

void pd765_out_port_1ffd(z80_byte value)
{
    //0x1ffd: Setting bit 3 high will turn the drive motor (or motors, if you have more than one drive attached) on. Setting bit 3 low will turn them off again. (0x1ffd is also used for memory control).

    if (value&8) pd765_motor_on();
    else pd765_motor_off();
 
}

void pd765_out_port_3ffd(z80_byte value)
{

    //Puertos disco +3
    pd765_write_command(value);

}

