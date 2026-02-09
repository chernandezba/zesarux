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
   Several mathematical utilities functions
*/

#include <stdio.h>

#include "utils_math.h"
#include "debug.h"
#include "cpu.h"


//Devolver valor sin signo
int util_get_absolute(int valor)
{
        if (valor<0) valor=-valor;

        return valor;
}

//Devolver signo de valor
int util_get_sign(int valor)
{
    if (valor<0) return -1;

    return +1;
}

int util_smaller(int a,int b)
{
    if (a<b) return a;
    else return b;
}



int util_cosine_table[91]={

10000,  //0
 9998,
 9994,
 9986,
 9976,
 9962,
 9945,
 9925,
 9903,
 9877,
 9848,
 9816,
 9781,
 9744,
 9703,

 9659, //15
 9613,
 9563,
 9511,
 9455,
 9397,
 9336,
 9272,
 9205,
 9135,
 9063,
 8988,
 8910,
 8829,
 8746,

 8660, //30,
 8572,
 8480,
 8387,
 8290,
 8192,
 8090,
 7986,
 7880,
 7771,
 7660,
 7547,
 7431,
 7314,
 7193,

 7071, //45
 6947,
 6820,
 6691,
 6561,
 6428,
 6293,
 6157,
 6018,
 5878,
 5736,
 5592,
 5446,
 5299,
 5150,

 5000, //60
 4848,
 4695,
 4540,
 4384,
 4226,
 4067,
 3907,
 3746,
 3584,
 3420,
 3256,
 3090,
 2924,
 2756,

 2588, //75
 2419,
 2250,
 2079,
 1908,
 1736, //80
 1564,
 1392,
 1219,
 1045,
  872,
  698,
  523,
  349,
  175,
    0  //90
};


//Retorna el coseno de un grado, multiplicado por 10000
int util_get_cosine(int degrees)
{

    //Ajustar a 360
    degrees = degrees % 360;

    //Hacerlo positivo
    if (degrees<0) degrees=360+degrees;


    if (degrees>=91 && degrees<=180) {
        return -util_cosine_table[180-degrees];
    }
    else if (degrees>=181 && degrees<=270) {
        return -util_cosine_table[degrees-180];
    }
    else if (degrees>=271 && degrees<=359) {
        return util_cosine_table[360-degrees];
    }
    else return util_cosine_table[degrees];
}

//Retorna el seno de un grado, multiplicado por 10000
int util_get_sine(int degrees)
{
    return util_get_cosine(90-degrees);
}

//Retorna los grados para un coseno dado
int util_get_acosine(int cosine)
{
    int i;
    int acosine;

    for (i=1;i<91;i++) {
        if (util_get_absolute(cosine)==util_cosine_table[i]) {
            acosine=i;
            break;
        }

        if (util_get_absolute(cosine)>util_cosine_table[i]) {
            acosine=i-1;
            break;
        }
    }

    //Si negativo
    if (cosine<0) {
        acosine=180-acosine;
    }

    return acosine;
}

//Antiguo algoritmo raiz cuadrada que recorre todos los elementos
int old_util_sqrt(int number)
{
    int resultado=0;
    while (resultado<=number) {
        if (resultado*resultado==number) return resultado; //exacto

        if (resultado*resultado>number) return resultado-1; //nos pasamos. restar 1 para no esceder

        resultado++;
    }

    return resultado;
}

//calcula la raiz cuadrada con valores enteros
//Algoritmo mejorado que va dividiendo conjunto total/2 cada vez, siendo mucho mas rapido
//Por ejemplo, 13 iteraciones para calcular raiz de 10000, mientras con el antiguo, serian 100 iteraciones
//(con el antiguo siempre son X iteraciones siendo X el resultado de la raiz cuadrada)
//Tipo resultado: 0 exacto, 1 aproximado, -1 valor negativo
//Si result_type==NULL, no guardar tipo resultado
//
//Nota para mi yo del futuro (o cualquiera que lea esto):
//Podria haber usado la funcion sqrt que utiliza libreria math del sistema (y probablemente directo a cpu),
//pero quiero hacerlo asi, asi aprendo un algoritmo simple
//Ademas, no necesito decimales (y si necesito algo de precision, podria usar un valor de entrada multiplicado por 10000 por ejemplo)
z80_64bit util_sqrt(z80_64bit number,int *result_type)
{
    //printf("-------\n");
    //z80_64bit resultado;
    z80_64bit final=number;
    z80_64bit inicio=0;

    int result_type_when_null;

    //Cuando parametro es NULL, lo cambiamos para que apunte a una variable local que no uso
    if (result_type==NULL) {
        result_type=&result_type_when_null;
    }

    while (inicio<=final) {
        //printf("Inicio %d final %d\n",inicio,final);

        //Si hay dos

        z80_64bit medio=(inicio+final)/2;

        z80_64bit cuadrado=medio*medio;

        if (cuadrado==number) {
            //printf("Exact result #1\n");
            *result_type=0;
            return medio; //exacto
        }

        z80_64bit delta=final-inicio;
        //si hay dos o 1 numeros en nuestro conjunto, hay que optar por el primero o segundo, sin que exceda
        if (delta<=1) {

            if (final*final==number) {
                //printf("Exact result #2\n");  //creo que esta solucion nunca se da
                *result_type=0;
                return final;
            }
            else {
                if (inicio*inicio==number) {
                    //printf("Exact result #3\n"); //creo que esta solucion nunca se da
                    *result_type=0;
                }
                else {
                    //printf("Aproximate result\n");
                    *result_type=1;
                }

                return inicio;
            }
        }

        if (cuadrado>number) {
            //Nos quedamos con la parte izquierda
            //printf("izq\n");
            final=medio;
        }

        else {
            //cuadrado<number
            //Nos quedamos con la parte derecha
            //printf("der\n");
            inicio=medio;
        }


    }

    //realmente por aqui solo deberiamos salir si inicio>final, o sea , si final es negativo
    //printf("Fallback return\n");
    *result_type=-1;
    return inicio;
}
