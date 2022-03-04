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



#include "divide.h"
#include "divmmc.h"
#include "diviface.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"

//z80_bit divide_enabled={0};

#define DIVIDE_DEFAULT_ROM_NAME "esxide085.rom"


//Nombre de la rom. Si "", nombre y ruta por defecto
char divide_rom_name[PATH_MAX]="";



z80_bit divide_ide_ports_enabled={0};
z80_bit divide_diviface_enabled={0};



void divide_ide_ports_enable(void)
{

  //Si ya esta habilitado, salir
  if (divide_ide_ports_enabled.v) return;

  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable divide ports on non Spectrum machine");
    return;
  }
        debug_printf (VERBOSE_INFO,"Enabling divide ide ports");
        divide_ide_ports_enabled.v=1;
}

void divide_ide_ports_disable(void)
{
        debug_printf (VERBOSE_INFO,"Disabling divide ide ports");
        divide_ide_ports_enabled.v=0;
}



//Activar solo la parte de paginacion cargando la rom de divide
void divide_diviface_enable(void)
{
  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable divide on non Spectrum machine");
    return;
  }

  //Si ya esta habilitado, salir
  if (divide_diviface_enabled.v) {
    debug_printf(VERBOSE_DEBUG,"Divide already enabled");
    return;
  }

  debug_printf (VERBOSE_INFO,"Enabling divide diviface ports");
  //Es excluyente con firmware de divmmc
  divmmc_diviface_disable();

  if (divide_rom_name[0]!=0) diviface_enable(divide_rom_name); //TODO: aunque esto acaba llamando a open_sharedfile y en casos de rutas relativas puede dar problemas
  else diviface_enable(DIVIDE_DEFAULT_ROM_NAME);

  if (diviface_enabled.v) divide_diviface_enabled.v=1;
}

void divide_diviface_disable(void)
{
        debug_printf (VERBOSE_INFO,"Disabling divide diviface paging");
        diviface_disable();
        divide_diviface_enabled.v=0;
}
