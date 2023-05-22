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

#ifndef DIVIFACE_H
#define DIVIFACE_H

#include "cpu.h"

#define DIVIFACE_FIRMWARE_KB 8
#define DIVIFACE_FIRMWARE_ALLOCATED_KB 64
#define DIVIFACE_RAM_ALLOCATED_KB 512

extern z80_bit diviface_enabled;

extern void diviface_enable(char *romfile);
extern void diviface_disable(void);

//extern z80_bit diviface_paginacion_manual_activa;
extern z80_bit diviface_paginacion_automatica_activa;
extern z80_byte diviface_control_register;

extern void diviface_write_control_register(z80_byte value);

extern void diviface_set_peek_poke_functions(void);
extern void diviface_restore_peek_poke_functions(void);

extern void diviface_pre_opcode_fetch(void);
extern void diviface_post_opcode_fetch(void);

extern int get_diviface_total_ram(void);

extern int diviface_current_ram_memory_bits;

extern z80_bit diviface_eprom_write_jumper;

extern z80_byte *diviface_memory_pointer;
extern z80_byte *diviface_ram_memory_pointer;

extern z80_bit diviface_allow_automatic_paging;

extern z80_byte diviface_read_control_register(void);

extern void diviface_reset(void);

extern int diviface_nested_id_peek_byte;
extern int diviface_nested_id_peek_byte_no_time;
extern int diviface_nested_id_poke_byte;
extern int diviface_nested_id_poke_byte_no_time;

extern int diviface_conmem_enabled(void);
extern int diviface_mapram_enabled(void);
extern int get_diviface_ram_mask(void);


#endif
