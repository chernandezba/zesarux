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


#include "ifrom.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"


z80_bit ifrom_enabled={0};

z80_bit ifrom_protected={0};

char ifrom_rom_file_name[PATH_MAX]="";

z80_byte *ifrom_memory_pointer;



int ifrom_nested_id_poke_byte;
int ifrom_nested_id_poke_byte_no_time;
int ifrom_nested_id_peek_byte;
int ifrom_nested_id_peek_byte_no_time;

//Banco activo. 0..31
z80_byte ifrom_active_bank=0;


/*
byte paginacion: al escribir en rom (a14=a15=0), con mreq=0 y wr=0
5 bits bajos: es la pagina de los 512 kb de eeprom
bit 7: bit de bloqueo (a 1), impide futuras escrituras en este byte de paginacion

botones: reset, nmi. nmi solo salta a la 66h
en un reset (y al arrancar) byte de paginacion=0
*/


int ifrom_check_if_rom_area(z80_int dir)
{
                if (dir<16384) {
			return 1;
                }
	return 0;
}

z80_byte ifrom_read_byte(z80_int dir)
{
	//Si no, memoria ifrom
	int puntero=ifrom_active_bank*16384+dir;
	return ifrom_memory_pointer[puntero];
}


void ifrom_handle_special_dirs(z80_int dir,z80_byte value)
{

	if (ifrom_protected.v) return;

	if (dir<=0x3FFF) {
		/*
		byte paginacion: al escribir en rom (a14=a15=0), con mreq=0 y wr=0
5 bits bajos: es la pagina de los 512 kb de eeprom
bit 7: bit de bloqueo (a 1), impide futuras escrituras en este byte de paginacion
		*/
		
		//Banco final
		ifrom_active_bank=value&31;


		//Si se habilita proteccion
		if (value&128) ifrom_protected.v=1;

		//printf ("Cambiando a banco %d proteccion %d\n",ifrom_active_bank,ifrom_protected.v);
	}

}


z80_byte ifrom_poke_byte(z80_int dir,z80_byte valor)
{

	//ifrom_original_poke_byte(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_call_previous(ifrom_nested_id_poke_byte,dir,valor);

	ifrom_handle_special_dirs(dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte ifrom_poke_byte_no_time(z80_int dir,z80_byte valor)
{
        //ifrom_original_poke_byte_no_time(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_no_time_call_previous(ifrom_nested_id_poke_byte_no_time,dir,valor);


	ifrom_handle_special_dirs(dir,valor);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte ifrom_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(ifrom_nested_id_peek_byte,dir);

	//ifrom_handle_special_dirs(dir);

	if (ifrom_check_if_rom_area(dir)) {
		return ifrom_read_byte(dir);
	}

	//return ifrom_original_peek_byte(dir);
	return valor_leido;
}

z80_byte ifrom_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(ifrom_nested_id_peek_byte_no_time,dir);

	//ifrom_handle_special_dirs(dir);

	if (ifrom_check_if_rom_area(dir)) {
                return ifrom_read_byte(dir);
        }

	return valor_leido;
}



//Establecer rutinas propias
void ifrom_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting ifrom poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	ifrom_nested_id_poke_byte=debug_nested_poke_byte_add(ifrom_poke_byte,"iFrom poke_byte");
	ifrom_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(ifrom_poke_byte_no_time,"iFrom poke_byte_no_time");
	ifrom_nested_id_peek_byte=debug_nested_peek_byte_add(ifrom_peek_byte,"iFrom peek_byte");
	ifrom_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(ifrom_peek_byte_no_time,"iFrom peek_byte_no_time");

}

//Restaurar rutinas de ifrom
void ifrom_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before ifrom");


	debug_nested_poke_byte_del(ifrom_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(ifrom_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(ifrom_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(ifrom_nested_id_peek_byte_no_time);
}



void ifrom_alloc_memory(void)
{
        int size=IFROM_SIZE;  

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for ifrom emulation",size/1024);

        ifrom_memory_pointer=malloc(size);
        if (ifrom_memory_pointer==NULL) {
                cpu_panic ("No enough memory for ifrom emulation");
        }


}

int ifrom_load_rom(void)
{

        FILE *ptr_ifrom_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading ifrom rom %s",ifrom_rom_file_name);

  			ptr_ifrom_romfile=fopen(ifrom_rom_file_name,"rb");
                if (!ptr_ifrom_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
                }

        if (ptr_ifrom_romfile!=NULL) {

                leidos=fread(ifrom_memory_pointer,1,IFROM_SIZE,ptr_ifrom_romfile);
                fclose(ptr_ifrom_romfile);

        }



        if (leidos!=IFROM_SIZE || ptr_ifrom_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading ifrom rom");
                return 1;
        }

        return 0;
}



void ifrom_enable(void)
{

  if (!MACHINE_IS_SPECTRUM && !MACHINE_IS_CPC) {
    debug_printf(VERBOSE_INFO,"Can not enable ifrom on non Spectrum or CPC machine");
    return;
  }

	if (ifrom_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"Already enabled");
		return;
	}

	if (ifrom_rom_file_name[0]==0) {
		debug_printf (VERBOSE_ERR,"Trying to enable iFrom but no ROM file selected");
		return;
	}

	ifrom_alloc_memory();
	if (ifrom_load_rom()) return;

	ifrom_set_peek_poke_functions();

	ifrom_enabled.v=1;

	ifrom_press_button();

	//ifrom_active_bank=0;
	//ifrom_protected.v=0;
	



}

void ifrom_disable(void)
{
	if (ifrom_enabled.v==0) return;

	ifrom_restore_peek_poke_functions();

	free(ifrom_memory_pointer);

	ifrom_enabled.v=0;
}


void ifrom_press_button(void)
{

        if (ifrom_enabled.v==0) {
                debug_printf (VERBOSE_ERR,"Trying to press iFrom button when it is disabled");
                return;
        }

	ifrom_active_bank=0;
	ifrom_protected.v=0;

	reset_cpu();


}
