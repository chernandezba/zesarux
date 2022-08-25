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

#ifndef ULA_H
#define ULA_H

#include "cpu.h"
#include "utils.h"

extern z80_byte out_254_original_value;
extern z80_byte out_254;
extern z80_bit keyboard_issue2;

extern z80_byte ula_databus_value;

extern z80_byte last_ula_attribute;
extern z80_byte last_ula_pixel;

extern z80_byte contador_parpadeo;
extern z80_bit estado_parpadeo;
extern z80_bit disable_change_flash;


extern z80_bit ula_late_timings;

extern z80_bit pentagon_timing;

extern z80_byte puerto_eff7;

extern void ula_disable_pentagon_timing(void);
extern void ula_enable_pentagon_timing(void);

extern z80_bit ula_disabled_ram_paging;
extern z80_bit ula_disabled_rom_paging;

extern z80_bit ula_im2_slow;

extern void ula_enable_pentagon_timing_no_common(void);




//Usamos puertos que estaban reservados para Spectrastik pero ese dispositivo no existe ya
#define ZESARUX_ZXI_PORT_REGISTER 0xCF3B
#define ZESARUX_ZXI_PORT_DATA     0xDF3B

#define ZESARUX_ZXI_ZX8081_PORT_REGISTER 0x35
#define ZESARUX_ZXI_ZX8081_PORT_DATA     0x53

extern z80_byte zesarux_zxi_last_register;

extern z80_byte zesarux_zxi_registers_array[];



extern z80_byte zesarux_zxi_read_last_register(void);
extern void zesarux_zxi_write_last_register(z80_byte value);
extern void zesarux_zxi_write_register_value(z80_byte value);
extern z80_byte zesarux_zxi_read_register_value(void);

extern void generate_nmi(void);
extern void generate_nmi_multiface_tbblue(void);


extern z80_bit keyboard_matrix_error;


extern void recreated_zx_spectrum_keyboard_convert(int tecla, enum util_teclas *tecla_final, int *pressrelease);
extern z80_bit recreated_zx_keyboard_support;
extern z80_bit recreated_zx_keyboard_pressed_caps;

extern void nmi_handle_pending_prepost_fetch(void);
extern int nmi_pending_pre_opcode;
extern int nmi_pending_post_opcode;
extern void generate_nmi_prepare_fetch(void);

extern z80_bit dinamic_sd1;

#endif
