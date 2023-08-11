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

Simulacion del chip de sonido SAA del Sam Coupe mediante conversion al chip AY del Spectrum

*/

#include <stdio.h>

#include "ay38912.h"
#include "cpu.h"
#include "audio.h"
#include "debug.h"
#include "saa_simul.h"


z80_byte sam_saa_simul_chip[32];
z80_byte sam_saa_simul_chip_last_selected;

int saa_simul_calcular_frecuencia(int freq, int octava)
{
	//frecuencia entre 31 y 61. O sea, 31 posibles valores

	//pasamos de 0...255 a 0...31. Dividir entre 8.22-> x/8.22 = x*100/822
	freq=freq*100;
	freq=freq/822;
	freq=31+freq;

	int multiplicador_octava;
	if (octava==0) multiplicador_octava=1;
	if (octava==1) multiplicador_octava=2;
	if (octava==2) multiplicador_octava=4;
	if (octava==3) multiplicador_octava=8;
	if (octava==4) multiplicador_octava=16;
	if (octava==5) multiplicador_octava=32;
	if (octava==6) multiplicador_octava=64;
	if (octava==7) multiplicador_octava=128;

	int frecuencia_final=freq*multiplicador_octava;

	//printf ("freq: %d octava: %d frecuencia final: %d Hz\n",freq,octava,frecuencia_final);

	return frecuencia_final;
}

void saa_simul_establece_frecuencia(z80_byte canal)
{


	int freq=sam_saa_simul_chip[8+canal];
	int octava;
	if (canal==0) octava=sam_saa_simul_chip[16]&7;
	if (canal==1) octava=(sam_saa_simul_chip[16]>>4)&7;
	if (canal==2) octava=sam_saa_simul_chip[17]&7;
	if (canal==3) octava=(sam_saa_simul_chip[17]>>4)&7;
	if (canal==4) octava=sam_saa_simul_chip[18]&7;
	if (canal==5) octava=(sam_saa_simul_chip[18]>>4)&7;

	int frecuencia_final=saa_simul_calcular_frecuencia(freq,octava); //max 7810

	//en chip ay, frecuencia =
	// f=FRECUENCIA_AY/(r*16). siendo r el contenido de los registros de tono del chip ay

	//deseamos obtener r
	//  r=FRECUENCIA_AY/(f*16)

	int frecuencia_registro;


	if (frecuencia_final==0) frecuencia_registro=0xFFF;
	else frecuencia_registro=FRECUENCIA_AY/(frecuencia_final*16);


	//enviamos los dos valores.
	ay_chip_selected=0;

	if (canal>2) {
		canal -=3;
		ay_chip_selected++;
	}

	int registro_ay=canal*2;

	//printf ("frecuencia final: %d Hz Registro frecuencia ay: %d\n",frecuencia_final,frecuencia_registro);


	out_port_ay(65533,registro_ay);
	out_port_ay(49149,frecuencia_registro & 0xFF);

	out_port_ay(65533,registro_ay+1);
	out_port_ay(49149,(frecuencia_registro>>8) & 0xF );


}

int saa_simul_calcular_frecuencia_ruido(z80_byte freq,int generador)
{

	//Suponemos:

	int frecuencia=0;

	z80_byte bits_bajos;

	if (generador==0) {
		bits_bajos=freq&3;
	}

	else {
		bits_bajos=(freq>>4)&3;
	}

	switch (bits_bajos) {
		case 0:
			frecuencia=31300;
		break;

		case 1:
			frecuencia=15600;
		break;

		case 2:
			frecuencia=7600;
		break;

		case 3:
			//La frecuencia viene por la frecuencia que indica el canal asociado al generador:
			//Generador 0: tono del canal 0
			//Generador 1: tono del canal 3
			frecuencia=ay_retorna_frecuencia(0,generador);
		break;
	}

	return frecuencia;
}

int saa_simul_convert_frec_ruido_saa_ay(int frecuencia)
{
	//int F=FRECUENCIA_NOISE/(r*16)
	//R= FRECUENCIA_NOISE/(F*16)
	//rango del chip AY 1787 hz - 886700 hz. 5 bits (0..31)

        int frecuencia_registro;

        if (frecuencia==0) frecuencia_registro=31;
        else frecuencia_registro=FRECUENCIA_NOISE/(frecuencia*16);

	return frecuencia_registro;
}


void saa_simul_establece_frecuencia_ruido(void)
{
	z80_byte freq=sam_saa_simul_chip[0x16];

	int frecuencia_final=saa_simul_calcular_frecuencia_ruido(freq,0);

	frecuencia_final=saa_simul_calcular_frecuencia_ruido(freq,0);
	int frecuencia_registro_gen0=saa_simul_convert_frec_ruido_saa_ay(frecuencia_final);
	//printf ("frecuencia ruido gen0 final: %d Hz Registro frecuencia ay: %d\n",frecuencia_final,frecuencia_registro_gen0);


	frecuencia_final=saa_simul_calcular_frecuencia_ruido(freq,1);
	int frecuencia_registro_gen1=saa_simul_convert_frec_ruido_saa_ay(frecuencia_final);
	//printf ("frecuencia ruido gen1 final: %d Hz Registro frecuencia ay: %d\n",frecuencia_final,frecuencia_registro_gen1);


	ay_chip_selected=0;
	out_port_ay(65533,6);
	out_port_ay(49149,frecuencia_registro_gen0 & 31);

	ay_chip_selected=1;
        out_port_ay(65533,6);
        out_port_ay(49149,frecuencia_registro_gen1 & 31);
}



void saa_simul_write_address(z80_byte value)
{
		//Seleccion registro chip sonido
		//printf ("saa_simul1099 address port. Value: %02XH\n",value);

		sam_saa_simul_chip_last_selected=value;
}


void saa_simul_write_data(z80_byte value)
{
		//Valor registro chip sonido
		//printf ("saa_simul1099 data port. Value: %02XH\n",value);

		sam_saa_simul_chip[sam_saa_simul_chip_last_selected&31]=value;

		//Si son volumenes
		if (sam_saa_simul_chip_last_selected<6) {
			//Hacemos out de parte baja y parte alta
			z80_byte highnibble=value>>4;
			value=value | highnibble;
			value=value & 15; //max volumen 15, para no activar envolventes

			ay_chip_selected=0;

			//Siguientes 3 canales en el otro chip de sonido
			if (sam_saa_simul_chip_last_selected>=3) {
				sam_saa_simul_chip_last_selected -=3;
				ay_chip_selected++;
			}

			out_port_ay(65533,8+sam_saa_simul_chip_last_selected);
			out_port_ay(49149,value);
		}

		//Si es frecuencia de ruido
		if (sam_saa_simul_chip_last_selected==0x16) {
			saa_simul_establece_frecuencia_ruido();
		}

		//Si son frecuencias o octavas
		if (
		(sam_saa_simul_chip_last_selected>=8 && sam_saa_simul_chip_last_selected<=13)
		||
		(sam_saa_simul_chip_last_selected>=16 && sam_saa_simul_chip_last_selected<=18)
		)
		{
			//Si son frecuencias
			if (sam_saa_simul_chip_last_selected>=8 && sam_saa_simul_chip_last_selected<=13) {
				saa_simul_establece_frecuencia(sam_saa_simul_chip_last_selected-8);

			}

			//Si son cambios de octavas
			if (sam_saa_simul_chip_last_selected==16) {
				saa_simul_establece_frecuencia(0); //Canal 0. Registro 16
				saa_simul_establece_frecuencia(1); //Canal 1. Registro 16
			}
			if (sam_saa_simul_chip_last_selected==17) {
				saa_simul_establece_frecuencia(2); //Canal 2. Registro 17
				saa_simul_establece_frecuencia(3); //Canal 3. Registro 17
			}
			if (sam_saa_simul_chip_last_selected==18) {
				saa_simul_establece_frecuencia(4); //Canal 4. Registro 18
				saa_simul_establece_frecuencia(5); //Canal 5. Registro 18
			}
		}

		//Activacion tono o ruido
		if (sam_saa_simul_chip_last_selected==20 || sam_saa_simul_chip_last_selected==21) {
/*
R7 � Control del mezclador y de E/S
D7 No utilizado
D6 1=puerta de entrada, 0=puerta de salida
D5 Ruido en el canal C
D4 Ruido en el canal B
D3 Ruido en el canal A
D2 Tono en el canal C
D1 Tono en el canal B
DO Tono en el canal A
*/
			//de momento solo tonos
			z80_byte valor_mixer=255;

			z80_byte mixer_tonos=sam_saa_simul_chip[20];
			z80_byte mixer_ruido=sam_saa_simul_chip[21];

			if (mixer_tonos&1) valor_mixer &=(255-1); //Canal 0 tono
			if (mixer_tonos&2) valor_mixer &=(255-2); //Canal 1 tono
			if (mixer_tonos&4) valor_mixer &=(255-4); //Canal 2 tono

			if (mixer_ruido&1) {
				valor_mixer &=(255-8); //Canal 0 ruido
				//Si canal 0 ruido, canal 0 tono desactivado
				valor_mixer |=1;
			}
			if (mixer_ruido&2) valor_mixer &=(255-16); //Canal 1 ruido
			if (mixer_ruido&4) valor_mixer &=(255-32); //Canal 2 ruido

			//printf ("set mixer chip ay 0. tonos: %02XH ruidos: %02XH final: %02XH\n",mixer_tonos,mixer_ruido,valor_mixer);

			ay_chip_selected=0;
			out_port_ay(65533,7);
			out_port_ay(49149,valor_mixer);

			valor_mixer=255;
                        if (mixer_tonos&8) valor_mixer &=(255-1); //Canal 0 tono
                        if (mixer_tonos&16) valor_mixer &=(255-2); //Canal 1 tono
                        if (mixer_tonos&32) valor_mixer &=(255-4); //Canal 2 tono

                        if (mixer_ruido&8) {
				valor_mixer &=(255-8); //Canal 0 ruido
				//Si canal 3 ruido, canal 0 tono desactivado
                                valor_mixer |=1;
			}
                        if (mixer_ruido&16) valor_mixer &=(255-16); //Canal 1 ruido
                        if (mixer_ruido&32) valor_mixer &=(255-32); //Canal 2 ruido

                        //printf ("set mixer chip ay 1. tonos: %02XH ruidos: %02XH final: %02XH\n",mixer_tonos,mixer_ruido,valor_mixer);

                        ay_chip_selected=1;
                        out_port_ay(65533,7);
                        out_port_ay(49149,valor_mixer);

		}
}
