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

#include "scrsimpletext.h"
#include "debug.h"
#include "screen.h"
#include "mem128.h"
#include "zx8081.h"
#include "operaciones.h"
#include "cpu.h"
#include "utils.h"
#include "zxvision.h"
#include "joystick.h"
#include "ula.h"
#include "disassemble.h"
#include "z88.h"
#include "timer.h"
#include "chardetect.h"
#include "tsconf.h"
#include "settings.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "svi.h"
#include "textspeech.h" 

void scrsimpletext_repinta_pantalla(void);


//A 1 si se ha recibido el caracter "1" de escape y se espera siguiente valor para saber longitud
//A 2 hay que ignorar longitud de caracter escape
//int simpletext_z88_escape_caracter=0;
//int simpletext_z88_escape_caracter_longitud=0;





int simpletext_x_position=0;

#define SIMPLETEXT_IZQ_BORDER 4
#define SIMPLETEXT_TOP_BORDER 4



//Total lineas en buffer
#define MAX_LINES_BUFFER 50




void scrsimpletext_putpixel_final_rgb(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color_rgb GCC_UNUSED)
{
}

void scrsimpletext_putpixel_final(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color GCC_UNUSED)
{
}



//Rutina de putchar para menu
void scrsimpletext_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{
	
	//No escribimos nada. Este driver no soporta menu
	//Para evitar warnings al compilar de "unused parameter"
	x=y;
	y=x;
	caracter=tinta=papel;
	tinta=papel=caracter;
	
}

void scrsimpletext_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel)
{
	
	//Para evitar warnings al compilar de "unused parameter"
	x=y;
	y=x;
	caracter=tinta=papel;
	tinta=papel=caracter;
	
}




void scrsimpletext_set_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scrsimpletext_reset_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scrsimpletext_z88_cpc_load_keymap(void)
{
	debug_printf (VERBOSE_INFO,"Loading keymap");
}

void scrsimpletext_detectedchar_print(z80_byte caracter)
{
        printf ("%c",caracter);
        //flush de salida standard
        fflush(stdout);

}

//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrsimpletext_get_menu_width(void)
{
        return 32;
}


int scrsimpletext_get_menu_height(void)
{
        return 24;
}

/*
int scrsimpletext_driver_can_ext_desktop (void)
{
        return 0;
}
*/

void scrsimpletext_textspeech_filter_welcome_message(void)
{

	char texto_welcome[40];
	sprintf(texto_welcome," Welcome to ZEsarUX v." EMULATOR_VERSION " ");
	textspeech_print_speech(texto_welcome); 

	char texto_edition[40];
	sprintf(texto_edition," " EMULATOR_EDITION_NAME " ");
	textspeech_print_speech(texto_edition);	

	
}

int scrsimpletext_init (void){ 
	
	debug_printf (VERBOSE_INFO,"Init simpletext Video Driver"); 
	
	
	//Mismos mensajes de bienvenida a traves de filtro texto
	if (opcion_no_welcome_message.v==0) scrsimpletext_textspeech_filter_welcome_message();
	
	
	scr_debug_registers=scrsimpletext_debug_registers;
	scr_messages_debug=scrsimpletext_messages_debug;
	
	scr_putchar_menu=scrsimpletext_putchar_menu;
	scr_putchar_footer=scrsimpletext_putchar_footer;

	scr_putpixel_final=scrsimpletext_putpixel_final;
	scr_putpixel_final_rgb=scrsimpletext_putpixel_final_rgb;

        scr_get_menu_width=scrsimpletext_get_menu_width;
        scr_get_menu_height=scrsimpletext_get_menu_height;
	//scr_driver_can_ext_desktop=scrsimpletext_driver_can_ext_desktop;
	
	
	scr_set_fullscreen=scrsimpletext_set_fullscreen;
	scr_reset_fullscreen=scrsimpletext_reset_fullscreen;
	scr_z88_cpc_load_keymap=scrsimpletext_z88_cpc_load_keymap;
	scr_detectedchar_print=scrsimpletext_detectedchar_print;

        //por defecto activamos esto en simpletext
        chardetect_printchar_enabled.v=1;

	
	
	scr_set_driver_name("simpletext");
	
	screen_simpletext_driver=1;
	
	
	return 0; 
	
}

void scrsimpletext_end(void)
{
	
	debug_printf (VERBOSE_INFO,"Closing simpletext video driver");
	
	
}

void scrsimpletext_refresca_pantalla_solo_driver(void)
{
        //Como esto solo lo uso de momento para drivers graficos, de momento lo dejo vacio
}


void scrsimpletext_refresca_pantalla(void){}
z80_byte scrsimpletext_lee_puerto(z80_byte puerto_h,z80_byte puerto_l){
	
	//Para evitar warnings al compilar de "unused parameter"
	puerto_h=puerto_l;
	puerto_l=puerto_h;
	
	return 255;
}





void scrsimpletext_actualiza_tablas_teclado(void){
	
	
	//if (stdout_simpletext_automatic_redraw.v && (contador_segundo%200)==0) {
	if (stdout_simpletext_automatic_redraw.v && (contador_segundo%(20*scrstdout_simpletext_refresh_factor))==0) {
		scrsimpletext_repinta_pantalla();
	}
	
	
}
void scrsimpletext_debug_registers(void){

char buffer[2048];

print_registers(buffer);

printf ("%s\r",buffer);

}



void scrsimpletext_messages_debug(char *s)
{
	printf ("%s\n",s);
	//flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
	fflush(stdout);
	
	
}















/*
z80_byte temp_antes_c;
z80_bit scr_simpletext_printchar_ignorar_siguiente={0};
int scr_simpletext_printchar_letras_e_seguidas=0;

void scr_simpletext_printchar_caracter_imprimible(z80_byte c)
{
	printf ("%c",c);
	simpletext_x_position++;
	
	
	
	if (stdout_simpletext_line_width) {
		if (simpletext_x_position>=stdout_simpletext_line_width) {
			simpletext_x_position=0;
			printf ("\n");
		}
		
	}
	
}

//Caracter anterior siempre que sea imprimible
z80_byte scr_simpletext_printchar_caracter_anterior=0;


void scr_simpletext_printchar_espacio_si_mayus(z80_byte c)
{
	//Si hay una mayuscula, y antes no habia mayuscula, meter espacio antes
	if (c>='A' && c<='Z' && !(scr_simpletext_printchar_caracter_anterior>='A' && scr_simpletext_printchar_caracter_anterior<='Z')) {
		//printf ("-caracter anterior: %d (%c)-",scr_simpletext_printchar_caracter_anterior,scr_simpletext_printchar_caracter_anterior);
		scr_simpletext_printchar_caracter_imprimible(' ');
	}
	
}

void scr_simpletext_printchar_caracter(z80_byte c)
{
	
	if (scr_simpletext_printchar_ignorar_siguiente.v) {
		scr_simpletext_printchar_ignorar_siguiente.v=0;
		c=0;
	}
	
	
	if (c!=0) {
		if (c>31 && c<128) {
			scr_simpletext_printchar_caracter_imprimible(c);
			scr_simpletext_printchar_caracter_anterior=c;
		}
		
		else if (c==13) {
			printf ("\n");
			simpletext_x_position=0;
			
		}
		
		else {
			debug_printf(VERBOSE_DEBUG,"Unknown character 0x%02X",c);
		}
	}
	
	//flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
	fflush(stdout);
	
	
}



void scr_simpletext_printchar(void)
{
	//Primer trap rst y z88 rom calls
	screen_text_printchar(scr_simpletext_printchar_caracter);
}



*/







void scr_simpletext_common_fun_color(z80_byte atributo GCC_UNUSED,int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{

        if (!screen_text_accept_ansi)  return;
        int paper,ink;
        int copia_paper;


        ink = atributo & 7;
        paper = ( atributo >> 3 ) & 7;


        if (atributo & 128) {
                //hay parpadeo
                if (estado_parpadeo.v) {
                        //estado de inversion de color
                        copia_paper=paper;
                        paper=ink;
                        ink=copia_paper;
                }
        }

        //printf ("\x1b[H");
        if (atributo & 64) {
                ink +=8;
                paper +=8;
        }

        screen_text_set_ansi_color_fg(ink);
        screen_text_set_ansi_color_bg(paper);


}

void scr_simpletext_common_fun_caracter (int x GCC_UNUSED,int y GCC_UNUSED,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{
	printf ("%c",caracter);
}

void scr_simpletext_common_fun_saltolinea (void)
{
	printf ("\n");
}





void scrsimpletext_repinta_pantalla(void)
{


        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;


	
	//enviar Ansi inicio pantalla
	screen_text_send_ansi_go_home();
	
		 //si todo de pixel a ascii art
     if (rainbow_enabled.v && screen_text_all_refresh_pixel.v) {
     
scr_refresca_pantalla_tsconf_text(scr_simpletext_common_fun_color,scr_simpletext_common_fun_caracter,scr_simpletext_common_fun_saltolinea,screen_text_all_refresh_pixel_scale);
     
     }
	
	
	else if (MACHINE_IS_ZX8081) {
		screen_text_repinta_pantalla_zx81();
	}
	
	else if (MACHINE_IS_Z88) {
		//Si esta vofile activo, hay que dibujar dentro del buffer rainbow
		//Aqui se llama 5 veces por segundo
		//Aunque vofile_send_frame se llama desde core_z88.c,
		//si se genera un video de mas de 5 FPS, el video tendra correctamente los FPS que toquen,
		//pero apareceran frames repetidos, ya que estamos generando nueva imagen en rainbow solo 5 veces por segundo
		if (vofile_inserted.v) {
			set_z88_putpixel_zoom_function();
			screen_z88_refresca_pantalla_comun();
		}
		
		screen_text_repinta_pantalla_z88();
	}

        else if (MACHINE_IS_ACE) {
                screen_text_repinta_pantalla_ace();
        }

        else if (MACHINE_IS_SAM) {
                screen_text_repinta_pantalla_sam();
        }
        
        else if (MACHINE_IS_CPC) {
                screen_text_repinta_pantalla_cpc();
        }

        else if (MACHINE_IS_CHLOE) {
                screen_text_repinta_pantalla_chloe();
        }


	else if (MACHINE_IS_TSCONF) {

                //Si es modo texto, hacer este refresh:
                z80_byte modo_video=tsconf_get_video_mode_display();
                if (modo_video==3) {
                        scr_refresca_pantalla_tsconf_text_textmode(scr_simpletext_common_fun_color,scr_simpletext_common_fun_caracter,scr_simpletext_common_fun_saltolinea,12);
			sem_screen_refresh_reallocate_layers=0;
                        return;
                }


		//con rainbow
		if (rainbow_enabled.v) {
			scr_refresca_pantalla_tsconf_text(scr_simpletext_common_fun_color,scr_simpletext_common_fun_caracter,scr_simpletext_common_fun_saltolinea,12);
		}

		//sin rainbow, como spectrum
		else {
			screen_text_repinta_pantalla_spectrum();
		}
	}	
	
	
	else {
		//Refresco en Spectrum
		screen_text_repinta_pantalla_spectrum();
		
	}


	sem_screen_refresh_reallocate_layers=0;
	
	
}





