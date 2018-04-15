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

#ifndef DEBUG_H
#define DEBUG_H

#include "cpu.h"

#define DEBUG_STRING_FLAGS         ( Z80_FLAGS & FLAG_S ? 'S' : ' '), ( Z80_FLAGS & FLAG_Z ? 'Z' : ' '), ( Z80_FLAGS & FLAG_5 ? '5' : ' '), ( Z80_FLAGS & FLAG_PV ? 'P' : ' '), ( Z80_FLAGS & FLAG_3 ? '3' : ' '), ( Z80_FLAGS & FLAG_H ? 'H' : ' '), ( Z80_FLAGS & FLAG_N ? 'N' : ' '), ( Z80_FLAGS & FLAG_C ? 'C' : ' ')

#define DEBUG_STRING_FLAGS_SHADOW         ( Z80_FLAGS_SHADOW & FLAG_S ? 'S' : ' '), ( Z80_FLAGS_SHADOW & FLAG_Z ? 'Z' : ' '), ( Z80_FLAGS_SHADOW & FLAG_5 ? '5' : ' '), ( Z80_FLAGS_SHADOW & FLAG_PV ? 'P' : ' '), ( Z80_FLAGS_SHADOW & FLAG_3 ? '3' : ' '), ( Z80_FLAGS_SHADOW & FLAG_H ? 'H' : ' '), ( Z80_FLAGS_SHADOW & FLAG_N ? 'N' : ' '), ( Z80_FLAGS_SHADOW & FLAG_C ? 'C' : ' ')


extern void print_registers(char *buffer);

extern void cpu_panic(char *mensaje);

extern void debug_tiempo_inicial(void);
extern void debug_tiempo_final(void);
extern int verbose_level;
extern void debug_printf (int debuglevel,__const char *__restrict __format, ...);

extern z80_bit menu_breakpoint_exception;

//breakpoints maximos de pc. no tiene mucho sentido porque se puede hacer con condicion pc=
//#define MAX_BREAKPOINTS 5

extern void init_breakpoints_table(void);

//extern int debug_breakpoints_array[];

#define MAX_BREAKPOINT_CONDITION_LENGTH 256

#define MAX_BREAKPOINTS_CONDITIONS 100
extern char debug_breakpoints_conditions_array[MAX_BREAKPOINTS_CONDITIONS][MAX_BREAKPOINT_CONDITION_LENGTH];

//Acciones al saltar un breakpoint
extern char debug_breakpoints_actions_array[MAX_BREAKPOINTS_CONDITIONS][MAX_BREAKPOINT_CONDITION_LENGTH];


extern int debug_breakpoints_conditions_saltado[MAX_BREAKPOINTS_CONDITIONS];

extern int debug_breakpoints_conditions_enabled[MAX_BREAKPOINTS_CONDITIONS];

extern z80_bit debug_breakpoints_cond_behaviour;

//#define MAX_BREAKPOINTS_PEEK 4
//extern int debug_breakpoints_peek_array[];

//extern z80_byte (*peek_byte_no_time_no_debug)(z80_int dir);
//extern z80_byte (*peek_byte_no_debug)(z80_int dir);
extern z80_byte peek_byte_no_time_debug (z80_int dir,z80_byte value GCC_UNUSED);
extern z80_byte peek_byte_debug (z80_int dir,z80_byte value GCC_UNUSED);

extern z80_byte poke_byte_no_time_debug(z80_int dir,z80_byte value);
extern z80_byte poke_byte_debug(z80_int dir,z80_byte value);


extern void out_port_debug(z80_int puerto,z80_byte value);
extern z80_byte lee_puerto_debug(z80_byte puerto_h,z80_byte puerto_l);


extern void set_peek_byte_function_debug(void);
extern void reset_peek_byte_function_debug(void);

//4 lineas
#define MAX_MESSAGE_CATCH_BREAKPOINT (32*4)
extern char catch_breakpoint_message[];

extern int catch_breakpoint_index;

extern void do_breakpoint_exception(char *message);


extern z80_bit debug_breakpoints_enabled;

//extern z80_bit debug_cpu_core_loop;
extern void set_cpu_core_loop(void);
extern int cpu_core_loop_active;
extern void (*cpu_core_loop) (void);
extern char *cpu_core_loop_name;

extern void breakpoints_enable(void);
extern void breakpoints_disable(void);

//#define CPU_CORE_SPECTRUM 1
//#define CPU_CORE_ZX8081 2


#define VERBOSE_ERR   0
#define VERBOSE_WARN  1
#define VERBOSE_INFO  2
#define VERBOSE_DEBUG  3
#define VERBOSE_PARANOID  4

#define VERBOSE_MESSAGE_ERR "Error: "
#define VERBOSE_MESSAGE_WARN "Warning: "
#define VERBOSE_MESSAGE_INFO "Info: "
#define VERBOSE_MESSAGE_DEBUG "Debug: "
#define VERBOSE_MESSAGE_PARANOID "Paranoid: "

#define DEBUG_MAX_MESSAGE_LENGTH 1024


extern void machine_emulate_memory_refresh_debug_counter(void);

extern z80_bit cpu_transaction_log_enabled;
extern void set_cpu_core_transaction_log(void);
extern void reset_cpu_core_transaction_log(void);
extern char transaction_log_filename[];

extern z80_bit cpu_transaction_log_store_datetime;
extern z80_bit cpu_transaction_log_store_address;
extern z80_bit cpu_transaction_log_store_tstates;
extern z80_bit cpu_transaction_log_store_opcode;
extern z80_bit cpu_transaction_log_store_registers;

extern char *spectrum_rom_tokens[];
extern char *zx81_rom_tokens[];
extern char *zx80_rom_tokens[];

extern int debug_breakpoint_condition_loop(char *texto,int debug);

extern char debug_watches_text_to_watch[];

extern void debug_watches_loop(char *texto,char *texto_destino);

extern char debug_watches_texto_destino[];

extern z80_byte debug_watches_y_position;

extern void debug_get_t_stados_parcial_post(void);
extern void debug_get_t_stados_parcial_pre(void);

//extern z80_bit debug_core_evitamos_inter;
extern z80_bit debug_core_lanzado_inter;
extern z80_int debug_core_lanzado_inter_retorno_pc_nmi;
extern z80_int debug_core_lanzado_inter_retorno_pc_maskable;

extern void debug_anota_retorno_step_nmi(void);
extern void debug_anota_retorno_step_maskable(void);


//Maximo elementos en una lista
#define MAX_DEBUG_NESTED_ELEMENTS 1000

#define MAX_DEBUG_FUNCTION_NAME 255

typedef z80_byte (*debug_nested_function)(z80_int dir, z80_byte value);

struct s_debug_nested_function_element {
        //texto de la funcion. Maximo+el 0 del final
	char function_name[MAX_DEBUG_FUNCTION_NAME+1];

	//identificador (unico) de la funcion. Lo asigna al agregarlo, y se usa al borrar
	int id;

	//funcion asignada
	//z80_byte (*funcion)(z80_int dir, z80_byte value);
	union {
		void *funcptr;
		debug_nested_function funcion;
	};

	//puntero a siguiente elemento en la lista. NULL si no hay mas
	struct s_debug_nested_function_element *next;

	//puntero a elemento anterior en la lista. NULL si es el primero
	struct s_debug_nested_function_element *previous;

};

typedef struct s_debug_nested_function_element debug_nested_function_element;

extern int debug_nested_core_add(debug_nested_function funcion,char *nombre);
extern void debug_nested_core_del(int id);
extern void debug_nested_core_call_previous(int id);

extern void debug_nested_cores_pokepeek_init(void);


extern int debug_nested_poke_byte_add(debug_nested_function funcion,char *nombre);
extern void debug_nested_poke_byte_del(int id);
extern void debug_nested_poke_byte_call_previous(int id,z80_int dir,z80_byte value);
extern int debug_nested_poke_byte_no_time_add(debug_nested_function funcion,char *nombre);
extern void debug_nested_poke_byte_no_time_del(int id);
extern void debug_nested_poke_byte_no_time_call_previous(int id,z80_int dir,z80_byte value);

extern int debug_nested_peek_byte_add(debug_nested_function funcion,char *nombre);
extern void debug_nested_peek_byte_del(int id);
extern z80_byte debug_nested_peek_byte_call_previous(int id,z80_int dir);
extern int debug_nested_peek_byte_no_time_add(debug_nested_function funcion,char *nombre);
extern void debug_nested_peek_byte_no_time_del(int id);
extern z80_byte debug_nested_peek_byte_no_time_call_previous(int id,z80_int dir);


extern debug_nested_function_element *debug_nested_find_id(debug_nested_function_element *e,int id);

extern debug_nested_function_element *debug_nested_find_function_name(debug_nested_function_element *e,char *nombre);


extern void poke_byte_nested_handler(z80_int dir,z80_byte value);
extern void cpu_core_loop_nested_handler(void);

extern debug_nested_function_element *nested_list_poke_byte;
extern debug_nested_function_element *nested_list_core;

extern void debug_dump_nested_functions(char *result);

extern int debug_change_register(char *texto);

extern void debug_set_breakpoint(int breakpoint_index,char *condicion);

extern void debug_set_breakpoint_action(int breakpoint_index,char *accion);

extern void debug_view_basic(char *results_buffer);

extern void debug_get_ioports(char *stats_buffer);

extern void debug_run_action_breakpoint(char *comando);

extern int debug_if_breakpoint_action_menu(int index);

#define MAX_TEXT_DEBUG_GET_MEMORY_PAGES 31
//extern void debug_get_memory_pages(char *texto_final);

extern void debug_run_until_return_interrupt(void);

extern void debug_get_paging_screen_state(char *s);



#define MAX_DEBUG_MEMORY_SEGMENTS 8
struct s_debug_memory_segment {
        //texto largo del nombre del segmento
        char longname[100];

	//texto corto
	char shortname[32];

	//Primera direccion del segmento
	int start;

	//Longitud del segmento
	int length;


};

typedef struct s_debug_memory_segment debug_memory_segment;

extern int debug_get_memory_pages_extended(debug_memory_segment *segmentos);


extern char memory_zone_by_file_name[];
extern z80_byte *memory_zone_by_file_pointer;
extern int memory_zone_by_file_size;

extern int si_cpu_step_over_jpret(void);

extern int debug_get_opcode_length(unsigned int direccion);

extern void debug_cpu_step_over(void);

#endif
