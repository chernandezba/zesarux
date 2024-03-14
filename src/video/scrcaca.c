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
#include <caca.h>

#include "cpu.h"
#include "scrcaca.h"
#include "screen.h"
#include "operaciones.h"
#include "mem128.h"
#include "debug.h"
#include "zx8081.h"
#include "audio.h"
#include "zxvision.h"
#include "utils.h"
#include "joystick.h"
#include "z88.h"
#include "settings.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "svi.h"
#include "pcw.h"

int scrcaca_imgwidth,scrcaca_imgheight;

int scrcaca_totalrealwidth;
int scrcaca_totalrealheight;
unsigned char *framebuffer_caca=NULL;

    caca_canvas_t *cv; caca_display_t *dp; caca_event_t ev;

int caca_last_message_shown_timer=0;
char caca_last_message_shown[DEBUG_MAX_MESSAGE_LENGTH];

static const char     *dither_antialias = "default";
static const char     *dither_charset   = "default";
static const char     *dither_color     = "default";
static const char     *dither_algo      = "none";



static unsigned int rmask = 0xff0000;
static unsigned int gmask = 0x00ff00;
static unsigned int bmask = 0x0000ff;
static unsigned int amask = 0;


static unsigned int bpp   = 24;
static unsigned int depth = 3;


static caca_dither_t  *dither           = NULL;

void scrcaca_get_image_params(void);


void scrcaca_z88_cpc_load_keymap(void)
{
	debug_printf (VERBOSE_INFO,"Loading keymap");
}

void scrcaca_messages_debug(char *s)
{

	sprintf (caca_last_message_shown,"%s",s);

//supuestamente 5 segundos (50*5)
caca_last_message_shown_timer=250;


}


//conversion de colores 0..15 de spectrum a color cacalib
//en spectrum: black, blue, red, magenta, green, cyan, yellow, white
int caca_text_colour_table[16]={
	//CACA_BLACK,   CACA_BLUE,     CACA_RED,     CACA_MAGENTA,     CACA_GREEN,     CACA_CYAN,     CACA_BROWN, CACA_LIGHTGRAY,
	CACA_BLACK,   CACA_BLUE,     CACA_RED,     CACA_MAGENTA,     CACA_GREEN,     CACA_CYAN,     CACA_YELLOW, CACA_LIGHTGRAY,
	CACA_DARKGRAY,CACA_LIGHTBLUE,CACA_LIGHTRED,CACA_LIGHTMAGENTA,CACA_LIGHTGREEN,CACA_LIGHTCYAN,CACA_YELLOW,CACA_WHITE
}; //No quiero que el amarillo sin brillo sea brown



//Rutina de putchar para menu
void scrcaca_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{
	int colorfg=caca_text_colour_table[tinta&15]; //importante mantenerlo en el limite de 16 colores
	int colorbg=caca_text_colour_table[papel&15];

	caca_set_color_ansi(cv,colorfg,colorbg);
        caca_printf(cv,x,y,"%c",caracter);

        //Para evitar warnings al compilar de "unused parameter"
        tinta=papel;
        papel=tinta;


}

void scrcaca_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel)
{

        tinta=tinta&15;
        papel=papel&15;

	//de momento nada de footer
        //para que no se queje el compilador de variables no usadas
        x++;
        y++;
        caracter++;
        tinta++;
        papel++;

}




void scrcaca_resize()
{
    debug_printf (VERBOSE_INFO,"caca resize");

scrcaca_get_image_params();

modificado_border.v=1;

}

void scrcaca_putpixel_final_rgb(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color_rgb GCC_UNUSED)
{
}

void scrcaca_putpixel_final(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color GCC_UNUSED)
{
}



void scrcaca_putpixel(int x,int y,unsigned int color)
{

	//printf ("x: %d y: %d color: %d   ",x,y,color);


	int offset=y*scrcaca_totalrealwidth*depth+x*depth;

	int r,g,b;

        b=spectrum_colortable[color] & 0xFF;
        g=(spectrum_colortable[color] >> 8 ) & 0xFF;
        r=(spectrum_colortable[color] >> 16 ) & 0xFF;

	//printf ("x: %d y: %d r:%d g:%d b:%d offset:%d\n",x,y,r,g,b,offset);

	framebuffer_caca[offset]=b;
	framebuffer_caca[offset+1]=g;
	framebuffer_caca[offset+2]=r;

}

void scrcaca_debug_registers(void)
{



char buffer[2048];

print_registers(buffer);

     //caca_printf(cv,0,1,"%s",buffer);

    //caca_refresh_display(dp);

//imprimir con caca_printf muy seguido al final hace petar la ventana. lo hacemos con printf
	printf ("%s\r",buffer);


return;



}


void scrcaca_refresca_border(void)
{

	scr_refresca_border();
}

void scrcaca_putchar_zx8081(int x,int y, z80_byte caracter)
{

        scr_putchar_zx8081_comun(x,y, caracter);


}

void scrcaca_refresca_pantalla_zx81(void)
{

scr_refresca_pantalla_y_border_zx8081();

}


void scrcaca_refresca_pantalla_solo_driver(void)
{
        //Como esto solo lo uso de momento para drivers graficos, de momento lo dejo vacio
}


void scrcaca_refresca_pantalla(void)
{


        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;


        if (MACHINE_IS_ZX8081) {
                scrcaca_refresca_pantalla_zx81();
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
	        	                scrcaca_refresca_border();
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

        else if (MACHINE_IS_PCW) {
                scr_refresca_pantalla_y_border_pcw();
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


	//printf ("caca_dither_bitmap scrcaca_imgwidth=%d scrcaca_imgheight=%d\n",scrcaca_imgwidth,scrcaca_imgheight);
    //caca_dither_bitmap(cv, 0, 0, scrcaca_imgwidth,scrcaca_imgheight, dither, framebuffer_caca);
    caca_dither_bitmap(cv, 0, 0, scrcaca_imgwidth,scrcaca_imgheight, dither, framebuffer_caca);

if (caca_last_message_shown_timer) {
	caca_last_message_shown_timer--;
	caca_set_color_ansi(cv,CACA_LIGHTGRAY,CACA_BLACK);
        caca_printf(cv,0,0,"%s",caca_last_message_shown);
}



	screen_render_menu_overlay_if_active();

        //Escribir footer
        draw_middle_footer();



	/* Refresh display */
	caca_refresh_display(dp);

sem_screen_refresh_reallocate_layers=0;

}


void scrcaca_deal_with_keys(int tecla,int pressrelease)
{

	//printf ("Tecla: %d\n",tecla);


        switch (tecla) {

                        //F1 pulsado
                        case CACA_KEY_F1:
                                util_set_reset_key(UTIL_KEY_F1,pressrelease);
                        break;

                        //F2 pulsado
                        case CACA_KEY_F2:
                                util_set_reset_key(UTIL_KEY_F2,pressrelease);
                        break;

                        //F3 pulsado
                        case CACA_KEY_F3:
                                util_set_reset_key(UTIL_KEY_F3,pressrelease);
                        break;

                        //F4 pulsado
                        case CACA_KEY_F4:
                                util_set_reset_key(UTIL_KEY_F4,pressrelease);
                        break;

                        //F5 pulsado
                        case CACA_KEY_F5:
                                util_set_reset_key(UTIL_KEY_F5,pressrelease);
                        break;

			//F6 es Ctrl / diamond
			//F7 es Alt / square

                        //F6 pulsado
                        case CACA_KEY_F6:
                                util_set_reset_key(UTIL_KEY_CONTROL_L,pressrelease);
                        break;

                        //F7 pulsado
                        case CACA_KEY_F7:
                                util_set_reset_key(UTIL_KEY_ALT_L,pressrelease);
                        break;

                        //F8 pulsado. osdkeyboard
                        case CACA_KEY_F8:
                                util_set_reset_key(UTIL_KEY_F8,pressrelease);
                        break;


                        //F9 pulsado. quickload
                        case CACA_KEY_F9:
                                util_set_reset_key(UTIL_KEY_F9,pressrelease);
                        break;

                        //F10 pulsado
                        case CACA_KEY_F10:
                                util_set_reset_key(UTIL_KEY_F10,pressrelease);
                        break;



			case CACA_KEY_ESCAPE:
				util_set_reset_key(UTIL_KEY_ESC,pressrelease);
			break;

                       case CACA_KEY_DELETE:
				util_set_reset_key(UTIL_KEY_DEL,pressrelease);
                        break;

                        case CACA_KEY_BACKSPACE:
				util_set_reset_key(UTIL_KEY_BACKSPACE,pressrelease);
                        break;

                        case CACA_KEY_HOME:
				joystick_possible_home_key(pressrelease);
                        break;

                        case CACA_KEY_END:
				            util_set_reset_key(UTIL_KEY_END,pressrelease);
                        break;

                        case CACA_KEY_LEFT:
                                util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
                        break;

                        case CACA_KEY_RIGHT:
                                util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
                        break;

                        case CACA_KEY_DOWN:
                                util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
                        break;

                        case CACA_KEY_UP:
                                util_set_reset_key(UTIL_KEY_UP,pressrelease);
                        break;



                        case CACA_KEY_TAB:
				util_set_reset_key(UTIL_KEY_TAB,pressrelease);
                        break;


                        //PgUp
                        case CACA_KEY_PAGEUP:
				util_set_reset_key(UTIL_KEY_PAGE_UP,pressrelease);
                        break;

                        //PgDn
                        case CACA_KEY_PAGEDOWN:
				util_set_reset_key(UTIL_KEY_PAGE_DOWN,pressrelease);
                        break;


                        default:
				ascii_to_keyboard_port_set_clear(tecla,pressrelease);

                        break;

                }



}




void scrcaca_end(void)
{

	debug_printf (VERBOSE_INFO,"Closing caca video driver");

	caca_free_dither(dither);
	dither = NULL;
	caca_free_display(dp);
	caca_free_canvas(cv);


//    caca_free_display(dp);
//    caca_free_canvas(cv);


}



void scrcaca_actualiza_tablas_teclado(void)
{

//int c;
int pressrelease=1;


	int tecla;

//printf ("scrcaca_actualiza_tablas_teclado\n");
    caca_event_t cev;
    while (caca_get_event(dp, CACA_EVENT_ANY, &cev, 0)) {

	//printf ("event: %d\n",cev.type);
        switch (cev.type) {

	//TODO: el RESIZE no lo recibo.... lo hago para cuando se pasa el raton por encima
	case CACA_EVENT_MOUSE_MOTION:
		scrcaca_resize();
		    scrcaca_imgwidth = caca_get_canvas_width(cv);
		    scrcaca_imgheight = caca_get_canvas_height(cv);
		    mouse_x=cev.data.mouse.x;
		    mouse_y=cev.data.mouse.y;

	            gunstick_x=mouse_x*scrcaca_totalrealwidth/scrcaca_imgwidth;
		    gunstick_y=mouse_y*scrcaca_totalrealheight/scrcaca_imgheight;


		    kempston_mouse_x=gunstick_x;
		    kempston_mouse_y=255-gunstick_y;


		debug_printf(VERBOSE_PARANOID,"CACA_EVENT_MOUSE_MOTION X=%d Y=%d kempston mouse: x: %d y: %d",mouse_x,mouse_y,kempston_mouse_x,kempston_mouse_y);
	break;

        case CACA_EVENT_RESIZE:
		debug_printf(VERBOSE_INFO,"CACA_EVENT_RESIZE");
            	scrcaca_resize();
            break;
	case CACA_EVENT_NONE:
	break;
	case CACA_EVENT_MOUSE_PRESS:
		util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,1);
		//mouse_left=1;
	break;
	case CACA_EVENT_MOUSE_RELEASE:
		util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,0);
		//mouse_left=0;
	break;
	case CACA_EVENT_QUIT:
	break;
	case CACA_EVENT_ANY:
	break;


        case CACA_EVENT_KEY_PRESS:

            tecla = cev.data.key.ch;
  	    //printf ("Tecla press: %d\n",tecla);
	    //temp if (tecla=='r') scrcaca_resize();
	    notificar_tecla_interrupcion_si_z88();
	    scrcaca_deal_with_keys(tecla,pressrelease);

	break;

	case CACA_EVENT_KEY_RELEASE:
		pressrelease=0;
            tecla = cev.data.key.ch;
            //printf ("Tecla release: %d\n",tecla);
            scrcaca_deal_with_keys(tecla,pressrelease);


	break;
    }
  }

}


z80_byte scrcaca_lee_puerto(z80_byte puerto_h,z80_byte puerto_l)
{

        //Para evitar warnings al compilar de "unused parameter"
        puerto_h=puerto_l;
        puerto_l=puerto_h;


	//en caca no se usa

	return 255;
}





//http://sunsite.ualberta.ca/Documentation/Graphics/cacalib-1.2/html_mono/cacalib.html



void scrcaca_get_image_params(void)
{

//tamanyo de la ventana caca
    scrcaca_imgwidth = caca_get_canvas_width(cv);
    scrcaca_imgheight = caca_get_canvas_height(cv);

//printf ("%d / %d = %d\n",scrcaca_imgwidth,(ANCHO_PANTALLA+LEFT_BORDER_NO_ZOOM*2*border_enabled.v),scrcaca_imgwidth/(ANCHO_PANTALLA+LEFT_BORDER_NO_ZOOM*2*border_enabled.v));

/*if (MACHINE_IS_Z88) {
zoom_x=1+scrcaca_imgwidth/640;
zoom_y=1+scrcaca_imgheight/(ALTO_PANTALLA+TOP_BORDER_NO_ZOOM+BOTTOM_BORDER_NO_ZOOM);
}
else {
zoom_x=1+scrcaca_imgwidth/(ANCHO_PANTALLA+LEFT_BORDER_NO_ZOOM*2*border_enabled.v);
zoom_y=1+scrcaca_imgheight/(ALTO_PANTALLA+TOP_BORDER_NO_ZOOM*border_enabled.v+BOTTOM_BORDER_NO_ZOOM*border_enabled.v);
}
*/

zoom_x=1+scrcaca_imgwidth/screen_get_emulated_display_width_no_zoom_border_en();
zoom_y=1+scrcaca_imgheight/screen_get_emulated_display_height_no_zoom_border_en();



set_putpixel_zoom();

/*
if (MACHINE_IS_Z88) {
scrcaca_totalrealwidth=640*zoom_x;
scrcaca_totalrealheight=ALTO_PANTALLA*zoom_y+TOP_BORDER+BOTTOM_BORDER;
}
else {
scrcaca_totalrealwidth=ANCHO_PANTALLA*zoom_x+LEFT_BORDER*2*border_enabled.v;
scrcaca_totalrealheight=ALTO_PANTALLA*zoom_y+TOP_BORDER*border_enabled.v+BOTTOM_BORDER*border_enabled.v;
}
*/

scrcaca_totalrealwidth=screen_get_emulated_display_width_zoom_border_en();
scrcaca_totalrealheight=screen_get_emulated_display_height_zoom_border_en();

debug_printf (VERBOSE_INFO,"cacawidth: %d cacaheight: %d scrcaca_totalrealwidth: %d scrcaca_totalrealheight: %d zoom_x: %d zoom_y: %d",scrcaca_imgwidth,scrcaca_imgheight,scrcaca_totalrealwidth,scrcaca_totalrealheight,zoom_x,zoom_y);


    caca_free_dither(dither);


//A diferencia de aalib, con cacalib tratamos con el tamanyo real que deseamos
//Luego al repintar la pantalla se hace sobre el tamanyo virtual de texto caca

    dither = caca_create_dither(bpp, scrcaca_totalrealwidth, scrcaca_totalrealheight,
                                depth * scrcaca_totalrealwidth,
                                rmask, gmask, bmask, amask);
    if (dither == NULL) {
        cpu_panic ("vo caca: caca_create_dither failed!\n");
    }

    caca_set_dither_antialias(dither, dither_antialias);
    caca_set_dither_charset(dither, dither_charset);
    caca_set_dither_color(dither, dither_color);
    caca_set_dither_algorithm(dither, dither_algo);

//liberamos memoria si conviene
if (framebuffer_caca!=NULL) free(framebuffer_caca);

//asignamos memoria
framebuffer_caca = malloc (scrcaca_totalrealwidth*depth*scrcaca_totalrealheight);


}



void scrcaca_set_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scrcaca_reset_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scrcaca_detectedchar_print(z80_byte caracter)
{
        printf ("%c",caracter);
        //flush de salida standard
        fflush(stdout);

}

//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrcaca_get_menu_width(void)
{
        return 32;
}


int scrcaca_get_menu_height(void)
{
        return 24;
}

/*
int scrcaca_driver_can_ext_desktop (void)
{
        return 0;
}
*/


int scrcaca_init (void)
{

debug_printf (VERBOSE_INFO,"Init cacalib Video Driver");

    /* Initialise libcaca */
    cv = caca_create_canvas(0, 0);
    dp = caca_create_display(cv);
    /* Set window title */
    caca_set_display_title(dp, "ZEsarUX");

    /* Choose drawing colours */
    //caca_set_color_ansi(cv, CUCUL_BLACK, CUCUL_WHITE);
    /* Draw a string at coordinates (0, 0) */
    //caca_putstr(cv, 0, 0, "This is a message");

scr_putpixel=scrcaca_putpixel;
scr_putpixel_final=scrcaca_putpixel_final;
scr_putpixel_final_rgb=scrcaca_putpixel_final_rgb;
scr_putchar_zx8081=scrcaca_putchar_zx8081;
scrcaca_get_image_params();

scr_debug_registers=scrcaca_debug_registers;
scr_messages_debug=scrcaca_messages_debug;
scr_putchar_menu=scrcaca_putchar_menu;
scr_putchar_footer=scrcaca_putchar_footer;

        scr_get_menu_width=scrcaca_get_menu_width;
        scr_get_menu_height=scrcaca_get_menu_height;
//scr_driver_can_ext_desktop=scrcaca_driver_can_ext_desktop;

scr_set_fullscreen=scrcaca_set_fullscreen;
scr_reset_fullscreen=scrcaca_reset_fullscreen;
scr_z88_cpc_load_keymap=scrcaca_z88_cpc_load_keymap;
scr_detectedchar_print=scrcaca_detectedchar_print;

//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
scr_set_driver_name("caca");

return 0;


}
