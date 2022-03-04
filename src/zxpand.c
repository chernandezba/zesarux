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
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>



#include "zxpand.h"
#include "mmc.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "menu.h"
#include "screen.h"
#include "zx8081.h"
#include "joystick.h"

z80_bit zxpand_enabled={0};


z80_byte *zxpand_memory_pointer;

//int zxpand_operating_counter=0;

//Si rom zxpand esta activa (1) o no (0)
z80_bit zxpand_overlay_rom={0};



//Todo el mensaje mas el 40, 1 del principio mas el FF del final no pueden ser mayor que 32. O sea el mensaje debe ser maximo 32-3=29
//1234567890123456789012345678901
//                         3.1-SN
char *zxpand_config_message="ZXpand ZEsarUX " EMULATOR_VERSION;

//Al leer directorio se usa zxpand_root_dir y zxpand_cwd
char zxpand_root_dir[PATH_MAX]="";
char zxpand_cwd[PATH_MAX]="";

/*
 * La rom del zxpand debe cargar en otra pagina de memoria aparte , y no en la zona de memoria_spectrum,
 * porque parece que esta rom no tiene tabla de caracteres, y dicha tabla de caracteres la ula la carga siempre de la rom principal
 * Por eso es que tenemos zxpand_memory_pointer=malloc(8192); y la rutina de peek byte de zx80 mira cuando zxpand esta enabled o no
 */

#define ZXPAND_ZX81_ROM_NAME "zxpand_zx81.rom"
#define ZXPAND_ZX80_ROM_NAME "zxpand_zx80.rom"



//Configuration byte parameters
z80_byte zxpand_configByte;

z80_bit dragons_lair_hack={0};
z80_byte dragons_lair_hack_counter=0;

FILE *ptr_zxpand_read_file_command=NULL;

/*
 * >> -variable LATD holds the value returned to a IN opcode
 * >> -variable PORTD holds the value sent to a OUT opcode
 * >> -variable PORTA is the 8 bit high port address (>>5) send to a IN or OUT function
 */
z80_byte zxpand_latd;
z80_byte zxpand_portd;
z80_byte zxpand_porta;

z80_byte zxpand_globalindex;
z80_byte zxpand_globaldata[256];

//usado en lectura de archivos
z80_int zxpand_globalAmount;

//donde guardar nombre fichero
z80_byte *zxpand_fp_fn = &zxpand_globaldata[128];

//Variables al abrir fichero
z80_int zxpand_flags;
z80_int zxpand_start;
z80_int zxpand_length;


//Control de reescritura de archivos
z80_byte zxpand_fsConfig=0;


char *zxpand_texto_comandos[8]={
	"data channel",
	"get data from globalbuffer",
	"put data into globalbuffer",
	"directory control port",
	"file control port",
	"get data from file",
	"put data to file",
	"interface control"
};

char zxpand_defaultExtension;
z80_int zxpand_defaultLoadAddr;


//El texto original era:
//char zx80Token2ascii[] = ";,()?-+*/???=<>";
/*
Pero esto suele generar el warning
warning: trigraph ??= ignored, use -trigraphs to enable [-Wtrigraphs]
En algunos compiladores viejos (y si se activase -trigraphs) esa sentencia ??= se sustituiria por #
Esto no sucedera, pero aun asi lo escapamos y asi tampoco se queja con warning
https://gcc.gnu.org/onlinedocs/cpp/Initial-processing.html

*/
char zx80Token2ascii[] = ";,()?-+*/??\?=<>";


char *SEMICOL = ";";
char *SEPARATOR = "=";

//No usados este:
//static const rom char* EIGHT40 =   "8-40K";



char *SIXTEEN48 = "16-48K";

char *EIGHT48 = "8-48K";


//usados al leer directorio
z80_byte zxpand_filinfo_fattrib;
struct dirent *zxpand_dp;
DIR *zxpand_dfd=NULL;


//ultimo directorio leido al listar archivos
char zxpand_last_dir_open[PATH_MAX]=".";

// take the character required for the equivalent joystick - 0x1b and
// report the offset at that index; Z = 0x3f. 0x3f-0x1b = 36d. offs2char[36] = 0.
//
// '0', or space, is hardcoded as 34d, n/l as 29d
//
static z80_byte zxpand_char2offs[] =
{
   35,19, // 1b
   14,15, // 1d
   16,17, // 1f
   18,23, // 21
   22,21, // 23
   20, 4, // 25
   38, 2, // 27
    6,11, // 29
    7, 8, // 2b
   33,26, // 2d
   32,31, // 2f
   30,36, // 31
   37,25, // 33
   24, 9, // 35
   12, 5, // 37
   13,27, // 39
    3,10, // 3b
    1,28, // 3d
    0, 0  // 3f + pad
};


z80_byte zxpand_jsmap[6];
void zxpand_changedir(char *d);



void zxpand_alloc_mem(void)
{
	zxpand_memory_pointer=malloc(8192);
	if (zxpand_memory_pointer==NULL) cpu_panic ("Can not allocate memory for zxpand ROM");
}

void zxpand_footer_print_zxpand_operating(void)
{

	generic_footertext_print_operating("ZXPAND");

	//Y poner icono en inverso
	if (!zxdesktop_icon_zxpand_inverse) {
			zxdesktop_icon_zxpand_inverse=1;
			menu_draw_ext_desktop();
	}		
}

void zxpand_footer_zxpand_operating(void)
{

		zxpand_footer_print_zxpand_operating();

}






//0 si ok
int zxpand_load_rom_overlay(void)
{
	FILE *ptr_zxpand_romfile;
	int leidos=0;

	char *romfile=ZXPAND_ZX81_ROM_NAME;

	if (MACHINE_IS_ZX80) romfile=ZXPAND_ZX80_ROM_NAME;

	debug_printf (VERBOSE_INFO,"Loading zxpand rom %s",romfile);

	//ptr_zxpand_romfile=fopen(ZXPAND_ROM_NAME,"rb");
	open_sharedfile(romfile,&ptr_zxpand_romfile);


	if (ptr_zxpand_romfile!=NULL) {

		leidos=fread(zxpand_memory_pointer,1,8192,ptr_zxpand_romfile);
		//leidos=fread(memoria_spectrum,1,8192,ptr_zxpand_romfile);
		fclose(ptr_zxpand_romfile);

	}



	if (leidos!=8192 || ptr_zxpand_romfile==NULL) {
		debug_printf (VERBOSE_ERR,"Error reading ZXPAND rom %s. Disabling ZXpand",romfile);
		zxpand_disable();
		return 1;
	}

	return 0;
}


//void zxpand_load_normal_rom(void)
//{
//	rom_load(NULL);
//}

void zxpand_enable(void)
{

	//esto solo puede pasar activandolo por linea de comandos
	if (!MACHINE_IS_ZX8081) {
		debug_printf (VERBOSE_INFO,"ZXpand can only be enabled on ZX80/81");
		return;
	}

	//asignar memoria
	zxpand_alloc_mem();

	//Cuando hay reset (o hard reset) hay que volver a meter activa esta rom
	zxpand_overlay_rom.v=1;

	//Volver si error
	if (zxpand_load_rom_overlay()) return;

	//ZXpand habilita 32kb. En las direcciones 8-40k o bien 16-48k
	//de momento yo habilito desde 8-48k
	enable_ram_in_32768();
	ram_in_8192.v=1;

	//habilitar real video y wrx
	enable_rainbow();
	enable_wrx();


	zxpand_enabled.v=1;

	//root dir se pone directorio actual si esta vacio
	if (zxpand_root_dir[0]==0) getcwd(zxpand_root_dir,PATH_MAX);

	//directorio zxpand vacio
	zxpand_cwd[0]=0;

	//configbyte a FF
	zxpand_configByte=0xFF;

	//desactivar dragons lair hack
	dragons_lair_hack.v=0;

	//Asignamos joystick tipo zxpand
	joystick_emulation=JOYSTICK_ZXPAND;

	//array jsmap. estos son valores estaticos
	//The default programming for the joystick INKEY$ emulation is cursor keys + 0, i.e CONFIG "J=76580"
	//z80_byte zxpand_jsmap[6];
	zxpand_jsmap[0]=zxpand_char2offs[0x23-0x1b];  //0x23 is char number for '7'
	zxpand_jsmap[1]=zxpand_char2offs[0x22-0x1b];  //0x22 is char number for '6'
	zxpand_jsmap[2]=zxpand_char2offs[0x21-0x1b];  //0x21 is char number for '5'
	zxpand_jsmap[3]=zxpand_char2offs[0x24-0x1b];  //0x24 is char number for '8'
	zxpand_jsmap[4]=zxpand_char2offs[0x1c-0x1b];  //0x1c is char number for '0'


}

void zxpand_disable(void)
{
	zxpand_overlay_rom.v=0;
	//zxpand_load_normal_rom();
	zxpand_enabled.v=0;
}






//Convertir cadena de texto de zx81 a ascii
void zxpand_deZeddify(unsigned char* buffer)
{
	unsigned char q;
	while (*buffer)
	{
		q = *buffer;
		if (q & 0x40)
		{
			if (q > 0xd6 && q < 0xe6)
			{
				*buffer = zx80Token2ascii[q - 0xd7];
			}
			else
			{
				*buffer = '?';
			}
		}
		else
		{
			*buffer = da_codigo_zx81_no_artistic(q & 0x3f);
		}

		++buffer;
	}

	//temp
	//ver contenido de zx80Token2ascii
	//int i;
	//for (i=0;i<strlen(zx80Token2ascii);i++) printf ("%c",zx80Token2ascii[i]);
	//printf (" %d\n",strlen(zx80Token2ascii) );

}

//Convertir cadena de texto ascii a zx81
void zxpand_zeddify(unsigned char *buffer)
{
	while (*buffer)
	{
		*buffer = ascii_to_zx81(*buffer);
		++buffer;
	}
	// force 'zeddy' type termination
	*buffer = 255;
}




// check that the supplied ascii filename only consists of alphanums slash and dot.
char zxpand_isValidFN(unsigned char* buffer)
{

	char c;

	while (*buffer)
	{
		//char c = chk_chr(validfns, *buffer);

		c=*buffer;

		if (
			!
			(
				(c>='A' && c<='Z') ||
				(c>='0' && c<'9') ||
				(c=='/') ||
				(c==';') ||
				(c=='.')

			)
		)
		{
			c=0;
		}

		if (c == 0 || c == ';') return c;
		++buffer;
	}

	return 1;
}




//Funciones de gestion de comandos

//modo lectura o escritura
//rellena fullpath con ruta completa
z80_byte zxpand_fileopen(char *modo,char *fullpath)
{

	char* token;
	char autogenext = 1;
	char* p = (char*)zxpand_globaldata;

	zxpand_deZeddify((unsigned char *)p);

	if (*p == '+' || *p == '>')
	{
		++p;
	}

	if (*p == '/')
	{
		autogenext = 0;
	}

	if (!zxpand_isValidFN((unsigned char*)p))
	{
		return 0x40 + ZXPAND_FR_INVALID_NAME;
	}

	//token = strtokpgmram(p, (RFC)SEMICOL);
	token = strtok(p, SEMICOL);
	if (token==NULL)
	{
		// no filename specified
		return 0x40 + ZXPAND_FR_INVALID_NAME;
	}

	// change $ for 0x7e, to support short LFN forms
	{
		if (p[6] == '$')
		{
			p[6] = 0x7e;
		}
	}


	zxpand_start = zxpand_defaultLoadAddr;
	zxpand_length = 0;
	zxpand_flags = 0;

	// parse optional parameters
	//
	//while ((token = strtokpgmram((char*)NULL, (RFC)SEMICOL)) != NULL)
	while ((token = strtok((char*)NULL, SEMICOL)) != NULL)
	{
		// if it starts with an alpha then it's a flag - add it to the bitset
		if (isalpha(*token))
		{
			if (*token == 'X')
			{
				zxpand_flags |= 1;
			}
		}
		else
		{
			// see if it's of the form start,end: if not it's just start
			//
			char* comma = strchr(token,',');
			zxpand_start = atoi(token);
			if (comma)
			{
				zxpand_length = atoi(comma+1);
			}
		}
	}

	// now all the params are parsed, we can create the filename
	//
	{
		char* newFN = (char *) zxpand_fp_fn;
		char found = 0;
		for(token = p; *token; ++token, ++newFN)
		{
			*newFN = *token;
			if (*token == '.')
			{
				found = 1;
			}
		}
		*newFN = 0;
		if (!found && autogenext)
		{
			*newFN = '.';
			++newFN;
			*newFN = zxpand_defaultExtension;
			++newFN;
			*newFN = 0;
		}
	}



	//TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a zxpand_root_dir
	sprintf (fullpath,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,(char *)zxpand_fp_fn);


	int existe_archivo=si_existe_archivo(fullpath);



	//Si no existe buscar archivo sin comparar mayusculas/minusculas
	//en otras partes del codigo directamente se busca primero el archivo con util_busca_archivo_nocase,
	//aqui se hace al reves. Se busca normal, y si no se encuentra, se busca nocase
	//if (!strcmp(modo,"rb")) {

		if (!existe_archivo) {
			debug_printf (VERBOSE_DEBUG,"File %s not found. Searching without case sensitive",fullpath);
			char encontrado[PATH_MAX];
			char directorio[PATH_MAX];
			util_get_complete_path(zxpand_root_dir,zxpand_cwd,directorio);
			//sprintf (directorio,"%s/%s",zxpand_root_dir,zxpand_cwd);
			if (util_busca_archivo_nocase ((char *)zxpand_fp_fn,directorio,encontrado) ) {
				debug_printf (VERBOSE_DEBUG,"Found with name %s",encontrado);
				existe_archivo=1;

				//cambiamos el nombre fullpath y el zxpand_fp_fn por el encontrado
				sprintf ((char *)zxpand_fp_fn,"%s",encontrado);

				sprintf (fullpath,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,(char *)zxpand_fp_fn);
				//sprintf (fullpath,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,encontrado);
				debug_printf (VERBOSE_DEBUG,"Found file %s searching without case sensitive",fullpath);
			}
		}
	//}

	//Si modo es escritura (wb), si existe, no sobreescribir
	if (!strcmp(modo,"wb")) {
		if (existe_archivo) {
			debug_printf (VERBOSE_DEBUG,"File %s already exists",fullpath);
			return 0x48;
		}
	}

	debug_printf (VERBOSE_DEBUG,"Calling fopen filename %s (fullpath: %s) mode %s",zxpand_fp_fn,fullpath,modo);

	ptr_zxpand_read_file_command=fopen(fullpath,modo);
	if (ptr_zxpand_read_file_command==NULL) {
		//file not found.
		return 0x40+ZXPAND_FR_NO_FILE;
	}

	return 0x40;


}

void zxpand_COM_FileOpenRead(void)
{

	//printf ("zxpand_COM_FileOpenRead\n");
	char fullpath[PATH_MAX];
	z80_byte res=zxpand_fileopen("rb",fullpath);

	if (res==0x40)
	{
		long int longitud_total=get_file_size(fullpath);


		if (zxpand_length == 0)
		{
			zxpand_length = longitud_total & 65535;
		}

		// hack to make programs auto-disable ROM if read-only attribute is set
		//if (zxpand_filinfo_fattrib & AM_RDO)
		//{
		//   flags |= 1;
		//}

		zxpand_globaldata[0] = zxpand_length & 255;
		zxpand_globaldata[1] = zxpand_length / 256;
		zxpand_globaldata[2] = zxpand_start & 255;
		zxpand_globaldata[3] = zxpand_start / 256;
		zxpand_globaldata[4] = zxpand_flags & 255;
		zxpand_globaldata[5] = zxpand_flags / 256;

		zxpand_globaldata[6] = longitud_total & 0xff;
		zxpand_globaldata[7] = (longitud_total >>  8) & 0xff;
		zxpand_globaldata[8] = (longitud_total >> 16) & 0xff;
		zxpand_globaldata[9] = (longitud_total >> 24) & 0xff;

		memset(&zxpand_globaldata[10], 0, 32-10);

		//printf ("zxpand_start: 0x%X zxpand_length: 0x%X zxpand_flags: 0x%X\n",zxpand_start,zxpand_length,zxpand_flags);

	}

	zxpand_latd = res;

}


//Retorna la ruta a un archivo sin tener en cuenta mayusculas/minusculas
//Retorna 1 si encontrado. 0 si no
//Orig: ruta de entrada. fullpath: ruta de salida
int zxpand_get_lowercase_path(char *orig,char *fullpath)
{

        char nombre_minus_encontrado[PATH_MAX];
        char nombre_a_buscar[PATH_MAX];
        char dir[PATH_MAX];

        util_get_dir(orig,dir);

        util_get_file_no_directory(orig,nombre_a_buscar);


        if (util_busca_archivo_nocase(nombre_a_buscar,dir,nombre_minus_encontrado)) {
                //Retornar fullpath
		util_get_complete_path(dir,nombre_minus_encontrado,fullpath);
                //sprintf (fullpath,"%s/%s",dir,nombre_minus_encontrado);
                debug_printf (VERBOSE_DEBUG,"Found file with non-uppercase name: %s",fullpath);
		return 1;
        }
	return 0;
}


//Devuelve codigo error zxpand. 0 si ok, etc..
int zxpand_rename(char *orig, char *dest)
{
	debug_printf (VERBOSE_DEBUG,"Renaming file %s to %s",orig,dest);
	char lowercasepath[PATH_MAX];
	if (zxpand_get_lowercase_path(orig,lowercasepath)==0) {
		//no encontrado
		return ZXPAND_FR_NO_FILE;
	}

	//Hacer rename
	if (rename(lowercasepath,dest)==0) return 0;
	return ZXPAND_FR_INVALID_NAME;


	//Aqui ya no se llega
	//TODO: borrar esto hasta final de funcion si todo va bien


	if (rename(orig,dest)==0) return 0;

	//Si falla el rename, probar antes que no vaya a ser que el archivo origen no este en mayusculas
	char nombre_minus_encontrado[PATH_MAX];
	char nombre_a_buscar[PATH_MAX];
	char dir[PATH_MAX];

	util_get_dir(orig,dir);

	util_get_file_no_directory(orig,nombre_a_buscar);


	if (util_busca_archivo_nocase(nombre_a_buscar,dir,nombre_minus_encontrado)) {
		//Probar de nuevo el rename
		char fullpath[PATH_MAX];
		util_get_complete_path(dir,nombre_minus_encontrado,fullpath);
		//sprintf (fullpath,"%s/%s",dir,nombre_minus_encontrado);
		debug_printf (VERBOSE_DEBUG,"Found source file with non-uppercase name: %s. Trying again to rename",fullpath);
		if (rename(fullpath,dest)==0) return 0;
	}


	//Cualquier otra cosa, error
	return ZXPAND_FR_INVALID_NAME;

	/*else {
		//no lo ha encontrado. devolver error
		return ZXPAND_FR_INVALID_NAME;
	}

	//Si falla el rename, devolver invalid name.
	if (rename(orig,dest)!=0) return ZXPAND_FR_INVALID_NAME;
	else return 0;
	*/
}

void zxpand_COM_FileOpenWrite(void)
{
	char fullpath[PATH_MAX];
	z80_byte res=zxpand_fileopen("wb",fullpath);

	  if (res == 0x48)
	  {
	  	// file exists
		  if (zxpand_globaldata[0] == '+' || (zxpand_fsConfig & 0x03) == 1)
		  {
			char zxpand_fp_fnBak[PATH_MAX];
		 	char* p = zxpand_fp_fnBak;
	 		memcpy((void*)zxpand_fp_fnBak, (void*)zxpand_fp_fn, 32);
		 	while(*p != '.'){++p;}
		 	strcpy(p, ".BAK");

			char orig[PATH_MAX],dest[PATH_MAX];
			  //TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a zxpand_root_dir
	                sprintf (orig,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,(char *)zxpand_fp_fn);
        	        sprintf (dest,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,(char *)zxpand_fp_fnBak);

			res = 0x40 | zxpand_rename(orig,dest);
		}

		if (zxpand_globaldata[0] == '>' || (zxpand_fsConfig & 0x03) == 2)
		{
			// overwrite (ala dos pipe)
			res = 0x40;
			if (unlink(fullpath)) res |=ZXPAND_FR_DENIED;
		}

		if (res == 0x40)
		{
			// now try again
			//res = 0x40 | f_open(&fil, zxpand_fp_fn, FA_CREATE_NEW|FA_WRITE);
			//printf ("trying to open again file\n");
		        ptr_zxpand_read_file_command=fopen(fullpath,"wb");
		        if (ptr_zxpand_read_file_command==NULL) {
                		//file not found.
		                res=0x40+ZXPAND_FR_NO_FILE;
		        }
		}
	}

	if (res==0x40)
	{

		long int longitud_total;

		//Si no existe, tiene sentido esto??
		//valores zxpand_length, zxpand_start, zxpand_flags, longitud_total tienen valores indefinidos

		//para que no se queje el compilador
		longitud_total=0;


		zxpand_globaldata[0] = zxpand_length & 255;
		zxpand_globaldata[1] = zxpand_length / 256;
		zxpand_globaldata[2] = zxpand_start & 255;
		zxpand_globaldata[3] = zxpand_start / 256;
		zxpand_globaldata[4] = zxpand_flags & 255;
		zxpand_globaldata[5] = zxpand_flags / 256;

		zxpand_globaldata[6] = longitud_total & 0xff;
		zxpand_globaldata[7] = (longitud_total >>  8) & 0xff;
		zxpand_globaldata[8] = (longitud_total >> 16) & 0xff;
		zxpand_globaldata[9] = (longitud_total >> 24) & 0xff;

		memset(&zxpand_globaldata[10], 0, 32-10);

	}

	zxpand_latd = res;

}

void zxpand_COM_FileClose(void)
{

	//printf ("zxpand_COM_FileClose\n");


	if (ptr_zxpand_read_file_command!=NULL) {
		fclose(ptr_zxpand_read_file_command);
		ptr_zxpand_read_file_command=NULL;
	}

	zxpand_latd=0x40;


}

void zxpand_COM_FileRead(void)
{

	//esto no deberia pasar:
	if (ptr_zxpand_read_file_command==NULL) {
		debug_printf (VERBOSE_DEBUG,"trying to read from a non opened file. error");
		return;
	}


	//z80_int read;

	if (zxpand_globalAmount == 0)
	{
		zxpand_globalAmount = 256;
	}

	//printf ("zxpand_COM_FileRead. reading %d bytes\n",zxpand_globalAmount);

	//read=fread(&zxpand_globaldata,1,zxpand_globalAmount,ptr_zxpand_read_file_command);
	fread(&zxpand_globaldata,1,zxpand_globalAmount,ptr_zxpand_read_file_command);

	//printf ("read: %d bytes\n",read);

	//siempre devolver ok
	zxpand_latd=0x40;

	//Llamadas al filesystem en zxpand original basadas en Chan's fat file system
	//f_read dice:
	/*


Return Values

FR_OK, FR_DISK_ERR, FR_INT_ERR, FR_INVALID_OBJECT, FR_TIMEOUT


Description

The file read/write pointer of the file object advances number of bytes read. After the function succeeded, *br should be checked to detect end of the file. In case of *br is less than btr, it means the read/write pointer reached end of the file during read operation.

	*/
	//por tanto en la llamada original:
	//LATD = 0x40 | f_read(&fil, globalData, globalAmount, &read);
	//no se devuelve error si se leen menos bytes (a no ser, claro esta, que se produzca alguno de :
	//FR_OK, FR_DISK_ERR, FR_INT_ERR, FR_INVALID_OBJECT, FR_TIMEOUT
	//como estos no suceden normalmente, devolver ok, incluso si se leen menos bytes


}


void zxpand_COM_FileWrite(void)
{

	//esto no deberia pasar:
	if (ptr_zxpand_read_file_command==NULL) {
		debug_printf (VERBOSE_DEBUG,"trying to write to a non opened file. error");
		return;
	}

	//z80_int written;

	if (zxpand_globalAmount == 0)
	{
		zxpand_globalAmount = 256;
	}

	//printf ("zxpand_COM_FileWrite. escribiendo %d bytes\n",zxpand_globalAmount);

	//written=fwrite(&zxpand_globaldata,1,zxpand_globalAmount,ptr_zxpand_read_file_command);
	fwrite(&zxpand_globaldata,1,zxpand_globalAmount,ptr_zxpand_read_file_command);

	//printf ("written %d bytes\n",written);


	//siempre devolver ok
	//misma razon que zxpand_COM_FileRead, ver ahi
	zxpand_latd=0x40;

}

void zxpand_COM_FileSeek(void)
{
	//TODO. Esto no se ha probado
	z80_int posicion;
	posicion=value_8_to_16(zxpand_globaldata[1],zxpand_globaldata[0]);
	fseek(ptr_zxpand_read_file_command,posicion,SEEK_SET);

	zxpand_latd=0x40;

}

void zxpand_COM_FileRename(void)
{
	char* token;
	char* p = (char*)zxpand_globaldata;

	char ret = 0x40 + ZXPAND_FR_INVALID_NAME;

	zxpand_deZeddify(zxpand_globaldata);

	token = strtok(p, SEMICOL);
	if (NULL != token)
	{
		token = strtok((char*)NULL, SEMICOL);
		if (NULL != token)
		{
			if (zxpand_isValidFN(zxpand_globaldata) && zxpand_isValidFN((unsigned char *)token))
			{

				debug_printf (VERBOSE_DEBUG,"Called rename %s to %s. Current dir: %s",(char *)&zxpand_globaldata[0],token,zxpand_cwd);
        	                char orig[PATH_MAX],dest[PATH_MAX];
                	        //TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a zxpand_root_dir
                        	sprintf (orig,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,(char *)&zxpand_globaldata[0]);
	                        sprintf (dest,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,token);

				//ret = 0x40 | zxpand_rename((char *)&zxpand_globaldata[0], token);
				ret = 0x40 | zxpand_rename(orig,dest);
			}
		}
	}

	zxpand_latd = ret;

}

void zxpand_COM_FileDelete(void)
{
	//Suponemos error
	z80_byte ret = 0x40 + ZXPAND_FR_INVALID_NAME;

	char fullpath[PATH_MAX];

	zxpand_deZeddify(zxpand_globaldata);
	if (zxpand_isValidFN(zxpand_globaldata))
	{

		//TODO: habria que proteger que en el nombre indicado no se use ../.. para ir a ruta raiz inferior a zxpand_root_dir
		sprintf (fullpath,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,zxpand_globaldata);

		debug_printf (VERBOSE_DEBUG,"Calling file delete file name %s (fullpath: %s)",zxpand_globaldata,fullpath);

		ret=0x40;


	        char lowercasepath[PATH_MAX];
        	if (zxpand_get_lowercase_path(fullpath,lowercasepath)==0) {
	                //no encontrado
        	        ret |=ZXPAND_FR_NO_FILE;
	        }

		else {
			if (unlink(lowercasepath)) ret |=ZXPAND_FR_NO_FILE;
			//Quiza si falla unlink podria ser por permisos, entonces se deberia retornar ZXPAND_FR_WRITE_PROTECTED
		}

		//if (unlink(fullpath)) ret |=ZXPAND_FR_NO_FILE;

	}

	zxpand_latd = ret;

}

void zxpand_go_high(void)
{
	enable_ram_in_32768();
	ram_in_8192.v=0;
}

void zxpand_go_low(void)
{
	enable_ram_in_32768();
	ram_in_8192.v=1;
}



z80_byte zxpand_read_joystick(void)
{


        z80_byte valor_joystick=0xF9;

        //si estamos con menu abierto, no retornar nada
        if (zxvision_key_not_sent_emulated_mach() ) return valor_joystick;


        //Si no es joystick de tipo zxpand, volver
        if (joystick_emulation!=JOYSTICK_ZXPAND) return valor_joystick;


        //0b11111001

        /*
         * Bit -- Direction
         * 7 Up 128
         * 6 Down       64
         * 5 Left       32
         * 4 Right      16
         * 3 Fire       8
         *
         * The corresponding bit will be 0 when the joystick is pushed in that direction. Bits 2 to 0 inclusive are
         * undefined, but you might just work out that bit 0 represents whether a card is present...
         */


        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

        if ((puerto_especial_joystick&1)) valor_joystick &=(255-16);
        if ((puerto_especial_joystick&2)) valor_joystick &=(255-32);
        if ((puerto_especial_joystick&4)) valor_joystick &=(255-64);
        if ((puerto_especial_joystick&8)) valor_joystick &=(255-128);
        if ((puerto_especial_joystick&16)) valor_joystick &=(255-8);
        return valor_joystick;
}


void zxpand_mapJS(z80_byte dirn, z80_byte val)
{
   int q = 0;

   if (val == 0)
   {
      q = 39;
   }
   else
   if (val == 0x76)
   {
      q = 29;
   }
   else
   {
      q = zxpand_char2offs[val - 0x1b];
   }

   zxpand_jsmap[dirn] = q;
}


void zxpand_decodeJS(void)
{
   z80_byte idx = 5;
   z80_byte temp = zxpand_read_joystick();

   // fire gets priority
   //
   if ((temp & 0x08) == 0)
   {
      idx = 4;
   }
   else if((temp & 0x80) == 0)
   {
      idx = 0;
   }
   else if ((temp & 0x40) == 0)
   {
      idx = 1;
   }
   else if ((temp & 0x20) == 0)
   {
      idx = 2;
   }
   else if ((temp & 0x10) == 0)
   {
      idx = 3;
   }

   if (idx != 5)
   {
      zxpand_latd = zxpand_jsmap[idx];
   }
   else
   {
      zxpand_latd = 0xff;
   }
}




//Comando Config
void zxpand_COM_ParseBuffer(void)
{
	char* token;
	z80_byte retcode = 0x40;

	// keep any raw keycodes that might be lost in a conversion
	//
	memcpy((void*)&zxpand_globaldata[128], (void*)&zxpand_globaldata[0], 128);

	zxpand_deZeddify(zxpand_globaldata);

	if(!isalpha(zxpand_globaldata[0]))
	{
		retcode |= ZXPAND_FR_INVALID_OBJECT;
		goto retok;
	}

	// 'A' = 3.
	zxpand_globaldata[0] = zxpand_globaldata[0]-'A'+3;
	//token = strtokpgmram((char*)&zxpand_globaldata[1], (RFC)SEPARATOR);
	token = strtok((char*)&zxpand_globaldata[1], SEPARATOR);

	if (zxpand_globaldata[0] == 'V'-'A'+3)
	{
		// version string

		//strcpypgm2ram((char*)&zxpand_globaldata[1], (RFC)VERSION);

		//zxpand_config_message

		sprintf((char *) &zxpand_globaldata[1],"%s",zxpand_config_message);

		zxpand_zeddify(&zxpand_globaldata[1]);
		zxpand_globaldata[0] = 1;
		goto retok;
	}

	else if (zxpand_globaldata[0] == 'D'-'A'+3)
	{
		// set current working directory

		if (!token)
		{
			zxpand_globaldata[32]='\\';
			zxpand_globaldata[33]=0;
			token = (char*)&zxpand_globaldata[32];
		}

		//retcode |= f_chdir(token);
		retcode=0x40;
		zxpand_changedir(token);
		goto retok;
	}

	else if (zxpand_globaldata[0] == 'M'-'A'+3)
	{
		// memory map control

		if (token)
		{
			if (*token == 'H')
			{
				// HI ram

				//Habilitamos 16-48K
				zxpand_go_high();
			}
			else if (*token == 'L')
			{
				// LO ram
				//Habilitamos 8-48K
				//Deberia ser 8-40K pero este modo en ZEsarUX no esta permitido
				zxpand_go_low();
			}
			else
			{
				retcode |= ZXPAND_FR_INVALID_OBJECT;
				goto retok;
			}
		}
		else
		{
			if (ram_in_8192.v)
			{
				strcpy((char*)&zxpand_globaldata[1], EIGHT48);
			}
			else
			{
				strcpy((char*)&zxpand_globaldata[1], SIXTEEN48);
			}

			zxpand_zeddify(&zxpand_globaldata[1]);
			zxpand_globaldata[0] = 1;
		}

		goto retok;
	}

	else if (zxpand_globaldata[0] == 'C'-'A'+3)
	{
		// config control

		if (token)
		{
			unsigned char n = *token - '0';
			if (n > 9) n -= 7;
			if (n > 15)
			{
				retcode |= ZXPAND_FR_INVALID_OBJECT;
				goto retok;
			}
			++token;
			zxpand_configByte = n * 16;
			n = *token - '0';
			if (n > 9) n -= 7;
			if (n > 15)
			{
				retcode |= ZXPAND_FR_INVALID_OBJECT;
				goto retok;
			}
			zxpand_configByte += n;

			debug_printf (VERBOSE_DEBUG,"Setting zxpand_configByte with %d",zxpand_configByte);

			//TODO WriteEEPROM(0x04, zxpand_configByte);
		}
		else
		{
			unsigned char* p = &zxpand_globaldata[0];
			*p = 1;
			++p;
			*p = ((zxpand_configByte & 0xf0) >> 4) + 0x1c;
			++p;
			*p = (zxpand_configByte & 15) + 0x1c;
			++p;
			*p = 0xff;
			++p;
			*p = zxpand_configByte;
		}

		goto retok;
	}



	 else if (zxpand_globaldata[0] == 'O'-'A'+3)
	 {
		 // overwrite control
		 if (token)
		 {
		 	unsigned char n = *token - '0';
		 	if (n > 2)
			{
				retcode |= ZXPAND_FR_INVALID_OBJECT;
				goto retok;
			}
			zxpand_fsConfig = n;
			//WriteEEPROM(0x05, zxpand_fsConfig);
		}
		else
		{
			unsigned char* p = &zxpand_globaldata[0];
			*p = 1;
			++p;
			if ((zxpand_fsConfig & 3) == 1)
			{
				strcpy((char*)p, "BAK");
			}

			else if ((zxpand_fsConfig & 3) == 2)
			{
				strcpy((char*)p, "OVR");
			}

			else
			{
				strcpy((char*)p, "ERR");
			}
			zxpand_zeddify(p);
			p+= 3;
			*p = 0xff;
		}

		goto retok;
	}


	else if (zxpand_globaldata[0] == 'J'-'A'+3)
	{
		// joystick mapping control

		if (token)
		{
			token += 128;
			zxpand_mapJS(0, *token);
			++token;
			zxpand_mapJS(1, *token);
			++token;
			zxpand_mapJS(2, *token);
			++token;
			zxpand_mapJS(3, *token);
			++token;
			zxpand_mapJS(4, *token & 0x3f); // might have top bit set being last char in string
			// might not. there might not be 4 chars there.
		}
		else
		{
			//printf ("sin token\n");
			//int i;
			//for (i=0;i<6;i++) printf ("i: %d value: 0x%02X\n",i,zxpand_jsmap[i]);

			retcode |= ZXPAND_FR_INVALID_OBJECT;
		}

		goto retok;
	}




	// generic command follows? e.g.:
	// R(amtop),nnnn

	{
		z80_byte n = 0;

		token = strtok((char*)&zxpand_globaldata[0], SEPARATOR);
		if (token)
		{
			// parse optional parameters
			//
			while ((token = strtok((char*)NULL, SEPARATOR)) != NULL)
			{
				zxpand_start = atoi(token);

				//printf ("comando config ramtop. valor: %d\n",zxpand_start);

				zxpand_globaldata[128+n] = zxpand_start & 255;
				zxpand_globaldata[129+n] = zxpand_start / 256;
				n += 2;
			}

			memcpy((void*)&zxpand_globaldata[1], (void*)&zxpand_globaldata[128], n);

			// values have been decoded to the buffer. parameterised commands may now execute



			/* comentado en origen
			 *	// to ensure that a paramterised command has its parameters, put it inside this clause
			 *	//
			 *	if (zxpand_globaldata[0] == 'B'-'A'+3)
			 *	{
			 *	// set bank
			 *
			 *	if (zxpand_globaldata[1])
			 *	{
			 *	GO_BANK1;
		}
		else
		{
		GO_BANK0;
		}
		}
		if (zxpand_globaldata[0] == 'E'-'A'+3)
		{
		retcode |= zxpand_globaldata[1];
		}
		fin comentado en origen  */


		}

		if (n == 0 && zxpand_globaldata[0] == 'R'-'A'+3)
		{
			retcode |= ZXPAND_FR_INVALID_OBJECT;
		}
	}



	retok:
	zxpand_latd = retcode;
}

//tener en cuenta raiz y directorio actual
//si localdir no es NULL, devolver directorio local (quitando zxpand_root_dir)
void zxpand_get_final_directory(char *dir, char *finaldir, char *localdir)
{


	//printf ("zxpand_get_final_directory. dir: %s zxpand_root_dir: %s\n",dir,zxpand_root_dir);

	//Guardamos directorio actual del emulador
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//cambiar a directorio indicado, juntando raiz, dir actual de zxpand, y dir
	char dir_pedido[PATH_MAX];

	//Si directorio pedido es absoluto, cambiar cwd
	if (dir[0]=='/') {
		sprintf (zxpand_cwd,"%s",dir);
		sprintf (dir_pedido,"%s/%s",zxpand_root_dir,zxpand_cwd);
	}


	else {
		sprintf (dir_pedido,"%s/%s/%s",zxpand_root_dir,zxpand_cwd,dir);
	}

	chdir(dir_pedido);

	//Ver en que directorio estamos
	//char dir_final[PATH_MAX];
	getcwd(finaldir,PATH_MAX);
	//printf ("total final directory: %s . zxpand_root_dir: %s\n",finaldir,zxpand_root_dir);
	//Esto suele retornar sim barra al final

	//Si finaldir no tiene barra al final, haremos que zxpand_root_dir tampoco la tenga
	int l=strlen(finaldir);

	if (l) {
		if (finaldir[l-1]!='/' && finaldir[l-1]!='\\') {
			//finaldir no tiene barra al final
			int m=strlen(zxpand_root_dir);
			if (m) {
				if (zxpand_root_dir[m-1]=='/' || zxpand_root_dir[m-1]=='\\') {
					//root dir tiene barra al final. quitarla
					//printf ("quitando barra del final de zxpand_root_dir\n");
					zxpand_root_dir[m-1]=0;
				}
			}
		}
	}


	//Ahora hay que quitar la parte del directorio raiz
	//printf ("running strstr (%s,%s)\n",finaldir,zxpand_root_dir);
	char *s=strstr(finaldir,zxpand_root_dir);

	if (s==NULL) {
		debug_printf (VERBOSE_DEBUG,"Directory change not allowed");
		//directorio final es el mismo que habia
		sprintf (finaldir,"%s",zxpand_cwd);
		return;
	}

	//Si esta bien, meter parte local
	if (localdir!=NULL) {
		int l=strlen(zxpand_root_dir);
		sprintf (localdir,"%s",&finaldir[l]);
		//printf ("local directory: %s\n",localdir);
	}

	//printf ("directorio final local de zxpand: %s\n",finaldir);


	//Restauramos directorio actual del emulador
	chdir(directorio_actual);
}

//Cambiar directorio
void zxpand_changedir(char *d)
{
	char directorio_final[PATH_MAX];

	debug_printf (VERBOSE_DEBUG,"Changing to directory %s",d);

	zxpand_get_final_directory(d,directorio_final,zxpand_cwd);

}


//Abrir directorio.
void zxpand_COM_DirectoryOpen(void)
{
	char ret = 0x40 + ZXPAND_FR_INVALID_NAME;

	zxpand_deZeddify(zxpand_globaldata);

	zxpand_filinfo_fattrib = 0; // normal dir mode

	if (*zxpand_globaldata == '>')
	{
		// we will change to the directory
		zxpand_changedir((char *)zxpand_globaldata + 1);
		ret=0x40;


		if (ret == 0x40)
		{
			// the CD succeeded, so instruct the 'read directory entry' routine to exit with 'all done'
			zxpand_filinfo_fattrib = 0x42;
		}
	}
	else if (*zxpand_globaldata == '+')
	{

		char directorio_actual[PATH_MAX];
		//obtener directorio final
		zxpand_get_final_directory("",directorio_actual,NULL);


		char directorio_final[PATH_MAX];
		util_get_complete_path(directorio_actual,(char *) (&zxpand_globaldata[1]),directorio_final);
		//sprintf (directorio_final,"%s/%s",directorio_actual,(char *) (&zxpand_globaldata[1]) );

		debug_printf (VERBOSE_DEBUG,"Creating directory %s (total path: %s)",(char *) (&zxpand_globaldata[1]),directorio_final);


		int retorno_mkdir;

		ret = 0x40;

		#ifndef MINGW
		retorno_mkdir=mkdir(directorio_final,S_IRWXU);
		#else
		retorno_mkdir=mkdir(directorio_final);
		#endif

		//Ver si mkdir ha funcionado
		if (retorno_mkdir) ret |=ZXPAND_FR_DENIED;


		if (ret == 0x40)
		{
			// the CD succeeded, so instruct the 'read directory entry' routine to exit with 'all done'
			zxpand_filinfo_fattrib = 0x42;
		}
	}
	else
	{
		// Separate wildcard and path
		//TODO GetWildcard();

		if (zxpand_isValidFN(zxpand_globaldata))
		{
			//ret = 0x40 + f_opendir(&dir, (const char*)&zxpand_globaldata[0]);

			//Si directorio es "", cambiar por "."
			/*if (zxpand_globaldata[0]==0) {
			 *		zxpand_globaldata[0]='.';
			 *		zxpand_globaldata[1]=0;
			}
			*/


			//printf ("opening directory %s\n",zxpand_globaldata);
			char directorio_final[PATH_MAX];
			//obtener directorio final
			zxpand_get_final_directory((char *) zxpand_globaldata,directorio_final,NULL);


			//Guardar directorio final, hace falta al leer cada entrada para saber su tamanyo
			sprintf (zxpand_last_dir_open,"%s",directorio_final);
			zxpand_dfd = opendir(directorio_final);

			if (zxpand_dfd == NULL) {
				debug_printf(VERBOSE_ERR,"Can't open directory %s (full: %s)", zxpand_globaldata,directorio_final);
				ret=0x40 + ZXPAND_FR_NO_PATH;
			}

			else ret=0x40;

		}
	}

	zxpand_latd = ret;
}


//comprobaciones de nombre de archivos en directorio
int zxpand_readdir_no_valido(char *s)
{

	//Si longitud mayor que 12 (8 nombre, punto, 3 extension)
	//if (strlen(s)>12) return 0;

	//printf ("checking is name %s is valid\n",s);


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

void zxpand_COM_DirectoryRead(void)
{

	if (zxpand_dfd==NULL) {
		zxpand_latd=0x3f;
		return;
	}

	do {

		zxpand_dp = readdir(zxpand_dfd);

		if (zxpand_dp == NULL) {
			closedir(zxpand_dfd);
			zxpand_dfd=NULL;
			//no hay mas archivos
			zxpand_latd=0x3f;
			return;
		}


	} while(!zxpand_readdir_no_valido(zxpand_dp->d_name));


	//if (zxpand_isValidFN(zxpand_globaldata)

	//meter flags
	zxpand_filinfo_fattrib=0;

	int longitud_nombre=strlen(zxpand_dp->d_name);

	//obtener nombre con directorio. obtener combinando directorio root, actual y inicio listado
	char nombre_final[PATH_MAX];
	util_get_complete_path(zxpand_last_dir_open,zxpand_dp->d_name,nombre_final);
	//sprintf (nombre_final,"%s/%s",zxpand_last_dir_open,zxpand_dp->d_name);

	//if (get_file_type(zxpand_dp->d_type,zxpand_dp->d_name)==2) {


	if (get_file_type(nombre_final)==2) {



		//meter flags directorio y nombre entre <>
		zxpand_filinfo_fattrib |=16;
		sprintf((char *) &zxpand_globaldata[0],"<%s>",zxpand_dp->d_name);
		longitud_nombre +=2;
	}

	else {
		sprintf((char *) &zxpand_globaldata[0],"%s",zxpand_dp->d_name);
	}


	zxpand_zeddify(&zxpand_globaldata[0]);

	//nombre acabado con 0
	zxpand_globaldata[longitud_nombre]=0;



	int indice=longitud_nombre+1;

	zxpand_globaldata[indice++]=zxpand_filinfo_fattrib;


	long int longitud_total=get_file_size(nombre_final);



	//copia para ir dividiendo entre 256
	long int l=longitud_total;

	zxpand_globaldata[indice++]=l&0xFF;

	l=l>>8;
	zxpand_globaldata[indice++]=l&0xFF;

	l=l>>8;
	zxpand_globaldata[indice++]=l&0xFF;

	l=l>>8;
	zxpand_globaldata[indice++]=l&0xFF;

	zxpand_latd=0x40;


}



//Funcion de lectura/escritura de zxpand
void zxpand_process(int waswrite)
{

	void (*worker)(void) = NULL;


	//Activar footer de ZXpand, pero no si solo leemos el joystick
	if (! (zxpand_porta==7 && ( zxpand_portd==0xA0 || zxpand_portd==0xA1) )) {
		zxpand_footer_zxpand_operating();
	}

	if (!waswrite && dragons_lair_hack.v) {
		//Devolver 1 byte
		//printf ("returning byte from dragons lair hack counter=%d. reg_hl=0x%X reg_pc=0x%X\n",dragons_lair_hack_counter,reg_hl,reg_pc);
       	        //zxpand_COM_FileRead_Dragons();
		zxpand_latd=zxpand_globaldata[dragons_lair_hack_counter];
		dragons_lair_hack_counter++;

		//Como incrementa antes y esto es de 8 bits, si vale 0 quiere decir que ha llegado a 256
		if (dragons_lair_hack_counter==0) dragons_lair_hack.v=0;
		return;
	}



	switch (zxpand_porta) {
		// data channel
		case 0x00:
			// poke 0 here to prepare for reading,
			// poke nonzero here to prepare for writing to globalbuffer.
			if (waswrite) {
				if (zxpand_portd==0) {
					// initialise the output latch with the 1st globalbuffer byte,
					// set the read index to the next byte to be read
					zxpand_latd = zxpand_globaldata[0];

					//En el codigo original se mete a 1, pero seguramente porque esto se ejecuta de manera asincrona
					//Aqui en el emulador debe ser 0
					//zxpand_globalindex = 1;
					//
					zxpand_globalindex = 0;

				}

				else if (zxpand_portd == 42) {
					//Dragons Lair hack
					zxpand_globalindex=0;
					//printf ("dragons lair hack.\n");
					dragons_lair_hack.v=1;
					dragons_lair_hack_counter=0;
					//zxpand_latd = zxpand_globaldata[0];
					zxpand_latd = 0x40;
					//The next 256 "in" opcodes will read from zxpand_globaldata directly
				}

				else {
					// reset buffer index ready for writing
					zxpand_globalindex = 0;
				}

			}


			break;




			// get data from globalbuffer
		case 0x01:
			// must have poked zero at port 0 before starting to peek data from here.
			zxpand_latd=(zxpand_globaldata[zxpand_globalindex]);
			//printf ("Returning value 0x%02X from buffer index %d\n",zxpand_latd,zxpand_globalindex);
			zxpand_globalindex++;
			break;

			// put data into globalbuffer
		case 0x02:
			// must have poked nonzero at port 0 before starting to poke data to here.
			if (waswrite) {
				zxpand_globaldata[zxpand_globalindex] = zxpand_portd;
				zxpand_globalindex++;
			}
			break;


			// directory control port
		case 0x03:
			if (waswrite)
			{
				zxpand_latd=0x80;

				if (zxpand_portd == 0u)
				{
					// reset the directory reader
					//
					worker = zxpand_COM_DirectoryOpen;
				}
				else
				{
					// get next directory entry
					//
					worker = zxpand_COM_DirectoryRead;
				}
			}

			break;


			// file control port
		case 0x04:

			if (waswrite)
			{


				z80_byte pd = zxpand_portd;
				zxpand_latd=0x80;

				if (pd & 2) // ZX80 defaults to 'O' file extension and load address of 0x4000
				{
					zxpand_defaultExtension = 'O';
					zxpand_defaultLoadAddr = 0x4000;
				}
				else
				{
					zxpand_defaultExtension = 'P';
					zxpand_defaultLoadAddr = 0x4009;
				}

				// remove bit 1 - the zx80/zx81 flag / bit 1 set = zx80 file
				pd &= 0xfd;

				if (pd == 0)
				{
					// open the file with name in global data buffer for read access
					// indicate there are no bytes in the sector buffer
					// leaves 32 byte file info in global data: size (W), load address (W), flags (W) + 26 others
					//
					worker = zxpand_COM_FileOpenRead;
				}
				else if (pd == 1)
				{
					// open the file with name in global data buffer for write access
					// creating if necessary. global buffer not damaged in case a delete() is following
					//
					worker = zxpand_COM_FileOpenWrite;
				}
				else if (pd == 0x80)
				{
					// close the open file
					//
					worker = zxpand_COM_FileClose;
				}
				else if (pd == 0xd0)
				{
					// seek information is already in the global buffer
					//
					worker = zxpand_COM_FileSeek;
				}
				else if (pd == 0xe0)
				{
					// delete the file with name in global data buffer
					//
					worker = zxpand_COM_FileRename;
				}
				else if (pd == 0xf0)
				{
					// delete the file with name in global data buffer
					//
					worker = zxpand_COM_FileDelete;
				}
			}

			break;

		// get data from file
		case 0x05:

			if (waswrite)
			{
				zxpand_latd=0x80;

				// read the next N bytes into the data buffer
				// PORTD = 0 means read 256 bytes.
				//
				zxpand_globalAmount = zxpand_portd;
				worker = zxpand_COM_FileRead;
			}

			break;

		// put data to file
		case 0x06:

			// write the next N bytes from the global data buffer
			// PORTD = 0 means write 256 bytes.
			//
			if (waswrite)
			{
				zxpand_latd=0x80;

				zxpand_globalAmount = zxpand_portd;
				worker = zxpand_COM_FileWrite;
			}

			break;


		// interface control
		case 0x7:
			//
			if (waswrite)
			{
				unsigned char pd = zxpand_portd;

				zxpand_latd=0x80;

				if (pd == 0x00)
				{
					// parse a command buffer
					worker = zxpand_COM_ParseBuffer;
				}

				//  0xAn functions return something
				else if (pd == 0xa0)
					//Joystick
				{
					// assemble joystick value/card detect and pop it into latd
					// 0b11111001
					//

					z80_byte temp = zxpand_latd=zxpand_read_joystick();
					zxpand_latd = temp & 0xf9;

				}


				else if (pd == 0xa1)
				{
					// fully decoded j/s - this command may take a few nanos.
					// used by INKEY$
					//
					zxpand_decodeJS();
				}


				else
					if (pd == 0xa2)
					{
						z80_byte temp = ~zxpand_read_joystick();
						z80_byte fb = (temp & 16) << 1;
						z80_byte db = temp >> 4;
						zxpand_latd = fb | db;
					}
				//else
				//if (pd == 0xaa) : see heartbeat, below
				else
					if (pd == 0xab)
					{
					//if (mousex < 0) mousex = 0;
					//if (mousex > 255) mousex = 255;
					//if (mousey < 0) mousey = 0;
					//if (mousey > 191) mousey = 191;
					//zxpand_globaldata[0] = mouseSerialNum;
					//zxpand_globaldata[1] = mousex;
					//zxpand_globaldata[2] = mousey;
					//zxpand_globaldata[3] = (PORTB & 8) | (PORTC & 128);
					//zxpand_globaldata[4] = ~mouseSerialNum;
					//++mouseSerialNum;
					zxpand_latd = 0x40;
				}


				else if (pd == 0xac)
				{
					zxpand_latd = zxpand_configByte;
					debug_printf (VERBOSE_DEBUG,"Returning value zxpand_configByte (%d)",zxpand_configByte);
				}

				else if (pd == 0xae)
				{
					// get EEPROM data; global data contains start,count
					z80_byte i = 0;
	 				z80_byte j = 2;
	 				z80_byte k = zxpand_globaldata[0] + 5;
	 				while (i < zxpand_globaldata[1])
	 				{
		 				//zxpand_globaldata[j] = ReadEEPROM(k);
		 				++j;
		 				++k;
		 				++i;
					}
					zxpand_latd = 0x40;
				}


				else
					if (pd == 0xaf)
					{
					// return whether a card is present
					//
					//LATD = CARDPRESENT();
					zxpand_latd=0x40;
				}



				//  0xBn functions set something and may take time
				else if (pd == 0xb0)
				{
					// temporarily disable overlay until next zeddy reset
					//DISABLE_OVERLAY;
					zxpand_overlay_rom.v=0;
					debug_printf (VERBOSE_DEBUG,"Disabling overlay rom until reset");
				}
				else if (pd == 0xb1)
				{
					// temporarily disable overlay until next zeddy reset
					//??disable or enable??
					//ENABLE_OVERLAY;
					zxpand_overlay_rom.v=1;
					debug_printf (VERBOSE_DEBUG,"Enabling overlay rom");
				}

				else if (pd == 0xb2)
				{
					// M=L
					zxpand_go_low();
				}
				else if (pd == 0xb3)
				{
					// M=H
					zxpand_go_high();
				}


				else
					if (pd == 0xb4)
					{
					//GO_BANK0;
				}
				else
					if (pd == 0xb5)
					{
					//GO_BANK1;
				}
				else
					if (pd == 0xb6)
					{
					//GREENLEDON();
				}
				else
					if (pd == 0xb7)
					{
					//GREENLEDOFF();
				}
				else
					if (pd == 0xb8)
					{
					//REDLEDON();
				}
				else
					if (pd == 0xb9)
					{
					//REDLEDOFF();
				}
				else
					if (pd == 0xba)
					{
					// enable mouse mode

					//mousex = 0;
					//mousey = 0;
					//GOMOUSE;
					}
				else
					if (pd == 0xbb)
					{
					//mousex = zxpand_globaldata[0];
					//mousey = zxpand_globaldata[1];
				}




				else if (pd == 0xbc) {
					// set config byte from global data buffer
					//
					zxpand_configByte = zxpand_globaldata[0];
					debug_printf (VERBOSE_DEBUG,"Setting zxpand_configByte = zxpand_globaldata[0] = %d",zxpand_configByte);
					//TODO: WriteEEPROM(0x04, zxpand_configByte);
					zxpand_latd = 0x40;
				}



				else if (pd == 0xbe)
				{
					z80_byte i = 0;
	 				z80_byte j = 2;
	 				z80_byte k = zxpand_globaldata[0] + 5;
	 				while(i < zxpand_globaldata[1])
	 				{
		 				//WriteEEPROM(k, zxpand_globaldata[j]);
		 				++j;
		 				++k;
		 				++i;
					}
					zxpand_latd = 0x40;
				}



				else if (pd == 0xf0)
				{
					// delay a couple of milliseconds, then disable
					//TODO delay DelayMillis(2);
					//DISABLE_OVERLAY;
					zxpand_overlay_rom.v=0;
					debug_printf (VERBOSE_DEBUG,"Disabling overlay rom until reset, with delay");
				}




				else if (pd == 0xaa)
			 	{
				 	// heartbeat/debug 0
				 	zxpand_latd = 0xf0;
				}



				else if (pd == 0x55)
				{
					// heartbeat/debug 1
					zxpand_latd = 0x0f;
				}



				if ((pd & 0xf0) == 0xb0)
				{
					zxpand_latd = 0x40;
				}



			}
			break;






	}

	if (worker) {
		//printf ("calling worker function %p\n",worker);
		worker();
	}

}


z80_byte zxpand_read(z80_byte puerto_h)
{
	zxpand_porta=(puerto_h>>5)&7;

	//if (dragons_lair_hack.v==0) printf ("zxpand read operation. command: %d (%s)\n",zxpand_porta,zxpand_texto_comandos[zxpand_porta]);
	//else printf ("zxpand read operation. with dragons lair hack enabled\n");

	zxpand_process(0);
	//printf ("zxpand read operation result: 0x%02X\n",zxpand_latd);
	return zxpand_latd;
}
void zxpand_write(z80_byte puerto_h,z80_byte value)
{
	zxpand_porta=(puerto_h>>5)&7;
	zxpand_portd=value;
	//printf ("zxpand write operation. command: %d (%s) value: 0x%02X\n",zxpand_porta,zxpand_texto_comandos[zxpand_porta],zxpand_portd);
	zxpand_process(1);
}
