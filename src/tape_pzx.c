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
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "tape.h"
#include "tape_pzx.h"
#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "utils.h"
#include "zvfs.h"
#include "settings.h"
#include "snap.h"


//
//Para PZX load
//

//
//Para PZX save
//

FILE *ptr_mycinta_pzx_out;

void tape_write_pzx_header_ptr(FILE *ptr_archivo, int in_fatfs, FIL *fil_pzxfile)
{
	//"PZXT",longitud,longitud,longitud,longitud,
        //version 1,subversion 0,
	//"string" + 0

        char cabecera[256];

        char time_string[40];
        snapshot_get_date_time_string_human(time_string);

        //de momento todo ascii para poder sacar strlen
        //----: reservado para longitud bloque.  00: reservado para version. se modifican luego
        sprintf(cabecera,"PZXT----00Created by ZEsarUX emulator " EMULATOR_VERSION " on %s",time_string);

        //Incluir el 0 del final de string tambien
        int longitud_cabecera=strlen(cabecera)+1;

        //printf ("longitud cabecera: %d\n",longitud_cabecera);

        z80_long_int longitud_bloque=longitud_cabecera-8; //-8 porque saltamos "PZXT" y los 4 bytes que indican la longitud
        //printf ("longitud bloque: %d\n",longitud_bloque);

        //Metemos longitud de bloque
        cabecera[4]=longitud_bloque & 0xFF;
        cabecera[5]=(longitud_bloque>>8) & 0xFF;
        cabecera[6]=(longitud_bloque>>16) & 0xFF;
        cabecera[7]=(longitud_bloque>>24) & 0xFF;

        //Metemos version
        cabecera[8]=PZX_CURRENT_MAJOR_VERSION;
        cabecera[9]=PZX_CURRENT_MINOR_VERSION;   
 
	//fwrite(cabecera, 1, longitud_cabecera, ptr_archivo);
    zvfs_fwrite(in_fatfs,(z80_byte *)cabecera,longitud_cabecera,ptr_archivo,fil_pzxfile);
}

void tape_write_pzx_header(void)
{

	struct stat buf_stat;

              //Escribir cabecera pzx. Pero si el archivo lo reutilizamos, tendra longitud>0, y no debemos reescribir la cabecera

                if (stat(tape_out_file, &buf_stat)!=0) {
			debug_printf(VERBOSE_INFO,"Unable to get status of file %s",tape_out_file);
		}

		else {
			//Tamaño del archivo es >0
			if (buf_stat.st_size!=0) {
				debug_printf(VERBOSE_INFO,"PZX File already has header");
				return;
			}
		}


	debug_printf(VERBOSE_INFO,"Writing PZX header");

	tape_write_pzx_header_ptr(ptr_mycinta_pzx_out,0,NULL); //No usamos descriptor de zvfs

	
}

int tape_out_block_pzx_open(void)
{

        ptr_mycinta_pzx_out=fopen(tape_out_file,"ab");

        if (!ptr_mycinta_pzx_out)
        {
                debug_printf(VERBOSE_ERR,"Unable to open output file %s",tape_out_file);
                tape_out_file=0;
                return 1;
        }

        return 0;

}



int tape_out_block_pzx_close(void)
{
        if (ptr_mycinta_pzx_out) fclose(ptr_mycinta_pzx_out);
	else debug_printf (VERBOSE_ERR,"Tape uninitialized");
        return 0;
}


int tape_block_pzx_save(void *dir,int longitud)
{

	if (ptr_mycinta_pzx_out) return fwrite(dir, 1, longitud, ptr_mycinta_pzx_out);
	else {
		debug_printf (VERBOSE_ERR,"Tape uninitialized");
        	return -1;
	}
}


void tape_block_pzx_begin_save_ptr(FILE *ptr_archivo,int longitud,z80_byte flag,int in_fatfs, FIL *fil_pzxfile)
{




	//Escribir id 10	
	//pausa de 1000 ms
	/*char buffer[]={0x10,232,3};
	fwrite(buffer, 1, sizeof(buffer), ptr_mycinta_pzx_out);*/

        //Meter pulso tono guia
        /*
        offset type     name   meaning
0      u32      tag    unique identifier for the block type.
4      u32      size   size of the block in bytes, excluding the tag and size fields themselves.
8      u8[size] data   arbitrary amount of block data.
        */

       /*
       
       PULS - Pulse sequence
---------------------

offset type   name      meaning
0      u16    count     bits 0-14 optional repeat count (see bit 15), always greater than zero
                        bit 15 repeat count present: 0 not present 1 present
2      u16    duration1 bits 0-14 low/high (see bit 15) pulse duration bits
                        bit 15 duration encoding: 0 duration1 1 ((duration1<<16)+duration2)
4      u16    duration2 optional low bits of pulse duration (see bit 15 of duration1) 
6      ...    ...       ditto repeated until the end of the block

       
       For example, the standard pilot tone of Spectrum header block (leader < 128)
may be represented by following sequence:

0x8000+8063,2168,667,735

The standard pilot tone of Spectrum data block (leader >= 128) would be:

0x8000+3223,2168,667,735
        */

       z80_byte block_buffer[256];
       block_buffer[0]='P';
       block_buffer[1]='U';
       block_buffer[2]='L';
       block_buffer[3]='S';

        //longitud
       block_buffer[4]=8;
       block_buffer[5]=0;
       block_buffer[6]=0;
       block_buffer[7]=0;    




/*
For example, the standard pilot tone of Spectrum header block (leader < 128)
may be represented by following sequence:

0x8000+8063,2168,667,735

The standard pilot tone of Spectrum data block (leader >= 128) would be:

0x8000+3223,2168,667,735
*/

       z80_int longitud_tono_guia=8063;

       if (flag>=128) longitud_tono_guia=3223;


        block_buffer[8]=value_16_to_8l(0x8000+longitud_tono_guia);
        block_buffer[9]=value_16_to_8h(0x8000+longitud_tono_guia);

        block_buffer[10]=value_16_to_8l(2168);
        block_buffer[11]=value_16_to_8h(2168);

        block_buffer[12]=value_16_to_8l(667);
        block_buffer[13]=value_16_to_8h(667);

        block_buffer[14]=value_16_to_8l(735);
        block_buffer[15]=value_16_to_8h(735);                        
	

        //Escribir bloque PULS
        //fwrite(block_buffer, 1, 16, ptr_archivo);
        zvfs_fwrite(in_fatfs,block_buffer, 16, ptr_archivo,fil_pzxfile);



        //Preparar bloque DATA
        /*
        DATA - Data block
-----------------

offset      type             name  meaning
0           u32              count bits 0-30 number of bits in the data stream
                                   bit 31 initial pulse level: 0 low 1 high
4           u16              tail  duration of extra pulse after last bit of the block
6           u8               p0    number of pulses encoding bit equal to 0.
7           u8               p1    number of pulses encoding bit equal to 1.
8           u16[p0]          s0    sequence of pulse durations encoding bit equal to 0.
8+2*p0      u16[p1]          s1    sequence of pulse durations encoding bit equal to 1.
8+2*(p0+p1) u8[ceil(bits/8)] data  data stream, see below.
        */

       /*
       For example, the sequences for standard ZX Spectrum bit encoding are:
(initial pulse level is high):

bit 0: 855,855
bit 1: 1710,1710
        */

       //z80_byte block_buffer[256];
       block_buffer[0]='D';
       block_buffer[1]='A';
       block_buffer[2]='T';
       block_buffer[3]='A';

        //longitud
        z80_long_int longitud_bloque=longitud+16; //estos 16 son desde block_buffer[8] hasta block_buffer[23]
       block_buffer[4]=longitud_bloque & 0xFF;
       block_buffer[5]=(longitud_bloque>>8) & 0xFF;
       block_buffer[6]=(longitud_bloque>>16) & 0xFF;
       block_buffer[7]=(longitud_bloque>>24) & 0xFF;      

       /*
offset      type             name  meaning
0           u32              count bits 0-30 number of bits in the data stream
                                   bit 31 initial pulse level: 0 low 1 high
4           u16              tail  duration of extra pulse after last bit of the block
6           u8               p0    number of pulses encoding bit equal to 0.
7           u8               p1    number of pulses encoding bit equal to 1.
8           u16[p0]          s0    sequence of pulse durations encoding bit equal to 0.
8+2*p0      u16[p1]          s1    sequence of pulse durations encoding bit equal to 1.
8+2*(p0+p1) u8[ceil(bits/8)] data  data stream, see below.       
       */ 

      //z80_long_int numero_bits=longitud_bloque*8;
      z80_long_int numero_bits=longitud*8;
 
       block_buffer[8]=numero_bits & 0xFF;
       block_buffer[9]=(numero_bits>>8) & 0xFF;
       block_buffer[10]=(numero_bits>>16) & 0xFF;
       block_buffer[11]=((numero_bits>>24) & 0x7F) | 128; //estado inicial high   

       z80_int tail=945;

       block_buffer[12]=tail & 0xFF;
       block_buffer[13]=(tail>>8) & 0xFF;       

/*
6           u8               p0    number of pulses encoding bit equal to 0.
7           u8               p1    number of pulses encoding bit equal to 1.
*/
        block_buffer[14]=2;
        block_buffer[15]=2;

/*
bit 0: 855,855
bit 1: 1710,1710
*/
        block_buffer[16]=value_16_to_8l(855);
        block_buffer[17]=value_16_to_8h(855);
        block_buffer[18]=value_16_to_8l(855);
        block_buffer[19]=value_16_to_8h(855);       

        block_buffer[20]=value_16_to_8l(1710);
        block_buffer[21]=value_16_to_8h(1710);
        block_buffer[22]=value_16_to_8l(1710);
        block_buffer[23]=value_16_to_8h(1710);   

        //Escribir bloque DATA
        //fwrite(block_buffer, 1, 24, ptr_archivo);        
        zvfs_fwrite(in_fatfs,block_buffer, 24, ptr_archivo,fil_pzxfile);

        //Y a partir de aqui ya vienen los datos, que los escribe desde tape_block_pzx_save     

}


void tape_block_pzx_begin_save(int longitud,z80_byte flag)
{
       
    //Escribir cabecera pzx si conviene
    tape_write_pzx_header();
	

    tape_block_pzx_begin_save_ptr(ptr_mycinta_pzx_out,longitud,flag,0,NULL); //No usamos descriptores de zvfs
}
