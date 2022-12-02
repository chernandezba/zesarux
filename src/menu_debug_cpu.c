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
   Menu Debug CPU
*/

//
// Archivo solo para el menu Debug CPU y submenus de Debug CPU
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "zxvision.h"
#include "menu_debug_cpu.h"
#include "menu_items.h"
#include "cpu.h"
#include "debug.h"
#include "operaciones.h"
#include "scmp.h"
#include "m68k.h"
#include "zx8081.h"
#include "z88.h"
#include "tbblue.h"
#include "prism.h"
#include "settings.h"
#include "screen.h"
#include "joystick.h"
#include "ula.h"
#include "timer.h"
#include "audio.h"
#include "disassemble.h"
#include "ay38912.h"
#include "realjoystick.h"
#include "utils_text_adventure.h"

//Opciones seleccionadas de menus
int mem_breakpoints_opcion_seleccionada=0;
int breakpoints_opcion_seleccionada=0;
int menu_watches_opcion_seleccionada=0;
int daad_tipo_mensaje_opcion_seleccionada=0;


//Indice a donde apunta el run backwards. El 0 sera el mas reciente
int indice_debug_cpu_backwards_history=0; 


int menu_debug_registers_print_main_step(zxvision_window *ventana);

void textadv_map_putpixel(zxvision_window *w,int x,int y,int color);


menu_z80_moto_int menu_debug_disassemble_bajar(menu_z80_moto_int dir_inicial)
{
	//Bajar 1 opcode en el listado
	        char buffer[32];
        size_t longitud_opcode;

	debugger_disassemble(buffer,30,&longitud_opcode,dir_inicial);

	dir_inicial +=longitud_opcode;

	return dir_inicial;
}



menu_z80_moto_int menu_debug_disassemble_subir(menu_z80_moto_int dir_inicial)
{
	//Subir 1 opcode en el listado

	//Metodo:
	//Empezamos en direccion-10 (en QL: direccion-30)
	//inicializamos un puntero ficticio de direccion a 0, mientras que mantenemos la posicion de memoria de lectura inicial en direccion-10/30
	//Vamos leyendo opcodes. Cuando el puntero ficticio este >=10 (o 30), nuestra direccion final será la inicial - longitud opcode anterior

	char buffer[32];
	size_t longitud_opcode;

	menu_z80_moto_int dir;

	int decremento=10;

	if (CPU_IS_MOTOROLA) decremento=30; //En el caso de motorola mejor empezar antes


	dir=dir_inicial-decremento;

	dir=menu_debug_hexdump_adjusta_en_negativo(dir,1);

	//menu_z80_moto_int dir_anterior=dir;

	int puntero_ficticio=0;


	do {

		//dir_anterior=dir;

		debugger_disassemble(buffer,30,&longitud_opcode,dir);
		
		dir+=longitud_opcode;
		dir=adjust_address_memory_size(dir);
		puntero_ficticio+=longitud_opcode;

		//printf ("dir %X dir_anterior %X puntero_ficticio %d\n",dir,dir_anterior,puntero_ficticio);

		if (puntero_ficticio>=decremento) {
			return menu_debug_hexdump_adjusta_en_negativo(dir_inicial-longitud_opcode,1);
		}
		

	} while (1);


}

//Desensamblando usando un maximo de 64 caracteres
void menu_debug_dissassemble_una_inst_sino_hexa(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode,int sino_hexa,int full_hexa_dump_motorola)
{

	char buf_temp_dir[65];
	char buf_temp_hexa[65];
	char buf_temp_opcode[65];

	size_t longitud_opcode;

	//int full_hexa_dump_motorola=1;

	//Direccion

	dir=adjust_address_memory_size(dir);


	//Texto direccion
	menu_debug_print_address_memory_zone(buf_temp_dir,dir);

	int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

	//metemos espacio en 0 final
	dumpassembler[longitud_direccion]=' ';

	int max_longitud_volcado_hexa=8;

	//Hasta instrucciones de 8 bytes si se indica full dump
	//Si no, como maximo mostrara 4 bytes (longitud hexa=8)
	//El full dump solo aparece en menu disassemble, pero no en debug cpu
	if (CPU_IS_MOTOROLA && full_hexa_dump_motorola) max_longitud_volcado_hexa=16;


	//Texto opcode
	debugger_disassemble(buf_temp_opcode,64,&longitud_opcode,dir);


	//Texto volcado hexa
	//Primero meter espacios hasta limite 64
	int i;
	for (i=0;i<64;i++) {
		buf_temp_hexa[i]=' ';
	}

	buf_temp_hexa[i]=0;

	menu_debug_registers_dump_hex(buf_temp_hexa,dir,longitud_opcode);
	int longitud_texto_hex=longitud_opcode*2;
	//quitar el 0 final
	buf_temp_hexa[longitud_texto_hex]=' ';


	//agregar un espacio final para poder meter "+" en caso necesario, esto solo sucede en Motorola
	if (CPU_IS_MOTOROLA) {
		buf_temp_hexa[max_longitud_volcado_hexa]=' ';
		buf_temp_hexa[max_longitud_volcado_hexa+1]=0;
	}

	else {
		//Meter el 0 final donde diga el limite de volcado
		buf_temp_hexa[max_longitud_volcado_hexa]=0;
	}

	//Si meter +
	if (longitud_texto_hex>max_longitud_volcado_hexa) {
		buf_temp_hexa[max_longitud_volcado_hexa]='+';
	}


	//Montar todo
	if (sino_hexa) {
		sprintf(dumpassembler,"%s %s %s",buf_temp_dir,buf_temp_hexa,buf_temp_opcode);
	}

	else {
		sprintf(dumpassembler,"%s %s",buf_temp_dir,buf_temp_opcode);
	}

	*longitud_final_opcode=longitud_opcode;

}


void menu_debug_dissassemble_una_instruccion(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode)
{
	menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,dir,longitud_final_opcode,1,0);
}



int menu_debug_show_memory_zones=0;
int menu_debug_memory_zone=-1;
menu_z80_moto_int menu_debug_memory_zone_size=65536;


//
// Inicio funciones de gestion de zonas de memoria
//

void menu_debug_set_memory_zone_attr(void)
{

	int readwrite;

	if (menu_debug_show_memory_zones==0) {
		menu_debug_memory_zone_size=65536;
		if (MACHINE_IS_QL) menu_debug_memory_zone_size=QL_MEM_LIMIT+1;
		return;
	}

	//Primero ver si zona actual no esta disponible, fallback a 0 que siempre esta
	 menu_debug_memory_zone_size=machine_get_memory_zone_attrib(menu_debug_memory_zone,&readwrite);
	if (!menu_debug_memory_zone_size) {
		//printf ("Zona no disponible. Fallback a memory mapped\n");
		menu_debug_set_memory_zone_mapped();
		//menu_debug_memory_zone=0;
		//menu_debug_memory_zone_size=machine_get_memory_zone_attrib(menu_debug_memory_zone,&readwrite);
	}
}

//Muestra byte mapeado de ram normal o de zona de menu mapeada
z80_byte menu_debug_get_mapped_byte(int direccion)
{

	//Mostrar memoria normal
	if (menu_debug_show_memory_zones==0) {
		//printf ("menu_debug_get_mapped_byte dir %04XH result %02XH\n",direccion,peek_byte_z80_moto(direccion));
		return peek_byte_z80_moto(direccion);
	}


	//Mostrar zonas mapeadas
	//printf ("menu_debug_get_mapped_byte 1\n");
	menu_debug_set_memory_zone_attr();

	//Aqui si se ha hecho fallback a mapped zone, recomprobar de nuevo
	if (menu_debug_show_memory_zones==0) {
		//printf ("menu_debug_get_mapped_byte dir %04XH result %02XH\n",direccion,peek_byte_z80_moto(direccion));
		//printf ("menu_debug_get_mapped_byte 1.5\n");
		return peek_byte_z80_moto(direccion);
	}	

	//printf ("menu_debug_get_mapped_byte 2\n");

	//printf ("menu_debug_get_mapped_byte menu_debug_memory_zone_size: %d\n",menu_debug_memory_zone_size);

	direccion=direccion % menu_debug_memory_zone_size;
	//printf ("menu_debug_get_mapped_byte 3\n");
	return *(machine_get_memory_zone_pointer(menu_debug_memory_zone,direccion));
	


}




//Escribe byte mapeado de ram normal o de zona de menu mapeada
void menu_debug_write_mapped_byte(int direccion,z80_byte valor)
{



	//Mostrar memoria normal
	if (menu_debug_show_memory_zones==0) {
		return poke_byte_z80_moto(direccion,valor);
	}


	//Mostrar zonas mapeadas
	menu_debug_set_memory_zone_attr();


	//Aqui si se ha hecho fallback a mapped zone, recomprobar de nuevo
	if (menu_debug_show_memory_zones==0) {
		return poke_byte_z80_moto(direccion,valor);
	}	

	direccion=direccion % menu_debug_memory_zone_size;
	*(machine_get_memory_zone_pointer(menu_debug_memory_zone,direccion))=valor;



}


menu_z80_moto_int adjust_address_memory_size(menu_z80_moto_int direccion)
{

	//Si modo mapeo normal
	if (menu_debug_show_memory_zones==0) {
		return adjust_address_space_cpu(direccion);
	}

	//Si zonas memoria mapeadas
	if (direccion>=menu_debug_memory_zone_size) {
		//printf ("ajustamos direccion %x a %x\n",direccion,menu_debug_memory_zone_size);
		direccion=direccion % menu_debug_memory_zone_size;
		//printf ("resultado ajustado: %x\n",direccion);
	}

	return direccion;
}


void menu_debug_set_memory_zone_mapped(void)
{
		menu_debug_memory_zone=-1;
		menu_debug_show_memory_zones=0;	
		menu_debug_memory_zone_size=65536;
}


//Retorna -1 si mapped memory. 0 o en adelante si otros. -2 si ESC
int menu_change_memory_zone_list_title(char *titulo)
{

        menu_item *array_menu_memory_zones;
        menu_item item_seleccionado;
        int retorno_menu;
		int menu_change_memory_zone_list_opcion_seleccionada=0;
        //do {

                char buffer_texto[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];

				

				menu_add_item_menu_inicial_format(&array_menu_memory_zones,MENU_OPCION_NORMAL,NULL,NULL,"Mapped memory");
				menu_add_item_menu_valor_opcion(array_menu_memory_zones,-1);

                int zone=-1;
				int i=1;
                do {

					zone++;
					zone=machine_get_next_available_memory_zone(zone);
					if (zone>=0) {
						machine_get_memory_zone_name(zone,buffer_texto);
						menu_add_item_menu_format(array_menu_memory_zones,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
						menu_add_item_menu_valor_opcion(array_menu_memory_zones,zone);

						if (menu_debug_memory_zone==zone) menu_change_memory_zone_list_opcion_seleccionada=i;

					}
					i++;
				} while (zone>=0);


                menu_add_item_menu(array_menu_memory_zones,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                
                menu_add_ESC_item(array_menu_memory_zones);

                retorno_menu=menu_dibuja_menu(&menu_change_memory_zone_list_opcion_seleccionada,&item_seleccionado,array_menu_memory_zones,titulo );

                


				if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
						//Cambiamos la zona
						int valor_opcion=item_seleccionado.valor_opcion;
						return valor_opcion;
                    
                }

	return -2;

}

int menu_change_memory_zone_list(void)
{
  return menu_change_memory_zone_list_title("Zones");
 }

void menu_set_memzone(int valor_opcion)
{
if (valor_opcion<0) {
		menu_debug_set_memory_zone_mapped();
	}
	else {
		menu_debug_show_memory_zones=1;
		menu_debug_memory_zone=valor_opcion;
	}	
}

void menu_debug_change_memory_zone(void)
{
	int valor_opcion=menu_change_memory_zone_list();
	if (valor_opcion==-2) return; //Pulsado ESC
	
	menu_set_memzone(valor_opcion);


	/*if (menu_debug_show_memory_zones==0) menu_debug_show_memory_zones=1;

	//Si se ha habilitado en el if anterior, entrara aqui
	if (menu_debug_show_memory_zones) {
		menu_debug_memory_zone++;
		menu_debug_memory_zone=machine_get_next_available_memory_zone(menu_debug_memory_zone);
		if (menu_debug_memory_zone<0)  {
			menu_debug_set_memory_zone_mapped();

		}
	}
	*/
}

void menu_debug_change_memory_zone_non_interactive(void)
{


	if (menu_debug_show_memory_zones==0) menu_debug_show_memory_zones=1;

	//Si se ha habilitado en el if anterior, entrara aqui
	if (menu_debug_show_memory_zones) {
		menu_debug_memory_zone++;
		menu_debug_memory_zone=machine_get_next_available_memory_zone(menu_debug_memory_zone);
		if (menu_debug_memory_zone<0)  {
			menu_debug_set_memory_zone_mapped();

		}
	}
	
}

void menu_debug_set_memory_zone(int zone)
{
	//Cambiar a zona memoria indicada
	int salir=0;

	//int zona_inicial=menu_debug_memory_zone;

	while (menu_debug_memory_zone!=zone && salir<2) {
		menu_debug_change_memory_zone_non_interactive();

		//Si ha pasado dos veces por la zona mapped, es que no existe dicha zona
		if (menu_debug_memory_zone<0) salir++;
	}
}

int menu_get_current_memory_zone_name_number(char *s)
{
	if (menu_debug_show_memory_zones==0) {
		strcpy(s,"Mapped memory");
		return -1;
	}

	machine_get_memory_zone_name(menu_debug_memory_zone,s);
	return menu_debug_memory_zone;
}

//Retorna el numero de digitos para representar un numero en hexadecimal
int menu_debug_get_total_digits_hexa(int valor)
{
	char temp_digitos[20];
	sprintf (temp_digitos,"%X",valor);
	return strlen(temp_digitos);
}

//Retorna el numero de digitos para representar un numero en decimal
int menu_debug_get_total_digits_dec(int valor)
{
	char temp_digitos[20];
	sprintf (temp_digitos,"%d",valor);
	return strlen(temp_digitos);
}

//Escribe una direccion en texto, en hexa, teniendo en cuenta zona memoria (rellenando espacios segun tamanyo zona)
void menu_debug_print_address_memory_zone(char *texto, menu_z80_moto_int address)
{
	//primero meter 6 espacios
	sprintf (texto,"      ");

	address=adjust_address_memory_size(address);
	//int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

	//Obtener cuantos digitos hexa se necesitan
	//char temp_digitos[20];
	//sprintf (temp_digitos,"%X",menu_debug_memory_zone_size-1);
	//int digitos=strlen(temp_digitos);

	int digitos=menu_debug_get_total_digits_hexa(menu_debug_memory_zone_size-1);

	//Obtener posicion inicial a escribir direccion. Suponemos maximo 6
	int posicion_inicial_digitos=6-digitos;


	//Escribimos direccion
	sprintf (&texto[posicion_inicial_digitos],"%0*X",digitos,address);
}


//
// Fin funciones de gestion de zonas de memoria
//



void menu_mem_breakpoints_edit(MENU_ITEM_PARAMETERS)
{


        int brkp_type,dir;

        char string_type[4];
        char string_dir[10];

        strcpy (string_dir,"0");

        menu_ventana_scanf("Address",string_dir,10);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }				

        strcpy (string_type,"0");

        menu_ventana_scanf("Type (1:RD,2:WR,3:RW)",string_type,4);

        brkp_type=parse_string_to_number(string_type);

        if (brkp_type<0 || brkp_type>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",brkp_type);
                return;
        }

	debug_set_mem_breakpoint(dir,brkp_type);
	//mem_breakpoint_array[dir]=brkp_type;
	

}

void menu_mem_breakpoints_list(MENU_ITEM_PARAMETERS)
{

        //int index_find;
		int index_buffer;

        char *results_buffer=util_malloc_max_texto_generic_message("Can not allocate memory for breakpoints");

        //margen suficiente para que quepa una linea
        //direccion+salto linea+codigo 0
        char buf_linea[33];

        index_buffer=0;

        int encontrados=0;

        int salir=0;

		int i;

        for (i=0;i<65536 && salir==0;i++) {
			z80_byte tipo=mem_breakpoint_array[i];
			if (tipo) {
				if (tipo<MAX_MEM_BREAKPOINT_TYPES) {
					sprintf (buf_linea,"%04XH : %s\n",i,mem_breakpoint_types_strings[tipo]);
				}
				else {
					sprintf (buf_linea,"%04XH : Unknown (%d)\n",i,tipo);
				}

				sprintf (&results_buffer[index_buffer],"%s\n",buf_linea);
                index_buffer +=strlen(buf_linea);
                encontrados++;
                

                //controlar maximo
                //33 bytes de margen
                if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-33) {
                        debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first %d",encontrados);
                        //forzar salir
                        salir=1;
                }
			}

        }

        results_buffer[index_buffer]=0;

        menu_generic_message("List Memory Breakpoints",results_buffer);

        free(results_buffer);
}

void menu_mem_breakpoints_clear(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Clear Mem breakpoints")) {
		clear_mem_breakpoints();
		menu_generic_message("Clear Mem breakpoints","OK. All memory breakpoints cleared");
	}
}


void menu_clear_all_breakpoints(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Clear breakpoints")) {
		init_breakpoints_table();
		menu_generic_message("Clear breakpoints","OK. All breakpoints cleared");
	}
}

void menu_mem_breakpoints(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();

        menu_item *array_menu_mem_breakpoints;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_edit,NULL,"~~Edit Breakpoint");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'e');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Edit Breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Edit Breakpoints");

		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_list,NULL,"~~List breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'l');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"List breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"List enabled memory breakpoints");


		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_clear,NULL,"~~Clear breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'c');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Clear all memory breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Clear all memory breakpoints");


                menu_add_item_menu(array_menu_mem_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_mem_breakpoints);
                retorno_menu=menu_dibuja_menu(&mem_breakpoints_opcion_seleccionada,&item_seleccionado,array_menu_mem_breakpoints,"Memory Breakpoints" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


int menu_breakpoints_cond(void)
{
	return debug_breakpoints_enabled.v;
}



void menu_breakpoints_conditions_set(MENU_ITEM_PARAMETERS)
{
        //printf ("linea: %d\n",breakpoints_opcion_seleccionada);

	//saltamos los breakpoints de registro pc y la primera linea
        //int breakpoint_index=breakpoints_opcion_seleccionada-MAX_BREAKPOINTS-1;

	//saltamos las primeras 2 lineas
	//int breakpoint_index=breakpoints_opcion_seleccionada-2;

	int breakpoint_index=valor_opcion;

  char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];

			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[breakpoint_index],string_texto,MAX_PARSER_TOKENS_NUM);
			
			

  menu_ventana_scanf("Condition",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

  debug_set_breakpoint(breakpoint_index,string_texto);

	//comprobar error
	if (if_pending_error_message) {
		menu_muestra_pending_error_message(); //Si se genera un error derivado del set breakpoint, mostrarlo y salir
		return;
	}


	sprintf (string_texto,"%s",debug_breakpoints_actions_array[breakpoint_index]);

  menu_ventana_scanf("Action? (enter=normal)",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

  debug_set_breakpoint_action(breakpoint_index,string_texto);

}

/*
void menu_breakpoints_condition_evaluate(MENU_ITEM_PARAMETERS)
{

        char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];
	string_texto[0]=0;

        menu_ventana_scanf("Condition",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

        int result=debug_breakpoint_condition_loop(string_texto,1);

        menu_generic_message_format("Result","%s -> %s",string_texto,(result ? "True" : "False " ));
}
*/

void menu_breakpoints_condition_evaluate_new(MENU_ITEM_PARAMETERS)
{

        char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];
	string_texto[0]=0;

        menu_ventana_scanf("Expression",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);


        //menu_generic_message_format("Result","%s -> %s",string_texto,(result ? "True" : "False " ));


	//int exp_par_evaluate_expression(char *entrada,char *salida)
	char buffer_salida[256]; //mas que suficiente
	char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

	int result=exp_par_evaluate_expression(string_texto,buffer_salida,string_detoken);
	if (result==0) {
		menu_generic_message_format("Result","Parsed string: %s\nResult: %s",string_detoken,buffer_salida);		
	}

	else if (result==1) {
		menu_error_message(buffer_salida);
	}

	else {
		menu_generic_message_format("Error","%s parsed string: %s",buffer_salida,string_detoken);
	}

	
}




void menu_breakpoints_enable_disable(MENU_ITEM_PARAMETERS)
{
        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

		breakpoints_enable();
        }


        else {
                debug_breakpoints_enabled.v=0;

		breakpoints_disable();
        }

}


void menu_breakpoints_condition_enable_disable(MENU_ITEM_PARAMETERS)
{
	debug_breakpoints_conditions_toggle(valor_opcion);

}




void menu_breakpoints(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();

        menu_item *array_menu_breakpoints;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_enable_disable,NULL,"~~Breakpoints: %s",
			(debug_breakpoints_enabled.v ? "On" : "Off") );
		menu_add_item_menu_shortcut(array_menu_breakpoints,'b');
		menu_add_item_menu_tooltip(array_menu_breakpoints,"Enable Breakpoints. All breakpoint types depend on this setting");
		menu_add_item_menu_ayuda(array_menu_breakpoints,"Enable Breakpoints. All breakpoint types depend on this setting");

		//char buffer_texto[40];

                int i;




		menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_condition_evaluate_new,NULL,"~~Evaluate Expression");
		menu_add_item_menu_shortcut(array_menu_breakpoints,'e');
		menu_add_item_menu_tooltip(array_menu_breakpoints,"Evaluate expression using parser");
		menu_add_item_menu_ayuda(array_menu_breakpoints,"Evaluate expression using parser. It's the same parser as breakpoint conditions below");


		menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints,NULL,"~~Memory breakpoints");
		menu_add_item_menu_shortcut(array_menu_breakpoints,'m');

		menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_clear_all_breakpoints,NULL,"Clear all breakpoints");



        for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
            #define LENGTH_STRING_CONDITION_SHOWN 30
			char string_condition_shown[LENGTH_STRING_CONDITION_SHOWN];

            #define LENGTH_STRING_ACTION_SHOWN 20
			char string_action_shown[LENGTH_STRING_ACTION_SHOWN];

            //sumar los dos, agregar caracteres ->
            #define LENGTH_STRING_CONDITION_ACTION (LENGTH_STRING_CONDITION_SHOWN+LENGTH_STRING_ACTION_SHOWN+3)
			char string_condition_action[LENGTH_STRING_CONDITION_ACTION];

			

			if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {
			
				//nuevo parser de breakpoints
				char buffer_temp_breakpoint[MAX_BREAKPOINT_CONDITION_LENGTH];
				exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp_breakpoint,MAX_PARSER_TOKENS_NUM);


				menu_tape_settings_trunc_name(buffer_temp_breakpoint,string_condition_shown,LENGTH_STRING_CONDITION_SHOWN);
				
				//printf ("brkp %d [%s]\n",i,string_condition_shown);

				menu_tape_settings_trunc_name(debug_breakpoints_actions_array[i],string_action_shown,LENGTH_STRING_ACTION_SHOWN);
				if (debug_breakpoints_actions_array[i][0]) sprintf (string_condition_action,"%s->%s",string_condition_shown,string_action_shown);

				//Si accion es menu, no escribir, para que quepa bien en pantalla
				//else sprintf (string_condition_action,"%s->menu",string_condition_shown);
				else sprintf (string_condition_action,"%s",string_condition_shown);
			}
			else {
				sprintf(string_condition_action,"None");
			}

            #define LENGTH_STRING_CONDITION_ACTION_SHOWN 40
			char string_condition_action_shown[LENGTH_STRING_CONDITION_ACTION_SHOWN];
			menu_tape_settings_trunc_name(string_condition_action,string_condition_action_shown,LENGTH_STRING_CONDITION_ACTION_SHOWN);

																																																										//0123456789012345678901234567890
			if (debug_breakpoints_conditions_enabled[i]==0 || debug_breakpoints_enabled.v==0) {														//Di 12345678901234: 12345678
				menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_conditions_set,menu_breakpoints_cond,
                    "Di %d: %s",i+1,string_condition_action_shown);
			}
            
			else {
				menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_conditions_set,menu_breakpoints_cond,
                    "En %d: %s",i+1,string_condition_action_shown);
			}

           
            menu_add_item_menu_tooltip(array_menu_breakpoints,"Set a condition breakpoint. Press Space to disable or enable");

			menu_add_item_menu_espacio(array_menu_breakpoints,menu_breakpoints_condition_enable_disable);

			menu_add_item_menu_valor_opcion(array_menu_breakpoints,i);

			menu_add_item_menu_ayuda(array_menu_breakpoints,"Set a condition breakpoint and its action. Press Space to disable or enable.\n"
						HELP_MESSAGE_CONDITION_BREAKPOINT
						"\n\n\n"
						HELP_MESSAGE_BREAKPOINT_ACTION

					);

        }

		//menu_add_item_menu(array_menu_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);






        menu_add_item_menu(array_menu_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);
        menu_add_ESC_item(array_menu_breakpoints);
        retorno_menu=menu_dibuja_menu(&breakpoints_opcion_seleccionada,&item_seleccionado,array_menu_breakpoints,"Breakpoints" );

                

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                        
                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



//Vuelca contenido hexa o decimal de memoria de spectrum en cadena de texto, finalizando con 0 la cadena de texto
void menu_debug_registers_dump_hex_decimal(char *texto,menu_z80_moto_int direccion,int longitud,int decimal)
{

	z80_byte byte_leido;

	int puntero=0;

	for (;longitud>0;longitud--) {
		
		direccion=adjust_address_memory_size(direccion);

        byte_leido=menu_debug_get_mapped_byte(direccion);

        direccion++;


        if (decimal) {
            sprintf (&texto[puntero],"%03d ",byte_leido);
            puntero+=4;
        }
		else {
            sprintf (&texto[puntero],"%02X",byte_leido);

		    puntero+=2;
        }

	}
}

//Vuelca contenido hexa de memoria de spectrum en cadena de texto, finalizando con 0 la cadena de texto
void menu_debug_registers_dump_hex(char *texto,menu_z80_moto_int direccion,int longitud)
{
    menu_debug_registers_dump_hex_decimal(texto,direccion,longitud,0);
}

//Vuelca contenido decimal de memoria de spectrum en cadena de texto, finalizando con 0 la cadena de texto
void menu_debug_registers_dump_decimal(char *texto,menu_z80_moto_int direccion,int longitud)
{
	menu_debug_registers_dump_hex_decimal(texto,direccion,longitud,1);
}

//Vuelca contenido ascii de memoria de spectrum en cadena de texto
//modoascii: 0: normal. 1:zx80. 2:zx81
void menu_debug_registers_dump_ascii(char *texto,menu_z80_moto_int direccion,int longitud,int modoascii,z80_byte valor_xor)
{

        z80_byte byte_leido;

        int puntero=0;
				//printf ("dir ascii: %d\n",direccion);

        for (;longitud>0;longitud--) {
							//direccion=adjust_address_space_cpu(direccion);
							direccion=adjust_address_memory_size(direccion);

                //Si mostramos RAM oculta de Inves
                //if (MACHINE_IS_INVES && menu_debug_hex_shows_inves_low_ram.v) {
                //        byte_leido=memoria_spectrum[direccion++];
                //}

                //else {
									//byte_leido=peek_byte_z80_moto(direccion);
									byte_leido=menu_debug_get_mapped_byte(direccion) ^ valor_xor;
									direccion++;
								//}



		if (modoascii==0) {
		if (byte_leido<32 || byte_leido>126) byte_leido='.';
		}

		else if (modoascii==1) {
			if (byte_leido>=64) byte_leido='.';
			else byte_leido=da_codigo_zx80_no_artistic(byte_leido);
		}

		else {
			if (byte_leido>=64) byte_leido='.';
                        else byte_leido=da_codigo_zx81_no_artistic(byte_leido);
                }


                sprintf (&texto[puntero],"%c",byte_leido);

                puntero+=1;

        }
}

//Retorna paginas mapeadas (nombres cortos) 
void menu_debug_get_memory_pages(char *s)
{
	debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
        int total_segmentos=debug_get_memory_pages_extended(segmentos);

        int i;
        int longitud;
        int indice=0;

        for (i=0;i<total_segmentos;i++) { 
        	longitud=strlen(segmentos[i].shortname)+1;
        	sprintf(&s[indice],"%s ",segmentos[i].shortname);

        	indice +=longitud;

        }
		
}


//Si muestra:
/*
//1=14 lineas assembler con registros a la derecha
//2=linea assembler, registros cpu, otros registros internos
//3=9 lineas assembler, otros registros internos
//4=14 lineas assembler
//5=9 lineas hexdump, otros registros internos  
//6=14 lineas hexdump   
//7=vista minima con ventana pequeña
//8=vista debug quill/paws/daad
*/
//
  
int menu_debug_registers_current_view=1;


//Ultima direccion mostrada en menu_disassemble
menu_z80_moto_int menu_debug_disassemble_last_ptr=0;

//const int menu_debug_num_lineas_full=14;

//Retorna total de lineas de debug de desensamblado
int get_menu_debug_num_lineas_full(zxvision_window *w)
{
	//return 13;

	//24->13
    //Para una altura por defecto de 24, devolvera 13 lineas de desensamblado
	int lineas=w->visible_height-11;

	if (lineas<2) lineas=2;

	return lineas;
}

int get_menu_debug_columna_registros(zxvision_window *w)
{
    //A partir de que columna aparecen los registros a la derecha
    //dependera del tamaño de la ventana
    int columna_registros;


    columna_registros=w->visible_width-13;   //32-13
    if (CPU_IS_MOTOROLA) columna_registros=w->visible_width-12; //32-12


    //Revisar un minimo y maximo
    if (columna_registros<19) columna_registros=19;

    //20 caracteres dan mas que de sobra para el texto de registros
    if (columna_registros>MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-20) columna_registros=MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-20;

    return columna_registros;

}


void menu_debug_registers_print_register_aux_moto(zxvision_window *w,char *textoregistros,int *linea,int numero,m68k_register_t registro_direccion,m68k_register_t registro_dato)
{

	sprintf (textoregistros,"A%d: %08X D%d: %08X",numero,m68k_get_reg(NULL, registro_direccion),numero,m68k_get_reg(NULL, registro_dato) );
	//menu_escribe_linea_opcion(*linea,-1,1,textoregistros);
	zxvision_print_string_defaults_fillspc(w,1,*linea,textoregistros);
	(*linea)++;

}

z80_bit menu_debug_follow_pc={1}; //Si puntero de direccion sigue al registro pc
menu_z80_moto_int menu_debug_memory_pointer=0; //Puntero de direccion

//linea en menu debug que tiene el cursor (indicado por >), desde 0 hasta 23 como mucho
int menu_debug_line_cursor=0;

char menu_debug_change_registers_last_reg[30]="";
char menu_debug_change_registers_last_val[30]="";


void menu_debug_change_registers(void)
{
	char string_registervalue[61]; //REG=VALUE

	menu_ventana_scanf("Register?",menu_debug_change_registers_last_reg,30);

	menu_ventana_scanf("Value?",menu_debug_change_registers_last_val,30);

	sprintf (string_registervalue,"%s=%s",menu_debug_change_registers_last_reg,menu_debug_change_registers_last_val);

	if (debug_change_register(string_registervalue)) {
        //Si lanzo con debug_print ERR, y estamos en modo step, se habilita multitarea, por que?
		//debug_printf(VERBOSE_ERR,"Error changing register");
        menu_error_message("Error changing register");
    }
}



void menu_debug_registers_change_ptr(void)
{



        char string_address[10];


                                        util_sprintf_address_hex(menu_debug_memory_pointer,string_address);
                menu_ventana_scanf("Address?",string_address,10);

        menu_debug_memory_pointer=parse_string_to_number(string_address);


        return;

}


#define MOD_REG_A 1
#define MOD_REG_F           (1<<1)
#define MOD_REG_AF          (MOD_REG_A|MOD_REG_F)
#define MOD_REG_AF_SHADOW   (1<<2)

#define MOD_REG_B           (1<<3)
#define MOD_REG_C           (1<<4)
#define MOD_REG_BC          (MOD_REG_B|MOD_REG_C)
#define MOD_REG_BC_SHADOW   (1<<5)

#define MOD_REG_H           (1<<6)
#define MOD_REG_L           (1<<7)
#define MOD_REG_HL          (MOD_REG_H|MOD_REG_L)
#define MOD_REG_HL_SHADOW   (1<<8)

#define MOD_REG_D           (1<<9)
#define MOD_REG_E           (1<<10)
#define MOD_REG_DE          (MOD_REG_D|MOD_REG_E)
#define MOD_REG_DE_SHADOW   (1<<11)

#define MOD_REG_SP          (1<<12)
#define MOD_REG_IFF         (1<<13)
#define MOD_REG_I           (1<<14)
#define MOD_REG_R           (1<<15)
#define MOD_REG_IM_MODE     (1<<16)

#define MOD_REG_IX_L        (1<<17)
#define MOD_REG_IX_H        (1<<18)
#define MOD_REG_IX          (MOD_REG_IX_L|MOD_REG_IX_H)

#define MOD_REG_IY_L        (1<<19)
#define MOD_REG_IY_H        (1<<20)
#define MOD_REG_IY          (MOD_REG_IY_L|MOD_REG_IY_H)

//para (HL)
#define MOD_REG_HL_MEM      (1<<21)
#define MOD_REG_DE_MEM      (1<<22)
#define MOD_REG_BC_MEM      (1<<23)

//Tabla de los registros modificados en los 256 opcodes sin prefijo
z80_long_int debug_modified_registers_list[256]={
    //0 NOP
    0,MOD_REG_B|MOD_REG_C,MOD_REG_BC_MEM,MOD_REG_BC,MOD_REG_B|MOD_REG_F,MOD_REG_B|MOD_REG_F,MOD_REG_B,MOD_REG_A|MOD_REG_F,
    MOD_REG_AF|MOD_REG_AF_SHADOW,   MOD_REG_HL|MOD_REG_F,MOD_REG_A,MOD_REG_BC,MOD_REG_C|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_C,MOD_REG_A|MOD_REG_F,
    //16 DJNZ dis
    MOD_REG_B,MOD_REG_DE,MOD_REG_DE_MEM,MOD_REG_DE,MOD_REG_D|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_D,MOD_REG_A|MOD_REG_F,
    0,MOD_REG_HL|MOD_REG_F,MOD_REG_A,MOD_REG_DE,MOD_REG_E|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_E,MOD_REG_A|MOD_REG_F,
    //32 JR NZ,DIS
    0,MOD_REG_HL,0,MOD_REG_HL,MOD_REG_H|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_H,MOD_REG_A|MOD_REG_F,
    0,MOD_REG_HL|MOD_REG_F,MOD_REG_HL,MOD_REG_HL,MOD_REG_L|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_L,MOD_REG_A|MOD_REG_F,
    //48 JR NC,DIS
    0,MOD_REG_SP,0,MOD_REG_SP,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_HL_MEM,MOD_REG_F,
    0,MOD_REG_HL|MOD_REG_F,MOD_REG_A,MOD_REG_SP,MOD_REG_A|MOD_REG_F,MOD_REG_A|MOD_REG_F,MOD_REG_A,MOD_REG_F,
    //64 LD B,B
    MOD_REG_B,MOD_REG_B,MOD_REG_B,MOD_REG_B,MOD_REG_B,MOD_REG_B,MOD_REG_B,MOD_REG_B,
    MOD_REG_C,MOD_REG_C,MOD_REG_C,MOD_REG_C,MOD_REG_C,MOD_REG_C,MOD_REG_C,MOD_REG_C,
    //80 LD D,B
    MOD_REG_D,MOD_REG_D,MOD_REG_D,MOD_REG_D,MOD_REG_D,MOD_REG_D,MOD_REG_D,MOD_REG_D,
    MOD_REG_E,MOD_REG_E,MOD_REG_E,MOD_REG_E,MOD_REG_E,MOD_REG_E,MOD_REG_E,MOD_REG_E,
    //96 LD H,B
    MOD_REG_H,MOD_REG_H,MOD_REG_H,MOD_REG_H,MOD_REG_H,MOD_REG_H,MOD_REG_H,MOD_REG_H,
    MOD_REG_L,MOD_REG_L,MOD_REG_L,MOD_REG_L,MOD_REG_L,MOD_REG_L,MOD_REG_L,MOD_REG_L,
    //112 LD (HL),B
    MOD_REG_HL_MEM,MOD_REG_HL_MEM,MOD_REG_HL_MEM,MOD_REG_HL_MEM,MOD_REG_HL_MEM,MOD_REG_HL_MEM,0,MOD_REG_HL_MEM,
    MOD_REG_A,MOD_REG_A,MOD_REG_A,MOD_REG_A,MOD_REG_A,MOD_REG_A,MOD_REG_A,0,
    //128 ADD A,B
    MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,
    MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,
    //144 SUB B
    MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,
    MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,
    //160 AND B
    MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,
    MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,
    //176 OR B
    MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    //192 RET NZ
    MOD_REG_SP,MOD_REG_SP|MOD_REG_BC,0,0,MOD_REG_SP,MOD_REG_SP,MOD_REG_AF,MOD_REG_SP,
    MOD_REG_SP,MOD_REG_SP,0,0,MOD_REG_SP,MOD_REG_SP,MOD_REG_AF,MOD_REG_SP,
    //208 RET NC
    MOD_REG_SP,MOD_REG_SP|MOD_REG_DE,0,0,MOD_REG_SP,MOD_REG_SP,MOD_REG_AF,MOD_REG_SP,
    MOD_REG_SP,MOD_REG_BC|MOD_REG_DE|MOD_REG_HL|MOD_REG_BC_SHADOW|MOD_REG_DE_SHADOW|MOD_REG_HL_SHADOW,0,MOD_REG_A,MOD_REG_SP,0,MOD_REG_AF,MOD_REG_SP,
    //224 RET PO
    MOD_REG_SP,MOD_REG_SP|MOD_REG_HL,0,MOD_REG_HL,MOD_REG_SP,MOD_REG_SP,MOD_REG_AF,MOD_REG_SP,
    MOD_REG_SP,0,0,MOD_REG_DE|MOD_REG_HL,MOD_REG_SP,0,MOD_REG_AF,MOD_REG_SP,
    //240 RET P
    MOD_REG_SP,MOD_REG_SP|MOD_REG_AF,0,MOD_REG_IFF,MOD_REG_SP,MOD_REG_SP,MOD_REG_AF,MOD_REG_SP,
    MOD_REG_SP,MOD_REG_SP,0,MOD_REG_IFF,MOD_REG_SP,0,MOD_REG_F,MOD_REG_SP
};

//Tabla de los registros modificados para opcodes con prefijo CB y tambien para DD/FD + CB
z80_long_int debug_modified_registers_cb_list[256]={
    //0 RLC B
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    //16 RL B
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    //32 SLA B
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    //48 SLL B
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    MOD_REG_B|MOD_REG_F,MOD_REG_C|MOD_REG_F,MOD_REG_D|MOD_REG_F,MOD_REG_E|MOD_REG_F,MOD_REG_H|MOD_REG_F,MOD_REG_L|MOD_REG_F,MOD_REG_F|MOD_REG_HL_MEM,MOD_REG_AF,
    //64 BIT 0,B
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    //80 BIT 2,B
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    //96 BIT 4,B
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    //112 BIT 6,B
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,MOD_REG_F,
    //128 RES 0,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    //144 RES 2,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    //160 RES 4,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    //176 RES 6,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    //192 SET 0,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    //208 SET 2,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    //224 SET 4,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    //240 SET 6,B
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A,
    MOD_REG_B,MOD_REG_C,MOD_REG_D,MOD_REG_E,MOD_REG_H,MOD_REG_L,MOD_REG_HL_MEM,MOD_REG_A
};

//Tabla de los registros modificados para opcodes con prefijo ED
z80_long_int debug_modified_registers_ed_list[256]={
    //0 NOPD
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //16 NOPD
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //32 NOPD
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //48 NOPD
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //64 IN B,(C)
    MOD_REG_B|MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,0,         MOD_REG_AF,MOD_REG_SP|MOD_REG_IFF,MOD_REG_IM_MODE,MOD_REG_I,
    MOD_REG_C|MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,MOD_REG_BC,MOD_REG_AF,MOD_REG_SP,MOD_REG_IM_MODE,MOD_REG_R,
    //80 IN D,(C)
    MOD_REG_D|MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,0,         MOD_REG_AF,MOD_REG_SP|MOD_REG_IFF,MOD_REG_IM_MODE,MOD_REG_AF,
    MOD_REG_E|MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,MOD_REG_DE,MOD_REG_AF,MOD_REG_SP|MOD_REG_IFF,MOD_REG_IM_MODE,MOD_REG_AF,
    //96 IN H,(C)
    MOD_REG_H|MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,0,         MOD_REG_AF,MOD_REG_SP|MOD_REG_IFF,MOD_REG_IM_MODE,MOD_REG_AF,
    MOD_REG_L|MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,MOD_REG_HL,MOD_REG_AF,MOD_REG_SP|MOD_REG_IFF,MOD_REG_IM_MODE,MOD_REG_AF,
    //112 IN F,(C)
    MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,0,                   MOD_REG_AF,MOD_REG_SP|MOD_REG_IFF,MOD_REG_IM_MODE,0,
    MOD_REG_A|MOD_REG_F,0,MOD_REG_HL|MOD_REG_F,MOD_REG_SP,MOD_REG_AF,MOD_REG_SP|MOD_REG_IFF,MOD_REG_IM_MODE,0,
    //128
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //144
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,

    //160 LDI, CPI
    MOD_REG_BC|MOD_REG_DE|MOD_REG_HL|MOD_REG_F|MOD_REG_DE_MEM, MOD_REG_BC|MOD_REG_HL|MOD_REG_F, 
    //INI, OUTI, NOP, NOP, NOP
    MOD_REG_B|MOD_REG_HL|MOD_REG_F|MOD_REG_HL_MEM, MOD_REG_B|MOD_REG_HL|MOD_REG_F,0,0,0,0,

    //168 LDD, CPD
    MOD_REG_BC|MOD_REG_DE|MOD_REG_HL|MOD_REG_F|MOD_REG_DE_MEM, MOD_REG_BC|MOD_REG_HL|MOD_REG_F, 
    //IND, OUTD, NOP, NOP, NOP
    MOD_REG_B|MOD_REG_HL|MOD_REG_F|MOD_REG_HL_MEM, MOD_REG_B|MOD_REG_HL|MOD_REG_F,0,0,0,0,

    //176 LDIR, CPIR
    MOD_REG_BC|MOD_REG_DE|MOD_REG_HL|MOD_REG_F|MOD_REG_DE_MEM, MOD_REG_BC|MOD_REG_HL|MOD_REG_F, 
    //INIR, OTIR, NOP, NOP, NOP
    MOD_REG_B|MOD_REG_HL|MOD_REG_F|MOD_REG_HL_MEM, MOD_REG_B|MOD_REG_HL|MOD_REG_F,0,0,0,0,

    //184 LDDR, CPDR
    MOD_REG_BC|MOD_REG_DE|MOD_REG_HL|MOD_REG_F|MOD_REG_DE_MEM, MOD_REG_BC|MOD_REG_HL|MOD_REG_F, 
    //INDR, OTDR, NOP, NOP, NOP
    MOD_REG_B|MOD_REG_HL|MOD_REG_F|MOD_REG_HL_MEM, MOD_REG_B|MOD_REG_HL|MOD_REG_F,0,0,0,0,

    //192
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //208
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //224
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //240
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};


//Tabla de los registros modificados para opcodes con prefijo DD o FD
z80_long_int debug_modified_registers_dd_fd_list[256]={
    //0 NOPD
    0,0,0,0,0,0,0,0,
    0,MOD_REG_IX|MOD_REG_F,0,0,0,0,0,0,
    //16 NOPD
    0,0,0,0,0,0,0,0,
    0,MOD_REG_IX|MOD_REG_F,0,0,0,0,0,0,
    //32 NOPD
    0,MOD_REG_IX,0,         MOD_REG_IX,MOD_REG_IX_H,MOD_REG_IX_H,MOD_REG_IX_H,0,
    0,MOD_REG_IX,MOD_REG_IX,MOD_REG_IX,MOD_REG_IX_L,MOD_REG_IX_L,MOD_REG_IX_L,0,
    //48 NOPD
    0,0,0,0,MOD_REG_F,MOD_REG_F,0,0,
    0,MOD_REG_IX,0,0,0,0,0,0,
    //64 NOPD
    0,0,0,0,MOD_REG_B,MOD_REG_B,MOD_REG_B,0,
    0,0,0,0,MOD_REG_C,MOD_REG_C,MOD_REG_C,0,
    //80 NOPD
    0,0,0,0,MOD_REG_D,MOD_REG_D,MOD_REG_D,0,
    0,0,0,0,MOD_REG_E,MOD_REG_E,MOD_REG_E,0,
    //96 LD IXh,B
    MOD_REG_IX_H,MOD_REG_IX_H,MOD_REG_IX_H,MOD_REG_IX_H,0,MOD_REG_IX_H,MOD_REG_H,MOD_REG_IX_H,
    MOD_REG_IX_L,MOD_REG_IX_L,MOD_REG_IX_L,MOD_REG_IX_L,MOD_REG_IX_L,0,MOD_REG_L,MOD_REG_IX_L,
    //112 NOPD
    0,0,0,0,0,0,0,0,
    0,0,0,0,MOD_REG_A,MOD_REG_A,MOD_REG_A,0,
    //128 NOPD
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    //144 NOPD
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    //160 NOPD
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    //176 NOPD
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    0,0,0,0,MOD_REG_AF,MOD_REG_AF,MOD_REG_AF,0,
    //192 NOPD
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //208 NOPD
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    //224 NOPD
    0,MOD_REG_IX|MOD_REG_SP,0,MOD_REG_IX,0,MOD_REG_SP,0,0,
    0,0,0,MOD_REG_IX|MOD_REG_DE,0,0,0,0,
    //240 NOPD
    0,0,0,0,0,0,0,0,
    0,MOD_REG_SP,0,0,0,0,0,0,
};

z80_long_int menu_debug_get_modified_registers(menu_z80_moto_int direccion)
{
    direccion=adjust_address_memory_size(direccion);
    z80_byte opcode=menu_debug_get_mapped_byte(direccion);

    //Para opcodes con CB
    if (opcode==0xCB) {
        direccion++;
        direccion=adjust_address_memory_size(direccion);
        opcode=menu_debug_get_mapped_byte(direccion);
        z80_long_int modificados=debug_modified_registers_cb_list[opcode];
        return modificados;
    }

    //Para opcodes con ED
    if (opcode==0xED) {
        direccion++;
        direccion=adjust_address_memory_size(direccion);
        opcode=menu_debug_get_mapped_byte(direccion);
        z80_long_int modificados=debug_modified_registers_ed_list[opcode];
        return modificados;
    }    

    //Para opcodes con DD o FD
    if (opcode==0xDD || opcode==0xFD) {
        direccion++;
        direccion=adjust_address_memory_size(direccion);
        z80_byte opcode_orig=opcode;
        opcode=menu_debug_get_mapped_byte(direccion);

        z80_long_int modificados;

        //SI CB
        if (opcode==0xCB) {
            //saltar desplazamiento y sacar opcode
            direccion++;
            direccion++;
            direccion=adjust_address_memory_size(direccion);
            opcode=menu_debug_get_mapped_byte(direccion);
            modificados=debug_modified_registers_cb_list[opcode];
        }

        else {
            modificados=debug_modified_registers_dd_fd_list[opcode];
        }

        //Si era FD, modificar
        if (opcode_orig==0xFD) {
            if (modificados & MOD_REG_IX_L) {
                //Cambiar ese por IY_L
                //Xor porque sabemos que ese bit esta a 1 y queremos quitarlo
                modificados ^=MOD_REG_IX_L;
                modificados |=MOD_REG_IY_L;
            }

            if (modificados & MOD_REG_IX_H) {
                //Cambiar ese por IY_H
                //Xor porque sabemos que ese bit esta a 1 y queremos quitarlo
                modificados ^=MOD_REG_IX_H;
                modificados |=MOD_REG_IY_H;
            }

        }
        return modificados;
    }   

    z80_long_int modificados=debug_modified_registers_list[opcode];

    return modificados;

}

//Muestra el registro que le corresponde para esta linea
//Tambien indica el registro que se modifica, de la siguiente manera:
//Se indican las columnas que se alteran, de tal manera que se muestre en otro color las columnas afectadas
//El numero de columna sera un valor entre 0 y 14, por tanto, permitimos cambiar hasta 3 columnas, codificando asi:
//(columna1+1)+(columna2+1)*16+(columna3+1)*256

//0123456789012
//HL 0000'0000
void menu_debug_show_register_line(int linea,char *textoregistros,int *columnas_modificadas)
{
	char buffer_flags[32];

    //de momento
    *columnas_modificadas=0;

    //Retornar que registros se modifican
    //Bits:
    //0=SP
    //1=A
    //2=F
    //3=AF'
    z80_long_int registros_modificados=menu_debug_get_modified_registers(menu_debug_memory_pointer);

    //temp
    //*columnas_modificadas=(1)+16*3+256*6;

	//char textopaginasmem[100];

	//char textopaginasmem_linea1[100];
	//char textopaginasmem_linea2[100];

        debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
        int total_segmentos=debug_get_memory_pages_extended(segmentos);

	int offset_bloque;

	//Por defecto, cadena vacia
	textoregistros[0]=0;

	//En vista daad, mostrar flags de daad
	if (menu_debug_registers_current_view==8) {
		int linea_origen=linea;
		if (linea_origen<0 || linea_origen>MENU_DEBUG_NUMBER_FLAGS_OBJECTS) return;

		//comprobar que no haya watches fuera de rango, como en quill
		menu_debug_daad_check_init_flagobject();

		menu_debug_daad_string_flagobject(linea_origen,textoregistros);

		//sprintf (textoregistros,"F%2d %d",flag_leer,util_daad_get_flag_value(flag_leer));

		return;
	}
	
	//para mostrar vector interrupcion
	char string_vector_int[10]="     ";
	if (im_mode==2) {
	
	
    z80_int temp_i;
    z80_int puntero_int;
    z80_byte dir_l,dir_h;

    temp_i=get_im2_interrupt_vector();
    dir_l=peek_byte_no_time(temp_i++);
    dir_h=peek_byte_no_time(temp_i);
    puntero_int=value_8_to_16(dir_h,dir_l);
							
	sprintf(string_vector_int,"@%04X",puntero_int);
	
	}

	if (CPU_IS_Z80) {

        switch (linea) {
            case 0:
                sprintf (textoregistros,"PC %04X",get_pc_register() );
            break;

            case 1:
                sprintf (textoregistros,"SP %04X",reg_sp);
                if (registros_modificados & MOD_REG_SP)         *columnas_modificadas |=(1|(2<<4));      //columna 1,2 registro SP
            break;

            case 2:
                sprintf (textoregistros,"AF %02X%02X'%02X%02X",reg_a,Z80_FLAGS,reg_a_shadow,Z80_FLAGS_SHADOW);
                if (registros_modificados & MOD_REG_A)          *columnas_modificadas |=1;      //columna 1 registro A
                if (registros_modificados & MOD_REG_F)          *columnas_modificadas |=(2<<4); //columna 2 registro F
                if (registros_modificados & MOD_REG_AF_SHADOW)  *columnas_modificadas |=(8<<8); //columna 8 registro AF'
            break;

            case 3:
                sprintf (textoregistros,"%c%c%c%c%c%c%c%c",DEBUG_STRING_FLAGS);
            break;		

            case 4:
                sprintf (textoregistros,"HL %04X'%02X%02X",HL,reg_h_shadow,reg_l_shadow);
                if (registros_modificados & MOD_REG_H)          *columnas_modificadas |=1;      //columna 1 registro H
                if (registros_modificados & MOD_REG_L)          *columnas_modificadas |=(2<<4); //columna 2 registro L
                if (registros_modificados & MOD_REG_HL_SHADOW)  *columnas_modificadas |=(8<<8); //columna 8 registro HL'
            break;

            case 5:
                sprintf (textoregistros,"DE %04X'%02X%02X",DE,reg_d_shadow,reg_e_shadow);
                if (registros_modificados & MOD_REG_D)          *columnas_modificadas |=1;      //columna 1 registro D
                if (registros_modificados & MOD_REG_E)          *columnas_modificadas |=(2<<4); //columna 2 registro E
                if (registros_modificados & MOD_REG_DE_SHADOW)  *columnas_modificadas |=(8<<8); //columna 8 registro DE'                
            break;

            case 6:
                sprintf (textoregistros,"BC %04X'%02X%02X",BC,reg_b_shadow,reg_c_shadow);
                if (registros_modificados & MOD_REG_B)          *columnas_modificadas |=1;      //columna 1 registro B
                if (registros_modificados & MOD_REG_C)          *columnas_modificadas |=(2<<4); //columna 2 registro C
                if (registros_modificados & MOD_REG_BC_SHADOW)  *columnas_modificadas |=(8<<8); //columna 8 registro BC'
            break;

            case 7:
                sprintf (textoregistros,"IX %04X",reg_ix);
                if (registros_modificados & MOD_REG_IX_H)          *columnas_modificadas |=1;      //columna 1 registro IX_H
                if (registros_modificados & MOD_REG_IX_L)          *columnas_modificadas |=(2<<4); //columna 2 registro IX_l
            break;

            case 8:
                sprintf (textoregistros,"IY %04X",reg_iy);
                if (registros_modificados & MOD_REG_IY_H)          *columnas_modificadas |=1;      //columna 1 registro IY_H
                if (registros_modificados & MOD_REG_IY_L)          *columnas_modificadas |=(2<<4); //columna 2 registro IY_l
            break;

            case 9:
                sprintf (textoregistros,"IR %02X%02X%s",reg_i,(reg_r&127)|(reg_r_bit7&128) , string_vector_int);
                if (registros_modificados & MOD_REG_I)          *columnas_modificadas |=1;      //columna 1 registro I
                if (registros_modificados & MOD_REG_R)          *columnas_modificadas |=(2<<4); //columna 2 registro R
            break;

            case 10:
                sprintf (textoregistros,"IM%d IFF%c%c %s",im_mode,DEBUG_STRING_IFF12,(z80_halt_signal.v ? "HLT" : "") );
                if (registros_modificados & MOD_REG_IFF)          *columnas_modificadas |=5|(6<<4)|(7<<8);      //columna 5,6,7 registro IFF
                //nunca se va a dar junto el cambio de IFF y IM
                if (registros_modificados & MOD_REG_IM_MODE)      *columnas_modificadas |=1|(2<<4);      //columna 1,2 registro IM
            break;

            case 11:
                sprintf (textoregistros,"(HL) %02X %02X",peek_byte_z80_moto(HL),peek_byte_z80_moto(HL+1));
                //mostrar cuando se modifica (HL)
                //columna 1,2,3,4 registro (HL)
                if (registros_modificados & MOD_REG_HL_MEM)          *columnas_modificadas |=1|(2<<4)|(3<<8)|(4<<12);      
            break;      

            case 12:
                sprintf (textoregistros,"(DE) %02X %02X",peek_byte_z80_moto(DE),peek_byte_z80_moto(DE+1));
                //mostrar cuando se modifica (DE)
                //columna 1,2,3,4 registro (DE)
                if (registros_modificados & MOD_REG_DE_MEM)          *columnas_modificadas |=1|(2<<4)|(3<<8)|(4<<12);                      
            break; 

            case 13:
                sprintf (textoregistros,"(BC) %02X %02X",peek_byte_z80_moto(BC),peek_byte_z80_moto(BC+1));
                //mostrar cuando se modifica (BC)    
                //columna 1,2,3,4 registro (BC)
                if (registros_modificados & MOD_REG_BC_MEM)          *columnas_modificadas |=1|(2<<4)|(3<<8)|(4<<12);                      
            break;               

    

            case 14:
                sprintf (textoregistros,"TSTATE %d",t_estados);
            break;

            case 15:
            case 16:
            case 17:
            case 18:
                //Por defecto, cad
                //Mostrar en una linea, dos bloques de memoria mapeadas
                offset_bloque=linea-15;  //este 15 debe coincidir con el primer case de este bloque
                                        //para que la primera linea de este bloque sea offset_bloque=0
                
                offset_bloque *=2; //2 bloques por cada linea
                //primer bloque
                if (offset_bloque<total_segmentos) {
                    sprintf (textoregistros,"[%s]",segmentos[offset_bloque].shortname);
                    offset_bloque++;

                    //Segundo bloque
                    if (offset_bloque<total_segmentos) {
                        int longitud=strlen(textoregistros);
                        sprintf (&textoregistros[longitud],"[%s]",segmentos[offset_bloque].shortname);
                    }
                }
            break;
            

        }

	}

	if (CPU_IS_SCMP) {
	        switch (linea) {
        	        case 0:
                	        sprintf (textoregistros,"PC %04X",get_pc_register() );
	                break;

        	        case 1:
                	        sprintf (textoregistros,"AC %02X",scmp_m_AC);
	                break;

        	        case 2:
                	        sprintf (textoregistros,"ER %02X",scmp_m_ER);
	                break;

			case 3:
				sprintf (textoregistros,"SR %02X",scmp_m_SR);
			break;

			case 4:
                                scmp_get_flags_letters(scmp_m_SR,buffer_flags);
				sprintf (textoregistros,"%s",buffer_flags);
			break;

			case 5:
				sprintf (textoregistros,"P1 %04X",scmp_m_P1.w.l);
			break;

			case 6:
				sprintf (textoregistros,"P2 %04X",scmp_m_P2.w.l);
			break;

			case 7:
				sprintf (textoregistros,"P3 %04X",scmp_m_P3.w.l);
			break;

		}

	}

	if (CPU_IS_MOTOROLA) {
		switch (linea) {

			case 0:
				 sprintf (textoregistros,"PC %05X",get_pc_register() );
			break;

			case 1:
				 sprintf (textoregistros,"SP %05X",m68k_get_reg(NULL, M68K_REG_SP) );
			break;

			case 2:
				 sprintf (textoregistros,"USP %05X",m68k_get_reg(NULL, M68K_REG_USP) );
			break;

			case 3:
				 sprintf (textoregistros,"SR %04X",m68k_get_reg(NULL, M68K_REG_SR) );
			break;

			case 4:
				motorola_get_flags_string(buffer_flags);
				sprintf (textoregistros,"%s",buffer_flags );
			break;

			case 5:
				 sprintf (textoregistros,"A0 %08X",m68k_get_reg(NULL, M68K_REG_A0) );
			break;

			case 6:
				 sprintf (textoregistros,"A1 %08X",m68k_get_reg(NULL, M68K_REG_A1) );
			break;

			case 7:
				 sprintf (textoregistros,"A2 %08X",m68k_get_reg(NULL, M68K_REG_A2) );
			break;

			case 8:
				 sprintf (textoregistros,"A3 %08X",m68k_get_reg(NULL, M68K_REG_A3) );
			break;
            
			case 9:
				 sprintf (textoregistros,"A4 %08X",m68k_get_reg(NULL, M68K_REG_A4) );
			break;

			case 10:
				 sprintf (textoregistros,"A5 %08X",m68k_get_reg(NULL, M68K_REG_A5) );
			break;

			case 11:
				 sprintf (textoregistros,"A6 %08X",m68k_get_reg(NULL, M68K_REG_A6) );
			break;

			case 12:
				 sprintf (textoregistros,"A7 %08X",m68k_get_reg(NULL, M68K_REG_A7) );
			break;


            //Estos solo para Motorola

			case 13:
				sprintf (textoregistros,"D0 %08X",m68k_get_reg(NULL, M68K_REG_D0) );
            break;

			case 14:
				sprintf (textoregistros,"D1 %08X",m68k_get_reg(NULL, M68K_REG_D1) );
            break;

			case 15:
				sprintf (textoregistros,"D2 %08X",m68k_get_reg(NULL, M68K_REG_D2) );
            break;

			case 16:
				sprintf (textoregistros,"D3 %08X",m68k_get_reg(NULL, M68K_REG_D3) );
            break;

			case 17:
				sprintf (textoregistros,"D4 %08X",m68k_get_reg(NULL, M68K_REG_D4) );
            break;

			case 18:
				sprintf (textoregistros,"D5 %08X",m68k_get_reg(NULL, M68K_REG_D5) );
            break;

			case 19:
				sprintf (textoregistros,"D6 %08X",m68k_get_reg(NULL, M68K_REG_D6) );
            break;

			case 20:
				sprintf (textoregistros,"D7 %08X",m68k_get_reg(NULL, M68K_REG_D7) );
            break;            
            

		}
	}
/*
   else if (CPU_IS_MOTOROLA) {
                             
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,0,M68K_REG_A0,M68K_REG_D0);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,1,M68K_REG_A1,M68K_REG_D1);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,2,M68K_REG_A2,M68K_REG_D2);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,3,M68K_REG_A3,M68K_REG_D3);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,4,M68K_REG_A4,M68K_REG_D4);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,5,M68K_REG_A5,M68K_REG_D5);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,6,M68K_REG_A6,M68K_REG_D6);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,7,M68K_REG_A7,M68K_REG_D7);

*/
}

//Longitud que ocupa el ultimo opcode desensamblado
size_t menu_debug_registers_print_registers_longitud_opcode=0;

//Ultima direccion en desemsamblado/vista hexa, para poder hacer pgup/pgdn
menu_z80_moto_int menu_debug_memory_pointer_last=0;


//Direcciones de cada linea en la vista numero 3
//menu_z80_moto_int menu_debug_lines_addresses[24];

//Numero de lineas del listado principal de la vista
int menu_debug_get_main_list_view(zxvision_window *w)
{
	int lineas=1;

    if (menu_debug_registers_current_view==3 || menu_debug_registers_current_view==5) lineas=9;
    if (menu_debug_registers_current_view==1 || menu_debug_registers_current_view==4 || menu_debug_registers_current_view==6) lineas=get_menu_debug_num_lineas_full(w);
	if (menu_debug_registers_current_view==8) lineas=get_menu_debug_num_lineas_full(w)-2;

	return lineas;
}

//Si vista actual tiene desensamblado u otros datos. En el primer de los casos, los movimientos de cursor se gestionan mediante saltos de opcodes
int menu_debug_view_has_disassemly(void)
{
	if (menu_debug_registers_current_view<=4) return 1;

	return 0;
}

menu_z80_moto_int menu_debug_disassemble_subir_veces(menu_z80_moto_int posicion,int veces)
{
        int i;
        for (i=0;i<veces;i++) {
                posicion=menu_debug_disassemble_subir(posicion);
        }
        return posicion;
}


menu_z80_moto_int menu_debug_register_decrement_half(menu_z80_moto_int posicion,zxvision_window *w)
{
	int i;
	for (i=0;i<get_menu_debug_num_lineas_full(w)/2;i++) {
		posicion=menu_debug_disassemble_subir(posicion);
	}
	return posicion;
}

 

int menu_debug_hexdump_change_pointer(int p)
{


        char string_address[10];

        sprintf (string_address,"%XH",p);


        //menu_ventana_scanf("Address? (in hex)",string_address,6);
        menu_ventana_scanf("Address?",string_address,10);

	//p=strtol(string_address, NULL, 16);
	p=parse_string_to_number(string_address);


	return p;

}


//Ajustar cuando se pulsa hacia arriba por debajo de direccion 0.
//Debe poner el puntero hacia el final de la zona de memoria
menu_z80_moto_int menu_debug_hexdump_adjusta_en_negativo(menu_z80_moto_int dir,int linesize)
{
	if (dir>=menu_debug_memory_zone_size) {
		dir=menu_debug_memory_zone_size-linesize;
	}
	//printf ("menu_debug_memory_zone_size %X\n",menu_debug_memory_zone_size);

	return dir;
}


//Si desensamblado en menu view registers muestra:
//0: lo normal. opcodes
//1: hexa
//2: decimal
//3: ascii
//4: disassemble sin nada a la derecha
int menu_debug_registers_subview_type=0;

//Modo ascii. 0 spectrum , 1 zx80, 2 zx81
int menu_debug_hexdump_with_ascii_modo_ascii=0;

void menu_debug_next_dis_show_hexa(void)
{
	menu_debug_registers_subview_type++;

	if (menu_debug_registers_subview_type==5) menu_debug_registers_subview_type=0;
}

void menu_debug_registers_adjust_ptr_on_follow(void)
{
	if (menu_debug_follow_pc.v) {
                menu_debug_memory_pointer=get_pc_register();
                //Si se esta mirando zona copper
                if (menu_debug_memory_zone==MEMORY_ZONE_NUM_TBBLUE_COPPER) {
                        menu_debug_memory_pointer=tbblue_copper_pc;
                }

        }
}


void menu_debug_registros_parte_derecha(int linea,char *buffer_linea,int columna_registros,int mostrar_separador,int *columnas_modificadas)
{

    char buffer_registros[33];
    if (menu_debug_registers_subview_type!=4) {

            //Quitar el 0 del final
            int longitud=strlen(buffer_linea);
            buffer_linea[longitud]=32;

            //Muestra el registro que le corresponde para esta linea
            menu_debug_show_register_line(linea,buffer_registros,columnas_modificadas);


            //En QL se pega siempre el opcode con los registros. meter espacio
            if (CPU_IS_MOTOROLA) buffer_linea[columna_registros-1]=' ';

            //Agregar registro que le corresponda. Columna 19 normalmente. Con el || del separador para quitar el color seleccionado
            if (mostrar_separador) sprintf(&buffer_linea[columna_registros],"||%s",buffer_registros);
            else sprintf(&buffer_linea[columna_registros],"%s",buffer_registros);
    }
}

//Indica si se cumple el flag indicado o no
//Entrada: numero flag: 0=NZ, 1=Z, etc
int menu_debug_if_flag(int numero_flag)
{
    switch(numero_flag)
    {
        case 0:
            if( !(Z80_FLAGS & FLAG_Z) ) return 1;
        break;

        case 1:
            if( Z80_FLAGS & FLAG_Z ) return 1;
        break;

        case 2:
            if( !(Z80_FLAGS & FLAG_C) ) return 1;
        break;

        case 3:
            if( Z80_FLAGS & FLAG_C ) return 1;
        break;        

        case 4:
            if( !(Z80_FLAGS & FLAG_PV) ) return 1;
        break;

        case 5:
            if( Z80_FLAGS & FLAG_PV ) return 1;
        break;    

        case 6:
            if( !(Z80_FLAGS & FLAG_S) ) return 1;
        break;

        case 7:
            if( Z80_FLAGS & FLAG_S ) return 1;
        break;   


    }

    return 0;
}

//Segun el opcode mira si se cumple condicion y mete en buffer la condicion que se cumple
//Si no, no mete nada 
//Retorna 0 si no se cumple, 1 si se cumple
int menu_debug_get_condicion_satisfy(z80_byte opcode,char *buffer)
{
    if (!CPU_IS_Z80) return 0;

    //Asumimos no condicion
    int condicion=-1;

    char *string_conditions[]={
        "NZ","Z","NC","C","PO","PE","P","M"
    };

    //JR CC, dis
    //001cc000
    if ((opcode & (1+2+4+32+64+128))==32) {
        condicion=(opcode>>3)&3;
    }

    //RET CC
    //11ccc000
    if ((opcode & (1+2+4+64+128))==64+128) {
        condicion=(opcode>>3)&7;
    }        

    //JP CC, NN
    //11ccc010
    if ((opcode & (1+2+4+64+128))==2+64+128) {
        condicion=(opcode>>3)&7;
    }    

    //CALL CC, NN
    //11ccc100
    if ((opcode & (1+2+4+64+128))==4+64+128) {
        condicion=(opcode>>3)&7;
    }      


    //Caso DJNZ dis, que no usa flag
    if (opcode==16 && reg_b!=1) {
        strcpy(buffer,"-> satisfy B!=1");
        return 1;        
    }

    if (condicion>=0 && menu_debug_if_flag(condicion)) {
        sprintf(buffer,"-> satisfy %s",string_conditions[condicion]);
        return 1;
    }

    return 0;
}

void menu_debug_registros_colorea_columnas_modificadas(zxvision_window *w,int linea,int xinicial,int columnas_modificadas)
{
    //no hacerlo si la vista no muestra registros
    if (menu_debug_registers_subview_type==4) return;

    int columna1=columnas_modificadas & 0xF;
    int columna2=(columnas_modificadas>>4) & 0xF;
    int columna3=(columnas_modificadas>>8) & 0xF;
    int columna4=(columnas_modificadas>>12) & 0xF;

    if (columna1) {
        columna1--;
        zxvision_set_attr(w,xinicial+columna1,linea,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,0);
    }

    if (columna2) {
        columna2--;
        zxvision_set_attr(w,xinicial+columna2,linea,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,0);
    }

    if (columna3) {
        columna3--;
        zxvision_set_attr(w,xinicial+columna3,linea,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,0);
    }

    if (columna4) {
        columna4--;
        zxvision_set_attr(w,xinicial+columna4,linea,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,0);
    }            
}

int menu_debug_registers_print_registers(zxvision_window *w,int linea)
{
	//printf("linea: %d\n",linea);
	char textoregistros[33];

	char dumpmemoria[33];

	char dumpassembler[65];

	//size_t longitud_opcode;

	//menu_z80_moto_int copia_reg_pc;
	int i;

	menu_z80_moto_int menu_debug_memory_pointer_copia;

	//menu_debug_registers_adjust_ptr_on_follow();

    int columnas_modificadas;


	//Conservamos valor original y usamos uno de copia
	menu_debug_memory_pointer_copia=menu_debug_memory_pointer;

	char buffer_linea[MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH];	



	//Por defecto
	menu_debug_registers_print_registers_longitud_opcode=8; //Esto se hace para que en las vistas de solo hexadecimal, se mueva arriba/abajo de 8 en 8


		if (menu_debug_registers_current_view==7) {
			menu_debug_print_address_memory_zone(dumpassembler,menu_debug_memory_pointer_copia);

			int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

			//metemos espacio en 0 final
			dumpassembler[longitud_direccion]=' ';


			//Assembler
			debugger_disassemble(&dumpassembler[longitud_direccion+1],17,&menu_debug_registers_print_registers_longitud_opcode,menu_debug_memory_pointer_copia);


			//debugger_disassemble(dumpassembler,32,&menu_debug_registers_print_registers_longitud_opcode,menu_debug_memory_pointer_copia );
                        menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia+menu_debug_registers_print_registers_longitud_opcode;

                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
			zxvision_print_string_defaults_fillspc(w,1,linea++,dumpassembler);

			sprintf (textoregistros,"TSTATES: %05d SCANL: %03dX%03d",t_estados,(t_estados % screen_testados_linea),t_scanline_draw);
			//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
		}


		if (menu_debug_registers_current_view==2) {

			debugger_disassemble(dumpassembler,32,&menu_debug_registers_print_registers_longitud_opcode,menu_debug_memory_pointer_copia );
			menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia+menu_debug_registers_print_registers_longitud_opcode;

			//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
			zxvision_print_string_defaults_fillspc(w,1,linea++,dumpassembler);


			if (CPU_IS_SCMP) {
				menu_debug_registers_dump_hex(dumpmemoria,get_pc_register(),8);
	     		sprintf (textoregistros,"PC: %04X : %s",get_pc_register(),dumpmemoria);
	     		 //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_dump_hex(dumpmemoria,scmp_m_P1.w.l,8);
				sprintf (textoregistros,"P1: %04X : %s",scmp_m_P1.w.l,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_dump_hex(dumpmemoria,scmp_m_P2.w.l,8);
				sprintf (textoregistros,"P2: %04X : %s",scmp_m_P2.w.l,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_dump_hex(dumpmemoria,scmp_m_P3.w.l,8);
				sprintf (textoregistros,"P3: %04X : %s",scmp_m_P3.w.l,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"AC: %02X ER: %02XH",scmp_m_AC, scmp_m_ER);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				char buffer_flags[9];
				scmp_get_flags_letters(scmp_m_SR,buffer_flags);

				sprintf (textoregistros,"SR: %02X %s",scmp_m_SR,buffer_flags);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);



			}

			else if (CPU_IS_MOTOROLA) {
				sprintf (textoregistros,"PC: %05X SP: %05X USP: %05X",get_pc_register(),m68k_get_reg(NULL, M68K_REG_SP),m68k_get_reg(NULL, M68K_REG_USP));

				/*
				case M68K_REG_A7:       return cpu->dar[15];
				case M68K_REG_SP:       return cpu->dar[15];
 				case M68K_REG_USP:      return cpu->s_flag ? cpu->sp[0] : cpu->dar[15];

				SP siempre muestra A7
				USP muestra: en modo supervisor, SSP. En modo no supervisor, SP/A7
				*/

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				unsigned int registro_sr=m68k_get_reg(NULL, M68K_REG_SR);

				char buffer_flags[32];
				motorola_get_flags_string(buffer_flags);
				sprintf (textoregistros,"SR: %04X : %s",registro_sr,buffer_flags);

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,0,M68K_REG_A0,M68K_REG_D0);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,1,M68K_REG_A1,M68K_REG_D1);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,2,M68K_REG_A2,M68K_REG_D2);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,3,M68K_REG_A3,M68K_REG_D3);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,4,M68K_REG_A4,M68K_REG_D4);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,5,M68K_REG_A5,M68K_REG_D5);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,6,M68K_REG_A6,M68K_REG_D6);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,7,M68K_REG_A7,M68K_REG_D7);



			}

			else {
				//Z80
				menu_debug_registers_dump_hex(dumpmemoria,get_pc_register(),8);

				sprintf (textoregistros,"PC: %04X : %s",get_pc_register(),dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


				menu_debug_registers_dump_hex(dumpmemoria,reg_sp,8);
				sprintf (textoregistros,"SP: %04X : %s",reg_sp,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"A: %02X F: %c%c%c%c%c%c%c%c",reg_a,DEBUG_STRING_FLAGS);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"A':%02X F':%c%c%c%c%c%c%c%c",reg_a_shadow,DEBUG_STRING_FLAGS_SHADOW);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"HL: %04X DE: %04X BC: %04X",HL,DE,BC);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"HL':%04X DE':%04X BC':%04X",(reg_h_shadow<<8)|reg_l_shadow,(reg_d_shadow<<8)|reg_e_shadow,(reg_b_shadow<<8)|reg_c_shadow);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"IX: %04X IY: %04X",reg_ix,reg_iy);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				char texto_nmi[10];
				if (MACHINE_IS_ZX81_TYPE) {
					sprintf (texto_nmi,"%s",(nmi_generator_active.v ? "NMI:On" : "NMI:Off"));
				}

				else {
					texto_nmi[0]=0;
				}

				sprintf (textoregistros,"R:%02X I:%02X IM%d IFF%c%c %s",
					(reg_r&127)|(reg_r_bit7&128),
					reg_i,
					im_mode,
					DEBUG_STRING_IFF12,
				
					texto_nmi);

				//01234567890123456789012345678901
				// R: 84 I: 1E DI IM1 NMI: Off
				// R: 84 I: 1E IFF1 IFF2 IM1 NMI: Off
				// R:84 I:1E IFF1 IFF2 IM1 NMI:Off

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

			}


		}

		if (menu_debug_registers_current_view==4 || menu_debug_registers_current_view==3) {


			int longitud_op;
			

			int limite=menu_debug_get_main_list_view(w);

			for (i=0;i<limite;i++) {
				menu_debug_dissassemble_una_instruccion(dumpassembler,menu_debug_memory_pointer_copia,&longitud_op);
				//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
				zxvision_print_string_defaults_fillspc(w,1,linea++,dumpassembler);
				menu_debug_memory_pointer_copia +=longitud_op;

				//Almacenar longitud del primer opcode mostrado
				if (i==0) menu_debug_registers_print_registers_longitud_opcode=longitud_op;
			}

			menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia;


		}



		//Linea de condact de daad
		if (menu_debug_registers_current_view==8) {

				int total_lineas_debug=7;

				size_t longitud_op;

				int i;

			

				z80_int direccion_desensamblar=value_8_to_16(reg_b,reg_c);		



				//char buffer_linea[MAX_LINE_CPU_REGISTERS_LENGTH];	


				//Si no esta en zona de parser
				if (!util_daad_is_in_parser() && !util_paws_is_in_parser() ) {
					strcpy(buffer_linea,"Not in condacts");
					//zxvision_print_string_defaults_fillspc(w,1,linea++,"Not in condacts");
				}

				else {				

					char buffer_verbo[6];
					char buffer_nombre[6];		

					z80_byte verbo=util_daad_get_flag_value(33);
					z80_byte nombre=util_daad_get_flag_value(34);

					//printf ("nombre: %d\n",nombre);

					//Por defecto
					strcpy(buffer_verbo,"_");
					strcpy(buffer_nombre,"_");

					//en quill no hay tipos de palabras. los establecemos a 0

					if (verbo!=255) util_daad_paws_locate_word(verbo,0,buffer_verbo);
					if (nombre!=255) {
						z80_byte tipo_palabra=2; 
						if (util_undaad_unpaws_is_quill() ) tipo_palabra=0;
						util_daad_paws_locate_word(nombre,tipo_palabra,buffer_nombre);
					}

					sprintf (buffer_linea,"%s %s",buffer_verbo,buffer_nombre);

					//zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);

				}

				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);

				zxvision_print_string_defaults_fillspc(w,1,linea++,"                    Watches");


/*
Para sacar el verbo + nombre de la entrada:

En el flag 33 está el código del verbo, el 34 el código del nombre. 
Si cualquiera de los dos vale 255 no buscas palabra y en su lugar pones un guion bajo (no-palabra)

Si es otro valor, en 0x8416  está la dirección donde está el vocabulario, si tomas esa direccion irás a una tabla en memoria con bloques de 7 bytes:

5 para 5 letras de la palabra (puede incluir espacios de padding al final si es más corta)
1 byte para el número de palabra (el flag 33)
1 byte para el tipo de palabra (verbo=0, nombre=2)

Solo tienes que buscar en esa tabla el número de palabra de flag 33, que sea de tipo 0 , y el código del flag 34 que sea de tipo 2
*/

				//linea++;	

				//Posicion fija para la columna de watches
				int columna_watches=20;		

				int terminador=0; //Si se ha llegado a algun terminador de linea	

				for (i=0;i<total_lineas_debug;i++) {

					//Inicializamos linea a mostrar con espacios primero
					//int j; 
					//for (j=0;j<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH;j++) buffer_linea[j]=32;
                    util_fill_string_character(buffer_linea,MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-1,32);

						//Si esta en zona de parser
						if (util_daad_is_in_parser() || util_paws_is_in_parser() ) {

							//$terminatorOpcodes = array(22, 23,103, 116,117,108);  //DONE/OK/NOTDONE/SKIP/RESTART/REDO

							int sera_terminador=0;


							//Si se llega a algun terminador
							if (!terminador) {
								z80_byte opcode=daad_peek(direccion_desensamblar);
								z80_byte opcode_res=opcode & 127;
								if (opcode_res==22 || opcode_res==23 || opcode_res==103 || opcode_res==116 || opcode_res==117 || opcode_res==108) sera_terminador=1;


								//Terminador de final y que no se mostrara
								if (opcode==0xFF) {
									//printf ("Hay terminador FF\n");
									terminador=1;
								}							
							}




							if (!terminador) {
								//Cambiamos temporalmente a zona de memoria de condacts de daad, para que desensamble como si fueran condacts
								int antes_menu_debug_memory_zone=menu_debug_memory_zone;
								if (util_daad_detect()) menu_debug_memory_zone=MEMORY_ZONE_NUM_DAAD_CONDACTS;	
								else menu_debug_memory_zone=MEMORY_ZONE_NUM_PAWS_CONDACTS;
								debugger_disassemble(dumpassembler,32,&longitud_op,direccion_desensamblar);
								menu_debug_memory_zone=antes_menu_debug_memory_zone;

								sprintf(buffer_linea,"%s",dumpassembler);

								terminador=sera_terminador;
							}

						}
						


						
						menu_debug_registros_parte_derecha(i,buffer_linea,columna_watches,0,&columnas_modificadas);
						

						//printf ("linea: %s\n",buffer_linea);

						zxvision_print_string_defaults_fillspc(w,1,linea,buffer_linea);

                        //esto no se usa en vista paws
                        //menu_debug_registros_colorea_columnas_modificadas(w,linea,columnas_modificadas);

                        linea++;


						direccion_desensamblar +=longitud_op;

				
		        }

                //Obtener versión parser
                char buffer_version[100];
                util_unpaws_daad_get_version_string(buffer_version);
                
                char buffer_idioma[100];
                buffer_idioma[0]=0;
                if (util_daad_detect() ) {
                    util_daad_get_language_parser(buffer_idioma);
                }


                sprintf(buffer_linea,"Info Parser: %s %s",buffer_version,buffer_idioma);
                linea++;
                zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);

				
                //temp. obtener conexiones
                //printf("conexiones: %d\n",util_textadventure_get_connections());


		}		

        if (menu_debug_registers_current_view==1) {


				size_t longitud_op;
				int limite=get_menu_debug_num_lineas_full(w);


				//printf ("%d\n",w->visible_width);


				//A partir de que columna aparecen los registros a la derecha
				//dependera del tamaño de la ventana
				int columna_registros=get_menu_debug_columna_registros(w);


				//Mi valor ptr
				menu_z80_moto_int puntero_ptr_inicial=menu_debug_memory_pointer_copia;

				//Donde empieza la vista. Subir desde direccion actual, desensamblando "hacia atras" , tantas veces como posicion cursor actual
				menu_debug_memory_pointer_copia=menu_debug_disassemble_subir_veces(puntero_ptr_inicial,menu_debug_line_cursor);
         



				//Comportamiento de 1 caracter de margen a la izquierda en ventana 
				int antes_menu_escribe_linea_startx=menu_escribe_linea_startx;

				menu_escribe_linea_startx=0;

                int guessed_next_pos_source=-1;
					
				//char buffer_linea[MAX_LINE_CPU_REGISTERS_LENGTH];
                for (i=0;i<limite;i++) {

					//Por si acaso
					//buffer_registros[0]=0;

					//Inicializamos linea a mostrar primero con espacios
					
					//int j; 
					//for (j=0;j<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH;j++) buffer_linea[j]=32;

                    util_fill_string_character(buffer_linea,MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-1,32);

					int opcion_actual=-1;

					int opcion_activada=1;  

					//Si esta linea tiene el cursor
					if (i==menu_debug_line_cursor) {
						opcion_actual=linea;			
						menu_debug_memory_pointer_copia=puntero_ptr_inicial;
						//printf ("draw line is the current. pointer=%04XH\n",menu_debug_memory_pointer_copia);
					}

					menu_z80_moto_int puntero_dir=adjust_address_memory_size(menu_debug_memory_pointer_copia);

					int tiene_brk=0;
					int tiene_pc=0;

					//Si linea tiene breakpoint
					if (debug_return_brk_pc_dir_condition(puntero_dir)>=0) tiene_brk=1;

					//Si linea es donde esta el PC
					if (puntero_dir==get_pc_register() ) tiene_pc=1;

                    char buffer_condicion[32];

                    buffer_condicion[0]=0;

					if (tiene_pc) {
                        buffer_linea[0]='>';

                        //Si estamos en backwards, otro cursor
                        if (indice_debug_cpu_backwards_history && cpu_step_mode.v) buffer_linea[0]='^';

                        //Meteremos texto, si conviene, de si se cumple condición o no
                        //prueba
                        z80_byte opcode_fires;
                        int direccion_condicion=menu_debug_memory_pointer_copia;


                        direccion_condicion=adjust_address_memory_size(direccion_condicion);
                        opcode_fires=menu_debug_get_mapped_byte(direccion_condicion);
                        menu_debug_get_condicion_satisfy(opcode_fires,buffer_condicion);
                        //strcpy(buffer_condicion," (satisfy NZ)");
                    }
					if (tiene_brk) {
						buffer_linea[0]='*';
						opcion_activada=0;
					}

					if (tiene_pc && tiene_brk) buffer_linea[0]='+'; //Cuando coinciden breakpoint y cursor

					

                    debugger_disassemble(dumpassembler,32,&longitud_op,menu_debug_memory_pointer_copia);

/*
//Si desensamblado en menu view registers muestra:
//0: lo normal. opcodes
//1: hexa
//2: decimal
//3: ascii
//4: lo normal pero sin mostrar registros a la derecha
int menu_debug_registers_subview_type=0;

*/
//menu_debug_memory_pointer=adjust_address_memory_size(menu_debug_memory_pointer);


					//Si mostramos en vez de desensamblado, volcado hexa, decimal o ascii
					if (menu_debug_registers_subview_type==1)	menu_debug_registers_dump_hex(dumpassembler,puntero_dir,longitud_op);
                    if (menu_debug_registers_subview_type==2)	menu_debug_registers_dump_decimal(dumpassembler,puntero_dir,longitud_op);
					if (menu_debug_registers_subview_type==3)  menu_debug_registers_dump_ascii(dumpassembler,puntero_dir,longitud_op,menu_debug_hexdump_with_ascii_modo_ascii,0);
					
					

                    //char buffer_desensamblado[200];

					sprintf(&buffer_linea[1],"%04X %s %s",puntero_dir,dumpassembler,buffer_condicion);

					//Guardar las direcciones de cada linea
					//menu_debug_lines_addresses[i]=puntero_dir;

                    //Si hay codigo fuente cargado
		            if (remote_tamanyo_archivo_raw_source_code) {
                        int pos_source=remote_disassemble_find_label(puntero_dir);
                        if (pos_source>=0) guessed_next_pos_source=pos_source;

                
                        if (pos_source>=0 || guessed_next_pos_source>=0) {
                            //Escribiremos directamente en buffer_linea
                            int longitud_texto=strlen(buffer_linea);
                            //quitamos fin de cadena
                            buffer_linea[longitud_texto]=' ';
                            int inicio=longitud_texto;
                            for (inicio=longitud_texto;inicio<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-1;inicio++) {
                                buffer_linea[inicio]=' ';
                            }

                            //final de cadena
                            buffer_linea[inicio]=0; 

                            //Y escribir linea codigo fuente
			                char *puntero_source=NULL;

		                    //int indice=remote_parsed_source_code_indexes_pointer[pos_source];
                            //puntero_source=&remote_raw_source_code_pointer[indice];  

                            //Intentamos mostrar la siguiente linea
                            if (pos_source>=0) {
                                int indice=remote_parsed_source_code_indexes_pointer[pos_source];
                                puntero_source=&remote_raw_source_code_pointer[indice];
                            }

                            else {
                                //Mostrar guessed
                                int indice=remote_parsed_source_code_indexes_pointer[guessed_next_pos_source];
                                puntero_source=&remote_raw_source_code_pointer[indice];
                            }


                            if (puntero_source!=NULL) {
                                int inicio=30; //posicion columna arbitraria
                                if (CPU_IS_MOTOROLA) inicio=40;
                                for (;inicio<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-1 && *puntero_source;inicio++) {
                                    buffer_linea[inicio]=*puntero_source;

                                    puntero_source++;
                                }
                            }
                        } 
                    }                   
                    if (guessed_next_pos_source>=0) guessed_next_pos_source++;

					
					menu_debug_registros_parte_derecha(i,buffer_linea,columna_registros,1,&columnas_modificadas);
					

					//printf ("buffer_linea: [%s]\n",buffer_linea);


					//zxvision_print_string_defaults_fillspc(w,1,linea,buffer_linea);

					//De los pocos usos de menu_escribe_linea_opcion_zxvision,
					//solo se usa en menus y aqui: para poder mostrar linea activada o en rojo

					menu_escribe_linea_opcion_zxvision(w,linea,opcion_actual,opcion_activada,buffer_linea,0);

                    menu_debug_registros_colorea_columnas_modificadas(w,linea,columna_registros,columnas_modificadas);

					//menu_escribe_linea_opcion_zxvision(w,linea,opcion_actual,opcion_activada,"0123456789001234567890012345678900123456789001234567890");

					//printf ("despues menu_escribe_linea_opcion_zxvision. i=%d\n",i);

					linea++;


					menu_debug_memory_pointer_copia +=longitud_op;

					//Almacenar longitud del primer opcode mostrado
					if (i==0) menu_debug_registers_print_registers_longitud_opcode=longitud_op;
                }


				menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia;


				//Vamos a ver si metemos una linea mas de la parte de la derecha extra, siempre que tenga contenido (primer caracter no espacio)
				//Esto sucede por ejemplo en tbblue, pues tiene 8 segmentos de memoria
				//Inicializamos a espacios
				//int j;
				//for (j=0;j<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH;j++) buffer_linea[j]=32;

                util_fill_string_character(buffer_linea,MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-1,32);

				
				menu_debug_registros_parte_derecha(i,buffer_linea,columna_registros,1,&columnas_modificadas);
				

				//primero borramos esa linea, por si cambiamos de subvista con M y hay "restos" ahi
				zxvision_print_string_defaults_fillspc(w,1,linea,"");

				//Si tiene contenido
				if (buffer_linea[columna_registros]!=' ' && buffer_linea[columna_registros]!=0) {
                                                //Agregamos linea perdiendo la linea en blanco de margen
						//menu_escribe_linea_opcion(linea,-1,1,buffer_linea);
						//zxvision_print_string_defaults_fillspc(w,1,linea,buffer_linea);

					//De los pocos usos de menu_escribe_linea_opcion_zxvision,
					//solo se usa en menus y dos veces en esta funcion
					//en este caso, es para poder procesar los caracteres "||"
					menu_escribe_linea_opcion_zxvision(w,linea,-1,1,buffer_linea,0);


				}

                menu_debug_registros_colorea_columnas_modificadas(w,linea,columna_registros,columnas_modificadas);

				linea++;

				menu_escribe_linea_startx=antes_menu_escribe_linea_startx;

			

				//Linea de stack
				//No mostrar stack en caso de scmp
				if (CPU_IS_Z80 || CPU_IS_MOTOROLA) {
					sprintf(buffer_linea,"(SP) ");

					int valores=10;
					if (CPU_IS_MOTOROLA) valores=5;
					debug_get_stack_values(valores,&buffer_linea[5]);
					//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
					zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);
				}

				//Linea de user stack
				if (CPU_IS_MOTOROLA) {
					int valores=5;
					sprintf(buffer_linea,"(USP) ");

					debug_get_user_stack_values(valores,&buffer_linea[5]);
					//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
					zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);
				}

				else {
					//En caso de Z80 o SCMP meter linea vacia
					zxvision_print_string_defaults_fillspc(w,1,linea++,"");
				}


        }

		if (menu_debug_registers_current_view==5 || menu_debug_registers_current_view==6) {

			//Hacer que texto ventana empiece pegado a la izquierda
			menu_escribe_linea_startx=0;

		
			int longitud_linea=8;
			

			int limite=menu_debug_get_main_list_view(w);

			for (i=0;i<limite;i++) {
					menu_debug_hexdump_with_ascii(dumpassembler,menu_debug_memory_pointer_copia,longitud_linea,0);
					//menu_debug_registers_dump_hex(dumpassembler,menu_debug_memory_pointer_copia,longitud_linea);
					//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
					zxvision_print_string_defaults_fillspc(w,0,linea++,dumpassembler);
					menu_debug_memory_pointer_copia +=longitud_linea;
			}

			menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia;


			//Restaurar comportamiento texto ventana
			menu_escribe_linea_startx=1;

		}

		//Aparecen otros registros y valores complementarios
		if (menu_debug_registers_current_view==2 || menu_debug_registers_current_view==3 || menu_debug_registers_current_view==5) {
            //Separador
        	sprintf (textoregistros," ");
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


			//
			// MEMPTR y T-Estados
			//
            sprintf (textoregistros,"MEMPTR: %04X TSTATES: %05d",memptr,t_estados);
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


			//
			// Mas T-Estados y parcial
			//

			char buffer_estadosparcial[32];
			/*int estadosparcial=debug_t_estados_parcial;
			

			if (estadosparcial>999999999) sprintf (buffer_estadosparcial,"%s","OVERFLOW");
			else sprintf (buffer_estadosparcial,"%09u",estadosparcial);*/

			debug_get_t_estados_parcial(buffer_estadosparcial);

            sprintf (textoregistros,"TSTATL: %03d TSTATP: %s",(t_estados % screen_testados_linea),buffer_estadosparcial );
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

			//
			// FPS y Scanline
			//

			if (MACHINE_IS_ZX8081) {
	        	sprintf (textoregistros,"SCANLIN: %03d FPS: %03d VPS: %03d",t_scanline_draw,ultimo_fps,last_vsync_per_second);
			}
			else {
	            sprintf (textoregistros,"SCANLINE: %03d FPS: %03d",t_scanline_draw,ultimo_fps);
			}
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);



			//
    	    // ULA
			//

			//no hacer autodeteccion de idle bus port, para que no se active por si solo
			z80_bit copia_autodetect_rainbow;
			copia_autodetect_rainbow.v=autodetect_rainbow.v;

			autodetect_rainbow.v=0;



			//
			//Puerto FE, Idle port y flash. cada uno para la maquina que lo soporte
			//Solo para Spectrum O Z88
			//
			if (MACHINE_IS_SPECTRUM || MACHINE_IS_Z88) {
				char feporttext[20];
				if (MACHINE_IS_SPECTRUM) {
					sprintf (feporttext,"FE: %02X ",out_254_original_value);
				}
				else feporttext[0]=0;

            	char flashtext[40];
            	if (MACHINE_IS_SPECTRUM) {
	            	sprintf (flashtext,"FLASH: %d ",estado_parpadeo.v);
    	       	}

        	    else if (MACHINE_IS_Z88) {
            		sprintf (flashtext,"FLASH: %d ",estado_parpadeo.v);
            	}
	
	            else flashtext[0]=0;



				char idleporttext[20];
				if (MACHINE_IS_SPECTRUM) {
					sprintf (idleporttext,"IDLEPORT: %02X",idle_bus_port(255) );
				}
				else idleporttext[0]=0;

	            sprintf (textoregistros,"%s%s%s",feporttext,flashtext,idleporttext );

				autodetect_rainbow.v=copia_autodetect_rainbow.v;
    	        //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


			}


			//
			// Linea audio 
			//
			if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081) {
                        sprintf (textoregistros,"AUDIO: BEEPER: %03d AY: %03d", (MACHINE_IS_ZX8081 ? da_amplitud_speaker_zx8081() :  value_beeper),da_output_ay() );
                        //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}						






			//
			// Linea solo de Prism
			//
			if (MACHINE_IS_PRISM) {
				//SI vram aperture prism
				if (prism_ula2_registers[1] & 1) sprintf (textoregistros,"VRAM0 VRAM1 aperture");

				else {
						//       012345678901234567890123456789012
						sprintf (textoregistros,"VRAM0 SRAM10 SRAM11 not apert.");
				}

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}


			//
			// Cosas de Z88
			//

			if (MACHINE_IS_Z88) {
				z80_byte srunsbit=blink_com >> 6;
				sprintf (textoregistros,"SRUN: %01d SBIT: %01d SNZ: %01d COM: %01d",(srunsbit>>1)&1,srunsbit&1,z88_snooze.v,z88_coma.v);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}


			//
			// Copper de TBBlue
			//

			if (MACHINE_IS_TBBLUE) {
				sprintf (textoregistros,"COPPER PC: %04XH CTRL: %02XH",tbblue_copper_pc,tbblue_copper_get_control_bits() );
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}


			//
			// Video zx80/81
			//
			if (MACHINE_IS_ZX8081) {
				sprintf (textoregistros,"LNCTR: %x LCNTR %s ULAV: %s",(video_zx8081_linecntr &7),(video_zx8081_linecntr_enabled.v ? "On" : "Off"),
					(video_zx8081_ula_video_output == 0 ? "+5V" : "0V"));
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}



			//
    		//Paginas memoria
			//
            char textopaginasmem[100];
			menu_debug_get_memory_pages(textopaginasmem);

			int max_longitud=31;
			//limitar a 31 por si acaso

    		//Si paging enabled o no, scr
    		char buffer_paging_state[32];
    		debug_get_paging_screen_state(buffer_paging_state);

    		//Si cabe, se escribe
    		int longitud_texto1=strlen(textopaginasmem);

    		//Lo escribo y ya lo limitará debajo a 31
			sprintf(&textopaginasmem[longitud_texto1]," %s",buffer_paging_state);


			textopaginasmem[max_longitud]=0;
    		//menu_escribe_linea_opcion(linea++,-1,1,textopaginasmem);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textopaginasmem);


		}




	return linea;

}

z80_bit menu_breakpoint_exception_pending_show={0};
int continuous_step=0;


/*
obsoleto
int menu_debug_registers_get_height_ventana_vista(void)
{
	int alto_ventana;

        if (menu_debug_registers_current_view==7) {
                alto_ventana=5;
        }

        else if (menu_debug_registers_current_view==8) {
                alto_ventana=16;
        }		

        else {
                alto_ventana=24;
        }

	return alto_ventana;	
}
*/

/*
obsoleto
void menu_debug_registers_zxvision_ventana_set_height(zxvision_window *w)
{

	int alto_ventana=menu_debug_registers_get_height_ventana_vista();

    

	zxvision_set_visible_height(w,alto_ventana);
}
*/

void menu_debug_registers_set_title(zxvision_window *w)
{
    char titulo[33];

	//En vista daad, meter otro titulo
	if (menu_debug_registers_current_view==8) {
		sprintf(w->window_title,"%s Debug",util_undaad_unpaws_ungac_get_parser_name() );
		return;
	}

    //menu_debug_registers_current_view

    //Por defecto
                    //0123456789012345678901
    sprintf (titulo,"Debug CPU           V%d",menu_debug_registers_current_view);

    if (menu_breakpoint_exception_pending_show.v==1 || menu_breakpoint_exception.v) {
                       //0123456789012345678901
        sprintf (titulo,"Debug CPU (brk cnd) V%d",menu_debug_registers_current_view);
        //printf ("breakpoint pending show\n");
    }
    else {

        if (cpu_step_mode.v) {
                                                                    //0123456789012345678901
            if (indice_debug_cpu_backwards_history) sprintf (titulo,"Debug CPU (bckstep) V%d",menu_debug_registers_current_view);
                                //0123456789012345678901
            else sprintf (titulo,"Debug CPU (step)    V%d",menu_debug_registers_current_view);

            menu_footer_activity("STEP");
        }
        //printf ("no breakpoint pending show\n");
    }

    //Poner numero de vista siempre en posicion 23
    //sprintf (&titulo[23],"%d",menu_debug_registers_current_view);

	strcpy(w->window_title,titulo);
}

/*
obsoleto
void menu_debug_registers_ventana_common(zxvision_window *ventana)
{
	//Cambiar el alto visible segun la vista actual
	menu_debug_registers_zxvision_ventana_set_height(ventana);

	ventana->can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll	
}
*/

void menu_debug_registers_zxvision_ventana(zxvision_window *ventana)
{


	int xorigin,yorigin,alto_ventana,ancho_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;
    //en este caso no usamos ancho_antes_minimize,alto_antes_minimize, pues estamos usando 
    //zxvision_new_window_nocheck_staticsize en vez de zxvision_new_window_gn_cim

	if (!util_find_window_geometry("debugcpu",&xorigin,&yorigin,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
		xorigin=menu_origin_x();
		yorigin=0;
		ancho_ventana=32;
		alto_ventana=24;
	}


	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	//zxvision_new_window_nocheck_staticsize(ventana,xorigin,yorigin,ancho_ventana,alto_ventana,ancho_ventana,alto_ventana-2,"Debug CPU");

    zxvision_new_window_gn_cim(ventana,xorigin,yorigin,ancho_ventana,alto_ventana,ancho_ventana,alto_ventana-2,"Debug CPU",
        "debugcpu",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);


	//Preservar ancho y alto anterior
	//menu_debug_registers_ventana_common(ventana);


	ventana->can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll
	ventana->can_be_backgrounded=1;
	//indicar nombre del grabado de geometria
	//strcpy(ventana->geometry_name,"debugcpu");
    //restaurar estado minimizado de ventana
    //ventana->is_minimized=is_minimized;    

	//Puede enviar hotkeys con raton
	ventana->can_mouse_send_hotkeys=1;


}



void menu_debug_registers_gestiona_breakpoint(void)
{
    menu_breakpoint_exception.v=0;
		menu_breakpoint_exception_pending_show.v=1;
    cpu_step_mode.v=1;

    //printf ("Reg pc: %d\n",reg_pc);
		continuous_step=0;

}

void menu_watches_daad(void)
{
		char string_line[10];
		char buffer_titulo[32];

		

		sprintf (buffer_titulo,"Line? (1-%d)",MENU_DEBUG_NUMBER_FLAGS_OBJECTS);
		string_line[0]=0;
        menu_ventana_scanf(buffer_titulo,string_line,2);
		int linea=parse_string_to_number(string_line);		
		if (linea<1 || linea>MENU_DEBUG_NUMBER_FLAGS_OBJECTS) return;
		linea--; //indice empieza en 0



        int tipo=menu_simple_two_choices("Watch type","Type","Flag","Object");
        if (tipo==0) return; //ESC	
		tipo--; //tipo empieza en 0


		string_line[0]=0;
		char ventana_titulo[33];

		char tipo_watch[10];

		int limite_max;

		if (tipo==0) {
			limite_max=util_daad_get_limit_flags();
			strcpy(tipo_watch,"Flag");
		}
		else {
			limite_max=util_daad_get_limit_objects();
			strcpy(tipo_watch,"Object");
		}

		

		sprintf (ventana_titulo,"%s? (max %d)",tipo_watch,limite_max);
		menu_ventana_scanf(ventana_titulo,string_line,4);
		int indice=parse_string_to_number(string_line);
	

		if (indice<0 || indice>limite_max) {
			menu_error_message("Out of range");
			return;
		}


		debug_daad_flag_object[linea].tipo=tipo;
		debug_daad_flag_object[linea].indice=indice;	
}



zxvision_window *menu_watches_overlay_window;

void menu_watches_overlay_mostrar_texto(void)
{
 int linea;

    linea=1; //Empezar justo en cada linea Result

    
  
				char buf_linea[32];

				//char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

				int i;

				for (i=0;i<DEBUG_MAX_WATCHES;i++) {
					
                        int error_code;

                        int resultado=exp_par_evaluate_token(debug_watches_array[i],MAX_PARSER_TOKENS_NUM,&error_code);
                        /* if (error_code) {
                                //printf ("%d\n",tokens[0].tipo);
                                menu_generic_message_format("Error","Error evaluating parsed string: %s\nResult: %d",
                                string_detoken,resultado);
                        }
                        else {
                                menu_generic_message_format("Result","Parsed string: %s\nResult: %d",
                                string_detoken,resultado);
                        }
						*/



	                sprintf (buf_linea,"  Result: %d",resultado); 
					zxvision_print_string_defaults_fillspc(menu_watches_overlay_window,1,linea,buf_linea);

					linea+=2;

								
				}

}



void menu_watches_overlay(void)
{

    if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

 	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_watches_overlay_window->is_minimized) return;

    //printf("overlay watches %d\n",contador_segundo);

 

    menu_watches_overlay_mostrar_texto();
    zxvision_draw_window_contents(menu_watches_overlay_window);

}



void menu_watches_edit(MENU_ITEM_PARAMETERS)
{
        int watch_index=valor_opcion;

  char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];

    exp_par_tokens_to_exp(debug_watches_array[watch_index],string_texto,MAX_PARSER_TOKENS_NUM);

  menu_ventana_scanf("Watch",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

  debug_set_watch(watch_index,string_texto);

  menu_muestra_pending_error_message(); //Si se genera un error derivado del set watch, mostrarlo

}



zxvision_window zxvision_window_watches;

//Esta funcion no se llama realmente desde una opcion de menu, y por tanto deberia ser con parametros (void),
//pero dado que está en el listado de zxvision_known_window_names_array, debe ser con este parámetro
void menu_watches(MENU_ITEM_PARAMETERS)
{


       //Si es modo debug daad
       if (menu_debug_registers_current_view==8) {
        menu_watches_daad();
               return;
       }

	
	
	//Watches normales

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

    zxvision_window *ventana;
    ventana=&zxvision_window_watches;

    //IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
    //si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
    //la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
    zxvision_delete_window_if_exists(ventana);


    int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

	if (!util_find_window_geometry("watches",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {

        xventana=menu_origin_x();
        yventana=1;

        ancho_ventana=32;
        alto_ventana=22;
	}



	//zxvision_new_window(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Watches");
    zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Watches","watches",
            is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);  

	ventana->can_be_backgrounded=1;	
	//indicar nombre del grabado de geometria
	//strcpy(ventana->geometry_name,"watches");
    //restaurar estado minimizado de ventana
    //ventana->is_minimized=is_minimized;    

	zxvision_draw_window(ventana);		



    //Cambiamos funcion overlay de texto de menu
    set_menu_overlay_function(menu_watches_overlay);

	menu_watches_overlay_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui	


	//Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
	//Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
	if (zxvision_currently_restoring_windows_on_start) {
			//printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
			return;
	}	

    menu_item *array_menu_watches_settings;
    menu_item item_seleccionado;
    int retorno_menu;						

    do {

		//Valido tanto para cuando multitarea es off y para que nada mas entrar aqui, se vea, sin tener que esperar el medio segundo 
		//que he definido en el overlay para que aparezca
		menu_watches_overlay_mostrar_texto();

        int lin=0;

		
		
		int i;

		char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

		menu_add_item_menu_inicial(&array_menu_watches_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);
		char texto_expresion_shown[27];


		for (i=0;i<DEBUG_MAX_WATCHES;i++) {
			
			//Convertir token de watch a texto 
			if (debug_watches_array[i][0].tipo==TPT_FIN) {
				strcpy(string_detoken,"None");
			}
			else exp_par_tokens_to_exp(debug_watches_array[i],string_detoken,MAX_PARSER_TOKENS_NUM);

			//Limitar a 27 caracteres
			menu_tape_settings_trunc_name(string_detoken,texto_expresion_shown,27);

 			menu_add_item_menu_format(array_menu_watches_settings,MENU_OPCION_NORMAL,menu_watches_edit,NULL,"%2d: %s",i+1,texto_expresion_shown);

			//En que linea va
			menu_add_item_menu_tabulado(array_menu_watches_settings,1,lin);		

			//Indicamos el indice
			menu_add_item_menu_valor_opcion(array_menu_watches_settings,i);
		

			lin+=2;			
		}	
		
				

    retorno_menu=menu_dibuja_menu(&menu_watches_opcion_seleccionada,&item_seleccionado,array_menu_watches_settings,"Watches" );

	if (retorno_menu!=MENU_RETORNO_BACKGROUND) {

	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
        

				//Nombre de ventana solo aparece en el caso de stdout
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");



									//restauramos modo normal de texto de menu para llamar al editor de watch
                                                                //con el sprite encima
                                    set_menu_overlay_function(normal_overlay_texto_menu);


                                      
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

								set_menu_overlay_function(menu_watches_overlay);
								zxvision_clear_window_contents(ventana); //limpiar de texto anterior en linea de watch
								zxvision_draw_window(ventana);


                                
                        }
                }
	}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && retorno_menu!=MENU_RETORNO_BACKGROUND);

	//Antes de restaurar funcion overlay, guardarla en estructura ventana, por si nos vamos a background
	zxvision_set_window_overlay_from_current(ventana);

       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);

        //En caso de menus tabulados, suele ser necesario esto. Si no, la ventana se quedaria visible
	   

	util_add_window_geometry_compact(ventana);	   


	if (retorno_menu==MENU_RETORNO_BACKGROUND) {
        zxvision_message_put_window_background();
    }

    else {	

		//En caso de menus tabulados, es responsabilidad de este de liberar ventana
		zxvision_destroy_window(ventana);			   

	}


}





void menu_debug_registers_set_view(zxvision_window *ventana,int vista)
{

	zxvision_clear_window_contents(ventana);

	if (vista<1 || vista>8) vista=1;

	//Si no es daad, no permite seleccionar vista 8
	if (vista==8 && !util_daad_detect() && !util_textadv_detect_paws_quill()) return;

	menu_debug_registers_current_view=vista;

    //no hacer nada mas de lo de abajo, cambiar vista no quiero que recree ni redimensione ventana nunca mas
    //esto tenia sentido hace tiempo cuando no existia ZX Vision y las ventanas eran estaticas y no redimensionables por el usuario
    /*

	
	//Dado que se cambia de vista, podemos estar en vista 7 , por ejemplo, que es pequeña, y el alto total es minimo,
	//y si se cambiara a vista 1 por ejemplo, es una vista mayor pero el alto total no variaria y no se veria mas que las primeras 3 lineas
	//Entonces, tenemos que destruir la ventana y volverla a crear
	 

	

	int ventana_x=ventana->x;
	int ventana_y=ventana->y;
	int ventana_visible_width=ventana->visible_width;

	//El alto es el que calculamos segun la vista actual. x,y,ancho los dejamos tal cual estaban
	int ventana_visible_height=menu_debug_registers_get_height_ventana_vista();


	zxvision_destroy_window(ventana);

	//Cerrar la ventana y volverla a crear pero cambiando maximo alto

	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	zxvision_new_window(ventana,ventana_x,ventana_y,ventana_visible_width,ventana_visible_height,ventana_visible_width,ventana_visible_height-2,"Debug CPU");	

	menu_debug_registers_ventana_common(ventana);

    */

}

/*
void menu_debug_registers_splash_memory_zone(void)
{

	menu_debug_set_memory_zone_attr();

	char textofinal[200];
	char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
	int zone=menu_get_current_memory_zone_name_number(zone_name);
	//machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);

	sprintf (textofinal,"Zone number: %d\nName: %s\nSize: %d (%d KB)", zone,zone_name,
		menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);

	menu_generic_message_splash("Memory Zone",textofinal);


}
*/

//Actualmente nadie usa esta funcion. Para que queremos cambiar la zona (en un menu visible) y luego hacer splash?
//antes tenia sentido pues el cambio de zona de memoria no era con menu, simplemente saltaba a la siguiente
/*void menu_debug_change_memory_zone_splash(void)
{
	menu_debug_change_memory_zone();

	menu_debug_registers_splash_memory_zone();


}
*/

void menu_debug_cpu_step_over(void)
{
  //Si apunta PC a instrucciones RET o JP, hacer un cpu-step
  if (si_cpu_step_over_jpret()) {
          debug_printf(VERBOSE_DEBUG,"Running only cpu-step as current opcode is JP or RET");
	  cpu_core_loop();
          return;
  }


  debug_cpu_step_over();


}


void menu_debug_cursor_up(void)
{


		if (menu_debug_line_cursor>0) {
			menu_debug_line_cursor--;
		}

                                        if (menu_debug_view_has_disassemly() ) { //Si vista con desensamblado
                                                menu_debug_memory_pointer=menu_debug_disassemble_subir(menu_debug_memory_pointer);
                                        }
                                        else {  //Vista solo hexa
                                                menu_debug_memory_pointer -=menu_debug_registers_print_registers_longitud_opcode;
                                        }
}


void menu_debug_cursor_down(zxvision_window *w)
{
		if (menu_debug_line_cursor<get_menu_debug_num_lineas_full(w)-1) {
			menu_debug_line_cursor++;
		}

                                        if (menu_debug_view_has_disassemly() ) { //Si vista con desensamblado
                                                menu_debug_memory_pointer=menu_debug_disassemble_bajar(menu_debug_memory_pointer);
                                        }
                                        else {  //Vista solo hexa
                                                menu_debug_memory_pointer +=menu_debug_registers_print_registers_longitud_opcode;
                                        }

}




void menu_debug_cursor_pgup(zxvision_window *w)
{

                                        int lineas=menu_debug_get_main_list_view(w);


                                        int i;
                                        for (i=0;i<lineas;i++) {
						menu_debug_cursor_up();
                                        }
}


void menu_debug_cursor_pgdn(zxvision_window *w)
{

                                        int lineas=menu_debug_get_main_list_view(w);


                                        int i;
                                        for (i=0;i<lineas;i++) {
                                                menu_debug_cursor_down(w);
                                        }

}

int menu_debug_breakpoint_is_daad(char *texto)
{
	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	if (!strcasecmp(texto,breakpoint_add)) return 1;
	else return 0;
}

int menu_debug_breakpoint_is_daad_runtoparse(char *texto)
{
	char breakpoint_add[64];

	debug_get_daad_runto_parse_string(breakpoint_add);

	if (!strcasecmp(texto,breakpoint_add)) return 1;
	else return 0;
}

//Si estamos haciendo un step to step de daad
z80_bit debug_stepping_daad={0};

//Si estamos haciendo un runto parse daad
z80_bit debug_stepping_daad_runto_parse={0};

//Si hay metido un breakpoint de daad en el interprete y con registro A para el condact ficticio
z80_bit debug_allow_daad_breakpoint={0};

z80_bit debug_daad_breakpoint_runtoparse_fired={0};

void menu_breakpoint_fired(char *s) 
{
/*
//Si mostrar aviso cuando se cumple un breakpoint
int debug_show_fired_breakpoints_type=0;
//0: siempre
//1: solo cuando condicion no es tipo "PC=XXXX"
//2: nunca
*/
	int mostrar=0;

	int es_pc_cond=debug_text_is_pc_condition(s);

	//printf ("es_pc_cond: %d\n",es_pc_cond);

	if (debug_show_fired_breakpoints_type==0) mostrar=1;
	if (debug_show_fired_breakpoints_type==1 && !es_pc_cond) mostrar=1;

	if (mostrar) {
		//Si no era un breakpoint de daad de step-to-step o runtoparse

		int esta_en_parser=0;
		if (util_daad_detect() ) {
			if (reg_pc==util_daad_get_pc_parser()) esta_en_parser=1;
		}

		if (util_textadv_detect_paws_quill()){
			if (reg_pc==util_paws_get_pc_parser()) esta_en_parser=1;
		}
        //printf("esta_en_parser: %d\n",esta_en_parser);
        //printf("debug_stepping_daad.v: %d debug_stepping_daad_runto_parse.v: %d\n",debug_stepping_daad.v,debug_stepping_daad_runto_parse.v);

		if ( (debug_stepping_daad.v || debug_stepping_daad_runto_parse.v) && esta_en_parser ) {

		}
		else {
            //printf("PC=%04XH\n",reg_pc);
            menu_generic_message_format("Breakpoint","Breakpoint fired: %s",catch_breakpoint_message);
        }
	}

	//Forzar follow pc
	menu_debug_follow_pc.v=1;



	//Si breakpoint disparado es el de daad
	if (menu_debug_breakpoint_is_daad(catch_breakpoint_message)) {
		//Accion es decrementar PC e incrementar BC
		debug_printf (VERBOSE_DEBUG,"Catch daad breakpoint. Decrementing PC and incrementing BC");
		reg_pc --;
		BC++;
	}

	//Si breakpoint disparado es el de daad runtoparse
	if (menu_debug_breakpoint_is_daad_runtoparse(catch_breakpoint_message)) {
		//Activamos un flag que se lee desde el menu debug cpu
		debug_printf (VERBOSE_DEBUG,"Catch daad breakpoint runtoparse");
		debug_daad_breakpoint_runtoparse_fired.v=1;
	}

}


void menu_debug_ret(void)
{
	if (CPU_IS_Z80) {
		reg_pc=pop_valor();
	}

	else {
		menu_warn_message("Ret operation only supported on Z80 cpu");
	}
}

void menu_debug_toggle_breakpoint(void)
{
	//Buscar primero direccion que indica el cursor
	menu_z80_moto_int direccion_cursor;

	//direccion_cursor=menu_debug_lines_addresses[menu_debug_line_cursor];
	direccion_cursor=menu_debug_memory_pointer;

	debug_printf (VERBOSE_DEBUG,"Address on cursor: %X",direccion_cursor);

	//Si hay breakpoint ahi, quitarlo
	int posicion=debug_return_brk_pc_dir_condition(direccion_cursor);
	if (posicion>=0) {
		debug_printf (VERBOSE_DEBUG,"Clearing breakpoint at index %d",posicion);
		debug_clear_breakpoint(posicion);

	}

	//Si no, ponerlo
	else {

		char condicion[30];
		sprintf (condicion,"PC=%XH",direccion_cursor);

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",condicion);

		debug_add_breakpoint_free(condicion,""); 
	}
}

void menu_debug_runto(void)
{
	//Buscar primero direccion que indica el cursor
	menu_z80_moto_int direccion_cursor;

	//direccion_cursor=menu_debug_lines_addresses[menu_debug_line_cursor];
	direccion_cursor=menu_debug_memory_pointer;

	debug_printf (VERBOSE_DEBUG,"Address on cursor: %X",direccion_cursor);

	//Si no hay breakpoint ahi, ponerlo
	int posicion=debug_return_brk_pc_dir_condition(direccion_cursor);
	if (posicion<0) {

		char condicion[30];
		sprintf (condicion,"PC=%XH",direccion_cursor);

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",condicion);

		debug_add_breakpoint_free(condicion,""); 
	}

	//Y salir
}




//Quitar todas las apariciones de dicho breakpoint, por si ha quedado alguno desactivado, y al agregar uno, aparecen dos
void menu_debug_delete_daad_step_breakpoint(void)
{

	char breakpoint_add[64];

	debug_get_daad_step_breakpoint_string(breakpoint_add);

	debug_delete_all_repeated_breakpoint(breakpoint_add);

}

void menu_debug_daad_step_breakpoint(void)
{


	//Antes quitamos cualquier otra aparicion
	menu_debug_delete_daad_step_breakpoint();	

	char breakpoint_add[64];
	debug_get_daad_step_breakpoint_string(breakpoint_add);

	debug_add_breakpoint_ifnot_exists(breakpoint_add);

	debug_stepping_daad.v=1;

	//Si no hay breakpoint ahi, ponerlo
	/*int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion<0) {

		debug_get_daad_step_breakpoint_string(breakpoint_add);

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;
                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,""); 
	}
*/
	//Y salir
}


//Quitar todas las apariciones de dicho breakpoint, por si ha quedado alguno desactivado, y al agregar uno, aparecen dos
void menu_debug_delete_daad_parse_breakpoint(void)
{

	char breakpoint_add[64];

	debug_get_daad_runto_parse_string(breakpoint_add);

	debug_delete_all_repeated_breakpoint(breakpoint_add);

}

void menu_debug_daad_parse_breakpoint(void)
{

	//Antes quitamos cualquier otra aparicion
	menu_debug_delete_daad_parse_breakpoint();	

	char breakpoint_add[64];
	debug_get_daad_runto_parse_string(breakpoint_add);

	debug_add_breakpoint_ifnot_exists(breakpoint_add);

}

void menu_debug_daad_runto_parse(void)
{
	menu_debug_daad_parse_breakpoint();
	debug_stepping_daad_runto_parse.v=1;
    //printf("menu_debug_daad_runto_parse. debug_stepping_daad_runto_parse.v=%d\n",debug_stepping_daad_runto_parse.v);
}


//Quitar todas las apariciones de dicho breakpoint, por si ha quedado alguno desactivado, y al agregar uno, aparecen dos
void menu_debug_delete_daad_special_breakpoint(void)
{

	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	debug_delete_all_repeated_breakpoint(breakpoint_add);

}



void menu_debug_add_daad_special_breakpoint(void)
{

	//Antes quitamos cualquier otra aparicion
	menu_debug_delete_daad_special_breakpoint();

	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	debug_add_breakpoint_ifnot_exists(breakpoint_add);

	//Si no hay breakpoint ahi, ponerlo
	/*int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion<0) {

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,""); 
	}*/

	//Y salir
}





/*void menu_debug_toggle_daad_breakpoint(void)
{
	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	//Si no hay breakpoint ahi, ponerlo
	int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion>=0) {
		debug_printf (VERBOSE_DEBUG,"Clearing breakpoint at index %d",posicion);
		debug_clear_breakpoint(posicion);
	}

	else {

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,""); 
	}

	//Y salir
}*/





int menu_debug_registers_show_ptr_text(zxvision_window *w,int linea)
{

	debug_printf (VERBOSE_DEBUG,"Refreshing ptr");


	char buffer_mensaje[64];
	char buffer_mensaje_short[64];
	char buffer_mensaje_long[64];
                //Forzar a mostrar atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
                menu_writing_inverse_color.v=1;


                                //Mostrar puntero direccion
                                menu_debug_memory_pointer=adjust_address_memory_size(menu_debug_memory_pointer);


				if (menu_debug_registers_current_view!=7 && menu_debug_registers_current_view!=8) {

                                char string_direccion[10];
                                menu_debug_print_address_memory_zone(string_direccion,menu_debug_memory_pointer);

				char maxima_vista='7';


				if (util_daad_detect() || util_textadv_detect_paws_quill() ) maxima_vista='8';

								sprintf(buffer_mensaje_short,"P~~tr:%sH [%c] ~~FlwPC ~~1-~~%c:View",
                                        string_direccion,(menu_debug_follow_pc.v ? 'X' : ' '),maxima_vista );

								sprintf(buffer_mensaje_long,"Poin~~ter:%sH [%c] ~~FollowPC ~~1-~~%c:View",
                                        string_direccion,(menu_debug_follow_pc.v ? 'X' : ' '),maxima_vista );



								menu_get_legend_short_long(buffer_mensaje,w->visible_width,buffer_mensaje_short,buffer_mensaje_long);


								//sprintf(buffer_mensaje,"P~~tr:%sH ~~FlwPC:%s ~~1-~~%c:View",
                                //        string_direccion,(menu_debug_follow_pc.v ? "Yes" : "No"),maxima_vista );

                                
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

				}

	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

	return linea;
}

void menu_debug_switch_follow_pc(void)
{
	menu_debug_follow_pc.v ^=1;
	
	//if (follow_pc.v==0) menu_debug_memory_pointer=menu_debug_register_decrement_half(menu_debug_memory_pointer);
}






void menu_debug_get_legend(int linea,char *s,zxvision_window *w)
{

	int ancho_visible=w->visible_width;

	switch (linea) {

		//Primera linea
		case 0:


			if (menu_debug_registers_current_view==8) {
							//01234567890123456789012345678901
							// chReg Brkp. Toggle Runto Watch		

				char step_condact_buffer[32];
				if (!util_daad_is_in_parser() && !util_paws_is_in_parser()) {
					strcpy(step_condact_buffer,"~~E~~n:runTo Condact");
				}
				else {
					strcpy(step_condact_buffer,"~~E~~n:Step Condact");
				}

				if (util_daad_detect()) sprintf(s,"%s [%c] Daadbr~~kpnt",step_condact_buffer,(debug_allow_daad_breakpoint.v ? 'X' : ' '));
				else sprintf(s,"%s",step_condact_buffer);
				return;
			}


			//Modo step mode
			if (cpu_step_mode.v) {
				if (menu_debug_registers_current_view==1) {

					menu_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// StM DAsm En:Stp StOvr CntSt Md	
							"~~StM ~~D~~Asm ~~E~~n:Stp St~~Ovr ~~CntSt ~~Md",
								//          10        20        30        40        50        60
								//012345678901234567890123456789012345678901234567890123456789012
							//     StepMode DisAssemble Enter:Step StepOver ContinuosStep Mode

							"~~StepMode ~~Dis~~Assemble ~~E~~n~~t~~e~~r:Step Step~~Over ~~ContinousStep ~~Mode"
					
					); 

				
				}

				else {

					menu_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// StpM DAsm Ent:Stp Stovr ContSt
							"~~StpM ~~D~~Asm ~~E~~n~~t:Stp St~~Ovr ~~ContSt",
								//          10        20        30        40        50        60
								//012345678901234567890123456789012345678901234567890123456789012
							//     StepMode Disassemble Enter:Step StepOver ContinuosStep 

							"~~StepMode ~~Dis~~Assemble ~~E~~nter:Step Step~~Over ~~ContinousStep"
					
					);

				}
			}

			//Modo NO step mode
			else {
				if (menu_debug_registers_current_view==1) {

					menu_get_legend_short_long(s,ancho_visible,

							//01234567890123456789012345678901
							// Stepmode Disassem Assem Mode
							"~~StepMode ~~Disassem ~~Assem ~~Mode",

							//012345678901234567890123456789012345678901234567890123456789012
							// StepMode Disassemble Assemble Mode
							"~~StepMode ~~Disassemble ~~Assemble ~~Mode"
					);



				}
				else {			
							//01234567890123456789012345678901
							// Stepmode Disassemble Assemble				
					sprintf(s,"~~StepMode ~~Disassemble ~~Assemble");
				}
			}
		break;


		//Segunda linea
		case 1:


			if (menu_debug_registers_current_view==8) {
				//de momento solo el run to parse en daad. en quill o paws no tiene sentido, dado que no usan el condacto "PARSE"
				//solo se usa en psi en paws
				if (util_daad_detect()) sprintf(s,"runtoafter~~Parse ~~Watch Wr~~ite M~~essages");
				else sprintf(s,"~~Watch Wr~~ite M~~essages");
				return;
			}

			if (menu_debug_registers_current_view==1) {

				menu_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// Chr brk wtch Togl Run Runto Ret	
							  "Ch~~r ~~brk ~~wtch Tog~~l Ru~~n R~~unto R~~et",

							// Changeregisters breakpoints watch Toggle Run Runto Ret	
							//012345678901234567890123456789012345678901234567890123456789012
							  "Change~~registers ~~breakpoints ~~watches Togg~~le Ru~~n R~~unto R~~et"
				);
			}

			else {

				menu_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// changeReg Breakpoints Watches					
							  "Change~~reg ~~breakpoints ~~watches",

							// Changeregisters breakpoints watches
							//012345678901234567890123456789012345678901234567890123456789012
							  "Change~~registers ~~breakpoints ~~watches"

				);

			}
		break;


		//Tercera linea
		case 2:

			if (menu_debug_registers_current_view==8) {

                char buffer_temp_graphics[100];
                buffer_temp_graphics[0]=0;

                if (util_daad_has_graphics()) strcpy(buffer_temp_graphics,"~~Graphics ");

				if (util_daad_condact_uses_message() ) sprintf(s,"%sCo~~nnections ~~advmap cond~~Message",buffer_temp_graphics);
				else sprintf(s,"%sCo~~nnections ~~advmap",buffer_temp_graphics);
				return;
			}

			char buffer_intermedio_short[128];
			char buffer_intermedio_long[128];

            //Tecla graficos para GAC
            char buffer_temp_gac_short[32];
            char buffer_temp_gac_long[32];
            buffer_temp_gac_short[0]=0;
            buffer_temp_gac_long[0]=0;

            if (util_gac_detect() ) {
                strcpy(buffer_temp_gac_short,"~~Gph ");
                strcpy(buffer_temp_gac_long,"~~Graphics ");
            }

			if (cpu_step_mode.v) {

							//01234567890123456789012345678901
							// ClrTstPart Write VScr MemZn 99	
				sprintf (buffer_intermedio_short,"ClrTst~~Part Wr~~ite ~~VScr %sMem~~Zn %d",buffer_temp_gac_short,menu_debug_memory_zone);
							//012345678901234567890123456789012345678901234567890123456789012
							// ClearTstatesPartial Write ViewScreen MemoryZone 99	
				sprintf (buffer_intermedio_long,"ClearTstates~~Partial Wr~~ite ~~ViewScreen %sMemory~~Zone %d",buffer_temp_gac_long,menu_debug_memory_zone);


				menu_get_legend_short_long(s,ancho_visible,buffer_intermedio_short,buffer_intermedio_long);
			}
			else {
							//01234567890123456789012345678901
							// Clrtstpart Write MemZone 99
				sprintf (buffer_intermedio_short,"ClrTst~~Part Wr~~ite %sMem~~Zone %d",buffer_temp_gac_short,menu_debug_memory_zone);

							//012345678901234567890123456789012345678901234567890123456789012
							// ClearTstatesPartial Write MemoryZone 99	
				sprintf (buffer_intermedio_long,"ClearTstates~~Partial Wr~~ite %sMemory~~Zone %d",buffer_temp_gac_long,menu_debug_memory_zone);

				menu_get_legend_short_long(s,ancho_visible,buffer_intermedio_short,buffer_intermedio_long);

			}
		break;

		//Cuarta linea
		case 3:
            if (menu_debug_registers_current_view==8) {
                //cadena vacia
                s[0]=0;
            }
            else {
                char string_nextpcbr[32];
                char string_backwards[32];

                if (debug_breakpoints_enabled.v) strcpy(string_nextpcbr," nextpc~^Brk");
                else string_nextpcbr[0]=0;


                if (cpu_step_mode.v && cpu_history_enabled.v && cpu_history_started.v) {
                    strcpy(string_backwards," back~^Step");
                    //printf("hay backstep\n");
                }

                else {
                    //printf("NO hay backstep\n");
                    string_backwards[0]=0;
                }                

                char string_history[32];
                if (CPU_IS_Z80) strcpy(string_history," cpu~^Hist");
                else string_history[0]=0;


                sprintf(s,"set~^Pc=ptr%s%s%s",string_nextpcbr,string_history,string_backwards);

                sprintf (buffer_intermedio_short,"~^Pc=ptr%s%s%s%s ~~F~~1:help",
                    ( debug_breakpoints_enabled.v ? " nxtpc~^Br" : "" ),
                    (CPU_IS_Z80 ? " st~~k" : ""),
                    (CPU_IS_Z80 ? " cpu~^Hst" : ""),
                    (cpu_step_mode.v && cpu_history_enabled.v && cpu_history_started.v ? " b~^Stp bru~^N" : "")
                    
                );                


                sprintf (buffer_intermedio_long,"set~^Pc=ptr%s%s%s%s ~~F~~1:help",
                    ( debug_breakpoints_enabled.v ? " nextpc~^Brk" : "" ),
                    (CPU_IS_Z80 ? " stac~~k" : ""),
                    (CPU_IS_Z80 ? " cpu~^Hist" : ""),
                    (cpu_step_mode.v && cpu_history_enabled.v && cpu_history_started.v ? " back~^Step backru~^N" : "")
                    
                );


                menu_get_legend_short_long(s,ancho_visible,buffer_intermedio_short,buffer_intermedio_long);
            }
        break;
	}
}

//0= pausa de 0.5
//1= pausa de 0.1
//2= pausa de 0.02
//3= sin pausa
int menu_debug_continuous_speed=0;

//Posicion del indicador para dar sensacion de velocidad. De 0 a 10
int menu_debug_continuous_speed_step=0;

void menu_debug_registers_next_cont_speed(void)
{
	menu_debug_continuous_speed++;
	if (menu_debug_continuous_speed==4) menu_debug_continuous_speed=0;
}

 

//Si borra el menu a cada pulsacion y muestra la pantalla de la maquina emulada debajo
void menu_debug_registers_if_cls(void)
{
                                
	//A cada pulsacion de tecla, mostramos la pantalla del ordenador emulado
	if (debug_settings_show_screen.v) {
		cls_menu_overlay();
		menu_refresca_pantalla();

		//Y forzar en este momento a mostrar pantalla
		//scr_refresca_pantalla_solo_driver();
		//printf ("refrescando pantalla\n");
	}

    if (menu_multitarea==0) {
        //printf ("Esperamos menu_multitarea = 0\n");
        menu_espera_no_tecla_no_cpu_loop();
    }


    else {
        if (cpu_step_mode.v==1) {
            //printf ("Esperamos cpu_step_mode=1\n");
            //menu_emulation_paused_on_menu
            int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
            menu_emulation_paused_on_menu=1;
            //menu_espera_no_tecla_no_cpu_loop();
            menu_espera_no_tecla();
            menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
            //printf ("Despues esperamos cpu_step_mode=1\n");
        }
        else {
            //printf ("Esperamos cpu_step_mode.v=0\n");
            menu_espera_no_tecla();
        }
    }


}


void menu_debug_cont_speed_progress(char *s)
{

	int max_position=19;
	//Meter caracteres con .
	int i;
	for (i=0;i<max_position;i++) s[i]='.';
	s[i]=0;

	//Meter tantas franjas > como velocidad
	i=menu_debug_continuous_speed_step;
	int caracteres=menu_debug_continuous_speed+1;

	while (caracteres>0) {
		s[i]='>';
		i++;
		if (i==max_position) i=0; //Si se sale por la derecha
		caracteres--;
	}

	menu_debug_continuous_speed_step++;
	if (menu_debug_continuous_speed_step==max_position) menu_debug_continuous_speed_step=0; //Si se sale por la derecha
}



/*
int screen_generic_getpixel_indexcolour(z80_int *destino,int x,int y,int ancho);
*/

#define ANCHO_SCANLINE_CURSOR 32

//Buffer donde guardamos el contenido anterior del cursor de scanline, antes de meter el cursor
int menu_debug_registers_buffer_precursor[ANCHO_SCANLINE_CURSOR];
int menu_debug_registers_buffer_pre_x=-1; //posicion anterior del cursor
int menu_debug_registers_buffer_pre_y=-1;

void menu_debug_showscan_putpixel(z80_int *destino,int x,int y,int ancho,int color)
{

	screen_generic_putpixel_indexcolour(destino,x,y,ancho,color);	

}

void menu_debug_registers_show_scan_pos_putcursor(int x_inicial,int y)
{

	int ancho,alto;

	ancho=get_total_ancho_rainbow();
	alto=get_total_alto_rainbow();

    //rojo, amarillo, verde, cyan
    int colores_rainbow[]={2+8,6+8,4+8,5+8};

	int x;
    int indice_color=0;

	//printf ("inicial %d,%d\n",x_inicial,y);

	if (x_inicial<0 || y<0) return;

	//TBBlue tiene doble de alto. El ancho ya lo viene multiplicado por 2 al entrar aqui
	if (MACHINE_IS_TBBLUE) y *=2;		

	//Restauramos lo que habia en la posicion anterior del cursor
	if (menu_debug_registers_buffer_pre_x>=0 && menu_debug_registers_buffer_pre_y>=0) {
	        for (x=0;x<ANCHO_SCANLINE_CURSOR;x++) {
	            int x_final=menu_debug_registers_buffer_pre_x+x;


				if (x_final<ancho) {
					int color_antes=menu_debug_registers_buffer_precursor[x];
					menu_debug_showscan_putpixel(rainbow_buffer,x_final,menu_debug_registers_buffer_pre_y,ancho,color_antes);
				}
			}
	}



	menu_debug_registers_buffer_pre_x=x_inicial;
	menu_debug_registers_buffer_pre_y=y;


	if (x_inicial<0) return;

	for (x=0;x<ANCHO_SCANLINE_CURSOR;x++) {
		int x_final=x_inicial+x;


		//Guardamos lo que habia antes de poner el cursor
		if (x_final<ancho) {
			int color_anterior;

			//printf ("%d, %d\n",x_final,y);

			if (y>=0 && y<alto && x>=0 && x<ancho) {

				color_anterior=screen_generic_getpixel_indexcolour(rainbow_buffer,x_final,y,ancho);

				menu_debug_registers_buffer_precursor[x]=color_anterior;

				//Y ponemos pixel
			
	    		menu_debug_showscan_putpixel(rainbow_buffer,x_final,y,ancho,colores_rainbow[indice_color]);
			}
		}



		//Trozos de colores de 4 pixeles de ancho
		if (x>0 && (x%8)==0) {
			indice_color++;
			if (indice_color==4) indice_color=0;
		}


    }
}




void menu_debug_registers_show_scan_position(void)
{

	if (menu_debug_registers_if_showscan.v==0) return;

	if (rainbow_enabled.v) {
		//copiamos contenido linea y border a buffer rainbow
/*
//temp mostrar contenido buffer pixeles y atributos
printf ("pixeles y atributos:\n");
int i;
for (i=0;i<224*2/4;i++) printf ("%02X ",scanline_buffer[i]);
printf ("\n");
*/

		if (MACHINE_IS_SPECTRUM) {
			screen_store_scanline_rainbow_solo_border();
			screen_store_scanline_rainbow_solo_display();
		}

		//Obtener posicion x e y e indicar posicion visualmente

		int si_salta_linea;
		int x,y;
		x=screen_get_x_coordinate_tstates(&si_salta_linea);

		y=screen_get_y_coordinate_tstates();

		//En caso de TBBLUE, doble de ancho

		if (MACHINE_IS_TBBLUE) x*=2;

		menu_debug_registers_show_scan_pos_putcursor(x,y+si_salta_linea);

				

	}
                                
}


int menu_debug_registers_print_legend(zxvision_window *w,int linea)
{



     if (menu_debug_registers_current_view!=7) {
		char buffer_mensaje[128];

				menu_debug_get_legend(0,buffer_mensaje,w);                                
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

				menu_debug_get_legend(1,buffer_mensaje,w);                            
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

				menu_debug_get_legend(2,buffer_mensaje,w);                                
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

				menu_debug_get_legend(3,buffer_mensaje,w);                                
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);                

      }

	return linea;

}




int menu_debug_registers_get_line_legend(zxvision_window *w)
{

	if (menu_debug_registers_current_view!=8) return get_menu_debug_num_lineas_full(w)+5; //19;
	else return 12; //get_menu_debug_num_lineas_full(w)-3; //12;


}	


void menu_debug_daad_edit_flagobject(void)
{
		char string_line[10];
		char buffer_titulo[32];

		
        int tipo=menu_simple_two_choices("Watch type","Type","Flag","Object");
        if (tipo==0) return; //ESC	
		tipo--; //tipo empieza en 0
		
		if (tipo==0) strcpy (buffer_titulo,"Flag to modify?");
		else strcpy (buffer_titulo,"Object to modify?");

		string_line[0]=0;
		menu_ventana_scanf(buffer_titulo,string_line,4);
		int indice=parse_string_to_number(string_line);
		if (indice<0 || indice>255) return;

		string_line[0]=0;
		menu_ventana_scanf("Value to set?",string_line,4);
		int valor=parse_string_to_number(string_line);
		if (valor<0 || valor>255) return;		

		if (tipo==0) {
			util_daad_put_flag_value(indice,valor);
		}

		else {
			util_daad_put_object_value(indice,valor);
		}

}

//Rutina para ver diferentes mensajes de Daad, segun tipo
//0=Objects
//1=User messages
//2=System messages
//3=Locations messages
//4=Compressed messages
//5=Vocabulary
void menu_debug_daad_view_messages(MENU_ITEM_PARAMETERS)
{

	int total_messages;
	char window_title[64];
	void (*funcion_mensajes) (z80_byte index,char *texto);

	char titulo_parser[20];

	strcpy(titulo_parser,util_undaad_unpaws_ungac_get_parser_name() );
	//char *entry_message;

	switch (valor_opcion) {
		case 1:
			total_messages=util_daad_get_num_user_messages();
			funcion_mensajes=util_daad_get_user_message;
			sprintf(window_title,"%s User Messages",titulo_parser);
			//entry_message="Message";
		break;

		case 2:
			total_messages=util_daad_get_num_sys_messages();
			funcion_mensajes=util_daad_get_sys_message;
			sprintf(window_title,"%s System Messages",titulo_parser);
			//entry_message="Sys Message";
		break;		

		case 3:
			total_messages=util_daad_get_num_locat_messages();
			funcion_mensajes=util_daad_get_locat_message;
			sprintf(window_title,"%s Locations Messages",titulo_parser);
			//entry_message="Location Message";
		break;		

		case 4:
			total_messages=128;
			funcion_mensajes=util_daad_get_compressed_message;
			sprintf(window_title,"%s Compression Tokens",titulo_parser);
			//entry_message="Compressed Message";
		break;		

		case 5:
			strcpy(window_title,"Vocabulary");
		break;		

		default:
			total_messages=util_daad_get_num_objects_description();
			funcion_mensajes=util_daad_get_object_description;
			sprintf(window_title,"%s Objects",titulo_parser);
			//entry_message="Object";
		break;
	}

	int i;

    char *texto=util_malloc_max_texto_generic_message("Can not allocate memory for showing messages");
	texto[0]=0;

	int resultado=0;


	if (valor_opcion==5) { 
			if (util_daad_detect() ) util_daad_dump_vocabulary(1,texto,MAX_TEXTO_GENERIC_MESSAGE);
			else util_paws_dump_vocabulary_tostring(1,texto,MAX_TEXTO_GENERIC_MESSAGE);
	}

	else {

		for (i=0;i<total_messages && !resultado;i++) {

			char buffer_temp[256];
			funcion_mensajes(i,buffer_temp); 
			//printf ("object %d: %s\n",i,buffer_temp);

			char buffer_linea[300];
			sprintf(buffer_linea,"%03d: %s\n",i,buffer_temp);

			//Y concatenar a final
			resultado=util_concat_string(texto,buffer_linea,MAX_TEXTO_GENERIC_MESSAGE);

		}
	}

	if (resultado) menu_warn_message("Reached maximum text size. Showing only allowed text");

	menu_generic_message(window_title,texto);

    free(texto);
}



void menu_debug_daad_connections(void)
{

    
	//int i;

    char *texto=util_malloc_max_texto_generic_message("Can not allocate memory for showing messages");
	texto[0]=0;

	//int resultado=0;

    util_textdaventure_dump_connections(texto,MAX_TEXTO_GENERIC_MESSAGE);


	menu_generic_message("Connections",texto);

    free(texto);
}




void menu_debug_daad_view_messages_ask(void)
{

	menu_item *array_menu_daad_tipo_mensaje;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

		

	    menu_add_item_menu_inicial_format(&array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Objects");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'o');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,0);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~User Messages");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'u');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,1);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~System Messages");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'s');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,2);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Locations");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'l');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,3);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Compression Tokens");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'c');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,4);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Vocabulary");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'v');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,5);
    

        menu_add_item_menu(array_menu_daad_tipo_mensaje,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_ESC_item(array_menu_daad_tipo_mensaje);

        retorno_menu=menu_dibuja_menu(&daad_tipo_mensaje_opcion_seleccionada,&item_seleccionado,array_menu_daad_tipo_mensaje,"Message type" );
                

		/*if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			menu_debug_daad_view_messages(daad_tipo_mensaje_opcion_seleccionada);

		}

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);*/




		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);	


}






//Muestra mensaje relacionado con condacto
void menu_debug_daad_get_condact_message(void)
{



	char buffer[256];
	
	util_daad_get_condact_message(buffer);
	menu_generic_message("Message",buffer);


}

//extern void zxvision_putpixel(zxvision_window *w,int x,int y,int color);

#define RENDER_PAWS_START_X_DRAW 1
#define RENDER_PAWS_START_Y_DRAW 5

//para reducir dibujo para poderlo mostrar en miniaturas de mapa
int paws_render_total_escalado=1;
int paws_render_total_offset_x=0;
int paws_render_total_offset_y=0;

void render_paws_putpixel(zxvision_window *w,int x,int y,int color)
{
    //putpixel teniendo el 0 abajo del todo
    y=175-y;

    //Si sale de margenes, no hacer putpixel
    if (y<0 || y>175 || x<0 || x>255) return;

    int y_final;
    int x_final;

    //Y sumar margen inicio pantalla
    if (paws_render_total_escalado==1) {
        y_final=y+RENDER_PAWS_START_Y_DRAW*8;
        x_final=x+RENDER_PAWS_START_X_DRAW*menu_char_width;
        zxvision_putpixel(w,x_final,y_final,color);
    }

    //en caso de incrustado en adventure map
    else {
        y_final=y;
        x_final=x;

        x_final /=paws_render_total_escalado;
        y_final /=paws_render_total_escalado;


        x_final +=map_adventure_offset_x;
        y_final +=map_adventure_offset_y;

        x_final +=paws_render_total_offset_x;
        y_final +=paws_render_total_offset_y;

        textadv_map_putpixel(w,x_final,y_final,color);

    }




    
}




zxvision_window zxvision_window_paws_render;

zxvision_window *menu_debug_daad_view_graphics_render_overlay_window;


int menu_debug_daad_view_graphics_render_localizacion=0;

//Para poder llamar de manera recursiva, estos parámetros se definen fuera

//x,y ultima no se usa en gac
int paws_render_last_x=0;
int paws_render_last_y=0;
int paws_render_ink=0;
int paws_render_paper=7;
int paws_render_bright=0;
int paws_render_mirror_x=+1;
int paws_render_mirror_y=+1;
int paws_render_escala=0;

int gac_render_default_ink=0;
int gac_render_default_paper=7;

//Coordenadas iniciales, importantes para cuando no hay un plot inicial (sobretodo subrutinas pero tambien algun picture)
int paws_render_initial_x=0;
int paws_render_initial_y=0;

//Habilitar/deshabilitar comandos en render
z80_bit paws_render_disable_block={0};
z80_bit paws_render_disable_gosub={0};
z80_bit paws_render_disable_plot={0};
z80_bit paws_render_disable_line={0};
z80_bit paws_render_disable_text={0};
z80_bit paws_render_disable_ink={0};
z80_bit paws_render_disable_bright={0};
z80_bit paws_render_disable_paper={0};

z80_bit paws_render_disable_rectangle={0};
z80_bit paws_render_disable_ellipse={0};



//Renderiza y/o retorna lista de comandos de una pantalla grafica en GAC
//si buffer_texto_comandos=NULL, no rellena texto
//si w==NULL, no dibuja nada
void menu_debug_daad_view_graphics_render_recursive_gac(zxvision_window *w,z80_byte location,int nivel_recursivo,char *buffer_texto_comandos)
{



    z80_int puntero_grafico;

    z80_byte gflag;
    char buffer_temporal[200];    


    //int contador_habitacion_gac=0;

    int longitud_habitacion_gac;


        

    //ungac de https://www.seasip.info/Unix/UnQuill/

    puntero_grafico=peek_word_no_time(0xA52F);

    //Info:
    //word: location
    //word: longitud contando estos 4 bytes
    //byte: numero comandos
    //comandos...



    int location_id;

    puntero_grafico=util_gac_get_graphics_location(location,&location_id);

    longitud_habitacion_gac=peek_byte_no_time(puntero_grafico++);
    //printf("longitud habitacion: %d\n",longitud_habitacion_gac);
    


    sprintf(buffer_temporal,"Location %-3d ID %d\n",location,
        location_id);

    if (buffer_texto_comandos!=NULL) {
        util_concat_string(buffer_texto_comandos,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);
    }



    //Solo hacer cambio de ink y paper si no es subrutina
    if (nivel_recursivo==0) {
        paws_render_ink=gac_render_default_ink;
        paws_render_paper=gac_render_default_paper;
        paws_render_bright=0;

        /*
        if (paws_render_disable_ink.v) {
            paws_render_ink=0;
            paws_render_paper=7;
            paws_render_bright=0;
        }
        */
  
        //Rellenamos ventana con color indicado
        //tener en cuenta char width
       
       
        int ancho_rellenar=256/menu_char_width;
        
        int rellena_x,rellena_y;
        for (rellena_y=RENDER_PAWS_START_Y_DRAW;rellena_y<RENDER_PAWS_START_Y_DRAW+24;rellena_y++) {
            for (rellena_x=RENDER_PAWS_START_X_DRAW;rellena_x<RENDER_PAWS_START_X_DRAW+ancho_rellenar;rellena_x++) {
                if (w!=NULL) zxvision_print_char_simple(w,rellena_x,rellena_y,gac_render_default_ink,
                            gac_render_default_paper,0,' ');
            }
        }
         
    }





    while (longitud_habitacion_gac>0) {

        gflag=peek_byte_no_time(puntero_grafico++);

        longitud_habitacion_gac--;

        //z80_byte nargs;



        int x1,x2,y1,y2;
                


        z80_int id_localizacion;  


        //Leer los siguientes 4 parámetros, algunos usados en diferentes comandos
        z80_byte parm0_byte=peek_byte_no_time(puntero_grafico);                       
        z80_byte parm1_byte=peek_byte_no_time(puntero_grafico+1);                       
        z80_byte parm2_byte=peek_byte_no_time(puntero_grafico+2);
        z80_byte parm3_byte=peek_byte_no_time(puntero_grafico+3);



        switch (gflag) {
            case 0x01:
                sprintf(buffer_temporal,"BORDER  %3d\n", parm0_byte);
                puntero_grafico++;
            break;

            case 0x02:
                sprintf(buffer_temporal,"PLOT    %3d %3d\n", parm0_byte, parm1_byte);
                puntero_grafico += 2;

                    
                if (paws_render_disable_plot.v==0 && w!=NULL) {
                    render_paws_putpixel(w,parm0_byte,parm1_byte,paws_render_ink+paws_render_bright*8);
                }      

            break;

            case 0x03:
                sprintf(buffer_temporal,"ELLIPSE %3d %3d %3d %3d\n",
                        parm0_byte, parm1_byte,
                        parm2_byte, parm3_byte);
                puntero_grafico += 4;

                x1=parm0_byte;
                y1=parm1_byte;

                int radio_x=parm2_byte-x1;
                int radio_y=parm3_byte-y1;

                if (paws_render_disable_ellipse.v==0 && w!=NULL) {
                    zxvision_draw_ellipse(w,x1,y1,radio_x,radio_y,paws_render_ink+paws_render_bright*8,render_paws_putpixel,360);
                }

            break;

            case 0x04:
                sprintf(buffer_temporal,"FILL    %3d %3d\n", parm0_byte, parm1_byte); 
                puntero_grafico += 2;
            break;

            case 0x05:
                sprintf(buffer_temporal,"BGFILL  %3d %3d\n", parm0_byte, parm1_byte); 
                puntero_grafico += 2;
            break;
            
            case 0x06:
                sprintf(buffer_temporal,"SHADE   %3d %3d\n", parm0_byte, parm1_byte); 
                puntero_grafico += 2;
            break;

            case 0x07:
                id_localizacion=parm1_byte * 256 + parm0_byte;
                sprintf(buffer_temporal,"CALL    %d\n", id_localizacion);
                puntero_grafico += 2;


                if (paws_render_disable_gosub.v==0 && w!=NULL) {

                    //Saltar a subrutina
                    if (nivel_recursivo>=10) {
                        //printf("Maximum nested gosub reached\n");
                    }
                    else {                         

                        //Buscar el numero de habitacion
                        int nueva_ubicacion=util_gac_get_index_location_by_id(id_localizacion);
                        //printf("Call. Nueva ubicacion para indice %d es %d\n",id_localizacion,nueva_ubicacion);
                        if (nueva_ubicacion>=0) {

                            //Al llamar a subrutina pone buffer a texto a null, para que no meta
                            //lista de comandos de la subrutina
                            //Esto solo cambiaria algo en el supuesto caso en que dibujamos con putpixel (w no es NULL) y
                            //aqui buffer_texto_comandos viene con no NULL (o sea, que dibujamos y listamos texto)
                            menu_debug_daad_view_graphics_render_recursive_gac(w,nueva_ubicacion,nivel_recursivo+1,NULL);
                            //printf("llamar recursivo text=%p w=%p\n",buffer_texto_comandos,w);
                        }
                        
                    }
                }


            break;

            case 0x08:
                sprintf(buffer_temporal,"RECT    %3d %3d %3d %3d\n",
                        parm0_byte, parm1_byte,
                        parm2_byte, parm3_byte);
                puntero_grafico += 4;

                x1=parm0_byte;
                y1=parm1_byte;

                x2=parm2_byte;
                y2=parm3_byte;                        

                if (paws_render_disable_rectangle.v==0 && w!=NULL) {
                    //Abajo                        
                    zxvision_draw_line(w,x1,y1,x2,y1,paws_render_ink+paws_render_bright*8,render_paws_putpixel);
                    //Arriba
                    zxvision_draw_line(w,x1,y2,x2,y2,paws_render_ink+paws_render_bright*8,render_paws_putpixel);
                    //Izquierda
                    zxvision_draw_line(w,x1,y1,x1,y2,paws_render_ink+paws_render_bright*8,render_paws_putpixel);
                    //Derecha
                    zxvision_draw_line(w,x2,y1,x2,y2,paws_render_ink+paws_render_bright*8,render_paws_putpixel);
                }                    
            break;

            case 0x09:
                sprintf(buffer_temporal,"LINE    %3d %3d %3d %3d\n",
                        parm0_byte, parm1_byte,
                        parm2_byte, parm3_byte);
                puntero_grafico += 4;

                
                x1=parm0_byte;
                y1=parm1_byte;

                x2=parm2_byte;
                y2=parm3_byte;                        

                if (paws_render_disable_line.v==0) {
                    if (w!=NULL) zxvision_draw_line(w,x1,y1,x2,y2,paws_render_ink+paws_render_bright*8,render_paws_putpixel);
                }

            break;

            case 0x10:
                sprintf (buffer_temporal,"INK     %3d\n",parm0_byte);  
                if (paws_render_disable_ink.v==0) paws_render_ink=parm0_byte & 7;
                puntero_grafico++;
            break;

            case 0x11:
                sprintf(buffer_temporal,"PAPER   %3d\n", parm0_byte);
                if (paws_render_disable_paper.v==0) paws_render_paper=parm0_byte;                
                puntero_grafico++;
            break;

            case 0x12:
                sprintf(buffer_temporal,"BRIGHT  %3d\n", parm0_byte);
                puntero_grafico++;
                if (paws_render_disable_bright.v==0) paws_render_bright=parm0_byte&1;
            break;

            case 0x13:
                sprintf(buffer_temporal,"FLASH   %3d\n", parm0_byte);
                puntero_grafico++;
            break;

            default: 
                sprintf(buffer_temporal,"OP%02x\n", gflag);
            
            break;                    


        }
            
     
        if (buffer_texto_comandos!=NULL) {
            //printf("Agregando texto %s\n",buffer_temporal);
            util_concat_string(buffer_texto_comandos,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);
        }


    }


}

int paws_render_default_ink=0;
int paws_render_default_paper=7;

#define DAAD_RENDER_GRAPHICS_MAX_NESTED_LEVELS 10
#define DAAD_RENDER_GRAPHICS_MAX_TOTAL_CALLS 100


//Renderiza y/o retorna lista de comandos de una pantalla grafica de quill, paws o daad
//si buffer_texto_comandos=NULL, no rellena texto
//si w==NULL, no dibuja nada
//retorna en p_total_comandos, p_total_tamanyo si no son NULL, total de comandos del dibujo y total de bytes

//contador_limite es para evitar errores con aventuras a medio cargar, que pueden quedarse en bucle haciendo gosubs y retornos,
//como al usar el Text Adventure Map, con zoom 5, y recargar chichen itza 1
//es un puntero dado que actualizamos en origen, para que siempre se incremente y no varie al llamarse recursivamente
//controla el maximo de veces totales que se llama a dicha funcion
void menu_debug_daad_view_graphics_render_recursive(zxvision_window *w,z80_byte location,int nivel_recursivo,char *buffer_texto_comandos,int *p_total_comandos,int *p_total_tamanyo,int *contador_limite)
{

    
    //int i;

    //printf("menu_debug_daad_view_graphics_render_recursive location: %d nivel_recursivo: %d contador_limite: %d\n",
    //    location,nivel_recursivo,*contador_limite);

    (*contador_limite)++;

    //printf("menu_debug_daad_view_graphics_render_recursive contador_limite: %d\n",*contador_limite);


    if ((*contador_limite)>DAAD_RENDER_GRAPHICS_MAX_TOTAL_CALLS) {
        debug_printf(VERBOSE_DEBUG,"Total limit of graphics render calls reached (%d). Usually it means error in graphics table",DAAD_RENDER_GRAPHICS_MAX_TOTAL_CALLS);
        return;
    }


    z80_int table_dir=util_daad_get_start_graphics();

    if (table_dir==0) {
        //menu_error_message("Graphics not found");
        //printf("Graphics not found\n");
        //zxvision_draw_window_contents(w);
        return;
    }

    z80_byte gflag;

    char buffer_temporal[200];

    int esdaad=util_daad_detect();


    //z80_int table_attr=util_daad_get_start_graphics_attr();

    int tinta_attr,paper_attr;
    int is_picture;
    z80_int table_attr=util_daad_get_graphics_attr(location,&tinta_attr,&paper_attr,&is_picture);    

    if (table_attr==0) {
        //menu_error_message("Graphics attributes not found");
        //printf("Graphics attributes not found\n");
        //zxvision_draw_window_contents(w);
        return;
    }        


   //Nota: El bit 6 del byte de attr no sé para que sirve y por tanto no lo muestro

    sprintf(buffer_temporal,"Location %-3d graphics flags: %s Ink=%d Paper=%d\n",location,
        (is_picture ? "Picture " : "Subroutine "),
        tinta_attr, paper_attr 
    );

    if (buffer_texto_comandos!=NULL) {
        util_concat_string(buffer_texto_comandos,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);
    }



    //Solo hacer cambio de ink y paper si no es subrutina
    if (is_picture) {
        paws_render_ink=tinta_attr;
        paws_render_paper=paper_attr;

        if (paws_render_disable_ink.v) {
            paws_render_ink=paws_render_default_ink;
            paws_render_paper=paws_render_default_paper;
            paws_render_bright=0;
        }        
  
        //Rellenamos ventana con color indicado
        //tener en cuenta char width
        //Solo si escala global es 1, pues de otra manera se trataria de dibujado dentro del mapa de conexiones
        if (paws_render_total_escalado==1) {
            int ancho_rellenar=256/menu_char_width;
            
            int rellena_x,rellena_y;
            for (rellena_y=RENDER_PAWS_START_Y_DRAW;rellena_y<RENDER_PAWS_START_Y_DRAW+24;rellena_y++) {
                for (rellena_x=RENDER_PAWS_START_X_DRAW;rellena_x<RENDER_PAWS_START_X_DRAW+ancho_rellenar;rellena_x++) {
                    if (w!=NULL) zxvision_print_char_simple(w,rellena_x,rellena_y,paws_render_ink+paws_render_bright*8,
                                paws_render_paper+paws_render_bright*8,0,' ');
                }
            }
        }
    }




    //Puntero a ese grafico concreto


    z80_int puntero_grafico=util_daad_get_graphics_location(location);

    z80_int original_puntero_grafico=puntero_grafico;

    //printf("Start graphics location %d: %d\n",location,graphics);
    //util_daad_get_message_table_lookup(index,table_dir,texto,util_daad_get_num_locat_messages() );

/*
char *plot_moves[]= {
" 001  000",
" 001  001",
" 000  001",
"-001  001",
"-001  000",
"-001 -001",
" 000 -001",
" 001 -001" 
};
*/ 

int new_plot_moves[8][2]={
    {0,   1},
    {1,   1},
    {1,   0},
    {-1,  1},
    {0,  -1},
    {-1, -1},
    {-1,  0},
    {1,  -1}     
};

    int salir=0;

    //z80_int neg[2];

    int signo[2];

    z80_int maintop;
    z80_int mainattr;

    int quillversion;

    util_unpaws_get_maintop_mainattr(&maintop,&mainattr,&quillversion);

    int total_comandos_parseados=0;


    while (!salir) {
        int line_comprimido=0;
        //printf("%d\n",puntero_grafico);
        gflag=peek_byte_no_time(puntero_grafico);
        //z80_byte nargs;

        total_comandos_parseados++;

        z80_byte value;
        char inv, ovr;

        int mirror_x,mirror_y;

        //int parm3;
        int parm0;
        int parm1;
        int parm2;
                

        //int estexto=0;

        //Formato del byte con el comando:
        //-----xxx 3 bits inferiores: comando
        //----x--- Bit 3 (0x08) : over / flags        -|
        //---x---- Bit 4 (0x10) : inverse / flags      |
        //--x----- Bit 5 (0x20): flags                 |  Parametro 0 ("value")
        //-x------ Bit 6 (0x40): signo parametro 1    -|
        //x------- Bit 7 (0x80): signo parametro 2 / flags   

        //neg[0]=neg[1]=0;
        signo[0]=signo[1]=+1;

        inv = ' '; ovr = ' ';
        if ((gflag & 8) != 0) ovr = 'o';
        if ((gflag & 16) !=0) inv = 'i';
        value = gflag /  8;
        
        //nargs=0;    

        int dibujar;    

        puntero_grafico++;


        //Leer los siguientes 4 parámetros, algunos usados en diferentes comandos
        z80_byte parm0_byte=peek_byte_no_time(puntero_grafico);                       
        z80_byte parm1_byte=peek_byte_no_time(puntero_grafico+1);                       
        z80_byte parm2_byte=peek_byte_no_time(puntero_grafico+2);
        z80_byte parm3_byte=peek_byte_no_time(puntero_grafico+3);

        switch (gflag & 7) {

            //ABS MOVE, PLOT
            case 0:

                dibujar=1;

                paws_render_last_x=parm0_byte;
                paws_render_last_y=parm1_byte;

                if ((ovr=='o') && (inv=='i')) {
                    sprintf (buffer_temporal,"ABS MOVE   %4d %4d\n",paws_render_last_x,paws_render_last_y);
                    dibujar=0;
                }
                else {
                    sprintf (buffer_temporal,"PLOT  %c%c   %4d %4d\n",ovr,inv,paws_render_last_x,paws_render_last_y);
                }

                if (dibujar && paws_render_disable_plot.v==0 && w!=NULL) {

                    int color_tinta=paws_render_ink+paws_render_bright*8;
                    if (inv=='i' && paws_render_disable_ink.v==0) color_tinta=paws_render_paper+paws_render_bright*8;

                    render_paws_putpixel(w,paws_render_last_x,paws_render_last_y,color_tinta);
                }

                puntero_grafico +=2;
                
            break;

            //REL MOVE, LINE
            case 1: 
                
                if ((gflag & 0x40) != 0) signo[0] = -1;
                if ((gflag & 0x80) != 0) signo[1] = -1;

                dibujar=1;

                if (esdaad) {
                    //Ver si tiene compresion
                    if (gflag & 0x20) {
                        line_comprimido=1;
                    }
                }                       
             
                
                if (line_comprimido) {
                    //Formato comprimido usando solo 1 byte para desplazamiento x,y
                    parm0=(parm0_byte>>4)&0xF;
                    parm0 *=signo[0];

                    parm1=(parm0_byte)&0xF;
                    parm1 *=signo[1];

                    puntero_grafico +=1;
                }

                else {
                    parm0=parm0_byte;
                    parm0 *=signo[0];

                    parm1=parm1_byte;
                    parm1 *=signo[1];

                    puntero_grafico +=2;
                }


                if (ovr=='o' && inv=='i') {
                        sprintf (buffer_temporal,"REL MOVE   %4d %4d\n",parm0,parm1);
                        dibujar=0; //solo mover
                }
                else {
                        sprintf (buffer_temporal,"LINE  %c%c   %4d %4d\n",ovr,inv,parm0,parm1);
                }


                int x1=paws_render_last_x;
                int y1=paws_render_last_y;

                parm0 *=paws_render_mirror_x;
                parm1 *=paws_render_mirror_y;

                //-firfurcio localizacion 11 usa varios gosub con scale
                //aplicar escala en curso
                int multpli=paws_render_escala;
                if (multpli==0) multpli=8;

                parm0=(parm0*multpli)/8;
                parm1=(parm1*multpli)/8;
                       
                //Punto final
                int x2=x1+parm0;
                int y2=y1+parm1;

                if (dibujar && paws_render_disable_line.v==0) {
                    int color_tinta=paws_render_ink+paws_render_bright*8;
                    if (inv=='i' && paws_render_disable_ink.v==0) color_tinta=paws_render_paper+paws_render_bright*8;
                    //Juanito y su baloncito tiene alguna pantalla con inverse (la 4)      

                    if (w!=NULL) zxvision_draw_line(w,x1,y1,x2,y2,color_tinta,render_paws_putpixel);
                }

                paws_render_last_x=x2;
                paws_render_last_y=y2;

                
		    break;


            //SHADE, BSHADE, BLOCK, SHADE, FILL
            case 2:
                
                if ((gflag & 0x10)!=0  && (gflag & 0x20)!=0)  {
		      
                    if ((gflag & 0x40) !=0) signo[0] = -1;
                    if ((gflag & 0x80) !=0) signo[1] = -1;
                        
                    parm0=parm0_byte*signo[0];
                    parm1=parm1_byte*signo[1];
                    parm2=parm2_byte;

                    puntero_grafico +=3;
        
                    if (quillversion==0) {
                        sprintf (buffer_temporal,"SHADE %c%c   %4d %4d %4d\n",ovr,inv,parm0,parm1,parm2);
                    }
                    else {
                        sprintf (buffer_temporal,"BSHADE     %4d %4d %4d\n",parm0,parm1,parm2);
                    }
                }


		        else if ((gflag & 0x10) !=0) {
      
                    z80_byte x1,y1,x2,y2;
                    z80_byte ancho,alto;

                    x1=parm2_byte;
                    y1=parm3_byte;
                    ancho=parm1_byte;
                    alto=parm0_byte;

                    puntero_grafico +=4;

                    sprintf (buffer_temporal,"BLOCK      %4d %4d %4d %4d\n",x1,y1,ancho,alto);

                    //Tener en cuenta char width
                    int temp_x=((x1+RENDER_PAWS_START_X_DRAW)*8)/menu_char_width;
                    x1=temp_x;

                        
                    //Tener en cuenta char width

                    int temp_ancho=(ancho*8)/menu_char_width;
                    ancho=temp_ancho;

                    x2=x1+ancho;
                    
                    y2=y1+alto;

                    
                    //ordenado. teniendo x1 el mas bajo, y1 el mas bajo
                    if (x1>x2) {
                        z80_byte temp_valor=x1;
                        x1=x2;
                        x2=temp_valor;
                    }

                    if (y1>y2) {
                        z80_byte temp_valor=y1;
                        y1=y2;
                        y2=temp_valor;
                    }


                    //Rellenamos trozo con color indicado
                    int rellena_x,rellena_y;

                    //TODO: eso realmente tiene que cambiar los atributos de una sección
                    //estoy cambiando el texto cuando hay caracter pero no cambia en este caso el color de una posible linea
                    //esto es casi imposible de hacer tal y como dibujo lineas
                    //probar grafico 20 en firfurcio
                    //o grafico 4 de juanito


                    if (paws_render_disable_block.v==0) {
                        rellena_y=y1;
                        for (;rellena_y<=y2;rellena_y++) {
                            rellena_x=x1;
                            for (;rellena_x<=x2;rellena_x++) {

                                    if (w!=NULL) {
                                        //printf("block color ink %d paper %d bright %d\n",paws_render_ink,paws_render_paper,paws_render_bright);
                                        zxvision_set_attr(w,rellena_x,rellena_y+RENDER_PAWS_START_Y_DRAW,
                                            paws_render_ink+paws_render_bright*8,paws_render_paper+paws_render_bright*8,0);
                                    }
                                    

                            
                            }
                        }
                    }

                }

            else if ((gflag & 0x20) !=0) {

                if ((gflag & 0x40) !=0 ) signo[0] = -1;
                if ((gflag & 0x80) !=0 ) signo[1] = -1;

                parm0=parm0_byte*signo[0];
                parm1=parm1_byte*signo[1];
                parm2=parm2_byte;

                puntero_grafico +=3;

                sprintf (buffer_temporal,"SHADE %c%c   %4d %4d %4d\n",ovr,inv,parm0,parm1,parm2);
            }

            else {
                if ((gflag & 0x40) !=0 ) signo[0] = -1;
                if ((gflag & 0x80) !=0 ) signo[1] = -1;
                
                parm0=parm0_byte*signo[0];
                parm1=parm1_byte*signo[1];

                puntero_grafico +=2;

                sprintf (buffer_temporal,"FILL       %4d %4d\n",parm0,parm1);
            }
		    
            
            break;

            //GOSUB
            case 3: 
                                
                mirror_x=(gflag&64 ? -1 : +1);
                mirror_y=(gflag&128 ? -1 : +1);

                z80_byte nueva_ubicacion=parm0_byte;

                puntero_grafico +=1;

                if (!esdaad) mirror_x=mirror_y=+1;

                //Chichen itza, localizacion 4 utiliza esto
                sprintf (buffer_temporal,"GOSUB sc=%d %s %s %4d\n",value & 7,
                        (mirror_x==-1 ? "MX" : "  "),
                        (mirror_y==-1 ? "MY" : "  "),
                        nueva_ubicacion
                );                        

                int escala=value&7;
                    
 
                //Chichen itza, localizacion 4 utiliza mirror_x

                //No hacemos recursivo si no hay puntero zxvision, pues es en el unico modo que tiene sentido,
                //ya que cuando solo se saca la lista de comandos (sin dibujar) no queremos saltar con gosub                
            
                if (paws_render_disable_gosub.v==0 && w!=NULL) {

                    //Saltar a subrutina
                    if (nivel_recursivo>=DAAD_RENDER_GRAPHICS_MAX_NESTED_LEVELS) {
                        //printf("Maximum nested gosub reached\n");
                    }
                    else {                         

                        //cambio temporal mirror
                        int antes_paws_render_mirror_x=paws_render_mirror_x;
                        int antes_paws_render_mirror_y=paws_render_mirror_y;
                        int antes_paws_render_escala=paws_render_escala;

                        paws_render_mirror_x=mirror_x;
                        paws_render_mirror_y=mirror_y;
                        paws_render_escala=escala;


                        //Al llamar a subrutina pone buffer a texto a null, para que no meta
                        //lista de comandos de la subrutina
                        //Esto solo cambiaria algo en el supuesto caso en que dibujamos con putpixel (w no es NULL) y
                        //aqui buffer_texto_comandos viene con no NULL (o sea, que dibujamos y listamos texto)

                        menu_debug_daad_view_graphics_render_recursive(w,nueva_ubicacion,nivel_recursivo+1,NULL,NULL,NULL,contador_limite);
                        //printf("llamar recursivo text=%p w=%p\n",buffer_texto_comandos,w);

                        paws_render_mirror_x=antes_paws_render_mirror_x;
                        paws_render_mirror_y=antes_paws_render_mirror_y;
                        paws_render_escala=antes_paws_render_escala;
                        
                    }
                }
                    
		    break;

            //TEXT, RPLOT
            case 4:
            
                if (quillversion==0) {
                    
                    parm0=parm0_byte;                       
                    parm1=parm1_byte;                       
                    parm2=parm2_byte;  

                    puntero_grafico +=3;

                    sprintf (buffer_temporal,"TEXT %c%c    %4d %4d(%c) %d %d\n",ovr,inv,value/4,parm0,
                            (parm0>=32 && parm0<=126 ? parm0 : '?'),
                            parm1,parm2);

                    //ajustar x a char width
                    int posx=parm1+RENDER_PAWS_START_X_DRAW;
                    posx *=8;
                    posx /= menu_char_width;

                    if (paws_render_disable_text.v==0 && w!=NULL) {
                        int color_tinta=paws_render_ink+paws_render_bright*8;
                        int color_papel=paws_render_paper+paws_render_bright*8;
                        if (inv=='i' && paws_render_disable_ink.v==0) {
                            color_tinta=paws_render_paper+paws_render_bright*8;
                            color_papel=paws_render_ink+paws_render_bright*8;
                        }

                        zxvision_print_char_simple(w,posx,parm2+RENDER_PAWS_START_Y_DRAW,color_tinta,
                            color_papel,0,parm0);
                    }

                }

                else {
                    parm0=new_plot_moves[value/4][0];
                    parm1=new_plot_moves[value/4][1];

                    sprintf (buffer_temporal,"FREEHAND %c%c  %2d   %2d\n",ovr,inv,parm0,parm1);
                    //printf("RPLOT location %d %d %d\n",location,parm0,parm1);

                    paws_render_last_x +=parm0;
                    paws_render_last_y +=parm1;

                    //TODO: no estoy seguro de que el funcionamiento de rplot sea este precisamente

                    //Se puede probar en bugsy localizaciones 50 y 51
                    //(se llaman desde la primera pantalla del juego)

                    if (paws_render_disable_plot.v==0 && w!=NULL) {
                        int color_tinta=paws_render_ink+paws_render_bright*8;
                        if (inv=='i' && paws_render_disable_ink.v==0) color_tinta=paws_render_paper+paws_render_bright*8;                            

                        render_paws_putpixel(w,paws_render_last_x,paws_render_last_y,color_tinta);
                    }

                }
                            

		    break;

            //BRIGHT, PAPER
	        case 5: 
            
                if ((gflag & 0x80) !=0) {
                    sprintf (buffer_temporal,"BRIGHT     %4d\n",value & 15);

                    if (paws_render_disable_bright.v==0) paws_render_bright=value&1;
                }

                else {
                    sprintf (buffer_temporal,"PAPER      %4d\n",value & 15);

                    if (paws_render_disable_paper.v==0) paws_render_paper=value & 15;                         
                }
        
            
            break;

            //FLASH, INK
            case 6: 
                
                if ((gflag & 0x80) !=0)  {
                    sprintf (buffer_temporal,"FLASH      %4d\n",value & 15);
                }

                else {
                    sprintf (buffer_temporal,"INK        %4d\n",value & 15);  
                    if (paws_render_disable_ink.v==0) paws_render_ink=value & 15;
                }
                      
		    
            break;

            //END
            case 7:
                sprintf (buffer_temporal,"END\n");
                salir=1;
            break;
        }

        
        if (buffer_texto_comandos!=NULL) {
            util_concat_string(buffer_texto_comandos,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);
        }
        //printf("%d %s\n",puntero_grafico,buffer_temporal);

    }

    if (p_total_comandos!=NULL) {
        *p_total_comandos=total_comandos_parseados;
    }

    if (p_total_tamanyo!=NULL) {
        int resta=puntero_grafico-original_puntero_grafico;
        //esto sucede en imagenes corruptas
        //da la vuelta. el valor no es real pero para evitar poner un valor negativo
        if (resta<0) resta=65536-original_puntero_grafico;
        *p_total_tamanyo=resta;
    } 

}


void menu_debug_daad_view_graphics_render_overlay(void)
{

    if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_debug_daad_view_graphics_render_overlay_window->is_minimized) return;

    //printf("overlay adventure graphics render %d\n",contador_segundo);

    zxvision_window *w;

    w=menu_debug_daad_view_graphics_render_overlay_window;


    //Por defecto
    paws_render_last_x=paws_render_initial_x;
    paws_render_last_y=paws_render_initial_y;    
    paws_render_ink=0;
    paws_render_paper=7;
    paws_render_bright=0;    
    paws_render_mirror_x=+1;
    paws_render_mirror_y=+1;
    paws_render_escala=0;


    //Si intenta renderizar mas alla de las pantallas definidas
    int max_localizaciones=util_gac_daad_get_total_graphics();


    if (menu_debug_daad_view_graphics_render_localizacion>=max_localizaciones) {
        //printf("limit reached\n");
        menu_debug_daad_view_graphics_render_localizacion=0;
    }


    z80_byte location=menu_debug_daad_view_graphics_render_localizacion;

    int contador_limite=0;

    if (util_gac_detect() ) {
        menu_debug_daad_view_graphics_render_recursive_gac(w,location,0,NULL);
    }

    else menu_debug_daad_view_graphics_render_recursive(w,location,0,NULL,NULL,NULL,&contador_limite);

    zxvision_draw_window_contents(w);
}

void menu_debug_daad_view_graphics_render_next(MENU_ITEM_PARAMETERS)
{
    int max_localizaciones=util_gac_daad_get_total_graphics();

    if (menu_debug_daad_view_graphics_render_localizacion<max_localizaciones-1) {
        menu_debug_daad_view_graphics_render_localizacion++;
    }
}

void menu_debug_daad_view_graphics_render_prev(MENU_ITEM_PARAMETERS)
{
    if (menu_debug_daad_view_graphics_render_localizacion>0) menu_debug_daad_view_graphics_render_localizacion--;
}

void menu_debug_daad_view_graphics_render_list_commands(MENU_ITEM_PARAMETERS)
{
    char *texto=util_malloc_max_texto_generic_message("Can not allocate memory for graphics commands");
	texto[0]=0;

    int contador_limite=0;

    if (util_gac_detect() ) {
        menu_debug_daad_view_graphics_render_recursive_gac(NULL,menu_debug_daad_view_graphics_render_localizacion,0,texto);
    }
    else menu_debug_daad_view_graphics_render_recursive(NULL,menu_debug_daad_view_graphics_render_localizacion,0,texto,NULL,NULL,&contador_limite);

    //util_daad_get_graphics_list_commands(menu_debug_daad_view_graphics_render_localizacion,texto); 
    menu_generic_message("Graphics commands",texto);

    free(texto);
}


void menu_debug_daad_view_graphics_render_set(MENU_ITEM_PARAMETERS)
{

        int max_localizaciones=util_gac_daad_get_total_graphics();


    menu_ventana_scanf_numero_enhanced("Graph number",&menu_debug_daad_view_graphics_render_localizacion,4,+1,0,max_localizaciones-1,0);


}

void menu_debug_daad_view_graphics_render_disable_block(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_block.v ^=1;
}   

void menu_debug_daad_view_graphics_render_disable_gosub(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_gosub.v ^=1;
}

void menu_debug_daad_view_graphics_render_disable_plot(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_plot.v ^=1;
}  

void menu_debug_daad_view_graphics_render_disable_line(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_line.v ^=1;
}  

void menu_debug_daad_view_graphics_render_disable_text(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_text.v ^=1;
}  

void menu_debug_daad_view_graphics_render_disable_ink(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_ink.v ^=1;
}  

void menu_debug_daad_view_graphics_render_disable_paper(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_paper.v ^=1;
}  

void menu_debug_daad_view_graphics_render_disable_bright(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_bright.v ^=1;
}  

void menu_debug_daad_view_graphics_render_disable_rectangle(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_rectangle.v ^=1;
}

void menu_debug_daad_view_graphics_render_disable_ellipse(MENU_ITEM_PARAMETERS)
{
    paws_render_disable_ellipse.v ^=1;
} 

void menu_debug_daad_view_graphics_render_initial_x(MENU_ITEM_PARAMETERS)
{

    menu_ventana_scanf_numero_enhanced("Initial X",&paws_render_initial_x,4,+1,0,255,0);

}

void menu_debug_daad_view_graphics_render_initial_y(MENU_ITEM_PARAMETERS)
{

    menu_ventana_scanf_numero_enhanced("Initial Y",&paws_render_initial_y,4,+1,0,175,0);

}

void menu_debug_daad_view_graphics_render_initial_ink(MENU_ITEM_PARAMETERS)
{

    menu_ventana_scanf_numero_enhanced("Default ink",&gac_render_default_ink,2,+1,0,7,0);

}

void menu_debug_daad_view_graphics_render_initial_paper(MENU_ITEM_PARAMETERS)
{

    menu_ventana_scanf_numero_enhanced("Default paper",&gac_render_default_paper,2,+1,0,7,0);

}


void menu_debug_daad_view_graphics(void)
{
    //Renderizar un grafico de paws

    menu_espera_no_tecla();
    menu_reset_counters_tecla_repeticion();    

    zxvision_window *ventana;
    ventana=&zxvision_window_paws_render; 


    int ancho_ventana,alto_ventana,xventana,yventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

    if (!util_find_window_geometry("textadvgraphics",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
        int ancho_minimo_deseado=41+RENDER_PAWS_START_X_DRAW;

        ancho_ventana=(256/menu_char_width)+7+RENDER_PAWS_START_X_DRAW; //para hacer 32+7=39 en una ventana de char width = 8

        //Minimo para que quepa todo el texto de opciones
        if (ancho_ventana<ancho_minimo_deseado) ancho_ventana=ancho_minimo_deseado;

        alto_ventana=26+RENDER_PAWS_START_Y_DRAW;


        xventana=menu_center_x()-ancho_ventana/2;
        yventana=menu_center_y()-alto_ventana/2;

    }    

    char titulo_ventana[100];
    sprintf(titulo_ventana,"%s Graphics Render",util_undaad_unpaws_ungac_get_parser_name() );

    //zxvision_new_window(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,titulo_ventana);

    zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,titulo_ventana,"textadvgraphics",
        is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);


    zxvision_draw_window(ventana);
    //indicar nombre del grabado de geometria
    //strcpy(ventana->geometry_name,"textadvgraphics");    
    //restaurar estado minimizado de ventana
    //ventana->is_minimized=is_minimized;

    //menu_debug_daad_view_graphics_render_localizacion=localizacion;

    //Cambiamos funcion overlay de texto de menu
    //Por cierto que esta ventana no la permitimos que se haga background. Por que? Quizá no tiene sentido,
    //pues no el contenido no varia a no ser que se esté en la ventana y tocando opciones
    //Ademas, creo que puede provocar efectos inesperados si esta la ventana abierta y cargamos otro juego
    menu_debug_daad_view_graphics_render_overlay_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui    

    set_menu_overlay_function(menu_debug_daad_view_graphics_render_overlay);


    //zxvision_wait_until_esc(ventana);



	//Dado que es una variable local, siempre podemos usar este nombre array_menu_common
	menu_item *array_menu_common;
	menu_item item_seleccionado;
	int retorno_menu;

	int comun_opcion_seleccionada=0;

    do {

        //Borramos cualquier resto de dibujos anteriores
        zxvision_cls(ventana);

        //Le digo que no haga cache del fondo solo la siguiente llamada a zxvision_draw_window_contents
        //Determinadas combinaciones de color de fondo de los graficos y el estilo de gui actual podria hacer que se quedase
        //"rastro" de pixeles por estar usando cache

        //Pasamos de usar 32% de cpu si activamos ventana->must_clear_cache_on_draw,
        //a usar 13% de cpu si solo activo ventana->must_clear_cache_on_draw_once
        ventana->must_clear_cache_on_draw_once=1;

        char buffer_linea[100];

        int tinta,papel,is_picture;

        util_daad_get_graphics_attr(menu_debug_daad_view_graphics_render_localizacion,&tinta,&papel,&is_picture); 

        int es_gac=util_gac_detect();

        if (es_gac) {
            int location_id;

            util_gac_get_graphics_location(menu_debug_daad_view_graphics_render_localizacion,&location_id);


            sprintf(buffer_linea,"Location: %d/%d ID location: %d",menu_debug_daad_view_graphics_render_localizacion,
            util_gac_daad_get_total_graphics(), location_id);

            zxvision_print_string_defaults_fillspc(ventana,1,0,buffer_linea);

            int location_commands,location_size;

            util_gac_get_graphics_size(menu_debug_daad_view_graphics_render_localizacion,&location_commands,&location_size);            

            sprintf(buffer_linea,"Size: %d commands (%d Bytes)",location_commands, location_size);

            zxvision_print_string_defaults_fillspc(ventana,1,1,buffer_linea);            
        }

        else {
            //obtener tamanyo en bytes y comandos
            //desactivar los gosub temporalmente
            int antes_paws_render_disable_gosub=paws_render_disable_gosub.v;
            paws_render_disable_gosub.v=1;

            int location_commands,location_size;

            int contador_limite=0;

            menu_debug_daad_view_graphics_render_recursive(NULL,menu_debug_daad_view_graphics_render_localizacion,0,NULL,&location_commands,&location_size,&contador_limite);
            paws_render_disable_gosub.v=antes_paws_render_disable_gosub;

            //printf("comandos: %d size %d\n",location_commands,location_size);


            //sprintf(buffer_linea,"Location: %d/%d %s Ink %d Paper %d"

            sprintf(buffer_linea,"Location: %d/%d %s",menu_debug_daad_view_graphics_render_localizacion,
            util_gac_daad_get_total_graphics(),
                (is_picture ? "Picture   " : "Subroutine"));

            zxvision_print_string_defaults_fillspc(ventana,1,0,buffer_linea);

            sprintf(buffer_linea,"Size %d commands (%d B) Ink %d Paper %d",
                location_commands,location_size,
                tinta,papel);

            zxvision_print_string_defaults_fillspc(ventana,1,1,buffer_linea);            
        }

        int linea=2;


		menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_prev,NULL,"~~Prev");
		menu_add_item_menu_tabulado(array_menu_common,1,linea);   
        menu_add_item_menu_shortcut(array_menu_common,'p'); 

		menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_next,NULL,"~~Next");
		menu_add_item_menu_tabulado(array_menu_common,6,linea); 
        menu_add_item_menu_shortcut(array_menu_common,'n');

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_set,NULL,"~~Set");
		menu_add_item_menu_tabulado(array_menu_common,11,linea);      
        menu_add_item_menu_shortcut(array_menu_common,'s');

        if (es_gac) {
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_initial_ink,NULL,
                                        "Ink %d",gac_render_default_ink);
            menu_add_item_menu_tabulado(array_menu_common,15,linea);

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_initial_paper,NULL,
                                        "Pap %d",gac_render_default_paper);
            menu_add_item_menu_tabulado(array_menu_common,21,linea);

        }

        else {
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_initial_x,NULL,
                                        "~~x %3d",paws_render_initial_x);
            menu_add_item_menu_tabulado(array_menu_common,15,linea);
            menu_add_item_menu_shortcut(array_menu_common,'x');

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_initial_y,NULL,
                                        "~~y %3d",paws_render_initial_y);
            menu_add_item_menu_tabulado(array_menu_common,21,linea);
            menu_add_item_menu_shortcut(array_menu_common,'y');
        }



        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_list_commands,NULL,"List ~~commands");
        menu_add_item_menu_tabulado(array_menu_common,27,linea);   
        menu_add_item_menu_shortcut(array_menu_common,'c');        


        //0123456789012345678901234567890123456789
        // [X] Gosub [X] Block [X] Text [X] Plot
        // [X] Line [X] Ink [X] Paper [X] Bright

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_gosub,NULL,
            "[%c] ~~Gosub",(paws_render_disable_gosub.v==0 ? 'X' : ' ') );
        menu_add_item_menu_tabulado(array_menu_common,1,linea+1);   
        menu_add_item_menu_shortcut(array_menu_common,'g');

        if (util_gac_detect() ) {
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_rectangle,NULL,
                "[%c] Rec~~t",(paws_render_disable_rectangle.v==0 ? 'X' : ' ') );
            menu_add_item_menu_tabulado(array_menu_common,11,linea+1);   
            menu_add_item_menu_shortcut(array_menu_common,'t');       

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_ellipse,NULL,
                "[%c] ~~Ellip",(paws_render_disable_ellipse.v==0 ? 'X' : ' ') );
            menu_add_item_menu_tabulado(array_menu_common,20,linea+1);   
            menu_add_item_menu_shortcut(array_menu_common,'e');                       
        }
        else {
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_block,NULL,
                "[%c] ~~Block",(paws_render_disable_block.v==0 ? 'X' : ' ') );
            menu_add_item_menu_tabulado(array_menu_common,11,linea+1);   
            menu_add_item_menu_shortcut(array_menu_common,'b');

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_text,NULL,
                "[%c] ~~Text",(paws_render_disable_text.v==0 ? 'X' : ' ') );
            menu_add_item_menu_tabulado(array_menu_common,21,linea+1);   
            menu_add_item_menu_shortcut(array_menu_common,'t');            
        }


        
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_plot,NULL,
            "[%c] Pl~~ot",(paws_render_disable_plot.v==0 ? 'X' : ' ') );
        menu_add_item_menu_tabulado(array_menu_common,30,linea+1);   
        menu_add_item_menu_shortcut(array_menu_common,'o');



        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_line,NULL,
            "[%c] ~~Line",(paws_render_disable_line.v==0 ? 'X' : ' ') );
        menu_add_item_menu_tabulado(array_menu_common,1,linea+2);   
        menu_add_item_menu_shortcut(array_menu_common,'l');

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_ink,NULL,
            "[%c] ~~Ink",(paws_render_disable_ink.v==0 ? 'X' : ' ') );
        menu_add_item_menu_tabulado(array_menu_common,10,linea+2);   
        menu_add_item_menu_shortcut(array_menu_common,'i');
                
        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_paper,NULL,
            "[%c] P~~aper",(paws_render_disable_paper.v==0 ? 'X' : ' ') );
        menu_add_item_menu_tabulado(array_menu_common,18,linea+2);   
        menu_add_item_menu_shortcut(array_menu_common,'a');

        menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_daad_view_graphics_render_disable_bright,NULL,
            "[%c] B~~right",(paws_render_disable_bright.v==0 ? 'X' : ' ') );
        menu_add_item_menu_tabulado(array_menu_common,28,linea+2);   
        menu_add_item_menu_shortcut(array_menu_common,'r');


		retorno_menu=menu_dibuja_menu(&comun_opcion_seleccionada,&item_seleccionado,array_menu_common,"PAWS Graphics Render");


			
			if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
					//llamamos por valor de funcion
					if (item_seleccionado.menu_funcion!=NULL) {
							//printf ("actuamos por funcion\n");
							item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
							
					}
			}

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

        //restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);


    //En caso de menus tabulados, suele ser necesario esto. Si no, la ventana se quedaria visible
    

    //Grabar geometria ventana
    util_add_window_geometry_compact(ventana);    

    //printf("salir_todos_menus %d\n",salir_todos_menus);

    zxvision_destroy_window(ventana);            
}




void menu_debug_registers_zxvision_save_size(zxvision_window *ventana,int *ventana_ancho_antes,int *ventana_alto_antes)
{

	//Guardar ancho y alto anterior para recrear la ventana si cambia
	*ventana_ancho_antes=ventana->visible_width;
	*ventana_alto_antes=ventana->visible_height;
}
	

zxvision_window zxvision_window_menu_debug_registers;

z80_byte menu_debug_cpu_handle_mouse(zxvision_window *ventana)
{
    //printf("menu mouse x %d y %d\n",menu_mouse_x,menu_mouse_y);

    //printf("wheel vertical: %d\n",mouse_wheel_vertical);

    if (!si_menu_mouse_activado()) return 0;

    if (!si_menu_mouse_en_ventana() ) return 0;

    if (!mouse_left) {
        //no pulsado boton izquierdo

        //movida rueda
        if (mouse_wheel_vertical) {

            if (menu_invert_mouse_scroll.v) mouse_wheel_vertical=-mouse_wheel_vertical;

            //teclas
            //10 cursor down
            //11 cursor up            
            if (mouse_wheel_vertical<0) {
                //abajo


                //Desplazamos abajo tanto como diga el wheel - 1 posicion,
                //esa posicion de mas la gestionara al volver de esta funcion con tecla de bajar cursor
                while (mouse_wheel_vertical<-1) {
                	//abajo
                    menu_debug_cursor_down(ventana);
                    mouse_wheel_vertical++;
                }

                mouse_wheel_vertical=0;
                return 10;


            }
            if (mouse_wheel_vertical>0) {
                //arriba

                //Desplazamos abajo tanto como diga el wheel - 1 posicion,
                //esa posicion de mas la gestionara al volver de esta funcion con tecla de subir cursor
                while (mouse_wheel_vertical>1) {
                	//arriba
                    menu_debug_cursor_up();
                    mouse_wheel_vertical--;
                }

                mouse_wheel_vertical=0;
                return 11;
            }

        }

        return 0;
        
    }

    //Pulsado boton izquierdo
    else {

        int inicio_disassemble=2+1; //linea 0 de mouse es el titulo. Contamos desde la 1

        int final_disassemble=inicio_disassemble+get_menu_debug_num_lineas_full(ventana);

        int columna_registros=get_menu_debug_columna_registros(ventana);

        if (menu_mouse_y>=inicio_disassemble && menu_mouse_y<final_disassemble) {
            //printf("mouse pulsado en seccion disassemble o registros\n");
            //Si zona registros
            if (menu_mouse_x>=columna_registros) {
                //printf("mouse pulsado en seccion registros\n");
                return 'r';
            }

            //printf("mouse pulsado en seccion disassemble\n");
            //obtener desplazamiento cursor
            int offset_cursor=menu_mouse_y-inicio_disassemble;


            //menu_debug_follow_pc.v=0; //se deja de seguir pc

            //Primero nos posicionamos en la direccion de arriba del todo
            //TODO: lo mejor sería tener una variable que cuando muestre vista 1 indique la direccion de memoria de la primera linea,
            //para no tener que recalcularla aqui 
            menu_debug_memory_pointer=menu_debug_disassemble_subir_veces(menu_debug_memory_pointer,menu_debug_line_cursor);

            //Cursor a 0
            menu_debug_line_cursor=0;

            //Y bajar hasta donde haya pulsado el raton

            int i;
            for (i=0;i<offset_cursor;i++) {
                menu_debug_cursor_down(ventana);
            }

            return 'l';
        }
    }

    return 0;
}

void menu_debug_registers_run_cpu_opcode(void)
{
    //Decirle que no esperamos final de frame ya
    //Si no hicieramos esto, podria pasar que el core estuviera en final de ejecucion de instrucciones
    //de un frame pero esperando el tiempo del final de ese frame, y por tanto, al lanzar un step de cpu, no ejecutaria ninguna instruccion
    interrupcion_timer_generada.v=1;
    esperando_tiempo_final_t_estados.v=0;
    cpu_core_loop();
}

int debug_cpu_next_breakpoint_pc_dir_alhpasort(menu_z80_moto_int *d1, menu_z80_moto_int *d2)
{

    return (*d1)-(*d2);

}

//Cambia el puntero ptr a siguiente breakpoint de tipo pc=dir
void debug_cpu_next_breakpoint_pc_dir(void)
{

    menu_z80_moto_int lista_breakpoints[MAX_BREAKPOINTS_CONDITIONS];

    int total=debug_return_brk_pc_dir_list(lista_breakpoints);

    if (!total) return;

    int i;

    
    //for (i=0;i<total;i++) {
    //    printf("i: %d = %d\n",i,lista_breakpoints[i]);
    //}

    //ordenar
	//lanzar qsort
	int (*funcion_compar)(const void *, const void *);

	funcion_compar=( int (*)(const void *, const void *)  ) debug_cpu_next_breakpoint_pc_dir_alhpasort;

	qsort(lista_breakpoints,total,sizeof(menu_z80_moto_int), funcion_compar);

    for (i=0;i<total;i++) {
        debug_printf(VERBOSE_DEBUG,"Breakpoint type PC=X sorted list. Item %d = %XH",i,lista_breakpoints[i]);
    }

    //Y ahora establecer Puntero ptr a breakpoint que sea mayor que dicho ptr
    menu_debug_follow_pc.v=0; //se deja de seguir pc

    for (i=0;i<total;i++) {
        if (lista_breakpoints[i]>menu_debug_memory_pointer) {
            menu_debug_memory_pointer=lista_breakpoints[i];
            debug_printf(VERBOSE_INFO,"Setting ptr to %XH",menu_debug_memory_pointer);
            
            return;
        }
    }

    //No hay ninguno mayor. Resetear al primero
    menu_debug_memory_pointer=lista_breakpoints[0];
    debug_printf(VERBOSE_INFO,"Setting ptr to %XH",menu_debug_memory_pointer);
    
}



void menu_debug_cpu_backwards_history(void)
{


    int total_elementos_in_history=cpu_history_get_total_elements();

    if (total_elementos_in_history==0) {
        menu_warn_message("History is empty");
        return;
    }    


    int indice=total_elementos_in_history-indice_debug_cpu_backwards_history-1;

    //Ver si no estamos ya en el ultimo item
    if (indice<0) {
        menu_warn_message("You are at the oldest item");
        return;
    }    

    
    cpu_history_regs_bin_restore(indice);

    indice_debug_cpu_backwards_history++;    
}

void menu_debug_cpu_backwards_history_run(zxvision_window *ventana)
{
    int total_elementos_in_history=cpu_history_get_total_elements();

    if (total_elementos_in_history==0) {
        menu_warn_message("History is empty");
        return;
    }

    int indice=total_elementos_in_history-indice_debug_cpu_backwards_history-1;

    //Ver si no estamos ya en el ultimo item
    if (total_elementos_in_history-indice_debug_cpu_backwards_history-1<0) {
        menu_warn_message("You are at the oldest item");
        return;
    }    

    //printf("indice_debug_cpu_backwards_history %d\n",indice_debug_cpu_backwards_history);

    do {

        if (debug_breakpoints_enabled.v) {
            //evaluar breakpoints
            cpu_core_loop_debug_check_breakpoints();
            //Volver si se cumple breakpoint
            if (menu_breakpoint_exception.v) return;
        }



        total_elementos_in_history=cpu_history_get_total_elements();

        indice=total_elementos_in_history-indice_debug_cpu_backwards_history-1;

        if (indice>=0) {
            //cada 10000 opcodes, refrescar pantalla
            //esto no es real, habria que contar realmente cuando pasa un frame de pantalla, contando testados
            //pero bueno, lo hago para que quede un efecto mas chulo
            if ((indice % 10000)==0) {

                //leer teclado. Tener en cuenta que no hay nadie leyendo el teclado aqui pues estamos en un bucle cerrado
                //Nota: en driver cocoa de Mac si que se lee el teclado pues se disparan eventos al pulsar teclas
                //en cambio en resto de drivers: xwindows, sdl, etc, no se disparan eventos y se debe llamar especificamente a scr_actualiza_tablas_teclado
                scr_actualiza_tablas_teclado();

                realjoystick_main();

                //volver si pulsada tecla
                if (menu_si_tecla_pulsada() ) {
                    menu_espera_no_tecla();
                    return;
                }


                menu_debug_registers_print_main_step(ventana);
                zxvision_draw_window_contents(ventana);

                //Si real video esta activado, este refresca pantalla no mostraria actualizaciones de pantalla,
                //dado que para eso necesitaria hacer el paso de pantalla a buffer rainbow
                //por tanto, desactivamos temporalmente real video
                //TODO: el efecto inesperado de esto es que yendo hacia atras, si tienes real video, aqui se ira refrescando bien,
                //pero cuando acabe de restaurar todo el historial, te refrescara la pantalla con rainbow y por tanto aparecera
                //el estado inicial, hasta que salgas del menu y se vea bien el estado actual
                //En este caso lo solvento con un mensaje tipo first aid
                int antes_rainbow=rainbow_enabled.v;

                rainbow_enabled.v=0;
                menu_refresca_pantalla();
                rainbow_enabled.v=antes_rainbow;


                //scr_refresca_pantalla();
                //printf("going back index %d\n",indice);

                //pausa de 10 milisegundos para cada "frame"
                //suponiendo que todas las rutinas de arriba: recuperar history, refrescar pantalla etc, no durasen nada,
                //estariamos lanzando unos 10000 opcodes (algo asi como aproximado los opcodes de un frame), a cada frame,
                //con pausa de 10 milisegundos. En cambio cada frame de pantalla son 20 milisegundos, por tanto, asumiendo
                //coste 0 de todo lo anterior, cada frame de pantalla iria 2x veces mas rapido
                //esto es probable que suceda asi (aprox el doble de rapido) en un PC (mucho) mas rapido que el mio
                usleep(10000);
            }

            menu_debug_cpu_backwards_history();
        }
    } while (indice>=0);


}

void menu_debug_cpu_history_select(MENU_ITEM_PARAMETERS)
{
    //menu_debug_memory_pointer=valor_opcion;

    //menu_debug_follow_pc.v=0; //se deja de seguir pc


    char string_destino_registros[1024];
    cpu_history_get_registers_element(valor_opcion,string_destino_registros);


    char string_destino_pc[64];
    //obtiene el historial de PC en esa posicion, en hexadecimal
    cpu_history_get_pc_register_element(valor_opcion,string_destino_pc);

    //Agregamos la H al final para parsear
    int longitud=strlen(string_destino_pc);
    string_destino_pc[longitud++]='H';
    string_destino_pc[longitud]=0;

    menu_z80_moto_int valor_pc=parse_string_to_number(string_destino_pc);


    char string_disassemble[64];


    size_t longitud_op;
    debugger_disassemble(string_disassemble,32,&longitud_op,valor_pc);

    char string_mensaje[2048];
    sprintf(string_mensaje,"%s\n\n%s",string_destino_registros,string_disassemble);

    menu_generic_message("Registers",string_mensaje);
}

void menu_debug_cpu_history(void)
{
    if (cpu_history_enabled.v==0 || cpu_history_started.v==0) {
        if (menu_confirm_yesno("Enable & start cpu history?")) {

            if (cpu_history_enabled.v==0) set_cpu_core_history();
    
            if (cpu_history_started.v==0) cpu_history_started.v=1;

            menu_generic_message_splash("Cpu history","Come back later after running some cpu opcodes");
            
        }

        return;
    }

    int total_items_menus=cpu_history_get_total_elements();

    //definir un maximo de 1000 a mostrar por pantalla
    //por ZRCP hay permitidos muchos mas
    if (total_items_menus>1000) total_items_menus=1000;

    int menu_debug_cpu_history_opcion_seleccionada=0;    

    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    do {



        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        int total_elementos_in_history=cpu_history_get_total_elements();

        int i;
        for (i=0;i<total_items_menus;i++) {
		
			//Al solicitarlo, el 0 es el item mas reciente. el 1 es el anterior a este
			int indice=total_elementos_in_history-i-1;

            char string_pc[32]; 
            //obtiene el historial de PC en esa posicion, en hexadecimal
            cpu_history_get_pc_register_element(indice,string_pc);

            //Agregamos la H al final solo para parsear
            int longitud=strlen(string_pc);
            string_pc[longitud++]='H';
            string_pc[longitud]=0;

            menu_z80_moto_int valor=parse_string_to_number(string_pc);

            char string_dir[32];
            menu_debug_print_address_memory_zone(string_dir,valor);

            //Obtener el opcode de esa direccion y ponerla en el item de menu

            char string_disassemble[64];
            size_t longitud_op;
            debugger_disassemble(string_disassemble,32,&longitud_op,valor);            

            //quitamos la H de la string para no mostrarla, queda redundante
            string_pc[longitud-1]=0;

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_debug_cpu_history_select,NULL,"%s %s",
                string_pc,string_disassemble);

            menu_add_item_menu_ayuda(array_menu_common,"The element at the top is the most recent opcode ran");
            
            
            //en item de menu metemos el indice a historial
            menu_add_item_menu_valor_opcion(array_menu_common,indice);
        }

        if (total_items_menus==0) {
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"(Empty)");
        }


        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&menu_debug_cpu_history_opcion_seleccionada,&item_seleccionado,array_menu_common,"CPU History");



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);   
}

int menu_debug_registers_print_main_step(zxvision_window *ventana)
{

    int linea;

    menu_debug_registers_set_title(ventana);
    zxvision_draw_window(ventana);

    menu_breakpoint_exception_pending_show.v=0;

    menu_debug_registers_adjust_ptr_on_follow();

    linea=0;
    linea=menu_debug_registers_show_ptr_text(ventana,linea);

    linea++;


    //Zona central de la vista: desensamblado, registros, etc
    linea=menu_debug_registers_print_registers(ventana,linea);

    //linea=19;
    linea=menu_debug_registers_get_line_legend(ventana);


    return linea; 
}

void menu_debug_help(void)
{
    if (gui_language==GUI_LANGUAGE_SPANISH) {
        menu_generic_message("Ayuda",
        "En esta ventana de Debug CPU se pueden tener diferentes vistas (seleccionables con las teclas 1-8), cada una mostrando diferente información:\n"
        "1 - Vista por defecto. Se muestra una zona superior con desensamblado de instrucciones y registros. Esta zona se puede cambiar mediante tecla m, "
        "la cual conmuta en vista de desensamblado, vista hexadecimal, vista ascii, y vista de desensamblado sin registros. En esta vista 1, "
        "se puede mover el cursor mediante teclado o ratón; también, al pulsar con ratón en una linea de desensamblado, se asigna un breakpoint. \n"
        "En esta vista también se visualizan, en color rojo, los registros que modificará la instrucción en que está posicionado el cursor.\n"
        "\n"
        "2 - Vista de una línea de desensamblado, registros y diferentes puertos hardware\n"
        "\n"
        "3 - Vista de 9 líneas de desensamblado y diferentes puertos hardware\n"
        "\n"
        "4 - Vista de 19 líneas de desensamblado\n"
        "\n"
        "5 - Vista de 9 líneas de hexadecimal y ascii y diferentes puertos hardware\n"
        "\n"
        "6 - Vista de 19 líneas de hexadecimal y ascii\n"
        "\n"
        "7 - Vista de una línea de desensamblado y el contador de t-estados y scanline actual\n"   
        "\n"
        "8 - Vista de depuración de aventura conversacional, sólo disponible si se carga una aventura hecha con Quill, Paws, o Daad\n"
        "\n"
        "En la mayoría de las vistas, hay diferentes teclas que realizan acciones. Hay que tener en cuenta que se distingue mayúsculas "
        "de minúsculas, por tanto, las teclas en mayúsculas hay que accionarlas junto con la tecla Caps shift:\n"
        "\n"
        "t: cambiar el puntero donde estamos visualizando el listado\n"
        "\n"
        "f: habilita/deshabilita el seguimiento del puntero con la posición del registro PC de la cpu\n"
        "\n"
        "s: modo paso a paso: Habilita/deshabilita el modo paso a paso. En el modo paso a paso se puede entrar manualmente con dicha tecla "
        "o también se entra de manera automática siempre que se tenga deshabilitado el Multitask menu o se tenga habilitada opción de Stop emulation on menu.\n"
        "Cuando no se está en modo paso a paso, la emulación de la máquina sigue ejecutándose. En cambio, en modo paso a paso, "
        "la emulación está detenida, y se ejecuta una instrucción a cada pulsación de la tecla Enter\n"
        "\n"
        "d: desensamblar: Se tiene una ventana adicional de desensamblado y exportación del listado a archivo de texto\n"
        "\n"
        "a: ensamblar: Se puede ensamblar codigo máquina, linea a linea\n"
        "\n"
        "Enter: ejecutar siguiente instrucción cuando se está en modo paso a paso\n"
        "\n"
        "o: Ejecutar hasta volver de la siguiente instrucción, útil por ejemplo para volver justo después de un CALL\n"
        "\n"
        "m: Cambiar entre los diferentes modos de la vista 1\n"
        "\n"
        "r: Modificar registros\n"
        "\n"
        "b: Ir a la ventana de breakpoints\n"
        "\n"
        "w: Ir a la ventana de watches\n"
        "\n"
        "l: Poner/Quitar breakpoint en la posición indicada por el puntero\n"
        "\n"
        "n: Salir de la vista de Debug CPU y seguir ejecutando\n"
        "\n"
        "u: Ejecutar hasta que se llegue a la posición indicada por el puntero\n"
        "\n"
        "e: Ejecutar la instrucción RET, que provocará alterar registros PC y SP\n"
        "\n"
        "p: Resetear contador de t-estados parcial. Dicho contador se encuentra visible en las vistas 2, 3 y 5\n"
        "\n"
        "i: Escribir un valor en una dirección de memoria\n"
        "\n"
        "z: Seleccionar zona de memoria. Las diferentes zonas de memoria permiten acceder a bloques de memoria mas allá del direccionamiento "
        "habitual de la cpu. Por ejemplo, podemos acceder a los 128kb de memoria RAM de un Spectrum 128k, o a la memoria eprom de un divide\n"
        "\n"
        "P: Modificar el registro PC con el valor del puntero\n"
        "\n"
        "B: Buscar el siguiente breakpoint asignado de tipo PC=XXXX. Requiere activar breakpoints\n"
        "\n"
        "k: Ver valores del stack y modificar puntero de memoria al pulsar enter\n"
        "\n"
        "H: Habilitar y visualizar el historial de ejecución de la cpu\n"
        "\n"
        "S: Permite ejecutar instrucciones hacia atrás. Requiere tener el historial de la cpu habilitado y sólo está disponible en modo paso a paso\n"
        "\n"
        "N: Permite ejecutar continuamente hacia atrás. Requiere tener el historial de la cpu habilitado y sólo está disponible en modo paso a paso. "
        "Lógicamente esta ejecución hacia atrás tiene un límite, determinado en el menu Settings-> Debug-> Max history items\n"
        "\n"
        "g: Ver los gráficos de una aventura hecha con GAC\n"
        "\n"
        "Teclas sólo para la vista 8, de debug de aventura conversacional:\n"
        "\n"
        "Enter: Ejecutar hasta el siguiente condacto\n"
        "\n"
        "k: Establecer un punto de paro definido por la ejecución de un condacto especial en Daad\n"
        "\n"
        "p: Seguir la ejecución hasta el siguiente comando de Parse de Daad\n"
        "\n"
        "w: Modificar los watches, que permiten visualizar un listado de 7 objetos/flags\n"
        "\n"
        "i: Modificar el valor de un flag u objeto\n"
        "\n"
        "e: Listar los diferente mensajes de la aventura: Objetos, Mensajes de usuario, Localizaciones, etc\n"
        "\n"
        "v: Listar el vocabulario de la aventura\n"
        "\n"
        "g: Ver los gráficos de la aventura\n"
        );
    }

    else if (gui_language==GUI_LANGUAGE_CATALAN) {
        menu_generic_message("Ajuda",
        "En aquesta finestra de Debug CPU es poden tenir diferents vistes (seleccionables amb les tecles 1-8), cadascuna mostrant diferent informació:\n"
        "1 - Vista per defecte. Es mostra una zona superior amb desensamblat d'instruccions i registres. Aquesta zona es pot canviar mitjançant tecla m, "
        "la qual commuta en vista de desassemblat, vista hexadecimal, vista ascii, i vista de desassemblat sense registres. En aquesta vista 1, "
        "es pot moure el cursor mitjançant teclat o ratolí; també, en prémer amb ratolí en una línia de desassemblat, s'assigna un breakpoint. \n"
        "En aquesta vista també es visualitzen, en color vermell, els registres que modificarà la instrucció en que està posicionat el cursor.\n"
        "\n"
        "2 - Vista d'una línia de desassemblatge, registres i diferents ports de maquinari\n"
        "\n"
        "3 - Vista de 9 línies de desassemblatge i diferents ports de maquinari\n"
        "\n"
        "4 - Vista de 19 línies de desassemblatge\n"
        "\n"
        "5 - Vista de 9 línies de hexadecimal i ascii i diferents ports de maquinari\n"
        "\n"
        "6 - Vista de 19 línies de hexadecimal i ascii\n"
        "\n"
        "7 - Vista d'una línia de desassemblatge i el comptador de t-estats i scanline actual\n"
        "\n"
        "8 - Vista de depuració d'aventura conversacional, només disponible si es carrega una aventura feta amb Quill, Paws, o Daad\n"
        "\n"
        "A la majoria de les vistes, hi ha diferents tecles que realitzen accions. Cal tenir en compte que es distingeix majúscules"
        "de minúscules, per tant, les tecles en majúscules cal accionar-les juntament amb la tecla Caps shift:\n"
        "\n"
        "t: canviar el punter on estem visualitzant el llistat\n"
        "\n"
        "f: habilita/deshabilita el seguiment del punter amb la posició del registre PC de la cpu\n"
        "\n"
        "s: mode pas a pas: Habilita/deshabilita el mode pas a pas. En el mode pas a pas es pot entrar manualment amb aquesta tecla "
        "o també s'entra de manera automàtica sempre que es tingui deshabilitat el Multitask menu o es tingui habilitada opció de Stop emulation on menu.\n"
        "Quan no s'està en mode pas a pas, l'emulació de la màquina continua executant-se. En canvi, en mode pas a pas,"
        "l'emulació està aturada, i s'executa una instrucció a cada clic de la tecla Enter\n"
        "\n"
        "d: desassemblar: Es té una finestra addicional de desassemblat i exportació del llistat a fitxer de text\n"
        "\n"
        "a: assemblar: Es pot assemblar codi màquina, línia a línia\n"
        "\n"
        "Enter: executar següent instrucció quan s'està en mode pas a pas\n"
        "\n"
        "o: Executar fins a tornar de la següent instrucció, útil per exemple per tornar just després d'un CALL\n"
        "\n"
        "m: Canviar entre els diferents modes de la vista 1\n"
        "\n"
        "r: Modificar registres\n"
        "\n"
        "b: Anar a la finestra de breakpoints\n"
        "\n"
        "w: Anar a la finestra de watches\n"
        "\n"
        "l: Posar/treure breakpoint a la posició indicada pel punter\n"
        "\n"
        "n: Sortir de la vista de Debug CPU i continuar executant\n"
        "\n"
        "u: Executar fins que s'arribi a la posició indicada pel punter\n"
        "\n"
        "e: Executar la instrucció RET, que provocarà alterar registres PC i SP\n"
        "\n"
        "p: Resetejar comptador de t-estats parcial. Aquest comptador es troba visible a les vistes 2, 3 i 5\n"
        "\n"
        "i: Escriure un valor en una adreça de memòria\n"
        "\n"
        "z: Seleccionar zona de memòria. Les diferents zones de memòria permeten accedir a blocs de memòria més enllà del direccionament "
        "habitual de la cpu. Per exemple, podem accedir als 128kb de memòria RAM d'un Spectrum 128k, o a la memòria eprom d'un divide\n"
        "\n"
        "P: Modificar el registre PC amb el valor del punter\n"
        "\n"
        "B: Cerca el següent breakpoint assignat de tipus PC=XXXX. Requereix activar breakpoints\n"
        "\n"
        "k: Veure valors del stack i modificar punter de memoria al polsar enter\n"
        "\n"
        "H: Habilita i visualitza l'historial d'execució de la cpu\n"
        "\n"
        "S: Permet executar instruccions cap enrere. Requereix tenir l'historial de la cpu habilitat i només està disponible en mode pas a pas\n"
        "\n"
        "N: Permet executar contínuament cap enrere. Requereix tenir l'historial de la cpu habilitat i només està disponible en mode pas a pas."
        "Lògicament aquesta execució cap enrere té un límit, determinat al menú Settings-> Debug-> Max history items\n"
        "\n"
        "g: Vegeu els gràfics d'una aventura feta amb GAC\n"
        "\n"
        "Tecles només per a la vista 8, de debug d'aventura conversacional:\n"
        "\n"
        "Enter: Executar fins al següent condacte\n"
        "\n"
        "k: Establir un punt d'atur definit per l'execució d'un condacte especial a Daad\n"
        "\n"
        "p: Seguir l'execució fins el següent comandament de Parse de Daad\n"
        "\n"
        "w: Modificar els watches, que permeten visualitzar un llistat de 7 objectes/flags\n"
        "\n"
        "i: Modificar el valor d'un flag o objecte\n"
        "\n"
        "e: Llista els diferents missatges de l'aventura: Objectes, Missatges d'usuari, Localitzacions, etc\n"
        "\n"
        "v: Llistar el vocabulari de l'aventura\n"
        "\n"
        "g: Veure els gràfics de l'aventura\n"
        );
    }


    else {
        menu_generic_message("Help",
        "This Debug CPU window can show different views (chosen with keys 1-8), each of them showing different information:\n"
        "1 - Default view. You see a top section with opcodes disassembly and registers. This section can be changed pressing key m, "
        "switching from: disassembly, hexadecimal, ascii, and disassembly without registers. On this view 1, "
        "you can move cursor by using keys or mouse; also, pressing mouse on a disassembly line, you set a breakpoint. \n"
        "In this view you can also see, in red color, which registers will be modified by the opcode the cursor is in.\n"
        "\n"
        "2 - View with one disassembly line, registers and different hardware ports\n"
        "\n"
        "3 - View with 9 disassembly lines and different hardware ports\n"
        "\n"
        "4 - View with 19 disassembly lines\n"
        "\n"
        "5 - View with 9 hexadecimal and ascii lines and different hardware ports\n"
        "\n"
        "6 - View with 19 hexadecimal hexadecimal and ascii lines\n"
        "\n"
        "7 - View with one disassembly line, t-estates counter and current scanline\n"
        "\n"
        "8 - View for debugging Text Adventures, only available if the current loaded game is one made with Quill, Paws, or Daad\n"
        "\n"
        "In most of the views, there are some keys which run actions. Capitalisation in keys is taken care "
        "so, capital keys must be fired pressing Caps shift too:\n"
        "\n"
        "t: Change the listing pointer\n"
        "\n"
        "f: enable/disable following pointer with the PC cpu register\n"
        "\n"
        "s: step mode: enable/disable step mode. You can enter step mode manually by pressing that key "
        "or also entering this window having Multitasking disabled or enabling setting Stop emulation on menu.\n"
        "When not in step mode, machine emulated continues running. Besides, on step mode, "
        "emulation is stopped, and an opcode is run on every pressing of key Enter\n"
        "\n"
        "d: disassemble: An additional window for disassembly and exporting listings to text files\n"
        "\n"
        "a: assemble: You can assemble machine code, line by line\n"
        "\n"
        "Enter: run next opcode when on step mode\n"
        "\n"
        "o: Run until returning from the next opcode, useful, for example, to return after a CALL opcode\n"
        "\n"
        "m: Switch between different modes of view 1\n"
        "\n"
        "r: Modify registers\n"
        "\n"
        "b: Go to breakpoints window\n"
        "\n"
        "w: Go to watches window\n"
        "\n"
        "l: Add/Remove breakpoint on the current pointer position\n"
        "\n"
        "n: Exit from Debug CPU window and continue running\n"
        "\n"
        "u: Run until the cpu reaches the current pointer position\n"
        "\n"
        "e: Run a RET opcode, which will alter PC and SP registers\n"
        "\n"
        "p: Reset t-states partial counter. That counter can be seen on views 2, 3 and 5\n"
        "\n"
        "i: Write a value on a memory address\n"
        "\n"
        "z: Select memory zone. You can select different memory zones beyond the normal cpu addressing. "
        "For example, you can access to the 128kb RAM memory of a Spectrum 128k, or to the divide eprom memory\n"
        "\n"
        "P: Set PC register with the pointer value\n"
        "\n"
        "B: Look for the next breakpoint set of type PC=XXXX. Requires enabling breakpoints\n"
        "\n"
        "k: View stack values and set memory pointer when pressing enter\n"
        "\n"
        "H: Enable and view the cpu execution history\n"
        "\n"
        "S: You can step back opcodes. You need cpu execution history enabled and it's only available on step mode\n"
        "\n"
        "N: You can run backwards continuously. You need cpu execution history enabled and it's only available on step mode. "
        "This backwards execution has a limit defined on menu Settings-> Debug-> Max history items\n"
        "\n"
        "g: See the graphics of a GAC text adventure\n"
        "\n"
        "Keys exclusive for view 8, to debug text adventures:\n"
        "\n"
        "Enter: Run until next condact\n"
        "\n"
        "k: Set a breakpoint fired with a special Daad condact\n"
        "\n"
        "p: Continue execution until after executing the next Parse command on Daad\n"
        "\n"
        "w: Modify watches, that lists 7 objects/flags\n"
        "\n"
        "i: Modify a flag or an object\n"
        "\n"
        "e: List all the messages of an adventure: Objects, User messages, Localizations, etc\n"
        "\n"
        "v: List vocabulary of the adventure\n"
        "\n"
        "g: See the graphics of the adventure\n"
        );
    }
}

void menu_debug_registers_view_adventure(MENU_ITEM_PARAMETERS)
{

    //Iniciar con vista 8
    menu_debug_registers_current_view=8;

    menu_debug_registers(valor_opcion);
}

void menu_debug_cpu_view_stack(void)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

    int menu_debug_cpu_view_stack_opcion_seleccionada=0;


    do {

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        int i;

        for (i=0;i<30;i++) {
            menu_z80_moto_int stack_value=debug_get_stack_z80_value(i);

            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,"%04X",stack_value);
            menu_add_item_menu_valor_opcion(array_menu_common,stack_value);
        }


        menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&menu_debug_cpu_view_stack_opcion_seleccionada,&item_seleccionado,array_menu_common,"View Stack");



        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //Cambiar puntero con el valor seleccionado
            menu_debug_memory_pointer=item_seleccionado.valor_opcion;
            //printf("Set pointer to %XH\n",menu_debug_memory_pointer);
            menu_debug_follow_pc.v=0; //se deja de seguir pc

            //y salimos
            return;
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


}

void menu_debug_registers(MENU_ITEM_PARAMETERS)
{

    //printf("inicio debug registers. debug_daad_breakpoint_runtoparse_fired.v=%d\n",debug_daad_breakpoint_runtoparse_fired.v);



	z80_byte acumulado;

	//ninguna tecla pulsada inicialmente
	acumulado=MENU_PUERTO_TECLADO_NINGUNA;

	int linea=0;

	z80_byte tecla;

	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;


	//menu_debug_registers_current_view
	//Si estabamos antes en vista 8, pero ya no hay un programa daad en memoria, resetear a vista 1
	if (menu_debug_registers_current_view==8 && !util_daad_detect() && !util_textadv_detect_paws_quill() ) {
		menu_debug_registers_current_view=1;
	}



	//Inicializar info de tamanyo zona
	menu_debug_set_memory_zone_attr();

    //Resetear posicion de backwards siempre al entrar de nuevo en esta ventana
    indice_debug_cpu_backwards_history=0;        


	//Ver si hemos entrado desde un breakpoint
	/*if (menu_breakpoint_exception.v) menu_debug_registers_gestiona_breakpoint();

	else menu_espera_no_tecla();*/

    //printf("despues de menu_espera_no_tecla\n");

	char buffer_mensaje[64];

	//Si no esta multitarea activa o pause emulation en menu, modo por defecto es step to step
	if (menu_multitarea==0 || menu_emulation_paused_on_menu || menu_emulation_paused_on_menu_by_debug_step_mode) cpu_step_mode.v=1;


	//zxvision_window ventana;
	zxvision_window *ventana;
	ventana=&zxvision_window_menu_debug_registers;

    //IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
    //si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
    //la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
    zxvision_delete_window_if_exists(ventana);


	menu_debug_registers_zxvision_ventana(ventana);

    //guardar tamanyo inicial para cuando se recrea la ventana indicarlo como tamanyo de antes minimizado
    //int ancho_ventana_inicial=ventana->visible_width;
    //int alto_ventana_inicial=ventana->visible_height;    


	//Guardar ancho y alto anterior para recrear la ventana si cambia
    //Ya NO hace falta esto, pues zxvision ya recrea la ventana al ampliarla
    /*
	int ventana_ancho_antes;
	int ventana_alto_antes;

	menu_debug_registers_zxvision_save_size(ventana,&ventana_ancho_antes,&ventana_alto_antes);
    */

	menu_debug_registers_set_title(ventana);

    //Decir que la primera vez siempre muestra ventana
    int forzar_refresco_ventana=1;


        //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
        //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
        if (zxvision_currently_restoring_windows_on_start) {
                //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");

				//printf ("Overlay al finalizar desde inicio: %p\n",ventana->overlay_function);

                return;
        }	


        if (menu_breakpoint_exception.v) {
            menu_debug_registers_gestiona_breakpoint();
            //Ver tipo de accion para ese breakpoint
            //printf ("indice breakpoint & accion : %d\n",catch_breakpoint_index);
            osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

            debug_printf(VERBOSE_DEBUG,"There is a menu breakpoint exception");

            //Si accion nula o menu o break
            if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {

                debug_printf(VERBOSE_DEBUG,"Show breakpoint");


                int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
                menu_emulation_paused_on_menu=1;

                audio_playing.v=0;
                //printf ("pc: %d\n",reg_pc);

          
                menu_breakpoint_fired(catch_breakpoint_message);

                //restaurar estado multitarea

                menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;                


            }

            else {
                //Gestion acciones. Se gestionan desde el mismo core de debug y aqui no deberian escalarse nunca
                //debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
            }

        }


	//Si se habia lanzado un runtoparse de daad
    
	if (debug_daad_breakpoint_runtoparse_fired.v) {
		debug_printf (VERBOSE_DEBUG,"Going back from a daad breakpoint runtoparse. Adding a step to step condact breakpoint and exiting window");

        //printf("Going back from a daad breakpoint runtoparse. Adding a step to step condact breakpoint and exiting window\n");
		//Lo quitamos y metemos un breakpoint del step to step
		debug_daad_breakpoint_runtoparse_fired.v=0;
		debug_stepping_daad_runto_parse.v=0;
		menu_debug_delete_daad_parse_breakpoint();


        //La idea es que el runtoparse hace el primer breakpoint, hasta aqui, que le situa en un condacto de tipo parse,
        //pero se pone otro breakpoint automatico que retornara el usuario despues del comando parse
        //O sea, este comando "runtoparse" se convierte mas bien en "runtoafterparse"
        
		menu_debug_daad_step_breakpoint();
		salir_todos_menus=1;
              
        //Ademas quitamos el flag de abrir menu que se habia quedado activado
        //Esto realmente solo hace falta cuando se ejecuta step si el menu debug cpu se ha abierto como consecuencia de un breakpoint
        //esto fijarse en funcion menu_inicio cuando se usa zxvision_switch_to_window_on_open_menu, que se abre ventana
        //tanto en caso que haya multitarea como no
        //Esta sentencia y comentarios estan repetidos en varios sitios
        //TODO: Creo que en vez de cambiar este menu_event_open_menu.v=0, habria que llamar a menu_inicio_pre_retorno_reset_flags
        //cuando se sale de la apertura de ventana en el caso de zxvision_switch_to_window_on_open_menu
        menu_event_open_menu.v=0;

        //Este siguiente es necesario cuando se tiene ventanas en multitarea activado
        //Nota: esto por ejemplo no es necesario cuando se hace un "stepcondact" ya que la salida de la ventana es diferente de la que se hace aqui
        //Nota para mi yo del futuro: Y por que es diferente? Toda la explicacion tecnica ahora no se (y no tengo ganas de mirarlo),
        //pero esta claro que es diferente salir con la tecla de stepcondact, a la salida que se ha hecho aqui, que se ha entrado
        //desde un primer breakpoint de "ejecutar hasta parse" y luego metemos otro de "stepcondact"
        menu_pressed_close_all_menus.v=1;


		return;
        
	}
    



	debug_stepping_daad.v=0;
	debug_stepping_daad_runto_parse.v=0;
    //printf("inicio menu_debug_registers. debug_stepping_daad_runto_parse.v=0;\n");



	do {

	


		//Si es la vista 8, siempre esta en cpu step mode, y zona de memoria es la mapped
		if (menu_debug_registers_current_view==8) {
			cpu_step_mode.v=1;
			menu_debug_set_memory_zone_mapped();
		}

		//
		//Si no esta el modo step de la cpu
		//
		if (cpu_step_mode.v==0) {

            //printf("antes de ver contador\n");

			//Cuadrarlo cada 1/16 de segundo, justo lo mismo que el flash, asi
			//el valor de flash se ve coordinado
        	        //if ( (contador_segundo%(16*20)) == 0 || menu_multitarea==0) {
			if ( ((contador_segundo%(16*20)) == 0 && valor_contador_segundo_anterior!=contador_segundo ) || menu_multitarea==0 || forzar_refresco_ventana) {
				//printf ("Refresco pantalla. contador_segundo=%d\n",contador_segundo);
                forzar_refresco_ventana=0;
				valor_contador_segundo_anterior=contador_segundo;


				menu_debug_registers_set_title(ventana);
				zxvision_draw_window(ventana);
                //printf("despues de draw window\n");

				menu_debug_registers_adjust_ptr_on_follow();

                linea=0;
                linea=menu_debug_registers_show_ptr_text(ventana,linea);

                linea++;


                //Forzar a mostrar atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
                menu_writing_inverse_color.v=1;

                        
				linea=menu_debug_registers_print_registers(ventana,linea);
				//linea=19;


				//En que linea aparece la leyenda
				linea=menu_debug_registers_get_line_legend(ventana);
				linea=menu_debug_registers_print_legend(ventana,linea);


				//Restaurar estado mostrar atajos
				menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


				zxvision_draw_window_contents(ventana);

                if (menu_multitarea==0) menu_refresca_pantalla();


	        }

        

        	menu_cpu_core_loop();

			if (menu_breakpoint_exception.v) {
				//Si accion nula o menu o break
				if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
				  menu_debug_registers_gestiona_breakpoint();
				  //Y redibujar ventana para reflejar breakpoint cond
				  //menu_debug_registers_ventana();
				}

				else {
					//menu_breakpoint_exception.v=0;
                    //Gestion acciones. Se gestionan desde el mismo core de debug y aqui no deberian escalarse nunca
					//debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
				}
			}


            //printf("Antes menu_da_todas_teclas. wheel: %d\n",mouse_wheel_vertical);
            acumulado=menu_da_todas_teclas();
            //printf("Despues menu_da_todas_teclas. acumulado=%d\n",acumulado);

            //Si se ha movido mouse, al volver de menu_da_todas_teclas dira que ha habido alguna tecla "pulsada" y se quedara
            //esperando un poco mas abajo al llamar a zxvision_common_getkey_wheel_refresh_noesperanotec
            //Eliminar dicha "pulsacion"
            //Esto es un poco chapuza, no me acaba de gustar. Quiza lo de mouse_movido, cuando se lee a menu_da_todas_teclas,
            //no deberia considerarse, pero bueno alguna razon habrá para lo de mouse_movido ahí
            //TODO: esto puede que pase lo mismo en otras ventanas: donde al llamar a menu_da_todas_teclas, para saber si hay tecla pulsada,
            //y al mover raton, se cree que es tecla pulsada y llama a otra funcion de esperar pulsar tecla
            //Efecto parecido aunque no afecta igual: En Hexadecimal Editor, se queda esperando tecla. Si se mueve raton,
            //no leera tecla lógicamente pero la vista hexadecimal se refresca, se puede apreciar apuntando a alguna direccion
            //que esté variando (stack o pantalla por ejemplo) y se verá como refresca los datos al mover ratón
            if (mouse_movido) {
                //printf("mouse movido. decir no tecla\n");
                acumulado |=1;
            }

	    	//si no hay multitarea, esperar tecla y salir
        	if (menu_multitarea==0) {
            	menu_espera_tecla();
               	acumulado=0;
	        }

            int accion_mouse_pulsado=0;

            //printf("Despues menu_da_todas_teclas en modo no step\n");
            //Si se pulsa raton en vista 1
            //Evitar cuando se arrastra ventana y acaba el cursor dentro al liberar boton
            if ((mouse_left || mouse_wheel_vertical) && menu_debug_registers_current_view==1 && !mouse_is_dragging) {
                //printf("left: %d wheel: %d\n",mouse_left,mouse_wheel_vertical);
                tecla=menu_debug_cpu_handle_mouse(ventana);
                if (tecla!=0) {
                    accion_mouse_pulsado=1;
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA ^255; //cualquier variacion sobre MENU_PUERTO_TECLADO_NINGUNA nos vale
                    //printf("Accion mouse en modo no step\n");
                    //menu_espera_no_tecla();
                }
            }

            //printf("despues de varios eventos tecla\n");

            //Cualquier otra vista, si se pulsa rueda, resetearla
            //si no hicieramos esto, al mover rueda en una vista que no es la 1,
            //se interpretaria continuamente que hay tecla pulsada al llamar un poco mas abajo a zxvision_common_getkey_wheel_refresh_noesperanotec
            if (menu_debug_registers_current_view!=1 && mouse_wheel_vertical) {
                mouse_wheel_vertical=0;
            }

			//Hay tecla pulsada
			if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
				//tecla=zxvision_common_getkey_refresh();
                if (!accion_mouse_pulsado) { 
                    //printf("Antes zxvision_common_getkey_refresh_noesperanotec. wheel: %d acumulado: %d movido: %d\n",
                    //    mouse_wheel_vertical,acumulado,mouse_movido);

				    tecla=zxvision_common_getkey_wheel_refresh_noesperanotec();

                    //printf("Despues zxvision_common_getkey_refresh_noesperanotec\n");
                }

            	//Aqui suele llegar al mover raton-> se produce un evento pero no se pulsa tecla
                if (tecla==0) {
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

                else {
                    //printf ("tecla: %d\n",tecla);
                    //A cada pulsacion de tecla, mostramos la pantalla del ordenador emulado
                    menu_debug_registers_if_cls();
                    //menu_espera_no_tecla_no_cpu_loop();

                    //para forzar refresco rapido de pantalla
                    //importante para que se vea al momento acciones como mover el wheel de raton o pulsar cursores
                    forzar_refresco_ventana=1;
                    //printf("tecla pulsada. forzar refresco\n");                    
                }

                //printf("despues de tecla\n");


                if (tecla=='s') {
					cpu_step_mode.v=1;
					menu_debug_follow_pc.v=1; //se sigue pc
				}

				if (tecla=='z') {
					menu_debug_change_memory_zone();
				}


				if (tecla=='d') {
					menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					menu_debug_disassemble(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}


				if (tecla=='a') {
                    if (menu_debug_registers_current_view==8) {
                        int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
                        //permitimos multitarea
                        menu_emulation_paused_on_menu=0;


				    	//La cerramos pues el envio de watches a background no funciona bien si hay otra ventana detras
					    zxvision_destroy_window(ventana);
                        menu_debug_textadventure_map_connections(0);
                        menu_debug_registers_zxvision_ventana(ventana);		

                        menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
                    }
                    else {
					    menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					    menu_debug_assemble(0);
                    }
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}	
             			

				if (tecla=='b') {
					menu_breakpoints(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				if (tecla=='k' && menu_debug_registers_current_view!=8 && CPU_IS_Z80) {
                    menu_debug_cpu_view_stack();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }                

				if (tecla=='m' && menu_debug_registers_current_view==1) {
                    menu_debug_next_dis_show_hexa();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				if (tecla=='l' && menu_debug_registers_current_view==1) {
                    menu_debug_toggle_breakpoint();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

                //Graficos gac. No necesita vista 8 y se puede hacer tambien sin step mode
				if (tecla=='g'  && util_gac_detect() ) {
                    menu_debug_daad_view_graphics();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;                    
                }

				if (tecla=='u' && menu_debug_registers_current_view==1) {
					menu_debug_runto();
                    tecla=2; //Simular ESC
					salir_todos_menus=1;
                }			

				if (tecla=='n' && menu_debug_registers_current_view==1) {
					//run tal cual. como runto pero sin poner breakpoint
                    tecla=2; //Simular ESC
					salir_todos_menus=1;
                }	

				if (tecla=='n' && menu_debug_registers_current_view==8) {
					menu_debug_daad_connections();
                    tecla=2; //Simular ESC
					salir_todos_menus=1;
                }	                									

				if (tecla=='w') {
					//La cerramos pues el envio de watches a background no funciona bien si hay otra ventana detras
					zxvision_destroy_window(ventana);
                    menu_watches(0);
					menu_debug_registers_zxvision_ventana(ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

								
								
				if (tecla=='i') {
					last_debug_poke_dir=menu_debug_memory_pointer;
					if (menu_debug_registers_current_view==8) {
						menu_debug_daad_edit_flagobject();
					}
                    else menu_debug_poke(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }								

                if (tecla=='p') {
					if (menu_debug_registers_current_view==8) {
						//Esto es run hasta Parse Daad
						menu_debug_daad_runto_parse();
                    	tecla=2; //Simular ESC
						salir_todos_menus=1;	

                    //Ademas quitamos el flag de abrir menu que se habia quedado activado
                    //Esto realmente solo hace falta cuando se ejecuta step si el menu debug cpu se ha abierto como consecuencia de un breakpoint
                    //esto fijarse en funcion menu_inicio cuando se usa zxvision_switch_to_window_on_open_menu, que se abre ventana
                    //tanto en caso que haya multitarea como no
                    //Esta sentencia y comentarios estan repetidos en varios sitios
                    //TODO: Creo que en vez de cambiar este menu_event_open_menu.v=0, habria que llamar a menu_inicio_pre_retorno_reset_flags
                    //cuando se sale de la apertura de ventana en el caso de zxvision_switch_to_window_on_open_menu
                    menu_event_open_menu.v=0;                        					
					}
					else {
						debug_t_estados_parcial=0;
                    	//Decimos que no hay tecla pulsada
                    	acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					}
                }


                //Establecer PC con valor de PTR
		        if (tecla=='P') {
                    char buffer_temp[32];
                    sprintf(buffer_temp,"PC=%d",menu_debug_memory_pointer);
                    //printf("%s\n",buffer_temp);
                    debug_change_register(buffer_temp);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

                //Siguiente breakpoint tipo pc=dir
		        if (tecla=='B' && debug_breakpoints_enabled.v) {
                    debug_cpu_next_breakpoint_pc_dir();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }       


                if (tecla=='H' && CPU_IS_Z80) {
					//Detener multitarea pues interesa que no se "mueva" la cpu al sacar el historial
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_cpu_history();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;


                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
					
                }

                //ayuda
                if (tecla==MENU_TECLA_AYUDA) {

                    menu_debug_help();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					
                }                
                	                

				//Vista. Entre 1 y 8
				if (tecla>='1' && tecla<='8') {
					menu_debug_registers_set_view(ventana,tecla-'0');
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				if (tecla=='f') {
					menu_debug_switch_follow_pc();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				if (tecla=='t') {
					menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_registers_change_ptr();
					//Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

               	if (tecla=='r') {
					menu_debug_change_registers();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
            	}

				if (tecla==11) {
                    //arriba
					menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_up();
					//Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				if (tecla==10) {
                    //abajo
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_down(ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				//24 pgup
                if (tecla==24) {
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_pgup(ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				//25 pgwn
				if (tecla==25) {
					//PgDn
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_pgdn(ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				//Si tecla no es ESC o background, no salir
				if (tecla!=2 && tecla!=3) acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                if (tecla==3) {
                    //No mantener emulacion pausada ya que no estamos en step mode
                    //printf("Desactivar menu_emulation_paused_on_menu_by_debug_step_mode\n");
                    menu_emulation_paused_on_menu_by_debug_step_mode=0;
                }

			}

		}


		//
		//En modo Step mode
		//
		else {

            linea=menu_debug_registers_print_main_step(ventana);

            
            /*
			menu_debug_registers_set_title(ventana);
			zxvision_draw_window(ventana);

			menu_breakpoint_exception_pending_show.v=0;

			menu_debug_registers_adjust_ptr_on_follow();
	
   	        linea=0;
	        linea=menu_debug_registers_show_ptr_text(ventana,linea);

        	linea++;
		

            //Zona central de la vista: desensamblado, registros, etc
            linea=menu_debug_registers_print_registers(ventana,linea);

			//linea=19;
			linea=menu_debug_registers_get_line_legend(ventana);

        	//Forzar a mostrar atajos
	        z80_bit antes_menu_writing_inverse_color;
	        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        	menu_writing_inverse_color.v=1;

            */

            int si_ejecuta_una_instruccion=1;
            //Forzar a mostrar atajos
            z80_bit antes_menu_writing_inverse_color;
	        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
            menu_writing_inverse_color.v=1;
            


			if (continuous_step==0) {
								//      01234567890123456789012345678901
				linea=menu_debug_registers_print_legend(ventana,linea);
																	// ~~1-~~5 View
			}
			else {
				//Mostrar progreso

				if (menu_debug_registers_current_view!=7) {
					char buffer_progreso[32];
					menu_debug_cont_speed_progress(buffer_progreso);
					sprintf (buffer_mensaje,"~~C: Speed %d %s",menu_debug_continuous_speed,buffer_progreso);
					zxvision_print_string_defaults_fillspc(ventana,1,linea++,buffer_mensaje);

					zxvision_print_string_defaults_fillspc(ventana,1,linea++,"Any other key: Stop cont step");
													  //0123456789012345678901234567890

					//si lento, avisar
					if (menu_debug_continuous_speed<=1) {
						zxvision_print_string_defaults_fillspc(ventana,1,linea++,"Note: Make long key presses");
					}
					else {
						zxvision_print_string_defaults_fillspc(ventana,1,linea++,"                         ");
					}

                    //borrar la linea de abajo de leyenda
                    zxvision_print_string_defaults_fillspc(ventana,1,linea++,"");

				}


				//Pausa
				//0= pausa de 0.5
				//1= pausa de 0.1
				//2= pausa de 0.02
				//3= sin pausa

				if (menu_debug_continuous_speed==0) usleep(500000); //0.5 segundo
				else if (menu_debug_continuous_speed==1) usleep(100000); //0.1 segundo
				else if (menu_debug_continuous_speed==2) usleep(20000); //0.02 segundo
			}


			//Restaurar estado mostrar atajos
			menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

			//Actualizamos pantalla
			//zxvision_draw_window(&ventana);
			zxvision_draw_window_contents(ventana);
			menu_refresca_pantalla();


			//Esperamos tecla
			if (continuous_step==0)
			{ 


				//menu_espera_tecla_no_cpu_loop();
					
				//No quiero que se llame a core loop si multitarea esta activo pero aqui estamos en cpu step
				int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
				menu_emulation_paused_on_menu=1;

                //menu_espera_tecla();
                menu_espera_tecla_o_wheel();

                //printf("Despues espera tecla en modo step\n");
                int accion_mouse_pulsado=0;

                //Si se pulsa raton en vista 1
                //Evitar cuando se arrastra ventana y acaba el cursor dentro al liberar boton
                if ((mouse_left || mouse_wheel_vertical) && menu_debug_registers_current_view==1 && !mouse_is_dragging) {
                    tecla=menu_debug_cpu_handle_mouse(ventana);

                    if (tecla!=0) {
                        accion_mouse_pulsado=1;
                        //printf("Accion mouse en modo step\n");
                    }                    
                }

                //Cualquier otra vista, si se pulsa rueda, resetearla
                //si no hicieramos esto, al mover rueda en una vista que no es la 1,
                //se interpretaria continuamente que hay tecla pulsada al llamar un poco mas abajo a zxvision_common_getkey_wheel_refresh_noesperanotec
                if (menu_debug_registers_current_view!=1 && mouse_wheel_vertical) {
                    mouse_wheel_vertical=0;
                }


				//tecla=zxvision_common_getkey_refresh();
				if (!accion_mouse_pulsado) {
                    //printf("Antes zxvision_common_getkey_refresh_noesperanotec\n");
                    tecla=zxvision_common_getkey_wheel_refresh_noesperanotec();
                    //printf("Despues zxvision_common_getkey_refresh_noesperanotec\n");
                    //printf("tecla pulsada en modo step: %d\n",tecla);
                }
				menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;

				//Aqui suele llegar al mover raton-> se produce un evento pero no se pulsa tecla
				if (tecla==0) {
					acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;
				}

				else {
					//printf ("tecla: %d\n",tecla);

					//A cada pulsacion de tecla, mostramos la pantalla del ordenador emulado
					menu_debug_registers_if_cls();
					//menu_espera_no_tecla_no_cpu_loop();
				}


				if (tecla=='c') {
					continuous_step=1;
				}

                if (tecla=='o') {
                    menu_debug_cpu_step_over();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }

				if (tecla=='d') {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;
					menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					menu_debug_disassemble(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

					//Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
				}


				if (tecla=='a' && menu_debug_registers_current_view!=8) {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;
					menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					menu_debug_assemble(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

					//Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
				}


				if (tecla=='a' && menu_debug_registers_current_view==8) {
					//no Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=0;


                    //La cerramos pues el envio de watches a background no funciona bien si hay otra ventana detras
                    zxvision_destroy_window(ventana);
                    menu_debug_textadventure_map_connections(0);
                    menu_debug_registers_zxvision_ventana(ventana);		

                 

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;

                }                



				if (tecla=='z') {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_change_memory_zone();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;

					//Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;

				}								

                if (tecla=='b') {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_breakpoints(0);

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;


                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
					
                }

                if (tecla=='w') {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

					//La cerramos pues el envio de watches a background no funciona bien si hay otra ventana detras
					zxvision_destroy_window(ventana);
                    menu_watches(0);
					menu_debug_registers_zxvision_ventana(ventana);					

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
                }


                if (tecla=='i') {
                	//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;
				
					last_debug_poke_dir=menu_debug_memory_pointer;
					if (menu_debug_registers_current_view==8) {
						menu_debug_daad_edit_flagobject();
					}
                    else menu_debug_poke(0);					

                	//Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
                }


				if (tecla=='m' && menu_debug_registers_current_view==1) {
		            menu_debug_next_dis_show_hexa();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }


				//Mensaje al que apunta instruccion de condact
				if (tecla=='m' && menu_debug_registers_current_view==8 && util_daad_condact_uses_message() ) {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_daad_get_condact_message();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;

                }

				//Lista de todos mensajes
				if (tecla=='e' && menu_debug_registers_current_view==8) {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_daad_view_messages_ask();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;

                }			

				//Graficos paws/quill/daad y gac
				if (tecla=='g'  && (
                                    (menu_debug_registers_current_view==8 && util_daad_has_graphics() ) 
                                    ||
                                    (util_gac_detect() )
                                    )
                ) {


					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_daad_view_graphics();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;

                }	                	

		        if (tecla=='l' && menu_debug_registers_current_view==1) {
                    menu_debug_toggle_breakpoint();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }

				//Ret
		        if (tecla=='e' && menu_debug_registers_current_view==1) {
                    menu_debug_ret();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }			

                //Establecer PC con valor de PTR
		        if (tecla=='P') {
                    char buffer_temp[32];
                    sprintf(buffer_temp,"PC=%d",menu_debug_memory_pointer);
                    //printf("%s\n",buffer_temp);
                    debug_change_register(buffer_temp);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }	                	
								
				if (tecla=='u' && menu_debug_registers_current_view==1) {
                    menu_debug_runto();
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
					salir_todos_menus=1;
					cpu_step_mode.v=0;
					acumulado=0; //teclas pulsadas
					//Con esto saldremos
                }	

				
				if (tecla=='n' && menu_debug_registers_current_view==1) {
					//run tal cual. como runto pero sin poner breakpoint
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
					salir_todos_menus=1;
					cpu_step_mode.v=0;
					acumulado=0; //teclas pulsadas
					//Con esto saldremos
                }				

                if (tecla=='n' && menu_debug_registers_current_view==8) {
                    	menu_debug_daad_connections();
                    	//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    	si_ejecuta_una_instruccion=0;
						salir_todos_menus=1;
						cpu_step_mode.v=0;
						acumulado=0; //teclas pulsadas
						//Con esto saldremos						
                }                	


                if (tecla=='p') {
					if (menu_debug_registers_current_view==8) {
                    	menu_debug_daad_runto_parse();
                    	//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    	si_ejecuta_una_instruccion=0;
						salir_todos_menus=1;
						cpu_step_mode.v=0;
						acumulado=0; //teclas pulsadas
						//Con esto saldremos	

                    //Ademas quitamos el flag de abrir menu que se habia quedado activado
                    //Esto realmente solo hace falta cuando se ejecuta step si el menu debug cpu se ha abierto como consecuencia de un breakpoint
                    //esto fijarse en funcion menu_inicio cuando se usa zxvision_switch_to_window_on_open_menu, que se abre ventana
                    //tanto en caso que haya multitarea como no
                    //Esta sentencia y comentarios estan repetidos en varios sitios
                    //TODO: Creo que en vez de cambiar este menu_event_open_menu.v=0, habria que llamar a menu_inicio_pre_retorno_reset_flags
                    //cuando se sale de la apertura de ventana en el caso de zxvision_switch_to_window_on_open_menu
                    menu_event_open_menu.v=0;                        					
					}
					else {
						debug_t_estados_parcial=0;
                    	//Decimos que no hay tecla pulsada
                    	acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    	//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    	si_ejecuta_una_instruccion=0;
					}
                }



                //Siguiente breakpoint tipo pc=dir
		        if (tecla=='B' && debug_breakpoints_enabled.v) {
                    debug_cpu_next_breakpoint_pc_dir();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }   

                if (tecla=='H' && CPU_IS_Z80) {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_cpu_history();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;


                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
					
                }


                //ayuda
                if (tecla==MENU_TECLA_AYUDA) {
					//Detener multitarea pues interesa que no se "mueva" la cpu al abrir la ventana
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_help();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;                        

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
					
                }                  

                //backstep
                if (tecla=='S' && cpu_history_enabled.v && cpu_history_started.v) {
                    menu_debug_cpu_backwards_history();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;                    
                }

                //backrun
                if (tecla=='N' && cpu_history_enabled.v && cpu_history_started.v) {
					//Detener multitarea pues interesa que no se "mueva" la cpu si sale el aviso de first aid
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_cpu_backwards_history_run(ventana);
                    if (rainbow_enabled.v) {
                        menu_first_aid("back_run_rainbow");
                    }
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;    

                    //Restaurar estado multitarea 
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;                                    
                }                

				//Vista. Entre 1 y 8
				if (tecla>='1' && tecla<='8') {
                	menu_debug_registers_set_view(ventana,tecla-'0');
				    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
				}

				if (tecla=='f') {
					menu_debug_switch_follow_pc();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
				}

		        if (tecla=='t') {
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;
                    menu_debug_registers_change_ptr();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                                        
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
                }


                //Ver stack
                if (tecla=='k' && menu_debug_registers_current_view!=8 && CPU_IS_Z80) {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_cpu_view_stack();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;


                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
					
                }                          

				//Daad breakpoint
		        if (tecla=='k' && menu_debug_registers_current_view==8) {
					if (debug_allow_daad_breakpoint.v) {
						//Quitarlo
						menu_debug_delete_daad_special_breakpoint();
					}
                    else {
						//Ponerlo
						menu_debug_add_daad_special_breakpoint();
					}

					debug_allow_daad_breakpoint.v ^=1;

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }				

                if (tecla=='r') {
                	//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    menu_debug_change_registers();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

					//Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
					//de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;
                }


			    if (tecla==11) {
                	//arriba
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_up();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }

                if (tecla==10) {
                	//abajo
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_down(ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                	si_ejecuta_una_instruccion=0;
                }

                //24 pgup
                if (tecla==24) {
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_pgup(ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }
                
				//25 pgdn
                if (tecla==25) {
                    //PgDn
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_pgdn(ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }


				if (tecla=='v') {
					menu_espera_no_tecla_no_cpu_loop();
				    //para que no se vea oscuro
				    menu_set_menu_abierto(0);
					menu_cls_refresh_emulated_screen();
				    menu_espera_tecla_no_cpu_loop();
					menu_espera_no_tecla_no_cpu_loop();

					//vuelta a oscuro
				    menu_set_menu_abierto(1);

					menu_cls_refresh_emulated_screen();

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;

					//Y redibujar ventana
					zxvision_draw_window(ventana);
				}

				if (tecla=='s') { 
					cpu_step_mode.v=0;
					//Decimos que no hay tecla pulsada
					acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;
				}

				if (tecla==2) { //ESC
					cpu_step_mode.v=0;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                    acumulado=0; //teclas pulsadas
                    //Con esto saldremos

				}

				if (tecla==3) { //background
                    
                    //Mantener emulacion pausada fuera de aqui, ya que estamos en step mode
                    debug_printf(VERBOSE_DEBUG,"Keep emulation paused out of debug cpu as we are in step mode");
                    menu_emulation_paused_on_menu_by_debug_step_mode=1;
                
					cpu_step_mode.v=0;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                    acumulado=0; //teclas pulsadas
                    //Con esto saldremos

				}				

				//Cualquier tecla no enter, no ejecuta instruccion
				if (tecla!=13) {
                    //printf("tecla no es enter. no ejecutar instruccion\n");
                    si_ejecuta_una_instruccion=0;
                }

			}

			else {
				//Cualquier tecla Detiene el continuous loop excepto C
				//printf ("continuos loop\n");
				acumulado=menu_da_todas_teclas();
				if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) != MENU_PUERTO_TECLADO_NINGUNA) {

					//tecla=menu_get_pressed_key();

                	//Detener emulacion porque si no la funcion de leer teclado avanzara en la emulacion
					int antes_menu_emulation_paused_on_menu=menu_emulation_paused_on_menu;
					menu_emulation_paused_on_menu=1;

                    tecla=zxvision_common_getkey_refresh();

					//Restaurar estado pausa emulacion
					menu_emulation_paused_on_menu=antes_menu_emulation_paused_on_menu;


					if (tecla=='c') {
						menu_debug_registers_next_cont_speed();
						tecla=0;
						menu_espera_no_tecla_no_cpu_loop();
					}

					//Si tecla no es 0->0 se suele producir al mover el raton.
					if (tecla!=0) {
						continuous_step=0;
						//printf ("cont step: %d\n",continuous_step);

            			//Decimos que no hay tecla pulsada
            			acumulado=MENU_PUERTO_TECLADO_NINGUNA;

						menu_espera_no_tecla_no_cpu_loop();
					}

				}

			}


			//1 instruccion cpu
			if (si_ejecuta_una_instruccion) {
				//printf ("ejecutando instruccion en step-to-step o continuous. PC=%XH\n",reg_pc);
				debug_core_lanzado_inter.v=0;

				screen_force_refresh=1; //Para que no haga frameskip y almacene los pixeles/atributos en buffer rainbow

                //Resetear posicion de backwards
                indice_debug_cpu_backwards_history=0;


				//Si vista daad (8)
				if (menu_debug_registers_current_view==8) {
					//Poner breakpoint hasta parser

					menu_debug_daad_step_breakpoint();
                    tecla=2; //Simular ESC
					cpu_step_mode.v=0;
					salir_todos_menus=1;
					acumulado=0;

                    
                    //Ademas quitamos el flag de abrir menu que se habia quedado activado
                    //Esto realmente solo hace falta cuando se ejecuta step si el menu debug cpu se ha abierto como consecuencia de un breakpoint
                    //esto fijarse en funcion menu_inicio cuando se usa zxvision_switch_to_window_on_open_menu, que se abre ventana
                    //tanto en caso que haya multitarea como no
                    //Esta sentencia y comentarios estan repetidos en varios sitios
                    //TODO: Creo que en vez de cambiar este menu_event_open_menu.v=0, habria que llamar a menu_inicio_pre_retorno_reset_flags
                    //cuando se sale de la apertura de ventana en el caso de zxvision_switch_to_window_on_open_menu
                    menu_event_open_menu.v=0;
					
                }					

				else {
                    //printf("ejecutando cpu_core_loop. PC=%XH\n",reg_pc);
                    menu_debug_registers_run_cpu_opcode();
                    //printf("despues ejecutando cpu_core_loop. PC=%XH\n",reg_pc);
                }

				//Ver si se ha disparado interrupcion (nmi o maskable)
				//if (debug_core_lanzado_inter.v && debug_core_evitamos_inter.v) {
				if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
					debug_run_until_return_interrupt();
				}


				menu_debug_registers_show_scan_position();
			}

			if (menu_breakpoint_exception.v) {
				//Si accion nula o menu o break
				if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
					menu_debug_registers_gestiona_breakpoint();
				  	//Y redibujar ventana para reflejar breakpoint cond
					//menu_debug_registers_ventana();
				}

				else {
					//menu_breakpoint_exception.v=0;
                    //Gestion acciones. Se gestionan desde el mismo core de debug y aqui no deberian escalarse nunca
					//debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
				}
			}

		}

	//Hacer mientras step mode este activo o no haya tecla pulsada o no haya un salir_todos_menus
	//printf ("acumulado %d cpu_ste_mode: %d\n",acumulado,cpu_step_mode.v);
    //} while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA || cpu_step_mode.v==1);
    } while ( ((acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA || cpu_step_mode.v==1) && !salir_todos_menus);

	//Si no estamos haciendo stepping de daad, quitar breakpoint del parser
	if (debug_stepping_daad.v==0) {
		menu_debug_delete_daad_step_breakpoint();
	}

	//Si no estamos haciendo runto Parse de daad, quitar breakpoint del parser
	if (debug_stepping_daad_runto_parse.v==0) {
		menu_debug_delete_daad_parse_breakpoint();
	}	

	//Antes de restaurar funcion overlay, guardarla en estructura ventana, por si nos vamos a background
	//NO: dado que no tenemos overlay en esta ventana
	//zxvision_set_window_overlay_from_current(ventana);


    

	util_add_window_geometry_compact(ventana);


    //Caso especial. Pulsada tecla background o 
    //salir_todos_menus (que se ha pulsado tecla closeallmenus por ejemplo desde ventana breakpoints, con background permitido)
	if (tecla==3 || (salir_todos_menus && menu_allow_background_windows) ) {
		//En este caso, dado que no hay overlay, borramos contenido de la ventana
		//para que el usuario no piense que se esta actualizando continuamente
		zxvision_cls(ventana);

		zxvision_message_put_window_background();
	}

	else {
		zxvision_destroy_window(ventana);	
        //Se sale de ventana. quitar pausado de emulacion
        menu_emulation_paused_on_menu_by_debug_step_mode=0;	
 	}	


	//zxvision_destroy_window(ventana);

}

zxvision_window zxvision_window_textadv_map;

zxvision_window *menu_debug_textadventure_map_connections_overlay_window;


int menu_debug_textadventure_map_connections_zoom=0;

/*
para zoom 1. dibujo habitacion de 12x12
0123456789012345678901234567890

 --- --- --- 1
 |         | 2
 |         | 3
             4
 |         | 5
 |         | 6
 |         | 7
             8
 |         | 9
 |         | 10
 --- --- --- 11

*/

//Desplazamiento donde empieza el mapa. Las primeras 4 lineas de texto son para opciones y teclas
int map_adventure_offset_x=0;
int map_adventure_offset_y=32;



void menu_debug_textadventure_map_horiz_line(zxvision_window *w,int x,int y,int ancho,int color)
{
    int i;


    for (i=0;i<ancho;i++) {
        textadv_map_putpixel(w,x+map_adventure_offset_x+i,y+map_adventure_offset_y,color);
    }
}

/*void new_menu_debug_textadventure_map_horiz_line(zxvision_window *w,int base_x,int base_y,int offset_x,int offset_y,int ancho,int color)
{
    int i;
    

    printf("horiz line base x: %d longitud: %d\n",base_x,ancho);

    offset_x *=menu_debug_textadventure_map_connections_zoom;
    offset_y *=menu_debug_textadventure_map_connections_zoom;
    ancho *=menu_debug_textadventure_map_connections_zoom;

    printf("mult horiz line base x: %d longitud: %d\n",base_x,ancho);

    for (i=0;i<ancho;i++) {
        zxvision_putpixel(w,base_x+offset_x+i,base_y+offset_y,color);
        printf("horiz line. x:%d\n",base_x+offset_x+i);
    }


}*/

void menu_debug_textadventure_map_vert_line(zxvision_window *w,int x,int y,int alto,int color)
{
    int i;


    for (i=0;i<alto;i++) {
        textadv_map_putpixel(w,x+map_adventure_offset_x,y+map_adventure_offset_y+i,color);
    }
}

/*void new_menu_debug_textadventure_map_vert_line(zxvision_window *w,int base_x,int base_y,int offset_x,int offset_y,int alto,int color)
{
    int i;

    offset_x *=menu_debug_textadventure_map_connections_zoom;
    offset_y *=menu_debug_textadventure_map_connections_zoom;
    alto *=menu_debug_textadventure_map_connections_zoom;

    printf("vert line. x=%d\n",base_x+offset_x);

    for (i=0;i<alto;i++) {
        zxvision_putpixel(w,base_x+offset_x,base_y+offset_y+i,color);
    }
}*/

#define MAP_ADVENTURE_CELL_SIZE 12


//si mostramos objetos
int menu_debug_textadventure_map_connections_show_objects=1;

//si mostramos dibujos
int menu_debug_textadventure_map_connections_show_pictures=1;

//si mostramos habitaciones sin conexiones
int menu_debug_textadventure_map_connections_show_rooms_no_connections=0;

//posicion z actual cuando nos movemos por el mapa sin centrar en posicion actual
int menu_debug_textadventure_map_connections_current_z=0;

//Retornar lo que ocupa cada habitacion segun el nivel de zoom
int menu_debug_textadv_map_conn_get_room_size(void)
{
    if (menu_debug_textadventure_map_connections_zoom==0) return 4;
    else return MAP_ADVENTURE_CELL_SIZE*menu_debug_textadventure_map_connections_zoom;
}

void menu_debug_textadventure_map_connections_put_room(zxvision_window *w,int x,int y,int room,int current_room,int dudosa)
{



    int color_posicion=ESTILO_GUI_PAPEL_OPCION_MARCADA;

    //asumimos por defecto que no se ven
    int mostrar_dibujos=0;

    //temporal. hacer 4 pixeles de cada
    //mapa para zoom mas reducido (zoom=0). pixeles de 4x4
    if (menu_debug_textadventure_map_connections_zoom==0) {
        int ancho,alto;

        x *=4;
        y *=4;

        for (alto=0;alto<4;alto++) {
            for (ancho=0;ancho<4;ancho++) {

                int color=ESTILO_GUI_TINTA_NORMAL;

                if (room==current_room) color=color_posicion;

                textadv_map_putpixel(w,x+map_adventure_offset_x+ancho,y+map_adventure_offset_y+alto,color);
            }
        }
    }

    if (menu_debug_textadventure_map_connections_zoom>=1) {
        int ancho,alto;

        //tamanyo celda total, contando espacio en blanco derecha y abajo
        int tamanyo_celda=MAP_ADVENTURE_CELL_SIZE;

        int longitud_total_linea=tamanyo_celda-1; //11 en zoom 1


        int tamanyo_interior_celda=longitud_total_linea*menu_debug_textadventure_map_connections_zoom;

        tamanyo_celda *=menu_debug_textadventure_map_connections_zoom;

        x *=tamanyo_celda;
        y *=tamanyo_celda;
        
        int tamanyo_punto=2;

        tamanyo_punto *=menu_debug_textadventure_map_connections_zoom;

        //calcular punto central para indicar habitacion si estamos ahi
        int inicio_celda_x=x+map_adventure_offset_x;
        int inicio_celda_y=y+map_adventure_offset_y;

        
        int centro_x=inicio_celda_x+(tamanyo_celda/2);
        int centro_y=inicio_celda_y+(tamanyo_celda/2);

        int inicio_centro_x=centro_x-tamanyo_punto/2;
        int inicio_centro_y=centro_y-tamanyo_punto/2;

        if (current_room==room && menu_debug_textadventure_map_connections_zoom<3) {
            for (alto=0;alto<tamanyo_punto;alto++) {
                for (ancho=0;ancho<tamanyo_punto;ancho++) {

                    int color=ESTILO_GUI_TINTA_NORMAL;

                    color=color_posicion;

                    textadv_map_putpixel(w,inicio_centro_x+ancho,inicio_centro_y+alto,color);
                }
            }
        }

        int x_caracteres=(centro_x/menu_char_width)-1;
        int y_caracteres=centro_y/menu_char_height;        

        //mostrar numero habitacion si zoom suficientemente grande
        if (menu_debug_textadventure_map_connections_zoom>=3) {
            //calcular en que posicion x,y de caracteres se ubica
            char buffer_habitacion[10];

            if (dudosa) {
                sprintf(buffer_habitacion,"%d?",room);
            }
            else {
                sprintf(buffer_habitacion,"%d",room);
            }

            
            if (current_room==room) {
                zxvision_print_string(w,x_caracteres,y_caracteres,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,0,buffer_habitacion);
            }
            else {
                zxvision_print_string_defaults(w,x_caracteres,y_caracteres,buffer_habitacion);
            }
        }


        if (menu_debug_textadventure_map_connections_zoom>=5 && menu_debug_textadventure_map_connections_show_pictures) {
            //mostrar miniatura pantalla siempre que no sea subrutina
            int tinta_attr,paper_attr;
            int is_picture;
            util_daad_get_graphics_attr(room,&tinta_attr,&paper_attr,&is_picture);  

            if (is_picture) mostrar_dibujos=1; 
        }

        if (mostrar_dibujos) {

            int antes_paws_render_total_escalado=paws_render_total_escalado;

            //Solo dejar lineas y puntos. Todo lo demas: colores, bloques, etc desactivado
            int antes_paws_render_disable_block=paws_render_disable_block.v;
            paws_render_disable_block.v=1;

            int antes_paws_render_disable_text=paws_render_disable_text.v;
            paws_render_disable_text.v=1;

            int antes_paws_render_disable_ink=paws_render_disable_ink.v;
            paws_render_disable_ink.v=1;

            int antes_paws_render_disable_bright=paws_render_disable_bright.v;
            paws_render_disable_bright.v=1;

            int antes_paws_render_disable_paper=paws_render_disable_paper.v;
            paws_render_disable_paper.v=1;

            int antes_paws_render_default_ink=paws_render_default_ink;
            int antes_paws_render_default_paper=paws_render_default_paper;
    

            //Por defecto
            paws_render_last_x=0;
            paws_render_last_y=0;    

            paws_render_default_ink=ESTILO_GUI_TINTA_NORMAL;
            paws_render_default_paper=ESTILO_GUI_PAPEL_NORMAL;                  

            paws_render_bright=0;    
            paws_render_mirror_x=+1;
            paws_render_mirror_y=+1;
            paws_render_escala=0;


            

            paws_render_total_escalado=256/tamanyo_interior_celda;
            //reducir aun mas
            paws_render_total_escalado++;


            //printf("tamanyo celda: %d paws_render_total_escalado %d\n",tamanyo_celda,paws_render_total_escalado);
            //printf("maximo: %d\n",255/paws_render_total_escalado);


            paws_render_total_offset_x=x;
            paws_render_total_offset_y=y;
            int contador_limite=0;
            menu_debug_daad_view_graphics_render_recursive(w,room,0,NULL,NULL,NULL,&contador_limite);
            paws_render_total_escalado=antes_paws_render_total_escalado;


            paws_render_total_offset_x=0;
            paws_render_total_offset_y=0;

            paws_render_default_ink=antes_paws_render_default_ink;
            paws_render_default_paper=antes_paws_render_default_paper;            


            paws_render_disable_block.v=antes_paws_render_disable_block;
            paws_render_disable_text.v=antes_paws_render_disable_text;
            paws_render_disable_ink.v=antes_paws_render_disable_ink;
            paws_render_disable_bright.v=antes_paws_render_disable_bright;
            paws_render_disable_paper.v=antes_paws_render_disable_paper;

        }

        else if (menu_debug_textadventure_map_connections_zoom>=5) {
            //Si no se ven los dibujos, al menos mostrar descripción de las localidades
            int y_location=(inicio_celda_y/8)+1;
            int x_location=inicio_celda_x/menu_char_width+1;

            int maxima_longitud=(tamanyo_interior_celda/menu_char_width)-2;

            char texto_localidad[MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH+1];
            util_daad_get_locat_message(room,texto_localidad); 


            //trocear
            //Punteros a cada linea de esas
            //Esto es muy maximo, pero podria pasar
            //Un maximo de MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH lineas
            char *punteros_lineas[MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH];

    

            //maximo permitido en ancho
            #define MAX_GAMEMAP_LENGTH_LINE_LOCATION 100    
            if  (maxima_longitud>=MAX_GAMEMAP_LENGTH_LINE_LOCATION) maxima_longitud=MAX_GAMEMAP_LENGTH_LINE_LOCATION-1;

            
            //char buffer_lineas[256][MAX_GAMEMAP_LENGTH_LINE_LOCATION+1];

            char *buffer_lineas=util_malloc(MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH*MAX_GAMEMAP_LENGTH_LINE_LOCATION,"Can not allocate memory locaton messages");


            //Inicializar punteros a lineas
            int i;
            for (i=0;i<MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH;i++) {
                int offset_linea=i*MAX_GAMEMAP_LENGTH_LINE_LOCATION;
                punteros_lineas[i]=&buffer_lineas[offset_linea];
            }


            //int total_lineas=zxvision_trocear_string_lineas(texto_localidad,punteros_lineas);
            //printf("maxima longitud: %d\n",maxima_longitud);

            int total_lineas=zxvision_generic_message_aux_justificar_lineas(texto_localidad,strlen(texto_localidad),maxima_longitud,punteros_lineas);

            int maximo_permitido_alto=(((tamanyo_celda/2)-1)/8)-1; //-1 de margen de arriba

            if (total_lineas>maximo_permitido_alto) total_lineas=maximo_permitido_alto;
            
            for (i=0;i<total_lineas;i++) {
                    //printf("linea %d : %s\n",i,punteros_lineas[i]);
                    zxvision_print_string_defaults(w,x_location,y_location+i,punteros_lineas[i]);	  
            }            


            //0 al final
            //texto_localidad[maxima_longitud]=0;


            //zxvision_print_string_defaults(w,x_location,y_location,texto_localidad);
        }
            

        


        //mostrar objeto
        if (menu_debug_textadventure_map_connections_zoom>=5 && menu_debug_textadventure_map_connections_show_objects) {
            int objeto;

            //+1 para dar margen y que no se pegue a la izquierda
            int texto_objeto_x=(inicio_celda_x/menu_char_width)+1;
            int texto_objeto_y=y_caracteres+2;

            //TODO: seguro que cabe alguno mas si zoom mas alto, esto deberia recalcularse segun el zoom, pero de momento ya me vale asi
            //int max_mostrar_objetos=4; //con zoom 9 permitiamos 4 objetos

            int max_mostrar_objetos=((tamanyo_interior_celda/2)/menu_char_height)-2; 
            //-2 porque empezamos 2 lineas por debajo

            //caso especial para 12
            //se sale por debajo. Se podria hacer que en general hubiera una linea menos para evitar este caso especial,
            //pero eso supondria que con zoom 5 no cabe ni una linea de objetos (mientras que sí que cabe 1 linea de localidades)
            //mejor asi. Esto siempre es debido a las diferencias a dibujar con pixeles y escribir en posiciones multiples de
            //menu_char_width y menu_char_height
            if (menu_debug_textadventure_map_connections_zoom==12) max_mostrar_objetos--;

            //printf("interior: %d max objetos: %d\n",tamanyo_interior_celda,max_mostrar_objetos);

            int total_objetos_mostrados=0;

            //borrar zona de texto que ocupa el texto de objetos

            //maximo ancho mostrar. -2 para dar margen a la derecha
            int maxima_longitud=(tamanyo_interior_celda/menu_char_width)-2;

            int borra_y,borra_x;

            for (borra_y=0;borra_y<max_mostrar_objetos;borra_y++) {

                for (borra_x=0;borra_x<maxima_longitud;borra_x++) {
                    zxvision_print_string_defaults(w,texto_objeto_x+borra_x,texto_objeto_y+borra_y," ");
                }

            }
            
                  


            for (objeto=0;objeto<util_daad_get_num_objects_description();objeto++) {
                int pos_objeto=util_daad_get_object_value(objeto);

                /*
                TODO: tratar especiales?

                252 es para objetos no creados, por ejemplo, todavía no existen dentro del juego. 
                253 tiene todos los objetos llevados encima (puestos) por el jugador.
                254 tiene todos los objetos llevados por el jugador, pero no puestos encima.                


                */

                if (pos_objeto==room && total_objetos_mostrados<max_mostrar_objetos) {

                    char buffer_temp[256];
                    util_daad_get_object_description(objeto,buffer_temp);


                    //recalcular de nuevo por caracteres que se hayan suprimido
                    int longitud_texto_objeto=strlen(buffer_temp);

                    //si excede
                    if (longitud_texto_objeto>maxima_longitud) {

                        //cortar texto
                        buffer_temp[maxima_longitud]=0;

                        //poner puntos suspensivos
                        //por si acaso. comprobacion adicional
                        if (maxima_longitud>=3) {
                            
                            buffer_temp[maxima_longitud-1]='.';
                            buffer_temp[maxima_longitud-2]='.';
                            buffer_temp[maxima_longitud-3]='.';

                        }
                    }

                    

                    zxvision_print_string_defaults_format(w,texto_objeto_x,texto_objeto_y,buffer_temp);

                    texto_objeto_y++;
                    total_objetos_mostrados++;
                }

            }


        }
                

        //dibujar contorno
        int longitud_linea=3*menu_debug_textadventure_map_connections_zoom;

        int longitud_escalon=1*menu_debug_textadventure_map_connections_zoom;

        

        longitud_total_linea *=menu_debug_textadventure_map_connections_zoom;

        int color_pared=ESTILO_GUI_TINTA_NORMAL;
        int color_paso=ESTILO_GUI_PAPEL_NORMAL; //el paso se dibuja como el color de fondo, para que se vea un hueco tal cual //ESTILO_GUI_PAPEL_SELECCIONADO;        //temporal, color por determinar

/*
para zoom 1. dibujo habitacion de 12x12

x1   x2  x3  x4
0123456789012345678901234567890

*** *** ***  0        y1
*         *  1
*         *  2
             3
*         *  4        y2
*         *  5
*         *  6
             7
*         *  8        y3
*         *  9
*** *** ***  10       y4


Escalones: Entre esquina inferior derecha

Subir. Esquina inferior izquierda


              x8 x7

*         *      ###             y3
*         *      #      
*         *    ###               y5
*         *    #        
*         *    #        
********* *********         y4

                  x2



Bajar. Esquina inferior derecha


            x5 x6

          ###     *         y3
            #     *
            ###   *         y5
              #   *
              #   *
********* *********         y4

          x3

*/        

        int x1=x;
        int y1=y;
        int x2=x+4*menu_debug_textadventure_map_connections_zoom;
        int y2=y+4*menu_debug_textadventure_map_connections_zoom;

        int x3=x+8*menu_debug_textadventure_map_connections_zoom;
        int y3=y+8*menu_debug_textadventure_map_connections_zoom;

        int y4=y1+longitud_total_linea-1;
        int x4=x1+longitud_total_linea-1;

        int y5=y3+longitud_escalon-1;
        int x5=x3+longitud_escalon-1;

        int x6=x5+longitud_escalon-1;

        int x7=x2-longitud_escalon+1;
        int x8=x7-longitud_escalon+1;



        menu_debug_textadventure_map_horiz_line(w,x1,y1,longitud_total_linea,color_pared);

        menu_debug_textadventure_map_horiz_line(w,x1,y4,longitud_total_linea,color_pared);

        menu_debug_textadventure_map_vert_line(w,x1,y1,longitud_total_linea,color_pared);

        menu_debug_textadventure_map_vert_line(w,x4,y1,longitud_total_linea,color_pared);


        //direcciones de esa ubicacion
        int north,south,west,east,northwest,northeast,southwest,southeast,up,down;

        int dudoso_north,dudoso_south,dudoso_west,dudoso_east,dudoso_northwest,dudoso_northeast,dudoso_southwest,dudoso_southeast,dudoso_up,dudoso_down;

        north=text_adventure_connections_table[room].north;
        south=text_adventure_connections_table[room].south;
        west=text_adventure_connections_table[room].west;
        east=text_adventure_connections_table[room].east;

        northwest=text_adventure_connections_table[room].northwest;
        northeast=text_adventure_connections_table[room].northeast;
        southwest=text_adventure_connections_table[room].southwest;
        southeast=text_adventure_connections_table[room].southeast;   

        up=text_adventure_connections_table[room].up;
        down=text_adventure_connections_table[room].down;

        dudoso_north=text_adventure_connections_table[room].dudoso_north;
        dudoso_south=text_adventure_connections_table[room].dudoso_south;
        dudoso_west=text_adventure_connections_table[room].dudoso_west;
        dudoso_east=text_adventure_connections_table[room].dudoso_east;

        dudoso_northwest=text_adventure_connections_table[room].dudoso_northwest;
        dudoso_northeast=text_adventure_connections_table[room].dudoso_northeast;
        dudoso_southwest=text_adventure_connections_table[room].dudoso_southwest;
        dudoso_southeast=text_adventure_connections_table[room].dudoso_southeast;   

        dudoso_up=text_adventure_connections_table[room].dudoso_up;
        dudoso_down=text_adventure_connections_table[room].dudoso_down;

        //Si se puede ir al norte
        if (north>=0) {
            int color_final=color_paso;
            if (dudoso_north) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x2,y1,longitud_linea,color_final);
            //Si es zoom suficientemente grande, y para el caso de salidas dudosas, que se vea mejor la linea, doble de grueso
            //lo mismo para resto de direcciones, no vuelvo a escribir el comentario de nuevo...
            if (menu_debug_textadventure_map_connections_zoom>=3) menu_debug_textadventure_map_horiz_line(w,x2,y1-1,longitud_linea,color_final);
        }

        //Si se puede ir al sur
        if (south>=0) {
            int color_final=color_paso;
            if (dudoso_south) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x2,y4,longitud_linea,color_final);
            if (menu_debug_textadventure_map_connections_zoom>=3) menu_debug_textadventure_map_horiz_line(w,x2,y4+1,longitud_linea,color_final);
        }        

        //Si se puede ir al oeste
        if (west>=0) {
            int color_final=color_paso;
            if (dudoso_west) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_vert_line(w,x1,y2,longitud_linea,color_final);
            if (menu_debug_textadventure_map_connections_zoom>=3) menu_debug_textadventure_map_vert_line(w,x1-1,y2,longitud_linea,color_final);
        }

        //Si se puede ir al este
        if (east>=0) {
            int color_final=color_paso;
            if (dudoso_east) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_vert_line(w,x4,y2,longitud_linea,color_final);
            if (menu_debug_textadventure_map_connections_zoom>=3) menu_debug_textadventure_map_vert_line(w,x4+1,y2,longitud_linea,color_final);
        }     

        //Si se puede ir al noroeste
        if (northwest>=0) {
            int color_final=color_paso;
            if (dudoso_northwest) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x1,y1,longitud_linea,color_final);
            menu_debug_textadventure_map_vert_line(w,x1,y1,longitud_linea,color_final);
            if (menu_debug_textadventure_map_connections_zoom>=3) {
                menu_debug_textadventure_map_horiz_line(w,x1,y1-1,longitud_linea,color_final);
                menu_debug_textadventure_map_vert_line(w,x1-1,y1,longitud_linea,color_final);                
            }
        }   

        //Si se puede ir al noreste
        if (northeast>=0) {
            int color_final=color_paso;
            if (dudoso_northeast) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x3,y1,longitud_linea,color_final);
            menu_debug_textadventure_map_vert_line(w,x4,y1,longitud_linea,color_final);
            if (menu_debug_textadventure_map_connections_zoom>=3) {
                menu_debug_textadventure_map_horiz_line(w,x3,y1-1,longitud_linea,color_final);
                menu_debug_textadventure_map_vert_line(w,x4+1,y1,longitud_linea,color_final);                
            }
        }   

        //Si se puede ir al suroeste
        if (southwest>=0) {
            int color_final=color_paso;
            if (dudoso_southwest) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x1,y4,longitud_linea,color_final);
            menu_debug_textadventure_map_vert_line(w,x1,y3,longitud_linea,color_final);
            if (menu_debug_textadventure_map_connections_zoom>=3) {
                menu_debug_textadventure_map_horiz_line(w,x1,y4+1,longitud_linea,color_final);
                menu_debug_textadventure_map_vert_line(w,x1-1,y3,longitud_linea,color_final);                
            }
        }      

        //Si se puede ir al sureste
        if (southeast>=0) {
            int color_final=color_paso;
            if (dudoso_southeast) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x3,y4,longitud_linea,color_final);
            menu_debug_textadventure_map_vert_line(w,x4,y3,longitud_linea,color_final);
            if (menu_debug_textadventure_map_connections_zoom>=3) {
                menu_debug_textadventure_map_horiz_line(w,x3,y4+1,longitud_linea,color_final);
                menu_debug_textadventure_map_vert_line(w,x4+1,y3,longitud_linea,color_final);                
            }
        }                               

        //Si se puede subir
        if (up>=0) {
            int color_final=color_pared;
            if (dudoso_up) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x7,y3,longitud_escalon,color_final);
            menu_debug_textadventure_map_vert_line(w,x7,y3,longitud_escalon,color_final);
            menu_debug_textadventure_map_horiz_line(w,x8,y5,longitud_escalon,color_final);
            menu_debug_textadventure_map_vert_line(w,x8,y5,longitud_escalon,color_final);
        }         

        //Si se puede bajar
        if (down>=0) {
            int color_final=color_pared;
            if (dudoso_down) color_final=ESTILO_GUI_COLOR_AVISO;
            menu_debug_textadventure_map_horiz_line(w,x3,y3,longitud_escalon,color_final);
            menu_debug_textadventure_map_vert_line(w,x5,y3,longitud_escalon,color_final);
            menu_debug_textadventure_map_horiz_line(w,x5,y5,longitud_escalon,color_final);
            menu_debug_textadventure_map_vert_line(w,x6,y5,longitud_escalon,color_final);
        }

/*
          x3 x5 x6

          ###     *         y3
            #     *
            ###   *         y5
              #   *
              #   *
********* *********         y4  */

      
    }

    
}

//Si centramos el mapa
int menu_debug_textadventure_map_connections_center_current=0;

//si mostrar todas habitaciones siempre, no solo las que se ha entrado jugando
int menu_debug_textadventure_map_connections_show_unvisited=1;



//offsets anteriores de ventana, para saber si se ha movido y hacer cls
int textadventure_map_last_offset_x=0;
int textadventure_map_last_offset_y=0;
//para saber si ha cambiado la z (hemos subido o bajado) y hay que hacer cls
int textadventure_map_last_z=0;




int textadventure_last_total_rooms=0;

//recorrer conexiones solo si ha cambiado el juego
void menu_debug_textadventure_follow_connections(zxvision_window *w)
{

    //para ello, ver diferencias

    //si numero de habitaciones actuales distintas

    int cambiado=0;

    int total_rooms=util_daad_get_num_locat_messages();
    if (total_rooms!=textadventure_last_total_rooms) cambiado=1;

    //printf("total_rooms %d textadventure_last_total_rooms %d\n",total_rooms,textadventure_last_total_rooms);
    //printf("menu_overlay_function %p\n",menu_overlay_function);


    textadventure_last_total_rooms=total_rooms;

    if (cambiado) {
        debug_printf(VERBOSE_DEBUG,"Text Adventure Game has changed. Regenerate connections map");

        //metemos zoom a 0. Esto permite evitar cuelgues a medio cargar de una aventura, como sucede con
        //chichen itza, al cargar se cuelga. location 22, con pictures on. porque se llaman continuamente dos rutinas gosub de grafico
        //Al meterse zoom 0, no se ven dibujos (aunque con chichen itza no siempre evito que se cuelgue, prueba a cargar dos veces esa misma con zoom 5)
        //Esto tambien permite centrar desde offset 0 la aventura
        //NO recrear la ventana, esto no se debe hacer nunca desde funciones overlay, sino desde la rutina principal
        //porque si no, provoca que cambie la ventana activa a esta, independientemente de la ventana en que estemos
        //TODO: si tenemos zoom grande, al cargar otra aventura, zoom pasa a pequeño, pero al no recrearse la ventana (porque NO se debe hacer)
        //se veran las franjas de scroll, porque la ventana sigue siendo grande, hasta que entremos en la ventana y entonces ya si, se recrea pequeña
        menu_debug_textadventure_map_connections_zoom=0;
        zxvision_cls(w);

        init_textadventure_entrada_jugando();
        textadventure_follow_connections(menu_debug_textadventure_map_connections_show_rooms_no_connections);
    }


}

void compass_putpixel(zxvision_window *w,int x,int y,int color)
{
    zxvision_putpixel(w,x,y,color);
}

#define COMPASS_ALTO_TRIANGULO 18
#define COMPASS_ANCHO_TRIANGULO 15

#define COMPASS_ANCHO_LETRA 9
#define COMPASS_ALTO_LETRA 12

#define COMPASS_SEPARACION_LETRA_TRIANGULO 8

#define COMPASS_RADIO_CIRCULO ((COMPASS_ALTO_TRIANGULO/2)+6)

#define COMPASS_ESPACIO_DERECHA (COMPASS_RADIO_CIRCULO+20)

int textadv_map_get_space_compass_right(void)
{
    return COMPASS_ESPACIO_DERECHA;
}


int textadv_map_if_visible_compass(zxvision_window *w)
{
    //solo mostrarlo si hay un minimo de ancho
    if (w->visible_width<30) return 0;

    return 1;    
}

//putpixel evitando escribir en las lineas superiores de texto
/*
void old_textadv_map_putpixel(zxvision_window *w,int x,int y,int color)
{

    if (textadv_map_if_visible_compass(w)) {
        //calcular la posicion y donde se ve en pantalla, o sea, considerando scroll vertical
        int efectiva_y=y-w->offset_y*8;

        if (efectiva_y>=0 && efectiva_y<map_adventure_offset_y) return;

        //Y mas chulo aun. Evitar la zona de la brujula
        int efectiva_x=x-w->offset_x*menu_char_width;

        
        int inicio_compass_x=((w->visible_width-textadv_map_get_space_compass_right() )*menu_char_width)-COMPASS_RADIO_CIRCULO;
        int final_compass_x=inicio_compass_x+COMPASS_RADIO_CIRCULO*2+2*menu_char_width;  //2*menu_char_width para margen por la derecha

        //printf("inicio_compass_x: %d efectiva_x: %d\n",inicio_compass_x,efectiva_x);

        if (efectiva_x>=inicio_compass_x && efectiva_x<final_compass_x) {
            int final_compass_y=map_adventure_offset_y+COMPASS_ALTO_LETRA+COMPASS_SEPARACION_LETRA_TRIANGULO+COMPASS_RADIO_CIRCULO*2;

            if (efectiva_y>=0 && efectiva_y<final_compass_y) return;
        }
    }


    zxvision_putpixel(w,x,y,color);
}
*/


//putpixel evitando escribir en las lineas superiores de texto
void textadv_map_putpixel(zxvision_window *w,int x,int y,int color)
{
    //calcular la posicion y donde se ve en pantalla, o sea, considerando scroll vertical
    int efectiva_y=y-w->offset_y*8;

    if (efectiva_y>=0 && efectiva_y<map_adventure_offset_y) return;

    zxvision_putpixel(w,x,y,color);
}

//Dibujar señal que apunta al norte
void menu_debug_textadventure_map_connections_draw_map_compass(zxvision_window *w)
{

    //solo mostrarlo si hay un minimo de ancho
    if (!textadv_map_if_visible_compass(w) ) return;

    //inicio del texto considerando scroll horizontal y vertical
    int texto_x=w->offset_x+w->visible_width;
    int texto_y=w->offset_y;

      

    int pos_x_linea=texto_x*menu_char_width-textadv_map_get_space_compass_right();
    int pos_y_linea=(texto_y+(map_adventure_offset_y/8))*8;


    int p1_x;
    int p1_y;

    int p2_x;
    int p2_y;

    int p3_x;
    int p3_y;

    int p4_x;
    int p4_y;
    
    //int p5_x;
    //int p5_y;    

    //la "n" de la brujula
    /*

    p1    p2
    *     *
    **    *
    * *   *
    *  *  *
    *   * *
    *    **
    *     *
    p3    p4 
    
    */
    int ancho_letra=COMPASS_ANCHO_LETRA;
    int alto_letra=COMPASS_ALTO_LETRA;


    p1_x=pos_x_linea;
    p1_y=pos_y_linea; 

    p2_x=pos_x_linea+ancho_letra;
    p2_y=p1_y;    

    p3_x=p1_x;
    p3_y=p1_y+alto_letra;

    p4_x=p2_x;
    p4_y=p3_y;

    int ancho_triangulo=COMPASS_ANCHO_TRIANGULO;
    int alto_triangulo=COMPASS_ALTO_TRIANGULO;    


    //Primero borramos (ponemos con color de fondo) la zona de la brujula, para evitar que haya texto que se meta ahi y se mezcle
    int inicio_borrado_x=p1_x+ancho_letra/2-COMPASS_RADIO_CIRCULO;
    int final_borrado_x=inicio_borrado_x+COMPASS_RADIO_CIRCULO*2;

    int inicio_borrado_y=p1_y;
    

    int centro_circulo_y=pos_y_linea+alto_letra+COMPASS_SEPARACION_LETRA_TRIANGULO+alto_triangulo/2;
    int final_borrado_y=centro_circulo_y+COMPASS_RADIO_CIRCULO;


    int bx,by;
    for (by=inicio_borrado_y;by<=final_borrado_y;by++) {
        for (bx=inicio_borrado_x;bx<=final_borrado_x;bx++) {
            zxvision_putpixel(w,bx,by,ESTILO_GUI_PAPEL_NORMAL);
        }
    }


    zxvision_draw_line(w,p1_x,p1_y,p3_x,p3_y,ESTILO_GUI_TINTA_NORMAL,compass_putpixel);
    zxvision_draw_line(w,p1_x,p1_y,p4_x,p4_y,ESTILO_GUI_TINTA_NORMAL,compass_putpixel);
    zxvision_draw_line(w,p2_x,p2_y,p4_x,p4_y,ESTILO_GUI_TINTA_NORMAL,compass_putpixel);        



    //el "triangulo" de la brujula
/*

brujula sencilla


           *             p1
          * *
         *   *
        *     *
       *       *
      *    *    *          p4
     *    * *    *
    *   *     *   *
   *  *         *  *
  * *             * *
 **                 **     p2,p3


*/    


    p1_x=pos_x_linea+ancho_letra/2;
    p1_y=pos_y_linea+alto_letra+COMPASS_SEPARACION_LETRA_TRIANGULO; //COMPASS_SEPARACION_LETRA_TRIANGULO pixeles por debajo de la letra

    p2_x=p1_x-ancho_triangulo/2;
    p2_y=p1_y+alto_triangulo;

    p3_x=p1_x+ancho_triangulo/2;
    p3_y=p2_y;

    p4_x=p1_x;
    p4_y=p1_y+alto_triangulo/2;

    //p5_x=p1_x;
    //p5_y=p1_y+alto_triangulo/2;    


    //circulo. En el centro del triangulo
    int radio=COMPASS_RADIO_CIRCULO; //(alto_triangulo/2)+6;


    /*
    //primero "borramos" lo de dentro del circulo, con color de fondo, para poder ocultar cualquier texto que se meta dentro del circulo
    //esto seria una especie de mascara cutre para que asi nuestro circulo de radio este vacio por dentro sin que el texto pueda entrar ahi
    int r;
    for (r=radio-1;r>0;r--) {
        zxvision_draw_ellipse(w,p5_x,p5_y,r,r,ESTILO_GUI_PAPEL_NORMAL,compass_putpixel,360);
    }
    */

    //El circulo
    zxvision_draw_ellipse(w,p4_x,p4_y,radio,radio,ESTILO_GUI_TINTA_NORMAL,compass_putpixel,360);    


    //la flecha de la brujula
    zxvision_draw_line(w,p1_x,p1_y,p2_x,p2_y,ESTILO_GUI_TINTA_NORMAL,compass_putpixel);
    zxvision_draw_line(w,p2_x,p2_y,p4_x,p4_y,ESTILO_GUI_TINTA_NORMAL,compass_putpixel);
    zxvision_draw_line(w,p4_x,p4_y,p3_x,p3_y,ESTILO_GUI_TINTA_NORMAL,compass_putpixel);
    zxvision_draw_line(w,p3_x,p3_y,p1_x,p1_y,ESTILO_GUI_TINTA_NORMAL,compass_putpixel);
    
    
}



void menu_debug_textadventure_map_connections_overlay(void)
{
    //printf("overlay %d\n",contador_segundo);

    if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_debug_textadventure_map_connections_overlay_window->is_minimized) return;

    //printf("overlay adventure graphics render %d\n",contador_segundo);

    zxvision_window *w;

    w=menu_debug_textadventure_map_connections_overlay_window;

    //ajustar offset x
    map_adventure_offset_x=menu_char_width;



    if (util_textadventure_is_daad_quill_paws() ) {

        menu_debug_textadventure_follow_connections(w);

        //Si tenemos que borrar ventana porque se ha desplazado scroll
        //sin esto, los mensajes que aparecen siempre arriba se irian repartiendo por la ventana a medida que se hace scroll
        if (w->offset_x!=textadventure_map_last_offset_x || w->offset_y!=textadventure_map_last_offset_y) {
            //printf("Hacemos cls porque se ha desplazado ventana\n");
            zxvision_cls(w);
            textadventure_map_last_offset_x=w->offset_x;
            textadventure_map_last_offset_y=w->offset_y;
        }





        //de momento recorrer todas habitaciones
        int ancho_mapa,alto_mapa,min_x_mapa,max_x_mapa,min_y_mapa,max_y_mapa;
        
        textadventure_get_size_map(0,0,&ancho_mapa,&alto_mapa,&min_x_mapa,&max_x_mapa,&min_y_mapa,&max_y_mapa,1);    

        int i;
        int z;

        z=menu_debug_textadventure_map_connections_current_z;

        //z sera la de la habitacion actual si centramos
        int current_room=util_textadventure_get_current_location();

        if (current_room<TEXT_ADVENTURE_MAX_LOCATIONS) {

            if (text_adventure_connections_table[current_room].recorrida && menu_debug_textadventure_map_connections_center_current) {
           
                menu_debug_textadventure_map_connections_current_z=text_adventure_connections_table[current_room].z;   
            }

        }      

        //Si ha cambiado posicion z
        if (z!=textadventure_map_last_z) {
            //printf("Hacemos cls porque ha cambiado la z (current: %d last: %d)\n",z,textadventure_map_last_z);
            zxvision_cls(w);
        }

        textadventure_map_last_z=z;


        for (i=0;i<util_daad_get_num_locat_messages();i++) {
            //printf("location: %d\n",i);
            if (text_adventure_connections_table[i].recorrida && textadventure_room_has_exits(i) &&
                text_adventure_connections_table[i].z==z /* &&
                !text_adventure_connections_table[i].habitacion_dudosa */
            
            ) {
                int x=text_adventure_connections_table[i].x;
                int y=text_adventure_connections_table[i].y;

                //invertir coordenada, de arriba a abajo
                y=max_y_mapa-y;



                //int color=0;

                //if (i==current_room) color=2;

                //Y si esta el setting de mostrar todos o la hemos entrado jugando

                if (i==current_room) {
                    text_adventure_connections_table[i].entrado_jugando=1;
                }

                int mostrar=0;
                if (menu_debug_textadventure_map_connections_show_unvisited) {
                    mostrar=1;
                }

                else {
                    if (text_adventure_connections_table[i].entrado_jugando) mostrar=1;
                }

                if (mostrar) menu_debug_textadventure_map_connections_put_room(w,x,y,i,current_room,text_adventure_connections_table[i].habitacion_dudosa);


                //Centrar el mapa en la zona de habitacion actual
                if (menu_debug_textadventure_map_connections_center_current && i==current_room) {
                    int tamanyo_habitacion=menu_debug_textadv_map_conn_get_room_size();

                    //justo el centro de la habitacion
                    int offset_x=(x*tamanyo_habitacion+map_adventure_offset_x+tamanyo_habitacion/2)/menu_char_width;
                    int offset_y=(y*tamanyo_habitacion+map_adventure_offset_y+tamanyo_habitacion/2)/8;
                    
                    //cuanto es la mitad de pantalla
                    int mitad_ancho=(w->visible_width)/2;
                    int mitad_alto=(w->visible_height)/2;

                    offset_x -=mitad_ancho;
                    offset_y -=mitad_alto;

                    if (offset_x<0) offset_x=0;
                    if (offset_y<0) offset_y=0;

                    //printf("Setting offset x %d y %d (room x %d y %d)\n",offset_x,offset_y,x,y);

                    //Solo cambiarlo si son distintos. Si lo cambiamos siempre esto provoca que la linea de scroll este
                    //dibujandose siempre por encima de otras ventanas activas
                    //Y controlar el maximo, porque si le metemos mas de lo permitido, aunque no controla, se redibujan las barras de scroll
                    //y se meten por encima de otras ventanas
                    int max_offset_x=zxvision_maximum_offset_x(w);
                    if (offset_x>max_offset_x) offset_x=max_offset_x;

                    if (w->offset_x!=offset_x) {
                        debug_printf(VERBOSE_DEBUG,"Text adventure map. Following current location and set window scroll x: %d (was %d)",offset_x,w->offset_x);
                        zxvision_set_offset_x(w,offset_x);
                    }

                    int max_offset_y=zxvision_maximum_offset_y(w);
                    if (offset_y>max_offset_y) offset_y=max_offset_y;

                    if (w->offset_y!=offset_y) {
                        debug_printf(VERBOSE_DEBUG,"Text adventure map. Following current location and set window scroll y: %d (was %d)",offset_y,w->offset_y);
                        zxvision_set_offset_y(w,offset_y);
                    }
                

                }
            }


            //printf("room %3d: %d,%d,%d\n",
            //    i,text_adventure_connections_table[i].x,text_adventure_connections_table[i].y,text_adventure_connections_table[i].z);
        }

        //Mostrar este texto siempre arriba de pantalla,
        //en este caso no nos sirve     ventana->upper_margin pues eso solo vale para scroll vertical, si se hace scroll horizontal se desplaza
        //Escribimos el texto en la posicion 1,1 sumando el scroll de la ventana propiamente

        int texto_x=1+w->offset_x;
        int texto_y=w->offset_y;

        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;

        //Forzar a mostrar atajos
        menu_writing_inverse_color.v=1;


        zxvision_print_string_defaults_fillspc_format(w,texto_x,texto_y,"Zoom: %d Current location: %d Coords: %d,%d Z: %d",
        menu_debug_textadventure_map_connections_zoom,current_room,
            text_adventure_connections_table[current_room].x,text_adventure_connections_table[current_room].y,menu_debug_textadventure_map_connections_current_z);


        texto_y++;


        zxvision_print_string_defaults_fillspc_format(w,texto_x,texto_y,"~~Z: -zoom ~~X: +zoom ~~up ~~down ~~Teleport ~~f~~1: help");

        texto_y++;   

        zxvision_print_string_defaults_fillspc_format(w,texto_x,texto_y,"[%c] ~~Follow [%c] ~~Objects [%c] ~~Pictures",
            (menu_debug_textadventure_map_connections_center_current ? 'X' : ' ' ),
            (menu_debug_textadventure_map_connections_show_objects ? 'X' : ' ' ),
            (menu_debug_textadventure_map_connections_show_pictures ? 'X' : ' ' )
            );    

        texto_y++;      

        zxvision_print_string_defaults_fillspc_format(w,texto_x,texto_y,"[%c] Also un~~visited [%c] Also un~~connected",            
            (menu_debug_textadventure_map_connections_show_unvisited ? 'X' : ' ' ),
            (menu_debug_textadventure_map_connections_show_rooms_no_connections ? 'X' : ' ' )
            );


        //Restaurar mostrar atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


        //Dibujar señal que apunta al norte
        menu_debug_textadventure_map_connections_draw_map_compass(w);
    }     

    else {
        zxvision_set_offset_x(w,0);
        zxvision_set_offset_y(w,0);
        zxvision_cls(w);
        zxvision_print_string_defaults_fillspc(w,1,0,"No DAAD/PAWS/Quill game loaded");
    }

    zxvision_draw_window_contents(w);

}

void menu_debug_textadventure_map_connections_set_center_current(void)
{
    menu_debug_textadventure_map_connections_center_current ^=1;
}

#define MAX_TEXTADVENTURE_MAP_ZOOM 14

void menu_debug_textadventure_map_connections_inc_zoom(void)
{
    if (menu_debug_textadventure_map_connections_zoom<MAX_TEXTADVENTURE_MAP_ZOOM) menu_debug_textadventure_map_connections_zoom++;

}

void menu_debug_textadventure_map_connections_dec_zoom(void)
{
    if (menu_debug_textadventure_map_connections_zoom>0) menu_debug_textadventure_map_connections_zoom--;

    if (menu_debug_textadventure_map_connections_zoom==0) {
    
        //si no esta centrado, ponemos offset 0, porque al hacerse pequeña podemos ver las vista de donde esta el mapa
        if (!menu_debug_textadventure_map_connections_center_current) {
            zxvision_set_offset_x(menu_debug_textadventure_map_connections_overlay_window,0);
            zxvision_set_offset_y(menu_debug_textadventure_map_connections_overlay_window,0);
        }
    }

}

void menu_debug_textadventure_map_connections_teleport(void)
{
    int actual=util_textadventure_get_current_location();

    int total_locations=util_daad_get_num_locat_messages();

    menu_ventana_scanf_numero_enhanced("Location",&actual,4,+1,0,total_locations,0);

    util_daad_put_flag_value(util_textadventure_get_current_location_flag(),actual);
}

void menu_debug_textadventure_map_connections_create_window(zxvision_window *ventana)
{
    int ancho_ventana,alto_ventana,xventana,yventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

    if (!util_find_window_geometry("textadvmap",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {

        ancho_ventana=40;

        alto_ventana=22;


        xventana=menu_center_x()-ancho_ventana/2;
        yventana=menu_center_y()-alto_ventana/2;

    }    


    //obener ancho y alto total
    int ancho_mapa,alto_mapa,min_x_mapa,max_x_mapa,min_y_mapa,max_y_mapa;
    
    textadventure_get_size_map(0,0,&ancho_mapa,&alto_mapa,&min_x_mapa,&max_x_mapa,&min_y_mapa,&max_y_mapa,1); 

    //de momento fuerzo alto y ancho total
    int alto_total; //=400;
    int ancho_total; //=300;     


    if (menu_debug_textadventure_map_connections_zoom==0) {
        ancho_total=ancho_mapa/2;  //de 8/4 pixeles por cada ubicacion
        alto_total=alto_mapa/2;
    }

    else {
        int tamanyo_celda=MAP_ADVENTURE_CELL_SIZE;

        tamanyo_celda *=menu_debug_textadventure_map_connections_zoom;  

        ancho_total=ancho_mapa;
        alto_total=alto_mapa;        

        ancho_total *=tamanyo_celda;  
        alto_total *=tamanyo_celda;  

        //pasar a caracteres
        ancho_total /=menu_char_width;
        alto_total /=8;
    }

    //darle mas para los offsets
    ancho_total +=map_adventure_offset_x/menu_char_width;
    alto_total +=map_adventure_offset_y/8;

    //1 mas por cada, de los decimales al dividir
    ancho_total++;
    alto_total++;

    //Y un minimo asegurado
    if (ancho_total<50) ancho_total=50;
    if (alto_total<20) alto_total=20;

    zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_total,alto_total,
            "Text Adventure Map","textadvmap",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

    ventana->can_be_backgrounded=1;

    //No refrescar contenido al cambiar scroll. Esto permite que la parte superior de la ventana, donde estan
    //las teclas y la leyenda no se mueva temporalmente al cambiar scroll. Total la funcion de overlay ya 
    //refrescara convenientemente
    ventana->no_refresh_change_offset=1;

    //decimos que tiene que borrar fondo cada vez al redibujar
    //por tanto es como decirle que no use cache de putchar
    //dado que el fondo de texto es casi todo texto con caracter " " eso borra los pixeles que metemos con overlay del frame anterior
    ventana->must_clear_cache_on_draw=1;    
}


void menu_debug_textadventure_map_connections_recreate_window(zxvision_window *ventana)
{
    //recrear para calcular tamaño segun zoom
    
    //guardar geometria por si se redimensiona y luego pulsa boton zoom, que sepa tamaño anterior
    util_add_window_geometry_compact(ventana);    
    zxvision_destroy_window(ventana);

    menu_debug_textadventure_map_connections_create_window(ventana);

    zxvision_draw_window(ventana);        
}

//incdec: si es +1, es incrementar. Si 0, decrementar
void menu_debug_textadventure_map_incdec_zoom(zxvision_window *ventana,int incdec)
{
    //Obtener el tanto por ciento (por 10000 realmente) del scroll usado
    //TODO: mejorar esto, si reduzco y amplio varias veces, se va desplazando la vista, no quedandose siempre en el mismo punto
    //Se ha probado tambien, en vez de sacar el porcentaje, sino aplicando el cociente de antes_offset_x/antes_total_width
    //pero tambien resulta ser algo imperfecto. Creo que la solución perfecta dependería de cambiar el scroll para hacerlo de otra 
    //manera, como por ejemplo saber qué localidad tengo visible en pantalla y aplicar scroll siempre para que estuviera ahi visible
    int porcentaje_scroll_x;
    //Por si acaso evitar divisiones por 0, aunque no deberia suceder este caso nunca
    if (ventana->total_width==0) porcentaje_scroll_x=0;
    else porcentaje_scroll_x=(ventana->offset_x*10000)/ventana->total_width;

    int porcentaje_scroll_y;
    //Por si acaso evitar divisiones por 0, aunque no deberia suceder este caso nunca
    if (ventana->total_height==0) porcentaje_scroll_y=0;
    else porcentaje_scroll_y=(ventana->offset_y*10000)/ventana->total_height;

    if (incdec) menu_debug_textadventure_map_connections_inc_zoom();
    else menu_debug_textadventure_map_connections_dec_zoom();

    menu_debug_textadventure_map_connections_recreate_window(ventana);        

    //Y aplicar el scroll
    int scroll_final_x=(ventana->total_width*porcentaje_scroll_x)/10000;
    int scroll_final_y=(ventana->total_height*porcentaje_scroll_y)/10000;

    zxvision_set_offset_x(ventana,scroll_final_x);
    zxvision_set_offset_y(ventana,scroll_final_y);
}


void menu_debug_text_adventure_help(void)
{
    if (gui_language==GUI_LANGUAGE_SPANISH) {
        menu_generic_message("Ayuda",
        "Esta ventana es un mapa de la aventura conversacional. Se genera automaticamente sin necesidad de visitar cada posicion (localidad).\n"
        "ZEsarUX se encarga de visitar el mapeado de localidades entero nada mas cargar el juego, utilizando un algoritmo recursivo, "
        "donde se genera un mapa en cuadricula y se van ubicando cada localidad.\n"
        "\n"
        "El resultado es este mapa, donde se muestra cada localidad, y en cada una:\n"
        "- Salidas de cada localidad, indicando salidas posibles en cada punto cardinal (norte, sur, este y oeste), sus diagonales, y subir y bajar\n"
        "- Descripciones de cada localidad, en caso que dicha localidad no tenga dibujo o los hayamos deshabilitado del todo\n"
        "- Dibujos de cada localidad\n"
        "- Objetos de cada localidad\n"
        "Dependiendo del zoom aplicado, se ven mas detalles o menos.\n"
        "\n"
        "Debido al funcionamiento del algoritmo, y que las aventuras realmente no nos indican la posicion en el mapa, es posible que en ocasiones "
        "algunas localidades ocupen las mismas posiciones que otras. En este caso, se puede visualizar de la siguiente manera:\n"
        "- Localizaciones con ? en el numero son dudosas, indican ubicacion ocupada por mas de una localidad, como bosques...\n"
        "- Salidas marcadas en rojo nos llevan a habitaciones dudosas.\n"
        "\n"
        "Las siguientes teclas actuan sobre el mapa:\n"
        "z: Reducir zoom\n"
        "x: Aumentar zoom\n"
        "u: Subir\n"
        "d: Bajar\n"
        "t: Teletransporte, permite modificar la habitacion actual del juego\n"
        "f1: Esta ayuda\n"
        "f: Seguir la posicion actual en el mapa\n"
        "o: Mostrar objetos\n"
        "p: Mostrar dibujos\n"
        "v: Mostrar todas las localidades, o solo las localidades que hemos visitado\n"
        "c: Mostrar tambien las habitaciones sin conectar (que no tienen salidas)\n"
        "Aparte de estas teclas, mediante los cursores, PgUp/Down, y el raton, se puede desplazar el mapa convenientemente.\n"
        );
    }

    else if (gui_language==GUI_LANGUAGE_CATALAN) {
        menu_generic_message("Ajuda",
        "Aquesta finestra és un mapa de l'aventura conversacional. Es genera automàticament sense necessitat de visitar cada posició (localitat).\n"
        "ZEsarUX s'encarrega de visitar el mapejat de localitats sencer tot just carregar el joc, utilitzant un algoritme recursiu,"
        "on es genera un mapa en quadrícula i es van ubicant cada localitat.\n"
        "\n"
        "El resultat és aquest mapa, on es mostra cada localitat, i a cadascuna:\n"
        "- Sortides de cada localitat, indicant sortides possibles a cada punt cardinal (nord, sud, est i oest), les seves diagonals, i pujar i baixar\n"
        "- Descripcions de cada localitat, en cas que aquesta localitat no tingui dibuix o els haguem deshabilitat del tot\n"
        "- Dibuixos de cada localitat\n"
        "- Objectes de cada localitat\n"
        "Depenent del zoom aplicat, es veuen més detalls o menys.\n"
        "\n"
        "A causa del funcionament de l'algoritme, i que les aventures realment no ens indiquen la posició al mapa, és possible que de vegades "
        "algunes localitats ocupin les mateixes posicions que altres. En aquest cas, es pot visualitzar de la manera següent:\n"
        "- Localitzacions amb ? en el número són dubtoses, indiquen ubicació ocupada per mes d'una localitat, com a boscos...\n"
        "- Sortides marcades en vermell ens porten a habitacions dubtoses.\n"
        "\n"
        "Les tecles següents actuen sobre el mapa:\n"
        "z: Reduir zoom\n"
        "x: Augmentar zoom\n"
        "u: Pujar\n"
        "d: Baixar\n"
        "t: Teletransport, permet modificar l'habitació actual del joc\n"
        "f1: Aquesta ajuda\n"
        "f: Seguir la posició actual al mapa\n"
        "o: Mostra objectes\n"
        "p: Mostra dibuixos\n"
        "v: Mostra totes les localitats, o només les localitats que hem visitat\n"
        "c: Mostra també les habitacions sense connectar (que no tenen sortides)\n"
        "A part d'aquestes tecles, mitjançant els cursors, PgUp/Down, i el ratolí, podeu desplaçar el mapa convenientment.\n"
        );
    }

    else {
        menu_generic_message("Help",
        "This window is a map of the text adventure. It is generated automatically without the need to visit each position (location).\n"
        "ZEsarUX takes care of visiting the entire location map as soon as the game loads, using a recursive algorithm, "
        "where a grid map is generated and each location is located.\n"
        "\n"
        "The result is this map, where each location is shown, and in each one:\n"
        "- Exits of each location, indicating possible exits in each cardinal point (north, south, east and west), their diagonals, and going up and down\n"
        "- Descriptions of each location, in case location does not have a drawing or we have disabled them completely\n"
        "- Drawings of each location\n"
        "- Items from each location\n"
        "Depending on the zoom applied, more or less details are seen.\n"
        "\n"
        "Due to the way the algorithm works, and that the adventures don't really indicate the position on the map, it is possible that sometimes "
        "some localities occupy the same positions as others. In this case, it can be displayed as follows:\n"
        "- Locations with ? in the number are doubtful, they indicate location occupied by more than one location, like forests...\n"
        "- Exits marked in red lead us to doubtful rooms.\n"
        "\n"
        "The following keys act on the map:\n"
        "z: Zoom out\n"
        "x: Zoom in\n"
        "u: Go up\n"
        "d: Go down\n"
        "t: Teleport, allows you to modify the current room in the game\n"
        "f1: This help window\n"
        "f: Follow the current position on the map\n"
        "o: Show objects\n"
        "p: Show pictures\n"
        "v: Show all locations, or just the locations we have visited\n"
        "c: Show also unconnected rooms (that have no exits)\n"
        "Apart from these keys, by using the arrow keys, PgUp/Down, and the mouse, you can scroll the map conveniently.\n"                  
        );
    }
}

void menu_debug_textadventure_map_connections(MENU_ITEM_PARAMETERS)
{


    //Renderizar un grafico de paws

    menu_espera_no_tecla();
    menu_reset_counters_tecla_repeticion();    

    zxvision_window *ventana;
    ventana=&zxvision_window_textadv_map; 

    //IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
    //si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
    //la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
    zxvision_delete_window_if_exists(ventana);



    menu_debug_textadventure_map_connections_create_window(ventana);

    zxvision_draw_window(ventana);

    //Cambiamos funcion overlay de texto de menu
    //Por cierto que esta ventana no la permitimos que se haga background. Por que? Quizá no tiene sentido,
    //pues no el contenido no varia a no ser que se esté en la ventana y tocando opciones
    //Ademas, creo que puede provocar efectos inesperados si esta la ventana abierta y cargamos otro juego
    menu_debug_textadventure_map_connections_overlay_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui    

    set_menu_overlay_function(menu_debug_textadventure_map_connections_overlay);

    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
        //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
        return;
    }    


    //Entrar en la ventana siempre regenera las conexiones, por si la deteccion automatica de cambio de aventura en el overlay no funciona
    textadventure_follow_connections(menu_debug_textadventure_map_connections_show_rooms_no_connections);


    //ajustar offset x
    //map_adventure_offset_x=menu_char_width;

    z80_byte tecla;
    do {

        //borrar restos de texto anterior (numeros habitaciones, etc)
        zxvision_cls(ventana);


        //int antes_scroll_x=ventana->offset_x;
        //int antes_scroll_y=ventana->offset_y;

        tecla=zxvision_common_getkey_refresh();
        zxvision_handle_cursors_pgupdn(ventana,tecla);

        //si se ha hecho scroll, detectarlo y quitar parametro centrado
        //esto de momento no reacciona bien. TODO: ver porque
        /*
        if (menu_debug_textadventure_map_connections_center_current) {
            if (ventana->offset_x!=antes_scroll_x || ventana->offset_y!=antes_scroll_y) {
                menu_debug_textadventure_map_connections_center_current=0;
            }
        }
        */


       //Teclas no haran nada si no se ha cargado una aventura
       if (util_textadventure_is_daad_quill_paws()) {

            if (tecla=='z' && menu_debug_textadventure_map_connections_zoom>0) {
                menu_debug_textadventure_map_incdec_zoom(ventana,0);
            }   

            if (tecla=='x' && menu_debug_textadventure_map_connections_zoom<MAX_TEXTADVENTURE_MAP_ZOOM ) {
                menu_debug_textadventure_map_incdec_zoom(ventana,1);
            }   


            if (tecla=='f') {
                menu_debug_textadventure_map_connections_set_center_current();
            }

            if (tecla=='t') {
                menu_debug_textadventure_map_connections_teleport();
                zxvision_draw_window(ventana);
            }    

            if (tecla=='v') {
                menu_debug_textadventure_map_connections_show_unvisited ^=1;
            }

            if (tecla=='o') {
                menu_debug_textadventure_map_connections_show_objects ^=1;
            }

            if (tecla=='p') {
                menu_debug_textadventure_map_connections_show_pictures ^=1;
            }            

            if (tecla=='c') {
                menu_debug_textadventure_map_connections_show_rooms_no_connections ^=1;

                //Y volver a crear mapa
                textadventure_follow_connections(menu_debug_textadventure_map_connections_show_rooms_no_connections);

                menu_debug_textadventure_map_connections_recreate_window(ventana);                    
            }

            if (tecla=='u') {
                menu_debug_textadventure_map_connections_current_z++;
            }

            if (tecla=='d') {
                menu_debug_textadventure_map_connections_current_z--;
            }

            if (tecla==MENU_TECLA_AYUDA) {
                menu_debug_text_adventure_help();
            }


       }    

        //printf ("tecla: %d\n",tecla);
    } while (tecla!=2 && tecla!=3);    


    
    //Antes de restaurar funcion overlay, guardarla en estructura ventana, por si nos vamos a background
    zxvision_set_window_overlay_from_current(ventana);


 

        //restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);


    

    //Grabar geometria ventana
    util_add_window_geometry_compact(ventana);    

    //printf("salir_todos_menus %d\n",salir_todos_menus);
  

    if (tecla==3) {
        zxvision_message_put_window_background();
    }

    else {
        zxvision_destroy_window(ventana);
    }         
}

