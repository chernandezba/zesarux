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

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "kartusho.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"


z80_bit kartusho_enabled={0};

z80_bit kartusho_protected={0};

char kartusho_rom_file_name[PATH_MAX]="";

z80_byte *kartusho_memory_pointer;



int kartusho_nested_id_poke_byte;
int kartusho_nested_id_poke_byte_no_time;
int kartusho_nested_id_peek_byte;
int kartusho_nested_id_peek_byte_no_time;

//Banco activo. 0..31
z80_byte kartusho_active_bank=0;


int kartusho_check_if_rom_area(z80_int dir)
{
                if (dir<16384) {
			return 1;
                }
	return 0;
}

z80_byte kartusho_read_byte(z80_int dir)
{
	//Si no, memoria kartusho
	int puntero=kartusho_active_bank*16384+dir;
	return kartusho_memory_pointer[puntero];
}


void kartusho_handle_special_dirs(z80_int dir)
{

	if (kartusho_protected.v) return;

	if (dir>=0x3FFC && dir<=0x3FFF) {
		//Valor bit A0
		z80_byte value_a0=dir&1;

		//Mover banco * 2
		kartusho_active_bank = kartusho_active_bank << 1;

		kartusho_active_bank |=value_a0;

		//Mascara final
		kartusho_active_bank=kartusho_active_bank&31;



		//Si se habilita proteccion
		if (dir&2) kartusho_protected.v=1;
	}

}


z80_byte kartusho_poke_byte(z80_int dir,z80_byte valor)
{

	//kartusho_original_poke_byte(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_call_previous(kartusho_nested_id_poke_byte,dir,valor);

	kartusho_handle_special_dirs(dir);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte kartusho_poke_byte_no_time(z80_int dir,z80_byte valor)
{
        //kartusho_original_poke_byte_no_time(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_no_time_call_previous(kartusho_nested_id_poke_byte_no_time,dir,valor);


	kartusho_handle_special_dirs(dir);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte kartusho_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(kartusho_nested_id_peek_byte,dir);

	kartusho_handle_special_dirs(dir);

	if (kartusho_check_if_rom_area(dir)) {
		return kartusho_read_byte(dir);
	}

	//return kartusho_original_peek_byte(dir);
	return valor_leido;
}

z80_byte kartusho_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(kartusho_nested_id_peek_byte_no_time,dir);

	kartusho_handle_special_dirs(dir);

	if (kartusho_check_if_rom_area(dir)) {
                return kartusho_read_byte(dir);
        }

	return valor_leido;
}



//Establecer rutinas propias
void kartusho_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting kartusho poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	kartusho_nested_id_poke_byte=debug_nested_poke_byte_add(kartusho_poke_byte,"Kartusho poke_byte");
	kartusho_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(kartusho_poke_byte_no_time,"Kartusho poke_byte_no_time");
	kartusho_nested_id_peek_byte=debug_nested_peek_byte_add(kartusho_peek_byte,"Kartusho peek_byte");
	kartusho_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(kartusho_peek_byte_no_time,"Kartusho peek_byte_no_time");

}

//Restaurar rutinas de kartusho
void kartusho_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before kartusho");


	debug_nested_poke_byte_del(kartusho_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(kartusho_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(kartusho_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(kartusho_nested_id_peek_byte_no_time);
}



void kartusho_alloc_memory(void)
{
        int size=KARTUSHO_SIZE;  

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for kartusho emulation",size/1024);

        kartusho_memory_pointer=malloc(size);
        if (kartusho_memory_pointer==NULL) {
                cpu_panic ("No enough memory for kartusho emulation");
        }


}

int kartusho_load_rom(void)
{

        FILE *ptr_kartusho_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading kartusho rom %s",kartusho_rom_file_name);

  			ptr_kartusho_romfile=fopen(kartusho_rom_file_name,"rb");
                if (!ptr_kartusho_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
                }

        if (ptr_kartusho_romfile!=NULL) {

                leidos=fread(kartusho_memory_pointer,1,KARTUSHO_SIZE,ptr_kartusho_romfile);
                fclose(ptr_kartusho_romfile);

        }



        if (leidos!=KARTUSHO_SIZE || ptr_kartusho_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading kartusho rom");
                return 1;
        }

        return 0;
}



void kartusho_enable(void)
{

  if (!MACHINE_IS_SPECTRUM && !MACHINE_IS_CPC) {
    debug_printf(VERBOSE_INFO,"Can not enable kartusho on non Spectrum or CPC machine");
    return;
  }

	if (kartusho_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"Already enabled");
		return;
	}

	if (kartusho_rom_file_name[0]==0) {
		debug_printf (VERBOSE_ERR,"Trying to enable Kartusho but no ROM file selected");
		return;
	}

	kartusho_alloc_memory();
	if (kartusho_load_rom()) return;

	kartusho_set_peek_poke_functions();

	kartusho_enabled.v=1;

	kartusho_press_button();

	//kartusho_active_bank=0;
	//kartusho_protected.v=0;
	



}

void kartusho_disable(void)
{
	if (kartusho_enabled.v==0) return;

	kartusho_restore_peek_poke_functions();

	free(kartusho_memory_pointer);

	kartusho_enabled.v=0;
}


void kartusho_press_button(void)
{

        if (kartusho_enabled.v==0) {
                debug_printf (VERBOSE_ERR,"Trying to press Kartusho button when it is disabled");
                return;
        }

	kartusho_active_bank=0;
	kartusho_protected.v=0;

	reset_cpu();


}
