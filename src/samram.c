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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "samram.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "joystick.h"


z80_bit samram_enabled={0};



char samram_rom_file_name[PATH_MAX]="";

z80_byte *samram_memory_pointer;



int samram_nested_id_poke_byte;
int samram_nested_id_poke_byte_no_time;
int samram_nested_id_peek_byte;
int samram_nested_id_peek_byte_no_time;


//Byte de config
//Bit 0: write protect, bit 1: cmos ram enabled, etc
z80_byte samram_settings_byte;



/*

5.6  The SamRam


    The SamRam contains a 32K static CMOS Ram chip, and some I/O logic for
    port 31.  If this port is read, it returns the position of the
    joystick, as a normal Kempston joystickinterface would.  If written to,
    the port controls a programmable latch chip (the 74LS259) which
    contains 8 latches:
 

       Bit    7   6   5   4   3   2   1   0
            ÚÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄÂÄÄÄ¿
       WRITE³   ³   ³   ³   ³  address  ³bit³
            ÀÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÁÄÄÄÙ


    The address selects on of the eight latches; bit 0 is the new state of
    the latch.  The 16 different possibilities are collected in the diagram
    below:

        OUT 31,   ³  Latch  ³ Result
        ÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
            0     ³    0    ³ Switch on write protect of CMOS RAM
            1     ³    "    ³ Writes to CMOS RAM allowed
            2     ³    1    ³ turn on CMOS RAM (see also 6/7)
            3     ³    "    ³ turn off CMOS RAM (standard Spec.  ROM)
            4     ³    2    ³ -
            5     ³    "    ³ Ignore all OUT's to 31 hereafter
            6     ³    3    ³ Select CMOS bank 0 (Basic ROM)
            7     ³    "    ³ Select CMOS bank 1 (Monitor,...)
            8     ³    4    ³ Select interface 1
            9     ³    "    ³ Turn off IF 1 (IF1 rom won't be paged)
           10     ³    5    ³ Select 32K ram bank 0 (32768-65535)
           11     ³    "    ³ Select 32K ram bank 1 (32768-65535)
           12     ³    6    ³ Turn off beeper
           13     ³    "    ³ Turn on beeper
           14     ³    7    ³ -
           15     ³    "    ³ -

    At reset, all latches are 0.  If an OUT 31,5 is issued, only a reset
    will give you control over the latches again.  The write protect latch
    is not emulated; you're never able to write the emulated CMOS ram in
    the emulator.  Latch 4 pulls up the M1 output at the expansion port of
    the Spectrum.  The Interface I won't page its ROM anymore then.


*/

//retorna direccion a memoria samram que hace referencia dir
//NULL si no es memoria samram 

/*
Samram tambien usa opcodes nuevos (que entiendo que solo funcionaban en el emulador)

;EDF9=LD A,NORMAL_ROM:(DE) \ LD (HL),A \ INC E \ INC H

Otros posibles:

;EDFF=NAAR DOS
;EDFE=ROMPOKE HL,A
;EDFD=LDIR ROMPOKE
;EDFC=LDDR ROMPOKE
;EDFB=RAMBANK LD HL,A
;EDFA=RAMBANK LD A,HL
;EDF9=LD A,NORMAL_ROM:(DE) \ LD (HL),A \ INC E \ INC H


ORG 0B7+(24 SHL 8)              ;ED FB
LDOBHLA:                                ;Used in SamRam ROM, but also multi

        

TABLES.8
ED_TABLE:
...

  F8    F9      FA     FB      FC       FD       FE
queer1,queer2,ldobahl,ldobhla,getbyte,sendbyte,ldromhla        
*/


z80_byte samram_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED);


void samram_opcode_edf9(void)
{
    //;EDF9=LD A,NORMAL_ROM:(DE) \ LD (HL),A \ INC E \ INC H

    //La documentacion es un poco confusa.... esa NORMAL_ROM yo jugaria que es la rom standard del spectrum ,pero no,
    //se refiere a su samrom
    //Esto se usa al escribir en pantalla, desde el menu nmi y el debugger, el tipo de letra
    //Si hiciera la lectura de la rom standard del spectrum, el tipo de letra seria el del spectrum, no el suyo
    //
    //En el codigo fuente, ese opcode tiene como comentario (traducido del neerlandés):
    //"ESTE ES UN 'OPCODE' QUE GARANTIZA UNA RUTINA DE IMPRESIÓN MÁS RÁPIDA;"
    //"OBTENER UN BYTE DEL SAMROM"
    //Aquí si se entiende mejor...

    //printf("EDF9=LD A,NORMAL_ROM:(DE) , LD (HL),A , INC E , INC H   HL=%d DE=%d\n",HL,DE);

    //z80_byte valor_leido=debug_nested_peek_byte_call_previous(samram_nested_id_peek_byte,DE);

    z80_byte valor_leido=samram_memory_pointer[DE];
    reg_a=valor_leido;

//instruccion_119()

//LD (HL),A
	poke_byte(HL,reg_a);




//instruccion_28()

//INC E
        inc_8bit(reg_e);



//instruccion_36()

//INC H
	inc_8bit(reg_h);
    


}


void samram_opcode_edfa(void)
{
//;EDFA=RAMBANK LD A,HL
    //printf("EDFA=RAMBANK LD A,HL HL=%d\n",HL);


    if (HL<32768) reg_a=debug_nested_peek_byte_no_time_call_previous(samram_nested_id_peek_byte,HL);

    
    else {
        //reg_a=samram_peek_byte_no_time(HL,0);
        //reg_a=debug_nested_peek_byte_no_time_call_previous(samram_nested_id_peek_byte,HL);



        //TODO: esto hace que se lea memoria samram desde el debugger, aunque se deberia ver memoria de debajo
        //pero si lo cambio, entonces el CAT! no funciona bien
        //reg_a=samram_memory_pointer[HL];


        //temp. no estoy seguro de esto. leer del bloque de ram contrario al que esta mapeado
        if (samram_settings_byte & 32) {
            reg_a=debug_nested_peek_byte_no_time_call_previous(samram_nested_id_peek_byte,HL);
        }
        else {
            reg_a=samram_memory_pointer[HL];
        }

    }
    
    
    
    //reg_a=samram_memory_pointer[HL];    
}


void samram_opcode_edfb(void)
{
//;EDFB=RAMBANK LD HL,A
   //printf("EDFB=RAMBANK LD HL,A HL=%d\n",HL);

    if (HL<32768) debug_nested_poke_byte_no_time_call_previous(samram_nested_id_poke_byte,HL,reg_a);

    else {

        //samram_memory_pointer[HL]=reg_a;

        //temp. no estoy seguro de esto. escribir en el bloque de ram contrario al que esta mapeado
        if (samram_settings_byte & 32) {
            debug_nested_poke_byte_no_time_call_previous(samram_nested_id_poke_byte,HL,reg_a);

        }
        else {
            samram_memory_pointer[HL]=reg_a;
        }        

    }
    
}

void samram_opcode_edfe(void)
{
    //printf("EDFE=ROMPOKE HL,A HL=%d\n",HL);    

    debug_nested_poke_byte_no_time_call_previous(samram_nested_id_poke_byte,HL,reg_a);
}

int samram_if_rom_mapped(void)
{
    if (samram_settings_byte & 2) return 0;
    else return 1;
}

int samram_get_rom_mapped(void)
{
    //rom mapeada, ver cual
    int romblock=samram_settings_byte & 8;
    romblock=romblock >> 3;

    return romblock;
}

int samram_tap_save_detect(void)
{

    if (reg_pc!=1222) return 0;

    //no rom mapeada
    if (!samram_if_rom_mapped()) {
        return 1;
    }

    //samrom mapeada. Sera la 0?
    int rommapped=samram_get_rom_mapped();

    if (rommapped==1) return 0;

    return 1;


}

z80_byte *samram_check_if_sam_area(z80_int dir,int writing)
{

  //si espacio rom
  if (dir<16384) {
    if (!samram_if_rom_mapped()) return NULL;

    //Si intenta escribir, denegar. Tal y como se hacia en emulador Z80, no se permite escribir en esa ram desde el emulador
    if (writing) {
        //printf("Trying to write mapped cmos ram-rom dir %d\n",dir);
        return NULL;
    }

    //rom mapeada, ver cual
    int romblock=samram_get_rom_mapped();
    
    int offset=dir+16384*romblock;
    //printf("romblock %d\n",romblock);
    return &samram_memory_pointer[offset];
    
  }
  
  //si mayor 32767
  if (dir>32767) {
      
    if (samram_settings_byte & 32) {
        int offset=dir; //la shadow ram esta justo despues de la rom
        return &samram_memory_pointer[offset];
    }
  }
  
  return NULL;
}





z80_byte samram_poke_byte(z80_int dir,z80_byte valor)
{

	//samram_original_poke_byte(dir,valor);
    //Llamar a anterior
    //pero no cuando escribimos en shadow ram
    int escribir=1;
    
    
    z80_byte *samdir;
    samdir=samram_check_if_sam_area(dir,1);
    if (samdir!=NULL) {
        //si escribimos en shadow, no escribir en la ram normal
        if (dir>32767) {
            //printf("Poke byte shadow ram %d\n",dir);
            escribir=0;
        }
        
        *samdir=valor;
    }

    if (escribir) debug_nested_poke_byte_call_previous(samram_nested_id_poke_byte,dir,valor);


    //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
    return 0;


}

z80_byte samram_poke_byte_no_time(z80_int dir,z80_byte valor)
{
    //samram_original_poke_byte_no_time(dir,valor);
    //Llamar a anterior
    //pero no cuando escribimos en shadow ram
    int escribir=1;
    
    
    z80_byte *samdir;
    samdir=samram_check_if_sam_area(dir,1);
    if (samdir!=NULL) {
        //si escribimos en shadow, no escribir en la ram normal
        if (dir>32767) {
            //printf("Poke byte shadow ram %d\n",dir);
            escribir=0;
        }
        
        *samdir=valor;
    }

    if (escribir) debug_nested_poke_byte_no_time_call_previous(samram_nested_id_poke_byte,dir,valor);


    //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
    return 0;        
        

}

z80_byte samram_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(samram_nested_id_peek_byte,dir);



	z80_byte *samdir;
        samdir=samram_check_if_sam_area(dir,0);
        if (samdir!=NULL) {
          if (dir>32767) {
               //printf("peek byte shadow ram %d\n",dir);
           }            
           return *samdir;
        }


	return valor_leido;
}

z80_byte samram_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(samram_nested_id_peek_byte_no_time,dir);

	
	z80_byte *samdir;
        samdir=samram_check_if_sam_area(dir,0);
        if (samdir!=NULL) {
          if (dir>32767) {
               //printf("peek byte shadow ram %d\n",dir);
           }               
           return *samdir;
        }

	return valor_leido;
}



//Establecer rutinas propias
void samram_set_peek_poke_functions(void)
{
    debug_printf (VERBOSE_DEBUG,"Setting samram poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	samram_nested_id_poke_byte=debug_nested_poke_byte_add(samram_poke_byte,"Samram poke_byte");
	samram_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(samram_poke_byte_no_time,"Samram poke_byte_no_time");
	samram_nested_id_peek_byte=debug_nested_peek_byte_add(samram_peek_byte,"Samram peek_byte");
	samram_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(samram_peek_byte_no_time,"Samram peek_byte_no_time");

}

//Restaurar rutinas de samram
void samram_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before samram");


	debug_nested_poke_byte_del(samram_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(samram_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(samram_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(samram_nested_id_peek_byte_no_time);
}



void samram_alloc_memory(void)
{
        int size=SAMRAM_SIZE;  

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for samram emulation",size/1024);

        samram_memory_pointer=malloc(size);
        if (samram_memory_pointer==NULL) {
                cpu_panic ("No enough memory for samram emulation");
        }


}

int samram_load_rom(void)
{

        FILE *ptr_samram_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading samram rom %s",samram_rom_file_name);

  			ptr_samram_romfile=fopen(samram_rom_file_name,"rb");
                if (!ptr_samram_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
                }

        if (ptr_samram_romfile!=NULL) {

                leidos=fread(samram_memory_pointer,1,32768,ptr_samram_romfile);
                fclose(ptr_samram_romfile);

        }



        if (leidos!=32768 || ptr_samram_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading samram rom");
                return 1;
        }

        return 0;
}



void samram_enable(void)
{

  if (!MACHINE_IS_SPECTRUM_48) {
    debug_printf(VERBOSE_INFO,"Can not enable samram on non Spectrum 48 machine");
    return;
  }

	if (samram_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"Already enabled");
		return;
	}

	if (samram_rom_file_name[0]==0) {
		debug_printf (VERBOSE_ERR,"Trying to enable Samram but no ROM file selected");
		return;
	}

	samram_alloc_memory();
	if (samram_load_rom()) return;

	samram_set_peek_poke_functions();

	samram_enabled.v=1;

	
	//samram_settings_byte=2; //no mapeado

	samram_settings_byte=0; //activar cmos ram y seleccionar bank 0


	joystick_emulation=JOYSTICK_KEMPSTON;
	debug_printf(VERBOSE_DEBUG,"Setting joystick 1 emulation to Kempston as NMI menu needs it");    

	reset_cpu();    



}

void samram_disable(void)
{
	if (samram_enabled.v==0) return;

	samram_restore_peek_poke_functions();

	free(samram_memory_pointer);

	samram_enabled.v=0;
}

void samram_nmi(void)
{
    //Si se dispara la nmi, se habilita mapeo de la samrom
    samram_settings_byte=0; //activar cmos ram y seleccionar bank 0

}

/*
void samram_press_button(void)
{

        if (samram_enabled.v==0) {
                debug_printf (VERBOSE_ERR,"Trying to press Samram button when it is disabled");
                return;
        }

	samram_settings_byte=0; //activar cmos ram y seleccionar bank 0

	reset_cpu();


}
*/



void samram_write_port(z80_byte value)
{
  //printf ("write sam ram value %02XH\n",value);

  int bitvalue=value&1;
  
  int bitnumber=(value>>1)&7;
  
  //samram_settings_byte
  z80_byte maskzero=1;
  
  maskzero=maskzero<<bitnumber;
  
  maskzero ^=255;
  
  //poner a cero
  samram_settings_byte &=maskzero;
  
  //y OR con valor
  bitvalue=bitvalue<<bitnumber;
  
  samram_settings_byte |=bitvalue;

  //printf ("write sam ram final settings %02XH\n",samram_settings_byte);



    //no hacer los printf siguientes
  return;

  switch (value) {
      case 0:
        printf("0      Switch on write protect of CMOS RAM\n");
      break;

            
                  case 1:
        printf("1          Writes to CMOS RAM allowed\n");
              break;


                  case 2:
        printf("2         turn on CMOS RAM (see also 6/7)\n");
              break;


                  case 3:
        printf("3          turn off CMOS RAM (standard Spec.  ROM)\n");
              break;


                  case 4:
        printf("4          -\n");
              break;


                  case 5:
        printf("5         Ignore all OUT's to 31 hereafter\n");
              break;


                  case 6:
        printf("6          Select CMOS bank 0 (Basic ROM)\n");
              break;


                  case 7:
        printf("7      Select CMOS bank 1 (Monitor,...)\n");
              break;


                  case 8:
        printf("8          Select interface 1\n");
              break;


                  case 9:
        printf("9          Turn off IF 1 (IF1 rom won't be paged)\n");
              break;


                 case 10:
        printf("10          Select 32K ram bank 0 (32768-65535)\n");
              break;


                 case 11:
        printf("11      Select 32K ram bank 1 (32768-65535)\n");
              break;


                 case 12:
        printf("12      Turn off beeper\n");
              break;


                 case 13:
        printf("13      Turn on beeper\n");
              break;


                 case 14:
        printf("14      -\n");
              break;


                 case 15:
        printf("15      -\n");
              break;


  }


}
