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

#include "tv.h"
#include "zx8081.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "core_zx8081.h"
#include "operaciones.h"
#include "zxvision.h"
#include "ula.h"

/*

TV (CRT) emulation routines.
This module emulates a TV, it's only used on ZX80 and ZX81 but could be used on more machines in the future, like CPC for example

*/

/*

TODO

vsync_per_second
zx8081_detect_vsync_sound

*/

//Contador de tiempo del CRT, para saber donde esta
int tv_time=0;

//En que linea esta el electron
int tv_y=0;
//En que coordenada x está el electron (multiple de 8)
int tv_x=0;

//Señales del CRT
int tv_vsync_signal=0;
int tv_vsync_signal_length=0;

int tv_hsync_signal=0;
int tv_hsync_signal_pending=0;



void tv_time_event_store_chunk_image_sprite(int x,int y,z80_byte byte_leido,int colortinta,int colorpapel)
{
    int color;


    if (byte_leido & 128 ) color=colortinta;
    else color=colorpapel;

    rainbow_buffer[y*get_total_ancho_rainbow()+x]=color;


}



void tv_time_event_store_chunk_image(int delta)
{

    if (rainbow_enabled.v) {
        int y=tv_y;
        y=y-screen_invisible_borde_superior;

        int xorig=tv_x*2;
        int xmax=xorig+delta*2;

        int totalancho=get_total_ancho_rainbow();

        if (y>=0 && y<get_total_alto_rainbow() ) {
            int x;

            //if (y==42) printf("x %3d xmax %3d y %3d sprite %02XH\n",xorig,xmax,y,zx80801_last_sprite_video);

            for (x=xorig;x<xmax;x++) {

                if (x>=0 && x<totalancho )  {

                    //si linea no coincide con entrelazado, volvemos
                    if (if_store_scanline_interlace(y) ) {

                        //Este bloque es dependiente de la maquina
                        if (MACHINE_IS_ZX8081) {
                            //if (y==42) printf("x %3d bit %3d delta %d\n",x,bit_inicio,delta);
                            tv_time_event_store_chunk_image_sprite(x,y,zx80801_last_sprite_video,zx80801_last_sprite_video_tinta,zx80801_last_sprite_video_papel);

                            //Rotar circular
                            int bit_alto=(zx80801_last_sprite_video & 128 ? 1 : 0);
                            zx80801_last_sprite_video=zx80801_last_sprite_video<<1;
                            zx80801_last_sprite_video |=bit_alto;
                        }
                    }
                }
                else {
                    //printf("x fuera de rango: %d\n",x);
                }
            }
        }
    }

    tv_x +=delta;



    //Dejar el valor por defecto si la ula de la maquina no envia nada
    //Esto es dependiente del hardware
    //TODO fix
    /*
    if (MACHINE_IS_ZX8081) {
        zx80801_last_sprite_video=video_zx8081_ula_video_output;
        zx80801_last_sprite_video_tinta=0;
        zx80801_last_sprite_video_papel=15;
    }
    */

}

void tv_increase_line(void)
{

    tv_y++;

    //controlar vsync timeout
    int total_lineas=screen_testados_total/screen_testados_linea;
    //Si no se va mas alla del limite
    //El oscilador "libre" es de 50 hz
    //if (tv_y>(total_lineas*120)/100) {

    //le damos un pelin mas de margen
    if (tv_y>=total_lineas+5) {
        printf("vsync timeout en %d\n",tv_y);
        //sleep(1);

        //vsync solo mueve la y, no la X?
        //tv_x=0;

        tv_y=0;
    }
}

int temp_extend_debug=0;

int tv_linea_inicio_vsync=0;

int debug_first_vsync=0;

extern int contador_segundo;

int ejecutando_vsync=0;

//Funcion mas importante, evento de tiempo
void tv_time_event(int delta)
{
    //Hacer todo lo que corresponda desde tv_time hasta tv_time+delta-1
    //if (tv_vsync_signal==0 && tv_hsync_signal==0)
    //printf("TV x: %3d y: %3d hsync %d vsync %d\n",tv_x,tv_y,tv_hsync_signal,tv_vsync_signal);

    int move_electron=1;

    if (tv_vsync_signal && ejecutando_vsync) {
        //printf("Do not move electron on x=%d y=%d\n",tv_x,tv_y);
        move_electron=0;
    }

    if (move_electron) {
        tv_time_event_store_chunk_image(delta);
    }

    //Hay vsync? Y siempre que dure mas de xxx tiempo
    /*
    en PAL:
    A true VSync basically has a pulse of 2.5 scan lines (2.5 * 64us = 160us = 517.5 T-states at 3.25MHz).
Either side are scan lines containing pre and post equalizing pulses, but these can be ignored.
A VSync of 160us worked for my analogue TV using the RF connection. Since the ZX81 generates the pulse in software,
it can produce any length VSync it wants. It is then a matter of whether the TV is tolerant enough to accept it.
    */

    ejecutando_vsync=0;

    if (tv_vsync_signal && nmi_generator_active.v && hsync_generator_active.v) {
        //printf("####---- vsync y nmi generator y hsync generator en t_estados %6d Y=%d\n",t_estados,tv_get_y());
    }

    if (temp_extend_debug) printf("tv hsync pending en t_estados %6d Y=%d\n",t_estados,tv_get_y());

    if (tv_vsync_signal) {
        if (temp_extend_debug) printf("tv vsync enabled en t_estados %6d Y=%d\n",t_estados,tv_get_y());
        tv_vsync_signal_length+=delta;
        /*
        Qué sucede si hay VSync pero NO HSync

        El nivel de vídeo baja (aparenta un pulso vertical)

        El TV no recibe HSync dentro del intervalo

        La rampa vertical entra en free-run (corre a su frecuencia natural, no sincronizada)

        Resultado:

        El haz no llega arriba,

        La imagen “rueda” o se fragmenta

        Se ven bandas horizontales o patrones inestables

        Este es exactamente el comportamiento del ZX81 en modo FAST: el nivel de vídeo baja como VSync,
        pero no hay HSync, así que la TV no reinicia la rampa vertical.
        */

        //cuando el vsync es mayor de cierto valor, ya no se admite
        //esto es lo que sucede en modo fast, esta siempre vsync y al final el haz de electrones "se libera" y sigue dibujando
        //la pantalla en negro

        if (tv_hsync_signal) {
            //printf("*** hsync dentro de vsync en %d length %d\n",tv_y,tv_vsync_signal_length);
        }

        //TODO En principio vsync necesita hsync. Esto es lo que provoca que al hacer save, la imagen no se vaya arriba

        if (tv_vsync_signal_length>DEFAULT_MINIMO_DURACION_VSYNC && tv_vsync_signal_length<PERMITIDO_MAXIMO_DURACION_VSYNC) {
            //if (tv_y>=300) {
                //printf("TV vsync en %d length %d\n",tv_y,tv_vsync_signal_length);


                //vsync solo mueve la y, no la X?
                //tv_x=0;

                int sync=1;

                /*
                if (MACHINE_IS_ZX81_TYPE) {
                    if (tv_linea_inicio_vsync<MINIMA_LINEA_ADMITIDO_VSYNC) {
                        printf("Intento de inicio sync en %d\n",tv_linea_inicio_vsync);
                        sync=0;
                    }
                }
                */

                if (sync) {
                    if (debug_first_vsync) {
                        debug_first_vsync=0;
                        //printf("TV fired           vsync x: %6d y: %6d contador %d\n",tv_x,tv_y,contador_segundo);
                    }
                    tv_y=0;
                    ejecutando_vsync=1;
                    video_zx8081_ula_video_output=255;

                    //Con esto Mazogs se ve bien pero pacman o manic miner no
                    //video_zx8081_linecntr=0;
                }
            //}
        }

    }

    else {

        if (temp_extend_debug) printf("tv no hay vsync en t_estados %6d Y=%d\n",t_estados,tv_get_y());

        //Hay hsync?
        if (tv_hsync_signal && !ejecutando_vsync) {
            if (temp_extend_debug) printf("tv hsync signal en t_estados %6d Y=%d\n",t_estados,tv_get_y());
            tv_x=0;

        }

        if (tv_hsync_signal_pending) {
            tv_hsync_signal_pending=0;
            //printf("3) tv hsync fired en t_estados %6d Y=%d tv_vsync_signal=%d\n",t_estados,tv_get_y(),tv_vsync_signal);
            tv_increase_line();
            tv_x=0;
        }

    }

    //controlar hsync timeout
    //TODO: valor arbitrario de timeout
    /*
    Oscila aproximadamente a la frecuencia horizontal estándar (≈15.734 kHz en NTSC, ≈15.625 kHz en PAL)
    //screen_testados_total=screen_testados_linea*screen_scanlines;
    //eso son 20 ms. 50 hz
    //Al final es justo lo que tarda el scanline

    */
    //if (tv_x>(screen_testados_linea*110)/100 && !ejecutando_vsync) {

    //le damos un pelin mas de margen, si no el QS defenda no se ve completo por debajo
    if (tv_x>=screen_testados_linea+65 && !ejecutando_vsync) {
        printf("hsync timeout en x=%d y=%d\n",tv_x,tv_y);
        tv_x=0;
        tv_increase_line();
    }


    //Finalmente avanzar el tiempo
    tv_time +=delta;

    //Si da la vuelta, truncar
    if (tv_time>=screen_testados_total) {
        tv_time -=screen_testados_total;
    }

}

//En parametro delta decimos cuantos t-estados han pasado desde que se debia lanzar el hsync
//Ejemplo: si es 0, es ahora mismo
//Si es 1, el hsync viene de un t-estado antes
void tv_enable_hsync(void)
{
    if (tv_hsync_signal==0) {
        //printf("TV enable hsync x: %6d y: %6d\n",tv_x,tv_y);
        //sleep(1);
        tv_hsync_signal=1;
        tv_hsync_signal_pending=1;
    }
    else {
        //printf("Received hsync but was already enabled\n");
    }
}

void tv_disable_hsync(void)
{
    if (tv_hsync_signal) {
        //printf("TV disable hsync x: %6d y: %6d\n",tv_x,tv_y);
        //sleep(1);
        tv_hsync_signal=0;
        video_zx8081_ula_video_output=0;
    }
}

void tv_enable_vsync(void)
{
    if (simulate_lost_vsync.v) return;


    if (tv_vsync_signal==0) {

        //no admitir vsync si electron no esta por debajo de posicion ...?
        //Para estabilizar mazogs. lo hago solo para ZX81 por no romper la imagen en ZX80
        //TODO: esta emulación de TV no debería depender de la máquina
        //es probable que los timings de ZX81 no estén bien y por eso sucede eso


        if (MACHINE_IS_ZX81_TYPE) {

            if (tv_y<MINIMA_LINEA_ADMITIDO_VSYNC) {
                printf("TV tried to enable vsync x: %6d y: %6d\n",tv_x,tv_y);
                return;
            }
            else {
                printf("TV Enabled vsync         x: %6d y: %6d contador %d\n",tv_x,tv_y,contador_segundo);
            }

            tv_linea_inicio_vsync=tv_y;

            debug_first_vsync=1;
        }



        //printf("TV enable vsync x: %6d y: %6d\n",tv_x,tv_y);
        tv_vsync_signal=1;
        tv_vsync_signal_length=0;
        //sleep(1);
    }
}

void tv_disable_vsync(void)
{

    if (tv_vsync_signal) {
        //printf("TV disable vsync x: %6d y: %6d\n",tv_x,tv_y);
        tv_vsync_signal=0;
        //Con esto el titulo del menu de pacman se ve bien pero en el juego no
        //video_zx8081_linecntr=0;
        //sleep(1);
    }
}

int tv_get_y(void)
{
    return tv_y;
}

