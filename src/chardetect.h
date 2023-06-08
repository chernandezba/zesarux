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

#ifndef CHARDETECT_H
#define CHARDETECT_H

#include "cpu.h"

#define TRAP_CHAR_DETECTION_ROUTINES_TOTAL 10

#define TRAP_CHAR_DETECTION_ROUTINE_NONE 0
#define TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC 1
#define TRAP_CHAR_DETECTION_ROUTINE_AD42 2
#define TRAP_CHAR_DETECTION_ROUTINE_COMMON_ONE 3
#define TRAP_CHAR_DETECTION_ROUTINE_COMMON_TWO 4
#define TRAP_CHAR_DETECTION_ROUTINE_COMMON_THREE 5
#define TRAP_CHAR_DETECTION_ROUTINE_COMMON_FOUR 6
#define TRAP_CHAR_DETECTION_ROUTINE_COMMON_FIVE 7
#define TRAP_CHAR_DETECTION_ROUTINE_COMMON_SIX 8

//de multiplicacion por ocho
#define TRAP_CHAR_DETECTION_ROUTINE_MULTIPLY_EIGHT 9

#define MAX_STDOUT_SUM32_COUNTER 50


extern int trap_char_detection_routine_number;

extern void chardetect_detect_char(void);

extern z80_int chardetect_second_trap_char_dir;
extern z80_int chardetect_third_trap_char_dir;
extern z80_bit chardetect_second_trap_sum32;
extern z80_byte chardetect_char_filter;
#define CHAR_FILTER_TOTAL 5
#define CHAR_FILTER_NONE 0
#define CHAR_FILTER_GENERIC 1
#define CHAR_FILTER_AD42 2
#define CHAR_FILTER_PAWS 3
#define CHAR_FILTER_HOBBIT 4
extern char *chardetect_char_filter_names[];

extern void charfilter_print_list(void);
extern int charfilter_set(char *s);


//calculo de cuantas escrituras minimas: 8 (de un caracter) * 32 (de  una linea) * 10 (10 lineas)
#define MAX_DEBUG_POKE_DISPLAY 8*32*10

extern z80_bit chardetect_line_width_wait_space;
extern z80_bit chardetect_line_width_wait_dot;
extern z80_bit chardetect_ignore_newline;

extern int chardetect_second_trap_sum32_counter;

extern int chardetect_second_trap_detect_pc_min;
extern int chardetect_second_trap_detect_pc_max;

extern void chardetect_init_automatic_char_detection(void);
extern char *trap_char_detection_routines_texto[];

extern z80_byte chardetect_line_width;
extern int chardetect_x_position;

extern z80_bit chardetect_detect_char_enabled;

extern z80_bit chardetect_printchar_enabled;

extern z80_bit chardetect_rom_compat_numbers;

extern void chardetect_printchar(void);

extern void chardetect_end_automatic_char_detection(void);

extern void chardetect_init_trap_detection_routine(void);

extern void chardetect_debug_char_table_routines_poke(z80_int dir);

//extern void (*original_char_detect_poke_byte)(z80_int dir,z80_byte valor);

extern int chardetect_automatic_nested_id_poke_byte;




#endif
