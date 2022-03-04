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
#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif



#include "divmmc.h"
#include "divide.h"
#include "diviface.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"



//z80_bit divmmc_enabled={0};

z80_bit divmmc_mmc_ports_enabled={0};
z80_bit divmmc_diviface_enabled={0};

#define DIVMMC_DEFAULT_ROM_NAME "esxmmc085.rom"

//Nombre de la rom. Si "", nombre y ruta por defecto
char divmmc_rom_name[PATH_MAX]="";



void divmmc_mmc_ports_enable(void)
{

  //Si ya esta habilitado, salir
  if (divmmc_mmc_ports_enabled.v) return;

	if (!MACHINE_IS_SPECTRUM) {
		debug_printf(VERBOSE_INFO,"Can not enable divmmc ports on non Spectrum machine");
		return;
	}
	debug_printf (VERBOSE_INFO,"Enabling divmmc mmc ports");
	divmmc_mmc_ports_enabled.v=1;
}

void divmmc_mmc_ports_disable(void)
{
	debug_printf (VERBOSE_INFO,"Disabling divmmc mmc ports");
        divmmc_mmc_ports_enabled.v=0;
}



//Activar solo la parte de paginacion cargando la rom de divmmc
void divmmc_diviface_enable(void)
{

	//printf ("%d\n",divmmc_diviface_enabled.v);
  //Si ya esta habilitado, salir
  if (divmmc_diviface_enabled.v) {
  	debug_printf(VERBOSE_DEBUG,"Divmmc already enabled");
  	return;
  }

	if (!MACHINE_IS_SPECTRUM) {
		debug_printf(VERBOSE_INFO,"Can not enable divmmc on non Spectrum machine");
		return;
	}

	debug_printf (VERBOSE_INFO,"Enabling divmmc diviface ports");
	//Es excluyente con firmware de divide
	divide_diviface_disable();

	if (divmmc_rom_name[0]!=0) diviface_enable(divmmc_rom_name); //TODO: aunque esto acaba llamando a open_sharedfile y en casos de rutas relativas puede dar problemas
	else {
		if (MACHINE_IS_CHLOE) {
			debug_printf(VERBOSE_INFO,"Loading divmmc chloe firmware unodos3.rom");
			diviface_enable("unodos3.rom");
		}
		else {
			diviface_enable(DIVMMC_DEFAULT_ROM_NAME);
		}
	}

	if (diviface_enabled.v) divmmc_diviface_enabled.v=1;
}

void divmmc_diviface_disable(void)
{
	debug_printf (VERBOSE_INFO,"Disabling divmmc diviface paging");
	diviface_disable();
	divmmc_diviface_enabled.v=0;
}
