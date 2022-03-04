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

#include "sam.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "contend.h"
#include "joystick.h"
#include "menu.h"
#include "operaciones.h"
#include "utils.h"
#include "audio.h"
#include "utils.h"
#include "tape.h"


z80_byte sam_vmpr;
z80_byte sam_hmpr;
z80_byte sam_lmpr;

z80_byte sam_border;

//vale 31 si sam de 512k
//vale 15 si sam de 256k
z80_byte sam_memoria_total_mascara=31;

//Direcciones actuales mapeadas para lectura, 4 bloques de 16 kb
z80_byte *sam_memory_paged[4];

//Las 32 paginas de ram
z80_byte *sam_ram_memory[32];

//Las 2 paginas de rom
z80_byte *sam_rom_memory[2];

//Tipo de paginas mapeadas. 0 si ram, 1 si rom
z80_byte sam_memory_paged_type[4];


//Paginas mapeadas en cada zona de RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
z80_byte debug_sam_paginas_memoria_mapeadas[4];


//Paleta de sam coupe
z80_byte sam_palette[16];


//Puerto teclado FFFE:
z80_byte puerto_65534=255;
/*
	D4	D3	D2	D1	D0
	RIGHT	LEFT	DOWN	UP	CTRL
*/


//Otras teclas sam coupe
z80_byte puerto_teclado_sam_fef9=255;
z80_byte puerto_teclado_sam_fdf9=255;
z80_byte puerto_teclado_sam_fbf9=255;
z80_byte puerto_teclado_sam_f7f9=255;

z80_byte puerto_teclado_sam_eff9=255;
z80_byte puerto_teclado_sam_dff9=255;
z80_byte puerto_teclado_sam_bff9=255;
z80_byte puerto_teclado_sam_7ff9=255;
/*

	D7	D6	D5 
FE 	F3	F2	F1
FD	F6	F5	F4
FB	F9	F8	F7
F7	CAPS	TAB	ESC
EF	DEL	+	-
DF	F0	"	=
BF	EDIT	:	;
7F	INV	.	,
*/


//Establecer sam_memory_paged segun paginas mapeadas
void sam_set_memory_pages(void)
{
/*
LMPR - Low Memory Page Register (250 dec)
This read/write register is used mainly for the control of paging low memory in the CPU's addressing range.
Bit 0 R/W   BCD 1 of low memory page control.

Bit 1 R/W   BCD 2 of low memory page control.

Bit 2 R/W   BCD 4 of low memory page control.

Bit 3 R/W   BCD 8 of low memory page control.

Bit 4 R/W   BCD 16 of low memory bank control.

Bit 5 R/W   RAM0 when bit set high, RAM replaces the
first half of the ROM (ie ROM0) in
section A of the CPU address map.

Bit 6 R/W ROM1 when bit set high, the second half
of the ROM (ie ROM1) replaces the
RAM in section D of the CPU address
map

Bit 7 R/W WPRAM Write Protection of the RAM in
section A of the CPU address map is
enabled when this bit is set high.



HMPR - High Memory Page Register (251 dec)
This read/write register is used mainly for the control of paging memory
in the CPU's addressing range.
      Bit 0 R/W   BCD   1 of high memory page control.

      Bit 1 R/W   BCD   2 of high memory page control.

      Bit 2 R/W   BCD   3 of high memory page control.

      Bit 3 R/W   BCD   4 of high memory page control.

      Bit 4 R/W   BCD   16 of high memory page control.

      Bit 5 R/W   MD3S0 BCD 4 of the colour look-up address
available only in mode 3.

      Bit 6 R/W   MD3S1 BCD 8 of the colour look-up address available only in mode 3.

      Bit 7 R/W   MCNTRL
(See section entitled
CLUT IN MODE 4 & MODE 3
in the text ahead)
If this bit is set when the CPU addresses high memory, then the external signal XMEM goes low and the Coupe looks on its expansion connector for memory sections C and D (addresses 32768 to 65536).	


*/


/*
LMPR manages the block A.B, and the HMPR manages the block C.D
If we write 00H to the LMPR then page 0 of the memory is allocated to section A of the CPU address range. Section B is always automatically allocated one page above section A, in this case to page 1.
If we write 02H to the HMPR then page 2 of the memory is allocated to section C of the CPU address range. Section D is always automatically allocated one page above section C, in this case to page 3.
*/

	z80_byte pagina_lmpr=sam_lmpr & sam_memoria_total_mascara;
	z80_byte pagina_hmpr=sam_hmpr & sam_memoria_total_mascara;

	//printf ("Sam coupe paginas: lmpr: %d hmpr: %d\n",pagina_lmpr,pagina_hmpr);


//z80_byte *sam_memory_paged[4];
//z80_byte *sam_ram_memory[32];
//z80_byte *sam_rom_memory[2];

	//De momento asumir todo es RAM
	int pagina0=pagina_lmpr;
	sam_memory_paged[0]=	sam_ram_memory[pagina0];
	debug_sam_paginas_memoria_mapeadas[0]= pagina0;

	int pagina1=(pagina_lmpr+1)&sam_memoria_total_mascara;
	sam_memory_paged[1]=	sam_ram_memory[pagina1];  
	debug_sam_paginas_memoria_mapeadas[1]= pagina1;

	int pagina2=pagina_hmpr;
	sam_memory_paged[2]=	sam_ram_memory[pagina2];
	debug_sam_paginas_memoria_mapeadas[2]= pagina2;

	int pagina3=(pagina_hmpr+1)&sam_memoria_total_mascara;
	sam_memory_paged[3]=	sam_ram_memory[pagina3];  
	debug_sam_paginas_memoria_mapeadas[3]= pagina3;


	//Tipo de paginas mapeadas. 0 si ram, 1 si rom
	sam_memory_paged_type[0]=sam_memory_paged_type[1]=sam_memory_paged_type[2]=sam_memory_paged_type[3]=0;


	//Ahora ver si rom esta en pagina 0 y/o 3
	if ( (sam_lmpr&32)==0) {
		sam_memory_paged[0]=sam_rom_memory[0];
		sam_memory_paged_type[0]=1;
		debug_sam_paginas_memoria_mapeadas[0]=0;
		//printf ("Rom is in page 0\n");
	}

	if (sam_lmpr&64) {
		sam_memory_paged[3]=sam_rom_memory[1];
		sam_memory_paged_type[3]=1;
		debug_sam_paginas_memoria_mapeadas[3]=1;
		//printf ("Rom is in page 3\n");
	}
}
	

void sam_init_memory_tables(void)
{
	z80_byte *puntero;
	puntero=memoria_spectrum;

	sam_rom_memory[0]=puntero;
	puntero +=16384;
	sam_rom_memory[1]=puntero;
	puntero +=16384;

	int i;
	for (i=0;i<32;i++) {
		sam_ram_memory[i]=puntero;
		puntero +=16384;
	}
}




void sam_splash_videomode_change(void) {

	z80_byte modo_video=(sam_vmpr>>5)&3;

        char mensaje[32*24];

	switch (modo_video) {
		case 0:
			sprintf (mensaje,"Setting screen mode 0, 256x192, 16 colours 8x8, compatible Spectrum");
		break;

		case 1:
			sprintf (mensaje,"Setting screen mode 1, 256x192, 16 colours 8x1");
		break;

		case 2:
			sprintf (mensaje,"Setting screen mode 2, 512x192, 4 colours per pixel");
		break;

		case 3:
			sprintf (mensaje,"Setting screen mode 3, 256x192, 16 colours per pixel");
		break;

		default:
			//Esto no deberia suceder nunca
			sprintf (mensaje,"Setting unknown vide mode");
		break;

	}

        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);

}

