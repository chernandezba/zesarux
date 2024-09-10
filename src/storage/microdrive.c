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
#include "menu_items.h"
#include "screen.h"


z80_byte *if1_microdrive_buffer;


int mdr_total_sectors=0;


char microdrive_file_name[PATH_MAX]="";

z80_bit microdrive_enabled={0};

z80_bit microdrive_write_protect={0};


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

    printf("siguiente sector. actual=%d\n",mdr_current_sector);
}


z80_byte mdr_next_byte(void)
{

    if (microdrive_enabled.v==0) return 0;

    if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        //Si estamos al final del sector, devolver 0
        //Esto no es lo real en el hardware, pero nos sirve
        //Realmente pasaremos al siguiente sector cuando se lea del puerto ef y hayamos pasado el tiempo final
        return 0;
    }


    int offset_to_sector=mdr_current_sector*MDR_BYTES_PER_SECTOR;

    int offset_efectivo;


    offset_efectivo=mdr_current_offset_in_sector;


    offset_efectivo +=offset_to_sector;

    z80_byte valor=if1_microdrive_buffer[offset_efectivo];

    microdrive_set_visualmem_read(offset_efectivo);

    printf("Retornando byte mdr de offset en PC=%04XH sector %d, offset %d (offset_efectivo=%d) =0x%02X\n",
        reg_pc,mdr_current_sector,mdr_current_offset_in_sector,offset_efectivo,valor);



    mdr_current_offset_in_sector++;


    /*
    if (mdr_current_offset_in_sector>=MDR_BYTES_PER_SECTOR) {
        mdr_next_sector();
    }
    */



    return valor;

}


void microdrive_insert(void)
{
      //Cargar microdrive de prueba
       if1_microdrive_buffer=util_malloc(MDR_MAX_FILE_SIZE,"No enough memory for Microdrive buffer");


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

               //leer byte de write protect

               microdrive_write_protect.v=0;

               if (if1_microdrive_buffer[leidos-1]) microdrive_write_protect.v=1;
       }
}

void microdrive_eject(void)
{
	if (microdrive_enabled.v==0) return;

	//Hacer flush si hay algun cambio
	//microdrive_flush_contents_to_disk();


	free(if1_microdrive_buffer);


	microdrive_enabled.v=0;
}


void microdrive_footer_operating(void)
{

    generic_footertext_print_operating("MDR");

    //Y poner icono en inverso
    if (!zxdesktop_icon_mdv1_inverse) {
        zxdesktop_icon_mdv1_inverse=1;
        menu_draw_ext_desktop();
    }
}

//Contador simple para saber si tenemos que devolver gap, sync o datos
int contador_estado_microdrive=0;



z80_byte microdrive_status_ef(void)
{
        //printf ("In Port %x asked, PC after=0x%x\n",puerto_l+256*puerto_h,reg_pc);

        /*
        Microdrive cartridge
        GAP      PREAMBLE      15 byte      GAP      PREAMBLE      15 byte    512     1
        [-----][00 00 ... ff ff][BLOCK HEAD][-----][00 00 ... ff ff][REC HEAD][ DATA ][CHK]
        Preamble = 10 * 0x00 + 2 * 0xff (12 byte)
        */


        contador_estado_microdrive++;



        z80_byte return_value=0;

        //numero arbitrario realmente, cada cuanto incrementamos el contador para pasar de un estado al otro
        //La rom del interface1 por ejemplo cuando está leyendo datos (puerto e7) no está leyendo el puerto de estado (ef)
        //por tanto ese incremento del estado de datos (valor 0) a paso a estado gap lo producimos cuando ha pasado el contadort
        //aunque en dispositivo real esto sucederia justo al dejar de enviar los 543 bytes
        //Logicamente esto no va a la velocidad real ni cuento t-estados ni nada, por ejemplo si lees del puerto
        //de datos, te llegará el siguiente byte, y su vuelves a leer, aunque no haya pasado el tiempo "real" del microdrive
        //para que llegue el siguiente byte, te llegará
        #define MICRODRIVE_PASOS_CAMBIO_ESTADO 20

        if      (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO)   return_value=MICRODRIVE_STATUS_BIT_GAP; //gap
        else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*2) return_value=MICRODRIVE_STATUS_BIT_SYNC; //sync
        else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*3) return_value=0; //datos

        else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*4) return_value=MICRODRIVE_STATUS_BIT_GAP; //gap
        else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*5) return_value=MICRODRIVE_STATUS_BIT_SYNC; //sync
        else if (contador_estado_microdrive<MICRODRIVE_PASOS_CAMBIO_ESTADO*6) return_value=0; //datos

        if (contador_estado_microdrive>=MICRODRIVE_PASOS_CAMBIO_ESTADO*6) {
            mdr_next_sector();
            contador_estado_microdrive=0;
        }

        printf ("In Port ef asked, PC after=0x%x contador_estado_microdrive=%d return_value=0x%x\n",reg_pc,contador_estado_microdrive,return_value);


        if (microdrive_write_protect.v==0) return_value |=MICRODRIVE_STATUS_BIT_NOT_WRITE_PROTECT;

        interface1_last_read_status_ef=return_value;

        return return_value;

}