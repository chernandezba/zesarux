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

#ifndef OPERACIONES_H
#define OPERACIONES_H

#include "compileoptions.h"

#include "cpu.h"
#include "hilow.h"

#ifdef EMULATE_MEMPTR
        #define set_memptr(value) memptr=value
#else
        #define set_memptr(value)
#endif


#ifdef EMULATE_VISUALMEM

#define VISUALMEM_MMC_BUFFER_SIZE (1024*1024)
#define VISUALMEM_HILOW_BUFFER_SIZE (HILOW_DEVICE_SIZE)


extern z80_byte *visualmem_buffer;
extern z80_byte *visualmem_read_buffer;
extern z80_byte *visualmem_opcode_buffer;

extern void init_visualmembuffer(void);
extern void set_visualmembuffer(int dir);
extern void clear_visualmembuffer(int dir);
extern int get_visualmem_size(void);

extern void set_visualmemopcodebuffer(int dir);
extern void clear_visualmemopcodebuffer(int dir);

extern void set_visualmemreadbuffer(int dir);
extern void clear_visualmemreadbuffer(int dir);

extern z80_byte *visualmem_mmc_read_buffer;
extern void set_visualmemmmc_read_buffer(int dir);
extern void clear_visualmemmmc_read_buffer(int dir);

extern z80_byte *visualmem_mmc_write_buffer;
extern void set_visualmemmmc_write_buffer(int dir);
extern void clear_visualmemmmc_write_buffer(int dir);

extern z80_byte *visualmem_hilow_read_buffer;
extern void set_visualmemhilow_read_buffer(int dir);
extern void clear_visualmemhilow_read_buffer(int dir);

extern z80_byte *visualmem_hilow_write_buffer;
extern void set_visualmemhilow_write_buffer(int dir);
extern void clear_visualmemhilow_write_buffer(int dir);


#endif


//solo para debug con printf
#include <stdio.h>

//solo para algun exit
#include <stdlib.h>

//extern void store_flags_shadow(z80_byte flags);
//extern void store_flags(z80_byte flags);
//extern void set_undocumented_flags_bits(z80_byte value);
//extern void set_undocumented_flags_bits_memptr(void);
extern void set_flags_overflow_inc(z80_byte antes,z80_byte result);
extern void set_flags_overflow_dec(z80_byte antes,z80_byte result);
extern void set_flags_overflow_resta(z80_byte antes,z80_byte result);

extern z80_byte sz53_table[];
extern z80_byte sz53p_table[];


#define set_flags_zero_sign_16(value)			\
{                                      			\
        Z80_FLAGS &=(255-FLAG_Z-FLAG_S); 		\
        if (!value) Z80_FLAGS |=FLAG_Z;			\
                                     			\
        if ( (value) & 32768 ) Z80_FLAGS |=FLAG_S;	\
}          						\


#define set_flags_zero_sign(value)		\
{						\
	Z80_FLAGS &=(255-FLAG_Z-FLAG_S);	\
	if (!value) Z80_FLAGS |=FLAG_Z;		\
						\
	Z80_FLAGS |=(value&FLAG_S);		\
}						\


#define set_flags_carry_16_suma(antes,result) \
{ \
        if (result<antes) Z80_FLAGS |=FLAG_C; \
        else Z80_FLAGS &=(255-FLAG_C); \
} \

#define set_flags_carry_suma(antes,result) \
{ \
        if (result<antes) Z80_FLAGS |=FLAG_C; \
        else Z80_FLAGS &=(255-FLAG_C); \
 \
        set_flags_halfcarry_suma(antes,result); \
} \

#define set_flags_carry_resta(antes,result) \
{ \
        if (result>antes) Z80_FLAGS |=FLAG_C; \
        else Z80_FLAGS &=(255-FLAG_C); \
 \
        set_flags_halfcarry_resta(antes,result); \
} \



#define jr_dis(desp) \
{ \
        z80_int desp16; \
        desp16=desp8_to_16(desp); \
        reg_pc +=desp16; \
} \



#define rst(dir)\
{\
        push_valor(reg_pc,PUSH_VALUE_TYPE_RST); \
        reg_pc=dir; \
} \


#define call()\
{\
  z80_byte calltempl, calltemph; \
  calltempl=peek_byte(reg_pc++);\
  calltemph=peek_byte(reg_pc); \
  contend_read_no_mreq(reg_pc, 1 );  \
  reg_pc++;\
  push_valor(reg_pc,PUSH_VALUE_TYPE_CALL); \
  reg_pc=value_8_to_16(calltemph,calltempl); \
  set_memptr(reg_pc); \
} \


#define JR()\
{\
  z80_byte jrtemp = peek_byte (reg_pc); \
  contend_read_no_mreq( reg_pc, 1 ); contend_read_no_mreq( reg_pc, 1 ); \
  contend_read_no_mreq( reg_pc, 1 ); contend_read_no_mreq( reg_pc, 1 ); \
  contend_read_no_mreq( reg_pc, 1 ); \
  jr_dis(jrtemp);\
} \


#define store_flags(flags) { \
	Z80_FLAGS=flags; \
} \

#define store_flags_shadow(flags) { \
	Z80_FLAGS_SHADOW=flags; \
} \

//#define get_flags() (   (Z80_FLAGS & FLAG_C) | (Z80_FLAGS & FLAG_N) | (Z80_FLAGS & FLAG_PV) | (Z80_FLAGS & FLAG_3) | (Z80_FLAGS & FLAG_H) | (Z80_FLAGS & FLAG_5) | (Z80_FLAGS & FLAG_Z) | (Z80_FLAGS & FLAG_S)   )

#define get_flags() ( Z80_FLAGS )
#define get_flags_shadow() ( Z80_FLAGS_SHADOW )


//Activa los flags 5,3 segun valor
#define set_undocumented_flags_bits(value) 						\
{											\
	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_3-FLAG_5)) | ( value & ( FLAG_3|FLAG_5 ) );	\
}											\

//Activa los flags 5,3 segun MEMPTR
#define set_undocumented_flags_bits_memptr(void)        					\
{												\
	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_3-FLAG_5)) | ( (memptr>>8) & ( FLAG_3|FLAG_5 ) );	\
}												\



extern void set_flags_halfcarry_suma(z80_byte antes,z80_byte result);
extern void set_flags_halfcarry_resta(z80_byte antes,z80_byte result);

extern void set_flags_parity(z80_byte value);


extern void init_cpu_tables(void);


//extern void set_flags_rld_rrd(z80_byte rh,z80_byte rl);
extern void set_flags_in_reg(z80_byte value);

extern void cpu_common_jump_im01(void);

extern z80_byte peek_byte_vacio(z80_int dir GCC_UNUSED);
extern void poke_byte_vacio(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED);
extern z80_byte lee_puerto_vacio(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l GCC_UNUSED);
extern void out_port_vacio(z80_int puerto GCC_UNUSED,z80_byte value GCC_UNUSED);
extern z80_byte fetch_opcode_vacio(void);



extern void (*poke_byte)(z80_int dir,z80_byte valor);
extern void (*poke_byte_no_time)(z80_int dir,z80_byte valor);
extern z80_byte (*peek_byte)(z80_int dir);
extern z80_byte (*peek_byte_no_time)(z80_int dir);

extern z80_byte (*fetch_opcode)(void);

extern z80_byte fetch_opcode_spectrum(void);
extern z80_byte fetch_opcode_zx81(void);
extern z80_byte fetch_opcode_ace(void);
extern z80_byte fetch_opcode_cpc(void);
extern z80_byte fetch_opcode_sam(void);
extern z80_byte fetch_opcode_msx(void);
extern z80_byte fetch_opcode_coleco(void);
extern z80_byte fetch_opcode_sg1000(void);
extern z80_byte fetch_opcode_sms(void);
extern z80_byte fetch_opcode_svi(void);

extern void poke_byte_spectrum_48k(z80_int dir,z80_byte valor);

extern z80_byte chardetect_automatic_poke_byte(z80_int dir,z80_byte valor);

extern z80_byte lee_puerto_spectrum_no_time(z80_byte puerto_h,z80_byte puerto_l);                    
extern z80_byte lee_puerto_z88_no_time(z80_byte puerto_h,z80_byte puerto_l);                            
extern z80_byte lee_puerto_ace_no_time(z80_byte puerto_h,z80_byte puerto_l);                 
extern z80_byte lee_puerto_cpc_no_time(z80_byte puerto_h,z80_byte puerto_l);           
extern z80_byte lee_puerto_sam_no_time(z80_byte puerto_h,z80_byte puerto_l);               
extern z80_byte lee_puerto_msx1_no_time(z80_byte puerto_h,z80_byte puerto_l);                 
extern z80_byte lee_puerto_coleco_no_time(z80_byte puerto_h,z80_byte puerto_l);                
extern z80_byte lee_puerto_sg1000_no_time(z80_byte puerto_h,z80_byte puerto_l);     
extern z80_byte lee_puerto_sms_no_time(z80_byte puerto_h,z80_byte puerto_l);           
extern z80_byte lee_puerto_svi_no_time(z80_byte puerto_h,z80_byte puerto_l);
                        

extern void poke_byte_spectrum_128k(z80_int dir,z80_byte valor);
extern void poke_byte_spectrum_128kp2a(z80_int dir,z80_byte valor);
extern void poke_byte_zxuno(z80_int dir,z80_byte valor);
extern void poke_byte_spectrum_inves(z80_int dir,z80_byte valor);
extern void poke_byte_spectrum_16k(z80_int dir,z80_byte valor);
extern void poke_byte_zx80(z80_int dir,z80_byte valor);
extern void poke_byte_ace(z80_int dir,z80_byte valor);
extern void poke_byte_chloe(z80_int dir,z80_byte valor);
extern void poke_byte_prism(z80_int dir,z80_byte valor);
extern void poke_byte_timex(z80_int dir,z80_byte valor);
extern void poke_byte_cpc(z80_int dir,z80_byte valor);
extern void poke_byte_sam(z80_int dir,z80_byte valor);
extern void poke_byte_tbblue(z80_int dir,z80_byte valor);
extern void poke_byte_msx1(z80_int dir,z80_byte valor);
extern void poke_byte_coleco(z80_int dir,z80_byte valor);
extern void poke_byte_sg1000(z80_int dir,z80_byte valor);
extern void poke_byte_sms(z80_int dir,z80_byte valor);
extern void poke_byte_svi(z80_int dir,z80_byte valor);

extern void poke_byte_no_time_spectrum_48k(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_spectrum_128k(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_spectrum_128kp2a(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_zxuno(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_spectrum_inves(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_spectrum_16k(z80_int dir,z80_byte valor);
extern void poke_byte_zx80_no_time(z80_int dir,z80_byte valor);
extern void poke_byte_ace_no_time(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_chloe(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_prism(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_timex(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_cpc(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_sam(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_tbblue(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_msx1(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_coleco(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_sg1000(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_sms(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_svi(z80_int dir,z80_byte valor);

extern void poke_byte_zx81(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_spectrum_48k(z80_int dir);
extern z80_byte peek_byte_spectrum_128k(z80_int dir);
extern z80_byte peek_byte_spectrum_128kp2a(z80_int dir);
extern z80_byte peek_byte_zxuno(z80_int dir);
extern z80_byte peek_byte_spectrum_16k(z80_int dir);
extern z80_byte peek_byte_spectrum_inves(z80_int dir);
extern z80_byte peek_byte_zx80(z80_int dir);
extern z80_byte peek_byte_zx81(z80_int dir);
extern z80_byte peek_byte_ace(z80_int dir);
extern z80_byte peek_byte_chloe(z80_int dir);
extern z80_byte peek_byte_prism(z80_int dir);
extern z80_byte peek_byte_timex(z80_int dir);
extern z80_byte peek_byte_cpc(z80_int dir);
extern z80_byte peek_byte_sam(z80_int dir);
extern z80_byte peek_byte_tbblue(z80_int dir);
extern z80_byte peek_byte_msx1(z80_int dir);
extern z80_byte peek_byte_coleco(z80_int dir);
extern z80_byte peek_byte_sg1000(z80_int dir);
extern z80_byte peek_byte_sms(z80_int dir);
extern z80_byte peek_byte_svi(z80_int dir);

extern z80_byte peek_byte_zx80_no_time(z80_int dir);
extern z80_byte peek_byte_no_time_spectrum_48k(z80_int dir);
extern z80_byte peek_byte_no_time_spectrum_128k(z80_int dir);
extern z80_byte peek_byte_no_time_spectrum_128kp2a(z80_int dir);
extern z80_byte peek_byte_no_time_zxuno(z80_int dir);
extern z80_byte peek_byte_no_time_spectrum_16k(z80_int dir);
extern z80_byte peek_byte_no_time_spectrum_inves(z80_int dir);
extern z80_byte peek_byte_ace_no_time(z80_int dir);
extern z80_byte peek_byte_no_time_chloe(z80_int dir);
extern z80_byte peek_byte_no_time_prism(z80_int dir);
extern z80_byte peek_byte_no_time_timex(z80_int dir);
extern z80_byte peek_byte_no_time_cpc(z80_int dir);
extern z80_byte peek_byte_no_time_sam(z80_int dir);
extern z80_byte peek_byte_no_time_tbblue(z80_int dir);
extern z80_byte peek_byte_no_time_msx1(z80_int dir);
extern z80_byte peek_byte_no_time_coleco(z80_int dir);
extern z80_byte peek_byte_no_time_sg1000(z80_int dir);
extern z80_byte peek_byte_no_time_sms(z80_int dir);
extern z80_byte peek_byte_no_time_svi(z80_int dir);


extern void poke_byte_chrome(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_chrome(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_chrome(z80_int dir);
extern z80_byte peek_byte_no_time_chrome(z80_int dir);

extern void poke_byte_tsconf(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_tsconf(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_tsconf(z80_int dir);
extern z80_byte peek_byte_no_time_tsconf(z80_int dir);

extern void poke_byte_baseconf(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_baseconf(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_baseconf(z80_int dir);
extern z80_byte peek_byte_no_time_baseconf(z80_int dir);

extern void poke_byte_mk14(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_mk14(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_mk14(z80_int dir);
extern z80_byte peek_byte_no_time_mk14(z80_int dir);
extern z80_byte lee_puerto_legacy_mk14(z80_byte h GCC_UNUSED,z80_byte l GCC_UNUSED);

extern void poke_word(z80_int dir,z80_int valor);
extern z80_int peek_word(z80_int dir);
extern z80_int peek_word_no_time(z80_int dir);


extern z80_int lee_word_pc(void);

#define lee_byte_pc() peek_byte(reg_pc++)

extern z80_byte peek_byte_desp(z80_int dir,z80_byte desp);
extern void poke_byte_desp(z80_int dir,z80_byte desp,z80_byte valor);


extern z80_int add_16bit(z80_int reg, z80_int value);
//extern z80_int inc_16bit(z80_int reg);
//extern z80_int dec_16bit(z80_int reg);

#define inc_8bit(reg)  \
{  \
	(reg)++; \
	Z80_FLAGS = ( Z80_FLAGS & FLAG_C ) | ( (reg)==128 ? FLAG_PV : 0 ) | ( (reg)&0x0f ? 0 : FLAG_H ) | sz53_table[reg]; \
} \


#define dec_8bit(reg) \
{ \
	Z80_FLAGS = ( Z80_FLAGS & FLAG_C ) | ( (reg)&0x0f ? 0 : FLAG_H ) | FLAG_N; \
	(reg)--; \
	Z80_FLAGS |= ( (reg)==127 ? FLAG_PV : 0 ) | sz53_table[reg]; \
} \



extern z80_int sbc_16bit(z80_int reg, z80_int value);
extern z80_byte sub_value(z80_byte value);

extern z80_int desp8_to_16(z80_byte desp);
extern void neg(void);


#define TOTAL_PUSH_VALUE_TYPES 6

enum push_value_type {
	PUSH_VALUE_TYPE_DEFAULT=0,
	PUSH_VALUE_TYPE_CALL,
	PUSH_VALUE_TYPE_RST,
	PUSH_VALUE_TYPE_PUSH,
	PUSH_VALUE_TYPE_MASKABLE_INTERRUPT,
        PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT
};

extern char *push_value_types_strings[];



extern void (*push_valor)(z80_int valor,z80_byte tipo); 

extern void push_valor_default(z80_int valor,z80_byte tipo) ;

extern z80_int pop_valor();


#define cp_reg(value) \
{ \
 \
        z80_byte result,antes; \
 \
        set_undocumented_flags_bits(value); \
 \
        result=reg_a; \
        antes=reg_a; \
 \
        result -=value; \
 \
        set_flags_zero_sign(result); \
        set_flags_carry_resta(antes,result); \
        set_flags_overflow_resta(antes,result); \
        Z80_FLAGS |=FLAG_N; \
 \
} \

#define and_reg(value) \
{ \
	reg_a &= value; \
	Z80_FLAGS=FLAG_H | sz53p_table[reg_a]; \
} \

#define or_reg(value) \
{ \
	reg_a |=value; \
	Z80_FLAGS=sz53p_table[reg_a]; \
} \

#define xor_reg(value) \
{ \
	reg_a ^=value; \
	Z80_FLAGS=sz53p_table[reg_a]; \
} \


extern z80_byte rlc_valor(z80_byte value);
extern void rlc_reg(z80_byte *reg);
extern void rlca(void);
extern z80_byte rl_valor(z80_byte value);
extern void rl_reg(z80_byte *reg);
extern void rla(void);
extern z80_byte rr_valor(z80_byte value);
extern void rr_reg(z80_byte *reg);
extern void rra(void);
extern z80_byte rrc_valor(z80_byte value);
extern void rrc_reg(z80_byte *reg);
extern void rrca(void);
extern z80_byte sla_valor(z80_byte value);
extern void sla_reg(z80_byte *reg);
extern z80_byte sra_valor(z80_byte value);
extern void sra_reg(z80_byte *reg);
extern z80_byte srl_valor(z80_byte value);
extern void srl_reg(z80_byte *reg);
extern z80_byte sls_valor(z80_byte value);
extern void sls_reg(z80_byte *reg);
extern void add_a_reg(z80_byte value);
extern void adc_a_reg(z80_byte value);
extern void sub_a_reg(z80_byte value);
extern void sbc_a_reg(z80_byte value);
extern z80_int adc_16bit(z80_int reg, z80_int value);

#define add_hl(value)			\
{					\
        HL=add_16bit(HL,value);		\
}					\

#define adc_hl(value)			\
{					\
        HL=adc_16bit(HL,value);		\
}					\

#define sbc_hl(value)			\
{					\
        HL=sbc_16bit(HL,value);		\
}					\



extern void rl_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void rr_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void rlc_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void rrc_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void sla_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void sra_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void srl_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void sls_ixiy_desp_reg(z80_byte desp,z80_byte *registro);
extern void res_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp, z80_byte *registro);
extern void set_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp, z80_byte *registro);
//extern void bit_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp, z80_byte *registro);
extern void bit_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp);
extern z80_byte *devuelve_reg_offset(z80_byte valor);

extern void rl_cb_reg(z80_byte *registro);
extern void rr_cb_reg(z80_byte *registro);
extern void rlc_cb_reg(z80_byte *registro);
extern void rrc_cb_reg(z80_byte *registro);
extern void sla_cb_reg(z80_byte *registro);
extern void sra_cb_reg(z80_byte *registro);
extern void srl_cb_reg(z80_byte *registro);
extern void sls_cb_reg(z80_byte *registro);
extern void res_bit_cb_reg(z80_byte numerobit, z80_byte *registro);
extern void set_bit_cb_reg(z80_byte numerobit, z80_byte *registro);
extern void bit_bit_cb_reg(z80_byte numerobit, z80_byte *registro);

extern z80_byte lee_puerto_spectrum_zx8081(z80_byte puerto_h,z80_byte puerto_l);
extern z80_byte lee_puerto_spectrum(z80_byte h,z80_byte l);
extern z80_byte lee_puerto_zx81(z80_byte h,z80_byte l);
extern z80_byte lee_puerto_zx80(z80_byte h,z80_byte l);
extern z80_byte lee_puerto_ace(z80_byte h,z80_byte l);
extern z80_byte lee_puerto_cpc(z80_byte h,z80_byte l);
extern z80_byte lee_puerto_sam(z80_byte h,z80_byte l);
extern z80_byte lee_puerto_msx1(z80_byte puerto_h,z80_byte puerto_l);
extern z80_byte lee_puerto_coleco(z80_byte puerto_h,z80_byte puerto_l);
extern z80_byte lee_puerto_sg1000(z80_byte puerto_h,z80_byte puerto_l);
extern z80_byte lee_puerto_sms(z80_byte puerto_h,z80_byte puerto_l);
extern z80_byte lee_puerto_svi(z80_byte puerto_h,z80_byte puerto_l);
extern z80_byte (*lee_puerto)(z80_byte puerto_h,z80_byte puerto_l);


extern void (*out_port)(z80_int puerto,z80_byte value);
extern void cpi_cpd_common(void);

extern void out_port_spectrum(z80_int puerto,z80_byte value);
extern void out_port_zx80(z80_int puerto,z80_byte value);
extern void out_port_zx81(z80_int puerto,z80_byte value);
extern void out_port_ace(z80_int puerto,z80_byte value);
extern void out_port_cpc(z80_int puerto,z80_byte value);
extern void out_port_sam(z80_int puerto,z80_byte value);
extern void out_port_msx1(z80_int puerto,z80_byte value);
extern void out_port_coleco(z80_int puerto,z80_byte value);
extern void out_port_sg1000(z80_int puerto,z80_byte value);
extern void out_port_sms(z80_int puerto,z80_byte value);
extern void out_port_svi(z80_int puerto,z80_byte value);

extern void out_port_spectrum_no_time(z80_int puerto,z80_byte value);

extern z80_byte idle_bus_port(z80_int puerto);

extern void set_peek_byte_function_ram_refresh(void);
extern void reset_peek_byte_function_ram_refresh(void);

extern void poke_inves_rom(z80_byte value);

extern z80_byte get_border_colour_from_out(void);

extern z80_byte envia_load_ctrlenter_cpc(z80_byte index_keyboard_table);

//extern z80_byte envia_load_f8_sam(z80_byte puerto_h,z80_byte puerto_l);
extern z80_byte envia_load_comillas_sam(z80_byte puerto_h,z80_byte puerto_l);

extern z80_byte *sam_return_segment_memory(z80_int dir);

extern z80_byte *tbblue_return_segment_memory(z80_int dir);

#endif
