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

#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "audio.h"
#include "tape.h"
#include "ay38912.h"
#include "mem128.h"
#include "zx8081.h"
#include "zxvision.h"
#include "screen.h"
#include "compileoptions.h"
#include "contend.h"
#include "joystick.h"
#include "ula.h"
#include "utils.h"
#include "printers.h"
#include "disassemble.h"
#include "scrstdout.h"
#include "ulaplus.h"
#include "zxuno.h"
#include "chardetect.h"
#include "mmc.h"
#include "ide.h"
#include "divmmc.h"
#include "divide.h"
#include "diviface.h"
#include "zxpand.h"
#include "spectra.h"
#include "spritechip.h"
#include "jupiterace.h"
#include "chloe.h"
#include "prism.h"
#include "timex.h"
#include "cpc.h"
#include "sam.h"
#include "atomlite.h"
#include "if1.h"
#include "timer.h"
#include "pd765.h"
#include "tbblue.h"
#include "superupgrade.h"
#include "snap_rzx.h"
#include "multiface.h"
#include "ql.h"
#include "chrome.h"
#include "ds1307.h"
#include "tsconf.h"
#include "mk14.h"
#include "betadisk.h"
#include "baseconf.h"
#include "zxevo.h"
#include "settings.h"
#include "saa_simul.h"
#include "datagear.h"
#include "hilow_datadrive.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "sn76489an.h"
#include "svi.h"
#include "vdp_9918a.h"
#include "gs.h"
#include "samram.h"
#include "vdp_9918a_sms.h"
#include "hilow_barbanegra.h"
#include "transtape.h"
#include "phoenix.h"
#include "ramjet.h"
#include "plus3dos_handler.h"
#include "pcw.h"
#include "dsk.h"
#include "utils_text_adventure.h"


void (*poke_byte)(z80_int dir,z80_byte valor);
void (*poke_byte_no_time)(z80_int dir,z80_byte valor);
z80_byte (*peek_byte)(z80_int dir);
z80_byte (*peek_byte_no_time)(z80_int dir);
z80_byte (*lee_puerto)(z80_byte puerto_h,z80_byte puerto_l);
void (*out_port)(z80_int puerto,z80_byte value);
z80_byte (*fetch_opcode)(void);

void (*push_valor)(z80_int valor,z80_byte tipo);

z80_byte lee_puerto_teclado(z80_byte puerto_h);
z80_byte lee_puerto_spectrum_no_time(z80_byte puerto_h,z80_byte puerto_l);

void set_value_beeper (int v);

z80_byte idle_bus_port_atribute(void);

//Tablas para instrucciones adc, sbc y otras
z80_byte overflow_add_table[] = { 0, 0, 0, FLAG_PV, FLAG_PV, 0, 0, 0 };

z80_byte halfcarry_add_table[] ={ 0, FLAG_H, FLAG_H, FLAG_H, 0, 0, 0, FLAG_H };
z80_byte halfcarry_sub_table[] = { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };

z80_byte overflow_sub_table[] = { 0, FLAG_PV, 0, 0, 0, 0, FLAG_PV, 0 };

z80_byte parity_table[256];
z80_byte sz53_table[256];
z80_byte sz53p_table[256];



#ifdef EMULATE_VISUALMEM


int get_visualmem_size(void)
{
	int visualmem_size=(QL_MEM_LIMIT)+1;
	return visualmem_size;
}

//Entre 1 y 255 indica memoria modificada y cuantas veces
//A 0 se establece desde opcion de menu
//char visualmem_buffer[65536];
z80_byte *visualmem_buffer=NULL;

//lo mismo pero para lectura de memoria
z80_byte *visualmem_read_buffer=NULL;

//lo mismo pero para ejecucion de opcodes
z80_byte *visualmem_opcode_buffer=NULL;

//lo mismo pero para mmc lectura
z80_byte *visualmem_mmc_read_buffer=NULL;

//lo mismo pero para mmc escritura
z80_byte *visualmem_mmc_write_buffer=NULL;


//lo mismo pero para hilow lectura
z80_byte *visualmem_hilow_read_buffer=NULL;

//lo mismo pero para hilow escritura
z80_byte *visualmem_hilow_write_buffer=NULL;

void init_visualmembuffer(void)
{
	//int visualmem_size=65536;

	//int visualmem_size=(QL_MEM_LIMIT)+1;
	int visualmem_size=get_visualmem_size();

	debug_printf(VERBOSE_INFO,"Allocating %d bytes for visualmem write buffer",visualmem_size);
	visualmem_buffer=util_malloc_fill(visualmem_size,"Can not allocate visualmem write buffer",0);


	debug_printf(VERBOSE_INFO,"Allocating %d bytes for visualmem read buffer",visualmem_size);
	visualmem_read_buffer=util_malloc_fill(visualmem_size,"Can not allocate visualmem read buffer",0);


	debug_printf(VERBOSE_INFO,"Allocating %d bytes for visualmem opcode buffer",visualmem_size);
	visualmem_opcode_buffer=util_malloc_fill(visualmem_size,"Can not allocate visualmem opcode buffer",0);


	debug_printf(VERBOSE_INFO,"Allocating %d bytes for visualmem mmc read buffer",VISUALMEM_MMC_BUFFER_SIZE);
	visualmem_mmc_read_buffer=util_malloc_fill(VISUALMEM_MMC_BUFFER_SIZE,"Can not allocate visualmem mmc read buffer",0);


	debug_printf(VERBOSE_INFO,"Allocating %d bytes for visualmem mmc write buffer",VISUALMEM_MMC_BUFFER_SIZE);
	visualmem_mmc_write_buffer=util_malloc_fill(VISUALMEM_MMC_BUFFER_SIZE,"Can not allocate visualmem mmc write buffer",0);


	debug_printf(VERBOSE_INFO,"Allocating %d bytes for visualmem HiLow read buffer",VISUALMEM_HILOW_BUFFER_SIZE);
	visualmem_hilow_read_buffer=util_malloc_fill(VISUALMEM_HILOW_BUFFER_SIZE,"Can not allocate visualmem HiLow read buffer",0);


	debug_printf(VERBOSE_INFO,"Allocating %d bytes for visualmem HiLow write buffer",VISUALMEM_HILOW_BUFFER_SIZE);
	visualmem_hilow_write_buffer=util_malloc_fill(VISUALMEM_HILOW_BUFFER_SIZE,"Can not allocate visualmem HiLow write buffer",0);


}

void set_visualmembuffer(int dir)
{
	//visualmem_buffer[dir]=1;
	z80_byte valor=visualmem_buffer[dir];
	if (valor<255) visualmem_buffer[dir]=valor+1;

	//printf ("dir: %d\n",dir);
}

void set_visualmemreadbuffer(int dir)
{
	//visualmem_buffer[dir]=1;
	z80_byte valor=visualmem_read_buffer[dir];
	if (valor<255) visualmem_read_buffer[dir]=valor+1;

	//printf ("dir: %d\n",dir);
}

void set_visualmemopcodebuffer(int dir)
{
	//visualmem_buffer[dir]=1;
    //printf ("dir: %d\n",dir);
	z80_byte valor=visualmem_opcode_buffer[dir];
	if (valor<255) visualmem_opcode_buffer[dir]=valor+1;


}

void set_visualmemmmc_read_buffer(int dir)
{
        z80_byte valor=visualmem_mmc_read_buffer[dir];
        if (valor<255) visualmem_mmc_read_buffer[dir]=valor+1;
}

void set_visualmemmmc_write_buffer(int dir)
{
        z80_byte valor=visualmem_mmc_write_buffer[dir];
        if (valor<255) visualmem_mmc_write_buffer[dir]=valor+1;
}

void set_visualmemhilow_read_buffer(int dir)
{
        z80_byte valor=visualmem_hilow_read_buffer[dir];
        if (valor<255) visualmem_hilow_read_buffer[dir]=valor+1;
}

void set_visualmemhilow_write_buffer(int dir)
{
        z80_byte valor=visualmem_hilow_write_buffer[dir];
        if (valor<255) visualmem_hilow_write_buffer[dir]=valor+1;
}

void clear_visualmembuffer(int dir)
{
        visualmem_buffer[dir]=0;
}

void clear_visualmemreadbuffer(int dir)
{
        visualmem_read_buffer[dir]=0;
}

void clear_visualmemopcodebuffer(int dir)
{
        visualmem_opcode_buffer[dir]=0;
}

void clear_visualmemmmc_read_buffer(int dir)
{
        visualmem_mmc_read_buffer[dir]=0;
}

void clear_visualmemmmc_write_buffer(int dir)
{
        visualmem_mmc_write_buffer[dir]=0;
}

void clear_visualmemhilow_read_buffer(int dir)
{
        visualmem_hilow_read_buffer[dir]=0;
}

void clear_visualmemhilow_write_buffer(int dir)
{
        visualmem_hilow_write_buffer[dir]=0;
}

#endif


//La mayoria de veces se llama aqui desde set_flags_carry_, dado que casi todas las operaciones que tocan el carry tocan el halfcarry
//hay algunas operaciones, como inc8, que tocan el halfcarry pero no el carry
void set_flags_halfcarry_suma(z80_byte antes,z80_byte result)
{
        antes=antes & 0xF;
        result=result & 0xF;

        if (result<antes) Z80_FLAGS |=FLAG_H;
        else Z80_FLAGS &=(255-FLAG_H);
}

//La mayoria de veces se llama aqui desde set_flags_carry_, dado que casi todas las operaciones que tocan el carry tocan el halfcarry
//hay algunas operaciones, como inc8, que tocan el halfcarry pero no el carry
void set_flags_halfcarry_resta(z80_byte antes,z80_byte result)
{
        antes=antes & 0xF;
        result=result & 0xF;

        if (result>antes) Z80_FLAGS |=FLAG_H;
        else Z80_FLAGS &=(255-FLAG_H);
}


//Well as stated in chapter 3 if the result of an operation in two's complement produces a result that's signed incorrectly then there's an overflow
//o So overflow flag = carry-out flag XOR carry from bit 6 into bit 7.
/*
void set_flags_overflow_inc(z80_byte antes,z80_byte result)
{

	if (result==128) Z80_FLAGS |=FLAG_PV;
	else Z80_FLAGS &=(255-FLAG_PV);

}
*/

/*
void set_flags_overflow_dec(z80_byte antes,z80_byte result)
//Well as stated in chapter 3 if the result of an operation in two's complement produces a result that's signed incorrectly then there's an overflow
//o So overflow flag = carry-out flag XOR carry from bit 6 into bit 7.
{
	if (result==127) Z80_FLAGS |=FLAG_PV;
	else Z80_FLAGS &=(255-FLAG_PV);
}
*/

void set_flags_overflow_suma(z80_byte antes,z80_byte result)

//Siempre llamar a esta funcion despues de haber actualizado el Flag de Carry, pues lo utiliza

//Well as stated in chapter 3 if the result of an operation in two's complement produces a result that's signed incorrectly then there's an overflow
//o So overflow flag = carry-out flag XOR carry from bit 6 into bit 7.
{
	//127+127=254 ->overflow    01111111 01111111 = 11111110    NC   67=y    xor=1
	//-100-100=-200 -> overflow 10011100 10011100 = 00111000     C    67=n   xor=1

	//-2+127=125 -> no overflow 11111110 01111111 = 11111101     C    67=y   xor=0
	//127-2=125 -> no overlow   01111111 11111110 = 11111101     C    67=y   xor=0

	//10-100=-90 -> no overflow 00001010 10011100 = 10100110    NC    67=n   xor=0

	z80_byte overflow67;

        if ( (result & 127) < (antes & 127) ) overflow67=FLAG_C;
        else overflow67=0;

	if ( (Z80_FLAGS & FLAG_C ) ^ overflow67) Z80_FLAGS |=FLAG_PV;
	else Z80_FLAGS &=(255-FLAG_PV);

}

void set_flags_overflow_resta(z80_byte antes,z80_byte result)

//Siempre llamar a esta funcion despues de haber actualizado el Flag de Carry, pues lo utiliza

//Well as stated in chapter 3 if the result of an operation in two's complement produces a result that's signed incorrectly then there's an overflow
//o So overflow flag = carry-out flag XOR carry from bit 6 into bit 7.
{
        //127+127=254 ->overflow    01111111 01111111 = 11111110    NC   67=y    xor=1
        //-100-100=-200 -> overflow 10011100 10011100 = 100111000    C    67=n   xor=1

        //-2+127=125 -> no overflow 11111110 01111111 = 11111101     C    67=y   xor=0
        //127-2=125 -> no overlow   01111111 11111110 = 11111101     C    67=y   xor=0

        //10-100=-90 -> no overflow 00001010 10011100 = 10100110    NC    67=n   xor=0

        z80_byte overflow67;


        if ( (result & 127) > (antes & 127) ) overflow67=FLAG_C;
        else overflow67=0;

	if ( (Z80_FLAGS & FLAG_C ) ^ overflow67) Z80_FLAGS |=FLAG_PV;
	else Z80_FLAGS &=(255-FLAG_PV);

}


void set_flags_overflow_suma_16(z80_int antes,z80_int result)

//Siempre llamar a esta funcion despues de haber actualizado el Flag de Carry, pues lo utiliza

//Well as stated in chapter 3 if the result of an operation in two's complement produces a result that's signed incorrectly then there's an overflow
//o So overflow flag = carry-out flag XOR carry from bit 6 into bit 7.
{
        //127+127=254 ->overflow    01111111 01111111 = 11111110    NC   67=y    xor=1
        //-100-100=-200 -> overflow 10011100 10011100 = 00111000    C    67=n   xor=1

        //-2+127=125 -> no overflow 11111110 01111111 = 11111101     C    67=y   xor=0
        //127-2=125 -> no overlow   01111111 11111110 = 11111101     C    67=y   xor=0

        //10-100=-90 -> no overflow 00001010 10011100 = 10100110    NC    67=n   xor=0

        z80_byte overflow67;


        if ( (result & 32767) < (antes & 32767) ) overflow67=FLAG_C;
        else overflow67=0;

	if ( (Z80_FLAGS & FLAG_C ) ^ overflow67) Z80_FLAGS |=FLAG_PV;
	else Z80_FLAGS &=(255-FLAG_PV);


}

void set_flags_overflow_resta_16(z80_int antes,z80_int result)

//Siempre llamar a esta funcion despues de haber actualizado el Flag de Carry, pues lo utiliza

//Well as stated in chapter 3 if the result of an operation in two's complement produces a result that's signed incorrectly then there's an overflow
//o So overflow flag = carry-out flag XOR carry from bit 6 into bit 7.
{
        //127+127=254 ->overflow    01111111 01111111 = 11111110    NC   67=y    xor=1
        //-100-100=-200 -> overflow 10011100 10011100 = 00111000     C    67=n   xor=1

        //-2+127=125 -> no overflow 11111110 01111111 = 11111101     C    67=y   xor=0
        //127-2=125 -> no overlow   01111111 11111110 = 11111101     C    67=y   xor=0

        //10-100=-90 -> no overflow 00001010 10011100 = 10100110    NC    67=n   xor=0

        z80_byte overflow67;


        if ( (result & 32767) > (antes & 32767) ) overflow67=FLAG_C;
        else overflow67=0;

	if ( (Z80_FLAGS & FLAG_C ) ^ overflow67) Z80_FLAGS |=FLAG_PV;
	else Z80_FLAGS &=(255-FLAG_PV);

}



//activa flags segun instruccion in r,(c)
void set_flags_in_reg(z80_byte value)
{
	Z80_FLAGS=( Z80_FLAGS & FLAG_C) | sz53p_table[value];
}

void set_flags_parity(z80_byte value)
{
	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_PV) ) | parity_table[value];
}



//Parity set if even number of bits set
//Paridad si numero par de bits
z80_byte get_flags_parity(z80_byte value)
{
	z80_byte result;

        result=FLAG_PV;
        z80_byte mascara=1;
        z80_byte bit;

        for (bit=0;bit<8;bit++) {
                if ( (value) & mascara) result = result ^ FLAG_PV;
                mascara = mascara << 1;
        }

	return result;
}



//Inicializar algunas tablas para acelerar cpu
void init_cpu_tables(void)
{
	z80_byte value=0;

	int contador=0;

	debug_printf (VERBOSE_INFO,"Initializing cpu flags tables");

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)


	//Tabla paridad, sz53
	for (contador=0;contador<256;contador++,value++) {
		parity_table[value]=get_flags_parity(value);
		debug_printf (VERBOSE_PARANOID,"Parity table: value: %3d (" BYTETOBINARYPATTERN ") parity: %d",value,BYTETOBINARY(value),parity_table[value]);


		sz53_table[value]=value & ( FLAG_3|FLAG_5|FLAG_S );

		if (value==0) sz53_table[value] |=FLAG_Z;

		debug_printf (VERBOSE_PARANOID,"SZ53 table: value: %3d (" BYTETOBINARYPATTERN ") flags: (" BYTETOBINARYPATTERN ") ",value,BYTETOBINARY(value),BYTETOBINARY(sz53_table[value])) ;


		sz53p_table[value]=sz53_table[value] | parity_table[value];
		debug_printf (VERBOSE_PARANOID,"SZ53P table: value: %3d (" BYTETOBINARYPATTERN ") flags: (" BYTETOBINARYPATTERN ") ",value,BYTETOBINARY(value),BYTETOBINARY(sz53p_table[value])) ;

	}
}

void neg(void)
{
/*
        z80_byte result,antes;

        antes=0;

        result=antes-reg_a;
        reg_a=result;

        set_flags_carry_resta(antes,result);
        set_flags_overflow_resta(antes,result);
        Z80_FLAGS=(Z80_FLAGS & (255-FLAG_3-FLAG_5-FLAG_Z-FLAG_S)) | FLAG_N | sz53_table[result];

*/

        z80_byte tempneg=reg_a;
        reg_a=0;
	sub_a_reg(tempneg);
}



//Comun al generar interrupcion en im0/1
void cpu_common_jump_im01(void)
{
	//if (im_mode==0 || im_mode==1) {
	reg_pc=56;
	t_estados += 7;


	//Im modo 0 la interrupción es un t-estado más rápido que con Im modo 1, en cpu mostek

	if (im_mode==0 && z80_cpu_current_type==Z80_TYPE_MOSTEK) t_estados--;
}


//Rutinas de cpu core vacias para que, al parsear breakpoints del config file, donde aun no hay inicializada maquina,
//funciones como opcode1=XX , peek(x), etc no peten porque utilizan funciones peek. Inicializar también las de puerto por si acaso

z80_byte peek_byte_vacio(z80_int dir GCC_UNUSED)
{
	return 0;
}

void poke_byte_vacio(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED)
{

}


z80_byte lee_puerto_vacio(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l GCC_UNUSED)
{
	return 0;
}


void out_port_vacio(z80_int puerto GCC_UNUSED,z80_byte value GCC_UNUSED)
{

}

z80_byte fetch_opcode_vacio(void)
{
	return 0;
}





void poke_byte_no_time_spectrum_48k(z80_int dir,z80_byte valor)
{
        if (dir>16383) {
		memoria_spectrum[dir]=valor;

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
	}
}

void poke_byte_spectrum_48k(z80_int dir,z80_byte valor)
{
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif

        //Y sumamos estados normales
        t_estados += 3;


        if (dir>16383) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		memoria_spectrum[dir]=valor;


	}
}


z80_byte chardetect_automatic_poke_byte(z80_int dir,z80_byte valor)
{
	//poke_byte_spectrum_48k(dir,valor);
	//printf ("Original: %d\n",original_char_detect_poke_byte);
	//original_char_detect_poke_byte(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_call_previous(chardetect_automatic_nested_id_poke_byte,dir,valor);


        //Para hacer debug de aventuras de texto, ver desde donde se escribe en pantalla
        chardetect_debug_char_table_routines_poke(dir);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}


void poke_byte_no_time_spectrum_128k(z80_int dir,z80_byte valor)
{
int segmento;
z80_byte *puntero;

                segmento=dir / 16384;

        if (dir>16383) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
                dir = dir & 16383;
                puntero=memory_paged[segmento]+dir;

//              printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
                *puntero=valor;
        }
}


void poke_byte_spectrum_128k(z80_int dir,z80_byte valor)
{
int segmento;
z80_byte *puntero;

		segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
			//printf ("t_estados: %d\n",t_estados);
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;



        if (dir>16383) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		dir = dir & 16383;
		puntero=memory_paged[segmento]+dir;

//		printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
		*puntero=valor;
	}
}

void poke_byte_no_time_spectrum_128kp2a(z80_int dir,z80_byte valor)
{
int segmento;
z80_byte *puntero;
                segmento=dir / 16384;

        if (dir>16383) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
                dir = dir & 16383;
                puntero=memory_paged[segmento]+dir;

//              printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
                *puntero=valor;
        }

        else {
                //memoria normalmente ROM. Miramos a ver si esta page RAM in ROM
                if ((puerto_8189&1)==1) {
                        //printf ("Poke en direccion normalmente ROM pero hay RAM. Dir=%d Valor=%d\n",dir,valor);
                        //segmento=0;
                        //dir = dir & 16383;
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
                        puntero=memory_paged[0]+dir;
                        *puntero=valor;
                }
        }

}


void poke_byte_spectrum_128kp2a(z80_int dir,z80_byte valor)
{
int segmento;
z80_byte *puntero;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;


        if (dir>16383) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
                dir = dir & 16383;
                puntero=memory_paged[segmento]+dir;

//              printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
                *puntero=valor;
        }

	else {
		//memoria normalmente ROM. Miramos a ver si esta page RAM in ROM
		if ((puerto_8189&1)==1) {
			//printf ("Poke en direccion normalmente ROM pero hay RAM. Dir=%d Valor=%d\n",dir,valor);
                	//segmento=0;
	                //dir = dir & 16383;
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
	                puntero=memory_paged[0]+dir;
	                *puntero=valor;
		}
	}

}


//Poke en Inves afecta a los 64 kb de RAM
void poke_byte_no_time_spectrum_inves(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
	memoria_spectrum[dir]=valor;
}


void poke_byte_spectrum_inves(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif

        //Y sumamos estados normales
        t_estados += 3;

	poke_byte_no_time_spectrum_inves(dir,valor);

}

void poke_byte_no_time_spectrum_16k(z80_int dir,z80_byte valor)
{

        if (dir>16383 && dir<32768) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		memoria_spectrum[dir]=valor;
	}
}



void poke_byte_spectrum_16k(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif

        //Y sumamos estados normales
        t_estados += 3;



        if (dir>16383 && dir<32768) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		memoria_spectrum[dir]=valor;
	}
}



//Quicksilva QS Sound board
void out_port_chip_ay_zx8081(z80_int puerto,z80_byte valor)
{
      if ( puerto==32766 || puerto==32767 ) {
                //printf("out Puerto sonido chip AY puerto=%d valor=%d\n",puerto,value);
                if (puerto==32767) puerto=65533;
                else if (puerto==32766) puerto=49149;
                //printf("out Puerto cambiado sonido chip AY puerto=%d valor=%d\n",puerto,value);

		//no autoactivamos chip AY, pues este metodo aqui se activa cuando se lee direccion 32766 o 32767 y esto pasa siempre
		//que se inicializa el ZX81
		//activa_ay_chip_si_conviene();
                if (ay_chip_present.v==1) out_port_ay(puerto,valor);
        }
}


void poke_byte_zx80_no_time(z80_int dir,z80_byte valor)
{

	//if (dir==49296) printf ("poke dir=%d valor=%d\n",dir,valor);

	//Modulo RAM en 49152
	if (ram_in_49152.v==1 && dir>49151) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		memoria_spectrum[dir]=valor;
                return;
        }

	//Modulo RAM en 32768
	if (ram_in_32768.v==1 && dir>32767) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		memoria_spectrum[dir]=valor;
                return;
        }



        //Modulo RAM en 2000H
        if (ram_in_8192.v==1 && dir>8191 && dir<16384) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
                //printf ("poke en rom 8192-16383 %d %d\n",dir,valor);
                memoria_spectrum[dir]=valor;
                return;
        }


        //chip ay
        if (dir==32766 || dir==32767) {
                //printf ("poke Chip ay dir=%d valor=%d\n",dir,valor);
                out_port_chip_ay_zx8081(dir,valor);
        }
        if (dir>ramtop_zx8081) dir = (dir&(ramtop_zx8081));

        if (dir>16383 && dir<=ramtop_zx8081) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		memoria_spectrum[dir]=valor;
	}



}


void poke_byte_zx80(z80_int dir,z80_byte valor)
{

	t_estados +=3;
	poke_byte_zx80_no_time(dir,valor);

}


/*
Gestionar direcciones shadow de RAM desde 2000H hasta 3FFFH

2000H-23FFH shadow de 2400H
2400H-27FFH 1K Ram

2800H-2BFFH Shadow de 2C00H
2C00H-2FFFH 1K Ram

3000H-33FFH Shadow de 3C00H
3400H-37FFH Shadow de 3C00H
3800H-3BFFH Shadow de 3C00H
3C00H-3FFFH 1K Ram

Y tambien validar que no se vaya mas alla de la ramtop

*/

z80_int jupiterace_adjust_memory_pointer(z80_int dir)
{
	if (dir>=0x2000 && dir<=0x23ff) {
		dir |=0x0400;
	}

	else if (dir>=0x2800 && dir<=0x2bff) {
		dir |=0x0400;
	}

	else if (dir>=0x3000 && dir<=0x3bff) {
		dir |=0x0C00;
	}


	//Casos ramtop:
	//3  KB. ramtop=16383=3FFH=00111111 11111111
	//19 KB. ramtop=32767=7FFH=01111111 11111111
	//35 KB. ramtop=49151=BFFH=10111111 11111111

	//Si hay 35 KB y se accede a direccion, por ejemplo, 65535, se convierte en: FFFF AND BFFH = BFFH
	//Si se accede a 49152 es: C000H AND BFFH = 8000H = 32768

        else {
		if (dir>ramtop_ace) dir = (dir&(ramtop_ace));
	}

	return dir;
}


void poke_byte_ace_no_time(z80_int dir,z80_byte valor)
{


	dir=jupiterace_adjust_memory_pointer(dir);


        if (dir>8191 && dir<=ramtop_ace) {
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif

                memoria_spectrum[dir]=valor;
        }


}


void poke_byte_ace(z80_int dir,z80_byte valor)
{

        t_estados +=3;
        poke_byte_ace_no_time(dir,valor);

}


z80_byte peek_byte_zx80_no_time(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

	//Si se pide por VRAM (c000-c3ffh)
	/*if (dir>0xc000 && dir<0xc3ff) {
		printf ("pidiendo por %04XH retornando %02XH\n",dir,memoria_spectrum[dir-32768]);
	}*/

	//Modulo RAM en 49152
	if (ram_in_49152.v==1 && dir>49151) {
		return memoria_spectrum[dir];
	}

	//Modulo RAM en 32768
	if (ram_in_32768.v==1 && dir>32767) {
                return memoria_spectrum[dir];
        }

	//Si se pide por VRAM (c000-c3ffh)
	/*if (dir>0xc000 && dir<0xc3ff) {
		printf ("pidiendo por %04XH retornando %02XH\n",dir,memoria_spectrum[dir-32768]);
		return memoria_spectrum[dir-32768];
	}*/

        if (dir>ramtop_zx8081) {
                dir = (dir&(ramtop_zx8081));
        }

	//Si ZXpand
	if (zxpand_enabled.v) {
		if (dir<8192) {
			if (zxpand_overlay_rom.v) return zxpand_memory_pointer[dir];
		}

		return memoria_spectrum[dir];
	}
        else return memoria_spectrum[dir];

	//return memoria_spectrum[dir];
}



z80_byte peek_byte_zx80(z80_int dir)
{

	t_estados +=3;

	return peek_byte_zx80_no_time(dir);

}

z80_byte peek_byte_ace_no_time(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

	dir=jupiterace_adjust_memory_pointer(dir);
        return memoria_spectrum[dir];

}


z80_byte peek_byte_ace(z80_int dir)
{

        t_estados +=3;

        return peek_byte_ace_no_time(dir);

}


z80_byte fetch_opcode_spectrum(void)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif
	return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_ace(void)
{
#ifdef EMULATE_VISUALMEM
        set_visualmemopcodebuffer(reg_pc);
#endif
        return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_cpc(void)
{
#ifdef EMULATE_VISUALMEM
        set_visualmemopcodebuffer(reg_pc);
#endif
        return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_sam(void)
{
#ifdef EMULATE_VISUALMEM
        set_visualmemopcodebuffer(reg_pc);
#endif
        return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_pcw(void)
{
    /*
#ifdef EMULATE_VISUALMEM
        set_visualmemopcodebuffer(reg_pc);
#endif
*/
//printf("antes %p\n",peek_byte_no_time);
z80_byte value= peek_byte_no_time (reg_pc);

        //printf("despues %p\n",peek_byte_no_time);
        return value;
}

z80_byte fetch_opcode_coleco(void)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif

	return peek_byte_no_time (reg_pc);
}


z80_byte fetch_opcode_sg1000(void)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif

	return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_sms(void)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif

	return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_msx(void)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif

	//sumar 1 t-estado, por wait en M1
	t_estados ++;

	return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_svi(void)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemopcodebuffer(reg_pc);
#endif

	//sumar 1 t-estado, por wait en M1
	t_estados ++;

	return peek_byte_no_time (reg_pc);
}

z80_byte fetch_opcode_zx81(void)
{
#ifdef EMULATE_VISUALMEM
    set_visualmemopcodebuffer(reg_pc);
#endif

	return fetch_opcode_zx81_graphics();

}

//Si dir>16383, leemos RAM. Sino, leemos ROM siempre que la ram oculta este oculta
z80_byte peek_byte_no_time_spectrum_inves(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	if (dir<16384) {
		if ( (zesarux_zxi_registers_array[0]&1)==0 ) {
			return memoria_spectrum[65536+dir];  //Mostrar ROM
		}
		else return memoria_spectrum[dir]; //Mostrar RAM baja
	}

	else return memoria_spectrum[dir];
}



z80_byte peek_byte_spectrum_inves(z80_int dir)
{
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                //printf ("%d\n",t_estados);
                t_estados += contend_table[ t_estados ];
        }
#endif

        t_estados +=3;


	return peek_byte_no_time_spectrum_inves(dir);

}



z80_byte peek_byte_no_time_spectrum_48k(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

        return memoria_spectrum[dir];
}


z80_byte peek_byte_spectrum_48k(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
		//printf ("%d\n",t_estados);
                t_estados += contend_table[ t_estados ];
        }
#endif

        t_estados +=3;


#ifdef DEBUG_SECOND_TRAP_STDOUT

	//Para hacer debug de aventuras de texto, investigar desde donde se estan leyendo las tablas de caracteres
	//hay que definir este DEBUG_SECOND_TRAP_STDOUT manualmente en compileoptions.h despues de ejecutar el configure
	scr_stdout_debug_char_table_routines_peek(dir);

#endif



	return memoria_spectrum[dir];

}


//Funcion usada en la emulacion de refresco de RAM
z80_byte peek_byte_spectrum_48k_refresh_memory(z80_int dir)
{
	        //emulacion refresco memoria superior en 48kb
                //si es memoria alta y ha llegado a la mitad del maximo, memoria ram inestable y retornamos valores indeterminados
                if (machine_emulate_memory_refresh_counter>=MAX_EMULATE_MEMORY_REFRESH_LIMIT) {
                        if (dir>32767) {
				//Alteramos el valor enviandolo a 255
				z80_byte c=memoria_spectrum[dir];
				if (c<255) {
					c++;
					memoria_spectrum[dir]=c;
                                	debug_printf (VERBOSE_DEBUG,"RAM is bad, altering byte value at address: %d",dir);
				}


                                //valor ligeramente aleatorio->invertimos valor y metemos bit 1 a 1
                                //printf ("devolvemos valor ram sin refrescar dir: %d\n",dir);
                                //return (memoria_spectrum[dir] ^ 255) | 1;
                        }
                }

	return peek_byte_spectrum_48k(dir);
}

//Puntero a la funcion original
z80_byte (*peek_byte_no_ram_refresh)(z80_int dir);


void set_peek_byte_function_ram_refresh(void)
{

        debug_printf(VERBOSE_INFO,"Enabling RAM refresh on peek_byte");

        peek_byte_no_ram_refresh=peek_byte;
        peek_byte=peek_byte_spectrum_48k_refresh_memory;

}

void reset_peek_byte_function_ram_refresh(void)
{
	debug_printf(VERBOSE_INFO,"Setting normal peek_byte");
        peek_byte=peek_byte_no_ram_refresh;
}


z80_byte peek_byte_no_time_spectrum_128k(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

	dir = dir & 16383;
	puntero=memory_paged[segmento]+dir;
//		printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
	return *puntero;
}


z80_byte peek_byte_spectrum_128k(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;



	dir = dir & 16383;
	puntero=memory_paged[segmento]+dir;
//              printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
	return *puntero;
}




z80_byte peek_byte_no_time_spectrum_128kp2a(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

	dir = dir & 16383;
	puntero=memory_paged[segmento]+dir;
//      printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
	return *puntero;
}

z80_byte peek_byte_spectrum_128kp2a(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

	int segmento;
	z80_byte *puntero;
	segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;


	dir = dir & 16383;
	puntero=memory_paged[segmento]+dir;
//              printf ("segmento: %d dir: %d puntero: %p\n",segmento,dir,puntero);
	return *puntero;

}



z80_byte peek_byte_no_time_spectrum_16k(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

    if (dir<32768) return memoria_spectrum[dir];
	else return idle_bus_port(255); //No hay memoria alli, devolvemos lo que la ula ha dejado en el bus
}

z80_byte peek_byte_spectrum_16k(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

#ifdef EMULATE_CONTEND
    if ( (dir&49152)==16384) {
            t_estados += contend_table[ t_estados ];
    }
#endif

    t_estados += 3;


    if (dir<32768) return memoria_spectrum[dir];
    else return idle_bus_port(255); //No hay memoria alli, devolvemos lo que la ula ha dejado en el bus
}

z80_byte *zxuno_return_segment_memory(z80_int dir)
{
	int segmento;
	z80_byte *puntero;

	segmento=dir/8192;
	puntero=zxuno_memory_paged_brandnew[segmento];
	return puntero;
}

void poke_byte_no_time_zxuno(z80_int dir,z80_byte valor)
{
	z80_byte *puntero;

	puntero=zxuno_return_segment_memory(dir);

	//Modo BOOTM

	if (ZXUNO_BOOTM_ENABLED) {
		//Si no es rom
		if (dir>16383) {
			//printf ("Poke bootm %X %X\n",dir,valor);
			dir = dir & 8191;

			puntero=puntero+dir;
			*puntero=valor;
		}
	}

	else {
		//Modo no bootm. como un +2a

		if (dir>16383 || (puerto_8189&1)==1 ) {  //Si en ram, o rom con page RAM in ROM
#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
			dir = dir & 8191;
			puntero=puntero+dir;
			*puntero=valor;
		}


	}

}



void poke_byte_zxuno(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;

	poke_byte_no_time_zxuno(dir,valor);
}




z80_byte peek_byte_no_time_zxuno(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

        //int segmento;
        z80_byte *puntero;

				puntero=zxuno_return_segment_memory(dir);

				dir = dir & 8191;
				puntero=puntero+dir;

				return *puntero;


}

z80_byte peek_byte_zxuno(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;

	return peek_byte_no_time_zxuno(dir);

}



void poke_byte_no_time_msx1(z80_int dir,z80_byte valor)
{


	z80_byte *puntero_memoria;
	int tipo;

	puntero_memoria=msx_return_segment_address(dir,&tipo);

    //Si hay automapper...
    if (tipo==MSX_SLOT_MEMORY_TYPE_ROM && msx_mapper_type!=MSX_MAPPER_TYPE_NONE) {
        if (msx_mapper_type==MSX_MAPPER_TYPE_ASCII_16KB) {
            //ascii 16kb
            /*
            And the address to change banks:

	Bank 1: 6000h - 67FFh (6000h used)
	Bank 2: 7000h - 77FFh (7000h and 77FFh used)
            */
           if (dir>=0x6000 && dir<=0x67ff) {
                //printf("Mapping Segment %d on Bank 1\n",valor);
                msx_mapper_rom_cartridge_pages[0]=valor;
           }

           if (dir>=0x7000 && dir<=0x77ff) {
                //printf("Mapping Segment %d on Bank 2\n",valor);
                msx_mapper_rom_cartridge_pages[1]=valor;
           }
        }

        if (msx_mapper_type==MSX_MAPPER_TYPE_ASCII_8KB) {
            //ascii 8kb
            /*
	Bank 1: 6000h - 67FFh (6000h used)
	Bank 2: 6800h - 6FFFh (6800h used)
	Bank 3: 7000h - 77FFh (7000h used)
	Bank 4: 7800h - 7FFFh (7800h used)
            */
           if (dir>=0x6000 && dir<=0x67ff) {
                //printf("Mapping Segment %d on Bank 1\n",valor);
                msx_mapper_rom_cartridge_pages[0]=valor;
           }

           if (dir>=0x6800 && dir<=0x6FFF) {
                //printf("Mapping Segment %d on Bank 2\n",valor);
                msx_mapper_rom_cartridge_pages[1]=valor;
           }

           if (dir>=0x7000 && dir<=0x77ff) {
                //printf("Mapping Segment %d on Bank 3\n",valor);
                msx_mapper_rom_cartridge_pages[2]=valor;
           }

           if (dir>=0x7800 && dir<=0x7FFF) {
                //printf("Mapping Segment %d on Bank 4\n",valor);
                msx_mapper_rom_cartridge_pages[3]=valor;
           }
        }

        if (msx_mapper_type==MSX_MAPPER_TYPE_KONAMI_MEGAROM_WITHOUT_SCC) {
            /*
And the address to change banks:

	Bank 1: <none>
	Bank 2: 6000h - 7FFFh (6000h used)
	Bank 3: 8000h - 9FFFh (8000h used)
	Bank 4: A000h - BFFFh (A000h used)
            */
           /*if (dir>=0x6000 && dir<=0x67ff) {
                //printf("Mapping Segment %d on Bank 1\n",valor);
                msx_mapper_rom_cartridge_pages[0]=valor;
           }*/

           if (dir>=0x6000 && dir<=0x7FFF) {
                //printf("Mapping Segment %d on Bank 2\n",valor);
                msx_mapper_rom_cartridge_pages[1]=valor;
           }

           if (dir>=0x8000 && dir<=0x9fff) {
                //printf("Mapping Segment %d on Bank 3\n",valor);
                msx_mapper_rom_cartridge_pages[2]=valor;
           }

           if (dir>=0xa000 && dir<=0xbFFF) {
                //printf("Mapping Segment %d on Bank 4\n",valor);
                msx_mapper_rom_cartridge_pages[3]=valor;
           }
        }

    if (msx_mapper_type==MSX_MAPPER_TYPE_KONAMI_MEGAROM_WITH_SCC) {
            /*
And the address to change banks:

	Bank 1: 5000h - 57FFh (5000h used)
	Bank 2: 7000h - 77FFh (7000h used)
	Bank 3: 9000h - 97FFh (9000h used)
	Bank 4: B000h - B7FFh (B000h used)
            */
           if (dir>=0x5000 && dir<=0x57ff) {
                //printf("Mapping Segment %d on Bank 1\n",valor);
                msx_mapper_rom_cartridge_pages[0]=valor;
           }

           if (dir>=0x7000 && dir<=0x77FF) {
                //printf("Mapping Segment %d on Bank 2\n",valor);
                msx_mapper_rom_cartridge_pages[1]=valor;
           }

           if (dir>=0x9000 && dir<=0x97ff) {
                //printf("Mapping Segment %d on Bank 3\n",valor);
                msx_mapper_rom_cartridge_pages[2]=valor;
           }

           if (dir>=0xb000 && dir<=0xb7FF) {
                //printf("Mapping Segment %d on Bank 4\n",valor);
                msx_mapper_rom_cartridge_pages[3]=valor;
           }
        }

        if (msx_mapper_type==MSX_MAPPER_TYPE_RTYPE) {

            /*
            And the address to change banks:

	Bank 1: Fixed at 0Fh or 17h
	Bank 2: 7000h - 7FFFh (7000h and 7800h used)
            */
           /*if (dir>=0x6000 && dir<=0x67ff) {
                //printf("Mapping Segment %d on Bank 1\n",valor);
                msx_mapper_rom_cartridge_pages[0]=valor;
           }*/

           if (dir>=0x7000 && dir<=0x77ff) {
                //printf("Mapping Segment %d on Bank 2\n",valor);
                msx_mapper_rom_cartridge_pages[1]=valor;
           }
        }
    }


	//Si esta vacio o es ROM, no hacer nada. O sea, si no es RAM
	if (tipo!=MSX_SLOT_MEMORY_TYPE_RAM) return;

	else {

#ifdef EMULATE_VISUALMEM

		set_visualmembuffer(dir);

#endif

		*puntero_memoria=valor;

	}


}

void poke_byte_msx1(z80_int dir,z80_byte valor)
{
/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

	//Y sumamos estados normales
	t_estados += 3;


	poke_byte_no_time_msx1(dir,valor);
}




z80_byte peek_byte_no_time_msx1(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

		//z80_byte *msx_return_segment_address(z80_int direccion,int *tipo)

		z80_byte *puntero_memoria;
		int tipo;

		puntero_memoria=msx_return_segment_address(dir,&tipo);

		//Si esta vacio, retornar 0 o 255???
		if (tipo==MSX_SLOT_MEMORY_TYPE_EMPTY) return 255;

        else return *puntero_memoria;
}


z80_byte peek_byte_msx1(z80_int dir)
{

/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
		//printf ("%d\n",t_estados);
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

        t_estados +=3;





	return peek_byte_no_time_msx1(dir);

}



void poke_byte_no_time_svi(z80_int dir,z80_byte valor)
{


	z80_byte *puntero_memoria;
	int tipo;

	puntero_memoria=svi_return_segment_address(dir,&tipo);

	//Si esta vacio o es ROM, no hacer nada. O sea, si no es RAM
	if (tipo!=SVI_SLOT_MEMORY_TYPE_RAM) return;

	else {

#ifdef EMULATE_VISUALMEM

		set_visualmembuffer(dir);

#endif

		*puntero_memoria=valor;

	}


}

void poke_byte_svi(z80_int dir,z80_byte valor)
{
/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

	//Y sumamos estados normales
	t_estados += 3;


	poke_byte_no_time_svi(dir,valor);
}




z80_byte peek_byte_no_time_svi(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

		//z80_byte *svi_return_segment_address(z80_int direccion,int *tipo)

		z80_byte *puntero_memoria;
		int tipo;

		puntero_memoria=svi_return_segment_address(dir,&tipo);

		//Si esta vacio, retornar 0 o 255???
		if (tipo==SVI_SLOT_MEMORY_TYPE_EMPTY) return 255;

        else return *puntero_memoria;
}


z80_byte peek_byte_svi(z80_int dir)
{

/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
		//printf ("%d\n",t_estados);
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

        t_estados +=3;





	return peek_byte_no_time_svi(dir);

}







void poke_byte_no_time_coleco(z80_int dir,z80_byte valor)
{


	z80_byte *puntero_memoria;
	int tipo;

	puntero_memoria=coleco_return_segment_address(dir,&tipo);

	//Si esta vacio o es ROM, no hacer nada. O sea, si no es RAM
	if (tipo!=COLECO_SLOT_MEMORY_TYPE_RAM) return;

	else {

#ifdef EMULATE_VISUALMEM

		set_visualmembuffer(dir);

#endif

		*puntero_memoria=valor;

	}


}

void poke_byte_coleco(z80_int dir,z80_byte valor)
{
/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

	//Y sumamos estados normales
	t_estados += 3;


	poke_byte_no_time_coleco(dir,valor);
}




z80_byte peek_byte_no_time_coleco(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

		//z80_byte *coleco_return_segment_address(z80_int direccion,int *tipo)

		z80_byte *puntero_memoria;
		int tipo;

		puntero_memoria=coleco_return_segment_address(dir,&tipo);

		//Si esta vacio, retornar 0 o 255???
		if (tipo==COLECO_SLOT_MEMORY_TYPE_EMPTY) return 255;

        else return *puntero_memoria;
}


z80_byte peek_byte_coleco(z80_int dir)
{

/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
		//printf ("%d\n",t_estados);
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

        t_estados +=3;





	return peek_byte_no_time_coleco(dir);

}



void poke_byte_no_time_sg1000(z80_int dir,z80_byte valor)
{


	z80_byte *puntero_memoria;
	int tipo;

	puntero_memoria=sg1000_return_segment_address(dir,&tipo);

	//Si esta vacio o es ROM, no hacer nada. O sea, si no es RAM
	if (tipo!=SG1000_SLOT_MEMORY_TYPE_RAM) return;

	else {

#ifdef EMULATE_VISUALMEM

		set_visualmembuffer(dir);

#endif

		*puntero_memoria=valor;

	}


}

void poke_byte_sg1000(z80_int dir,z80_byte valor)
{
/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

	//Y sumamos estados normales
	t_estados += 3;


	poke_byte_no_time_sg1000(dir,valor);
}




z80_byte peek_byte_no_time_sg1000(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

		//z80_byte *sg1000_return_segment_address(z80_int direccion,int *tipo)

		z80_byte *puntero_memoria;
		int tipo;

		puntero_memoria=sg1000_return_segment_address(dir,&tipo);

		//Si esta vacio, retornar 0 o 255???
		if (tipo==SG1000_SLOT_MEMORY_TYPE_EMPTY) return 255;

        else return *puntero_memoria;
}


z80_byte peek_byte_sg1000(z80_int dir)
{

/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
		//printf ("%d\n",t_estados);
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

        t_estados +=3;





	return peek_byte_no_time_sg1000(dir);

}


void poke_byte_no_time_sms(z80_int dir,z80_byte valor)
{


	z80_byte *puntero_memoria;
	int tipo;

    //Si actua sobre mappers sega
    if (sms_mapper_type==SMS_MAPPER_TYPE_SEGA) {
        if (dir>=0xFFFC) {
            //printf("Writing on sms mapper %X value %x\n",dir,valor);
            switch (dir) {
                case 0xFFFC:
                    sms_mapper_FFFC=valor;
                    //printf("Writing on sms mapper %X value %x\n",dir,valor);
                break;

                case 0xFFFD:
                    sms_mapper_FFFD=valor;
                break;

                case 0xFFFE:
                    sms_mapper_FFFE=valor;
                break;

                case 0xFFFF:
                    sms_mapper_FFFF=valor;
                break;


            }
        }
    }

    //Si actua sobre mappers code masters
    if (sms_mapper_type==SMS_MAPPER_TYPE_CODEMASTERS) {
        //printf("Writing on sms mapper %X value %x\n",dir,valor);
        switch (dir) {
            case 0x0000:
                sms_mapper_FFFD=valor;
            break;

            case 0x4000:
                sms_mapper_FFFE=valor;
            break;

            case 0x8000:
                sms_mapper_FFFF=valor;
            break;


        }

    }

	puntero_memoria=sms_return_segment_address(dir,&tipo);

	//Si esta vacio o es ROM, no hacer nada. O sea, si no es RAM
	if (tipo!=SMS_SLOT_MEMORY_TYPE_RAM) return;

	else {

#ifdef EMULATE_VISUALMEM

		set_visualmembuffer(dir);

#endif

		*puntero_memoria=valor;

	}


}

void poke_byte_sms(z80_int dir,z80_byte valor)
{
/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

	//Y sumamos estados normales
	t_estados += 3;


	poke_byte_no_time_sms(dir,valor);
}




z80_byte peek_byte_no_time_sms(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif

		//z80_byte *sms_return_segment_address(z80_int direccion,int *tipo)
    //Si actua sobre mappers sega
    if (sms_mapper_type==SMS_MAPPER_TYPE_SEGA) {
        if (dir>=0xFFFC) {
            //printf("Reading from sms mapper %X\n",dir);

            switch (dir) {
                case 0xFFFC:
                    return sms_mapper_FFFC;
                break;

                case 0xFFFD:
                    return sms_mapper_FFFD;
                break;

                case 0xFFFE:
                    return sms_mapper_FFFE;
                break;

                case 0xFFFF:
                    return sms_mapper_FFFF;
                break;


            }

        }
    }
		z80_byte *puntero_memoria;
		int tipo;

		puntero_memoria=sms_return_segment_address(dir,&tipo);

		//Si esta vacio, retornar 0 o 255???
		if (tipo==SMS_SLOT_MEMORY_TYPE_EMPTY) return 255;

        else return *puntero_memoria;
}


z80_byte peek_byte_sms(z80_int dir)
{

/*
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
		//printf ("%d\n",t_estados);
                t_estados += contend_table[ t_estados ];
        }
#endif
*/

        t_estados +=3;





	return peek_byte_no_time_sms(dir);

}





z80_byte *chloe_return_segment_memory(z80_int dir)
{
	int segmento;
	z80_byte *puntero;

	segmento=dir/8192;
	puntero=chloe_memory_paged[segmento];
	return puntero;
}


void poke_byte_no_time_chloe(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif


		z80_byte *puntero;
		puntero=chloe_return_segment_memory(dir);

		//proteccion rom
		////Constantes definidas en CHLOE_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX
		//z80_byte chloe_type_memory_paged[8];
		if (dir<16384) {
			//Segmento 0 o 1
			int segmento=dir/8192;
			if (chloe_type_memory_paged[segmento]==CHLOE_MEMORY_TYPE_ROM) return;
		}

		dir = dir & 8191;

		puntero=puntero+dir;
		*puntero=valor;

}

void poke_byte_chloe(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];

                }
#endif
                t_estados += 3;

        poke_byte_no_time_chloe(dir,valor);
}



z80_byte peek_byte_no_time_chloe(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

		z80_byte *puntero;
		puntero=chloe_return_segment_memory(dir);

		dir = dir & 8191;

		puntero=puntero+dir;
		return *puntero;
}


z80_byte peek_byte_chloe(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;

        return peek_byte_no_time_chloe(dir);

}



z80_byte *prism_return_segment_memory(z80_int dir)
{
	int segmento;
	z80_byte *puntero;

	segmento=dir/8192;

	//Caso ram en rom del +2A
	if (puerto_8189 & 1) {
                       z80_byte page_type;
                        page_type=(puerto_8189 >>1) & 3;
			if (segmento==2 || segmento==3) {
                        //En este modo no se gestiona vram aperture


                        //printf ("prism_return_segment_memory con ram in rom. Segmento: %d dir: %d\n",segmento,dir);

                                //debug_printf (VERBOSE_DEBUG,"Pages 0,1,2,3");
                                //debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");
                                //debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,3");
                                //debug_printf (VERBOSE_DEBUG,"Pages 4,7,6,3");
                        switch (page_type) {
                                case 0:
                                        //Pagina 16 KB RAM 1. SRAM en PRISM es 2,3 (8 KB)
                                        return prism_ram_mem_table[segmento];
                                break;

                                case 1:
                                case 2:
                                        //Pagina RAM 5 (VRAM 0,1)
                                        //No hacer nada. Pagina 5 igual que modo estandard.
                                        return prism_vram_mem_table[segmento-2]; //Retornar vrams 0 y 1
                                break;
				case 3:
                                        //Pagina RAM 7 (VRAM 2,3)
                                        return prism_vram_mem_table[segmento]; //Retornar vrams 2 y 3
                                break;
                        }
							}

							if (segmento==6 || segmento==7) {
								//caso modo paginación 1 //debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");
								if (page_type==1) return prism_vram_mem_table[segmento-4]; //VRAM 2,3
							}
                 }






	//Caso segmento 2,3 (4000H-7FFH). Aqui sin vram aperture:
	//Segmento 2:
	//4000-5affh: vram0
	//5b00-5fffh: sram 10
	//Segmento 3
	//6000-7fffh: sram 11
	//TODO vram aperture
	if (segmento==2 || segmento==3) {
		//Esto solo sucede si se mapea HOME RAM (y no DOCK ni EX)
		z80_byte ram_page_at_seg_2=10;
		z80_byte vram_at_seg_2=0;



		if (prism_type_memory_paged[segmento]==PRISM_MEMORY_TYPE_HOME) {
			//ULA2 registro 1
			//xxx1 - Map VRAM0 at 0x4000 and VRAM1 at 0x6000

			//CON vram aperture
			if (prism_ula2_registers[1] & 1) {
				return prism_vram_mem_table[vram_at_seg_2+segmento-2]; //Retornar vrams 0 y 1
			}

			else {

				//SIN vram aperture
				if (segmento==2) {
					//Entre 4000-5affh
					if (dir>=0x4000 && dir<=0x5aff) {
						return prism_vram_mem_table[vram_at_seg_2];
					}

					else {
						//Entre 5b00h-5fffh
						return prism_ram_mem_table[ram_page_at_seg_2];
					}
				}

				if (segmento==3) return prism_ram_mem_table[ram_page_at_seg_2+1];
			}
		}
	}

	puntero=prism_memory_paged[segmento];
	return puntero;
}


void poke_byte_no_time_prism(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
		z80_byte *puntero;
		puntero=prism_return_segment_memory(dir);

		//proteccion rom
		////Constantes definidas en PRISM_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX
		//z80_byte prism_type_memory_paged[8];
		if (dir<16384) {
			//Segmento 0 o 1
			int segmento=dir/8192;
			if (prism_type_memory_paged[segmento]==PRISM_MEMORY_TYPE_ROM) return;
		}


		//Caso de write mask video
		if (dir>=0x4000 && dir<=0x5aff) {
			dir = dir & 8191;
			z80_byte mascara=prism_ula2_registers[5];
			if (mascara==0) {
				puntero=puntero+dir;
	                        *puntero=valor;
			}
			else {
				z80_byte *screen;
				if (mascara&8) {
					screen=prism_vram_mem_table[3];
					screen[dir]=valor;
				}
				if (mascara&4) {
					screen=prism_vram_mem_table[2];
					screen[dir]=valor;
				}
				if (mascara&2) {
					screen=prism_vram_mem_table[1];
					screen[dir]=valor;
				}
				if (mascara&1) {
					screen=prism_vram_mem_table[0];
					screen[dir]=valor;
				}
			}
		}

		else {
			dir = dir & 8191;
			puntero=puntero+dir;
			*puntero=valor;
		}

}

void poke_byte_prism(z80_int dir,z80_byte valor)
{

                t_estados += 3;

        poke_byte_no_time_prism(dir,valor);
}



z80_byte peek_byte_no_time_prism(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

		z80_byte *puntero;
		puntero=prism_return_segment_memory(dir);

		dir = dir & 8191;

		puntero=puntero+dir;
		return *puntero;
}


z80_byte peek_byte_prism(z80_int dir)
{

                t_estados += 3;

        return peek_byte_no_time_prism(dir);

}


z80_byte *tbblue_return_segment_memory(z80_int dir)
{
	int segmento;

	segmento=dir/8192;

	return tbblue_memory_paged[segmento];

}



z80_byte *tbblue_get_altrom_dir(z80_int dir)
{
	/*
	   -- 0x018000 - 0x01BFFF (16K)  => Alt ROM0 128k           A20:A16 = 00001,10
   -- 0x01c000 - 0x01FFFF (16K)  => Alt ROM1 48k            A20:A16 = 00001,11
	*/

/*
0x8C (140) => Alternate ROM
(R/W) (hard reset = 0)
IMMEDIATE
  bit 7 = 1 to enable alt rom
  bit 6 = 1 to make alt rom visible only during writes, otherwise replaces rom during reads
  bit 5 = 1 to lock ROM1 (48K rom)
  bit 4 = 1 to lock ROM0 (128K rom)
*/

	int puntero;

	//Ver si es rom 0 o rom 1
	int altrom;

	altrom=tbblue_get_altrom();

	//if (dir<2048) printf("tbblue_get_altrom_dir. altrom=%d\n",altrom);

	puntero=tbblue_get_altrom_offset_dir(altrom,dir&16383);

	return &memoria_spectrum[puntero];

}

int tbblue_get_total_offset_layer2(void)
{
    int offset=tbblue_get_offset_start_layer2();



        //Bit 2-0	16ki bank relative offset (+0 .. +7) applied to Layer 2 memory mapping
        z80_byte offset_16k=tbblue_port_123b_second_byte&7;
        //printf("offset: %d\n",offset_16k);
        offset +=offset_16k*16384;
        //printf("offset: %d\n",offset);




        //printf("tbblue: %d\n",tbblue_port_123b);



        z80_byte region=tbblue_port_123b&(64+128);
        switch (region) {
            case 64:
                offset +=16384;
            break;

            case 128: //TODO: en la documentacion dice 192... tiene logica???
                offset +=32768;
            break;
        }




    return offset;
}

void poke_byte_no_time_tbblue(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif

	//Altrom. Si escribe en espacio de memoria de rom 0-3fffh
	if (dir<16384 && (  (tbblue_registers[0x8c] & 192) ==192)   ) {
		/*
		0x8C (140) => Alternate ROM
(R/W) (hard reset = 0)
IMMEDIATE
  bit 7 = 1 to enable alt rom
  bit 6 = 1 to make alt rom visible only during writes, otherwise replaces rom during reads

  //bit 6 =0 , only for read. bit 6=1, only for write
  */

		//printf ("Escribiendo en altrom dir: %04XH valor : %02XH  PC=%04XH diviface control: %d active: %d\n",dir,valor,reg_pc,
		//		        diviface_control_register&128, diviface_paginacion_automatica_activa.v);


		int escribir=1;

		if (! (
				(diviface_control_register&128)==0 && diviface_paginacion_automatica_activa.v==0)
		      )
		{
			escribir=0;
			//printf ("No escribimos pues esta diviface ram conmutada\n");
        }


		//Y escribimos
		if (escribir) {
			z80_byte *altrompointer;

			altrompointer=tbblue_get_altrom_dir(dir);
			*altrompointer=valor;
		}
	}




    //Si se escribe en memoria layer2
    if (dir<tbblue_layer2_size_mapped() && tbblue_write_on_layer2() ) {

        //Pero si no hay por encima la memoria divmmc encima:
        /*
            -- memory decode order
--
-- 0-16k:
--   1. bootrom
--   2. machine config mapping
--   3. multiface
--   4. divmmc
--   5. layer 2 mapping
--   6. mmu
--   7. romcs expansion bus
--   8. rom
        */

        int escribir=1;


        if (tbblue_layer2_size_mapped()==16384) {
            if ((diviface_control_register&128) || diviface_paginacion_automatica_activa.v==1) {
                //printf("no escribir pues divmmc activo. dir: %d\n",dir);
                escribir=0;
            }
        }



        if (escribir) {

            int offset=tbblue_get_total_offset_layer2();


            offset +=dir;
            memoria_spectrum[offset]=valor;

            //Y volvemos pues no hay que escribir en otras rams que estén mapeadas aquí
            return;

        }

        //printf ("Escribiendo layer 2 direccion %d valor %d offset %d region %d\n",dir,valor,offset,region);
    }

    //Si se puede escribir en espacio ROM (0-16383)
    if (dir<16384) {
        if (tbblue_low_segment_writable.v==0) {
            //Aqui puede pasar que por MMU si se permita (registro 80/81 a valor no 255)
            //printf ("No se puede escribir en la rom. Dir=%d PC=%d\n",dir,reg_pc);
            if (!tbblue_is_writable_segment_mmu_rom_space(dir) ) return;
        }
    }

    //if (dir<16384) printf ("Writing on tbblue rom address %XH value %XH\n",dir,valor);

    z80_byte *puntero;
    puntero=tbblue_return_segment_memory(dir);

    dir = dir & 8191;
    puntero=puntero+dir;

    *puntero=valor;

}


void poke_byte_tbblue(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif


                t_estados += 3;

        poke_byte_no_time_tbblue(dir,valor);
}



z80_byte peek_byte_no_time_tbblue(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

    //Si esta layer2 visible para lectura
    if (dir<tbblue_layer2_size_mapped() && tbblue_read_on_layer2() ) {
        int offset=tbblue_get_total_offset_layer2();

        offset +=dir;
        return memoria_spectrum[offset];
    }

		z80_byte *puntero;
		puntero=tbblue_return_segment_memory(dir);

		//printf ("Returning tbblue memory address %X\n",dir);

		dir = dir & 8191;
		puntero=puntero+dir;

		//printf ("Returning tbblue memory value %X\n",*puntero);

		return *puntero;
}


z80_byte peek_byte_tbblue(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif

                t_estados += 3;

        return peek_byte_no_time_tbblue(dir);

}




z80_byte *chrome_return_segment_memory(z80_int dir)
{
	int segmento;

	segmento=dir/16384;

	return chrome_memory_paged[segmento];

}


void poke_byte_no_time_chrome(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif

		z80_byte *puntero;
		puntero=chrome_return_segment_memory(dir);

		if (dir<16384) {
			if (!chrome_ram89_at_00()) return; //No esta ram 8 o 9 en zona 0000-3fff. volver
		}

		dir = dir & 16383;
		puntero=puntero+dir;

		*puntero=valor;

}

void poke_byte_chrome(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif


                t_estados += 3;

        poke_byte_no_time_chrome(dir,valor);
}



z80_byte peek_byte_no_time_chrome(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

		z80_byte *puntero;
		puntero=chrome_return_segment_memory(dir);

		dir = dir & 16383;
		puntero=puntero+dir;

		return *puntero;
}


z80_byte peek_byte_chrome(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif

                t_estados += 3;

        return peek_byte_no_time_chrome(dir);

}


z80_byte *baseconf_return_segment_memory(z80_int dir)
{
	int segmento;

	segmento=dir/16384;

	return baseconf_memory_paged[segmento];

}


void poke_byte_no_time_baseconf(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM
	set_visualmembuffer(dir);
#endif

		z80_byte *puntero;
		puntero=baseconf_return_segment_memory(dir);


		//if (dir<16384) {
	//return;

			if (baseconf_memory_segments_type[dir/16384]==0) {
				//0 rom
				return;
			}
		//}

		dir = dir & 16383;
		puntero=puntero+dir;

		*puntero=valor;

}

void poke_byte_baseconf(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif


                t_estados += 3;

        poke_byte_no_time_baseconf(dir,valor);
}



z80_byte peek_byte_no_time_baseconf(z80_int dir)
{

	lee_byte_evo_aux(dir);

	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

		z80_byte *puntero;
		puntero=baseconf_return_segment_memory(dir);

		dir = dir & 16383;
		puntero=puntero+dir;

		return *puntero;
}


z80_byte peek_byte_baseconf(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif

                t_estados += 3;

        return peek_byte_no_time_baseconf(dir);

}







z80_byte *tsconf_return_segment_memory(z80_int dir)
{
	int segmento;

	segmento=dir/16384;

	return tsconf_memory_paged[segmento];

}


void poke_byte_no_time_tsconf(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM
	set_visualmembuffer(dir);
#endif

		z80_byte *puntero;
		puntero=tsconf_return_segment_memory(dir);


		//Si se escribe en fmaps
		if ((tsconf_af_ports[0x15]&16)!=0) {

			z80_int tsconf_fmaps_start=tsconf_af_ports[0x15]&0xF;
			tsconf_fmaps_start=tsconf_fmaps_start<<12;

			//Si escribe en zona fmaps, y permitir 256 mas para escritura en tsconf registros
			z80_int tsconf_fmaps_end=tsconf_fmaps_start+TSCONF_FMAPS_SIZE+256-1;

			if (dir>=tsconf_fmaps_start && dir<=tsconf_fmaps_end) {

				z80_int tsconf_fmaps_offset=dir-tsconf_fmaps_start;
				//printf ("Escribiendo fmaps dir: %04XH valor: %02XH offset: %d\n",dir,valor,tsconf_fmaps_offset);

				tsconf_write_fmaps(tsconf_fmaps_offset,valor);




			}


		}

		if (dir<16384) {


			if ((tsconf_get_memconfig()&8)==0) return; //si no ram en rom
		}

		dir = dir & 16383;
		puntero=puntero+dir;

		*puntero=valor;

}

void poke_byte_tsconf(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif


                t_estados += 3;

        poke_byte_no_time_tsconf(dir,valor);
}



z80_byte peek_byte_no_time_tsconf(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

		z80_byte *puntero;
		puntero=tsconf_return_segment_memory(dir);

		dir = dir & 16383;
		puntero=puntero+dir;

		return *puntero;
}


z80_byte peek_byte_tsconf(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif

                t_estados += 3;

        return peek_byte_no_time_tsconf(dir);

}



void poke_byte_no_time_mk14(z80_int dir,z80_byte valor)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmembuffer(dir);
	#endif

	z80_int zona=dir & 0x0F00;
	if (zona==0x900 || zona==0xD00)  {
			//I/O en 900H or D00H
			mk14_write_io_port(dir,valor);
			return;
	}

		//De momento hacer que 0-7FF es rom
		if (dir<0x800) return;
    memoria_spectrum[dir]=valor;

}

void poke_byte_mk14(z80_int dir,z80_byte valor)
{

        poke_byte_no_time_mk14(dir,valor);
}



z80_byte peek_byte_no_time_mk14(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

		z80_int zona=dir & 0x0F00;
		if (zona==0x900 || zona==0xD00)  {
				//I/O en 900H or D00H
				return mk14_get_io_port(dir);
		}
    return memoria_spectrum[dir];

}

z80_byte peek_byte_mk14(z80_int dir)
{
        return peek_byte_no_time_mk14(dir);

}


z80_byte lee_puerto_legacy_mk14(z80_byte h GCC_UNUSED,z80_byte l GCC_UNUSED)
{
        debug_printf(VERBOSE_ERR,"Calling lee_puerto function on a MK14 machine. TODO fix it!");
        return 0;
}

z80_byte *timex_return_segment_memory(z80_int dir)
{
        int segmento;
        z80_byte *puntero;

        segmento=dir/8192;
        puntero=timex_memory_paged[segmento];
        return puntero;
}


void poke_byte_no_time_timex(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif
                z80_byte *puntero;
                puntero=timex_return_segment_memory(dir);

                //proteccion rom
                ////Constantes definidas en TIMEX_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX
                //z80_byte timex_type_memory_paged[8];
                if (dir<16384) {
                        //Segmento 0 o 1
                        int segmento=dir/8192;
			if (timex_type_memory_paged[segmento]!=TIMEX_MEMORY_TYPE_HOME) return;
                }

                dir = dir & 8191;

                puntero=puntero+dir;
                *puntero=valor;

}


void poke_byte_timex(z80_int dir,z80_byte valor)
{

//TODO. tabla contend de timex la hacemos que funcione como spectrum 48k: dir<32768, contend
//pese a que haya ex o dock mapeado
#ifdef EMULATE_CONTEND
        if ( (dir&49152)==16384) {
                t_estados += contend_table[ t_estados ];
        }
#endif

                t_estados += 3;

        poke_byte_no_time_timex(dir,valor);
}



z80_byte peek_byte_no_time_timex(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

                z80_byte *puntero;
                puntero=timex_return_segment_memory(dir);

                dir = dir & 8191;

                puntero=puntero+dir;
                return *puntero;
}


z80_byte peek_byte_timex(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;

        return peek_byte_no_time_timex(dir);

}



z80_byte *cpc_return_segment_memory_read(z80_int dir)
{
        int segmento;
        z80_byte *puntero;

        segmento=dir/16384;
        puntero=cpc_memory_paged_read[segmento];
        return puntero;
}

z80_byte *cpc_return_segment_memory_write(z80_int dir)
{
        int segmento;
        z80_byte *puntero;

        segmento=dir/16384;
        puntero=cpc_memory_paged_write[segmento];
        return puntero;
}

void poke_byte_no_time_cpc(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif

                z80_byte *puntero;
                puntero=cpc_return_segment_memory_write(dir);

                dir = dir & 16383;

                puntero=puntero+dir;
                *puntero=valor;

}

void poke_byte_cpc(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];

                }
#endif
                t_estados += 3;

        poke_byte_no_time_cpc(dir,valor);
}


z80_byte peek_byte_no_time_cpc(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

                z80_byte *puntero;
                puntero=cpc_return_segment_memory_read(dir);

                dir = dir & 16383;

                puntero=puntero+dir;
                return *puntero;
}


z80_byte peek_byte_cpc(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;

        return peek_byte_no_time_cpc(dir);

}



z80_byte lee_puerto_cpc_no_time(z80_byte puerto_h,z80_byte puerto_l GCC_UNUSED)
{

	debug_fired_in=1;

    z80_int puerto=value_8_to_16(puerto_h,puerto_l);

	//z80_int puerto=(puerto_h<<8)||puerto_l;
	//if (puerto==0xFA7E || puerto==0xFB7E || puerto==0xFB7F) printf ("Puerto FDC\n");

	//Controladora 8255 PPI
	if ((puerto_h & 8)==0) {
		//printf ("Leyendo puerto cpc %02XH\n",puerto_h);
        //sleep(2);
		return cpc_in_ppi(puerto_h);
	}

	//printf ("Returning unused cpc port %02X%02XH\n",puerto_h,puerto_l);

    //Puertos PD765
    /*
    Port FA7Eh - Floppy Motor On/Off Flipflop
    Port FB7Eh - FDC 765 Main Status Register (read only)
    Port FB7Fh - FDC 765 Data Register (read/write)
    */
    if (MACHINE_IS_CPC_HAS_FLOPPY) {
        //Puertos disco
        if (pd765_enabled.v) {
            if (puerto==0xFB7E) return pd765_read_status_register();


            if (puerto==0xFB7F) return pd765_read();
        }


        else {
            if (puerto==0xFB7E) return 255;


            if (puerto==0xFB7F) return 255;
        }
    }

	return 255;

}


z80_byte lee_puerto_cpc(z80_byte puerto_h,z80_byte puerto_l)
{
  z80_int port=value_8_to_16(puerto_h,puerto_l);
  ula_contend_port_early( port );
  ula_contend_port_late( port );
  z80_byte valor = lee_puerto_cpc_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}



void out_port_cpc_no_time(z80_int puerto,z80_byte value)
{
	debug_fired_out=1;
	//if (puerto==0xFA7E || puerto==0xFB7E || puerto==0xFB7F) printf ("Puerto FDC\n");

	z80_byte puerto_h=(puerto>>8)&0xFF;

	//Puerto Gate Array
/*
The gate array is controlled by I/O. The gate array is selected when bit 15 of the I/O port address is set to "0" and bit 14 of the I/O port address is set to "1". The values of the other bits are ignored. However, to avoid conflict with other devices in the system, these bits should be set to "1".

The recommended I/O port address is &7Fxx.
*/
	if ((puerto & (32768+16384))==16384) {
		//printf ("cpc out port gate array. port=0x%04X value=0x%02X\n",puerto,value);
		cpc_out_port_gate(value);
	}


	//Puerto CRTC
/*
The 6845 is selected when bit 14 of the I/O port address is set to "0".
Bit 9 and 8 of the I/O port address define the function to access.
The remaining bits can be any value, but it is adviseable to set these to "1" to avoid conflict with other devices in the system.
*/

	if ( (puerto & 16384)==0) {
		//printf ("cpc out port crtc. port=0x%04X value=0x%02X\n",puerto,value);
                cpc_out_port_crtc(puerto,value);
        }




        //Controladora 8255 PPI
        if ((puerto_h & 8)==0) {
                cpc_out_ppi(puerto_h,value);
        }

    //Puerto upper rom select
    if (puerto_h==0xDF) {
        //printf("Out port DF value %02XH\n",value);
        cpc_out_port_df(value);
    }

    //Puertos PD765
    /*
    Port FA7Eh - Floppy Motor On/Off Flipflop
    Port FB7Eh - FDC 765 Main Status Register (read only)
    Port FB7Fh - FDC 765 Data Register (read/write)
    */
   if (MACHINE_IS_CPC_HAS_FLOPPY && pd765_enabled.v) {
        if (puerto==0xFA7E) {
            cpc_out_port_fa7e(value);
        }

        if (puerto==0xFB7F) {
            pd765_out_port_data_register(value);
        }
   }

	//printf ("fin out_port_cpc_no_time\n");

}



void out_port_cpc(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_cpc_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}


z80_byte *sam_return_segment_memory(z80_int dir)
{
        int segmento;
        z80_byte *puntero;

        segmento=dir/16384;
        puntero=sam_memory_paged[segmento];
        return puntero;
}


void poke_byte_no_time_sam(z80_int dir,z80_byte valor)
{

#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif

		//Ver si es segmento de rom o ram
int segmento;
		segmento=dir/16384;
		if (sam_memory_paged_type[segmento]) return;


                z80_byte *puntero;
                puntero=sam_return_segment_memory(dir);

                dir = dir & 16383;

                puntero=puntero+dir;
                *puntero=valor;

}

void poke_byte_sam(z80_int dir,z80_byte valor)
{
int segmento;
                segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];

                }
#endif
                t_estados += 3;

        poke_byte_no_time_sam(dir,valor);
}


z80_byte peek_byte_no_time_sam(z80_int dir)
{
	#ifdef EMULATE_VISUALMEM
		set_visualmemreadbuffer(dir);
	#endif

                z80_byte *puntero;
                puntero=sam_return_segment_memory(dir);

                dir = dir & 16383;

                puntero=puntero+dir;
                return *puntero;
}


z80_byte peek_byte_sam(z80_int dir)
{
        int segmento;
        segmento=dir / 16384;

#ifdef EMULATE_CONTEND
                if (contend_pages_actual[segmento]) {
                        t_estados += contend_table[ t_estados ];
                }
#endif
                t_estados += 3;

        return peek_byte_no_time_sam(dir);

}


z80_byte lee_puerto_sam_no_time(z80_byte puerto_h,z80_byte puerto_l)
{
	debug_fired_in=1;
	//printf ("Leer puerto sam H: %d L: %d\n",puerto_h,puerto_l);

        //Decodificacion completa del puerto o no?
        if (puerto_l==252) {
                return sam_vmpr;
        }

        if (puerto_l==251) {
                return sam_hmpr;
        }

        if (puerto_l==250) {
                return sam_lmpr;
        }

	if (puerto_l==254) {
		z80_byte valor;

		if (puerto_h==255) {
			valor=(puerto_65534)&31;
			if (joystick_emulation==JOYSTICK_CURSOR_SAM) {
				if (zxvision_key_not_sent_emulated_mach() ) {

				}
				else {
					//z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
					if (puerto_especial_joystick&1 ) valor &=(255-16);
					if (puerto_especial_joystick&2 ) valor &=(255-8);
					if (puerto_especial_joystick&4 ) valor &=(255-4);
					if (puerto_especial_joystick&8 ) valor &=(255-2);
				}
			}


		}

		else {


 	               if (initial_tap_load.v==1 && initial_tap_sequence) {
        	                return envia_load_comillas_sam(puerto_h,puerto_l);
                	}


			//Puerto teclado. Parte compatible spectrum
			valor=lee_puerto_teclado(puerto_h) &31;
			//printf ("lectura teclado. puerto_h: %d valor: %d\n",puerto_h,valor&31);
		}

                if (realtape_inserted.v && realtape_playing.v) {
                        if (realtape_get_current_bit_playing()) {
                                valor=valor|64;
                                //printf ("1 ");
                        }
                        else {
                                valor=(valor & (255-64));
                                //printf ("0 ");
                        }
                }



                return valor;
	}

	if (puerto_l==249) {
		//Puerto STATUS.
		z80_byte valor;

		valor=255 & (255-8);
		//De momento devolver con bit FRAME a 0. Cuando devuelve 1???


		if (initial_tap_load.v==1 && initial_tap_sequence) {
			z80_byte valor2=envia_load_comillas_sam(puerto_h,puerto_l);
			//Quitar bits que sobren
			valor2=valor2&(32+64+128);
			valor=valor&31;

			valor=valor|valor2;
			return valor;
                }



                //si estamos en el menu, no devolver tecla
                if (zxvision_key_not_sent_emulated_mach() ) return valor;


		//Y agregar bits superiores teclado
		if ( (puerto_h&1) ==0 ) valor=valor & puerto_teclado_sam_fef9;

		if ( (puerto_h&2) ==0 ) valor=valor & puerto_teclado_sam_fdf9;

		if ( (puerto_h&4) ==0 ) valor=valor & puerto_teclado_sam_fbf9;

		if ( (puerto_h&8) ==0 ) valor=valor & puerto_teclado_sam_f7f9;

		if ( (puerto_h&16)==0 ) valor=valor & puerto_teclado_sam_eff9;

		if ( (puerto_h&32)==0 ) valor=valor & puerto_teclado_sam_dff9;

		if ( (puerto_h&64)==0 ) valor=valor & puerto_teclado_sam_bff9;

		if ( (puerto_h&128)==0 ) valor=valor & puerto_teclado_sam_7ff9;


		return valor;
	}


	// Floppy drive 2 *OR* the ATOM hard disk
	if (atomlite_enabled.v) {
		if ( (puerto_l & 0xf8)==0xF0) {
			//printf ("Read atom hard disk. Puerto_l : %02XH PC=%d\n",puerto_l,reg_pc);
			//return ide_read_command_block_register(puerto_l&7);
			z80_byte valor=atomlite_in(puerto_l);
			//printf ("Returning valor: %d\n",valor);
			return valor;
		}
	}

	//printf ("Unknown port IN: %02X%02X\n",puerto_h,puerto_l);

	//Otros puertos retornar 255
        return 255;

}


z80_byte lee_puerto_sam(z80_byte puerto_h,z80_byte puerto_l)
{
  z80_int port=value_8_to_16(puerto_h,puerto_l);
  ula_contend_port_early( port );
  ula_contend_port_late( port );
  z80_byte valor = lee_puerto_sam_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}


void out_port_sam_no_time(z80_int puerto,z80_byte value)
{

	debug_fired_out=1;

        z80_byte puerto_h=(puerto>>8)&0xFF;
        z80_byte puerto_l=puerto&0xFF;

	//printf ("Out puerto sam puerto: %d puerto_l: %d valor: %d\n",puerto,puerto_l,value);

	if (puerto_l==254) {
		sam_border=value;
		modificado_border.v=1;

		//Mismo comportamiento que Spectrum
		silence_detection_counter=0;
		beeper_silence_detection_counter=0;
                set_value_beeper( (!!(value & 0x10) << 1) + ( (!(value & 0x8))  ) );
		set_value_beeper_on_array(value_beeper);
		//printf ("beeper: %d\n",value_beeper);


	}

	//Decodificacion completa del puerto o no?
	//Parece que NO
	if (puerto_l==252) {
		z80_byte modo_video_antes=(sam_vmpr>>5)&3;
		sam_vmpr=value;
		z80_byte modo_video_despues=(sam_vmpr>>5)&3;
		if (modo_video_antes!=modo_video_despues) {
			sam_splash_videomode_change();
		}
		//modo video
		//z80_byte modo_video=(sam_vmpr>>5)&3;
		//printf ("Setting video mode :%d\n",modo_video);
		//sleep(1);
	}

	if (puerto_l==251) {
		sam_hmpr=value;
		sam_set_memory_pages();
	}

	if (puerto_l==250) {
		sam_lmpr=value;

		sam_set_memory_pages();
	}


	if (puerto_l==248) {
		//Paleta de colores
		//printf ("cambio paleta. Color %d valor %d\n",puerto_h,value);
		sam_palette[puerto_h&15]=value;
		modificado_border.v=1;
	}

       // Floppy drive 2 *OR* the ATOM hard disk
	if (atomlite_enabled.v) {
	        if ( (puerto_l & 0xf8)==0xF0) {
        	        //printf ("Write atom hard disk. Port : %02XH value: %02XH PC=%d\n",puerto_l,value,reg_pc);
                	//return ide_write_command_block_register(puerto_l&7,value);
			atomlite_out(puerto_l,value);
        	}
	}

	if (puerto==0x01ff) {
		saa_simul_write_address(value);
	}

	if (puerto==0x00ff) {
		saa_simul_write_data(value);
	}

}




void out_port_sam(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_sam_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}




void poke_byte_no_time_pcw(z80_int dir,z80_byte valor)
{

    z80_byte *puntero;

    puntero=pcw_get_memory_offset_write(dir);

    *puntero=valor;



/*#ifdef EMULATE_VISUALMEM

set_visualmembuffer(dir);

#endif*/

}

void poke_byte_pcw(z80_int dir,z80_byte valor)
{


        //Y sumamos estados normales
        t_estados += 3;


	poke_byte_no_time_pcw(dir,valor);


}

void out_port_pcw_no_time(z80_int puerto,z80_byte value)
{
    debug_fired_out=1;


    z80_byte puerto_l=puerto&0xFF;

    if (puerto_l==0x01) {
        //printf("OUT FDC data register %02XH\n",value);
        pd765_out_port_data_register(value);
    }

    /*

    &F0 O   Select bank for &0000
    &F1 O   Select bank for &4000
    &F2 O   Select bank for &8000
    &F3 O   Select bank for &C000. Usually &87.
    */

    if (puerto_l>=0xF0 && puerto_l<=0xF3) {
        pcw_out_port_bank(puerto_l,value);
    }

    if (puerto_l==0xF4) {
        pcw_out_port_f4(value);
    }

    if (puerto_l==0xF5) {
        pcw_out_port_f5(value);
    }

    if (puerto_l==0xF6) {
        pcw_out_port_f6(value);
    }

    if (puerto_l==0xF7) {
        pcw_out_port_f7(value);
    }

    if (puerto_l==0xF8) {
        pcw_out_port_f8(value);
    }

    //13.1 DKTronics sound generator
    //TODO: Registro 0E para lectura de joystick dk tronics
    //
    /*
    Bit 7 Ignored.
    Bit 6 0 if the fire button is pressed.
    Bit 5 0 if the joystick is pushed up.
    Bit 4 0 if the joystick is pushed down.
    Bit 3 0 if the joystick is pushed to the right.
    27
    Bit 2 0 if the joystick is pushed to the left. Bit 1 Ignored.
    Bit 0 Ignored.
    */
   /*
   Usado en:
   Abadia del Crimen
   Head Over Heels
   */

    if (puerto_l==0xAA && ay_chip_present.v) out_port_ay(65533,value);
    if (puerto_l==0xAB && ay_chip_present.v) out_port_ay(49149,value);



    if (puerto_l!=0x01 && puerto_l!=0xF4 && puerto_l!=0xf8 && puerto_l!=0xAA && puerto_l!=0xAB && (puerto_l<0xf0 || puerto_l>0xf8)) {
        //printf("Out port UNKNOWN %02XH value %02XH\n",puerto_l,value);
        //sleep(3);
    }

}


void out_port_pcw(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_pcw_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}



z80_byte lee_puerto_pcw_no_time(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l)
{

	debug_fired_in=1;

    //printf("LEE puerto %02XH\n",puerto_l);

    if (puerto_l==0x00) {
        //printf("IN FDC status register\n");
        return pd765_read_status_register();
    }

    if (puerto_l==0x01) {
        //printf("IN FDC data register\n");
        return pd765_read();
    }
    if (puerto_l==0xF4) {
        return pcw_in_port_f4();
    }

    if (puerto_l==0xF8) {
        return pcw_in_port_f8();


    }

    if (puerto_l==0xFD) {
        return pcw_in_port_fd();
    }

    if (ay_chip_present.v) {
        if (puerto_l==0xA9) {

            //Registro 14, dktronics joystick
            if ( (ay_3_8912_registro_sel[ay_chip_selected] & 15) == 14) {
                return pcw_in_port_dktronics_joystick();
    		}

            return in_port_ay(0xFF);
        }
    }

    if (puerto_l==0x9f) {
        return pcw_in_port_9f();
    }

    if (puerto_l==0xe0) {
        return pcw_in_port_e0();
    }


    //printf ("In Port %x unknown asked, PC after=0x%x\n",puerto_l+256*puerto_h,reg_pc);
    return 255;


}

z80_byte lee_puerto_pcw(z80_byte puerto_h,z80_byte puerto_l)
{
  z80_int port=value_8_to_16(puerto_h,puerto_l);
  ula_contend_port_early( port );
  ula_contend_port_late( port );
  z80_byte valor = lee_puerto_pcw_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}

z80_byte peek_byte_no_time_pcw(z80_int dir)
{
/*
#ifdef EMULATE_VISUALMEM
	set_visualmemreadbuffer(dir);
#endif
*/
    //Lectura de teclado
    int offset=dir & 16383;
    if (offset>=0x3FF0) {
        int segmento=dir/16384;

        if (pcw_banks_paged_read[segmento]==3) {
            return pcw_read_keyboard(offset);
        }

    }


    z80_byte *puntero;

    puntero=pcw_get_memory_offset_read(dir);

    return *puntero;

}


z80_byte peek_byte_pcw(z80_int dir)
{

        t_estados +=3;

        return peek_byte_no_time_pcw(dir);

}

z80_int peek_word(z80_int dir)
{
        z80_byte h,l;

        l=peek_byte(dir);
        h=peek_byte(dir+1);

        return (h<<8) | l;
}

z80_int peek_word_no_time(z80_int dir)
{
        z80_byte h,l;

        l=peek_byte_no_time(dir);
        h=peek_byte_no_time(dir+1);

        return (h<<8) | l;
}



z80_int lee_word_pc(void)
{
        z80_int valor;

        valor=peek_word(reg_pc);
        reg_pc +=2;
        return valor;

	//este reg_pc+=2 es incorrecto, no funciona
	//return peek_word(reg_pc+=2);
}

void poke_word(z80_int dir,z80_int valor)
{
        poke_byte(dir,valor & 0xFF);
        poke_byte(dir+1, (valor >> 8) & 0xFF );
}



z80_byte peek_byte_desp(z80_int dir,z80_byte desp)
{


//RETURN (dir+desp)

        z80_int desp16,puntero;

        desp16=desp8_to_16(desp);

        puntero=dir + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif

        return peek_byte(puntero);

}

void poke_byte_desp(z80_int dir,z80_byte desp,z80_byte valor)
{
        z80_int desp16,puntero;

        desp16=desp8_to_16(desp);

        puntero=dir + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif

	poke_byte(puntero,valor);
}





//Esto no es macro porque se usa en sitios como cp(hl) donde se hace cp_reg(peek_byte(hl)) y provocaria error de timing
void cp_reg(z80_byte value)
{

        z80_byte result,antes;

        set_undocumented_flags_bits(value);

        result=reg_a;
        antes=reg_a;

        result -=value;

        set_flags_zero_sign(result);
        set_flags_carry_resta(antes,result);
        set_flags_overflow_resta(antes,result);
        Z80_FLAGS |=FLAG_N;

}






z80_int sbc_16bit(z80_int reg, z80_int value)
{

	z80_int result;
        z80_byte h;
        int result_32bit;

        set_memptr(reg+1);



        result_32bit=reg-value-( Z80_FLAGS & FLAG_C );

        z80_int lookup =      ( (  (reg) & 0x8800 ) >> 11 ) | \
                            ( (  (value) & 0x8800 ) >> 10 ) | \
                            ( ( result_32bit & 0x8800 ) >>  9 );  \


        result=result_32bit & 65535;
        h=value_16_to_8h(result);
        set_undocumented_flags_bits(h);


        if (result_32bit & 0x10000) Z80_FLAGS |=FLAG_C;
        else Z80_FLAGS &=(255-FLAG_C);

        set_flags_zero_sign_16(result);

	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_H-FLAG_PV)) | halfcarry_sub_table[lookup&0x07] | overflow_sub_table[lookup >> 4] | FLAG_N;

	return result;


}





z80_int add_16bit(z80_int reg, z80_int value)
{
        z80_int result,antes;
        z80_byte h;

	set_memptr(reg+1);


        antes=reg;
        result=reg;

        result +=value;

	z80_int lookup = ( (       (reg) & 0x0800 ) >> 11 ) | \
                            ( (  (value) & 0x0800 ) >> 10 ) | \
                            ( (   result & 0x0800 ) >>  9 );  \


        h=value_16_to_8h(result);
        set_undocumented_flags_bits(h);

        set_flags_carry_16_suma(antes,result);


	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_H-FLAG_N)) | halfcarry_add_table[lookup];


        return result;
}

z80_int adc_16bit(z80_int reg, z80_int value)
{

        z80_int result;
        z80_byte h;
	int result_32bit;

	set_memptr(reg+1);


        result_32bit=reg+value+( Z80_FLAGS & FLAG_C );

	z80_int lookup =      ( (  (reg) & 0x8800 ) >> 11 ) | \
                            ( (  (value) & 0x8800 ) >> 10 ) | \
                            ( ( result_32bit & 0x8800 ) >>  9 );  \


	result=result_32bit & 65535;
        h=value_16_to_8h(result);
        set_undocumented_flags_bits(h);


        if (result_32bit & 0x10000) Z80_FLAGS |=FLAG_C;
        else Z80_FLAGS &=(255-FLAG_C);

        set_flags_zero_sign_16(result);

        Z80_FLAGS=(Z80_FLAGS & (255-FLAG_H-FLAG_PV-FLAG_N)) | halfcarry_add_table[lookup&0x07] | overflow_add_table[lookup >> 4];


        return result;


}

/*z80_int inc_16bit(z80_int reg)
{
//INC 16 bit register
	reg++;
	return reg;
}

z80_int dec_16bit(z80_int reg)
{
//DEC 16 bit register
	reg--;
	return reg;
}
*/

/*

z80_int inc_16bit(z80_int reg)
{
//INC 16 bit register

	z80_byte h;

        reg++;

        h=value_16_to_8h(reg);
        set_undocumented_flags_bits(h);

	return reg;

}


z80_int dec_16bit(z80_int reg)
{
//DEC 16 bit register
	z80_byte h;

        reg--;

        h=value_16_to_8h(reg);
        set_undocumented_flags_bits(h);

        return reg;

}
*/

z80_int desp8_to_16(z80_byte desp)
{

	z80_int desp16;

        if (desp>127) {
		desp=256-desp;
                desp16=-desp;
	}
        else
                desp16=desp;

	return desp16;

}



z80_int pop_valor()
{
	z80_int valor;

        valor=peek_word(reg_sp);
        reg_sp +=2;
        return valor;

}

//Tener en cuenta los valores de tipo para los strings:
/*enum push_value_type {
	PUSH_VALUE_TYPE_DEFAULT=0,
	PUSH_VALUE_TYPE_CALL,
	PUSH_VALUE_TYPE_RST,
	PUSH_VALUE_TYPE_PUSH,
	PUSH_VALUE_TYPE_MASKABLE_INTERRUPT,
        PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT
};
*/

char *push_value_types_strings[TOTAL_PUSH_VALUE_TYPES]={
	"default",
	"call",
	"rst",
	"push",
	"maskable_interrupt",
	"non_maskable_interrupt"
};



//En la funcion por defecto no usamos el tipo
//El tipo realmente sera un valor en el rango del enum de push_value_type
//lo pongo como z80_byte y no como enum porque luego al activar extended_stack, las funciones de nested
//requieren que ese parametro sea z80_byte
void push_valor_default(z80_int valor,z80_byte tipo GCC_UNUSED)
{
        reg_sp -=2;
        poke_word(reg_sp,valor);
}



z80_byte rlc_valor_comun(z80_byte value)
{
        if (value & 128) Z80_FLAGS |=FLAG_C;
        else Z80_FLAGS &=(255-FLAG_C);

        value=value << 1;
        value |= ( Z80_FLAGS & FLAG_C );

        set_undocumented_flags_bits(value);
	Z80_FLAGS &=(255-FLAG_N-FLAG_H);

        return value;

}

z80_byte rlc_valor(z80_byte value)
{
	value=rlc_valor_comun(value);
	set_flags_zero_sign(value);
	set_flags_parity(value);
	return value;
}


void rlc_reg(z80_byte *reg)
{
        z80_byte value=*reg;

        *reg=rlc_valor(value);

}

void rlca(void)
{
        //RLCA no afecta a S, Z ni P
        reg_a=rlc_valor_comun(reg_a);
}



//rutina comun a rl y rla
z80_byte rl_valor_comun(z80_byte value)
{
        z80_bit flag_C_antes;
        flag_C_antes.v=( Z80_FLAGS & FLAG_C );

        if (value & 128) Z80_FLAGS |=FLAG_C;
        else Z80_FLAGS &=(255-FLAG_C);

        value=value << 1;
        value |= flag_C_antes.v;

        set_undocumented_flags_bits(value);

        Z80_FLAGS &=(255-FLAG_N-FLAG_H);

        return value;

}

z80_byte rl_valor(z80_byte value)
{
	value=rl_valor_comun(value);
	set_flags_zero_sign(value);
	set_flags_parity(value);
        return value;

}

void rl_reg(z80_byte *reg)
{
        z80_byte value=*reg;

        *reg=rl_valor(value);

}


void rla(void)
{
	//RLA no afecta a S, Z ni P
        reg_a=rl_valor_comun(reg_a);
}



z80_byte rr_valor_comun(z80_byte value)
{
        z80_bit flag_C_antes;
        flag_C_antes.v=( Z80_FLAGS & FLAG_C );

        if (value & 1) Z80_FLAGS |=FLAG_C;
        else Z80_FLAGS &=(255-FLAG_C);

        value=value >> 1;
        value |= (flag_C_antes.v*128);

        set_undocumented_flags_bits(value);
	Z80_FLAGS &=(255-FLAG_N-FLAG_H);

        return value;

}

z80_byte rr_valor(z80_byte value)
{
	value=rr_valor_comun(value);
        set_flags_zero_sign(value);
	set_flags_parity(value);
	return value;
}


void rr_reg(z80_byte *reg)
{
        z80_byte value=*reg;

        *reg=rr_valor(value);

}


void rra(void)
{
        //RRA no afecta a S, Z ni P
        reg_a=rr_valor_comun(reg_a);

}

z80_byte rrc_valor_comun(z80_byte value)
{
        if (value & 1) Z80_FLAGS |=FLAG_C;
        else Z80_FLAGS &=(255-FLAG_C);

        value=value >> 1;
        value |= (( Z80_FLAGS & FLAG_C )*128);

        set_undocumented_flags_bits(value);
	Z80_FLAGS &=(255-FLAG_N-FLAG_H);

        return value;

}

z80_byte rrc_valor(z80_byte value)
{
	value=rrc_valor_comun(value);
	set_flags_zero_sign(value);
	set_flags_parity(value);
	return value;
}

void rrc_reg(z80_byte *reg)
{
        z80_byte value=*reg;

        *reg=rrc_valor(value);

}



void rrca(void)
{
        //RRCA no afecta a S, Z ni P
        reg_a=rrc_valor_comun(reg_a);
}





z80_byte sla_valor(z80_byte value)
{

	Z80_FLAGS=0;

        if (value & 128) Z80_FLAGS |=FLAG_C;

        value=value << 1;

	Z80_FLAGS |=sz53p_table[value];

        return value;

}

void sla_reg(z80_byte *reg)
{

        z80_byte value=*reg;

        *reg=sla_valor(value);

}

z80_byte sra_valor(z80_byte value)
{
        z80_byte value7=value & 128;

	Z80_FLAGS=0;

        if (value & 1) Z80_FLAGS |=FLAG_C;

        value=value >> 1;
        value |= value7;

	Z80_FLAGS |=sz53p_table[value];

        return value;

}

void sra_reg(z80_byte *reg)
{
        z80_byte value=*reg;

        *reg=sra_valor(value);

}

z80_byte srl_valor(z80_byte value)
{

	Z80_FLAGS=0;

        if (value & 1) Z80_FLAGS |=FLAG_C;

        value=value >> 1;

	Z80_FLAGS |=sz53p_table[value];

        return value;

}

void srl_reg(z80_byte *reg)
{
        z80_byte value=*reg;

        *reg=srl_valor(value);

}

z80_byte sls_valor(z80_byte value)
{

	Z80_FLAGS=0;

        if (value & 128) Z80_FLAGS |=FLAG_C;

        value=(value << 1) | 1;

	Z80_FLAGS |=sz53p_table[value];

        return value;

}

void sls_reg(z80_byte *reg)
{

        z80_byte value=*reg;

        *reg=sls_valor(value);

}


void add_a_reg(z80_byte value)
{

        z80_byte result,antes;

        result=reg_a;
        antes=reg_a;

        result +=value;
        reg_a=result;


        set_flags_carry_suma(antes,result);
        set_flags_overflow_suma(antes,result);
        Z80_FLAGS=(Z80_FLAGS & (255-FLAG_N-FLAG_Z-FLAG_S-FLAG_5-FLAG_3)) | sz53_table[result];


}


void adc_a_reg(z80_byte value)
{

	//printf ("flag_C antes: %d ",( Z80_FLAGS & FLAG_C ));

        z80_byte result,lookup;
	z80_int result_16bit;


        result_16bit=reg_a;
        result_16bit=result_16bit+value+( Z80_FLAGS & FLAG_C );

	lookup = ( (       reg_a & 0x88 ) >> 3 ) |
                     ( ( (value) & 0x88 ) >> 2 ) |
                ( ( result_16bit & 0x88 ) >> 1 );


	result=value_16_to_8l(result_16bit);
        reg_a=result;


	Z80_FLAGS = ( result_16bit & 0x100 ? FLAG_C : 0 );

	Z80_FLAGS=Z80_FLAGS | halfcarry_add_table[lookup & 0x07] | overflow_add_table[lookup >> 4] | sz53_table[result];

	//printf ("antes: %d value: %d result: %d flag_c: %d flag_pv: %d\n", antes, value, result, ( Z80_FLAGS & FLAG_C ),flag_PV.v);

}


//Devuelve el resultado de A-value sin alterar A
z80_byte sub_value(z80_byte value)
{
        z80_byte result,antes;

        result=reg_a;
        antes=reg_a;

        result -=value;

        set_flags_carry_resta(antes,result);
        set_flags_overflow_resta(antes,result);
        Z80_FLAGS=(Z80_FLAGS & (255-FLAG_Z-FLAG_S-FLAG_5-FLAG_3)) | FLAG_N | sz53_table[result];

	return result;


}

void sub_a_reg(z80_byte value)
{
	reg_a=sub_value(value);

}

void sbc_a_reg(z80_byte value)
{
        z80_byte result,lookup;
        z80_int result_16bit;

        result_16bit=reg_a;
	result_16bit=result_16bit-value-( Z80_FLAGS & FLAG_C );


        lookup = ( (       reg_a & 0x88 ) >> 3 ) |
                     ( ( (value) & 0x88 ) >> 2 ) |
                ( ( result_16bit & 0x88 ) >> 1 );

	result=value_16_to_8l(result_16bit);
        reg_a=result;


	Z80_FLAGS = ( result_16bit & 0x100 ? FLAG_C : 0 );

        Z80_FLAGS=Z80_FLAGS | halfcarry_sub_table[lookup & 0x07] | overflow_sub_table[lookup >> 4] | FLAG_N | sz53_table[result];


}




void rl_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
        z80_byte valor_leido;
        z80_int desp16,puntero;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
	set_memptr(puntero);
#endif

        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = rl_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}

void rr_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
        z80_byte valor_leido;
        z80_int desp16,puntero;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = rr_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}



void rlc_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
	z80_byte valor_leido;
	z80_int desp16,puntero;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = rlc_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}

void rrc_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
        z80_byte valor_leido;
        z80_int desp16,puntero;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = rrc_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}

void sla_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
        z80_byte valor_leido;
        z80_int desp16,puntero;


	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = sla_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}

void sra_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
        z80_byte valor_leido;
        z80_int desp16,puntero;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = sra_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}

void srl_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
        z80_byte valor_leido;
        z80_int desp16,puntero;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = srl_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}

void sls_ixiy_desp_reg(z80_byte desp,z80_byte *registro)
{
        z80_byte valor_leido;
        z80_int desp16,puntero;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = sls_valor(valor_leido);
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
}


void res_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp, z80_byte *registro)
{

	z80_byte valor_and,valor_leido;
	z80_int desp16,puntero;

	//printf ("res bit=%d desp=%d reg=%x   \n",numerobit,desp,registro);

	valor_and=1;

	if (numerobit) valor_and = valor_and << numerobit;

	//cambiar 0 por 1
	valor_and = valor_and ^ 0xFF;

	desp16=desp8_to_16(desp);
	puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


	valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
	valor_leido = valor_leido & valor_and;
	poke_byte(puntero,valor_leido);


	if (registro!=0) *registro=valor_leido;
	//printf (" valor_and = %d valor_final = %d \n",valor_and,valor_leido);

}

void set_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp, z80_byte *registro)
{

        z80_byte valor_or,valor_leido;
        z80_int desp16,puntero;

        //printf ("set bit=%d desp=%d reg=%x   \n",numerobit,desp,registro);

        valor_or=1;

        if (numerobit) valor_or = valor_or << numerobit;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );
        valor_leido = valor_leido | valor_or;
        poke_byte(puntero,valor_leido);

        if (registro!=0) *registro=valor_leido;
        //printf (" valor_set = %d valor_final = %d \n",valor_or,valor_leido);

}

//void bit_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp, z80_byte *registro)
void bit_bit_ixiy_desp_reg(z80_byte numerobit, z80_byte desp)
{

        z80_byte valor_and,valor_leido;
        z80_int desp16,puntero;

        //printf ("bit bit=%d desp=%d reg=%x   \n",numerobit,desp,registro);

        valor_and=1;

        if (numerobit) valor_and = valor_and << numerobit;

	desp16=desp8_to_16(desp);
        puntero=*registro_ixiy + desp16;

#ifdef EMULATE_MEMPTR
        set_memptr(puntero);
#endif


        valor_leido=peek_byte(puntero);
	contend_read_no_mreq( puntero, 1 );

//                                        ;Tambien hay que poner el flag P/V con el mismo valor que
//                                        ;coge el flag Z, y el flag S debe tener el valor del bit 7


	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_N-FLAG_Z-FLAG_PV-FLAG_S)) | FLAG_H;

	if (!(valor_leido & valor_and) ) {
		Z80_FLAGS |=FLAG_Z|FLAG_PV;
	}

        if (numerobit==7 && (valor_leido & 128) ) Z80_FLAGS |=FLAG_S;

        //En teoria el BIT no tiene este comportamiento de poder asignar el resultado a un registro
        //if (registro!=0) *registro=valor_leido;

        //printf (" valor_bit = %d valor_final = %d \n",valor_or,valor_leido);

        set_undocumented_flags_bits(value_16_to_8h(puntero));

}




z80_byte *devuelve_reg_offset(z80_byte valor)
{

	//printf ("devuelve_reg de: %d\n",valor);
	switch (valor) {
		case 0:
			return &reg_b;
		;;
		case 1:
			return &reg_c;
		;;
		case 2:
			return &reg_d;
		;;
		case 3:
			return &reg_e;
		;;
		case 4:
			return &reg_h;
		;;
		case 5:
			return &reg_l;
		;;
		case 6:
			return 0;
		;;
		case 7:
			return &reg_a;
		;;

		default:
			cpu_panic("Critical Error devuelve_reg_offset valor>7");
			//aqui no deberia llegar nunca
			return 0;
		break;
	}

}



void rl_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

	if (registro==0) {
		//(HL)
		valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
		valor_leido=rl_valor(valor_leido);
        	poke_byte(HL,valor_leido);
	}

	else rl_reg(registro);

}

void rr_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido=rr_valor(valor_leido);
                poke_byte(HL,valor_leido);
        }

        else rr_reg(registro);
}



void rlc_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido=rlc_valor(valor_leido);
                poke_byte(HL,valor_leido);
        }

        else rlc_reg(registro);
}

void rrc_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido=rrc_valor(valor_leido);
                poke_byte(HL,valor_leido);
        }

        else rrc_reg(registro);
}

void sla_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido=sla_valor(valor_leido);
                poke_byte(HL,valor_leido);
        }

        else sla_reg(registro);
}

void sra_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido=sra_valor(valor_leido);
                poke_byte(HL,valor_leido);
        }

        else sra_reg(registro);
}

void srl_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido=srl_valor(valor_leido);
                poke_byte(HL,valor_leido);
        }

        else srl_reg(registro);

}

void sls_cb_reg(z80_byte *registro)
{
        z80_byte valor_leido;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido=sls_valor(valor_leido);
                poke_byte(HL,valor_leido);
        }

        else sls_reg(registro);
}


void res_bit_cb_reg(z80_byte numerobit, z80_byte *registro)
{

	z80_byte valor_and,valor_leido;

	//printf ("res bit=%d reg=%x  \n",numerobit,registro);

	valor_and=1;

	if (numerobit) valor_and = valor_and << numerobit;

	//cambiar 0 por 1
	valor_and = valor_and ^ 0xFF;

	if (registro==0) {
		//(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
		valor_leido=valor_leido & valor_and;
                poke_byte(HL,valor_leido);
	}
	else {
		valor_leido = (*registro) & valor_and;
		*registro = valor_leido;
	}

	//printf (" valor_and = %d valor_final = %d \n",valor_and,valor_leido);

}

void set_bit_cb_reg(z80_byte numerobit, z80_byte *registro)
{

        z80_byte valor_or,valor_leido;

	//printf ("set bit=%d reg=%x  \n",numerobit,registro);

        valor_or=1;

        if (numerobit) valor_or = valor_or << numerobit;

        if (registro==0) {
                //(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
                valor_leido = valor_leido | valor_or;
                poke_byte(HL,valor_leido);
        }
        else {

                valor_leido = (*registro) | valor_or;
                *registro = valor_leido;
        }

        //printf (" valor_set = %d valor_final = %d \n",valor_or,valor_leido);

}

void bit_bit_cb_reg(z80_byte numerobit, z80_byte *registro)
{

        z80_byte valor_or,valor_leido;

	//printf ("bit bit=%d reg=%x ",numerobit,registro);

        valor_or=1;

        if (numerobit) valor_or = valor_or << numerobit;

	if (registro==0) {
		//(HL)
                valor_leido=peek_byte(HL);
		contend_read_no_mreq( HL, 1 );
		set_undocumented_flags_bits_memptr();

	}
	else {
		valor_leido=*registro;
	        if (numerobit==5 && (valor_leido & 32 )) Z80_FLAGS |=FLAG_5;
	        else Z80_FLAGS &=(255-FLAG_5);

        	if (numerobit==3 && (valor_leido & 8 )) Z80_FLAGS |=FLAG_3;
	        else Z80_FLAGS &=(255-FLAG_3);

	}

//                                        ;Tambien hay que poner el flag P/V con el mismo valor que
//                                        ;coge el flag Z, y el flag S debe tener el valor del bit 7


	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_N-FLAG_Z-FLAG_PV-FLAG_S)) | FLAG_H;

	if (!(valor_leido & valor_or) ) {
		Z80_FLAGS |=FLAG_Z|FLAG_PV;
	}

	if (numerobit==7 && (valor_leido & 128) ) Z80_FLAGS |=FLAG_S;

        //printf (" valor_bit = %d valor_final = %d \n",valor_or,valor_leido);


}


//;                    Bits:  4    3    2    1    0     ;desplazamiento puerto
//puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
//puerto_65022   db    255  ; G    F    D    S    A     ;1
//puerto_64510    db              255  ; T    R    E    W    Q     ;2
//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H                J         K      L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7

z80_byte lee_puerto_zx80_no_time(z80_byte puerto_h,z80_byte puerto_l)
{

	debug_fired_in=1;
	z80_byte valor;

	z80_int puerto=value_8_to_16(puerto_h,puerto_l);

	//xx1D Zebra Joystick                          - - - F R L D U   (0=Pressed)
	if ( puerto_l==0x1d) {
		 if (joystick_emulation==JOYSTICK_ZEBRA) {
			z80_byte valor_joystick=255;
			//si estamos con menu abierto, no retornar nada
			if (zxvision_key_not_sent_emulated_mach() ) return valor_joystick;

			//z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

			if ((puerto_especial_joystick&1)) valor_joystick &=(255-8);
			if ((puerto_especial_joystick&2)) valor_joystick &=(255-4);
			if ((puerto_especial_joystick&4)) valor_joystick &=(255-2);
			if ((puerto_especial_joystick&8)) valor_joystick &=(255-1);
			if ((puerto_especial_joystick&16)) valor_joystick &=(255-16);

                        return valor_joystick;
                }
		return 255;
	}

	//xxDF Mikro-Gen Digital Joystick (Port DFh) - - - F L R D U   (0=Pressed)

        else if ( puerto_l==0xdf) {
                 if (joystick_emulation==JOYSTICK_MIKROGEN) {
                        z80_byte valor_joystick=255;
                        //si estamos con menu abierto, no retornar nada
                        if (zxvision_key_not_sent_emulated_mach() ) return valor_joystick;

                        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

                        if ((puerto_especial_joystick&1)) valor_joystick &=(255-4);
                        if ((puerto_especial_joystick&2)) valor_joystick &=(255-8);
                        if ((puerto_especial_joystick&4)) valor_joystick &=(255-2);
                        if ((puerto_especial_joystick&8)) valor_joystick &=(255-1);
                        if ((puerto_especial_joystick&16)) valor_joystick &=(255-16);

                        return valor_joystick;
                }
                return 255;
        }

	//zx printer
        else if (puerto_l==0xFB) {
                if (zxprinter_enabled.v==1) {
                        return zxprinter_get_port();
                }


                else return 255;
        }

	//zxpand
	else if (puerto_l==0x07 && zxpand_enabled.v) {
        	//printf ("In Port ZXpand 0x%X asked, PC after=0x%x\n",puerto_l+256*puerto_h,reg_pc);
		z80_byte valor_retorno=zxpand_read(puerto_h);
		//printf ("Returning value 0x%X\n",valor_retorno);
		return valor_retorno;
	}


	//Puerto con A0 cero
	else if ( (puerto_l&1)==0) {


                if (nmi_generator_active.v==0) {

			//printf ("lee puerto. t_estados=%d t_scanline_draw_timeout: %d\n",t_estados,t_scanline_draw_timeout);

			//si es inicio vsync, guardar tiempo inicial

			if (video_zx8081_linecntr_enabled.v==1) {


				//solo admitir inicio vsync hacia el final de pantalla o hacia principio
				//if (t_scanline_draw_timeout>MINIMA_LINEA_ADMITIDO_VSYNC || t_scanline_draw_timeout<=3) {
				if (t_scanline_draw_timeout>MINIMA_LINEA_ADMITIDO_VSYNC) {
					inicio_pulso_vsync_t_estados=t_estados;
					//printf ("admitido inicio pulso vsync: t_estados: %d linea: %d\n",inicio_pulso_vsync_t_estados,t_scanline_draw_timeout);

					//video_zx8081_linecntr=0;
					//video_zx8081_linecntr_enabled.v=0;
				}

				else {
					//printf ("no se admite inicio pulso vsync : t_estados: %d linea: %d\n",inicio_pulso_vsync_t_estados,t_scanline_draw_timeout);
				}

			}


			video_zx8081_linecntr=0;
			video_zx8081_linecntr_enabled.v=0;


                        //printf("Disabling the HSYNC generator t_scanline_draw=%d\n",t_scanline_draw);
                        hsync_generator_active.v=0;
                        modificado_border.v=1;


                	//y ponemos a low la salida del altavoz
	                bit_salida_sonido_zx8081.v=0;

			set_value_beeper_on_array(da_amplitud_speaker_zx8081() );


			if (zx8081_vsync_sound.v==1) {
				//solo resetea contador de silencio cuando esta activo el vsync sound - beeper
				silence_detection_counter=0;
				beeper_silence_detection_counter=0;
			}


			video_zx8081_ula_video_output=255;


			//ejecutado_zona_pantalla.v=0;

		}

                //Teclado

		//probamos a enviar lo mismo que con teclado de spectrum
		valor=lee_puerto_teclado(puerto_h);

/*
  Bit  Expl.
  0-4  Keyboard column bits (0=Pressed)
  5    Not used             (1)
  6    Display Refresh Rate (0=60Hz, 1=50Hz)
  7    Cassette input       (0=Normal, 1=Pulse)
*/
		//decimos que no hay pulso de carga. 50 Hz refresco

		valor = (valor & 31) | (32+64);

		//valor &= (255-128);


		//decimos que hay pulso de carga, alternado
		//if (reg_b!=0) temp_cinta_zx81=128;
		//else temp_cinta_zx81=0;
		//valor |=temp_cinta_zx81;

		//printf ("valor: %d\n",valor);

            int leer_cinta_real=0;

            if (realtape_inserted.v && realtape_playing.v) leer_cinta_real=1;

            if (audio_can_record_input()) {
                if (audio_is_recording_input) {
                    leer_cinta_real=1;
                }
            }

            if (leer_cinta_real) {
			    if (realtape_get_current_bit_playing()) {
                                valor=valor|128;
                                //printf ("1 ");
                        }
                        else {
                                valor=(valor & (255-128));
                                //printf ("0 ");
                        }
            }


		return valor;


	}

	//ZEsarUX ZXI ports
	if (hardware_debug_port.v) {
		if (puerto==ZESARUX_ZXI_ZX8081_PORT_REGISTER) return zesarux_zxi_read_last_register();
		if (puerto==ZESARUX_ZXI_ZX8081_PORT_DATA)     return zesarux_zxi_read_register_value();
    	}



	//chroma 81
	if (puerto_l==0xef && puerto_h==0x7f) {

		//autoactivar
		if (chroma81.v==0 && autodetect_chroma81.v) {
			debug_printf (VERBOSE_INFO,"Autoenabling Chroma81");
			enable_chroma81();
		}

		if (chroma81.v) {
			//printf ("in port chroma 81\n");
			//bit 5: 0=Colour modes available
			return 0;
		}
	}




	//Cualquier otro puerto
	//debug_printf (VERBOSE_DEBUG,"In Port %x unknown asked, PC after=0x%x",puerto_l+256*puerto_h,reg_pc);
	return 255;
}

z80_byte lee_puerto_ace_no_time(z80_byte puerto_h,z80_byte puerto_l)
{

	debug_fired_in=1;
        //Puerto ULA, cualquier puerto par

        if ((puerto_l & 1)==0) {

		//Any read from this port toggles the speaker "off".
		                        //y ponemos a low la salida del altavoz
                        bit_salida_sonido_ace.v=0;

                        set_value_beeper_on_array(da_amplitud_speaker_ace() );

				//No alteramos detector silencio aqui. Se hace en out port, dado que
				//la funcion de lectura se llama siempre (al leer teclado) y provocaria que nunca
				//se activase el silencio
                                //silence_detection_counter=0;
                                //beeper_silence_detection_counter=0;

                z80_byte valor;
                valor=lee_puerto_teclado(puerto_h);

		valor = (valor & 31);

/*
Port FEh Read (or any Read with A0=0)
  0-4  Keyboard Bits
  5    Cassette Input (EAR/LOAD)
  6-7  Not used
*/


            int leer_cinta_real=0;

            if (realtape_inserted.v && realtape_playing.v) leer_cinta_real=1;

            if (audio_can_record_input()) {
                if (audio_is_recording_input) {
                    leer_cinta_real=1;
                }
            }

            if (leer_cinta_real) {

                        if (realtape_get_current_bit_playing()) {
                                valor=valor|32;
                                //printf ("1 ");
                        }
                        else {
                                valor=(valor & (255-32));
                                //printf ("0 ");
                        }
                }

        //printf("valor teclado: %d\n",valor);
		return valor;

        }

	//Soundbox
        if (puerto_l==0xFF) {
                        activa_ay_chip_si_conviene();
                        if (ay_chip_present.v==1) return in_port_ay(0xFF);
        }



        //debug_printf (VERBOSE_DEBUG,"In Port %x unknown asked, PC after=0x%x",puerto_l+256*puerto_h,reg_pc);
        return 255;


}

z80_byte lee_puerto_ace(z80_byte puerto_h,z80_byte puerto_l)
{
  z80_int port=value_8_to_16(puerto_h,puerto_l);
  ula_contend_port_early( port );
  ula_contend_port_late( port );
  z80_byte valor = lee_puerto_ace_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}


z80_byte lee_puerto_zx81(z80_byte puerto_h,z80_byte puerto_l)
{
  return lee_puerto_zx80(puerto_h,puerto_l);
}


z80_byte lee_puerto_zx80(z80_byte puerto_h,z80_byte puerto_l)
{
  z80_int port=value_8_to_16(puerto_h,puerto_l);
  ula_contend_port_early( port );
  ula_contend_port_late( port );
  z80_byte valor = lee_puerto_zx80_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}

void envia_jload_desactivar(void)
{
	initial_tap_load.v=0;

	//Si estaba autoload en top speed, desactivar

	if (fast_autoload.v) top_speed_timer.v=0;
}

z80_byte envia_load_pp_spectrum(z80_byte puerto_h);
z80_byte envia_load_spectrum_nextos(z80_byte puerto_h);

z80_byte envia_jload_pp_spectrum(z80_byte puerto_h)
{

//#define DURA_TECLA 20
//#define DURA_SILENCIO 15
#define DURA_TECLA 30
#define DURA_SILENCIO 22

#define SEQUENCE_ENTER1_MIN DURA_SILENCIO
#define SEQUENCE_ENTER1_MAX SEQUENCE_ENTER1_MIN+DURA_TECLA

#define SEQUENCE_J_MIN SEQUENCE_ENTER1_MAX+DURA_SILENCIO
#define SEQUENCE_J_MAX SEQUENCE_J_MIN+DURA_TECLA

#define SEQUENCE_SYM_MIN SEQUENCE_J_MAX+DURA_SILENCIO

#define SEQUENCE_P1_MIN SEQUENCE_SYM_MIN+DURA_SILENCIO
#define SEQUENCE_P1_MAX SEQUENCE_P1_MIN+DURA_TECLA

#define SEQUENCE_P2_MIN SEQUENCE_P1_MAX+DURA_SILENCIO*2
#define SEQUENCE_P2_MAX SEQUENCE_P2_MIN+DURA_TECLA

#define SEQUENCE_SYM_MAX SEQUENCE_P2_MAX+DURA_SILENCIO

#define SEQUENCE_ENTER2_MIN SEQUENCE_SYM_MAX+DURA_SILENCIO
#define SEQUENCE_ENTER2_MAX SEQUENCE_ENTER2_MIN+DURA_TECLA

			if (autoload_spectrum_loadpp_mode==2) {
				//L O A D "" para spectrum 128k spanish
				return envia_load_pp_spectrum(puerto_h);

			}

			if (autoload_spectrum_loadpp_mode==3) {
				//Cursor arriba una vez y enter dos veces para NextOS
				return envia_load_spectrum_nextos(puerto_h);

			}


                        if (initial_tap_sequence>SEQUENCE_ENTER1_MIN && initial_tap_sequence<SEQUENCE_ENTER1_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }

			//Si es modo 128k (solo enter) no enviar todo el load pp, solo el primer enter
			if (initial_tap_sequence>SEQUENCE_J_MIN && autoload_spectrum_loadpp_mode==0) {
				envia_jload_desactivar();
				return 255;
			}


                        if (initial_tap_sequence>SEQUENCE_J_MIN && initial_tap_sequence<SEQUENCE_J_MAX && puerto_h==191)  {
                                return 255-8; //J
                        }

                        if (initial_tap_sequence>SEQUENCE_SYM_MIN && initial_tap_sequence<SEQUENCE_SYM_MAX && puerto_h==127) {
                                return 255-2; //sym
                        }

                        if (initial_tap_sequence>SEQUENCE_P1_MIN && initial_tap_sequence<SEQUENCE_P1_MAX && puerto_h==223) {
                                return 255-1; //P
                        }

                        if (initial_tap_sequence>SEQUENCE_P2_MIN && initial_tap_sequence<SEQUENCE_P2_MAX && puerto_h==223) {
                                return 255-1; //P
                        }

                        if (initial_tap_sequence>SEQUENCE_ENTER2_MIN && initial_tap_sequence<SEQUENCE_ENTER2_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence<SEQUENCE_ENTER2_MAX) initial_tap_sequence++;
			else envia_jload_desactivar();

			return 255;
/*
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7
*/


//0=nada
//1 esperando a llegar a main-1 (48k) -> 0x12a9 y se enviara J
//10 se dejara pulsado sym. se enviara p
//20 se enviara p
//30 se libera sym
//40 se enviara ENTER


}


//Enviar L O A D " " para spectrum 128k spanish
z80_byte envia_load_pp_spectrum(z80_byte puerto_h)
{

//#define DURA2_TECLA 20
//#define DURA2_SILENCIO 15
#define DURA2_TECLA 30
#define DURA2_SILENCIO 22

#define SEQUENCE2_ENTER1_MIN DURA2_SILENCIO
#define SEQUENCE2_ENTER1_MAX SEQUENCE2_ENTER1_MIN+DURA2_TECLA*2

#define SEQUENCE2_L_MIN SEQUENCE2_ENTER1_MAX+DURA2_SILENCIO
#define SEQUENCE2_L_MAX SEQUENCE2_L_MIN+DURA2_TECLA

#define SEQUENCE2_O_MIN SEQUENCE2_L_MAX+DURA2_SILENCIO
#define SEQUENCE2_O_MAX SEQUENCE2_O_MIN+DURA2_TECLA

#define SEQUENCE2_A_MIN SEQUENCE2_O_MAX+DURA2_SILENCIO
#define SEQUENCE2_A_MAX SEQUENCE2_A_MIN+DURA2_TECLA

#define SEQUENCE2_D_MIN SEQUENCE2_A_MAX+DURA2_SILENCIO
#define SEQUENCE2_D_MAX SEQUENCE2_D_MIN+DURA2_TECLA


#define SEQUENCE2_SYM_MIN SEQUENCE2_D_MAX+DURA2_SILENCIO

#define SEQUENCE2_P1_MIN SEQUENCE2_SYM_MIN+DURA2_SILENCIO
#define SEQUENCE2_P1_MAX SEQUENCE2_P1_MIN+DURA2_TECLA

#define SEQUENCE2_P2_MIN SEQUENCE2_P1_MAX+DURA2_SILENCIO*2
#define SEQUENCE2_P2_MAX SEQUENCE2_P2_MIN+DURA2_TECLA

#define SEQUENCE2_SYM_MAX SEQUENCE2_P2_MAX+DURA2_SILENCIO

#define SEQUENCE2_ENTER2_MIN SEQUENCE2_SYM_MAX+DURA2_SILENCIO
#define SEQUENCE2_ENTER2_MAX SEQUENCE2_ENTER2_MIN+DURA2_TECLA



                        if (initial_tap_sequence>SEQUENCE2_ENTER1_MIN && initial_tap_sequence<SEQUENCE2_ENTER1_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence>SEQUENCE2_L_MIN && initial_tap_sequence<SEQUENCE2_L_MAX && puerto_h==191)  {
                                return 255-2; //L
                        }

                        if (initial_tap_sequence>SEQUENCE2_O_MIN && initial_tap_sequence<SEQUENCE2_O_MAX && puerto_h==223)  {
                                return 255-2; //O
                        }

                        if (initial_tap_sequence>SEQUENCE2_A_MIN && initial_tap_sequence<SEQUENCE2_A_MAX && puerto_h==253)  {
                                return 255-1; //A
                        }

                        if (initial_tap_sequence>SEQUENCE2_D_MIN && initial_tap_sequence<SEQUENCE2_D_MAX && puerto_h==253)  {
                                return 255-4; //D
                        }

                        if (initial_tap_sequence>SEQUENCE2_SYM_MIN && initial_tap_sequence<SEQUENCE2_SYM_MAX && puerto_h==127) {
                                return 255-2; //sym
                        }

                        if (initial_tap_sequence>SEQUENCE2_P1_MIN && initial_tap_sequence<SEQUENCE2_P1_MAX && puerto_h==223) {
                                return 255-1; //P
                        }

                        if (initial_tap_sequence>SEQUENCE2_P2_MIN && initial_tap_sequence<SEQUENCE2_P2_MAX && puerto_h==223) {
                                return 255-1; //P
                        }

                        if (initial_tap_sequence>SEQUENCE2_ENTER2_MIN && initial_tap_sequence<SEQUENCE2_ENTER2_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence<SEQUENCE2_ENTER2_MAX) initial_tap_sequence++;
                        else envia_jload_desactivar();

                        return 255;
/*
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7
//puerto_65022    db              255  ; G    F    D    S    A     ;1
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6

*/


}




//Enviar Espacio, cursor arriba una vez, enter dos veces para nextos
z80_byte envia_load_spectrum_nextos(z80_byte puerto_h)
{

#define DURA3_TECLA 30
#define DURA3_SILENCIO 22

#define SEQUENCE3_SPACE_MIN DURA3_SILENCIO
#define SEQUENCE3_SPACE_MAX SEQUENCE3_SPACE_MIN+DURA3_TECLA*14

#define SEQUENCE3_CURSOR_MIN SEQUENCE3_SPACE_MAX+DURA3_SILENCIO*5
#define SEQUENCE3_CURSOR_MAX SEQUENCE3_CURSOR_MIN+DURA3_TECLA

#define SEQUENCE3_ENTER1_MIN SEQUENCE3_CURSOR_MAX+DURA3_SILENCIO
#define SEQUENCE3_ENTER1_MAX SEQUENCE3_ENTER1_MIN+DURA3_TECLA

//Dado que es la misma tecla dos veces, hay que dar mas pausa (*3) para que detecte dos teclas separadas, y no la misma pulsada
#define SEQUENCE3_ENTER2_MIN SEQUENCE3_ENTER1_MAX+DURA3_SILENCIO*3
#define SEQUENCE3_ENTER2_MAX SEQUENCE3_ENTER2_MIN+DURA3_TECLA

                        if (initial_tap_sequence>SEQUENCE3_SPACE_MIN && initial_tap_sequence<SEQUENCE3_SPACE_MAX && puerto_h==127)  {
				//printf ("Enviando espacio\n");
                                return 255-1; //espacio
                        }


                        if (initial_tap_sequence>SEQUENCE3_CURSOR_MIN && initial_tap_sequence<SEQUENCE3_CURSOR_MAX && puerto_h==239)  {
				//printf ("Enviando cursor arriba\n");
                                return 255-8; //Cursor arriba
                        }



                        if (initial_tap_sequence>SEQUENCE3_ENTER1_MIN && initial_tap_sequence<SEQUENCE3_ENTER1_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence>SEQUENCE3_ENTER2_MIN && initial_tap_sequence<SEQUENCE3_ENTER2_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence<SEQUENCE3_ENTER2_MAX) initial_tap_sequence++;
                        else envia_jload_desactivar();

                        return 255;
/*
//Tablas teclado
z80_byte puerto_65278=255; //    db    255  ; V    C    X    Z    Sh    ;0
z80_byte puerto_65022=255; //    db    255  ; G    F    D    S    A     ;1
z80_byte puerto_64510=255; //    db              255  ; T    R    E    W    Q     ;2
z80_byte puerto_63486=255; //    db              255  ; 5    4    3    2    1     ;3
z80_byte puerto_61438=255; //    db              255  ; 6    7    8    9    0     ;4
z80_byte puerto_57342=255; //    db              255  ; Y    U    I    O    P     ;5
z80_byte puerto_49150=255; //    db              255  ; H                J         K      L    Enter ;6
z80_byte puerto_32766=255; //    db              255  ; B    N    M    Simb Space ;7

*/


}



z80_byte envia_load_pp_zx80(z80_byte puerto_h)
{

#define DURA_ZX80_TECLA 60
#define DURA_ZX80_SILENCIO 44

#define SEQUENCE_ZX80_ENTER1_MIN DURA_ZX80_SILENCIO
#define SEQUENCE_ZX80_ENTER1_MAX SEQUENCE_ZX80_ENTER1_MIN+DURA_ZX80_TECLA

#define SEQUENCE_ZX80_W_MIN SEQUENCE_ZX80_ENTER1_MAX+DURA_ZX80_SILENCIO
#define SEQUENCE_ZX80_W_MAX SEQUENCE_ZX80_W_MIN+DURA_ZX80_TECLA


#define SEQUENCE_ZX80_ENTER2_MIN SEQUENCE_ZX80_W_MAX+DURA_ZX80_SILENCIO
#define SEQUENCE_ZX80_ENTER2_MAX SEQUENCE_ZX80_ENTER2_MIN+DURA_ZX80_TECLA

                        //printf ("initial_tap_sequence zx80: %d\n",initial_tap_sequence);

                        if (initial_tap_sequence>SEQUENCE_ZX80_ENTER1_MIN && initial_tap_sequence<SEQUENCE_ZX80_ENTER1_MAX && puerto_h==191)  {
                                //no enviar enter inicial como en spectrum
                                //return 255-1; //ENTER
                        }

                        if (initial_tap_sequence>SEQUENCE_ZX80_W_MIN && initial_tap_sequence<SEQUENCE_ZX80_W_MAX && puerto_h==251)  {
                                return 255-2; //W
                        }


                        if (initial_tap_sequence>SEQUENCE_ZX80_ENTER2_MIN && initial_tap_sequence<SEQUENCE_ZX80_ENTER2_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence<SEQUENCE_ZX80_ENTER2_MAX) initial_tap_sequence++;
                        else envia_jload_desactivar();

                        return 255;
/*
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7
//puerto_65278    db              255  ; V    C    X    Z    Sh    ;0

//puerto_64510    db              255  ; T    R    E    W    Q     ;2

*/


}


z80_byte envia_load_pp_zx81(z80_byte puerto_h)
{

#define DURA_ZX81_TECLA 60
#define DURA_ZX81_SILENCIO 44

#define SEQUENCE_ZX81_ENTER1_MIN DURA_ZX81_SILENCIO
#define SEQUENCE_ZX81_ENTER1_MAX SEQUENCE_ZX81_ENTER1_MIN+DURA_ZX81_TECLA

#define SEQUENCE_ZX81_J_MIN SEQUENCE_ZX81_ENTER1_MAX+DURA_ZX81_SILENCIO
#define SEQUENCE_ZX81_J_MAX SEQUENCE_ZX81_J_MIN+DURA_ZX81_TECLA

#define SEQUENCE_ZX81_SYM_MIN SEQUENCE_ZX81_J_MAX+DURA_ZX81_SILENCIO

#define SEQUENCE_ZX81_P1_MIN SEQUENCE_ZX81_SYM_MIN+DURA_ZX81_SILENCIO
#define SEQUENCE_ZX81_P1_MAX SEQUENCE_ZX81_P1_MIN+DURA_ZX81_TECLA

#define SEQUENCE_ZX81_P2_MIN SEQUENCE_ZX81_P1_MAX+DURA_ZX81_SILENCIO*2
#define SEQUENCE_ZX81_P2_MAX SEQUENCE_ZX81_P2_MIN+DURA_ZX81_TECLA

#define SEQUENCE_ZX81_SYM_MAX SEQUENCE_ZX81_P2_MAX+DURA_ZX81_SILENCIO

#define SEQUENCE_ZX81_ENTER2_MIN SEQUENCE_ZX81_SYM_MAX+DURA_ZX81_SILENCIO
#define SEQUENCE_ZX81_ENTER2_MAX SEQUENCE_ZX81_ENTER2_MIN+DURA_ZX81_TECLA

                        //printf ("initial_tap_sequence zx81: %d\n",initial_tap_sequence);

                        if (initial_tap_sequence>SEQUENCE_ZX81_ENTER1_MIN && initial_tap_sequence<SEQUENCE_ZX81_ENTER1_MAX && puerto_h==191)  {
				//no enviar enter inicial como en spectrum
                                //return 255-1; //ENTER
                        }

                        if (initial_tap_sequence>SEQUENCE_ZX81_J_MIN && initial_tap_sequence<SEQUENCE_ZX81_J_MAX && puerto_h==191)  {
                                return 255-8; //J
                        }

                        if (initial_tap_sequence>SEQUENCE_ZX81_SYM_MIN && initial_tap_sequence<SEQUENCE_ZX81_SYM_MAX && puerto_h==254) {
                                return 255-1; //shift
                        }

                        if (initial_tap_sequence>SEQUENCE_ZX81_P1_MIN && initial_tap_sequence<SEQUENCE_ZX81_P1_MAX && puerto_h==223) {
                                return 255-1; //P
                        }

                        if (initial_tap_sequence>SEQUENCE_ZX81_P2_MIN && initial_tap_sequence<SEQUENCE_ZX81_P2_MAX && puerto_h==223) {
                                return 255-1; //P
                        }

                        if (initial_tap_sequence>SEQUENCE_ZX81_ENTER2_MIN && initial_tap_sequence<SEQUENCE_ZX81_ENTER2_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence<SEQUENCE_ZX81_ENTER2_MAX) initial_tap_sequence++;
                        else envia_jload_desactivar();

                        return 255;
/*
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7
//puerto_65278    db    	  255  ; V    C    X    Z    Sh    ;0

*/


}


z80_byte envia_load_ctrlenter_cpc(z80_byte index_keyboard_table)
{

#define DURA_CPC_TECLA 60
#define DURA_CPC_SILENCIO 44

#define SEQUENCE_CPC_CTRL_MIN DURA_CPC_SILENCIO
#define SEQUENCE_CPC_CTRL_MAX SEQUENCE_CPC_CTRL_MIN+DURA_CPC_TECLA

#define SEQUENCE_CPC_ENTER1_MIN SEQUENCE_CPC_CTRL_MAX+DURA_CPC_SILENCIO
#define SEQUENCE_CPC_ENTER1_MAX SEQUENCE_CPC_ENTER1_MIN+DURA_CPC_TECLA


#define SEQUENCE_CPC_ENTER2_MIN SEQUENCE_CPC_ENTER1_MAX+DURA_CPC_SILENCIO
#define SEQUENCE_CPC_ENTER2_MAX SEQUENCE_CPC_ENTER2_MIN+DURA_CPC_TECLA

                        //printf ("initial_tap_sequence CPC: %d\n",initial_tap_sequence);

                        if (initial_tap_sequence>SEQUENCE_CPC_CTRL_MIN && initial_tap_sequence<SEQUENCE_CPC_ENTER1_MAX && index_keyboard_table ==2)  {
                                //CTRL inicial se mantiene hasta antes del segundo enter
                                return 255-128; //CTRL
                        }

                        if (initial_tap_sequence>SEQUENCE_CPC_ENTER1_MIN && initial_tap_sequence<SEQUENCE_CPC_ENTER1_MAX && index_keyboard_table ==0)  {
                                return 255-64; //ENTER
                        }


                        if (initial_tap_sequence>SEQUENCE_CPC_ENTER2_MIN && initial_tap_sequence<SEQUENCE_CPC_ENTER2_MAX && index_keyboard_table ==0)  {
                                return 255-64; //ENTER
                        }


                        if (initial_tap_sequence<SEQUENCE_CPC_ENTER2_MAX) initial_tap_sequence++;
                        else {
				envia_jload_desactivar();
				debug_printf (VERBOSE_INFO,"End sending CTRL+Enter. Releasing all keys");
			}

                        return 255;
/*
&40     F Dot   ENTER   F3      F6      F9      CURDOWN CURRIGHT        CURUP
&41     F0      F2      F1      F5      F8      F7      COPY    CURLEFT
&42     CONTROL \       SHIFT   F4      ]       RETURN  [       CLR
&43     .       /        :       ;      P       @       -       ^
&44     ,       M       K       L       I       O       9       0
&45     SPACE   N       J       H       Y       U       7       8
&46     V       B       F       G (Joy2 fire)   T (Joy2 right)  R (Joy2 left)   5 (Joy2 down)   6 (Joy 2 up)
&47     X       C       D       S       W       E       3       4
&48     Z       CAPSLOCK        A       TAB     Q       ESC     2       1
&49     DEL     Joy 1 Fire 3 (CPC only) Joy 1 Fire 2    Joy1 Fire 1     Joy1 right      Joy1 left       Joy1 down       Joy1 up
*/
}


//Enviamos Enter y F8
/*
z80_byte envia_load_f8_sam(z80_byte puerto_h,z80_byte puerto_l)
{

#define DURA_SAM_TECLA 60
#define DURA_SAM_SILENCIO 44

#define SEQUENCE_SAM_ENTER_MIN DURA_SAM_SILENCIO
#define SEQUENCE_SAM_ENTER_MAX SEQUENCE_SAM_ENTER_MIN+DURA_SAM_TECLA

#define SEQUENCE_SAM_F8_MIN SEQUENCE_SAM_ENTER_MAX+DURA_SAM_SILENCIO
#define SEQUENCE_SAM_F8_MAX SEQUENCE_SAM_F8_MIN+DURA_SAM_TECLA


                        printf ("initial_tap_sequence SAM: %d\n",initial_tap_sequence);

                        if (initial_tap_sequence>SEQUENCE_SAM_ENTER_MIN && initial_tap_sequence<SEQUENCE_SAM_ENTER_MAX && puerto_l==254 && puerto_h==0xBF)  {
				printf ("Sending enter\n");
                                return 255-1; //ENTER
                        }

			if (initial_tap_sequence>SEQUENCE_SAM_F8_MIN && initial_tap_sequence<SEQUENCE_SAM_F8_MAX) {
				printf ("puerto_h %02X puerto_l %02X\n",puerto_h,puerto_l);
			}

                        //if (initial_tap_sequence>SEQUENCE_SAM_F8_MIN && initial_tap_sequence<SEQUENCE_SAM_F8_MAX && puerto_l==0xF9 && puerto_h==0xFB)  {
			//Manera peculiar de leer teclas sam coupe
                        if (initial_tap_sequence>SEQUENCE_SAM_F8_MIN && initial_tap_sequence<SEQUENCE_SAM_F8_MAX && puerto_l==0xF9 && (puerto_h&4)==0)  {
				printf ("Sending F8\n");
				initial_tap_sequence++;
                                return 255-64; //F8
                        }



                        if (initial_tap_sequence<SEQUENCE_SAM_F8_MAX) initial_tap_sequence++;
                        else {
                                envia_jload_desactivar();



				debug_printf (VERBOSE_INFO,"End sending F8. Releasing all keys");
                        }

                        return 255;

}

*/

z80_byte envia_load_comillas_sam(z80_byte puerto_h,z80_byte puerto_l)
{

#define DURASAM_TECLA 30
#define DURASAM_SILENCIO 22

#define SEQUENCE_SAM_ENTER1_MIN DURASAM_SILENCIO
#define SEQUENCE_SAM_ENTER1_MAX SEQUENCE_SAM_ENTER1_MIN+DURASAM_TECLA*10

#define SEQUENCE_SAM_L_MIN SEQUENCE_SAM_ENTER1_MAX+DURASAM_SILENCIO
#define SEQUENCE_SAM_L_MAX SEQUENCE_SAM_L_MIN+DURASAM_TECLA

#define SEQUENCE_SAM_O_MIN SEQUENCE_SAM_L_MAX+DURASAM_SILENCIO
#define SEQUENCE_SAM_O_MAX SEQUENCE_SAM_O_MIN+DURASAM_TECLA

#define SEQUENCE_SAM_A_MIN SEQUENCE_SAM_O_MAX+DURASAM_SILENCIO
#define SEQUENCE_SAM_A_MAX SEQUENCE_SAM_A_MIN+DURASAM_TECLA

#define SEQUENCE_SAM_D_MIN SEQUENCE_SAM_A_MAX+DURASAM_SILENCIO
#define SEQUENCE_SAM_D_MAX SEQUENCE_SAM_D_MIN+DURASAM_TECLA


//#define SEQUENCE_SAM_SYM_MIN SEQUENCE_SAM_D_MAX+DURASAM_SILENCIO

//#define SEQUENCE_SAM_P1_MIN SEQUENCE_SAM_SYM_MIN+DURASAM_SILENCIO
#define SEQUENCE_SAM_P1_MIN SEQUENCE_SAM_D_MAX+DURASAM_SILENCIO
#define SEQUENCE_SAM_P1_MAX SEQUENCE_SAM_P1_MIN+DURASAM_TECLA

#define SEQUENCE_SAM_P2_MIN SEQUENCE_SAM_P1_MAX+DURASAM_SILENCIO*2
#define SEQUENCE_SAM_P2_MAX SEQUENCE_SAM_P2_MIN+DURASAM_TECLA

#define SEQUENCE_SAM_SYM_MAX SEQUENCE_SAM_P2_MAX+DURASAM_SILENCIO

#define SEQUENCE_SAM_ENTER2_MIN SEQUENCE_SAM_SYM_MAX+DURASAM_SILENCIO
#define SEQUENCE_SAM_ENTER2_MAX SEQUENCE_SAM_ENTER2_MIN+DURASAM_TECLA

	                //printf ("initial_tap_sequence SAM: %d\n",initial_tap_sequence);
                        //printf ("puerto_h %02X puerto_l %02X\n",puerto_h,puerto_l);


			if (initial_tap_sequence>SEQUENCE_SAM_ENTER1_MIN && initial_tap_sequence<SEQUENCE_SAM_ENTER1_MAX && puerto_h==191)  {
                                return 255-1; //ENTER
                        }


                        if (initial_tap_sequence>SEQUENCE_SAM_L_MIN && initial_tap_sequence<SEQUENCE_SAM_L_MAX && puerto_h==191 && puerto_l==254)  {
                                return 255-2; //L
                        }

                        if (initial_tap_sequence>SEQUENCE_SAM_O_MIN && initial_tap_sequence<SEQUENCE_SAM_O_MAX && puerto_h==223 && puerto_l==254)  {
                                return 255-2; //O
                        }

                        if (initial_tap_sequence>SEQUENCE_SAM_A_MIN && initial_tap_sequence<SEQUENCE_SAM_A_MAX && puerto_h==253 && puerto_l==254)  {
                                return 255-1; //A
                        }

                        if (initial_tap_sequence>SEQUENCE_SAM_D_MIN && initial_tap_sequence<SEQUENCE_SAM_D_MAX && puerto_h==253 && puerto_l==254)  {
                                return 255-4; //D
                        }

                        //if (initial_tap_sequence>SEQUENCE_SAM_SYM_MIN && initial_tap_sequence<SEQUENCE_SAM_SYM_MAX && puerto_h==254) {
                        //        return 255-1; //shift
                        //}

                        if (initial_tap_sequence>SEQUENCE_SAM_P1_MIN && initial_tap_sequence<SEQUENCE_SAM_P1_MAX && puerto_h==0xdf && puerto_l==0xf9) {
                                return 255-64; //"
                        }

                        if (initial_tap_sequence>SEQUENCE_SAM_P2_MIN && initial_tap_sequence<SEQUENCE_SAM_P2_MAX && puerto_h==0xdf && puerto_l==0xf9) {
                                return 255-64; //"
                        }

                        if (initial_tap_sequence>SEQUENCE_SAM_ENTER2_MIN && initial_tap_sequence<SEQUENCE_SAM_ENTER2_MAX && puerto_h==191 && puerto_l==254)  {
                                return 255-1; //ENTER
                        }
			 if (initial_tap_sequence<SEQUENCE_SAM_ENTER2_MAX) initial_tap_sequence++;
                        else envia_jload_desactivar();

                        return 255;
/*
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7
//puerto_65022    db              255  ; G    F    D    S    A     ;1
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6

*/


}



z80_byte lee_puerto_spectrum(z80_byte puerto_h,z80_byte puerto_l)
{
  z80_int port=value_8_to_16(puerto_h,puerto_l);
  ula_contend_port_early( port );
  ula_contend_port_late( port );
  z80_byte valor = lee_puerto_spectrum_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}


z80_byte idle_bus_port_atribute_aux(void)
{

	//Retorna el byte que lee la ULA
	//de momento solo retornar el ultimo atributo, teniendo en cuenta que el rainbow debe estar habilitado
	//TODO: retornar tambien byte de pixeles
	//Si no esta habilitado rainbow, retornara algun valor aleatorio fijo

	//TODO: mirar que usa fuse en funcion spectrum_unattached_port


	//Si no esta habilitado rainbow y hay que detectar rainbow, habilitar rainbow, siempre que no este en ROM
	if (rainbow_enabled.v==0) {
		if (autodetect_rainbow.v) {
			if (reg_pc>=16384) {
				debug_printf(VERBOSE_INFO,"Autoenabling realvideo so the program seems to need it (Idle bus port reading on Spectrum)");
				enable_rainbow();
			}
		}

	}

//printf ("last ula attribute: %d t_states: %d\n",last_ula_attribute,t_estados);
	//Si estamos en zona de border, retornar 255
	/*
	Due to the resistors which decouple the two dedicated buses inside the Spectrum, when the Z80 reads from an unattached port
	(such as 0xFF) it actually reads the data currently present on the ULA bus, which may happen to be a byte being transferred
	by the ULA from video memory for the screen rendering. If the ULA is building the border, then its data bus is idle (0xFF),
	otherwise we are perfectly able to predict whether it is reading a bitmap byte, an attribute or again it's idle.
	This "nice" effect goes under the name of floating bus. Unfortunately some programs do rely on the exact behaviour of the
	floating bus, so we can't simply forget about it; notable examples are Arkanoid (first edition), Cobra, Sidewize, Duet and DigiSynth.

    https://web.archive.org/web/20080509193736/http://www.ramsoft.bbk.org/floatingbus.html

    The ULA reads a whole row of pixels and attributes using 16 x 8T cycles, the first of which starts at 14368
    on the 128K and 14347 on the 48K. The screen bytes are fetched during the first 4T of each cycle
    (order: bitmap, attribute, bitmap+1, attribute+1), while in the last 4T the bus is idle (0xFF).
    After that 128T, the ULA is busy drawing the border and it does not read the video memory,
    so it leaves the bus idle for the remaining 228-128 = 100T (or 224-128 = 96T on the 48K).
    Then, the second row of pixels is started to be read at 14368+228 = 14596T (or 14347+224 = 14571T on the 48K)
    with the same scheme, and so on until the 192th row is finished.


    However, the timings used are based on the first byte being
    returned at 14338 (48K) and 14364 (128K) respectively, not
    14347 and 14368 as used by Ramsoft.
	*/
	int t_estados_en_linea=(t_estados % screen_testados_linea);
	if (t_estados_en_linea>=128) return get_ula_databus_value();

	switch( t_estados_en_linea % 8 ) {
		/* Attribute bytes */
		case 5:
		case 3:
			return last_ula_attribute;
		break;

		/* Screen data */
		case 4:
		case 2:
		  //printf ("pixel: %d\n",last_ula_pixel);
			return last_ula_pixel;
		break;

		default:
		/* Idle bus */
			return get_ula_databus_value();
		break;

	}


}

z80_byte idle_bus_port_atribute(void)
{
	z80_byte valor=idle_bus_port_atribute_aux();

	if (dinamic_sd1.v) valor=valor & (255-32);

	return valor;
}

z80_byte idle_bus_port(z80_int puerto)
{

	//debug_printf(VERBOSE_DEBUG,"Idle bus port reading: %x",puerto);

	//Caso Inves, siempre 255
	if (MACHINE_IS_INVES) return 255;

    if (MACHINE_IS_ZXUNO) {
        //cuando el puerto 1FFD está operativo, el puerto FF devuelve FF
        z80_byte devcontrol=zxuno_ports[0x0E];
        //DISD  ENMMU   DIROMSEL1F  DIROMSEL7F  DI1FFD  DI7FFD  DITAY   DIAY
        //DI1FFD: a 1 para deshabilitar el sistema de paginación compatible con +2A/+3.
        //Deshabilitando esta interfaz se libera el puerto $1FFD en escritura.
        //Tenga en cuenta que la decodificación del puerto $7FFD, si está activa, es diferente dependiendo de si el puerto $1FFD está activo o no.
        if (devcontrol&8) { //A 1, esta deshabilitado puerto 1ffd
            z80_byte valor_idle=idle_bus_port_atribute();
            return valor_idle;
        }
        else return 255;

    }

	//Caso 48k, 128k, tsconf. Retornar atributo de pantalla
	//Parece que si en tsconf no retornamos esto, no acaba de arrancar la bios
	if (MACHINE_IS_SPECTRUM_16_48 || MACHINE_IS_SPECTRUM_128_P2 || MACHINE_IS_TBBLUE || MACHINE_IS_PRISM || MACHINE_IS_TSCONF) {
		z80_byte valor_idle=idle_bus_port_atribute();

		//int t_estados_en_linea=(t_estados % screen_testados_linea);

		//int cc=t_estados_en_linea % 8;

		//printf ("last ula attribute: %d t_states: %d\n",valor_idle,t_estados);
		return valor_idle;
	}

	//Caso +2A
	/*
When the paging is disabled (bit 5 of port 32765 set to 1), we will
always read 255; when it is not, we will have the byte that the ULA reads
(with bit 0 set) if the port number has the following mask:
0000XXXXXXXXXX01b, I mean, starting from port 1, stepping 4, until port 4093
(1,5,9,13,..., 4093)

//valor:
0000000000000001b=1

//mascara:
1111000000000011b=61443

	*/
	if (MACHINE_IS_SPECTRUM_P2A_P3) {
		//Paginacion deshabilitada
		if ( (puerto_32765&32) == 32) return 255;

		if ( (puerto&61443) == 1) return ( idle_bus_port_atribute() | 1 );

	}


	//Resto de casos o maquinas, retornamos 255
	return 255;

}

//Teclado en Spectrum y Jupiter Ace es casi igual excepto en la fila inferior de teclas
void jupiter_ace_adapta_teclas_fila_inferior(z80_byte *valor_puerto_65278,z80_byte *valor_puerto_32766)
{
//Spectrum:
//puerto_65278    db              255  ; V    C    X    Z    Sh    ;0
//puerto_32766    db              255  ; B    N    M    Simb Space ;7


//Jupiter Ace
//puerto_65278    db              255  ; C    X    Z    Simb Sh
//puerto_32766    db              255  ; V    B    N    M    Space

	z80_byte p65278,p32766;

	//Obtener cada tecla por separado en formato teclado spectrum y luego convertirlo a teclas jupiterace
	z80_byte tecla_sh,tecla_z,tecla_x,tecla_c,tecla_v,tecla_space,tecla_simb,tecla_m,tecla_n,tecla_b;

	tecla_sh=(puerto_65278&1 ? 1 :0 );
	tecla_z=(puerto_65278&2 ? 1 :0 );
	tecla_x=(puerto_65278&4 ? 1 :0 );
	tecla_c=(puerto_65278&8 ? 1 :0 );
	tecla_v=(puerto_65278&16 ? 1 :0 );

	tecla_space=(puerto_32766&1 ? 1 :0 );
	tecla_simb=(puerto_32766&2 ? 1 :0 );
	tecla_m=(puerto_32766&4 ? 1 :0 );
	tecla_n=(puerto_32766&8 ? 1 :0 );
	tecla_b=(puerto_32766&16 ? 1 :0 );

	//Pasarlas a puertos de jupiter ace
	p65278=tecla_sh | (tecla_simb<<1) | (tecla_z<<2) | (tecla_x<<3) | (tecla_c<<4);
	p32766=tecla_space | (tecla_m<<1) | (tecla_n<<2) | (tecla_b<<3) | (tecla_v<<4);


	*valor_puerto_65278=p65278;
	*valor_puerto_32766=p32766;


}

z80_byte jupiter_ace_retorna_puerto_65278(void)
{
	z80_byte a,b;

	jupiter_ace_adapta_teclas_fila_inferior(&a,&b);

	return a;
}

z80_byte jupiter_ace_retorna_puerto_32766(void)
{
        z80_byte a,b;

        jupiter_ace_adapta_teclas_fila_inferior(&a,&b);

        return b;
}

z80_byte teclado_and_todas(z80_byte valor)
{
     int ceros=0;
                ceros +=util_return_ceros_byte(puerto_65278|224); //Valor de fila teclado poniendo los otros bits a 1
                ceros +=util_return_ceros_byte(puerto_65022|224);
                ceros +=util_return_ceros_byte(puerto_64510|224);
                ceros +=util_return_ceros_byte(puerto_63486|224);
                ceros +=util_return_ceros_byte(puerto_61438|224);
                ceros +=util_return_ceros_byte(puerto_57342|224);
                ceros +=util_return_ceros_byte(puerto_49150|224);
                ceros +=util_return_ceros_byte(puerto_32766|224);

                if (ceros>2) {
                    valor &= puerto_65278 & puerto_65022 & puerto_64510 & puerto_63486 & puerto_61438 & puerto_57342 & puerto_49150 & puerto_32766;
                }
    return valor;
}

z80_byte teclado_return_valor_fila(z80_byte fila)
{
    switch (fila) {
        case 0:
            return puerto_65278;
        break;

        case 1:
            return puerto_65022;
        break;

        case 2:
            return puerto_64510;
        break;

        case 3:
            return puerto_63486;
        break;

        case 4:
            return puerto_61438;
        break;

        case 5:
            return puerto_57342;
        break;

        case 6:
            return puerto_49150;
        break;

        case 7:
            return puerto_32766;
        break;

        default:
            //no deberia suceder
            return 255;
        break;


    }
}

/*
Lo que ocurre exactamente es lo siguiente: cuando se pulsan dos teclas que pertenecen a distintas filas,
pero que pertenecen a la misma columna (como la A y la G en el ejemplo), las filas de ambas teclas adquieren
el potencial de 0 voltios, así que aunque nosotros hayamos seleccionado un fila para leer, en realidad se
están seleccionando dos filas para leer. Si en la fila que no pretendíamos leer hay más de una tecla pulsada (la I),
ésta obviamente aparecerá en la línea de salida.
*/


//Para una fila que vamos a leer, comparamos si con el resto de filas, coinciden teclas en misma columna
//si es asi, se agregara esa fila para leer en el puerto final

//Para saber si se pulsan teclas de misma columna, hacer xor a nivel de bit

z80_byte teclado_matrix_que_filas(z80_byte fila,z80_byte puerto_h)
{
    int i;

    z80_byte valor_fila_leida=teclado_return_valor_fila(fila);
    z80_byte mascara_fila=254; //11111110 realmente movemos el 0 a la izquierda

    for (i=0;i<8;i++) {
        if (i!=fila) { //No comparar fila con ella misma
            int bit;
            int mascara_bit=1;
            for (bit=0;bit<5;bit++) {
                //Si se ha pulsado esa tecla en la fila que estamos observando
                if ((valor_fila_leida&mascara_bit)==0) {
                    //Ver si se ha pulsado tambien tecla en la fila con la que comparamos
                    if ((teclado_return_valor_fila(i)&mascara_bit)==0) {
                        //Leeremos fila adicional
                        puerto_h &=mascara_fila;
                    }
                }
                mascara_bit=mascara_bit<<1;
            }
        }
        mascara_fila=(mascara_fila<<1)|1; //Bit que resetearemos para leer fila adicional si conviene
    }

    return puerto_h;
}

//Que puerto o puertos se leeran finalmente aplicando bug de matrix error
z80_byte teclado_matrix_puerto_final(z80_byte puerto_h)
{
    int mascara=1;
    int i;

    z80_byte final_puerto_h=puerto_h;

    if (MACHINE_IS_SPECTRUM && keyboard_matrix_error.v) {

        //char puerto_binario[9];

        //util_byte_to_binary(puerto_h,puerto_binario);
        //printf ("Port to read: %02XH (%s) ",puerto_h,puerto_binario);

        for (i=0;i<8;i++) {
          if ((puerto_h&mascara)==0) final_puerto_h=teclado_matrix_que_filas(i,puerto_h);
          mascara=mascara<<1;
        }

        //util_byte_to_binary(final_puerto_h,puerto_binario);
        //printf ("Port finally read: %02XH (%s)\n",final_puerto_h,puerto_binario);

    }



    return final_puerto_h;

}

//comun para spectrum y zx80/81 y sam y ace
z80_byte lee_puerto_teclado(z80_byte puerto_h)
{

    z80_byte acumulado;

    if (initial_tap_load.v==1 && initial_tap_sequence) {
        if (MACHINE_IS_SPECTRUM) {
            return envia_jload_pp_spectrum(puerto_h);
        }
        else {
            if (MACHINE_IS_ZX80_TYPE) return envia_load_pp_zx80(puerto_h);
            else return envia_load_pp_zx81(puerto_h);
        }
    }

    //puerto teclado

    //si estamos en el menu, no devolver tecla
    if (zxvision_key_not_sent_emulated_mach() ) return 255;


    //Si esta spool file activo, generar siguiente tecla
    if (input_file_keyboard_is_playing() ) {
        if (input_file_keyboard_turbo.v==0) {
            input_file_keyboard_get_key();
        }

        else {
            //en modo turbo enviamos cualquier tecla pero la rom realmente lee de la direccion 23560
            ascii_to_keyboard_port(' ');
        }
    }



    acumulado=255;

    puerto_h=teclado_matrix_puerto_final(puerto_h);

    //A zero in one of the five lowest bits means that the corresponding key is pressed.
    //If more than one address line is made low, the result is the logical AND of all single inputs,
    //so a zero in a bit means that at least one of the appropriate keys is pressed.
    //For example, only if each of the five lowest bits of the result from reading from Port 00FE
    //(for instance by XOR A/IN A,(FE)) is one, no key is pressed

    //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
    if ((puerto_h & 1) == 0)   {
        if (MACHINE_IS_ACE) {
            acumulado &=jupiter_ace_retorna_puerto_65278();
        }
        else acumulado &=puerto_65278;

        //acumulado=teclado_matrix_error(puerto_65278,acumulado);

        //Si hay alguna tecla del joystick cursor pulsada, enviar tambien shift
        if (joystick_emulation==JOYSTICK_CURSOR_WITH_SHIFT && puerto_especial_joystick!=0) {
            acumulado &=(255-1);
        }
    }

    //puerto_65022   db    255  ; G    F    D    S    A     ;1
    if ((puerto_h & 2) == 0)   {
        acumulado &=puerto_65022;
        //acumulado=teclado_matrix_error(puerto_65022,acumulado);

        //OPQASPACE Joystick
        if (joystick_emulation==JOYSTICK_OPQA_SPACE) {
                if ((puerto_especial_joystick&4)) acumulado &=(255-1);
        }

    }

    //puerto_64510    db              255  ; T    R    E    W    Q     ;2
    if ((puerto_h & 4) == 0)   {
        acumulado &=puerto_64510;
        //acumulado=teclado_matrix_error(puerto_64510,acumulado);

        //OPQASPACE Joystick
        if (joystick_emulation==JOYSTICK_OPQA_SPACE) {
                if ((puerto_especial_joystick&8)) acumulado &=(255-1);
        }

    }



//z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

    //Para cursor, sinclair joystick
    //z80_byte puerto_63486=255; //    db              255  ; 5    4    3    2    1     ;3
    if ((puerto_h & 8) == 0)   {
        acumulado &=puerto_63486;
        //acumulado=teclado_matrix_error(puerto_63486,acumulado);
        //sinclair 2 joystick
        if (joystick_emulation==JOYSTICK_SINCLAIR_2) {
            if ((puerto_especial_joystick&1)) acumulado &=(255-2);
            if ((puerto_especial_joystick&2)) acumulado &=(255-1);
            if ((puerto_especial_joystick&4)) acumulado &=(255-4);
            if ((puerto_especial_joystick&8)) acumulado &=(255-8);
            if ((puerto_especial_joystick&16)) acumulado &=(255-16);
        }

        //cursor joystick 5 iz 8 der 6 abajo 7 arriba 0 fire
        if (joystick_emulation==JOYSTICK_CURSOR || joystick_emulation==JOYSTICK_CURSOR_WITH_SHIFT) {
            //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

            //Left
            if ((puerto_especial_joystick&2)) acumulado &=(255-16);
        }

        //gunstick
        if (gunstick_emulation==GUNSTICK_SINCLAIR_2) {
            if (mouse_left!=0) {

                acumulado &=(255-1);

                if (gunstick_view_white()) acumulado &=(255-4);


            }
        }

    }

    //z80_byte puerto_61438=255; //    db              255  ; 6    7    8    9    0     ;4
    if ((puerto_h & 16) == 0)  {
        acumulado &=puerto_61438;
        //acumulado=teclado_matrix_error(puerto_61438,acumulado);

        //sinclair 1 joystick
        if (joystick_emulation==JOYSTICK_SINCLAIR_1) {
            //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
            if ((puerto_especial_joystick&1)) acumulado &=(255-8);
            if ((puerto_especial_joystick&2)) acumulado &=(255-16);
            if ((puerto_especial_joystick&4)) acumulado &=(255-4);
            if ((puerto_especial_joystick&8)) acumulado &=(255-2);
            if ((puerto_especial_joystick&16)) acumulado &=(255-1);
        }
        //cursor joystick 5 iz 8 der 6 abajo 7 arriba 0 fire
        if (joystick_emulation==JOYSTICK_CURSOR  || joystick_emulation==JOYSTICK_CURSOR_WITH_SHIFT) {
            //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

            //Right -> 8
            if ((puerto_especial_joystick&1)) acumulado &=(255-4);

            //Down -> 6
            if ((puerto_especial_joystick&4)) {
                //En jupiter ace es con el 7
                if (MACHINE_IS_ACE) acumulado &=(255-8);
                else acumulado &=(255-16);
            }

            //Up -> 7
            if ((puerto_especial_joystick&8)) {
                //En jupiter ace es con el 6
                if (MACHINE_IS_ACE) acumulado &=(255-16);
                else acumulado &=(255-8);
            }

            //Fire -> 0
            if ((puerto_especial_joystick&16)) acumulado &=(255-1);
        }


        //gunstick
        if (gunstick_emulation==GUNSTICK_SINCLAIR_1) {
            if (mouse_left!=0) {

                acumulado &=(255-1);

                if (gunstick_view_white()) acumulado &=(255-4);


            }
        }
    }

    //puerto_57342    db              255  ; Y    U    I    O    P     ;5
    if ((puerto_h & 32) == 0)  {
        acumulado &=puerto_57342;
        //acumulado=teclado_matrix_error(puerto_57342,acumulado);

        //OPQASPACE Joystick
        if (joystick_emulation==JOYSTICK_OPQA_SPACE) {
                if ((puerto_especial_joystick&1)) acumulado &=(255-1);
                if ((puerto_especial_joystick&2)) acumulado &=(255-2);
        }


    }

    //puerto_49150    db              255  ; H                J         K      L    Enter ;6
    if ((puerto_h & 64) == 0)  {
        acumulado &=puerto_49150;
        //acumulado=teclado_matrix_error(puerto_49150,acumulado);
    }

    //puerto_32766    db              255  ; B    N    M    Simb Space ;7
    if ((puerto_h & 128) == 0) {
        if (MACHINE_IS_ACE) {
            acumulado &=jupiter_ace_retorna_puerto_32766();
        }
        else acumulado &=puerto_32766;

        //acumulado=teclado_matrix_error(puerto_32766,acumulado);

        //OPQASPACE Joystick
        if (joystick_emulation==JOYSTICK_OPQA_SPACE) {
            if ((puerto_especial_joystick&16)) acumulado &=(255-1);
        }

    }


    return acumulado;


}


//Devuelve valor para el puerto de kempston (joystick o gunstick kempston)
z80_byte get_kempston_value(void)
{

                        z80_byte acumulado=0;

                        //si estamos con menu abierto, no retornar nada
                        if (zxvision_key_not_sent_emulated_mach() ) return 0;

                        if (joystick_emulation==JOYSTICK_KEMPSTON) {
                                //mapeo de ese puerto especial es igual que kempston
                                acumulado=puerto_especial_joystick;
                        }

                       //gunstick
                       if (gunstick_emulation==GUNSTICK_KEMPSTON) {

                                if (mouse_left!=0) {

                                    acumulado |=16;

                                    if (gunstick_view_white()) acumulado |=4;

                                }
                        }

                        return acumulado;
}

//Ultimo valor de dicho bit
z80_byte last_bit_6_feh=0;

z80_byte get_last_bit_6_feh(void)
{

    z80_byte valor=0;

    int leer_cinta_real=0;

    if (realtape_inserted.v && realtape_playing.v) leer_cinta_real=1;

    if (audio_can_record_input()) {
        if (audio_is_recording_input) {
            leer_cinta_real=1;
        }
    }


    if (leer_cinta_real) {
        if (realtape_get_current_bit_playing()) {
                valor=64;
                //printf ("1 ");
        }
        else {
                valor=0;
                //printf ("0 ");
        }
    }

    else {
        //devolver ultimo valor
        valor=last_bit_6_feh;
    }

    last_bit_6_feh=valor;

    //printf("last valor: %d\n",last_bit_6_feh);

    return valor;
}

z80_byte lee_puerto_spectrum_ula(z80_byte puerto_h)
{

    //Si deteccion de localidades activa
    textadv_location_desc_read_keyboard_port();


    z80_byte valor;
    valor=lee_puerto_teclado(puerto_h);


    //Dinamic SD1
    if (dinamic_sd1.v) valor=valor & (255-32);

    valor=(valor & (255-64));
    valor=valor | get_last_bit_6_feh();



    //Lectura de cinta desde cable
    /*if (audio_can_record_input()) {
        if (audio_is_recording_input) {
            if (audio_last_record_input_sample>0) valor=valor|64;
            else valor=(valor & (255-64));
        }
    }*/

    //Si esta cinta insertada en realtape, acelerar
    if (accelerate_loaders.v && realtape_inserted.v && realtape_playing.v) {
        //Mirar que esta haciendo
        tape_check_known_loaders();

        if( acceleration_mode ) {

            //Si no estaba modo turbo, poner turbo
            if (top_speed_timer.v==0) {
                debug_printf (VERBOSE_DEBUG,"Setting Top speed");
                top_speed_timer.v=1;
                draw_tape_text_top_speed();
            }
        }

        //Si no esta en cargador, pero estabamos en turbo, detener
        else {
            if (top_speed_timer.v) {
                debug_printf (VERBOSE_DEBUG,"Resetting top speed");
                top_speed_timer.v=0;
            }
        }

    }

    //Miramos si estamos en una rutina de carga de cinta
    //Y esta cinta standard

    if (standard_to_real_tape_fallback.v) {

        //Por defecto detectar real tape
        int detectar=1;

        //Si ya hay una cinta insertada y en play, no hacerlo
        if (realtape_inserted.v && realtape_playing.v) detectar=0;

        //Si no hay cinta standard, no hacerlo
        if (tapefile==0) detectar=0;

        //Si no esta cinta standard insertada, no hacerlo
        if ((tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) detectar=0;

        if (detectar) tape_detectar_realtape();

    }

    return valor;


}

z80_byte betadisk_temp_puerto_1f=0;
z80_byte betadisk_temp_puerto_3f=0;
z80_byte betadisk_temp_puerto_5f=0;
z80_byte betadisk_temp_puerto_7f=0;



z80_byte temp_tsconf_first_sd_0=1;


//Devuelve valor puerto para maquinas Spectrum
z80_byte lee_puerto_spectrum_no_time(z80_byte puerto_h,z80_byte puerto_l)
{

    //printf ("In Port %x asked, PC after=0x%x\n",puerto_l+256*puerto_h,reg_pc);

	debug_fired_in=1;
	//extern z80_byte in_port_ay(z80_int puerto);
	//65533 o 49149
	//FFFDh (65533), BFFDh (49149)

	if (rzx_reproduciendo && !zxvision_key_not_sent_emulated_mach() ) {
		z80_byte retorno;
		if (rzx_lee_puerto(&retorno)) return retorno;
	}

	z80_int puerto=value_8_to_16(puerto_h,puerto_l);


	if (puerto_l==0xFD) {
		if (puerto_h==0xFF) {
			activa_ay_chip_si_conviene();
			if (ay_chip_present.v==1) return in_port_ay(puerto_h);
		}

		//Puertos disco +3
		if (pd765_enabled.v) {
            //Tipicamente 2ffd
            //  2FFD 0010..........0. R   Spectrum +3 Floppy FDC NEC uPD765 status
			if ((puerto & 0xF002) == 0x2000) return pd765_read_status_register();

            //Tipicamente 3ffd
            //  3FFD 0011..........0. R/W Spectrum +3 Floppy FDC NEC uPD765 data
            if ((puerto & 0xF002) == 0x3000) return pd765_read();
		}

        //Con handler
        else if (plus3dos_traps.v) {
            //Bit Request for Master
			if (puerto_h==0x2F) return 0x80;

			if (puerto_h==0x3F) return 255;
        }

		else {
			if (puerto_h==0x2F) return 255;
			if (puerto_h==0x3F) return 255;
		}

	}

    //Test betadisk
    if (betadisk_enabled.v) {

        if (betadisk_check_if_rom_area(reg_pc)) {

            z80_byte return_value;

           if (puerto_l==0xFF) {
               return_value=255;
               //debug_printf (VERBOSE_DEBUG,"Reading port betadisk %02X%02XH Beta Disk Status PC=%04XH return=%02XH",puerto_h,puerto_l,reg_pc,return_value);

               return return_value;
          }

         if (puerto_l==0x1f) {
                betadisk_temp_puerto_1f ^=2;
                return_value=betadisk_temp_puerto_1f;
                //debug_printf (VERBOSE_DEBUG,"Reading port betadisk %02X%02XH Beta Disk FDC Status PC=%04XH return=%02XH",puerto_h,puerto_l,reg_pc,return_value);

              return return_value; //Parece que al conmutar bit 1 al menos detecta que hay disco, aunque luego da error
          }

         if (puerto_l==0x3f) {
               return_value=betadisk_temp_puerto_3f;
               //debug_printf (VERBOSE_DEBUG,"Reading port betadisk %02X%02XH Beta Disk FDC Track PC=%04XH return=%02XH",puerto_h,puerto_l,reg_pc,return_value);

               return return_value;
         }

         if (puerto_l==0x5f) {
               return_value=betadisk_temp_puerto_5f;
               //debug_printf (VERBOSE_DEBUG,"Reading port betadisk %02X%02XH Beta Disk FDC Sector PC=%04XH return=%02XH",puerto_h,puerto_l,reg_pc,return_value);

               return return_value;
         }


         if (puerto_l==0x7f) {
                return_value=betadisk_temp_puerto_7f;
                //debug_printf (VERBOSE_DEBUG,"Reading port betadisk %02X%02XH Beta Disk FDC Data PC=%04XH return=%02XH",puerto_h,puerto_l,reg_pc,return_value);

               return return_value;
         }
     }
    }


	//Esto tiene que estar antes de zxmmc y fuller dado que interfiere con puerto 3fh
		if (multiface_enabled.v) {
			/*
			xx9F ........             Multiface I In
	xx1F ........             Multiface I Out
	xxBF ........             Multiface 128 In
	xx9F ........             Multiface 128 In v2 (Disciple) (uh/what?)
	xx3F ........             Multiface 128 Out
	xx3F ........             Multiface III Button
	xx3F ........             Multiface III In
	xxBF ........             Multiface III Out
	7F3F                      Multiface III P7FFD (uh?)
	1F3F                      Multiface III P1FFD (uh?)
	FF3F                      British Micro Grafpad Pen up/
			*/

			if (puerto_l==0x1f && multiface_type==MULTIFACE_TYPE_ONE) {
				multiface_unmap_memory();
			}

			if (puerto_l==0x3f) {
				switch(multiface_type)
				{
					case MULTIFACE_TYPE_128:
						multiface_unmap_memory();
						return 0xff;
					break;

					case MULTIFACE_TYPE_THREE:
						multiface_map_memory();
						if (puerto_h==0x7f) return puerto_32765;
						if (puerto_h==0x1f) return puerto_8189;
						return 0xff;
					break;
				}

			}

			if (puerto_l==0x9f && multiface_type==MULTIFACE_TYPE_ONE) {
				multiface_map_memory();
			}

			if (puerto_l==0xbf) {
				switch(multiface_type)
				{
					case MULTIFACE_TYPE_128:
						if (!multiface_lockout) {
							multiface_map_memory();
							return (puerto_32765&8)<<4;
						}
						return 0xff;
					break;

					case MULTIFACE_TYPE_THREE:
						multiface_unmap_memory();
						return 0xff;
					break;
				}
			}


		}




	//Fuller audio box. Interfiere con MMC
	if (zxmmc_emulation.v==0 && (puerto_l==0x3f && puerto_h==0)) {

		activa_ay_chip_si_conviene();
		if (ay_chip_present.v==1) {

			return in_port_ay(0xFF);

		}

	}


	//Fuller Joystick
	//The Fuller Audio Box included a joystick interface. Results were obtained by reading from port 0x7f in the form F---RLDU, with active bits low.
        if (puerto_l==0x7f) {
                if (joystick_emulation==JOYSTICK_FULLER) {
			z80_byte valor_joystick=255;
			//si estamos con menu abierto, no retornar nada
			if (zxvision_key_not_sent_emulated_mach() ) return valor_joystick;


			if ((puerto_especial_joystick&1)) valor_joystick &=(255-8);
			if ((puerto_especial_joystick&2)) valor_joystick &=(255-4);
			if ((puerto_especial_joystick&4)) valor_joystick &=(255-2);
			if ((puerto_especial_joystick&8)) valor_joystick &=(255-1);
			if ((puerto_especial_joystick&16)) valor_joystick &=(255-128);

                        return valor_joystick;
                }

        }

	//zx printer
	if (puerto_l==0xFB) {
                if (zxprinter_enabled.v==1) {
			return zxprinter_get_port();
                }


        }

	//ulaplus
	if (ulaplus_presente.v && puerto_l==0x3B && puerto_h==0xFF) {
		return ulaplus_return_port_ff3b();
	}


	//Puerto DF parece leerse desde juego Bestial Warrior en el menu, aunque dentro del juego no se lee
	//Quiza hace referencia a la Stack Light Rifle

	//Dejarlo comentado dado que entra en conflicto con kempston mouse

	/*

	if ( puerto_l==0xdf) {
		//gunstick.
//Stack Light Rifle (Stack Computer Services Ltd) (1983)
//connects to expansion port, accessed by reading from Port DFh:

//  bit1       = Trigger Button (0=Pressed, 1=Released)
//  bit4       = Light Sensor   (0=Light, 1=No Light)
//  other bits = Must be "1"    (the programs use compare FDh to test if bit1=0)

               if (gunstick_emulation==GUNSTICK_PORT_DF) {

                        z80_byte acumulado=255;
                        if (mouse_left!=0) {

                            acumulado &=(255-2);

                            if (gunstick_view_white()) acumulado &=(255-16);


                        }

			printf ("gunstick acumulado: %d\n",acumulado);

                        return acumulado;
                }

		//puerto kempston tambien es DF en kempston mouse
		//else return idle_bus_port(puerto_l+256*puerto_h);

	}

	*/

	//kempston mouse. Solo con menu cerrado
	if ( !menu_abierto && kempston_mouse_emulation.v  &&  (puerto_l&32) == 0  &&  ( (puerto_h&7)==3 || (puerto_h&7)==7 || (puerto_h&2)==2 ) ) {
		//printf ("kempston mouse. port 0x%x%x\n",puerto_h,puerto_l);

//IN 64479 - return X axis (0-255)
//IN 65503 - return Y axis (0-255)
//IN 64223 - return button status
//D0 - right button
//D1 - left button
//D2-D7 - not used

//From diagram ports

//X-Axis  = port 64479 xxxxx011 xx0xxxxx
//Y-Axis  = port 65503 xxxxx111 xx0xxxxx
//BUTTONS = port 64223 xxxxxx10 xx0xxxxx

		z80_byte acumulado=0;


		if ((puerto_h&7)==3) {
			//X-Axis
			acumulado=kempston_mouse_x*kempston_mouse_factor_sensibilidad;
		}

                if ((puerto_h&7)==7) {
                        //Y-Axis
			acumulado=kempston_mouse_y*kempston_mouse_factor_sensibilidad;
                }

                if ((puerto_h&3)==2) {
                        //Buttons
			acumulado=255;

			//left button
			if (mouse_left) acumulado &=(255-2);
                        //right button
                        if (mouse_right) acumulado &=(255-1);

                        //en Next, bits altos se usan para wheel, como no los emulamos, a 0
                        //https://specnext.dev/wiki/Kempston_Mouse_Buttons
                        if (MACHINE_IS_TBBLUE) acumulado &=0x0F;
                }

		//printf ("devolvemos valor: %d\n",acumulado);
		return acumulado;

	}

	//If you read from a port that activates both the keyboard and a joystick port (e.g. Kempston), the joystick takes priority.

        //kempston joystick en Inves
	//En Inves, Si A5=0 solamente
        if ( (puerto_l & 32) ==0 && (MACHINE_IS_INVES) ) {

                if (joystick_emulation==JOYSTICK_KEMPSTON || gunstick_emulation==GUNSTICK_KEMPSTON) {
			return get_kempston_value();
                }

        }

        //kempston joystick para maquinas no Inves
        //Si A5=A6=A7=0 y A0=1, kempston joystick
        if ( (puerto_l & (1+32+64+128)) == 1 && !(MACHINE_IS_INVES) ) {

                if (joystick_emulation==JOYSTICK_KEMPSTON || gunstick_emulation==GUNSTICK_KEMPSTON) {
			return get_kempston_value();
                }

		//Kempston en zxuno, cuando no esta seleccionado kempston devolver 0 en vez de FF o si no, la bios no funciona
	        if (MACHINE_IS_ZXUNO) return 0;

                //Los juegos: alteregobeta, xnx, y todos los de los mojon twins, leen el joystick kempston
                //Si retorno el valor del bus idle, esos juegos se saltan los menus , como si hubiese tecla pulsada
                //Retorno 0, por si no se ha seleccionado joystick kempston en el menu de ZEsarUX

                if (MACHINE_IS_TSCONF) return 0;

        }

        //ZXUNO
        if ( MACHINE_IS_ZXUNO && (puerto==0xFC3B  || puerto==0xFD3B)) {
		return zxuno_read_port(puerto);
        }


	if (MACHINE_IS_TBBLUE) {
		//Puertos divmmc sin tener que habilitar divmmc paging.
		//if (divmmc_mmc_ports_enabled.v && mmc_enabled.v==1 && puerto_l == 0xEB) return mmc_read();


		//Las puertas que se pueden leer son: 24D5, 24DD y 24DF.
		//if (puerto==0x24D5) return tbblue_read_port_24d5();
		//if (puerto==0x24DD) return tbblue_config2;
		//if (puerto==0x24DF) return tbblue_port_24df;
		if (puerto==TBBLUE_REGISTER_PORT) return tbblue_get_register_port();
		if (puerto==TBBLUE_VALUE_PORT) return tbblue_get_value_port();
		if (puerto==TBBLUE_SPRITE_INDEX_PORT)	return tbblue_get_port_sprite_index();
		if (puerto==TBBLUE_LAYER2_PORT)	return tbblue_get_port_layer2_value();

		if (puerto==DS1307_PORT_CLOCK) return ds1307_get_port_clock();
		if (puerto==DS1307_PORT_DATA) return ds1307_get_port_data();

		//Puertos DIVMMC/DIVIDE. El de Paginacion
		//Este puerto solo se puede leer en TBBLUE y es necesario para que NextOS funcione bien
		//if (puerto_l==0xe3 && diviface_enabled.v) return diviface_read_control_register();
		if (puerto_l==0xe3) return diviface_read_control_register();

		//TODO puerto UART de tbbue. De momento retornamos 0, en la demo de Pogie espera que ese valor (el bit 1 concretamente)
		//sea 0 antes de empezar
		// http://devnext.referata.com/wiki/UART_TX
		// https://www.specnext.com/the-next-on-the-network/
		//if (puerto==TBBLUE_UART_RX_PORT) return 0;


		if (puerto==TBBLUE_UART_RX_PORT) return tbblue_uartbridge_readdata();

		//puerto estado es el de escritura pero en lectura
		if (puerto==TBBLUE_UART_TX_PORT) return tbblue_uartbridge_readstatus();

		//TODO puerto segundo joystick. De momento retornar 0
		if (puerto==TBBLUE_SECOND_KEMPSTON_PORT) return 0;


	}

	if (datagear_dma_emulation.v && (puerto_l==DATAGEAR_DMA_FIRST_PORT || puerto_l==DATAGEAR_DMA_SECOND_PORT) ) {
			//printf ("Reading Datagear DMA Port %04XH\n",puerto);
			//TODO
			return 0;
	}

        //Puertos ZXMMC. Interfiere con Fuller Audio Box
        if (zxmmc_emulation.v && (puerto_l==0x1f || puerto_l==0x3f) ) {
              //printf ("Puerto ZXMMC Read: 0x%02x\n",puerto_l);
		if (puerto_l==0x3f) {
			z80_byte valor_leido=mmc_read();
			//printf ("Valor leido: %d\n",valor_leido);
			return valor_leido;
		}

		return 255;

        }

	//Puertos 8-bit simple ide
	if (eight_bit_simple_ide_enabled.v && (puerto_l&16)==0) {
		return eight_bit_simple_ide_read(puerto_l);
	}

	if (if1_enabled.v) {
		if (puerto_l==0xf7) {
			//Net
			return 255;
		}

		if (puerto_l==0xef) {
			//Status bits
			//return 0;
			return 255;
		}

		if (puerto_l==0xe7) {
			//Data read/write
			return 255;
		}
	}


	//Puertos DIVMMC
	if (divmmc_mmc_ports_enabled.v && (puerto_l==0xe7 || puerto_l==0xeb) ) {
		//printf ("Puerto DIVMMC Read: 0x%02x\n",puerto_l);

	        //Si en ZXUNO y DIVEN desactivado.
		//Aunque cuando se toca el bit DIVEN de ZX-Uno se sincroniza divmmc_enable,
		//pero por si acaso... por si se activa manualmente desde menu del emulador
		//el divmmc pero zxuno espera que este deshabilitado, como en la bios
	        //if (MACHINE_IS_ZXUNO_DIVEN_DISABLED) return 255;

                if (puerto_l==0xeb) {
                        z80_byte valor_leido=mmc_read();
                        //printf ("Valor leido: %d\n",valor_leido);
                        return valor_leido;
                }

                return 255;

        }


        //Puerto DIVIDE
        if (divide_ide_ports_enabled.v && ( (puerto_l&(128+64+32+2+1))==128+32+2+1) ) {
                //printf ("Puerto DIVIDE Read: %02x%02xH command: %d PC=%X\n",puerto_h,puerto_l,(puerto_l>>2)&7,reg_pc);

                //So you can access all
                //eight IDE-registers from so caled command block (rrr=0..7) at addresses
                //xxxx xxxx  101r rr11
                z80_byte ide_retorno=ide_read_command_block_register((puerto_l>>2)&7);
                //printf("Retorno: %X\n",ide_retorno);

                return ide_retorno;

        }



	//Puerto Spectra
	if (spectra_enabled.v && puerto==0x7FDF) return spectra_read();

	//Sprite Chip
	if (spritechip_enabled.v && (puerto==SPRITECHIP_COMMAND_PORT || puerto==SPRITECHIP_DATA_PORT) ) return spritechip_read(puerto);




        //Puerto ULA, cualquier puerto par. En un Spectrum normal, esto va al final
	//En un Inves, deberia ir al principio, pues el inves hace un AND con el valor de los perifericos que retornan valor en el puerto leido
        if ( (puerto_l & 1)==0 && !(MACHINE_IS_CHLOE) && !(MACHINE_IS_TIMEX_TS_TC_2068) && !(MACHINE_IS_PRISM) ) {

		return lee_puerto_spectrum_ula(puerto_h);

        }


        //Puerto 254 solamente, para chloe y timex y prism
        if ( puerto_l==254 && (MACHINE_IS_CHLOE || MACHINE_IS_TIMEX_TS_TC_2068 || MACHINE_IS_PRISM) ) {
		return lee_puerto_spectrum_ula(puerto_h);
        }


        //Puerto HiLow
        if (hilow_enabled.v && puerto_l==0xFF) {
		return hilow_read_port_ff(puerto);
        }


        //Puerto Timex Video. 8 bit bajo a ff
        if (timex_video_emulation.v && puerto_l==0xFF) {
		return timex_port_ff;
        }

	//Puerto Timex Paginacion
        if (puerto_l==0xf4 && (MACHINE_IS_CHLOE || MACHINE_IS_TIMEX_TS_TC_2068 || MACHINE_IS_PRISM || is_zxuno_chloe_mmu() ) ) {
		return timex_port_f4;

        }


	//Puerto Chip AY para Timex y Chloe
        if (puerto_l==0xF6 && (MACHINE_IS_CHLOE || MACHINE_IS_TIMEX_TS_TC_2068) ) {
            activa_ay_chip_si_conviene();
            if (ay_chip_present.v==1) return in_port_ay(0xFF);
	    }

	//Puertos paginacion,disco en Prism 2ffd, 3ffd retornan 255
	if (MACHINE_IS_PRISM) {
		if (puerto==0x2ffd || puerto==0x3ffd) return 255;


		//0x9E3B (read) - returns Prism microcode version (I'd suggest returning '0' to indicate emulation)
		if (puerto==0x9E3B) return 0;
	}


	z80_byte valor_idle_bus_port=idle_bus_port(puerto_l+256*puerto_h);

	//ZEsarUX ZXI ports
    if (hardware_debug_port.v) {
	   if (puerto==ZESARUX_ZXI_PORT_REGISTER) return zesarux_zxi_read_last_register();
	   if (puerto==ZESARUX_ZXI_PORT_DATA)     return zesarux_zxi_read_register_value();
    }


  //Puertos paginacion chrome se pueden leer
	//TODO: solo los puertos exactos o tambien vale mascara?
	/*
	Port 1FFDh (read/write)
Bit 5 If set disable Chrome features ( reading/writing to port 1FFDh, reading from port 7FFDh, i2c interface. This downgrade Chrome to a simple 128K spectrum clone)
*/
	if (MACHINE_IS_CHROME) {
		if (puerto==32765 && si_chrome_features_enabled()) return puerto_32765;
		if (puerto==8189) return puerto_8189;
	}

		if (MACHINE_IS_PENTAGON) {
					if (puerto==0xeff7) return puerto_eff7;


				}

    if (gs_enabled.v) {
        if (puerto_l==0xBB) return gs_read_port_bb_from_speccy();
        if (puerto_l==0xB3) return gs_read_port_b3_from_speccy();
    }

	if (MACHINE_IS_TSCONF) {

		//Puertos nvram
		if (puerto==0xeff7) return puerto_eff7;
		if (puerto==0xdff7) return zxevo_last_port_dff7;
		if (puerto==0xbff7) return zxevo_nvram[zxevo_last_port_dff7];
		if (puerto_l==0xaf) return tsconf_get_af_port(puerto_h);

		//Puertos ZIFI
		if (puerto==TSCONF_ZIFI_ERROR_REG) return tsconf_zifi_read_error_reg();
		if (puerto==TSCONF_ZIFI_DATA_REG) return tsconf_zifi_read_data_reg();
		if (puerto==TSCONF_ZIFI_INPUT_FIFO_STATUS) return tsconf_zifi_read_input_fifo_status();
		if (puerto==TSCONF_ZIFI_OUTPUT_FIFO_STATUS) return tsconf_zifi_read_output_fifo_status();

		//Puerto desconocido pero que usa la demo zifi. Tambien lo usa en escritura pero no se como va
		if (puerto==0x57) return tsconf_read_port_57();

		//Otros puertos
		//printf ("Leyendo puerto %04XH\n",puerto);

	}

	if (MACHINE_IS_BASECONF) {
		//printf ("Baseconf reading port %04XH on pc=%04XH\n",puerto,reg_pc);

		//Puertos nvram. TODO gestion puertos shadow
		if (puerto==0xeff7 /*&& !baseconf_shadow_ports_available()*/ ) return puerto_eff7;

		if (puerto==0xdff7 /*&& !baseconf_shadow_ports_available()*/ ) return zxevo_last_port_dff7;
		if (puerto==0xdef7 /*&& baseconf_shadow_ports_available()*/ ) return zxevo_last_port_dff7;

		if (puerto==0xbff7 /*&& !baseconf_shadow_ports_available()*/ ) {
			//printf ("baseconf reading nvram register %02XH\n",zxevo_last_port_dff7);
			//return zxevo_nvram[zxevo_last_port_dff7];


			return (puerto_eff7 & 0x80) ? zxevo_nvram[zxevo_last_port_dff7] : 0xff;
		}

        if ( (puerto&0x00FF)==0xBF ) {
               return baseconf_last_port_bf;
        }



		if (puerto==0xbef7 /*&& baseconf_shadow_ports_available()*/ ) {
			//printf ("baseconf reading nvram register %02XH\n",zxevo_last_port_dff7);

			return zxevo_nvram[zxevo_last_port_dff7];
		}

		printf ("Baseconf reading unhandled port %04XH on pc=%04XH\n",puerto,reg_pc);

	}


//Final. Puertos de paginacion y puerto no asignado. No agregar nada despues de aqui
//Puertos de Paginacion. En caso de 128k y +2, acaba escribiendo el valor leido del bus idle en el puerto de paginacion
if (MACHINE_IS_SPECTRUM_128_P2)
{
	//Para 128k
	//Puerto tipicamente 32765

	if ( (puerto & 32770) == 0 ) {
			out_port_spectrum_no_time(32765,valor_idle_bus_port);
	}

}



	if (MACHINE_IS_TSCONF) {
		//printf ("In Port %x unknown asked, PC after=0x%x\n",puerto_l+256*puerto_h,reg_pc);

		//xnx lee estos tambien:
		//In Port fbdf unknown asked, PC after=0xe292
		//In Port ffdf unknown asked, PC after=0xe2a2


		//Puertos SD
		if (mmc_enabled.v && puerto_l==0x77) {
			//Read: always 0 (the card is inserted in the regime R / W - in accordance with the interpretation of the Z-controller). The actual presence of maps should attempt to verify its initialization time an out.
			//printf ("Returning SD port %04XH value 1\n",puerto);
			return 1; //1=inserted
		}

		//if (!sdc->image || !sdc->on || sdc->cs) return 0xff;    // no image or OFF or !CS

		 if (mmc_enabled.v && puerto_l==0x57) {
			if (!baseconf_sd_enabled || baseconf_sd_cs) return 0xFF;
                        z80_byte valor_leido=mmc_read();
			//printf ("Returning SD port %04XH value %02XH PC=%04XH\n",puerto,valor_leido,reg_pc);

			//algunos  0 , lo convertimos en FF. chapuza muy fea para probar
			/*if (valor_leido==0) {
				if (temp_tsconf_first_sd_0) {
					//printf ("Changing return value to ff\n");
					valor_leido=0xFF;
				}
				temp_tsconf_first_sd_0--;
			}*/

			//otro parche feo
			if (reg_pc==0x06B3 || reg_pc==0x695) {
				//printf ("Changing return value to ff\n");
				return 0xff; //Parche
			}

			return valor_leido;
		}


	}


	//debug_printf (VERBOSE_DEBUG,"In Port %x unknown asked, PC after=0x%x",puerto_l+256*puerto_h,reg_pc);
	//printf ("In Port %x unknown asked, PC after=0x%x\n",puerto_l+256*puerto_h,reg_pc);
	//printf ("idle bus port: %d\n",puerto_l+256*puerto_h);
	return valor_idle_bus_port;

}











void cpi_cpd_common(void)
{

	z80_byte antes,result;

	antes=reg_a;
        result=reg_a-peek_byte(HL);

        contend_read_no_mreq( HL, 1 ); contend_read_no_mreq( HL, 1 );
        contend_read_no_mreq( HL, 1 ); contend_read_no_mreq( HL, 1 );
        contend_read_no_mreq( HL, 1 );



        set_undocumented_flags_bits(result);
        set_flags_zero_sign(result);
	set_flags_halfcarry_resta(antes,result);
        Z80_FLAGS |=FLAG_N;


        BC--;

        if (!BC) Z80_FLAGS &=(255-FLAG_PV);
        else Z80_FLAGS |=FLAG_PV;

	if ((Z80_FLAGS & FLAG_H)) result--;

        if (result & 8 ) Z80_FLAGS |=FLAG_3;
        else Z80_FLAGS &=(255-FLAG_3);

        if (result & 2 ) Z80_FLAGS |=FLAG_5;
        else Z80_FLAGS &=(255-FLAG_5);


}

void out_port_ace_no_time(z80_int puerto,z80_byte value)
{
	debug_fired_out=1;


        z80_byte puerto_l=puerto&0xFF;
        //z80_byte puerto_h=(puerto>>8)&0xFF;

       //Puerto ULA, cualquier puerto par

        if ((puerto_l & 1)==0) {

		//Any write to this port toggles the speaker "on".
                                        //y ponemos a high la salida del altavoz
                        bit_salida_sonido_ace.v=1;

                        set_value_beeper_on_array(da_amplitud_speaker_ace() );

                                silence_detection_counter=0;
                                beeper_silence_detection_counter=0;


			//bit_salida_sonido_ace.v=value&1;
			//set_value_beeper_on_array(da_amplitud_speaker_ace() );
			//printf ("Out port ACE ula value: %d\n",bit_salida_sonido_ace.v);


	}


        //Soundbox
        if (puerto_l==0xFD) {
                        activa_ay_chip_si_conviene();
                        if (ay_chip_present.v==1) out_port_ay(65533,value);
                }

        if (puerto_l==0xFF) {
                        activa_ay_chip_si_conviene();
                        if (ay_chip_present.v==1) out_port_ay(49149,value);
        }


}


void out_port_ace(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_ace_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}

void out_port_zx80_no_time(z80_int puerto,z80_byte value)
{

	debug_fired_out=1;
	//Esto solo sirve para mostrar en menu debug i/o ports
	zx8081_last_port_write_value=value;

	z80_byte puerto_l=puerto&0xFF;
	z80_byte puerto_h=(puerto>>8)&0xFF;

	//Bi-Pak ZON-X81 Sound
		if (puerto_l==0xDF || puerto_l==0xCF) {
			activa_ay_chip_si_conviene();
			if (ay_chip_present.v==1) out_port_ay(65533,value);
		}

		if (puerto_l==0x0F || puerto_l==0x1F) {
			activa_ay_chip_si_conviene();
			if (ay_chip_present.v==1) out_port_ay(49149,value);
		}



        //zx printer
        if (puerto_l==0xFB) {
                if (zxprinter_enabled.v==1) {
                        zxprinter_write_port(value);

                }
        }

	//chroma
	if (puerto==0x7FEF && chroma81.v) {
		chroma81_port_7FEF=value;
		//if (chroma81_port_7FEF&16) debug_printf (VERBOSE_INFO,"Chroma 81 mode 1");
		//else debug_printf (VERBOSE_INFO,"Chroma 81 mode 0");
		debug_printf (VERBOSE_INFO,"Setting Chroma 81 parameters: Border: %d, Mode: %s, Enable: %s",
			chroma81_port_7FEF&15,(chroma81_port_7FEF&16 ? "1 - Attribute file" : "0 - Character code"),
			(chroma81_port_7FEF&32 ? "Yes" : "No") );
	}


        //zxpand
        if (puerto_l==0x07 && zxpand_enabled.v) {
		//z80_byte letra=value;
		//if (letra>=128) letra -=128;
		//if (letra<64) letra=da_codigo_zx81_no_artistic(letra);
		//else letra='?';

                //printf ("Out Port ZXpand 0x%X value : 0x%0x (%c), PC after=0x%x\n",puerto,value,letra,reg_pc);
		zxpand_write(puerto_h,value);
        }

	//ZEsarUX ZXI ports
    	if (hardware_debug_port.v) {
		if (puerto==ZESARUX_ZXI_ZX8081_PORT_REGISTER) zesarux_zxi_write_last_register(value);
	   	if (puerto==ZESARUX_ZXI_ZX8081_PORT_DATA)     zesarux_zxi_write_register_value(value);
    	}


	//Cualquier puerto generara vsync


	//printf ("Sending vsync with hsync_generator_active : %d video_zx8081_ula_video_output: %d\n",hsync_generator_active.v,video_zx8081_ula_video_output);


        //printf("Enabling the HSYNC generator t_scanline_draw=%d\n",t_scanline_draw);

        hsync_generator_active.v=1;
        modificado_border.v=1;


	//reseteamos contador de deteccion de modo fast-pantalla negra. Para modo no-realvideo
	video_fast_mode_next_frame_black=0;


	//y ponemos a high la salida del altavoz
	bit_salida_sonido_zx8081.v=1;

	set_value_beeper_on_array(da_amplitud_speaker_zx8081() );


	if (zx8081_vsync_sound.v==1) {
		//solo resetea contador de silencio cuando esta activo el vsync sound - beeper
		silence_detection_counter=0;
		beeper_silence_detection_counter=0;
	}



        //Calcular cuanto ha tardado el vsync
        int longitud_pulso_vsync;

        if (t_estados>inicio_pulso_vsync_t_estados) longitud_pulso_vsync=t_estados-inicio_pulso_vsync_t_estados;

        //contador de t_estados ha dado la vuelta. estamos al reves
        else longitud_pulso_vsync=screen_testados_total-inicio_pulso_vsync_t_estados+t_estados;

        //printf ("escribe puerto. final vsync  t_estados=%d. diferencia: %d t_scanline_draw: %d t_scanline_draw_timeout: %d\n",t_estados,longitud_pulso_vsync,t_scanline_draw,t_scanline_draw_timeout);


	if (video_zx8081_linecntr_enabled.v==0) {
		if (longitud_pulso_vsync >= minimo_duracion_vsync) {
			//if (t_scanline_draw_timeout>MINIMA_LINEA_ADMITIDO_VSYNC || t_scanline_draw_timeout<=3) {
			if (t_scanline_draw_timeout>MINIMA_LINEA_ADMITIDO_VSYNC) {
				//printf ("admitido final pulso vsync\n");

		                if (!simulate_lost_vsync.v) {



					if (zx8081_detect_vsync_sound.v) {
						//printf ("vsync total de zx8081 t_estados: %d\n",t_estados);
						if (zx8081_detect_vsync_sound_counter>0) zx8081_detect_vsync_sound_counter--;

					}

					generar_zx8081_vsync();
					vsync_per_second++;
				}


			}
		}

		else {
			//printf ("no admitimos pulso vsync, duracion: %d\n",longitud_pulso_vsync);
		}

	}


	video_zx8081_linecntr_enabled.v=1;

	//modo slow ?¿
	/*
	if (nmi_generator_active.v==1) {
		video_zx8081_lnctr_adjust.v=1;
	}
	else {
		video_zx8081_lnctr_adjust.v=0;
	}
	*/

 	video_zx8081_ula_video_output=0;

	//Prueba para no tener que usar ajuste lnctr
	//con esto: modo zx80 lnctr desactivado y zx81 activado y todos juegos se ven bien. PERO modo fast zx81 se ve mal
	//video_zx8081_linecntr=0;


	//prueba if (nmi_generator_active.v==1) video_zx8081_linecntr=0;


}
void out_port_zx81_no_time(z80_int puerto,z80_byte value)
{

	debug_fired_out=1;

	if ((puerto&0xFF)==0xfd) {
		//debug_printf (VERBOSE_DEBUG,"Disabling NMI generator\n");
		nmi_generator_active.v=0;
		//return;
	}

        if ((puerto&0xFF)==0xfe) {
		//debug_printf (VERBOSE_DEBUG,"Enabling NMI generator\n");
                nmi_generator_active.v=1;
		//return;
        }


	//TODO
	//Con el luego wrx/nucinv16.p esto NO da imagen estable
	//para imagen estable, los if de antes no deben finalizar con return, es decir,
	//se debe activar/desactivar nmi y luego lanzar la sentencia out_port_zx80_no_time(puerto,value);
	//para wrx/nucinv16.p se hace el truco de bajar el timeout de vsync
	out_port_zx80_no_time(puerto,value);



}



void out_port_msx1_no_time(z80_int puerto,z80_byte value)
{

	debug_fired_out=1;
        //Los OUTS los capturan los diferentes interfaces que haya conectados, por tanto no hacer return en ninguno, para que se vayan comprobando
        //uno despues de otro
	z80_byte puerto_l=puerto&255;
	//z80_byte puerto_h=(puerto>>8)&0xFF;

	//printf ("Out msx port: %04XH value: %02XH char: %c PC=%04XH\n",puerto,value,
	//  (value>=32 && value<=126 ? value : '?'),reg_pc );


/*
	printf ("Out msx port: %04XH value: %02XH char: %c\n",puerto,value,
	  (value>=32 && value<=126 ? value : '?') );

	if (puerto_l==0xA8) printf ("Puerto PPI Port R/W Port A\n");
	if (puerto_l==0x98) printf ("VDP Video Ram Data\n");
	if (puerto_l==0x99) printf ("VDP Command and status register\n");*/


	//if (puerto_l==0x98) printf ("%c",
	//  (value>=32 && value<=126 ? value : '?') );

	if (puerto_l==0x98) {
		//printf ("VDP Video Ram Data\n");
		msx_out_port_vdp_data(value);
	}

	if (puerto_l==0x99) {
		//printf ("VDP Command and status register\n");
		msx_out_port_vdp_command_status(value);
	}

	if (puerto_l>=0x9A && puerto_l<0xA0) {
		//printf ("Out port possibly vdp. Port %04XH value %02XH\n",puerto,value);
	}

	if (puerto_l>=0xA0 && puerto_l<=0xA7) {
		msx_out_port_psg(puerto_l,value);
	}

	if (puerto_l>=0xA8 && puerto_l<=0xAB) {
		msx_out_port_ppi(puerto_l,value);
	}

}

void out_port_msx1(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_msx1_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}


int temp_conta_lee_puerto_msx1_no_time;

//Devuelve valor puerto para maquinas MSX1
z80_byte lee_puerto_msx1_no_time(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l)
{

	debug_fired_in=1;
	//extern z80_byte in_port_ay(z80_int puerto);
	//65533 o 49149
	//FFFDh (65533), BFFDh (49149)

	//z80_int puerto=value_8_to_16(puerto_h,puerto_l);


	//printf ("Lee puerto msx %04XH PC=%04XH\n",puerto,reg_pc);

	//if (puerto_l==0x98) printf ("VDP Video Ram Data\n");
	//if (puerto_l==0x99) printf ("VDP Command and status register\n");


	//A8.
	//if (puerto==0xa8) return 0x50; //temporal

	if (puerto_l==0x98) {
		//printf ("VDP Video Ram Data IN\n");
		return msx_in_port_vdp_data();
	}

	if (puerto_l==0x99) {
		//printf ("VDP Status IN\n");
		return msx_in_port_vdp_status();
	}

	if (puerto_l>=0xA8 && puerto_l<=0xAB) {
		return msx_in_port_ppi(puerto_l);
	}


	//if (puerto_l==0xA0 && ay_3_8912_registro_sel[0]==14) {
	if (puerto_l==0xA2) {
		//printf ("reading from psg\n");
        return msx_read_psg();
	}


	return 255;


}

z80_byte lee_puerto_msx1(z80_byte puerto_h,z80_byte puerto_l)
{
  //z80_int port=value_8_to_16(puerto_h,puerto_l);
  //ula_contend_port_early( port );
  //ula_contend_port_late( port );
  z80_byte valor = lee_puerto_msx1_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}



void out_port_svi_no_time(z80_int puerto,z80_byte value)
{

	debug_fired_out=1;
        //Los OUTS los capturan los diferentes interfaces que haya conectados, por tanto no hacer return en ninguno, para que se vayan comprobando
        //uno despues de otro
	z80_byte puerto_l=puerto&255;
	//z80_byte puerto_h=(puerto>>8)&0xFF;

	//printf ("Out svi port: %04XH value: %02XH char: %c PC=%04XH\n",puerto,value,
	//  (value>=32 && value<=126 ? value : '?'),reg_pc );


/*
	printf ("Out svi port: %04XH value: %02XH char: %c\n",puerto,value,
	  (value>=32 && value<=126 ? value : '?') );

	if (puerto_l==0xA8) printf ("Puerto PPI Port R/W Port A\n");
	if (puerto_l==0x98) printf ("VDP Video Ram Data\n");
	if (puerto_l==0x99) printf ("VDP Command and status register\n");*/


	//if (puerto_l==0x98) printf ("%c",
	//  (value>=32 && value<=126 ? value : '?') );

	if (puerto_l==0x80) {
		//printf ("VDP Video Ram Data\n");
		svi_out_port_vdp_data(value);
	}

	if (puerto_l==0x81) {
		//printf ("VDP Command and status register\n");
		svi_out_port_vdp_command_status(value);
	}

	if (puerto_l>=0x9A && puerto_l<0xA0) {
		//printf ("Out port possibly vdp. Port %04XH value %02XH\n",puerto,value);
	}

	if (puerto_l==0x88 || puerto_l==0x8c) {
		svi_out_port_psg(puerto_l,value);
	}

	if (puerto_l>=0x96 && puerto_l<=0x99) {
		svi_out_port_ppi(puerto_l,value);
	}

}

void out_port_svi(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_svi_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}


//Devuelve valor puerto para maquinas SVI1
z80_byte lee_puerto_svi_no_time(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l)
{

	debug_fired_in=1;
	//extern z80_byte in_port_ay(z80_int puerto);
	//65533 o 49149
	//FFFDh (65533), BFFDh (49149)

	//z80_int puerto=value_8_to_16(puerto_h,puerto_l);


	//printf ("Lee puerto svi %04XH PC=%04XH\n",puerto,reg_pc);

	//if (puerto_l==0x98) printf ("VDP Video Ram Data\n");
	//if (puerto_l==0x99) printf ("VDP Command and status register\n");


	//A8.
	//if (puerto==0xa8) return 0x50; //temporal

	if (puerto_l==0x84) {
		//printf ("VDP Video Ram Data IN\n");
		return svi_in_port_vdp_data();
	}

	if (puerto_l==0x81) {
		//printf ("VDP Status IN\n");
		return svi_in_port_vdp_status();
	}

	if (puerto_l==0x96 || puerto_l==0x98 || puerto_l==0x99 || puerto_l==0x9a) {
		return svi_in_port_ppi(puerto_l);
	}


	//if (puerto_l==0xA0 && ay_3_8912_registro_sel[0]==14) {
	if (puerto_l==0x90) {
        return svi_read_psg();
	}


	return 255;


}

z80_byte lee_puerto_svi(z80_byte puerto_h,z80_byte puerto_l)
{
  //z80_int port=value_8_to_16(puerto_h,puerto_l);
  //ula_contend_port_early( port );
  //ula_contend_port_late( port );
  z80_byte valor = lee_puerto_svi_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}



void out_port_coleco_no_time(z80_int puerto,z80_byte value)
{

	debug_fired_out=1;
        //Los OUTS los capturan los diferentes interfaces que haya conectados, por tanto no hacer return en ninguno, para que se vayan comprobando
        //uno despues de otro
	z80_byte puerto_l=puerto&255;
	//z80_byte puerto_h=(puerto>>8)&0xFF;

	//if (puerto_l!=0xBE && puerto_l!=0xBF) {
	//	printf ("Out coleco port: %04XH value: %02XH char: %c PC=%04XH\n",puerto,value,(value>=32 && value<=126 ? value : '?'),reg_pc );
	//}




/*
	printf ("Out coleco port: %04XH value: %02XH char: %c\n",puerto,value,
	  (value>=32 && value<=126 ? value : '?') );

	if (puerto_l==0xA8) printf ("Puerto PPI Port R/W Port A\n");
	if (puerto_l==0x98) printf ("VDP Video Ram Data\n");
	if (puerto_l==0x99) printf ("VDP Command and status register\n");*/


	//if (puerto_l==0x98) printf ("%c",
	//  (value>=32 && value<=126 ? value : '?') );


       if (puerto_l==0xBE) {
              //printf ("VDP Video Ram Data\n");
               coleco_out_port_vdp_data(value);
       }

       if (puerto_l==0xBF) {
               //printf ("VDP Command and status register\n");
               coleco_out_port_vdp_command_status(value);
       }

	   if (puerto_l>=0xE0 && puerto_l<=0xFF) {
		   //printf ("Puerto sonido %04XH valor %02XH\n",puerto,value);
		   sn_out_port_sound(value);
	   }

	if (puerto_l >=0x80 && puerto_l <=0x9F) {
		//printf ("Out coleco port: Controls _ Set to keypad mode %04XH value: %02XH char: %c\n",puerto,value,(value>=32 && value<=126 ? value : '?') );

		coleco_set_keypad_mode();
	}

	if (puerto_l >=0xC0 && puerto_l <=0xDF) {
		//printf ("Out coleco port: Controls _ Set to joystick mode %04XH value: %02XH char: %c\n",puerto,value,(value>=32 && value<=126 ? value : '?') );

		coleco_set_joystick_mode();
	}

}

void out_port_coleco(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_coleco_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}


//Devuelve valor puerto para maquinas Coleco
z80_byte lee_puerto_coleco_no_time(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l)
{

	debug_fired_in=1;
	//extern z80_byte in_port_ay(z80_int puerto);
	//65533 o 49149
	//FFFDh (65533), BFFDh (49149)

	//z80_int puerto=value_8_to_16(puerto_h,puerto_l);


	//printf ("Lee puerto coleco %04XH PC=%04XH\n",puerto,reg_pc);

	//if (puerto_l==0x98) printf ("VDP Video Ram Data\n");
	//if (puerto_l==0x99) printf ("VDP Command and status register\n");


	//A8.
	//if (puerto==0xa8) return 0x50; //temporal

       //temp coleco
       //printf ("In port : %04XH\n",puerto);
       if (puerto_l==0xBE) {
               //printf ("VDP Video Ram Data IN\n");
               return coleco_in_port_vdp_data();
       }

       if (puerto_l==0xBF) {
               //printf ("VDP Status IN\n");
               return coleco_in_port_vdp_status();
       }


       //FC- Reading this port gives the status of controller #1. (farthest from front)
	   //Temporal fila de teclas
       if (puerto_l==0xFC) {
		   //printf ("In coleco controller A\n");
               return coleco_get_controller_a();
       }

//FF- Reading this one gives the status of controller #2. (closest to front)
       if (puerto_l==0xFF) {
		   //printf ("In coleco controller B\n");
               return coleco_get_controller_b();
       }

//printf ("In port : %04XH\n",puerto);

	return 255;


}

z80_byte lee_puerto_coleco(z80_byte puerto_h,z80_byte puerto_l)
{
  //z80_int port=value_8_to_16(puerto_h,puerto_l);
  //ula_contend_port_early( port );
  //ula_contend_port_late( port );
  z80_byte valor = lee_puerto_coleco_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}




void out_port_sg1000_no_time(z80_int puerto,z80_byte value)
{

	debug_fired_out=1;
        //Los OUTS los capturan los diferentes interfaces que haya conectados, por tanto no hacer return en ninguno, para que se vayan comprobando
        //uno despues de otro
	z80_byte puerto_l=puerto&255;
	//z80_byte puerto_h=(puerto>>8)&0xFF;



	//if (puerto_l==0x98) printf ("%c",
	//  (value>=32 && value<=126 ? value : '?') );

	//sg1000 sound
		   if (puerto_l==0x7F) {
		   //printf ("Puerto sonido %04XH valor %02XH\n",puerto,value);
		   sn_out_port_sound(value);
	   }


       if (puerto_l==0xBE) {
              //printf ("VDP Video Ram Data\n");
               sg1000_out_port_vdp_data(value);
       }

       if (puerto_l==0xBF) {
               //printf ("VDP Command and status register\n");
               sg1000_out_port_vdp_command_status(value);
       }



}

void out_port_sg1000(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_sg1000_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}


//Devuelve valor puerto para maquinas SG1000
z80_byte lee_puerto_sg1000_no_time(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l)
{

	debug_fired_in=1;
	//extern z80_byte in_port_ay(z80_int puerto);
	//65533 o 49149
	//FFFDh (65533), BFFDh (49149)

	//z80_int puerto=value_8_to_16(puerto_h,puerto_l);


	//printf ("Lee puerto sg1000 %04XH PC=%04XH\n",puerto,reg_pc);

	//if (puerto_l==0x98) printf ("VDP Video Ram Data\n");
	//if (puerto_l==0x99) printf ("VDP Command and status register\n");


	//A8.
	//if (puerto==0xa8) return 0x50; //temporal


       //printf ("In port : %04XH\n",puerto);
       if (puerto_l==0xBE) {
               //printf ("VDP Video Ram Data IN\n");
               return sg1000_in_port_vdp_data();
       }

       if (puerto_l==0xBF) {
               //printf ("VDP Status IN\n");
               return sg1000_in_port_vdp_status();
       }


       //FC- Reading this port gives the status of controller #1. (farthest from front)
	   //Temporal fila de teclas
       /*if (puerto_l==0xFC) {

//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4

				//345 67890
               return (puerto_61438 & 31) | ((puerto_63486<<3) & (128+64+32) );

       }*/
	   /*
	   Lee puerto sg1000 04DCH PC=1D7AH


Lee puerto sg1000 03DDH PC=1D7DH
Lee puerto sg1000 02DEH PC=1DBFH
*/

		//tipicamente DC
       if ((puerto_l & 193) == 192 ) {
		   return sg1000_get_joypad_a();


       }


		//tipicamente DD
       if ((puerto_l & 193) == 193) {
		   return sg1000_get_joypad_b();


       }




	//printf ("Lee puerto sg1000 %04XH PC=%04XH\n",puerto,reg_pc);


	return 255;


}

z80_byte lee_puerto_sg1000(z80_byte puerto_h,z80_byte puerto_l)
{
  //z80_int port=value_8_to_16(puerto_h,puerto_l);
  //ula_contend_port_early( port );
  //ula_contend_port_late( port );
  z80_byte valor = lee_puerto_sg1000_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}

void out_port_sms_no_time(z80_int puerto,z80_byte value)
{

    debug_fired_out=1;
    //Los OUTS los capturan los diferentes interfaces que haya conectados, por tanto no hacer return en ninguno, para que se vayan comprobando
    //uno despues de otro
    z80_byte puerto_l=puerto&255;
    //z80_byte puerto_h=(puerto>>8)&0xFF;


    // The address decoding for the I/O ports is done with A7, A6, and A0 of
    // the Z80 address bus

    z80_byte puerto_escrito_efectivo=puerto_l & 0xC1;

    switch (puerto_escrito_efectivo) {

        case 0x40:
        case 0x41:
            /*
                Typically 7E. SN76489 data (write)
                Typically 7F. SN76489 data (write, mirror)
            */
            //7E & 193 = 0x40
            //7F & 193 = 0x41
            //printf("Puerto sonido %04XH valor %02XH\n",puerto,value);
            sn_out_port_sound(value);
        break;


        case 0x80:
            //BEH & 193 = 0x80
            //printf ("VDP Video Ram Data\n");
            sms_out_port_vdp_data(value);
        break;

        case 0x81:
            //BFH & 193 = 0x81
            //printf ("VDP Command and status register\n");
            sms_out_port_vdp_command_status(value);
        break;

        default:
            //printf("Unhandled out port %04XH (effective %02XH) value %02XH\n",puerto,puerto_escrito_efectivo,value);
        break;
    }

        //if (puerto_l!=0xBE && puerto_l!=0xBF && puerto_l!=0x7f) printf("Out Puerto %04XH valor %02XH\n",puerto,value);

}

void out_port_sms(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_sms_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}


//Devuelve valor puerto para maquinas SMS
z80_byte lee_puerto_sms_no_time(z80_byte puerto_h GCC_UNUSED,z80_byte puerto_l)
{

    debug_fired_in=1;
    //extern z80_byte in_port_ay(z80_int puerto);
    //65533 o 49149
    //FFFDh (65533), BFFDh (49149)

    //z80_int puerto=value_8_to_16(puerto_h,puerto_l);

    z80_byte puerto_leido_efectivo=puerto_l & 0xC1;

    switch (puerto_leido_efectivo) {


        //printf ("In port : %04XH\n",puerto);

        case 0x40:
            //7EH & 193 = 0x40

            //0x7E : Reading: returns VDP V counter
            //printf("scanline draw: %d\n",t_scanline_draw);
            return t_scanline_draw;

            //return 0xB0; //sonic por ejemplo espera este valor
        break;

        case 0x41:
            //7FH & 193 = 0x41
            //TODO: 0x7F : Reading: returns VDP H counter
            printf("Reading unemulated port VDP H counter(%02XH)\n",puerto_l);
        break;

        case 0x80:
            //BEH & 193 = 0x80
            //printf ("VDP Video Ram Data IN\n");
            //TODO: este reset de vdp_9918a_last_command_status_bytes_counter deberia estar en teoria para todas las maquinas con el vdp 9918a
            //Y no solo para SMS
            //Sin este reset, el rainbow islands no se ve nada
            /*
            In order for the VDP to know if it is recieving the first or second byte
            of the command word, it has a flag which is set after the first one is sent,
            and cleared when the second byte is written. The flag is also cleared when
            the control port is read, and when the data port is read or written. This
            is primarily used to initialize the flag to zero after it has been modified
            unpredictably, such as after an interrupt routine has executed.

            */
            vdp_9918a_last_command_status_bytes_counter=0;
            return sms_in_port_vdp_data();
        break;

        case 0x81:
            //BFH & 193 = 0x81
            //printf ("VDP Status IN\n");
            //TODO: este reset de vdp_9918a_last_command_status_bytes_counter deberia estar en teoria para todas las maquinas con el vdp 9918a
            //Y no solo para SMS
            //Sin este reset, el rainbow islands no se ve nada
            vdp_9918a_last_command_status_bytes_counter=0;

            //Si line interrupt pendiente
            /*if (sms_pending_line_interrupt) {
                printf("Generando interrupcion line interrupt\n");
                sms_pending_line_interrupt=0;
                interrupcion_maskable_generada.v=1;
            }*/

            return sms_in_port_vdp_status();
        break;


        case 0xC0:
            //tipicamente DC
            // The address decoding for the I/O ports is done with A7, A6, and A0 of
            //the Z80 address bus
            //193 = 11000001

            //192 = 0xC0
            return sms_get_joypad_a();
        break;

        case 0xC1:
            //tipicamente DD
            //193 = 0xC1
            return sms_get_joypad_b();
        break;

        default:
            //printf("Unhandled in port %04XH (effective %02XH)\n",puerto,puerto_leido_efectivo);
        break;

    }

	//printf ("Lee puerto sms %04XH PC=%04XH\n",puerto,reg_pc);


	return 255;


}

z80_byte lee_puerto_sms(z80_byte puerto_h,z80_byte puerto_l)
{
  //z80_int port=value_8_to_16(puerto_h,puerto_l);
  //ula_contend_port_early( port );
  //ula_contend_port_late( port );
  z80_byte valor = lee_puerto_sms_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}




//Extracted from Fuse emulator
void set_value_beeper (int v)
{
  static int beeper_ampl[] = { 0, AMPLITUD_TAPE, AMPLITUD_BEEPER,
                               AMPLITUD_BEEPER+AMPLITUD_TAPE };
/*
  if( tape_is_playing() ) {
    // Timex machines have no loading noise
    if( !settings_current.sound_load || machine_current->timex ) on = on & 0x02;
  } else {
    // ULA book says that MIC only isn't enough to drive the speaker as output voltage is below the 1.4v threshold
    if( on == 1 ) on = 0;
  }
*/

  //Si estamos en rutina de SAVE, enviar sonido teniendo en cuenta como referencia el 0 y de ahi hacia arriba o abajo
  //siempre que el parametro de filtro lo tengamos activo en el menu

  //valores de PC en hacer out 254:
  //rutina save : 1244, 1262, 1270, 1310. oscila solo bit MIC
  //rutina load: 1374
  //rutina beep: 995
  if (reg_pc>1200 && reg_pc<1350 && output_beep_filter_on_rom_save.v) {
		//Estamos en save.
		//printf ("valor beeper: %d\n",v);
		value_beeper=( v ? AMPLITUD_TAPE*2 : -AMPLITUD_TAPE*2);

		//Si activamos modo alteracion beeper. Ideal para que se escuche mas alto y poder enviar a inves
		/*
		En audacity, despues de exportar con valor 122 de beeper, aplicar reduccion de ruido:
		db 3, sensibilidad 0, suavidad 150 hz, ataque 0.15
		Tambien se puede aplicar reduccion de agudos -5
		Luego reproducir con volumen del pc al maximo
		*/

		if (output_beep_filter_alter_volume.v) {
			//value_beeper=( v ? 122 : -122);
			value_beeper=( v ? output_beep_filter_volume : -output_beep_filter_volume);
		}



  }

  else {



	//Por defecto el sonido se genera en negativo y de ahi oscila

	//temp normal en fuse
	value_beeper = -beeper_ampl[3] + beeper_ampl[v]*2;



  }



}

z80_byte get_border_colour_from_out(void)
{

z80_byte color_border;

if (MACHINE_IS_ZXUNO && zxuno_is_prism_mode_enabled() ) {
//IxxxxGRB puerto FE
    color_border=out_254&7;
    if (out_254 & 128) color_border+=8;
    //printf("color border: %d\n",color_border);
    return color_border;
}


                                        if (spectra_enabled.v) {
                                                if (spectra_display_mode_register&16) {
                                                        //enhanced border
                                                        //spectra_display_mode_register&4 ->extra colours
                                                        if (spectra_display_mode_register&4) {
                                                                //extra colours

                                                                color_border=
                                                                        (out_254&32  ? 1 : 0) +
                                                                        (out_254&1   ? 2 : 0) +
                                                                        (out_254&64  ? 4 : 0) +
                                                                        (out_254&2   ? 8 : 0) +
                                                                        (out_254&128 ? 16 : 0) +
                                                                        (out_254&4   ? 32 : 0) ;



                                                        }
                                                        else {

                                                                color_border=out_254&7;
                                                                //basic colours
                                                                //flash?
                                                                if (out_254 & 128) {
                                                                        if (estado_parpadeo.v) {
                                                                                //parpadeo a negro o blanco
                                                                                if (out_254 & 32) color_border=7;
                                                                                else color_border=0;
                                                                        }
                                                                }
                                                                //brillo?
                                                                if (out_254 & 64) color_border+=8;

                                                        }
                                                }
                                                else {
                                                        //standard border
                                                        color_border=out_254 & 7;
                                                }
              }

                                        //no es color spectra
                                        else {
                                                color_border=out_254 & 7;
                                        }


	return color_border;
}

//Alterar el bit 6 de lectura segun los bits 3 y 4 en escritura
//TODO: realmente ese cambio de 1 a 0 va retrasado entre 50 microsegundos y 800 microsegundos,
//pero nosotros estamos haciendo el cambio al momento
void out_port_spectrum_fe_issue(z80_byte value)
{
    //Issue 2
    if (keyboard_issue2.v) {
        if ((value&(8+16))==0) last_bit_6_feh=0;
        else last_bit_6_feh=64;
    }

    //Issue 3
    else {
        if ((value&(16))==0) last_bit_6_feh=0;
        else last_bit_6_feh=64;
    }
}


void out_port_spectrum_border(z80_int puerto,z80_byte value)
{

    //Actualizar bit 6 de puerto feh segun si teclado issue 2 o 3
    out_port_spectrum_fe_issue(value);

	//printf ("out port 254 desde reg_pc=%d. puerto: %04XH value: %02XH\n",reg_pc,puerto,value);

	//Guardamos temporalmente el valor anterior para compararlo con el actual
	//en el metodo de autodeteccion de border real video
	z80_byte anterior_out_254=out_254;

    modificado_border.v=1;
    silence_detection_counter=0;
    beeper_silence_detection_counter=0;

    //ay_player_silence_detection_counter=0;

    out_254_original_value=value;

    //printf ("valor mic y ear: %d\n",value&(8+16));
    //printf ("pc: %d\n",reg_pc);

    if (MACHINE_IS_INVES) {
        //Inves
        //printf ("Inves");
        //AND con valor de la memoria RAM. Este AND en Inves solo aplica a puertos de la ULA

        //temp
        //if (puerto>31*256+254)
        //	printf ("puerto: %d (%XH) valor %d (%XH)\n", puerto,puerto,value,value);

        value=value & memoria_spectrum[puerto];
        out_254=value;

        //y xor con bits 3 y 4
        z80_byte bit3,bit4;
        bit3=value & 8;
        bit4=value & 16;
        bit4=bit4 >> 1;

        bit3= (bit3 ^ bit4) & 8;

        if ( (bit3 & 8) != (ultimo_altavoz & 8) ) {
            //valor diferente. conmutar altavoz
            //bit_salida_sonido.v ^=1;
            set_value_beeper((!!(bit3 & 8) << 1));
        }
        ultimo_altavoz=bit3;


    }
    else {
        out_254=value;

        if (MACHINE_IS_TSCONF) {
            tsconf_af_ports[0xF]=(value&7)|0xF0; //Colores de border mediante puerto FEH siempre suma F0 (254)
        }

        /*
        if ( (value & 24) != (ultimo_altavoz & 24) ) {
        //valor diferente. conmutar altavoz
        bit_salida_sonido.v ^=1;
        }
        ultimo_altavoz=value;
        */

        //Extracted from Fuse emulator
        set_value_beeper( (!!(value & 0x10) << 1) + ( (!(value & 0x8))  ) );
    }


    if (rainbow_enabled.v) {
        //printf ("%d\n",t_estados);
        int i;

        i=t_estados;
        //printf ("t_estados %d screen_testados_linea %d bord: %d\n",t_estados,screen_testados_linea,i);

        //Con esto se ve la ukflag, la confusio y la rage se ven perfectas
        if (pentagon_timing.v) i -=2;

        else {
            //Maquinas no pentagon, pero de 128k
            //esto hace que se vea bien la ula128 y scroll2017
            if (MACHINE_IS_SPECTRUM_128_P2) i+=2;
        }

        //Este i>=0 no haria falta en teoria
        //pero ocurre a veces que justo al activar rainbow, t_estados_linea_actual tiene un valor descontrolado
        if (i>=0 && i<CURRENT_FULLBORDER_ARRAY_LENGTH) {

            //Ajuste valor de indice a multiple de 4 (lo normal en la ULA)
            //O sea, quitamos los 2 bits inferiores
            //Excepto en pentagon
            if (pentagon_timing.v==0) i=i&(0xFFFFFFFF-3);

            //En Inves. Snow in border
            if (MACHINE_IS_INVES) {
                //Enviamos color real, sin mascara a la ROM
                fullbuffer_border[i]=out_254_original_value & 7;

                //E inmediatamente despues, color con mascara
                i++;

                //Si nos salimos del array, volver atras y sobreescribir color real
                if (i==CURRENT_FULLBORDER_ARRAY_LENGTH) i--;

                //Finalmente escribimos valor con mascara a la ROM
                fullbuffer_border[i]=out_254 & 7;
            }

            else {
                int actualiza_fullbuffer_border=1;
                //No si esta desactivado en tbblue
                if (MACHINE_IS_TBBLUE && tbblue_store_scanlines_border.v==0) {
                    actualiza_fullbuffer_border=0;
                }

                if (actualiza_fullbuffer_border) {
                    fullbuffer_border[i]=get_border_colour_from_out();
                }
            }
            //printf ("cambio border i=%d color: %d\n",i,out_254 & 7);
        }
        //else printf ("Valor de border scanline fuera de rango: %d\n",i);
    }


    else {
        //Realvideo desactivado. Si esta autodeteccion de realvideo, hacer lo siguiente
        if (autodetect_rainbow.v) {
            //if (reg_pc>=16384) {
                //Ver si hay cambio de border
                if ( (anterior_out_254&7) != (out_254&7) ) {
                    //printf ("cambio de border\n");
                    detect_rainbow_border_changes_in_frame++;
                }
            //}
        }
    }

    set_value_beeper_on_array(value_beeper);

}


void out_port_spectrum_no_time(z80_int puerto,z80_byte value)
{

    //printf("out_port_spectrum_no_time port %04XH value %02XH\n",puerto,value);

	debug_fired_out=1;
        //Los OUTS los capturan los diferentes interfaces que haya conectados, por tanto no hacer return en ninguno, para que se vayan comprobando
        //uno despues de otro
	z80_byte puerto_l=puerto&255;
	z80_byte puerto_h=(puerto>>8)&0xFF;

        //super wonder boy usa puerto 1fe
        //paperboy usa puertos xxfe
	//Puerto 254 realmente es cualquier puerto par
       	if ( (puerto & 1 )==0 && !(MACHINE_IS_CHLOE) && !(MACHINE_IS_TIMEX_TS_TC_2068) && !(MACHINE_IS_PRISM) ) {

		out_port_spectrum_border(puerto,value);

        }


	//Puerto 254 solamente, para chloe y timex
	if ( puerto_l==254 && (MACHINE_IS_CHLOE || MACHINE_IS_TIMEX_TS_TC_2068 || MACHINE_IS_PRISM) ) {
		out_port_spectrum_border(puerto,value);

        }


//ay chip
/*
El circuito de sonido contiene dieciseis registros; para seleccionarlos, primero se escribe
el numero de registro en la puerta de escritura de direcciones, FFFDh (65533), y despues
lee el valor del registro (en la misma direccion) o se escribe en la direccion de escritura
de registros de datos, BFFDh (49149). Una vez seleccionado un registro, se puede realizar
cualquier numero de operaciones de lectura o escritura de datos. S~1o habr~ que volver
escribir en la puerta de escritura de direcciones cuando se necesite seleccio~ar otro registro.
La frecuencia de reloj basica de este circuito es 1.7734 MHz (con precision del 0.01~~o).
*/
	//el comportamiento de los puertos ay es con mascaras AND... esto se ve asi en juegos como chase hq
/*
Peripheral: 128K AY Register.
Port: 11-- ---- ---- --0-

Peripheral: 128K AY (Data).
Port: 10-- ---- ---- --0-

49154=1100000000000010

*/

	//Puertos Chip AY
	if ( ((puerto & 49154) == 49152) || ((puerto & 49154) == 32768) ) {
		z80_int puerto_final=puerto;

		if ((puerto & 49154) == 49152) puerto_final=65533;
		else if ((puerto & 49154) == 32768) puerto_final=49149;

		activa_ay_chip_si_conviene();
		if (ay_chip_present.v==1) out_port_ay(puerto_final,value);

	}

	//samram
	if (MACHINE_IS_SPECTRUM_48 && puerto_l==31 && samram_enabled.v) {
	  samram_write_port(value);
	}


   //Test betadisk
    if (betadisk_enabled.v) {
        if (betadisk_check_if_rom_area(reg_pc)) {


         if (puerto_l==0xFF) {
               //debug_printf (VERBOSE_DEBUG,"Writing port betadisk %02X%02XH value %02XH Beta Disk Status PC=%04XH",puerto_h,puerto_l,value,reg_pc);
         }

         if (puerto_l==0x1f) {
               //debug_printf (VERBOSE_DEBUG,"Writing port betadisk %02X%02XH value %02XH Beta Disk FDC Status PC=%04XH",puerto_h,puerto_l,value,reg_pc);
          }

         if (puerto_l==0x3f) {
               //debug_printf (VERBOSE_DEBUG,"Writing port betadisk %02X%02XH value %02XH Beta Disk FDC Track PC=%04XH",puerto_h,puerto_l,value,reg_pc);
               betadisk_temp_puerto_3f=value;
         }

         if (puerto_l==0x5f) {
               //debug_printf (VERBOSE_DEBUG,"Writing port betadisk %02X%02XH value %02XH Beta Disk FDC Sector PC=%04XH",puerto_h,puerto_l,value,reg_pc);
               betadisk_temp_puerto_5f=value;
         }


         if (puerto_l==0x7f) {
                //debug_printf (VERBOSE_DEBUG,"Writing port betadisk %02X%02XH value %02XH Beta Disk FDC Data PC=%04XH",puerto_h,puerto_l,value,reg_pc);
                betadisk_temp_puerto_7f=value;
         }
     }
    }



	//Puertos de Paginacion
	if (MACHINE_IS_SPECTRUM_128_P2)
	{
		//Para 128k
		//Puerto tipicamente 32765
		//The additional memory features of the 128K/+2 are controlled to by writes to port 0x7ffd. As normal on Sinclair hardware, the port address is in fact only partially decoded and the hardware will respond to any port address with
		//bits 1 and 15 reset. However, 0x7ffd should be used if at all possible to avoid conflicts with other hardware.

		if ( (puerto & 32770) == 0 ) {

			//printf ("paginacion pc: %d\n",reg_pc);
			//printf ("puerto_32765: %d enviando valor: %d\n",puerto_32765,value);

			//ver si paginacion desactivada
			//if (puerto_32765 & 32) return;

			//if ((puerto_32765 & 32)==0) {
			if (mem_paging_is_enabled()) {

				puerto_32765=value;
				//Paginar RAM y ROM
                	        //32 kb rom, 128 ram

	                        //asignar ram
        	                mem_page_ram_128k();

                	        //asignar rom
                        	mem_page_rom_128k();

			}



        }
	}

	if (MACHINE_IS_SPECTRUM_P2A_P3)
	{
		//Para +2A


		//Puerto tipicamente 32765
		// the hardware will respond only to those port addresses
		//with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
	        if ( (puerto & 49154) == 16384 ) {
			mem128_p2a_write_page_port(puerto,value);


		}

		//Puerto tipicamente 8189

		// the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
		if ( (puerto & 61442 )== 4096) {
			mem128_p2a_write_page_port(puerto,value);

            //Puertos disco +3. Motor on/off
            if (pd765_enabled.v) {

                if (value&8) {
                    dsk_show_activity();
                    pd765_motor_on();
                }
                else {
                    //Pues realmente si motor va a off, no hay actividad
                    pd765_motor_off();
                }

			    //pd765_out_port_1ffd(value);
            }

		}


        //Puertos disco +3
        //Tipicamente 3ffd
        //  3FFD 0011..........0. R/W Spectrum +3 Floppy FDC NEC uPD765 data
        // Twin World e Iron Lord, de Ubisoft, ambos envian valores con OUTI, con valor antes del outi de BC=3ffd
        //el valor de puerto que se envia sera 3efd, y segun esta mascara, funcionara bien.
        //si en cambio solo detectasemos puerto 3ffd, no cargarian
        if ((puerto & 0xF002) == 0x3000 && pd765_enabled.v) {
            pd765_out_port_data_register(value);
        }

	}


	//Puertos paginacion +2A pero en ZXUNO con BOOTM disabled
        if (MACHINE_IS_ZXUNO_BOOTM_DISABLED)
        {


        	        //Puerto tipicamente 32765
	                // the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
        	        if ( (puerto & 49154) == 16384 ) {
                	        zxuno_p2a_write_page_port(puerto,value);


        	        }

	                //Puerto tipicamente 8189

        	        // the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
	                if ( (puerto & 61442 )== 4096) {
        	                zxuno_p2a_write_page_port(puerto,value);

        	        }


        }



	//Puertos paginacion +2A pero en ZXUNO con BOOTM enabled. No hacen ningun efecto
        if (MACHINE_IS_ZXUNO_BOOTM_ENABLED)
        {


                        //Puerto tipicamente 32765
                        // the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
                        if ( (puerto & 49154) == 16384 ) {
                                //zxuno_p2a_write_page_port(puerto,value);
				//printf ("Paginacion 32765 con bootm activo\n");


                        }

                        //Puerto tipicamente 8189

                        // the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
                        if ( (puerto & 61442 )== 4096) {
                                //zxuno_p2a_write_page_port(puerto,value);
				//printf ("Paginacion 8189 con bootm activo\n");

                        }


        }


	//Puerto paginacion 32765 para Chloe
	if (MACHINE_IS_CHLOE) {
        //Puerto tipicamente 32765
        // the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
        if ( (puerto & 49154) == 16384 ) {
            puerto_32765=value;
            chloe_set_memory_pages();
        }

        //Puerto ula2 como en prism. Solo implementamos registro 0 que es el que controla el turbo
        if (puerto==36411) {
            chloe_out_ula2(value);
        }

	}




	//Puertos paginacion para superupgrade
 if (superupgrade_enabled.v)
        {
                //Para +2A


                //Puerto tipicamente 32765
                // the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
                if ( (puerto & 49154) == 16384 ) {
                        superupgrade_write_7ffd(value);

                }

                //Puerto tipicamente 8189

                // the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
                if ( (puerto & 61442 )== 4096) {
                        superupgrade_write_1ffd(value);

                }

			if (puerto==0x43B) {
					superupgrade_write_43b(value);


        		}
	}

	//Puertos paginacion 32765 y 8189 y 60987 para Prism y 36411 para ULA2 prism
	if (MACHINE_IS_PRISM) {
                        //Puerto tipicamente 32765
                        // the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
                        if ( (puerto & 49154) == 16384 ) {
                                puerto_32765=value;
				//Bit 4 de 32765 afecta a Bit 0 de 60987

				prism_rom_page &=(255-1);
				prism_rom_page |= (puerto_32765>>4)&1;

                                prism_set_memory_pages();
                        }


			//Puerto tipicamente 8189
			 // the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
                        if ( (puerto & 61442 )== 4096) {
				puerto_8189=value;
				//Bit 2 de 8189  afecta a Bit 1 de 60987(prism_rom_page)

				prism_rom_page &=(255-2);
				prism_rom_page |= (puerto_8189>>1)&2;

                                prism_set_memory_pages();
                        }

			if (puerto==60987) {
				prism_rom_page=value;
				prism_set_memory_pages();
                        }

			if (puerto==36411) {
				prism_out_ula2(value);
			}

/*0xBE3B ULA2 Palette control - Colour #

Output a logical colour number to this port to select it for definition.
*/
			if (puerto==0xBE3B) {
				prism_ula2_palette_control_colour=value;
				prism_ula2_palette_control_index=0;
			}

/*
0xFE3B (write) Palette control - RGB Data

Write 3 times to redefine red then green then blue levels for the colour selected with 0xBE3B. Accepts an 8 bit value for each colour element (different implementations of ULA2 may resample these values to lower bit-depths depending on the hardware - Prism converts this to 4 bits per element for example).

After the 3rd value has been written, the colour selected for redefinition increments to reduce the number of OUTs needed to redefine consecutive colours.
*/

			if (puerto==0xFE3B) {
				z80_byte col_index=prism_ula2_palette_control_index;
				//Si se ha salido de rango el indice, forzarlo al ultimo
				if (col_index>=3) col_index=2;

				prism_ula2_palette_control_rgb[col_index]=value;
				col_index++;
				if (col_index>=3) {
					col_index=0;
					//cambio color palette 2
					int rgb_12bit_color;
					int r=prism_ula2_palette_control_rgb[0]/16;
					int g=prism_ula2_palette_control_rgb[1]/16;
					int b=prism_ula2_palette_control_rgb[2]/16;

					rgb_12bit_color=b+(g<<4)+(r<<8);

					prism_palette_two[prism_ula2_palette_control_colour]=rgb_12bit_color;


					prism_ula2_palette_control_colour++;


				}


				prism_ula2_palette_control_index=col_index;
			}




/*
antiguo: 0x9E3B (write) - ULA2's 256 colour BORDER

The border is set to this colour when the "BORDER 0" command has been issued
(BORDER 1, BORDER 2 etc all work as expected on a normal Spectrum).
This register defaults to '0' so Spectrum software setting a black border
acts as expected unless this registe is explicitly changed by the user/software.
*/

			/*if (puerto==0x9E3B) {
				prism_ula2_border_colour=value;
				modificado_border.v=1;

				int i;
				i=t_estados;
	                        //printf ("t_estados %d screen_testados_linea %d bord: %d\n",t_estados,screen_testados_linea,i);

        	                //Este i>=0 no haria falta en teoria
                	        //pero ocurre a veces que justo al activar rainbow, t_estados_linea_actual tiene un valor descontrolado
                        	if (i>=0 && i<CURRENT_PRISM_BORDER_BUFFER) {

	                               prism_ula2_border_colour_buffer[i]=value;


        	                }




			}*/

            if (puerto==0x9E3B) {
                prism_out_9e3b(value);
            }


            if (puerto==0xAE3B) {
                prism_last_ae3b=value;
            }


        }


				//Puerto paginacion 32765 para Chrome
				if (MACHINE_IS_CHROME) {
						//Puerto tipicamente 32765
															// the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
						if ( (puerto & 49154) == 16384  && ((puerto_32765 & 32)==0)  ) {
							puerto_32765=value;
							chrome_set_memory_pages();
						}

						//Puerto tipicamente 8189
						 // the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
						if ( si_chrome_features_enabled() && ((puerto_32765 & 32)==0) && (puerto & 61442 )== 4096) {
//printf ("TBBLUE changing port 8189 value=0x%02XH\n",value);
										puerto_8189=value;


										chrome_set_memory_pages();
						}

				}

				if (MACHINE_IS_PENTAGON) {
					if (puerto==0xeff7) {
						z80_byte estado_antes=puerto_eff7 & 1;
						puerto_eff7=value;

						//splash si el modo esta disponible
						if (pentagon_16c_mode_available.v) {

						if ( (value&1) != estado_antes) {
							if (value) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling 16C mode");
							else screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling 16C mode");
						}
						}
					}



				}

                if (gs_enabled.v) {
                    if (puerto_l==0xBB) gs_write_port_bb_from_speccy(value);
                    if (puerto_l==0xB3) gs_write_port_b3_from_speccy(value);
                }


				//Puertos de Paginacion
				if (MACHINE_IS_TSCONF)
				{
					//Para 128k
					//Puerto tipicamente 32765
					//The additional memory features of the 128K/+2 are controlled to by writes to port 0x7ffd. As normal on Sinclair hardware, the port address is in fact only partially decoded and the hardware will respond to any port address with bits 1 and 15 reset. However, 0x7ffd should be used if at all possible to avoid conflicts with other hardware.


					if ( (puerto & 32770) == 0 ) {

						//printf ("paginacion pc: %d\n",reg_pc);
						//printf ("puerto_32765: %d enviando valor: %d\n",puerto_32765,value);

						//ver si paginacion desactivada
						//if (puerto_32765 & 32) return;

						//printf ("Paginando mediante 32765. valor: %02XH\n",value);


						//if (1==1) {
						if ((puerto_32765 & 32)==0) {
                        //if (puerto==32765) {

							puerto_32765=value;

							//Port 0x7FFD is an alias of Page3, page3=tsconf_af_ports[0x13];
							tsconf_af_ports[0x13]=value&7;



							//Bit 4 de 32765 es bit 0 de #21AF
							z80_byte memconfig=tsconf_af_ports[0x21];
							memconfig &=(255-1); //Reset del bit 0

							//Bit vshadow
							z80_byte vpage=(value&8 ? 7 : 5);
							tsconf_af_ports[1]=vpage;


							//Y ponemos a 1 si conviene
							if (value&16) memconfig |=1;

							tsconf_af_ports[0x21]=memconfig;

							//Paginar RAM y ROM
							tsconf_set_memory_pages();


						}

						else {
							//printf ("Paginacion 32765 desactivada\n");
						}
			    }

					//Puertos NVRAM
					if (puerto==0xeff7) puerto_eff7=value;
					if (puerto==0xdff7) zxevo_last_port_dff7=value;


					if (puerto==0xbff7) {
						//Si esta permitida la escritura
						if (puerto_eff7&128) zxevo_nvram[zxevo_last_port_dff7]=value;
					}

					if (puerto_l==0xaf) tsconf_write_af_port(puerto_h,value);

					//Puerto de baseconf que parece que aqui no se usa
					/*if (puerto_l==0xbf) {
						printf ("Escritura puerto 0xbf valor: %02XH\n",value);
						sleep(1);
					}*/


					//Puertos SD
					//if (puerto_l==0x57 || puerto_l==0x77) printf ("Writing SD port %04XH value %02XH\n",puerto,value);

					if (puerto_l==0x77 && mmc_enabled.v) {
						//printf ("Writing SD port %04Xh value %02XH\n",puerto,value);
/*
Bits 7 .. 2: set to 0 for compatibility

Bit 1: signal CS, 1 after the reset, set to 0 to select the SD-card

Bit 0: Set to 1 for compatibility with Z-controller


        comp->sdc->on = val & 1;
        comp->sdc->cs = (val & 2) ? 1 : 0;

*/

						baseconf_sd_enabled=value&1;
						baseconf_sd_cs=(value & 2) ? 1 : 0;

						mmc_cs(0xFE);  //TODO. Este valor FE deberia depender de baseconf_sd_cs...
					}

					if (puerto_l==0x57 && mmc_enabled.v) {
						//printf ("Writing SD port %04XH 57h value %02XH\n",puerto,value);

/* Envia valores FF y no se porque. Quien entienda esto que me lo explique...

Note: in the cycle of exchange on the SPI, initiated by writing or reading port # xx57, occurs simultaneously sending bytes to the SD-card and receive bytes from it. Sending byte is the same as that recorded in the port (if the cycle is initiated by the exchange of records), or # FF, if the cycle of exchange initiated by reading the port.
Received bytes is stored in an internal buffer and is available for the reading from the same port. This reading of the re-initiates the cycle of exchange, etc.
Allowed to read / write port # xx57 teams INIR and OTIR. Example of reading the sector:

	LD	C,#57
	LD	B,0
	INIR
	INIR

*/
					if (!baseconf_sd_enabled || baseconf_sd_cs) return;

						mmc_write(value);
					}

					//Puertos ZIFI
					if (puerto==TSCONF_ZIFI_COMMAND_REG) tsconf_zifi_write_command_reg(value);
					if (puerto==TSCONF_ZIFI_DATA_REG) tsconf_zifi_write_data_reg(value);



					//Otros puertos en escritura, hacer debug
					if ( (puerto & 32770) != 0 && puerto_l!=0xFE && puerto_l!=0xAF) {
						//printf ("Writing TSConf port %04XH value %02XH\n",puerto,value);
					}
				}

	if (MACHINE_IS_BASECONF) {
		if (puerto_l==0xBF || puerto_l==0x77 || (puerto&0x0FFF)==0xff7 || (puerto&0x0FFF)==0x7f7 || puerto==0x7ffd || puerto==0xeff7
			|| puerto==0xEFF7 || puerto==0xDFF7 || puerto==0xDEF7 || puerto==0xBFF7 || puerto==0xBEF7)
		{
			//printf ("Out port baseconf port %04XH value %02XH. PC=%04XH\n",puerto,value,reg_pc);
			baseconf_out_port(puerto,value);
		}

		else {
			if (puerto_l!=0xFE) {
				//printf ("Unhandled Out port baseconf port %04XH value %02XH. PC=%04XH\n",puerto,value,reg_pc);
			}
		}
	}

	//Puertos especiales de TBBLUE y de paginacion 128kb
	if (MACHINE_IS_TBBLUE) {
		//if (puerto==0x24D9 || puerto==0x24DB || puerto==0x24DD || puerto==0x24DF) tbblue_out_port(puerto,value);
		if (puerto==TBBLUE_REGISTER_PORT) tbblue_set_register_port(value);
		if (puerto==TBBLUE_VALUE_PORT) tbblue_set_value_port(value);

		//Puerto tipicamente 32765
		// the hardware will respond only to those port addresses with
		//bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
        if ( (puerto & 49154) == 16384 ) tbblue_out_port_32765(value);


		//Puerto tipicamente 8189
			// the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
		if ( (puerto & 61442 )== 4096) tbblue_out_port_8189(value);

		if (puerto==TBBLUE_SPRITE_INDEX_PORT)	tbblue_out_port_sprite_index(value);
		if (puerto==TBBLUE_LAYER2_PORT) tbblue_out_port_layer2_value(value);

		//if (puerto_l==TBBLUE_SPRITE_PALETTE_PORT)	tbblue_out_sprite_palette(value);
		if (puerto_l==TBBLUE_SPRITE_PATTERN_PORT) tbblue_out_sprite_pattern(value);

		//Mantenemos puerto 0x55 port compatibilidad temporalmente. Se eliminara al subir nueva beta o la estable
		if (puerto_l==0x55) tbblue_out_sprite_pattern(value);

		if (puerto_l==TBBLUE_SPRITE_SPRITE_PORT) tbblue_out_sprite_sprite(value);

		if (puerto==DS1307_PORT_CLOCK) ds1307_write_port_clock(value);
		if (puerto==DS1307_PORT_DATA) ds1307_write_port_data(value);

		if (puerto==TBBLUE_UART_TX_PORT) tbblue_uartbridge_writedata(value);


	}

	if (datagear_dma_emulation.v && (puerto_l==DATAGEAR_DMA_FIRST_PORT || puerto_l==DATAGEAR_DMA_SECOND_PORT) ) {
			//printf ("Writing Datagear DMA port %04XH with value %02XH\n",puerto,value);
			datagear_write_value(value);
	}

	//Fuller audio box
	//The sound board works on port numbers 0x3f and 0x5f. Port 0x3f is used to select the active AY register and to
	//receive data from the AY-3-8912, while port 0x5f is used for sending data
	//Interfiere con MMC
	if (zxmmc_emulation.v==0 && ( puerto==0x3f || puerto==0x5f) ) {
		activa_ay_chip_si_conviene();
		if (ay_chip_present.v==1) {
			z80_int puerto_final=puerto;
			//printf("out Puerto sonido chip AY puerto=%d valor=%d\n",puerto,value);
                	if (puerto==0x3f) puerto_final=65533;
	                else if (puerto==0x5f) puerto_final=49149;
        	        //printf("out Puerto cambiado sonido chip AY puerto=%d valor=%d\n",puerto,value);

                	out_port_ay(puerto_final,value);

		}
	}

	//zx printer
	if ((puerto&0xFF)==0xFB) {
		if (zxprinter_enabled.v==1) {
			zxprinter_write_port(value);

		}
	}



  //UlaPlus
  //if (ulaplus_presente.v && (puerto&0xFF)==0x3b) {
	if (ulaplus_presente.v && (puerto==0xbf3b || puerto==0xff3b) ) {
		ulaplus_write_port(puerto,value);
	}

	//ZXUNO
	if (MACHINE_IS_ZXUNO && (puerto==0xFC3B  || puerto==0xFD3B)) {
		zxuno_write_port(puerto,value);
		//return;
	}

	//Puerto para modos extendidos ulaplus o seleccion modo turbo chloe, pero cuando la maquina no es zxuno
	if (!MACHINE_IS_ZXUNO && (puerto==0xFC3B  || puerto==0xFD3B)) {
		if (puerto==0xFC3B) last_port_FC3B=value;

		if (puerto==0xFD3B) {

	        zxuno_ports[last_port_FC3B]=value;
			if (last_port_FC3B==0x40) ulaplus_set_extended_mode(value);

			if (MACHINE_IS_CHLOE && last_port_FC3B==0x0B) {
				zxuno_set_emulator_setting_scandblctrl();
			}
		}
	}

	//Puertos divmmc sin tener que habilitar divmmc. Solo si divmmc no esta activado
	//if (MACHINE_IS_TBBLUE && divmmc_enabled.v==0 && mmc_enabled.v==1) {
	//	if (puerto_l == 0xE7) mmc_cs(value);
	//	if (puerto_l == 0xEB) mmc_write(value);
	//}


	//Puertos ZXMMC. Interfiere con Fuller Audio Box
	if (zxmmc_emulation.v && (puerto_l==0x1f || puerto_l==0x3f)) {
		//printf ("Puerto ZXMMC Write: 0x%02x valor: 0x%02X\n",puerto_l,value);
		if (puerto_l==0x1f) mmc_cs(value);
		if (puerto_l==0x3f) mmc_write(value);
	}

       //Puertos 8-bit simple ide
        if (eight_bit_simple_ide_enabled.v && (puerto_l&16)==0) {
                eight_bit_simple_ide_write(puerto_l,value);
        }


        //Puertos DIVMMC. El de MMC
        if (divmmc_mmc_ports_enabled.v && (puerto_l==0xe7 || puerto_l==0xeb) ) {
		//printf ("Puerto DIVMMC Write: 0x%02x valor: 0x%02X\n",puerto_l,value);
        	//Si en ZXUNO y DIVEN desactivado
                //Aunque cuando se toca el bit DIVEN de ZX-Uno se sincroniza divmmc_enable,
                //pero por si acaso... por si se activa manualmente desde menu del emulador
                //el divmmc pero zxuno espera que este deshabilitado, como en la bios
	        //if (MACHINE_IS_ZXUNO_DIVEN_DISABLED) return;
		//if (!MACHINE_IS_ZXUNO_DIVEN_DISABLED) {

	                if (puerto_l==0xe7) {
				//Parece que F6 es la tarjeta 0 en divmmc
				if (value==0xF6) value=0xFE;
				mmc_cs(value);
			}
        	        if (puerto_l==0xeb) mmc_write(value);
		//}
        }

        //Puertos DIVIDE. El de IDE
        if (divide_ide_ports_enabled.v && ( (puerto_l&(128+64+32+2+1))==128+32+2+1) ) {
                //printf ("Puerto DIVIDE Write: 0x%02x command: %d valor: 0x%02X\n",puerto_l,(puerto_l>>2)&7,value);
		//So you can access all
		//eight IDE-registers from so caled command block (rrr=0..7) at addresses
		//xxxx xxxx  101r rr11

		ide_write_command_block_register((puerto_l>>2)&7,value);


        }



	//Puertos DIVMMC/DIVIDE. El de Paginacion
	if (diviface_enabled.v && puerto_l==0xe3) {
	        //Si en ZXUNO y DIVEN desactivado
                //Aunque cuando se toca el bit DIVEN de ZX-Uno se sincroniza divmmc_enable,
                //pero por si acaso... por si se activa manualmente desde menu del emulador
                //el divmmc pero zxuno espera que este deshabilitado, como en la bios
        	//if (MACHINE_IS_ZXUNO_DIVEN_DISABLED) return;
        	if (!MACHINE_IS_ZXUNO_DIVEN_DISABLED) {

			//printf ("Puerto control paginacion DIVMMC Write: 0x%02x valor: 0x%02X\n",puerto_l,value);
			//printf ("antes control register: %02XH paginacion automatica: %d\n",diviface_control_register,diviface_paginacion_automatica_activa.v);
			diviface_write_control_register(value);
			//printf ("despues control register: %02XH paginacion automatica: %d\n",diviface_control_register,diviface_paginacion_automatica_activa.v);
		}
	}


        //Puerto Spectra
        if (spectra_enabled.v && puerto==0x7FDF) spectra_write(value);

	//Sprite Chip
	if (spritechip_enabled.v && (puerto==SPRITECHIP_COMMAND_PORT || puerto==SPRITECHIP_DATA_PORT) ) spritechip_write(puerto,value);


	//Puerto HiLow Datadrive
	if (hilow_enabled.v && puerto_l==0xFF) {
		hilow_write_port_ff(puerto,value);
	}

    //Puerto HiLow Barbanegra
	if (hilow_bbn_enabled.v && puerto_l==0xFD) {
		hilow_bbn_write_port_fd(puerto,value);
	}

    //Puerto Phoenix
	if (phoenix_enabled.v && puerto_l==0xDF) {
		phoenix_write_port_df();
	}

    //Puerto Ramjet
    //a7 a 0. a11=a15=1
    if (ramjet_enabled.v && (puerto & 0x8880) == 0x8800) {
        ramjet_write_port(value);
    }

    //Puertos transtape
    if (transtape_enabled.v) {
        //Bit 0 a 1, luego dependiendo bits 6 y 7
        //00xxxxxxx1, 01xxxxxxx1, 10xxxxxxx1
        z80_byte puerto_enmascarado=puerto_l & (1+64+128);
        if (puerto_enmascarado==1 || puerto_enmascarado==65 || puerto_enmascarado==129) {
            transtape_write_port(puerto_enmascarado,value);
        }
    }


	//Puerto Timex Video. 8 bit bajo a ff
	if (timex_video_emulation.v && puerto_l==0xFF) {
		set_timex_port_ff(value);
	}


	//Puerto Timex Paginacion
	if (puerto_l==0xf4 && (MACHINE_IS_CHLOE || MACHINE_IS_TIMEX_TS_TC_2068 || MACHINE_IS_PRISM || is_zxuno_chloe_mmu()) ) {

		//Si prism y puerto f4 desactivado
		if (MACHINE_IS_PRISM) {
			//0001 - Register 1 - Compatiblity Options
			//xx1x - Disable port F4 (disable Timex/Chloe/SE sideways RAM paging)
			if ((prism_ula2_registers[1]&2)==0) {
				timex_port_f4=value;
			}

		}

		else {
			timex_port_f4=value;
		}

		if (MACHINE_IS_CHLOE_280SE) chloe_set_memory_pages();
		if (MACHINE_IS_PRISM) prism_set_memory_pages();
		if (MACHINE_IS_TIMEX_TS_TC_2068) timex_set_memory_pages();
		if (is_zxuno_chloe_mmu() ) zxuno_set_memory_pages();

    }


	//Puertos AY para Timex y Chloe
	if (puerto_l==0xf5 || puerto_l==0xf6) {
		if (MACHINE_IS_CHLOE || MACHINE_IS_TIMEX_TS_TC_2068) {
            z80_int puerto_final=puerto;
            if (puerto_l==0xf5) puerto_final=65533;
            else puerto_final=49149;

            activa_ay_chip_si_conviene();
            if (ay_chip_present.v==1) out_port_ay(puerto_final,value);
		}
    }





	//DAC Audio
	if (audiodac_enabled.v && puerto_l==audiodac_types[audiodac_selected_type].port) {
		//Parche para evitar bug de envio de sonido en esxdos. que activa modo turbo en zxbadaloc y coincide con puerto DF de Specdrum
		//printf ("PC %04XH\n",reg_pc);
		int sonido=1;

		if (puerto_l==0xDF && reg_pc<0x2000) sonido=0;

		if (sonido) {
			audiodac_send_sample_value(value);
			//audiodac_last_value_data=value;
			//silence_detection_counter=0;
		}
	}


	//No estoy seguro que esto también haga falta en el caso de OUT
	//Parece que si lo habilito aqui tambien, hacer un "save" desde multiface no funciona, se cuelga
	if (multiface_enabled.v) {
		if (puerto_l==0x3f && multiface_type==MULTIFACE_TYPE_128) {
			//printf ("Setting multiface lockout to 1\n");
			multiface_lockout=1;
		}
	}




	/*
	Parece que hay especificados estos otros 3 puertos para audiodac, aunqe no veo que se usen, mas alla de enviar un 80H
	en cada canal al cargar la demo. Supuestamente tiene 3 canales pero sumando el DF y estos 3 puertos ya son 4???
	if (puerto_l==0xFF || puerto_l==0x9f || puerto_l==0xbf) {
		printf ("Puerto audiodac : %04XH value: %d\n",puerto,value);
	}*/

//ZEsarUX ZXI ports
    if (hardware_debug_port.v) {
	   if (puerto==ZESARUX_ZXI_PORT_REGISTER) zesarux_zxi_write_last_register(value);
	   if (puerto==ZESARUX_ZXI_PORT_DATA)     zesarux_zxi_write_register_value(value);
    }


	//Prueba pentevo
	if (puerto_l==0xAF) {
		//printf ("Out port Pentevo %04XH value %02XH\n",puerto,value);
	}


	//debug_printf (VERBOSE_DEBUG,"Out Port %x unknown written with value %x, PC after=0x%x",puerto,value,reg_pc);
}





void out_port_spectrum(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_spectrum_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}


void out_port_zx80(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_zx80_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}

void out_port_zx81(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_zx81_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}





//Pokea parte de la ram oculta del inves con valor
void poke_inves_rom(z80_byte value)
{
        z80_int dir;

	/*
        Bit   7   6   5   4   3   2   1   0
            +-------------------------------+
            |   |   |   | E | M |   Border  |
            +-------------------------------+

	Bits y valores usados normalmente al enviar sonido son 5 bits inferiores (0...31)
	Pero por ejemplo el juego Hysteria (A year after mix) usa todos los bits.... con el resultado que los puertos usados
	van desde el 0 hasta el 65534! Entonces las direcciones AND que usa son toda la RAM!

	//31*256+254=8190
	*/

	//Pokeamos toda la ram baja aunque en la mayoria de juegos (no el hysteria) seria suficiente con ir desde 0 hasta 8190
        for (dir=0;dir<16384;dir++) {
                poke_byte_no_time(dir,value);
        }
}
