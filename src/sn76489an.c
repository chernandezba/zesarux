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



CV's sound chip
---------------

The sound chip in the CV is a SN76489AN manufactured by Texas Instruments.
It has 4 sound channels- 3 tone and 1 noise.

The volume of each channel can be controlled seperately in 16 steps from
full volume to silence.

A byte written into the sound chip determines which register is used, along
with the frequency/ attenuation information.

The frequency of each channel is represented by 10 bits.  10 bits won't
fit into 1 byte, so the data is written in as 2 bytes.

Here's the control word:

+--+--+--+--+--+--+--+--+
|1 |R2|R1|R0|D3|D2|D1|D0|
+--+--+--+--+--+--+--+--+

1: This denotes that this is a control word
R2-R0 the register number:

000 Tone 1 Frequency
001 Tone 1 Volume
010 Tone 2 Frequency
011 Tone 2 Volume
100 Tone 3 Frequency
101 Tone 3 Volume
110 Noise Control
111 Noise Volume

D3-D0 is the data

Here's the second frequency register:

+--+--+--+--+--+--+--+--+
|0 |xx|D9|D8|D7|D6|D5|D4|
+--+--+--+--+--+--+--+--+

0: This denotes that we are sending the 2nd part of the frequency

D9-D4 is 6 more bits of frequency 


To write a 10-bit word for frequenct into the sound chip you must first
send the control word, then the second frequency register.  Note that the
second frequency register doesn't have a register number.  When you write
to it, it uses which ever register you used in the control word.

So, if we want to output 11 0011 1010b to tone channel 1:

First, we write the control word:

LD A,1000 1010b
OUT (F0h),A

Then, the second half of the frequency:

LD A,0011 0011b
OUT (F0h),A

To tell the frequency of the wave generated, use this formula:


   3579545
f= -------
     32n

Where f= frequency out,
and n= your 10-bit binary number in


To control the Volume:


+--+--+--+--+--+--+--+--+
|1 |R2|R1|R0|V3|V2|V1|V0|
+--+--+--+--+--+--+--+--+

R2-R0 tell the register

V3-V0 tell the volume:

0000=Full volume
.
.
.
1111=Silence


The noise source is quite intresting.  It has several modes of operation.
Here's a control word:

+--+--+--+--+--+--+--+--+
|1 |1 |1 |0 |xx|FB|M1|M0|
+--+--+--+--+--+--+--+--+

FB= Feedback:

0= 'Periodic' noise
1= 'white' noise 

The white noise sounds, well, like white noise.
The periodic noise is intresting.  Depending on the frequency, it can
sound very tonal and smooth.

M1-M0= mode bits:

00= Fosc/512  Very 'hissy'; like grease frying
01= Fosc/1024 Slightly lower
10= Fosc/2048 More of a high rumble
11= output of tone generator #3

You can use the output of gen. #3 for intresting effects.  If you sweep
the frequency of gen. #3, it'll cause a cool sweeping effect of the noise.
The usual way of using this mode is to attenuate gen. #3, and use the
output of the noise source only.

The attenuator for noise works in the same way as it does for the other
channels.



*/


// This audio emulation code derived from the ay emulation core from myself,
// just reusing AY code and removing what doesnt exist on the SN

#include <stdio.h>
//#include <math.h>


#include "sn76489an.h"
#include "cpu.h"
#include "audio.h"
#include "debug.h"
#include "joystick.h"


//Indica si esta presente el chip o no
z80_bit sn_chip_present;


//Por defecto la frecuencia del spectrum
int sn_chip_frequency=FRECUENCIA_COLECO_SN;



//Valores de volumen
//Al reves:
/*
0000=Full volume
.
.
.
1111=Silence
*/
char sn_volume_table[16]={24,20,16,15,
			   10,8,6,4,
			   3,2,2,1,
			   1,1,0,0};			   





//valores de 16 bits con signo
//tempp  short sn_sine_table[FRECUENCIA_CONSTANTE_NORMAL_SONIDO];

short sn_sine_table[FRECUENCIA_CONSTANTE_NORMAL_SONIDO];

//tabla suficientemente grande como para que al aumentar la frecuencia del sonido no se salga de rango
//soportar hasta cpu 1000% (10 veces msnor)
//short sn_sine_table[15600*10];






//Ultimo registro seleccionado por el puerto 65533
//z80_byte sn_3_8912_registro_sel[MAX_SN_CHIPS];
z80_byte sn_3_8912_registro_sel;


//Ultimo canal seleccionado al cambiar valor frecuencia tipo fino
z80_byte sn_last_audio_channel_frequency=0;


//frecuencia de cada canal
//int sn_freq_tono_A[MAX_SN_CHIPS];
int sn_freq_tono_A;
//int sn_freq_tono_B[MAX_SN_CHIPS];
int sn_freq_tono_B;
//int sn_freq_tono_C[MAX_SN_CHIPS];
int sn_freq_tono_C;

//contador de cada canal... (FRECUENCIA_CONSTANTE_NORMAL_SONIDO/freq_tono)
//int sn_contador_tono_A[MAX_SN_CHIPS];
int sn_contador_tono_A;
//int sn_contador_tono_B[MAX_SN_CHIPS];
int sn_contador_tono_B;
//int sn_contador_tono_C[MAX_SN_CHIPS];
int sn_contador_tono_C;

//ultimo valor enviado para cada canal, valores con signo:
//short sn_ultimo_valor_tono_A[MAX_SN_CHIPS];
//short sn_ultimo_valor_tono_B[MAX_SN_CHIPS];
//short sn_ultimo_valor_tono_C[MAX_SN_CHIPS];

short sn_ultimo_valor_tono_A;
short sn_ultimo_valor_tono_B;
short sn_ultimo_valor_tono_C;


//frecuencia de canal de ruido
//int sn_freq_ruido[MAX_SN_CHIPS];
int sn_freq_ruido;

//contador de canal de ruido .... (FRECUENCIA_CONSTANTE_NORMAL_SONIDO/sn_freq_ruido)
//int sn_contador_ruido[MAX_SN_CHIPS];
int sn_contador_ruido;

//ultimo valor enviado para canal de ruido. valor con signo:
//short sn_ultimo_valor_ruido[MAX_SN_CHIPS];
short sn_ultimo_valor_ruido;

//valor randomize
//z80_int sn_randomize_noise[MAX_SN_CHIPS];
z80_int sn_randomize_noise;




/*
RO � Ajuste fino del tono, canal A (4 low bits)
R1 � Ajuste aproximado del tono, canal A- (6 high bits)
R2 � Ajuste fino del tono, canal B
R3 � Ajuste aproximado del tono, canal B
R4 � Ajuste fino del tono, canal C
R5 � Ajuste aproximado del tono, canal C

R6 Volume A
R7 Volume B
R8 Volume C
R9 Noise
R10 Noise Volume

//Aunque solo uso 10, dejo total 16 
*/
z80_byte sn_chip_registers[16];




//Si hsn que autoactivar chip SN en el caso que alguien lo use (lectura o escritura en el puerto SN)
z80_bit autoenable_sn_chip={1};

void init_chip_sn(void)
{


	//inicializamos el chip aunque este desactivado
	//if (sn_chip_present.v==0) return;

	debug_printf (VERBOSE_INFO,"Initializing SN Chip");




	//resetear valores de puertos de sonido
	int r;
	for (r=0;r<16;r++) sn_chip_registers[r]=255;



	//ultimo valor enviado para cada canal, valores con signo:
	sn_ultimo_valor_tono_A=+32767;
	sn_ultimo_valor_tono_B=+32767;
	sn_ultimo_valor_tono_C=+32767;

	

	sn_ultimo_valor_ruido=+32767;
	

	int i;



// Onda cuadrada
	for (i=0;i<FRECUENCIA_CONSTANTE_NORMAL_SONIDO/2;i++) {
		sn_sine_table[i]=+32767;
	}
	for (;i<FRECUENCIA_CONSTANTE_NORMAL_SONIDO;i++) {
		sn_sine_table[i]=-32767;
	}

	//Establecemos frecuencia
	if (MACHINE_IS_SG1000) sn_chip_frequency=FRECUENCIA_SG1000_SN;
    else if (MACHINE_IS_SMS) sn_chip_frequency=FRECUENCIA_SMS_SN;
	else sn_chip_frequency=FRECUENCIA_COLECO_SN;


	debug_printf (VERBOSE_INFO,"Setting SN chip frequency to %d HZ",sn_chip_frequency);




//Onda senoidal. activar -lm en proceso de compilacion
/*
float sineval,radians;
	for (i=0;i<FRECUENCIA_CONSTANTE_NORMAL_SONIDO;i++) {
		radians=i;
		radians=radians*6.28318530718;

		radians=radians/((float)(FRECUENCIA_CONSTANTE_NORMAL_SONIDO));
		sineval=sin(radians);
		sn_sine_table[i]=32767*sineval;

		debug_printf (VERBOSE_DEBUG,"i=%d radians=%f sine=%f value=%d",i,radians,sineval,sn_sine_table[i]);
	}

*/


}





void sn_randomize(void) 
{
/*
;Seguimos la misma formula RND del spectrum:
;0..1 -> n=(75*(n+1)-1)/65536
;0..65535 -> n=65536/(75*(n+1)-1)
generar_random_noise:
*/

        int resultado;
        int r;

        r=sn_randomize_noise;

        resultado=(75*(r+1)-1);

        sn_randomize_noise=resultado & 0xFFFF;

        //printf ("sn_randomize_noise: %d\n",sn_randomize_noise);


}



//Generar salida aleatoria
void sn_chip_valor_aleatorio(void)
{

	sn_randomize();

/*
          ;Generar +1 o -1
          ;0..32767 -> +1
          ;32768..65535 -> -1
  */

	if (sn_randomize_noise<32768) sn_ultimo_valor_ruido=+32767;
	else sn_ultimo_valor_ruido=-32767;

	//printf ("Cambio ruido a : %d\n",sn_ultimo_valor_ruido);

}



//Devuelve la salida del canal indicado, devuelve valor con signo
char sn_da_output_canal(short ultimo_valor_tono,z80_byte volumen)
{

	//valor con signo
	char valor8;
	int valor;

	
	valor=ultimo_valor_tono;
	silence_detection_counter=0;
	
	volumen=volumen & 15; //Evitar valores de volumen fuera de rango que vengan de los registros de volumen


    valor=valor*sn_volume_table[volumen];
	valor=valor/32767;
	valor8=valor;


	return valor8;


}



//Devuelve la salida del canal de ruido, devuelve valor con signo
char sn_da_output_canal_ruido(void)
{

	//valor con signo
	char valor8;
	int valor;

	z80_byte volumen=sn_chip_registers[10];


	
	valor=sn_ultimo_valor_ruido;
	silence_detection_counter=0;
        


	volumen=volumen & 15; //Evitar valores de volumen fuera de rango que vengan de los registros de volumen

	//if (volumen>15) printf ("  Error volumen >15 : %d\n",volumen);
    valor=valor*sn_volume_table[volumen];
	valor=valor/32767;
	valor8=valor;
	//printf ("valor final tono: %d\n",valor8);

	//if (valor8>24) printf ("valor final tono: %d\n",valor8);
	//if (valor8<-24) printf ("valor final tono: %d\n",valor8);

	return valor8;


}

//Devuelve el sonido de salida del chip de los 3 canales
char da_output_sn(void)
{


	int valor_enviar_sn=0;
	if (sn_chip_present.v==1) {


		valor_enviar_sn +=sn_da_output_canal(sn_ultimo_valor_tono_A,sn_chip_registers[6]);
		valor_enviar_sn +=sn_da_output_canal(sn_ultimo_valor_tono_B,sn_chip_registers[7]);
		valor_enviar_sn +=sn_da_output_canal(sn_ultimo_valor_tono_C,sn_chip_registers[8]);

		valor_enviar_sn +=sn_da_output_canal_ruido();

	}


	return valor_enviar_sn;

}




//Calcular e invertir , si conviene, salida de cada canal
void sn_chip_siguiente_ciclo_siguiente(void)
{

	if (sn_chip_present.v==0) return;



	//actualizamos contadores de frecuencias
	sn_ultimo_valor_tono_A=sn_sine_table[sn_contador_tono_A];
	sn_contador_tono_A +=sn_freq_tono_A;
	if (sn_contador_tono_A>=FRECUENCIA_CONSTANTE_NORMAL_SONIDO) {
			sn_contador_tono_A -=FRECUENCIA_CONSTANTE_NORMAL_SONIDO;
	}

	sn_ultimo_valor_tono_B=sn_sine_table[sn_contador_tono_B];
	sn_contador_tono_B +=sn_freq_tono_B;
	if (sn_contador_tono_B>=FRECUENCIA_CONSTANTE_NORMAL_SONIDO) {
			sn_contador_tono_B -=FRECUENCIA_CONSTANTE_NORMAL_SONIDO;
	}

	sn_ultimo_valor_tono_C=sn_sine_table[sn_contador_tono_C];
	sn_contador_tono_C +=sn_freq_tono_C;
	if (sn_contador_tono_C>=FRECUENCIA_CONSTANTE_NORMAL_SONIDO) {
			sn_contador_tono_C -=FRECUENCIA_CONSTANTE_NORMAL_SONIDO;
	} 


	sn_contador_ruido +=sn_freq_ruido;
	if (sn_contador_ruido>=FRECUENCIA_CONSTANTE_NORMAL_SONIDO) {
			sn_contador_ruido -=FRECUENCIA_CONSTANTE_NORMAL_SONIDO;
			sn_chip_valor_aleatorio();
	}

	

}



void sn_chip_siguiente_ciclo(void)
{


	sn_chip_siguiente_ciclo_siguiente();
	

}


//Calcular contadores de incremento
//void sn_establece_frecuencia_tono(z80_byte indice, int *freq_tono, int *contador_tono)
void sn_establece_frecuencia_tono(z80_byte indice, int *freq_tono)
{

/*
Frecuencia real= X = (CPU Speed / 32) / Desired frequency
*/


	int freq_temp;
	

	freq_temp=(sn_chip_registers[indice] & 0xF) | ((sn_chip_registers[indice+1] & 63)<<4);

	//printf ("Valor freq_temp : %d Hz\n",freq_temp);

	//controlamos divisiones por cero
	if (!freq_temp) freq_temp++;

	freq_temp=freq_temp*SN_DIVISOR_FRECUENCIA;




	*freq_tono=FRECUENCIA_SN/freq_temp;


    if (*freq_tono>FRECUENCIA_CONSTANTE_NORMAL_SONIDO) {
		
		*freq_tono=FRECUENCIA_CONSTANTE_NORMAL_SONIDO;
	}

	//si la frecuencia del tono es exactamente igual a la del sonido, pasara que siempre el valor de retorno
	//sera el primer valor en el arrsn de sn_sine_table.. seguramente +1 (32767)
	//alteramos un poco el valor
	if ( (*freq_tono)==FRECUENCIA_CONSTANTE_NORMAL_SONIDO) (*freq_tono)=FRECUENCIA_CONSTANTE_NORMAL_SONIDO-10;

	//de logica deberia resetear esto a 0.... pero si lo hago distorsiona el sonido
	//mejor dejar el contador con el valor actual
	//temp
	//*contador_tono=0;




}

//Retorna la frecuencia del tono sobre un valor concreto del chip de sonido
//Se le pasa valores fino (8 bits bajos) y algo (8 bits altos)
int sn_retorna_frecuencia_valor_registro(z80_byte value_l,z80_byte value_h)
{
	int freq_temp;
	
int freq_tono;
	
	freq_temp=(value_l & 0xF) | ((value_h & 63)<<4);

	//printf ("Valor freq_temp : %d Hz\n",freq_temp);

	//controlamos divisiones por cero
	if (!freq_temp) freq_temp++;

	freq_temp=freq_temp*SN_DIVISOR_FRECUENCIA;




	freq_tono=FRECUENCIA_SN/freq_temp;


	return freq_tono;

}


//Retorna la frecuencia de un registro concreto del chip AY de sonido
int sn_retorna_frecuencia(int registro)
{

	int indice=registro*2;

	return sn_retorna_frecuencia_valor_registro(sn_chip_registers[indice],sn_chip_registers[indice+1]);



}


void sn_set_register_port(z80_byte value)
{
		//seleccion de registro
		sn_3_8912_registro_sel=value & 15; //evitamos valores fuera de rango	
}


void sn_establece_frecuencia_ruido(void)
{

	
			//Frecuencia ruido
			int freq_temp=sn_chip_registers[9] & 31;
	       		//printf ("Valor registros ruido : %d Hz\n",freq_temp);

			//controlamos divisiones por cero
			if (!freq_temp) freq_temp++;

			freq_temp=freq_temp*SN_DIVISOR_FRECUENCIA;



			sn_freq_ruido=SN_FRECUENCIA_NOISE/freq_temp;
			//printf ("Frecuencia ruido: %d Hz\n",sn_freq_ruido);

			//sn_freq_ruido realmente tiene frecuencia*2... dice cada cuando se conmuta de signo
			
			sn_freq_ruido=sn_freq_ruido*2;



			if (sn_freq_ruido>FRECUENCIA_CONSTANTE_NORMAL_SONIDO) {
	                  
        	          sn_freq_ruido=FRECUENCIA_CONSTANTE_NORMAL_SONIDO;
			}


			//si la frecuencia del ruido es exactamente igual a la del sonido
			//alteramos un poco el valor
			if ( sn_freq_ruido==FRECUENCIA_CONSTANTE_NORMAL_SONIDO) sn_freq_ruido=FRECUENCIA_CONSTANTE_NORMAL_SONIDO-10;



			//printf ("Frecuencia ruido final: %d Hz\n",sn_freq_ruido);


		

}


//Enviar valor a puerto
void sn_set_value_register(z80_byte value)
{

	//Resetear detector de silencio
	silence_detection_counter=0;



		//valor a registro
		

		sn_chip_registers[sn_3_8912_registro_sel&15]=value;




		if (sn_3_8912_registro_sel ==0 || sn_3_8912_registro_sel == 1) {
			//Canal A
			sn_establece_frecuencia_tono(0,&sn_freq_tono_A);

		}

		if (sn_3_8912_registro_sel ==2 || sn_3_8912_registro_sel == 3) {
			//Canal B
			sn_establece_frecuencia_tono(2,&sn_freq_tono_B);

		}


		if (sn_3_8912_registro_sel ==4 || sn_3_8912_registro_sel == 5) {
			//Canal C
			sn_establece_frecuencia_tono(4,&sn_freq_tono_C);
		}

		if (sn_3_8912_registro_sel ==9) {
			//Frecuencia ruido
			sn_establece_frecuencia_ruido();

		}

	

	
}





/*

Filtros de salida sn chip
por chip(0...2) y canal (0..2)

tono si/no
ruido si/no
desactivado del todo si/no


R7 � Control del mezclador y de E/S
D7 No utilizado
D6 1=puerta de entrada, 0=puerta de salida
D5 Ruido en el canal C
D4 Ruido en el canal B
D3 Ruido en el canal A
D2 Tono en el canal C
D1 Tono en el canal B
DO Tono en el canal A

a 1 para desactivar eso
Canal A todo activado: mascara xxxx0xx0 -> mascara or
Canal A sin ruido: xxxx1xx0
Canal A sin tono: xxxx0xx1
Canal A sin nada: xxxx1xx1


Canal B todo activado: mascara xxx0xx0x -> mascara or
Canal B sin ruido: xxx1xx0x
Canal B sin tono: xxx0xx1x
Canal B sin nada: xxx1xx1x

Canal C todo activado: mascara xx0xx0xx -> mascara or
Canal C sin ruido: xx1xx0xx
Canal C sin tono: xx0xx1xx
Canal C sin nada: xx1xx1xx

 */

//A 0 todos para normal
z80_byte sn_filtros;

void sn_init_filters(void)
{

		sn_filtros=0;
	
}





void sn_set_volume_noise(z80_byte volume)
{

                sn_chip_registers[10]=volume;

}


void sn_set_volume_tone_channel(z80_byte canal,z80_byte volumen_final)
{

            sn_set_register_port(6+canal);
            sn_set_value_register(volumen_final);        
}

//de momento no se establece tipo
void sn_set_noise_type(z80_byte tipo)
{
/*

TODO: emulacion correcta de esto:

The noise source is quite intresting.  It has several modes of operation.
Here's a control word:

+--+--+--+--+--+--+--+--+
|1 |1 |1 |0 |xx|FB|M1|M0|
+--+--+--+--+--+--+--+--+

FB= Feedback:

0= 'Periodic' noise
1= 'white' noise 

The white noise sounds, well, like white noise.
The periodic noise is intresting.  Depending on the frequency, it can
sound very tonal and smooth.

M1-M0= mode bits:

00= Fosc/512  Very 'hissy'; like grease frying
01= Fosc/1024 Slightly lower
10= Fosc/2048 More of a high rumble
11= output of tone generator #3

You can use the output of gen. #3 for intresting effects.  If you sweep
the frequency of gen. #3, it'll cause a cool sweeping effect of the noise.
The usual way of using this mode is to attenuate gen. #3, and use the
output of the noise source only.

The attenuator for noise works in the same way as it does for the other
channels.


				*/

	sn_set_register_port(9);

	z80_byte frecuencia_ruido=31;


	tipo &=3;

	z80_byte divisor_ruido=tipo+1; //Evitamos divisiones por 0

	frecuencia_ruido /=divisor_ruido;

	sn_set_value_register(frecuencia_ruido); //mitad del maximo aprox (31/2)

                /*
                R6 � Control del generador de ruido, D4-DO
El periodo del generador de ruido se toma contando los cinco bits inferiores del regis-
tro de ruido cada periodo del reloj de sonido dividido por 16.
                */	
}


//4 low bits of frequency
void sn_set_channel_fine_tune(z80_byte canal,z80_byte fino)
{


            sn_set_register_port(2*canal);
            sn_set_value_register(fino);   

}

//10 low bits of frequency
void sn_set_channel_aprox_tune(z80_byte canal,z80_byte aproximado)
{
            sn_set_register_port(1+2*canal);
            sn_set_value_register(aproximado);   
}




void sn_out_port_sound(z80_byte value)
{


    if (value & 128) {
        //|1 |R2|R1|R0|D3|D2|D1|D0|
        z80_byte sound_register=(value >>4) &7;
        z80_byte sound_data=value & 15;

        /*
        1: This denotes that this is a control word
R2-R0 the register number:

000 Tone 1 Frequency
001 Tone 1 Volume
010 Tone 2 Frequency
011 Tone 2 Volume
100 Tone 3 Frequency
101 Tone 3 Volume
110 Noise Control
111 Noise Volume
        */



        int canal=sound_register/2;

        //printf ("Canal: %d\n",canal);

        int tipo=sound_register & 1;



        if (canal==3) {
            /*
            110 Noise Control
            111 Noise Volume
            */
            //ruido
            if (tipo==0) {
                //|1 |1 |1 |0 |xx|FB|M1|M0|
                
                //establecer frecuencia ruido
                sn_set_noise_type(sound_data);

				//printf ("Establecer ruido: %02XH\n",sound_data & 7);

            }
            if (tipo==1) {
                //Noise volume
                //printf ("ruido\n");
                sn_set_volume_noise(sound_data);
            }

            return;
        }

        

        if (tipo==0) {
            sn_last_audio_channel_frequency=canal;

            sn_set_register_port(2*canal);
            sn_set_value_register(sound_data);               
        }

        if (tipo==1) {
            sn_set_volume_tone_channel(canal,sound_data);
        }




    }

    else {

        //Parte alta de la frecuencia
        /*
Here's the second frequency register:

+--+--+--+--+--+--+--+--+
|0 |xx|D9|D8|D7|D6|D5|D4|
+--+--+--+--+--+--+--+--+

0: This denotes that we are sending the 2nd part of the frequency

D9-D4 is 6 more bits of frequency 


To write a 10-bit word for frequenct into the sound chip you must first
send the control word, then the second frequency register.  Note that the
second frequency register doesn't have a register number.  When you write
to it, it uses which ever register you used in the control word.

        */

       //sn_last_audio_channel_frequency


            //sn_chip_registers[sn_last_audio_channel_frequency*2+1]=value;


            //int frecuencia=coleco_get_frequency_channel(sn_last_audio_channel_frequency);
            //coleco_set_sn_freq(sn_last_audio_channel_frequency,frecuencia);


            sn_set_register_port(2*sn_last_audio_channel_frequency+1);
            sn_set_value_register(value);               
    }
}



void sn_establece_frecuencias_todos_canales(void)
{
		
	sn_establece_frecuencia_tono(0,&sn_freq_tono_A);

	sn_establece_frecuencia_tono(2,&sn_freq_tono_B);

	sn_establece_frecuencia_tono(4,&sn_freq_tono_C);

	sn_establece_frecuencia_ruido();
		
}			