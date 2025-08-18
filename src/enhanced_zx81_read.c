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
 bajada no al menos un 70% de AMPLITUD_MAXIMA (media), volver a ciclo de subida
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
Crestas de bajada que sean 3 o 4 veces de mayor longitud que la cresta de subide implica que hay un silencio despues de dicha onda
(esto ultimo no debería darse en teoria)

Nota: el final se marca con un pulso aislado (a diferencia de bits 0 - 4 pulsos - o bits 1 - 8 pulsos)

Como detectar inicio de los datos? Mientras no haya un pulso similar a amplitud media
Pero puede ser entonces que el primer pulso sea enorme (desde posicion 0 hasta la posicion que empieza el primer pulso de datos)
Eso es un problema? Realmente no, simplemente la  longitud de la cresta de subida será un valor muy grande

*/

//Actualmente esta rutina de amplitud maxima NO VA BIEN. Detecta como pulsos los silencios previos a cada pulso de bit

//Quiza se podria hacer que fuese el usuario quien dijera la amplitud maxima (la media realmente) de un pulso de bits

//Array que indica cuantas amplitudes de cada valor se han encontrado
z80_64bit enh_amplitudes[256];



void enh_get_amplitud_maxima(z80_byte *enhanced_memoria,z80_64bit tamanyo_memoria)
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
                    if (1/*amplitud_este_pulso>60*/) {
                        printf("%lld Pico pulso Pulso. sample anterior %d sample actual %d amplitud: %d\n",
                            i,valor_sample_anterior,valor_sample,amplitud_este_pulso);
                    }
                    acumulada_amplitud +=amplitud_este_pulso;

                    enh_amplitudes[amplitud_este_pulso]++;

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
                    if (1/*amplitud_este_pulso>60*/) {
                        printf("%lld Final Pulso-inicio. valor_sample: %d\n",i,valor_sample_inicio_pulso);
                    }
                }
            break;
        }

        valor_sample_anterior=valor_sample;
    }

    printf("Amplitud media: %lld\n",acumulada_amplitud/total_pulsos);

    //return amplitud_maxima;


}

z80_byte   caracteres_zx81_no_artistic[64]=" ??????????\"?$:?()><=+-*/;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

z80_byte da_codigo81(z80_byte codigo)
{

  if (codigo>127) {

        codigo-=128;
  }




    return (codigo<64 ? caracteres_zx81_no_artistic[codigo] : '~');


}

int enh_get_pulsos(z80_byte *enhanced_memoria,z80_64bit tamanyo_memoria,z80_byte amplitud_media,z80_byte *destino_p81,int *longitud_nombre)
{
    z80_64bit i;
    z80_64bit amplitud_maxima=0;


    int estado_pulso=0; //0: subiendo, 1: bajando

    z80_byte amplitud_este_pulso=0;
    z80_byte valor_sample_anterior=enhanced_memoria[0];
    z80_byte valor_sample_inicio_pulso=enhanced_memoria[0];
    z80_byte valor_sample_pico_alto=0;

    //z80_64bit longitud_cresta_subida=0;
    //z80_64bit longitud_cresta_bajada=0;

    z80_64bit posicion_cresta_subida=0;
    z80_64bit posicion_cresta_bajada=0;

    z80_64bit pulsos_leidos=0;
    int conteo_pulsos_de_bit=0;

    z80_byte acumulado_byte=0;
    int numero_bit_en_byte=0;

    int indice_destino_p81=0;
    int leido_nombre=0;
    *longitud_nombre=0;

    for (i=0;i<tamanyo_memoria;i++) {
        z80_byte valor_sample=enhanced_memoria[i];

        //if (i>=62920 && i<=62957) printf("i: %lld\n",i);

        switch(estado_pulso) {
            case 0:
                //subiendo
                if (valor_sample<valor_sample_anterior) {
                    amplitud_este_pulso=valor_sample_anterior-valor_sample_inicio_pulso;
                    valor_sample_pico_alto=valor_sample_anterior;
                    if (1/*amplitud_este_pulso>60*/) {
                        printf("%lld Pico pulso Pulso. sample anterior %d sample actual %d amplitud: %d\n",
                            i,valor_sample_anterior,valor_sample,amplitud_este_pulso);
                    }



                    if (amplitud_este_pulso>amplitud_maxima) amplitud_maxima=amplitud_este_pulso;
                    estado_pulso=1;
                    posicion_cresta_bajada=i;
                }
            break;

            case 1:
                //bajando. solo esperar a que finalice
                if (valor_sample>valor_sample_anterior) {

                    /*
                    ** si está subiendo y sube y baja y luego sube (o sea aparentemente llegamos al final), si amplitud de la parte de
 bajada no al menos un 70% de AMPLITUD_MAXIMA (media), volver a ciclo de subida
                    */
                    int amplitud_bajada=valor_sample_pico_alto-valor_sample_anterior;
                    int amplitud_media_minimo=(amplitud_media*70)/100;
                    if (amplitud_bajada<amplitud_media_minimo) {
                        estado_pulso=0;
                    }

                    else {
                        valor_sample_inicio_pulso=valor_sample;
                        estado_pulso=0;


                        z80_64bit longitud_cresta_subida=posicion_cresta_bajada-posicion_cresta_subida;
                        z80_64bit longitud_cresta_bajada=i-posicion_cresta_bajada;


                        printf("%lld Final Pulso-inicio. valor_sample: %d. amplitud subida: %d amplitud bajada %d long subida %lld long bajada: %lld\n",
                            i,valor_sample_inicio_pulso,amplitud_este_pulso,amplitud_bajada,
                            longitud_cresta_subida,longitud_cresta_bajada);



                        posicion_cresta_subida=i;

                        //Crestas de subida que sean 3 o 4 veces de mayor longitud que la cresta de bajada implica que hay un silencio antes de dicha onda
                        if (longitud_cresta_subida>longitud_cresta_bajada*3 && pulsos_leidos) {
                            printf("Hay fin de bit antes de este pulso. Conteo pulsos de bit: %d\n",conteo_pulsos_de_bit);

                            //4 para 0. 8 o 9 para 1. TODO: no deberia ser 8 y no 9 siempre???

                            //z80_byte acumulado_byte=0;
                            //int numero_bit_en_byte=0;

                            int bit_leido=0;
                            if (conteo_pulsos_de_bit==4) bit_leido=0;
                            else if (conteo_pulsos_de_bit==8 ||conteo_pulsos_de_bit==9) bit_leido=1;
                            else if (conteo_pulsos_de_bit==1) {
                                printf("1 solo pulso. Quiza final de archivo?\n");
                                return indice_destino_p81;
                            }
                            else printf("No sabemos que bit es cuando hay %d pulsos\n",conteo_pulsos_de_bit);

                            acumulado_byte=acumulado_byte<<1;
                            acumulado_byte |=bit_leido;
                            numero_bit_en_byte++;
                            if (numero_bit_en_byte==8) {
                                printf("Byte final: %3d (%02XH) caracter %c\n",acumulado_byte,acumulado_byte,da_codigo81(acumulado_byte));
                                destino_p81[indice_destino_p81++]=acumulado_byte;

                                if (!leido_nombre) {
                                    if (acumulado_byte&128) {
                                        *longitud_nombre=indice_destino_p81;
                                        leido_nombre=1;
                                    }
                                }

                                acumulado_byte=0;
                                numero_bit_en_byte=0;
                            }

                            conteo_pulsos_de_bit=0;
                        }

                        conteo_pulsos_de_bit++;

                        pulsos_leidos++;


                    }
                }
            break;
        }

        valor_sample_anterior=valor_sample;
    }



    //return amplitud_maxima;
    return indice_destino_p81;


}

int main_enhanced_zx81_read(z80_byte *enhanced_memoria,z80_64bit tamanyo_memoria,z80_byte *memoria_p81,int *longitud_nombre)
{
    int i;
    for (i=0;i<256;i++) enh_amplitudes[i]=0;

    enh_get_amplitud_maxima(enhanced_memoria,tamanyo_memoria);

    //printf("Amplitud maxima: %d\n",amplitud_maxima);

    for (i=0;i<256;i++) printf("Amplitud %i cantidad: %lld\n",i,enh_amplitudes[i]);

    //prueba. Para control de stocks
    z80_byte amplitud_media=18;


    int longitud_p81=enh_get_pulsos(enhanced_memoria,tamanyo_memoria,amplitud_media,memoria_p81,longitud_nombre);

    return longitud_p81;
}