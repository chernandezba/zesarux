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
#include <stdlib.h>
#include <string.h>


#include "cpu.h"


//tipos de datos que luego se obtendrán desde cpu.h
//64 bits
//typedef long long int z80_64bit;


z80_byte enh_get_amplitud_media(z80_byte *enhanced_memoria,z80_64bit tamanyo_memoria)
{
    z80_64bit i;
    z80_64bit acumulada_amplitud=0;
    z80_64bit total_pulsos=0;

    int estado_pulso=0; //0: subiendo, 1: bajando

    z80_byte amplitud_este_pulso=0;
    z80_byte valor_sample_anterior=enhanced_memoria[0];
    z80_byte valor_sample_inicio_pulso=enhanced_memoria[0];

    for (i=0;i<tamanyo_memoria;i++) {
        z80_byte valor_sample=enhanced_memoria[i];

        switch(estado_pulso) {
            case 0:
                //subiendo
                if (valor_sample<valor_sample_anterior) {
                    amplitud_este_pulso=valor_sample-valor_sample_inicio_pulso;
                    acumulada_amplitud +=amplitud_este_pulso;
                    estado_pulso=1;
                }
            break;

            case 1:
                //bajando. solo esperar a que finalice
                if (valor_sample>valor_sample_anterior) {
                    valor_sample_inicio_pulso=valor_sample;
                    estado_pulso=0;
                }
            break;
        }

        valor_sample_anterior=valor_sample;
    }

    total_pulsos++;

    return acumulada_amplitud/total_pulsos;
}


int main_enhanced_zx81_read(z80_byte *enhanced_memoria,z80_64bit tamanyo_memoria)
{

    z80_byte amplitud_media=enh_get_amplitud_media(enhanced_memoria,tamanyo_memoria);

    printf("Amplitud media: %d\n",amplitud_media);


    return 0;
}