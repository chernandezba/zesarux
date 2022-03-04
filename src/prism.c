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
#include "prism.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "timex.h"
#include "utils.h"
#include "menu.h"



//Direcciones donde estan cada pagina de rom. 256 paginas de 16 kb
z80_byte *prism_rom_mem_table[256];

//Direcciones donde estan cada pagina de ram total, en paginas de 8 kb
z80_byte *prism_ram_mem_table[64];

//Direcciones donde estan cada pagina de ram ex, en paginas de 8 kb
z80_byte *prism_ex_ram_mem_table[8];

//Direcciones donde estan cada pagina de ram dock, en paginas de 8 kb
z80_byte *prism_dock_ram_mem_table[8];

//Direcciones actuales mapeadas, bloques de 8 kb
z80_byte *prism_memory_paged[8];


//Direcciones de las 4 VRAM
z80_byte *prism_vram_mem_table[4];


//Tipos de memoria mapeadas
//0=rom
//1=home
//2=dock
//3=ex
//Constantes definidas en PRISM_MEMORY_TYPE_ROM, _HOME, _DOCK, _EX
z80_byte prism_type_memory_paged[8];


//Paginas mapeadas en cada zona de RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
z80_byte debug_prism_paginas_memoria_mapeadas[8];





//Modo failsave
z80_bit prism_failsafe_mode={0};



//Puntero a failsafe rom
z80_byte *prism_failsafe_rom;

//puerto 60987
z80_byte prism_rom_page;

/*
0x9E3B (write) - ULA2's 256 colour BORDER

The border is set to this colour when the "BORDER 0" command has been issued (BORDER 1, BORDER 2 etc all work as expected on a normal Spectrum). This register defaults to '0' so Spectrum software setting a black border acts as expected unless this registe is explicitly changed by the user/software.
*/

//z80_byte prism_ula2_border_colour;

//prism_ula2_border_colour se debe sustituir por prism_ae3b_registers[0];

z80_byte get_prism_ula2_border_colour(void)
{
	return prism_ae3b_registers[0];
}


//Guardar aqui cada cambio en el out, tal y como se hace con border normal
z80_int prism_ula2_border_colour_buffer[MAX_PRISM_BORDER_BUFFER+MAX_STATES_LINE];


/*
0xBE3B ULA2 Palette control - Colour #

Output a logical colour number to this port to select it for definition.

*/
z80_byte prism_ula2_palette_control_colour;


/*
0xFE3B (write) Palette control - RGB Data

Write 3 times to redefine red then green then blue levels for the colour selected with 0xBE3B. Accepts an 8 bit value for each colour element (different implementations of ULA2 may resample these values to lower bit-depths depending on the hardware - Prism converts this to 4 bits per element for example).

After the 3rd value has been written, the colour selected for redefinition increments to reduce the number of OUTs needed to redefine consecutive colours.
*/

z80_byte prism_ula2_palette_control_index=0;
z80_byte prism_ula2_palette_control_rgb[3];


/*

Paginacion de VRAM.
Hay 4 paginas de VRAM de 8 kb cada una, independientes de la SRAM

En los dos segmentos C000 y E000 , al paginar paginas 10,11, 14 o 15, entran paginas de VRAM, y NO de SRAM

By DEFAULT ... at boot time, when the "16K VRAM aperture" register is unset, then page 10 is between 5B00 and 5FFF

(in Prism, it's bytes 1B00 - 1FFF of page 10)

page 11 is between page 6000 and 7FFF

y VRAM0 esta entre 4000H y 5affH


y,
when "16K vram aperture" is turned on, then VRAM0 is at 0x4000 - 0x5FFF and VRAM1 is at 0x6000 - 0x7FFF

*/


/*
8E3B (36411) read - RESERVED
8E3B (36411) write - 16 x 4 bit control registers.
IO port 36411 is used to write to sixteen registers which control much of ULA2's functionality.

The register to be changed is selected by the most significant nibble of the data output to 0x8E3B, and the register's new contents is contained in the least significant nibble eg OUT 36411,BIN "00000010" changes the CPU speed to 7MHz, OUT 36411,BIN"00000000" changes it back to 3.5MHz and OUT 36411,BIN"00100011" changes the screen resolution to 512x384.
 
OUT 36411,BIN "RRRRxxxx" where RRRR is a 4-bit number selecting the register and xxxx is the value you're setting it to. This method was chosen to reduce the number of IO ports Prism uses but still give single-OUT control over the majority of Prism/ULA2 functionality.
*/

z80_byte prism_ula2_registers[16];

/*
port AE3BH (44603) (register select) and 9E3B (40507) (data) implement up to 256 x 8 bit registers  (currently 3 are implemented):
OUT 44603,register - selects register
	(OUT 44603,0 selects 256 colour border register
	OUT 44603,1 selects chunk-o mode II colour 1  register
	OUT 44603,2 selects chunk-o mode II colour 2 register)
OUT 40507,x Set register selected above to value X


Default value of colour register 0 (the one which controls the prism border colour) is BIN "00000000". 
Default vaulue of colour register 1 is "11000111"    
Default value of colour register 2 is "11000100".
*/


//Tambien llamados "8-bit registers"
z80_byte prism_ae3b_registers[256]={0,199,196};

z80_byte prism_last_ae3b;


void prism_out_9e3b(z80_byte value)
{
	prism_ae3b_registers[prism_last_ae3b]=value;

	if (prism_last_ae3b==0) {

				modificado_border.v=1;

				int i;
				i=t_estados;
	                        //printf ("t_estados %d screen_testados_linea %d bord: %d\n",t_estados,screen_testados_linea,i);

        	                //Este i>=0 no haria falta en teoria
                	        //pero ocurre a veces que justo al activar rainbow, t_estados_linea_actual tiene un valor descontrolado
                        	if (i>=0 && i<CURRENT_PRISM_BORDER_BUFFER) {

	                               prism_ula2_border_colour_buffer[i]=value;
					
        	                }
     }
                        
}


//12 bit palette values
z80_int prism_palette_zero[256];
z80_int prism_palette_two[256];

//Palette 1 0001 - Colours 0-255 calculated using the same "GGGRRRBB" formulae as when defining a ULAplus colour (so colour 1 is a very dark blue, colour 255 is white). Potentially useful for converting MSX graphics in 256 colour modes, but mainly included as a curiosity (was used by an early implementation of ULAplus)
//esta paleta 1 sale de ULAPLUS_INDEX_FIRST_COLOR+valor


//Para modos de paginacion de ram en rom del +2A
void prism_page_ram_in_rom(void)
{
        z80_byte page_type;

        page_type=(puerto_8189 >>1) & 3;
			z80_byte paginas[4];

        switch (page_type) {
                case 0:
                        //debug_printf (VERBOSE_DEBUG,"Pages 0,1,2,3");
								paginas[0]=0;
								paginas[1]=1;
								paginas[2]=2;
								paginas[3]=3;
                   break;

                case 1:
                        //debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");
								paginas[0]=4;
								paginas[1]=5;
								paginas[2]=6;
								paginas[3]=7;
          
                break;



 					case 2:
								//debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,3");

								paginas[0]=4;
								paginas[1]=5;
								paginas[2]=6;
								paginas[3]=3;
                           


                break;

                case 3:
                        //debug_printf (VERBOSE_DEBUG,"Pages 4,7,6,3");
              
								paginas[0]=4;
								paginas[1]=7;
								paginas[2]=6;
								paginas[3]=3;
                           
                break;


        }

			int i;
			for (i=0;i<4;i++) {
				z80_byte pag=paginas[i];
				//printf ("Segmento %d RAM pagina: %d\n",i,pag);

			//Segmento 0 16 KB
			prism_memory_paged[i*2]=prism_ram_mem_table[pag*2];
			prism_memory_paged[i*2+1]=prism_ram_mem_table[pag*2+1];

        debug_prism_paginas_memoria_mapeadas[i*2]=pag*2;
        debug_prism_paginas_memoria_mapeadas[i*2+1]=pag*2+1;


        prism_type_memory_paged[i*2]=PRISM_MEMORY_TYPE_HOME;
        prism_type_memory_paged[i*2+1]=PRISM_MEMORY_TYPE_HOME;

		}
 

}


//Retorna que ram entra en segmento alto C000-FFFF
z80_byte prism_retorna_ram_entra(void)
{

        z80_byte ram_entra;

        //Con modo compatibilidad 128k
        if (prism_ula2_registers[1]&8) {
                ram_entra= (puerto_32765 & 7);
                //printf ("Paging Prism with 128k compatibility mode\n");
        }

        else {
                //modo normal. toda la ram
                ram_entra= (puerto_32765 & 7) | ( (puerto_32765>>3) & 24) ;
        }

	return ram_entra;

}

void prism_set_memory_pages(void)
{
	//timex_port_f4;
	//Cada bit determina si se mapea home(0) o dock/ex
	//timex_port_ff
	//Bit 7:    Selects which bank the horizontal MMU should use. 0=DOCK, 1=EX-ROM.

	/*
z80_byte puerto_32765=0;

El conmutador de hardware esta en la direccion de E/S 7FFDh (32765). El campo del bit
para esta direccion es el siguiente:
DO a D2         Seleccion de RAM. Bits 0,1,2
D3              Seleccion de pantalla
D4                      Seleccion de ROM
D5       Inhabilitacion de la paginacion
D6       Bit 3 de RAM
D7       Bit 4 de RAM



Bit 2 de 1FFDh                 | Bit 4 de 7FFDh              |      ROM que entra en
(Variable de sistema: BANK678) | (Variable de sistema: BANKM)|  0000h-3FFFh
-------------------------------|-----------------------------|-----------------
0                              |     0                       |      0
0                              |     1                       |      1
1                              |     0                       |      2
1                              |     1                       |      3

Bit 2 de 8189  afecta a Bit 1 de 60987(prism_rom_page)
Bit 4 de 32765 afecta a Bit 0 de 60987

	*/


	//Paginacion de RAM en ROM
	if (puerto_8189&1) {
		prism_page_ram_in_rom();
		return;
	}
	



	//Paginas de ram entre 0 y 7
	//Paginas 6 y 7 vienen determinadas por byte de paginacion de 128 puerto 32765
	//En prism 280 se, paginas son segun puertos de paginacion timex y ultima pagina de ram

	//Asignamos primero como en prism 140 se. Luego para el 280se, cambiamos segun bits de paginacion timex
	z80_byte rom_entra=prism_rom_page;

	z80_byte *puntero_rom=prism_rom_mem_table[rom_entra];

	prism_memory_paged[0]=puntero_rom;
	prism_memory_paged[1]=&puntero_rom[8192];

	prism_type_memory_paged[0]=PRISM_MEMORY_TYPE_ROM;
	prism_type_memory_paged[1]=PRISM_MEMORY_TYPE_ROM;

	debug_prism_paginas_memoria_mapeadas[0]=rom_entra;
	debug_prism_paginas_memoria_mapeadas[1]=rom_entra;
	//printf ("Setting ROM %d\n",rom_entra);


	//Pagina 5
	prism_memory_paged[2]=prism_ram_mem_table[10];  //10=5*2
	prism_memory_paged[3]=prism_ram_mem_table[11];  //11=5*2+1
        prism_type_memory_paged[2]=PRISM_MEMORY_TYPE_HOME;
        prism_type_memory_paged[3]=PRISM_MEMORY_TYPE_HOME;
	debug_prism_paginas_memoria_mapeadas[2]=10;
	debug_prism_paginas_memoria_mapeadas[3]=11;

	//Pagina 2
	prism_memory_paged[4]=prism_ram_mem_table[4];
	prism_memory_paged[5]=prism_ram_mem_table[5];
        prism_type_memory_paged[4]=PRISM_MEMORY_TYPE_HOME;
        prism_type_memory_paged[5]=PRISM_MEMORY_TYPE_HOME;
	debug_prism_paginas_memoria_mapeadas[4]=4;
	debug_prism_paginas_memoria_mapeadas[5]=5;

	//Pagina segun puerto 32765
	// 7  6  5  4  3  2  1  0  ->Bits puerto
	// 4  3  x  x  x  2  1  0  ->Pagina
	//    64 32 16 8  4  2  1 
	z80_byte ram_entra;


	/*
	//Con modo compatibilidad 128k
	if (prism_ula2_registers[1]&8) {
		ram_entra= (puerto_32765 & 7);
		//printf ("Paging Prism with 128k compatibility mode\n");
	}

	else {
		//modo normal. toda la ram
		ram_entra= (puerto_32765 & 7) | ( (puerto_32765>>3) & 24) ;
	}
	*/

	ram_entra=prism_retorna_ram_entra();



	ram_entra=ram_entra*2;

	//Casos de entrada de paginas de VRAM:
	//SRAM10->VRAM0
	//SRAM11->VRAM1
	//SRAM14->VRAM2
	//SRAM15->VRAM3

	if (ram_entra==10 || ram_entra==14) {
		int vram_base=(ram_entra==10 ? 0 : 2);
		//printf ("Setting VRAM%d to 0x%04X\n",vram_base,6*8192);
		//printf ("Setting VRAM%d to 0x%04X\n",vram_base+1,7*8192);

		prism_memory_paged[6]=prism_vram_mem_table[vram_base];
		prism_memory_paged[7]=prism_vram_mem_table[vram_base+1];
	}

	else {

	        //z80_byte *puntero_ram_entra=prism_ram_mem_table[ram_entra];
		//printf ("Setting RAM %d to 0x%04X\n",ram_entra,6*8192);
		//printf ("Setting RAM %d to 0x%04X\n",ram_entra+1,7*8192);

        	prism_memory_paged[6]=prism_ram_mem_table[ram_entra];
	        prism_memory_paged[7]=prism_ram_mem_table[ram_entra+1];

	}

        prism_type_memory_paged[6]=PRISM_MEMORY_TYPE_HOME;
        prism_type_memory_paged[7]=PRISM_MEMORY_TYPE_HOME;
	debug_prism_paginas_memoria_mapeadas[6]=ram_entra;
	debug_prism_paginas_memoria_mapeadas[7]=ram_entra+1;



	//contend_pages_actual[3]=contend_pages_prism[ram_entra];


	//Aplicamos segun paginacion Timex
	int bloque_ram;
	z80_byte mascara_puerto_f4=1;

	for (bloque_ram=0;bloque_ram<8;bloque_ram++) {
			//Ver cada bit del puerto f4
			if (timex_port_f4 & mascara_puerto_f4) {
				//Se pagina ex o dock
			        //timex_port_ff
			        //Bit 7:    Selects which bank the horizontal MMU should use. 0=DOCK, 1=EX-ROM.
				z80_byte *puntero_memoria;
				if (timex_port_ff&128) {
					//EX
					puntero_memoria=prism_ex_ram_mem_table[bloque_ram];
					prism_type_memory_paged[bloque_ram]=PRISM_MEMORY_TYPE_EX;
					//printf ("Setting EX RAM %d to 0x%04X\n",bloque_ram,bloque_ram*8192);
				}
				else {
					//DOCK
					puntero_memoria=prism_dock_ram_mem_table[bloque_ram];
					prism_type_memory_paged[bloque_ram]=PRISM_MEMORY_TYPE_DOCK;
					//printf ("Setting DOCK RAM %d to 0x%04X\n",bloque_ram,bloque_ram*8192);
				}
				prism_memory_paged[bloque_ram]=puntero_memoria;
				debug_prism_paginas_memoria_mapeadas[bloque_ram]=bloque_ram;


				if (bloque_ram==6 || bloque_ram==7) {
				        //contend_pages_actual[3]=0;
				}

			}

			mascara_puerto_f4=mascara_puerto_f4<<1;
	}


	//Si esta en modo failsafe, cambiar pagina de la rom
	if (prism_failsafe_mode.v) {
		prism_memory_paged[0]=prism_failsafe_rom;
		prism_memory_paged[1]=&prism_failsafe_rom[8192];

        	prism_type_memory_paged[0]=PRISM_MEMORY_TYPE_ROM;
	        prism_type_memory_paged[1]=PRISM_MEMORY_TYPE_ROM;
	}

	//debug paginas mapeadas
/*
	int j;
	for (j=0;j<8;j++) {
extern z80_byte *prism_return_segment_memory(z80_int dir);
		//Y direccion segun poke 
		z80_byte *puntero=prism_return_segment_memory(j*8192);
		printf ("Paged segment %d address: %p initial addr: %p\n",j,prism_memory_paged[j],puntero);
		

	}
	//debug direcciones habituales, vram
	for (j=0;j<8;j++) {
		printf ("Ram %d address: %p\n",j,prism_ram_mem_table[j]);
	}

	for (j=0;j<4;j++) {
		printf ("VRam %d address: %p\n",j,prism_vram_mem_table[j]);
	}
*/

	//fin debug
	

	//printf ("end prism_set_memory_pages\n\n");

}


void prism_init_memory_tables(void)
{
	debug_printf (VERBOSE_DEBUG,"Initializing Prism memory pages");

	z80_byte *puntero;
	puntero=memoria_spectrum;

	int i;
	for (i=0;i<256;i++) {
		prism_rom_mem_table[i]=puntero;
		puntero +=16384;
	}

	for (i=0;i<64;i++) {
		prism_ram_mem_table[i]=puntero;
		puntero +=8192;
	}

	for (i=0;i<8;i++) {
		prism_dock_ram_mem_table[i]=prism_ram_mem_table[32+i];
	}

	for (i=0;i<8;i++) {
		prism_ex_ram_mem_table[i]=prism_ram_mem_table[40+i];
	}

	//Tablas contend
	//contend_pages_actual[0]=0;
	//contend_pages_actual[1]=contend_pages_prism[5];
	//contend_pages_actual[2]=contend_pages_prism[2];
	//contend_pages_actual[3]=contend_pages_prism[0];



}

void prism_load_failsafe_rom(void)
{

	debug_printf (VERBOSE_DEBUG,"Loading Prism failsafe rom");

	FILE *ptr_romfile;


	prism_failsafe_rom=malloc(16384);
	if (prism_failsafe_rom==NULL) cpu_panic("Cannot allocate memory for Prism failsafe rom");



	open_sharedfile(PRISM_FAILSAFE_ROM_NAME,&ptr_romfile);
	if (!ptr_romfile) {
		cpu_panic("Cannot open Prism failsafe rom");
	}

	int leidos=fread(prism_failsafe_rom,1,16384,ptr_romfile);

	if (leidos!=16384) {
		cpu_panic("Cannot read Prism failsafe rom");
	}

	fclose(ptr_romfile);

}


void prism_malloc_vram(void)
{

	z80_byte *puntero;

	puntero=malloc(8192*4);

	if (puntero==NULL) cpu_panic("Cannot allocate vram Prism memory");

	int i;

	for (i=0;i<4;i++) {
		prism_vram_mem_table[i]=puntero;
		puntero +=8192;
	}


}


//Usados en prism_splash_videomode_change
char *prism_texts_screentype[16]={
	"256x192, 8x8 Attributes","512x192, 8x8 Attributes","256x384, 8x8 Attributes","512x384, 8x8 Attributes",
	"256x192, 8x1 Attributes","Reserved(5)","256x384, 8x1 Attributes","Reserved(7)",
	"Linear","Reserved(9)","Reserved(10)","Reserved(11)",
	"Reserved(12)","Reserved(13)","Reserved(14)","Reserved(15)"
};

char *prism_texts_screendatadecoding[16]={
	"Spectrum","16+16 Colour","32 Colour","Chunk-o-blend",
	"256 Colour mode 1","256 Colour mode 2","4 Plane planar mode","3 Plane planar mode",
	"Overlay mode 1","Overlay mode 2","Gigablend mode","Brainebow",
	"4 Plane planar mode 256 colour","Jowett mode","Chunk-o-vision 128x192 mode I","Chunk-o-vision 128x192 mode II"
};

char *prism_texts_linear_modes[16]={
	"Reserved linear(0)","Reserved linear(1)","128x128 256 colours linear","256x128 256 colours linear",
	"Reserved linear(4)","Reserved linear(5)","Reserved linear(6)","Reserved linear(7)",
	"Reserved linear(8)","Reserved linear(9)","Reserved linear(10)","Reserved linear(11)",
	"Reserved linear(12)","Reserved linear(13)","Reserved linear(14)","Reserved linear(15)"
};

void prism_splash_videomode_change(void) {

	char mensaje[32*24];
	z80_byte screentype=prism_ula2_registers[2];
	z80_byte screendatadecoding=prism_ula2_registers[3];
	z80_byte palette=prism_ula2_registers[4];

	//Caso especial modo linear
	if (screentype==8) {
		screentype=prism_ula2_registers[6];
		sprintf (mensaje,"Setting screen type: %s, colour type: %s, palette: %d",
			prism_texts_linear_modes[screentype],prism_texts_screendatadecoding[screendatadecoding],palette);
	}		

	else {

		sprintf (mensaje,"Setting screen type: %s, colour type: %s, palette: %d",
			prism_texts_screentype[screentype],prism_texts_screendatadecoding[screendatadecoding],palette);
	}


	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);

}

void prism_out_ula2(z80_byte value)
{

	z80_byte ula2_register=(value>>4)&15;

	value=value&15;


	z80_byte anterior_compatibility_128=prism_ula2_registers[1]&8;
	z80_byte anterior_timex_enabled=prism_ula2_registers[1]&4;
	z80_byte anterior_timex_paging=prism_ula2_registers[1]&2;


	//printf ("Setting Prism ULA2 register %d to 0x%02X\n",ula2_register,value);

	prism_ula2_registers[ula2_register]=value;

	//printf para debug
	if (ula2_register==1) {
		if (value&1) {
			//printf ("Setting vram aperture\n");
		}


		//else printf ("Resetting vram aperture\n");
	}

        //Si hay cambio en bit compatibilidad 128
        if (ula2_register==1) {
                if ((value&8)!=anterior_compatibility_128) {
                        //printf ("Changing 128k compatibility to %s\n",(value&8 ? "Yes" : "No") );
                        prism_set_memory_pages();
                }


		//x1xx - Disable Timex screen modes selected using port FF
		//Disabling timex screen modes turns off high res or high colour mode if either is selected. 
		//It also resets the timex shadow screen if it had been selected.
		if ((value&4)!=anterior_timex_enabled) {
			if (value&4) {
				timex_port_ff=0;
				disable_timex_video();
			}
			else {
				enable_timex_video();
			}
		}

		//xx1x - Disable port F4 (disable Timex/Chloe/SE sideways RAM paging)
		//Disabling port F4 resets register F4 to "00000000" and prevents further writes (until a reset or you set the compatibility register to xx0x
		if ((value&2)!=anterior_timex_paging) {
			if (value&2) timex_port_f4=0;
			prism_set_memory_pages();
		}
        }

	if (ula2_register==2 || ula2_register==3 || ula2_register==4 || ula2_register==6) {
		prism_splash_videomode_change();
	}

	if (ula2_register==0) {
		prism_set_emulator_setting_cpuspeed();
	}

}


void hard_reset_cpu_prism(void)
{
                //Resetear paginacion Prism
                prism_rom_page=0;

                //Esto siempre antes de prism_set_memory_pages
                prism_failsafe_mode.v=0;

                //Resetear a 0 ULA2 registros
                int i;
                for (i=0;i<16;i++) prism_ula2_registers[i]=0;


                prism_last_ae3b=0;

                prism_set_memory_pages();

		//Resetear paleta 2
		init_prism_palette_two();
}


/*
About video modes and scanlines:


Caso 1)
794 columns, 525 rows

it's a 640x480 visible area
3500000/50/525=133 t-states per scanline
6 pixeles por cada t-estado
6*133=798. 798-640=158 pixeles no visibles de border=31 t-estados aprox

525 scanlines
525-480=45 lineas no visibles de border



So, border is:

top:
48 pixel height
bottom:
48 pixel height
hidden border: 0?

left:
64 pixel width
right:
64 pixel width



*/


/*

Sobre paletas de colores:

0100 - Register 4 - Palette select

0000 - Default (hard-coded) colour palette.

Colours 0-7 as per Spectrum, "bright 0" (intensity 1)

Colours 8-15 as per Spectrum "bright 1" (intensity 3)

Colours 16-23 as per spectrum but intensity 0

Colours 24-31 as per spectrum but intensity 2

Colours 32-255 yet to be "officially" defined (but let me know when you implement this and I can send you what's currently hard-coded on the prototype)

0001 - Colours 0-255 calculated using the same "GGGRRRBB" formulae as when defining a ULAplus colour (so colour 1 is a very dark blue, colour 255 is white). Potentially useful for converting MSX graphics in 256 colour modes, but mainly included as a curiosity (was used by an early implementation of ULAplus)
->El color sale de tabla ulaplus de conversion: color=ULAPLUS_INDEX_FIRST_COLOR+prism_ula2_border_colour;

0010 - ULA2 256 colour user-redefinable palette. Power-on defaults for this palette are yet to be "officially" established (but default the same values as hard-coded in palette 0).
->El color del border sale de aqui

*/

//16 initial redefinable palette. 
int initial_prism_palette_two[]={
/*
Colour 0 - "0000"&"0000"&"0000", -- Black
Colour 1 - "0000"&"0000"&"0011", -- Midnight Blue
Colour 2 - "0100"&"0001"&"0000", -- Reddy Brown
Colour 3 - "0100"&"0000"&"0110", -- Purple
Colour 4 - "0000"&"0011"&"0000", -- Forest Green
Colour 5 - "0011"&"0111"&"0111", -- Bluey grey
Colour 6 - "1111"&"1010"&"0000", -- Orange
Colour 7 - "0110"&"0110"&"0110", -- Light Grey
Colour 8 -  "0000"&"0000"&"0000", -- Black
 9-            "000000001111", -- Blue
10             "110000000000", -- Red
11           "111100001010", -- Pink
12             "001111110000", -- Lime Green
13             "000010101111", -- Sky Blue
14             "111111100000", -- Yellow
 15            "111111111111",-- White
         others=> "111111111111"
*/

0,3,1040,1030,
48,887,4000,1638,
0,15,3072,3850,
1008,175,4064,4095
};


//Ver docs/prism/palette_0.txt 
int initial_prism_palette_zero[]={
0,7,1792,1799,112,119,1904,1911,
0,15,3840,3855,240,255,4080,4095,
0,4,1024,1028,64,68,1088,1092,
0,10,2560,2570,160,170,2720,2730,
0,273,546,819,1092,1365,1638,1911,
2184,2457,2730,3003,3276,3549,3822,4095,
0,1,2,3,4,5,6,7,
8,9,10,11,12,13,14,15,
0,16,32,48,64,80,96,112,
128,144,160,176,192,208,224,240,
0,256,512,768,1024,1280,1536,1792,
2048,2304,2560,2816,3072,3328,3584,3840,
0,17,34,51,68,85,102,119,
136,153,170,187,204,221,238,255,
0,272,544,816,1088,1360,1632,1904,
2176,2448,2720,2992,3264,3536,3808,4080,
0,257,514,770,1026,1285,1542,1799,
2056,2313,2827,3084,3084,3341,3598,3855,
15,287,559,831,1103,1375,1919,1919,
2191,2463,2735,3007,3279,3551,3823,4095,
240,497,754,1011,1268,1525,1782,2039,
2296,2553,2810,3067,3324,3581,3838,4095,
3840,3857,3874,3891,3908,3925,3942,3959,
3976,3993,4010,4027,4044,4061,4078,4095,
10,282,554,826,1098,1370,1914,1914,
2186,2458,2730,3002,3274,3546,3818,4090,
160,417,674,931,1188,1445,1702,1959,
2216,2473,2730,2987,3244,3501,3758,4015,
2560,2577,2594,2611,2628,2645,2662,2679,
2696,2713,2730,2747,2764,2781,2798,2815,
0,3,1040,1030,48,887,4000,1638,
273,15,3072,3788,1008,175,4064,4095
};

void init_prism_palette_two(void)
{
	int i;

        //Palette Two
        for (i=0;i<16;i++) {
                prism_palette_two[i]=initial_prism_palette_two[i];
        }
        //Resto colores 255
        for (;i<256;i++) prism_palette_two[i]=4095;

}

void init_prism_palettes(void)
{

	int i;

	//Palette Zero
	for (i=0;i<256;i++) {
                prism_palette_zero[i]=initial_prism_palette_zero[i];
        }


	init_prism_palette_two();


}

//Ajustar modo turbo. Resto de bits de ese registro no usados 
void prism_set_emulator_setting_cpuspeed(void)
{
        z80_byte speed=prism_ula2_registers[0];
	if (speed&1) cpu_turbo_speed=1; //Valores impares no los emulo. Turbo=1
        else if (speed==0) cpu_turbo_speed=1;
        else if (speed==2) cpu_turbo_speed=2;
        else if (speed==4) cpu_turbo_speed=4;
        else if (speed==6) cpu_turbo_speed=8;
	else cpu_turbo_speed=16;

        cpu_set_turbo_speed();
}
