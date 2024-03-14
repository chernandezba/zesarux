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

Ramjet emulation

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#include "ramjet.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"


z80_bit ramjet_enabled={0};


z80_byte *ramjet_memory_pointer;


int ramjet_nested_id_peek_byte;
int ramjet_nested_id_peek_byte_no_time;

z80_bit ramjet_mapped_rom_memory={0};

//Nota: que yo sepa, no tiene manera de desmapearse una vez se ha activado

int ramjet_version=3;

z80_byte ramjet_read_rom_byte(z80_int dir)
{
    //D3 y D4 estan intercambiados
    z80_byte valor_leido=ramjet_memory_pointer[dir];

    z80_byte valor_d3=valor_leido & 0x08;
    z80_byte valor_d4=valor_leido & 0x10;

    valor_d3 <<=1;
    valor_d4 >>=1;

    valor_leido &= 0xE7;

    valor_leido |=valor_d3;
    valor_leido |=valor_d4;

	return valor_leido;
}

int ramjet_check_if_rom_area(z80_int dir)
{
    if (dir<16384 && ramjet_mapped_rom_memory.v) return 1;
    else return 0;
}

z80_byte ramjet_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(ramjet_nested_id_peek_byte,dir);

	if (ramjet_check_if_rom_area(dir)) {
		return ramjet_read_rom_byte(dir);
	}


	return valor_leido;
}

z80_byte ramjet_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(ramjet_nested_id_peek_byte_no_time,dir);

	if (ramjet_check_if_rom_area(dir)) {
		return ramjet_read_rom_byte(dir);
	}

	return valor_leido;
}



void ramjet_nmi(void)
{
    if (ramjet_mapped_rom_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling ramjet memory from nmi triggered");
        ramjet_mapped_rom_memory.v=1;
    }
}




//Establecer rutinas propias. Solo tiene rom por tanto peek y no poke
void ramjet_set_peek_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting ramjet peek functions");

	ramjet_nested_id_peek_byte=debug_nested_peek_byte_add(ramjet_peek_byte,"ramjet peek_byte");
	ramjet_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(ramjet_peek_byte_no_time,"ramjet peek_byte_no_time");


}

//Restaurar rutinas de ramjet
void ramjet_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original peek functions before ramjet");


	debug_nested_peek_byte_del(ramjet_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(ramjet_nested_id_peek_byte_no_time);



}



int ramjet_load_rom(void)
{

    FILE *ptr_ramjet_romfile;
    int leidos=0;

    char nombre_rom[PATH_MAX];

    if (ramjet_version==2) {
        strcpy(nombre_rom,RAMJET_ROM_V2);
    }
    else {
        strcpy(nombre_rom,RAMJET_ROM_V3);
    }

    debug_printf (VERBOSE_INFO,"Loading ramjet rom %s",nombre_rom);

    open_sharedfile(nombre_rom,&ptr_ramjet_romfile);
    if (!ptr_ramjet_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_ramjet_romfile!=NULL) {

        leidos=fread(ramjet_memory_pointer,1,RAMJET_ROM_SIZE,ptr_ramjet_romfile);
        fclose(ptr_ramjet_romfile);
    }



    if (leidos!=RAMJET_ROM_SIZE || ptr_ramjet_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading ramjet rom file: %s",nombre_rom);
        return 1;
    }

    return 0;
}

void ramjet_alloc_memory(void)
{

    int size=RAMJET_ROM_SIZE;

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for ramjet emulation",size/1024);

    ramjet_memory_pointer=util_malloc(size,"Can not allocate memory for ramjet emulation");


}

void ramjet_reset(void)
{

    if (ramjet_enabled.v==0) {
        return;
    }

    //En este interfaz hacemos lo contrario, con el reset aparece activa
    ramjet_mapped_rom_memory.v=1;

}

void ramjet_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable ramjet on non Spectrum machine");
        return;
    }

	if (ramjet_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"ramjet Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling ramjet interface");

	ramjet_alloc_memory();


	if (ramjet_load_rom()) return;

	ramjet_set_peek_functions();

	ramjet_enabled.v=1;


}

void ramjet_disable(void)
{
	if (ramjet_enabled.v==0) return;


	ramjet_restore_peek_poke_functions();

	free(ramjet_memory_pointer);


	ramjet_enabled.v=0;
}


void ramjet_write_port(z80_byte value)
{
    //printf ("Out Port Ramjet %x written with value %x, PC after=0x%x\n",puerto,value,reg_pc);

    //TODO: validar esto
    if (value & 128) {
        //printf("Desactivando ramjet a peticion puerto\n");
        ramjet_mapped_rom_memory.v=0;
    }
    else {
        //printf("Activando ramjet a peticion puerto\n");
        ramjet_mapped_rom_memory.v=1;
    }
}


int ramjet_save_detect(void)
{
    /*
05bbh: save en ramjet3.rom ex af,af', inc de, .....
2202h: save en ramjet3.rom desde copion

0581h: save en ramjet2.rom. ex af,af', inc de , dec ix, di, ld a,2....
184ch: save en ramjet2.rom desde copion
    */
    if (ramjet_version==2) {
        if (reg_pc==0x0581 || reg_pc==0x184c) return 1;
    }

    else {
        if (reg_pc==0x05bb || reg_pc==0x2202) return 1;
    }

    //temp
    return 0;
}