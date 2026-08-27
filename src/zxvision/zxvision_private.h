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

#ifndef ZXVISION_PRIVATE_H
#define ZXVISION_PRIVATE_H



#include "cpu.h"

//Para usar PATH_MAX
#include "zesarux.h"


extern int zxvision_si_icono_cerca(int x,int y);
extern int zxvision_get_minimum_y_icon_position(void);
extern void zxvision_get_start_valid_positions_icons(int *p_xinicial,int *p_xfinal,int *p_yinicial,int *p_yfinal);
extern int zxvision_search_trash_configurable_icon(void);
extern int if_zxdesktop_trash_not_empty(void);

extern int zxvision_draw_icon_papelera_abierta;
extern overlay_screen footer_screen_array[];
extern int cuadrado_activo_resize;
extern int ventana_activa_puede_minimizar;
extern int ventana_es_background;
extern int menu_clipboard_size;
extern int current_win_x,current_win_y,current_win_ancho,current_win_alto,current_win_minimize_button_position;
extern int ventana_tipo_activa;
extern char *string_config_key_aid_startup;
extern int menu_dibuja_cuadrado_putpixel_background_ultima_fila;
extern int menu_dibuja_cuadrado_putpixel_background_ultima_columna;
extern int no_dibuja_ventana_muestra_pending_error_message;
extern int zxvision_mouse_x;
extern int zxvision_mouse_y;
extern zxvision_window *menu_dibuja_submenu_primer_submenu;
extern zxvision_window zxvision_window_splash_text;
extern int window_is_being_moved;
extern zxvision_window *menu_dibuja_cuadrado_putpixel_background_ventana;
extern int ventana_marca_redimensionado_raton_encima;
extern int splash_zesarux_logo_active;
extern int menu_dibuja_cuadrado_putpixel_background_ultima_dibujar;
extern char zxvision_switch_to_window_on_open_menu_name[];
extern z80_bit force_next_menu_position;
extern z80_bit menu_disable_special_chars;
extern int footer_last_cpu_use;

extern void menu_calculate_mouse_xy(void);
extern char **get_direct_function_icon_bitmap_final(int id_accion);
extern void menu_add_item_menu_funcion_texto_item(menu_item *m,char *(*menu_funcion_texto_item)(struct s_menu_item *));
extern void menu_add_item_menu_no_indexar_busqueda(menu_item *m);
extern void menu_add_item_menu_catalan(menu_item *m,char *s);
extern void menu_dibuja_cuadrado_putpixel_background_reset_cache(void);
extern void deletechar_menu_overlay(int x,int y);
extern int menu_calcular_ancho_string_item(char *texto);
extern void menu_copy_clipboard(char *texto);
extern void cls_footer(void);

extern void menu_escribe_texto(int x,int y,int tinta,int papel,char *texto);
extern int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea);
extern void menu_first_aid_random_startup(void);
extern int menu_first_aid_title(char *key_setting,char *title);
extern void menu_dibuja_submenu_free_all(void);
extern void menu_espera_tecla_o_joystick(void);
extern void menu_espera_tecla_o_pending_error_message(void);
extern int menu_ext_desktop_enabled_place_menu(void);
extern void menu_draw_last_fps(void);
extern void menu_draw_cpu_use_last(void);

extern void menu_ventana_draw_vertical_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido);
extern void menu_ventana_draw_horizontal_perc_bar(zxvision_window *w,int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido);
extern char **menu_get_extdesktop_button_bitmap(int numero_boton,int *es_set_machine);
extern int menu_inicio_return_button_userdef(int boton);
extern int menu_simple_ten_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2,char *opcion3,char *opcion4,char *opcion5,char *opcion6,char *opcion7,char *opcion8,char *opcion9,char *opcion10);
extern void putchar_menu_overlay(int x,int y,z80_byte caracter,int tinta,int papel);
extern void new_menu_putchar_footer(int x,int y,z80_byte caracter,int tinta,int papel);
extern void screen_print_splash_text(int y,int tinta,int papel,char *texto);
extern int menu_get_width_characters_ext_desktop(void);
extern void putchar_footer_array(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo);

extern int zxvision_coords_in_superior_windows(zxvision_window *w,int x,int y);
extern int zxvision_draw_overlay_if_exists(zxvision_window *w);
extern zxvision_window *zxvision_coords_in_below_windows(zxvision_window *w,int x,int y);
extern void zxvision_draw_below_windows(zxvision_window *w);
extern void zxvision_draw_overlays_below_windows(zxvision_window *w);
extern int zxvision_change_gui_style_select_by_name(char *estilo);
extern void zxvision_draw_filled_triangle_habitual(zxvision_window *w,int color_relleno,int color_aristas,int x1,int y1,int x2,int y2,int x3,int y3);
extern void zxvision_draw_filled_triangle(zxvision_window *w,int color_relleno,int color_aristas,int x1,int y1,int x2,int y2,int x3,int y3,
    void (*fun_putpixel) (zxvision_window *w,int x,int y,int color),int arista_no_dibujar);
extern z80_byte zxvision_common_getkey_refresh_noesperatecla(void);
extern int zxvision_add_configurable_icon_by_id_action(enum defined_f_function_ids id_funcion);

extern void zxvision_draw_window_contents_no_speech(zxvision_window *ventana);
extern int zxvision_if_vertical_scroll_bar(zxvision_window *w);
extern void zxvision_handle_maximize_maximize(zxvision_window *w);
extern void zxvision_espera_tecla_timeout_window_splash(int tipo);
extern index_menu *zxvision_index_add_replace_menu(char *titulo_menu);
extern void zxvision_index_add_menu_linea(index_menu *indice_menu,char *nombre_linea);
extern index_menu *zxvision_index_search_menu(char *nombre);
extern index_menu *zxvision_index_entrada_menu(char *titulo);
extern void zxvision_if_configurable_icon_not_on_valid_position_set(int icon);
extern void zxvision_espera_tecla_condicion_progreso(zxvision_window *w,int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) );

extern unsigned char zxvision_retorna_caracter_flecha_izquierda(void);
extern unsigned char zxvision_retorna_caracter_flecha_derecha(void);
extern void zxvision_set_offset_y_visible(zxvision_window *w,int y);
extern void zxvision_minimize_window(zxvision_window *w);
extern void zxvision_maximize_window(zxvision_window *w);
extern void zxvision_send_scroll_left(zxvision_window *w);
extern void zxvision_send_scroll_right(zxvision_window *w);
extern void zxvision_set_not_resizable(zxvision_window *w);
extern void zxvision_set_not_minimizable(zxvision_window *w);
extern void zxvision_reapply_style_colours_all_windows(void);

extern int zxvision_which_upper_button_is_mouse(void);
extern int zxvision_which_lower_button_is_mouse(void);
extern int zxvision_window_can_be_backgrounded(zxvision_window *w);
extern void zxvision_set_window_overlay_from_current(zxvision_window *ventana);
extern void zxvision_set_resizable(zxvision_window *w);
extern void zxvision_sound_event_error_menu(void);
extern void zxvision_sound_event_close_window(void);
extern void zxvision_sound_event_new_window(void);

extern char **menu_ext_desktop_draw_configurable_icon_return_machine_icon(void);
extern index_menu *zxvision_index_add_menu(char *titulo_menu_orig);
extern int menu_allows_mouse(void);
extern int menu_escribe_texto_si_cambio_tinta(char *texto,int indice);
extern int menu_escribe_texto_si_inverso(char *texto, int indice);
extern int menu_escribe_texto_si_parpadeo(char *texto, int indice);
extern int menu_first_aid_get_setting(char *texto);
extern int zxdesktop_lowericon_find_index(int icono);
extern int zxvision_coords_in_window(zxvision_window *w,int x,int y);
extern int zxvision_get_effective_height(zxvision_window *w);
extern int zxvision_get_id_direct_funcion_index(enum defined_f_function_ids id_funcion);
extern int zxvision_if_configurable_icon_on_valid_position(int x,int y);
extern void menu_dibuja_cuadrado(int x1,int y1,int x2,int y2,int color,int color_marca_redimensionado,int ventana_en_primer_plano,int alterar_estilo_cuadrado,zxvision_window *w);
extern void menu_dibuja_ventana_franja_arcoiris_oscuro_current(int indice);
extern void menu_dibuja_ventana_franja_arcoiris_trozo_current(int trozos);
extern void menu_ventana_draw_perc_bar_aux(zxvision_window *w,int x,int y,z80_byte caracter,int tinta,int papel);
extern void reset_splash_zesarux_logo(void);
extern void screen_print_splash_text_by_window(int lineas);
extern void zxvision_draw_filled_triangle_putpixel(zxvision_window *w,int x,int y,int color);
extern void zxvision_draw_filled_triangle_putpixel_buffer(int x,int y,int min_x,int min_y,int ancho,int *buffer);
extern void zxvision_draw_line_for_filled_triangle(int x1,int y1,int x2,int y2,int min_x,int min_y,int ancho,int *buffer,
    void (*fun_putpixel) (int x,int y,int min_x,int min_y,int ancho,int *buffer) );
extern void zxvision_index_erase_all_menu_lines(index_menu *menu);
extern void zxvision_reapply_style_colours_one_window(zxvision_window *w);
extern void zxvision_retorna_coordenadas_marco(int x,int y,int ancho,int alto,int *x1,int *y1,int *x2,int *y2);
extern void zxvision_sound_event_aux(char *nota,int duracion);

extern void menu_dibuja_cuadrado_putpixel_background(int x,int y,int color,int zoom_level);
extern void menu_dibuja_ventana_franja_arcoiris_oscuro(int x, int y, int ancho,int indice);
extern void menu_dibuja_ventana_franja_arcoiris_trozo(int x, int y, int ancho,int franjas);
extern void menu_dibuja_ventana_titulo(zxvision_window *w,char *titulo_original_utf);
extern void zxvision_change_space_colour(zxvision_window *w,int papel);

extern int menu_dibuja_ventana_ret_ancho_titulo(int ancho,char *titulo);
extern void putchar_menu_overlay_parpadeo_cache_or_not(int x,int y,z80_byte caracter,int tinta,int papel,int parpadeo,int use_cache_mismo_caracter);
extern z80_byte menu_retorna_caracter_cerrar(void);

extern void menu_ext_desktop_buttons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_inicio_botones,int *p_xfinal_botones);
extern void menu_ext_desktop_lower_buttons_get_geometry(int *p_ancho_boton,int *p_alto_boton,int *p_total_botones,int *p_xinicio_botones,int *p_xfinal_botones,int *p_yinicio_botones);

#endif
