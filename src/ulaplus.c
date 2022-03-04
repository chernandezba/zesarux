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


#include "ulaplus.h"
#include "screen.h"
#include "debug.h"
#include "spectra.h"
#include "menu.h"



/*
Ulaplus incompatible con scanlines o gigascreen
Ula plus al cambiar un color, clear putpixel
Ula plus tabla de conversión de valor r,g de 3 bits a 8 bits. Tabla de valor b de 2 bits a 8 bits

Ula plus http://scratchpad.wikia.com/wiki/ZX_Spectrum_64_Colour_RGB_Mode

http://scratchpad.wikia.com/wiki/ULAplus

https://sites.google.com/site/ulaplus/home


*/

//Indica si esta presente el soporte de ulaplus
z80_bit ulaplus_presente={0};



//Indica si esta activo ulaplus
z80_bit ulaplus_enabled={0};

//Ultimo valor enviado a puerto BF3B
z80_byte ulaplus_last_send_BF3B;

//Ultimo valor enviado a puerto FF3B
z80_byte ulaplus_last_send_FF3B;

//modo ulaplus standard actual. modo 0 es desactivado (paleta normal spectrum), modo 1 rgb, modo 3 radastan
z80_byte ulaplus_mode=0;

//modo ulaplus extendido actual. modo 0 es desactivado (paleta normal spectrum), modo 3 radastan, modos 1,5,9 ZEsarUX
z80_byte ulaplus_extended_mode=0;

//Paleta actual de ulaplus
z80_byte ulaplus_palette_table[64];

//Registro 64 de ulaplus
//z80_byte ulaplus_register_64;

/*
How do you switch on the 64-colour mode?
OUT 48955,64: OUT 65339,1.

How do you switch off the 64-colour mode?
OUT 48955,64: OUT 65339,0.
*/

/*

0xBF3B is the register port (write only)

The byte output will be interpreted as follows:

Bits 0-5: Select the register sub-group
Bits 6-7: Select the register group. Two groups are currently available:
     00 - palette group
          When this group is selected, the sub-group determines the
          entry in the palette table (0-63).
     01 - mode group
          The sub-group must be zero. Sub-group entries 1-63 are
          reserved for mode specific controls.

0xFF3B is the data (read/write)

When the palette group is selected, the byte output will be interpreted as follows:

Bits 0-1: Blue intensity.
Bits 2-4: Red intensity.
Bits 5-7: Green intensity.

*/




//tabla de conversion de colores de rgb ulaplus (8 bit) a 32 bit
//Cada elemento del array contiene el valor rgb real, por ejemplo,
//un valor rgb 33 de ulaplus, en la posicion 33 de este array retorna el color en formato rgb de 32 bits
int ulaplus_rgb_table[256];
/*
Colores 8 bit rgb de ulaplus:
Bits 0-1: Blue intensity.
Bits 2-4: Red intensity.
Bits 5-7: Green intensity.

This mode uses a sub-set of 9-bit RGB. The low bit is duplicated (Bb becomes Bbb). This gives access to a fixed half the potential 512 colour palette. The jump in intensity happens in the lower range where it is less noticeable.

La tabla ulaplus_rgb_table tiene para cada valor posible de rgb de ula plus su correspondiente en 24 bits (8 bit r, 8 bit g, 8 bit b)


*/

//Inicializar tabla de colores de rgb ulaplus (8 bit) a 32 bit
//Esta ulaplus_rgb_table solo se utiliza directamente una sola vez, y es al inicializar la tabla de colores del emulador, que se queda en el offset ULAPLUS_INDEX_FIRST_COLOR
void init_ulaplus_table(void)
{
	debug_printf (VERBOSE_DEBUG,"Initializing ULAplus rgb color table conversion");
	//Cada componente r,g,b de ulaplus se pasa a 8 bits, sumando tendremos color de 24 bits
	//Componentes red y green son de 3 bits, y b es de 2 bits
	//Maximo r,g: 3 bits a 1 -> 7. maximo en 8 bits: 255 . 255/7=36.42
	//Valor para 2 por ejemplo = 2 * 36.42 = 72.85

	//Minitablas de conversion de 3 bits a 8 bits
	z80_byte color_3_to_8[8]={
	0,36,73,109,146,182,219,255
	};

	int color,color32;
	z80_byte r,g,b;
	z80_byte r8,g8,b8;

	for (color=0;color<256;color++) {
		r=(color>>2)&7;
		g=(color>>5)&7;

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
		ulaplus_rgb_table[color]=color32;

		debug_printf (VERBOSE_PARANOID,"ULAplus RGB 0x%02X is 0x%06X 32 bit RGB",color,color32);
	}

	debug_printf (VERBOSE_DEBUG,"Initializing ULAplus 64 colour table to black");
	int i;
	for (i=0;i<64;i++) ulaplus_palette_table[i]=0;

}



//Cambiar un color de la paleta ulaplus
//Entrada: indice entre 0...63 y color formato rgb8 de ulaplus
void ulaplus_change_palette_colour(z80_byte index,z80_byte color)
{
	//Actualizamos entrada en tabla ulaplus
    //printf ("change ulaplus color index: %d value: %02XH\n",index,color);
	ulaplus_palette_table[index]=color;

}


void disable_ulaplus(void)
{
	debug_printf (VERBOSE_INFO,"Disabling ULAplus");
	ulaplus_presente.v=0;

}

void enable_ulaplus(void)
{

  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable ULAplus on non Spectrum machine");
    return;
  }

        if (MACHINE_IS_TBBLUE) {
                //Ulaplus no está para tbblue
                return;
        }
        debug_printf (VERBOSE_INFO,"Enabling ULAplus");
        ulaplus_presente.v=1;

        //son excluyentes
        //disable_interlace();
	disable_scanlines();
	disable_gigascreen();
	spectra_disable();

        //necesita real video
        enable_rainbow();

}


//Para modos estandard ulaplus
void ulaplus_set_mode(z80_byte value)
{
                                //establecer modo
                                //modo 0: normal spectrum->ulaplus desactivado. modos 1 rgb. modos 2 y superiores reservados

                                z80_byte ulaplus_mode_anterior=ulaplus_mode;

                                ulaplus_mode=(value&63);

                                switch (ulaplus_mode) {
                                        case 0:
                                                debug_printf (VERBOSE_DEBUG,"Disabling ULAplus (mode 0)");
                                                ulaplus_enabled.v=0;
                                                if (ulaplus_mode!=ulaplus_mode_anterior) {
                                                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling ULAplus (mode 0)");
                                                }

                                        break;

                                        case 1:
                                                ulaplus_enabled.v=1;
                                                debug_printf (VERBOSE_DEBUG,"Enabling ULAplus mode 1. RGB");
                                                if (ulaplus_mode!=ulaplus_mode_anterior) {
                                                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling ULAplus mode 1. RGB");
                                                }
                                        break;

                                        default:
                                                debug_printf (VERBOSE_DEBUG,"Unknown ulaplus mode %d",ulaplus_mode);
                                        break;

				}
}


//Para modos extendidos ulaplus
void ulaplus_set_extended_mode(z80_byte value)
{
                                /*

				Implementado mediante registro 40H de zxuno:
				 el bit 1 indicaría, si está a 1, que se va a usar el modo radastaniano y el resto de bits, del 2 al 7, tienen un significado que me reservo por el momento. Si el bit 1 está a 0, entonces estaríamos en modos cesarianos, y el significado del resto de bits, del 2 al 7, queda a tu antojo. En ambos casos, el bit 0 indica, si está a 1, que se habilite el modo (el que sea). Si está a 0, se ignoran el resto de bits, del 1 al 7.

				Registro 40H

				Modo Radastan 128x96
				7 6 5 4 3 2 1 0
				R R R R R R 1 1

				R: Reservado

				Modos ZEsarUX:
				7 6 5 4 3  2  1 0
				R R R R Z1 Z0 0 1

				R: Reservado
				Z0, Z1: Bits bajo y alto de los modos ZEsarUX:
				0: 256x96  (valor del registro: 1)
                                1: 128x192 (valor del registro: 5)
                                2: 256x192 (valor del registro: 9)
				3: no definido
                                */

                                z80_byte ulaplus_mode_anterior=ulaplus_extended_mode;

                                ulaplus_extended_mode=(value&255);

				//printf ("ulaplus extended mode: %d\n",ulaplus_extended_mode);

                                switch (ulaplus_extended_mode) {
                                        case 0:
                                                debug_printf (VERBOSE_DEBUG,"Disabling ULAplus (extended mode 0)");
                                                ulaplus_enabled.v=0;
                                                if (ulaplus_extended_mode!=ulaplus_mode_anterior) {
                                                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling ULAplus (extended mode 0)");
                                                }

                                        break;

                                        case 3:
                                                ulaplus_enabled.v=1;
					        debug_printf (VERBOSE_DEBUG,"Enabling linear mode Radastan. 128x96");
                                                if (ulaplus_extended_mode!=ulaplus_mode_anterior) {
                                                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling linear mode Radastan. 128x96");
                                                }

                                        break;

                                        case 1:
                                                ulaplus_enabled.v=1;
                                                debug_printf (VERBOSE_DEBUG,"Enabling linear mode ZEsarUX 0. 256x96");
                                                if (ulaplus_extended_mode!=ulaplus_mode_anterior) {
                                                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling linear mode ZEsarUX 0. 256x96");
                                                }

                                        break;

                                        case 5:
                                                ulaplus_enabled.v=1;
                                                debug_printf (VERBOSE_DEBUG,"Enabling linear mode ZEsarUX 1. 128x192");
                                                if (ulaplus_extended_mode!=ulaplus_mode_anterior) {
                                                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling linear mode ZEsarUX 1. 128x192");
                                                }

                                        break;

                                        case 9:
                                                ulaplus_enabled.v=1;
                                                debug_printf (VERBOSE_DEBUG,"Enabling linear mode ZEsarUX 2. 256x192");
                                                if (ulaplus_extended_mode!=ulaplus_mode_anterior) {
                                                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling linear mode ZEsarUX 2. 256x192");
                                                }

                                        break;


                                        default:
                                                debug_printf (VERBOSE_DEBUG,"Unknown ulaplus mode %d",ulaplus_extended_mode);
                                        break;

                                }
}


z80_byte ulaplus_return_port_ff3b(void)
{
		//Retornar valor para el registro escogido, siempre considerando registros entre 0..63 o el registro 64
		/* Recordemos:
		Bits 0-5: Select the register sub-group
		Bits 6-7: Select the register group. Two groups are currently available:

		00 - palette group
		When this group is selected, the sub-group determines the entry in the palette table (0-63).

		01 - mode group
		...

		0xFF3B is the data port (read/write)

		When the palette group is selected, the byte written will describe the color.

		When the mode group is selected, the byte output will be interpreted as follows:

		Bit 0: ULAplus palette on (1) / off (0)
		....

		Por tanto al seleccionar 01-mode group (valor X1XXXXXXB) (mascara 64), al enviar valor a puerto ff3b se dice si se activa ulaplus o no -
		valor de variable ulaplus_mode
		*/
		z80_byte indice=ulaplus_last_send_BF3B;
		if (indice&64) return ulaplus_mode;
		return ulaplus_palette_table[indice&63];
}


void ulaplus_write_port(z80_int puerto,z80_byte value)
{
	z80_byte puerto_h=puerto>>8;

//register port
	if (puerto_h==0xbf) {
/*

I/O ports

ULAplus is controlled by two ports.

0xBF3B is the register port (write only)

The byte output will be interpreted as follows:

Bits 0-5: Select the register sub-group
Bits 6-7: Select the register group. Two groups are currently available:

00 - palette group
When this group is selected, the sub-group determines the entry in the palette table (0-63).

01 - mode group

The sub-group is (optionally) used to mirror the video functionality of Timex port #FF as follows:

Bits 0-2: Screen mode. 000=screen 0, 001=screen 1, 010=hi-colour, 110=hi-res (bank 5)
100=screen 0, 101=screen 1, 011=hi-colour, 111=hi-res (bank 7)
Bits 3-5: Sets the screen colour in hi-res mode.
000 - Black on White 100 - Green on Magenta
001 - Blue on Yellow 101 - Cyan on Red
010 - Red on Cyan 110 - Yellow on Blue
011 - Magenta on Green 111 - White on Black

0xFF3B is the data port (read/write)

When the palette group is selected, the byte written will describe the color.

When the mode group is selected, the byte output will be interpreted as follows:

Bit 0: ULAplus palette on (1) / off (0)
Bit 1: (optional) greyscale: on (1) / off (0) (same as turning the color off on the television)

Implementations that support the Timex video modes use the #FF register as the primary means to set the video mode, as per the Timex machines. It is left to the individual implementations to determine if reading the port returns the previous write or the floating bus.


*/
		ulaplus_last_send_BF3B=value;
	}

	//data port
	if (puerto_h==0xff) {
		ulaplus_last_send_FF3B=value;
		if ( (ulaplus_last_send_BF3B&(64+128))==0) {
			//establecer color
			ulaplus_change_palette_colour((ulaplus_last_send_BF3B&63),value);

                        //Esto solo afecta en modo timex 512x192 con real 512x192 setting, dado 
                        //que el border viene refrescado desde rutina normal sin real video
                        modificado_border.v=1;

		}

		if ( (ulaplus_last_send_BF3B&(64+128))==64) {
			//establecer modo
			ulaplus_set_mode(value);

                        //Esto solo afecta en modo timex 512x192 con real 512x192 setting, dado 
                        //que el border viene refrescado desde rutina normal sin real video
                        modificado_border.v=1;
		}
	}
}
