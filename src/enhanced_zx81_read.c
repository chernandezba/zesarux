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


#include "enhanced_zx81_read.h"


//#include "cpu.h"
//HAY QUE mantener este código con el mínimo de dependencias pues se tiene que poder compilar externamente junto con main_enhanced_zx81_read.c
//y adicionalmente se compila también en ZEsarUX


/*

-Nuevo algoritmo de lectura de audio zx81 -> .P

* calculo aproximado de maxima amplitud. Para saber la media de amplitud.
valor inicial. Que suba hasta un máximo y baje a un mínimo. Eso para todo el audio. Eso es un pulso de subida
Eso devuelve un valor AMPLITUD_MEDIA -> Ese algoritmo está obsoleto y no se usa
Al final se calcula la mejor amplitud en base a la que proporciona mayor bytes generados en el archivo de salida.
Pero esto lo hace el programa que llama aqui, por tanto se debe invocar a enh_zx81_lee_datos con la amplitud ideal ya calculada

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
 bajada no al menos un 70% de AMPLITUD_MEDIA, volver a ciclo de subida
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



//Array que indica cuantas amplitudes de cada valor se han encontrado
int enh_amplitudes[256];


//Funcion obsoleta
void enh_get_amplitud_media(z80_byte *enhanced_memoria,int tamanyo_memoria)
{
    int i;
    int acumulada_amplitud=0;
    int amplitud_maxima=0;
    int total_pulsos=0;

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
                    amplitud_este_pulso=valor_sample_anterior-valor_sample_inicio_pulso;
                    if (1/*amplitud_este_pulso>60*/) {
                        printf("%d Pico pulso Pulso. sample anterior %d sample actual %d amplitud: %d\n",
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
                        printf("%d Final Pulso-inicio. valor_sample: %d\n",i,valor_sample_inicio_pulso);
                    }
                }
            break;
        }

        valor_sample_anterior=valor_sample;
    }

    printf("Amplitud media: %d\n",acumulada_amplitud/total_pulsos);

    //return amplitud_maxima;


}

z80_byte zx81_char_table[64]=" ??????????\"?$:?()><=+-*/;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

z80_byte return_zx81_char(z80_byte codigo)
{

    if (codigo>127) {
        codigo-=128;
    }

    return (codigo<64 ? zx81_char_table[codigo] : '?');

}

//Para calcular longitud media de un pulso. Esto nos servira para adivinar frecuencia de sampleo
//sumar 100

int enh_zx81_acumulado_longitud_pulso_medio=0;

//valor medio final
int enh_zx81_longitud_pulso_medio=0;

//cuantos hemos sumado
int enh_zx81_longitud_pulso_medio_cuantos=0;

//frecuencia de sampleo adivinada
int enh_global_guessed_sample_rate=0;


int enh_global_input_position=0;
z80_byte enh_global_last_audio_sample=0;
int enh_global_output_position=0;
z80_byte enh_global_last_byte_read=0;
z80_byte enh_global_partial_byte_read=0;
int enh_global_last_bit_read=0;
int enh_global_bit_position_in_byte=0;
int enh_global_pulses_of_a_bit=0;
int enh_global_rise_position=0;
int enh_global_start_bit_position=0;
int enh_global_start_byte_position=0;
int enh_global_total_input_size=0;

//ultimos bytes leidos. El de mas de la derecha (el ultimo) es el ultimo byte leido
z80_byte enh_global_last_bytes[ENHANCED_GLOBAL_INFO_LAST_BYTES_LENGTH];


//funcion para obtener posicion actual: pos sample, pos output, ultimo byte, ultimo bit, conteo de pulsos de bit, ...
void enh_zx81_lee_get_global_info(struct s_enh_zx81_lee_global_info *info)
{
    info->enh_global_input_position=enh_global_input_position;
    info->enh_global_total_input_size=enh_global_total_input_size;
    info->enh_global_last_audio_sample=enh_global_last_audio_sample;
    info->enh_global_output_position=enh_global_output_position;
    info->enh_global_last_byte_read=enh_global_last_byte_read;
    info->enh_global_partial_byte_read=enh_global_partial_byte_read;
    info->enh_global_last_bit_read=enh_global_last_bit_read;
    info->enh_global_bit_position_in_byte=enh_global_bit_position_in_byte;
    info->enh_global_pulses_of_a_bit=enh_global_pulses_of_a_bit;
    info->enh_global_rise_position=enh_global_rise_position;
    info->enh_global_start_bit_position=enh_global_start_bit_position;
    info->enh_global_start_byte_position=enh_global_start_byte_position;
    info->enh_global_guessed_sample_rate=enh_global_guessed_sample_rate;

    int i;

    for (i=0;i<ENHANCED_GLOBAL_INFO_LAST_BYTES_LENGTH;i++) {
        info->enh_global_last_bytes[i]=enh_global_last_bytes[i];
    }

}

//rotar el array de ultimos bytes
//de derecha a izquierda
void enh_zx81_lee_rotate_last_bytes(void)
{
    int i;
    for (i=0;i<ENHANCED_GLOBAL_INFO_LAST_BYTES_LENGTH-1;i++) {
        enh_global_last_bytes[i]=enh_global_last_bytes[i+1];
    }

    enh_global_last_bytes[i]=0;
}



//cancel_process puntero a valor int que dice si se cancela el proceso (valor diferente de 0). Si no se usa, pasar puntero a NULL. Esto
//si puede activar desde thread externo
//callback es una rutina que se llama en cada iteración del bucle, si no es NULL
int enh_zx81_lee_datos(z80_byte *enhanced_memoria,int tamanyo_memoria,z80_byte *destino_p81,
    z80_byte amplitud_media, int debug_print,int *longitud_nombre,void (*fun_print)(char *),int *cancel_process,
    void (*callback)(void),int *total_pulsos_sospechosos)
{

    //Inicializar globales que se pueden leer desde thread externo
    enh_global_input_position=0;
    enh_global_output_position=0;
    enh_global_last_byte_read=0;
    enh_global_last_bit_read=0;
    enh_global_bit_position_in_byte=0;
    enh_global_pulses_of_a_bit=0;
    enh_global_rise_position=0;
    enh_global_start_bit_position=0;
    enh_global_start_byte_position=0;
    enh_zx81_acumulado_longitud_pulso_medio=0;
    enh_zx81_longitud_pulso_medio=0;
    enh_zx81_longitud_pulso_medio_cuantos=0;
    enh_global_guessed_sample_rate=0;


    enh_global_total_input_size=tamanyo_memoria;

    if (total_pulsos_sospechosos!=NULL) {
        *total_pulsos_sospechosos=0;
    }


    int i;
    for (i=0;i<ENHANCED_GLOBAL_INFO_LAST_BYTES_LENGTH;i++) {
        enh_global_last_bytes[i]=0;
    }


    int amplitud_maxima=0;


    int estado_pulso=0; //0: subiendo, 1: bajando

    z80_byte amplitud_este_pulso=0;
    z80_byte valor_sample_anterior=enhanced_memoria[0];
    z80_byte valor_sample_inicio_pulso=enhanced_memoria[0];
    z80_byte valor_sample_pico_alto=0;

    int posicion_cresta_subida=0;
    int posicion_cresta_bajada=0;

    int pulsos_leidos=0;
    int conteo_pulsos_de_bit=0;

    z80_byte acumulado_byte=0;
    int numero_bit_en_byte=0;

    int indice_destino_p81=0;
    int leido_nombre=0;
    *longitud_nombre=0;

    char buffer_print[200];

    for (i=0;i<tamanyo_memoria;i++) {
        enh_global_input_position=i;
        enh_global_output_position=indice_destino_p81;
        enh_global_partial_byte_read=acumulado_byte;
        enh_global_pulses_of_a_bit=conteo_pulsos_de_bit;
        enh_global_bit_position_in_byte=numero_bit_en_byte;

        if (cancel_process!=NULL) {
            if (*cancel_process) {
                //printf("Cancelled reading data\n");
                return indice_destino_p81;
            }
        }

        z80_byte valor_sample=enhanced_memoria[i];

        enh_global_last_audio_sample=valor_sample;

        if (callback!=NULL) {
            callback();
        }

        switch(estado_pulso) {
            case 0:
                //subiendo
                if (valor_sample<valor_sample_anterior) {
                    amplitud_este_pulso=valor_sample_anterior-valor_sample_inicio_pulso;
                    valor_sample_pico_alto=valor_sample_anterior;
                    if (debug_print && fun_print!=NULL) {
                        sprintf(buffer_print,"%d: Top part of pulse. previous sample: %d current sample: %d amplitude: %d",
                            i,valor_sample_anterior,valor_sample,amplitud_este_pulso);
                        fun_print(buffer_print);
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
                    //int amplitud_media_alternativa_minimo=(amplitud_media_alternativa*70)/100;

                    if (amplitud_bajada<amplitud_media_minimo /*&& amplitud_bajada<amplitud_media_alternativa_minimo*/) {
                        estado_pulso=0;
                    }

                    else {
                        valor_sample_inicio_pulso=valor_sample;
                        estado_pulso=0;


                        int longitud_cresta_subida=posicion_cresta_bajada-posicion_cresta_subida;
                        int longitud_cresta_bajada=i-posicion_cresta_bajada;

                        if (debug_print && fun_print!=NULL) {
                            sprintf(buffer_print,"%d End pulse. sample value: %d. Rise amplitude: %d "
                                                 "Fall amplitude: %d Rise length: %d Fall length: %d",
                                i,valor_sample_inicio_pulso,amplitud_este_pulso,amplitud_bajada,
                                longitud_cresta_subida,longitud_cresta_bajada);
                            fun_print(buffer_print);
                        }


                        posicion_cresta_subida=i;
                        enh_global_rise_position=i;

                        //Crestas de subida que sean 3 o 4 veces de mayor longitud que la cresta de bajada implica que hay un silencio antes de dicha onda
                        //Ejemplo en Control de Stocks, la mayoria de finales de bit son:
                        //Rise length: 21 Fall length: 3
                        //Por tanto hay mucho mas que 3 veces mas
                        if (longitud_cresta_subida>longitud_cresta_bajada*3 && pulsos_leidos) {

                        //para turbo
                        //if (longitud_cresta_subida>(longitud_cresta_bajada*25)/10 && pulsos_leidos) {
                            if (debug_print && fun_print!=NULL) {
                                sprintf(buffer_print,"%d End of bit before this current pulse. Total bit pulses: %d",i,conteo_pulsos_de_bit);
                                fun_print(buffer_print);
                            }

                            //4 para 0. 8 o 9 para 1. TODO: no deberia ser 8 y no 9 siempre???

                            //z80_byte acumulado_byte=0;
                            //int numero_bit_en_byte=0;

                            int bit_leido=0;

                            //Conteo de pulsos que habia inicialmente, leido stocks.p81 con md5 6775784b5fd35b5a9f3444fdf10a7447
                            //if (conteo_pulsos_de_bit==3 || conteo_pulsos_de_bit==4 || conteo_pulsos_de_bit==5) bit_leido=0;
                            //else if (conteo_pulsos_de_bit==8 || conteo_pulsos_de_bit==9 || conteo_pulsos_de_bit==10) bit_leido=1;

                            //Conteo de pulsos que debe ser
                            if (conteo_pulsos_de_bit==4) bit_leido=0;
                            else if (conteo_pulsos_de_bit==8 || conteo_pulsos_de_bit==9) bit_leido=1;


                            //Prueba conteo de pulsos para turbo
                            //if (conteo_pulsos_de_bit==2) bit_leido=0;
                            //else if (conteo_pulsos_de_bit==4 || conteo_pulsos_de_bit==5 || conteo_pulsos_de_bit==6) bit_leido=1;

                            else if (conteo_pulsos_de_bit==1) {
                                if (fun_print!=NULL) {
                                    sprintf(buffer_print,"%d Only one pulse. Assume end of program",i);
                                    fun_print(buffer_print);
                                }
                                return indice_destino_p81;
                            }
                            else {
                                if (total_pulsos_sospechosos!=NULL) {
                                    *total_pulsos_sospechosos=(*total_pulsos_sospechosos)+1;
                                }
                                if (fun_print!=NULL) {
                                    sprintf(buffer_print,"%d We do not know what bit value is when found %d pulses",i,conteo_pulsos_de_bit);
                                    fun_print(buffer_print);
                                }
                            }

                            acumulado_byte=acumulado_byte<<1;
                            acumulado_byte |=bit_leido;
                            enh_global_last_bit_read=bit_leido;

                            enh_global_start_bit_position=i;

                            numero_bit_en_byte++;
                            if (numero_bit_en_byte==8) {
                                if (debug_print && fun_print!=NULL)  {
                                    sprintf(buffer_print,"%d Final Byte: %3d (%02XH) Character %c",i,acumulado_byte,acumulado_byte,return_zx81_char(acumulado_byte));
                                    fun_print(buffer_print);
                                }

                                destino_p81[indice_destino_p81++]=acumulado_byte;
                                enh_global_last_byte_read=acumulado_byte;

                                enh_zx81_lee_rotate_last_bytes();
                                enh_global_last_bytes[ENHANCED_GLOBAL_INFO_LAST_BYTES_LENGTH-1]=acumulado_byte;

                                enh_global_start_byte_position=i;

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

                        else {
                            //Pulso que no finaliza un bit
                            //Ignorar primer byte para no tener en cuenta silencios y ondas largas del principio
                            if (indice_destino_p81>0) {
                                //Para sacar longitud media de un pulso y adivinar sample rate del audio
                                if (enh_zx81_longitud_pulso_medio_cuantos<ENH_ZX81_LONG_MEDIA_CONTAR_PULSOS) {
                                    //printf("%d Rise length: %d Fall length: %d suma: %d\n",
                                    //    i,longitud_cresta_subida,longitud_cresta_bajada,longitud_cresta_subida+longitud_cresta_bajada);


                                    enh_zx81_acumulado_longitud_pulso_medio +=longitud_cresta_subida+longitud_cresta_bajada;
                                    enh_zx81_longitud_pulso_medio_cuantos++;

                                    if (enh_zx81_longitud_pulso_medio_cuantos==ENH_ZX81_LONG_MEDIA_CONTAR_PULSOS) {
                                        //no dividimos por el total por no perder decimales
                                        //valores referencia (con ENH_ZX81_LONG_MEDIA_CONTAR_PULSOS=100):
                                        //15600 hz : acumulado: 476 dividido: 4
                                        //11111 hz: acumulado: 337 dividido: 3. Calculamos (337*15600)/476=11044 aprox 11111
                                        //44100 hz: acumulado: 1342. dividido: 13
                                        enh_zx81_longitud_pulso_medio=enh_zx81_acumulado_longitud_pulso_medio; ///ENH_ZX81_LONG_MEDIA_CONTAR_PULSOS;
                                        //printf("Longitud pulso medio: %d\n",enh_zx81_longitud_pulso_medio);

                                        //Aprox hz por valor referencia de 15600 hz
                                        //int freq_sampleo_aprox=(enh_zx81_longitud_pulso_medio*15600)/476;

                                        //Aprox hz por valor referencia de 44100 hz
                                        int freq_sampleo_aprox=(enh_zx81_longitud_pulso_medio*44100)/1342;


                                        //printf("Freq sampleo aprox: %d Hz\n",freq_sampleo_aprox);
                                        enh_global_guessed_sample_rate=freq_sampleo_aprox;


                                    }
                                }
                            }
                        }

                        conteo_pulsos_de_bit++;

                        pulsos_leidos++;


                    }
                }
            break;
        }

        valor_sample_anterior=valor_sample;
    }



    return indice_destino_p81;


}


//Funcion obsoleta
int main_enhanced_zx81_read(z80_byte *enhanced_memoria,int tamanyo_memoria,z80_byte *memoria_p81,
    z80_byte amplitud_media,int analizar_amplitudes,int debug_print,int *longitud_nombre)
{
    if (analizar_amplitudes) {
        /*
        Algoritmo no usado pero se pretendia determinar la mejor amplitud, aunque creo que solo con esto no se puede encontrar
        por ejemplo con STOCKS vemos que el valor mas alto es
        Amplitud 38 cantidad: 11926

        Pero en cambio ese valor de 38 no da una lectura correcta del programa
        (amplitud_media=39 Longitud nombre: 14 Longitud p81: 4248 Nombre: CONTROL STOCKS)
        y realmente ocupa 6433 bytes y no 4248
        */
        int i;
        for (i=0;i<256;i++) enh_amplitudes[i]=0;

        enh_get_amplitud_media(enhanced_memoria,tamanyo_memoria);

        //printf("Amplitud maxima: %d\n",amplitud_maxima);

        if (debug_print)  {
            for (i=0;i<256;i++) printf("Amplitud %i cantidad: %d\n",i,enh_amplitudes[i]);
        }

    }


    int longitud_p81=enh_zx81_lee_datos(enhanced_memoria,tamanyo_memoria,memoria_p81,amplitud_media,debug_print,longitud_nombre,NULL,NULL,NULL,NULL);

    return longitud_p81;
}