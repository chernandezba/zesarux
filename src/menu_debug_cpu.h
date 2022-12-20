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

#ifndef MENU_DEBUG_CPU_H
#define MENU_DEBUG_CPU_H

#include "cpu.h"
#include "zxvision.h"

#define MAX_LENGTH_ADDRESS_MEMORY_ZONE 6

extern void menu_debug_registers(MENU_ITEM_PARAMETERS);
extern void menu_debug_registers_view_adventure(MENU_ITEM_PARAMETERS);
extern void menu_watches(MENU_ITEM_PARAMETERS);
extern void menu_debug_registers_run_cpu_opcode(void);
extern void menu_debug_textadventure_map_connections(MENU_ITEM_PARAMETERS);

extern void menu_debug_dissassemble_una_instruccion(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode);
extern void menu_debug_dissassemble_una_inst_sino_hexa(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode,int sino_hexa,int full_hexa_dump_motorola);
extern menu_z80_moto_int menu_debug_disassemble_last_ptr;
extern menu_z80_moto_int menu_debug_disassemble_subir(menu_z80_moto_int dir_inicial);
extern menu_z80_moto_int menu_debug_disassemble_bajar(menu_z80_moto_int dir_inicial);

extern int menu_debug_hexdump_with_ascii_modo_ascii;
extern z80_byte menu_debug_get_mapped_byte(int direccion);
extern void menu_debug_write_mapped_byte(int direccion,z80_byte valor);
extern void menu_debug_set_memory_zone_mapped(void);
extern void menu_debug_set_memory_zone_attr(void);
extern menu_z80_moto_int menu_debug_hexdump_adjusta_en_negativo(menu_z80_moto_int dir,int linesize);
extern int menu_debug_memory_zone;
extern menu_z80_moto_int menu_debug_memory_zone_size;
extern void menu_debug_print_address_memory_zone(char *texto, menu_z80_moto_int address);
extern void menu_debug_set_memory_zone(int zone);
extern int menu_debug_show_memory_zones;
extern menu_z80_moto_int adjust_address_memory_size(menu_z80_moto_int direccion);
//extern menu_z80_moto_int menu_debug_hexdump_change_pointer(menu_z80_moto_int p);
extern int menu_debug_cpu_calculate_expression(char *string_address,menu_z80_moto_int *output_value);
extern void menu_debug_change_memory_zone(void);
extern int menu_debug_get_total_digits_hexa(int valor);
extern int menu_get_current_memory_zone_name_number(char *s);


extern int map_adventure_offset_x;
extern int map_adventure_offset_y;

//"[VARIABLE][VOP][CONDITION][VALUE] [OPERATOR] [VARIABLE][VOP][CONDITION][VALUE] [OPERATOR] .... where: \n" 

#define HELP_MESSAGE_CONDITION_BREAKPOINT \
"A condition breakpoint evaluates an expression and the breakpoint will be fired if the expression is not 0.\n" \
"An expression (or just 'e' to shorten it) has the following syntax:" \
"[VALUE][LOGICOPERATOR]  [VALUE][LOGICOPERATOR] ... where: \n" \
"[VALUE] can be a combination of VARIABLE, a FUNCTION, a NUMERICVALUE or OPERATOR \n" \
"You can use parenthesis to prioritize some values over others, you can use any of these three: [{( to open parenthesis, and: )}] to close parenthesis\n" \
"\n" \
"[VARIABLE] can be a CPU register or some pseudo variables: A,B,C,D,E,F,H,L,AF,BC,DE,HL,A',B',C',D',E',F',H',L',AF',BC',DE',HL',I,R,SP,PC,IX,IY," \
"D0,D1,D2,D3,D4,D5,D6,D7,A0,A1,A2,A3,A4,A5,A6,A7,AC,ER,SR,P1,P2,P3\n" \
"FS,FZ,FP,FV,FH,FN,FC: Flags\n" \
"IFF1, IFF2: Interrupt bits,\n" \
"COPPERPC: returns the Copper PC register from TBBlue,\n" \
"OPCODE1: returns the byte at address PC, so the byte of the opcode being read,\n" \
"OPCODE2: returns the word at address PC, MSB order,\n" \
"OPCODE3: returns the three byte at adress PC, MSB order,\n" \
"OPCODE4: returns the four bytes at adress PC, MSB order,\n" \
"RAM: RAM mapped on 49152-65535 on Spectrum 128 or Prism,\n" \
"ROM: ROM mapped on 0-16383 on Spectrum 128,\n" \
"SEG0, SEG1, SEG2, SEG3: memory banks mapped on each 4 memory segments on Z88\n" \
"SEG0, SEG1, ...., SEG7: memory banks mapped on each 8 memory segments on TBBlue\n" \
"HILOWMAPPED: returns 1 if HiLow ROM & RAM is mapped\n" \
"PD765PCN: current cylinder of PD765 floppy drive\n" \
"MRV: value returned on read memory operation\n" \
"MWV: value written on write memory operation\n" \
"MRA: address used on read memory operation\n" \
"MWA: address used on write memory operation\n" \
"PRV: value returned on read port operation\n" \
"PWV: value written on write port operation\n" \
"PRA: address used on read port operation\n" \
"PWA: address used on write port operation\n" \
"OUTFIRED: returns 1 if last Z80 opcode was an OUT operation\n" \
"INFIRED: returns 1 if last Z80 opcode was an IN operation\n" \
"INTFIRED: returns 1 when an interrupt has been generated\n" \
"ENTERROM: returns 1 the first time PC register is on ROM space (0-16383)\n" \
"EXITROM: returns 1 the first time PC register is out ROM space (16384-65535)\n" \
"Note: The last two only return 1 the first time the breakpoint is fired, or a watch is shown, " \
"it will return 1 again only exiting required space address and entering again\n" \
"TSTATES: t-states total in a frame\n" \
"TSTATESL: t-states in a scanline\n" \
"TSTATESP: t-states partial\n" \
"SCANLINE: scanline counter\n" \
"\n" \
"[FUNCTION] can be:\n" \
"PEEK(e): returns the byte at address e, where e is any expression. Address e is in the range of the visible cpu address space\n" \
"PEEKW(e): returns the word at address e\n" \
"FPEEK(e): returns the byte at far address e, where e is any expression. Address e is in the range of the total SRAM machine address space, "\
"useful for example on Spectrum Next, ZX-Uno or machines with 128kb RAM. On QL and machines machines with 48kb ram or less, FPEEK works the " \
"same as PEEK\n" \
"IN(e): returns the byte at port e\n" \
"NOT(e): negates expression e: if it's 0, returns 1. Otherwhise, return 0\n" \
"ABS(e): returns absolute value of expression e\n" \
"BYTE(e): same as (e)&FFH\n" \
"WORD(e): same as (e)&FFFFH\n" \
"\n" \
"[NUMERICVALUE] must be a numeric value, it can have a suffix indicating the numeric base, otherwise it's decimal:\n" \
"suffix H: hexadecimal\n" \
"suffix %: binary\n" \
"between quotes '' or \"\": ascii\n" \
"\n" \
"[OPERATOR] must be one of the following: =, <, > , <>, <=, >=, +, -, *, / , \n" \
"& : bitwise and\n" \
"| : bitwise or\n" \
"^ : bitwise xor\n" \
"\n" \
"[LOGICOPERATOR] must be one of the following: and, or, xor\n" \
"\n" \
"Examples of conditions:\n" \
"A: it will match when A register is not zero\n" \
"SP<32768 : it will match when SP register is below 32768\n" \
"PWA & FFH=FEH : it will match when last port write address, doing an AND bitwise (&) with FFH, is equal to FEH\n" \
"A|1=255 : it will match when register A, doing OR bitwise (|), it equal to 255\n" \
"PEEK(32768)&0FH=3 : it will match when memory address 32768 has the lower 4 bits set to value 3\n" \
"OUTFIRED=1 AND PWA&00FFH=FEH AND PWV&7=1 : it will match when changing border color to blue\n" \
"HL=DE : it will mach when HL is equal to DE register\n" \
"32768>PC : it will match when PC<32768\n" \
"1=1 : it will match when 1=1, so always ;) \n" \
"FS=1: it will match when flag S is set\n" \
"A=10 and BC<33 : it will match when A register is 10 and BC is below 33\n" \
"A+1=10 : it will match when A+1 equals to 10\n" \
"BC=(DE+HL)*2: it will match when BC register is (DE+HL)*2\n" \
"OPCODE2=ED4AH : it will match when running opcode ADC HL,BC\n" \
"OPCODE1=21H : it will match when running opcode LD HL,NN\n" \
"OPCODE3=210040H : it will match when running opcode LD HL,4000H\n" \
"SEG2=40H : when memory bank 40H is mapped to memory segment 2 (49152-65535 range) on Z88\n" \
"MWA<16384 : it will match when attempting to write in ROM\n" \
"ENTERROM=1 : it will match when entering ROM space address\n" \
"TSTATESP>69888 : it will match when partial counter has executed a 48k full video frame (you should reset it before)\n" \
"\nNote 1: Any condition in the whole list can trigger a breakpoint" \
"\nNote 2: It you are using the substract operator (-) and using 3 or more values, you should use parenthesis. So an expression like:\n" \
"A-B-C will be calculated wrong. You should write it as:\n" \
"(A-B)-C . The reason for that is that the expression parser uses a fast approach to calculate expressions recursively, from left to right, and it " \
"does not prioritize some operators, like the substract\n" \
"\nNote 3: Breakpoint types PC=XXXX, MWA=XXXX and MRA=XXXX are a lot faster than the rest, because they use a breakpoint optimizer"


#define HELP_MESSAGE_BREAKPOINT_ACTION \
"Action can be one of the following: \n\n" \
"menu or break or empty string: Breaks current execution of program\n" \
"call address: Calls memory address, address is an expression\n" \
"disassemble address: Dissassemble address, address is an expression\n" \
"printc expression: Print ascii character to console\n" \
"printe expression: Print expression following the same syntax as breakpoints and evaluate expression\n" \
"printregs: Print registers\n" \
"prints string: Prints string to console\n" \
"putv expression: Adds expression result value in the Debug Memory Zone. Result is always treated as a 8-bit value. Zone is cleared when running Reset\n" \
"quicksave: Saves a quick snapshot\n" \
"reset-tstatp: Resets t-states partial counter\n" \
"set-register expression: Sets register indicated on expression. Example: set-register PC=BC+10\n" \
"start-transaction-log: Starts the transaction log, require that you define the log file before\n" \
"stop-transaction-log: Stops the transaction log\n" \
"write address value: Write memory address with indicated value, address and value are expressions\n" \


#endif
