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


#include "transtape.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "mem128.h"



z80_bit transtape_enabled={0};


//16 KB ROM, 2 KB RAM
//RAM mapeada en 3800H
z80_byte *transtape_memory_pointer;

//Info esquemas:
//https://www.ciberpucela.com/wp1/2022/04/06/que-tiempos-aquellos-todos-queriamos-una-transtape/


int transtape_nested_id_poke_byte;
int transtape_nested_id_poke_byte_no_time;
int transtape_nested_id_peek_byte;
int transtape_nested_id_peek_byte_no_time;

z80_bit transtape_mapped_ram_memory={0};
z80_bit transtape_mapped_rom_memory={0};

//z80_int conmutadores_load_save_turbo=1024+2048;
z80_int conmutadores_load_save_turbo=1024+2048;

int transtape_check_if_rom_area(z80_int dir)
{
    if (dir<16384 && transtape_mapped_rom_memory.v) {
        return 1;
    }
	return 0;
}

int transtape_check_if_ram_area(z80_int dir)
{
    if (dir>=0x3800 && dir<16384 && transtape_mapped_ram_memory.v) {
        //printf("transtape_check_if_ram_area dir %04XH . is ram\n",dir);
        return 1;
    }
    //printf("transtape_check_if_ram_area dir %04XH . is NOT ram\n",dir);
	return 0;
}

z80_byte transtape_read_rom_byte(z80_int dir)
{
	//printf ("Read rom byte from %04XH\n",dir);
    dir &=(65535-1024-2048); //quitar bits 10 y 11
    //y ponerlo segun indique la mascara
    dir |=conmutadores_load_save_turbo;
	return transtape_memory_pointer[dir];
}


z80_byte transtape_read_ram_byte(z80_int dir)
{

	//printf ("Read ram byte from %04XH\n",dir);
	

    dir &= (TRANSTAPE_RAM_SIZE-1);


	//La RAM esta despues de la rom
	return transtape_memory_pointer[TRANSTAPE_ROM_SIZE+dir];
}

void transtape_poke_ram(z80_int dir,z80_byte value)
{

	if (transtape_check_if_ram_area(dir) ) {
		printf ("Poke ram byte to %04XH with value %02XH\n",dir,value);

        dir &= (TRANSTAPE_RAM_SIZE-1);

		//La RAM esta despues de la rom
		transtape_memory_pointer[TRANSTAPE_ROM_SIZE+dir]=value;	
	}

    else {
        if (dir<16384) printf ("NOT Poke ram byte to %04XH with value %02XH\n",dir,value);
    }

}


z80_byte transtape_poke_byte(z80_int dir,z80_byte valor)
{

    //Llamar a anterior
    debug_nested_poke_byte_call_previous(transtape_nested_id_poke_byte,dir,valor);

	transtape_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte transtape_poke_byte_no_time(z80_int dir,z80_byte valor)
{
 
	//Llamar a anterior
	debug_nested_poke_byte_no_time_call_previous(transtape_nested_id_poke_byte_no_time,dir,valor);


	transtape_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte transtape_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(transtape_nested_id_peek_byte,dir);

    //RAM tiene prioridad
	if (transtape_check_if_ram_area(dir)) {
		return transtape_read_ram_byte(dir);
	}

	if (transtape_check_if_rom_area(dir)) {
		return transtape_read_rom_byte(dir);
	}


	return valor_leido;
}

z80_byte transtape_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(transtape_nested_id_peek_byte_no_time,dir);

    //RAM tiene prioridad
	if (transtape_check_if_ram_area(dir)) {
		return transtape_read_ram_byte(dir);
	}

	if (transtape_check_if_rom_area(dir)) {
        return transtape_read_rom_byte(dir);
    }
	

	return valor_leido;
}

int transtape_if_rom_basic_enabled(void)
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



void transtape_nmi(void)
{
    if (transtape_mapped_rom_memory.v==0) {
        debug_printf(VERBOSE_DEBUG,"Enabling transtape memory from nmi triggered");
        transtape_mapped_ram_memory.v=1;
        transtape_mapped_rom_memory.v=1;
    }   
}

void transtape_footer_operating(void)
{
    /*generic_footertext_print_operating("transtape");

    //Y poner icono en inverso
    if (!zxdesktop_icon_transtape_inverse) {
        //printf("icon activity\n");
        zxdesktop_icon_transtape_inverse=1;
        menu_draw_ext_desktop();
    }
    */
}




//Establecer rutinas propias
void transtape_set_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting transtape poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	transtape_nested_id_poke_byte=debug_nested_poke_byte_add(transtape_poke_byte,"transtape poke_byte");
	transtape_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(transtape_poke_byte_no_time,"transtape poke_byte_no_time");
	transtape_nested_id_peek_byte=debug_nested_peek_byte_add(transtape_peek_byte,"transtape peek_byte");
	transtape_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(transtape_peek_byte_no_time,"transtape peek_byte_no_time");



}

//Restaurar rutinas de transtape
void transtape_restore_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before transtape");


	debug_nested_poke_byte_del(transtape_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(transtape_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(transtape_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(transtape_nested_id_peek_byte_no_time);


}



void transtape_alloc_rom_ram_memory(void)
{
    //memoria de la ram y rom
    int size=TRANSTAPE_MEM_SIZE;  

    debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for transtape emulation",size/1024);

    transtape_memory_pointer=malloc(size);
    if (transtape_memory_pointer==NULL) {
        cpu_panic ("No enough memory for transtape emulation");
    }


}



int transtape_load_rom(void)
{

    FILE *ptr_transtape_romfile;
    int leidos=0;

    debug_printf (VERBOSE_INFO,"Loading transtape rom %s",TRANSTAPE_ROM_FILE_NAME);

    ptr_transtape_romfile=fopen(TRANSTAPE_ROM_FILE_NAME,"rb");
    if (!ptr_transtape_romfile) {
            debug_printf (VERBOSE_ERR,"Unable to open ROM file");
    }

    if (ptr_transtape_romfile!=NULL) {

        leidos=fread(transtape_memory_pointer,1,TRANSTAPE_ROM_SIZE,ptr_transtape_romfile);
        fclose(ptr_transtape_romfile);

    }



    if (leidos!=TRANSTAPE_ROM_SIZE || ptr_transtape_romfile==NULL) {
        debug_printf (VERBOSE_ERR,"Error reading transtape rom");
        return 1;
    }

    return 0;
}

void transtape_reset(void)
{

    if (transtape_enabled.v==0) {
        return;
    }

	transtape_mapped_ram_memory.v=0;
    transtape_mapped_rom_memory.v=0;

}

void transtape_enable(void)
{

    if (!MACHINE_IS_SPECTRUM) {
        debug_printf(VERBOSE_INFO,"Can not enable transtape on non Spectrum machine");
        return;
    }

	if (transtape_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"transtape Already enabled");
		return;
	}

    debug_printf (VERBOSE_DEBUG,"Enabling transtape interface");

	transtape_alloc_rom_ram_memory();


	if (transtape_load_rom()) return;

	transtape_set_peek_poke_functions();

	transtape_enabled.v=1;

	transtape_reset();

}

void transtape_disable(void)
{
	if (transtape_enabled.v==0) return;


	transtape_restore_peek_poke_functions();

	free(transtape_memory_pointer);


	transtape_enabled.v=0;
}



void transtape_write_port(z80_byte puerto_l,z80_byte value)
{

    printf("transtape_write_port %d value %d pc: %04XH\n",puerto_l,value,reg_pc);

    switch (puerto_l) {
        case 63: //3FH= 00111111
            //rom spectrum pero ram transtape
            transtape_mapped_ram_memory.v=1;
            transtape_mapped_rom_memory.v=0;
        break;

        case 127: //7FH = 01111111
            //rom y ram de transtape
            transtape_mapped_ram_memory.v=1;
            transtape_mapped_rom_memory.v=1;            
        break;

        case 191: //BFH =10111111
            //desactivar rom y ram de transtape
            transtape_mapped_ram_memory.v=0;
            transtape_mapped_rom_memory.v=0;              
        break;
    }
}


/*
z80_byte transtape_read_port_fd(z80_int puerto)
{

    

    return 255; 


}

*/