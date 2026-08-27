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
   Private menu interface functions, also known as "ZX Vision"
*/

//
// Archivo para funciones auxiliares de soporte de menu, funciones y objetos privados que no tienen por que verlo desde los menu*.c
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>



#include "zxvision.h"
#include "zxvision_private.h"
#include "zxvision_topbar.h"
#include "menu_items.h"
#include "menu_items_settings.h"
#include "menu_items_storage.h"
#include "menu_bitmaps.h"
#include "menu_debug_cpu.h"
#include "menu_file_viewer_browser.h"
#include "menu_filesel.h"
#include "menu_zeng_online.h"
#include "screen.h"
#include "cpu.h"
#include "start.h"
#include "debug.h"
#include "ay38912.h"
#include "tape.h"
#include "audio.h"
#include "timer.h"
#include "operaciones.h"
#include "utils.h"
#include "utils_math.h"
#include "joystick.h"
#include "ula.h"
#include "realjoystick.h"
#include "scrstdout.h"
#include "autoselectoptions.h"
#include "charset.h"
#include "chardetect.h"
#include "textspeech.h"
#include "prism.h"
#include "cpc.h"
#include "sam.h"
#include "tbblue.h"
#include "remote.h"
#include "tsconf.h"
#include "settings.h"
#include "stats.h"
#include "network.h"
#include "ql.h"
#include "zvfs.h"
#include "pcw.h"
#include "pd765.h"
#include "dsk.h"
#include "zeng_online_client.h"
#include "ql_qdos_handler.h"
#include "ql_i8049.h"

#if defined(__APPLE__)
    #include <sys/syslimits.h>

    #include <sys/resource.h>

#endif

#include "compileoptions.h"

#ifdef COMPILE_CURSES
    #include "scrcurses.h"
#endif

#ifdef COMPILE_AA
    #include "scraa.h"
#endif

#ifdef COMPILE_STDOUT
    #include "scrstdout.h"
//macro llama a funcion real
    #define scrstdout_menu_print_speech_macro scrstdout_menu_print_speech
//funcion llama
#else
//funcion no llama a nada
    #define scrstdout_menu_print_speech_macro(x)
#endif


#ifdef COMPILE_XWINDOWS
    #include "scrxwindows.h"
#endif


//si estaba visible o no
z80_bit switchzxdesktop_button_visible={0};


//Ver cuantos iconos hay cerca para saber si se puede posicionar uno o no
int zxvision_si_icono_cerca(int x,int y)
{
    int xminimo=x-ZESARUX_ASCII_LOGO_ANCHO;
    int xmaximo=x+ZESARUX_ASCII_LOGO_ANCHO;

    int yminimo=y-ZESARUX_ASCII_LOGO_ANCHO;
    int ymaximo=y+ZESARUX_ASCII_LOGO_ANCHO;

    int i;

    int iconos_cerca=0;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            int icon_x=zxdesktop_configurable_icons_list[i].pos_x;
            int icon_y=zxdesktop_configurable_icons_list[i].pos_y;

            //Si hay uno cerca de ahi, volver con 1
            if (icon_x>=xminimo && icon_x<xmaximo && icon_y>=yminimo && icon_y<ymaximo) iconos_cerca++;

        }
    }

    return iconos_cerca;

}

//Retorna la coordenada y minima que puede tener un icono en el zx desktop
//coordenada sin considerar zoom_y
int zxvision_get_minimum_y_icon_position(void)
{
    //yinicial debajo de botones superiores
    int alto_boton;
    menu_ext_desktop_buttons_get_geometry(NULL,&alto_boton,NULL,NULL,NULL);
    alto_boton /=zoom_y;


    int yinicial=alto_boton+16;

    //Si no hay menus superiores activados
    if (menu_zxdesktop_upper_buttons_enabled.v==0) {
        yinicial=0;
    }

    //Si está topbar, la yinicial es mas arriba
    if (zxvision_topbar_menu_enabled.v) {
        //1 fila de texto. con algo de margen
        yinicial=menu_char_height*2;
    }

    //printf("yinicial: %d\n",yinicial);

    return yinicial;
}

void zxvision_get_start_valid_positions_icons(int *p_xinicial,int *p_xfinal,int *p_yinicial,int *p_yfinal)
{
    /*

    Zona de forma cuadrada delimitada por:
    x: desde derecha pantalla emulada hasta final x
    y: desde debajo botones superiores hasta por encima botones inferiores

    */


    int inicio_x_zxdesktop=screen_get_emulated_display_width_no_zoom_border_en();


    //Empezar a ubicarlos con algo de margen
    int xinicial=inicio_x_zxdesktop+24;

    int xfinal=screen_get_total_width_window_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;

    int yinicial=zxvision_get_minimum_y_icon_position();

    //Hasta llegar a los iconos de dispositivos inferiores
    int yfinal;
    menu_ext_desktop_lower_buttons_get_geometry(NULL,NULL,NULL,NULL,NULL,&yfinal);
    //Posiciones menos el zoom
    yfinal /=zoom_y;
    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    yfinal -=ZESARUX_ASCII_LOGO_ANCHO;

    *p_xinicial=xinicial;
    *p_xfinal=xfinal;
    *p_yinicial=yinicial;
    *p_yfinal=yfinal;

}


//Dice si algun icono custom en el escritorio es la papelera
//-1 si no
int zxvision_search_trash_configurable_icon(void)
{
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
            //id de la tabla de acciones
            int id_tabla=zxdesktop_configurable_icons_list[i].indice_funcion;

            enum defined_f_function_ids id_funcion=defined_direct_functions_array[id_tabla].id_funcion;

            if (id_funcion==F_FUNCION_DESKTOP_TRASH) return i;
        }
    }

    return -1;
}



int if_zxdesktop_trash_not_empty(void)
{
    int i;

    for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
        if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_DELETED) {
            return 1;
        }
    }

    return 0;
}

//Boton habilitado y ademas visible (visibilidad determinada por movimiento de raton)
int zxvision_if_lower_button_switch_zxdesktop_visible(void)
{
    if (zxvision_if_lower_button_switch_zxdesktop_enabled() && switchzxdesktop_button_visible.v) return 1;
    else return 0;
}

int zxvision_if_lower_button_switch_zxdesktop_enabled(void)
{

    //con emulacion de kempston mouse o lightgun, no se dispara evento de abrir menu al pulsar con raton, por tanto,
    //no se puede gestionar pulsaciones sobre el boton de switch

    if (si_complete_video_driver() && scr_driver_can_ext_desktop() &&
        menu_footer && zxdesktop_switch_button_enabled.v && !ventana_fullscreen &&
        mouse_menu_disabled.v==0 && kempston_mouse_emulation.v==0 && lightgun_emulation_enabled.v==0 && ql_qimi_mouse_enabled==0) return 1;
    else return 0;
}


//Si ampliar_reducir_ancho=1, dice si posicion ampliar alto
//Si no, dice posicion de reducir alto
int zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_height(int ampliar_reducir_alto)
{
    if (zxvision_if_lower_button_switch_zxdesktop_visible() && mouse_left) {

        int x,y,xboton,yboton;

        zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common(&x,&y,&xboton,&yboton);

        //Los de cambio de alto estan en posicion x -1
        xboton--;

        //Boton abajo: ampliar alto
        if (ampliar_reducir_alto) {
            if (x==xboton && y==yboton+1) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop enlarge height button");
                return 1;
            }
        }

        //Boton arriba: reducir alto
        else {
            if (x==xboton && y==yboton) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop reduce height button");
                return 1;
            }
        }


    }

    return 0;
}


int zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_width(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_width(1);
}

int zxvision_if_mouse_in_lower_button_reduce_zxdesktop_width(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_width(0);
}

int zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_height(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_height(1);
}

int zxvision_if_mouse_in_lower_button_reduce_zxdesktop_height(void)
{
    return zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_height(0);
}



//Comun para obtener posicion de raton y de botones de zxdesktop
void zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common(int *p_x,int *p_y,int *p_xboton,int *p_yboton)
{

    //TODO: Estas posiciones donde estan los botones, se obtienen de manera distinta en las funciones:
    //menu_put_switch_zxdesktop_footer
    //zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common
    //Aunque se obtienen de diferentes maneras pero el resultado final (en teoria) es el mismo

    int mouse_pixel_x,mouse_pixel_y;
    menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

    //printf("si pulsado en boton switch zxdesktop. x %d y %d\n",mouse_pixel_x,mouse_pixel_y);

    int x=mouse_x;
    int y=mouse_y;
    //Quitarle el zoom
    x=x/zoom_x;
    y=y/zoom_y;

    //y la escala de 8
    x /=8;
    y /=8;
    //printf("si pulsado en boton switch zxdesktop. mouse_x %d mouse_y %d\n",x,y);

    //donde esta el boton
    //int yboton=screen_get_emulated_display_height_no_zoom_border_en()/8;
    int yboton=(screen_get_emulated_display_height_no_zoom_border_en()+screen_get_ext_desktop_height_no_zoom()) /8;

    int xboton=screen_get_window_size_width_no_zoom_border_en()/8-2; //justo 2 posicion menos
    //esta es la posicion x de los botones de +- ancho zx desktop
    //printf("si pulsado en boton switch zxdesktop. xboton %d yboton %d x %d y %d\n",xboton,yboton,x,y);

    *p_x=x;
    *p_y=y;
    *p_xboton=xboton;
    *p_yboton=yboton;
}



//Si ampliar_reducir_ancho=1, dice si posicion de arriba de ampliar ancho
//Si no, dice posicion de abajo de reducir ancho
int zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_width(int ampliar_reducir_ancho)
{
    if (zxvision_if_lower_button_switch_zxdesktop_visible() && mouse_left) {

        int x,y,xboton,yboton;

        zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common(&x,&y,&xboton,&yboton);

        //Boton arriba: ampliar ancho
        if (ampliar_reducir_ancho) {
            if (x==xboton && y==yboton) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop enlarge width button");
                return 1;
            }
        }

        //Boton abajo: reducir ancho
        else {
            if (x==xboton && y==yboton+1) {
                debug_printf(VERBOSE_INFO,"Pressed on ZX Desktop reduce width button");
                return 1;
            }
        }


    }

    return 0;
}