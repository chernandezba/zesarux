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

int zxvision_draw_icon_papelera_abierta=0;
overlay_screen footer_screen_array[WINDOW_FOOTER_COLUMNS*WINDOW_FOOTER_LINES];
int cuadrado_activo_resize=0;
int ventana_activa_puede_minimizar=0;
int ventana_es_background=0;
int menu_clipboard_size=0;
int current_win_x,current_win_y,current_win_ancho,current_win_alto,current_win_minimize_button_position;
int ventana_tipo_activa=1;
char *string_config_key_aid_startup=NULL;
int menu_dibuja_cuadrado_putpixel_background_ultima_fila=-1;
int menu_dibuja_cuadrado_putpixel_background_ultima_columna=-1;
int no_dibuja_ventana_muestra_pending_error_message=0;
int zxvision_mouse_x=0;
int zxvision_mouse_y=0;
zxvision_window *menu_dibuja_submenu_primer_submenu=NULL;
zxvision_window zxvision_window_splash_text;
int window_is_being_moved=0;
zxvision_window *menu_dibuja_cuadrado_putpixel_background_ventana=NULL;
int ventana_marca_redimensionado_raton_encima=0;
int splash_zesarux_logo_active=0;
int menu_dibuja_cuadrado_putpixel_background_ultima_dibujar=1;
char zxvision_switch_to_window_on_open_menu_name[MAX_NAME_WINDOW_GEOMETRY];
z80_bit force_next_menu_position={0};
z80_bit menu_disable_special_chars={0};
int footer_last_cpu_use=0;


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

void deletechar_menu_overlay(int x,int y)
{
    putchar_menu_overlay_parpadeo(x,y,0,0,0,0);
}

void cls_footer(void)
{
    if (!menu_footer) return;

    int x,y;
    for (y=0;y<WINDOW_FOOTER_LINES;y++) {
        for (x=0;x<WINDOW_FOOTER_COLUMNS;x++) {
            putchar_footer_array(x,y,' ',WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER,0);
        }
    }

}

char **get_direct_function_icon_bitmap_final(int id_accion)
{
    char **bitmap=defined_direct_functions_array[id_accion].bitmap_button;
    bitmap=alter_zesarux_ascii_logo(bitmap);

    //Si icono es papelera, decir que cambiamos si la papelera no esta vacia

    enum defined_f_function_ids id_funcion=defined_direct_functions_array[id_accion].id_funcion;

    //Comportamiento icono diferente para trash
    if (id_funcion==F_FUNCION_DESKTOP_TRASH) {
        //Ver si papelera no esta vacia
        if (if_zxdesktop_trash_not_empty()) {
            bitmap=bitmap_button_ext_desktop_trash_not_empty;

            //Y si papelera abierta, porque se esta arrastrando algun icono cerca
            if (zxvision_draw_icon_papelera_abierta) {
              bitmap=bitmap_button_ext_desktop_trash_open_not_empty;
            }
        }

        else {
            //Papelera vacia

            //Y si papelera abierta, porque se esta arrastrando algun icono cerca
            if (zxvision_draw_icon_papelera_abierta) {
                bitmap=bitmap_button_ext_desktop_trash_open;
            }
        }
    }

    //Comportamiento icono diferente para My Machine
    if (id_funcion==F_FUNCION_DESKTOP_MY_MACHINE) {
        bitmap=menu_ext_desktop_draw_configurable_icon_return_machine_icon();
    }


    return bitmap;
}

void menu_dibuja_cuadrado_putpixel_background_reset_cache(void)
{
    menu_dibuja_cuadrado_putpixel_background_ultima_fila=-1;
    menu_dibuja_cuadrado_putpixel_background_ultima_columna=-1;
}

void menu_calculate_mouse_xy(void)
{
    int x,y;
    if (menu_allows_mouse() ) {
        menu_calculate_mouse_xy_absolute_interface(&x,&y);
    /*
        int mouse_en_emulador=0;
        //printf ("x: %04d y: %04d\n",mouse_x,mouse_y);

        int ancho=screen_get_window_size_width_zoom_border_en();

        ancho +=screen_get_ext_desktop_width_zoom();

        if (mouse_x>=0 && mouse_y>=0
            && mouse_x<=ancho && mouse_y<=screen_get_window_size_height_zoom_border_en() ) {
                //Si mouse esta dentro de la ventana del emulador
                mouse_en_emulador=1;
        }

        if (  (mouse_x!=last_mouse_x || mouse_y !=last_mouse_y) && mouse_en_emulador) {
            mouse_movido=1;
        }
        else mouse_movido=0;

        last_mouse_x=mouse_x;
        last_mouse_y=mouse_y;

        //printf ("x: %04d y: %04d movido=%d\n",mouse_x,mouse_y,mouse_movido);

        //Quitarle el zoom
        x=mouse_x/zoom_x;
        y=mouse_y/zoom_y;

        //Considerar borde pantalla

        //Todo lo que sea negativo o exceda border, nada.

        //printf ("x: %04d y: %04d\n",x,y);



        //margenes de zona interior de pantalla. para modo rainbow
                int margenx_izq;
                int margeny_arr;
                menu_retorna_margenes_border(&margenx_izq,&margeny_arr);

    //Ya no hace falta restar margenes
    margenx_izq=margeny_arr=0;

    x -=margenx_izq;
    y -=margeny_arr;

    //printf ("x: %04d y: %04d\n",x,y);

    //Aqui puede dar negativo, en caso que cursor este en el border
    //si esta justo en los ultimos 8 pixeles, dara entre -7 y -1. al dividir entre 8, retornaria 0, diciendo erroneamente que estamos dentro de ventana

    if (x<0) x-=(menu_char_width*menu_gui_zoom); //posicion entre -7 y -1 y demas, cuenta como -1, -2 al dividir entre 8
    if (y<0) y-=(8*menu_gui_zoom);

    x /=menu_char_width;
    y /=8;

    x /= menu_gui_zoom;
    y /= menu_gui_zoom;

    //printf ("antes de restar: %d,%d\n",x,y);
    */

    zxvision_mouse_x=x;
    zxvision_mouse_y=y;

    x -=current_win_x;
    y -=current_win_y;

    menu_mouse_x=x;
    menu_mouse_y=y;

    //if (x<=0 || y<=0) printf ("x: %04d y: %04d final\n",x,y);

    //printf ("ventana_x %d margen_izq %d\n",ventana_x,margenx_izq);

    //Coordenadas menu_mouse_x y tienen como origen 0,0 en zona superior izquierda de ventana (titulo ventana)
    //Y en coordenadas de linea (y=0 primera linea, y=1 segunda linea, etc)

    }
}

int menu_calcular_ancho_string_item(char *texto)
{
    //Devuelve longitud de texto teniendo en cuenta de no sumar caracteres ~~ o ^^ o $$X
    unsigned int l;
    int ancho_calculado=strlen(texto);

    for (l=0;l<strlen(texto);l++) {
            if (menu_escribe_texto_si_inverso(texto,l)) ancho_calculado-=2;
            if (menu_escribe_texto_si_parpadeo(texto,l)) ancho_calculado-=2;
            if (menu_escribe_texto_si_cambio_tinta(texto,l)) ancho_calculado-=3;
    }

    return ancho_calculado;
}

void menu_add_item_menu_no_indexar_busqueda(menu_item *m)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    m->no_indexar_busqueda=1;
}

void menu_add_item_menu_catalan(menu_item *m,char *s)
{
    //busca el ultimo item i le añade el indicado

    while (m->siguiente_item!=NULL)
    {
            m=m->siguiente_item;
    }

    strcpy(m->texto_opcion_catalan,s);

}

void menu_add_item_menu_funcion_texto_item(menu_item *m,char *(*menu_funcion_texto_item)(struct s_menu_item *))
{
//busca el ultimo item i le añade el indicado

        while (m->siguiente_item!=NULL)
        {
                m=m->siguiente_item;
        }

        m->menu_funcion_texto_item=menu_funcion_texto_item;
}

void menu_copy_clipboard(char *texto)
{

    //Si puntero no NULL, liberamos clipboard anterior
    if (menu_clipboard_pointer!=NULL) {
        debug_printf(VERBOSE_INFO,"Freeing previous clipboard memory");
        free(menu_clipboard_pointer);
        menu_clipboard_pointer=NULL;
    }

    //Si puntero NULL, asignamos memoria
    if (menu_clipboard_pointer==NULL) {
        menu_clipboard_size=strlen(texto);
        debug_printf(VERBOSE_INFO,"Allocating %d bytes to clipboard",menu_clipboard_size+1);
        menu_clipboard_pointer=malloc(menu_clipboard_size+1); //+1 del 0 final
        if (menu_clipboard_pointer==NULL) {
            debug_printf(VERBOSE_ERR,"Error allocating clipboard memory");
            return;
        }
        strcpy((char *)menu_clipboard_pointer,texto);
    }


}

void menu_draw_cpu_use_last(void)
{

    int cpu_use=menu_last_cpu_use;
    debug_printf (VERBOSE_PARANOID,"cpu: %d",cpu_use );

    //error
    if (cpu_use<0) return;

    //control de rango
    if (cpu_use>100) cpu_use=100;
    if (cpu_use<0) cpu_use=0;

    //temp
    //cpu_use=100;

    //printf ("mostrando cpu use\n");

    char buffer_perc[9];
    sprintf (buffer_perc,"%3d%% CPU",cpu_use);

    footer_last_cpu_use=cpu_use;

    int x;

    x=WINDOW_FOOTER_ELEMENT_X_CPU_USE;

    int color_tinta=WINDOW_FOOTER_INK;

    //Color en rojo si uso cpu sube
    if (cpu_use>=85) color_tinta=ESTILO_GUI_COLOR_AVISO;

    menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_CPU_USE,buffer_perc,color_tinta,WINDOW_FOOTER_PAPER);

}

void menu_draw_last_fps(void)
{


        //int fps=ultimo_fps;
        int fps=sensor_get_value("fps");
        debug_printf (VERBOSE_PARANOID,"FPS: %d",fps);

        //algun error al leer fps
        if (fps<0) return;

        //control de rango
        if (fps>50) fps=50;

        //const int ancho_maximo=6;
        #define DRAW_FPS_ANCHO_MAXIMO 6

            //printf ("mostrando fps\n");

        char buffer_fps[DRAW_FPS_ANCHO_MAXIMO+1];
        sprintf (buffer_fps,"%02d FPS",fps);

        //primero liberar esas zonas
        int x;


        //luego escribimos el texto
        x=WINDOW_FOOTER_ELEMENT_X_FPS;


        int color_tinta=WINDOW_FOOTER_INK;

    //Color en rojo si uso fps bajo sube
    if (fps<10) color_tinta=ESTILO_GUI_COLOR_AVISO;


    menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_FPS,buffer_fps,color_tinta,WINDOW_FOOTER_PAPER);

}

void menu_escribe_texto(int x,int y,int tinta,int papel,char *texto)
{
        unsigned int i;
    z80_byte letra;

    int parpadeo=0;

    int era_utf=0;

    //y luego el texto
    for (i=0;i<strlen(texto);i++) {
        letra=texto[i];

        //Si dos ^ seguidas, invertir estado parpadeo
        if (menu_escribe_texto_si_parpadeo(texto,i)) {
            parpadeo ^=1;
            //y saltamos esos codigos de negado
                        i +=2;
                        letra=texto[i];
        }

        //codigo control color tinta
        if (menu_escribe_texto_si_cambio_tinta(texto,i)) {
            tinta=texto[i+2]-'0';
            i+=3;
            letra=texto[i];
        }

        //ver si dos ~~ seguidas y cuidado al comparar que no nos vayamos mas alla del codigo 0 final
        if (menu_escribe_texto_si_inverso(texto,i)) {
            //y saltamos esos codigos de negado
            i +=2;
            letra=texto[i];

            if (menu_writing_inverse_color.v) putchar_menu_overlay_parpadeo(x,y,letra,papel,tinta,parpadeo);
            else putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
        }

        else {

            //Si estaba prefijo utf activo

            if (era_utf) {
                letra=menu_escribe_texto_convert_utf(era_utf,letra);
                era_utf=0;

                //Caracter final utf
                putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
            }


            //Si no, ver si entra un prefijo utf
            else {
                //printf ("letra: %02XH\n",letra);
                //Prefijo utf
                            if (menu_es_prefijo_utf(letra)) {
                                    era_utf=letra;
                    //printf ("activado utf\n");
                            }

                else {
                    //Caracter normal
                    putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
                }
            }


        }

        //if (x>=32) {
        //	printf ("Escribiendo caracter [%c] en x: %d\n",letra,x);
        //}


        if (!era_utf) x++;
    }

}

int menu_ext_desktop_enabled_place_menu(void)
{
    return screen_ext_desktop_place_menu && screen_ext_desktop_enabled*scr_driver_can_ext_desktop();
}

void menu_espera_tecla_o_pending_error_message(void)
{

    //Esperar a pulsar una tecla o hay pendiente mostrar un mensaje de error
    z80_byte acumulado;

    //Si al entrar aqui ya hay tecla pulsada, volver
    acumulado=menu_da_todas_teclas();
    if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) return;


    do {
        menu_cpu_core_loop();

        acumulado=menu_da_todas_teclas();


    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && !if_pending_error_message);

    //Al salir del bucle, reseteamos contadores de repeticion
    menu_reset_counters_tecla_repeticion();

}

void menu_espera_tecla_o_joystick(void)
{

        realjoystick_hit=0;

        //Esperar a pulsar una tecla o joystick
        z80_byte acumulado;

        do {
                menu_cpu_core_loop();


                acumulado=menu_da_todas_teclas();

        //printf ("menu_espera_tecla_o_joystick acumulado: %d\n",acumulado);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && !realjoystick_hit );

}

void menu_dibuja_submenu_free_all(void)
{
    if (menu_show_submenus_tree.v==0) return;

    zxvision_window *w=menu_dibuja_submenu_primer_submenu;


    while (w!=NULL) {
        zxvision_window *next_w=w->submenu_next;
        free(w);
        w=next_w;
    }


    menu_dibuja_submenu_primer_submenu=NULL;

}

int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea)
{
    /*if (mostrar_cursor) {
        if (linea_cursor<alto_ventana-3) return 1;
    }

    else*/ if (primera_linea+alto_ventana-2<indice_linea) return 1;

    return 0;
}

void menu_first_aid_random_startup(void)
{

    //printf ("menu_first_aid_random_startup\n");
    menu_first_aid_startup=0;

    //Si no hay autoguardado de config, no mostrarlo (pues no se podria desactivar)
    if (save_configuration_file_on_exit.v==0) return;

    //Si desactivadas ayudas first-use
    if (menu_disable_first_aid.v) return;

    //Si desactivado multitask
    if (!menu_multitarea) return;

    //si video driver no permite menu normal (no stdout ni simpletext ni null)
    if (!si_normal_menu_video_driver() ) return;

    //Lanzar la primera que este activa y sea de tipo si_startup=1
    int i;
    int encontrado=0;
    for (i=0;i<total_first_aid && !encontrado;i++) {
        int *valor_opcion;
        if (first_aid_list[i].si_startup) {
            valor_opcion=first_aid_list[i].puntero_setting;
            if ((*valor_opcion)==0) {
                string_config_key_aid_startup=first_aid_list[i].config_name;
                encontrado=1;
                menu_set_menu_abierto(1);
                menu_first_aid_must_show_startup=1;
            }
        }
    }

    if (string_config_key_aid_startup!=NULL) debug_printf (VERBOSE_DEBUG,"Set first-use of the day to: %s",string_config_key_aid_startup);

}

int menu_first_aid_title(char *key_setting,char *title) //(enum first_aid_number_list indice)
{

    //Si no hay autoguardado de config, no mostrarlo (pues no se podria desactivar)
    if (save_configuration_file_on_exit.v==0) return 0;

    //Si desactivadas ayudas first-use
    if (menu_disable_first_aid.v) return 0;

    //Si driver no permite menu normal
    if (!si_normal_menu_video_driver()) return 0;

    int indice=menu_first_aid_get_setting(key_setting);
    if (indice<0) return 0;

    int *valor_opcion;
    char *texto_opcion;


    valor_opcion=first_aid_list[indice].puntero_setting;
    texto_opcion=first_aid_list[indice].texto_opcion;

    //Variable a 1. no mostrar nada
    if (*valor_opcion) return 0;


    //Variable a 0. La marcamos por defecto (que significará: no la muestres de nuevo)
    *valor_opcion=1;

    zxvision_menu_generic_message_setting(title,texto_opcion,"Do not show it again",valor_opcion);

    return 1;

}

void putchar_menu_overlay(int x,int y,z80_byte caracter,int tinta,int papel)
{
    putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,0); //sin parpadeo
}

void new_menu_putchar_footer(int x,int y,z80_byte caracter,int tinta,int papel)
{

    putchar_footer_array(x,y,caracter,tinta,papel,0);


}

void putchar_footer_array(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo)
{

    if (!menu_footer) return;

    //int xfinal=((x*menu_char_width)+menu_char_width-1)/8;

    //Controlar limite
    if (x<0 || y<0 || x>=WINDOW_FOOTER_COLUMNS || y>=WINDOW_FOOTER_LINES) {
        //printf ("Out of range. X: %d Y: %d Character: %c\n",x,y,caracter);
        return;
    }

    if (ESTILO_GUI_SOLO_MAYUSCULAS) caracter=letra_mayuscula(caracter);

    int pos_array=y*WINDOW_FOOTER_COLUMNS+x;
    footer_screen_array[pos_array].tinta=tinta;
    footer_screen_array[pos_array].papel=papel;
    footer_screen_array[pos_array].parpadeo=parpadeo;
    footer_screen_array[pos_array].caracter=caracter;


}

char **menu_get_extdesktop_button_bitmap(int numero_boton,int *es_set_machine)
{
    char **puntero_bitmap;

    *es_set_machine=0;

    //por defecto
    puntero_bitmap=zxdesktop_buttons_bitmaps[numero_boton];
    puntero_bitmap=alter_zesarux_ascii_logo(puntero_bitmap);

    int boton_id=numero_boton-1;

    //El 0 no esta permitido
    if (boton_id>=0 && boton_id<MAX_USERDEF_BUTTONS) {

        enum defined_f_function_ids accion;

        int indice_tabla=defined_buttons_functions_array[boton_id];
        accion=menu_da_accion_direct_functions_indice(indice_tabla);


        if (accion!=F_FUNCION_DEFAULT) {
            puntero_bitmap=get_direct_function_icon_bitmap_final(indice_tabla);


            //Si icono es OPEN_WINDOW, adoptar icono de la ventana que se va a abrir
            //printf("Accion open window\n");
            if (accion==F_FUNCION_OPEN_WINDOW) {
                char *geometry_name;
                geometry_name=defined_buttons_functions_array_parameters[boton_id];
                char **possible_bitmap=zxvision_find_icon_for_known_window(geometry_name);
                if (possible_bitmap!=NULL) puntero_bitmap=possible_bitmap;
            }



            //Si icono es F_FUNCION_SET_MACHINE y tiene parametro de set machine, dibujamos el icono de la maquina y luego la "flechita"
            //De tal manera que estamos dibujando un icono sobre el otro. Este es el unico caso de momento que hago eso
            if (accion==F_FUNCION_SET_MACHINE) {


                char *machine_name;
                machine_name=defined_buttons_functions_array_parameters[boton_id];

                if (machine_name[0]) {

                    //Obtener bitmap en base al parametro
                    puntero_bitmap=get_machine_icon_by_name(machine_name);

                    *es_set_machine=1;
                }

            }


        }



    }


    return puntero_bitmap;
}

int menu_get_width_characters_ext_desktop(void)
{
    return get_effective_zxdesktop_width()/menu_char_width/menu_gui_zoom;
}

void menu_ventana_draw_horizontal_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido)
{
        if (porcentaje<0) porcentaje=0;
        if (porcentaje>100) porcentaje=100;

        // mostrar * abajo para indicar donde estamos en porcentaje
        int xbase=x+2;

        int tinta_boton_arriba=ESTILO_GUI_TINTA_NORMAL;
        int tinta_boton_abajo=ESTILO_GUI_TINTA_NORMAL;
        int tinta_barra=ESTILO_GUI_TINTA_NORMAL;

        int papel_boton_arriba=ESTILO_GUI_PAPEL_NORMAL;
        int papel_boton_abajo=ESTILO_GUI_PAPEL_NORMAL;
        int papel_barra=ESTILO_GUI_PAPEL_NORMAL;

        int tinta_aux;

        switch (estilo_invertido) {
            case 1:
                tinta_aux=tinta_boton_arriba;
                tinta_boton_arriba=papel_boton_arriba;
                papel_boton_arriba=tinta_aux;
            break;

            case 2:
                tinta_aux=tinta_boton_abajo;
                tinta_boton_abajo=papel_boton_abajo;
                papel_boton_abajo=tinta_aux;
            break;

            case 3:
                tinta_aux=tinta_barra;
                tinta_barra=papel_barra;
                papel_barra=tinta_aux;
            break;

        }


            //mostrar cursores izquierda y derecha
        menu_ventana_draw_perc_bar_aux(w,xbase-1,y+alto-1,zxvision_retorna_caracter_flecha_izquierda(),tinta_boton_arriba,papel_boton_arriba);
        menu_ventana_draw_perc_bar_aux(w,xbase+ancho-3,y+alto-1,zxvision_retorna_caracter_flecha_derecha(),tinta_boton_abajo,papel_boton_abajo);

        //mostrar linea horizontal para indicar que es zona de porcentaje
        z80_byte caracter_barra='-';
        if (menu_hide_vertical_percentaje_bar.v) caracter_barra=' ';

        int i;
        for (i=0;i<ancho-3;i++) menu_ventana_draw_perc_bar_aux(w,xbase+i,y+alto-1,caracter_barra,tinta_barra,papel_barra);


        int sumarancho=((ancho-4)*porcentaje)/100;

        menu_ventana_draw_perc_bar_aux(w,xbase+sumarancho,y+alto-1,'*',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
}

void menu_ventana_draw_vertical_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido)
{
        if (porcentaje<0) porcentaje=0;
        if (porcentaje>100) porcentaje=100;

        // mostrar * a la derecha para indicar donde estamos en porcentaje
        int ybase=y+2;

        int tinta_boton_arriba=ESTILO_GUI_TINTA_NORMAL;
        int tinta_boton_abajo=ESTILO_GUI_TINTA_NORMAL;
        int tinta_barra=ESTILO_GUI_TINTA_NORMAL;

        int papel_boton_arriba=ESTILO_GUI_PAPEL_NORMAL;
        int papel_boton_abajo=ESTILO_GUI_PAPEL_NORMAL;
        int papel_barra=ESTILO_GUI_PAPEL_NORMAL;

        int tinta_aux;

        switch (estilo_invertido) {
            case 1:
                tinta_aux=tinta_boton_arriba;
                tinta_boton_arriba=papel_boton_arriba;
                papel_boton_arriba=tinta_aux;
            break;

            case 2:
                tinta_aux=tinta_boton_abajo;
                tinta_boton_abajo=papel_boton_abajo;
                papel_boton_abajo=tinta_aux;
            break;

            case 3:
                tinta_aux=tinta_barra;
                tinta_barra=papel_barra;
                papel_barra=tinta_aux;
            break;

        }


        //mostrar cursores arriba y abajo
        //putchar_menu_overlay(x+ancho-1,ybase-1,'^',tinta_boton_arriba,papel_boton_arriba);
        menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase-1,zxvision_retorna_caracter_flecha_arriba(),tinta_boton_arriba,papel_boton_arriba);

        //putchar_menu_overlay(x+ancho-1,ybase+alto-3,'v',tinta_boton_abajo,papel_boton_abajo);
        menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase+alto-3,zxvision_retorna_caracter_flecha_abajo(),tinta_boton_abajo,papel_boton_abajo);

        //mostrar linea vertical para indicar que es zona de porcentaje
        z80_byte caracter_barra='|';
        if (menu_hide_vertical_percentaje_bar.v) caracter_barra=' ';

        //mostrar linea vertical para indicar que es zona de porcentaje
        int i;
        for (i=0;i<alto-3;i++) 	menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase+i,caracter_barra,tinta_barra,papel_barra);


        int sumaralto=((alto-4)*porcentaje)/100;
        menu_ventana_draw_perc_bar_aux(w,x+ancho-1,ybase+sumaralto,'*',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
}

void screen_print_splash_text(int y,int tinta,int papel,char *texto)
{

    //Si no hay driver video
    if (scr_putpixel==NULL || scr_putpixel_zoom==NULL) return;


    if (menu_abierto==0 && screen_show_splash_texts.v==1) {

        //Si background windows y even when menu closed
        //Temporalmente siempre lo hacemos asi
        int mostrar_splash_con_ventana=0;

        if (menu_allow_background_windows && menu_multitarea && always_force_overlay_visible_when_menu_closed) mostrar_splash_con_ventana=1;


        int x;

        #define MAX_LINEAS_SPLASH 24
        const int max_ancho_texto=31;
        //al trocear, si hay un espacio despues, se agrega, y por tanto puede haber linea de 31+1=32 caracteres

        //texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
        //33 es ancho total linea(32)+1
        char buffer_lineas[MAX_LINEAS_SPLASH][33];



        int indice_linea=0;
        int indice_texto=0;
        int ultimo_indice_texto=0;
        int longitud=strlen(texto);

        //int indice_segunda_linea;


        do {
            indice_texto+=max_ancho_texto;

            //Controlar final de texto
            if (indice_texto>=longitud) indice_texto=longitud;

            //Si no, miramos si hay que separar por espacios
            else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

            //Separamos por salto de linea, filtramos caracteres extranyos
            indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

            //copiar texto
            int longitud_texto=indice_texto-ultimo_indice_texto;



            menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
            buffer_lineas[indice_linea++][longitud_texto]=0;
            //printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


            //printf ("texto indice: %d : longitud: %d: -%s-\n",indice_linea-1,longitud_texto,buffer_lineas[indice_linea-1]);
            //printf ("indice_linea: %d indice_linea+y: %d MAX: %d\n",indice_linea,indice_linea+y,MAX_LINEAS_SPLASH);

            if (indice_linea==MAX_LINEAS_SPLASH) {
                    //cpu_panic("Max lines on menu_generic_message reached");
                    debug_printf(VERBOSE_INFO,"Max lines on screen_print_splash_text reached (%d)",MAX_LINEAS_SPLASH);
                    //finalizamos bucle
                    indice_texto=longitud;
            }

            ultimo_indice_texto=indice_texto;
            //printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);

        } while (indice_texto<longitud);

        if (mostrar_splash_con_ventana) {
            screen_print_splash_text_by_window(indice_linea);
        }

        else {

            cls_menu_overlay();

        }

        int i;
        for (i=0;i<indice_linea && y<scr_get_menu_height();i++) {
            int longitud_linea=strlen(buffer_lineas[i]);
            debug_printf (VERBOSE_DEBUG,"line %d y: %d length: %d contents: -%s-",i,y,longitud_linea,buffer_lineas[i]);


            if (mostrar_splash_con_ventana) {
                x=(zxvision_window_splash_text.total_width)/2-strlen(buffer_lineas[i])/2;
                if (x<1) x=1;
                //printf("ZXvision print %d,%d : %s\n",x,i,buffer_lineas[i]);
                zxvision_print_string_defaults_fillspc(&zxvision_window_splash_text,x,i,buffer_lineas[i]);
            }
            else {
                x=menu_center_x()-strlen(buffer_lineas[i])/2;
                if (x<0) x=0;
                menu_escribe_texto(x,y,tinta,papel,buffer_lineas[i]);
            }
            y++;
        }

        if (mostrar_splash_con_ventana) {
            menu_speech_set_tecla_pulsada(); //Si no, envia continuamente todo ese texto a speech

            zxvision_draw_window_contents(&zxvision_window_splash_text);
        }
        else {
            set_menu_overlay_function(normal_overlay_texto_menu);
        }


        menu_splash_text_active.v=1;
        menu_splash_segundos=5;

        //no queremos que reaparezca el logo, por si no había llegado al final de splash. Improbable? Si. Pero mejor ser precavidos
        reset_splash_zesarux_logo();
   }

}

int menu_simple_ten_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,
    char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8,char *opcion9,char *opcion10)
{


    menu_espera_no_tecla();


    menu_item *array_menu_simple_nine_choices;
    menu_item item_seleccionado;
    int retorno_menu;

    //Siempre indicamos la primera opcion
    int simple_nine_choices_opcion_seleccionada=1;
        do {

            menu_add_item_menu_inicial_format(&array_menu_simple_nine_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion3);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion4);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion5);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion6);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion7);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion8);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion9);

            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion10);

            //separador adicional para que quede mas grande la ventana y mas mono
            menu_add_item_menu_format(array_menu_simple_nine_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



            retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&simple_nine_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_nine_choices,texto_ventana);


            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                    //llamamos por valor de funcion
                    return simple_nine_choices_opcion_seleccionada;
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

    return 0;


}

int menu_inicio_return_button_userdef(int boton)
{
    //El 0 no esta permitido
    if (boton<0 || boton>=MAX_USERDEF_BUTTONS) return -1;

    enum defined_f_function_ids accion;

    int indice_tabla=defined_buttons_functions_array[boton];
    accion=menu_da_accion_direct_functions_indice(indice_tabla);

    //printf("menu_inicio_return_button_userdef Boton: %d indice_tabla: %d accion: %d\n",boton,indice_tabla,accion);

    if (accion!=F_FUNCION_DEFAULT) {
        return indice_tabla;
    }

    return -1;

}

int zxvision_add_configurable_icon_by_id_action(enum defined_f_function_ids id_funcion)
{
    //Crear un icono
    int indice_accion=zxvision_get_id_direct_funcion_index(id_funcion);
    return zxvision_add_configurable_icon(indice_accion);
}

void zxvision_draw_below_windows(zxvision_window *w)
{
    //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;

    //printf ("original window: %p\n",w);
        //printf ("\noriginal window: %p. Title: %s\n",w,w->window_title);




    pointer_window=zxvision_find_first_window_below_this(w);

    //printf ("after while pointer_window->previous_window!=NULL\n");

    int antes_ventana_tipo_activa=ventana_tipo_activa;
    zxvision_reset_ventana_tipo_activa(); //Redibujar las de debajo como inactivas

    //Redibujar diciendo que estan por debajo
    ventana_es_background=1;

    //Y ahora de ahi hacia arriba
    //Si puntero es NULL, es porque se ha borrado alguna ventana de debajo. Salir
    //esto puede suceder haciendo esto:
    //entrar a debug cpu-breakpoints. activarlo y dejar que salte el tooltip
    //ir a ZRCP. Meter breakpoint que de error, ejemplo: "sb 1 pc=kkkk("
    //ir a menu. enter y enter. Se provoca esta situacion. Por que? Probablemente porque se ha llamado a destroy window y
    //se ha generado una ventana de error cuando habia un tooltip abierto
    //Ver comentarios en zxvision_destroy_window

    //if (pointer_window==NULL) {
    //	printf ("Pointer was null before loop redrawing below windows\n");
    //}

    //printf ("\nStart loop redrawing below windows\n");

    zxvision_window *always_visible_window=NULL;

    //no mostrar mensajes de error pendientes
    //si eso se hiciera, aparece en medio de la lista de ventanas una que apunta a null y de ahi la condicion pointer_window!=NULL
    //asi entonces dicha condicion pointer_window!=NULL ya no seria necesaria pero la dejamos por si acaso...
    int antes_no_dibuja_ventana_muestra_pending_error_message=no_dibuja_ventana_muestra_pending_error_message;
    no_dibuja_ventana_muestra_pending_error_message=1;

    while (pointer_window!=w && pointer_window!=NULL) {
        //printf ("window from bottom to top %p\n",pointer_window);
        //printf ("window from bottom to top %p. name: %s\n",pointer_window,pointer_window->window_title);

        debug_printf(VERBOSE_DEBUG,"Redrawing window %s",pointer_window->window_title);

        zxvision_draw_window(pointer_window);
        //printf("Decimos tecla pulsada\n");
        //menu_speech_set_tecla_pulsada();
        zxvision_draw_window_contents(pointer_window);
        //printf("Despues de tecla pulsada: menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);

        if (pointer_window->always_visible) always_visible_window=pointer_window;


        pointer_window=pointer_window->next_window;
    }


    //Si hay alguna que tiene always visible, redibujarla encima solo la ventana (el contenido ya se habra mostrado en teoria)
    //Nota: en este caso se dibujaria la visible que esta mas arriba de todas
    if (always_visible_window!=NULL) {
        //printf("Redibujando arriba del todo ventana %s\n",always_visible_window->window_title);
        zxvision_draw_window(always_visible_window);
    }


    no_dibuja_ventana_muestra_pending_error_message=antes_no_dibuja_ventana_muestra_pending_error_message;



    ventana_es_background=0;
    ventana_tipo_activa=antes_ventana_tipo_activa;
}

int zxvision_draw_overlay_if_exists(zxvision_window *w)
{
        void (*overlay_function)(void);
        overlay_function=w->overlay_function;

        //printf ("Funcion overlay: %p. ventana: %s. current window: %p\n",overlay_function,w->window_title,zxvision_current_window);


        //Esto pasa en ventanas que por ejemplo actualizan no a cada frame, al menos refrescar aqui con ultimo valor

        if (overlay_function!=NULL) {
            //printf ("llamando a funcion overlay %p\n",overlay_function);

            overlay_function(); //llamar a funcion overlay
            return 1;
        }
        else {
            //printf ("NO llamando a funcion overlay %p\n",overlay_function);
            return 0;
        }
}

void zxvision_draw_overlays_below_windows(zxvision_window *w)
{
    //printf ("drawing overlay start---------\n");

    //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;


    //if (w!=NULL) printf ("\nDraw below windows with overlay. original window: %p. Title: %s\n",w,w->window_title);


    //Si no hay ventanas, volver
    if (zxvision_current_window==NULL) return;

    pointer_window=w;

    while (pointer_window->previous_window!=NULL) {
            //debug_printf (VERBOSE_PARANOID,"zxvision_draw_overlays_below_windows below window: %p",pointer_window->previous_window);
            pointer_window=pointer_window->previous_window;
    }

    int antes_ventana_tipo_activa=ventana_tipo_activa;
    zxvision_reset_ventana_tipo_activa(); //Redibujar las de debajo como inactivas

    int antes_cuadrado_activo_resize=cuadrado_activo_resize;
    cuadrado_activo_resize=0;

    //Redibujar diciendo que estan por debajo
    ventana_es_background=1;

    //Y ahora de ahi hacia arriba, incluido la ultima


    //printf ("\n");

    //zxvision_drawing_in_background=1;


    zxvision_time_total_drawing_overlay_except_current=0;




    //Dibujar todas ventanas excepto la de mas arriba.
    //while (pointer_window!=w && pointer_window!=NULL) {

    //Dibujar todas ventanas.
    while (pointer_window!=NULL) {
        //while (pointer_window!=w) {
                //printf ("window from bottom to top %p. next: %p nombre: %s\n",pointer_window,pointer_window->next_window,pointer_window->window_title);

        //Somos la ventana de mas arriba
        if (pointer_window==w) {
            ventana_es_background=0;
            ventana_tipo_activa=antes_ventana_tipo_activa;
        };

        //en principio no hace falta. Ya se redibuja por el redibujado normal
        //zxvision_draw_window(pointer_window);

        //Dibujamos contenido anterior, ya que draw_window la borra con espacios
        //en principio no hace falta. Ya se redibuja por el redibujado normal
        //zxvision_draw_window_contents(pointer_window);

        //printf ("drawing overlay %p name: %s\n",pointer_window,pointer_window->window_title);

        struct timeval zxvision_time_antes,zxvision_time_despues;

        //calcular tiempo que tarda en dibujarse

        timer_stats_current_time(&zxvision_time_antes);


        int existe=zxvision_draw_overlay_if_exists(pointer_window);

        //TODO: posible implementacion para parametro frameskip de cada overlay de zxvision_window

        if (existe) {
            //printf("existe\n");
            long transcurrido=timer_stats_diference_time(&zxvision_time_antes,&zxvision_time_despues);
            pointer_window->last_spent_time_overlay=transcurrido;
            //printf ("tiempo transcurrido: %ld microsec\n\n",transcurrido);

            zxvision_time_total_drawing_overlay_except_current +=transcurrido;
        }
        else {
            //printf("no existe\n");
            //Si no tiene overlay se puede decir que el tiempo transcurrido es tal cual, 0
            pointer_window->last_spent_time_overlay=0;
        }


        //dibujar marco ventana
        int x=pointer_window->x;
        int y=pointer_window->y;
        int ancho=pointer_window->visible_width;
        int alto=pointer_window->visible_height;
        int x1,y1,x2,y2;
        zxvision_retorna_coordenadas_marco(x,y,ancho,alto,&x1,&y1,&x2,&y2);
        int color_recuadro=ESTILO_GUI_COLOR_RECUADRO_INACTIVO;
        if (ventana_tipo_activa) color_recuadro=ESTILO_GUI_COLOR_RECUADRO;

        if (zxvision_show_minimized(pointer_window)) menu_dibuja_cuadrado(x1,y1,x2,y2,color_recuadro,color_recuadro,0,!ventana_es_background,pointer_window);


        pointer_window=pointer_window->next_window;
    }

    //if (w!=NULL) printf ("\nEND Draw below windows with overlay. original window: %p. Title: %s\n\n",w,w->window_title);

    //printf ("tiempo TOTAL transcurrido: %ld microsec\n",zxvision_time_total_drawing_overlay);

    //zxvision_drawing_in_background=0;

    ventana_es_background=0;
    ventana_tipo_activa=antes_ventana_tipo_activa;
    cuadrado_activo_resize=antes_cuadrado_activo_resize;

}

int zxvision_coords_in_superior_windows(zxvision_window *w,int x,int y)
{
    //Si esta ventana tiene flag always_visible, siempre escribe
    if (w->always_visible) return 0;

    //if (!menu_allow_background_windows) return 0;

    zxvision_window *orig_w;

    orig_w=w;

    if (w==NULL) return 0;

    //if (zxvision_current_window==w) return 0;

    do {
        zxvision_window *superior_window;

        superior_window=w->next_window;

        if (superior_window!=NULL) {
            //printf("ventana %s encima de la que se redibuja %s\n",superior_window->window_title,w->window_title);
            if (zxvision_coords_in_window(superior_window,x,y)) return 1;

        }


        w=superior_window;

    } while (w!=zxvision_current_window && w!=NULL);

    //return 0;

    //O si hay alguna ventana por debajo que tenga el flag de siempre por encima
    w=orig_w;
    do {
        zxvision_window *inferior_window;

        inferior_window=w->previous_window;

        if (inferior_window!=NULL) {
            if (inferior_window->always_visible) {
                //printf("ventana %s con always visible encima de la que se redibuja %s\n",inferior_window->window_title,orig_w->window_title);
                if (zxvision_coords_in_window(inferior_window,x,y)) return 1;
            }

        }


        w=inferior_window;

    } while (w!=zxvision_current_window && w!=NULL);

    return 0;

}

zxvision_window *zxvision_coords_in_below_windows(zxvision_window *w,int x,int y)
{
    if (!menu_allow_background_windows) return NULL;

    if (w==NULL) return NULL;

    //Empezamos de arriba hacia abajo

    //La primera que encontramos y que contiene las coordenadas indicadas
    zxvision_window *final_lower_window=NULL;

    do {
        zxvision_window *lower_window;

        lower_window=w->previous_window;

        if (lower_window!=NULL) {

            //Si hay ventana por debajo y no hemos escogido una ya
            //O si esta ventana tiene switch de always visible
            //Nota: en caso que haya varias ventanas con always visible, tendra precedencia, en este caso, la de mas abajo
            if (final_lower_window==NULL || lower_window->always_visible) {

                if (zxvision_coords_in_window(lower_window,x,y)) {
                    //printf("Elegida como final ventana: %s\n",lower_window->window_title);
                    final_lower_window=lower_window;
                }

            }

        }


        w=lower_window;

    } while (w!=NULL);

    return final_lower_window;

    //return NULL;

}

z80_byte zxvision_common_getkey_refresh_noesperatecla(void)
//Igual que zxvision_common_getkey_refresh pero sin esperar tecla cuando multitarea activa
{

    z80_byte tecla;

                menu_cpu_core_loop();

                //si no hay multitarea, refrescar pantalla para mostrar contenido ventana rellenada antes, esperar tecla,
                if (menu_multitarea==0) {
                        menu_refresca_pantalla();
                        menu_espera_tecla();
                        //acumulado=0;
                }

                tecla=zxvision_read_keyboard();

                //Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder
                //refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
                //(a no ser que este multitarea off)

                if (tecla==13 && mouse_left) {
                    tecla=0;
                }


        if (tecla) {
            //printf ("Esperamos no tecla\n");
            menu_espera_no_tecla_con_repeticion();
        }

    return tecla;
}

int zxvision_change_gui_style_select_by_name(char *estilo)
{

    int id_estilo=menu_get_gui_index_by_name(estilo);
    if (id_estilo<0) return 1;

    zxvision_change_gui_style_select_id(id_estilo);
    return 0;

}

void zxvision_draw_filled_triangle(zxvision_window *w,
    int color_relleno,int color_aristas,int x1,int y1,int x2,int y2,int x3,int y3,void (*fun_putpixel) (zxvision_window *w,int x,int y,int color),int arista_no_dibujar)
{

    /*
    Almacenar en una buffer en memoria los pixeles usados por las aristas, sin dibujarlos inicialmente
    Para el relleno, empezar desde abajo hacia arriba, dibujando lineas horizontales, viendo valor minimo x y maximo x para la posición y correspondiente
    Y finalmente si que dibujamos las aristas
    */

    //Buscamos minimo x
    int min_x=x1;
    if (x2<min_x) min_x=x2;
    if (x3<min_x) min_x=x3;

    //Buscamos minimo y
    int min_y=y1;
    if (y2<min_y) min_y=y2;
    if (y3<min_y) min_y=y3;

    //Y maximos x,y
    int max_x=x1;
    if (x2>max_x) max_x=x2;
    if (x3>max_x) max_x=x3;

    int max_y=y1;
    if (y2>max_y) max_y=y2;
    if (y3>max_y) max_y=y3;


    //Creamos el buffer de pixeles usados, como un cuadrado desde los minimos hasta los maximos
    int ancho=max_x-min_x+1;
    int alto=max_y-min_y+1;

    int *buffer_pixeles_aristas=util_malloc_fill(ancho*alto*sizeof(int),"Can not allocate memory for pixel buffer",0);

    //Dibujamos los 3 vertices del triangulo
    //de x1,y1 a x2,y2
    //de x1,y1 a x3,y3
    //de x2,y2 a x3,y3

    //Meter en buffer
    zxvision_draw_line_for_filled_triangle(x1,y1,x2,y2,min_x,min_y,ancho,buffer_pixeles_aristas,zxvision_draw_filled_triangle_putpixel_buffer);
    zxvision_draw_line_for_filled_triangle(x1,y1,x3,y3,min_x,min_y,ancho,buffer_pixeles_aristas,zxvision_draw_filled_triangle_putpixel_buffer);
    zxvision_draw_line_for_filled_triangle(x2,y2,x3,y3,min_x,min_y,ancho,buffer_pixeles_aristas,zxvision_draw_filled_triangle_putpixel_buffer);


    //Ahora vamos a hacer render desde posicion Y menor (en coordenadas de zxvision el 0,0 esta arriba del todo)
    int y;
    for (y=min_y;y<=max_y;y++) {
        //Buscamos posicion x menor, eso dice el area del triangulo donde empieza por la izquierda
        int x;
        int izquierda=-1;
        for (x=min_x;x<=max_x;x++) {
            int offset_buffer=(y-min_y)*ancho+(x-min_x);
            if (buffer_pixeles_aristas[offset_buffer]) {
                izquierda=x;
                break;
            }
        }

        //Buscamos posicion y mayor, eso dice el area del triangulo donde empieza por la derecha
        int derecha=-1;
        for (x=max_x;x>=min_x;x--) {
            int offset_buffer=(y-min_y)*ancho+(x-min_x);
            if (buffer_pixeles_aristas[offset_buffer]) {
                derecha=x;
                break;
            }
        }

        if (izquierda!=-1 && derecha!=-1) {
            //Rellenar
            zxvision_draw_line(w,izquierda,y,derecha,y,color_relleno,fun_putpixel);
        }
    }

    //Finalmente dibujamos las aristas
    if (arista_no_dibujar!=1) zxvision_draw_line(w,x1,y1,x2,y2,color_aristas,fun_putpixel);
    if (arista_no_dibujar!=2) zxvision_draw_line(w,x1,y1,x3,y3,color_aristas,fun_putpixel);
    if (arista_no_dibujar!=3) zxvision_draw_line(w,x2,y2,x3,y3,color_aristas,fun_putpixel);

    free(buffer_pixeles_aristas);

}

void zxvision_draw_filled_triangle_habitual(zxvision_window *w,int color_relleno,int color_aristas,int x1,int y1,int x2,int y2,int x3,int y3)
{
    zxvision_draw_filled_triangle(w,color_relleno,color_aristas,x1,y1,x2,y2,x3,y3,zxvision_draw_filled_triangle_putpixel,-1);
}

void zxvision_if_configurable_icon_not_on_valid_position_set(int icon)
{

    if (zxdesktop_configurable_icons_list[icon].status==ZXDESKTOP_CUSTOM_ICON_EXISTS) {
        int x=zxdesktop_configurable_icons_list[icon].pos_x;
        int y=zxdesktop_configurable_icons_list[icon].pos_y;
        if (!zxvision_if_configurable_icon_on_valid_position(x,y)) {
            debug_printf(VERBOSE_DEBUG,"Relocate icon %d (%s) because it is on an invalid position %d,%d",icon,zxdesktop_configurable_icons_list[icon].text_icon,x,y);
            zxvision_get_next_free_icon_position(&x,&y);
            debug_printf(VERBOSE_DEBUG,"Relocate icon %d to %d,%d",icon,x,y);
            zxvision_set_configurable_icon_position(icon,x,y);
        }
    }

}

int zxvision_if_vertical_scroll_bar(zxvision_window *w)
{
    if (!w->can_be_scrolled) return 0;

    //Si esta minimizada, no hay scroll
    if (w->is_minimized) return 0;

    if (w->can_use_all_width==1) {
        //w->applied_can_use_all_width=1;
        return 0;
    }
    int effective_height=zxvision_get_effective_height(w);
    if (w->total_height>effective_height && w->visible_height>=6) return 1;

    return 0;
}

void zxvision_draw_window_contents_no_speech(zxvision_window *ventana)
{
                //No queremos que el speech vuelva a leer la ventana, solo cargar ventana
        int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
                menu_speech_set_tecla_pulsada();
                zxvision_draw_window_contents(ventana);

        /*if (menu_speech_tecla_pulsada && !antes_menu_speech_tecla_pulsada) {
            printf("Pasar de 1 a 0 desde \n");
            debug_exec_show_backtrace();
        }*/

        menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;

}

void zxvision_handle_maximize_maximize(zxvision_window *w)
{
    debug_printf (VERBOSE_DEBUG,"Maximize window");

    //printf ("visible width %d\n",max_width);
    //Almacenar ultima posicion en caso que se haya movido
    w->x_before_max_min_imize=w->x;
    w->y_before_max_min_imize=w->y;

    int max_width;
    int max_height;
    int xinicial;
    int yinicial;

    //Tratar esto diferente si hay zx desktop activado y si la apertura de ventanas es en zx desktop
    //En ese caso obtiene el maximo que cabe en zxdesktop
    if (if_zxdesktop_enabled_and_driver_allows() && screen_ext_desktop_place_menu) {
        //printf("zx desktop enabled\n");
        xinicial=menu_origin_x();
        yinicial=0;

        max_width=menu_get_width_characters_ext_desktop();
        max_height=scr_get_menu_height();



        //Si hay botones parte superior zxdesktop, origen_y lo incrementamos
        if (menu_zxdesktop_upper_buttons_enabled.v && zxvision_topbar_menu_enabled.v==0) {
            yinicial=EXT_DESKTOP_BUTTONS_TOTAL_SIZE/8;
            max_height-=(EXT_DESKTOP_BUTTONS_TOTAL_SIZE/8);
        }


        if (menu_zxdesktop_lower_buttons_enabled.v) {
            //Y quitamos ese alto disponible para no sobreescribir botones inferiores
            max_height-=(EXT_DESKTOP_BUTTONS_TOTAL_SIZE/8);
        }


    }

    //En este caso obtiene el maximo total en pantalla
    else {
        //printf("zx desktop disabled\n");
        xinicial=0;
        yinicial=0;
        max_width=scr_get_menu_width();
        max_height=scr_get_menu_height();
    }


    //si hay top bar, aplica independientemente si se tiene zx desktop o no
    if (zxvision_topbar_menu_enabled.v) {
        yinicial=1;
        //Le quitamos la linea superior
        max_height--;
    }

    //Cuando restauramos ventanas maximizadas al inicio, podemos tener un tamaño de ventana mas grande de lo que cabe
    //porque se haya reducido el zx desktop. Por tanto, antes de moverlas a su posicion final (evitando que no nos deje moverla porque no quepa),
    //movemos a 0,0, cambiamos tamaño,
    //y luego ya movemos a posicion final

    //Lo hago asi para que no implique redraw
    w->x=0;
    w->y=0;

    if (zxvision_topbar_menu_enabled.v) {
        w->y=1;
    }

    zxvision_set_visible_width(w,max_width);
    zxvision_set_visible_height(w,max_height);

    //printf("x inicial: %d\n",xinicial);
    zxvision_set_x_position(w,xinicial);
    zxvision_set_y_position(w,yinicial);


    w->is_maximized=1;

    //printf("x final: %d\n",w->x);

}

void zxvision_espera_tecla_timeout_window_splash(int tipo)
{

    z80_byte tecla;
    //printf ("espera splash\n");
    do {

        //Esperar a pulsar una tecla o timeout de window splash
        //z80_byte acumulado;

        int contador_antes=menu_window_splash_counter_ms;
        int trozos=4;
        //WINDOW_SPLASH_SECONDS.
        //5 pasos. total de WINDOW_SPLASH_SECONDS
        int tiempototal=1000*WINDOW_SPLASH_SECONDS;
        //Quitamos 1 segundo
        tiempototal-=1000;

        //Intervalo de cambio
        int intervalo=tiempototal/5; //5 pasos
        //printf ("intervalo: %d\n",intervalo);

        int indice_apagado=0;




    do {
                menu_cpu_core_loop();


                //acumulado=menu_da_todas_teclas();
                tecla=zxvision_read_keyboard();

                //con boton izquierdo no salimos
                if (tecla==13 && mouse_left) {
                    tecla=0;
                }

                //Cada 400 ms
                if (menu_window_splash_counter_ms-contador_antes>intervalo) {
                    //trozos--;
                    contador_antes=menu_window_splash_counter_ms;
                    //printf ("dibujar franjas trozos: %d\n",trozos);
                    if (trozos>=0) {
                        if (tipo==1) {
                            menu_dibuja_ventana_franja_arcoiris_trozo_current(trozos);
                            //printf("trozos: %d\n",trozos);
                        }
                    }

                    trozos--;

                    if (tipo==2) menu_dibuja_ventana_franja_arcoiris_oscuro_current(indice_apagado);
                    indice_apagado++;
                }


        //printf ("menu_espera_tecla_timeout_tooltip acumulado: %d\n",acumulado);
        //printf ("contador splash: %d\n",menu_window_splash_counter);


    } while (tecla==0 && menu_window_splash_counter<WINDOW_SPLASH_SECONDS);

    menu_window_splash_counter=0;
    menu_window_splash_counter_ms=0;

    } while (tipo==2 && tecla==0);

}

void zxvision_espera_tecla_condicion_progreso(zxvision_window *w,int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) )
{

    z80_byte tecla;
    int condicion=0;
    int contador_antes=menu_window_splash_counter_ms;
    int intervalo=20*ZXVISION_SIMPLE_PROGRESS_WINDOW_FRAMES_REFRESH; //En milisegundos

    //contador en us
    int contador_no_multitask=0;


    //printf ("espera splash\n");
    do {

                menu_cpu_core_loop();
                int pasado_cuarto_segundo=0;

                //TODO: se puede dar el caso que se llame aqui pero el thread aun no se haya creado, lo que provoca
                //que dice que el thread no esta en ejecucion aun y por tanto cree que esta finalizado, diciendo que la condicion de salida es verdadera
                //y salga cuando aun no ha finalizado
                //Seria raro, porque el intervalo de comprobacion es cada 1/4 de segundo, y en ese tiempo se tiene que haber lanzado el thread de sobra

                //En caso que no haya multitarea, activar las funciones de print y condicion de otra manera
                 if (!menu_multitarea) {
                    contador_no_multitask+=MENU_CPU_CORE_LOOP_SLEEP_NO_MULTITASK;


                    if (contador_no_multitask>=intervalo*1000) {
                        //printf ("Pasado medio segundo %d\n",contador_segundo);
                        contador_no_multitask=0;
                        pasado_cuarto_segundo=1;

                        //printf ("refresca pantalla\n");
                        menu_refresca_pantalla();

                    }
                }

                //acumulado=menu_da_todas_teclas();
                tecla=zxvision_read_keyboard();

                //con boton izquierdo no salimos
                if (tecla==13 && mouse_left) {
                    tecla=0;
                }

                if (menu_window_splash_counter_ms-contador_antes>intervalo) pasado_cuarto_segundo=1;

                //Cada 224 ms
                if (pasado_cuarto_segundo) {
                    //trozos--;
                    contador_antes=menu_window_splash_counter_ms;
                    //printf ("dibujar franjas trozos: %d\n",trozos);
                    //llamar a la condicion
                    if (funcioncond!=NULL) condicion=funcioncond(w);

                    //llamar a funcion print
                    if (funcionprint!=NULL) funcionprint(w);

                }


    } while (tecla==0 && !condicion);


}

index_menu *zxvision_index_search_menu(char *nombre)
{

    //Convertir a string sin acentos etc
    char nombre_sin_acentos[MAX_LENGTH_FULL_PATH_SUBMENU];

    util_convert_utf_no_utf(nombre,nombre_sin_acentos,strlen(nombre));


    index_menu *menu=first_index_menu;

    while (menu!=NULL) {
        //printf("Comparar [%s] [%s]\n",nombre_sin_acentos,menu->titulo_menu);
        if (!strcasecmp(nombre_sin_acentos,menu->titulo_menu)) return menu;

        menu=menu->next_menu;
    }

    return NULL;
}

void zxvision_index_add_menu_linea(index_menu *indice_menu,char *nombre_linea_orig)
{

    //Si linea en blanco, no hacer nada
    if (nombre_linea_orig[0]==0) return;

    if (nombre_linea_orig[0]==' ' && nombre_linea_orig[1]==0) return;

    //Ver si excede maximo linea
    if (strlen(nombre_linea_orig)>MAX_TEXTO_OPCION-1) {
        debug_printf(VERBOSE_DEBUG,"Adding menu entry to index exceeds limit: [%s]. Do not add it",nombre_linea_orig);
        return;
    }

    //Convertir a string sin acentos etc
    char nombre_linea[MAX_TEXTO_OPCION];

    util_convert_utf_no_utf(nombre_linea_orig,nombre_linea,strlen(nombre_linea_orig));

    //printf("%02X %02X %02X\n",nombre_linea[0],nombre_linea[1],nombre_linea[2]);

    //if (nombre_linea[0]=='\n' && nombre_linea[1]==0) return;

    //printf("Agregando linea %s a menu %s\n",nombre_linea,indice_menu->titulo_menu);


    //Ir hasta la ultima linea
    index_menu_linea *antes=indice_menu->first_item_menu;

    if (antes!=NULL) {

        while (antes->next_item_menu!=NULL) {
            antes=antes->next_item_menu;
        }
    }

    index_menu_linea *nueva_linea=util_malloc(sizeof(index_menu_linea),"Can not allocate memory for index menu line");

    //Quitar de la linea caracteres hotkey ~~
    char texto_opcion[MAX_TEXTO_OPCION];
    int origen=0;
    int destino=0;

    for (;nombre_linea[origen];origen++,destino++) {
        //Saltar dos ~~ seguidos
        if (nombre_linea[origen]=='~' && nombre_linea[origen+1]=='~') origen+=2;

        texto_opcion[destino]=nombre_linea[origen];
    }

    texto_opcion[destino]=0;

    //Asignar texto linea
    strcpy(nueva_linea->texto_opcion,texto_opcion);

    //printf("Agregando linea despues procesado %s a menu %s\n",texto_opcion,indice_menu->titulo_menu);

    //Siguiente linea a null
    nueva_linea->next_item_menu=NULL;

    //Y decirle a la linea anterior que el siguiente es este (o si no hay, este es el primero)
    if (antes==NULL) indice_menu->first_item_menu=nueva_linea;
    else antes->next_item_menu=nueva_linea;

}

index_menu *zxvision_index_add_replace_menu(char *titulo_menu)
{

    //Si menu en blanco, no hacer nada. Esto no deberia suceder, pero por si acaso
    //if (titulo_menu[0]==0) return;

    //ver si ya existe
    index_menu *menu=zxvision_index_search_menu(titulo_menu);

    if (menu==NULL) {
        //printf("Menu %s no existe. Crear\n",titulo_menu);
        return zxvision_index_add_menu(titulo_menu);
    }
    else {
        //reemplaza, borrar items de menu
        zxvision_index_erase_all_menu_lines(menu);
        //printf("Menu %s ya existe. Reemplazar\n",titulo_menu);
        return menu;
    }
}

index_menu *zxvision_index_entrada_menu(char *titulo)
{

    //printf("Menu [%s]\n",nombre_menu_con_submenu_para_indice);
    index_menu *indice_menu_actual=zxvision_index_add_replace_menu(titulo);

    return indice_menu_actual;
}

unsigned char zxvision_retorna_caracter_flecha_izquierda(void)
{
    //Si driver video permite caracteres redefinidos, poner los cursores "bonitos"
    if (si_complete_video_driver()) {
        return CHAR_ARROW_LEFT;
    }
    else return '<';
}

unsigned char zxvision_retorna_caracter_flecha_derecha(void)
{
    //Si driver video permite caracteres redefinidos, poner los cursores "bonitos"
    if (si_complete_video_driver()) {
        return CHAR_ARROW_RIGHT;
    }
    else return '>';
}

void zxvision_reapply_style_colours_all_windows(void)
{

    if (zxvision_current_window==NULL) return;

    //Primero ir a buscar la de abajo del todo
    zxvision_window *pointer_window;


    pointer_window=zxvision_find_first_window_below_this(zxvision_current_window);


    while (pointer_window!=zxvision_current_window && pointer_window!=NULL) {
        zxvision_reapply_style_colours_one_window(pointer_window);

        //El resto de colores se refrescaran por si solos cuando el overlay de cada ventana diga que hay que refrescar

        pointer_window=pointer_window->next_window;
    }

    //Y a la ventana actual
    if (pointer_window!=NULL && pointer_window==zxvision_current_window) {
        zxvision_reapply_style_colours_one_window(pointer_window);
    }


}

void zxvision_set_not_minimizable(zxvision_window *w)
{
    //Decimos que no se puede minimizar
    //printf ("set not resizable\n");
    ventana_activa_puede_minimizar=0;

    w->can_be_minimized=0;
}

void zxvision_set_not_resizable(zxvision_window *w)
{
    //Decimos que no se puede redimensionar
    //printf ("set not resizable\n");
    cuadrado_activo_resize=0;

    w->can_be_resized=0;
}

void zxvision_set_offset_y_visible(zxvision_window *w,int y)
{

    int linea_final;

    //El cursor esta por arriba. Decimos que este lo mas arriba posible
    if (y<w->offset_y) {
        linea_final=y;
        //printf ("adjust verticall scroll por arriba to %d\n",linea_final);

    }

    //El cursor esta por abajo. decimos que el cursor este lo mas abajo posible
    else if (y>=w->offset_y+w->visible_height-2) {
        linea_final=y-(w->visible_height-2)+1;
        //Ejemplo
        //total height 12
        //visble 10->efectivos son 8
        //establecemos a linea 7
        //linea_final=7-(10-2)+1 = 7-8+1=0

        //printf ("adjust verticall scroll por abajo to %d\n",linea_final);
    }

    else return;

    int ultima_linea_scroll=w->total_height-(w->visible_height-2);

    /*
    Ejemplo: visible_height 10-> efectivos son 8
    total_height 12
    podremos hacer 4 veces scroll
    12-(10-2)=12-8=4
    */

    if (ultima_linea_scroll<0) ultima_linea_scroll=0;
    if (linea_final>ultima_linea_scroll) linea_final=ultima_linea_scroll;

    //printf ("final scroll %d\n",linea_final);

    zxvision_set_offset_y(w,linea_final);


}

void zxvision_send_scroll_left(zxvision_window *w)
{
    if (w->offset_x>0) {
        zxvision_set_offset_x(w,w->offset_x-1);
    }
}

void zxvision_send_scroll_right(zxvision_window *w)
{
    if (w->offset_x<(w->total_width-1)) {
        zxvision_set_offset_x(w,w->offset_x+1);
    }
}

void zxvision_minimize_window(zxvision_window *w)
{
    if (w!=NULL) {

        //Primero decimos que no esta minimizada
        w->is_minimized=0;

        zxvision_toggle_minimize_window(w);
    }
}

void zxvision_maximize_window(zxvision_window *w)
{
    if (w!=NULL) {

        //Primero decimos que no esta maximizada
        w->is_maximized=0;

        zxvision_toggle_maximize_window(w);
    }
}

int zxvision_window_can_be_backgrounded(zxvision_window *w)
{
    if (w==NULL) return 0;

    if (menu_allow_background_windows && w->can_be_backgrounded) return 1;
    else return 0;
}

void zxvision_set_resizable(zxvision_window *w)
{
    //Decimos que se puede redimensionar
    //printf ("set resizable\n");
    cuadrado_activo_resize=1;

    w->can_be_resized=1;
}

void zxvision_set_window_overlay_from_current(zxvision_window *ventana)
{

    /*
    realmente comparar con la de normal overlay nos sirve para evitar que ventanas que no tienen overlay
    pero que pueden ir a background (como debug cpu) les ponga como overlay el propio de normal overlay
    Es un poco chapucero pero funciona
    TODO: seria mejor indicar con un flag (por defecto a 0) que la ventana tiene un overlay activo diferente de normal_overlay
    */
    if (menu_overlay_function!=normal_overlay_texto_menu) {
        ventana->overlay_function=menu_overlay_function;
    }
}

int zxvision_which_upper_button_is_mouse(void)
{
    int mouse_pixel_x,mouse_pixel_y;
    menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

    //multiplicamos por zoom
    mouse_pixel_x *=zoom_x;
    mouse_pixel_y *=zoom_y;

    //Si esta en zona botones de zx desktop. Y si estan habilitados

    if (menu_zxdesktop_upper_buttons_enabled.v && zxvision_topbar_menu_enabled.v==0) {
        int ancho_boton,alto_boton,total_botones,xinicio_botones,xfinal_botones;
        menu_ext_desktop_buttons_get_geometry(&ancho_boton,&alto_boton,&total_botones,&xinicio_botones,&xfinal_botones);

        if (mouse_pixel_x>=xinicio_botones && mouse_pixel_x<xfinal_botones &&
            mouse_pixel_y>=0 && mouse_pixel_y<alto_boton
        ) {
            //printf ("Pulsado en zona botones del ext desktop\n");

            //en que boton?
            int numero_boton=(mouse_pixel_x-xinicio_botones)/ancho_boton;

            return numero_boton;
        }
    }

    return -1;
}

int zxvision_which_lower_button_is_mouse(void)
{

    int mouse_pixel_x,mouse_pixel_y;
    menu_calculate_mouse_xy_absolute_interface_pixel(&mouse_pixel_x,&mouse_pixel_y);

    //multiplicamos por zoom
    mouse_pixel_x *=zoom_x;
    mouse_pixel_y *=zoom_y;

    if (menu_zxdesktop_lower_buttons_enabled.v) {
        int ancho_boton,alto_boton,xinicio_botones,xfinal_botones,yinicio_botones;
        menu_ext_desktop_lower_buttons_get_geometry(&ancho_boton,&alto_boton,NULL,&xinicio_botones,&xfinal_botones,&yinicio_botones);

        if (mouse_pixel_x>=xinicio_botones && mouse_pixel_x<xfinal_botones &&
            mouse_pixel_y>=yinicio_botones && mouse_pixel_y<yinicio_botones+alto_boton
        ) {
            //printf ("Pulsado en zona lower icons del ext desktop\n");

            //en que boton?
            int numero_boton=(mouse_pixel_x-xinicio_botones)/ancho_boton;
            //printf("boton pulsado: %d\n",numero_boton);

            //Buscar indice array
            int indice_array=zxdesktop_lowericon_find_index(numero_boton);

            if (indice_array>=0) {

                return numero_boton;
            }

            else {
                //printf ("boton NO esta visible\n");
            }


        }
    }

    return -1;
}

void zxvision_sound_event_error_menu(void)
{
    //printf("sonido error\n");

    //Este sonido de error tiene prioridad. Si hay alguno sonando, lo cancelamos para que suene este
    audio_menu_tone_generator_stop();

    zxvision_sound_event_aux("C2",50);
}

void zxvision_sound_event_close_window(void)
{
    //printf("sound close\n");
    zxvision_sound_event_aux("C7",5);
}

void zxvision_sound_event_new_window(void)
{
    zxvision_sound_event_aux("C4",5);
}

int zxvision_get_id_direct_funcion_index(enum defined_f_function_ids id_funcion)
{

    int i;

    for (i=0;i<MAX_F_FUNCTIONS;i++) {
        if (defined_direct_functions_array[i].id_funcion==id_funcion) {
            return i;
        }
    }

    return -1;
}

int zxvision_if_configurable_icon_on_valid_position(int x,int y)
{
    //Ver si posicion no se sale del rango de ventana total (sin footer)
    int total_width=screen_get_total_width_window_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;
    int total_height=screen_get_total_height_window_no_footer_plus_zxdesktop_no_zoom()-ZESARUX_ASCII_LOGO_ANCHO;

    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo

    if (x<0 || y<0 || x>total_width || y>total_height) {
        //printf("Check icon position: %d,%d out of range (%d,%d)\n",x,y,total_width,total_height);
        return 0;
    }

    int xinicio_botones,xfinal_botones,yinicio_botones; //,alto_boton;

    //Ver si en posicion de iconos superiores
    menu_ext_desktop_buttons_get_geometry(NULL,NULL,NULL,&xinicio_botones,&xfinal_botones);
    //Posiciones menos el zoom
    xinicio_botones /=zoom_x;
    xfinal_botones /=zoom_x;
    //alto_boton /=zoom_y;

    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    xinicio_botones -=ZESARUX_ASCII_LOGO_ANCHO;

    int minima_y=zxvision_get_minimum_y_icon_position();

    if (y<minima_y && x>=xinicio_botones && x<xfinal_botones) {
        //printf("Check icon position: %d,%d on upper buttons position (y<%d)\n",x,y,alto_boton);
        return 0;
    }

    //Ver si en posicion de iconos inferiores

    menu_ext_desktop_lower_buttons_get_geometry(NULL,NULL,NULL,&xinicio_botones,&xfinal_botones,&yinicio_botones);
    //Posiciones menos el zoom
    xinicio_botones /=zoom_x;
    xfinal_botones /=zoom_x;
    yinicio_botones /=zoom_y;

    //Consideramos el tamanyo del icono (ZESARUX_ASCII_LOGO_ANCHO) para que no se pueda ubicar medio icono fuera de rango por ejemplo
    xinicio_botones -=ZESARUX_ASCII_LOGO_ANCHO;
    yinicio_botones -=ZESARUX_ASCII_LOGO_ANCHO;

    if (y>=yinicio_botones && x>=xinicio_botones && x<=xfinal_botones) {
        //printf("Check icon position: %d,%d on lower device icons position\n",x,y);
        return 0;
    }


    //Ver si icono en posicion de pantalla emulada
    int ancho_maquina=screen_get_emulated_display_width_no_zoom_border_en();
    int alto_maquina=screen_get_emulated_display_height_no_zoom_border_en();

    if (x<ancho_maquina && y<alto_maquina) {
        //printf("Check icon position: %d,%d on emulated machine display\n",x,y);
        return 0;
    }

    return 1;

}

int zxdesktop_lowericon_find_index(int icono)
{
    //int total_botones;
    int total_botones=TOTAL_ZXDESKTOP_MAX_LOWER_BUTTONS;

    if (icono>=total_botones || icono<0) return -1;

    int i;

    int i_enabled=0;

    for (i=0;i<total_botones;i++) {
        int (*funcion_is_visible)(void);
        funcion_is_visible=zdesktop_lowericons_array[i].is_visible;

        int visible=funcion_is_visible();

//Para poder forzar visibilidad de iconos para debug
#ifdef FORCE_VISIBLE_ALL_LOWER_BUTTONS
    visible=1;
#endif

        if (visible) {
            if (i_enabled==icono) {
                //printf("buscando %d encontrado indice %d\n",icono,i);
                return i;
            }

            i_enabled++;
        }
    }

    return -1;

}

char **menu_ext_desktop_draw_configurable_icon_return_machine_icon(void)
{
    char buffer_name[255];

    get_machine_config_name_by_number(buffer_name,current_machine_type);

    return get_machine_icon_by_name(buffer_name);

}

int menu_escribe_texto_si_inverso(char *texto, int indice)
{

    if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='~') return 0;
    if (texto[indice]!='~' && texto[indice]!='^' && texto[indice]!='!') {
        return 0;
    }

    indice++;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;
}

int menu_escribe_texto_si_parpadeo(char *texto, int indice)
{

    if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='^') return 0;
    if (texto[indice++]!='^') return 0;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;
}

int menu_escribe_texto_si_cambio_tinta(char *texto,int indice)
{
    if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='$') return 0;
    if (texto[indice++]!='$') return 0;
    if (texto[indice]<'0' || texto[indice]>'7'+8) return 0; //Soportar colores con brillo
    indice++;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;

}

void menu_dibuja_cuadrado(int x1,int y1,int x2,int y2,int color,int color_marca_redimensionado,int ventana_en_primer_plano,int alterar_estilo_cuadrado,zxvision_window *w)
{

    if (!ESTILO_GUI_MUESTRA_RECUADRO) return;

    //funcion de putpixel. dependiendo si es ventana en primer plano o no
    void (*putpixel_function)(int x,int y,int color,int zoom_level);

    if (ventana_en_primer_plano) {
        putpixel_function=scr_putpixel_gui_zoom;
        menu_dibuja_cuadrado_putpixel_background_ventana=NULL; //aunque no se usara en este caso, pero por si acaso
    }
    else {
        putpixel_function=menu_dibuja_cuadrado_putpixel_background;
        menu_dibuja_cuadrado_putpixel_background_ventana=w;
        menu_dibuja_cuadrado_putpixel_background_reset_cache();
    }


    int x,y;

    //Si ratón está encima de la zona de redimensionado
    if (ventana_marca_redimensionado_raton_encima && menu_change_frame_when_resize_zone.v && alterar_estilo_cuadrado) {
        color=ESTILO_GUI_PAPEL_SELECCIONADO;


        //Poner otro color si llega al minimo y maximo necesario por la ventana
        //Aunque tried_write_beyond_size se pondra a 0 en cuanto
        zxvision_window *w=zxvision_current_window;
        if (w!=NULL) {
            //printf("Beyond: %d,%d current %d,%d\n",
            //        w->current_window_char_written_beyond_size_width,w->current_window_char_written_beyond_size_height,w->total_width,w->total_height);

                if (w->current_window_char_written_beyond_size_width>0 || w->current_window_char_written_beyond_size_height>0) {

                    //printf("actual: %d,%d minimo: %d,%d\n",w->total_width,w->total_height,w->current_window_char_written_beyond_size_width,w->current_window_char_written_beyond_size_height);
                    if (w->total_width-1<w->current_window_char_written_beyond_size_width || w->total_height-1<w->current_window_char_written_beyond_size_height) {
                        color=ESTILO_GUI_COLOR_AVISO;
                    }
                }

        }




        color_marca_redimensionado=color;
    }

    //Para poner una marca en la ventana indicando si es de tipo zxvision
    //int centro_marca_zxvison_x=x2-3-6;
    //int centro_marca_zxvison_y=y1+3+2;

    //int longitud_marca_zxvision=3;
    //int mitad_long_marca_zxvision=longitud_marca_zxvision/2;
    //int color_marca_zxvision=ESTILO_GUI_PAPEL_NORMAL;


    //printf ("Cuadrado %d,%d - %d,%d\n",x1,y1,x2,y2);


    //solo hacerlo en el caso de drivers completos
    if (si_complete_video_driver() ) {

        //Si estaba en titulo y moviendo la ventana
        if (mouse_is_dragging && alterar_estilo_cuadrado && window_is_being_moved) {
            zxvision_window *w=zxvision_current_window;
            if (w!=NULL) {
                char titulo[ZXVISION_MAX_WINDOW_TITLE];

                sprintf(titulo,"%d,%d",w->x,w->y);

                menu_dibuja_ventana_titulo(w,titulo);
            }

        }

        //TODO: se dibuja zona punteada cuando mouse_is_dragging, esto es un error, porque cualquier acción de arrastrar algo en la ventana,
        //ocasiona que se dibuje punteado, no solo al arrastrar la ventana propiamente
        //Solo que no se nota porque el dibujado punteado consiste en no dibujar 1 de cada 2 pixeles, y como ya están dibujados, su efecto no se nota
        //Pero tendria que hacerse mirando algún otro tipo de condición, por ejemplo se podria mirar ventana_marca_redimensionado_raton_encima

        //printf("dibuja_cuadrado %d %d\n",mouse_is_dragging,ventana_marca_redimensionado_raton_encima);
        if (mouse_is_dragging && ventana_marca_redimensionado_raton_encima && alterar_estilo_cuadrado) {
            zxvision_window *w=zxvision_current_window;
            if (w!=NULL) {
                char titulo[ZXVISION_MAX_WINDOW_TITLE];

                sprintf(titulo,"%d x %d",w->visible_width,w->visible_height);

                menu_dibuja_ventana_titulo(w,titulo);
            }

        }

        //parte inferior
        for (x=x1;x<=x2;x++) {
            if (mouse_is_dragging && alterar_estilo_cuadrado && (x%2)==0) continue; //punteado cuando se mueve o redimensiona
            putpixel_function(x*menu_gui_zoom,y2*menu_gui_zoom,color,menu_gui_zoom);
        }


        //izquierda
        for (y=y1;y<=y2;y++) {
            if (mouse_is_dragging && alterar_estilo_cuadrado && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
            putpixel_function(x1*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
        }



        //derecha
        for (y=y1;y<=y2;y++) {
            if (mouse_is_dragging && alterar_estilo_cuadrado && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
            putpixel_function(x2*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
        }




        //Marca redimensionado
        if (cuadrado_activo_resize) {
            //marca de redimensionado
            //		  *
            //		 **
            //		***
            //     ****

            //Arriba del todo
            putpixel_function((x2-1)*menu_gui_zoom,(y2-4)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

            //Medio
            putpixel_function((x2-1)*menu_gui_zoom,(y2-3)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            putpixel_function((x2-2)*menu_gui_zoom,(y2-3)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

            //Abajo
            putpixel_function((x2-1)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            putpixel_function((x2-2)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            putpixel_function((x2-3)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

            //Abajo del todo
            putpixel_function((x2-1)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            putpixel_function((x2-2)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            putpixel_function((x2-3)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);
            putpixel_function((x2-4)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color_marca_redimensionado,menu_gui_zoom);

        }




    }


}

void menu_dibuja_ventana_franja_arcoiris_trozo_current(int trozos)
{

    menu_dibuja_ventana_franja_arcoiris_trozo(current_win_x,current_win_y,current_win_ancho,trozos);
}

void menu_dibuja_ventana_franja_arcoiris_oscuro_current(int indice)
{

    menu_dibuja_ventana_franja_arcoiris_oscuro(current_win_x,current_win_y,current_win_ancho,indice);
}

void zxvision_retorna_coordenadas_marco(int x,int y,int ancho,int alto,int *x1,int *y1,int *x2,int *y2)
{

    int xpixel=x*menu_char_width;
    int ypixel=(y+1)*menu_char_height; //La barra de titulo no tendra linea como tal
    int anchopixel=ancho*menu_char_width;
    int altopixel=alto*menu_char_height;

    int xderecha=xpixel+anchopixel-1;

    *x1=xpixel;
    *y1=ypixel;
    *x2=xderecha;
    *y2=ypixel+altopixel-1-menu_char_height;

}

void zxvision_reapply_style_colours_one_window(zxvision_window *w)
{
    if (w==NULL) return;

    w->dirty_must_draw_contents=1;
    w->dirty_user_must_draw_contents=1;
    w->default_paper=ESTILO_GUI_PAPEL_NORMAL;

    //Cambiar color caracter espacio
    zxvision_change_space_colour(w,ESTILO_GUI_PAPEL_NORMAL);

}

int zxvision_get_effective_height(zxvision_window *w)
{
    //Alto del contenido es 2 menos, por el titulo de ventana y la linea por debajo de margen
    return w->visible_height-2;
}

int zxvision_coords_in_window(zxvision_window *w,int x,int y)
{

    if (!zxvision_show_minimized(w)) return 0;

    int other_x=w->x;
    int other_y=w->y;
    int other_width=w->visible_width;
    int other_height=w->visible_height;

    //printf ("x %d y %d other x %d y %d w %d h %d\n",x,y,other_x,other_y,other_width,other_height);

    if (x>=other_x && x<other_x+other_width &&
        y>=other_y && y<other_y+other_height
        )
        {
            return 1;
        }

    return 0;

}

void zxvision_draw_line_for_filled_triangle(int x1,int y1,int x2,int y2,int min_x,int min_y,int ancho,int *buffer,
    void (*fun_putpixel) (int x,int y,int min_x,int min_y,int ancho,int *buffer) )
{
 int x,y,dx,dy,dx1,dy1,px,py,xe,ye;
 dx=x2-x1;
 dy=y2-y1;
 dx1=util_get_absolute(dx);
 dy1=util_get_absolute(dy);
 px=2*dy1-dx1;
 py=2*dx1-dy1;
 if(dy1<=dx1)
 {
  if(dx>=0)
  {
   x=x1;
   y=y1;
   xe=x2;
  }
  else
  {
   x=x2;
   y=y2;
   xe=x1;
  }
  fun_putpixel(x,y,min_x,min_y,ancho,buffer);
  while(x<xe)
  {
   x=x+1;
   if(px<0)
   {
    px=px+2*dy1;
   }
   else
   {
    if((dx<0 && dy<0) || (dx>0 && dy>0))
    {
     y=y+1;
    }
    else
    {
     y=y-1;
    }
    px=px+2*(dy1-dx1);
   }
   fun_putpixel(x,y,min_x,min_y,ancho,buffer);
  }
 }
 else
 {
  if(dy>=0)
  {
   x=x1;
   y=y1;
   ye=y2;
  }
  else
  {
   x=x2;
   y=y2;
   ye=y1;
  }
  fun_putpixel(x,y,min_x,min_y,ancho,buffer);
  while(y<ye)
  {
   y=y+1;
   if(py<=0)
   {
    py=py+2*dx1;
   }
   else
   {
    if((dx<0 && dy<0) || (dx>0 && dy>0))
    {
     x=x+1;
    }
    else
    {
     x=x-1;
    }
    py=py+2*(dx1-dy1);
   }
   fun_putpixel(x,y,min_x,min_y,ancho,buffer);
  }
 }
}

int menu_allows_mouse(void)
{
    //Primero, fbdev no permite raton
    if (!strcmp(scr_new_driver_name,"fbdev")) return 0;

    //Luego, el resto de los drivers completos (xwindows, sdl, cocoa, ...)

    return si_complete_video_driver();
}

void menu_ventana_draw_perc_bar_aux(zxvision_window *w,int x,int y,z80_byte caracter,int tinta,int papel)
{
    //Ver si caracter final tiene ventana por encima
    int ventana_encima=zxvision_coords_in_superior_windows(w,x,y);

    if (!ventana_encima) {
        putchar_menu_overlay(x,y,caracter,tinta,papel);
    }

}

void reset_splash_zesarux_logo(void)
{
    splash_zesarux_logo_active=0;
}

void screen_print_splash_text_by_window(int lineas)
{


    zxvision_window *ventana;
    ventana=&zxvision_window_splash_text;


    //Si ya existe, cerrarla esa ventana
    if (zxvision_if_window_already_exists(ventana)) {
        debug_printf(VERBOSE_INFO,"Splash window already exists. Closing it and generating a new one");
        zxvision_destroy_window(ventana);
    }



    int xventana,yventana,ancho_ventana,alto_ventana;


    //printf("lineas: %d\n",lineas);

    ancho_ventana=33;
    alto_ventana=lineas+2;

    xventana=menu_center_x()-ancho_ventana/2;
    yventana=menu_center_y()-alto_ventana/2;

    int is_minimized=0;
    int is_maximized=0;
    int ancho_antes_minimize=ancho_ventana;
    int alto_antes_minimize=alto_ventana;


    menu_speech_set_tecla_pulsada(); //no anunciar por speech la creacion de esta ventana

    zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Splash",
        "splashwindow",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);


    zxvision_draw_window(ventana);



}

int menu_first_aid_get_setting(char *texto)
{
    //if (!strcasecmp(texto,"filesel_uppercase_keys")) first_aid_no_filesel_uppercase_keys=1;
    //buscar texto en array
    int i;
    int encontrado=-1;
    for (i=0;i<total_first_aid && encontrado==-1;i++) {
        if (!strcasecmp(texto,first_aid_list[i].config_name)) encontrado=i;
    }

    if (encontrado==-1) {
        debug_printf (VERBOSE_DEBUG,"Can not find first-use setting %s",texto);
        return -1;
    }



    //printf ("setting indice %d nombre [%s]\n",encontrado,first_aid_list[encontrado].config_name);

    //return first_aid_list[i].puntero_setting;

    return encontrado;

}

index_menu *zxvision_index_add_menu(char *titulo_menu_orig)
{

    //Convertir a string sin acentos etc
    char titulo_menu[MAX_LENGTH_FULL_PATH_SUBMENU];

    util_convert_utf_no_utf(titulo_menu_orig,titulo_menu,strlen(titulo_menu_orig));

    //Ir hasta el ultimo
    index_menu *antes=first_index_menu;

    if (antes!=NULL) {

        while (antes->next_menu!=NULL) {
            antes=antes->next_menu;
        }
    }

    //Asignar memoria
    index_menu *nuevo_menu=util_malloc(sizeof(index_menu),"Can not allocate memory for index menu");

    //Asignar titulo
    strcpy(nuevo_menu->titulo_menu,titulo_menu);

    //Siguiente menu a NULL
    nuevo_menu->next_menu=NULL;

    //Primer item de ese menu a null tambien
    nuevo_menu->first_item_menu=NULL;

    //Y decirle al menu anterior que el siguiente es este (o si no hay, este es el primero)
    if (antes==NULL) first_index_menu=nuevo_menu;
    else antes->next_menu=nuevo_menu;

    return nuevo_menu;
}

void zxvision_index_erase_all_menu_lines(index_menu *menu)
{
    index_menu_linea *linea_menu=menu->first_item_menu;

    while (linea_menu!=NULL) {
        //Guardarlo antes de liberar memoria
        index_menu_linea *siguiente_linea=linea_menu->next_item_menu;
        free(linea_menu);

        linea_menu=siguiente_linea;
    }

    menu->first_item_menu=NULL;
}

void zxvision_sound_event_aux(char *nota,int duracion)
{
    if (accessibility_enable_gui_sounds.v) audio_menu_tone_generator_play_note(nota,duracion);
}

void zxvision_draw_filled_triangle_putpixel(zxvision_window *w,int x,int y,int color)
{
    zxvision_putpixel(w,x,y,color);
}

void zxvision_draw_filled_triangle_putpixel_buffer(int x,int y,int min_x,int min_y,int ancho,int *buffer)
{
    //Simplemente indicar en el buffer que usamos esa posicion

    int offset_x=x-min_x;
    int offset_y=y-min_y;


    int offset_final=offset_y*ancho+offset_x;

    buffer[offset_final]=1;
}

void menu_dibuja_cuadrado_putpixel_background(int x,int y,int color,int zoom_level)
{
    int dibujar=1;

    if (menu_dibuja_cuadrado_putpixel_background_ventana!=NULL) {
        int columna=x/menu_char_width/menu_gui_zoom;
        int fila=y/menu_char_height/menu_gui_zoom;
        if (columna==menu_dibuja_cuadrado_putpixel_background_ultima_columna && fila==menu_dibuja_cuadrado_putpixel_background_ultima_fila) {
            dibujar=menu_dibuja_cuadrado_putpixel_background_ultima_dibujar;
        }
        else {
            //esta funcion puede ser costosa si hay muchas ventanas. Por eso usamos calculos cacheados
            if (zxvision_coords_in_superior_windows(menu_dibuja_cuadrado_putpixel_background_ventana,columna,fila)) {
                dibujar=0;
            }
        }

        menu_dibuja_cuadrado_putpixel_background_ultima_columna=columna;
        menu_dibuja_cuadrado_putpixel_background_ultima_fila=fila;
        menu_dibuja_cuadrado_putpixel_background_ultima_dibujar=dibujar;
    }


    if (dibujar) {
        scr_putpixel_gui_zoom(x,y,color,zoom_level);
    }
}

void menu_dibuja_ventana_franja_arcoiris_oscuro(int x, int y, int ancho,int indice)
{

    if (!ventana_tipo_activa) return;

    //int cr[]={2,6,4,5};

    int cr[4];
    //Copiar del estilo actual aqui, pues internamente lo modificamos
    int i;
    int *temp_ptr;
    temp_ptr=ESTILO_GUI_FRANJAS_OSCURAS;
    for (i=0;i<4;i++) {
        cr[i]=temp_ptr[i];
    }
    //int *cr;
    //cr=ESTILO_GUI_FRANJAS_OSCURAS;

    //int indice=4-franjas;

    if (indice>=0 && indice<=3) {
        //cr[indice]+=8;
        //Coger color de las normales brillantes
        int *temp_ptr_brillo;
        temp_ptr_brillo=ESTILO_GUI_FRANJAS_NORMALES;
        cr[indice]=temp_ptr_brillo[indice];
    }

    int restar=0;

    if (zxvision_window_can_be_backgrounded(zxvision_current_window)) restar++;

    x-=restar;

    //Esto va considerando que se tienen 3 botones a la derecha de background, minimizar y maximizar
    //total debe ser 7 para 3 botones
    int margen=ZXVISION_WIDTH_RAINBOW_TITLE+ZXVISION_TOTAL_BUTTONS_RIGHT-1;
    int indice_inicio_franjas=x+ancho-margen;

    z80_byte caracter_franja=128;

    //Quiero que salgan las franjas aunque el estilo no lo permita en ventanas de error y splash message
    //Excepto BEOS
    if (!ESTILO_GUI_NO_RELLENAR_TITULO) {
    //if (1/*ESTILO_GUI_MUESTRA_RAINBOW*/) {

        if (si_complete_video_driver() ) {

            if (ESTILO_GUI_CARACTER_FRANJA!=0) {
                caracter_franja=ESTILO_GUI_CARACTER_FRANJA;

                for (i=0;i<4;i++) {
                    //Empieza desde el final al principio, pues si hay menos franjas
                    int posicion_x=indice_inicio_franjas+4-i;
                    int indice_color=3-i;
                    //printf("Pos: %d indice: %d\n",posicion_x,indice_color);

                     putchar_menu_overlay(posicion_x,y,caracter_franja,cr[indice_color],ESTILO_GUI_PAPEL_TITULO);

                }


            }

            else {


                putchar_menu_overlay(indice_inicio_franjas,y,caracter_franja,cr[0],ESTILO_GUI_PAPEL_TITULO);
                putchar_menu_overlay(indice_inicio_franjas+1,y,caracter_franja,cr[1],cr[0]);
                putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],cr[1]);
                putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
            }
        }

        //en caso de curses o caca, hacerlo con lineas de colores
        if (!strcmp(scr_new_driver_name,"curses") || !strcmp(scr_new_driver_name,"caca") ) {


            putchar_menu_overlay(indice_inicio_franjas+1,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
            putchar_menu_overlay(indice_inicio_franjas+2,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
            putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
            putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
        }

    }
}

void menu_dibuja_ventana_franja_arcoiris_trozo(int x, int y, int ancho,int franjas)
{

    if (!ventana_tipo_activa) return;

    //int cr[]={2+8,6+8,4+8,5+8};
    int *cr;
    cr=ESTILO_GUI_FRANJAS_NORMALES;

    int restar=0;

    if (zxvision_window_can_be_backgrounded(zxvision_current_window)) restar++;

    x-=restar;

    //Esto va considerando que se tienen 3 botones a la derecha de background, minimizar y maximizar
    //total debe ser 7 para 3 botones
    int margen=ZXVISION_WIDTH_RAINBOW_TITLE+ZXVISION_TOTAL_BUTTONS_RIGHT-1;
    int indice_inicio_franjas=x+ancho-margen;

    z80_byte caracter_franja=128;

    //Quiero que salgan las franjas aunque el estilo no lo permita en ventanas de error y splash message
    //Excepto BEOS
    if (!ESTILO_GUI_NO_RELLENAR_TITULO) {
    //if (1/*ESTILO_GUI_MUESTRA_RAINBOW*/) {
        //en el caso de drivers completos, hacerlo real
        if (si_complete_video_driver() ) {
            //5 espacios negro primero
            int i;
            for (i=margen;i>=margen-4;i--) putchar_menu_overlay(x+ancho-i,y,menu_retorna_caracter_espacio_titulo(),ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);

            if (ESTILO_GUI_CARACTER_FRANJA!=0) {
                caracter_franja=ESTILO_GUI_CARACTER_FRANJA;

                for (i=0;i<franjas;i++) {
                    //Empieza desde el final al principio, pues si hay menos franjas
                    int posicion_x=indice_inicio_franjas+4-i;
                    int indice_color=3-i;
                    //printf("Pos: %d indice: %d\n",posicion_x,indice_color);

                    //por si acaso
                    if (indice_color>=0) {
                        putchar_menu_overlay(posicion_x,y,caracter_franja,cr[indice_color],ESTILO_GUI_PAPEL_TITULO);
                    }
                }


            }

            else {

                    if (franjas==4) {
                        putchar_menu_overlay(indice_inicio_franjas,y,caracter_franja,cr[0],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+1,y,caracter_franja,cr[1],cr[0]);
                        putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],cr[1]);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }

                     if (franjas==3) {
                        putchar_menu_overlay(indice_inicio_franjas+1,y,caracter_franja,cr[1],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],cr[1]);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }


                    if (franjas==2) {
                        putchar_menu_overlay(indice_inicio_franjas+2,y,caracter_franja,cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],cr[2]);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }

                    if (franjas==1) {
                        putchar_menu_overlay(indice_inicio_franjas+3,y,caracter_franja,cr[3],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,caracter_franja,ESTILO_GUI_PAPEL_TITULO,cr[3]);
                    }
            }


        }

        //en caso de curses o caca, hacerlo con lineas de colores
            if (!strcmp(scr_new_driver_name,"curses") || !strcmp(scr_new_driver_name,"caca") ) {
                    //putchar_menu_overlay(x+ancho-6,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
                    //putchar_menu_overlay(x+ancho-5,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                    //putchar_menu_overlay(x+ancho-4,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                    //putchar_menu_overlay(x+ancho-3,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);


                    //5 espacios negro primero
                    int i;
                    for (i=margen;i>=margen-4;i--) putchar_menu_overlay(x+ancho-i,y,' ',ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_PAPEL_TITULO);
                    if (franjas==4) {
                        putchar_menu_overlay(indice_inicio_franjas+1,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+2,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }

                     if (franjas==3) {
                        putchar_menu_overlay(indice_inicio_franjas+2,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }


                    if (franjas==2) {
                        putchar_menu_overlay(indice_inicio_franjas+3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }

                    if (franjas==1) {
                        putchar_menu_overlay(indice_inicio_franjas+4,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
                    }
            }
    }
}

void menu_dibuja_ventana_titulo(zxvision_window *w,char *titulo_original_utf)
{

    int x=w->x;
    int y=w->y;
    int ancho=w->visible_width;
    int alto=w->visible_height;


    //Convertir de cadena utf con posibles acentos a caracteres internos
    char titulo_original[ZXVISION_MAX_WINDOW_TITLE];
    util_convert_utf_charset(titulo_original_utf,(z80_byte *)titulo_original,strlen(titulo_original_utf));


    //Pasar titulo a string temporal. Agregamos un espacio al final en estilos que no rellenan toda la barra de titulo (como BeOS)
    char titulo[ZXVISION_MAX_WINDOW_TITLE];

    sprintf(titulo,"%s%s",titulo_original,(ESTILO_GUI_NO_RELLENAR_TITULO ? " " : "")  );


    //printf ("valor menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);

    int i,j;


    //contenido en blanco normalmente en estilo ZEsarUX
    //Sin usar cache
    //Para evitar por ejemplo que ventanas como daad graphics, que aparecen encima de otra ventana tipo visualmem o audio chip piano con pixeles,
    //que esos pixeles no se metan "dentro" de la ventana de daad graphics
    for (i=0;i<alto-1;i++) {
        for (j=0;j<ancho;j++) {
            putchar_menu_overlay_parpadeo_cache_or_not(x+j,y+i+1,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,0);
        }
    }


    int color_tinta_titulo;
    int color_papel_titulo;


    if (ventana_tipo_activa) {
        color_tinta_titulo=ESTILO_GUI_TINTA_TITULO;
        color_papel_titulo=ESTILO_GUI_PAPEL_TITULO;
    }

    else {
        color_tinta_titulo=ESTILO_GUI_TINTA_TITULO_INACTIVA;
        color_papel_titulo=ESTILO_GUI_PAPEL_TITULO_INACTIVA;
    }

    z80_byte caracter_espacio_titulo=menu_retorna_caracter_espacio_titulo();

    //si ventana es background o no es ventana activa, caracter fondo titulo es siempre espacio
    if (ventana_es_background || !ventana_tipo_activa) caracter_espacio_titulo=' ';

    //titulo
    //primero franja toda negra normalmente en estilo ZEsarUX
    if (w->do_not_have_title_bar==0) {
    for (i=0;i<ancho;i++) {
        if (ESTILO_GUI_NO_RELLENAR_TITULO) {
            //Caso del estilo BeOS, si estan los botones de antes, al irse la ventana a background,
            //hay que borrar botones
            deletechar_menu_overlay(x+i,y);
        }
        else {
            putchar_menu_overlay(x+i,y,caracter_espacio_titulo,color_tinta_titulo,color_papel_titulo);
        }
    }
    }


    int ancho_mostrar_titulo=menu_dibuja_ventana_ret_ancho_titulo(ancho,titulo);

    char titulo_mostrar[ZXVISION_MAX_WINDOW_TITLE];
    z80_byte caracter_cerrar=menu_retorna_caracter_cerrar();



    if (menu_hide_close_button.v || ventana_es_background || !ventana_tipo_activa) {
        //strcpy(titulo_mostrar,titulo);
        //Ancho del titulo sera igual, aun sin el boton de cerrar
        sprintf (titulo_mostrar,"%c%c%s",caracter_espacio_titulo,caracter_espacio_titulo,titulo);
    }
    else {
        sprintf (titulo_mostrar,"%c%c%s",caracter_cerrar,caracter_espacio_titulo,titulo);
    }


    //y luego el texto. titulo mostrar solo lo que cabe de ancho


    for (i=0;i<ancho_mostrar_titulo && titulo_mostrar[i];i++) {
        char caracter_mostrar=titulo_mostrar[i];
        if (w->do_not_have_title_bar==0) {
        putchar_menu_overlay(x+i,y,caracter_mostrar,color_tinta_titulo,color_papel_titulo);
        }
    }

    //Indicar posicion del boton minimizar
    current_win_minimize_button_position=i+1;

    if (current_win_minimize_button_position+1>=ancho) current_win_minimize_button_position=ancho-2;




    char buffer_titulo[100];
    sprintf (buffer_titulo,"Window: %s",titulo);
    menu_textspeech_send_text(buffer_titulo);


}

void zxvision_change_space_colour(zxvision_window *w,int papel)
{

    int total_width=w->total_width;
    int total_height=w->total_height;

    int buffer_size=total_width*total_height;


    int i;
    overlay_screen *p;
    p=w->memory;

    for (i=0;i<buffer_size;i++) {
        if (p->caracter==' ') {

            p->papel=papel;

        }

        p++;
    }
}

void putchar_menu_overlay_parpadeo_cache_or_not(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo,int use_cache_mismo_caracter)
{

    //int xusado=x;

    //if (menu_char_width!=8) {
        //xusado=(x*menu_char_width)/8;
    //}

    //int xfinal=((x*menu_char_width)+menu_char_width-1)/8;

    //Controlar limite
    if (x<0 || y<0 || x>=scr_get_menu_width() || y>=scr_get_menu_height() ) {
        //printf ("Out of range. X: %d Y: %d Character: %c\n",x,y,caracter);
        return;
    }

    int pos_array=y*scr_get_menu_width()+x;
    if (ESTILO_GUI_SOLO_MAYUSCULAS) caracter=letra_mayuscula(caracter);

/*
Otra optimizacion mas. Si se escribe el mismo caracter con iguales atributos, decimos que esta en cache
Con esto se pasa, por ejemplo, enseñando el Sonic, con varias ventanas de menu abiertas, de usar
88 % de cpu
a usar
45% de cpu

Esto requiere por otra parte que, ventanas que dibujan con putpixel (como waveform, audio chip sheet etc)
esperan que siempre se borre con espacios la ventana y luego ellos escriben encima sus pixeles con el fondo "ya limpio"
Esto ya no sucede mas, pues el fondo limpio con espacios, al no alterarse, no se redibuja limpiando los pixeles anteriores
Requiere entonces que llamen a una función que limpia la ventana e indicando parametro de .modificado
*/
//#ifdef ZXVISION_USE_CACHE_OVERLAY_TEXT
    //Cualquier atributo alterado del caracter, o el propio caracter, decimos a la cache que se ha alterado y hay que redibujar
    if (
        use_cache_mismo_caracter==0 ||
        overlay_screen_array[pos_array].tinta!=tinta ||
        overlay_screen_array[pos_array].papel!=papel ||
        overlay_screen_array[pos_array].parpadeo!=parpadeo ||
        overlay_screen_array[pos_array].caracter!=caracter
    ) {
        overlay_screen_array[pos_array].modificado=1;
    }
//#endif

    overlay_screen_array[pos_array].tinta=tinta;
    overlay_screen_array[pos_array].papel=papel;
    overlay_screen_array[pos_array].parpadeo=parpadeo;
    overlay_screen_array[pos_array].caracter=caracter;


    //overlay_usado_screen_array[y*scr_get_menu_width()+xusado]=1;

}

int menu_dibuja_ventana_ret_ancho_titulo(int ancho,char *titulo)
{
    int ancho_mostrar_titulo=menu_da_ancho_titulo(titulo);

    int ancho_disponible_titulo=ancho;

    //printf("menu_da_ancho_titulo: %3d ancho_disponible_titulo %3d\n",ancho_mostrar_titulo,ancho_disponible_titulo);

    if (ancho_disponible_titulo<ancho_mostrar_titulo) ancho_mostrar_titulo=ancho_disponible_titulo;

    return ancho_mostrar_titulo;
}

z80_byte menu_retorna_caracter_cerrar(void)
{

    z80_byte caracter=ESTILO_GUI_BOTON_CERRAR;

    //Si caracter es un udg especial y no es driver video completo, retornar por defecto
    if (caracter>126 && !si_complete_video_driver()) return '*';

    else return caracter;
}

void menu_ext_desktop_buttons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_inicio_botones,int *p_xfinal_botones)
{
    int total_botones=EXT_DESKTOP_TOTAL_BUTTONS;

    int ancho_zx_desktop=screen_get_ext_desktop_width_zoom();
    int xinicio=screen_get_ext_desktop_start_x();


    int ancho_boton=ancho_zx_desktop/total_botones;

    //Minimo 32 pixeles
    if (ancho_boton<EXT_DESKTOP_BUTTONS_TOTAL_SIZE) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE;

    //Maximo 64 pixeles
    if (ancho_boton>EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2;


    int alto_boton=ancho_boton;

    int xfinal_ventana=xinicio+ancho_zx_desktop;

    int xfinal_botones=xinicio+total_botones*ancho_boton;

    //no caben todos los botones
    if (xfinal_botones>xfinal_ventana) {
        total_botones=ancho_zx_desktop/ancho_boton;
        xfinal_botones=xinicio+total_botones*ancho_boton;
    }

    if (p_ancho_boton!=NULL) *p_ancho_boton=ancho_boton;
    if (p_alto_boton!=NULL) *p_alto_boton=alto_boton;
    if (p_total_botones!=NULL) *p_total_botones=total_botones;
    if (p_inicio_botones!=NULL) *p_inicio_botones=xinicio;
    if (p_xfinal_botones!=NULL) *p_xfinal_botones=xfinal_botones;

}

void menu_ext_desktop_lower_buttons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_xinicio_botones,int *p_xfinal_botones,int *p_yinicio_botones)
{

    //int total_botones=TOTAL_ZXDESKTOP_MAX_LOWER_BUTTONS;

    //Considerar los botones que estan visibles solamente para la geometria, no todos los posibles


    int total_botones=0;


    int i;


    for (i=0;i<TOTAL_ZXDESKTOP_MAX_LOWER_BUTTONS;i++) {
        int (*funcion_is_visible)(void);
        funcion_is_visible=zdesktop_lowericons_array[i].is_visible;

        int visible=funcion_is_visible();

//Para poder forzar visibilidad de iconos para debug
#ifdef FORCE_VISIBLE_ALL_LOWER_BUTTONS
    visible=1;
#endif
        if (visible) {

            total_botones++;
        }
    }

    //printf ("total iconos visibles: %d\n",total_botones);

    int ancho_zx_desktop=screen_get_ext_desktop_width_zoom();
    int xinicio=screen_get_ext_desktop_start_x();


    int ancho_boton;

    if (total_botones==0) {
        //evitar división por cero
        ancho_boton=32;
    }
    else {
        ancho_boton=ancho_zx_desktop/total_botones;
    }

    //Minimo 32 pixeles
    if (ancho_boton<EXT_DESKTOP_BUTTONS_TOTAL_SIZE) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE;

    //Maximo 64 pixeles
    if (ancho_boton>EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2) ancho_boton=EXT_DESKTOP_BUTTONS_TOTAL_SIZE*2;


    int alto_boton=ancho_boton;

    int xfinal_ventana=xinicio+ancho_zx_desktop;

    int xfinal_botones=xinicio+total_botones*ancho_boton;

    //no caben todos los botones
    if (xfinal_botones>xfinal_ventana) {
        total_botones=ancho_zx_desktop/ancho_boton;
        xfinal_botones=xinicio+total_botones*ancho_boton;
    }


/*

    int xinicio=screen_get_ext_desktop_start_x();


    int ancho=screen_get_ext_desktop_width_zoom();


    int xfinal;



    int nivel_zoom=1;

    //Si hay espacio para meter iconos con zoom 2
    //6 pixeles de margen
    if (ancho_boton>=(6+EXT_DESKTOP_BUTTONS_ANCHO*2)) nivel_zoom=2;




*/
    int alto_zx_desktop=screen_get_total_alto_ventana_zoom();
    int yinicio=alto_zx_desktop-alto_boton;

    //printf ("alto_boton: %d alto_zx_desktop: %d yinicio: %d\n",alto_boton,alto_zx_desktop,yinicio);

    if (p_ancho_boton!=NULL) *p_ancho_boton=ancho_boton;
    if (p_alto_boton!=NULL) *p_alto_boton=alto_boton;
    if (p_total_botones!=NULL) *p_total_botones=total_botones;
    if (p_xinicio_botones!=NULL) *p_xinicio_botones=xinicio;
    if (p_xfinal_botones!=NULL) *p_xfinal_botones=xfinal_botones;
    if (p_yinicio_botones!=NULL) *p_yinicio_botones=yinicio;

}
