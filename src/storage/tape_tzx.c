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
#include "tape_tzx.h"
#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "utils.h"
#include "zvfs.h"
#include "settings.h"


/*
void (*tape_block_open)(void);
void (*tape_block_read)(void *dir,int longitud);
void (*tape_block_seek)(int longitud,int direccion);
*/

FILE *ptr_mycinta_tzx;
FILE *ptr_mycinta_tzx_out;

z80_byte last_id_read=0xFF;
z80_int last_length_read;
z80_int begin_of_id=0;


//Establecer a 1 siempre que se inserte cinta nueva
//int tzx_save_no_header_yet=1;
//esto realmente no hace falta. Para saber si hay que escribir cabecera, lo vemos por tamaño de archivo



int tape_block_tzx_open(void)
{

		ptr_mycinta_tzx=fopen(tapefile,"rb");

                if (!ptr_mycinta_tzx)
                {
                        debug_printf(VERBOSE_ERR,"Unable to open tzx input file %s",tapefile);
			tapefile=0;
                        return 1;
                }

		last_id_read=0xFF;

		return 0;

}

void tape_block_tzx_rewindbegin(void)
{
    fseek(ptr_mycinta_tzx,0, SEEK_SET);

    last_id_read=0xFF;
}

int tape_block_tzx_read_header(void)
{

	z80_byte tzx_major,tzx_minor;
	char buffer_signature[8];
	fread(buffer_signature,1,7,ptr_mycinta_tzx);
	buffer_signature[7]=0;

	debug_printf(VERBOSE_INFO,"TZX Signature: %s",buffer_signature);
	if (strcmp(buffer_signature,"ZXTape!")!=0) {
		debug_printf(VERBOSE_ERR,"Unknown TZX Signature");
		return 1;
	}

	//saltamos 0x1A
	fread(buffer_signature,1,1,ptr_mycinta_tzx);

	//leemos major,minor
	fread(&tzx_major,1,1,ptr_mycinta_tzx);
	fread(&tzx_minor,1,1,ptr_mycinta_tzx);
	debug_printf(VERBOSE_INFO,"TZX Version major:%d minor:%d",tzx_major,tzx_minor);


	return 0;
}

void tzx_read_id(void)
{
	fread(&last_id_read,1,1,ptr_mycinta_tzx);
	debug_printf(VERBOSE_DEBUG,"Read ID 0x%x",last_id_read);
	begin_of_id=1;
}

int last_tzx_pause=0;

//Dice que la operacion de read volvio con unknown id. Usado en quickload para detectar si la cinta
//tzx se reconoce toda o tiene tags desconocidos
z80_bit tzx_read_returned_unknown_id;


void tape_tzx_get_archive_info(z80_byte type_text,char *buffer_text_description)
{
	switch (type_text) {
							case 0:
								sprintf(buffer_text_description,"Full title");
							break;

							case 1:
								sprintf(buffer_text_description,"Software house/publisher");
							break;

							case 2:
								sprintf(buffer_text_description,"Author(s)");
							break;

							case 3:
								sprintf(buffer_text_description,"Year of publication");
							break;

							case 4:
								sprintf(buffer_text_description,"Language");
							break;

							case 5:
								sprintf(buffer_text_description,"Game/utility type");
							break;

							case 6:
								sprintf(buffer_text_description,"Price");
							break;

							case 7:
								sprintf(buffer_text_description,"Protection scheme/loader");
							break;

							case 8:
								sprintf(buffer_text_description,"Origin");
							break;

							case 0xFF:
								sprintf(buffer_text_description,"Comment(s)");
							break;



							default:
								sprintf(buffer_text_description,"unknown");
							break;
						}
}

int tape_block_tzx_read(void *dir,int longitud)
{

	tzx_read_returned_unknown_id.v=0;

	if (!ptr_mycinta_tzx) {
		debug_printf (VERBOSE_ERR,"Tape uninitialized");
		return 0;
	}

	do {

		switch (last_id_read) {

			case 0xFF:
				debug_printf(VERBOSE_DEBUG,"Last TZX ID not initialized. Begin Tape");
				if (tape_block_tzx_read_header()!=0) return 0;

				tzx_read_id();
			break;

			case 0x10:
				if (begin_of_id==1) {
					debug_printf(VERBOSE_DEBUG,"Begin of TZX id 0x10");
					//leemos pausa
					z80_byte read_buffer[2];

					fread(read_buffer,1,2,ptr_mycinta_tzx);
					//int tzx_pausa=value_8_to_16(read_buffer[1],read_buffer[0]);
					last_tzx_pause=value_8_to_16(read_buffer[1],read_buffer[0]);

                    if (last_tzx_pause>0 && tzx_suppress_pause.v) last_tzx_pause=0;

					debug_printf(VERBOSE_DEBUG,"TZX Pause Readed: %d ms",last_tzx_pause);

					begin_of_id=0;
				}
				else {
					debug_printf(VERBOSE_DEBUG,"Last TZX ID was 0x10");
					debug_printf(VERBOSE_DEBUG,"Reading %d bytes.",longitud);
					int leidos=fread(dir,1,longitud,ptr_mycinta_tzx);
					if (leidos==0) {
						if (!quickload_guessing_tzx_type.v) debug_printf(VERBOSE_INFO,"Error reading TZX tape");
						return 0;
					}


					//Para debug
					/*
					z80_byte byte_leido=*((z80_byte *)dir);
					printf ("Byte leido: %d\n",byte_leido);
					if (longitud>1) {
                                        byte_leido=*((z80_byte *)(dir+1));
                                        printf ("Byte leido+1: %d\n",byte_leido);
					}
					*/

					last_length_read -=longitud;
					debug_printf(VERBOSE_DEBUG,"Remaining bytes in block: %d",last_length_read);
					if (last_length_read==0) tzx_read_id();
					//tzx_read_id();


					if (last_tzx_pause!=0) {
						//calculamos en fracciones de 1/50s
						tape_pause=last_tzx_pause/20;

						//maximo 2 segundos
						if (tape_pause>100) tape_pause=100;
						//Pause
						debug_printf(VERBOSE_INFO,"Read TZX Pause of: %d ms, but making %d ms (max 2000 ms)",last_tzx_pause,tape_pause*20);
					}


					return leidos;
				}
			break;

			case 0x30:
				if (begin_of_id==1) {
					debug_printf(VERBOSE_DEBUG,"Begin of TZX id 0x30");
					//leemos longitud
					z80_byte tzx_text_longitud;
					fread(&tzx_text_longitud,1,1,ptr_mycinta_tzx);
					debug_printf(VERBOSE_DEBUG,"TZX Block length: %d",tzx_text_longitud);
					z80_byte read_buffer[256];
					fread(read_buffer,1,tzx_text_longitud,ptr_mycinta_tzx);
					read_buffer[tzx_text_longitud]=0;
					debug_printf(VERBOSE_INFO,"TZX Comment: %s",read_buffer);
					tzx_read_id();
				}
			break;

			case 0x32:
				debug_printf(VERBOSE_DEBUG,"Begin of TZX id 0x32");
                                        z80_byte read_buffer[2];

                                        fread(read_buffer,1,2,ptr_mycinta_tzx);


                                        //int tzx_archive_length=value_8_to_16(read_buffer[1],read_buffer[0]);
					//de momento lo saltamos
					//fseek(ptr_mycinta_tzx,tzx_archive_length,SEEK_CUR);

					//int text_strings;
					z80_byte text_strings;
					fread(&text_strings,1,1,ptr_mycinta_tzx);
					z80_byte type_text,long_text;
					char buffer_text[256];
					char buffer_text_description[100];

					for (;text_strings;text_strings--) {
						fread(&type_text,1,1,ptr_mycinta_tzx);
						tape_tzx_get_archive_info(type_text,buffer_text_description);

						//leemos long
						fread(&long_text,1,1,ptr_mycinta_tzx);
						//texto longitud
						debug_printf(VERBOSE_DEBUG,"TZX Text length: %d",long_text);
						//leemos texto
						fread(buffer_text,1,long_text,ptr_mycinta_tzx);
						buffer_text[long_text]=0;
						debug_printf(VERBOSE_INFO,"TZX Text id %d : %s : %s",type_text,buffer_text_description,buffer_text);

					}
				tzx_read_id();



			break;

            case 0x33:
                //ID 33 - Hardware type
                //0x00	N	BYTE	Number of machines and hardware types for which info is supplied
                //0x01	-	HWINFO[N]	List of machines and hardware
                //length: [00]*03+01

                //Lo saltamos tal cual
                debug_printf(VERBOSE_DEBUG,"Begin of TZX id 0x33");

                //leemos longitud
                z80_byte tzx_text_longitud;
                fread(&tzx_text_longitud,1,1,ptr_mycinta_tzx);

                int longitud_total=tzx_text_longitud*3;

                //saltar todo eso
                z80_byte byte_leido;

                for (;longitud_total>0;longitud_total--) {
                    fread(&byte_leido,1,1,ptr_mycinta_tzx);
                }

                tzx_read_id();


            break;

			default:
				if (!quickload_guessing_tzx_type.v) debug_printf(VERBOSE_ERR,"TZX ID 0x%x not supported on standard tape",last_id_read);
				else debug_printf (VERBOSE_INFO,"TZX ID 0x%x not supported on standard tape",last_id_read);
				tzx_read_returned_unknown_id.v=1;
				return 0;

			break;
		}

	} while (1);


}

int tape_block_tzx_readlength(void)
{

	if (!ptr_mycinta_tzx) {
        	debug_printf (VERBOSE_ERR,"Tape uninitialized");
	        return 0;
	}

		debug_printf(VERBOSE_DEBUG,"TZX Read length");
		//if (last_id_read!=0xFF) tzx_read_id();

		z80_byte buffer[2];
		last_length_read=65535;
		if (tape_block_tzx_read(buffer,2)==0) return 0;

		last_length_read=value_8_to_16(buffer[1],buffer[0]);

		debug_printf(VERBOSE_DEBUG,"TZX Block length: %d",last_length_read);
	//	sleep(1);
		return last_length_read;
}



int tape_block_tzx_seek(int longitud,int direccion)
{

if (!ptr_mycinta_tzx) {
        debug_printf (VERBOSE_ERR,"Tape uninitialized");
        return -1;
}

int ret;
debug_printf(VERBOSE_DEBUG,"TZX Seek %d bytes",longitud);
ret=fseek(ptr_mycinta_tzx,longitud,direccion);
last_length_read -=longitud;
if (last_length_read==0) tzx_read_id();

return ret;
}

int tape_block_tzx_feof(void)
{
    return feof(ptr_mycinta_tzx);
}


int tape_out_block_tzx_open(void)
{

                ptr_mycinta_tzx_out=fopen(tape_out_file,"ab");

                if (!ptr_mycinta_tzx_out)
                {
                        debug_printf(VERBOSE_ERR,"Unable to open output file %s",tape_out_file);
                        tape_out_file=0;
                        return 1;
                }


		//tzx_save_no_header_yet=1

                return 0;

}


int tape_out_block_tzx_close(void)
{
        if (ptr_mycinta_tzx_out) fclose(ptr_mycinta_tzx_out);
	else debug_printf (VERBOSE_ERR,"Tape uninitialized");
        return 0;
}

void tape_write_tzx_header_ptr(FILE *ptr_archivo, int in_fatfs, FIL *fil_tzxfile)
{
	//"ZXTape!",0x1a,version 1,subversion 20
	char cabecera[]={0x5a,0x58,0x54, 0x61, 0x70, 0x65, 0x21, 0x1a, 0x01, 0x14};
    zvfs_fwrite(in_fatfs,(z80_byte *)cabecera,sizeof(cabecera),ptr_archivo,fil_tzxfile);



    char time_string[40];
    snapshot_get_date_time_string_human(time_string);
    char texto_description[256];
    sprintf(texto_description,"Created by ZEsarUX emulator " EMULATOR_VERSION " on %s",time_string);

    //ID 30 - Text description
    z80_byte block_id=0x30;
    zvfs_fwrite(in_fatfs,(z80_byte *)&block_id,1,ptr_archivo,fil_tzxfile);
    z80_byte longitud_texto=strlen(texto_description);
    zvfs_fwrite(in_fatfs,(z80_byte *)&longitud_texto,1,ptr_archivo,fil_tzxfile);

    zvfs_fwrite(in_fatfs,(z80_byte *)texto_description,longitud_texto,ptr_archivo,fil_tzxfile);
}

void tape_write_tzx_header(void)
{
	struct stat buf_stat;

              //Escribir cabecera tzx. Pero si el archivo lo reutilizamos, tendra longitud>0, y no debemos reescribir la cabecera

                if (stat(tape_out_file, &buf_stat)!=0) {
			debug_printf(VERBOSE_INFO,"Unable to get status of file %s",tape_out_file);
		}

		else {
			//Tamaño del archivo es >0
			if (buf_stat.st_size!=0) {
				debug_printf(VERBOSE_INFO,"TZX File already has header");
				return;
			}
		}


	debug_printf(VERBOSE_INFO,"Writing TZX header");

	//tape_write_tzx_header_ptr(ptr_mycinta_tzx_out);

    //Como esto no se grabara en volumen montado FatFS, puntero a NULL
    tape_write_tzx_header_ptr(ptr_mycinta_tzx_out, 0 , NULL);

	/*

	//"ZXTape!",0x1a,version 1,subversion 20
	//30, longitud, "Created by ZEsarUX emulator"
	char cabecera[]={0x5a,0x58,0x54, 0x61, 0x70, 0x65, 0x21, 0x1a, 0x01, 0x14,
	0x30,27,
	0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,0x62,0x79,0x20,0x5a,0x45,0x73,
        0x61,0x72,0x55,0x58,0x20,0x65,0x6d,0x75,0x6c,0x61,0x74,0x6f,0x72
	};

	//no contamos el 0 del final
	fwrite(cabecera, 1, sizeof(cabecera), ptr_mycinta_tzx_out);
	*/
}


void tape_block_tzx_begin_save(int longitud GCC_UNUSED,z80_byte flag GCC_UNUSED)
{

	tape_write_tzx_header();


	//Escribir id 10
	//pausa de 1000 ms
	char buffer[]={0x10,232,3};
	fwrite(buffer, 1, sizeof(buffer), ptr_mycinta_tzx_out);

}



int tape_block_tzx_save(void *dir,int longitud)
{
	//debug_printf (VERBOSE_INFO,"Writing %d bytes",longitud);
	//if (longitud) return fwrite(dir, 1, longitud, ptr_mycinta_tzx_out);
	//else return 0;
	if (ptr_mycinta_tzx_out) return fwrite(dir, 1, longitud, ptr_mycinta_tzx_out);
	else {
		debug_printf (VERBOSE_ERR,"Tape uninitialized");
        	return -1;
	}
}



