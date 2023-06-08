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

#include "chardetect.h"

#include "cpu.h"
#include "operaciones.h"
#include "debug.h"
#include "screen.h"
#include "zxvision.h"
#include "textspeech.h"
#include "disassemble.h"
#include "settings.h"
#include "utils_text_adventure.h"

#include <string.h>
#include <stdio.h>




char *trap_char_detection_routines_texto[]={
        "None",
        "Auto&all",
        "AD Adv 42 ch",
        "Common 1",
        "Common 2",
        "Common 3",
        "Common 4",
        "Common 5",
        "Common 6",
        "Multiply 8"
};


//Rutina de deteccion. 0 si ninguna
int trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;


//en juegos de Aventuras AD 42 columnas
z80_byte detection_pattern_ad42[]={0xED, 0x5B, 0xF7, 0xFF, 0xF1, 0x6F, 0x26, 0x00, 0x29, 0x29, 0x29, 0x19, 0xD1, 0xD5, 0x3E, 0x08, 0x08, 0xE5};

//encontrado en sherlock
z80_byte detection_pattern_common_one[]={0xF5 ,0xC5 ,0xD5 ,0xE5 ,0x6F ,0x26,0x00 ,0x29 ,0x29 ,0x29 };

//encontrado en redmoon
z80_byte detection_pattern_common_two[]={0xD6, 0x20 , 0x5F , 0x16 , 0x00, 0xcb, 0x23, 0xcb, 0x12, 0xcb, 0x23, 0xcb, 0x12, 0xcb, 0x23, 0xcb, 0x12, 0x21,
0x00,0x3d,0x19 };

//encontrado en lancelot
z80_byte detection_pattern_common_three[]={ 0xB7 , 0xC8 , 0xF5 , 0x2A ,0x73, 0xF7 , 0xCD ,0xAD, 0xF9,  0x3E ,0x46,  0x12 , 0xF1 , 0xC9 };

//encontrado en gremlins
z80_byte detection_pattern_common_four[]={ 0xF5 , 0xFE ,0x5B , 0x28 ,0x10 , 0xFE ,0x5C , 0x28 ,0x0C , 0x3A ,0x54 ,0x63 , 0xFE ,0x29 , 0xCA ,0xDB ,0x6A , 0x3C , 0x32 ,0x54 ,0x63 , 0x3A ,0x54 ,0x63 };

//encontrado en perseus & andromeda
z80_byte detection_pattern_common_five[]={ 0xED,0x5B,0x38,0x63,0x01,0x30,0x60,0xD6,0x20,0x26,0x00,0x6F,0x29,0x29,0x29,0x09,0xEB,0xE5,0x3A,0x3B,0x63,0xE6,0x03,0xFE,0x03 };

//encontrado en mordon's quest
z80_byte detection_pattern_common_six[]={ 0xE5,0xD5,0xC5,0xFE,0x0D,0x28,0x6B,0x6F,0x26,0x00,0xED,0x5B,0x36,0x5C,0x29,0x29,0x29,0x19,0xEB };

//rutina comun de multiplicar por ocho
z80_byte detection_pattern_multiply_eight[]={ 0x29, 0x29, 0x29 };


//Segundo Trap de envio de caracter
z80_int chardetect_second_trap_char_dir=0;

//Tercer Trap de envio de caracter. El hobbit por ejemplo usa el segundo y el tercero
z80_int chardetect_third_trap_char_dir=0;


//Si tiene que sumar +32 al caracter que lee second trap
z80_bit chardetect_second_trap_sum32={0};
z80_byte chardetect_char_filter=0;

//ignorar saltos de linea
z80_bit chardetect_ignore_newline={0};


//rangos iniciales minimo y maximo
int chardetect_second_trap_detect_pc_min=65535;
int chardetect_second_trap_detect_pc_max=0;
//numero de veces que se ha leido que una rutina escribe en pantalla
int chardetect_debug_poke_display_num=0;

int chardetect_char_detection_automatic_finding_range=0;

int chardetect_second_trap_sum32_counter=0;


z80_byte chardetect_line_width=5;
int chardetect_x_position=0;


//si el ancho de linea antes de cortar debe esperar a un espacio de separacion de palabras o una coma o un ;
z80_bit chardetect_line_width_wait_space={0};

//si el ancho de linea antes de cortar debe esperar a un punto
z80_bit chardetect_line_width_wait_dot={1};


//Si hay que llamar a rutinas de deteccion de caracteres
z80_bit chardetect_detect_char_enabled={0};

//Si hay que llamar a rutinas de impresion de caracteres: de rst16&oz rom, second trap, third trap
z80_bit chardetect_printchar_enabled={0};

//Si en vez de hacer trap en rst16, se hace trap en rutina final de rom, que va bien para escribir numeros, pero no para aventuras por ejempo de paws
z80_bit chardetect_rom_compat_numbers={0};


//Mantener cada nombre sin espacios, asi en --experthelp se visualiza bien cada uno separado por espacios
char *chardetect_char_filter_names[]={
        "None",
        "Generic",
        "AD-42-char",
        "PAWS",
        "Hobbit"
};

void charfilter_print_list(void)
{
    int i;

    for (i=0;i<CHAR_FILTER_TOTAL;i++) {
        printf("%s ",chardetect_char_filter_names[i]);
    }
}

//Retorna no 0 si no existe char filter
int charfilter_set(char *s)
{
    int i;

    for (i=0;i<CHAR_FILTER_TOTAL;i++) {
        if (!strcasecmp(s,chardetect_char_filter_names[i])) {
            chardetect_char_filter=i;
            return 0;
        }
    }    

    return 1;
}

int chardetect_detect_trap_aux(z80_byte *pat,int length)
{

        //si reg_pc en rom, no hacer
        if (reg_pc<16384) return 0;

        z80_int index=0;

        for (;length>0;length--,index++) {
                if (peek_byte_no_time(reg_pc+index)!=pat[index]) return 0;
        }

        return 1;
}

void chardetect_disassemble_trozo(z80_int dir)
{

        if (verbose_level>=VERBOSE_DEBUG) {
                //Y debugamos un trozo
                int i;
                char buffer[33];

                size_t longitud_opcode;


                for (i=0;i<20;i++) {
                        debugger_disassemble(buffer,32,&longitud_opcode,dir);
                        printf ("%d ",dir);
                        //volcado hexa
                        unsigned int j;
                        for (j=0;j<longitud_opcode;j++) {
                                printf ("%02X ",peek_byte_no_time(dir));
                                dir++;
                        }

                        printf ("\t%s\n",buffer);
                        //dir +=longitud_opcode;
                }
        }

}

void chardetect_print_splash_detected(void)
{
	char buffer[100];

	sprintf (buffer,"Detected routine at %d using %s method",reg_pc,trap_char_detection_routines_texto[trap_char_detection_routine_number]);
	printf ("%s\n",buffer);
	textspeech_print_speech(buffer);

	if (scr_putpixel!=NULL) {
		screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer);
	}
}


//Deteccion de rutinas de caracteres
//Impresion de caracter. Rutina que llama es driver->detectedchar_pritn
//Speech del caracter si conviene


//Mira si hay que detectar rutina de impresion de caracteres de second trap
void chardetect_detect_char(void)
{



	
	
	if (chardetect_second_trap_char_dir==0 && trap_char_detection_routine_number && reg_pc>=chardetect_second_trap_detect_pc_min && reg_pc<=chardetect_second_trap_detect_pc_max) {
		
		
		
		switch (trap_char_detection_routine_number) {
			case TRAP_CHAR_DETECTION_ROUTINE_AD42:
				//detectar esto:
				/*				
				 * 
				 * ED 5B F7 FF LD DE,(FFF7)
				 * F1 POP AF
				 * 6F LD L,A   <-Aqui registro A tiene el caracter a imprimir
				 * 26 00 LD H,00
				 * 29 ADD HL,HL
				 * 29 ADD HL,HL
				 * 29 ADD HL,HL
				 * 19 ADD HL,DE
				 * D1 POP DE
				 * D5 PUSH DE
				 * 3E 08 LD A,08
				 * 08 EX AF,AF'
				 * E5 PUSH HL
				 * 
				 */
				
				if (chardetect_detect_trap_aux(detection_pattern_ad42,sizeof(detection_pattern_ad42)) ) {
					chardetect_print_splash_detected ();
					chardetect_disassemble_trozo(reg_pc);
					chardetect_second_trap_char_dir=reg_pc+5;
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
					//chardetect_line_width=42;
					chardetect_char_filter=CHAR_FILTER_AD42;
					chardetect_line_width_wait_space.v=0;
                    chardetect_line_width_wait_dot.v=1;
                    chardetect_ignore_newline.v=1;
				}		
				
				break;
				
				//Observado en: Sherlock
			case TRAP_CHAR_DETECTION_ROUTINE_COMMON_ONE:
				//detectar esto:
				/*
				 * 
				 * 50056 F5 PUSH AF
				 * 50057 C5 PUSH BC
				 * 50058 D5 PUSH DE
				 * 50059 E5 PUSH HL
				 * 50060 6F LD L,A
				 * 50061 26 00 LD H,00
				 * 50063 29 ADD HL,HL
				 * 50064 29 ADD HL,HL
				 * 50065 29 ADD HL,HL
				 */
				
				if (chardetect_detect_trap_aux(detection_pattern_common_one,sizeof(detection_pattern_common_one)) ) {		
					chardetect_print_splash_detected ();
					chardetect_disassemble_trozo(reg_pc);
					chardetect_second_trap_char_dir=reg_pc;
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
					//chardetect_line_width=32;
					chardetect_char_filter=CHAR_FILTER_GENERIC;
					chardetect_line_width_wait_space.v=0;
                    chardetect_line_width_wait_dot.v=1;
                    chardetect_ignore_newline.v=1;
				}
				
				break;
				
				
				//Observado en: Red Moon
			case TRAP_CHAR_DETECTION_ROUTINE_COMMON_TWO:
				//detectar esto:
				/*
				 * 63092 D6 20 SUB 20
				 * 
				 * 63094 5F        LD E,A
				 * 63095 16 00     LD D,00
				 * 63097 CB 23     SLA E
				 * 63099 CB 12     RL D
				 * 63101 CB 23     SLA E
				 * 63103 CB 12     RL D
				 * 63105 CB 23     SLA E
				 * 63107 CB 12     RL D
				 * 63109 21 00 3D  LD HL,3D00
				 * 63112 19        ADD HL,DE
				 * 
				 * 
				 */
				
				if (chardetect_detect_trap_aux(detection_pattern_common_two,sizeof(detection_pattern_common_two)) ) {
					chardetect_print_splash_detected ();
					chardetect_disassemble_trozo(reg_pc);
					chardetect_second_trap_char_dir=reg_pc;
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
					//chardetect_line_width=32;
					chardetect_char_filter=CHAR_FILTER_GENERIC;
					chardetect_line_width_wait_space.v=0;
                    chardetect_line_width_wait_dot.v=1;
                    chardetect_ignore_newline.v=1;
				}
				
				break;
				
				//Observado en: Lancelot
			case TRAP_CHAR_DETECTION_ROUTINE_COMMON_THREE:
				//detectar esto:
				/*
				 * 
				 * 63928 B7        OR A
				 * 63929 C8        RET Z
				 * 63930 F5        PUSH AF
				 * 63931 2A 73 F7  LD HL,(F773)
				 * 63934 CD AD F9  CALL F9AD
				 * 63937 3E 46     LD A,46
				 * 63939 12        LD (DE),A
				 * 63940 F1        POP AF
				 * 63941 C9        RET
				 * 
				 */
				
				if (chardetect_detect_trap_aux(detection_pattern_common_three,sizeof(detection_pattern_common_three)) ) {
					chardetect_print_splash_detected ();
					chardetect_disassemble_trozo(reg_pc);
					chardetect_second_trap_char_dir=reg_pc;
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
					//chardetect_line_width=32;
					chardetect_char_filter=CHAR_FILTER_GENERIC;
					chardetect_line_width_wait_space.v=0;
                    chardetect_line_width_wait_dot.v=1;
                    chardetect_ignore_newline.v=1;
				}
				
				break;
				
				
			case TRAP_CHAR_DETECTION_ROUTINE_COMMON_FOUR:
				//detectar esto:
				/*
				 * 
				 * 27512 F5        PUSH AF
				 * 27513 FE 5B     CP 5B
				 * 27515 28 10     JR Z,6B8D
				 * 27517 FE 5C     CP 5C
				 * 27519 28 0C     JR Z,6B8D
				 * 27521 3A 54 63  LD A,(6354)
				 * 27524 FE 29     CP 29
				 * 27526 CA DB 6A  JP Z,6ADB
				 * 27529 3C        INC A
				 * 27530 32 54 63  LD (6354),A
				 * 27533 3A 54 63  LD A,(6354)
				 * 
				 */
				
				if (chardetect_detect_trap_aux(detection_pattern_common_four,sizeof(detection_pattern_common_four)) ) {
					chardetect_print_splash_detected ();
					chardetect_disassemble_trozo(reg_pc);
					chardetect_second_trap_char_dir=reg_pc;
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
					//chardetect_line_width=42;
					chardetect_char_filter=CHAR_FILTER_GENERIC;
					chardetect_line_width_wait_space.v=0;
                    chardetect_line_width_wait_dot.v=1;
                    chardetect_ignore_newline.v=1;
				}
				
				break;
	
			case TRAP_CHAR_DETECTION_ROUTINE_COMMON_FIVE:
				//detectar esto:
				/*
				 * 
				 * 26999 ED 5B 38 63       LD DE,(6338)
				 * 27003 01 30 60  LD BC,6030
				 * 27006 D6 20     SUB 20
				 * 27008 26 00     LD H,00
				 * 27010 6F        LD L,A
				 * 27011 29        ADD HL,HL
				 * 27012 29        ADD HL,HL
				 * 27013 29        ADD HL,HL
				 * 27014 09        ADD HL,BC
				 * 27015 EB        EX DE,HL
				 * 27016 E5        PUSH HL
				 * 27017 3A 3B 63  LD A,(633B)
				 * 27020 E6 03     AND 03
				 * 27022 FE 03     CP 03
				 * 
				 * 
				 */

				if (chardetect_detect_trap_aux(detection_pattern_common_five,sizeof(detection_pattern_common_five)) ) {
					chardetect_print_splash_detected ();
					chardetect_disassemble_trozo(reg_pc);
					chardetect_second_trap_char_dir=reg_pc;
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
					//chardetect_line_width=42;
					chardetect_char_filter=CHAR_FILTER_GENERIC;
					chardetect_line_width_wait_space.v=0;
                    chardetect_line_width_wait_dot.v=1;
                    chardetect_ignore_newline.v=1;
				}
				
				break;
			
				
			case TRAP_CHAR_DETECTION_ROUTINE_COMMON_SIX:
				//detectar esto:
				/*                                      
				 * 
				 * 62557 E5        PUSH HL
				 * 62558 D5        PUSH DE
				 * 62559 C5        PUSH BC
				 * 62560 FE 0D     CP 0D
				 * 62562 28 6B     JR Z,F4CF
				 * 62564 6F        LD L,A
				 * 62565 26 00     LD H,00
				 * 62567 ED 5B 36 5C       LD DE,(5C36)
				 * 62571 29        ADD HL,HL
				 * 62572 29        ADD HL,HL
				 * 62573 29        ADD HL,HL
				 * 62574 19        ADD HL,DE
				 * 62575 EB        EX DE,HL
				 * 
				 * 
				 */
				if (chardetect_detect_trap_aux(detection_pattern_common_six,sizeof(detection_pattern_common_six)) ) {
					chardetect_print_splash_detected ();
					chardetect_disassemble_trozo(reg_pc);
					chardetect_second_trap_char_dir=reg_pc;
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
					//chardetect_line_width=42;
					chardetect_char_filter=CHAR_FILTER_GENERIC;
					chardetect_line_width_wait_space.v=0;
                    chardetect_line_width_wait_dot.v=1;
                    chardetect_ignore_newline.v=1;
				}       
				
				break;
				
			case TRAP_CHAR_DETECTION_ROUTINE_MULTIPLY_EIGHT:
				//detectar esto:
				/*                                      
				 * 
				 * 62571 29        ADD HL,HL
				 * 62572 29        ADD HL,HL
				 * 62573 29        ADD HL,HL
				 * 62574 19        ADD HL,DE
				 * o
				 * 62574 09        ADD HL,BC
				 * 
				 * 
				 */
				
				
				//y siempre que reg_a sea ascii
				if (reg_a>31 && reg_a<127) {
					
					if (chardetect_detect_trap_aux(detection_pattern_multiply_eight,sizeof(detection_pattern_multiply_eight)) ) {
						
						//ver si ultima sentencia add hl,de o add hl,bc
						if (peek_byte_no_time(reg_pc+3)==0x09 || peek_byte_no_time(reg_pc+3)==0x19 ) {
							chardetect_print_splash_detected ();
							chardetect_disassemble_trozo(reg_pc-8);
							chardetect_second_trap_char_dir=reg_pc;
							trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;
							//chardetect_line_width=32;
							chardetect_char_filter=CHAR_FILTER_GENERIC;
							chardetect_line_width_wait_space.v=0;
                            chardetect_line_width_wait_dot.v=1;
                            chardetect_ignore_newline.v=1;
						}
					}       
				}
				
				break;

			case TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC:
				//printf ("modo automatico total\n");
				//detectar cualquiera de los patterns anteriores

				//siempre que ya tengamos un rango de direcciones delimitado
				if (chardetect_char_detection_automatic_finding_range==0) {

					//printf ("buscando direccion concreta para rango delimitado\n");

					//llamar a esta misma funcion alterando el valor de trap_char_detection_routine_number
					trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_AD42;



					//Probar todas las anteriores mientras no se encuentre second_trap_char_dir
					while (trap_char_detection_routine_number!=TRAP_CHAR_DETECTION_ROUTINES_TOTAL 
						&& chardetect_second_trap_char_dir==0) {
							//printf ("probando con rutina: %d\n",trap_char_detection_routine_number);
							chardetect_detect_char();
							trap_char_detection_routine_number++;
					}



					//si no lo ha encontrado, volver a dejarlo como estaba
					if (chardetect_second_trap_char_dir==0) trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC;

					//si lo ha encontrado, quitar el automatico
					else trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_NONE;

					

				}
		
			break;
				
		}
	}
}



//Inicializar algunas variables de las rutinas de deteccion de caracteres
void chardetect_init_trap_detection_routine(void)
{

        //Al establecer un trap, cambiamos direccion a 0, sino la deteccion de trap no funcionara
        chardetect_second_trap_char_dir=0;


        if (trap_char_detection_routine_number==TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC) chardetect_init_automatic_char_detection();
        else {
                //rangos abiertos
                chardetect_second_trap_detect_pc_min=23296;
                chardetect_second_trap_detect_pc_max=65535;
                //y rutina de poke normal
                chardetect_end_automatic_char_detection();
        }
}


//rutina de poke byte antes de cambiarla
//void (*original_char_detect_poke_byte)(z80_int dir,z80_byte valor);

int chardetect_automatic_nested_id_poke_byte;

void chardetect_set_poke_function(void)
{

       int activar=0;

        //Ver si ya no estaban activas. Porque ? Tiene sentido esto? Esto seguramente vino de diviface.c en que a veces se llama aqui
        //estando ya la intefaz activa. Pero quiza en este chardetect_automatic no sucedera nunca. Quitar esta comprobacion?
        if (poke_byte!=poke_byte_nested_handler) {
                debug_printf (VERBOSE_DEBUG,"poke_byte_nested_handler is not enabled calling chardetect_automatic_set_peek_poke_functions. Enabling");
                activar=1;
        }

        else {
                //Esta activo el handler. Vamos a ver si esta activo el chardetect_automatic dentro
                if (debug_nested_find_function_name(nested_list_poke_byte,"chardetect_automatic poke_byte")==NULL) {
                        //No estaba en la lista
                        activar=1;
                        debug_printf (VERBOSE_DEBUG,"poke_byte_nested_handler is enabled but not found chardetect_automatic poke_byte. Enabling");
                }

        }


        if (activar) {
                debug_printf (VERBOSE_DEBUG,"Setting chardetect_automatic poke / peek functions");
       		chardetect_automatic_nested_id_poke_byte=debug_nested_poke_byte_add(chardetect_automatic_poke_byte,"chardetect_automatic poke_byte");
                
        }


}

void chardetect_init_automatic_char_detection(void)
{

        if (trap_char_detection_routine_number==TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC) {
                chardetect_second_trap_sum32.v=0;
                chardetect_second_trap_sum32_counter=0;
                chardetect_debug_poke_display_num=0;
                chardetect_second_trap_detect_pc_min=65535;
                chardetect_second_trap_detect_pc_max=0;
		chardetect_char_detection_automatic_finding_range=1;

                if (MACHINE_IS_SPECTRUM) {


			//Si valor rutina poke no apunta a donde toca, establecerla
			//if (poke_byte!=poke_byte_spectrum_48k_chardetect_automatic_detect_char) {
			//	original_char_detect_poke_byte=poke_byte;
        	        //        poke_byte=poke_byte_spectrum_48k_chardetect_automatic_detect_char;
			//}

			chardetect_set_poke_function();

                        printf ("\nWARNING: Setting internal writing Spectrum operations to a slow function, to use Automatic character detection routine\n");
			if (scr_putpixel!=NULL) {
				screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Setting internal writing Spectrum operations to a slow function");
			}
                }
        }
}

void chardetect_end_automatic_char_detection(void)
{
        if (MACHINE_IS_SPECTRUM) {
		//Ya no hace falta comprobar esto. La funcion de debug_nested_poke_byte_del borra la funcion si la encuentra,
		//y si poke_byte no apunta a nested poke_byte_nested_handler, vuelve tal cual
		//temp if (poke_byte==poke_byte_nested_handler) {
			debug_nested_poke_byte_del(chardetect_automatic_nested_id_poke_byte);
			//poke_byte=original_char_detect_poke_byte;
	                printf ("Setting internal writing Spectrum operations to normal mode\n");
			screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Setting internal writing Spectrum operations to normal mode");
		//temp}
        }
	chardetect_char_detection_automatic_finding_range=0;
}



//se llama aqui desde poke_byte
void chardetect_debug_char_table_routines_poke(z80_int dir)
{

        //Siempre que rutina de deteccion sea automatica
        if (trap_char_detection_routine_number!=TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC) return;


        //Para saber quien escribe en la pantalla. Tercio inferior de la pantalla

        if (chardetect_debug_poke_display_num<MAX_DEBUG_POKE_DISPLAY && reg_pc>16383) {

                //Rutina de impresion en pantalla tiene que estar alojado a partir de la 23296
                if (dir>=16384+4096 && dir<=22527 && reg_pc>=23296) {
                        //printf ("accessing display: %d reg_pc: %d ",dir,reg_pc);

                        z80_int pc_min=reg_pc-500;
                        if (pc_min<chardetect_second_trap_detect_pc_min) chardetect_second_trap_detect_pc_min=pc_min;

                        z80_int pc_max=reg_pc+500;
                        if (pc_max>chardetect_second_trap_detect_pc_max) chardetect_second_trap_detect_pc_max=pc_max;


                        //printf ("range routine: %d-%d\n",chardetect_second_trap_detect_pc_min,chardetect_second_trap_detect_pc_max);

                        chardetect_debug_poke_display_num++;

                        if (chardetect_debug_poke_display_num==MAX_DEBUG_POKE_DISPLAY) {
                                //trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_MULTIPLY_EIGHT;

                                printf ("\nAutomatic char detection routine: Range routine is: %d-%d. Guessing exact routine using all methods\n",chardetect_second_trap_detect_pc_min,chardetect_second_trap_detect_pc_max);
                                chardetect_end_automatic_char_detection();
                        }


                }

        }

}


//Para hacer debug de aventuras de texto, investigar desde donde se estan leyendo las tablas de caracteres
//se llama aqui desde peek_byte
void chardetect_debug_char_table_routines_peek(z80_int dir)
{

        //Para poder debugar rutina que imprima texto. Util para aventuras conversacionales del CAAD

        //Para ver quien accede a tabla de caracteres que hay hacia la 0xF000

        /*
         *      if (dir>0xF000 && reg_pc!=65525) {
         *              printf ("dir: %d reg_pc: %d\n",dir,reg_pc);
}
*/

        /*
         *      //pruebas hobbit
         *      if (dir>0x7890 && dir<0x7A00) {
         *              printf ("dir: %d reg_pc: %d\n",dir,reg_pc);
}
*/

        /*
         *      //Para saber quien accede a la tabla de caracteres de la ROM
         *      if (dir>=0x3d00 && dir<=0x3fff) {
         *              printf ("accessing rom char table dir: %d reg_pc: %d\n",dir,reg_pc);
}
*/


        //Para que no se queje el compilador de no usado
        dir++; dir--;

}

//Para poder debugar rutina que imprima texto. Util para aventuras conversacionales
//se llama aqui desde core_spectrum
void chardetect_debug_print_char_routine(void)
{

        //Sabiendo que la rutina esta entre la 31000 y la 32000, capturar cuando reg_a='a' o reg_a='b', para localizarlo facilmente
        //se escribe una a o una b en el juego y aparece esto

        //normal para aventuras AD
        //if (reg_pc>30300 && reg_pc<31300 ) {

        //para sherlock
        //if (reg_pc>50050 && reg_pc<50090) {

        //para redmoon
        //if (reg_pc>63000 && reg_pc<63200) {

        //para lancelot
        //if (reg_pc>=63928 && reg_pc<63940) {

        //para gremlins
        //if (reg_pc>=27490 && reg_pc<27520) {

        //Para mordons quest
        /*if (reg_pc>=27850 && reg_pc<27999) {
         *
         *
         *              if (reg_a=='A' || reg_a=='B') {
         *                      printf ("reg_pc: %d reg_a: %d (%c)\n",reg_pc,reg_a,reg_a);
         *
         *                      //Y debugamos un trozo
         *                      chardetect_disassemble_trozo(reg_pc-4);
         *
}
}
*/
}






//z80_byte temp_antes_c;
z80_bit chardetect_printchar_ignorar_siguiente={0};
int chardetect_printchar_letras_e_seguidas=0;

void chardetect_printchar_caracter_imprimible(z80_byte c)
{
	
    //Escribirlo en consola
	scr_detectedchar_print(c);
	chardetect_x_position++;
	

    //printf ("caracter: %d\n",c);
    //Y pasarlo al buffer de speech
	textspeech_add_character(c);


    //Pasarlo al buffer de localizationes
    textadv_location_add_char(c);

    //printf("chardetect_line_width %d\n",chardetect_line_width);
	
	if (chardetect_line_width) {
        //printf("chardetect_x_position %d\n",chardetect_x_position);
		if (chardetect_x_position>=chardetect_line_width) {
			int saltar=0;

            //printf("caracter: %c\n",c);
			
			//con texto justificado, no cortar si no hay espacio o , o ; o .
			if (chardetect_line_width_wait_space.v==1 || chardetect_line_width_wait_dot.v==1) {
                if (chardetect_line_width_wait_space.v) {
				    if (c==' ' || c==',' || c==';') saltar=1;
                }

                if (chardetect_line_width_wait_dot.v==1) {
                    if (c=='.') saltar=1;
                }                  
			}

            else saltar=1;
						
			if (saltar) {
				chardetect_x_position=0;

                //No queremos forzar salto de linea en consola
                //esto se enviaba en versiones previas a 9.3
				//scr_detectedchar_print ('\n');


                //enviar speech a consola debug window si conviene
                //if (textspeech_get_stdout.v) {
                //    textspeech_add_speech_fifo_debugconsole_yesno(1);
                //}

				textspeech_add_speech_fifo();
			}
		}
		
	}

	else {
		//printf ("width es 0.\n");
		//if (c==13) printf ("salto de linea\n");
	}
	
}

//Caracter anterior siempre que sea imprimible
z80_byte chardetect_printchar_caracter_anterior=0;


void chardetect_printchar_espacio_si_mayus(z80_byte c)
{
	//Si hay una mayuscula, y antes no habia mayuscula, meter espacio antes
	if (c>='A' && c<='Z' && !(chardetect_printchar_caracter_anterior>='A' && chardetect_printchar_caracter_anterior<='Z')) {
		//printf ("-caracter anterior: %d (%c)-",chardetect_printchar_caracter_anterior,chardetect_printchar_caracter_anterior);
		chardetect_printchar_caracter_imprimible(' ');
	}
	
}



z80_byte chardetect_printchar_caracter_gestion_filtros(z80_byte c)
{
	
	//Gestionar filtros
	switch (chardetect_char_filter) {
		//Ninguno
		case CHAR_FILTER_NONE:
			
			break;
			
			//Generico
		case CHAR_FILTER_GENERIC:
			//Si hay una mayuscula, y antes no habia mayuscula, meter espacio antes
			//chardetect_printchar_espacio_si_mayus(c);
			
			//Despues de punto, ","  o ";" espacio o simbolos de interrogacion/exclamacion
			if (c=='.' || c==',' || c==';' || c=='?' || c=='!') {
				chardetect_printchar_caracter_imprimible(c);
				c=' ';
			}

			//Ignorar control de tinta
			if (c==16) chardetect_printchar_ignorar_siguiente.v=1;
			//Ignorar control de papel
			if (c==17) chardetect_printchar_ignorar_siguiente.v=1;
			
			break;
			
			//Aventuras AD
		case CHAR_FILTER_AD42:		
			
			
			
			//Acentuadas. De momento las retornamos tal cual sin acentos
			/*
			if (c=='\x15') {
				//printf ("antes: %d\n",temp_antes_c);
				c='a';
				
			}
			
			if (c=='\x16') c='e';
			if (c=='\x17') c='i';
			if (c=='\x18') c='o';
			if (c=='\x19') c='u';

			//eñe
			if (c=='\x1a') c='n';
			*/

			c=chardetect_convert_daad_accents(c);
			
			//ignorar "_"
			if (c=='_') c=0;

			//ignorar ">"
			if (c=='>') c=0;
			
			if (c==8 || c==9) chardetect_printchar_ignorar_siguiente.v=1;
			
			//Si hay una mayuscula, y antes no habia mayuscula, meter espacio antes
			//chardetect_printchar_espacio_si_mayus(c);
			
			//Despues de punto, ","  o ";" espacio o simbolos de interrogacion/exclamacion
			if (c=='.' || c==',' || c==';' || c=='?' || c=='!') {
				chardetect_printchar_caracter_imprimible(c);
				c=' ';
			}
			
			
			//Diosa cozumel genera texto "eeeeeeeeee" a veces. ignorar 3 seguidos
			if (c>31 && c<127 && chardetect_printchar_ignorar_siguiente.v==0) {
				if (c=='e') {
					chardetect_printchar_letras_e_seguidas++;
					//printf (" letra e. conta: %d\n ",chardetect_printchar_letras_e_seguidas);
					if (chardetect_printchar_letras_e_seguidas>=3) c=0;
				}
				else {
					//if (chardetect_printchar_letras_e_seguidas) printf (" no letra e. conta: %d letra: %d\n ",chardetect_printchar_letras_e_seguidas,c);
					chardetect_printchar_letras_e_seguidas=0;
				}
			}
			
			break;
			
		case CHAR_FILTER_PAWS:
			
            c=chardetect_convert_paws_accents(c);
	

			//simbolos de cursores y cosas que no interesa que no se vean

			//Visto en el anillo, cursores >> y <<
			//if (c=='^') c=13;
			//if (c=='^') c=0;
			//if (c=='`') c=0;

			//abrir corchete es abrir exclamacion. poner espacio
			if (c=='[') c=32;
			
			//cerrar corchete es abrir interrogacion. poner espacio
			if (c==']') c=32;

			//salto de linea
			if (c==7) c=13;

                        //Ignorar control de tinta
                        if (c==16) chardetect_printchar_ignorar_siguiente.v=1;
                        //Ignorar control de papel
                        if (c==17) chardetect_printchar_ignorar_siguiente.v=1;

			
			//Si hay una mayuscula, meter espacio antes
			//chardetect_printchar_espacio_si_mayus(c);
			
			//Despues de punto, ","  o ";" espacio o simbolos de interrogacion/exclamacion
			if (c=='.' || c==',' || c==';' || c=='?' || c=='!') {
				
				chardetect_printchar_caracter_imprimible(c);
				c=' ';
			}
			
			
			break;
			
		case CHAR_FILTER_HOBBIT:
			
			//Eliminar simbolo "+"
			if (c=='+') c=0;
			
			//Si hay una mayuscula, meter espacio antes
			//chardetect_printchar_espacio_si_mayus(c);
			
			//Despues de punto, ","  o ";" espacio o simbolos de interrogacion/exclamacion
			if (c=='.' || c==',' || c==';' || c=='?' || c=='!') {
				chardetect_printchar_caracter_imprimible(c);
				c=' ';
			}
			
			break;
			
	}
	
	
	return c;
	
}	

void chardetect_printchar_caracter(z80_byte c)
{
	
	if (chardetect_printchar_ignorar_siguiente.v) {
		chardetect_printchar_ignorar_siguiente.v=0;
		c=0;
	}
	
	//Debug para ver caracteres especiales
	//Parece ser que el juego "El anillo" utiliza caracteres > 128 (por ejemplo 201) para hacer espacios a final de linea
	//O quiza es que los caracteres > 128 son tokens (conjuntos de letras) que salen del compresor de texto del PAWS
	//if (c>=32 && c<=126) printf (" %c ",c);
	//else 
 	//printf (" %d=%c ",c,(c>31 && c<128 ? c : '.' ));


	c=chardetect_printchar_caracter_gestion_filtros(c);
	
	//printf (" %d=%c ",c,(c>31 && c<128 ? c : '.' ));	
	
	//temp_antes_c=c;

	
	
	if (c!=0) {
		//Codigo 22 (AT) equivaldra a mismo salto de linea
		if (c==22) c=13;

        //Codigo 127 es (c). Cambiarlo
        if (c==127) c='C';


        //ignorar saltos de linea
        //printf("Ignoramos salto de linea\n");
        if (chardetect_ignore_newline.v && c==13) c=32;        


		if (c>31 && c<127) {
			chardetect_printchar_caracter_imprimible(c);
			chardetect_printchar_caracter_anterior=c;
		}
	
		//salto de linea 
		else if (c==13) {
			if (chardetect_line_width) {
				scr_detectedchar_print ('\n');
				chardetect_x_position=0;
			
				textspeech_add_speech_fifo();
			}
			else {
				//Si line width 0, meter un espacio al recibir un salto de linea
				chardetect_printchar_caracter_imprimible(' ');
				chardetect_printchar_caracter_anterior=c;
	                }
			
		}

		//Retroceder un caracter
		else if (c==8) {		
			scr_detectedchar_print (8);
			if (chardetect_x_position) chardetect_x_position--;


        		textspeech_add_character(c);

			
		}
		
		else {
			debug_printf(VERBOSE_DEBUG,"Unknown character 0x%02X",c);
		}
	}
	
	//flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
	//fflush(stdout);
	
	
}


//Imprime caracter si es que estamos en trap de rst16 (o oz rom), second trap o third trap
void chardetect_printchar(void)
{
	
	
	
	//Primer trap rst y z88 rom calls
	screen_text_printchar(chardetect_printchar_caracter);

	//Segundo tipo de trap
	if (chardetect_second_trap_char_dir) {
		if (reg_pc==chardetect_second_trap_char_dir) {

			//printf ("coincide reg pc con rutina second\n");
			
			//Si leemos algun espacio, quiere decir que tenemos que sumar 32
			//solo durante los primeros caracteres
			if (chardetect_second_trap_sum32_counter<MAX_STDOUT_SUM32_COUNTER && chardetect_second_trap_sum32.v==0) {
				chardetect_second_trap_sum32_counter++;
				if (reg_a==0) {
					chardetect_second_trap_sum32.v=1;
					printf ("\nAutomatic char detection routine sets sum 32 to character\n");
					//Cuando lo hace una vez, ya no hacer mas, a no ser que se inicialice la deteccion automatica
					chardetect_second_trap_sum32_counter=MAX_STDOUT_SUM32_COUNTER;
				}
				
			}

            z80_byte caracter_final=reg_a;
			
			if (chardetect_second_trap_sum32.v) {
                caracter_final +=32;
            }
			
			screen_text_printchar_next(caracter_final,chardetect_printchar_caracter);
			
			
			return;
		}
	}
	
	
	//Tercer tipo de trap
	if (chardetect_third_trap_char_dir) {
		if (reg_pc==chardetect_third_trap_char_dir) {
			screen_text_printchar_next(reg_a,chardetect_printchar_caracter);
		}
	}
	
}

