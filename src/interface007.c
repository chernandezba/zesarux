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

Interface007 emulation

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "interface007.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"


z80_bit interface007_enabled={0};


z80_byte *interface007_memory_pointer;


int interface007_nested_id_peek_byte;
int interface007_nested_id_peek_byte_no_time;

z80_bit interface007_mapped_rom_memory={0};


z80_byte interface007_read_rom_byte(z80_int dir)
{
	return interface007_memory_pointer[dir];
}

int interface007_check_if_rom_area(z80_int dir)
{
    if (dir<2048 && interface007_mapped_rom_memory.v) return 1;
    else return 0;
}

z80_byte interface007_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(interface007_nested_id_peek_byte,dir);

	if (interface007_check_if_rom_area(dir)) {
		return interface007_read_rom_byte(dir);
	}


	return valor_leido;
}

z80_byte interface007_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(interface007_nested_id_peek_byte_no_time,dir);

	if (interface007_check_if_rom_area(dir)) {
		return interface007_read_rom_byte(dir);
	}

	return valor_leido;
}



void interface007_nmi(void)
{
    if (interface007_mapped_rom_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling interface007 memory from nmi triggered");
        interface007_mapped_rom_memory.v=1;
    }   
}




//Establecer rutinas propias. Solo tiene rom por tanto peek y no poke
void interface007_set_peek_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting interface007 peek functions");

	//Asignar mediante nuevas funciones de core anidados
	interface007_nested_id_peek_byte=debug_nested_peek_byte_add(interface007_peek_byte,"interface007 peek_byte");
	interface007_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(interface007_peek_byte_no_time,"interface007 peek_byte_no_time");



}

//Restaurar rutinas de interface007
void interface007_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original peek functions before interface007");


	debug_nested_peek_byte_del(interface007_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(interface007_nested_id_peek_byte_no_time);


}



int interface007_load_rom(void)
{

    FILE *ptr_interface007_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading interface007 rom %s",INTERFACE007_ROM);

    open_sharedfile(INTERFACE007_ROM,&ptr_interface007_romfile);
    if (!ptr_interface007_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_interface007_romfile!=NULL) {

        leidos=fread(interface007_memory_pointer,1,INTERFACE007_ROM_SIZE,ptr_interface007_romfile);
        fclose(ptr_interface007_romfile);
    }



    if (leidos!=INTERFACE007_ROM_SIZE || ptr_interface007_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading Interface007 rom file: %s",INTERFACE007_ROM);
        return 1;
    }

    return 0;
}

void interface007_alloc_memory(void)
{

    int size=INTERFACE007_ROM_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for interface007 emulation",size/1024);

    interface007_memory_pointer=util_malloc(size,"Can not allocate memory for interface007 emulation");


}

void interface007_reset(void)
{

    if (interface007_enabled.v==0) {
        return;
    }

    interface007_mapped_rom_memory.v=0;

}

void interface007_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable interface007 on non Spectrum machine");
        return;
    }

	if (interface007_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"interface007 Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling interface007 interface");

	interface007_alloc_memory();


	if (interface007_load_rom()) return;

	interface007_set_peek_functions();

	interface007_enabled.v=1;


}

void interface007_disable(void)
{
	if (interface007_enabled.v==0) return;


	interface007_restore_peek_poke_functions();

	free(interface007_memory_pointer);


	interface007_enabled.v=0;
}


