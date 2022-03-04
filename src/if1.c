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
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "menu.h"

z80_bit if1_enabled={0};
z80_byte *if1_memory_pointer;
z80_bit if1_rom_paged={0};

z80_byte *if1_microdrive_buffer;
int indice_microdrive_buffer=0;

int if1_antes_pedido_longitud=-1;

#define IF1_ROM_NAME "if1-v2.rom"

/*
http://thespeccyzone.emuunlim.org/hardwarefiles/technical_files/i1_microdrives.htm

http://faqwiki.zxnet.co.uk/wiki/ZX_Interface_1

http://www.sinclair.hu/speccyalista/konyvtar/kezikonyvek/ZXInterface1_Microdrive_Manual.pdf

http://k1.spdns.de/Vintage/Sinclair/82/Peripherals/Interface%201%20and%20Microdrives%20(Sinclair)/ROMs/

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



z80_byte temp_valor_prueba;

//void cpu_core_loop_if1(void)
z80_byte cpu_core_loop_if1(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

	int despaginar=0;

	//Gestionar traps
	if (if1_rom_paged.v==0) {
		if (reg_pc==8 || reg_pc==5896) {
			printf ("paging if1 rom when pc=%d\n",reg_pc);
			if1_rom_paged.v=1;
		}
	}


	if (if1_rom_paged.v==1) {
		if (reg_pc==1792) {
                        printf ("unpaging if1 rom after pc=%d\n",reg_pc);
                        //if1_rom_paged.v=0;
			despaginar=1;
                }
		//Traps de funciones
		if (reg_pc==0x1F3F) {
			//THE 'READ SECTOR' HOOK CODE
			//Sector esta en IX+$0D  CHREC 
			printf ("Read sector routine. Sector=%d\n",peek_byte_no_time(reg_iy+0x0d) );
		}

		/*
;; CHK-PRES
L154E:  CALL    L163E           ; routine TEST-BRK allows the user to stop.

        IN      A,($EF)         ; read the microdrive port.
        AND     $04             ; test for the gap bit
        JR      NZ,L155B        ; forward, if not, to NOPRES

		*/
		if (reg_pc==0x1553) {
			printf ("CHK-PRES. Force microdrive present\n");
			//Cambiamos A. Microdrive presente
			reg_a=0;
		}

		if (reg_pc==0x15e2) {
			printf ("GET-M-HD THE 'RECEIVE BLOCK FROM MICRODRIVE HEADER' ROUTINE\n");
			//menu_abierto=1;
		}

		if (reg_pc==0x15f2) {
			printf ("GET-M-BLK\n");
			//En DE longitud
			//En stack, destino
			z80_int destino=peek_word_no_time(reg_sp);
			printf ("Destino: %d Longitud: %d\n",destino,reg_de);

			//if (reg_de!=15) sleep(3);

			//quitamos valor de la pila
			//reg_sp++;
			//reg_sp++;

			if (if1_antes_pedido_longitud>=0) {
				//Si antes era 15 y ahora 15, saltamos 528 bytes
				if (if1_antes_pedido_longitud==15 && reg_de==15) {
					indice_microdrive_buffer+=528;
					printf ("antes pedido 15 bytes. saltar 528 de datos\n");
				}
				else {
					printf ("\n");
					//sleep(3);
				}
			}

			int i;
			z80_byte caracter;

			//Si puntero esta en el ultimo sector y se habian leido antes 15 bytes
			if (indice_microdrive_buffer>=(98284-528) ) {
				printf ("indice: %d\n",indice_microdrive_buffer);
				//sleep(3);
			}

			//Debug del sector que tiene en la cabecera
			printf ("Sector en cabecera (+3): %d\n",3+if1_microdrive_buffer[indice_microdrive_buffer+1]);
			printf ("Posible sector que busca: %d\n",peek_byte_no_time(23753) );
			if (peek_byte_no_time(23753)==3+if1_microdrive_buffer[indice_microdrive_buffer+1]) {
				printf ("Match\n");
				//sleep(1);
			}

			for (i=0;i<reg_de;i++) {
				//-1 porque saltamos el byte final que indica proteccion escritura
				if (indice_microdrive_buffer>=(98284-1)) indice_microdrive_buffer=0;
				caracter=if1_microdrive_buffer[indice_microdrive_buffer++];

				poke_byte_no_time(destino+i,caracter);
				printf ("%c ",(caracter>=32 && caracter<=127 ? caracter : '.' ) );
			}
			if1_antes_pedido_longitud=reg_de;

			//Ir al final de la rutina
			reg_pc=0x1638;
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
                debug_printf (VERBOSE_DEBUG,"poke_byte_nested_handler is not enabled calling if1_set_peek_poke_functions. Enabling");
                activar=1;
        }

        else {
                //Esta activo el handler. Vamos a ver si esta activo el if1 dentro
                if (debug_nested_find_function_name(nested_list_poke_byte,"if1 poke_byte")==NULL) {
                        //No estaba en la lista
                        activar=1;
                        debug_printf (VERBOSE_DEBUG,"poke_byte_nested_handler is enabled but not found if1 poke_byte. Enabling");
                }

        }


        if (activar) {
                debug_printf (VERBOSE_DEBUG,"Setting IF1 poke / peek functions");
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
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before IF1");
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
                debug_printf (VERBOSE_DEBUG,"cpu_core_loop_nested_handler is not enabled calling if1_set_core_functions. Enabling");
                activar=1;
        }

        else {
                //Esta activo el handler. Vamos a ver si esta activo el if1 dentro
                if (debug_nested_find_function_name(nested_list_core,"if1 cpu_core_loop")==NULL) {
                        //No estaba en la lista
                        activar=1;
                        debug_printf (VERBOSE_DEBUG,"cpu_core_loop_nested_handler is enabled but not found if1 cpu_core_loop. Enabling");
                }

        }


        if (activar) {
		debug_printf (VERBOSE_DEBUG,"Setting IF1 Core loop");
		//Guardar anterior
		//cpu_core_loop_no_if1=cpu_core_loop;

		//Modificar
		//cpu_core_loop=cpu_core_loop_if1;
		if1_nested_id_core=debug_nested_core_add(cpu_core_loop_if1,"if1 cpu_core_loop");
	}
}

void if1_restore_core_function(void)
{
	debug_printf (VERBOSE_DEBUG,"Restoring original if1 core loop");
	//cpu_core_loop=cpu_core_loop_no_if1;
        debug_nested_core_del(if1_nested_id_core);
}
		
void disable_if1(void)
{
	if1_restore_core_function();
	if1_restore_peek_poke_functions();
	if1_enabled.v=0;
	free(if1_memory_pointer);
}

void enable_if1(void)
{
	if (if1_enabled.v) return;
	//Asignar memoria
        int size=8192;

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for Interface 1 emulation",size/1024);

        if1_memory_pointer=malloc(size);
        if (if1_memory_pointer==NULL) {
                cpu_panic ("No enough memory for Interface 1 emulation emulation");
        }

	//Cargar ROM
	FILE *ptr_if1_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading if1 firmware %s",IF1_ROM_NAME);
        
        open_sharedfile(IF1_ROM_NAME,&ptr_if1_romfile);
        
        
        if (ptr_if1_romfile!=NULL) {
                leidos=fread(if1_memory_pointer,1,size,ptr_if1_romfile);
                fclose(ptr_if1_romfile);
        }
        
        
        
        if (leidos!=size || ptr_if1_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading Interface 1 firmware, file " IF1_ROM_NAME );
                //Lo desactivamos asi porque el disable hace otras cosas, como cambiar el core loop, que no queremos
                if1_enabled.v=0;
                return ;
        }

	//Cargar microdrive de prueba
	if1_microdrive_buffer=malloc(256*1024);
	if (if1_microdrive_buffer==NULL) {
                cpu_panic ("No enough memory for Microdrive buffer");
	}

	FILE *ptr_microdrive_file;

	//Leer archivo mdr
        ptr_microdrive_file=fopen("prueba.mdr","rb");

	if (ptr_microdrive_file==NULL) {
		debug_printf (VERBOSE_ERR,"Cannot locate prueba.mdr");
	}

	else {
		//Leer todo el archivo microdrive de prueba
		int leidos=fread(if1_microdrive_buffer,1,98284,ptr_microdrive_file);	
		printf ("leidos %d bytes de microdrive\n",leidos);
		fclose(ptr_microdrive_file);
	}
	


	//Cambio core cpu
	if1_set_core_function();
	//Cambio peek, poke function
	if1_set_peek_poke_functions();


	if1_rom_paged.v=0;
	if1_enabled.v=1;
}
