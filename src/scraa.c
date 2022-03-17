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
#include <aalib.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "scraa.h"
#include "screen.h"
#include "operaciones.h"
#include "mem128.h"
#include "debug.h"
#include "zx8081.h"
#include "audio.h"
#include "zxvision.h"
#include "utils.h"
#include "joystick.h"
#include "ula.h"
#include "z88.h"
#include "textspeech.h"
#include "settings.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "svi.h"


aa_context *context;
aa_renderparams *rparams;
int scraa_fast=1;
int scraa_imgwidth,scraa_imgheight;

int scraa_totalrealwidth;
int scraa_totalrealheight;
int paleta[EMULATOR_TOTAL_PALETTE_COLOURS];
unsigned char *framebuffer_aa;

int scraa_font_height=0;

z80_bit aa_sends_release;
z80_bit aa_sends_ESC;

int aa_last_message_shown_timer=0;
char aa_last_message_shown[DEBUG_MAX_MESSAGE_LENGTH];

unsigned char tecla_alternativa_esc;

void scraa_z88_cpc_load_keymap(void)
{
	debug_printf (VERBOSE_INFO,"Loading keymap");
}

void scraa_messages_debug(char *s)
{


sprintf (aa_last_message_shown,"%s",s);

//supuestamente 5 segundos (50*5)
aa_last_message_shown_timer=250;



}

//Rutina de putchar para menu
void scraa_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{

        tinta=tinta&15;
        papel=papel&15;

	enum aa_attribute attr=AA_NORMAL;
	if (tinta>7 || papel>7) attr=AA_BOLD;

	//color para entradas de menu activas. hacemos texto inverso
	if (papel==5+8) attr=AA_REVERSE;

	//color para shortcuts
        if (papel==0 && tinta==7+8) attr=AA_REVERSE;

	aa_printf(context,x,y,attr,"%c",caracter);


        //Para evitar warnings al compilar de "unused parameter"
        tinta=papel;
	papel=tinta;


}

void scraa_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel)
{
	//De momento Nada de footer
	//Calcular posicion y. Total de alto-3
	//int yorigin=aa_scrheight(context)-3;
	//aa_printf(context,x,yorigin+y,AA_NORMAL,"%c",caracter);

        tinta=tinta&15;
        papel=papel&15;

	//para que no se queje el compilador de variables no usadas
	x++;
	y++;
	caracter++;
	tinta++;
	papel++;
}

void scraa_putpixel_final_rgb(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color_rgb GCC_UNUSED)
{
}

void scraa_putpixel_final(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color GCC_UNUSED)
{
}

void scraa_putpixel(int x,int y,unsigned int color)
{


	//Escalar a la pantalla real mostrada
        x=x*scraa_imgwidth/scraa_totalrealwidth;
        y=y*scraa_imgheight/scraa_totalrealheight;

	//printf ("x: %d y: %d color: %d colorpaleta: %d scraa_imgwidth: %d \n",x,y,color,paleta[color],scraa_imgwidth);

	//Nota: vigilar si x o y se salen de rango, genera segmentation fault

	framebuffer_aa[y*scraa_imgwidth+x]=paleta[color];

}

void scraa_debug_registers(void)
{


char buffer[2048];

print_registers(buffer);

//        aa_printf(context,0,1,AA_NORMAL,"%s",buffer);
//aa_flush (context);
//imprimir con aa_printf muy seguido al final hace petar la ventana. lo hacemos con printf
	printf ("%s\r",buffer);

return;


}



void scraa_putchar_zx8081(int x,int y, z80_byte caracter)
{

        scr_putchar_zx8081_comun(x,y, caracter);


}

void scraa_refresca_pantalla_zx81(void)
{

scr_refresca_pantalla_y_border_zx8081();


}




void scraa_refresca_border(void)
{
	scr_refresca_border();
	return;

//      printf ("Refresco border\n");

        int x,y;
        z80_byte color=out_254 & 7;

        //parte superior
        for (y=0;y<TOP_BORDER;y++) {
                for (x=0;x<ANCHO_PANTALLA*zoom_x+LEFT_BORDER*2;x++) {
                                scraa_putpixel(x,y,color);


                }
        }

        //parte inferior
        for (y=0;y<BOTTOM_BORDER;y++) {
                for (x=0;x<ANCHO_PANTALLA*zoom_x+LEFT_BORDER*2;x++) {
                                scraa_putpixel(x,TOP_BORDER+y+ALTO_PANTALLA*zoom_y,color);


                }
        }


        //laterales
        for (y=0;y<ALTO_PANTALLA*zoom_y;y++) {
                for (x=0;x<LEFT_BORDER;x++) {
                        scraa_putpixel(x,TOP_BORDER+y,color);
                        scraa_putpixel(LEFT_BORDER+ANCHO_PANTALLA*zoom_x+x,TOP_BORDER+y,color);
                }

        }

}

void scraa_refresca_pantalla_solo_driver(void)
{
	//Como esto solo lo uso de momento para drivers graficos, de momento lo dejo vacio
}

void scraa_refresca_pantalla(void)
{


        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;


        if (MACHINE_IS_ZX8081) {
                scraa_refresca_pantalla_zx81();
        }

	else if (MACHINE_IS_SPECTRUM) {

		if (MACHINE_IS_TSCONF)	screen_tsconf_refresca_pantalla();


		  else { //Spectrum no TSConf

                //modo clasico. sin rainbow
                if (rainbow_enabled.v==0) {

	        	if (border_enabled.v) {
		                //ver si hay que refrescar border
        		        if (modificado_border.v)
	                	{
	        	                scraa_refresca_border();
		                        modificado_border.v=0;
	                	}

        		}

			scr_refresca_pantalla_comun();
		}

                else {
                        //modo rainbow - real video
                        scr_refresca_pantalla_rainbow_comun();
                }

		}

	}

        else if (MACHINE_IS_Z88) {
                screen_z88_refresca_pantalla();
        }

        else if (MACHINE_IS_ACE) {
                scr_refresca_pantalla_y_border_ace();
        }

	else if (MACHINE_IS_CPC) {
                scr_refresca_pantalla_y_border_cpc();
        }

        else if (MACHINE_IS_SAM) {
                scr_refresca_pantalla_y_border_sam();
        }

        else if (MACHINE_IS_QL) {
                scr_refresca_pantalla_y_border_ql();
        }

	else if (MACHINE_IS_MK14) {
		scr_refresca_pantalla_y_border_mk14();
	}


	else if (MACHINE_IS_MSX) {
		scr_refresca_pantalla_y_border_msx();
	}    

	else if (MACHINE_IS_SVI) {
		scr_refresca_pantalla_y_border_svi();
	}    	

	else if (MACHINE_IS_COLECO) {
		scr_refresca_pantalla_y_border_coleco();
	}            

	else if (MACHINE_IS_SG1000) {
		scr_refresca_pantalla_y_border_sg1000();
	}     

	else if (MACHINE_IS_SMS) {
		scr_refresca_pantalla_y_border_sms();
	}     


	//Espacio para footer
	if (scraa_fast) {
		//aa_fastrender(context, 0,0,scraa_imgwidth,scraa_imgheight-20);
		aa_fastrender(context, 0,0,scraa_imgwidth,scraa_imgheight);
		//aa_fastrender(context, 0,0,scraa_imgwidth,aa_scraa_imgheight(context)-30);
	}

	else {
		aa_render(context,rparams, 0,0,scraa_imgwidth,scraa_imgheight );
	}


	if (aa_last_message_shown_timer) {
        	aa_last_message_shown_timer--;
		aa_printf(context,0,0,AA_NORMAL,"%s",aa_last_message_shown);
	}


	screen_render_menu_overlay_if_active();

	
        //Escribir footer
        draw_middle_footer();



	aa_flush (context);


sem_screen_refresh_reallocate_layers=0;


}


void scraa_deal_with_keys(int tecla,int pressrelease)
{
 

	//printf ("tecla: %d\n",tecla);


        switch (tecla) {

                        //AA_BACKSPACE (valor 304) detecta tanto DEL como BACKSPACE
                        case AA_BACKSPACE:
				util_set_reset_key(UTIL_KEY_BACKSPACE,pressrelease);
			break;

			//Joystick fire
			//Tecla HOME no documentada. No detectamos pressrelease. Simplemente conmutamos estado
                        case 224:
				puerto_especial_joystick ^=16;
                        break;

                        //teclas no documentadas. No detectamos pressrelease. Simplemente conmutamos estado
                        //F1 pulsado
                        case 334:
                                        puerto_especial2 ^=1;
                                        blink_kbd_a14 ^= 128;
                        break;

                        //F2 pulsado
                        case 335:
                                        puerto_especial2 ^=2;
                                        blink_kbd_a15 ^= 16;
                        break;

                        //F3 pulsado
                        case 336:
                                        puerto_especial2 ^=4;
                                        blink_kbd_a14 ^=8;
                        break;

			//F4 pulsado
			case 337:
				textspeech_enviar_speech_pantalla();
			break;


			//F6 es Ctrl / diamond
	                //F7 es Alt / square

			case 339:
                                if (MACHINE_IS_ZX8081) {
					//para zx80/81
                                        //aqui hace lo mismo que mayusculas
                                        puerto_65278 ^=1;
                                }


                                else {
					puerto_32766 ^=2;
					blink_kbd_a14 ^= 16;
                                }

			break;				


			case 340:
				if (MACHINE_IS_ZX8081) {
					//para zx80/81
                                        //aqui hace lo mismo que mayusculas
                                        puerto_65278 ^=1;
                                }


                                else {
					puerto_32766 ^=2;
					blink_kbd_a15 ^= 64;
                                }

			break;
		



                        case AA_LEFT:
                                util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
                        break;

                        case AA_RIGHT:
                                util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
                        break;

                        case AA_DOWN:
                                util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
                        break;

                        case AA_UP:
                                util_set_reset_key(UTIL_KEY_UP,pressrelease);
                        break;


                        case AA_ESC:
				util_set_reset_key(UTIL_KEY_ESC,pressrelease);
                        break;



			//TABULADOR
                        case 9:
				util_set_reset_key(UTIL_KEY_TAB,pressrelease);
                        break;


			//Alternativa a F5
			case '*':
				util_set_reset_key(UTIL_KEY_F5,pressrelease);
				//y dejar que tambien establezca la "*", por eso no hay break

                        default:
				//printf ("%d %d \n",tecla,pressrelease);
				ascii_to_keyboard_port_set_clear(tecla,pressrelease);

                        break;

                }

}




void scraa_end(void)
{

	debug_printf (VERBOSE_INFO,"Closing aa video driver");

	aa_close(context);

}


//;                    Bits:  4    3    2    1    0     ;desplazamiento puerto
//puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
//puerto_65022   db    255  ; G    F    D    S    A     ;1
//puerto_64510    db              255  ; T    R    E    W    Q     ;2
//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7


#define SCRAA_MAX_LIBERA_TECLA 5
z80_byte scraa_contador_libera_tecla=0;

void scraa_actualiza_tablas_teclado(void)
{

int c;
int pressrelease;


//parece que no siempre se envia la orden release en modo consola... 
if (aa_sends_release.v==0) {
	//Liberar tecla solo cuando ha pasado un tiempo y antes habia tecla pulsada
	if (scraa_contador_libera_tecla>0) {
		scraa_contador_libera_tecla++;
		if (scraa_contador_libera_tecla==SCRAA_MAX_LIBERA_TECLA) {
			scraa_contador_libera_tecla=0;
			reset_keyboard_ports();
		}
	}
		
}

	do {
		c = aa_getevent(context, 0);
	        	//printf ("Tecla: %d %c \n",c,c);

	        if (c!=0) {
			scraa_contador_libera_tecla=1;
	        	//printf ("Tecla: %d %c\n",c,c);


				if (c>AA_RELEASE && aa_sends_release.v) {
					pressrelease=0;
					//printf ("Release\n");
					c  &= ~AA_RELEASE;
					scraa_deal_with_keys(c,pressrelease);	
				}
				else {
					pressrelease=1;
					notificar_tecla_interrupcion_si_z88();
					//printf ("Press\n");
					if (aa_sends_ESC.v==0 && c==tecla_alternativa_esc) {
						//envio ESC a traves de tecla alternativa
						scraa_deal_with_keys(AA_ESC,pressrelease);	
					}
						
					else scraa_deal_with_keys(c,pressrelease);	
				}

			}

	} while (c!=0);

}


z80_byte scraa_lee_puerto(z80_byte puerto_h,z80_byte puerto_l)
{

	//en aa no se usa

        //Para evitar warnings al compilar de "unused parameter"
        puerto_h=puerto_l;
        puerto_l=puerto_h;


	return 255;
}





//http://sunsite.ualberta.ca/Documentation/Graphics/aalib-1.2/html_mono/aalib.html



void scraa_get_image_params(void)
{


	scraa_imgwidth=aa_imgwidth(context);
	scraa_imgheight=aa_imgheight(context);


zoom_x=1+scraa_imgwidth/screen_get_emulated_display_width_no_zoom_border_en();
zoom_y=1+scraa_imgheight/screen_get_emulated_display_height_no_zoom_border_en();



set_putpixel_zoom();

scraa_totalrealwidth=screen_get_emulated_display_width_zoom_border_en();
scraa_totalrealheight=screen_get_emulated_display_height_zoom_border_en(); //+scraa_font_height*3;


debug_printf (VERBOSE_INFO,"aawidth: %d aaheight: %d scraa_totalrealwidth: %d scraa_totalrealheight: %d zoom_x: %d zoom_y: %d",scraa_imgwidth,scraa_imgheight,scraa_totalrealwidth,scraa_totalrealheight,zoom_x,zoom_y);
framebuffer_aa = aa_image (context);


        //obtener tamanyo fuente
	if (context!=NULL) {
		if (context->params.font!=NULL) {
			scraa_font_height=context->params.font->height;
		        //printf ("altura: %d\n",scraa_font_height);
		}
	}



}

static void scraaresize(aa_context * c)
{

    debug_printf (VERBOSE_INFO,"aa resize");

//       Do  resize  action.  This function ought to be called when application takes into account the AA_RESIZE event.  The context is reinitialized
//       and set to new sizes.


   clear_putpixel_cache();
  modificado_border.v=1;


    aa_resize(c);

scraa_get_image_params();


}


void scraa_set_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scraa_reset_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scraa_setpalette(int index,int r,int g,int b)
{
	aa_setpalette (paleta, index, r,g,b);

	debug_printf (VERBOSE_DEBUG,"Setting aalib colour palete index %d red: %d green: %d blue: %d",index,r,g,b);
}

void scraa_inicializa_colores(void)
{
	int i;

	for (i = 0; i < EMULATOR_TOTAL_PALETTE_COLOURS; i++) {
		scraa_setpalette(i, (spectrum_colortable[i] >> 16) & 0xFF, (spectrum_colortable[i] >> 8) & 0xFF, (spectrum_colortable[i] ) & 0xFF);

	}


}

void scraa_detectedchar_print(z80_byte caracter)
{
        printf ("%c",caracter);
        //flush de salida standard
        fflush(stdout);

}


//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scraa_get_menu_width(void)
{
        return 32;
}


int scraa_get_menu_height(void)
{
        return 24;
}


int scraa_driver_can_ext_desktop (void)
{
        return 0;
}


int scraa_init (void) 
{

	debug_printf (VERBOSE_INFO,"Init AAlib Video Driver");

    context = aa_autoinit(&aa_defparams);
    if (context == NULL) return 1;

	debug_printf (VERBOSE_INFO,"Using aalib driver: %s",context->driver->name);

//obtener drivers recomendados

//	printf ("driver: %s\n",context->driver->name);

/*
	char *recomend;
	for (i=0;aa_kbdrecommended[i];i++) {
		printf ("kbd driver %s\n",aa_kbdrecommended[i]);
	}
*/


	//Abrir menu con "*"
	openmenu_key_message="*";


int aa_kbdmode;
	//si en modo consola normal, las operaciones RELEASE no se envian correctamente	
	if (strstr(context->driver->name,"X11") !=NULL) {
	//driver X11
	aa_sends_release.v=1;
	aa_kbdmode=AA_SENDRELEASE;
	aa_sends_ESC.v=1;
	}

	else {
	//cualquier otro driver
	debug_printf (VERBOSE_WARN,"Setting KBD release to none. Expect bad keyboard performance");
	aa_sends_release.v=0;
	aa_kbdmode=0;
	aa_sends_ESC.v=0;
	//Volver (ESC) con tecla "<"
	esc_key_message="<";
	tecla_alternativa_esc='<';
	}

    if (aa_autoinitkbd(context,aa_kbdmode)==0) {
	debug_printf (VERBOSE_ERR,"Error aalib aa_autoinitkbd");
	return 1;
    }


    aa_hidecursor(context);


scr_putpixel=scraa_putpixel;
scr_putpixel_final=scraa_putpixel_final;
scr_putpixel_final_rgb=scraa_putpixel_final_rgb;
scr_putchar_zx8081=scraa_putchar_zx8081;
scr_putchar_menu=scraa_putchar_menu;
scr_putchar_footer=scraa_putchar_footer;

        scr_get_menu_width=scraa_get_menu_width;
        scr_get_menu_height=scraa_get_menu_height;
	scr_driver_can_ext_desktop=scraa_driver_can_ext_desktop;

rparams = aa_getrenderparams();
aa_resizehandler(context, scraaresize);

scraa_get_image_params();


scraa_inicializa_colores();






if (scraa_fast==0) debug_printf (VERBOSE_INFO,"Using aalib slow rendering");



scr_debug_registers=scraa_debug_registers;
scr_messages_debug=scraa_messages_debug;
scr_set_fullscreen=scraa_set_fullscreen;
scr_reset_fullscreen=scraa_reset_fullscreen;
scr_z88_cpc_load_keymap=scraa_z88_cpc_load_keymap;
scr_detectedchar_print=scraa_detectedchar_print;


//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
scr_set_driver_name("aa");
return 0;

}



