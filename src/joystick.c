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

#include <stdio.h>
#include <string.h>


#include "joystick.h"
#include "cpu.h"
#include "debug.h"
#include "screen.h"
#include "zxvision.h"
#include "zeng.h"
#include "mem128.h"
#include "ula.h"
#include "menu_bitmaps.h"

#ifdef COMPILE_CURSES
        #include "scrcurses.h"
#endif


int joystick_emulation=JOYSTICK_CURSOR_WITH_SHIFT;
int joystick_autofire_frequency=0;
int joystick_autofire_counter=0;

//auto left right
int joystick_autoleftright_enabled=0;
int joystick_autoleftright_frequency=1;
int joystick_autoleftright_counter=0;

//0=left, 1=right
int joystick_autoleftright_status=0;

/*
Los dispositivos baratos meten "basura" en el bus: al generar una interrupción, en vez de haber el FFH habitual,
meten el valor de lectura del disposito, porque creen que se está leyendo el joystick.
Ejemplo extenso explicado por Miguel Angel Rodriguez, respecto a joysticks kempston que solo leen A5=0 y no tienen en cuenta
que se active la señal de lectura:

Resulta que durante el ciclo de reconocimiento de la interrupción, el Z80 activa IORQ, como si fuese
un ciclo de entrada salida, pero no activa ni la señal de lectura ni la de escritura. Esas interfaces
decodifican falsamente esa situación como una situación de lectura de puerto. Si da la casualidad de que
en el bus de direcciones, en ese momento, A5 esté a cero (y eso depende de qué dirección de memoria es la
que se estaba ejecutando en el momento en que ocurrió la interrupción) tienes al aparato enviando por el
bus de datos lo que esté leyendo del Joy stick en ese momento, y el Z80 interpretándolo como un vector de interrupción

IORQ se activa durante el ACK de una interrupción, es lo esperado, y es un comportamiento documentado.
No es un error del procesador. En lugar de usar un pin del chip específico para notificar el reconocimiento de
interrupción, se usa la secuencia de activar IORQ y también M1. Esa secuencia no se activa a la vez en ningún otro momento

---

Realmente si emulase solo dispositivo kempston barato, tendria que al menos ver si A5=0 y meter basura en el bus en ese caso
Como lo estoy haciendo como una opción de joystick genérico barato, no miro A5=0 y meto siempre basura en el bus al generar interrupción

Juegos que fallan con esta opción: pussy.tap, simulador de raton (Antonio Bermudez, 1991, Microhobby), The Humans

*/
int joystick_barato=0;

//Que tecla actua como el Fire del joystick. Por defecto, Home
//Si es <0, no actua ninguna tecla
//Para los 4 botones de joystick
int joystick_defined_key_fire_array[4]={JOYSTICK_KEY_FIRE_IS_HOME,-1,-1,-1};

char *joystick_defined_fire_texto[]={
    "Home",
    "RightAlt",
    "RightCtrl",
    "RightShift",
    "LeftAlt",
    "LeftCtrl",
    "LeftShift",
    "Tab"
};

void handle_pressed_a_fire_key(int joy_fire_to_check,int key_pressed,int pressrelease)
{
    if (joystick_defined_key_fire_array[0]==joy_fire_to_check && joystick_emulation!=JOYSTICK_NONE) util_set_reset_key(UTIL_KEY_FIRE,pressrelease);
    else if (joystick_defined_key_fire_array[1]==joy_fire_to_check && joystick_emulation!=JOYSTICK_NONE) util_set_reset_key(UTIL_KEY_FIRE2,pressrelease);
    else if (joystick_defined_key_fire_array[2]==joy_fire_to_check && joystick_emulation!=JOYSTICK_NONE) util_set_reset_key(UTIL_KEY_FIRE3,pressrelease);
    else if (joystick_defined_key_fire_array[3]==joy_fire_to_check && joystick_emulation!=JOYSTICK_NONE) util_set_reset_key(UTIL_KEY_FIRE4,pressrelease);
    else util_set_reset_key(key_pressed,pressrelease);
}

//Funciones que segun la tecla pulsada de posibles joystick fire (home, rightalt, etc)
//acaban actuando sobre el fire o sobre la tecla
void joystick_possible_home_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_HOME,UTIL_KEY_HOME,pressrelease);
}


void joystick_possible_rightshift_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_RIGHTSHIFT,UTIL_KEY_SHIFT_R,pressrelease);
}

void joystick_possible_rightalt_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_RIGHTALT,UTIL_KEY_ALT_R,pressrelease);
}

void joystick_possible_rightctrl_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_RIGHTCTRL,UTIL_KEY_CONTROL_R,pressrelease);
}

void joystick_possible_leftshift_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_LEFTSHIFT,UTIL_KEY_SHIFT_L,pressrelease);
}

void joystick_possible_leftalt_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_LEFTALT,UTIL_KEY_ALT_L,pressrelease);
}

void joystick_possible_leftctrl_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_LEFTCTRL,UTIL_KEY_CONTROL_L,pressrelease);
}

void joystick_possible_tab_key(int pressrelease)
{
    handle_pressed_a_fire_key(JOYSTICK_KEY_FIRE_IS_TAB,UTIL_KEY_TAB,pressrelease);
}

//Emulación de pistolas ópticas, lapices ópticos, etc
z80_bit lightgun_emulation_enabled={0};

int lightgun_emulation_type=0;

z80_bit lightgun_scope={0};

//Coordenadas x,y en formato scanlines y pixeles totales, es decir,
//x entre 0 y 351
//y entre 0 y 295
//0,0 esta arriba a la izquierda

int lightgun_x=0;
int lightgun_y=0;



//Coordenadas x,y tal cual las retorna el driver de video, segun el tamanyo de ventana activo
//en xwindows, normalmente entre
//x entre 0 y 351
//y entre 0 y 295
//0,0 esta arriba a la izquierda
//con zoom 1
//Con zoom 2, el doble:
//x entre 0 y 703
//y entre 0 y 591
//Para cacalib, tambien devuelve coordenadas dentro del tamanyo de la ventana
//por defecto, 80x32
//Al entrar, no sabemos donde está el ratón, asumimos fuera de pantalla (-100 para que sea "lejos" de la pantalla). Si en cambio establecieramos 0,0,
//al entrar y pulsar F5 (con topbar habilitado) sin haber movido el raton antes, se desplegaria el menu Z
//porque se pensaria que el raton está arriba del todo a la izquierda
int mouse_x=-100,mouse_y=-100;

int mouse_pressed_close_window=0;
int mouse_pressed_background_window=0;


int mouse_pressed_hotkey_window=0;
//y que tecla puslada
int mouse_pressed_hotkey_window_key=0;

//si esta activa la emulacion de kempston mouse
z80_bit kempston_mouse_emulation;

//pulsado boton izquierdo razon
int mouse_left=0;
//pulsado boton derecho raton
int mouse_right=0;

//accionado wheel vertical. Positivo: scroll arriba. Negativo: scroll abajo
int mouse_wheel_vertical=0;

//accionado wheel horizontal. Positivo: scroll izquierda. Negativo: scroll derecha
int mouse_wheel_horizontal=0;

//Coordenadas x,y de retorno a puerto kempston
//Entre 0 y 255 las dos. Coordenada Y hacia abajo resta
//se toma como base el mismo formato que lightgun x e y pero con modulo % 256
z80_byte kempston_mouse_x=0,kempston_mouse_y=0;


//Mascara de bits (desde 7 hasta 0): Fire4 Fire3 Fire2 Fire1 Up Down Left Right
z80_byte puerto_especial_joystick=0;



char *joystick_texto[]={
    "None",
    "Kempston",
    "Sinclair 1",
    "Sinclair 2",
    "Cursor",
    "Cursor&Shift",
    "OPQA Space",
    "Fuller",
    "Zebra ZX81",
    "MikroGen ZX81",
    "ZXpand ZX81",
    "Cursor Sam",
    "CPC Joy1",
    "MSX",
    "Spectravideo",
    "PCW Cascade",
    "PCW DKTronics"
};


char *lightgun_types_list[LIGHTGUN_TOTAL]={
    "Gunstick Sinclair 1",
    "Gunstick Sinclair 2",
    "Gunstick Kempston",
    "Magnum (AUX port)",
    "Magnum (EAR port)",
    "Defender Light Gun",
    "Stack Light Rifle",
    "Trojan Light Pen (EAR port)"
};

void lightgun_print_types(void)
{
    int i;

    for (i=0;i<LIGHTGUN_TOTAL;i++) {
        printf ("%s",lightgun_types_list[i]);
        if (i!=LIGHTGUN_TOTAL-1) printf (", ");
    }
}

//Devuelve 0 si ok
int lightgun_set_type(char *tipo)
{

    debug_printf (VERBOSE_INFO,"Setting lightgun type %s",tipo);

    int i;
    for (i=0;i<LIGHTGUN_TOTAL;i++) {
        if (!strcasecmp(tipo,lightgun_types_list[i])) break;
    }

    if (i>=LIGHTGUN_TOTAL) {
        return 1;
    }

    lightgun_emulation_type=i;
    return 0;
}

void joystick_cycle_next_type_autofire(void)
{

    //desactivamos autofire
    joystick_autofire_frequency=0;
    //y ponemos tecla fire a 0, por si se habia quedado activa
    puerto_especial_joystick=0;

}

void joystick_cycle_next_type(void)
{
    if (joystick_emulation==JOYSTICK_TOTAL) joystick_emulation=0;
    else joystick_emulation++;

    joystick_cycle_next_type_autofire();

}

//Liberar auto left right si conviene, y quitar direccion actual
void joystick_clear_leftright(void)
{
    if (joystick_autoleftright_enabled) {
        joystick_autoleftright_enabled=0;
        puerto_especial_joystick &=(255-3); //quitar left y right
        //quitar linea de footer LEFT/RIGHT
        delete_generic_footertext();
    }
}


void joystick_set_right(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

    //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
    puerto_especial_joystick |=1;
    debug_printf(VERBOSE_DEBUG,"joystick_set_right");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_RIGHT,1);
}

void joystick_release_right(int si_enviar_zeng_event)
{
    puerto_especial_joystick &=255-1;
    debug_printf(VERBOSE_DEBUG,"joystick_release_right");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_RIGHT,0);
}


void joystick_set_left(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

    //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
    puerto_especial_joystick |=2;
    debug_printf(VERBOSE_DEBUG,"joystick_set_left");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_LEFT,1);
}

void joystick_release_left(int si_enviar_zeng_event)
{
    puerto_especial_joystick &=255-2;
    debug_printf(VERBOSE_DEBUG,"joystick_release_left");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_LEFT,0);
}



void joystick_set_down(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

    //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
    puerto_especial_joystick |=4;
    debug_printf(VERBOSE_DEBUG,"joystick_set_down");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_DOWN,1);
}

void joystick_release_down(int si_enviar_zeng_event)
{
    puerto_especial_joystick &=255-4;
    debug_printf(VERBOSE_DEBUG,"joystick_release_down");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_DOWN,0);
}

void joystick_set_up(int si_enviar_zeng_event)
{
    joystick_clear_leftright();

    //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
    puerto_especial_joystick |=8;
    debug_printf(VERBOSE_DEBUG,"joystick_set_up");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_UP,1);
}

void joystick_release_up(int si_enviar_zeng_event)
{
    puerto_especial_joystick &=255-8;
    debug_printf(VERBOSE_DEBUG,"joystick_release_up");

    if (si_enviar_zeng_event) zeng_send_key_event(UTIL_KEY_JOY_UP,0);
}

//fire_button: indica que boton de fuego: 0 primer boton, 1 segundo boton, etc
void joystick_set_fire(int si_enviar_zeng_event,int fire_button)
{
    joystick_clear_leftright();

    //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
    int mascara_fuego=16 << fire_button;
    puerto_especial_joystick |=mascara_fuego;
    debug_printf(VERBOSE_DEBUG,"joystick_set_fire");

    if (si_enviar_zeng_event) {
        if (fire_button==0)      zeng_send_key_event(UTIL_KEY_JOY_FIRE,1);
        else if (fire_button==1) zeng_send_key_event(UTIL_KEY_JOY_FIRE2,1);
        else if (fire_button==2) zeng_send_key_event(UTIL_KEY_JOY_FIRE3,1);
        else if (fire_button==3) zeng_send_key_event(UTIL_KEY_JOY_FIRE4,1);
    }
}

//fire_button: indica que boton de fuego: 0 primer boton, 1 segundo boton, etc
void joystick_release_fire(int si_enviar_zeng_event,int fire_button)
{
    int mascara_fuego=16 << fire_button;
    puerto_especial_joystick &=255-mascara_fuego;
    debug_printf(VERBOSE_DEBUG,"joystick_release_fire");

    if (si_enviar_zeng_event) {
        if (fire_button==0)      zeng_send_key_event(UTIL_KEY_JOY_FIRE,0);
        else if (fire_button==1) zeng_send_key_event(UTIL_KEY_JOY_FIRE2,0);
        else if (fire_button==2) zeng_send_key_event(UTIL_KEY_JOY_FIRE3,0);
        else if (fire_button==3) zeng_send_key_event(UTIL_KEY_JOY_FIRE4,0);
    }
}

//Retornar color de la pantalla, teniendo rainbow activo
//De momento esto no funciona, juegos con la magnum no van bien con este método
int lightgun_view_pixel_color_rainbow(int x,int y)
{
    int ancho,alto;

    ancho=get_total_ancho_rainbow();
    alto=get_total_alto_rainbow();

    if (x>=0 && y>=0 && x<ancho && y<alto) {
        int indice;

        indice=(ancho*y)+x;

        int color=rainbow_buffer[indice];

        printf("x %d y %d color %d\n",x,y,color);

        return color;
    }

    else return 0;
}

//Retornar color de un pixel de la memoria de pantalla del Spectrum
//si fuera de pantalla, retornar color del border
int lightgun_view_pixel_color_no_rainbow(int x,int y)
{

    if (x>255 || y>191 || x<0 || y<0) {

        //TODO: esto obviamente lee un solo color para todo el border,
        //si el juego está cambiando el color del border en el mismo frame (franjas por ejemplo u otros efectos)
        //no funcionará
        int color=out_254 & 7;
        //printf("out of range. border color %d\n",color);
        //retornar color border
        return color;
    }

    int dir_atributo=y/8;
    dir_atributo *=32;
    dir_atributo +=x/8;

    int dir_pixel=screen_addr_table[y*32+(x/8)];

    int bit_pixel=x % 8;

    //printf("dpixel %d datributo %d\n",dir_pixel,dir_atributo);


    z80_byte *screen_atributo=get_base_mem_pantalla_attributes();
    z80_byte *screen_pixel=get_base_mem_pantalla();

    z80_byte valor_pixeles=screen_pixel[dir_pixel];
    z80_byte valor_atributo=screen_atributo[dir_atributo];

    int tinta=valor_atributo & 7;
    int papel=(valor_atributo>>3) & 7;

    if (valor_atributo & 128) {
        if (estado_parpadeo.v) {
            int temp_tinta=tinta;
            tinta=papel;
            papel=temp_tinta;
        }
    }

    int mascara=128 >> bit_pixel;
    int pix=valor_pixeles & mascara;

    int color;
    if (pix) color=tinta;
    else color=papel;

    //printf("## x %3d y %3d color %d\n",x,y,color);

    return color;

}

//Ver si la zona donde apunta el raton (lightgun) esta en blanco. Usado en Gunstick de MHT
int old_lightgun_view_white(void)
{

//Si curses
#ifdef COMPILE_CURSES
    if (!strcmp(scr_new_driver_name,"curses")) {
        return (scrcurses_return_lightgun_view_white() );
    }
#endif

    //ver zona en blanco
    //Proteccion para que no se salga de putpixel_cache
    //dimensiones putpixel_cache
    int ancho,alto;
    //ancho=LEFT_BORDER_NO_ZOOM+ANCHO_PANTALLA+RIGHT_BORDER_NO_ZOOM;
    //alto=TOP_BORDER_NO_ZOOM+ALTO_PANTALLA+BOTTOM_BORDER_NO_ZOOM;

    ancho=screen_get_emulated_display_width_no_zoom();
    alto=screen_get_emulated_display_height_no_zoom();

    if (lightgun_x<ancho && lightgun_y<alto) {
        int indice_cache;

        indice_cache=(get_total_ancho_rainbow()*lightgun_y)+lightgun_x;

        z80_byte color=putpixel_cache[indice_cache];

        //color blanco con o sin brillo
        if (color==15 || color==7) {
            //debug_printf (VERBOSE_DEBUG,"white zone detected on lightgun");
            return 1;
        }
    }

    return 0;
}

//Ver si la zona donde apunta el raton (lightgun) esta en blanco. Usado en Gunstick de MHT
int lightgun_view_white(void)
{

//Si curses
#ifdef COMPILE_CURSES
    if (!strcmp(scr_new_driver_name,"curses")) {
        return (scrcurses_return_lightgun_view_white() );
    }
#endif

    //ver zona en blanco

    int color;


    color=lightgun_view_pixel_color_no_rainbow(lightgun_x-screen_testados_total_borde_izquierdo*2,
                                    lightgun_y-screen_borde_superior);

    //color blanco con o sin brillo
    if (color==15 || color==7) {
        //debug_printf (VERBOSE_DEBUG,"white zone detected on lightgun");
        return 1;
    }

    else {
        return 0;
    }
}



int lightgun_view_electron_color(void)
{

    int x,y;

    //Ver si hay algo en blanco cerca de donde se ha disparado

    x=lightgun_x;
    y=lightgun_y;


    int color;


    //printf("lightgun %3d %3d\n",x,y);
    /*
    if (rainbow_enabled.v) {
        color=lightgun_view_pixel_color_rainbow(x,y);
    }
    else {
        color=lightgun_view_pixel_color_no_rainbow(x-screen_testados_total_borde_izquierdo*2,
                                    y-screen_borde_superior);
    }
    */

    color=lightgun_view_pixel_color_no_rainbow(x-screen_testados_total_borde_izquierdo*2,
                                y-screen_borde_superior);

    if (color!=0 && color!=8) {
        //debug_printf (VERBOSE_DEBUG,"Non black zone detected on lightgun. lightgun x: %d y: %d, color=%d",lightgun_x,lightgun_y,color);
        return 1;
    }



    return 0;
}




//Ver si la zona donde apunta el raton (lightgun) esta pasando el electron (o si ha pasado hace poco rato) y hay color blanco
//Como máximo el electron puede haber pasado hace 1 scanline (64 microsegundos máximo)
//Usado en magnum light phaser y stack light rifle
int lightgun_view_electron(void)
{
    //ver electron

    //x inicialmente esta posicionada dentro de pantalla... sin contar border
    int x=t_estados % screen_testados_linea;
    int y=t_estados/screen_testados_linea;

    //restamos zona no visible superior
    //printf ("y calculada: %d t_scanline_draw: %d\n",y,t_scanline_draw);
    y -=screen_invisible_borde_superior;

    x=x+screen_testados_total_borde_izquierdo;


   //x esta en t_estados. pasamos a pixeles

    x=x*2;


    //debug_printf (VERBOSE_PARANOID,"electron is at t_estados: %6d x: %3d y: %3d . gun is at x: %3d y: %3d",
    //    t_estados,x,y,lightgun_x,lightgun_y);

    //printf ("electron is at t_estados: %6d x: %3d y: %3d . gun is at x: %3d y: %3d\n",
    //    t_estados,x,y,lightgun_x,lightgun_y);


    //Nuevo calculo para saber si esta en rango. Mediante offset total
    //Contamos rango en pixeles (de ahí los *2)
    int electron_offset=y*(screen_testados_linea*2)+x;
    int lightgun_offset=lightgun_y*(screen_testados_linea*2)+lightgun_x;

    int max_offset=(screen_testados_linea*2);
    int delta_offset=electron_offset-lightgun_offset;

    //printf("Offsets electron %6d lightgun %6d offset %7d max_offset %6d\n",electron_offset,lightgun_offset,delta_offset,max_offset);



    if (delta_offset<0) {
        //printf("No ha llegado aun el electron a donde esta la pistola\n");
        return 0;
    }

    if (delta_offset>max_offset) {
        //printf("Electron esta muy lejos de la pistola\n");
        return 0;
    }

    //printf("Electron esta en rango de la pistola\n");

    //debug_printf (VERBOSE_DEBUG,"lightgun y (%d) is in range of electron (%d)",lightgun_y,y);


    return lightgun_view_electron_color();




}


void joystick_print_types(void)
{
    int i;

    for (i=0;i<=JOYSTICK_TOTAL;i++) {
        printf ("%s",joystick_texto[i]);
        if (i!=JOYSTICK_TOTAL) printf (", ");
    }
}

//dibujar la mirilla
void lightgun_draw_scope(void)
{
    if (lightgun_emulation_enabled.v==0) return;

    if (lightgun_scope.v==0) return;

    if (rainbow_enabled.v==0) return;

    if (menu_abierto) return;

    int ancho_rainbow=get_total_ancho_rainbow();
    int alto_rainbow=get_total_alto_rainbow();
    int offset_x=13;
    int offset_y=13;

    char **bitmap=bitmap_button_ext_desktop_mirilla_lightgun;

    if (mouse_left) bitmap=bitmap_button_ext_desktop_mirilla_lightgun_disparada;

    //margen en los laterales para poder poner la mirilla centrada
    if (lightgun_x>=offset_x && lightgun_y>=offset_y &&
        lightgun_x<=ancho_rainbow-ZESARUX_ASCII_LOGO_ANCHO+offset_x && lightgun_y<=alto_rainbow-ZESARUX_ASCII_LOGO_ALTO+offset_y) {
    screen_put_asciibitmap_generic(bitmap,rainbow_buffer,lightgun_x-offset_x,lightgun_y-offset_y,
        ZESARUX_ASCII_LOGO_ANCHO,ZESARUX_ASCII_LOGO_ALTO, get_total_ancho_rainbow(),screen_generic_putpixel_indexcolour,1,0,0);
    }
}
