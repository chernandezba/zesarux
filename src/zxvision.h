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

#ifndef ZXVISION_H
#define ZXVISION_H

#include <dirent.h>
#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "cpu.h"


//Por el tema de usar PATH_MAX en windows
#ifdef MINGW
#include <stdlib.h>
#define PATH_MAX MAX_PATH
#define NAME_MAX MAX_PATH
#endif

/*
 ZXVISION_USE_CACHE_OVERLAY_TEXT : Activar cache de overlay_screen_array. Si no se ha modificado una posicion concreta en overlay_screen_array,
 no se reescribe en pantalla desde normal_overlay_texto_menu
 Se ha validado que esto sea compatible con el mezclado de capas de menu:
 de renderizado maquina: escribe en buffer_layer_machine, acaba llamando a screen_putpixel_mix_layers. y este a scr_putpixel_final_rgb
 de menu, scr_putchar_menu_comun_zoom llama a scr_putpixel_gui_zoom,  escribe en buffer_layer_menu, y acaba llamando a screen_putpixel_mix_layers
 Se implementó a partir de versión 10.1-RC1
 Se puede desactivar simplemente comentando aquí esta definición

Pruebas de uso cpu:

make -j 3 && ./zesarux  --noconfigfile --ao null --enable-zxdesktop --zxdesktop-width 600 --disablebetawarning 10.1-RC1 --disable-autoframeskip


Abri help->readme y maximizarlo para que ocupe pantalla completa

Usos cpu:

-Compilado con  O3
-con ZXVISION_USE_CACHE_OVERLAY_TEXT: 15%
-sin ZXVISION_USE_CACHE_OVERLAY_TEXT: 87%

-Compilado Sin O3
-con ZXVISION_USE_CACHE_OVERLAY_TEXT: 20%
-sin ZXVISION_USE_CACHE_OVERLAY_TEXT: 92%



de renderizado maquina: escribe en buffer_layer_machine, acaba llamando a screen_putpixel_mix_layers. y este a scr_putpixel_final_rgb
de menu, scr_putchar_menu_comun_zoom llama a scr_putpixel_gui_zoom,  escribe en buffer_layer_menu, y acaba llamando a screen_putpixel_mix_layers

*/

#define ZXVISION_USE_CACHE_OVERLAY_TEXT

struct s_overlay_screen {
	int tinta,papel,parpadeo;
	z80_byte caracter;
//#ifdef ZXVISION_USE_CACHE_OVERLAY_TEXT
    int modificado; //Indica si se ha modificado el caracter y por tanto hay que redibujarlo desde normal_overlay_texto_menu
//#endif
};

//Para poder debugar los caracteres de overlay que no pasan por la cache, estos se incrementan en cada redibujado completo
//los que no se incrementan, es que estan en cache
//Esto normalmente deberia estar comentado
//#define DEBUG_ZXVISION_USE_CACHE_OVERLAY_TEXT

typedef struct s_overlay_screen overlay_screen;


extern int stats_normal_overlay_menu_total_chars;
extern int stats_normal_overlay_menu_drawn_chars;

//Idiomas de la interfaz
#define GUI_LANGUAGE_DEFAULT 0
#define GUI_LANGUAGE_SPANISH 1
#define GUI_LANGUAGE_CATALAN 2

//tecla F1
#define MENU_TECLA_AYUDA 21

//Para el guardado de la geometria de ventanas

//Maximo de ventanas que se guarda geometria
#define MAX_CONFIG_WINDOW_GEOMETRY 100
//Tamaño maximo del nombre para la geometria
#define MAX_NAME_WINDOW_GEOMETRY 100

#define ZXVISION_MAX_WINDOW_TITLE 256

//Nuevas ventanas zxvision
struct s_zxvision_window {
	overlay_screen *memory;
	int visible_width,visible_height;
	int x,y;

    //identificador de proceso. TODO: siempre se incrementa, hay que detectar que no use pids ya existentes
    unsigned int pid;

    //las primeras posiciones especificadas aqui seran de upper margin
	int upper_margin;
    //las siguientes seran de lower margin
	int lower_margin;

	//char *text_margin[20]; //Hasta 20 lineas de texto que se usan como texto que no se mueve. La ultima finaliza con 0

	int offset_x,offset_y;

	int total_width,total_height;
	char window_title[ZXVISION_MAX_WINDOW_TITLE];

	//Para el guardado de geometria
	char geometry_name[MAX_NAME_WINDOW_GEOMETRY];

	int can_be_resized;

    int can_be_minimized;

    //Si el contenido se recrea al aumentar tamaño ventana
    int contents_can_be_enlarged;
	int is_minimized;
	int is_maximized;

    //papel usado por ejemplo al hacer zxvision_cls
    //aunque no se usa en casi ninguna funcion mas, por ejemplo, zxvision_print_string_defaults no lo usa
    //esta pensado para cuando se quiere poner un fondo de color concreto y mayormente llenar la pantalla
    //de pixeles, como en Visual Floppy
    int default_paper;

	//Si boton de background aparece en ventana. Nota: en principio F6 funciona aunque esto no se establezca
	int can_be_backgrounded;

	//Si se pueden enviar hotkeys desde raton, pulsando en letras inversas
	int can_mouse_send_hotkeys;

    //No refrescar contenido ventana cuando se cambia offset. Usado por ejemplo en Text Adventure Map
    int no_refresh_change_offset;

	int height_before_max_min_imize;
	int width_before_max_min_imize;
	int x_before_max_min_imize;
	int y_before_max_min_imize;

    //zona de coloreado cuando se va de limite de tamanyo
    int beyond_x,beyond_y,beyond_width,beyond_height,beyond_color;

	int can_use_all_width; //Si tenemos usable también la ultima columna derecha

    //Si se debe forzar que al llamar a draw_window_contents, se redibuje contenido vaciando la cache
    //especialmente usado en ventanas con contenido de pixel (como waveform por ejemplo) que se necesita
    //que al escribir el texto con espacios, esos espacios borren la cache de putchar cada vez
    int must_clear_cache_on_draw;

    //Lo mismo que el setting must_clear_cache_on_draw pero se pondra a 0 al llamar a draw_window_contents
    //usado por ejemplo en Daad/paws/gac graphics
    int must_clear_cache_on_draw_once;

    //indica a zxvision_draw_window_contents que tiene que redibujar esa ventana por haberse redimensionado, movido, o alterado otras ventanas
    int dirty_must_draw_contents;

    //se activa en mismas situaciones que dirty_must_draw_contents, pero no se resetea a 0 nunca
    //esto es para uso de usuario, para que sepa que una ventana se ha redimensionado, etc, esto es util
    //para ventanas con overlay de pixel, en que habitualmente no dibujan todos los pixeles, solo los cambios,
    //pero en caso que dirty_user_must_draw_contents, ese frame lo tienen que dibujar con todos los pixeles, y
    //luego corresponde al usuario ponerlo a 0
    //Ver uso en juego zxlife o en menu About
    int dirty_user_must_draw_contents;

    //indica a algunas ventanas (como Keyboard Help) que se ha lanzado zxvision_draw_window_contents y redibujado el fondo de texto
    int has_been_drawn_contents;

    //indica que la ventana siempre debe mostrarse por encima de las demas
    int always_visible;

    //indica que no se le altera tamaño o posición por acciones del menu Windows (minimize all, cascade, etc)
    //usado en process switcher por ejemplo
    int not_altered_by_massive_changes;

    //indica que se ha intentado escribir mas alla del tamanyo de la ventana
    int tried_write_beyond_size;

    //indica que esta ventana no se avisara de que se ha intentado escribir mas alla del tamanyo de la ventana
    //por ejemplo ventana Filesel cuando hace scroll de campo de carpeta actual, intenta escribir mas alla
    //y no quiero que indique
    int do_not_warn_tried_write_beyond_size;

	//Posicion del cursor y si esta visible
	int visible_cursor;
	int cursor_line;
    //Acortar el cursor en ancho en X caracteres a ambos lados
    int acortar_cursor;

    //tiempo total transcurrido, en microsegundos, dibujando overlay
    long last_spent_time_overlay;

	//Ventana anterior. Se van poniendo una encima de otra
	struct s_zxvision_window *previous_window;

	//Ventana siguiente.
	struct s_zxvision_window *next_window;


	//Puntero a funcion de overlay
	void (*overlay_function) (void);

    //Lo siguiente solo usado por los submenus
    struct s_zxvision_window *submenu_next;
    struct s_zxvision_window *submenu_previous;
    //linea seleccionada con enter
    int submenu_linea_seleccionada;
};

typedef struct s_zxvision_window zxvision_window;



//extern long zxvision_time_total_drawing_overlay;
extern long zxvision_time_total_drawing_overlay_except_current;
extern long normal_overlay_time_total_drawing_overlay;

//Aqui hay un problema, y es que en utils.h se esta usando zxvision_window, y hay que declarar este tipo de ventana antes
#include "utils.h"

//Valor para ninguna tecla pulsada
//Tener en cuenta que en spectrum y zx80/81 se usan solo 5 bits pero en Z88 se usan 8 bits
//en casos de spectrum y zx80/81 se agregan los 3 bits faltantes
#define MENU_PUERTO_TECLADO_NINGUNA 255

#define MENU_ITEM_PARAMETERS int valor_opcion GCC_UNUSED

//Total de paletas mapeadas
//Usado en ver sprites y ver colores mapeados
#define MENU_TOTAL_MAPPED_PALETTES 20

#define MENU_LAST_DIR_FILE_NAME "zesarux_last_dir.txt"
#define MENU_SCR_INFO_FILE_NAME "zesarux_scr_info.txt"

extern int menu_overlay_activo;
extern void (*menu_overlay_function)(void);

extern int overlay_visible_when_menu_closed;

extern char esc_key_message[];
extern char *openmenu_key_message;

extern z80_bit menu_desactivado;
extern z80_bit menu_desactivado_andexit;
extern z80_bit menu_desactivado_file_utilities;

extern int zxvision_switch_to_window_on_open_menu;
extern char zxvision_switch_to_window_on_open_menu_name[];
extern void zxvision_open_menu_with_window(char *geometry_name);
extern void zxvision_open_window_by_name(char *nombre);

extern void set_menu_overlay_function(void (*funcion)(void) );
extern void reset_menu_overlay_function(void);
extern void pruebas_texto_menu(void);
extern void cls_menu_overlay(void);
extern void menu_cls_refresh_emulated_screen();
extern void menu_escribe_texto(int x,int y,int tinta,int papel,char *texto);
extern void normal_overlay_texto_menu(void);
extern int si_menu_mouse_en_ventana(void);
extern int si_menu_mouse_en_ventana_no_en_scrolls(void);
extern void menu_calculate_mouse_xy(void);
extern z80_byte menu_retorna_caracter_espacio_titulo(void);

extern int mouse_is_clicking;
extern int mouse_is_double_clicking;
extern int menu_mouse_x;
extern int menu_mouse_y;

extern int forzar_no_mostrar_caracteres_extendidos;

extern unsigned char menu_escribe_texto_convert_utf(unsigned char prefijo_utf,unsigned char caracter);
extern int menu_es_prefijo_utf(z80_byte caracter);

extern void menu_ventana_draw_vertical_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido);
extern void menu_ventana_draw_horizontal_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido);

extern void menu_espera_tecla(void);
extern void menu_espera_no_tecla_con_repeticion(void);
extern void menu_espera_no_tecla_no_cpu_loop(void);
extern void menu_espera_tecla_no_cpu_loop(void);
extern int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea);
extern void menu_espera_tecla_timeout_window_splash(void);
extern void menu_espera_tecla_o_wheel(void);

extern int f_functions;
extern char *string_esc_closes_menus;

#define MENU_CPU_CORE_LOOP_SLEEP_NO_MULTITASK 500

extern void menu_cpu_core_loop(void);
extern z80_byte menu_get_pressed_key(void);
extern void menu_about_read_file(char *title,char *aboutfile,int show_err_if_big);

extern void menu_about_about_load_editionamegame(void);

//extern int menu_cond_zx8081(void);
//extern int menu_cond_zx8081_realvideo(void);
//extern int menu_cond_zx8081_no_realvideo(void);
//extern int menu_cond_realvideo(void);
//extern int menu_display_rainbow_cond(void);
//extern int menu_cond_stdout(void);
//extern int menu_cond_simpletext(void);
//extern int menu_cond_curses(void);

//extern int zxvision_drawing_in_background;


extern void menu_interface_rgb_inverse_common(void);

extern z80_bit menu_writing_inverse_color;
extern int menu_dibuja_menu_permite_repeticiones_hotk;

extern z80_bit menu_symshift;
extern z80_bit menu_capshift;
extern z80_bit menu_backspace;
extern z80_bit menu_tab;


extern int menu_footer;

extern void enable_footer(void);
extern void disable_footer(void);
extern void switch_footer(void);
extern void menu_init_footer(void);
extern void menu_footer_z88(void);
extern void menu_clear_footer(void);

extern int mouse_is_dragging;
extern int menu_mouse_left_double_click_counter;

extern z80_bit christmas_mode;
extern z80_bit avoid_christmas_mode;





#define OVERLAY_SCREEN_MAX_WIDTH 256
#define OVERLAY_SCREEN_MAX_HEIGTH 128

//Tamanyo inicial maximo. Aunque luego se puede hacer mas grande
/*
#define ZXVISION_MAX_ANCHO_VENTANA 32
#define ZXVISION_MAX_ALTO_VENTANA 24


#define ZXVISION_MAX_X_VENTANA (scr_get_menu_width()-1)
#define ZXVISION_MAX_Y_VENTANA (scr_get_menu_height()-1)
*/


#define ZXVISION_MAX_ANCHO_VENTANA (scr_get_menu_width())
#define ZXVISION_MAX_ALTO_VENTANA (scr_get_menu_height())


#define ZXVISION_MAX_X_VENTANA (ZXVISION_MAX_ANCHO_VENTANA-1)
#define ZXVISION_MAX_Y_VENTANA (ZXVISION_MAX_ALTO_VENTANA-1)




extern void zxvision_new_window(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title);
//extern void legacy_zxvision_new_window_gn_cim(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title,char *geometry_name,int is_minimized,int width_before_max_min_imize,int height_before_max_min_imize);
extern void zxvision_new_window_gn_cim(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title,char *geometry_name,int is_minimized,int is_maximized,int width_before_max_min_imize,int height_before_max_min_imize);
extern void zxvision_new_window_nocheck_staticsize(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title);
extern void zxvision_cls(zxvision_window *w);
extern void zxvision_destroy_window(zxvision_window *w);
extern void zxvision_draw_window(zxvision_window *w);
extern void zxvision_set_attr(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo);
extern void zxvision_print_char(zxvision_window *w,int x,int y,overlay_screen *caracter);
extern void zxvision_print_char_simple(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,z80_byte caracter);
extern void zxvision_draw_window_contents(zxvision_window *w);
extern void zxvision_draw_window_contents_no_speech(zxvision_window *ventana);
extern int zxvision_wait_until_esc(zxvision_window *w);

extern int zxvision_draw_overlay_if_exists(zxvision_window *w);
//extern void menu_draw_background_windows_overlay(void);

extern void zxvision_window_move_this_window_on_top(zxvision_window *ventana);
extern void zxvision_window_move_this_window_to_bottom(zxvision_window *ventana);
extern void zxvision_activate_this_window(zxvision_window *ventana);
extern int zxvision_if_window_already_exists(zxvision_window *w);
extern void zxvision_window_delete_this_window(zxvision_window *ventana);
extern zxvision_window *zxvision_return_n_window_from_top(int indice);
extern void zxvision_redraw_all_windows(void);
extern void zxvision_delete_window_if_exists(zxvision_window *ventana);
extern int zxvision_window_can_be_backgrounded(zxvision_window *w);
extern void zxvision_message_put_window_background(void);
extern void zxvision_window_delete_all_windows(void);
extern void zxvision_window_delete_all_windows_and_clear_geometry(void);
extern int zxvision_scanf(zxvision_window *ventana,char *string,unsigned int max_length,int max_length_shown,int x,int y,int volver_si_fuera_foco,int volver_si_flecha_abajo,int forzar_cursor);
extern int zxvision_scanf_history(char *titulo,char *texto,int max_length,char **textos_historial);
//extern void zxvision_scanf_history_insert(char **textos_historial,char *texto);

//Contando el NULL del final
//#define ZXVISION_SCANF_HISTORY_MAX_LINES 11

extern int zxvision_clicked_mouse_button(void);

extern zxvision_window *zxvision_coords_in_below_windows(zxvision_window *w,int x,int y);
extern zxvision_window *zxvision_find_window_in_background(char *geometry_name);

extern int clicked_on_background_windows;

extern zxvision_window *which_window_clicked_on_background;

extern void zxvision_set_window_overlay_from_current(zxvision_window *ventana);
extern void zxvision_set_window_overlay(zxvision_window *ventana,void (*overlay_function) (void));
extern void zxvision_reset_window_overlay(zxvision_window *ventana);

extern zxvision_window *zxvision_find_first_window_below_this(zxvision_window *w);

//extern int tecla_f_background;
//extern int menu_get_mask_puerto_especial(int tecla_f);
//extern z80_byte *menu_get_port_puerto_especial(int tecla_f);

#define MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH 128

extern void menu_escribe_linea_opcion_zxvision(zxvision_window *ventana,int indice,int opcion_actual,int opcion_activada,char *texto_entrada,
    int tiene_submenu,int opcion_marcada,int genera_ventana);

extern void zxvision_set_ventana_tipo_activa(void);
extern void zxvision_reset_ventana_tipo_activa(void);

extern int zxvision_pressed_right_mouse_button(void);

extern void zxvision_set_next_menu_position_from_current_mouse(void);
extern void zxvision_reset_set_next_menu_position(void);
extern void zxvision_set_offset_x(zxvision_window *w,int offset_x);
extern void zxvision_set_offset_y(zxvision_window *w,int offset_y);
extern int zxvision_maximum_offset_x(zxvision_window *w);
extern int zxvision_maximum_offset_y(zxvision_window *w);
extern void zxvision_set_offset_y_visible(zxvision_window *w,int y);
extern void zxvision_set_offset_y_or_maximum(zxvision_window *w,int offset_y);
extern void zxvision_set_x_position(zxvision_window *w,int x);
extern void zxvision_set_y_position(zxvision_window *w,int y);
extern void zxvision_set_visible_width(zxvision_window *w,int visible_width);
extern void zxvision_set_visible_height(zxvision_window *w,int visible_height);
extern void zxvision_set_total_height(zxvision_window *w,int total_height);

extern void zxvision_print_string(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,char *texto);
extern void zxvision_print_string_format (zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo, const char * format , ...);
extern void zxvision_print_string_defaults(zxvision_window *w,int x,int y,char *texto);
extern void zxvision_print_string_defaults_format (zxvision_window *w,int x,int y, const char * format , ...);
extern void zxvision_print_char_defaults(zxvision_window *w,int x,int y,char c);
extern void zxvision_print_string_fillspc(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,char *texto);
extern void zxvision_print_string_defaults_fillspc(zxvision_window *w,int x,int y,char *texto);
extern void zxvision_print_string_defaults_fillspc_format(zxvision_window *w,int x,int y,const char * format , ...);

extern void zxvision_handle_mouse_events(zxvision_window *w);


extern void zxvision_handle_click_minimize(zxvision_window *w);
extern void zxvision_minimize_window(zxvision_window *w);
extern void zxvision_toggle_minimize_window(zxvision_window *w);
extern void zxvision_maximize_window(zxvision_window *w);
extern void zxvision_toggle_maximize_window(zxvision_window *w);
extern void zxvision_handle_maximize(zxvision_window *w);

extern int zxvision_trocear_string_lineas(char *texto,char *buffer_lineas[]);
extern void zxvision_print_mensaje_lineas_troceado(zxvision_window *ventana,char *mensaje_entrada);

extern void zxvision_send_scroll_up(zxvision_window *w);
extern void zxvision_send_scroll_down(zxvision_window *w);
extern void zxvision_send_scroll_left(zxvision_window *w);
extern void zxvision_send_scroll_right(zxvision_window *w);

extern void zxvision_draw_below_windows(zxvision_window *w);
extern void zxvision_draw_overlays_below_windows(zxvision_window *w);
extern void zxvision_clear_window_contents(zxvision_window *w);
extern void zxvision_fill_window_transparent(zxvision_window *w);

extern void zxvision_set_not_resizable(zxvision_window *w);
extern void zxvision_set_resizable(zxvision_window *w);
extern void zxvision_set_not_minimizable(zxvision_window *w);


extern void zxvision_window_save_size(zxvision_window *ventana,int *ventana_ancho_antes,int *ventana_alto_antes);
extern int zxvision_window_get_pixel_x_position(zxvision_window *ventana);
extern int zxvision_window_get_pixel_y_position(zxvision_window *ventana);

#define MAX_LENGTH_TEXT_ICON 100

//Maximo de ventanas que se pueden restaurar
#define MAX_RESTORE_WINDOWS_START 50

extern char restore_window_array[MAX_RESTORE_WINDOWS_START][MAX_NAME_WINDOW_GEOMETRY];

extern void add_window_to_restore(char *nombre_ventana);

extern void zxvision_add_all_windows_to_restore(void);


struct s_zxvision_known_window_names {
//Ventanas conocidas y sus funciones que las inicializan. Usado al restaurar ventanas al inicio
    //nombre usado de geometria, es un nombre unico que identifica la ventana
	char nombre[MAX_NAME_WINDOW_GEOMETRY];
    //nombre usado en process switcher, algo mas descriptivo, en principio no se usa en otro sitio
    char nombre_corto[MAX_LENGTH_TEXT_ICON];
	void (*start)(MENU_ITEM_PARAMETERS);
    char **bitmap_button;
};

//#define MAX_KNOWN_WINDOWS 100
typedef struct s_zxvision_known_window_names zxvision_known_window_names;

extern zxvision_known_window_names zxvision_known_window_names_array[];

extern char **zxvision_find_icon_for_known_window(char *nombre);

extern int zxvision_known_window_is_valid_by_index(int indice);

extern int zxvision_find_known_window(char *nombre);

extern int total_restore_window_array_elements;

extern void zxvision_change_gui_style(int estilo);

extern int zxvision_currently_restoring_windows_on_start;
extern void zxvision_restore_windows_on_startup(void);
extern void zxvision_restore_one_window(char *ventana_a_restaurar);
extern void zxvision_restart_all_background_windows(void);
extern void zxvision_reapply_style_colours_all_windows(void);
extern void menu_calculate_mouse_xy_absolute_interface(int *resultado_x,int *resultado_y);
extern void menu_calculate_mouse_xy_absolute_interface_pixel(int *resultado_x,int *resultado_y);
extern void zxvision_get_mouse_in_window(zxvision_window *ventana,int *posx,int *posy);

extern void zxvision_putpixel(zxvision_window *w,int x,int y,int color);
extern void zxvision_putpixel_no_zoom(zxvision_window *w,int x,int y,int color);
extern void zxvision_draw_line(zxvision_window *w,int x1,int y1,int x2,int y2,int c, void (*fun_putpixel) (zxvision_window *w,int x,int y,int color) );
extern void zxvision_draw_ellipse(zxvision_window *w,int x1,int y1,int radius_x,int radius_y,int c, void (*fun_putpixel) (zxvision_window *w,int x,int y,int color) ,int limite_grados);
extern void zxvision_draw_arc(zxvision_window *w,int x1,int y1,int radius_x,int radius_y,int c, void (*fun_putpixel) (zxvision_window *w,int x,int y,int color) ,int inicio_grados,int limite_grados);


#define ZXVISION_TOTAL_WIDGET_TYPES 11
#define ZXVISION_WIDGET_TYPE_SPEEDOMETER 0
#define ZXVISION_WIDGET_TYPE_SPEAKER 1
#define ZXVISION_WIDGET_TYPE_CIRCLE 2
#define ZXVISION_WIDGET_TYPE_CIRCLE_CONCEN 3
#define ZXVISION_WIDGET_TYPE_ELLIPSE 4
#define ZXVISION_WIDGET_TYPE_ELLIPSE_CONCEN 5
#define ZXVISION_WIDGET_TYPE_CURVE 6
#define ZXVISION_WIDGET_TYPE_PARTICLES 7
#define ZXVISION_WIDGET_TYPE_VOLUME 8
#define ZXVISION_WIDGET_TYPE_VALUE 9
#define ZXVISION_WIDGET_TYPE_SIERPINSKY 10

extern char *zxvision_widget_types_names[];

extern void widget_list_print(void);

extern int zxvision_widget_find_name_type(char *name);

#define ZXVISION_WIDGET_TYPE_SPEEDOMETER_LINE_LENGTH 32
//#define GRAPHIC_METER_SPEEDOMETER_LINE_LENGTH 32
#define ZXVISION_WIDGET_TYPE_SPEAKER_RADIUS 16
#define ZXVISION_WIDGET_TYPE_CIRCLE_RADIUS 16
#define ZXVISION_WIDGET_TYPE_CURVE_LENGTH 32
#define ZXVISION_WIDGET_TYPE_PARTICLES_RADIUS 29
//cuando se usaba la anterior funcion 3D que usa coseno y seno
//#define ZXVISION_WIDGET_TYPE_PARTICLES_RADIUS 32

extern void zxvision_widgets_draw_speedometer_common(zxvision_window *ventana,int xorigen_linea,int yorigen_linea,int percentaje,int color_linea,int color_contorno);
extern void zxvision_widgets_draw_metter_common_by_shortname(zxvision_window *ventana,int columna_texto,int fila_texto,char *short_name,int tipo,int valor_en_vez_de_perc,int tinta_texto_descripcion,int papel_texto_descripcion,int escribir_espacios);
extern void zxvision_widgets_erase_speedometer(zxvision_window *ventana,int xcentro_widget,int ycentro_widget);
extern void zxvision_draw_filled_rectangle(zxvision_window *ventana,int xinicio,int yinicio,int ancho,int alto,int color);
extern void zxvision_draw_rectangle(zxvision_window *ventana,int x1,int y1,int ancho,int alto,int color);
extern void zxvision_draw_rectangle_function(zxvision_window *ventana,int x1,int y1,int ancho,int alto,int color,
    void (*fun_putpixel) (zxvision_window *w,int x,int y,int color)  );
extern void zxvision_widgets_draw_particles_3d_convert(int x,int y,int z,int *xfinal,int *yfinal);

extern z80_byte zxvision_read_keyboard(void);
void zxvision_handle_cursors_pgupdn(zxvision_window *ventana,z80_byte tecla);
extern z80_byte zxvision_common_getkey_refresh(void);
extern z80_byte zxvision_common_getkey_refresh_noesperatecla(void);
extern z80_byte zxvision_common_getkey_refresh_noesperanotec(void);
extern z80_byte zxvision_common_getkey_wheel_refresh_noesperanotec(void);
extern z80_byte zxvision_common_getkey_refresh_noesperanotec_todasteclas(void);

extern zxvision_window *zxvision_current_window;

extern int zxvision_keys_event_not_send_to_machine;
//extern void zxvision_espera_tecla_timeout_window_splash(void);
extern void zxvision_espera_tecla_timeout_window_splash(int tipo);
extern int zxvision_key_not_sent_emulated_mach(void);
//extern void menu_linea_zxvision(zxvision_window *ventana,int x,int y1,int y2,int color);
extern void zxvision_fill_width_spaces(zxvision_window *w,int y);
extern void zxvision_fill_width_spaces_paper(zxvision_window *w,int y,int papel);
extern void zxvision_fill_width_spaces_paper_width(zxvision_window *w,int y,int papel,int start_x,int after_end_x);

struct s_first_aid_list
{
	//enum first_aid_number_list indice_setting; //numero
	char config_name[100]; //nombre en la config
	int *puntero_setting;
	char *texto_opcion;
	int si_startup; //Si mensaje puede aparecer en startup del emulador
};

#define MAX_MENU_FIRST_AID 100

extern struct s_first_aid_list first_aid_list[];

extern int menu_first_aid_startup;
extern z80_bit menu_disable_first_aid;
extern void menu_first_aid_disable(char *texto);
extern int total_first_aid;
extern int menu_first_aid(char *key_setting);
extern void menu_first_aid_restore_all(void);
extern void menu_first_aid_init(void);
extern void menu_first_aid_random_startup(void);
extern int menu_first_aid_title(char *key_setting,char *title);

#define MAX_F_FUNCTIONS 63

enum defined_f_function_ids {
	//reset, hard-reset, nmi, open menu, ocr, smartload, osd keyboard, exitemulator.
	F_FUNCION_DEFAULT=100,    //Empiezo a 100 por asegurarme de un cambio de codigo que todo vaya bien, temporalmente, en version 10.0. Luego se puede quitar
	F_FUNCION_NOTHING,
	F_FUNCION_RESET,
	F_FUNCION_HARDRESET,
	F_FUNCION_NMI,  //5
	F_FUNCION_OPENMENU,
	F_FUNCION_OCR,
	F_FUNCION_SMARTLOAD,
	F_FUNCION_QUICKLOAD,
	F_FUNCION_QUICKSAVE,
    F_FUNCION_QUICKSAVE_SCREEN,
    F_FUNCION_REWIND,
    F_FUNCION_FFW,
	F_FUNCION_LOADBINARY,
	F_FUNCION_SAVEBINARY,
    F_FUNCION_SETTINGS,
    F_FUNCION_WAVEFORM,
    F_FUNCION_AUDIO_REGISTERS,
    F_FUNCION_AUDIO_SHEET,
    F_FUNCION_AUDIO_PIANO,
    F_FUNCION_WAVE_PIANO,
    F_FUNCION_AYMIXER,
    F_FUNCION_AYPLAYER,
    F_FUNCION_COLOUR_PALETTES,
    F_FUNCION_HEX_EDITOR,
    F_FUNCION_VIEW_SPRITES,
    F_FUNCION_FILE_UTILITIES,
    F_FUNCION_ONLINE_SPECCY,
    F_FUNCION_ONLINE_ZX81,
	F_FUNCION_ZENG_SENDMESSAGE,
    F_FUNCION_ZENG_ONLINE_SENDMESSAGE,
	F_FUNCION_OSDKEYBOARD,
	F_FUNCION_OSDTEXTKEYBOARD,
    F_FUNCION_SWITCHBORDER,
	F_FUNCION_SWITCHFULLSCREEN,
    F_FUNCION_SWITCHFOOTER,
	F_FUNCION_RELOADMMC,
	F_FUNCION_REINSERTSTDTAPE,
	F_FUNCION_PAUSEUNPAUSEREALTAPE,
    F_FUNCION_REINSERTREALTAPE,
    F_FUNCION_REWINDREALTAPE,
    F_FUNCION_FFWDREALTAPE,
    F_FUNCION_VISUALREALTAPE,
    F_FUNCION_DEBUGCPU,
	F_FUNCION_PAUSE,
	F_FUNCION_TOPSPEED,
 	F_FUNCION_EXITEMULATOR,
	F_FUNCION_BACKGROUND_WINDOW,
	F_FUNCION_OVERLAY_WINDOWS,
    F_FUNCION_CLOSE_ALL_MENUS,
    F_FUNCION_ZXUNO_PRISM,
    F_FUNCION_LEFTRIGHT_JOY,
    F_FUNCION_DEBUGCPU_VIEW_ADVENTURE,
    F_FUNCION_TEXT_ADVENTURE_MAP,
    F_FUNCION_DESKTOP_TRASH,
    F_FUNCION_DESKTOP_SNAPSHOT,
    F_FUNCION_DESKTOP_TAPE,
    F_FUNCION_DESKTOP_GENERIC_SMARTLOAD,
    F_FUNCION_MACHINE_SELECTION,
    F_FUNCION_SET_MACHINE,
    F_FUNCION_DESKTOP_MY_MACHINE,
    F_FUNCION_POKE,
    F_FUNCION_OPEN_WINDOW
};
//Nota: F_FUNCION_BACKGROUND_WINDOW no se llama de la misma manera que las otras funciones F
//solo esta aqui para evitar que una misma tecla F se asigne a una funcion F normal y tambien a background window

//Define teclas F que se pueden mapear a acciones y botones
#define MAX_DEFINED_F_FUNCION_NAME_LENGTH 30

struct s_defined_f_function {
	char texto_funcion[MAX_DEFINED_F_FUNCION_NAME_LENGTH];
	enum defined_f_function_ids id_funcion;

    char **bitmap_button;

    //Indica el nombre asociado a esa app/ventana (si es que es una ventana que se puede quedar en background)
    char geometry_name[MAX_NAME_WINDOW_GEOMETRY];

};

typedef struct s_defined_f_function defined_f_function;


extern defined_f_function defined_direct_functions_array[];

enum zxdesktop_custom_icon_status_ids {
    ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS,
    ZXDESKTOP_CUSTOM_ICON_EXISTS,
    ZXDESKTOP_CUSTOM_ICON_DELETED
};



//Identifica a un icono del escritorio
struct s_zxdesktop_configurable_icon {
    enum zxdesktop_custom_icon_status_ids status; //Existe, no existe, o borrado
    int pos_x,pos_y;

    //indice sobre la tabla defined_direct_functions_array
    int indice_funcion;
    //Para obtener el bitmap, se buscara en el array defined_direct_functions_array segun
    //el id_funcion

    //Nombre del icono
    char text_icon[MAX_LENGTH_TEXT_ICON];

    //Por ejemplo para guardar información de la ruta a un snapshot en la funcion de F_FUNCION_DIRECT_SNAPSHOT
    char extra_info[PATH_MAX];

};

typedef struct s_zxdesktop_configurable_icon zxdesktop_configurable_icon;

#define MAX_ZXDESKTOP_CONFIGURABLE_ICONS 100

extern zxdesktop_configurable_icon zxdesktop_configurable_icons_list[];

extern int zxdesktop_configurable_icons_current_executing;


extern char **menu_get_extdesktop_button_bitmap(int numero_boton,int *es_set_machine);
extern char **get_direct_function_icon_bitmap_final(int id_accion);

//Maximo de teclas F posibles a mapear
#define MAX_F_FUNCTIONS_KEYS 15

//Maximo de botones posibles a mapear
#define MAX_USERDEF_BUTTONS 11

//Array de teclas F mapeadas
extern int defined_f_functions_keys_array[];
extern char defined_f_functions_keys_array_parameters[MAX_F_FUNCTIONS_KEYS][PATH_MAX];
extern char defined_buttons_functions_array_parameters[MAX_USERDEF_BUTTONS][PATH_MAX];

extern int defined_buttons_functions_array[];
extern int get_defined_direct_functions(char *funcion);

extern int menu_define_key_function(int tecla,char *funcion);
extern int menu_define_key_function_extra_info(int tecla,char *extra_info);
extern int menu_define_button_function(int tecla,char *funcion);
extern void menu_inicio_handle_button_presses(void);
extern void menu_inicio_handle_lower_icon_presses(void);
extern void zxvision_helper_menu_shortcut_init(void);

extern enum defined_f_function_ids menu_da_accion_direct_functions_indice(int indice);

extern overlay_screen overlay_screen_array[];
//extern overlay_screen second_overlay_screen_array[];

//definiciones para funcion menu_generic_message
#define MAX_LINEAS_VENTANA_GENERIC_MESSAGE 20

//#define MAX_LINEAS_TOTAL_GENERIC_MESSAGE 1000

//archivo LICENSE ocupa 1519 lineas ya parseado
//archivo changelog ocupa 65184 bytes (en version 9.2)
#define MAX_LINEAS_TOTAL_GENERIC_MESSAGE 2000

#define MAX_ANCHO_LINEAS_GENERIC_MESSAGE 100
#define MAX_TEXTO_GENERIC_MESSAGE (MAX_LINEAS_TOTAL_GENERIC_MESSAGE*MAX_ANCHO_LINEAS_GENERIC_MESSAGE)

//#define NEW_MAX_ANCHO_LINEAS_GENERIC_MESSAGE 100
//#define NEW_MAX_TEXTO_GENERIC_MESSAGE (MAX_LINEAS_TOTAL_GENERIC_MESSAGE*NEW_MAX_ANCHO_LINEAS_GENERIC_MESSAGE)


extern int menu_generic_message_aux_wordwrap(char *texto,int inicio, int final);
extern void menu_generic_message_aux_copia(char *origen,char *destino, int longitud);
extern int menu_generic_message_aux_filter(char *texto,int inicio, int final);

struct s_generic_message_tooltip_return {
	char texto_seleccionado[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];
	int linea_seleccionada;
	int estado_retorno; //Retorna 1 si sale con enter. Retorna 0 si sale con ESC
};

typedef struct s_generic_message_tooltip_return generic_message_tooltip_return;

extern void zxvision_generic_message_tooltip(char *titulo, int return_after_print_text, int volver_timeout, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, int resizable, const char * texto_format , ...);
extern int zxvision_generic_message_aux_justificar_lineas(char *orig_texto,int longitud,int max_ancho_texto,char **buffer_lineas);

//Posiciones de texto mostrado en second overlay
#define WINDOW_FOOTER_ELEMENT_X_JOYSTICK 0
#define WINDOW_FOOTER_ELEMENT_X_FPS 0
#define WINDOW_FOOTER_ELEMENT_X_PRINTING 10
#define WINDOW_FOOTER_ELEMENT_X_FLASH 10
#define WINDOW_FOOTER_ELEMENT_X_MDFLP 10
#define WINDOW_FOOTER_ELEMENT_X_GENERICTEXT 10
#define WINDOW_FOOTER_ELEMENT_X_MMC 11
#define WINDOW_FOOTER_ELEMENT_X_IDE 11
#define WINDOW_FOOTER_ELEMENT_X_ESX 11
#define WINDOW_FOOTER_ELEMENT_X_DISK 11
#define WINDOW_FOOTER_ELEMENT_X_TEXT_FILTER 10
#define WINDOW_FOOTER_ELEMENT_X_ZXPAND 10
#define WINDOW_FOOTER_ELEMENT_X_TAPE 11
#define WINDOW_FOOTER_ELEMENT_X_FLAP 11
#define WINDOW_FOOTER_ELEMENT_X_CPUSTEP 11
#define WINDOW_FOOTER_ELEMENT_X_CPU_TEMP 16
#define WINDOW_FOOTER_ELEMENT_X_CPU_USE 7
//#define WINDOW_FOOTER_ELEMENT_X_BATERIA 30

//#define WINDOW_FOOTER_ELEMENT_X_ACTIVITY 24

#define WINDOW_FOOTER_ELEMENT_Y_CPU_USE 1
#define WINDOW_FOOTER_ELEMENT_Y_CPU_TEMP 1
#define WINDOW_FOOTER_ELEMENT_Y_FPS 1

#define WINDOW_FOOTER_ELEMENT_Y_F5MENU 2
#define WINDOW_FOOTER_ELEMENT_Y_ZESARUX_EMULATOR 2

/*

Como quedan los textos:

01234567890123456789012345678901
50 FPS 100% CPU 29.3C   -SPEECH-

           -TAPE-
          -FLASH-
          -PRINT-
	   MMC
          -ZXPAND-
*/


#define MENU_OPCION_SEPARADOR 0
#define MENU_OPCION_NORMAL 1
#define MENU_OPCION_ESC 2


//item de menu con memoria asignada pero item vacio
//si se va a agregar un item, se agrega en el primero con este tipo de opcion
#define MENU_OPCION_UNASSIGNED 254




#define MENU_RETORNO_NORMAL 0
#define MENU_RETORNO_ESC -1
#define MENU_RETORNO_F1 -2
#define MENU_RETORNO_F2 -3
#define MENU_RETORNO_F10 -4
#define MENU_RETORNO_BACKGROUND -5

extern void menu_footer_activity(char *texto);
extern void menu_delete_footer_activity(void);

//funcion a la que salta al darle al enter. valor_opcion es un valor que quien crea el menu puede haber establecido,
//para cada item de menu, un valor diferente
//al darle enter se envia el valor de ese item seleccionado a la opcion de menu
typedef void (*t_menu_funcion)(MENU_ITEM_PARAMETERS);

//funcion que retorna 1 o 0 segun si opcion activa
typedef int (*t_menu_funcion_activo)(void);



//Aunque en driver xwindows no cabe mas de 30 caracteres, en stdout, por ejemplo, cabe mucho mas.
#define MAX_TEXTO_OPCION 100
#define MENU_MAX_TEXTO_MISC 1024

struct s_menu_item {
	//texto de la opcion
	//char *texto;

	//Aunque en driver xwindows no cabe mas de 30 caracteres, en stdout, por ejemplo, cabe mucho mas
	char texto_opcion[MAX_TEXTO_OPCION];

    char texto_opcion_spanish[MAX_TEXTO_OPCION];

    char texto_opcion_catalan[MAX_TEXTO_OPCION];

    //posible prefijo
    char texto_opcion_prefijo[MAX_TEXTO_OPCION];

    //posible sufijo
    char texto_opcion_sufijo[MAX_TEXTO_OPCION];

    //item final concatenando prefijo y texto
    char texto_opcion_concatenado[MAX_TEXTO_OPCION];

	//Texto misc para usuario, para guardar url por ejemplo en online browser
	char texto_misc[MENU_MAX_TEXTO_MISC];

	//texto de ayuda
	char *texto_ayuda;

	//texto de tooltip
	char *texto_tooltip;

	//atajo de teclado
	z80_byte atajo_tecla;

	//un valor enviado a la opcion, que puede establecer la funcion que agrega el item
	int valor_opcion;

	//Para tipos de menus "tabulados", aquellos en que:
	//-no se crea ventana al abrir
	//-las opciones tienen coordenadas X e Y relativas a la ventana activa
	//-se puede mover tambien usando teclas izquierda, derecha
	//-el texto de las opciones no se rellena con espacios por la derecha, se muestra tal cual en las coordenadas X e Y indicadas

	int es_menu_tabulado;
	int menu_tabulado_x;
	int menu_tabulado_y;


	//tipo de la opcion
	int tipo_opcion;
	//funcion a la que debe saltar
	t_menu_funcion menu_funcion;
	//funcion que retorna 1 o 0 segun si opcion activa
	t_menu_funcion_activo menu_funcion_activo;

	//funcion a la que debe saltar al pulsar espacio
	t_menu_funcion menu_funcion_espacio;

    //Opcion está marcada, simplemente mostrar en otro color
    //Usado por ejemplo en teclados de perfil de teclas de ZENG Online
    int opcion_marcada;

	//siguiente item
	struct s_menu_item *siguiente_item;

    //funcion que salta al seleccionar un item
    //Esto es, al mover el cursor sobre esa opción, podemos hacer que llame a una función
    void (*menu_funcion_seleccionada)(struct s_menu_item *item_seleccionado);

    //Indica para ciertos cuadros de dialogo, que usan funcion de menu pero no se comportan como menus
    //les deja por ejemplo redimensionar el menu. Quien lo usa? De momento nadie...
    int no_es_realmente_un_menu;

    //Indica que al disparar la accion, se cerraran todos los menus (se activa salir_todos_menus=1)
    //no haria falta activar esto para items que abren menus tabulados, pues en este caso, al pulsar ESC dentro de un menu tabulado,
    //este ya enviara la orden de cerrar todos menus
    int menu_se_cerrara;

    //si este item de menu desplega otro menu
    int tiene_submenu;

    //si este item de menu despliega una ventana y por tanto agregamos puntos suspensivos
    int genera_ventana;

    //Si es un item avanzado
    //TODO: Quiza en menus tipo tabulado se ha probado poco
    //en caso que se usase, hacerle mas pruebas. Esto es poco probable,
    //porque los menus avanzados se usan en items de menu normales,
    //pero en cambio los items tabulados suelen ser para opciones de ventanas
    int item_avanzado;

    //Si no queremos indexar la busqueda en este menu. Por ejemplo Disk Info-> Tracks list
    int no_indexar_busqueda;

    //este menu se selecciona enter una vez o escape una vez, pero la funcion que la llamada no vuelve
    //a dibujar el menu, por tanto a nivel de path de indexacion de busqueda, se pierde el ultimo path de submenu
    //Ejemplo de esto: menu_simple_ten_choices
    int one_time;

    //puntero a opcion que conmuta, gestionado directamente desde funcion de menu
    z80_bit *opcion_conmuta;

    //Apunta al texto del nombre de la ruta completa de este menu, para indexar la busqueda
    char *index_full_path;
};

typedef struct s_menu_item menu_item;



//Para el indice de opciones de menu. Cada item de un menu
struct s_index_menu_linea {
    char texto_opcion[MAX_TEXTO_OPCION];

    //siguiente item menu. NULL si no hay mas
    struct s_index_menu_linea *next_item_menu;
};

typedef struct s_index_menu_linea index_menu_linea;

//Que indique rama, por ejemplo: settings-> display settings
#define MAX_LENGTH_FULL_PATH_SUBMENU ((MAX_TEXTO_OPCION*10)+10*3)

//Para el indice de opciones de menu
struct s_index_menu {
    //Nombre menu : Que indique rama, por ejemplo: settings-> display settings
    //Cabe hasta 10 veces (10 submenus) considerando como maximo texto titulo el maximo longitud de un item
    char titulo_menu[MAX_LENGTH_FULL_PATH_SUBMENU];

    //Puntero a siguiente menu. NULL si no hay mas
    struct s_index_menu *next_menu;

    //Puntero al primer item de menu
    index_menu_linea *first_item_menu;


};

typedef struct s_index_menu index_menu;

#define ZESARUX_INDEX_MENU_FILE "zesarux_index_menu.idx"

extern index_menu *first_index_menu;
extern index_menu *zxvision_index_add_replace_menu(char *titulo_menu);
extern void zxvision_index_add_menu_linea(index_menu *indice_menu,char *nombre_linea);
extern index_menu *zxvision_index_search_menu(char *nombre);
extern void zxvision_index_search_init_menu_path_main_menu(void);
extern void zxvision_index_delete_last_submenu_path(void);
extern index_menu *zxvision_index_entrada_menu(char *titulo);
extern void zxvision_index_search_init_menu_path(void);
extern void zxvision_index_menu_init(void);
extern void zxvision_index_save_to_disk(void);
extern void zxvision_index_load_from_disk(void);

extern void menu_convierte_texto_sin_modificadores(char *texto,char *texto_destino);
extern int menu_dibuja_menu_recorrer_menus;
extern int menu_dibuja_menu_recorrer_menus_entrado_submenu;

extern int menu_ventana_scanf(char *titulo,char *texto,int max_length);
extern int menu_ventana_scanf_numero(char *titulo,char *texto,int max_length,int incremento,int minimo,int maximo,int circular);
extern int menu_ventana_scanf_numero_enhanced(char *titulo,int *variable,int max_length,int incremento,int minimo,int maximo,int circular);
extern int zxvision_menu_filesel(char *titulo,char *filtros[],char *archivo);
//extern char menu_filesel_last_directory_seen[];

extern int menu_storage_string_root_dir(char *string_root_dir);

extern void menu_add_item_menu_inicial(menu_item **m,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo);
extern void menu_add_item_menu_inicial_format(menu_item **p,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...);
extern void menu_add_item_menu(menu_item *m,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo);
extern void menu_add_item_menu_format(menu_item *m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...);
extern void menu_add_item_menu_ayuda(menu_item *m,char *texto_ayuda);
extern void menu_add_item_menu_tooltip(menu_item *m,char *texto_tooltip);
extern void menu_add_item_menu_shortcut(menu_item *m,z80_byte tecla);
extern void menu_add_item_menu_valor_opcion(menu_item *m,int valor_opcion);
extern void menu_add_item_menu_tabulado(menu_item *m,int x,int y);
extern void menu_add_item_menu_espacio(menu_item *m,t_menu_funcion menu_funcion_espacio);
extern void menu_add_item_menu_misc(menu_item *m,char *texto_misc);
extern void menu_add_item_menu_opcion_conmuta(menu_item *m,z80_bit *opcion);
extern void menu_add_item_menu_marcar_opcion(menu_item *m,int valor);
extern void menu_add_item_menu_seleccionado(menu_item *m,void (*menu_funcion_seleccionada)(struct s_menu_item *));
extern void menu_add_item_menu_index_full_path(menu_item *m,char *index_name_en,char *index_name_es,char *index_name_ca);
extern void menu_add_item_menu_separator(menu_item *m);
extern void menu_add_item_menu_no_es_realmente_un_menu(menu_item *m);
extern void menu_add_item_menu_no_indexar_busqueda(menu_item *m);

//Flags como alternativa para usar las funciones de menu_add_item_menu_tiene_submenu, menu_add_item_menu_genera_ventana, etc
#define MENU_ITEM_FLAG_TIENE_SUBMENU 1
#define MENU_ITEM_FLAG_GENERA_VENTANA 2
#define MENU_ITEM_FLAG_ES_AVANZADO 4
#define MENU_ITEM_FLAG_SE_CERRARA 8

extern void menu_add_item_menu_add_flags(menu_item *m,int flags);

extern void menu_add_item_menu_tiene_submenu(menu_item *m);
extern void menu_add_item_menu_genera_ventana(menu_item *m);
extern void menu_add_item_menu_es_avanzado(menu_item *m);
extern void menu_add_item_menu_se_cerrara(menu_item *m);


extern void menu_add_item_menu_spanish(menu_item *m,char *s);
extern void menu_add_item_menu_spanish_format(menu_item *m,const char * format , ...);

extern void menu_add_item_menu_catalan(menu_item *m,char *s);
extern void menu_add_item_menu_catalan_format(menu_item *m,const char * format , ...);

extern void menu_add_item_menu_spanish_catalan(menu_item *m,char *spanish,char *catalan);

extern void menu_add_item_menu_en_es_ca(menu_item *m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,char *english,char *spanish,char *catalan);
extern void menu_add_item_menu_en_es_ca_inicial(menu_item **m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,char *english,char *spanish,char *catalan);

extern void menu_add_item_menu_prefijo(menu_item *m,char *s);
extern void menu_add_item_menu_prefijo_format(menu_item *m,const char * format , ...);

extern void menu_add_item_menu_sufijo(menu_item *m,char *s);
extern void menu_add_item_menu_sufijo_format(menu_item *m,const char * format , ...);


extern void menu_warn_message(char *texto);
extern void menu_error_message(char *texto);

extern void menu_generic_message(char *titulo, const char * texto);
extern void menu_generic_message_format(char *titulo, const char * format , ...);
extern void menu_generic_message_splash(char *titulo, const char * texto);
extern void menu_generic_message_warn(char *titulo, const char * texto);

extern void zxvision_menu_generic_message_setting(char *titulo, const char *texto, char *texto_opcion, int *valor_opcion);
extern int zxvision_menu_generic_message_two_buttons(char *titulo, const char *texto,char *texto_opcion1, char *texto_opcion2);


extern void zxvision_rearrange_background_windows(int si_cascada,int si_aplicar_a_inmutables);

extern void menu_generic_message_tooltip(char *titulo, int volver_timeout, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, const char * texto_format , ...);

#define TOOLTIP_SECONDS 4
#define WINDOW_SPLASH_SECONDS 3

extern void menu_add_ESC_item(menu_item *array_menu_item);
extern int menu_dibuja_menu_no_title_lang(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo);
extern int menu_dibuja_menu(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo_en,char *titulo_es,char *titulo_ca);
extern int menu_dibuja_menu_dialogo_no_title_lang(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo);
extern int menu_dibuja_menu_no_indexado(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo);
extern void menu_dibuja_submenu_cierra_todos_submenus(void);
extern void menu_dibuja_submenu_free_all(void);
extern void menu_tape_settings_trunc_name(char *orig,char *dest,int max);

extern int menu_confirm_yesno(char *texto_ventana);
extern int menu_confirm_yesno_texto(char *texto_ventana,char *texto_interior);
extern int menu_confirm_yesno_texto_additional_item(char *texto_ventana,char *texto_interior,char *(*texto_adicional)(void),void (*funcion_trigger_texto_adicional) (void),char *(*texto_adicional2)(void),void (*funcion_trigger_texto_adicional2) (void) );
//extern int menu_ask_no_append_truncate_texto(char *texto_ventana,char *texto_interior);
extern int menu_simple_two_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2);
extern int menu_simple_three_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3);
extern int menu_simple_four_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4);
extern int menu_simple_five_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5);
extern int menu_simple_six_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5,char *opcion6);
extern int menu_simple_seven_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5,char *opcion6,char *opcion7);
extern int menu_simple_eight_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8);
extern int menu_simple_nine_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8,char *opcion9);
extern int menu_simple_ten_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8,char *opcion9,char *opcion10);

extern void menu_refresca_pantalla(void);

extern int menu_debug_sprites_total_colors_mapped_palette(int paleta);
extern int menu_debug_sprites_return_index_palette(int paleta, z80_byte color);
extern int menu_debug_sprites_return_color_palette(int paleta, z80_byte color);
extern int menu_debug_sprites_max_value_mapped_palette(int paleta);
extern void menu_dibuja_rectangulo_relleno(zxvision_window *w,int x, int y, int ancho, int alto, int color);
extern void menu_debug_sprites_get_palette_name(int paleta, char *s);

extern int menu_debug_get_total_digits_dec(int valor);


extern void menu_debug_registers_show_scan_position(void);

//Fin funciones basicas que se suelen usar desde menu_items.c






extern void putchar_menu_overlay(int x,int y,z80_byte caracter,int tinta,int papel);
extern void putchar_menu_overlay_parpadeo(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo);
extern void deletechar_menu_overlay(int x,int y);

extern void new_menu_putchar_footer(int x,int y,z80_byte caracter,int tinta,int papel);
extern void menu_putstring_footer(int x,int y,char *texto,int tinta,int papel);
extern void menu_footer_clear_bottom_line(void);
extern void cls_menu_overlay(void);
extern int menu_multitarea;
extern int menu_emulation_paused_on_menu;
extern int menu_emulation_paused_on_menu_by_debug_step_mode;
extern int menu_abierto;
extern int footer_last_cpu_use;

extern void menu_muestra_pending_error_message(void);

extern void menu_set_menu_abierto(int valor);

extern z80_bit menu_hide_vertical_percentaje_bar;
extern z80_bit menu_hide_submenu_indicator;
extern z80_bit menu_hide_minimize_button;
extern z80_bit menu_hide_maximize_button;
extern z80_bit menu_hide_close_button;
extern z80_bit menu_change_frame_when_resize_zone;
extern z80_bit menu_hide_background_button_on_inactive;
extern z80_bit menu_invert_mouse_scroll;
extern z80_bit menu_mouse_right_send_esc;
extern z80_bit menu_old_behaviour_close_menus;
extern z80_bit menu_show_submenus_tree;

extern z80_bit auto_frameskip_even_when_movin_windows;
extern z80_bit frameskip_draw_zxdesktop_background;

extern int if_pending_error_message;
extern char pending_error_message[];

extern void menu_footer_bottom_line(void);


extern void menu_inicio(void);
extern void menu_inicio_bucle(void);

extern void set_welcome_message(void);
extern void reset_welcome_message(void);
extern void get_welcome_message(char *texto_welcome);
extern z80_bit menu_splash_text_active;
extern int menu_splash_segundos;
extern void show_all_windows_startup(void);
extern z80_byte menu_da_todas_teclas(void);
extern z80_byte menu_da_todas_teclas_si_reset_mouse_movido(int reset_mouse_movido,int absolutamente_todas_teclas);
extern z80_byte menu_da_todas_teclas_cualquiera(void);
extern int menu_si_tecla_pulsada(void);
extern void menu_espera_tecla_o_joystick(void);
extern void menu_espera_no_tecla(void);
extern void menu_get_dir(char *ruta,char *directorio);

extern int menu_tell_if_realjoystick_detected_counter;
extern void menu_tell_if_realjoystick_detected(void);
extern void menu_hardware_realjoystick_test_reset_last_values(void);


extern int menu_info_joystick_last_button;
extern int menu_info_joystick_last_type;
extern int menu_info_joystick_last_value;
extern int menu_info_joystick_last_index;
extern int menu_info_joystick_last_raw_value;

extern int menu_da_ancho_titulo(char *titulo);

extern int menu_tooltip_counter;

extern int menu_window_splash_counter;
extern int menu_window_splash_counter_ms;

extern z80_bit tooltip_enabled;

extern z80_bit mouse_menu_disabled;
extern z80_bit mouse_menu_ignore_click_open;

extern int mouse_movido;

extern char *quickfile;
extern char quickload_file[];

extern z80_bit menu_button_smartload;
extern z80_bit menu_button_osdkeyboard;
extern z80_bit menu_button_osdkeyboard_return;
extern z80_bit menu_button_osd_adv_keyboard_return;
extern z80_bit menu_button_osd_adv_keyboard_openmenu;
extern z80_bit menu_button_exit_emulator;
extern z80_bit menu_event_drag_drop;
extern z80_bit menu_event_open_zmenu_file;
extern z80_bit menu_event_pending_zmenu_file_menu_open;
extern z80_bit menu_event_pending_drag_drop_menu_open;
extern z80_bit menu_event_new_version_show_changes;
extern z80_bit menu_event_new_update;

extern char menu_open_zmenu_file_path[];

extern void zmenu_parse_file(char *archivo);


//extern char menu_event_drag_drop_file[PATH_MAX];
extern z80_bit menu_event_remote_protocol_enterstep;
extern z80_bit menu_button_f_function;
extern int menu_button_f_function_index;

extern z80_bit zrcp_easter_egg_running;

//numero maximo de entradas
#define MAX_OSD_ADV_KEYB_WORDS 1000
//longitud maximo de cada entrada
#define MAX_OSD_ADV_KEYB_TEXT_LENGTH 31

extern int osd_adv_kbd_defined;
extern char osd_adv_kbd_list[MAX_OSD_ADV_KEYB_WORDS][MAX_OSD_ADV_KEYB_TEXT_LENGTH];

extern int menu_calcular_ancho_string_item(char *texto);
extern void menu_get_legend_short_long(char *destination_string,int ancho_visible,char *short_string,char *long_string);


extern int menu_contador_teclas_repeticion;
extern int menu_segundo_contador_teclas_repeticion;

extern int menu_speech_tecla_pulsada;

extern char binary_file_load[];
extern char binary_file_save[];

extern int menu_ask_file_to_save(char *titulo_ventana,char *filtro,char *file_save);

extern char menu_buffer_textspeech_filter_program[];

extern char menu_buffer_textspeech_stop_filter_program[];

extern void menu_textspeech_send_text(char *texto);

extern void menu_textspeech_filter_corchetes(char *texto_orig,char *texto);

extern void screen_print_splash_text(int y,int tinta,int papel,char *texto);
extern void screen_print_splash_text_center(int tinta,int papel,char *texto);
extern void screen_print_splash_text_center_no_if_previous(int tinta,int papel,char *texto);

extern char menu_realtape_name[];

extern z80_bit menu_force_writing_inverse_color;

//extern int menu_filesel_mkdir(char *directory);

extern int menu_mmc_image_montada;

extern z80_bit force_confirm_yes;

extern void draw_middle_footer(void);

extern z80_int menu_mouse_frame_counter;

struct s_estilos_gui {

		int require_complete_video_driver;

        char nombre_estilo[20];
        int papel_normal;
        int tinta_normal;

        int muestra_cursor; //si se muestra cursor > en seleccion de opcion
                                        //Esto es asi en ZXSpectr
                                        //el cursor entonces se mostrara con los colores indicados a continuacion

        int muestra_recuadro; //si se muestra recuadro en ventana

        int muestra_rainbow;  //si se muestra rainbow en titulo

        int solo_mayusculas; //para ZX80/81


        int papel_seleccionado;
        int tinta_seleccionado;

        int papel_no_disponible; //Colores cuando una opción no esta disponible
        int tinta_no_disponible;

        int papel_seleccionado_no_disponible;    //Colores cuando una opcion esta seleccionada pero no disponible
        int tinta_seleccionado_no_disponible;

        int papel_titulo, tinta_titulo;
        int color_recuadro; //color del recuadro de ventana. Para la mayoria de estilos, es igual que papel_titulo
		int papel_titulo_inactiva, tinta_titulo_inactiva; //Colores de titulo con ventana inactiva

        int color_waveform;  //Color para forma de onda en view waveform
        int color_unused_visualmem; //Color para zona no usada en visualmem

        int color_block_visualtape; //Color para el bloque marcado en Visual Real Tape


	int papel_opcion_marcada; //Color para opcion marcada, de momento solo usado en osd keyboard
	int tinta_opcion_marcada;

    int papel_fileselector_files; //Color para fileselector la zona de seleccion de archivos
    int tinta_fileselector_files;

	z80_byte boton_cerrar; //caracter de cerrado de ventana

    z80_byte boton_minimizar; //caracter de minimizado de ventana

    z80_byte boton_maximizar; //caracter de maximizado de ventana

    z80_byte boton_restaurar; //caracter de restaurado de ventana

    z80_byte boton_background; //caracter de background de ventana

    z80_byte caracter_espacio_titulo; //caracter de fondo de titulo de ventana

	int color_aviso; //caracter de aviso de volumen alto, cpu alto, etc. normalmente rojo

	//franjas de color normales con brillo
	int *franjas_brillo;

	//franjas de color oscuras
	int *franjas_oscuras;

    //si texto inverso solo cambia color tinta, en vez de invertir papel y tinta. -1 si no lo hace. Otro valor es el color de tinta
    int inverse_tinta;

    //si no se rellena con espacios todo el titulo de la ventana. BeOS por ejemplo hace esto
    //ademas boton de minimizar esta a la derecha del titulo
    int no_rellenar_titulo;

    //charset usado
    unsigned char *style_char_set;

    //tipo franjas: 0 normal, diferente de 0 indica que caracter usar
    z80_byte caracter_franja;

};

typedef struct s_estilos_gui estilos_gui;


#define ESTILOS_GUI 31




extern void estilo_gui_retorna_nombres(void);

extern int estilo_gui_activo;

extern estilos_gui definiciones_estilos_gui[];

extern void set_charset_from_gui(void);

extern void menu_draw_ext_desktop(void);

extern int menu_ext_desktop_enabled_place_menu(void);
extern int menu_get_width_characters_ext_desktop(void);

extern void menu_ext_desktop_buttons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_inicio_botones,int *p_xfinal_botones);
extern void menu_ext_desktop_lower_icons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_xinicio_botones,int *p_xfinal_botones,int *p_yinicio_botones);

#define MENU_MAX_EXT_DESKTOP_FILL_NUMBER 7

extern int menu_ext_desktop_fill;
extern int menu_ext_desktop_fill_first_color;
extern int menu_ext_desktop_fill_second_color;

extern z80_bit menu_ext_desktop_degraded_inverted;

extern z80_bit menu_ext_desktop_transparent_lower_icons;
extern z80_bit menu_ext_desktop_transparent_upper_icons;

extern z80_bit menu_ext_desktop_disable_box_upper_icons;
extern z80_bit menu_ext_desktop_disable_box_lower_icons;

extern z80_bit menu_ext_desktop_transparent_configurable_icons;
extern z80_bit menu_ext_desktop_configurable_icons_text_background;

extern int lowericon_realtape_frame;
extern int lowericon_cf2_floppy_frame;

extern int menu_pressed_zxdesktop_lower_icon_which;
extern int menu_pressed_zxdesktop_button_which;
extern int menu_pressed_zxdesktop_configurable_icon_which;
extern int menu_pressed_zxdesktop_right_button_background;
extern z80_bit menu_pressed_close_all_menus;

extern void init_zxdesktop_configurable_icons(void);
extern void create_default_zxdesktop_configurable_icons(void);
extern void zxvision_reorder_configurable_icons(void);
extern void zxvision_set_configurable_icon_position(int icon,int x,int y);
extern void zxvision_recover_configurable_icon_from_trash(int indice_icono);
extern void zxvision_empty_trash(void);
extern void zxvision_move_configurable_icon_to_trash(int indice_icono);
extern int zxvision_add_configurable_icon(int indice_funcion);
extern int zxvision_add_configurable_icon_by_id_action(enum defined_f_function_ids id_funcion);
extern void menu_inicio_handle_configurable_icon_presses(void);
extern void menu_inicio_handle_right_button_background(void);
extern void zxvision_check_all_configurable_icons_positions(void);
extern int zxvision_add_configurable_icon_no_add_position(int indice_funcion);
extern void zxvision_set_configurable_icon_text(int indice_icono,char *texto);
extern void zxvision_set_configurable_icon_extra_info(int indice_icono,char *extra_info);
extern int zxdesktop_configurable_icons_enabled_and_visible(void);
extern void zxvision_create_configurable_icon_file_type(enum defined_f_function_ids id_funcion,char *nombre);
extern void zxvision_create_link_desktop_from_window(zxvision_window *w);
extern void zxvision_get_next_free_icon_position(int *p_x,int *p_y);

extern int pulsado_alguna_ventana_con_menu_cerrado;


#define ESTILO_GUI_PAPEL_NORMAL (definiciones_estilos_gui[estilo_gui_activo].papel_normal)
#define ESTILO_GUI_TINTA_NORMAL (definiciones_estilos_gui[estilo_gui_activo].tinta_normal)
#define ESTILO_GUI_PAPEL_SELECCIONADO (definiciones_estilos_gui[estilo_gui_activo].papel_seleccionado)
#define ESTILO_GUI_TINTA_SELECCIONADO (definiciones_estilos_gui[estilo_gui_activo].tinta_seleccionado)


#define ESTILO_GUI_PAPEL_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].papel_no_disponible)
#define ESTILO_GUI_TINTA_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].tinta_no_disponible)
#define ESTILO_GUI_PAPEL_SEL_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].papel_seleccionado_no_disponible)
#define ESTILO_GUI_TINTA_SEL_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].tinta_seleccionado_no_disponible)

#define ESTILO_GUI_PAPEL_OPCION_MARCADA (definiciones_estilos_gui[estilo_gui_activo].papel_opcion_marcada)
#define ESTILO_GUI_TINTA_OPCION_MARCADA (definiciones_estilos_gui[estilo_gui_activo].tinta_opcion_marcada)

#define ESTILO_GUI_PAPEL_FILESELECTOR_FILES (definiciones_estilos_gui[estilo_gui_activo].papel_fileselector_files)
#define ESTILO_GUI_TINTA_FILESELECTOR_FILES (definiciones_estilos_gui[estilo_gui_activo].tinta_fileselector_files)

#define ESTILO_GUI_PAPEL_TITULO (definiciones_estilos_gui[estilo_gui_activo].papel_titulo)
#define ESTILO_GUI_TINTA_TITULO (definiciones_estilos_gui[estilo_gui_activo].tinta_titulo)
#define ESTILO_GUI_COLOR_RECUADRO (definiciones_estilos_gui[estilo_gui_activo].color_recuadro)

#define ESTILO_GUI_PAPEL_TITULO_INACTIVA (definiciones_estilos_gui[estilo_gui_activo].papel_titulo_inactiva)
#define ESTILO_GUI_TINTA_TITULO_INACTIVA (definiciones_estilos_gui[estilo_gui_activo].tinta_titulo_inactiva)

#define ESTILO_GUI_COLOR_WAVEFORM (definiciones_estilos_gui[estilo_gui_activo].color_waveform)
#define ESTILO_GUI_COLOR_UNUSED_VISUALMEM (definiciones_estilos_gui[estilo_gui_activo].color_unused_visualmem)

#define ESTILO_GUI_COLOR_BLOCK_VISUALTAPE (definiciones_estilos_gui[estilo_gui_activo].color_block_visualtape)

#define ESTILO_GUI_MUESTRA_CURSOR (definiciones_estilos_gui[estilo_gui_activo].muestra_cursor)
#define ESTILO_GUI_MUESTRA_RECUADRO (definiciones_estilos_gui[estilo_gui_activo].muestra_recuadro)
#define ESTILO_GUI_MUESTRA_RAINBOW (definiciones_estilos_gui[estilo_gui_activo].muestra_rainbow)
#define ESTILO_GUI_SOLO_MAYUSCULAS (definiciones_estilos_gui[estilo_gui_activo].solo_mayusculas)

#define ESTILO_GUI_BOTON_CERRAR (definiciones_estilos_gui[estilo_gui_activo].boton_cerrar)

#define ESTILO_GUI_BOTON_MINIMIZAR (definiciones_estilos_gui[estilo_gui_activo].boton_minimizar)

#define ESTILO_GUI_BOTON_MAXIMIZAR (definiciones_estilos_gui[estilo_gui_activo].boton_maximizar)

#define ESTILO_GUI_BOTON_RESTAURAR (definiciones_estilos_gui[estilo_gui_activo].boton_restaurar)

#define ESTILO_GUI_BOTON_BACKGROUND (definiciones_estilos_gui[estilo_gui_activo].boton_background)

#define ESTILO_GUI_CARACTER_ESPACIO_TITULO (definiciones_estilos_gui[estilo_gui_activo].caracter_espacio_titulo)

#define ESTILO_GUI_COLOR_AVISO (definiciones_estilos_gui[estilo_gui_activo].color_aviso)

#define ESTILO_GUI_FRANJAS_NORMALES (definiciones_estilos_gui[estilo_gui_activo].franjas_brillo)

#define ESTILO_GUI_FRANJAS_OSCURAS (definiciones_estilos_gui[estilo_gui_activo].franjas_oscuras)

#define ESTILO_GUI_CARACTER_FRANJA (definiciones_estilos_gui[estilo_gui_activo].caracter_franja)


#define ESTILO_GUI_REQUIRE_COMPLETE_VIDEO_DRIVER (definiciones_estilos_gui[estilo_gui_activo].require_complete_video_driver)

#define ESTILO_GUI_INVERSE_TINTA (definiciones_estilos_gui[estilo_gui_activo].inverse_tinta)


#define ESTILO_GUI_NO_RELLENAR_TITULO (definiciones_estilos_gui[estilo_gui_activo].no_rellenar_titulo)

//#define ESTILO_GUI_CHARSET (definiciones_estilos_gui[estilo_gui_activo].style_char_set)


#define MENU_ANCHO_FRANJAS_TITULO 5


extern void menu_adjust_gui_style_to_driver(void);

//extern z80_bit menu_espera_tecla_no_cpu_loop_flag_salir;
extern int salir_todos_menus;

extern int si_valid_char(z80_byte caracter);

extern z80_bit menu_event_open_menu;

extern z80_bit menu_was_open_by_left_mouse_button;
extern z80_bit menu_was_open_by_right_mouse_button;

extern z80_bit force_next_menu_position;


extern void menu_chdir_sharedfiles(void);

extern void menu_debug_registers_dump_hex(char *texto,menu_z80_moto_int direccion,int longitud);

extern void menu_debug_registers_dump_ascii(char *texto,menu_z80_moto_int direccion,int longitud,int modoascii,z80_byte valor_xor);

extern int menu_gui_zoom;

extern int menu_escribe_linea_startx;

extern z80_bit menu_disable_special_chars;

extern int menu_active_item_primera_vez;

extern int menu_ask_list_texto(char *texto_ventana,char *texto_interior,char *entradas_lista[]);

extern z80_byte *menu_clipboard_pointer;

extern void menu_paste_clipboard_to_file(char *destination_file);

extern void zxvision_copy_contents_to_clipboard(zxvision_window *ventana);

extern void zxvision_sound_event_error_menu(void);
extern void zxvision_sound_event_cursor_movement(void);
extern void zxvision_sound_event_close_window(void);
extern void zxvision_sound_event_new_window(void);

extern char *menu_get_string_language(char *texto);

extern int timer_osd_keyboard_menu;

extern char snapshot_load_file[];
extern char snapshot_save_file[];

extern int menu_char_width;
extern int menu_char_height;

//extern int overlay_usado_screen_array[];


extern int return_color_zesarux_ascii(char c);

//extern z80_bit menu_filesel_posicionar_archivo;
//extern char menu_filesel_posicionar_archivo_nombre[];

extern z80_bit menu_limit_menu_open;

extern int menu_contador_conmutar_ventanas;

//extern z80_bit menu_filesel_hide_dirs;

//extern z80_bit menu_filesel_hide_size;

//extern z80_bit menu_filesel_utils_allow_folder_delete;

//extern z80_bit menu_filesel_show_previews;

extern int osd_kb_no_mostrar_desde_menu;

extern void menu_fire_event_open_menu(void);

extern int menu_button_f_function_action;

//extern z80_bit screen_bw_no_multitask_menu;

extern int menu_hardware_autofire_cond(void);

//extern void menu_file_mmc_browser_show_file(z80_byte *origen,char *destino,int sipuntoextension,int longitud);

//extern void menu_file_viewer_read_text_file(char *title,char *file_name);



extern int menu_dsk_getoff_track_sector(z80_byte *dsk_memoria,int total_pistas,int pista_buscar,int sector_buscar,int longitud_dsk);


extern int menu_decae_ajusta_valor_volumen(int valor_decae,int valor_volumen);

extern int menu_decae_dec_valor_volumen(int valor_decae,int valor_volumen);

extern void menu_reset_counters_tecla_repeticion(void);

extern void menu_debug_cpu_stats_diss_complete_no_print (z80_byte opcode,char *buffer,z80_byte preffix1,z80_byte preffix2);

extern void menu_string_volumen(char *texto,z80_byte registro_volumen,int indice_decae);
extern int menu_string_volumen_maxmin(char *texto,int valor_actual,int valor_previo,int valor_maximo);

extern void menu_copy_clipboard(char *texto);

extern int menu_change_memory_zone_list_title(char *titulo);
extern void menu_set_memzone(int valor_opcion);
extern void menu_network_error(int error);


//Tamanyo del array de char asignado para el browser de file utils
//Debe ser algo menor que MAX_TEXTO_GENERIC_MESSAGE
#define MAX_TEXTO_BROWSER (MAX_TEXTO_GENERIC_MESSAGE-1024)


#define MAX_LAST_FILESUSED 18

//#define ZXVISION_MAX_WINDOW_WIDTH 32
//#define ZXVISION_MAX_WINDOW_HEIGHT 24


extern void last_filesused_clear(void);
extern void last_filesused_insert(char *s);
extern char last_files_used_array[MAX_LAST_FILESUSED][PATH_MAX];
extern char file_utils_file_name[PATH_MAX];
extern void lastfilesuser_scrolldown(int posicion_up,int posicion_down);

//extern z80_bit menu_filesel_show_utils;


extern void menu_draw_last_fps(void);
extern void menu_draw_cpu_use_last(void);

extern int menu_last_cpu_use;

extern int cpu_use_total_acumulado;
extern int cpu_use_total_acumulado_medidas;

extern void putchar_footer_array(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo);
extern void redraw_footer(void);
extern void cls_footer(void);

extern int menu_get_gui_index_by_name(char *nombre);

extern int menu_center_x(void);
extern int menu_center_x_from_width(int ancho_ventana);
extern int menu_origin_x(void);
extern int menu_center_y(void);

extern void set_menu_gui_zoom(void);

extern z80_bit no_close_menu_after_smartload;

extern z80_bit menu_zxdesktop_buttons_enabled;

extern z80_bit zxdesktop_switch_button_enabled;

extern void zxdesktop_switchdesktop_timer_event(void);


extern void zxdesktop_draw_scrfile_load(void);
extern char zxdesktop_draw_scrfile_name[PATH_MAX];
extern int zxdesktop_draw_scrfile_enabled;
extern int zxdesktop_draw_scrfile_centered;
extern int zxdesktop_draw_scrfile_fill_scale;
extern int zxdesktop_draw_scrfile_scale_factor;
extern int zxdesktop_draw_scrfile_disable_flash;
extern int zxdesktop_draw_scrfile_mix_background;

extern int gamelife_timer_counter;

struct s_zxdesktop_lowericons_info {
	int (*is_visible)(void);
	int (*is_active)(void);
	void (*accion)(void);
	char **bitmap_active;
	char **bitmap_inactive;
	int *icon_is_inverse;
};



#define TOTAL_ZXDESKTOP_MAX_LOWER_ICONS 22



extern void zxvision_espera_tecla_condicion_progreso(zxvision_window *w,int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) );
extern void zxvision_simple_progress_window(char *titulo, int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) );
extern void menu_uncompress_zip_progress(char *zip_file,char *dest_dir);

extern int zxvision_adjust_cursor_top(zxvision_window *ventana);
extern int zxvision_adjust_cursor_bottom(zxvision_window *ventana);
extern void zxvision_set_cursor_line(zxvision_window *ventana,int linea);
extern void zxvision_inc_cursor_line(zxvision_window *ventana);
extern void zxvision_dec_cursor_line(zxvision_window *ventana);
extern void zxvision_set_visible_cursor(zxvision_window *ventana);
extern void zxvision_reset_visible_cursor(zxvision_window *ventana);

extern z80_bit menu_current_drive_mmc_image;

//Ancho limite hasta el que se hace incrementos de 128 desde item de menu. Luego se salta a tamaño 2560
#define ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS 1280
#define ZXDESKTOP_MAX_HEIGHT_MENU_FIXED_INCREMENTS 640

//Limite admitido desde menu
//TODO: Esto es mayor que el maximo de overlay de menu, cosa que no tiene mucho sentido, porque se puede hacer la ventana mas grande
//pero no se puede usar desde menu
//#define ZXDESKTOP_MAX_WIDTH_MENU_LIMIT (ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS*2)

//Limite admitido desde menu. El overlay total incluye pantalla emulada y zx desktop
//por tanto el maximo zxdesktop habria que quitarle el ancho minimo de una maquina (256)
//Da igual realmente que esto fuera mas grande, pues las funciones de driver de video (scrXXXX_get_menu_width)
//ya controlan que no se pueda usar mas ancho de overlay
#define ZXDESKTOP_MAX_WIDTH_MENU_LIMIT ((OVERLAY_SCREEN_MAX_WIDTH*8)-256)
#define ZXDESKTOP_MAX_HEIGHT_MENU_LIMIT ((OVERLAY_SCREEN_MAX_HEIGTH*8)-256)

//Total botones a la derecha: 3 (background, minimizar, maximizar)
#define ZXVISION_TOTAL_BUTTONS_RIGHT 3

//Espacio ocupado por las franjas de colores del titulo
#define ZXVISION_WIDTH_RAINBOW_TITLE 5

//Minimo de ancho de ventana: boton cierre, 5 de barras, 3 botones (!-+)
#define ZXVISION_MINIMUM_WIDTH_WINDOW (1+ZXVISION_WIDTH_RAINBOW_TITLE+ZXVISION_TOTAL_BUTTONS_RIGHT)

//Maximo ancho permitido usando boton. Por settings de menu, en cambio, se puede agrandar
#define ZXDESKTOP_MAXIMUM_WIDTH_BY_BUTTON (ZXDESKTOP_MAX_WIDTH_MENU_FIXED_INCREMENTS/zoom_x)
#define ZXDESKTOP_MINIMUM_WIDTH_BY_BUTTON (512/zoom_x)

//Maximo alto permitido usando boton. Por settings de menu, en cambio, se puede agrandar
#define ZXDESKTOP_MAXIMUM_HEIGHT_BY_BUTTON (ZXDESKTOP_MAX_HEIGHT_MENU_FIXED_INCREMENTS/zoom_y)
#define ZXDESKTOP_MINIMUM_HEIGHT_BY_BUTTON (32/zoom_y)

//Minimo de alto para mostrar recuadro alrededor de pantalla emulada
#define ZXDESKTOP_MINIMUM_HEIGHT_SHOW_FRAME 16

extern void menu_ext_desk_settings_width_enlarge_reduce(int enlarge_reduce);
extern void menu_ext_desk_settings_height_enlarge_reduce(int enlarge_reduce);

//mas que suficiente
#define MAX_ZXVISION_HELPER_SHORTCUTS_LENGTH 30

extern char zxvision_helper_shorcuts_accumulated[];


#endif
