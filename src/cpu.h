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

#ifndef CPU_H
#define CPU_H

#include "compileoptions.h"

/*
Emulator version format:
XX.YY-S

XX: Major version number. 1 or 2 digits number. Currently using only one digit but could be greater (on ZEsarUX version 10 or higher)
YY: Minor version number. 1 or 2 digits number
-S: Optional string, only present on non-stable versions. S it's normally "SN" or "RC", where RC can also be RCX, being X a digit: RC1, RC2, etc

Examples

6.0: Means major version 6, minor version 0. It's a stable version (no suffix string -....)

6.1-RC: Means major version 6, minor version 1, beta version: RC
6.2-RC3: Means major version 6, minor version 2, beta version: RC3

*/


#define EMULATOR_VERSION "10.3-SN"
//#define EMULATOR_VERSION "10.3"
//#define EMULATOR_VERSION "10.3-RC"
//#define EMULATOR_VERSION "10.3"
#define SNAPSHOT_VERSION

#define EMULATOR_DATE "02 November 2022"
#define EMULATOR_SHORT_DATE "02/11/2022"
#define EMULATOR_GAME_EDITION "xxx"
#define EMULATOR_EDITION_NAME EMULATOR_GAME_EDITION " edition"
#define ZESARUX_EXTRAS_HOST "github.com" 


//testeo o para forzar una versión en develop
//#define ZESARUX_EXTRAS_URL "https://github.com/chernandezba/zesarux/releases/download/ZEsarUX-10.0/ZEsarUX_extras-10.0.zip"

//final
#define ZESARUX_EXTRAS_URL "/chernandezba/zesarux/releases/download/ZEsarUX-" EMULATOR_VERSION "/ZEsarUX_extras-" EMULATOR_VERSION ".zip"

#define EDITION_NAME_GAME "editionnamegame.tap"


//8 bits
typedef unsigned char z80_byte;
//16 bits
typedef unsigned short z80_int;
//32 bits
typedef unsigned int z80_long_int;
typedef unsigned int z80_32bit;
//64 bits
typedef long long int z80_64bit;

//Usado en menu, para registros de 16 bits de z80 y de 32 de motorola
typedef unsigned int menu_z80_moto_int;

struct s_z80_bit {
	unsigned int v:1;
};


typedef union {
#ifdef WORDS_BIGENDIAN
  struct { z80_byte h,l; } b;
#else
  struct { z80_byte l,h; } b;
#endif

  z80_int w;
} z80_registro;


typedef struct s_z80_bit z80_bit;


extern int zoom_x,zoom_y;
extern int zoom_x_original,zoom_y_original;

extern z80_bit autochange_zoom_big_display;


extern z80_registro registro_hl;
#define reg_h registro_hl.b.h
#define reg_l registro_hl.b.l
#define HL registro_hl.w
#define reg_hl registro_hl.w


extern z80_registro registro_de;
#define reg_d registro_de.b.h
#define reg_e registro_de.b.l
#define DE registro_de.w
#define reg_de registro_de.w


extern z80_registro registro_bc;
#define reg_b registro_bc.b.h
#define reg_c registro_bc.b.l
#define BC registro_bc.w
#define reg_bc registro_bc.w


extern z80_byte reg_i;
extern z80_byte reg_r,reg_r_bit7;

#define IR ( ( reg_i ) << 8 | ( reg_r_bit7 & 0x80 ) | ( reg_r & 0x7f ) )

extern z80_byte reg_h_shadow,reg_l_shadow;
extern z80_byte reg_b_shadow,reg_c_shadow;
extern z80_byte reg_d_shadow,reg_e_shadow;
extern z80_byte reg_a_shadow;

#define REG_AF (value_8_to_16(reg_a,Z80_FLAGS))

#define REG_AF_SHADOW (value_8_to_16(reg_a_shadow,Z80_FLAGS_SHADOW))
#define REG_HL_SHADOW (value_8_to_16(reg_h_shadow,reg_l_shadow))
#define REG_BC_SHADOW (value_8_to_16(reg_b_shadow,reg_c_shadow))
#define REG_DE_SHADOW (value_8_to_16(reg_d_shadow,reg_e_shadow))

extern void set_machine(char *romfile);
extern void set_machine_params(void);
extern void post_set_machine(char *romfile);
extern void post_set_machine_no_rom_load(void);

extern z80_int reg_pc;
extern z80_byte reg_a;
extern z80_int reg_sp;
extern z80_int reg_ix;
extern z80_int reg_iy;

extern z80_byte Z80_FLAGS;
extern z80_byte Z80_FLAGS_SHADOW;

#define FLAG_C  0x01
#define FLAG_N  0x02
#define FLAG_PV 0x04
#define FLAG_3  0x08
#define FLAG_H  0x10
#define FLAG_5  0x20
#define FLAG_Z  0x40
#define FLAG_S  0x80

extern z80_int memptr;

extern z80_byte scf_ccf_undoc_flags_before;
extern int scf_ccf_undoc_flags_after_changed;

extern int z80_ejecutada_instruccion_bloque_ld_cp;
extern int z80_ejecutada_instruccion_bloque_ot_in;
extern z80_byte z80_last_data_transferred_ot_in;

extern z80_bit iff1,iff2;

extern z80_bit interrupcion_pendiente;
extern z80_bit z80_halt_signal;
extern z80_byte im_mode;
extern z80_bit cpu_step_mode;
extern int core_refetch;
extern int cpu_duracion_pulso_interrupcion;
extern z80_bit core_end_frame_check_zrcp_zeng_snap;

extern unsigned int buildnumber_int;
extern unsigned int last_buildnumber_int;
extern z80_bit zesarux_has_been_downgraded;
extern char last_version_text_string[];

extern z80_int get_im2_interrupt_vector(void);

#ifndef GCC_UNUSED

#ifdef __GNUC__
#  define GCC_UNUSED __attribute__((unused))
#else
#  define GCC_UNUSED
#endif

#endif


//Para haiku
#ifdef HAIKU_OS
//Para incluir PATH_MAX
#include <limits.h>
#endif


extern z80_byte puerto_65278; //    db    255  ; V    C    X    Z    Sh    ;0
extern z80_byte puerto_65022; //    db    255  ; G    F    D    S    A     ;1
extern z80_byte puerto_64510; //    db              255  ; T    R    E    W    Q     ;2
extern z80_byte puerto_63486; //    db              255  ; 5    4    3    2    1     ;3
extern z80_byte puerto_61438; //    db              255  ; 6    7    8    9    0     ;4
extern z80_byte puerto_57342; //    db              255  ; Y    U    I    O    P     ;5
extern z80_byte puerto_49150; //    db              255  ; H                J         K      L    Enter ;6
extern z80_byte puerto_32766; //    db              255  ; B    N    M    Simb Space ;7
extern z80_byte puerto_especial1;
extern z80_byte puerto_especial2;
extern z80_byte puerto_especial3;
extern z80_byte puerto_especial4;


extern z80_int *registro_ixiy;

extern z80_bit border_enabled;


extern void (*codsinpr[]) ();
extern void (*codprcb[]) ();
extern void (*codpred[]) ();

extern void (*codprddfd[]) ();

extern z80_byte *memoria_spectrum;

//extern z80_byte inves_ula_delay_factor;
extern z80_byte current_machine_type;
extern z80_bit modificado_border;

extern z80_bit inves_ula_bright_error;




extern void set_undocumented_flags_bits(z80_byte value);
extern void set_flags_zero_sign(z80_byte value);
extern void set_flags_zero_sign_16(z80_int value);
extern void set_flags_carry_suma(z80_byte antes,z80_byte result);
extern void set_flags_carry_resta(z80_byte antes,z80_byte result);
extern void set_flags_carry_16_suma(z80_int antes,z80_int result);
extern void set_flags_carry_16_resta(z80_int antes,z80_int result);

extern void z80_adjust_flags_interrupt_block_opcode(void);
extern void z80_no_ejecutado_block_opcodes(void);
extern void reset_cpu(void);
extern void cold_start_cpu_registers(void);

#define value_8_to_16(h,l) (((h)<<8)|l)

#define value_16_to_8l(hl) ((hl) & 0xFF)
#define value_16_to_8h(hl) (((hl)>>8) & 0xFF)

extern z80_byte *devuelve_reg_offset(z80_byte valor);

extern int video_zx8081_linecntr;
extern z80_bit video_zx8081_linecntr_enabled;

extern z80_byte video_zx8081_ula_video_output;


//#define TEMP_LINEAS_ZX8081 192
//#define TEMP_BORDE_SUP_ZX8081 56


//#define macro_invalid_opcode(MESSAGE) debug_printf(VERBOSE_ERR,"Invalid opcode " MESSAGE ". Final PC: %X",reg_pc)

//extern z80_bit debug_cpu_core_loop;
//extern int cpu_core_loop_active;
//extern void (*cpu_core_loop) (void);

#define CPU_CORE_SPECTRUM 1
#define CPU_CORE_ZX8081 2
#define CPU_CORE_Z88 3
#define CPU_CORE_ACE 4
#define CPU_CORE_CPC 5
#define CPU_CORE_SAM 6
#define CPU_CORE_QL 7
#define CPU_CORE_MK14 8
#define CPU_CORE_MSX 9
#define CPU_CORE_COLECO 10
#define CPU_CORE_SG1000 11
#define CPU_CORE_SVI 12
#define CPU_CORE_SMS 13
#define CPU_CORE_PCW 14

extern struct timeval z80_interrupts_timer_antes, z80_interrupts_timer_ahora;
extern long z80_timer_difftime, z80_timer_seconds, z80_timer_useconds;
extern struct timeval zesarux_start_time;
extern z80_bit interrupcion_timer_generada;
extern int contador_frames_veces_buffer_audio;
extern z80_bit esperando_tiempo_final_t_estados;

extern z80_bit interrupcion_maskable_generada;
extern z80_bit interrupcion_non_maskable_generada;
extern z80_bit interrupcion_timer_generada;
extern z80_byte reg_r_antes_zx8081;
extern z80_bit temp_zx8081_lineasparimpar;

//40 mas que suficiente
#define MAX_MACHINE_NAME 40

struct s_machine_names {
	char nombre_maquina[MAX_MACHINE_NAME];  
	int id;
};

extern struct s_machine_names machine_names[];

extern char *get_machine_name(z80_byte m);

extern z80_bit cpu_random_r_register;

extern int porcentaje_velocidad_emulador;
extern void set_emulator_speed(void);

//T-estados totales del frame
extern int t_estados;

extern int t_scanline;

//Scan line actual
extern int t_scanline_draw;
extern int t_scanline_draw_timeout;

//Autoseleccionar opciones de emulacion (audio, realvideo, etc) segun snap o cinta cargada
extern z80_bit autoselect_snaptape_options;

extern z80_bit tape_loading_simulate;
extern z80_bit tape_loading_simulate_fast;
extern void end_emulator(void);
extern void end_emulator_saveornot_config(int saveconfig);
extern void end_emulator_autosave_snapshot(void);
//extern int ending_emulator_flag;

extern z80_bit snow_effect_enabled;

struct s_driver_struct
{
        char driver_name[30];
        int (*funcion_init) () ;
        int (*funcion_set) () ;
};

typedef struct s_driver_struct driver_struct;

extern driver_struct scr_driver_array[];
extern int num_scr_driver_array;

extern driver_struct audio_driver_array[];
extern int num_audio_driver_array;

extern char *driver_audio;

#define MAX_SCR_INIT 15
#define MAX_AUDIO_INIT 15

extern z80_bit stdout_simpletext_automatic_redraw;
//Valores para stdout. Estan aqui porque se graban en archivo .zx, aunque no este el driver stdout compilado
//fin valores para stdout

//
// Identificadores de maquinas. Valor de 8 bits
//
#define MACHINE_ID_SPECTRUM_16			    0
#define MACHINE_ID_SPECTRUM_48			    1
#define MACHINE_ID_INVES			        2
#define MACHINE_ID_MICRODIGITAL_TK90X		3
#define MACHINE_ID_MICRODIGITAL_TK90X_SPA	4
#define MACHINE_ID_MICRODIGITAL_TK95		5
#define MACHINE_ID_SPECTRUM_128		    	6
#define MACHINE_ID_SPECTRUM_128_SPA		    7
#define MACHINE_ID_SPECTRUM_P2		    	8
#define MACHINE_ID_SPECTRUM_P2_FRE		    9
#define MACHINE_ID_SPECTRUM_P2_SPA	    	10
#define MACHINE_ID_SPECTRUM_P2A_40		    11
#define MACHINE_ID_SPECTRUM_P2A_41  		12
#define MACHINE_ID_SPECTRUM_P2A_SPA	    	13
#define MACHINE_ID_ZXUNO			        14
#define MACHINE_ID_CHLOE_140SE  			15
#define MACHINE_ID_CHLOE_280SE	    		16
#define MACHINE_ID_TIMEX_TS2068		    	17
#define MACHINE_ID_PRISM		    	    18
#define MACHINE_ID_TBBLUE			        19
#define MACHINE_ID_SPECTRUM_48_PLUS_SPA		20
#define MACHINE_ID_PENTAGON	        		21
#define MACHINE_ID_CHROME			        22
#define MACHINE_ID_TSCONF		        	23
#define MACHINE_ID_BASECONF			        24
#define MACHINE_ID_SPECTRUM_P3_40	    	25
#define MACHINE_ID_SPECTRUM_P3_41		    26
#define MACHINE_ID_SPECTRUM_P3_SPA		    27
#define MACHINE_ID_SPECTRUM_48_PLUS_ENG		28
#define MACHINE_ID_TIMEX_TC2048			    29
#define MACHINE_ID_TIMEX_TC2068			    30
#define MACHINE_ID_MICRODIGITAL_TK95_SPA	31

#define MACHINE_ID_COLECO                   100

#define MACHINE_ID_SG1000                   101

#define MACHINE_ID_SVI_318                  102
#define MACHINE_ID_SVI_328                  103

#define MACHINE_ID_SMS                      104

#define MACHINE_ID_MSX1                     110

#define MACHINE_ID_ZX80			        	120
#define MACHINE_ID_ZX81		        		121
#define MACHINE_ID_ACE		        		122
#define MACHINE_ID_TIMEX_TS1000	        	123
#define MACHINE_ID_TIMEX_TS1500	        	124
#define MACHINE_ID_MICRODIGITAL_TK80       	125
#define MACHINE_ID_MICRODIGITAL_TK82       	126
#define MACHINE_ID_MICRODIGITAL_TK82C      	127
#define MACHINE_ID_MICRODIGITAL_TK83       	128
#define MACHINE_ID_MICRODIGITAL_TK85       	129

#define MACHINE_ID_Z88		        		130

#define MACHINE_ID_CPC_464	        		140
#define MACHINE_ID_CPC_4128	        		141
#define MACHINE_ID_CPC_664	        		142
#define MACHINE_ID_CPC_6128	        		143

#define MACHINE_ID_SAM		        		150

#define MACHINE_ID_QL_STANDARD	    		160

#define MACHINE_ID_MK14_STANDARD	    	180

#define MACHINE_ID_PCW_8256                 190
#define MACHINE_ID_PCW_8512                 191


//
//Condiciones de maquinas activas
//De maquinas únicas
//

#define MACHINE_IS_SPECTRUM_16              (current_machine_type==MACHINE_ID_SPECTRUM_16)
#define MACHINE_IS_SPECTRUM_48              (current_machine_type==MACHINE_ID_SPECTRUM_48)
#define MACHINE_IS_INVES                    (current_machine_type==MACHINE_ID_INVES)
#define MACHINE_IS_MICRODIGITAL_TK90X       (current_machine_type==MACHINE_ID_MICRODIGITAL_TK90X)
#define MACHINE_IS_MICRODIGITAL_TK90X_SPA   (current_machine_type==MACHINE_ID_MICRODIGITAL_TK90X_SPA)
#define MACHINE_IS_MICRODIGITAL_TK95        (current_machine_type==MACHINE_ID_MICRODIGITAL_TK95)
#define MACHINE_IS_MICRODIGITAL_TK95_SPA    (current_machine_type==MACHINE_ID_MICRODIGITAL_TK95_SPA)
#define MACHINE_IS_SPECTRUM_128             (current_machine_type==MACHINE_ID_SPECTRUM_128)
#define MACHINE_IS_SPECTRUM_128_SPA         (current_machine_type==MACHINE_ID_SPECTRUM_128_SPA)

#define MACHINE_IS_ZXUNO                    (current_machine_type==MACHINE_ID_ZXUNO)

#define MACHINE_IS_CHLOE_140SE              (current_machine_type==MACHINE_ID_CHLOE_140SE)
#define MACHINE_IS_CHLOE_280SE              (current_machine_type==MACHINE_ID_CHLOE_280SE)

//MACHINE_IS_TIMEX_TS_TC_2068 engloba la TS2068 (Timex Sinclair 2068) y TC2068 (Timex Computer 2048) que son iguales practicamente
//fisicamente son identicas excepto que en el logo una dice Timex Sinclair y la otra Timex Computer

#define MACHINE_IS_TIMEX_TS2068             (current_machine_type==MACHINE_ID_TIMEX_TS2068)
#define MACHINE_IS_TIMEX_TC2068             (current_machine_type==MACHINE_ID_TIMEX_TC2068)
#define MACHINE_IS_TIMEX_TC2048             (current_machine_type==MACHINE_ID_TIMEX_TC2048)

#define MACHINE_IS_PRISM                    (current_machine_type==MACHINE_ID_PRISM)
#define MACHINE_IS_TBBLUE                   (current_machine_type==MACHINE_ID_TBBLUE)
#define MACHINE_IS_PENTAGON                 (current_machine_type==MACHINE_ID_PENTAGON)
#define MACHINE_IS_CHROME                   (current_machine_type==MACHINE_ID_CHROME)
#define MACHINE_IS_TSCONF                   (current_machine_type==MACHINE_ID_TSCONF)
#define MACHINE_IS_BASECONF                 (current_machine_type==MACHINE_ID_BASECONF)

#define MACHINE_IS_COLECO                   (current_machine_type==MACHINE_ID_COLECO)
#define MACHINE_IS_SG1000                   (current_machine_type==MACHINE_ID_SG1000)
#define MACHINE_IS_SMS                      (current_machine_type==MACHINE_ID_SMS)
#define MACHINE_IS_SVI_318                  (current_machine_type==MACHINE_ID_SVI_318)
#define MACHINE_IS_SVI_328                  (current_machine_type==MACHINE_ID_SVI_328)
#define MACHINE_IS_MSX1                     (current_machine_type==MACHINE_ID_MSX1)

//Basados en ZX80
#define MACHINE_IS_ZX80                     (current_machine_type==MACHINE_ID_ZX80)
#define MACHINE_IS_MICRODIGITAL_TK80        (current_machine_type==MACHINE_ID_MICRODIGITAL_TK80)
#define MACHINE_IS_MICRODIGITAL_TK82        (current_machine_type==MACHINE_ID_MICRODIGITAL_TK82)


//Basados en ZX81
#define MACHINE_IS_ZX81                     (current_machine_type==MACHINE_ID_ZX81)
#define MACHINE_IS_TIMEX_TS1000             (current_machine_type==MACHINE_ID_TIMEX_TS1000)
#define MACHINE_IS_TIMEX_TS1500             (current_machine_type==MACHINE_ID_TIMEX_TS1500)
#define MACHINE_IS_MICRODIGITAL_TK82C       (current_machine_type==MACHINE_ID_MICRODIGITAL_TK82C)
#define MACHINE_IS_MICRODIGITAL_TK83        (current_machine_type==MACHINE_ID_MICRODIGITAL_TK83)
#define MACHINE_IS_MICRODIGITAL_TK85        (current_machine_type==MACHINE_ID_MICRODIGITAL_TK85)

#define MACHINE_IS_ACE                      (current_machine_type==MACHINE_ID_ACE)

#define MACHINE_IS_Z88                      (current_machine_type==MACHINE_ID_Z88)

#define MACHINE_IS_CPC_464                  (current_machine_type==MACHINE_ID_CPC_464)
#define MACHINE_IS_CPC_4128                 (current_machine_type==MACHINE_ID_CPC_4128)
#define MACHINE_IS_CPC_6128                 (current_machine_type==MACHINE_ID_CPC_6128)
#define MACHINE_IS_CPC_664                  (current_machine_type==MACHINE_ID_CPC_664)

#define MACHINE_IS_CPC_HAS_FLOPPY           (MACHINE_IS_CPC_6128 || MACHINE_IS_CPC_664)
#define MACHINE_IS_CPC_HAS_128K             (MACHINE_IS_CPC_4128 || MACHINE_IS_CPC_6128)
#define MACHINE_IS_CPC_HAS_64K              (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_664)

#define MACHINE_IS_SAM                      (current_machine_type==MACHINE_ID_SAM)

#define MACHINE_IS_QL_STANDARD              (current_machine_type==MACHINE_ID_QL_STANDARD)

#define MACHINE_IS_MK14_STANDARD            (current_machine_type==MACHINE_ID_MK14_STANDARD)

#define MACHINE_IS_PCW_8256                 (current_machine_type==MACHINE_ID_PCW_8256)
#define MACHINE_IS_PCW_8512                 (current_machine_type==MACHINE_ID_PCW_8512)

//
//Condiciones de maquinas activas
//De maquinas Combinadas
//
#define MACHINE_IS_SPECTRUM                     (current_machine_type<40)

#define MACHINE_IS_SPECTRUM_48_PLUS_SPA         (current_machine_type==MACHINE_ID_SPECTRUM_48_PLUS_SPA)
#define MACHINE_IS_SPECTRUM_48_PLUS_ENG         (current_machine_type==MACHINE_ID_SPECTRUM_48_PLUS_ENG)
#define MACHINE_IS_SPECTRUM_16_48               ( (current_machine_type<=MACHINE_ID_MICRODIGITAL_TK95) || MACHINE_IS_SPECTRUM_48_PLUS_SPA || MACHINE_IS_SPECTRUM_48_PLUS_ENG || MACHINE_IS_TIMEX_TC2048 || MACHINE_IS_MICRODIGITAL_TK95_SPA)

#define MACHINE_IS_SPECTRUM_128_P2              ( (current_machine_type>=MACHINE_ID_SPECTRUM_128 && current_machine_type<=MACHINE_ID_SPECTRUM_P2_SPA) || MACHINE_IS_PENTAGON)
#define MACHINE_IS_SPECTRUM_P2                  ( (current_machine_type>=MACHINE_ID_SPECTRUM_P2 && current_machine_type<=MACHINE_ID_SPECTRUM_P2_SPA))
#define MACHINE_IS_SPECTRUM_P3                  (current_machine_type==MACHINE_ID_SPECTRUM_P3_40 || current_machine_type==MACHINE_ID_SPECTRUM_P3_41 || current_machine_type==MACHINE_ID_SPECTRUM_P3_SPA)

//MACHINE_IS_SPECTRUM_P2A_P3 antes de emular el +3, era MACHINE_IS_SPECTRUM_P2A
#define MACHINE_IS_SPECTRUM_P2A_P3              ( (current_machine_type>=MACHINE_ID_SPECTRUM_P2A_40 && current_machine_type<=MACHINE_ID_SPECTRUM_P2A_SPA) || MACHINE_IS_SPECTRUM_P3)
#define MACHINE_IS_SPECTRUM_P2A                 ( current_machine_type>=MACHINE_ID_SPECTRUM_P2A_40 && current_machine_type<=MACHINE_ID_SPECTRUM_P2A_SPA)

//MACHINE_IS_SPECTRUM_128_P2_P2A_P3 era MACHINE_IS_SPECTRUM_128_P2_P2A antes de emular el +3
#define MACHINE_IS_SPECTRUM_128_P2_P2A_P3       ( MACHINE_IS_SPECTRUM_128_P2 || MACHINE_IS_SPECTRUM_P2A_P3)
#define MACHINE_IS_SPECTRUM_16_48_128_P2_P2A_P3 (MACHINE_IS_SPECTRUM_16_48 || MACHINE_IS_SPECTRUM_128_P2_P2A_P3)

#define MACHINE_IS_CHLOE                        (MACHINE_IS_CHLOE_140SE || MACHINE_IS_CHLOE_280SE)

#define MACHINE_IS_ZXEVO                        (MACHINE_IS_TSCONF || MACHINE_IS_BASECONF)

#define MACHINE_IS_TIMEX_TS_TC_2068             (MACHINE_IS_TIMEX_TS2068 || MACHINE_IS_TIMEX_TC2068)

#define MACHINE_IS_ZX80_TYPE                    (MACHINE_IS_ZX80 || MACHINE_IS_MICRODIGITAL_TK80 || MACHINE_IS_MICRODIGITAL_TK82)
#define MACHINE_IS_ZX81_TYPE                    (MACHINE_IS_ZX81 || MACHINE_IS_TIMEX_TS1000 || MACHINE_IS_TIMEX_TS1500 || MACHINE_IS_MICRODIGITAL_TK82C || MACHINE_IS_MICRODIGITAL_TK83 || MACHINE_IS_MICRODIGITAL_TK85)
#define MACHINE_IS_ZX8081                       (MACHINE_IS_ZX80_TYPE || MACHINE_IS_ZX81_TYPE)
#define MACHINE_IS_ZX8081ACE                    (MACHINE_IS_ZX8081 || MACHINE_IS_ACE)

#define MACHINE_IS_CPC                          (current_machine_type>=MACHINE_ID_CPC_464 && current_machine_type<=149)

#define MACHINE_IS_MSX                          (current_machine_type>=MACHINE_ID_MSX1 && current_machine_type<=119)

#define MACHINE_IS_SVI                          (current_machine_type==MACHINE_ID_SVI_318 || current_machine_type==MACHINE_ID_SVI_328)

/*
160=QL
161-179 reservado para otros QL
*/


#define MACHINE_IS_QL                           (current_machine_type>=MACHINE_ID_QL_STANDARD && current_machine_type<=179)

/*
180=MK14 Standard
181-189 reservado para otros MK14
*/


#define MACHINE_IS_MK14                         (current_machine_type>=MACHINE_ID_MK14_STANDARD && current_machine_type<=189)

#define MACHINE_IS_PCW                          (current_machine_type>=MACHINE_ID_PCW_8256 && current_machine_type<=199)


//
//Condiciones de CPU
//
#define CPU_IS_MOTOROLA (MACHINE_IS_QL)
#define CPU_IS_SCMP (MACHINE_IS_MK14)
#define CPU_IS_Z80 (!CPU_IS_MOTOROLA && !CPU_IS_SCMP)


//Maquinas que tienen el chip de memoria vdp 9918a
#define MACHINE_HAS_VDP_9918A (MACHINE_IS_MSX || MACHINE_IS_COLECO || MACHINE_IS_SG1000 || MACHINE_IS_SVI || MACHINE_IS_SMS)


//Familias de maquinas
enum machine_families_list
{
    MACHINE_FAMILY_SPECTRUM,
    MACHINE_FAMILY_ZX80,
    MACHINE_FAMILY_ZX81,
    MACHINE_FAMILY_COLECO,
    MACHINE_FAMILY_SG1000,
    MACHINE_FAMILY_SVI,
    MACHINE_FAMILY_SMS,
    MACHINE_FAMILY_MSX,
    MACHINE_FAMILY_ACE,
    MACHINE_FAMILY_Z88,
    MACHINE_FAMILY_CPC,
    MACHINE_FAMILY_PCW,
    MACHINE_FAMILY_QL,
    MACHINE_FAMILY_MK14,

    MACHINE_FAMILY_EOF  //Usado para indicar final
};

#define MAX_FAMILY_NAME_LENGTH 30
struct s_machine_family_names
{
    enum machine_families_list family_id;
    char family_name[MAX_FAMILY_NAME_LENGTH];
};

struct s_machine_family
{
    z80_byte machine_id;
    enum machine_families_list family_id;
};


extern int machine_emulate_memory_refresh;
extern int machine_emulate_memory_refresh_counter;

extern z80_bit cpu_ldir_lddr_hack_optimized;

extern z80_byte last_inves_low_ram_poke_menu;

extern void random_ram_inves(z80_byte *puntero,int longitud);

//Tipos de CPU Z80
#define TOTAL_Z80_CPU_TYPES 3
enum z80_cpu_types
{
  Z80_TYPE_GENERIC,
  Z80_TYPE_MOSTEK,
  Z80_TYPE_CMOS
};

extern enum z80_cpu_types z80_cpu_current_type;

extern char *z80_cpu_types_strings[];

//valor obtenido probando
#define MAX_EMULATE_MEMORY_REFRESH_COUNTER 1500000
#define MAX_EMULATE_MEMORY_REFRESH_LIMIT (MAX_EMULATE_MEMORY_REFRESH_COUNTER/2)


extern int zesarux_main (int main_argc,char *main_argv[]);
extern z80_bit no_cambio_parametros_maquinas_lentas;
extern z80_bit opcion_no_welcome_message;

extern int argc;
extern char **argv;
extern int puntero_parametro;
extern int siguiente_parametro(void);

extern void siguiente_parametro_argumento(void);

extern int joystickkey_definidas;
extern void rom_load(char *romfilename);
extern void hard_reset_cpu(void);

extern z80_bit zxmmc_emulation;

extern z80_bit quickexit;

extern z80_bit azerty_keyboard;

extern z80_int ramtop_ace;

extern z80_bit allow_write_rom;

extern int z88_cpc_keymap_type;

extern z80_bit chloe_keyboard;

extern char *realmachine_keymap_strings_types[];

extern unsigned int debug_t_estados_parcial;

#define MAX_CPU_TURBO_SPEED 16

extern int cpu_turbo_speed;

extern void cpu_set_turbo_speed(void);

extern z80_bit windows_no_disable_console;

extern char *string_machines_list_description;

extern void set_menu_gui_zoom(void);


extern int exit_emulator_after_seconds;

extern int exit_emulator_after_seconds_counter;

extern z80_bit do_no_show_changelog_when_update;

extern z80_bit sdl_raw_keyboard_read;

extern z80_bit core_spectrum_uses_reduced;


extern char parameter_disablebetawarning[];

extern int total_minutes_use;

extern char macos_path_to_executable[];


#endif
