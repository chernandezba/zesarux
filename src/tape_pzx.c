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
//TODO: Es un poco chapuza el codigo, asume que los bloques DATA tienen correctamente indicado la longitud en bits,
//por lo que al leer todos los datos que supuestamente hay ahi, ya nos ubicamos en el siguiente bloque
//esto no tiene por que ser asi y fallar en algun pzx corrupto?
//Este codigo esta basado en parte en el de TZX, el cual ya era un poco chapuza, y no estoy contento con el resultado pero... funciona
//


FILE *ptr_mycintanew_pzx;
z80_int last_length_read;

char pzx_last_block_id_name[5]=""; //Inicializado a cadena vacia

z80_long_int pzx_last_blockmem_length;

//memoria del bloque pzx leido
z80_byte *pzx_blockmem_pointer=NULL;

//posicion del siguiente byte a leer
z80_long_int pzx_blockmem_position;

int pzx_blockmem_feof;

void tape_block_pzx_blockmem_free(void)
{
    if (pzx_blockmem_pointer!=NULL) {
        free(pzx_blockmem_pointer);
        pzx_blockmem_pointer=NULL;
        pzx_last_blockmem_length=0;
    }
}

//Retorna un byte del bloque de memoria
z80_byte tape_block_pzx_blockmem_get(z80_long_int offset)
{
    if (offset<0 || offset>pzx_last_blockmem_length) {
        debug_printf(VERBOSE_ERR,"Trying to read beyond pzx block");
        return 0;
    }

    else return pzx_blockmem_pointer[offset];
}

//Retorna el siguiente byte en el bloque
z80_byte tape_block_pzx_blockmem_getnext(void)
{
    return tape_block_pzx_blockmem_get(pzx_blockmem_position++);
}

//Lee una cantidad de bytes desde la posicion actual
int tape_block_pzx_blockmem_fread(z80_byte *puntero_memoria,int leer)
{
    //TODO: se deberia retornar cantidad leida efectiva
    int leer_orig=leer;

    while (leer>0) {
        *puntero_memoria=tape_block_pzx_blockmem_getnext();
        puntero_memoria++;
    } 

    return leer_orig;
}

int tape_block_pzx_open(void)
{

    ptr_mycintanew_pzx=fopen(tapefile,"rb");

    if (!ptr_mycintanew_pzx)
    {
        debug_printf(VERBOSE_ERR,"Unable to open input file %s",tapefile);
        tapefile=0;
        return 1;
    }

    pzx_blockmem_feof=0;

    return 0;

}

void tape_block_pzx_rewindbegin(void)
{
    fseek(ptr_mycintanew_pzx,0, SEEK_SET);

    tape_block_pzx_blockmem_free();
}

int pzx_read_id(void)
{
    //Lee el id de bloque  (los 4 bytes) acabando con 0, y lee la longitud del bloque

    int leidos=fread(pzx_last_block_id_name,1,4,ptr_mycintanew_pzx);
    if (!leidos) {
        debug_printf(VERBOSE_DEBUG,"End of PZX file");
        pzx_blockmem_feof=1;
        return 0;
    }
    pzx_last_block_id_name[4]=0;

    z80_byte buffer_longitud[4];
    leidos=fread(buffer_longitud,1,4,ptr_mycintanew_pzx);
    if (!leidos) {
        debug_printf(VERBOSE_DEBUG,"End of PZX file");
        pzx_blockmem_feof=1;
        return 0;
    }

    pzx_last_blockmem_length=   buffer_longitud[0]+
                            (buffer_longitud[1]*256)+
                            (buffer_longitud[2]*65536)+
                            (buffer_longitud[3]*16777216);

    //printf("pzx read id. name: %02XH %02XH %02XH %02XH\n",
    //    pzx_last_block_id_name[0],pzx_last_block_id_name[1],pzx_last_block_id_name[2],pzx_last_block_id_name[3]);

    //pzx_begin_of_id=1;

    //Leer el contenido del bloque en memoria
    tape_block_pzx_blockmem_free();

    pzx_blockmem_pointer=util_malloc(pzx_last_blockmem_length,"Can not allocate memory for PZX read");
    leidos=fread(pzx_blockmem_pointer,1,pzx_last_blockmem_length,ptr_mycintanew_pzx);
    if (!leidos) {
        debug_printf(VERBOSE_DEBUG,"End of PZX file");
        pzx_blockmem_feof=1;
        return 0;
    }
      

    return 1;

}

void pzx_jump_block(void)
{

    //esto ya no hace nada?

    //fseek(ptr_mycinta_pzx,pzx_last_block_id_length,SEEK_CUR);

    //pzx_last_block_id_name[0]=0;
}




int tape_block_pzx_read(void *dir,int longitud)
{

	//pzx_read_returned_unknown_id.v=0;
    //Asumimos que estamos en un bloque DATA

	/*if (!ptr_mycinta_pzx) {
		debug_printf (VERBOSE_ERR,"Tape uninitialized");
		return 0;
	}
    */




				
    debug_printf(VERBOSE_DEBUG,"Reading %d bytes.",longitud);
    int leidos=tape_block_pzx_blockmem_fread(dir,longitud);
    if (leidos==0) {
        //if (!quickload_guessing_tzx_type.v) debug_printf(VERBOSE_INFO,"Error reading TZX tape");
        return 0;
    }

    /*printf("bloque leido:\n");
    int i;
    for (i=0;i<longitud;i++) {
        z80_byte caracter=((char *)dir)[i];
        if (caracter>=32 && caracter<=126) printf("%c",caracter);
        else printf(" %02XH ",caracter);
    }
    printf("\n");*/

    last_length_read -=longitud;
    debug_printf(VERBOSE_DEBUG,"Remaining bytes in block: %d",last_length_read);
    

    return leidos;



}


int tape_pzx_seek_data_block(void)
{

	//pzx_read_returned_unknown_id.v=0;

	/*if (!ptr_mycinta_pzx) {
		debug_printf (VERBOSE_ERR,"Tape uninitialized");
		return;
	}*/

	do {
        //sleep(1);
        //if (tape_block_pzx_feof()) return;

        if (!pzx_read_id()) return 0;

        debug_printf(VERBOSE_INFO,"PZX Read Block type: [%s]",pzx_last_block_id_name);
        
        if (!strcasecmp(pzx_last_block_id_name,"DATA")) {
            return 1;
        }

        else if (!strcasecmp(pzx_last_block_id_name,"PZXT")) {
            //Si es texto, mostrarlo como info de debug

            //Leer mayor y minor
            z80_byte buffer_version[2];
            tape_block_pzx_blockmem_fread(buffer_version,2);
            //pzx_last_block_id_length -=2;

            debug_printf(VERBOSE_INFO,"PZX PZXT block. Version %d.%d",buffer_version[0],buffer_version[1]);
		           
            //asigna memoria
            z80_byte *info_bloque;
            info_bloque=util_malloc(pzx_last_blockmem_length,"Can not allocate for PZXT block");

            //Copiar el contenido
            tape_block_pzx_blockmem_fread(info_bloque,pzx_last_blockmem_length-2);

            //Mostrar las cadenas de texto cada una separadas por espacio
            int longitud_textos=pzx_last_blockmem_length-2;

            //Puntero que vamos moviendo a cada string
            z80_byte *puntero_strings;

            puntero_strings=info_bloque;

            while (longitud_textos>0) {
                debug_printf(VERBOSE_INFO,"PZX PZXT block. Tape info: %s",puntero_strings);
                int longitud_string=strlen((char *)puntero_strings);

                puntero_strings +=longitud_string+1; //saltar incluso el 0 final

                //Y restar de la longitud total
                longitud_textos -=(longitud_string+1); 
            }

            free(info_bloque);


            //Y no se llama a pzx_jump_block porque estamos justo en el siguiente bloque
        }

        else {
            //Cualquier otra cosa ignorarla
            pzx_jump_block();
        } 
    } while(1);   
}

int tape_pzx_see_if_standard_tape(void)
{

    //TODO
    return 1;
    /*

	if (!ptr_mycinta_pzx) {
		debug_printf (VERBOSE_ERR,"Tape uninitialized");
		return 1;
	}

	do {
        if (tape_block_pzx_feof()) return 1;

        if (!pzx_read_id()) return 1;

        //printf("tape_pzx_see_if_standard_tape. Bloque %s\n",pzx_last_block_id_name);
        
        if (!strcasecmp(pzx_last_block_id_name,"DATA")) {

            

            
            //saltar los 6 bytes que indican longitud en bits y la duracion del tail
            char buffer_nada[6];
            fread(buffer_nada,1,6,ptr_mycinta_pzx);

            //Bloque datos standard para spectrum
            //
            //6           u8               p0    number of pulses encoding bit equal to 0.
            //7           u8               p1    number of pulses encoding bit equal to 1.
            //8           u16[p0]          s0    sequence of pulse durations encoding bit equal to 0.
            //8+2*p0      u16[p1]          s1    sequence of pulse durations encoding bit equal to 1.

            //bit 0: 855,855
            //bit 1: 1710,1710
            
            z80_byte pzx_data_block[10]={0x02,0x02,0x57,0x03,0x57,0x03,0xae,0x06,0xae,0x06};


            z80_byte buffer_data[10];

		    fread(buffer_data,1,10,ptr_mycinta_pzx);

            

            if (memcmp(pzx_data_block,buffer_data,10)) return 0;

            pzx_last_block_id_length -=(6+10);


            pzx_jump_block();
        }

        else if (!strcasecmp(pzx_last_block_id_name,"PULS")) {

            //TODO restar de pzx_last_block_id_length bytes leidos    

            //Si longitud no es 8, seguro que no es un pulso estandard
            if (pzx_last_block_id_length!=8) return 0;  

            //Tono guia de un flag <128 (largo)
            z80_byte pzx_pulses_flag_long[8]= {0x7f,0x9f,0x78,0x08,0x9b,0x02,0xdf,0x02};

            //Tono guia de un flag >=128 (corto)
            z80_byte pzx_pulses_flag_short[8]={0x97,0x8c,0x78,0x08,0x9b,0x02,0xdf,0x02};

            //Comparar los 8 bytes siguientes con los dos tipos de pulsos habituales
            //TODO: si el flag no es 0 o 255, aun asi podria ser una carga estandard, pero directamente
            //decimos que no lo es

            z80_byte buffer_pulsos[8];

		    fread(buffer_pulsos,1,8,ptr_mycinta_pzx);

            int pulso_valido=0;

            //printf("Pulso a comparar: ");
            //int i;
            //for (i=0;i<8;i++) printf("%02XH ",buffer_pulsos[i]);

            //printf("\n");

            if (!memcmp(pzx_pulses_flag_long,buffer_pulsos,8)) pulso_valido=1;

            if (!pulso_valido) {
                if (!memcmp(pzx_pulses_flag_short,buffer_pulsos,8)) pulso_valido=1;
            }

            if (!pulso_valido) return 0;

            pzx_last_block_id_length -=8;

            pzx_jump_block();
        }        

        else {
            //Cualquier otra cosa ignorarla
            pzx_jump_block();
        } 
    } while(1); 
    */      
}

int tape_block_pzx_readlength(void)
{

	/*if (!ptr_mycinta_pzx) {
        	debug_printf (VERBOSE_ERR,"Tape uninitialized");
	        return 0;
	}*/

    debug_printf(VERBOSE_DEBUG,"PZX Read length");
    //if (last_id_read!=0xFF) pzx_read_id();


    if (!tape_pzx_seek_data_block()) {
        //printf("End of tape\n");
        return 0;
    }

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

        //leemos longitud
        z80_byte buffer_longitud[4];
        tape_block_pzx_blockmem_fread(buffer_longitud,4);

        int longitud_bits=   buffer_longitud[0]+
                                (buffer_longitud[1]*256)+
                                (buffer_longitud[2]*65536)+
                                ((buffer_longitud[3]&127)*16777216);      

        //printf("longitud bits: %d\n",longitud_bits);
		last_length_read=longitud_bits/8;


        //De momento saltar secuencias de pulsos
        z80_byte buffer_nada[2];

		tape_block_pzx_blockmem_fread(buffer_nada,2);

        z80_byte pulsos_cero,pulsos_uno;
        tape_block_pzx_blockmem_fread(&pulsos_cero,1);
        tape_block_pzx_blockmem_fread(&pulsos_uno,1);

        //printf("Pulsos cero: %d Pulsos uno: %d\n",pulsos_cero,pulsos_uno);

        for(;pulsos_cero;pulsos_cero--) tape_block_pzx_blockmem_fread(buffer_nada,2);
        for(;pulsos_uno;pulsos_uno--) tape_block_pzx_blockmem_fread(buffer_nada,2);


        //printf("PZX Data Block length: %d\n",last_length_read);

		debug_printf(VERBOSE_DEBUG,"PZX Data Block length: %d",last_length_read);
		//sleep(2);
		return last_length_read;
}



int tape_block_pzx_seek(int longitud,int direccion)
{

    /*if (!ptr_mycinta_pzx) {
            debug_printf (VERBOSE_ERR,"Tape uninitialized");
            return -1;
    }*/

    if (direccion!=SEEK_CUR) {
        debug_printf(VERBOSE_ERR,"PZX block seek direction %d not supported",direccion);
        return -1;
    }

   
    debug_printf(VERBOSE_DEBUG,"PZX Seek %d bytes",longitud);

    pzx_blockmem_position +=longitud;
    //ret=fseek(ptr_mycinta_pzx,longitud,direccion);
    last_length_read -=longitud;
    //if (last_length_read==0) pzx_read_id();

    return 0;
}

int tape_block_pzx_feof(void)
{
    return pzx_blockmem_feof=1;
}


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
