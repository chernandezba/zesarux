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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cpu.h"
#include "esxdos_handler.h"
#include "operaciones.h"
#include "debug.h"
#include "menu_filesel.h"
#include "utils.h"
#include "diviface.h"
#include "screen.h"

#if defined(__APPLE__)
	#include <sys/syslimits.h>
#endif


//Al leer directorio se usa esxdos_handler_root_dir y esxdos_handler_cwd
char esxdos_handler_root_dir[PATH_MAX]="";
char esxdos_handler_cwd[PATH_MAX]="";

z80_int *registro_parametros_hl_ix;


const char *esxdos_plus3dos_signature="PLUS3DOS";

//Modo solo lectura, no se permiten escrituras
z80_bit esxdos_handler_readonly={0};


void esxdos_handler_footer_esxdos_handler_operating(void)
{

        generic_footertext_print_operating("ESX");
}




struct s_esxdos_fopen esxdos_fopen_files[ESXDOS_MAX_OPEN_FILES];


z80_bit esxdos_handler_enabled={0};

//Indica que al hacer reset, esxdos se quitara
//Esto sucede por ejemplo al hacer smartload de un archivo .nex, donde monta la carpeta donde esta ubicado el .nex,
//para poder soportar cargas de archivos de recursos o cualquier cosa que necesite el juego
z80_bit esxdos_umount_on_reset={0};

//Retorna contador a array de estructura de archivo vacio. Retorna -1 si no hay
int esxdos_find_free_fopen(void)
{
	int i;

	for (i=0;i<ESXDOS_MAX_OPEN_FILES;i++) {
		//Evitar valor 0 de numero de archivo. Parece que Dungeonette no le gusta en el primer fopen que el file handle sea 0
		//Sera algo normal? O fallo de ese juego? En cualquier caso, evitar retornar el handle numero 0
		if (esxdos_fopen_files[i].open_file.v==0 && i!=0) {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Free handle: %d",i);
			return i;
		}
	}

	return -1;
}


void esxdos_handler_fill_size_struct(z80_int puntero,z80_long_int l)
{

	poke_byte_no_time(puntero++,l&0xFF);

	l=l>>8;
	poke_byte_no_time(puntero++,l&0xFF);

	l=l>>8;
	poke_byte_no_time(puntero++,l&0xFF);

	l=l>>8;
	poke_byte_no_time(puntero++,l&0xFF);
}


void esxdos_handler_fill_date_struct(z80_int puntero,z80_byte hora,z80_byte minuto,z80_byte doblesegundos,
			z80_byte dia,z80_byte mes,z80_byte anyo)
	{

//Fecha.
/*
22-23   Time (5/6/5 bits, for hour/minutes/doubleseconds)
24-25   Date (7/4/5 bits, for year-since-1980/month/day)
*/
z80_int campo_tiempo;

//       15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//       -- hora ------ --- minutos ----- -- doblesec --

campo_tiempo=(hora<<11)|(minuto<<5)|doblesegundos;


poke_byte_no_time(puntero++,campo_tiempo&0xFF);
poke_byte_no_time(puntero++,(campo_tiempo>>8)&0xff);

z80_int campo_fecha;

//       15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//       ----- anyo --------  --- mes --- ---- dia -----

campo_fecha=(anyo<<9)|(mes<<5)|dia;



poke_byte_no_time(puntero++,campo_fecha&0xFF);
poke_byte_no_time(puntero++,(campo_fecha>>8)&0xff);
}

void esxdos_handler_copy_register_to_string(char *buffer_fichero,z80_int registro)
{

	int i;

	for (i=0;peek_byte_no_time(registro+i);i++) {
		buffer_fichero[i]=peek_byte_no_time(registro+i);
	}

	buffer_fichero[i]=0;
}

void esxdos_handler_copy_hl_to_string(char *buffer_fichero)
{


	esxdos_handler_copy_register_to_string(buffer_fichero,*registro_parametros_hl_ix);

/*
	int i;

	for (i=0;peek_byte_no_time((*registro_parametros_hl_ix)+i);i++) {
		buffer_fichero[i]=peek_byte_no_time((*registro_parametros_hl_ix)+i);
	}

	buffer_fichero[i]=0;
*/
}

void esxdos_handler_no_error_uncarry(void)
{
	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_C));
}

void esxdos_handler_error_carry(z80_byte error)
{
	Z80_FLAGS |=FLAG_C;
	reg_a=error;
}

void esxdos_handler_old_return_call(void)
{
	//Funcion que ya no hace nada. Antes todas las funciones llamaban aqui para saltar el byte de despues de rst8,
	//como final del handler. Pero ahora se llama solo cuando retorna finalmente el handler
	//para aquellas funciones que no gestiona el handler, no se llama aqui y se llama a rst8 normal


	//Este footer sale al finalizar el handler... Quiza seria hacerlo antes pero entonces habria que lanzarlo desde cada una
	//de las diferentes rst8 que se gestionan en el switch de esxdos_handler_begin_handling_commands
	//esxdos_handler_footer_esxdos_handler_operating();
	//reg_pc++;
}

void esxdos_handler_new_return_call(void)
{
	//Este footer sale al finalizar el handler... Quiza seria hacerlo antes pero entonces habria que lanzarlo desde cada una
	//de las diferentes rst8 que se gestionan en el switch de esxdos_handler_begin_handling_commands
	esxdos_handler_footer_esxdos_handler_operating();
	reg_pc++;
}

//TODO: esta funcion hace las primeras lineas de esxdos_handler_pre_fileopen. Cambiar esxdos_handler_pre_fileopen para que llame aqui
void esxdos_handler_get_fullpath(char *nombre_inicial,char *fullpath)
{
	//Si nombre archivo empieza por /, olvidar cwd
	if (nombre_inicial[0]=='/' || nombre_inicial[0]=='\\') sprintf (fullpath,"%s%s",esxdos_handler_root_dir,nombre_inicial);

	//TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a esxdos_handler_root_dir
	else sprintf (fullpath,"%s/%s/%s",esxdos_handler_root_dir,esxdos_handler_cwd,nombre_inicial);
}

//rellena fullpath con ruta completa
//funcion similar a zxpand_fileopen
void esxdos_handler_pre_fileopen(char *nombre_inicial,char *fullpath)
{


	//Si nombre archivo empieza por /, olvidar cwd
	if (nombre_inicial[0]=='/' || nombre_inicial[0]=='\\') sprintf (fullpath,"%s%s",esxdos_handler_root_dir,nombre_inicial);

	//TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a esxdos_handler_root_dir
	else sprintf (fullpath,"%s/%s/%s",esxdos_handler_root_dir,esxdos_handler_cwd,nombre_inicial);


	int existe_archivo=si_existe_archivo(fullpath);



	//Si no existe buscar archivo sin comparar mayusculas/minusculas
	//en otras partes del codigo directamente se busca primero el archivo con util_busca_archivo_nocase,
	//aqui se hace al reves. Se busca normal, y si no se encuentra, se busca nocase


		if (!existe_archivo) {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: File %s not found. Searching without case sensitive",fullpath);
			char encontrado[PATH_MAX];
			char directorio[PATH_MAX];
			util_get_complete_path(esxdos_handler_root_dir,esxdos_handler_cwd,directorio);
			//sprintf (directorio,"%s/%s",esxdos_handler_root_dir,esxdos_handler_cwd);
			if (util_busca_archivo_nocase ((char *)nombre_inicial,directorio,encontrado) ) {
				debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Found with name %s",encontrado);
				existe_archivo=1;

				//cambiamos el nombre fullpath y el nombre_inicial por el encontrado
				sprintf ((char *)nombre_inicial,"%s",encontrado);

				sprintf (fullpath,"%s/%s/%s",esxdos_handler_root_dir,esxdos_handler_cwd,(char *)nombre_inicial);
				//sprintf (fullpath,"%s/%s/%s",esxdos_handler_root_dir,esxdos_handler_cwd,encontrado);
				debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Found file %s searching without case sensitive",fullpath);
			}
		}





}


void esxdos_handler_debug_file_flags(z80_byte b)
{
	if (b&ESXDOS_RST8_FA_READ) debug_printf (VERBOSE_DEBUG,"ESXDOS handler: FA_READ|");
	if (b&ESXDOS_RST8_FA_WRITE) debug_printf (VERBOSE_DEBUG,"ESXDOS handler: FA_WRITE|");
	if (b&ESXDOS_RST8_FA_OPEN_EXIST) debug_printf (VERBOSE_DEBUG,"ESXDOS handler: FA_OPEN_EXIST|");
	if (b&ESXDOS_RST8_FA_OPEN_CREAT) debug_printf (VERBOSE_DEBUG,"ESXDOS handler: FA_OPEN_CREAT|");
	if (b&ESXDOS_RST8_FA_CREAT_NOEXIST) debug_printf (VERBOSE_DEBUG,"ESXDOS handler: FA_CREAT_NOEXIST|");
	if (b&ESXDOS_RST8_FA_USE_HEADER) debug_printf (VERBOSE_DEBUG,"ESXDOS handler: FA_USE_HEADER|");

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ");
}


//Obtiene de un nombre de archivo con path completo:
//atributo, timestamp, datestamp, filesize
//atributo retornado en variable attr. timestamp,datestamp, filesize se llena en memoria donde apunta puntero
//Retorna 0 si ok. 1 si error (por ejemplo no existe)
int esxdos_handler_get_attr_etc(char *nombre,z80_int puntero,z80_byte *atributo)
{

	if (!si_existe_archivo(nombre)) return 1;

	*atributo=0;

	/*
	Attribute - a bitvector. Bit 0: read only. Bit 1: hidden.
    	    Bit 2: system file. Bit 3: volume label. Bit 4: subdirectory.
        	Bit 5: archive. Bits 6-7: unused.

	*/


	if (get_file_type_from_name(nombre)==2) {
		//meter flags directorio y nombre entre <>
		//esxdos_handler_filinfo_fattrib |=16;
		//sprintf((char *) &esxdos_handler_globaldata[0],"<%s>",esxdos_handler_dp->d_name);
		//longitud_nombre +=2;
		*atributo |=16;
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Is a directory");
	}

	else {
		//sprintf((char *) &esxdos_handler_globaldata[0],"%s",esxdos_handler_dp->d_name);
	}

	//Meter nombre. Saltamos primer byte.
	//poke_byte_no_time((*registro_parametros_hl_ix)++,0);


	/*
	;                                                                       // <dword>  date
	;                                                                       // <dword>  filesize
	*/

	//Fecha.
	/*
	22-23   Time (5/6/5 bits, for hour/minutes/doubleseconds)
	24-25   Date (7/4/5 bits, for year-since-1980/month/day)
	*/

	int hora;
	int minutos;
	int doblesegundos;

	int anyo;
	int mes;
	int dia;


	get_file_date_from_name(nombre,&hora,&minutos,&doblesegundos,&dia,&mes,&anyo);

	anyo-=1980;
	doblesegundos *=2;

	esxdos_handler_fill_date_struct(puntero,hora,minutos,doblesegundos,dia,mes,anyo);


	//Tamanyo

	//copia para ir dividiendo entre 256
	long long int longitud_total=get_file_size(nombre);

	z80_long_int l=longitud_total;

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: length file: %d",l);
	esxdos_handler_fill_size_struct(puntero+4,l);

	return 0;


}


void esxdos_handler_call_f_unlink(void)
{
/*
; ***************************************************************************
; * F_UNLINK ($ad) *
; ***************************************************************************
; Delete file.
; Entry:
; A=drive specifier (overridden if filespec includes a drive)
; IX=filespec, null-terminated
; Exit (success):
; Fc=0
; Exit (failure):
; Fc=1
; A=error code
*/

	char nombre_archivo[PATH_MAX];
	char fullpath[PATH_MAX];
	esxdos_handler_copy_hl_to_string(nombre_archivo);


	esxdos_handler_pre_fileopen(nombre_archivo,fullpath);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: fullpath file: %s",fullpath);

	if (!si_existe_archivo(fullpath)) {
		esxdos_handler_error_carry(ESXDOS_ERROR_ENOENT);
		return;
	}

    //Ver si read only
    if (esxdos_handler_readonly.v) {
        debug_printf(VERBOSE_DEBUG,"ESXDOS handler: Device is open read only");
        esxdos_handler_error_carry(ESXDOS_ERROR_ERDONLY);
        return;

    }

	unlink(fullpath);

	esxdos_handler_no_error_uncarry();

}

void esxdos_handler_call_f_rename(void)
{
/*
; ***************************************************************************
; * F_RENAME ($b0) *
; ***************************************************************************
; Rename or move a file.
; Entry:
; A=drive specifier (overridden if filespec includes a drive)
; IX=source filespec, null-terminated
; DE=destination filespec, null-terminated
; Exit (success):
; Fc=0
; Exit (failure):
; Fc=1
; A=error code

*/

	char nombre_archivo[PATH_MAX];
	char fullpath[PATH_MAX];

	char nombre_archivo_destino[PATH_MAX];
	char fullpath_destino[PATH_MAX];

	esxdos_handler_copy_hl_to_string(nombre_archivo);
	esxdos_handler_pre_fileopen(nombre_archivo,fullpath);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: fullpath file: %s",fullpath);

	if (!si_existe_archivo(fullpath)) {
		esxdos_handler_error_carry(ESXDOS_ERROR_ENOENT);
		return;
	}

    //Ver si read only
    if (esxdos_handler_readonly.v) {
        debug_printf(VERBOSE_DEBUG,"ESXDOS handler: Device is open read only");
        esxdos_handler_error_carry(ESXDOS_ERROR_ERDONLY);
        return;

    }

	esxdos_handler_copy_register_to_string(nombre_archivo_destino,reg_de);
	esxdos_handler_pre_fileopen(nombre_archivo_destino,fullpath_destino);


	rename(fullpath,fullpath_destino);

	esxdos_handler_no_error_uncarry();

}


void esxdos_handler_call_f_stat(void)
{
/*
; ***************************************************************************
; * F_STAT ($ac) *
; ***************************************************************************
; Get unopened file information/status.
; Entry:
; A=drive specifier (overridden if filespec includes a drive)
; IX=filespec, null-terminated
; DE=11-byte buffer address
; Exit (success):
; Fc=0
; Exit (failure):
; Fc=1
; A=error code
;
; NOTES:
; The following details are returned in the 11-byte buffer:
; +0(1) drive specifier
; +1(1) $81
; +2(1) file attributes (MS-DOS format)
; +3(2) timestamp (MS-DOS format)
; +5(2) datestamp (MS-DOS format)
; +7(4) file size in bytes
*/

	char nombre_archivo[PATH_MAX];
	char fullpath[PATH_MAX];
	esxdos_handler_copy_hl_to_string(nombre_archivo);


	esxdos_handler_pre_fileopen(nombre_archivo,fullpath);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: fullpath file: %s",fullpath);

	z80_int puntero=reg_de;
	poke_byte_no_time(puntero++,0); //drive

	poke_byte_no_time(puntero++,0x81);

	z80_byte atributos;

	//de momento saltar byte atributos
	z80_int puntero_atrib=puntero;

	puntero++;

	if (esxdos_handler_get_attr_etc(fullpath,puntero_atrib,&atributos)) {
		/*
		; Exit (failure):
; Fc=1
; A=error code
		*/
		esxdos_handler_error_carry(ESXDOS_ERROR_ENOENT);
		return;
	}

	//Meter atributos
	poke_byte_no_time(puntero_atrib,atributos);

	esxdos_handler_no_error_uncarry();

}


void esxdos_handler_call_f_open_post(int handle,char *nombre_archivo,char *fullpath)
{

		//Indicar handle ocupado
		esxdos_fopen_files[handle].open_file.v=1;

		esxdos_fopen_files[handle].is_a_directory.v=0;

		//Y poner nombres para debug
		strcpy(esxdos_fopen_files[handle].debug_name,nombre_archivo);
		strcpy(esxdos_fopen_files[handle].debug_fullpath,fullpath);
}

void esxdos_handler_change_backslashes(char *string)
{
	while (*string) {
		if (*string=='\\') *string='/';

		string++;
	}
}


void esxdos_handler_call_f_open(void)
{
	/*
	;                                                                       // Open file. A=drive. HL=Pointer to null-
;                                                                       // terminated string containg path and/or
;                                                                       // filename. B=file access mode. DE=Pointer
;                                                                       // to BASIC header data/buffer to be filled
;                                                                       // with 8 byte PLUS3DOS BASIC header. If you
;                                                                       // open a headerless file, the BASIC type is
;                                                                       // $ff. Only used when specified in B.
;                                                                       // On return without error, A=file handle.
*/

	char fopen_mode[10];

	z80_byte modo_abrir=reg_b;

	esxdos_handler_debug_file_flags(modo_abrir);

	//Si se debe escribir la cabecera plus3 . escribir en el sentido de:
	//Si se abre para lectura, se pasaran los 8 bytes del basic a destino DE
	//Si se abre para escritura, se leeran 8 bytes desde DE al archivo en cuanto se escriba la primera vez
	int debe_usar_plus3_header=0;

	//Si se abre archivo para lectura
	int escritura=1;

	//Si debe escribir cabecera PLUS3DOS
	if ( (modo_abrir&ESXDOS_RST8_FA_USE_HEADER)==ESXDOS_RST8_FA_USE_HEADER) {
		//volcar contenido DE 8 bytes a buffer
		debe_usar_plus3_header=1;
		modo_abrir &=(255-ESXDOS_RST8_FA_USE_HEADER);

	}

	//Modos soportados


	switch (modo_abrir) {
			case ESXDOS_RST8_FA_READ:
				strcpy(fopen_mode,"rb");
				escritura=0;
			break;

			case ESXDOS_RST8_FA_READ|ESXDOS_RST8_FA_WRITE:
				strcpy(fopen_mode,"wb");
			break;

			case ESXDOS_RST8_FA_CREAT_NOEXIST|ESXDOS_RST8_FA_WRITE:
				strcpy(fopen_mode,"wb");
			break;

			case ESXDOS_RST8_FA_OPEN_CREAT|ESXDOS_RST8_FA_WRITE:
				strcpy(fopen_mode,"wb");
			break;

			/*
			#define ESXDOS_RST8_FA_CREAT_NOEXIST 0x04
// create if does not exist, else error

#define ESXDOS_RST8_FA_OPEN_CREAT  0x08
			*/

			case ESXDOS_RST8_FA_CREATE_TRUNC:
				//significa ESXDOS_MODE_CREAT_TRUNC :  create or replace an existing file; fp = 0
				strcpy(fopen_mode,"wb");
			break;

/*
Debug: ESXDOS handler: FA_WRITE|
Debug: ESXDOS handler: FA_OPEN_CREAT|
Debug: ESXDOS handler: FA_CREAT_NOEXIST|

Esto se usa en NextDaw, es open+truncate
*/
			case ESXDOS_RST8_FA_WRITE|ESXDOS_RST8_FA_OPEN_CREAT|ESXDOS_RST8_FA_CREAT_NOEXIST:
				strcpy(fopen_mode,"wb");
			break;

            //Usado en SE Basic
            case ESXDOS_RST8_FA_READ|ESXDOS_RST8_FA_WRITE|ESXDOS_RST8_FA_OPEN_CREAT:
				strcpy(fopen_mode,"wb");
			break;

			//case FA_WRITE|FA_CREAT_NOEXIST|FA_USE_HEADER

			default:

				debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Unsupported fopen mode: %02XH",reg_b);
				esxdos_handler_error_carry(ESXDOS_ERROR_EIO);
				return;
			break;
	}

    //Ver si read only
    //Se comprueba si fopen_mode es "wb"
    if (esxdos_handler_readonly.v) {
        if (!strcmp(fopen_mode,"wb")) {
            debug_printf(VERBOSE_DEBUG,"ESXDOS handler: Device is open read only");
            esxdos_handler_error_carry(ESXDOS_ERROR_ERDONLY);
            return;
        }
    }


	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Opening file in system mode: [%s]",fopen_mode);


	//Ver si no se han abierto el maximo de archivos y obtener handle libre
	int free_handle=esxdos_find_free_fopen();
	if (free_handle==-1) {
		esxdos_handler_error_carry(ESXDOS_ERROR_ENFILE);
		esxdos_handler_old_return_call();
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: no free handles");
		return;
	}

	char nombre_archivo[PATH_MAX];
	char fullpath[PATH_MAX];
	esxdos_handler_copy_hl_to_string(nombre_archivo);

	//Parece que nextos permite rutas con barras invertidas. Cambiarlas
	if (MACHINE_IS_TBBLUE) esxdos_handler_change_backslashes(nombre_archivo);

	esxdos_fopen_files[free_handle].tiene_plus3dos_header.v=0;

	if (debe_usar_plus3_header && escritura) {
		debug_printf (VERBOSE_DEBUG,"Preparing PLUS3DOS 8 byte header");
		esxdos_fopen_files[free_handle].tiene_plus3dos_header.v=1;
		int i;
		for (i=0;i<8;i++) {
			z80_byte byte_leido=peek_byte_no_time(reg_de+i);
			esxdos_fopen_files[free_handle].buffer_plus3dos_header[i]=byte_leido;
			//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: %02XH ",byte_leido);
		}

		//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ");
	}


	esxdos_handler_pre_fileopen(nombre_archivo,fullpath);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: fullpath file: %s",fullpath);


	//Ver tipos de apertura que dan error si existe
	if ( modo_abrir==(ESXDOS_RST8_FA_CREAT_NOEXIST | ESXDOS_RST8_FA_WRITE))  {
		if (si_existe_archivo(fullpath)) {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: file exists and using mode FA_CREAT_NOEXIST|ESXDOS_RST8_FA_WRITE. Error");
			esxdos_handler_error_carry(ESXDOS_ERROR_EEXIST);
			esxdos_handler_old_return_call();
			return;
		}
	}


	//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: file type: %d",get_file_type_from_name(fullpath));
	//sleep(5);

	//Si archivo es directorio, error
	if (get_file_type_from_name(fullpath)==2) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: is a directory. can't fopen it");
		esxdos_handler_error_carry(ESXDOS_ERROR_EISDIR);
		esxdos_handler_old_return_call();
		return;
	}

	//Abrir el archivo.
	esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix=fopen(fullpath,fopen_mode);


	if (esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix==NULL) {
		esxdos_handler_error_carry(ESXDOS_ERROR_ENOENT);
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_open file: %s",fullpath);
		esxdos_handler_old_return_call();
		return;
	}
	else {
		//temp_esxdos_last_open_file_handler=1;
		if (debe_usar_plus3_header && escritura==0) {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Reading PLUS3DOS header at DE=%04XH",reg_de);
			//char buffer_registros[1024];
			//print_registers(buffer_registros);
			//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: %s",buffer_registros);

			//TODO: El NMI handler, al abrir la pantalla de ayuda, utiliza esto, aunque realmente es un .scr sin cabecera
			//como workaround, hacemos que el que realmente no tenga cabecera (no empieza por "plus3dos"), no se le lea cabecera
			//Si no, la pantalla de ayuda se veria desplazada estos 128 bytes
			/*
        	Bytes 0...7     - +3DOS signature - 'PLUS3DOS'
        	Byte 8          - 1Ah (26) Soft-EOF (end of file)
        	Byte 9          - Issue number
        	Byte 10         - Version number
        	Bytes 11...14   - Length of the file in bytes, 32 bit number,
                            least significant byte in lowest address
        	Bytes 15...22   - +3 BASIC header data
			*/

			//Leer los primeros 8 bytes
			char buffer_signature[8+1]; //8+1 del final
			fread(&buffer_signature,1,8,esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix);

			//meter 0 del final
			buffer_signature[8]=0;

			//Ver si es la firma esperada
			if (strcmp(esxdos_plus3dos_signature,buffer_signature))  {
				//No lo es. reabrir archivo
				debug_printf(VERBOSE_DEBUG,"ESXDOS handler: Requested PLUS3DOS header but file does not have one");
				fclose(esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix);
				esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix=fopen(fullpath,fopen_mode);
			}

			else {
				debug_printf(VERBOSE_DEBUG,"ESXDOS handler: File seems to have good PLUS3DOS header");
				//Saltar los primeros 15-8=7
				char buffer_siete[7];
				fread(&buffer_siete,1,7,esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix);

				//Y meter en DE los siguientes 8
				int i;
				z80_byte byte_leido;
				for (i=0;i<8;i++) {
					fread(&byte_leido,1,1,esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix);
					poke_byte_no_time(reg_de+i,byte_leido);
					//poke_byte_no_time(reg_de+i,0xFF); //temp
					//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: %02XH ",byte_leido);
				}

				//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ");

				//Y saltar otros (128-23)
				char buffer_restante[128-23];
				fread(&buffer_restante,1,128-23,esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix);
			}
		}


		reg_a=free_handle;
		esxdos_handler_no_error_uncarry();
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Successfully esxdos_handler_call_f_open handle: %d file: %s",free_handle,fullpath);


		if (stat(fullpath, &esxdos_fopen_files[free_handle].last_file_buf_stat)!=0) {
						debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Unable to get status of file %s",fullpath);
		}


		esxdos_handler_call_f_open_post(free_handle,nombre_archivo,fullpath);


	}


	esxdos_handler_old_return_call();


}

void esxdos_handler_call_f_read(void)
{

	int file_handler=reg_a;

	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_read. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}

	if (esxdos_fopen_files[file_handler].open_file.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_read. Handler %d not found",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}
	else {

		//Si es un directorio, error
		if (esxdos_fopen_files[file_handler].is_a_directory.v) {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_read. Handler %d is a directory",file_handler);
			esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
			esxdos_handler_old_return_call();
			return;
		}

		/*
		f_read                  equ fsys_base + 5;      // $9d  sbc a,l
;                                                                       // Read BC bytes at HL from file handle A.
;                                                                       // On return BC=number of bytes successfully
;                                                                       // read. File pointer is updated.
*/
		z80_int total_leidos=0;
		z80_int bytes_a_leer=reg_bc;
		int leidos=1;

		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: esxdos_handler_call_f_read. Unix file handle: %p",esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

		while (bytes_a_leer && leidos) {
			z80_byte byte_read;
			leidos=fread(&byte_read,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			if (leidos) {
					poke_byte_no_time((*registro_parametros_hl_ix)+total_leidos,byte_read);
					total_leidos++;
					bytes_a_leer--;
			}
		}

		reg_bc=total_leidos;
		//(*registro_parametros_hl_ix) +=total_leidos; //???
		esxdos_handler_no_error_uncarry();

		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Successfully esxdos_handler_call_f_read total bytes read: %d",total_leidos);

	}

	esxdos_handler_old_return_call();
}

void esxdos_handler_call_f_seek(void)
{

	int file_handler=reg_a;

	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_seek. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}

	if (esxdos_fopen_files[file_handler].open_file.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_seek. Handler %d not found",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}

	//Si es un directorio, error
	if (esxdos_fopen_files[file_handler].is_a_directory.v) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_seek. Handler %d is a directory",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


	long initial_offset=ftell(esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: offset was at %ld",initial_offset);

/*
F_SEEK: Seek BCDE bytes. A=handle

IXL / L=mode (0 from start of file, 1 fwd from current pos, 2 bak from current pos).

On return BCDE=current file pointer. FIXME-Should return bytes actually seeked
*/

//fwrite(cabe,1,8,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

	long offset=(reg_b<<24) | (reg_c<<16) | (reg_d<<8) | reg_e;

	int whence;

	//Usar reg_l o IX_L

	z80_byte f_seek_mode;

	//f_seek_mode=(z80_byte) *registro_parametros_hl_ix;

	//Mas elegante asi:
	f_seek_mode=(z80_byte) ((*registro_parametros_hl_ix) & 0xff);

	switch (f_seek_mode) {
		case 0:
			whence=SEEK_SET;
		break;

		case 1:
			whence=SEEK_CUR;
		break;

		case 2:
			whence=SEEK_CUR;
			offset=-offset;
		break;

		default:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_seek. Unsupported mode %d",f_seek_mode);
			esxdos_handler_error_carry(ESXDOS_ERROR_EIO);
			esxdos_handler_old_return_call();
			return;
		break;

	}




	if (fseek (esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix, offset, whence)!=0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error running fseek system call");
	}

	//Retornar BCDE
	offset=ftell(esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: offset is now at %ld",offset);

	//sleep (5);

	//long offset=(reg_b<<24) | (reg_c<<16) | (reg_d<<8) | reg_e;
	reg_b=(offset>>24)&0xFF;
	reg_c=(offset>>16)&0xFF;
	reg_d=(offset>>8)&0xFF;
	reg_e= offset & 0xFF;

	esxdos_handler_no_error_uncarry();
	esxdos_handler_old_return_call();
}


void esxdos_handler_call_f_write(void)
{

	int file_handler=reg_a;

	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_write. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}

	if (esxdos_fopen_files[file_handler].open_file.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_write. Handler %d not found",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}
	else {
		//Si es un directorio, error
		if (esxdos_fopen_files[file_handler].is_a_directory.v) {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_write. Handler %d is a directory",file_handler);
			esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
			esxdos_handler_old_return_call();
			return;
		}


		/*
		f_write                 equ fsys_base + 6;      // $9e  sbc a,(hl)
		;                                                                       // Write BC bytes from HL to file handle A.
		;                                                                       // On return BC=number of bytes successfully
		;                                                                       // written. File pointer is updated.
		*/

		if (esxdos_fopen_files[file_handler].tiene_plus3dos_header.v) {
			//Escribir primero cabecera PLUS3DOS
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Adding PLUS3DOS Header");
			//TODO: asumimos que se escriben todos los bytes de golpe->BC contiene la longitud total del bloque
			/*

Offset	Length	Description
0	8	"PLUS3DOS" identification text
8	1	0x1a marker byte (note 1)
9	1	Issue number   (01 en esxdos)
10	1	Version number (00 en esxdos)
11	4	Length of the file in bytes (note 2)
15..22		+3 Basic header data
23..126		Reserved (note 3)
127		Checksum (note 4))
*/
			//char *cabe="PLUS3DOS";
			fwrite(esxdos_plus3dos_signature,1,8,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

			z80_byte marker=0x1a,issue=1,version=1;
			fwrite(&marker,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			fwrite(&issue,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			fwrite(&version,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

			//longitud
			fwrite(&reg_c,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			fwrite(&reg_b,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			z80_byte valor_nulo=0;
			fwrite(&valor_nulo,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			fwrite(&valor_nulo,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

			//La cabecera que teniamos guardada
			int i;
			for (i=0;i<8;i++) {
				fwrite(&esxdos_fopen_files[file_handler].buffer_plus3dos_header[i],1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			}

			//Reservado
			for (i=0;i<104;i++) fwrite(&valor_nulo,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

			//Checksum. De momento inventarlo
			z80_byte valor_checksum=0;
			fwrite(&valor_checksum,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
		}



		z80_int bytes_a_escribir=reg_bc;
		z80_byte byte_read;
		z80_int total_leidos=0;

		while (bytes_a_escribir) {
			byte_read=peek_byte_no_time((*registro_parametros_hl_ix)+total_leidos);
			fwrite(&byte_read,1,1,esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
			total_leidos++;
			bytes_a_escribir--;

		}

		esxdos_handler_no_error_uncarry();

		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Successfully esxdos_handler_call_f_write total bytes write: %d",total_leidos);

	}

	esxdos_handler_old_return_call();
}

void esxdos_handler_call_f_close(void)
{

	//fclose tambien se puede llamar para cerrar lectura de directorio.

	int file_handler=reg_a;

	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_close. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}



	if (esxdos_fopen_files[file_handler].open_file.v==0) {
		//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_close. Handler %d not found",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;

	}

	else {
		if (esxdos_fopen_files[file_handler].is_a_directory.v==0) {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Closing a file");
			fclose(esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);
		}

		else {
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Closing a directory");
			closedir(esxdos_fopen_files[file_handler].esxdos_handler_dfd);
		}

		esxdos_fopen_files[file_handler].open_file.v=0;
		esxdos_handler_no_error_uncarry();
	}

	esxdos_handler_old_return_call();
}


void esxdos_handler_call_f_sync(void)
{

	int file_handler=reg_a;

	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_sync. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		return;
	}



	if (esxdos_fopen_files[file_handler].open_file.v==0) {
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		return;

	}

	//Si es un directorio, error
	if (esxdos_fopen_files[file_handler].is_a_directory.v) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_sync. Handler %d is a directory",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


	fflush(esxdos_fopen_files[file_handler].esxdos_last_open_file_handler_unix);

}


//tener en cuenta raiz y directorio actual
//si localdir no es NULL, devolver directorio local (quitando esxdos_handler_root_dir)
//funcion igual a zxpand_get_final_directory, solo adaptando nombres de variables zxpand->esxdos_handler
void esxdos_handler_get_final_directory(char *dir, char *finaldir, char *localdir)
{


	//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: esxdos_handler_get_final_directory. dir: %s esxdos_handler_root_dir: %s",dir,esxdos_handler_root_dir);

	//Guardamos directorio actual del emulador
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//cambiar a directorio indicado, juntando raiz, dir actual de esxdos_handler, y dir
	char dir_pedido[PATH_MAX];

	//Si directorio pedido es absoluto, cambiar cwd
	if (dir[0]=='/' || dir[0]=='\\') {
		sprintf (esxdos_handler_cwd,"%s",dir);
		sprintf (dir_pedido,"%s/%s",esxdos_handler_root_dir,esxdos_handler_cwd);
	}


	else {
		sprintf (dir_pedido,"%s/%s/%s",esxdos_handler_root_dir,esxdos_handler_cwd,dir);
	}

	chdir(dir_pedido);

	//Ver en que directorio estamos
	//char dir_final[PATH_MAX];
	getcwd(finaldir,PATH_MAX);
	//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: total final directory: %s . esxdos_handler_root_dir: %s",finaldir,esxdos_handler_root_dir);
	//Esto suele retornar sim barra al final

	//Si finaldir no tiene barra al final, haremos que esxdos_handler_root_dir tampoco la tenga
	int l=strlen(finaldir);

	if (l) {
		if (finaldir[l-1]!='/' && finaldir[l-1]!='\\') {
			//finaldir no tiene barra al final
			int m=strlen(esxdos_handler_root_dir);
			if (m) {
				if (esxdos_handler_root_dir[m-1]=='/' || esxdos_handler_root_dir[m-1]=='\\') {
					//root dir tiene barra al final. quitarla
					//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: quitando barra del final de esxdos_handler_root_dir");
					esxdos_handler_root_dir[m-1]=0;
				}
			}
		}
	}


	//Ahora hay que quitar la parte del directorio raiz
	//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: running strstr (%s,%s)",finaldir,esxdos_handler_root_dir);
	char *s=strstr(finaldir,esxdos_handler_root_dir);

	if (s==NULL) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Directory change not allowed");
		//directorio final es el mismo que habia
		sprintf (finaldir,"%s",esxdos_handler_cwd);
		return;
	}

	//Si esta bien, meter parte local
	if (localdir!=NULL) {
		int l=strlen(esxdos_handler_root_dir);
		sprintf (localdir,"%s",&finaldir[l]);
		//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: local directory: %s",localdir);
	}

	//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: directorio final local de esxdos_handler: %s",finaldir);


	//Restauramos directorio actual del emulador
	chdir(directorio_actual);
}


void esxdos_handler_call_f_chdir(void)
{

	char ruta[PATH_MAX];
	esxdos_handler_copy_hl_to_string(ruta);



		char directorio_final[PATH_MAX];

		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Changing to directory %s",ruta);

		esxdos_handler_get_final_directory(ruta,directorio_final,esxdos_handler_cwd);

		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Final directory %s . cwd: %s",directorio_final,esxdos_handler_cwd);


	esxdos_handler_no_error_uncarry();
	esxdos_handler_old_return_call();
}


void esxdos_handler_call_f_mkdir(void)
{

	char nombre_archivo[PATH_MAX];
	char fullpath[PATH_MAX];
	esxdos_handler_copy_hl_to_string(nombre_archivo);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Mkdir %s",nombre_archivo);
	esxdos_handler_get_fullpath(nombre_archivo,fullpath);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: mkdir: fullpath: %s",fullpath);

	menu_filesel_mkdir(fullpath);

	esxdos_handler_no_error_uncarry();


}

void esxdos_handler_copy_string_to_hl(char *s)
{
	z80_int p=0;

	while (*s) {
		poke_byte_no_time((*registro_parametros_hl_ix)+p,*s);
		s++;
		p++;
	}

	poke_byte_no_time((*registro_parametros_hl_ix)+p,0);
}

void esxdos_handler_call_f_getcwd(void)
{

	// Get current folder path (null-terminated)
// to buffer. A=drive. HL=pointer to buffer.

	esxdos_handler_copy_string_to_hl(esxdos_handler_cwd);

	esxdos_handler_no_error_uncarry();
	esxdos_handler_old_return_call();
}

void esxdos_handler_call_f_opendir(void)
{

	/*

	;                                                                       // Open folder. A=drive. HL=Pointer to zero
;                                                                       // terminated string with path to folder.
;                                                                       // B=folder access mode. Only the BASIC
;                                                                       // header bit matters, whether you want to
;                                                                       // read header information or not. On return
;                                                                       // without error, A=folder handle.

*/

//Ver si no se han abierto el maximo de archivos y obtener handle libre
int free_handle=esxdos_find_free_fopen();
if (free_handle==-1) {
	esxdos_handler_error_carry(ESXDOS_ERROR_ENFILE);
	esxdos_handler_old_return_call();
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: no free handles");
	return;
}


	char directorio[PATH_MAX];

	esxdos_handler_copy_hl_to_string(directorio);
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: opening directory %s, drive %d, folder access mode %02XH",directorio,reg_a,reg_b);

	char directorio_final[PATH_MAX];
	//obtener directorio final
	esxdos_handler_get_final_directory((char *) directorio,directorio_final,NULL);


	//Guardar directorio final, hace falta al leer cada entrada para saber su tamanyo
	sprintf (esxdos_fopen_files[free_handle].esxdos_handler_last_dir_open,"%s",directorio_final);
	esxdos_fopen_files[free_handle].esxdos_handler_dfd = opendir(directorio_final);

	if (esxdos_fopen_files[free_handle].esxdos_handler_dfd == NULL) {
	 	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Can't open directory %s (full: %s)", directorio,directorio_final);
	  esxdos_handler_error_carry(ESXDOS_ERROR_ENOENT);
		esxdos_handler_old_return_call();
		return;
	}

	else {
		esxdos_fopen_files[free_handle].open_file.v=1;
		esxdos_fopen_files[free_handle].is_a_directory.v=1;
		esxdos_fopen_files[free_handle].contador_directorio=0;
		reg_a=free_handle;
		esxdos_handler_no_error_uncarry();
	}


	esxdos_handler_old_return_call();
}

//comprobaciones de nombre de archivos en directorio
int esxdos_handler_readdir_no_valido(char *s)
{

	//Si longitud mayor que 12 (8 nombre, punto, 3 extension)
	//if (strlen(s)>12) return 0;

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: checking if name %s is valid",s);


	char extension[NAME_MAX];
	char nombre[NAME_MAX];

	util_get_file_extension(s,extension);
	util_get_file_without_extension(s,nombre);

	//si nombre mayor que 8
	if (strlen(nombre)>8) return 0;

	//si extension mayor que 3
	if (strlen(extension)>3) return 0;


	//si hay letras minusculas
	//int i;
	//for (i=0;s[i];i++) {
	//	if (s[i]>='a' && s[i]<'z') return 0;
	//}



	return 1;

}

int esxdos_handler_string_to_msdos(char *fullname,z80_int puntero)
{
	z80_int i;

	//for (i=0;i<12;i++) poke_byte_no_time(puntero+i,' ');

	for (i=0;i<12 && fullname[i];i++) {
		poke_byte_no_time(puntero+i,fullname[i]);
	}

	poke_byte_no_time(puntero+i,0);
	i++;

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: length name: %d",i);
	//if (i<11) poke_byte_no_time(puntero+i,0);

	//return 12;
	return i;

	//Primero rellenar con espacios




	//Aunque aqui no deberia exceder de 8 y 3, pero por si acaso
	char nombre[PATH_MAX];
	char extension[PATH_MAX];

	util_get_file_without_extension(fullname,nombre);
	util_get_file_extension(fullname,extension);

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: name: %s extension: %s",nombre,extension);

	//Escribir nombre
	for (i=0;i<8 && nombre[i];i++) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: %c",nombre[i]);
		poke_byte_no_time(puntero+i,nombre[i]);
	}

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: .");

	//Escribir extension
	for (i=0;i<3 && extension[i];i++) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: %c",extension[i]);
		poke_byte_no_time(puntero+8+i,extension[i]);
	}

	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ");

}

//Retorna 0 si no hay mas archivos
//Retorna 1 si ok
int esxdos_aux_readdir(int file_handler)
{
	do {

	esxdos_fopen_files[file_handler].esxdos_handler_dp = readdir(esxdos_fopen_files[file_handler].esxdos_handler_dfd);

	if (esxdos_fopen_files[file_handler].esxdos_handler_dp == NULL) {


		//temp closedir(esxdos_fopen_files[file_handler].esxdos_handler_dfd);
		//temp esxdos_fopen_files[file_handler].esxdos_handler_dfd=NULL;
		debug_printf (VERBOSE_DEBUG,"No more files on readdir");


		//no hay mas archivos
		//reg_a=0;
		//esxdos_handler_no_error_uncarry();
		//esxdos_handler_old_return_call();
		return 0;
	}


	} while(!esxdos_handler_readdir_no_valido(esxdos_fopen_files[file_handler].esxdos_handler_dp->d_name));

	return 1;
}


void esxdos_handler_call_f_readdir(void)
{

	//Guardamos hl por si acaso, porque lo modificaremos para comodidad
	z80_int old_hl=(*registro_parametros_hl_ix);

	int file_handler=reg_a;

	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_readdir. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}

	/*
	f_readdir               equ fsys_base + 12;     // $a4  and h
;                                                                       // Read a folder entry to a buffer pointed
;                                                                       // to by HL. A=handle. Buffer format:
;                                                                       // <ASCII>  file/dirname
;                                                                       // <byte>   attributes (MS-DOS format)
;                                                                       // <dword>  date
;                                                                       // <dword>  filesize
;                                                                       // If opened with BASIC header bit, the
;                                                                       // BASIC header follows the normal entry
;                                                                       // (with type=$ff if headerless).
;                                                                       // On return, if A=1 there are more entries.
;                                                                       // If A=0 then it is the end of the folder.
;                                                                       // Does not currently return the size of an
;                                                                       // entry, or zero if end of folder reached.
*/

if (esxdos_fopen_files[file_handler].open_file.v==0) {
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_readdir. Handler %d not found",file_handler);
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}

if (esxdos_fopen_files[file_handler].esxdos_handler_dfd==NULL) {
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}

	//Si no es un directorio, error
	if (esxdos_fopen_files[file_handler].is_a_directory.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_readdir. Handler %d is not a directory",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


	if (!esxdos_aux_readdir(file_handler)) {
		//no hay mas archivos
		debug_printf (VERBOSE_DEBUG,"Returning no more files to readdir");
		reg_a=0;
		esxdos_handler_no_error_uncarry();
		esxdos_handler_old_return_call();

		return;
	}



//obtener nombre con directorio. obtener combinando directorio root, actual y inicio listado
char nombre_final[PATH_MAX];
util_get_complete_path(esxdos_fopen_files[file_handler].esxdos_handler_last_dir_open,esxdos_fopen_files[file_handler].esxdos_handler_dp->d_name,nombre_final);

z80_byte atributo_archivo;



z80_int puntero=(*registro_parametros_hl_ix);
//Saltamos byte Atributos de momento. Se rellena despues
puntero++;

int retornado_nombre=esxdos_handler_string_to_msdos(esxdos_fopen_files[file_handler].esxdos_handler_dp->d_name,puntero);


puntero+=retornado_nombre;


//Obtener atributo, hora, fecha, longitud
esxdos_handler_get_attr_etc(nombre_final,puntero,&atributo_archivo);

//Meter atributo en direccion de registro_parametros_hl_ix, por tanto recuperamos puntero inicial
puntero=(*registro_parametros_hl_ix);
poke_byte_no_time(puntero,atributo_archivo);


//para telldir
esxdos_fopen_files[file_handler].contador_directorio +=32;

//Dejamos hl como estaba por si acaso
(*registro_parametros_hl_ix)=old_hl;

reg_a=1; //Hay mas ficheros
esxdos_handler_no_error_uncarry();
esxdos_handler_old_return_call();



}


void esxdos_handler_call_f_seekdir(void)
{
	int posicion=(BC*65536)+DE;
	posicion /=32;

	int file_handler=reg_a;

	debug_printf (VERBOSE_DEBUG,"Skipping %d files on seekdir file handler %d",posicion,file_handler);


	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_seekdir. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


	if (esxdos_fopen_files[file_handler].open_file.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_seekdir. Handler %d not found",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}

	if (esxdos_fopen_files[file_handler].esxdos_handler_dfd==NULL) {
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}

	//Si no es un directorio, error
	if (esxdos_fopen_files[file_handler].is_a_directory.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_seekdir. Handler %d is not a directory",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


	//Reabrimos el directorio
	rewinddir(esxdos_fopen_files[file_handler].esxdos_handler_dfd);

	esxdos_fopen_files[file_handler].contador_directorio=0;

	//Y leemos tantos como se indique la pisicion



	while (posicion>0)
	{
		//printf ("quedan: %d\n",posicion);
		if (!esxdos_aux_readdir(file_handler)) {
			//no hay mas archivos
			reg_a=0;
			esxdos_handler_no_error_uncarry();
			esxdos_handler_old_return_call();
			return;
		}

		//temporal sacar nombre
		char nombre_final[PATH_MAX];
		util_get_complete_path(esxdos_fopen_files[file_handler].esxdos_handler_last_dir_open,esxdos_fopen_files[file_handler].esxdos_handler_dp->d_name,nombre_final);
		debug_printf(VERBOSE_DEBUG,"Current name: %s",nombre_final);

		esxdos_fopen_files[file_handler].contador_directorio +=32;

		posicion--;
	}

	debug_printf (VERBOSE_DEBUG,"End skipping");

	//Ya nos hemos posicionado
	esxdos_handler_no_error_uncarry();
	esxdos_handler_old_return_call();

}

void esxdos_handler_call_f_rewinddir(void)
{

	int file_handler=reg_a;


if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_rewinddir. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


if (esxdos_fopen_files[file_handler].open_file.v==0) {
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_rewinddir. Handler %d not found",file_handler);
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}

if (esxdos_fopen_files[file_handler].esxdos_handler_dfd==NULL) {
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}

	//Si no es un directorio, error
	if (esxdos_fopen_files[file_handler].is_a_directory.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_rewinddir. Handler %d is not a directory",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


	//Reabrimos el directorio
	rewinddir(esxdos_fopen_files[file_handler].esxdos_handler_dfd);


	esxdos_handler_no_error_uncarry();
	esxdos_handler_old_return_call();

}


void esxdos_handler_call_f_telldir(void)
{


	int file_handler=reg_a;

	if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_telldir. Handler %d out of range",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}


if (esxdos_fopen_files[file_handler].open_file.v==0) {
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_telldir. Handler %d not found",file_handler);
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}

if (esxdos_fopen_files[file_handler].esxdos_handler_dfd==NULL) {
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}

	//Si no es un directorio, error
	if (esxdos_fopen_files[file_handler].is_a_directory.v==0) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_telldir. Handler %d is not a directory",file_handler);
		esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
		esxdos_handler_old_return_call();
		return;
	}



/*
F_TELLDIR: Returns current offset of directory in BCDE. A=dir handle
*/
	reg_b=(esxdos_fopen_files[file_handler].contador_directorio>>24)&255;
	reg_c=(esxdos_fopen_files[file_handler].contador_directorio>>16)&255;
	reg_d=(esxdos_fopen_files[file_handler].contador_directorio>>8)&255;
	reg_e=(esxdos_fopen_files[file_handler].contador_directorio   )&255;


	esxdos_handler_no_error_uncarry();
	esxdos_handler_old_return_call();

}


void esxdos_handler_call_f_fstat(void)
{

/*
f_fstat                 equ fsys_base + 9;      // $a1  and c
;                                                                       // Get file info/status to buffer at HL.
;                                                                       // A=handle. Buffer format:
;                                                                       // <byte>  drive
;                                                                       // <byte>  device
;                                                                       // <byte>  file attributes (MS-DOS format)
;                                                                       // <dword> date
;                                                                       // <dword> file size
*/
int file_handler=reg_a;

if (file_handler>=ESXDOS_MAX_OPEN_FILES) {
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_stat. Handler %d out of range",file_handler);
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}

if (esxdos_fopen_files[file_handler].open_file.v==0) {
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Error from esxdos_handler_call_f_stat. Handler %d not found",file_handler);
	esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);
	esxdos_handler_old_return_call();
	return;
}


	//TODO. segun file handler en A
	poke_byte_no_time((*registro_parametros_hl_ix),0); //drive
	poke_byte_no_time((*registro_parametros_hl_ix)+1,0); //device


	z80_byte atributo_archivo=0;
	if (get_file_type_from_stat(&esxdos_fopen_files[file_handler].last_file_buf_stat)==2) {
		debug_printf (VERBOSE_DEBUG,"ESXDOS handler: fstat: is a directory");
		atributo_archivo|=16;
	}

	poke_byte_no_time((*registro_parametros_hl_ix)+2,atributo_archivo); //attrs

	//Fecha
	int hora=11;
	int minuto=15;
	int doblesegundos=20*2;

	int anyo=37;
	int mes=9;
	int dia=18;


	get_file_date_from_stat(&esxdos_fopen_files[file_handler].last_file_buf_stat,&hora,&minuto,&doblesegundos,&dia,&mes,&anyo);

	doblesegundos *=2;
	anyo -=1980;


	esxdos_handler_fill_date_struct((*registro_parametros_hl_ix)+3,hora,minuto,doblesegundos,dia,mes,anyo);

	z80_long_int size=esxdos_fopen_files[file_handler].last_file_buf_stat.st_size;
	esxdos_handler_fill_size_struct((*registro_parametros_hl_ix)+7,size);

	esxdos_handler_no_error_uncarry();
	esxdos_handler_old_return_call();

}

void esxdos_handler_run_normal_rst8(void)
{
	debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Running normal rst 8 call");
	rst(8);
}

void esxdos_handler_call_disk_status(void)
{

	//Si primer disco, retornar ok
	if (reg_a==0x80) esxdos_handler_no_error_uncarry();
	else esxdos_handler_error_carry(ESXDOS_ERROR_ENODRV);
}

void esxdos_handler_call_m_drive_info(void)
{
	//Deducir:
	//Entrada: a=numero de disco
	//Retorno: a=numero de particiones. Escrito en HL
/*
Info de David Pesqueira:
M_DRIVEINFO 0x8a
================
HL -> Dirección del buffer
Formato de la información devuelta:
Offset Size Description
0   1  byte   Drive unit (40h, 41h... '@','A'... hd1, hd2...)
1   1  byte   Device
2   1  byte   Flags
3   1  dword  Drive size in 512 bytes blocks (little-endian)
7   -  asciiz File System Type
-   -  asciiz Volume Label
*/

	//char *texto="HO";

	z80_int puntero=*registro_parametros_hl_ix;

	int i;

	/*for (i=0;texto[i];i++) {
		poke_byte_no_time((*registro_parametros_hl_ix),texto[i]);
	}*/

	z80_byte partinfo1=0x00| (('s'-96)<<3); //3 bits bajos: numero particion. 5 bits altos: letra-96
	poke_byte_no_time(puntero++,partinfo1);

	z80_byte partinfo2=0xFF; //disk-info??
	poke_byte_no_time(puntero++,partinfo2);

	z80_byte partinfo3=0xFF;
	poke_byte_no_time(puntero++,partinfo3);

	//Size??
	z80_byte partinfo4=0xFF;
	poke_byte_no_time(puntero++,partinfo4);

	z80_byte partinfo5=0xFF;
	poke_byte_no_time(puntero++,partinfo5);

	z80_byte partinfo6=0xFF;
	poke_byte_no_time(puntero++,partinfo6);

	z80_byte partinfo7=0xFF;
	poke_byte_no_time(puntero++,partinfo7);

	//Name?
	char *fsname="ZXFAT";

	for (i=0;fsname[i];i++) {
		poke_byte_no_time(puntero++,fsname[i]);
	}

	poke_byte_no_time(puntero++,0);

	//Label?
	char *fslabel="ESXHandler";

	for (i=0;fslabel[i];i++) {
		poke_byte_no_time(puntero++,fslabel[i]);
	}

	poke_byte_no_time(puntero++,0);


	reg_a=1; //Numero particiones en a


}

void esxdos_handler_call_disk_info(void)
{
/*
DISK_INFO: If A=0 -> Get a buffer at address HL filled with a list of available block devices. If A<>0 -> Get info for a specific device. Buffer format:

<byte>  Device Path (see below)
<byte>  Device Flags (to be documented, block size, etc)
<dword> Device size in blocks

The buffer is over when you read a Device Path and you get a 0. FIXME: Make so that on return A=# of devs

; Device Entry Description
;
; [BYTE] DEVICE PATH
;
; ---------------------------------
; |       MAJOR       |  MINOR    |
; +-------------------------------+
; | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
; +---+---+---+---+---+---+---+---+
; | E | D | C | B   B | A   A   A |
;
;
; A: MINOR
; --------
; 000 : RAW (whole device)
; 001 : 0       (first partition/session)
; 010 : 1       (second partition/session)
; 011 : 2       (etc...)
; 100 : 3
; 101 : 4
; 110 : 5
; 111 : 6
;
; B:
; --
; 00 : RESERVED
; 01 : IDE
; 10 : FLOPPY
; 11 : VIRTUAL
;
; C:
; --
; 0 : PRIMARY
; 1 : SECONDARY
;
; D:
; --
; 0 : MASTER
; 1 : SLAVE
;
; E:
; --
; 0 : ATA
; 1 : ATAPI

This needs changing/fixing for virtual devs, etc.

*/
	z80_int puntero=*registro_parametros_hl_ix;

	//int i;

	//Retornar el primer disco solo
	z80_byte byte_info=8; //IDE

	//DISK_INFO: If A=0 -> Get a buffer at address HL filled with a list of available block devices. If A<>0 -> Get info for a specific device. Buffer format:
	poke_byte_no_time(puntero++,byte_info);


	//Flags
	poke_byte_no_time(puntero++,0xFF);

	//Dword size
	poke_byte_no_time(puntero++,0xFF);
	poke_byte_no_time(puntero++,0xFF);
	poke_byte_no_time(puntero++,0xFF);
	poke_byte_no_time(puntero++,0xFF);

	//Disk label.
	//Label?
	/*char *disklabel="ZEsarUX";

	for (i=0;disklabel[i];i++) {
		poke_byte_no_time(puntero++,disklabel[i]);
	}
	*/

	if (reg_a==0) {
		//Siguiente byte a 0
		poke_byte_no_time(puntero++,0);
	}

	esxdos_handler_no_error_uncarry();

}

void esxdos_handler_begin_handling_commands(void)
{
	z80_byte funcion=peek_byte_no_time(reg_pc);

	char buffer_fichero[256];
	char buffer_fichero2[256];

	z80_byte f_seek_mode;

	switch (funcion)
	{

		case ESXDOS_RST8_DISK_STATUS:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_DISK_STATUS. A register: %02XH",reg_a);
			esxdos_handler_call_disk_status();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_DISK_READ:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_DISK_READ");
			esxdos_handler_run_normal_rst8();
		break;

		case ESXDOS_RST8_DISK_INFO:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_DISK_INFO. A register: %02XH",reg_a);
			esxdos_handler_call_disk_info();
			//esxdos_handler_run_normal_rst8();
			//esxdos_handler_no_error_uncarry();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_M_DRIVEINFO:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_M_DRIVE_INFO. A register: %02XH",reg_a);
			esxdos_handler_call_m_drive_info();
			//esxdos_handler_no_error_uncarry();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_MOUNT:
			//Pues de momento retornar ok tal cual
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_MOUNT. A register: %02XH",reg_a);
			esxdos_handler_no_error_uncarry();
			esxdos_handler_new_return_call();
		break;


		case ESXDOS_RST8_M_GETSETDRV:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_M_GETSETDRV");
			//M_GETSETDRV: If A=0 -> Get default drive in A. Else set default drive passed in A.

			/*
			; --------------------------------------------------
; BIT   |         7-3           |       2-0        |
; --------------------------------------------------
;       | Drive letter from A-Z | Drive number 0-7 |
; --------------------------------------------------
*/
			if (reg_a==0) {
				reg_a=(8<<3); //1=a, 2=b, .... 8=h
			}
			esxdos_handler_no_error_uncarry();
			esxdos_handler_new_return_call();
	  break;

		case ESXDOS_RST8_F_OPEN:

			esxdos_handler_copy_hl_to_string(buffer_fichero);
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_OPEN. Mode: %02XH File: %s",reg_b,buffer_fichero);
			esxdos_handler_call_f_open();
			esxdos_handler_new_return_call();

		break;

		case ESXDOS_RST8_F_CLOSE:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_CLOSE");
			esxdos_handler_call_f_close();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_SYNC:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_SYNC");
			esxdos_handler_call_f_sync();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_READ:
		//Read BC bytes at HL from file handle A.
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_READ. Read %d bytes at %04XH from file handle %d",reg_bc,(*registro_parametros_hl_ix),reg_a);
			esxdos_handler_call_f_read();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_WRITE:
		//Write BC bytes at HL from file handle A.
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_Write. Write %d bytes from %04XH from file handle %d",reg_bc,(*registro_parametros_hl_ix),reg_a);

			//temp
			//debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_Write. content 1 byte: %02XH",peek_byte_no_time(*registro_parametros_hl_ix));

			esxdos_handler_call_f_write();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_SEEK:
			//f_seek_mode=(z80_byte) *registro_parametros_hl_ix;

			//Mas elegante asi:
			f_seek_mode=(z80_byte) ((*registro_parametros_hl_ix) & 0xff);

			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_SEEK. Move %04X%04XH bytes mode %d from file handle %d",reg_bc,reg_de,f_seek_mode,reg_a);
			esxdos_handler_call_f_seek();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_GETCWD:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_GETCWD");
			esxdos_handler_call_f_getcwd();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_CHDIR:
			esxdos_handler_copy_hl_to_string(buffer_fichero);
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_CHDIR: %s",buffer_fichero);
			esxdos_handler_call_f_chdir();
			esxdos_handler_new_return_call();
		break;


		case ESXDOS_RST8_F_MKDIR:
			esxdos_handler_copy_hl_to_string(buffer_fichero);
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_MKDIR: %s",buffer_fichero);
			esxdos_handler_call_f_mkdir();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_STAT:
			esxdos_handler_copy_hl_to_string(buffer_fichero);
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_STAT: %s",buffer_fichero);
			esxdos_handler_call_f_stat();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_UNLINK:
			esxdos_handler_copy_hl_to_string(buffer_fichero);
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_UNLINK: %s",buffer_fichero);
			esxdos_handler_call_f_unlink();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_RENAME:
			esxdos_handler_copy_register_to_string(buffer_fichero,*registro_parametros_hl_ix);
			esxdos_handler_copy_register_to_string(buffer_fichero2,reg_de);
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_RENAME: %s to %s",buffer_fichero,buffer_fichero2);
			esxdos_handler_call_f_rename();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_OPENDIR:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_OPENDIR");
			esxdos_handler_call_f_opendir();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_READDIR:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_READDIR");
			esxdos_handler_call_f_readdir();
			//printf ("Codigos retorno: A=%02XH HL=%02XH carry=%d PC=%02XH\n",reg_a,HL,Z80_FLAGS & FLAG_C,reg_pc);
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_TELLDIR:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_TELLDIR");
			esxdos_handler_call_f_telldir();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_SEEKDIR:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_SEEKDIR. Offset: %04X%04XH",reg_bc,reg_de);
			//printf ("dir handle: %02XH Offset %04X%04X\n",reg_a,BC,DE);
		//temporal. SEEKDIR. F_SEEKDIR: Sets offset of directory. A=dir handle, BCDE=offset
			esxdos_handler_call_f_seekdir();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_REWINDDIR:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_REWINDDIR");
			esxdos_handler_call_f_rewinddir();
			esxdos_handler_new_return_call();
		break;

		case ESXDOS_RST8_F_FSTAT:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_F_FSTAT");
			esxdos_handler_call_f_fstat();
			esxdos_handler_new_return_call();
		break;

		case 0xB3:
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Unknown ESXDOS_RST8 B3H. Return ok");
		//desconocida. salta cuando se hace un LOAD *"NOMBRE"
		//hace un fread con flags  FA_READ|FA_USE_HEADER  y luego llama a este 0xB3
			esxdos_handler_no_error_uncarry();
			esxdos_handler_new_return_call();
		break;

		/*case ESXDOS_RST8_DISK_IOCTL:
			//Ni idea de que hace esto
			debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Unimplemented ESXDOS_RST8_DISK_IOCTL. Return ok. PC=%XH",reg_pc);
		//desconocida. salta al hacer "list" en tr-dos con una imagen trd montada
			esxdos_handler_run_normal_rst8();

			//reg_a=0xff; //???
			//poke_byte_no_time(0x2382,0x81);
			esxdos_handler_no_error_uncarry();
			esxdos_handler_old_return_call();
		break;*/

        case ESXDOS_RST8_DISK_FILEMAP:
            //De momento esto solo lo he encontrado en el Pogie de Next
            if (MACHINE_IS_TBBLUE) {
                /*
; *************************************************************************** ; * DISK_FILEMAP ($85) * ; ***************************************************************************
; Obtain a map of card addresses describing the space occupied by the file.
; Can be called multiple times if buffer is filled, continuing from previous. ; Entry:
; A=file handle (just opened, or following previous DISK_FILEMAP calls)
;       IX [HL from dot command]=buffer (must be located >= $4000)
;       DE=max entries (each 6 bytes: 4 byte address, 2 byte sector count)
; Exit (success):
; Fc=0
; DE=max entries-number of entries returned
;       HL=address in buffer after last entry
;       A=card flags: bit 0=card id (0 or 1)
;
; Exit (failure):
; Fc=1
;       A=error
                */
                debug_printf (VERBOSE_DEBUG,"ESXDOS handler: ESXDOS_RST8_DISK_FILEMAP. File handle: %02XH DE=%04XH PC=%04XH",reg_a,DE,reg_pc);
                //de momento dar error, no tengo claro como simular esto mediante esxdos handler, o directamente no tiene sentido
                //a no ser que uses una MMC real
                //DE=0;
                //reg_a=0;
	            //esxdos_handler_no_error_uncarry();
                esxdos_handler_error_carry(ESXDOS_ERROR_EBADF);

                esxdos_handler_new_return_call();
            }
            else {
               debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Unhandled ESXDOS_RST8 : %02XH (DISK_FILEMAP)!! ",funcion);
               rst(8);
            }
        break;


		default:
			if (funcion>=0x80) {
				debug_printf (VERBOSE_DEBUG,"ESXDOS handler: Unhandled ESXDOS_RST8: %02XH !! ",funcion);
				char buffer_registros[1024];
				print_registers(buffer_registros);
				debug_printf (VERBOSE_DEBUG,"ESXDOS handler: %s",buffer_registros);

			}
			rst(8); //No queremos que muestre mensaje de debug
			//esxdos_handler_run_normal_rst8();
		break;
	}
}


void esxdos_handler_run(void)
{
/*
RST $08
-------

Main syscall entry point. Parameters are the same for code inside ESXDOS (.commands, etc) and on speccy ram, EXCEPT on speccy RAM you must use IX instead of HL.


*/
	//Ver si se usa IX o HL

	registro_parametros_hl_ix=&reg_hl;
	if (reg_pc>16383) {
		debug_printf(VERBOSE_DEBUG,"Using IX register instead of HL because PC>16383");
		registro_parametros_hl_ix=&reg_ix;
	}

	esxdos_handler_begin_handling_commands();

}

void esxdos_handler_reset(void)
{
	//Inicializar array de archivos abiertos
	//esxdos_fopen_files[ESXDOS_MAX_OPEN_FILES];
	debug_printf(VERBOSE_DEBUG,"Clearing esxdos file handler open files list");
	int i;
	for (i=0;i<ESXDOS_MAX_OPEN_FILES;i++) {
		esxdos_fopen_files[i].open_file.v=0;
	}
}

void esxdos_handler_enable(void)
{

	//esto solo puede pasar activandolo por linea de comandos
	if (!MACHINE_IS_SPECTRUM) {
		debug_printf (VERBOSE_INFO,"ESXDOS handler can only be enabled on Spectrum");
		return;
	}

	//De momento permitir activar esto aunque paging este desactivado. Lo hago por dos razones
	//1. tbblue debe arrancar con paging deshabilitado. Entonces cuando se arrancase tbblue y se tuviese esxdos handler activo en config, fallaria
	//2. para quiza en un futuro permitir este handler aunque no haya nada de divide/divmmc activado

	//Si no esta diviface paging
	//if (diviface_enabled.v==0) {
	//	debug_printf(VERBOSE_ERR,"ESXDOS handler needs divmmc or divide paging emulation");
	//	return;
	//}

	debug_printf(VERBOSE_DEBUG,"Enabling ESXDOS handler");

	//root dir se pone directorio actual si esta vacio
	if (esxdos_handler_root_dir[0]==0) getcwd(esxdos_handler_root_dir,PATH_MAX);

	esxdos_handler_enabled.v=1;
	//directorio  vacio
	esxdos_handler_cwd[0]=0;

	esxdos_handler_reset();
}



void esxdos_handler_disable(void)
{
	debug_printf(VERBOSE_DEBUG,"Disabling ESXDOS handler");
	esxdos_handler_enabled.v=0;
}
