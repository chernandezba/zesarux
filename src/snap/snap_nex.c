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


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cpu.h"
#include "debug.h"
#include "operaciones.h"
#include "snap_nex.h"

#include "autoselectoptions.h"
#include "compileoptions.h"
#include "esxdos_handler.h"
#include "mem128.h"
#include "settings.h"
#include "snap.h"
#include "tbblue.h"
#include "timex.h"
#include "ula.h"

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

void load_nex_snapshot_change_to_next(void)
{
	//cambio a maquina tbblue, siempre
	//if (!MACHINE_IS_TBBLUE) {

        current_machine_type=MACHINE_ID_TBBLUE;

		//temporalmente ponemos tbblue fast boot mode y luego restauramos valor anterior
		z80_bit antes_tbblue_fast_boot_mode;
		antes_tbblue_fast_boot_mode.v=tbblue_fast_boot_mode.v;
		tbblue_fast_boot_mode.v=1;

        set_machine(NULL);
        hard_reset_cpu();

		tbblue_fast_boot_mode.v=antes_tbblue_fast_boot_mode.v;
	//}
}

void load_nex_snapshot_if_mount_exdos_folder(char *archivo)
{
    if (automount_esxdos_nex.v==0) return;


    if (esxdos_handler_enabled.v==0) {
        esxdos_handler_enable();
    }

    util_get_dir(archivo,esxdos_handler_root_dir);

    //Y decir que al hacer reset, se quitara
    esxdos_umount_on_reset.v=1;
}




//Devuelve el offset al bloque de memoria, teniendo en cuenta que los primeros 6 estan desordenados:
//5,2,0,1,3,4,6,7,8,9,10,...,111
/*int load_nex_snapshot_get_ram_offset(int block)
{
	switch (block) {
		default:
			return block*16384;
		break;
	}
}*/

//funcion derivada y reducida de esxdos_handler_call_f_open
//Si variable forzar_filehandle_cero es diferente de 0, es siempre el 0
//Retorna file handle. Si <0, error
int load_nex_snapshot_open_esxdos(char *nombre_archivo,int forzar_filehandle_cero)
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

	//Abrir para lectura
	char *fopen_mode="rb";

	//z80_byte modo_abrir=reg_b;


	//Ver si no se han abierto el maximo de archivos y obtener handle libre
    int free_handle;

    if (forzar_filehandle_cero==0) {
	    free_handle=esxdos_find_free_fopen();
    }

    else {
        free_handle=0;
    }

	if (free_handle==-1) {
		//esxdos_handler_error_carry(ESXDOS_ERROR_ENFILE);
		//esxdos_handler_old_return_call();
		return -1;
	}


	char fullpath[PATH_MAX];


	esxdos_fopen_files[free_handle].tiene_plus3dos_header.v=0;


	//esxdos_handler_pre_fileopen(nombre_archivo,fullpath);
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//TODO: si archivo a cargar .nex esta fuera del esxdos root dir, deberia dar error
	//Si empieza por /, es ruta absoluta
	if (nombre_archivo[0]=='/' || nombre_archivo[0]=='\\') strcpy (fullpath,nombre_archivo);
	else sprintf (fullpath,"%s/%s",directorio_actual,nombre_archivo);

	debug_printf (VERBOSE_INFO,"ESXDOS handler: fullpath file: %s",fullpath);



	//Abrir el archivo.
	esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix=fopen(fullpath,fopen_mode);


	if (esxdos_fopen_files[free_handle].esxdos_last_open_file_handler_unix==NULL) {
		//esxdos_handler_error_carry(ESXDOS_ERROR_ENOENT);
		debug_printf (VERBOSE_INFO,"ESXDOS handler: Error from esxdos_handler_call_f_open file: %s",fullpath);
		//esxdos_handler_old_return_call();
		return -1;
	}
	else {



		//reg_a=free_handle;
		//esxdos_handler_no_error_uncarry();
		debug_printf (VERBOSE_INFO,"ESXDOS handler: Successfully esxdos_handler_call_f_open file: %s",fullpath);


		if (stat(fullpath, &esxdos_fopen_files[free_handle].last_file_buf_stat)!=0) {
						debug_printf (VERBOSE_INFO,"ESXDOS handler: Unable to get status of file %s",fullpath);
		}


		esxdos_handler_call_f_open_post(free_handle,nombre_archivo,fullpath);


	}

	return free_handle;


}

void load_snx_snapshot(char *archivo)
{
    int antes_sna_setting_no_change_machine=sna_setting_no_change_machine.v;


    load_nex_snapshot_change_to_next();

    //Decimos que no cambiamos a maquina spectrum, ya que forzaremos siempre next
    sna_setting_no_change_machine.v=1;

    load_sna_snapshot_common(archivo);


    sna_setting_no_change_machine.v=antes_sna_setting_no_change_machine;


    load_nex_snapshot_if_mount_exdos_folder(archivo);

/*
".snx is a Spectrum snapshot file, more suitable as emulator compatibility than a real format.
It is identical to a 128K .sna file, but when loaded using the browser or the SPECTRUM command,
NextZXOS leaves file handle 0 open for further use by the program.
The program is expected close the handle before exiting. .snx files may also have private data appended to them.
They are not supported by esxDOS."
*/

    //Si no esta esxdos handler habilitado, avisar y no hacer nada mas
    if (esxdos_handler_enabled.v) {
        load_nex_snapshot_open_esxdos(archivo,1);
        //printf("handler: %d\n",esx_file_handler);
    }



}

void load_nex_snapshot_set_default_next_registers(void)
{
    //revisar todos valores por defecto de
    //https://gitlab.com/thesmog358/tbblue/-/blob/master/src/asm/nexload/nexload.asm?ref_type=heads

    tbblue_registers[18]=9;     // layer2 page
    tbblue_registers[19]=12;    // layer2 shadow page
    tbblue_registers[20]=0xe3;  // transparent index
    tbblue_registers[21]=1;     // priorities + sprite over border + sprite enable

    tbblue_registers[22]=0;     // layer2 xy scroll
    tbblue_registers[23]=0;


    tbblue_registers[45]=0;     // sound drive reset

    tbblue_registers[50]=0;     // lores XY scroll
    tbblue_registers[51]=0;

    tbblue_registers[67]=0;     // ula palette
    tbblue_registers[66]=15;    // allow flashing

    tbblue_registers[74]=0;     // transparency fallback value
    tbblue_registers[75]=0xe3;  // sprite transparency index

}


//Cargar snapshot nex
void load_nex_snapshot(char *archivo)
{

	debug_printf(VERBOSE_DEBUG,"Loading .nex snapshot %s",archivo);





	//buffer para la cabecera
	z80_byte nex_header[NEX_HEADER_SIZE];


        FILE *ptr_nexfile;


        int leidos;

        //Load File
        ptr_nexfile=fopen(archivo,"rb");
        if (ptr_nexfile==NULL) {
                debug_printf(VERBOSE_ERR,"Error opening %s",archivo);
                return;
        }

        leidos=fread(nex_header,1,NEX_HEADER_SIZE,ptr_nexfile);
        if (leidos!=NEX_HEADER_SIZE) {
                        debug_printf(VERBOSE_ERR,"Error reading %d bytes of header",NEX_HEADER_SIZE);
                        return;
        }


        //Ver si signatura correcta
        //Inicialmente la especificacion decia que solo se encontraba "Next" pero por ejemplo el exploding fist dice "NEXT"
        //Por tanto comparamos sin mirar las mayusculas
        char version_string[5];
        version_string[0]=nex_header[0];
        version_string[1]=nex_header[1];
        version_string[2]=nex_header[2];
        version_string[3]=nex_header[3];
        version_string[4]=0;
        if (strcasecmp(version_string,"next")) {
            debug_printf(VERBOSE_ERR,"Unknown NEX signature: 0x%x 0x%x 0x%x 0x%x",nex_header[0],nex_header[1],nex_header[2],nex_header[3]);
            return;
        }


	//cambio a maquina tbblue, siempre
	load_nex_snapshot_change_to_next();


    load_nex_snapshot_if_mount_exdos_folder(archivo);

    load_nex_snapshot_set_default_next_registers();

	//Al cargar .nex lo pone en turbo x 8
	debug_printf(VERBOSE_DEBUG,"Setting turbo x 8 because it's the usual speed when loading .nex files from NextOS");

	z80_byte reg7=tbblue_registers[7];

	reg7 &=(255-3); //Quitar los dos bits bajos

	reg7 |=3;
	//(R/W)	07 => Turbo mode
    /*
    %00 = 3.5MHz
    %01 = 7MHz
    %10 = 14MHz
    %11 = 28MHz (works since core 3.0
    */


	tbblue_registers[7]=reg7;
	tbblue_set_emulator_setting_turbo();


	//desactivamos interrupciones. No esta en el formato pero supongo que es asi
	iff1.v=iff2.v=0;


	//check version. Permitir 1.0, 1.1 y 1.2 y avisar si mayor de 1.2

	char snap_version[5];
	//4	4	string with NEX file version, currently "V1.0", "V1.1" or "V1.2"
	snap_version[0]=nex_header[4];
	snap_version[1]=nex_header[5];
	snap_version[2]=nex_header[6];
	snap_version[3]=nex_header[7];
	snap_version[4]=0;

	//no imprimirlo por si no es una string normal
	//printf ("Snapshot version: %s\n",snap_version);

    //Inicialmente la especificacion decia que la v era mayusculas, pero por ejemplo el exploding fist dice "v"
	if (
		!
		(
		!strcasecmp(snap_version,"v1.0") ||
		!strcasecmp(snap_version,"v1.1") ||
		!strcasecmp(snap_version,"v1.2")
		)
	) {

		debug_printf (VERBOSE_ERR,"Unsupported snapshot version. Loading it anyway");
	}


	//8	1	RAM required: 0 = 768k, 1 = 1792k
	//En ZEsarUX, si activo los 2048 kb, es 1792 KB para el sistema.
	//En ZEsarUX, si activo los 1024 kb, es 768 KB para el sistema.
	//tbblue_extra_512kb_blocks 1 o 3
	if (nex_header[8]) {
		debug_printf(VERBOSE_DEBUG,"Uses 1792 kb");
		tbblue_extra_512kb_blocks=3;
	}
	else {
		debug_printf(VERBOSE_DEBUG,"Uses 768k kb");
		tbblue_extra_512kb_blocks=1;
	}

	//Ofset 8: Number of 16k Banks to Load: 0-112 (see also the byte array at offset 18, which must yield this count)
	//Que sentido tiene si ya hay un array en el offset 18?? Pasamos de esto

	//border
    out_254=nex_header[11] & 7;
    modificado_border.v=1;

	//SP
	reg_sp=value_8_to_16(nex_header[13],nex_header[12]);

	//PC
	z80_int possible_pc=value_8_to_16(nex_header[15],nex_header[14]);

	if (possible_pc!=0) {
		reg_pc=possible_pc;
		debug_printf(VERBOSE_DEBUG,"Register PC: %04XH",reg_pc);
	}

	//modo timex
	set_timex_port_ff(nex_header[138]);
	debug_printf(VERBOSE_DEBUG,"Timex mode: %02XH",timex_port_ff);


	debug_printf(VERBOSE_DEBUG,"Mapping ram %d at C000H",nex_header[139]);
	tbblue_out_port_32765(nex_header[139]);

	//file handler address
	z80_int nex_file_handler=value_8_to_16(nex_header[141],nex_header[140]);
	debug_printf(VERBOSE_DEBUG,"File handler: %d",nex_file_handler);



	int cargar_paleta=0;
	z80_byte load_screen_blocks=nex_header[10];


	// Only Layer2 and Lo-Res screens expect the palette block (unless +128 flag set
	if ( (load_screen_blocks & 1) || (load_screen_blocks & 4) ) {
		cargar_paleta=1;
	}

	if (load_screen_blocks & 128) cargar_paleta=0;

	//Cargar paleta optional palette (for Layer2 or LoRes screen)
	if (cargar_paleta) {
		debug_printf(VERBOSE_DEBUG,"Loading palette");
		leidos=fread(tbblue_palette_layer2_second,1,512,ptr_nexfile);
	}

	//Cargar Layer2 loading screen
	if (load_screen_blocks & 1) {
		debug_printf(VERBOSE_DEBUG,"Loading Layer2 loading screen");
		int tbblue_layer2_offset=tbblue_get_offset_start_layer2();
		leidos=fread(&memoria_spectrum[tbblue_layer2_offset],1,49152,ptr_nexfile);
		//Asumimos que esta activo modo layer2 entonces
        //Esto es necesario en Head Over Heels por ejemplo
		tbblue_out_port_layer2_value(2);

		//tbblue_registers[0x15]=4; //Layer priority L S U
	}

	//classic ULA loading screen
	if (load_screen_blocks & 2) {
		debug_printf(VERBOSE_DEBUG,"Loading classic ULA loading screen");
		z80_byte *pant;
		pant=get_base_mem_pantalla();
		leidos=fread(pant,1,6912,ptr_nexfile);
	}

	//LoRes loading screen
	if (load_screen_blocks & 4) {
		debug_printf(VERBOSE_DEBUG,"Loading LoRes loading screen");
		z80_byte *pant;
		pant=get_lores_pointer(0);
		leidos=fread(pant,1,12288,ptr_nexfile);

		//Asumimos modo lores
		//tbblue_registers[0x15]=128;
	}


	//Timex HiRes (512x192) loading screen
	if (load_screen_blocks & 8) {
		debug_printf(VERBOSE_DEBUG,"Loading Timex HiRes loading screen");
		z80_byte *pant;
		pant=tbblue_ram_memory_pages[5*2];

		//primer bloque
		leidos=fread(pant,1,6144,ptr_nexfile);

		pant +=0x2000;

		//segundo bloque
		leidos=fread(pant,1,6144,ptr_nexfile);
	}

	//Timex HiCol (8x1) loading screen
	if (load_screen_blocks & 16) {
		debug_printf(VERBOSE_DEBUG,"Timex HiCol (8x1) loading screen");
		z80_byte *pant;
		pant=tbblue_ram_memory_pages[5*2];

		//primer bloque
		leidos=fread(pant,1,6144,ptr_nexfile);

		pant +=0x2000;

		//segundo bloque
		leidos=fread(pant,1,6144,ptr_nexfile);
	}

	//TODO: que modo activo de video esta? aparte del timex, no se puede saber si esta lores, o layer2, o ula normal
	//pruebo a activar el modo de la pantalla que carga pero no parece que ningun snapshot tenga una pantalla valida


	//16kiB raw memory bank data in predefined order: 5,2,0,1,3,4,6,7,8,9,10,...,111 (particular bank may be omitted completely)
	//Vamos a cargar los posibles 112 bloques en ram
	//Aunque no vayan a existir esos bloques, los cargamos todos y luego ya vemos
#define NEX_RAM_BLOCKS 112


	//Gestionar los primeros 6 bloques desordenados

	int array_bloques[6]={5,2,0,1,3,4};

	int i;
	for (i=0;i<6;i++) {
		//int offset_bloque;
		int bloque_cargar=array_bloques[i];
		z80_byte esta_presente=nex_header[18+bloque_cargar];
		if (esta_presente) {
			debug_printf(VERBOSE_DEBUG,"Loading ram block %d",bloque_cargar);
			z80_byte *destino=tbblue_ram_memory_pages[bloque_cargar*2];
			leidos=fread(destino,1,16384,ptr_nexfile);
		}
	}

	//Leer el resto de bloques
	for (i=6;i<112;i++) {
		//int offset_bloque;
		int bloque_cargar=i;
		z80_byte esta_presente=nex_header[18+bloque_cargar];
		if (esta_presente) {
			debug_printf(VERBOSE_DEBUG,"Loading ram block %d",bloque_cargar);
			z80_byte *destino=tbblue_ram_memory_pages[bloque_cargar*2];
			leidos=fread(destino,1,16384,ptr_nexfile);
		}
	}

	//Gestionar file handler
	/*
	-.nex format. file handler:
“File handle address: 0 = NEX file is closed by the loader, 1..0x3FFF values (1 recommended) = NEX loader keeps NEX file open and does
pass the file handle in BC register, 0x4000..0xFFFF values (for 0xC000..0xFFFF see also "Entry bank") = NEX loader
keeps NEX file open and the file handle is written into memory at the desired address.”

If the word at offset is 0, I just simply close the file and I don’t do anything else. Set bc=255

If the value is between 1...0x3fff value, I keep the file open and the file handler number is copied to register BC

If the value is between 0x4000 and ffff, I write the file handler number at the address that poitnts this offset 140. Set bc to 255

Y esto hacerlo después de marear toda la ram y cargar los bloques de memoria, lógicamente

-Parámetro config de tipo background ZX desktop. Más tipos?
y parámetro de color del tipo de fondo sólido
	 */

	//por defecto hacemos que registro bc=255, error
	BC=255;

	if (nex_file_handler>0) {


		debug_printf(VERBOSE_DEBUG,"Uses NextOS file handler");

		//De momento asumimos error y escribimos 255 en la memoria tambien (en caso que nex_file_handler esta entre 0x4000 y ffff)
		//Asi le daremos un file handler erroneo si pasase cualquier cosa
		if (nex_file_handler>=0x4000 && nex_file_handler<0xffff) {
			poke_byte_no_time(nex_file_handler,255);
		}

		//Si no esta esxdos handler habilitado, avisar y no hacer nada mas
		if (esxdos_handler_enabled.v) {
			//Obtener offset actual sobre archivo snapshot abierto
			long initial_offset=ftell(ptr_nexfile);
			debug_printf(VERBOSE_DEBUG,"Current offset of .nex file after loading it: %ld",initial_offset);

			//Abrir este mismo archivo desde esxdos handler. Luego hacer seek hasta el offset que tenemos

			//Cerrarlo antes, por problemas de bloqueo en sistema operativo al tenerlo dos veces (en windows?)
			fclose(ptr_nexfile);


			int esx_file_handler=load_nex_snapshot_open_esxdos(archivo,0);
			debug_printf(VERBOSE_DEBUG,"file handle of esxdos open file: %d",esx_file_handler);

			if (esx_file_handler>=0) {
				//Hacer fseek
				if (fseek (esxdos_fopen_files[esx_file_handler].esxdos_last_open_file_handler_unix, initial_offset, SEEK_CUR)!=0) {
					debug_printf (VERBOSE_ERR,"ESXDOS handler: Error running fseek system call");
				}

				//Retornar BCDE
				long cur_offset=ftell(esxdos_fopen_files[esx_file_handler].esxdos_last_open_file_handler_unix);

				debug_printf(VERBOSE_DEBUG,"ESXDOS handler: offset is now at %ld",cur_offset);


				//If the value is between 1...0x3fff value, I keep the file open and the file handler number is copied to register BC

				//If the value is between 0x4000 and ffff, I write the file handler number at the address that poitnts this offset 140. Set bc to 255
				if (nex_file_handler<=0x3fff) {
					BC=esx_file_handler;
					debug_printf(VERBOSE_DEBUG,"Setting BC register to value %04XH",BC);
				}
				else {
					//copiar handler en memoria
					debug_printf(VERBOSE_DEBUG,"Writing file handler to memory, address %04XH",nex_file_handler);
					poke_byte_no_time(nex_file_handler,esx_file_handler);
				}

			}

		}
		else {
			debug_printf (VERBOSE_ERR,"Snapshot uses NextOS file handler. You should enable esxdos handler before loading it");
			fclose(ptr_nexfile);
		}
	}
	else {
		fclose(ptr_nexfile);
	}

}
