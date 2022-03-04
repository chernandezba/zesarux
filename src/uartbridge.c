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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>



#include "uartbridge.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "chardevice.h"
#include "screen.h"


//Nombre de la ruta al dispositivo uart bridge
//char uartbridge_name[PATH_MAX]="";
//temp
char uartbridge_name[PATH_MAX]="/dev/ttyUSB0";

//Si esta habilitado uart bridge
z80_bit uartbridge_enabled={0};

//file handler del dispositivo uart bridge. -1 si no esta abierto
int uartbridge_handler=-1;

enum chardevice_speed uartbridge_speed=CHDEV_SPEED_DEFAULT;

int uartbridge_available(void)
{
	//No dispositivo abierto
	if (uartbridge_enabled.v==0 || uartbridge_handler<0) return 0;
	else return 1;
}

void uartbridge_enable(void)
{

	debug_printf(VERBOSE_DEBUG,"Opening uart bridge");

	if (uartbridge_enabled.v) return;

	uartbridge_handler=chardevice_open(uartbridge_name,CHDEV_RDWR);

	if (uartbridge_handler>=0) {
		uartbridge_enabled.v=1;
        if (uartbridge_speed!=CHDEV_SPEED_DEFAULT) chardevice_setspeed(uartbridge_handler,uartbridge_speed);
	}
	else {
		debug_printf (VERBOSE_ERR,"Error opening uart bridge %s",uartbridge_name);
		uartbridge_enabled.v=0;
	}
}

void uartbridge_disable(void)
{
	if (uartbridge_enabled.v==0) return;	

	debug_printf(VERBOSE_DEBUG,"Closing uart bridge");

	if (chardevice_close(uartbridge_handler)<0) {
		debug_printf (VERBOSE_ERR,"Error closing uart bridge");		
	}

	uartbridge_enabled.v=0;
}


z80_byte uartbridge_readdata(void)
{


	//No dispositivo abierto
	if (!uartbridge_available()) return 0;

    generic_footertext_print_operating("UART");

	//printf ("Reading uart data\n");

	//Devolver 0 por defecto si error leyendo
	z80_byte byte_leido=0;

	//printf ("Before chardevice_read\n");
	int status=chardevice_read(uartbridge_handler,&byte_leido);
	//printf ("After chardevice_read\n");

	if (status<1) {
		//printf ("Error reading uart data: %d\n",status);
		//Error leyendo
		//de momento no decir nada
	}

	return byte_leido;
}


void uartbridge_writedata(z80_byte value)
{

	//No dispositivo abierto
	if (!uartbridge_available()) return;

    generic_footertext_print_operating("UART");

	//printf ("Writing uart data\n");

	int status=chardevice_write(uartbridge_handler,value);

	if (status<1) {
		//Error escribiendo
		//de momento no decir nada
	}	

}

