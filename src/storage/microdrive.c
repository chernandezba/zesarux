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


#include "microdrive.h"
#include "if1.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "zxvision.h"
#include "compileoptions.h"


z80_byte *if1_microdrive_buffer;


int mdr_total_sectors=0;


char microdrive_file_name[PATH_MAX]="";

z80_bit microdrive_enabled={0};


//int puntero_mdr=0;

//-1 si no aplica
int microdrive_get_visualmem_position(unsigned int address)
{
#ifdef EMULATE_VISUALMEM


    //El buffer de visualmem en este caso tiene mismo tamaño que dispositivo microdrive
    int posicion_final=address;

    //por si acaso
    if (posicion_final>=0 && posicion_final<VISUALMEM_MICRODRIVE_BUFFER_SIZE) {
        return posicion_final;


    }


#endif

	return -1;
}

void microdrive_set_visualmem_read(unsigned int address)
{
#ifdef EMULATE_VISUALMEM
	int posicion_final=microdrive_get_visualmem_position(address);
	if (posicion_final>=0) {
		set_visualmemmicrodrive_read_buffer(posicion_final);
	}

#endif
}

void microdrive_set_visualmem_write(unsigned int address)
{
#ifdef EMULATE_VISUALMEM
	int posicion_final=microdrive_get_visualmem_position(address);
	if (posicion_final>=0) {
		set_visualmemmicrodrive_write_buffer(posicion_final);
	}

#endif
}

int mdr_current_sector=0;
int mdr_current_offset_in_sector=0;

void mdr_next_sector(void)
{

    mdr_current_offset_in_sector=0;
    mdr_current_sector++;
    if (mdr_current_sector>=mdr_total_sectors) mdr_current_sector=0;
}


z80_byte mdr_next_byte(void)
{



    int offset_to_sector=mdr_current_sector*MDR_BYTES_PER_SECTOR;

    int offset_efectivo;


    offset_efectivo=mdr_current_offset_in_sector;


    offset_efectivo +=offset_to_sector;

    z80_byte valor=if1_microdrive_buffer[offset_efectivo];

    microdrive_set_visualmem_read(offset_efectivo);

    printf("Retornando byte mdr de offset en PC=%04XH sector %d, offset %d (offset_efectivo=%d) =0x%02X\n",
        reg_pc,mdr_current_sector,mdr_current_offset_in_sector,offset_efectivo,valor);



    mdr_current_offset_in_sector++;


    if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        mdr_next_sector();

    }



    return valor;

}


void microdrive_insert(void)
{
      //Cargar microdrive de prueba
       if1_microdrive_buffer=malloc(MDR_MAX_FILE_SIZE);
       if (if1_microdrive_buffer==NULL) {
                cpu_panic ("No enough memory for Microdrive buffer");
       }

       FILE *ptr_microdrive_file;
       //Leer archivo mdr
        ptr_microdrive_file=fopen(microdrive_file_name,"rb");

       if (ptr_microdrive_file==NULL) {
               debug_printf (VERBOSE_ERR,"Cannot locate %s",microdrive_file_name);
       }

       else {
               //Leer todo el archivo microdrive de prueba
               int leidos=fread(if1_microdrive_buffer,1,MDR_MAX_FILE_SIZE,ptr_microdrive_file);
               printf ("leidos %d bytes de microdrive\n",leidos);

                mdr_total_sectors=leidos/MDR_BYTES_PER_SECTOR;

               fclose(ptr_microdrive_file);

               microdrive_enabled.v=1;
       }
}