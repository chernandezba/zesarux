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

#ifndef PCW_H
#define PCW_H


#include "cpu.h"


//Hacer que estos valores de border sean multiples de 8
//TODO: no tengo claro del ancho del border pero le pongo suficiente para que quepa el logo de ZEsarUX a la izquierda
#define PCW_LEFT_BORDER_NO_ZOOM 32


#define PCW_MAX_RAM_PAGES 128

#define PCW_TOP_BORDER_NO_ZOOM 24



#define PCW_DISPLAY_WIDTH 720

//Doblamos pixeles *2 en alto
#define PCW_DISPLAY_HEIGHT 512

//#define PCW_TOTAL_SCANLINES ((PCW_DISPLAY_HEIGHT+PCW_TOP_BORDER_NO_ZOOM*2)/2)

#define PCW_LEFT_BORDER PCW_LEFT_BORDER_NO_ZOOM*zoom_x
#define PCW_TOP_BORDER  PCW_TOP_BORDER_NO_ZOOM*zoom_y

//Para poder hacer debug_printf con la clase PCW adecuada
#define DBG_PRINT_PCW debug_printf(VERBOSE_CLASS_PCW|

extern z80_byte *pcw_get_memory_offset_read(z80_int dir);
extern z80_byte *pcw_get_memory_offset_write(z80_int dir);
extern void pcw_reset(void);
extern void pcw_init_memory_tables(void);
extern void pcw_set_memory_pages(void);
extern void pcw_out_port_bank(z80_byte puerto_l,z80_byte value);
extern void pcw_out_port_f4(z80_byte value);
extern void pcw_out_port_f5(z80_byte value);
extern void pcw_out_port_f6(z80_byte value);
extern void pcw_out_port_f7(z80_byte value);
extern void pcw_out_port_f8(z80_byte value);
extern z80_byte pcw_get_port_f8_value(void);

extern z80_byte pcw_in_port_f4(void);
extern z80_byte pcw_in_port_f8(void);
extern z80_byte pcw_in_port_fd(void);
extern z80_byte pcw_in_port_9f(void);
extern z80_byte pcw_in_port_e0(void);
extern z80_byte pcw_in_port_dktronics_joystick(void);

extern void pcw_increment_interrupt_counter(void);
extern void pcw_keyboard_ticker_update(void);

extern z80_byte *pcw_ram_mem_table[];

extern int pcw_total_ram;

extern z80_byte pcw_banks_paged_read[];
extern z80_byte pcw_read_keyboard(z80_int dir);

extern z80_byte pcw_keyboard_table[];

extern z80_byte pcw_scanline_counter;

extern z80_bit pcw_pending_interrupt;

extern void pcw_interrupt_from_pd765(void);

extern void scr_refresca_pantalla_y_border_pcw(void);

extern z80_byte pcw_bank_registers[];
extern z80_byte pcw_port_f4_value;
extern z80_byte pcw_port_f5_value;
extern z80_byte pcw_port_f6_value;
extern z80_byte pcw_port_f7_value;
extern z80_byte pcw_port_f8_value;
extern z80_byte pcw_interrupt_counter;

extern int pcw_rgb_table[];
extern z80_bit pcw_black_white_display;
extern z80_bit pcw_always_on_display;
extern z80_bit pcw_do_not_inverse_display;
extern z80_bit pcw_do_not_scroll;

extern void pcw_boot_locoscript(void);
extern void pcw_boot_cpm(void);
extern z80_bit pcw_boot_reinsert_previous_dsk;
extern int pcw_boot_timer;
extern z80_bit pcw_failback_cpm_when_no_boot;
extern void pcw_handle_end_boot_disk(void);
extern void pcw_boot_check_dsk_not_bootable(void);
extern void pcw_boot_timer_handle(void);

extern int pcw_video_mode;
extern char *pcw_video_mode_names[];

#endif
