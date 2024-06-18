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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "settings.h"

#include "contend.h"
#include "utils.h"
#include "z88.h"

#include "compileoptions.h"
#include "snap_rzx.h"
#include "esxdos_handler.h"

z80_int *registro_ixiy;

void instruccion_0()
{
//NOP
}

void instruccion_1 ()
//LD BC,NN
{
	reg_c=lee_byte_pc();
	reg_b=lee_byte_pc();
}

void instruccion_2()
//LD (BC),A
{
	poke_byte(BC,reg_a);

#ifdef EMULATE_MEMPTR
        set_memptr(value_8_to_16(reg_a,((BC+1)&0xff)));
#endif

}

void instruccion_3()
//INC BC
{

        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );


	BC++;

}



void instruccion_4()
{

//INC B

        inc_8bit(reg_b);


}

void instruccion_5()
{
//DEC B

	dec_8bit(reg_b);



}


void instruccion_6()
{
//LD B,N
	reg_b=lee_byte_pc();
}




void instruccion_7()
{
//RLCA
	rlca();

}

void instruccion_8()
{

//EX AF,AF'
	z80_byte valor;

	valor=reg_a;
	reg_a=reg_a_shadow;
	reg_a_shadow=valor;

	valor=Z80_FLAGS;
	Z80_FLAGS=Z80_FLAGS_SHADOW;
	Z80_FLAGS_SHADOW=valor;

}

void instruccion_9()
{
//ADD HL,BC
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

	add_hl(BC);

}

void instruccion_10()
{
//LD A,(BC)

	reg_a=peek_byte(BC);
	set_memptr(BC+1);


}



void instruccion_11()
{
//DEC BC
        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );


	BC--;


}


void instruccion_12()
{
//INC C
	inc_8bit(reg_c);
}

void instruccion_13()
{
//DEC C
        dec_8bit(reg_c);
}



void instruccion_14()
{

//LD C,N
        reg_c=lee_byte_pc();

}



void instruccion_15()
{
//RRCA
	rrca();

}

void instruccion_16()
{

//codigo16:               ;DJNZ DIS

	contend_read_no_mreq( IR, 1 );

	reg_b--;

	if (reg_b) {
		JR();
	}

	else {
        	contend_read( reg_pc, 3 );
	}

	reg_pc++;

#ifdef EMULATE_MEMPTR
	if(reg_b) set_memptr(reg_pc);
#endif


}

void instruccion_17()
{
//LD DE,NN
	reg_e=lee_byte_pc();
	reg_d=lee_byte_pc();
}

void instruccion_18()
//LD (DE),A
{
	poke_byte(DE,reg_a);


#ifdef EMULATE_MEMPTR
        set_memptr(value_8_to_16(reg_a,((DE+1)&0xff)));
#endif

}


void instruccion_19()
{

//INC DE

        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );


	DE++;

}



void instruccion_20()
{
//INC D
	inc_8bit(reg_d);
}

void instruccion_21()
{
//DEC D
        dec_8bit(reg_d);
}


void instruccion_22()
{
//LD D,N
        reg_d=lee_byte_pc();
}

void instruccion_23()
{
//RLA
	rla();

}

void instruccion_24()
{
//JR DIS


        JR();
        reg_pc++;

#ifdef EMULATE_MEMPTR
        set_memptr(reg_pc);
#endif


}

void instruccion_25()
{
//ADD HL,DE
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

	add_hl(DE);
}

void instruccion_26()
{
//LD A,(DE)

	reg_a=peek_byte(DE);
	set_memptr(DE+1);


}

void instruccion_27()
{
//DEC DE

        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );


	DE--;

}


void instruccion_28()
{
//INC E
        inc_8bit(reg_e);
}

void instruccion_29()
{
//DEC E
        dec_8bit(reg_e);
}


void instruccion_30()
{
//LD E,N
        reg_e=lee_byte_pc();

}

void instruccion_31()
{
//RRA
	rra();

}

void instruccion_32()
{
//JR NZ,DIS

	if( !(Z80_FLAGS & FLAG_Z) ) {
		JR();
		reg_pc++;
#ifdef EMULATE_MEMPTR
        set_memptr(reg_pc);
#endif
	}
	else {
		contend_read( reg_pc, 3 );
		reg_pc++;
	}





}

void instruccion_33()
{
//LD HL,NN
	reg_l=lee_byte_pc();
	reg_h=lee_byte_pc();

}

void instruccion_34()
{
//LD (NN),HL

	z80_int dir;
        dir=lee_word_pc();

	poke_byte(dir,reg_l);
	poke_byte(dir+1,reg_h);

	set_memptr(dir+1);



}

void instruccion_35()
{

//INC HL
        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );


	HL++;


}



void instruccion_36()
{
//INC H
	inc_8bit(reg_h);
}

void instruccion_37()
{
//DEC H
	dec_8bit(reg_h);
}

void instruccion_38()
{
//LD H,N
        reg_h=lee_byte_pc();
}



void instruccion_39()
{
//DAA
/*
Extraido del documento z80-documented, segun una tabla
*/

	z80_byte low_a,high_a;
	z80_byte diff=0;
	z80_byte flag_C_final=0;
	z80_byte flag_H_final=0;

	//Calculo de valor a sumar
	low_a=reg_a & 0xF;
	high_a=(reg_a>>4) & 0xF;

	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0x0 && high_a<=0x9) && (Z80_FLAGS & FLAG_H)==0 && (low_a>=0x0 && low_a<=0x9)) diff=0x00;
	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0x0 && high_a<=0x9) && (Z80_FLAGS & FLAG_H) && (low_a>=0x0 && low_a<=0x9)) diff=0x06;
	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0x0 && high_a<=0x8) &&                (low_a>=0xa && low_a<=0xf)) diff=0x06;
	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0xa && high_a<=0xf) && (Z80_FLAGS & FLAG_H)==0 && (low_a>=0x0 && low_a<=0x9)) diff=0x60;
	if ((Z80_FLAGS & FLAG_C) &&                                 (Z80_FLAGS & FLAG_H)==0 && (low_a>=0x0 && low_a<=0x9)) diff=0x60;
	if ((Z80_FLAGS & FLAG_C) &&                                 (Z80_FLAGS & FLAG_H) && (low_a>=0x0 && low_a<=0x9)) diff=0x66;
	if ((Z80_FLAGS & FLAG_C) &&                                                (low_a>=0xa && low_a<=0xf)) diff=0x66;
	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0x9 && high_a<=0xf) &&                (low_a>=0xa && low_a<=0xf)) diff=0x66;
	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0xa && high_a<=0xf) && (Z80_FLAGS & FLAG_H) && (low_a>=0x0 && low_a<=0x9)) diff=0x66;


	if ((Z80_FLAGS & FLAG_N)==0) reg_a +=diff;
	else reg_a -=diff;

	//Calculo de flags

	//Calculo flag C
	//if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0x0 && high_a<=0x9) && (low_a>=0x0 && low_a<=0x9)) flag_C_final=0;

	//if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0x0 && high_a<=0x8) && (low_a>=0xa && low_a<=0xf)) flag_C_final=0;

	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0x9 && high_a<=0xf) && (low_a>=0xa && low_a<=0xf)) flag_C_final=FLAG_C;

	if ((Z80_FLAGS & FLAG_C)==0 && (high_a>=0xa && high_a<=0xf) && (low_a>=0x0 && low_a<=0x9)) flag_C_final=FLAG_C;

	if ((Z80_FLAGS & FLAG_C))                                                               flag_C_final=FLAG_C;

	//flag_C.v=flag_C_final.v;
	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_C)) | flag_C_final;


	//Calculo flag H
	//if ((Z80_FLAGS & FLAG_N)==0 &&                (low_a>=0x0 && low_a<=0x9)) flag_H_final=0;

	if ((Z80_FLAGS & FLAG_N)==0 &&                (low_a>=0xa && low_a<=0xf)) flag_H_final=FLAG_H;

	//if ((Z80_FLAGS & FLAG_N) && (Z80_FLAGS & FLAG_H)==0                              ) flag_H_final=0;

	//if ((Z80_FLAGS & FLAG_N) && (Z80_FLAGS & FLAG_H) && (low_a>=0x6 && low_a<=0xf)) flag_H_final=0;

	if ((Z80_FLAGS & FLAG_N) && (Z80_FLAGS & FLAG_H) && (low_a>=0x0 && low_a<=0x5)) flag_H_final=FLAG_H;

	//flag_H.v=flag_H_final.v;
	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_H-FLAG_S-FLAG_Z-FLAG_3-FLAG_5-FLAG_PV)) | flag_H_final | sz53p_table[reg_a];

}

void instruccion_40()
{
//JR Z,DIS

        if (Z80_FLAGS & FLAG_Z) {
                JR();
        	reg_pc++;
#ifdef EMULATE_MEMPTR
        set_memptr(reg_pc);
#endif
        }
        else {
                contend_read( reg_pc, 3 );
	        reg_pc++;
        }




}

void instruccion_41()
{
//ADD HL,HL
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );


	add_hl(HL);
}

void instruccion_42()
{
//LD HL,(NN)

	z80_int dir;

        dir=lee_word_pc();

        reg_l=peek_byte(dir);
        reg_h=peek_byte(dir+1);

	set_memptr(dir+1);

}

void instruccion_43()
{

//DEC HL

        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );


	HL--;

}



void instruccion_44()
{
//INC L
	inc_8bit(reg_l);
}

void instruccion_45()
{
//DEC L
	dec_8bit(reg_l);
}



void instruccion_46()
{
//LD L,N
        reg_l=lee_byte_pc();
}

void instruccion_47()
{

//CPL
	reg_a=reg_a ^ 0xFF;

	Z80_FLAGS = Z80_FLAGS | FLAG_H | FLAG_N ;

        //Flags: H,N a 1. Resto no afectados

	set_undocumented_flags_bits(reg_a);

}


void instruccion_48()
{
//JR NC,DIS

	if (!(Z80_FLAGS & FLAG_C)) {
		JR();
		reg_pc++;
#ifdef EMULATE_MEMPTR
        set_memptr(reg_pc);
#endif
	}
	else {
		contend_read( reg_pc, 3 );
		reg_pc++;
	}




}

void instruccion_49()
{

//LD SP,NN
        reg_sp=lee_word_pc();

}

void instruccion_50()
{
//LD (NN),A

        z80_int dir;

        dir=lee_word_pc();
        poke_byte(dir,reg_a);

#ifdef EMULATE_MEMPTR
	set_memptr(value_8_to_16(reg_a,((dir+1)&0xff)));
#endif

}

void instruccion_51()
{
//INC SP
        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );

        reg_sp++;
        //set_undocumented_flags_bits( (reg_sp>>8)  & 0xFF);

        //Z80_FLAGS &=(255-FLAG_N);

}

void instruccion_52()
{
//INC (HL)
        z80_byte valor;

        valor=peek_byte(HL);

	contend_read_no_mreq( HL, 1 );

        //valor=inc_8bit(valor);
        inc_8bit(valor);
        poke_byte(HL,valor);


}

void instruccion_53()
{

//DEC (HL)
        z80_byte valor;

        valor=peek_byte(HL);

	contend_read_no_mreq( HL, 1 );

        //valor=dec_8bit(valor);
        dec_8bit(valor);
        poke_byte(HL,valor);

}

void instruccion_54()
{

//LD (HL),N

		z80_byte valor;
		valor=lee_byte_pc();
		poke_byte(HL,valor);

}


void aux_scf_ccf_undoc_flags(void)
{
   /*
https://www.worldofspectrum.org/forums/discussion/comment/669314
In other words, the content of A is copied to flags 5+3 after SCF/CCF if the previous operation did set the flags,
whereas it is ORed in there if it didn't set the flags.

Try it with z80flags.tap
*/
#ifdef EMULATE_SCF_CCF_UNDOC_FLAGS
        //printf ("Flags changed before: %d\n",scf_ccf_undoc_flags_after_changed);
        if (scf_ccf_undoc_flags_after_changed) {
                set_undocumented_flags_bits(reg_a);
        }
        else {
                z80_byte temp_value=Z80_FLAGS | reg_a;
                set_undocumented_flags_bits(temp_value);
        }

        //Y en este caso cambiar la variable before a lo contrario que Z80_FLAGS para indicar que ha cambiado el contenido
        scf_ccf_undoc_flags_before=~Z80_FLAGS;
#else
		set_undocumented_flags_bits(reg_a);
#endif
}

void instruccion_55()
{
//SCF
		Z80_FLAGS=(Z80_FLAGS & (255-FLAG_H-FLAG_N)) | FLAG_C;

                aux_scf_ccf_undoc_flags();

}

void instruccion_56()
{

//JR C,DIS

        if (Z80_FLAGS & FLAG_C) {
                JR();
		reg_pc++;
#ifdef EMULATE_MEMPTR
        set_memptr(reg_pc);
#endif
        }
        else {
                contend_read( reg_pc, 3 );
		reg_pc++;
        }




}

void instruccion_57()
{
//ADD HL,SP
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );


	add_hl(reg_sp);


}

void instruccion_58()
{
//LD A,(NN)

        z80_int dir;

        dir=lee_word_pc();
        reg_a=peek_byte(dir);

	set_memptr(dir+1);

}

void instruccion_59()
{
//DEC SP
        contend_read_no_mreq( IR, 1 );
        contend_read_no_mreq( IR, 1 );

        reg_sp--;
        //set_undocumented_flags_bits( (reg_sp>>8) & 0xFF);

        //Z80_FLAGS |=FLAG_N;

}


void instruccion_60()
//INC A
{
	inc_8bit(reg_a);
}


void instruccion_61()
{
//DEC A
	dec_8bit(reg_a);
}

void instruccion_62()
{
//LD A,N
        reg_a=lee_byte_pc();
}

void instruccion_63()
{
//CCF
	z80_byte temp=(Z80_FLAGS & FLAG_C)<<4;

	Z80_FLAGS=(Z80_FLAGS & (255-FLAG_H-FLAG_N))  ^ ( FLAG_C);
	Z80_FLAGS |= temp;

        aux_scf_ccf_undoc_flags();

}


void instruccion_64()
{
//LD B,B
}

void instruccion_65()
{
//LD B,C
        reg_b=reg_c;
}

void instruccion_66()
{
//LD B,D
        reg_b=reg_d;
}

void instruccion_67()
{
//LD B,E
        reg_b=reg_e;
}

void instruccion_68()
{
//LD B,H
        reg_b=reg_h;
}

void instruccion_69()
{
//LD B,L
        reg_b=reg_l;
}

void instruccion_70()
{
//LD B,(HL)
	reg_b=peek_byte(HL);
}

void instruccion_71()
{
//LD B,A
        reg_b=reg_a;
}


void instruccion_72()
{
//LD C,B
        reg_c=reg_b;
}

void instruccion_73()
{
//LD C,C
}

void instruccion_74()
{
//LD C,D
        reg_c=reg_d;
}

void instruccion_75()
{
//LD C,E
        reg_c=reg_e;
}

void instruccion_76()
{
//LD C,H
        reg_c=reg_h;
}

void instruccion_77()
{
//LD C,L
        reg_c=reg_l;
}

void instruccion_78()
{
//LD C,(HL)
	reg_c=peek_byte(HL);
}

void instruccion_79()
{
//LD C,A
        reg_c=reg_a;
}


void instruccion_80()
{
//LD D,B
        reg_d=reg_b;
}

void instruccion_81()
{
//LD D,C
        reg_d=reg_c;
}

void instruccion_82()
{
//LD D,D
}

void instruccion_83()
{
//LD D,E
        reg_d=reg_e;
}

void instruccion_84()
{
//LD D,H
        reg_d=reg_h;
}

void instruccion_85()
{
//LD D,L
        reg_d=reg_l;
}

void instruccion_86()
{
//LD D,(HL)
	reg_d=peek_byte(HL);
}

void instruccion_87()
{
//LD D,A
        reg_d=reg_a;
}


void instruccion_88()
{
//LD E,B
        reg_e=reg_b;
}

void instruccion_89()
{
//LD E,C
        reg_e=reg_c;
}

void instruccion_90()
{
//LD E,D
        reg_e=reg_d;
}

void instruccion_91()
{
//LD E,E
}

void instruccion_92()
{
//LD E,H
        reg_e=reg_h;
}

void instruccion_93()
{
//LD E,L
        reg_e=reg_l;
}

void instruccion_94()
{
//LD E,(HL)
	reg_e=peek_byte(HL);
}

void instruccion_95()
{
//LD E,A
        reg_e=reg_a;
}


void instruccion_96()
{
//LD H,B
        reg_h=reg_b;
}

void instruccion_97()
{
//LD H,C
        reg_h=reg_c;
}

void instruccion_98()
{
//LD H,D
        reg_h=reg_d;
}

void instruccion_99()
{
//LD H,E

        reg_h=reg_e;
}

void instruccion_100()
{
//LD H,H
}

void instruccion_101()
{
//LD H,L
        reg_h=reg_l;
}

void instruccion_102()
{
//LD H,(HL)
	reg_h=peek_byte(HL);
}

void instruccion_103()
{
//LD H,A
        reg_h=reg_a;
}


void instruccion_104()
{
//LD L,B
        reg_l=reg_b;
}

void instruccion_105()
{
//LD L,C
        reg_l=reg_c;
}

void instruccion_106()
{
//LD L,D
        reg_l=reg_d;
}

void instruccion_107()
{
//LD L,E
        reg_l=reg_e;
}

void instruccion_108()
{
//LD L,H
        reg_l=reg_h;
}

void instruccion_109()
{
//LD L,L
}

void instruccion_110()
{
//LD L,(HL)
	reg_l=peek_byte(HL);
}

void instruccion_111()
{
//LD L,A
        reg_l=reg_a;
}



void instruccion_112()
{
//LD (HL),B
	poke_byte(HL,reg_b);
}

void instruccion_113()
{
//LD (HL),C
	poke_byte(HL,reg_c);
}

void instruccion_114()
{
//LD (HL),D
	poke_byte(HL,reg_d);
}

void instruccion_115()
{
//LD (HL),E
	poke_byte(HL,reg_e);
}

void instruccion_116()
{
//LD (HL),H
	poke_byte(HL,reg_h);
}

void instruccion_117()
{
//LD (HL),L
	poke_byte(HL,reg_l);
}

void instruccion_118()
{
//HALT

	z80_halt_signal.v=1;

	//reg_pc--;

	/*
	if (MACHINE_IS_Z88) {
		debug_printf (VERBOSE_DEBUG,"halt en Z88. reg_i=%d",reg_i);
	}
	*/

	if (MACHINE_IS_Z88) z88_enable_coma();

    //Indicar en que scanline se ha producido un halt
    if (debug_settings_show_fired_halt.v) core_spectrum_executed_halt_in_this_scanline=1;
}

void instruccion_119()
{
//LD (HL),A
	poke_byte(HL,reg_a);
}


void instruccion_120()
{
//LD A,B
        reg_a=reg_b;
}

void instruccion_121()
{
//LD A,C
        reg_a=reg_c;
}

void instruccion_122()
{
//LD A,D
        reg_a=reg_d;
}

void instruccion_123()
{
//LD A,E
        reg_a=reg_e;
}

void instruccion_124()
{
//LD A,H
        reg_a=reg_h;
}

void instruccion_125()
{
//LD A,L
        reg_a=reg_l;
}

void instruccion_126()
{
//LD A,(HL)
        reg_a=peek_byte(HL);
}

void instruccion_127()
{
//LD A,A
}


void instruccion_128()
{
//ADD A,B
        add_a_reg(reg_b);
}

void instruccion_129()
{
//ADD A,C
        add_a_reg(reg_c);
}

void instruccion_130()
{
//ADD A,D
        add_a_reg(reg_d);
}

void instruccion_131()
{
//ADD A,E
        add_a_reg(reg_e);
}

void instruccion_132()
{
//ADD A,H
        add_a_reg(reg_h);
}

void instruccion_133()
{
//ADD A,L
        add_a_reg(reg_l);
}


void instruccion_134()
{
//ADD A,(HL)
        add_a_reg( peek_byte(HL)   );
}

void instruccion_135()
{
//ADD A,A
        add_a_reg(reg_a);
}

void instruccion_136()
{
//ADC A,B
        adc_a_reg(reg_b);
}

void instruccion_137()
{
//ADC A,C
        adc_a_reg(reg_c);
}

void instruccion_138()
{
//ADC A,D
        adc_a_reg(reg_d);
}

void instruccion_139()
{
//ADC A,E
        adc_a_reg(reg_e);
}

void instruccion_140()
{
//ADC A,H
        adc_a_reg(reg_h);
}

void instruccion_141()
{
//ADC A,L
        adc_a_reg(reg_l);
}

void instruccion_142()
{
//ADC A,(HL)
        adc_a_reg( peek_byte(HL) );
}

void instruccion_143()
{
//ADC A,A
        adc_a_reg(reg_a);
}



void instruccion_144()
{
//SUB B
        sub_a_reg(reg_b);
}

void instruccion_145()
{
//SUB C
        sub_a_reg(reg_c);
}

void instruccion_146()
{
//SUB D
        sub_a_reg(reg_d);
}

void instruccion_147()
{
//SUB E
        sub_a_reg(reg_e);
}

void instruccion_148()
{
//SUB H
        sub_a_reg(reg_h);
}

void instruccion_149()
{
//SUB L
        sub_a_reg(reg_l);
}


void instruccion_150()
{
//SUB (HL)
        sub_a_reg( peek_byte(HL)   );
}

void instruccion_151()
{
//SUB A
        sub_a_reg(reg_a);
}

void instruccion_152()
{
//SBC A,B
        sbc_a_reg(reg_b);
}

void instruccion_153()
{
//SBC A,C
        sbc_a_reg(reg_c);
}

void instruccion_154()
{
//SBC A,D
        sbc_a_reg(reg_d);
}

void instruccion_155()
{
//SBC A,E
        sbc_a_reg(reg_e);
}

void instruccion_156()
{
//SBC A,H
        sbc_a_reg(reg_h);
}

void instruccion_157()
{
//SBC A,L
        sbc_a_reg(reg_l);
}

void instruccion_158()
{
//SBC A,(HL)
        sbc_a_reg( peek_byte(HL) );
}

void instruccion_159()
{
//SBC A,A
        sbc_a_reg(reg_a);
}



void instruccion_160()
{
//AND B
	and_reg(reg_b);
}

void instruccion_161()
{
//AND C
	and_reg(reg_c);
}

void instruccion_162()
{
//AND D
	and_reg(reg_d);
}

void instruccion_163()
{
//AND E
	and_reg(reg_e);
}

void instruccion_164()
{
//AND H
	and_reg(reg_h);
}

void instruccion_165()
{
//AND L
	and_reg(reg_l);
}

void instruccion_166()
{
//AND (HL)
	and_reg( peek_byte(HL) );
}

void instruccion_167()
{
//AND A
	and_reg(reg_a);
}

void instruccion_168()
{
//XOR B
        xor_reg(reg_b);
}

void instruccion_169()
{
//XOR C
        xor_reg(reg_c);
}

void instruccion_170()
{
//XOR D
        xor_reg(reg_d);
}

void instruccion_171()
{
//XOR E
        xor_reg(reg_e);
}

void instruccion_172()
{
//XOR H
        xor_reg(reg_h);
}

void instruccion_173()
{
//XOR L
        xor_reg(reg_l);
}

void instruccion_174()
{
//XOR (HL)
        xor_reg( peek_byte(HL) );
}

void instruccion_175()
{
//XOR A
        xor_reg(reg_a);
}


void instruccion_176()
{
//OR B
        or_reg(reg_b);
}

void instruccion_177()
{
//OR C
        or_reg(reg_c);
}

void instruccion_178()
{
//OR D
        or_reg(reg_d);
}

void instruccion_179()
{
//OR E
        or_reg(reg_e);
}

void instruccion_180()
{
//OR H
        or_reg(reg_h);
}

void instruccion_181()
{
//OR L
        or_reg(reg_l);
}

void instruccion_182()
{
//OR (HL)
        or_reg( peek_byte(HL) );
}

void instruccion_183()
{
//OR A
        or_reg(reg_a);
}


void instruccion_184()
{
//CP B
        cp_reg(reg_b);
}

void instruccion_185()
{
//CP C
        cp_reg(reg_c);
}

void instruccion_186()
{
//CP D
        cp_reg(reg_d);
}

void instruccion_187()
{
//CP E
        cp_reg(reg_e);
}

void instruccion_188()
{
//CP H
        cp_reg(reg_h);
}

void instruccion_189()
{
//CP L
        cp_reg(reg_l);
}

void instruccion_190()
{
//CP (HL)
        cp_reg( peek_byte(HL) );
}

void instruccion_191()
{
//CP A
        cp_reg(reg_a);
}



void instruccion_192()
{
//RET NZ
	contend_read_no_mreq( IR, 1 );

        if (!(Z80_FLAGS & FLAG_Z)) {
		reg_pc=pop_valor();
	}

}

void instruccion_193()
{
//POP BC
	BC=pop_valor();

}

void instruccion_194()
{
//JP NZ,NN
	z80_int valor;
	valor=lee_word_pc();

	set_memptr(valor);

	if (!(Z80_FLAGS & FLAG_Z)) reg_pc=valor;


}



void instruccion_195()
{
//JP NN
        reg_pc=lee_word_pc();
	set_memptr(reg_pc);
}


/*
void instruccion_196()
{
//CALL NZ,NN
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

        if (!(Z80_FLAGS & FLAG_Z)) call_address(salto);

}
*/

void instruccion_196()
{
//CALL NZ,NN
	if (!(Z80_FLAGS & FLAG_Z)) {
		call();
	}
	else {
		z80_int salto=lee_word_pc();
		set_memptr(salto);
	}


}

void instruccion_197()
{
//PUSH BC
	contend_read_no_mreq( IR, 1 );
	push_valor(  BC , PUSH_VALUE_TYPE_PUSH);
}

void instruccion_198()
{
//ADD A,N
	add_a_reg(lee_byte_pc() );
}

void instruccion_199()
{
//RST 0
	contend_read_no_mreq( IR, 1 );
	rst(0);
}

void instruccion_200()
{
//RET Z
	contend_read_no_mreq( IR, 1 );
	if (Z80_FLAGS & FLAG_Z) {
		reg_pc=pop_valor();
	}
}

void instruccion_201()
{
//RET
	reg_pc=pop_valor();
}

void instruccion_202()
{
//JP Z,NN
        z80_int valor;
        valor=lee_word_pc();

	set_memptr(valor);


        if (Z80_FLAGS & FLAG_Z) reg_pc=valor;

}

//Variable solo usada en instruccion_203. La dejamos aqui para no tenerla que asignar siempre en el stack
z80_byte pref203_opcode_leido;

void instruccion_203()
{

//Prefijo 203 CB

	z80_byte *registro;
	z80_byte numerobit;
	//z80_byte byte_leido2;


                contend_read( reg_pc, 4 );
		pref203_opcode_leido=fetch_opcode ();

#ifdef EMULATE_CPU_STATS
                util_stats_increment_counter(stats_codprcb,pref203_opcode_leido);
#endif

                reg_pc++;
                reg_r++;
								rzx_in_fetch_counter_til_next_int_counter++;


		//printf ("Pref 203 : opcode : %d ",byte_leido2);


		switch (pref203_opcode_leido & 192) {
			case 64:
                                //printf ("aquibit ");
                                registro=devuelve_reg_offset(pref203_opcode_leido & 7);
                                numerobit=(pref203_opcode_leido >> 3) & 7;
				//printf ("Pref 203 : opcode : %d numerobit : %d registro: %d   ",pref203_opcode_leido,numerobit,registro);
                                bit_bit_cb_reg(numerobit,registro);
			break;

			case 128:
				//printf ("aquires ");
				registro=devuelve_reg_offset(pref203_opcode_leido & 7);
				numerobit=(pref203_opcode_leido >> 3) & 7;
				//printf ("Pref 203 : opcode : %d numerobit : %d registro: %d   ",pref203_opcode_leido,numerobit,registro);
				res_bit_cb_reg(numerobit,registro);
			break;
			case 192:
		                //printf ("aquiset ");
		                registro=devuelve_reg_offset(pref203_opcode_leido & 7);
		                numerobit=(pref203_opcode_leido >> 3) & 7;
				//printf ("Pref 203 : opcode : %d numerobit : %d registro: %d   ",pref203_opcode_leido,numerobit,registro);

				//temp
				//if (numerobit==4 && (pref203_opcode_leido & 7)==1) printf ("SET 4,C\n");

		                set_bit_cb_reg(numerobit,registro);
			break;

			default:
				switch(pref203_opcode_leido & 56) {

					case 0:
						rlc_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;

					case 8:
						rrc_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;

					case 16:
						rl_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;

					case 24:
						rr_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;

					case 32:
						sla_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;

					case 40:
						sra_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;

					case 48:
						sls_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;

					case 56:
						srl_cb_reg(devuelve_reg_offset(pref203_opcode_leido & 7) );
					break;


					default:
                                        //Aqui no deberia llegar nunca
					cpu_panic ("Opcode CB. Invalid mask instruction");
					break;
				}
			break;

		}

}

/*
void instruccion_204()
{
//CALL Z,NN
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

        if (Z80_FLAGS & FLAG_Z) call_address(salto);

}
*/

void instruccion_204()
{
//CALL Z,NN
        if ((Z80_FLAGS & FLAG_Z)) {
                call();
        }
        else {
                z80_int salto=lee_word_pc();
                set_memptr(salto);
        }


}


/*
void instruccion_205()
{
//CALL NN
	z80_int salto;
	salto=lee_word_pc();

	set_memptr(salto);

	call_address(salto);

}

*/


void instruccion_205()
{
//CALL NN
                call();
}


void instruccion_206()
{
//ADC A,N
	adc_a_reg ( lee_byte_pc()  );

}

void instruccion_207()
{
//RST 8
	contend_read_no_mreq( IR, 1 );

	if (esxdos_handler_enabled.v && MACHINE_IS_SPECTRUM) {
		esxdos_handler_run();
	}
	else {
		rst(8);
	}
}

void instruccion_208()
{
//RET NC
	contend_read_no_mreq( IR, 1 );
        if (!(Z80_FLAGS & FLAG_C)) {
		reg_pc=pop_valor();
	}

}

void instruccion_209()
{
//POP DE
	DE=pop_valor();

}


void instruccion_210()
{
//JP NC,NN
        z80_int valor;
        valor=lee_word_pc();

	set_memptr(valor);


        if (!(Z80_FLAGS & FLAG_C)) reg_pc=valor;

}

void instruccion_211()
{
//OUT(N),A
	z80_int puerto=(reg_a<<8)|lee_byte_pc();

	//printf ("puerto: %d\n",puerto);
	//printf ("out_port: %p\n",out_port);
	out_port(puerto,reg_a);

#ifdef EMULATE_MEMPTR
        set_memptr(value_8_to_16(reg_a,((puerto+1)&0xff)));
#endif

}

/*
void instruccion_212()
{
//CALL NC,NN
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

        if (!(Z80_FLAGS & FLAG_C)) call_address(salto);

}
*/

void instruccion_212()
{
//CALL NC,NN
        if (!(Z80_FLAGS & FLAG_C)) {
                call();
        }
        else {
                z80_int salto=lee_word_pc();
                set_memptr(salto);
        }


}


void instruccion_213()
{
//PUSH DE
	contend_read_no_mreq( IR, 1 );
	push_valor(  DE , PUSH_VALUE_TYPE_PUSH);
}

void instruccion_214()
{
//SUB N
	sub_a_reg( lee_byte_pc() );
}

void instruccion_215()
{
//RST 16
	contend_read_no_mreq( IR, 1 );
	rst(16);

}

void instruccion_216()
{
//RET C
	contend_read_no_mreq( IR, 1 );
        if (Z80_FLAGS & FLAG_C) {
		reg_pc=pop_valor();
	}

}

void instruccion_217()
{
//EXX
	z80_byte h,l;

	h=reg_b;
	l=reg_c;
	reg_b=reg_b_shadow;
	reg_c=reg_c_shadow;
	reg_b_shadow=h;
	reg_c_shadow=l;



        h=reg_d;
        l=reg_e;
        reg_d=reg_d_shadow;
        reg_e=reg_e_shadow;
        reg_d_shadow=h;
        reg_e_shadow=l;



        h=reg_h;
        l=reg_l;
        reg_h=reg_h_shadow;
        reg_l=reg_l_shadow;
        reg_h_shadow=h;
        reg_l_shadow=l;


}

void instruccion_218()
{
//JP C,NN
        z80_int valor;
        valor=lee_word_pc();

	set_memptr(valor);

        if (Z80_FLAGS & FLAG_C) reg_pc=valor;

}

void instruccion_219()
{
//IN A,(N)
#ifdef EMULATE_MEMPTR
	z80_byte port;
	port=lee_byte_pc();
	set_memptr( (reg_a<<8) + port +1 );
	reg_a=lee_puerto(reg_a,port);
#else
	reg_a=lee_puerto(reg_a,lee_byte_pc());
#endif



}

/*
void instruccion_220()
{
//CALL C,NN
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

        if (Z80_FLAGS & FLAG_C) call_address(salto);


}
*/


void instruccion_220()
{
//CALL C,NN
        if ((Z80_FLAGS & FLAG_C)) {
                call();
        }
        else {
                z80_int salto=lee_word_pc();
                set_memptr(salto);
        }


}


//Variable solo usada en instruccion_221. La dejamos aqui para no tenerla que asignar siempre en el stack
z80_byte pref221_opcode_leido;

void instruccion_221()
{

//Prefijo 221 DD - Instrucciones XY

                contend_read( reg_pc, 4 );


		pref221_opcode_leido=fetch_opcode ();

#ifdef EMULATE_CPU_STATS
		util_stats_increment_counter(stats_codprdd,pref221_opcode_leido);
#endif

                reg_pc++;
                reg_r++;
								rzx_in_fetch_counter_til_next_int_counter++;

		registro_ixiy=&reg_ix;
                codprddfd[pref221_opcode_leido]  () ;



}

void instruccion_222()
{
//SBC A,N
	sbc_a_reg( lee_byte_pc() );
}

void instruccion_223()
{
//RST 24
	contend_read_no_mreq( IR, 1 );
	rst(24);
}

void instruccion_224()
{
//RET PO. P/V=0
	contend_read_no_mreq( IR, 1 );
	if (!(Z80_FLAGS & FLAG_PV)) {
		reg_pc=pop_valor();
	}

}

void instruccion_225()
{
//POP HL
	HL=pop_valor();

}

void instruccion_226()
{
//JP PO,NN. P/V=0
        z80_int valor;
        valor=lee_word_pc();

	set_memptr(valor);


        if (!(Z80_FLAGS & FLAG_PV)) reg_pc=valor;


}

void instruccion_227()
{
//EX (SP),HL

        z80_byte bytetempl, bytetemph;
        bytetempl = peek_byte( reg_sp );
        bytetemph = peek_byte( reg_sp + 1 );
	contend_read_no_mreq( reg_sp + 1, 1 );

        poke_byte( reg_sp + 1, reg_h );
        poke_byte( reg_sp,     reg_l  );
        contend_write_no_mreq( reg_sp, 1 );
	contend_write_no_mreq( reg_sp, 1 );

        reg_l=bytetempl; reg_h=bytetemph;

	set_memptr(HL);


}

/*
void instruccion_228()
{
//CALL PO,NN
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

	if (!(Z80_FLAGS & FLAG_PV)) call_address(salto);
}
*/


void instruccion_228()
{
//CALL PO,NN
        if (!(Z80_FLAGS & FLAG_PV)) {
                call();
        }
        else {
                z80_int salto=lee_word_pc();
                set_memptr(salto);
        }


}


void instruccion_229()
{
//PUSH HL
	contend_read_no_mreq( IR, 1 );
	push_valor(  HL , PUSH_VALUE_TYPE_PUSH);

}

void instruccion_230()
{
//AND N
	z80_byte valor;
        valor=lee_byte_pc();

	and_reg(valor);

}

void instruccion_231()
{
//RST 32
	contend_read_no_mreq( IR, 1 );
	rst(32);

}


void instruccion_232()
{
//RET PE. P/V is set. P/V=1
	contend_read_no_mreq( IR, 1 );
        if (Z80_FLAGS & FLAG_PV) {
		reg_pc=pop_valor();
	}

}

void instruccion_233()
{
//JP (HL)
	reg_pc=HL;
}

void instruccion_234()
{
//JP PE,NN P/V=1
        z80_int valor;
        valor=lee_word_pc();


        if (Z80_FLAGS & FLAG_PV) reg_pc=valor;
}

void instruccion_235()
{
//EX DE,HL



        z80_byte h,l;

        h=reg_h;
        l=reg_l;

        reg_h=reg_d;
        reg_l=reg_e;

        reg_d=h;
        reg_e=l;


}

/*
void instruccion_236()
{
//CALL PE,NN
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

        if (Z80_FLAGS & FLAG_PV) call_address(salto);


}
*/


void instruccion_236()
{
//CALL PE,NN
        if ((Z80_FLAGS & FLAG_PV)) {
                call();
        }
        else {
                z80_int salto=lee_word_pc();
                set_memptr(salto);
        }


}


//Variable usada en instruccion_237 y tambien al generar interrupcion maskable. La dejamos aqui para no tenerla que asignar siempre en el stack
z80_byte pref237_opcode_leido;

void instruccion_237()
{

		//printf ("en instruccion_237\n");
//Prefijo 237 ED

        	contend_read( reg_pc, 4 );
		//printf ("despues de contend_read\n");
		pref237_opcode_leido=fetch_opcode ();

#ifdef EMULATE_CPU_STATS
		util_stats_increment_counter(stats_codpred,pref237_opcode_leido);
#endif

		reg_pc++;
		reg_r++;
		rzx_in_fetch_counter_til_next_int_counter++;

		//printf ("antes de ejecucion 237 %d\n",pref237_opcode_leido);

		codpred[pref237_opcode_leido]  () ;

		//printf ("fin instruccion_237\n");

}



void instruccion_238()
{
//XOR N
        z80_byte valor;
        valor=lee_byte_pc();

        xor_reg(valor);

}

void instruccion_239()
{
//RST 40
	contend_read_no_mreq( IR, 1 );
	rst(40);

}

void instruccion_240()
{
//RET P ;S flag is reset
	contend_read_no_mreq( IR, 1 );
	if (!(Z80_FLAGS & FLAG_S)) {
		reg_pc=pop_valor();
	}

}

void instruccion_241()
{
//POP AF
        z80_int valor;
	z80_byte flags;

        valor=pop_valor();

        reg_a=value_16_to_8h(valor);
        flags=value_16_to_8l(valor);
	store_flags(flags);


}

void instruccion_242()
{
//JP P,NN P=No negativo S flag is 0
        z80_int valor;
        valor=lee_word_pc();

        if (!(Z80_FLAGS & FLAG_S)) reg_pc=valor;

}

void instruccion_243()
{
//DI

	iff1.v=iff2.v=0;
}

/*
void instruccion_244()
{
//CALL P,NN P=No Signo S flag is 0
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

        if (!(Z80_FLAGS & FLAG_S)) call_address(salto);
}
*/

void instruccion_244()
{
//CALL P,NN
        if (!(Z80_FLAGS & FLAG_S)) {
                call();
        }
        else {
                z80_int salto=lee_word_pc();
                set_memptr(salto);
        }


}


void instruccion_245()
{
//PUSH AF
	contend_read_no_mreq( IR, 1 );
	z80_int valor;
	z80_byte flags;

	flags=get_flags();

	valor=value_8_to_16(reg_a,flags);

	push_valor(valor,PUSH_VALUE_TYPE_PUSH);
}

void instruccion_246()
{
//OR N
        z80_byte valor;
        valor=lee_byte_pc();

        or_reg(valor);

}

void instruccion_247()
{
//RST 48
	contend_read_no_mreq( IR, 1 );
	rst(48);
}


void instruccion_248()
{

//RET M M=Signo negativo S flag is 1
	contend_read_no_mreq( IR, 1 );

        if (Z80_FLAGS & FLAG_S) {
		reg_pc=pop_valor();
	}

}


void instruccion_249()
{
//LD SP,HL
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

	reg_sp=HL;


}

void instruccion_250()
{
//JP M,NN M=Signo negativo S flag is 1
        z80_int valor;
        valor=lee_word_pc();


        if (Z80_FLAGS & FLAG_S) reg_pc=valor;

}

void instruccion_251()
{
//EI

       iff1.v=iff2.v=1;

}

/*
void instruccion_252()
{
//CALL M,NN M=Signo negativo. S flag is 1
        z80_int salto;
        salto=lee_word_pc();

	set_memptr(salto);

        if (Z80_FLAGS & FLAG_S) call_address(salto);

}
*/

void instruccion_252()
{
//CALL M,NN
        if ((Z80_FLAGS & FLAG_S)) {
                call();
        }
        else {
                z80_int salto=lee_word_pc();
                set_memptr(salto);
        }


}


//Variable solo usada en instruccion_253. La dejamos aqui para no tenerla que asignar siempre en el stack
z80_byte pref253_opcode_leido;

void instruccion_253()
{

//Prefijo 253 FD - Instrucciones XY

                contend_read( reg_pc, 4 );

		pref253_opcode_leido=fetch_opcode ();

#ifdef EMULATE_CPU_STATS
                util_stats_increment_counter(stats_codprfd,pref253_opcode_leido);
#endif

                reg_pc++;
                reg_r++;
								rzx_in_fetch_counter_til_next_int_counter++;

                registro_ixiy=&reg_iy;
                codprddfd[pref253_opcode_leido]  () ;

}

void instruccion_254()
{
//CP N

        z80_byte valor;
        valor=lee_byte_pc();

        cp_reg(valor);

}

void instruccion_255()
{
//RST 56
	contend_read_no_mreq( IR, 1 );
	rst(56);
}



void (*codsinpr[]) ()   = {
instruccion_0,
instruccion_1,
instruccion_2,
instruccion_3,
instruccion_4,
instruccion_5,
instruccion_6,
instruccion_7,
instruccion_8,
instruccion_9,
instruccion_10,
instruccion_11,
instruccion_12,
instruccion_13,
instruccion_14,
instruccion_15,
instruccion_16,
instruccion_17,
instruccion_18,
instruccion_19,
instruccion_20,
instruccion_21,
instruccion_22,
instruccion_23,
instruccion_24,
instruccion_25,
instruccion_26,
instruccion_27,
instruccion_28,
instruccion_29,
instruccion_30,
instruccion_31,
instruccion_32,
instruccion_33,
instruccion_34,
instruccion_35,
instruccion_36,
instruccion_37,
instruccion_38,
instruccion_39,
instruccion_40,
instruccion_41,
instruccion_42,
instruccion_43,
instruccion_44,
instruccion_45,
instruccion_46,
instruccion_47,
instruccion_48,
instruccion_49,
instruccion_50,
instruccion_51,
instruccion_52,
instruccion_53,
instruccion_54,
instruccion_55,
instruccion_56,
instruccion_57,
instruccion_58,
instruccion_59,
instruccion_60,
instruccion_61,
instruccion_62,
instruccion_63,
instruccion_64,
instruccion_65,
instruccion_66,
instruccion_67,
instruccion_68,
instruccion_69,
instruccion_70,
instruccion_71,
instruccion_72,
instruccion_73,
instruccion_74,
instruccion_75,
instruccion_76,
instruccion_77,
instruccion_78,
instruccion_79,
instruccion_80,
instruccion_81,
instruccion_82,
instruccion_83,
instruccion_84,
instruccion_85,
instruccion_86,
instruccion_87,
instruccion_88,
instruccion_89,
instruccion_90,
instruccion_91,
instruccion_92,
instruccion_93,
instruccion_94,
instruccion_95,
instruccion_96,
instruccion_97,
instruccion_98,
instruccion_99,
instruccion_100,
instruccion_101,
instruccion_102,
instruccion_103,
instruccion_104,
instruccion_105,
instruccion_106,
instruccion_107,
instruccion_108,
instruccion_109,
instruccion_110,
instruccion_111,
instruccion_112,
instruccion_113,
instruccion_114,
instruccion_115,
instruccion_116,
instruccion_117,
instruccion_118,
instruccion_119,
instruccion_120,
instruccion_121,
instruccion_122,
instruccion_123,
instruccion_124,
instruccion_125,
instruccion_126,
instruccion_127,
instruccion_128,
instruccion_129,
instruccion_130,
instruccion_131,
instruccion_132,
instruccion_133,
instruccion_134,
instruccion_135,
instruccion_136,
instruccion_137,
instruccion_138,
instruccion_139,
instruccion_140,
instruccion_141,
instruccion_142,
instruccion_143,
instruccion_144,
instruccion_145,
instruccion_146,
instruccion_147,
instruccion_148,
instruccion_149,
instruccion_150,
instruccion_151,
instruccion_152,
instruccion_153,
instruccion_154,
instruccion_155,
instruccion_156,
instruccion_157,
instruccion_158,
instruccion_159,
instruccion_160,
instruccion_161,
instruccion_162,
instruccion_163,
instruccion_164,
instruccion_165,
instruccion_166,
instruccion_167,
instruccion_168,
instruccion_169,
instruccion_170,
instruccion_171,
instruccion_172,
instruccion_173,
instruccion_174,
instruccion_175,
instruccion_176,
instruccion_177,
instruccion_178,
instruccion_179,
instruccion_180,
instruccion_181,
instruccion_182,
instruccion_183,
instruccion_184,
instruccion_185,
instruccion_186,
instruccion_187,
instruccion_188,
instruccion_189,
instruccion_190,
instruccion_191,
instruccion_192,
instruccion_193,
instruccion_194,
instruccion_195,
instruccion_196,
instruccion_197,
instruccion_198,
instruccion_199,
instruccion_200,
instruccion_201,
instruccion_202,
instruccion_203,
instruccion_204,
instruccion_205,
instruccion_206,
instruccion_207,
instruccion_208,
instruccion_209,
instruccion_210,
instruccion_211,
instruccion_212,
instruccion_213,
instruccion_214,
instruccion_215,
instruccion_216,
instruccion_217,
instruccion_218,
instruccion_219,
instruccion_220,
instruccion_221,
instruccion_222,
instruccion_223,
instruccion_224,
instruccion_225,
instruccion_226,
instruccion_227,
instruccion_228,
instruccion_229,
instruccion_230,
instruccion_231,
instruccion_232,
instruccion_233,
instruccion_234,
instruccion_235,
instruccion_236,
instruccion_237,
instruccion_238,
instruccion_239,
instruccion_240,
instruccion_241,
instruccion_242,
instruccion_243,
instruccion_244,
instruccion_245,
instruccion_246,
instruccion_247,
instruccion_248,
instruccion_249,
instruccion_250,
instruccion_251,
instruccion_252,
instruccion_253,
instruccion_254,
instruccion_255};
