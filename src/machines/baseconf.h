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

#ifndef BASECONF_H
#define BASECONF_H

#include "cpu.h"


#define BASECONF_LEFT_BORDER_NO_ZOOM 0
#define BASECONF_TOP_BORDER_NO_ZOOM 0

#define BASECONF_LEFT_BORDER BASECONF_LEFT_BORDER_NO_ZOOM*zoom_x
#define BASECONF_TOP_BORDER BASECONF_TOP_BORDER_NO_ZOOM*zoom_y

#define BASECONF_DISPLAY_WIDTH 720
#define BASECONF_DISPLAY_HEIGHT 576

#define BASECONF_FMAPS_SIZE 1024
#define BASECONF_ROM_PAGES 32
#define BASECONF_RAM_PAGES 256

//#define BASECONF_MAX_SPRITES 85

//extern z80_byte baseconf_last_port_eff7;
//extern z80_byte baseconf_last_port_dff7;
//extern z80_byte baseconf_nvram[];

//extern void baseconf_write_af_port(z80_byte puerto_h,z80_byte value);
//extern z80_byte baseconf_get_af_port(z80_byte index);
extern void baseconf_init_memory_tables(void);
extern void baseconf_set_memory_pages(void);
extern z80_byte *baseconf_memory_paged[];
extern z80_byte *baseconf_rom_mem_table[];
extern z80_byte *baseconf_ram_mem_table[];
//extern z80_byte baseconf_af_ports[];

//extern z80_byte baseconf_fmaps[];

//extern int temp_baseconf_in_system_rom_flag;

extern void baseconf_reset_cpu(void);
extern void baseconf_hard_reset(void);

extern void lee_byte_evo_aux(z80_int direccion);

extern z80_byte baseconf_memory_segments[];
extern z80_byte baseconf_memory_segments_type[];

extern void baseconf_out_port(z80_int puerto,z80_byte valor);

extern int baseconf_sd_enabled;

extern int baseconf_sd_cs;

extern z80_byte baseconf_last_port_bf;

extern int baseconf_shadow_ports_available(void);






#endif
