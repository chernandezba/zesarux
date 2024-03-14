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

/*

This is a template for a video driver. It can be used as an example for a graphical video driver,
like xwindows or fbdev. Some other drivers, like curses or cacalib works in a different way

Replace:
VIDEONAME_CAP with the video driver name in capital letters, like "XWINDOWS"
videoname with the video driver name in lowercase letters, like "xwindows"

*/



#include "scrvideoname.h"

#include "compileoptions.h"
#include "cpu.h"
#include "screen.h"
#include "charset.h"
#include "debug.h"
#include "zxvision.h"
#include "cpc.h"
#include "pcw.h"
#include "settings.h"

#include <string.h>
#include <stdio.h>


//Funcion de poner pixel en pantalla de driver, teniendo como entrada el color en RGB
void scrvideoname_putpixel_final_rgb(int x,int y,unsigned int color_rgb)
{
	//Putpixel Call (x,y,color_rgb);
}

//Funcion de poner pixel en pantalla de driver, teniendo como entrada el color indexado
void scrvideoname_putpixel_final(int x,int y,unsigned int color)
{
	//Putpixel Call (x,y,spectrum_colortable[color]);
}


void scrvideoname_putpixel(int x,int y,unsigned int color)
{

        if (menu_overlay_activo==0) {
                //Putpixel con menu cerrado
                scrvideoname_putpixel_final(x,y,color);
                return;
        }

        //Metemos pixel en layer adecuado
	buffer_layer_machine[y*ancho_layer_menu_machine+x]=color;

        //Putpixel haciendo mix
        screen_putpixel_mix_layers(x,y);

}


void scrvideoname_putchar_zx8081(int x,int y, z80_byte caracter)
{
        scr_putchar_zx8081_comun(x,y, caracter);
}

void scrvideoname_debug_registers(void)
{

	char buffer[2048];
	print_registers(buffer);

	printf ("%s\r",buffer);
}

void scrvideoname_messages_debug(char *s)
{
        printf ("%s\n",s);
        //flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
        fflush(stdout);

}

//Rutina de putchar para menu
void scrvideoname_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{

        z80_bit inverse,f;

        inverse.v=0;
        f.v=0;
        //128 y 129 corresponden a franja de menu y a letra enye minuscula
        if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';

        scr_putchar_menu_comun_zoom(caracter,x,y,inverse,tinta,papel,menu_gui_zoom);

}

//Rutina de putchar para footer window
void scrvideoname_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel)
{
    scr_putchar_footer_comun_zoom(caracter,x,y,tinta,papel);
}


void scrvideoname_set_fullscreen(void)
{

}


void scrvideoname_reset_fullscreen(void)
{

}


void scrvideoname_refresca_pantalla_zx81(void)
{

        scr_refresca_pantalla_y_border_zx8081();

}


void scrvideoname_refresca_border(void)
{
        scr_refresca_border();

}


//Common routine for a graphical driver
void scrvideoname_refresca_pantalla(void)
{

        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;


        if (MACHINE_IS_ZX8081) {


                //scr_refresca_pantalla_rainbow_comun();
                scrvideoname_refresca_pantalla_zx81();
        }

        else if (MACHINE_IS_SPECTRUM) {


                //modo clasico. sin rainbow
                if (rainbow_enabled.v==0) {
                        if (border_enabled.v) {
                                //ver si hay que refrescar border
                                if (modificado_border.v)
                                {
                                        scrvideoname_refresca_border();
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


        //printf ("%d\n",spectrum_colortable[1]);

	screen_render_menu_overlay_if_active();


        //Escribir footer
        draw_middle_footer();

sem_screen_refresh_reallocate_layers=0;

}


void scrvideoname_end(void)
{
}

z80_byte scrvideoname_lee_puerto(z80_byte puerto_h,z80_byte puerto_l)
{
	return 255;
}

void scrvideoname_actualiza_tablas_teclado(void)
{

	//Si se pulsa tecla y en modo z88, notificar interrupcion
	//if (pressrelease) notificar_tecla_interrupcion_si_z88();

}


void scrvideoname_z88_cpc_load_keymap(void)
{
	debug_printf (VERBOSE_INFO,"Loading keymap");
}

void scrvideoname_detectedchar_print(z80_byte caracter)
{
        printf ("%c",caracter);
        //flush de salida standard
        fflush(stdout);

}


//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrvideoname_get_menu_width(void)
{
        int max=screen_get_emulated_display_width_no_zoom_border_en()/menu_char_width/menu_gui_zoom;
        if (max>OVERLAY_SCREEN_MAX_WIDTH) max=OVERLAY_SCREEN_MAX_WIDTH;

                //printf ("max x: %d %d\n",max,screen_get_emulated_display_width_no_zoom_border_en());

        return max;
}


int scrvideoname_get_menu_height(void)
{
        int max=screen_get_emulated_display_height_no_zoom_border_en()/8/menu_gui_zoom;
        if (max>OVERLAY_SCREEN_MAX_HEIGTH) max=OVERLAY_SCREEN_MAX_HEIGTH;

                //printf ("max y: %d %d\n",max,screen_get_emulated_display_height_no_zoom_border_en());
        return max;
}

/*
int scrvideoname_driver_can_ext_desktop (void)
{
        return 0;
}
*/

int scrvideoname_init (void) {

	debug_printf (VERBOSE_INFO,"Init VIDEONAME_CAP Video Driver");


        //Inicializaciones necesarias
        scr_putpixel=scrvideoname_putpixel;
        scr_putpixel_final=scrvideoname_putpixel_final;
        scr_putpixel_final_rgb=scrvideoname_putpixel_final_rgb;

        scr_get_menu_width=scrvideoname_get_menu_width;
        scr_get_menu_height=scrvideoname_get_menu_height;
	//scr_driver_can_ext_desktop=scrvideoname_driver_can_ext_desktop;


        scr_putchar_zx8081=scrvideoname_putchar_zx8081;
        scr_debug_registers=scrvideoname_debug_registers;
        scr_messages_debug=scrvideoname_messages_debug;
        scr_putchar_menu=scrvideoname_putchar_menu;
	scr_putchar_footer=scrvideoname_putchar_footer;
        scr_set_fullscreen=scrvideoname_set_fullscreen;
        scr_reset_fullscreen=scrvideoname_reset_fullscreen;
	scr_z88_cpc_load_keymap=scrvideoname_z88_cpc_load_keymap;
	scr_detectedchar_print=scrvideoname_detectedchar_print;
        scr_tiene_colores=1;
        screen_refresh_menu=1;


        //Otra inicializacion necesaria
        //Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
        scr_set_driver_name("videoname");

	//importante: modificar funcion int si_complete_video_driver(void) de screen.c si este driver es completo, agregarlo ahi
	//importante: definicion de f_functions en menu.c, si driver permite teclas F


	scr_z88_cpc_load_keymap();

        return 0;

}


