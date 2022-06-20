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

#ifndef SCREEN_H
#define SCREEN_H

#include "cpu.h"
#include "z88.h"

#define SCREEN_TEXT_IZQ_BORDER 4
#define SCREEN_TEXT_TOP_BORDER 4


extern z80_int devuelve_direccion_pantalla(z80_byte x,z80_byte y);
extern void init_screen_addr_table(void);
extern z80_int *screen_addr_table;
//extern int spectrum_colortable_oscuro[];
//extern int spectrum_colortable_grises[];

extern int *spectrum_colortable;
extern int spectrum_colortable_normal[];

//extern int spectrum_colortable_new_blanco_y_negro[];

extern int screen_gray_mode;

extern z80_bit inverse_video;


extern void scr_refresca_border(void);


extern void (*scr_refresca_pantalla) (void);
extern void (*scr_refresca_pantalla_solo_driver) (void);
extern void (*scr_set_fullscreen) (void);
extern void (*scr_reset_fullscreen) (void);
extern int ventana_fullscreen;
extern int (*scr_init_pantalla) (void);
extern void (*scr_end_pantalla) (void);
extern z80_byte (*scr_lee_puerto) (z80_byte puerto_h,z80_byte puerto_l);
extern void (*scr_actualiza_tablas_teclado) (void);
extern void (*scr_debug_registers)(void);
extern void (*scr_messages_debug)(char *mensaje);

extern void (*scr_putpixel) (int x,int y,unsigned int color);
extern void (*scr_putpixel_final_rgb) (int x,int y,unsigned int color_rgb);
extern void (*scr_putpixel_final) (int x,int y,unsigned int color);

extern int (*scr_get_menu_width) (void);
extern int (*scr_get_menu_height) (void);

extern int (*scr_driver_can_ext_desktop) (void);

extern int screen_ext_desktop_width;
extern int screen_ext_desktop_height;
//extern int screen_ext_desktop_width_before_disabling;
extern int screen_ext_desktop_enabled;

extern int if_zxdesktop_enabled_and_driver_allows(void);

extern int screen_get_ext_desktop_width_no_zoom(void);
extern int screen_get_ext_desktop_width_zoom(void);

extern int screen_get_ext_desktop_height_no_zoom(void);
extern int screen_get_ext_desktop_height_zoom(void);

extern int screen_get_total_alto_ventana_zoom(void);

extern void screen_init_ext_desktop(void);
extern int screen_ext_desktop_place_menu;

extern int screen_get_ext_desktop_start_x(void);

extern void scr_return_margenxy_rainbow(int *margenx_izq,int *margeny_arr);

extern void (*scr_putpixel_zoom) (int x,int y,unsigned color);
extern void (*scr_putpixel_zoom_rainbow)(int x,int y,unsigned int color);

extern void (*scr_z88_cpc_load_keymap) (void);

extern void (*scr_detectedchar_print) (z80_byte caracter);


extern void scr_refresca_pantalla_comun(void);
extern void scr_refresca_border(void);

extern void screen_prism_refresca_pantalla(void);

extern void scr_refresca_pantalla_y_border_mk14(void);

extern z80_int *putpixel_cache;

extern void scr_refresca_pantalla_rainbow_comun(void);

extern void (*scr_putchar_zx8081) (int x,int y, z80_byte caracter);
extern void scr_refresca_pantalla_zx8081(void);
extern void scr_refresca_pantalla_y_border_zx8081(void);
extern void scr_putsprite_zx8081(z80_int direccion,int x,int y,z80_bit inverse);
extern void scr_putchar_zx8081_comun(int x,int y, z80_byte caracter);
extern void scr_refresca_border_zx8081(void);

extern void scr_refresca_pantalla_y_border_cpc(void);

extern z80_bit simulate_screen_zx8081;
extern int umbral_simulate_screen_zx8081;
extern int scr_get_4pixel(int x,int y);
extern int scr_get_4pixel_rainbow(int x,int y);

extern int if_store_scanline_interlace(int y);

extern void convertir_color_spectrum_paleta_to_rgb(z80_int valor,int *r,int *g,int *b);

extern z80_byte compare_char(unsigned char *origen,unsigned char *inverse);
extern z80_byte compare_char_step(z80_byte *origen,z80_byte *inverse,int step);
extern z80_byte compare_char_tabla_step(z80_byte *origen,z80_byte *inverse,z80_byte *tabla_leemos,int step);

extern z80_byte compare_char_tabla_rainbow(z80_byte *origen,z80_byte *inverse,z80_byte *tabla_leemos);
extern int calcula_offset_screen (int x,int y);

extern void (*scr_putchar_menu) (int x,int y, z80_byte caracter,int tinta,int papel);
extern void (*scr_putchar_footer) (int x,int y, z80_byte caracter,int tinta,int papel);
extern void scr_putsprite_comun(z80_byte *puntero,int x,int y,z80_bit inverse,int tinta,int papel,z80_bit fast_mode);
extern void scr_putsprite_comun_zoom(z80_byte *puntero,int x,int y,z80_bit inverse,int tinta,int papel,z80_bit fast_mode,int zoom_level);
extern void scr_putchar_menu_comun_zoom(z80_byte caracter,int x,int y,z80_bit inverse,int tinta,int papel,int zoom_level);
extern void scr_putchar_footer_comun_zoom(z80_byte caracter,int x,int y,z80_bit inverse,int tinta,int papel);

extern void scr_putpixel_gui_zoom(int x,int y,int color,int zoom_level);
extern void scr_putpixel_gui_no_zoom(int x,int y,int color,int zoom_level);

extern int scr_tiene_colores;

extern z80_bit ocr_settings_not_look_23606;


#define WINDOW_FOOTER_LINES 3
#define WINDOW_FOOTER_COLUMNS 32
#define WINDOW_FOOTER_SIZE (8*WINDOW_FOOTER_LINES*menu_footer)


//#define WINDOW_FOOTER_PAPER (7+8)
//#define WINDOW_FOOTER_INK 0


#define WINDOW_FOOTER_PAPER (ESTILO_GUI_PAPEL_NORMAL)
#define WINDOW_FOOTER_INK (ESTILO_GUI_TINTA_NORMAL)

extern char scr_new_driver_name[];

extern void scr_set_driver_name(char *nombre);

extern z80_bit texto_artistico;
extern int umbral_arttext;

extern z80_bit screen_text_all_refresh_pixel;

extern int screen_text_all_refresh_pixel_scale;

extern int scr_refresca_pantalla_tsconf_text_max_ancho;
extern int scr_refresca_pantalla_tsconf_text_offset_x;
extern int scr_refresca_pantalla_tsconf_text_max_alto;
extern int scr_refresca_pantalla_tsconf_text_offset_y;

extern z80_bit screen_text_all_refresh_pixel_invert;

extern int screen_refresh_menu;

extern int screen_stdout_driver;
extern int screen_simpletext_driver;

extern void load_screen(char *scrfile);
extern void save_screen_scr(char *scrfile);
extern void save_screen(char *screen_save_file);

//extern z80_bit interlaced_frame_par;
extern z80_byte interlaced_numero_frame;

extern z80_bit video_interlaced_mode;

extern z80_bit video_interlaced_scanlines;

extern z80_bit gigascreen_enabled;

extern z80_bit pentagon_16c_mode_available;

extern int screen_mode_16c_is_enabled(void);
extern void enable_16c_mode(void);
extern void disable_16c_mode(void);


//Tamanyos pantalla para zxspectrum. Usados en funciones de repintado
#define LEFT_BORDER_NO_ZOOM 48
#define RIGHT_BORDER_NO_ZOOM 48

//hay 8 pixeles arriba que no son visibles
#define TOP_BORDER_NO_ZOOM 56
#define ZX8081ACE_TOP_BORDER_NO_ZOOM 48

#define BOTTOM_BORDER_NO_ZOOM 56

/*


<-96->  <-----48----->       <-256->           <-----48----->  ^
                                                               | 8
        -----------------------------------------------------  v
        -                                                   -  ^ 56
        -                                                   -  |
        -            --------------------------             -  v
        -            -                        -             -
        -            -                        -             -
        -            -                        -             -
        -            -                        -             -
        -            -                        -             -
        -            -                        -             -
        -            -                        -             -
        -            -                        -             -
        -            -                        -             -
        -            --------------------------             -  ^
        -                                                   -  |
        -                                                   -  |  56
        -----------------------------------------------------  v


*/


#define LEFT_BORDER LEFT_BORDER_NO_ZOOM*zoom_x

#define TOP_BORDER TOP_BORDER_NO_ZOOM*zoom_y

#define ZX8081ACE_TOP_BORDER ZX8081ACE_TOP_BORDER_NO_ZOOM*zoom_y

#define BOTTOM_BORDER BOTTOM_BORDER_NO_ZOOM*zoom_y




#define ANCHO_PANTALLA 256
#define ALTO_PANTALLA 192
#define DISPLAY_SCREEN_HEIGHT ALTO_PANTALLA


#define SCREEN_Z88_WIDTH (640)
//#define SCREEN_Z88_HEIGHT (ALTO_PANTALLA+TOP_BORDER_NO_ZOOM+BOTTOM_BORDER_NO_ZOOM)
#define SCREEN_Z88_HEIGHT (192)
//#define SCREEN_SPECTRUM_WIDTH (ANCHO_PANTALLA+LEFT_BORDER_NO_ZOOM*2*border_enabled.v)
//#define SCREEN_SPECTRUM_HEIGHT (ALTO_PANTALLA+TOP_BORDER_NO_ZOOM*border_enabled.v+BOTTOM_BORDER_NO_ZOOM*border_enabled.v)

#define SCREEN_SPECTRUM_WIDTH (ANCHO_PANTALLA+LEFT_BORDER_NO_ZOOM+RIGHT_BORDER_NO_ZOOM)
#define SCREEN_SPECTRUM_HEIGHT (ALTO_PANTALLA+TOP_BORDER_NO_ZOOM+BOTTOM_BORDER_NO_ZOOM)

//Valores usados en real video

extern z80_bit rainbow_enabled;
extern z80_bit autodetect_rainbow;

extern z80_int *new_scalled_rainbow_buffer;

extern void screen_scale_075_function(int ancho,int alto);

extern z80_bit screen_reduce_075_antialias;


//normalmente a 8
extern int screen_invisible_borde_superior;
//normalmente a 56.
extern int screen_borde_superior;

//estos dos anteriores se suman aqui. es 64 en 48k, y 63 en 128k. por tanto, uno de los dos valores vale 1 menos
extern int screen_total_borde_superior;

//normalmente a 56
extern int screen_total_borde_inferior;

//normalmente a 48
extern int screen_total_borde_izquierdo;

extern int screen_testados_total_borde_izquierdo;

//normalmente a 96
extern int screen_invisible_borde_derecho;

//normalmente a 48
extern int screen_total_borde_derecho;

//estos dos anteriores se suman aqui. es 64 en 48k, y 63 en 128k. por tanto, uno de los dos valores vale 1 menos
extern int screen_indice_inicio_pant;

//suma del anterior+192
extern int screen_indice_fin_pant;

extern int screen_testados_indice_borde_derecho;

extern void screen_set_video_params_indices(void);

extern void init_rainbow(void);

//extern int get_total_ancho_rainbow(void);
//extern int get_total_alto_rainbow(void);

extern z80_int *rainbow_buffer;

extern int screen_scanlines;

//Total de t_estados por linea
extern int screen_testados_linea;

extern int screen_testados_total;

//extern void screen_store_scanline_rainbow(void);
extern void screen_store_scanline_rainbow_solo_display(void);
extern void screen_store_scanline_rainbow_solo_border(void);


//256 es el maximo de estados en una linea (caso de cpc)
//525 es el maximo de lineas en un frame (caso de Prism)

//
//Valores maximos
//
#define MAX_STATES_LINE (256*MAX_CPU_TURBO_SPEED)

//Prism es la que tiene mas lineas
//#define FULLBORDER_ARRAY_LENGTH (525*MAX_STATES_LINE)

//Total maximo para un frame
#define MAX_FULLBORDER_ARRAY_LENGTH (525*MAX_STATES_LINE)

//
//Valores actuales
//

//Maximo de estados en linea de cualquier maquina pero teniendo en cuenta velocidad turbo actual
#define MAX_CURRENT_STATES_LINE (256*cpu_turbo_speed)
//Total en un frame actual
#define CURRENT_FULLBORDER_ARRAY_LENGTH (525*MAX_CURRENT_STATES_LINE)


extern z80_byte fullbuffer_border[];


//lo que ocupa el buffer de scanline (pixel+atributo) multiplicado por 2 y sin tener en cuenta modos turbo
//#define ATRIBUTOS_ARRAY_LENGTH ((32+32)*2)
#define SCANLINEBUFFER_ONE_ARRAY_LENGTH ((32+32)*2)
extern z80_byte scanline_buffer[];

extern int last_x_atributo;

extern void clear_putpixel_cache(void);

extern void init_cache_putpixel(void);

//extern void screen_store_scanline_char_zx8081(int x,int y,z80_byte byte_leido);
extern void screen_store_scanline_char_zx8081(int x,int y,z80_byte byte_leido,z80_byte caracter,int inverse);
extern void screen_store_scanline_char_zx8081_border_scanline(int x,int y,z80_byte byte_leido);

extern void siguiente_frame_pantalla(void);


extern int framedrop_total;
extern int frames_total;
extern int framescreen_saltar;
extern int next_frame_skip_render_scanlines;
extern int ultimo_fps;

//vofile
extern char *vofilename;
extern void init_vofile(void);
extern void close_vofile(void);
extern void vofile_send_frame(z80_int *buffer);
extern z80_bit vofile_inserted;

extern int vofile_fps;
extern int vofile_frame_actual;
extern void print_helper_aofile_vofile(void);

extern void screen_init_colour_table(void);

extern void scr_fadeout(void);

extern void screen_reset_scr_driver_params(void);

extern int scr_si_color_oscuro(void);

extern int frameskip;
extern int frameskip_counter;
extern z80_bit autoframeskip;

extern void cpu_loop_refresca_pantalla(void);

extern int si_complete_video_driver(void);
extern int si_normal_menu_video_driver(void);

extern void screen_print(int x,int y,int tinta,int papel,char *mensaje);

extern int screen_print_y;

extern void set_putpixel_zoom(void);

extern void enable_rainbow(void);
extern void disable_rainbow(void);

extern void recalcular_get_total_ancho_rainbow(void);
extern void recalcular_get_total_alto_rainbow(void);
extern void enable_border(void);
extern void disable_border(void);


extern int get_total_ancho_rainbow_cached;
extern int get_total_alto_rainbow_cached;

//Variables cacheadas para que sea mas rapido
#define get_total_ancho_rainbow(x) (get_total_ancho_rainbow_cached)
#define get_total_alto_rainbow(x) (get_total_alto_rainbow_cached)


extern void set_t_scanline_draw_zero(void);
extern void t_scanline_next_line(void);

extern void screen_z88_refresca_pantalla(void);

extern void screen_z88_refresca_pantalla_comun(void);

extern void screen_z88_draw_lower_screen(void);

extern void scr_refresca_pantalla_y_border_ace(void);

extern void screen_text_repinta_pantalla_ace(void);

extern void scr_refresca_pantalla_ace(void);

extern void screen_text_repinta_pantalla_chloe(void);

extern void screen_tbblue_refresca_pantalla(void);

//extern void z88_return_character_atributes(z80_byte *sbr,int *ascii_caracter,int *ancho,int *inverse,int *subrallado,int *parpadeo,int *gris,int *null_caracter);


//Estructura usada para funcion de conversion de caracter z88, y para funcion de bucle de refresco de pantalla de z88 en ascii
struct s_z88_return_character_atributes
{
	//z80_byte *sbr;
	z88_dir sbr;
	int ascii_caracter;
	int ancho;
	int inverse;
	int subrallado;
	int parpadeo;
	int gris;
	int null_caracter;

	//Estos siguientes solo usados en funcion de bucle de refresco de pantalla
	int x,y;
	void (*f_print_char)(struct s_z88_return_character_atributes *s);
	void (*f_new_line)(struct s_z88_return_character_atributes *s);


	//int temp_orig;
};

extern void z88_return_character_atributes(struct s_z88_return_character_atributes *z88_caracter);

extern void screen_repinta_pantalla_z88(struct s_z88_return_character_atributes *z88_caracter);

extern int screen_get_emulated_display_width_no_zoom(void);
extern int screen_get_emulated_display_height_no_zoom(void);
extern int screen_get_emulated_display_width_zoom(void);
extern int screen_get_emulated_display_height_zoom(void);

extern int screen_get_emulated_display_width_no_zoom_border_en(void);
extern int screen_get_emulated_display_height_no_zoom_border_en(void);
extern int screen_get_emulated_display_width_zoom_border_en(void);
extern int screen_get_emulated_display_height_zoom_border_en(void);

extern int screen_get_emulated_display_height_no_zoom_bottomborder_en(void);

extern int screen_get_window_size_height_zoom_border_en(void);
extern int screen_get_window_size_width_zoom_border_en(void);

extern int screen_get_window_size_height_no_zoom_border_en(void);
extern int screen_get_window_size_width_no_zoom_border_en(void);

extern int screen_get_window_size_height_no_zoom(void);
extern int screen_get_window_size_width_no_zoom(void);

extern void disable_interlace(void);
extern void enable_interlace(void);
extern void disable_scanlines(void);
extern void enable_scanlines(void);

extern void all_interlace_scr_refresca_pantalla(void);

extern int get_gigascreen_color(int c0,int c1);

extern int get_gigascreen_rgb_value(int c0,int c1);

extern void disable_gigascreen(void);
extern void enable_gigascreen(void);

extern void screen_switch_rainbow_buffer(void);

#define SPECCY_TOTAL_PALETTE_COLOURS 16
#define SPECCY_GREY_SCANLINE_TOTAL_PALETTE_COLOURS 16



#define GIGASCREEN_TOTAL_PALETTE_COLOURS 256

#define Z88_PXCOLON (SPECCY_TOTAL_PALETTE_COLOURS+SPECCY_GREY_SCANLINE_TOTAL_PALETTE_COLOURS+GIGASCREEN_TOTAL_PALETTE_COLOURS+0)
#define Z88_PXCOLGREY (SPECCY_TOTAL_PALETTE_COLOURS+SPECCY_GREY_SCANLINE_TOTAL_PALETTE_COLOURS+GIGASCREEN_TOTAL_PALETTE_COLOURS+1)
#define Z88_PXCOLOFF (SPECCY_TOTAL_PALETTE_COLOURS+SPECCY_GREY_SCANLINE_TOTAL_PALETTE_COLOURS+GIGASCREEN_TOTAL_PALETTE_COLOURS+2)
#define Z88_PXCOLSCROFF (SPECCY_TOTAL_PALETTE_COLOURS+SPECCY_GREY_SCANLINE_TOTAL_PALETTE_COLOURS+GIGASCREEN_TOTAL_PALETTE_COLOURS+3)

#define Z88_TOTAL_PALETTE_COLOURS 4

//#define SPECCY_1648_REAL_PALETTE_FIRST_COLOR (Z88_PXCOLSCROFF+1)
//#define SPECCY_1648_REAL_PALETTE_COLOURS 16

#define ULAPLUS_INDEX_FIRST_COLOR (Z88_PXCOLSCROFF+1)
//#define ULAPLUS_INDEX_FIRST_COLOR (SPECCY_1648_REAL_PALETTE_FIRST_COLOR+SPECCY_1648_REAL_PALETTE_COLOURS)
#define ULAPLUS_TOTAL_PALETTE_COLOURS 256

//Sumamos los 256 de la tabla rgb de ULAPLUS
#define SPECTRA_INDEX_FIRST_COLOR (ULAPLUS_INDEX_FIRST_COLOR+ULAPLUS_TOTAL_PALETTE_COLOURS)
#define SPECTRA_TOTAL_PALETTE_COLOURS 64

#define CPC_INDEX_FIRST_COLOR (SPECTRA_INDEX_FIRST_COLOR+SPECTRA_TOTAL_PALETTE_COLOURS)
#define CPC_TOTAL_PALETTE_COLOURS 32

#define PRISM_INDEX_FIRST_COLOR (CPC_INDEX_FIRST_COLOR+CPC_TOTAL_PALETTE_COLOURS)
#define PRISM_TOTAL_PALETTE_COLOURS 4096

#define SAM_INDEX_FIRST_COLOR (PRISM_INDEX_FIRST_COLOR+PRISM_TOTAL_PALETTE_COLOURS)
#define SAM_TOTAL_PALETTE_COLOURS 128

#define RGB9_INDEX_FIRST_COLOR (SAM_INDEX_FIRST_COLOR+SAM_TOTAL_PALETTE_COLOURS)
#define RGB9_TOTAL_PALETTE_COLOURS 512

#define TSCONF_INDEX_FIRST_COLOR (RGB9_INDEX_FIRST_COLOR+RGB9_TOTAL_PALETTE_COLOURS)
#define TSCONF_TOTAL_PALETTE_COLOURS 32768

#define HEATMAP_INDEX_FIRST_COLOR (TSCONF_INDEX_FIRST_COLOR+TSCONF_TOTAL_PALETTE_COLOURS)
#define HEATMAP_TOTAL_PALETTE_COLOURS 256

#define SOLARIZED_INDEX_FIRST_COLOR (HEATMAP_INDEX_FIRST_COLOR+HEATMAP_TOTAL_PALETTE_COLOURS)
#define SOLARIZED_TOTAL_PALETTE_COLOURS 16


#define SOLARIZED_base03 (SOLARIZED_INDEX_FIRST_COLOR+0)


#define SOLARIZED_COLOUR_base03  (SOLARIZED_INDEX_FIRST_COLOR+0)
#define SOLARIZED_COLOUR_base02  (SOLARIZED_INDEX_FIRST_COLOR+1)
#define SOLARIZED_COLOUR_base01  (SOLARIZED_INDEX_FIRST_COLOR+2)
#define SOLARIZED_COLOUR_base00  (SOLARIZED_INDEX_FIRST_COLOR+3)
#define SOLARIZED_COLOUR_base0   (SOLARIZED_INDEX_FIRST_COLOR+4)
#define SOLARIZED_COLOUR_base1   (SOLARIZED_INDEX_FIRST_COLOR+5)
#define SOLARIZED_COLOUR_base2   (SOLARIZED_INDEX_FIRST_COLOR+6)
#define SOLARIZED_COLOUR_base3   (SOLARIZED_INDEX_FIRST_COLOR+7)
#define SOLARIZED_COLOUR_yellow  (SOLARIZED_INDEX_FIRST_COLOR+8)
#define SOLARIZED_COLOUR_orange  (SOLARIZED_INDEX_FIRST_COLOR+9)
#define SOLARIZED_COLOUR_red     (SOLARIZED_INDEX_FIRST_COLOR+10)
#define SOLARIZED_COLOUR_magenta (SOLARIZED_INDEX_FIRST_COLOR+11)
#define SOLARIZED_COLOUR_violet  (SOLARIZED_INDEX_FIRST_COLOR+12)
#define SOLARIZED_COLOUR_blue    (SOLARIZED_INDEX_FIRST_COLOR+13)
#define SOLARIZED_COLOUR_cyan    (SOLARIZED_INDEX_FIRST_COLOR+14)
#define SOLARIZED_COLOUR_green   (SOLARIZED_INDEX_FIRST_COLOR+15)


//Paleta para carga de bmp indexado
#define BMP_INDEX_FIRST_COLOR (SOLARIZED_INDEX_FIRST_COLOR+SOLARIZED_TOTAL_PALETTE_COLOURS)
#define BMP_TOTAL_PALETTE_COLOURS 256


//Paleta para chip vdp de MSX
#define VDP_9918_INDEX_FIRST_COLOR (BMP_INDEX_FIRST_COLOR+BMP_TOTAL_PALETTE_COLOURS)
#define VDP_9918_TOTAL_PALETTE_COLOURS 16

//Paleta para QL
#define QL_INDEX_FIRST_COLOR (VDP_9918_INDEX_FIRST_COLOR+VDP_9918_TOTAL_PALETTE_COLOURS)
#define QL_TOTAL_PALETTE_COLOURS 8

#define TURBOVISION_INDEX_FIRST_COLOR (QL_INDEX_FIRST_COLOR+QL_TOTAL_PALETTE_COLOURS)
#define TURBOVISION_TOTAL_PALETTE_COLOURS 16

#define TURBOVISION_COLOUR_black (TURBOVISION_INDEX_FIRST_COLOR+0)
#define TURBOVISION_COLOUR_red (TURBOVISION_INDEX_FIRST_COLOR+1)
#define TURBOVISION_COLOUR_green (TURBOVISION_INDEX_FIRST_COLOR+2)
#define TURBOVISION_COLOUR_blue (TURBOVISION_INDEX_FIRST_COLOR+4)
#define TURBOVISION_COLOUR_grey (TURBOVISION_INDEX_FIRST_COLOR+8)
#define TURBOVISION_COLOUR_cyan (TURBOVISION_INDEX_FIRST_COLOR+6)
#define TURBOVISION_COLOUR_white (TURBOVISION_INDEX_FIRST_COLOR+7)
#define TURBOVISION_COLOUR_lightwhite (TURBOVISION_INDEX_FIRST_COLOR+15)

#define SMS_INDEX_FIRST_COLOR (TURBOVISION_INDEX_FIRST_COLOR+TURBOVISION_TOTAL_PALETTE_COLOURS)
#define SMS_TOTAL_PALETTE_COLOURS 64

#define BEOS_INDEX_FIRST_COLOR (SMS_INDEX_FIRST_COLOR+SMS_TOTAL_PALETTE_COLOURS)
#define BEOS_TOTAL_PALETTE_COLOURS 6

#define BEOS_COLOUR_yellow (BEOS_INDEX_FIRST_COLOR+0)
#define BEOS_COLOUR_grey_box (BEOS_INDEX_FIRST_COLOR+1)
#define BEOS_COLOUR_grey_menu (BEOS_INDEX_FIRST_COLOR+2)
#define BEOS_COLOUR_grey_selection (BEOS_INDEX_FIRST_COLOR+3)
#define BEOS_COLOUR_grey_inactive_title (BEOS_INDEX_FIRST_COLOR+4)
#define BEOS_COLOUR_blue_hotkey (BEOS_INDEX_FIRST_COLOR+5)


#define RETROMAC_INDEX_FIRST_COLOR (BEOS_INDEX_FIRST_COLOR+BEOS_TOTAL_PALETTE_COLOURS)
#define RETROMAC_TOTAL_PALETTE_COLOURS 5

#define RETROMAC_COLOUR_active_title (RETROMAC_INDEX_FIRST_COLOR+0)
#define RETROMAC_COLOUR_paper (RETROMAC_INDEX_FIRST_COLOR+1)
#define RETROMAC_COLOUR_selected_paper (RETROMAC_INDEX_FIRST_COLOR+2)
#define RETROMAC_COLOUR_unavailable_ink (RETROMAC_INDEX_FIRST_COLOR+3)
#define RETROMAC_COLOUR_window_box (RETROMAC_INDEX_FIRST_COLOR+4)


#define AMIGAOS_INDEX_FIRST_COLOR (RETROMAC_INDEX_FIRST_COLOR+RETROMAC_TOTAL_PALETTE_COLOURS)
#define AMIGAOS_TOTAL_PALETTE_COLOURS 4

#define AMIGAOS_COLOUR_blue (AMIGAOS_INDEX_FIRST_COLOR+0)
#define AMIGAOS_COLOUR_inactive_title_ink (AMIGAOS_INDEX_FIRST_COLOR+1)
#define AMIGAOS_COLOUR_orange (AMIGAOS_INDEX_FIRST_COLOR+2)
#define AMIGAOS_COLOUR_red (AMIGAOS_INDEX_FIRST_COLOR+3)


#define ATARITOS_INDEX_FIRST_COLOR (AMIGAOS_INDEX_FIRST_COLOR+AMIGAOS_TOTAL_PALETTE_COLOURS)
#define ATARITOS_TOTAL_PALETTE_COLOURS 2

#define ATARITOS_COLOUR_green (ATARITOS_INDEX_FIRST_COLOR+0)
#define ATARITOS_COLOUR_white (ATARITOS_INDEX_FIRST_COLOR+1)


#define OSDOS_INDEX_FIRST_COLOR (ATARITOS_INDEX_FIRST_COLOR+ATARITOS_TOTAL_PALETTE_COLOURS)
#define OSDOS_TOTAL_PALETTE_COLOURS 3

#define OSDOS_COLOUR_BLUE (OSDOS_INDEX_FIRST_COLOR+0)
#define OSDOS_COLOUR_GRAY (OSDOS_INDEX_FIRST_COLOR+1)
#define OSDOS_COLOUR_GRAY_INACTIVE (OSDOS_INDEX_FIRST_COLOR+2)


#define ZESARUX_PLUS_INDEX_FIRST_COLOR (OSDOS_INDEX_FIRST_COLOR+OSDOS_TOTAL_PALETTE_COLOURS)
#define ZESARUX_PLUS_TOTAL_PALETTE_COLOURS 1

#define ZESARUX_PLUS_COLOUR_WHITE (ZESARUX_PLUS_INDEX_FIRST_COLOR+0)


#define RISCOS_INDEX_FIRST_COLOR (ZESARUX_PLUS_INDEX_FIRST_COLOR+ZESARUX_PLUS_TOTAL_PALETTE_COLOURS)
#define RISCOS_TOTAL_PALETTE_COLOURS 4

#define RISCOS_COLOUR_DARKBLUE  (RISCOS_INDEX_FIRST_COLOR+0)
#define RISCOS_COLOUR_ORANGE    (RISCOS_INDEX_FIRST_COLOR+1)
#define RISCOS_COLOUR_LIGHTBLUE (RISCOS_INDEX_FIRST_COLOR+2)
#define RISCOS_COLOUR_RED       (RISCOS_INDEX_FIRST_COLOR+3)

//16 colores normales spectrum, 16 grises de modo scanline, 256 de gigascreen, 4 de z88, 16 de spectrum 17/48/+ real, 256 de ulaplus, 64 de spectra, 32 de CPC, 4096 de Prism, 128 de SAM, 256 de RGB8, 32768 de TSCONF, 16 de solarized
//actualizar aqui y tambien estructura de total_palette_colours_array y #define TOTAL_PALETAS_COLORES 
#define EMULATOR_TOTAL_PALETTE_COLOURS (SPECCY_TOTAL_PALETTE_COLOURS+SPECCY_GREY_SCANLINE_TOTAL_PALETTE_COLOURS+GIGASCREEN_TOTAL_PALETTE_COLOURS+Z88_TOTAL_PALETTE_COLOURS+ULAPLUS_TOTAL_PALETTE_COLOURS+SPECTRA_TOTAL_PALETTE_COLOURS+CPC_TOTAL_PALETTE_COLOURS+PRISM_TOTAL_PALETTE_COLOURS+SAM_TOTAL_PALETTE_COLOURS+RGB9_TOTAL_PALETTE_COLOURS+TSCONF_TOTAL_PALETTE_COLOURS+HEATMAP_TOTAL_PALETTE_COLOURS+SOLARIZED_TOTAL_PALETTE_COLOURS+BMP_TOTAL_PALETTE_COLOURS+VDP_9918_TOTAL_PALETTE_COLOURS+QL_TOTAL_PALETTE_COLOURS+TURBOVISION_TOTAL_PALETTE_COLOURS+SMS_TOTAL_PALETTE_COLOURS+BEOS_TOTAL_PALETTE_COLOURS+RETROMAC_TOTAL_PALETTE_COLOURS+AMIGAOS_TOTAL_PALETTE_COLOURS+ATARITOS_TOTAL_PALETTE_COLOURS+OSDOS_TOTAL_PALETTE_COLOURS+ZESARUX_PLUS_TOTAL_PALETTE_COLOURS+RISCOS_TOTAL_PALETTE_COLOURS)


struct s_total_palette_colours {
        char nombre_paleta[32];
				char descripcion_paleta[32];
				int indice_inicial;
				int total_colores;
};

typedef struct s_total_palette_colours total_palette_colours;

//Esto usado en menu display->ver paleta total
#define TOTAL_PALETAS_COLORES 14

extern total_palette_colours total_palette_colours_array[];

//macro usado en paletas de colores (opciones --red, --green, --blue)
#define VALOR_GRIS_A_R_G_B \
                                if (screen_gray_mode & 1) b=valorgris; \
                                else b=0; \
				\
                                if (screen_gray_mode & 2) g=valorgris; \
                                else g=0; \
				\
                                if (screen_gray_mode & 4) r=valorgris; \
                                else r=0; \

extern void screen_get_sprite_char(int x,int y,z80_byte *caracter);
extern void screen_set_colour_normal(int index, int colour);

extern void screen_reset_putpixel_maxmin_y(void);
extern int putpixel_max_y, putpixel_min_y;


extern z80_bit screen_show_splash_texts;
extern z80_bit screen_show_cpu_usage;
extern z80_bit screen_show_cpu_temp;
extern z80_bit screen_show_fps;
extern void screen_set_parameters_slow_machines(void);

extern z80_bit mouse_pointer_shown;

extern z80_int screen_border_last_color;

extern void set_z88_putpixel_zoom_function(void);

extern int screen_text_accept_ansi;

extern void screen_text_set_normal_text(void);

extern void screen_text_send_ansi_go_home(void);
extern void screen_text_repinta_pantalla_z88_print_char(struct s_z88_return_character_atributes *z88_caracter);
extern char screen_text_return_color_border(void);

extern void screen_text_borde_horizontal_zx8081(void);
extern void screen_text_borde_vertical_zx8081(void);
extern void screen_text_borde_horizontal(void);
extern void screen_text_borde_vertical(void);

extern void screen_text_repinta_pantalla_zx81_rainbow(void);
extern void screen_text_repinta_pantalla_zx81_no_rainbow(void);
extern void screen_text_repinta_pantalla_spectrum(void);
extern void screen_text_repinta_pantalla_z88_new_line(struct s_z88_return_character_atributes *z88_caracter GCC_UNUSED);
extern void screen_text_repinta_pantalla_z88(void);
extern void screen_text_repinta_pantalla_zx81(void);
extern void screen_text_printchar_next(z80_byte caracter, void (*puntero_printchar_caracter) (z80_byte)  );
extern void screen_text_printchar(void (*puntero_printchar_caracter) (z80_byte) );

//Veces dentro de un frame en que se considera que hay un posible efecto rainbow
//A partir de 2. En aqua plane hay dos cambios de border por cada frame
#define DETECT_RAINBOW_BORDER_MAX_IN_FRAMES 2

//Cuando se repite lo anterior en este numero de frames, se activa realvideo
//1/5 de segundo
#define DETECT_RAINBOW_BORDER_TOTAL_FRAMES 10


extern int detect_rainbow_border_changes_in_frame;
extern int detect_rainbow_border_total_frames;

extern void screen_text_repinta_pantalla_spectrum_comun(int si_border,void (*puntero_printchar_caracter) (z80_byte),int solo_texto);
extern void screen_text_repinta_pantalla_spectrum_comun_addr(int si_border,void (*puntero_printchar_caracter) (z80_byte),int no_ansi,int solo_texto,z80_byte *scrscreen_text_screen);
extern void screen_text_repinta_pantalla_zx81_comun(int si_border,void (*puntero_printchar_caracter) (z80_byte),int solo_texto );

extern int snow_effect_min_value;

extern char last_message_helper_aofile_vofile_file_format[];
extern char last_message_helper_aofile_vofile_util[];
extern char last_message_helper_aofile_vofile_bytes_minute_audio[];
extern char last_message_helper_aofile_vofile_bytes_minute_video[];

extern void disable_timex_video(void);
extern void enable_timex_video(void);

extern void scr_refresca_pantalla_cpc(void);

extern void scr_refresca_pantalla_y_border_sam(void);

extern z80_byte sam_retorna_byte_pantalla_mode1(z80_byte *s,z80_int *o,z80_byte *at);

extern z80_byte sam_retorna_byte_pantalla(z80_byte *s,z80_int *o);

extern void sam_convert_mode3_char_to_bw(z80_byte *origen,z80_byte *buffer_letra,z80_byte *atributo);
extern void sam_convert_mode2_char_to_bw(z80_byte *origen,z80_byte *buffer_letra,z80_byte *atributo,int bit);

extern void scr_refresca_pantalla_sam_modo_013(int modo,void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) );

extern void scr_refresca_pantalla_sam_modo_2(void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo,
 unsigned char inv,z80_byte caracter ));

extern void screen_text_repinta_pantalla_sam(void);

extern char scr_artistic_retorna_artistic_char(z80_byte *origen, int incremento_origen);

extern void scr_refresca_pantalla_cpc_text(void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) , void (*fun_saltolinea) (void) );

extern void screen_text_repinta_pantalla_cpc(void);

extern z80_bit scr_refresca_sin_colores;

extern void scr_refresca_pantalla_y_border_ql(void);

extern z80_bit no_fadeout_exit;

extern void screen_set_window_zoom(int z);

//extern int get_rgb8_color (z80_byte color);
extern int get_rgb9_color (z80_int color);

extern void screen_tsconf_refresca_pantalla(void);



extern z80_bit zxuno_tbblue_disparada_raster;
extern z80_byte get_zxuno_tbblue_rasterctrl(void);
extern void zxuno_tbblue_handle_raster_interrupts();

extern int generic_footertext_operating_counter;
extern void generic_footertext_print_operating(char *s);
extern void delete_generic_footertext(void);

extern int rgb_to_grey(int r,int g,int b);



/*
Macro de obtencion de color de un pixel
Nota: Quiza el if (ulaplus_presente.v && ulaplus_enabled.v) consume algunos ciclos de cpu,
y se podria haber evitado, haciendo rutina de screen_store_scanline_rainbow_solo_display solo para ulaplus,
con lo cual el if se evitaria y ahorrariamos ciclos de cpu...
Esto a la practica no ahorra cpu apreciable, y ademas, se tendria
otra rutina de screen_store_scanline_rainbow_solo_display diferente de la que no tiene ulaplus,
agregando duplicidad de funciones sin verdadera necesidad...
*/
#define GET_PIXEL_COLOR \
            if (ulaplus_presente.v && ulaplus_enabled.v) {  \
                GET_PIXEL_ULAPLUS_COLOR \
            } \
            else { \
                        ink=attribute &7; \
                        paper=(attribute>>3) &7; \
                        bright=(attribute)&64; \
                        flash=(attribute)&128; \
                        if (flash) { \
                                if (estado_parpadeo.v) { \
                                        aux=paper; \
                                        paper=ink; \
                                        ink=aux; \
                                } \
                        } \
            \
            if (bright) {   \
                paper+=8; \
                ink+=8; \
            } \
            } \





//Macro de obtencion de color de un pixel con ulaplus
#define GET_PIXEL_ULAPLUS_COLOR \
                        ink=attribute &7; \
                        paper=(attribute>>3) &7; \
                        bright=( (attribute)&64 ? 1 : 0 )  ; \
                        flash=( (attribute)&128 ? 1 : 0 ) ; \
            \
            z80_int temp_color=(flash * 2 + bright) * 16; \
            ink=temp_color+ink; \
            ink=ulaplus_palette_table[ink]+ ULAPLUS_INDEX_FIRST_COLOR ; \
            paper=temp_color+paper+8;  \
            paper=ulaplus_palette_table[paper]+ ULAPLUS_INDEX_FIRST_COLOR ; \




//Funcion normal
#define store_value_rainbow(p,x) *p++=x;

//Funcion con debug
//#define store_value_rainbow(p,x) store_value_rainbow_debug(&p,x);            


extern z80_bit spectrum_1648_use_real_palette;

extern int scr_ver_si_refrescar_por_menu_activo(int x,int fila);

extern void tsconf_fast_tilesprite_render(void);

extern int screen_text_brightness;


extern void scr_refresca_pantalla_tsconf_text(void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) , void (*fun_saltolinea) (void) , int factor_division);


extern void scr_refresca_pantalla_tsconf_text_textmode (void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) , void (*fun_saltolinea) (void) , int factor_division);

extern z80_bit screen_reduce_075;

extern int screen_reduce_offset_x;
extern int screen_reduce_offset_y;

extern int screen_watermark_position;


extern z80_bit screen_watermark_enabled;

extern void screen_put_watermark_generic(z80_int *destino,int x,int y,int ancho, void (*putpixel) (z80_int *destino,int x,int y,int ancho,int color) );
extern void screen_put_asciibitmap_generic(char **origen,z80_int *destino,int x,int y,int ancho_orig, int alto_orig, int ancho_destino, void (*putpixel) (z80_int *destino,int x,int y,int ancho_destino,int color),int zoom,int inverso );
extern void screen_put_asciibitmap_generic_offset_inicio(char **origen,z80_int *destino,int x,int y,int ancho_orig, int alto_orig, int ancho_destino, void (*putpixel) (z80_int *destino,int x,int y,int ancho_destino,int color),int zoom,int inverso,int offset_inicio_agregar );


#define ZESARUX_WATERMARK_LOGO_MARGIN 4

extern int screen_colores_rainbow[];
extern int screen_colores_rainbow_nobrillo[];


extern int scrstdout_simpletext_refresh_factor;

extern char screen_common_caracteres_artisticos[];

extern void scr_set_fps_stdout_simpletext(int fps);

extern int screen_if_refresh(void);

extern void screen_text_set_ansi_color_bg(z80_byte paper);

extern void screen_text_set_ansi_color_fg(z80_byte ink);

extern void screen_generic_putpixel_indexcolour(z80_int *destino,int x,int y,int ancho,int color);

extern int screen_generic_getpixel_indexcolour(z80_int *destino,int x,int y,int ancho);

extern int screen_force_refresh;

extern int screen_get_x_coordinate_tstates(int *si_salta_linea);

extern int screen_get_y_coordinate_tstates(void);

extern int screen_init_pantalla_and_others(void);
extern void screen_init_pantalla_and_others_and_realjoystick(void);

extern char *get_spectrum_ula_string_video_mode(void);

extern int screen_ega_to_spectrum_colour(int ega_col);

extern void screen_putpixel_mix_layers(int x,int y);

#define MAX_MENU_MIX_METHODS 3

extern char *screen_menu_mix_methods_strings[];

extern int screen_menu_mix_method;
extern int screen_menu_mix_transparency; 
extern z80_bit screen_menu_reduce_bright_machine;
extern void scr_clear_layer_menu(void);
extern z80_bit screen_machine_bw_no_multitask;

extern z80_int *buffer_layer_machine;
extern z80_int *buffer_layer_menu;

extern int ancho_layer_menu_machine;
extern int alto_layer_menu_machine;	

extern void scr_reallocate_layers_menu(int ancho,int alto);
extern void scr_init_layers_menu(void);

extern int sem_screen_refresh_reallocate_layers;



extern void screen_end_pantalla_save_overlay(void (**previous_function)(void),int *menu_antes );
extern void screen_restart_pantalla_restore_overlay(void (*previous_function)(void),int menu_antes);
extern void screen_render_bmpfile(z80_byte *mem,int indice_paleta_color,zxvision_window *ventana,int x_ignore,int follow_zoom,int ancho_mostrar,int indice_color_transparente,int color_final_transparente);

extern void screen_render_menu_overlay_if_active(void);

#define SCREEN_LAYER_TRANSPARENT_MENU 65535

			


#endif
