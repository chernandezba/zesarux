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


#include "joystick.h"
#include "cpu.h"
#include "debug.h"
#include "screen.h"
#include "menu.h"
#include "zeng.h"

#ifdef COMPILE_CURSES
        #include "scrcurses.h"
#endif


int joystick_emulation=JOYSTICK_CURSOR_WITH_SHIFT;
int joystick_autofire_frequency=0;
int joystick_autofire_counter=0;

//auto left right
int joystick_autoleftright_enabled=0;
int joystick_autoleftright_frequency=1;
int joystick_autoleftright_counter=0;

//0=left, 1=right
int joystick_autoleftright_status=0;

//Que tecla actua como el Fire del joystick. Por defecto, Home
//#define JOYSTICK_KEY_FIRE_IS_HOME 0
//#define JOYSTICK_KEY_FIRE_IS_RIGHTALT 1
//#define JOYSTICK_KEY_FIRE_IS_RIGHTCTRL 2
//#define JOYSTICK_KEY_FIRE_IS_RIGHTSHIFT 3
int joystick_defined_key_fire=JOYSTICK_KEY_FIRE_IS_HOME;

char *joystick_defined_fire_texto[]={
    "Home",
    "RightAlt",
    "RightCtrl",
    "RightShift"
};

//Funciones que segun la tecla pulsada de posibles joystick fire (home, rightalt, etc)
//acaban actuando sobre el fire o sobre la tecla
void joystick_possible_home_key(int pressrelease)
{
    if (joystick_defined_key_fire==JOYSTICK_KEY_FIRE_IS_HOME) util_set_reset_key(UTIL_KEY_FIRE,pressrelease);
    else util_set_reset_key(UTIL_KEY_HOME,pressrelease);
}


void joystick_possible_rightshift_key(int pressrelease)
{
    if (joystick_defined_key_fire==JOYSTICK_KEY_FIRE_IS_RIGHTSHIFT) util_set_reset_key(UTIL_KEY_FIRE,pressrelease);
    else util_set_reset_key(UTIL_KEY_SHIFT_R,pressrelease);
}

void joystick_possible_rightalt_key(int pressrelease)
{

    if (joystick_defined_key_fire==JOYSTICK_KEY_FIRE_IS_RIGHTALT) util_set_reset_key(UTIL_KEY_FIRE,pressrelease);
    else util_set_reset_key(UTIL_KEY_ALT_R,pressrelease);
}

void joystick_possible_rightctrl_key(int pressrelease)
{            
    if (joystick_defined_key_fire==JOYSTICK_KEY_FIRE_IS_RIGHTCTRL) util_set_reset_key(UTIL_KEY_FIRE,pressrelease);
    else util_set_reset_key(UTIL_KEY_CONTROL_R,pressrelease);
}


int gunstick_emulation=0;

//Coordenadas x,y en formato scanlines y pixeles totales, es decir,
//x entre 0 y 351 
//y entre 0 y 295
//0,0 esta arriba a la izquierda

int gunstick_x,gunstick_y;

//rangos de deteccion de electron, para pistola magnum light gun
int gunstick_range_x=64;
int gunstick_range_y=8;

int gunstick_y_offset=0;

//si detecta solo zonas con blanco y brillo 1. sino, detecta blanco sea con brillo o no
int gunstick_solo_brillo=0;

//Coordenadas x,y tal cual las retorna el driver de video, segun el tamanyo de ventana activo
//en xwindows, normalmente entre 
//x entre 0 y 351
//y entre 0 y 295
//0,0 esta arriba a la izquierda
//con zoom 1
//Con zoom 2, el doble:
//x entre 0 y 703
//y entre 0 y 591
//Para cacalib, tambien devuelve coordenadas dentro del tamanyo de la ventana
//por defecto, 80x32
int mouse_x=0,mouse_y=0;

int mouse_pressed_close_window=0;
int mouse_pressed_background_window=0;


int mouse_pressed_hotkey_window=0;
//y que tecla puslada
int mouse_pressed_hotkey_window_key=0;

//si esta activa la emulacion de kempston mouse
z80_bit kempston_mouse_emulation;

//pulsado boton izquierdo razon
int mouse_left=0;
//pulsado boton derecho raton
int mouse_right=0;

//accionado wheel vertical. Positivo: scroll arriba. Negativo: scroll abajo
int mouse_wheel_vertical=0;

//accionado wheel horizontal. Positivo: scroll izquierda. Negativo: scroll derecha
int mouse_wheel_horizontal=0;

//Coordenadas x,y de retorno a puerto kempston
//Entre 0 y 255 las dos. Coordenada Y hacia abajo resta
//se toma como base el mismo formato que gunstick x e y pero con modulo % 256
z80_byte kempston_mouse_x=0,kempston_mouse_y=0;



z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

//z80_byte puerto_especial_gunstick=0; //Fire 0 o 1

char *joystick_texto[]={
        "None",
        "Kempston",
        "Sinclair 1",
        "Sinclair 2",
        "Cursor",
        "Cursor&Shift",
	"OPQA Space",
        "Fuller",
	"Zebra ZX81",
	"MikroGen ZX81",
	"ZXpand ZX81",
	"Cursor Sam",
    "CPC Joy1",
    "MSX",
    "Spectravideo"
};

char *gunstick_texto[]={
        "None",
        "Sinclair 1",
        "Sinclair 2",
        "Kempston",
	"AYChip",
	"Port DFH"
};


void joystick_cycle_next_type(void)
{
        if (joystick_emulation==JOYSTICK_TOTAL) joystick_emulation=0;
        else joystick_emulation++;

	//Si no esta autofire
        if (menu_hardware_autofire_cond()==0) {
                //desactivamos autofire
                joystick_autofire_frequency=0;
                //y ponemos tecla fire a 0, por si se habia quedado activa
                puerto_especial_joystick=0;
        }
}

//Liberar auto left right si conviene, y quitar direccion actual
void joystick_clear_leftright(void)
{
    if (joystick_autoleftright_enabled) {
        joystick_autoleftright_enabled=0;
        puerto_especial_joystick &=(255-3); //quitar left y right
        //quitar linea de footer LEFT/RIGHT
        delete_generic_footertext();
    }
}


void joystick_set_right(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
        puerto_especial_joystick |=1;
	debug_printf(VERBOSE_DEBUG,"joystick_set_right");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_RIGHT,1);
}

void joystick_release_right(int si_enviar_zeng_event)
{
        puerto_especial_joystick &=255-1;
	debug_printf(VERBOSE_DEBUG,"joystick_release_right");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_RIGHT,0);
}


void joystick_set_left(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
        puerto_especial_joystick |=2;
	debug_printf(VERBOSE_DEBUG,"joystick_set_left");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_LEFT,1);
}

void joystick_release_left(int si_enviar_zeng_event)
{
        puerto_especial_joystick &=255-2;
	debug_printf(VERBOSE_DEBUG,"joystick_release_left");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_LEFT,0);
}



void joystick_set_down(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
        puerto_especial_joystick |=4;
	debug_printf(VERBOSE_DEBUG,"joystick_set_down");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_DOWN,1);
}

void joystick_release_down(int si_enviar_zeng_event)
{
        puerto_especial_joystick &=255-4;
	debug_printf(VERBOSE_DEBUG,"joystick_release_down");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_DOWN,0);
}

void joystick_set_up(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
        puerto_especial_joystick |=8;
	debug_printf(VERBOSE_DEBUG,"joystick_set_up");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_UP,1);
}

void joystick_release_up(int si_enviar_zeng_event)
{
        puerto_especial_joystick &=255-8;
	debug_printf(VERBOSE_DEBUG,"joystick_release_up");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_UP,0);
}

void joystick_set_fire(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

        //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
        puerto_especial_joystick |=16;
	debug_printf(VERBOSE_DEBUG,"joystick_set_fire");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_FIRE,1);
}

void joystick_release_fire(int si_enviar_zeng_event)
{
        puerto_especial_joystick &=255-16;
	debug_printf(VERBOSE_DEBUG,"joystick_release_fire");

	if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_FIRE,0);
}


//Ver si la zona donde apunta el raton (gunstick) esta en blanco. Usado en gunstick de MHT
int gunstick_view_white(void)
{

//Si curses
#ifdef COMPILE_CURSES
	if (!strcmp(scr_new_driver_name,"curses")) {
		return (scrcurses_return_gunstick_view_white() );
	}
#endif

	//ver zona en blanco
	//Proteccion para que no se salga de putpixel_cache
	//dimensiones putpixel_cache
	int ancho,alto;
	//ancho=LEFT_BORDER_NO_ZOOM+ANCHO_PANTALLA+RIGHT_BORDER_NO_ZOOM;
	//alto=TOP_BORDER_NO_ZOOM+ALTO_PANTALLA+BOTTOM_BORDER_NO_ZOOM;

	ancho=screen_get_emulated_display_width_no_zoom();
	alto=screen_get_emulated_display_height_no_zoom();

	if (gunstick_x<ancho && gunstick_y<alto) {
		int indice_cache;
                                       
		indice_cache=(get_total_ancho_rainbow()*gunstick_y)+gunstick_x;
                                        
		z80_byte color=putpixel_cache[indice_cache];
                                        
		//color blanco con o sin brillo
		if (color==15 || color==7) {
			debug_printf (VERBOSE_DEBUG,"white zone detected on lightgun");
			return 1;
		}
	}

	return 0;
}

//Ver si la zona donde apunta el raton (gunstick) esta pasando el electron. Usado en magnum light phaser
int gunstick_view_electron(void)
{
   //ver electron

   //x inicialmente esta posicionada dentro de pantalla... sin contar border
   int x=t_estados % screen_testados_linea;
   int y=t_estados/screen_testados_linea;

   //restamos zona no visible superior
   //printf ("y calculada: %d t_scanline_draw: %d\n",y,t_scanline_draw);
   y -=screen_invisible_borde_superior;

   x=x+screen_testados_total_borde_izquierdo;

   if (x>=screen_testados_linea) {
        y++;
        x=x-screen_testados_linea;
   }

   //x esta en t_estados. pasamos a pixeles

   x=x*2;

   debug_printf (VERBOSE_PARANOID,"electron is at t_estados: %d x: %d y: %d. gun is at x: %d y: %d",t_estados,x,y,gunstick_x,gunstick_y);

	//aproximacion. solo detectamos coordenada y. Parece que los juegos no hacen barrido de toda la x.
	//TODO. los juegos que leen pistola asi no funcionan
	//if (y==gunstick_y) return 1;

	//rango de y
	int dif=y-gunstick_y;
	if (dif<0) dif=-dif;


	if (dif<gunstick_range_y) {

		debug_printf (VERBOSE_DEBUG,"gunstick y (%d) is in range of electron (%d)",gunstick_y,y);

		//Proteccion para que no se salga de putpixel_cache
   		//dimensiones putpixel_cache
	        int ancho,alto;

	        ancho=screen_get_emulated_display_width_no_zoom();
        	alto=screen_get_emulated_display_height_no_zoom();
     
        	if (x<ancho && y<alto) {

			//Ver si hay algo en blanco cerca de donde se ha disparado
                                              int indice_cache;
						//x=gunstick_x;

			int rango_y;
			y=gunstick_y-gunstick_range_y/2;

			//Restar offset
			y=y-gunstick_y_offset;

			if (y<0) y=0;
			for (rango_y=gunstick_range_y;rango_y>0;rango_y--,y++) {


				x=gunstick_x-gunstick_range_x/2;
				if (x<0) x=0;

                                indice_cache=(get_total_ancho_rainbow()*y)+x;

				int rango_x;
				z80_byte color;
				for (rango_x=gunstick_range_x;rango_x>0;rango_x--) {

                                	color=putpixel_cache[indice_cache];

                                        //color blanco con o sin brillo
					//si no es color valido	
					if (color>15) return 0;

					int maskbrillo=7;
					if (gunstick_solo_brillo) maskbrillo=15;

                                        if ( (color&maskbrillo)==maskbrillo) {
						debug_printf (VERBOSE_DEBUG,"White zone detected on lightgun. gunstick x: %d y: %d, color=%d",gunstick_x,gunstick_y,color);
						return 1;
					}
					indice_cache++;

					//printf ("rango_x: %d rango_y: %d x: %d y: %d\n",rango_x,rango_y,x,y);

				}
			}
		}

	}

  return 0;

   

  //}

}


void joystick_print_types(void)
{
	int i;

	for (i=0;i<=JOYSTICK_TOTAL;i++) {
		printf ("%s",joystick_texto[i]);
		if (i!=JOYSTICK_TOTAL) printf (", ");
	}
}
