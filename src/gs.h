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

#ifndef GS_H
#define GS_H

#include "cpu.h"


//Tamanyos en KB de la rom y ram
#define GS_ROM_SIZE 32
#define GS_ROM_BLOCKS (GS_ROM_SIZE/16)
#define GS_RAM_SIZE 1024
#define GS_RAM_BLOCKS (GS_RAM_SIZE/16)

#define GS_ROM_NAME "gs105a.rom"

//Estados totales de cpu por scanline
#define GS_MAX_STATES_LINE 750

//Cada cuantos estados de esos lanzar una interrupcion
#define GS_INTERRUPTS_STATES 334


extern z80_bit gs_enabled;
extern void gs_enable(void);
extern void gs_disable(void);
extern void gs_fetch_opcodes_scanlines(void);
extern void gs_new_video_frame(void);

extern z80_byte gs_memory_mapping_mask_pages;

struct gs_machine_state {

    z80_int r_pc,r_sp;

    z80_int r_bc,r_de,r_hl,r_af;
    z80_int r_bc_shadow,r_de_shadow,r_hl_shadow,r_af_shadow;


    z80_int r_ix,r_iy;

//header[20]=(reg_r&127) | (reg_r_bit7&128);

    z80_int r_ir;

    z80_bit iff1,iff2;
    z80_byte im_mode;

    int t_estados;

    z80_bit z80_halt_signal;

    void (*poke_byte)(z80_int dir,z80_byte valor);
    void (*poke_byte_no_time)(z80_int dir,z80_byte valor);
    z80_byte (*peek_byte)(z80_int dir);
    z80_byte (*peek_byte_no_time)(z80_int dir);    
    z80_byte (*lee_puerto)(z80_byte puerto_h,z80_byte puerto_l);
    void (*out_port)(z80_int puerto,z80_byte value);
    z80_byte (*fetch_opcode)(void);
    void (*contend_read)(z80_int direccion,int time);
    void (*contend_read_no_mreq)(z80_int direccion,int time);
    void (*contend_write_no_mreq)(z80_int direccion,int time);

    //En teoria este no haria falta
    z80_byte *memoria_spectrum;
};

extern void gs_write_port_bb_from_speccy(z80_byte value);
extern void gs_write_port_b3_from_speccy(z80_byte value);
extern z80_byte gs_read_port_bb_from_speccy(void);
extern z80_byte gs_read_port_b3_from_speccy(void);
extern void gs_mix_dac_channels(void);

extern z80_byte gs_command_register;
extern z80_byte gs_data_register;
extern z80_byte gs_state_register;

extern z80_byte gs_output_register;

extern z80_byte gs_dac_channels[];
extern z80_byte gs_volumes[];

extern z80_byte gs_memory_mapping_value;

extern z80_bit gs_stereo_mode;

extern z80_byte gs_dac_valor_final_left;
extern z80_byte gs_dac_valor_final_right;



//Estado de la Z80 del general sound
extern struct gs_machine_state general_sound_z80_cpu;


#endif
