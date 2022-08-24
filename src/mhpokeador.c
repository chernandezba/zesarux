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

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "mhpokeador.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"



z80_bit mhpokeador_enabled={0};



z80_byte *mhpokeador_memory_pointer;


int mhpokeador_nested_id_poke_byte;
int mhpokeador_nested_id_poke_byte_no_time;
int mhpokeador_nested_id_peek_byte;
int mhpokeador_nested_id_peek_byte_no_time;


char mhpokeador_rom_filename[PATH_MAX]="";

int mhpokeador_tipo_rom_cargar=MHPOKEADOR_TIPO_ROM_POKEADOR;

int mhpokeador_check_if_ram_area(z80_int dir)
{   
    if ( 
        (dir>=0x60 && dir<=0x6f)
        ||
        (dir>=0x3900 && dir<=0x3cff)
    ) {

        return 1;

    }

    else return 0;
}

z80_byte *mhpokeador_return_memory_pointer(z80_int dir)
{
	if (dir>=0x60 && dir<=0x6f) {
        //Primer KB de RAM
        return &mhpokeador_memory_pointer[dir];
    }
    else {
        //Segundo KB de RAM
        dir -=0x3900;
	    dir &= 1023;
        return &mhpokeador_memory_pointer[1024+dir];
    }    
}


z80_byte mhpokeador_read_ram_byte(z80_int dir)
{

    z80_byte *puntero;

    puntero=mhpokeador_return_memory_pointer(dir);

	return *puntero;
}

void mhpokeador_poke_ram(z80_int dir,z80_byte value)
{

	if (mhpokeador_check_if_ram_area(dir) ) {
		//printf ("Poke ram byte to %04XH with value %02XH\n",dir,value);

        z80_byte *puntero;

        puntero=mhpokeador_return_memory_pointer(dir);
        *puntero=value;

	}

}



z80_byte mhpokeador_poke_byte(z80_int dir,z80_byte valor)
{

    //Llamar a anterior
    debug_nested_poke_byte_call_previous(mhpokeador_nested_id_poke_byte,dir,valor);

	mhpokeador_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte mhpokeador_poke_byte_no_time(z80_int dir,z80_byte valor)
{
 
	//Llamar a anterior
	debug_nested_poke_byte_no_time_call_previous(mhpokeador_nested_id_poke_byte_no_time,dir,valor);


	mhpokeador_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte mhpokeador_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(mhpokeador_nested_id_peek_byte,dir);

	if (mhpokeador_check_if_ram_area(dir)) {
		return mhpokeador_read_ram_byte(dir);
	}




	return valor_leido;
}

z80_byte mhpokeador_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(mhpokeador_nested_id_peek_byte_no_time,dir);

	if (mhpokeador_check_if_ram_area(dir)) {
		return mhpokeador_read_ram_byte(dir);
	}



	return valor_leido;
}




void mhpokeador_nmi(void)
{
    //Realmente no hay que hacer nada, ni paginar ni nada...
}




//Establecer rutinas propias
void mhpokeador_set_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting mhpokeador poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	mhpokeador_nested_id_poke_byte=debug_nested_poke_byte_add(mhpokeador_poke_byte,"mhpokeador poke_byte");
	mhpokeador_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(mhpokeador_poke_byte_no_time,"mhpokeador poke_byte_no_time");
	mhpokeador_nested_id_peek_byte=debug_nested_peek_byte_add(mhpokeador_peek_byte,"mhpokeador peek_byte");
	mhpokeador_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(mhpokeador_peek_byte_no_time,"mhpokeador peek_byte_no_time");



}

//Restaurar rutinas de mhpokeador
void mhpokeador_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before mhpokeador");


	debug_nested_poke_byte_del(mhpokeador_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(mhpokeador_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(mhpokeador_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(mhpokeador_nested_id_peek_byte_no_time);


}



void mhpokeador_alloc_ram_memory(void)
{
    //Asignamos 2 kb de ram, igual que la interfaz real
    //aunque realmente se usan unos pocos bytes del primer kb (60h-6fh), y el segundo kb (3900h-3cffh)
    int size=MHPOKEADOR_RAM_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for mhpokeador emulation",size/1024);

    //Inicializamos memoria con FF, que es como (creo) se comportaba el interfaz original
    //Ya que no sobreescribimos toda la ram, lo que no se modifica, quedará con FF
    mhpokeador_memory_pointer=util_malloc_fill(size,"Can not allocate memory for mhpokeador emulation",0xFF);


}



int mhpokeador_load_rom(void)
{

    //Aunque realmente no es una rom, sino una ram
    //Al final es un bloque de 1kb que se lee del PC hasta la ram del Pokeador
    //Pero lo guardamos en archivos .rom para que sea menos confuso para el usuario
    //Hay otro posible firmware, transpoke.tap, que se encuentra en el repo de extras
    //No se incluye aqui pues es un pokeador+copiador, requiere de un programa basic
    //para preparar los pokes antes de lanzar la nmi

    FILE *ptr_mhpokeador_romfile;
    int leidos=0;

    char nombre_rom[PATH_MAX];

    if (mhpokeador_rom_filename[0]!=0) {
        strcpy(nombre_rom,mhpokeador_rom_filename);
    }
    else {
        //Roms por defecto
        if (mhpokeador_tipo_rom_cargar==MHPOKEADOR_TIPO_ROM_TRANSFER) strcpy(nombre_rom,MHPOKEADOR_ROM_TRANSFER);
        else if (mhpokeador_tipo_rom_cargar==MHPOKEADOR_TIPO_ROM_SALVAPAN) strcpy(nombre_rom,MHPOKEADOR_ROM_SALVAPAN);
        else strcpy(nombre_rom,MHPOKEADOR_ROM_POKEADOR);
    }

    debug_printf (VERBOSE_INFO,"Loading mhpokeador rom %s",nombre_rom);

    open_sharedfile(nombre_rom,&ptr_mhpokeador_romfile);
    if (!ptr_mhpokeador_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_mhpokeador_romfile!=NULL) {

        //Cargarlo en el segundo KB
        leidos=fread(&mhpokeador_memory_pointer[1024],1,MHPOKEADOR_ROM_SIZE,ptr_mhpokeador_romfile);
        fclose(ptr_mhpokeador_romfile);

        //Y meter el JP en la 66h del primer KB
        mhpokeador_memory_pointer[0x66]=0xC3;
        mhpokeador_memory_pointer[0x67]=0x00;
        mhpokeador_memory_pointer[0x68]=0x39;
    }



    if (leidos!=MHPOKEADOR_ROM_SIZE || ptr_mhpokeador_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading Microhobby Pokeador Automatico rom file: %s",nombre_rom);
        return 1;
    }

    return 0;
}


void mhpokeador_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable mhpokeador on non Spectrum machine");
        return;
    }

	if (mhpokeador_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"mhpokeador Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling mhpokeador interface");

	mhpokeador_alloc_ram_memory();


	if (mhpokeador_load_rom()) return;

	mhpokeador_set_peek_poke_functions();

	mhpokeador_enabled.v=1;


}

void mhpokeador_disable(void)
{
	if (mhpokeador_enabled.v==0) return;


	mhpokeador_restore_peek_poke_functions();

	free(mhpokeador_memory_pointer);


	mhpokeador_enabled.v=0;
}


