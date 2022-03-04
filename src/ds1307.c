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
#include <dirent.h>
#include <time.h>
#include <sys/time.h>

#include "ds1307.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "menu.h"



z80_bit ds1307_last_clock_bit={0};
z80_bit ds1307_last_data_bit={0};


//numero de bit enviando a una operacion de lectura
//z80_byte ds13072_bitnumber_read=128;


z80_byte ds1307_registers[64];

z80_byte ds_1307_received_register_number;

//int temp_conta=0;

int ds1307_sending_data_from_speccy=1;

int ds1307_sending_data_status=0;
//0=Preparado para recibir comando
//1=Preparado para recibir registro

int ds1307_sending_data_num_bits; //Entre 0 y 7 para bits y 8 para ack
z80_byte ds1307_last_command_received;//Normalmente D0 o D1
z80_byte ds1307_last_register_received=0; //Normalmente entre 0 y 63

z80_byte ds1307_last_command_read_mask=128;


void ds1307_reset(void)
{
	ds1307_last_clock_bit.v=0;
	ds1307_last_data_bit.v=0;


	//numero de bit enviando a una operacion de lectura
	//ds13072_bitnumber_read=128;


	ds1307_sending_data_from_speccy=1;

	ds1307_sending_data_status=0;

	ds1307_last_command_read_mask=128;

	ds1307_last_register_received=0;
}


z80_byte ds1307_decimal_to_bcd(z80_byte valor_decimal)
{
	z80_byte nibble_bajo=(valor_decimal%10)&15;
	z80_byte nibble_alto=(valor_decimal/10)&15;

	z80_byte resultado=( (nibble_alto<<4) | nibble_bajo );

	//printf ("decimal a bcd. decimal: %d bcd: %02XH nibble_bajo %d nibble_alto %d\n",valor_decimal,resultado,nibble_bajo,nibble_alto);

	return resultado;
}

z80_byte ds1307_get_register(z80_byte index)
{
	index=index&63;

	if (index<8) {

/*
obtener fecha
*/

//fecha grabacion
time_t tiempo = time(NULL);
struct tm tm = *localtime(&tiempo);

//printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

/*header[75]=tm.tm_mday;
header[76]=tm.tm_mon+1;

z80_int year;
year=tm.tm_year + 1900;

header[77]=value_16_to_8l(year);
header[78]=value_16_to_8h(year);

header[79]=tm.tm_hour;
header[80]=tm.tm_min;*/

ds1307_registers[0]=ds1307_decimal_to_bcd(tm.tm_sec); // segundos
ds1307_registers[1]=ds1307_decimal_to_bcd(tm.tm_min); //56 minutos
ds1307_registers[2]=ds1307_decimal_to_bcd(tm.tm_hour); //09 horas

//dia semana, dia, mes, anyo
//Prueba 18/09/2017
ds1307_registers[3]=0x01; //temp dia de la semana 1
ds1307_registers[4]=ds1307_decimal_to_bcd(tm.tm_mday); //dia
ds1307_registers[5]=ds1307_decimal_to_bcd(tm.tm_mon+1); //mes
ds1307_registers[6]=ds1307_decimal_to_bcd(tm.tm_year+1900-2000); //año


							//temp. ds1307_registers.
							//Prueba 09:56:33
							/*ds1307_registers[0]=0x33; //33 segundos
							ds1307_registers[1]=0x56; //56 minutos
							ds1307_registers[2]=0x09; //09 horas

							//dia semana, dia, mes, anyo
							//Prueba 18/09/2017
							ds1307_registers[3]=0x03;
							ds1307_registers[4]=0x18;
							ds1307_registers[5]=0x09;
							ds1307_registers[6]=0x17;*/

	}

	//printf ("Getting RTC index %d reg_e: %d\n",index,reg_e);

	//Signatura para Next
	if (MACHINE_IS_TBBLUE) {
		if (index==62 || index==63) {
			//printf ("Setting RTC index 62 and 63 (index=%d)\n",index);
			ds1307_registers[62]='Z';
			ds1307_registers[63]='X';
		}
	}

	return ds1307_registers[index];


						//}
}


void tbblue_trap_return_rtc(void)
{
	

//fecha grabacion
time_t tiempo = time(NULL);
struct tm tm = *localtime(&tiempo);

int segundos=tm.tm_sec; // segundos
int minutos=tm.tm_min; //56 minutos
int horas=tm.tm_hour; //09 horas

//dia semana, dia, mes, anyo
//Prueba 18/09/2017
//int diasemana=0x01; //temp dia de la semana 1
int dia=tm.tm_mday; //dia
int mes=tm.tm_mon+1; //mes
int anyo=tm.tm_year+1900; //año

/*

; OUTPUT
; reg BC is Date 
;	year - 1980 (7 bits) + month (4 bits) + day (5 bits). EJ= BC=3200H -> 2005

;   

;	note that DS1307 only supports 2000-2099.
;
; reg DE is Time
;	hours (5 bits) + minutes (6 bits) + seconds/2 (5 bits) EJ DE=9000H -> 18:00:00
;
; Carry set if no valid signature in 0x3e and 0x3f i.e. letters 'ZX'  (HL=585A)
; this is used to detect no RTC or unset date / time.



Codigo de esxdos (RTC.SYS) que obtiene la fecha, usado en NextOS para ponerla en los menus
Ver rtcesx.asm (aunque el codigo que veo ahi no es el mismo desensamblado, pero sirve de referencia)

  279B PUSH BC
  279C CALL 27BD
  279F POP BC
  27A0 ADD A,14
  27A2 SLA A
  27A4 OR B
  27A5 LD B,A

  27A6 2A3C28 LD HL,(283C)
  27A9 C9     RET
  27AA 37     SCF
  27AB C9     RET


  27AC EI
  27AD RET
  27AE XOR A



*/



	HL=0x585a;
	BC= ((anyo-1980)<<9) | (mes<<5) | dia;
	DE= (horas<<11) | (minutos<<5) | (segundos/2);

	Z80_FLAGS=0; //dejamos todos los flags a 0. Interesa realmente solo el C
	//permitimos el trap entrando desde 27a9 y a7aa. Si es el segundo caso, quitariamos el flag C y todos felices

	//printf ("RTC trap\n");

}

z80_byte ds1307_get_port_clock(void)
{
	return 0;
}

z80_byte ds1307_get_port_data(void)
{

		z80_byte value=ds1307_get_register(ds1307_last_register_received) & ds1307_last_command_read_mask;
		if (value) value=1;
		else value=0;

		//if (ds1307_last_command_read_mask==128) printf ("Returning first bit of register %d value %02XH",ds1307_last_register_received&63,ds1307_registers[ds1307_last_register_received&63]);

		//printf ("Returning %d bit of register %d value %02XH\n",ds1307_last_command_read_mask,ds1307_last_register_received,value);

		ds1307_last_command_read_mask=ds1307_last_command_read_mask>>1;

		//Siguiente byte
		if (ds1307_last_command_read_mask==0) {
			ds1307_last_command_read_mask=128;
			ds1307_last_register_received++;
		}

	//printf ("%d Returning value %d register %d final_mask %d\n",temp_conta++,value,ds1307_last_register_received,ds1307_last_command_read_mask);
	return value;
}


void ds1307_write_port_data(z80_byte value)
{
	//printf ("%d Write ds1307 data port value:  %d bit 0: %d\n",temp_conta_ds++,value,value&1);


	if (ds1307_last_clock_bit.v) {
		if (ds1307_last_data_bit.v==1 && (value&1)==0) {
			debug_printf (VERBOSE_DEBUG,"DS1307 RTC. Received START sequence");




			ds1307_sending_data_from_speccy=1;
			ds1307_sending_data_status=0;
			ds1307_sending_data_num_bits=0;

			ds1307_last_command_read_mask=128;

		}

		if (ds1307_last_data_bit.v==0 && (value&1)==1) {
			debug_printf (VERBOSE_DEBUG,"DS1307 RTC. Received STOP sequence");

			ds1307_sending_data_from_speccy=1;
			ds1307_sending_data_status=0;
			ds1307_sending_data_num_bits=0;
			ds1307_last_command_read_mask=128;

		}

		//printf ("Setting ds1307_last_data_bit\n");
		ds1307_last_data_bit.v=value&1;
		return;
	}



	ds1307_last_data_bit.v=value&1;
	if (ds1307_sending_data_from_speccy) {

		//Recibiendo comando
		if (ds1307_sending_data_status==0) {
			//printf ("Receiving data command\n");
			ds1307_sending_data_num_bits++;
			if (ds1307_sending_data_num_bits<=8) {
				ds1307_last_command_received=ds1307_last_command_received<<1;
				ds1307_last_command_received &=(255-1);
				ds1307_last_command_received|=value&1;
			}

			//Es ack. ignorar y cambiar estado
			if (ds1307_sending_data_num_bits==9) {
				//printf ("Received ACK\n");
				//printf ("Command received: %02XH\n",ds1307_last_command_received);
				ds1307_sending_data_status++;

				//Si el comando es D0, envia datos. Si es D1, solo recibe y por tanto ignoramos escrituras en D
				if (ds1307_last_command_received==0xD0) ds1307_sending_data_from_speccy=1;
				else ds1307_sending_data_from_speccy=0;

				ds1307_sending_data_num_bits=0;
			}

		}

		//Recibiendo registro
		else if (ds1307_sending_data_status==1) {
			//printf ("Receiving data register. ds1307_sending_data_num_bits %d ds1307_last_register_received %d\n",ds1307_sending_data_num_bits,ds1307_last_register_received);
			ds1307_sending_data_num_bits++;
			if (ds1307_sending_data_num_bits<=8) {
				ds1307_last_register_received=ds1307_last_register_received<<1;
				ds1307_last_register_received &=(255-1);
				ds1307_last_register_received|=value&1;

				
			}

			//Es ack. ignorar y cambiar estado
			if (ds1307_sending_data_num_bits==9) {
				//printf ("Received ACK\n");
				//printf ("Register received: %02XH\n",ds1307_last_register_received);
				ds1307_sending_data_status++;
				ds1307_sending_data_num_bits=0;
			}

		}
	}

	//else {
		//printf ("Ignoring write on data\n");
	//}




}

void ds1307_write_port_clock(z80_byte value)
{
	//printf ("%d Write ds1307 clock port value: %d bit 0: %d\n",temp_conta++,value,value&1);
	ds1307_last_clock_bit.v=value&1;

}


/*

Ejemplo con time.asm . Secuencia bytes:

TIME

READ_TIME:
	;---------------------------------------------------
	; Talk to DS1307 and request the first reg
	call START_SEQUENCE


START_SEQUENCE:

C=1
D=1
D=0
C=0


	ld l,0xD0
	call SEND_DATA

Enviar 8 bits de D0, a cada bit, C=1, C=0
ACK: Enviar D=1. C=1, C=0
28 Write ds1307 data port value:  1 bit 0: 1
29 Write ds1307 clock port value: 1 bit 0: 1
30 Write ds1307 clock port value: 0 bit 0: 0





	ld l,0x00
	call SEND_DATA


Enviar 8 bits de 00, a cada bit, C=1, C=0
ACK: Enviar D=1. C=1, C=0
55 Write ds1307 data port value:  1 bit 0: 1
56 Write ds1307 clock port value: 1 bit 0: 1
57 Write ds1307 clock port value: 0 bit 0: 0



	call START_SEQUENCE

C=1
D=1
D=0
C=0



	ld l,0xD1
	call SEND_DATA

Enviar 8 bits de D1, a cada bit, C=1, C=0
ACK: Enviar D=1. C=1, C=0

86 Write ds1307 data port value:  1 bit 0: 1
87 Write ds1307 clock port value: 1 bit 0: 1
88 Write ds1307 clock port value: 0 bit 0: 0


Lectura 7 registros del RTC.
Para cada uno:

READ:
D=1 (free the data line). Al principio
bucle: C=1
Leer bit
C=0
ir a bucle:

Enviar ACK, siempre que no sea ultimo byte. D=0, C=1, C=0, D=1
114 Write ds1307 data port value:  0 bit 0: 0
115 Write ds1307 clock port value: 1 bit 0: 1
116 Write ds1307 clock port value: 0 bit 0: 0
117 Write ds1307 data port value:  1 bit 0: 1   (Este es para el primer byte, segundo etc excepto para el ultimo)
Enviar NACK  para el ultimo byte
D=1, C=1, C=0, D=1

Y asi hasta los 7 registros


		;STOP_SEQUENCE:

D=0
C=1
D=1

	CALL SDA0
	CALL SCL1
	CALL SDA1



*/
