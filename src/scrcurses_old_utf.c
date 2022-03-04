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
#include <ncursesw/curses.h>
#include <string.h>
#include <unistd.h>
#include <locale.h> 

#include "cpu.h"
#include "scrcurses.h"
#include "operaciones.h"
#include "mem128.h"
#include "debug.h"
#include "zx8081.h"
#include "screen.h"
#include "audio.h"
#include "menu.h"
#include "utils.h"
#include "joystick.h"
#include "ula.h"
#include "z88.h"
#include "sam.h"
#include "charset.h"
#include "tsconf.h"
#include "settings.h"
#include "chloe.h"
#include "timex.h"


#define CURSES_IZQ_BORDER 4
#define CURSES_TOP_BORDER 4


//contiene el puntero a la pantalla de spectrum, actualizado en varias funciones de scrcurses
unsigned char *scrcurses_screen;

int colores;
WINDOW * mainwin;

int curses_last_message_shown_timer=0;
char curses_last_message_shown[DEBUG_MAX_MESSAGE_LENGTH];
int curses_last_message_length=0;

void scrcurses_z88_cpc_load_keymap(void)
{
	debug_printf (VERBOSE_INFO,"Loading keymap");
}

void scrcurses_putpixel_final_rgb(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color_rgb GCC_UNUSED)
{
}

void scrcurses_putpixel_final(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color GCC_UNUSED)
{
}



//no hacer nada
//Aqui en teoria solo se llama desde opcion de view waveform
void scrcurses_putpixel(int x,int y,unsigned int color)
{

	//Para que no se queje el compilador de variable no usada
	x++;
	y++;
	color++;
}


void scrcurses_messages_debug(char *s)
{


//si hay un mensaje anterior, asegurarnos que se borra antes,
//comprobando el de mayor longitud
if (curses_last_message_length) {
	int t=strlen(curses_last_message_shown);
	if (t>curses_last_message_length) curses_last_message_length=t;
}

else curses_last_message_length=strlen(curses_last_message_shown);

sprintf (curses_last_message_shown,"%s",s);

//supuestamente 5 segundos (50*5)
curses_last_message_shown_timer=250;



}

//Rutina de putchar para menu
void scrcurses_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{

	int brillo;

        tinta=tinta&15;
        papel=papel&15;
	

	//brillo para papel o tinta
	//tinta=(tinta&7);

	if (papel>7 || tinta>7) {
		brillo=A_BOLD;
		papel=(papel&7);
		tinta=(tinta&7);
	}

	//Parece que en curses de mac solo se ven letras con brillo cuando tinta=papel


        else brillo=0;

        attron(COLOR_PAIR(tinta+papel*8+1));


	if (MACHINE_IS_Z88) {
	move(y,x);
	}
	else {
	move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);
	}

	addch(caracter|brillo);

}

#define CURSES_LINE_FOOTER (24+(CURSES_TOP_BORDER*2)*border_enabled.v+0+ (MACHINE_IS_TSCONF ? 4 : 0) )

#define CURSES_LINE_DEBUG_REGISTERS (24+(CURSES_TOP_BORDER*2)*border_enabled.v+3)
#define CURSES_LINE_MESSAGES (24+(CURSES_TOP_BORDER*2)*border_enabled.v+4)

void scrcurses_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel)
{

        int brillo;

        tinta=tinta&15;
        papel=papel&15;

        
        //brillo para papel o tinta
        tinta=(tinta&7);
        
        if (papel>7 || tinta>7) {
                brillo=A_BOLD;
                papel=(papel&7);
                tinta=(tinta&7);
        }

        
        else brillo=0;
        
        attron(COLOR_PAIR(tinta+papel*8+1));

	y +=CURSES_LINE_FOOTER;

	x +=CURSES_IZQ_BORDER*border_enabled.v;

        
	move(y,x);
        
        addch(caracter|brillo);
}

void scrcurses_blank_footer(void)
{       
        if (menu_footer==0) return;


        int brillo;

	z80_byte tinta=WINDOW_FOOTER_INK; 
	z80_byte papel=WINDOW_FOOTER_PAPER;

        
        //brillo para papel o tinta
        tinta=(tinta&7);
        
        if (papel>7 || tinta>7) {
                brillo=A_BOLD;
                papel=(papel&7);
                tinta=(tinta&7);
        }

        
        else brillo=0;
        
        attron(COLOR_PAIR(tinta+papel*8+1));
        
        int x,y;

        for (y=0;y<3;y++) {
                for (x=0;x<32+(2*CURSES_IZQ_BORDER)*border_enabled.v;x++) {
			move(y+CURSES_LINE_FOOTER,x);
			addch(' '|brillo);
                }
        }
}



void scrcurses_debug_registers(void)
{

	char texto_debug[2048];
	print_registers(texto_debug);

	attron(COLOR_PAIR(0+7*8+1));
	mvaddstr(CURSES_LINE_DEBUG_REGISTERS, 0, texto_debug);

}


void asigna_color_atributo(unsigned char atributo,int *brillo,int *parpadeo)
{

        int paper,ink;
        int copia_paper;


        ink = atributo & 7;
        paper = ( atributo >> 3 ) & 7;



        //parpadeo
        //no hacemos parpadeo mediante A_BLINK
        *parpadeo=0;
        //if (scrcurses_screen[offset] & 128) *parpadeo=A_BLINK;
        //else *parpadeo=0;

        if (atributo & 128) {
                //hay parpadeo
                if (estado_parpadeo.v) {
                        //estado de inversion de color
                        copia_paper=paper;
                        paper=ink;
                        ink=copia_paper;
                }
        }


        if (atributo & 64) *brillo=A_BOLD;
        else *brillo=0;

        attron(COLOR_PAIR(ink+paper*8+1));

}

//Asigna color al siguiente caracter, obteniendolo de la pantalla de spectrum
void asigna_color (int x,int y,int *brillo,int *parpadeo)
{

        int offset;
	unsigned char atributo;

        offset=6144;

        offset = offset + y*32 ;

        offset = offset +x ;

//      printf ("%d ",offset);

	atributo=scrcurses_screen[offset];

	if (scr_refresca_sin_colores.v) atributo=56;

	asigna_color_atributo(atributo,brillo,parpadeo);

/*


        ink = atributo & 7;
        paper = ( atributo >> 3 ) & 7;



	//parpadeo 
	//no hacemos parpadeo mediante A_BLINK
	*parpadeo=0;
        //if (scrcurses_screen[offset] & 128) *parpadeo=A_BLINK;
        //else *parpadeo=0;

	if (atributo & 128) {
		//hay parpadeo
		if (estado_parpadeo.v) {
			//estado de inversion de color
			copia_paper=paper;
			paper=ink;
			ink=copia_paper;
		}
	}


        if (atributo & 64) *brillo=A_BOLD;
        else *brillo=0;

        attron(COLOR_PAIR(ink+paper*8+1));

*/

}



/*


The whole matter becomes at good deal clearer if we look at the screen address in binary.


           High Byte                |               Low Byte

0   1   0   T   T   L   L   L          Cr Cr Cr Cc Cc Cc Cc Cc



I have used some abbreviations to make things a bit clearer:

T - these two bits refer to which third of the screen is being addressed:  00 - Top,  01 - Middle,    10 - Bottom

L - these three bits indicate which line is being addressed:  from 0 - 7, or 000 - 111 in binary

Cr - these three bits indicate which character row is being addressed:  from 0 - 7

Cc - these five bits refer to which character column is being addressed: from 0 - 31


The  top three bits ( 010 ) of the high byte don't change.


*/



void scrcurses_refresca_border_comun(z80_byte color)
{
	int x,y;
	//z80_byte color=out_254 & 7;



        attron(COLOR_PAIR(0+color*8+1));


	//parte superior
	for (y=0;y<CURSES_TOP_BORDER;y++) {
		for (x=0;x<32+CURSES_IZQ_BORDER*2;x++) {
                                move(y,x);
                                addch(' ');
		}
	}
	//parte inferior
        for (y=24+CURSES_TOP_BORDER;y<24+CURSES_TOP_BORDER*2;y++) {
                for (x=0;x<32+CURSES_IZQ_BORDER*2;x++) {
                                move(y,x);
                                addch(' ');

                }
        }

        //laterales
        for (y=0;y<24;y++) {
		for (x=0;x<CURSES_IZQ_BORDER;x++) {
			move(y+CURSES_TOP_BORDER,x);
			addch(' ');
			move(y+CURSES_TOP_BORDER,CURSES_IZQ_BORDER+32+x);
			addch(' ');
		}
        }


}

void scrcurses_refresca_border_sam_mode2(z80_byte color)
{
        int x,y;

        attron(COLOR_PAIR(0+color*8+1));


        //parte superior
        for (y=0;y<CURSES_TOP_BORDER;y++) {
                for (x=0;x<85+CURSES_IZQ_BORDER*2;x++) {
                                move(y,x);
                                addch(' ');
                }
        }
        //parte inferior
        for (y=24+CURSES_TOP_BORDER;y<24+CURSES_TOP_BORDER*2;y++) {
                for (x=0;x<85+CURSES_IZQ_BORDER*2;x++) {
                                move(y,x);
                                addch(' ');

                }
        }

        //laterales
        for (y=0;y<24;y++) {
                for (x=0;x<CURSES_IZQ_BORDER;x++) {
                        move(y+CURSES_TOP_BORDER,x);
                        addch(' ');
                        move(y+CURSES_TOP_BORDER,CURSES_IZQ_BORDER+85+x);
                        addch(' ');
                }
        }


}



void scrcurses_refresca_border(void) {
	z80_byte color;
	color=out_254 & 7;
	if (scr_refresca_sin_colores.v) color=7;
        scrcurses_refresca_border_comun(color);
}


//Para refrescado de pantalla en zx8081 y ace
void scrcurses_putchar_zx8081(int x,int y, z80_byte caracter)
{


	z80_bit inverse;

	//Caso especial para Jupiter ACE
	if (MACHINE_IS_ACE) {
		if (caracter>=128) {
			inverse.v=0;
			caracter -=128;
		}
		else inverse.v=1;

                if (!inverse.v) attron(COLOR_PAIR(0+7*8+1));
                else attron(COLOR_PAIR(7+0*8+1));


	}


	else {

		//Para ZX80/81


		caracter=da_codigo81(caracter,&inverse);
		if (!inverse.v) attron(COLOR_PAIR(0+7*8+1));
		else attron(COLOR_PAIR(7+0*8+1));


		//simular modo fast
		if (video_fast_mode_emulation.v==1 && video_fast_mode_next_frame_black==LIMIT_FAST_FRAME_BLACK) {
			attron(COLOR_PAIR(1));
			move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);
			addch(' ');
			return;
		}

	}


	move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

        addch(caracter);


}

void scrcurses_refresca_pantalla_zx81(void)
{


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
                        if (video_fast_mode_next_frame_black==LIMIT_FAST_FRAME_BLACK) scrcurses_refresca_border_comun(0);
                        else scrcurses_refresca_border_comun(7);
                        modificado_border.v=0;
                }

        }



	scr_refresca_pantalla_zx8081();




	//refresh();


}


void scrcurses_refresca_pantalla_ace(void)
{
        if (border_enabled.v) {
                //ver si hay que refrescar border
                if (modificado_border.v)
                {
                        scrcurses_refresca_border_comun(0);
                        modificado_border.v=0;
                }

        }



        scr_refresca_pantalla_ace();

}

void scrcurses_refresca_pantalla_no_rainbow(void)
{

        char caracter;
        int x,y;
        unsigned char inv;

        int valor_get_pixel;

        int brillo,parpadeo;

	char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";

	  scrcurses_screen=get_base_mem_pantalla();

	  for (y=0;y<24;y++) {
                for (x=0;x<32;x++) {


                        caracter=compare_char(&scrcurses_screen[  calcula_offset_screen(x,y)  ] , &inv);

                        if (colores) {
                          asigna_color(x,y,&brillo,&parpadeo);
                        }
                        else {
                          brillo=0;
                        }


                        if (caracter) {

                                move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);
                        }

                        else {

                                inv=0;


                                if (texto_artistico.v==1) {
                                        //si caracter desconocido, hacerlo un poco mas artistico
                                        valor_get_pixel=0;
                                        if (scr_get_4pixel(x*8,y*8)>=umbral_arttext) valor_get_pixel+=1;
                                        if (scr_get_4pixel(x*8+4,y*8)>=umbral_arttext) valor_get_pixel+=2;
                                        if (scr_get_4pixel(x*8,y*8+4)>=umbral_arttext) valor_get_pixel+=4;
                                        if (scr_get_4pixel(x*8+4,y*8+4)>=umbral_arttext) valor_get_pixel+=8;

                                        caracter=caracteres_artisticos[valor_get_pixel];
       				}

                                else caracter='?';



                                move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);
                                //addch('~'|brillo);
				//
	//char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";

   if (valor_get_pixel==13) {
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

   else if (valor_get_pixel==1) {
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
				const cchar_t wch = {A_NORMAL, L"▖"};
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

   else {
                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);
  }

                        }

                }



          }


}


void scrcurses_refresca_pantalla_chloe(void)
{

        z80_byte caracter;
        int x,y;
        unsigned char inv;

        //int valor_get_pixel;

        //int parpadeo;

        //char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";

	z80_byte *chloe_screen;

          chloe_screen=chloe_home_ram_mem_table[7];

	chloe_screen += (0xd800-0xc000); //text display in offset d800 in ram 7

	//Colores chloe
	//int papel=get_timex_paper_mode6_color();
	//int tinta=get_timex_ink_mode6_color();
	int brillo=0;
	int parpadeo=0;

	//unsigned char atributo=brillo*64+papel*8+tinta;

	unsigned char atributo_ega=peek_byte_no_time(23693);
	//High four bits are foreground, low four bits are background
	int papel=atributo_ega&7;
	int tinta=(atributo_ega>>4)&7;
	//En curses solo tenemos 8 colores. Descartamos brillo pues no lo hace bien la consola



	papel=screen_ega_to_spectrum_colour(papel);
	tinta=screen_ega_to_spectrum_colour(tinta);

	unsigned char atributo=tinta+papel*8;

	//in normal video do OUT 255,6. and you can do COLOR f,b

          for (y=0;y<24;y++) {
                for (x=0;x<80;x++,chloe_screen++) {


                        caracter=*chloe_screen; 

			brillo=0;
			inv=0;

			

                        if (colores) {
                          //(x,y,&brillo,&parpadeo);
			  asigna_color_atributo(atributo,&brillo,&parpadeo);
                        }
                        else {
                          brillo=0;
                        }
			

			if (caracter==0) {
				//caracter='C'; //copyright character
				caracter=' '; //blank space until it is fixed in the rom
			}

			if (caracter<32 || caracter>126) caracter='?';


                                move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);

                }

          }

}

void sam_temp_debug_char(z80_byte *buffer_letra)
{
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


void scrcurses_refresca_pantalla_sam_modo_013_fun_color(z80_byte color, int *brillo, int *parpadeo)
{

                                if (colores) {
                                  asigna_color_atributo(color,brillo,parpadeo);
                                }
                                else {
                                  *brillo=0;
                                }

}

void scrcurses_refresca_pantalla_sam_modo_013_fun_caracter(int x,int y,int brillo, unsigned char inv,z80_byte caracter )
{
                       move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);

}

void scrcurses_refresca_pantalla_sam_modo_013(int modo)
{
	scr_refresca_pantalla_sam_modo_013(modo,scrcurses_refresca_pantalla_sam_modo_013_fun_color,scrcurses_refresca_pantalla_sam_modo_013_fun_caracter);
}

void scrcurses_refresca_pantalla_sam_modo_2(void)
{
	scr_refresca_pantalla_sam_modo_2(scrcurses_refresca_pantalla_sam_modo_013_fun_color,scrcurses_refresca_pantalla_sam_modo_013_fun_caracter);
}



void scrcurses_refresca_pantalla_sam(void)
{
	z80_byte modo_video=(sam_vmpr>>5)&3;

	                 if (border_enabled.v) {
                        //ver si hay que refrescar border
                        if (modificado_border.v)
                        {
				if (modo_video!=2) {
                                	scrcurses_refresca_border_comun(sam_border&7);
				}

				else scrcurses_refresca_border_sam_mode2(sam_border&7);
				
                                modificado_border.v=0;
                        }

                }


	switch (modo_video) {
		case 0:
			scrcurses_refresca_pantalla_sam_modo_013(0);
		break;

		case 1:
			scrcurses_refresca_pantalla_sam_modo_013(1);
		break;

		case 2:
			scrcurses_refresca_pantalla_sam_modo_2();
		break;

		case 3:
			scrcurses_refresca_pantalla_sam_modo_013(3);
		break;
	}
}


//Refrescar pantalla en zx80/81 con real video
void scrcurses_refresca_pantalla_zx8081_rainbow(void)
{
        z80_byte caracter;
        int x,y;
        //unsigned char inv;

        int valor_get_pixel;

        char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";


	z80_int direccion;

	//Tabla de caracteres para ZX80,81
        if (MACHINE_IS_ZX80) direccion=0x0E00;
        else direccion=0x1E00;

	z80_byte inverse;

	//Nuestro caracter en pantalla a comparar con tabla de caracteres
	z80_byte caracter_sprite[8];

	//Alto de pantalla total en curses
	int alto=24;
	alto=alto+2*CURSES_TOP_BORDER*border_enabled.v;

	//Ancho de pantalla total en curses
	int ancho=32;
	ancho=ancho+2*CURSES_IZQ_BORDER*border_enabled.v;

	//Posicion en pantalla rainbow / 8
	//0,0 indica inicio de rainbow
	int yinicial=0;
	int xinicial=0;

	//Nos situamos en el pequeño border definido para curses
	yinicial=yinicial+screen_borde_superior*border_enabled.v/8-CURSES_TOP_BORDER*border_enabled.v;

	xinicial=xinicial+screen_total_borde_izquierdo*border_enabled.v/8-CURSES_IZQ_BORDER*border_enabled.v;

	//Posicion en pantalla curses
	int xencurses,yencurses;

	for (y=yinicial,yencurses=0;y<yinicial+alto;y++,yencurses++) {
                for (x=xinicial,xencurses=0;x<xinicial+ancho;x++,xencurses++) {
                
			int spritelin;
			caracter=255;

			//Buscar caracteres en posicion y...y+7
				for (spritelin=0;spritelin<8 && caracter==255;spritelin++) {
					screen_get_sprite_char(x*8,y*8+spritelin,caracter_sprite);
					caracter=compare_char_tabla_rainbow(caracter_sprite,&inverse,&memoria_spectrum[direccion]);
					//if (caracter) debug_printf (VERBOSE_ERR,"xx: %d spritelin: %d caracter: %d ",xx,spritelin,caracter);
				}
			

                        if (caracter!=255) {

				//printf ("hay caracter :%d ",caracter);
				z80_bit inv;
				if (inverse) caracter+=128;
				caracter=da_codigo81(caracter,&inv);
				attron(COLOR_PAIR(0+7*8+1));


                                //move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);
                                move(yencurses,xencurses);
				if (inv.v) addch(caracter | WA_REVERSE  );
				else addch(caracter);


                        }

                        else {

                                if (texto_artistico.v==1) {
                                        //si caracter desconocido, hacerlo un poco mas artistico
                                        valor_get_pixel=0;
                                        if (scr_get_4pixel_rainbow(x*8,y*8)>=umbral_arttext) valor_get_pixel+=1;
                                        if (scr_get_4pixel_rainbow(x*8+4,y*8)>=umbral_arttext) valor_get_pixel+=2;
                                        if (scr_get_4pixel_rainbow(x*8,y*8+4)>=umbral_arttext) valor_get_pixel+=4;
                                        if (scr_get_4pixel_rainbow(x*8+4,y*8+4)>=umbral_arttext) valor_get_pixel+=8;

                                        caracter=caracteres_artisticos[valor_get_pixel];

					attron(COLOR_PAIR(0+7*8+1));
                                 }

                                else caracter='?';


                                //move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);
                                move(yencurses,xencurses);

				addch(caracter);

                        }

                }


          }


}

void scrcurses_refresca_pantalla_z88_new_line(struct s_z88_return_character_atributes *z88_caracter)
{

	//Para que no se queje el compilador de no usado
	if (z88_caracter) {}

}

void scrcurses_refresca_pantalla_z88_print_char(struct s_z88_return_character_atributes *z88_caracter)
{

	int caracter=z88_caracter->ascii_caracter;


                                //Gestion inverse
                                if (z88_caracter->inverse) {
                                        caracter |= WA_REVERSE;
                                }

                                //Gestion subrallado
                                if (z88_caracter->subrallado) {
                                       caracter |= WA_UNDERLINE; 
                                }

                                //Gestion parpadeo
                                if (z88_caracter->parpadeo) {
                                        caracter |= WA_BLINK;
                                }

                                //Gestion gris
                                if (z88_caracter->gris) {
                                        caracter |= WA_DIM;
                                }



                        //Si caracter no es nulo

                        if (z88_caracter->null_caracter==0) {
				//attron(COLOR_PAIR(7+0*8+1));
				attron(COLOR_PAIR(0+7*8+1));
				move(z88_caracter->y,z88_caracter->x);
				addch(caracter);
                        }

}



void scrcurses_refresca_pantalla_z88(void)
{
        struct s_z88_return_character_atributes z88_caracter;

        z88_caracter.f_new_line=scrcurses_refresca_pantalla_z88_new_line;
        z88_caracter.f_print_char=scrcurses_refresca_pantalla_z88_print_char;


        screen_repinta_pantalla_z88(&z88_caracter);

}



void scrcurses_refresca_pantalla_cpc_fun_color(z80_byte color, int *brillo, int *parpadeo)
{

                                if (colores) {
                                  asigna_color_atributo(color,brillo,parpadeo);
                                }
                                else {
                                  *brillo=0;
                                }

}


void scrcurses_refresca_pantalla_common_fun_color(z80_byte color, int *brillo, int *parpadeo)
{

                                if (colores) {
                                  asigna_color_atributo(color,brillo,parpadeo);
                                }
                                else {
                                  *brillo=0;
                                }

}

void scrcurses_refresca_pantalla_cpc_fun_saltolinea(void)
{

//En este driver no hacemos nada

}


void scrcurses_refresca_pantalla_common_fun_saltolinea(void)
{

//En este driver no hacemos nada

}

void scrcurses_refresca_pantalla_cpc_fun_caracter(int x,int y,int brillo, unsigned char inv,z80_byte caracter )
{
                       move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);

}

void scrcurses_refresca_pantalla_common_fun_caracter(int x,int y,int brillo, unsigned char inv,z80_byte caracter )
{
                       move(y+CURSES_TOP_BORDER*border_enabled.v,x+CURSES_IZQ_BORDER*border_enabled.v);

                                if (inv) addch(caracter | WA_REVERSE | brillo );
                                else addch(caracter|brillo);

}



void scrcurses_refresca_pantalla_cpc(void)
{
        
	if (border_enabled.v) {
                        //ver si hay que refrescar border
                        if (modificado_border.v)
                        {
                                //TODO
                                modificado_border.v=0;
                        }
	}


scr_refresca_pantalla_cpc_text(scrcurses_refresca_pantalla_cpc_fun_color,scrcurses_refresca_pantalla_cpc_fun_caracter,scrcurses_refresca_pantalla_cpc_fun_saltolinea);
}


void scrcurses_refresca_pantalla_solo_driver(void)
{
        //Como esto solo lo uso de momento para drivers graficos, de momento lo dejo vacio
}


void scrcurses_refresca_pantalla(void)
{
	//int i;
	//char c;
	//int brillo,parpadeo;

	//char caracteres_artisticos[]=" ''\".|/r.\\|7_LJ#";

	//char my_string[2];
	//char caracter;
	//int x,y;
	//unsigned char inv;

	//int valor_get_pixel;


        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;


	if (MACHINE_IS_ZX8081) {

                if (rainbow_enabled.v==0) {
			//modo clasico. sin rainbow
			scrcurses_refresca_pantalla_zx81();
		}

                else {
                        //modo rainbow - real video. De momento hacemos igual que sin realvideo
			//scrcurses_refresca_pantalla_zx81();

			scrcurses_refresca_pantalla_zx8081_rainbow();
		}
	}

	else if (MACHINE_IS_TSCONF) {
                   //Si es modo texto, hacer este refresh:
                    z80_byte modo_video=tsconf_get_video_mode_display();
                    if (modo_video==3) {
                                        scr_refresca_pantalla_tsconf_text_textmode(scrcurses_refresca_pantalla_common_fun_color,scrcurses_refresca_pantalla_common_fun_caracter,scrcurses_refresca_pantalla_common_fun_saltolinea,23);
                    }

		    else {

	                //con rainbow
			if (rainbow_enabled.v) {
	                        scr_refresca_pantalla_tsconf_text(scrcurses_refresca_pantalla_common_fun_color,scrcurses_refresca_pantalla_common_fun_caracter,scrcurses_refresca_pantalla_common_fun_saltolinea,19);  //23 seria 720x576 -> 31x25
			}

			else {

			//sin rainbow, refresh como spectrum
                        if (border_enabled.v) {
                                //ver si hay que refrescar border
                                if (modificado_border.v)
                                {
                                        scrcurses_refresca_border();
                                        modificado_border.v=0;
                                }

                        }

                        scrcurses_refresca_pantalla_no_rainbow();

			}
		  }


	}

	else if (MACHINE_IS_CHLOE) {
		scrcurses_refresca_pantalla_chloe();
	}


	else if (MACHINE_IS_SPECTRUM && !MACHINE_IS_TSCONF) { 

	        if (rainbow_enabled.v==0) {
        		//modo clasico. sin rainbow



			if (border_enabled.v) {
				//ver si hay que refrescar border
				if (modificado_border.v)
				{
					scrcurses_refresca_border();
					modificado_border.v=0;
				}

		  	}  

			scrcurses_refresca_pantalla_no_rainbow();


		}

		else {





                        //modo rainbow - real video. De momento hacemos igual que sin realvideo
                        //scrcurses_refresca_pantalla_rainbow();

			if (border_enabled.v) {
                                //ver si hay que refrescar border
                                if (modificado_border.v)
                                {
                                        scrcurses_refresca_border();
                                        modificado_border.v=0;
                                }

                        }

                        scrcurses_refresca_pantalla_no_rainbow();

			}



          }



	else if (MACHINE_IS_Z88) {
		//Si esta vofile activo, hay que dibujar dentro del buffer rainbow
		if (vofile_inserted.v) {
			set_z88_putpixel_zoom_function();
			screen_z88_refresca_pantalla_comun();
		}
		
		scrcurses_refresca_pantalla_z88();
	}


	else if (MACHINE_IS_ACE) {
		scrcurses_refresca_pantalla_ace();
	}

	else if (MACHINE_IS_SAM) {
                scrcurses_refresca_pantalla_sam();
        }

	else if (MACHINE_IS_CPC) {
		scrcurses_refresca_pantalla_cpc();
	}


	if (curses_last_message_shown_timer) {
	        curses_last_message_shown_timer--;



		if (curses_last_message_shown_timer==0) {
			//borrar mensaje con espacios
	        	attron(COLOR_PAIR(7+1));
			int mensaje_long=strlen(curses_last_message_shown);
			for (;mensaje_long;mensaje_long--)
				mvaddstr(CURSES_LINE_MESSAGES, mensaje_long-1, " ");
		}
        
		else  {
			
			//borramos lo que queda del texto anterior
			attron(COLOR_PAIR(7+1));
			if (curses_last_message_length) {

		                for (;curses_last_message_length;curses_last_message_length--)
	                        	mvaddstr(CURSES_LINE_MESSAGES, curses_last_message_length-1, " ");
			}

			//y mostramos el mensaje
	        	attron(COLOR_PAIR(0+7*8+1));
			mvaddstr(CURSES_LINE_MESSAGES, 0, curses_last_message_shown);

		}
	}

	screen_render_menu_overlay_if_active();


        //Escribir footer
        draw_middle_footer();


	refresh();


sem_screen_refresh_reallocate_layers=0;


}


unsigned char retorna_color_curses (unsigned char c) 
{

	switch (c) {

		case 0:
		return COLOR_BLACK;
		break;

		case 1:
		return COLOR_BLUE;
		break;

		case 2:
		return COLOR_RED;
		break;

		case 3:
		return COLOR_MAGENTA;
		break;

		case 4:
		return COLOR_GREEN;
		break;

		case 5:
		return COLOR_CYAN;
		break;

		case 6:
		return COLOR_YELLOW;
		break;

		case 7:
		return COLOR_WHITE;
		break;

		default:
		return COLOR_BLACK;
	}

}


void scrcurses_inicializa_colores(void)
{

	int color=1;
	int paper,ink;
	for (paper=0;paper<8;paper++) {

		for (ink=0;ink<8;ink++)	{
			if (inverse_video.v==0) init_pair (color,retorna_color_curses(ink),retorna_color_curses(paper));
			else init_pair (color,retorna_color_curses(7-ink),retorna_color_curses(7-paper));
			color++;
		}
	}
		

}


void scrcurses_fade_color(int color)
{
	if (color>=1 && color<=64) init_pair(color,COLOR_BLACK,COLOR_BLACK);

}


//extern unsigned char char_set[];
//unsigned char *tabla=char_set;



void scrcurses_end(void)
{
	debug_printf (VERBOSE_INFO,"Closing curses video driver");
	clear();
	endwin();

	//Al finalizar curses deja todo bien excepto el flushing de stdout... todas las llamadas a printf deberan tener un 
	//fflush(stdout); sino no se vera el texto

}

void scrcurses_set_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scrcurses_reset_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}


//        y +=CURSES_LINE_FOOTER;

//        x +=CURSES_IZQ_BORDER*border_enabled.v;


//        move(y,x);

//        addch(caracter|brillo);

int last_x_detectedchar_print=0;

void scrcurses_detectedchar_print(z80_byte caracter)
{
	//attron(COLOR_PAIR(tinta+papel*8+1));
	attron(COLOR_PAIR(0+7*8+1));


	move(CURSES_LINE_FOOTER+3,last_x_detectedchar_print);
	if (caracter>=32) addch(caracter);

	last_x_detectedchar_print++;
	if (last_x_detectedchar_print==32 || caracter=='\n') {
		last_x_detectedchar_print=0;
		//borrar esa zona
		int x;
		for (x=0;x<32;x++) {
			move (CURSES_LINE_FOOTER+3,x);
			addch(' ');
		}
	}

        //printf ("%c",caracter);
        //flush de salida standard
        //fflush(stdout);

}

//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrcurses_get_menu_width(void)
{
        return 32;
}


int scrcurses_get_menu_height(void)
{
        return 24;
}


int scrcurses_driver_can_ext_desktop (void)
{
        return 0;
}

int scrcurses_init (void) {

	debug_printf (VERBOSE_INFO,"Init Curses Video Driver");

    
	/*  Initialize ncurses  */


	if ( (mainwin = initscr()) == NULL ) {
		debug_printf (VERBOSE_ERR,"Error initialising ncurses.");
		return 1;
	}

	nodelay(mainwin,TRUE);
	keypad(mainwin,TRUE);


        //cbreak();
	//cbreak overrides raw
        noecho();

	//pruebas para que el teclado del zx80/81 vaya mejor... sin exito
	intrflush(mainwin,TRUE);



	wtimeout(mainwin,0);
	notimeout(mainwin, TRUE);

	//enviar todas teclas y no gestionar ni CTRL+C 
	raw();

	mousemask(ALL_MOUSE_EVENTS, NULL);


	curs_set(0);

	if(has_colors() == FALSE)
	{
		
		colores=0;
	}

	else {
//		if (COLOR_PAIRS>=8*8) {
			colores=1;
			start_color();
			scrcurses_inicializa_colores();
			scr_tiene_colores=1;
//		}

//		else colores=0;
	}

setlocale(LC_ALL, "");
scr_putchar_zx8081=scrcurses_putchar_zx8081;
scr_debug_registers=scrcurses_debug_registers;
                        scr_messages_debug=scrcurses_messages_debug;
scr_putchar_menu=scrcurses_putchar_menu;
scr_putchar_footer=scrcurses_putchar_footer;

        scr_get_menu_width=scrcurses_get_menu_width;
        scr_get_menu_height=scrcurses_get_menu_height;
scr_driver_can_ext_desktop=scrcurses_driver_can_ext_desktop;

scr_putpixel=scrcurses_putpixel;
scr_putpixel_final=scrcurses_putpixel_final;
scr_putpixel_final_rgb=scrcurses_putpixel_final_rgb;


scr_set_fullscreen=scrcurses_set_fullscreen;
scr_reset_fullscreen=scrcurses_reset_fullscreen;
scr_z88_cpc_load_keymap=scrcurses_z88_cpc_load_keymap;
scr_detectedchar_print=scrcurses_detectedchar_print;

//sprintf (curses_last_message_shown,"");
curses_last_message_shown[0]=0x0;

//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
scr_set_driver_name("curses");


scrcurses_blank_footer();

menu_init_footer();

return 0;

}


//Convierte coordenadas mouse curses a gunstick, kempston mouse
void scrcurses_convert_mouse_xy(int curses_x,int curses_y)
{
	
/*
//Coordenadas x,y en formato scanlines y pixeles totales, es decir,
//x entre 0 y 351 
//y entre 0 y 295
//0,0 esta arriba a la izquierda

int gunstick_x,gunstick_y;

//Coordenadas x,y tal cual las retorna el driver de video, segun el tamanyo de ventana activo
int mouse_x=0,mouse_y=0;

//Coordenadas x,y de retorno a puerto kempston
//Entre 0 y 255 las dos. Coordenada Y hacia abajo resta
//se toma como base el mismo formato que gunstick x e y pero con modulo % 256
z80_byte kempston_mouse_x=0,kempston_mouse_y=0;

*/

	//Como el border en curses no es de tamanyo real ni proporcional al real, hay que comprobar si el cursor
	//esta en el border o no

	if (border_enabled.v==0) {
		//Sumar borde superior e inferior
		curses_x +=screen_total_borde_izquierdo/8;
		curses_y +=screen_borde_superior/8;
	}


	else {
		//Sumar trozo del border correspondiente

		int border_izq_entre_ocho=screen_total_borde_izquierdo/8;
		int border_arr_entre_ocho=screen_borde_superior/8;

		if (curses_x<CURSES_IZQ_BORDER) {
			//Dentro del border
			curses_x = (curses_x*border_izq_entre_ocho)/CURSES_IZQ_BORDER;
		}
		else {
			curses_x=(curses_x-CURSES_IZQ_BORDER)+border_izq_entre_ocho;
		}


                if (curses_y<CURSES_TOP_BORDER) {
			//Dentro del border
                        curses_y = (curses_y*border_arr_entre_ocho)/CURSES_TOP_BORDER;
                }       
                else {
                        curses_y=(curses_y-CURSES_TOP_BORDER)+border_arr_entre_ocho;
                }

	}

	gunstick_x=curses_x*8;
	gunstick_y=curses_y*8;

	kempston_mouse_x=(gunstick_x)%256;
	kempston_mouse_y=255-(gunstick_y)%256;
}
	


#define SCRCURSES_MAX_CONTADOR_NOTECLA 5

//contador que se activa cuando no hay tecla pulsada. Cuando se llegue a max, notificar realmente a core z80 que no hay tecla pulsada
//Esto es debido a que el driver de curses, cuando se deja tecla pulsada, retorna alternativamente ERR y tecla, segun un factor de repeticion
//Para que ese ERR no se interprete como no tecla pulsada, damos un minimo de veces que se debe suceder para que realmente se considere no tecla
int scrcurses_contador_notecla=0;

void scrcurses_actualiza_tablas_teclado(void)
{



        int c;

        c = getch();

        //printf ("Tecla: %d  \r",c);


        if (c==ERR) {
                if (scrcurses_contador_notecla<=SCRCURSES_MAX_CONTADOR_NOTECLA) {
                        scrcurses_contador_notecla++;
                        //hasta que no pasen unas cuantas veces, no liberar esa tecla
                        return;
                }
        }

        //printf ("Tecla: %d  \r",c);

        //inicializar todas las teclas a nada - 255
	reset_keyboard_ports();

	//inicializar botones de raton a nada
	mouse_left=mouse_right=0;


	MEVENT event;



        if (c!=ERR) {
        //printf ("Tecla: %d  \r",c);
                scrcurses_contador_notecla=0;


		notificar_tecla_interrupcion_si_z88();

		if (c==27) {
                                        //ALT
                                        //printf ("Alt\n");

                                        set_symshift();

                                        c = getch();
                                        if (c==ERR) {
                                                //pulsado ESC
						util_set_reset_key(UTIL_KEY_ESC,1);

                                        }
		}
		
		//simular esc en menu con @
		if (c=='@' && menu_abierto) {
		  util_set_reset_key(UTIL_KEY_ESC,1);
		  return;
		}

		if (c==KEY_F(1)) {
			util_set_reset_key(UTIL_KEY_F1,1);
		}

		else if (c==KEY_F(2)) {
                        util_set_reset_key(UTIL_KEY_F2,1);
                }

                else if (c==KEY_F(3)) {
                        util_set_reset_key(UTIL_KEY_F3,1);
                }

                else if (c==KEY_F(4)) {
                        util_set_reset_key(UTIL_KEY_F4,1);
                }

                else if (c==KEY_F(5)) {
                        util_set_reset_key(UTIL_KEY_F5,1);
                }


		//F6 es Ctrl /diamond
		//F7 es Alt / square
                else if (c==KEY_F(6)) {
                        util_set_reset_key(UTIL_KEY_CONTROL_L,1);
                }

                else if (c==KEY_F(7)) {
                        util_set_reset_key(UTIL_KEY_ALT_L,1);
                }


                else if (c==KEY_F(8)) {
                        util_set_reset_key(UTIL_KEY_F8,1);
                }

                else if (c==KEY_F(9)) {
                        util_set_reset_key(UTIL_KEY_F9,1);
                }

                else if (c==KEY_F(10)) {
                        util_set_reset_key(UTIL_KEY_F10,1);
                }



		switch (c) {
			        case KEY_HOME:
                                        joystick_set_fire();
                                break;

                                case KEY_LEFT:
                                        joystick_set_left();
                           		blink_kbd_a12 &= (255-64);
                                break;

                                case KEY_RIGHT:
                                        joystick_set_right();
					blink_kbd_a11 &= (255-64);
                                break;

                                case KEY_DOWN:
                                        joystick_set_down();
					blink_kbd_a10 &= (255-64);
                                break;

                                case KEY_UP:
                                        joystick_set_up();
					blink_kbd_a9 &= (255-64);
                                break;

                                case KEY_BACKSPACE:
				//En algunos terminales, como Mac, genera 127
				//Se puede tambien simular mediante CTRL-H
				case 127:
                                        puerto_65278 &=255-1;
                                        puerto_61438 &=255-1;
					blink_kbd_a8 &= (255-128);
                                break;

                                //TAB Emula shift+symbol -> Extended
                                case 9:
                                        puerto_32766 &=255-2;
                                        puerto_65278 &=255-1;
					blink_kbd_a14 &= (255-32);
                                break;

	                        //PgUp
        	                case 339:

                                        puerto_especial1 &=255-2;
                	        break;

                        	//PgDn
	                        case 338:

                                        puerto_especial1 &=255-4;
        	                break;


				//Mouse
				case KEY_MOUSE:
					//printf ("evento mouse\n");
					if (getmouse(&event) == OK) {
						//coordenadas (0,0) arriba a la izquierda
						if (event.bstate & BUTTON1_PRESSED) {
							util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,1);
							//mouse_left=1;
						}
						if (event.bstate & BUTTON3_PRESSED) {
							util_set_reset_mouse(UTIL_MOUSE_RIGHT_BUTTON,1);
							//mouse_right=1;
						}

						/* Los release no se reciben casi nunca
						por tanto los desactivamos y hacemos que por defecto, al hacer reset_keys, luego metemos botones a 0
						Por tanto luego el lightgun no suele funcionar porque libera a un tiempo diferente
						del que esperan los juegos
						*/
						
						if (event.bstate & BUTTON1_RELEASED) {
							util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,0);
							//mouse_left=0;
						}
						if (event.bstate & BUTTON3_RELEASED) {
							util_set_reset_mouse(UTIL_MOUSE_RIGHT_BUTTON,0);
							//mouse_right=0;
						}
						

						mouse_x=event.x;
						mouse_y=event.y;
						scrcurses_convert_mouse_xy(mouse_x,mouse_y);

						//printf ("gunstick x: %d y: %d kempst x: %d y: %d\n",gunstick_x,gunstick_y,kempston_mouse_x,kempston_mouse_y);
						
					}
				break;



			default:
        	        	ascii_to_keyboard_port(c);
			break;

		}
	}

}

int scrcurses_return_gunstick_view_white(void)
{

	//printf ("view white\n");

	chtype caracter;

	move(mouse_y,mouse_x);
	
	caracter=inch();

	int color=((caracter & A_COLOR)>>8)-1;
	//printf ("color: %d\n",color);

	int tinta=(color&7);
	int papel=(color>>3)&7;

	//int brillo=(  (caracter & A_BOLD) ? 1 : 0 );

	//printf ("tinta: %d papel: %d brillo: %d\n",tinta,papel,brillo);

	//color blanco con o sin brillo
	//sea tinta o papel. esto es muy aproximado, en juegos como targetplus funciona bien,
	//pero no en solo.tzx
	if (papel==7 || tinta==7) {
		debug_printf (VERBOSE_DEBUG,"white zone detected on lightgun");
		//printf ("white zone detected on lightgun\n");
		return 1;
	}

	return 0;
}

	


z80_byte scrcurses_lee_puerto(z80_byte puerto_h,z80_byte puerto_l)
{

        //Para evitar warnings al compilar de "unused parameter"
        puerto_h=puerto_l;
        puerto_l=puerto_h;


	//en ncurses no se usa

	return 255;
}




void scrcurses_z88_draw_lower_screen(void)
{
	int x,y;

        //attron(COLOR_PAIR(tinta+papel*8+1));
        attron(COLOR_PAIR(7+0*8+1));


                for (y=8;y<24;y++) {
                        for (x=0;x<106;x++) {
				move(y,x);
				addch (' ');
                        }
                }

}


