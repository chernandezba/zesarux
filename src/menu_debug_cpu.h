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
extern int menu_debug_hexdump_change_pointer(int p);
extern void menu_debug_change_memory_zone(void);
extern int menu_debug_get_total_digits_hexa(int valor);
extern int menu_get_current_memory_zone_name_number(char *s);


extern int map_adventure_offset_x;
extern int map_adventure_offset_y;

#endif