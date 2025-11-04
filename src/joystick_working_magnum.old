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
//#define JOYSTICK_KEY_FIRE_IS_HOME 0
//#define JOYSTICK_KEY_FIRE_IS_RIGHTALT 1
//#define JOYSTICK_KEY_FIRE_IS_RIGHTCTRL 2
//#define JOYSTICK_KEY_FIRE_IS_RIGHTSHIFT 3
//int joystick_defined_key_fire=JOYSTICK_KEY_FIRE_IS_HOME;

//Para los otros 3 posibles botones de joystick
//int joystick_defined_key_fire2=-1;
//int joystick_defined_key_fire3=-1;
//int joystick_defined_key_fire4=-1;

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

//int gunstick_emulation=0;

//temp
int gunstick_emulation=GUNSTICK_AYCHIP;

//Coordenadas x,y en formato scanlines y pixeles totales, es decir,
//x entre 0 y 351
//y entre 0 y 295
//0,0 esta arriba a la izquierda

int gunstick_x,gunstick_y;

//rangos de deteccion de electron, para pistola magnum light gun
//Rango x 32 , y 8 va algo bien (aunque lejos de perfecto) en Acid
int gunstick_range_x=1;
int gunstick_range_y=1;

int gunstick_y_offset=0;

//si detecta solo zonas con blanco y brillo 1. sino, detecta blanco sea con brillo o no
int gunstick_solo_brillo=0;

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
//se toma como base el mismo formato que gunstick x e y pero con modulo % 256
z80_byte kempston_mouse_x=0,kempston_mouse_y=0;


//Mascara de bits (desde 7 hasta 0): Fire4 Fire3 Fire2 Fire1 Up Down Left Right
z80_byte puerto_especial_joystick=0;

//z80_byte puerto_especial_gunstick=0; //Fire 0 o 1

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

char *gunstick_texto[]={
        "None",
        "Sinclair 1",
        "Sinclair 2",
        "Kempston",
    "AYChip",
    "Port DFH"
};


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

int gunstick_view_electron_colors_by_putpixelcache(int x,int y)
{
    //Proteccion para que no se salga de putpixel_cache
    //dimensiones putpixel_cache
    int ancho,alto;

    ancho=screen_get_emulated_display_width_no_zoom();
    alto=screen_get_emulated_display_height_no_zoom();

    if (x<ancho && y<alto) {

        //No mirar color. si coincide el electron por donde lee el programa, dar por bueno
        //printf("Electron is on gunstick. return TRUE\n");
        //return 1;

        //Ver si hay algo en blanco cerca de donde se ha disparado
        int indice_cache;
                    //x=gunstick_x;

        int rango_y;
        y=gunstick_y-gunstick_range_y/2;

        //Restar offset
        y=y-gunstick_y_offset;

        if (y<0) y=0;
        for (rango_y=gunstick_range_y;rango_y>0;rango_y--,y++) {


            x=gunstick_x; //-gunstick_range_x/2;
            if (x<0) x=0;

            indice_cache=(get_total_ancho_rainbow()*y)+x;

            int rango_x;
            z80_byte color;
            for (rango_x=gunstick_range_x;rango_x>0;rango_x--) {

                color=putpixel_cache[indice_cache];

                //color blanco con o sin brillo
                //si no es color valido
                //if (color>15) return 0;


                /*
                int maskbrillo=7;
                if (gunstick_solo_brillo) maskbrillo=15;

                //if ( (color&maskbrillo)>0) {
                if ( (color&maskbrillo)==maskbrillo) {
                    debug_printf (VERBOSE_DEBUG,"White zone detected on lightgun. gunstick x: %d y: %d, color=%d",gunstick_x,gunstick_y,color);
                    return 1;
                }*/

                if (color!=0 && color!=8) {
                    debug_printf (VERBOSE_DEBUG,"Non black zone detected on lightgun. gunstick x: %d y: %d, color=%d",gunstick_x,gunstick_y,color);
                    return 1;
                }

                indice_cache++;

                //printf ("rango_x: %d rango_y: %d x: %d y: %d\n",rango_x,rango_y,x,y);

            }
        }
    }

    return 0;
}


//Ver si la zona donde apunta el raton (gunstick) esta en blanco. Usado en gunstick de MHT
int gunstick_view_white(void)
{

//Si curses
#ifdef COMPILE_CURSES
    if (!strcmp(scr_new_driver_name,"curses")) {
        return (scrcurses_return_gunstick_view_white() );
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

    if (gunstick_x<ancho && gunstick_y<alto) {
        int indice_cache;

        indice_cache=(get_total_ancho_rainbow()*gunstick_y)+gunstick_x;

        z80_byte color=putpixel_cache[indice_cache];

        //color blanco con o sin brillo
        if (color==15 || color==7) {
            debug_printf (VERBOSE_DEBUG,"white zone detected on lightgun");
            return 1;
        }
    }

    return 0;
}

int gunstick_view_pixel_color(int x,int y)
{

    //
    // ESTA RUTINA FUNCIONA BIEN. NO TOCAR!!!
    //

    if (x>255 || y>191 || x<0 || y<0) {

        int color=out_254 & 7;
        printf("out of range. border color %d\n",color);
        //retornar color border
        return color;
    }

    int dir_atributo=y/8;
    dir_atributo *=32;
    dir_atributo +=x/8;

    int dir_pixel=screen_addr_table[y*32+(x/8)];

    int bit_pixel=x % 8;

    printf("dpixel %d datributo %d\n",dir_pixel,dir_atributo);


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

    printf("## x %3d y %3d color %d\n",x,y,color);

    return color;

}

int gunstick_view_electron_colors(void)
{

    int x,y;

    //Proteccion para que no se salga de putpixel_cache
    //dimensiones putpixel_cache
    int ancho,alto;

    ancho=screen_get_emulated_display_width_no_zoom();
    alto=screen_get_emulated_display_height_no_zoom();

    //printf("## x %d y %d\n",x,y);


    //Ver si hay algo en blanco cerca de donde se ha disparado

    int rango_y;
    y=gunstick_y-gunstick_range_y/2;

    //Restar offset
    y=y-gunstick_y_offset;

    if (y<0) y=0;
    for (rango_y=gunstick_range_y;rango_y>0;rango_y--,y++) {


        x=gunstick_x;

        int rango_x;
        z80_byte color;
        for (rango_x=0;rango_x<gunstick_range_x;rango_x++) {

            printf("gunstick %3d %3d\n",x,y);
            color=gunstick_view_pixel_color(x+rango_x-screen_testados_total_borde_izquierdo*2,
                                            y+rango_y-screen_borde_superior);

            if (color!=0 && color!=8) {
                debug_printf (VERBOSE_DEBUG,"Non black zone detected on lightgun. gunstick x: %d y: %d, color=%d",gunstick_x,gunstick_y,color);
                return 1;
            }


        }
    }


    return 0;
}




//Ver si la zona donde apunta el raton (gunstick) esta pasando el electron. Usado en magnum light phaser
int gunstick_view_electron(void)
{
    //ver electron

    //x inicialmente esta posicionada dentro de pantalla... sin contar border
    int x=t_estados % screen_testados_linea;
    int y=t_estados/screen_testados_linea;

    //restamos zona no visible superior
    //printf ("y calculada: %d t_scanline_draw: %d\n",y,t_scanline_draw);
    y -=screen_invisible_borde_superior;

    x=x+screen_testados_total_borde_izquierdo;


    if (x>=screen_testados_linea) {
        y++;
        x=x-screen_testados_linea;
        printf("SALTADO LINEA\n");
    }


   //x esta en t_estados. pasamos a pixeles

    x=x*2;



        int si_salta_linea;
        int new_x,new_y;
        new_x=screen_get_x_coordinate_tstates(&si_salta_linea);

        new_y=screen_get_y_coordinate_tstates();



        //int x=new_x;
        //int y=new_y+si_salta_linea;




    debug_printf (VERBOSE_PARANOID,"electron is at t_estados: %6d x: %3d y: %3d new x %3d new y %3d. gun is at x: %3d y: %3d",
        t_estados,x,y,new_x,new_y+si_salta_linea,gunstick_x,gunstick_y);

    printf ("electron is at t_estados: %6d x: %3d y: %3d new x %3d new y %3d. gun is at x: %3d y: %3d\n",
        t_estados,x,y,new_x,new_y+si_salta_linea,gunstick_x,gunstick_y);


    //Nuevo calculo para saber si esta en rango. Mediante offset total
    int electron_offset=y*(screen_testados_linea*2)+x;
    int gunstick_offset=gunstick_y*(screen_testados_linea*2)+gunstick_x;

    int max_offset=gunstick_range_y*(screen_testados_linea*2)+gunstick_range_x;
    int delta_offset=electron_offset-gunstick_offset;

    printf("Offsets electron %6d gunstick %6d offset %7d max_offset %6d\n",electron_offset,gunstick_offset,delta_offset,max_offset);

/*
    //rango de y
    int dif_y=y-gunstick_y;
    if (dif_y<0) dif_y=-dif_y;

    if (dif_y>=gunstick_range_y) {
        printf("Electron is NOT on gunstick, it's on other scanline. return FALSE\n");
        return 0;
    }

    //Si el electrón no ha llegado donde está el mouse, retornar 0
    if (x<gunstick_x) {
        printf("Electron is NOT on gunstick, it's to the LEFT. return FALSE\n");
        return 0;
    }

    int dif_x=x-gunstick_x;
    if (dif_x<0) dif_x=-dif_x;



    if (dif_x>gunstick_range_x) {
        printf("Electron is NOT on gunstick, it's to the RIGHT (dif_x=%d). return FALSE\n",dif_x);
        return 0;
    }
*/



    if (delta_offset<0) {
        printf("No ha llegado aun el electron a donde esta la pistola\n");
        return 0;
    }

    if (delta_offset>max_offset) {
        printf("Electron esta muy lejos de la pistola\n");
        return 0;
    }

    printf("Electron esta en rango de la pistola\n");

    debug_printf (VERBOSE_DEBUG,"gunstick y (%d) is in range of electron (%d)",gunstick_y,y);

    //return gunstick_view_electron_colors_by_putpixelcache(x,y);
    return gunstick_view_electron_colors();


  //}

}


void joystick_print_types(void)
{
    int i;

    for (i=0;i<=JOYSTICK_TOTAL;i++) {
        printf ("%s",joystick_texto[i]);
        if (i!=JOYSTICK_TOTAL) printf (", ");
    }
}
