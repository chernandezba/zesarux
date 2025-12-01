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
    int bit,color;

    for (bit=0;bit<8;bit++) {
        if (byte_leido & 128 ) color=colortinta;
        else color=colorpapel;

        rainbow_buffer[y*get_total_ancho_rainbow()+x+bit]=color;

        byte_leido=(byte_leido&127)<<1;

    }
}



void tv_time_event_store_chunk_image(int delta)
{
    if (0/*rainbow_enabled.v*/) {

        int y=tv_y;


        y=y-screen_invisible_borde_superior;


        int x=tv_x;

        //temp
        //x=128;
        //y=88;

        //if (y>=0) printf("store sprite x: %d y: %d\n",x,y);

        int totalancho=get_total_ancho_rainbow();



        if (y>=0 && y<get_total_alto_rainbow() ) {



            if (x>=0 && x<totalancho )  {

                //si linea no coincide con entrelazado, volvemos
                if (if_store_scanline_interlace(y) ) {

                    z80_byte sprite=zx80801_last_sprite_video;
                    sprite=0x01;
                    //screen_store_scanline_char_zx8081(x,y,sprite,caracter,caracter_inverse.v);
                    tv_time_event_store_chunk_image_sprite(x,y,sprite,0,15);
                }
            }
            else {
                //printf("x fuera de rango: %d\n",x);
            }
        }
    }

    //TODO incrementar esto 1 a 1 en bucle
    tv_x +=delta*2;

}

//Funcion mas importante, evento de tiempo
void tv_time_event(int delta)
{
    //Hacer todo lo que corresponda desde tv_time hasta tv_time+delta-1
    if (tv_vsync_signal==0 && tv_hsync_signal==0) printf("tv_time: %6d x: %3d y: %3d hsync %d vsync %d\n",tv_time,tv_x,tv_y,tv_hsync_signal,tv_vsync_signal);


    tv_time_event_store_chunk_image(delta);

    //Hay vsync? Y siempre que dure mas de xxx tiempo
    /*
    en PAL:
    A true VSync basically has a pulse of 2.5 scan lines (2.5 * 64us = 160us = 517.5 T-states at 3.25MHz).
Either side are scan lines containing pre and post equalizing pulses, but these can be ignored.
A VSync of 160us worked for my analogue TV using the RF connection. Since the ZX81 generates the pulse in software,
it can produce any length VSync it wants. It is then a matter of whether the TV is tolerant enough to accept it.
    */

    if (tv_vsync_signal) {
        tv_vsync_signal_length+=delta;

        if (tv_vsync_signal_length>DEFAULT_MINIMO_DURACION_VSYNC) {
            //printf("TV vsync\n");
            tv_x=0;
            tv_y=0;
        }
    }

    else {

        //Hay hsync?
        if (tv_hsync_signal) {
            tv_x=0;
            if (tv_hsync_signal_pending) {
                tv_hsync_signal_pending=0;
                int total_lineas=screen_testados_total/screen_testados_linea;
                //Si no se va mas alla del limite
                if (tv_y<=total_lineas) {
                    printf("Incrementar Y por hsync. y antes: %d\n",tv_y);
                    //sleep(1);
                    tv_y++;
                }
            }
        }
    }


    //Finalmente avanzar el tiempo
    tv_time +=delta;

    //Si da la vuelta, truncar
    if (tv_time>=screen_testados_total) {
        tv_time -=screen_testados_total;
    }

}

void tv_enable_hsync(void)
{
    if (tv_hsync_signal==0) {
        printf("TV enable hsync x: %6d y: %6d\n",tv_x,tv_y);
        //sleep(1);
        tv_hsync_signal=1;
        tv_hsync_signal_pending=1;
    }
}

void tv_disable_hsync(void)
{
    if (tv_hsync_signal) {
        printf("TV disable hsync x: %6d y: %6d\n",tv_x,tv_y);
        //sleep(1);
        tv_hsync_signal=0;
    }
}

void tv_enable_vsync(void)
{

    if (tv_vsync_signal==0) {
        printf("TV enable vsync x: %6d y: %6d\n",tv_x,tv_y);
        tv_vsync_signal=1;
        tv_vsync_signal_length=0;
        //sleep(1);
    }
}

void tv_disable_vsync(void)
{

    if (tv_vsync_signal) {
        printf("TV disable vsync x: %6d y: %6d\n",tv_x,tv_y);
        tv_vsync_signal=0;
        //sleep(1);
    }
}