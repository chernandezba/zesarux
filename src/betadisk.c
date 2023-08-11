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

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "betadisk.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "mem128.h"
#include "screen.h"
#include "tsconf.h"
#include "tape.h"
#include "menu_items.h"


z80_bit betadisk_enabled={0};


z80_byte *betadisk_memory_pointer;

z80_bit betadisk_active={0};

z80_bit betadisk_allow_boot_48k={1};


//Motor simulado de betadisk
int betadisk_simulated_motor=0;

int betadisk_nested_id_core;
int betadisk_nested_id_peek_byte;
int betadisk_nested_id_peek_byte_no_time;


char trd_file_name[PATH_MAX]="";

char *betadisk_rom_file_name="trdos.rom";

z80_bit trd_enabled={0};

void betadisk_trdoshandler_read_write_sectors(void);

int trd_must_flush_to_disk=0;

z80_bit trd_write_protection={0};


//Si cambios en escritura se hace flush a disco
z80_bit trd_persistent_writes={1};

//http://problemkaputt.de/zxdocs.htm#spectrumdiscbetabetaplusbeta128diskinterfacetrdos

void betadisk_format_disk(void);

int betadisk_check_if_rom_area(z80_int dir)
{

	//Si no betadisk activo, volver
	if (betadisk_active.v==0) return 0;

	//Si maquina 128k y es rom 0, volver
	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		if (!(puerto_32765&16)) return 0;
	}

	if (MACHINE_IS_TSCONF) {
		//Si no es rom 3, volver
		if (tsconf_get_rom_bank()!=3) return 0; //Esto puede fallar si se gestiona "dos_signal" desde esa funcion de tsconf
		//en ese caso retornaria la rom de trdos (1) y no la 3, y aqui no saltaria nunca
		//if (tsconf_af_ports[0x10]!=3) return 0;
	}

	if (dir<16384) return 1;
	else return 0;
}

z80_byte betadisk_read_byte(z80_int dir)
{
	//printf ("Returning betadisk data address %d\n",dir);
	if (MACHINE_IS_TSCONF) {
		z80_byte *puntero=tsconf_rom_mem_table[1]; //Rom 1 es la de tr-dos
		//printf ("retornando byte de tr-dos rom dir %04XH\n",dir);
		return puntero[dir];
	}
	return betadisk_memory_pointer[dir];
}


z80_byte betadisk_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(betadisk_nested_id_peek_byte,dir);

	if (betadisk_check_if_rom_area(dir)) {
       		//t_estados +=3;
		return betadisk_read_byte(dir);
	}

	//return betadisk_original_peek_byte(dir);
	return valor_leido;
}

z80_byte betadisk_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(betadisk_nested_id_peek_byte_no_time,dir);

	if (betadisk_check_if_rom_area(dir)) {
                return betadisk_read_byte(dir);
        }

	//else return debug_nested_peek_byte_no_time_call_previous(betadisk_nested_id_peek_byte_no_time,dir);
	return valor_leido;
}



//Establecer rutinas propias
void betadisk_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting betadisk poke / peek functions");

	//Asignar mediante funciones de core anidados
	betadisk_nested_id_peek_byte=debug_nested_peek_byte_add(betadisk_peek_byte,"Betadisk peek_byte");
	betadisk_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(betadisk_peek_byte_no_time,"Betadisk peek_byte_no_time");

}

//Restaurar rutinas de betadisk
void betadisk_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before betadisk");


	debug_nested_peek_byte_del(betadisk_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(betadisk_nested_id_peek_byte_no_time);
}

void betadisk_cambio_pagina(z80_int dir)
{



	if (betadisk_active.v) {
		if (dir>=0x4000) {
			//printf ("Unactivating betadisk rom space on PC=%04XH\n",reg_pc);
			betadisk_active.v=0;
		}
	}

	else {
		if (dir>=0x3C00 && dir<=0x3DFF) {
			//printf ("Activating betadisk rom space on PC=%04XH\n",reg_pc);
			betadisk_active.v=1;
		}
	}
}

void betadisk_reset(void)
{

	betadisk_active.v=0;

	//Al hacer reset, se activa betadisk en maquinas 48k

	if (MACHINE_IS_SPECTRUM_16_48 && betadisk_allow_boot_48k.v) {
		betadisk_active.v=1;
	}
}

void betadisk_show_activity(void)
{

	generic_footertext_print_operating("TRD");

	//Y poner icono en inverso
	if (!zxdesktop_icon_betadisk_inverse) {
			zxdesktop_icon_betadisk_inverse=1;
			menu_draw_ext_desktop();
	}
}

z80_byte cpu_core_loop_betadisk(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{
	//Llamar a anterior
	debug_nested_core_call_previous(betadisk_nested_id_core);


	//http://problemkaputt.de/zxdocs.htm#spectrumdiscbetabetaplusbeta128diskinterfacetrdos

	/*
	MEM:3C00h..3CFFh - Enable ROM & I/O Ports (Beta/TRDOSv1 and BetaPlus/TRDOSv4)
	MEM:3D00h..3DFFh - Enable ROM & I/O Ports (Beta128/TRDOSv5.0x)
	The enable region was originally at 3Cxxh (which is FFh-filled in Spectrum 16K/48K/Plus BIOSes), and was invoked via RANDOMIZE USR 15360 and 15363 from BASIC. For compatibility with the Spectrum 128 (which contains BIOS code in that region), the region was moved to 3Dxxh (which contains only character set data, but no program code) and is now invoked via USR 15616 and 15619. The first address is used to enter TRDOS,
  	RANDOMIZE USR 15360 or 15616  --> switch from BASIC to TRDOS ;3C00h/3D00h
  	RETURN (aka Y-key)            --> switch from TRDOS to BASIC
	The second address is used as prefix to normal BASIC commands, eg.:
  	RANDOMIZE USR 15363 or 15619:REM:LOAD"filename"              ;3C03h/3D03h

  	MEM:4000h..FFFFh - Disable ROM & I/O Ports (all versions)
	Opcode fetches outside of the ROM region (ie. at 4000h..FFFFh) do automatically disable the TRDOS ROM and I/O ports.

	*/

	betadisk_cambio_pagina(reg_pc);


	//http://www.worldofspectrum.org/pub/sinclair/hardware-info/TR-DOS_Programming.txt

	//Debug
	//if (betadisk_check_if_rom_area(reg_pc) ) {
		//if (reg_pc==0x3D13 || reg_pc==0x3C13) {
		//	printf ("TR-DOS Call function %02XH on addr %04XH\n",reg_c,reg_pc);
		//}
	//}


	//Handler
	if (trd_enabled.v) {
		if (betadisk_check_if_rom_area(reg_pc) ) {
			/*if (reg_pc==0x1e3d) {
				char buffer_registros[8192];
				print_registers(buffer_registros);
				printf ("Handler for read sectors\n");
				printf ("%s\n",buffer_registros);
				betadisk_trdoshandler_read_write_sectors();
			}*/


			//Si A=0, lectura. Si A=255, escritura
			/*
			Trapping 1e67H:
			transfer_B_sectors_first_sector_DE_address_HL:
transfer_sectors:
  ld (trdos_variable.sector_rw_flag),a
l1e67h:
  ld (trdos_variable.current_sector),de
  push bc
  push hl
  call sub_1e36h

			*/

/*
			if (reg_pc==15635) {
				if (reg_c==5) {

// #05 ³Read sectors. HL=memory address, DE=start sector/trackº
//      ³B=number of sectors

	z80_byte numero_sectores=reg_b;
	z80_byte pista=reg_d;
	z80_byte sector=reg_e;

	z80_int destino=reg_hl;

	 debug_printf (VERBOSE_DEBUG,"From 15635 Reading %d sectors from track %d sector %d to address %04XH",numero_sectores,pista,sector,destino);



				}

				if (reg_c!=5 && reg_c!=6) {
					debug_printf (VERBOSE_DEBUG,"From 15635  entry point not read nor write: %d",reg_c);
				}

			}


// 			read_sectors:                                    equ 0x1E3D
// write_sectors:                                           equ 0x1E4D

// B = número de sectores
// D = pista del primer sector a usar (0..159)
// E = primer sector a usar de la pista (0..15)
// HL = dirección de memoria para carga o lectura de los sectores
//

			if (reg_pc==0x1e3d || reg_pc==0x1e4d) {
	z80_byte numero_sectores=reg_b;
	z80_byte pista=reg_d;
	z80_byte sector=reg_e;

	z80_int destino=reg_hl;

	 debug_printf (VERBOSE_DEBUG,"From %04XH %s %d sectors from track %d sector %d  address %04XH",
	 reg_pc,
	 (reg_pc==0x1e3d ? "Reading" : "Writing" ),

	 numero_sectores,pista,sector,destino);

			}
*/
			if (reg_pc==0x1e67) {
                //printf("PC on addr 0x1e67\n");
				if (reg_a==0 || reg_a==255) {
					//char buffer_registros[8192];
					//print_registers(buffer_registros);
					//printf ("\n\nHandler for transfer_sectors\n");
					//printf ("%s\n",buffer_registros);
					betadisk_show_activity();
					betadisk_trdoshandler_read_write_sectors();
				}
			}


			if (reg_pc==0x1ee8) {
				//format
				debug_printf(VERBOSE_DEBUG,"Formating trd disk");
				betadisk_show_activity();
				betadisk_format_disk();
			}

		}
	}



	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;

}




void betadisk_set_core_function(void)
{
	debug_printf (VERBOSE_DEBUG,"Setting betadisk Core loop");
	//Asignar mediante nuevas funciones de core anidados
	betadisk_nested_id_core=debug_nested_core_add(cpu_core_loop_betadisk,"Betadisk core");
}




void betadisk_restore_core_function(void)
{
        debug_printf (VERBOSE_DEBUG,"Restoring original betadisk core");
	debug_nested_core_del(betadisk_nested_id_core);
}



void betadisk_alloc_memory(void)
{
        int size=BETADISK_SIZE;

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for betadisk emulation",size/1024);

        betadisk_memory_pointer=malloc(size);
        if (betadisk_memory_pointer==NULL) {
                cpu_panic ("No enough memory for betadisk emulation");
        }


}

int betadisk_load_rom(void)
{
	//si es tsconf, tiene rom propia
	if (MACHINE_IS_TSCONF) return 0;

        FILE *ptr_betadisk_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading betadisk rom %s",BETADISK_ROM_FILENAME);

        open_sharedfile(BETADISK_ROM_FILENAME,&ptr_betadisk_romfile);

  			//ptr_betadisk_romfile=fopen(betadisk_rom_file_name,"rb");
        if (!ptr_betadisk_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
        }

        if (ptr_betadisk_romfile!=NULL) {

                leidos=fread(betadisk_memory_pointer,1,BETADISK_SIZE,ptr_betadisk_romfile);
                fclose(ptr_betadisk_romfile);

        }



        if (leidos!=BETADISK_SIZE || ptr_betadisk_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading betadisk rom");
                return 1;
        }

        return 0;


}


z80_byte *trd_memory_pointer;

int betadisk_bytes_por_sector=256;
int betadisk_sectores_por_pista=16;


void trd_insert(void)
{

        //Si existe
        if (!si_existe_archivo(trd_file_name)) {
                debug_printf (VERBOSE_ERR,"File %s does not exist",trd_file_name);
                trd_disable();
                return;
        }


	trd_memory_pointer=malloc(TRD_FILE_SIZE);


        if (trd_memory_pointer==NULL) {
                cpu_panic ("No enough memory for TRD emulation");
        }

        FILE *ptr_trdfile;
	debug_printf (VERBOSE_INFO,"Opening TRD File %s",trd_file_name);
	ptr_trdfile=fopen(trd_file_name,"rb");

        if (!ptr_trdfile) {
                debug_printf (VERBOSE_ERR,"Unable to open trd file");
                trd_disable();
                return;
        }

        int leidos=fread(trd_memory_pointer,1,TRD_FILE_SIZE,ptr_trdfile);

	//TODO gestion de tamanyos validos
	/*
	- Logical sectors per track are 16
	- Sector dimension is 256 bytes
	*/
	//Minimo 1 pista, 16 sectores
	int minsize=16*256;
	if (leidos<minsize) {
		debug_printf (VERBOSE_ERR,"Error reading trd file");
		trd_disable();
	}

        fclose(ptr_trdfile);



}

//Retorna -1 si error
int betadisk_get_offset_tracksectorbyte(int pista, int sector, int byte_en_sector)
{
        int bytes_por_pista=betadisk_sectores_por_pista*betadisk_bytes_por_sector;
        int offset=pista*bytes_por_pista+sector*betadisk_bytes_por_sector+byte_en_sector;

        if (offset>=TRD_FILE_SIZE) {
                debug_printf (VERBOSE_ERR,"Error. Trying to access beyond trd. Size: %ld Asked: %u. Disabling TRD",TRD_FILE_SIZE,offset);
                trd_disable();
                return -1;
        }

	return offset;
}

z80_byte betadisk_get_byte_disk(int pista, int sector, int byte_en_sector)
{

	int offset=betadisk_get_offset_tracksectorbyte(pista,sector,byte_en_sector);
	if (offset<0) return 0;

	z80_byte byte_leido=trd_memory_pointer[offset];

	return byte_leido;
}


void betadisk_put_byte_disk(int pista, int sector, int byte_en_sector,z80_byte byte_a_escribir)
{


	int offset=betadisk_get_offset_tracksectorbyte(pista,sector,byte_en_sector);
        if (offset<0) return;

        if (trd_write_protection.v) return;

	trd_memory_pointer[offset]=byte_a_escribir;
	trd_must_flush_to_disk=1;

}

void betadisk_format_disk(void)
{

	if (trd_write_protection.v) return;

	int offset=0;
	for (offset=0;offset<TRD_FILE_SIZE;offset++) {
		trd_memory_pointer[offset]=0;
	}

	trd_must_flush_to_disk=1;

}

void betadisk_trdoshandler_read_write_sectors(void)
{
	/*


read_sectors:                                               equ 0x1E3D
write_sectors:                                              equ 0x1E4D

B = número de sectores
D = pista del primer sector a usar (0..159)
E = primer sector a usar de la pista (0..15)
HL = dirección de memoria para carga o lectura de los sectores


A=0 read, A=255 write

	#05 - Read group of sectors. In HL must be  loaded address where  sector data
       should  be readed, B must be  loaded with  number of sectors to read, D
       must be loaded  with track number and E with sector number. If B loaded
       with  #00 then interface will only read sector address mark - useful if
       you only want check is there sector with defined number on the track.

	*/

	//z80_byte numero_sectores=reg_a;
	z80_byte numero_sectores=reg_b;
	z80_byte pista=reg_d;
	z80_byte sector=reg_e;
	int byte_en_sector;
	z80_int destino=reg_hl;

	if (reg_a==0) {
        debug_printf (VERBOSE_DEBUG,"Reading %d sectors from track %d sector %d to address %04XH",numero_sectores,pista,sector,destino);
        //printf ("Reading %d sectors from track %d sector %d to address %04XH\n",numero_sectores,pista,sector,destino);
    }
	if (reg_a==255) {
        debug_printf (VERBOSE_DEBUG,"Writing %d sectors to track %d sector %d from address %04XH",numero_sectores,pista,sector,destino);
        //printf ("Writing %d sectors to track %d sector %d from address %04XH\n",numero_sectores,pista,sector,destino);
    }


		//poke_byte_no_time(TRD_SYM_trdos_variable_sector_rw_flag,reg_a);






	int leidos=0;

	for (;numero_sectores>0;numero_sectores--) {
		for (byte_en_sector=0;byte_en_sector<betadisk_bytes_por_sector;byte_en_sector++) {

            //Notificar visualfloppy. Visualfloppy basado en 9 sectores de 512 bytes. mientras que betadisk:
            //betadisk_bytes_por_sector=256;
            //betadisk_sectores_por_pista=16;
            int visualfloppy_sector=sector/2;
            int visualfloppy_byte_en_sector=byte_en_sector*2;
            menu_visual_floppy_buffer_add(pista,visualfloppy_sector,visualfloppy_byte_en_sector);
            menu_visual_floppy_buffer_add(pista,visualfloppy_sector,visualfloppy_byte_en_sector+1);

            //Y simulamos "motor on" de betadisk
            //3 segundos
            betadisk_simulated_motor=3;

			if (reg_a==0) {
			z80_byte byte_leido=betadisk_get_byte_disk(pista,sector,byte_en_sector);
			poke_byte_no_time(destino,byte_leido);
			}

			if (reg_a==255) {
			z80_byte byte_leido=peek_byte_no_time(destino);
			betadisk_put_byte_disk(pista,sector,byte_en_sector,byte_leido);
			}

			destino++;

			leidos++;
		}
		sector++;
	}

	//printf ("\ntotal leidos: %d\n",leidos);

	//No error
	reg_a=0;
	Z80_FLAGS |=FLAG_Z;

	//??
	reg_hl=destino;
	reg_e=sector;
	reg_d=pista;

/*
   23807 | 1* | Sector number for sector read/write Tr-Dos functions
   23808 | 2* | Current address of buffer for Tr-Dos #05 and #06 functions
*/
	//23807 sector number
	//poke_byte_no_time(23807,sector);
	//poke_byte_no_time(23808,reg_l);
	//poke_byte_no_time(23809,reg_h);


	poke_byte_no_time(TRD_SYM_trdos_variable_current_sector,reg_e);
	poke_byte_no_time(TRD_SYM_trdos_variable_current_track,reg_d);



	//Return
	reg_pc=pop_valor();
}








void betadisk_enable(void)
{

  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable betadisk on non Spectrum machine");
    return;
  }

	if (betadisk_enabled.v) return;


	betadisk_alloc_memory();
	if (betadisk_load_rom()) return;

	betadisk_set_peek_poke_functions();

	betadisk_set_core_function();

	betadisk_active.v=0;


	betadisk_enabled.v=1;


}

void betadisk_disable(void)
{
	if (betadisk_enabled.v==0) return;

	betadisk_restore_peek_poke_functions();

	free(betadisk_memory_pointer);

	betadisk_restore_core_function();

	betadisk_enabled.v=0;
}






void trd_enable(void)
{

	if (trd_enabled.v) return;

        debug_printf (VERBOSE_INFO,"Enabling trd");
        trd_enabled.v=1;

        trd_insert();


}

void trd_disable(void)
{
	if (trd_enabled.v==0) return;


        debug_printf (VERBOSE_INFO,"Disabling trd");
        trd_enabled.v=0;
	free(trd_memory_pointer);


}



//TODO: siempre se hace flush de disco de 640 kb. Se deberia poder hacer flush de menos, para floppy mas pequenyo?
void trd_flush_contents_to_disk(void)
{

        if (trd_enabled.v==0) return;

        if (trd_must_flush_to_disk==0) {
                debug_printf (VERBOSE_DEBUG,"Trying to flush TRD to disk but no changes made");
                return;
        }


        if (trd_persistent_writes.v==0) {
                debug_printf (VERBOSE_DEBUG,"Trying to flush TRD to disk but persistent writes disabled");
                return;
        }



        debug_printf (VERBOSE_INFO,"Flushing TRD to disk");


        FILE *ptr_trdfile;

        debug_printf (VERBOSE_INFO,"Opening TRD File %s",trd_file_name);
        ptr_trdfile=fopen(trd_file_name,"wb");



        int escritos=0;
        long long int size;
        size=TRD_FILE_SIZE;


        if (ptr_trdfile!=NULL) {
                z80_byte *puntero;
                puntero=trd_memory_pointer;

                //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
                //metera flush a 1
                trd_must_flush_to_disk=0;

                escritos=fwrite(puntero,1,size,ptr_trdfile);

                fclose(ptr_trdfile);


        }

        //printf ("ptr_trdfile: %d\n",ptr_trdfile);
        //printf ("escritos: %d\n",escritos);

        if (escritos!=size || ptr_trdfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error writing to TRD file. Disabling write file operations");
                trd_persistent_writes.v=0;
        }

}

void trd_insert_disk(char *nombre)
{

  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable insert trd betadisk on non Spectrum machine");
    return;
  }

	if (noautoload.v==0) {
		reset_cpu();
	}

	//Quitar si había alguno
	trd_disable();

	strcpy(trd_file_name,nombre);
    trd_enable();
}
