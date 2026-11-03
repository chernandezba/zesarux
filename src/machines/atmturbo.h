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

#ifndef ATMTURBO_H
#define ATMTURBO_H

#include "cpu.h"


#define ATM_TURBO_ROM_SIZE 64
#define ATM_TURBO_RAM_SIZE 512
#define ATM_TURBO_TOTAL_RAM_PAGES (ATM_TURBO_RAM_SIZE/16)
#define ATM_TURBO_TOTAL_ROM_PAGES (ATM_TURBO_ROM_SIZE/16)

#define ATM_TURBO_DISPLAY_WIDTH 640
#define ATM_TURBO_DISPLAY_HEIGHT 400



#define ATM_TURBO_LEFT_BORDER_NO_ZOOM 40
#define ATM_TURBO_RIGHT_BORDER_NO_ZOOM 40
#define ATM_TURBO_TOP_BORDER_NO_ZOOM 88
#define ATM_TURBO_BOTTOM_BORDER_NO_ZOOM 88

#define ATM_TURBO_LEFT_BORDER ATM_TURBO_LEFT_BORDER_NO_ZOOM*zoom_x
#define ATM_TURBO_RIGHT_BORDER ATM_TURBO_RIGHT_BORDER_NO_ZOOM*zoom_x
#define ATM_TURBO_TOP_BORDER ATM_TURBO_TOP_BORDER_NO_ZOOM*zoom_y
#define ATM_TURBO_BOTTOM_BORDER ATM_TURBO_BOTTOM_BORDER_NO_ZOOM*zoom_y

extern z80_byte *atmturbo_ram_mem_table[];

extern z80_byte *atmturbo_rom_mem_table[];


extern void poke_byte_atmturbo(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_atmturbo(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_atmturbo(z80_int dir);
extern z80_byte peek_byte_no_time_atmturbo(z80_int dir);
extern z80_byte fetch_opcode_atmturbo(void);

extern void atmturbo_reset(void);

extern void atmturbo_malloc_mem_machine(void);

extern z80_byte atmturbo_read_port_fb(void);
extern z80_byte atmturbo_read_port_7b(void);

extern void atmturbo_out_port_32765(z80_byte value);
extern void atmturbo_out_port_fdfd(z80_byte value);
extern void atmturbo_out_port_fe(z80_int port,z80_byte value);
extern z80_byte atmturbo_get_border_colour(void);
extern z80_byte atmturbo_read_port_fe(z80_byte value);
extern z80_byte atmturbo2_read_port_fe(z80_byte value);
extern void atmturbo2_update_timing(void);
extern void atmturbo_out_port_7dfd(z80_byte value);
extern int atmturbo2_shadow_ports_enabled(void);
extern void atmturbo2_out_port_77(z80_int port,z80_byte value);
extern void atmturbo2_out_port_f7(z80_int port,z80_byte value);
extern void atmturbo2_out_port_fe(z80_int port,z80_byte value);
extern void atmturbo2_out_port_ff(z80_byte value);
extern int atmturbo2_palette_port_enabled(void);
extern int atmturbo_trdos_is_active(void);
extern int atmturbo_fdc_is_active(z80_int port,int reading);
extern z80_byte atmturbo_fdc_read_port(z80_int port);
extern void atmturbo_fdc_write_port(z80_int port,z80_byte value);

extern void atmturbo_refresca_pantalla(void);
extern void atmturbo_store_scanline_rainbow_border(void);
extern void atmturbo_store_scanline_rainbow_display(void);

#endif
