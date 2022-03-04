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
#include "chrome.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"



//Direcciones donde estan cada pagina de rom. 4 paginas de 16 kb
z80_byte *chrome_rom_mem_table[4];

//Direcciones donde estan cada pagina de ram home, en paginas de 16 kb
z80_byte *chrome_ram_mem_table[10];


//Direcciones actuales mapeadas, bloques de 16 kb
z80_byte *chrome_memory_paged[4];

/*
Memory Map
Chrome memory map is similar to original 128K Speccy, but add to this other two 16K pages of ram and two of rom. Memory paging is controlled by port 7FFDh and 1FFDh, you can read these ports. Notice that only Bank 2 and 5 are contended with video CPLD.
Port 7FFDh (read/write)
Bit 0-2 RAM page (0-7) to map into memory at C000h
Bit 3 Select normal (0) or shadow (1) screen to be displayed. The normal screen is in bank 5, whilst the shadow screen is in bank 7. Note that this does not affect the memory between 0x4000 and 0x7fff, which is always bank 5.
Bit 4 Low bit of ROM selection.
Bit 5 If set, memory paging will be disabled and further output to this port will be ignored until the computer is reset.
Bit 6-7 not used

Port 1FFDh (read/write)
Bit 0 If 1 maps banks 8 or 9 at 0000h (switch off rom).
Bit 1 High bit of ROM selection and bank 8 (0) or 9 (1) if bit0 = 1.
Bit 2 if 1 maps bank9 at 4000h, video page is still displayed and can be accessed at C000h.
Bit 3 Clock frequency 3.58MHz (0), 7.1MHz (1)
Bit 4 Disable (1) floppy disk memory paging
Bit 5 If set disable Chrome features ( reading/writing to port 1FFDh, reading from port 7FFDh, i2c interface. This downgrade Chrome to a simple 128K spectrum clone)
Bit 6 I2C interface - SCL signal
Bit 7 I2C interface - SDA signal




FFFFh +--------+--------+--------+--------+--------+--------+--------+--------+
      | Bank 0 | Bank 1 | Bank 2 | Bank 3 | Bank 4 | Bank 5 | Bank 6 | Bank 7 |
      |        |        |(also at|        |        |(also at|        |        |
      |        |        | 8000h) |        |        | 4000h) |        |        |
      |        |        |        |        |        | screen |        | screen |
C000h +--------+--------+--------+--------+--------+--------+--------+--------+
      | Bank 2 |
      |        |
      |        |
      |        |
8000h +--------+--------+
      | Bank 5 | Bank 9 |
      |        |        |
      |        |        |
      | screen |        |
4000h +--------+--------+--------+--------+--------+--------+--------+
      | ROM 0  | ROM 1  | ROM 2  | ROM 3  | Bank 8 | Bank 9 | RAM +D |
      |        |        |        |        |        |        |________|2000h
      |        |        |        |        |        |        |        |
      |        |        |        |        |        |        | ROM +D |
0000h +--------+--------+--------+--------+--------+--------+--------+


*/

z80_byte chrome_get_rom_bank(void)
{


        z80_byte banco;

        banco=((puerto_32765>>4)&1);

				/*
				Port 1FFDh (read/write)
			Bit 5 If set disable Chrome features ( reading/writing to port 1FFDh, reading from port 7FFDh, i2c interface. This downgrade Chrome to a simple 128K spectrum clone)
			*/
			  if (si_chrome_features_enabled()) {
				banco +=((puerto_8189>>1)&2);
				}



        return banco;
}

z80_byte chrome_get_ram_bank_c0(void)
{
        z80_byte banco;

      	banco=puerto_32765&7;

        return banco;
}

z80_byte chrome_get_ram_bank_40(void)
{
        z80_byte banco=5;


				/*Port 1FFDh (read/write)
				Bit 0 If 1 maps banks 8 or 9 at 0000h (switch off rom).
				Bit 1 High bit of ROM selection and bank 8 (0) or 9 (1) if bit0 = 1.
				Bit 2 if 1 maps bank9 at 4000h, video page is still displayed and can be accessed at C000h.*/


				/*
				Port 1FFDh (read/write)
			Bit 5 If set disable Chrome features ( reading/writing to port 1FFDh, reading from port 7FFDh, i2c interface. This downgrade Chrome to a simple 128K spectrum clone)
			*/
				if (si_chrome_features_enabled() ) {
					if (puerto_8189&4) banco=9;
				}

        return banco;
}

int si_chrome_features_enabled(void)
{
	/*
	Port 1FFDh (read/write)
Bit 5 If set disable Chrome features ( reading/writing to port 1FFDh, reading from port 7FFDh, i2c interface. This downgrade Chrome to a simple 128K spectrum clone)
*/
	if ( (puerto_8189&32)==0) return 1;
	return 0;
}

//Retorna 0 si no esta ram 8 o 9 mapeados en 0-3fffh
//Retorna 8 o 9 segun ram mapeada ahi
int chrome_ram89_at_00(void)
{
  if (puerto_8189&1  && si_chrome_features_enabled()) {

    if (puerto_8189&1) return 9;
    else return 8;
  }

  return 0;
}

void chrome_set_memory_pages(void)
{
	z80_byte rom_page=chrome_get_rom_bank();
	z80_byte ram_page_c0=chrome_get_ram_bank_c0();
	z80_byte ram_page_40=chrome_get_ram_bank_40();


	/*
	Port 1FFDh (read/write)
	Bit 0 If 1 maps banks 8 or 9 at 0000h (switch off rom).
	Bit 1 High bit of ROM selection and bank 8 (0) or 9 (1) if bit0 = 1.
	*/
	
  z80_byte r0;
  r0=chrome_ram89_at_00();
	if (r0) {
		chrome_memory_paged[0]=chrome_ram_mem_table[r0];
		debug_paginas_memoria_mapeadas[0]=r0;
	}

	else {
		chrome_memory_paged[0]=chrome_rom_mem_table[rom_page];
		debug_paginas_memoria_mapeadas[0]=DEBUG_PAGINA_MAP_ES_ROM+rom_page;
	}



	chrome_memory_paged[1]=chrome_ram_mem_table[ram_page_40];
	chrome_memory_paged[2]=chrome_ram_mem_table[2];
	chrome_memory_paged[3]=chrome_ram_mem_table[ram_page_c0];


	        debug_paginas_memoria_mapeadas[1]=ram_page_40;
	        debug_paginas_memoria_mapeadas[2]=2;
	        debug_paginas_memoria_mapeadas[3]=ram_page_c0;

}


void chrome_init_memory_tables(void)
{
	debug_printf (VERBOSE_DEBUG,"Initializing Chrome memory pages");

	//memoria_spectrum
	//64 kb rom
	//160 kb ram

	z80_byte *puntero;
	puntero=memoria_spectrum;

	int i;
	for (i=0;i<4;i++) {
		chrome_rom_mem_table[i]=puntero;
		puntero +=16384;
	}

	for (i=0;i<10;i++) {
		chrome_ram_mem_table[i]=puntero;
		puntero +=16384;
	}

	//Tablas contend
	contend_pages_actual[0]=0;
	contend_pages_actual[1]=contend_pages_chrome[5];
	contend_pages_actual[2]=contend_pages_chrome[2];
	contend_pages_actual[3]=contend_pages_chrome[0];


}
