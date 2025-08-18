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

/*

-Nuevo algoritmo de lectura de audio zx81 -> .P

* calculo aproximado de maxima amplitud. Para saber la media de amplitud.
valor inicial. Que suba hasta un máximo y baje a un mínimo. Eso para todo el audio. Eso es un pulso de subida
Eso devuelve un valor AMPLITUD_MAXIMA

*Para lectura de pulso exacto, no aproximado:
valor inicial. que suba hasta un máximo y baje a un mínimo.
-Dos estados: subiendo y bajando. Inicial en subiendo.
-En estado subiendo:
--Mientras valor leido sea mayor que anterior, sigue en subiendo
--Si valor leido es menor que anterior, pasa a estado bajando
-En estado bajando:
--Mientras valor leido sea inferior a anterior, sigue en bajando
--Si valor leido es mayor que anterior, finaliza el pulso

Con condiciones:
** si está subiendo y sube y baja y luego sube (o sea aparentemente llegamos al final), si amplitud de la parte de
 bajada no al menos un 70% de AMPLITUD_MAXIMA media, saltar a ciclo de subida
Esto es para evitar crestas que tienen "rugosidades" (forma de U en la misma cresta) o los "picos" producidos con los silencios
de delante de cada bit inicial
-> amplitud de la cresta de subida= valor máximo-valor inicial
-> amplitud de la cresta de bajada= valor máximo-valor final (actual)
Devolver:
-longitud de la cresta de subida: desde posición inicial hasta posición de máximo valor
-longitud de la cresta de bajada: desde posición de máximo valor hasta posición final
-amplitud de la cresta de subida
-amplitud de la cresta de bajada
Crestas de subida que sean 3 o 4 veces de mayor longitud que la cresta de bajada implica que hay un silencio antes de dicha onda
Crestas de bajada que sean 3 o 4 veces de mayor longiutd que la cresta de subide implica que hay un silencio despues de dicha onda
(esto ultimo no debería darse en teoria)

Nota: el final se marca con un pulso aislado (a diferencia de bits 0 - 4 pulsos - o bits 1 - 8 pulsos)

*/

//Actualmente esta rutina de amplitud maxima NO VA BIEN. Detecta como pulsos los silencios previos a cada pulso de bit




z80_byte enh_get_amplitud_maxima(z80_byte *enhanced_memoria,z80_64bit tamanyo_memoria)
{
    z80_64bit i;
    z80_64bit acumulada_amplitud=0;
    z80_64bit amplitud_maxima=0;
    z80_64bit total_pulsos=0;

    int estado_pulso=0; //0: subiendo, 1: bajando

    z80_byte amplitud_este_pulso=0;
    z80_byte valor_sample_anterior=enhanced_memoria[0];
    z80_byte valor_sample_inicio_pulso=enhanced_memoria[0];

    for (i=0;i<tamanyo_memoria;i++) {
        z80_byte valor_sample=enhanced_memoria[i];

        //if (i>=62920 && i<=62957) printf("i: %lld\n",i);

        switch(estado_pulso) {
            case 0:
                //subiendo
                if (valor_sample<valor_sample_anterior) {
                    amplitud_este_pulso=valor_sample_anterior-valor_sample_inicio_pulso;
                    if (amplitud_este_pulso>60) printf("%lld Pico pulso Pulso amplitud: %d\n",i,amplitud_este_pulso);
                    acumulada_amplitud +=amplitud_este_pulso;
                    if (amplitud_este_pulso>amplitud_maxima) amplitud_maxima=amplitud_este_pulso;
                    estado_pulso=1;
                }
            break;

            case 1:
                //bajando. solo esperar a que finalice
                if (valor_sample>valor_sample_anterior) {
                    valor_sample_inicio_pulso=valor_sample;
                    estado_pulso=0;
                    total_pulsos++;
                    if (amplitud_este_pulso>60) printf("%lld Final Pulso amplitud: %d\n",i,amplitud_este_pulso);
                }
            break;
        }

        valor_sample_anterior=valor_sample;
    }

    printf("Amplitud media: %lld\n",acumulada_amplitud/total_pulsos);

    return amplitud_maxima;


}


int main_enhanced_zx81_read(z80_byte *enhanced_memoria,z80_64bit tamanyo_memoria)
{

    z80_byte amplitud_maxima=enh_get_amplitud_maxima(enhanced_memoria,tamanyo_memoria);

    printf("Amplitud maxima: %d\n",amplitud_maxima);


    return 0;
}