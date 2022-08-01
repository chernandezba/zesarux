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

//#if defined(__APPLE__)
//        #include <sys/syslimits.h>
//#endif


#include "hilow_barbanegra.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"

//hilow_bbn = hilow barba negra

z80_bit hilow_bbn_enabled={0};


//8 KB ROM, 2 KB RAM
//RAM mapeada en modo mirror en 2000h, 2800h, 3000h, 3800h
z80_byte *hilow_bbn_memory_pointer;



int hilow_bbn_nested_id_poke_byte;
int hilow_bbn_nested_id_poke_byte_no_time;
int hilow_bbn_nested_id_peek_byte;
int hilow_bbn_nested_id_peek_byte_no_time;

z80_bit hilow_bbn_mapped_memory={0};



int hilow_bbn_check_if_rom_area(z80_int dir)
{
    if (dir<8192 && hilow_bbn_mapped_memory.v) {
        return 1;
    }
	return 0;
}

int hilow_bbn_check_if_ram_area(z80_int dir)
{
    if (dir>=8192 && dir<16384 && hilow_bbn_mapped_memory.v) {
        return 1;
    }
	return 0;
}

z80_byte hilow_bbn_read_rom_byte(z80_int dir)
{
	//printf ("Read rom byte from %04XH\n",dir);
	return hilow_bbn_memory_pointer[dir];
}


z80_byte hilow_bbn_read_ram_byte(z80_int dir)
{

	//printf ("Read ram byte from %04XH\n",dir);
	

    dir &= (HILOW_BARBANEGRA_RAM_SIZE-1);


	//La RAM esta despues de los 8kb de rom
	return hilow_bbn_memory_pointer[HILOW_BARBANEGRA_ROM_SIZE+dir];
}

void hilow_bbn_poke_ram(z80_int dir,z80_byte value)
{

	if (hilow_bbn_check_if_ram_area(dir) ) {
		//printf ("Poke ram byte to %04XH with value %02XH\n",dir,value);

        //8kb ram
        dir &= (HILOW_BARBANEGRA_RAM_SIZE-1);

		//La RAM esta despues de los 8kb de rom
		hilow_bbn_memory_pointer[HILOW_BARBANEGRA_ROM_SIZE+dir]=value;	
	}

}


z80_byte hilow_bbn_poke_byte(z80_int dir,z80_byte valor)
{

    //Llamar a anterior
    debug_nested_poke_byte_call_previous(hilow_bbn_nested_id_poke_byte,dir,valor);

	hilow_bbn_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte hilow_bbn_poke_byte_no_time(z80_int dir,z80_byte valor)
{
 
	//Llamar a anterior
	debug_nested_poke_byte_no_time_call_previous(hilow_bbn_nested_id_poke_byte_no_time,dir,valor);


	hilow_bbn_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte hilow_bbn_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(hilow_bbn_nested_id_peek_byte,dir);


	if (hilow_bbn_check_if_rom_area(dir)) {
		return hilow_bbn_read_rom_byte(dir);
	}

	if (hilow_bbn_check_if_ram_area(dir)) {
		return hilow_bbn_read_ram_byte(dir);
	}	

	return valor_leido;
}

z80_byte hilow_bbn_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(hilow_bbn_nested_id_peek_byte_no_time,dir);


	if (hilow_bbn_check_if_rom_area(dir)) {
        return hilow_bbn_read_rom_byte(dir);
    }

	if (hilow_bbn_check_if_ram_area(dir)) {
		return hilow_bbn_read_ram_byte(dir);
	}			

	return valor_leido;
}

int hilow_bbn_if_rom_basic_enabled(void)
{
    //Dice si esta activada la rom que contiene el basic
    //en 48k-> siempre
    //en maquinas 128k: rom3 en el caso de +2A/+3, rom 1 en caso de 128k
    if (MACHINE_IS_SPECTRUM_16_48) {

        //maquina 16k, inves ,48k o tk
        return 1;
    }

    if (MACHINE_IS_SPECTRUM_128_P2)  {

        //maquina 128k. rom 1 mapeada
        if ((puerto_32765 & 16) ==16)
        return 1;
    }    

    if (MACHINE_IS_SPECTRUM_P2A_P3) {
        //maquina +2A
        if ((puerto_32765 & 16) ==16   && ((puerto_8189&4) ==4  ))
        return 1;
    }    

    return 0;
}



void hilow_bbn_nmi(void)
{
    if (hilow_bbn_mapped_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling hilow_bbn memory from nmi triggered");
        hilow_bbn_mapped_memory.v=1;
    }   
}

void hilow_bbn_footer_operating(void)
{
    /*generic_footertext_print_operating("hilow_bbn");

    //Y poner icono en inverso
    if (!zxdesktop_icon_hilow_bbn_inverse) {
        //printf("icon activity\n");
        zxdesktop_icon_hilow_bbn_inverse=1;
        menu_draw_ext_desktop();
    }
    */
}




//Establecer rutinas propias
void hilow_bbn_set_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting hilow_bbn poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	hilow_bbn_nested_id_poke_byte=debug_nested_poke_byte_add(hilow_bbn_poke_byte,"hilow_bbn poke_byte");
	hilow_bbn_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(hilow_bbn_poke_byte_no_time,"hilow_bbn poke_byte_no_time");
	hilow_bbn_nested_id_peek_byte=debug_nested_peek_byte_add(hilow_bbn_peek_byte,"hilow_bbn peek_byte");
	hilow_bbn_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(hilow_bbn_peek_byte_no_time,"hilow_bbn peek_byte_no_time");



}

//Restaurar rutinas de hilow_bbn
void hilow_bbn_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before hilow_bbn");


	debug_nested_poke_byte_del(hilow_bbn_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(hilow_bbn_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(hilow_bbn_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(hilow_bbn_nested_id_peek_byte_no_time);


}



void hilow_bbn_alloc_rom_ram_memory(void)
{
    //memoria de la ram y rom
    int size=HILOW_BARBANEGRA_MEM_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for hilow_bbn emulation",size/1024);

    hilow_bbn_memory_pointer=malloc(size);
    if (hilow_bbn_memory_pointer==NULL) {
        cpu_panic ("No enough memory for hilow_bbn emulation");
    }


}



int hilow_bbn_load_rom(void)
{

    FILE *ptr_hilow_bbn_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading hilow_bbn rom %s",HILOW_BARBANEGRA_ROM_FILE_NAME);

    ptr_hilow_bbn_romfile=fopen(HILOW_BARBANEGRA_ROM_FILE_NAME,"rb");
    if (!ptr_hilow_bbn_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_hilow_bbn_romfile!=NULL) {

        leidos=fread(hilow_bbn_memory_pointer,1,HILOW_BARBANEGRA_ROM_SIZE,ptr_hilow_bbn_romfile);
        fclose(ptr_hilow_bbn_romfile);

    }



    if (leidos!=HILOW_BARBANEGRA_ROM_SIZE || ptr_hilow_bbn_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading hilow_bbn rom");
        return 1;
    }

    return 0;
}

void hilow_bbn_reset(void)
{

    if (hilow_bbn_enabled.v==0) {
        return;
    }

	hilow_bbn_mapped_memory.v=0;

}

void hilow_bbn_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable hilow_bbn on non Spectrum machine");
        return;
    }

	if (hilow_bbn_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"hilow_bbn Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling hilow_bbn interface");

	hilow_bbn_alloc_rom_ram_memory();


	if (hilow_bbn_load_rom()) return;

	hilow_bbn_set_peek_poke_functions();

	hilow_bbn_enabled.v=1;

	hilow_bbn_reset();

}

void hilow_bbn_disable(void)
{
	if (hilow_bbn_enabled.v==0) return;


	hilow_bbn_restore_peek_poke_functions();

	free(hilow_bbn_memory_pointer);


	hilow_bbn_enabled.v=0;
}



void hilow_bbn_write_port_fd(z80_int port GCC_UNUSED,z80_byte value GCC_UNUSED)
{
    //printf("hilow_bbn_write_port_fd: port %04XH value %02XH PC %04XH\n",port,value,reg_pc);

    //Simplemente desmapear la interfaz al enviar cualquier valor

    hilow_bbn_mapped_memory.v=0;
}


/*
z80_byte hilow_bbn_read_port_fd(z80_int puerto)
{

    

    return 255; 


}

*/