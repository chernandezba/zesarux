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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "screen.h"
#include "debug.h"
#include "mem128.h"
#include "operaciones.h"
#include "zx8081.h"
#include "charset.h"
#include "zxvision.h"
#include "menu_bitmaps.h"
#include "menu_items.h"
#include "audio.h"
#include "contend.h"
#include "ula.h"
#include "tape_smp.h"
#include "z88.h"
#include "ulaplus.h"
#include "zxuno.h"
#include "spectra.h"
#include "spritechip.h"
#include "timex.h"
#include "chloe.h"
#include "cpc.h"
#include "prism.h"
#include "sam.h"
#include "ql.h"
#include "timer.h"
#include "tbblue.h"
#include "tsconf.h"
#include "mk14.h"
#include "settings.h"
#include "vdp_9918a.h"
#include "msx.h"
#include "coleco.h"
#include "ql_zx8302.h"
#include "stats.h"

//Incluimos estos dos para la funcion de fade out
#ifdef COMPILE_XWINDOWS
	#include "scrxwindows.h"
#endif

#ifdef COMPILE_AA
	#include "scraa.h"
#endif

#ifdef COMPILE_CURSES
	#include "scrcurses.h"
#endif


//Tabla que indica cada direccion de pantalla para cada coordenada
z80_int *screen_addr_table;

//ultima posicion y para funcion screen_print
int screen_print_y=0;

//Indica si terminal soporta codigos ANSI o no
int screen_text_accept_ansi=0;

//contraste para renderizados de modo texto. 0=todo negro, 100=todo blanco
int screen_text_brightness=50;

//si se muestran determinados mensajes en splash, como los de cambio de modo de video
//no confundir con el mensaje de bienvenida
z80_bit screen_show_splash_texts={1};

//mostrar uso de cpu en footer
z80_bit screen_show_cpu_usage={1};

//mostrar temperatura de cpu en footer
z80_bit screen_show_cpu_temp={1};

//mostrar fps en footer
z80_bit screen_show_fps={1};

//Si pantalla final rainbow se reduce tamanyo a 4/3 (dividir por 4, mult por 3)
z80_bit screen_reduce_075={0};

//Antialias al reducir
z80_bit screen_reduce_075_antialias={1};

int screen_reduce_offset_x=0;
int screen_reduce_offset_y=0;



z80_bit ocr_settings_not_look_23606={0};

//Rutinas de pantalla
void (*scr_refresca_pantalla) (void);
void (*scr_refresca_pantalla_solo_driver) (void);
void (*scr_set_fullscreen) (void);
void (*scr_reset_fullscreen) (void);
int ventana_fullscreen=0;
int (*scr_init_pantalla) (void);
void (*scr_end_pantalla) (void);
z80_byte (*scr_lee_puerto) (z80_byte puerto_h,z80_byte puerto_l);
void (*scr_actualiza_tablas_teclado) (void);

void (*scr_putpixel_zoom) (int x,int y,unsigned int color);
void (*scr_putpixel_zoom_rainbow)(int x,int y,unsigned int color);

void (*scr_putpixel) (int x,int y,unsigned int color);
void (*scr_putpixel_final_rgb) (int x,int y,unsigned int color_rgb);
void (*scr_putpixel_final) (int x,int y,unsigned int color);

int (*scr_get_menu_width) (void);
int (*scr_get_menu_height) (void);

int (*scr_driver_can_ext_desktop) (void);


void (*scr_z88_cpc_load_keymap) (void);

//Rutina a la que se llama desde chardetect_printchar_caracter, normalmente es un printf
void (*scr_detectedchar_print) (z80_byte caracter);




//Rutina que muestra los mensajes de registros de cpu, propio para cada driver
void (*scr_debug_registers)(void);

//Rutina que muestra los mensajes de "debug_printf", propio para cada driver
void (*scr_messages_debug)(char *mensaje);

//Rutina para imprimir un caracter del menu
void (*scr_putchar_menu) (int x,int y, z80_byte caracter,int tinta,int papel);

//Rutina para imprimir un caracter en el pie de la pantalla
void (*scr_putchar_footer) (int x,int y, z80_byte caracter,int tinta,int papel);

void (*scr_putchar_zx8081) (int x,int y, z80_byte caracter);

//indica que el driver de pantalla tiene colores reales. valido para xwindows y curses
int scr_tiene_colores=0;

//nombre del driver: aa, null, xwindows, etc. inicializado por cada driver en init
//renombrado a _new_ para evitar posible error de segfault
char scr_new_driver_name[100];

void scr_tsconf_putpixel_zx_mode(int x,int y,unsigned color);
void scr_refresca_border_tsconf_cont(void);
void screen_tsconf_refresca_rainbow(void);

//esto se usa para curses y stdout, tambien afecta
//a que los caracteres no imprimibles de zx8081 se muestren como ? o como caracteres simulados
z80_bit texto_artistico;
int umbral_arttext=4;


//si el refresco de pantalla para drivers de texto, con rainbow, siempre convierte todo de pixel a ascii art
z80_bit screen_text_all_refresh_pixel={0};

int screen_text_all_refresh_pixel_scale=10;
          
z80_bit screen_text_all_refresh_pixel_invert={0};


//Para frameskip manual
//valor indicado en menu para frameskip
int frameskip=0;
//conteo actual para ver si toca refrescar frame (cuando vale 0)
int frameskip_counter=0;

//Si se muestra puntero raton
z80_bit mouse_pointer_shown={1};


//Intento de hacer las rutinas de refresco de pantalla mas rapidas
//Se guarda minima y dibujada y maxima y, para solo refrescar esa zona que se haya tocado con putpixel
//probado de momento con X11 y consume mas cpu aun
int putpixel_max_y=-1;
int putpixel_min_y=99999;


//Parametro de refresco para drivers stdout y simpletext
//Si vale 1, refresca a cada frame (50 fps)
//Si vale 2, refresca cada dos frames (25 fps)
//Si vale 3, refresca cada tres frames (16 fps)
//Si vale 4, refresca cada cuatro frames (12.5 fps)
//Si vale 5, refresca cada cinco frames (10 fps)
//Si vale 10, 5 fps
//Si vale 25, 2 fps
//Si vale 50, 1 fps

//De fps a esta variable: variable=50/fps -> siempre que fps no sea cero

//Valores seleccionables: 50, 25, 10, 5, 2,1
// -> 1,2,5,10,25,1
int scrstdout_simpletext_refresh_factor=5;

//Usado en varias rutinas
char screen_common_caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";


total_palette_colours total_palette_colours_array[TOTAL_PALETAS_COLORES]={
	{"Speccy","16 colour standard",0,SPECCY_TOTAL_PALETTE_COLOURS},
	//{"SpeccyReal","Real 16/48/+ palette",SPECCY_1648_REAL_PALETTE_FIRST_COLOR,SPECCY_1648_REAL_PALETTE_COLOURS},
	{"Gigascreen","256 gigascreen",SPECCY_TOTAL_PALETTE_COLOURS+SPECCY_GREY_SCANLINE_TOTAL_PALETTE_COLOURS,GIGASCREEN_TOTAL_PALETTE_COLOURS},
	{"Z88","Z88 4 colour",Z88_PXCOLON,Z88_TOTAL_PALETTE_COLOURS},
	{"ULAPlus","ULAPlus GRB palette",ULAPLUS_INDEX_FIRST_COLOR,ULAPLUS_TOTAL_PALETTE_COLOURS},
	{"Spectra","Spectra palette",SPECTRA_INDEX_FIRST_COLOR,SPECTRA_TOTAL_PALETTE_COLOURS},
	{"CPC","CPC palette",CPC_INDEX_FIRST_COLOR,CPC_TOTAL_PALETTE_COLOURS},
	{"Prism","Prism 12 bit palette",PRISM_INDEX_FIRST_COLOR,PRISM_TOTAL_PALETTE_COLOURS},
	{"Sam Coupe","Sam 128 colour palette",SAM_INDEX_FIRST_COLOR,SAM_TOTAL_PALETTE_COLOURS},
	{"TBBlue RGB9","TBBlue 512 colour palette",RGB9_INDEX_FIRST_COLOR,RGB9_TOTAL_PALETTE_COLOURS},
	{"TSConf/ZX-Uno Prism","TSConf/ZX-Uno Prism 15 bit",TSCONF_INDEX_FIRST_COLOR,TSCONF_TOTAL_PALETTE_COLOURS},
	{"VDP9918A","16 colour standard",VDP_9918_INDEX_FIRST_COLOR,VDP_9918_TOTAL_PALETTE_COLOURS},
    {"SMS Mode 4","64 colour standard",SMS_INDEX_FIRST_COLOR,SMS_TOTAL_PALETTE_COLOURS},
    {"QL","8 colour",QL_INDEX_FIRST_COLOR,QL_TOTAL_PALETTE_COLOURS},
    {"BMP File","256 colour",BMP_INDEX_FIRST_COLOR,BMP_TOTAL_PALETTE_COLOURS}
};


//simular modo video zx80/81 en spectrum
z80_bit simulate_screen_zx8081;

//Modo 16C de pentagon
z80_bit pentagon_16c_mode_available={0};

//Refrescar pantalla spectrum sin colores. Solo para modo no realvideo
z80_bit scr_refresca_sin_colores={0};

//umbral de pixeles para dibujar o no un punto
int umbral_simulate_screen_zx8081=4;

//Dice que no hay que hacer fadeout al salir. Se activa desde Mac os x cocoa driver
z80_bit no_fadeout_exit={0};


void scr_set_driver_name(char *nombre)
{
	strcpy(scr_new_driver_name,nombre);
}

//colores usados para el fondo cuando hay menu/overlay activo
//int spectrum_colortable_oscuro[EMULATOR_TOTAL_PALETTE_COLOURS];

//colores usados para grises, red, green, etc
//int spectrum_colortable_grises[EMULATOR_TOTAL_PALETTE_COLOURS];

//y puntero que indica una tabla o la otra
int *spectrum_colortable;


//Colores normales (los primeros sin oscuro), sean ya colores o gama de grises
//los 16 colores (de 16..31) son mas oscuros usados en modos interlaced
//los 256 colores siguientes son los usados en gigascreen
//los 4 colores siguientes son los usados en Z88
//int *spectrum_colortable_normal;
int spectrum_colortable_normal[EMULATOR_TOTAL_PALETTE_COLOURS];

//Colores solo en blanco y negro para cuando se abre el menu y el emulador esta con multitask off
//int spectrum_colortable_new_blanco_y_negro[EMULATOR_TOTAL_PALETTE_COLOURS];

//Tabla con los colores reales del Spectrum. Formato RGB
int spectrum_colortable_original_new[16] =
{

0x000000,  //negro
0x0000C0,  //azul
0xC00000,  //rojo
0xC000C0,  //magenta
0x00C000,  //verde
0x00C0C0,  //cyan
0xC0C000,  //amarillo
0xC0C0C0,  //blanco

0x000000,
0x0000FF,
0xFF0000,
0xFF00FF,
0x00FF00,
0x00FFFF,
0xFFFF00,
0xFFFFFF

};

//Tabla con los colores exactamente iguales para Spectrum 16/48/+, segun calculos de Richard Atkinson
int spectrum_colortable_1648_real[16] =
{
0x060800,
0x0D13A7,
0xBD0707,
0xC312AF,
0x07BA0C,
0x0DC6B4,
0xBCB914,
0xC2C4BC,
0x060800,
0x161CB0,
0xCE1818,
0xDC2CC8,
0x28DC2D,
0x36EFDE,
0xEEEB46,
0xFDFFF7
};

int *screen_return_spectrum_palette(void)
{
	//Retorna la tabla de colores basicos de spectrum 0-16 segun si paleta real activa y segun si maquina tiene paleta diferente o no
	//Paleta real segun info de Richard Atkinson

        if (MACHINE_IS_SPECTRUM_16 || MACHINE_IS_SPECTRUM_48) {
                if (spectrum_1648_use_real_palette.v) {
			return spectrum_colortable_1648_real;
                }
        }

	return spectrum_colortable_original_new;
}



z80_bit spectrum_1648_use_real_palette={0};


//Tabla con los colores reales del Z88. Formato RGB
const int z88_colortable_original[4]={
0x461B7D, //Enabled pixel
0x90B0A7, //Grey enabled pixel
0xD2E0B9, //Empty pixel when screen is switched on
0xE0E0E0 //Empty pixel when screen is switched off
};

//ubicacion en el array de colores de los de Z88
//ver screen.h, Z88_PXCOLON, etc


//colores para chip de msx
const int vdp9918_colortable_original[16]={
0x000000,
0x010101,
0x3eb849,
0x74d07d,
0x5955e0,
0x8076f1,
0xb95e51,
0x65dbef,
0xdb6559,
0xff897d,
0xccc35e,
0xded087,
0x3aa241,
0xb766b5,
0xcccccc,
0xffffff
};



//colores para QL
const int ql_colortable_original[8]={
0x000000, //Negro
0x0000ff, //Azul
0xff0000, //Rojo
0xff00ff, //Magenta
0x00ff00, //Verde
0x00ffff, //Cyan
0xffff00, //Amarillo
0xffffff  //Blanco
};


//Tabla con colores para tema de GUI Solarized. 
/*
SOLARIZED HEX     16/8 TERMCOL  XTERM/HEX   L*A*B      RGB         HSB
--------- ------- ---- -------  ----------- ---------- ----------- -----------
base03    #002b36  8/4 brblack  234 #1c1c1c 15 -12 -12   0  43  54 193 100  21
base02    #073642  0/4 black    235 #262626 20 -12 -12   7  54  66 192  90  26
base01    #586e75 10/7 brgreen  240 #585858 45 -07 -07  88 110 117 194  25  46
base00    #657b83 11/7 bryellow 241 #626262 50 -07 -07 101 123 131 195  23  51
base0     #839496 12/6 brblue   244 #808080 60 -06 -03 131 148 150 186  13  59
base1     #93a1a1 14/4 brcyan   245 #8a8a8a 65 -05 -02 147 161 161 180   9  63
base2     #eee8d5  7/7 white    254 #e4e4e4 92 -00  10 238 232 213  44  11  93
base3     #fdf6e3 15/7 brwhite  230 #ffffd7 97  00  10 253 246 227  44  10  99
yellow    #b58900  3/3 yellow   136 #af8700 60  10  65 181 137   0  45 100  71
orange    #cb4b16  9/3 brred    166 #d75f00 50  50  55 203  75  22  18  89  80
red       #dc322f  1/1 red      160 #d70000 50  65  45 220  50  47   1  79  86
magenta   #d33682  5/5 magenta  125 #af005f 50  65 -05 211  54 130 331  74  83
violet    #6c71c4 13/5 brmagenta 61 #5f5faf 50  15 -45 108 113 196 237  45  77
blue      #268bd2  4/4 blue      33 #0087ff 55 -10 -45  38 139 210 205  82  82
cyan      #2aa198  6/6 cyan      37 #00afaf 60 -35 -05  42 161 152 175  74  63
green     #859900  2/2 green     64 #5f8700 60 -20  65 133 153   0  68 100  60
*/
const int solarized_colortable_original[16]={
0x002b36, //base03
0x073642, //base02
0x586e75, //base01
0x657b83, //base00
0x839496, //base0
0x93a1a1, //base1   (5)
0xeee8d5, //base2
0xfdf6e3, //base3
0xb58900, //yellow
0xcb4b16, //orange
0xdc322f, //red     (10)
0xd33682, //magenta
0x6c71c4, //violet
0x268bd2, //blue
0x2aa198, //cyan
0x859900, //green    (15)
};

const int turbovision_colortable_original[TURBOVISION_TOTAL_PALETTE_COLOURS]={

0x000000,  //0
0xa80000,
0x00a800,  //2 green
0xa85400,
0x0000a8,
0xa800a8,
0x00a8a8,  //6 cyan
0xa8a8a8,  //7 white
0x545454,  //8 grey
0xfc5454,
0x54fc54,
0xfcfc54,
0x5454fc,
0xfc54fc,
0x54fcfc,
0xffffff

};

const int beos_colortable_original[BEOS_TOTAL_PALETTE_COLOURS]={

0xffce00,  //Amarillo del titulo
0x989898,  //Recuadro
0xdedede,  //Gris menus
0x9c9c9c, //Gris opcion seleccionada
0xefefef, //Gris titulo ventana no seleccionada
//0x63bdce  //Azul hotkey
0x00c000  //Verde hotkey


};

const int retromac_colortable_original[RETROMAC_TOTAL_PALETTE_COLOURS]={
    0xcccccc, //Titulo ventana
    0xdddddd, //Titulo ventana inactiva, papel
    0x322e9c, //Texto seleccionado
    0x777777, //Tinta opcion no disponible
    0x999999  //Color para el marco ventana
};

const int amigaos_colortable_original[AMIGAOS_TOTAL_PALETTE_COLOURS]={
    0x0055aa, //Tinta Titulo ventana
    0x416b94, //Tinta Titulo ventana inactiva
    0xff8a00, //Naranja
    0xe84c44  //Rojo alterado. Original: 0xdf2020  

};

const int ataritos_colortable_original[ATARITOS_TOTAL_PALETTE_COLOURS]={
    0x00de00,  //Verde
    0xdedede   //Blanco
};

const int osdos_colortable_original[OSDOS_TOTAL_PALETTE_COLOURS]={
    0x0000a3,  //Azul titulo ventana papel, texto seleccionado
    0xd7d7d7, //Tinta gris ventana inactiva, papel
    0x808080 //Tinta opcion no disponible, titulo ventana inactiva
};

const int zesarux_plus_colortable_original[ZESARUX_PLUS_TOTAL_PALETTE_COLOURS]={
    0xE0E0E0  //Blanco menos brillante que el blanco con brillo de Spectrum
};

const int riscos_colortable_original[RISCOS_TOTAL_PALETTE_COLOURS]={
    0x000052,  //Azul oscuro, para texto en ventana
    0xffbc63,  //Naranja para titulo ventana inactiva
    0x05058b,   //Azul para texto en titulo ventana activa 
    0xfe0000  //Titulo ventana activa
};

//Tabla con los colores extra del Spectra.
//Valores para intensidades de color:
/*
C1 C0  Voltage  Output
0  0   0        0%           -> 0
0  1   0.24     34.8%    -> 255/100*34.8 = 88.74 -> 89
1  0   0.50     72.5%    -> 184.875 -> 185
1  1   0.69     100%     -> 255
*/
#define SPECTRA_COL_ZERO 0
#define SPECTRA_COL_LOW 89
#define SPECTRA_COL_MEDIUM 185
#define SPECTRA_COL_HIGH 255

//La tabla se inicializa en screen_init_colour_table y luego se copia a las tablas de color de spectrum spectrum_colortable_normal y spectrum_colortable_grises, etc
int spectra_colortable_original[64];



//Modo de grises activo
//0: colores normales
//1: componente Blue
//2: componente Green
//4: componente Red
//Se pueden sumar para diferentes valores
int screen_gray_mode=0;

//Indica que esta el driver de pantalla stdout
//y por tanto, el bucle de cpu debe interceptar llamadas a RST16
int screen_stdout_driver=0;


//Indica que esta el driver de pantalla simpletext
//y por tanto, el bucle de cpu debe interceptar llamadas a RST16
int screen_simpletext_driver=0;


z80_bit inverse_video;


//Meter marca de agua en la derecha, abajo, en la zona de pantalla reducida
int screen_watermark_position=3; //0: arriba izq 1: arriba der 2 abajo izq 3 abajo der

//Si marca de agua habilitada
z80_bit screen_watermark_enabled={0};


//Indica que el driver de video (por el momento, solo xwindows y fbdev) debe repintar la pantalla
//teniendo en cuenta si hay menu activo, y por tanto evitar pintar zonas donde hay texto del menu
//esta variable la pone a 1 el driver de xwindows y fbdev
int screen_refresh_menu=0;


//si esta activado real video
z80_bit rainbow_enabled;

//Si hay autodetecccion de modo rainbow
z80_bit autodetect_rainbow;


//Valores usados en real video
//normalmente a 8
int screen_invisible_borde_superior;
//normalmente a 56.
int screen_borde_superior;

//estos dos anteriores se suman aqui. es 64 en 48k, y 63 en 128k. por tanto, uno de los dos valores vale 1 menos
int screen_indice_inicio_pant;

//suma del anterior+192
int screen_indice_fin_pant;

//normalmente a 56
int screen_total_borde_inferior;

//normalmente a 48
int screen_total_borde_izquierdo;
//lo mismo en t_estados
int screen_testados_total_borde_izquierdo;

//normalmente a 48
int screen_total_borde_derecho;

//normalmente a 96
int screen_invisible_borde_derecho;

//lo mismo pero en testados
//int screen_invisible_testados_borde_derecho;


//Total de scanlines. usualmente 312 o 311
int screen_scanlines;

//Total de t_estados por linea
int screen_testados_linea;

//Total de t_estados de un frame entero
int screen_testados_total;


//donde empieza borde derecho, en testados
int screen_testados_indice_borde_derecho;


int temp_borrar=3;

//Estos valores se inicializaran la primera vez en set_machine
int get_total_ancho_rainbow_cached;

int temp_borrar2=0;

int get_total_alto_rainbow_cached;


//buffer border de linea actual
//#define BORDER_ARRAY_LENGTH 228+24 -> en screen.h
//24 de screen_total_borde_izquierdo/2
//Un scanline empieza con display, border derecho, retrace horizontal y borde izquierdo. y ahi se inicia una nueva scanline
//A la hora de dibujar en nuestra rutina consideramos: borde izq-display-borde derecho-retrace
//Pero el border en cambio si que lo guardamos teniendo en cuenta esto,
//sentencias out guardan valor de border comenzando en posicion 24
//z80_byte buffer_border[BORDER_ARRAY_LENGTH];

//Array de border que guarda colores teniendo en cuenta toda la pantalla (para cada t-estado posible)
//+xx para que haya un margen por debajo por si la funcion de lectura de border en screen.c se va de array
z80_byte fullbuffer_border[MAX_FULLBORDER_ARRAY_LENGTH+MAX_STATES_LINE];

//pixeles y atributos
z80_byte scanline_buffer[SCANLINEBUFFER_ONE_ARRAY_LENGTH*MAX_CPU_TURBO_SPEED];

//ultima posicion leida en buffer_atributos
int last_x_atributo;



//frames que hemos saltado
int framedrop_total=0;
//frames que se iban a dibujar, saltados o no... cuando se llega a 50, se resetea, y se muestra por pantalla los drop
int frames_total=0;

//ha llegado la interrupcion de final de frame antes de redibujar la pantalla. Normalmente no dibujar ese frame para ganar velocidad
int framescreen_saltar;

/*
Si en el frame anterior hemos saltado ese frame a la hora de dibujarlo
Y por tanto indica proxima decision en el siguiente frame para renderizar scanlines o no
Utilizado por ejemplo en tbblue para no renderizar cada scanline si frame anterior se ha saltado

El funcionamiento del sistema de framedrop es:

-contar tiempo entre el inicio de los t-estados de un frame de video hasta el final. 
Si ha pasado mas del tiempo esperado (habitualmente 20ms) indicarlo con variable framescreen_saltar

Si framescreen_saltar y tenemos autoframeskip, no dibujar ese frame en pantalla  (funcion cpu_loop_refresca_pantalla retorna sin hacer nada)
Al mismo tiempo, si no se ha dibujado ese frame, indicar next_frame_skip_render_scanlines=1

Ese next_frame_skip_render_scanlines se comprueba cuando se renderiza cada scanline (funciones screen_store_scanline_rainbow*)
Si está el flag a 1, no se renderiza esa linea. Esto lo que hace es que si el frame anterior no lo hemos dibujado en pantalla 
(funcion cpu_loop_refresca_pantalla retorna sin hacer nada), al siguiente frame no renderizaremos ninguna scanline.

Esto permite ahorrar mucho tiempo de proceso


*/
int next_frame_skip_render_scanlines=0;

//Se hace auto frameskip
z80_bit autoframeskip={1};

//Ultimo FPS leido. Para mostrar en consola o debug menu
int ultimo_fps=0;

//frame entrelazado par
//z80_bit interlaced_frame_par;

//numero de frame actual. para interlaced
z80_byte interlaced_numero_frame=0;

//si esta activo el modo entrelazado
z80_bit video_interlaced_mode;

//si esta activo el scanlines mode. requiere interlaced
z80_bit video_interlaced_scanlines={0};

//si esta activo gigascreen
z80_bit gigascreen_enabled={0};



//vofile
FILE *ptr_vofile;
char *vofilename;

z80_bit vofile_inserted;

//fps del archivo final=50/vofile_fps
int vofile_fps=10;

int vofile_frame_actual;


//Para deteccion de realvideo segun veces que cambia el border
//Veces que ha cambiado el color del border en un mismo frame
int detect_rainbow_border_changes_in_frame=0;
//Numero de frames seguidos en que el border se ha cambiado cada frame mas o igual de DETECT_RAINBOW_BORDER_MAX_IN_FRAMES veces
int detect_rainbow_border_total_frames=0;


//Funcion usada para putpixel en z88. apunta a funcion de rainbow o no rainbow
//no esta inicializada con nada, pues antes que se vaya a usar (en refresca_pantalla o draw_lower_screen) se inicializara
void (*scr_putpixel_zoom_z88) (int x,int y,unsigned color);


//Colores usados en pantalla panic, en logo de extended desktop etc. Son los colores del rainbow de spectrum
//rojo, amarillo, verde, cyan,negro
int screen_colores_rainbow[]={2+8,6+8,4+8,5+8,0};

int screen_colores_rainbow_nobrillo[]={2,6,4,5,0};

//devuelve 1 si hay que dibujar la linea, de acorde al entrelazado
/*
int if_store_scanline_interlace(int y)
{
         //si linea no coincide con entrelazado, volvemos
         if (video_interlaced_mode.v==0) return 1;
         if ((y&1) == interlaced_frame_par.v ) return 1;

        return 0;
}
*/

int if_store_scanline_interlace(int y)
{

	//para que no se queje el compilador
	y++;

	return 1;
}

//Retorna 1 si el driver grafico es completo
int si_complete_video_driver(void)
{
        if (!strcmp(scr_new_driver_name,"xwindows")) return 1;
        if (!strcmp(scr_new_driver_name,"sdl")) return 1;
        if (!strcmp(scr_new_driver_name,"fbdev")) return 1;
        if (!strcmp(scr_new_driver_name,"cocoa")) return 1;
        return 0;
}


//Retorna 1 si el driver grafico permite menu normal
int si_normal_menu_video_driver(void)
{

	//printf ("video driver: %s\n",scr_new_driver_name);

	if (si_complete_video_driver() ) return 1;

	//curses, aa, caca, pero ningun otro (ni stdout, ni simpletext, ni null... )
        if (!strcmp(scr_new_driver_name,"curses")) return 1;
        if (!strcmp(scr_new_driver_name,"aa")) return 1;
        if (!strcmp(scr_new_driver_name,"caca")) return 1;
        return 0;
}

//establece valor de screen_indice_inicio_pant y screen_indice_fin_pant
void screen_set_video_params_indices(void)
{
	screen_indice_inicio_pant=screen_invisible_borde_superior+screen_borde_superior;

	if (MACHINE_IS_PRISM) {
		//Prism es la unica maquina que tiene 384 pixeles de alto en zona de display
		screen_indice_fin_pant=screen_indice_inicio_pant+384;
	}

	else {
		screen_indice_fin_pant=screen_indice_inicio_pant+192;
	}

	screen_scanlines=screen_indice_fin_pant+screen_total_borde_inferior;
	//screen_testados_linea=(screen_invisible_borde_izquierdo+screen_total_borde_izquierdo+256+screen_total_borde_derecho)/2;

	screen_testados_total=screen_testados_linea*screen_scanlines;

	//TODO. hacer esto de manera mas elegante

	//timer para Z88 es cada 5 ms y ejecutamos los mismos ciclos que un spectrum para cada 5 ms. Por tanto, va 4 veces mas rapido que spectrum


	if (MACHINE_IS_Z88) {
		//dado que se genera interrupcion cada 5 ms (y no cada 20ms) esto equivale a 4 veces menos

		//Desactivado.
		//Si hago esto, saltando a interrupciones solo con EI: no funciona el teclado, quiza porque entonces el core del Z88 es demasiado lento...
		//si salta interrupciones aunque haya un DI, entonces se llaman interrupciones como parte del proceso inicial del boot y no arranca
		screen_testados_total /=4;
		//screen_testados_total /=2;
	}



	screen_testados_total_borde_izquierdo=screen_total_borde_izquierdo/2;

	screen_testados_indice_borde_derecho=screen_testados_total_borde_izquierdo+128;

	//printf ("t_estados_linea: %d\n",screen_testados_linea);
	//printf ("scanlines: %d\n",screen_scanlines);
	//printf ("t_estados_total: %d\n",screen_testados_total);
	//sleep(2);

}


//el formato del buffer del video rainbow es:
//1 byte por pixel, cada pixel tiene el valor de color 0..15.
//Valores 16..255 no tienen sentido, de momento
//Valores mas alla de 255 son usados en ulaplus. ver tablas exactas
z80_int *rainbow_buffer=NULL;


//Para gigascreen
z80_int *rainbow_buffer_one=NULL;
z80_int *rainbow_buffer_two=NULL;

//cache de putpixel. Solo usado en modos rainbow (segun recuerdo)
z80_int *putpixel_cache=NULL;


//funcion con debug. usada en el macro con debug

void store_value_rainbow_debug(z80_int **p, z80_int valor)
{
	z80_int *puntero_buf_rainbow;
	puntero_buf_rainbow=*p;

        int ancho,alto,tamanyo;

	ancho=screen_get_emulated_display_width_no_zoom();
	alto=screen_get_emulated_display_height_no_zoom();


        tamanyo=ancho*alto*2;

        //Asignamos mas bytes dado que la ultima linea de pantalla genera datos (de borde izquierdo) mas alla de donde corresponde
        tamanyo=tamanyo+ancho;

	tamanyo*=MAX_CPU_TURBO_SPEED;


	if (puntero_buf_rainbow==NULL) {
		printf ("puntero_buf_rainbow NULL\n");
		return ;
	}

	if (puntero_buf_rainbow>(rainbow_buffer+tamanyo)) {
		//printf ("puntero_buf_rainbow mayor limite final: %d (max %d)\n",puntero_buf_rainbow-rainbow_buffer,tamanyo);
		return;
	}

	if (puntero_buf_rainbow<rainbow_buffer) {
                //printf ("puntero_buf_rainbow menor que inicial: -%d\n",rainbow_buffer-puntero_buf_rainbow);
                return;
        }


	*puntero_buf_rainbow=valor;
	(*p)++;

}





void recalcular_get_total_ancho_rainbow(void)
{
	debug_printf (VERBOSE_INFO,"Recalculate get_total_ancho_rainbow");
	if (MACHINE_IS_Z88) {
		get_total_ancho_rainbow_cached=SCREEN_Z88_WIDTH;
	}

	else if (MACHINE_IS_CPC) {
                get_total_ancho_rainbow_cached=2*CPC_LEFT_BORDER_NO_ZOOM*border_enabled.v+CPC_DISPLAY_WIDTH;
        }

	else if (MACHINE_IS_PRISM) {
		get_total_ancho_rainbow_cached=(screen_total_borde_izquierdo+screen_total_borde_derecho)*border_enabled.v+512;
        }

	else if (MACHINE_IS_TSCONF) {
		get_total_ancho_rainbow_cached=TSCONF_DISPLAY_WIDTH;
	}

	else if (MACHINE_IS_TBBLUE) {
		get_total_ancho_rainbow_cached=2*TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v+512;
	}	

	else if (MACHINE_IS_SAM) {
                get_total_ancho_rainbow_cached=2*SAM_LEFT_BORDER_NO_ZOOM*border_enabled.v+SAM_DISPLAY_WIDTH;
        }

				else if (MACHINE_IS_QL) {
			                get_total_ancho_rainbow_cached=2*QL_LEFT_BORDER_NO_ZOOM*border_enabled.v+QL_DISPLAY_WIDTH;
			        }



	else {
		get_total_ancho_rainbow_cached=(screen_total_borde_izquierdo+screen_total_borde_derecho)*border_enabled.v+256;
	}

	//printf ("get_total_ancho_rainbow_cached: %d\n",get_total_ancho_rainbow_cached);
	//printf ("get_total_ancho_rainbow_cached: %d\n",get_total_ancho_rainbow());
	//sleep(2);
}

//sin contar la parte invisible
void recalcular_get_total_alto_rainbow(void)
{
        debug_printf (VERBOSE_INFO,"Recalculate get_total_alto_rainbow");
	if (MACHINE_IS_Z88) {
		get_total_alto_rainbow_cached=SCREEN_Z88_HEIGHT;
        }

        else if (MACHINE_IS_CPC) {
                get_total_alto_rainbow_cached=2*CPC_TOP_BORDER_NO_ZOOM*border_enabled.v+CPC_DISPLAY_HEIGHT;
        }

        else if (MACHINE_IS_PRISM) {
                //get_total_alto_rainbow_cached=2*PRISM_TOP_BORDER_NO_ZOOM*border_enabled.v+PRISM_DISPLAY_HEIGHT;
		get_total_alto_rainbow_cached=(screen_borde_superior+screen_total_borde_inferior)*border_enabled.v+384;
        }

	else if (MACHINE_IS_TSCONF) {
		get_total_alto_rainbow_cached=TSCONF_DISPLAY_HEIGHT;
	}

	else if (MACHINE_IS_TBBLUE) {
		get_total_alto_rainbow_cached=(TBBLUE_TOP_BORDER_NO_ZOOM+TBBLUE_BOTTOM_BORDER_NO_ZOOM)*border_enabled.v+384;
	}

        else if (MACHINE_IS_SAM) {
                get_total_alto_rainbow_cached=2*SAM_TOP_BORDER_NO_ZOOM*border_enabled.v+SAM_DISPLAY_HEIGHT;
        }

	else if (MACHINE_IS_QL) {
		get_total_alto_rainbow_cached=2*QL_TOP_BORDER_NO_ZOOM*border_enabled.v+QL_DISPLAY_HEIGHT;
	}

	else {
	        get_total_alto_rainbow_cached=(screen_borde_superior+screen_total_borde_inferior)*border_enabled.v+192;
	}

	//printf ("get_total_alto_rainbow_cached: %d\n",get_total_alto_rainbow_cached);
	//printf ("get_total_alto_rainbow_cached: %d\n",get_total_alto_rainbow());
	//sleep(2);
}

//esas dos funciones, get_total_ancho_rainbow y get_total_alto_rainbow ahora son macros definidos en screen.h


void init_rainbow(void)
{

        if (rainbow_buffer_one!=NULL) {
                debug_printf (VERBOSE_INFO,"Freeing previous rainbow video buffer");
                free(rainbow_buffer_one);
		free(rainbow_buffer_two);
        }


	int ancho,alto,tamanyo;



        ancho=screen_get_emulated_display_width_no_zoom();
        alto=screen_get_emulated_display_height_no_zoom();


	tamanyo=ancho*alto*2; //buffer de 16 bits (*2 bytes)

	//Asignamos mas bytes dado que la ultima linea de pantalla genera datos (de borde izquierdo) mas alla de donde corresponde
	tamanyo=tamanyo+ancho;

	tamanyo*=MAX_CPU_TURBO_SPEED;

	debug_printf (VERBOSE_INFO,"Initializing two rainbow video buffer of size: %d x %d , %d bytes each",ancho,alto,tamanyo);



	rainbow_buffer_one=malloc(tamanyo);
	if (rainbow_buffer_one==NULL) {
		cpu_panic("Error allocating rainbow video buffer");
	}


        rainbow_buffer_two=malloc(tamanyo);
        if (rainbow_buffer_two==NULL) {
                cpu_panic("Error allocating rainbow video buffer");
        }




	rainbow_buffer=rainbow_buffer_one;


}

void init_cache_putpixel(void)
{

#ifdef PUTPIXELCACHE
	debug_printf (VERBOSE_INFO,"Initializing putpixel_cache");
	if (putpixel_cache!=NULL) {
		debug_printf (VERBOSE_INFO,"Freeing previous putpixel_cache");
		free(putpixel_cache);
	}

        int ancho,alto,tamanyo;


        //ancho=screen_get_emulated_display_width_no_zoom();
        //alto=screen_get_emulated_display_height_no_zoom();
	//Incluir en tamanyo el footer
        //ancho=screen_get_window_size_width_no_zoom_border_en();
        //alto=screen_get_window_size_height_no_zoom_border_en();
        ancho=screen_get_window_size_width_no_zoom();
        alto=screen_get_window_size_height_no_zoom();


	//El tamanyo depende del footer. Pero no del border (siempre se incluye con border)


        tamanyo=ancho*alto;
	//para poder hacer putpixel cache con zoom y*2 y interlaced, doble de tamanyo
	tamanyo *=2;

	//para poder hacer putpixel cache con modo timex 512x192
	tamanyo *=2;


	putpixel_cache=malloc(tamanyo*2); //*2 porque es z80_int


	debug_printf (VERBOSE_INFO,"Initializing putpixel_cache of size: %d bytes",tamanyo);

	if (putpixel_cache==NULL) {
		cpu_panic("Error allocating putpixel_cache video buffer");
	}

	clear_putpixel_cache();
#else
	debug_printf (VERBOSE_INFO,"Putpixel cache disabled on compilation time");
#endif

}


//#define put_putpixel_cache(x,y) putpixel_cache[x]=y


//rutina para comparar un caracter
//entrada:
//p: direccion de pantalla en sp


//salida:
//caracter que coincide
//0 si no hay coincidencia

//inverse si o no

//usado en scrcurses y en simular video de zx80/81
//sprite origen a intervalos de "step"
z80_byte compare_char_tabla_step(z80_byte *origen,z80_byte *inverse,z80_byte *tabla_leemos,int step) {

        z80_byte *copia_origen;
	z80_byte *tabla_comparar;


        z80_byte caracter=32;

        for (;caracter<128;caracter++) {
                //printf ("%d\n",caracter);
                //tabla_leemos apunta siempre al primer byte de la tabla del caracter que leemos
                tabla_comparar=tabla_leemos;
                copia_origen=origen;

                //tabla_comparar : puntero sobre la tabla de caracteres
                //copia_origen: puntero sobre la pantalla

                //
                int numero_byte=0;
                for (numero_byte=0; (numero_byte<8) && (*copia_origen == *tabla_comparar) ;numero_byte++,copia_origen+=step,tabla_comparar++) {
                }

                if (numero_byte == 8) {
                        *inverse=0;
                        return caracter;
                }


                //probar con texto inverso

                numero_byte=0;
                for (numero_byte=0; (numero_byte<8) && (*copia_origen == (*tabla_comparar ^ 255 )) ;numero_byte++,copia_origen+=step,tabla_comparar++) {
                }

                if (numero_byte == 8) {
                        *inverse=1;
                        return caracter;
                }


                tabla_leemos +=8;
        }



        return 0;
}

//comparar sprite de origen (con direccionamiento de spectrum, cada linea a intervalo 256) con la tabla de caracteres de la ROM
z80_byte compare_char_tabla(z80_byte *origen,z80_byte *inverse,z80_byte *tabla_leemos) {
	return compare_char_tabla_step(origen,inverse,tabla_leemos,256);
}



//usado en scrcurses con rainbow en zx8081
//devuelve 255 si no coincide
z80_byte compare_char_tabla_rainbow(z80_byte *origen,z80_byte *inverse,z80_byte *tabla_leemos) {

        z80_byte *tabla_comparar;


        z80_byte caracter=0;



        for (;caracter<64;caracter++) {
                //printf ("%d\n",caracter);
                //tabla_leemos apunta siempre al primer byte de la tabla del caracter que leemos
                tabla_comparar=tabla_leemos;
                //copia_origen=origen;

                //tabla_comparar : puntero sobre la tabla de caracteres
                //copia_origen: puntero sobre la pantalla

                //
                int numero_byte=0;
                for (numero_byte=0; (numero_byte<8) && (origen[numero_byte] == *tabla_comparar) ;numero_byte++,tabla_comparar++) {
                }

                if (numero_byte == 8) {
                        *inverse=0;
                        return caracter;
                }


                //probar con texto inverso
               	numero_byte=0;
                for (numero_byte=0; (numero_byte<8) && (origen[numero_byte] == (*tabla_comparar ^ 255 )) ;numero_byte++,tabla_comparar++) {
       	        }

               	if (numero_byte == 8) {
			*inverse=1;
                        return caracter;
       	        }


                tabla_leemos +=8;
        }



        return 255;
}



z80_byte compare_char_step(z80_byte *origen,z80_byte *inverse,int step)
{
        z80_byte *tabla_leemos;
	z80_byte caracter;

	//Tenemos que buscar en toda la tabla de caracteres. Primero en tabla conocida y luego en la que apunta a 23606/7
	//Tabla conocida es la del spectrum, pero tambien vale para ZX81
	tabla_leemos=char_set_spectrum;

	caracter=compare_char_tabla_step(origen,inverse,tabla_leemos,step);
	if (caracter!=0) return caracter;
	
	
	//si no consultamos a 23606/7, retornar 0
	if (ocr_settings_not_look_23606.v) return 0;

	z80_int puntero_tabla_caracteres;
	if (MACHINE_IS_SPECTRUM_16_48) {
		puntero_tabla_caracteres=value_8_to_16(memoria_spectrum[23607],memoria_spectrum[23606])+256;

		caracter=compare_char_tabla_step(origen,inverse,&memoria_spectrum[puntero_tabla_caracteres],step);

		return caracter;
	}

	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED || MACHINE_IS_CHLOE || MACHINE_IS_TIMEX_TS2068) {
		z80_byte *offset_ram_5;

		//Si zxuno sin bootm
		if (MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
			offset_ram_5=zxuno_sram_mem_table_new[5];
		}

		else if (MACHINE_IS_CHLOE) {
			offset_ram_5=chloe_home_ram_mem_table[5];
		}

		else if (MACHINE_IS_TIMEX_TS2068) {
			offset_ram_5=timex_home_ram_mem_table[0];
		}

		else {
			//modelos 128k y +2a
			offset_ram_5=ram_mem_table[5];
		}

		//buscamos el puntero
		offset_ram_5 +=(23606-16384);

		puntero_tabla_caracteres=value_8_to_16(*(offset_ram_5+1),*offset_ram_5)+256;
		//printf ("puntero: %d\n",puntero_tabla_caracteres);
		//convertimos ese puntero de spectrum en puntero de ram


		z80_int dir=puntero_tabla_caracteres;
		//obtenido tal cual de peek byte

		z80_byte *puntero;


		//apunta a rom?
		if (dir<16384) {

			//hemos de suponer que siempre apunta a rom3(+2a)/rom1(128k)

			if (MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
				z80_byte *offset_rom3;
				offset_rom3=zxuno_sram_mem_table_new[3+8];
				puntero=offset_rom3+dir-16384;
			}

			else if (MACHINE_IS_CHLOE) {
				//ROM1
				puntero=chloe_rom_mem_table[1]+dir;
			}

			else if (MACHINE_IS_TIMEX_TS2068) {
				//ROM
				puntero=timex_rom_mem_table[0]+dir;
			}


			//modelos 128k
			else if (MACHINE_IS_SPECTRUM_128_P2) {
				//ROM1
				puntero=&memoria_spectrum[16384]+dir;
			}

			//modelos +2a
			//else if (MACHINE_IS_SPECTRUM_P2A_P3) {
			else {
				//ROM 3
				puntero=&memoria_spectrum[49152]+dir;
			}

		}
		else {

			int segmento;
			z80_int dir_orig=dir;

			segmento=dir / 16384;
			dir = dir & 16383;
			puntero=memory_paged[segmento];	
	                


			//Segmentos de 8kb
			if (MACHINE_IS_TIMEX_TS2068) {
				segmento=dir_orig / 8192;
				dir = dir_orig & 8191;
				puntero=timex_memory_paged[segmento];
				puntero +=dir;
			}

			else if (MACHINE_IS_CHLOE) {
                                segmento=dir_orig / 8192;
                                dir = dir_orig & 8191;
                                puntero=chloe_memory_paged[segmento];
				puntero +=dir;
                        }

			else if (MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
				segmento=dir_orig / 8192;
				dir = dir_orig & 8191;
				puntero=zxuno_memory_paged_brandnew[segmento];
				puntero +=dir;
			}


			//Segmentos de 16 kb
			else puntero +=dir;
		}

		caracter=compare_char_tabla_step(origen,inverse,puntero,step);
		return caracter;

	}

	return caracter;
}

z80_byte compare_char(z80_byte *origen,z80_byte *inverse)
{
	return compare_char_step(origen,inverse,256);
}

z80_int devuelve_direccion_pantalla_no_table(z80_byte x,z80_byte y)
{

        z80_byte linea,high,low;

        linea=y/8;

        low=x+ ((linea & 7 )<< 5);
        high= (linea  & 24 )+ (y%8);



        return low+high*256;
}

void init_screen_addr_table(void)
{

	int x,y;
	int index=0;
	z80_int direccion;

	screen_addr_table=malloc(6144*2);
	if (screen_addr_table==NULL) {
		cpu_panic ("Error allocating sprite table");
	}


	for (y=0;y<192;y++) {
                for (x=0;x<32;x++) {
                direccion=devuelve_direccion_pantalla_no_table(x,y);
		screen_addr_table[index++]=direccion;
		}
	}

}



int scr_si_color_oscuro(void)
{

	//desactivamos por completo el cambio a color oscuro. Y como consecuencia tambien, el cambio a blanco y negro cuando esta menu abierto y multitask off
	return 0;

	if (menu_overlay_activo) {

		//si esta modo ulaplus activo, no gris
		//if (ulaplus_presente.v && ulaplus_enabled.v) return 0;

		//si esta modo spectra activo, no gris
		//if (spectra_enabled.v) return 0;

                //pero si no hay menu y esta la segunda capa de overlay, no poner en gris


                //pero si hay texto splash, si hay que poner en gris
		//si hay texto de guessing loading, hay que poner gris

                if (menu_abierto==1) {
			return 1;
                }

		else {
			//si no estamos en menu, hacerlo solo cuando este splash   //o guessing tape
			if (menu_splash_text_active.v) return 1;
			//if (tape_guessing_parameters) return 1;
		}
	}

	return 0;
}


//Rutina comun de refresco de border de zx80,81,ace  y spectrum
void scr_refresca_border_comun_spectrumzx8081(unsigned int color)
{
//      printf ("Refresco border\n");

        int x,y;

	//simular modo fast
        if (MACHINE_IS_ZX8081 && video_fast_mode_emulation.v==1 && video_fast_mode_next_frame_black==LIMIT_FAST_FRAME_BLACK) {

		//printf ("color border 0\n");
		color=0;
	}


	//Top border cambia en spectrum y zx8081 y ace
	int topborder=TOP_BORDER;
	if (MACHINE_IS_ZX8081ACE) topborder=ZX8081ACE_TOP_BORDER;

	//color +=spectrum_palette_offset;


        //parte superior
        for (y=0;y<topborder;y++) {
                for (x=0;x<ANCHO_PANTALLA*zoom_x+LEFT_BORDER*2;x++) {
                                scr_putpixel(x,y,color);
                }
        }

        //parte inferior
        for (y=0;y<BOTTOM_BORDER;y++) {
                for (x=0;x<ANCHO_PANTALLA*zoom_x+LEFT_BORDER*2;x++) {
                                scr_putpixel(x,topborder+y+ALTO_PANTALLA*zoom_y,color);


                }
        }


        //laterales
        for (y=0;y<ALTO_PANTALLA*zoom_y;y++) {
                for (x=0;x<LEFT_BORDER;x++) {
                        scr_putpixel(x,topborder+y,color);
                        scr_putpixel(LEFT_BORDER+ANCHO_PANTALLA*zoom_x+x,topborder+y,color);
                }

        }


}

void scr_refresca_border(void)
{
	int color;

	if (simulate_screen_zx8081.v==1) color=15;
	else color=out_254 & 7;

	if (scr_refresca_sin_colores.v) color=7;

	scr_refresca_border_comun_spectrumzx8081(color);
}

void scr_refresca_border_zx8081(void)
{

	scr_refresca_border_comun_spectrumzx8081(15);
}




void screen_prism_refresca_no_rainbow_border(int color)
{

        int x,y;


        //parte superior
        for (y=0;y<PRISM_TOP_BORDER;y++) {
                for (x=0;x<PRISM_DISPLAY_WIDTH*zoom_x+PRISM_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,y,color);


                }
        }

        //parte inferior
        for (y=0;y<PRISM_TOP_BORDER;y++) {
                for (x=0;x<PRISM_DISPLAY_WIDTH*zoom_x+PRISM_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,PRISM_TOP_BORDER+y+PRISM_DISPLAY_HEIGHT*zoom_y,color);


                }
        }


        //laterales
        for (y=0;y<PRISM_DISPLAY_HEIGHT*zoom_y;y++) {
                for (x=0;x<PRISM_LEFT_BORDER;x++) {
                        scr_putpixel(x,PRISM_TOP_BORDER+y,color);
                        scr_putpixel(PRISM_LEFT_BORDER+PRISM_DISPLAY_WIDTH*zoom_x+x,PRISM_TOP_BORDER+y,color);
                }

        }


}


void screen_prism_refresca_pantalla_comun_prism(int x,int y,unsigned int color)
{

        int dibujar=0;

        //if (x>255) dibujar=1;
        //else if (y>191) dibujar=1;
        if (scr_ver_si_refrescar_por_menu_activo(x/8,y/8)) dibujar=1;

        if (dibujar) {
		scr_putpixel_zoom(x,y,color);
                scr_putpixel_zoom(x,y+1,color);
                scr_putpixel_zoom(x+1,y,color);
                scr_putpixel_zoom(x+1,y+1,color);
        }
}


//Refresco pantalla sin rainbow para prism
void screen_prism_refresca_pantalla_comun(void)
{
        int x,y,bit;
        z80_int direccion,dir_atributo;
        z80_byte byte_leido;
        int color=0;
        int fila;
        //int zx,zy;

        z80_byte attribute,ink,paper,bright,flash,aux;


       z80_byte *screen=get_base_mem_pantalla();

        //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
        z80_byte x_hi;

        for (y=0;y<192;y++) {
                //direccion=16384 | devuelve_direccion_pantalla(0,y);

                //direccion=16384 | screen_addr_table[(y<<5)];
                direccion=screen_addr_table[(y<<5)];


                fila=y/8;
                dir_atributo=6144+(fila*32);
                for (x=0,x_hi=0;x<32;x++,x_hi +=8) {

		        //int dibujar=0;

			if (1==1) {

                                byte_leido=screen[direccion];
                                attribute=screen[dir_atributo];


                                ink=attribute &7;
                                paper=(attribute>>3) &7;
				bright=(attribute) &64;
                                flash=(attribute)&128;
                                if (flash) {
                                        //intercambiar si conviene
                                        if (estado_parpadeo.v) {
                                                aux=paper;
                                                paper=ink;
                                                ink=aux;
                                        }
                                }

                                if (bright) {
                                        ink +=8;
                                        paper +=8;
                                }

                                for (bit=0;bit<8;bit++) {

                                        color= ( byte_leido & 128 ? ink : paper );

					//Por cada pixel, hacer *2s en ancho y alto.
					//Esto es muy simple dado que no soporta modo rainbow y solo el estandard 256x192
					screen_prism_refresca_pantalla_comun_prism((x_hi+bit)*2,y*2,color);
					/*
                                        scr_putpixel_zoom((x_hi+bit)*2,y*2,color);
                                        scr_putpixel_zoom((x_hi+bit)*2,y*2+1,color);
                                        scr_putpixel_zoom((x_hi+bit)*2+1,y*2,color);
                                        scr_putpixel_zoom((x_hi+bit)*2+1,y*2+1,color);
					*/

                                        byte_leido=byte_leido<<1;
                                }
                        }

                        //temp
                        //else {
                        //      printf ("no refrescamos zona x %d fila %d\n",x,fila);
                        //}


                        direccion++;
                        dir_atributo++;
                }

        }

}




void screen_prism_refresca_no_rainbow(void)
{

	if (border_enabled.v) {
		//ver si hay que refrescar border
		if (modificado_border.v) {
			int color;
			color=out_254 & 7;


			if (color==0) {
				//tiene que venir de la tabla ula2 de 256 colores
				color=PRISM_INDEX_FIRST_COLOR+prism_palette_two[get_prism_ula2_border_colour()];
				//printf ("Color prism %d Index 12 bit: %d   Colour RGB: 0x%X\n",prism_ula2_border_colour,prism_palette_two[prism_ula2_border_colour],
				//	spectrum_colortable_normal[  PRISM_INDEX_FIRST_COLOR+prism_palette_two[prism_ula2_border_colour]  ] );
			}

			//En caso de no rainbow y color no 0, la paleta de colores es la misma

			screen_prism_refresca_no_rainbow_border(color);
			modificado_border.v=0;
		}

	}

	screen_prism_refresca_pantalla_comun();

}



void screen_prism_refresca_rainbow(void) {

	int ancho,alto;

        ancho=get_total_ancho_rainbow();
        alto=get_total_alto_rainbow();

        int x,y,bit;

        //margenes de zona interior de pantalla. Para overlay menu
        int margenx_izq=screen_total_borde_izquierdo*border_enabled.v;
        int margenx_der=screen_total_borde_izquierdo*border_enabled.v+512;
        int margeny_arr=screen_borde_superior*border_enabled.v;
        int margeny_aba=screen_borde_superior*border_enabled.v+384;


        //para overlay menu tambien
        //int fila;
        //int columna;

        z80_int color_pixel;
        z80_int *puntero;

        puntero=rainbow_buffer;
        int dibujar;

	int menu_x,menu_y;

        for (y=0;y<alto;y++) {
                for (x=0;x<ancho;x+=8) {
                        dibujar=1;

                        //Ver si esa zona esta ocupada por texto de menu u overlay

                        if (y>=margeny_arr && y<margeny_aba && x>=margenx_izq && x<margenx_der) {



                                //normalmente a 48
                                //int screen_total_borde_izquierdo;

				dibujar=0;
				menu_x=(x-margenx_izq)/8;
				menu_y=(y-margeny_arr)/8;
				//if (menu_x>31) dibujar=1;
				//else if (menu_y>23) dibujar=1;
				if (scr_ver_si_refrescar_por_menu_activo(menu_x,menu_y)) dibujar=1;

                        }


                        if (dibujar==1) {

                                        for (bit=0;bit<8;bit++) {


                                                //printf ("prism refresca x: %d y: %d\n",x,y);

                                                color_pixel=*puntero++;

                                                scr_putpixel_zoom_rainbow(x+bit,y,color_pixel);
                                        }
                        }
                        else puntero+=8;

                }
        }


}




void screen_prism_refresca_pantalla(void) {

                //modo clasico. sin rainbow
                if (rainbow_enabled.v==0) {
                        screen_prism_refresca_no_rainbow();
                }

                else {
                        //modo rainbow - real video
                        //en spectrum normal era: scr_refresca_pantalla_rainbow_comun();


			screen_prism_refresca_rainbow();
                }

}


void screen_tbblue_refresca_pantalla(void)
{

                //modo clasico. sin rainbow
                if (rainbow_enabled.v==0) {
                        screen_tbblue_refresca_no_rainbow();
                }

                else {
                        //modo rainbow - real video
                        //en spectrum normal era: scr_refresca_pantalla_rainbow_comun();
//scr_refresca_pantalla_rainbow_comun(); //Se puede usar esta funcion comun a todos

			screen_tbblue_refresca_rainbow();
                }

}

void clear_putpixel_cache(void)
{

#ifdef PUTPIXELCACHE

    if (putpixel_cache==NULL) return;

	debug_printf (VERBOSE_INFO,"Clearing putpixel cache");




	int tamanyo_y;

	tamanyo_y=screen_get_window_size_height_no_zoom_border_en();

	if (video_interlaced_mode.v) tamanyo_y *=2;

	int tamanyo_x;

	tamanyo_x=screen_get_window_size_width_no_zoom_border_en();

	if (timex_si_modo_512() ) tamanyo_x *=2;


	//printf ("Clearing putpixel cache %d X %d\n",tamanyo_x,tamanyo_y);
	/*int x,y;
	int indice=0;
	
	for (y=0;y<tamanyo_y;y++) {
		for (x=0;x<tamanyo_x;x++) {
			//cambiar toda la cache
			//ponemos cualquier valor que no pueda existir, para invalidarla
			putpixel_cache[indice]=65535;

			indice++;
		}
	}*/

	//Alternativa con memset mas rapido
	int longitud=tamanyo_y*tamanyo_x*2; //*2 porque es un z80_int
	memset(putpixel_cache,255,longitud);

	//printf ("clear putpixel cache get_total_ancho_rainbow=%d get_total_alto_rainbow=%d \n",get_total_ancho_rainbow(),get_total_alto_rainbow() );
#endif

}

//putpixel escalandolo al zoom necesario y teniendo en cuenta toda la pantalla entera (rainbow)
//y con cache
//por tanto, (0,0) = arriba izquierda del border
void scr_putpixel_zoom_rainbow_mas_de_uno(int x,int y,unsigned int color)
{

#ifdef PUTPIXELCACHE
	int indice_cache;

	indice_cache=(get_total_ancho_rainbow()*y)+x;

	if (putpixel_cache[indice_cache]==color) return;

	//printf ("not in cache: x %d y %d\n",x,y);
	//put_putpixel_cache(indice_cache,color);
	putpixel_cache[indice_cache]=color;
#endif

        int zx,zy;
        int xzoom=x*zoom_x;
        int yzoom=y*zoom_y;


        //Escalado a zoom indicado
        for (zx=0;zx<zoom_x;zx++) {
        	for (zy=0;zy<zoom_y;zy++) {
                        scr_putpixel(xzoom+zx,yzoom+zy,color);
                }
        }

}


//putpixel con zoom y multiple de 2 y teniendo en cuenta el interlaced
void scr_putpixel_zoom_rainbow_interlaced_zoom_two(int x,int y,unsigned int color)
{
	int zyinicial=( (interlaced_numero_frame & 1)==1 ? 1 : 0);

	//interlaced mode, linea impar mas oscura
	if (zyinicial && video_interlaced_scanlines.v) color +=16;

	//printf ("%d\n",zyinicial);

	y=y*2;

#ifdef PUTPIXELCACHE
        int indice_cache;


	//putpixel cache en caso de interlaced zoom y*2 tiene doble de alto
        indice_cache=(get_total_ancho_rainbow()*(y+zyinicial) )+x;


        if (putpixel_cache[indice_cache]==color) return;

        //printf ("not in cache: x %d y %d\n",x,y);
        //put_putpixel_cache(indice_cache,color);
        putpixel_cache[indice_cache]=color;
#endif

        int zx,zy;
        int xzoom=x*zoom_x;

	int zoom_y_result=zoom_y/2;
        int yzoom=(y+zyinicial)*zoom_y_result;



        //Escalado a zoom indicado
        for (zx=0;zx<zoom_x;zx++) {
                for (zy=0;zy<zoom_y_result;zy++) {
                        scr_putpixel(xzoom+zx,yzoom+zy,color);
                }
		//scr_putpixel(xzoom+zx,y,color);
        }

}

//putpixel escalandolo con zoom 1 - sin escalado
//y con cache
//por tanto, (0,0) = arriba izquierda del border
void scr_putpixel_zoom_rainbow_uno(int x,int y,unsigned int color)
{

#ifdef PUTPIXELCACHE
        int indice_cache;

        indice_cache=(get_total_ancho_rainbow()*y)+x;

        if (putpixel_cache[indice_cache]==color) return;

        //printf ("not in cache: x %d y %d\n",x,y);
        //put_putpixel_cache(indice_cache,color);
        putpixel_cache[indice_cache]=color;
#endif

	scr_putpixel(x,y,color);
}


//putpixel escalandolo al zoom necesario y teniendo en cuenta el border
//por tanto, (0,0) = dentro de pantalla
void scr_putpixel_zoom_mas_de_uno(int x,int y,unsigned int color)
{

#ifdef PUTPIXELCACHE
	int indice_cache;

	if (MACHINE_IS_Z88) {
		indice_cache=(get_total_ancho_rainbow()*(y)) + x;
	}

	else if (MACHINE_IS_CPC) {
		indice_cache=(get_total_ancho_rainbow()*(CPC_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + CPC_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
        }

	else if (MACHINE_IS_PRISM) {
		indice_cache=(get_total_ancho_rainbow()*(PRISM_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + PRISM_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
        }

				else if (MACHINE_IS_TSCONF) {
					indice_cache=(get_total_ancho_rainbow()*(TSCONF_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + TSCONF_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
			        }

				else if (MACHINE_IS_TBBLUE) {
					indice_cache=(get_total_ancho_rainbow()*(TBBLUE_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
			        }							

	else if (MACHINE_IS_SAM) {
                indice_cache=(get_total_ancho_rainbow()*(SAM_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + SAM_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
        }

				else if (MACHINE_IS_QL) {
											indice_cache=(get_total_ancho_rainbow()*(QL_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + QL_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
							}

	else {
		indice_cache=(get_total_ancho_rainbow()*(screen_borde_superior*border_enabled.v+y)) + screen_total_borde_izquierdo*border_enabled.v+x;
	}

	if (putpixel_cache[indice_cache]==color) return;

	//printf ("scr_putpixel_zoom not in cache: x %d y %d indice_cache=%d \n",x,y,indice_cache);
	//put_putpixel_cache(indice_cache,color);
	putpixel_cache[indice_cache]=color;
#endif

        int zx,zy;
	int offsetx,offsety;

	if (MACHINE_IS_Z88) {
		offsetx=0;
		offsety=0;
	}

	else if (MACHINE_IS_CPC) {
		offsetx=CPC_LEFT_BORDER*border_enabled.v;
                offsety=CPC_TOP_BORDER*border_enabled.v;
	}

	else if (MACHINE_IS_PRISM) {
		offsetx=PRISM_LEFT_BORDER*border_enabled.v;
                offsety=PRISM_TOP_BORDER*border_enabled.v;
	}

	else if (MACHINE_IS_TSCONF) {
		offsetx=TSCONF_LEFT_BORDER*border_enabled.v;
                offsety=TSCONF_TOP_BORDER*border_enabled.v;
	}

	else if (MACHINE_IS_TBBLUE) {
		offsetx=TBBLUE_LEFT_BORDER*border_enabled.v;
                offsety=TBBLUE_TOP_BORDER*border_enabled.v;
	}	

        else if (MACHINE_IS_SAM) {
                offsetx=SAM_LEFT_BORDER*border_enabled.v;
                offsety=SAM_TOP_BORDER*border_enabled.v;
        }

				else if (MACHINE_IS_QL) {
								offsetx=QL_LEFT_BORDER*border_enabled.v;
								offsety=QL_TOP_BORDER*border_enabled.v;
				}

				else if (MACHINE_IS_ZX8081ACE) {
								offsetx=LEFT_BORDER*border_enabled.v;
								offsety=ZX8081ACE_TOP_BORDER*border_enabled.v;
				}

	else {
	        offsetx=LEFT_BORDER*border_enabled.v;
        	offsety=TOP_BORDER*border_enabled.v;
	}
        int xzoom=x*zoom_x;
        int yzoom=y*zoom_y;



	//Escalado a zoom indicado
        for (zx=0;zx<zoom_x;zx++) {
        	for (zy=0;zy<zoom_y;zy++) {
                	scr_putpixel(offsetx+xzoom+zx,offsety+yzoom+zy,color);
		}
	}
}

//putpixel escalandolo a zoom 1 -> no zoom
//por tanto, (0,0) = dentro de pantalla
void scr_putpixel_zoom_uno(int x,int y,unsigned int color)
{

#ifdef PUTPIXELCACHE
        int indice_cache;

	if (MACHINE_IS_Z88) {
		indice_cache=(get_total_ancho_rainbow()*(y)) + x;
	}

	else if (MACHINE_IS_CPC) {
                indice_cache=(get_total_ancho_rainbow()*(CPC_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + CPC_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
		//printf ("total ancho rainbow : %d\n",get_total_ancho_rainbow() );
		//printf ("get_total_ancho_rainbow_cached: %d\n",get_total_ancho_rainbow_cached);
		//sleep(1);
        }

	else if (MACHINE_IS_PRISM) {
                indice_cache=(get_total_ancho_rainbow()*(PRISM_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + PRISM_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
		//printf ("total ancho rainbow : %d\n",get_total_ancho_rainbow() );
		//printf ("get_total_ancho_rainbow_cached: %d\n",get_total_ancho_rainbow_cached);
		//sleep(1);
        }

				else if (MACHINE_IS_TSCONF) {
            indice_cache=(get_total_ancho_rainbow()*(TSCONF_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + TSCONF_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;

	        }

				else if (MACHINE_IS_TBBLUE) {
            indice_cache=(get_total_ancho_rainbow()*(TBBLUE_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;

	        }					

        else if (MACHINE_IS_SAM) {
                indice_cache=(get_total_ancho_rainbow()*(SAM_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + SAM_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
                //printf ("total ancho rainbow : %d\n",get_total_ancho_rainbow() );
                //printf ("get_total_ancho_rainbow_cached: %d\n",get_total_ancho_rainbow_cached);
                //sleep(1);
        }

				else if (MACHINE_IS_QL) {
								indice_cache=(get_total_ancho_rainbow()*(QL_TOP_BORDER_NO_ZOOM*border_enabled.v+y)) + QL_LEFT_BORDER_NO_ZOOM*border_enabled.v+x;
								//printf ("total ancho rainbow : %d\n",get_total_ancho_rainbow() );
								//printf ("get_total_ancho_rainbow_cached: %d\n",get_total_ancho_rainbow_cached);
								//sleep(1);
				}

	else {
        	indice_cache=(get_total_ancho_rainbow()*(screen_borde_superior*border_enabled.v+y)) + screen_total_borde_izquierdo*border_enabled.v+x;
	}

        if (putpixel_cache[indice_cache]==color) return;

        //printf ("scr_putpixel_zoom color %d not in cache: x %d y %d indice_cache=%d contenido=%d\n",color,x,y,indice_cache,putpixel_cache[indice_cache]);
        //put_putpixel_cache(indice_cache,color);
        putpixel_cache[indice_cache]=color;
#endif

	        int offsetx,offsety;


	if (MACHINE_IS_Z88) {
		offsetx=0;
		offsety=0;
	}

	else if (MACHINE_IS_CPC) {
                offsetx=CPC_LEFT_BORDER*border_enabled.v;
                offsety=CPC_TOP_BORDER*border_enabled.v;
        }

	else if (MACHINE_IS_PRISM) {
                offsetx=PRISM_LEFT_BORDER*border_enabled.v;
                offsety=PRISM_TOP_BORDER*border_enabled.v;
        }

				else if (MACHINE_IS_TSCONF) {
			                offsetx=TSCONF_LEFT_BORDER*border_enabled.v;
			                offsety=TSCONF_TOP_BORDER*border_enabled.v;
			        }

				else if (MACHINE_IS_TBBLUE) {
			                offsetx=TBBLUE_LEFT_BORDER*border_enabled.v;
			                offsety=TBBLUE_TOP_BORDER*border_enabled.v;
			        }							

        else if (MACHINE_IS_SAM) {
                offsetx=SAM_LEFT_BORDER*border_enabled.v;
                offsety=SAM_TOP_BORDER*border_enabled.v;
        }

				else if (MACHINE_IS_QL) {
								offsetx=QL_LEFT_BORDER*border_enabled.v;
								offsety=QL_TOP_BORDER*border_enabled.v;
				}

				else if (MACHINE_IS_ZX8081ACE) {
								offsetx=LEFT_BORDER*border_enabled.v;
								offsety=ZX8081ACE_TOP_BORDER*border_enabled.v;
				}

	else {
        offsetx=LEFT_BORDER*border_enabled.v;
        offsety=TOP_BORDER*border_enabled.v;
	}

	scr_putpixel(offsetx+x,offsety+y,color);
}


void set_putpixel_zoom(void)
{
	if (zoom_x==1 && zoom_y==1) {
		scr_putpixel_zoom=scr_putpixel_zoom_uno;
		scr_putpixel_zoom_rainbow=scr_putpixel_zoom_rainbow_uno;
		debug_printf (VERBOSE_INFO,"Setting putpixel functions to zoom 1");
	}

	//zoom_y multiple de dos (valor par) y interlaced
	else if (zoom_y>=2 && (zoom_y&1)==0 && video_interlaced_mode.v) {
		scr_putpixel_zoom=scr_putpixel_zoom_mas_de_uno;
                scr_putpixel_zoom_rainbow=scr_putpixel_zoom_rainbow_interlaced_zoom_two;
		debug_printf (VERBOSE_INFO,"Setting putpixel functions to interlaced zoom multiple of two");
	}

	else {
		scr_putpixel_zoom=scr_putpixel_zoom_mas_de_uno;
		scr_putpixel_zoom_rainbow=scr_putpixel_zoom_rainbow_mas_de_uno;
		debug_printf (VERBOSE_INFO,"Setting putpixel functions to variable zoom");
	}
}

int ancho_layer_menu_machine=0;
int alto_layer_menu_machine=0;


z80_int *buffer_layer_machine=NULL;
z80_int *buffer_layer_menu=NULL;
int tamanyo_memoria_buffer_layer_menu=0;


//Especie de semaforo que indica:
//Pantalla esta siendo actualizada
//o
//Se esta reasignando layers de menu machine
//No se pueden dar las dos condiciones a la vez, pues si esta por debajo redibujando y reasignamos layers, petara todo
int sem_screen_refresh_reallocate_layers=0;


int running_realloc=0;

void scr_reallocate_layers_menu(int ancho,int alto)
{

	//ancho +=screen_ext_desktop_enabled*scr_driver_can_ext_desktop()*screen_ext_desktop_width*zoom_x;

	debug_printf (VERBOSE_DEBUG,"Allocating memory for menu layers %d X %d",ancho,alto);
	//debug_exec_show_backtrace();

	if (!menu_overlay_activo) {
		//No estrictamente necesario, pero evitamos usos de buffer_layer_menu o machine (especialmente desde thread de redibujo de cocoa) mientras se reasignan layers
		debug_printf (VERBOSE_DEBUG,"Returning reallocate layers as there are no active menu");
		return;
	}

	//Si el tamanyo anterior es igual que ahora, no tiene sentido tocarlo
	if (ancho_layer_menu_machine==ancho && alto_layer_menu_machine==alto) {
		debug_printf (VERBOSE_DEBUG,"Returning reallocate layers as the current size is the same as the new (%d X %d)",ancho,alto);
		return;
	}


	if (running_realloc) {
		debug_printf (VERBOSE_DEBUG,"Another realloc already running. sem_screen_refresh_reallocate_layers: %d width %d height %d",sem_screen_refresh_reallocate_layers,ancho,alto);
		return;
	}

  if (running_realloc) debug_printf (VERBOSE_DEBUG,"Reallocate layers, screen currently reallocating... wait");

	while (running_realloc) {
		//printf ("screen currently reallocating... wait\n");
		usleep(100);
	}	

	running_realloc=1;

	//No se puede reasignar layers si esta por debajo refrescando pantalla. Esperar a que finalice
	if (sem_screen_refresh_reallocate_layers) debug_printf (VERBOSE_DEBUG,"Reallocate layers, screen currently redrawing... wait");
	while (sem_screen_refresh_reallocate_layers) {
		//printf ("screen currently redrawing... wait\n");
		usleep(100);
	}

	sem_screen_refresh_reallocate_layers=1;




	ancho_layer_menu_machine=ancho;
	alto_layer_menu_machine=alto;	

	//printf ("antes buffer_layer_machine %p buffer_layer_menu %p\n",buffer_layer_machine,buffer_layer_menu);
	
	//Liberar si conviene
	if (buffer_layer_machine!=NULL) {
		//printf ("liberando buffer_layer_machine\n");
		free (buffer_layer_machine);
		buffer_layer_machine=NULL;
	}

	//printf ("despues si liberar buffer_layer_machine\n");
	
	if (buffer_layer_menu!=NULL) {
		//printf ("Liberando buffer_layer_menu\n");
		free(buffer_layer_menu);
		buffer_layer_menu=NULL;
		tamanyo_memoria_buffer_layer_menu=0;
	}


	//printf ("despues si liberar buffer_layer_menu\n");

	//Asignar
	int numero_elementos=ancho_layer_menu_machine*alto_layer_menu_machine;
	int size_layers=numero_elementos*sizeof(z80_int);

	//printf ("Asignando layer tamanyo %d\n",size_layers);

	buffer_layer_machine=malloc(size_layers);
	buffer_layer_menu=malloc(size_layers);
	tamanyo_memoria_buffer_layer_menu=numero_elementos;


	//printf ("despues buffer_layer_machine %p buffer_layer_menu %p\n",buffer_layer_machine,buffer_layer_menu);

	if (buffer_layer_machine==NULL || buffer_layer_menu==NULL) {
		//printf ("Cannot allocate memory for menu layers\n");
		cpu_panic("Cannot allocate memory for menu layers");	
	}


	//Inicializar layers. Esto puede dar problemas si se llama aqui sin tener el driver de video inicializado del todo
	//por esto hay que tener cuidado en que cuando se llama aqui, esta todo correcto
	//Si esto da problemas, quiza quitar el scr_clear_layer_menu y hacerlo mas tarde
	//o quiza scr_clear_layer_menu no deberia llamar a scr_redraw_machine_layer(); (y llamar a ahi desde otro sitio)

	scr_clear_layer_menu();


	sem_screen_refresh_reallocate_layers=0;	

	running_realloc=0;

}

void scr_init_layers_menu(void)
{
	int ancho,alto;

	ancho=screen_get_window_size_width_zoom_border_en();

	ancho +=screen_ext_desktop_enabled*scr_driver_can_ext_desktop()*screen_ext_desktop_width*zoom_x;

  alto=screen_get_window_size_height_zoom_border_en();

	scr_reallocate_layers_menu(ancho,alto);

	//printf("scr_reallocate_layers_menu ancho %d alto: %d\n",ancho,alto);

}

void scr_putpixel_layer_menu_no_zoom(int x,int y,int color)
{
	int xzoom=x;
	int yzoom=y;

      
	int xdestino=xzoom;
	int ydestino=yzoom;
	//scr_putpixel(xzoom+zx,yzoom+zy,color);
	if (buffer_layer_menu==NULL) {
		//printf ("scr_putpixel_layer_menu NULL\n"); //?????
	}
	else {
		//Proteger que no se salga de rango
		int offset=ydestino*ancho_layer_menu_machine+xdestino;

		if (offset<tamanyo_memoria_buffer_layer_menu) {
		
			buffer_layer_menu[offset]=color;

			//Y hacer mix
			screen_putpixel_mix_layers(xdestino,ydestino); 														
		}

		else {
			//printf ("fuera de rango %d %d\n",xdestino,ydestino);
		}
	}

												  
                
        
}

void scr_putpixel_layer_menu(int x,int y,int color)
{
	int xzoom=x*zoom_x;
	int yzoom=y*zoom_y;

	int zx,zy;


	//Escalado a zoom indicado
	for (zx=0;zx<zoom_x;zx++) {
		for (zy=0;zy<zoom_y;zy++) {
			int xdestino=xzoom+zx;
			int ydestino=yzoom+zy;
			//scr_putpixel(xzoom+zx,yzoom+zy,color);
			if (buffer_layer_menu==NULL) {
				//printf ("scr_putpixel_layer_menu NULL\n"); //?????
			}
			else {
				//Proteger que no se salga de rango
				int offset=ydestino*ancho_layer_menu_machine+xdestino;

				if (offset<tamanyo_memoria_buffer_layer_menu) {
				
					buffer_layer_menu[offset]=color;

					//Y hacer mix
					screen_putpixel_mix_layers(xdestino,ydestino); 
				}
				else {
					//printf ("fuera de rango %d %d\n",xdestino,ydestino);
				}

			}

											
		}
	}
}

void scr_redraw_machine_layer(void)
{

	debug_printf (VERBOSE_DEBUG,"Redraw machine layer");


	if (scr_putpixel==NULL) return;	

		if (buffer_layer_machine==NULL) return;
		if (!si_complete_video_driver() ) return;	

	int x,y;
	//int posicion=0;

	int ancho_layer=ancho_layer_menu_machine;
	int alto_layer=alto_layer_menu_machine;

	int ancho_ventana=screen_get_window_size_width_zoom_border_en();
  int alto_ventana=screen_get_window_size_height_zoom_border_en();	

	//Si son tamaños distintos, no hacer nada
	if (ancho_ventana!=ancho_layer || alto_ventana!=alto_layer) {
		//printf ("Window size does not match menu layers size\n");
		return;
	}

  
	//Obtener el tamaño menor
	/*
	Por que hacemos esto?
	porque vamos a recorrer el layer de maquina, entero, y redibujar cada pixel en pantalla
	Dado que puede haber diferencias de tamaños entre ambos (al redimensionar ventanas) nos limitamos
	a la zona mas pequeña
	*/
	int ancho,alto;
	if (ancho_layer<ancho_ventana) ancho=ancho_layer;
	else ancho=ancho_ventana;

	if (alto_layer<alto_ventana) alto=alto_layer;
	else alto=alto_ventana;

	for (y=0;y<alto;y++) {
		for (x=0;x<ancho;x++) {
			//printf ("x %d y %d p %p\n",x,y,scr_putpixel_final);
			int posicion=ancho_layer_menu_machine*y+x;
			z80_int color=buffer_layer_machine[posicion];
			scr_putpixel_final(x,y,color);
		}
	}


}

unsigned int screen_get_color_from_rgb(unsigned char red,unsigned char green,unsigned char blue)
{
	return (red<<16)|(green<<8)|blue;
}

void screen_reduce_color_rgb(int percent,unsigned int *red,unsigned int *green,unsigned int *blue)
{
	*red=((*red)*percent)/100;
	*green=((*green)*percent)/100;
	*blue=((*blue)*percent)/100;
}

void screen_get_rgb_components(unsigned int color_rgb,unsigned int *red,unsigned int *green,unsigned int *blue)
{
	*blue=color_rgb & 0xFF;
	color_rgb >>=8;

	*green=color_rgb & 0xFF;
	color_rgb >>=8;

	*red=color_rgb & 0xFF;

}

/*
0=Menu por encima de maquina, si no es transparente
1=Menu por encima de maquina, si no es transparente. Y Color Blanco con brillo es transparente
2=Mix de los dos colores, con control de transparecnai


Otro setting=Maquina bajar brillo, se combina con los anteriores
*/
int screen_menu_mix_method=0; //Por defecto, no mezclar
int screen_menu_mix_transparency=10; //Dice la opacidad de la capa de menu.  Si 100, transparente total. Si 0, opaco total

//Si reducimos brillo de la maquina al abrir el menu, solo vale para metodos 0  y 1
z80_bit screen_menu_reduce_bright_machine={0};

//Color en blanco de y negro de maquina con menu abierto cuando multitask esta off
z80_bit screen_machine_bw_no_multitask={0};

char *screen_menu_mix_methods_strings[]={
	"Over","Chroma","Mix"
};

unsigned int screen_convert_rgb_to_bw(unsigned int color_rgb)
{
					//blanco y negro
				if (!menu_multitarea && menu_abierto && screen_machine_bw_no_multitask.v) {
unsigned int red_machine,green_machine,blue_machine;

					screen_get_rgb_components(color_rgb,&red_machine,&green_machine,&blue_machine);	
					int color_gris=rgb_to_grey(red_machine,green_machine,blue_machine);
					red_machine=green_machine=blue_machine=color_gris;
					color_rgb=screen_get_color_from_rgb(red_machine,green_machine,blue_machine);
				}


	return color_rgb;
}

//Mezclar dos pixeles de layer menu y layer maquina
void screen_putpixel_mix_layers(int x,int y)
{
        //Obtener los dos pixeles
        z80_int color_menu=buffer_layer_menu[y*ancho_layer_menu_machine+x];
        z80_int color_machine=buffer_layer_machine[y*ancho_layer_menu_machine+x];


				unsigned int color_rgb;

				unsigned int color_rgb_menu,color_rgb_maquina;

				unsigned int red_menu,green_menu,blue_menu;
				unsigned int red_machine,green_machine,blue_machine;

				unsigned char red_final,green_final,blue_final;

				z80_int color_indexado;

				int metodo_mix=screen_menu_mix_method & 3;


				switch (metodo_mix) {


					case 1:
        		//Si es transparente menu, o color 15, poner machine
        		if (color_menu==SCREEN_LAYER_TRANSPARENT_MENU || color_menu==ESTILO_GUI_PAPEL_NORMAL) {
							color_indexado=color_machine;
							color_rgb=spectrum_colortable[color_indexado];

							color_rgb=screen_convert_rgb_to_bw(color_rgb);

							if (screen_menu_reduce_bright_machine.v) {
								screen_get_rgb_components(color_rgb,&red_machine,&green_machine,&blue_machine);	
								screen_reduce_color_rgb(50,&red_machine,&green_machine,&blue_machine);	
								color_rgb=screen_get_color_from_rgb(red_machine,green_machine,blue_machine);						
							}							
						}
        		else {
							color_indexado=color_menu;
							color_rgb=spectrum_colortable[color_indexado];
						}

											
					break;

					case 2:

						//Mezclar los dos con control de opacidad, siempre que color_menu no sea transparente
						if (color_menu==SCREEN_LAYER_TRANSPARENT_MENU) {
							color_rgb=spectrum_colortable[color_machine];
							color_rgb=screen_convert_rgb_to_bw(color_rgb);
						}							

						else {
							color_rgb_menu=spectrum_colortable[color_menu];
							

							color_rgb_maquina=spectrum_colortable[color_machine];
							color_rgb_maquina=screen_convert_rgb_to_bw(color_rgb_maquina);

							screen_get_rgb_components(color_rgb_menu,&red_menu,&green_menu,&blue_menu);
							screen_get_rgb_components(color_rgb_maquina,&red_machine,&green_machine,&blue_machine);


							//Mezclarlos			

							screen_reduce_color_rgb(100-screen_menu_mix_transparency,&red_menu,&green_menu,&blue_menu);


							int machine_transparency=screen_menu_mix_transparency;
							screen_reduce_color_rgb(machine_transparency,&red_machine,&green_machine,&blue_machine);

							red_final=red_menu+red_machine;
							green_final=green_menu+green_machine;
							blue_final=blue_menu+blue_machine;

							color_rgb=screen_get_color_from_rgb(red_final,green_final,blue_final);
						}

					break;

					default:
				
        		//Si es transparente menu, poner machine
        		if (color_menu==SCREEN_LAYER_TRANSPARENT_MENU) {
							color_indexado=color_machine;
							color_rgb=spectrum_colortable[color_indexado];

							color_rgb=screen_convert_rgb_to_bw(color_rgb);

							if (screen_menu_reduce_bright_machine.v) {
								screen_get_rgb_components(color_rgb,&red_machine,&green_machine,&blue_machine);	
								screen_reduce_color_rgb(50,&red_machine,&green_machine,&blue_machine);	
								color_rgb=screen_get_color_from_rgb(red_machine,green_machine,blue_machine);						
							}

						}

        		else {
							color_indexado=color_menu;
							color_rgb=spectrum_colortable[color_indexado];
						}


					break;					

				}

				//blanco y negro
				//color_rgb=screen_convert_rgb_to_bw(color_rgb);
				/*if (!menu_multitarea) {
					screen_get_rgb_components(color_rgb,&red_machine,&green_machine,&blue_machine);	
					int color_gris=rgb_to_grey(red_machine,green_machine,blue_machine);
					red_machine=green_machine=blue_machine=color_gris;
					color_rgb=screen_get_color_from_rgb(red_machine,green_machine,blue_machine);
				}*/

				scr_putpixel_final_rgb(x,y,color_rgb);
}



void scr_clear_layer_menu(void)
{
		if (buffer_layer_menu==NULL) return;
		if (!si_complete_video_driver() ) return;

		debug_printf (VERBOSE_DEBUG,"Clearing layer menu");
		//sleep(1);


		int i;
		int size=ancho_layer_menu_machine*alto_layer_menu_machine;
		//printf ("Clearing layer size %d. buffer_layer_menu %p realloc layers %d\n",size,buffer_layer_menu,sem_screen_refresh_reallocate_layers);
		//size/=16;

		//z80_int *initial_p;



		//initial_p=buffer_layer_menu;
		for (i=0;i<size;i++) {
			//if (initial_p!=buffer_layer_menu) {
			//if (buffer_layer_menu==NULL) {
			//if (sem_screen_refresh_reallocate_layers) {
			//	printf ("---i %d %p realloc layers %d\n",i,buffer_layer_menu,sem_screen_refresh_reallocate_layers);
			//	sleep(5);
			//}
			buffer_layer_menu[i]=SCREEN_LAYER_TRANSPARENT_MENU; //color transparente	
		}

		//printf ("After Clearing layer size %d. buffer_layer_menu %p\n",size,buffer_layer_menu);

		//printf ("Before clear putpixel cache\n");
		clear_putpixel_cache();
		//printf ("After clear putpixel cache\n");

		//printf ("End clearing layer menu\n");

}


//Hacer un putpixel en la coordenada indicada pero haciendo tan gordo el pixel como diga zoom_level
//Y sin lanzar zoom_x ni zoom_y
//Usado en help keyboard
void scr_putpixel_gui_no_zoom(int x,int y,int color,int zoom_level)
{ 
	//Hacer zoom de ese pixel si conviene
	int incx,incy;
	for (incy=0;incy<zoom_level;incy++) {
		for (incx=0;incx<zoom_level;incx++) {
			//printf("putpixel %d,%d\n",x+incx,y+incy);
			scr_putpixel_layer_menu_no_zoom(x+incx,y+incy,color);
			
		}
	}
}

//Hacer un putpixel en la coordenada indicada pero haciendo tan gordo el pixel como diga zoom_level
void scr_putpixel_gui_zoom(int x,int y,int color,int zoom_level)
{ 
	//Hacer zoom de ese pixel si conviene
	int incx,incy;
	for (incy=0;incy<zoom_level;incy++) {
		for (incx=0;incx<zoom_level;incx++) {
			//printf("putpixel %d,%d\n",x+incx,y+incy);
			scr_putpixel_layer_menu(x+incx,y+incy,color);
			//if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(x+incx,y+incy,color);

			//else scr_putpixel_zoom(x+incx,y+incy,color);
		}
	}
}

//Usado solo antes de iniciar emulador
int scrgeneric_driver_can_ext_desktop(void)
{
	return 0;
}


void screen_init_ext_desktop(void)
{
	scr_driver_can_ext_desktop=scrgeneric_driver_can_ext_desktop;
}

//Si la opcion esta habilitada y el driver lo permite. o sea a efectos practicos, que la interfaz muestra zxdesktop
int if_zxdesktop_enabled_and_driver_allows(void) 
{
    if (screen_ext_desktop_enabled && scr_driver_can_ext_desktop() ) return 1;
    else return 0;
}

//Gestion de extension de desktop a ventana. Antes se llamaba extended desktop. Ahora es ZX Desktop
int screen_ext_desktop_enabled=0;


int screen_ext_desktop_width=256; //se multiplicara por zoom

//valor anterior si el usuario lo ha ocultado con el boton del footer
//int screen_ext_desktop_width_before_disabling=-1;


int screen_ext_desktop_place_menu=0; //Si abrimos siempre ventanas en la zona de desktop por defecto

int screen_get_ext_desktop_width_no_zoom(void)
{
	return screen_ext_desktop_enabled*scr_driver_can_ext_desktop()*screen_ext_desktop_width;
}

int screen_get_ext_desktop_width_zoom(void)
{
	return screen_get_ext_desktop_width_no_zoom()*zoom_x;
}

int screen_get_ext_desktop_start_x(void)
{
	return screen_get_emulated_display_width_zoom_border_en();
}

void scr_return_margenxy_rainbow(int *margenx_izq,int *margeny_arr)
{

        *margenx_izq=screen_total_borde_izquierdo*border_enabled.v;
        *margeny_arr=screen_borde_superior*border_enabled.v;

if (MACHINE_IS_Z88) {
		//no hay border. estas variables se leen en modo rainbow
		*margenx_izq=*margeny_arr=0;
	}

	else if (MACHINE_IS_CPC) {
		*margenx_izq=CPC_LEFT_BORDER_NO_ZOOM*border_enabled.v;
		*margeny_arr=CPC_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

	else if (MACHINE_IS_PRISM) {
		*margenx_izq=PRISM_LEFT_BORDER_NO_ZOOM*border_enabled.v;
		*margeny_arr=PRISM_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

	else if (MACHINE_IS_TSCONF) {
		*margenx_izq=TSCONF_LEFT_BORDER_NO_ZOOM*border_enabled.v;
		*margeny_arr=TSCONF_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

	else if (MACHINE_IS_TBBLUE) {
		*margenx_izq=TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v;
		*margeny_arr=TBBLUE_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

        else if (MACHINE_IS_SAM) {
                *margenx_izq=SAM_LEFT_BORDER_NO_ZOOM*border_enabled.v;
                *margeny_arr=SAM_TOP_BORDER_NO_ZOOM*border_enabled.v;
        }

				else if (MACHINE_IS_QL) {
								*margenx_izq=QL_LEFT_BORDER_NO_ZOOM*border_enabled.v;
								*margeny_arr=QL_TOP_BORDER_NO_ZOOM*border_enabled.v;
				}

}

//Retorna 0 si ese pixel no se debe mostrar debido a tamaño de caracter < 8
int scr_putchar_menu_comun_zoom_reduce_charwidth(int bit)
{

	//Reducciones segun cada tamaño de letra
	int saltar_pixeles_size7;
	int saltar_pixeles_size6[2];
	int saltar_pixeles_size5[3];


	//Escalados por defecto
	//Saltar primer pixel en caso tamaño 7
	saltar_pixeles_size7=0;

	//Saltar primer pixel y ultimo pixel en caso tamaño 6
	saltar_pixeles_size6[0]=0;	
	saltar_pixeles_size6[1]=7;	
	
	//Saltar primer pixel y ultimos pixeles en caso tamaño 5
	saltar_pixeles_size5[0]=0;	
	saltar_pixeles_size5[1]=6;
	saltar_pixeles_size5[2]=7;	

	//Segun tipo de letra
	if (char_set==char_set_msx)	{
		saltar_pixeles_size7=7;

		saltar_pixeles_size6[0]=7;	
		saltar_pixeles_size6[1]=6;		

		saltar_pixeles_size5[0]=7;	
		saltar_pixeles_size5[1]=6;			
		saltar_pixeles_size5[2]=5;			
	}

	if (char_set==char_set_z88)	{
		saltar_pixeles_size7=0;

		saltar_pixeles_size6[0]=0;	
		saltar_pixeles_size6[1]=1;	
		
		saltar_pixeles_size5[0]=0;	
		saltar_pixeles_size5[1]=1;
		saltar_pixeles_size5[2]=2;			
	}	

	if (char_set==char_set_sam) {
		saltar_pixeles_size7=0;

		saltar_pixeles_size6[0]=0;	
		saltar_pixeles_size6[1]=1;	
		
		saltar_pixeles_size5[0]=0;	
		saltar_pixeles_size5[1]=1;
		saltar_pixeles_size5[2]=7;			
	}

    if (char_set==char_set_retromac) {
        saltar_pixeles_size7=7;

        saltar_pixeles_size6[0]=6;
        saltar_pixeles_size6[1]=7;

        saltar_pixeles_size5[0]=0;
        saltar_pixeles_size5[1]=6;
        saltar_pixeles_size5[2]=7;
    }

    if (char_set==char_set_beos) {
        saltar_pixeles_size7=7;

        saltar_pixeles_size6[0]=6;
        saltar_pixeles_size6[1]=7;

        saltar_pixeles_size5[0]=0;
        saltar_pixeles_size5[1]=6;
        saltar_pixeles_size5[2]=7;
    }

    if (char_set==char_set_amigaos) {
        saltar_pixeles_size7=7;

        saltar_pixeles_size6[0]=0;
        saltar_pixeles_size6[1]=7;
        
        saltar_pixeles_size5[0]=0;
        saltar_pixeles_size5[1]=6;
        saltar_pixeles_size5[2]=7;
    }

    if (char_set==char_set_ataritos) {
        saltar_pixeles_size7=7;

        saltar_pixeles_size6[0]=0;
        saltar_pixeles_size6[1]=7;
        
        saltar_pixeles_size5[0]=0;
        saltar_pixeles_size5[1]=6;
        saltar_pixeles_size5[2]=7;
    }    

    if (char_set==char_set_dos) {
        saltar_pixeles_size7=7;

        saltar_pixeles_size6[0]=6;
        saltar_pixeles_size6[1]=7;

        saltar_pixeles_size5[0]=0;
        saltar_pixeles_size5[1]=6;
        saltar_pixeles_size5[2]=7;
    }

	//Los demas se ajustan bien al escalado por defecto


	if (menu_char_width==8) {
		return 1;
	}

	//Si 7, saltar un pixel
	else if (menu_char_width==7) {
		if (bit==saltar_pixeles_size7) {
			return 0;
		}
	}

	//Si 6, saltar dos pixeles
	else if (menu_char_width==6) {
		if (bit==saltar_pixeles_size6[0] || bit==saltar_pixeles_size6[1]) {
			return 0;
		}
	}

	//Si 5, saltar tres pixeles
	else if (menu_char_width==5) {
		if (bit==saltar_pixeles_size5[0] || bit==saltar_pixeles_size5[1] || bit==saltar_pixeles_size5[2]) {
			return 0;
		}
	}	


	//Por defecto
	return 1;
}


//Muestra un caracter en pantalla, usado en menu
//entrada: caracter
//x,y: coordenadas en x-0..31 e y 0..23 
//inverse si o no
//ink, paper
//y valor de zoom
void scr_putchar_menu_comun_zoom(z80_byte caracter,int x,int y,z80_bit inverse,int tinta,int papel,int zoom_level)
{

	int color;
  z80_byte bit;
  z80_byte line;
  z80_byte byte_leido;

  //printf ("tinta %d papel %d\n",tinta,papel);

  //margenes de zona interior de pantalla. Para modo rainbow
  int margenx_izq;
  int margeny_arr;

	z80_byte *puntero;
	puntero=&char_set[(caracter-32)*8];


	scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

	//temp prueba
	//margenx_izq=margeny_arr=0;

	//Caso de pentagon y en footer
	if (pentagon_timing.v && y>=31) margeny_arr=56*border_enabled.v;
	
  y=y*8;

  for (line=0;line<8;line++,y++) {
		byte_leido=*puntero++;
		if (inverse.v==1) byte_leido = byte_leido ^255;

		int px=0; //Coordenada x del pixel final
		for (bit=0;bit<8;bit++) {
			if (byte_leido & 128 ) color=tinta;
			else color=papel;

   

			byte_leido=(byte_leido&127)<<1;

			//este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)

			int xfinal,yfinal;

			//xfinal=(((x*menu_char_width)+bit)*zoom_level);

			xfinal=(((x*menu_char_width)+px)*zoom_level);
			yfinal=y*zoom_level;


			//No hay que sumar ya los margenes
			/*if (rainbow_enabled.v==1) {
				xfinal +=margenx_izq;

				yfinal +=margeny_arr;
			}*/




			//Hacer zoom de ese pixel si conviene

		
			//Ancho de caracter 8, 7 y 6 pixeles
			if (scr_putchar_menu_comun_zoom_reduce_charwidth(bit)) {
				scr_putpixel_gui_zoom(xfinal,yfinal,color,zoom_level);
				px++;
			}


			/*
			if (menu_char_width==8) {
				scr_putpixel_gui_zoom(xfinal,yfinal,color,zoom_level);
				px++;
			}

			//Si 7, saltar primer pixel a la izquierda
			else if (menu_char_width==7) {
				if (bit!=0) {
					scr_putpixel_gui_zoom(xfinal,yfinal,color,zoom_level);
					px++;
				}
			}

			//Si 6, saltar dos pixeles: primero izquierda y primero derecha
			else if (menu_char_width==6) {
				if (bit!=0 && bit!=7) {
					scr_putpixel_gui_zoom(xfinal,yfinal,color,zoom_level);
					px++;
				}
			}

			//Si 5, saltar tres pixeles: primero izquierda y centro y primero derecha
			else if (menu_char_width==5) {
				if (bit!=0 && bit!=6 && bit!=7) {
					scr_putpixel_gui_zoom(xfinal,yfinal,color,zoom_level);
					px++;
				}
			}
			*/


    }
  }
}


void scr_putchar_footer_comun_zoom(z80_byte caracter,int x,int y,z80_bit inverse,int tinta,int papel)
{
    int color;
    z80_byte bit;
    z80_byte line;
    z80_byte byte_leido;

    //printf ("tinta %d papel %d\n",tinta,papel);

    //margenes de zona interior de pantalla. Para modo rainbow
    int margenx_izq;
    int margeny_arr;

	int zoom_level=1;

	z80_byte *puntero;
	puntero=&char_set[(caracter-32)*8];

    scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

    //Caso de pentagon y en footer
    if (pentagon_timing.v && y>=31) margeny_arr=56*border_enabled.v;

    y=y*8;

    for (line=0;line<8;line++,y++) {
        byte_leido=*puntero++;
        if (inverse.v==1) byte_leido = byte_leido ^255;
        for (bit=0;bit<8;bit++) {
            if (byte_leido & 128 ) color=tinta;
            else color=papel;


            byte_leido=(byte_leido&127)<<1;

            //este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)

            int xfinal,yfinal;

            if (rainbow_enabled.v==1) {
                    //xfinal=(((x*8)+bit)*zoom_level);
                    xfinal=(((x*8)+bit)*zoom_level);
                    xfinal +=margenx_izq;

                    yfinal=y*zoom_level;
                    yfinal +=margeny_arr;
            }

            else {
                    //xfinal=((x*8)+bit)*zoom_level;
                    xfinal=((x*8)+bit)*zoom_level;
                    yfinal=y*zoom_level;
            }


            //Hacer zoom de ese pixel si conviene


            if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(xfinal,yfinal,color);

            else scr_putpixel_zoom(xfinal,yfinal,color);							



        }
    }
}								



//Muestra un caracter en footer
//Se utiliza solo al dibujar en zx81/81 y ace, y spectrum (simulado zx81) pero no en menu
//entrada: puntero=direccion a tabla del caracter
//x,y: coordenadas en x-0..31 e y 0..23 del zx81
//inverse si o no
//ink, paper
//si emula fast mode o no
//y valor de zoom
void old_scr_putchar_footer_comun_zoom(z80_byte caracter,int x,int y,z80_bit inverse,int tinta,int papel)
{

        int color;
        z80_byte bit;
        z80_byte line;
        z80_byte byte_leido;

        //printf ("tinta %d papel %d\n",tinta,papel);

        //margenes de zona interior de pantalla. Para modo rainbow
        int margenx_izq;
        int margeny_arr;


	z80_byte *puntero;
	puntero=&char_set[(caracter-32)*8];

	scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

	//Caso de pentagon y en footer
	//if (pentagon_timing.v && y>=31) margeny_arr=56*border_enabled.v;
	
        y=y*8;

        for (line=0;line<8;line++,y++) {
          byte_leido=*puntero++;
          if (inverse.v==1) byte_leido = byte_leido ^255;
          for (bit=0;bit<8;bit++) {
                if (byte_leido & 128 ) color=tinta;
                else color=papel;

                //simular modo fast para zx81
	

                byte_leido=(byte_leido&127)<<1;

		//este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)

		int xfinal,yfinal;

		xfinal=(((x*8)+bit));
		yfinal=y;

		//if (rainbow_enabled.v==1) {
			xfinal +=margenx_izq;

			yfinal +=margeny_arr;
		//}


		//Hacer zoom de ese pixel si conviene		
		scr_putpixel_gui_zoom(xfinal,yfinal,color,1);

		//footer va en capa de machine

		//printf ("%d %d\n",xfinal,yfinal);
		//scr_putpixel_zoom(xfinal,yfinal,color);


           }
        }
}



//Muestra un caracter en pantalla, al estilo del spectrum o zx80/81 o jupiter ace
//Se utiliza solo al dibujar en zx81/81 y ace, y spectrum (simulado zx81) pero no en menu
//entrada: puntero=direccion a tabla del caracter
//x,y: coordenadas en x-0..31 e y 0..23 del zx81
//inverse si o no
//ink, paper
//si emula fast mode o no
//y valor de zoom
void scr_putsprite_comun_zoom(z80_byte *puntero,int x,int y,z80_bit inverse,int tinta,int papel,z80_bit fast_mode,int zoom_level)
{

        int color;
        z80_byte bit;
        z80_byte line;
        z80_byte byte_leido;

        //printf ("tinta %d papel %d\n",tinta,papel);

        //margenes de zona interior de pantalla. Para modo rainbow
        int margenx_izq;
        int margeny_arr;



	scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

	//Caso de pentagon y en footer
	//if (pentagon_timing.v && y>=31) margeny_arr=56*border_enabled.v;
	
        y=y*8;

        for (line=0;line<8;line++,y++) {
          byte_leido=*puntero++;
          if (inverse.v==1) byte_leido = byte_leido ^255;
          for (bit=0;bit<8;bit++) {
                if (byte_leido & 128 ) color=tinta;
                else color=papel;

                //simular modo fast para zx81
		if (MACHINE_IS_ZX8081) {
	                if (fast_mode.v==1 && video_fast_mode_emulation.v==1 && video_fast_mode_next_frame_black==LIMIT_FAST_FRAME_BLACK) color=0;
		}

                byte_leido=(byte_leido&127)<<1;

		//este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)

		int xfinal,yfinal;

		xfinal=(((x*8)+bit)*zoom_level);
		yfinal=y*zoom_level;

		if (rainbow_enabled.v==1) {
			xfinal +=margenx_izq;

			yfinal +=margeny_arr;
		}




		//Hacer zoom de ese pixel si conviene		
		//scr_putpixel_gui_zoom(xfinal,yfinal,color,zoom_level);


                                if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(xfinal,yfinal,color);

                                else scr_putpixel_zoom(xfinal,yfinal,color);		


	



           }
        }
}


//putsprite pero sin zoom
void scr_putsprite_comun(z80_byte *puntero,int x,int y,z80_bit inverse,int tinta,int papel,z80_bit fast_mode)
{
	scr_putsprite_comun_zoom(puntero,x,y,inverse,tinta,papel,fast_mode,1);
}

//Muestra un caracter en pantalla, al estilo del zx80/81
//entrada: puntero=direccion a tabla del caracter
//x,y: coordenadas en x-0..31 e y 0..23 del zx81
//inverse si o no
void scr_putsprite(z80_byte *puntero,int x,int y,z80_bit inverse)
{

        z80_bit f;
        f.v=0;

	scr_putsprite_comun(puntero,x,y,inverse,0,15,f);
}




//Muestra un caracter de zx80/zx81 en pantalla
//entrada: direccion=tabla del caracter en direccion de memoria_spectrum
//x,y: coordenadas en x-0..31 e y 0..23 del zx81
//inverse si o no
void scr_putsprite_zx8081(z80_int direccion,int x,int y,z80_bit inverse)
{
	z80_bit f;

	f.v=1;

        scr_putsprite_comun(&memoria_spectrum[direccion],x,y,inverse,0,15,f);
        return;


}

//Devuelve bit pixel, en coordenadas 0..255,0..191. En pantalla rainbow para zx8081
int scr_get_pixel_rainbow(int x,int y)
{

	z80_byte byte_leido;

	z80_int *puntero_buf_rainbow;

        puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow()+x ];

	byte_leido=(*puntero_buf_rainbow)&15;
	if (byte_leido==0) return 1;
	else return 0;

}

//Devuelve pixel a 1 o 0, en coordenadas 0..255,0..191. En pantalla de spectrum con puntero de entrada
int scr_get_pixel_adr(int x,int y,z80_byte *screen)
{

	z80_int direccion;
	z80_byte byte_leido;
	z80_byte bit;
	z80_byte mascara;

       direccion=screen_addr_table[(y<<5)]+x/8;
       byte_leido=screen[direccion];


	bit=x%8;
	mascara=128;
	if (bit) mascara=mascara>>bit;
	if ((byte_leido & mascara)==0) return 0;
	else return 1;

}

//Devuelve pixel a 1 o 0, en coordenadas 0..255,0..191. En pantalla de spectrum
int scr_get_pixel(int x,int y)
{

	z80_int direccion;
	z80_byte byte_leido;
	z80_byte bit;
	z80_byte mascara;

       z80_byte *screen=get_base_mem_pantalla();
       direccion=screen_addr_table[(y<<5)]+x/8;
       byte_leido=screen[direccion];


	bit=x%8;
	mascara=128;
	if (bit) mascara=mascara>>bit;
	if ((byte_leido & mascara)==0) return 0;
	else return 1;

}


//Devuelve suma de pixeles a 1 en un cuadrado de 4x4, en coordenadas 0..255,0..191. En pantalla de spectrum
int scr_get_4pixel(int x,int y)
{

	int result=0;
	int dx,dy;

        for (dx=0;dx<4;dx++) {
                for (dy=0;dy<4;dy++) {
			result +=scr_get_pixel(x+dx,y+dy);
		}
	}

	return result;

}

//Devuelve suma de pixeles a 1 en un cuadrado de 4x4, en coordenadas 0..255,0..191. En pantalla de spectrum con puntero de entrada
int scr_get_4pixel_adr(int x,int y,z80_byte *screen)
{

	int result=0;
	int dx,dy;

        for (dx=0;dx<4;dx++) {
                for (dy=0;dy<4;dy++) {
			result +=scr_get_pixel_adr(x+dx,y+dy,screen);
		}
	}

	return result;

}


//Devuelve suma de pixeles de colores en un cuadrado de 4x4, en coordenadas 0..255,0..191. En rainbow para zx8081
int scr_get_4pixel_rainbow(int x,int y)
{

	int result=0;
	int dx,dy;

        for (dx=0;dx<4;dx++) {
                for (dy=0;dy<4;dy++) {
                        result +=scr_get_pixel_rainbow(x+dx,y+dy);
                }
        }
        return result;


}


void scr_simular_video_zx8081_put4pixel(int x,int y,int color)
{

	int dx,dy;
	//int zx,zy;

	for (dx=0;dx<4;dx++) {
		for (dy=0;dy<4;dy++) {
				scr_putpixel_zoom(x+dx,y+dy,color);


		}
	}


}

int calcula_offset_screen (int x,int y)
{

        unsigned char high,low;

        low=x+ ((y & 7 )<< 5);
        high= y  & 24;



        return low+high*256;



}




//Simular pantalla del zx80/81 en spectrum
//Se busca para cada bloque de 8x8 coincidencias con tablas de caracter
//sino, se divide en 4 bloques de 4x4 y para cada uno, si los pixeles a 1 es mayor o igual que el umbral, se pone pixel(color 0). Si no, se quita (color 15)
void scr_simular_video_zx8081(void)
{

int x,y;
z80_byte caracter;
z80_byte *screen;


screen=get_base_mem_pantalla();
unsigned char inv;
z80_bit inversebit;

for (y=0;y<192;y+=8) {
	for (x=0;x<256;x+=8) {

                //Ver en casos en que puede que haya menu activo y hay que hacer overlay
               if (scr_ver_si_refrescar_por_menu_activo(x/8,y/8)) {

			caracter=compare_char(&screen[  calcula_offset_screen(x/8,y/8)  ] , &inv);

			if (caracter) {
				if (inv) inversebit.v=1;
				else inversebit.v=0;
				//printf ("caracter: %d\n",caracter);

				//En ZX81 solo existen mayusculas
				caracter=letra_mayuscula(caracter);

				scr_putsprite(&char_set[(caracter-32)*8],x/8,y/8,inversebit);

			}


			else {
				//Pixel izquierda arriba
				if (scr_get_4pixel(x,y)>=umbral_simulate_screen_zx8081) scr_simular_video_zx8081_put4pixel(x,y,0);
				else scr_simular_video_zx8081_put4pixel(x,y,15);

				//Pixel derecha arriba
				if (scr_get_4pixel(x+4,y)>=umbral_simulate_screen_zx8081) scr_simular_video_zx8081_put4pixel(x+4,y,0);
				else scr_simular_video_zx8081_put4pixel(x+4,y,15);

				//Pixel derecha abajo
				if (scr_get_4pixel(x+4,y+4)>=umbral_simulate_screen_zx8081) scr_simular_video_zx8081_put4pixel(x+4,y+4,0);
				else scr_simular_video_zx8081_put4pixel(x+4,y+4,15);

				//Pixel izquierda abajo
				if (scr_get_4pixel(x,y+4)>=umbral_simulate_screen_zx8081) scr_simular_video_zx8081_put4pixel(x,y+4,0);
				else scr_simular_video_zx8081_put4pixel(x,y+4,15);
			}
		}

	}
}

}


//Retorna 0 si no hay que refrescar esa zona
//Pese a que en cada driver de video, cuando refresca pantalla, luego llama a overlay menu
//Pero en xwindows, se suele producir un refresco por parte del servidor X que provoc
//parpadeo entre la pantalla de spectrum y el menu
//por tanto, es preferible que si esa zona de pantalla de spectrum esta ocupada por algun texto del menu, no repintar para no borrar texto del menu
//Esto incluye tambien el texto de splash del inicio
//No incluiria cualquier otra funcion de overlay diferente del menu o el splash
int scr_ver_si_refrescar_por_menu_activo(int x GCC_UNUSED,int fila GCC_UNUSED)
{


	//Esta funcion ya no tiene sentido. Escribir siempre 
	return 1;

    /*
	x /=menu_gui_zoom;
	fila /=menu_gui_zoom;


	if (x>31 || fila>23) return 1;



	//Ver en casos en que puede que haya menu activo y hay que hacer overlay
  if (screen_refresh_menu==1) {
		if (menu_overlay_activo==1) {
                                        //hay menu activo. no refrescar esa coordenada si hay texto del menu
			int pos=fila*32+x;

			if (overlay_usado_screen_array[pos]) {
                                        //if (overlay_screen_array[pos].caracter!=0) {
                                                //no hay que repintar en esa zona
				return 0;
			}

			

		}
	}
	return 1;
    */

}

//putpixel escalandolo a zoom 1 -> no zoom
//por tanto, (0,0) = dentro de pantalla
void scr_putpixel_zoom_timex_mode6(int x,int y,unsigned int color)
{

#ifdef PUTPIXELCACHE
/*
        int indice_cache;

	//printf ("--%d\n",get_total_ancho_rainbow() );

        //indice_cache=(get_total_ancho_rainbow()*(screen_borde_superior*border_enabled.v+y)) + screen_total_borde_izquierdo*border_enabled.v+x;

	// multiplicar por 2 dado que es 512 de ancho
        //indice_cache=(get_total_ancho_rainbow()*2*(screen_borde_superior*border_enabled.v+y)) + screen_total_borde_izquierdo*border_enabled.v+x;

#define ANCHO_TIMEX 512
	indice_cache=ANCHO_TIMEX*y+x;

        if (putpixel_cache[indice_cache]==color) return;

        putpixel_cache[indice_cache]=color;
*/
#endif

                int offsetx,offsety;

	//Aqui se llama ya haciendo 512x192. En caso de zoom 4, pues tenemos que dividir entre dos


        offsetx=LEFT_BORDER*border_enabled.v;
        offsety=TOP_BORDER*border_enabled.v;


	int zx,zy;
        int xzoom=x*zoom_x/2;
        int yzoom=y*zoom_y;

        //Escalado a zoom indicado
        for (zx=0;zx<zoom_x;zx++) {
                for (zy=0;zy<zoom_y;zy++) {
                        scr_putpixel(offsetx+xzoom+zx,offsety+yzoom+zy,color);
                }
        }


}


void scr_putpixel_zoom_timex_mode6_interlaced(int x,int y,unsigned int color)
{

//int zyinicial=( (interlaced_numero_frame & 1)==1 ? 1 : 0);

        //interlaced mode, linea impar mas oscura
        //if (zyinicial && video_interlaced_scanlines.v) color +=16;

#ifdef PUTPIXELCACHE
/*
        int indice_cache;

        //printf ("--%d\n",get_total_ancho_rainbow() );

        //indice_cache=(get_total_ancho_rainbow()*(screen_borde_superior*border_enabled.v+y)) + screen_total_borde_izquierdo*border_enabled.v+x;

        // multiplicar por 2 dado que es 512 de ancho
        //indice_cache=(get_total_ancho_rainbow()*2*(screen_borde_superior*border_enabled.v+y)) + screen_total_borde_izquierdo*border_enabled.v+x;

#define ANCHO_TIMEX 512
        indice_cache=ANCHO_TIMEX*y+x;

        if (putpixel_cache[indice_cache]==color) return;

        putpixel_cache[indice_cache]=color;
*/
#endif

                int offsetx,offsety;

        //Aqui se llama ya haciendo 512x192. En caso de zoom 4, pues tenemos que dividir entre dos


        offsetx=LEFT_BORDER*border_enabled.v;
        offsety=TOP_BORDER*border_enabled.v;

	int zyinicial=( (interlaced_numero_frame & 1)==1 ? zoom_y/2 : 0);
	int zyfinal=  ( (interlaced_numero_frame & 1)==1 ? zoom_y   : zoom_y/2);

	//Color scanlines
	if (video_interlaced_scanlines.v && zyinicial) color +=16;


        int zx,zy;
        int xzoom=x*zoom_x/2;
        int yzoom=y*zoom_y;

        //Escalado a zoom indicado
        for (zx=0;zx<zoom_x;zx++) {
                for (zy=zyinicial;zy<zyfinal;zy++) {
                        scr_putpixel(offsetx+xzoom+zx,offsety+yzoom+zy,color);
                }
        }


}


void scr_refresca_pantalla_timex_512x192(void)
{
        int x,y,bit;
        z80_int direccion;
        z80_byte byte_leido;
        int fila;
        //int zx,zy;

        int col6;
        int tin6, pap6;





       z80_byte *screen=get_base_mem_pantalla();

        //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
        int x_hi;


				tin6=get_timex_ink_mode6_color();


                //Obtenemos color
                pap6=get_timex_paper_mode6_color();

				//printf ("antes tin6: %d pap6: %d\n",tin6,pap6);


				//Poner brillo1
				tin6 +=8;
				pap6 +=8;

				if (ulaplus_presente.v && ulaplus_enabled.v) {
					//Colores en ulaplus en este modo son:
					/*
BITS INK PAPER BORDER
000  24 31 31
001  25 30 30
010  26 29 29
011  27 28 28
100  28 27 27
101  29 26 26
110  30 25 25
111  31 24 24
					*/

					tin6 +=16;
					pap6 +=16;

					//printf ("tin6: %d pap 6: %d\n",tin6,pap6);

					tin6=ulaplus_palette_table[tin6]+ULAPLUS_INDEX_FIRST_COLOR;
					pap6=ulaplus_palette_table[pap6]+ULAPLUS_INDEX_FIRST_COLOR;
					//printf ("P tin6: %d pap 6: %d\n",tin6,pap6);

				}

				//Si tbblue
				if (MACHINE_IS_TBBLUE) {
					z80_byte attribute_temp=(pap6&7)*8  + (tin6&7) + 64;
					z80_int tinta_temp=tin6;
					z80_int papel_temp=pap6;
					get_pixel_color_tbblue(attribute_temp,&tinta_temp,&papel_temp);

					tin6=tinta_temp;
					pap6=papel_temp;
					tin6=RGB9_INDEX_FIRST_COLOR+tbblue_get_palette_active_ula(tin6);
					pap6=RGB9_INDEX_FIRST_COLOR+tbblue_get_palette_active_ula(pap6);
					//printf ("attr: %d tin6: %d pap6: %d\n",attribute_temp,tin6,pap6);
				}

		z80_int incremento_offset=0;


	//Refrescar border si conviene
	if (border_enabled.v) {
                        //ver si hay que refrescar border
                        if (modificado_border.v)
                        {
                                //printf ("refrescamos border\n");
                                scr_refresca_border_comun_spectrumzx8081(pap6);
                                modificado_border.v=0;
                        }

        }



        for (y=0;y<192;y++) {
                direccion=screen_addr_table[(y<<5)];


                fila=y/8;
                for (x=0,x_hi=0;x<64;x++,x_hi +=8) {


                        //Ver en casos en que puede que haya menu activo y hay que hacer overlay
			//if (1==1) {
                        if (scr_ver_si_refrescar_por_menu_activo(x/2,fila)) {

                                byte_leido=screen[direccion+incremento_offset];


                                for (bit=0;bit<8;bit++) {
					if (byte_leido&128) col6=tin6;
					else col6=pap6;


					//printf ("color: %d\n",col6);

                                        //scr_putpixel(offsetx+x_hi+bit,offsety+y,col6);
					//printf ("x: %d y: %d\n",x_hi+bit,y*2);


					if (video_interlaced_mode.v==0) {
						scr_putpixel_zoom_timex_mode6(x_hi+bit,y,col6);
					}

					else {
						scr_putpixel_zoom_timex_mode6_interlaced(x_hi+bit,y,col6);
					}

                                        byte_leido=byte_leido<<1;
                                }
                        }

			incremento_offset ^=8192;


                        if (incremento_offset==0) direccion++;
			//printf ("direccion:%d\n",direccion);
                }

        }

}

//Mezclar dos colores si estan en rango spectrum 0-15, retornando el gigascreen. Si no, devolver el primero
z80_int screen_scale_075_mix_two(z80_int color1, z80_int color2)
{
	if (color1<16 && color2<16 && screen_reduce_075_antialias.v) return get_gigascreen_color(color1,color2);
	else return color1;
}	

void screen_scale_rainbow_43(z80_int *orig,int ancho,int alto,z80_int *dest)
{

	int x,y;

	int ancho_destino=(ancho*3)/4;
	int alto_destino=(alto*3)/4;

	int diferencia_ancho=ancho-ancho_destino;
	int diferencia_alto=alto-alto_destino;

	//Controlar offsets
	if (screen_reduce_offset_x>diferencia_ancho) screen_reduce_offset_x=diferencia_ancho;
	if (screen_reduce_offset_y>diferencia_alto) screen_reduce_offset_y=diferencia_alto-1;

	if (screen_reduce_offset_x<0) screen_reduce_offset_x=0;
	if (screen_reduce_offset_y<0) screen_reduce_offset_y=0;	

	dest +=screen_reduce_offset_x;
	dest +=screen_reduce_offset_y*ancho;

	z80_int color_izq;
	z80_int color_der;
	z80_int color_arr;
	z80_int color_aba;
/*

La reducción funciona de la siguiente manera, se divide la imagen origen en bloques de 4x4 pixeles, y cada una de las de destino será de 3x3
Se parte de la imagen origen:

abcd
efgh
ijkl
mnop

A la de destino:

ab3
ef6
789

De la imagen origen, el primer bloque de 2x2 se traspasa tal cual, así:

ab
ef

Se traspasa a:

ab
ef

Luego, las primeras dos filas, se escalan asi:
Las ultimas dos columnas se mezclan los colores, mediante la funcion de mezclado: si hay antialias, se saca el color medio de los dos pixeles, el de la izquierda y derecha. 
Si no hay antialas, se escoge el primer pixel.
Así:

cd   -> 3
gh   -> 6


Luego las ultimas dos filas, se escalan así:

Las primeras 3 columnas se mezclan los colores, de manera similar a la anterior, pero mezclando el pixel de arriba y abajo.
Así:

i 
   ->  7
m   


j
   -> 8
n


k
   -> 9
o

De esto se ve que siempre se descarta dos pixeles como minimo, el l y p

*/

	for (y=0;y<alto;y++) {
		

		for (x=0;x<ancho;x+=4) {

			//Las dos primeras lineas, las dos primeras columnas, color tal cual. La tercera columna, se mezclan
			if ( (y%4)<2) {
			*dest=*orig;
			dest++;
			orig++;
			
			*dest=*orig;
			dest++;
			orig++;

			//Mezclar los ultimos dos
			color_izq=*orig;
			orig++;
			color_der=*orig;
			orig++;

			*dest=screen_scale_075_mix_two(color_izq,color_der);
			dest++;
			}

			//Las ultimas dos lineas, mezclamos arriba y abajo en las tres primeras columnas. La cuarta columna se descarta
			if ( (y%4)==2) {
			color_arr=*orig;
			color_aba=orig[ancho];

			*dest=screen_scale_075_mix_two(color_arr,color_aba);
			dest++;
			orig++;

			
			color_arr=*orig;
			color_aba=orig[ancho];

			*dest=screen_scale_075_mix_two(color_arr,color_aba);
			dest++;
			orig++;


			//Mezclar los ultimos dos
			/*color_izq=*orig;
			orig++;
			color_der=*orig;
			orig++;

			*dest=screen_scale_075_mix_two(color_izq,color_der);
			dest++;*/

			color_arr=*orig;
			color_aba=orig[ancho];

			*dest=screen_scale_075_mix_two(color_arr,color_aba);
			dest++;
			orig++;

			orig++;
			

			}

			
			
		}
		

		if ( (y%4)==2) {
			//Saltar la cuarta linea
			orig+=ancho;
			y++;
		}

		dest+=diferencia_ancho;
	}

}


//Meter pixel en un buffer rainbow de color indexado 16 bits. Usado en watermark y se podria usar en mas cosas
void screen_generic_putpixel_indexcolour(z80_int *destino,int x,int y,int ancho,int color)
{
	int offset=y*ancho+x;

	destino[offset]=color;
}

//Obtiene pixel de un buffer rainbow de color indexado 16 bits. Usado en watermark y se podria usar en mas cosas. Justo lo contrario de screen_generic_putpixel_indexcolour
int screen_generic_getpixel_indexcolour(z80_int *destino,int x,int y,int ancho)
{
        int offset=y*ancho+x;

        return destino[offset];
}

//Hacer putpixel en pantalla de color indexado 16 bits. Usado en watermark para no rainbow
void screen_generic_putpixel_no_rainbow_watermark(z80_int *destino GCC_UNUSED,int x,int y,int ancho GCC_UNUSED,int color)
{
	scr_putpixel(x,y,color);
}


//Mete un bitmap en formato ascii en un bitmap generico
void screen_put_asciibitmap_generic(char **origen,z80_int *destino,int x,int y,int ancho_orig, int alto_orig, int ancho_destino, void (*putpixel) (z80_int *destino,int x,int y,int ancho_destino,int color), int zoom,int inverso)
{
	int fila,columna;

	for (fila=0;fila<alto_orig;fila++) {
		//int offset_fila=fila*ancho_orig;
		char *texto;
		
		texto=origen[fila];
		for (columna=0;columna<ancho_orig;columna++) {
			char caracter=texto[columna];

			if (caracter!=' ') {
				int color_pixel=return_color_zesarux_ascii(caracter);

				if (inverso) {
					//Se supone que el color esta entre 0 y 15 pero por si acaso
					if (color_pixel>=0 && color_pixel<=15) {
						color_pixel=15-color_pixel;
					}
				}

				int zx,zy;
				for (zx=0;zx<zoom;zx++) {
					for (zy=0;zy<zoom;zy++) {
						putpixel(destino,x+columna*zoom+zx,y+fila*zoom+zy,ancho_destino,color_pixel);
					}
				}
			}

			

			//temp zoom 2
			/*
			printf ("fila %d columna %d\n",fila,columna);
			
			if (caracter!=' ') {
				putpixel(destino,x+columna*2,y+fila*2,ancho_destino,return_color_zesarux_ascii(caracter));
				putpixel(destino,x+columna*2+1,y+fila*2,ancho_destino,return_color_zesarux_ascii(caracter));
				putpixel(destino,x+columna*2,y+fila*2+1,ancho_destino,return_color_zesarux_ascii(caracter));
				putpixel(destino,x+columna*2+1,y+fila*2+1,ancho_destino,return_color_zesarux_ascii(caracter));
			}
			*/
			
			
		}
	}
}



void screen_put_watermark_generic(z80_int *destino,int x,int y,int ancho_destino, void (*putpixel) (z80_int *destino,int x,int y,int ancho,int color) )
{
		screen_put_asciibitmap_generic(zesarux_ascii_logo,destino,x,y,ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, ancho_destino,putpixel,1,0);
}

void screen_get_offsets_watermark_position(int position,int ancho, int alto, int *x, int *y)
{

	int watermark_x=*x;
	int watermark_y=*y;

	int rango_extremo=ZESARUX_WATERMARK_LOGO_MARGIN; //4;

		switch (position) {
			case 0:
				watermark_x=rango_extremo;
				watermark_y=rango_extremo;
			break;

			case 1:
				watermark_x=ancho-ZESARUX_ASCII_LOGO_ANCHO-rango_extremo;
				watermark_y=rango_extremo;
			break;

			case 2:
				watermark_x=rango_extremo;
				watermark_y=alto-ZESARUX_ASCII_LOGO_ALTO-rango_extremo;
			break;				

			case 3:
			default:
				watermark_x=ancho-ZESARUX_ASCII_LOGO_ANCHO-rango_extremo;
				watermark_y=alto-ZESARUX_ASCII_LOGO_ALTO-rango_extremo;
			break;			

		}

		*x=watermark_x;
		*y=watermark_y;
}

//Comunes a escalado normal y escalado con gigascreen
int scalled_rainbow_ancho=0;
int scalled_rainbow_alto=0;

//Punteros de escalado 0.75 para gigascreen
z80_int *new_scalled_rainbow_buffer_gigascren_one=NULL;
z80_int *new_scalled_rainbow_buffer_gigascren_two=NULL;

void screen_scale_075_and_watermark_function(z80_int *origen,z80_int *destino,int ancho,int alto)
{
			//int ancho_destino=ancho; // (ancho*3)/4;
		//int alto_destino=alto; //(alto*3)/4;

		//solo asignar buffer la primera vez o si ha cambiado el tamanyo
		//int asignar=0;
		
		//Si ha cambiado el tamanyo
		/*if (scalled_rainbow_ancho!=ancho || scalled_rainbow_alto!=alto) {
			//Liberar si existia
			if (scalled_rainbow_buffer!=NULL) {
				debug_printf(VERBOSE_DEBUG,"Freeing previous scaled rainbow buffer");
				free (scalled_rainbow_buffer);
				scalled_rainbow_buffer=NULL;


			}

			asignar=1;
		}

		//O si no hay buffer asignado
		if (scalled_rainbow_buffer==NULL) asignar=1;

		if (asignar) {
			debug_printf(VERBOSE_DEBUG,"Allocating scaled rainbow buffer");
			scalled_rainbow_buffer=malloc(ancho*alto*2); // *2 por que son valores de 16 bits
			if (scalled_rainbow_buffer==NULL) cpu_panic("Can not allocate scalled rainbow buffer");

			//Llenarlo de cero
			int i;
			for (i=0;i<ancho*alto;i++) scalled_rainbow_buffer[i]=0;

			scalled_rainbow_ancho=ancho;
			scalled_rainbow_alto=alto;
		}
		*/

		screen_scale_rainbow_43(origen,ancho,alto,destino);

		//Forzamos meter watermark

		int watermark_x;
		int watermark_y;

		//Misma variable que watermark general
		screen_get_offsets_watermark_position(screen_watermark_position,((ancho*3)/4),((alto*3)/4),&watermark_x,&watermark_y);

		watermark_x +=screen_reduce_offset_x;
		watermark_y +=screen_reduce_offset_y;

		
		screen_put_watermark_generic(destino,watermark_x,watermark_y,scalled_rainbow_ancho,screen_generic_putpixel_indexcolour);

		//Y decimos que el puntero de dibujado ahora lo pilla de la pantalla escalada
		//return scalled_rainbow_buffer;	


}




void screen_scale_075_gigascreen_function(int ancho,int alto)
{


                //solo asignar buffer la primera vez o si ha cambiado el tamanyo
                int asignar=0;
                
                //Si ha cambiado el tamanyo
                if (scalled_rainbow_ancho!=ancho || scalled_rainbow_alto!=alto) {
                        //Liberar si existia
                        if (new_scalled_rainbow_buffer_gigascren_one!=NULL) {
                                debug_printf(VERBOSE_DEBUG,"Freeing previous scaled gigascreen rainbow buffers");
                                free (new_scalled_rainbow_buffer_gigascren_one);
								free (new_scalled_rainbow_buffer_gigascren_two);
                                new_scalled_rainbow_buffer_gigascren_one=NULL;
								new_scalled_rainbow_buffer_gigascren_two=NULL;
                        }

                        asignar=1;
                }

                //O si no hay buffer asignado
                if (new_scalled_rainbow_buffer_gigascren_one==NULL) asignar=1;

				if (asignar) {
                        debug_printf(VERBOSE_DEBUG,"Allocating scaled gigascreen rainbow buffers");
                        new_scalled_rainbow_buffer_gigascren_one=malloc(ancho*alto*2); //*2 por que son valores de 16 bits
						new_scalled_rainbow_buffer_gigascren_two=malloc(ancho*alto*2); //*2 por que son valores de 16 bits

                        if (new_scalled_rainbow_buffer_gigascren_one==NULL || new_scalled_rainbow_buffer_gigascren_two==NULL) cpu_panic("Can not allocate scalled gigascreen rainbow buffers");

                        //Llenarlo de cero
                        int i;
                        for (i=0;i<ancho*alto;i++) {
							new_scalled_rainbow_buffer_gigascren_one[i]=0;
							new_scalled_rainbow_buffer_gigascren_two[i]=0;
						}

                        scalled_rainbow_ancho=ancho;
                        scalled_rainbow_alto=alto;
                }

				screen_scale_075_and_watermark_function(rainbow_buffer_one,new_scalled_rainbow_buffer_gigascren_one,ancho,alto);
				screen_scale_075_and_watermark_function(rainbow_buffer_two,new_scalled_rainbow_buffer_gigascren_two,ancho,alto);
}


void scr_refresca_pantalla_rainbow_comun_gigascreen(void)
{
	if ((interlaced_numero_frame&1)==0) {




		//printf ("refresco con gigascreen\n");

        //aqui no tiene sentido (o si?) el modo simular video zx80/81 en spectrum
        int ancho,alto;

        ancho=get_total_ancho_rainbow();
        alto=get_total_alto_rainbow();

        int x,y,bit;

        //margenes de zona interior de pantalla. Para overlay menu
        int margenx_izq=screen_total_borde_izquierdo*border_enabled.v;
        int margenx_der=screen_total_borde_izquierdo*border_enabled.v+256;
        int margeny_arr=screen_borde_superior*border_enabled.v;
        int margeny_aba=screen_borde_superior*border_enabled.v+192;
        //para overlay menu tambien
        //int fila;
        //int columna;

	//Para gigascreen, valores que se encontraran en el buffer rainbow seran entre 0 y 15
        z80_byte color_pixel_one,color_pixel_two;

	int color_pixel_final;
        z80_int *puntero_one,*puntero_two;

        puntero_one=rainbow_buffer_one;
        puntero_two=rainbow_buffer_two;


		//Reducimos los dos bufferes si conviene-escalado 0.75
        if (screen_reduce_075.v) {
                screen_scale_075_gigascreen_function(ancho,alto);
				puntero_one=new_scalled_rainbow_buffer_gigascren_one;
				puntero_two=new_scalled_rainbow_buffer_gigascren_two;
        }



        int dibujar;

        for (y=0;y<alto;y++) {
                for (x=0;x<ancho;x+=8) {
                        dibujar=1;

                        //Ver si esa zona esta ocupada por texto de menu u overlay

                        if (y>=margeny_arr && y<margeny_aba && x>=margenx_izq && x<margenx_der) {


                                //normalmente a 48
                                //int screen_total_borde_izquierdo;

                                if (!scr_ver_si_refrescar_por_menu_activo( (x-margenx_izq)/8, (y-margeny_arr)/8) )
                                        dibujar=0;

                        }
                        if (dibujar==1) {

                                        for (bit=0;bit<8;bit++) {


                                                //printf ("x: %d y: %d\n",x,y);

                                                color_pixel_one=*puntero_one++;
						color_pixel_two=*puntero_two++;


						color_pixel_final=get_gigascreen_color(color_pixel_one,color_pixel_two);


                                                scr_putpixel_zoom_rainbow(x+bit,y,color_pixel_final);
                                        }
                        }
                        else {
				puntero_one+=8;
				puntero_two+=8;
			}

                }
        }




	}

	screen_switch_rainbow_buffer();
}


void scr_refresca_pantalla_rainbow_unalinea_timex(int y)
{

	//printf ("timex modo 512x192 linea y: %d\n",y);
	//return;

	        int x,bit;
        z80_int direccion;
        z80_byte byte_leido;
        int fila;
        //int zx,zy;

        int col6;
        int tin6, pap6;


       z80_byte *screen=get_base_mem_pantalla();

        //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
        int x_hi;


                                tin6=get_timex_ink_mode6_color();


                                //Obtenemos color
                                pap6=get_timex_paper_mode6_color();


                                //Poner brillo1
                                tin6 +=8;
                                pap6 +=8;

                                if (ulaplus_presente.v && ulaplus_enabled.v) {
                                        //Colores en ulaplus en este modo son:
                                        /*
BITS INK PAPER BORDER
000 24 31 31
001 25 30 30
010 26 29 29
011 27 28 28
100 28 27 27
101 29 26 26
110 30 25 25
111 31 24 24
                                        */

                                        tin6 +=16;
                                        pap6 +=16;


					tin6=ulaplus_palette_table[tin6]+ULAPLUS_INDEX_FIRST_COLOR;
                                        pap6=ulaplus_palette_table[pap6]+ULAPLUS_INDEX_FIRST_COLOR;
                                }

                z80_int incremento_offset=0;


        
                direccion=screen_addr_table[(y<<5)];


                fila=y/8;
                for (x=0,x_hi=0;x<64;x++,x_hi +=8) {


                        //Ver en casos en que puede que haya menu activo y hay que hacer overlay
                        //if (1==1) {
                        if (scr_ver_si_refrescar_por_menu_activo(x/2,fila)) {

                                byte_leido=screen[direccion+incremento_offset];


                                for (bit=0;bit<8;bit++) {
                                        if (byte_leido&128) col6=tin6;
                                        else col6=pap6;


                                        //printf ("color: %d\n",col6);

                                        //scr_putpixel(offsetx+x_hi+bit,offsety+y,col6);
                                        //printf ("x: %d y: %d\n",x_hi+bit,y*2);


                                        if (video_interlaced_mode.v==0) {
                                                scr_putpixel_zoom_timex_mode6(x_hi+bit,y,col6);
                                        }

                                        else {


scr_putpixel_zoom_timex_mode6_interlaced(x_hi+bit,y,col6);
                                        }

                                        byte_leido=byte_leido<<1;
                                }
                        }

                        incremento_offset ^=8192;


                        if (incremento_offset==0) direccion++;
                        //printf ("direccion:%d\n",direccion);
                }

       

}


z80_int *new_scalled_rainbow_buffer=NULL;





void screen_scale_075_function(int ancho,int alto)
{


		//solo asignar buffer la primera vez o si ha cambiado el tamanyo
		int asignar=0;
		
		//Si ha cambiado el tamanyo
		if (scalled_rainbow_ancho!=ancho || scalled_rainbow_alto!=alto) {
			//Liberar si existia
			if (new_scalled_rainbow_buffer!=NULL) {
				debug_printf(VERBOSE_DEBUG,"Freeing previous scaled rainbow buffer");
				free (new_scalled_rainbow_buffer);
				new_scalled_rainbow_buffer=NULL;
			}

			asignar=1;
		}

		//O si no hay buffer asignado
		if (new_scalled_rainbow_buffer==NULL) asignar=1;

		if (asignar) {
			debug_printf(VERBOSE_DEBUG,"Allocating scaled rainbow buffer");
			new_scalled_rainbow_buffer=malloc(ancho*alto*2); //*2 por que son valores de 16 bits
			if (new_scalled_rainbow_buffer==NULL) cpu_panic("Can not allocate scalled rainbow buffer");

			//Llenarlo de cero
			int i;
			for (i=0;i<ancho*alto;i++) new_scalled_rainbow_buffer[i]=0;

			scalled_rainbow_ancho=ancho;
			scalled_rainbow_alto=alto;
		}
		

		//Destino va a ser el mismo
		//screen_scale_075_and_watermark_function(rainbow_buffer,rainbow_buffer,ancho,alto);
		screen_scale_075_and_watermark_function(rainbow_buffer,new_scalled_rainbow_buffer,ancho,alto);

}


void screen_add_watermark_rainbow(void)
{
	//Si esta opcion de watermark general pero no esta el reduce de 0.75 (porque este reduce fuerza siempre watermark)
	if (screen_watermark_enabled.v && screen_reduce_075.v==0) {
		int watermark_x;
		int watermark_y;

		int ancho,alto;

		ancho=get_total_ancho_rainbow();
		alto=get_total_alto_rainbow();

		//Misma variable que watermark general
		screen_get_offsets_watermark_position(screen_watermark_position,ancho,alto,&watermark_x,&watermark_y);

		screen_put_watermark_generic(rainbow_buffer,watermark_x,watermark_y,ancho,screen_generic_putpixel_indexcolour);

	}
	
}

void screen_add_watermark_no_rainbow(void)
{
        //Si esta opcion de watermark general pero no esta el reduce de 0.75 (porque este reduce fuerza siempre watermark)
        if (screen_watermark_enabled.v && screen_reduce_075.v==0) {
                int watermark_x;
                int watermark_y;

                int ancho,alto;

                ancho=screen_get_emulated_display_width_zoom_border_en();
                alto=screen_get_emulated_display_height_zoom_border_en();

                //Misma variable que watermark general
                screen_get_offsets_watermark_position(screen_watermark_position,ancho,alto,&watermark_x,&watermark_y);

                screen_put_watermark_generic(rainbow_buffer,watermark_x,watermark_y,ancho,screen_generic_putpixel_no_rainbow_watermark);
                //screen_put_watermark_generic(rainbow_buffer,watermark_x,watermark_y,ancho,screen_generic_putpixel_indexcolour);

        }

}

//Refresco pantalla con rainbow
void scr_refresca_pantalla_rainbow_comun(void)
{

	if (gigascreen_enabled.v) {
		scr_refresca_pantalla_rainbow_comun_gigascreen();
		return;
	}

	//Si es modo timex 512x192, llamar a otra funcion
	if (timex_si_modo_512_y_zoom_par() ) {
		//Si zoom x par
					if (timex_mode_512192_real.v) {
						scr_refresca_pantalla_timex_512x192();
						return;
					}
	}


	//aqui no tiene sentido (o si?) el modo simular video zx80/81 en spectrum
	int ancho,alto;

	ancho=get_total_ancho_rainbow();
	alto=get_total_alto_rainbow();

	int x,y,bit;

	//margenes de zona interior de pantalla. Para overlay menu
	int margenx_izq=screen_total_borde_izquierdo*border_enabled.v;
	int margenx_der=screen_total_borde_izquierdo*border_enabled.v+256;
	int margeny_arr=screen_borde_superior*border_enabled.v;
	int margeny_aba=screen_borde_superior*border_enabled.v+192;

	if (MACHINE_IS_Z88) {
		margenx_izq=0;
		margenx_der=256;
		margeny_arr=0;
		margeny_aba=192;
	}

	//para overlay menu tambien
	//int fila;
	//int columna;

	z80_int color_pixel;
	z80_int *puntero;

	puntero=rainbow_buffer;
	int dibujar;


	//Si se reduce la pantalla 0.75
	if (screen_reduce_075.v) {
		screen_scale_075_function(ancho,alto);
		puntero=new_scalled_rainbow_buffer;
	}
	//Fin reduccion pantalla 0.75





	for (y=0;y<alto;y++) {
		//Truco para tener a partir de una posicion y modo timex 512x192

		int altoborder=screen_borde_superior;
		int linea_cambio_timex=timex_ugly_hack_last_hires-screen_invisible_borde_superior;
		

		if (timex_mode_512192_real.v==0 && timex_video_emulation.v && timex_ugly_hack_enabled && timex_ugly_hack_last_hires>0 && 
			y>=linea_cambio_timex && y<192+altoborder && ((zoom_x&1)==0) ) {
			
			scr_refresca_pantalla_rainbow_unalinea_timex(y-altoborder);
			puntero +=ancho;
		}
		else {
		for (x=0;x<ancho;x+=8) {
			dibujar=1;

			//Ver si esa zona esta ocupada por texto de menu u overlay

			if (y>=margeny_arr && y<margeny_aba && x>=margenx_izq && x<margenx_der) {
				if (!scr_ver_si_refrescar_por_menu_activo( (x-margenx_izq)/8, (y-margeny_arr)/8) )
					dibujar=0;
			}


			if (dibujar==1) {
					for (bit=0;bit<8;bit++) {
						color_pixel=*puntero++;
						scr_putpixel_zoom_rainbow(x+bit,y,color_pixel);
					}
			}
			else puntero+=8;

		}
		}
	}

	//timex_ugly_hack_last_hires=0;



}



//Refresco pantalla sin rainbow
void scr_refresca_pantalla_comun(void)
{
	int x,y,bit;
        z80_int direccion,dir_atributo;
        z80_byte byte_leido;
        int color=0;
        int fila;
        //int zx,zy;

        z80_byte attribute,ink,paper,bright,flash,aux;


	if (simulate_screen_zx8081.v==1) {
		//simular modo video zx80/81
		scr_simular_video_zx8081();
		return;
	}


       z80_byte *screen=get_base_mem_pantalla();

        //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
	z80_byte x_hi;

        for (y=0;y<192;y++) {
                //direccion=16384 | devuelve_direccion_pantalla(0,y);

                //direccion=16384 | screen_addr_table[(y<<5)];
                direccion=screen_addr_table[(y<<5)];


                fila=y/8;
                dir_atributo=6144+(fila*32);
                for (x=0,x_hi=0;x<32;x++,x_hi +=8) {


			//Ver en casos en que puede que haya menu activo y hay que hacer overlay
			if (scr_ver_si_refrescar_por_menu_activo(x,fila)) {

                	        byte_leido=screen[direccion];
	                        attribute=screen[dir_atributo];

				//Prueba de un modo de video inventado en que el color de la tinta sale de los 4 bits de la zona de pixeles
				//int ink1,ink2;

				if (scr_refresca_sin_colores.v) {
					attribute=56;
					//ink1=(byte_leido >>4)&0xF;
					//ink2=(byte_leido    )&0xF;					
				}


        	                ink=attribute &7;
                	        paper=(attribute>>3) &7;
	                        bright=(attribute) &64;
        	                flash=(attribute)&128;
                	        if (flash) {
                        	        //intercambiar si conviene
	                                if (estado_parpadeo.v) {
        	                                aux=paper;
                	                        paper=ink;
	                                        ink=aux;
        	                        }
                	        }

				if (bright) {
					ink +=8;
					paper +=8;
				}

                        	for (bit=0;bit<8;bit++) {

					color= ( byte_leido & 128 ? ink : paper );
					//if (scr_refresca_sin_colores.v) {
					//	if (bit<=3) color= ( byte_leido & 128 ? ink1 : paper );
					//	else color= ( byte_leido & 128 ? ink2 : paper );
					//}
					scr_putpixel_zoom(x_hi+bit,y,color);

	                                byte_leido=byte_leido<<1;
        	                }
			}

			//temp
			//else {
			//	printf ("no refrescamos zona x %d fila %d\n",x,fila);
			//}


                        direccion++;
			dir_atributo++;
                }

        }

}









void scr_mk14_linea(int x,int y,int longitud,int incx,int incy,int color)
{
	while (longitud) {
		if (scr_ver_si_refrescar_por_menu_activo(x/8,y/8)) {
			scr_putpixel_zoom(x,y,color);
		}
		x +=incx;
		y +=incy;

		longitud--;
	}
}

/*
Digitos. Bitmap:

   0123456789012345
00   xxxxxx
01   xxxxxx
02 xx      xx
03 xx      xx
04 xx      xx
05 xx      xx
06   xxxxxx
07   xxxxxx
08 xx      xx
09 xx      xx
10 xx      xx
11 xx      xx
12   xxxxxx  xx
13   xxxxxx  xx
14
15


Estado leds
bit

    0
    _
5  |  |  1
	  -
4	 |  |  2
    _
    3

6: centro
7: punto

*/

void scr_mk14_draw_led(z80_byte valor,int x,int y,int color)
{
	if (valor&1) {
		scr_mk14_linea(x+2,y,6,+1,0,color);
		scr_mk14_linea(x+2,y+1,6,+1,0,color);
	}

	if (valor&2) {
		scr_mk14_linea(x+8,y+2,4,0,+1,color);
		scr_mk14_linea(x+9,y+2,4,0,+1,color);
	}

	if (valor&4) {
		scr_mk14_linea(x+8,y+8,4,0,+1,color);
		scr_mk14_linea(x+9,y+8,4,0,+1,color);
	}

	if (valor&8) {
		scr_mk14_linea(x+2,y+12,6,+1,0,color);
		scr_mk14_linea(x+2,y+13,6,+1,0,color);
	}

	if (valor&16) {
		scr_mk14_linea(x,y+8,4,0,+1,color);
		scr_mk14_linea(x+1,y+8,4,0,+1,color);
	}

	if (valor&32) {
		scr_mk14_linea(x,y+2,4,0,+1,color);
		scr_mk14_linea(x+1,y+2,4,0,+1,color);
	}

	if (valor&64) {
		scr_mk14_linea(x+2,y+6,6,+1,0,color);
		scr_mk14_linea(x+2,y+7,6,+1,0,color);
	}


	if (valor&128) {
		scr_mk14_linea(x+10,y+12,2,+1,0,color);
		scr_mk14_linea(x+10,y+13,2,+1,0,color);
	}
}

//Refresco pantalla Para mk14. De momento solo poner la pantalla en blanco
void scr_refresca_pantalla_y_border_mk14(void)
{

	if (border_enabled.v) {
					//ver si hay que refrescar border
					if (modificado_border.v)
					{
//printf ("refrescamos border\n");
									scr_refresca_border_comun_spectrumzx8081(7);
									modificado_border.v=0;
//sleep (1);
					}

	}

	int x,y,bit;

        int color=0;
        int fila;


	z80_byte x_hi;

        for (y=0;y<192;y++) {

                fila=y/8;

                for (x=0,x_hi=0;x<32;x++,x_hi +=8) {


									//Ver en casos en que puede que haya menu activo y hay que hacer overlay
									if (scr_ver_si_refrescar_por_menu_activo(x,fila)) {

                	 for (bit=0;bit<8;bit++) {

											color=7;
											scr_putpixel_zoom(x_hi+bit,y,color);


        	         }
								 }


              }

      }


			//Dibujar digitos
			int i;
			x=0;
			y=0;
			for (i=MK14_DIGITS-1;i>=0;i--) {
				scr_mk14_draw_led(mk14_ledstat[i],x,y,2);

				x += 14;
			}

}


//Rutina usada por todos los drivers para escribir caracteres en pantalla en zx8081 y ace, en rutina de refresco que
//lee directament de DFILE
void scr_putchar_zx8081_comun(int x,int y, z80_byte caracter)
{


	z80_bit inverse;
	z80_int direccion;




	if (caracter>127) {
        	inverse.v=1;
	        caracter-=128;
	}

	else inverse.v=0;


	//Caso especial para jupiter ACE
	if (MACHINE_IS_ACE) {
		inverse.v ^=1;
		direccion=0x2c00;
		scr_putsprite_zx8081(direccion+caracter*8,x,y,inverse);
		return;
	}



	//con los caracteres fuera de rango, devolvemos '?'
	if (caracter>63) caracter=15;

        if (MACHINE_IS_ZX80) direccion=0x0E00;
        else direccion=0x1E00;

        scr_putsprite_zx8081(direccion+caracter*8,x,y,inverse);

}




void scr_refresca_pantalla_y_border_zx8081(void)
{


        //modo caracteres alta resolucion- rainbow - metodo nuevo
        if (rainbow_enabled.v==1) {
		scr_refresca_pantalla_rainbow_comun();
		return;
	}




        //simulacion pantalla negra fast
        if (hsync_generator_active.v==0) {

                if (video_fast_mode_next_frame_black!=LIMIT_FAST_FRAME_BLACK) {
			video_fast_mode_next_frame_black++;
		}

		if (video_fast_mode_next_frame_black==LIMIT_FAST_FRAME_BLACK) {
                        debug_printf(VERBOSE_DEBUG,"Detected fast mode");
			//forzar refresco de border
                        modificado_border.v=1;

                }
        }




                if (border_enabled.v) {
                        //ver si hay que refrescar border
                        if (modificado_border.v)
                        {
				//printf ("refrescamos border\n");
                                scr_refresca_border_zx8081();
                                modificado_border.v=0;
				//sleep (1);
                        }

                }

	//modo caracteres normal
        if (rainbow_enabled.v==0) scr_refresca_pantalla_zx8081();


}



void scr_refresca_pantalla_zx8081(void)
{
        int x,y;
        z80_byte caracter;

        //zx81
        z80_int video_pointer;

	//puntero pantalla en DFILE
        video_pointer=peek_word_no_time(0x400C);


	//Pruebas alterando video pointer para ver si funcionan los juegos flicker free
	//este medio funciona: Space\ Invaders\ 1K\ \(Macronics\ 1981\)\ Reconstruction.o  --zx8081mem 3

	while (video_pointer>ramtop_zx8081) {
		//debug_printf (VERBOSE_DEBUG,"invalid video_pointer: %d",video_pointer);
		video_pointer -=0x4000;
                //debug_printf (VERBOSE_DEBUG,"new video_pointer: %d",video_pointer);

	}



        //se supone que el primer byte es 118 . saltarlo
        video_pointer++;
        y=0;
        x=0;
        while (y<24) {

                caracter=memoria_spectrum[video_pointer++];
                if (caracter==118) {
                        //rellenar con espacios hasta final de linea
				//if (x<32) printf ("compressed line %d \n",y);
                                for (;x<32;x++) {
					if (scr_ver_si_refrescar_por_menu_activo(x,y)) scr_putchar_zx8081(x,y,0);
                                }
                                y++;

                                x=0;
                }

                else {
			if (scr_ver_si_refrescar_por_menu_activo(x,y)) scr_putchar_zx8081(x,y,caracter);

                        x++;

                        if (x==32) {
                                if (memoria_spectrum[video_pointer]!=118) {
									//debug_printf (VERBOSE_DEBUG,"End of line %d is not 118 opcode. Is: 0x%x",y,memoria_spectrum[video_pointer]);
								}
                                //saltamos el HALT que debe haber en el caso de linea con 32 caracteres
                                video_pointer++;
                                x=0;
                                y++;
                        }

                }


    }

}

void load_screen(char *scrfile)
{

	if (MACHINE_IS_SPECTRUM) {
		debug_printf (VERBOSE_INFO,"Loading Screen File");
		FILE *ptr_scrfile;
		ptr_scrfile=fopen(scrfile,"rb");
                if (!ptr_scrfile) {
			debug_printf (VERBOSE_ERR,"Unable to open Screen file");
		}

		else {

			/*if (MACHINE_IS_SPECTRUM_16_48) fread(memoria_spectrum+16384,1,6912,ptr_scrfile);

			else {

				//modo 128k. cargar en pagina 5 o 7
				int pagina=5;
				if ( (puerto_32765&8) ) pagina=7;

				fread(ram_mem_table[pagina],1,6912,ptr_scrfile);
			}
			*/
			z80_byte leido;
			int i;
			for (i=0;i<6912;i++) {
				fread(&leido,1,1,ptr_scrfile);
				poke_byte_no_time(16384+i,leido);
			}



			fclose(ptr_scrfile);

		}

	}

	else {
		debug_printf (VERBOSE_ERR,"Screen loading only allowed on Spectrum models");
	}

}

void save_screen_scr(char *scrfile)
{

                 if (MACHINE_IS_SPECTRUM) {
                                debug_printf (VERBOSE_INFO,"Saving Screen File");

FILE *ptr_scrfile;
                                  ptr_scrfile=fopen(scrfile,"wb");
                                  if (!ptr_scrfile)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open Screen file");
                                  }
                                else {



					/*
	                                   if (MACHINE_IS_SPECTRUM_16_48)
							fwrite(memoria_spectrum+16384, 1, 6912, ptr_scrfile);

					else {

        	                                //modo 128k. grabar pagina 5 o 7
                	                        int pagina=5;
                        	                if ( (puerto_32765&8) ) pagina=7;
						fwrite(ram_mem_table[pagina], 1, 6912, ptr_scrfile);
	                                  }
					*/

					z80_byte escrito;
	        	                int i;
        	        	        for (i=0;i<6912;i++) {
						escrito=peek_byte_no_time(16384+i);
						fwrite(&escrito,1,1,ptr_scrfile);
		                        }



	                               fclose(ptr_scrfile);

                                }
                        }

                        else {
                                debug_printf (VERBOSE_ERR,"Screen .scr saving only allowed on Spectrum models");
                        }


}

//Grabar pantalla segun si extension scr, pbm o bmp
void save_screen(char *screen_save_file)
{
	if (!util_compare_file_extension(screen_save_file,"scr")) {
		save_screen_scr(screen_save_file);
	}

	else if (!util_compare_file_extension(screen_save_file,"pbm")) {

		if (!MACHINE_IS_SPECTRUM) {
			debug_printf (VERBOSE_ERR,"Screen .pbm saving only allowed on Spectrum models");
			return;
        }

		//Asignar buffer temporal
		int longitud=6144;
		z80_byte *buf_temp=malloc(longitud);
		if (buf_temp==NULL) {
				debug_printf(VERBOSE_ERR,"Error allocating temporary buffer");
		}

		//Convertir pantalla a sprite ahi
		z80_byte *origen;
		origen=get_base_mem_pantalla();
		util_convert_scr_sprite(origen,buf_temp);

		util_write_pbm_file(screen_save_file,256,192,8,buf_temp);

		free(buf_temp);


	}

	else if (!util_compare_file_extension(screen_save_file,"bmp")) {

		util_write_screen_bmp(screen_save_file);

	}		

	else {
		debug_printf(VERBOSE_ERR,"Unsuported file type");
		return;
	} 

}


//Guardar en buffer rainbow una linea del caracter de zx8081. usado en modo de video real
void screen_store_scanline_char_zx8081(int x,int y,z80_byte byte_leido,z80_byte caracter,int inverse)
{
	int bit;
        z80_byte color;

	z80_byte colortinta=0;
	z80_byte colorpapel=15;

	//Si modo chroma81 y lo ha activado

	if (color_es_chroma() ) {
		//ver modo
		if ((chroma81_port_7FEF&16)!=0) {
			//1 attribute file
			chroma81_return_mode1_colour(reg_pc,&colortinta,&colorpapel);
			//printf ("modo 1\n");
		}
		else {
			//0 character code
			//tablas colores van primero para los 64 normales y luego para los 64 inversos
			if (inverse) {
				caracter +=64;
			}

			z80_int d=caracter*8+0xc000;
			d=d+(y&7);
			z80_byte leido=peek_byte_no_time(d);
			colortinta=leido&15;
			colorpapel=(leido>>4)&15;

			//printf ("modo 0\n");

		}
	}



        for (bit=0;bit<8;bit++) {
		if (byte_leido & 128 ) color=colortinta;
		else color=colorpapel;

		rainbow_buffer[y*get_total_ancho_rainbow()+x+bit]=color;

		byte_leido=(byte_leido&127)<<1;

        }

}

void screen_store_scanline_char_zx8081_border_scanline(int x,int y,z80_byte byte_leido)
{
        int bit;
        z80_byte color;

        z80_byte colortinta=0;
        z80_byte colorpapel=15;

        //Si modo chroma81 y lo ha activado
        if (color_es_chroma() ) {
		//color border
		colorpapel=chroma81_port_7FEF&15;
        }



        for (bit=0;bit<8;bit++) {
                if (byte_leido & 128 ) color=colortinta;
                else color=colorpapel;

                rainbow_buffer[y*get_total_ancho_rainbow()+x+bit]=color;

                byte_leido=(byte_leido&127)<<1;

        }

}

z80_int screen_return_border_ulaplus_color(void)
{
	//En modos lineal (radastan, 5, 7, 9) color del border depende de radaspalbank y sale de ulaplus
	//En resto ulaplus, sale de colores (8-15) de ulaplus
	int offset=8;

	if (ulaplus_extended_mode>=1) {

		z80_byte radaspalbank_offset=zxuno_get_radaspalbank_offset();

		offset=radaspalbank_offset;


/* bit 2:
BOR3: dentro de la paleta actual seleccionada, indica si el color del borde se tomará de las 
entradas 0 a 7 (0) o de las entradas 8 a 15 (1). Puede considerarse como el bit 3 del color del borde en modo radastaniano.
*/
		if (zxuno_ports[0x43]&4) offset+=8;
	}

	/*
		printf ("%d %d %06X %06X\n",ulaplus_palette_table[screen_border_last_color]+ULAPLUS_INDEX_FIRST_COLOR,ulaplus_palette_table[screen_border_last_color+8]+ULAPLUS_INDEX_FIRST_COLOR,
		spectrum_colortable[ulaplus_palette_table[screen_border_last_color]+ULAPLUS_INDEX_FIRST_COLOR],spectrum_colortable[ulaplus_palette_table[screen_border_last_color+8]+ULAPLUS_INDEX_FIRST_COLOR]
		);
	*/

	return ulaplus_palette_table[screen_border_last_color+offset]+ULAPLUS_INDEX_FIRST_COLOR;
}

void screen_incremento_border_si_ulaplus(void)
{
		//no necesario
		return;
                        //Modos ulaplus (cualquiera) el color del border es del puerto 254, indexado a la tabla de paper
                        if (ulaplus_presente.v && ulaplus_enabled.v) {
                                //screen_border_last_color=screen_border_last_color+ULAPLUS_INDEX_FIRST_COLOR+8;
                                screen_border_last_color=ulaplus_palette_table[screen_border_last_color+8]+ULAPLUS_INDEX_FIRST_COLOR;
                        }

}

void screen_incremento_border_si_spectra(void)
{
	if (spectra_enabled.v) {
		//Border mejorado y extra colours
		if ((spectra_display_mode_register&16) && (spectra_display_mode_register&4) ) {
				screen_border_last_color=screen_border_last_color+SPECTRA_INDEX_FIRST_COLOR;
		}
	}
}





//No hacer si modo ulaplus activo
#define PRISM_ADJUST_COLOUR_PALETTE \
			if (!(ulaplus_presente.v && ulaplus_enabled.v)) { \
                        switch (palette) { \
                                case 0: \
                                default: \
                                        color=PRISM_INDEX_FIRST_COLOR+prism_palette_zero[color+prism_offset_colour_screen_data_decoding_doce]; \
                                break; \
                                \
                                case 2: \
                                        color=PRISM_INDEX_FIRST_COLOR+prism_palette_two[color+prism_offset_colour_screen_data_decoding_doce]; \
                                break; \
                                \
                                case 1: \
                                        color=ULAPLUS_INDEX_FIRST_COLOR+color+prism_offset_colour_screen_data_decoding_doce; \
                                break; \
                        } \
			} \

//ultimo color leido por rutina de screen_store_scanline_rainbow_border_comun
z80_int screen_border_last_color;

//ultimo color leido por rutina de screen_store_scanline_rainbow_border_comun, en maquina prism y color de border 0
z80_int screen_border_last_color_prism;


z80_int screen_store_scanline_rainbow_border_get_colour(z80_byte ultimo_numero_color_border,z80_byte palette,z80_byte color_border_prism)
{
	//Esto se usa en dibujo de pantalla pero aqui tambien es comun, aunque el offset no tenga sentido
	z80_byte prism_offset_colour_screen_data_decoding_doce=0;

	z80_int color=ultimo_numero_color_border;

                        if (ulaplus_presente.v && ulaplus_enabled.v) {
                                //color=ulaplus_palette_table[screen_border_last_color+8]+ULAPLUS_INDEX_FIRST_COLOR;
                                color=screen_return_border_ulaplus_color();
				return color;
                        }

	//Si es cero
	if (color==0) {
                                color=color_border_prism;

                                PRISM_ADJUST_COLOUR_PALETTE

                                return color;
	}

	//No es cero
	PRISM_ADJUST_COLOUR_PALETTE


                        if (timex_video_emulation.v) {
                                z80_byte modo_timex=timex_port_ff&7;
                                if (modo_timex==4 || modo_timex==6) {
                                        color=get_timex_border_mode6_color();
                                }
                        }

	return color;

}



void screen_store_scanline_rainbow_border_comun_prism(z80_int *puntero_buf_rainbow,int xinicial)
{


        int ancho_pantalla=256;
        if (MACHINE_IS_PRISM) ancho_pantalla=PRISM_DISPLAY_WIDTH;

        int t_estados_por_pixel=2;
        if (MACHINE_IS_PRISM) t_estados_por_pixel=6;

        int indice_border=t_scanline*screen_testados_linea;
        int inicio_retrace_horiz=indice_border+(ancho_pantalla+screen_total_borde_derecho)/t_estados_por_pixel;
        int final_retrace_horiz=inicio_retrace_horiz+screen_invisible_borde_derecho/t_estados_por_pixel;
        //printf ("indice border: %d inicio_retrace_horiz: %d final_retrace_horiz: %d\n",indice_border,inicio_retrace_horiz,final_retrace_horiz);

        //X inicial de nuestro bucle. Siempre empieza en la zona de display-> al acabar borde izquierdo
        int x=screen_total_borde_izquierdo;

        z80_int border_leido;
	z80_int border_leido_prism;


        //Para modo interlace
        int y=t_scanline_draw;

        z80_int color_border;
        z80_int color_border_prism;

	//z80_int color;

	z80_byte palette=prism_ula2_registers[4];

	z80_byte ultimo_numero_color_border;

	ultimo_numero_color_border=screen_border_last_color;

	color_border_prism=screen_border_last_color_prism;

	color_border=screen_store_scanline_rainbow_border_get_colour(ultimo_numero_color_border,palette,color_border_prism);


        //Hay que recorrer el array del border para la linea actual
        int final_border_linea=indice_border+screen_testados_linea;
        for (;indice_border<final_border_linea;indice_border++) {
                //obtenemos si hay cambio de border

		border_leido_prism=prism_ula2_border_colour_buffer[indice_border];
		if (border_leido_prism!=65535) {
			screen_border_last_color_prism=border_leido_prism;
			color_border_prism=border_leido_prism;
			color_border=screen_store_scanline_rainbow_border_get_colour(ultimo_numero_color_border,palette,color_border_prism);
		}


                border_leido=fullbuffer_border[indice_border];
                if (border_leido!=255) {
                        screen_border_last_color=border_leido;
			ultimo_numero_color_border=border_leido;
			color_border=screen_store_scanline_rainbow_border_get_colour(ultimo_numero_color_border,palette,color_border_prism);


                }



                //Si estamos en x a partir del parametro inicial y Si no estamos en zona de retrace horizontal, dibujar border e incrementar posicion
                if (x>=xinicial) {
 //si nos pasamos de border izquierdo
                        if ( (indice_border<inicio_retrace_horiz || indice_border>=final_retrace_horiz) ) {
                                //Por cada t_estado van 6 pixeles en prism
                                        int jj;
	                                for (jj=0;jj<t_estados_por_pixel;jj++) store_value_rainbow(puntero_buf_rainbow,color_border);
                        }

                        //Se llega a siguiente linea
                        if (indice_border==inicio_retrace_horiz) y++;
                }

                x+=t_estados_por_pixel;

        }

	//Debido a desajustes con estados por linea en prism, si no agregamos esto, se queda una zona en negro entre el borde izquierdo y la pantalla central
	//Estos dos para zona donde hay borde izquierdo y derecho
	store_value_rainbow(puntero_buf_rainbow,color_border);
	store_value_rainbow(puntero_buf_rainbow,color_border);

	//Y estos para la primera linea de pantalla
	store_value_rainbow(puntero_buf_rainbow,color_border);
	store_value_rainbow(puntero_buf_rainbow,color_border);




}

unsigned int screen_store_scanline_border_si_incremento_real(unsigned int color_border)
{
	//if (ulaplus_presente.v==0 && spectra_enabled.v==0) color_border +=spectrum_palette_offset;

	//color_border +=spectrum_palette_offset;

	return color_border;
}


void screen_store_scanline_rainbow_border_comun(z80_int *puntero_buf_rainbow,int xinicial)
{


	if (MACHINE_IS_PRISM) {
		screen_store_scanline_rainbow_border_comun_prism(puntero_buf_rainbow,xinicial);
		return;
	}


	int ancho_pantalla=256;
	//if (MACHINE_IS_PRISM) ancho_pantalla=PRISM_DISPLAY_WIDTH;

	int t_estados_por_pixel=2;

	int indice_border=t_scanline*screen_testados_linea;
	int inicio_retrace_horiz=indice_border+(ancho_pantalla+screen_total_borde_derecho)/t_estados_por_pixel;
	int final_retrace_horiz=inicio_retrace_horiz+screen_invisible_borde_derecho/t_estados_por_pixel;
	//printf ("indice border: %d inicio_retrace_horiz: %d final_retrace_horiz: %d\n",indice_border,inicio_retrace_horiz,final_retrace_horiz);

	//X inicial de nuestro bucle. Siempre empieza en la zona de display-> al acabar borde izquierdo
	int x=screen_total_borde_izquierdo;

	z80_byte border_leido;

	//Para modo interlace
	int y=t_scanline_draw;

	z80_int color_border;

	color_border=screen_border_last_color;

	color_border=screen_store_scanline_border_si_incremento_real(color_border);

	if (ulaplus_presente.v && ulaplus_enabled.v) {
		//color_border=ulaplus_palette_table[screen_border_last_color+8]+ULAPLUS_INDEX_FIRST_COLOR;
		color_border=screen_return_border_ulaplus_color();
	}

	if (MACHINE_IS_TBBLUE) {
		//En tbblue, color border depends on several machine settings, has also own Timex mode handling
		color_border=tbblue_get_border_color(color_border);
	}
	else if (timex_video_emulation.v) {
		z80_byte modo_timex=timex_port_ff&7;
		if (modo_timex==4 || modo_timex==6) {
			color_border=get_timex_border_mode6_color();
		}
	}

    if (MACHINE_IS_ZXUNO && zxuno_is_prism_mode_enabled() ) {    
        color_border=zxuno_prism_get_border_color();  

        /*
        printf("last: %d color orig: %02X%02X%02XH color: %04XH\n",screen_border_last_color,
        zxuno_prism_current_palette[screen_border_last_color].rgb[0],
        zxuno_prism_current_palette[screen_border_last_color].rgb[1],
        zxuno_prism_current_palette[screen_border_last_color].rgb[2],
        zxuno_prism_current_palette[screen_border_last_color].index_palette_15bit);
        printf("%06XH\n",spectrum_colortable_normal[color_border]);
        */
    }


	//Hay que recorrer el array del border para la linea actual
	int final_border_linea=indice_border+screen_testados_linea;
	for (;indice_border<final_border_linea;indice_border++) {
		//obtenemos si hay cambio de border. En tbblue puede que no esté activado
		if (MACHINE_IS_TBBLUE && tbblue_store_scanlines_border.v==0) {
			border_leido=255; 
		}

		else {
			border_leido=fullbuffer_border[indice_border];
		}

		if (border_leido!=255) {

			screen_border_last_color=border_leido;
			color_border=screen_border_last_color;

			color_border=screen_store_scanline_border_si_incremento_real(color_border);

			//if (indice_border!=0) printf ("cambio color en indice_border=%d color=%d\n",indice_border,last_color);

			//screen_incremento_border_si_ulaplus();
			screen_incremento_border_si_spectra();

			if (ulaplus_presente.v && ulaplus_enabled.v) {
				//color_border=ulaplus_palette_table[screen_border_last_color+8]+ULAPLUS_INDEX_FIRST_COLOR;
				color_border=screen_return_border_ulaplus_color();
			}

			if (MACHINE_IS_TBBLUE) {
					//En tbblue, color border depends on several machine settings, has also own Timex mode handling
					color_border=tbblue_get_border_color(color_border);
			}
            
			else if (timex_video_emulation.v) {
				z80_byte modo_timex=timex_port_ff&7;
				if (modo_timex==4 || modo_timex==6) {
					color_border=get_timex_border_mode6_color();
				}
			}

            if (MACHINE_IS_ZXUNO && zxuno_is_prism_mode_enabled() ) {    
                color_border=zxuno_prism_get_border_color();  
            }
		}

		int ancho_rainbow=get_total_ancho_rainbow();

		//Si estamos en x a partir del parametro inicial y Si no estamos en zona de retrace horizontal, dibujar border e incrementar posicion
		if (x>=xinicial) {

			//si nos pasamos de border izquierdo
			if ( (indice_border<inicio_retrace_horiz || indice_border>=final_retrace_horiz) ) {
				//Por cada t_estado van 2 pixeles normalmente
					int jj;
					for (jj=0;jj<t_estados_por_pixel;jj++) {
						store_value_rainbow(puntero_buf_rainbow,color_border);
						if (MACHINE_IS_TBBLUE) {
							puntero_buf_rainbow[ancho_rainbow]=color_border; //pixel de abajo a la derecha
							puntero_buf_rainbow[ancho_rainbow-1]=color_border; //pixel de abajo 
							store_value_rainbow(puntero_buf_rainbow,color_border); //pixel de derecha y incrementamos
						}
							

					}
			}

			//Se llega a siguiente linea
			if (indice_border==inicio_retrace_horiz) {
				y++;
				//En caso de tbblue hay que saltar una linea mas en buffer rainbow, ya que hacemos doble de alto
				if (MACHINE_IS_TBBLUE) {
					puntero_buf_rainbow +=ancho_rainbow;
				}
			}
		}

		//Por cada t_estado van 2 pixeles
		x+=t_estados_por_pixel;

	}


}

//Guardar en buffer rainbow linea actual de borde superior o inferior
void screen_store_scanline_rainbow_border_comun_supinf(void)
{

	int scanline_copia=t_scanline_draw-screen_invisible_borde_superior;

	z80_int *puntero_buf_rainbow;

	int x=screen_total_borde_izquierdo;

	//printf ("%d\n",scanline_copia*get_total_ancho_rainbow());
	//esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
	puntero_buf_rainbow=&rainbow_buffer[scanline_copia*get_total_ancho_rainbow()+x];

	//Empezamos desde x en zona display, o sea, justo despues del ancho del borde izquierdo
	screen_store_scanline_rainbow_border_comun(puntero_buf_rainbow,x );


}






//para snow effect
z80_byte byte_leido_antes;
int snow_effect_counter=0;

//Con 2 se ve bien la demo de snow.tap. Pero se ve demasiado para robocop3 por ejemplo
int snow_effect_min_value=3;


//cada cuantos bytes hacemos snow
#define MIN_SNOW_EFFECT_COUNTER 45

//parece que deberia ser 2 lo normal
//int temp_min_snow=2;
//#define MIN_SNOW_EFFECT_COUNTER temp_min_snow

int snow_effect_counter_hang=0;
//contador que dice cuando se reseteara
#define MIN_SNOW_EFFECT_RESET_COUNTER 40000


//Devuelve 1 si reg_i apunta a memoria contended
//Si no apunta a memoria contended, reseteamos a 0 contador de snow_effect_counter_hang
//asi en juegos como VECTRON, que hacen el efecto intencionadamente,
//como no lo hace durante mucho tiempo, la maquina no se resetea (PC no va a 0)
int snow_effect_si_contend(void)
{
	//En modos 48k
	if (MACHINE_IS_SPECTRUM_16_48) {
		if (reg_i>=64 && reg_i<=127) return 1;
		else {
			snow_effect_counter_hang=0;
			return 0;
		}
	}

	//Otros casos, suponemos 128k
	z80_int segmento;
	segmento=reg_i / 64;
	if (contend_pages_actual[segmento]) return 1;
	else {
		snow_effect_counter_hang=0;
		return 0;
	}
}

int temp_min_snow=0;

//Devuelve 1 si hay que hacer snow effect
int si_toca_snow_effect(int x)
{


	//Maquinas +2A, +3 no tienen efecto snow
	if (MACHINE_IS_SPECTRUM_P2A_P3) return 0;



	if (snow_effect_si_contend () ) {

		//calcular en que t_estados estamos
		int estado=t_estados+(x*8)/2;

		//if (estado%8 == temp_min_snow) return 1;

		//ver si ese estado tiene contienda (valor mayor que 2 normalmente)
		//if (contend_table[estado]>temp_min_snow) return 1;
		//if (contend_table[estado]>=2 && contend_table[estado]<=3) return 1;


		//Valor 2 puesto a ojo. Si pongo valor mayor que 2, demo snow no se mueve nada
		//Valores 0, 1 o 2 parece que muestran snow.tap igual
		//if (contend_table[estado]>2) return 1;


		if (contend_table[estado]>=snow_effect_min_value) return 1;


		/*
		snow_effect_counter++;

                                                //cada X bytes, perdemos uno
                                                if (snow_effect_counter==MIN_SNOW_EFFECT_COUNTER) {
                                                        snow_effect_counter=0;

							snow_effect_counter_hang++;

							//printf ("snow_effect_counter_hang: %d\n",snow_effect_counter_hang);

							if (snow_effect_counter_hang==MIN_SNOW_EFFECT_RESET_COUNTER) {
								snow_effect_counter_hang=0;
								//Si maquina 48k, reseteamos cuando el contador llega al limite
								if (MACHINE_IS_SPECTRUM_16_48) {
									debug_printf (VERBOSE_DEBUG,"Reseting CPU due to snow effect, disabled");
									//reg_pc=0;
									//registro i ya se resetea desde la rom
									//reg_i=0;
								}
							}

							return 1;

                                                }

		*/

	}

	return 0;

}


//Guardar en buffer rainbow la linea actual. Modos ulaplus lineales, incluido radastan. Para Spectrum. solo display
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
//Comun para spectrum y modelo prism
void screen_store_scanline_rainbow_solo_display_ulaplus_lineal(void)
{

        //printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

        //linea que se debe leer
        int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

        int veces_ancho_pixel=1;
        if (MACHINE_IS_PRISM) {
                //Dado que es 192, dividir linea entre dos para duplicar pixeles en altura
                //printf ("dividir scanline copia\n");
                scanline_copia /=2;
                veces_ancho_pixel=2;
        }



        //la copiamos a buffer rainbow
        z80_int *puntero_buf_rainbow;
        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        int y;

        y=t_scanline_draw-screen_invisible_borde_superior;
        if (border_enabled.v==0) y=y-screen_borde_superior;

        puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];

        puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;

	int resta_offset=0;


        int x;
        z80_int direccion=0;
        z80_byte byte_leido;

        int color_rada;
        z80_byte *screen;



	if (ulaplus_extended_mode==9) {
		//Hacemos un "truco"
		z80_byte antes_puerto_32765=puerto_32765;
		puerto_32765 &=(255-8); //Quitamos bit de video shadow
		//Si es a partir de media pantalla para abajo, es ram7 (o vram2 en prism)
		if (scanline_copia>=96) {
			puerto_32765 |=8;
			resta_offset=12288;
		}

		screen=get_base_mem_pantalla();

		//Restauramos valor original
		puerto_32765=antes_puerto_32765;

		//printf ("%d %p\n",scanline_copia,screen);

	}

	else {
	        screen=get_base_mem_pantalla();
	}



	/*
                                modo 3 es radastan 128x96, aunque va en contra de la ultima especificacion

                                modo 5: 256x96

                                modo 7: 128x192

				modo 9: 256x192.
	*/

	//modos 3 y 5 duplican cada pixel por alto, por tanto:
	//pixel y=0->direccion de pantalla linea 0
	//pixel y=1->direccion de pantalla linea 0
	//pixel y=2->direccion de pantalla linea 1
        //pixel y=3->direccion de pantalla linea 1
        //pixel y=4->direccion de pantalla linea 2
	//etc...


	//Offset de pantalla
	z80_int radasoffset=0;



	int bytes_por_linea=0;
	int incremento_x=0;

	switch (ulaplus_extended_mode) {
		//Radastan 128x96
		case 3:
			//dividimos y/2
			scanline_copia/=2;
			//bytes_por_linea=64;
			incremento_x=2;

			//Obtener radasoffset. Es de 14 bits
			radasoffset=zxuno_radasoffset&16383;

			//Obtener radaspadding
			bytes_por_linea=64+zxuno_ports[0x42];


		break;

		//256x96
                case 1:
                        //dividimos y/2
                        scanline_copia/=2;
                        bytes_por_linea=128;
			incremento_x=1;
                break;

		//128x192
                case 5:
                        bytes_por_linea=64;
			incremento_x=2;
                break;

		//256x192
		case 9:
			bytes_por_linea=128;
			incremento_x=1;
                break;



	}



	//direccion=direccion-resta_offset+bytes_por_linea*scanline_copia;

	//Teniendo en cuenta registros radasoffset, radaspadding
	direccion=direccion-resta_offset+radasoffset+bytes_por_linea*scanline_copia;

	//printf ("y: %d pun: %ld\n",y,&screen[direccion]);

	z80_byte radaspalbank_offset=zxuno_get_radaspalbank_offset();


        for (x=0;x<128;x+=incremento_x) {

					direccion=direccion % 16384; //Evitar que se salga de vram.

			//temp controlar esto
			//if (direccion>22527) printf ("direccion: %d scanline_copia: %d\n",direccion,scanline_copia);

			//Cada byte tiene dos pixeles de color
			//Cada pixel duplicado en ancho en modos 3 y 7

			//if (ulaplus_mode==9) {
			//	byte_leido=peek_byte_no_time(direccion);
			//}
			//else {
	                        byte_leido=screen[direccion];
			//}
                        color_rada=ulaplus_palette_table[radaspalbank_offset+(byte_leido>>4)]+ULAPLUS_INDEX_FIRST_COLOR;
                        //if (color_rada>15) printf ("c: %d ",radaspalbank_offset+(byte_leido>>4));

			int i;

			for (i=0;i<veces_ancho_pixel;i++) {
				store_value_rainbow(puntero_buf_rainbow,color_rada);
				if (incremento_x==2) store_value_rainbow(puntero_buf_rainbow,color_rada);
			}



                        color_rada=ulaplus_palette_table[radaspalbank_offset+(byte_leido&15)]+ULAPLUS_INDEX_FIRST_COLOR;


			for (i=0;i<veces_ancho_pixel;i++) {
				store_value_rainbow(puntero_buf_rainbow,color_rada);
				if (incremento_x==2) store_value_rainbow(puntero_buf_rainbow,color_rada);
			}


                        direccion++;

        }




}

//Para modo 16C de pentagon
void screen_store_scanline_rainbow_solo_display_16c(void)
{

        //printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

        //linea que se debe leer
        int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

              

        //la copiamos a buffer rainbow
        z80_int *puntero_buf_rainbow;
        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        int y;

        y=t_scanline_draw-screen_invisible_borde_superior;
        if (border_enabled.v==0) y=y-screen_borde_superior;

        puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];

        puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;



        int x;
        z80_int direccion;
        

        //z80_byte *screen;


		//screen=get_base_mem_pantalla();

		direccion=screen_addr_table[(scanline_copia<<5)];

		/*
		Pentagon 16C mode
		More info:

		http://speccy.info/16col

		http://zxpress.ru/article.php?id=8610

		*/

		
		int page1=5;
		int page2=4;
		if (puerto_32765 & 8) {
			page1=7;
			page2=6;
		}

		z80_byte *vram1;
		z80_byte *vram2;
		z80_byte *vram3;
		z80_byte *vram4;

		vram1=ram_mem_table[page2];
		vram2=ram_mem_table[page1];
		vram3=ram_mem_table[page2]+0x2000;
		vram4=ram_mem_table[page1]+0x2000;

		vram1 +=direccion;
		vram2 +=direccion;
		vram3 +=direccion;
		vram4 +=direccion;

		int pix;

		z80_byte byte_leido;

        for (x=0;x<32;x++) {

			for (pix=0;pix<4;pix++) {
				int color_izq,color_der;			

				//Bytes orden @RAM4 , @RAM5, @RAM4|0x2000, @RAM5|0x2000

				switch (pix) {


					case 0:
						byte_leido=*vram1;
					break;


					case 1:
						byte_leido=*vram2;
					break;

					case 2:
						byte_leido=*vram3;
					break;	

					default:
						byte_leido=*vram4;
					break;									


				}

				z80_byte brillo_izq,brillo_der;

				//Codificacion en byte:
				// 7 6  5  4  3  2  1  0
				//BD BI CD CD CD CI CI CI
				//BD: brillo pixel derecho
				//BI: brillo pixel izquierdo
				//CD: color pixel derecho
				//CI: color pixel izquierdo 

				//de bit 6 a bit 3
				brillo_izq=(byte_leido >>3)&0x8;

				color_izq=byte_leido & 0x07;
				color_izq |=brillo_izq;



				//de bit 7 a bit 3
				brillo_der=(byte_leido >>4)&0x8;

				color_der=(byte_leido >> 3)&0x07;
				color_der |=brillo_der;
				

				store_value_rainbow(puntero_buf_rainbow,color_izq);
				store_value_rainbow(puntero_buf_rainbow,color_der);

				

			}
			//direccion++;
			vram1++;
			vram2++;
			vram3++;
			vram4++;
		}
	        


}


z80_int spectra_get_which_ram_display(void)
{

	z80_int indice=0;

        //Ver que ram es la que muestra el display
        //if (spectra_display_mode_register&32) indice=16384;
        //indice=spectra_get_which_ram_display();

	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
                       //128k display selected
                       z80_byte display_128_selected=(puerto_32765&8 ? 1 : 0);

                        //Look at spectra display bank
                       z80_byte display_spectra_selected=(spectra_display_mode_register&32 ? 1 : 0);

			z80_byte which_display=(display_128_selected ^ display_spectra_selected);

			if (which_display==1) indice=16384;
	}

	else {
		//48k
		if (spectra_display_mode_register&32) indice=16384;
	}

	return indice;
}


void screen_store_scanline_rainbow_solo_display_spectra(void)
{
        //printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

        //linea que se debe leer
        int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

        //la copiamos a buffer rainbow
        z80_int *puntero_buf_rainbow;
        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        int y;

        y=t_scanline_draw-screen_invisible_borde_superior;
        if (border_enabled.v==0) y=y-screen_borde_superior;

        puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];

        puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;


        int x,bit;
        z80_int direccion,dir_atributo;
        z80_byte byte_leido;


        int color=0;

        z80_byte attribute,bright,flash;

	//Segundo color de atributo para modos doublebyte
	z80_byte attribute2;

	//Segundo bit de brillo y parpadeo para modos doublebyte
	z80_byte bright2,flash2;

        z80_int ink,paper,aux;

	//modos halfcell
        z80_int inkleft,paperleft,inkright,paperright;


        //z80_byte *screen=get_base_mem_pantalla();
	z80_byte *screen;
	z80_int indice=0;
	//Ver que ram es la que muestra el display
	//if (spectra_display_mode_register&32) indice=16384;
	indice=spectra_get_which_ram_display();

	screen=&spectra_ram[indice];

        direccion=screen_addr_table[(scanline_copia<<5)];


        //fila=scanline_copia/8;
        //dir_atributo=6144+(fila*32);

	//Prueba Row/Quad
	int spectra_line_height=spectra_display_mode_register&3;

	//0,1 o 2
	int numero_area=scanline_copia/64;

	int numero_row=(scanline_copia/8)&7;

	int numero_pixel_line=scanline_copia%8;

	int spectra_basicextra_colors=(spectra_display_mode_register>>2)&1;

	int spectra_doublebyte_colors=(spectra_display_mode_register>>3)&1;

	int spectra_half_cell=(spectra_display_mode_register>>7)&1;

	//Cuanto hay que sumar para obtener el segundo byte de atributo
	z80_int offset_double_byte_colour;

/*
The area number is specified by bits An, the row number within an area by bits Rn, the pixel line number within a row by bits Ln and the column number by bits Cn.
*/

	switch (spectra_line_height) {
		//Row
		case 0:
			dir_atributo=6144+(numero_row*32)+(numero_area*256);
			offset_double_byte_colour=1024;
			//printf ("modo row. dir_atributo: %d numero_area: %d numero_row: %d numero_pixel_line: %d\n",dir_atributo,numero_area,numero_row,numero_pixel_line);
		break;

		//Quad
		case 1:
			dir_atributo=8192+(numero_row*32)+(numero_pixel_line/4)*256+(numero_area*512);
			offset_double_byte_colour=2048;
			//printf ("modo quad. dir_atributo: %d numero_area: %d numero_row: %d numero_pixel_line: %d\n",dir_atributo,numero_area,numero_row,numero_pixel_line);
		break;

		//Dual
		case 2:
			offset_double_byte_colour=4096;
			dir_atributo=8192+(numero_row*32)+(numero_pixel_line/2)*256+(numero_area*1024);
			//printf ("modo dual. dir_atributo: %d numero_area: %d numero_row: %d numero_pixel_line: %d\n",dir_atributo,numero_area,numero_row,numero_pixel_line);
		break;

		//Single
		case 3:
			if (spectra_doublebyte_colors==0) {
				dir_atributo=8192+(numero_row*32)+(numero_pixel_line)*256+(numero_area*2048);
			}

			else {
				//Double byte colors en este modo es especial
				if (scanline_copia<128) {
					dir_atributo=8192+(numero_row*32)+(numero_pixel_line)*256+((numero_area&1)*2048);
					offset_double_byte_colour=4096;
				}
				else {
					dir_atributo=(4096+2048)+(numero_row*32)+((numero_pixel_line>>1)&3)*256;
					offset_double_byte_colour=1024;
				}

			}

			//printf ("modo single. dir_atributo: %d numero_area: %d numero_row: %d numero_pixel_line: %d\n",dir_atributo,numero_area,numero_row,numero_pixel_line);
		break;

	}


        for (x=0;x<32;x++) {


			//Obtener coloures en base a:
			//Half/Full cell
			//Single/Double byte colours
			//Basic/Extra colours

                        byte_leido=screen[direccion];

			attribute=screen[dir_atributo];

			if (spectra_doublebyte_colors) {
				attribute2=screen[dir_atributo|offset_double_byte_colour];

				//temp
				//if ( (dir_atributo|offset_double_byte_colour)>16383) printf ("fuera de rango :%d\n",dir_atributo|offset_double_byte_colour);
			}



			//Obtener colores
			if (spectra_basicextra_colors==0)  {
				//Colores basicos.
				if (spectra_doublebyte_colors==0) {
					//Colores basicos. Single byte colour
					if (spectra_half_cell) {
						//Colores basicos. Single byte colour. Half cell

						inkright=attribute &7;
						inkleft=(attribute>>3) &7;
						paperleft=paperright=0;


						bright=(attribute)&64;
						flash=(attribute)&128;

						if (bright) {
							inkleft+=8;
							inkright+=8;
						}


						if (flash) {
        		                        	if (estado_parpadeo.v) {
	        	        	                        aux=paperright;
        	        	        	                paperright=inkright;
                	        	        	        inkright=aux;
	        	        	                        aux=paperleft;
        	        	        	                paperleft=inkleft;
                	        	        	        inkleft=aux;
	                	        	        }
						}
					}


					else {
						//Colores basicos. Single byte colour. Full Cell
			                 	ink=attribute &7;
						paper=(attribute>>3) &7;
						bright=(attribute)&64;
						flash=(attribute)&128;

						if (bright) {
							paper+=8;
							ink+=8;
						}

		                        	if (flash) {
        		                        	if (estado_parpadeo.v) {
	        	        	                        aux=paper;
        	        	        	                paper=ink;
                	        	        	        ink=aux;
	                	        	        }
	        	                	}
					}

				}

				//Colores basicos. Double byte colour
				else {

					if (spectra_half_cell==0) {
						//Colores basicos. Double byte colour. Full cell
		                        	ink=attribute &7;
        			                bright=(attribute)&64;
                			        flash=(attribute)&128;

        	        		        if (bright) {
                        		        	ink+=8;
	                        		}

			                        paper=attribute2 &7;
        			                bright2=(attribute2)&64;
                			        flash2=(attribute2)&128;

        	        		        if (bright2) {
                	        		        paper+=8;
	                        		}

						//Flash de los dos
		                        	if (flash && flash2) {
        		                        	if (estado_parpadeo.v) {
	                		                        aux=paper;
        	                		                paper=ink;
                	                		        ink=aux;
		                	                }
        		                	}

						//Flash solo de tinta
						else if (flash) {
        	                	        	if (estado_parpadeo.v) {
                	                		        ink=paper;
	                	                	}
						}

						//Flash solo de paper
						else if (flash2) {
        	                	        	if (estado_parpadeo.v) {
                	                		        paper=ink;
	                	                	}
						}

					}

					else {
                                                //Colores basicos. Double byte colour. Half cell
                                                inkright=attribute &7;
						inkleft=(attribute>>3)&7;
                                                bright=(attribute)&64;
                                                flash=(attribute)&128;

                                                if (bright) {
                                                        inkleft+=8;
                                                        inkright+=8;
                                                }

                                                paperright=attribute2 &7;
                                                paperleft=(attribute2>>3)&7;
                                                bright2=(attribute2)&64;
                                                flash2=(attribute2)&128;

                                                if (bright2) {
                                                        paperleft+=8;
                                                        paperright+=8;
                                                }

                                                //Flash de los dos
                                                if (flash && flash2) {
                                                        if (estado_parpadeo.v) {
                                                                aux=paperleft;
                                                                paperleft=inkleft;
                                                                inkleft=aux;
                                                                aux=paperright;
                                                                paperright=inkright;
                                                                inkright=aux;
                                                        }
                                                }

                                                //Flash solo de tinta
                                                else if (flash) {
                                                        if (estado_parpadeo.v) {
                                                                inkleft=paperleft;
                                                                inkright=paperright;
                                                        }
                                                }

                                                //Flash solo de paper
                                                else if (flash2) {
                                                        if (estado_parpadeo.v) {
                                                                paperleft=inkleft;
                                                                paperright=inkright;
                                         	       }
	                                        }

					}

				}


			}

			else {

				//Colores extendidos
				if (spectra_doublebyte_colors==0) {
					//Colores extendidos. Single byte colour
					if (spectra_half_cell==0) {
						//Colores extendidos. Single byte colour. Full cell
						ink=SPECTRA_INDEX_FIRST_COLOR+(attribute&63);


						//blanco
						if (attribute&64) paper=SPECTRA_INDEX_FIRST_COLOR+63;

						//o negro
						else paper=SPECTRA_INDEX_FIRST_COLOR+0;

						flash=(attribute)&128;

	                        		if (flash) {
		        	                        if (estado_parpadeo.v) {
        		        	                        aux=paper;
                		        	                paper=ink;
                        		        	        ink=aux;
	                        		        }
	        	                	}
					}
					else {
						//Colores extendidos. Single byte colour. Half cell
						inkright=SPECTRA_INDEX_FIRST_COLOR+(attribute&63);

                                                //blanco
                                                if (attribute&64) inkleft=SPECTRA_INDEX_FIRST_COLOR+63;

                                                //o negro
                                                else inkleft=paperright=SPECTRA_INDEX_FIRST_COLOR+0;

						//Papel siempre negro
						paperleft=paperright=SPECTRA_INDEX_FIRST_COLOR+0;


						flash=(attribute)&128;

                                                if (flash) {
                                                        if (estado_parpadeo.v) {
                                                                aux=paperright;
                                                                paperright=inkright;
                                                                inkright=aux;
                                                                aux=paperleft;
                                                                paperleft=inkleft;
                                                                inkleft=aux;
                                                        }
                                                }


					}
				}
				else {
					//Colores extendidos.  Double byte colour
					if (spectra_half_cell==0) {
						//Colores extendidos.  Double byte colour. full cell

	                                        ink=SPECTRA_INDEX_FIRST_COLOR+(attribute&63);
        	                                flash=(attribute)&128;


                	                        paper=SPECTRA_INDEX_FIRST_COLOR+(attribute2&63);
                        	                flash2=(attribute2)&128;


                                	        //Flash de los dos
                                        	if (flash && flash2) {
	                                                if (estado_parpadeo.v) {
        	                                                aux=paper;
                	                                        paper=ink;
                        	                                ink=aux;
                                	                }
                                        	}

		                                //Flash solo de tinta
                 	                       else if (flash) {
                        	                        if (estado_parpadeo.v) {
                                	                        ink=paper;
                                        	        }
	                                        }

        	                                //Flash solo de paper
                	                        else if (flash2) {
                        	                        if (estado_parpadeo.v) {
                                	                        paper=ink;
                                        	        }
	                                        }
					}

					else {
						//Colores extendidos.  Double byte colour. Half cell
                                               inkright=SPECTRA_INDEX_FIRST_COLOR+(attribute&63);
                                                flash=(attribute)&128;


                                                paperright=SPECTRA_INDEX_FIRST_COLOR+(attribute&63);
						/*
						Segun la documentacion:
						Although the format includes a paper bit to allow support for two background colours
						(0=black and 1=white), this is not supported by the SPECTRA interface due to a lack
						of resources and so only a paper colour of black is available
						*/
						paperright=SPECTRA_INDEX_FIRST_COLOR;



                                                inkleft=SPECTRA_INDEX_FIRST_COLOR+(attribute2&63);
						flash2=(attribute2)&128;

                                                paperleft=SPECTRA_INDEX_FIRST_COLOR+(attribute2&63);
						paperleft=SPECTRA_INDEX_FIRST_COLOR;



                                                //Flash de los dos
                                                if (flash && flash2) {
                                                        if (estado_parpadeo.v) {
                                                                aux=paperleft;
                                                                paperleft=inkleft;
                                                                inkleft=aux;
                                                                aux=paperright;
                                                                paperright=inkright;
                                                                inkright=aux;
                                                        }
                                                }

                                                //Flash solo de derecha
                                                else if (flash) {
                                                        if (estado_parpadeo.v) {
                                                                inkright=paperright;
                                                        }
                                                }

                                                //Flash solo de izquierda
                                                else if (flash2) {
                                                        if (estado_parpadeo.v) {
                                                                inkleft=paperleft;
                                                        }
                                                }
					}
				}

			}





			if (spectra_half_cell==0) {
	                        for (bit=0;bit<8;bit++) {

        	                        color= ( byte_leido & 128 ? ink : paper ) ;

        	                        store_value_rainbow(puntero_buf_rainbow,color);
					//if (spectra_basicextra_colors && color!=SPECTRA_INDEX_FIRST_COLOR && x<5) printf ("x: %d color: %d\n",x,color);

                        	        byte_leido=byte_leido<<1;
	                        }
			}

			else {
                                for (bit=0;bit<4;bit++) {

                                        color= ( byte_leido & 128 ? inkleft : paperleft ) ;

                                        store_value_rainbow(puntero_buf_rainbow,color);

                                        byte_leido=byte_leido<<1;
                                }
                                for (;bit<8;bit++) {

                                        color= ( byte_leido & 128 ? inkright : paperright ) ;

                                        store_value_rainbow(puntero_buf_rainbow,color);

                                        byte_leido=byte_leido<<1;
                                }

			}


                        direccion++;
                        dir_atributo++;

                }


}

void screen_get_pixel_ink_paper_common_prism(z80_byte screendatadecoding,z80_byte attribute0,z80_int *p_ink0,z80_int *p_paper0)
{

                //z80_byte attribute1,attribute2,attribute3;
                z80_byte bright0,bright2;
                //z80_byte bright1,bright3;
                z80_byte flash0;
		z80_int ink0;
                //z80_int ink1,ink2,ink3;
                z80_int paper0;
		//z80_int paper1,paper2,paper3;
		z80_int aux;
		z80_byte clut;

	//Color ulaplus
	if (ulaplus_presente.v && ulaplus_enabled.v) {
	        z80_byte attribute,bright,flash;
        	z80_int ink,paper;
		attribute=attribute0;
		GET_PIXEL_ULAPLUS_COLOR
	        *p_ink0=ink;
	        *p_paper0=paper;
		//printf ("paper %d ink %d\n",*p_paper0,*p_ink0);
		//temp
		return;
	}


	switch (screendatadecoding) {

	case 0:
	default:
		//Normal. SPECTRUM ATTR: D0-D2 ink, D3-D5, paper, D6 bright, D7 Flash
		ink0=attribute0 &7;
		paper0=(attribute0>>3) &7;
		bright0=(attribute0)&64;
		flash0=(attribute0)&128;
		if (flash0) {
		  if (estado_parpadeo.v) {
		 aux=paper0;
		 paper0=ink0;
		 ink0=aux;
		  }
		}

		if (bright0) {
		  paper0+=8;
		  ink0+=8;
		}
	break;

	case 1:
		//16+16 Colour: ATTR: D0-D2 ink, D3-D5, paper, D6 ink bright, D7 paper bright
		ink0=attribute0 &7;
		paper0=(attribute0>>3) &7;
		bright0=(attribute0)&64;
		bright2=(attribute0)&128;
		if (bright0) {
		  ink0+=8;
		}
		if (bright2) {
		  paper0+=8;
		}
	break;

	case 2:
		//32 Colour ATTR: D0-D2 ink, D3-D5 paper, D6-D7 "CLUT". In the standard palette, colours 0-15 are the same as the normal Spectrum palette and colours 16-31 are darker versions of the normal spectrum palette)
		ink0=attribute0 &7;
		paper0=(attribute0>>3) &7;
		clut=((attribute0)&(64+128))>>6;
		ink0 +=8*clut;
		paper0 +=8*clut;
	break;

	case 4:
		//256 Colour mode 1 - D0-D7 = ink colour. Paper colour is determined by ULA2 BORDER (IO 0x9E3B)
		ink0=attribute0;
		paper0=get_prism_ula2_border_colour();
	break;
	}


	*p_ink0=ink0;
	*p_paper0=paper0;

}

//Funcion que dibuja el border centrar en modos lineales de menos de 192 pixeles de alto
void screen_prism_dibuja_border_central(void)
{

	screen_store_scanline_rainbow_border_comun_supinf();
}

int screen_prism_get_blend_color(int color1, int color2)
{

	//Para mezcla gigablend
						int red_a,green_a,blue_a;
						int red_b,green_b,blue_b;
						int rgb_a,rgb_b,red_final,green_final,blue_final;
						int color_final;
	//de momento paleta por defecto prism_palette_zero[color]
						//Obtenemos color 12 bits
						rgb_a=prism_palette_zero[color1];
						red_a=(rgb_a>>8)&15;
						green_a=(rgb_a>>4)&15;
						blue_a=(rgb_a)&15;

						rgb_b=prism_palette_zero[color2];
						red_b=(rgb_b>>8)&15;
						green_b=(rgb_b>>4)&15;
						blue_b=(rgb_b)&15;

						//Y montamos colores finales
						//RED= REDa(3) & REDb(2) & REDb(1) & REDa(0)
						//GREEN = REDa(3) & REDb(2) & REDb(1) & REDa(0)
						//BLUE = REDa(3) & REDb(2) & REDb(1) & REDa(0)
						red_final=(red_a &8) | (red_b &4) | (red_b & 2) | (red_a & 1);
						green_final=(green_a &8) | (green_b &4) | (green_b & 2) | (green_a & 1);
						blue_final=(blue_a &8) | (blue_b &4) | (blue_b & 2) | (blue_a & 1);


						//temp mi metodo de gigablend
						red_final=(red_a+red_b)/2;
						green_final=(green_a+green_b)/2;
						blue_final=(blue_a+blue_b)/2;

						//Montamos color final rgb 12 bits
						color_final=(red_final<<8) | (green_final<<4) | (blue_final);
	return color_final;						
}


void screen_store_scanline_rainbow_solo_display_prism(void)
{

	if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {


	        //linea que se debe leer
        	int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

	        //la copiamos a buffer rainbow
        	z80_int *puntero_buf_rainbow;
	        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        	int y;

	        y=t_scanline_draw-screen_invisible_borde_superior;
        	if (border_enabled.v==0) y=y-screen_borde_superior;

	        puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];

        	puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;


	        int x,bit;
        	z80_int direccion;

	        z80_byte byte_leido0,byte_leido1,byte_leido2,byte_leido3;


	        int color=0;
        	int color2=0;
	        int fila;

	        z80_byte attribute0,attribute2;
		//z80_byte attribute1,attribute3;
		z80_byte bright0,bright2;
		//z80_byte bright1,bright3;
		z80_byte flash0,flash2;
	        z80_int ink0,ink2;
	        //z80_int ink1,ink3;
		z80_int paper0,paper2,aux;
		//z80_int paper1,paper3;


		



	        z80_byte screentype=prism_ula2_registers[2];
        	z80_byte screendatadecoding=prism_ula2_registers[3];
	        z80_byte palette=prism_ula2_registers[4];
		z80_byte overlay_mode_type=prism_ula2_registers[7];

		z80_byte linear_mode=0;
		z80_byte linear_mode_mode;
		z80_byte prism_offset_colour_screen_data_decoding_doce=0;

		//Modos permitidos linear. De momento solo 2 y 3
		if (screentype==8) {
			linear_mode_mode=prism_ula2_registers[6];
			if (linear_mode_mode==2 ||linear_mode_mode==3) {
				linear_mode=1;
			}
		}


		//Asumimos que es ancho 256 y por tanto hay que hacer doble pixel en ancho
		int ancho_256=1;

		//Compabilidad con modos timex. Si es timex 8x1, hacemos prism modo 4, que es lo mismo
		z80_byte timex_video_mode=timex_port_ff&7;

                switch (timex_video_mode) {
			case 1:
				screentype=0;
			break;

                        case 2:
                                //Color hi-res 8x1
				screentype=4;
                        break;

                        case 4:
                        case 6:
                                //512x192 monocromo
				ancho_256=0;
			break;
		}




		//Modos de 512 de ancho
		switch (screentype) {
			case 1:  //Modo 0001 - 512x192, 8x8 Attributes (2 normal Spectrum screens next to each other: either VRAM0+VRAM1 or VRAM2+VRAM3 - NOT interpolated monochrome like Timex)
				ancho_256=0;
			break;

			case 3: //0011 - 512x384, 8x8 Attributes 4 normal spectrum screens: VRAM0 to the left of VRAM1 on top of VRAM2 to the left of VRAM3
				ancho_256=0;
			break;
		}


        	z80_byte *screen0,*screen1,*screen2,*screen3;


		z80_int red,green,blue;


		z80_int colortransparente1,colortransparente2;


	        //z80_byte clut;


        	z80_byte *puntero_buffer_atributos0,*puntero_buffer_atributos1,*puntero_buffer_atributos2,*puntero_buffer_atributos3;

		screen0=prism_vram_mem_table[0]; //VRAM0
		screen1=prism_vram_mem_table[1]; //VRAM1
		screen2=prism_vram_mem_table[2]; //VRAM2
		screen3=prism_vram_mem_table[3]; //VRAM3


		//Si modo timex1, intercambiar direcciones de vram
		if (timex_video_mode==1) {
			screen0=prism_vram_mem_table[1]; //VRAM1
			screen1=prism_vram_mem_table[0]; //VRAM0
		}

		//Casos de resoluciones 384 de alto
		switch (screentype) {
			case 2:  //0010 - 256x384, 8x8 Attributes (2 normal Spectrum screens one on top of the other: either VRAM0+VRAM2 or VRAM1+VRAM3)

				if ((puerto_32765 & 8)==0) {

	                        	//Ver desde que vram pillamos los datos. si la superior o la inferior
					if (scanline_copia>=192) {
						screen0=screen2;
						scanline_copia-=192;
					}
				}

				else {
					//Bit de pagina de vram activo

	                        	//Ver desde que vram pillamos los datos. si la superior o la inferior
					if (scanline_copia>=192) {   //Zona inferior
						screen0=screen3;
						scanline_copia-=192;
					}

					else screen0=screen1;        //Zona superior

				}

			break;

			case 6: //256x384, 8x1 Attributes (Two Timex 8x1 mode screens stacked one above the other: VRAM0 pixel data with attrs from VRAM1 above VRAM2 pixel data with attrs from VRAM3)


                                        //Ver desde que vram pillamos los datos. si la superior o la inferior
                                        if (scanline_copia>=192) {
                                                screen0=screen2;
						screen1=screen3;
                                                scanline_copia-=192;
                                        }


                        break;


			case 3: //0011 - 512x384, 8x8 Attributes 4 normal spectrum screens: VRAM0 to the left of VRAM1 on top of VRAM2 to the left of VRAM3
				//Ver desde que vram pillamos los datos. si la superior o la inferior
                                if (scanline_copia>=192) {
                                        screen0=screen2;
					screen1=screen3;
                                        scanline_copia-=192;
                                }
			break;


			default:
				//Dado que es 192, dividir linea entre dos para duplicar pixeles en altura
				scanline_copia /=2;
				//Intercambio de paginas de manera generica si bit paginacion
				//Nota: En modos que usan las 4 paginas (como planar, o 512x384) no tiene mucho sentido alterar este bit de paginacion
				if (puerto_32765 & 8) {

					//En modo Gigablend no alteramos esto
					if (screendatadecoding!=10) {
						screen0=prism_vram_mem_table[2]; //VRAM2
						screen1=prism_vram_mem_table[3]; //VRAM3
						screen2=prism_vram_mem_table[0]; //VRAM0
						screen3=prism_vram_mem_table[1]; //VRAM1
					}
				}
			break;
		}


		//Direcciones de la posicion de pantalla en direccionamiento de pantalla de spectrum habitual (no linear)
		if (!linear_mode) {
			direccion=screen_addr_table[(scanline_copia<<5)];

			fila=scanline_copia/8;

			puntero_buffer_atributos0=screen0 + 6144 + fila*32;
			puntero_buffer_atributos1=screen1 + 6144 + fila*32;
			puntero_buffer_atributos2=screen2 + 6144 + fila*32;
			puntero_buffer_atributos3=screen3 + 6144 + fila*32;

		}

		else {

			//Esto de momento no los uso pero los inicializo para que no den segmentation faults y similar
			puntero_buffer_atributos0=puntero_buffer_atributos1=puntero_buffer_atributos2=puntero_buffer_atributos3=screen0;

			linear_mode_mode=prism_ula2_registers[6];
			//printf ("modo linear. mode=%d\n",linear_mode_mode);
			int linea_linear;
			int vram_actual;
			int sc32;
			switch (linear_mode_mode) {
				case 2:

				//0010 - 128x128 res, 8 bits per pixel (256 colour). First 64 lines from VRAM0, next 64 from VRAM1

					//Control de maximo y
                                        if (scanline_copia<32 || scanline_copia>=160) {
                                                //printf ("Volver. scanline_copia=%d\n",scanline_copia);
						//dibujar ese segmento con color de border
						screen_prism_dibuja_border_central();
                                                return;
                                        }

                                        sc32=scanline_copia-32;
                                        linea_linear=sc32&63;
                                        direccion=linea_linear*128;

                                        vram_actual=sc32/64;
                                        screen0=prism_vram_mem_table[vram_actual];

                                        //printf ("vram: %d screen0: %p direccion: %d linea_linear: %d scanline_copia: %d\n",
                                        //      vram_actual,screen0,direccion,linea_linear,scanline_copia);


				break;

				case 3:

				//0011 - 256x128 pixels, one byte per pixel, linear. first 32 lines from VRAM0, next 32 from VRAM1, next 32 from VRAM2, final 32 from VRAM3
					//Control de maximo y
					if (scanline_copia<32 || scanline_copia>=160) {
						//printf ("Volver. scanline_copia=%d\n",scanline_copia);
						//dibujar ese segmento con color de border
						screen_prism_dibuja_border_central();
						return;
					}

					sc32=scanline_copia-32;
					linea_linear=sc32&31;
					direccion=linea_linear*256;

					vram_actual=sc32/32;
					screen0=prism_vram_mem_table[vram_actual];

					//printf ("vram: %d screen0: %p direccion: %d linea_linear: %d scanline_copia: %d\n",
					//	vram_actual,screen0,direccion,linea_linear,scanline_copia);
				break;
			}

		}

		//Preparacion para modos overlay 8 y 9
                colortransparente1=0;

                if (screendatadecoding==8) colortransparente2=8;
                else colortransparente2=0;

		int bucle_ancho;

		//Repetir el bucle dos veces si ancho es 512
		if (!ancho_256) bucle_ancho=2;
		else bucle_ancho=1;

		for (;bucle_ancho>0;bucle_ancho--) {

		for (x=0;x<32;x++) {


			byte_leido0=screen0[direccion];
			byte_leido1=screen1[direccion];
			byte_leido2=screen2[direccion];
			byte_leido3=screen3[direccion];

			//printf ("X1\n");

			attribute0=puntero_buffer_atributos0[x];
			//attribute1=puntero_buffer_atributos1[x];
			attribute2=puntero_buffer_atributos2[x];
			//attribute3=puntero_buffer_atributos3[x];

			//printf ("X2\n");

			if (timex_video_mode==4 || timex_video_mode==6) {
				if (x&1) byte_leido0=byte_leido1;
			}


			if (screentype==4 || screentype==6) { //256x192, 8x1 Attributes (Same as Timex 8x1 mode), 256x384 8x1
				attribute0=screen1[direccion];
			}

			switch (screendatadecoding) {
				case 5: ////0101 - 256 Colour mode 2 (256 colour overlay mode - 3 colours in each 8x8 square)
				break;


				case 6: //4 Plane planar mode (16 colour "clashless" mode)
				case 7: //3 Plane planar mode (Cheveron Mode)
				break;

				case 12: //4 Plane planar mode (256 colour "clashless" mode)
					//Si es 0, se suma 0
					//Si es 1, se suma 16
					//...
					//Si es 15, se suma 240
					prism_offset_colour_screen_data_decoding_doce=(attribute0&15)*16;
				break;

				case 8: //1000 - Overlay mode 1
				case 9: //1001 - Overlay mode 2
					//printf ("Overlay mode type: %d\n",overlay_mode_type);
					/*
                                	ink0=attribute0 &7;
					paper0=(attribute0>>3) &7;
					bright0=(attribute0)&64;
					if (bright0) {
						ink0+=8;
						paper0+=8;
					}
					*/

					//Ajuste de color segun modo overlay
					//falta modo 11 overlay_mode_type
					screen_get_pixel_ink_paper_common_prism( (overlay_mode_type>>2)&3, attribute0,&ink0,&paper0);

					/*
                                	ink2=attribute2 &7;
	                                paper2=(attribute2>>3) &7;
        	                        bright2=(attribute2)&64;
                	                if (bright2) {
                        	                ink2+=8;
                                	        paper2+=8;
	                                }
					*/

					screen_get_pixel_ink_paper_common_prism( (overlay_mode_type)&3, attribute2,&ink2,&paper2);

				break;

				case 10: //Gigablend mode
                                        ink0=attribute0 &7;
                                        paper0=(attribute0>>3) &7;
                                        bright0=(attribute0)&64;
					flash0=(attribute0)&128; \
                                        if (bright0) {
                                                ink0+=8;
                                                paper0+=8;
                                        }

                                        ink2=attribute2 &7;
                                        paper2=(attribute2>>3) &7;
                                        bright2=(attribute2)&64;
					flash2=(attribute2)&128; \
                                        if (bright2) {
                                                ink2+=8;
                                                paper2+=8;
                                        }


					if (estado_parpadeo.v) {
			                        if (flash0) {
                                		        aux=paper0;
		                                        paper0=ink0;
                		                        ink0=aux;
						}

						if (flash2) {
                                		        aux=paper2;
		                                        paper2=ink2;
                		                        ink2=aux;
                                		}
		                        }

				break;



				case 11: //Brainebow mode
					//Ajuste a 4 bits
					blue=puntero_buffer_atributos0[x]/16;
					red=puntero_buffer_atributos2[x]/16;
					green=puntero_buffer_atributos3[x]/16;

					//Ajuste a indice de color RGB
					red=red<<8;
					green=green<<4;
				break;

				case 13: //1101 - Jowett mode

				break;

				default:
					screen_get_pixel_ink_paper_common_prism(screendatadecoding,attribute0,&ink0,&paper0);
				break;
			}

			for (bit=0;bit<8;bit++) {


				switch (screendatadecoding) {
					case 3:  //0011 - Chunk-o-blend
						//Leer los dos bits superiores 
						color=( byte_leido0 & 128 ? 2 : 0 ) ;
						//Leer siguiente bit
						byte_leido0=byte_leido0<<1;
						bit++;
						color |=( byte_leido0 & 128 ? 1 : 0 ) ;


						int mezcla=screen_prism_get_blend_color(paper0,ink0);
						mezcla=mezcla+PRISM_INDEX_FIRST_COLOR;



						//Tenemos los dos colores. Obtener componentes RGB
						//REDa,GREENa,BLUEa
						//REDb,GREENb,BLUEb
						
						//de momento paleta por defecto prism_palette_zero[color]
						//Obtenemos color 12 bits
						/*
						rgb_a=prism_palette_zero[paper0];
						red_a=(rgb_a>>8)&15;
						green_a=(rgb_a>>4)&15;
						blue_a=(rgb_a)&15;

						rgb_b=prism_palette_zero[ink0];
						red_b=(rgb_b>>8)&15;
						green_b=(rgb_b>>4)&15;
						blue_b=(rgb_b)&15;

						//Y montamos colores finales
						//RED= REDa(3) & REDb(2) & REDb(1) & REDa(0)
						//GREEN = REDa(3) & REDb(2) & REDb(1) & REDa(0)
						//BLUE = REDa(3) & REDb(2) & REDb(1) & REDa(0)
						red_final=(red_a &8) | (red_b &4) | (red_b & 2) | (red_a & 1);
						green_final=(green_a &8) | (green_b &4) | (green_b & 2) | (green_a & 1);
						blue_final=(blue_a &8) | (blue_b &4) | (blue_b & 2) | (blue_a & 1);


						//temp mi metodo de gigablend
						red_final=(red_a+red_b)/2;
						green_final=(green_a+green_b)/2;
						blue_final=(blue_a+blue_b)/2;

						//Montamos color final rgb 12 bits
						int mezcla=(red_final<<8) | (green_final<<4) | (blue_final);*/
						

						switch (color) {
							case 0:
								color=paper0;
								PRISM_ADJUST_COLOUR_PALETTE
								//printf ("cpaper ");
							break;

							case 1:
								color=mezcla;
								//printf ("cink16 ");
							break;

							case 2:
								color=mezcla;
								//printf ("cink24 ");
							break;

							case 3:
								color=ink0;
								PRISM_ADJUST_COLOUR_PALETTE
								//printf ("cink8 ");
							break;

						}

						
						store_value_rainbow(puntero_buf_rainbow,color);
						if (ancho_256) store_value_rainbow(puntero_buf_rainbow,color);
					break;

					case 5: //0101 - 256 Colour mode 2 (256 colour overlay mode - 3 colours in each 8x8 square)
						if ( (byte_leido0&128)==0) {
							if ( (byte_leido2&128)==0) color=get_prism_ula2_border_colour();
							else color=attribute2;
						}

						else color=attribute0;

	                                        PRISM_ADJUST_COLOUR_PALETTE
					break;

					case 6: //4 Plane planar mode (16 colour "clashless" mode)
						color=(byte_leido3/128)*8+(byte_leido2/128)*4+(byte_leido1/128)*2+(byte_leido0/128)*1;
						PRISM_ADJUST_COLOUR_PALETTE
					break;

					case 7: //3 Plane planar mode (Cheveron Mode)
						color=8+(byte_leido3/128)*4+(byte_leido2/128)*2+(byte_leido0/128)*1;
						PRISM_ADJUST_COLOUR_PALETTE
					break;

					case 8: //Overlay mode 1
					case 9: //Overlay mode 2
						color2= ( byte_leido2 & 128 ? ink2 : paper2 ) ;
						if (!(color2==colortransparente1 || color2==colortransparente2)) {
							//Overlay de vram2 sobre vram0
							ink0=ink2;
							paper0=paper2;
							byte_leido0=byte_leido2;
						}


						color= ( byte_leido0 & 128 ? ink0 : paper0 ) ;

						PRISM_ADJUST_COLOUR_PALETTE

					break;

					case 10: //Gigablend
						color= ( byte_leido0 & 128 ? ink0 : paper0 ) ; //de vram0
						color2=( byte_leido2 & 128 ? ink2 : paper2 ) ; //de vram2

						color=screen_prism_get_blend_color(color,color2);

						//Tenemos los dos colores. Obtener componentes RGB
						//REDa,GREENa,BLUEa
						//REDb,GREENb,BLUEb


						//de momento paleta por defecto prism_palette_zero[color]
						//Obtenemos color 12 bits
						/*rgb_a=prism_palette_zero[color];
						red_a=(rgb_a>>8)&15;
						green_a=(rgb_a>>4)&15;
						blue_a=(rgb_a)&15;

						rgb_b=prism_palette_zero[color2];
						red_b=(rgb_b>>8)&15;
						green_b=(rgb_b>>4)&15;
						blue_b=(rgb_b)&15;

						//Y montamos colores finales
						//RED= REDa(3) & REDb(2) & REDb(1) & REDa(0)
						//GREEN = REDa(3) & REDb(2) & REDb(1) & REDa(0)
						//BLUE = REDa(3) & REDb(2) & REDb(1) & REDa(0)
						red_final=(red_a &8) | (red_b &4) | (red_b & 2) | (red_a & 1);
						green_final=(green_a &8) | (green_b &4) | (green_b & 2) | (green_a & 1);
						blue_final=(blue_a &8) | (blue_b &4) | (blue_b & 2) | (blue_a & 1);


						//temp mi metodo de gigablend
						red_final=(red_a+red_b)/2;
						green_final=(green_a+green_b)/2;
						blue_final=(blue_a+blue_b)/2;

						//Montamos color final rgb 12 bits
						color=(red_final<<8) | (green_final<<4) | (blue_final);*/

						color=color+PRISM_INDEX_FIRST_COLOR;

					break;

					case 11: //Brainebow
						color=0;
						if (byte_leido0&128) color=color|blue;
						if (byte_leido2&128) color=color|red;
						if (byte_leido3&128) color=color|green;
						color=color+PRISM_INDEX_FIRST_COLOR;
					break;

					case 12: //4 Plane planar mode (256 colour "clashless" mode)
                                                color=(byte_leido3/128)*8+(byte_leido2/128)*4+(byte_leido1/128)*2+(byte_leido0/128)*1;
                                                PRISM_ADJUST_COLOUR_PALETTE
						//Se sumara el offset correspondiente de paleta en PRISM_ADJUST_COLOUR_PALETTE
                                        break;


					case 13: //1101 - Jowett mode
						//TODO. Deberia ser un modo con buffer para poder cambiar el color del border mas de una vez en la linea
						color=get_prism_ula2_border_colour();
						PRISM_ADJUST_COLOUR_PALETTE
					break;


					case 14:  //1110 - Chunk-o-vision 128x192 mode I
						//Leer los dos bits superiores para saber el offset sobre la paleta
						color=( byte_leido0 & 128 ? 2 : 0 ) ;
						//Leer siguiente bit
						byte_leido0=byte_leido0<<1;
						bit++;
						color |=( byte_leido0 & 128 ? 1 : 0 ) ;

						switch (color) {
							case 0:
								color=paper0;
								//printf ("cpaper ");
							break;

							case 1:
								color=ink0+16;
								//printf ("cink16 ");
							break;

							case 2:
								color=ink0+24;
								//printf ("cink24 ");
							break;

							case 3:
								color=ink0+8;
								//printf ("cink8 ");
							break;

						}

						PRISM_ADJUST_COLOUR_PALETTE
						store_value_rainbow(puntero_buf_rainbow,color);
						if (ancho_256) store_value_rainbow(puntero_buf_rainbow,color);
					break;


					case 15:  //1110 - Chunk-o-vision 128x192 mode II
						//Leer los dos bits superiores 
						color=( byte_leido0 & 128 ? 2 : 0 ) ;
						//Leer siguiente bit
						byte_leido0=byte_leido0<<1;
						bit++;
						color |=( byte_leido0 & 128 ? 1 : 0 ) ;

						switch (color) {
							case 0:
								color=paper0;
								//printf ("cpaper ");
							break;

							case 1:
								color=prism_ae3b_registers[1];
								
							break;

							case 2:
								color=prism_ae3b_registers[2];
								
							break;

							case 3:
								color=ink0;
								//printf ("cink8 ");
							break;

						}

						PRISM_ADJUST_COLOUR_PALETTE
						store_value_rainbow(puntero_buf_rainbow,color);
						if (ancho_256) store_value_rainbow(puntero_buf_rainbow,color);
					break;

					default:
						color= ( byte_leido0 & 128 ? ink0 : paper0 ) ;
						PRISM_ADJUST_COLOUR_PALETTE
					break;
				}

				//Si modo timex 512x192
				if (timex_video_mode==4 || timex_video_mode==6) {
					/*z80_int direccion_en_512=direccion & (65535-31);
					z80_byte x_en_512=x/2;
					z80_byte mitad=3-bucle_ancho;
					direccion_en_512 = direccion_en_512 | ((x_en_512)*mitad);*/

					z80_byte tin6, pap6;


	                                tin6=get_timex_ink_mode6_color();


        	                        //Obtenemos color
        	                        pap6=get_timex_paper_mode6_color();

                                //Y con brillo
					tin6 +=8;
					pap6 +=8;

					if (ulaplus_presente.v && ulaplus_enabled.v) {
						tin6 +=16;
	                                        pap6 +=16;
        	                                tin6=ulaplus_palette_table[tin6]+ULAPLUS_INDEX_FIRST_COLOR;
                	                        pap6=ulaplus_palette_table[pap6]+ULAPLUS_INDEX_FIRST_COLOR;
					}


					//temp
					//ink0=0;
					//paper0=7;
					color= ( byte_leido0 & 128 ? tin6 : pap6 ) ;
					PRISM_ADJUST_COLOUR_PALETTE
				}



				if (!linear_mode) {
					//2 pixeles por cada uno
					store_value_rainbow(puntero_buf_rainbow,color);
					if (ancho_256) store_value_rainbow(puntero_buf_rainbow,color);
				}

				else {
					//printf ("store color\n");
					if (linear_mode_mode==2 || linear_mode_mode==3) {
						color=screen0[direccion];
						if (linear_mode_mode==3) direccion++;
						if (linear_mode_mode==2) {
							//Dado que es 128 de ancho, avanzar direccion cada 2 pixeles escritos
							if (x&1) direccion++;
						}
        	                                PRISM_ADJUST_COLOUR_PALETTE
						store_value_rainbow(puntero_buf_rainbow,color);
						store_value_rainbow(puntero_buf_rainbow,color);
					}


				}


				//printf ("x: %d t_scanline_draw: %d color: %d\n",x,t_scanline_draw,color);

				byte_leido0=byte_leido0<<1;
				byte_leido1=byte_leido1<<1;
				byte_leido2=byte_leido2<<1;
				byte_leido3=byte_leido3<<1;
			}


			if (!linear_mode) {

				if (timex_video_mode==4 || timex_video_mode==6) {
					if (x&1) direccion++;
				}

				else direccion++;
			}


			else {
				if (linear_mode_mode==2 || linear_mode_mode==3) {
					//if (bit==7) direccion++;
				}
			}


		}


		//Repetir el bucle en ancho si no es 256
		switch (screentype) {
			case 1: //512x192, 8x8 Attributes (2 normal Spectrum screens next to each other: either VRAM0+VRAM1 or VRAM2+VRAM3 - NOT interpolated monochrome like Timex)
				//Cambio puntero screen0, atributos
				screen0=screen1;
				screen2=screen3;
				puntero_buffer_atributos0=puntero_buffer_atributos1;
				puntero_buffer_atributos2=puntero_buffer_atributos3;
			break;


                        case 3: //0011 - 512x384, 8x8 Attributes 4 normal spectrum screens: VRAM0 to the left of VRAM1 on top of VRAM2 to the left of VRAM3
				screen0=screen1;
				puntero_buffer_atributos0=puntero_buffer_atributos1;
                        break;

		}

		//volver al principio de la linea en la otra vram, para bucle en ancho si no es 256
		if (timex_video_mode==4 || timex_video_mode==6) {
		}

		else {

	                direccion -=32;

		}

		//Cierre bucle de ancho si no es 256
		}





	}
}






//Guardar en buffer rainbow la linea actual. Para Spectrum. solo display
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
void screen_store_scanline_rainbow_solo_display(void)
{
	//Si maquina tsconf. Dado que tiene border tamaño variable, hacerlo desde aqui tal cual
	if (MACHINE_IS_TSCONF) {
		screen_store_scanline_rainbow_solo_display_tsconf();
		return;
	}


	//Funcion aparte para tbblue
	if (MACHINE_IS_TBBLUE) {
		screen_store_scanline_rainbow_solo_display_tbblue();
		return;
	}



	//si linea no coincide con entrelazado, volvemos
	if (if_store_scanline_interlace(t_scanline_draw)==0) return;

  if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {


	//Hacer scroll de spritechip
	spritechip_do_scroll();

	if (ulaplus_presente.v && ulaplus_enabled.v && ulaplus_extended_mode>=1) {
		//Aqui se entra tanto si es spectrum como si es prism
		screen_store_scanline_rainbow_solo_display_ulaplus_lineal();
		spritechip_do_overlay();
		return;
	}

	//Si modos spectra
	if (spectra_enabled.v) {
		screen_store_scanline_rainbow_solo_display_spectra();
		spritechip_do_overlay();
		return;
	}

	//Si maquina prism
	if (MACHINE_IS_PRISM) {
		screen_store_scanline_rainbow_solo_display_prism();
		//TODO: no hace sprite chip
		return;
	}

	//Si modo 16C pentagon
	if (MACHINE_IS_PENTAGON && screen_mode_16c_is_enabled()) {
		screen_store_scanline_rainbow_solo_display_16c();
		return;
	}

    //Si modo zxuno prism
    if (MACHINE_IS_ZXUNO && zxuno_is_prism_mode_enabled() ) {
        zxuno_prism_screen_store_scanline_rainbow();
        return;
    }

        //printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

        //linea que se debe leer
        int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

        //la copiamos a buffer rainbow
        z80_int *puntero_buf_rainbow;
        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        int y;

        y=t_scanline_draw-screen_invisible_borde_superior;
        if (border_enabled.v==0) y=y-screen_borde_superior;

        puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];

        puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;


        int x,bit;
        z80_int direccion;
	//z80_int dir_atributo;
        z80_byte byte_leido;


        int color=0;
        int fila;

        z80_byte attribute,bright,flash;
	//z80_int ink,paper,aux;
	unsigned int ink,paper,aux;


        z80_byte *screen=get_base_mem_pantalla();

        direccion=screen_addr_table[(scanline_copia<<5)];

				


        fila=scanline_copia/8;
        //dir_atributo=6144+(fila*32);


	z80_byte *puntero_buffer_atributos;


	//Si modo timex 512x192 pero se hace modo escalado
	//Si es modo timex 512x192, llamar a otra funcion
        if (timex_si_modo_512_y_zoom_par() ) {
                //Si zoom x par
                                        if (timex_mode_512192_real.v) {
                                                return;
                                        }
        }


	//temporal modo 6 timex 512x192 pero hacemos 256x192
	z80_byte temp_prueba_modo6[SCANLINEBUFFER_ONE_ARRAY_LENGTH];
	z80_byte col6;
	z80_byte tin6, pap6;

	z80_byte timex_video_mode=timex_port_ff&7;
	z80_byte timexhires_resultante;
	z80_int timexhires_origen;

	z80_bit si_timex_hires={0};

	//Por defecto
	puntero_buffer_atributos=scanline_buffer;


	if (timex_video_emulation.v) {
		//Modos de video Timex
		/*
000 - Video data at address 16384 and 8x8 color attributes at address 22528 (like on ordinary Spectrum);

001 - Video data at address 24576 and 8x8 color attributes at address 30720;

010 - Multicolor mode: video data at address 16384 and 8x1 color attributes at address 24576;

110 - Extended resolution: without color attributes, even columns of video data are taken from address 16384, and odd columns of video data are taken from address 24576
		*/
		switch (timex_video_mode) {

			case 4:
			case 6:
				//512x192 monocromo. aunque hacemos 256x192
				//y color siempre fijo
				/*
bits D3-D5: Selection of ink and paper color in extended screen resolution mode (000=black/white, 001=blue/yellow, 010=red/cyan, 011=magenta/green, 100=green/magenta, 101=cyan/red, 110=yellow/blue, 111=white/black); these bits are ignored when D2=0

				black, blue, red, magenta, green, cyan, yellow, white
				*/

				//Si D2==0, these bits are ignored when D2=0?? Modo 4 que es??

				//col6=(timex_port_ff>>3)&7;
				tin6=get_timex_ink_mode6_color();


				//Obtenemos color
				//tin6=col6;
				pap6=get_timex_paper_mode6_color();

				//Y con brillo
				col6=((pap6*8)+tin6)+64;

				//Nos inventamos un array de colores, con mismo color siempre, correspondiente a lo que dice el registro timex
				//Saltamos de dos en dos
				//De manera similar al buffer scanlines_buffer, hay pixel, atributo, pixel, atributo, etc
				//por eso solo llenamos la parte que afecta al atributo

				puntero_buffer_atributos=temp_prueba_modo6;
				int i;
				for (i=1;i<SCANLINEBUFFER_ONE_ARRAY_LENGTH;i+=2) {
					temp_prueba_modo6[i]=col6;
				}
				si_timex_hires.v=1;
			break;


		}
	}


	int posicion_array_pixeles_atributos=0;
        for (x=0;x<32;x++) {


                        //byte_leido=screen[direccion];
                        byte_leido=puntero_buffer_atributos[posicion_array_pixeles_atributos++];

			//Timex. Reducir 512x192 a 256x192.
			//Obtenemos los dos bytes implicados, metemos en variable de 16 bits,
			//Y vamos comprimiendo cada 2 pixeles. De cada 2 pixeles, si los dos son 0, metemos 0. Si alguno o los dos son 1, metemos 1
			//Esto es muy lento

			if (si_timex_hires.v) {

					//comprimir bytes
					timexhires_resultante=0;
					//timexhires_origen=byte_leido*256+screen[direccion+8192];
					timexhires_origen=screen[direccion]*256+screen[direccion+8192];

					//comprimir pixeles
					int i;
					for (i=0;i<8;i++) {
						timexhires_resultante=timexhires_resultante<<1;
						if ( (timexhires_origen&(32768+16384))   ) timexhires_resultante |=1;
						timexhires_origen=timexhires_origen<<2;
					}

					byte_leido=timexhires_resultante;

			}



                        attribute=puntero_buffer_atributos[posicion_array_pixeles_atributos++];


                                //snow effect
				//TODO: ver exactamente el comportamiento real del snow effect
                                if (snow_effect_enabled.v==1) {

					if (si_toca_snow_effect(x) ) {

							//Byte leido es byte anterior
							//byte leido es (DIR & FF00) | reg_r;
							z80_int puntero_snow;
							//puntero_snow=direccion & 0xFF00;
							//z80_byte calculado_reg_r=(reg_r&127) | (reg_r_bit7&128);

							//Probar a quitar solo 7 bits
							puntero_snow=direccion & 0xFF80;
							z80_byte calculado_reg_r=(reg_r&127);

							//restamos a registro r para situarnos en la primera columna
							calculado_reg_r -= 74;

							//y sumamos a reg_r columna*2 (esto simula incremento de registro R)
							//1 linea=224 estados. 1 instruccion=3 estados=1 incremento de R
							//en una linea, 74 instrucciones simples=74 incrementos de R
							//32 columnas * 2 = 64 = casi 74
					 		calculado_reg_r +=x*2;


							puntero_snow |=calculado_reg_r;

							byte_leido=screen[puntero_snow];



							//atributo debe ser el de misma columna que el byte que lee la ula
                        				//attribute=puntero_buffer_atributos[puntero_snow&31];


							//Atributo lo sacamos generando con misma formula
							//A)
                        				//z80_int puntero_attr=6144+fila*32;
							//attribute=screen[puntero_attr];

							//B)
							z80_int puntero_attr=6144+fila*32;
							puntero_attr &=0xFF80;
							puntero_attr |=calculado_reg_r;
							attribute=screen[puntero_attr];

        	                        }
				}


			GET_PIXEL_COLOR

			int cambiada_tinta,cambiada_paper;

			cambiada_tinta=cambiada_paper=0;

                        for (bit=0;bit<8;bit++) {

				//ula color bug para Inves. Solo cuando color siguiente es negro. Da igual color anterior ?¿
				if (MACHINE_IS_INVES && inves_ula_bright_error.v) {  //Paper 8 indica paper 0 con brillo
					//printf ("bright\n");
					if (bit==0) {
						if (paper==8 || ink==8) {
							//printf ("cambiado\n");
							//Si ha cambiado de 0 a 1
							z80_byte brillo_temp;
							if (x!=0) brillo_temp=puntero_buffer_atributos[posicion_array_pixeles_atributos-3]&64;
							else brillo_temp=0;
							if (brillo_temp==0) {
								if (paper==8) {
									paper=1; //temp
									cambiada_paper=1;
								}
								if (ink==8) {
									ink=1;
									cambiada_tinta=1;
								}
							}

							else cambiada_tinta=cambiada_paper=0;
						}

					}

					else {
						if (cambiada_paper==1) {
							cambiada_paper=0;
							paper=8;
							//printf ("cambiado2\n");
						}
						if (cambiada_tinta==1) {
							cambiada_tinta=0;
							ink=8;
							//printf ("cambiado3\n");
						}
					}

					//if (x<3) printf ("bright: %d paper: %d x: %d bit: %d cpap: %d ctin: %d\n",bright,paper,x,bit,
					//	cambiada_paper,cambiada_tinta);

				}


				color= ( byte_leido & 128 ? ink : paper ) ;

				


				
                                store_value_rainbow(puntero_buf_rainbow,color);
				

                                byte_leido=byte_leido<<1;


																//tbblue_layer2_offset++;
				
                        }
			direccion++;
                	//dir_atributo++;


        	}




	}
	spritechip_do_overlay();


}

//Guardar en buffer rainbow linea actual de borde superior o inferior
void screen_store_scanline_rainbow_border_tbblue_supinf(void)
{

	int scanline_copia=t_scanline_draw-screen_invisible_borde_superior;

	z80_int *puntero_buf_rainbow;

	int x=screen_total_borde_izquierdo;

	//printf ("%d\n",scanline_copia*get_total_ancho_rainbow());
	//esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
	//int offset=
	puntero_buf_rainbow=&rainbow_buffer[scanline_copia*get_total_ancho_rainbow()*2+x*2]; //*2 porque es doble de alto

	//Empezamos desde x en zona display, o sea, justo despues del ancho del borde izquierdo
	screen_store_scanline_rainbow_border_comun(puntero_buf_rainbow,x );


}





void screen_store_scanline_rainbow_solo_border_tbblue(void)
{


	int ancho_pantalla=256;


        //zona de border superior o inferior. Dibujar desde posicion x donde acaba el ancho izquierdo de borde, linea horizontal
	//hasta derecha del todo, y luego trozo de ancho izquiero del borde de linea siguiente
        if ( (t_scanline_draw>=screen_invisible_borde_superior && t_scanline_draw<screen_indice_inicio_pant) ||
             (t_scanline_draw>=screen_indice_fin_pant && t_scanline_draw<screen_indice_fin_pant+screen_total_borde_inferior)
	   ) {

		screen_store_scanline_rainbow_border_tbblue_supinf();
		//printf ("borde superior o inferior\n");
        }



        //zona de border + pantalla + border
	//Dibujar desde borde derecho hasta borde izquierdo de linea siguiente
        else if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {

	        //linea que se debe leer
	        //int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

        	z80_int *puntero_buf_rainbow;
	        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        	int y;

	        y=t_scanline_draw-screen_invisible_borde_superior;

		//nos situamos en borde derecho
		//y se dibujara desde el borde derecho hasta el izquierdo de la siguiente linea
		int offset_derecha=(screen_total_borde_izquierdo+ancho_pantalla)*2; //*2 porque es doble de ancho
		puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow()*2+offset_derecha ]; //*2 porque es doble de alto


	        screen_store_scanline_rainbow_border_comun(puntero_buf_rainbow,screen_total_borde_izquierdo+ancho_pantalla);

        }




	//primera linea de border. Realmente empieza una linea atras y acaba la primera linea de borde
	//con el borde izquierdo de la primera linea visible
	//Esto solo sirve para dibujar primera linea de border (de ancho izquierdo solamente)

	else if ( t_scanline_draw==screen_invisible_borde_superior-1 ) {
		z80_int *puntero_buf_rainbow;

		puntero_buf_rainbow=&rainbow_buffer[0];

		int xinicial=screen_total_borde_izquierdo+ancho_pantalla+screen_total_borde_derecho+screen_invisible_borde_derecho;
		//printf ("primera linea de borde: %d empezamos en xinicial: %d \n",t_scanline_draw,xinicial);


		//si se ha cambiado el border en la zona superior invisible de border, actualizarlo
		//Esto sucede en aquaplane
		//Quiza habria que buscar en el array de border, en toda la zona inicial que corresponde a la parte no visible de border,
		//el ultimo valor enviado. Pero esto seria muy lento. Basta con leer ultimo valor enviado (esto es aproximado,
		//el valor que tenemos en out_254 es el del final de esta linea actual, que no tiene por que coincidir con el valor de la linea anterior,
		//aunque seria un caso muy raro)

		//screen_border_last_color=out_254 & 7;
		screen_border_last_color=get_border_colour_from_out();


		screen_store_scanline_rainbow_border_comun(puntero_buf_rainbow,xinicial);

	}




}



/*

  Guardar en buffer rainbow la linea actual-solo border. Para Spectrum
  Cada linea en t-estados empieza en la posicion X donde se dibuja la pantalla de "pixels" propiamente, o sea, dentro del border:


    Aqui
      |
      |
      |
      v

  ------------------------
  ------------------------
  ----                ----
  ----                ----
  ----                ----
  ----                ----
  ----                ----
  ----                ----
  ----                ----
  ------------------------
  ------------------------

Por tanto, si estamos en zona inferior o superior del borde, se dibuja partiendo de la posicion X de ancho de border, se llena toda la linea
horizontal, y se dibuja la parte izquierda de borde (de ancho X) de la linea siguiente.
Si estamos en zona central (borde+display+borde) se dibuja desde borde derecho hasta el izquierdo de la siguiente linea
Hay que tener en cuenta que la rutina de dibujar de borde, screen_store_scanline_rainbow_border_comun, recorre siempre todo el array del border
de la linea actual del border (incluso en la zona central) pero no dibuja en pantalla hasta que se alcanza la posicion que se le dice como parametro


*/
void screen_store_scanline_rainbow_solo_border(void)
{

	if (border_enabled.v==0) return;


	int ancho_pantalla=256;

	if (MACHINE_IS_PRISM) ancho_pantalla=PRISM_DISPLAY_WIDTH;

	if (MACHINE_IS_TBBLUE) {
		screen_store_scanline_rainbow_solo_border_tbblue();
		return;
	}



	if (MACHINE_IS_TSCONF) {
		//se gestiona todo desde el solo_display
		return;
	}


        //si linea no coincide con entrelazado, volvemos
        //if (if_store_scanline_interlace(t_scanline_draw)==0) return;


        //zona de border superior o inferior. Dibujar desde posicion x donde acaba el ancho izquierdo de borde, linea horizontal
	//hasta derecha del todo, y luego trozo de ancho izquiero del borde de linea siguiente
        if ( (t_scanline_draw>=screen_invisible_borde_superior && t_scanline_draw<screen_indice_inicio_pant) ||
             (t_scanline_draw>=screen_indice_fin_pant && t_scanline_draw<screen_indice_fin_pant+screen_total_borde_inferior)
	   ) {

		screen_store_scanline_rainbow_border_comun_supinf();
        }

        //zona de border + pantalla + border
	//Dibujar desde borde derecho hasta borde izquierdo de linea siguiente
        else if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {

	        //linea que se debe leer
	        //int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

        	z80_int *puntero_buf_rainbow;
	        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        	int y;

	        y=t_scanline_draw-screen_invisible_borde_superior;

		//nos situamos en borde derecho
		//y se dibujara desde el borde derecho hasta el izquierdo de la siguiente linea
		puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow()+screen_total_borde_izquierdo+ancho_pantalla ];


	        screen_store_scanline_rainbow_border_comun(puntero_buf_rainbow,screen_total_borde_izquierdo+ancho_pantalla);

        }

	//primera linea de border. Realmente empieza una linea atras y acaba la primera linea de borde
	//con el borde izquierdo de la primera linea visible
	//Esto solo sirve para dibujar primera linea de border (de ancho izquierdo solamente)

	else if ( t_scanline_draw==screen_invisible_borde_superior-1 ) {
		z80_int *puntero_buf_rainbow;

		puntero_buf_rainbow=&rainbow_buffer[0];

		int xinicial=screen_total_borde_izquierdo+ancho_pantalla+screen_total_borde_derecho+screen_invisible_borde_derecho;
		//printf ("primera linea de borde: %d empezamos en xinicial: %d \n",t_scanline_draw,xinicial);


		//si se ha cambiado el border en la zona superior invisible de border, actualizarlo
		//Esto sucede en aquaplane
		//Quiza habria que buscar en el array de border, en toda la zona inicial que corresponde a la parte no visible de border,
		//el ultimo valor enviado. Pero esto seria muy lento. Basta con leer ultimo valor enviado (esto es aproximado,
		//el valor que tenemos en out_254 es el del final de esta linea actual, que no tiene por que coincidir con el valor de la linea anterior,
		//aunque seria un caso muy raro)

		//screen_border_last_color=out_254 & 7;
		screen_border_last_color=get_border_colour_from_out();
		screen_border_last_color_prism=get_prism_ula2_border_colour();

		screen_incremento_border_si_ulaplus();
		screen_incremento_border_si_spectra();


		//printf ("ultimo valor a border invisible superior: %d t_scanline_draw=%d\n",screen_border_last_color,t_scanline_draw);

		screen_store_scanline_rainbow_border_comun(puntero_buf_rainbow,xinicial);

	}




}




void siguiente_frame_pantalla(void)
{

	frames_total++;
        if (frames_total==50) {

                              //contador framedrop
                                if (framedrop_total!=0) {
					//si no hay frameskip forzado
                                        if (!frameskip && ultimo_fps!=50) debug_printf(VERBOSE_INFO,"FPS: %d",ultimo_fps);
                                }


				ultimo_fps=50-framedrop_total;

                                framedrop_total=0;
                                frames_total=0;
        }


	//Gestion de autoactivado de realvideo cuando hay cambios de border
	if (MACHINE_IS_SPECTRUM && rainbow_enabled.v==0 && autodetect_rainbow.v) {
		//Si el numero de cambios de border en un frame pasa el minimo
		//printf ("numero de cambios: %d\n",detect_rainbow_border_changes_in_frame);
		if (detect_rainbow_border_changes_in_frame>=DETECT_RAINBOW_BORDER_MAX_IN_FRAMES) {

			//printf ("total frames: %d\n",detect_rainbow_border_total_frames);
			//Conteo de frames, incrementar
			if (detect_rainbow_border_total_frames==DETECT_RAINBOW_BORDER_TOTAL_FRAMES) {
				//Activar realvideo
				debug_printf (VERBOSE_INFO,"Enabling realvideo due to repeated border changes. Minimum border changes in frame: %d. Total frames repeated: %d",DETECT_RAINBOW_BORDER_MAX_IN_FRAMES,detect_rainbow_border_total_frames);
				enable_rainbow();
				//Reseteamos contadores, por si se desactiva y vuelve a activar posteriormente
				detect_rainbow_border_changes_in_frame=0;
				detect_rainbow_border_total_frames=0;
			}
			else detect_rainbow_border_total_frames++;
		}

		else {
			//Si no, resetear total frames
			//printf ("no pasa el minimo de cambios. resetear\n");
			detect_rainbow_border_total_frames=0;
		}


		//Nuevo frame. Numero de cambios en frame a 0
		detect_rainbow_border_changes_in_frame=0;
	}

}

char last_message_helper_aofile_vofile_file_format[1024]="";
char last_message_helper_aofile_vofile_util[1024]="";
char last_message_helper_aofile_vofile_bytes_minute_audio[1024]="";
char last_message_helper_aofile_vofile_bytes_minute_video[1024]="";
z80_byte *vofile_buffer;

void print_helper_aofile_vofile(void)
{

         int ancho,alto;


        ancho=screen_get_emulated_display_width_no_zoom_border_en();
        alto=screen_get_emulated_display_height_no_zoom_border_en();


#define AOFILE_TYPE_RAW 0
#define AOFILE_TYPE_WAV 1
//extern int aofile_type;

        char buffer_texto_video[500];
        char buffer_texto_audio[500];


				int audio_bytes_per_second,video_bytes_per_second; //bytes por segundo

				audio_bytes_per_second=FRECUENCIA_SONIDO*2; //*2 porque es stereo en wav
				video_bytes_per_second=ancho*3*alto*(50/vofile_fps);//*3 porque son 24 bits

        sprintf(buffer_texto_video,"-demuxer rawvideo -rawvideo fps=%d:w=%d:h=%d:format=bgr24",50/vofile_fps,ancho,alto);

	if (aofile_type==AOFILE_TYPE_RAW) {
		audio_bytes_per_second /=2; //porque es mono en rwa
        	sprintf(buffer_texto_audio,"-audiofile %s -audio-demuxer rawaudio -rawaudio channels=1:rate=%d:samplesize=1",aofilename,FRECUENCIA_SONIDO);
	}

	if (aofile_type==AOFILE_TYPE_WAV) {
		sprintf(buffer_texto_audio,"-audiofile %s",aofilename);
	}




	if (aofile_inserted.v==1 && vofile_inserted.v==0) {

		if (aofile_type==AOFILE_TYPE_RAW) {
			sprintf(last_message_helper_aofile_vofile_util,"You can convert it with: sox  -t .raw -r %d -b 8 -e unsigned -c 1 %s outputfile.wav",FRECUENCIA_SONIDO,aofilename);
		}

		//Si es wav, texto de conversion vacio
		else {
			last_message_helper_aofile_vofile_util[0]=0;
		}


	}

	if (aofile_inserted.v==0 && vofile_inserted.v==1) {
		sprintf(last_message_helper_aofile_vofile_util,"You can play it with : mplayer %s %s",buffer_texto_video,vofilename);
	}

	if (aofile_inserted.v==1 && vofile_inserted.v==1) {
		sprintf(last_message_helper_aofile_vofile_util,"You can play both audio & video files with : mplayer %s %s %s",buffer_texto_video,buffer_texto_audio,vofilename);
	}

	sprintf(last_message_helper_aofile_vofile_bytes_minute_audio,"Every minute of file uses %d KB",audio_bytes_per_second*60/1024);
	sprintf(last_message_helper_aofile_vofile_bytes_minute_video,"Every minute of file uses %d KB",video_bytes_per_second*60/1024);

	debug_printf(VERBOSE_INFO,"%s",last_message_helper_aofile_vofile_util);

}

void init_vofile(void)
{

                //debug_printf (VERBOSE_INFO,"Initializing Audio Output File");

                ptr_vofile=fopen(vofilename,"wb");
                //printf ("ptr_vofile: %p\n",ptr_vofile);

                if (!ptr_vofile)
                {
                        debug_printf(VERBOSE_ERR,"Unable to create vofile %s",vofilename);
                        vofilename=NULL;
                        vofile_inserted.v=0;
                        return;
                }

         int ancho,alto,tamanyo;
        //ancho=LEFT_BORDER_NO_ZOOM+ANCHO_PANTALLA+RIGHT_BORDER_NO_ZOOM;
        //alto=TOP_BORDER_NO_ZOOM+ALTO_PANTALLA+BOTTOM_BORDER_NO_ZOOM;

	//Z88: 640x192 = 122880
	//Spectrum: 352x296 = 104192
	//Prism: 640x480 = 307200
	//QL: 512x512
	//TSConf: 720x576
	

	//Si se esta con vofile activo y se cambia de maquina, el buffer tiene que ser suficientemente grande para que quepa,
	//y este buffer se asigna solo al principio. Sino petaria con segmentation fault seguramente

        //ancho=screen_get_emulated_display_width_no_zoom();
        //alto=screen_get_emulated_display_height_no_zoom();
	ancho=720;
	alto=576;
	//esto es mucho mas de lo que necesita


        tamanyo=ancho*alto;

        vofile_buffer=malloc(tamanyo*3);
        if (vofile_buffer==NULL) {
                cpu_panic("Error allocating video output buffer");
        }

	//Hay que activar realvideo dado que el video se genera en base a esto
	enable_rainbow();


	vofile_frame_actual=0;

        vofile_inserted.v=1;

        ancho=screen_get_emulated_display_width_no_zoom_border_en();
        alto=screen_get_emulated_display_height_no_zoom_border_en();


        sprintf(last_message_helper_aofile_vofile_file_format,"Writing video output file, format raw, %d FPS, %d X %d, bgr24",50/vofile_fps,ancho,alto);
        debug_printf(VERBOSE_INFO,"%s",last_message_helper_aofile_vofile_file_format);
	print_helper_aofile_vofile();
}




unsigned char buffer_rgb[3];


/*

Paleta antigua para vofile no usada ya. Usamos misma paleta activa de color

// Paletas VGA en 6 bit, Paleta archivo raw 8 bit, multiplicar por 4
#define BRI0      (42+5)*4
#define BRI1      (16)*4

// Tabla para los colores reales

unsigned char tabla_colores[]={
//      RED       GREEN     BLUE                 G R B
    	0,	  0,        0,			// 0 En SP: 0 0 0 Black
    	0,        0,        BRI0,	      	// 1        0 0 1 Blue
	BRI0,     0,	    0,         		// 2        0 1 0 Red
	BRI0,	  0,	    BRI0,      		// 3        0 1 1 Magenta
	0,	  BRI0,	    0,			// 4        1 0 0 Green
	0,	  BRI0,	    BRI0,		// 5        1 0 1 Cyan
	BRI0,	  BRI0,	    0,			// 6        1 1 0 Yellow
	BRI0,	  BRI0,	    BRI0,		// 7        1 1 1 White


//With brightness

	0,	  0,        0,			// 0        0 0 0 Black
    	0,        0,        BRI0+BRI1, 		// 1        0 0 1 Blue
	BRI0+BRI1,0,	    0,         		// 2        0 1 0 Red
	BRI0+BRI1,0,	    BRI0+BRI1, 		// 3        0 1 1 Magenta
	0,	  BRI0+BRI1,0,			// 4        1 0 0 Green
	0,	  BRI0+BRI1,BRI0+BRI1,		// 5        1 0 1 Cyan
	BRI0+BRI1,BRI0+BRI1,0,			// 6        1 1 0 Yellow
	BRI0+BRI1,BRI0+BRI1,BRI0+BRI1,		// 7        1 1 1 White

};

*/

void convertir_paleta(z80_int valor)
{

	unsigned char valor_r,valor_g,valor_b;

	//colores originales
	//int color=spectrum_colortable_original[valor];

	//colores de tabla activa
	int color=spectrum_colortable[valor];


	valor_r=(color & 0xFF0000) >> 16;
	valor_g=(color & 0x00FF00) >> 8;
	valor_b= color & 0x0000FF;


	buffer_rgb[0]=valor_b;
	buffer_rgb[1]=valor_g;
	buffer_rgb[2]=valor_r;

}

//En principio esto vale para cualquier maquina, no solo spectrum
void convertir_color_spectrum_paleta_to_rgb(z80_int valor,int *r,int *g,int *b)
{

	//unsigned char valor_r,valor_g,valor_b;

	//colores de tabla activa
	int color=spectrum_colortable[valor];


	*r=(color & 0xFF0000) >> 16;
	*g=(color & 0x00FF00) >> 8;
	*b= color & 0x0000FF;



}

/*
	convertir_paleta(valor);
   fwrite( &buffer_rgb, 1, 3, fichero_out);
*/

int vofile_add_oldstyle_watermark_aux_indice_xy(int x,int y)
{
	         int ancho;
        ancho=screen_get_emulated_display_width_no_zoom_border_en();


	return ancho*y*3+x*3;
}

//Antigua "Z" como marca de agua. La nueva es mucho mas bonita :)
void vofile_add_oldstyle_watermark(void)
{


	int x,y;
	int pos;

	//offset respecto a la esquina superior
	int offset_x=8;
	int offset_y=8;

	//Tamanyo de la Z
	int z_size=24;

	//Parte de arriba de la Z. 2 lineas de grueso
	for (x=0;x<z_size;x++) {
		convertir_paleta(x&15);
		pos=vofile_add_oldstyle_watermark_aux_indice_xy(x+offset_x,0+offset_y);
		vofile_buffer[pos++]=buffer_rgb[0];
		vofile_buffer[pos++]=buffer_rgb[1];
		vofile_buffer[pos++]=buffer_rgb[2];

                pos=vofile_add_oldstyle_watermark_aux_indice_xy(x+offset_x,1+offset_y);
                vofile_buffer[pos++]=buffer_rgb[0];
                vofile_buffer[pos++]=buffer_rgb[1];
                vofile_buffer[pos++]=buffer_rgb[2];

	}

	//Diagonal de la z. 2 pixeles de ancho
        for (y=1,x=z_size-2;y<z_size-1;x--,y++) {
        	convertir_paleta(x&15);
                int pos=vofile_add_oldstyle_watermark_aux_indice_xy(x+offset_x,y+offset_y);
                vofile_buffer[pos++]=buffer_rgb[0];
                vofile_buffer[pos++]=buffer_rgb[1];
                vofile_buffer[pos++]=buffer_rgb[2];

                vofile_buffer[pos++]=buffer_rgb[0];
                vofile_buffer[pos++]=buffer_rgb[1];
                vofile_buffer[pos++]=buffer_rgb[2];

        }

        //Parte de abajo de la Z. 2 lineas de grueso
        for (x=0;x<z_size;x++) {
        	convertir_paleta(x&15);
                pos=vofile_add_oldstyle_watermark_aux_indice_xy(x+offset_x,z_size-2+offset_y);
                vofile_buffer[pos++]=buffer_rgb[0];
                vofile_buffer[pos++]=buffer_rgb[1];
                vofile_buffer[pos++]=buffer_rgb[2];

                pos=vofile_add_oldstyle_watermark_aux_indice_xy(x+offset_x,z_size-1+offset_y);
                vofile_buffer[pos++]=buffer_rgb[0];
                vofile_buffer[pos++]=buffer_rgb[1];
                vofile_buffer[pos++]=buffer_rgb[2];


        }



}

void vofile_send_frame(z80_int *buffer)
{

        if (vofile_inserted.v==0) return;

	vofile_frame_actual++;
	//printf ("actual %d tope %d\n",vofile_frame_actual,vofile_fps);
	if (vofile_frame_actual!=vofile_fps) return;
	vofile_frame_actual=0;

        int escritos;

         int ancho,alto,tamanyo;

        ancho=screen_get_emulated_display_width_no_zoom_border_en();
        alto=screen_get_emulated_display_height_no_zoom_border_en();


        tamanyo=ancho*alto;

	int origen_buffer=0;
	z80_byte *destino_buffer;
	destino_buffer=vofile_buffer;
	//z80_byte byte_leido;
	z80_int color_leido;


	//printf ("tamanyo: %d vofile_buffer: %p\n",tamanyo,vofile_buffer);

	for (;origen_buffer<tamanyo;origen_buffer++) {
		//byte_leido=*buffer++;
		//convertir_paleta(byte_leido);
		color_leido=*buffer++;
		convertir_paleta(color_leido);
	 	*destino_buffer++=buffer_rgb[0];
	 	*destino_buffer++=buffer_rgb[1];
	 	*destino_buffer++=buffer_rgb[2];
	}


        //printf ("buffer: %p ptr_vofile: %p\n",buffer,ptr_vofile);

        //escritos=fwrite(buffer, 1, tamanyo, ptr_vofile);


	//agregamos marca de agua
	//fuerzo watermark siempre. Aunque el usuario vaya al menu y lo deshabilite, se volverá a activar
	screen_watermark_enabled.v=1;
	//vofile_add_oldstyle_watermark();

	escritos=fwrite(vofile_buffer,1,tamanyo*3, ptr_vofile);
        if (escritos!=tamanyo*3) {

                        debug_printf(VERBOSE_ERR,"Unable to write to vofile %s",vofilename);
                        vofilename=NULL;
                        vofile_inserted.v=0;

                //debug_printf(VERBOSE_ERR,"Bytes escritos: %d\n",escritos);
                //cpu_panic("Error writing vofile\n");
        }


}

void close_vofile(void)
{

        if (vofile_inserted.v==0) {
                debug_printf (VERBOSE_INFO,"Closing vofile. But already closed");
                return;
        }

        vofile_inserted.v=0;


	debug_printf (VERBOSE_INFO,"Closing vofile type RAW");
	fclose(ptr_vofile);
}

//Resetea algunos parametros de drivers de video, ya seteados a 0 al arrancar
//se llama aqui al cambiar el driver de video en caliente
void screen_reset_scr_driver_params(void)
{
	scr_tiene_colores=0;

	screen_stdout_driver=0;

	screen_simpletext_driver=0;

	screen_refresh_menu=0;

	scr_messages_debug=NULL;

	esc_key_message="ESC";
}

void screen_set_colour_normal(int index, int colour)
{

	spectrum_colortable_normal[index]=colour;


#ifdef COMPILE_AA
        //para aalib, tiene su propia paleta que hay que actualizar
        if (!strcmp(scr_new_driver_name,"aa")) {
                scraa_setpalette(index,(colour >> 16) & 0xFF, (colour >> 8) & 0xFF, (colour) & 0xFF );
        }
#endif


}

//Para cada valor 0, 1, 2, 3 devuelve
//#define SPECTRA_COL_ZERO 0
//#define SPECTRA_COL_LOW 89
//#define SPECTRA_COL_MEDIUM 185
//#define SPECTRA_COL_HIGH 255

z80_byte spectra_return_intensity(int c)
{

	z80_byte v;

	switch (c) {
		case 0:
			v=SPECTRA_COL_ZERO;
		break;

		case 1:
			v=SPECTRA_COL_LOW;
		break;

		case 2:
			v=SPECTRA_COL_MEDIUM;
		break;

                case 3:
			v=SPECTRA_COL_HIGH;
		break;

		default:
			debug_printf (VERBOSE_DEBUG,"Invalid spectra colour component for: %d",c);
			v=0;
		break;

	}

	return v;
}

void screen_init_colour_table_siguiente(void)
{

                	int i,j,r,g,b,r2,g2,b2,valorgris;


		//Primero construir la tabla de colores de spectra. Que siempre tiene valores fijos
//Tabla con los colores extra del Spectra.
//Valores para intensidades de color:
/*
C1 C0  Voltage  Output
0  0   0        0%           -> 0
0  1   0.24     34.8%    -> 255/100*34.8 = 88.74 -> 89
1  0   0.50     72.5%    -> 184.875 -> 185
1  1   0.69     100%     -> 255
*/
//#define SPECTRA_COL_ZERO 0
//#define SPECTRA_COL_LOW 89
//#define SPECTRA_COL_MEDIUM 185
//#define SPECTRA_COL_HIGH 255


		/*
32 16  8   4   2   1
G  G   R   R   B   B

		Ejemplos: 0: negro, 1: azul intensidad baja, 2:azul medio, 3: azul alto, 4: rojo bajo, 5: rojo bajo+azul bajo,
		8: rojo medio, 12: rojo alto, 16: verde bajo, 32: verde medio, 48: verde alto
		*/

		int spectra_color;

		//Bits obtenidos de cada numero de color
		int spectra_blue,spectra_red,spectra_green;

		//Intensidades asociadas a cada componente de color
		int spectra_int_blue,spectra_int_red,spectra_int_green;

		for (spectra_color=0;spectra_color<64;spectra_color++) {


			spectra_blue=spectra_color&3;
			spectra_red=(spectra_color>>2)&3;
			spectra_green=(spectra_color>>4)&3;

			spectra_int_blue=spectra_return_intensity(spectra_blue);
			spectra_int_red=spectra_return_intensity(spectra_red);
			spectra_int_green=spectra_return_intensity(spectra_green);


			spectra_colortable_original[spectra_color]=(spectra_int_red<<16)|(spectra_int_green<<8)|spectra_int_blue;

			debug_printf (VERBOSE_PARANOID,"Initializing Spectra Colour. Index: %d Value: 0x%06X",spectra_color,spectra_colortable_original[spectra_color]);

		}


/*
Viejo metodo para tener paleta con componentes de grises / r, g o b
Con el nuevo metodo en cambio se crea la paleta normal, y luego se modifica para los componentes grises / r,g o b e inverso

		if (screen_gray_mode!=0) {




#define GRAY_MODE_CONST 30
#define GRAY_MODE_CONST_BRILLO 20


	        for (i=0;i<16;i++) {
				valorgris=(i&7)*GRAY_MODE_CONST;

				if (i>=8) valorgris +=GRAY_MODE_CONST_BRILLO;

				VALOR_GRIS_A_R_G_B

				screen_set_colour_normal(i,(r<<16)|(g<<8)|b);

	        }

			//El color 8 es negro, con brillo 1. Pero negro igual
			screen_set_colour_normal(8,0);

			//spectrum_colortable_normal=spectrum_colortable_grises;


			//tramas de grises para Z88
			//en ese caso me baso en los mismos colores del spectrum con gris, es decir:
			//pixel on, color negro
			//pixel gris, color blanco sin brillo
			//pixel off o pantalla off: color blanco con brillo

			screen_set_colour_normal(Z88_PXCOLON,spectrum_colortable_normal[0]);
			screen_set_colour_normal(Z88_PXCOLGREY,spectrum_colortable_normal[7]);
			screen_set_colour_normal(Z88_PXCOLOFF,spectrum_colortable_normal[15]);
			screen_set_colour_normal(Z88_PXCOLSCROFF,spectrum_colortable_normal[15]);




			//trama de grises para ulaplus
			//z80_byte color;
			int color32;
                        for (i=0;i<256;i++) {
		                int r,g,b;
		                int valorgris=i;
                		VALOR_GRIS_A_R_G_B

		                color32=(r<<16)|(g<<8)|b;

                                screen_set_colour_normal(ULAPLUS_INDEX_FIRST_COLOR+i, color32);

                        }

			//trama de grises para Spectra
                        for (i=0;i<64;i++) {
                                valorgris=i*4;

                                VALOR_GRIS_A_R_G_B

                                screen_set_colour_normal(SPECTRA_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);

                        }

			//trama de grises para CPC
                        for (i=0;i<32;i++) {
				valorgris=i*8;
				VALOR_GRIS_A_R_G_B
				screen_set_colour_normal(CPC_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);
                        }

			//printf ("TODO grey for Prism\n");

			//trama de grises para Prism
			for (i=0;i<4096;i++) {
                                valorgris=i/16;
                                VALOR_GRIS_A_R_G_B
                                screen_set_colour_normal(PRISM_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);
                        }

			//trama de grises para Sam
			for (i=0;i<128;i++) {
                                valorgris=i*2;
                                VALOR_GRIS_A_R_G_B
                                screen_set_colour_normal(SAM_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);
                        }


												//trama de grises para rgb9
												//z80_byte color;

									for (i=0;i<512;i++) {
									int r,g,b;
									int valorgris=i;
									valorgris=i/2;
									VALOR_GRIS_A_R_G_B

									color32=(r<<16)|(g<<8)|b;

										screen_set_colour_normal(RGB9_INDEX_FIRST_COLOR+i, color32);

									}
			//trama de grises para tsconf
			for (i=0;i<32768;i++) {
                                valorgris=i/128;
                                VALOR_GRIS_A_R_G_B
                                screen_set_colour_normal(TSCONF_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);
                        }

			//Colores HEATMAP
			for (i=0;i<256;i++) {
				valorgris=i;
				VALOR_GRIS_A_R_G_B
				screen_set_colour_normal(HEATMAP_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);
			}

				//Colores Solarized. No los pasamos a grises estos
				for (i=0;i<SOLARIZED_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(SOLARIZED_INDEX_FIRST_COLOR+i,solarized_colortable_original[i]);
				}			


				//Colores VDP9918
				for (i=0;i<VDP_9918_TOTAL_PALETTE_COLOURS;i++) {
					valorgris=i*16;
					VALOR_GRIS_A_R_G_B
					screen_set_colour_normal(VDP_9918_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);					
				}

				//Colores QL
				for (i=0;i<QL_TOTAL_PALETTE_COLOURS;i++) {
					valorgris=i*32;
					VALOR_GRIS_A_R_G_B
					screen_set_colour_normal(QL_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);					
				}                

				//Colores Turbovision. No los pasamos a grises estos
				for (i=0;i<TURBOVISION_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(TURBOVISION_INDEX_FIRST_COLOR+i,turbovision_colortable_original[i]);
				}	

				//Colores SMS
				for (i=0;i<SMS_TOTAL_PALETTE_COLOURS;i++) {
					valorgris=i*4;
                    //printf("i %d valorgris %d\n",i,valorgris);
					VALOR_GRIS_A_R_G_B
					screen_set_colour_normal(SMS_INDEX_FIRST_COLOR+i,(r<<16)|(g<<8)|b);					
				}   

				//Colores BeOS. No los pasamos a grises estos
				for (i=0;i<BEOS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(BEOS_INDEX_FIRST_COLOR+i,beos_colortable_original[i]);
				}	                             

				//Colores Retromac. No los pasamos a grises estos
				for (i=0;i<RETROMAC_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(RETROMAC_INDEX_FIRST_COLOR+i,retromac_colortable_original[i]);
                }

				//Colores AmigaOS. No los pasamos a grises estos
				for (i=0;i<AMIGAOS_TOTAL_PALETTE_COLOURS;i++) {
                    screen_set_colour_normal(AMIGAOS_INDEX_FIRST_COLOR+i,amigaos_colortable_original[i]);
				}	 

				//Colores AtariTOS. No los pasamos a grises estos
				for (i=0;i<ATARITOS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(ATARITOS_INDEX_FIRST_COLOR+i,ataritos_colortable_original[i]);
				}	                                                    


		}
        */

		//else {

			//Crear primero paleta de colores normales. Posteriormente si conviene se hacen grises / r, g o b e inverso
			//spectrum_colortable_normal=(int *)spectrum_colortable_original;
			//int i;
			int color32;
			int *paleta;
			paleta=screen_return_spectrum_palette();
			for (i=0;i<16;i++) {
				color32=paleta[i];
				//debug_printf(VERBOSE_DEBUG,"Initializing Standard Spectrum Color. Index: %i  Value: %06XH",i,spectrum_colortable_original[i]);
				//screen_set_colour_normal(i,spectrum_colortable_original[i]);
				debug_printf(VERBOSE_PARANOID,"Initializing Standard Spectrum Color. Index: %i  Value: %06XH",i,color32);
				screen_set_colour_normal(i,color32);
			}


			//colores para Z88
			screen_set_colour_normal(Z88_PXCOLON,z88_colortable_original[0]);
			screen_set_colour_normal(Z88_PXCOLGREY,z88_colortable_original[1]);
			screen_set_colour_normal(Z88_PXCOLOFF,z88_colortable_original[2]);
			screen_set_colour_normal(Z88_PXCOLSCROFF,z88_colortable_original[3]);


			//Colores reales de spectrum 16/48/+
			/*
			for (i=0;i<16;i++) {
                                debug_printf(VERBOSE_DEBUG,"Initializing Standard Spectrum 16/48/+ Real Color. Index: %i  Value: %06XH",i,spectrum_colortable_1648_real[i]);
                                screen_set_colour_normal(SPECCY_1648_REAL_PALETTE_FIRST_COLOR+i,spectrum_colortable_1648_real[i]);
                        }
			*/

			//colores ulaplus
			//ulaplus_rgb_table
			//ULAPLUS_INDEX_FIRST_COLOR

			for (i=0;i<256;i++) {
				color32=ulaplus_rgb_table[i];
				debug_printf(VERBOSE_PARANOID,"Initializing ULAPlus Color. Index: %i  Value: %06XH",i,color32);
				screen_set_colour_normal(ULAPLUS_INDEX_FIRST_COLOR+i, color32);
			}

			//Colores spectra
                        for (i=0;i<64;i++) {
                        	debug_printf(VERBOSE_PARANOID,"Initializing Spectra Color. Index: %i  Value: %06XH",i,spectra_colortable_original[i]);
                                screen_set_colour_normal(SPECTRA_INDEX_FIRST_COLOR+i,spectra_colortable_original[i]);
                        }

			//Colores CPC
			for (i=0;i<32;i++) {
                                color32=cpc_rgb_table[i];
                                debug_printf(VERBOSE_PARANOID,"Initializing CPC Color. Index: %i  Value: %06XH",i,color32);
                                screen_set_colour_normal(CPC_INDEX_FIRST_COLOR+i, color32);
                        }

			//Colores Prism
			//Tenemos tabla de conversion de valor de 4 bits a 8 bits
			z80_byte prism_4_to_8[16]={
				/* 0000     0001     0010     0011     0100     0101     0110     0111     1000     1001     1010     1011     1100     1101     1110     1111 */
				/* 00000000 00000011 00001100 00001111 00110000 00110011 00111100 00111111 11000000 11000011 11001100 11001111 11110000 11110011 11111100 11111111 */
				//0,          3,       12,      15,      48,      51,      60,      63,      192,     195,     204,     207,     240,     243,     252,     255
				0*16+0,     1*16+1,  2*16+2,  3*16+3,  4*16+4,  5*16+5,  6*16+6,  7*16+7,  8*16+8,  9*16+9,  10*16+10,11*16+11,12*16+12,13*16+13,14*16+14,15*16+15


				};
			for (i=0;i<4096;i++) {


                                b= i & 0xF;
                                g=(i >> 4 ) & 0xF;
                                r=(i >> 8 ) & 0xF;

				debug_printf (VERBOSE_PARANOID,"Prism color: %d. 12 bit: r: %d g: %d b: %d",i,r,g,b);

				r=prism_4_to_8[r];
				g=prism_4_to_8[g];
				b=prism_4_to_8[b];


                                color32=(r<<16)|(g<<8)|b;

				debug_printf (VERBOSE_PARANOID,"32 bit: r: %d g: %d b: %d",
					r,g,b);

                                screen_set_colour_normal(PRISM_INDEX_FIRST_COLOR+i, color32);
                        }

			//Colores sam coupe
            for (i=0;i<128;i++) {
				/*

Bit 0 BLU0 least significant bit of blue.
Bit 1 RED0 least significant bit of red.
Bit 2 GRN0 least significant bit of green.

Bit 3 BRIGHT half  bit intensity on all colours.
Bit 4 BLU1 most  significant bit of blue.
Bit 5 RED1 most  significant bit of red.
Bit 6 GRN1 most  significant bit of green.
				*/
				int brillo=127*  ((i&8)>>3);

				b=42*(  (i     &1) | ((i>>3)&2))   +brillo;
				r=42*(  ((i>>1)&1) | ((i>>4)&2))   +brillo;
				g=42*(  ((i>>2)&1) | ((i>>5)&2))   +brillo;



                                debug_printf (VERBOSE_PARANOID,"Sam color: %d. 6 bit: r: %d g: %d b: %d",i,r,g,b);


                                color32=(r<<16)|(g<<8)|b;

                                debug_printf (VERBOSE_PARANOID,"32 bit: r: %d g: %d b: %d",
                                        r,g,b);

                                screen_set_colour_normal(SAM_INDEX_FIRST_COLOR+i, color32);
                }


				//Colores RGB9
				for (i=0;i<512;i++) {
					debug_printf (VERBOSE_PARANOID,"RGB9 color: %02XH 32 bit: %06XH",i,get_rgb9_color(i));
					screen_set_colour_normal(RGB9_INDEX_FIRST_COLOR+i,get_rgb9_color(i));
				}


				//Tenemos tabla de conversion de valor de 5 bits a 8 bits. Temporal aproximado
				/*z80_byte tsconf_5_to_8[32];

				for (i=0;i<32;i++) {
					tsconf_5_to_8[i]=i*8;
				}*/

				//Colores tsconf
				for (i=0;i<32768;i++) {

					b= i & 0x1F;
					g=(i >> 5 ) & 0x1F;
					r=(i >> 10 ) & 0x1F;

					debug_printf (VERBOSE_PARANOID,"tsconf color: %d. 15 bit: r: %d g: %d b: %d",i,r,g,b);

//r=tsconf_5_to_8[r];
//g=tsconf_5_to_8[g];
//b=tsconf_5_to_8[b];

					//tsconf_rgb_5_to_8
					r=tsconf_rgb_5_to_8(r);
					g=tsconf_rgb_5_to_8(g);
					b=tsconf_rgb_5_to_8(b);


					color32=(r<<16)|(g<<8)|b;

					debug_printf (VERBOSE_PARANOID,"32 bit: r: %d g: %d b: %d",r,g,b);

					screen_set_colour_normal(TSCONF_INDEX_FIRST_COLOR+i, color32);

				}


				//Colores HEATMAP
				for (i=0;i<256;i++) {
					int colorheat=i<<16;
					debug_printf (VERBOSE_PARANOID,"Heatmap color: %02XH 32 bit: %06XH",i,colorheat);
					screen_set_colour_normal(HEATMAP_INDEX_FIRST_COLOR+i,colorheat);
				}

				//Colores Solarized
				for (i=0;i<SOLARIZED_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(SOLARIZED_INDEX_FIRST_COLOR+i,solarized_colortable_original[i]);
				}

					

				//Colores VDP 9918
				for (i=0;i<VDP_9918_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(VDP_9918_INDEX_FIRST_COLOR+i,vdp9918_colortable_original[i]);
				}

				//Colores QL
				for (i=0;i<QL_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(QL_INDEX_FIRST_COLOR+i,ql_colortable_original[i]);
				}         

				//Colores Turbovision
				for (i=0;i<TURBOVISION_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(TURBOVISION_INDEX_FIRST_COLOR+i,turbovision_colortable_original[i]);
				}      

				//Colores SMS
				for (i=0;i<SMS_TOTAL_PALETTE_COLOURS;i++) {
					//Es formato %00BBGGRR
					r=i & 3;
					g=(i >> 2) & 3;
					b=(i >> 4) & 3;

                    //Pasar de 3 hasta 255
                    r *=85;
                    g *=85;
                    b *=85;


					color32=(r<<16)|(g<<8)|b;

					//debug_printf (VERBOSE_PARANOID,"32 bit: r: %d g: %d b: %d",r,g,b);

                    //printf ("sms %d 32 bit: r: %d g: %d b: %d\n",i,r,g,b);

					screen_set_colour_normal(SMS_INDEX_FIRST_COLOR+i, color32);

				} 

				//Colores BeOS
				for (i=0;i<BEOS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(BEOS_INDEX_FIRST_COLOR+i,beos_colortable_original[i]);
				}       

				//Colores Retromac
				for (i=0;i<RETROMAC_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(RETROMAC_INDEX_FIRST_COLOR+i,retromac_colortable_original[i]);
				}

				//Colores AmigaOS
				for (i=0;i<AMIGAOS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(AMIGAOS_INDEX_FIRST_COLOR+i,amigaos_colortable_original[i]);
				} 

				//Colores AtariTOS
				for (i=0;i<ATARITOS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(ATARITOS_INDEX_FIRST_COLOR+i,ataritos_colortable_original[i]);
				}   

				//Colores OS/2
				for (i=0;i<OSDOS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(OSDOS_INDEX_FIRST_COLOR+i,osdos_colortable_original[i]);
				}

				//Colores ZEsarUX Plus
				for (i=0;i<ZESARUX_PLUS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(ZESARUX_PLUS_INDEX_FIRST_COLOR+i,zesarux_plus_colortable_original[i]);
				}

				//Colores RiscOs
				for (i=0;i<RISCOS_TOTAL_PALETTE_COLOURS;i++) {
					screen_set_colour_normal(RISCOS_INDEX_FIRST_COLOR+i,riscos_colortable_original[i]);
				}                


		//}

		//Colores para interlaced scanlines. Linea impar mas oscura
		//copiamos del color generado del spectrum al color scanline (indice + 16)
                for (i=0;i<16;i++) {
                                b=spectrum_colortable_normal[i] & 0xFF;
                                g=(spectrum_colortable_normal[i] >> 8 ) & 0xFF;
                                r=(spectrum_colortable_normal[i] >> 16 ) & 0xFF;

                                //Valores mas oscuros para scanlines
                                r=r/2;
                                g=g/2;
                                b=b/2;

                                //printf ("%x %x %x\n",r,g,b);

                                screen_set_colour_normal(i+16,(r<<16)|(g<<8)|b);
                }


		//colores para gigascreen
		int index_giga=32;
		for (i=0;i<16;i++) {
	                for (j=0;j<16;j++) {

                                b=spectrum_colortable_normal[i] & 0xFF;
                                g=(spectrum_colortable_normal[i] >> 8 ) & 0xFF;
                                r=(spectrum_colortable_normal[i] >> 16 ) & 0xFF;

                                b2=spectrum_colortable_normal[j] & 0xFF;
                                g2=(spectrum_colortable_normal[j] >> 8 ) & 0xFF;
                                r2=(spectrum_colortable_normal[j] >> 16 ) & 0xFF;

				r=get_gigascreen_rgb_value(r,r2);
				g=get_gigascreen_rgb_value(g,g2);
				b=get_gigascreen_rgb_value(b,b2);

                                //printf ("index: %d %x %x %x\n",index_giga,r,g,b);
				//printf ("%06X\n",(r<<16)|(g<<8)|b);

                                screen_set_colour_normal(index_giga++,(r<<16)|(g<<8)|b);

        	        }

		}



		//Si video inverso o grises
        //Modo de grises activo en screen_gray_mode
        //0: colores normales
        //1: componente Blue
        //2: componente Green
        //4: componente Red
        //Se pueden sumar para diferentes valores

		if (inverse_video.v==1 || screen_gray_mode!=0) {
        	        for (i=0;i<EMULATOR_TOTAL_PALETTE_COLOURS;i++) {
                	        b=spectrum_colortable_normal[i] & 0xFF;
                        	g=(spectrum_colortable_normal[i] >> 8 ) & 0xFF;
	                        r=(spectrum_colortable_normal[i] >> 16 ) & 0xFF;

                            //Tipos de grises
                            if (screen_gray_mode!=0) {
                                valorgris=rgb_to_grey(r,g,b);
                                VALOR_GRIS_A_R_G_B
                            }

                            //Inverso
                            if (inverse_video.v==1) {
        	                    r=r^255;
                	            g=g^255;
                        	    b=b^255;
                            }

				screen_set_colour_normal(i,(r<<16)|(g<<8)|b);
			}
		}


                //inicializar tabla de colores oscuro
		/*
                for (i=0;i<EMULATOR_TOTAL_PALETTE_COLOURS;i++) {
                        b=spectrum_colortable_normal[i] & 0xFF;
                        g=(spectrum_colortable_normal[i] >> 8 ) & 0xFF;
                        r=(spectrum_colortable_normal[i] >> 16 ) & 0xFF;

                        r=r/2;
                        g=g/2;
                        b=b/2;

                        spectrum_colortable_oscuro[i]=(r<<16)|(g<<8)|b;
                }
		*/

		//Establecemos tabla actual
                spectrum_colortable=spectrum_colortable_normal;


#ifdef COMPILE_CURSES
		//Si driver curses, su paleta es diferente
		if (!strcmp(scr_new_driver_name,"curses")) scrcurses_inicializa_colores();
#endif


//#ifdef COMPILE_AA
//		//Si driver aa, reinicializar paleta
//		if (!strcmp(scr_new_driver_name,"aa")) scraa_inicializa_colores();
//#endif



}

void screen_init_colour_table(void)
{
	/*
	Primero generamos tabla de colores grises. Esa tabla se usa cuando se abre el menu con multitask off, donde los colores se ponen en gris
	TODO: hacerlos en gris y tambien oscuros
	Para ello, se genera tabla con forzado a gris, lo copio a tabla de grises, y luego se genera colores normales
	*/

	debug_printf (VERBOSE_INFO,"Creating colour tables for %d colours",EMULATOR_TOTAL_PALETTE_COLOURS);
	if (EMULATOR_TOTAL_PALETTE_COLOURS>65535) cpu_panic("More than 65536 colours to allocate. This is fatal!");

	int antes_screen_gray_mode=screen_gray_mode;
	screen_gray_mode=7;
	screen_init_colour_table_siguiente();



	//Copiamos de tabla normal, que seran grises, a tabla grises y ademas oscuros
	/*
	int i,r,g,b;
	for (i=0;i<EMULATOR_TOTAL_PALETTE_COLOURS;i++) {

                        b=spectrum_colortable_normal[i] & 0xFF;
                        g=(spectrum_colortable_normal[i] >> 8 ) & 0xFF;
                        r=(spectrum_colortable_normal[i] >> 16 ) & 0xFF;

                        r=r/2;
                        g=g/2;
                        b=b/2;

                        spectrum_colortable_new_blanco_y_negro[i]=(r<<16)|(g<<8)|b;

	}
	*/


	screen_gray_mode=antes_screen_gray_mode;
	screen_init_colour_table_siguiente();

}


void scr_fadeout(void)
{
    int color,i,r,g,b,j;

	//Si quickexit, no hacer fadeout
	if (quickexit.v) return;

	int color_curses=0;


	//en stdout, simpletext y null no hacerlo
	if (!strcmp(scr_new_driver_name,"stdout"))  return;
	if (!strcmp(scr_new_driver_name,"simpletext"))  return;
	if (!strcmp(scr_new_driver_name,"null"))  return;

	//en aalib va muy lento y no se por que. no hacerlo
	if (!strcmp(scr_new_driver_name,"aa"))  return;

	//Si tiene gigascreen, quitar, sino hace un efecto extranyo
	disable_gigascreen();
	disable_interlace();

	debug_printf (VERBOSE_INFO,"Refreshing screen before fade out");
	scr_refresca_pantalla();


	debug_printf (VERBOSE_INFO,"Making fade out");


#ifdef COMPILE_XWINDOWS
	//parece que con shm activo, no hace fadeout en xwindows
	shm_used=0;
#endif

#define MAX_FADE 256
#define INC_FADE 10
#define TOTAL_SECONDS 1

#define SLEEPTIME (1000000*TOTAL_SECONDS/(MAX_FADE/INC_FADE))

	int incremento_color_curses=64/INC_FADE;

	if (incremento_color_curses<1) incremento_color_curses=1;


	int spectrum_colortable_fade[EMULATOR_TOTAL_PALETTE_COLOURS];

	for (j=0;j<MAX_FADE;j+=INC_FADE) {
		spectrum_colortable=spectrum_colortable_fade;

		//printf ("%p\n",spectrum_colortable);

        for (i=0;i<EMULATOR_TOTAL_PALETTE_COLOURS;i++) {

			color=spectrum_colortable_normal[i];
            b=color & 0xFF;
            g=(color >> 8 ) & 0xFF;
            r=(color >> 16 ) & 0xFF;

			r=r-j;
			g=g-j;
			b=b-j;

			if (r<0) r=0;
			if (g<0) g=0;
			if (b<0) b=0;



			color=(r<<16)|(g<<8)|b;

			//spectrum_colortable_normal[i]=color;
			spectrum_colortable_fade[i]=color;


			//en el caso de aalib usa una paleta diferente
#ifdef COMPILE_AA
            //Si driver aa, reinicializar paleta
            if (!strcmp(scr_new_driver_name,"aa")) scraa_inicializa_colores();
            //scraa_setpalette (i, r,g,b);
#endif




        }

#ifdef COMPILE_CURSES
        if (!strcmp(scr_new_driver_name,"curses")) {
            int bucle_curses;
            for (bucle_curses=0;bucle_curses<incremento_color_curses;bucle_curses++) {
                scrcurses_fade_color(color_curses++);
            }
        }
#endif

		clear_putpixel_cache();
		modificado_border.v=1;
		screen_z88_draw_lower_screen();

        menu_clear_footer();
        redraw_footer();

		menu_draw_ext_desktop();
		all_interlace_scr_refresca_pantalla();

		usleep(SLEEPTIME);

	}
}


int screen_force_refresh=0;

//Retorna 1 si se tiene que refrescar pantalla. Aplica frameskip y autoframeskip
int screen_if_refresh(void)
{

	//Forzado puntual de refresco de pantalla, para que no haga frameskip. Usado por ejemplo en debug cpu
	//Cuando se hace una vez, luego se resetea a 0
	if (screen_force_refresh) {
		screen_force_refresh=0;
		return 1;
	}

	//Si esta en top speed, solo 1 frame
	if (timer_condicion_top_speed() ) {
		if (MACHINE_IS_Z88) {
			if ((top_speed_real_frames%200)<197) return 0;
		}

		else {
			if ((top_speed_real_frames%50)!=0) return 0;
		}
		return 1;
	}


	if ( (framescreen_saltar==0 || autoframeskip.v==0) && frameskip_counter==0) {
		return 1;
	}


	return 0;
}

void screen_before_menu_overlay_timer(void)
{
    //Calcular tiempo usado en refrescar pantalla
	timer_stats_current_time(&core_render_menu_overlay_antes);    
}
 
void screen_after_menu_overlay_timer(void)
{
            
    //Calcular tiempo usado en refrescar pantalla
    core_render_menu_overlay_difftime=timer_stats_diference_time(&core_render_menu_overlay_antes,&core_render_menu_overlay_despues);

    //media de tiempo. 
    core_render_menu_overlay_media=(core_render_menu_overlay_media+core_render_menu_overlay_difftime)/2;
}

void screen_render_menu_overlay_if_active(void)
{
	if (menu_overlay_activo) {
        screen_before_menu_overlay_timer();
        menu_overlay_function();
        screen_after_menu_overlay_timer();
    }
}

void cpu_loop_refresca_pantalla_return(void)
{
        //Calcular tiempo usado en refrescar pantalla
        core_cpu_timer_refresca_pantalla_difftime=timer_stats_diference_time(&core_cpu_timer_refresca_pantalla_antes,&core_cpu_timer_refresca_pantalla_despues);

        //media de tiempo
        core_cpu_timer_refresca_pantalla_media=(core_cpu_timer_refresca_pantalla_media+core_cpu_timer_refresca_pantalla_difftime)/2;


		TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_cpu_timer_refresca_pantalla);
}

void cpu_loop_refresca_pantalla(void)
{

	//Calcular tiempo usado en refrescar pantalla
	timer_stats_current_time(&core_cpu_timer_refresca_pantalla_antes);

	TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_cpu_timer_refresca_pantalla);

	//Para calcular el tiempo entre frames. Idealmente 20 ms
	//Diferencia tiempo
	core_cpu_timer_each_frame_difftime=timer_stats_diference_time(&core_cpu_timer_each_frame_antes,&core_cpu_timer_each_frame_despues);
	//Media de tiempo
	core_cpu_timer_each_frame_media=(core_cpu_timer_each_frame_media+core_cpu_timer_each_frame_difftime)/2;
	//Siguiente tiempo
	timer_stats_current_time(&core_cpu_timer_each_frame_antes);

    //printf("cpu_loop_refresca_pantalla on scanline %d\n",t_scanline_draw);
    next_frame_skip_render_scanlines=0;

    stats_frames_total++;


	if (rainbow_enabled.v) screen_add_watermark_rainbow();
	else screen_add_watermark_no_rainbow();

	//Si esta en top speed, solo 1 frame
	if (timer_condicion_top_speed() ) {

		if (screen_if_refresh() ) {
			//printf ("top_speed_real_frames:%d\n",top_speed_real_frames);
	
			top_speed_real_frames=1;
			debug_printf (VERBOSE_DEBUG,"Refreshing screen on top speed");
			scr_refresca_pantalla();
			frameskip_counter=frameskip;
            stats_frames_total_drawn++;

		}
		cpu_loop_refresca_pantalla_return();
		return;
	}

    //printf ("saltar: %d counter %d\n",framescreen_saltar,frameskip_counter);

    //Si se ha llegado antes a final de frame, y no hay frameskip manual
    //Si no hay autoframeskip, el primer parentesis siempre se cumple
    //Si hay autoframeskip, y se ha tardado mucho en llegar a final de frame (framescreen_saltar>0) , el primer parentesis no se cumple y por tanto no se redibuja pantalla

    //if ( (framescreen_saltar==0 || autoframeskip.v==0) && frameskip_counter==0) {
    if (screen_if_refresh() ) {
        //printf ("refrescando\n");
        scr_refresca_pantalla();
        frameskip_counter=frameskip;
        stats_frames_total_drawn++;
    }


    //Si no se ha llegado a final de frame antes, o hay frameskip manual
    else {
        next_frame_skip_render_scanlines=1;
        stats_frames_total_dropped++;
        //printf ("-no refrescando. frameskip_counter %d\n",frameskip_counter);
        if (frameskip_counter) frameskip_counter--;
        else {
            //printf("Framedrop %d\n",framedrop_total);
            //printf("frameskip_counter %d\n",frameskip_counter);
            //printf("framescreen_saltar %d\n",framescreen_saltar);
            debug_printf(VERBOSE_DEBUG,"Framedrop %d",framedrop_total);
        }


        framedrop_total++;

    }

	cpu_loop_refresca_pantalla_return();
}




//Escribe texto en pantalla empezando por en x,y, gestionando salto de linea
//de momento solo se usa en panic para xwindows y fbdev
//ultima posicion y queda guardada en screen_print_y
void screen_print(int x,int y,int tinta,int papel,char *mensaje)
{
	while (*mensaje) {
		scr_putchar_menu(x++,y,*mensaje++,tinta,papel);
		if (x==32) {
			x=0;
			y++;
		}
	}
	screen_print_y=y;
}


void screen_set_parameters_slow_machines(void)
{

	if (no_cambio_parametros_maquinas_lentas.v==1) {
		debug_printf (VERBOSE_INFO,"Parameter nochangeslowparameters enabled. Do not change any frameskip or realvideo parameters");
		return;
	}

	//Parametros por defecto en Raspberry.
#ifdef EMULATE_RASPBERRY

	//Real beeper desactivado pues consume mas cpu (un 7 o 8 % mas en pc)
	if (beeper_real_enabled) {
		beeper_real_enabled=0;
		debug_printf (VERBOSE_INFO,"It is a raspberry system. Disabling Real Beeper");
	}


	//Frameskip 3 como minimo para realvideo
	if (rainbow_enabled.v==1) {
	        if (frameskip<3) {
        	        frameskip=3;
                	debug_printf (VERBOSE_INFO,"It is a raspberry system. With realvideo, setting frameskip to: %d",frameskip);
			return;
	        }
	}

	//Sin realvideo, frameskip 1 minimo
	if (rainbow_enabled.v==0) {
		if (frameskip<1) {
			frameskip=1;
			debug_printf (VERBOSE_INFO,"It is a raspberry system. Without realvideo, setting frameskip to: %d",frameskip);
			return;
		}
	}

	return;

#endif



}


//Activar rainbow y el estabilizador de imagen de zx8081
void enable_rainbow(void) {

	debug_printf (VERBOSE_INFO,"Enabling RealVideo");

    //solo para ver de donde se llama a aqui
    //debug_exec_show_backtrace();

	//si hay un cambio
	if (rainbow_enabled.v==0) {
        	rainbow_enabled.v=1;
		screen_set_parameters_slow_machines();
	}

        video_zx8081_estabilizador_imagen.v=1;


		/*Modos rainbow usan putpixel cache. Vaciarla por lo que pudiera haber antes
		//Si no se vaciase, si por ejemplo estamos con un programa en basic tipo:
		// 1 border 2: border 3: border 4: pause 1: cls
		//Si cambiamos de realvideo on , a off, y luego a on, al hacer on, que pasara:
		1: vemos franjas de border bien, con realvideo on, y usando putpixel cache
		2: no vemos colores, real video esta a off, y probablemente border 7 entero (lo normal). En modo no real video no usa putpixel cache
		3: volvemos a modo realvideo. Border estaba todo blanco. Como putpixel cache estaba antes con las franjas de colores,
		ahora las franjas estan mas o menos en el mismo sitio, y la cache dice que no hay que redibujarlas. Total: se ve todo el border 7
		*/
		clear_putpixel_cache();

}

//Desactivar rainbow
void disable_rainbow(void) {
	debug_printf (VERBOSE_INFO,"Disabling RealVideo");

	//Vofile necesita de rainbow para funcionar. no dejar desactivarlo si esta esto activo
	if (vofile_inserted.v==1) {
		debug_printf (VERBOSE_ERR,"Video out to file needs realvideo to work. You can not disable realvideo with video out enabled");
		return;
	}

	//si hay un cambio
	if (rainbow_enabled.v==1) {
	        rainbow_enabled.v=0;
		screen_set_parameters_slow_machines();
        }

        modificado_border.v=1;

	//Desactivar estos cuatro. Asi siempre que realvideo sea 0, ninguno de estos tres estara activo
	disable_interlace();
	disable_gigascreen();
	disable_ulaplus();
	spectra_disable();
}



void enable_border(void)
{
	border_enabled.v=1;
	modificado_border.v=1;
    
	//Recalcular algunos valores cacheados
    recalcular_get_total_ancho_rainbow();
    recalcular_get_total_alto_rainbow();

	//Siempre que se redimensiona tamanyo ventana (sin contar zoom) o se reinicia driver video hay que reiniciar cache putpixel
	init_cache_putpixel();
}

void disable_border(void)
{
    border_enabled.v=0;
	modificado_border.v=1;
    
	//Recalcular algunos valores cacheados
    recalcular_get_total_ancho_rainbow();
    recalcular_get_total_alto_rainbow();

	//Siempre que se redimensiona tamanyo ventana (sin contar zoom) o se reinicia driver video hay que reiniciar cache putpixel
	init_cache_putpixel();
}



void set_t_scanline_draw_zero(void)
{
        t_scanline_draw=0;

}

void t_scanline_next_line(void)
{

        t_scanline_draw++;

        if (MACHINE_IS_INVES) {
                //Inves

                if (t_scanline_draw>=screen_scanlines) {
                        set_t_scanline_draw_zero();
                        //printf ("reset inves a 0\n");
                }
        }


        t_scanline++;


}

int scr_ver_si_refrescar_por_menu_activo_z88(int x,int fila)
{

	//Usado en refresco de z88
	//sin rainbow se llama a la funcion normal
	//con rainbow, siempre debe hacer putpixel, que esto va al buffer rainbow. importante luego para que en video out no
	//aparezcan rectangulos negros al abrir el menu
	if (rainbow_enabled.v) return 1;

	else {
		/* Curiosidad:
		primero esto estaba mal y en vez del return y la funcion estaba solo la funcion, asi:
		scr_ver_si_refrescar_por_menu_activo(x,fila);
		Esto curiosamente deberia retornar un valor indefinido, pero en dos maquinas Linux, retornan el valor correcto de la funcion
		En cambio en mac os x, no retornaba valor correcto
		*/

		return scr_ver_si_refrescar_por_menu_activo(x,fila);
	}
}

void screen_z88_return_sbr(z88_dir *dir)
{

        z80_byte bank;
        z80_int direccion;

        int extAddressBank = (blink_sbr << 5) & 0xFF00;
        int extAddressOffset = (blink_sbr << 3) & 0x0038;

        bank=extAddressBank>>8;
        direccion=extAddressOffset<<8;

	dir->bank=bank;
	dir->dir=direccion;

}

void screen_z88_return_pb0(z88_dir *dir)
{

        z80_byte bank;
        z80_int direccion;

        int extAddressBank = (blink_pixel_base[0] << 3) & 0xF700;
        int extAddressOffset = (blink_pixel_base[0] << 1) & 0x003F;


        bank=extAddressBank>>8;
        direccion=extAddressOffset<<8;

        dir->bank=bank;
        dir->dir=direccion;

}

void screen_z88_return_pb1(z88_dir *dir)
{

        z80_byte bank;
        z80_int direccion;

        int extAddressBank = (blink_pixel_base[1] << 6) & 0xFF00;
        int extAddressOffset = (blink_pixel_base[1] << 4) & 0x0030;

        bank=extAddressBank>>8;
        direccion=extAddressOffset<<8;

        dir->bank=bank;
        dir->dir=direccion;


}

void screen_z88_return_pb2(z88_dir *dir)
{

        z80_byte bank;
        z80_int direccion;

        int extAddressBank = (blink_pixel_base[2] << 7) & 0xFF00;
        int extAddressOffset = (blink_pixel_base[2] << 5) & 0x0020;


        bank=extAddressBank>>8;
        direccion=extAddressOffset<<8;

        dir->bank=bank;
        dir->dir=direccion;

}

void screen_z88_return_pb3(z88_dir *dir)
{

        z80_byte bank;
        z80_int direccion;

        int extAddressBank = (blink_pixel_base[3] << 5) & 0xFF00;
        int extAddressOffset = (blink_pixel_base[3] << 3) & 0x0038;

        bank=extAddressBank>>8;
        direccion=extAddressOffset<<8;

        dir->bank=bank;
        dir->dir=direccion;

}




void screen_z88_dibujar_udg(z88_dir *tabla_caracter,int x,int y,int ancho,int inverse,int subrallado,int parpadeo,int gris,int lorescursor)
{


	/*
	//TODO. temp. comprovacion puntero
	if (tabla_caracter==NULL) {
		debug_printf (VERBOSE_INFO,"screen_z88_dibujar_udg. tabla_caracter=NULL");
		return;
	}
	*/


	z80_int color;
	z80_byte caracter;

	z80_int colorblanco;
	z80_int colornegro;
	z80_int colorgris;

	int offsety;
	int offsetx;

	int xmenu;
	int ymenu;


	//printf ("sc refres %d menu_over: %d\n",screen_refresh_menu,menu_overlay_activo);


	//colorblanco=15;
	colorblanco=Z88_PXCOLOFF;

	//colornegro=0;
	colornegro=Z88_PXCOLON;

	//colorgris=7;
	colorgris=Z88_PXCOLGREY;

	//Ver si se sale el ancho
	if (x+ancho>640) {
		//printf ("limite ancho en x: %d y: %d\n",x,y);
		int xorig=x;

		//borramos esa zona
		for (offsety=0;offsety<8;offsety++) {
			for (x=xorig;x<640;x++) {
				scr_putpixel_zoom_z88(x,y+offsety,colorblanco);
				//printf ("borrar zona %d %d\n",x,y+offsety);
			}
		}
		return;
	}

        //Caracter con parpadeo, no cursor
        if (parpadeo && estado_parpadeo.v && !lorescursor) {

                        //Parpadeo activo y no es cursor. borramos esa zona.
                        //El parpadeo funciona asi: no activo: dibujamos caracter. activo: borramos zona con color blanco
                        for (offsety=0;offsety<8;offsety++) {
                                for (offsetx=0;offsetx<ancho;offsetx++) {
                                        //Ver si esta zona esta ocupada por el menu
                                        xmenu=(x+offsetx)/8;
                                        ymenu=(y+offsety)/8;

                                        if (xmenu>=0 && ymenu>=0 /*&& xmenu<=31 && ymenu<=23*/) {
                                                if (scr_ver_si_refrescar_por_menu_activo_z88(xmenu,ymenu)) {
                                                        scr_putpixel_zoom_z88(x+offsetx,y+offsety,colorblanco);
                                                }
                                        }
                                        else scr_putpixel_zoom_z88(x+offsetx,y+offsety,colorblanco);


                                }
                        }


                        return;
        }

        //Cursor con parpadeo
        if (parpadeo && estado_parpadeo_cursor.v && lorescursor) {

                //Es cursor. Invertir colores
                        z80_int c;
                        //Invertir colores
                        c=colorblanco;
                        colorblanco=colornegro;
                        colornegro=c;
        }



	for (offsety=0;offsety<8;offsety++) {


		//caracter=*tabla_caracter++;
		//if (caracter=='!') printf ("tabla caracter: dir: %x bank: %x\n",tabla_caracter->dir,tabla_caracter->bank);

		caracter=peek_byte_no_time_z88_bank_no_check_low(tabla_caracter->dir,tabla_caracter->bank);
		tabla_caracter->dir++;

		if (inverse) caracter = caracter ^255;

		if (subrallado && offsety==7) caracter=255;

		//printf ("caracter: %x\n",caracter);
		if (ancho==6) caracter <<=2;
		for (offsetx=0;offsetx<ancho;offsetx++) {
			if (caracter&128) {
				color=colornegro;
				if (gris) color=colorgris;
			}

			else color=colorblanco;


                        //Ver si esta zona esta ocupada por el menu
                        xmenu=(x+offsetx)/8;
                        ymenu=(y+offsety)/8;
                        if (xmenu>=0 && ymenu>=0 /*&& xmenu<=31 && ymenu<=23*/) {
                                if (scr_ver_si_refrescar_por_menu_activo_z88(xmenu,ymenu)) {
                                        scr_putpixel_zoom_z88(x+offsetx,y+offsety,color);
                                }
                        }

			else scr_putpixel_zoom_z88(x+offsetx,y+offsety,color);



			caracter <<=1;
		}
	}


}

void screen_z88_putpixel_zoom_rainbow (int x,int y,unsigned int color)
{
	//Metemos en buffer rainbow putpixel de pantalla Z88
        z80_int *puntero_buf_rainbow;

	puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow()+x ];

	*puntero_buf_rainbow=color;
}

void set_z88_putpixel_zoom_function(void)
{

                if (rainbow_enabled.v==0) {
                        scr_putpixel_zoom_z88=scr_putpixel_zoom;
                }

                else {
                        //modo realvideo
                        scr_putpixel_zoom_z88=screen_z88_putpixel_zoom_rainbow;
                }

}

void screen_z88_refresca_pantalla(void)
{

	//Realmente Z88 no hace modo realvideo, es decir, no dibujamos linea a linea a cada final de scanline de la pantalla
	//sino que cada vez que se va a refrescar pantalla, primero se dibuja toda la pantalla del z88 pero dentro del buffer rainbow
	//luego se dibuja el buffer rainbow en pantalla como cualquier otra maquina
	//Esto permite tener grabacion de video a archivo, ya que requiere modo realvideo para grabar a archivo

	//Para que las funciones de dibujado de z88 sean comunes para no-rainbow y rainbow,
	//las llamadas a putpixel seran las que establecemos ahora

	set_z88_putpixel_zoom_function();

                if (rainbow_enabled.v==0) {
			screen_z88_refresca_pantalla_comun();
		}

                else {
                        //modo realvideo
			screen_z88_refresca_pantalla_comun();

			scr_refresca_pantalla_rainbow_comun();
                }

}


//Refrescar pantalla para drivers graficos
void screen_z88_refresca_pantalla_comun(void)
{

	/*
	z80_byte *sbr=screen_z88_return_sbr();
	z80_byte *lores0=screen_z88_return_pb0();
	z80_byte *lores1=screen_z88_return_pb1();
	z80_byte *hires0=screen_z88_return_pb2();
	z80_byte *hires1=screen_z88_return_pb3();
	*/



        if ((blink_com&1)==0) {
                debug_printf (VERBOSE_DEBUG,"LCD is OFF");
                int x,y,xmenu,ymenu;

                for (y=0;y<64;y++) {
                        for (x=0;x<screen_get_emulated_display_width_no_zoom();x++) {

	                        //Ver si esta zona esta ocupada por el menu
        	                xmenu=x/8;
	                        ymenu=y/8;
	                        if (xmenu>=0 && ymenu>=0 /*&& xmenu<=31 && ymenu<=23*/) {
        	                        if (scr_ver_si_refrescar_por_menu_activo_z88(xmenu,ymenu)) {
						scr_putpixel_zoom_z88(x,y,Z88_PXCOLSCROFF);
					}
				}
				else {
                                	scr_putpixel_zoom_z88(x,y,Z88_PXCOLSCROFF);
				}
                        }
                }
                return;
        }


	z88_dir sbr,lores0,lores1,hires0,hires1;

	screen_z88_return_sbr(&sbr);
	screen_z88_return_pb0(&lores0);
	screen_z88_return_pb1(&lores1);
	screen_z88_return_pb2(&hires0);
	screen_z88_return_pb3(&hires1);

	//temp. hacer dir>16384

	/*
	sbr.bank--;
	sbr.dir+=16384;

	lores0.bank--;
	lores0.dir+=16384;

        lores1.bank--;
        lores1.dir+=16384;

        hires0.bank--;
        hires0.dir+=16384;

        hires1.bank--;
        hires1.dir+=16384;
	*/


	/*
	printf ("sbr bank %x add %x\n",sbr.bank,sbr.dir);
	printf ("lores0 bank %x add %x\n",lores0.bank,lores0.dir);
	printf ("lores1 bank %x add %x\n",lores1.bank,lores1.dir);
	printf ("hires0 bank %x add %x\n",hires0.bank,hires0.dir);
	printf ("hires1 bank %x add %x\n",hires1.bank,hires1.dir);
	*/


	z80_byte caracter,atributo;


	int x;
	int y;
	int ancho;

	z88_dir copia_sbr;

	copia_sbr.bank=sbr.bank;
	copia_sbr.dir=sbr.dir;

	//z80_byte *tabla_caracteres;
	z88_dir tabla_caracteres;

	int null_caracter;
	int inverse,subrallado,parpadeo,gris;
	int lorescursor;

	int caracteres_linea;


	for (y=0;y<64;y+=8) {
		x=0;
		caracteres_linea=0;
		while (x<640 && caracteres_linea<108) {


			//printf ("temp. x: %d y: %d\n",x,y);

			//caracter=*sbr++;
                	caracter=peek_byte_no_time_z88_bank_no_check_low(sbr.dir,sbr.bank);
			sbr.dir++;


			//atributo=*sbr++;
                        atributo=peek_byte_no_time_z88_bank_no_check_low(sbr.dir,sbr.bank);
                        sbr.dir++;


			//printf ("sbr bank: %x dir: %x caracter: %x atributo: %x\n",sbr.bank,sbr.dir,caracter,atributo);
			//if (caracter>31 && caracter<128) printf ("%c",caracter);

			caracteres_linea++;

			inverse=(atributo & 16 ? 1 : 0);
			subrallado=0;


			parpadeo=(atributo & 8 ? 1 : 0);
			gris=(atributo & 4 ? 1 : 0);
			/*

Attribute 2 (odd address):      Attribute 1 (even address):
7   6   5   4   3   2   1   0   7   6   5   4   3   2   1   0
---------------------------------------------------------------
sw1 sw2 lrs rev fls gry und ch8 ch7 ch6 ch5 ch4 ch3 ch2 ch1 ch0
---------------------------------------------------------------

sw1 : no hardware effect (used to store tiny flag)
sw2 : no hardware effect (used to store bold flag)
hrs : refer to Hires font (i.e. shift 8 bits in register), else Lores
rev : reverse (i.e. XOR)
fls : flash (1 second period)
gry : grey (probably a faster flash period)
und : underline (i.e. set the 8 bits when on 8th row), only valid for Lores
      it becomes ch9 when hrs is set.

The Lores fonts are addressed by 9 bits. It represents an amount of 512 characters ($000-$1FF).
- Lores1 ($000-$1BF) is the 6 * 8 OZ characters file in ROM
- Lores0 ($1C0-$1FF) is the 6 * 8 user defined characters file in RAM. Assignment starts from top address with the character for code '@'.

The Hires chars are addressed by 10 bits. It represents an amount of 1024 characters ($000-$3FF).
- Hires0 ($000-$2FF) is the 8 * 8 map file in RAM (only 256 are used)
- Hires1 ($300-$3FF) is the 8 * 8 characters for the OZ window (only 128 are used)


               5     4     3     2     1     0,  7-0
               hrs   rev   fls   gry   und   ch8-ch0
----------------------------------------------------
LORES          0     v     v     v     v     000-1FF
LORES CURSOR   1     1     1     v     v     000-1FF
NULL CHARACTER 1     1     0     1     xxx    -  xxx
HIRES          1     0     v     v     000    -  3FF



			*/
			z80_byte tipo_caracter=atributo&0x3E; //00111110
				null_caracter=0;
			//LORES o LORESCURSOR
			lorescursor=( (tipo_caracter & (32+16+8+4))==32+16+8 ? 1 : 0);

			if ( (tipo_caracter & 32)==0  || lorescursor ) {
				//printf ("(tipo_caracter & 32)==0  || lorescursor\n");
				ancho=6;
				//LORES
				if (atributo&2) subrallado=1;
				//Ver si 0 o 1
				z80_int caracter16=caracter | ((atributo&1)<<8);
				if (caracter16<=0x1BF) {
					//LORES1
					//tabla_caracteres=lores1+caracter16*8;
					tabla_caracteres.bank=lores1.bank;
					tabla_caracteres.dir=lores1.dir+caracter16*8;

					//if (parpadeo) printf ("en lores1 x: %d y: %d caracter16: %d lorescursor: %d\n",x,y,caracter16,lorescursor);
					//printf ("lores1 caracter16: %d\n",caracter16);


					//if (caracter16=='a') printf ("tabla caracter: dir: %x bank: %x\n",tabla_caracteres.dir,tabla_caracteres.bank);


				}
				else {
					//LORES0
					//tabla_caracteres=lores0+(caracter16-0x1c0)*8;
                                        tabla_caracteres.bank=lores0.bank;
                                        tabla_caracteres.dir=lores0.dir+(caracter16-0x1c0)*8;

					//printf ("lores0 caracter16: %d\n",caracter16);

				}

			}

			else if ((tipo_caracter & 48)==32) {
				//printf ("((tipo_caracter & 48)==32)\n");
				//HIRES
				ancho=8;
				//Ver si 0 o 1
				z80_int caracter16=caracter | ((atributo&3)<<8);
				if (caracter16<=0x2ff) {
					//HIRES0
					//tabla_caracteres=hires0+caracter16*8;
                                        tabla_caracteres.bank=hires0.bank;
                                        tabla_caracteres.dir=hires0.dir+caracter16*8;

					//if (parpadeo) printf ("en hires0 x: %d y: %d caracter16: %d *8: %d\n",x,y,caracter16,caracter16*8);
					//printf ("hires0 caracter16: %d\n",caracter16);


				}
				else {
					//HIRES1
					//tabla_caracteres=hires1+(caracter16-0x300)*8;
                                        tabla_caracteres.bank=hires1.bank;
                                        tabla_caracteres.dir=hires1.dir+(caracter16-0x300)*8;
					//printf ("hires1 caracter16: %d\n",caracter16);


					//temp
					//tabla_caracteres=hires1+caracter*8;
					//if (parpadeo) printf ("en hires1 x: %d y: %d caracter16: %d *8: %d\n",x,y,caracter16,(caracter16-0x300)*8 );

					//temp
					//if (caracter16==928) tabla_caracteres=hires1+(caracter16-0x300-128)*8;
					//if (parpadeo) parpadeo=0;

                                }

			}

			else if ((tipo_caracter & (32+16+8+4) )==32+16+4) {
				//NULL character
				null_caracter=1;
			}

			else {
				//Cualquier otro caso. por ejemplo, atributo: 51 (110011 ), no coincide con nada
				//Caracter corrupto
				//establecer ancho a algun valor. esto es importante, porque la variable ancho
				//no viene inicializada, y si el primer byte no coincide con ningun caracter normal, tendra valor indeterminado,
				//y en el siguiente codigo (mas abajo) donde hace x=x+ancho, x se sale de rango y provoca segmentation fault
				ancho=6;
				//printf ("ninguno de los anteriores caracter: %d atributo: %d\n",caracter,atributo);
			}

			//temp
			//if (parpadeo) printf ("parpadeo x: %d y: %d caracteres_linea: %d caracter: %d atributo: %d ancho: %d\n",x,y,caracteres_linea,caracter,atributo,ancho);

			if (null_caracter==0) {
				screen_z88_dibujar_udg(&tabla_caracteres,x,y,ancho,inverse,subrallado,parpadeo,gris,lorescursor);
				//if (ancho>20 || ancho<0) printf ("temp. x: %d y: %d ancho: %d\n",x,y,ancho);
				x=x+ancho;
				//if (ancho>20 || ancho<0)  printf ("temp. x: %d y: %d\n",x,y);
			}

		}

		//Restauramos valor inicial y sumamos 256. bank no se altera
		copia_sbr.dir +=256;
		sbr.dir=copia_sbr.dir;
	}

}


void screen_z88_draw_lower_screen(void)
{
	if (!MACHINE_IS_Z88) return;

	set_z88_putpixel_zoom_function();

#ifdef COMPILE_CURSES
	if (!strcmp(scr_new_driver_name,"curses")) {
		scrcurses_z88_draw_lower_screen();
		return;
	}
#endif


	if (si_complete_video_driver() ) {


		debug_printf (VERBOSE_DEBUG,"screen_z88_draw_lower_screen");

		int x,y;

		for (y=64;y<screen_get_emulated_display_height_no_zoom();y++) {
			for (x=0;x<screen_get_emulated_display_width_no_zoom();x++) {
				scr_putpixel_zoom_z88(x,y,7);
			}
		}
	}
}

void z88_return_character_atributes(struct s_z88_return_character_atributes *z88_caracter)
{

	z80_byte caracter,atributo;

	z88_dir sbr;
	sbr.dir=z88_caracter->sbr.dir;
	sbr.bank=z88_caracter->sbr.bank;

	//z80_byte *sbr=z88_caracter->sbr;

			//caracter=*sbr++;
                        caracter=peek_byte_no_time_z88_bank_no_check_low(sbr.dir,sbr.bank);
                        sbr.dir++;


                        //atributo=*sbr++;
                        atributo=peek_byte_no_time_z88_bank_no_check_low(sbr.dir,sbr.bank);
                        sbr.dir++;


			z88_caracter->inverse=(atributo & 16 ? 1 : 0);
			z88_caracter->subrallado=0;
			z88_caracter->parpadeo=(atributo & 8 ? 1 : 0);
			z88_caracter->gris=(atributo & 4 ? 1 : 0);
			/*

Attribute 2 (odd address):      Attribute 1 (even address):
7   6   5   4   3   2   1   0   7   6   5   4   3   2   1   0
---------------------------------------------------------------
sw1 sw2 lrs rev fls gry und ch8 ch7 ch6 ch5 ch4 ch3 ch2 ch1 ch0
---------------------------------------------------------------

sw1 : no hardware effect (used to store tiny flag)
sw2 : no hardware effect (used to store bold flag)
hrs : refer to Hires font (i.e. shift 8 bits in register), else Lores
rev : reverse (i.e. XOR)
fls : flash (1 second period)
gry : grey (probably a faster flash period)
und : underline (i.e. set the 8 bits when on 8th row), only valid for Lores
      it becomes ch9 when hrs is set.

The Lores fonts are addressed by 9 bits. It represents an amount of 512 characters ($000-$1FF).
- Lores1 ($000-$1BF) is the 6 * 8 OZ characters file in ROM
- Lores0 ($1C0-$1FF) is the 6 * 8 user defined characters file in RAM. Assignment starts from top address with the character for code '@'.

The Hires chars are addressed by 10 bits. It represents an amount of 1024 characters ($000-$3FF).
- Hires0 ($000-$2FF) is the 8 * 8 map file in RAM (only 256 are used)
- Hires1 ($300-$3FF) is the 8 * 8 characters for the OZ window (only 128 are used)


               5     4     3     2     1     0,  7-0
               hrs   rev   fls   gry   und   ch8-ch0
----------------------------------------------------
LORES          0     v     v     v     v     000-1FF
LORES CURSOR   1     1     1     v     v     000-1FF
NULL CHARACTER 1     1     0     1     xxx    -  xxx
HIRES          1     0     v     v     000    -  3FF



			*/

			z80_byte tipo_caracter=atributo&0x3E; //00111110
				z88_caracter->null_caracter=0;

			int caracter16=0; //lo inicializo a 0 para evitar warnings al compilar

                        //Cursor. no hace falta
                        /*
                        if ((tipo_caracter & (32+16+8+4) )==32+16+8 ) {
                                ascii_caracter=' ';
                        }
                        */




			//LORES o LORESCURSOR
			if ( (tipo_caracter & 32)==0  || (tipo_caracter & (32+16+8+4) )==32+16+8 ) {
				z88_caracter->ancho=6;
				//LORES
				if (atributo&2) z88_caracter->subrallado=1;
				//Ver si 0 o 1
				caracter16=caracter | ((atributo&1)<<8);
				if (caracter16<=0x1BF) {
					//LORES1

					//ENTER
                                        if (caracter16==259) caracter16='E';
                                        if (caracter16==260) caracter16='N';
                                        if (caracter16==261) caracter16='T';

					//ESC
                                        if (caracter16==268) caracter16='E';
                                        if (caracter16==269) caracter16='S';
                                        if (caracter16==270) caracter16='C';



					//MENU
					if (caracter16==274) caracter16='M';
					if (caracter16==275) caracter16='E';
					if (caracter16==276) caracter16='N';
				}
				else {
					//LORES0
					caracter16-= 0x1c0;


				}

				//Recuadros. por ejemplo del flashstore
				if (caracter16==387) caracter16='|';

				if (caracter16==389) caracter16='-';
				if (caracter16==390) caracter16='|';

				if (caracter16==393) caracter16='|';

				if (caracter16==394) caracter16='|';
				if (caracter16==395) caracter16='|';

				if (caracter16==396) caracter16='|';

				if (caracter16==398) caracter16='|';

				//Diamond
				if (caracter16==400) caracter16='D';

				//Square
				if (caracter16==401) caracter16='S';

				//Izq
				if (caracter16==406) caracter16='<';
				if (caracter16==407) caracter16='-';

				//Der
				if (caracter16==408) caracter16='-';
				if (caracter16==409) caracter16='>';

				//Aba
				if (caracter16==410) caracter16='v';
				if (caracter16==411) caracter16='v';

				//Arr
				if (caracter16==412) caracter16='^';
				if (caracter16==413) caracter16='^';


				//ENTER
				if (caracter16==419) caracter16='E';
                                if (caracter16==420) caracter16='N';
                                if (caracter16==421) caracter16='T';

				//TAB
                                if (caracter16==422) caracter16='T';
                                if (caracter16==423) caracter16='A';
                                if (caracter16==424) caracter16='B';



				//ESC
				if (caracter16==428) caracter16='E';
				if (caracter16==429) caracter16='S';
				if (caracter16==430) caracter16='C';

				//MENU
                                if (caracter16==434) caracter16='M';
                                if (caracter16==435) caracter16='E';
                                if (caracter16==436) caracter16='N';


				//shift
				if (caracter16==440) caracter16='S';
				if (caracter16==441) caracter16='H';
				if (caracter16==442) caracter16='I';

				//linea vertical superior de menu
				if (caracter16==446) caracter16='|';
				if (caracter16==447) caracter16='|';


			}

			else if ((tipo_caracter & 48)==32) {
				//HIRES
				z88_caracter->ancho=8;
				//Ver si 0 o 1
				caracter16=caracter | ((atributo&3)<<8);
				if (caracter16<=0x2ff) {
					//HIRES0
				}
				else {
					//HIRES1
					caracter16-= 0x300;

					//Caracteres especiales:
					//OZ
					if (caracter16==128) caracter16='O';
					if (caracter16==129) caracter16='Z';

					//CAPS
					if (caracter16==132) caracter16='C';
					if (caracter16==133) caracter16='L';

					//Diamond
					if (caracter16==144) caracter16='D';

					//Square
					if (caracter16==145) caracter16='S';





                                }

			}

			else if ((tipo_caracter & (32+16+8+4) )==32+16+4) {
				//NULL character
				z88_caracter->null_caracter=1;
			}


			//Temp guardamos en estructura caracter si conversion a "." si no es printable
			//esto sirve para obtener el valor de codigos no printables, como por ejemplo, diamond
			//z88_caracter->temp_orig=caracter16;

			//Adaptar caracter a conjunto ASCII
			if (z88_caracter->null_caracter==0) {
				if (caracter16<32) caracter16 +=32;

                                caracter16 &=127;

				if (caracter16<32 || caracter16>127)  caracter16='.';
			}


			z88_caracter->ascii_caracter=caracter16;

}




//Refrescar pantalla para drivers de texto
void screen_repinta_pantalla_z88(struct s_z88_return_character_atributes *z88_caracter)
{

	/*
        z80_byte bank;
        z80_int direccion;


        bank = (blink_sbr >>3) ;
        direccion = (blink_sbr << 3) & 0x38;
        direccion=direccion<<8;


        //Offset dentro del slot de memoria
        z80_long_int offset=bank*16384;

        offset+=(direccion&16383);
	*/

        z88_dir sbr;

        screen_z88_return_sbr(&sbr);


	z88_dir copiasbr;
	copiasbr.bank=sbr.bank;
	copiasbr.dir=sbr.dir;


        //z80_long_int offsetcopia=offset;
        //        offset=offsetcopia;


        //debug_printf (VERBOSE_DEBUG,"bank: 0x%x direccion: 0x%x (%d)-----",bank,direccion,direccion);

	if ((blink_com&1)==0) {
		debug_printf (VERBOSE_DEBUG,"LCD is OFF");

		//metemos toda la pantalla en blanco
		for (z88_caracter->y=0; z88_caracter->y<8 ; z88_caracter->y++) {
			for (z88_caracter->x=0 ; z88_caracter->x<106 ; z88_caracter->x++) {
				z88_caracter->ascii_caracter=' ';
				z88_caracter->inverse=0;
				z88_caracter->subrallado=0;
				z88_caracter->parpadeo=0;
				z88_caracter->gris=0;
				z88_caracter->null_caracter=0;
				z88_caracter->f_print_char(z88_caracter);
			}
	                //Linea siguiente
        	        z88_caracter->f_new_line(z88_caracter);
		}
		return;
	}



        for (z88_caracter->y=0; z88_caracter->y<8 ; z88_caracter->y++) {
                int bytes_leidos;

                //Cada linea mientras se lean menos de 106 caracteres no nulos, y mientras se lean menos de 256 bytes
                for (z88_caracter->x=0 , bytes_leidos=0; z88_caracter->x<106 && bytes_leidos<256; z88_caracter->x++,bytes_leidos +=2) {

			z88_caracter->sbr.bank=sbr.bank;
			z88_caracter->sbr.dir=sbr.dir;

                        z88_return_character_atributes(z88_caracter);

			sbr.dir +=2;


                        //Si caracter no es nulo
                        if (z88_caracter->null_caracter==0) {
				z88_caracter->f_print_char(z88_caracter);

                        }

                        //Y si es nulo, no avanzamos caracter
                        else {
                                z88_caracter->x--;
                        }

                }

                //Linea siguiente
		z88_caracter->f_new_line(z88_caracter);

		copiasbr.dir +=256;
		sbr.bank=copiasbr.bank;
		sbr.dir=copiasbr.dir;

        }


}




//Redibujado de pantalla para jupiter ace en entorno grafico (y para curses)
void scr_refresca_pantalla_ace(void)
{


	//La pantalla del jupiter ACE esta en la direccion 2400H, ocupa 32*24=768 bytes
	//La tabla de caracteres esta en la direccion 2C00H, los primeros 32 bytes son UDG y graficos predefinidos
	//Caracteres por defecto son en color inverso. Si bit 7 esta a 1, caracteres son en negro sobre blanco


	z80_int direccion=0x2400;

	int x,y;
	z80_byte caracter;
	//z80_bit inverse;

	//z80_int puntero_tabla;

	for (y=0;y<24;y++) {
		for (x=0;x<32;x++) {
			caracter=peek_byte_no_time(direccion++);


			if (scr_ver_si_refrescar_por_menu_activo(x,y)) scr_putchar_zx8081(x,y,caracter);
		}
	}


}


//Redibujado de border para jupiter ace en entorno grafico
void scr_refresca_pantalla_y_border_ace(void)
{



			if (border_enabled.v) {
        //Border en Ace es 0 y no cambia nunca
        if (modificado_border.v) {
                //Dibujar border. Color 0
                scr_refresca_border_comun_spectrumzx8081(0);
                modificado_border.v=0;
        }
			}


        scr_refresca_pantalla_ace();
}











//Debug registros crtc
//int debug_regs_muestra=0;






//Hace putpixel en x,y
void sam_putpixel_zoom(int x,int y,unsigned int color)
{

        int dibujar=0;

        //if (x>255) dibujar=1;
        //else if (y>191) dibujar=1;
        if (scr_ver_si_refrescar_por_menu_activo(x/8,y/8)) dibujar=1;

        if (dibujar) {
                scr_putpixel_zoom(x,y,SAM_INDEX_FIRST_COLOR+  (  (sam_palette[color&15]) & 127) );
                scr_putpixel_zoom(x,y+1,SAM_INDEX_FIRST_COLOR+(  (sam_palette[color&15]) & 127) );
        }
}

//Retorna byte pantalla
z80_byte sam_retorna_byte_pantalla(z80_byte *s,z80_int *o)
{

	z80_byte segmento;
	segmento=*s;

	z80_int offset;
	offset=*o;

	z80_byte *puntero;
	puntero=sam_ram_memory[segmento]+offset;
	z80_byte valor=*puntero;

	offset++;
	if (offset>16383) {
		offset=0;
		segmento++;
		if (segmento>31) {
			segmento=0;
		}
	}

	*s=segmento;
	*o=offset;

	return valor;
}

//Retorna byte pantalla
z80_byte sam_retorna_byte_pantalla_mode1(z80_byte *s,z80_int *o,z80_byte *at)
{

        z80_byte segmento;
        segmento=*s;

        z80_int offset;
        offset=*o;

        z80_byte *puntero;
        puntero=sam_ram_memory[segmento]+offset;
        z80_byte valor=*puntero;

	puntero=sam_ram_memory[segmento]+offset+8192;
	*at=*puntero;

        offset++;
        if (offset>16383) {
                offset=0;
                segmento++;
                if (segmento>31) {
                        segmento=0;
                }
        }

        *s=segmento;
        *o=offset;

        return valor;
}

z80_byte sam_retorna_byte_pantalla_spectrum(z80_byte segmento,int x,int y,z80_byte *at)
{
	int col=x/16;
	z80_int direccion=screen_addr_table[(y<<5)]+col;
	z80_byte *puntero;
	puntero=sam_ram_memory[segmento]+direccion;

	z80_byte valor=*puntero;


	int fila;
        fila=y/8;
        z80_int dir_atributo=6144+(fila*32)+col;
	puntero=sam_ram_memory[segmento]+dir_atributo;
	*at=*puntero;

	return valor;
}

void scr_refresca_border_sam(unsigned int color)
{
//      printf ("Refresco border sam\n");

        int x,y;

	color=SAM_INDEX_FIRST_COLOR+(  (sam_palette[color&15]) & 127 );


        //parte superior
        for (y=0;y<SAM_TOP_BORDER;y++) {
                for (x=0;x<SAM_DISPLAY_WIDTH*zoom_x+SAM_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,y,color);


                }
        }

        //parte inferior
        for (y=0;y<SAM_TOP_BORDER;y++) {
                for (x=0;x<SAM_DISPLAY_WIDTH*zoom_x+SAM_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,SAM_TOP_BORDER+y+SAM_DISPLAY_HEIGHT*zoom_y,color);


                }
        }


        //laterales
        for (y=0;y<SAM_DISPLAY_HEIGHT*zoom_y;y++) {
                for (x=0;x<SAM_LEFT_BORDER;x++) {
                        scr_putpixel(x,SAM_TOP_BORDER+y,color);
                        scr_putpixel(SAM_LEFT_BORDER+SAM_DISPLAY_WIDTH*zoom_x+x,SAM_TOP_BORDER+y,color);
                }

        }


}

void scr_refresca_border_ql(unsigned int color)
{
//      printf ("Refresco border sam\n");

        int x,y;


        //parte superior
        for (y=0;y<QL_TOP_BORDER;y++) {
                for (x=0;x<QL_DISPLAY_WIDTH*zoom_x+QL_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,y,color);


                }
        }

        //parte inferior
        for (y=0;y<QL_TOP_BORDER;y++) {
                for (x=0;x<QL_DISPLAY_WIDTH*zoom_x+QL_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,QL_TOP_BORDER+y+QL_DISPLAY_HEIGHT*zoom_y,color);


                }
        }


        //laterales
        for (y=0;y<QL_DISPLAY_HEIGHT*zoom_y;y++) {
                for (x=0;x<QL_LEFT_BORDER;x++) {
                        scr_putpixel(x,QL_TOP_BORDER+y,color);
                        scr_putpixel(QL_LEFT_BORDER+QL_DISPLAY_WIDTH*zoom_x+x,QL_TOP_BORDER+y,color);
                }

        }


}



//Refresco de pantalla SAM sin rainbow
void scr_refresca_pantalla_sam(void)
{
	//Obtener direccion
	//int direccion=((sam_vmpr&sam_memoria_total_mascara)+1)*16384;
	//printf ("direccion pantalla: %d\n",direccion);
	z80_byte segmento_pantalla=sam_vmpr&sam_memoria_total_mascara;
	//printf ("segmento pantalla: %d\n",segmento_pantalla);
	z80_int offset=0;

	z80_byte modo_video=(sam_vmpr>>5)&3;
	int total_alto;
	int total_ancho;
	int x,y;

	unsigned int color;
	z80_byte byte_leido;
	z80_byte atributo_spectrum;
	z80_byte ink,paper,aux,bright,flash;
	int bit;


		total_alto=192;
		total_ancho=512;

		for (y=0;y<total_alto;y++){
			for (x=0;x<total_ancho;) {
				//printf ("x: %d\n",x);

				switch (modo_video) {
					case 0:
					byte_leido=sam_retorna_byte_pantalla_spectrum(segmento_pantalla,x,y,&atributo_spectrum);
					ink=atributo_spectrum &7;
                                	paper=(atributo_spectrum>>3) &7;
	                                bright=(atributo_spectrum) &64;
        	                        flash=(atributo_spectrum)&128;
                	                if (flash) {
                        	                //intercambiar si conviene
                                	        if (estado_parpadeo.v) {
	                                                aux=paper;
        	                                        paper=ink;
                	                                ink=aux;
                        	                }
	                                }

        	                        if (bright) {
                	                        ink +=8;
                        	                paper +=8;
	                                }

        	                        for (bit=0;bit<8;bit++) {

                	                        color= ( byte_leido & 128 ? ink : paper );

						//printf ("x: %d\n",x+bit);
						sam_putpixel_zoom(x,y*2,color);
						x++;

						//printf ("x: %d\n",x+bit);
						sam_putpixel_zoom(x,y*2,color);
						x++;

	                                        byte_leido=byte_leido<<1;
        	                        }

					break;

					case 1:
					//256x192
					byte_leido=sam_retorna_byte_pantalla_mode1(&segmento_pantalla,&offset,&atributo_spectrum);
                                        ink=atributo_spectrum &7;
                                        paper=(atributo_spectrum>>3) &7;
                                        bright=(atributo_spectrum) &64;
                                        flash=(atributo_spectrum)&128;
                                        if (flash) {
                                                //intercambiar si conviene
                                                if (estado_parpadeo.v) {
                                                        aux=paper;
                                                        paper=ink;
                                                        ink=aux;
                                                }
                                        }

                                        if (bright) {
                                                ink +=8;
                                                paper +=8;
                                        }

                                        for (bit=0;bit<8;bit++) {

                                                color= ( byte_leido & 128 ? ink : paper );

                                                //printf ("x: %d\n",x+bit);
                                                sam_putpixel_zoom(x,y*2,color);
                                                x++;

                                                //printf ("x: %d\n",x+bit);
                                                sam_putpixel_zoom(x,y*2,color);
                                                x++;

                                                byte_leido=byte_leido<<1;
                                        }


					break;

					case 2:
					//512x192, 4 colours per pixel (2 bits per byte)

					byte_leido=sam_retorna_byte_pantalla(&segmento_pantalla,&offset);

					color=((byte_leido)>>6)&3;
					sam_putpixel_zoom(x++,y*2,color);

					color=((byte_leido)>>4)&3;
					sam_putpixel_zoom(x++,y*2,color);

					color=((byte_leido)>>2)&3;
					sam_putpixel_zoom(x++,y*2,color);

					color=((byte_leido))&3;
					sam_putpixel_zoom(x++,y*2,color);

					break;


					case 3:
					//256x192. 16 colours per pixel (4 bits per byte)

					byte_leido=sam_retorna_byte_pantalla(&segmento_pantalla,&offset);
					color=((byte_leido)>>4)&15;
					sam_putpixel_zoom(x++,y*2,color);
					sam_putpixel_zoom(x++,y*2,color);

					color=((byte_leido))&15;
                                        sam_putpixel_zoom(x++,y*2,color);
                                        sam_putpixel_zoom(x++,y*2,color);

					break;

				}
			}
		}

}

void scr_refresca_pantalla_y_border_sam(void)
{

        //Refrescar border si conviene
        if (border_enabled.v) {
                if (modificado_border.v) {
                        //Dibujar border. Color 0
                        unsigned int color=sam_border&7;
                        //color=sam_palette_table[color];
                        //color +=SAM_INDEX_FIRST_COLOR;

                        scr_refresca_border_sam(color);
                        modificado_border.v=0;
                }

        }


        scr_refresca_pantalla_sam();
}



//Redibujado de pantalla para jupiter ace en entorno texto
void screen_text_repinta_pantalla_ace(void)
{
        //Desde 2400H 768 bytes

        int i=0;

        z80_byte letra;

        printf ("\n");

        int columna=0;

        for (i=0;i<768;i++) {
                letra=peek_byte_no_time(0x2400+i);

                //Eliminar bit de caracter inverso
                letra=letra&127;

                if (letra>31 || letra<128) printf ("%c",letra);
                else printf (".");

                columna++;
                if (columna==32) {
                        printf ("\n");
                        columna=0;
                }
        }

}

//Hace putpixel en x,y doblando en alto
void ql_putpixel_zoom(int x,int y,unsigned int color)
{

        scr_putpixel_zoom(x,y,QL_INDEX_FIRST_COLOR+color);
        scr_putpixel_zoom(x,y+1,QL_INDEX_FIRST_COLOR+color);

}


//int temp_offset_ql_pan=131072;
//int temp_offset_cuando=0;

//Refresco de pantalla ql sin rainbow
void scr_refresca_pantalla_ql(void)
{


/*
$18063	MC_STAT		Master chip status register
Bit	Purpose
1	0 = Screen on
  1 = Screen off

3	0 = 4 colour (mode 4)
  1 = 8 colour (mode 8)

7	0 = Use screen 0 (allegedly at $20000)
  1 = Use screen 1 (allegedly at $280000)

*/

    z80_byte mc_stat=ql_mc_stat;
    int video_mode=(mc_stat>>3)&1;
    //printf ("mc_stat: %02XH video_mode: %d\n",mc_stat,video_mode);



    int total_alto;
    int total_ancho;
    int x,y;

    unsigned int color1;
    //unsigned int color2;

    z80_byte green,red,blue;

    z80_byte byte_leido_h,byte_leido_l;

    unsigned char *memoria_pantalla_ql;

    memoria_pantalla_ql=&memoria_ql[0x20000 + ((mc_stat & 0x80) << 8)];



    total_alto=256;
    total_ancho=512;

    int flashing_color;

    for (y=0;y<total_alto;y++){
        //Al principio de cada linea, flash es siempre 0
        int ql_linea_flashing=0;
        for (x=0;x<total_ancho;) {
            
/*
In 512-pixel mode, two bits per pixel are used, and the GREEN and BLUE signals are tied together, giving a choice of four colours:
black, white, green and red. On a monochrome screen, this will translate as a four level greyscale.
In 256-pixel mode, four bits per pixel are used: one bit each for Red, Green and Blue, and one bit for flashing.
The flash bit operates as a toggle: when set for the first time, it freezes the background colour at the value set by R, G and B,
and starts flashing at the next bit in the line; when set for the second time, it stops flashing.
Flashing is always cleared at the beginning of a raster line.


Addressing for display memory starts at the bottom of dynamic RAM and progresses in the order of the raster
scan - from left to right and from top to bottom of the picture. Each word in display memory is formatted as follows:

High byte (A0=0)						Low Byte (A0=1)						Mode
D7 D6 D5 D4 D3 D2 D1 D0			D7 D6 D5 D4 D3 D2 D1 D0
G7 G6 G5 G4 G3 G2 G1 G0			R7 R6 R5 R4 R3 R2 R1 R0		512-pixel
G3 F3 G2 F2 G1 F1 G0 F0			R3 B3 R2 B2 R1 B1 R0 B0		256-pixel


R, G, Band F in the above refer to Red, Green, Blue and Flash. The numbering is such that a binary
word appears written as it will appear on the display: ie R0 is the value of Red for the rightmost pixel,
that is the last pixel to be shifted out onto the raster.
10.3 Display Control Register
This is a write-only register, which is at $18063 in the QL .
One of its bits is available through the Qdos MT.DMODE trap: bit 3, which is 0 for 512-pixel mode and 1 for 256-pixel mode.
The other two bits of the display control register are not supported by Qdos, these being bit 1 of the display
control register, which can be used to blank the display completely, and bit 7, which can be used to switch the base of
screen memory from $20000 to $28000. Future versions of Qdos may allow the system variables to be
initialised at $30000 to take advantage of this dual- screen feature: the present version does not.
Bits 0,2,4,5 and 6 of the display control register should never be set to anything other than zero, as they are
reserved and may have unpredictable results in future versions of the QL hardware.
*/
            //En modo 256x256 hay parpadeo


            byte_leido_h=*memoria_pantalla_ql;
            memoria_pantalla_ql++;

            byte_leido_l=*memoria_pantalla_ql;
            memoria_pantalla_ql++;

            if (video_mode==1) {

                int npixel;
                for (npixel=7;npixel>=0;npixel-=2) {

                    //G3 F3 G2 F2 G1 F1 G0 F0                 R3 B3 R2 B2 R1 B1 R0 B0         256-pixel

                    
                    green=((byte_leido_h)>>npixel)&1;
                    red=((byte_leido_l)>>npixel)&1;
                    blue=((byte_leido_l)>>(npixel-1))&1;
                                                                        

/*
//colores para QL
const int ql_colortable_original[8]={
0x000000, //Negro
0x0000ff, //Azul
0xff0000, //Rojo
0xff00ff, //Magenta
0x00ff00, //Verde
0x00ffff, //Cyan
0xffff00, //Amarillo
0xffffff  //Blanco
};
*/

                    color1=green*4+red*2+blue;	// GRB
                    //printf ("estado parpadeo: %d\n",estado_parpadeo.v);

                    if (ql_linea_flashing && estado_parpadeo.v) {
                        color1=flashing_color;
                    }

          			ql_putpixel_zoom(x++,y*2,color1);
          			ql_putpixel_zoom(x++,y*2,color1);

                    //Ver si cambia valor bit flash
                    int bit_flashing=((byte_leido_h)>>(npixel-1))&1;
                    if (bit_flashing) {
                        ql_linea_flashing ^=1;
                        flashing_color=color1;
                    }

                }



            }

            //Al arrancar, esta mc_stat=2A=0010 1010 -> modo 4 colores
            //512x256. 4 colours per pixel (2 bits per byte)


            if (video_mode==0) {

                int npixel;

                for (npixel=7;npixel>=0;npixel--) {
                    //G7 G6 G5 G4 G3 G2 G1 G0			R7 R6 R5 R4 R3 R2 R1 R0		512-pixel

                    
                    green=((byte_leido_h))&128;
                    red=((byte_leido_l))&128;

                    byte_leido_h=byte_leido_h<<1;
                    byte_leido_l=byte_leido_l<<1;

                    if (green==0 && red==0) color1=0;
                    else if (green && red==0) color1=4;
                    else if (green==0 && red) color1=2;
                    else color1=7;

                    ql_putpixel_zoom(x++,y*2,color1);

                }

            }

        }
    }
}


void scr_refresca_pantalla_y_border_ql(void)
{

        //Refrescar border si conviene
        if (border_enabled.v) {
                if (modificado_border.v) {
                        //Dibujar border. Color 0
                        unsigned int color=0;
                        //color=sam_palette_table[color];
                        //color +=SAM_INDEX_FIRST_COLOR;

                        scr_refresca_border_ql(color);
                        modificado_border.v=0;
                }

        }


        scr_refresca_pantalla_ql();
}



//Tamanyo pantalla emulada sin contar border
int screen_get_emulated_display_width_no_zoom(void)
{

        if (MACHINE_IS_Z88) {
        return SCREEN_Z88_WIDTH;
        }

	else if (MACHINE_IS_CPC) {
        return CPC_DISPLAY_WIDTH+CPC_LEFT_BORDER_NO_ZOOM*2;
	}

	else if (MACHINE_IS_PRISM) {
        return PRISM_DISPLAY_WIDTH+PRISM_LEFT_BORDER_NO_ZOOM*2;
	}

	else if (MACHINE_IS_TSCONF) {
        return TSCONF_DISPLAY_WIDTH;
	}

	else if (MACHINE_IS_TBBLUE) {
        return TBBLUE_DISPLAY_WIDTH+TBBLUE_LEFT_BORDER_NO_ZOOM*2;
	}	

        else if (MACHINE_IS_SAM) {
        return SAM_DISPLAY_WIDTH+SAM_LEFT_BORDER_NO_ZOOM*2;
        }

				else if (MACHINE_IS_QL) {
        return QL_DISPLAY_WIDTH+QL_LEFT_BORDER_NO_ZOOM*2;
        }

        else {
        return SCREEN_SPECTRUM_WIDTH;
        }
}

//Tamanyo pantalla emulada sin contar border
int screen_get_emulated_display_height_no_zoom(void)
{

        if (MACHINE_IS_Z88) {
        return SCREEN_Z88_HEIGHT;
        }

	else if (MACHINE_IS_CPC) {
        return CPC_DISPLAY_HEIGHT+CPC_TOP_BORDER_NO_ZOOM*2;
	}

	else if (MACHINE_IS_PRISM) {
        return PRISM_DISPLAY_HEIGHT+PRISM_TOP_BORDER_NO_ZOOM*2;
	}

	else if (MACHINE_IS_TSCONF) {
        return TSCONF_DISPLAY_HEIGHT;
	}

	else if (MACHINE_IS_TBBLUE) {
        return TBBLUE_DISPLAY_HEIGHT+TBBLUE_TOP_BORDER_NO_ZOOM+TBBLUE_BOTTOM_BORDER_NO_ZOOM;
	}

        else if (MACHINE_IS_SAM) {
        return SAM_DISPLAY_HEIGHT+SAM_TOP_BORDER_NO_ZOOM*2;
        }

				else if (MACHINE_IS_QL) {
        return QL_DISPLAY_HEIGHT+QL_TOP_BORDER_NO_ZOOM*2;
        }

				else if (MACHINE_IS_ZX8081ACE) {
					return ALTO_PANTALLA+ZX8081ACE_TOP_BORDER_NO_ZOOM+BOTTOM_BORDER_NO_ZOOM;
				}

        else {
        return SCREEN_SPECTRUM_HEIGHT;
        }
}


//Tamanyo pantalla emulada contando border
int screen_get_emulated_display_width_no_zoom_border_en(void)
{

        if (MACHINE_IS_Z88) {
        return SCREEN_Z88_WIDTH;
        }

	else if (MACHINE_IS_CPC) {
	return CPC_DISPLAY_WIDTH+(CPC_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v;
	}

	else if (MACHINE_IS_PRISM) {
	return PRISM_DISPLAY_WIDTH+(PRISM_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v;
	}

	else if (MACHINE_IS_TSCONF) {
	return TSCONF_DISPLAY_WIDTH;
	}

	else if (MACHINE_IS_TBBLUE) {
	return TBBLUE_DISPLAY_WIDTH+(TBBLUE_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v;
	}	

        else if (MACHINE_IS_SAM) {
        return SAM_DISPLAY_WIDTH+(SAM_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v;
        }

				else if (MACHINE_IS_QL) {
				return QL_DISPLAY_WIDTH+(QL_LEFT_BORDER_NO_ZOOM*2)*border_enabled.v;
				}


        else {
        return ANCHO_PANTALLA+(LEFT_BORDER_NO_ZOOM+RIGHT_BORDER_NO_ZOOM)*border_enabled.v;
	}
}

//Tamanyo pantalla emulada contando pantalla + borde inferior - usado en scr*putchar_footer
//Como la funcion screen_get_emulated_display_height_no_zoom_border_en pero sin contar borde superior
int screen_get_emulated_display_height_no_zoom_bottomborder_en(void)
{

        if (MACHINE_IS_Z88) {
        return SCREEN_Z88_HEIGHT;
        }

        else if (MACHINE_IS_CPC) {
        return CPC_DISPLAY_HEIGHT+(CPC_TOP_BORDER_NO_ZOOM)*border_enabled.v;
        }

        else if (MACHINE_IS_PRISM) {
        return PRISM_DISPLAY_HEIGHT+(PRISM_TOP_BORDER_NO_ZOOM)*border_enabled.v;
        }

	else if (MACHINE_IS_TSCONF) {
        return TSCONF_DISPLAY_HEIGHT;
        }

	else if (MACHINE_IS_TBBLUE) {
        return TBBLUE_DISPLAY_HEIGHT+(TBBLUE_BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;
        }				

        else if (MACHINE_IS_SAM) {
        return SAM_DISPLAY_HEIGHT+(SAM_TOP_BORDER_NO_ZOOM)*border_enabled.v;
        }

        else if (MACHINE_IS_QL) {
        return QL_DISPLAY_HEIGHT+(QL_TOP_BORDER_NO_ZOOM)*border_enabled.v;
        }



        else {
        return ALTO_PANTALLA+(BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;
        }
}



//Tamanyo pantalla emulada contando border
int screen_get_emulated_display_height_no_zoom_border_en(void)
{

        if (MACHINE_IS_Z88) {
        return SCREEN_Z88_HEIGHT;
        }

	else if (MACHINE_IS_CPC) {
	return CPC_DISPLAY_HEIGHT+(CPC_TOP_BORDER_NO_ZOOM*2)*border_enabled.v;
	}

	else if (MACHINE_IS_PRISM) {
	return PRISM_DISPLAY_HEIGHT+(PRISM_TOP_BORDER_NO_ZOOM*2)*border_enabled.v;
	}

	else if (MACHINE_IS_TSCONF) {
	return TSCONF_DISPLAY_HEIGHT;
	}

	else if (MACHINE_IS_TBBLUE) {
	return TBBLUE_DISPLAY_HEIGHT+(TBBLUE_TOP_BORDER_NO_ZOOM+TBBLUE_BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;
	}	

        else if (MACHINE_IS_SAM) {
        return SAM_DISPLAY_HEIGHT+(SAM_TOP_BORDER_NO_ZOOM*2)*border_enabled.v;
        }

				else if (MACHINE_IS_QL) {
				return QL_DISPLAY_HEIGHT+(QL_TOP_BORDER_NO_ZOOM*2)*border_enabled.v;
				}

				else if (MACHINE_IS_ZX8081ACE) {
        return ALTO_PANTALLA+(ZX8081ACE_TOP_BORDER_NO_ZOOM+BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;
				}

        else {
        return ALTO_PANTALLA+(TOP_BORDER_NO_ZOOM+BOTTOM_BORDER_NO_ZOOM)*border_enabled.v;
				}
}



//Tamanyo pantalla emulada sin contar border, y multiplicando por zoom
int screen_get_emulated_display_width_zoom(void)
{
	return screen_get_emulated_display_width_no_zoom()*zoom_x;
}

//Tamanyo pantalla emulada sin contar border, y multiplicando por zoom
int screen_get_emulated_display_height_zoom(void)
{
	return screen_get_emulated_display_height_no_zoom()*zoom_y;
}


//Tamanyo pantalla emulada contando border y multiplicando por zoom
int screen_get_emulated_display_width_zoom_border_en(void)
{
        return screen_get_emulated_display_width_no_zoom_border_en()*zoom_x;
}

//Tamanyo pantalla emulada contando border y multiplicando por zoom
int screen_get_emulated_display_height_zoom_border_en(void)
{
        return screen_get_emulated_display_height_no_zoom_border_en()*zoom_y;
}



//Tamanyo ventana contando border y multiplicando por zoom
//Suele ser el mismo tamanyo de la pantalla emulada pero sumando el margen inferior
int screen_get_window_size_height_zoom_border_en(void)
{
	return screen_get_emulated_display_height_zoom_border_en()+WINDOW_FOOTER_SIZE*zoom_y;
}

//Tamanyo ventana contando border y multiplicando por zoom
//Suele ser el mismo tamanyo de la pantalla emulada pero sumando el margen inferior
int screen_get_window_size_width_zoom_border_en(void)
{
        return screen_get_emulated_display_width_zoom_border_en()+0;
}


int screen_get_window_size_height_no_zoom_border_en(void)
{
        return screen_get_emulated_display_height_no_zoom_border_en()+WINDOW_FOOTER_SIZE;
}


int screen_get_window_size_width_no_zoom_border_en(void)
{
        return screen_get_emulated_display_width_no_zoom_border_en()+0;
}

int screen_get_window_size_height_no_zoom(void)
{
	return screen_get_emulated_display_height_no_zoom()+WINDOW_FOOTER_SIZE;
}

int screen_get_window_size_width_no_zoom(void)
{
        return screen_get_emulated_display_width_no_zoom()+0;
}




void disable_gigascreen(void)
{
	debug_printf (VERBOSE_INFO,"Disable gigascreen");
	gigascreen_enabled.v=0;
}

void enable_gigascreen(void)
{
	debug_printf (VERBOSE_INFO,"Enable gigascreen");
	if (gigascreen_enabled.v==0) {
		screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling Gigascreen mode");
	}

	gigascreen_enabled.v=1;

	//son excluyentes
	disable_interlace();
	disable_scanlines();
	disable_ulaplus();
	spectra_disable();

	//necesita real video
	enable_rainbow();

}

void disable_scanlines(void)
{
	debug_printf (VERBOSE_INFO,"Disable scanlines");
	video_interlaced_scanlines.v=0;
}

void enable_scanlines(void)
{
	debug_printf (VERBOSE_INFO,"Enable scanlines");
	video_interlaced_scanlines.v=1;

	//son excluyentes
	disable_gigascreen();
	disable_ulaplus();
	spectra_disable();
}

void disable_interlace(void)
{
	debug_printf (VERBOSE_INFO,"Disable interlace");
	video_interlaced_mode.v=0;
	set_putpixel_zoom();
	clear_putpixel_cache();
}

void enable_interlace(void)
{

	debug_printf (VERBOSE_INFO,"Enable interlace");
	if (video_interlaced_mode.v==0) {
		screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling Interlace video mode");
	}

	//son excluyentes
	disable_gigascreen();
	//disable_ulaplus();
	//spectra_disable();

	//necesita real video
	enable_rainbow();

	int reinicia_ventana=0;

	//Si zoom y no es multiple de 2
	if ((zoom_y&1)!=0) reinicia_ventana=1;

  void (*previous_function)(void);
  int menu_antes;

        if (reinicia_ventana) {
	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando


	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);
		zoom_y=2;
		zoom_x=2;
	}

        video_interlaced_mode.v=1;

        if (reinicia_ventana) {
					screen_init_pantalla_and_others_and_realjoystick();
				}

        set_putpixel_zoom();

	interlaced_numero_frame=0;

	clear_putpixel_cache();


if (reinicia_ventana) {
					screen_restart_pantalla_restore_overlay(previous_function,menu_antes);					
				}	


}

//refresco de pantalla, 2 veces, para que cuando haya modo interlaced o gigascreen y multitask on, se dibujen los dos frames, el par y el impar
void all_interlace_scr_refresca_pantalla(void)
{
	//printf ("antes de scr_refresca_pantalla\n");
    scr_refresca_pantalla();
	//printf ("despues de scr_refresca_pantalla\n");
    if (video_interlaced_mode.v || gigascreen_enabled.v) {
      interlaced_numero_frame++;
			screen_switch_rainbow_buffer();
      scr_refresca_pantalla();
    }

	//Modo timex real necesita esto
	if (timex_video_emulation.v && (timex_mode_512192_real.v || timex_ugly_hack_enabled)) clear_putpixel_cache();

}



int get_gigascreen_color(int c0,int c1)
{
	//return c0 ^ c1;
	int index=32+(c0*16)+c1;


	//if (index>=287) {
	//	printf ("index: %d value: %06X\n",index,spectrum_colortable_normal[index]);
	//}

	return index;
}


/*

Viejo metodo
int old_get_gigascreen_rgb_value(int c0,int c1,int i0,int i1)
{
//C = (C0 / 3 * 2 + C0 * I0 / 3 + C1 / 2 * 3 + C1 * I1 / 3) / 2, where
//C0, C1 - corresponding color (R, G, B) of 0 and 1 ekranki taking the values 0 or 1,
//I - Bright, 0 or 1.
//C - with the intensity of the resulting color in the range of 0-1, wherein 0 - the zero level of the video, 1 - max.
	int value=(c0 / 3 * 2 +  c0 * i0 / 3  +  c1 / 2 * 3  +  c1 * i1 / 3) / 2;


	//int value=(c0/2 + (c0/2)*i0    +    c1/2 + (c1/2)*i1  ) /2;

	//printf ("c0: %d c1: %d i0: %d i1:%d    value:%d\n",c0,c1,i0,i1,value);

	//valor llega a valer 289 en algunos casos
	if (value>255) value=255;
	//printf ("value corrected: %d\n",value);

	return value;
}

*/



//Nuevo metodo. Valor final es media de color entre los dos
/*
Con este metodo salen 102 colores diferentes. Porque?
Una primera aproximacion nos daria 15*14=210 colores (15 colores diferentes * 14 ->combinacion sin repetir)
Pero vemos por ejemplo:

0x000000,  //negro
0x0000CD,  //azul
0xCD0000,  //rojo
0xCD00CD,  //magenta

Vemos que sumar negro y magenta y dividir entre dos da:
(0x000000 + 0xCD00CD)/2 = 0x660066

Y sumar azul y rojo da:
(0x0000CD + 0xCD0000)/2 = 0x660066

Da el mismo color resultante. Asi hay varios casos mas, de ahi que se reduzca el numero total de combinaciones

*/
int get_gigascreen_rgb_value(int c0,int c1)
{
	int value=(c0+c1)/2;

	//printf ("c0: %d c1: %d  value: %d\n",c0,c1,value);
	//printf ("%X\n",value);

	if (value>255) value=255;
	return value;
}




void screen_switch_rainbow_buffer(void)
{
        //conmutar para gigascreen
        if (rainbow_buffer==rainbow_buffer_one) rainbow_buffer=rainbow_buffer_two;
        else rainbow_buffer=rainbow_buffer_one;
}


//devuelve sprite caracter de posicion rainbow
//0,0 -> inicio rainbow
//El sprite de 8x8 en posicion x,y es guardado en direccion *caracter
//Usado por stdout y curses en modo zx81 con realvideo
void screen_get_sprite_char(int x,int y,z80_byte *caracter)
{
    z80_int *origen_rainbow;

        origen_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow()+x ];
        //origen_rainbow +=screen_total_borde_izquierdo*border_enabled.v;


        //construimos bytes de origen a comparar

        int bit,j;
        z80_byte acumulado;
        z80_int leido;
        for (j=0;j<8;j++) {

                acumulado=0;

                for (bit=0;bit<8;bit++) {
                        acumulado=acumulado*2;
                        leido=*origen_rainbow;
			//Si color 0, bit a 1. Sino, bit a 0
                        if ( leido==0 ) acumulado |=1;

                        origen_rainbow++;
                }

                *caracter=acumulado;
                caracter++;

                //siguiente byte origen
                origen_rainbow=origen_rainbow+get_total_ancho_rainbow()-8;


        }
}



void screen_reset_putpixel_maxmin_y(void)
{
	putpixel_max_y=-1;
	putpixel_min_y=99999;
}







void screen_text_send_ansi_go_home(void)
{
        if (!screen_text_accept_ansi) return;
        printf ("\x1b[H");
}







void screen_text_repinta_pantalla_z88_print_char(struct s_z88_return_character_atributes *z88_caracter)
{

   //Caracteres ansi
                        if (screen_text_accept_ansi) {

                                //Primero establecemos texto normal
                                printf ("\x1b[0m");

                                //Gestion inverse
                                if (z88_caracter->inverse) {
                                        printf ("\x1b[7m");
                                }

                                //Gestion subrallado
                                if (z88_caracter->subrallado) {
                                        printf ("\x1b[4m");
                                }

                                //Gestion parpadeo
                                if (z88_caracter->parpadeo) {
                                        printf ("\x1b[5m");
                                }

                                //Gestion gris
                                if (z88_caracter->gris) {
                                        printf ("\x1b[2m");
                                }


                        }


                        //Si caracter no es nulo
                        if (z88_caracter->null_caracter==0) {
                                printf ("%c",z88_caracter->ascii_caracter);
                        }

}


void screen_text_set_normal_text(void)
{

       //establecemos texto normal
        if (screen_text_accept_ansi) printf ("\x1b[0m");

}

//black, blue, red , magenta, green, cyan, yellow, white
//Colores con brillo en ansi, para fg es + 60, en bg es +60
z80_byte colores_ansi_fg[8]={
30,34,31,35,32,36,33,37
};


z80_byte colores_ansi_bg[8]={
40,44,41,45,42,46,43,47
};


//Colores entre 0..15.
void screen_text_set_ansi_color_fg(z80_byte ink)
{
	z80_byte color_fg=colores_ansi_fg[ink&7];

	//Si hay brillo, y no es color negro (porque en spectrum el negro con brillo es negro igual)
	if (ink&8 && ink!=8) color_fg +=60;
	printf ("\x1b[%dm",color_fg);
}

//Colores entre 0..15
void screen_text_set_ansi_color_bg(z80_byte paper)
{
	z80_byte color_bg=colores_ansi_bg[paper&7];

	//Si hay brillo, y no es color negro (porque en spectrum el negro con brillo es negro igual)
	if (paper&8 && paper!=8) color_bg +=60;

	printf ("\x1b[%dm",color_bg);
}

char screen_text_return_color_border(void)
{

        z80_byte border_colour=out_254 & 7;

				if (screen_text_accept_ansi) {
					screen_text_set_ansi_color_bg(border_colour);
					return ' ';
				}

				else {

        	//Hacemos un calculo muy aproximado de trama de grises
        	//4 tonos diferentes, desde mas oscuro a mas claro : #:.
        	if (border_colour<=1) return '#';
        	if (border_colour<=3) return ':';
        	if (border_colour<=5) return '.';

        	return ' ';

				}

}


void screen_text_borde_horizontal_zx8081(void)
{

        if (border_enabled.v==0) return;

        int i,x;
        //borde blanco
        char color_borde=' ';

        for (i=0;i<SCREEN_TEXT_TOP_BORDER;i++) {
                for (x=0;x<32+SCREEN_TEXT_IZQ_BORDER*2;x++) {
                        printf ("%c",color_borde);
                }
                printf ("\n");
        }
}

void screen_text_borde_vertical_zx8081(void)
{
        if (border_enabled.v==0) return;

        int i;
        //borde blanco
        char color_borde=' ';

        for (i=0;i<SCREEN_TEXT_IZQ_BORDER;i++) {
                printf ("%c",color_borde);
        }
}


void screen_text_borde_horizontal(void)
{
        if (border_enabled.v==0) return;

        int i,x;
        char color_borde=screen_text_return_color_border();

        for (i=0;i<SCREEN_TEXT_TOP_BORDER;i++) {
                for (x=0;x<32+SCREEN_TEXT_IZQ_BORDER*2;x++) {
                        printf ("%c",color_borde);
                }
                printf ("\n");
        }
}

void screen_text_borde_vertical(void)
{
        if (border_enabled.v==0) return;

        int i;
        char color_borde=screen_text_return_color_border();

        for (i=0;i<SCREEN_TEXT_IZQ_BORDER;i++) {
                printf ("%c",color_borde);
        }
}



void screen_text_repinta_pantalla_zx81_no_rainbow_comun(int si_border,void (*puntero_printchar_caracter) (z80_byte),int solo_texto )
{

        if (si_border) screen_text_borde_horizontal_zx8081();
        if (si_border) screen_text_borde_vertical_zx8081();

        int x,y;
        z80_byte caracter;

        //zx81
        z80_int video_pointer;

        //puntero pantalla en DFILE
        video_pointer=peek_word_no_time(0x400C);


        //se supone que el primer byte es 118 . saltarlo
        video_pointer++;
        y=0;
        x=0;
        while (y<24) {

                caracter=memoria_spectrum[video_pointer++];
                if (caracter==118) {
                        //rellenar con espacios hasta final de linea
                                for (;x<32;x++) {
                                        //printf (" ");
					puntero_printchar_caracter(' ');
                                }
                                y++;

                                if (si_border) screen_text_borde_vertical_zx8081();

                                //printf ("\n");
				puntero_printchar_caracter('\n');

                                if (y<24 && si_border) screen_text_borde_vertical_zx8081();

                                x=0;
                }
                else {
                        z80_bit inverse;

			if (!solo_texto) caracter=da_codigo81(caracter,&inverse);
			else caracter=da_codigo81_solo_letras(caracter,&inverse);

                        //printf ("%c",caracter);
			puntero_printchar_caracter(caracter);

                        x++;

                        if (x==32) {
                                if (memoria_spectrum[video_pointer]!=118) {
                                        //debug_printf (VERBOSE_DEBUG,"End of line %d is not 118 opcode. Is: 0x%x",y,memoria_spectrum[video_pointer]);
								}
                                //saltamos el HALT que debe haber en el caso de linea con 32 caracteres
                                video_pointer++;
                                x=0;
                                y++;

                                if (si_border) screen_text_borde_vertical_zx8081();

                                //printf ("\n");
				puntero_printchar_caracter('\n');

                                if (y<24 && si_border) screen_text_borde_vertical_zx8081();
                        }

                }


    }

        if (si_border)  screen_text_borde_horizontal_zx8081();

}


//Rutina extraida de scrcurses
//void screen_text_repinta_pantalla_zx81_rainbow(void)
//void screen_text_repinta_pantalla_zx81_rainbow_comun(int si_border,void (*puntero_printchar_caracter) (z80_byte) )
void screen_text_repinta_pantalla_zx81_rainbow_comun(void (*puntero_printchar_caracter) (z80_byte) , int solo_texto)
{
       z80_byte caracter;
        int x,y;

        int valor_get_pixel;

        //char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";


        z80_int direccion;

        //Tabla de caracteres para ZX80,81
        if (MACHINE_IS_ZX80) direccion=0x0E00;
        else direccion=0x1E00;

        z80_byte inverse;

        //Nuestro caracter en pantalla a comparar con tabla de caracteres
        z80_byte caracter_sprite[8];

        //Alto de pantalla total en stdout
        int alto=24;
        alto=alto+2*SCREEN_TEXT_TOP_BORDER*border_enabled.v;

        //Ancho de pantalla total en stdout
        int ancho=32;
        ancho=ancho+2*SCREEN_TEXT_IZQ_BORDER*border_enabled.v;

	//printf ("ancho: %d alto: %d\n",ancho,alto);

        //Posicion en pantalla rainbow / 8
        //0,0 indica inicio de rainbow
        int yinicial=0;
        int xinicial=0;

        //Nos situamos en el pequeño border definido para stdout
        yinicial=yinicial+screen_borde_superior*border_enabled.v/8-SCREEN_TEXT_TOP_BORDER*border_enabled.v;

        xinicial=xinicial+screen_total_borde_izquierdo*border_enabled.v/8-SCREEN_TEXT_IZQ_BORDER*border_enabled.v;
        //Posicion en pantalla stdout
        int xenstdout,yenstdout;

        for (y=yinicial,yenstdout=0;y<yinicial+alto;y++,yenstdout++) {
                for (x=xinicial,xenstdout=0;x<xinicial+ancho;x++,xenstdout++) {

                        int spritelin;
                        caracter=255;

                        //Buscar caracteres en posicion y...y+7
                                for (spritelin=0;spritelin<8 && caracter==255;spritelin++) {
                                        screen_get_sprite_char(x*8,y*8+spritelin,caracter_sprite);
                                        caracter=compare_char_tabla_rainbow(caracter_sprite,&inverse,&memoria_spectrum[direccion]);
                                }



                        if (caracter!=255) {

                                //printf ("hay caracter :%d ",caracter);
                                z80_bit inv;

                                //caso particular de bloque negro completamente, es el mismo caracter que el espacio, pero con inverse 1
                                //para no mostrar simplemente un espacio en blanco (ya que no tenemos color negro en stdout) mostramos #
                                //espacio es caracter 0
                                if (caracter==0 && inverse && !solo_texto) {
                                        caracter='#';
                                }


                                else {
                                	if (!solo_texto) caracter=da_codigo81(caracter,&inv);
					else caracter=da_codigo81_solo_letras(caracter,&inv);
                                }


                                //printf ("%c",caracter);
				puntero_printchar_caracter(caracter);
                        }
                      else {

			  if (!solo_texto) {
                                if (texto_artistico.v==1) {
                                        //si caracter desconocido, hacerlo un poco mas artistico
                                        valor_get_pixel=0;
                                        if (scr_get_4pixel_rainbow(x*8,y*8)>=umbral_arttext) valor_get_pixel+=1;
                                        if (scr_get_4pixel_rainbow(x*8+4,y*8)>=umbral_arttext) valor_get_pixel+=2;
                                        if (scr_get_4pixel_rainbow(x*8,y*8+4)>=umbral_arttext) valor_get_pixel+=4;
                                        if (scr_get_4pixel_rainbow(x*8+4,y*8+4)>=umbral_arttext) valor_get_pixel+=8;

                                        caracter=screen_common_caracteres_artisticos[valor_get_pixel];

                                        //attron(COLOR_PAIR(0+7*8+1));
                                 }

                                else caracter='?';
			 }
			 else caracter=' ';

                         //printf ("%c",caracter);
			 puntero_printchar_caracter(caracter);

                        }

                }
                //printf ("\n");
		puntero_printchar_caracter('\n');


          }
}





void screen_text_ansi_asigna_color_atributo(z80_byte atributo)
{

        int paper,ink;
        int copia_paper;


        ink = atributo & 7;
        paper = ( atributo >> 3 ) & 7;


        if (atributo & 128) {
                //hay parpadeo
                if (estado_parpadeo.v) {
                        //estado de inversion de color
                        copia_paper=paper;
                        paper=ink;
                        ink=copia_paper;
                }
        }

	//printf ("\x1b[H");
	if (atributo & 64) {
		ink +=8;
		paper +=8;
	}

	screen_text_set_ansi_color_fg(ink);
	screen_text_set_ansi_color_bg(paper);

}


//Asigna color al siguiente caracter, obteniendolo de la pantalla de spectrum
void screen_text_ansi_asigna_color (int x,int y)
{

	if (!screen_text_accept_ansi)  return;

        int offset;
        z80_byte atributo;

        offset=6144;

        offset = offset + y*32 ;

        offset = offset +x ;

//      printf ("%d ",offset);

	z80_byte *pan;

	pan=get_base_mem_pantalla();

        atributo=pan[offset];

        if (scr_refresca_sin_colores.v) atributo=56;

        screen_text_ansi_asigna_color_atributo(atributo);

}


//Parametros
//si_border: Si debe dibujar con border. Luego las funciones screen_text_borde_horizontal, etc no lo dibujan si border en general esta desactivado
//rutina puntero_printchar_caracter apunta a rutina de impresion de texto
//solo_texto: solo muestra texto normal, nada de ascii art ni ? si no se reconoce caracter
//scrscreen_text_screen: puntero a la direccion de pantalla
//no_ansi: no enviar caracteres ansi a consola
void screen_text_repinta_pantalla_spectrum_comun_addr(int si_border,void (*puntero_printchar_caracter) (z80_byte),int no_ansi,int solo_texto,z80_byte *scrscreen_text_screen)
{


    char caracter;
    int x,y;
    unsigned char inv;

    int valor_get_pixel;


    //Refresco en Spectrum
    //unsigned char *scrscreen_text_screen;
    //scrscreen_text_screen=get_base_mem_pantalla();

    if (si_border) screen_text_borde_horizontal();

    for (y=0;y<24;y++) {
        if (si_border) screen_text_borde_vertical();
        for (x=0;x<32;x++) {

			if (!no_ansi) {
                screen_text_ansi_asigna_color(x,y);
                //printf("ansii\n");
            }

            caracter=compare_char(&scrscreen_text_screen[  calcula_offset_screen(x,y)  ] , &inv);

            if (caracter) {

                //printf ("%c",caracter);
				puntero_printchar_caracter(caracter);

            }

            else {

				if (!solo_texto) {

                    inv=0;

                    if (texto_artistico.v==1) {

                        //si caracter desconocido, hacerlo un poco mas artistico
                        valor_get_pixel=0;
                        if (scr_get_4pixel_adr(x*8,y*8,scrscreen_text_screen)>=umbral_arttext) valor_get_pixel+=1;
                        if (scr_get_4pixel_adr(x*8+4,y*8,scrscreen_text_screen)>=umbral_arttext) valor_get_pixel+=2;
                        if (scr_get_4pixel_adr(x*8,y*8+4,scrscreen_text_screen)>=umbral_arttext) valor_get_pixel+=4;
                        if (scr_get_4pixel_adr(x*8+4,y*8+4,scrscreen_text_screen)>=umbral_arttext) valor_get_pixel+=8;

                        caracter=screen_common_caracteres_artisticos[valor_get_pixel];

                    }

                    else caracter='?';

				}

				//solo_texto. caracteres desconocidos son espacios
				else caracter=' ';



                //printf ("%c",caracter);
				puntero_printchar_caracter(caracter);


            }

        }
		if (!solo_texto) screen_text_set_normal_text();
        if (si_border) screen_text_borde_vertical();

		puntero_printchar_caracter('\n');

    }

    if (si_border) screen_text_borde_horizontal();


    if (!solo_texto) screen_text_set_normal_text();

}




//Parametros
//si_border: Si debe dibujar con border. Luego las funciones screen_text_borde_horizontal, etc no lo dibujan si border en general esta desactivado
//rutina puntero_printchar_caracter apunta a rutina de impresion de texto
//solo_texto: solo muestra texto normal, nada de ascii art ni ? si no se reconoce caracter
void screen_text_repinta_pantalla_spectrum_comun(int si_border,void (*puntero_printchar_caracter) (z80_byte),int solo_texto)
{

        //char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";

        char caracter;
        int x,y;
        unsigned char inv;

        int valor_get_pixel;
	//int brillo,parpadeo;

          //Refresco en Spectrum
          unsigned char *scrscreen_text_screen;
          scrscreen_text_screen=get_base_mem_pantalla();

        if (si_border) screen_text_borde_horizontal();

          for (y=0;y<24;y++) {
                if (si_border) screen_text_borde_vertical();
                for (x=0;x<32;x++) {



			if (!solo_texto) screen_text_ansi_asigna_color(x,y);

                        caracter=compare_char(&scrscreen_text_screen[  calcula_offset_screen(x,y)  ] , &inv);

                        if (caracter) {

                            //No mostrar caracter 127 (c), sustituirlo
                            if (caracter==127) {
                                caracter='C';
                            }

                                //printf ("%c",caracter);
				puntero_printchar_caracter(caracter);

                        }

                        else {

				if (!solo_texto) {

	                        	inv=0;

		                        if (texto_artistico.v==1) {

        			                //si caracter desconocido, hacerlo un poco mas artistico
                			        valor_get_pixel=0;
                        			if (scr_get_4pixel(x*8,y*8)>=umbral_arttext) valor_get_pixel+=1;
		                	        if (scr_get_4pixel(x*8+4,y*8)>=umbral_arttext) valor_get_pixel+=2;
        		                	if (scr_get_4pixel(x*8,y*8+4)>=umbral_arttext) valor_get_pixel+=4;
	                		        if (scr_get_4pixel(x*8+4,y*8+4)>=umbral_arttext) valor_get_pixel+=8;

		        	                caracter=screen_common_caracteres_artisticos[valor_get_pixel];

        		                }

                		        else caracter='?';

				}

				//solo_texto. caracteres desconocidos son espacios
				else caracter=' ';



                        	//printf ("%c",caracter);
				puntero_printchar_caracter(caracter);


                        }

                }
		if (!solo_texto) screen_text_set_normal_text();
                if (si_border) screen_text_borde_vertical();
                //printf ("\n");
		puntero_printchar_caracter('\n');

          }

        if (si_border) screen_text_borde_horizontal();


				if (!solo_texto) screen_text_set_normal_text();

}


//Printf normal para rutinas de repintado de pantalla de spectrum, zx80, etc...
void screen_text_repinta_pantalla_normal_printf (z80_byte c)
{
	printf ("%c",c);
}

void screen_text_repinta_pantalla_spectrum(void)
{
	screen_text_repinta_pantalla_spectrum_comun(1,screen_text_repinta_pantalla_normal_printf,0);
}


void screen_text_repinta_pantalla_chloe(void)
{

        z80_byte caracter;
        int x,y;
        //unsigned char inv;

        //int valor_get_pixel;

        //int parpadeo;
				//int brillo;



        z80_byte *chloe_screen;

          chloe_screen=chloe_home_ram_mem_table[7];

        chloe_screen += (0xd800-0xc000); //text display in offset d800 in ram 7

          for (y=0;y<24;y++) {
                for (x=0;x<80;x++,chloe_screen++) {


                        caracter=*chloe_screen;

                        //brillo=0;
                        //inv=0;

                        /*

                        if (colores) {
                          asigna_color(x,y,&brillo,&parpadeo);
                        }
                        else {
                          brillo=0;
                        }
                        */

                        if (caracter==0) {
                                //caracter='C'; //copyright character
                                caracter=' '; //blank space until it is fixed in the rom
                        }

                        if (caracter<32 || caracter>126) caracter='?';


                                //move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);
				printf ("%c",caracter);




				//if (inv) addch(caracter | WA_REVERSE | brillo );
                                //else addch(caracter|brillo);

                }

		printf ("\n");

          }

}



void screen_text_refresca_pantalla_sam_modo_013_fun_color(z80_byte color GCC_UNUSED, int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{

/* No hacer nada
                                if (colores) {
                                  asigna_color_atributo(color,brillo,parpadeo);
                                }
                                else {
                                  *brillo=0;
                                }
*/

}


//Para saber cuando hay salto de linea
int screen_text_refresca_pantalla_sam_modo_013_last_y=-1;
void screen_text_refresca_pantalla_sam_modo_013_fun_caracter(int x GCC_UNUSED,int y,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{

	if (y!=screen_text_refresca_pantalla_sam_modo_013_last_y) printf ("\n");
	printf ("%c",caracter);

	screen_text_refresca_pantalla_sam_modo_013_last_y=y;
	/*
                       move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);
		*/

}



void screen_text_refresca_pantalla_sam_modo_2(void)
{
        scr_refresca_pantalla_sam_modo_2(screen_text_refresca_pantalla_sam_modo_013_fun_color,screen_text_refresca_pantalla_sam_modo_013_fun_caracter);
}

void screen_text_refresca_pantalla_sam_modo_013(int modo)
{
        scr_refresca_pantalla_sam_modo_013(modo,screen_text_refresca_pantalla_sam_modo_013_fun_color,screen_text_refresca_pantalla_sam_modo_013_fun_caracter);
}


void screen_text_repinta_pantalla_sam(void)
{
        z80_byte modo_video=(sam_vmpr>>5)&3;

                         if (border_enabled.v) {
                        //ver si hay que refrescar border
                        if (modificado_border.v)
                        {		/*
                                if (modo_video!=2) {
                                        scrcurses_refresca_border_comun(sam_border&7);
                                }

                                else scrcurses_refresca_border_sam_mode2(sam_border&7);

                                modificado_border.v=0;
					*/
                        }

                }


        switch (modo_video) {
                case 0:
                        screen_text_refresca_pantalla_sam_modo_013(0);
                break;

                case 1:
                        screen_text_refresca_pantalla_sam_modo_013(1);
                break;

                case 2:
                        screen_text_refresca_pantalla_sam_modo_2();
                break;

                case 3:
                        screen_text_refresca_pantalla_sam_modo_013(3);
                break;
        }
}

void screen_text_refresca_pantalla_cpc_fun_color(z80_byte color GCC_UNUSED, int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{

/* No hacer nada

*/

}


void screen_text_refresca_pantalla_cpc_fun_saltolinea(void)
{

	printf ("\n");

}

void screen_text_refresca_pantalla_cpc_fun_caracter(int x GCC_UNUSED,int y GCC_UNUSED,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{

        printf ("%c",caracter);


}



void screen_text_repinta_pantalla_cpc(void)
{
                         if (border_enabled.v) {
                        //ver si hay que refrescar border
                        if (modificado_border.v)
                        {               /*
                                        */
                        }

                }

	scr_refresca_pantalla_cpc_text(screen_text_refresca_pantalla_cpc_fun_color,screen_text_refresca_pantalla_cpc_fun_caracter,screen_text_refresca_pantalla_cpc_fun_saltolinea);

}



void screen_text_repinta_pantalla_z88_new_line(struct s_z88_return_character_atributes *z88_caracter GCC_UNUSED)
{
        printf ("\n");
}


void screen_text_repinta_pantalla_z88(void)
{
        struct s_z88_return_character_atributes z88_caracter;

        z88_caracter.f_new_line=screen_text_repinta_pantalla_z88_new_line;
        z88_caracter.f_print_char=screen_text_repinta_pantalla_z88_print_char;

        screen_repinta_pantalla_z88(&z88_caracter);

        //Fin de repintar pantalla

        //establecemos texto normal ansi
        screen_text_set_normal_text();


        printf ("\n");

}


void screen_text_repinta_pantalla_zx81_comun(int si_border,void (*puntero_printchar_caracter) (z80_byte),int solo_texto )
{
        if (rainbow_enabled.v==0) {
                        //modo clasico. sin rainbow
                screen_text_repinta_pantalla_zx81_no_rainbow_comun(si_border,puntero_printchar_caracter,solo_texto);
        }

        else {
                screen_text_repinta_pantalla_zx81_rainbow_comun(puntero_printchar_caracter,solo_texto);
        }
}

void screen_text_repinta_pantalla_zx81(void)
{

	screen_text_repinta_pantalla_zx81_comun(1,screen_text_repinta_pantalla_normal_printf,0);
}


//A 1 si se ha recibido el caracter "1" de escape y se espera siguiente valor para saber longitud
//A 2 hay que ignorar longitud de caracter escape
int printchar_next_z88_escape_caracter=0;

int printchar_next_z88_escape_caracter_longitud=0;

//Pide direccion de printchar
//Esta rutina es comun para primer trap como para second o third
//Por tanto no tiene mucho sentido en maquinas de mas de 48k andar mirando que rom esta activa...
void screen_text_printchar_next(z80_byte caracter, void (*puntero_printchar_caracter) (z80_byte)  )
{


                if (MACHINE_IS_SPECTRUM_16_48) {

                                //maquina 16k, inves ,48k o tk
                                puntero_printchar_caracter(caracter);

                }

		if (MACHINE_IS_ACE) {
			//Ignorar bit 7
			puntero_printchar_caracter(caracter&127);
		}


		if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) {
			puntero_printchar_caracter(caracter&127);
                }

		if (MACHINE_IS_SAM) {
                        puntero_printchar_caracter(caracter&127);
                }


                 if (MACHINE_IS_SPECTRUM_128_P2)  {
			if (reg_pc!=16) puntero_printchar_caracter(caracter);
			else {
				//Si rutina de la rom, comprobamos que rom esta activa
        	                //maquina 128k. rom 1 mapeada
                	        if ((puerto_32765 & 16) ==16) puntero_printchar_caracter(caracter);
			}
                }


		if (MACHINE_IS_CHLOE_280SE) {
			if (reg_pc!=16) puntero_printchar_caracter(caracter);
			else {
				//Si rutina de la rom, comprobamos que rom esta activa
				if (chloe_type_memory_paged[0]==CHLOE_MEMORY_TYPE_ROM) {
					//Rom mapeada. Es la 1?
					if ((puerto_32765 & 16) ==16) puntero_printchar_caracter(caracter);
				}
			}
		}

		if (MACHINE_IS_CHLOE_140SE) {
			if (reg_pc!=16) puntero_printchar_caracter(caracter);
			else {
				//Si rutina de la rom, comprobamos que rom esta activa
				//Rom es la 1?
				if ((puerto_32765 & 16) ==16) puntero_printchar_caracter(caracter);
			}
                }


		if (MACHINE_IS_PRISM) {
			if (reg_pc!=16) puntero_printchar_caracter(caracter);

			else {
				//Si rutina de la rom, comprobamos que rom esta activa
				//PC Vale 16, pero no comprobamos ram porque no sabemos que tipo de maquina esta emulando ZX-Uno.
				//Comprobamos por instrucciones que hay en la RST 16
				//0010: C3 F2 15    JP 15F2


				if (peek_byte_no_time(reg_pc)==0xC3 &&
				  peek_byte_no_time(reg_pc+1)==0xF2 &&
				  peek_byte_no_time(reg_pc+2)==0x15) {
					puntero_printchar_caracter(caracter);
				}
			}
		}

		if (MACHINE_IS_TBBLUE) {
                        if (reg_pc!=16) puntero_printchar_caracter(caracter);

                        else {
                                //Si rutina de la rom, comprobamos que rom esta activa
                                //PC Vale 16, pero no comprobamos ram porque no sabemos que tipo de maquina esta emulando ZX-Uno.
                                //Comprobamos por instrucciones que hay en la RST 16
                                //0010: C3 F2 15    JP 15F2


                                if (peek_byte_no_time(reg_pc)==0xC3 &&
                                  peek_byte_no_time(reg_pc+1)==0xF2 &&
                                  peek_byte_no_time(reg_pc+2)==0x15) {
                                        puntero_printchar_caracter(caracter);
                                }
                        }
                }



		if (MACHINE_IS_TIMEX_TS2068) {
			if (reg_pc!=16) puntero_printchar_caracter(caracter);

			else {
				//Si rutina de la rom, comprobamos que rom esta activa
				if (timex_type_memory_paged[0]==CHLOE_MEMORY_TYPE_ROM) puntero_printchar_caracter(caracter);
			}
		}



                if (MACHINE_IS_SPECTRUM_P2A_P3) {
			if (reg_pc!=16) puntero_printchar_caracter(caracter);

			else {
				//Si rutina de la rom, comprobamos que rom esta activa
                                //maquina +2A
                                if ((puerto_32765 & 16) ==16   && ((puerto_8189&4) ==4  )) puntero_printchar_caracter(caracter);
			}
                }

                if (MACHINE_IS_ZX8081) {
                        //printf ("(%x)",caracter);
                        if (caracter==118) {
                                //printf ("salto linea\n");
                                puntero_printchar_caracter(13);
                        }

                        else {
                                z80_bit inverse;
                                z80_byte c=da_codigo81(caracter,&inverse);
                                //printf ("zx81\n");
                                puntero_printchar_caracter(c);
                        }
                }

		if (MACHINE_IS_ZXUNO) {
			if (reg_pc!=16) puntero_printchar_caracter(caracter);

			else {
				//Si rutina de la rom, comprobamos que rom esta activa


				//PC Vale 16, pero no comprobamos ram porque no sabemos que tipo de maquina esta emulando ZX-Uno.
				//Comprobamos por instrucciones que hay en la RST 16
				//0010: C3 F2 15    JP 15F2


				if (peek_byte_no_time(reg_pc)==0xC3 &&
				  peek_byte_no_time(reg_pc+1)==0xF2 &&
				  peek_byte_no_time(reg_pc+2)==0x15) {
					puntero_printchar_caracter(caracter);
				}


				//Para rutinas de la bios. buscamos pos0, pos1, pos2 o pos3:
				/*
				pos0    ld      a, (ix)
				->      inc     ix
				        add     a, a
				        jr      z, posf
				*/

				if (peek_byte_no_time(reg_pc)==0xDD &&
                	          peek_byte_no_time(reg_pc+1)==0x23) {
                        	        puntero_printchar_caracter(caracter);
	                        }
			}


		}



                if (MACHINE_IS_Z88) {
                        if (printchar_next_z88_escape_caracter==1) {
                                //Ver longitud
/*
Escape Sequences
The characters SOH ($01) is used as an escape character to prefix the special character combinations. It may be followed either by a single special character, or if more than one parameter is to follow, by a count byte which specifies the number of following parameters - this may be in the form of a binary number with the top bit set, ie. count plus 128, or an ASCII code for a single decimal digit - clearly the latter is impossible for a count of more than 9. Thus the following sequences are equivalent - both move the cursor to character position (x,y):
1, '3', '@', 32+x, 32+y 1, 128+3, '@', 32+x, 32+y
*/
                                printchar_next_z88_escape_caracter=2;

                                if (caracter>=128) printchar_next_z88_escape_caracter_longitud=(caracter-128);
                                else if (caracter>='0' && caracter<='9') printchar_next_z88_escape_caracter_longitud=caracter-'0';


                                else {
                                        //Solo ocupa 1 (este byte)
                                        //fin sentencia ESC
                                        printchar_next_z88_escape_caracter=0;

                                        //Algunos conocidos

                                        //square
                                        if (caracter==0x2a) puntero_printchar_caracter('S');
                                        //diamond
                                        else if (caracter==0x2b) puntero_printchar_caracter('D');
                                        //vertical unbroken bar
                                        else if (caracter==0x7d) puntero_printchar_caracter('|');
                                }
                                return;
                        }

                        if (printchar_next_z88_escape_caracter==2) {
                                printchar_next_z88_escape_caracter_longitud--;
                                if (printchar_next_z88_escape_caracter_longitud<=0) printchar_next_z88_escape_caracter=0;
                                return;
                        }

                        if (caracter==1) {
                        printchar_next_z88_escape_caracter=1;
                                return;
                        }

                        puntero_printchar_caracter(caracter);
                }

}

//Pide direccion de printchar
void screen_text_printchar(void (*puntero_printchar_caracter) (z80_byte) )
{

		//if (stdout_simpletext_trap_rst16.v) {

			if (MACHINE_IS_Z88) {
				////RST 20H, DEFB 27H
				if (peek_byte_no_time(reg_pc)==0xE7) {

					//DEFB 27H. OS_OUT
					if (peek_byte_no_time(reg_pc+1)==0x27) screen_text_printchar_next(reg_a,puntero_printchar_caracter);

					//DEFW 2E09H. New line
					else if (peek_byte_no_time(reg_pc+1)==0x09 && peek_byte_no_time(reg_pc+2)==0x2e) {
						screen_text_printchar_next(13,puntero_printchar_caracter);
					}

					//DEFW 3A09H. GN_SOP. Imprimir desde HL hasta codigo 0
					else if (peek_byte_no_time(reg_pc+1)==0x09 && peek_byte_no_time(reg_pc+2)==0x3a) {
						z80_int temp_reg=reg_hl;
						while (peek_byte_no_time(temp_reg)!=0) {
							screen_text_printchar_next(peek_byte_no_time(temp_reg++),puntero_printchar_caracter);
						}
					}

					//DEFW 3C09H. GN_SOE. Imprimir desde BHL hasta codigo 0. B es codigo de banco
					else if (peek_byte_no_time(reg_pc+1)==0x09 && peek_byte_no_time(reg_pc+2)==0x3c) {
						z80_int temp_reg=reg_hl;
						while (peek_byte_no_time_z88_bank_no_check_low(temp_reg,reg_b)!=0) {
							screen_text_printchar_next( peek_byte_no_time_z88_bank_no_check_low(temp_reg++,reg_b),puntero_printchar_caracter  );
						}
					}
					//DEFB 90H. OS_BOUT. Imprimir desde BHL hasta codigo 0. B es codigo de banco
					else if (peek_byte_no_time(reg_pc+1)==0x90 ) {
						z80_int temp_reg=reg_hl;
						while (peek_byte_no_time_z88_bank_no_check_low(temp_reg,reg_b)!=0) {
							screen_text_printchar_next( peek_byte_no_time_z88_bank_no_check_low(temp_reg++,reg_b),puntero_printchar_caracter  );
						}
					}


					//DEFB 93H. OS_POUT Imprimir desde PC hasta codigo 0
					else if (peek_byte_no_time(reg_pc+1)==0x93) {
						z80_int temp_reg=reg_pc+2;
						while (peek_byte_no_time(temp_reg)!=0) {
							screen_text_printchar_next(peek_byte_no_time(temp_reg++),puntero_printchar_caracter);
						}
					}


				}
			}

			else {

				//En ZX80 las llamadas a RST 16 solo son para el cursor
				//El resto de llamadas, incluso rst16, van por la direccion 0x0560
				if (MACHINE_IS_ZX80) {
					if (reg_pc==0x0560) screen_text_printchar_next(reg_a,puntero_printchar_caracter);
					return;
				}


				//Para Jupiter Ace se hace con rst 8
				if (MACHINE_IS_ACE) {
					if (reg_pc==8) {
						screen_text_printchar_next(reg_a,puntero_printchar_caracter);
					}
                                        return;
                                }

				//Para Amstrad cpc 464
				if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) {
					if (reg_pc==0xBB5A) {
                                                screen_text_printchar_next(reg_a,puntero_printchar_caracter);
                                        }
                                        return;
                                }

				//Para Sam Coupe
				if (MACHINE_IS_SAM) {
					if (reg_pc==16) {
                                                screen_text_printchar_next(reg_a,puntero_printchar_caracter);
                                        }
                                        return;
                                }





				//Para Spectrum, ZX81, ZX-Uno
				if (reg_pc==16) {
					screen_text_printchar_next(reg_a,puntero_printchar_caracter);
					return;
				}


				//Si maquina ZX-Uno, para print de la bios. Estas direcciones estan puestas a pelo, si cambian
				//esas direcciones, no funcionara
				if (MACHINE_IS_ZXUNO && (
					reg_pc==0xad64 || reg_pc==0xad82 || reg_pc==0xadaf || reg_pc==0xaddc)
					)
				{
					screen_text_printchar_next(reg_a,puntero_printchar_caracter);
					return;
				}
			}
		//}
}


void disable_timex_video(void)
{
	debug_printf (VERBOSE_DEBUG,"Disabling Timex Video");
	timex_video_emulation.v=0;
}

void enable_timex_video(void)
{
	//Si no esta activo y si maquina es spectrum
	if (timex_video_emulation.v==0 && MACHINE_IS_SPECTRUM) {
		debug_printf (VERBOSE_DEBUG,"Enabling Timex Video");
		//es excluyente
		spectra_disable();

		//necesita real video
		enable_rainbow();

		timex_video_emulation.v=1;
	}
}

/*
                                if (colores) {
                                  asigna_color_atributo(atributo_spectrum,&brillo,&parpadeo);
                                }
                                else {
                                  brillo=0;
                                }

*/

/*

                                move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);

*/

//Para convertir sprite 8x8 en artistico
//Contar bits a 1 en 1 byte
int scr_artistic_count_bits_byte(z80_byte valor)
{
	int i;
	int resultado=0;


	for (i=0;i<8;i++) {
		if (valor&128) resultado++;

		//rotar
		valor = valor << 1;
	}
	return resultado;
}

//Contar cuantos bits a 1 en 4 bytes
int scr_artistic_count_bits_4_bytes(z80_byte *origen)
{
	int resultado=0;
	int i;

	for (i=0;i<4;i++) {
		resultado+=scr_artistic_count_bits_byte(*origen);

		origen++;
	}

	return resultado;
}

//copiar 8 bytes desde origen a destino saltando incremento_origen en origen
void scr_artistic_copy_8_line_sprite(z80_byte *origen, int incremento_origen, z80_byte *destino)
{
	int i;
	for (i=0;i<8;i++) {
		*destino=*origen;

		origen +=incremento_origen;
		destino++;
	}
}


//Retorna caracter artistico partiendo de sprite origen, saltando incremento_origen
char scr_artistic_retorna_artistic_char(z80_byte *origen, int incremento_origen)
{

	//TODO. meter esto como constante comun
	//char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";

	//Primero convertir sprite a array lineal
	z80_byte sprite_destino[8];

	scr_artistic_copy_8_line_sprite(origen,incremento_origen,sprite_destino);

	//Dividir sprite en 4 partes de 4x4


	//trozo 1: arriba, izquierda
	//trozo 2: arriba, derecha
	//trozo 3: abajo, izquierda
	//trozo 4: abajo, derecha

	//Mascara izquierda: 11110000b = 240
	z80_byte sprite_troceado[4][4];

	//Arriba izquierda
	sprite_troceado[0][0]=sprite_destino[0] & 240;
	sprite_troceado[0][1]=sprite_destino[1] & 240;
	sprite_troceado[0][2]=sprite_destino[2] & 240;
	sprite_troceado[0][3]=sprite_destino[3] & 240;

	//Arriba derecha
	sprite_troceado[1][0]=sprite_destino[0] & 15;
	sprite_troceado[1][1]=sprite_destino[1] & 15;
	sprite_troceado[1][2]=sprite_destino[2] & 15;
	sprite_troceado[1][3]=sprite_destino[3] & 15;

	//Arriba izquierda
	sprite_troceado[2][0]=sprite_destino[4] & 240;
	sprite_troceado[2][1]=sprite_destino[5] & 240;
	sprite_troceado[2][2]=sprite_destino[6] & 240;
	sprite_troceado[2][3]=sprite_destino[7] & 240;

	//Arriba derecha
	sprite_troceado[3][0]=sprite_destino[4] & 15;
	sprite_troceado[3][1]=sprite_destino[5] & 15;
	sprite_troceado[3][2]=sprite_destino[6] & 15;
	sprite_troceado[3][3]=sprite_destino[7] & 15;


	//Y sacar el conteo de bits de cada 4 partes

	int valor_get_pixel=0;



	if (scr_artistic_count_bits_4_bytes(sprite_troceado[0])>=umbral_arttext) valor_get_pixel+=1;
	if (scr_artistic_count_bits_4_bytes(sprite_troceado[1])>=umbral_arttext) valor_get_pixel+=2;
	if (scr_artistic_count_bits_4_bytes(sprite_troceado[2])>=umbral_arttext) valor_get_pixel+=4;
	if (scr_artistic_count_bits_4_bytes(sprite_troceado[3])>=umbral_arttext) valor_get_pixel+=8;

	char caracter;

	caracter=screen_common_caracteres_artisticos[valor_get_pixel];

	return caracter;
}



void scr_refresca_pantalla_sam_modo_013(int modo,void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) )
{

        char caracter;
        int x,y;
        unsigned char inv;

        //int valor_get_pixel;

        int brillo,parpadeo;



//          scrcurses_screen=get_base_mem_pantalla();


                                z80_byte buffer_letra[8];

        z80_byte segmento_pantalla=sam_vmpr&sam_memoria_total_mascara;
        //scrcurses_screen=sam_ram_memory[segmento_pantalla];

                                z80_byte atributo_spectrum;

        //Tabla caracteres sam coupe los he sacado de direccion 0x9190 de la ram de sam
        //en la rom no estan visibles, quiza estan encriptados???


z80_int offset=0;

//para caracter artistico
z80_byte *artistic_puntero_origen;
int artistic_incremento_origen;

          for (y=0;y<24;y++) {
                for (x=0;x<32;x++) {


                        //Organizacion como spectrum
                        if (modo==0) {
				z80_byte *puntero;
				puntero=sam_ram_memory[segmento_pantalla];




                                caracter=compare_char_tabla_step(&puntero[calcula_offset_screen(x,y)], &inv,char_set_sam,256);
				artistic_puntero_origen=&puntero[calcula_offset_screen(x,y)];
				artistic_incremento_origen=256;


				sam_retorna_byte_pantalla_spectrum(segmento_pantalla,x*16,y*8,&atributo_spectrum);

				fun_color(atributo_spectrum,&brillo,&parpadeo);


                        }

                        //Organizacion lineal
                        if (modo==1) {
                                //int off=y*256+x;

                                z80_byte *puntero;
                                puntero=sam_ram_memory[segmento_pantalla]+offset;

                                caracter=compare_char_tabla_step(puntero , &inv,char_set_sam,32);
				artistic_puntero_origen=puntero;
				artistic_incremento_origen=32;
                                sam_retorna_byte_pantalla_mode1(&segmento_pantalla,&offset,&atributo_spectrum);

				fun_color(atributo_spectrum,&brillo,&parpadeo);

                        }

                        //                                        case 3: //256x192. 16 colours per pixel (4 bits per pixel)
                        //cada linea ocupa 128 bytes
                        if (modo==3) {
                                z80_byte *puntero;
                                puntero=sam_ram_memory[segmento_pantalla]+offset;

                                //Convertir letra en colores a blanco y negro

				sam_convert_mode3_char_to_bw(puntero,buffer_letra,&atributo_spectrum);

                                caracter=compare_char_tabla_step(buffer_letra , &inv,char_set_sam,1);
				artistic_puntero_origen=buffer_letra;
				artistic_incremento_origen=1;

                                //Saltar 4 bytes
                                sam_retorna_byte_pantalla(&segmento_pantalla,&offset);
                                sam_retorna_byte_pantalla(&segmento_pantalla,&offset);
                                sam_retorna_byte_pantalla(&segmento_pantalla,&offset);
                                sam_retorna_byte_pantalla(&segmento_pantalla,&offset);

				fun_color(atributo_spectrum,&brillo,&parpadeo);


                        }




                        if (caracter) {
				fun_caracter(x,y,brillo,inv,caracter);
                        }
                          else {

                                inv=0;

				//Hagamos caracter artistico

				if (texto_artistico.v==1) {
					caracter=scr_artistic_retorna_artistic_char(artistic_puntero_origen,artistic_incremento_origen);
				}
				else {
	                                caracter='?';
				}

				fun_caracter(x,y,brillo,inv,caracter);


                        }

                }

                //Siguiente linea
                if (modo==1) {
                        int j;
                        //Incrementar puntero
                        for (j=0;j<8*32;j++) {
                                sam_retorna_byte_pantalla_mode1(&segmento_pantalla,&offset,&atributo_spectrum);
                        }
                }

                if (modo==3) {
                        int j;
                        //Incrementar puntero
                        for (j=0;j<8*4*32;j++) {
                                sam_retorna_byte_pantalla(&segmento_pantalla,&offset);
                        }
                }



          }

}


void scr_refresca_pantalla_sam_modo_2(void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo,
 unsigned char inv,z80_byte caracter ))
{

        char caracter;
        int x,y;
        unsigned char inv;

        //int valor_get_pixel;

        int brillo,parpadeo;



//          scrcurses_screen=get_base_mem_pantalla();


                                z80_byte buffer_letra[8];

        z80_byte segmento_pantalla=sam_vmpr&sam_memoria_total_mascara;
        //scrcurses_screen=sam_ram_memory[segmento_pantalla];

                                z80_byte atributo_spectrum;

        //Tabla caracteres sam coupe los he sacado de direccion 0x9190 de la ram de sam
        //en la rom no estan visibles, quiza estan encriptados???


z80_int offset=0;
z80_byte bit;

          for (y=0;y<24;y++) {
                bit=0;
                for (x=0;x<85;x++) {



                        //512x192. 4 colours per pixel (2 bits per pixel)
                        //cada linea ocupa 64 bytes

 //cada caracter ocupa 6 pixeles de ancho
                                z80_byte *puntero;
                                puntero=sam_ram_memory[segmento_pantalla]+offset;

                                //Convertir letra en colores a blanco y negro


                                sam_convert_mode2_char_to_bw(puntero,buffer_letra,&atributo_spectrum,bit);

                                caracter=compare_char_tabla_step(buffer_letra , &inv,char_set_sam,1);

                                //Saltar 1 caracter. Cada caracter ocupa 6 pixeles, que son 12 bits, que son 1 byte y medio
                                //Caracter   0   1   2   3   4   5   6   7   8
                                //Direccion  0   1   3   4   6   7   9  10
                                //Bit        0   4   0   4   0   4   0   4   0


                                //Saltar 1 byte
                                sam_retorna_byte_pantalla(&segmento_pantalla,&offset);

                                if (bit==4) {
                                        //Saltar otro byte
                                        sam_retorna_byte_pantalla(&segmento_pantalla,&offset);
                                }

                                bit ^=4;

                                fun_color(atributo_spectrum,&brillo,&parpadeo);


                        if (caracter) {

                                fun_caracter(x,y,brillo,inv,caracter);                        }
                          else {

                                inv=0;


                                  // De momento nada de caracter artistico


			 //Hagamos caracter artistico

                                if (texto_artistico.v==1) {
                                        caracter=scr_artistic_retorna_artistic_char(buffer_letra,1);
                                }
                                else {
                                        caracter='?';
                                }


                               fun_caracter(x,y,brillo,inv,caracter);

                        }


                }


  //Siguiente linea
                        int j;
                        //Incrementar puntero
                        for (j=0;j<8*4*32;j++) {
                                sam_retorna_byte_pantalla(&segmento_pantalla,&offset);
                        }

                        sam_retorna_byte_pantalla(&segmento_pantalla,&offset);



          }

}


void sam_convert_mode3_char_to_bw(z80_byte *origen,z80_byte *buffer_letra,z80_byte *atributo)
{
	//256x192. 16 colours per pixel (4 bits per pixel)
	//cada linea ocupa 128 bytes
	//NOTA: seguro que esta funcion se puede simpilificar y mucho....

	int linea;
	int pixel;
	//z80_byte color_resultante;
	z80_byte colorleft,colorright;

	//Bits finales. a -1 no inicializado
	int bitfinal_left,bitfinal_right;

	//Suponemos que solo hay dos colores en ese caracter. Los inicializamos a desconocido
	int inicial_0=-1;
	int inicial_1=-1;



	for (linea=0;linea<8;linea++) {
		//Inicializamos ese byte de letra a 0
		*buffer_letra=0;
		for (pixel=0;pixel<8;pixel+=2) {
			bitfinal_left=bitfinal_right=-1;

			colorleft=((*origen)>>4)&15;
			colorright=(*origen)&15;

			//Si los dos colores iniciales 0,1 estan desconocidos
			if (inicial_0==-1 && inicial_1==-1) {
				//Indicamos que el 0 sera el color de la izquierda
				inicial_0=colorleft;
			}

			//Si el 1 esta desconocido
			if (inicial_0!=-1 && inicial_1==-1) {
					//Sera el otro color, siempre que sea distinto del 0
					if (colorleft!=inicial_0) inicial_1=colorleft;
					else if (colorright!=inicial_0) inicial_1=colorright;
			}

			//Asignar 0 o 1
			if (inicial_0!=-1) {
				if (colorleft==inicial_0) bitfinal_left=0;
				if (colorright==inicial_0) bitfinal_right=0;
			}

			if (inicial_1!=-1) {
				if (colorleft==inicial_1) bitfinal_left=1;
				if (colorright==inicial_1) bitfinal_right=1;
			}

			//Y si los colores no coinciden con ninguno de los dos, asignarlos como 0 (a ver si hay suerte)
			if (bitfinal_left==-1) {
				//printf ("%d %d %d.",colorleft,inicial_0,inicial_1);
				bitfinal_left=0;
			}
			if (bitfinal_right==-1) {
				//printf ("%d %d %d.",colorright,inicial_0,inicial_1);
				bitfinal_right=0;
			}




			//Rotamos byte resultante dos a la izquierda
			(*buffer_letra)=(*buffer_letra)<<2;

			//Metemos esos dos bits
			*buffer_letra |= (bitfinal_left<<1)+bitfinal_right;

			origen++;


			//printf ("%d%d",colorleft,colorright);
		}

		origen +=(128-4);
		buffer_letra++;
		//printf ("\r\n");
	}

	//Retornar atributo en formato spectrum
	//Si solo ha habido un color
	//TODO. considerar colores de paleta, y no indexados como si fuesen siempre de spectrum
	//TODO. no se considera brillo
	z80_byte papel,tinta;
	if (inicial_1==-1) {
		papel=tinta=inicial_0&7;
	}

	else {
		papel=inicial_0&7;
		tinta=inicial_1&7;
	}


	*atributo=papel*8+tinta;
			//debug
			/*
			if (inicial_0!=-1 && inicial_1!=-1 && inicial_0!=inicial_1) {
				//Hacer debug del caracter
				buffer_letra -=8;
				int i,j;
				int bit;
				for (i=0;i<8;i++) {
					bit=*buffer_letra;
					for (j=0;j<8;j++) {
						if (bit&128) printf ("1");
						else printf ("0");
						bit=bit*2;
					}
					printf ("\n\r");
					buffer_letra++;
				}
			}
			*/

}


void sam_convert_mode2_char_to_bw(z80_byte *origen,z80_byte *buffer_letra,z80_byte *atributo,int bit)
{
        //512x192. 4 colours per pixel (2 bits per pixel)
        //cada linea ocupa 64 bytes
        //NOTA: seguro que esta funcion se puede simpilificar y mucho....

        int linea;
        int pixel;
        //z80_byte color_resultante;
        z80_byte colorleft;

	z80_byte byte_origen;
	int bit_actual;

	z80_byte *origen_copia;

	int bitfinal_left;

	int color_tinta,color_papel;
	color_tinta=color_papel=-1;

        for (linea=0;linea<8;linea++) {
                //Inicializamos ese byte de letra a 0
                *buffer_letra=0;
		byte_origen=*origen;
		byte_origen= byte_origen<<bit;
		bit_actual=bit;
		origen_copia=origen;
                for (pixel=0;pixel<6;pixel++) {
                        colorleft=(byte_origen>>6)&3;
			byte_origen=byte_origen<<2;

			//Deteccion colores
			if (color_papel==-1) color_papel=colorleft;

			if (color_tinta==-1) {
				if (colorleft!=color_papel) color_tinta=colorleft;
			}

			if (colorleft==color_tinta) bitfinal_left=1;
			else bitfinal_left=0;


                        //Rotamos byte resultante uno a la izquierda
                        (*buffer_letra)=(*buffer_letra)<<1;

                        //Metemos ese bit
                        *buffer_letra |= bitfinal_left;

			bit_actual+=2;
			if (bit_actual>=8) {
				bit_actual=0;
				origen++;
				byte_origen=*origen;
			}

//printf ("%d%d",colorleft,colorright);
                }

		//Rotar 1 bit a la izquierda para centrar el caracter
		(*buffer_letra)=(*buffer_letra)<<1;


                origen=origen_copia+128;
                buffer_letra++;
                //printf ("\r\n");
        }

        //Retornar atributo en formato spectrum
        //Si solo ha habido un color
        //TODO. considerar colores de paleta, y no indexados como si fuesen siempre de spectrum
        //TODO. no se considera brillo
        z80_byte papel,tinta;
        if (color_tinta==-1) {
                papel=tinta=color_papel&7;
        }

        else {
                papel=color_papel&7;
                tinta=color_tinta&7;
        }

	//En este modo, el color 3 representa el blanco (7). cambiar
	if (papel==3) papel=7;
	if (tinta==3) tinta=7;


        *atributo=papel*8+tinta;


}

//Da un valor de color segun:
//Puntero a inicio de linea
//bits por pixel: 1,2,4,8
//Coordenada x en pixel
//Si modo cpc, los colores para modos de mas de 1 bpp salen diferente de lo habitual
z80_byte scr_get_colour_byte(z80_byte *inicio_linea,int bpp, int x, int si_cpc)
{
	int pixeles_por_byte;
	z80_byte mascara;
	//Para silenciar al compilador
	pixeles_por_byte=0;
	mascara=0;

	switch (bpp) {
		case 1:
			pixeles_por_byte=8;
			mascara=1;
		break;

		case 2:
			pixeles_por_byte=4;
			mascara=3;
		break;

		case 4:
			pixeles_por_byte=2;
			mascara=15;
		break;

		case 8:
			pixeles_por_byte=1;
			mascara=255;
		break;

		default:
			cpu_panic("Invalid value bpp on scr_get_colour_byte");
		break;
	}

	//Resto
	int resto=x%pixeles_por_byte;

	//Situarnos en el byte correspondiente
	x=x/pixeles_por_byte;


	//Obtener byte
	z80_byte valor_leido=*(inicio_linea+x);

	//Rotar dentro del byte tanto como sea necesario de resto
	//1bpp:  C0 C1 C2 C3 C4 C5 C6 C7
	//2bpp:  C0 C0 C1 C1 C2 C2 C3 C3
	//4bpp:  C0 C0 C0 C0 C1 C1 C1 C1
	//8bpp:  C0 C0 C0 C0 C0 C0 C0 C0

	//Para CPC
	if (si_cpc && bpp>1) {
		switch (bpp) {
			case 2:
				if (resto==0) valor_leido=(valor_leido&128)>>7 | ((valor_leido&8)>>2);
				if (resto==1) valor_leido=(valor_leido&64)>>6 | ((valor_leido&4)>>1);
				if (resto==2) valor_leido=(valor_leido&32)>>5 | ((valor_leido&2));
				if (resto==3) valor_leido=(valor_leido&16)>>4 | ((valor_leido&1)<<1   );
			break;

			case 4:
				if (resto==0) valor_leido=(valor_leido&128)>>7 | (valor_leido&8)>>2 | (valor_leido&32)>>3 | (valor_leido&2)<<2;
				if (resto==1) valor_leido=(valor_leido&64)>>6  | (valor_leido&4)>>1 | (valor_leido&16)>>2 | (valor_leido&1)<<3;
			break;

		}
	}

	else {

		for (;resto<pixeles_por_byte-1;resto++) {
			//printf ("resto: %d valor_leido: %d\n",resto,valor_leido);
			valor_leido = valor_leido >> bpp;
		}
	}

	//printf ("resultado: %d\n",valor_leido&mascara);
	//sleep(2);

	//return (valor_leido&1);
	return (valor_leido&mascara);

}


//y en coordenadas de pixel (0..199)
//x en coordenadas de caracter (0..79)
void scr_refresca_pantalla_cpc_get_sprite_mode_x(int x,int y,z80_byte *destino,int bpp)
{

	z80_int direccion_pixel;

	z80_byte crtc_video_page=(cpc_crtc_registers[12]>>4)&3;

	int scanline;
	z80_byte *puntero;

	/* Bueno
	for (scanline=0;scanline<8;scanline++) {
		direccion_pixel=cpc_line_display_table[y*8+scanline]+x;

		puntero=cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383);

		*destino=*puntero;
		destino++;
	}
	*/

	int xx;
	z80_byte color_resultado;
	for (scanline=0;scanline<8;scanline++) {
                direccion_pixel=cpc_line_display_table[y*8+scanline];

                puntero=cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383);

		color_resultado=0;
		z80_byte pixel;
		for (xx=0;xx<8;xx++) {
			if (bpp==1) {
				color_resultado=color_resultado<<1;
				pixel=scr_get_colour_byte(puntero,1,x*8+xx,1);
				color_resultado |=pixel;
			}

			if (bpp==2) {
				color_resultado=color_resultado<<1;
				pixel=scr_get_colour_byte(puntero,2,x*8+xx,1);

				//debug
				//if (y==0 && x==2) printf ("%d ",pixel);

				if (pixel>0) pixel=1;
				color_resultado |=pixel;


			}


			if (bpp==4) {
                                color_resultado=color_resultado<<1;
                                pixel=scr_get_colour_byte(puntero,4,x*8+xx,1);
                                if (pixel>0) pixel=1;
                                color_resultado |=pixel;

                                //debug
                                //if (y==0 && x==0) printf ("%d ",pixel);

                        }
		}
		*destino=color_resultado;
                destino++;

		//if (y==0 && x==2) printf (" - ");
        }

	//debug
	//if (y==0 && x==2) printf (" I ");

}


void scr_refresca_pantalla_cpc_text(void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) , void (*fun_saltolinea) (void) )
{

        z80_byte modo_video=cpc_gate_registers[2] &3;

        if (cpc_forzar_modo_video.v) {
                modo_video=cpc_forzar_modo_video_modo;
        }

	int ancho_total;

	//z80_byte crtc_video_page=(cpc_crtc_registers[12]>>4)&3;

	//z80_int direccion_pixel;



	int alto_caracter=8;

        int yy;
        z80_int offset;
        for (yy=0;yy<200;yy++) {
                //offset=((yy / 8) * cpc_crtc_registers[1]*2) + ((yy % 8) * 2048);
                offset=((yy / alto_caracter) * cpc_crtc_registers[1]*2) + ((yy % alto_caracter) * 2048);
                cpc_line_display_table[yy]=offset;
        }




        switch (modo_video) {
                case 0:
                        //printf ("Mode 0, 160x200 resolution, 16 colours\n");
			ancho_total=20;
                break;

                case 1:
                        //printf ("Mode 1, 320x200 resolution, 4 colours\n");
			ancho_total=40;
                break;

                case 2:
                        //printf ("Mode 2, 640x200 resolution, 2 colours\n");
			ancho_total=80;
                break;

                case 3:
                        //printf ("Mode 3, 160x200 resolution, 4 colours (undocumented)\n");
			ancho_total=20;
                break;
        }



        char caracter;
        int x,y;
        unsigned char inv;


        int brillo,parpadeo;



                                z80_byte buffer_letra[8];


                                z80_byte atributo_spectrum;




	//para caracter artistico
	z80_byte *artistic_puntero_origen;
	int artistic_incremento_origen;


	//temp mientras no haya todos modos videos
	caracter='?';
	brillo=parpadeo=0;
	atributo_spectrum=56;

          for (y=0;y<25;y++) {

		//direccion_pixel=cpc_line_display_table[y*8];

                for (x=0;x<ancho_total;x++) {

			//printf ("Mode 0, 160x200 resolution, 16 colours\n");
			if (modo_video==0) {

                                scr_refresca_pantalla_cpc_get_sprite_mode_x(x,y,buffer_letra,4);

                                caracter=compare_char_tabla_step(buffer_letra, &inv,char_set_cpc,1);
                                artistic_puntero_origen=buffer_letra;
                                artistic_incremento_origen=1;

                                //De momento papel 7 tinta 0
                                brillo=0;
                                parpadeo=0;
                                atributo_spectrum=56;
                                fun_color(atributo_spectrum,&brillo,&parpadeo);

                        }


			//printf ("Mode 1, 320x200 resolution, 4 colours\n");
			if (modo_video==1) {

                                scr_refresca_pantalla_cpc_get_sprite_mode_x(x,y,buffer_letra,2);

                                caracter=compare_char_tabla_step(buffer_letra, &inv,char_set_cpc,1);
                                artistic_puntero_origen=buffer_letra;
                                artistic_incremento_origen=1;

                                //De momento papel 7 tinta 0
                                brillo=0;
                                parpadeo=0;
                                atributo_spectrum=56;
                                fun_color(atributo_spectrum,&brillo,&parpadeo);

                        }


			//printf ("Mode 2, 640x200 resolution, 2 colours\n");
                        if (modo_video==2) {

				scr_refresca_pantalla_cpc_get_sprite_mode_x(x,y,buffer_letra,1);

                                caracter=compare_char_tabla_step(buffer_letra, &inv,char_set_cpc,1);
				artistic_puntero_origen=buffer_letra;
				artistic_incremento_origen=1;

				//De momento papel 7 tinta 0
				brillo=0;
				parpadeo=0;
				atributo_spectrum=56;
				fun_color(atributo_spectrum,&brillo,&parpadeo);

                        }


                        if (caracter) {
				fun_caracter(x,y,brillo,inv,caracter);
                        }
                          else {

                                inv=0;

				//Hagamos caracter artistico

				if (texto_artistico.v==1) {
					caracter=scr_artistic_retorna_artistic_char(artistic_puntero_origen,artistic_incremento_origen);
				}
				else {
	                                caracter='?';
				}

				fun_caracter(x,y,brillo,inv,caracter);


                        }

                }

		fun_saltolinea();




          }



}

//Guardamos funcion de overlay y lo desactivamos, y finalizamos pantalla
void screen_end_pantalla_save_overlay(void (**previous_function)(void),int *menu_antes ) {
	*previous_function=menu_overlay_function;	
	*menu_antes=menu_overlay_activo;

	menu_overlay_activo=0;			
	scr_end_pantalla();
}

//Restauramos funcion de overlay y lo activamos
void screen_restart_pantalla_restore_overlay(void (*previous_function)(void),int menu_antes)
{
	menu_overlay_function=previous_function;
	menu_overlay_activo=menu_antes;


	//Si hay menu activo, reallocate layers, ya que probablemente habra cambiado el tamaño (activar border, footer, etc)
	if (menu_overlay_activo) {
		scr_init_layers_menu();
	}
}

void screen_set_window_zoom(int z)
{

	if (z>9 || z<1) {
		debug_printf (VERBOSE_ERR,"Invalid zoom value %d",z);
		return;
	}

	//printf ("funcion anterior: %p\n",menu_overlay_function);

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  	void (*previous_function)(void);
	int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);

	//printf ("funcion leida: %p\n",previous_function);

	//printf ("despues end pantalla\n");

	zoom_x=zoom_y=z;
	modificado_border.v=1;

	screen_init_pantalla_and_others_and_realjoystick();
	set_putpixel_zoom();


	menu_init_footer();

	//printf ("despues init footer\n");

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);


	//menu_overlay_function=previous_function;
	//menu_overlay_activo=1;

	//printf ("---------final cambio zooom\n");
}






//En DESUSO:
//Retorna color RGB en formato 32 bits para un color rgb en formato 8 bit de RRRGGGBB. 
//Actualmente de momento solo usado en TBBLUE
//NO es mismo formato que tabla de ulaplus. Ulaplus tiene formato GGGRRRBB
/*
int get_rgb8_color (z80_byte color)
{
	//Minitablas de conversion de 3 bits a 8 bits
	z80_byte color_3_to_8[8]={
	0,36,73,109,146,182,219,255
	};

	int color32;
	z80_byte r,g,b;
	z80_byte r8,g8,b8;


		r=(color>>5)&7;
		g=(color>>2)&7;

		//componente b es un tanto esoterico
		//The missing lowest blue bit is set to OR of the other two blue bits (Bb becomes 000 for 00, and Bb1 for anything else)
		b=(color&3);
		b=(b<<1);
		if (b) b=b|1;

		//Pasamos cada componente de 3 bits a su correspondiente de 8 bits
		r8=color_3_to_8[r];
		g8=color_3_to_8[g];
		b8=color_3_to_8[b];

		color32=(r8<<16)|(g8<<8)|b8;
		return color32;


}
*/


//Retorna color RGB en formato 32 bits para un color rgb en formato 9 bit de RRRGGGBBB. 
//Actualmente de momento solo usado en TBBLUE

int get_rgb9_color (z80_int color)
{
	//Minitablas de conversion de 3 bits a 8 bits
	z80_byte color_3_to_8[8]={
	0,36,73,109,146,182,219,255
	};

	int color32;
	z80_byte r,g,b;
	z80_byte r8,g8,b8;
	//formato en color:
	//876543210
	//RRRGGGBBB

		r=(color>>6)&7;
		g=(color>>3)&7;
		b=color&7;

		//Pasamos cada componente de 3 bits a su correspondiente de 8 bits
		r8=color_3_to_8[r];
		g8=color_3_to_8[g];
		b8=color_3_to_8[b];

		color32=(r8<<16)|(g8<<8)|b8;
		return color32;


}


z80_bit zxuno_tbblue_disparada_raster={0};

z80_byte get_zxuno_tbblue_rasterctrl(void)
{
	if (MACHINE_IS_ZXUNO) return zxuno_ports[0x0d];
	//suponemos tbblue
	else return tbblue_registers[34];
}

void set_zxuno_tbblue_rasterctrl(z80_byte valor)
{
	if (MACHINE_IS_ZXUNO) zxuno_ports[0x0d]=valor;
	//suponemos tbblue
	else tbblue_registers[34]=valor;
}

z80_byte get_zxuno_tbblue_rasterline(void)
{
	if (MACHINE_IS_ZXUNO) return zxuno_ports[0x0c];
	//suponemos tbblue
	else return tbblue_registers[35];
}

void zxuno_tbblue_handle_raster_interrupts()
{


/*
TBBUE y ZXUNO gestionan la interrupcion raster de la misma manera


ZXUNO

$0C RASTERLINE Lectura/Escritura	Almacena los 8 bits menos significativos de la línea de pantalla en la que se desea provocar
un disparo de una interrupción enmascarable. Un valor 0 para este registro (con LINE8 también igual a 0) establece que la
interrupción ráster se disparará, si está habilitada, justo al comenzar el borde derecho de la línea anterior a la primera
 línea de pantalla en la que comienza la zona de "paper". Dicho en otras palabras: el conteo de líneas de esta interrupción
  asume que una línea de pantalla se compone de: borde derecho + intervalo de blanking horizontal + borde izquierdo + zona
	 de paper. Si se asume de esta forma, el disparo de la interrupción se haría al comienzo de la línea seleccionada.
Un valor para RASTERLINE igual a 192 (con LINE8 igual a 0) dispara la interrupción ráster al comienzo del borde inferior.
 Los números de línea para el fin del borde inferior y comienzo del borde superior dependen de los timings empleados.
  El mayor valor posible en la práctiva para RASTERLINE corresponde a una interrupción ráster disparada en
	 la última línea del borde superior (ver RASTERCTRL)

$0D	RASTERCTRL	Lectura/Escritura	Registro de control y estado de la interrupción ráster. Se definen los siguientes bits.
INT	0	0	0	0	DISVINT	ENARINT	LINE8
INT: este bit sólo está disponible en lectura. Vale 1 durante 32 ciclos de reloj a partir del momento en que se dispara la interrupción ráster. Este bit está disponible aunque el procesador tenga las interrupciones deshabilitadas. No está disponible si el bit ENARINT vale 0.
DISVINT: a 1 para deshabilitar las interrupciones enmascarables por retrazo vertical (las originales de la ULA). Tras un reset, este bit vale 0.
ENARINT: a 1 para habilitar las interrupciones enmascarables por línea ráster. Tras un reset, este bit vale 0.
LINE8: guarda el bit 8 del valor de RASTERLINE, para poder definir cualquier valor entre 0 y 511, aunque en la práctica, el mayor valor está limitado por el número de líneas generadas por la ULA (311 en modo 48K, 310 en modo 128K, 319 en modo Pentagon). Si se establece un número de línea superior al límite, la interrupción ráster no se producirá.


TBBLUE:

	(R/W) 34 => Raster line interrupt control
	  bit 7 = (R) INT flag, 1=During INT (even if the processor has interrupt disabled)
	  bits 6-3 = Reserved, must be 0
	  bit 2 = If 1 disables original ULA interrupt (Reset to 0 after a reset)
	  bit 1 = If 1 enables Raster line interrupt (Reset to 0 after a reset)
	  bit 0 = MSB of Raster line interrupt value (Reset to 0 after a reset)

	(R/W) 35 => Raster line interrupt value LSB
	  bits 7-0 = Raster line value LSB (0-255)(Reset to 0 after a reset)

*/

					if (iff1.v==1 && (get_zxuno_tbblue_rasterctrl() & 2) ) {
						//interrupciones raster habilitadas
						//printf ("interrupciones raster habilitadas en %d\n",zxuno_ports[0x0c] + (256 * (zxuno_ports[0x0d]&1) ));


						//Ver si estamos entre estado 128 y 128+32
						int estados_en_linea=t_estados % screen_testados_linea;

						if (estados_en_linea>=128 && estados_en_linea<128+32) {
							//Si no se ha disparado la interrupcion
							if (zxuno_tbblue_disparada_raster.v==0) {
								//Comprobar la linea definida
								//El contador de lineas considera que la línea 0 es la primera línea de paper, la linea 192 por tanto es la primera línea de borde inferior.
								// El último valor del contador es 311 si estamos en un 48K, 310 si estamos en 128K, o 319 si estamos en Pentagon, y coincidiría con la última línea del borde superior.
								//se dispara justo al comenzar el borde derecho de la línea anterior a aquella que has seleccionado
								int linea_raster=get_zxuno_tbblue_rasterline() + (256 * (get_zxuno_tbblue_rasterctrl()&1) );

								int disparada_raster=0;


								//se dispara en linea antes... ?
								/*if (linea_raster>0) linea_raster--;
								else {
									linea_raster=screen_scanlines-1;
								}*/


								//es zona de vsync y borde superior
								//Aqui el contador raster tiene valor (192+56 en adelante)
								//contador de scanlines del core, entre 0 y screen_indice_inicio_pant ,
								if (t_scanline<screen_indice_inicio_pant) {
									if (t_scanline==linea_raster-192-screen_total_borde_inferior) disparada_raster=1;
								}

								//Esto es zona de paper o borde inferior
								//Aqui el contador raster tiene valor 0 .. <(192+56)
								//contador de scanlines del core, entre screen_indice_inicio_pant y screen_testados_total
								else {
									if (t_scanline-screen_indice_inicio_pant==linea_raster) disparada_raster=1;
								}

								if (disparada_raster) {
									//Disparar interrupcion
									zxuno_tbblue_disparada_raster.v=1;
									interrupcion_maskable_generada.v=1;

									//printf ("Generando interrupcion raster en scanline %d, raster: %d , estados en linea: %d, t_estados %d\n",
									//	t_scanline,linea_raster+1,estados_en_linea,t_estados);

									//Activar bit INT
									z80_byte valor=get_zxuno_tbblue_rasterctrl();
									valor |=128;
									set_zxuno_tbblue_rasterctrl(valor);
								}

								else {
									//Resetear bit INT
									//zxuno_ports[0x0d] &=(255-128);
								}
							}
						}

						//Cualquier otra zona de t_estados, meter a 0
						else {
							zxuno_tbblue_disparada_raster.v=0;
							//Resetear bit INT
							z80_byte valor=get_zxuno_tbblue_rasterctrl();
							valor &=(255-128);
							set_zxuno_tbblue_rasterctrl(valor);
						}

					}



}



int generic_footertext_operating_counter=0;

void generic_footertext_print_operating_aux(char *s)
{

        if (generic_footertext_operating_counter) {
        			
								menu_footer_activity(s);
        }
}

void old_delete_generic_footertext_print_operating_aux(char *s)
{

        if (generic_footertext_operating_counter) {
        			//01234567
        	char string_aux[]="        "; //2 espacios, 6 caracteres y 0 final
        	int longitud=strlen(s);
        	if (longitud>6) longitud=6;

        	int indice_string=0;

        	string_aux[indice_string++]=' ';

        	//printf ("texto: %s\n",s);
        	for (;longitud;indice_string++,longitud--,s++) {
        		//printf ("[%d] [%d] [%c] [%c]\n",indice_string,longitud,string_aux[indice_string],*s);
        		string_aux[indice_string]=*s;
        	}

        	string_aux[indice_string++]=' ';
        	string_aux[indice_string]=0;

                //		      					       01234567
                //menu_putstring_footer(WINDOW_FOOTER_ELEMENT_X_GENERICTEXT,1,string_aux,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);
								
								menu_footer_activity(string_aux);
        }
}

void generic_footertext_print_operating(char *s)
{
	//printf ("footer %s\n",s);

        //Si ya esta activo, no volver a escribirlo. Porque ademas el menu_putstring_footer consumiria mucha cpu
        if (!generic_footertext_operating_counter) { 
            //printf("printing footer %s\n",s); 
        	//Borrar si habia alguno otro diferente
					//printf ("delete footer\n");
        	delete_generic_footertext();
        	  
		generic_footertext_operating_counter=2;
                generic_footertext_print_operating_aux(s);

        }

	generic_footertext_operating_counter=2;
}


void delete_generic_footertext(void)
{
	menu_delete_footer_activity();

	//Redibujar zxdesktop para redibujar iconos, para poner a normal los que se hayan puesto en inverso (con actividad)
	//Poner iconos en normal, sin inverso
	zxdesktop_icon_tape_inverse=0;
    zxdesktop_icon_tape_real_inverse=0;
	zxdesktop_icon_mmc_inverse=0;
	zxdesktop_icon_plus3_inverse=0;
	zxdesktop_icon_betadisk_inverse=0;
	zxdesktop_icon_ide_inverse=0;
	zxdesktop_icon_zxpand_inverse=0;
	zxdesktop_icon_mdv_flp_inverse=0;
    zxdesktop_icon_dandanator_inverse=0;
    zxdesktop_icon_zxunoflash_inverse=0;
    zxdesktop_icon_hilow_inverse=0;

	menu_draw_ext_desktop();
}


//Devolver color en rango de 0 a 255
//Entrada: R, G, B cada componente en 8 bit
//Salida: valor de 0 a 255
int rgb_to_grey(int r,int g,int b)
{
/* luminosity method
The luminosity method is a more sophisticated version of the average method. It also averages the values, but it forms a weighted average to account for human perception. We’re more sensitive to green than other colors, so green is weighted most heavily. The formula for luminosity is 0.21 R + 0.72 G + 0.07 B.

https://www.johndcook.com/blog/2009/08/24/algorithms-convert-color-grayscale/
*/

	r=(r*21)/100;
	g=(g*72)/100;
	b=(b*7)/100;

	return r+g+b;

}

int screen_convert_rainbow_to_blackwhite(z80_int *source_bitmap,int source_width,int source_height,int total_ancho)
{
	//Para un cuadrado dado, retorna si es 1 (cercano a negro) o 0 (cercano a blanco)
    //Me parece mas natural asi, al menos en el caso de spectrum (papel 7 tinta 0 habitualmente)
	int total_superficie=source_width*source_height;

	//Sumar todos los colores
	int x,y;

	int acumulado_red,acumulado_green,acumulado_blue;
	acumulado_red=acumulado_green=acumulado_blue=0;

	int rgbcolor,red,green,blue;

	for (x=0;x<source_width;x++) {
		for (y=0;y<source_height;y++) {
			z80_int color=source_bitmap[y*total_ancho+x];


		            rgbcolor=spectrum_colortable[color];
		            red=(rgbcolor>>16)&255;
		            green=(rgbcolor>>8)&255;
		            blue=(rgbcolor)&255;

				acumulado_red +=red;
				acumulado_green +=green;
				acumulado_blue +=blue;

		}
	}

	//printf ("%d %d %d %d\n",acumulado_red,acumulado_green,acumulado_blue,total_superficie);

	//Dividir los componentes de color
	acumulado_red /=total_superficie;
	acumulado_green /=total_superficie;
	acumulado_blue /=total_superficie;

	int color_gris_final;

	//color_gris_final=acumulado_red+acumulado_green+acumulado_blue;
	color_gris_final=rgb_to_grey(acumulado_red,acumulado_green,acumulado_blue);

	//rango 0 a 256. pasar a 0.100
	int porc_gris=(color_gris_final*100)/256;

	//printf ("%d\n",porc_gris);


	//screen_text_brightness: valor general que va de 0 a 100. contraste 50: division entre 2
	//contraste para renderizados de modo texto. 0=todo negro, 100=todo blanco

	//suma de componentes maximo da valor umbral
	//int bw_final;

	int brillo=100-screen_text_brightness;
	
	int valor_uno=1;
	int valor_cero=0;
	
	if (screen_text_all_refresh_pixel_invert.v) {
	valor_uno=0;
	valor_cero=1;
	}

	if (porc_gris>=brillo) return valor_cero;
	else return valor_uno;


}


z80_byte screen_convert_rainbow_to_text_char(z80_int *source_bitmap,int source_width,int source_height,int total_ancho)
{

	//char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";

	int valor_get_pixel=0;





	//Devuelve para un rectangulo dado, su "caracter" zx81
	int anchomitad=source_width/2;
	int altomitad=source_height/2;

    //printf("ancho mitad %d alto mitad %d\n",anchomitad,altomitad);

    if (anchomitad>0 && altomitad>0) {

	int cuadrado_izq=screen_convert_rainbow_to_blackwhite(source_bitmap,anchomitad,altomitad,total_ancho);
	int cuadrado_der=screen_convert_rainbow_to_blackwhite(&source_bitmap[anchomitad],anchomitad,altomitad,total_ancho);
	int cuadrado_aba=screen_convert_rainbow_to_blackwhite(&source_bitmap[altomitad*total_ancho],anchomitad,altomitad,total_ancho);
	int cuadrado_abader=screen_convert_rainbow_to_blackwhite(&source_bitmap[altomitad*total_ancho+anchomitad],anchomitad,altomitad,total_ancho);

	if (cuadrado_izq) valor_get_pixel+=1;
	if (cuadrado_der) valor_get_pixel+=2;
	if (cuadrado_aba) valor_get_pixel+=4;
	if (cuadrado_abader) valor_get_pixel+=8;

	return screen_common_caracteres_artisticos[valor_get_pixel];
    }

    else {
        int cuadrado=screen_convert_rainbow_to_blackwhite(source_bitmap,1,1,total_ancho);
        if (cuadrado) return screen_common_caracteres_artisticos[15];
        else return screen_common_caracteres_artisticos[0];
    }

}

/*
Funcion para convertir buffer de rainbow (o cualquier otro buffer de pantalla con colores indexados 16 bits) a texto
Destino tiene que tener mismas proporciones que origen, por lo que del destino solo se pide factor de division

Se divide origen en cuadriculas, y cada cuadricula en 2x2, y le aplicamos misma conversion que zx81

Ejemplo: origen ancho: 100 destino ancho: 10
cuadriculas: 10 de ancho, cada una dividida en 2x2 -> 20 cuadriculas de ancho

*/
void screen_convert_rainbow_to_text(z80_int *source_bitmap,int source_width,int source_height,z80_byte *destination_text,int division_factor)
{
	int incremento_x=division_factor;
	int incremento_y=division_factor;

	int xorig,yorig,xdest,ydest;

	int ancho_dest=source_width/division_factor;

	for (yorig=0,ydest=0;yorig<source_height;yorig+=incremento_y,ydest++) {
		for (xorig=0,xdest=0;xorig<source_width;xorig+=incremento_x,xdest++) {
			z80_byte caracter=screen_convert_rainbow_to_text_char(&source_bitmap[yorig*source_width+xorig],incremento_x,incremento_y,source_width);

			destination_text[ydest*ancho_dest+xdest]=caracter;
			//*destination_text=caracter;
			//destination_text++;
		}
	}
}


//Maximo ancho a renderizar, en caracteres
int scr_refresca_pantalla_tsconf_text_max_ancho=9999;

//Offset para no mostrar x caracteres a la izquierda
int scr_refresca_pantalla_tsconf_text_offset_x=0;

//Lo mismo pero para vertical
int scr_refresca_pantalla_tsconf_text_max_alto=9999;
int scr_refresca_pantalla_tsconf_text_offset_y=0;

void scr_refresca_pantalla_tsconf_text(void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) , void (*fun_saltolinea) (void) , int factor_division)
{

	//Si no esta realvideo, salir
	if (rainbow_enabled.v==0) return;



			        int ancho,alto;

			        ancho=get_total_ancho_rainbow();
			        alto=get_total_alto_rainbow();

				int ancho_final=ancho/factor_division;
				int alto_final=alto/factor_division;

				z80_byte *buffer_texto;
				buffer_texto=malloc(ancho_final*(alto_final+10)); //Algo mas por si acaso
				if (buffer_texto==NULL) cpu_panic("Can not allocate text buffer");

				screen_convert_rainbow_to_text(rainbow_buffer,ancho,alto,buffer_texto,factor_division);

				z80_byte *buffer_texto_copia;
				buffer_texto_copia=buffer_texto;
	
				int brillo=0;
				int parpadeo=0;

				//de momento papel 7 tinta 0
				fun_color(56,&brillo,&parpadeo);

				int x,y;
				int xfinal,yfinal;

				yfinal=0;
				for (y=scr_refresca_pantalla_tsconf_text_offset_y;y<alto_final &&
					y<scr_refresca_pantalla_tsconf_text_max_alto+scr_refresca_pantalla_tsconf_text_offset_y;y++) {
					xfinal=0;
					for (x=scr_refresca_pantalla_tsconf_text_offset_x;x<ancho_final && 
						x<scr_refresca_pantalla_tsconf_text_max_ancho+scr_refresca_pantalla_tsconf_text_offset_x;x++) {
						
						z80_byte caracter=buffer_texto_copia[y*ancho_final+x];
						fun_caracter (xfinal,yfinal,0,0,caracter);

						xfinal++;
					}
					fun_saltolinea();
					yfinal++;
				}

				free(buffer_texto);


}

void scr_refresca_pantalla_tsconf_text_textmode (void (*fun_color) (z80_byte color,int *brillo, int *parpadeo), void (*fun_caracter) (int x,int y,int brillo, unsigned char inv,z80_byte caracter ) , void (*fun_saltolinea) (void) , int factor_division GCC_UNUSED)
{

	//Solo para modo de texto tsconf
         z80_byte modo_video=tsconf_get_video_mode_display();


                                        if (modo_video!=3) return;

        int ancho_caracter=8;
        int ancho_linea=tsconf_current_pixel_width*2;
        int alto_pantalla=tsconf_current_pixel_height;

        //z80_int puntero=0xc000;

        z80_int puntero=0x0000;

        z80_byte *screen;
        screen=tsconf_ram_mem_table[tsconf_get_vram_page() ];

        int ancho_linea_caracteres=256;
        int x=0;
        int y=0;

	int columna=0;
	int fila=0;

        //z80_byte font_page=tsconf_get_text_font_page();

        //z80_byte *puntero_fuente;
        //puntero_fuente=tsconf_ram_mem_table[font_page];

        z80_int puntero_orig=puntero;

        z80_byte caracter;
        //z80_byte caracter_text;




        //z80_bit inverse;

        //inverse.v=0;

        //z80_int offset_caracter;

        z80_byte papel,tinta;

        z80_byte atributo;

        for (;puntero<7680;) {

                caracter=screen[puntero];
                atributo=screen[puntero+128];

                //printf ("%d ",atributo);

                puntero++;


		//offset_caracter=caracter*8;

                //No tengo ni idea de si se leen los atributos asi, pero parece similar al real
                //tinta=atributo&15;
                //papel=(atributo>>4)&15;
                tinta=atributo&7;
                papel=(atributo>>4)&7;

		int brillo=(atributo>>7)&1; //Bit alto de valor de papel
		int parpadeo=0;




		z80_byte caracter_imprimir=caracter;

		if (caracter_imprimir>127 || caracter_imprimir<32) {

			//caracteres decorativos de menu de tsconf
			if (caracter_imprimir==205) caracter_imprimir='=';
			else if (caracter_imprimir==186) caracter_imprimir='I';
			else if (caracter_imprimir==187) caracter_imprimir='\\';
			else if (caracter_imprimir==188) caracter_imprimir='/';
			else if (caracter_imprimir==200) caracter_imprimir='\\';
			else if (caracter_imprimir==201) caracter_imprimir='/';
			else caracter_imprimir='?';
		}

		fun_color(tinta + papel*8,&brillo,&parpadeo);

		fun_caracter (columna,fila,brillo,0,caracter_imprimir);

                //scr_tsconf_putsprite_comun(&puntero_fuente[offset_caracter],8,x,y,inverse,tinta,papel,NULL);


                x+=ancho_caracter;
		columna++;
                if (x+ancho_caracter>ancho_linea) {
                        //printf ("\n");
                        x=0;
			columna=0;
                        y+=8;
			fila++;
                        if (y+8>alto_pantalla) {
                                //provocar fin
                                puntero=7680;
                        }
                        puntero=puntero_orig+ancho_linea_caracteres; //saltar atributos
                        puntero_orig=puntero;

			fun_saltolinea();
                }
        }
}




void scr_set_fps_stdout_simpletext(int fps)
{
	if (fps<1 || fps>50) {
		debug_printf(VERBOSE_ERR,"Invalid value");
		return;
	}
	scrstdout_simpletext_refresh_factor=50/fps;

}


//Retorna Y posicion de pixeles para un scanline determinado
//Dice si salta linea siguiente
//Si y es negativo quiere decir que no esta visible (en zona de vsync por ejemplo)
int screen_get_y_coordinate_tstates(void)
{
	int y;
	
	y=t_scanline_draw-screen_invisible_borde_superior;

	if (MACHINE_IS_ZX8081) y=t_scanline_draw-ZX8081_LINEAS_SUP_NO_USABLES;

	return y;

}
//Retorna X posicion de pixeles para un scanline determinado
//Dice si salta linea siguiente
//Si x es negativo quiere decir que no esta visible (en zona de hsync por ejemplo)
int screen_get_x_coordinate_tstates(int *si_salta_linea)
{
        int estados_en_linea=t_estados % screen_testados_linea;

/*


Estos en pixeles
//normalmente a 48
int screen_total_borde_izquierdo;

//normalmente a 48
int screen_total_borde_derecho;

//normalmente a 96
int screen_invisible_borde_derecho;


*/

//printf ("screen_total_borde_derecho: %d\n",screen_total_borde_derecho);
//printf ("screen_total_borde_izquierdo: %d\n",screen_total_borde_izquierdo);
//printf ("screen_invisible_borde_derecho: %d\n",screen_invisible_borde_derecho);
//printf ("\n");

	*si_salta_linea=0; //por defecto
	int x;

	//En zx8081
	if (MACHINE_IS_ZX8081) {
		int inicio_hsync=24+128+24;
		if (estados_en_linea>=inicio_hsync) return -1;

		return estados_en_linea*2;
	}


	//Las variables que usamos, estas screen_X son valores en pixeles

	//Por tanto pasamos los t-estados actuales a turbo x1
	estados_en_linea /=cpu_turbo_speed;

	//Lo siguiente en t-estados
	int inicio_borde_derecho;

	//inicio_borde_derecho=128;
	inicio_borde_derecho=screen_testados_linea/cpu_turbo_speed-screen_total_borde_izquierdo/2-screen_total_borde_derecho/2-screen_invisible_borde_derecho/2;

	//printf ("%d\n",inicio_borde_derecho);
	int inicio_borde_derecho_invisible=inicio_borde_derecho+screen_total_borde_derecho/2;
	int inicio_borde_izquierdo=inicio_borde_derecho_invisible+screen_invisible_borde_derecho/2;

	//Si estoy mas alla del border izquierdo, avisar de salto de coordenada
	if (estados_en_linea>=inicio_borde_izquierdo) {
		*si_salta_linea=1;
		//Ajustar a la izquierda
		estados_en_linea -=inicio_borde_izquierdo;

		//coordenada final
		int x=estados_en_linea*2;
		return x;
	}

	//Si estoy en la zona de parte derecha invisible
	if (estados_en_linea>=inicio_borde_derecho_invisible && estados_en_linea<inicio_borde_izquierdo) {
		//Zona invisible
		return -1;
	}

	//Zona display o de border derecho
	x=estados_en_linea*2;
	x +=screen_total_borde_izquierdo; //Sumarle el ancho de pixeles de borde izquierdo
	return x;
}


//Cambia la paleta de color a Modo blanco y negro cuando se abre menu y multitarea esta a off
/*
void screen_change_bw_menu_multitask(void)
{

	if (menu_multitarea==0 && menu_abierto) {
		if (screen_bw_no_multitask_menu.v) spectrum_colortable=spectrum_colortable_new_blanco_y_negro;
	}

}
*/

/*
Nadie deberia llamar a scr_init_pantalla() directamente. Hay que usar esta funcion, por la razon de que:
Al cambiar por ejemplo footer, se cierra y se abre driver de video. Si no hay resize/removimiento de ventana , lo que sucede es que
al abrir la ventana se genera una ventana con fondo negro. Dado que la putpixel cache no habria cambiado, no se refresca,
y por tanto se queda en negro. Esto pasa al cambiar otros parametros tambien de la ventana, como al cambiar zoom por ejemplo
*/

/*
Nota: antes de usar esta funcion, sucedia por ejemplo que, al desactivar/activar footer, se hacia scr_end_pantalla y scr_init_pantalla,
generalmente el driver xwindows, genera un evento de scrxwindows_resize, que a su vez, generaba un clear_putpixel_cache, por lo que todo iba bien
Pero a veces, en concreto en mi pc (quiza dependa de la version de Xorg) no genera dicho evento, con lo que no se hacia clear_putpixel_cache,
dejando la ventana en negro como se comenta antes
*/

int screen_init_pantalla_and_others(void)
{
	int retorno=scr_init_pantalla();
  
	//Siempre que se redimensiona tamanyo ventana (sin contar zoom) o se reinicia driver video hay que reiniciar cache putpixel
  init_cache_putpixel();

	//printf ("screen_init_pantalla_and_others\n");
	//menu_init_footer();

	return retorno;
}


void screen_init_pantalla_and_others_and_realjoystick(void)
{



	screen_init_pantalla_and_others();

	/*
	Al iniciar driver video, en el caso de SDL por ejemplo, apunta a las funciones de realjoystick sdl. Si no inicializamos dicho joystick,
	sucedera que al hacer el poll de joystick, usara un joystick no inicializado y petara 

	TODO: hacer que el init de sdl de video, tambien inicialice el joystick (en el caso que no usemos driver linux nativo)
	*/

	//Ya no hace falta

    //realjoystick_reopen_driver();
}


const char *s_spectrum_video_mode_standard="256x192";
const char *s_spectrum_video_mode_timex_standard_one="256x192 Timex Screen 1";
const char *s_spectrum_video_mode_timex_hicol="256x192 Timex 8x1 Color";
const char *s_spectrum_video_mode_timex_hires="512x192 Timex monochrome";

const char *s_spectrum_video_mode_tbblue_lores="128x96 256 colours";

char *get_spectrum_ula_string_video_mode(void)
{

	//Por defecto
	const char *string_mode=s_spectrum_video_mode_standard;

	if (timex_video_emulation.v) {

			if ((timex_port_ff&7)==1) string_mode=s_spectrum_video_mode_timex_standard_one;
			else if ((timex_port_ff&7)==2) string_mode=s_spectrum_video_mode_timex_hicol;
			else if ((timex_port_ff&7)==6) string_mode=s_spectrum_video_mode_timex_hires;

	}

	if (MACHINE_IS_TBBLUE) {
		if (tbblue_registers[21]&128) string_mode=s_spectrum_video_mode_tbblue_lores;
	}

	return (char *)string_mode;

}


//Convertir paleta EGA de 16 colores a Spectrum
int screen_ega_to_spectrum_colour(int ega_col)
{
//https://en.wikipedia.org/wiki/Enhanced_Graphics_Adapter
	//Ega:      0 black, 1 blue, 2 green, 3 cyan,    4 red,   5 magenta, 6 brown,  7 white, bright black, bright blue, bright green, bright cyan, bright red, bright magenta, bright yellow, white, 
	//Spectrum: 0 black, 1 blue, 2 red,   3 magenta, 4 green, 5 cyan,    6 yellow, 7 white, + brillos

	int lookup_table[]={0,1,4,5,2,3,6,7};

	int brillo=0;

	if (ega_col>7) {
		brillo=1;
		ega_col -=8;
	}
	//Por si acaso

	ega_col &=7;

	int color_final=lookup_table[ega_col]+8*brillo;
	return color_final;
}



int screen_mode_16c_is_enabled(void)
{
	if (pentagon_16c_mode_available.v && (puerto_eff7 & 1) ) return 1;
	else return 0;
}

void enable_16c_mode(void)
{
    //necesita real video
    enable_rainbow();
	pentagon_16c_mode_available.v ^=1;
}


void disable_16c_mode(void)
{

	pentagon_16c_mode_available.v ^=1;
}


/*
//Se puede especificar un x_offset que desplaza la imagen x pixeles a la derecha,
Se puede especificar un x_ignore que hace que X lineas desde la izquierda no se dibujen,
lo que pretendemos es que la imagen no quede pegada a la izquierda y justo encima del marco de la ventana, 
produciendo parpadeo pues sobreescribe el marco, y el marco sobreescribe a la imagen
Desplazandolo a la izquierda ese espacio que ocupa el marco, evitamos el parpadeo


TODO: 1. si hay barra de scroll horizontal y desplazamos la imagen, esta se "comera" esos pixeles de margen y se irá a la zona del marco
TODO: 2. si desplazamos la ventana y evitamos el marco, en las ventanas de help keyboard, cuando la ventana no tiene el foco,
se redibujará toda la ventana pero excepto el marco (lo habitual) pero como esas ventanas son transparentes (excepto logicamente para la imagen),
el marco forma parte de la zona transparente y por tanto se verá transparente dicho marco, mostrando el fondo
*/

//parametro de follow_zoom hace seguir al zoom de la interfaz, usado al visualizar el logo de la salamandra,
//pero no al visualizar el help keyboard
//si ancho_mostrar=0, no hacemos caso

//indice_color_transparente: indica que numero de color se tratara como transparente. Indicar a -1 para no hacer transparencia
//color_final_transparente: color que se mostrara en el caso de transparente
//Realmente no es transparencia, sino indicar que determinado color se mostrara con un color concreto y no de la paleta
//pero se usa para simular transparencia, desde el menu about, para indicar color igual al del papel de fondo
void screen_render_bmpfile(z80_byte *mem,int indice_paleta_color,zxvision_window *ventana,int x_ignore,int follow_zoom,
    int ancho_mostrar,int indice_color_transparente,int color_final_transparente)
{

						//putpixel del archivo bmp
			

/*
Name	Size	Offset	Description
Header	
 	Signature	2 bytes	0000h	'BM'
FileSize	4 bytes	0002h	File size in bytes
reserved	4 bytes	0006h	unused (=0)
DataOffset	4 bytes	000Ah	Offset from beginning of file to the beginning of the bitmap data

 	Size	4 bytes	000Eh	Size of InfoHeader =40 
Width	4 bytes	0012h	Horizontal width of bitmap in pixels
Height	4 bytes	0016h	Vertical height of bitmap in pixels
*/		


	//ancho y alto de la cabecera. maximo 16 bit
						int ancho=mem[18] + 256 * mem[19];
						int alto=mem[22] + 256 * mem[23];

						//printf ("ancho: %d alto: %d\n",ancho,alto);


						//118 bytes de cabecera ignorar
						//Cuantos bytes de cabecera ignorar?

/*
						Name	Size	Offset	Description
Header	
 	Signature	2 bytes	0000h	'BM'
FileSize	4 bytes	0002h	File size in bytes
reserved	4 bytes	0006h	unused (=0)
DataOffset	4 bytes	000Ah	Offset from beginning of file to the beginning of the bitmap data
*/
						//Pillamos el offset como valor de 16 bits para simplificar

						int offset_bmp=mem[10] + 256 * mem[11];
						//printf ("offset pixeles: %d\n",offset_bmp);

								int ancho_calculo=ancho;
								//Nota: parece que el ancho tiene que ser par, para poder calcular el offset
								if ( (ancho_calculo % 2 ) !=0) ancho_calculo++;						

                        int ancho_mostrar_final=ancho;

                        if (ancho_mostrar!=0) ancho_mostrar_final=ancho_mostrar;

                        //printf ("ancho mostrar final : %d\n",ancho_mostrar_final);

						int x,y;
						for (y=0;y<alto;y++) {
							for (x=0;x<ancho_mostrar_final;x++) {
                                if (x<x_ignore) continue;
								//lineas empiezan por la del final en un bmp
								//1 byte por pixel, color indexado
			

								int offset_final=(alto-1-y)*ancho_calculo + x + offset_bmp;

								


								//printf ("offset_final_ %d\n",offset_final);
								z80_byte byte_leido=mem[offset_final];
                                //printf("byte leido: %d\n",byte_leido);
                                //if (byte_leido==255) byte_leido=0;
								z80_int color_final;

                                if (indice_color_transparente>=0 && byte_leido==indice_color_transparente) color_final=color_final_transparente;
                                else color_final=indice_paleta_color+byte_leido;

								//zxvision_putpixel(ventana,x,y,color_final);

                                if (!follow_zoom) {
								    zxvision_putpixel_no_zoom(ventana,x,y,color_final);
                                }
                                else {
                                    int zx,zy;
                                    for (zx=0;zx<zoom_x;zx++) {
                                        for (zy=0;zy<zoom_y;zy++) {
                                            zxvision_putpixel_no_zoom(ventana,x*zoom_x+zx,y*zoom_y+zy,color_final);
                                        }
                                    }
                                    
                                }
							}
						}
						

}

