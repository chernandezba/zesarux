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

Phoenix emulation

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "phoenix.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"


z80_bit phoenix_enabled={0};


z80_byte *phoenix_memory_pointer;


int phoenix_nested_id_core;
int phoenix_nested_id_peek_byte;
int phoenix_nested_id_peek_byte_no_time;

z80_bit phoenix_mapped_rom_memory={0};

/*
TODO: Actualmente emulamos el Phoenix 3. Existió el 1 y el 2??
*/

z80_byte phoenix_read_rom_byte(z80_int dir)
{
	return phoenix_memory_pointer[dir];
}

int phoenix_check_if_rom_area(z80_int dir)
{
    if (dir<16384 && phoenix_mapped_rom_memory.v) return 1;
    else return 0;
}

z80_byte phoenix_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(phoenix_nested_id_peek_byte,dir);

	if (phoenix_check_if_rom_area(dir)) {
		return phoenix_read_rom_byte(dir);
	}


	return valor_leido;
}

z80_byte phoenix_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(phoenix_nested_id_peek_byte_no_time,dir);

	if (phoenix_check_if_rom_area(dir)) {
		return phoenix_read_rom_byte(dir);
	}

	return valor_leido;
}

z80_byte cpu_core_loop_phoenix(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{
    //printf("phoenix core dir %X\n",reg_pc);
    /*
    phoenix: parece que se desmapea rom al llegar a la 71H, esto en la rom normal:
    70H POP HL
    71H POP AF
    72H RETN    
    */

    //Direccion de retorno
    //ahi hay un pop af , que compensa su push af en su rom al mapear:

    //66H JP 0074H
    //...
    //74H PUSH AF
    //75H LD A,R

    //Ese PUSH AF se encuentra tambien logicamente en la rom normal:
    //66H PUSH AF
    //67H PUSH HL    

    if (phoenix_mapped_rom_memory.v && reg_pc==0x71) {
        //printf("Unmapping phoenix rom from dir %X\n",reg_pc);
        phoenix_mapped_rom_memory.v=0;
    }

    //Llamar a anterior
    debug_nested_core_call_previous(phoenix_nested_id_core);

    //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
    return 0;    

}

void phoenix_nmi(void)
{
    if (phoenix_mapped_rom_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling phoenix memory from nmi triggered");
        phoenix_mapped_rom_memory.v=1;
    }   
}




//Establecer rutinas propias. Solo tiene rom por tanto peek y no poke
void phoenix_set_peek_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting phoenix peek/core functions");

	//Asignar mediante nuevas funciones de core anidados
	phoenix_nested_id_peek_byte=debug_nested_peek_byte_add(phoenix_peek_byte,"phoenix peek_byte");
	phoenix_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(phoenix_peek_byte_no_time,"phoenix peek_byte_no_time");

    phoenix_nested_id_core=debug_nested_core_add(cpu_core_loop_phoenix,"phoenix core");

}

//Restaurar rutinas de phoenix
void phoenix_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original peek functions before phoenix");


	debug_nested_peek_byte_del(phoenix_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(phoenix_nested_id_peek_byte_no_time);
    debug_nested_core_del(phoenix_nested_id_core);


}



int phoenix_load_rom(void)
{

    FILE *ptr_phoenix_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading phoenix rom %s",PHOENIX_ROM);

    open_sharedfile(PHOENIX_ROM,&ptr_phoenix_romfile);
    if (!ptr_phoenix_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_phoenix_romfile!=NULL) {

        leidos=fread(phoenix_memory_pointer,1,PHOENIX_ROM_SIZE,ptr_phoenix_romfile);
        fclose(ptr_phoenix_romfile);
    }



    if (leidos!=PHOENIX_ROM_SIZE || ptr_phoenix_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading Phoenix rom file: %s",PHOENIX_ROM);
        return 1;
    }

    return 0;
}

void phoenix_alloc_memory(void)
{

    int size=PHOENIX_ROM_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for phoenix emulation",size/1024);

    phoenix_memory_pointer=util_malloc(size,"Can not allocate memory for phoenix emulation");


}

void phoenix_reset(void)
{

    if (phoenix_enabled.v==0) {
        return;
    }

    phoenix_mapped_rom_memory.v=0;

}

void phoenix_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable phoenix on non Spectrum machine");
        return;
    }

	if (phoenix_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"phoenix Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling phoenix interface");

	phoenix_alloc_memory();


	if (phoenix_load_rom()) return;

	phoenix_set_peek_functions();

	phoenix_enabled.v=1;


}

void phoenix_disable(void)
{
	if (phoenix_enabled.v==0) return;


	phoenix_restore_peek_poke_functions();

	free(phoenix_memory_pointer);


	phoenix_enabled.v=0;
}


