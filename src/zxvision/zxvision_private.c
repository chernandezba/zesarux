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