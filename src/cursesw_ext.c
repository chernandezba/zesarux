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

/*
compilar con:
CFLAGS= ...  -D_GNU_SOURCE -D_DEFAULT_SOURCE -I/usr/include/ncursesw -lncursesw -ltinfo

LDFLAGS= ... -D_GNU_SOURCE -D_DEFAULT_SOURCE -I/usr/include/ncursesw -lncursesw -ltinfo

Que sale de:

ncursesw5-config --cflags --libs

*/

#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <locale.h> 

#include "cpu.h"
#include "debug.h"
#include "settings.h"
#include "cursesw_ext.h"

//Rutinas para usar caracteres utf "blocky" para simular mejor los pixeles mediante texto


//Parece que este tipo de caracteres no permite brillo
void cursesw_ext_print_pixel(int valor_get_pixel)
{

        
   if (valor_get_pixel==1) {
				const cchar_t wch = {A_NORMAL, L"▘"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==2) {
				const cchar_t wch = {A_NORMAL, L"▝"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==3) {
				const cchar_t wch = {A_NORMAL, L"▀"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==4) {
				const cchar_t wch = {A_NORMAL, L"▖"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==5) {
				const cchar_t wch = {A_NORMAL, L"▌"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==6) {
				const cchar_t wch = {A_NORMAL, L"▞"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==7) {
				const cchar_t wch = {A_NORMAL, L"▛"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==8) {
				const cchar_t wch = {A_NORMAL, L"▗"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==9) {
				const cchar_t wch = {A_NORMAL, L"▚"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==10) {
				const cchar_t wch = {A_NORMAL, L"▐"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==11) {
				const cchar_t wch = {A_NORMAL, L"▜"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==12) {
				const cchar_t wch = {A_NORMAL, L"▄"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==13) {
				const cchar_t wch = {A_NORMAL, L"▙"};
				add_wch(&wch);
                                	
   }   


   else if (valor_get_pixel==14) {
				const cchar_t wch = {A_NORMAL, L"▟"};
				add_wch(&wch);
   }

   else if (valor_get_pixel==15) {
				const cchar_t wch = {A_NORMAL, L"█"};
				add_wch(&wch);
   }


   else {
	   //0 o cualquier otro.
				const cchar_t wch = {A_NORMAL, L" "};
				add_wch(&wch);
   }
   

}


void cursesw_ext_init(void)
{
	if (use_scrcursesw.v) setlocale(LC_ALL, "");
}

