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

Defcon emulation
https://speccy4ever.speccy.org/_OC.htm

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "defcon.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"


z80_bit defcon_enabled={0};


z80_byte *defcon_memory_pointer;


int defcon_nested_id_peek_byte;
int defcon_nested_id_peek_byte_no_time;

z80_bit defcon_mapped_rom_memory={0};

//Nota: que yo sepa, no tiene manera de desmapearse una vez se ha activado

z80_byte defcon_read_rom_byte(z80_int dir)
{
	return defcon_memory_pointer[dir];
}

int defcon_check_if_rom_area(z80_int dir)
{
    if (dir<16384 && defcon_mapped_rom_memory.v) return 1;
    else return 0;
}

z80_byte defcon_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(defcon_nested_id_peek_byte,dir);

	if (defcon_check_if_rom_area(dir)) {
		return defcon_read_rom_byte(dir);
	}


	return valor_leido;
}

z80_byte defcon_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(defcon_nested_id_peek_byte_no_time,dir);

	if (defcon_check_if_rom_area(dir)) {
		return defcon_read_rom_byte(dir);
	}

	return valor_leido;
}



void defcon_nmi(void)
{
    if (defcon_mapped_rom_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling defcon memory from nmi triggered");
        defcon_mapped_rom_memory.v=1;
    }   
}




//Establecer rutinas propias. Solo tiene rom por tanto peek y no poke
void defcon_set_peek_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting defcon peek functions");

	defcon_nested_id_peek_byte=debug_nested_peek_byte_add(defcon_peek_byte,"defcon peek_byte");
	defcon_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(defcon_peek_byte_no_time,"defcon peek_byte_no_time");


}

//Restaurar rutinas de defcon
void defcon_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original peek functions before defcon");


	debug_nested_peek_byte_del(defcon_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(defcon_nested_id_peek_byte_no_time);



}



int defcon_load_rom(void)
{

    FILE *ptr_defcon_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading defcon rom %s",DEFCON_ROM);

    open_sharedfile(DEFCON_ROM,&ptr_defcon_romfile);
    if (!ptr_defcon_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_defcon_romfile!=NULL) {

        leidos=fread(defcon_memory_pointer,1,DEFCON_ROM_SIZE,ptr_defcon_romfile);
        fclose(ptr_defcon_romfile);
    }



    if (leidos!=DEFCON_ROM_SIZE || ptr_defcon_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading Defcon rom file: %s",DEFCON_ROM);
        return 1;
    }

    return 0;
}

void defcon_alloc_memory(void)
{

    int size=DEFCON_ROM_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for defcon emulation",size/1024);

    defcon_memory_pointer=util_malloc(size,"Can not allocate memory for defcon emulation");


}

void defcon_reset(void)
{

    if (defcon_enabled.v==0) {
        return;
    }

    defcon_mapped_rom_memory.v=0;

}

void defcon_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable defcon on non Spectrum machine");
        return;
    }

	if (defcon_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"defcon Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling defcon interface");

	defcon_alloc_memory();


	if (defcon_load_rom()) return;

	defcon_set_peek_functions();

	defcon_enabled.v=1;


}

void defcon_disable(void)
{
	if (defcon_enabled.v==0) return;


	defcon_restore_peek_poke_functions();

	free(defcon_memory_pointer);


	defcon_enabled.v=0;
}

