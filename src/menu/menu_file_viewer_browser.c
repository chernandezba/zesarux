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
   Funciones de visor y browser de tipos de archivos
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>


#include "menu_file_viewer_browser.h"
#include "zxvision.h"
#include "menu_items.h"
#include "compileoptions.h"
#include "screen.h"
#include "cpu.h"
#include "debug.h"
#include "settings.h"
#include "zvfs.h"
#include "snap.h"
#include "tape_tzx.h"
#include "msx.h"
#include "snap_spg.h"
#include "snap_zsf.h"
#include "hilow_datadrive.h"
#include "zx8081.h"
#include "microdrive.h"

#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif





//Devuelve 1 si caracter es imprimible en pantalla (entre 32 y 126 o caracteres 10, 13, y algun otro)
int menu_file_viewer_read_text_file_char_print(z80_byte caracter)
{
	if (caracter>=32 && caracter<=126) return 1;

	if (caracter>=9 && caracter<=13) return 1; //9=horiz tab, 10=LF, 11=vert tab, 12=new page, 13=CR

	return 0;

}

void menu_file_viewer_sped_show(char *file_read_memory,int longitud)
{


        int index_buffer;

        char *results_buffer=util_malloc_max_texto_generic_message("Can not allocate memory for sped file show");

        index_buffer=0;

        int salir=0;

		int x=0;

		int lineas=0;

		while (!salir && longitud) {
			z80_byte caracter=*file_read_memory;
			file_read_memory++;

			if (caracter==13) {
				caracter=10;
				lineas++;
			}

			else if (caracter==127) {
				//(C)
				caracter='c';
			}

			else if (caracter>=128) {
				caracter -=128;
				int tabcolumn;
				if (x<7) tabcolumn=7;
				else tabcolumn=12;

				while (x<tabcolumn) {
					results_buffer[index_buffer++]=' ';
					x++;
				}

			}

			results_buffer[index_buffer++]=caracter;
			//controlar maximo
            //100 bytes de margen
            if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-100 || lineas>=MAX_LINEAS_TOTAL_GENERIC_MESSAGE) {
            	debug_printf (VERBOSE_ERR,"Too many lines to show. Showing only the first %d",lineas);
                //forzar salir
                salir=1;
            }

            x++;
            if (caracter==10) x=0;

			longitud--;
		}

        results_buffer[index_buffer]=0;

        menu_generic_message("SPED file",results_buffer);

        free(results_buffer);

}

void menu_file_viewer_read_text_file(char *title,char *file_name)
{

    //printf("inicio menu_file_viewer_read_text_file\n");






	debug_printf (VERBOSE_INFO,"Loading %s File",file_name);

    //printf("cargando txt file: %s\n",file_name);

	FILE *ptr_file_name;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("antes zvfs_fopen_read\n");

    if (zvfs_fopen_read(file_name,&in_fatfs,&ptr_file_name,&fil)<0) {
        debug_printf (VERBOSE_ERR,"Unable to open file");
        return;
    }

    //printf("despues zvfs_fopen_read\n");

    /*
    =util_path_is_prefix_mmc_fatfs(file_name);
    printf("txt esta en fatfs: %d\n",in_fatfs);

    if (in_fatfs) {
        fr = f_open(&fil, file_name, FA_READ);
        if (fr!=FR_OK)
        {
            debug_printf (VERBOSE_ERR,"Unable to open %s file",file_name);
            return;
        }

        //Esto solo para que no se queje el compilador al llamar a zvfs_fread
        ptr_file_name=NULL;
    }

    else {
	    ptr_file_name=fopen(file_name,"rb");



        if (!ptr_file_name)
        {
            debug_printf (VERBOSE_ERR,"Unable to open %s file",file_name);
            return;
        }
    }

    */

	int leidos;

    //printf("antes zvfs_fread\n");

    char *file_read_memory=util_malloc_max_texto_generic_message("Can not allocate memory for reading the file");

    leidos=zvfs_fread(in_fatfs,(z80_byte *)file_read_memory,MAX_TEXTO_GENERIC_MESSAGE,ptr_file_name,&fil);

    //printf("despues zvfs_fread\n");
/*
    if (in_fatfs) {
        UINT leidos_fatfs;
        FRESULT resultado=f_read(&fil,file_read_memory,MAX_TEXTO_GENERIC_MESSAGE,&leidos_fatfs);
        leidos=leidos_fatfs;
    }

    else {
        leidos=fread(file_read_memory,1,MAX_TEXTO_GENERIC_MESSAGE,ptr_file_name);
    }
*/


	debug_printf (VERBOSE_INFO,"Read %d bytes of file: %s",leidos,file_name);
    //printf ("Read %d bytes of file: %s\n",leidos,file_name);
	int avisolimite=0;

	if (leidos==MAX_TEXTO_GENERIC_MESSAGE) {
		avisolimite=1;
		leidos--;
	}

	file_read_memory[leidos]=0;

    zvfs_fclose(in_fatfs,ptr_file_name,&fil);

    /*
    if (in_fatfs) {
        f_close(&fil);
    }
    else {
	    fclose(ptr_file_name);
    }
    */

   //printf("antes deteccion 17\n");
	//Si longitud de bloque es 17, y byte inicial es 0,1,2 o 3, visor de cabecera de bloque de spectrum
	z80_byte byte_inicial=file_read_memory[0];
	if (leidos==17 && byte_inicial<4) {
		debug_printf(VERBOSE_DEBUG,"File 17 bytes length and first byte is <4: assume Spectrum tape header");

		//int longitud_bloque;
		char buffer_texto[60];

        	z80_byte flag=0;
		z80_int longitud=19;

                util_tape_get_info_tapeblock((z80_byte *)file_read_memory,flag,longitud,buffer_texto);



		zxvision_generic_message_tooltip("Tape browser" , 0 , 0, 0, 1, NULL, 1, "%s", buffer_texto);

        free(file_read_memory);

		return;
	}

    //printf("despues deteccion 17\n");


	//Ahora deducir si el archivo cargado es texto o binario.
	//Codigos mayores de 127 o menores de 32 (sin ser el 10, 13 y algun otro) hacen disparar el aviso. Cuantos tiene que haber? Por porcentaje del archivo o por numero?
	//Mejor por porcentaje. Cualquier archivo con un 10% minimo de codigos no imprimibles, se considerara binario
	int i;
	int codigos_no_imprimibles=0;
	z80_byte caracter;

	/* Detector archivos sped:
	-que hayan códigos 13 de salto de línea
	-que no hayan otros códigos menores que 32 excepto los saltos de línea
	-que haya ascii
	-que haya códigos mayores que 128
	*/

	int sped_cr=0;
	int sped_below_32=0;
	int sped_ascii=0;
	int sped_beyond_128=0;

	for (i=0;i<leidos;i++) {
		caracter=file_read_memory[i];
		//if (caracter>127) codigos_no_imprimibles++;
		if (!menu_file_viewer_read_text_file_char_print(caracter)) codigos_no_imprimibles++;

		//Deteccion sped
		if (caracter<32) {
			if (caracter==13) sped_cr=1;
			else sped_below_32=1;
		}

		if (caracter>=32 && caracter<=127) sped_ascii=1;
		if (caracter>127) sped_beyond_128=1;

	}

	//Deteccion sped
	if (sped_cr && !sped_below_32 && sped_ascii && sped_beyond_128) {
		debug_printf(VERBOSE_INFO,"File possibly is a sped file");
		menu_file_viewer_sped_show(file_read_memory,leidos);
        free(file_read_memory);
		return;

	}

	//Sacar porcentaje 10%
	int umbral=leidos/10;

	if (codigos_no_imprimibles>umbral) {
		debug_printf(VERBOSE_INFO,"Considering file as hexadecimal because the invalid characters are higher than 10%% of the total size (%d/%d)",
			codigos_no_imprimibles,leidos);
		menu_file_hexdump_browser_show(file_name);
	}

	else {
		debug_printf(VERBOSE_INFO,"Considering file as text because the invalid characters are lower than 10%% of the total size (%d/%d)",
			codigos_no_imprimibles,leidos);

		if (avisolimite) debug_printf (VERBOSE_ERR,"Read max text buffer: %d bytes. Showing only these",leidos);

		menu_generic_message(title,file_read_memory);
	}

	free(file_read_memory);

}





void menu_file_sp_browser_show(char *filename)
{

	//Leemos cabecera archivo sp
        FILE *ptr_file_sp_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_sp_browser,&fil)<0) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
    }

    /*
        ptr_file_sp_browser=fopen(filename,"rb");

        if (!ptr_file_sp_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
    */

	//Leer 38 bytes de la cabecera
	z80_byte sp_header[38];

        int leidos;

        leidos=zvfs_fread(in_fatfs,sp_header,38,ptr_file_sp_browser,&fil);

        //leidos=fread(sp_header,1,38,ptr_file_sp_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_sp_browser,&fil);
        //fclose(ptr_file_sp_browser);

        //Testear cabecera "SP" en los primeros bytes
        if (sp_header[0]!='S' || sp_header[1]!='P') {
        	debug_printf(VERBOSE_ERR,"Invalid .SP file");
        	return;
        }

	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

    char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

	sprintf(buffer_texto,"Machine: Spectrum 48k");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_int registro_leido;

    registro_leido=value_8_to_16(sp_header[31],sp_header[30]);
    sprintf(buffer_texto,"PC Register: %04XH",registro_leido);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    registro_leido=value_8_to_16(sp_header[29],sp_header[28]);
    sprintf(buffer_texto,"SP Register: %04XH",registro_leido);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte im_leido=sp_header[36] & 2;
	if (im_leido==1) im_leido=2;
    sprintf(buffer_texto,"IM mode: %d",im_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte i_leido=sp_header[27];
    sprintf(buffer_texto,"I register: %02XH",i_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte ints_leido=sp_header[36] &1;
    sprintf(buffer_texto,"Interrupts: %s", (ints_leido ? "Enabled" : "Disabled"));
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("SP file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("SP file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


    free(texto_browser);

}



void menu_file_zx_browser_show(char *filename)
{

	//Leemos cabecera archivo zx
        FILE *ptr_file_zx_browser;


        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_zx_browser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return;
        }

        /*
        ptr_file_zx_browser=fopen(filename,"rb");

        if (!ptr_file_zx_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
    */

	//Leer 201 bytes de la cabecera
	z80_byte zx_header[201];

        int leidos;

        leidos=zvfs_fread(in_fatfs,zx_header,201,ptr_file_zx_browser,&fil);
        //leidos=fread(zx_header,1,201,ptr_file_zx_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_zx_browser,&fil);
        //fclose(ptr_file_zx_browser);

        //Testear cabecera "ZX" en los primeros bytes
        if (zx_header[0]!='Z' || zx_header[1]!='X') {
        	debug_printf(VERBOSE_ERR,"Invalid .ZX file");
        	return;
        }

	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;


     //printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);
	z80_byte zx_version=zx_header[38];
	sprintf(buffer_texto,"ZX File version: %d",zx_version);
	//longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
	//sprintf (&texto_browser[indice_buffer],"%s\n",buffer_texto);
 	//indice_buffer +=longitud_texto+1;

 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	if (zx_version>=3) {
 		z80_byte zx_machine=zx_header[71];
 		if (zx_machine<=24) {
 			sprintf(buffer_texto,"Machine: %s",zxfile_machines_id[zx_machine]);
 			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
 		}
 	}


 	if (zx_version>=4) {
		sprintf(buffer_texto,"Saved on: %d/%02d/%02d %02d:%02d ",
			value_8_to_16(zx_header[78],zx_header[77]),zx_header[76],zx_header[75],zx_header[79],zx_header[80]);

		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
        }


        z80_int zx_pc_reg=value_8_to_16(zx_header[31],zx_header[30]);
        sprintf(buffer_texto,"PC Register: %04XH",zx_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_int registro_leido;
    registro_leido=value_8_to_16(zx_header[29],zx_header[28]);
    sprintf(buffer_texto,"SP Register: %04XH",registro_leido);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte im_leido=zx_header[36] & 2;
        if (im_leido==1) im_leido=2;
    sprintf(buffer_texto,"IM mode: %d",im_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte i_leido=zx_header[27];
    sprintf(buffer_texto,"I register: %02XH",i_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte ints_leido=zx_header[36] &1;
    sprintf(buffer_texto,"Interrupts: %s", (ints_leido ? "Enabled" : "Disabled"));
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("ZEsarUX ZX file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("ZEsarUX ZX file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


    free(texto_browser);

}



void menu_file_zsf_browser_show(char *filename)
{


	long long int bytes_to_load=get_file_size(filename);

	z80_byte *zsf_file_memory;
	zsf_file_memory=malloc(bytes_to_load);
	if (zsf_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	//Leemos archivo zsf
        FILE *ptr_file_zsf_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("menu_file_p_browser_show %s\n",filename);

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_zsf_browser,&fil)<0) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(zsf_file_memory);
    }

    /*

        ptr_file_zsf_browser=fopen(filename,"rb");

        if (!ptr_file_zsf_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(zsf_file_memory);
		return;
	}
    */

        int leidos;

        leidos=zvfs_fread(in_fatfs,zsf_file_memory,bytes_to_load,ptr_file_zsf_browser,&fil);
        //leidos=fread(zsf_file_memory,1,bytes_to_load,ptr_file_zsf_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_zsf_browser,&fil);
        //fclose(ptr_file_zsf_browser);





	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;




 	sprintf(buffer_texto,"ZSF ZEsarUX Snapshot");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


int indice_zsf=0;


//Verificar que la cabecera inicial coincide
  //zsf_magic_header

  char buffer_magic_header[256];

  int longitud_magic=strlen(zsf_magic_header);


  if (leidos<longitud_magic) {
    debug_printf(VERBOSE_ERR,"Invalid ZSF file, small magic header");
    free(texto_browser);
    return;
  }

  //Comparar texto
  memcpy(buffer_magic_header,zsf_file_memory,longitud_magic);
  buffer_magic_header[longitud_magic]=0;

  if (strcmp(buffer_magic_header,zsf_magic_header)) {
    debug_printf(VERBOSE_ERR,"Invalid ZSF file, invalid magic header");
    free(texto_browser);
    return;
  }

  indice_zsf+=longitud_magic;
  bytes_to_load -=longitud_magic;


/*
Format ZSF:
* All numbers are LSB

Every block is defined with a header:

2 bytes - 16 bit: block ID
4 bytes - 32 bit: block Lenght
After these 6 bytes, the data for the block comes.
*/


	while (bytes_to_load>0) {
		    z80_int block_id;
    			block_id=value_8_to_16(zsf_file_memory[indice_zsf+1],zsf_file_memory[indice_zsf+0]);
    			unsigned int block_lenght=zsf_file_memory[indice_zsf+2]+(zsf_file_memory[indice_zsf+3]*256)+(zsf_file_memory[indice_zsf+4]*65536)+(zsf_file_memory[indice_zsf+5]*16777216);

    			debug_printf (VERBOSE_INFO,"Block id: %u Size: %u",block_id,block_lenght);

    			sprintf(buffer_texto,"Id: %2u (%s) Size: %u",block_id,zsf_get_block_id_name(block_id),block_lenght);
    			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    			bytes_to_load -=6;
    			bytes_to_load -=block_lenght;

				//Saltar la cabecera del bloque
    			indice_zsf +=6;

				//Mas info si el campo es de fecha
				if (block_id==ZSF_DATETIME) {
    				sprintf(buffer_texto,"        %d/%02d/%02d %02d:%02d",
						value_8_to_16(zsf_file_memory[indice_zsf+3],zsf_file_memory[indice_zsf+2]),zsf_file_memory[indice_zsf+1],
						zsf_file_memory[indice_zsf+0],zsf_file_memory[indice_zsf+4],zsf_file_memory[indice_zsf+5]);

    				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
				}

				//Mas info si el campo es de creator
				if (block_id==ZSF_CREATOR) {
    				sprintf(buffer_texto,"        %s",&zsf_file_memory[indice_zsf]);

    				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
				}

				//Saltar la longitud del bloque
    			indice_zsf +=block_lenght;

	}





	texto_browser[indice_buffer]=0;

	//  menu_generic_message_tooltip("ZSF file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("ZSF file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


	free(zsf_file_memory);
    free(texto_browser);

}




//Funcion de card browser pero las funciones usan punteros de memoria en vez de variables z88_dir
void menu_z88_new_ptr_card_browser(char *archivo)
{

		//printf ("Z88 card browser\n");

        //Asignar 1 mb


        int bytes_to_load=1024*1024;

        z80_byte *flash_file_memory;
        flash_file_memory=malloc(bytes_to_load);
        if (flash_file_memory==NULL) {
                debug_printf(VERBOSE_ERR,"Unable to assign memory");
                return;
        }

        //Leemos cabecera archivo
        FILE *ptr_file_flash_browser;
        ptr_file_flash_browser=fopen(archivo,"rb");

        if (!ptr_file_flash_browser) {
                debug_printf(VERBOSE_ERR,"Unable to open file");
                free(flash_file_memory);
                return;
        }


        int leidos=fread(flash_file_memory,1,bytes_to_load,ptr_file_flash_browser);

        if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_flash_browser);

        //El segundo byte tiene que ser 0 (archivo borrado) o '/'
        if (flash_file_memory[1]!=0 && flash_file_memory[1]!='/') {
			//printf ("Calling read text file\n");
        	menu_file_viewer_read_text_file("Flash file",archivo);
        	free(flash_file_memory);
        	return;
        }


	#define MAX_TEXTO 4096
        char texto_buffer[MAX_TEXTO];

        int max_texto=MAX_TEXTO;


        //z88_dir dir;
        z88_eprom_flash_file file;

        //z88_eprom_flash_find_init(&dir,slot);

        z80_byte *dir;

        dir=flash_file_memory;

        char buffer_nombre[Z88_MAX_CARD_FILENAME+1];

        int retorno;
        int longitud;


        int indice_buffer=0;

        do {
                retorno=z88_eprom_new_ptr_flash_find_next(&dir,&file);
                if (retorno) {
                        z88_eprom_flash_get_file_name(&file,buffer_nombre);

			if (buffer_nombre[0]=='.') buffer_nombre[0]='D'; //archivo borrado

			//printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);
                        longitud=strlen(buffer_nombre)+1; //Agregar salto de linea
                        if (indice_buffer+longitud>max_texto-1) retorno=0;
                        else {
                                sprintf (&texto_buffer[indice_buffer],"%s\n",buffer_nombre);
                                indice_buffer +=longitud;
                        }

                }
        } while (retorno!=0);

        texto_buffer[indice_buffer]=0;

	//menu_generic_message_tooltip("Z88 Card Browser", 0, 0, 1, NULL, "%s", texto_buffer);
	zxvision_generic_message_tooltip("Z88 Card Browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_buffer);

        free(flash_file_memory);

}



void menu_file_zxuno_flash_browser_show(char *filename)
{


	//Asignar 4 mb


	int bytes_to_load=4*1024*1024;

	z80_byte *zxuno_flash_file_memory;
	zxuno_flash_file_memory=malloc(bytes_to_load);
	if (zxuno_flash_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	//Leemos cabecera archivo zxuno_flash
        FILE *ptr_file_zxuno_flash_browser;
        ptr_file_zxuno_flash_browser=fopen(filename,"rb");

        if (!ptr_file_zxuno_flash_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(zxuno_flash_file_memory);
		return;
	}


        int leidos=fread(zxuno_flash_file_memory,1,bytes_to_load,ptr_file_zxuno_flash_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_zxuno_flash_browser);





	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;




 	sprintf(buffer_texto,"ZX-Uno Flash image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00006000  00 01 3c 3c 00 00 00 00  fd 5e 00 00 00 00 00 00  |..<<.....^......|
00006010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00006020  5a 58 20 53 70 65 63 74  72 75 6d 20 34 38 4b 20  |ZX Spectrum 48K |
00006030  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |                |
00006040  01 04 3d 00 00 00 00 00  be eb f9 91 e7 97 39 98  |..=...........9.|
00006050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00006060  5a 58 20 53 70 65 63 74  72 75 6d 20 2b 32 41 20  |ZX Spectrum +2A |
00006070  45 4e 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |EN              |
00006080  05 02 3d 28 00 00 00 00  dc ec ef fc 00 00 00 00  |..=(............|
00006090  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000060a0  5a 58 20 53 70 65 63 74  72 75 6d 20 31 32 38 4b  |ZX Spectrum 128K|
000060b0  20 45 4e 20 20 20 20 20  20 20 20 20 20 20 20 20  | EN             |
000060c0  07 01 3c 3c 00 00 00 00  8e f2 00 00 00 00 00 00  |..<<............|
000060d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000060e0  49 6e 76 65 73 20 53 70  65 63 74 72 75 6d 2b 20  |Inves Spectrum+ |
000060f0  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |                |

*/


 	sprintf(buffer_texto,"\nROMS:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int total_roms_show=32;
	int i;
	for (i=0;i<total_roms_show;i++) {
		util_binary_to_ascii(&zxuno_flash_file_memory[0x6020+i*64],buffer_texto,30,30);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}


/*
Bitstreams
00007100  53 61 6d 20 43 6f 75 70  65 20 20 20 20 20 20 20  |Sam Coupe       |
00007110  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007120  4a 75 70 69 74 65 72 20  41 43 45 20 20 20 20 20  |Jupiter ACE     |
00007130  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007140  4d 61 73 74 65 72 20 53  79 73 74 65 6d 20 20 20  |Master System   |
00007150  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007160  42 42 43 20 4d 69 63 72  6f 20 20 20 20 20 20 20  |BBC Micro       |
00007170  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007180  50 61 63 20 4d 61 6e 20  20 20 20 20 20 20 20 20  |Pac Man         |
00007190  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
000071a0  4f 72 69 63 20 41 74 6d  6f 73 20 20 20 20 20 20  |Oric Atmos      |
000071b0  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
000071c0  41 70 70 6c 65 20 32 20  28 56 47 41 29 20 20 20  |Apple 2 (VGA)   |
000071d0  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
000071e0  4e 45 53 20 28 56 47 41  29 20 20 20 20 20 20 20  |NES (VGA)       |
000071f0  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |

*/


	sprintf(buffer_texto,"\nBitstreams:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int total_bitstream_show=8;

	for (i=0;i<total_bitstream_show;i++) {
		util_binary_to_ascii(&zxuno_flash_file_memory[0x7100+i*32],buffer_texto,30,30);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}


	//  menu_generic_message_tooltip("ZX-Uno Flash browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("ZX-Uno Flash browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);



	free(zxuno_flash_file_memory);
    free(texto_browser);

}


void menu_file_superupgrade_flash_browser_show(char *filename)
{


	//Asignar 512 kb


	int bytes_to_load=4*1024*1024;

	z80_byte *superupgrade_flash_file_memory;
	superupgrade_flash_file_memory=malloc(bytes_to_load);
	if (superupgrade_flash_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	//Leemos cabecera archivo superupgrade_flash
        FILE *ptr_file_superupgrade_flash_browser;
        ptr_file_superupgrade_flash_browser=fopen(filename,"rb");

        if (!ptr_file_superupgrade_flash_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(superupgrade_flash_file_memory);
		return;
	}


        int leidos=fread(superupgrade_flash_file_memory,1,bytes_to_load,ptr_file_superupgrade_flash_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_superupgrade_flash_browser);



	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;




 	sprintf(buffer_texto,"Superupgrade Flash image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*

00000300  0f 30 2d 53 50 45 43 54  52 55 4d 20 20 20 20 20  |.0-SPECTRUM     |
00000310  0f 31 2d 49 4e 56 45 53  20 20 20 20 20 20 20 20  |.1-INVES        |
00000320  0f 32 2d 54 4b 2d 39 30  58 20 20 20 20 20 20 20  |.2-TK-90X       |
00000330  0f 33 2d 53 50 45 43 2b  33 45 20 4d 4d 43 20 20  |.3-SPEC+3E MMC  |
00000340  0f 34 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.4----------01  |
00000350  0f 35 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 32 20 20  |.5----------02  |
00000360  0f 36 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 33 20 20  |.6----------03  |
00000370  0f 37 2d 53 50 45 43 54  52 55 4d 2b 32 20 20 20  |.7-SPECTRUM+2   |
00000380  0f 38 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.8----------01  |
00000390  0f 39 2d 53 50 45 43 54  2e 20 20 31 32 38 20 20  |.9-SPECT.  128  |
000003a0  0f 41 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.A----------01  |
000003b0  0f 42 2d 53 50 45 43 54  52 55 4d 2b 33 45 20 20  |.B-SPECTRUM+3E  |
000003c0  0f 43 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.C----------01  |
000003d0  0f 44 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 32 20 20  |.D----------02  |
000003e0  0f 45 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 33 20 20  |.E----------03  |
000003f0  0f 46 2d 4a 55 50 49 54  45 52 20 41 43 45 20 20  |.F-JUPITER ACE  |
00000400  0f 47 2d 54 4b 2d 39 35  20 20 20 20 20 20 20 20  |.G-TK-95        |
00000410  0f 48 2d 54 45 53 54 20  4d 43 4c 45 4f 44 20 20  |.H-TEST MCLEOD  |
00000420  0f 49 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.I----------01  |
00000430  0f 4a 2d 5a 58 20 54 45  53 54 20 52 4f 4d 20 20  |.J-ZX TEST ROM  |
00000440  0f 4b 2d 53 45 20 42 41  53 49 43 20 20 20 20 20  |.K-SE BASIC     |
00000450  0f 4c 2d 4d 41 4e 49 43  20 4d 49 4e 45 52 20 20  |.L-MANIC MINER  |
00000460  0f 4d 2d 4a 45 54 20 53  45 54 20 57 49 4c 4c 59  |.M-JET SET WILLY|
00000470  0f 4e 2d 53 54 41 52 20  57 41 52 53 20 20 20 20  |.N-STAR WARS    |
00000480  0f 4f 2d 44 45 41 54 48  20 43 48 41 53 45 20 20  |.O-DEATH CHASE  |
00000490  0f 50 2d 4d 49 53 43 4f  20 4a 4f 4e 45 53 20 20  |.P-MISCO JONES  |
000004a0  0f 51 2d 4c 41 4c 41 20  50 52 4f 4c 4f 47 55 45  |.Q-LALA PROLOGUE|
000004b0  0f 52 2d 53 50 41 43 45  20 52 41 49 44 45 52 53  |.R-SPACE RAIDERS|
000004c0  0f 53 2d 43 48 45 53 53  20 20 20 20 20 20 20 20  |.S-CHESS        |
000004d0  0f 54 2d 51 42 45 52 54  20 20 20 20 20 20 20 20  |.T-QBERT        |
000004e0  0f 55 2d 50 4f 50 45 59  45 20 20 20 20 20 20 20  |.U-POPEYE       |
000004f0  0f 56 2d 48 45 52 52 41  4d 49 45 4e 54 41 53 20  |.V-HERRAMIENTAS |
00000500  e0 61 62 63 04 05 06 07  48 49 4a 4b 0c 0d 0e 0f  |.abc....HIJK....|



*/


 	sprintf(buffer_texto,"\nROMS:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int total_roms_show=32;
	int i;
	for (i=0;i<total_roms_show;i++) {
		util_binary_to_ascii(&superupgrade_flash_file_memory[0x301+i*16],buffer_texto,15,15);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}



	//menu_generic_message_tooltip("Superupgrade Flash browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Superupgrade Flash browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


	free(superupgrade_flash_file_memory);
    free(texto_browser);

}




void menu_file_flash_browser_show(char *filename)
{



	//Leemos cabecera archivo flash
	z80_byte flash_cabecera[256];

        FILE *ptr_file_flash_browser;
        ptr_file_flash_browser=fopen(filename,"rb");

        if (!ptr_file_flash_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}


        int leidos=fread(flash_cabecera,1,256,ptr_file_flash_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_flash_browser);


        if (get_file_size(filename)==4*1024*1024
        	&& flash_cabecera[0]==0xff
        	&& flash_cabecera[1]==0xff
        	&& flash_cabecera[2]==0xff
        	&& flash_cabecera[3]==0xff
        	) {
        	menu_file_zxuno_flash_browser_show(filename);
       	}


       	else if (
       		   flash_cabecera[0]==1
        	&& flash_cabecera[1]==0
        	&& flash_cabecera[2]==0
        	&& flash_cabecera[3]==0
        	&& flash_cabecera[4]==0
        	&& flash_cabecera[5]==0
       	 	) {
       		//adivinar que es z88 flash
       		//01 00 00 00 00 00

       		menu_z88_new_ptr_card_browser(filename);
       	}

       	else if (
       		   flash_cabecera[0]==0x01
        	&& flash_cabecera[1]==0xfd
        	&& flash_cabecera[2]==0x7f
		) {
       		//adivinar superupgrade
       		//00000000  01 fd 7f 3e c5 ed 79 01
       		menu_file_superupgrade_flash_browser_show(filename);
       	}


       	else {
       		//binario
       		 menu_file_viewer_read_text_file("Flash file",filename);
       	}

}

const int menu_dsk_sector_sizes_numbers[]={
    0,    //0: TODO: no tengo claro que 0 sea tal cual sector size 0
    256,  //1
    512,  //2
    1024, //3
    2048, //4
    4096, //5
    8192, //6
    16384 //7
};

int menu_dsk_get_sector_size_from_n_value(int n_value)
{

    //It is assumed that sector sizes are defined as 3 bits only, so that a sector size of N="8" is equivalent to N="0".
    //Mot o Mundial de futbol tienen algunos sectores con tamaño 8
    n_value &=7;

    int sector_size=menu_dsk_sector_sizes_numbers[n_value];

    return sector_size;
}

//entrada: offset a track information block
int menu_dsk_get_total_sectors_track_from_offset(z80_byte *dsk_file_memory,int longitud_dsk,int offset)
{
    int total_sectors=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset+0x15);

    return total_sectors;
}

int menu_dsk_detect_extended_dsk(z80_byte *dsk_memoria)
{


    if (!memcmp("EXTENDED",dsk_memoria,8)) {
        //printf("Detected Extended DSK\n");
        return 1;
    }

    else return 0;
}

//entrada: offset a track information block
int menu_dsk_get_sector_size_track_from_offset(z80_byte *dsk_file_memory,int longitud_dsk,int offset)
{
    int sector_size_byte=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset+0x14);


    return menu_dsk_get_sector_size_from_n_value(sector_size_byte);
}

//Retorna numero de pista.
//Entrada: offset: offset a track-info
int menu_dsk_get_track_number_from_offset(z80_byte *dsk_file_memory,int longitud_dsk,int offset)
{
    z80_byte track_number=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset+0x10);
    return track_number;
}

//Retorna numero de cata.
//Entrada: offset: offset a track-info
int menu_dsk_get_track_side_from_offset(z80_byte *dsk_file_memory,int longitud_dsk,int offset)
{
    z80_byte side_number=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset+0x11);
    return side_number;
}

int menu_dsk_get_total_sides(z80_byte *dsk_file_memory,int longitud_dsk)
{
    return util_get_byte_protect(dsk_file_memory,longitud_dsk,0x31);
}

int menu_dsk_get_total_pistas(z80_byte *dsk_file_memory,int longitud_dsk)
{
    int total_pistas=util_get_byte_protect(dsk_file_memory,longitud_dsk,0x30);
    return total_pistas;
}

int menu_dsk_extended_get_start_track(z80_byte *dsk_file_memory,int longitud_dsk,int pista_encontrar,int cara_encontrar)
{
    int pista,cara;
    int offset=0x100;
    int offset_track_table=0x34;

    int total_pistas=menu_dsk_get_total_pistas(dsk_file_memory,longitud_dsk);

    for (pista=0;pista<total_pistas;pista++) {
        for (cara=0;cara<menu_dsk_get_total_sides(dsk_file_memory,longitud_dsk);cara++) {
            //printf("Pista: %d cara: %d\n",pista,cara);


            z80_byte track_number=menu_dsk_get_track_number_from_offset(dsk_file_memory,longitud_dsk,offset);
            z80_byte side_number=menu_dsk_get_track_side_from_offset(dsk_file_memory,longitud_dsk,offset);

            //printf("menu_dsk_extended_get_start_track: pista: %d current_track: %d offset: %XH buscar pista: %d\n",
            //    pista,track_number,offset,pista_encontrar);

            if (track_number==pista_encontrar && side_number==cara_encontrar) {
                //printf("dsk_extended_get_start_track: return %X\n",offset);
                return offset;
            }

            int sector_size=menu_dsk_get_sector_size_track_from_offset(dsk_file_memory,longitud_dsk,offset);
            if (sector_size<0) {
                debug_printf(VERBOSE_ERR,"DSK Extended: Sector size not supported on track %d side %d",pista,cara);
                return -1;
            }


            int saltar=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset_track_table)*256;
            offset +=saltar;


            offset_track_table++;


        }
    }

    return -1;

}


int menu_dsk_basic_get_start_track(z80_byte *dsk_file_memory,int longitud_dsk,int pista_encontrar,int cara_encontrar)
{
    int pista;
    int offset=0x100;

    int total_pistas=menu_dsk_get_total_pistas(dsk_file_memory,longitud_dsk);

    for (pista=0;pista<total_pistas;pista++) {
        //printf("Pista: %d\n",pista);


        z80_byte track_number=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset+0x10);
        z80_byte side_number=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset+0x11);

        if (track_number==pista_encontrar && side_number==cara_encontrar) {
            return offset;
        }

        int sector_size=menu_dsk_get_sector_size_track_from_offset(dsk_file_memory,longitud_dsk,offset);
        if (sector_size<0) {
            debug_printf(VERBOSE_ERR,"MENU DSK Basic: Sector size not supported on track %d",pista);
            return -1;
        }

        int total_sectors=menu_dsk_get_total_sectors_track_from_offset(dsk_file_memory,longitud_dsk,offset);

        int saltar=total_sectors*sector_size+256; //256 ocupa el sector block

        offset +=saltar;
    }

    return -1;

}

//Retorna -1 si pista no encontrada
//Retorna offset al Track information block
int menu_dsk_get_start_track(z80_byte *dsk_file_memory,int longitud_dsk,int pista,int cara)
{
    //Hacerlo diferente si dsk basico o extendido
    if (menu_dsk_detect_extended_dsk(dsk_file_memory)) return menu_dsk_extended_get_start_track(dsk_file_memory,longitud_dsk,pista,cara);
    else return menu_dsk_basic_get_start_track(dsk_file_memory,longitud_dsk,pista,cara);
}


//Retorna el offset al dsk segun la pista y sector dados (ambos desde 0...)
//-1 si no se encuentra
int menu_dsk_getoff_track_sector(z80_byte *dsk_memoria,int total_pistas,int pista_buscar,int sector_buscar,int longitud_dsk)
{

/*
sectores van alternados:
00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

1,6,2,7,3,8


0 1 2 3 4 5 6 7 8
0,5,1,6,2,7,3,8,4

*/

	int pista;
	int sector;

	//int iniciopista_orig=256;

    //printf("menu_dsk_getoff_track_sector. pista_buscar=%d sector_buscar=%d\n",pista_buscar,sector_buscar);

	//Buscamos en todo el archivo dsk
	for (pista=0;pista<total_pistas;pista++) {

        //TODO: de momento cara 0
        int iniciopista_orig=menu_dsk_get_start_track(dsk_memoria,longitud_dsk,pista_buscar,0);

        //printf("before getting sectores_en_pista iniciopista_orig=%XH\n",iniciopista_orig);

		//int sectores_en_pista=dsk_memoria[iniciopista_orig+0x15];

        int sectores_en_pista=util_get_byte_protect(dsk_memoria,longitud_dsk,iniciopista_orig+0x15);
		//debug_printf(VERBOSE_DEBUG,"Iniciopista: %XH (%d). Sectores en pista %d: %d. IDS pista:  ",iniciopista_orig,iniciopista_orig,pista,sectores_en_pista);

        //printf("Iniciopista: %XH (%d). Sectores en pista %d: %d. IDS pista:  \n",iniciopista_orig,iniciopista_orig,pista,sectores_en_pista);

		//int iniciopista_orig=traps_plus3dos_getoff_start_trackinfo(pista);
		int iniciopista=iniciopista_orig;
		//saltar 0x18
		iniciopista +=0x18;

		for (sector=0;sector<sectores_en_pista;sector++) {
			int offset_tabla_sector=sector*8;

			//printf("before getting pista_id sumando %d %d\n",iniciopista,offset_tabla_sector);

			int offset_leer_dsk;

			offset_leer_dsk=iniciopista+offset_tabla_sector;
			//Validar offset
			if (offset_leer_dsk>=longitud_dsk) return -1;

            //printf("before getting pistaid\n");
			//z80_byte pista_id=dsk_memoria[offset_leer_dsk]; //Leemos pista id

            z80_byte pista_id=util_get_byte_protect(dsk_memoria,longitud_dsk,offset_leer_dsk); //Leemos pista id


			//printf("after getting pistaid\n");


			//Validar offset
			offset_leer_dsk=iniciopista+offset_tabla_sector+2;
			if (offset_leer_dsk>=longitud_dsk) return -1;


            //printf("before getting sector_id\n");
			//z80_byte sector_id=dsk_memoria[offset_leer_dsk]; //Leemos c1, c2, etc

            z80_byte sector_id=util_get_byte_protect(dsk_memoria,longitud_dsk,offset_leer_dsk); //Leemos c1, c2, etc

			//printf("after getting sector_id\n");

			//debug_printf(VERBOSE_DEBUG,"%02X ",sector_id);


            //printf("C: %02XH R: %02X \n",pista_id,sector_id);

			sector_id &=0xF;

			sector_id--;  //empiezan en 1...

			if (pista_id==pista_buscar && sector_id==sector_buscar) {
				//printf("Found sector %d/%d at %d/%d\n",pista_buscar,sector_buscar,pista,sector);
		                //int offset=traps_plus3dos_getoff_start_track(pista);
		                int offset=iniciopista_orig+256;

                		//int iniciopista=traps_plus3dos_getoff_start_track(pista);
						//printf("returning ok\n");
		                return offset+512*sector;
			}

            //printf("after if\n");

		}

        //printf("after for\n");

		//debug_printf(VERBOSE_DEBUG,"");

		iniciopista_orig +=256;
		iniciopista_orig +=512*sectores_en_pista;
	}

    //printf("Not found sector %d/%d\n",pista_buscar,sector_buscar);

	debug_printf(VERBOSE_DEBUG,"Not found sector %d/%d",pista_buscar,sector_buscar);

	//retornamos offset fuera de rango
	//printf("returning -1\n");
	return -1;


}


//Retorna los dos sectores, indicando los offset de ambos que ocupa un bloque de 1kb
void menu_dsk_getoff_block(z80_byte *dsk_file_memory,int longitud_dsk,int bloque,int *offset1,int *offset2,int incremento_pista)
{
			//int total_pistas=longitud_dsk/4864;
            int total_pistas=menu_dsk_get_total_pistas(dsk_file_memory,longitud_dsk);
            //printf("total_pistas: %d\n",total_pistas);
			int pista;
			int sector_en_pista;

			int sector_total;

			sector_total=bloque*2; //cada bloque es de 2 sectores

            //Incremento_pista indica sumar x pistas de desplazamiento al bloque
            sector_total +=9*incremento_pista;

			//tenemos sector total en variable bloque
			//sacar pista
			pista=sector_total/9; //9 sectores por pista
			sector_en_pista=sector_total % 9;

			//printf ("pista: %d sector en pista: %d\n",pista,sector_en_pista);

			//offset a los datos dentro del dsk
			//int offset=pista*4864+sector_en_pista*512;


			//printf("before getting offset1\n");
			*offset1=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,pista,sector_en_pista,longitud_dsk);

			sector_total++;
			pista=sector_total/9; //9 sectores por pista
			sector_en_pista=sector_total % 9;

			//printf("before getting offset2\n");
			*offset2=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,pista,sector_en_pista,longitud_dsk);
			//printf("after getting offset2\n");

}



int menu_dsk_get_start_filesystem(z80_byte *dsk_file_memory,int longitud_dsk,int *p_pista)
{
    int total_pistas=util_get_byte_protect(dsk_file_memory,longitud_dsk,0x30);

    int pista;

    int puntero;

    for (pista=0;pista<=2;pista++) {

        //printf("Pista: %d\n",pista);

        puntero=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,pista,0,longitud_dsk);
		//Si contiene e5 en el nombre, nos vamos a pista 1
        //O si segundo caracter no es ascii

        if (puntero>=0) {

            z80_byte byte_name=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+1);

            if (byte_name!=0xe5 && (byte_name>=32 && byte_name<=127)) {

                break;

            }

        }

    }

    //Por defecto
    if (puntero<0) {
        //printf ("Filesystem track/sector not found. Guessing it\n");
        puntero=0x200;
    }
    *p_pista=pista;

    //printf("Found filesystem at track %d. Puntero=%X\n",pista,puntero);

    return puntero;
}

int old_menu_dsk_get_start_filesystem(z80_byte *dsk_file_memory,int longitud_dsk)
{

    //int total_pistas=longitud_dsk/4864;
    int total_pistas=menu_dsk_get_total_pistas(dsk_file_memory,longitud_dsk);
    //printf("total pistas: %d\n",total_pistas);


    int pista_offset=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,0,0,longitud_dsk);

    //printf("pista_offset: %XH\n",pista_offset);

    return pista_offset;

}

char *menu_dsk_spec_formats[]={
    "Std PCW range DD SS ST (and +3)",
	"Std CPC range DD SS ST system format",
    "Std CPC range DD SS ST data only format",
    "Std PCW range DD DS DT"
};

//Parecido a menu_file_mmc_browser_show_file pero considerando archivos de filesystem +3, CPC, PCW... que siguen estandar de CP/M

void menu_file_dsk_browser_show_file(z80_byte *origen,char *destino,int sipuntoextension)
{
	int i;
	int salir=0;

    int file_readonly=0;
    int file_system=0;

    int longitud=11;

	for (i=0;i<longitud && !salir;i++) {
		char caracter;
		caracter=*origen;

            //Si en extension, primer byte bit 7: read only. segundo byte: system
            if (i==8 && (caracter & 128)) file_readonly=1;
            if (i==9 && (caracter & 128)) file_system=1;

            caracter &=127;

            if (caracter<32 || caracter>126) caracter='?';

			origen++;

            /*
			if (caracter<32 || caracter>126) {
				//Si detectamos final de texto y siempre que no este en primer caracter
				if (i) salir=1;
				else caracter='?';
			}
            */

			if (!salir) {
				*destino=caracter;
				destino++;

				if (sipuntoextension && i==7) {
					*destino='.';
					destino++;
				}
			}

	}

    if (file_readonly) {
        strcpy(destino," (RO)");
        destino +=5;
    }

    if (file_system) {
        strcpy(destino," (SYS)");
        destino +=6;
    }


	*destino=0;
}


void menu_file_dsk_browser_add_sector_visual_floppy(int pista,int sector)
{
    int i;

    for (i=0;i<512;i++) {
        menu_visual_floppy_buffer_add_persistent(pista,sector,i);
    }
}



z80_byte *menu_file_dsk_browser_show_click_file_dsk_file_memory;
int menu_file_dsk_browser_show_click_file_longitud_dsk;
int menu_file_dsk_browser_show_click_file_incremento_pista_filesystem;
int menu_file_dsk_browser_show_click_file_archivo_seleccionado;

void menu_file_dsk_browser_visualmem_all_blocks(int archivo_seleccionado)
{

    z80_byte bloques[256];

    int total_bloques=util_dsk_get_blocks_entry_file(menu_file_dsk_browser_show_click_file_dsk_file_memory,
        menu_file_dsk_browser_show_click_file_longitud_dsk,bloques,archivo_seleccionado);

    //int indice_buffer=0;

    int i;
    for (i=0;i<total_bloques;i++) {
        z80_byte bloque=bloques[i];
        //printf("---Bloque %d : %02XH\n",i,bloque);

        //de cada bloque obtener pista y sector
        int pista1,sector1,pista2,sector2;
        util_dsk_getsectors_block(menu_file_dsk_browser_show_click_file_dsk_file_memory,
            menu_file_dsk_browser_show_click_file_longitud_dsk,bloque,
            &sector1,&pista1,&sector2,&pista2,menu_file_dsk_browser_show_click_file_incremento_pista_filesystem);
        //printf("---pista1 %d sector1 %d pista2 %d sector2 %d\n",pista1,sector1,pista2,sector2);

        menu_file_dsk_browser_add_sector_visual_floppy(pista1,sector1);
        menu_file_dsk_browser_add_sector_visual_floppy(pista2,sector2);



    }
}


//Aqui entra al pasar el cursor por cualquier de las lineas que no tienen pista y sector
void menu_file_dsk_browser_all_sectors(struct s_menu_item *item_seleccionado GCC_UNUSED)
{
    menu_visual_floppy_buffer_reset();
    menu_file_dsk_browser_visualmem_all_blocks(menu_file_dsk_browser_show_click_file_archivo_seleccionado);
}

//Aqui entra al pasar el cursor por una linea con pista y sector
void menu_file_dsk_browser_separate_sectors(struct s_menu_item *item_seleccionado)
{

    int sector=(item_seleccionado->valor_opcion) & 0xFF;
    int pista=(item_seleccionado->valor_opcion)>>8;

    //printf("Llamado funcion para item: %s pista %d sector %d\n",item_seleccionado->texto_opcion,pista,sector);

    menu_visual_floppy_buffer_reset();
    menu_file_dsk_browser_add_sector_visual_floppy(pista,sector);
}



void menu_file_dsk_browser_show_click_file(MENU_ITEM_PARAMETERS)
{

    menu_file_dsk_browser_show_click_file_archivo_seleccionado=valor_opcion;

    char buffer_texto[64]; //2 lineas, por si acaso


    z80_byte bloques[256];

    //printf("\n\nArchivo %s bloques:\n",buffer_texto);

    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    int opcion_seleccionada=0;

    do {


        //menu_file_dsk_browser_visualmem_all_blocks(valor_opcion);

        int total_bloques=util_dsk_get_blocks_entry_file(menu_file_dsk_browser_show_click_file_dsk_file_memory,
            menu_file_dsk_browser_show_click_file_longitud_dsk,bloques,valor_opcion);

        menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Total Blocks: %d (KB)",total_bloques);
        //Gestion de visual floppy que muestra todos los bloques
        menu_add_item_menu_seleccionado(array_menu_common,menu_file_dsk_browser_all_sectors);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Used blocks:");
        //Gestion de visual floppy que muestra todos los bloques
        menu_add_item_menu_seleccionado(array_menu_common,menu_file_dsk_browser_all_sectors);



        //printf("Total bloques: %d\n",total_bloques);

        int j;

        int indice_buffer=0;
        int items_en_linea=0;

        for (j=0;j<total_bloques;j++) {
            z80_byte bloque=bloques[j];
            //printf("---Bloque %d : %02XH\n",j,bloque);

            //de cada bloque obtener pista y sector
            int pista1,sector1,pista2,sector2;
            util_dsk_getsectors_block(menu_file_dsk_browser_show_click_file_dsk_file_memory,
                menu_file_dsk_browser_show_click_file_longitud_dsk,bloque,
                &sector1,&pista1,&sector2,&pista2,menu_file_dsk_browser_show_click_file_incremento_pista_filesystem);
            //printf("---pista1 %d sector1 %d pista2 %d sector2 %d\n",pista1,sector1,pista2,sector2);

            sprintf(&buffer_texto[indice_buffer],"%02X ",bloque);
            indice_buffer +=3;

            //Cada 8 bloques salto de linea
            if (((j+1)%8)==0) {
                //strcpy(&texto_browser[indice_buffer],"\n");
                menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
                //Gestion de visual floppy que muestra todos los bloques
                menu_add_item_menu_seleccionado(array_menu_common,menu_file_dsk_browser_all_sectors);

                indice_buffer=0;
                items_en_linea=0;
            }
            else {
                items_en_linea++;
            }

        }

        //Si queda algun item por agregar de la linea
        if (items_en_linea) {
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
            //Gestion de visual floppy que muestra todos los bloques
            menu_add_item_menu_seleccionado(array_menu_common,menu_file_dsk_browser_all_sectors);
        }

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Tracks and physical sectors");
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"for every block");
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"(Note: Open visual floppy to");
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"see real location on disk)");


        for (j=0;j<total_bloques;j++) {
            z80_byte bloque=bloques[j];
            //printf("---Bloque %d : %02XH\n",j,bloque);

            //de cada bloque obtener pista y sector
            int pista1,sector1,pista2,sector2;
            util_dsk_getsectors_block(menu_file_dsk_browser_show_click_file_dsk_file_memory,
                menu_file_dsk_browser_show_click_file_longitud_dsk,bloque,
                &sector1,&pista1,&sector2,&pista2,menu_file_dsk_browser_show_click_file_incremento_pista_filesystem);
            //printf("---pista1 %d sector1 %d pista2 %d sector2 %d\n",pista1,sector1,pista2,sector2);

            //Cada bloque son dos sectores:


            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Track %02X Sector %X",pista1,sector1);
            //Gestion de visual floppy para cada item separado, al seleccionar, sin tener que pulsar enter
            menu_add_item_menu_seleccionado(array_menu_common,menu_file_dsk_browser_separate_sectors);
            //Indica con opcion pista y sector
            menu_add_item_menu_valor_opcion(array_menu_common,pista1*256+sector1);


            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Track %02X Sector %X",pista2,sector2);
            //Gestion de visual floppy para cada item separado, al seleccionar, sin tener que pulsar enter
            menu_add_item_menu_seleccionado(array_menu_common,menu_file_dsk_browser_separate_sectors);
            //Indica con opcion pista y sector
            menu_add_item_menu_valor_opcion(array_menu_common,pista2*256+sector2);


        }



        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Blocks");

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


    menu_visual_floppy_buffer_reset();

}

void menu_file_dsk_browser_show(char *filename)
{


	int tamanyo_dsk_entry=32;

	int max_entradas_dsk=64;

	//Asignamos para 16 entradas
	//int bytes_to_load=tamanyo_dsk_entry*max_entradas_dsk;

	//Leemos 300kb. aunque esto excede con creces el tamanyo para leer el directorio, podria pasar que la pista 0 donde esta
	//el directorio estuviera ordenada al final del archivo
	int bytes_to_load=300000;  //temp. 4096

	z80_byte *dsk_file_memory;
	dsk_file_memory=malloc(bytes_to_load);
	if (dsk_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	int longitud_dsk=bytes_to_load;

    menu_file_dsk_browser_show_click_file_longitud_dsk=longitud_dsk;
    menu_file_dsk_browser_show_click_file_dsk_file_memory=dsk_file_memory;

	//Leemos archivo dsk
    FILE *ptr_file_dsk_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_dsk_browser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open file");
        free(dsk_file_memory);
        return;
    }




    int leidos;

    leidos=zvfs_fread(in_fatfs,dsk_file_memory,bytes_to_load,ptr_file_dsk_browser,&fil);


	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading file");
        return;
    }

    zvfs_fclose(in_fatfs,ptr_file_dsk_browser,&fil);


	char buffer_texto[64]; //2 lineas, por si acaso


    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    int opcion_seleccionada=0;

    do {


        menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"DSK disk image");


        /*
        00000000  45 58 54 45 4e 44 45 44  20 43 50 43 20 44 53 4b  |EXTENDED CPC DSK|
        00000010  20 46 69 6c 65 0d 0a 44  69 73 6b 2d 49 6e 66 6f  | File..Disk-Info|
        00000020  0d 0a 43 50 44 52 65 61  64 20 76 33 2e 32 34 00  |..CPDRead v3.24.|
        00000030  2d 01 00 00 13 13 13 13  13 13 13 13 13 13 13 13  |-...............|
        00000040  13 13 13 13 13 13 13 13  13 13 13 13 13 13 13 13  |................|
        00000050  13 13 13 13 13 13 13 13  13 13 13 13 00 00 00 00  |................|
        00000060  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
        *
        00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
        00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
        00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
        00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
        00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
        00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
        00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
        *
        00000200  00 43 4f 4d 50 49 4c 45  52 c2 49 4e 00 00 00 80  |.COMPILER.IN....|
        00000210  02 03 04 05 06 07 08 09  0a 0b 0c 0d 0e 0f 10 11  |................|
        00000220  00 43 4f 4d 50 49 4c 45  52 c2 49 4e 01 00 00 59  |.COMPILER.IN...Y|
        00000230  12 13 14 15 16 17 18 19  1a 1b 1c 1d 00 00 00 00  |................|
        00000240  00 4b 49 54 31 32 38 4c  44 c2 49 4e 00 00 00 03  |.KIT128LD.IN....|
        00000250  1e 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
        00000260  00 4b 49 54 31 32 38 20  20 c2 49 4e 00 00 00 80  |.KIT128  .IN....|
        00000270  1f 20 21 22 23 24 25 26  27 28 29 2a 2b 2c 2d 2e  |. !"#$%&'()*+,-.|
        00000280  00 4b 49 54 31 32 38 20  20 c2 49 4e 01 00 00 59  |.KIT128  .IN...Y|
        */

        //La extension es de 1 byte





        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Signature:");


        util_binary_to_ascii(&dsk_file_memory[0], buffer_texto, 34, 34);
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

        menu_add_item_menu_separator(array_menu_common);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Creator:");


        util_binary_to_ascii(&dsk_file_memory[0x22], buffer_texto, 14, 14);
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

        menu_add_item_menu_separator(array_menu_common);


        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Total tracks: %d",dsk_file_memory[0x30]);


        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Total sides: %d",dsk_file_memory[0x31]);


        //Si tiene Especificacion de formato PCW/+3
        int puntero;
        int total_pistas=menu_dsk_get_total_pistas(dsk_file_memory,longitud_dsk);
        puntero=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,0,0,longitud_dsk);

        z80_byte spec_disk_type=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero);
        z80_byte spec_disk_sides=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+1) & 3;
        z80_byte spec_tracks_side=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+2);
        z80_byte spec_sectors_track=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+3);

        int i;

        if (spec_disk_type<=3 && spec_disk_sides<=2 && spec_tracks_side<50 && spec_sectors_track<10) {
            /*
            Byte 0		Disk type
                0 = Standard PCW range DD SS ST (and +3)
                1 = Standard CPC range DD SS ST system format
                2 = Standard CPC range DD SS ST data only format
                3 = Standard PCW range DD DS DT
                All other values reserved

            Byte 1		Bits 0...1 Sidedness
                        0 = Single sided
                        1 = Double sided (alternating sides)
                        2 = Double sided (successive sides)
                    Bits 2...6 Reserved (set to 0)
                    Bit 7 Double track

            Byte 2		Number of tracks per side

            Byte 3		Number of sectors per track

            Byte 4		Log2(sector size) - 7

            Byte 5		Number of reserved tracks

            Byte 6		Log2(block size / 128)

            Byte 7		Number of directory blocks

            Byte 8		Gap length (read/write)

            Byte 9		Gap length (format)

            Bytes 10...14	Reserved

            Byte 15		Checksum (used only if disk is bootable)
    */

            menu_add_item_menu_separator(array_menu_common);

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"Known disc format:");

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Type: %s",menu_dsk_spec_formats[spec_disk_type]);


            int sides_show=spec_disk_sides;
            if (sides_show>=2) sides_show=1;
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,
                " Sides: %d%s",sides_show+1,(spec_disk_sides==2 ? "(successive sides)" : ""));


            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Tracks/Sides: %d",
                spec_tracks_side);

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Sectors/Track: %d",
                spec_sectors_track);


            int sector_size=128 << util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+4);
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Sector Size: %d",
                sector_size);

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Reserved Tracks: %d",
                util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+5));

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Block size: %d",
                util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+6));

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Directory blocks: %d",
                util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+7));

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Gap length (rw): %d",
                util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+8));

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Gap length (format): %d",
                util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+9));


            //Calcular checksum de todo el sector
            int i;
            z80_byte calculated_checksum=0;

            for (i=0;i<sector_size;i++) {
                calculated_checksum +=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+i);
            }

            calculated_checksum ^=255;
            calculated_checksum +=4;

            //printf("calculated_checksum: %02XH\n",calculated_checksum);

            z80_byte checksum_in_disk=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+15);

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Checksum: %02XH",checksum_in_disk);


            if (calculated_checksum==checksum_in_disk) {
                menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL," Bootable disk");
            }

        }

        menu_add_item_menu_separator(array_menu_common);

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"First Filesystem entries:");
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"(Enter on any for more info)");


        int pista_filesystem;
        puntero=menu_dsk_get_start_filesystem(dsk_file_memory,bytes_to_load,&pista_filesystem);


        //printf("Inicio filesystem: %XH\n",puntero);


        //puntero++; //Saltar el primer byte en la entrada de filesystem

        menu_file_dsk_browser_show_click_file_incremento_pista_filesystem=pista_filesystem;
        for (i=0;i<max_entradas_dsk;i++) {

            z80_byte user_number=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero);

            //Solo mostrar entradas de archivo con primer extent
            z80_byte extent_ex=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+12);
            z80_byte extent_s2=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+13);

            if (extent_ex==0 && extent_s2==0 && user_number!=0xE5) {

                menu_file_dsk_browser_show_file(&dsk_file_memory[puntero+1],buffer_texto,1);
                if (buffer_texto[0]!='?') {
                    menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,
                        menu_file_dsk_browser_show_click_file,NULL,buffer_texto);
                    //Le indicamos el numero de bloque como opcion
                    menu_add_item_menu_valor_opcion(array_menu_common,i);
                    menu_add_item_menu_tiene_submenu(array_menu_common);

                }
            }

            puntero +=tamanyo_dsk_entry;



        }


        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&opcion_seleccionada,&item_seleccionado,array_menu_common,"DSK file viewer");

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


	free(dsk_file_memory);


}

z80_byte *mdr_file_memory;

z80_byte menu_file_mdr_browser_show_get_byte(int microdrive_seleccionado,int sector,int sector_offset)
{
    int offset=sector*MDR_BYTES_PER_SECTOR;

    offset +=sector_offset;

    return mdr_file_memory[offset];
}


void menu_file_mdr_browser_show(char *filename)
{


	int bytes_to_load=get_file_size(filename);


	mdr_file_memory=util_malloc(bytes_to_load,"Can not allocate memory for mdr browser");



    FILE *ptr_file_mdr_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */

    int in_fatfs;

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_mdr_browser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open file");
        free(mdr_file_memory);
        return;
    }


    int leidos;

    leidos=zvfs_fread(in_fatfs,mdr_file_memory,bytes_to_load,ptr_file_mdr_browser,&fil);


    if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading file");
        return;
    }

    zvfs_fclose(in_fatfs,ptr_file_mdr_browser,&fil);


    free(mdr_file_memory);

    int total_sectors=leidos/MDR_BYTES_PER_SECTOR;


    int ancho=50;
    int alto=20;
    int x=menu_center_x()-ancho/2;
    int y=menu_center_y()-alto/2;

    zxvision_window ventana;

    //pueden haber tantos sectores como archivos
    int alto_total_ventana=MDR_MAX_SECTORS;

    zxvision_new_window(&ventana,x,y,ancho,alto,
                                            ancho-1,alto_total_ventana,"Microdrive Browse");


    int linea=menu_microdrive_map_browse(&ventana,1,0,0,menu_file_mdr_browser_show_get_byte,total_sectors);


    //Ajustar al final
    zxvision_set_visible_height(&ventana,linea+2);
    zxvision_set_total_height(&ventana,linea);

    //Recalcular centro
    y=menu_center_y()-ventana.visible_height/2;

    zxvision_set_y_position(&ventana,y);

    zxvision_draw_window(&ventana);
    zxvision_draw_window_contents(&ventana);

    zxvision_wait_until_esc(&ventana);


    zxvision_destroy_window(&ventana);

}



void menu_file_trd_browser_show(char *filename,char *tipo_imagen)
{


	int tamanyo_trd_entry=16;

	int max_entradas_trd=16;

	//Asignamos para 16 entradas
	//int bytes_to_load=tamanyo_trd_entry*max_entradas_trd;

	//Leemos 4kb. esto permite leer el directorio y el label
	int bytes_to_load=4096;

	z80_byte *trd_file_memory;
	trd_file_memory=malloc(bytes_to_load);
	if (trd_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	//Leemos cabecera archivo trd
        FILE *ptr_file_trd_browser;

        //Soporte para FatFS
        FIL fil;        /* File object */

        int in_fatfs;

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_trd_browser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            free(trd_file_memory);
            return;
        }

        /*
        ptr_file_trd_browser=fopen(filename,"rb");

        if (!ptr_file_trd_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(trd_file_memory);
		return;
	}
    */


        int leidos;

        leidos=zvfs_fread(in_fatfs,trd_file_memory,bytes_to_load,ptr_file_trd_browser,&fil);
        //leidos=fread(trd_file_memory,1,bytes_to_load,ptr_file_trd_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_trd_browser,&fil);
        //fclose(ptr_file_trd_browser);





	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;




 	sprintf(buffer_texto,"TRD disk image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00000000  43 43 31 30 41 59 20 20  42 4a 00 4a 00 31 00 01  |CC10AY  BJ.J.1..|
00000010  74 69 74 6c 65 20 20 20  43 20 20 00 1b 1b 01 04  |title   C  .....|
00000020  73 6b 69 6e 20 20 20 20  43 20 20 00 1b 1b 0c 05  |skin    C  .....|
00000030  20 4e 69 67 68 74 49 6e  6d 20 20 b9 0a 0b 07 07  | NightInm  .....|
00000040  20 45 76 61 31 20 20 20  70 74 33 26 0c 0d 02 08  | Eva1   pt3&....|
00000050  20 45 76 61 32 20 20 20  70 74 33 c4 1e 1f 0f 08  | Eva2   pt3.....|
00000060  20 52 6f 73 65 31 20 20  70 74 33 ef 03 04 0e 0a  | Rose1  pt3.....|
00000070  20 52 6f 73 65 32 20 20  70 74 33 a2 04 05 02 0b  | Rose2  pt3.....|
00000080  20 53 74 72 6f 6c 6c 6e  70 74 33 a6 19 1a 07 0b  | Strollnpt3.....|
00000090  20 53 77 6f 6f 6e 43 56  6d 20 20 7f 0f 10 01 0d  | SwoonCVm  .....|
000000a0  20 53 6f 70 68 69 65 45  6d 20 20 76 11 12 01 0e  | SophieEm  v....|
000000b0  20 73 75 6d 6d 65 72 31  70 74 33 e0 06 07 03 0f  | summer1pt3.....|
000000c0  20 73 75 6d 6d 65 72 32  70 74 33 8d 05 06 0a 0f  | summer2pt3.....|
000000d0  20 62 74 74 66 31 20 20  70 74 33 60 0b 0c 00 10  | bttf1  pt3`....|
000000e0  20 62 74 74 66 32 20 20  70 74 33 41 04 05 0c 10  | bttf2  pt3A....|
000000f0  20 54 6f 52 69 73 6b 54  6d 20 20 e3 28 29 01 11  | ToRiskTm  .()..|
*/

	//La extension es de 1 byte


	sprintf(buffer_texto,"Filesystem: TRDOS");
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int start_track_8=256*8;


        char trd_label[8+1];
        menu_file_mmc_browser_show_file(&trd_file_memory[0x8f5],trd_label,0,8);
        sprintf(buffer_texto,"TRD Label: %s",trd_label);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	sprintf(buffer_texto,"Free sectors on disk: %d",trd_file_memory[start_track_8+229]+256*trd_file_memory[start_track_8+230]);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"First free sector %dt:%ds",trd_file_memory[start_track_8+226],trd_file_memory[start_track_8+225]);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	char *trd_disk_types[]={
	"80 tracks, double side",
	"40 tracks, double side",
	"80 tracks, single side",
	"40 tracks, single side"};

	char buffer_trd_disk_type[32];
	z80_byte trd_disk_type=trd_file_memory[start_track_8+227];

	if (trd_disk_type>=0x16 && trd_disk_type<=0x19) {
		strcpy(buffer_trd_disk_type,trd_disk_types[trd_disk_type-0x16]);
	}
	else strcpy(buffer_trd_disk_type,"Unknown");

	sprintf(buffer_texto,"Disk type: %04XH (%s)",trd_disk_type,buffer_trd_disk_type);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Files on disk: %d",trd_file_memory[start_track_8+228]);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Deleted files on disk: %d",trd_file_memory[start_track_8+244]);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	sprintf(buffer_texto,"First file entries:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int puntero,i;

	puntero=0;

	for (i=0;i<max_entradas_trd;i++) {
		menu_file_mmc_browser_show_file(&trd_file_memory[puntero],buffer_texto,1,9);
		if (buffer_texto[0]!='?') {
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
		}
		z80_byte start_sector=trd_file_memory[puntero+14];
		z80_byte start_track=trd_file_memory[puntero+15];
		debug_printf (VERBOSE_DEBUG,"File %s starts at track %d sector %d",buffer_texto,start_track,start_sector);

		puntero +=tamanyo_trd_entry;
	}



	texto_browser[indice_buffer]=0;
	char titulo_ventana[32];
	sprintf(titulo_ventana,"%s file viewer",tipo_imagen);
	//menu_generic_message_tooltip(titulo_ventana, 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip(titulo_ventana , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);



	free(trd_file_memory);
    free(texto_browser);

}




void menu_file_mmc_browser_show_file(z80_byte *origen,char *destino,int sipuntoextension,int longitud)
{
	int i;
	int salir=0;
	for (i=0;i<longitud && !salir;i++) {
		char caracter;
		caracter=*origen;


			origen++;
			if (caracter<32 || caracter>126) {
				//Si detectamos final de texto y siempre que no este en primer caracter
				if (i) salir=1;
				else caracter='?';
			}

			if (!salir) {
				*destino=caracter;
				destino++;

				if (sipuntoextension && i==7) {
					*destino='.';
					destino++;
				}
			}

	}

	*destino=0;
}

void menu_file_mmc_browser_show(char *filename,char *tipo_imagen)
{

	/*
	Ejemplo directorio en fat para imagenes mmc de 32 MB:
00110200  54 42 42 4c 55 45 20 20  46 57 20 20 00 5d 10 95  |TBBLUE  FW  .]..|
00110210  54 49 58 4b 00 00 6f 9d  58 4b 04 00 00 7e 01 00  |TIXK..o.XK...~..|
00110220  54 4d 50 20 20 20 20 20  20 20 20 10 00 5d 10 95  |TMP        ..]..|
00110230  54 49 54 49 00 00 10 95  54 49 5d 00 00 00 00 00  |TITI....TI].....|
00110240  42 49 4e 20 20 20 20 20  20 20 20 10 00 79 4f 9e  |BIN        ..yO.|
00110250  58 4b 58 4b 00 00 4f 9e  58 4b 03 00 00 00 00 00  |XKXK..O.XK......|
00110260  42 49 54 4d 41 50 53 20  20 20 20 10 00 2d 77 9e  |BITMAPS    ..-w.|
00110270  58 4b 58 4b 00 00 77 9e  58 4b 79 00 00 00 00 00  |XKXK..w.XKy.....|
00110280  52 54 43 20 20 20 20 20  20 20 20 10 00 27 88 9e  |RTC        ..'..|
00110290  58 4b 58 4b 00 00 88 9e  58 4b 7d 00 00 00 00 00  |XKXK....XK}.....|
001102a0  53 59 53 20 20 20 20 20  20 20 20 10 00 89 a5 9e  |SYS        .....|
001102b0  58 4b 58 4b 00 00 a5 9e  58 4b 5c 01 00 00 00 00  |XKXK....XK\.....|
001102c0  54 42 42 4c 55 45 20 20  20 20 20 10 00 0f c5 9e  |TBBLUE     .....|
001102d0  58 4b 58 4b 00 00 c5 9e  58 4b f5 01 00 00 00 00  |XKXK....XK......|
001102e0  53 50 52 49 54 45 53 20  20 20 20 10 00 92 95 9e  |SPRITES    .....|
001102f0  58 4b 58 4b 00 00 95 9e  58 4b 09 01 00 00 00 00  |XKXK....XK......|
00110300  e5 45 53 54 49 4e 4f 20  20 20 20 20 08 6c 53 9d  |.ESTINO     .lS.|
	*/

	int tamanyo_vfat_entry=32;
	int tamanyo_plus3_entry=32;

	int max_entradas_vfat=16;

	//Asignamos para 16 entradas
	int bytes_to_load=0x110200+tamanyo_vfat_entry*max_entradas_vfat;

	z80_byte *mmc_file_memory;
	mmc_file_memory=malloc(bytes_to_load);
	if (mmc_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	//Leemos cabecera archivo mmc
        FILE *ptr_file_mmc_browser;
        ptr_file_mmc_browser=fopen(filename,"rb");

        if (!ptr_file_mmc_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(mmc_file_memory);
		return;
	}


        int leidos=fread(mmc_file_memory,1,bytes_to_load,ptr_file_mmc_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_mmc_browser);





	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;




 	sprintf(buffer_texto,"RAW disk image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00100020  00 00 00 00 80 00 29 bb  4f 74 10 4e 4f 20 4e 41  |......).Ot.NO NA|
00100030  4d 45 20 20 20 20 46 41  54 31 36 20 20 20 0e 1f  |ME    FAT16   ..|
*/

/*
00000000  50 4c 55 53 49 44 45 44  4f 53 20 20 20 20 20 20  |PLUSIDEDOS      |
00000010  01 00 00 00 00 00 00 7f  00 00 00 00 00 00 00 00  |................|
00000020  02 01 02 80 00 01 04 00  38 38 00 00 00 00 00 00  |........88......|
00000030  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000040  64 61 74 6f 73 20 20 20  20 20 20 20 20 20 20 20  |datos           |
00000050  03 00 00 01 80 00 00 ff  7f 00 00 00 00 00 00 00  |................|
00000060  00 02 06 3f 03 f7 07 ff  01 c0 00 00 80 00 00 02  |...?............|
00000070  03 00 ff 80 00 00 02 00  00 00 00 00 43 00 00 00  |............C...|
00000080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000090  ff 80 00 01 01 01 01 7f  81 00 00 00 00 00 00 00  |................|
000000a0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00010000  00 53 50 45 43 54 34 38  4b 52 4f 4d 00 00 00 80  |.SPECT48KROM....|
00010010  02 00 03 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010020  00 42 49 4f 53 20 20 20  20 52 4f 4d 00 00 00 80  |.BIOS    ROM....|
00010030  04 00 05 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010040  00 49 4e 56 45 53 20 20  20 52 4f 4d 00 00 00 80  |.INVES   ROM....|
00010050  06 00 07 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010060  00 54 4b 39 30 58 20 20  20 52 4f 4d 00 00 00 80  |.TK90X   ROM....|
00010070  08 00 09 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010080  00 50 33 33 45 4d 4d 43  30 52 4f 4d 00 00 00 80  |.P33EMMC0ROM....|
*/

	//char filesystem[32];
	//memcpy(filesystem,&mmc_file_memory[0x100036],5);

	//filesystem[5]=0;
	//if (!strcmp(filesystem,"FAT16")) {
    if (util_if_filesystem_fat16(mmc_file_memory,leidos)) {


        sprintf(buffer_texto,"Filesystem: FAT16");
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		char vfat_label[8+3+1];
		menu_file_mmc_browser_show_file(&mmc_file_memory[0x10002b],vfat_label,0,11);
		sprintf(buffer_texto,"FAT Label: %s",vfat_label);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		sprintf(buffer_texto,"First FAT entries:");
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

		int puntero,i;

		puntero=0x110200;
/*


https://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html


Bytes   Content
0-10    File name (8 bytes) with extension (3 bytes)
11      Attribute - a bitvector. Bit 0: read only. Bit 1: hidden.
        Bit 2: system file. Bit 3: volume label. Bit 4: subdirectory.
        Bit 5: archive. Bits 6-7: unused.
12-21   Reserved (see below)
22-23   Time (5/6/5 bits, for hour/minutes/doubleseconds)
24-25   Date (7/4/5 bits, for year-since-1980/month/day)
26-27   Starting cluster (0 for an empty file)
28-31   Filesize in bytes
*/

		for (i=0;i<max_entradas_vfat;i++) {
			z80_byte file_attribute=mmc_file_memory[puntero+11];
			menu_file_mmc_browser_show_file(&mmc_file_memory[puntero],buffer_texto,1,11);
			if (buffer_texto[0]!='?') {
				//Es directorio?
				if (file_attribute&16) {
					//Agregamos texto <DIR>
					int l=strlen(buffer_texto);
					strcpy(&buffer_texto[l]," <DIR>");
				}

				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
			}

			puntero +=tamanyo_vfat_entry;
		}
	}



	//memcpy(filesystem,&mmc_file_memory[0],10);

	//filesystem[10]=0;
	//if (!strcmp(filesystem,"PLUSIDEDOS")) {
    if (util_if_filesystem_plusidedos(mmc_file_memory,leidos)) {

		sprintf(buffer_texto,"Filesystem: PLUSIDEDOS");
        	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		char plus3_label[16+1];
		menu_file_mmc_browser_show_file(&mmc_file_memory[0x40],plus3_label,0,11);
		sprintf(buffer_texto,"Label: %s",plus3_label);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		sprintf(buffer_texto,"First PLUS3 entries:");
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

		int puntero,i;

		puntero=0x10000+1;

		for (i=0;i<max_entradas_vfat;i++) {
			menu_file_mmc_browser_show_file(&mmc_file_memory[puntero],buffer_texto,1,11);
			if (buffer_texto[0]!='?') {
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
			}

			puntero +=tamanyo_plus3_entry;
		}
	}


	texto_browser[indice_buffer]=0;
	char titulo_ventana[32];
	sprintf(titulo_ventana,"%s file viewer",tipo_imagen);
	//menu_generic_message_tooltip(titulo_ventana, 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip(titulo_ventana , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);



	free(mmc_file_memory);
    free(texto_browser);

}



void menu_file_p_browser_show(char *filename)
{

	//Leemos cabecera archivo p
        FILE *ptr_file_p_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("menu_file_p_browser_show %s\n",filename);

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_p_browser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open file");
        return;
    }

    /*
        ptr_file_p_browser=fopen(filename,"rb");

        if (!ptr_file_p_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
    */

    //Saltar el nombre del principio si es un .p81, guardando en un buffer
    char nombre_desde_p81[256]="";
    if (!util_compare_file_extension(filename,"p81")) {
        //maximo 255
        int bytes_to_load=255;
        int i=0;
        z80_byte byte_leido=0;
        while (bytes_to_load>0 && (byte_leido&128)==0) {
            //printf("saltando byte\n");
            zvfs_fread(in_fatfs,&byte_leido,1,ptr_file_p_browser,&fil);

            z80_bit inverse;
            z80_byte caracter_ascii=da_codigo81_solo_letras(byte_leido,&inverse);
            nombre_desde_p81[i]=caracter_ascii;

            bytes_to_load--;
            i++;
        }
        nombre_desde_p81[i]=0;
    }

	//Leer 128 bytes de la cabecera. Nota: archivos .P no tienen cabecera como tal
	z80_byte p_header[128];

        int leidos;

        leidos=zvfs_fread(in_fatfs,p_header,128,ptr_file_p_browser,&fil);

        //leidos=fread(p_header,1,128,ptr_file_p_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_p_browser,&fil);

        //fclose(ptr_file_p_browser);


	char buffer_texto[64]; //2 lineas, por si acaso



	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

	sprintf(buffer_texto,"Machine: ZX-81");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    char nombre_corto[PATH_MAX];
    util_get_file_no_directory(filename,nombre_corto);

    char nombre_sin_extension[PATH_MAX];
    util_get_file_without_extension(nombre_corto,nombre_sin_extension);
    string_a_mayusculas(nombre_sin_extension,nombre_sin_extension);

    if (!util_compare_file_extension(filename,"p81")) {
        strcpy(nombre_sin_extension,nombre_desde_p81);
    }

	sprintf(buffer_texto,"Program name: %s",nombre_sin_extension);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    int tamanyo_archivo=get_file_size(filename);
	sprintf(buffer_texto,"Size: %d",tamanyo_archivo);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"\nSystem variables:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //Inicio de los datos de un archivo P va a la direccion 0x4009 (16393)
    z80_int start_data=16393;

    //16394 E_PPC Number of current line (with program cursor)
    z80_int sysvar_e_ppc=p_header[16394-start_data]+256*p_header[16395-start_data];
	sprintf(buffer_texto,"E_PPC  (current curs. line):   %d",sysvar_e_ppc);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16425 NXTLIN Address of next program line to be executed
    z80_int sysvar_nxtlin=p_header[16425-start_data]+256*p_header[16426-start_data];
	sprintf(buffer_texto,"NXTLIN (add. next line ex.):   %d",sysvar_nxtlin);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16427 OLDPPC Line number to which CONT jumps.
    z80_int sysvar_oldppc=p_header[16427-start_data]+256*p_header[16428-start_data];
	sprintf(buffer_texto,"OLDPPC (line CONT sentence):   %d",sysvar_oldppc);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16400 VARS
    z80_int sysvar_vars=p_header[16400-start_data]+256*p_header[16401-start_data];
	sprintf(buffer_texto,"VARS   (Basic Variables):      %d",sysvar_vars);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    int total_vars=start_data+tamanyo_archivo-sysvar_vars;
    total_vars-=2; //Por alguna razon que desconozco esto es 2 menos, probado con archivos sin variables y con una sola variable
    if (total_vars<0) total_vars=0;
	sprintf(buffer_texto," Basic Variables size:         %d",total_vars);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16434 SEED. The seed for RND. This is the variable that is set by RAND
    z80_int sysvar_seed=p_header[16434-start_data]+256*p_header[16435-start_data];
	sprintf(buffer_texto,"SEED   (seed for RND):         %d",sysvar_seed);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16436 FRAMES. Counts the frames displayed on the television. Bit 15 is 1.
    //Bits 0 to 14 are decremented for each frame sent to the television.
    z80_int sysvar_frames=p_header[16436-start_data]+256*p_header[16437-start_data];
	sprintf(buffer_texto,"FRAMES (frames displayed-dec): %d",sysvar_frames);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);




	texto_browser[indice_buffer]=0;

    zxvision_generic_message_tooltip("P file Browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


    free(texto_browser);

}


void menu_file_o_browser_show(char *filename)
{

	//Leemos cabecera archivo o
        FILE *ptr_file_o_browser;

        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;

        //printf("menu_file_p_browser_show %s\n",filename);

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_o_browser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return;
        }

        /*
        ptr_file_o_browser=fopen(filename,"rb");

        if (!ptr_file_o_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
    */

	//Leer 128 bytes de la cabecera. Nota: archivos .O no tienen cabecera como tal
	z80_byte o_header[128];

        int leidos;

        leidos=zvfs_fread(in_fatfs,o_header,128,ptr_file_o_browser,&fil);
        //leidos=fread(o_header,1,128,ptr_file_o_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_o_browser,&fil);
        //fclose(ptr_file_o_browser);


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

	sprintf(buffer_texto,"Machine: ZX-80");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    int tamanyo_archivo=get_file_size(filename);
	sprintf(buffer_texto,"Size: %d",tamanyo_archivo);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //Inicio de los datos de un archivo O va a la direccion 0x4000
	sprintf(buffer_texto,"\nSystem variables:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_int start_data=16384;


    //16390 E_PPC Number of current line (with program cursor)
    z80_int sysvar_e_ppc=o_header[16390-start_data]+256*o_header[16391-start_data];
	sprintf(buffer_texto,"E_PPC  (current curs. line): %d",sysvar_e_ppc);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16427 OLDPPC Line number to which CONT jumps.
    z80_int sysvar_oldppc=o_header[16407-start_data]+256*o_header[16408-start_data];
	sprintf(buffer_texto,"OLDPPC (line CONT sentence): %d",sysvar_oldppc);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16392 VARS
    z80_int sysvar_vars=o_header[16392-start_data]+256*o_header[16393-start_data];
	sprintf(buffer_texto,"VARS   (Basic Variables):    %d",sysvar_vars);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    int total_vars=start_data+tamanyo_archivo-sysvar_vars;
    total_vars-=2; //Por alguna razon que desconozco esto es 2 menos, probado con archivos sin variables y con una sola variable
    if (total_vars<0) total_vars=0;
	sprintf(buffer_texto," Basic Variables size:       %d",total_vars);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16412 SEED. The seed for RND. This is the variable that is set by RAND
    z80_int sysvar_seed=o_header[16412-start_data]+256*o_header[16413-start_data];
	sprintf(buffer_texto,"SEED   (seed for RND):       %d",sysvar_seed);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    //16414 FRAMES. number of frames displayed since the ZX-80 was switched on
    z80_int sysvar_frames=o_header[16414-start_data]+256*o_header[16415-start_data];
	sprintf(buffer_texto,"FRAMES (frames displayed):   %d",sysvar_frames);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("O file browser", 0, 0, 1, NULL, "%s", texto_browser);
    zxvision_generic_message_tooltip("O file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tao_get_info(z80_byte *tape,char *texto)

    free(texto_browser);
}




void menu_file_hexdump_browser_show(char *filename)
{


	//Cargamos maximo 4 kb
#define MAX_HEXDUMP_FILE 4096
	int bytes_to_load=MAX_HEXDUMP_FILE;

	z80_byte *hexdump_file_memory;
	hexdump_file_memory=malloc(bytes_to_load);
	if (hexdump_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	//Leemos cabecera archivo hexdump
        FILE *ptr_file_hexdump_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */




    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_hexdump_browser,&fil)<0) {
        debug_printf (VERBOSE_ERR,"Unable to open file");
        free(hexdump_file_memory);
        return;
    }


/*
    =util_path_is_prefix_mmc_fatfs(filename);
    printf("txt esta en fatfs: %d\n",in_fatfs);

    if (in_fatfs) {
        fr = f_open(&fil, filename, FA_READ);
        if (fr!=FR_OK)
        {
            debug_printf (VERBOSE_ERR,"Unable to open %s file",filename);
            return;
        }

        //Esto solo para que no se queje el compilador al llamar a zvfs_fread
        ptr_file_hexdump_browser=NULL;

    }

    else {

        ptr_file_hexdump_browser=fopen(filename,"rb");

        if (!ptr_file_hexdump_browser) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            free(hexdump_file_memory);
            return;
	    }
    }
*/

        int leidos;

    leidos=zvfs_fread(in_fatfs,hexdump_file_memory,bytes_to_load,ptr_file_hexdump_browser,&fil);

/*
    if (in_fatfs) {
        UINT leidos_fatfs;
        FRESULT resultado=f_read(&fil,hexdump_file_memory,bytes_to_load,&leidos_fatfs);
        leidos=leidos_fatfs;
    }

    else {

        leidos=fread(hexdump_file_memory,1,bytes_to_load,ptr_file_hexdump_browser);
    }
*/

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

    zvfs_fclose(in_fatfs,ptr_file_hexdump_browser,&fil);

    /*
    if (in_fatfs) {
        f_close(&fil);
    }
    else {

        fclose(ptr_file_hexdump_browser);
    }
    */



	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;
#define MAX_TEXTO_BROWSER_HEX (MAX_HEXDUMP_FILE*5)
//Por cada linea de texto de 33 bytes, se muestran 8 bytes del fichero. Por tanto, podemos aproximar por lo alto,
//que el texto ocupa 5 veces los bytes. Ejemplo 8*5=40

	char texto_browser[MAX_TEXTO_BROWSER_HEX];
	int indice_buffer=0;


 	sprintf(buffer_texto,"Hexadecimal view");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	long long int tamanyo=get_file_size(filename);
 	sprintf(buffer_texto,"File size: %lld bytes",tamanyo);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	if (tamanyo>bytes_to_load) {
		leidos=bytes_to_load;
		sprintf(buffer_texto,"Showing first %d bytes",leidos);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}

	int i;
	//char buffer_indice[5];
	char buffer_hex[8*2+1];
	char buffer_ascii[8+1];

	for (i=0;i<leidos;i+=8) {
		util_binary_to_hex(&hexdump_file_memory[i],buffer_hex,8,leidos-i);
		util_binary_to_ascii(&hexdump_file_memory[i],buffer_ascii,8,leidos-i);
		sprintf(buffer_texto,"%04X %s %s",i,buffer_hex,buffer_ascii);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}



	texto_browser[indice_buffer]=0;

	//menu_generic_message_tooltip("Hex viewer", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Hex viewer" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	free(hexdump_file_memory);

}




void menu_file_realtape_browser_show(char *filename)
{

    char *texto_browser=util_malloc_max_texto_browser();



    util_realtape_browser(filename, texto_browser, MAX_TEXTO_BROWSER,NULL,NULL,0,NULL);


    if (texto_browser[0]==0) {
        //Intentamos con conversión ZX80/81

        //TODO: no autodetectara cintas de ZX80, hay que seleccionar maquina ZX80 como actual para que el conversor asuma ZX80
        convert_realtape_to_po(filename, NULL, texto_browser,0);
    }

    if (texto_browser[0]==0) {
        strcpy(texto_browser,"Tape empty or unknown audio data");
    }


    zxvision_generic_message_tooltip("Realtape file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

    free(texto_browser);

}




//Retorna nombre de cinta cas (6 bytes)
void menu_file_cas_browser_show_getname(z80_byte *tape,char *texto)
{
	int i;
	z80_byte caracter;

	for (i=0;i<6;i++) {
		caracter=*tape++;
		if (caracter<32 || caracter>126) caracter='.';

		*texto++=caracter;
	}

	*texto=0;
}


void menu_file_cas_browser_show(char *filename)
{
	long posicion_lectura;
	z80_byte buffer[10];
	FILE *ptr_file_cas_browser;

	int indice_buffer=0;

	//Extracted from https://github.com/joyrex2001/castools/blob/master/casdir.c

	#define CAS_NEXT_NONE   0
	#define CAS_NEXT_BINARY 1
	#define CAS_NEXT_DATA   2

	char cas_filename[7]="123456"; //no deberia entrar inicializado pero por si acaso

	int next=CAS_NEXT_NONE;

	z80_byte cas_ascii[10]={ 0xEA,0xEA,0xEA,0xEA,0xEA,0xEA,0xEA,0xEA,0xEA,0xEA };
	z80_byte cas_bin[10]={ 0xD0,0xD0,0xD0,0xD0,0xD0,0xD0,0xD0,0xD0,0xD0,0xD0 };
	z80_byte cas_basic[10]={ 0xD3,0xD3,0xD3,0xD3,0xD3,0xD3,0xD3,0xD3,0xD3,0xD3 };


	char buffer_texto[300]; //Para poder contener info de msx cas extensa

    //Soporte para FatFS
    FIL fil;        /* File object */

    int in_fatfs;

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_cas_browser,&fil)<0) {
		debug_printf(VERBOSE_ERR,"Error opening cas file %s",filename);
		return;
    }

    /*
	ptr_file_cas_browser=fopen(filename,"rb");

	if (ptr_file_cas_browser==NULL) {
		debug_printf(VERBOSE_ERR,"Error opening cas file %s",filename);
		return;
	}
    */

	posicion_lectura=0;

    char *texto_browser=util_malloc_max_texto_browser();


	while (zvfs_fread(in_fatfs,buffer,8,ptr_file_cas_browser,&fil)==8) {

		if (!memcmp(buffer,msx_cabecera_firma,8)) {


			if (zvfs_fread(in_fatfs,buffer,10,ptr_file_cas_browser,&fil)==10) {

				if (next==CAS_NEXT_BINARY) {

					zvfs_fseek(in_fatfs,ptr_file_cas_browser,posicion_lectura+8,SEEK_SET,&fil);

                    //void zvfs_fseek(int in_fatfs,FILE *ptr_file, long offset, int whence,FIL *fil)

					z80_byte buffer_datos[6];

                    zvfs_fread(in_fatfs,buffer_datos,6,ptr_file_cas_browser,&fil);
					//fread(buffer_datos,1,6,ptr_file_cas_browser);

					z80_int start,stop,exec;

					start=buffer_datos[0]+256*buffer_datos[1];
					stop=buffer_datos[2]+256*buffer_datos[3];
					exec=buffer_datos[4]+256*buffer_datos[5];

					if (!exec) exec=start;

					sprintf(buffer_texto,"Binary: %s\n  Start: %d Stop: %d\n  Exec:  %d",cas_filename,start,stop,exec);
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
					next=CAS_NEXT_NONE;
				}

				else if (next==CAS_NEXT_DATA) {
					next=CAS_NEXT_NONE;
				}

				else if (!memcmp(buffer,cas_ascii,10)) {

					z80_byte buffer_nombre[7];

                    zvfs_fread(in_fatfs,buffer_nombre,6,ptr_file_cas_browser,&fil);
					//fread(buffer_nombre,1,6,ptr_file_cas_browser);

					menu_file_cas_browser_show_getname(buffer_nombre,cas_filename);

					sprintf(buffer_texto,"Ascii: %s",cas_filename);
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

					while (zvfs_fgetc(in_fatfs,ptr_file_cas_browser,&fil)!=0x1a && !zvfs_feof(in_fatfs,ptr_file_cas_browser,&fil) );





                    //long zvfs_ftell(int in_fatfs,FILE *ptr_file, FIL *fil)
					posicion_lectura=zvfs_ftell(in_fatfs,ptr_file_cas_browser,&fil);
				}

				else if (!memcmp(buffer,cas_bin,10)) {
					z80_byte buffer_nombre[7];

                    zvfs_fread(in_fatfs,buffer_nombre,6,ptr_file_cas_browser,&fil);
					//fread(buffer_nombre,1,6,ptr_file_cas_browser);

					menu_file_cas_browser_show_getname(buffer_nombre,cas_filename);
					next=CAS_NEXT_BINARY;
				}

				else if (!memcmp(buffer,cas_basic,10)) {
					z80_byte buffer_nombre[7];

                    zvfs_fread(in_fatfs,buffer_nombre,6,ptr_file_cas_browser,&fil);
					//fread(buffer_nombre,1,6,ptr_file_cas_browser);

					menu_file_cas_browser_show_getname(buffer_nombre,cas_filename);

					next=CAS_NEXT_DATA;
					sprintf(buffer_texto,"Basic: %s", cas_filename);
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
				}

				else  {
					sprintf(buffer_texto,"Custom");
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
				}

			}

    	}

		posicion_lectura++;
        zvfs_fseek(in_fatfs,ptr_file_cas_browser,posicion_lectura,SEEK_SET,&fil);
    	//fseek(ptr_file_cas_browser,posicion_lectura,SEEK_SET);


	}

    zvfs_fclose(in_fatfs,ptr_file_cas_browser,&fil);
	//fclose(ptr_file_cas_browser);

	texto_browser[indice_buffer]=0;
	zxvision_generic_message_tooltip("CAS file viewer" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

    free(texto_browser);

}





void menu_file_pzx_browser_show(char *filename)
{

	int filesize=get_file_size(filename);
	z80_byte *pzx_file_mem;

	pzx_file_mem=malloc(filesize);
	if (pzx_file_mem==NULL) {
		debug_printf(VERBOSE_ERR,"Can not allocate memory for pzx browser");
		return;
	}


	//Leemos cabecera archivo pzx
        FILE *ptr_file_pzx_browser;

        //Soporte para FatFS
        FIL fil;        /* File object */

        int in_fatfs;

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_pzx_browser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return;
        }

        /*
        ptr_file_pzx_browser=fopen(filename,"rb");

    if (!ptr_file_pzx_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
        */

        int leidos;

        leidos=zvfs_fread(in_fatfs,pzx_file_mem,filesize,ptr_file_pzx_browser,&fil);
        //leidos=fread(pzx_file_mem,1,filesize,ptr_file_pzx_browser);

	if (leidos!=filesize) {
                debug_printf(VERBOSE_ERR,"Error reading file");
		free(pzx_file_mem);
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_pzx_browser,&fil);
        //fclose(ptr_file_pzx_browser);


	//char buffer_texto[300]; //Para poder contener info de pzx extensa



	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

	//Ir leyendo hasta llegar al final del archivo
	//z80_long_int puntero_lectura=0;
	long long int puntero_lectura=0;

	char buffer_bloque[512];
	//char buffer_bloque2[512];
	//char buffer_bloque3[512];

	int salir=0;

	while (puntero_lectura<filesize && !salir) {

		if (indice_buffer>=MAX_TEXTO_BROWSER-1024) {
			debug_printf(VERBOSE_ERR,"Too many entries. Showing only what is allowed on memory");
			//printf ("bucle inicial %d %d %d\n",indice_buffer,MAX_TEXTO_BROWSER,MAX_TEXTO_GENERIC_MESSAGE);
			salir=1;
			break;
		}

		/*
		Leer datos identificador de bloque
		offset type     name   meaning
		0      u32      tag    unique identifier for the block type.
		4      u32      size   size of the block in bytes, excluding the tag and size fields themselves.
		8      u8[size] data   arbitrary amount of block data.
		*/

		char tag_name[5];
		tag_name[0]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[1]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[2]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[3]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[4]=0;

		z80_long_int block_size;



		block_size=pzx_file_mem[puntero_lectura]+
					(pzx_file_mem[puntero_lectura+1]*256)+
					(pzx_file_mem[puntero_lectura+2]*65536)+
					(pzx_file_mem[puntero_lectura+3]*16777216);
		puntero_lectura +=4;

		//printf ("Block tag name: [%s] size: [%u]\n",tag_name,block_size);

		sprintf(buffer_bloque,"%s Block",tag_name);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);


		//Tratar cada tag
		if (!strcmp(tag_name,"PZXT")) {
			z80_byte pzx_version_major=pzx_file_mem[puntero_lectura];
        	z80_byte pzx_version_minor=pzx_file_mem[puntero_lectura+1];

        	sprintf(buffer_bloque," file version: %d.%d",pzx_version_major,pzx_version_minor);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

			z80_byte *memoria;
			memoria=&pzx_file_mem[puntero_lectura];


			int block_size_tag=block_size;
			block_size_tag -=2;
			memoria +=2;

			//Los strings los vamos guardando en un array de char separado. Asumimos que ninguno ocupa mas de 1024. Si es asi, esto petara...

			char text_string[1024];
			int index_string=0;

			while (block_size_tag>0) {
					char caracter=*memoria;

					if (caracter==0) {
							text_string[index_string++]=0;
							//fin de cadena
							sprintf(buffer_bloque," %s",text_string);
							indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);
							index_string=0;
					}

					else {
							text_string[index_string++]=util_return_valid_ascii_char(caracter);
					}

					memoria++;
					block_size_tag--;

			}

			//Final puede haber acabado con byte 0 o no. Lo metemos por si acaso
			if (index_string!=0) {
					text_string[index_string++]=0;
					sprintf(buffer_bloque," %s",text_string);
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);
			}



		}

		else if (!strcmp(tag_name,"PULS")) {
				//convert_pzx_to_rwa_tag_puls(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);

				z80_byte *memoria;
				memoria=&pzx_file_mem[puntero_lectura];

			z80_int count;
			int duration;

			//int valor_pulso_inicial=0;
			//int t_estado_actual=*p_t_estado_actual;

			int block_size_tag=block_size;

			while (block_size_tag>0 && !salir) {

					count = 1 ;

					//duration = fetch_u16() ;
					duration = (*memoria)|((memoria[1])<<8);
					memoria +=2;
					block_size_tag -=2;

					if ( duration > 0x8000 ) {
							count = duration & 0x7FFF ;
							//duration = fetch_u16() ;
							duration = (*memoria)|((memoria[1])<<8);
							memoria +=2;
							block_size_tag -=2;
					}
					if ( duration >= 0x8000 ) {
							duration &= 0x7FFF ;
							duration <<= 16 ;
							//duration |= fetch_u16() ;
							duration |= (*memoria)|((memoria[1])<<8);
							memoria +=2;
							block_size_tag -=2;
					}

					//printf ("count: %d duration: %d\n",count,duration);
					//printf("size: %d count: %d duration: %d\n",block_size_tag,count,duration);

					sprintf(buffer_bloque," count: %d duration: %d",count,duration);
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

					//Proteccion aqui tambien porque pueden generarse muchos bloques en este bucle
					if (indice_buffer>=MAX_TEXTO_BROWSER-1024) {
							debug_printf(VERBOSE_ERR,"Too many entries. Showing only what is allowed on memory");
							salir=1;
							break;
					}


			}



		}

		else if (!strcmp(tag_name,"DATA")) {
				//convert_pzx_to_rwa_tag_data(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);

				z80_byte *memoria;
				memoria=&pzx_file_mem[puntero_lectura];

  				int initial_pulse;

        z80_long_int count;

        //int t_estado_actual=*p_t_estado_actual;


        count=memoria[0]+
                (memoria[1]*256)+
                (memoria[2]*65536)+
                ((memoria[3]&127)*16777216);

        initial_pulse=(memoria[3]&128)>>7;

        memoria +=4;

        z80_int tail=memoria[0]+
                (memoria[1]*256);

        memoria +=2;

        z80_byte num_pulses_zero=*memoria;
        memoria++;

        z80_byte num_pulses_one=*memoria;
        memoria++;

        //Secuencias que identifican a un cero y un uno
        //z80_int seq_pulses_zero[256];
        //z80_int seq_pulses_one[256];

        //Metemos las secuencias de 0 y 1 en array
        int i;
        for (i=0;i<num_pulses_zero;i++) {
               //seq_pulses_zero[i]=memoria[0]+(memoria[1]*256);

                memoria +=2;
        }


        for (i=0;i<num_pulses_one;i++) {
               //seq_pulses_one[i]=memoria[0]+(memoria[1]*256);

                memoria +=2;
        }




        //Procesar el total de bits
        //int bit_number=7;
        //z80_byte processing_byte;

        //z80_int *sequence_bit;
        //int longitud_sequence_bit;

        //z80_long_int total_bits_read;




			//Saltamos flag
			z80_byte flag=*memoria;
			memoria++;

			//Metemos un espacio delante
			buffer_bloque[0]=' ';
			util_tape_get_info_tapeblock(memoria,flag,count/8,&buffer_bloque[1]);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);


			//Y debug del bloque
		    sprintf(buffer_bloque," Length: %d (%d bits)",count/8,count);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

        	sprintf(buffer_bloque," count: %d initial_pulse: %d tail: %d num_pulses_0: %d num_pulses_1: %d",
            		count,initial_pulse,tail,num_pulses_zero,num_pulses_one);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);


		}

		else if (!strcmp(tag_name,"PAUS")) {
				//convert_pzx_to_rwa_tag_paus(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);

			//int initial_pulse;

			z80_long_int count;

							z80_byte *memoria;
					memoria=&pzx_file_mem[puntero_lectura];




			count=memoria[0]+
					(memoria[1]*256)+
					(memoria[2]*65536)+
					((memoria[3]&127)*16777216);

			//initial_pulse=(memoria[3]&128)>>7;

			memoria +=4;

			sprintf(buffer_bloque," count: %d",count);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);


		}

		else {
			//debug_printf (VERBOSE_DEBUG,"PZX: Unknown block type: %02XH %02XH %02XH %02XH. Skipping it",
			//    tag_name[0],tag_name[1],tag_name[2],tag_name[3]);
		}


		//Y saltar al siguiente bloque
		puntero_lectura +=block_size;

	}


	texto_browser[indice_buffer]=0;
	zxvision_generic_message_tooltip("PZX file viewer" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	free(pzx_file_mem);

    free(texto_browser);

}




void menu_file_tzx_browser_show(char *filename)
{

	int filesize=get_file_size(filename);
	z80_byte *tzx_file_mem;

	tzx_file_mem=malloc(filesize);
	if (tzx_file_mem==NULL) {
		debug_printf(VERBOSE_ERR,"Can not allocate memory for tzx browser");
		return;
	}


	//Leemos cabecera archivo tzx
        FILE *ptr_file_tzx_browser;

        //Soporte para FatFS
        FIL fil;        /* File object */

        int in_fatfs;

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_tzx_browser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return;
        }
        /*
        ptr_file_tzx_browser=fopen(filename,"rb");

        if (!ptr_file_tzx_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
    */

        int leidos;

        leidos=zvfs_fread(in_fatfs,tzx_file_mem,filesize,ptr_file_tzx_browser,&fil);
        //leidos=fread(tzx_file_mem,1,filesize,ptr_file_tzx_browser);

	if (leidos!=filesize) {
                debug_printf(VERBOSE_ERR,"Error reading file");
		free(tzx_file_mem);
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_tzx_browser,&fil);
        //fclose(ptr_file_tzx_browser);

        //Testear cabecera "ZXTape!" en los primeros bytes
	char signature[8];
	memcpy(signature,tzx_file_mem,7);
	signature[7]=0;
        if (strcmp(signature,"ZXTape!")) {
        	debug_printf(VERBOSE_ERR,"Invalid .TZX file");
		free(tzx_file_mem);
        	return;
        }

	char buffer_texto[300]; //Para poder contener info de tzx extensa

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

	//TODO. controlar que no se salga del maximo de texto_browser

	z80_byte tzx_version_major=tzx_file_mem[8];
	z80_byte tzx_version_minor=tzx_file_mem[9];
	sprintf(buffer_texto,"TZX File version: %d.%d",tzx_version_major,tzx_version_minor);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);




	int puntero=10;
	int salir=0;

	for (;puntero<filesize && !salir;) {
		z80_byte tzx_id=tzx_file_mem[puntero++];
		z80_int longitud_bloque,longitud_sub_bloque;
		int longitud_larga;
		char buffer_bloque[256];

		//printf ("ID %02XH puntero: %d indice_buffer: %d\n",tzx_id,puntero,indice_buffer);
		//Aproximacion a llenado de buffer
		if (indice_buffer>=MAX_TEXTO_BROWSER-1024) {
			debug_printf(VERBOSE_ERR,"Too many entries. Showing only what is allowed on memory");
			salir=1;
			break;
		}
		//int subsalir;

		switch (tzx_id) {

			case 0x10:

				//Es un poco molesto mostrar el texto cada vez
				//sprintf(buffer_texto,"ID 10 Std. Speed Data Block");
				//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


				puntero+=2;


			        longitud_sub_bloque=util_tape_tap_get_info(&tzx_file_mem[puntero],buffer_bloque,0);
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);


				puntero +=longitud_sub_bloque;



			break;


			case 0x11:


				sprintf(buffer_texto,"ID 11 - Turbo Data Block:");
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

				longitud_larga=tzx_file_mem[puntero+15]+256*tzx_file_mem[puntero+16]+65536*tzx_file_mem[puntero+17];
				//printf("longitud larga: %d\n",longitud_larga);

				//Longitud en este tipo de bloque es de 3 bytes, saltamos el primero para que la rutina
				//de obtener cabecera de tap siga funcionando
				puntero+=18;

				//Lo escribimos con espacio
				buffer_bloque[0]=' ';

				util_tape_get_info_tapeblock(&tzx_file_mem[puntero+1],tzx_file_mem[puntero],longitud_larga,&buffer_bloque[1]);

				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

				puntero +=longitud_larga;



			break;



			case 0x20:


				sprintf(buffer_texto,"ID 20 - Pause");
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


				puntero+=2;



			break;

			case 0x30:
				sprintf(buffer_texto,"ID 30 Text description:");
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

				longitud_bloque=tzx_file_mem[puntero];
				//printf ("puntero: %d longitud: %d\n",puntero,longitud_bloque);
				util_binary_to_ascii(&tzx_file_mem[puntero+1],buffer_bloque,longitud_bloque,longitud_bloque);
                sprintf (buffer_texto," %s",buffer_bloque);
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

				puntero+=1;
				puntero+=longitud_bloque;
				//printf ("puntero: %d\n",puntero);
			break;

                        case 0x31:
                                sprintf(buffer_texto,"ID 31 Message block:");
                                indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                longitud_bloque=tzx_file_mem[puntero+1];
                                util_binary_to_ascii(&tzx_file_mem[puntero+2],buffer_bloque,longitud_bloque,longitud_bloque);
                                indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

                                puntero+=2;
                                puntero+=longitud_bloque;
                        break;

                        case 0x32:
                                sprintf(buffer_texto,"ID 32 Archive info:");
                                indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                longitud_bloque=tzx_file_mem[puntero]+256*tzx_file_mem[puntero+1];
                                puntero+=2;

                                z80_byte nstrings=tzx_file_mem[puntero++];

                                char buffer_text_id[256];
                                char buffer_text_text[256];

                                for (;nstrings;nstrings--) {
                                	z80_byte text_type=tzx_file_mem[puntero++];
                                	z80_byte longitud_texto=tzx_file_mem[puntero++];
                                	tape_tzx_get_archive_info(text_type,buffer_text_id);
                                	util_binary_to_ascii(&tzx_file_mem[puntero],buffer_text_text,longitud_texto,longitud_texto);
                                	sprintf (buffer_texto," %s: %s",buffer_text_id,buffer_text_text);

                                	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                	puntero +=longitud_texto;
                                }

                        break;

                        case 0x33:
                            //ID 33 - Hardware type
                            //0x00	N	BYTE	Number of machines and hardware types for which info is supplied
                            //0x01	-	HWINFO[N]	List of machines and hardware
                            //length: [00]*03+01
                            sprintf(buffer_texto,"ID 33 Hardware type");
                            indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
                            //TODO de momento lo saltamos sin mostrar mucha info
                            longitud_bloque=(tzx_file_mem[puntero]*3)+1;
                            puntero+=longitud_bloque;
                        break;

			default:
				sprintf(buffer_texto,"Unhandled TZX ID %02XH",tzx_id);
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
				salir=1;
			break;
		}
	}

	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("TZX file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("TZX file viewer" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	free(tzx_file_mem);
    free(texto_browser);

}


//indice_bloque_actual se usa por ejemplo en cinta de entrada para decirle cual es el siguiente bloque a leer,
//y se visualiza en el visor con un ">". Si vale <0, no se visualizara nunca
//Retorna valor>=0 si el usuario ha pulsado enter en alguna linea, lo cual pretende hacer seek a ese bloque
int menu_tape_browser_show(char *filename,int indice_bloque_actual)
{

	//Si tzx o cdt
	if (!util_compare_file_extension(filename,"tzx") ||
	    !util_compare_file_extension(filename,"cdt")
		) {
		menu_file_tzx_browser_show(filename);
		return -1;
	}

	//Si pzx
	if (!util_compare_file_extension(filename,"pzx")
		) {
		menu_file_pzx_browser_show(filename);
		return -1;
	}

	//Si cas
	if (!util_compare_file_extension(filename,"cas")
		) {
		menu_file_cas_browser_show(filename);
		return -1;
	}

    //wav, rwa, etc
	if (!util_compare_file_extension(filename,"wav") ||
        !util_compare_file_extension(filename,"smp") ||
        !util_compare_file_extension(filename,"rwa")
		) {
		menu_file_realtape_browser_show(filename);
		return -1;
	}



    //ZX80 O, ZX81 P
    if (!util_compare_file_extension(filename,"p") || !util_compare_file_extension(filename,"p81")) {
        menu_file_p_browser_show(filename);
        return -1;
    }

    if (!util_compare_file_extension(filename,"o")) {
        menu_file_o_browser_show(filename);
        return -1;
    }


	//tapefile
	if (util_compare_file_extension(filename,"tap")!=0) {
		debug_printf(VERBOSE_ERR,"Tape browser not supported for this tape type");
		return -1;
	}

	//Leemos cinta en memoria
	int total_mem=get_file_size(filename);

	z80_byte *taperead;



        FILE *ptr_tapebrowser;

   //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("menu_tape_browser_show %s\n",filename);

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open tape for browsing");
        return -1;
    }


    /*
        ptr_tapebrowser=fopen(filename,"rb");

        if (!ptr_tapebrowser) {
		debug_printf(VERBOSE_ERR,"Unable to open tape for browsing");
		return;
	}
    */


	taperead=malloc(total_mem);
	if (taperead==NULL) cpu_panic("Error allocating memory for tape browser");

	z80_byte *puntero_lectura;
	puntero_lectura=taperead;


        int leidos;

        leidos=zvfs_fread(in_fatfs,taperead,total_mem,ptr_tapebrowser,&fil);

        //leidos=fread(taperead,1,total_mem,ptr_tapebrowser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
                return -1;
        }

        zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);
        //fclose(ptr_tapebrowser);

	char buffer_texto[70];

	int longitud_bloque;

	int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

    int offset_debug=0;

    int id_bloque_leido=0;

	while(total_mem>0) {
		longitud_bloque=util_tape_tap_get_info(puntero_lectura,&buffer_texto[1],1);

        //dejamos un espacio para poder indicar, si conviene, donde está el puntero actual de lectura
        if (id_bloque_leido==indice_bloque_actual) buffer_texto[0]='>';
        else buffer_texto[0]=32;
		total_mem-=longitud_bloque;
		puntero_lectura +=longitud_bloque;
		debug_printf (VERBOSE_DEBUG,"Tape browser. Offset %d Block: %s",offset_debug,buffer_texto);

        offset_debug +=longitud_bloque;


     //printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);
                        longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
                        if (indice_buffer+longitud_texto>MAX_TEXTO_BROWSER-1) {
				debug_printf (VERBOSE_ERR,"Too much headers. Showing only the allowed in memory");
				total_mem=0; //Finalizar bloque
			}

                        else {
                                sprintf (&texto_browser[indice_buffer],"%s\n",buffer_texto);
                                indice_buffer +=longitud_texto;
                        }

        id_bloque_leido++;

	}

    strcpy(buffer_texto," End of Tape");

    if (id_bloque_leido==indice_bloque_actual) {
        buffer_texto[0]='>';
    }

    longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
    sprintf (&texto_browser[indice_buffer],"%s\n",buffer_texto);
    indice_buffer +=longitud_texto;

	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("Tape browser", 0, 0, 1, NULL, "%s", texto_browser);

    generic_message_tooltip_return retorno_ventana;

	zxvision_generic_message_tooltip("Tape viewer" , 0 , 0, 0, 1, &retorno_ventana, 1, "%s", texto_browser);



	free(taperead);
    free(texto_browser);


	//Si se sale con ESC
    if (retorno_ventana.estado_retorno==0) return -1;


	return retorno_ventana.linea_seleccionada;


}

void menu_hilow_datadrive_browser_aux_get_label(z80_byte *orig,char *dest)
{
    memcpy(dest,orig,9);
    dest[9]=0;
}

/*int menu_hilow_datadrive_browser_get_file_offset(int indice_archivo)
{
    return hilow_util_get_file_offset(indice_archivo);
}
*/

int menu_hilow_datadrive_browser_get_total_files(z80_byte *puntero_memoria)
{
    int max=hilow_util_get_total_files(0,puntero_memoria);

    if (max>HILOW_MAX_FILES_DIRECTORY) max=HILOW_MAX_FILES_DIRECTORY;

    return max;


}

void menu_hilow_datadrive_browser_get_name_info(int indice_archivo,z80_byte *puntero_memoria,char *buffer_file_name,char *buffer_file_info)
{

    int offset_archivo=hilow_util_get_file_offset(indice_archivo);


    //Maximo 17 bytes. Copiamos a buffer temporal para evitar que se salga puntero de sitio
    z80_byte buffer_temp[17];
    util_memcpy_protect_origin(buffer_temp, &puntero_memoria[offset_archivo], 17, 0, 17);
    util_tape_get_info_tapeblock(buffer_temp,0,19,buffer_file_name);

    //printf("Archivo: %s\n",buffer_file_name);


    z80_int cabecera_longitud=value_8_to_16(puntero_memoria[offset_archivo+12],puntero_memoria[offset_archivo+11]);
    z80_int cabecera_inicio=value_8_to_16(puntero_memoria[offset_archivo+14],puntero_memoria[offset_archivo+13]);
    z80_int cabecera_aux=value_8_to_16(puntero_memoria[offset_archivo+16],puntero_memoria[offset_archivo+15]);



    sprintf(buffer_file_info," Start: %d Lenght: %d Aux: %d",cabecera_inicio,cabecera_longitud,cabecera_aux);
}

int menu_hilow_datadrive_browser_get_sectors_file(int indice_archivo,z80_byte *puntero_memoria,int *sectores)
{

   return hilow_util_get_sectors_file(0,indice_archivo,puntero_memoria,sectores);

}

#define HILOW_BROWSER_MAPA_SECTORS_LINEA 50
void menu_hilow_datadrive_browser_get_xy_mapa_sector(int sector,int *x,int *y)
{
    *y=sector/HILOW_BROWSER_MAPA_SECTORS_LINEA;
    *x=sector % HILOW_BROWSER_MAPA_SECTORS_LINEA;
}

int hilow_browser_inicio_y_mapa=15;

void menu_hilow_browser_print_char_sector(zxvision_window *w,int sector,char caracter)
{
    int xmapa,ymapa;

    menu_hilow_datadrive_browser_get_xy_mapa_sector(sector,&xmapa,&ymapa);

    zxvision_print_string_defaults_format(w,xmapa+1,ymapa+hilow_browser_inicio_y_mapa,"%c",caracter);
}

int hilow_browser_fragmentation_total_sectors=0;
int hilow_browser_fragmentation_fragmented_sectors=0;

void menu_hilow_browser_print_used_sectors(zxvision_window *w,z80_byte *puntero_memoria,int total_files)
{
    //hasta 27 sectores (HILOW_MAX_SECTORS_PER_FILE)
    int sectores[HILOW_MAX_SECTORS_PER_FILE];

    int current_file;

    hilow_browser_fragmentation_total_sectors=0;
    hilow_browser_fragmentation_fragmented_sectors=0;


    for (current_file=0;current_file<total_files;current_file++) {

            int total_sectores=menu_hilow_datadrive_browser_get_sectors_file(current_file,puntero_memoria,sectores);

            int i;
            z80_byte sector_anterior=255;

            int parcial_fragmentacion=0;

            for (i=0;i<total_sectores;i++) {
                z80_byte sector_actual=sectores[i];

                //Id de sectores usados empieza por el 1
                //No mostrar sector si es menor que 3
                if (sector_actual>=3) menu_hilow_browser_print_char_sector(w,sector_actual-1,'u');

                //sectores en cara B se numeran igual en orden pero con bit 7 alzado
                //Considerar por ejemplo sector 3 y 131, ambos son consecutivos
                if (i>0) {
                    if (
                    sector_actual!=sector_anterior+1 &&
                    (sector_actual & 127)!=(sector_anterior & 127)
                    )
                    {
                        parcial_fragmentacion++;
                    }
                }


                sector_anterior=sector_actual;

                hilow_browser_fragmentation_total_sectors++;
            }

            if (parcial_fragmentacion) {
                //Si hay fragmentacion, minimo es 2:
                //2 sectores que están no consecutivos, consideramos que son 2 sectores fragmentados
                //3 sectores que están no consecutivos, consideramos que son 3 sectores fragmentados
                //1 sector fragmentado en cambio no tiene sentido, o es 0, o es a partir de 2
                parcial_fragmentacion++;
            }


            hilow_browser_fragmentation_fragmented_sectors +=parcial_fragmentacion;

    }

}

void menu_hilow_datadrive_browser(z80_byte *puntero_memoria_orig)
{

    //Preguntar si ver copia del sector 0 o 1
    /*int opcion=menu_simple_two_choices("Directory browser","You want to see","Sector 0","Sector 1");

    if (opcion==0) return; //ESC


    z80_byte *puntero_memoria;

    puntero_memoria=puntero_memoria_orig;

    if (opcion==2) {
        //Ir al siguiente sector
        puntero_memoria +=HILOW_SECTOR_SIZE;
    }*/

    int ancho=52;
    int alto=22;
    int x=menu_center_x()-ancho/2;
    int y=menu_center_y()-alto/2;


    zxvision_window ventana;

    zxvision_new_window(&ventana,x,y,ancho,alto,ancho-1,alto-2,"Hilow Data Drive browser");

    ventana.can_mouse_send_hotkeys=1;

    zxvision_draw_window(&ventana);

    /*
Label: cinta
Usage counter: 714
Free sectors: 89 (178 KB)

Screen$ .pant
 Start: 16384 Lenght: 6912 Aux: 32990
 Sectors (4): 129 130 131 132

Program .prog
 Start: 32800 Lenght: 222 Aux: 222
 Sectors (1):   2

Maximo sectores por archivo: 25

    0123456789012345678901234567890123456789012345678901
0    Label: cinta
1    Usage counter: 714
2    Free sectors: 89 (178 KB)
3    File: 11/20 Next File Previous File
4    Directory sector: 0
5    Fragmentation: 30%
6
7    File: Screen$ .pant
8    Start: 16384 Lenght: 6912 Aux: 32990
9    Sectors (25): 255 255 255 255 255 255 255 255
10                 255 255 255 255 255 255 255 255
11                 255 255 255 255 255 255 255 255
12                 255
13
14   Legend
15   uu................................................
16   ..................................................
17   .............XXXX.................................
18   ..................................................
19   ..................................................

    . : libre
    u : usado
    X : usado por ese archivo

    */

   z80_byte tecla;
   int current_file=0;
   int sector_directorio=0;

    do {

        z80_byte *puntero_memoria;

        puntero_memoria=puntero_memoria_orig;

        if (sector_directorio) {
            //Ir al siguiente sector
            puntero_memoria +=HILOW_SECTOR_SIZE;
        }

        int linea=0;

        zxvision_cls(&ventana);

        //Forzar a mostrar atajos
        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        menu_writing_inverse_color.v=1;


        char buffer_file_label[10];
        menu_hilow_datadrive_browser_aux_get_label(&puntero_memoria[2],buffer_file_label);
        buffer_file_label[9]=0;
        zxvision_print_string_defaults_format(&ventana,1,linea++,"Label: %s",buffer_file_label);

        z80_int usage_counter=hilow_util_get_usage_counter(0,puntero_memoria);
        zxvision_print_string_defaults_format(&ventana,1,linea++,"Usage counter: %d",usage_counter);

        z80_byte free_sectors=hilow_util_get_free_sectors(0,puntero_memoria);
        zxvision_print_string_defaults_format(&ventana,1,linea++,"Free sectors: %d (%d KB)",free_sectors,(free_sectors*HILOW_SECTOR_SIZE)/1024);

        zxvision_print_string_defaults_format(&ventana,1,linea++,"~~Directory sector: %d",sector_directorio);

        //Espacio para fragmentacion
        int linea_fragmentacion=linea;
        linea++;

        //Y linea en blanco
        linea++;

        int total_files=menu_hilow_datadrive_browser_get_total_files(puntero_memoria);


        if (!total_files) {
            zxvision_print_string_defaults_format(&ventana,1,linea++,"No files");
        }
        else {

            zxvision_print_string_defaults_format(&ventana,1,linea++,"File: %2d/%2d ~~Next ~~Previous",current_file+1,total_files);


            char buffer_file_name[100];
            char buffer_file_info[100];

            menu_hilow_datadrive_browser_get_name_info(current_file,puntero_memoria,buffer_file_name,buffer_file_info);

            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_file_name);
            zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_file_info);

            //hasta 27 sectores (HILOW_MAX_SECTORS_PER_FILE)
            int sectores[HILOW_MAX_SECTORS_PER_FILE];
            //cada sector: 000 -> 4 caracteres
            //#define HILOW_BUFFER_SECTORS (HILOW_MAX_SECTORS_PER_FILE*4)

            //char buffer_sectors[HILOW_BUFFER_SECTORS*2]; //mas que suficiente (espero)

            int total_sectores=menu_hilow_datadrive_browser_get_sectors_file(current_file,puntero_memoria,sectores);

            zxvision_print_string_defaults_fillspc_format(&ventana,1,linea,"Sectors (%2d): ",total_sectores);

            //Inicializar mapa de sectores, de momento todos a "."

            int col=0;
            int i;

            for (i=0;i<HILOW_MAX_SECTORS;i++) {
                menu_hilow_browser_print_char_sector(&ventana,i,'.');
            }


            //mostrar los usados con "u". Y calcular fragmentacion
            menu_hilow_browser_print_used_sectors(&ventana,puntero_memoria,total_files);

            //Y el 0 y el 1 los usa las dos copias de directorio
            menu_hilow_browser_print_char_sector(&ventana,0,'D');
            menu_hilow_browser_print_char_sector(&ventana,1,'D');

            //linea fragmentacion
            //int hilow_browser_fragmentation_total_sectors=0;
            //int hilow_browser_fragmentation_fragmented_sectors=0;
            int porcentaje_frag=0;


            if (hilow_browser_fragmentation_total_sectors!=0) {
                porcentaje_frag=(hilow_browser_fragmentation_fragmented_sectors*100)/hilow_browser_fragmentation_total_sectors;
            }

            zxvision_print_string_defaults_fillspc_format(&ventana,1,linea_fragmentacion,"Fragmentation: %d%%",porcentaje_frag);


            //escribir lista de sectores usados por ese archivo
            int columna_sectores=14;
            int sectores_por_linea=8;




            col=0;

            for (i=0;i<total_sectores;i++) {

                zxvision_print_string_defaults_format(&ventana,columna_sectores+col,linea,"%3d ",sectores[i]);

                //Id de sectores empieza a numerar en 1
                int sector_file=sectores[i];
                //No mostrar sector si es menor que 3
                if (sector_file>=3) menu_hilow_browser_print_char_sector(&ventana,sector_file-1,'F');


                if ((i+1) % sectores_por_linea == 0) {
                    linea++;
                    col=0;
                }
                else {
                    col +=4;
                }
            }


            zxvision_print_string_defaults(&ventana,1,hilow_browser_inicio_y_mapa-1,"Legend: [D]irectory [u]sed [F]ile [.]free");
        }

        //Restaurar comportamiento atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

        zxvision_draw_window_contents(&ventana);


		tecla=zxvision_common_getkey_refresh();


		switch (tecla) {

			case 'p':
				if (current_file>0) current_file--;
			break;

			case 'n':
                if (current_file<total_files-1) current_file++;
            break;

            case 'd':
                sector_directorio ^=1;
                current_file=0;
            break;
		}



	} while (tecla!=2 && tecla!=3);


    zxvision_destroy_window(&ventana);

    return;


}




void menu_file_ddh_browser_show(char *filename)
{



    //Leer solo sector 0 y 1
	long long int bytes_to_load=HILOW_SECTOR_SIZE*2;

	z80_byte *ddh_file_memory;
	ddh_file_memory=malloc(bytes_to_load);
	if (ddh_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}

	//Leemos archivo
    FILE *ptr_file_ddh_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("menu_file_p_browser_show %s\n",filename);

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_ddh_browser,&fil)<0) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(ddh_file_memory);
    }



    int leidos;

    leidos=zvfs_fread(in_fatfs,ddh_file_memory,bytes_to_load,ptr_file_ddh_browser,&fil);

	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading file");
        return;
    }

    zvfs_fclose(in_fatfs,ptr_file_ddh_browser,&fil);

    //Permitir mostrar caracteres hotkeys, los desactiva desde menu_file_viewer_read_file
    int antes_menu_disable_special_chars=menu_disable_special_chars.v;
    menu_disable_special_chars.v=0;

    menu_hilow_datadrive_browser(ddh_file_memory);

    menu_disable_special_chars.v=antes_menu_disable_special_chars;

	free(ddh_file_memory);


}


z80_byte *last_bas_browser_memory;
int last_bas_browser_memory_size=1;

z80_byte menu_file_bas_browser_show_peek(z80_int dir)
{
	return last_bas_browser_memory[dir % last_bas_browser_memory_size]; //con % para no salirnos de la memoria
}


void menu_file_basic_browser_show(char *filename)
{

	//Leemos archivo .bas
        FILE *ptr_file_bas_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("menu_file_basic_browser_show %s\n",filename);

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_bas_browser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open file");
        return;
    }
    /*
        ptr_file_bas_browser=fopen(filename,"rb");

        if (!ptr_file_bas_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
    */

	int tamanyo=get_file_size(filename);
    //printf("tamanyo: %d\n",tamanyo);


	z80_byte *memoria;
	memoria=malloc(tamanyo);
	if (memoria==NULL) cpu_panic ("Can not allocate memory for bas read");

	last_bas_browser_memory_size=tamanyo;

	last_bas_browser_memory=memoria;

	//Leer archivo


    int leidos;

    //leidos=fread(memoria,1,tamanyo,ptr_file_bas_browser);
    leidos=zvfs_fread(in_fatfs,memoria,tamanyo,ptr_file_bas_browser,&fil);

	if (leidos!=tamanyo) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
    }

    zvfs_fclose(in_fatfs,ptr_file_bas_browser,&fil);
    //fclose(ptr_file_bas_browser);

    char *results_buffer=util_malloc_max_texto_generic_message("Can not allocate memory for reading the file");

	char titulo_ventana[33];
	strcpy(titulo_ventana,"View Basic");

  	char **dir_tokens;
  	int inicio_tokens;

	int dir_inicio_linea=0;

	int tipo=0; //Asumimos spectrum

    //printf("antes de ver extension\n");

	//Si es basic zx81
	if (!util_compare_file_extension(filename,"baszx81")) {

                //ZX81
                //dir_inicio_linea=116; //16509-0x4009
				dir_inicio_linea=0;

                //D_FILE
                //final_basic=peek_byte_no_time(0x400C)+256*peek_byte_no_time(0x400D);

                dir_tokens=zx81_rom_tokens;

                inicio_tokens=192;

				tipo=2;

				strcpy(titulo_ventana,"View ZX81 Basic");

	}

	else if (!util_compare_file_extension(filename,"baszx80")) {

                //ZX80
            //ZX80
                  //dir_inicio_linea=40; //16424-16384
				  dir_inicio_linea=0;


                  //VARS
                  //final_basic=peek_byte_no_time(0x4008)+256*peek_byte_no_time(0x4009);

                  dir_tokens=zx80_rom_tokens;

                  inicio_tokens=213;

                tipo=1;

		strcpy(titulo_ventana,"View ZX80 Basic");

	}

	//Si extension z88 o si firma de final de archivo parece ser .z88
	else if (!util_compare_file_extension(filename,"basz88") || file_is_z88_basic(filename)) {
		dir_inicio_linea=0;
		debug_view_z88_basic_from_memory(results_buffer,dir_inicio_linea,tamanyo,menu_file_bas_browser_show_peek);

  		menu_generic_message_format("View Z88 Basic","%s",results_buffer);
		free(memoria);
	//void debug_view_z88_basic_from_memory(char *results_buffer,int dir_inicio_linea,int final_basic,
	//z80_byte (*lee_byte_function)(z80_int dir) )
		//Este finaliza aqui
        free(results_buffer);
		return;
	}

	else {
		//O es texto tal cual o es tokens de spectrum
		//.bas , .b

		//Deducimos si es un simple .bas de texto normal, o es de basic spectrum
		//Comprobacion facil, primeros dos bytes contiene numero de linea. Asumimos que si los dos son caracteres ascii imprimibles, son de texto
		//Y siempre que extension no sea .B (en este caso es spectrum con tokens)

        //Tambien contemplar archivos .bas con tokens pero con la cabecera de PLUS3DOS delante
        //Estos son los archivos .bas generados por ejemplo desde Next y grabados en la mmc

        int indice_memoria=0;

        char *plus3dos_signature="PLUS3DOS";
        if (leidos>128) {
            if (!memcmp(plus3dos_signature,memoria,8)) {
                //Simplemente avanzamos el puntero del visor y decrementamos tamanyo
                last_bas_browser_memory +=128;
                last_bas_browser_memory_size -=128;
                indice_memoria +=128;
            }
        }

		if (leidos>2 && util_compare_file_extension(filename,"b") ) {
			z80_byte caracter1=memoria[indice_memoria];
			z80_byte caracter2=memoria[indice_memoria+1];
			if (caracter1>=32 && caracter1<=127 && caracter2>=32 && caracter2<=127) {
				//Es ascii. abrir visor ascii
				debug_printf(VERBOSE_INFO,".bas file type is guessed as simple text");
				free(memoria);
				menu_file_viewer_read_text_file("Basic file",filename);
				return;
			}
			else {
				debug_printf(VERBOSE_INFO,".bas file type is guessed as Spectrum/ZX80/ZX81");
				strcpy(titulo_ventana,"View ZX Spectrum Basic");
			}
		}




		//basic spectrum
  			dir_tokens=spectrum_rom_tokens;

  			inicio_tokens=163;
	}


	debug_view_basic_from_memory(results_buffer,dir_inicio_linea,tamanyo,dir_tokens,inicio_tokens,menu_file_bas_browser_show_peek,tipo,debug_view_basic_show_address.v,0);

  	menu_generic_message_format(titulo_ventana,"%s",results_buffer);
	free(memoria);
    free(results_buffer);

}




void menu_file_spg_browser_show(char *filename)
{

	//Leemos cabecera archivo spg


	//Leer  bytes de la cabecera
	int tamanyo_cabecera=sizeof(struct hdrSPG1_0);
	struct hdrSPG1_0 spg_header;

    int leidos=lee_archivo(filename,(char *)&spg_header,tamanyo_cabecera);

	if (leidos!=tamanyo_cabecera) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;


	if (memcmp(&spg_header.sign, "SpectrumProg", 12)) {
    		debug_printf(VERBOSE_ERR,"Unknown snapshot signature");
    		return;
    	}






        //z80_int spg_pc_reg=value_8_to_16(spg_header[31],spg_header[30]);
        //sprintf(buffer_texto,"PC Register: %04XH",spg_pc_reg);
 	//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);



    	z80_byte snap_type = spg_header.ver;

        if ((snap_type != 0) && (snap_type != 1) && (snap_type != 2) && (snap_type != 0x10)) {
      		debug_printf(VERBOSE_ERR,"Unknown snapshot type: %02XH",snap_type);
      		return;
      	}

 	sprintf(buffer_texto,"Machine: ZX-Evolution TS-Conf");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Snapshot type: %02XH",snap_type);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Author/Name: %s",spg_header.author);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Creator: %s",spg_header.creator);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Saved on: %d/%02d/%02d %02d:%02d:%02d ",
			2000+spg_header.year,spg_header.month,spg_header.day,spg_header.hour,spg_header.minute,spg_header.second);

	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	z80_int zx_pc_reg=spg_header.pc;
        sprintf(buffer_texto,"PC Register: %04XH",zx_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("SPG file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("SPG file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

    free(texto_browser);


}


void menu_file_nex_browser_show(char *filename)
{

	//Leemos cabecera archivo spg


	//Leer  bytes de la cabecera
	//buffer para la cabecera
	z80_byte nex_header[NEX_HEADER_SIZE];

    int leidos=lee_archivo(filename,(char *)nex_header,NEX_HEADER_SIZE);

	if (leidos!=NEX_HEADER_SIZE) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;


	char snap_version[5];
	//4	4	string with NEX file version, currently "V1.0", "V1.1" or "V1.2"
	snap_version[0]=nex_header[4];
	snap_version[1]=nex_header[5];
	snap_version[2]=nex_header[6];
	snap_version[3]=nex_header[7];
	snap_version[4]=0;


        //z80_int spg_pc_reg=value_8_to_16(spg_header[31],spg_header[30]);
        //sprintf(buffer_texto,"PC Register: %04XH",spg_pc_reg);
 	//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	sprintf(buffer_texto,"File version: %s",snap_version);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


    z80_int registro_leido;

    registro_leido=value_8_to_16(nex_header[15],nex_header[14]);
    sprintf(buffer_texto,"PC Register: %04XH",registro_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    registro_leido=value_8_to_16(nex_header[13],nex_header[12]);
    sprintf(buffer_texto,"SP Register: %04XH",registro_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    registro_leido=value_8_to_16(nex_header[141],nex_header[140]);
    sprintf(buffer_texto,"File handler: %04XH",registro_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    sprintf(buffer_texto,"RAM required: %dk",(nex_header[8] ? 1792 : 768) );
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    sprintf(buffer_texto,"16k Banks: %d",nex_header[9]);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    sprintf(buffer_texto,"Required core v. %d.%d.%d",nex_header[135],nex_header[136],nex_header[137]);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
    //135	3	Required core version, three bytes
    //0..15 "major", 0..15 "minor", 0..255 "subminor" version numbers.
    //(core version is checked only when reported machine-ID is 10 = "Next",
    //on other machine or emulator=8 the latest loaders will skip the check)


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("SPG file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("NEX file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

    free(texto_browser);


}




void menu_file_sna_browser_show(char *filename)
{

	//Leemos cabecera archivo sna
	//Leer 27 bytes de la cabecera
	z80_byte sna_header[27];
    /*
        FILE *ptr_file_sna_browser;
        ptr_file_sna_browser=fopen(filename,"rb");

        if (!ptr_file_sna_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}



        int leidos=fread(sna_header,1,27,ptr_file_sna_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_sna_browser);
    */


    lee_archivo(filename,(char *)sna_header,27);


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;


	int indice_buffer=0;



        //z80_int sna_pc_reg=value_8_to_16(sna_header[31],sna_header[30]);
        //sprintf(buffer_texto,"PC Register: %04XH",sna_pc_reg);
 	//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	//si 49179, snapshot de 48k
 	long long int tamanyo=get_file_size(filename);
 	if (tamanyo==49179) {
 		sprintf(buffer_texto,"Machine: Spectrum 48k");
 	}
 	else if (tamanyo==131103 || tamanyo==147487) {
 		sprintf(buffer_texto,"Machine: Spectrum 128k");
 	}

 	else {
 		debug_printf(VERBOSE_ERR,"Invalid .SNA file");
 		return;
 	}

    char *texto_browser=util_malloc_max_texto_browser();

	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_int registro_leido;

    /*
    registro_leido=value_8_to_16(sna_header[31],sp_header[30]);
    sprintf(buffer_texto,"PC Register: %04XH",registro_leido);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
     */

    registro_leido=value_8_to_16(sna_header[24],sna_header[23]);
    sprintf(buffer_texto,"SP Register: %04XH",registro_leido);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte im_leido=sna_header[25] & 3;
    sprintf(buffer_texto,"IM mode: %d",im_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte i_leido=sna_header[0];
    sprintf(buffer_texto,"I register: %02XH",i_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte ints_leido;
	if (sna_header[19] & 4) ints_leido=1;
	else ints_leido=0;
    sprintf(buffer_texto,"Interrupts: %s", (ints_leido ? "Enabled" : "Disabled"));
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);



	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("SNA file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("SNA file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


    free(texto_browser);

}



void menu_file_sms_browser_show(char *filename)
{

	//Leemos cabecera archivo sms
    FILE *ptr_file_z80_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_z80_browser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open file");
        return;
    }

    //Asignamos 32768 bytes (cabecera hasta 0x7fff)
    z80_byte *buffer_cabecera;

    buffer_cabecera=malloc(32768);

    if (buffer_cabecera==NULL) cpu_panic("Can not allocate memory for file read");


    int leidos;

    leidos=zvfs_fread(in_fatfs,buffer_cabecera,32768,ptr_file_z80_browser,&fil);


    if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading file");
        return;
    }

    zvfs_fclose(in_fatfs,ptr_file_z80_browser,&fil);

    char *texto_browser=util_malloc_max_texto_browser();

    //The header can be at offset $1ff0, $3ff0 or $7ff0 in the ROM,
    //although only the last of these seems to be used in known software.
    //The header is 16 bytes long.

    /*
    TMR SEGA ($7ff0, 8 bytes)
    The first eight bytes of the header are the ASCII text "TMR SEGA".
    The export Master System and Game Gear BIOSes require this to be present to indicate valid data.
    */


    char *signature="TMR SEGA";

    //Averiguar offset 0x1ff0, 0x3ff0 o 0x7ff0
    int offset=-1;
    if (!memcmp(signature,&buffer_cabecera[0x1ff0],8)) offset=0x1ff0;
    else if (!memcmp(signature,&buffer_cabecera[0x3ff0],8)) offset=0x3ff0;
    else if (!memcmp(signature,&buffer_cabecera[0x7ff0],8)) offset=0x7ff0;

    if (offset==-1) {
        menu_warn_message("No valid header found");
    }

    else {




    char buffer_texto[512];
	int indice_buffer=0;

	/*
Checksum ($7ffa, 2 bytes)
This little-endian word gives the ROM checksum for export SMS BIOSes. Game Gear and Japanese releases tend not to have a correct checksum there.
The BIOS checksum routines compare the calculated value to this to determine if the cartridge data is valid.

Product code ($7ffc, 2.5 bytes)
The first 2 bytes are a Binary Coded Decimal representation of the last four digits of the product code.
Hence, data 26 70 gives a product code 7026.
The high 4 bits of the next byte (hence, 0.5 bytes) are a hexadecimal representation of any remaining digits of the product code.
Hence, data 26 70 2 gives a product code of 27026 and 26 70 a gives a product code of 107026.

Version ($7ffe, 0.5 bytes)
The low 4 bits of the 15th byte of the header give a version number.
This is generally 0 for the first release and incremented for later revisions (which often have bugfixes).

Region code (0x7fff, 0.5 bytes)
The high 4 bits of the 16th byte of the header give the region and system for which the cartridge is intended:
Value	System/region
$3	SMS Japan
$4	SMS Export
$5	GG Japan
$6	GG Export
$7	GG International
Only the export SMS BIOS actually checks this.

ROM size (0x7fff, 0.5 bytes)
The final 4 bits give the ROM size, which may be used by the BIOS to determine the range over which to perform the checksum. Values are:
Value	Rom size	Comment
$a	8KB	Unused
$b	16KB	Unused
$c	32KB
$d	48KB	Unused, buggy
$e	64KB	Rarely used
$f	128KB
$0	256KB
$1	512KB	Rarely used
$2	1MB	Unused, buggy

	*/

 	sprintf(buffer_texto,"Checksum: %02X%02XH",buffer_cabecera[offset+0xa],buffer_cabecera[offset+0xb]);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

/*
Product code ($7ffc, 2.5 bytes)
The first 2 bytes are a Binary Coded Decimal representation of the last four digits of the product code.
Hence, data 26 70 gives a product code 7026.
The high 4 bits of the next byte (hence, 0.5 bytes) are a hexadecimal representation of any remaining digits of the product code.
Hence, data 26 70 2 gives a product code of 27026 and 26 70 a gives a product code of 107026.
*/
 	sprintf(buffer_texto,"Serial Number: %02X%02X",
        buffer_cabecera[offset+0xd],
        buffer_cabecera[offset+0xc]);

 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


 	sprintf(buffer_texto,"Software Revision: %d",buffer_cabecera[offset+0xe]);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte region_code=(buffer_cabecera[offset+0xf] >> 4) & 0xF;
    char buffer_region[32];
    strcpy(buffer_region,"Unknown");

    switch (region_code) {
        case 0x3:
            strcpy(buffer_region,"SMS Japan");
        break;

        case 0x4:
            strcpy(buffer_region,"SMS Export");
        break;

        case 0x5:
            strcpy(buffer_region,"GG Japan");
        break;

        case 0x6:
            strcpy(buffer_region,"GG Export");
        break;

        case 0x7:
            strcpy(buffer_region,"GG International");
        break;

    }

 	sprintf(buffer_texto,"Region: %d %s",region_code,buffer_region);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte rom_size=buffer_cabecera[offset+0xf] & 0xF;
    int total_rom_size=0;

    switch (rom_size) {


        case 0xa:
            total_rom_size=8;
        break;

        case 0xb:
            total_rom_size=16;
        break;

        case 0xc:
            total_rom_size=32;
        break;

        case 0xd:
            total_rom_size=48;
        break;

        case 0xe:
            total_rom_size=64;
        break;

        case 0xf:
            total_rom_size=128;
        break;

        case 0x0:
            total_rom_size=256;
        break;

        case 0x1:
            total_rom_size=512;
        break;

        case 0x2:
            total_rom_size=1024;
        break;


    }

 	sprintf(buffer_texto,"ROM size: %d KB",total_rom_size);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	texto_browser[indice_buffer]=0;
	zxvision_generic_message_tooltip("SMS file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);




    }

    free(buffer_cabecera);
    free(texto_browser);


}




void menu_file_col_browser_show(char *filename)
{

	//Leemos cabecera archivo col
        FILE *ptr_file_z80_browser;

        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;


        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_z80_browser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return;
        }

        /*
        ptr_file_z80_browser=fopen(filename,"rb");

        if (!ptr_file_z80_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
    */

	//Leemos primeros 256 bytes de cabecera
	z80_byte z80_header[256];

        int leidos;

        leidos=zvfs_fread(in_fatfs,z80_header,256,ptr_file_z80_browser,&fil);
        //leidos=fread(z80_header,1,256,ptr_file_z80_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        zvfs_fclose(in_fatfs,ptr_file_z80_browser,&fil);
		//fclose(ptr_file_z80_browser);

		char texto_info[256];
		char buffer_texto[512];

		//8000 - 8001:   If bytes are AAh and 55h, the CV will show a title screen
        //       and game name, etc.
        //       If bytes are 55h and AAh, the CV will jump directly to the
        //       start of code vector.


		if (z80_header[0]==0xAA && z80_header[1]==0x55) {

			//Offset a Texto info
			int i=0x24;

			//8024 - nnnn:   String with two delemiters "/" as "LINE2/LINE1/YEAR"

			int salir=0;




			int destino=0;

			int contador_barras=0;

			int digitos_anyos=0;

			for (;i<256 && !salir;i++) {
				z80_byte letra=z80_header[i];

				//Si dos barras, contar 4 digitos de anyos
				if (contador_barras>=2) {
					digitos_anyos++;

					//Fin
					if (digitos_anyos==4) salir=1;
				}

				//Contar cuantas barras division
				if (letra=='/') contador_barras++;

				//Evitar caracteres raros
				if (letra<32 || letra>126) letra='.';

				//Cada barra es un salto de linea
				if (letra=='/') letra='\n';

				texto_info[destino++]=letra;




			}

			texto_info[destino]=0;

		}

		else {
			strcpy(texto_info,"Unknown");
		}

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

	sprintf(buffer_texto,"Game description:\n%s\n",texto_info);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	/*
	8002 - 8003:   Pointer to RAM copy of the sprite name table
8004 - 8005:   Pointer to RAM sprite table
8006 - 8007:   Pointer to free buffer space in RAM
8008 - 8009:   Pointer to controller memory map
800A - 800B:   Pointer to start of code
	*/

 	sprintf(buffer_texto,"Sprite name table copy: %02X%02XH",z80_header[0x03],z80_header[0x02]);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	sprintf(buffer_texto,"Sprite table:           %02X%02XH",z80_header[0x05],z80_header[0x04]);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	sprintf(buffer_texto,"Free buffer space:      %02X%02XH",z80_header[0x07],z80_header[0x06]);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	sprintf(buffer_texto,"Controller memory map:  %02X%02XH",z80_header[0x09],z80_header[0x08]);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	sprintf(buffer_texto,"Code start:             %02X%02XH",z80_header[0x0b],z80_header[0x0a]);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("Z80 file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Colecovision file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

    free(texto_browser);


}




void menu_file_z80_browser_show(char *filename)
{

	//Leemos cabecera archivo z80
        FILE *ptr_file_z80_browser;

        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_z80_browser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return;
        }

        /*
        ptr_file_z80_browser=fopen(filename,"rb");

        if (!ptr_file_z80_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}
        */

	//Leemos primeros 30 bytes de la cabecera
	z80_byte z80_header[87];

        int leidos;

        leidos=zvfs_fread(in_fatfs,z80_header,30,ptr_file_z80_browser,&fil);
        //leidos=fread(z80_header,1,30,ptr_file_z80_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }




        //Ver si version 2 o 3
        z80_byte z80_version=1;

        z80_int z80_pc_reg=value_8_to_16(z80_header[7],z80_header[6]);

        if (z80_pc_reg==0) {
        	z80_version=2; //minimo 2

        	//Leer cabecera adicional
            zvfs_fread(in_fatfs,&z80_header[30],57,ptr_file_z80_browser,&fil);
        	//fread(&z80_header[30],1,57,ptr_file_z80_browser);

        	z80_pc_reg=value_8_to_16(z80_header[33],z80_header[32]);

        	if (z80_header[30]!=23) z80_version=3;
        }

        zvfs_fclose(in_fatfs,ptr_file_z80_browser,&fil);
        //fclose(ptr_file_z80_browser);


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char *texto_browser=util_malloc_max_texto_browser();
	int indice_buffer=0;

	sprintf(buffer_texto,"Z80 File version: %d",z80_version);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	//Maquina
 	if (z80_version==1) {
 		sprintf(buffer_texto,"Machine: Spectrum 48k");
 		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
 	}
 	else {
 		z80_byte z80_machine=z80_header[34];
 		if (z80_machine<7) {
 			if (z80_version==2) {
 				if (z80_machine==3 || z80_machine==4) {
 					z80_machine++;
 				}
 			}
 			sprintf(buffer_texto,"Machine: %s",z80file_machines_id[z80_machine]);
 		}

 		else if (z80_machine>=7 && z80_machine<=15) {
 			sprintf(buffer_texto,"Machine: %s",z80file_machines_id[z80_machine]);
 		}

 		else if (z80_machine==128) {
 			sprintf(buffer_texto,"Machine: TS2068");
 		}

 		else sprintf(buffer_texto,"Machine: Unknown");
 		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
 	}

 	sprintf(buffer_texto,"PC Register: %04XH",z80_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


    z80_int registro_leido;

    registro_leido=value_8_to_16(z80_header[9],z80_header[8]);
    sprintf(buffer_texto,"SP Register: %04XH",registro_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte im_leido=z80_header[29] & 3;
    sprintf(buffer_texto,"IM mode: %d",im_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte i_leido=z80_header[10];
    sprintf(buffer_texto,"I register: %02XH",i_leido);
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    z80_byte ints_leido=(z80_header[27] == 0 ? 0 : 1);
    sprintf(buffer_texto,"Interrupts: %s", (ints_leido ? "Enabled" : "Disabled"));
    indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);



	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("Z80 file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Z80 file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

    free(texto_browser);


}




void menu_file_viewer_read_file(char *title,char *file_name)
{

	//No mostrar caracteres especiales
	menu_disable_special_chars.v=1;

    if (menu_file_viewer_always_hex.v) {
        menu_file_hexdump_browser_show(file_name);
    }

    else {

        //printf ("extension vacia: %d\n",util_compare_file_extension(file_name,"") );
        //printf ("es z88 basic: %d\n",file_is_z88_basic(file_name));

        //Algunos tipos conocidos
        if (!util_compare_file_extension(file_name,"tap")) menu_tape_browser_show(file_name,-1);

        else if (!util_compare_file_extension(file_name,"zx")) menu_file_zx_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"sp")) menu_file_sp_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"z80")) menu_file_z80_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"col")) menu_file_col_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"sms")) menu_file_sms_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"sna")) menu_file_sna_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"snx")) menu_file_sna_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"nex")) menu_file_nex_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"spg")) menu_file_spg_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"bas")) menu_file_basic_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"b")) menu_file_basic_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"baszx80")) menu_file_basic_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"baszx81")) menu_file_basic_browser_show(file_name);

        //Aunque esa extension no la usa nadie pero es una manera de forzar que se pueda mostrar un archivo de tokens
        //z88 en caso que la deteccion automatica (que se hace aqui mas abajo) falle
        else if (!util_compare_file_extension(file_name,"basz88")) menu_file_basic_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"p") || !util_compare_file_extension(file_name,"p81")) menu_file_p_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"o")) menu_file_o_browser_show(file_name);

        //TODO .mmc viewer no soportado cuando el archivo .mmc esta dentro de una imagen fatfs (o sea no soporta zvfs)
        else if (!util_compare_file_extension(file_name,"mmc")) menu_file_mmc_browser_show(file_name,"MMC");

        //Suponemos que un mmcide (que sale de un hdf) es un mmc
        else if (!util_compare_file_extension(file_name,"mmcide")) menu_file_mmc_browser_show(file_name,"MMC");

        else if (!util_compare_file_extension(file_name,"ide")) menu_file_mmc_browser_show(file_name,"IDE");

        else if (!util_compare_file_extension(file_name,"trd")) menu_file_trd_browser_show(file_name,"TRD");

        else if (!util_compare_file_extension(file_name,"dsk")) menu_file_dsk_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"ddh")) menu_file_ddh_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"tzx")) menu_file_tzx_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"cas")) menu_file_cas_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"pzx")) menu_file_pzx_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"cdt")) menu_file_tzx_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"rwa")) menu_file_realtape_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"smp")) menu_file_realtape_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"wav")) menu_file_realtape_browser_show(file_name);

        //TODO .flash viewer no soportado cuando el archivo .mmc esta dentro de una imagen fatfs (o sea no soporta zvfs)
        else if (!util_compare_file_extension(file_name,"flash")) menu_file_flash_browser_show(file_name);

        //TODO .epr viewer no soportado cuando el archivo .mmc esta dentro de una imagen fatfs (o sea no soporta zvfs)
        else if (!util_compare_file_extension(file_name,"epr")) menu_z88_new_ptr_card_browser(file_name);

        //TODO .eprom viewer no soportado cuando el archivo .mmc esta dentro de una imagen fatfs (o sea no soporta zvfs)
        else if (!util_compare_file_extension(file_name,"eprom")) menu_z88_new_ptr_card_browser(file_name);

        else if (!util_compare_file_extension(file_name,"zsf")) menu_file_zsf_browser_show(file_name);

        else if (!util_compare_file_extension(file_name,"mdr")) menu_file_mdr_browser_show(file_name);

        //Si archivo no tiene extension pero su contenido parece indicar que es z88 basic
        else if (!util_compare_file_extension(file_name,"") && file_is_z88_basic(file_name)) menu_file_basic_browser_show(file_name);


        //Por defecto, texto
        else menu_file_viewer_read_text_file(title,file_name);


    }


	//IMPORTANTE: siempre se debe salir de la funcion desde aqui abajo, para que se vuelva a mostrar los caracteres especiales
	//Volver a mostrar caracteres especiales
	menu_disable_special_chars.v=0;
}


