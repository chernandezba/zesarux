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


#include "ql_i8049.h"
#include "ql.h"
#include "debug.h"
#include "utils.h"
#include "audio.h"

//Para usar funcion random
#include "ay38912.h"


/*

Intel 8049 "Intelligent Peripheral Controller (IPC)" emulation

Actually it is not a real emulation, I am not emulating the cpu, but simulating its behaviour

Info:

The 8049, IC24, is a totally self-sufficient 8-bit single chip microcomputer containing 2 k bytes of program memory and 128 bytes of RAM. It is clocked internally at 11 MHz from crystal X4

In this application the function of the 8049 is to:

receive RS232 interface signals,
monitor the keyboard,
control the loudspeaker,
control the joystick.
The IPC utilises a data bus, two 8-bit I/O ports and some control lines to control these functions.

*/

#define QL_STATUS_IPC_IDLE 0
#define QL_STATUS_IPC_WRITING 1

//Valores que me invento para gestionar pulsaciones de teclas no ascii
/*
#define QL_KEYCODE_F1 256
#define QL_KEYCODE_F2 257
#define QL_KEYCODE_UP 258
#define QL_KEYCODE_DOWN 259
#define QL_KEYCODE_LEFT 260
#define QL_KEYCODE_RIGHT 261
*/

unsigned char ql_ipc_last_write_value=0;

//Ultimo comando recibido
unsigned char ql_ipc_last_command=0;

//Alterna bit ready o no leyendo el serial bit de ipc
int ql_ipc_reading_bit_ready=0;

//Ultimo parametro de comando recibido
unsigned char ql_ipc_last_command_parameter;

int ql_ipc_last_write_bits_enviados=0;
int ql_estado_ipc=QL_STATUS_IPC_IDLE;
int ql_ipc_bytes_received=0;

unsigned char ql_ipc_last_nibble_to_read[32];
int ql_ipc_last_nibble_to_read_mascara=8;
int ql_ipc_last_nibble_to_read_index=0;
int ql_ipc_last_nibble_to_read_length=1;


// Parametros de sonido
int i8049_chip_present=0;




/*
Formato del mensaje ipc de enviar sonido:

8 bits pitch 1
8 bits pitch 2
16 bits  interval between steps (grad_x)
16 bits duration
4 bits step in pitch (grad_y)
4 bits wrap
4 bits randomness of step
4 bits fuzziness

no reply

Para aproximar, cada "duration" es un scanline


*/

unsigned char ql_audio_pitch1;
unsigned char ql_audio_pitch2;
moto_int ql_audio_grad_x;
moto_int ql_audio_duration;
unsigned char ql_audio_grad_y;
unsigned char ql_audio_wrap;
unsigned char ql_audio_randomness_of_step;
unsigned char ql_audio_fuziness;


//Estos dos modificados para usar como contadores de la frecuencia
moto_int ql_audio_pitch_counter_initial=0;
moto_int ql_audio_pitch_counter_current=0;

//Bit 0/1 del sonido en ejecucion
int ql_audio_output_bit=0;


//Duracion actual del sonido, se va decrementando
moto_int ql_current_sound_duration=0;


//Si hay sonido produciendose
int ql_audio_playing=0;


// Fin Parametros de sonido




const int ql_i8049_sound_chip_frequency=11000000;



/*

Tabla que relaciona pitch de QL con frecuencias

Valores reales obtenidos de tablas de varios programas de QL:

Nota, frecuencia, pitch
{"A3",220,41},
{"A#3",233,38},
{"B3",246,36},

{"C4",261,33},
{"C#4",277,31},
{"D4",293,28},
{"D#4",311,26},
{"E4",329,24},

{"F4",349,22},
{"F#4",369,20},
{"G4",392,19},
{"G#4",415,17},
{"A4",440,15},
{"A#4",466,14},
{"B4",493,12},
{"C5",523,11},
{"C#5",554,10},
{"D5",587,9},
{"D#5",622,8},
{"E5",659,7},
{"F5",698,6},
{"F#5",739,5},
{"G5",783,4},
{"G#5",830,3},
{"A5",880,2},
{"A#5",932,1},
{"B5",987,1},
{"C6",1046,0},



El resto obtenido desde el programa playscale, con las formulas:

    int a=11447;
    float b=10.6;
    float f=a/(pitch+b-0.5);


Tabla con decimales:

1046,932,880,830,783,739,698,659,622,587,
554,523,493,495.541107,466,440,438.582367,415,407.366547,392,
369,368.070740,349,345.830841,329,326.125366,311,308.544495,293,292.762146,
285.461365,277,271.900238,261,259.569183,253.813751,246,243.036102,233,233.136459,
228.483047,220,219.712097,215.574387,211.589661,207.749557,204.046356,200.472855,197.022385,193.688675,
190.465897,187.348618,184.331726,181.410461,178.580353,175.837173,173.177002,170.596130,168.091049,165.658463,
163.295303,160.998596,158.765610,156.593704,154.480438,152.423431,150.420502,148.469528,146.568512,144.715546,
142.908859,141.146729,139.427536,137.749695,136.111771,134.512344,132.950058,131.423660,129.931900,128.473633,
127.047729,125.653130,124.288818,122.953812,121.647186,120.368034,119.115509,117.888779,116.687057,115.509590,
114.355644,113.224533,112.115578,111.028130,109.961578,108.915321,107.888786,106.881424,105.892693,104.922089,
103.969124,103.033302,102.114182,101.211319,100.324280,99.452652,98.596039,97.754059,96.926338,96.112511,
95.312241,94.525185,93.751022,92.989441,92.240128,91.502800,90.777161,90.062943,89.359871,88.667694,
87.986160,87.315025,86.654045,86.002998,85.361664,84.729828,84.107269,83.493797,82.889206,82.293312,
81.705917,81.126854,80.555946,79.993011,79.437889,78.890419,78.350441,77.817810,77.292366,76.773972,
76.262489,75.757774,75.259697,74.768120,74.282928,73.803993,73.331192,72.864418,72.403542,71.948456,
71.499062,71.055244,70.616898,70.183937,69.756241,69.333733,68.916313,68.503891,68.096367,67.693672,
67.295708,66.902397,66.513649,66.129402,65.749565,65.374069,65.002838,64.635796,64.272881,63.914013,
63.559132,63.208172,62.861065,62.517746,62.178162,61.842247,61.509937,61.181183,60.855927,60.534107,
60.215675,59.900574,59.588753,59.280163,58.974754,58.672474,58.373276,58.077118,57.783947,57.493721,
57.206394,56.921928,56.640274,56.361397,56.085251,55.811798,55.540997,55.272812,55.007206,54.744141,
54.483578,54.225483,53.969826,53.716564,53.465668,53.217106,52.970844,52.726852,52.485096,52.245548,
52.008175,51.772953,51.539845,51.308830,51.079872,50.852951,50.628040,50.405106,50.184128,49.965080,
49.747936,49.532669,49.319256,49.107677,48.897907,48.689919,48.483692,48.279205,48.076439,47.875366,
47.675968,47.478222,47.282112,47.087616,46.894714,46.703384,46.513611,46.325375,46.138653,45.953430,
45.769691,45.587414,45.406582,45.227180,45.049191,44.872597,44.697384,44.523529,44.351025,44.179852,
44.009995,43.841438,43.674168,43.508171,43.343430,43.179932,

*/

int ql_pitch_frequency_table[256]={
1046,932,880,830,783,739,698,659,622,587,
554,523,493,496,466,440,439,415,407,392,
369,368,349,346,329,326,311,309,293,293,
285,277,272,261,260,254,246,243,233,233,
228,220,220,216,212,208,204,200,197,194,
190,187,184,181,179,176,173,171,168,166,
163,161,159,157,154,152,150,148,147,145,
143,141,139,138,136,135,133,131,130,128,
127,126,124,123,122,120,119,118,117,116,
114,113,112,111,110,109,108,107,106,105,
104,103,102,101,100,99,99,98,97,96,
95,95,94,93,92,92,91,90,89,89,
88,87,87,86,85,85,84,83,83,82,
82,81,81,80,79,79,78,78,77,77,
76,76,75,75,74,74,73,73,72,72,
71,71,71,70,70,69,69,69,68,68,
67,67,67,66,66,65,65,65,64,64,
64,63,63,63,62,62,62,61,61,61,
60,60,60,59,59,59,58,58,58,57,
57,57,57,56,56,56,56,55,55,55,
54,54,54,54,53,53,53,53,52,52,
52,52,52,51,51,51,51,50,50,50,
50,50,49,49,49,49,48,48,48,48,
48,47,47,47,47,47,47,46,46,46,
46,46,45,45,45,45,45,45,44,44,
44,44,44,44,43,43,
};


//Para gestionar repeticiones
int ql_mantenido_pulsada_tecla=0;
int ql_mantenido_pulsada_tecla_timer=0;
//adicionales
int ql_pressed_backspace=0;

//ql_keyboard_table[0] identifica a fila 7 F4     F1      5     F2     F3     F5      4      7
//...
//ql_keyboard_table[7] identifica a fila 0 Shift   Ctrl    Alt      x      v      /      n      ,

//Bits se cuentan desde la izquierda:

// 1      2      4     8      16     32      64    128
//F4     F1      5     F2     F3     F5      4      7

unsigned char ql_keyboard_table[8]={
	255,
	255,
	255,
	255,
	255,
	255,
	255,
	255
};



//Nota: esta matrix de teclas y su numeracion de cada fila está documentada erroneamente en la info del QL de manera ascendente (de 0 a 7),
//mientras que lo correcto, cuando se habla de filas en resultado de comandos ipc, es descendente, tal y como está a continuación:

// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7     ql_keyboard_table[0]
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down     ql_keyboard_table[1]
// 5|     ]      z      .      c      b  Pound      m      '     ql_keyboard_table[2]
// 4|     [   Caps      k      s      f      =      g      ;     ql_keyboard_table[3]
// 3|     l      3      h      1      a      p      d      j     ql_keyboard_table[4]
// 2|     9      w      i    Tab      r      -      y      o     ql_keyboard_table[5]
// 1|     8      2      6      q      e      0      t      u     ql_keyboard_table[6]
// 0| Shift   Ctrl    Alt      x      v      /      n      ,     ql_keyboard_table[7]


//Retorna fila y columna para una tecla pulsada mirando ql_keyboard_table. No se puede retornar por ahora mas de una tecla a la vez
//Se excluye shift, ctrl y alt de la respuesta
//Retorna fila -1 y columna -1 si ninguna tecla pulsada

void ql_return_columna_fila_puertos(int *columna,int *fila)
{

	int c,f;
	c=-1;
	f=-1;

	int i;
	int rotacion;

	unsigned char valor_puerto;
	int salir=0;
	for (i=0;i<8 && salir==0;i++){
		valor_puerto=ql_keyboard_table[i];
		//Si shift ctrl y alt quitarlos
		if (i==7) valor_puerto |=1+2+4;

		//Ver si hay alguna tecla pulsada

		for (rotacion=0;rotacion<8 && salir==0;rotacion++) {
			if ((valor_puerto&1)==0) {
				c=rotacion;
				f=7-i;
				salir=1;
				//printf ("c: %d f: %d\n",c,f);
			}
			else {
				valor_puerto=valor_puerto>>1;
			}
		}
	}

	*columna=c;
	*fila=f;
}


//Mete en valores ipc de vuelta segun teclas pulsadas
/*Desde la rom, cuando se genera una PC_INTR, se pone a leer de ipc lo siguiente y luego ya llama a write_ipc comando 8:


Read PC_INTR pulsado tecla
Returning ipc: 0H. Index nibble: 0 mask: 1
Returning ipc: 0H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Write ipc command 1 pressed key
Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1

Returning ipc: 80H. Index nibble: 0 mask: 8
QL Trap ROM: Tell one key pressed
Returning ipc: 80H. Index nibble: 0 mask: 4
Returning ipc: 80H. Index nibble: 0 mask: 2
Returning ipc: 80H. Index nibble: 0 mask: 1


--Aparentemente estas lecturas anteriores no afectan al valor de la tecla

Lectura de teclado comando. PC=00002F8EH
letra: a
no repeticion


*/


//int temp_flag_reves=0;
void ql_ipc_write_ipc_teclado(void)
{
	/*
	* This returns one nibble, plus up to 7 nibble/byte pairs:
	* first nibble, ms bit: set if final last keydef is still held
	* first nibble, ls 3 bits: count of keydefs to follow.
	* then, for each of the 0..7 keydefs:
	* nibble, bits are 3210=lsca: lost keys (last set only), shift, ctrl and alt.
	* byte, bits are 76543210=00colrow: column and row as keyrow table.
	* There is a version of the IPC used on the thor that will also return keydef
	* values for a keypad. This needs looking up
	*/

	/*Devolveremos una tecla. Esto es:
	* first nibble, ms bit: set if final last keydef is still held
	* first nibble, ls 3 bits: count of keydefs to follow.
	valor 1
	//nibble, bits are 3210=lsca: lost keys (last set only), shift, ctrl and alt.
	valor 0
	//byte, bits are 76543210=00colrow: column and row as keyrow table
// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down
// 5|     ]      z      .      c      b  Pound      m      '
// 4|     [   Caps      k      s      f      =      g      ;
// 3|     l      3      h      1      a      p      d      j
// 2|     9      w      i    Tab      r      -      y      o
// 1|     8      2      6      q      e      0      t      u
// 0| Shift   Ctrl    Alt      x      v      /      n      ,


	//F1 es columna 1, row 0
	valor es 7654=00co=0000=0
	valor es lrow=1000 =8
	col=001 row=000
	00 001 000

	*/


	int columna;
	int fila;


    int i;
	//Si tecla no pulsada
	//if ((puerto_49150&1)) {
	if (!ql_pulsado_tecla()) {
		for (i=0;i<ql_ipc_last_nibble_to_read_length;i++) ql_ipc_last_nibble_to_read[i]=0;
	}


    ql_return_columna_fila_puertos(&columna,&fila);
    int tecla_shift=0;
    int tecla_control=0;
    int tecla_alt=0;


    if ( (columna>=0 && fila>=0) || ql_pressed_backspace) {

        if (ql_mantenido_pulsada_tecla==0 || (ql_mantenido_pulsada_tecla==1 && ql_mantenido_pulsada_tecla_timer>=50) )  {
            if (ql_mantenido_pulsada_tecla==0) {
                ql_mantenido_pulsada_tecla=1;
                ql_mantenido_pulsada_tecla_timer=0;
            }


            if (ql_pressed_backspace) {
                //CTRL + flecha izquierda
                tecla_control=1;
                // 6|   Ret   Left     Up    Esc  Right      \  Space   Down
                fila=6;
                columna=1;

            }

            //printf ("------fila %d columna %d\n",fila,columna);
            unsigned char byte_tecla=((fila&7)<<3) | (columna&7);



            ql_ipc_last_nibble_to_read[2]=(byte_tecla>>4)&15;
            ql_ipc_last_nibble_to_read[3]=(byte_tecla&15);

            if ((ql_keyboard_table[7]&1)==0) tecla_shift=1;
            if ((ql_keyboard_table[7]&2)==0) tecla_control=1;
            if ((ql_keyboard_table[7]&4)==0) tecla_alt=1;


            ql_ipc_last_nibble_to_read[1]=0;  //lsca
            if (tecla_shift) ql_ipc_last_nibble_to_read[1] |=4;
            if (tecla_control) ql_ipc_last_nibble_to_read[1] |=2;
            if (tecla_alt) ql_ipc_last_nibble_to_read[1] |=1;


        }
        else {
            //debug_printf (VERBOSE_PARANOID,"Repeating key");
            ql_ipc_last_nibble_to_read[0]=ql_ipc_last_nibble_to_read[1]=ql_ipc_last_nibble_to_read[2]=ql_ipc_last_nibble_to_read[3]=ql_ipc_last_nibble_to_read[4]=ql_ipc_last_nibble_to_read[5]=0;
        }
    }


    else {
        //debug_printf (VERBOSE_PARANOID,"Unknown key");
        ql_mantenido_pulsada_tecla=0;
        ql_ipc_last_nibble_to_read[0]=ql_ipc_last_nibble_to_read[1]=ql_ipc_last_nibble_to_read[2]=ql_ipc_last_nibble_to_read[3]=ql_ipc_last_nibble_to_read[4]=0;
    }


    ql_ipc_last_nibble_to_read_mascara=8;
    ql_ipc_last_nibble_to_read_index=0;
    ql_ipc_last_nibble_to_read_length=5; //5;

        //printf ("Ultimo pc_intr: %d\n",temp_pcintr);

    for (i=0;i<ql_ipc_last_nibble_to_read_length;i++) {
        //debug_printf (VERBOSE_PARANOID,"Return IPC values:[%d] = %02XH",i,ql_ipc_last_nibble_to_read[i]);
    }

}





void ql_ipc_write_ipc_read_keyrow(int row)
{

    /*
    kbdr_cmd equ    9       keyboard direct read
    * kbdr_cmd requires one nibble which selects the row to be read.
    * The top bit of this is ignored (at least on standard IPC's...).
    * It responds with a byte whose bits indicate which of the up to eight keys on
    * the specified row of the keyrow table are held down. */

	unsigned char resultado_row;

	resultado_row=ql_keyboard_table[row&7] ^ 255;
	//Bit a 1 para cada tecla pulsada
	//row numerando de 0 a 7
	/*
	// ================================== matrix ============================
	//        0      1      2      3      4      5      6      7
	//  +-------------------------------------------------------
	// |    F4     F1      5     F2     F3     F5      4      7
	// |   Ret   Left     Up    Esc  Right      \  Space   Down
	// |     ]      z      .      c      b  Pound      m      '
	// |     [   Caps      k      s      f      =      g      ;
	// |     l      3      h      1      a      p      d      j
	// |     9      w      i    Tab      r      -      y      o
	// |     8      2      6      q      e      0      t      u
	// | Shift   Ctrl    Alt      x      v      /      n      ,
	Por ejemplo, para leer si se pulsa Space, tenemos que leer row 1, y ver luego si bit 6 está a 1 (40H)
	*/

    //Si menu abierto, no tecla pulsada
	if (zxvision_key_not_sent_emulated_mach() )  resultado_row=0;

	debug_printf (VERBOSE_PARANOID,"i8049: Reading ipc command 9: read keyrow. row %d returning %02XH",row,resultado_row);

    ql_ipc_last_nibble_to_read[0]=(resultado_row>>4)&15;
    ql_ipc_last_nibble_to_read[1]=resultado_row&15;
    ql_ipc_last_nibble_to_read_mascara=8;
    ql_ipc_last_nibble_to_read_index=0;
    ql_ipc_last_nibble_to_read_length=2;


}



void ql_ipc_reset(void)
{
	ql_ipc_last_write_value=0;
	ql_ipc_last_write_bits_enviados=0;
	ql_estado_ipc=QL_STATUS_IPC_IDLE;
	ql_ipc_last_command=0;
	ql_ipc_bytes_received=0;
	ql_ipc_last_nibble_to_read_mascara=8;
	ql_ipc_last_nibble_to_read_index=0;
	ql_ipc_last_nibble_to_read_length=1;
	ql_ipc_reading_bit_ready=0;
}




//unsigned char temp_read_ipc;
unsigned char ql_read_ipc(void)
{



	unsigned char valor_retorno=0;


	//Ir alternando valor retornado
	if (ql_ipc_reading_bit_ready==0) {
		ql_ipc_reading_bit_ready=1;
		return 0;
	}



	if (ql_ipc_last_nibble_to_read[ql_ipc_last_nibble_to_read_index]&ql_ipc_last_nibble_to_read_mascara) valor_retorno |=128; //Valor viene en bit 7


	//Solo mostrar este debug si hay tecla pulsada

	//if (ql_pulsado_tecla() )printf ("Returning ipc: %XH. Index nibble: %d mask: %d\n",valor_retorno,ql_ipc_last_nibble_to_read_index,ql_ipc_last_nibble_to_read_mascara);

	if (ql_ipc_last_nibble_to_read_mascara!=1) ql_ipc_last_nibble_to_read_mascara=ql_ipc_last_nibble_to_read_mascara>>1;
	else {

		//Siguiente byte
		ql_ipc_last_nibble_to_read_mascara=8;
		ql_ipc_last_nibble_to_read_index++;
		if (ql_ipc_last_nibble_to_read_index>=ql_ipc_last_nibble_to_read_length) ql_ipc_last_nibble_to_read_index=0; //Si llega al final, dar la vuelta
		//if (ql_ipc_last_nibble_to_read_index>=ql_ipc_last_nibble_to_read_length) ql_ipc_last_nibble_to_read_index=ql_ipc_last_nibble_to_read_length; //dejarlo al final
	}


	return valor_retorno;



	/*
	* Receiving data from the IPC is done by writing %1110 to pc_ipcwr for each bit
* of the data, once again waiting for bit 6 at pc_ipcrd to go to zero, and
* then reading bit 7 there as the data bit. The data is received msb first.
*/


}



int ql_pulsado_tecla(void)
{

	if (zxvision_key_not_sent_emulated_mach() ) return 0;

	//Si backspace
	if (ql_pressed_backspace) return 1;

	unsigned char acumulado;

	acumulado=255;

	int i;
	for (i=0;i<8;i++) acumulado &=ql_keyboard_table[i];

	if (acumulado==255) return 0;
	return 1;

}




/*
// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down
// 5|     ]      z      .      c      b  Pound      m      '
// 4|     [   Caps      k      s      f      =      g      ;
// 3|     l      3      h      1      a      p      d      j
// 2|     9      w      i    Tab      r      -      y      o
// 1|     8      2      6      q      e      0      t      u
// 0| Shift   Ctrl    Alt      x      v      /      n      ,
*/

struct x_tabla_columna_fila
{
        int columna;
        int fila;
};

struct x_tabla_columna_fila ql_col_fil_numeros[]={
	{5,1}, //0
	{3,3},
	{1,1}, //2
	{1,3},
	{6,7},  //4
	{2,7},
	{2,1},  //6
	{7,7},
	{0,1}, //8
	{0,2}
};


/*
// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 7|    F4     F1      5     F2     F3     F5      4      7
// 6|   Ret   Left     Up    Esc  Right      \  Space   Down
// 5|     ]      z      .      c      b  Pound      m      '
// 4|     [   Caps      k      s      f      =      g      ;
// 3|     l      3      h      1      a      p      d      j
// 2|     9      w      i    Tab      r      -      y      o
// 1|     8      2      6      q      e      0      t      u
// 0| Shift   Ctrl    Alt      x      v      /      n      ,
*/
struct x_tabla_columna_fila ql_col_fil_letras[]={
	{4,3}, //A
	{4,5},
	{3,5},
	{6,3},
	{4,1}, //E
	{4,4},
	{6,4},
	{2,3}, //H
	{2,2},
	{7,3},
	{2,4}, //K
	{0,3},
	{6,5},
	{6,0}, //N
	{7,2},
	{5,3},
	{3,1}, //Q
	{4,2},
	{3,4},
	{6,1}, //T
	{7,1},
	{4,0},
	{1,2}, //W
	{3,0},
	{6,2},
	{1,5} //Z
};


//Returna fila y columna para una tecla dada.
/*
void ql_return_fila_columna_tecla(int tecla,int *columna,int *fila)
{
	int c;
	int f;
	int indice;

	//Por defecto
	c=-1;
	f=-1;

	if (tecla>='0' && tecla<='9') {
		indice=tecla-'0';
		c=ql_col_fil_numeros[indice].columna;
		f=ql_col_fil_numeros[indice].fila;
	}

	else if (tecla>='a' && tecla<='z') {
		indice=tecla-'a';
		c=ql_col_fil_letras[indice].columna;
		f=ql_col_fil_letras[indice].fila;
	}

	else if (tecla==32) {
		c=6;
		f=6;
	}

	else if (tecla==10) {
		c=0;
		f=6;
	}

	else if (tecla==QL_KEYCODE_F1) {
		c=1;
		f=7;
	}

	else if (tecla==QL_KEYCODE_F2) {
		c=3;
		f=7;
	}

	else if (tecla=='.') {
		c=2;
		f=5;
	}

	else if (tecla==',') {
		c=7;
		f=0;
	}

	//        0      1      2      3      4      5      6      7
	//  +-------------------------------------------------------
	// 6|   Ret   Left     Up    Esc  Right      \  Space   Down

	else if (tecla==QL_KEYCODE_UP) {
		c=2;
		f=6;
	}

	else if (tecla==QL_KEYCODE_DOWN) {
		c=7;
		f=6;
	}


	else if (tecla==QL_KEYCODE_LEFT) {
		c=1;
		f=6;
	}


	else if (tecla==QL_KEYCODE_RIGHT) {
		c=4;
		f=6;
	}


	*columna=c;
	*fila=f;
}
*/


//Retorna caracter ascii segun tecla pulsada en ql_keyboard_table
//Miramos segun tabla de tecla a puertos (ql_tabla_teclado_letras)
//NO USADO YA esto
/*
int ql_return_ascii_key_pressed(void)
{
	//temp
	//if ((ql_keyboard_table[4]&1)==0) return 'l';
	//if ((ql_keyboard_table[4]&16)==0) return 'a';

	int letra;
	int indice=0;

	for (letra='a';letra<='z';letra++) {
		if ((*ql_tabla_teclado_letras[indice].puerto & ql_tabla_teclado_letras[indice].mascara)==0) {
			//printf ("letra: %c\n",letra);
			return letra;
		}

		indice++;
	}

	indice=0;
	for (letra='0';letra<='9';letra++) {
		if ((*ql_tabla_teclado_numeros[indice].puerto & ql_tabla_teclado_numeros[indice].mascara)==0) {
			//printf ("numero: %c\n",letra);
			return letra;
		}

		indice++;
	}

	//Otras teclas
	//Enter
	if ((ql_keyboard_table[1]&1)==0) return 10;

	//Punto
	if ((ql_keyboard_table[2]&4)==0) return '.';

	//Coma
	if ((ql_keyboard_table[7]&128)==0) return ',';

	//Espacio
	if ((ql_keyboard_table[1]&64)==0) return 32;


	//F1
	if ((ql_keyboard_table[0]&2)==0) return QL_KEYCODE_F1;

	//F2
	if ((ql_keyboard_table[0]&8)==0) return QL_KEYCODE_F2;

// 1|   Ret   Left     Up    Esc  Right      \  Space   Down
	if ((ql_keyboard_table[1]&4)==0) return QL_KEYCODE_UP;

	if ((ql_keyboard_table[1]&128)==0) return QL_KEYCODE_DOWN;

	if ((ql_keyboard_table[1]&2)==0) return QL_KEYCODE_LEFT;

	if ((ql_keyboard_table[1]&16)==0) return QL_KEYCODE_RIGHT;


	return 0;
}

*/



void ql_stop_sound(void)
{

    ql_audio_playing=0;
}

moto_int ql_get_counter_from_pitch(moto_byte pitch)
{

    //Le suma fuziness
    /*
    int aux_pitch=pitch;

    aux_pitch +=ql_audio_fuziness;

    //Si no se sale del rango de 8 bits
    if (aux_pitch<256) pitch = aux_pitch;
    */

    //Es una tabla de 256 elementos. Dado que ql_audio_pitch1 es variable de 8 bits, no hay peligro de salirnos de la tabla
    int frecuencia=ql_pitch_frequency_table[pitch];

    //printf("frecuencia: %d\n",frecuencia);



    //Para 7800 hz (15600/2), contador valdra 1
    //Para 3900 Hz, contador valdra 2

    if (frecuencia==0) {
        //Esto no deberia pasar, pero por si acaso para evitar division por 0
        return (FRECUENCIA_CONSTANTE_NORMAL_SONIDO/2);
    }
    //no estoy seguro que fuziness debe aplicar aqui
    else {
        return ql_audio_fuziness+(FRECUENCIA_CONSTANTE_NORMAL_SONIDO/2)/frecuencia;
    }
}

int ql_audio_switch_pitch_current_index=0;

//0=high pitch, 1=low pitch
moto_byte ql_audio_switch_pitch_array[2];

//Current pitch
moto_byte ql_audio_switch_pitch_current_pitch;

//Veces llamadas a ql_audio_next_cycle
int ql_audio_next_cycle_counter=0;

//incremento entre notas
int signed_ql_audio_grad_y;

//Conteo de cuantas veces ha hecho "wrap"
int ql_audio_wrap_counter=0;


//Settings para activar/desactivar features de sonido desde menu
int ql_sound_feature_pitch2_enabled=1;
int ql_sound_feature_grad_x_enabled=1;
int ql_sound_feature_grad_y_enabled=1;
int ql_sound_feature_wrap_enabled=1;
int ql_sound_feature_fuzzy_enabled=1;
int ql_sound_feature_random_enabled=1;


void ql_adjust_audio_settings_with_mixer(void)
{
    if (!ql_sound_feature_pitch2_enabled) ql_audio_pitch2=0;
    if (!ql_sound_feature_grad_x_enabled) ql_audio_grad_x=0;
    if (!ql_sound_feature_grad_y_enabled) ql_audio_grad_y=0;

    if (!ql_sound_feature_wrap_enabled) ql_audio_wrap=0;
    if (!ql_sound_feature_fuzzy_enabled) ql_audio_fuziness=0;
    if (!ql_sound_feature_random_enabled) ql_audio_randomness_of_step=0;

}




//Inicialización de los procesos de cambio entre dos pitches
void ql_audio_switch_pitches_init(void)
{
    //Si pitch2, o grad_x, o grad_y es 0, no hacer cambios
    if (!ql_audio_pitch2 || !ql_audio_grad_x || !ql_audio_grad_y) {

        ql_audio_pitch_counter_initial=ql_get_counter_from_pitch(ql_audio_pitch1);


    //printf("contador: %d\n",ql_audio_pitch_counter_initial);

        ql_audio_pitch_counter_current=ql_audio_pitch_counter_initial;

        return;
    }


    //    //grad_y
    //ql_audio_grad_y
    //-8,7: range is -8 to 7 where step 1 to 7 scales downwards high to low pitch and -8 to 0 starts the sequence

    //Ver en cual de los dos pitch empezamos
     moto_byte lower_pitch,higher_pitch;

    if (ql_audio_pitch1>ql_audio_pitch2) {
        higher_pitch=ql_audio_pitch1;
        lower_pitch=ql_audio_pitch2;
    }
    else {
        higher_pitch=ql_audio_pitch2;
        lower_pitch=ql_audio_pitch1;
    }

    //convertir grad_y a algo con signo

    if (ql_audio_grad_y<=7) {
        signed_ql_audio_grad_y=ql_audio_grad_y;
    }
    else {
        //-1=15
        //-2=14
        //....
        //-6=10
        //-7=9
        //-8=8
       signed_ql_audio_grad_y=-16+ql_audio_grad_y;
    }

    //printf("i8049: higher pitch: %d lower pitch: %d signed_grad_y: %d\n",higher_pitch,lower_pitch,signed_ql_audio_grad_y);



    ql_audio_switch_pitch_array[0]=higher_pitch;
    ql_audio_switch_pitch_array[1]=lower_pitch;



    //Incremento positivo, bajar de high to low
    if (signed_ql_audio_grad_y>=0) ql_audio_switch_pitch_current_index=0; //empieza en high
    else ql_audio_switch_pitch_current_index=1; //empieza en low

    //Realmente si bajamos, seria signo negativo. Si subimos, es positivo. por tanto invertir lo que nos ha llegado
    signed_ql_audio_grad_y =-signed_ql_audio_grad_y;



    ql_audio_switch_pitch_current_pitch=ql_audio_switch_pitch_array[ql_audio_switch_pitch_current_index];

    //printf("final_pitch: %d\n",ql_audio_switch_pitch_current_pitch);


    ql_audio_pitch_counter_initial=ql_get_counter_from_pitch(ql_audio_switch_pitch_current_pitch);


    ql_audio_pitch_counter_current=ql_audio_pitch_counter_initial;


}

moto_int ql_get_audio_interval_steps_random(void)
{
        //Random just randomises the steps
        //Retornar el steps aplicando random
    //Si random 0, nada
    if (ql_audio_randomness_of_step==0) return ql_audio_grad_x;


    //Valor random entre 1 y 15, y ver que total no excede 32768
    if (ql_audio_grad_x>32000) return ql_audio_grad_x;


  ay_randomize(0);

  //valor_random es valor de 16 bits
  int valor_random=randomize_noise[0];


    int step_add_random=valor_random % ql_audio_randomness_of_step;

    //printf("Adding random %d to step (max %d value random: %d)\n",step_add_random,ql_audio_randomness_of_step,valor_random);

    return ql_audio_grad_x+step_add_random;


}

//Modifica el pitch, si conviene, cuando pitch2 no es 0
void ql_audio_switch_pitches(void)
{
    /*
    pitch       0,255: pitch 1 is high, 255 is low
    pitch2      0,255: a second pitch level between which the sound will "bounce"
    grad_x      -32768,32767: time interval between pitch steps
    grad_y      -8,7: size of each step. grad_x and grad_y control the rate at which the pitch bounces between levels
    wrap        0,15: will force the sound to wrap around the specified number of times. if wrap is equal to 15 the sound
                will grap around forever
    fuzzy       0,15: defined the amount of fuzziness to be added to the sound
    random      0,15: defined the amount of randomness to be added to the sound
    */

   /*
   Harmonic
    The second pitch (pitch_2), I will refer to as the Harmonic, add a Time Interval (grad_x) and a Pitch Step (grad_y),
    they create a sequence of sound variations ordered by the time duration and number of steps between the main pitch and second pitch.

    The Time Interval (grad_x) again is in multiple units of 72 microseconds for each note in the sequence.

    The Pitch Step (grad_y) range is -8 to 7 where step 1 to 7 scales downwards high to low pitch and -8 to 0 starts the sequence
    scaling upwards from low to a higher pitch. From then on the sequence bounces between the two pitches.

    A Harmonic without a Time Interval (grad_x) and/or Pitch Step (grad_y) has no affect. Adding a Pitch step of 1 when Harmonic and Time Interval are both 0
    identifies the pitch as a high zero. Harmonic plus a Pitch step with Time Interval 0 just changes Main Pitch to the Harmonic.

    Wraps
    Wraps repeat the sequence of harmonics produced by the pitch_1, pitch_2, grad_x, grad_y parameters a number of times.
    Zero continues the bounce affect of the harmonic. Increasing values 1 to 7 creates scaling high to low for the number of Wraps.
    Scaling 8 to 15 creates Wraps from low to high.


    Fuzzy & Random
    Fuzzy decreases the purity of the pitch, Random just randomises the steps until little of the original sequence is evident.
    Both of these have a range 0 to 15, zero has no effect and the active range is more like 8 to 15.
    Increasing the fuzzy range as said before blurs the pitch to a buzz.

    */

   //Comando beep:
   //beep duration, pitch, pitch2, grad_x, grad_y, wrap, fuzzy, random

   //TODO: que significa grad_x negativo???
   // grad_x is in multiple units of 72 microseconds for each note. Dado que ql_audio_switch_pitches puede ejecutarse mas tarde
   //que 72 microsegundos, hay que tener un contador para esto en ql_audio_next_cycle

   //: segun grad_y negativo o positivo, hay que hacer al inicio del sonido que se empiece en uno u otro pitch

   //Reaplicar cambios en el mixer
   ql_adjust_audio_settings_with_mixer();

   //Si pitch2, o grad_x, o grad_y es 0, no hacer cambios
   if (!ql_audio_pitch2 || !ql_audio_grad_x || !ql_audio_grad_y) return;

    //Ver si hay que cambiar la nota en curso

    //Random just randomises the steps

    if (ql_audio_next_cycle_counter>=ql_get_audio_interval_steps_random() ) {
        //ql_audio_next_cycle_counter -=ql_audio_grad_x; //restamos en vez de poner a 0 para que sea tiempo acumulativo

        ql_audio_next_cycle_counter =0;
        debug_printf(VERBOSE_PARANOID,"i8049: Next note in step");

        //cambiar nota
        //tenemos ql_audio_switch_pitch_current_pitch nota actual
        //    ql_audio_switch_pitch_array[0]=higher_pitch;
        // ql_audio_switch_pitch_array[1]=lower_pitch;
        //Y incremento en signed_ql_audio_grad_y


        ql_audio_switch_pitch_current_pitch += signed_ql_audio_grad_y;

        //Ver si subimos o bajamos
        if (signed_ql_audio_grad_y>=0) {
            //Ver si nos pasamos
            if (ql_audio_switch_pitch_current_pitch>=ql_audio_switch_pitch_array[0]) {
                //Sobrepasado limite.
                //printf("Reached upper limit.\n");
                //printf("wrap counter: %d\n",ql_audio_wrap_counter);

                //No tengo claro que la funcion de wrap sea esta

                ql_audio_wrap_counter++;
                if (ql_audio_wrap_counter>=ql_audio_wrap && ql_audio_wrap!=15) {
                    //printf("reached maximum wraps. do not change anymore\n");
                    ql_audio_pitch2=ql_audio_grad_x=ql_audio_grad_y=0;
                }
                else {
                    ql_audio_switch_pitch_current_pitch=ql_audio_switch_pitch_array[1];
                }
            }
        }
        else {
            //Ver si nos pasamos por debajo
            if (ql_audio_switch_pitch_current_pitch<=ql_audio_switch_pitch_array[1]) {
                //Sobrepasado limite. TODO
                //printf("Reached lower limit.\n");
                //printf("wrap counter: %d\n",ql_audio_wrap_counter);

                //No tengo claro que la funcion de wrap sea esta

                ql_audio_wrap_counter++;
                if (ql_audio_wrap_counter>=ql_audio_wrap && ql_audio_wrap!=15) {
                    //printf("reached maximum wraps. do not change anymore\n");
                    ql_audio_pitch2=ql_audio_grad_x=ql_audio_grad_y=0;
                }
                else {
                    ql_audio_switch_pitch_current_pitch=ql_audio_switch_pitch_array[0];
                }
            }
        }

        //printf("current pitch: %d\n",ql_audio_switch_pitch_current_pitch);

        ql_audio_pitch_counter_initial=ql_get_counter_from_pitch(ql_audio_switch_pitch_current_pitch);


        ql_audio_pitch_counter_current=ql_audio_pitch_counter_initial;
    }

}


void ql_audio_next_cycle(void)
{

    if (!ql_audio_playing) return;

    if (!i8049_chip_present) return;

    ql_audio_next_cycle_counter++;

    //Contador para el pitch, para conmutar valor altavoz
    ql_audio_pitch_counter_current--;
    if (ql_audio_pitch_counter_current==0) {
        ql_audio_pitch_counter_current=ql_audio_pitch_counter_initial;
        ql_audio_output_bit ^=1;

        ql_audio_switch_pitches();
    }


    //Contador de la duracion del sonido
    //Si es 0, dejarlo tal cual
    if (ql_current_sound_duration!=0) {

        ql_current_sound_duration--;

        if (ql_current_sound_duration==0) {
            //Silenciar
            //printf("stop sound\n");
            ql_stop_sound();
        }
    }
}


char ql_audio_da_output(void)
{

    if (!ql_audio_playing) return 0;

    if (!i8049_chip_present) return 0;

    silence_detection_counter=0;

    const int amplitud_onda=50;

    if (ql_audio_output_bit) return amplitud_onda;
    else return -amplitud_onda;

}


//void ql_simulate_sound(unsigned char pitch1,moto_int duration)
//{
    			//de momento solo tonos
                /*
			unsigned char valor_mixer=255;


			valor_mixer &=(255-1); //Canal 0 tono

			//printf ("set mixer chip ay 0. tonos: %02XH ruidos: %02XH final: %02XH\n",mixer_tonos,mixer_ruido,valor_mixer);

			ay_chip_selected=0;
			out_port_ay(65533,7);
			out_port_ay(49149,valor_mixer);

            //volumen

            out_port_ay(65533,8);
			out_port_ay(49149,15);

            //Tono
            //RO � Ajuste fino del tono, canal A
            //R1 � Ajuste aproximado del tono, canal A- (4 bits)
            //En AY: valor mas alto en el chip, frecuencia mas alta
            //igual que ql: pitch       0,255: pitch 1 is high, 255 is low
            */

           /*
    unsigned char frecuencia=pitch1;

	out_port_ay(65533,0);
	out_port_ay(49149,(frecuencia << 4) & 0xF0); //Aqui los 4 bits bajos

	out_port_ay(65533,1);
	out_port_ay(49149,(frecuencia>>4) & 0xF );  //Y aqui los 4 bits altos
*/


//}



//8 bloques de 4 bits. MSB first
unsigned char ql_ipc_sound_command_buffer[8*2];


void ql_ipc_set_sound_parameters(void)
{

    /*
    BEEP syntax:

    duration    -32768,32768: duration in units of 72 microseconds. duration of 0 will run the sound until terminated by another beep command
    pitch       0,255: pitch 1 is high, 255 is low
    pitch2      0,255: a second pitch level between which the sound will "bounce"
    grad_x      -32768,32767: time interval between pitch steps
    grad_y      -8,7: size of each step. grad_x and grad_y control the rate at which the pitch bounces between levels
    wrap        0,15: will force the sound to wrap around the specified number of times. if wrap is equal to 15 the sound will grap around forever
    fuzzy       0,15: defined the amount of fuzziness to be added to the sound
    random      0,15: defined the amount of randomness to be added to the sound

    Formato del mensaje ipc:

    8 bits pitch 1
    8 bits pitch 2
    16 bits  interval between steps (grad_x)
    16 bits duration
    4 bits step in pitch (grad_y)
    4 bits wrap
    4 bits randomness of step
    4 bits fuzziness

    no reply

    Para aproximar, cada "duration" es un scanline


    */


    ql_audio_pitch1=(ql_ipc_sound_command_buffer[0]<<4)|ql_ipc_sound_command_buffer[1];
    ql_audio_pitch2=(ql_ipc_sound_command_buffer[2]<<4)|ql_ipc_sound_command_buffer[3];

    //OJO a como se ordena esto:
    //grad_x
    ql_audio_grad_x=(ql_ipc_sound_command_buffer[6]<<12)|(ql_ipc_sound_command_buffer[7]<<8)|
                    (ql_ipc_sound_command_buffer[4]<<4)|ql_ipc_sound_command_buffer[5];


    //OJO a como se ordena esto:
    ql_audio_duration=(ql_ipc_sound_command_buffer[10]<<12)|(ql_ipc_sound_command_buffer[11]<<8)|
            (ql_ipc_sound_command_buffer[8]<<4)|ql_ipc_sound_command_buffer[9];

    //grad_y
    ql_audio_grad_y=ql_ipc_sound_command_buffer[12];

    ql_audio_wrap=ql_ipc_sound_command_buffer[13];
    ql_audio_randomness_of_step=ql_ipc_sound_command_buffer[14];
    ql_audio_fuziness=ql_ipc_sound_command_buffer[15];


    ql_adjust_audio_settings_with_mixer();


    debug_printf(VERBOSE_PARANOID,"i8049: setting sound: pitch1 %d pitch2 %d interval_steps %d duration %d step_in_pitch %d wrap %d randomness_of_step %d fuziness %d",
        ql_audio_pitch1,ql_audio_pitch2,ql_audio_grad_x,ql_audio_duration,
        ql_audio_grad_y,ql_audio_wrap,ql_audio_randomness_of_step,ql_audio_fuziness);

    //ql_simulate_sound(ql_audio_pitch1,ql_audio_duration);


    ql_current_sound_duration=ql_audio_duration;

    //ql_audio_pitch_counter_initial=ql_audio_pitch1;

    //Calculamos ql_audio_pitch_counter_initial
    //int frecuencia=get_note_frequency_from_ql_pitch(ql_audio_pitch1);



    ql_audio_switch_pitches_init();

    ql_audio_next_cycle_counter=0;
    ql_audio_wrap_counter=0;

    ql_audio_playing=1;


}

int ql_ipc_get_frecuency_sound_value(int pitch)
{
    //ql_audio_pitch

    //Cada scanline, se decrementa el contador. Por tanto, son 15600 hz de base

    //No deberia ser 0 nunca, pero por si acaso
    if (pitch==0) return FRECUENCIA_SONIDO;

    int frecuencia=FRECUENCIA_SONIDO/pitch/2;

    return frecuencia;
}


//Retorna la frecuencia del sonido que se esta produciendo ahora
//NO es necesariamente pitch1 o pitch2, porque si pitch2 es diferente de 0, el sonido oscilara entre pitch1 y pitch2
int ql_ipc_get_frecuency_sound_current_pitch(void)
{

    return ql_ipc_get_frecuency_sound_value(ql_audio_pitch_counter_initial);
}


void ql_write_ipc(unsigned char Data)
{

    ql_ipc_reading_bit_ready=0;

	/*
	* Commands and data are sent msb first, by writing a byte containg %11x0 to
	* location pc_ipcwr ($18023), where the "x" is one data bit. Bit 6 at location
	* pc_ipcrd ($18020) is then examined, waiting for it to go zero to indicate
	* that the bit has been received by the IPC.
	*/
	/*
	* Receiving data from the IPC is done by writing %1110 to pc_ipcwr for each bit
* of the data, once again waiting for bit 6 at pc_ipcrd to go to zero, and
* then reading bit 7 there as the data bit. The data is received msb first.
	*/

	//Si dato tiene formato: 11x0  (8+4+1)
	if ((Data&13)!=12) return;

	int bitdato=(Data>>1)&1;
	//printf ("Escribiendo bit ipc: %d. bits enviados: %d\n",bitdato,ql_ipc_last_write_bits_enviados);
	ql_ipc_last_write_value=ql_ipc_last_write_value<<1;
	ql_ipc_last_write_value |=bitdato;
	ql_ipc_last_write_bits_enviados++;

	if (ql_ipc_last_write_bits_enviados==4) {

			switch (ql_estado_ipc) {
			  case QL_STATUS_IPC_IDLE:
					ql_ipc_last_command=ql_ipc_last_write_value&15;
					//printf ("Resultante ipc command: %d (%XH)\n",ql_ipc_last_command,ql_ipc_last_command); //Se generan 4 bits cada vez
					ql_ipc_last_write_bits_enviados=0;

					//Actuar segun comando
					switch (ql_ipc_last_command)
					{


						case 0:
						//*rset_cmd equ    0       resets the IPC software
							ql_ipc_reset();
						break;

						case 1:
/*
stat_cmd equ    1       report input status


* returns a byte, the bits of which are:
ipc..kb equ     0       set if data available in keyboard buffer, or key held
ipc..so equ     1       set if sound is still being generated
*               2       set if kbd shift setting has changed, with key held
*               3       set if key held down
ipc..s1 equ     4       set if input is pending from RS232 channel 1
ipc..s2 equ     5       set if input is pending from RS232 channel 2
ipc..wp equ     6       return state of p26, currently not connected
*               7       was set if serial transfer was being zapped, now zero
*/

							ql_ipc_last_nibble_to_read[0]=0; //ipc..kb equ     0       set if data available in keyboard buffer, or key held


							//Decir tecla pulsada

							ql_ipc_last_nibble_to_read[0]=15; //Devolver valor entre 8 y 15 implica que acabara leyendo el teclado

                            ql_ipc_last_nibble_to_read_mascara=8;
                            ql_ipc_last_nibble_to_read_index=0;
                            ql_ipc_last_nibble_to_read_length=1;


							//Si tecla no pulsada
							if (!ql_pulsado_tecla()) ql_ipc_last_nibble_to_read[0]=4;


                            //Nuevo metodo
                            ql_ipc_last_nibble_to_read[0]=0;

                            /*
                            ipc..kb equ     0       set if data available in keyboard buffer, or key held
                                            3       set if key held down
                            */
                            if (ql_pulsado_tecla()) {
                                ql_ipc_last_nibble_to_read[0] |=1;

                                //Duda de la diferencia entre estos dos bits
                                ql_ipc_last_nibble_to_read[0] |=8;
                            }

                            //tecla shift
                            // bit              2       set if kbd shift setting has changed, with key held
                            if ((ql_keyboard_table[7]&1)==0) {
                                //printf("pressed shift\n");
                                ql_ipc_last_nibble_to_read[0] |=4;
                            }

                            //Fin Nuevo metodo

                            //Bit de beeping
                            if (ql_audio_playing) ql_ipc_last_nibble_to_read[0] |=2;
                            else ql_ipc_last_nibble_to_read[0] &=(255-2);


                            //printf("ipc command stat_cmd\n");

						break;

						case 8:

							//debug_printf (VERBOSE_DEBUG,"Write ipc command 8: Read key. PC=%08XH",get_pc_register() );

							ql_ipc_write_ipc_teclado();


						break;

						case 9:
							//debug_printf (VERBOSE_ERR,"Write ipc command 9: Reading keyrow. Not implemented");
							/*
							kbdr_cmd equ    9       keyboard direct read
* kbdr_cmd requires one nibble which selects the row to be read.
* The top bit of this is ignored (at least on standard IPC's...).
* It responds with a byte whose bits indicate which of the up to eight keys on
* the specified row of the keyrow table are held down. */

							ql_estado_ipc=QL_STATUS_IPC_WRITING;


						break;

						case 10:
						/*
						inso_cmd equ    10      initiate sound process
* This requires no less than 64 bits of data. it starts sound generation.
* Note that the 16 bit values below need to have their ls 8 bits sent first!
* 8 bits pitch 1
* 8 bits pitch 2
* 16 bits interval between steps
* 16 bits duration (0=forever)
* 4 bits signed step in pitch
* 4 bits wrap
* 4 bits randomness (none unless msb is set)
* 4 bits fuziness (none unless msb is set)
*/
							debug_printf (VERBOSE_PARANOID,"i8049: ipc command 10 inso_cmd initiate sound process");
                            //printf ("ipc command 10 inso_cmd initiate sound process\n");
							//sleep(5);

                            ql_estado_ipc=QL_STATUS_IPC_WRITING; //Mejor lo desactivo porque si no se queda en estado writing y no sale de ahi
						break;

                        case 11:
                            //kiso_cmd equ    11      immediately stop sound generation
                            ql_stop_sound();
                        break;

						//baud_cmd
						case 13:
						/*
						baud_cmd equ    13      change baud rate
* This expects one nibble, of which the 3 lsbs select the baud rate for both
* serial channels. The msb is ignored. Values 7 down to zero correspond to baud
* rates 75/300/600/1200/2400/4800/9600/19200.
* The actual clock rate is supplied from the PC to the IPC, but this command is
* also needed in the IPC for timing out transfers!
						*/
						    ql_estado_ipc=QL_STATUS_IPC_WRITING;
						break;

                        case 14:
                        //La rom no parece hacer uso de este comando, ni con rnd ni con randomise. Sera otro bug de la rom?
                        //Para randomise utiliza el RTC
                        //rand_cmd equ    14      random number generator, returns a sixteen bit number.
                            //printf("i8049: IPC write. status idle. command rand_cmd\n");

                        break;


						case 15:
							//El que se acaba enviando para leer del puerto ipc. No hacer nada
						break;

                        default:
                            debug_printf(VERBOSE_DEBUG,"i8049: IPC write. status idle. unhandled command: %d",ql_ipc_last_command);

                        break;

					}
				break;


			case QL_STATUS_IPC_WRITING:
				ql_ipc_last_command_parameter=ql_ipc_last_write_value&15;
				//printf ("Parametro recibido de ultimo comando %d: %d\n",ql_ipc_last_command,ql_ipc_last_command_parameter);
				//Segun ultimo comando

					switch (ql_ipc_last_command) {
						case 9:
						/*
						kbdr_cmd equ    9       keyboard direct read
* kbdr_cmd requires one nibble which selects the row to be read.
* The top bit of this is ignored (at least on standard IPC's...).
* It responds with a byte whose bits indicate which of the up to eight keys on
* the specified row of the keyrow table are held down. */
//ql_ipc_write_ipc_read_keyrow();
							//printf ("Parametro recibido de ultimo comando read keyrow %d: %d\n",ql_ipc_last_command,ql_ipc_last_command_parameter);
							ql_estado_ipc=QL_STATUS_IPC_IDLE;
							ql_ipc_last_write_bits_enviados=0;
							ql_ipc_write_ipc_read_keyrow(ql_ipc_last_command_parameter);


						break;


						case 10:
							debug_printf (VERBOSE_PARANOID,"i8049: parameter sound %d: %d",ql_ipc_bytes_received,ql_ipc_last_command_parameter);

                            //printf ("parameter sound %d: %d\n",ql_ipc_bytes_received,ql_ipc_last_command_parameter);
                            ql_ipc_sound_command_buffer[ql_ipc_bytes_received]=ql_ipc_last_command_parameter;

							ql_ipc_bytes_received++;
                            ql_ipc_last_write_bits_enviados=0;
                            //sleep(1);

                            //16 bytes (o sea, cada byte me lleva 4 bits efectivos, en total: 8 bytes efectivos)
							if (ql_ipc_bytes_received>=16) {
								debug_printf (VERBOSE_PARANOID,"i8049: End receiving ipc parameters");
								ql_estado_ipc=QL_STATUS_IPC_IDLE;
								//ql_ipc_last_write_bits_enviados=0;
								ql_ipc_bytes_received=0;
                                //sleep(10);

                                ql_ipc_set_sound_parameters();
							}
                            /*
                            BEEP syntax:
                            duration    -32768,32768: duration in units of 72 microseconds. duration of 0 will run the sound until terminated by another beep command
                            pitch       0,255: pitch 1 is high, 255 is low
                            pitch2      0,255: a second pitch level between which the sound will "bounce"
                            grad_x      -32768,32767: time interval between pitch steps
                            grad_y      -8,7: size of each step. grad_x and grad_y control the rate at which the pitch bounces between levels
                            wrap        0,15: will force the sound to wrap around the specified number of times. if wrap is equal to 15 the sound will grap around forever
                            fuzzy       0,15: defined the amount of fuzziness to be added to the sound
                            random      0,15: defined the amount of randomness to be added to the sound

                            Formato del mensaje ipc:
                            8 bits pitch 1
                            8 bits pitch2
                            16 bits  interval between steps
                            16 bits duration
                            4 bits step in pitch
                            4 bits wrap
                            4 bits randomness of step
                            4 bits fuzziness

                            no reply


                            */
						break;

						case 13:
							ql_estado_ipc=QL_STATUS_IPC_IDLE;
							//Fin de parametros de comando. Establecer baud rate y dejar status a idle de nuevo para que la siguiente escritura se interprete como comando inicial
						break;

                        case 14:
                        //rand_cmd equ    14      random number generator, returns a sixteen bit number.
                            printf("i8049: IPC write. status writing. command rand_cmd\n");

                        break;


                        default:
                            printf("i8049: IPC write. status writing. unhandled command: %d\n",ql_ipc_last_command);

                        break;

					}
			break;
			}

	}


}