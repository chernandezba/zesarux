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

#include "cpu.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "zxuno.h"
#include "chloe.h"
#include "prism.h"
#include "timex.h"
#include "cpc.h"
#include "sam.h"
#include "ula.h"
#include "tbblue.h"
#include "superupgrade.h"
#include "chrome.h"
#include "tsconf.h"
#include "baseconf.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "svi.h"
#include "if1.h"

//Direcciones donde estan cada pagina de ram
//Antes habian 8 solo (8 paginas de 16kb cada una)
//Ahora hay 64 (para un maximo de 1024 kb)
//z80_byte *ram_mem_table[8];
z80_byte *ram_mem_table[64];

//Direcciones donde estan cada pagina de rom
//array para +2a usamos 4 elementos
//para 128, +2 usamos 2 elementos
z80_byte *rom_mem_table[4];

//Direcciones actuales mapeadas
z80_byte *memory_paged[4];

z80_byte puerto_32765=0;
z80_byte puerto_8189=0;

//Si hay mas de 128kb para maquinas tipo 128k y +2a
//Si vale 1, solo 128k
//Si vale 2, hay 256 kb
//Si vale 4, hay 512 kb
int mem128_multiplicador=1;


//Paginas mapeadas en cada zona de RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
//Si numero pagina >=32768, numero pagina=numero pagina-32768 y se trata de ROM. Si no, es RAM
//Permitir hasta 8 bloques para compatibilidad con mmu de 8 paginas, como tbblue
z80_int debug_paginas_memoria_mapeadas[8];

/*
El conmutador de hardware esta en la direccion de E/S 7FFDh (32765). El campo del bit
para esta direccion es el siguiente:
DO a D2         Seleccion de RAM
D3              Seleccion de pantalla
D4                      Seleccion de ROM
D5       Inhabilitacion de la paginacion

D2 a DO crean un numero de tres bits que selecciona que RAM entra en el hueco compren-
dido entre C000h y FFFFh. En BASIC normalmente esta seleccionada la pagina 0; duran-
te la edicion y el acceso a +3DOS se usa la pagina 7 para almacenar diversos tampones
y memorias transitorias.
D3 conmuta pantallas; la pantalla 0 est en la pagina 5 de la
RAM (que normalmente empieza en 4000h) y es la que utiliza BASIC; la pantalla 1 esta
en la pagina 7 (a partir de COOOh) y solo puede ser usada por programas de codigo de ma-
quina. Es perfectamente factible preparar una pantalla en la pagina 7 y despues <AB>descon-
mutarla<BB>; de esta forma se deja los 48K libres para datos y programa. (Tengase en cuenta
que la orden de copia de ficheros, COPY, puede crear tampones en la zona de la segunda
pantalla, y por lo tanto destruir la imagen que hubieramos almacenado en ella.)
D4 determina, junto con el bit 2 de la puerta 1FFDh, que ROM debe ser colocada en las direcciones
0000h a 3FFFh. D5 es un dispositivo de seguridad; en cuanto se pone a 1 este bit, quedan
imposibilitadas todas las operaciones de paginacion. Este sistema es el utilizado cuando
el ordenador adopta la configuracion del Spectrum de 48K estandar y los circuitos de pagi-
nacion de memoria estan bloqueados. A partir de ese momento no se lo puede volver a
convertir en un ordenador de 128K mas que apagandolo o pulsando el boton REINIC;
no obstante, el circuito de sonido puede seguir siendo controlado por OUT. Si se carga
desde el disco un programa de juego dise<F1>ado para el Spectrum de 48K y no funciona,
quiza lo haga si se da la orden SPECTRUM y luego OUT 32765,48 (que bloquea el bit
5 en esta puerta).

El +2A utiliza la puerta de E/S 1FFDh para realizar ciertas operaciones de conmutacion
de ROM y RAM. El campo del bit para esta direccion es el siguiente:
D0                      Decide si D1 y D2 afectan a la ROM o a la RAM    (1)
D1 y D2         Conmutacion de ROM/RAM                           (2)
D4                      Motor del disco
D5                      Se<F1>al STROBE en la puerta paralelo (activa a nivel bajo)

(1) Nota: En el manual se menciona erroneamente como bit D3
(2) Nota: En el manual se mencionan erroneamente como bits D0 y D1

Cuando el bit 0 esta a 0, el bit 1 es indiferente y el bit 2 constituye un conmutador 'vertical'
para la ROM (que elige entre ROM 0 y ROM 2, o entre ROM 1 y ROM 3). El bit 4 de
la puerta 7FFDh es un conmutador 'horizontal' para la ROM (que elige entre ROM 0 y
ROM 1, o entre ROM 2 y ROM 3). El siguiente diagrama ilustra las diferentes posibilidades:

 ----------         Bit 4 de 7FFDh (23388)         ------------
 | ROM 0  |       variable de sistema: BANKM)    |      ROM 1    |
 |        |    <-     horizontal       ->        |               |
 | Editor |                                      |  Sintaxis     |
 ----------                                        ------------
          ^                                                ^
          |                                                |

Bit 2 de 1FFDh (23399)
(variable de sistema: BANK678)
        vertical                                        vertical

        |                                                 |
        v                                                 v

  ----------                                         ------------
 |  ROM 2   |                                       |    ROM 3   |
 |          |   <-       horizontal        ->       |            |
 |   DOS    |                                       |   48 BASIC |
  ----------                                         ------------

                Conmutacion horizontal y vertical de la ROM

Es util imaginar que el bit 4 de la puerta 7FFDh y el bit 2 de la puerta 1FFDh se combinan
para formar un numero de dos bits (margen de 0 a 3) que determina que ROM entra en
la zona de 0000h a 3FFFh. Al formar ese numero, el bit 4 de la puerta 7FFDh seria el
menos significativo, y el otro el mas significativo.


Bit 2 de 1FFDh                 | Bit 4 de 7FFDh              |      ROM que entra en
(Variable de sistema: BANK678) | (Variable de sistema: BANKM)|  0000h-3FFFh
-------------------------------|-----------------------------|-----------------
0                              |     0                       |      0
0                              |     1                       |      1
1                              |     0                       |      2
1                              |     1                       |      3

                Conmutacion de ROM (con el bit 0 de 1FFDh puesto a 0)
Cuando el bit 0 de la puerta 1FFDh esta a 1, los bits 1 y 2 controlan que combinacion
de paginas de RAM ocupan los 64K posibles. Estas combinaciones no son utilizadas por
BASIC, pero pueden serlo por los autores de sistemas operativos y programas de juegos.
Cuando se invoca la rutina 'DOS CARGAR', el cargador inicial es transferido al <AB>entor-
no<BB> de las paginas 4, 7, 6, 3. Las opciones de paginacion de RAM del +2A son:

Bit 2 de 1FFDh          |               Bit 1 de 1FFDh          |               Paginas de RAM que
                                                |                                                               |                       entran en
                                                |                                                               |                       0000h-3FFFh,
                                                |                                                               |                       4000h-4FFFh, etc.
------------------|-----------------------|--------------------------

                                                |                       |
                0                               |                               0                               |                       0,1,2,3
                0                               |                               1                               |                       4,5,6,7
                1                               |                               0                               |                       4,5,6,3
                1                               |                               1                               |                       4,7,6,3



                Conmutacion de paginas de RAM (con el bit 0 de 1FFDh puesto a 1)

*/





z80_byte *get_base_mem_pantalla_continue(void)
{

	if (superupgrade_enabled.v) return superupgrade_ram_memory_table[5];

	if (MACHINE_IS_SPECTRUM_16_48) return &memoria_spectrum[16384];

	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {

		if (puerto_32765 & 8) {
			return ram_mem_table[7];
		}
		else return ram_mem_table[5];
	}

	if (MACHINE_IS_ZXUNO) {

	                if (puerto_32765 & 8) {
        	                return zxuno_sram_mem_table_new[7];
                	}
	                else return zxuno_sram_mem_table_new[5];
	}

        if (MACHINE_IS_CHLOE) {

                if (puerto_32765 & 8) {
                        return chloe_home_ram_mem_table[7];
                }
                else return chloe_home_ram_mem_table[5];
        }


      if (MACHINE_IS_CHROME) {

                if (puerto_32765 & 8) {
                        return chrome_ram_mem_table[7];
                }
                else return chrome_ram_mem_table[5];
        }
			if (MACHINE_IS_BASECONF) {



					if (puerto_32765 & 8) {
						return baseconf_ram_mem_table[7];
					}
					else return baseconf_ram_mem_table[5];

				}

				if (MACHINE_IS_TSCONF) {

					//TODO pantalla shadow 128

                                        z80_byte vram_page=tsconf_get_vram_page();
										//printf ("vpage: %02XH\n",vram_page);
                                        return tsconf_ram_mem_table[vram_page];

					/*

					if (puerto_32765 & 8) {
						return tsconf_ram_mem_table[7];
					}
					else return tsconf_ram_mem_table[5];
					*/
				}


        if (MACHINE_IS_TBBLUE) {

		//z80_byte maquina=(tbblue_config1>>6)&3;
		z80_byte maquina=(tbblue_registers[3])&3;
		if (maquina==0) {
			//modo s_config
			return tbblue_memory_paged[1*2];
		}

		else {

			if (maquina==1) {
				//48kb
				return tbblue_ram_memory_pages[5*2];
			}


	                if (puerto_32765 & 8) {
        	                return tbblue_ram_memory_pages[7*2];
                	}
	                else return tbblue_ram_memory_pages[5*2];

		}
        }

        if (MACHINE_IS_PRISM) {

                if (puerto_32765 & 8) {
                        //return prism_ram_mem_table[7*2];
			return prism_vram_mem_table[2]; //VRAM2
                }
                //else return prism_ram_mem_table[5*2];
		else return prism_vram_mem_table[0]; //VRAM0
        }


	if (MACHINE_IS_TIMEX_TS_TC_2068) {
		return timex_home_ram_mem_table[0];
	}


	//esto no seria necesario. lo hacemos solo porque de momento el real video al activarlo peta con cpc porque entra aqui, y si no metemos esto, genera cpu_panic
	//tambien hace falta para que no pete automatic redraw en stdout
	if (MACHINE_IS_CPC) {
		return cpc_ram_mem_table[0];
	}

    //TODO
	if (MACHINE_IS_PCW) {
		return memoria_spectrum;
	}

	if (MACHINE_IS_SAM) {
		return sam_ram_memory[0];
	}

	if (MACHINE_IS_MK14) {
		return memoria_spectrum;
	}

	if (MACHINE_IS_QL) {
		return memoria_spectrum;
	}

	if (MACHINE_IS_MSX) {
		return msx_vram_memory;
	}

	if (MACHINE_IS_SVI) {
		return svi_vram_memory;
	}

	if (MACHINE_IS_COLECO) {
		return coleco_vram_memory;
	}

	if (MACHINE_IS_SG1000) {
		return sg1000_vram_memory;
	}

	if (MACHINE_IS_SMS) {
		return sms_vram_memory;
	}

	cpu_panic("get_base_mem_pantalla on this machine has no sense");

	//aunque aqui no se llega, lo hacemos para que no se queje el compilador
	return 0;

}

z80_byte *get_base_mem_pantalla(void)
{
	z80_byte *screen;

	screen=get_base_mem_pantalla_continue();

	//Incrementar 8192 bytes en caso de pantalla timex pero no para prism
	if (!MACHINE_IS_PRISM) {
		if (timex_si_modo_shadow()) {
			return screen+8192;
		}
	}

	return screen;
}


//Para obtener direccion atributos. normalmente esta siempre igual que resto pantalla,
//excepto modo timex 8x1 que esta 8192 bytes mas adelante
z80_byte *get_base_mem_pantalla_attributes(void)
{
        z80_byte *screen;

        screen=get_base_mem_pantalla();

	//Incrementar 8192 bytes en caso de pantalla timex 8x1 para atributos pero no para prism
        if (!MACHINE_IS_PRISM) {
                if (timex_si_modo_8x1()) {
                        return screen+8192;
                }
        }

        return screen+6144;
}



void mem_page_ram_rom(void)
{
	z80_byte page_type;

	page_type=(puerto_8189 >>1) & 3;

	switch (page_type) {
		case 0:
			//debug_printf (VERBOSE_DEBUG,"Pages 0,1,2,3");
                	memory_paged[0]=ram_mem_table[0];
	                memory_paged[1]=ram_mem_table[1];
	                memory_paged[2]=ram_mem_table[2];
	                memory_paged[3]=ram_mem_table[3];

			contend_pages_actual[0]=contend_pages_128k_p2a[0];
			contend_pages_actual[1]=contend_pages_128k_p2a[1];
			contend_pages_actual[2]=contend_pages_128k_p2a[2];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];

			debug_paginas_memoria_mapeadas[0]=0;
			debug_paginas_memoria_mapeadas[1]=1;
			debug_paginas_memoria_mapeadas[2]=2;
			debug_paginas_memoria_mapeadas[3]=3;

		break;

		case 1:
			//debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");
                	memory_paged[0]=ram_mem_table[4];
	                memory_paged[1]=ram_mem_table[5];
	                memory_paged[2]=ram_mem_table[6];
	                memory_paged[3]=ram_mem_table[7];


                        contend_pages_actual[0]=contend_pages_128k_p2a[4];
                        contend_pages_actual[1]=contend_pages_128k_p2a[5];
                        contend_pages_actual[2]=contend_pages_128k_p2a[6];
                        contend_pages_actual[3]=contend_pages_128k_p2a[7];

                        debug_paginas_memoria_mapeadas[0]=4;
                        debug_paginas_memoria_mapeadas[1]=5;
                        debug_paginas_memoria_mapeadas[2]=6;
                        debug_paginas_memoria_mapeadas[3]=7;



		break;

		case 2:
			//debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,3");
                	memory_paged[0]=ram_mem_table[4];
	                memory_paged[1]=ram_mem_table[5];
	                memory_paged[2]=ram_mem_table[6];
	                memory_paged[3]=ram_mem_table[3];

                        contend_pages_actual[0]=contend_pages_128k_p2a[4];
                        contend_pages_actual[1]=contend_pages_128k_p2a[5];
                        contend_pages_actual[2]=contend_pages_128k_p2a[6];
                        contend_pages_actual[3]=contend_pages_128k_p2a[3];

                        debug_paginas_memoria_mapeadas[0]=4;
                        debug_paginas_memoria_mapeadas[1]=5;
                        debug_paginas_memoria_mapeadas[2]=6;
                        debug_paginas_memoria_mapeadas[3]=3;


		break;

		case 3:
			//debug_printf (VERBOSE_DEBUG,"Pages 4,7,6,3");
                	memory_paged[0]=ram_mem_table[4];
	                memory_paged[1]=ram_mem_table[7];
	                memory_paged[2]=ram_mem_table[6];
	                memory_paged[3]=ram_mem_table[3];

                        contend_pages_actual[0]=contend_pages_128k_p2a[4];
                        contend_pages_actual[1]=contend_pages_128k_p2a[7];
                        contend_pages_actual[2]=contend_pages_128k_p2a[6];
                        contend_pages_actual[3]=contend_pages_128k_p2a[3];

                        debug_paginas_memoria_mapeadas[0]=4;
                        debug_paginas_memoria_mapeadas[1]=7;
                        debug_paginas_memoria_mapeadas[2]=6;
                        debug_paginas_memoria_mapeadas[3]=3;


		break;

	}
}

void mem_set_normal_pages_128k(void)
{


                memory_paged[0]=rom_mem_table[0];
                memory_paged[1]=ram_mem_table[5];
                memory_paged[2]=ram_mem_table[2];
                memory_paged[3]=ram_mem_table[0];


		contend_pages_actual[0]=0;
		contend_pages_actual[1]=contend_pages_128k_p2a[5];
		contend_pages_actual[2]=contend_pages_128k_p2a[2];
		contend_pages_actual[3]=contend_pages_128k_p2a[0];

                debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+0;
                debug_paginas_memoria_mapeadas[1]=5;
                debug_paginas_memoria_mapeadas[2]=2;
                debug_paginas_memoria_mapeadas[3]=0;

}

void mem_set_normal_pages_p2a(void)
{

                memory_paged[0]=rom_mem_table[0];
                memory_paged[1]=ram_mem_table[5];
                memory_paged[2]=ram_mem_table[2];
                memory_paged[3]=ram_mem_table[0];

                contend_pages_actual[0]=0;
                contend_pages_actual[1]=contend_pages_128k_p2a[5];
                contend_pages_actual[2]=contend_pages_128k_p2a[2];
                contend_pages_actual[3]=contend_pages_128k_p2a[0];

                debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+0;
                debug_paginas_memoria_mapeadas[1]=5;
                debug_paginas_memoria_mapeadas[2]=2;
                debug_paginas_memoria_mapeadas[3]=0;


}


void mem_page_ram_p2a(void)
{
        //asignar ram
	mem_page_ram_128k();

}

void mem_page_rom_p2a(void)
{

	if (ula_disabled_rom_paging.v) return;

                        //asignar rom
                        z80_byte rom_entra=((puerto_32765>>4)&1) + ((puerto_8189>>1)&2);
                        memory_paged[0]=rom_mem_table[rom_entra];
                        //printf ("Entra rom: %d\n",rom_entra);

			contend_pages_actual[0]=0;
			debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+rom_entra;
}

//En maquinas 128k, devuelve bits 3 bajos de puerto 32765
//Si se pasa a 256k o 512k, se usan tambien bits 6 y 7
//Bits 0-2: 3 bits bajos de numero de pagina que entra
//Bit 6: Bit 3 de pagina que entra
//Bit 7: Bit 4 de pagina que entra
z80_byte mem_get_ram_page(void)
{

	//printf ("Valor 32765: %d\n",puerto_32765);

	z80_byte ram_entra=puerto_32765&7;

	z80_byte bit3=0;
	z80_byte bit4=0;
	z80_byte bit5=0;

	if (mem128_multiplicador==2 || mem128_multiplicador==4 || mem128_multiplicador==8) {
		bit3=puerto_32765&64;  //Bit 6
		//Lo movemos a bit 3
		bit3=bit3>>3;
	}

  if (mem128_multiplicador==4 || mem128_multiplicador==8) {
      bit4=puerto_32765&128;  //Bit 7
      //Lo movemos a bit 4
      bit4=bit4>>3;
  }

	if (mem128_multiplicador==8) {
		bit5=puerto_32765&32;  //Bit 5 tal cual
	}

	ram_entra=ram_entra|bit3|bit4|bit5;

	//en pentagon 1024, puerto eff7 bit 2 puede forzar 128kb
	/*
	From:
	https://zx-pk.ru/archive/index.php/t-11490.html
	Pentagon 1024 kB
port 7FFD: (adressation 01xxxxxx xxxxxx0x )
D0 = bank 0 ;128 kB memory
D1 = bank 1 ;128 kB memory
D2 = bank 2 ;128 kB memory
D3 = videoram
D4 = rom
D5 = bank 5 ;1024 kB memory (if D2 of port EFF7=0)
D6 = bank 3 ;256 kB memory
D7 = bank 4 ;512 kB memory
port EFF7: (adressation 1110xxxx xxxx0xxx )
D2 = 1 - set 128 kB mode
0 - enable 1MB memory
(if D2 of port EFF7=1 then D5 of port 7FFD is used for disable paging)
D3 = 1 - disable rom and connect ram page 0 in adress space 0-3FFF
	*/

	if (MACHINE_IS_PENTAGON && mem128_multiplicador==8 && (puerto_eff7 & 4) ) {
	   ram_entra &= 7;
	}


	//printf ("ram entra: %d\n",ram_entra);

	return ram_entra;
}

void mem_set_multiplicador_128(z80_byte valor)
{
	if (valor==1 || valor==2 || valor==4 || valor==8) {
		mem128_multiplicador=valor;
	}
	else {
		cpu_panic("128k multiplier ram value invalid");
	}
}



void mem_page_ram_128k(void)
{

	if (ula_disabled_ram_paging.v) return;

	//z80_byte ramentra=puerto_32765&7;
	z80_byte ramentra=mem_get_ram_page();
        //asignar ram
	memory_paged[3]=ram_mem_table[ramentra];

	contend_pages_actual[3]=contend_pages_128k_p2a[ramentra];
	debug_paginas_memoria_mapeadas[3]=ramentra;


	//printf ("ram paginada: %d\n",ramentra);
}



void mem_page_rom_128k(void)
{

	if (ula_disabled_rom_paging.v) return;

	//asignar rom
	//memory_paged[0]=rom_mem_table[(puerto_32765>>4)&1];
	z80_byte rom_entra=(puerto_32765>>4)&1;
	memory_paged[0]=rom_mem_table[rom_entra];

	contend_pages_actual[0]=0;
	debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+rom_entra;
}


int get_actual_rom_p2a(void)
{
	return ((puerto_32765>>4)&1) + ((puerto_8189>>1)&2);
}

int get_actual_rom_128k(void)
{
	return (puerto_32765>>4)&1;
}


void mem128_p2a_write_page_port(z80_int puerto, z80_byte value)
{
		//Puerto tipicamente 32765
		// the hardware will respond only to those port addresses with bit 1 reset, bit 14 set and bit 15 reset (as opposed to just bits 1 and 15 reset on the 128K/+2).
	        if ( (puerto & 49154) == 16384 ) {
			//ver si paginacion desactivada
			//if (puerto_32765 & 32) return;

			if (!mem_paging_is_enabled()) return;

			puerto_32765=value;

			//si modo de paginacion ram en rom, volver
			if ((puerto_8189&1)==1) {
				//printf ("Ram in ROM enabled. So RAM paging change with 32765 not allowed. out value:%d\n",value);
				//Livingstone supongo II hace esto, continuamente cambia de Screen 5/7 y ademas cambia
				//formato de paginas de 4,5,6,3 a 4,7,6,3 tambien continuamente
				//Hay que tener en cuenta que pese a que no hace cambio de paginacion,
				//si que permite cambiar de video shadow 5/7 ya que el puerto_32765 ya se ha escrito antes de entrar aqui
				return;
			}

	               //64 kb rom, 128 ram

                        //asignar ram
			mem_page_ram_p2a();

                        //asignar rom
			mem_page_rom_p2a();

			return;
		}

		//Puerto tipicamente 8189

		// the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).
		if ( (puerto & 61442 )== 4096) {
			//ver si paginacion desactivada
			if (puerto_32765 & 32) return;

			//Modo paginacion especial RAM en ROM
			if (value & 1) {
				puerto_8189=value;
				debug_printf (VERBOSE_DEBUG,"Paging RAM in ROM");
				mem_page_ram_rom();

				return;
			}

			else {
				//valor de puerto indica paginado normal
				//miramos si se vuelve de paginado especial RAM in ROM
				if ((puerto_8189&1)==1) {
					debug_printf (VERBOSE_DEBUG,"Going back from paging RAM in ROM");
					mem_set_normal_pages_p2a();
		                        //asignar ram
                		        mem_page_ram_p2a();
				}
				puerto_8189=value;

	                        //asignar rom
				mem_page_rom_p2a();

				//printf ("temp. paging rom value: %d\n",value);

        	                return;
			}
		}
}

void mem_init_memory_tables_128k(void)
{

                int puntero=0;
                int i;
                for (i=0;i<2;i++) {
                        rom_mem_table[i]=&memoria_spectrum[puntero];
                        puntero +=16384;
                }

				//1 MB
                for (i=0;i<64;i++) {
                        ram_mem_table[i]=&memoria_spectrum[puntero];
                        puntero +=16384;
                }
}


void mem_init_memory_tables_p2a(void)
{

                int puntero=0;
                int i;
                for (i=0;i<4;i++) {
                        rom_mem_table[i]=&memoria_spectrum[puntero];
                        puntero +=16384;
                }

				//1 MB
                for (i=0;i<64;i++) {
                        ram_mem_table[i]=&memoria_spectrum[puntero];
                        puntero +=16384;
                }
}


int mem_paging_is_enabled(void)
{
	//Si emulamos 1024 KB, paginacion siempre activa, excepto pentagon 1024 con bit 2 puerto eff7
	if (mem128_multiplicador==8) {
	  if (MACHINE_IS_PENTAGON) {
	    if ((puerto_eff7 & 4)==0) {
	      //pentagon con bit 2 de eff7 a cero  paginamos 1024 y por tanto no hay bloqueo de paginacion
	      return 1;
	    }
	    //pentagon 1024 pero bloqueado a 128kb
	    //dejar que siga al ultimo if
	  }

	  //no es pentagon
	  else {
	    return 1;
	  }

	}

	if ((puerto_32765 & 32)==0) return 1;
	else return 0;
}


//Dice en maquina spectrum (16/48, 128/+2 o +2a) si la rom paginada es la del basic
int if_spectrum_basic_rom_paged_in(void)
{
    if (MACHINE_IS_SPECTRUM_16_48) {
        //Excepcion si interface 1 activado y mapeado
        if (if1_enabled.v) {
            if (if1_rom_paged.v) return 0;
        }

        //maquina 16k, inves ,48k o tk
        return 1;
    }

    if (MACHINE_IS_SPECTRUM_128_P2)  {

        //maquina 128k. rom 1 mapeada
        if ((puerto_32765 & 16) ==16) {
            //Excepcion si interface 1 activado y mapeado
            if (if1_enabled.v) {
                if (if1_rom_paged.v) return 0;
            }
            return 1;
        }
        else return 0;
    }


    if (MACHINE_IS_SPECTRUM_P2A_P3) {
        //maquina +2A
        if ((puerto_32765 & 16) ==16   && ((puerto_8189&4) ==4  )) return 1;
        else return 0;
    }

    return 0;

}