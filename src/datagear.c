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

#include "datagear.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "screen.h"



//Si esta recibiendo parametros de comando.
//Si no es 0, indica cuantos parámetros le quedan por recibir
//int datagear_receiving_parameters=0;

//Mascara de los parametros a leer. Por ejemplo si WR0 enviara los 4 parametros, tendra valor 00001111b
z80_byte datagear_mask_commands=0;

//Indice al numero de parametro leido
int datagear_command_index;
//Ejemplo, en WR0 vale 0 cuando vamos a leer el primer parametro (Port A starting address Low byte), vale 1 cuando vamos a leer Port A adress high byte

//Indica ultimo comando leido, tal cual el primer byte
//z80_byte datagear_last_command_byte;


//Indica ultimo comando leido, 0=WR0, 1=WR1, etc. Caso especial: 128+2 (130) para valor de ZXN PRESCALAR (FIXED TIME TRANSFER) de WR2 en TBBLUE

z80_byte datagear_last_command;

z80_byte datagear_port_a_start_addr_low;
z80_byte datagear_port_a_start_addr_high;

z80_byte datagear_port_b_start_addr_low;
z80_byte datagear_port_b_start_addr_high;

z80_byte datagear_block_length_low;
z80_byte datagear_block_length_high;

z80_byte datagear_port_a_variable_timing_byte;
z80_byte datagear_port_b_variable_timing_byte;

//Ultimo valor recibido para los registros
z80_byte datagear_wr0;
z80_byte datagear_wr1;
z80_byte datagear_wr2;
z80_byte datagear_wr3;
z80_byte datagear_wr4;
z80_byte datagear_wr5;
z80_byte datagear_wr6;

//Si esta activada la emulacion de dma
z80_bit datagear_dma_emulation={0};

//Si esta desactivada la dma. Si esta desactivada, se puede acceder igualmente a todo, excepto que no ejecuta transferencias DMA
z80_bit datagear_dma_is_disabled={0};

//Si hay transferencia de dma activa
z80_bit datagear_is_dma_transfering={0};

//Valor de la DMA de TBBLUE de prescaler
z80_byte datagear_dma_tbblue_prescaler;

int datagear_dma_last_testados=0;

void datagear_dma_disable(void)
{
    datagear_dma_emulation.v=0;
}

void datagear_dma_enable(void)
{
    datagear_dma_emulation.v=1;
}


void datagear_reset(void)
{
    datagear_mask_commands=0;

    datagear_wr0=datagear_wr1=datagear_wr2=datagear_wr3=datagear_wr4=datagear_wr5=datagear_wr6=0;
    datagear_is_dma_transfering.v=0;

}

/*void datagear_do_transfer(void)
{
    if (datagear_dma_is_disabled.v) return;

				z80_int transfer_length=value_8_to_16(datagear_block_length_high,datagear_block_length_low);
				z80_int transfer_port_a,transfer_port_b;


					transfer_port_a=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
					transfer_port_b=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);


				if (datagear_wr0 & 4) printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_a,transfer_port_b);
                else printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_b,transfer_port_a);

                if (datagear_wr1 & 8) printf ("Port A I/O. not implemented yet\n");
                if (datagear_wr2 & 8) printf ("Port B I/O. not implemented yet\n");

				//while (transfer_length) {
                    z80_byte byte_leido;
                    if (datagear_wr0 & 4) {
                        byte_leido=peek_byte_no_time(transfer_port_a);
					    poke_byte_no_time(transfer_port_b,byte_leido);
                    }

                    else {
                        byte_leido=peek_byte_no_time(transfer_port_b);
					    poke_byte_no_time(transfer_port_a,byte_leido);
                    }

                    if ( (datagear_wr1 & 32) == 0 ) {
                        if (datagear_wr1 & 16) transfer_port_a++;
                        else transfer_port_a--;
                    }

                    if ( (datagear_wr2 & 32) == 0 ) {
                        if (datagear_wr2 & 16) transfer_port_b++;
                        else transfer_port_b--;
                    }

					transfer_length--;
				//}

    if (transfer_length==0) datagear_is_dma_transfering.v=0;

}
*/

void datagear_write_value(z80_byte value)
{
	//gestionar si estamos esperando parametros de comando
	if (datagear_mask_commands) {
		switch (datagear_last_command) {

            //printf("Datagear datagear_last_command: %d. value=%02XH\n",datagear_last_command,value);

			//WR0
			case 0:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port A starting address Low byte
				//1=Port A starting address High byte
				//2=Block length low byte
				//3=Block length high byte
				switch (datagear_command_index) {
					case 0:
						datagear_port_a_start_addr_low=value;
						//printf ("Setting port a start address low to %02XH\n",value);
					break;

					case 1:
						datagear_port_a_start_addr_high=value;
						//printf ("Setting port a start address high to %02XH\n",value);
					break;

					case 2:
						datagear_block_length_low=value;
						//printf ("Setting block length low to %02XH\n",value);
					break;

					case 3:
						datagear_block_length_high=value;
						//printf ("Setting block length high to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;

			break;

			//WR1
			case 1:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port A variable timing byte

				switch (datagear_command_index) {
					case 0:
						datagear_port_a_variable_timing_byte=value;
						//printf ("Setting port a variable timing byte to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;
			break;

			//WR2
			case 2:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port B variable timing byte

				switch (datagear_command_index) {
					case 0:
						datagear_port_b_variable_timing_byte=value;
						//printf ("Setting port b variable timing byte to %02XH\n",value);
						if (value&32 && MACHINE_IS_TBBLUE) {

							//De momento solo leo el valor del prescaler, aunque no hago nada con el

/*
TODO: Solo en Next. Si bit 5 no es 0, se leera otro parametro:
D7  D6  D5  D4  D3  D2  D1  D0  ZXN PRESCALAR (FIXED TIME TRANSFER)
#
# The ZXN PRESCALAR is a feature of the ZXN DMA implementation.
# If non-zero, a delay will be inserted after each byte is transferred
# such that the total time needed for the transfer is at least the number
# of cycles indicated by the prescalar.  This works in both the continuous
# mode and the burst mode.

# The ZXN DMA's speed matches the current CPU speed so it can operate
# at 3.5MHz, 7MHz or 14MHz.  Since the prescalar delay is a cycle count,
# the actual duration depends on the speed of the DMA.  A prescalar
# delay set to N cycles will result in a real time transfer taking N/fCPU
# seconds.  For example, if the DMA is operating at 3.5MHz and the max
# prescalar of 255 is set, the transfer time for each byte will be
# 255/3.5MHz = 72.9us.  If the DMA is used to send sampled audio, the
# sample rate would be 13.7kHz and this is the lowest sample rate possible
# using the prescalar.
#
# If the DMA is operated in burst mode, the DMA will give up any waiting
# time to the CPU so that the CPU can run while the DMA is idle.



*/

						//printf ("Will receive ZXN Prescaler\n");
                        //sleep(3);
						datagear_last_command=128+2;

						datagear_mask_commands=1;        //Realmente esto cualquier cosa diferente de 0 nos vale

						}
					break;
				}

				//Siempre que no vayamos a recibir el prescaler
				if (datagear_last_command!=130) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}
			break;

			//WR3
			case 3:
			break;

			//WR4
			case 4:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port B starting address Low byte
				//1=Port B starting address High byte
				switch (datagear_command_index) {
					case 0:
						datagear_port_b_start_addr_low=value;
						//printf ("Setting port b start address low to %02XH\n",value);
					break;

					case 1:
						datagear_port_b_start_addr_high=value;
						//printf ("Setting port b start address high to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;
			break;

			//WR5
			case 5:
			break;

			//WR6
			case 6:
			break;


			case 130:
				//printf ("Reading ZXN Prescaler = %02XH\n",value);
				datagear_dma_tbblue_prescaler=value;
				datagear_mask_commands=0;
                //sleep(1);
			break;

		}

	}

	else {

        //printf("Datagear. tipo de comando. value=%02XH\n",value);
		//datagear_last_command_byte=value;
		datagear_command_index=0;
	//Obtener tipo de comando
	//SI WR0

	z80_byte value_mask_wr0_wr3=value&(128+2+1);
	if (value_mask_wr0_wr3==1 || value_mask_wr0_wr3==2 ||value_mask_wr0_wr3==3 ) {
		//printf ("WR0\n");
		datagear_last_command=0;
		datagear_wr0=value;

		//Ver bits 4,5,6,7 y longitud comando
/*
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT A STARTING ADDRESS (LOW BYTE)
#       |   |   V
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT A STARTING ADDRESS (HIGH BYTE)
#       |   V
#  D7  D6  D5  D4  D3  D2  D1  D0  BLOCK LENGTH (LOW BYTE)
#       V
#  D7  D6  D5  D4  D3  D2  D1  D0  BLOCK LENGTH (HIGH BYTE)
*/

		datagear_mask_commands=(value>>3)&15;

		//z80_byte transfer_type=value&3;
		/*if (transfer_type==1) printf ("Type: transfer\n");
		else if (transfer_type==2) printf ("Type: search\n");
		else if (transfer_type==3) printf ("Type: search/transfer\n");

		if (value&4) printf ("Port A -> Port B\n");
		else printf ("Port B -> Port A\n");*/


	}

	if (value_mask_wr0_wr3==128) {
		//printf ("WR3\n");
		datagear_last_command=3;
		datagear_wr3=value;
	}

	if (value_mask_wr0_wr3==129) {
		//printf ("WR4 = %02XH\n",value);
		datagear_last_command=4;
		datagear_wr4=value;

		datagear_mask_commands=(value>>2)&3;



	}

	if (value_mask_wr0_wr3==128+2+1) {
		//printf ("WR6\n");
		datagear_last_command=6;
		datagear_wr6=value;

		//Tratar todos los diferentes comandos
		switch (value) {
			case 0xCF:
				//printf ("Load starting address for both ports, clear byte counter\n");
			break;

			case 0xAB:
				//printf ("Enable interrupts\n");
			break;

			case 0x87:
				//printf ("Enable DMA\n");
                datagear_is_dma_transfering.v=1;
                //datagear_do_transfer();
                datagear_dma_last_testados=t_estados;

			break;

			case 0x83:
				//printf ("Disable DMA\n");
                datagear_is_dma_transfering.v=0;

			break;

			case 0xB3:
				//printf ("Force an internal ready condition independent 'on the rdy' input\n");
			break;

			case 0xB7:
				//printf ("Enable after RETI so dma requests bus only after receiving a reti\n");
			break;


		}


	}

	z80_byte value_mask_wr1_wr2=value&(128+4+2+1);
	if (value_mask_wr1_wr2==4) {
		//printf ("WR1\n");
		datagear_last_command=1;
		datagear_wr1=value;

		//Ver bits D6
        //D6 Port A variable timing byte

		datagear_mask_commands=(value>>6)&1;

	}

	if (value_mask_wr1_wr2==0) {
		//printf ("WR2\n");
		datagear_last_command=2;
		datagear_wr2=value;

		//Ver bits D6
        //D6 Port B variable timing byte

		datagear_mask_commands=(value>>6)&1;
	}

	z80_byte value_mask_wr5=value&(128+64+4+2+1);
	if (value_mask_wr5==128+2) {
		//printf ("WR5 = %02XH\n",value);
		datagear_last_command=5;
		datagear_wr5=value;
	}

	}
}




z80_byte datagear_read_operation(z80_int address,z80_byte dma_mem_type)
{

    z80_byte byte_leido;

    if (dma_mem_type) {
        z80_int puerto_h=(address>>8)&0xFF;
        z80_int puerto_l=address & 0xFF;
        //printf("Dma read i/o port=%02X%02XH\n",puerto_h,puerto_l);
        byte_leido=lee_puerto_spectrum_no_time(puerto_h,puerto_l);
    }
    else {
        //printf("Dma read mem address=%XH\n",address);

        byte_leido=peek_byte_no_time(address);

        //temp debug
        //if (address>=0x1CE7 && address<=0x1Cf0) sleep(3);
    }

    return byte_leido;
}

void datagear_write_operation(z80_int address,z80_byte value,z80_byte dma_mem_type)
{
    if (dma_mem_type) {
        //printf("dma write i/o port=%XH\n",address);
        //if (address==0x253B) sleep(1);
		out_port_spectrum_no_time(address,value);
		//printf ("Port %04XH value %02XH\n",address,value);
		t_estados +=1; //Por ejemplo ;)
	}

    else {
        //printf("dma write mem address=%XH\n",address);
		poke_byte_no_time(address,value);
	}
}


int datagear_return_resta_testados(int anterior, int actual)
{
	//screen_testados_total

	int resta=actual-anterior;

	if (resta<0) {
        resta=screen_testados_total-anterior+actual;
    }

    //printf("datagear resta testados: %d. testados %d\n",resta,t_estados);

	return resta;
}

int datagear_condicion_transferencia(z80_int transfer_length,int dma_continuous,int resta,int dmapre)
{

	//printf ("dma condition length: %d dma_cont %d resta %d dmapre %d\n",transfer_length,dma_continuous,resta,dmapre);

	//Si hay bytes a transferir
	if (transfer_length==0) return 0;

	//Si es modo continuo
	if (dma_continuous) return 1;

	//Modo burst. Permitiendo ejecutar la cpu entre medio
	if (resta>=dmapre) return 1;


	//Otro caso, retornar 0
	return 0;


}

void datagear_handle_dma(void)
{
        if (datagear_is_dma_transfering.v==0) return;


      				z80_int transfer_length=value_8_to_16(datagear_block_length_high,datagear_block_length_low);
				z80_int transfer_port_a,transfer_port_b;


					transfer_port_a=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
					transfer_port_b=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);


				//if (datagear_wr0 & 4) printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_a,transfer_port_b);
                //else printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_b,transfer_port_a);






        int dmapre=2; //Cada 2 estados, una transferencia

		int resta=datagear_return_resta_testados(datagear_dma_last_testados,t_estados);

		//dmapre *=cpu_turbo_speed;

		int resta_antes=resta;


		//printf ("Antes transferencia: dmapre: %d datagear_dma_last_testados %d t_estados %d resta %d dmapre %d length %d\n",
		//	dmapre,datagear_dma_last_testados,t_estados,resta,dmapre,transfer_length);

		//Ver si modo continuo o modo burst
		//WR4. Bits D6 D5:
		//#       0   0 = Byte mode -> Do not use (Behaves like Continuous mode, Byte mode on Z80 DMA)
		//#       0   1 = Continuous mode
		//#       1   0 = Burst mode
		//#       1   1 = Do not use

		//Por defecto, modo continuo (todo de golpe) (dma_continuous=1). Modo burst (dma_continuous=0), permite ejecutar la cpu entre medio
		int dma_continuous=1;

        //printf("DMA WR4=%02XH\n",datagear_wr4);

		z80_byte modo_transferencia=datagear_wr4 & (64+32);

        // Burst mode nominally means the DMA lets the CPU run if either port is not ready
		if (modo_transferencia==64) dma_continuous=0;

/*
Excepción:
# The ZXN DMA can operate in either burst or continuous mode.  Continuous mode means the DMA chip
# runs to completion without allowing the CPU to run.  Burst mode nominally means the DMA lets the
# CPU run if either port is not ready.  This condition can't happen in the ZXN DMA chip except when
# operated in the special fixed time transfer mode.  In this mode, the ZXN DMA chip will let the CPU
# run while it waits for the fixed time to expire between bytes transferred.  Note that there is no
# byte transfer mode as in the Z80 DMA.
*/

		//Por tanto de momento:
		if (MACHINE_IS_TBBLUE && dma_continuous==0) {
			//Modo burst en tbblue

			//Si tiene pre escalar, se permite modo burst. Si no, no

			//printf ("Tbblue and burst mode\n");

			if ( (datagear_port_b_variable_timing_byte & 32)==0 ) {
				//printf ("burst mode not allowed on tbblue because it has no pre escalar\n");
				dma_continuous=1; //no tiene pre escalar
			}
		}


		//TODO Ver ese delay
		/*
# WR2 - Write Register Group 2
#
#  D7  D6  D5  D4  D3  D2  D1  D0  BASE REGISTER BYTE
#   0   |   |   |   |   0   0   0
#       |   |   |   |
#       |   |   |   0 = Port B is memory
#       |   |   |   1 = Port B is IO
#       |   |   |
#       |   0   0 = Port B address decrements
#       |   0   1 = Port B address increments
#       |   1   0 = Port B address is fixed
#       |   1   1 = Port B address is fixed
#       |
#       V
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT B VARIABLE TIMING BYTE
#   0   0   |   0   0   0   |   |
#           |               0   0 = Cycle Length = 4
#           |               0   1 = Cycle Length = 3
#           |               1   0 = Cycle Length = 2
#           |               1   1 = Do not use
#           |
#           V
#  D7  D6  D5  D4  D3  D2  D1  D0  ZXN PRESCALAR (FIXED TIME TRANSFER)
#
# The ZXN PRESCALAR is a feature of the ZXN DMA implementation.
# If non-zero, a delay will be inserted after each byte is transferred
# such that the total time needed for the transfer is at least the number
# of cycles indicated by the prescalar.  This works in both the continuous
# mode and the burst mode.
		*/

		//TEMP hacerlo de golpe. ejemplo: dmafill
		//while (transfer_length>0) {

		//TEMP hacerlo combinando tiempo con cpu
		//while (resta>=dmapre && transfer_length>0) {

			//dma_continuous=1;



        //Si prescaler=0, no se permite ejecucion de la cpu
        //TODO: gestionar otros valores de prescaler, permitiendo ejecucion de la cpu entre dos transferencias de la dma,
        //las cuales estan separadas por un tiempo determinado por el propio prescaler
        //Atic Atac usa prescaler=0
        if (datagear_dma_tbblue_prescaler==0) dma_continuous=1;

		//if (dma_continuous) printf ("Transferencia modo continuous\n");
		//else printf ("Transferencia modo burst\n");

		while ( datagear_condicion_transferencia(transfer_length,dma_continuous,resta,dmapre) ) {

			//for (i=0;i<cpu_turbo_speed;i++) {
				//printf ("dma op ");
			           z80_byte byte_leido;
                    if (datagear_wr0 & 4) {
                        //printf("DMA READ A, WRITE B\n");
                        byte_leido=datagear_read_operation(transfer_port_a,datagear_wr1 & 8);
					    datagear_write_operation(transfer_port_b,byte_leido,datagear_wr2 & 8);
                    }

                    else {
                        //printf("DMA READ B, WRITE A\n");
                        byte_leido=datagear_read_operation(transfer_port_b,datagear_wr2 & 8);
					    datagear_write_operation(transfer_port_a,byte_leido,datagear_wr1 & 8);
                    }

                    if ( (datagear_wr1 & 32) == 0 ) {
                        if (datagear_wr1 & 16) transfer_port_a++;
                        else transfer_port_a--;
                    }

                    if ( (datagear_wr2 & 32) == 0 ) {
                        if (datagear_wr2 & 16) transfer_port_b++;
                        else transfer_port_b--;
                    }

					transfer_length--;

			//}

			datagear_dma_last_testados +=dmapre;

			//Ajustar a total t-estados
			//printf ("pre ajuste %d\n",datagear_dma_last_testados);
			datagear_dma_last_testados %=screen_testados_total;
			//printf ("post ajuste %d\n",datagear_dma_last_testados);

			resta=datagear_return_resta_testados(datagear_dma_last_testados,t_estados);

			//printf ("En transferencia: dmapre: %6d datagear_dma_last_testados %6d t_estados %6d resta %6d\n",	dmapre,datagear_dma_last_testados,t_estados,resta);

		//si da la vuelta
			if (resta<resta_antes) {
				//provocar fin
				resta=0;
			}


			resta_antes=resta;

		}


        //Guardar valores contadores

        datagear_block_length_low=value_16_to_8l(transfer_length);
        datagear_block_length_high=value_16_to_8h(transfer_length);

        datagear_port_a_start_addr_low=value_16_to_8l(transfer_port_a);
        datagear_port_a_start_addr_high=value_16_to_8h(transfer_port_a);

        datagear_port_b_start_addr_low=value_16_to_8l(transfer_port_b);
        datagear_port_b_start_addr_high=value_16_to_8h(transfer_port_b);


				//}

    if (transfer_length==0) datagear_is_dma_transfering.v=0;

	//printf ("length: %d\n",transfer_length);

}