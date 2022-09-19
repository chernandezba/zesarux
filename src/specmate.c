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

Microhobby Pokeador Automatico emulation

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "specmate.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"


z80_bit specmate_enabled={0};


z80_byte *specmate_memory_pointer;


int specmate_nested_id_peek_byte;
int specmate_nested_id_peek_byte_no_time;

z80_bit specmate_mapped_rom_memory={0};



z80_byte specmate_read_rom_byte(z80_int dir)
{
	return specmate_memory_pointer[dir];
}

int specmate_check_if_rom_area(z80_int dir)
{
    if (dir<8192 && specmate_mapped_rom_memory.v) return 1;
    else return 0;
}

z80_byte specmate_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(specmate_nested_id_peek_byte,dir);

	if (specmate_check_if_rom_area(dir)) {
		return specmate_read_rom_byte(dir);
	}


	return valor_leido;
}

z80_byte specmate_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(specmate_nested_id_peek_byte_no_time,dir);

	if (specmate_check_if_rom_area(dir)) {
		return specmate_read_rom_byte(dir);
	}

	return valor_leido;
}




void specmate_nmi(void)
{
    if (specmate_mapped_rom_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling specmate memory from nmi triggered");
        specmate_mapped_rom_memory.v=1;
    }   
}




//Establecer rutinas propias. Solo tiene rom por tanto peek y no poke
void specmate_set_peek_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting specmate peek functions");

	//Asignar mediante nuevas funciones de core anidados
	specmate_nested_id_peek_byte=debug_nested_peek_byte_add(specmate_peek_byte,"specmate peek_byte");
	specmate_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(specmate_peek_byte_no_time,"specmate peek_byte_no_time");



}

//Restaurar rutinas de specmate
void specmate_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original peek functions before specmate");


	debug_nested_peek_byte_del(specmate_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(specmate_nested_id_peek_byte_no_time);


}



int specmate_load_rom(void)
{

    //Aunque realmente no es una rom, sino una ram
    //Al final es un bloque de 1kb que se lee del PC hasta la ram del Pokeador
    //Pero lo guardamos en archivos .rom para que sea menos confuso para el usuario
    //Hay otro posible firmware, transpoke.tap, que se encuentra en el repo de extras
    //No se incluye aqui pues es un pokeador+copiador, requiere de un programa basic
    //para preparar los pokes antes de lanzar la nmi

    FILE *ptr_specmate_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading specmate rom %s",SPECMATE_ROM);

    open_sharedfile(SPECMATE_ROM,&ptr_specmate_romfile);
    if (!ptr_specmate_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_specmate_romfile!=NULL) {

        leidos=fread(specmate_memory_pointer,1,SPECMATE_ROM_SIZE,ptr_specmate_romfile);
        fclose(ptr_specmate_romfile);
    }



    if (leidos!=SPECMATE_ROM_SIZE || ptr_specmate_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading Specmate rom file: %s",SPECMATE_ROM);
        return 1;
    }

    return 0;
}

void specmate_alloc_memory(void)
{
    //Asignamos 2 kb de ram, igual que la interfaz real
    //aunque realmente se usan unos pocos bytes del primer kb (60h-6fh), y el segundo kb (3900h-3cffh)
    int size=SPECMATE_ROM_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for specmate emulation",size/1024);

    //Inicializamos memoria con FF, que es como (creo) se comportaba el interfaz original
    //Ya que no sobreescribimos toda la ram, lo que no se modifica, quedará con FF
    specmate_memory_pointer=util_malloc_fill(size,"Can not allocate memory for mhpokeador emulation",0xFF);


}

void specmate_reset(void)
{

    if (specmate_enabled.v==0) {
        return;
    }

    specmate_mapped_rom_memory.v=0;

}

void specmate_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable specmate on non Spectrum machine");
        return;
    }

	if (specmate_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"specmate Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling specmate interface");

	specmate_alloc_memory();


	if (specmate_load_rom()) return;

	specmate_set_peek_functions();

	specmate_enabled.v=1;


}

void specmate_disable(void)
{
	if (specmate_enabled.v==0) return;


	specmate_restore_peek_poke_functions();

	free(specmate_memory_pointer);


	specmate_enabled.v=0;
}


