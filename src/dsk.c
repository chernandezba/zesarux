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

/*

DSK emulation

*/


#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "dsk.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "tape.h"
#include "menu_items.h"
#include "screen.h"


char dskplusthree_file_name[PATH_MAX]="";

z80_bit dskplusthree_write_protection={0};

int dskplusthree_must_flush_to_disk=0;


//Si cambios en escritura se hace flush a disco
z80_bit dskplusthree_persistent_writes={1};


z80_byte p3dsk_buffer_disco[DSK_MAX_BUFFER_DISCO];
int p3dsk_buffer_disco_size=DSK_MAX_BUFFER_DISCO; //Tamanyo del dsk leido. De momento establecemos en maximo

void dsk_insert_disk(char *nombre)
{
                strcpy(dskplusthree_file_name,nombre);
                
    if (noautoload.v==0) {
		reset_cpu();
	}
}


z80_bit dskplusthree_emulation={0};


void dskplusthree_flush_contents_to_disk(void)
{

        if (dskplusthree_emulation.v==0) return;

        if (dskplusthree_must_flush_to_disk==0) {
                debug_printf (VERBOSE_DEBUG,"Trying to flush DSK to disk but no changes made");
                return;
        }


        if (dskplusthree_persistent_writes.v==0) {
                debug_printf (VERBOSE_DEBUG,"Trying to flush DSK to disk but persistent writes disabled");
                return;
        }



        debug_printf (VERBOSE_INFO,"Flushing DSK to disk");


        FILE *ptr_dskplusthreefile;

        debug_printf (VERBOSE_INFO,"Opening DSK File %s",dskplusthree_file_name);
        ptr_dskplusthreefile=fopen(dskplusthree_file_name,"wb");



        int escritos=0;
        long long int size;
        size=p3dsk_buffer_disco_size;


        if (ptr_dskplusthreefile!=NULL) {
                z80_byte *puntero;
                puntero=p3dsk_buffer_disco; 

                //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
                //metera flush a 1
                dskplusthree_must_flush_to_disk=0;

                escritos=fwrite(puntero,1,size,ptr_dskplusthreefile);

                fclose(ptr_dskplusthreefile);


        }

        //debug_printf(VERBOSE_DEBUG,"ptr_dskplusthreefile: %d",ptr_dskplusthreefile);
        //debug_printf(VERBOSE_DEBUG,"escritos: %d",escritos);

        if (escritos!=size || ptr_dskplusthreefile==NULL) {
                debug_printf (VERBOSE_ERR,"Error writing to DSK file");
        }

}




void dskplusthree_disable(void)
{

	if (dskplusthree_emulation.v==0) return;

	debug_printf (VERBOSE_INFO,"Disabling DSK emulation");

	dskplusthree_emulation.v=0;
}

void dskplusthree_enable(void)
{

	if (dskplusthree_emulation.v) return;

	debug_printf (VERBOSE_INFO,"Enabling DSK emulation");
	debug_printf (VERBOSE_INFO,"Opening DSK File %s",dskplusthree_file_name);

	long long int tamanyo=get_file_size(dskplusthree_file_name);

	if (tamanyo>DSK_MAX_BUFFER_DISCO) {
		debug_printf(VERBOSE_ERR,"DSK size too big");
		return;
	}

        FILE *ptr_dskfile;
        ptr_dskfile=fopen(dskplusthree_file_name,"rb");

        if (!ptr_dskfile) {
                debug_printf(VERBOSE_ERR,"Unable to open disk %s",dskplusthree_file_name);
                return;
        }

        //int leidos=fread(p3dsk_buffer_disco,1,200000,ptr_configfile);
        fread(p3dsk_buffer_disco,1,DSK_MAX_BUFFER_DISCO,ptr_dskfile);


        fclose(ptr_dskfile);

	p3dsk_buffer_disco_size=tamanyo;

        dskplusthree_emulation.v=1;

}


z80_byte plus3dsk_get_byte_disk(int offset)
{

        if (dskplusthree_emulation.v==0) return 0;

        if (offset>=p3dsk_buffer_disco_size) {
                debug_printf (VERBOSE_ERR,"Error. Trying to read beyond dsk. Size: %d Asked: %d. Disabling DSK",p3dsk_buffer_disco_size,offset);
                dskplusthree_disable();
                return 0;
        }


        return p3dsk_buffer_disco[offset];
}

void plus3dsk_put_byte_disk(int offset,z80_byte value)
{
        if (dskplusthree_emulation.v==0) return;

        if (offset>=p3dsk_buffer_disco_size) {
                debug_printf (VERBOSE_ERR,"Error. Trying to read beyond dsk. Size: %d Asked: %d. Disabling DSK",p3dsk_buffer_disco_size,offset);
                dskplusthree_disable();
                return;
        }


	if (dskplusthree_write_protection.v) return;

        p3dsk_buffer_disco[offset]=value;

	dskplusthree_must_flush_to_disk=1;
}


void dsk_show_activity(void)
{
	generic_footertext_print_operating("DISK");

	//Y poner icono en inverso
	if (!zxdesktop_icon_plus3_inverse) {
			zxdesktop_icon_plus3_inverse=1;
			menu_draw_ext_desktop();
	}	
}