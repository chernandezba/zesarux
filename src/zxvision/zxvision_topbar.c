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
z80_bit topbar_esta_visible_por_timer={0};

//temporizador desde que empieza a no moverse
int switchtopbar_button_visible_timer=0;

//indica a la funcion de overlay que estamos en el topbar
int topbar_overlay_we_are_on_topbar=0;

int dibujar_cursor_topbar=0;
int dibujar_cursor_topbar_pos_cursor=0;

//Dice si estan los menus desplegados o solo estamos en la linea superior
int topbar_menu_desplegado=0;

//Si se muestran los hotkeys por timer
int topbarmenu_mostrar_hotkeys_por_timer=0;

//Generar posiciones de donde está cada menu
//20 posiciones maximo, incluyendo el primero
int posiciones_menus[20];

//lo defino como un array de char para que pueda cambiar el caracter 0 por la Z pequeña del logo
                                                  //01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
                                                  //0         1         2         3         4         5         6         7         8         9         10
char topbar_string_linea_menus_with_zxdesktop[]=   "Z  Smartload  Snapshot  Machine  Audio  Display  Storage  Debug  Network  Windows  Settings  Help";
char topbar_string_linea_menus_without_zxdesktop[]="Z SL Sna Mch Aud Dsp Sto Dbg Net Win Set Hlp";

//tecla Z para primer menu ZEsarUX
//tecla S para Smartload
//tecla N para snapshot
//etc
char *topbar_hotkeys="zsnmadteowil";

//Indica que se ha pulsado en la barra de menu antes de entrar en menu_topbarmenu()
int menu_topbarmenu_pressed_bar=0;

#define MAX_SWITCH_TOPBAR_VISIBLE_TIMER 100

void topbar_make_topbar_visible(void)
{

    if (zxvision_topbar_menu_enabled.v==0) return;

    debug_printf(VERBOSE_INFO,"Make topbar visible");
    printf("Make topbar visible\n");
    topbar_esta_visible_por_timer.v=1;

}


void reset_topbar_overlay_we_are_on_topbar(void)
{
    if (zxvision_topbar_menu_enabled.v==0) return;

    topbar_overlay_we_are_on_topbar=0;

    dibujar_cursor_topbar=0;


    //TODO: realmente no la borramos del texto overlay,
    //sino que le decimos que no esta ahi,
    //y supuestamente en los siguientes refrescos enteros del entorno zxvision
    //se borrara todo el overlay y desaparecera
    //Si a veces no desaparece, habria que escribir en la primera
    //linea de overlay caracteres a 0, para "liberar" esas posiciones de overlay y que se vea el fondo
    //Esto conllevaria un efecto secundario y es que si hay alguna ventana que ocupa esa primera linea,
    //se borraria ese texto de la primera linea, aunque no deberia pasar, normalmente las ventanas
    //estaran por debajo de esa linea


}

void topbar_make_topbar_invisible(void)
{
    if (zxvision_topbar_menu_enabled.v==0) return;

    //Esto puede ser redundante desde abajo donde se llama pero a esta funcion se llama desde otros sitios
    //e interesa establecer el timer como conviene
    switchtopbar_button_visible_timer=MAX_SWITCH_TOPBAR_VISIBLE_TIMER;

    debug_printf(VERBOSE_INFO,"Make topbar hidden");
    printf("Make topbar hidden\n");
    topbar_esta_visible_por_timer.v=0;


    if (topbar_overlay_we_are_on_topbar) return;

    //Para borrar el texto de topbar
    cls_menu_overlay();

    zxvision_redraw_all_windows();
    //menu_refresca_pantalla();

}

//Retorna la fila donde está el ratón
int get_pos_y_mouse_topbar(void)
{
    int posicion_y=mouse_y/menu_char_height/menu_gui_zoom/zoom_y;

    return posicion_y;
}

int get_pos_x_mouse_topbar(void)
{
    int columna_posicion_x=mouse_x/menu_char_width/menu_gui_zoom/zoom_x;
    return columna_posicion_x;
}

//Ocultar o mostrar topbar cuando menu cerrado
void topbar_timer_event(void)
{

    if (zxvision_topbar_menu_enabled.v==0) return;

    if (zxvision_topbar_appears_move_mouse_top.v==0) return;

    int movido=0;

    if (previous_switchtopbar_timer_event_mouse_x!=mouse_x || previous_switchtopbar_timer_event_mouse_y!=mouse_y)
    {
        movido=1;
    }

    previous_switchtopbar_timer_event_mouse_x=mouse_x;
    previous_switchtopbar_timer_event_mouse_y=mouse_y;

    /*
    if (movido) {
        if (zxvision_mouse_in_zesarux_window()) {
            printf("En ventana\n");
        }
        else {
            printf("NO en ventana\n");
        }
    }
    */


    //No estaba visible
    if (topbar_esta_visible_por_timer.v==0) {
        if (movido && get_pos_y_mouse_topbar()==0 && zxvision_mouse_in_zesarux_window()) {
            topbar_make_topbar_visible();
        }
    }

    //Estaba visible
    else {
        if (movido && get_pos_y_mouse_topbar()==0) {
            switchtopbar_button_visible_timer=0;
        }

        else {
            switchtopbar_button_visible_timer++;

            //en 2 segundos (50*2 frames) desaparece
            if (switchtopbar_button_visible_timer==MAX_SWITCH_TOPBAR_VISIBLE_TIMER) {
                topbar_make_topbar_invisible();
            }
        }

    }
}



z80_byte menu_topbarmenu_get_key(void)
{

    menu_tooltip_counter=0;

    z80_byte tecla;

    if (!menu_multitarea) {
            //printf ("refresca pantalla\n");
            menu_refresca_pantalla();
    }


    menu_cpu_core_loop();



    menu_espera_tecla_timeout_tooltip();
    printf("Despues timeout\n");



    //menu_espera_tecla();
    tecla=zxvision_read_keyboard();

    //Ver los hotkeys si ha pasado el timer o si se pulsa una tecla de letra
    if (menu_tooltip_counter>=TOOLTIP_SECONDS || (tecla>='a' && tecla<='z') ) {
        topbarmenu_mostrar_hotkeys_por_timer=1;
    }


    return tecla;
}





char *menu_topbar_get_text_topbar(void)
{
    if (if_zxdesktop_enabled_and_driver_allows() ) return topbar_string_linea_menus_with_zxdesktop;
    else return topbar_string_linea_menus_without_zxdesktop;
}


void menu_topbarmenu_write_bar(void)
{
    //printf("Escribir topbar\n");

    //
    char *topbar_string_linea_menus=menu_topbar_get_text_topbar();

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

    int pos_menu=0;

    int x;
    for (x=0;topbar_string_linea_menus[x];x++) {
        int tinta=ESTILO_GUI_TINTA_NORMAL;
        int papel=ESTILO_GUI_PAPEL_NORMAL;

        char caracter_escribir=topbar_string_linea_menus[x];

        //Resaltar menu donde apunta el cursor
        if (dibujar_cursor_topbar && caracter_escribir!=' ') {
            //printf("dibujar top bar cursor\n");

            int x_inicio=posiciones_menus[dibujar_cursor_topbar_pos_cursor];

            int x_final=posiciones_menus[dibujar_cursor_topbar_pos_cursor+1]-1;

            if (x>=x_inicio && x<=x_final) {
                tinta=ESTILO_GUI_TINTA_SELECCIONADO;
                papel=ESTILO_GUI_PAPEL_SELECCIONADO;
            }

        }



        if (posiciones_menus[pos_menu]==x) {
            //Esta en una posicion de hotkey
            pos_menu++;

            //Mostrar hotkeys
            //Que aparezcan solo cuando es visible la primera linea y ademas,
            //cuando estemos realmente dentro, no cuando aparece al mover el raton arriba y sin pulsar
            int mostrar_hotkeys=0;

            if (menu_force_writing_inverse_color.v) mostrar_hotkeys=1;
            if (topbarmenu_mostrar_hotkeys_por_timer) mostrar_hotkeys=1;

            if (!topbar_menu_desplegado && topbar_overlay_we_are_on_topbar && mostrar_hotkeys) {
                //printf("mostrar hotkey en x %d\n",x);
                if (ESTILO_GUI_INVERSE_TINTA!=-1) {
                    tinta=ESTILO_GUI_INVERSE_TINTA;
                }
                else {
                    //Intercambiar
                    int temp_papel=papel;
                    papel=tinta;
                    tinta=temp_papel;
                }
            }
        }

        putchar_menu_overlay_parpadeo(x,0,caracter_escribir,tinta,papel,0);
    }

    /*
    if (dibujar_cursor_topbar) {
        //printf("dibujar top bar cursor\n");

        int x_inicio=posiciones_menus[dibujar_cursor_topbar_pos_cursor];

        int x_final=posiciones_menus[dibujar_cursor_topbar_pos_cursor+1]-1;


        //Escribir ese nombre de menu seleccionado en color inverso o lo que corresponda según el estilo actual de GUI
        for (;x_inicio<=x_final && topbar_string_linea_menus[x_inicio]!=' ';x_inicio++) {
            putchar_menu_overlay_parpadeo(x_inicio,0,topbar_string_linea_menus[x_inicio],ESTILO_GUI_TINTA_SELECCIONADO,ESTILO_GUI_PAPEL_SELECCIONADO,0);
            //menu_escribe_texto(x_inicio,0,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,topbar_string_linea_menus[x_inicio]);
        }
    }
    */
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



void menu_topbarmenu_preexit(void)
{
    dibujar_cursor_topbar=0;
    topbar_overlay_we_are_on_topbar=0;
    topbar_menu_desplegado=0;
    topbar_make_topbar_invisible();
    salir_todos_menus=1;
}

int menu_topbarmenu_crear_indice_posiciones(void)
{
    posiciones_menus[0]=0;

    int i,total_posiciones;
    int leido_espacio=0;
    char *topbar_string_linea_menus=menu_topbar_get_text_topbar();
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

    return total_posiciones;
}

void menu_topbarmenu(void)
{
    printf("Entramos en topbar menu. mouse_left: %d menu_topbarmenu_pressed_bar: %d\n",mouse_left,menu_topbarmenu_pressed_bar);
    printf("Entramos en topbar menu. zxvision_keys_event_not_send_to_machine: %d menu_abierto: %d\n",zxvision_keys_event_not_send_to_machine,menu_abierto);

    topbar_overlay_we_are_on_topbar=1;
    topbarmenu_mostrar_hotkeys_por_timer=0;

    //Esto es necesario al entrar pulsando boton izquierdo raton en el fondo
    //TODO: porque por alguna razón, al entrar con boton izquierdo no se cambia
    //aunque cuando se entra pulsando por ejemplo F5 o si se pulsa boton izquierdo en el top bar si que se cambia
    zxvision_keys_event_not_send_to_machine=1;

    zxvision_redraw_all_windows();


    int total_posiciones=menu_topbarmenu_crear_indice_posiciones();
    /*posiciones_menus[0]=0;

    int i,total_posiciones;
    int leido_espacio=0;
    char *topbar_string_linea_menus=menu_topbar_get_text_topbar();
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
    */

    int i;

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

    int estabamos_en_smartload=0;

    //Si hemos salido de menus con flecha izquierda o derecha, desplegar el siguiente menu, pero,
    //si el siguiente es smartload, se desplega una ventana y dejamos de navegar
    //por tanto en ese caso, si es smartload, no desplegaremos menu sino cursor arriba
    if ((ultimo_menu_salido_con_flecha_izquierda || ultimo_menu_salido_con_flecha_derecha) && dibujar_cursor_topbar_pos_cursor==1) {
        ultimo_menu_salido_con_flecha_izquierda=ultimo_menu_salido_con_flecha_derecha=0;
        estabamos_en_smartload=1;
    }

    int last_pos_x_mouse_columna=-1;
    int last_pos_y_mouse_fila=-1;

    do {

        dibujar_cursor_topbar=1;

        //Este bucle controla el movimiento del cursor sobre los menus superiores sin abrir ningún menú
        //Esperar tecla/raton, y siempre que no se haya entrado abriendo el menu pulsando ya en barra superior
        //o se haya pulsado flecha izquierda o derecha en menus abiertos
        //Moverse por los titulos superiores
        int salir_linea_superior=0;
        while (!salir_linea_superior && !menu_topbarmenu_pressed_bar && !ultimo_menu_salido_con_flecha_izquierda && !ultimo_menu_salido_con_flecha_derecha) {

            //dibujar_cursor_topbar=1;

            printf("cursor_pos %d total_menus %d\n",dibujar_cursor_topbar_pos_cursor,total_menus);

            //menu_refresca_pantalla();
            //scr_refresca_pantalla();
            //zxvision_redraw_all_windows();
            if (!menu_multitarea) menu_refresca_pantalla();

            tecla_leida=menu_topbarmenu_get_key();

            printf("tecla leida: %d\n",tecla_leida);

            //Si se ha pulsado tecla F5 estando aqui, se quedaria esta variable activada que
            //provocaria que para salir del menu hubiera que pulsar ESC dos veces
            menu_pressed_open_menu_while_in_menu.v=0;

            //si se ha movido el raton por la parte superior
            if (tecla_leida==0) {
                if (get_pos_y_mouse_topbar()==0)  {
                    int columna_actual=get_pos_x_mouse_topbar();
                    if (columna_actual!=last_pos_x_mouse_columna) {
                        printf("Raton movido de columna en topbar\n");

                        //Detectar que menu esta seleccionando
                        int i;
                        for (i=0;i<total_posiciones;i++) {
                            if (columna_actual<posiciones_menus[i]) break;
                        }

                        if (i<total_posiciones) {
                            dibujar_cursor_topbar_pos_cursor=i-1;
                        }

                    }
                }
            }

            last_pos_x_mouse_columna=get_pos_x_mouse_topbar();
            last_pos_y_mouse_fila=get_pos_y_mouse_topbar();

            //si boton derecho, salir
            if (mouse_right) {
                menu_topbarmenu_preexit();
                return;
            }

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
                        if (estabamos_en_smartload) {
                            printf("Estabamos en smartload. Desplegar menu\n");
                            salir_linea_superior=1;
                            tecla_leida=13;
                        }
                    break;

                    case 9:
                        printf("derecha\n");
                        menu_topbarmenu_flecha_derecha(total_menus);
                        if (estabamos_en_smartload) {
                            printf("Estabamos en smartload. Desplegar menu\n");
                            salir_linea_superior=1;
                            tecla_leida=13;
                        }
                    break;

                    case 2:
                        //ESC
                        menu_topbarmenu_preexit();
                        //salir_todos_menus=1;
                        printf("Salir del menu con ESC. menu_event_open_menu.v=%d menu_abierto=%d\n",menu_event_open_menu.v,menu_abierto);
                        //menu_pressed_open_menu_while_in_menu.v=0;
                        return;
                    break;
                }
            }

        }


        //dibujar_cursor_topbar=0;

        //Aqui se gestiona la apertura de un menu
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

            printf("--Antes menu_topbarmenu_pressed_bar %d\n",menu_topbarmenu_pressed_bar);
            printf("0 antes abrir menu audio menu_pressed_open_menu_while_in_menu.v=%d\n",menu_pressed_open_menu_while_in_menu.v);

            //Entrado por raton
            if ((tecla_leida==13 && mouse_left) || menu_topbarmenu_pressed_bar) {
                menu_topbarmenu_pressed_bar=0;
                columna_posicion_x=get_pos_x_mouse_topbar();
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

            printf("--Despues menu_topbarmenu_pressed_bar %d\n",menu_topbarmenu_pressed_bar);
            printf("1 antes abrir menu audio menu_pressed_open_menu_while_in_menu.v=%d\n",menu_pressed_open_menu_while_in_menu.v);

            if (posicion_y==0) {
                menu_espera_no_tecla_con_repeticion();

                zxvision_enable_next_menu_position();
                force_next_menu_position_y=1;
            }

            printf("2 antes abrir menu audio menu_pressed_open_menu_while_in_menu.v=%d\n",menu_pressed_open_menu_while_in_menu.v);

            //int posicion_y=mouse_y/menu_char_height/menu_gui_zoom/zoom_y;

            //printf("posicion x: %d\n",columna_posicion_x);

            printf("posicion cursor: %d\n",pos_cursor);


            //asumimos que saldremos del topbar
            salir_topbar=1;

            //desde el menu_espera_no_tecla_con_repeticion se puede activar menu_pressed_open_menu_while_in_menu.v
            //si se ha pulsado F5 dos veces, desplegado un menu con raton y el raton esta en el topbar
            //Cosa que provoca un efecto que está detallado mas abajo en el TODO de Si se pulsa F5 dos veces...

            //Usado para detectar salida del menu principal y parar un full search index
            int detectar_salir_menu_principal=0;

            if (posicion_y==0 && pos_cursor>=0) {

                topbar_menu_desplegado=1;

                //actualizar posicion de cursor global con lo calculado segun el raton
                dibujar_cursor_topbar_pos_cursor=pos_cursor;

                force_next_menu_position_x=posiciones_menus[pos_cursor];

                //hemos pulsado en topbar, nos mantenemos
                salir_topbar=0;

                switch(pos_cursor) {
                    case 0:
                        detectar_salir_menu_principal=menu_inicio_mostrar_main_menu(detectar_salir_menu_principal);
                    break;

                    case 1:
                        zxvision_helper_menu_shortcut_print('s');
                        menu_smartload(0);
                    break;

                    case 2:
                        zxvision_helper_menu_shortcut_print('n');
                        menu_snapshot(0);
                    break;

                    case 3:
                        zxvision_helper_menu_shortcut_print('m');
                        menu_machine_selection(0);
                    break;

                    case 4:
                        zxvision_helper_menu_shortcut_print('a');
                        menu_audio(0);
                    break;

                    case 5:
                        zxvision_helper_menu_shortcut_print('d');
                        menu_display_settings(0);
                    break;

                    case 6:
                        zxvision_helper_menu_shortcut_print('t');
                        menu_storage(0);
                    break;

                    case 7:
                        zxvision_helper_menu_shortcut_print('e');
                        menu_debug_main(0);
                    break;

                    case 8:
                        zxvision_helper_menu_shortcut_print('o');
                        menu_network(0);
                    break;

                    case 9:
                        zxvision_helper_menu_shortcut_print('w');
                        menu_windows(0);
                    break;

                    case 10:
                        zxvision_helper_menu_shortcut_print('i');
                        menu_settings(0);
                    break;

                    case 11:
                        zxvision_helper_menu_shortcut_print('l');
                        menu_help(0);
                    break;
                }


                printf("despues switch. if_menu_topbarmenu_pressed_bar= %d mouse_left= %d\n",
                    if_menu_topbarmenu_pressed_bar(),mouse_left);

                //Para parar el reindexado
                if (detectar_salir_menu_principal) {
                    //printf("PARAMOS REINDEXADO\n");
                    menu_dibuja_menu_recorrer_menus=0;
                }

                //Necesario para cerrar submenus, por ejemplo si estamos en un item de menu con submenus,
                //y simplemente pulsamos fuera del menu, con lo que se simula pulsado ESC
                //pero deja submenus abiertos
                menu_dibuja_submenu_cierra_todos_submenus();


                if (ultimo_menu_salido_con_flecha_izquierda || ultimo_menu_salido_con_flecha_derecha) {
                    printf("Ultimo menu pulsado izquierda o derecha. menu_topbarmenu_pressed_bar=%d\n",menu_topbarmenu_pressed_bar);
                }


                //TODO: Creo que aqui no se llega porque al salir de los menus se espera a liberar teclas y raton
                //y por tanto mouse_left no estará pulsado
                if (if_menu_topbarmenu_pressed_bar() && mouse_left)  {
                    //continuamos aqui y sin tener que esperar tecla
                    printf("Reentraremos en menu\n");
                    menu_topbarmenu_pressed_bar=1;
                }

                //TODO
                //Si se pulsa F5 dos veces, abrimos menu Audio (o cualquier otro menu) con raton, pulsar ESC y se volvera a entrar al topbar, sin cerrar todo,
                //porque menu_pressed_open_menu_while_in_menu.v estaba activado
                //Pero por otra parte, ese comportamiento es el mismo que si abrimos menu audio , se despliega el menu,
                //y luego pulso en otro menu, como en debug por ejemplo
                //Se activa salir_todos_menus y abrimos menu Audio tenemos que dejar que se reentre al menu
                //En resumen: pulsar F5 dos veces y abrir menu con raton provoca que se tenga que pulsar ESC dos veces,
                //no puedo activar menu_pressed_open_menu_while_in_menu.v=0 o dejaria de poderse cambiar a otro menu
                //Esto sucede en parte por la gestión que se tiene de la tecla F5 o de la pulsación de raton para abrir menu,
                //que ambos hacen generalmente la misma acción y cuesta distinguirlos
                //En resumen, que es una acción tan remota que no vale la pena liarme para corregir esto, porque además
                //supondría tener que cambiar varias cosas del core de zxvision de apertura de menu
                //Y realmente el efecto secundario no es tan catastrofico, se puede vivir con él
                //if (salir_todos_menus) {
                    //menu_pressed_open_menu_while_in_menu.v=0;
                //}

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

    menu_topbarmenu_preexit();
}

int topbar_text_overlay_llamado_a_crear_indice_posiciones=0;

void topbar_text_overlay(void)
{
    //Dibujar topbar
    if (zxvision_topbar_menu_enabled.v) {
        int mostrar_topbar=0;


        if (topbar_overlay_we_are_on_topbar) mostrar_topbar=1;

        /*
        if (overlay_visible_when_menu_closed) {
            //si menu cerrado pero se ha movido raton
            if (topbar_esta_visible_por_timer.v) mostrar_topbar=1;
        }
        */

        if (topbar_esta_visible_por_timer.v) mostrar_topbar=1;

        if (mostrar_topbar) {
            if (!topbar_text_overlay_llamado_a_crear_indice_posiciones) {
                printf("----Creamos indice posiciones\n");
                menu_topbarmenu_crear_indice_posiciones();
                topbar_text_overlay_llamado_a_crear_indice_posiciones=1;
            }
            menu_topbarmenu_write_bar();
        }
    }
}