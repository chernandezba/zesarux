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


//
// Top bar menu functions
//


#include "zxvision_topbar.h"
#include "cpu.h"
#include "zxvision.h"
#include "joystick.h"
#include "debug.h"
#include "settings.h"
#include "menu_items.h"
#include "menu_items_settings.h"
#include "charset.h"
#include "screen.h"

int previous_switchtopbar_timer_event_mouse_x=0;
int previous_switchtopbar_timer_event_mouse_y=0;
//si estaba visible o no
z80_bit switchtopbar_button_visible={0};

//temporizador desde que empieza a no moverse
int switchtopbar_button_visible_timer=0;

void topbar_make_switchbutton_visible(void)
{

    if (zxvision_topbar_menu_enabled.v==0) return;

    debug_printf(VERBOSE_INFO,"Make topbar switch button visible");
    printf("Make topbar switch button visible\n");
    switchtopbar_button_visible.v=1;

}

#define MAX_SWITCH_TOPBAR_VISIBLE_TIMER 100

void topbar_make_switchbutton_invisible(void)
{
    if (zxvision_topbar_menu_enabled.v==0) return;

    //Esto puede ser redundante desde abajo donde se llama pero a esta funcion se llama desde otros sitios
    //e interesa establecer el timer como conviene
    switchtopbar_button_visible_timer=MAX_SWITCH_TOPBAR_VISIBLE_TIMER;

    debug_printf(VERBOSE_INFO,"Make topbar switch button hidden");
    printf("Make topbar switch button hidden\n");
    switchtopbar_button_visible.v=0;

    if (menu_abierto) return;

    //Para borrar el texto de topbar
    cls_menu_overlay();

}

//Ocultar o mostrar topbar cuando menu cerrado
void topbar_timer_event(void)
{

    if (zxvision_topbar_menu_enabled.v==0) return;


    int movido=0;

    if (previous_switchtopbar_timer_event_mouse_x!=mouse_x || previous_switchtopbar_timer_event_mouse_y!=mouse_y)
    {
        movido=1;
    }

    previous_switchtopbar_timer_event_mouse_x=mouse_x;
    previous_switchtopbar_timer_event_mouse_y=mouse_y;


    //No estaba visible
    if (switchtopbar_button_visible.v==0) {
        if (movido) {
            topbar_make_switchbutton_visible();
        }
    }

    //Estaba visible
    else {
        if (movido) {
            switchtopbar_button_visible_timer=0;
        }

        else {
            switchtopbar_button_visible_timer++;

            //en 2 segundos (50*2 frames) desaparece
            if (switchtopbar_button_visible_timer==MAX_SWITCH_TOPBAR_VISIBLE_TIMER) {
                topbar_make_switchbutton_invisible();
            }
        }

    }
}





z80_byte menu_topbarmenu_get_key(void)
{
    z80_byte tecla;

    if (!menu_multitarea) {
            //printf ("refresca pantalla\n");
            menu_refresca_pantalla();
    }


    menu_cpu_core_loop();


    menu_espera_tecla();
    tecla=zxvision_read_keyboard();



    return tecla;
}


int dibujar_cursor_topbar=0;
int dibujar_cursor_topbar_pos_cursor=0;

//Generar posiciones de donde está cada menu
//20 posiciones maximo, incluyendo el primero
int posiciones_menus[20];

//lo defino como un array de char para que pueda cambiar el caracter 0 por la Z pequeña del logo
                               //01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
                               //0         1         2         3         4         5         6         7         8         9         10
char topbar_string_linea_menus[]="Z  Smartload  Snapshot  Machine  Audio  Display  Storage  Debug  Network  Windows  Settings  Help";

void menu_topbarmenu_write_bar(void)
{

    //Aunque topbar en principio solo va a estar con drivers completos, por si acaso solo lo cambio en esos casos
    //asi seria compatible con curses por ejemplo, dejaria la "Z" normal
    if (si_complete_video_driver() ) {
        //El primer caracter lo cambiamos por la Z pequeña del logo, que tiene colores
        topbar_string_linea_menus[0]=(unsigned char) CHAR_Z_LOGO_SMALL_TOPBAR;
    }

    //Y si el estilo es retromac, metemos icono de manzana en vez de la Z
    //if (char_set==char_set_retromac) {
    //    topbar_string_linea_menus[0]=(unsigned char) APPLE_LOGO_IN_CHARSET_RETROMAC;
    //}


    menu_escribe_texto(0,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,topbar_string_linea_menus);

    if (dibujar_cursor_topbar) {
        //printf("dibujar top bar cursor\n");

        int x_inicio=posiciones_menus[dibujar_cursor_topbar_pos_cursor];
        //menu_escribe_texto(x_inicio,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"*");
        int x_final=posiciones_menus[dibujar_cursor_topbar_pos_cursor+1]-1;
        //menu_escribe_texto(x_final,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"=");

        //Escribir ese nombre de menu seleccionado en color inverso
        for (;x_inicio<=x_final;x_inicio++) {
            putchar_menu_overlay_parpadeo(x_inicio,0,topbar_string_linea_menus[x_inicio],ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,0);
            //menu_escribe_texto(x_inicio,0,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,topbar_string_linea_menus[x_inicio]);
        }
    }
}

int get_pos_y_mouse_topbar(void)
{
    int posicion_y=mouse_y/menu_char_height/menu_gui_zoom/zoom_y;

    return posicion_y;
}

int if_menu_topbarmenu_pressed_bar(void)
{
    int posicion_y=get_pos_y_mouse_topbar();

    if (posicion_y==0) return 1;
    else return 0;

}


int if_menu_topbarmenu_enabled_and_pressed_bar(void)
{
    if (zxvision_topbar_menu_enabled.v==0) return 0;

    return if_menu_topbarmenu_pressed_bar();
}

void menu_topbarmenu_flecha_izquierda(void)
{
    if (dibujar_cursor_topbar_pos_cursor>0) dibujar_cursor_topbar_pos_cursor--;
}

void menu_topbarmenu_flecha_derecha(int total_menus)
{
    if (dibujar_cursor_topbar_pos_cursor<total_menus-1) dibujar_cursor_topbar_pos_cursor++;
}

//Indica que se ha pulsado en la barra de menu antes de entrar en menu_topbarmenu()
int menu_topbarmenu_pressed_bar=0;

void menu_topbarmenu(void)
{
    printf("Entramos en topbar menu. mouse_left: %d menu_topbarmenu_pressed_bar: %d\n",mouse_left,menu_topbarmenu_pressed_bar);

    //tecla Z para primer menu ZEsarUX
    //tecla S para Smartload
    //tecla N para snapshot
    //etc
    char *topbar_hotkeys="zsnmadteowil";

    posiciones_menus[0]=0;

    int i,total_posiciones;
    int leido_espacio=0;
    for (i=0,total_posiciones=1;topbar_string_linea_menus[i];i++) {
        if (leido_espacio) {
            if (topbar_string_linea_menus[i]!=' ') {
                //printf("posicion %d i %d\n",total_posiciones,i);
                posiciones_menus[total_posiciones++]=i;
                leido_espacio=0;
            }
        }
        else {
            if (topbar_string_linea_menus[i]==' ') leido_espacio=1;
        }
    }

    //El del menu Help
    posiciones_menus[total_posiciones++]=i;

    int tecla_leida=0;
    int salir_topbar=0;

    //int dibujar_cursor_topbar_pos_cursor=0;

    int total_menus=total_posiciones-1;

    if (ultimo_menu_salido_con_flecha_izquierda) {
        menu_topbarmenu_flecha_izquierda();
    }

    if (ultimo_menu_salido_con_flecha_derecha) {
        menu_topbarmenu_flecha_derecha(total_menus);
    }

    //Si hemos salido de menus con flecha izquierda o derecha, desplegar el siguiente menu , pero,
    //si el siguiente es smartload, se desplega una ventana y dejamos de navegar
    //por tanto en ese caso, si es smartload, no desplegaremos menu sino cursor arriba
    if (dibujar_cursor_topbar_pos_cursor==1) {
        ultimo_menu_salido_con_flecha_izquierda=ultimo_menu_salido_con_flecha_derecha=0;
    }

    do {

        //Esperar tecla/raton, y siempre que no se haya entrado abriendo el menu pulsando ya en barra superior
        //o se haya pulsado flecha izquierda o derecha en menus abiertos
        //Moverse por los titulos superiores
        int salir_linea_superior=0;
        while (!salir_linea_superior && !menu_topbarmenu_pressed_bar && !ultimo_menu_salido_con_flecha_izquierda && !ultimo_menu_salido_con_flecha_derecha) {

            dibujar_cursor_topbar=1;

            printf("cursor_pos %d total_menus %d\n",dibujar_cursor_topbar_pos_cursor,total_menus);

            menu_refresca_pantalla();

            tecla_leida=menu_topbarmenu_get_key();

            printf("tecla leida: %d\n",tecla_leida);

            if (mouse_left) {
                tecla_leida=13;
            }

            //tecla abajo es como enter
            if (tecla_leida==10) tecla_leida=13;

            if (tecla_leida==13) salir_linea_superior=1;

            if (tecla_leida!=13) {

                menu_espera_no_tecla();

                if (tecla_leida>='a' && tecla_leida<='z') {
                    //ver hotkeys
                    for (i=0;topbar_hotkeys[i];i++) {
                        if (tecla_leida==topbar_hotkeys[i]) break;
                    }
                    if (topbar_hotkeys[i]) {
                        dibujar_cursor_topbar_pos_cursor=i;
                        tecla_leida=13;
                        salir_linea_superior=1;
                    }
                }

                switch(tecla_leida) {
                    case 8:
                        printf("izquierda\n");
                        menu_topbarmenu_flecha_izquierda();
                    break;

                    case 9:
                        printf("derecha\n");
                        menu_topbarmenu_flecha_derecha(total_menus);
                    break;
                }
            }

        }

        dibujar_cursor_topbar=0;

        //Si pulsado boton raton o enter en el paso anterior o se haya entrado abriendo el menu pulsando ya en barra superior
        if ( tecla_leida==13 || menu_topbarmenu_pressed_bar || ultimo_menu_salido_con_flecha_izquierda || ultimo_menu_salido_con_flecha_derecha) {

            //Por si acaso reseteamos estos dos, ya no se necesitan para ver que hemos salido de un menu pulsando cursor izquierda o derecha
            //creo que realmente no haria falta, pero por si en un futuro hay manera de volver a entrar en esta funcion sin pasar por dibujado
            //del menu, que es donde se resetean tambien
            ultimo_menu_salido_con_flecha_izquierda=ultimo_menu_salido_con_flecha_derecha=0;

            int columna_posicion_x;

            //Asumimos no pulsado bien
            int pos_cursor=-1;

            int posicion_y=0;

            //Entrado por raton
            if ((tecla_leida==13 && mouse_left) || menu_topbarmenu_pressed_bar) {
                menu_topbarmenu_pressed_bar=0;
                columna_posicion_x=mouse_x/menu_char_width/menu_gui_zoom/zoom_x;
                posicion_y=get_pos_y_mouse_topbar();

                if (posicion_y==0) {

                    //Detectar que menu hemos pulsado
                    int i;
                    for (i=0;i<total_posiciones;i++) {
                        if (columna_posicion_x<posiciones_menus[i]) break;
                    }

                    if (i<total_posiciones) {
                        pos_cursor=i-1;
                    }
                }
            }

            //Entrado por teclado
            else {
                pos_cursor=dibujar_cursor_topbar_pos_cursor;
            }

            if (posicion_y==0) {
                menu_espera_no_tecla_con_repeticion();

                force_next_menu_position.v=1;
                force_next_menu_position_y=1;
            }


            //int posicion_y=mouse_y/menu_char_height/menu_gui_zoom/zoom_y;

            //printf("posicion x: %d\n",columna_posicion_x);

            printf("posicion cursor: %d\n",pos_cursor);


            //asumimos que saldremos del topbar
            salir_topbar=1;

            if (posicion_y==0 && pos_cursor>=0) {

                //actualizar posicion de cursor global con lo calculado segun el raton
                dibujar_cursor_topbar_pos_cursor=pos_cursor;

                force_next_menu_position_x=posiciones_menus[pos_cursor];

                //hemos pulsado en topbar, nos mantenemos
                salir_topbar=0;

                switch(pos_cursor) {
                    case 0:
                        menu_inicio_bucle_main();
                    break;

                    case 1:
                        menu_smartload(0);
                    break;

                    case 2:
                        menu_snapshot(0);
                    break;

                    case 3:
                        menu_machine_selection(0);
                    break;

                    case 4:
                        menu_audio(0);
                    break;

                    case 5:
                        menu_display_settings(0);
                    break;

                    case 6:
                        menu_storage(0);
                    break;

                    case 7:
                        menu_debug_main(0);
                    break;

                    case 8:
                        menu_network(0);
                    break;

                    case 9:
                        menu_windows(0);
                    break;

                    case 10:
                        menu_settings(0);
                    break;

                    case 11:
                        menu_help(0);
                    break;
                }

                printf("despues switch. if_menu_topbarmenu_pressed_bar= %d mouse_left= %d\n",
                    if_menu_topbarmenu_pressed_bar(),mouse_left);

                //Necesario para cerrar submenus, por ejemplo si estamos en un item de menu con submenus,
                //y simplemente pulsamos fuera del menu, con lo que se simula pulsado ESC
                //pero deja submenus abiertos
                menu_dibuja_submenu_cierra_todos_submenus();


                if (ultimo_menu_salido_con_flecha_izquierda || ultimo_menu_salido_con_flecha_derecha) {
                    printf("Ultimo menu pulsado izquierda o derecha\n");
                }


                if (if_menu_topbarmenu_pressed_bar() && mouse_left)  {
                    //continuamos aqui y sin tener que esperar tecla
                    printf("Reentraremos en menu\n");
                    menu_topbarmenu_pressed_bar=1;
                }

            }


        }

        //temp salimos siempre
        //el problema es que para reentrar hay que tener mouse_left activo, pero de cada menu
        //siempre se sale y se espera a liberar boton, entonces mouse_left sera 0 siempre porque el boton no se estara pulsando
        //Hay que reentrar aqui de otra manera
        salir_topbar=1;

        //salir en caso que pulsado en otra zona que no es menu
    } while (!salir_topbar);

    menu_espera_no_tecla_con_repeticion();

    printf("salir menu_topbarmenu\n");
}


void topbar_text_overlay(void)
{
    //Dibujar topbar
    if (zxvision_topbar_menu_enabled.v) {
        int mostrar_topbar=0;
        if (menu_abierto) mostrar_topbar=1;

        if (overlay_visible_when_menu_closed) {
            //si menu cerrado pero se ha movido raton
            if (switchtopbar_button_visible.v) mostrar_topbar=1;
        }

        if (mostrar_topbar) {
            menu_topbarmenu_write_bar();
        }
    }
}