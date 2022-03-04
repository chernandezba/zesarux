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

#ifndef CHARDEVICE_H
#define CHARDEVICE_H

#include "cpu.h"

enum chardevice_openmode {
	CHDEV_RDONLY,
	CHDEV_WRONLY,
	CHDEV_RDWR
};

#define	CHDEV_ST_RD_AVAIL_DATA 1


extern int chardevice_open(char *path,enum chardevice_openmode mode);
extern int chardevice_read(int handler,z80_byte *buffer);
extern int chardevice_write(int handler,z80_byte valor_escribir);
extern int chardevice_close(int handler);
extern int chardevice_status(int handler);


enum chardevice_speed {
    CHDEV_SPEED_DEFAULT,
    CHDEV_SPEED_9600,
    CHDEV_SPEED_19200,
    CHDEV_SPEED_38400,
    CHDEV_SPEED_57600,
    CHDEV_SPEED_115200
};

extern void chardevice_setspeed(int handler,enum chardevice_speed velocidad);
extern int chardevice_getspeed_enum_int(enum chardevice_speed velocidad);

#endif
