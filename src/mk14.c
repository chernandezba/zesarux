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

#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>


#include "mk14.h"
#include "scmp.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "zxvision.h"


z80_byte mk14_keystatus[MK14_DIGITS]={
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

/*
Matriz teclado
 128 64  32  16

  0  8   -   A	mk14_keystatus[0]
	1  9   -   B	mk14_keystatus[1]
	2  -   GO  C	mk14_keystatus[2]
	3  -   MEM D	mk14_keystatus[3]
	4  -   ABR -	mk14_keystatus[4]
	5  -   -   -	mk14_keystatus[5]
	6  -   -   E	mk14_keystatus[6]
	7  -   TER F	mk14_keystatus[7]

GO: mapeado a G
MEM: mapeado a M
ABR: mapeado a Z
TERM: mapeado a T

*/



//Parece que keystatus pone los 4 bits superiores a 0 por cada 4 teclas
/*

Por ejemplo, mk14_keystatus[0] indica 4 posibles teclas pulsadas:
01111111  primera tecla
10111111  segunda tecla
11011111  tercera tecla
11101111  cuarta tecla
*/

int mk14_ledstat[MK14_DIGITS];
int mk14_ledlit[MK14_DIGITS];
int mk14_latencycount[MK14_DIGITS];

int mk14_digitlatch=0;
int mk14_segmentlatch=0;
int mk14_keybuffer=0xFF;




/*void mk14_condraw_led(int led,int pattern)
{
if (hDCWork != NULL)
        {
        pattern = pattern & 0xFF;
        refreshled(hDCWork,led,pattern,mk14_ledstatus[led]);
        mk14_ledstatus[led] = pattern;
        }

}*/


void mk14_init_display(void)
{
	int n;
	for (n = 0;n < MK14_DIGITS;n++)
	  // Put a - pattern...
        {
        mk14_ledlit[n] = mk14_ledstat[n] = 0xFF;
        //mk14_condraw_led(n,mk14_ledstat[n]);
        mk14_latencycount[n] = 1;                            // Low latency
        mk14_keystatus[n] = 0xFF;                            // Keys not pressed
        }
}


void mk14_reset(void)
{
	mk14_init_display();
}

void mk14_set_digit_latch(int n,int Write)
{
	if (n < MK14_DIGITS && Write != 0)  {
		// Only 0..8 are LED digits


        if (mk14_ledstat[n] != mk14_segmentlatch)  {
					 // Update display if changed

                mk14_ledlit[n] = mk14_ledlit[n] | mk14_segmentlatch;
                //mk14_condraw_led(MK14_DIGITS-1-n,mk14_ledlit[n]);
                mk14_ledstat[n] = mk14_segmentlatch;
        }

        mk14_latencycount[n] = 5;           //Refresh latency anyway
        }
				mk14_digitlatch = n;   // Update the latch
}

void mk14_dibuja_led(z80_byte valor)
{
	/*
	char *dibujoled=
	" - \n"
	"| |\n"
	" - \n"
	"| |\n"
	" - ";
	*/

	char vaciodibujoled[]=
	"   \n"  //0
	"   \n"  //4
	"   \n"  //8
	"   \n"  //12
	"    \n"; //16
	if (valor&1) vaciodibujoled[1] ='-';
	if (valor&2) vaciodibujoled[6] ='|';
	if (valor&4) vaciodibujoled[14]='|';
	if (valor&8) vaciodibujoled[17]='-';
	if (valor&16) vaciodibujoled[12]='|';
	if (valor&32) vaciodibujoled[4]='|';
	if (valor&64) vaciodibujoled[9]='-';
	if (valor&128) vaciodibujoled[19]='.';

	printf ("%s",vaciodibujoled);

}

/*void temp_dibuja_leds(void)
{
	printf ("\n\n\n\n");

	int i;
	for (i=0;i<MK14_DIGITS;i++) {
		//printf ("%i %d %d\n",i,mk14_ledlit[i],mk14_ledstat[i]);
		//printf ("%i %d\n",i,mk14_ledstat[i]);
		mk14_dibuja_led(mk14_ledstat[i]);
	}
}*/

z80_byte mk14_get_io_port(z80_int dir)
{
//temp
/*int i;
for (i=0;i<MK14_DIGITS;i++) {
	//printf ("%i %d %d\n",i,mk14_ledlit[i],mk14_ledstat[i]);
	printf ("%i %d\n",i,mk14_ledstat[i]);
	mk14_dibuja_led(mk14_ledstat[i]);
}*/

/*
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

	dir &= 0x0F;

	mk14_set_digit_latch(dir,0);  //* Digit select latch changed

	if (dir<8) {
		//printf ("--Leyendo tecla indice %d\n",dir);
		if (zxvision_key_not_sent_emulated_mach() ) return 255;
		else return mk14_keystatus[dir];


	}
	else return 0xFF;
}


void mk14_write_io_port(z80_int dir,z80_byte value)
{
	mk14_segmentlatch=value;

	dir &= 0x0F;

	mk14_set_digit_latch(dir,1);
}
