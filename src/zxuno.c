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
#include <string.h>

#include "zxuno.h"
#include "cpu.h"
#include "debug.h"
#include "contend.h"
#include "mem128.h"
#include "utils.h"
#include "ula.h"
#include "screen.h"
#include "zxvision.h"
#include "divmmc.h"
#include "ay38912.h"
#include "ulaplus.h"
#include "operaciones.h"
#include "chloe.h"
#include "chardevice.h"
#include "uartbridge.h"

z80_byte last_port_FC3B=0;

//Primer registro, masterconf, bit 0 a 1 (modo bootm)
z80_byte zxuno_ports[256]={1};

//Primer registro, masterconf, bit 0 a 0 (no modo bootm)
//z80_byte zxuno_ports[256]={0};

//Emulacion de SPI Flash de Winbond W25X40BV
//http://zxuno.speccy.org/ficheros/spi_datasheet.pdf
//Datos de escritura al bus spi
z80_byte zxuno_spi_bus[8];
z80_byte zxuno_spi_bus_index=0;

int last_spi_write_address;
int last_spi_read_address;
z80_byte next_spi_read_byte;

/*Registro de estado de la spi flash

Bit 7: Status register protect (non-volatile)
Bit 6: Reserved
Bit 5: Top/Bottom write protect (non-volatile)
Bit 4,3,2: Block protect bits (non-volatile)
Bit 1: Write enable latch
Bit 0: Erase or write in progress

*/

z80_byte zxuno_spi_status_register=0;


//Denegar modo turbo en boot de bios
z80_bit zxuno_deny_turbo_bios_boot={0};


//Para radas offset registro 41h. Indica a 1 que se va a escribir byte alto de registro 41
z80_bit zxuno_radasoffset_high_byte={0};

//Este es el registro 41 h pero realmente es de 14 bits,
//cuando escribes en ese registro, lo primero que se escribe son los 8 bits menos significativos
//y luego, los 6 más significativos (en la segunda escritura, los dos bits más significativos se descartan)
z80_int zxuno_radasoffset=0;


//Direcciones donde estan cada pagina de ram. 512kb ram. 32 paginas
//con bootm=0, se comporta como spectrum +2a, y,
//rams 0-7 de +2a corresponden a las primeras 8 paginas de esta tabla
//roms 0-3 de +2a corresponden a las siguientes 4 paginas de esta tabla
//z80_byte *zxuno_sram_mem_table[ZXUNO_SRAM_PAGES];
z80_byte *zxuno_sram_mem_table_new[ZXUNO_SRAM_PAGES];

//Direcciones actuales mapeadas, con modo bootm a 1
//z80_byte *zxuno_bootm_memory_paged[4];

//Direcciones actuales mapeadas, con modo bootm a 0
//z80_byte *zxuno_no_bootm_memory_paged[4];


//Direcciones actuales mapeadas, modo nuevo sin tener que distinguir entre bootm a 0 o 1
z80_byte *zxuno_memory_paged_brandnew[8];


int zxuno_flash_must_flush_to_disk=0;


//Si las escrituras de la spi flash luego se hacen flush a disco
z80_bit zxuno_flash_persistent_writes={0};


//si se permiten escrituras a la flash
z80_bit zxuno_flash_write_protection={0};


//Nombre de la flash. Si "", nombre y ruta por defecto
char zxuno_flash_spi_name[PATH_MAX]="";



//Aviso de operacion de flash spi en footer
//int zxuno_flash_operating_counter=0;

/*
Para registro de COREID:
$FF	COREID	Lectura	Cada operación de lectura proporciona el siguiente carácter ASCII de la cadena que contiene 
la revisión actual del core del ZX-Uno. Cuando la cadena termina,lecturas posteriores emiten bytes con el valor 0 
(al menos se emite uno de ellos) hasta que vuelve a comenzar la cadena. Este puntero vuelve a 0 automáticamente 
tras un reset o una escritura en el registro $FC3B. Los caracteres entregados que forman parte de la cadena 
son ASCII estándar imprimibles (códigos 32-127). Cualquier otro valor es indicativo de que este registro no está operativo.
*/
int zxuno_core_id_indice=0;

char *zxuno_core_id_message="Z27-20082020";
//Lo hago corresponder con el mismo ID del core zxuno que mas se parece a la emulación en ZEsarUX, cambiando la T inicial por una Z
//En caso de cores EXP tambien cambio EXP por Z



//Registros de DMA
/*
DMA register number to follow.
DMACTRL             equ 0a0h

DMASRC              equ 0a1h
DMADST              equ 0a2h
DMAPRE              equ 0a3h
DMALEN              equ 0a4h
DMAPROB             equ 0a5h

DMASTAT             equ 0a6h
*/

//Registros de 16 bits. Primero dmasrc, luego dmadst, etc
z80_byte zxuno_dmareg[5][2];

//dmactrl y dmastat vienen directamente de los registros zxuno tal cual

//Indices de escritura a dichos registros de 16 bits. Primero el de dmasrc, luego dmadst, etc
z80_byte zxuno_index_nibble_dma_write[5];

//Indices de lectura a dichos registros de 16 bits Primero el de dmasrc, luego dmadst, etc
z80_byte zxuno_index_nibble_dma_read[5];

//Valores en curso de la transferencia dma

z80_int zxuno_dma_current_src;
z80_int zxuno_dma_current_dst;
z80_int zxuno_dma_current_len;


z80_bit zxuno_dma_disabled={0};


//Donde empieza la vram0 de zxuno, de modo prism 256x192x4bpp 
//Luego va vram1, 2 y vram3
z80_byte *zxuno_begin_vram0_pointer;

//Archivo adicional de carga en espacio de memoria
char zxuno_initial_64k_file[PATH_MAX]="";


void zxuno_test_if_prob(void)
{

	//Si activamos bit prob
	z80_int current_prob_address;

	z80_byte dma_ctrl=zxuno_ports[0xa0];


	if (dma_ctrl & 16) {
		//1 = address in DMAPROB is related to destination (write) memory address.
		current_prob_address=zxuno_dma_current_dst;
	}
	else {
		//0 = address in DMAPROB is related to source (read) memory address
		current_prob_address=zxuno_dma_current_src;
	}

	z80_int prob_address=value_8_to_16(zxuno_dmareg[4][1],zxuno_dmareg[4][0]);

	if (prob_address==current_prob_address) zxuno_ports[0xa6] |=128;
}

char *zxuno_dma_types[]={
//   01234567890123456789
	"RAM to RAM",
	"RAM to I/O",
	"I/O to RAM",
	"I/O to I/O"
};

char *zxuno_dma_modes[]={
//   01234567890123456789
	"Stopped",
	"burst DMA",
	"timed DMA (1 shot)",
	"timed DMA (retrig)"
};

void zxuno_dma_operate(void)
{

	zxuno_test_if_prob();

	z80_byte dma_source_value;


	z80_byte dma_ctrl=zxuno_ports[0xa0];

	z80_byte mode_dma=(dma_ctrl & (4+8))>>2;

	//TODO: Transfers per second = 28000000 / preescaler_value (for memory to memory transfers)


	//DMA source
	if (mode_dma&2) {
		//1 = source address is I/O
		dma_source_value=lee_puerto_spectrum_no_time( (zxuno_dma_current_src>>8)&0xFF,zxuno_dma_current_src & 0xFF);
	}

	else {
		//0 = source address is memory
		dma_source_value=peek_byte_no_time(zxuno_dma_current_src++);
	}

	//DMA destination
	if (mode_dma&1) {
		//1 = destination address is I/O
		out_port_spectrum_no_time(zxuno_dma_current_dst,dma_source_value);
		//printf ("puerto: %04XH\n",zxuno_dma_current_dst);
	}
	else {
		//0 = destination address is memory
		poke_byte_no_time(zxuno_dma_current_dst++,dma_source_value);
	}


	zxuno_dma_current_len--;
	if (zxuno_dma_current_len==0) {
		if ( (dma_ctrl&3)==3) {
			//retrigger. TODO meter esto en funcion aparte que tambien lanza cuando se cambia dma_ctrl
	    	zxuno_dma_current_src=value_8_to_16(zxuno_dmareg[0][1],zxuno_dmareg[0][0]);
        	zxuno_dma_current_dst=value_8_to_16(zxuno_dmareg[1][1],zxuno_dmareg[1][0]);
        	zxuno_dma_current_len=value_8_to_16(zxuno_dmareg[3][1],zxuno_dmareg[3][0]);	
		}

		else {
			//DMA stop
			zxuno_ports[0xa0] &=(255-1-2);
		}
	}
}


int zxuno_return_resta_testados(int anterior, int actual)
{
	//screen_testados_total

	int resta=actual-anterior;

	if (resta<0) resta=screen_testados_total-anterior+actual;

	return resta; 
}



int zxuno_dma_last_testados=0;

void zxuno_handle_dma(void)
{
	//Si esta la dma activada
	z80_byte dma_ctrl=zxuno_ports[0xa0];

	if ( (dma_ctrl&3)==0) return;


	int dmapre=value_8_to_16(zxuno_dmareg[2][1],zxuno_dmareg[2][0]);

	//Siempre es +1 el preescaler
	//Segun dmaplayw.asm : PREESCALER          equ 223  ; la cuenta va de 0 a 223, es decir, 224 ciclos
	dmapre++;

	//Por si acaso
	if (dmapre==65536) return;
	

	//Si es ram to ram, la velocidad es 8 veces mas que otro tipo
	/*
	Transfers per second = 28000000 / preescaler_value (for memory to memory transfers)
	Transfers per second = 3500000 / preescaler_value (for transfers involving some sort of I/O address)
	*/

	if ( (dma_ctrl&(4+8))==0 ) {
		dmapre /=8;
		if (dmapre==0) dmapre=1; //Lo mas rapido que se puede
	}

	//TODO Modo dma burst transfer de momento la hago de golpe la transferencia
	if ((dma_ctrl&3)==1) { //Esto es modo  01 = burst DMA transfer. CPU is halted during the transfer. One shot.
		//transferir de golpe
		//todo cuando pre=0 
		//printf ("burst dma transfer source %04XH dest %04XH lenght %04XH\n",zxuno_dma_current_src,zxuno_dma_current_dst,zxuno_dma_current_len);

		do {
			zxuno_dma_operate();
		} while (zxuno_dma_current_len!=0);

	}


	if (dma_ctrl&2) {   //Esto incluye modos 2,3 ( 10 = timed DMA transfer. One shot. 11 = timed DMA transfer, retriggerable.)
		if (dmapre==0) return; //No hay transferencia posible . division por cero
		if (dmapre==1) return; //No hay transferencia posible . division por cero
		//printf ("Operando dma. dmapre=%d\n",dmapre);
		//Modo timed dma transfer

		int resta=zxuno_return_resta_testados(zxuno_dma_last_testados,t_estados);

		dmapre *=cpu_turbo_speed;

		//printf ("Antes transferencia: dmapre: %d zxuno_dma_last_testados %d t_estados %d\n",dmapre,zxuno_dma_last_testados,t_estados);

		while (resta>=dmapre) {
			//for (i=0;i<cpu_turbo_speed;i++) {
				zxuno_dma_operate();
			//}

			zxuno_dma_last_testados +=dmapre;

			//Ajustar a total t-estados
			//printf ("pre ajuste %d\n",zxuno_dma_last_testados);
			zxuno_dma_last_testados %=screen_testados_total;
			//printf ("post ajuste %d\n",zxuno_dma_last_testados);

			resta=zxuno_return_resta_testados(zxuno_dma_last_testados,t_estados);

			//printf ("En transferencia: dmapre: %6d zxuno_dma_last_testados %6d t_estados %6d resta %6d\n",dmapre,zxuno_dma_last_testados,t_estados,resta);

		}

		//printf ("Despues transferencia: dmapre: %d zxuno_dma_last_testados %d t_estados %d\n",dmapre,zxuno_dma_last_testados,t_estados);

		//printf ("resta %d\n",resta);

		
	}


}


void zxuno_spi_set_write_enable(void)
{
	zxuno_spi_status_register |=ZXUNO_SPI_WEL;
}

void zxuno_spi_clear_write_enable(void)
{
        //quitamos write enable de la spi flash zxuno
        zxuno_spi_status_register &=(255-ZXUNO_SPI_WEL);
}

int zxuno_spi_is_write_enabled(void)
{
	if (zxuno_spi_status_register & ZXUNO_SPI_WEL) return 1;
	return 0;
}

void zxuno_footer_print_flash_operating(void)
{


	generic_footertext_print_operating("FLASH");

    //Y poner icono en inverso
    if (!zxdesktop_icon_zxunoflash_inverse) {
        //printf("icon activity\n");
        zxdesktop_icon_zxunoflash_inverse=1;
        menu_draw_ext_desktop();
    }    
}

void zxuno_footer_flash_operating(void)
{

	zxuno_footer_print_flash_operating();


}




void zxuno_set_emulador_settings(void)
{
	//Sincronizar settings de emulador con los valores de puertos de zxuno
  zxuno_set_emulator_setting_contend();
  zxuno_set_emulator_setting_devcontrol_diay();
  zxuno_set_emulator_setting_devcontrol_ditay();
  zxuno_set_emulator_setting_disd();
  zxuno_set_emulator_setting_ditimex();
  zxuno_set_emulator_setting_diulaplus(); 
  zxuno_set_emulator_setting_diven(); 
  zxuno_set_emulator_setting_i2kb();
  zxuno_set_emulator_setting_scandblctrl();
  zxuno_set_emulator_setting_timing();
  
  
}


void hard_reset_cpu_zxuno(void)
{
	//quitamos write enable de la spi flash zxuno
	zxuno_spi_status_register &=(255-ZXUNO_SPI_WEL);



	//resetear todos los bits de masterconf y dejar solo bootm
	zxuno_ports[0]=1;

	zxuno_ports[0x0C]=255;
	zxuno_ports[0x0D]=1;
	zxuno_ports[0x0E]=0;
	zxuno_ports[0x0F]=0;
	zxuno_ports[0x40]=0;

	zxuno_radasoffset_high_byte.v=0;
	zxuno_radasoffset=0;

	zxuno_ports[0x42]=0;


	//registro RADASPALBANK, el numero 0x43 (67)
	zxuno_ports[0x43]=0;

    //registro prism
    zxuno_ports[0x50]=0;

    //colores paleta prism
    zxuno_prism_set_default_palette();

	//Sincronizar settings de emulador con los valores de puertos de zxuno
	zxuno_set_emulador_settings();

	reset_cpu();

    zxuno_load_additional_64k_block();

}




//Escribir 1 byte en la memoria spi
void zxuno_spi_page_program(int address,z80_byte valor_a_escribir)
{
	//Solo 1 MB de la spi
	int indice_spi=address & ZXUNO_SPI_SIZE_BYTES_MASK;

	if (zxuno_flash_write_protection.v) return;

	memoria_spectrum[ (ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE)*1024 + indice_spi ]=valor_a_escribir;

	zxuno_flash_must_flush_to_disk=1;

}

//Operacion de escritura a la flash spi
void zxuno_write_spi(z80_byte value)
{
	//Indicar actividad en la flash
	zxuno_footer_flash_operating();

	zxuno_spi_bus[zxuno_spi_bus_index]=value;
	if (zxuno_spi_bus_index<7) zxuno_spi_bus_index++;

	switch (zxuno_spi_bus[0]) {

		case 0x03:
			//Si instruccion es read data (3), mostrar direccion 24 bits
			//Ver si se han recibido los 24 bits
			if (zxuno_spi_bus_index==4) {
				last_spi_read_address=zxuno_spi_bus[1]*65536+zxuno_spi_bus[2]*256+zxuno_spi_bus[3];
				debug_printf (VERBOSE_DEBUG,"Write SPI. SPI Read data command, received start Address: 0x%06x",
					      last_spi_read_address);


				//Primer byte devuelve valor enviado aqui, no byte flash. O sea, el primer byte la bios lo descarta
				next_spi_read_byte=value;

				//zxuno_footer_flash_operating();
			}

			break;

		case 0x02:
			//Instruccion es page program (write)

			//Comprobacion de Write enable y puesta a 0
			if (zxuno_spi_bus_index==1) {
				if (!zxuno_spi_is_write_enabled() ) {
					debug_printf (VERBOSE_INFO,"Write SPI Page Program command but Write Enable Latch (WEL) is not enabled");
					//reseteamos indice
					zxuno_spi_bus_index=0;
				}
				//desactivamos write enable. Siguiente instruccion debera habilitarla
				zxuno_spi_clear_write_enable();
			}

			//Se ha recibido el ultimo de los 24 bits que indica direccion
			if (zxuno_spi_bus_index==4) {
				last_spi_write_address=zxuno_spi_bus[1]*65536+zxuno_spi_bus[2]*256+zxuno_spi_bus[3];
				debug_printf (VERBOSE_DEBUG,"Write SPI. SPI Page Program command, received start Adress: 0x%06x",last_spi_write_address);

			}

			//Se estan recibiendo los datos a escribir
			if (zxuno_spi_bus_index==5) {
				z80_byte valor_a_escribir=zxuno_spi_bus[4];
				debug_printf (VERBOSE_PARANOID,"Write SPI. SPI Page Program command, writing at Adress: 0x%06x Value: 0x%02x",
					last_spi_write_address,valor_a_escribir);

				//escribimos el valor
				zxuno_spi_page_program(last_spi_write_address,valor_a_escribir);

				last_spi_write_address++;

				//decrementamos el indice para que siempre se guarde el dato en posicion 4
				zxuno_spi_bus_index--;

				//zxuno_footer_flash_operating();
			}

			break;

		case 0x04:
			//Instruccion es write disable
			debug_printf (VERBOSE_DEBUG,"Write SPI. SPI Write Disable command.");
			zxuno_spi_clear_write_enable();
			break;

		case 0x06:
			//Instruccion es write enable
			debug_printf (VERBOSE_DEBUG,"Write SPI. SPI Write Enable command.");
			zxuno_spi_set_write_enable();
			break;

		case 0x20:
			//Instruccion es sector erase
			//Comprobacion de Write enable y puesta a 0
			if (zxuno_spi_bus_index==1) {
				if (!zxuno_spi_is_write_enabled() ) {
					debug_printf (VERBOSE_INFO,"Write SPI Sector Erase command but Write Enable Latch (WEL) is not enabled");
					//reseteamos indice
					zxuno_spi_bus_index=0;
				}
				//desactivamos write enable. Siguiente instruccion debera habilitarla
				zxuno_spi_clear_write_enable();
			}



			//Ver si se han recibido los 24 bits
			if (zxuno_spi_bus_index==4) {
				int sector_erase_address=zxuno_spi_bus[1]*65536+zxuno_spi_bus[2]*256+zxuno_spi_bus[3];
				debug_printf (VERBOSE_DEBUG,"Write SPI. SPI Sector Erase (4KB) command, received Address: 0x%06x",
					      sector_erase_address);

				//Hacemos esa direccion multiple de 4 kb
				sector_erase_address &=(0xFFFFFF-0xFFF);

				//Y mascara a tamanyo SPI
				sector_erase_address &= ZXUNO_SPI_SIZE_BYTES_MASK;
                                debug_printf (VERBOSE_DEBUG,"Write SPI. SPI Sector Erase (4KB) command, effective Address: 0x%06x",
                                              sector_erase_address);

				//Y borrar (poner a 255)
				int len;
				for (len=0;len<4096;len++) {
				        //memoria_spectrum[ (ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE)*1024 + sector_erase_address ]=255;
/*
        int indice_spi=address & ZXUNO_SPI_SIZE_BYTES_MASK;
        memoria_spectrum[ (ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE)*1024 + indice_spi ]=valor_a_escribir;
*/
					zxuno_spi_page_program(sector_erase_address,255);
					
					debug_printf (VERBOSE_PARANOID,"Sector Erase in progress. Address: 0x%06x",sector_erase_address);
					sector_erase_address++;
				}

			        //zxuno_flash_must_flush_to_disk=1;

			}

			//zxuno_footer_flash_operating();
			break;

		case 0x05:
			//Instruccion es read status register
			if (zxuno_spi_bus[0]==0x5) {
				debug_printf (VERBOSE_DEBUG,"Write SPI. SPI Read Status Register. ");
			}
			break;

		default:
			debug_printf (VERBOSE_DEBUG,"Write SPI. Command 0x%02X not implemented yet",zxuno_spi_bus[0]);
			break;

	}

}

//Operacion de lectura de la flash spi
z80_byte zxuno_read_spi(void)
{

	//Indicar actividad en la flash
	zxuno_footer_flash_operating();

	//debug_printf (VERBOSE_DEBUG,"Read SPI");
	z80_byte valor_leido;

	//Ver que instruccion se habia mandado a la spi
	switch (zxuno_spi_bus[0]) {


		case 0x03:
			//Lectura de datos
			valor_leido=next_spi_read_byte;


			//siguiente valor

			//Solo 1 MB de la spi
			int indice_spi=last_spi_read_address & ZXUNO_SPI_SIZE_BYTES_MASK;

			next_spi_read_byte = memoria_spectrum[ (ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE)*1024 + indice_spi ];

			debug_printf (VERBOSE_PARANOID,"Read SPI. Read Data command. Address: 0x%06X Next Value: 0x%02X",
				      indice_spi,next_spi_read_byte);

			last_spi_read_address++;


			return valor_leido;
			break;

		case 0x05:
			//Read Status Register

			debug_printf (VERBOSE_DEBUG,"Read SPI. Read Status Register. Value: 0x%02X",zxuno_spi_status_register);
			return zxuno_spi_status_register;
			break;

		case 0x9f:
			//Read JEDEC ID (9Fh)


			debug_printf (VERBOSE_DEBUG,"Read SPI. Read JEDEC ID (9Fh)");
            //Incompleto. Retornar siempre el mismo byte (0x16), para indicar flash de 4MB y poder actualizar la flash de zxuno desde la bios de zxuno
			return 0x16; //16h= 4 MB. 18h=16 MB
			break;            

		default:
			debug_printf (VERBOSE_DEBUG,"Read SPI. Command 0x%02X not implemented",zxuno_spi_bus[0]);
			return 0;
			break;

	}


}


//Lectura de puerto ZX-Uno
z80_byte zxuno_read_port(z80_int puerto)
{

	z80_byte dma_index_register,dma_index_nibble;

	if (puerto==0xFC3B) {
		//Leer valor indice
		//printf ("In Port %x ZX-Uno read, value %d, PC after=%x\n",puerto,last_port_FC3B,reg_pc);
		return last_port_FC3B;
	}

	if (puerto==0xFD3B) {
		//Leer contenido de uno de los 256 posibles registros
		//printf ("In Port %x ZX-Uno read, register %d, value %d, PC after=%x\n",puerto,last_port_FC3B,zxuno_ports[last_port_FC3B],reg_pc);

		if (last_port_FC3B==2) {

			//Si esta bit LOCK, no permitir lectura de SPI
			if ((zxuno_ports[0]&128)==128) {
				debug_printf (VERBOSE_DEBUG,"LOCK bit set. Not allowed FLASHSPI read");
				return 255;
			}
			//printf ("Leyendo FLASHSPI\n");
			//devolver byte de la flash spi

			return zxuno_read_spi();


		}

		//Registro de Core ID
		else if (last_port_FC3B==0xFF) {
			//Retorna Core ID
			int len=strlen(zxuno_core_id_message);
			if (zxuno_core_id_indice==len) {
				zxuno_core_id_indice=0;
				return 0;
			}
			else {
				return zxuno_core_id_message[zxuno_core_id_indice++];
			}
		}

		else if (last_port_FC3B>=0xa1 && last_port_FC3B<=0xa5) {
			dma_index_register=last_port_FC3B-0xa1;
			dma_index_nibble=zxuno_index_nibble_dma_read[dma_index_register];
				
			//Leer de registro dma indicado
			z80_byte valor_retorno=zxuno_dmareg[dma_index_register][dma_index_nibble];

			printf ("Leyendo registro dma %d valor %02XH\n",dma_index_register,valor_retorno);


			//Cambiar al otro nibble de 16 bits
			zxuno_index_nibble_dma_read[dma_index_register] ^=1;
			return valor_retorno;
		}

		else if (last_port_FC3B==0xa6) {
			//DMASTAT
			/*
			DMASTAT :
8 bit status register. Currently, it uses only bit 7.
Bit 7: set to 1 when DMAPROB address has been reached. It automatically reset to 0 after reading this register.
8 bit, read only.
			*/
			z80_byte valor_retorno=zxuno_ports[0xa6];
			zxuno_ports[0xa6] &=127; //Resetear bit 7

			return valor_retorno;
		}


		else if (last_port_FC3B==ZXUNO_UART_DATA_REG) {
			//UART_DATA_REG
			return zxuno_uartbridge_readdata();
		}

		else if (last_port_FC3B==ZXUNO_UART_STAT_REG) {
			//UART_STAT_REG
			return zxuno_uartbridge_readstatus();
		}


		return zxuno_ports[last_port_FC3B];
	}

	//Sera uno de los dos puertos... pero para que no se queje el compilador, hacemos return 0
	return 0;

}


void zxuno_prism_mode_splash(void)
{

    if (zxuno_ports[0x50] & 128) {
        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling ZX-Uno Prism mode. 256x192x4bpp");
    }
    else {
        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling ZX-Uno Prism mode");
    }
}



//Escritura de puerto ZX-Uno
void zxuno_write_port(z80_int puerto, z80_byte value)
{

	z80_byte dma_index_register,dma_index_nibble;

	if (puerto==0xFC3B) {
		//printf ("Out Port %x ZX-Uno written with value %x, PC after=%x\n",puerto,value,reg_pc);
		last_port_FC3B=value;
		zxuno_core_id_indice=0;
	}
	if (puerto==0xFD3B) {

		z80_byte anterior_masterconf=zxuno_ports[0];
		z80_byte anterior_devcontrol=zxuno_ports[0x0E];
		z80_byte anterior_devctrl2=zxuno_ports[0x0F];
		z80_byte anterior_scandblctrl=zxuno_ports[0x0B];
        z80_byte anterior_prism=zxuno_ports[0x50];
		//Si se va a tocar masterconf, ver cambios en bits, como por ejemplo I2KB

		//El indice al registro viene por last_port_FC3B
		//printf ("Out Port %x ZX-Uno written, index: %d with value %x, PC after=%x\n",puerto,last_port_FC3B,value,reg_pc);
		//Si Lock esta a 1, en masterconf, mascara de bits y avisar
		if (last_port_FC3B==0 && (zxuno_ports[0]&128)==128 ) {
			debug_printf (VERBOSE_DEBUG,"MASTERCONF lock bit set to 1");

			//Solo se pueden modificar bits 6,5,4,3
			//Hacemos mascara del valor nuevo quitando bits 7,0,1,2 (que son los que no se tocan)
			value=value&(255-128-1-2-4);

			z80_byte valor_puerto=zxuno_ports[0];

			//Valor actual de masterconf conservamos bits 7,0,1,2
			valor_puerto = valor_puerto & (128+1+2+4);

			//Y or con lo que teniamos
			valor_puerto |= value;

			value = valor_puerto;
		}


		zxuno_ports[last_port_FC3B]=value;

		//Que registro se ha tocado?
		switch (last_port_FC3B) {
			case 0:
				//Si 0, masterconf
				debug_printf (VERBOSE_DEBUG,"Write Masterconf. BOOTM=%d",value&1);

				//ver cambio en bit bootm
				z80_byte bootm=zxuno_ports[0]&1;
			 if ( (anterior_masterconf&1) != bootm) {
				 zxuno_set_memory_pages();
			 }

				//ver cambio en bit diven
				 z80_byte diven=zxuno_ports[0]&2;
        if ( (anterior_masterconf&2) != diven) {
          zxuno_set_emulator_setting_diven();
        }

				//ver cambios en bits, por ejemplo I2KB
				z80_byte i2kb=zxuno_ports[0]&8;
				if ( (anterior_masterconf&8) != i2kb) {
					zxuno_set_emulator_setting_i2kb();
				}


				//ver cambio en bits timings
				z80_byte timing=zxuno_ports[0]&(16+64);
				if ( (anterior_masterconf&(16+64)) != timing) {
					zxuno_set_emulator_setting_timing();
				}

				//ver cambio en bit contended
				z80_byte contend=zxuno_ports[0]&32;
				if ( (anterior_masterconf&32) != contend) {
					zxuno_set_emulator_setting_contend();
				}



				break;

			case 1:
				debug_printf (VERBOSE_DEBUG,"Write Mastermapper. Bank=%d",value&31);

				zxuno_set_memory_pages();

				/*
				//Si esta en modo ejecucion, no hacer nada
				if ( (zxuno_ports[0] &1)==0) {
					debug_printf (VERBOSE_DEBUG,"Write Mastermapper but zxuno is not on boot mode, so it has no effect");
				}

				else {

					//Paginar 512kb de zxuno
					z80_byte bank=value&31;
					zxuno_page_ram(bank);

				}
				*/

				break;

			case 2:
				//debug_printf (VERBOSE_DEBUG,"Write SPI. Value: 0x%02x",value);

				//Si esta bit LOCK, no permitir escritura en SPI
				if ((zxuno_ports[0]&128)==128) {
					debug_printf (VERBOSE_DEBUG,"LOCK bit set. Not allowed FLASHSPI command");
				}
				else zxuno_write_spi(value);


				break;

			case 3:
				debug_printf (VERBOSE_DEBUG,"Write FLASHCS. Value: %d %s",value,(value&1 ? "not selected" : "selected") );
				//Si esta bit LOCK, no permitir escritura en SPI
				if ((zxuno_ports[0]&128)==128) {
					debug_printf (VERBOSE_DEBUG,"LOCK bit set. Not allowed FLASHCS command");
				}
				else {
					if ( (value&1)==0) {
						zxuno_spi_bus_index=0;
					}
					//Indicar actividad en la flash
					zxuno_footer_flash_operating();
				}

				break;

			case 0x0B:
				//Si 0x0B, SCANDBLCTRL
				debug_printf (VERBOSE_DEBUG,"Write SCANDBLCTRL. Value=%d",value);

				z80_byte scn=zxuno_ports[0x0B];
				if (anterior_scandblctrl!=scn) zxuno_set_emulator_setting_scandblctrl();
			break;


                        case 0x0E:
                                //Si 0x0E, DEVCONTROL
                                debug_printf (VERBOSE_DEBUG,"Write Devcontrol. Value=%d",value);


                                //ver cambio en bit diay
                                 z80_byte diay=zxuno_ports[0x0E]&1;
                                if ( (anterior_devcontrol&1) != diay) {
					zxuno_set_emulator_setting_devcontrol_diay();
                                }

				//ver cambio en bit ditay
				z80_byte ditay=zxuno_ports[0x0E]&2;
				if ( (anterior_devcontrol&2) != ditay) {
                                        zxuno_set_emulator_setting_devcontrol_ditay();
                                }

		                  //ver cambio en bit disd
                                 z80_byte disd=zxuno_ports[0x0E]&128;
                                if ( (anterior_devcontrol&128) != disd) {
                                        zxuno_set_emulator_setting_disd();
                                }


			break;

			case 0x0F:
				//Si 0x0F, DEVCTRL2
				debug_printf (VERBOSE_DEBUG,"Write DEVCTRL2. Value=%d",value);
				/*bit 2 DIRADAS: a 1 para deshabilitar el modo radastaniano. Tenga en cuenta que si el modo radastaniano no se deshabilita, pero se deshabilita la ULAplus, al intentar usar el modo radastaniano, el datapath usado en la ULA no será el esperado y el comportamiento de la pantalla en este caso no está documentado.
				DITIMEX: a 1 para deshabilitar los modos de pantalla compatible Timex. Cualquier escritura al puerto $FF es por tanto ignorada. Si la MMU del Timex está habilitada, una lectura al puerto $FF devolverá 0.
				DIULAPLUS: a 1 para deshabilitar la ULApluis. Cualquier escritura a los puertos de ULAplus se ignora. Las lecturas a dichos puertos devuelven el valor del bus flotante. No obstante tenga en cuenta que el mecanismo de contención para este puerto sigue funcionando aunque esté deshabilitado.
				*/

				//TODO: poder desactivar modo radastan

				z80_byte ditimex=zxuno_ports[0x0F]&2;
				if ( (anterior_devctrl2 & 2) !=ditimex) {
					zxuno_set_emulator_setting_ditimex();
				}

				z80_byte diulaplus=zxuno_ports[0x0F]&1;
				if ( (anterior_devctrl2 & 1) !=diulaplus) {
					zxuno_set_emulator_setting_diulaplus();
				}
			break;



			case 0x40:
                                ulaplus_set_extended_mode(value);

			break;

			case 0x41:
				//radasoffset
				if (zxuno_radasoffset_high_byte.v) {
						zxuno_radasoffset &=255;
						zxuno_radasoffset |=(value<<8);
					}
				else {
					zxuno_radasoffset &= 0xFF00;
					zxuno_radasoffset |=value;
				}

				zxuno_radasoffset_high_byte.v ^=1;

			break;

            case 0x50:
                //printf("Change prism mode, mapping etc value: %d\n",zxuno_ports[0x50]);
                //Mapping vram prism zxuno
                if ((zxuno_ports[0x50] & 64) != (anterior_prism & 64)) {
                    //printf("Setting memory pages after change prism mode\n");
                    zxuno_set_memory_pages();
                }

                if ((zxuno_ports[0x50] & 128) != (anterior_prism & 128)) {
                    zxuno_prism_mode_splash();
                }

            break;

            case 0x51:
                zxuno_prism_set_color_palette();
            break;
			

			//Registros DMA de 16 bits
			case 0xa1:
			case 0xa2:
			case 0xa3:
			case 0xa4:
			case 0xa5:
				dma_index_register=last_port_FC3B-0xa1;
				dma_index_nibble=zxuno_index_nibble_dma_write[dma_index_register];
				
				//Escribir en registro dma indicado
				zxuno_dmareg[dma_index_register][dma_index_nibble]=value;

				//printf ("Escribiendo registro dma %d valor %02XH\n",dma_index_register,value);


				//Cambiar al otro nibble de 16 bits
				zxuno_index_nibble_dma_write[dma_index_register] ^=1;

			break;

			case 0xa0:
				//DMACTRL
				zxuno_dma_current_src=value_8_to_16(zxuno_dmareg[0][1],zxuno_dmareg[0][0]);
				zxuno_dma_current_dst=value_8_to_16(zxuno_dmareg[1][1],zxuno_dmareg[1][0]);
				zxuno_dma_current_len=value_8_to_16(zxuno_dmareg[3][1],zxuno_dmareg[3][0]);

				zxuno_dma_last_testados=t_estados;

				//printf ("Starting DMA src=%04XH dst=%02XH len=%04XH\n",zxuno_dma_current_src,zxuno_dma_current_dst,zxuno_dma_current_len);
				//sleep(1);
			break;



			//UART_DATA_REG
			case ZXUNO_UART_DATA_REG:
				zxuno_uartbridge_writedata(value);
			break;

			//UART_STAT_REG
			//case ZXUNO_UART_STAT_REG:
				//registro no es de escritura
			//break;			

			case 0xfd:
				/*
				COREBOOT
				Registro de control de arranque. Escribiendo un 1 en el bit 0 de este registro (el resto de bits están reservados y deben quedarse a 0) hace que se desencadene el mecanismo interno de la FPGA que permite arrancar otro core. La dirección de comienzo de este segundo core será la última que se escribiera usando el registro COREADDR. 
				*/
				if (value & 1) hard_reset_cpu();
			break;


		}
	}


}

z80_byte zxuno_get_devcontrol_di7ffd(void)
{
	//Paginacion desactivada por puertos de zxuno DEVCONTROL. DI7FFD
	return zxuno_ports[0x0E]&4;
}

z80_byte zxuno_get_devcontrol_di1ffd(void)
{
	//Paginacion desactivada por puertos de zxuno DEVCONTROL. DI1FFD
	return zxuno_ports[0x0E]&8;
}

//Rutinas de puertos paginacion zxuno pero cuando bootm=0, o sea, como plus2a
void zxuno_p2a_write_page_port(z80_int puerto, z80_byte value)
{
	//Puerto tipicamente 32765
	// the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
	if ( (puerto & 49154) == 16384 ) {
		//ver si paginacion desactivada
		if (puerto_32765 & 32) return;

		//64 kb rom, 128 ram

		//Paginacion desactivada por puertos de zxuno DEVCONTROL. DI7FFD
		//if (zxuno_ports[0x0E]&4) return;
		if (zxuno_get_devcontrol_di7ffd()) return;

		puerto_32765=value;


		//si modo de paginacion ram en rom, volver
		/*
		Este es el comportamiento habitual en paginacion de 128kb, pero no en zxuno,
		lo habitual es no repaginar cuando esta la paginacion ram en rom, ya que paginaria una ram probablemente distinta en c000h
		Pero en zxuno esto ya lo detecto en la funcion siguiente zxuno_set_memory_pages,
		ya que ahí verá que está la paginacion ram en rom y pondrá correctamente la paginacion que toca, sin poner una ram distinta en c000h
		Realmente, si esta la paginación de ram en rom, los cambios en el puerto 32765 no tienen efecto a nivel de páginas (pero si 
		a nivel de cambio video shadow 5/7 o incluso desactivar paginacion)
		TODO: se deberia cambiar la paginacion habitual del 128kb tal y como la hago aqui, con una funcion comun set_memory_pages
		para puertos 32765 y 8189 y detecte si esta paginacion ram en rom 
		if ((puerto_8189&1)==1) {
			//printf ("Ram in ROM enabled. So RAM paging change with 32765 not allowed. out value:%d\n",value);
			//Livingstone supongo II hace esto, continuamente cambia de Screen 5/7 y ademas cambia
			//formato de paginas de 4,5,6,3 a 4,7,6,3 tambien continuamente
			//Hay que tener en cuenta que pese a que no hace cambio de paginacion,
			//si que permite cambiar de video shadow 5/7 ya que el puerto_32765 ya se ha escrito antes de entrar aqui
			return;
		}
		*/	

		zxuno_set_memory_pages();

		return;
	}

	//Puerto tipicamente 8189

	// the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
	if ( (puerto & 61442 )== 4096) {
		//ver si paginacion desactivada
		if (puerto_32765 & 32) return;

		//Paginacion desactivada por puertos de zxuno DEVCONTROL. DI7FFD
		//if (zxuno_ports[0x0E]&4) return;
		if (zxuno_get_devcontrol_di7ffd()) return;

		//Paginacion desactivada por puertos de zxuno DEVCONTROL. DI1FFD
		//if (zxuno_ports[0x0E]&8) return;
		if (zxuno_get_devcontrol_di1ffd()) return;

		puerto_8189=value;

		zxuno_set_memory_pages();

		return;

	}
}

void zxuno_load_spi_flash(void)
{

	FILE *ptr_flashfile;
	int leidos=0;

	if (zxuno_flash_spi_name[0]==0) {
		open_sharedfile(ZXUNO_SPI_FLASH_NAME,&ptr_flashfile);
	}
	else {
		debug_printf (VERBOSE_INFO,"Opening ZX-Uno Custom Flash File %s",zxuno_flash_spi_name);
		ptr_flashfile=fopen(zxuno_flash_spi_name,"rb");
	}


	if (ptr_flashfile!=NULL) {

		//la flash esta despues de los primeros 16kb rom y 512kb sram
		leidos=fread(&memoria_spectrum[ (ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE)*1024 ],1,ZXUNO_SPI_SIZE*1024,ptr_flashfile);
		fclose(ptr_flashfile);

	}



	if (leidos!=ZXUNO_SPI_SIZE*1024 || ptr_flashfile==NULL) {
		debug_printf (VERBOSE_ERR,"Error reading ZX-Uno SPI Flash");
		//cpu_panic("Error loading ZX-Uno SPI Flash");
	}


}


void zxuno_set_timing_128k(void)
{
	//Mismos timings que 128k
	contend_read=contend_read_128k;
	contend_read_no_mreq=contend_read_no_mreq_128k;
	contend_write_no_mreq=contend_write_no_mreq_128k;

	ula_contend_port_early=ula_contend_port_early_128k;
	ula_contend_port_late=ula_contend_port_late_128k;


	screen_testados_linea=228;
	screen_invisible_borde_superior=7;
	screen_invisible_borde_derecho=104;

	port_from_ula=port_from_ula_48k; 
	contend_pages_128k_p2a=contend_pages_128k;


}


void zxuno_set_timing_48k(void)
{
	contend_read=contend_read_48k;
	contend_read_no_mreq=contend_read_no_mreq_48k;
	contend_write_no_mreq=contend_write_no_mreq_48k;

	ula_contend_port_early=ula_contend_port_early_48k;
	ula_contend_port_late=ula_contend_port_late_48k;

	screen_testados_linea=224;
	screen_invisible_borde_superior=8;
	screen_invisible_borde_derecho=96;

	port_from_ula=port_from_ula_48k;

	//esto no se usara...
	contend_pages_128k_p2a=contend_pages_128k;

}

void zxuno_change_timing(int timing)
{
	if (timing==0) zxuno_set_timing_48k();
	else if (timing==1) zxuno_set_timing_128k();
	else if (timing==2) ula_enable_pentagon_timing();

	screen_set_video_params_indices();
	inicializa_tabla_contend();

}


void zxuno_set_emulator_setting_i2kb(void)
{

	z80_byte i2kb=zxuno_ports[0]&8;

	debug_printf (VERBOSE_INFO,"Apply MASTERCONF.I2KB change: issue%d",(i2kb ? 2 : 3) );
	if (i2kb) keyboard_issue2.v=1;
	else keyboard_issue2.v=0;
}


void zxuno_set_emulator_setting_timing(void)
{
	/*MASTERCONF:
	LOCK	MODE1	DISCONT	MODE0	I2KB	DISNMI	DIVEN	BOOTM
	MODE1,MODE0: especifica el modo de timing de la ULA para acomodarse a diferentes modelos de Spectrum. 00 = ULA ZX Spectrum 48K PAL, 01 = ZX Spectrum 128K/+2 gris, 10 = Pentagon 128, 11 = Reservado.
	*/


	z80_byte timing;
	timing=(zxuno_ports[0]&16)>>4;
	timing|=(zxuno_ports[0]&64)>>5;

	if (timing==0) debug_printf (VERBOSE_INFO,"Apply MASTERCONF.TIMING change: 48k");
	else if (timing==1) debug_printf (VERBOSE_INFO,"Apply MASTERCONF.TIMING change: 128k");
	else if (timing==2) debug_printf (VERBOSE_INFO,"Apply MASTERCONF.TIMING change: Pentagon");
	else debug_printf (VERBOSE_INFO,"Apply MASTERCONF.TIMING unknown");
	zxuno_change_timing(timing);

}

void zxuno_set_emulator_setting_contend(void)
{
	z80_byte contend=zxuno_ports[0]&32;
	debug_printf (VERBOSE_INFO,"Apply MASTERCONF.DISCONT change: %s",(contend ? "disabled" : "enabled") );
	if (contend) contend_enabled.v=0;
	else contend_enabled.v=1;
	inicializa_tabla_contend();
}

void zxuno_set_emulator_setting_diven(void)
{
	z80_byte diven=zxuno_ports[0]&2;
	debug_printf (VERBOSE_INFO,"Apply MASTERCONF.DIVEN change: %s",(diven ? "enabled" : "disabled") );
        if (diven) divmmc_diviface_enable();
	else divmmc_diviface_disable();
}


void zxuno_set_emulator_setting_disd(void)
{
        z80_byte disd=zxuno_ports[0x0E]&128;
        debug_printf (VERBOSE_INFO,"Apply DEVCONTROL.DISD change: %s",(disd ? "enabled" : "disabled") );
        if (disd) divmmc_mmc_ports_disable();
        else divmmc_mmc_ports_enable();
}


void zxuno_set_emulator_setting_ditimex(void)
{
        z80_byte ditimex=zxuno_ports[0x0F]&2;
        debug_printf (VERBOSE_INFO,"Apply DEVCTRL2.DITIMEX change: %s",(ditimex ? "disabled" : "enabled") );
        if (ditimex) disable_timex_video();
        else enable_timex_video();
}

void zxuno_set_emulator_setting_diulaplus(void)
{
        z80_byte diulaplus=zxuno_ports[0x0F]&1;
        debug_printf (VERBOSE_INFO,"Apply DEVCTRL2.DIULAPLUS change: %s",(diulaplus ? "disabled" : "enabled") );
        if (diulaplus) disable_ulaplus();
        else enable_ulaplus();
}




//El resto de bits son leidos directamente por el core, actuando en consecuencia
void zxuno_set_emulator_setting_devcontrol_diay(void)
{
	z80_byte diay=zxuno_ports[0x0E]&1;
	debug_printf (VERBOSE_INFO,"Apply DEVCONTROL.DIAY change: %s",(diay ? "disabled" : "enabled") );

	if (diay) ay_chip_present.v=0;
	else ay_chip_present.v=1;
}


void zxuno_set_emulator_setting_devcontrol_ditay(void)
{
        z80_byte ditay=zxuno_ports[0x0E]&2;
        debug_printf (VERBOSE_INFO,"Apply DEVCONTROL.DITAY change: %s",(ditay ? "disabled" : "enabled") );

        if (ditay) set_total_ay_chips(1);
        else set_total_ay_chips(2);

}






//Ajustar modo turbo. Resto de bits de ese registro no usados
void zxuno_set_emulator_setting_scandblctrl(void)
{
	z80_byte scn=(zxuno_ports[0x0B]>>6)&3;

/* Limitar cuando se activa modo turbo. Dado que si se activa en la bios se satura mucho el emulador, no dejo hacerlo ni en arranque ni en bios
turbo modo 8 en pc=50
turbo modo 8 en pc=312
turbo modo 8 en pc=50
turbo modo 8 en pc=312
*/

	int t;

	if (scn==0) t=1;
	else if (scn==1) t=2;
	else if (scn==2) t=4;
	else t=8;

	debug_printf (VERBOSE_INFO,"Set zxuno turbo mode %d with pc=%d",t,reg_pc);

    //printf ("Set zxuno turbo mode %d with pc=%d\n",t,reg_pc);

	if (zxuno_deny_turbo_bios_boot.v) {

		if (t>1) {
			if (reg_pc==50 || reg_pc==356) {
				debug_printf (VERBOSE_INFO,"Not changing cpu speed on zxuno bios. We dont want to use too much real cpu for this!");
				//printf ("Not changing cpu speed on zxuno bios. We dont want to use too much real cpu for this!\n");
				return;
			}
		}

	}

	cpu_turbo_speed=t;

	cpu_set_turbo_speed();
}



void zxuno_flush_flash_to_disk(void)
{
	if (!MACHINE_IS_ZXUNO) return;


	if (zxuno_flash_must_flush_to_disk==0) {
		debug_printf (VERBOSE_DEBUG,"Trying to flush SPI FLASH to disk but no changes made");
		return;
	}


	if (zxuno_flash_persistent_writes.v==0) {
		debug_printf (VERBOSE_DEBUG,"Trying to flush SPI FLASH to disk but persistent writes disabled");
		return;
	}

	debug_printf (VERBOSE_INFO,"Flushing ZX-Uno FLASH to disk");


	FILE *ptr_spiflashfile;

	if (zxuno_flash_spi_name[0]==0) {
		open_sharedfile_write(ZXUNO_SPI_FLASH_NAME,&ptr_spiflashfile);
	}
	else {
		debug_printf (VERBOSE_INFO,"Opening ZX-Uno Custom Flash File %s",zxuno_flash_spi_name);
		ptr_spiflashfile=fopen(zxuno_flash_spi_name,"wb");
	}


	int escritos=0;
	int size;
	size=ZXUNO_SPI_SIZE*1024;


	if (ptr_spiflashfile!=NULL) {
		z80_byte *puntero;
		puntero=&memoria_spectrum[ (ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE)*1024 ];

                //Justo antes del fwrite se pone flush a 0, porque si mientras esta el fwrite entra alguna operacion de escritura,
                //metera flush a 1
		zxuno_flash_must_flush_to_disk=0;
		escritos=fwrite(puntero,1,size,ptr_spiflashfile);

		fclose(ptr_spiflashfile);


	}

	//printf ("ptr_spiflashfile: %d\n",ptr_spiflashfile);
	//printf ("escritos: %d\n",escritos);

	if (escritos!=size || ptr_spiflashfile==NULL) {
		debug_printf (VERBOSE_ERR,"Error writing to SPI Flash file. Disabling write file operations");
		zxuno_flash_persistent_writes.v=0;
	}

}




//Nuevas funciones de MMU





void zxuno_chloe_init_memory_tables(void)
{
	debug_printf (VERBOSE_DEBUG,"Initializing Chloe memory pages");

	/*
//Direcciones donde estan cada pagina de rom. 2 paginas de 16 kb
//z80_byte *chloe_rom_mem_table[2];

//Direcciones donde estan cada pagina de ram home
//z80_byte *chloe_home_ram_mem_table[8];

//Direcciones donde estan cada pagina de ram ex
//z80_byte *chloe_ex_ram_mem_table[8];

//Direcciones donde estan cada pagina de ram dock
//z80_byte *chloe_dock_ram_mem_table[8];

//Direcciones actuales mapeadas, bloques de 8 kb
	*/

	//memoria_spectrum
	//32 kb rom
	//128kb home
	//64 kb ex
	//64 kb dock

	z80_byte *puntero;
	

	//int puntero=16384; //saltamos los primeros 16kb de rom del bootloader

	puntero=zxuno_sram_mem_table_new[8]; //ROMS
	chloe_rom_mem_table[0]=puntero;
	chloe_rom_mem_table[1]=&puntero[16384];

	int i;

	puntero=zxuno_sram_mem_table_new[0]; //RAMS
	for (i=0;i<8;i++) {
		chloe_home_ram_mem_table[i]=puntero;
		puntero +=16384;
	}

	puntero=zxuno_sram_mem_table_new[24]; //EXT
	for (i=0;i<8;i++) {
		chloe_ex_ram_mem_table[i]=puntero;
		puntero +=8192;
	}

	puntero=zxuno_sram_mem_table_new[28]; //EXT
	for (i=0;i<8;i++) {
		chloe_dock_ram_mem_table[i]=puntero;
		puntero +=8192;
	}



}


void zxuno_init_memory_tables(void)
{

                int puntero=16384; //saltamos los primeros 16kb de rom del bootloader
                int i;

                //Paginas SRAM de zxuno
                for (i=0;i<ZXUNO_SRAM_PAGES;i++) {
                        zxuno_sram_mem_table_new[i]=&memoria_spectrum[puntero];
                        puntero +=16384;
                }

    //Paginas VRAM
    /*
    		malloc_machine((ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE+ZXUNO_SPI_SIZE+8*3)*1024);
    */

    //Van justo despues de la flash spi

    zxuno_begin_vram0_pointer=&memoria_spectrum[(ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE+ZXUNO_SPI_SIZE)*1024];

	zxuno_chloe_init_memory_tables();
}


z80_byte zxuno_get_rom_page(void)
{

	//asignar rom
	//z80_byte rom_entra=((puerto_32765>>4)&1) + ((puerto_8189>>1)&2);

	z80_byte rom1f=(puerto_8189>>1)&2;
	z80_byte rom7f=(puerto_32765>>4)&1;

	z80_byte dirom1f=((zxuno_ports[0x0E]>>4)^255)&2; //Tiene que ir al bit 1
	z80_byte dirom7f=((zxuno_ports[0x0E]>>4)^255)&1; //Tiene que ir al bit 0

	z80_byte rom_entra=(rom1f&dirom1f)+(rom7f&dirom7f);


	return rom_entra;
}

//Rutinas de puertos paginacion zxuno pero cuando bootm=0, o sea, como plus2a
z80_byte  zxuno_get_ram_page(void)
{
	return puerto_32765&7;
}



void zxuno_set_memory_pages_ram_rom(void)
{

		z80_byte page_type;

		z80_byte pagina0, pagina1, pagina2, pagina3;

		page_type=(puerto_8189 >>1) & 3;

		switch (page_type) {
			case 0:

				debug_printf (VERBOSE_DEBUG,"Pages 0,1,2,3");
				pagina0=0;
				pagina1=1;
				pagina2=2;
				pagina3=3;

				break;

			case 1:
				debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");
				pagina0=4;
				pagina1=5;
				pagina2=6;
				pagina3=7;

				break;

			case 2:
				debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,3");
				pagina0=4;
				pagina1=5;
				pagina2=6;
				pagina3=3;

				break;

			case 3:
				debug_printf (VERBOSE_DEBUG,"Pages 4,7,6,3");
				pagina0=4;
				pagina1=7;
				pagina2=6;
				pagina3=3;

				break;

		}

		zxuno_memory_paged_brandnew[0*2]=zxuno_sram_mem_table_new[pagina0];
		zxuno_memory_paged_brandnew[0*2+1]=zxuno_sram_mem_table_new[pagina0]+8192;

		zxuno_memory_paged_brandnew[1*2]=zxuno_sram_mem_table_new[pagina1];
		zxuno_memory_paged_brandnew[1*2+1]=zxuno_sram_mem_table_new[pagina1]+8192;

		zxuno_memory_paged_brandnew[2*2]=zxuno_sram_mem_table_new[pagina2];
		zxuno_memory_paged_brandnew[2*2+1]=zxuno_sram_mem_table_new[pagina2]+8192;

		zxuno_memory_paged_brandnew[3*2]=zxuno_sram_mem_table_new[pagina3];
		zxuno_memory_paged_brandnew[3*2+1]=zxuno_sram_mem_table_new[pagina3]+8192;

		contend_pages_actual[0]=contend_pages_128k_p2a[pagina0];
		contend_pages_actual[1]=contend_pages_128k_p2a[pagina1];
		contend_pages_actual[2]=contend_pages_128k_p2a[pagina2];
		contend_pages_actual[3]=contend_pages_128k_p2a[pagina3];

		debug_paginas_memoria_mapeadas[0]=pagina0;
		debug_paginas_memoria_mapeadas[1]=pagina1;
		debug_paginas_memoria_mapeadas[2]=pagina2;
		debug_paginas_memoria_mapeadas[3]=pagina3;



}

int zxuno_is_chloe_mmu(void)
{
	return zxuno_ports[0x0e] & 64;
}

int is_zxuno_chloe_mmu(void)
{
	if (MACHINE_IS_ZXUNO && zxuno_is_chloe_mmu() ) return 1;
	else return 0;
}

void zxuno_set_memory_pages(void)
{



	//Muy facil
	z80_byte pagina0, pagina1, pagina2, pagina3;

	//Si es bootm a 1, son unas tablas
	if (ZXUNO_BOOTM_ENABLED) {
		pagina0=0;
		pagina1=5;
		pagina2=2;
		pagina3=zxuno_ports[1]&31;

		//Los 16kb de rom del zxuno
		zxuno_memory_paged_brandnew[0*2]=memoria_spectrum;
		zxuno_memory_paged_brandnew[0*2+1]=memoria_spectrum+8192;

		zxuno_memory_paged_brandnew[1*2]=zxuno_sram_mem_table_new[pagina1];
		zxuno_memory_paged_brandnew[1*2+1]=zxuno_sram_mem_table_new[pagina1]+8192;

		zxuno_memory_paged_brandnew[2*2]=zxuno_sram_mem_table_new[pagina2];
		zxuno_memory_paged_brandnew[2*2+1]=zxuno_sram_mem_table_new[pagina2]+8192;

		zxuno_memory_paged_brandnew[3*2]=zxuno_sram_mem_table_new[pagina3];
		zxuno_memory_paged_brandnew[3*2+1]=zxuno_sram_mem_table_new[pagina3]+8192;

		contend_pages_actual[0]=0;
		contend_pages_actual[1]=contend_pages_128k_p2a[pagina1];
		contend_pages_actual[2]=contend_pages_128k_p2a[pagina2];
		contend_pages_actual[3]=contend_pages_128k_p2a[pagina3&7];

		debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+pagina0;
		debug_paginas_memoria_mapeadas[1]=pagina1;
		debug_paginas_memoria_mapeadas[2]=pagina2;
		debug_paginas_memoria_mapeadas[3]=pagina3;


	}

	//Sin bootm
	else {


	//Si hay habilitado paginacion timex/chloe
	if (zxuno_is_chloe_mmu ()) {
		//printf ("Usando chloe mmu\n");
		chloe_set_memory_pages();

		int i;
		for (i=0;i<8;i++) {
			zxuno_memory_paged_brandnew[i]=chloe_memory_paged[i];
		}

		return;
	}

		//Modo +2A
		//Si modo de rom en ram
		if (puerto_8189 & 1) {
			debug_printf (VERBOSE_DEBUG,"Paging RAM in ROM");
			zxuno_set_memory_pages_ram_rom();
		}

		else {
			//Modo normal

			//Los 16kb de rom del zxuno
			pagina0=zxuno_get_rom_page();
			pagina1=5;
			pagina2=2;
			pagina3=zxuno_get_ram_page();


            //0-1fffh      (0-8191)
			zxuno_memory_paged_brandnew[0*2]=zxuno_sram_mem_table_new[pagina0+8];
            //2000h-3fffh  (8192-16383)
			zxuno_memory_paged_brandnew[0*2+1]=zxuno_sram_mem_table_new[pagina0+8]+8192;
			//En la tabla zxuno_sram_mem_table hay que saltar las 8 primeras, que son las 8 rams del modo 128k

            //4000h-5fffh  (16384-24575)
            //Si habilitado modo prism
            if (zxuno_is_prism_mapping_enabled()) {
                zxuno_memory_paged_brandnew[1*2]=zxuno_get_vram_mapped_address();
            }
            else { 
			    zxuno_memory_paged_brandnew[1*2]=zxuno_sram_mem_table_new[pagina1];
            }


            //6000h-7fffh  (24576-32767)
			zxuno_memory_paged_brandnew[1*2+1]=zxuno_sram_mem_table_new[pagina1]+8192;

            //8000h-9fffh  (32768-40959)
			zxuno_memory_paged_brandnew[2*2]=zxuno_sram_mem_table_new[pagina2];

            //A000h-bfffh  (40960-49151)
			zxuno_memory_paged_brandnew[2*2+1]=zxuno_sram_mem_table_new[pagina2]+8192;

            //C000h-dfffh  (49152-57343)
			zxuno_memory_paged_brandnew[3*2]=zxuno_sram_mem_table_new[pagina3];

            //E000h-ffffh  (57344-65535)
			zxuno_memory_paged_brandnew[3*2+1]=zxuno_sram_mem_table_new[pagina3]+8192;

			contend_pages_actual[0]=0;
			contend_pages_actual[1]=contend_pages_128k_p2a[pagina1];
			contend_pages_actual[2]=contend_pages_128k_p2a[pagina2];
			contend_pages_actual[3]=contend_pages_128k_p2a[pagina3&7];

			debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+pagina0;
			debug_paginas_memoria_mapeadas[1]=pagina1;
			debug_paginas_memoria_mapeadas[2]=pagina2;
			debug_paginas_memoria_mapeadas[3]=pagina3;


		}
	}


}


z80_byte zxuno_get_radaspalbank_offset(void)
{
	/*

	¿Se pueden usar distintos bancos de paleta en modo radastan?
	Sí. Para ello se puede usar, desde el core EXP25, el registro RADASPALBANK, el numero 0x43 (67). Se usan los bits 2,1 y 0.

La paleta de ULAPlus tiene 64 colores y a priori en el modo radastan podemos usar los 16 primeros, 
con los bits 1 y 0 indicamos cual de los cuatro bancos de 16 colores de entre los 64 totales se usará, 
si los 16 primeros (si valen 00), los 16 siguientes si vale 01, los 16 terceros si vale (10) o los 16 últimos si vale (00). 
Por defecto es 00 y por eso se usan los 16 primeros.

El bit 2 indica que mitad de la paleta del modo radastan se usará para fijar el color de borde, 
indicando un 0 que se usa las primeras 8 entradas del bloque de 16 seleccionado, y un 1 las últimas 8. 
Así un BORDER 7 usará el color 7 de la paleta si el bit es 0, y el color 15 si el bit 2 vale 1, 
pero si hemos cambiado a otro banco de 16 colores, en lugar del 7 o el 15 un BORDER 7 usará el color 7 del bloque actual, 
o el color 15 del bloque actual. Por ejemplo, si los tres bits valen 1 (111) un BORDER 7 usará la entrada 63 de la paleta.

Notese que utilizando la interrupción raster y este registro al tiempo, es posible conseguir 
hasta 64 colores en pantalla (cambiando de bloque de paleta cada vez que llega la interrupción)
*/
	return (zxuno_ports[0x43]&3)*16;

}



z80_byte zxuno_uartbridge_readdata(void)
{

	return uartbridge_readdata();
}


void zxuno_uartbridge_writedata(z80_byte value)
{

	uartbridge_writedata(value);


}

z80_byte zxuno_uartbridge_readstatus(void)
{
	//No dispositivo abierto
	if (!uartbridge_available()) return 0;

	
	int status=chardevice_status(uartbridge_handler);
	

	z80_byte status_retorno=0;

	if (status & CHDEV_ST_RD_AVAIL_DATA) status_retorno |= ZXUNO_UART_BYTE_RECEIVED_BIT;

	return status_retorno;
}


//Retorna puntero a vram de los modos Prism
z80_byte *zxuno_get_vram_address(int vram)
{
    //if (vram==0) return zxuno_sram_mem_table_new[5];

    //else {
        int offset=vram*8192;
        return &zxuno_begin_vram0_pointer[offset];
    //}
}

int zxuno_is_prism_mode_enabled(void)
{
    return (zxuno_ports[0x50]&128);
}

int zxuno_is_prism_mapping_enabled(void)
{
    return (zxuno_ports[0x50]&64);
}

//Retorna numero de vram que entra en segmento 16384-22527 si modo prism activo
//En este caso es el mismo valor que la rom que entra del modo +2A
int zxuno_get_vram_mapped(void)
{
    int numero_vram=zxuno_get_rom_page();
    //printf("vram mapeada: %d\n",numero_vram);
    return numero_vram;
}

//Retorna puntero a vram mapeada
z80_byte *zxuno_get_vram_mapped_address(void)
{
    return zxuno_get_vram_address(zxuno_get_vram_mapped());
}

int zuxno_prism_default_palette[16]={
    /*
    Paleta por defecto
IGRB  color      puro    real
0000  negro      000000  060800
0001  azul       0000C0  0D13A7
0010  rojo       C00000  BD0707
0011  magenta    C000C0  C312AF
0100  verde      00C000  07BA0C
0101  cyan       00C0C0  0DC6B4
0110  amarillo   C0C000  BCB914
0111  blanco     C0C0C0  C2C4BC
1000  negro b    606060  64665E
1001  azul b     0000FF  161CB0
1010  rojo b     FF0000  CE1818
1011  magenta b  FF00FF  DC2CC8
1100  verde b    00FF00  28DC2D
1101  cyan b     00FFFF  36EFDE
1110  amarillo b FFFF00  EEEB46
1111  blanco b   FFFFFF  FDFFF7
*/

0x000000,
0x0000C0,  
0xC00000,  
0xC000C0,  
0x00C000,  
0x00C0C0,  
0xC0C000,  
0xC0C0C0,  
0x606060,  
0x0000FF,  
0xFF0000,  
0xFF00FF,  
0x00FF00,  
0x00FFFF,  
0xFFFF00,  
0xFFFFFF  

};



struct s_zxuno_prism_palette_item zxuno_prism_current_palette[16];

//Indica 0,1 o 2 segun si ultimo componente
//cambiado de un color es 0 (red) , 1 (green) o 2 (blue)
int zxuno_prism_index_last_palette_component=0;

void zxuno_prism_set_color_palette_15bit(int indice_color)
{
    //Por si acaso
    indice_color &=15; //mantener entre 0 y 15

    int r,g,b;



    r=zxuno_prism_current_palette[indice_color].rgb[0];
    g=zxuno_prism_current_palette[indice_color].rgb[1];
    b=zxuno_prism_current_palette[indice_color].rgb[2];


    //Convertir de 8 bits a 5 bits
    r=(r>>3)&31;
    g=(g>>3)&31;
    b=(b>>3)&31;

    int rgb15=(r<<10)|(g<<5)|b;

    zxuno_prism_current_palette[indice_color].index_palette_15bit=rgb15;

    //printf("setting zxuno prism index color %d to %04XH\n",indice_color,rgb15);
}

void zxuno_prism_set_color_palette(void)
{
    //cambia el color de la paleta segun el contenido de los registros 0x50 y 0x51
    int indice_paleta=zxuno_ports[0x50] & 15;

    int valor_color=zxuno_ports[0x51];

    //Mantener entre 0 y 2 
    zxuno_prism_index_last_palette_component = zxuno_prism_index_last_palette_component % 3;

    zxuno_prism_current_palette[indice_paleta].rgb[zxuno_prism_index_last_palette_component]=valor_color;

    zxuno_prism_set_color_palette_15bit(indice_paleta);

    //Siguiente componente
    zxuno_prism_index_last_palette_component++;

    //Mantener entre 0 y 2
    zxuno_prism_index_last_palette_component = zxuno_prism_index_last_palette_component % 3;

    //Si es 0, hemos completado rgb y por tanto, incrementar indice
    if (zxuno_prism_index_last_palette_component==0) {
        //printf("Siguiente indice color\n");
        indice_paleta++;
        indice_paleta &=15;

        z80_byte valor_registro=zxuno_ports[0x50] & 0xF0;
        valor_registro |=indice_paleta;
        zxuno_ports[0x50]=valor_registro;

    }
    //printf("indice componente: %d\n",zxuno_prism_index_last_palette_component);
    //printf("indice color: %02XH\n",zxuno_ports[0x50]);
}



//Modifica el valor de indice de 15 bit segun los componentes rgb en la estructura


void zxuno_prism_set_default_palette(void)
{
    int i;

    for (i=0;i<16;i++) {
        int color_defecto=zuxno_prism_default_palette[i];

        int r,g,b;

        r=(color_defecto>>16) & 0xFF;
        g=(color_defecto>>8 ) & 0xFF;
        b=(color_defecto    ) & 0xFF;

        //Meter componentes de 24 bits
        zxuno_prism_current_palette[i].rgb[0]=r;
        zxuno_prism_current_palette[i].rgb[1]=g;
        zxuno_prism_current_palette[i].rgb[2]=b;

        //Y ajustar el color de 15 bits
        zxuno_prism_set_color_palette_15bit(i);
    }
}

//Retorna color 4 bits teniendo en cuenta bit alto de cada byte,
//El bit 7 de byte_vram0 sera el bit 0 del color
//El bit 7 de byte_vram1 sera el bit 1 del color
//El bit 7 de byte_vram2 sera el bit 2 del color
//El bit 7 de byte_vram3 sera el bit 3 del color
int zxuno_get_prism_pixel_color(z80_byte byte_vram0,z80_byte byte_vram1,z80_byte byte_vram2,z80_byte byte_vram3)
{
    z80_byte indice_color=
            ((byte_vram0>>7)&1)  |
            ((byte_vram1>>6)&2)  |
            ((byte_vram2>>5)&4)  |
            ((byte_vram3>>4)&8)  ;


    //Obtener el valor de 15 bits de ese color de la paleta
    int rgb15=zxuno_prism_current_palette[indice_color].index_palette_15bit;

    return TSCONF_INDEX_FIRST_COLOR+rgb15;
    
        
}


//Renderizar modo zxuno prism
void zxuno_prism_screen_store_scanline_rainbow(void)
{


  if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {


    //printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

    //linea que se debe leer
    int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

    //la copiamos a buffer rainbow
    z80_int *puntero_buf_rainbow;
    //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
    int y;

    y=t_scanline_draw-screen_invisible_borde_superior;
    if (border_enabled.v==0) y=y-screen_borde_superior;

    puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];

    puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;


    int x,bit;
    z80_int direccion;

    z80_byte byte_vram0,byte_vram1,byte_vram2,byte_vram3;

    int color;

      
    direccion=screen_addr_table[(scanline_copia<<5)];


    z80_byte *vram0_pointer=zxuno_get_vram_address(0);
    z80_byte *vram1_pointer=zxuno_get_vram_address(1);
    z80_byte *vram2_pointer=zxuno_get_vram_address(2);
    z80_byte *vram3_pointer=zxuno_get_vram_address(3);


    for (x=0;x<32;x++) {
            
        byte_vram0=vram0_pointer[direccion];
        byte_vram1=vram1_pointer[direccion];
        byte_vram2=vram2_pointer[direccion];
        byte_vram3=vram3_pointer[direccion];

        for (bit=0;bit<8;bit++) {

            color=zxuno_get_prism_pixel_color(byte_vram0,byte_vram1,byte_vram2,byte_vram3);
            
            store_value_rainbow(puntero_buf_rainbow,color);
            

            byte_vram0=byte_vram0<<1;
            byte_vram1=byte_vram1<<1;
            byte_vram2=byte_vram2<<1;
            byte_vram3=byte_vram3<<1;
            
        }
        direccion++;

    }


  }


}


int zxuno_prism_get_border_color(void)
{ 
                int rgb15=zxuno_prism_current_palette[screen_border_last_color].index_palette_15bit;
                return TSCONF_INDEX_FIRST_COLOR+rgb15;  
}


//int zxuno_already_loaded_block=0;

/*
Cargar un archivo de maximo 64kb en la memoria mapeada. Se hace justo despues de inicializar la mmu y cargar la rom
De hecho esa rom se quedará sobreescrita con el bloque que indiquemos aqui
Tambien se hace despues de cada hard reset

Como probar esto?
Aunque es exclusivo de zxuno la manera facil es:

-Meter en los primeros 16kb una rom cualquiera, la de 48.rom vale. Luego agregarle una pantalla .scr.

cp 48.rom pruebabloque.raw
cat chasehq.scr >> pruebabloque.raw

-Al arrancar con : 
./zesarux --noconfigfile --machine zxuno --zxuno-initial-64k pruebabloque.raw

Se vera la pantalla un instante y se hara el reset normal la rom del spectrum 48

Logicamente esto no es un archivo de 64kb total pero para la prueba vale

Se puede tambien agregar cadenas de texto al archivo y luego visualizarlas con debug->hexadecimal editor 
(habiendo pausado antes la emulacion, claro, o se reseteara)

cat prueba.txt >> pruebabloque.raw

*/
void zxuno_load_additional_64k_block(void)
{

    debug_printf(VERBOSE_INFO,"ZX-Uno: Checking if loading additional 64kb block (parameter --zxuno-initial-64k)");

    //hay path?
    if (zxuno_initial_64k_file[0]==0) return;

    
    

    //printf("bootm=%d\n",zxuno_ports[0]);    

    //solo hacerlo una vez. aunque esto solo se llama al crear la maquina, no deberia suceder mas veces
    //if (zxuno_already_loaded_block) {
    //    debug_printf(VERBOSE_INFO,"ZX-Uno: additional 64kb block already loaded. exiting");
    //    return;
    //}

    //zxuno_already_loaded_block=1;

    debug_printf(VERBOSE_INFO,"ZX-Uno: Loading additional 64kb block from file %s",zxuno_initial_64k_file);    

    //printf("first time load block\n");

    //mapeo: rom, pagina 5, pagina 2, pagina mastermapper(0 defecto)
    //de todas maneras miro en los segmentos actuales, da igual si son esos u otros


   
    FILE *ptr_configfile;
    ptr_configfile=fopen(zxuno_initial_64k_file,"rb");


    if (!ptr_configfile) {
            debug_printf(VERBOSE_ERR,"Unable to open file %s",zxuno_initial_64k_file);
            return;
    }

    //leer en trocitos de 8kb
    int i;
    for (i=0;i<8;i++) {
        debug_printf(VERBOSE_INFO,"ZX-Uno: Loading 8kb block to segment %d",i);
        fread(zxuno_memory_paged_brandnew[i],1,8192,ptr_configfile);
    }     




    fclose(ptr_configfile);


}