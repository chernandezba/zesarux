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
#include <string.h>

#include "z88.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "core_z88.h"
#include "contend.h"
#include "joystick.h"
#include "zxvision.h"
#include "operaciones.h"
#include "utils.h"
#include "audio.h"
#include "timer.h"
#include "compileoptions.h"


char valor_sonido_3200hz=+50;



z80_int blink_pixel_base[4];
z80_int blink_sbr;

z80_byte blink_com=0;

/*
The COM register ( COM, $B0)

This register is detailed first because it controls various diverse aspects of BLINK operations:

Bit         Name        Function
7           SRUN        Speaker source (0=SBIT, 1=TxD or 3200hz)
6           SBIT        SRUN=0: 0=low, 1=high; SRUN=1: 0=3200 hz, 1=TxD
5           OVERP       Set to overprogram EPROM s
4           RESTIM      Set to reset the RTC, clear to continue
3           PROGRAM     Set to enable EPROM  programming
2           RAMS        Binding of lower 8K of segment 0: 0=bank 0, 1=bank 20
1           VPPON       Set to turn programming voltage ON
0           LCDON       Set to turn LCD ON, clear to turn LCD OFF
*/

//Puerto de escritura INT (puerto 0xB1)
z80_byte blink_int=0;

/*

The INT ($B1) register controls which interrupts are enabled:

BIT         NAME        Function
7           KWAIT       If set, reading the keyboard will Snooze
6           A19         If set, an active high on A19 will exit Coma
5           FLAP        If set, flap interrupts are enabled
4           UART        If set, UART interrupts are enabled
3           BTL         If set, battery low interrupts are enabled
2           KEY         If set, keyboard interrupts (Snooze or Coma) are enabl.
1           TIME        If set, RTC interrupts are enabled
0           GINT        If clear, no interrupts get out of blink

*/



//Puerto de lectura STA (puerto 0xB1)
z80_byte blink_sta=0;

/*

The STA ($B1) register provides information about which interrupt has actually occurred:

BIT         NAME        Function
7           FLAPOPEN    If set, flap open else flap closed
6           A19         If set, high level on A19 occurred during coma
5           FLAP        If set, positive edge has occurred on FLAPOPEN
4           UART        If set, an enabled UART interrupt is active
3           BTL         If set, battery low pin is active
2           KEY         If set, a column has gone low in snooze (or coma)
1           -           -
0           TIME        If set, an enabled TIME interrupt is active



*/



//Puerto de escritura EPR (puerto 0xB3)
z80_byte blink_epr=0;

//Puerto de escritura TACK (puerto 0xB4)
//z80_byte blink_tack=0;

//Puerto de escritura TMK (puerto 0xB5)
z80_byte blink_tmk=0;

//Puerto de lectura TSTA (puerto 0xB5)
z80_byte blink_tsta=0;

//indica que segmentos de memoria estan mapeados en las direcciones. (SR0,1,2,3)
//0000-3FFF
//4000-7FFF
//8000-BFFF
//C000-FFFF
z80_byte blink_mapped_memory_banks[4];

//Puertos de lectura TIM0..4 (puertos 0xD0 hasta D4)
z80_byte blink_tim[5]={0,0,0,0,0};

//Puerto de lectura RXD (puerto 0xE0)
z80_byte blink_rxd=0;

//Puerto de lectura RXD (puerto 0xE1)
z80_byte blink_rxe=0;

//Puerto de escritura RXC (puerto 0xE2)
z80_byte blink_rxc=0;

//Puerto de escritura TXD (puerto 0xE3)
z80_byte blink_txd=0;

//Puerto de escritura TXC (puerto 0xE4)
z80_byte blink_txc=0;

//Puerto de escritura UMK (puerto 0xE5)
z80_byte blink_umk=0;

//Puerto de lectura UIT (puerto 0xE5)
z80_byte blink_uit=0;


//Puertos de teclado
/*
-------------------------------------------------------------------------
         | D7     D6      D5      D4      D3      D2      D1      D0
-------------------------------------------------------------------------
A15 (#7) | RSH    SQR     ESC     INDEX   CAPS    .       /       £
A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
A13 (#5) | [      SPACE   1       Q       A       Z       L       0
A12 (#4) | ]      LFT     2       W       S       X       M       P
A11 (#3) | -      RGT     3       E       D       C       K       9
A10 (#2) | =      DWN     4       R       F       V       J       O
A9  (#1) | \      UP      5       T       G       B       U       I
A8  (#0) | DEL    ENTER   6       Y       H       N       7       8
------------------------------------------------------------------------- 


CTRL = Diamond
ALT = Square
F1 = Help
F2 = Index
F3 = Menu


*/

//se incrementa en 1 cada 1/200 s desde core_z88
unsigned int z88_contador_para_flap;

z80_byte blink_kbd_a15=255;
z80_byte blink_kbd_a14=255;
z80_byte blink_kbd_a13=255;
z80_byte blink_kbd_a12=255;
z80_byte blink_kbd_a11=255;
z80_byte blink_kbd_a10=255;
z80_byte blink_kbd_a9=255;
z80_byte blink_kbd_a8=255;


z80_bit z88_snooze={0};
z80_bit z88_coma={0};
z80_bit estado_parpadeo_cursor;

int z88_footer_timer_slot3_activity_indicator=0;

int z88_slot3_activity_indicator=0;

//para que nunca se salte a modo snooze. Quitar esto cuando ya funcione bien
int snooze_desactivado=1;

//0 RAM, 1 ROM(no usado), 2 EPROM, 3 EPROM, 4 Hybrida 512kb RAM+512 kb EPROM
//El concepto de rom en slot 1, 2, 3 es incorrecto y solo se usaba en ZEsarUX 2.0
char *z88_memory_types[5]={
	"RAM",
	"ROM",  //ROM no usado. se deja aqui igualmente porque valores de ram es 0 y eprom 2 , y hay un "hueco" en el 1
	"EPROM",
	"Flash Int",
	"Hyb R+E"
};

/*

Slots de memoria totales:

0: ROM y RAM internas
1: RAM/EPROM/FLASH
2: RAM/EPROM/FLASH
3: RAM/EPROM/FLASH


        Banks   00 - 3F  are internal (lower half ROM, upper half RAM)
        Banks   40 - 7F  are wired to Slot 1
        Banks   80 - BF  are wired to Slot 2
        Banks   C0 - FF  are wired to Slot 3 (usually EPROM).

se necesita saber, de cada slot (del 1 al 3):
tamanyo total  (formato z80_long_int)
tipo: ram, rom, eprom, flash, hybrida ram+eprom
RAM->0
ROM->1
EPROM->2
FLASH->3
RAM+EPROM->4


slot 0 hay que indicar tamanyo de la rom interna y de la ram inerna




Memoria interna no se define en el mismo array

*/


//slot 0 identifica la memoria interna
z88_memory_slot z88_memory_slots[4];


//si la tarjeta eprom o flash debe escribir a disco ya que se ha escrito en alguna direccion
int z88_eprom_or_flash_must_flush_to_disk=0;


//modo programacion forzado para una flash
//esto se usa por ejemplo para escribir archivo desde menu, desde pc a flash, para no tener que ponerla en modo programacion
z80_bit z88_flash_forced_writing_mode={0};


//memoria interna
//size-1, para usar valor como mascara
z80_long_int z88_internal_rom_size;
z80_long_int z88_internal_ram_size;



z80_byte *z88_puntero_memoria;

/*

Disposicion del teclado Z88

ESC  1 2 3 4 5 6 7 8 9 0  -_  =+  \|  DEL
TAB q w e r t y u i o p [{  ]}
DIA  a s d f g h j k l ;: '" libra~ ENTER
SHIFT z x c v b n m ,< .> /? SHIFT   UP
INDEX MENU HELP SQU SPACE  CAPSLOCK  LEFT RIGHT DOWN


Teclado PC english
`~ 1 2 3 4 5 6 7 8 9 0  -_  =+  BACKSPACE
TAB q w e r t y u i o p [{ ]}  \|
CAPS a s d f g h j k l  ;:  '"  ENTER
SHIFT z x c v b n m  ,<  .>  /?  SHIFT UP
CTRL ALT SPACE ALT CTRL

Teclado PC Spanish
ºª 1 2 3 4 5 6 7 8 9 0 '?  ¡¿  BACKSPACE
TAB q w e r t y u i o p `^  +*
CAPS a s d f g h j k l  ñ  '"  çÇ  ENTER
SHIFT <>  z x c v b n m  ,;  .:  -_  SHIFT
CTRL ALT SPACE ALT CTRL


*/

//0=default
//1=spanish
//int z88_keymap_type=0;



void z88_set_default_memory_pages(void)
{
	blink_mapped_memory_banks[0]=0;	
	blink_mapped_memory_banks[1]=0;	
	blink_mapped_memory_banks[2]=0;	
	blink_mapped_memory_banks[3]=0;	

	// COM.RAMS = 0 (lower 8K = Bank 0)
	blink_com &=(255-4);

}


/*
        Banks   00 - 3F  are internal (lower half ROM, upper half RAM)
        Banks   40 - 7F  are wired to Slot 1
        Banks   80 - BF  are wired to Slot 2
        Banks   C0 - FF  are wired to Slot 3 (usually EPROM).
*/

//Si se llama a init_z88_memory_slots desde cpu_set_turbo_speed, no ejecutar cambio de slots,
//si no, al cambiar speed cpu, se perderian los slots insertados
int do_not_run_init_z88_memory_slots=0;

void init_z88_memory_slots(void)
{

    if (do_not_run_init_z88_memory_slots) return;

	//Offsets siempre fijos. NO TOCAR!!
	z88_memory_slots[0].offset_total=0;
	z88_memory_slots[1].offset_total=0x40*16384;
        z88_memory_slots[2].offset_total=0x80*16384;
        z88_memory_slots[3].offset_total=0xC0*16384;

	//de slot 0 solo se usa offset_total
	//z88_memory_slots[0].size=(1024*1024)-1;
	//z88_memory_slots[0].type=1;

	//128KB RAM memoria interna
        z88_internal_ram_size=131071;


	//Sin ninguna tarjeta insertada

	z88_memory_slots[1].size=0;
	z88_memory_slots[1].type=0;


	z88_memory_slots[2].size=0;
        z88_memory_slots[2].type=0;


	z88_memory_slots[3].size=0;
        z88_memory_slots[3].type=0;


	//Nombre eprom vacio. esto solo es necesario para la opcion de menu de insert eprom. nos aseguramos que es texto vacio inicialmente
	z88_memory_slots[1].eprom_flash_nombre_archivo[0]=0;
	z88_memory_slots[2].eprom_flash_nombre_archivo[0]=0;
	z88_memory_slots[3].eprom_flash_nombre_archivo[0]=0;


	//Tarjetas flash no estan en modo comando
	z88_memory_slots[0].z88_flash_card_in_command_mode.v=0;
	z88_memory_slots[1].z88_flash_card_in_command_mode.v=0;
	z88_memory_slots[2].z88_flash_card_in_command_mode.v=0;
	z88_memory_slots[3].z88_flash_card_in_command_mode.v=0;


}

void z88_reset_slot3_activity_indicator(void)
{

    z88_slot3_activity_indicator=0;

    menu_footer_z88();

    //Reflejar cambios en el icono del slot 3
    menu_draw_ext_desktop();   
}

//Indicador de actividad para escritura en slot 3
//No lo llamamos para lectura, porque si por ejemplo metemos un juego en slot 3, se estaria
//llamando continuamente aqui
void z88_set_slot3_activity_indicator(void)
{

    //Si ya estaba activo el indicador, no hacer nada
    if (z88_slot3_activity_indicator) return;

    z88_slot3_activity_indicator=1;

    menu_footer_z88();

    //Reflejar cambios en el icono del slot 3
    menu_draw_ext_desktop(); 

    z88_footer_timer_slot3_activity_indicator=2;    
}

void z88_set_z88_eprom_or_flash_must_flush_to_disk(void)
{

    //marcamos bit para decir que se ha escrito en eprom para luego hacer flush
    z88_eprom_or_flash_must_flush_to_disk=1;    

    //Indicar actividad en slot 3
    z88_set_slot3_activity_indicator();


}

//Borrar bloque en la tarjeta flash
void z88_flash_erase_block(z80_byte slot,z80_long_int offset)
{
                                if (slot!=3) {
                                        z88_memory_slots[slot].statusRegister = 0xA8; // SR.7 = Ready, SR.5 = 1 (Block Erase Error), SR.3 = 1 (no VPP)
                                        return;
                                }

                                //1           VPPON       Set to turn programming voltage ON
                                if ((blink_com & 1)==0) {
                                        z88_memory_slots[slot].statusRegister = 0xA8; // SR.7 = Ready, SR.5 = 1 (Block Erase Error), SR.3 = 1 (no VPP)
                                        debug_printf (VERBOSE_DEBUG,"Trying to erase Flash but VPP programming voltage bit not enabled");
                                        return;
                                }

                                //slot 3 y eprom programming. dejamos escritura
                                z88_memory_slots[slot].statusRegister = 0x80; // SR.7 = Ready, SR.5 = 0 (Block Erase OK), SR.4 = 0 (Program OK), SR.3 = 0 (VPP OK)

				//nos situamos en slot 3
				offset=offset-(1024*1024*3);

				//printf ("offset relativo a slot 3: %d\n",offset);


				//Obtener que numero de bloque de 64 kb vamos a borrar
				z80_byte block_number=offset/65536;

				debug_printf (VERBOSE_INFO,"Clearing Flash block %d of 64 kb size",block_number);

				z80_long_int offset_en_bloque=z88_memory_slots[slot].offset_total + block_number*65536;


				int longitud=65536;

				for (;longitud>0;offset_en_bloque++,longitud--) {
					z88_puntero_memoria[offset_en_bloque]=0xFF;
				}

                                //marcamos bit para decir que se ha escrito en eprom para luego hacer flush
                                z88_set_z88_eprom_or_flash_must_flush_to_disk();

}


//Procesar escrituras en la flash
void z88_procesar_flash_command(z80_byte valor,z80_byte slot,z80_long_int offset)
{

	//Si no estamos en modo comando
        if (z88_memory_slots[slot].z88_flash_card_in_command_mode.v == 0) {
            //Conmutar a modo comando
            z88_memory_slots[slot].z88_flash_card_in_command_mode.v = 1;
            z88_memory_slots[slot].executing_command_number = 0;
        }

	//Si estamos en modo comando
        if (z88_memory_slots[slot].z88_flash_card_in_command_mode.v) {
            if (z88_memory_slots[slot].executing_command_number == 0x10 || z88_memory_slots[slot].executing_command_number == 0x40) {
                // Byte Program Command, Part 2 (initial Byte Program command received), 
                // we've fetched the Byte Program Address & Data, programming will now begin...
                //programByteAtAddress(addr, b);

                                if (slot!=3) {
					z88_memory_slots[slot].statusRegister = 0x98; // SR.7 = Ready, SR.4 = 1 (Program Error), SR.3 = 1 (no VPP)
					return;
				}
                              
				//1           VPPON       Set to turn programming voltage ON  
                                if ((blink_com & 1)==0) {
					z88_memory_slots[slot].statusRegister = 0x98; // SR.7 = Ready, SR.4 = 1 (Program Error), SR.3 = 1 (no VPP)
                                        debug_printf (VERBOSE_DEBUG,"Trying to write to Flash but VPP programming voltage bit not enabled");
                                        return;
                                }

                                //slot 3 y eprom programming. dejamos escritura
				z88_memory_slots[slot].statusRegister = 0x80; // SR.7 = Ready, SR.4 = 0 (Program OK), SR.3 = 0 (VPP OK)

                                //Escritura normal
                                z88_puntero_memoria[offset]=valor;

                                //marcamos bit para decir que se ha escrito en eprom para luego hacer flush
                                z88_set_z88_eprom_or_flash_must_flush_to_disk();

		                z88_memory_slots[slot].executing_command_number = 0x70;
          } 

	  else {
            switch (valor) {

		
		case 0x20:  // Erase Command, part 1  
			// wait for new sub command sequence for erase block
			z88_memory_slots[slot].executing_command_number = 0x20;
			break;
		

		case 0x50: // Clear Status Register
			z88_memory_slots[slot].executing_command_number  = 0;
			// SR.7 = Ready, SR.5 = 0 (Block Erase OK), SR.4 = 0 (Program OK), SR.3 = 0 (VPP OK)
			z88_memory_slots[slot].statusRegister = 0x80;
		break;

		case 0x70:  // Read Status Register
			z88_memory_slots[slot].executing_command_number = 0x70;
		break;


		case 0x90:  // Chip Identification (get Manufacturer & Device Code) 
			z88_memory_slots[slot].executing_command_number = 0x90;
		break;


		case 0x10:
		case 0x40:  // Byte Program Command, Part 1, get address and byte to program.. 
			z88_memory_slots[slot].executing_command_number = 0x40;
		break;

		case 0xD0: // Block Erase Command (which this bank is part of), part 2 
			if (z88_memory_slots[slot].executing_command_number == 0x20) {
			z88_memory_slots[slot].executing_command_number = 0xD0;
				z88_flash_erase_block(slot,offset);
			}
		break;


		case 0xFF:  // Reset chip to Read Arrary Mode
			z88_memory_slots[slot].executing_command_number=0;
			z88_memory_slots[slot].z88_flash_card_in_command_mode.v=0;
		break;


		default:
				debug_printf (VERBOSE_DEBUG,"Z88 Flash command 0x%X not implemented",valor);
				// command was not identified; Either 2. part of a prev. command or unknown...
				z88_memory_slots[slot].z88_flash_card_in_command_mode.v=0;
				z88_memory_slots[slot].executing_command_number=0;

				return;
		break; 
	   }
	  }
	}
				
}

//Retornar status de la flash card
z80_byte z88_get_flash_status(z80_byte slot,z80_int addr)
{

	debug_printf (VERBOSE_DEBUG,"Calling Get flash status, command=0x%X",z88_memory_slots[slot].executing_command_number);

	addr &= 0x3FFF; // only bank offset range..

        switch (z88_memory_slots[slot].executing_command_number) {
            case 0x10: // Byte Program Command
            case 0x40: // Byte Program Command
            case 0x70: // Read Status Register
            case 0xD0: // Block Erase Command
                return z88_memory_slots[slot].statusRegister;
	    break;

            case 0x90: // Get Device Identification
                //if ((getBankNumber() & 0x3F) == 0) {
                    // Device and Manufacturer Code can only be  
                    // fetched in bottom bank of card...


                    switch (addr) {
                        case 0:
                            return z88_memory_slots[slot].flash_manufacturer_code;    // 0000 = Manufacturer Code 
			break;

                        case 1:
                            return z88_memory_slots[slot].flash_device_code;     // 0001 = Device Code
			break;

                        default:
				debug_printf (VERBOSE_DEBUG,"Unknown address 0x%X where returning device identification (command 0x90)",addr);
                            return 0xFF;                // XXXX = Unknown behaviour...
			break;
                    }

                //} else {
                //    return 0xFF;                // XXXX = Unknown behaviour...
                //}

	    break;

            default: // unknown command! 
		debug_printf (VERBOSE_DEBUG,"Unknown command 0x%X where reading flash status",z88_memory_slots[slot].executing_command_number);
                return 0xFF;
	    break;
        }
}

//Funcion auxiliar para poke y peek
//Hacer poke o peek, si es ROM, no
//dir solo se usa en caso de flash, en get_flash_estatus
z80_byte poke_peek_byte_no_time_z88_aux(z80_byte bank,z80_byte slot,z80_long_int offset,z80_byte valor,int accion,z80_int dir)
{

    z80_byte seccion;


    if (accion==0) {

        //Escritura

        //Volver si rom interna
        if (bank<0x20) return 0;


        //mirar si eprom o ram
        if (slot>=1) {

            switch (z88_memory_slots[slot].type) {
                case 1:
                    //Error. Esto no deberia suceder
                    cpu_panic ("ROM cards do not exist on Z88");
                    return 0;
                break;

                case 2:
                    //EPROM. ver si slot 3 y si esta programacion de eprom activa
                    if (slot!=3) return 0;

                    if ((blink_com & 8)==0) {
                        debug_printf (VERBOSE_DEBUG,"Trying to write to EPROM but EPROM PROGRAM bit not enabled");
                        return 0;
                    }

                    //slot 3 y eprom programming. dejamos escritura

                    //marcamos bit para decir que se ha escrito en eprom para luego hacer flush
                    z88_set_z88_eprom_or_flash_must_flush_to_disk();
                break;

                case 3:
                    //FLASH
                    if (z88_flash_forced_writing_mode.v==1) {
                        //escribimos directamente, sin tener que pasar por modo programacion
                        z88_puntero_memoria[offset]=valor;
                        z88_set_z88_eprom_or_flash_must_flush_to_disk();
                        return 0;
                    }

                    else {
                        debug_printf (VERBOSE_DEBUG,"Processing Flash Command 0x%x on slot: %d",valor,slot);
                        z88_procesar_flash_command(valor,slot,offset);
                        return 0;
                    }

                break;

                case 4:
                    //Primeros 512kb: RAM
                    //Siguientes 512kb: EPROM
                    

                    //Si zona eprom
/*
    Banks   40 - 7F  are wired to Slot 1
    Banks   80 - BF  are wired to Slot 2
    Banks   C0 - FF  are wired to Slot 3 
*/
                    //They are organized with ram on the bottom (bank 00-1F) and flash (bank 20-3F) on top.
                    seccion=bank-slot*0x40;
                    if (seccion>=0x20) {

                        //EPROM. ver si slot 3 y si esta programacion de eprom activa
                        if (slot!=3) return 0;

                        if ((blink_com & 8)==0) {
                                debug_printf (VERBOSE_DEBUG,"Trying to write to EPROM on hybdrid card but EPROM PROGRAM bit not enabled");
                                return 0;
                        }

                        //slot 3 y eprom programming. dejamos escritura

                        //marcamos bit para decir que se ha escrito en eprom para luego hacer flush
                        z88_set_z88_eprom_or_flash_must_flush_to_disk();

                    }
                break;
            }
        }


        //Escritura normal, ya sea en RAM o EPROM con EPROM PROGRAM bit activo
        z88_puntero_memoria[offset]=valor;
        return 0;
    }

    else {

        //Lectura

        //Si es tarjeta flash y en modo comando, actuar en consecuencia
        if (z88_memory_slots[slot].type==3 && z88_memory_slots[slot].z88_flash_card_in_command_mode.v) {
            z80_byte flash_status_value;
            flash_status_value=z88_get_flash_status(slot,dir);
            debug_printf (VERBOSE_DEBUG,"Returning flash value 0x%X when in command: 0x%X, address: 0x%X",flash_status_value,z88_memory_slots[slot].executing_command_number,dir);
            return flash_status_value;
        }

        else {
            return z88_puntero_memoria[offset];
        }
    }

}

//Funciones de poke y peek SIN comprobar paginas que entra en 0..8192..16383
//Usado como auxiliar por funcion normal poke_peek_byte_no_time_z88_bank
//y en dibujado de pantalla, ya que la direccion suele venir entre 0...16383 y no queremos que compruebe 0..8192..16383
//poke y peek
//accion: 0 poke, 1 peek
z80_byte poke_peek_byte_no_time_z88_bank_no_check_low(z80_int dir,z80_byte bank,z80_byte valor,int accion)
{


	//Aumentar bank segun dir
	bank +=dir/16384;
	//dir mascara 16383
	dir &=16383;

        int slot=bank/0x40;
        z80_long_int offset_in_slot=((bank & 0x3f)*16384) + dir;
        z80_long_int slot_size;
        slot_size=z88_memory_slots[slot].size;


        switch (slot) {
                case 0:
                        //z88_internal_ram_size
                        if (bank>=0x20) {
                                //Internal RAM
                                if (offset_in_slot>z88_internal_ram_size+512*1024) {
                                        offset_in_slot=512*1024+(offset_in_slot & (z88_internal_ram_size));
                                        //printf ("llegado al limite de ram\n");
                                }
                        }

                        //TODO: es necesario limitar tamanyo rom interna?
                break;

                case 1:
                case 2:
                case 3:

                        //caso tamanyo cero del slot (no hay slot)
                        if (slot_size==0) {
                                //Peek siempre devuelve 255
                                //Nota: esto lo he intentado hacer con un offset &= 0,
                                //como seria lo logico, pero no funciona
                                //lo mejor es devolver 255 siempre para peek, cuando slot no existe, y ya funciona
                                if (accion==1) return 255;
                        }

                        if (offset_in_slot>slot_size) offset_in_slot=offset_in_slot & (slot_size);
                break;
        }

        z80_long_int offset_total=z88_memory_slots[slot].offset_total + offset_in_slot;



	return poke_peek_byte_no_time_z88_aux(bank,slot,offset_total,valor,accion,dir);

        
        
}


//Funcion de poke en la zona baja de la memoria 0...8192..16383
//poke y peek
//accion: 0 poke, 1 peek
z80_byte poke_peek_byte_no_time_z88_low(z80_int dir,z80_byte bank,z80_byte valor,int accion)
{

	//Ver division de memoria 0...8192...16383

	z80_long_int offset_total;

	//Se establece a 0, se lee en funcion auxiliar
	int slot=0;
        
        
        //caso segmento 0-16383
        if (dir<8192) { 
                //ROM0 o RAM20H
                if ((blink_com & 4)==0) {//ROM0
                        bank=0;
                }       
                else {  
                        //printf ("leer/escribir en direccion<8192 y es ram 20H. dir=%d\n",dir);
                        bank=0x20;
                }       
                offset_total=bank*16384;
                offset_total+=dir;
                
                return poke_peek_byte_no_time_z88_aux(bank,slot,offset_total,valor,accion,dir);
                
                
        }
        
        else {
                //8kb superiores
                if ((bank&1)==0) dir &= 0x1FFF;
                bank=bank&254;
                
                offset_total=bank*16384;
                offset_total+=dir;
                return poke_peek_byte_no_time_z88_aux(bank,slot,offset_total,valor,accion,dir);
        }       
        

}



//poke con bank. no comprueba zona 0...16383
//dir puede ser cualquiera entre 0...65535
void poke_byte_no_time_z88_bank_no_check_low(z80_int dir,z80_byte bank,z80_byte valor)
{
        poke_peek_byte_no_time_z88_bank_no_check_low(dir,bank,valor,0);
}

//peek con bank. no comprueba zona 0...16383
//dir puede ser cualquiera entre 0...65535
z80_byte peek_byte_no_time_z88_bank_no_check_low(z80_int dir,z80_byte bank)
{
        //valor no se usa,ponemos 0 (cualquier cosa)
        return poke_peek_byte_no_time_z88_bank_no_check_low(dir,bank,0,1);
}



//Funciones poke/peek normales que vienen de la cpu
//poke y peek
//accion: 0 poke, 1 peek
z80_byte poke_peek_byte_no_time_z88(z80_int dir,z80_byte valor,int accion)
{

        //Obtenemos zona de memoria
        z80_byte zona=dir/16384;

        //Obtenemos numero de banco mapeado
        z80_byte bank=blink_mapped_memory_banks[zona];

        if (dir>=16384) {
		//dir entre 16384...65535. como bank ya esta ajustado a la zona, ajustamos tambien direccion
		dir &=16383;

                return poke_peek_byte_no_time_z88_bank_no_check_low(dir,bank,valor,accion);
        }


	//Zona baja de la memoria (0...16383)
        return poke_peek_byte_no_time_z88_low(dir,bank,valor,accion);

}


//Funcion poke normal de la cpu, sin contar t-estados
void poke_byte_no_time_z88(z80_int dir,z80_byte valor) {

#ifdef EMULATE_VISUALMEM
        set_visualmembuffer(dir);
#endif


	poke_peek_byte_no_time_z88(dir,valor,0);
}

//Funcion poke normal de la cpu
void poke_byte_z88(z80_int dir,z80_byte valor)
{

        //sumamos estados de escritura
        t_estados += 3;

	poke_byte_no_time_z88(dir,valor);

}

//Funcion peek normal de la cpu, sin contar t-estados
z80_byte peek_byte_no_time_z88(z80_int dir)
{
#ifdef EMULATE_VISUALMEM
        set_visualmemreadbuffer(dir);
#endif
	return poke_peek_byte_no_time_z88(dir,0,1);
}


//Funcion peek normal de la cpu
z80_byte peek_byte_z88(z80_int dir)
{

	//sumamos estados de lectura
        t_estados +=3;

        return peek_byte_no_time_z88(dir);

}


//z80_byte temp_fila_leida_antes_de_snooze;
//z80_byte temp_byte_leido_core_z88_antes_de_snooze;

void z88_awake_from_coma(void)
{
	z88_coma.v=0;
}

void z88_enable_coma(void)
{
	z88_coma.v=1;
}

void z88_awake_from_snooze(void)
{
	z88_snooze.v=0;
}

void z88_enable_snooze(void)
{
        z88_snooze.v=1;
}


z80_byte z88_return_keyboard_port_value(z80_byte puerto_h)
{

                //si estamos en el menu, no devolver tecla
                if (zxvision_key_not_sent_emulated_mach() ) return 255;


                //Si esta spool file activo, generar siguiente tecla
                if (input_file_keyboard_is_playing() ) {
                        input_file_keyboard_get_key();
                }


                        z80_byte acumulado=255;

                        if ((puerto_h&128)==0) {
                                acumulado &=blink_kbd_a15;
                        }

                        if ((puerto_h&64)==0) {
                                acumulado &=blink_kbd_a14;
                        }

                        if ((puerto_h&32)==0) {
                                acumulado &=blink_kbd_a13;
                        }

                        if ((puerto_h&16)==0) {
                                acumulado &=blink_kbd_a12;
                        }

                        if ((puerto_h&8)==0) {
                                acumulado &=blink_kbd_a11;
                        }

                        if ((puerto_h&4)==0) {
                                acumulado &=blink_kbd_a10;
                        }

                        if ((puerto_h&2)==0) {
                                acumulado &=blink_kbd_a9;
                        }

                        if ((puerto_h&1)==0) {
                                acumulado &=blink_kbd_a8;
                        }


	return acumulado;

}

int antes_top_speed_real_frames=0;


//Envio de maskable al leer teclado. Pero con top speed esto no se lleva bien, intentar enviar maskable
//solo en determinadas ocasiones pero esto no funciona bien
void z88_generar_maskable_si_top_speed(void)
{
                                        if (timer_condicion_top_speed()) {
                                                int dif=top_speed_real_frames-antes_top_speed_real_frames;
						if (dif<0) dif=-dif;
                                                //printf ("antes %d ahora %d dif %d\n",antes_top_speed_real_frames,top_speed_real_frames,dif);
                                                if (dif>0) {
                                                        antes_top_speed_real_frames=top_speed_real_frames;
                                                        interrupcion_maskable_generada.v=1;
                                                        //printf ("Generar interrupcion de lectura puerto\n");
                                                }
                                        }
                                        else {
                                                interrupcion_maskable_generada.v=1;
                                        }


}

void z88_set_low_battery(void)
{
    blink_sta |=8;

    printf("blink_sta %02XH blink_int %02XH\n",blink_sta,blink_int);

    //blink_sta |=32;

    //blink_sta |=128;

    //con esto aparece low battery pero se cuelga
    //blink_sta &=(255-1);

    /*
    open flap
    	blink_sta |=128+32;
	blink_sta &=(255-1);

    close flap
    	//Notificar cierre tapa
	blink_sta &=(255-128-32);


    The STA ($B1) register provides information about which interrupt has actually occurred:

BIT         NAME        Function
7           FLAPOPEN    If set, flap open else flap closed
6           A19         If set, high level on A19 occurred during coma
5           FLAP        If set, positive edge has occurred on FLAPOPEN
4           UART        If set, an enabled UART interrupt is active
3           BTL         If set, battery low pin is active
2           KEY         If set, a column has gone low in snooze (or coma)
1           -           -
0           TIME        If set, an enabled TIME interrupt is active
    */
}

z80_byte lee_puerto_z88_no_time(z80_byte puerto_h,z80_byte puerto_l)
{

	debug_fired_in=1;
	z80_byte acumulado;


        switch (puerto_l) {


                case 0xb1:

			//temp bateria baja - battery low. lo fuerzo pero no aparece
			//blink_sta |=8;


                        return blink_sta;
                break;

                case 0xb2:
                        //debug_printf (VERBOSE_DEBUG,"puerto teclado. reg_i=%d",reg_i);

			//printf ("leemos puerto teclado. blink int: %d\n",blink_int);

                        //teclado
                        //en puerto_h fila



			acumulado=z88_return_keyboard_port_value(puerto_h);

			if (!snooze_desactivado) {
		        	if ( ((blink_int & BM_INTKWAIT) != 0) ) {

                                	z88_enable_snooze();

	                                //debug_printf (VERBOSE_DEBUG,"nos vamos a snooze");
        	                        //printf ("nos vamos a snooze. blink_int=%d\n",blink_int);

					//temp_fila_leida_antes_de_snooze=puerto_h;
					//temp_byte_leido_core_z88_antes_de_snooze=byte_leido_core_z88;

					z88_generar_maskable_si_top_speed();
					

			
					//cuando se vuelve de snooze, el valor devuelto a sentencia IN al despertar, no importa	
					//acumulado=255;

                	        }

			}

			else {
				//temp. no nos vamos a snooze y notificamos interrupcion. si activo snooze, el teclado
				//responde de manera no continua y los juegos se para la musica a no ser que pulse una tecla
				z88_generar_maskable_si_top_speed();
				//if (blink_int!=187) printf ("blink_int=%d pc=%d\n",blink_int,reg_pc);

			}



			return acumulado;
		break;

                case 0xb5:
			//debug_printf (VERBOSE_DEBUG,"devolvemos valor %d para puerto 0xb5",blink_tsta);
                        return blink_tsta;
                break;

                case 0xd0:
                case 0xd1:
                case 0xd2:
                case 0xd3:
                case 0xd4:
                        return blink_tim[puerto_l-0xd0];
                break;


		case 0xe0:
			return blink_rxd;
		break;

                case 0xe1:
                        return blink_rxe;
                break;



		case 0xe5:
			return blink_uit;
		break;


                default:
                        //debug_printf (VERBOSE_DEBUG,"devolvemos valor 0 para puerto 0x%x",puerto_l);
                        return 0;
                break;


        }

        return 0;
}


z80_byte lee_puerto_z88(z80_byte puerto_h,z80_byte puerto_l)
{
  z80_int port=value_8_to_16(puerto_h,puerto_l);
  ula_contend_port_early( port );
  ula_contend_port_late( port );
  z80_byte valor = lee_puerto_z88_no_time( puerto_h, puerto_l );

  t_estados++;

  return valor;

}



void out_port_z88_no_time(z80_int puerto,z80_byte value)
{
	debug_fired_out=1;
        z80_byte puerto_l=value_16_to_8l(puerto);
        z80_byte puerto_h=value_16_to_8h(puerto);

        z80_int valor16=value_8_to_16(puerto_h,value);

        //debug_printf (VERBOSE_DEBUG,"Out port 0x%x. value=0x%x pc=0x%x",puerto,value,reg_pc);
        switch (puerto_l) {
                /*
$70         PB0, pixel base reg.0   -
$71         PB1, pixel base reg.1   -
$72         PB2, pixel base reg.2   -
$73         PB3, pixel base reg.3   -
$74         SBR, screen base reg.   -

                */

                case 0x70:
                case 0x71:
                case 0x72:
                case 0x73:
                        blink_pixel_base[puerto_l-0x70]=valor16;
                        //debug_printf (VERBOSE_DEBUG,"------establecer pb%d con valor: 0x%x",puerto_l-0x70,valor16);
                break;

                case 0x74:
                        blink_sbr=valor16;
                        //debug_printf (VERBOSE_DEBUG,"------establecer sbr con valor: 0x%x",valor16);
                break;

                case 0xB0:

			if (value & 16) {
				//reset RTC
				blink_tim[0]=0;
				blink_tim[1]=0;
				blink_tim[2]=0;
				blink_tim[3]=0;
				blink_tim[4]=0;
			}
				

                        //debug_printf (VERBOSE_DEBUG,"establecer puerto b0 a valor: 0x%x",value);

			/*
7           SRUN        Speaker source (0=SBIT, 1=TxD or 3200hz)
6           SBIT        SRUN=0: 0=low, 1=high; SRUN=1: 0=3200 hz, 1=TxD
			*/

			//Para el detector de silencio. Ver si bit 7 o 6 cambian
			if (  (blink_com&(128+64)) != (value&(128+64))  ) {
				//Cambio. posible sonido
				silence_detection_counter=0;
				beeper_silence_detection_counter=0;
			}
			
                        blink_com=value;

			set_value_beeper_on_array(z88_get_beeper_sound() );

                break;

		case 0xB1:
			blink_int=value;
			//debug_printf (VERBOSE_DEBUG,"establecer puerto b1 a valor: 0x%x",value);
			//printf ("establecer blink_int a valor: %02XH\n",value);
		break;


		case 0xB3:
			blink_epr=value;
		break;

		case 0xB4:

			//realmente blink_tack no se usa para nada, solo para debug
			//blink_tack=value;

			//ozvm lo hace de esta manera pero no se porque:
        		//if ((blink_tsta & value) != 0) blink_tsta &=~value;

			blink_tsta &=~value;

		break;

		case 0xB5:
			blink_tmk=value;
		break;

		case 0xB6:
			//ozvm lo hace de esta manera pero no se porque:
		        //if ((blink_sta & value ) == 0) blink_sta &= ~value;

            //printf("antes B6 blink_sta=%02XH. value=%02XH\n",blink_sta,value);
			blink_sta &= ~value;
            //printf("despues B6 blink_sta=%02XH\n",blink_sta);

		break;


                case 0xD0:
                case 0xD1:
                case 0xD2:
                case 0xD3:
                        blink_mapped_memory_banks[puerto_l-0xD0]=value;
                        //debug_printf(VERBOSE_DEBUG,"Mapping bank 0x%x on segment %d",value,puerto_l-0xD0);
                break;


		case 0xE2:
			blink_rxc=value;
		break;

		case 0xE3:
			blink_txd=value;
		break;

		case 0xE4:
			blink_txc=value;
		break;

		case 0xE5:
			blink_umk=value;
		break;

		case 0xE6:
			value=value ^255;	
			blink_uit &=value;
		break;

                default:
                        debug_printf(VERBOSE_DEBUG,"Out unknown port 0x%0x value 0x%0x",puerto,value);
                break;
        }
}




void out_port_z88(z80_int puerto,z80_byte value)
{
  ula_contend_port_early( puerto );
  out_port_z88_no_time(puerto,value);
  ula_contend_port_late( puerto ); t_estados++;
}




void temp_z88_write_port(void)
{
	char buffer[10];
	int valor;

	printf ("port? en hexa y minus\n");
	scanf("%s",buffer);

	printf ("valor? en decimal\n");
	scanf ("%d",&valor);

	if (!strcmp(buffer,"d0")) {
		printf ("Estableciendo puerto d0 con valor %d\n",valor);
		blink_tim[0]=valor;
	}

        else if (!strcmp(buffer,"b1")) {
                printf ("Estableciendo puerto b1 con valor %d\n",valor);
                blink_sta=valor;
        }

        else if (!strcmp(buffer,"e5")) {
                printf ("Estableciendo puerto e5 con valor %d\n",valor);
                blink_uit=valor;
        }

        else if (!strcmp(buffer,"e0")) {
                printf ("Estableciendo puerto e0 con valor %d\n",valor);
                blink_rxd=valor;
        }

        else if (!strcmp(buffer,"b5")) {
                printf ("Estableciendo puerto b5 con valor %d\n",valor);
                blink_tsta=valor;
        }




	else {
		printf ("puerto desconocido\n");
		//sleep(2);
	}


}




void z88_notificar_tecla(void)
{

	//printf ("z88_notificar_tecla\n");

	//avisamos que ya no estamos en snooze
	if (z88_snooze.v) {
		z88_awake_from_snooze();
		//parece que al llegar aqui blink_int suele ser 187 -> 10111011 

		//truco
		//reg_a=z88_return_keyboard_port_value(temp_fila_leida_antes_de_snooze);

		//printf("salimos de snooze desde z88_notificar_tecla. blink_int: %d blink_sta: %d\n",blink_int,blink_sta);



		//Averiguar que instruccion habia hecho saltar esto
		//si un IN A,(N) o un IN reg,(C)

		/*

		switch (temp_byte_leido_core_z88_antes_de_snooze) {
			case 237:
				//Es un IN reg,(C)
				printf ("In con prefijo 237\n");
			break;

	

			case 219:
				//Es un IN A,(N)
				//reg_a=z88_return_keyboard_port_value(temp_fila_leida_antes_de_snooze);
				//printf ("devolvemos registro a=%d para fila %d\n",reg_a,temp_fila_leida_antes_de_snooze);
			break;
		}
		*/

		

	}



        if ((blink_int & BM_INTKEY) == BM_INTKEY && ((blink_sta & BM_STAKEY) == 0)) {
            // If keyboard interrupts are enabled, then signal that a key was pressed.

            if ((blink_int & BM_INTGINT) == BM_INTGINT) {
                // But only if the interrupt is allowed to escape the Blink...

                if (z88_coma.v == 1) {
                    // I register hold the address lines to be read

			//printf ("reg_i %d puerto: %d\n",z88_return_keyboard_port_value(reg_i),reg_i);

			if (z88_return_keyboard_port_value(reg_i)==z88_return_keyboard_port_value(reg_i) ) {
                    //if (keyboard.scanKeyRow(z80.I()) == z80.I()) {
                        z88_awake_from_coma();
                     }
                } else {
                    blink_sta |= BM_STAKEY;
                }

		//printf ("notificamos a blink_sta\n");

		//blink_sta |= BM_STAKEY;

                // Signal INT interrupt when KEY interrupts are enabled
		interrupcion_maskable_generada.v=1;
            }
        }

}

void z88_pausa_open_close_flap(void)
{
    //Ya no hace falta la pausa. Simplemente hacemos que el cierre de tapa va retrasado 1 segundo y se hace mediante el timer
    return;

	//2 segundos cada vez
	z88_contador_para_flap=0;


	//2 segundos, contador se incrementa cada 1/200 s
	unsigned int final_contador=z88_contador_para_flap+2*200;

	while (z88_contador_para_flap!=final_contador) {
		cpu_core_loop();
	}

}


void z88_open_flap(void)
{

	debug_printf (VERBOSE_DEBUG,"Open Z88 flap");

    //printf("Open flap\n");
    //debug_exec_show_backtrace();


	//este texto no se suele ver dado que casi siempre entra aqui con menu abierto y en esos casos no se muesta mensaje
	//screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Opening Flap");

	generic_footertext_print_operating("FLAP");

	/*
        if (((blink_int & BM_INTFLAP) == BM_INTFLAP) & ((blink_int & BM_INTGINT) == BM_INTGINT)) {
            blink_sta = (blink_sta | BM_STAFLAPOPEN | BM_STAFLAP) & ~BM_STATIME;
            z88_awake_from_coma();
            z88_awake_from_snooze();
        }
	*/


	//Notificar apertura tapa
	blink_sta |=BM_STAFLAPOPEN+BM_STAFLAP;
	blink_sta &=(255-1);

	z88_pausa_open_close_flap();

    /*
    En la apertura de flap se interesa que haga esto precisamente:
    - Abrir tapa
    - Ejecutar unos ciclos de cpu (z88_pausa_open_close_flap) para que el sistema se entere que se ha abierto la tapa
    */

   menu_footer_z88();

    //Reflejar cambios al abrir el flap
    menu_draw_ext_desktop();      

   //notificar apertura
   //iff1.v=1;
   //interrupcion_maskable_generada.v=1;

}

void z88_close_flap_ahora(void)
{
	debug_printf (VERBOSE_DEBUG,"Close Z88 flap");

    //printf("Close flap\n");
    //debug_exec_show_backtrace();

	//este texto no se suele ver dado que casi siempre entra aqui con menu abierto y en esos casos no se muesta mensaje
	//screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Closing Flap");


	z88_pausa_open_close_flap();

	//Notificar cierre tapa
	blink_sta &=(255-BM_STAFLAPOPEN-BM_STAFLAP);

	menu_footer_z88();

    //Reflejar cambios al abrir el flap
    menu_draw_ext_desktop();          

    /*
    En el cierre de flap se interesa que haga esto precisamente:
    - Ejecutar unos ciclos de cpu (z88_pausa_open_close_flap) para que el sistema se entere del evento que se ha producido: insertar/quitar cartucho, hard reset
    - Cerrar tapa
    */

   //interrupcion_maskable_generada.v=1;
}

int z88_pendiente_cerrar_tapa_timer=0;

void z88_close_flap(void)
{
    //Decir cerrar tapa en un segundo
    //TODO: realmente este valor se podria bajar a 10 y continuaria siendo seguro en apariencia,
    //pero por si acaso lo dejamos asi
    z88_pendiente_cerrar_tapa_timer=50;

}

void hard_reset_cpu_z88(void)
{
        z88_open_flap();


	//z80_int dir=0;
	z80_byte valor=0;

        //numero de banco 
	z80_byte bank=0x21;

        //Offset dentro del slot de memoria
	z80_long_int offset=0;

        //memory.setByte(0x210000, 0);    // remove RAM filing system tag 5A A5
        //memory.setByte(0x210001, 0);

	poke_peek_byte_no_time_z88_aux(bank,bank/0x40,offset,valor,0,0);
	poke_peek_byte_no_time_z88_aux(bank,bank/0x40,offset+1,valor,0,1);

        //Pulsar reset con tapa abierta
        reset_cpu();

	//Y cerrar tapa
        z88_close_flap();

}

void z88_flush_card_if_writable(int slot)
{
        //si habia una eprom o flash o hibdrida y en el slot 3, vaciarla a disco antes
        if (slot==3) {
                if (z88_memory_slots[slot].size!=0 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3 || z88_memory_slots[slot].type==4) ) {
                        debug_printf (VERBOSE_INFO,"Flush flash/eprom changes to disk if necessary before removing it");
                        z88_flush_eprom_or_flash_to_disk();
                }
        }

}



//Cargar tarjeta EPROM
//Formato puede ser:
//-Archivo epr: un solo archivo
//-archivos .62, .63 de 16kb cada uno, se cargan en orden, desde el numero mas bajo: 
// puede haber un solo .63 ,o un .62 y un .63, o varios desde .48 hasta el .63, etc...
void z88_load_eprom_card(char *archivo, int slot)
{

	if (slot<1 || slot>3) cpu_panic ("Invalid slot on load card");

	debug_printf(VERBOSE_INFO,"Inserting Z88 eprom card: %s on slot: %d",archivo,slot);


        //si habia una eprom o flash y en el slot 3, vaciarla a disco antes
	z88_flush_card_if_writable(3);
        //if (slot==3) { 
        //        if (z88_memory_slots[slot].size!=0 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3) ) {
        //                debug_printf (VERBOSE_INFO,"Flush flash/eprom changes to disk if necessary before removing it"); 
        //                z88_flush_eprom_or_flash_to_disk();
        //        }       
        //}   


	z88_open_flap();


	if (!util_compare_file_extension(archivo,"63")) {
		//archivos .63, .62, .61 etc  se cargan en orden empezando por el mas bajo.
		//ver el archivo mas bajo
		
		//obtenemos nombre archivo y directorio por separado
		char nombre[NAME_MAX];
		char nombre_sin_ext[NAME_MAX];

		//nombre sin directorio
		char nombre_bucle[NAME_MAX];
		//nombre con directorio
		char nombre_bucle_fullpath[NAME_MAX];

		char dir[PATH_MAX];

		util_get_dir(archivo,dir);
		util_get_file_no_directory(archivo,nombre);
		util_get_file_without_extension(nombre,nombre_sin_ext);

		debug_printf (VERBOSE_DEBUG,"File: %s dir: %s name: %s name_without_ext: %s",archivo,dir,nombre,nombre_sin_ext);


		//Ir a buscar el archivo con extension mas baja
		int contador=63;
		int salir=0;

		do {
			sprintf (nombre_bucle,"%s.%02d",nombre_sin_ext,contador);
			util_get_complete_path(dir,nombre_bucle,nombre_bucle_fullpath);
			debug_printf (VERBOSE_DEBUG,"Searching %s",nombre_bucle_fullpath);
			if (!si_existe_archivo(nombre_bucle_fullpath)) salir=1;
			if (!salir) {

				//ultimo valido=48. si existe el 47, mal asunto...
				if (contador==47) {
					debug_printf (VERBOSE_ERR,"Too many files. Exceed 256KB/16 files of 16KB size");
					return;
				}

				contador--;
			}
		} while (!salir);

		contador++;
		int total_archivos=64-contador;

		debug_printf (VERBOSE_DEBUG,"Last file: %s.%02d total files: %d",nombre_sin_ext,contador,total_archivos);

		//Controlar tamanyos conocidos: 32kb (2 archivos), 128kb (8), 256kb (16)
                if (total_archivos!=2 && total_archivos!=8 && total_archivos!=16) {
                        debug_printf (VERBOSE_ERR,"EPROM size not valid. Must be 32 KB, 128 KB or 256 KB");
                        return;
                }




		int offset=0;

		z80_long_int leidos;

		//indicamos lectura de archivo ok
		leidos=16384;

		while (contador<=63 && leidos==16384) {
                        sprintf (nombre_bucle,"%s.%02d",nombre_sin_ext,contador);
                        util_get_complete_path(dir,nombre_bucle,nombre_bucle_fullpath);

			debug_printf (VERBOSE_INFO,"Opening EPROM segment file %s at slot %d offset %d",nombre_bucle_fullpath,slot,offset);

	                FILE *ptr_romfile;
        	        ptr_romfile=fopen(nombre_bucle_fullpath,"rb");
	                //16384 bytes cada uno
	                leidos=fread(z88_puntero_memoria+z88_memory_slots[slot].offset_total+offset,1,16384,ptr_romfile);
        	        fclose(ptr_romfile);

			contador++;
			offset +=16384;
		}


		if (leidos!=16384) {
			debug_printf (VERBOSE_ERR,"Error reading file %s. Bytes read: %d",nombre_bucle_fullpath,leidos);
			//quitar slot que pudiera haber
		        z88_memory_slots[slot].size=0;
		        z88_memory_slots[slot].type=0;
		}

		else {

	                //metemos cartucho
			debug_printf (VERBOSE_INFO,"Setting EPROM card at slot %d with %d bytes length",slot,total_archivos*16384);
        	        z88_memory_slots[slot].size=(total_archivos*16384)-1;
                	z88_memory_slots[slot].type=2;

		}


                //guardamos nombre archivo
                strcpy(z88_memory_slots[slot].eprom_flash_nombre_archivo,archivo);



	}

	else if (!util_compare_file_extension(archivo,"epr")
	      || !util_compare_file_extension(archivo,"eprom")

		) {


        	//Controlar tamanyos conocidos.
	        int size_file=get_file_size(archivo);

        	if (size_file!=32*1024 && size_file!=128*1024 && size_file!=256*1024) {
                	debug_printf (VERBOSE_ERR,"EPROM size not valid. Must be 32 KB, 128 KB or 256 KB");
	                return;
        	}


		FILE *ptr_romfile;
		ptr_romfile=fopen(archivo,"rb");
		//maximo 1 MB
        	z80_long_int leidos=fread(z88_puntero_memoria+z88_memory_slots[slot].offset_total,1,1024*1024,ptr_romfile);
		fclose(ptr_romfile);

		//metemos cartucho 
		debug_printf (VERBOSE_INFO,"Setting EPROM card at slot %d with %d bytes length",slot,leidos);
		z88_memory_slots[slot].size=leidos-1;
        	z88_memory_slots[slot].type=2;


	        //guardamos nombre archivo
        	strcpy(z88_memory_slots[slot].eprom_flash_nombre_archivo,archivo);

	}

	else {
		debug_printf (VERBOSE_ERR,"Invalid Z88 eprom format file");
	}


	z88_close_flap();

	//reset_cpu();
}

//Insertar tarjeta EPROM
/*
void old_z88_load_eprom_card(char *archivo, int slot)
{

        if (slot<1 || slot>3) cpu_panic ("Invalid slot on load card");

	debug_printf(VERBOSE_INFO,"Inserting Z88 eprom card: %s on slot: %d",archivo,slot);


	//Controlar tamanyo archivo demasiado grande
	int size_file=get_file_size(archivo);
	if (size_file>1024*1024) {
		debug_printf (VERBOSE_ERR,"EPROM file size too big. Maximum 1 MB");
		return;
	}

	//Controlar tamanyos conocidos. 
	if (size_file!=32*1024 && size_file!=128*1024 && size_file!=256*1024) {
		debug_printf (VERBOSE_ERR,"EPROM size not valid. Must be 32 KB, 128 KB or 256 KB");
		return;
	}


        //si habia una eprom o flash y en el slot 3, vaciarla a disco antes
	z88_flush_card_if_writable(3);
        //if (slot==3) {
        //        if (z88_memory_slots[slot].size!=0 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3) ) {
        //                debug_printf (VERBOSE_INFO,"Flush flash/eprom changes to disk if necessary before removing it");
        //                z88_flush_eprom_or_flash_to_disk();
        //        }
        //}


        z88_open_flap();

        FILE *ptr_epromfile;
        ptr_epromfile=fopen(archivo,"rb");
        //maximo 1 MB
        z80_long_int leidos=fread(z88_puntero_memoria+z88_memory_slots[slot].offset_total,1,1024*1024,ptr_epromfile);
        fclose(ptr_epromfile);

        //metemos cartucho
        z88_memory_slots[slot].size=leidos-1;
        z88_memory_slots[slot].type=2;

        //guardamos nombre archivo
        strcpy(z88_memory_slots[slot].eprom_flash_nombre_archivo,archivo);

        z88_close_flap();

}
*/


//Insertar tarjeta Flash Intel
//Formato tiene que ser un unico archivo (normalmente con extension .flash)
void z88_load_flash_intel_card(char *archivo, int slot)
{
        
        if (slot<1 || slot>3) cpu_panic ("Invalid slot on load card");
        
        debug_printf(VERBOSE_INFO,"Inserting Z88 Intel Flash card: %s on slot: %d",archivo,slot);

        
        //Controlar tamanyo archivo demasiado grande
        int size_file=get_file_size(archivo);
        if (size_file>1024*1024) {
                debug_printf (VERBOSE_ERR,"Flash file size too big. Maximum 1 MB");
                return;
        }
        
        //Controlar tamanyos conocidos. 
        if (size_file!=512*1024 && size_file!=1024*1024) {
                debug_printf (VERBOSE_ERR,"Intel Flash size not valid. Must be 512 KB or 1MB");
                return;
        }

        //si habia una eprom o flash y en el slot 3, vaciarla a disco antes
	z88_flush_card_if_writable(3);
        //if (slot==3) {
        //        if (z88_memory_slots[slot].size!=0 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3) ) {
        //                debug_printf (VERBOSE_INFO,"Flush flash/eprom changes to disk if necessary before removing it");
        //                z88_flush_eprom_or_flash_to_disk();
        //        }
        //}

        
        z88_open_flap();
        
        FILE *ptr_flashfile;
        ptr_flashfile=fopen(archivo,"rb");
        //maximo 1 MB
        z80_long_int leidos=fread(z88_puntero_memoria+z88_memory_slots[slot].offset_total,1,1024*1024,ptr_flashfile);
        fclose(ptr_flashfile);

        
        //metemos cartucho
        z88_memory_slots[slot].size=leidos-1;
        z88_memory_slots[slot].type=3;

	z88_memory_slots[slot].flash_manufacturer_code=0x89;

	if (leidos==512*1024) z88_memory_slots[slot].flash_device_code=0xA7;
	else z88_memory_slots[slot].flash_device_code=0xA6;

        //guardamos nombre archivo
        strcpy(z88_memory_slots[slot].eprom_flash_nombre_archivo,archivo);
        
        z88_close_flap();

}



void z88_load_hybrid_eprom_card(char *archivo, int slot)
{

        if (slot<1 || slot>3) cpu_panic ("Invalid slot on load card");

        debug_printf(VERBOSE_INFO,"Inserting Z88 Hybrid RAM+Eprom card: %s on slot: %d",archivo,slot);


        //Controlar tamanyo archivo demasiado grande
        int size_file=get_file_size(archivo);
        if (size_file>512*1024) {
                debug_printf (VERBOSE_ERR,"Eprom file size too big. Maximum 512 KB");
                return;
        }

        //Controlar tamanyos conocidos.
        if (size_file!=512*1024) {
                debug_printf (VERBOSE_ERR,"Eprom size not valid. Must be 512 KB");
                return;
        }

        //si habia una eprom o flash y en el slot 3, vaciarla a disco antes
	z88_flush_card_if_writable(3);
        //if (slot==3) {
        //        if (z88_memory_slots[slot].size!=0 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3) ) {
        //                debug_printf (VERBOSE_INFO,"Flush flash/eprom changes to disk if necessary before removing it");
        //                z88_flush_eprom_or_flash_to_disk();
        //        }
        //}


        z88_open_flap();

        FILE *ptr_hybridfile;
        ptr_hybridfile=fopen(archivo,"rb");
        //maximo 512 KB. Leemos saltando los primeros 512 kb en slot destino
	z80_long_int leidos=fread(z88_puntero_memoria+z88_memory_slots[slot].offset_total+512*1024,1,512*1024,ptr_hybridfile);
        fclose(ptr_hybridfile);

	//no uso para nada leidos (de momento). Hago lo siguiente para que no se queje el compilador
	leidos++;
	leidos--;


        //metemos cartucho
        //z88_memory_slots[slot].size=leidos-1;
        z88_memory_slots[slot].size=1024*1024-1;
        z88_memory_slots[slot].type=4;

        /*z88_memory_slots[slot].flash_manufacturer_code=0x89;

        if (leidos==512*1024) z88_memory_slots[slot].flash_device_code=0xA7;
        else z88_memory_slots[slot].flash_device_code=0xA6;*/

        //guardamos nombre archivo
        strcpy(z88_memory_slots[slot].eprom_flash_nombre_archivo,archivo);

        z88_close_flap();

}




//Insertar tarjeta de RAM
//Tamanyo debe ser: 0,32768,65536,131072,262144,524288,1048576
void z88_insert_ram_card(int size,int slot)
{
	if (slot<1 || slot>3) cpu_panic ("Invalid slot on insert ram card");
	if (size<0 || size>1048576) cpu_panic ("Invalid slot size");


	//si habia una eprom o flash y en el slot 3, vaciarla a disco antes
	z88_flush_card_if_writable(3);
	//if (slot==3) {
	//	if (z88_memory_slots[slot].size!=0 && (z88_memory_slots[slot].type==2 || z88_memory_slots[slot].type==3) ) {
	//		debug_printf (VERBOSE_INFO,"Flush flash/eprom changes to disk if necessary before removing it");
	//		z88_flush_eprom_or_flash_to_disk();
	//	}
	//}

	z88_open_flap();

	z88_memory_slots[slot].type=0;

	//Ram size 0. no hay tarjeta
	if (size==0) z88_memory_slots[slot].size=0;

	else {
		z88_memory_slots[slot].size=size-1;

		//Inicializamos RAM a 255
        	int i;
	        z80_byte *puntero;

        	puntero=z88_puntero_memoria+z88_memory_slots[slot].offset_total;

	        for (i=0;i<size;i++) {
        	        *puntero=0;
                	puntero++;
		}
        }



	z88_close_flap();
}

//Inicializamos eprom o flash (en memoria) a 255. Luego el proceso de flush ya lo escribira a disco duro
void z88_erase_eprom_flash(void)
{

	int size=z88_memory_slots[3].size;

	//Ver si hay eprom/flash insertada en slot 3 y size!=0
	if (size==0) {
		debug_printf (VERBOSE_ERR,"Empty slot 3");
		return;
	}

	if (z88_memory_slots[3].type!=2 && z88_memory_slots[3].type!=3) {
		debug_printf (VERBOSE_ERR,"Slot 3 is not EPROM or Flash");
		return;
	}

	debug_printf (VERBOSE_INFO,"Erasing EPROM/Flash");


	z88_open_flap();

        //Inicializamos eprom a 255
        int i;
	z80_byte *puntero;

	size++;

	puntero=z88_puntero_memoria+z88_memory_slots[3].offset_total;

	//escribimos directo a memoria para no tener que poner las flash, por ejemplo, en modo programacion
        for (i=0;i<size;i++) {
                *puntero=255;
		puntero++;
        }

	//decimos que EPROM se debe escribir a disco
	z88_set_z88_eprom_or_flash_must_flush_to_disk();

	z88_close_flap();
}


void z88_flush_eprom_or_flash_to_disk_one_file(char *nombre,z80_byte *puntero,int size)
{
        FILE *ptr_epromfile;
        ptr_epromfile=fopen(nombre,"wb");


	int offset=0;
	//Si tipo ram+eprom, alterar offset
	if (z88_memory_slots[3].type==4) offset +=512*1024;


	if (ptr_epromfile==NULL) {
		debug_printf (VERBOSE_ERR,"Error writing file %s",nombre);
	}

	else {

	        fwrite(puntero+offset,1,size,ptr_epromfile);
        	fclose(ptr_epromfile);

	}

}


void z88_flush_eprom_or_flash_to_disk_63(int size)
{

	char *nombre_archivo=z88_memory_slots[3].eprom_flash_nombre_archivo;

        //Si tipo ram+eprom, alterar size
        if (z88_memory_slots[3].type==4) size /=2;


                //Ver cuantos archivos tiene que contener
                int archivos=(size/16384);

                int contador=64-archivos;

                //obtenemos nombre nombre_archivo y directorio por separado
                char nombre[NAME_MAX];
                char nombre_sin_ext[NAME_MAX];

                //nombre sin directorio
                char nombre_bucle[NAME_MAX];
                //nombre con directorio
                char nombre_bucle_fullpath[NAME_MAX];

                char directorio[PATH_MAX];

                util_get_dir(nombre_archivo,directorio);
                util_get_file_no_directory(nombre_archivo,nombre);
                util_get_file_without_extension(nombre,nombre_sin_ext);

                debug_printf (VERBOSE_DEBUG,"File: %s dir: %s name: %s name_without_ext: %s",nombre_archivo,directorio,nombre,nombre_sin_ext);




                int offset=0;

                while (contador<=63) {
                        sprintf (nombre_bucle,"%s.%02d",nombre_sin_ext,contador);
                        util_get_complete_path(directorio,nombre_bucle,nombre_bucle_fullpath);

                        debug_printf (VERBOSE_INFO,"Writing eprom segment file %s offset %d",nombre_bucle_fullpath,offset);


                
			z88_flush_eprom_or_flash_to_disk_one_file(nombre_bucle_fullpath,
				z88_puntero_memoria+z88_memory_slots[3].offset_total+offset,16384);



                        contador++;
                        offset +=16384;
                }


}


//Guardar cambios de memoria de slot3 eprom/flash al archivo de disco si esta en Z88 y hay cambios
void z88_flush_eprom_or_flash_to_disk(void)
{


	if (!MACHINE_IS_Z88) return;


        int size=z88_memory_slots[3].size;

        //Ver si hay eprom insertada en slot 3 y size!=0
        if (size==0) {
                return;
        }

	//Si no es eprom ni flash ni hibrida ram+eprom, volver
        if (z88_memory_slots[3].type!=2 && z88_memory_slots[3].type!=3 && z88_memory_slots[3].type!=4) {
                return;
        }



	if (z88_eprom_or_flash_must_flush_to_disk==0) {
		debug_printf (VERBOSE_DEBUG,"Trying to flush EPROM/FLASH to disk but no changes made");
		return;
	}

	debug_printf (VERBOSE_INFO,"Flushing EPROM/FLASH to disk");

	size++;

        //Si tipo ram+eprom, alterar size
        if (z88_memory_slots[3].type==4) size /=2;


        //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
        //metera flush a 1
	z88_eprom_or_flash_must_flush_to_disk=0;



	//Ver si extension eprom es .63
	if (!util_compare_file_extension(z88_memory_slots[3].eprom_flash_nombre_archivo,"63")) {
		//NOTA: Aunque en teoria permite grabar tarjetas hybridas eprom con extension .63, no lo permitimos al insertar.
		//Esas tarjetas solo pueden tener extension .epr y .eprom
		z88_flush_eprom_or_flash_to_disk_63(size);

	}


	else {

		//Eprom o flash es un solo archivo (.eprom o .epr o .flash)
		z88_flush_eprom_or_flash_to_disk_one_file(z88_memory_slots[3].eprom_flash_nombre_archivo,
			z88_puntero_memoria+z88_memory_slots[3].offset_total,size);
	}



}


//Crear un archivo (o archivos en el caso de .63) de eprom o flash, inicializando a 255 como una eprom vacia
//Formato .63 solo valido para eprom, no para flash. Flash solo extension .flash

//Retorna 0 si ok
//Diferente de 0: error
int z88_create_blank_eprom_flash_file(char *nombre_archivo,int size)
{

	z80_byte byte_eprom=255;

        FILE *ptr_epromfile;

	//Ver si extension eprom es .63
	if (!util_compare_file_extension(nombre_archivo,"63")) {
		//Crear archivos .63 de 16kb

		//Ver cuantos archivos tiene que contener
                int archivos=(size/16384);

                int contador=64-archivos;

                //obtenemos nombre nombre_archivo y directorio por separado
                char nombre[NAME_MAX];
                char nombre_sin_ext[NAME_MAX];

                //nombre sin directorio
                char nombre_bucle[NAME_MAX];
                //nombre con directorio
                char nombre_bucle_fullpath[NAME_MAX];

                char dir[PATH_MAX];

                util_get_dir(nombre_archivo,dir);
                util_get_file_no_directory(nombre_archivo,nombre);
                util_get_file_without_extension(nombre,nombre_sin_ext);

                debug_printf (VERBOSE_DEBUG,"File: %s dir: %s name: %s name_without_ext: %s",nombre_archivo,dir,nombre,nombre_sin_ext);


                while (contador<=63) {
                        sprintf (nombre_bucle,"%s.%02d",nombre_sin_ext,contador);
                        util_get_complete_path(dir,nombre_bucle,nombre_bucle_fullpath);

                        debug_printf (VERBOSE_INFO,"Writing eprom segment file %s",nombre_bucle_fullpath);
                                ptr_epromfile=fopen(nombre_bucle_fullpath,"wb");
                                int i;
                                for (i=0;i<16384;i++) {
                                        fwrite(&byte_eprom,1,1,ptr_epromfile);
                                }
                                fclose(ptr_epromfile);
			contador++;
		}


	}

	else {
		ptr_epromfile=fopen(nombre_archivo,"wb");


		if (ptr_epromfile==NULL) {
			debug_printf (VERBOSE_ERR,"Error creating card file");
			return 1;
		}


		debug_printf (VERBOSE_INFO,"Creating card file %s with size %d",nombre_archivo,size);
		int i;
                for (i=0;i<size;i++) {
			fwrite(&byte_eprom,1,1,ptr_epromfile);
	        }
                fclose(ptr_epromfile);
	}

	return 0;
}

void z88_change_internal_ram(int size)
{
	z88_internal_ram_size=size-1;

	hard_reset_cpu();
}

//incrementar puntero. si direccion=16384, aumentar bank
//funcion antigua cuando era necesario que dir (en funciones de peek/poke) siempre fuese 0...16383
/*
void temp_z88_increment_pointer(z88_dir *dir)
{
//	printf ("z88_increment_pointer before bank: %x dir: %x\n",dir->bank,dir->dir);

	dir->dir++;
	if (dir->dir==16384) {
		//saltamos de banco cuando es 16384 la direccion
		dir->bank +=1;
		dir->dir=0;
		//printf ("cambio bank. bank: %x dir: %x\n",dir->bank,dir->dir);
		//sleep(5);
	}

//	printf ("z88_increment_pointer after bank: %x dir: %x\n",dir->bank,dir->dir);
}
*/

//incrementar puntero. si direccion=0, aumentar bank +4
void z88_increment_pointer(z88_dir *dir)
{
//      printf ("z88_increment_pointer before bank: %x dir: %x\n",dir->bank,dir->dir);

        dir->dir++;
        if (dir->dir==0) {
                //saltamos de banco cuando es 0 la direccion
                dir->bank +=4;
                //dir->dir=0;
                //printf ("cambio bank. bank: %x dir: %x\n",dir->bank,dir->dir);
                //sleep(5);
        }

//      printf ("z88_increment_pointer after bank: %x dir: %x\n",dir->bank,dir->dir);
}




void z88_add_pointer (z88_dir *dir,z80_long_int size)
{
	//printf ("z88_add_pointer  before bank: %x dir: %x size: D\n",dir->bank,dir->dir,size);

	//printf ("z88_add_pointer size: %d\n",size);

	//Alguna comprobacion de maximo 4MB
	if (size>4*1024*1024) {
		//le decimos que se vaya a bank0, para "avisar" de que ha dado la vuelta
		dir->bank=0;
		return;
	}

	for (;size>0;size--) {
		//printf ("z88_add_pointer bank: %x dir: %x\n",dir->bank,dir->dir);
		z88_increment_pointer(dir);
	}

	//printf ("z88_add_pointer  after bank: %x dir: %x\n",dir->bank,dir->dir);
}



//Nota: las rutinas de manipulacion del filesystem eprom tambien sirven para flash
//Llena estructura file con el nombre al que apunta dir
//z88_dir queda incrementado y posicionado justo donde empiezan los datos
void z88_return_eprom_flash_file (z88_dir *dir,z88_eprom_flash_file *file)
{
	z80_byte namelength=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);

	file->namelength=namelength;

	
	if (namelength==255) return;

	z88_increment_pointer(dir);

	int i;

	//copiar nombre
	for (i=0;i<namelength;i++) {
		file->name[i]=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);
		z88_increment_pointer(dir);
	}	

	//copiar tamanyo
	for (i=0;i<4;i++) {
		file->size[i]=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);
		z88_increment_pointer(dir);
        }

	//Indicar donde estan los datos
	file->datos.bank=dir->bank;
	file->datos.dir=dir->dir;

}


//Nota: las rutinas de manipulacion del filesystem eprom tambien sirven para flash
//Llena estructura file con el nombre al que apunta dir
//z88_dir queda incrementado y posicionado justo donde empiezan los datos
//Misma funcion que z88_return_eprom_flash_file pero usando punteros a memoria
void z88_return_new_ptr_eprom_flash_file (z80_byte **ptr_dir,z88_eprom_flash_file *file)
{
	//z80_byte namelength=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);

	z80_byte *dir;
	dir=*ptr_dir;

	z80_byte namelength=*dir;

	file->namelength=namelength;

	
	if (namelength==255) return;

	//z88_increment_pointer(dir);
	dir++;

	int i;

	//copiar nombre
	for (i=0;i<namelength;i++) {
		//file->name[i]=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);
		file->name[i]=*dir;
		
		//z88_increment_pointer(dir);
		dir++;
	}	

	//copiar tamanyo
	for (i=0;i<4;i++) {
		//file->size[i]=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);
		file->size[i]=*dir;
		
		//z88_increment_pointer(dir);
		dir++;
        }

	//Indicar donde estan los datos
	//file->datos.bank=dir->bank;
	//file->datos.dir=dir->dir;

        //Atencion! Esta guardando en la estructura file el puntero hacia la direccion del nombre,
        //esto no tiene que suponer un problema siempre que no se liberen los datos del puntero ptr_dir (cosa que no debe suceder)
	file->datos_ptr=dir;

	*ptr_dir=dir;

}		


//Funcion auxiliar. Retorna tamanyo eprom/flash total (tamanyo EPROM/FLASH), ocupado (segun puntero actual dir) y disponible (restando incluso bytes de final)
//Retorna en variables de entrada total_eprom, used_eprom, free_eprom
void z88_write_eprom_flash_file_aux_return_free(z88_dir *dir, z80_long_int *total_eprom,z80_long_int *used_eprom, z80_long_int *free_eprom, int slot)
{
        //Tenemos puntero a datos, que es de SLOT1,2 o 3
        z80_byte used_bank_eprom=dir->bank-z88_get_bank_slot(slot);
        *used_eprom=(used_bank_eprom*16384)+dir->dir;

        debug_printf (VERBOSE_INFO,"Used Eprom/Flash bytes: %u",*used_eprom);

        //Tamanyo eprom total
        //Espacio libre

        *total_eprom=z88_memory_slots[slot].size+1;

        *free_eprom=*total_eprom-*used_eprom;

        //Bytes de final de EPROM
        *free_eprom -=64;

        debug_printf (VERBOSE_INFO,"Free Eprom/Flash bytes: %u",*free_eprom);

}

//Retorna tamanyo eprom/flash total (tamanyo EPROM/FLASH), ocupado (buscando el primer hueco libre), disponible (restando incluso bytes de final)
void z88_eprom_flash_free(z80_long_int *total_eprom,z80_long_int *used_eprom, z80_long_int *free_eprom,int slot)
{

        z88_dir dir;
        //z88_eprom_flash_file file;


        z88_find_eprom_flash_free_space(&dir,slot);


        z88_write_eprom_flash_file_aux_return_free(&dir, total_eprom, used_eprom, free_eprom, slot);

}

//Escribe archivo en EPROM/FLASH teniendo como entrada el puntero a siguiente espacio vacio en eprom, archivo en *file y datos a grabar
//En dir esta la primera direccion libre
//Retorna 0 si ok
//Retorna 1 si no hay espacio disponible
int z88_write_eprom_flash_file(z88_dir *dir,z88_eprom_flash_file *file,z80_byte *datos)
{

        z80_long_int size=file->size[0]+(file->size[1]*256)+(file->size[2]*65536)+(file->size[3]*16777216);



	//Comprobar si hay espacio en eprom/flash
	//Calcular espacio libre en eprom

	z80_long_int total_eprom,used_eprom, free_eprom;



	z88_write_eprom_flash_file_aux_return_free(dir, &total_eprom,&used_eprom, &free_eprom, 3);


	//Restamos espacio que ocupa la cabecera del nombre
	//4 bytes del tamanyo, 1 byte de longitud del nombre, longitud nombre
	free_eprom=free_eprom-4-1-file->namelength;


	//Comprobamos finalmente si cabe o no
	if (size>free_eprom) {
		debug_printf (VERBOSE_ERR,"Not enough free space on Card: Total Card: %d Used: %d Available: %d File Size: %d",
			total_eprom,used_eprom,free_eprom,size);
		return 1;
	}


	z80_byte blink_com_before=blink_com;
	blink_com |= 8;

	//si es tarjeta flash, activar modo programacion forzado, solo para aqui
	z88_flash_forced_writing_mode.v=1;

	int i;
	
	//Escribimos tamanyo nombre
	poke_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank,file->namelength);
	z88_increment_pointer(dir);

	//escribimos nombre
	for (i=0;i<file->namelength;i++) {
		poke_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank,file->name[i]);
		z88_increment_pointer(dir);
        }

	//escribimos tamanyo
	for (i=0;i<4;i++) {
		poke_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank,file->size[i]);
                z88_increment_pointer(dir);
        }


	//escribimos datos

	z80_byte byte_leido;

	for (;size>0;size--) {
		byte_leido=*datos;
		datos++;

		poke_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank,byte_leido);

		//printf ("escribiendo archivo bank %x dir %x\n",dir->bank,dir->dir);

                z88_increment_pointer(dir);
        }
	blink_com=blink_com_before;

	//printf ("fin escribir archivo\n");

	z88_flash_forced_writing_mode.v=0;

	return 0;
}	

//Escribe archivo en EPROM/FLASH de slot 3 teniendo el nombre y puntero a datos
//Tener en cuenta que nombre en eprom es sin ruta ni nada
//datos apunta a los datos a escribir
//Retorna 0 Si ok
//1 si nombre archivo muy corto
//2 si no hay espacio en eprom
int z88_eprom_flash_fwrite(char *nombre,z80_byte *datos,z80_long_int longitud)
{
        z88_dir dir;
        //z88_eprom_flash_file file;


        z88_find_eprom_flash_free_space(&dir,3);

	//printf ("z88_eprom_flash_fwrite. free space en bank %x dir %x\n",dir.bank,dir.dir);


        //Prueba escribir archivo

        //nombre de archivo minimo 4 caracteres contando la "/". Sino, el Filer no es capaz de hacer fetch. Porque??

	int namelength=strlen(nombre);

	if (namelength<4) {
		debug_printf (VERBOSE_ERR,"Name too short");
		return 1;
	}

        z88_eprom_flash_file archivo_salida;
        archivo_salida.namelength=namelength+1;
        archivo_salida.name[0]='/';

	int i;
	for (i=0;i<namelength;i++) {
		archivo_salida.name[1+i]=nombre[i];
	}

        archivo_salida.size[0]=longitud&0xFF;
	longitud >>=8;

        archivo_salida.size[1]=longitud&0xFF;
        longitud >>=8;

        archivo_salida.size[2]=longitud&0xFF;
        longitud >>=8;

        archivo_salida.size[3]=longitud&0xFF;


        int ret=z88_write_eprom_flash_file(&dir,&archivo_salida,datos);
	if (ret!=0) return 2;

	return 0;

}



//Imprime nombre de archivo eprom y su contenido, en pantalla. Para debug
void z88_debug_print_eprom_flash_file(z88_eprom_flash_file *file)
{

                        //Imprimir nombre, maximo 20 caracteres
			const unsigned int max_nombre=20;
			char buffer[max_nombre+1];

			//printf ("z88_debug_print_eprom_flash_file nombre: ");
                        unsigned int i;
			z80_byte caracter;
                        for (i=0;i<file->namelength && i<max_nombre;i++) {
				caracter=file->name[i];
				if (caracter<32 || caracter>127) caracter='.';
				buffer[i]=caracter;
                        }

			buffer[i]=0;

			debug_printf (VERBOSE_DEBUG,"Eprom/Flash File: %s",buffer);


                        //mostrar contenido
			
			//No. Volvemos sin mostrar contenido
			return;

                        z88_dir dirdatos;
                        dirdatos.bank=file->datos.bank;
                        dirdatos.dir=file->datos.dir;

			z80_long_int size=file->size[0]+(file->size[1]*256)+(file->size[2]*65536)+(file->size[3]*16777216);


                        printf ("size: %d\n",size);
                        printf ("size [4]: %d %d %d %d  (%d)\n",file->size[0],file->size[1],file->size[2],file->size[3],size);


                        for (i=0;i<size;i++) {
                                caracter=peek_byte_no_time_z88_bank_no_check_low(dirdatos.dir,dirdatos.bank);
                                z88_increment_pointer(&dirdatos);

                                if (caracter>31 && caracter<128) printf ("%c",caracter);
                                else printf (".");
                        }
                        printf ("\n");


}

z80_byte z88_get_bank_slot(int slot)
{

	z80_byte bank;

        switch (slot) {
                case 1:
                        bank=0x40;
                break;  
                
                case 2:
                        bank=0x80;
                break;  
              
                case 3:
                        bank=0xc0;
                break;  
                
                default:
                        cpu_panic("Invalid slot number on z88_get_bank_slot");

			//aunque aqui no se llega, solo lo dejo para el compilador no se queje de warning
			bank=0;
                break;  

        }

	return bank;


}

void z88_eprom_flash_find_init(z88_dir *dir,int slot)
{
        if (slot<1 || slot>3) cpu_panic("Invalid slot number on z88_find_eprom_flash_free_space");

        dir->bank=z88_get_bank_slot(slot);


        dir->dir=0;

}
 
//Retorna 0 si no hay mas archivos.
//Funcion similar a z88_eprom_flash_find_next pero las funciones usan punteros de memoria en vez de variables z88_dir
int z88_eprom_new_ptr_flash_find_next(z80_byte **ptr_dir,z88_eprom_flash_file *file)
{

		z80_byte *dir;
		dir=*ptr_dir;

		z80_byte *dir_tope; //Para controlar si puntero se sale de madre
		dir_tope=dir+1024*1024; //1 mb mas alla

                //z88_return_eprom_flash_file(dir,file);
                z88_return_new_ptr_eprom_flash_file(&dir,file);

                //el nombre al menos debe ocupar 1 byte
                if (file->namelength==0) {
                        debug_printf (VERBOSE_INFO,"Invalid EPROM/FLASH Card when getting free space");
                        return 0;
                }



                if (file->namelength==255) {
                        //printf ("no hay mas archivos. bank: %x dir: %x\n",dir->bank,dir->dir);
                }

                else {

                        z80_long_int size=file->size[0]+(file->size[1]*256)+(file->size[2]*65536)+(file->size[3]*16777216);

                        z88_debug_print_eprom_flash_file(file);



                        //siguiente direccion
                        dir=file->datos_ptr;
                        //dir->bank=file->datos.bank;
                        //dir->dir=file->datos.dir;

                        //printf ("z88_find_eprom_free_space size: %d\n",size);

                        //z88_add_pointer (dir,size);
                        dir +=size;

                        //Si hay una eprom corrupta, acabara pasando que excedera la memoria y el banco se ira a 0
                        //printf ("bank: %x\n",dir->bank);

                        if (dir>dir_tope) {
                        // TODO if (dir->bank < 0x40) {
                                debug_printf (VERBOSE_INFO,"Going beyond memory card");
                                return 0;
                        }

                }


                *ptr_dir=dir;


	if (file->namelength==255) return 0;
	else return 1;

}


//Retorna 0 si no hay mas archivos
int z88_eprom_flash_find_next(z88_dir *dir,z88_eprom_flash_file *file)
{

                z88_return_eprom_flash_file(dir,file);

                //el nombre al menos debe ocupar 1 byte
                if (file->namelength==0) {
                        debug_printf (VERBOSE_INFO,"Invalid EPROM/FLASH Card when getting free space");
                        return 0;
                }



                if (file->namelength==255) {
                        //printf ("no hay mas archivos. bank: %x dir: %x\n",dir->bank,dir->dir);
                }

                else {

                        z80_long_int size=file->size[0]+(file->size[1]*256)+(file->size[2]*65536)+(file->size[3]*16777216);

                        z88_debug_print_eprom_flash_file(file);



                        //siguiente direccion
                        dir->bank=file->datos.bank;
                        dir->dir=file->datos.dir;

                        //printf ("z88_find_eprom_free_space size: %d\n",size);

                        z88_add_pointer (dir,size);

                        //Si hay una eprom corrupta, acabara pasando que excedera la memoria y el banco se ira a 0
                        //printf ("bank: %x\n",dir->bank);

                        if (dir->bank < 0x40) {
                                debug_printf (VERBOSE_INFO,"Memory Bank < 40H when getting free space");
                                return 0;
                        }

                }


	if (file->namelength==255) return 0;
	else return 1;

}

void z88_eprom_flash_get_file_name(z88_eprom_flash_file *file,char *nombre)
{


                        const unsigned int max_nombre=Z88_MAX_CARD_FILENAME;

                        //printf ("z88_debug_print_eprom_flash_file nombre: ");
                        unsigned int i;
                        z80_byte caracter;
                        for (i=0;i<file->namelength && i<max_nombre;i++) {
                                caracter=file->name[i];
                                if (caracter<32 || caracter>127) caracter='.';
                                nombre[i]=caracter;
                        }

                        nombre[i]=0;

}






//Encontrar donde empieza el espacio vacio en la eprom, de slot indicado
void z88_find_eprom_flash_free_space (z88_dir *dir,int slot)
{
        z88_eprom_flash_file file;

	if (slot<1 || slot>3) cpu_panic("Invalid slot number on z88_find_eprom_flash_free_space");

	dir->bank=z88_get_bank_slot(slot);


        dir->dir=0;


        do {

	        z88_return_eprom_flash_file(dir,&file);

		//el nombre al menos debe ocupar 1 byte
		if (file.namelength==0) {
			debug_printf (VERBOSE_INFO,"Invalid EPROM/FLASH Card when getting free space");
			return;
		}

        

	        if (file.namelength==255) {
        	        //printf ("no hay mas archivos. bank: %x dir: %x\n",dir->bank,dir->dir);
	        }

        	else {

		        z80_long_int size=file.size[0]+(file.size[1]*256)+(file.size[2]*65536)+(file.size[3]*16777216);

			z88_debug_print_eprom_flash_file(&file);



		        //siguiente direccion
		        dir->bank=file.datos.bank;
		        dir->dir=file.datos.dir;

			//printf ("z88_find_eprom_free_space size: %d\n",size);

		        z88_add_pointer (dir,size);

			//Si hay una eprom corrupta, acabara pasando que excedera la memoria y el banco se ira a 0
			//printf ("bank: %x\n",dir->bank);

			if (dir->bank < 0x40) {
				debug_printf (VERBOSE_INFO,"Memory Bank < 40H when getting free space");
	                        return;
			}

	        }

        } while (file.namelength!=255);

}


//Encontrar un archivo concreto. Lo retorna en file. dir queda alterado. Especificar tambien la / del inicio
void z88_find_eprom_flash_file (z88_dir *dir,z88_eprom_flash_file *file,char *nombre, int slot)
{
                //z88_eprom_flash_file file;


        dir->bank=z88_get_bank_slot(slot);
        dir->dir=0;


        do {

                z88_return_eprom_flash_file(dir,file);

                //el nombre al menos debe ocupar 1 byte
                if (file->namelength==0) {
                        debug_printf (VERBOSE_INFO,"Invalid EPROM/FLASH Card when getting free space");
                        return;
                }

        

                if (file->namelength==255) {
                        //printf ("no hay mas archivos. bank: %x dir: %x\n",dir->bank,dir->dir);
                }

                else {

			//Ver si el nombre corresponde
			if (strlen(nombre)==file->namelength) {
				int i;
				for (i=0;i<file->namelength && file->name[i]==nombre[i];i++);

				//Si coincide, i=file.namelength
				if (i==file->namelength) return;
			}

                        z80_long_int size=file->size[0]+(file->size[1]*256)+(file->size[2]*65536)+(file->size[3]*16777216);

                        z88_debug_print_eprom_flash_file(file);

                        //siguiente direccion
                        dir->bank=file->datos.bank;
                        dir->dir=file->datos.dir;

                        //printf ("z88_find_eprom_free_space size: %d\n",size);

                        z88_add_pointer (dir,size);

                        //Si hay una eprom corrupta, acabara pasando que excedera la memoria y el banco se ira a 0
                        //printf ("bank: %x\n",dir->bank);

                        if (dir->bank < 0x40) {
                                debug_printf (VERBOSE_INFO,"Memory Bank < C0H when getting free space");
                                return;
                        }

                }

        } while (file->namelength!=255);

}


//Liberar espacio de eprom/flash por archivos borrados
//O tambien, recupera archivos borrados (les pone el / inicial)
//Retorna espacio liberado en el caso de reclaim, o total de archivos recuperados en el caso de recover
z80_long_int z88_eprom_flash_reclaim_free_space_and_recover(int recoverdeleted)
{

        if (!MACHINE_IS_Z88) return 0;


        int original_size=z88_memory_slots[3].size;

        //Ver si hay eprom insertada en slot 3 y size!=0
        if (original_size==0) {
                return 0;
        }

        //Si no es eprom ni flash, volver
        if (z88_memory_slots[3].type!=2 && z88_memory_slots[3].type!=3) {
                return 0;
        }


	//Puntero destino. 

	z88_dir dest_dir;

        dest_dir.bank=0xc0;
        dest_dir.dir=0;


	//Puntero origen.

        z88_dir orig_dir;

        orig_dir.bank=0xc0;
        orig_dir.dir=0;

	z80_byte byte_leido;

	byte_leido=peek_byte_no_time_z88_bank_no_check_low(orig_dir.dir,orig_dir.bank);

	//z80_byte longitud_nombre;
	z88_eprom_flash_file file;


        z80_byte blink_com_before=blink_com;
        blink_com |= 8;
        //si es tarjeta flash, activar modo programacion forzado, solo para aqui
        z88_flash_forced_writing_mode.v=1;


	int i;

	z88_dir datos;

	z80_long_int undeleted_total=0;


	//Hasta que leamos un FF al principio o agotemos tamanyo eprom
	while (original_size>0 && byte_leido!=0xFF) {
		//leer nombre
		z88_return_eprom_flash_file (&orig_dir,&file);

		z88_debug_print_eprom_flash_file(&file);

		z80_long_int file_size=file.size[0]+(file.size[1]*256)+(file.size[2]*65536)+(file.size[3]*16777216);

		//Por defecto no copiamos
		int copiamos=0;


                //Detectar si es archivo "/" nulo presente en las flash
                int archivonulo=0;
                //Ver si archivo borrado, nombre null, longitud nombre 1, longitud archivo 0
                if (file.name[0]==0 && file.namelength==1 && file_size==0) {
                         debug_printf (VERBOSE_DEBUG,"Do not touch null file '/' of the initialization of flash card");
                         archivonulo=1;
                }


		//Si es undelete
		if (recoverdeleted) {
			//en todos casos de undelete, recuperar. mas adelante se ve si es el archivo nulo, no poner el / inicial
			copiamos=1;
		}

		else {
			//es reclaim. si no esta borrado o es archivo nulo, copiarlo tal cual
			if (file.name[0]!=0 || archivonulo) {
				copiamos=1;
			}
		}



		//Copiar archivo tal cual o undelete
		if (copiamos) {

			if (recoverdeleted==0) debug_printf (VERBOSE_DEBUG,"Copying file to destination");
			//archivo no esta borrado
			//escribir en destino


			//si hay que recuperar, forzar siempre nombre archivo inicial con  / excepto con archivonulo
			if (recoverdeleted) {
				if (file.name[0]==0 && archivonulo==0) {
					file.name[0]='/';
					undeleted_total++;
					debug_printf (VERBOSE_DEBUG,"File is deleted. Recovering it");
				}
			}

		        //Escribimos tamanyo nombre
		        poke_byte_no_time_z88_bank_no_check_low(dest_dir.dir,dest_dir.bank,file.namelength);
		        z88_increment_pointer(&dest_dir);

		        //escribimos nombre
       			for (i=0;i<file.namelength;i++) {
		                poke_byte_no_time_z88_bank_no_check_low(dest_dir.dir,dest_dir.bank,file.name[i]);
	        	        z88_increment_pointer(&dest_dir);
		        }

       			//escribimos tamanyo
		        for (i=0;i<4;i++) {
               			poke_byte_no_time_z88_bank_no_check_low(dest_dir.dir,dest_dir.bank,file.size[i]);
		                z88_increment_pointer(&dest_dir);
		        }

			 //escribimos datos
			datos.bank=file.datos.bank;
			datos.dir=file.datos.dir;

			z80_long_int copia_file_size=file_size;
	
		        for (;copia_file_size>0;copia_file_size--) {
               			byte_leido=peek_byte_no_time_z88_bank_no_check_low(datos.dir,datos.bank);
		                z88_increment_pointer(&datos);

       	        		poke_byte_no_time_z88_bank_no_check_low(dest_dir.dir,dest_dir.bank,byte_leido);
               			z88_increment_pointer(&dest_dir);
			}

		}

		else {
			//No copiar archivo. Reclaim
		}

		//incrementar puntero origen
		z88_add_pointer(&orig_dir,file_size);

		//restar tamanyo leido, para controlar final
		//tamanyo - file_size - byte longitud nombre - longitud nombre - 4 bytes que indican tamanyo
		original_size=original_size-file_size-1-file.namelength-4;

		byte_leido=peek_byte_no_time_z88_bank_no_check_low(orig_dir.dir,orig_dir.bank);
	}

	z80_long_int total_liberados=0;

	//Si la eprom no estaba corrupta, original_size>=0

	//printf ("original_size: %d dest_dir.dir %d orig_dir.dir %d dest_dir.bank %d orig_dir.bank %d\n",original_size,
	//	dest_dir.dir,orig_dir.dir,dest_dir.bank,orig_dir.bank);

	if (original_size>=0) {
	//El hueco entre destino y origen, llenarlo con 0xFF
		while (!(dest_dir.dir == orig_dir.dir && dest_dir.bank == orig_dir.bank)) {
			poke_byte_no_time_z88_bank_no_check_low(dest_dir.dir,dest_dir.bank,0xFF);
			z88_increment_pointer(&dest_dir);
			total_liberados++;
		}

		debug_printf (VERBOSE_INFO,"Reclaimed %d bytes",total_liberados);
	}


        blink_com=blink_com_before;
        z88_flash_forced_writing_mode.v=0;

	if (recoverdeleted) return undeleted_total;

	return total_liberados;
	

}

//Liberar espacio de eprom/flash por archivos borrados
//Retorna espacio liberado.
z80_long_int z88_eprom_flash_reclaim_free_space(void)
{
	return z88_eprom_flash_reclaim_free_space_and_recover(0);
}

//Recupera archivos borrados de eprom/flash 
//Retorna archivos recuperados
z80_long_int z88_eprom_flash_recover_deleted(void)
{
        return z88_eprom_flash_reclaim_free_space_and_recover(1);
}


char z88_get_beeper_sound(void)
{

	if (blink_com & 64) return +100;
	else return -100;
}


//Aqui se llama desde cada driver scr cuando se pulsa una tecla
//Necesario para que juegos y programas siguientes no se cuelguen: Lemmings (justo al iniciar), Dstar (no lee ninguna tecla), Pipedream (al escribir muy rapido y hacer ENTER)
void notificar_tecla_interrupcion_si_z88(void)
{
	if (MACHINE_IS_Z88) {
		if (zxvision_key_not_sent_emulated_mach() ) {

		}

		else {

		//TODO
		//Solo se deberian enviar interrupciones cuando se permite (blink_int & BM_INTKEY) == BM_INTKEY
		//PERO si lo hago asi, estos juegos, pipedream y demas, se bloquean igualmente
		/*
		if ( (blink_int & BM_INTKEY) == BM_INTKEY) {
			debug_printf (VERBOSE_DEBUG,"Generate Maskable Interrupt to notify Z88 key press");
			interrupcion_maskable_generada.v=1;
		}
		else {
			debug_printf (VERBOSE_DEBUG,"Key press on Z88 but keyboard interrupts are not enabled");
		}
		*/


		//No compruebo si se permite interrupcion y la envio siempre que se pulsa tecla
		//debug_printf (VERBOSE_DEBUG,"Generate Maskable Interrupt to notify Z88 key press");
		interrupcion_maskable_generada.v=1;
		}
	}
}


//Funcion auxiliar que retorna cabecera de la tarjeta
void z88_return_card_type_aux(z88_dir *dir,char *header)
{

        *header=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);
	header++;
	z88_increment_pointer(dir);
        *header=peek_byte_no_time_z88_bank_no_check_low(dir->dir,dir->bank);
}


//Retorna tipo de eprom/flash insertada:
//-1: desconocido
//0: Solo aplicaciones
//1: Solo archivos
//2: Mixed: archivos y aplicaciones
//TODO: Tarjetas mixed no se detectan, tampoco se puede leer/escribir de ellas (en teoria)
int z88_return_card_type (int slot)
{

	char header[2];
	z88_dir dir;

        if (slot<1 || slot>3) cpu_panic("Invalid slot number on z88_return_card_type");

        dir.bank=z88_get_bank_slot(slot);

	//Situarse en el final de la tarjeta
	z80_long_int total_bancos=((z88_memory_slots[slot].size)+1)/16384;
	dir.bank +=total_bancos-1;


        dir.dir=16382;
	z88_return_card_type_aux(&dir,header);
	debug_printf (VERBOSE_DEBUG,"Z88 Card header: 0x%02X 0x%02X",header[0],header[1]);
	if (header[0]=='O' && header[1]=='Z') return 0;
	if (header[0]=='o' && header[1]=='z') return 1;


	return -1;
}

//Dice la cantidad de ram total de la maquina, contando memoria interna y cartuchos
int z88_get_total_ram(void)
{
    int total_ram=z88_internal_ram_size+1;

    int i;

    for (i=1;i<=3;i++) {

        if (z88_memory_slots[i].type==0) {
    	    total_ram +=z88_memory_slots[i].size+1;
        }
	}

    return total_ram;
}

int z88_flap_is_open(void)
{
    return (blink_sta & BM_STAFLAPOPEN ? 1 : 0);
}


//Establece el reloj del Z88 con el actual
void z88_set_system_clock_to_z88(void)
{
    z80_64bit segundos=util_get_seconds();

    /*
bank 20h
8040bh:

        uwSysDateLow            ds.w    1       ; 11 bytes buffer for system time  -> segundos??
        uwSysDateMid            ds.w    1
        uwSysDateHigh           ds.w    1

1 enero 0000: 00:00:00
es:

00229f2f9600H

1 enero 1970 00:00:00
es:

003118a41200H    


000000000000H = 23/11/4713 BC
    */

    segundos +=0x3118a41200;

    z80_int dir=0x40b;

    int i;

    //Modificar el reloj que ha establecido el usuario
    for (i=0;i<6;i++) {
        z80_byte valor=segundos & 0xFF;
        segundos = segundos >> 8;
        poke_byte_no_time_z88_bank_no_check_low(dir++,0x20,valor);
    }

    //Y timer a 0. Esto es lo que se incrementa por hardware,
    //en cambio el reloj establecido por el usuario no se incrementa.
    //Así el reloj que muestra el sistema es:
    // uwSysDate + TIM
    blink_tim[0]=blink_tim[1]=blink_tim[2]=blink_tim[3]=blink_tim[4]=0;
}
