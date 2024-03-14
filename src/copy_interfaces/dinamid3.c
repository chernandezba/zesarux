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

Dinamid3 emulation

*/

//Parece que dinamid3, interface007 y phoenix comparten código rom y debieron originarse de un mismo sitio
//https://foro.speccy.org/viewtopic.php?t=4997
//https://www.va-de-retro.com/foros/viewtopic.php?t=693&start=40

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "dinamid3.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"


z80_bit dinamid3_enabled={0};


z80_byte *dinamid3_memory_pointer;


int dinamid3_nested_id_peek_byte;
int dinamid3_nested_id_peek_byte_no_time;

z80_bit dinamid3_mapped_rom_memory={0};


z80_byte dinamid3_read_rom_byte(z80_int dir)
{
	return dinamid3_memory_pointer[dir];
}

int dinamid3_check_if_rom_area(z80_int dir)
{
    if (dir<2048 && dinamid3_mapped_rom_memory.v) return 1;
    else return 0;
}

z80_byte dinamid3_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(dinamid3_nested_id_peek_byte,dir);

	if (dinamid3_check_if_rom_area(dir)) {
		return dinamid3_read_rom_byte(dir);
	}


	return valor_leido;
}

z80_byte dinamid3_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(dinamid3_nested_id_peek_byte_no_time,dir);

	if (dinamid3_check_if_rom_area(dir)) {
		return dinamid3_read_rom_byte(dir);
	}

	return valor_leido;
}



void dinamid3_nmi(void)
{
    if (dinamid3_mapped_rom_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling dinamid3 memory from nmi triggered");
        dinamid3_mapped_rom_memory.v=1;
    }
}




//Establecer rutinas propias. Solo tiene rom por tanto peek y no poke
void dinamid3_set_peek_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting dinamid3 peek functions");

	//Asignar mediante nuevas funciones de core anidados
	dinamid3_nested_id_peek_byte=debug_nested_peek_byte_add(dinamid3_peek_byte,"dinamid3 peek_byte");
	dinamid3_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(dinamid3_peek_byte_no_time,"dinamid3 peek_byte_no_time");



}

//Restaurar rutinas de dinamid3
void dinamid3_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original peek functions before dinamid3");


	debug_nested_peek_byte_del(dinamid3_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(dinamid3_nested_id_peek_byte_no_time);


}



int dinamid3_load_rom(void)
{

    FILE *ptr_dinamid3_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading dinamid3 rom %s",DINAMID3_ROM);

    open_sharedfile(DINAMID3_ROM,&ptr_dinamid3_romfile);
    if (!ptr_dinamid3_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_dinamid3_romfile!=NULL) {

        leidos=fread(dinamid3_memory_pointer,1,DINAMID3_ROM_SIZE,ptr_dinamid3_romfile);
        fclose(ptr_dinamid3_romfile);
    }



    if (leidos!=DINAMID3_ROM_SIZE || ptr_dinamid3_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading Dinamid3 rom file: %s",DINAMID3_ROM);
        return 1;
    }

    return 0;
}

void dinamid3_alloc_memory(void)
{

    int size=DINAMID3_ROM_SIZE;

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for dinamid3 emulation",size/1024);

    dinamid3_memory_pointer=util_malloc(size,"Can not allocate memory for dinamid3 emulation");


}

void dinamid3_reset(void)
{

    if (dinamid3_enabled.v==0) {
        return;
    }

    dinamid3_mapped_rom_memory.v=0;

}

void dinamid3_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable dinamid3 on non Spectrum machine");
        return;
    }

	if (dinamid3_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"dinamid3 Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling dinamid3 interface");

	dinamid3_alloc_memory();


	if (dinamid3_load_rom()) return;

	dinamid3_set_peek_functions();

	dinamid3_enabled.v=1;


}

void dinamid3_disable(void)
{
	if (dinamid3_enabled.v==0) return;


	dinamid3_restore_peek_poke_functions();

	free(dinamid3_memory_pointer);


	dinamid3_enabled.v=0;
}


