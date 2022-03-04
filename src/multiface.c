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


#include "multiface.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"


z80_bit multiface_enabled={0};

char multiface_rom_file_name[PATH_MAX]="";
//char multiface_rom_file_name[PATH_MAX]="mf1.rom";

z80_byte *multiface_memory_pointer;
z80_byte *multiface_ram_memory_pointer;


z80_bit multiface_switched_on={0};

int multiface_nested_id_poke_byte;
int multiface_nested_id_poke_byte_no_time;
int multiface_nested_id_peek_byte;
int multiface_nested_id_peek_byte_no_time;


//Puerto para  mapeo y desmapeo de memoria multiface segun el tipo de interfaz seleccionado
z80_byte multiface_current_port_in;
z80_byte multiface_current_port_out;

int multiface_lockout=0;


char *multiface_types_string[MULTIFACE_TOTAL_TYPES]={"One","128","Three"};

z80_byte multiface_type=0;

int multiface_check_if_ramrom_area(z80_int dir)
{
        if (multiface_switched_on.v) {
                if (dir<16384) {
			return 1;
                }
        }
	return 0;
}

int multiface_check_if_ram_area(z80_int dir)
{
        if (multiface_switched_on.v) {
                if (dir<16384 && dir>8191) {
                        return 1;
                }
        }
        return 0;
}

z80_byte multiface_read_byte(z80_int dir)
{
	if (dir<8192) return multiface_memory_pointer[dir];
        else return multiface_ram_memory_pointer[dir-8192];
}

void multiface_write_byte(z80_int dir,z80_byte value)
{
        //Este if no haria falta ya que cuando se entra aqui ya se sabe que se esta en esa region
        if (dir>=8192 && dir<=16383) multiface_ram_memory_pointer[dir-8192]=value;
}


z80_byte multiface_poke_byte(z80_int dir,z80_byte valor)
{

	//multiface_original_poke_byte(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_call_previous(multiface_nested_id_poke_byte,dir,valor);

	if (multiface_check_if_ram_area(dir)) {
		multiface_write_byte(dir,valor);
	}

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte multiface_poke_byte_no_time(z80_int dir,z80_byte valor)
{
        //multiface_original_poke_byte_no_time(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_no_time_call_previous(multiface_nested_id_poke_byte_no_time,dir,valor);


	if (multiface_check_if_ram_area(dir)) {
                multiface_write_byte(dir,valor);
        }

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte multiface_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(multiface_nested_id_peek_byte,dir);

	if (multiface_check_if_ramrom_area(dir)) {
		return multiface_read_byte(dir);
	}

	return valor_leido;
}

z80_byte multiface_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(multiface_nested_id_peek_byte_no_time,dir);

	if (multiface_check_if_ramrom_area(dir)) {
                return multiface_read_byte(dir);
        }

	return valor_leido;
}



//Establecer rutinas propias
void multiface_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting multiface poke / peek functions");
                //Guardar anteriores
                //multiface_original_poke_byte=poke_byte;
                //multiface_original_poke_byte_no_time=poke_byte_no_time;
                //multiface_original_peek_byte=peek_byte;
                //multiface_original_peek_byte_no_time=peek_byte_no_time;

                //Modificar y poner las de multiface
                //poke_byte=multiface_poke_byte;
                //poke_byte_no_time=multiface_poke_byte_no_time;
                //peek_byte=multiface_peek_byte;
                //peek_byte_no_time=multiface_peek_byte_no_time;


	//Asignar mediante nuevas funciones de core anidados
	multiface_nested_id_poke_byte=debug_nested_poke_byte_add(multiface_poke_byte,"Multiface poke_byte");
	multiface_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(multiface_poke_byte_no_time,"Multiface poke_byte_no_time");
	multiface_nested_id_peek_byte=debug_nested_peek_byte_add(multiface_peek_byte,"Multiface peek_byte");
	multiface_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(multiface_peek_byte_no_time,"Multiface peek_byte_no_time");

}

//Restaurar rutinas de multiface
void multiface_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before multiface");
                //poke_byte=multiface_original_poke_byte;
                //poke_byte_no_time=multiface_original_poke_byte_no_time;
                //peek_byte=multiface_original_peek_byte;
                //peek_byte_no_time=multiface_original_peek_byte_no_time;


	debug_nested_poke_byte_del(multiface_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(multiface_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(multiface_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(multiface_nested_id_peek_byte_no_time);
}


void multiface_alloc_memory(void)
{
        int size=(MULTIFACE_SIZE);

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for multiface emulation",size/1024);

        multiface_memory_pointer=malloc(size);
        if (multiface_memory_pointer==NULL) {
                cpu_panic ("No enough memory for multiface emulation");
        }

        multiface_ram_memory_pointer=&multiface_memory_pointer[8192];

}

int multiface_load_rom(void)
{

        FILE *ptr_multiface_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading multiface rom %s",multiface_rom_file_name);

  			ptr_multiface_romfile=fopen(multiface_rom_file_name,"rb");
                if (!ptr_multiface_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
                }

        if (ptr_multiface_romfile!=NULL) {

                leidos=fread(multiface_memory_pointer,1,8192,ptr_multiface_romfile);
                fclose(ptr_multiface_romfile);

        }



	//8kb rom
        if (leidos!=8192 || ptr_multiface_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading multiface rom %s",multiface_rom_file_name);
                return 1;
        }

        return 0;
}



void multiface_enable(void)
{

  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable multiface on non Spectrum machine");
    return;
  }

	if (multiface_enabled.v) return;

        //TBBLUE tiene su propio multiface
        if (MACHINE_IS_TBBLUE) {

                //old: 0x014000 – 0x017FFF (16K) => Multiface ROM
                //new: -- 0x014000 - 0x017FFF (16K)  => Multiface ROM,RAM       A20:A16 = 00001,01
                multiface_memory_pointer=&memoria_spectrum[0x014000];

                //old: 0x01c000 – 0x01FFFF (16K) => Multiface RAM
                //   -- 0x014000 - 0x017FFF (16K)  => Multiface ROM,RAM       A20:A16 = 00001,01
                multiface_ram_memory_pointer=&memoria_spectrum[0x016000];
        }

        else {

	        if (multiface_rom_file_name[0]==0) {
		        debug_printf (VERBOSE_ERR,"Trying to enable Multiface but no ROM file selected");
		        return;
	        }


	        multiface_alloc_memory();
	        if (multiface_load_rom()) return;
        }

        

	multiface_set_peek_poke_functions();


  /*switch (multiface_type) {
    case MULTIFACE_TYPE_ONE:
      multiface_current_port_in=MULTIFACE_PORT_ONE_IN;
      multiface_current_port_out=MULTIFACE_PORT_ONE_OUT;
    break;

    case MULTIFACE_TYPE_128:
      multiface_current_port_in=MULTIFACE_PORT_128_IN;
      multiface_current_port_out=MULTIFACE_PORT_128_OUT;
    break;

    case MULTIFACE_TYPE_THREE:
      multiface_current_port_in=MULTIFACE_PORT_THREE_IN;
      multiface_current_port_out=MULTIFACE_PORT_THREE_OUT;
    break;

    default:
      cpu_panic("Unsupported multiface type");
    break;
  }*/





	multiface_switched_on.v=0;

	multiface_enabled.v=1;


}

void multiface_disable(void)
{
	if (multiface_enabled.v==0) return;

	multiface_restore_peek_poke_functions();

        //TBBLUE tiene su propio multiface
        if (!MACHINE_IS_TBBLUE) {
	        free(multiface_memory_pointer);
        }


	multiface_enabled.v=0;
}


void multiface_map_memory(void)
{

	multiface_switched_on.v=1;
	//debug_printf(VERBOSE_DEBUG,"Mapping Multiface RAM and ROM with PC=%04XH",reg_pc);

}

void multiface_unmap_memory(void)
{

        multiface_switched_on.v=0;
	//debug_printf(VERBOSE_DEBUG,"Unmapping Multiface RAM and ROM with PC=%04XH",reg_pc);

}
