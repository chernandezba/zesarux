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


#include "if1.h"
#include "microdrive.h"
#include "microdrive_raw.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "zxvision.h"
#include "compileoptions.h"
#include "mem128.h"
#include "screen.h"

z80_bit if1_enabled={0};
z80_byte *if1_memory_pointer;
z80_bit if1_rom_paged={0};


z80_byte interface1_last_value_port_ef=0xff;
//Guardarlo para debug
z80_byte interface1_last_read_status_ef=0xff;

z80_byte interface1_last_value_port_e7=0;
//Guardarlo para debug
z80_byte interface1_last_read_e7=0;





#define IF1_ROM_NAME "if1-v2.rom"

/*
http://thespeccyzone.emuunlim.org/hardwarefiles/technical_files/i1_microdrives.htm

http://faqwiki.zxnet.co.uk/wiki/ZX_Interface_1

http://www.sinclair.hu/speccyalista/konyvtar/kezikonyvek/ZXInterface1_Microdrive_Manual.pdf

http://k1.spdns.de/Vintage/Sinclair/82/Peripherals/Interface%201%20and%20Microdrives%20(Sinclair)/ROMs/


https://worldofspectrum.org/faq/reference/formats.htm

*/


//puntero a cpu core normal sin if1
//void (*cpu_core_loop_no_if1) (void);

//Rutinas originales antes de cambiarlas
//void (*if1_original_poke_byte)(z80_int dir,z80_byte valor);
//void (*if1_original_poke_byte_no_time)(z80_int dir,z80_byte valor);
//z80_byte (*if1_original_peek_byte)(z80_int dir);
//z80_byte (*if1_original_peek_byte_no_time)(z80_int dir);

int if1_nested_id_core;
int if1_nested_id_poke_byte;
int if1_nested_id_poke_byte_no_time;
int if1_nested_id_peek_byte;
int if1_nested_id_peek_byte_no_time;



z80_byte cpu_core_loop_if1(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

	int despaginar=0;

	//Gestionar traps
	if (if1_rom_paged.v==0) {
		if (reg_pc==8 || reg_pc==5896) {

            //Entra esa rom si esta activa rom 1 de spectrum 128

            int entra_if1_rom=0;

            if (MACHINE_IS_SPECTRUM_128_P2) {
                //Solo si estabamos en rom 1
                if ((puerto_32765 & 16) ==16) entra_if1_rom=1;
            }
            else entra_if1_rom=1;

            if (entra_if1_rom) {
                DBG_PRINT_IF1 VERBOSE_PARANOID,"IF1: paging if1 rom when pc=%d",reg_pc);
                if1_rom_paged.v=1;
            }
		}
	}


	if (if1_rom_paged.v==1) {
		if (reg_pc==1792) {
            DBG_PRINT_IF1 VERBOSE_PARANOID,"IF1: unpaging if1 rom after pc=%d",reg_pc);
            //if1_rom_paged.v=0;
            despaginar=1;
        }

        if (reg_pc==0x01b5) {
            //Asumir no formateo
            microdrive_formateando=0;

            if (reg_a==2) {
                DBG_PRINT_IF1 VERBOSE_PARANOID,"IF1: Detected FORMAT command\n");
                if (!microdrive_current_is_raw() ) microdrive_formateando=1;
            }
            else {
                //escrito_byte_info_una_vez=0;
            }

        }



    }

    //Llamar a anterior
    debug_nested_core_call_previous(if1_nested_id_core);

	if (despaginar) if1_rom_paged.v=0;

    //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
    return 0;

}

z80_byte if1_poke_byte(z80_int dir,z80_byte valor)
{
	//Como ambas som rom, esto no tiene tratamiento especial... quiza si metiesemos una ram entre 8192-16383 entonces si...
	//if1_original_poke_byte(dir,valor);
    //Llamar a anterior
    debug_nested_poke_byte_call_previous(if1_nested_id_poke_byte,dir,valor);

    //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
    return 0;
}

z80_byte if1_poke_byte_no_time(z80_int dir,z80_byte valor)
{
    //Como ambas som rom, esto no tiene tratamiento especial... quiza si metiesemos una ram entre 8192-16383 entonces si...
    //if1_original_poke_byte_no_time(dir,valor);
    //Llamar a anterior
    debug_nested_poke_byte_no_time_call_previous(if1_nested_id_poke_byte_no_time,dir,valor);


    //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
    return 0;


}

z80_byte if1_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(if1_nested_id_peek_byte,dir);


	if (if1_rom_paged.v && dir<16384) {
		//printf ("Returning peek at if1 %d 0x%04X\n",dir,dir);
		return if1_memory_pointer[dir&8191];
	}

	//return if1_original_peek_byte(dir);
	return valor_leido;
}

z80_byte if1_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(if1_nested_id_peek_byte_no_time,dir);

    if (if1_rom_paged.v && dir<16384) {
		//printf ("Returning peek at if1 %d 0x%04X\n",dir,dir);
		return if1_memory_pointer[dir&8191];
	}

        //return if1_original_peek_byte_no_time(dir);
	return valor_leido;
}



//Establecer rutinas propias
void if1_set_peek_poke_functions(void)
{

    int activar=0;

    //Ver si ya no estaban activas. Porque ? Tiene sentido esto? Esto seguramente vino de diviface.c en que a veces se llama aqui
    //estando ya la intefaz activa. Pero quiza en este if1 no sucedera nunca. Quitar esta comprobacion?
    if (poke_byte!=poke_byte_nested_handler) {
        DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: poke_byte_nested_handler is not enabled calling if1_set_peek_poke_functions. Enabling");
        activar=1;
    }

    else {
        //Esta activo el handler. Vamos a ver si esta activo el if1 dentro
        if (debug_nested_find_function_name(nested_list_poke_byte,"if1 poke_byte")==NULL) {
            //No estaba en la lista
            activar=1;
            DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: poke_byte_nested_handler is enabled but not found if1 poke_byte. Enabling");
    }

    }


    if (activar) {
        DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: Setting IF1 poke / peek functions");
        //Guardar anteriores
        //if1_original_poke_byte=poke_byte;
        //if1_original_poke_byte_no_time=poke_byte_no_time;
        //if1_original_peek_byte=peek_byte;
        //if1_original_peek_byte_no_time=peek_byte_no_time;

        //Modificar y poner las de if1
        //poke_byte=if1_poke_byte;
        //poke_byte_no_time=if1_poke_byte_no_time;
        //peek_byte=if1_peek_byte;
        //peek_byte_no_time=if1_peek_byte_no_time;


        //Asignar mediante nuevas funciones de core anidados
        if1_nested_id_poke_byte=debug_nested_poke_byte_add(if1_poke_byte,"if1 poke_byte");
        if1_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(if1_poke_byte_no_time,"if1 poke_byte_no_time");
        if1_nested_id_peek_byte=debug_nested_peek_byte_add(if1_peek_byte,"if1 peek_byte");
        if1_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(if1_peek_byte_no_time,"if1 peek_byte_no_time");

    }
}

//Restaurar rutinas de if1
void if1_restore_peek_poke_functions(void)
{
    DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: Restoring original poke / peek functions before IF1");
    //poke_byte=if1_original_poke_byte;
    //poke_byte_no_time=if1_original_poke_byte_no_time;
    //peek_byte=if1_original_peek_byte;
    //peek_byte_no_time=if1_original_peek_byte_no_time;

    debug_nested_poke_byte_del(if1_nested_id_poke_byte);
    debug_nested_poke_byte_no_time_del(if1_nested_id_poke_byte_no_time);
    debug_nested_peek_byte_del(if1_nested_id_peek_byte);
    debug_nested_peek_byte_no_time_del(if1_nested_id_peek_byte_no_time);

}

void if1_set_core_function(void)
{
    int activar=0;

    //Ver si ya no estaban activas. Porque ? Tiene sentido esto? Esto seguramente vino de diviface.c en que a veces se llama aqui
    //estando ya la intefaz activa. Pero quiza en este if1 no sucedera nunca. Quitar esta comprobacion?
    if (cpu_core_loop!=cpu_core_loop_nested_handler) {
        DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: cpu_core_loop_nested_handler is not enabled calling if1_set_core_functions. Enabling");
        activar=1;
    }

    else {
        //Esta activo el handler. Vamos a ver si esta activo el if1 dentro
        if (debug_nested_find_function_name(nested_list_core,"if1 cpu_core_loop")==NULL) {
            //No estaba en la lista
            activar=1;
            DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: cpu_core_loop_nested_handler is enabled but not found if1 cpu_core_loop. Enabling");
        }

    }


    if (activar) {
		DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: Setting IF1 Core loop");
		//Guardar anterior
		//cpu_core_loop_no_if1=cpu_core_loop;

		//Modificar
		//cpu_core_loop=cpu_core_loop_if1;
		if1_nested_id_core=debug_nested_core_add(cpu_core_loop_if1,"if1 cpu_core_loop");
	}
}

void if1_restore_core_function(void)
{
	DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: Restoring original if1 core loop");
	//cpu_core_loop=cpu_core_loop_no_if1;
    debug_nested_core_del(if1_nested_id_core);
}

void disable_if1(void)
{
    DBG_PRINT_IF1 VERBOSE_INFO,"IF1: Disabling Interface 1 emulation");

	if1_restore_core_function();
	if1_restore_peek_poke_functions();
	if1_enabled.v=0;
	free(if1_memory_pointer);
}



void enable_if1(void)
{
	if (if1_enabled.v) return;

    DBG_PRINT_IF1 VERBOSE_INFO,"IF1: Enabling Interface 1 emulation");

	//Asignar memoria
    int size=8192;

    DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: Allocating %d kb of memory for Interface 1 emulation",size/1024);

    if1_memory_pointer=util_malloc(size,"No enough memory for Interface 1 emulation emulation");


	//Cargar ROM
	FILE *ptr_if1_romfile;
    int leidos=0;

    DBG_PRINT_IF1 VERBOSE_DEBUG,"IF1: Loading if1 firmware %s",IF1_ROM_NAME);

    open_sharedfile(IF1_ROM_NAME,&ptr_if1_romfile);


    if (ptr_if1_romfile!=NULL) {
        leidos=fread(if1_memory_pointer,1,size,ptr_if1_romfile);
        fclose(ptr_if1_romfile);
    }



    if (leidos!=size || ptr_if1_romfile==NULL) {
        DBG_PRINT_IF1 VERBOSE_ERR,"IF1: Error reading Interface 1 firmware, file " IF1_ROM_NAME );
        //Lo desactivamos asi porque el disable hace otras cosas, como cambiar el core loop, que no queremos
        if1_enabled.v=0;
        return ;
    }




	//Cambio core cpu
	if1_set_core_function();
	//Cambio peek, poke function
	if1_set_peek_poke_functions();


	if1_rom_paged.v=0;
	if1_enabled.v=1;


    //Motores a 0
    int i;
    for (i=0;i<MAX_MICRODRIVES;i++) microdrive_status[i].motor_on=0;
}




z80_byte interface1_get_value_port(z80_byte puerto_l)
{
    //printf("get value port %X\n",puerto_l);

    //if (microdrive_enabled.v==0) return 0;

    //Puerto de estado
    if (puerto_l==0xef) {
        //microdrive_footer_operating();

        if (microdrive_current_is_raw()) {
            return microdrive_raw_read_port_ef();
        }
        else return microdrive_status_ef();

    }

    //Leer byte
    if (puerto_l==0xe7) {

        microdrive_footer_operating();

        z80_byte value;

        if (microdrive_current_is_raw()) {
            value=microdrive_raw_read_port_e7();
        }


        else value=mdr_next_byte();

        interface1_last_read_e7=value;

        return value;
    }

    if (puerto_l==0xf7) {
        //printf("Interface1 net read\n");
        return 0;
    }


    return 0;

}

void interface1_write_value_port(z80_byte puerto_l,z80_byte value)
{
    //Escribir byte
    if (puerto_l==0xe7) {
        interface1_last_value_port_e7=value;
        DBG_PRINT_IF1 VERBOSE_PARANOID,"IF1: Saving byte to port E7 value %02XH",value);
        microdrive_footer_operating();

        if (microdrive_current_is_raw()) mdr_raw_write_byte(value);
        else mdr_write_byte(value);
    }

    if (puerto_l==0xef) {
        microdrive_write_port_ef(value);
    }

    if (puerto_l==0xf7) {
        //printf("Interface1 net write\n");
        //printf("Write to port F7 value %02XH\n",value);
    }

}

//
// Para gestionar microdrive en formato raw
//

//Valor de t_estados anterior
int interface1_estados_anterior=0;

//Cuantos t_estados han pasado desde el ultimo avance del microdrive
int interface1_transcurrido_desde_ultimo_movimiento=0;

void interface1_count_tstates(void)
{
    if (if1_enabled.v==0) return;

    if (!microdrive_current_is_raw()) return;

    int microdrive_activo=microdrive_primer_motor_activo();
    //Si no hay ninguno activo, nada
    if (microdrive_activo<0) return;

    if (!microdrive_status[microdrive_activo].motor_on) return;

    int delta_testados;

    //Cada 168 t-estados, avanzar un byte
    if (t_estados>interface1_estados_anterior) {
        delta_testados=t_estados-interface1_estados_anterior;
    }

    else {
        //ha dado la vuelta
        //printf("ha dado la vuelta\n");
        delta_testados=screen_testados_total-interface1_estados_anterior;
        //printf("parcial delta: %5d t_estados: %d screen_testados_total: %d\n",delta_testados,t_estados,screen_testados_total);
        delta_testados +=t_estados;
    }

    //if (t_estados<100 || t_estados>70000) printf("delta: %5d t_estados: %d\n",delta_testados,t_estados);

    interface1_transcurrido_desde_ultimo_movimiento +=delta_testados;

    if (interface1_transcurrido_desde_ultimo_movimiento>=168) {
        interface1_transcurrido_desde_ultimo_movimiento -=168;
        //printf("mover cabezal en testados: %d testados_parc: %d interface1_transcurrido_desde_ultimo_movimiento: %d\n",t_estados,debug_t_estados_parcial,interface1_transcurrido_desde_ultimo_movimiento);
        microdrive_raw_move();
    }



    interface1_estados_anterior=t_estados;
}
