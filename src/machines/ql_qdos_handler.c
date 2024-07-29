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
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "ql.h"
#include "ql_qdos_handler.h"
#include "m68k.h"
#include "debug.h"
#include "utils.h"
#include "zxvision.h"
#include "menu_items.h"
#include "operaciones.h"
#include "screen.h"
#include "settings.h"
#include "ay38912.h"
#include "ql_i8049.h"
#include "ql_zx8302.h"
#include "timer.h"


#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

/*
Functions to handle QDOS calls
*/

//Parametros de config
char ql_mdv1_root_dir[PATH_MAX]="";
char ql_mdv2_root_dir[PATH_MAX]="";
char ql_flp1_root_dir[PATH_MAX]="";

int ql_microdrive_floppy_emulation=0;



//Parametro que deducimos cuando el archivo ejecutable no tiene cabecera
moto_long ql_task_default_data_size=8192;

int ql_device_mdv1_readonly=0;
int ql_device_mdv2_readonly=0;
int ql_device_flp1_readonly=0;
//Fin Parametros de config

void ql_post_trap_two(void);
void ql_post_trap_three(void);

z80_byte ql_last_trap=0;

int ql_previous_trap_was_4=0;

//0: mdv1
//1: mdv2
//2: flp1
int ql_qdos_last_unit_used=0;


void ql_footer_mdflp_operating(void)
{


    if (ql_qdos_last_unit_used==0) {

        generic_footertext_print_operating("MDV1");

        //Y poner icono en inverso
        if (!zxdesktop_icon_mdv1_inverse) {
                zxdesktop_icon_mdv1_inverse=1;
                menu_draw_ext_desktop();
        }

    }

    if (ql_qdos_last_unit_used==1) {

        generic_footertext_print_operating("MDV2");

        //Y poner icono en inverso
        if (!zxdesktop_icon_mdv2_inverse) {
                zxdesktop_icon_mdv2_inverse=1;
                menu_draw_ext_desktop();
        }

    }

    if (ql_qdos_last_unit_used==2) {

        generic_footertext_print_operating("FLP1");

        //Y poner icono en inverso
        if (!zxdesktop_icon_flp1_inverse) {
                zxdesktop_icon_flp1_inverse=1;
                menu_draw_ext_desktop();
        }

    }
}



struct s_qltraps_fopen qltraps_fopen_files[QLTRAPS_MAX_OPEN_FILES];



void qltraps_init_fopen_files_array(void)
{
	int i;
	for (i=0;i<QLTRAPS_MAX_OPEN_FILES;i++) {
		qltraps_fopen_files[i].open_file.v=0;
	}
}

//Ver si el numero del canal del fichero esta en el rango que gestiona este trap de emulacion
int qltrap_if_file_in_range(unsigned int channel)
{
	unsigned int rangomin=QLTRAPS_START_FILE_NUMBER;
	unsigned int rangomax=QLTRAPS_START_FILE_NUMBER+QLTRAPS_MAX_OPEN_FILES-1;
	if (channel<rangomin || channel>rangomax) return 0;
	return 1;
}


//Retorna indice al array. Si -1, no encontrado/no abierto
int qltraps_find_open_file(unsigned int channel)
{
	if (!qltrap_if_file_in_range(channel)) return -1;

	unsigned int indice=channel-QLTRAPS_START_FILE_NUMBER;
	if (qltraps_fopen_files[indice].open_file.v) return indice;
	else return -1;
}



//Retorna contador a array de estructura de archivo vacio. Retorna -1 si no hay
int qltraps_find_free_fopen(void)
{
	int i;

	for (i=0;i<QLTRAPS_MAX_OPEN_FILES;i++) {
		if (qltraps_fopen_files[i].open_file.v==0) {
			debug_printf (VERBOSE_DEBUG,"QDOS handler: Free handle: %d",i+QLTRAPS_START_FILE_NUMBER);
			return i;
		}
	}

	return -1;
}





//Para almacenar los registros previos a la gestión del trap
unsigned int pre_io_open_a[8];
unsigned int pre_io_open_d[8];

unsigned int pre_io_close_a[8];
unsigned int pre_io_close_d[8];

unsigned int pre_io_delet_a[8];
unsigned int pre_io_delet_d[8];

unsigned int pre_io_sstrg_a[8];
unsigned int pre_io_sstrg_d[8];

unsigned int pre_fs_headr_a[8];
unsigned int pre_fs_headr_d[8];

unsigned int pre_fs_load_a[8];
unsigned int pre_fs_load_d[8];

unsigned int pre_fs_save_a[8];
unsigned int pre_fs_save_d[8];

unsigned int pre_fs_mdinf_a[8];
unsigned int pre_fs_mdinf_d[8];

unsigned int pre_fs_heads_a[8];
unsigned int pre_fs_heads_d[8];

unsigned int pre_io_fline_fstrg_a[8];
unsigned int pre_io_fline_fstrg_d[8];

unsigned int pre_io_edlin_a[8];
unsigned int pre_io_edlin_d[8];


void ql_store_a_registers(unsigned int *destino, int ultimo)
{
  if (ultimo>=0) destino[0]=m68k_get_reg(NULL,M68K_REG_A0);
  if (ultimo>=1) destino[1]=m68k_get_reg(NULL,M68K_REG_A1);
  if (ultimo>=2) destino[2]=m68k_get_reg(NULL,M68K_REG_A2);
  if (ultimo>=3) destino[3]=m68k_get_reg(NULL,M68K_REG_A3);
  if (ultimo>=4) destino[4]=m68k_get_reg(NULL,M68K_REG_A4);
  if (ultimo>=5) destino[5]=m68k_get_reg(NULL,M68K_REG_A5);
  if (ultimo>=6) destino[6]=m68k_get_reg(NULL,M68K_REG_A6);
  if (ultimo>=7) destino[7]=m68k_get_reg(NULL,M68K_REG_A7);
}

void ql_store_d_registers(unsigned int *destino, int ultimo)
{
  if (ultimo>=0) destino[0]=m68k_get_reg(NULL,M68K_REG_D0);
  if (ultimo>=1) destino[1]=m68k_get_reg(NULL,M68K_REG_D1);
  if (ultimo>=2) destino[2]=m68k_get_reg(NULL,M68K_REG_D2);
  if (ultimo>=3) destino[3]=m68k_get_reg(NULL,M68K_REG_D3);
  if (ultimo>=4) destino[4]=m68k_get_reg(NULL,M68K_REG_D4);
  if (ultimo>=5) destino[5]=m68k_get_reg(NULL,M68K_REG_D5);
  if (ultimo>=6) destino[6]=m68k_get_reg(NULL,M68K_REG_D6);
  if (ultimo>=7) destino[7]=m68k_get_reg(NULL,M68K_REG_D7);
}



void ql_restore_a_registers(unsigned int *origen, int ultimo)
{
  if (ultimo>=0) m68k_set_reg(M68K_REG_A0,origen[0]);
  if (ultimo>=1) m68k_set_reg(M68K_REG_A1,origen[1]);
  if (ultimo>=2) m68k_set_reg(M68K_REG_A2,origen[2]);
  if (ultimo>=3) m68k_set_reg(M68K_REG_A3,origen[3]);
  if (ultimo>=4) m68k_set_reg(M68K_REG_A4,origen[4]);
  if (ultimo>=5) m68k_set_reg(M68K_REG_A5,origen[5]);
  if (ultimo>=6) m68k_set_reg(M68K_REG_A6,origen[6]);
  if (ultimo>=7) m68k_set_reg(M68K_REG_A7,origen[7]);
}


void ql_restore_d_registers(unsigned int *origen, int ultimo)
{
  if (ultimo>=0) m68k_set_reg(M68K_REG_D0,origen[0]);
  if (ultimo>=1) m68k_set_reg(M68K_REG_D1,origen[1]);
  if (ultimo>=2) m68k_set_reg(M68K_REG_D2,origen[2]);
  if (ultimo>=3) m68k_set_reg(M68K_REG_D3,origen[3]);
  if (ultimo>=4) m68k_set_reg(M68K_REG_D4,origen[4]);
  if (ultimo>=5) m68k_set_reg(M68K_REG_D5,origen[5]);
  if (ultimo>=6) m68k_set_reg(M68K_REG_D6,origen[6]);
  if (ultimo>=7) m68k_set_reg(M68K_REG_D7,origen[7]);
}

void core_ql_trap_one(void)
{

  //Ver pagina 173. 18.14 Trap Keys

  debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1. D0=%02XH D1=%02XH D2=%02XH D3=%02XH A0=%08XH A1=%08XH A6=%08XH PC=%05XH is : ",
    m68k_get_reg(NULL,M68K_REG_D0),m68k_get_reg(NULL,M68K_REG_D1),m68k_get_reg(NULL,M68K_REG_D2),
    m68k_get_reg(NULL,M68K_REG_D3),m68k_get_reg(NULL,M68K_REG_A0),
    m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_PC));

    //En principio mostramos con VERBOSE_PARANOID traps que no gestionamos,
    //y VERBOSE_DEBUG traps que gestionamos

  switch(m68k_get_reg(NULL,M68K_REG_D0)) {

      case 0x00:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.INF");
      break;

      case 0x01:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.CJOB");
      break;

      case 0x0A:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.ACTIV");
      break;

      case 0x0C:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.ALLOC");
      break;

      case 0x0D:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.LNKFR");
      break;

      case 0x10:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.DMODE");
      break;

      case 0x11:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.IPCOM");
      break;

      case 0x16:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.ALBAS allocate BASIC area");
      break;

      case 0x17:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: MT.REBAS release BASIC area");
      break;


      default:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 1: Unknown call : %02XH",m68k_get_reg(NULL,M68K_REG_D0));
      break;

    }

}



void core_ql_trap_two(void)
{


  //Ver pagina 173. 18.14 Trap Keys

  debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 2. D0=%02XH D1=%02XH A0=%08XH A1=%08XH D3=%08XH PC=%05XH is : ",
    m68k_get_reg(NULL,M68K_REG_D0),m68k_get_reg(NULL,M68K_REG_D1),m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_D3),
    m68k_get_reg(NULL,M68K_REG_PC));

    //En principio mostramos con VERBOSE_PARANOID traps que no gestionamos,
    //y VERBOSE_DEBUG traps que gestionamos

  switch(m68k_get_reg(NULL,M68K_REG_D0)) {

      case 1:
        debug_printf(VERBOSE_DEBUG,"QDOS handler: Trap 2. IO.OPEN");
        //Open a channel. IO.OPEN Guardo todos registros A y D yo internamente de D2,D3,A2,A3 para restaurarlos despues de que se hace el trap de microdrive
        ql_store_a_registers(pre_io_open_a,7);
        ql_store_d_registers(pre_io_open_d,7);
      break;

      case 2:
        debug_printf(VERBOSE_DEBUG,"QDOS handler: Trap 2. IO.CLOSE");
        ql_store_a_registers(pre_io_close_a,7);
        ql_store_d_registers(pre_io_close_d,7);
      break;

      case 4:
        debug_printf(VERBOSE_DEBUG,"QDOS handler: Trap 2. IO.DELET");
        ql_store_a_registers(pre_io_delet_a,7);
        ql_store_d_registers(pre_io_delet_d,7);
      break;


      default:
        debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 2. Unknown call : %02XH",m68k_get_reg(NULL,M68K_REG_D0));
      break;

    }

}

void core_ql_trap_three(void)
{

//Ver pagina 173. 18.14 Trap Keys

  debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 3. D0=%02XH A0=%08XH A1=%08XH A6=%08XH PC=%05XH is : ",
    m68k_get_reg(NULL,M68K_REG_D0),m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),
    m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_PC));


    //En principio mostramos con VERBOSE_PARANOID traps que no gestionamos,
    //y VERBOSE_DEBUG traps que gestionamos

  switch(m68k_get_reg(NULL,M68K_REG_D0)) {
    case 0x2:
      debug_printf(VERBOSE_DEBUG,"QDOS handler: Trap 3: IO.FLINE. fetch a line of bytes terminated by ASCII LF (10)");
      	      //Guardar registros
      ql_store_a_registers(pre_io_fline_fstrg_a,7);
      ql_store_d_registers(pre_io_fline_fstrg_d,7);
    break;

	case 0x3:
      debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: IO.FSTRG. fetch a string of bytes");
      	      //Guardar registros
      ql_store_a_registers(pre_io_fline_fstrg_a,7);
      ql_store_d_registers(pre_io_fline_fstrg_d,7);
	break;

    case 0x4:
      debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: IO.EDLIN");
      	      //Guardar registros
      ql_store_a_registers(pre_io_edlin_a,7);
      ql_store_d_registers(pre_io_edlin_d,7);
    break;

    case 0x5:
      debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 3: IO.SBYTE");
    break;

    case 0x7:
      debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: IO.SSTRG");
      //Guardar registros
      ql_store_a_registers(pre_io_sstrg_a,7);
      ql_store_d_registers(pre_io_sstrg_d,7);
    break;


    case 0xB:
      debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 3: SD.CHENQ");
    break;


    case 0xF:
      debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 3: SD.CURS");
    break;

    case 0x45:
        debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: FS.MDINF");

        //Guardar registros
        ql_store_a_registers(pre_fs_mdinf_a,7);
        ql_store_d_registers(pre_fs_mdinf_d,7);
    break;

    case 0x46:
    	debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: FS.HEADS");

    	      //Guardar registros
      ql_store_a_registers(pre_fs_heads_a,7);
      ql_store_d_registers(pre_fs_heads_d,7);
    break;


    case 0x47:
      debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: FS.HEADR");
      //Guardar registros
      ql_store_a_registers(pre_fs_headr_a,7);
      ql_store_d_registers(pre_fs_headr_d,7);


    break;

    case 0x48:
      debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: FS.LOAD. Length: %d Channel: %d Address: %05XH"
          ,m68k_get_reg(NULL,M68K_REG_D2),m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1)  );
      //D2.L length of file. A0 channellD. A1 base address for load

      //ql_debug_force_breakpoint("En FS.LOAD");

      //Guardar registros
      ql_store_a_registers(pre_fs_load_a,7);
      ql_store_d_registers(pre_fs_load_d,7);


    break;

    case 0x49:
      debug_printf (VERBOSE_DEBUG,"QDOS handler: Trap 3: FS.SAVE. Length: %d Channel: %d Address: %05XH"
          ,m68k_get_reg(NULL,M68K_REG_D2),m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1)  );
      //D2.L length of file. A0 channellD. A1 base address for load

      //ql_debug_force_breakpoint("En FS.LOAD");

      //Guardar registros
      ql_store_a_registers(pre_fs_save_a,7);
      ql_store_d_registers(pre_fs_save_d,7);


    break;

    default:
      debug_printf (VERBOSE_PARANOID,"QDOS handler: Trap 3: Unknown call : %02XH",m68k_get_reg(NULL,M68K_REG_D0));
    break;

  }
}


/*
Llamadas al sistema que han pasado antes por un trap4 y que usan punteros, se les suma A6
En este caso aplica a A1 pero podria aplicar a cualquier otro puntero


https://qlforum.co.uk/viewtopic.php?f=3&t=2230

"
Because of this complicated floating BASIC areas, the system cannot simply hand an absolute pointer to any system
call that wants one (like the buffer address for IO.FLINE). QDOS works around this by introducing TRAP #4,
which is basically a switch - The next system call after a TRAP #4 is instructed to interpret anything that
is a pointer as relative to a6. Let's assume a6 is $30000 and an IO.FLINE trap is issued without a TRAP #4
before, with A1 containing $20000, the system will use $20000 as a target load address. If, however, a TRAP #4
has been issued before an IO.FLINE call, the system will load to (a6, a1), thus at absolute address $50000.
Thus, pointers in traps have to be interpreted according to this switch (which is per-job and re-set after
the next TRAP #1, 2, or 3).
"

So, short answer to your question after all this explanation:

If the very same job that calls the IO.FLINE trap has not issued a TRAP #4 directly before, a1 is the absolute target load address, and will be updated after the call to the end of used buffer area.
If, however, the last TRAP issued by the job in question directly before the IO.FLINE trap was a TRAP #4, the address is
(a6,a1),   and a1 will be updated relatively (i.e. only incremented by the amount of bytes read).

*/
unsigned int ql_get_a1_after_trap_4(void)
{
	if (ql_previous_trap_was_4) {
				return m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);
	}
	else {
		return m68k_get_reg(NULL,M68K_REG_A1);
	}
}



char *ql_qdos_header_magic="]!QDOS File Header";

//Dice si archivo tiene cabecera QDOS valida o no
//Retorna >0 si existe, y su longitud total
//Retorna 0 si no existe
int ql_if_file_has_header(unsigned int indice_canal)
{
    FILE *ptr_file;
    ptr_file=qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix;

    moto_byte buffer[QL_MAX_FILE_HEADER_LENGTH];

    //Leemos
    fread(buffer,1,QL_MAX_FILE_HEADER_LENGTH,ptr_file);

    //Mover puntero de lectura al principio de nuevo
    fseek(ptr_file, 0, SEEK_SET);

    //Ponemos 0 al final del magic de la cabecera y comparamos
    int longitud_magic=strlen(ql_qdos_header_magic);

    if (!memcmp(ql_qdos_header_magic,buffer,longitud_magic)) {
        debug_printf(VERBOSE_DEBUG,"QDOS handler: File has QDOS extended header");

        //Obtener longitud
        int header_length=buffer[19]*2;

        if (header_length==QL_POSSIBLE_HEADER_LENGTH_ONE || header_length==QL_POSSIBLE_HEADER_LENGTH_TWO) {
            debug_printf(VERBOSE_DEBUG,"QDOS handler: And header lenght is valid");
            return header_length;
        }
        else {
            debug_printf(VERBOSE_DEBUG,"QDOS handler: But header is invalid");
        }
    }
    else {
        //Esto no es deteccion de cabecera, esto es para detectar
        //que es un archivo SIN cabecera, pero con la info tipica de un QDOS task
        //Ejemplo del QL chess (chess_exe)


        /*
        00000000  60 00 00 1a 00 00 4a fb  00 12 50 73 69 6f 6e 20  |`.....J...Psion |
        00000010  43 68 65 73 73 20 56 32  2e 30 32 51 41 fa 01 b6  |Chess V2.02QA...|
        00000020  20 08 42 00 22 40 45 fa  00 30 24 89 30 3c 82 a5  | .B."@E..0$.0<..|
        00000030  47 fa 01 a2 49 fa 01 36  97 cc 26 0b 91 c3 93 c3  |G...I..6..&.....|


        */



       if (buffer[6]==0x4a && buffer[7]==0xfb) {
           //Para indentificar. No es un header que busquemos realmente
            debug_printf(VERBOSE_DEBUG,"QDOS handler: Has QDOS task header (and NO file header)");
       }


    }

    return 0;

}

void ql_get_file_header(unsigned int indice_canal,unsigned int destino)
{
    /*

    Sobre cabeceras en archivos de datos generados por emulador:

QL files have a special piece of information associated with them, called the ‘QDOS file header’.
The header stores such information as the file name and whether the file is an executable program.

Q-emuLator for Windows stores part of the header at the beginning of files.
The header is present only when it is useful, ie. only if it contains non-default information.

The header has the following format:
CÓDIGO: SELECCIONAR TODO
OFFSET LENGTH(bytes)       CONTENT
0      18                  “]!QDOS File Header“
18     1                   0 (reserved)
19     1                   total length_of_header, in 16 bit words
20 length_of_header*2-20   QDOS INFO

The first 18 bytes are there to detect whether the header is present (ID string).

The headers Q-emuLator supports can be 30 bytes or 44 bytes long (the value of the corresponding byte at offset 19 is either 15 or 22).
In the first case, there are 10 bytes with the values present in bytes 4 to 13 of the 64 bytes QDOS header.
In the second case the same piece of information is followed by 14 bytes containing a microdrive sector header,
useful for emulating microdrive protection schemes. Additional header information (file length, name, dates) is obtained
directly from the file through the host file system.

    */



  /*
  Sobre cabeceras del QDOS:


  pagina 38 qltm.pdf. 7.0 Directory Device Drivers
  Each file is assumed to have a 64-byte header (the logical beginning of file is set to byte 64, not byte zero).
  This header should be formatted as follows:

  00  long        file length
  04  byte        file access key (not yet implemented - currently always zero)
  05  byte        file type
  06  8 bytes     file type-dependent information
  0E  2+36 bytes  filename
  34 long         reserved for update date (not yet implemented)
  38 long         reserved for reference date (not yet implemented)
  3c long         reserved for backup date (not yet implemented)

  The current file types allowed are: 2, which is a relocatable object file;
  1, which is an executable program;
  255 is a directory;
  and 0 which is anything else

  In the case of file type 1,the first longword of type-dependent information holds
  the default size of the data space for the program.

  Ejecutable quiere decir binario??
  Si es un basic, es tipo 0?

  Segun http://www.dilwyn.me.uk/gen/pcqlxfer/index.html
  Las cabeceras de los archivos de QL se pierden cuando se descomprimen de un zip:

"
  If the zip file contains a QL executable (a program you can EXEC), DO NOT unzip it in Windows.
  It won't work. Not at all. Windows doesn't understand QL file headers and these are lost totally
  and irretrievably by Windows. You can usually tell when you copy it to a QL and find that when you try to
  EXEC a program it stops with a 'bad parameter' or similar error when the QL realises there is no executable
  file header and no dataspace information.
"

Por tanto lo unico que podemos hacer aqui es adivinar el contenido

Mas info:

https://qlforum.co.uk/viewtopic.php?t=113


  */

  debug_printf(VERBOSE_DEBUG,"QDOS handler: Returning header for file on address %05XH",destino);

  //Inicializamos cabecera a 0
  int i;
  //for (i=0;i<64;i++) ql_writebyte(destino+i,0);

  for (i=0;i<14;i++) ql_writebyte(destino+i,0);


  //unsigned int tamanyo=get_file_size(nombre);

	unsigned int tamanyo=qltraps_fopen_files[indice_canal].last_file_buf_stat.st_size;

    //temporal para decirle que pasamos de los 64-6 bytes iniciales del archivo

/*
    En la llamada a EXEC del QDOS, llama a FS.HEADR y luego:

    //Comparar que tipo sea ejecucion
        CMPI.B  #$01,-$09(A6,A1.L)
       BNE     L06990


       MOVEQ   #$01,D0
       MOVEQ   #-$01,D1
    //Leer longitud archivo (primeros 0-3 bytes cabecera)
       MOVE.L  -$0E(A6,A1.L),D2
    //Leer los primeros 4 bytes de file type-dependent information
       MOVE.L  -$08(A6,A1.L),D3
       SUBA.L  A1,A1
       MOVEM.L D2/A0/A3,-(A7)

    //Nota: Estos -$0E(A6,A1.L),D2 se supone que lee la direccion:
    //A6+A1-0eH, y el contenido lo guarda en D2


    //Llamar a MT.CJOB
    //En este:
    D2.L length of code (bytes)
    D3.L length of data space AO
    A1 start address or 0

       TRAP    #$01

    //Ese trap no inicia la tarea, sino que la asigna:

    This trap allocates space in the transient program area,
    and sets up a job entry in the scheduler tables. This does not invoke the job and the only initialisation
    is that two words of 0 are put on the stack. The program itself would normally be loaded, by another job,
    into the space allocated, after this system call. The stack pointer saved in the job control area points
    initially to two zero words on the stack (at the highest addresses in the job's data area); if channels are
    to be opened for the job, or a command string is to be passed to the job, then this can be done before the job is activated.
    If D1 is negative, the new job is independent, otherwise it is owned by the calling job.
*/

    //Guardar tamanyo big endian
    //TODO: deberia restar el tamano de la cabecera: de la que tiene firma Magic o la que no tiene?
    ql_writebyte(destino+0,(tamanyo>>24)&255);
    ql_writebyte(destino+1,(tamanyo>>16)&255);
    ql_writebyte(destino+2,(tamanyo>>8)&255);
    ql_writebyte(destino+3,tamanyo&255);

    //printf("poniendo tamanyo %d en offset %X\n",tamanyo,destino);

  //Y nos inventamos los primers 4 bytes del file type-dependent information
  //Que es el tamaño de los datos
  //Ponemos arbitrariamente 8192
  //Ya se sobreescribira si hace falta
    //06 tenemos : 00 00 20 00 = 8192


    //printf("assuming default data size: %d\n",ql_task_default_data_size);

    ql_writebyte(destino+6,(ql_task_default_data_size>>24)&255);
    ql_writebyte(destino+7,(ql_task_default_data_size>>16)&255);
    ql_writebyte(destino+8,(ql_task_default_data_size>>8)&255);
    ql_writebyte(destino+9,ql_task_default_data_size&255);

    //Tipo. Asumimos siempre ejecutable 1
    ql_writebyte(destino+5,1); //ejecutable 1




  //Ver si tiene cabecera el archivo
  if (qltraps_fopen_files[indice_canal].has_header_on_read) {

      //Leemos esa cabecera, que ya tenemos en la estructura de archivos abiertos
        debug_printf(VERBOSE_DEBUG,"QDOS handler: Returning header (QDOS magic) with some of values from file header");


        //Valores usados de esa cabecera,desde el offset 20:
        //there are 10 bytes with the values present in bytes 4 to 13 of the 64 bytes QDOS header.
        //  04  byte        file access key (not yet implemented - currently always zero)
        //05  byte        file type
        //06  8 bytes     file type-dependent information

        //O sea saltamos los 4 primeros, que ya hemos puesto un poco antes
        int i;
        for (i=0;i<10;i++) {
            moto_byte byte_leido=qltraps_fopen_files[indice_canal].file_header[20+i];
            unsigned int destino_cabecera=destino+4+i;
            //printf("Setting offset %02d value %02XH\n",i,byte_leido);


            ql_writebyte(destino_cabecera,byte_leido);
        }


        //printf("Nombre: %s\n",qltraps_fopen_files[indice_canal].ql_file_name);

        //int longitud=strlen(qltraps_fopen_files[indice_canal].ql_file_name);

        //Nombre. Esto solo se deberia escribir si longitud pedida > 14
        /*
        ql_writebyte(destino+0xe,0); //longitud nombre en big endian
        ql_writebyte(destino+0xf,longitud); //longitud nombre en big endian


        for (i=0;i<longitud;i++) {
            ql_writebyte(destino+0x10+i,qltraps_fopen_files[indice_canal].ql_file_name[i]);
        }
        */

  }



    else {
        debug_printf(VERBOSE_DEBUG,"QDOS handler: Returning header but file has no header. Guessing some values");
    }

    //printf("assuming default data size: %d\n",ql_task_default_data_size);

    unsigned int leido_data_size=(ql_readbyte(destino+6)<<24)|(ql_readbyte(destino+7)<<16)|(ql_readbyte(destino+8)<<8)|(ql_readbyte(destino+9));
    debug_printf(VERBOSE_DEBUG,"QDOS handler: Final data size: %d",leido_data_size);


}


void ql_put_file_header(unsigned int indice_canal,unsigned int destino)
{


  debug_printf(VERBOSE_DEBUG,"QDOS handler: Writing header for file on address %05XH",destino);

  //Con mi cabecera pequeña de 30 es suficiente
  //QL_POSSIBLE_HEADER_LENGTH_ONE
  moto_byte buffer_header[QL_POSSIBLE_HEADER_LENGTH_ONE];

  //Inicializamos cabecera a 0
  int i;
  for (i=0;i<QL_POSSIBLE_HEADER_LENGTH_ONE;i++) buffer_header[i]=0;

  //Metemos magic
  int longitud_magic=strlen(ql_qdos_header_magic);

  memcpy(buffer_header,ql_qdos_header_magic,longitud_magic);

/*
OFFSET LENGTH(bytes)       CONTENT
0      18                  “]!QDOS File Header“
18     1                   0 (reserved)
19     1                   total length_of_header, in 16 bit words
20 length_of_header*2-20   QDOS INFO

The first 18 bytes are there to detect whether the header is present (ID string).

The headers Q-emuLator supports can be 30 bytes or 44 bytes long (the value of the corresponding byte at offset 19 is either 15 or 22).
In the first case, there are 10 bytes with the values present in bytes 4 to 13 of the 64 bytes QDOS header.


  Sobre cabeceras del QDOS:


  pagina 38 qltm.pdf. 7.0 Directory Device Drivers
  Each file is assumed to have a 64-byte header (the logical beginning of file is set to byte 64, not byte zero).
  This header should be formatted as follows:

  00  long        file length
  04  byte        file access key (not yet implemented - currently always zero)
  05  byte        file type
  06  8 bytes     file type-dependent information
  */


    //Indicar longitud
    buffer_header[19]=QL_POSSIBLE_HEADER_LENGTH_ONE/2;

    //Y luego ya la info que nos llega





    //printf("Writing qdos header\n");

    /*
    //mostrar algunos bytes
     unsigned int reg_a1=ql_get_a1_after_trap_4();
    for (i=0;i<10;i++) {
        unsigned int offset=reg_a1+4+i;
        moto_byte byte_leido;
        byte_leido=peek_byte_z80_moto(offset);
        buffer_header[20+i]=byte_leido;

        printf ("%02XH ",byte_leido);
    }

    printf("\n");
    */


    //Y escribir dicha cabecera
    FILE *ptr_file;
    ptr_file=qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix;
    fwrite(buffer_header,1,QL_POSSIBLE_HEADER_LENGTH_ONE,ptr_file);





}




//Dado una ruta de QL tipo mdv1_programa , retorna mdv1 y programa por separados
void ql_split_path_device_name(char *ql_path, char *ql_device, char *ql_file,int replace_underscore_dot,int replace_underscore_dot_only_one)
{
  //Buscar hasta _
  int i;
  for (i=0;ql_path[i] && ql_path[i]!='_';i++);
  if (ql_path[i]==0) {
    //No encontrado
    ql_device[0]=0;
    ql_file[0]=0;
  }

  //Copiar desde inicio hasta aqui en ql_device
  int iorig=i;
  //Vamos del final hacia atras
  ql_device[i]=0;
  i--;
  char c;
  for (;i>=0;i--) {
    c=letra_minuscula(ql_path[i]);
    ql_device[i]=c;
  }

  //Restauramos indice y vamos de ahi+1 al final. Si nombre de archivo contiene un _, sustituir por .
  //Y pasar a minusculas todo
  i=iorig+1;
  int destino=0;

	//Lo paso a minusculas a destino
  for (;ql_path[i];i++,destino++) {
    c=letra_minuscula(ql_path[i]);

    ql_file[destino]=c;
  }

  ql_file[destino]=0;



	//Y en destino, cambio las "_", empezando desde el final, y solo quitando una "_" dependiendo de replace_underscore_dot_only_one
	if (replace_underscore_dot) {

		i=strlen(ql_file)-1;
		int salir=0;

		for (;i>=0 && !salir;i--) {
			c=ql_file[i];
			if (c=='_') {
				c='.';
				ql_file[i]=c;
				if (replace_underscore_dot_only_one) salir=1;
			}
		}
	}


    debug_printf(VERBOSE_DEBUG,"QDOS handler: Source path: %s Device: %s File: %s",ql_path,ql_device,ql_file);
}

int ql_device_mdv1_enabled=0;
int ql_device_mdv2_enabled=0;
int ql_device_flp1_enabled=0;

//Dado un device y un nombre de archivo, retorna ruta a archivo en filesystem final
//Retorna 1 si error
//Tambien asigna ultima unidad usada para visualizarlo en el icono y el footer
int ql_return_full_path(char *device, char *file, char *fullpath)
{
    char *sourcepath;

    if (!strcasecmp(device,"mdv1")) {
        ql_qdos_last_unit_used=0;
        if (!ql_device_mdv1_enabled) return 1;
        sourcepath=ql_mdv1_root_dir;

    }
    else if (!strcasecmp(device,"mdv2")) {
        ql_qdos_last_unit_used=1;
        if (!ql_device_mdv2_enabled) return 1;
        sourcepath=ql_mdv2_root_dir;

    }
    else if (!strcasecmp(device,"flp1")) {
        ql_qdos_last_unit_used=2;
        if (!ql_device_flp1_enabled) return 1;
        sourcepath=ql_flp1_root_dir;
    }
    else return 1;

    if (sourcepath[0])  sprintf(fullpath,"%s/%s",sourcepath,file);
    else sprintf(fullpath,"%s",file); //Ruta definida como vacia


    return 0;
}

void ql_copy_if_flp1_follow_mdv1(void)
{
    //Si se copia misma ruta de mdv1 a flp1
    if (ql_flp1_follow_mdv1.v) strcpy(ql_flp1_root_dir,ql_mdv1_root_dir);
}

void ql_insert_mdv_flp(enum ql_qdos_unidades unidad,char *dir_to_mount)
{

    //char *string_root_dir;

    switch (unidad) {

        case QL_QDOS_UNIT_MDV2:
            sprintf (ql_mdv2_root_dir,"%s",dir_to_mount);
            ql_device_mdv2_enabled=1;
        break;

        case QL_QDOS_UNIT_FLP1:
            sprintf (ql_flp1_root_dir,"%s",dir_to_mount);
            ql_device_flp1_enabled=1;
        break;

        case QL_QDOS_UNIT_MDV1:
        default:
            sprintf (ql_mdv1_root_dir,"%s",dir_to_mount);
            ql_device_mdv1_enabled=1;

            ql_copy_if_flp1_follow_mdv1();
        break;
    }


    if (noautoload.v==0) {
        debug_printf (VERBOSE_INFO,"Restarting autoload");

        ql_initial_autoload=1;
        ql_initial_autoload_counter=0;
        //initial_tap_load.v=1;
        //initial_tap_sequence=0;

        debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
        reset_cpu();

        //Activamos top speed si conviene
        /*if (fast_autoload.v && !MACHINE_IS_MSX && !MACHINE_IS_ACE) {
            debug_printf (VERBOSE_INFO,"Set top speed from TAP open");
            top_speed_timer.v=1;
        }*/

    }

    else {
        //initial_tap_load.v=0;
    }



}


//Dice si la ruta que se le ha pasado corresponde a un mdv1_, o mdv2_, o flp1_
int ql_si_ruta_parametro(char *texto,char *ruta)
{
  char *encontrado;

  encontrado=util_strcasestr(texto, ruta);
  if (encontrado) return 1;

  return 0;
}

//Dice si la ruta que se le ha pasado corresponde a un mdv1_, o mdv2_, o flp1_
int ql_si_ruta_mdv_flp(char *texto)
{
  char *encontrado;

  char *buscar_mdv1="mdv1_";
  encontrado=util_strcasestr(texto, buscar_mdv1);
  if (encontrado) return 1;

  char *buscar_mdv2="mdv2_";
  encontrado=util_strcasestr(texto, buscar_mdv2);
  if (encontrado) return 1;

  char *buscar_flp1="flp1_";
  encontrado=util_strcasestr(texto, buscar_flp1);
  if (encontrado) return 1;

  return 0;
}

//int temp_conta_dir=0;

//Leer archivo linea a linea. Retorna bytes leidos, y valor de retorno.
//Si hay eof, se debe retornar solo el eof y sin bytes leidos


unsigned int ql_read_io_fline(unsigned int canal,unsigned int puntero_destino,unsigned int *valor_retorno,unsigned int longitud_buffer)
{

	//printf("longitud buffer: %d\n",longitud_buffer);

	FILE *ptr_archivo;

	//Si habia final de fichero, retornar solo eso
	if (qltraps_fopen_files[canal].next_eof_ptr_io_fline) {
		debug_printf (VERBOSE_PARANOID,"QDOS handler: Returning eof");
		qltraps_fopen_files[canal].next_eof_ptr_io_fline=0;
		*valor_retorno=-10;
		return 0;
	}

	//Por defecto
	*valor_retorno=0;




	ptr_archivo=qltraps_fopen_files[canal].qltraps_last_open_file_handler_unix;

	unsigned int total_leidos=0;
	//Ir leyendo hasta codigo 10 o final de fichero
	int salir=0;

	while (!salir) {
		int bytes_leidos=fgetc(ptr_archivo);
		//Si negativo, asumimos final de fichero
		if (bytes_leidos<0) {
			//printf("\nEOF\n");
			qltraps_fopen_files[canal].next_eof_ptr_io_fline=1;
			salir=1;
		}

		if (!salir) {

			if (total_leidos>=longitud_buffer) {
				//printf("\nOverflow\n");
				*valor_retorno=QDOS_ERROR_CODE_BO;

				//ese byte esta fuera de buffer y no se retornara. "Rebobinar" puntero lectura 1 byte
				fseek(ptr_archivo,-1,SEEK_CUR);

				return total_leidos;
			}


			//printf ("Escribiendo byte %d (%c) direccion %XH\n",bytes_leidos,(bytes_leidos>32 && bytes_leidos<128 ? bytes_leidos : '.'),puntero_destino);

			/*
			if (bytes_leidos>=32 && bytes_leidos<=126) {
				printf("%c",(bytes_leidos>=32 && bytes_leidos<=126 ? bytes_leidos : '.'));
			}
			else {
				printf("-%02XH-",bytes_leidos);
			}
			*/



			ql_writebyte(puntero_destino++,bytes_leidos);
			total_leidos++;

		}

		//Si salto de linea y funcion IO.FLINE, salir
		if (m68k_get_reg(NULL,M68K_REG_D0)==0x2 && bytes_leidos==10) salir=1;

	}

	//printf("\nEND ql_read_io_fline\n");

	return total_leidos;


}


//Leer archivo linea a linea. Retorna bytes leidos, y valor de retorno.
//Si hay eof, se debe retornar solo el eof y sin bytes leidos


unsigned int ql_read_io_edlin(unsigned int canal,unsigned int puntero_destino,unsigned int *valor_retorno,unsigned int longitud_buffer)
{

	//printf("longitud buffer: %d\n",longitud_buffer);

	FILE *ptr_archivo;

	//Si habia final de fichero, retornar solo eso
	if (qltraps_fopen_files[canal].next_eof_ptr_io_fline) {
		debug_printf (VERBOSE_PARANOID,"QDOS handler: Returning eof");
		qltraps_fopen_files[canal].next_eof_ptr_io_fline=0;
		*valor_retorno=-10;
		return 0;
	}

	//Por defecto
	*valor_retorno=0;



	ptr_archivo=qltraps_fopen_files[canal].qltraps_last_open_file_handler_unix;

	unsigned int total_leidos=0;
	//Ir leyendo hasta codigo 10 o final de fichero
	int salir=0;

	while (!salir) {
		int bytes_leidos=fgetc(ptr_archivo);
		//Si negativo, asumimos final de fichero
		if (bytes_leidos<0) {
			//printf("\nEOF\n");
			qltraps_fopen_files[canal].next_eof_ptr_io_fline=1;
			salir=1;
		}

		if (!salir) {

			if (total_leidos>=longitud_buffer) {
				//printf("\nOverflow\n");
				*valor_retorno=QDOS_ERROR_CODE_BO;

				//ese byte esta fuera de buffer y no se retornara. "Rebobinar" puntero lectura 1 byte
				fseek(ptr_archivo,-1,SEEK_CUR);

				return total_leidos;
			}


			ql_writebyte(puntero_destino++,bytes_leidos);
			total_leidos++;

		}

		//Si salto de linea, salir
		if (bytes_leidos==10) salir=1;

	}

	//printf("\nEND ql_read_io_fline\n");

	return total_leidos;


}


void ql_load_binary_file(FILE *ptr_file,unsigned int valor_leido_direccion, unsigned int valor_leido_longitud)
{

    int leidos=1;

    z80_byte byte_leido;
    while (valor_leido_longitud>0 && leidos>0) {
            leidos=fread(&byte_leido,1,1,ptr_file);
            if (leidos>0) {
                    //poke_byte_no_time(valor_leido_direccion,byte_leido);
                    poke_byte_z80_moto(valor_leido_direccion,byte_leido);
                    valor_leido_direccion++;
                    valor_leido_longitud--;
            }
    }
}

void ql_dos_set_error(int number)
{
    m68k_set_reg(M68K_REG_D0,number);
}

void ql_qdos_set_return_no_error(void)
{


    //Como decir no error
    /*
    pg 11 qltm.pdf
    When the TRAP operation is complete, control is returned to the program at the location following the TRAP instruction,
    with an error key in all 32 bits of D0. This key is set to zero if the operation has been completed successfully,
    and is set to a negative number for any of the system-defined errors (see section 17.1 for a list of the meanings
    of the possible error codes). The key may also be set to a positive number, in which case that number is a pointer
    to an error string, relative to address $8000. The string is in the usual Qdos form of a word giving the length of
    the string, followed by the characters.
    */


    //No error.
    ql_dos_set_error(0);
}

void ql_qdos_return_from_trap(void)
{
    //Volver de ese trap
    //en direccion 0x5E hay un "RTE": Return from exception
    //Serviria cualquier otra dirección con RTE
    m68k_set_reg(M68K_REG_PC,0x5e);

    //Ajustar stack para volver. Saltamos 3 long words
    unsigned int reg_a7=m68k_get_reg(NULL,M68K_REG_A7);
    reg_a7 +=12;
    m68k_set_reg(M68K_REG_A7,reg_a7);
}

//int temp_conta=0;


//comprobaciones de nombre de archivos en directorio
int qltraps_dir_aux_readdir_no_valido(char *s)
{


	debug_printf(VERBOSE_PARANOID,"QDOS handler: checking if name %s is valid",s);


    //Si nombre . o .. o zesarux_last_dir.txt
    if (!strcmp(s,".") || !strcmp(s,"..") || !strcmp(s,MENU_LAST_DIR_FILE_NAME)) return 0;

    //TODO ocultar subdirectorios

	return 1;

}


//Retorna 0 si no hay mas archivos
//Retorna 1 si ok
int qltraps_dir_aux_readdir(int indice_canal)
{
	do {

        qltraps_fopen_files[indice_canal].qltraps_handler_dp = readdir(qltraps_fopen_files[indice_canal].qltraps_handler_dfd);

        if (qltraps_fopen_files[indice_canal].qltraps_handler_dp == NULL) {


            debug_printf (VERBOSE_DEBUG,"QDOS handler: No more files on readdir");


            return 0;
        }


	} while(!qltraps_dir_aux_readdir_no_valido(qltraps_fopen_files[indice_canal].qltraps_handler_dp->d_name));

	return 1;
}


void qltraps_dir(int indice_canal)
{

    //Si no esta abierto el directorio

    if (qltraps_fopen_files[indice_canal].qltraps_handler_dfd==NULL) {
        debug_printf (VERBOSE_DEBUG,"QDOS handler: Error from qltraps_dir. Directory not open");
        ql_dos_set_error(QDOS_ERROR_CODE_NC);
        ql_qdos_return_from_trap();

        return;
    }

	//Si no es un directorio, error
	if (qltraps_fopen_files[indice_canal].es_dispositivo==0) {
		debug_printf (VERBOSE_DEBUG,"QDOS handler: Error from qltraps_dir. Handler %d is not a device",indice_canal);
        ql_dos_set_error(QDOS_ERROR_CODE_NC);
        ql_qdos_return_from_trap();
		return;
	}


   	if (!qltraps_dir_aux_readdir(indice_canal)) {
		//no hay mas archivos
		debug_printf(VERBOSE_DEBUG,"QDOS handler: Returning no more files to readdir");
        ql_dos_set_error(QDOS_ERROR_CODE_EF);
        m68k_set_reg(M68K_REG_D1,0);  //0 byte leido
        ql_qdos_return_from_trap();

        return;


	}

    /*
    Cuando hace dir mdv1_

    1) IO.OPEN dispositivo "mdv1_"
    2) FS.MDINF para obtener nombre microdrive
    3) IO.FSTRG. D2=64 (Channel ID=32 Base of buffer A1=00000100H A3=00000668H A6=00028000H). Hasta que se retorne EOF
    */

    //Lo que ocupa una entrada de directorio
    int longitud_entrada=64;



    unsigned int puntero=ql_get_a1_after_trap_4();

    //printf("Escribiendo entrada directorio en %X, length buffer: %d\n",puntero,m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF);

    //de momento poner a 0
    int longitud_buffer=m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF;
    int i;
    for (i=0;i<longitud_buffer;i++) ql_writebyte(puntero+i,0);

    //Asumimos que pide:
    /*
        00  long        file length
        04  byte        file access key (not yet implemented - currently always zero)
        05  byte        file type
        06  8 bytes     file type-dependent information
        0E  2+36 bytes  filename
        34 long         reserved for update date (not yet implemented)
        38 long         reserved for reference date (not yet implemented)
        3c long         reserved for backup date (not yet implemented)
    */

    //file length. me lo invento
    ql_writebyte(puntero,0);
    ql_writebyte(puntero+1,0);
    ql_writebyte(puntero+2,16);
    ql_writebyte(puntero+3,0);

    //file type normal
    ql_writebyte(puntero+5,0);

    puntero +=0x0E;

    int longitud_nombre=strlen(qltraps_fopen_files[indice_canal].qltraps_handler_dp->d_name);

    debug_printf(VERBOSE_DEBUG,"QDOS handler: Writing directory entry name: %s",qltraps_fopen_files[indice_canal].qltraps_handler_dp->d_name);

    //int i;
    for (i=0;i<36 && i<longitud_nombre;i++) {
        ql_writebyte(puntero+2+i,qltraps_fopen_files[indice_canal].qltraps_handler_dp->d_name[i]);

    }

    //16 bytes MSB. Pero siempre menor que 255
    ql_writebyte(puntero++,0);
    ql_writebyte(puntero++,longitud_nombre);


    unsigned int registro_a1=m68k_get_reg(NULL,M68K_REG_A1);
    registro_a1 +=longitud_entrada;



    m68k_set_reg(M68K_REG_A1,registro_a1);




    ql_qdos_set_return_no_error();
    m68k_set_reg(M68K_REG_D1,longitud_entrada);

    ql_qdos_return_from_trap();
}

void handle_trap_io_fline_fstrg(void)
{

		//printf("last trap = %d previous was trap4: %d\n",ql_last_trap,ql_previous_trap_was_4);

		if (m68k_get_reg(NULL,M68K_REG_D0)==0x2) {
        debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.FLINE. Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH D2=%d",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),m68k_get_reg(NULL,M68K_REG_A6)
				,m68k_get_reg(NULL,M68K_REG_D2) );
		}
		else {
			debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.FSTRG. Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH D2=%d",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),m68k_get_reg(NULL,M68K_REG_A6)
				,m68k_get_reg(NULL,M68K_REG_D2) );
		}

        //printf("PC=%X D0 %X\n",get_pc_register(),m68k_get_reg(NULL,M68K_REG_D0));


        //Si canal es de los mios
        //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        int indice_canal=qltraps_find_open_file(pre_io_fline_fstrg_a[0] & 0xFFFF);
        if (indice_canal>=0) {

        	//Indicar actividad en md flp
        	ql_footer_mdflp_operating();

        	debug_printf (VERBOSE_PARANOID,"QDOS handler: Returning IO.FLINE/IO.FSTRG from our microdrive channel without error");

          	ql_restore_d_registers(pre_io_fline_fstrg_d,7);
          	ql_restore_a_registers(pre_io_fline_fstrg_a,6);


        	//Si es un dispositivo entero, estamos haciendo un dir
        	if (qltraps_fopen_files[indice_canal].es_dispositivo) {
                debug_printf(VERBOSE_DEBUG,"QDOS handler: Reading directory %s",qltraps_fopen_files[indice_canal].ql_file_name);


                if (!ql_microdrive_floppy_emulation) {
                    debug_printf(VERBOSE_DEBUG,"QDOS handler: Microdrive emulation not enabled");
                    //Retornar Not found (NF)
                    //m68k_set_reg(M68K_REG_D0,-7);

                    //Volver sin mas
                    return;
                }

                qltraps_dir(indice_canal);

      			return;
        	}



          	/*
          	D0=$2 IO.FLINE fetch a line of characters terminated by ASCII <LF> ($A)
			D0=$3 IO.FSTRG fetch a string of bytes
          	*/

          	/*
          	Entrada:
          	D2.W length of buffer
          	D3.W timeout
          	A0 channel ID
          	A1 base of buffer

          	Salida:
          	D1.W nr. of bytes fetched
          	A1 updated ptr to buffer

          	Errores:
          	NC not complete
          	NO channel not open
          	EF end of file
          	BO buffer overflow (fetch line only)

          	*/

        	//Dudas!! Donde se guarda los datos leidos? En A1+A6??
        	//Registro de salida A1 a donde debe apuntar??
        	//unsigned int puntero_destino=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);

        	//O a A1 a secas
        	//depende de si se ha llamado trap4 o no



          	unsigned int puntero_destino;

			/*
			If the very same job that calls the IO.FLINE trap has not issued a TRAP #4 directly before,
			a1 is the absolute target load address, and will be updated after the call to the end of used buffer area.

			If, however, the last TRAP issued by the job in question directly before the IO.FLINE trap was a TRAP #4,
			the address is (a6,a1), and a1 will be updated relatively (i.e. only incremented by the amount of bytes read).

			*/

            puntero_destino=ql_get_a1_after_trap_4();


          	debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.FLINE - Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH dest pointer: %08XH max length: %d",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),
        		m68k_get_reg(NULL,M68K_REG_A6),puntero_destino, m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF);



          	unsigned int valor_retorno;

          	unsigned int leidos=ql_read_io_fline(indice_canal,puntero_destino,&valor_retorno,m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF);



          	m68k_set_reg(M68K_REG_D0,valor_retorno);

          	unsigned int registro_a1=m68k_get_reg(NULL,M68K_REG_A1);
          	registro_a1 +=leidos;
          	m68k_set_reg(M68K_REG_A1,registro_a1);

          	//printf ("Leidos: %d\n",leidos);
          	m68k_set_reg(M68K_REG_D1,leidos);



          //Volver de ese trap
          ql_qdos_return_from_trap();



        }
}



void handle_trap_io_edlin(void)
{

		//printf("last trap = %d previous was trap4: %d\n",ql_last_trap,ql_previous_trap_was_4);

		if (m68k_get_reg(NULL,M68K_REG_D0)==0x2) {
        debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.EDLIN. Channel ID=%d End of line: A1=%08XH A3=%08XH A6=%08XH D2=%d",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),m68k_get_reg(NULL,M68K_REG_A6)
				,m68k_get_reg(NULL,M68K_REG_D2) );
		}


        //Si canal es de los mios
        //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        int indice_canal=qltraps_find_open_file(pre_io_edlin_a[0] & 0xFFFF);
        if (indice_canal>=0) {

        	 //Indicar actividad en md flp
        	ql_footer_mdflp_operating();

        	debug_printf (VERBOSE_PARANOID,"QDOS handler: Returning IO.EDLIN from our microdrive channel without error. EXPERIMENTAL!!!");


        	//Si es un dispositivo entero
        	if (qltraps_fopen_files[indice_canal].es_dispositivo) {

        		m68k_set_reg(M68K_REG_D0,QDOS_ERROR_CODE_EF);
          		debug_printf (VERBOSE_DEBUG,"QDOS handler: IO.FLINE - returning EOF");
          		m68k_set_reg(M68K_REG_D1,0);  //0 byte leido
      			return;
        	}

          	ql_restore_d_registers(pre_io_edlin_d,7);
          	ql_restore_a_registers(pre_io_edlin_a,6);

          	/*
          	Entrada:
            D1 cursor/line length
            D2.W length of buffer
            D3.W timeout
            A0 channel ID
            A1 pointer to end of line

          	Salida:
          	D1 cursor/line length
            D2 preserved
            D3 preserved

            A0 preserved
            A1 pointer to end of line
            A2 preserved
            A3 preserved


          	Errores:
          	NC not complete
          	NO channel not open
          	BO buffer overflow

            NOTA: !! A1 dice que va el puntero a final de linea, PERO lo trato igual que IO.FLINE (base buffer) y parece funcionar
            Para probar esto, por ejemplo:
            OPEN_IN#5;mdv1_Mago_dat:FOR n=1TO 28:INPUT#5;a$
            Y tiene que leer cada linea separada por codigo 10 (LF)

            NOTA2: Supuestamente la documentacion dice:
            Edit a line of characters (console driver only)
            Pero como se ve, se usa en INPUT#...., no solo en console driver (a no ser que console driver sea tambien un INPUT#)

          	*/

        	//Dudas!! Donde se guarda los datos leidos? En A1+A6??
        	//Registro de salida A1 a donde debe apuntar??
        	//unsigned int puntero_destino=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);

        	//O a A1 a secas
        	//depende de si se ha llamado trap4 o no



          	unsigned int puntero_destino;

			/*
			If the very same job that calls the IO.FLINE trap has not issued a TRAP #4 directly before,
			a1 is the absolute target load address, and will be updated after the call to the end of used buffer area.

			If, however, the last TRAP issued by the job in question directly before the IO.FLINE trap was a TRAP #4,
			the address is (a6,a1), and a1 will be updated relatively (i.e. only incremented by the amount of bytes read).

			*/

            puntero_destino=ql_get_a1_after_trap_4();


          	debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.EDLIN - Channel ID=%d End of line: A1=%08XH A3=%08XH A6=%08XH dest pointer: %08XH max length: %d",
        		m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),
        		m68k_get_reg(NULL,M68K_REG_A6),puntero_destino, m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF);




          	unsigned int valor_retorno;

          	unsigned int leidos=ql_read_io_edlin(indice_canal,puntero_destino,&valor_retorno,m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF);



          	m68k_set_reg(M68K_REG_D0,valor_retorno);

          	unsigned int registro_a1=m68k_get_reg(NULL,M68K_REG_A1);
          	registro_a1 +=leidos;
          	m68k_set_reg(M68K_REG_A1,registro_a1);

          	//printf ("Leidos: %d\n",leidos);
          	m68k_set_reg(M68K_REG_D1,leidos);



          //Volver de ese trap
          ql_qdos_return_from_trap();



        }
}

void handle_trap_fs_headr(void)
{
    //debug_printf (VERBOSE_PARANOID,"FS.HEADR. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

    debug_printf (VERBOSE_PARANOID,"QDOS handler: FS.HEADR. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

    //Si canal es el mio ficticio 100
    //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
    int indice_canal=qltraps_find_open_file(pre_fs_headr_a[0] & 0xFFFF);
    if (indice_canal>=0 ) {

        //Indicar actividad en md flp
        ql_footer_mdflp_operating();

        ql_restore_d_registers(pre_fs_headr_d,7);
        ql_restore_a_registers(pre_fs_headr_a,6);

        //printf("Length buffer: %d\n",m68k_get_reg(NULL,M68K_REG_D2)&0xFFFF);
        //printf("Base buffer: %X  (A1=%4XH)\n",ql_get_a1_after_trap_4(),m68k_get_reg(NULL,M68K_REG_A1));

        //TODO: asumimos siempre meter 14 bytes de cabecera, aunque se hayan pedido menos


        ql_get_file_header(indice_canal,ql_get_a1_after_trap_4() );


        //Volver de ese trap
        ql_qdos_return_from_trap();

        //No error.
        ql_qdos_set_return_no_error();

        int longitud=14;

        //D1.W length of header read. A1 top of read buffer
        m68k_set_reg(M68K_REG_D1,longitud);

        //Decimos que A1 es A1 top of read buffer

        //A1 retorna tal cual como entraba (sin sumar posible A6) y sumar la longitud 14
        unsigned int reg_a1;
        reg_a1=m68k_get_reg(NULL,M68K_REG_A1);
        reg_a1 +=longitud;
        //printf ("Retornando A1 con %X\n",reg_a1);
        m68k_set_reg(M68K_REG_A1,reg_a1);

    }
}


void handle_trap_fs_mdinf(void)
{
    debug_printf (VERBOSE_PARANOID,"QDOS handler: FS.MDINF. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

    //printf("last trap = %d previous was trap4: %d\n",ql_last_trap,ql_previous_trap_was_4);

    //Si canal es el mio ficticio 100
    //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
    int indice_canal=qltraps_find_open_file(pre_fs_mdinf_a[0] & 0xFFFF);
    if (indice_canal>=0 ) {

        //Indicar actividad en md flp
        ql_footer_mdflp_operating();

        //printf ("Mi canal MDINF\n");


        ql_restore_d_registers(pre_fs_mdinf_d,7);
        ql_restore_a_registers(pre_fs_mdinf_a,6);

        //Retornamos :
        //D1.L empty/good sectors. The number of empty sectors is in the most significant word (msw) of D1,
        //the total available on the medium is in the least significant word (lsw). A sector is 512 bytes.
        //de momento ,MDV files in QLAY format. Thee files must be exactly 174930 bytes 174930/512->aprox 341
        m68k_set_reg(M68K_REG_D1,32*65536+341); //32 sectores libres, 341 sectores ocupados
        //Por qué 32 sectores libres? Nada, por probar, porque no diga 0 simplemente



        //Volver de ese trap
        ql_qdos_return_from_trap();

        //No error.
        ql_qdos_set_return_no_error();

        //D1.W length of header read. A1 top of read buffer
        //m68k_set_reg(M68K_REG_D1,64);

        //Devolver medium name
        //A1 end of medium name  (entrada: A1 ptr to 10 byte buffer)

        unsigned int reg_a1=m68k_get_reg(NULL,M68K_REG_A1);
        unsigned int puntero=ql_get_a1_after_trap_4();

        reg_a1 +=10;
        m68k_set_reg(M68K_REG_A1,reg_a1);

        ql_writebyte(puntero++,'Z');
        ql_writebyte(puntero++,'E');
        ql_writebyte(puntero++,'s');
        ql_writebyte(puntero++,'a');
        ql_writebyte(puntero++,'r'); //5
        ql_writebyte(puntero++,'U');
        ql_writebyte(puntero++,'X');
        ql_writebyte(puntero++,'M');
        ql_writebyte(puntero++,'D');
        ql_writebyte(puntero++,' '); //10


        }
}


void handle_trap_fs_heads(void)
{
        debug_printf (VERBOSE_PARANOID,"QDOS handler: FS.HEADS. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );



		//printf("last trap = %d previous was trap4: %d\n",ql_last_trap,ql_previous_trap_was_4);

        //Si canal es el mio ficticio 100
        //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        int indice_canal=qltraps_find_open_file(pre_fs_heads_a[0] & 0xFFFF);
        if (indice_canal>=0 ) {

             //Indicar actividad en md flp
        	ql_footer_mdflp_operating();


        	ql_restore_d_registers(pre_fs_heads_d,7);
          	ql_restore_a_registers(pre_fs_heads_a,6);

            //ql_qdos_save_header(indice_canal);
            ql_put_file_header(indice_canal,ql_get_a1_after_trap_4() );


            //Return
            //D1.W length of header set
            //A1 end of header def

            //printf("Returning from FS.HEADS with no error\n");

            m68k_set_reg(M68K_REG_D1,14);

            //mostrar algunos bytes
          unsigned int reg_a1=ql_get_a1_after_trap_4();


          /*
          printf("Begin header\n");
          int i=0;
          for (i=0;i<64;i++) {
              printf ("%02XH ",peek_byte_z80_moto(reg_a1+i));
          }
          printf("End header\n");
          */

          reg_a1 +=14;
          m68k_set_reg(M68K_REG_A1,reg_a1);


          //Volver de ese trap
          ql_qdos_return_from_trap();

          //No error.
          ql_qdos_set_return_no_error();


        }
}


void handle_trap_io_sstrg(void)
{
        debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.SSTRG. Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH D2=%08XH",
        m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),
        m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_D2) );

		//printf("last trap = %d previous was trap4: %d\n",ql_last_trap,ql_previous_trap_was_4);

        //Si canal es el mio ficticio 100
        //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        int indice_canal=qltraps_find_open_file(pre_io_sstrg_a[0] & 0xFFFF);
        if (indice_canal>=0 ) {

            //Indicar actividad en md flp
        	ql_footer_mdflp_operating();

        	debug_printf (VERBOSE_PARANOID,"QDOS handler: Returning IO.SSTRG from our microdrive channel without error");

          	ql_restore_d_registers(pre_io_sstrg_d,7);
          	ql_restore_a_registers(pre_io_sstrg_a,6);

          	/*
          	Entrada:
          	D2.W nr of bytes to be sent
          	D3.W timeout
          	A0 channel ID
          	A1 base of buffer

          	Salida:
          	D1.W nr. of bytes sent
          	A1 updated ptr to buffer

          	Errores:
          	NC not complete
          	NO channel not open
          	EF end of file
          	BO buffer overflow (fetch line only)

          	*/



        	//O a A1 a secas
        	//depende de si se ha llamado trap4 o no



          	unsigned int puntero_origen;


			puntero_origen=ql_get_a1_after_trap_4();



          	debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.SSTRG - restoreg registers. Channel ID=%d Base of buffer A1=%08XH A3=%08XH A6=%08XH D2=%08XH",
            m68k_get_reg(NULL,M68K_REG_A0),m68k_get_reg(NULL,M68K_REG_A1),m68k_get_reg(NULL,M68K_REG_A3),
            m68k_get_reg(NULL,M68K_REG_A6),m68k_get_reg(NULL,M68K_REG_D2) );



          	int longitud=m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF;

            //Grabar los datos en disco
        	FILE *ptr_file;
        	ptr_file=qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix;
            int i=0;
            for (i=0;i<longitud;i++) {
                moto_byte byte_leido;
                byte_leido=peek_byte_z80_moto(puntero_origen+i);
                fwrite(&byte_leido,1,1,ptr_file);
            }


          	//Mostrar parte del mensaje enviado en SSTRG
          	//unsigned int puntero_destino=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);
            char buffer_mensaje[256];
          	if (longitud>32) longitud=32;

          	i=0;
          	char byte_leido;

          	while (longitud>=0) {
          		byte_leido=ql_readbyte(puntero_origen);
          		if (byte_leido>=32 && byte_leido<=126) {
          			buffer_mensaje[i]=byte_leido;
          			i++;
          		}
          		else {
          			sprintf(&buffer_mensaje[i],"%02XH ",byte_leido);

          			i+=4;
          		}

          		puntero_origen++;
				  longitud--;
          	}

          	buffer_mensaje[i]=0;

          	debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.SSTRG - message sent: %s",buffer_mensaje);


          	//bytes enviados
          	m68k_set_reg(M68K_REG_D1,m68k_get_reg(NULL,M68K_REG_D2) );


          	//Aumentar puntero A1
          	unsigned int registro_a1=m68k_get_reg(NULL,M68K_REG_A1);
          	registro_a1 +=m68k_get_reg(NULL,M68K_REG_D2);
          	m68k_set_reg(M68K_REG_A1,registro_a1);



        	  //No error.
          	ql_qdos_set_return_no_error();

          //Volver de ese trap
          ql_qdos_return_from_trap();



        }

		else {
			//Pruebas debug mensaje. Parece que solo muestra correctamente texto de los canales inferiores

          	unsigned int puntero_origen;


			puntero_origen=ql_get_a1_after_trap_4();


          	//Mostrar parte del mensaje enviado en SSTRG
          	//unsigned int puntero_destino=m68k_get_reg(NULL,M68K_REG_A1)+m68k_get_reg(NULL,M68K_REG_A6);
          	char buffer_mensaje[256];
          	int longitud=m68k_get_reg(NULL,M68K_REG_D2) & 0xFFFF;
          	if (longitud>32) longitud=32;

			//printf ("Debugging message sent. longitud=%d\n",longitud);

          	int i=0;
          	char byte_leido;

          	while (longitud>=0) {
          		byte_leido=ql_readbyte(puntero_origen);
          		if (byte_leido>=32 && byte_leido<=126) {
          			buffer_mensaje[i]=byte_leido;
          			i++;
          		}
          		else {
          			sprintf(&buffer_mensaje[i],"%02XH ",byte_leido);

          			i+=4;
          		}

          		puntero_origen++;
				longitud--;
          	}

          	buffer_mensaje[i]=0;

          	debug_printf (VERBOSE_PARANOID,"QDOS handler: IO.SSTRG - message sent: %s",buffer_mensaje);

			//printf ("message sent: %s\n",buffer_mensaje);


		}
}


void handle_trap_fs_load(void)
{
        debug_printf (VERBOSE_PARANOID,"QDOS handler: FS.LOAD. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

        //Si canal es el mio ficticio 100
        //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        int indice_canal=qltraps_find_open_file(pre_fs_load_a[0] & 0xFFFF);
        if (indice_canal>=0 ) {

            //Indicar actividad en md flp
        	ql_footer_mdflp_operating();

          ql_restore_d_registers(pre_fs_load_d,7);
          ql_restore_a_registers(pre_fs_load_a,6);

          unsigned int longitud=m68k_get_reg(NULL,M68K_REG_D2);


            debug_printf (VERBOSE_PARANOID,"QDOS handler: Loading file at address %05XH with length: %d",m68k_get_reg(NULL,M68K_REG_A1),longitud);
            //void load_binary_file(char *binary_file_load,int valor_leido_direccion,int valor_leido_longitud)


            //longitud la saco del propio archivo, ya que no me llega bien de momento pues no retornaba bien fs.headr
            //int longitud=get_file_size(ql_nombre_archivo_load);
        	FILE *ptr_file;
        	ptr_file=qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix;
            ql_load_binary_file(ptr_file,ql_get_a1_after_trap_4(),longitud);



          //Volver de ese trap
          ql_qdos_return_from_trap();

          //No error.
          ql_qdos_set_return_no_error();

          //m68k_set_reg(M68K_REG_D0,-7);


          //Decimos que A1 es A1 top address after load
          unsigned int reg_a1=m68k_get_reg(NULL,M68K_REG_A1);
          reg_a1 +=longitud;
          m68k_set_reg(M68K_REG_A1,reg_a1);



        }
}


void handle_trap_fs_save(void)
{
        debug_printf (VERBOSE_PARANOID,"QDOS handler: FS.SAVE. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) );

        //Si canal es el mio ficticio 100
        //int indice_canal=qltraps_find_open_file(m68k_get_reg(NULL,M68K_REG_A0));
        int indice_canal=qltraps_find_open_file(pre_fs_save_a[0] & 0xFFFF);
        if (indice_canal>=0 ) {

            //Indicar actividad en md flp
            ql_footer_mdflp_operating();

          ql_restore_d_registers(pre_fs_save_d,7);
          ql_restore_a_registers(pre_fs_save_a,6);

          unsigned int longitud=m68k_get_reg(NULL,M68K_REG_D2);


            debug_printf (VERBOSE_PARANOID,"QDOS handler: Saving file from address %05XH with length: %d",m68k_get_reg(NULL,M68K_REG_A1),longitud);
            //void load_binary_file(char *binary_file_load,int valor_leido_direccion,int valor_leido_longitud)



            //Grabar los datos en disco
        	FILE *ptr_file;
        	ptr_file=qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix;
            //ql_load_binary_file(ptr_file,ql_get_a1_after_trap_4(),longitud);

            unsigned int puntero_origen=ql_get_a1_after_trap_4();

            unsigned int i=0;
            for (i=0;i<longitud;i++) {
                moto_byte byte_leido;
                byte_leido=peek_byte_z80_moto(puntero_origen+i);
                fwrite(&byte_leido,1,1,ptr_file);
            }


          //Volver de ese trap
          ql_qdos_return_from_trap();

          //No error.
          ql_qdos_set_return_no_error();



          //Decimos que A1 es A1 top address after load
          unsigned int reg_a1=m68k_get_reg(NULL,M68K_REG_A1);
          reg_a1 +=longitud;
          m68k_set_reg(M68K_REG_A1,reg_a1);


        }
}



int ql_qdos_check_device_readonly(char *device)
{
    //Ver si ese device es read only
    //printf("device: %s\n",device);

    if (!strcasecmp(device,"mdv1")) return ql_device_mdv1_readonly;
    if (!strcasecmp(device,"mdv2")) return ql_device_mdv2_readonly;
    if (!strcasecmp(device,"flp1")) return ql_device_flp1_readonly;

    //Cualquier otra cosa, asumimos read only
    return 1;

}



void ql_rom_traps(void)
{
    if (ql_initial_autoload) {

        if (get_pc_register()==0x4af6) {


            debug_printf(VERBOSE_INFO,"QL Trap ROM: Read F1 or F2 when initial autoload. Simulate press F1");

            //printf("QL Trap ROM: Read F1 or F2 when initial autoload. Simulate press F1\n");

            ql_initial_autoload_counter=contador_segundo_infinito;

            //F1
            //ql_keyboard_table[0] &= (255-2);

        }

        if (ql_initial_autoload_counter!=0) {
            if (contador_segundo_infinito-ql_initial_autoload_counter>1000) {
                debug_printf(VERBOSE_INFO,"Release F1 after 1 second");

                //printf("Release F1 after 1 second\n");

                //ql_keyboard_table[0] |= 2;
                ql_initial_autoload_counter=0;
                ql_initial_autoload=0;
            }
        }

    }



    //Traps de intercepcion de teclado
    //F1 y F2 iniciales
    /*
    Para parar y simular F1:

sb 2 pc=4AF6H
set-register pc=4AF8H  (saltamos el trap 3)
set-register D1=E8H    (devolvemos tecla f1)
(o para simular F2: set-register D1=ECH)
  */

    //Si pulsado F1 o F2
    //Trap ya no hace falta. Esto era necesario cuando la lectura de teclado no funcionaba y esta era la unica manera de simular F1 o F2
    /*
    if ( get_pc_register()==0x4af6 &&
        ((ql_keyboard_table[0]&2)==0 || (ql_keyboard_table[0]&8)==0)
        )     {


          debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Read F1 or F2");

      //Saltar el trap 3
      m68k_set_reg(M68K_REG_PC,0x4AF8);

      //Si F1
      if ((ql_keyboard_table[0]&2)==0) m68k_set_reg(M68K_REG_D1,0xE8);
      //Pues sera F2
      else m68k_set_reg(M68K_REG_D1,0xEC);

    }
*/



    //Saltar otro trap que hace mdv1 boot
    /*
    04B4C trap    #$2
04B4E tst.l   D0
04B50 rts


TRAP #2 D0=$1
Open a channel
IO.OPEN


A0=00004BE4 :

L04BE4 DC.W    $0009
       DC.B    'MDV1_BOOT'
       DC.B    $00


saltamos ese trap
set-register pc=04B50h

    */
    //TODO: saltar esta llamada de manera mas elegante
    /*if ( get_pc_register()==0x04B4C) {
      debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Skipping MDV1 boot");
      m68k_set_reg(M68K_REG_PC,0x04B50);
    }*/


    //Decir 1 tecla pulsada. Se supone que este trap no deberia hacer falta si se gestionase del todo bien el ipc
    if (get_pc_register()==0x02D40) {
      //Decir que hay una tecla pulsada

      if (ql_pulsado_tecla()) {
        //debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Tell one key pressed");
        m68k_set_reg(M68K_REG_D7,0x01);
      }
      else {
        //printf ("no tecla pulsada\n");
        ql_mantenido_pulsada_tecla=0;
      }
    }

    //Decir 1 tecla pulsada. Se supone que este trap no deberia hacer falta si se gestionase del todo bien el ipc
    if (get_pc_register()==0x02E6A) {

      if (ql_pulsado_tecla()) {
        //debug_printf(VERBOSE_DEBUG,"QL Trap ROM: Tell 1 key pressed");
        m68k_set_reg(M68K_REG_D1,1);
        //L02E6A MOVE.B  D1,D5         * d1=d5=d4 : number of bytes in buffer
      }
      else {
        m68k_set_reg(M68K_REG_D1,0); //0 teclas pulsadas
      }
    }



/*
Info rapida de como funcionan los traps:
Detectar primero si se llama a trap 1, 2 o 3 , y llamar a core_ql_trap_one, two o three segun detectado
En esas funciones , cuando es alguna funcion de qdos que estamos gestionando, se guardan los registros A y D del Motorola, para su posterior uso

Esos traps en la rom acaban saltando a unas direcciones mas altas, y son las que posteriormente intercepto,
con la funcion exacta del qdos, como ejemplo:


//Trap2, IO.OPEN
if (get_pc_register()==0x032B4 && m68k_get_reg(NULL,M68K_REG_D0)==1) {

Cuando salta ahi, los registros A y D se han modificado algunos desde que he detectado el trap, por eso los guardo antes.
Cuando se detecta la funcion exacta del qdos, como este con D0=1, restauro los registros que tenia salvados antes para obtener
las variables de entrada (tal y como entraban al principio del trap). Con esos registros restaurados ya se qué hace la llamada a qdos.
Se realiza la función adecuada a esa llamada: abrir fichero, cerrar, leer, etc

Para volver del trap despues de haberlo interceptado, cambio el registro pc a una instruccion rte que hau en 53H de la rom
Ajusto también el stack de salida para que vuelva tal cual deberia del trap inicial (digamos que evito algun push y algun salto)
Y el registro D0 siempre contiene el codigo de error/ok de retorno del trap
Con esto ya se vuelve del trap: un tanto chapucero pero funciona

Esto probado con la rom ql_js.rom, con otras, es probable que falle.

*/



    if (get_pc_register()==0x0031e) {
        core_ql_trap_one();
    }



//00324 bsr     336
//Interceptar trap 2
/*
trap 2 salta a:
00324 bsr     336
*/
    if (get_pc_register()==0x00324) {
        core_ql_trap_two();
    }



    //Interceptar trap 3
    /*
    trap 3 salta a:
    0032A bsr     336
    */
    if (get_pc_register()==0x0032a) {
        core_ql_trap_three();
    }





    if (get_pc_register()==0x032B4) {
        ql_post_trap_two();
    }


    if (get_pc_register()==0x0337C) {
        ql_post_trap_three();
    }

}


void ql_post_trap_three(void)
{

    //Rutinas de post trap 3

    if (!ql_microdrive_floppy_emulation) return;

    switch(m68k_get_reg(NULL,M68K_REG_D0)) {

        //Trap 3 IO.FLINE
        case 0x2:
            handle_trap_io_fline_fstrg();
        break;

        //Trap 3 IO.FSTRG
        case 0x3:
            handle_trap_io_fline_fstrg();
        break;

        //Trap 3 IO.EDLIN
        case 0x4:
            handle_trap_io_edlin();
        break;

        //Trap 3 IO.SSTRG
        case 0x7:
            handle_trap_io_sstrg();
        break;

        //Trap 3 FS.MDINF
        case 0x45:
            handle_trap_fs_mdinf();
        break;

        //Trap 3 FS.HEADS. Se usa con sbytes: hace io.open y luego llama aqui
        case 0x46:
            handle_trap_fs_heads();
        break;

        //Quiza Trap 3 FS.HEADR acaba saltando a 0337C move.l  A0, D7
        case 0x47:
            handle_trap_fs_headr();
        break;

        //Trap 3. FS.LOAD
        case 0x48:
            handle_trap_fs_load();
        break;

        //Trap 3. FS.SAVE
        case 0x49:
            handle_trap_fs_save();
        break;

    }

}




void  ql_post_trap_two(void)
{

    //Rutinas de post trap 2


    //Interceptar trap 2, con d0=1, cuando ya sabemos la direccion
    //IO.OPEN
/*
command@cpu-step> cs
PC: 032B4 SP: 2846E USP: 3FFC0 SR: 2000 :  S         A0: 0003FDEE A1: 0003EE00 A2: 00006906 A3: 00000670 A4: 00000012 A5: 0002846E A6: 00028000 A7: 0002846E D0: 00000001 D1: FFFFFFFF D2: 00000058 D3: 00000001 D4: 00000001 D5: 00000000 D6: 00000000 D7: 0000007F
032B4 subq.b  #1, D0

D3.L: code:
0 old (exclusive) file or device
1 old (shared) file
2 new (exclusive) file
3 new (overwrite) file
4 open directory

*/

  //Nota: lo normal seria que no hagamos este trap a no ser que se habilite emulacion de ql_microdrive_floppy_emulation.
  //Pero, si lo hacemos asi, si no habilitamos emulacion de micro&floppy, al pasar del menu de inicio (F1,F2) buscara el archivo BOOT, y como no salta el
  //trap, se queda bloqueado
  //Mas adelante en este caso comprobamos si esta habilitada emulacion de microdrive
  //Ademas cualquier load desde microdrive (con ql_microdrive_floppy_emulation desactivado) se quedaria colgado si no lo interceptamos

    if (m68k_get_reg(NULL,M68K_REG_D0)==1) {
      //en A0
      char ql_nombre_archivo_load[255];
      int reg_a0=m68k_get_reg(NULL,M68K_REG_A0);

      //Aunque no entiendo aun, hay veces que A0 no entra bien cuando se guarda en pre_io_open_a[0]
      //int reg_a0=pre_io_open_a[0];
      //Haciendo: a=respr(10000): lbytes mdv1_ext,a -> mete a0=a1=f8ah y en esa f8ah no hay nada con sentido
      //Pero cuando entramos aquí en pc=0x032B4, A0 está bien. raro raro...
      //Es mas, justo antes de llamar a ese trap, los registros estan asi mal

	//if (ql_previous_trap_was_4) {
	//			reg_a0 += m68k_get_reg(NULL,M68K_REG_A6);
	//}

      debug_printf (VERBOSE_PARANOID,"QDOS handler: Pointer to file name: %X",reg_a0);
      int longitud_nombre=peek_byte_z80_moto(reg_a0)*256+peek_byte_z80_moto(reg_a0+1);
      reg_a0 +=2;
      debug_printf (VERBOSE_PARANOID,"QDOS handler: Length channel name: %d",longitud_nombre);

      //comprobar limite
      if (longitud_nombre>254) longitud_nombre=254;

      char c;
      int i=0;
      for (;longitud_nombre;longitud_nombre--) {
        c=peek_byte_z80_moto(reg_a0++);
        ql_nombre_archivo_load[i++]=c;
        //printf ("%c",c);
      }
      //printf ("\n");
      ql_nombre_archivo_load[i++]=0;


      debug_printf (VERBOSE_PARANOID,"QDOS handler: Channel name: %s",ql_nombre_archivo_load);




      //Hacer que si es mdv1_ ... volver



      int es_dispositivo=0;

      int hacer_trap=0;


        //Si tiene prefijo mdv1, mdv2 o flp1
      if (ql_si_ruta_mdv_flp(ql_nombre_archivo_load)) {
          //printf("hacer trap\n");
          hacer_trap=1;
      }

        /* esto en teoria ya no sirve
      if (!hacer_trap) {
      	if (
      		ql_si_ruta_parametro(ql_nombre_archivo_load,"mdv") ||
      		ql_si_ruta_parametro(ql_nombre_archivo_load,"flp")
      	    ) {
            printf("hacer trap y es dispositivo\n");
      		hacer_trap=1;
      		es_dispositivo=1;
      	}
      }
      */

     //Ver si ruta es tal cual mdv1_,mdv2_ o flp1_ que indica que se abre el dispositivo entero
     if (
         !strcasecmp(ql_nombre_archivo_load,"mdv1_") ||
         !strcasecmp(ql_nombre_archivo_load,"mdv2_") ||
         !strcasecmp(ql_nombre_archivo_load,"flp1_")
     )
     {
            //printf("hacer trap y es dispositivo\n");

            //Se supone que hacer_trap ya se ha activado en la comprobacion anterior, pero por si acaso

      		hacer_trap=1;
      		es_dispositivo=1;
     }


      moto_byte file_mode;

    //printf("antes de hacer_trap: %d\n",hacer_trap);

      if (hacer_trap) {

        //debug_printf (VERBOSE_PARANOID,"Returning from trap without opening anything because file is mdv1, mdv2 or flp1");

        //ql_debug_force_breakpoint("En IO.OPEN");

/*
069CC movea.l A1, A0                              |L069CC MOVEA.L A1,A0
069CE move.w  (A6,A1.l), -(A7)                    |       MOVE.W  $00(A6,A1.L),-(A7)
069D2 trap    #$4                                 |       TRAP    #$04
>069D4 trap    #$2                                 |       TRAP    #$02
069D6 moveq   #$3, D3                             |       MOVEQ   #$03,D3
069D8 add.w   (A7)+, D3                           |       ADD.W   (A7)+,D3
069DA bclr    #$0, D3                             |       BCLR    #$00,D3
069DE add.l   D3, ($58,A6)                        |       ADD.L   D3,$0058(A6)

Es ese trap 2 el que se llama al hacer lbytes mdv...

Y entra asi:
command@cpu-step> run
Running until a breakpoint, menu opening or other event
PC: 069D4 SP: 3FFC0 USP: 3FFC0 SR: 0000 :

A0: 00000D88 A1: 00000D88 A2: 00006906 A3: 00000668 A4: 00000012 A5: 00000670 A6: 0003F068 A7: 0003FFC0 D0: 00000001 D1: FFFFFFFF D2: 00000058 D3: 00000001 D4: 00000001 D5: 00000000 D6: 00000000 D7: 00000000
069D4 trap    #$2

*/

        //D2,D3,A2,A3 se tienen que preservar, segun dice el trap.
        //Segun la info general de los traps, tambien se deben guardar de D4 a D7 y A4 a A6. Directamente guardo todos los D y A excepto A7

        ql_restore_d_registers(pre_io_open_d,7);
        ql_restore_a_registers(pre_io_open_a,6);
        //ql_restore_a_registers(pre_io_open_a,7);


        //Volver de ese trap
        ql_qdos_return_from_trap();


        //No error.
        ql_qdos_set_return_no_error();

        char ql_io_open_device[PATH_MAX];
        char ql_io_open_file[PATH_MAX];

        char ql_nombrecompleto[PATH_MAX];

        if (!es_dispositivo) {


            //Si no hay root folder, directamente decimos que no se encuentra archivo
            if (!ql_microdrive_floppy_emulation) {
          		debug_printf(VERBOSE_DEBUG,"QDOS handler: Microdrive emulation not enabled");
          		//Retornar Not found (NF)
          		m68k_set_reg(M68K_REG_D0,-7);
          		return;
            }




   	        ql_split_path_device_name(ql_nombre_archivo_load,ql_io_open_device,ql_io_open_file,0,0);

            //printf("device: %s\n",ql_io_open_device);

            int error_full_path=ql_return_full_path(ql_io_open_device,ql_io_open_file,ql_nombrecompleto);

            //Indicar actividad en md flp
        	ql_footer_mdflp_operating();

        	if (error_full_path) {
                //Error dispositivo invalido o no habilitado
                debug_printf(VERBOSE_DEBUG,"QDOS handler: Invalid device or not enabled: %s",ql_io_open_device);
          		//Retornar Not found (NF)
          		m68k_set_reg(M68K_REG_D0,-7);
          		return;
            }

            //Ver modo archivo
            file_mode=m68k_get_reg(NULL,M68K_REG_D3);
            debug_printf(VERBOSE_DEBUG,"QDOS handler: File mode: %d",file_mode);
            /*
            D3.L: code:
0 old (exclusive) file or device
1 old (shared) file
2 new (exclusive) file
3 new (overwrite) file
4 open directory
            */

           //Si modo es 2 o 3, no ver si existe
           if (file_mode!=2 && file_mode!=3) {



        //Para siguientes io.fline
        //ptr_io_fline=NULL;
        	if (!si_existe_archivo(ql_nombrecompleto)) {
          		debug_printf(VERBOSE_DEBUG,"QDOS handler: File %s not found. Trying changing last _ to .",ql_nombrecompleto);

                ql_split_path_device_name(ql_nombre_archivo_load,ql_io_open_device,ql_io_open_file,1,1);

        	    ql_return_full_path(ql_io_open_device,ql_io_open_file,ql_nombrecompleto);
        	}

        	if (!si_existe_archivo(ql_nombrecompleto)) {
          		debug_printf(VERBOSE_DEBUG,"QDOS handler: File %s not found. Trying changing all _ to .",ql_nombrecompleto);

                ql_split_path_device_name(ql_nombre_archivo_load,ql_io_open_device,ql_io_open_file,1,0);

        	    ql_return_full_path(ql_io_open_device,ql_io_open_file,ql_nombrecompleto);
        	}


        	if (!si_existe_archivo(ql_nombrecompleto)) {
          		debug_printf(VERBOSE_DEBUG,"QDOS handler: File %s not found",ql_nombrecompleto);
          		//Retornar Not found (NF)
          		m68k_set_reg(M68K_REG_D0,-7);
          		return;
        	}

           }
	}



	//Obtenemos canal disponible
	int canal=qltraps_find_free_fopen();
	if (canal<0) {
		//No hay disponibles. Error.
  		m68k_set_reg(M68K_REG_D0,QDOS_ERROR_CODE_NC);
  		return;
	}

	//Se ha retornado indice al array. Canal sera sumando
	m68k_set_reg(M68K_REG_A0,canal+QLTRAPS_START_FILE_NUMBER);


	//Resetear eof
	qltraps_fopen_files[canal].next_eof_ptr_io_fline=0;


	strcpy(qltraps_fopen_files[canal].ql_file_name,ql_nombre_archivo_load);

    //printf("io open es dispositivo: %d\n",es_dispositivo);
	qltraps_fopen_files[canal].es_dispositivo=es_dispositivo;

    if (es_dispositivo) {

                if (!ql_microdrive_floppy_emulation) {
                    debug_printf(VERBOSE_PARANOID,"QDOS handler: Microdrive emulation not enabled");
                    //Retornar Not found (NF)
                    m68k_set_reg(M68K_REG_D0,-7);

                    return;
                }



   	        ql_split_path_device_name(ql_nombre_archivo_load,ql_io_open_device,ql_io_open_file,0,0);

            //printf("device: %s\n",ql_io_open_device);

            int error_full_path=ql_return_full_path(ql_io_open_device,ql_io_open_file,ql_nombrecompleto);

            //Indicar actividad en md flp
        	ql_footer_mdflp_operating();

        	if (error_full_path) {
                //Error dispositivo invalido o no habilitado
                debug_printf(VERBOSE_DEBUG,"QDOS handler: Invalid device or not enabled: %s",ql_io_open_device);
          		//Retornar Not found (NF)
          		m68k_set_reg(M68K_REG_D0,-7);
          		return;
            }

            //Se abre directorio
            debug_printf(VERBOSE_DEBUG,"QDOS handler: Opening directory %s",ql_nombrecompleto);

        	qltraps_fopen_files[canal].qltraps_handler_dfd = opendir(ql_nombrecompleto);

            if (qltraps_fopen_files[canal].qltraps_handler_dfd == NULL) {
                debug_printf(VERBOSE_DEBUG,"QDOS handler: Can't open directory %s",ql_nombrecompleto);

                //error
                m68k_set_reg(M68K_REG_D0,QDOS_ERROR_CODE_NC);
                return;
            }
    }

    //Asumimos que no tiene cabecera al leerlo
    qltraps_fopen_files[canal].has_header_on_read=0;
    qltraps_fopen_files[canal].has_header_no_magic_on_read=0;

	if (!es_dispositivo) {
		//Indicar file handle
		FILE *archivo;

        //printf("ql_nombrecompleto: %s\n",ql_nombrecompleto);

        //Si modo es escritura
        if (file_mode==2 || file_mode==3) {
            //Ver si ese device esta permitido escribir (ql_io_open_device)
            if (ql_qdos_check_device_readonly(ql_io_open_device)) {
                //Retornar error not complete (no hay error de read only)
                debug_printf(VERBOSE_DEBUG,"QDOS handler: Device %s is set as read only",ql_io_open_device);
                m68k_set_reg(M68K_REG_D0,QDOS_ERROR_CODE_NC);
                return;
            }

            archivo=fopen(ql_nombrecompleto,"wb");

        }
		else {
            archivo=fopen(ql_nombrecompleto,"rb");
        }

        //printf("archivo pointer: %p\n",archivo);

		if (archivo==NULL) {
        		debug_printf(VERBOSE_PARANOID,"QDOS handler: File %s not found",ql_nombrecompleto);
	  		//Retornar Not found (NF)
  			m68k_set_reg(M68K_REG_D0,-7);
  			return;
		}


		qltraps_fopen_files[canal].qltraps_last_open_file_handler_unix=archivo;


        if (file_mode!=2 && file_mode!=3) {
            //Leemos cabecera si es que tiene
            //Ver si tiene cabecera el archivo, en el caso de abrir para lectura
            //Y la guardamos en nuestra estructura de archivos
            int tiene_cabecera=ql_if_file_has_header(canal);

            if (tiene_cabecera) {


                //Leemos esa cabecera
                    //printf("Reading QDOS file header\n");



                    qltraps_fopen_files[canal].has_header_on_read=1;
                    fread(qltraps_fopen_files[canal].file_header,1,tiene_cabecera,archivo);



                    //Debug escribir cabecera
                    /*
                    int i;
                    for (i=0;i<tiene_cabecera;i++) {
                        moto_byte byte_leido=qltraps_fopen_files[canal].file_header[i];

                        if (byte_leido>=32 && byte_leido<=126) printf ("%c",byte_leido);
                        else printf(" %02XH ",byte_leido);
                    }
                    printf("\n");
                    */



            }
        }


		//Le hacemos un stat
		if (stat(ql_nombrecompleto, &qltraps_fopen_files[canal].last_file_buf_stat)!=0) {
			debug_printf (VERBOSE_DEBUG,"QDOS handler: Unable to get status of file %s",ql_nombrecompleto);
		}

	}


	//Indicamos en array que esta abierto
	qltraps_fopen_files[canal].open_file.v=1;

                //Y poner nombres para debug
                strcpy(qltraps_fopen_files[canal].debug_name,ql_nombre_archivo_load);
                strcpy(qltraps_fopen_files[canal].debug_fullpath,ql_nombrecompleto);



        //D1= Job ID. TODO. Parece que da error "error in expression" porque no se asigna un job id valido?
        //Parece que D1 entra con -1, que quiere decir "the channel will be associated with the current job"
        //m68k_set_reg(M68K_REG_D1,0); //Valor de D1 inventado. Da igual, tambien fallara
        /*

        */

        return;

      }




    }





    //IO.CLOSE
    if (m68k_get_reg(NULL,M68K_REG_D0)==2 && ql_microdrive_floppy_emulation) {


        //Tiene pinta que el canal son los 16 bits inferiores
    	//debug_printf (VERBOSE_DEBUG,"IO.CLOSE. Channel ID=%d",m68k_get_reg(NULL,M68K_REG_A0) & 0xFFFF );


        debug_printf (VERBOSE_DEBUG,"QDOS handler: IO.CLOSE. Channel ID=%d",pre_io_close_a[0] & 0xFFFF );


      	//Si canal es el mio ficticio
       	int indice_canal=qltraps_find_open_file(pre_io_close_a[0] & 0xFFFF);

        if (indice_canal>=0  ) {
            //Indicar actividad en md flp
        	ql_footer_mdflp_operating();

        	debug_printf (VERBOSE_DEBUG,"QDOS handler: Closing file/device %s",qltraps_fopen_files[indice_canal].ql_file_name);

    	    ql_restore_d_registers(pre_io_close_d,7);
            ql_restore_a_registers(pre_io_close_a,6);

        	//Si no es dispositivo, fclose
        	if (!qltraps_fopen_files[indice_canal].es_dispositivo) {
        		fclose(qltraps_fopen_files[indice_canal].qltraps_last_open_file_handler_unix);
        	}

        	//Liberar ese item del array
        	qltraps_fopen_files[indice_canal].open_file.v=0;


        	//Volver de ese trap
        	ql_qdos_return_from_trap();


        	//No error.
        	ql_qdos_set_return_no_error();


       }

    }

    //IO.DELET
    if (m68k_get_reg(NULL,M68K_REG_D0)==4) {
      //en A0
      char ql_nombre_archivo_delet[255];
      int reg_a0=m68k_get_reg(NULL,M68K_REG_A0);

      //Aunque no entiendo aun, hay veces que A0 no entra bien cuando se guarda en pre_io_open_a[0]
      //int reg_a0=pre_io_open_a[0];
      //Haciendo: a=respr(10000): lbytes mdv1_ext,a -> mete a0=a1=f8ah y en esa f8ah no hay nada con sentido
      //Pero cuando entramos aquí en pc=0x032B4, A0 está bien. raro raro...
      //Es mas, justo antes de llamar a ese trap, los registros estan asi mal

	//if (ql_previous_trap_was_4) {
	//			reg_a0 += m68k_get_reg(NULL,M68K_REG_A6);
	//}

      debug_printf (VERBOSE_PARANOID,"QDOS handler: Pointer to file name: %X",reg_a0);
      int longitud_nombre=peek_byte_z80_moto(reg_a0)*256+peek_byte_z80_moto(reg_a0+1);
      reg_a0 +=2;
      debug_printf (VERBOSE_PARANOID,"QDOS handler: Length channel name: %d",longitud_nombre);

      //comprobar limite
      if (longitud_nombre>254) longitud_nombre=254;

      char c;
      int i=0;
      for (;longitud_nombre;longitud_nombre--) {
        c=peek_byte_z80_moto(reg_a0++);
        ql_nombre_archivo_delet[i++]=c;
        //printf ("%c",c);
      }
      //printf ("\n");
      ql_nombre_archivo_delet[i++]=0;


      debug_printf (VERBOSE_PARANOID,"QDOS handler: Channel name: %s",ql_nombre_archivo_delet);





        //Si tiene prefijo mdv1, mdv2 o flp1
      if (ql_si_ruta_mdv_flp(ql_nombre_archivo_delet)) {


        ql_restore_d_registers(pre_io_delet_d,7);
        ql_restore_a_registers(pre_io_delet_a,6);
        //ql_restore_a_registers(pre_io_open_a,7);


        //Volver de ese trap
        ql_qdos_return_from_trap();


        //No error.
        ql_qdos_set_return_no_error();

        char ql_io_delet_device[PATH_MAX];
        char ql_io_delet_file[PATH_MAX];

        char ql_nombrecompleto[PATH_MAX];




        //Si no hay root folder, directamente decimos que no se encuentra archivo
        if (!ql_microdrive_floppy_emulation) {
            debug_printf(VERBOSE_DEBUG,"QDOS handler: Microdrive emulation not enabled");
            //Retornar Not found (NF)
            m68k_set_reg(M68K_REG_D0,-7);
            return;
        }


        //Indicar actividad en md flp
        ql_footer_mdflp_operating();

        ql_split_path_device_name(ql_nombre_archivo_delet,ql_io_delet_device,ql_io_delet_file,0,0);

        //printf("device: %s\n",ql_io_delet_device);

        ql_return_full_path(ql_io_delet_device,ql_io_delet_file,ql_nombrecompleto);





        if (!si_existe_archivo(ql_nombrecompleto)) {
            debug_printf(VERBOSE_DEBUG,"QDOS handler: File %s not found. Trying changing last _ to .",ql_nombrecompleto);

            ql_split_path_device_name(ql_nombre_archivo_delet,ql_io_delet_device,ql_io_delet_file,1,1);

            ql_return_full_path(ql_io_delet_device,ql_io_delet_file,ql_nombrecompleto);
        }

        if (!si_existe_archivo(ql_nombrecompleto)) {
            debug_printf(VERBOSE_DEBUG,"QDOS handler: File %s not found. Trying changing all _ to .",ql_nombrecompleto);

            ql_split_path_device_name(ql_nombre_archivo_delet,ql_io_delet_device,ql_io_delet_file,1,0);

            ql_return_full_path(ql_io_delet_device,ql_io_delet_file,ql_nombrecompleto);
        }


        if (!si_existe_archivo(ql_nombrecompleto)) {
            debug_printf(VERBOSE_DEBUG,"QDOS handler: File %s not found",ql_nombrecompleto);
            //Retornar Not found (NF)
            m68k_set_reg(M68K_REG_D0,-7);
            return;
        }




        //Ver si ese device esta permitido escribir (ql_io_delet_device)
        if (ql_qdos_check_device_readonly(ql_io_delet_device)) {
            //Retornar error not complete (no hay error de read only)
            debug_printf(VERBOSE_DEBUG,"QDOS handler: Device %s is set as read only",ql_io_delet_device);
            m68k_set_reg(M68K_REG_D0,QDOS_ERROR_CODE_NC);
            return;
        }

        debug_printf (VERBOSE_DEBUG,"Deleting file %s",ql_nombrecompleto);
        util_delete(ql_nombrecompleto);





	}



    return;

    }









}
