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


#include <string.h>
#include <stdio.h>
#if defined(__APPLE__)
        #include <SDL.h>
#else
	#include <SDL2/SDL.h>
#endif

#include "scrsdl2.h"
#include "common_sdl2.h"

#include "compileoptions.h"
#include "cpu.h"
#include "screen.h"
#include "charset.h"
#include "debug.h"
#include "zxvision.h"
#include "utils.h"
#include "joystick.h"
#include "cpc.h"
#include "prism.h"
#include "sam.h"
#include "ql.h"
#include "settings.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "svi.h"


SDL_Window *window=NULL;


//Indica que ha habido un evento de resize y queda pendiente redimensionar la ventana
//lo que sucede es que cuando se empieza a redimensionar una ventana, usando el raton,
//al recrear la ventana con el nuevo tamanyo, realmente lo que hace es cambiar el buffer de video interno de la ventana
//con el nuevo tamanyo, pero realmente el tamanyo de la ventana NO lo cambia, sino que queda siempre forzado con el tamanyo
//que estamos indicando con el raton
//Por tanto, lo que hacemos es que, activamos este indicador de redimensionar, y cuando hay un siguiente evento diferente de resize,
//la ventana se redimensiona
int scrsdl_debe_redimensionar=0;

unsigned char *scrsdl_pixeles;

SDL_Texture *scrsdl_texture;


SDL_Renderer *renderer;

//#define screen_get_window_size_width_zoom_border_en() screen_get_window_size_width_zoom_border_en()
#define screen_get_window_size_height_zoom_border_en() screen_get_window_size_height_zoom_border_en()





int scrsdl_crea_ventana(void)
{

  Uint32 flags;

  flags=SDL_WINDOW_RESIZABLE;

  if (ventana_fullscreen) {
	   flags |=SDL_WINDOW_FULLSCREEN;
   }


   debug_printf (VERBOSE_DEBUG,"Creating window %d X %d",screen_get_window_size_width_zoom_border_en(),screen_get_window_size_height_zoom_border_en() );

   int ancho=screen_get_window_size_width_zoom_border_en();
   ancho +=screen_get_ext_desktop_width_zoom();

   int alto=screen_get_window_size_height_zoom_border_en();
   alto +=screen_get_ext_desktop_height_zoom();

   if (SDL_CreateWindowAndRenderer(ancho,alto, flags, &window, &renderer)!=0) return 1;

        if ( window == NULL ) {
                return 1;
        }



        //SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	if (renderer==NULL) return 1;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_SetWindowTitle(window,"ZEsarUX "EMULATOR_VERSION);


    scrsdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ancho, alto);
    //Uint32 *pixels = new Uint32[screen_get_window_size_width_zoom_border_en() * screen_get_window_size_height_zoom_border_en()];
    scrsdl_pixeles=malloc(ancho * alto *4);


    if (scrsdl_pixeles==NULL) return 1;

                scr_reallocate_layers_menu(ancho,alto);

	if (mouse_pointer_shown.v==0) SDL_ShowCursor(0);

	modificado_border.v=1;

	screen_z88_draw_lower_screen();

	menu_init_footer();



  return 0;

}

void scrsdl_destruye_ventana(void)
{
  if (window!=NULL) SDL_DestroyWindow(window);
  window=NULL;
}

void scrsdl_putpixel_final_rgb(int x,int y,unsigned int color_rgb)
{	
        int ancho=screen_get_window_size_width_zoom_border_en();
        ancho +=screen_get_ext_desktop_width_zoom();
        Uint8 *p = (Uint8 *)scrsdl_pixeles + (y * ancho + x) * 4;


        //escribir de golpe los 32 bits.

        //agregar alpha
        color_rgb |=0xFF000000;
        //y escribir

        *(Uint32 *)p = color_rgb;
}

void scrsdl_putpixel_final(int x,int y,unsigned int color)
{

        unsigned int color32=spectrum_colortable[color];

        //y escribir
        scrsdl_putpixel_final_rgb(x,y,color32);                


}

void scrsdl_putpixel(int x,int y,unsigned int color)
{	
        if (menu_overlay_activo==0) {
                //Putpixel con menu cerrado
                scrsdl_putpixel_final(x,y,color);
                return;
        }          

        //Metemos pixel en layer adecuado
	buffer_layer_machine[y*ancho_layer_menu_machine+x]=color;        

        //Putpixel haciendo mix  
        screen_putpixel_mix_layers(x,y);   

}


void scrsdl_putchar_zx8081(int x,int y, z80_byte caracter)
{
        scr_putchar_zx8081_comun(x,y, caracter);
}

void scrsdl_debug_registers(void)
{

	char buffer[2048];
	print_registers(buffer);

	printf ("%s\r",buffer);
}

void scrsdl_messages_debug(char *s)
{
        printf ("%s\n",s);
        //flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
        fflush(stdout);

}

//Rutina de putchar para menu
void scrsdl_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{

        z80_bit inverse;

        inverse.v=0;

        //128 y 129 corresponden a franja de menu y a letra enye minuscula
        if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';

        scr_putchar_menu_comun_zoom(caracter,x,y,inverse,tinta,papel,menu_gui_zoom);

}

void scrsdl_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel) {


        int yorigen;

	yorigen=screen_get_emulated_display_height_no_zoom_bottomborder_en()/8;



        //scr_putchar_menu(x,yorigen+y,caracter,tinta,papel);
        y +=yorigen;
        z80_bit inverse;

        inverse.v=0;

        //128 y 129 corresponden a franja de menu y a letra enye minuscula
        if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';

        scr_putchar_footer_comun_zoom(caracter,x,y,inverse,tinta,papel);
}



void scrsdl_set_fullscreen(void)
{

	ventana_fullscreen=1;
  scrsdl_destruye_ventana();
	scrsdl_crea_ventana();

}


void scrsdl_reset_fullscreen(void)
{

	ventana_fullscreen=0;
  scrsdl_destruye_ventana();
	scrsdl_crea_ventana();

}


void scrsdl_refresca_pantalla_zx81(void)
{

        scr_refresca_pantalla_y_border_zx8081();

}


void scrsdl_refresca_border(void)
{
        scr_refresca_border();

}


void scrsdl_refresca_pantalla_solo_driver(void)
{

        int ancho=screen_get_window_size_width_zoom_border_en();
        ancho +=screen_get_ext_desktop_width_zoom();

  SDL_UpdateTexture(scrsdl_texture, NULL, scrsdl_pixeles, ancho * 4);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, scrsdl_texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

//Common routine for a graphical driver
void scrsdl_refresca_pantalla(void)
{


        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;


        if (MACHINE_IS_ZX8081) {


                //scr_refresca_pantalla_rainbow_comun();
                scrsdl_refresca_pantalla_zx81();
        }

        else if (MACHINE_IS_PRISM) {
                screen_prism_refresca_pantalla();
        }

        else if (MACHINE_IS_TBBLUE) {
                screen_tbblue_refresca_pantalla();
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
                                        scrsdl_refresca_border();
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


        //printf ("%d\n",spectrum_colortable[1]);

	screen_render_menu_overlay_if_active();


        //Escribir footer
        draw_middle_footer();

//printf ("refrescando\n");



        scrsdl_refresca_pantalla_solo_driver();



        sem_screen_refresh_reallocate_layers=0;


}


void scrsdl_end(void)
{
	debug_printf (VERBOSE_INFO,"Closing SDL2 video driver");
	scrsdl_inicializado.v=0;
  scrsdl_destruye_ventana(); //Nota: Cuando se cambia de resolucion en maquina (por ejemplo de spectrum a prism),
  //se llama a _end y luego se vuelve a inicializar el driver
  //en el caso de video sdl, si audio tambien es sdl, al llamar a commonsdl_end no se finaliza driver sdl, dado que esta usando el audio
  //Si se iniciase con otro driver de audio, si se finalizaria el driver
  //Por tanto, hay que destruir explicitamente la ventana en caso de cambios de resolucion y con driver audio sdl
  //Parece que esto solo afecta a windows. En mac y linux, al reinicializar el driver de video, desaparece la ventana anterior
	commonsdl_end();
}

z80_byte scrsdl_lee_puerto(z80_byte puerto_h,z80_byte puerto_l)
{

        //Para evitar warnings al compilar de "unused parameter"
        puerto_h=puerto_l;
        puerto_l=puerto_h;

	return 255;
}


//Teclas de Z88 asociadas a cada tecla del teclado fisico. Lo siguiente en teclado tipo Z88
int scrsdl_keymap_z88_cpc_minus;                //Tecla a la derecha del 0
int scrsdl_keymap_z88_cpc_equal;                //Tecla a la derecha de la anterior
int scrsdl_keymap_z88_cpc_backslash;            //Tecla a la derecha de la anterior (mapeado en teclado spanish en izquierda del 1)
int scrsdl_keymap_z88_cpc_bracket_left;         //Tecla a la derecha de la P
int scrsdl_keymap_z88_cpc_bracket_right;        //Tecla a la derecha de la anterior
int scrsdl_keymap_z88_cpc_semicolon;            //Tecla a la derecha de la L
int scrsdl_keymap_z88_cpc_apostrophe;           //Tecla a la derecha de la anterior
int scrsdl_keymap_z88_cpc_pound;                //Tecla a la derecha de la anterior
int scrsdl_keymap_z88_cpc_comma;                //Tecla a la derecha de la M
int scrsdl_keymap_z88_cpc_period;               //Tecla a la derecha de la anterior
int scrsdl_keymap_z88_cpc_slash;                //Tecla a la derecha de la anterior

int scrsdl_keymap_z88_cpc_circunflejo;          //Solo en CPC. Tecla a la derecha del 0. Misma que scrsdl_keymap_z88_cpc_equal
int scrsdl_keymap_z88_cpc_colon;                //Solo en CPC. Tecla a la derecha de la L. Misma que scrsdl_keymap_z88_cpc_semicolon
int scrsdl_keymap_z88_cpc_arroba;               //Solo en CPC. Tecla a la derecha de la P. Misma que scrsdl_keymap_z88_cpc_bracket_right

int scrsdl_keymap_z88_cpc_leftz;                //Tecla a la izquierda de la Z

//valores para estas teclas que vienen con compose, valores que me invento
//#define SDLK_LEFTBRACKET (16384+34)
//#define SDLK_TECLA_QUOTE (16384+48)


void scrsdl_z88_cpc_load_keymap(void)
{
	debug_printf (VERBOSE_INFO,"Loading keymap");

  #define SDLK_TECLA_SEMICOLON 241
  #define SDLK_TECLA_BACKSLASH 186
  #define SDLK_TECLA_EQUAL 161
  #define SDLK_TECLA_POUND 231

//valor inventado, tecla a la derecha de la enye:
  #define SDLK_TECLA_INVENTADA_D_ENYE 16384+39

        //Teclas se ubican en misma disposicion fisica del Z88, excepto:
        //libra~ -> spanish: cedilla (misma ubicacion fisica del z88). english: acento grave (supuestamente a la izquierda del 1)
        //backslash: en english esta en fila inferior del z88. en spanish, lo ubicamos a la izquierda del 1 (ºª\)
        switch (z88_cpc_keymap_type) {

                case 1:
			if (MACHINE_IS_Z88 || MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI) {
	                        scrsdl_keymap_z88_cpc_minus=SDLK_QUOTE;
        	                scrsdl_keymap_z88_cpc_equal=SDLK_TECLA_EQUAL;
                	        scrsdl_keymap_z88_cpc_backslash=SDLK_TECLA_BACKSLASH;

	                        scrsdl_keymap_z88_cpc_bracket_left=SDLK_LEFTBRACKET;
        	                scrsdl_keymap_z88_cpc_bracket_right=SDLK_PLUS;
                	        scrsdl_keymap_z88_cpc_semicolon=SDLK_TECLA_SEMICOLON;
	                        scrsdl_keymap_z88_cpc_apostrophe=SDLK_TECLA_INVENTADA_D_ENYE;
        	                scrsdl_keymap_z88_cpc_pound=SDLK_TECLA_POUND;
                	        scrsdl_keymap_z88_cpc_comma=SDLK_COMMA;
                        	scrsdl_keymap_z88_cpc_period=SDLK_PERIOD;
	                        scrsdl_keymap_z88_cpc_slash=SDLK_MINUS;
	                        scrsdl_keymap_z88_cpc_leftz=SDLK_LESS;
			}

			else if (MACHINE_IS_CPC) {
				scrsdl_keymap_z88_cpc_minus=SDLK_QUOTE;
	                        scrsdl_keymap_z88_cpc_circunflejo=SDLK_TECLA_EQUAL;

        	                scrsdl_keymap_z88_cpc_arroba=SDLK_LEFTBRACKET;
                	        scrsdl_keymap_z88_cpc_bracket_left=SDLK_PLUS;
	                        scrsdl_keymap_z88_cpc_colon=SDLK_TECLA_SEMICOLON;
        	                scrsdl_keymap_z88_cpc_semicolon=SDLK_TECLA_INVENTADA_D_ENYE;
                	        scrsdl_keymap_z88_cpc_bracket_right=SDLK_TECLA_POUND;
                        	scrsdl_keymap_z88_cpc_comma=SDLK_COMMA;
	                        scrsdl_keymap_z88_cpc_period=SDLK_PERIOD;
        	                scrsdl_keymap_z88_cpc_slash=SDLK_MINUS;

                	        scrsdl_keymap_z88_cpc_backslash=SDLK_TECLA_BACKSLASH;
	                        scrsdl_keymap_z88_cpc_leftz=SDLK_LESS;

			}

			else if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
                                scrsdl_keymap_z88_cpc_minus=SDLK_QUOTE;
                                scrsdl_keymap_z88_cpc_equal=SDLK_TECLA_EQUAL;
                                scrsdl_keymap_z88_cpc_backslash=SDLK_TECLA_BACKSLASH;

                                scrsdl_keymap_z88_cpc_bracket_left=SDLK_LEFTBRACKET;
                                scrsdl_keymap_z88_cpc_bracket_right=SDLK_PLUS;
                                scrsdl_keymap_z88_cpc_semicolon=SDLK_TECLA_SEMICOLON;
                                scrsdl_keymap_z88_cpc_apostrophe=SDLK_TECLA_INVENTADA_D_ENYE;
                                scrsdl_keymap_z88_cpc_pound=SDLK_TECLA_POUND;
                                scrsdl_keymap_z88_cpc_comma=SDLK_COMMA;
                                scrsdl_keymap_z88_cpc_period=SDLK_PERIOD;
                                scrsdl_keymap_z88_cpc_slash=SDLK_MINUS;
	                        scrsdl_keymap_z88_cpc_leftz=SDLK_LESS;
                        }

                break;


                default:
                        //0 o default
			if (MACHINE_IS_Z88 || MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI) {
	                        scrsdl_keymap_z88_cpc_minus=SDLK_MINUS;
        	                scrsdl_keymap_z88_cpc_equal=SDLK_EQUALS;
                	        scrsdl_keymap_z88_cpc_backslash=SDLK_BACKSLASH;

                        	scrsdl_keymap_z88_cpc_bracket_left=SDLK_LEFTBRACKET;
	                        scrsdl_keymap_z88_cpc_bracket_right=SDLK_RIGHTBRACKET;
        	                scrsdl_keymap_z88_cpc_semicolon=SDLK_SEMICOLON;
                	        scrsdl_keymap_z88_cpc_apostrophe=SDLK_TECLA_INVENTADA_D_ENYE;
                        	scrsdl_keymap_z88_cpc_pound=SDLK_BACKQUOTE;
	                        scrsdl_keymap_z88_cpc_comma=SDLK_COMMA;
        	                scrsdl_keymap_z88_cpc_period=SDLK_PERIOD;
                	        scrsdl_keymap_z88_cpc_slash=SDLK_SLASH;
	                        scrsdl_keymap_z88_cpc_leftz=SDLK_LESS;
			}

			else if (MACHINE_IS_CPC) {
				scrsdl_keymap_z88_cpc_minus=SDLK_MINUS;
                                scrsdl_keymap_z88_cpc_circunflejo=SDLK_EQUALS;

                                scrsdl_keymap_z88_cpc_arroba=SDLK_LEFTBRACKET;
                                scrsdl_keymap_z88_cpc_bracket_left=SDLK_RIGHTBRACKET;
                                scrsdl_keymap_z88_cpc_colon=SDLK_SEMICOLON;
                                scrsdl_keymap_z88_cpc_semicolon=SDLK_TECLA_INVENTADA_D_ENYE;
                                scrsdl_keymap_z88_cpc_bracket_right=SDLK_BACKQUOTE;
                                scrsdl_keymap_z88_cpc_comma=SDLK_COMMA;
                                scrsdl_keymap_z88_cpc_period=SDLK_PERIOD;
                                scrsdl_keymap_z88_cpc_slash=SDLK_SLASH;

                                scrsdl_keymap_z88_cpc_backslash=SDLK_BACKSLASH;
	                        scrsdl_keymap_z88_cpc_leftz=SDLK_LESS;
			}

			else if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
                                scrsdl_keymap_z88_cpc_minus=SDLK_MINUS;
                                scrsdl_keymap_z88_cpc_equal=SDLK_EQUALS;
                                scrsdl_keymap_z88_cpc_backslash=SDLK_BACKSLASH;

                                scrsdl_keymap_z88_cpc_bracket_left=SDLK_LEFTBRACKET;
                                scrsdl_keymap_z88_cpc_bracket_right=SDLK_RIGHTBRACKET;
                                scrsdl_keymap_z88_cpc_semicolon=SDLK_SEMICOLON;
                                scrsdl_keymap_z88_cpc_apostrophe=SDLK_TECLA_INVENTADA_D_ENYE;
                                scrsdl_keymap_z88_cpc_pound=SDLK_BACKQUOTE;
                                scrsdl_keymap_z88_cpc_comma=SDLK_COMMA;
                                scrsdl_keymap_z88_cpc_period=SDLK_PERIOD;
                                scrsdl_keymap_z88_cpc_slash=SDLK_SLASH;
	                        scrsdl_keymap_z88_cpc_leftz=SDLK_LESS;
                        }

                break;
        }
}


void scrsdl_deal_raw_keys(int pressrelease,int scancode)
{

	//printf ("scrsdl_deal_raw_keys: scancode: %d pressrelease: %d\n",scancode,pressrelease);

	switch (scancode) {
		case SDL_SCANCODE_0:
			util_set_reset_key('0',pressrelease);
		break;

		case SDL_SCANCODE_1:
			util_set_reset_key('1',pressrelease);
		break;

		case SDL_SCANCODE_2:
			util_set_reset_key('2',pressrelease);
		break;

		case SDL_SCANCODE_3:
			util_set_reset_key('3',pressrelease);
		break;

		case SDL_SCANCODE_4:
			util_set_reset_key('4',pressrelease);
		break;

		case SDL_SCANCODE_5:
			util_set_reset_key('5',pressrelease);
		break;

		case SDL_SCANCODE_6:
			util_set_reset_key('6',pressrelease);
		break;

		case SDL_SCANCODE_7:
			util_set_reset_key('7',pressrelease);
		break;

		case SDL_SCANCODE_8:
			util_set_reset_key('8',pressrelease);
		break;

		case SDL_SCANCODE_9:
			util_set_reset_key('9',pressrelease);
		break;

                case SDL_SCANCODE_Q:
                        util_set_reset_key('q',pressrelease);
                break;

                case SDL_SCANCODE_W:
                        util_set_reset_key('w',pressrelease);
                break;

                case SDL_SCANCODE_E:
                        util_set_reset_key('e',pressrelease);
                break;

                case SDL_SCANCODE_R:
                        util_set_reset_key('r',pressrelease);
                break;

                case SDL_SCANCODE_T:
                        util_set_reset_key('t',pressrelease);
                break;

                case SDL_SCANCODE_Y:
                        util_set_reset_key('y',pressrelease);
                break;

                case SDL_SCANCODE_U:
                        util_set_reset_key('u',pressrelease);
                break;

                case SDL_SCANCODE_I:
                        util_set_reset_key('i',pressrelease);
                break;

                case SDL_SCANCODE_O:
                        util_set_reset_key('o',pressrelease);
                break;

                case SDL_SCANCODE_P:
                        util_set_reset_key('p',pressrelease);
                break;

                case SDL_SCANCODE_A:
                        util_set_reset_key('a',pressrelease);
                break;

                case SDL_SCANCODE_S:
                        util_set_reset_key('s',pressrelease);
                break;

                case SDL_SCANCODE_D:
                        util_set_reset_key('d',pressrelease);
                break;

                case SDL_SCANCODE_F:
                        util_set_reset_key('f',pressrelease);
                break;

                case SDL_SCANCODE_G:
                        util_set_reset_key('g',pressrelease);
                break;

                case SDL_SCANCODE_H:
                        util_set_reset_key('h',pressrelease);
                break;

                case SDL_SCANCODE_J:
                        util_set_reset_key('j',pressrelease);
                break;

                case SDL_SCANCODE_K:
                        util_set_reset_key('k',pressrelease);
                break;

                case SDL_SCANCODE_L:
                        util_set_reset_key('l',pressrelease);
                break;

                case SDL_SCANCODE_Z:
                        util_set_reset_key('z',pressrelease);
                break;

                case SDL_SCANCODE_X:
                        util_set_reset_key('x',pressrelease);
                break;

                case SDL_SCANCODE_C:
                        util_set_reset_key('c',pressrelease);
                break;

                case SDL_SCANCODE_V:
                        util_set_reset_key('v',pressrelease);
                break;

                case SDL_SCANCODE_B:
                        util_set_reset_key('b',pressrelease);
                break;

                case SDL_SCANCODE_N:
                        util_set_reset_key('n',pressrelease);
                break;

                case SDL_SCANCODE_M:
                        util_set_reset_key('m',pressrelease);
                break;

                case SDL_SCANCODE_COMMA:
			util_set_reset_key(',',pressrelease);
                break;                

		case SDL_SCANCODE_PERIOD:
			util_set_reset_key('.',pressrelease);
                break;

		case SDL_SCANCODE_LSHIFT:
			util_set_reset_key(UTIL_KEY_SHIFT_L,pressrelease);
		break;

                case SDL_SCANCODE_LCTRL:
			util_set_reset_key(UTIL_KEY_CONTROL_L,pressrelease);
		break;

                case SDL_SCANCODE_ESCAPE:
                        util_set_reset_key(UTIL_KEY_ESC,pressrelease);
                break;

                case SDL_SCANCODE_RETURN:
                        util_set_reset_key(UTIL_KEY_ENTER,pressrelease);
                break;

                case SDL_SCANCODE_SPACE:
                        util_set_reset_key(UTIL_KEY_SPACE,pressrelease);
                break;

                case SDL_SCANCODE_LEFT:
                        util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
                break;

                case SDL_SCANCODE_RIGHT:
                        util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
                break;

                case SDL_SCANCODE_DOWN:
                        util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
                break;

                case SDL_SCANCODE_UP:
                        util_set_reset_key(UTIL_KEY_UP,pressrelease);
                break;

                        //F1 pulsado
                        case SDL_SCANCODE_F1:
                                util_set_reset_key(UTIL_KEY_F1,pressrelease);
                        break;

                        //F2 pulsado
                        case SDL_SCANCODE_F2:
                                util_set_reset_key(UTIL_KEY_F2,pressrelease);
                        break;

                        //F3 pulsado
                        case SDL_SCANCODE_F3:
                                util_set_reset_key(UTIL_KEY_F3,pressrelease);
                        break;

                        //F4 pulsado
                        case SDL_SCANCODE_F4:
                                util_set_reset_key(UTIL_KEY_F4,pressrelease);
                        break;

                        //F5 pulsado
                        case SDL_SCANCODE_F5:
                                util_set_reset_key(UTIL_KEY_F5,pressrelease);
                        break;

                        //F6 pulsado
                        case SDL_SCANCODE_F6:
                                util_set_reset_key(UTIL_KEY_F6,pressrelease);
                        break;

                        //F7 pulsado
                        case SDL_SCANCODE_F7:
                                util_set_reset_key(UTIL_KEY_F7,pressrelease);
                        break;


                        //F8 pulsado. osdkeyboard
                        case SDL_SCANCODE_F8:
                                util_set_reset_key(UTIL_KEY_F8,pressrelease);
                        break;

                        //F9 pulsado. quickload
                        case SDL_SCANCODE_F9:
                                util_set_reset_key(UTIL_KEY_F9,pressrelease);
                        break;


                        //F10 pulsado
                        case SDL_SCANCODE_F10:
                                util_set_reset_key(UTIL_KEY_F10,pressrelease);
                        break;


                        //F11 pulsado
                        case SDL_SCANCODE_F11:
                                util_set_reset_key(UTIL_KEY_F11,pressrelease);
                        break;

                        //F12 pulsado
                        case SDL_SCANCODE_F12:
                                util_set_reset_key(UTIL_KEY_F12,pressrelease);
                        break;

                        //F13 pulsado
                        case SDL_SCANCODE_F13:
                                util_set_reset_key(UTIL_KEY_F13,pressrelease);
                        break;

                        //F14 pulsado
                        case SDL_SCANCODE_F14:
                                util_set_reset_key(UTIL_KEY_F14,pressrelease);
                        break;

                        //F15 pulsado
                        case SDL_SCANCODE_F15:
                                util_set_reset_key(UTIL_KEY_F15,pressrelease);
                        break;

                case SDL_SCANCODE_DELETE:
                        util_set_reset_key(UTIL_KEY_DEL,pressrelease);
                break;

                case SDL_SCANCODE_BACKSPACE:
                        util_set_reset_key(UTIL_KEY_BACKSPACE,pressrelease);
                break;                

                case SDL_SCANCODE_LEFTBRACKET:
                        util_set_reset_key('[',pressrelease);
                break;              

                case SDL_SCANCODE_RIGHTBRACKET:
                        util_set_reset_key(']',pressrelease);
                break;                     

                case SDL_SCANCODE_MINUS:
                        util_set_reset_key('-',pressrelease);
                break;                                            

                case SDL_SCANCODE_EQUALS:
                        util_set_reset_key('=',pressrelease);
                break;                     

                case SDL_SCANCODE_SEMICOLON:
                        util_set_reset_key(';',pressrelease);
                break;                     

                case SDL_SCANCODE_APOSTROPHE:
                        util_set_reset_key('\'',pressrelease);
                break; 

                case SDL_SCANCODE_SLASH:
                        util_set_reset_key('/',pressrelease);
                break; 

                //En caso raw, enviamos cursore del keypad igual que cursores normales
                case SDL_SCANCODE_KP_4:
                        util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
                break;

                case SDL_SCANCODE_KP_6:
                        util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
                break;

                case SDL_SCANCODE_KP_2:
                        util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
                break;

                case SDL_SCANCODE_KP_8:
                        util_set_reset_key(UTIL_KEY_UP,pressrelease);
                break;        

	}
}


void scrsdl_deal_keys(int pressrelease,int tecla)
{

	//printf ("scrsdl_deal_keys tecla: %d 0x%X pressrelease: %d\n",tecla,tecla,pressrelease);
	//printf ("tecla: 0x%X\n",tecla);

        //Teclas que necesitan conversion de teclado para Chloe
        int tecla_gestionada_chloe=0;
        if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
                        tecla_gestionada_chloe=1;

                        if (tecla==scrsdl_keymap_z88_cpc_minus) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_MINUS,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_equal) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_EQUAL,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_backslash) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BACKSLASH,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_left) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BRACKET_LEFT,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_right) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BRACKET_RIGHT,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_semicolon) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_SEMICOLON,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_apostrophe) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_APOSTROPHE,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_pound) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_POUND,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_comma) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_COMMA,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_period) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_PERIOD,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_slash) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_SLASH,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_leftz) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_LEFTZ,pressrelease);

                        else tecla_gestionada_chloe=0;
        }


        if (tecla_gestionada_chloe) return;



        int tecla_gestionada_sam_ql=0;
        if (MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI) {
                tecla_gestionada_sam_ql=1;

                        if (tecla==scrsdl_keymap_z88_cpc_minus) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_MINUS,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_equal) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_EQUAL,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_backslash) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BACKSLASH,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_left) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BRACKET_LEFT,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_right) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BRACKET_RIGHT,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_semicolon) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_SEMICOLON,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_apostrophe) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_APOSTROPHE,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_pound) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_POUND,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_comma) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_COMMA,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_period) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_PERIOD,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_slash) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_SLASH,pressrelease);


                else tecla_gestionada_sam_ql=0;
        }

        if (tecla_gestionada_sam_ql) return;



        switch (tecla) {

                        case 32:
                                util_set_reset_key(UTIL_KEY_SPACE,pressrelease);
                        break;

                        case SDLK_RETURN:
                                util_set_reset_key(UTIL_KEY_ENTER,pressrelease);
                        break;

                        case SDLK_LSHIFT:
                                util_set_reset_key(UTIL_KEY_SHIFT_L,pressrelease);
                        break;

                        case SDLK_RSHIFT:
                                joystick_possible_rightshift_key(pressrelease);
                        break;

                        case SDLK_LALT:
                                util_set_reset_key(UTIL_KEY_ALT_L,pressrelease);
                        break;
                        case SDLK_RALT:
                                joystick_possible_rightalt_key(pressrelease);
                        break;


                        case SDLK_LCTRL:
                                util_set_reset_key(UTIL_KEY_CONTROL_L,pressrelease);
                        break;

                        case SDLK_RCTRL:
                                joystick_possible_rightctrl_key(pressrelease);
                        break;

                        case SDLK_LGUI:
                        case SDLK_RGUI:
                                util_set_reset_key(UTIL_KEY_WINKEY,pressrelease);
                        break;

                        case SDLK_DELETE:
                                util_set_reset_key(UTIL_KEY_DEL,pressrelease);
                        break;

                        //Teclas que generan doble pulsacion
                        case SDLK_BACKSPACE:
                                util_set_reset_key(UTIL_KEY_BACKSPACE,pressrelease);
                        break;

                        case SDLK_HOME:
                                joystick_possible_home_key(pressrelease);
                        break;

                        case SDLK_LEFT:
                                util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
                        break;

                        case SDLK_RIGHT:
                                util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
                        break;

                        case SDLK_DOWN:
                                util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
                        break;

                        case SDLK_UP:
                                util_set_reset_key(UTIL_KEY_UP,pressrelease);
                        break;

                        case SDLK_TAB:
                                util_set_reset_key(UTIL_KEY_TAB,pressrelease);
                        break;

                        case SDLK_CAPSLOCK:
                                util_set_reset_key(UTIL_KEY_CAPS_LOCK,pressrelease);
                        break;

                        case SDLK_COMMA:
                                util_set_reset_key(UTIL_KEY_COMMA,pressrelease);
                        break;

                        case SDLK_PERIOD:
                                util_set_reset_key(UTIL_KEY_PERIOD,pressrelease);
                        break;

                        case SDLK_MINUS:
                                util_set_reset_key(UTIL_KEY_MINUS,pressrelease);
                        break;

                        case SDLK_PLUS:
                                util_set_reset_key(UTIL_KEY_PLUS,pressrelease);
                        break;

                        case SDLK_SLASH:
                                util_set_reset_key(UTIL_KEY_SLASH,pressrelease);
                        break;

                        case SDLK_ASTERISK:
                                util_set_reset_key(UTIL_KEY_ASTERISK,pressrelease);
                        break;



                        //F1 pulsado
                        case SDLK_F1:
                                util_set_reset_key(UTIL_KEY_F1,pressrelease);
                        break;

                        //F2 pulsado
                        case SDLK_F2:
                                util_set_reset_key(UTIL_KEY_F2,pressrelease);
                        break;

                        //F3 pulsado
                        case SDLK_F3:
                                util_set_reset_key(UTIL_KEY_F3,pressrelease);
                        break;

                        //F4 pulsado
                        case SDLK_F4:
                                util_set_reset_key(UTIL_KEY_F4,pressrelease);
                        break;

                        //F5 pulsado
                        case SDLK_F5:
                                util_set_reset_key(UTIL_KEY_F5,pressrelease);
                        break;

                        //F6 pulsado
                        case SDLK_F6:
                                util_set_reset_key(UTIL_KEY_F6,pressrelease);
                        break;

                        //F7 pulsado
                        case SDLK_F7:
                                util_set_reset_key(UTIL_KEY_F7,pressrelease);
                        break;


                        //F8 pulsado. osdkeyboard
                        case SDLK_F8:
                                util_set_reset_key(UTIL_KEY_F8,pressrelease);
                        break;

                        //F9 pulsado. quickload
                        case SDLK_F9:
                                util_set_reset_key(UTIL_KEY_F9,pressrelease);
                        break;


                        //F10 pulsado
                        case SDLK_F10:
                                util_set_reset_key(UTIL_KEY_F10,pressrelease);
                        break;


                                                //F11 pulsado
                                                case SDLK_F11:
                                                        util_set_reset_key(UTIL_KEY_F11,pressrelease);
                                                break;

                                                //F12 pulsado
                                                case SDLK_F12:
                                                        util_set_reset_key(UTIL_KEY_F12,pressrelease);
                                                break;

                                                //F13 pulsado
                                                case SDLK_F13:
                                                        util_set_reset_key(UTIL_KEY_F13,pressrelease);
                                                break;

                                                //F14 pulsado
                                                case SDLK_F14:
                                                        util_set_reset_key(UTIL_KEY_F14,pressrelease);
                                                break;

                                                //F15 pulsado
                                                case SDLK_F15:
                                                        util_set_reset_key(UTIL_KEY_F15,pressrelease);
                                                break;



                        //ESC pulsado
                        case SDLK_ESCAPE:
                                util_set_reset_key(UTIL_KEY_ESC,pressrelease);
                        break;

                        //PgUp
                        case SDLK_PAGEUP:
                                util_set_reset_key(UTIL_KEY_PAGE_UP,pressrelease);
                        break;

                        //PgDn
                        case SDLK_PAGEDOWN:
                                util_set_reset_key(UTIL_KEY_PAGE_DOWN,pressrelease);
                        break;

			//Teclas del keypad
			case SDLK_KP_0:
                                util_set_reset_key(UTIL_KEY_KP0,pressrelease);
                        break;

                        case SDLK_KP_1:
                                util_set_reset_key(UTIL_KEY_KP1,pressrelease);
                        break;

                        case SDLK_KP_2:
                                util_set_reset_key(UTIL_KEY_KP2,pressrelease);
                        break;

                        case SDLK_KP_3:
                                util_set_reset_key(UTIL_KEY_KP3,pressrelease);
                        break;

                        case SDLK_KP_4:
                                util_set_reset_key(UTIL_KEY_KP4,pressrelease);
                        break;

                        case SDLK_KP_5:
                                util_set_reset_key(UTIL_KEY_KP5,pressrelease);
                        break;

                        case SDLK_KP_6:
                                util_set_reset_key(UTIL_KEY_KP6,pressrelease);
                        break;

                        case SDLK_KP_7:
                                util_set_reset_key(UTIL_KEY_KP7,pressrelease);
                        break;

                        case SDLK_KP_8:
                                util_set_reset_key(UTIL_KEY_KP8,pressrelease);
                        break;

                        case SDLK_KP_9:
                                util_set_reset_key(UTIL_KEY_KP9,pressrelease);
                        break;

                        case SDLK_KP_PERIOD:
                                util_set_reset_key(UTIL_KEY_KP_COMMA,pressrelease);
                        break;

                        case SDLK_KP_ENTER:
                                util_set_reset_key(UTIL_KEY_KP_ENTER,pressrelease);
                        break;



                        default:
                                if (tecla<256) {
					//printf ("tecla: %d\n",tecla);
					//convert_numeros_letras_puerto_teclado(tecla,pressrelease);
					util_set_reset_key(tecla,pressrelease);
				}
                        break;

                }


        //Fuera del switch
//Teclas que necesitan conversion de teclado para CPC
        if (MACHINE_IS_CPC) {

                        if (tecla==scrsdl_keymap_z88_cpc_minus) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_MINUS,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_circunflejo) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_CIRCUNFLEJO,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_arroba) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_ARROBA,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_left) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BRACKET_LEFT,pressrelease);



                        else if (tecla==scrsdl_keymap_z88_cpc_colon) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_COLON,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_semicolon) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_SEMICOLON,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_right) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BRACKET_RIGHT,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_comma) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_COMMA,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_period) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_PERIOD,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_slash) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_SLASH,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_backslash) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BACKSLASH,pressrelease);


        }


//Teclas que necesitan conversion de teclado para Z88
        if (!MACHINE_IS_Z88) return;

                        if (tecla==scrsdl_keymap_z88_cpc_minus) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_MINUS,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_equal) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_EQUAL,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_backslash) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BACKSLASH,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_left) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BRACKET_LEFT,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_bracket_right) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BRACKET_RIGHT,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_semicolon) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_SEMICOLON,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_apostrophe) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_APOSTROPHE,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_pound) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_POUND,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_comma) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_COMMA,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_period) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_PERIOD,pressrelease);

                        else if (tecla==scrsdl_keymap_z88_cpc_slash) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_SLASH,pressrelease);





}


void scrsdl_resize(int width,int height)
{

if (ventana_fullscreen) return ; //No hacer resizes cuando este en pantalla completa, sino nos metemos en un bucle de resizes continuos

	clear_putpixel_cache();

        int zoom_x_calculado,zoom_y_calculado;

        debug_printf (VERBOSE_INFO,"width: %d get_window_width: %d height: %d get_window_height: %d",width,screen_get_window_size_width_no_zoom_border_en(),height,screen_get_window_size_height_no_zoom_border_en());

        //printf ("allocate layers menu\n");
        scr_reallocate_layers_menu(width,height);   

	//zoom_x_calculado=width/screen_get_window_size_width_no_zoom_border_en();
        zoom_x_calculado=width/(screen_get_window_size_width_no_zoom_border_en()+screen_get_ext_desktop_width_no_zoom() );
	    zoom_y_calculado=height/(screen_get_window_size_height_no_zoom_border_en()+screen_get_ext_desktop_height_no_zoom() );


        if (!zoom_x_calculado) zoom_x_calculado=1;
        if (!zoom_y_calculado) zoom_y_calculado=1;

        debug_printf (VERBOSE_INFO,"zoom_x: %d zoom_y: %d zoom_x_calculated: %d zoom_y_calculated: %d",zoom_x,zoom_y,zoom_x_calculado,zoom_y_calculado);

        if (zoom_x_calculado!=zoom_x || zoom_y_calculado!=zoom_y) {
                //resize
		debug_printf (VERBOSE_INFO,"Resizing window");

                zoom_x=zoom_x_calculado;
                zoom_y=zoom_y_calculado;
                set_putpixel_zoom();

                //width=screen_get_window_size_width_zoom_border_en();
                //height=screen_get_window_size_height_zoom_border_en();


        }

	scrsdl_debe_redimensionar=1;
  scrsdl_destruye_ventana();
	scrsdl_crea_ventana();


}

int scrsdl_recibido_resize(SDL_Event *event)
{

  if (ventana_fullscreen) return 0; //No avisar de resizes cuando este en pantalla completa, sino nos metemos en un bucle de resizes continuos

  if (event->type==SDL_WINDOWEVENT) {
    if (event->window.event==SDL_WINDOWEVENT_RESIZED) return 1;
  }

  return 0;
}

void scrsdl_actualiza_tablas_teclado(void)
{


	SDL_Event event;
	int pressrelease;

//SDL_EventState(SDL_DROPFILE, SDL_ENABLE);


	while( SDL_PollEvent( &event ) ){

//printf ("evento: %d\n",event.type);

		//Si se ha dejado de redimensionar la ventana
		//if (event.type!=SDL_WINDOWEVENT_SIZE_CHANGED) {
    if (!scrsdl_recibido_resize(&event)) {
                        if (scrsdl_debe_redimensionar) {

				//printf ("evento: %d\n",event.type);

                                debug_printf (VERBOSE_DEBUG,"Resizing windows due to a previous pending resize");

                                clear_putpixel_cache();
                                scrsdl_debe_redimensionar=0;
                                scrsdl_destruye_ventana();
                                scrsdl_crea_ventana();
                        }
		}



		if (event.type==SDL_KEYDOWN || event.type==SDL_KEYUP) {
			if (event.type==SDL_KEYDOWN) pressrelease=1;
			if (event.type==SDL_KEYUP) pressrelease=0;



			SDL_Keysym keysym=event.key.keysym;
			SDL_Scancode scancode=keysym.scancode;

			//SDLKey sym=keysym.sym;
			int tecla=keysym.sym;

			/* temp revisar SDL2
			if (tecla==SDLK_COMPOSE) {
				if (keysym.scancode==34) tecla=SDLK_LEFTBRACKET;
				else if (keysym.scancode==48) tecla=SDLK_TECLA_QUOTE;
			}
			*/
/*
See the SDL documentation. Scancodes represent the physical position of the keys, modeled after a standard QWERTY keyboard, while Keycodes are the character obtained by pressing the key.
*/

			if (keysym.scancode==SDL_SCANCODE_APOSTROPHE) tecla=SDLK_TECLA_INVENTADA_D_ENYE;
			//printf ("tecla: %d scancode: %d\n",tecla,scancode);

			if (pressrelease) notificar_tecla_interrupcion_si_z88();

                        if (sdl_raw_keyboard_read.v) scrsdl_deal_raw_keys(pressrelease,scancode);
                        else scrsdl_deal_keys(pressrelease,tecla);

		}



		//resize
		//if (event.type==SDL_WINDOWEVENT_SIZE_CHANGED) {
    if (scrsdl_recibido_resize(&event)) {

			        scrsdl_resize(event.window.data1, event.window.data2);

		}


		//mouse motion
		if (event.type==SDL_MOUSEMOTION) {
                    mouse_x=event.motion.x;
                    mouse_y=event.motion.y;



                    kempston_mouse_x=mouse_x/zoom_x;
                    kempston_mouse_y=255-mouse_y/zoom_y;
                    //printf("Mouse is at (%d,%d)\n", kempston_mouse_x, kempston_mouse_y);

                        debug_printf (VERBOSE_PARANOID,"Mouse motion. X: %d Y:%d kempston x: %d y: %d",mouse_x,mouse_y,kempston_mouse_x,kempston_mouse_y);
		}


		//mouse button
		if (event.type==SDL_MOUSEBUTTONDOWN) {

			debug_printf (VERBOSE_PARANOID,"Mouse Button Press. x=%d y=%d", event.button.x, event.button.y);

                        if ( event.button.button == SDL_BUTTON_LEFT ) {
				//mouse_left=1;
				util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,1);
			}
                        if ( event.button.button == SDL_BUTTON_RIGHT ) {
                                //mouse_right=1;
				util_set_reset_mouse(UTIL_MOUSE_RIGHT_BUTTON,1);
                        }

                        //TODO: soporte rueda raton. Ya no se trata como un boton en SDL2, sino que es  SDL_MOUSEWHEEL  event

                        gunstick_x=event.button.x;
                        gunstick_y=event.button.y;
                        gunstick_x=gunstick_x/zoom_x;
                        gunstick_y=gunstick_y/zoom_y;

                        debug_printf (VERBOSE_PARANOID,"Mouse Button press. x=%d y=%d. gunstick: x: %d y: %d", event.button.x, event.button.y,gunstick_x
,gunstick_y);

		}

		if (event.type==SDL_MOUSEBUTTONUP) {
                        debug_printf (VERBOSE_PARANOID,"Mouse Button release. x=%d y=%d", event.button.x, event.button.y);
                        if ( event.button.button == SDL_BUTTON_LEFT ) {
				//mouse_left=0;
				util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,0);
			}
                        if ( event.button.button == SDL_BUTTON_RIGHT ) {
				//mouse_right=0;
				util_set_reset_mouse(UTIL_MOUSE_RIGHT_BUTTON,0);
			}

		}

		if (event.type==SDL_QUIT) {
	                debug_printf (VERBOSE_INFO,"Received window close event");
        	        //Hacemos que aparezca el menu con opcion de salir del emulador
                	menu_abierto=1;
	                menu_button_exit_emulator.v=1;
		}

                if (event.type==SDL_DROPFILE) {      // In case if dropped file
                    //printf ("Received drag&drop file %s\n",event.drop.file);
                        strcpy(quickload_file,event.drop.file);
                        menu_abierto=1;
                        menu_event_drag_drop.v=1;

			                  SDL_free(event.drop.file);    // Free dropped_filedir memory
               }


	}




}

void scrsdl_detectedchar_print(z80_byte caracter)
{
        printf ("%c",caracter);
        //flush de salida standard
        fflush(stdout);

}

//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrsdl_get_menu_width(void)
{
        //int max=screen_get_emulated_display_width_no_zoom_border_en()/menu_char_width/menu_gui_zoom;

        int max=screen_get_emulated_display_width_no_zoom_border_en();

        max +=screen_get_ext_desktop_width_no_zoom();

        max=max/menu_char_width/menu_gui_zoom;


        if (max>OVERLAY_SCREEN_MAX_WIDTH) max=OVERLAY_SCREEN_MAX_WIDTH;

                //printf ("max x: %d %d\n",max,screen_get_emulated_display_width_no_zoom_border_en());

        return max;
}


int scrsdl_get_menu_height(void)
{
        int max=screen_get_emulated_display_height_no_zoom_border_en();

        max +=screen_get_ext_desktop_height_no_zoom();

        max=max/menu_char_height/menu_gui_zoom;

        if (max>OVERLAY_SCREEN_MAX_HEIGTH) max=OVERLAY_SCREEN_MAX_HEIGTH;

                //printf ("max y: %d %d\n",max,screen_get_emulated_display_height_no_zoom_border_en());
        return max;
}

int scrsdl_driver_can_ext_desktop (void)
{
        return 1;
}


int scrsdl_init (void) {

	debug_printf (VERBOSE_INFO,"Init SDL2 Video Driver");


        //Inicializaciones necesarias
        scr_putpixel=scrsdl_putpixel;
        scr_putpixel_final=scrsdl_putpixel_final;
        scr_putpixel_final_rgb=scrsdl_putpixel_final_rgb;

        scr_get_menu_width=scrsdl_get_menu_width;
        scr_get_menu_height=scrsdl_get_menu_height;              
	scr_driver_can_ext_desktop=scrsdl_driver_can_ext_desktop;

        scr_putchar_zx8081=scrsdl_putchar_zx8081;
        scr_debug_registers=scrsdl_debug_registers;
        scr_messages_debug=scrsdl_messages_debug;
        scr_putchar_menu=scrsdl_putchar_menu;
        scr_putchar_footer=scrsdl_putchar_footer;
        scr_set_fullscreen=scrsdl_set_fullscreen;
        scr_reset_fullscreen=scrsdl_reset_fullscreen;
	scr_z88_cpc_load_keymap=scrsdl_z88_cpc_load_keymap;
	scr_detectedchar_print=scrsdl_detectedchar_print;
        scr_tiene_colores=1;
        screen_refresh_menu=1;



    	if (commonsdl_init() != 0 ) {
		debug_printf (VERBOSE_ERR,"scrsdl_init: Error initializing driver");
                return 1;
	}

	scrsdl_inicializado.v=1;

	if (scrsdl_crea_ventana()) {
		debug_printf (VERBOSE_ERR,"scrsdl_init: Could not set video mode");
		return 1;
	}



        //Otra inicializacion necesaria
        //Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
        scr_set_driver_name("sdl");


	scr_z88_cpc_load_keymap();

        return 0;

}



/* Esto no usado. Eran pruebas antiguas
SDL_Thread *sdl2_timer_pthread;


static int sdl2_timer_pthread_fn(void *ptr)
{
  thread_timer_function(NULL);

  return 0; //aunque aqui nunca llega
}

void scrsdl_crear_timer_thread(void)
{
  debug_printf(VERBOSE_INFO,"Creating thread using SDL2 SDL_CreateThread");
  sdl2_timer_pthread=SDL_CreateThread(sdl2_timer_pthread_fn,"SDL2 Timer thread",NULL);
}
*/
