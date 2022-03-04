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
#include <math.h>

#include "audio.h"


/*
Function to generate sine table to distort audio when using top speed
Inspired by Nagravision audio codec
Compile and run manually to generate table for audio_sine_table.c. 
*/

int main()


{

float sineval,radians;
int i;
const int freq=FREQ_TOP_SPEED_CHANGE;
int cols=0;

        for (i=0;i<freq;i++) {
                radians=i;
                radians=radians*6.28318530718;

                radians=radians/((float)(freq));
                sineval=sin(radians);
                char sinevalue_int=100*sineval;  //de 0 a 100 en porcentaje

                //printf ("i=%d radians=%f sine=%f value=%d\n",i,radians,sineval,sinevalue_int);
		printf ("%d,",sinevalue_int);
		cols++;
		if (cols==10) {
			cols=0;
			printf ("\n");
		}
        }
	return 0;
}
