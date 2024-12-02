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

//para memcpy de ldir hack
#include <string.h>

#include "cpu.h"
#include "operaciones.h"
#include "debug.h"

#include "contend.h"
#include "zxuno.h"
#include "tbblue.h"
#include "screen.h"
#include "settings.h"
#include "samram.h"


void invalid_opcode_ed(char *s)
{
	if (debug_shows_invalid_opcode.v) {
                debug_printf(VERBOSE_INFO,"Invalid opcode %s. Final PC: %04XH",s,reg_pc);
	        //printf ("Invalid opcode %s. Final PC: %04XH\n",s,reg_pc);
        }

        //Y acaba ejecutando un NOP tal cual

}


void instruccion_ed_0 ()
{
        invalid_opcode_ed("237 0");
}

void instruccion_ed_1 ()
{
        invalid_opcode_ed("237 1");
}

void instruccion_ed_2 ()
{
        invalid_opcode_ed("237 2");
}

void instruccion_ed_3 ()
{
        invalid_opcode_ed("237 3");
}

void instruccion_ed_4 ()
{
        invalid_opcode_ed("237 4");
}

void instruccion_ed_5 ()
{
        invalid_opcode_ed("237 5");
}

void instruccion_ed_6 ()
{
        invalid_opcode_ed("237 6");
}

void instruccion_ed_7 ()
{
        invalid_opcode_ed("237 7");
}

void instruccion_ed_8 ()
{
        invalid_opcode_ed("237 8");
}

void instruccion_ed_9 ()
{
        invalid_opcode_ed("237 9");
}

void instruccion_ed_10 ()
{
        invalid_opcode_ed("237 10");
}

void instruccion_ed_11 ()
{
        invalid_opcode_ed("237 11");
}

void instruccion_ed_12 ()
{
        invalid_opcode_ed("237 12");
}

void instruccion_ed_13 ()
{
        invalid_opcode_ed("237 13");
}

void instruccion_ed_14 ()
{
        invalid_opcode_ed("237 14");
}

void instruccion_ed_15 ()
{
        invalid_opcode_ed("237 15");
}

void instruccion_ed_16 ()
{
        invalid_opcode_ed("237 16");
}

void instruccion_ed_17 ()
{
        invalid_opcode_ed("237 17");
}

void instruccion_ed_18 ()
{
        invalid_opcode_ed("237 18");
}

void instruccion_ed_19 ()
{
        invalid_opcode_ed("237 19");
}

void instruccion_ed_20 ()
{
        invalid_opcode_ed("237 20");
}

void instruccion_ed_21 ()
{
        invalid_opcode_ed("237 21");
}

void instruccion_ed_22 ()
{
        invalid_opcode_ed("237 22");
}

void instruccion_ed_23 ()
{
        invalid_opcode_ed("237 23");
}

void instruccion_ed_24 ()
{
        invalid_opcode_ed("237 24");
}

void instruccion_ed_25 ()
{
        invalid_opcode_ed("237 25");
}

void instruccion_ed_26 ()
{
        invalid_opcode_ed("237 26");
}

void instruccion_ed_27 ()
{
        invalid_opcode_ed("237 27");
}

void instruccion_ed_28 ()
{
        invalid_opcode_ed("237 28");
}

void instruccion_ed_29 ()
{
        invalid_opcode_ed("237 29");
}

void instruccion_ed_30 ()
{
        invalid_opcode_ed("237 30");
}

void instruccion_ed_31 ()
{
        invalid_opcode_ed("237 31");
}

void instruccion_ed_32 ()
{
        invalid_opcode_ed("237 32");
}

void instruccion_ed_33 ()
{
        invalid_opcode_ed("237 33");
}

void instruccion_ed_34 ()
{
        invalid_opcode_ed("237 34");
}

void instruccion_ed_35 ()
{
        if (MACHINE_IS_TBBLUE) {
                //swapnib           ED 23          4+4  A bits 7-4 swap with A bits 3-0
                z80_byte final_l=(reg_a>>4)&0x0F;
                z80_byte final_h=(reg_a<<4)&0xF0;
                reg_a=final_l|final_h;
        }
        else invalid_opcode_ed("237 35");
}

void instruccion_ed_36 ()
{
        if (MACHINE_IS_TBBLUE) {
                //mirror a          ED 24           4+4 mirror the bits in A
                //76543210 -> 01234567
                int i;
                z80_byte result_a=0;
                for (i=0;i<8;i++) {
                  result_a = result_a >> 1;
                  if (reg_a&128) result_a |=128;
                  reg_a=reg_a << 1;
                }

                reg_a=result_a;
        }
        else invalid_opcode_ed("237 36");
}

void instruccion_ed_37 ()
{
        if (MACHINE_IS_TBBLUE) {
                //ld  hl,sp         ED 25          4+4 transfer SP to HL
                HL=reg_sp;
        }
        else invalid_opcode_ed("237 37");
}

void instruccion_ed_38 ()
{
        invalid_opcode_ed("237 38");
}

void instruccion_ed_39 ()
{

        if (MACHINE_IS_TBBLUE) {
                //test N            ED 27           4+4+3 And A with $XX and set all flags. A is not affected.
                //mismo que AND N pero sin afectar A
                z80_byte valor=lee_byte_pc();
                z80_byte temp_a=reg_a;
                temp_a &= valor;
                Z80_FLAGS=FLAG_H | sz53p_table[temp_a];

        }

        else invalid_opcode_ed("237 39");
}

void instruccion_ed_40 ()
{
    if (!MACHINE_IS_TBBLUE) {
        invalid_opcode_ed("237 40");
        return;
    }

    //BSLA DE,B   ED 28: DE = DE<<(B&31), no flags
    // barrel-shift left of DE, B (5 bits) times
    int shift_amount = reg_b & 31;
    if (0 == shift_amount) return;
    if (16 <= shift_amount) {           // 16+ shifts set DE to zero
        DE = 0;
    } else {
        DE = DE << shift_amount;
    }
}

void instruccion_ed_41 ()
{
    if (!MACHINE_IS_TBBLUE) {
        invalid_opcode_ed("237 41");
        return;
    }

    //BSRA DE,B   ED 29: DE = signed(DE)>>(B&31), no flags
    // aritmetic barrel-shift right of DE, B (5 bits) times
    int shift_amount = reg_b & 31;
    int de_is_negative = (1<<15) & DE;  // extract top bit
    if (0 == shift_amount) return;
    if (15 <= shift_amount) {           // 15+ shifts set DE either to 0 or ~0
        DE = de_is_negative ? ~0 : 0;
    } else {                            // for shift amount 1..14 do the shifting
        z80_int de_bottom_part = DE >> shift_amount;
        z80_int de_upper_part = 0;      // 0 for positive/zero values
        if (de_is_negative) {           // negative values have to fill vacant top bits with ones
            de_upper_part = 0xFFFF << (15-shift_amount);
        }
        DE = de_upper_part | de_bottom_part;
    }
}

void instruccion_ed_42 ()
{
    if (!MACHINE_IS_TBBLUE) {
        invalid_opcode_ed("237 42");
        return;
    }

    //BSRL DE,B   ED 2A: DE = unsigned(DE)>>(B&31), no flags
    // logical barrel-shift right of DE, B (5 bits) times
    int shift_amount = reg_b & 31;
    if (0 == shift_amount) return;
    if (16 <= shift_amount) {           // 16+ shifts set DE to 0
        DE = 0;
    } else {                            // for shift amount 1..15 do the shifting
        DE = DE >> shift_amount;        // DE is unsigned short, C shift is OK
    }
}

void instruccion_ed_43 ()
{
    if (!MACHINE_IS_TBBLUE) {
        invalid_opcode_ed("237 43");
        return;
    }

    //BSRF DE,B   ED 2B: DE = ~(unsigned(~DE)>>(B&31)), no flags
    // barrel-shift right of DE, B (5 bits) times, setting top bits with one
    int shift_amount = reg_b & 31;
    if (0 == shift_amount) return;
    if (16 <= shift_amount) {           // 16+ shifts set DE to ~0
        DE = ~0;
    } else {                            // for shift amount 1..15 do the shifting and setting
        z80_int de_bottom_part = DE >> shift_amount;
        z80_int de_upper_part = 0xFFFF << (16-shift_amount);
        DE = de_upper_part | de_bottom_part;
    }
}

void instruccion_ed_44 ()
{
    if (!MACHINE_IS_TBBLUE) {
        invalid_opcode_ed("237 44");
        return;
    }

    //BRLC DE,B   ED 2C: DE = DE<<(B&15) | DE>>(16-B&15), no flags
    // barrel-roll left without carry of DE, B (4 bits) times
    int rolls_amount = reg_b & 15;
    if (0 < rolls_amount) {
        z80_int de_upper_part = DE<<rolls_amount;
        z80_int de_bottom_part = DE>>(16-rolls_amount);
        DE = de_upper_part | de_bottom_part;
    }
}

void instruccion_ed_45 ()
{
        invalid_opcode_ed("237 45");
}

void instruccion_ed_46 ()
{
        invalid_opcode_ed("237 46");
}

void instruccion_ed_47 ()
{
        invalid_opcode_ed("237 47");
}

void instruccion_ed_48 ()
{
        if (MACHINE_IS_TBBLUE) {
		//mul  d,e          ED 30          4+4  D*E = DE
                z80_int resultado=reg_d*reg_e;

		reg_d=value_16_to_8h(resultado);
		reg_e=value_16_to_8l(resultado);
        }

        else invalid_opcode_ed("237 48");
}

void instruccion_ed_49 ()
{
        if (MACHINE_IS_TBBLUE) {
                //add  hl,a         ED 31          4+4  Add A to HL (no flags set)
                HL +=reg_a;
        }

        else invalid_opcode_ed("237 49");
}

void instruccion_ed_50 ()
{
        if (MACHINE_IS_TBBLUE) {
                //add  de,a         ED 32          4+4  Add A to DE (no flags set)
                DE +=reg_a;
        }

        else invalid_opcode_ed("237 50");
}

void instruccion_ed_51 ()
{
        if (MACHINE_IS_TBBLUE) {
                //add  bc,a         ED 33          4+4  Add A to BC (no flags set)
                BC +=reg_a;
        }

        else invalid_opcode_ed("237 51");
}

void instruccion_ed_52 ()
{
        if (MACHINE_IS_TBBLUE) {
                //add  hl,$0000     ED 34 LO HI     Add XXXX to HL


                z80_int operador;
                operador= lee_byte_pc();
                operador |= (lee_byte_pc()<<8);

                HL +=operador;


        }
        else invalid_opcode_ed("237 52");
}

void instruccion_ed_53 ()
{        if (MACHINE_IS_TBBLUE) {
                //add  de,$0000     ED 35 LO HI     Add XXXX to DE


                z80_int operador;
                operador= lee_byte_pc();
                operador |= (lee_byte_pc()<<8);

                DE +=operador;


        }
        else invalid_opcode_ed("237 53");
}

void instruccion_ed_54 ()
{        if (MACHINE_IS_TBBLUE) {
                //add  bc,$0000     ED 36 LO HI     Add XXXX to BC


                z80_int operador;
                operador= lee_byte_pc();
                operador |= (lee_byte_pc()<<8);

                BC +=operador;


        }
        else invalid_opcode_ed("237 54");
}

void instruccion_ed_55 ()
{
        if (MACHINE_IS_TBBLUE) {
                //inc dehl          ED 37          4+4 increment 32bit DEHL
                z80_long_int dehl= (DE << 16) | HL;
                dehl++;
                HL=dehl & 0xFFFF;
                DE=(dehl>>16) & 0xFFFF;
        }

        else invalid_opcode_ed("237 55");
}

void instruccion_ed_56 ()
{
        if (MACHINE_IS_TBBLUE) {
                //dec dehl          ED 37          4+4 decrement 32bit DEHL
                z80_long_int dehl= (DE << 16) | HL;
                dehl--;
                HL=dehl & 0xFFFF;
                DE=(dehl>>16) & 0xFFFF;
        }

        else invalid_opcode_ed("237 56");
}

void instruccion_ed_57 ()
{
        if (MACHINE_IS_TBBLUE) {
                //add dehl,a        ED 39          4+4 Add A to 32bit DEHL
                z80_long_int dehl= (DE << 16) | HL;
                dehl +=reg_a;
                HL=dehl & 0xFFFF;
                DE=(dehl>>16) & 0xFFFF;
        }

        else invalid_opcode_ed("237 57");
}

void instruccion_ed_58 ()
{
        if (MACHINE_IS_TBBLUE) {
                //add dehl,bc       ED 3A           Add BC to 32bit DEHL
                z80_long_int dehl= (DE << 16) | HL;
                dehl +=BC;
                HL=dehl & 0xFFFF;
                DE=(dehl>>16) & 0xFFFF;
        }

        else invalid_opcode_ed("237 58");
}

void instruccion_ed_59 ()
{
        if (MACHINE_IS_TBBLUE) {
                //add dehl,$0000    ED 3B LO HI    4+4 Add $0000 to 32bit DEHL
                z80_long_int dehl= (DE << 16) | HL;

                z80_int operador=0;
                operador |= lee_byte_pc();
                operador |= (lee_byte_pc()<<8);

                dehl +=operador;
                HL=dehl & 0xFFFF;
                DE=(dehl>>16) & 0xFFFF;
        }

        else invalid_opcode_ed("237 59");
}

void instruccion_ed_60 ()
{
        if (MACHINE_IS_TBBLUE) {
                //sub dehl,a        ED 3C          4+4 Subtract A from 32bit DEHL
                z80_long_int dehl= (DE << 16) | HL;
                dehl -=reg_a;
                HL=dehl & 0xFFFF;
                DE=(dehl>>16) & 0xFFFF;
        }

        else invalid_opcode_ed("237 60");
}

void instruccion_ed_61 ()
{
        if (MACHINE_IS_TBBLUE) {
                //sub dehl,bc       ED 3D          4+4 Subtract BC from 32bit DEHL
                z80_long_int dehl= (DE << 16) | HL;
                dehl -=BC;
                HL=dehl & 0xFFFF;
                DE=(dehl>>16) & 0xFFFF;
        }

        else invalid_opcode_ed("237 61");
}

void instruccion_ed_62 ()
{
        invalid_opcode_ed("237 62");
}

void instruccion_ed_63 ()
{
        invalid_opcode_ed("237 63");
}


void instruccion_ed_64 ()
{
//IN B,(C)

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif
    z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        reg_b=valor_puerto;
        set_flags_in_reg(reg_b);

    }
}

void instruccion_ed_65 ()
{
//OUT (C),B

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

	out_port(BC, reg_b);
}

void instruccion_ed_66 ()
{
//SBC HL,BC
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

        sbc_hl( BC  );
}

void instruccion_ed_67 ()
{
//LD (NN),BC

        z80_int dir;

        dir=lee_word_pc();

        poke_byte(dir,reg_c);
        poke_byte(dir+1,reg_b);

	set_memptr(dir+1);

}

void instruccion_ed_68 ()
{
//NEG

	neg();
}

void instruccion_ed_69 ()
{
	//RETN
	iff1.v=iff2.v;

    if (MACHINE_IS_TBBLUE) {
        //if (!tbblue_pendiente_retn_stackless) printf("Tbblue NO pendiente stackless en retn\n");
        /*
        0xC0 (192) => Interrupt Control
        (R/W) (soft reset = 0)
        bit 3 = Enable stackless nmi response**
        */
        //printf("RETN. tbblue_registers[0xC0] %02XH tbblue_pendiente_retn_stackless %d\n",tbblue_registers[0xC0],tbblue_pendiente_retn_stackless);
        if ((tbblue_registers[0xC0] & 0x08) && tbblue_pendiente_retn_stackless) {
            //printf("RETN. stackless nmi\n");
            tbblue_pendiente_retn_stackless=0;
            reg_sp +=2;
            reg_pc=(tbblue_registers[0xC2])|(tbblue_registers[0xC3]<<8);
            //printf("RETN stackless nmi return to : %04XH\n",reg_pc);

            //tbblue_prueba_dentro_nmi=0;
        }

        else {
            reg_pc=pop_valor();
        }
    }

	else {
        reg_pc=pop_valor();
    }

	//Si se vuelve de una nmi especial
	if (MACHINE_IS_ZXUNO && reg_pc==0x72) {
                //meter BOOTM a 0 (bit 0)
                zxuno_ports[0] &=(255-1);
	}


}

void instruccion_ed_70 ()
{
//coded70:                        ;IM 0
/*
Codificación de modos IM:

According to Goran Devic Z80 reverse engineering schema, and the conclusions from Simon Owen, Gerton Lunter, Miguel Angel Rodriguez Jodar:

Now, bits 4:3 are renamed as db[1] and db[0] in this schematic. They can be 00, which officially means IM 0. 10 officially means IM 1 and 11 officially means IM 2.

The Z80 has two registers named im1 and im2. Whichever of them is set indicates which interrupt mode is active (IM1 or IM 2). If none of them is set, then it's IM 0. You can see that it is so with the above combinarions:

Combination 0x makes the im1 register to store a 0, as the output from the AND gate will be 0. The same goes for combination 11 because of the inverter before one of the inputs to the AND. In fact, the only combination that makes a 1 to be stored is 10 (which is the official way to encode IM 1)

Combination 11 makes the second register (im2) to store a 1 because of the AND gate. Any other combination will make this register to store a 0. Again, this is consistent with the official way to encode IM 2.

Therefore, combination 00 and 01 won't set any of both registers, and this will be interpreted as IM 0.

The interrupt mode after a reset, as you can see, is IM 0.


Mirando los bits 4 y 3 de opcode

00: IM0
01: IM0
10: IM1
11: IM2

en este caso: 70 = 0100 0110
                      - -
                      0 0 = IM0

*/
	im_mode=0;
}

void instruccion_ed_71 ()
{
//coded71:                        ;LD I,A
      contend_read_no_mreq( IR, 1 );

	reg_i=reg_a;
}

void instruccion_ed_72 ()
{
//IN C,(C)

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

   z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        reg_c=valor_puerto;
	    set_flags_in_reg(reg_c);

    }



}

void instruccion_ed_73 ()
{
//OUT (C),C

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

        out_port(BC, reg_c);

}

void instruccion_ed_74 ()
{
//ADC HL,BC
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

        adc_hl(BC);

}

void instruccion_ed_75 ()
{
//LD BC,(NN)
        z80_int dir;

        dir=lee_word_pc();

        reg_c=peek_byte(dir);
        reg_b=peek_byte(dir+1);

	set_memptr(dir+1);

}

void instruccion_ed_76 ()
{
//NEG*

	neg();
}

void instruccion_ed_77 ()
{
//RETI
	//iff1.v=iff2.v; Reti no hace esto
	reg_pc=pop_valor();
}

void instruccion_ed_78 ()
{
//coded70:                        ;IM 0
/*
en este caso: 78 = 0100 1110
                      - -
                      0 1 = IM0
*/
        im_mode=0;
}

void instruccion_ed_79 ()
{
//coded79:                        ;LD R,A
      contend_read_no_mreq( IR, 1 );

	reg_r=reg_a;
	reg_r_bit7=reg_a & 128;

	if (machine_emulate_memory_refresh) {
		//Emulacion refresco memoria superior en 48kb
		if (machine_emulate_memory_refresh_counter<MAX_EMULATE_MEMORY_REFRESH_COUNTER) {
			machine_emulate_memory_refresh_counter++;
		}

		//Hacer debug de esto de vez en cuando
		if ((machine_emulate_memory_refresh_counter%10000)==0) machine_emulate_memory_refresh_debug_counter();
	}

}

void instruccion_ed_80 ()
{
//IN D,(C)

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

   z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        reg_d=valor_puerto;
	set_flags_in_reg(reg_d);

    }


}

void instruccion_ed_81 ()
{
//OUT (C),D

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

        out_port(BC, reg_d);

}

void instruccion_ed_82 ()
{
//SBC HL,DE
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

	sbc_hl( DE  );
}

void instruccion_ed_83 ()
{

//LD (NN),DE

        z80_int dir;

        dir=lee_word_pc();

        poke_byte(dir,reg_e);
        poke_byte(dir+1,reg_d);

	set_memptr(dir+1);

}

void instruccion_ed_84 ()
{
//NEG*
	neg();
}

void instruccion_ed_85 ()
{
//RETN*
	iff1.v=iff2.v;
	reg_pc=pop_valor();

}

void instruccion_ed_86 ()
{
//coded86:                        ;IM 1
/*
en este caso: 86 = 0101 0110
                      - -
                      1 0 = IM1
*/
	im_mode=1;
}

void instruccion_ed_87 ()
{
//LD A,I
      contend_read_no_mreq( IR, 1 );


        reg_a=reg_i;

	Z80_FLAGS=Z80_FLAGS & FLAG_C;

	Z80_FLAGS |=sz53_table[reg_a];

        if (iff2.v==1) Z80_FLAGS |=FLAG_PV;


}

void instruccion_ed_88 ()
{
//IN E,(C)

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

   z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        reg_e=valor_puerto;
	set_flags_in_reg(reg_e);

    }


}

void instruccion_ed_89 ()
{
//OUT (C),E

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

        out_port(BC, reg_e);

}

void instruccion_ed_90 ()
{
//ADC HL,DE
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

        adc_hl(DE);


}

void instruccion_ed_91 ()
{
//coded91:                        ;LD DE,(NN)

        z80_int dir;

        dir=lee_word_pc();

	reg_e=peek_byte(dir);
	reg_d=peek_byte(dir+1);

	set_memptr(dir+1);


}

void instruccion_ed_92 ()
{
//NEG*
	neg();
}

void instruccion_ed_93 ()
{
//RETN*
	iff1.v=iff2.v;
	reg_pc=pop_valor();

}

void instruccion_ed_94 ()
{
//IM 2
/*
en este caso: 94 = 0101 1110
                      - -
                      1 1 = IM2
*/
	im_mode=2;
}

void instruccion_ed_95 ()
{
//coded95:                        ;LD A,R
      contend_read_no_mreq( IR, 1 );

	reg_a=(reg_r&127) | (reg_r_bit7 &128);


        Z80_FLAGS=Z80_FLAGS & FLAG_C;

        Z80_FLAGS |=sz53_table[reg_a];

	if (iff2.v==1) Z80_FLAGS |=FLAG_PV;

}

void instruccion_ed_96 ()
{
//IN H,(C)

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

   z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        reg_h=valor_puerto;
	set_flags_in_reg(reg_h);

    }


}

void instruccion_ed_97 ()
{
//OUT (C),H

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

        out_port(BC, reg_h);

}

void instruccion_ed_98 ()
{
//coded98:                        ;SBC HL,HL
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );


        sbc_hl( HL  );

}

void instruccion_ed_99 ()
{
//LD (NN),HL

        z80_int dir;
        dir=lee_word_pc();

        poke_byte(dir,reg_l);
        poke_byte(dir+1,reg_h);

	set_memptr(dir+1);

}

void instruccion_ed_100 ()
{
//NEG*
	neg();
}

void instruccion_ed_101 ()
{
//RETN*
	iff1.v=iff2.v;
	reg_pc=pop_valor();

}

void instruccion_ed_102 ()
{
//IM 0
/*
en este caso: 102 = 0110 0110
                       - -
                       0 0 = IM0
*/
        im_mode=0;
}

void instruccion_ed_103 ()
{
//RRD

        z80_byte low_hl,low_hl_copia,high_hl,low_a,high_a;
        z80_byte bytehl;
	z80_int reg_temp_hl;


        reg_temp_hl=HL;
        bytehl=peek_byte(reg_temp_hl);
        contend_read_no_mreq( HL, 1 ); contend_read_no_mreq( HL, 1 );
        contend_read_no_mreq( HL, 1 ); contend_read_no_mreq( HL, 1 );

        low_hl_copia=low_hl=bytehl & 0xF;
        high_hl=(bytehl >> 4) & 0xF;
        low_a=reg_a & 0xF;

        //este sin rotar
        high_a=reg_a & 0xF0;

	low_hl=high_hl;
	high_hl=low_a;
	low_a=low_hl_copia;

        reg_a=high_a | low_a;
        bytehl=(high_hl<<4) | low_hl;

        poke_byte(HL,bytehl);


	//FLAG C no alterado
	Z80_FLAGS = Z80_FLAGS & FLAG_C;

	Z80_FLAGS |=sz53p_table[reg_a];

	//flags. The H and N flags are reset, P/V is parity, C is preserved, and S and Z are modified by definition.
	set_memptr(reg_temp_hl+1);


}

void instruccion_ed_104 ()
{
//IN L,(C)

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

   z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        reg_l=valor_puerto;
	set_flags_in_reg(reg_l);

    }


}

void instruccion_ed_105 ()
{
//OUT (C),L

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

        out_port(BC, reg_l);

}

void instruccion_ed_106 ()
{
//coded106:               ;ADC HL,HL
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

	adc_hl(HL);
}

void instruccion_ed_107 ()
{
//LD HL,(NN)

        z80_int dir;

        dir=lee_word_pc();

        reg_l=peek_byte(dir);
        reg_h=peek_byte(dir+1);

	set_memptr(dir+1);

}

void instruccion_ed_108 ()
{
//NEG*
	neg();
}

void instruccion_ed_109 ()
{
//RETN*
	iff1.v=iff2.v;
	reg_pc=pop_valor();

}

void instruccion_ed_110 ()
{
//IM 0
/*
en este caso: 110 = 0110 1110
                       - -
                       0 1 = IM0
*/
        im_mode=0;
}

void instruccion_ed_111 ()
{

//RLD

	z80_byte low_hl,high_hl,low_a,low_a_copia,high_a;
	z80_byte bytehl;
	z80_int reg_temp_hl;


	reg_temp_hl=HL;
	bytehl=peek_byte(reg_temp_hl);

        contend_read_no_mreq( HL, 1 ); contend_read_no_mreq( HL, 1 );
        contend_read_no_mreq( HL, 1 ); contend_read_no_mreq( HL, 1 );


	low_hl=bytehl & 0xF;
	high_hl=(bytehl >> 4) & 0xF;

	low_a_copia=low_a=reg_a & 0xF;

	//este sin rotar
	high_a=reg_a & 0xF0;

	low_a=high_hl;
	high_hl=low_hl;
	low_hl=low_a_copia;

	reg_a=high_a | low_a;
	bytehl=(high_hl<<4) | low_hl;

	poke_byte(HL,bytehl);

        //FLAG C no alterado
        Z80_FLAGS = Z80_FLAGS & FLAG_C;

        Z80_FLAGS |=sz53p_table[reg_a];


	set_memptr(reg_temp_hl+1);

}

void instruccion_ed_112 ()
{
//IN F,(C). solo afecta flags
	z80_byte result;

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

   z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        result=valor_puerto;
        set_flags_in_reg(result);

    }



}

void instruccion_ed_113 ()
{
//OUT (C),0

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

	//Esto es diferente entre NMOS y CMOS
        if (z80_cpu_current_type==Z80_TYPE_CMOS) {
	//if (MACHINE_IS_Z88 || MACHINE_IS_TSCONF) {
		//CMOS
        //printf("Outing 255 to %XH on pc=%d\n",BC,reg_pc);
		out_port(BC, 255);

	}

	else {
        //printf("Outing 0 to %XH on pc=%d\n",BC,reg_pc);
	        out_port(BC, 0);

	}
}

void instruccion_ed_114 ()
{
//coded114:               ;SBC HL,SP
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

        sbc_hl( reg_sp  );

}

void instruccion_ed_115 ()
{
//coded115:               ;LD (NN),SP


        z80_int dir;

        dir=lee_word_pc();

        poke_word(dir,reg_sp);

	set_memptr(dir+1);

}

void instruccion_ed_116 ()
{
//NEG*
	neg();
}

void instruccion_ed_117 ()
{
//RETN
	iff1.v=iff2.v;
	reg_pc=pop_valor();

}


void instruccion_ed_118 ()
{
//IM 1
/*
en este caso: 118 = 0111 0110
                       - -
                       1 0 = IM1
*/
        im_mode=1;
}

void instruccion_ed_119 ()
{
	invalid_opcode_ed("237 119");
}

void instruccion_ed_120 ()
{

//IN A,(C)

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif

   z80_byte valor_puerto=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        reg_a=valor_puerto;
	set_flags_in_reg(reg_a);

    }



}

void instruccion_ed_121 ()
{
//OUT (C),A

#ifdef EMULATE_MEMPTR
        set_memptr(BC+1);
#endif
        out_port(BC, reg_a);

}

void instruccion_ed_122 ()
{
//coded122:               ;ADC HL,SP
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );
      contend_read_no_mreq( IR, 1 );

	adc_hl(reg_sp);

}

void instruccion_ed_123 ()
{
//coded123:               ;LD SP,(NN)
        z80_int dir;


        dir=lee_word_pc();

        reg_sp=peek_word(dir);

	set_memptr(dir+1);


}

void instruccion_ed_124 ()
{
//NEG*
	neg();
}

void instruccion_ed_125 ()
{
//RETN*
	iff1.v=iff2.v;
	reg_pc=pop_valor();

}

void instruccion_ed_126 ()
{
//IM 2
/*
en este caso: 126 = 0111 1110
                       - -
                       1 1 = IM2
*/
        im_mode=2;
}

void instruccion_ed_127 ()
{
        invalid_opcode_ed("237 127");
}

void instruccion_ed_128 ()
{
        invalid_opcode_ed("237 128");
}

void instruccion_ed_129 ()
{
        invalid_opcode_ed("237 129");
}

void instruccion_ed_130 ()
{
        invalid_opcode_ed("237 130");
}

void instruccion_ed_131 ()
{
        invalid_opcode_ed("237 131");
}

void instruccion_ed_132 ()
{
        invalid_opcode_ed("237 132");
}

void instruccion_ed_133 ()
{
        invalid_opcode_ed("237 133");
}

void instruccion_ed_134 ()
{
        invalid_opcode_ed("237 134");
}

void instruccion_ed_135 ()
{
        invalid_opcode_ed("237 135");
}

void instruccion_ed_136 ()
{
        invalid_opcode_ed("237 136");
}

void instruccion_ed_137 ()
{
        invalid_opcode_ed("237 137");
}

void instruccion_ed_138 ()
{
        if (MACHINE_IS_TBBLUE) {
                //push NN        ED 8A HI LO   4+4+3+3+3+3 (3+3 for read, 3+3 for write)  push 16bit immediate value. big endian
                z80_int value=0;
                value |= (lee_byte_pc()<<8);
                value |= lee_byte_pc();
                push_valor( value , PUSH_VALUE_TYPE_PUSH);
        }

        else invalid_opcode_ed("237 138");
}

void instruccion_ed_139 ()
{

        invalid_opcode_ed("237 139");
}

void instruccion_ed_140 ()
{
        invalid_opcode_ed("237 140");
}

void instruccion_ed_141 ()
{
        invalid_opcode_ed("237 141");
}

void instruccion_ed_142 ()
{
        invalid_opcode_ed("237 142");
}

void instruccion_ed_143 ()
{
        invalid_opcode_ed("237 143");
}

void instruccion_ed_144 ()
{
        if (MACHINE_IS_TBBLUE) {
                //ED 90  OUTINB As OUTI but B is not incremented
	z80_byte value,aux;

        contend_read_no_mreq( IR, 1 );


        value=peek_byte(HL);


        //reg_b--;

#ifdef EMULATE_MEMPTR
        set_memptr( BC+1  );
#endif
        out_port(BC,value);

        HL++;

        aux=value+reg_l;
        if (value & 0x80) Z80_FLAGS |=FLAG_N;
        else Z80_FLAGS &=(255-FLAG_N);

        if (aux<value) {
                Z80_FLAGS |=FLAG_H|FLAG_C;
        }
        else {
                Z80_FLAGS &=(255-FLAG_H-FLAG_C);
        }

        Z80_FLAGS = (Z80_FLAGS & (255-FLAG_S-FLAG_Z-FLAG_3-FLAG_5) ) | sz53_table[reg_b];

        set_flags_parity(( aux & 0x07 ) ^ reg_b);

        }
        else invalid_opcode_ed("237 144");
}

void instruccion_ed_145 ()
{
	if (MACHINE_IS_TBBLUE) {
                //nextreg reg,val   ED 91 reg,val   Set a NEXT register (like doing out($243b),reg then out($253b),val )
		z80_byte registro=lee_byte_pc();
		z80_byte valor=lee_byte_pc();

		//tbblue_set_register_port(registro);
		//tbblue_set_value_port(valor);

                tbblue_set_value_port_position(registro,valor);
                //if (puerto==TBBLUE_REGISTER_PORT) tbblue_set_register_port(value);
                //if (puerto==TBBLUE_VALUE_PORT) tbblue_set_value_port(value);
        }
        else invalid_opcode_ed("237 145");
}

void instruccion_ed_146 ()
{
	if (MACHINE_IS_TBBLUE) {
                //nextreg reg,a     ED 92 reg       Set a NEXT register using A (like doing out($243b),reg then out($253b),A )
		z80_byte registro=lee_byte_pc();

                //tbblue_set_register_port(registro);
                //tbblue_set_value_port(reg_a);
                tbblue_set_value_port_position(registro,reg_a);
        }
        else invalid_opcode_ed("237 146");
}

void instruccion_ed_147 ()
{
        if (MACHINE_IS_TBBLUE) {
		//pixeldn ED 93	Moves HL down one line in the ULA screen
		//printf ("pixeldn before. hl=%d\n",reg_hl);
		int x,y;
		x=y=0;


                //Hay que preservar bits 7,6,5 de H
                z80_byte mascara_offset=reg_h & (128+64+32);

                //Quitar bits 7,6,5 (+32768, +16384, y +8192)
                //reg_h &=(255-128-64-32);

                //Esta funcion no tiene en cuenta los bits altos 15,14,13
		util_spectrumscreen_get_xy(reg_hl,&x,&y);
		//printf ("x: %d y: %d\n",x,y);

		y=y+1;
		if (y>191) y=0;

		//x sale en coordenadas de pixel
		x /=8;

                //z80_int resultado=16384+screen_addr_table[y*32+x];
                z80_int resultado=screen_addr_table[y*32+x];

                reg_h=value_16_to_8h(resultado) | mascara_offset;
                reg_l=value_16_to_8l(resultado);

		//printf ("pixeldn after. hl=%d. mascara: %02XH\n",reg_hl,mascara_offset);

        }
        else invalid_opcode_ed("237 147");
}

void instruccion_ed_148 ()
{
	if (MACHINE_IS_TBBLUE) {
        	//pixelad ED 94	Using D as Y, and E as X, work out address of ULA screen address and store in HL
		//screen_addr_table
		int x=reg_e;
		int y=reg_d;
		//printf ("pixelad x: %d y: %d\n",x,y);

		//x esta en coordenadas de pixel
		x /=8;

		z80_int resultado=16384+screen_addr_table[y*32+x];

		reg_h=value_16_to_8h(resultado);
		reg_l=value_16_to_8l(resultado);
		//printf ("pixelad after: hl=%d\n",reg_hl);
        }
        else invalid_opcode_ed("237 148");
}

void instruccion_ed_149 ()
{
        if (MACHINE_IS_TBBLUE) {
                //ED 95   DE holding the Y,X. Bits 0-2 of X (E) map to BIT 7 to 0 and store in A
                //SETAE : A = 2^(7 - E&0x7)  The bottom 3 bits of E is a bit number and 7-that_bit is the bit set in A.
                z80_byte numero_bit=reg_e & 7;
                z80_byte resultado=128;
                if (numero_bit>0) resultado=resultado>>numero_bit;
                reg_a=resultado;
        }
        else invalid_opcode_ed("237 149");
}

void instruccion_ed_150 ()
{
        invalid_opcode_ed("237 150");
}

void instruccion_ed_151 ()
{
        invalid_opcode_ed("237 151");
}

void instruccion_ed_152 ()
{
    if (!MACHINE_IS_TBBLUE) {
        invalid_opcode_ed("237 152");
        return;
    }

    //JP (C)   ED 98: PC = PC&$C000 + IN(C)<<6, PC is "next-instruction" aka "$+2", no flags
    // Jumps at one of 256 subroutines (64B long in 16kiB memory segment), selected by "IN (C)" value

    // IN (C) part
    z80_int in_valor = (z80_int)lee_puerto(reg_b,reg_c);    // read + extend to 16b

        //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

    // combine it into new PC, keeping two top bits from current PC (pointing at next instruction)
    in_valor <<= 6;
    reg_pc = (reg_pc&0xC000) | in_valor;
#ifdef EMULATE_MEMPTR
    set_memptr(reg_pc);     // not sure how this actually works, needs Cesar review
#endif
    }
}

void instruccion_ed_153 ()
{
        invalid_opcode_ed("237 153");
}

void instruccion_ed_154 ()
{
        invalid_opcode_ed("237 154");
}

void instruccion_ed_155 ()
{
        invalid_opcode_ed("237 155");
}

void instruccion_ed_156 ()
{
        invalid_opcode_ed("237 156");
}

void instruccion_ed_157 ()
{
        invalid_opcode_ed("237 157");
}

void instruccion_ed_158 ()
{
        invalid_opcode_ed("237 158");
}


void instruccion_ed_159 ()
{
        invalid_opcode_ed("237 159");
}

void instruccion_ed_160 ()
{

//LDI

/*
LDD         --0*0-  Load and Decrement    [DE]=[HL],HL=HL-1,#
LDDR        --000-  Load, Dec., Repeat    LDD till BC=0
LDI         --0*0-  Load and Increment    [DE]=[HL],HL=HL+1,#
LDIR        --000-  Load, Inc., Repeat    LDI till BC=0
*/

	z80_byte byte_leido;

	byte_leido=peek_byte(HL);
	poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 ); contend_write_no_mreq( DE, 1 );

	HL++;
	DE++;
	BC--;

	Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

	if (BC) Z80_FLAGS |=FLAG_PV;

	if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

	if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;


}

void instruccion_ed_161 ()
{
//coded161:               ;CPI

	cpi_cpd_common();

        HL++;

	set_memptr(memptr+1);

}

void instruccion_ed_162 ()
{
//coded162:               ;INI
        z80_byte value,aux;

        contend_read_no_mreq( IR, 1 );


        value=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        poke_byte(HL,value);
#ifdef EMULATE_MEMPTR
	set_memptr( BC+1  );
#endif
        reg_b--;
	HL++;

	aux=value+reg_c+1;
	if (value & 0x80) Z80_FLAGS |=FLAG_N;
	else Z80_FLAGS &=(255-FLAG_N);

	if (aux<value) {
		Z80_FLAGS |=FLAG_H|FLAG_C;
	}
	else {
		Z80_FLAGS &=(255-FLAG_H-FLAG_C);
	}

	Z80_FLAGS = (Z80_FLAGS & (255-FLAG_S-FLAG_Z-FLAG_3-FLAG_5) ) | sz53_table[reg_b];

	set_flags_parity(( aux & 0x07 ) ^ reg_b);
    }

}

void instruccion_ed_163 ()
{
//coded163:               ;OUTI
	z80_byte value,aux;

        contend_read_no_mreq( IR, 1 );


        value=peek_byte(HL);

        z80_last_data_transferred_ot_in=value;


        reg_b--;

#ifdef EMULATE_MEMPTR
        set_memptr( BC+1  );
#endif
        out_port(BC,value);

        HL++;

        aux=value+reg_l;
        if (value & 0x80) Z80_FLAGS |=FLAG_N;
        else Z80_FLAGS &=(255-FLAG_N);

        if (aux<value) {
                Z80_FLAGS |=FLAG_H|FLAG_C;
        }
        else {
                Z80_FLAGS &=(255-FLAG_H-FLAG_C);
        }

        Z80_FLAGS = (Z80_FLAGS & (255-FLAG_S-FLAG_Z-FLAG_3-FLAG_5) ) | sz53_table[reg_b];

        set_flags_parity(( aux & 0x07 ) ^ reg_b);

}

void instruccion_ed_164 ()
{
        if (MACHINE_IS_TBBLUE) {
                //LDIX    ED A4 As LDI, if byte == A then skips byte.
        //LDI

/*
LDD         --0*0-  Load and Decrement    [DE]=[HL],HL=HL-1,#
LDDR        --000-  Load, Dec., Repeat    LDD till BC=0
LDI         --0*0-  Load and Increment    [DE]=[HL],HL=HL+1,#
LDIR        --000-  Load, Inc., Repeat    LDI till BC=0
*/

	z80_byte byte_leido;

	byte_leido=peek_byte(HL);
        //As LDI, if byte == A then skips byte.
	if (reg_a!=byte_leido) poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 ); contend_write_no_mreq( DE, 1 );

	HL++;
	DE++;
	BC--;

	Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

	if (BC) Z80_FLAGS |=FLAG_PV;

	if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

	if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;
        }
        else invalid_opcode_ed("237 164");
}

void instruccion_ed_165 ()
{
	//Leer ed y opcode: 4+4=8 T estados

	if (MACHINE_IS_TBBLUE) {
		//LDWS (LoaD Wasp Special) (0xED 0xA5), (DE) = (HL) : D++ : L++ 14Ts. BC is not modified. Flags are set as if an INC D instruction was executed
		z80_byte byte_leido;

		byte_leido=peek_byte(HL);  //3 T
		poke_byte(DE,byte_leido);  //3 T

		reg_l++;

		inc_8bit(reg_d);
	}
	//Total T-estados = 8+3+3=14

        else invalid_opcode_ed("237 165");


}

void instruccion_ed_166 ()
{
        invalid_opcode_ed("237 166");
}

void instruccion_ed_167 ()
{
        invalid_opcode_ed("237 167");
}

void instruccion_ed_168 ()
{
//LDD

	z80_byte byte_leido;

	byte_leido=peek_byte(HL);
        poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 ); contend_write_no_mreq( DE, 1 );

        HL--;
        DE--;

        BC--;


        Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

        if (BC) Z80_FLAGS |=FLAG_PV;

        if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

        if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;



}

void instruccion_ed_169 ()
{
//coded169:               ;CPD

        cpi_cpd_common();

        HL--;

	set_memptr(memptr-1);


}

void instruccion_ed_170 ()
{
//coded170:               ;IND
        z80_byte value,aux;

        contend_read_no_mreq( IR, 1 );


        value=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        poke_byte(HL,value);
#ifdef EMULATE_MEMPTR
        set_memptr( BC-1  );
#endif

        reg_b--;
        HL--;

        aux=value+reg_c-1;
        if (value & 0x80) Z80_FLAGS |=FLAG_N;
        else Z80_FLAGS &=(255-FLAG_N);

        if (aux<value) {
                Z80_FLAGS |=FLAG_H|FLAG_C;
        }
        else {
                Z80_FLAGS &=(255-FLAG_H-FLAG_C);
        }

        Z80_FLAGS = (Z80_FLAGS & (255-FLAG_S-FLAG_Z-FLAG_3-FLAG_5) ) | sz53_table[reg_b];

        set_flags_parity(( aux & 0x07 ) ^ reg_b);

    }

}

void instruccion_ed_171 ()
{
//coded171:               ;OUTD
        z80_byte value,aux;

        contend_read_no_mreq( IR, 1 );


        value=peek_byte(HL);

        z80_last_data_transferred_ot_in=value;

        reg_b--;

#ifdef EMULATE_MEMPTR
        set_memptr( BC-1  );
#endif
        out_port(BC,value);

        HL--;

        aux=value+reg_l;
        if (value & 0x80) Z80_FLAGS |=FLAG_N;
        else Z80_FLAGS &=(255-FLAG_N);

        if (aux<value) {
                Z80_FLAGS |=FLAG_H|FLAG_C;
        }
        else {
                Z80_FLAGS &=(255-FLAG_H-FLAG_C);
        }

        Z80_FLAGS = (Z80_FLAGS & (255-FLAG_S-FLAG_Z-FLAG_3-FLAG_5) ) | sz53_table[reg_b];

        set_flags_parity(( aux & 0x07 ) ^ reg_b);



}

void instruccion_ed_172 ()
{
        if (MACHINE_IS_TBBLUE) {
                //LDDX    ED AC   As LDD, except DE++ and if byte == A then skips byte.
	z80_byte byte_leido;

	byte_leido=peek_byte(HL);
        if (reg_a!=byte_leido) poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 ); contend_write_no_mreq( DE, 1 );

        HL--;
        DE++;

        BC--;


        Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

        if (BC) Z80_FLAGS |=FLAG_PV;

        if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

        if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;

        }
        else invalid_opcode_ed("237 172");
}

void instruccion_ed_173 ()
{
        invalid_opcode_ed("237 173");
}

void instruccion_ed_174 ()
{
        invalid_opcode_ed("237 174");
}

void instruccion_ed_175 ()
{
        invalid_opcode_ed("237 175");
}

void instruccion_ed_176_optimized ()
{

//LDIR optimized
//printf ("LDIR optimized origen %d destino %d long %d\n",HL,DE,BC);
        if (0 /*MACHINE_IS_SPECTRUM_48*/) {

                //memcpy a lo bestia. lastima: esto no parece llevarse bien con cosas como:
                //LDIR optimized origen 22208 destino 22209 long 63
                //que hace el basic, porque se solapan
                memcpy(&memoria_spectrum[DE],&memoria_spectrum[HL],BC);
                HL +=BC;
                DE +=BC;
                BC = 0;
        }

        else {

                z80_byte byte_leido;

                do {

                        byte_leido=peek_byte_no_time(HL);
                        poke_byte_no_time(DE,byte_leido);

                        HL++; DE++;
                        BC--;
                } while (BC!=0);

        }

        Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

}



void instruccion_ed_176 ()
{

//LDIR

        if (cpu_ldir_lddr_hack_optimized.v) {

                instruccion_ed_176_optimized();

                return;
        }

#ifdef EMULATE_MEMPTR
        if (reg_b!=0 || reg_c!=1) set_memptr(reg_pc-1);
#endif

	//instruccion_ed_160();

        z80_byte byte_leido;

        byte_leido=peek_byte(HL);
        poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 );
	contend_write_no_mreq( DE, 1 );

	BC--;

        Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

        if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

        if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;

        if (BC) {
	  Z80_FLAGS |=FLAG_PV;
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
          reg_pc -=2;

          z80_ejecutada_instruccion_bloque_ld_cp=1;
        }
        HL++; DE++;


}


void instruccion_ed_177 ()
{
//coded177:               ;CPIR


        cpi_cpd_common();


        set_memptr(memptr+1);


        if ((Z80_FLAGS & FLAG_PV)==0 || (Z80_FLAGS & FLAG_Z)) {
		HL++;
		return;
	}

          contend_read_no_mreq( HL, 1 );
	  contend_read_no_mreq( HL, 1 );
          contend_read_no_mreq( HL, 1 );
	  contend_read_no_mreq( HL, 1 );
          contend_read_no_mreq( HL, 1 );

#ifdef EMULATE_MEMPTR
        set_memptr(reg_pc-1);
#endif

        reg_pc -=2;

        z80_ejecutada_instruccion_bloque_ld_cp=1;
	HL++;

}


void instruccion_ed_178 ()
{
//coded178:               ;INIR
        //instruccion_ed_162();

        z80_byte value,aux;

        contend_read_no_mreq( IR, 1 );


        value=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {


        z80_last_data_transferred_ot_in=value;
        poke_byte(HL,value);
#ifdef EMULATE_MEMPTR
        set_memptr( BC+1  );
#endif
        reg_b--;

        aux=value+reg_c+1;
        if (value & 0x80) Z80_FLAGS |=FLAG_N;
        else Z80_FLAGS &=(255-FLAG_N);

        if (aux<value) {
                Z80_FLAGS |=FLAG_H|FLAG_C;
        }
        else {
                Z80_FLAGS &=(255-FLAG_H-FLAG_C);
        }

        Z80_FLAGS = (Z80_FLAGS & (255-FLAG_S-FLAG_Z-FLAG_3-FLAG_5) ) | sz53_table[reg_b];

        set_flags_parity(( aux & 0x07 ) ^ reg_b);

        if ( reg_b ) {
          contend_write_no_mreq( HL, 1 );
	  contend_write_no_mreq( HL, 1 );
          contend_write_no_mreq( HL, 1 );
	  contend_write_no_mreq( HL, 1 );
          contend_write_no_mreq( HL, 1 );
          reg_pc -= 2;

          z80_ejecutada_instruccion_bloque_ot_in=1;
        }


        HL++;

    }

}


void instruccion_ed_179 ()
{
//coded179:               ;OTIR
        instruccion_ed_163();

        if ( reg_b ) {
          contend_read_no_mreq( BC, 1 );
	  contend_read_no_mreq( BC, 1 );
          contend_read_no_mreq( BC, 1 );
	  contend_read_no_mreq( BC, 1 );
          contend_read_no_mreq( BC, 1 );
          reg_pc -= 2;

          z80_ejecutada_instruccion_bloque_ot_in=1;
        }


}


void instruccion_ed_180 ()
{
        if (MACHINE_IS_TBBLUE) {
                //LDIRX ED B4     As LDIR, if byte == A then skips byte.

//Como instruccion ldir excepto el poke....
#ifdef EMULATE_MEMPTR
        if (reg_b!=0 || reg_c!=1) set_memptr(reg_pc-1);
#endif

	//instruccion_ed_160();

        z80_byte byte_leido;

        byte_leido=peek_byte(HL);
        //LDIRX ED B4     As LDIR, if byte == A then skips byte.
        if (reg_a!=byte_leido) poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 );
	contend_write_no_mreq( DE, 1 );

	BC--;

        Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

        if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

        if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;

        if (BC) {
	  Z80_FLAGS |=FLAG_PV;
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
          reg_pc -=2;
        }
        HL++; DE++;


        }
        else invalid_opcode_ed("237 180");
}

void instruccion_ed_181 ()
{
        invalid_opcode_ed("237 181");
}

void instruccion_ed_182 ()
{
        if (MACHINE_IS_TBBLUE) {
                //LDIRSCALE ED B6 as LDIR but 24 bit source pointer HLA' takes high 16 bits as address
                //??? no entiendo que hace

                invalid_opcode_ed("Unimplemented tbblue LDIRSCALE");

        }

        else invalid_opcode_ed("237 182");
}

void instruccion_ed_183 ()
{
    if (!MACHINE_IS_TBBLUE) {
        invalid_opcode_ed("237 183");
        return;
    }

    //LDPIRX ED B7: do{t:=(HL&$FFF8+E&7)*; {if t!=A DE*:=t;} DE++; BC--}while(BC>0)
    //LDPIRX is similar to LDIRX, but it's for 8 byte pattern fills
    //basically the lower 3 bits of E are put into lower 3 bits of L, (but real HL is not modified!)

    z80_int source_adr = (reg_hl & ~7) | (reg_e & 7);

//Como instruccion ldirx excepto el HL incrementar...
#ifdef EMULATE_MEMPTR
    if (reg_b!=0 || reg_c!=1) set_memptr(reg_pc-1);
#endif

    z80_byte byte_leido = peek_byte(source_adr);
    //if byte == A then skips byte.
    if (reg_a!=byte_leido) poke_byte(DE,byte_leido);

    contend_write_no_mreq( DE, 1 );
    contend_write_no_mreq( DE, 1 );

    BC--;
    if (BC) {
        contend_write_no_mreq( DE, 1 );
        contend_write_no_mreq( DE, 1 );
        contend_write_no_mreq( DE, 1 );
        contend_write_no_mreq( DE, 1 );
        contend_write_no_mreq( DE, 1 );
        reg_pc -=2;
    }

    DE++;

    //LDPIRX does not affect flags
}


void instruccion_ed_184_optimized ()
{

//LDDR optimized
//printf ("LDDR optimized origen %d destino %d long %d\n",HL,DE,BC);


        z80_byte byte_leido;

        do {

                byte_leido=peek_byte_no_time(HL);
                poke_byte_no_time(DE,byte_leido);

                HL--; DE--;
                BC--;
        } while (BC!=0);



        Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);

}




void instruccion_ed_184 ()
{
//LDDR

        if (cpu_ldir_lddr_hack_optimized.v) {

                instruccion_ed_184_optimized();

                return;
        }
#ifdef EMULATE_MEMPTR
        if (reg_b!=0 || reg_c!=1) set_memptr(reg_pc-1);
#endif

        //instruccion_ed_168();

        z80_byte byte_leido;

        byte_leido=peek_byte(HL);
        poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 );
	contend_write_no_mreq( DE, 1 );

        BC--;

	Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);


        if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

        if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;

        if (BC) {
	  Z80_FLAGS |=FLAG_PV;
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
          reg_pc -=2;

          z80_ejecutada_instruccion_bloque_ld_cp=1;
        }
        HL--; DE--;


}


void instruccion_ed_185 ()
{
//coded177:               ;CPDR


        cpi_cpd_common();


        set_memptr(memptr-1);


        if ((Z80_FLAGS & FLAG_PV)==0 || (Z80_FLAGS & FLAG_Z)) {
		HL--;
		return;
	}

          contend_read_no_mreq( HL, 1 );
	  contend_read_no_mreq( HL, 1 );
          contend_read_no_mreq( HL, 1 );
	  contend_read_no_mreq( HL, 1 );
          contend_read_no_mreq( HL, 1 );

#ifdef EMULATE_MEMPTR
        set_memptr(reg_pc-1);
#endif

        reg_pc -=2;

        z80_ejecutada_instruccion_bloque_ld_cp=1;
	HL--;

}


void instruccion_ed_186 ()
{
//coded186:               ;INDR

        z80_byte value,aux;

        contend_read_no_mreq( IR, 1 );


        value=lee_puerto(reg_b,reg_c);

    //Si se ha activado wait
    if (z80_wait_signal.v) {
        //Retroceder pc
        reg_pc--;
        reg_pc--;
    }

    else {

        z80_last_data_transferred_ot_in=value;
        poke_byte(HL,value);
#ifdef EMULATE_MEMPTR
        set_memptr( BC-1  );
#endif

        reg_b--;

        aux=value+reg_c-1;
        if (value & 0x80) Z80_FLAGS |=FLAG_N;
        else Z80_FLAGS &=(255-FLAG_N);

        if (aux<value) {
                Z80_FLAGS |=FLAG_H|FLAG_C;
        }
        else {
                Z80_FLAGS &=(255-FLAG_H-FLAG_C);
        }

        Z80_FLAGS = (Z80_FLAGS & (255-FLAG_S-FLAG_Z-FLAG_3-FLAG_5) ) | sz53_table[reg_b];

        set_flags_parity(( aux & 0x07 ) ^ reg_b);

        if (reg_b) {
          contend_write_no_mreq( HL, 1 );
	  contend_write_no_mreq( HL, 1 );
          contend_write_no_mreq( HL, 1 );
	  contend_write_no_mreq( HL, 1 );
          contend_write_no_mreq( HL, 1 );
          reg_pc -= 2;

          z80_ejecutada_instruccion_bloque_ot_in=1;
        }

        HL--;
    }

}


void instruccion_ed_187 ()
{
//coded187:               ;OTDR
        instruccion_ed_171();

        if ( reg_b ) {
          contend_read_no_mreq( BC, 1 );
	  contend_read_no_mreq( BC, 1 );
          contend_read_no_mreq( BC, 1 );
	  contend_read_no_mreq( BC, 1 );
          contend_read_no_mreq( BC, 1 );
          reg_pc -= 2;

          z80_ejecutada_instruccion_bloque_ot_in=1;
        }

}


void instruccion_ed_188 ()
{
        if (MACHINE_IS_TBBLUE) {
                //LDDRX   ED BC    As LDDR, except DE++ and if byte == A then skips byte.

#ifdef EMULATE_MEMPTR
        if (reg_b!=0 || reg_c!=1) set_memptr(reg_pc-1);
#endif

        //instruccion_ed_168();

        z80_byte byte_leido;

        byte_leido=peek_byte(HL);
        if (reg_a!=byte_leido) poke_byte(DE,byte_leido);

        contend_write_no_mreq( DE, 1 );
	contend_write_no_mreq( DE, 1 );

        BC--;

	Z80_FLAGS &=(255-FLAG_H-FLAG_N-FLAG_PV-FLAG_3-FLAG_5);


        if (byte_leido & 8 ) Z80_FLAGS |=FLAG_3;

        if (byte_leido & 2 ) Z80_FLAGS |=FLAG_5;

        if (BC) {
	  Z80_FLAGS |=FLAG_PV;
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
	  contend_write_no_mreq( DE, 1 );
          contend_write_no_mreq( DE, 1 );
          reg_pc -=2;
        }
        HL--; DE++;


        }
        else invalid_opcode_ed("237 188");
}

void instruccion_ed_189 ()
{
        invalid_opcode_ed("237 189");
}

void instruccion_ed_190 ()
{
        invalid_opcode_ed("237 190");
}

void instruccion_ed_191 ()
{
        invalid_opcode_ed("237 191");
}

void instruccion_ed_192 ()
{
        invalid_opcode_ed("237 192");
}

void instruccion_ed_193 ()
{
        invalid_opcode_ed("237 193");
}

void instruccion_ed_194 ()
{
        invalid_opcode_ed("237 194");
}

void instruccion_ed_195 ()
{
        invalid_opcode_ed("237 195");
}

void instruccion_ed_196 ()
{
        invalid_opcode_ed("237 196");
}

void instruccion_ed_197 ()
{
        invalid_opcode_ed("237 197");
}

void instruccion_ed_198 ()
{
        invalid_opcode_ed("237 198");
}

void instruccion_ed_199 ()
{
        invalid_opcode_ed("237 199");
}

void instruccion_ed_200 ()
{
        invalid_opcode_ed("237 200");
}

void instruccion_ed_201 ()
{
        invalid_opcode_ed("237 201");
}

void instruccion_ed_202 ()
{
        invalid_opcode_ed("237 202");
}

void instruccion_ed_203 ()
{
        invalid_opcode_ed("237 203");
}

void instruccion_ed_204 ()
{
        invalid_opcode_ed("237 204");
}

void instruccion_ed_205 ()
{
        invalid_opcode_ed("237 205");
}

void instruccion_ed_206 ()
{
        invalid_opcode_ed("237 206");
}

void instruccion_ed_207 ()
{
        invalid_opcode_ed("237 207");
}

void instruccion_ed_208 ()
{
        invalid_opcode_ed("237 208");
}

void instruccion_ed_209 ()
{
        invalid_opcode_ed("237 209");
}

void instruccion_ed_210 ()
{
        invalid_opcode_ed("237 210");
}

void instruccion_ed_211 ()
{
        invalid_opcode_ed("237 211");
}

void instruccion_ed_212 ()
{
        invalid_opcode_ed("237 212");
}

void instruccion_ed_213 ()
{
        invalid_opcode_ed("237 213");
}

void instruccion_ed_214 ()
{
        invalid_opcode_ed("237 214");
}

void instruccion_ed_215 ()
{
        invalid_opcode_ed("237 215");
}

void instruccion_ed_216 ()
{
        invalid_opcode_ed("237 216");
}

void instruccion_ed_217 ()
{
        invalid_opcode_ed("237 217");
}

void instruccion_ed_218 ()
{
        invalid_opcode_ed("237 218");
}

void instruccion_ed_219 ()
{
        invalid_opcode_ed("237 219");
}

void instruccion_ed_220 ()
{
        invalid_opcode_ed("237 220");
}

void instruccion_ed_221 ()
{
        invalid_opcode_ed("237 221");
}

void instruccion_ed_222 ()
{
        invalid_opcode_ed("237 222");
}

void instruccion_ed_223 ()
{
        invalid_opcode_ed("237 223");
}

void instruccion_ed_224 ()
{
        invalid_opcode_ed("237 224");
}

void instruccion_ed_225 ()
{
        invalid_opcode_ed("237 225");
}

void instruccion_ed_226 ()
{
        invalid_opcode_ed("237 226");
}

void instruccion_ed_227 ()
{
        invalid_opcode_ed("237 227");
}

void instruccion_ed_228 ()
{
        invalid_opcode_ed("237 228");
}

void instruccion_ed_229 ()
{
        invalid_opcode_ed("237 229");
}

void instruccion_ed_230 ()
{
        invalid_opcode_ed("237 230");
}

void instruccion_ed_231 ()
{
        invalid_opcode_ed("237 231");
}

void instruccion_ed_232 ()
{
        invalid_opcode_ed("237 232");
}

void instruccion_ed_233 ()
{
        invalid_opcode_ed("237 233");
}

void instruccion_ed_234 ()
{
        invalid_opcode_ed("237 234");
}

void instruccion_ed_235 ()
{
        invalid_opcode_ed("237 235");
}

void instruccion_ed_236 ()
{
        invalid_opcode_ed("237 236");
}

void instruccion_ed_237 ()
{
        invalid_opcode_ed("237 237");
}

void instruccion_ed_238 ()
{
        invalid_opcode_ed("237 238");
}

void instruccion_ed_239 ()
{
        invalid_opcode_ed("237 239");
}

void instruccion_ed_240 ()
{
        invalid_opcode_ed("237 240");
}

void instruccion_ed_241 ()
{
        invalid_opcode_ed("237 241");
}

void instruccion_ed_242 ()
{
        invalid_opcode_ed("237 242");
}

void instruccion_ed_243 ()
{
        invalid_opcode_ed("237 243");
}

void instruccion_ed_244 ()
{
        invalid_opcode_ed("237 244");
}

void instruccion_ed_245 ()
{
        invalid_opcode_ed("237 245");
}

void instruccion_ed_246 ()
{
        invalid_opcode_ed("237 246");
}

void instruccion_ed_247 ()
{
        invalid_opcode_ed("237 247");
}

void instruccion_ed_248 ()
{
        invalid_opcode_ed("237 248");
}

void instruccion_ed_249 ()
{
    if (samram_enabled.v) {
        samram_opcode_edf9();
    }
    else invalid_opcode_ed("237 249");
}

void instruccion_ed_250 ()
{
    if (samram_enabled.v) {
        samram_opcode_edfa();
    }
    else invalid_opcode_ed("237 250");
}

void instruccion_ed_251 ()
{
    if (samram_enabled.v) {
        samram_opcode_edfb();
    }
    else invalid_opcode_ed("237 251");
}

void instruccion_ed_252 ()
{
        invalid_opcode_ed("237 252");
}

void instruccion_ed_253 ()
{
        invalid_opcode_ed("237 253");
}

void instruccion_ed_254 ()
{
    if (samram_enabled.v) {
        samram_opcode_edfe();
    }
    else invalid_opcode_ed("237 254");
}

void instruccion_ed_255 ()
{
        invalid_opcode_ed("237 255");
}



void (*codpred[]) ()   = {
instruccion_ed_0,
instruccion_ed_1,
instruccion_ed_2,
instruccion_ed_3,
instruccion_ed_4,
instruccion_ed_5,
instruccion_ed_6,
instruccion_ed_7,
instruccion_ed_8,
instruccion_ed_9,
instruccion_ed_10,
instruccion_ed_11,
instruccion_ed_12,
instruccion_ed_13,
instruccion_ed_14,
instruccion_ed_15,
instruccion_ed_16,
instruccion_ed_17,
instruccion_ed_18,
instruccion_ed_19,
instruccion_ed_20,
instruccion_ed_21,
instruccion_ed_22,
instruccion_ed_23,
instruccion_ed_24,
instruccion_ed_25,
instruccion_ed_26,
instruccion_ed_27,
instruccion_ed_28,
instruccion_ed_29,
instruccion_ed_30,
instruccion_ed_31,
instruccion_ed_32,
instruccion_ed_33,
instruccion_ed_34,
instruccion_ed_35,
instruccion_ed_36,
instruccion_ed_37,
instruccion_ed_38,
instruccion_ed_39,
instruccion_ed_40,
instruccion_ed_41,
instruccion_ed_42,
instruccion_ed_43,
instruccion_ed_44,
instruccion_ed_45,
instruccion_ed_46,
instruccion_ed_47,
instruccion_ed_48,
instruccion_ed_49,
instruccion_ed_50,
instruccion_ed_51,
instruccion_ed_52,
instruccion_ed_53,
instruccion_ed_54,
instruccion_ed_55,
instruccion_ed_56,
instruccion_ed_57,
instruccion_ed_58,
instruccion_ed_59,
instruccion_ed_60,
instruccion_ed_61,
instruccion_ed_62,
instruccion_ed_63,
instruccion_ed_64,
instruccion_ed_65,
instruccion_ed_66,
instruccion_ed_67,
instruccion_ed_68,
instruccion_ed_69,
instruccion_ed_70,
instruccion_ed_71,
instruccion_ed_72,
instruccion_ed_73,
instruccion_ed_74,
instruccion_ed_75,
instruccion_ed_76,
instruccion_ed_77,
instruccion_ed_78,
instruccion_ed_79,
instruccion_ed_80,
instruccion_ed_81,
instruccion_ed_82,
instruccion_ed_83,
instruccion_ed_84,
instruccion_ed_85,
instruccion_ed_86,
instruccion_ed_87,
instruccion_ed_88,
instruccion_ed_89,
instruccion_ed_90,
instruccion_ed_91,
instruccion_ed_92,
instruccion_ed_93,
instruccion_ed_94,
instruccion_ed_95,
instruccion_ed_96,
instruccion_ed_97,
instruccion_ed_98,
instruccion_ed_99,
instruccion_ed_100,
instruccion_ed_101,
instruccion_ed_102,
instruccion_ed_103,
instruccion_ed_104,
instruccion_ed_105,
instruccion_ed_106,
instruccion_ed_107,
instruccion_ed_108,
instruccion_ed_109,
instruccion_ed_110,
instruccion_ed_111,
instruccion_ed_112,
instruccion_ed_113,
instruccion_ed_114,
instruccion_ed_115,
instruccion_ed_116,
instruccion_ed_117,
instruccion_ed_118,
instruccion_ed_119,
instruccion_ed_120,
instruccion_ed_121,
instruccion_ed_122,
instruccion_ed_123,
instruccion_ed_124,
instruccion_ed_125,
instruccion_ed_126,
instruccion_ed_127,
instruccion_ed_128,
instruccion_ed_129,
instruccion_ed_130,
instruccion_ed_131,
instruccion_ed_132,
instruccion_ed_133,
instruccion_ed_134,
instruccion_ed_135,
instruccion_ed_136,
instruccion_ed_137,
instruccion_ed_138,
instruccion_ed_139,
instruccion_ed_140,
instruccion_ed_141,
instruccion_ed_142,
instruccion_ed_143,
instruccion_ed_144,
instruccion_ed_145,
instruccion_ed_146,
instruccion_ed_147,
instruccion_ed_148,
instruccion_ed_149,
instruccion_ed_150,
instruccion_ed_151,
instruccion_ed_152,
instruccion_ed_153,
instruccion_ed_154,
instruccion_ed_155,
instruccion_ed_156,
instruccion_ed_157,
instruccion_ed_158,
instruccion_ed_159,
instruccion_ed_160,
instruccion_ed_161,
instruccion_ed_162,
instruccion_ed_163,
instruccion_ed_164,
instruccion_ed_165,
instruccion_ed_166,
instruccion_ed_167,
instruccion_ed_168,
instruccion_ed_169,
instruccion_ed_170,
instruccion_ed_171,
instruccion_ed_172,
instruccion_ed_173,
instruccion_ed_174,
instruccion_ed_175,
instruccion_ed_176,
instruccion_ed_177,
instruccion_ed_178,
instruccion_ed_179,
instruccion_ed_180,
instruccion_ed_181,
instruccion_ed_182,
instruccion_ed_183,
instruccion_ed_184,
instruccion_ed_185,
instruccion_ed_186,
instruccion_ed_187,
instruccion_ed_188,
instruccion_ed_189,
instruccion_ed_190,
instruccion_ed_191,
instruccion_ed_192,
instruccion_ed_193,
instruccion_ed_194,
instruccion_ed_195,
instruccion_ed_196,
instruccion_ed_197,
instruccion_ed_198,
instruccion_ed_199,
instruccion_ed_200,
instruccion_ed_201,
instruccion_ed_202,
instruccion_ed_203,
instruccion_ed_204,
instruccion_ed_205,
instruccion_ed_206,
instruccion_ed_207,
instruccion_ed_208,
instruccion_ed_209,
instruccion_ed_210,
instruccion_ed_211,
instruccion_ed_212,
instruccion_ed_213,
instruccion_ed_214,
instruccion_ed_215,
instruccion_ed_216,
instruccion_ed_217,
instruccion_ed_218,
instruccion_ed_219,
instruccion_ed_220,
instruccion_ed_221,
instruccion_ed_222,
instruccion_ed_223,
instruccion_ed_224,
instruccion_ed_225,
instruccion_ed_226,
instruccion_ed_227,
instruccion_ed_228,
instruccion_ed_229,
instruccion_ed_230,
instruccion_ed_231,
instruccion_ed_232,
instruccion_ed_233,
instruccion_ed_234,
instruccion_ed_235,
instruccion_ed_236,
instruccion_ed_237,
instruccion_ed_238,
instruccion_ed_239,
instruccion_ed_240,
instruccion_ed_241,
instruccion_ed_242,
instruccion_ed_243,
instruccion_ed_244,
instruccion_ed_245,
instruccion_ed_246,
instruccion_ed_247,
instruccion_ed_248,
instruccion_ed_249,
instruccion_ed_250,
instruccion_ed_251,
instruccion_ed_252,
instruccion_ed_253,
instruccion_ed_254,
instruccion_ed_255

};
