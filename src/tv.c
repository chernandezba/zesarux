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
#include "settings.h"

/*

TV (CRT) emulation routines.
This module emulates a TV, it's only used on ZX80 and ZX81 but could be used on more machines in the future, like CPC for example

*/



//Contador de tiempo del CRT, para saber donde esta
int tv_time=0;

//En que linea esta el electron
int tv_y=0;
//En que coordenada x está el electron (multiple de 8)
int tv_x=0;

//Señales del CRT
//int tv_vsync_signal=0;
int tv_vsync_signal_length=0;

int tv_hsync_signal=0;
int tv_hsync_signal_pending=0;

//minima longitud aceptada de vsync para tenerse en cuenta, en microsegundos
int tv_minimum_accepted_vsync=DEFAULT_TV_MINIMO_DURACION_VSYNC;

//intervalo, en porcentaje sobre 20ms, cada cuanto se acepta un vsync (contando desde principio a principio de siguiente vsync)
int tv_vsync_minimum_accepted_interval=DEFAULT_TV_MINIMUM_VSYNC_ACCEPTED_INTERVAL;

//maximo tiempo que puede durar una linea, en microsegundos, si no llega señal de hsync
//habitualmente deberia de ser de 64 microsegundos
//Estos 84 sirve para que el juego de QS Defenda se vean hasta las ultimas lineas del final
int tv_max_line_period=DEFAULT_TV_MAX_LINE_PERIOD;

//Maximo de lineas antes de lanzar un vsync interno si no se recibe dicha señal
int tv_max_lines=DEFAULT_TV_MAX_LINES;

void tv_time_event_store_chunk_image_sprite(int x,int y,z80_byte byte_leido,int colortinta,int colorpapel)
{
    int color;

    if (byte_leido & 128 ) color=colortinta;
    else color=colorpapel;

    rainbow_buffer[y*get_total_ancho_rainbow()+x]=color;

}

int tv_return_effective_y_coordinate(int y)
{
    return y-screen_invisible_borde_superior;
}

void tv_time_event_store_chunk_image(int delta)
{

    if (rainbow_enabled.v) {
        int xorig=tv_x*2;
        int y=tv_return_effective_y_coordinate(tv_y);

        //TODO: esto se hace completamente a mano, sirve para ZX80 y ZX81
        if (border_enabled.v==0) {
            if (MACHINE_IS_ZX80) {
                xorig=xorig-screen_total_borde_derecho;
                y=y-screen_borde_superior+8;
            }

            if (MACHINE_IS_ZX81) {
                xorig=xorig-screen_total_borde_derecho-2;
                y=y-screen_borde_superior+7;
            }
        }



        int xmax=xorig+delta*2;

        int totalancho=get_total_ancho_rainbow();

        int totalalto=get_total_alto_rainbow();

        //printf("ancho %d alto %d\n",totalancho,totalalto);

        if (y>=0 && y<totalalto) {
            int x;

            for (x=xorig;x<xmax;x++) {

                if (x>=0 && x<totalancho )  {

                    //si linea no coincide con entrelazado, volvemos
                    if (if_store_scanline_interlace(y) ) {

                        //Este bloque es dependiente de la maquina
                        if (MACHINE_IS_ZX8081) {
                            //if (y==42) printf("x %3d bit %3d delta %d\n",x,bit_inicio,delta);
                            //printf("Store x %3d y %3d sprite %02XH tinta %3d papel %3d\n",
                            //    x,y,zx80801_last_sprite_video,zx80801_last_sprite_video_tinta,zx80801_last_sprite_video_papel);

                            tv_time_event_store_chunk_image_sprite(x,y,zx80801_last_sprite_video,zx80801_last_sprite_video_tinta,zx80801_last_sprite_video_papel);

                            //Rotar circular
                            int bit_alto=(zx80801_last_sprite_video & 128 ? 1 : 0);
                            zx80801_last_sprite_video=zx80801_last_sprite_video<<1;
                            zx80801_last_sprite_video |=bit_alto;
                        }
                    }
                }

            }
        }
    }

    tv_x +=delta;




}

//Renderizar zonas que quedan mas alla de vsync y hsync
void tv_draw_line_beyond_syncs(int x_inicio,int y)
{

    if (rainbow_enabled.v) {

        int totalancho=get_total_ancho_rainbow();

        if (y>=0 && y<get_total_alto_rainbow() ) {
            int x;


            for (x=x_inicio;x<totalancho;x++) {


                //si linea no coincide con entrelazado, volvemos
                if (if_store_scanline_interlace(y) ) {
                    int color;

                    //video_zx8081_ula_video_output color negro: 255
                    //tinta normalmente a 0
                    //papel normalmente a 15

                    if (video_zx8081_ula_video_output) color=zx80801_last_sprite_video_tinta;
                    else color=zx80801_last_sprite_video_papel;

                    //opcion debug para mostrarlo en rojo
                    if (menu_debug_show_zones_beyond_sync.v) color=2;

                    rainbow_buffer[y*totalancho+x]=color;
                }


            }
        }
    }


}

void tv_draw_lines_beyond_vsync(int y)
{
    y=tv_return_effective_y_coordinate(y);
    int total_alto=get_total_alto_rainbow();

    for (;y<total_alto;y++) {
        tv_draw_line_beyond_syncs(0,y);
    }
}

void tv_draw_line_beyond_hsync(int y)
{
    //printf("tv_x %d\n",tv_x);
    y=tv_return_effective_y_coordinate(y);

    tv_draw_line_beyond_syncs(tv_x*2,y);
}

void tv_increase_line(void)
{

    tv_y++;

    //controlar vsync timeout
    //int total_lineas=screen_testados_total/screen_testados_linea;
    //Si no se va mas alla del limite
    //El oscilador "libre" es de 50 hz
    //if (tv_y>(total_lineas*120)/100) {

    //le damos un pelin mas de margen
    if (tv_y>tv_max_lines) {
        //printf("vsync timeout en %d\n",tv_y);


        //vsync solo mueve la y, no la X?
        //tv_x=0;
        if (tv_y>100) {
            tv_draw_lines_beyond_vsync(tv_y);
        }

        tv_y=0;
    }
}




//Cuanto ha pasado desde el inicio del ultimo vsync
int last_vsync_time_passed=0;


enum tv_vsync_status_list tv_vsync_status=VSYNC_DISABLED;

const char *string_tv_vsync_status_disabled="off";
const char *string_tv_vsync_status_received_not_accepted_yet="on_small";
const char *string_tv_vsync_status_accepting="on_accepting";
const char *string_tv_vsync_status_exceeded_length="on_exceed_lenght";
const char *string_tv_vsync_status_undefined="undefined";

const char *tv_get_vsync_signal_status_string(void)
{
    switch (tv_vsync_status) {
        case VSYNC_DISABLED:
            return string_tv_vsync_status_disabled;
        break;

        case VSYNC_RECEIVED_NOT_ACCEPTED_YET:
            return string_tv_vsync_status_received_not_accepted_yet;
        break;

        case VSYNC_ACCEPTING:
            return string_tv_vsync_status_accepting;
        break;

        case VSYNC_EXCEEDED_LENGTH:
            return string_tv_vsync_status_exceeded_length;
        break;

        default:
            return string_tv_vsync_status_undefined;
        break;
    }
}

//Funcion mas importante, evento de tiempo
void tv_time_event(int delta)
{
    //Hacer todo lo que corresponda desde tv_time hasta tv_time+delta-1
    //if (tv_vsync_signal==0 && tv_hsync_signal==0)
    //printf("TV x: %3d y: %3d hsync %d vsync %d\n",tv_x,tv_y,tv_hsync_signal,tv_vsync_signal);


    if (tv_vsync_status!=VSYNC_ACCEPTING) {
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



    if (tv_vsync_status!=VSYNC_DISABLED) {

        //printf("tv vsync enabled en t_estados %6d Y=%d\n",t_estados,tv_get_y());
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

        int minimo_t_estados;
        int maximo_t_estados=(PERMITIDO_MAXIMO_DURACION_VSYNC*screen_testados_linea)/64;
        int minimo_entre_vsyncs;

        switch (tv_vsync_status) {
            case VSYNC_RECEIVED_NOT_ACCEPTED_YET:
                minimo_t_estados=(tv_minimum_accepted_vsync*screen_testados_linea)/64;
                //printf("minimo %d\n",minimo_t_estados);


                //Para empezarse a aceptar vsync tiene que ser mayor que un minimo y ademas, desde el anterior inicio de vsync
                //hasta este, debe pasar casi 20 ms (90% de ese valor, por defecto)
                minimo_entre_vsyncs=(screen_testados_total*tv_vsync_minimum_accepted_interval)/100;
                    //printf("Try to enable vsync x: %3d y: %3d\n",tv_x,tv_y);
                    //printf("--- delta: %6d total frame %6d minimo vsync %d\n",last_vsync_time_passed,screen_testados_total,minimo);
                    //con 10% menos del tiempo de frame total, ya sirve como vsync

                //printf("tv_vsync_signal_length %d minimo_t_estados %d maximo_t_estados %d last_vsync_time_passed %d\n",
                //    tv_vsync_signal_length,minimo_t_estados,maximo_t_estados,last_vsync_time_passed);

                if (tv_vsync_signal_length>=minimo_t_estados && tv_vsync_signal_length<maximo_t_estados && last_vsync_time_passed>=minimo_entre_vsyncs) {

                    //printf("-Llegado al minimo de %d (%d)\n",minimo_entre_vsyncs,last_vsync_time_passed);


                    //printf("-TV enable vsync x: %3d y: %3d\n",tv_x,tv_y);
                    last_vsync_time_passed=0;

                    //Cambiar de estado
                    tv_vsync_status=VSYNC_ACCEPTING;

                    //printf("Cambiar a tv_vsync_status=VSYNC_ACCEPTING\n");
                    vsync_per_second++;

                }

            break;

            case VSYNC_ACCEPTING:
                //Debug de a partir que linea se ha hecho vsync
                if (tv_y>100) {
                    tv_draw_lines_beyond_vsync(tv_y);
                }

                tv_y=0;

                //Si se pasa longitud vsync
                //Esto pasa por ejemplo al hacer modo fast y "1 goto 1"

                if (tv_vsync_signal_length>maximo_t_estados) {

                    //printf("-Llegado al maximo de %d\n",maximo_t_estados);


                    //Cambiar de estado
                    tv_vsync_status=VSYNC_EXCEEDED_LENGTH;

                    //printf("Cambiar a tv_vsync_status=VSYNC_EXCEEDED_LENGTH\n");

                }

            break;

            default:
                //Cualquier otro caso, no hacer nada
            break;
        }

    }

    else {

        //printf("tv no hay vsync en t_estados %6d Y=%d\n",t_estados,tv_get_y());

        //Hay hsync?
        if (tv_hsync_signal && tv_vsync_status!=VSYNC_ACCEPTING) {
            //printf("tv hsync signal en tv_time %6d Y=%d\n",tv_time,tv_get_y());
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
    /*
    Oscila aproximadamente a la frecuencia horizontal estándar (≈15.734 kHz en NTSC, ≈15.625 kHz en PAL)
    //screen_testados_total=screen_testados_linea*screen_scanlines;
    //eso son 20 ms. 50 hz
    //Al final es justo lo que tarda el scanline

    */


    //le damos un pelin mas de margen, si no el QS defenda no se ve completo por debajo
    int microsec_max=(tv_max_line_period*screen_testados_linea)/64;

    if (tv_x>=microsec_max && tv_vsync_status!=VSYNC_ACCEPTING) {
        //printf("hsync timeout en x=%d y=%d\n",tv_x,tv_y);
        tv_increase_line();
        tv_x=0;
    }


    //Finalmente avanzar el tiempo
    tv_time +=delta;

    //Si da la vuelta, truncar - ha pasado 20 milisegundos
    if (tv_time>=screen_testados_total) {
        tv_time -=screen_testados_total;
    }

    last_vsync_time_passed +=delta;

}



void tv_enable_hsync(void)
{
    if (simulate_lost_hsync.v) return;

    if (tv_hsync_signal==0) {
        //printf("TV enable hsync x: %6d y: %6d\n",tv_x,tv_y);
        tv_hsync_signal=1;
        tv_hsync_signal_pending=1;
        tv_draw_line_beyond_hsync(tv_y);
    }

}

void tv_disable_hsync(void)
{
    if (tv_hsync_signal) {
        //printf("TV disable hsync x: %6d y: %6d\n",tv_x,tv_y);
        tv_hsync_signal=0;
    }
}



void tv_enable_vsync(void)
{
    if (simulate_lost_vsync.v) return;

    //Parche prueba para que no haga vsync al hacer SAVE
    //Juegos afectados. QS defenda (ZX80), space invaders 1k, HERO.81, orquesta, SAVE
    //if (tv_y<280) return;
    //Habria que contar que el tiempo pasado entre cada vsync sea cercano a 20 ms

    if (tv_vsync_status==VSYNC_DISABLED) {

        //printf("-TV enable vsync x: %3d y: %3d\n",tv_x,tv_y);

        tv_vsync_status=VSYNC_RECEIVED_NOT_ACCEPTED_YET;
        //printf("Cambiar a tv_vsync_status=VSYNC_RECEIVED_NOT_ACCEPTED_YET\n");

        tv_vsync_signal_length=0;

    }

}

void tv_disable_vsync(void)
{

    if (tv_vsync_status!=VSYNC_DISABLED) {
        //printf("TV disable vsync x: %3d y: %3d length: %d\n",tv_x,tv_y,tv_vsync_signal_length);

        tv_vsync_status=VSYNC_DISABLED;

        //printf("Cambiar a tv_vsync_status=VSYNC_DISABLED\n");


    }
}

int tv_get_x(void)
{
    return tv_x;
}

int tv_get_y(void)
{
    return tv_y;
}


int tv_get_time(void)
{
    return tv_time;
}


int tv_get_hsync_signal(void)
{
    return tv_hsync_signal;
}

