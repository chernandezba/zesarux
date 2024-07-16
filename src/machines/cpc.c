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

#include "cpc.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "contend.h"
#include "joystick.h"
#include "zxvision.h"
#include "operaciones.h"
#include "utils.h"
#include "audio.h"
#include "ay38912.h"
#include "tape.h"
#include "dsk.h"
#include "pd765.h"

//Direcciones donde estan cada pagina de rom. 2 o 3 paginas de 16 kb
z80_byte *cpc_rom_mem_table[3];

//Direcciones donde estan cada pagina de ram. 8 paginas de 16 kb cada una
z80_byte *cpc_ram_mem_table[8];

//Direcciones actuales mapeadas para lectura, bloques de 16 kb
z80_byte *cpc_memory_paged_read[4];
//Direcciones actuales mapeadas para escritura, bloques de 16 kb
z80_byte *cpc_memory_paged_write[4];

//Constantes definidas en CPC_MEMORY_TYPE_ROM, _RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
z80_byte debug_cpc_type_memory_paged_read[4];

//Paginas mapeadas en cada zona de RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
z80_byte debug_cpc_paginas_memoria_mapeadas_read[4];

//Offset a cada linea de pantalla
z80_int cpc_line_display_table[272];

//Forzar modo video para algunos juegos (p.ej. Paperboy)
z80_bit cpc_forzar_modo_video={0};
z80_byte cpc_forzar_modo_video_modo=0;


z80_byte cpc_gate_registers[4];

//Controla paginación upper rom
z80_byte cpc_port_df=0;

//Contador de linea para lanzar interrupcion.
z80_byte cpc_scanline_counter;

//Pendiente interrupcion de cpc crtc
z80_bit cpc_crt_pending_interrupt={0};

/*

Registro 2:

Bit	Value	Function
7	1	Gate Array function
6	0
5	-	not used
4	x	Interrupt generation control
3	x	1=Upper ROM area disable, 0=Upper ROM area enable
2	x	1=Lower ROM area disable, 0=Lower ROM area enable
1	x	Screen Mode slection
0	x

*/

z80_byte cpc_border_color=0;

z80_byte cpc_crtc_registers[32];

z80_byte cpc_crtc_last_selected_register=0;

//0=Port A
//1=Port B
//2=Port C
//3=Control
z80_byte cpc_ppi_ports[4];

//Paleta actual de CPC
z80_byte cpc_palette_table[16];

//tabla de conversion de colores de rgb cpc (8 bit) a 32 bit
//Cada elemento del array contiene el valor rgb real, por ejemplo,
//un valor rgb 11 de cpc, en la posicion 11 de este array retorna el color en formato rgb de 32 bits
int cpc_rgb_table[32]={

//0x000000,  //negro
//0x0000CD,  //azul
//0xCD0000,  //rojo
//0xCD00CD,  //magenta
//0x00CD00,  //verde
//0x00CDCD,  //cyan

0x808080, //0
0x808080, //1
0x00FF80, //2
0xFFFF80, //3

0x000080, //4 Azul
0xFF0080, //5
0x008080, //6
0xFF8080, //7

0xFF0080, //8
0xFFFF80, //9  //Pastel Yellow
0xFFFF00, //10 //Bright Yellow
0xFFFFFF, //11

0xFF0000, //12
0xFF00FF, //13
0xFF8000, //14
0xFF80FF, //15

0x0000FF, //16
0x00FF80, //17
0x00FF00, //18
0x00FFFF, //19

0x000000, //20
0x0000FF, //21
0x008000, //22
0x0080FF, //23

0x800080, //24
0x80FF80, //25
0x80FF00, //26
0x80FFFF, //27

0x800000, //28
0x8000FF, //29
0x808000, //30
0x8080FF //31

};



/*Tablas teclado
Bit:
Line	7       6       5	    4	    3	    2	    1	    0
&40	    FDot	ENTER	F3	    F6	    F9	    CURDOWN	CURRIGHT CURUP
&41	    F0	    F2	    F1	    F5	    F8	    F7	    COPY	CURLEFT
&42	    CONTROL	\	    SHIFT	F4	    ]	    RETURN	[	    CLR
&43	    .   	/	     :	    ;	    P	    @	    -	    ^
&44	    ,	    M	    K	    L	    I	    O	    9	    0
&45	    SPACE	N	    J	    H	    Y	    U	    7	    8
&46	    V	    B	    F	    G (Joy2 fire)	T (Joy2 right)	R (Joy2 left)	5 (Joy2 down)	6 (Joy 2 up)
&47	    X	    C	    D	    S	    W	    E	    3	    4
&48	    Z	    CAPSLOCK A	    TAB	    Q	    ESC	    2	    1
&49	    DEL	    Joy 1   Fire 3 (CPC only)	     Joy 1 Fire 2	   Joy1 Fire 1	   Joy1 right	  Joy1 left	   Joy1 down	Joy1 up


For CPC464 and CPC664 users:
ENTER is the small ENTER key,
RETURN is the large ENTER key

*/


//Aunque solo son 10 filas, metemos array de 16 pues es el maximo valor de indice seleccionable por el PPI
z80_byte cpc_keyboard_table[16]={
255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255
};


//z80_bit cpc_send_double_vsync={0};

z80_bit cpc_debug_borders={0};

z80_bit cpc_vsync_signal={0};

//Ultima linea que se ha escrito en el buffer rainbow. Sirve para luego al renderizar, las que quedan por debajo, ponerlas en negro
int cpc_last_drawn_line=0;

//Pendiente de mostrar esas lineas en negro
z80_bit cpc_pending_last_drawn_lines_black={0};


//Para forzar cambio scanline a 0 cuando se finaliza un frame de video
//esto no deberia ser asi, se deberia ir a scanline 0 solo cuando hay vsync
//z80_bit cpc_endframe_workaround={0};

//Contador de cuanto falta para mostrar esas lineas en negro, en frames
int cpc_pending_last_drawn_lines_black_counter=0;

void cpc_reset_last_drawn_line(void)
{
    cpc_last_drawn_line=0;
    //Pendiente de mostrar esas lineas en negro
    cpc_pending_last_drawn_lines_black.v=1;

    //Contador de cuanto falta para mostrar esas lineas en negro, en frames
    cpc_pending_last_drawn_lines_black_counter=50;
}

void cpc_set_memory_pages()
{

	//Array de paginas que entran. Por defecto en 64kb, paginas 0,1,2,3
	int pages_array[4];

	//Por defecto
	pages_array[0]=0;
	pages_array[1]=1;
	pages_array[2]=2;
	pages_array[3]=3;


	//Si es maquina de 128kb, reasignar paginas
	if (MACHINE_IS_CPC_HAS_128K) {
		z80_byte ram_config=cpc_gate_registers[3] & 7;

		//printf ("Setting 128k ram config value %d\n",ram_config);


/*
memoria extendida mas alla de 64 kb

Register 3 - RAM Banking
This register exists only in CPCs with 128K RAM (like the CPC 6128, or CPCs with Standard Memory Expansions). Note: In the CPC 6128, the register is a separate PAL that assists the Gate Array chip.

Bit	Value	Function
7	1	Gate Array function 3
6	1
5	b	64K bank number (0..7); always 0 on an unexpanded CPC6128, 0-7 on Standard Memory Expansions
4	b
3	b
2	x	RAM Config (0..7)
1	x
0	x


The 3bit RAM Config value is used to access the second 64K of the total 128K RAM that is built into the CPC 6128
or the additional 64K-512K of standard memory expansions. These contain up to eight 64K ram banks,
which are selected with bit 3-5. A standard CPC 6128 only contains bank 0.
Normally the register is set to 0, so that only the first 64K RAM are used (identical to the CPC 464 and 664 models).
The register can be used to select between the following eight predefined configurations only:

 -Address-     0      1      2      3      4      5      6      7
 0000-3FFF   RAM_0  RAM_0  RAM_4  RAM_0  RAM_0  RAM_0  RAM_0  RAM_0
 4000-7FFF   RAM_1  RAM_1  RAM_5  RAM_3  RAM_4  RAM_5  RAM_6  RAM_7
 8000-BFFF   RAM_2  RAM_2  RAM_6  RAM_2  RAM_2  RAM_2  RAM_2  RAM_2
 C000-FFFF   RAM_3  RAM_7  RAM_7  RAM_7  RAM_3  RAM_3  RAM_3  RAM_3
The Video RAM is always located in the first 64K, VRAM is in no way affected by this register.

*/
		switch (ram_config) {

			case 1:
				pages_array[3]=7;
			break;

			case 2:
				pages_array[0]=4;
				pages_array[1]=5;
				pages_array[2]=6;
				pages_array[3]=7;
			break;

			case 3:
				pages_array[1]=3;
				pages_array[3]=7;
			break;

			case 4:
				pages_array[1]=4;
			break;

			case 5:
				pages_array[1]=5;
			break;

			case 6:
				pages_array[1]=6;
			break;

			case 7:
				pages_array[1]=7;
			break;

		}


	}


	//printf ("paginas que entran: %d %d %d %d\n",pages_array[0],pages_array[1],pages_array[2],pages_array[3]);

	//Escritura siempre en RAM
	int i;
	for (i=0;i<4;i++) {
		//cpc_memory_paged_write[i]=cpc_ram_mem_table[i];
		int pagina_entra=pages_array[i];
		cpc_memory_paged_write[i]=cpc_ram_mem_table[pagina_entra];
	}



	int pagina_entra;

	//Bloque 0-16383
	if (cpc_gate_registers[2] &4 ) {
		//Entra RAM
		pagina_entra=pages_array[0];
		cpc_memory_paged_read[0]=cpc_ram_mem_table[pagina_entra];
		debug_cpc_type_memory_paged_read[0]=CPC_MEMORY_TYPE_RAM;
		debug_cpc_paginas_memoria_mapeadas_read[0]=pagina_entra;
	}
	else {
		//Entra ROM
		cpc_memory_paged_read[0]=cpc_rom_mem_table[0];
		debug_cpc_type_memory_paged_read[0]=CPC_MEMORY_TYPE_ROM;
		debug_cpc_paginas_memoria_mapeadas_read[0]=0;
	}




	//Bloque 16384-32767
	//RAM
	pagina_entra=pages_array[1];
	cpc_memory_paged_read[1]=cpc_ram_mem_table[pagina_entra];
	debug_cpc_type_memory_paged_read[1]=CPC_MEMORY_TYPE_RAM;
	debug_cpc_paginas_memoria_mapeadas_read[1]=pagina_entra;


	//Bloque 32768-49151
	//RAM
	pagina_entra=pages_array[2];
	cpc_memory_paged_read[2]=cpc_ram_mem_table[pagina_entra];
	debug_cpc_type_memory_paged_read[2]=CPC_MEMORY_TYPE_RAM;
	debug_cpc_paginas_memoria_mapeadas_read[2]=2;

    //Bloque 49152-65535
    if (cpc_gate_registers[2] &8 ) {
    	//Entra RAM
		pagina_entra=pages_array[3];
        cpc_memory_paged_read[3]=cpc_ram_mem_table[pagina_entra];
		debug_cpc_type_memory_paged_read[3]=CPC_MEMORY_TYPE_RAM;
		debug_cpc_paginas_memoria_mapeadas_read[3]=pagina_entra;
    }
    else {
        //Entra ROM
        //en 6128 y 664 ver si entra amsdos rom
        if ((MACHINE_IS_CPC_HAS_FLOPPY) && cpc_port_df==7) {
            //Entra AMSDOS
            cpc_memory_paged_read[3]=cpc_rom_mem_table[2];
        }
        else {
            cpc_memory_paged_read[3]=cpc_rom_mem_table[1];
        }

		debug_cpc_type_memory_paged_read[3]=CPC_MEMORY_TYPE_ROM;
		debug_cpc_paginas_memoria_mapeadas_read[3]=1;
    }

    //Info: la rom de 6128 consiste en: os+basic+amsdos

}


void cpc_out_port_df(z80_byte value)
{
    cpc_port_df=value;

    cpc_set_memory_pages();

}

void cpc_init_memory_tables()
{
	debug_printf (VERBOSE_DEBUG,"Initializing cpc memory tables");

        z80_byte *puntero;
        puntero=memoria_spectrum;

        cpc_rom_mem_table[0]=puntero;
        puntero +=16384;
        cpc_rom_mem_table[1]=puntero;
        puntero +=16384;

        if (MACHINE_IS_CPC_HAS_FLOPPY) {
            cpc_rom_mem_table[2]=puntero;
            puntero +=16384;
        }


        int i;
        for (i=0;i<8;i++) {
                cpc_ram_mem_table[i]=puntero;
                puntero +=16384;
        }

}

//Numero de cambios de modo de video en un frame de video (desde un vsync hasta el otro)
//para poder detectar 2 o mas cambios de modo y asi autoactivar realvideo
int cpc_video_modes_change_frame_counter=0;

//Parecido pero para cambios de border
int cpc_border_change_frame_counter=0;

void cpc_out_port_gate(z80_byte value)
{
	/*
Data Bit 7	Data Bit 6	Function
0	0	Select pen
0	1	Select colour for selected pen
1	0	Select screen mode, ROM configuration and interrupt control
1	1	RAM Memory Management (note 1)

note 1: This function is not available in the Gate-Array, but is performed by a device at the same I/O port address location. In the CPC464, CPC664 and KC compact, this function is performed in a memory-expansion (e.g. Dk'Tronics 64K RAM Expansion), if this expansion is not present then the function is not available. In the CPC6128, this function is performed by a PAL located on the main PCB, or a memory-expansion. In the 464+ and 6128+ this function is performed by the ASIC or a memory expansion. Please read the document on RAM management for more information.
	*/


        z80_byte modo_video_antes=cpc_gate_registers[2] &3;


	z80_byte funcion=(value>>6)&3;


	cpc_gate_registers[funcion]=value;


	z80_byte modo_video_despues=cpc_gate_registers[2] &3;

	if (modo_video_despues!=modo_video_antes) {
        cpc_video_modes_change_frame_counter++;
        //printf("contador: %d\n",cpc_video_modes_change_frame_counter);
        cpc_if_autoenable_realvideo_on_changemodes();
        cpc_splash_videomode_change();
    }


	//printf ("Changing register %d of gate array\n",funcion);

	switch (funcion) {
		case 0:
			//Seleccion indice color a paleta o border
			if (value&16) {
				//printf ("Seleccion border. TODO\n");
				/*z80_byte color=value&15;
				if (cpc_border_color!=color) {
					cpc_border_color=color;
					//printf ("Setting border color with value %d\n",color);
					modificado_border.v=1;
				}
                */
			}

			else {
				//Seleccion indice. Guardado en cpc_gate_registers[0] el indice en los 4 bits inferiores
				//printf ("Setting index palette %d\n",cpc_gate_registers[0]&15);
			}
		break;

		case 1:
                        if (cpc_gate_registers[0] & 16) {
                                //Seleccion color borde. Color directo de paleta
                                z80_byte color=value&31;
                                cpc_border_color=color;
                                //printf("cambio color border. color =%d contador= %d\n",color,cpc_border_change_frame_counter);
                                modificado_border.v=1;


                                cpc_border_change_frame_counter++;
                                //printf("cambio color border. total cambios: %d\n",cpc_border_change_frame_counter);
                                cpc_if_autoenable_realvideo_on_changeborder();
                        }

                        else {
                                //Seleccion color para indice. Guardado en cpc_gate_registers[0] el indice en los 4 bits inferiores
				z80_byte indice=cpc_gate_registers[0]&15;

				z80_byte color=value&31;

				cpc_palette_table[indice]=color;

				//printf ("Setting index color %d with value %d\n",indice,color);


				//Si se cambia color para indice de color de border, refrescar border
				//if (indice==cpc_border_color) modificado_border.v=1;

			}


		break;

		case 2:

                /*

                Registro 2:

                Bit	Value	Function
                7	1	Gate Array function
                6	0
                5	-	not used
                4	x	Interrupt generation control
                3	x	1=Upper ROM area disable, 0=Upper ROM area enable
                2	x	1=Lower ROM area disable, 0=Lower ROM area enable
                1	x	Screen Mode slection
                0	x

                */

			//Cambio paginacion y modo video y gestion interrupcion
			cpc_set_memory_pages();

			if (value&16) {
				//Esto resetea bit 5 de contador de scanline
				//printf ("Resetting bit 5 of cpc_scanline_counter\n");
                //If bit 4 of the "Select screen mode and rom configuration" register of the Gate-Array
                //(bit 7="1" and bit 6="0") is set to "1" then the interrupt request is cleared and the 6-bit counter is reset to "0".
				cpc_scanline_counter &=(255-32);
                cpc_crt_pending_interrupt.v=0;
			}
		break;

		case 3:
			//printf ("Memory management only on cpc 6128. Setting value %02XH\n",value);
			//Cambio paginacion en modos 128kb ram
			cpc_set_memory_pages();
/*
Register 3 - RAM Banking
This register exists only in CPCs with 128K RAM (like the CPC 6128, or CPCs with Standard Memory Expansions). Note: In the CPC 6128, the register is a separate PAL that assists the Gate Array chip.

Bit	Value	Function
7	1	Gate Array function 3
6	1
5	b	64K bank number (0..7); always 0 on an unexpanded CPC6128, 0-7 on Standard Memory Expansions
4	b
3	b
2	x	RAM Config (0..7)
1	x
0	x


The 3bit RAM Config value is used to access the second 64K of the total 128K RAM that is built into the CPC 6128
or the additional 64K-512K of standard memory expansions.
These contain up to eight 64K ram banks, which are selected with bit 3-5. A standard CPC 6128 only contains bank 0.
Normally the register is set to 0, so that only the first 64K RAM are used (identical to the CPC 464 and 664 models).
The register can be used to select between the following eight predefined configurations only:

 -Address-     0      1      2      3      4      5      6      7
 0000-3FFF   RAM_0  RAM_0  RAM_4  RAM_0  RAM_0  RAM_0  RAM_0  RAM_0
 4000-7FFF   RAM_1  RAM_1  RAM_5  RAM_3  RAM_4  RAM_5  RAM_6  RAM_7
 8000-BFFF   RAM_2  RAM_2  RAM_6  RAM_2  RAM_2  RAM_2  RAM_2  RAM_2
 C000-FFFF   RAM_3  RAM_7  RAM_7  RAM_7  RAM_3  RAM_3  RAM_3  RAM_3
The Video RAM is always located in the first 64K, VRAM is in no way affected by this register.



*/
		break;
	}
}




void init_cpc_line_display_table(void)
{
	debug_printf (VERBOSE_DEBUG,"Init CPC Line Display Table");
	int y;
	z80_int offset;
	for (y=0;y<200;y++) {
		offset=((y / 8) * 80) + ((y % 8) * 2048);
		cpc_line_display_table[y]=offset;
		debug_printf (VERBOSE_PARANOID,"CPC Line: %d Offset: 0x%04X",y,offset);
	}
}


//  return (unsigned char *)0xC000 + ((nLine / 8) * 80) + ((nLine % 8) * 2048);


int cpc_crtc_get_total_vsync_height_ga(void)
{
    /*
    una cosa es la vsync del crtc y otra la del ga
    la del crtc dispara la del ga
    y la del ga mide siempre 24 lineas
    independientemente de la del crtc
    la del ga es la que va al monitor
    */
    return 24;
}


int cpc_crtc_get_total_vsync_height_crtc(void)
{
    /*
    una cosa es la vsync del crtc y otra la del ga
    la del crtc dispara la del ga
    y la del ga mide siempre 24 lineas
    independientemente de la del crtc
    la del ga es la que va al monitor
    */
    return (cpc_crtc_registers[3]>>4) & 15;
}

int cpc_crtc_get_vsync_position(void)
{
    int valor=cpc_crtc_registers[7]&127;

    //esta en caracteres
	valor *=cpc_crtc_get_height_character();

    return valor;
}

z80_int cpc_ctrc_get_offset_videoram(void)
{

    z80_int crtc_offset_videoram=(cpc_crtc_registers[13])  + (256*(cpc_crtc_registers[12]&3));

    crtc_offset_videoram *= 2;

    return crtc_offset_videoram;

}

//Obtener scanline donde acaba despues de borde superior, en el caso del conteo que hace el crtc:
//0...x pixeles
//..... borde inferior
//..... vsync
//..... borde superior
int cpc_get_crtc_final_display_zone(void)
{

    return cpc_crtc_get_total_vertical();


}

//t_scanline_draw va contando en el caso de cpc:
//0...x pixeles
//..... borde inferior
//..... vsync
//..... borde superior
//Convertir ese valor a un contador de linea para el buffer rainbow donde el orden sea:
//..... borde superior
//..... pixeles
//..... borde inferior
//..... vsync
//Y esto contando scanlines. Luego el buffer rainbow al final lo multiplicara X2 para tener doble de alto
int cpc_convert_scanline_to_final_y(void)
{
    int borde_superior=cpc_crtc_get_top_border_height();

    int borde_inferior=cpc_crtc_get_bottom_border_height();


    int pixeles_alto=cpc_crtc_get_total_pixels_vertical();


    int vsync_alto=cpc_crtc_get_total_vsync_height_crtc();

    int inicio_borde_superior=pixeles_alto+borde_inferior+vsync_alto;


    //printf("Final borders %d %d pixeles %d vsync %d inicio borde superior: %d\n",borde_superior,borde_inferior,pixeles_alto,vsync_alto,inicio_borde_superior);

    //Si esta en el inicio del borde superior
    if (t_scanline_draw>=inicio_borde_superior) {
        return t_scanline_draw-inicio_borde_superior;
    }

    else {
        return t_scanline_draw+borde_superior;
    }


}


//Decir si vsync está activo o no, según en qué posición de pantalla estamos,
//y resetear t_scanline_draw a 0 cuando finaliza dicha vsync
//Ver http://www.cpcwiki.eu/index.php/CRTC#HSYNC_and_VSYNC
/*
The VSYNC is also modified before being sent to the monitor. It happens two lines* after the VSYNC from the CRTC
and stay two lines (same cut rule if VSYNC is lower than 4). PAL (50Hz) does need two lines VSYNC_width, and 4us HSYNC_width.
*/

int cpc_crtc_contador_scanline=0;

void cpc_handle_vsync_state(void)
{

    cpc_crtc_contador_scanline++;

    //mi t_scanline_draw es el del monitor (el ga gate array)
    //cpc_crtc_contador_scanline es la cuenta de scanlines del CRTC


	//Duracion vsync

    int vsync_lenght=cpc_crtc_get_total_vsync_height_crtc();

	//Si es 0, en algunos chips significa 16
	if (vsync_lenght==0) vsync_lenght=16;

	//if (cpc_send_double_vsync.v) vsync_lenght *=2;

	int vsync_position=cpc_crtc_get_vsync_position();



	int final_vsync=vsync_position+vsync_lenght;


    if (cpc_crtc_contador_scanline>=vsync_position && cpc_crtc_contador_scanline<final_vsync) {
        cpc_vsync_signal.v=1;

/*
The Gate-Array senses the VSYNC signal. If two HSYNCs have been detected following the start of the VSYNC
then there are two possible actions:

If the top bit of the 6-bit counter is set to "1" (i.e. the counter >=32), then there is no interrupt request,
and the 6-bit counter is reset to "0". (If a interrupt was requested and acknowledged it would be closer than 3
2 HSYNCs compared to the position of the previous interrupt).
If the top bit of the 6-bit counter is set to "0" (i.e. the counter <32), then a interrupt request is issued,
and the 6-bit counter is reset to "0".
In both cases the following interrupt requests are synchronised with the VSYNC.
*/

        //TODO: no estoy del todo seguro con esto
        //No seria la condicion al reves? cpc_scanline_counter<32 Esta al reves

        //O querra decir que se resetea el interrupt request si >=32, pero en caso contrario, no se activa un interrupt request?

        /*
        http://cpctech.cpc-live.com/source/split.html
        ;; The interrupt counter is updated every HSYNC.
        ;; The interrupt counter reset is synchronised to the start of the VSYNC.
        ;; A interrupt request is issued when the interrupt counter reaches 52.
        ;;
        ;; The next interrupt could occur in two HSYNC times, assuming that
        ;; the previous interrupt was not serviced less than 32 lines ago.
        ;;
        ;; Otherwise the next interrupt will occur in 52+2 HSYNC times.
        ;;
        ;; A perfect split relies on predicting the position of the start of the VSYNC
        ;; and the position of the interrupts, as these are the signals we use to
        ;; synchronise with the display, and this means that we can setup the next split
        ;; block at the correct position).
        */

       //Por tanto creo que vsync solo resetea cpc_scanline_counter y nada mas


        if (cpc_crtc_contador_scanline==vsync_position+2) {
            if (cpc_scanline_counter>=32 && ay_player_playing.v==0) {
            	cpc_crt_pending_interrupt.v=1;
            }

            cpc_scanline_counter=0;

            //Resetear contador de cambios de modo de video en un frame
            cpc_video_modes_change_frame_counter=0;
            //Y de border
            cpc_border_change_frame_counter=0;
        }


    }
	else {
        cpc_vsync_signal.v=0;
    }



}

z80_byte cpc_get_vsync_bit(void)
{


	return cpc_vsync_signal.v;
}





//http://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning
//http://www.cpcwiki.eu/index.php/8255
z80_byte cpc_in_ppi(z80_byte puerto_h)
{

/*
Bit 9	Bit 8	PPI Function	Read/Write status
0	0	Port A data	Read/Write
0	1	Port B data	Read/Write
1	0	Port C data	Read/Write
1	1	Control	Write Only


I/O address	A9	A8	Description	Read/Write status	Used Direction	Used for
&F4xx		0	0	Port A Data	Read/Write		In/Out		PSG (Sound/Keyboard/Joystick)
&F5xx		0	1	Port B Data	Read/Write		In		Vsync/Jumpers/PrinterBusy/CasIn/Exp
&F6xx		1	0	Port C Data	Read/Write		Out		KeybRow/CasOut/PSG
&F7xx		1	1	Control		Write Only		Out		Control
*/

	z80_byte port_number=puerto_h&3;
	z80_byte valor;

	z80_byte psg_function;

	switch (port_number) {
		case 0:
			//printf ("Reading PPI port A\n");
			// cpc_ppi_ports[3]
			psg_function=(cpc_ppi_ports[2]>>6)&3;
                        if (psg_function==1) {
				//printf ("Leer de registro PSG. Registro = %d\n",ay_3_8912_registro_sel[ay_chip_selected]);
				if (ay_3_8912_registro_sel[ay_chip_selected]==14) {
					//leer teclado
					//Linea a leer
					z80_byte linea_teclado=cpc_ppi_ports[2] & 15;
					//printf ("Registro 14. Lee fila teclado: 0x%02X\n",linea_teclado | 0x40);


					if (initial_tap_load.v==1 && initial_tap_sequence) {
						return envia_load_ctrlenter_cpc(linea_teclado);
					}

			                //si estamos en el menu, no devolver tecla
			                if (zxvision_key_not_sent_emulated_mach() ) return 255;

           //Si esta spool file activo, generar siguiente tecla
                if (input_file_keyboard_is_playing() ) {
                                input_file_keyboard_get_key();
                }



                    if (linea_teclado==9) {
                        //printf("leyendo joystick\n");
                        //&49	DEL	Joy 1 Fire 3 (CPC only)	Joy 1 Fire 2	Joy1 Fire 1	Joy1 right	Joy1 left	Joy1 down	Joy1 up

                        //Solo preservar bit 7 donde está la tecla DEL
                        z80_byte valor_joystick=cpc_keyboard_table[9] & 128;

                        //resto de teclas de esa linea de teclado, asumimos no pulsadas

                        valor_joystick |=127;

                        if (joystick_emulation==JOYSTICK_CPC_1) {
                            //si estamos con menu abierto, no retornar nada
                            if (zxvision_key_not_sent_emulated_mach() ) return valor_joystick;

                            //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
                            if ((puerto_especial_joystick&1)) valor_joystick &=(255-8);   //right
                            if ((puerto_especial_joystick&2)) valor_joystick &=(255-4);   //left
                            if ((puerto_especial_joystick&4)) valor_joystick &=(255-2);  //down
                            if ((puerto_especial_joystick&8)) valor_joystick &=(255-1);  //up
                            if ((puerto_especial_joystick&16)) valor_joystick &=(255-16);  //fire1
                        }

                        return valor_joystick;


                    }

                    //Retornar fila teclado
                    z80_byte byte_teclado=cpc_keyboard_table[linea_teclado];

                    //Si es fila 0 y los cursores emulan el joystick de cpc, no indicar pulsaciones de cursor de cpc
                    if (linea_teclado==0 && joystick_emulation==JOYSTICK_CPC_1) {
                        //&40	F Dot	ENTER	F3	F6	F9	CURDOWN	CURRIGHT	CURUP
                        byte_teclado |=1+2+4;
                    }

                    //Si es fila 1 y los cursores emulan el joystick de cpc, no indicar pulsaciones de cursor de cpc ni tecla copy
                    if (linea_teclado==1 && joystick_emulation==JOYSTICK_CPC_1) {
                        //&41	F0	F2	F1	F5	F8	F7	COPY	CURLEFT
                        byte_teclado |=1+2;

                    }

					return byte_teclado;

				}


				else if (ay_3_8912_registro_sel[ay_chip_selected]<14) {
					//Registros chip ay
                    //printf("leyendo registro PSG %d en PC=%d\n",ay_3_8912_registro_sel[ay_chip_selected],reg_pc);
                    //sleep(2);
					return in_port_ay(0xFF);
				}
			}
		break;

		case 1:
			//printf ("Reading PPI port B\n");
			valor=cpc_ppi_ports[1];

			//Metemos fabricante amstrad
			valor |= (2+4+8);

			//Refresco 50 hz
			valor |=16;

			//Parallel, expansion port a 0
			valor &=(255-64-32);

			//Bit 0 (vsync)
			valor &=(255-1);
			valor |=cpc_get_vsync_bit();

            int leer_cinta_real=0;

            if (realtape_inserted.v && realtape_playing.v) leer_cinta_real=1;

            if (audio_can_record_input()) {
                if (audio_is_recording_input) {
                    leer_cinta_real=1;
                }
            }

            if (leer_cinta_real) {
                        	if (realtape_get_current_bit_playing()) {
                                	valor=valor|128;
	                                //printf ("1 ");
        	                }
                	        else {
                        	        valor=(valor & (255-128));
                                	//printf ("0 ");
	                        }
			}
			return valor;


		break;

		case 2:
			//printf ("Reading PPI port C\n");
			return cpc_ppi_ports[2];
		break;

		case 3:
			//printf ("Reading PPI port control write only\n");
            //sleep(1);
		break;

	}

	return 255;

}

z80_byte cpc_psg_control_bits=0;

void cpc_cassette_motor_control (int valor_bit)
{
    //primero vemos si hay cinta insertada
    if (realtape_name!=NULL && realtape_inserted.v) {

            if (valor_bit) {
                    //Activar motor si es que no estaba ya activado
                    if (realtape_playing.v==0) {
                            debug_printf (VERBOSE_INFO,"CPC motor on function received. Start playing real tape");
                            realtape_start_playing();
                    }
            }
            else {
                    //Desactivar motor si es que estaba funcionando
                    if (realtape_playing.v) {
                            debug_printf (VERBOSE_INFO,"CPC motor off function received. Stop playing real tape");
                            //desactivado, hay juegos que envian motor off cuando no conviene realtape_stop_playing();
                    }
            }
    }
}


void cpc_out_ppi(z80_byte puerto_h,z80_byte value)
{
/*
Bit 9   Bit 8   PPI Function    Read/Write status
0       0       Port A data     Read/Write
0       1       Port B data     Read/Write
1       0       Port C data     Read/Write
1       1       Control Write Only
*/

        z80_byte port_number=puerto_h&3;
	z80_byte psg_function;


    //printf ("Writing PPI port %02XH (PPI funcion %d) value %d on PC=%d\n",puerto_h,puerto_h&3,value,reg_pc);

        switch (port_number) {
                case 0:

                        //printf ("Writing PPI port A value %d en pc=%d\n",value,reg_pc);
                        cpc_ppi_ports[0]=value;


                        //Si control esta en output
                    /*
                    if ((cpc_psg_control_bits & 16)==0) {
                        printf ("Writing PPI port A value %d en pc en modo escritura=%d\n",value,reg_pc);
			            cpc_ppi_ports[0]=value;
                    }

                    else {
                        printf ("Writing PPI port A value %d en pc en modo lectura=%d\n",value,reg_pc);
			            //cpc_ppi_ports[0]=value;
                        ay_3_8912_registro_sel[ay_chip_selected]=value;
                    }
                    */


                break;

                case 1:
                        //printf ("Writing PPI port B value 0x%02X pc=%d\n",value,reg_pc);
			cpc_ppi_ports[1]=value;
                break;

                case 2:
                        //printf ("Writing PPI port C value 0x%02X\n",value);
/*
Bit 7	Bit 6	Function
0	0	Inactive
0	1	Read from selected PSG register
1	0	Write to selected PSG register
1	1	Select PSG register

*/
			psg_function=(value>>6)&3;
            //printf ("Writing PPI port C value %d psg_funcion %d pc=%d\n",value,psg_function,reg_pc);


            //Select PSG register
			if (psg_function==3) {
				//Seleccionar ay chip registro indicado en port A
				//printf ("Seleccionamos PSG registro %d en pc=%d\n",cpc_ppi_ports[0],reg_pc);

                //no estoy seguro si realmente debe seleccionar el registro del chip AY, o esto solo se hace con funcion 0
				out_port_ay(65533,cpc_ppi_ports[0]);
                cpc_ppi_ports[2]=value;
			}


			//Write to selected PSG register
			if (psg_function==2) {
				//Enviar valor a psg
				//printf ("Enviamos PSG valor %d al registro %d\n",cpc_ppi_ports[0],ay_3_8912_registro_sel[ay_chip_selected]);
                out_port_ay(49149,cpc_ppi_ports[0]);
                cpc_ppi_ports[2]=value;
            }

            //Read from selected PSG register
            if (psg_function==1) {
                cpc_ppi_ports[2]=value;
            }

            //Inactive
            if (psg_function==0) {
                //printf("psg funcion 0 on pc=%d. pre haciendo seleccion registro psg %d\n",reg_pc,cpc_ppi_ports[0]);
                //Esto es necesario para que la musica del sword of ianna se escuche bien
                //No acabo de entender bien lo que hace el inactive mode pero...
                out_port_ay(65533,cpc_ppi_ports[0]);
                cpc_ppi_ports[2]=value;
                /*
                Previo a versión ZEsarUX 9.2 aquí no se hacia nada, pero revisando con juegos con musica con el Arkos Player
                se observa que es necesario (caso del sword of ianna por ejemplo)
                Usa sentencias OUT (C), 0 para pasar a este modo inactivo
                La traza de ejecuciones es:

                Enviamos PSG valor 0 al registro 0
Writing PPI port C value 192 psg_funcion 3 pc=13215
AY chip seleccion registro 0
Writing PPI port A value 7 en pc=13219
Outing 0 to F607H on pc=13222
Writing PPI port C value 0 psg_funcion 0 pc=13222
psg funcion 0 on pc=13222. pre haciendo seleccion registro psg 7


Writing PPI port A value 61 en pc=13226 -> guarda en PPI port A valor 61
Writing PPI port C value 128 psg_funcion 2 pc=13229 ->Enviar valor de port A a ultimo registro seleccionado en chip AY (el 61)

**Enviamos PSG valor 61 al registro 0**
**Writing PPI port C value 192 psg_funcion 3 pc=13231  -> Seleccion registro de port A (61)??? Es lo que ya se hacia antes **

AY chip seleccion registro 61
Writing PPI port A value 8 en pc=13235  -> guarda en PPI port A valor 8
Outing 0 to F608H on pc=13238
Writing PPI port C value 0 psg_funcion 0 pc=13238
**psg funcion 0 on pc=13238. pre haciendo seleccion registro psg 8 -> Y agrego selección de registro en funcion 0 también**
Writing PPI port A value 0 en pc=13241  -> guarda en PPI port A valor 0
Writing PPI port C value 128 psg_funcion 2 pc=13244
**Enviamos PSG valor 0 al registro 13 (61 & 15)**
Writing PPI port C value 192 psg_funcion 3 pc=13246
AY chip seleccion registro 0


Info:

Register selection

Before reading or writing to the PSG, the appropiate register must be selected.

This is done by putting the register number (0-14) into port &F4xx, and setting bits 7 and 6 of port &F6xx to 1.
The register will now be selected and the user can now read or write a value to it. Finally, the PSG must be put into
an inactive state by setting bit 7 and 6 to 0 of port &F6xx.

This is necessary, otherwise if the register select command was still in operation, and a byte was sent to port &F4xx,
it would use this and change the data in the last register selected. (see below)

Writing to a PSG register

To write data to the PSG, the user must put the data in port &F4xx, and then set bit 7 to 1 and bit 6 to 0 of port &F6xx.
The data will be written into the register. Finally, the PSG must be put into an inactive state by setting bit 7 and 6 to 0 of port &F6xx.

                */
            }

			//cpc_ppi_ports[2]=value;


                break;

                case 3:
                        //printf ("Writing PPI port control write only value 0x%02X on pc=%d\n",value,reg_pc);
                        //sleep(1);
            /*
            PPI Control with Bit7=1
If Bit 7 is "1" then the other bits will initialize Port A-B as Input or Output:

 Bit 0    IO-Cl    Direction for Port C, lower bits (always 0=Output in CPC)
 Bit 1    IO-B     Direction for Port B             (always 1=Input in CPC)
 Bit 2    MS0      Mode for Port B and Port Cl      (always zero in CPC)
 Bit 3    IO-Ch    Direction for Port C, upper bits (always 0=Output in CPC)
 Bit 4    IO-A     Direction for Port A             (0=Output, 1=Input)
 Bit 5,6  MS0,MS1  Mode for Port A and Port Ch      (always zero in CPC)
 Bit 7    SF       Must be "1" to setup the above bits
CAUTION: Writing to PIO Control Register (with Bit7 set), automatically resets PIO Ports A,B,C to 00h each!
In the CPC only Bit 4 is of interest, all other bits are always having the same value.
In order to write to the PSG sound registers, a value of 82h must be written to this register.
In order to read from the keyboard (through PSG register 0Eh), a value of 92h must be written to this register.

PPI Control with Bit7=0
Otherwise, if Bit 7 is "0" then the register is used to set or clear a single bit in Port C:

 Bit 0    B        New value for the specified bit (0=Clear, 1=Set)
 Bit 1-3  N0,N1,N2 Specifies the number of a bit (0-7) in Port C
 Bit 4-6  -        Not Used
 Bit 7    SF       Must be "0" in this case



            */



			cpc_ppi_ports[3]=value;
			//CAUTION: Writing to PIO Control Register (with Bit7 set), automatically resets PIO Ports A,B,C to 00h each!
			if (value&128) {
				cpc_ppi_ports[0]=cpc_ppi_ports[1]=cpc_ppi_ports[2]=0;
				//tambien motor off
				//cpc_cassette_motor_control(0);

                cpc_psg_control_bits=value;
			}

			else {
				//Bit 7 a 0.
				/*
Otherwise, if Bit 7 is "0" then the register is used to set or clear a single bit in Port C:

 Bit 0    B        New value for the specified bit (0=Clear, 1=Set)
 Bit 1-3  N0,N1,N2 Specifies the number of a bit (0-7) in Port C
 Bit 4-6  -        Not Used
 Bit 7    SF       Must be "0" in this case
				*/
				z80_byte valor_bit=value&1;
				z80_byte mascara=1;
				z80_byte numero_bit=(value>>1)&7;
				if (numero_bit) {
					valor_bit=valor_bit << numero_bit;
					mascara=mascara<<numero_bit;
				}
				mascara = ~mascara;

				//printf ("Estableciendo Reg C: numero bit: %d valor: %d. valor antes Reg C: %d\n",numero_bit,valor_bit,cpc_ppi_ports[2]);

				cpc_ppi_ports[2] &=mascara;
				cpc_ppi_ports[2] |=valor_bit;

				//printf ("Valor despues Reg C: %d\n",cpc_ppi_ports[2]);

				if (numero_bit==4) {
					//motor control
					cpc_cassette_motor_control(valor_bit);
				}
			}

                break;


        }

}



void cpc_out_port_crtc(z80_int puerto,z80_byte value)
{

	z80_byte puerto_h=(puerto>>8)&0xFF;

	z80_byte funcion=puerto_h&3;

	switch (funcion) {
		case 0:
			cpc_crtc_last_selected_register=value&31;
			//printf ("Select 6845 register %d\n",cpc_crtc_last_selected_register);
		break;

		case 1:
			//printf ("Write 6845 register %d with 0x%02X\n",cpc_crtc_last_selected_register,value);
			cpc_crtc_registers[cpc_crtc_last_selected_register]=value;

            //Si cambia alguno de los registros de crtc vertical
            if (cpc_crtc_last_selected_register>=4 && cpc_crtc_last_selected_register<=7) {
                //printf("cambiando crtc registro %d valor %d\n",cpc_crtc_last_selected_register,value);
                cpc_reset_last_drawn_line();
            }

            //Ver si autoactivar realvideo
            cpc_if_autoenable_realvideo();
		break;

		case 2:
		break;

		case 3:
		break;
	}

}

const char *cpc_video_modes_strings[4]={
    "160x200, 16 col",
    "320x200, 4 col",
    "640x200, 2 col",
    "160x200, 4 col"
};

void cpc_splash_videomode_change(void) {

	z80_byte modo_video=cpc_gate_registers[2] &3;

        char mensaje[32*24];

        switch (modo_video) {
                case 0:
                case 1:
                case 2:
                        sprintf (mensaje,"Setting screen mode %d, %s",modo_video,cpc_video_modes_strings[modo_video]);
                break;

                case 3:
                        sprintf (mensaje,"Setting screen mode 3, %s (undocumented)",cpc_video_modes_strings[3]);
                break;

                default:
                        //Esto no deberia suceder nunca
                        sprintf (mensaje,"Setting unknown vide mode");
                break;

        }


        screen_print_splash_text_center_no_if_previous(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);


}




z80_int cpc_refresca_ajusta_offet(z80_int direccion_pixel)
{

	//de momento no hacemos nada
	//hasta entender que significa:
/*
On a hardware scrolling screen, there is a problem:
C7FF->C000
CFFF->C800
D7FF->D000
DFFF->D800
E7FF->E000
EFFF->E800
F7FF->F000
FFFF->F800
*/

	//return direccion_pixel+1;

                                        switch (direccion_pixel) {
                                                case 0x07FF:
                                                        direccion_pixel=0x0000;
                                                break;

                                                case 0x0FFF:
                                                        direccion_pixel=0x0800;
                                                break;

                                                case 0x17FF:
                                                        direccion_pixel=0x1000;
                                                break;

                                                case 0x1FFF:
                                                        direccion_pixel=0x1800;
                                                break;

                                                case 0x27FF:
                                                        direccion_pixel=0x2000;
                                                break;

                                                case 0x2FFF:
                                                        direccion_pixel=0x2800;
                                                break;

                                                case 0x37FF:
                                                        direccion_pixel=0x3000;
                                                break;

                                                case 0x3FFF:
                                                        direccion_pixel=0x3800;
                                                break;

						default:
							direccion_pixel++;
						break;


                                        }

	return direccion_pixel;
}

int cpc_crtc_get_total_horizontal(void)
{
    int valor=(cpc_crtc_registers[0]+1)*16;



    return valor;
}


int cpc_crtc_get_total_vertical(void)
{

	int valor=cpc_crtc_registers[4]+1; //en R0 tambien se suma 1

    int alto_caracter=cpc_crtc_get_height_character();
	valor *=alto_caracter;



    return valor;
}

//Zona de pixeles
int cpc_crtc_get_total_pixels_horizontal(void)
{
    int valor=cpc_crtc_registers[1]*16;
    //printf("total pixels horiz: %d\n",valor);
    return valor;
}

//Zona de pixeles
//En scanlines, no en pixels
int cpc_crtc_get_total_pixels_vertical(void)
{

    int alto_caracter=cpc_crtc_get_height_character();

    return cpc_crtc_registers[6]*alto_caracter;
}

int cpc_crtc_get_height_character(void)
{
    return (cpc_crtc_registers[9]&7)+1;
}

int cpc_crtc_get_total_hsync_width(void)
{
    int valor=(cpc_crtc_registers[3] & 15) * 16;
    if (valor==0) valor=16*16; //HSync pulse width in characters (0 means 16 on some CRTC),
    //printf("hsync width: %d\n",valor);
    return valor;
}

int cpc_crtc_get_hsync_position(void)
{
    int valor=cpc_crtc_registers[2]*16;
    //printf("hsync position: %d\n",valor);
    return valor;
}

//Lo que mostramos realmente en pantalla
int cpc_get_maximum_width_window(void)
{
    return CPC_DISPLAY_WIDTH+CPC_LEFT_BORDER_NO_ZOOM*2;
}

int cpc_get_remaining_width_for_borders(void)
{
    return cpc_get_maximum_width_window()-cpc_crtc_get_total_pixels_horizontal();
}

//Retornar tamanyos border ajustando a lo que mostramos en pantalla
void cpc_adjust_vertical_border_sizes(int *p_left,int *p_right)
{
    int remaining=cpc_get_remaining_width_for_borders();

    int right_border_crtc=cpc_crtc_get_hsync_position()-cpc_crtc_get_total_pixels_horizontal();

    int left_border_crtc=cpc_crtc_get_total_horizontal()-cpc_crtc_get_hsync_position()-cpc_crtc_get_total_hsync_width();

    //Si al sumarlos excede lo disponible para border, ajustar los dos

    if (left_border_crtc+right_border_crtc>remaining) {
        //printf("ajustando borders\n");
        left_border_crtc=right_border_crtc=remaining/2;
    }

    *p_left=left_border_crtc;
    *p_right=right_border_crtc;
}

int cpc_crtc_get_total_right_border(void)
{
    int left,right;
    cpc_adjust_vertical_border_sizes(&left,&right);
    return right;
}

int cpc_crtc_get_total_left_border(void)
{
    int left,right;
    cpc_adjust_vertical_border_sizes(&left,&right);
    return left;
}

//Lo que mostramos realmente en pantalla
//En scanlines, no en pixels
int cpc_get_maximum_height_window(void)
{
    return (CPC_DISPLAY_HEIGHT+CPC_TOP_BORDER_NO_ZOOM*2)/2;
}


//En scanlines, no en pixels
int cpc_get_remaining_height_for_borders(void)
{
    return cpc_get_maximum_height_window()-cpc_crtc_get_total_pixels_vertical();
}


//Retornar tamanyos border ajustando a lo que mostramos en pantalla
void cpc_adjust_horizontal_border_sizes(int *p_top,int *p_bottom)
{
    int remaining=cpc_get_remaining_height_for_borders();

    int top_border_crtc=cpc_crtc_get_total_vertical()-cpc_crtc_get_vsync_position()-cpc_crtc_get_total_vsync_height_crtc();

    int bottom_border_crtc=cpc_crtc_get_vsync_position()-cpc_crtc_get_total_pixels_vertical();


    //Si al sumarlos excede lo disponible para border, ajustar los dos

    //printf("remaining: %d top: %d bottom: %d\n",remaining,top_border_crtc,bottom_border_crtc);

    if (top_border_crtc+bottom_border_crtc>remaining) {
        //printf("ajustando borders\n");
        top_border_crtc=bottom_border_crtc=remaining/2;
    }

    //printf("remaining: %d top: %d bottom: %d\n",remaining,top_border_crtc,bottom_border_crtc);

    *p_top=top_border_crtc;
    *p_bottom=bottom_border_crtc;
}



//En scanlines, no en pixels
int cpc_crtc_get_top_border_height(void)
{


    int top,bottom;
    cpc_adjust_horizontal_border_sizes(&top,&bottom);

    return top;
}



//En scanlines, no en pixels
int cpc_crtc_get_bottom_border_height(void)
{

    int top,bottom;
    cpc_adjust_horizontal_border_sizes(&top,&bottom);

    return bottom;

}

void scr_cpc_return_ancho_alto(int *an,int *al,int *al_car,int *off_x)
{

   int alto_caracter=cpc_crtc_get_height_character();




        int ancho_total=cpc_crtc_get_total_pixels_horizontal();
        int total_alto=cpc_crtc_get_total_pixels_vertical();




        //CRTC registro: 2 valor: 46 . Normal
        //CRTC registro: 2 valor: 42. En dynamite dan 2. Esto significa mover el offset 4*16  (4 sale de 46-42)
        int offset_x=(46-cpc_crtc_registers[2])*16;

        //printf ("offset_x: %d\n",offset_x);

        //Controlar maximos
        if (ancho_total>640) ancho_total=640;
        if (total_alto>200) total_alto=200;
        if (offset_x<0) offset_x=0;
        if (offset_x+ancho_total>640) offset_x=640-ancho_total;
        //printf ("offset_x: %d\n",offset_x);



        *an=ancho_total;
        *al=total_alto;
        *al_car=alto_caracter;
        *off_x=offset_x;
}


//Hace putpixel en y e y+1, ya que pantalla cpc es de 640x200 pero hacemos 640x400 la ventana
void cpc_putpixel_zoom(int x,int y,unsigned int color)
{

	int dibujar=0;

	//if (x>255) dibujar=1;
	//else if (y>191) dibujar=1;
	if (scr_ver_si_refrescar_por_menu_activo(x/8,y/8)) dibujar=1;

	if (dibujar) {
		scr_putpixel_zoom(x,y,color);
		scr_putpixel_zoom(x,y+1,color);
	}
}

void cpc_putpixel_border(int x,int y,unsigned int color)
{

	scr_putpixel(x,y,color);

}

void scr_refresca_border_cpc(unsigned int color)
{
//      printf ("Refresco border cpc\n");


    int alto_caracter,ancho_pantalla,alto_pantalla,offset_x_pantalla;
	scr_cpc_return_ancho_alto(&ancho_pantalla,&alto_pantalla,&alto_caracter,&offset_x_pantalla);

	//Control minimos
	if (ancho_pantalla==0) ancho_pantalla=640;
	if (alto_pantalla==0) alto_pantalla=200;


	int ancho_border=CPC_LEFT_BORDER_NO_ZOOM+   (640-ancho_pantalla)/2;
	int alto_border=CPC_TOP_BORDER_NO_ZOOM+ (200-alto_pantalla)/2;

	//printf ("ancho pantalla: %d alto_pantalla: %d offset_x_pantalla: %d anchoborder: %d altoborder: %d\n",ancho_pantalla,alto_pantalla,offset_x_pantalla,ancho_border,alto_border);



        int x,y;

	int x_borde_derecho=(ancho_border+ancho_pantalla)*zoom_x;
	//printf ("x borde derecho: %d total ventana: %d\n",x_borde_derecho,(640+CPC_LEFT_BORDER_NO_ZOOM*2)*zoom_x);


        //parte superior e inferior
        for (y=0;y<alto_border*zoom_y;y++) {
                for (x=0;x<(CPC_DISPLAY_WIDTH+CPC_LEFT_BORDER_NO_ZOOM*2)*zoom_x;x++) {
				//printf ("x: %d y: %d\n",x,y);
                                cpc_putpixel_border(x,y,color);
				cpc_putpixel_border(x,(alto_border+alto_pantalla*2)*zoom_y+y,color);
                }
        }

        //laterales
        for (y=0;y<alto_pantalla*2*zoom_y;y++) {
                for (x=0;x<ancho_border*zoom_x;x++) {
                        cpc_putpixel_border(x,alto_border*zoom_y+y,color);
                        cpc_putpixel_border(x_borde_derecho+x,alto_border*zoom_y+y,color);
                }

        }

}


z80_int cpc_sumar_direccion_pantalla(z80_int direccion_pixel,z80_int sumar)
{

    for (;sumar>0;sumar--) {
        direccion_pixel=cpc_refresca_ajusta_offet(direccion_pixel);
    }

    return direccion_pixel;

}

//Refresco de pantalla CPC sin rainbow
void scr_refresca_pantalla_cpc(void)
{
	z80_byte modo_video=cpc_gate_registers[2] &3;

	if (cpc_forzar_modo_video.v) {
		modo_video=cpc_forzar_modo_video_modo;
	}


	z80_int x,y;

	z80_int offset_tabla;
	z80_byte byte_leido;

	z80_int direccion_pixel;

	int color0,color1,color2,color3;

	int yfinal;
	int bit;

/*
http://www.cpcwiki.eu/index.php/CRTC
The 6845 Registers
The Internal registers of the 6845 are:

Register Index	Register Name	Range	CPC Setting	Notes
0	Horizontal Total	00000000	63	Width of the screen, in characters. Should always be 63 (64 characters). 1 character == 1μs.
1	Horizontal Displayed	00000000	40	Number of characters displayed. Once horizontal character count (HCC) matches this value, DISPTMG is set to 1.
2	Horizontal Sync Position	00000000	46	When to start the HSync signal.
3	Horizontal and Vertical Sync Widths	VVVVHHHH	128+14	HSync pulse width in characters (0 means 16 on some CRTC), should always be more than 8; VSync width in scan-lines. (0 means 16 on some CRTC. Not present on all CRTCs, fixed to 16 lines on these)
4	Vertical Total	x0000000	38	Height of the screen, in characters.
5	Vertical Total Adjust	xxx00000	0	Measured in scanlines, can be used for smooth vertical scrolling on CPC.
6	Vertical Displayed	x0000000	25	Height of displayed screen in characters. Once vertical character count (VCC) matches this value, DISPTMG is set to 1.
7	Vertical Sync position	x0000000	30	When to start the VSync signal, in characters.
8	Interlace and Skew	xxxxxx00	0	00: No interlace; 01: Interlace Sync Raster Scan Mode; 10: No Interlace; 11: Interlace Sync and Video Raster Scan Mode
9	Maximum Raster Address	xxx00000	7	Maximum scan line address on CPC can hold between 0 and 7, higher values' upper bits are ignored
10	Cursor Start Raster	xBP00000	0	Cursor not used on CPC. B = Blink On/Off; P = Blink Period Control (Slow/Fast). Sets first raster row of character that cursor is on to invert.
11	Cursor End Raster	xxx00000	0	Sets last raster row of character that cursor is on to invert
12	Display Start Address (High)	xx000000	32
13	Display Start Address (Low)	00000000	0	Allows you to offset the start of screen memory for hardware scrolling, and if using memory from address &0000 with the firmware.
14	Cursor Address (High)	xx000000	0
15	Cursor Address (Low)	00000000	0
16	Light Pen Address (High)	xx000000		Read Only
17	Light Pen Address (Low)	00000000		Read Only

*/


//http://www.grimware.org/doku.php/documentations/devices/crtc

	z80_int crtc_offset_videoram=cpc_ctrc_get_offset_videoram();
	z80_byte crtc_video_page=(cpc_crtc_registers[12]>>4)&3;
	//printf ("offset: %d video page: %d\n",crtc_offset_videoram,crtc_video_page);



	int alto_caracter,ancho_total,total_alto,offset_x;



	scr_cpc_return_ancho_alto(&ancho_total,&total_alto,&alto_caracter,&offset_x);


/*
For a 16k screen:

screen start byte offset = (((R12 & 0x03)*256) + (R13 & 255))*2

For a 32k screen:

screen start byte offset = (((R12 & 15)*256) + (R13 & 255))*2

On a hardware scrolling screen, there is a problem:
C7FF->C000
CFFF->C800
D7FF->D000
DFFF->D800
E7FF->E000
EFFF->E800
F7FF->F000
FFFF->F800
*/







	//Temporal. Quiza no tener que inicializar la tabla cada vez??? Esta tabla
	//sale tal cual de init_cpc_line_display_table pero cambiando valor 80
        int yy;
        z80_int offset;
        for (yy=0;yy<200;yy++) {
                //offset=((yy / 8) * cpc_crtc_registers[1]*2) + ((yy % 8) * 2048);
                offset=((yy / alto_caracter) * cpc_crtc_registers[1]*2) + ((yy % alto_caracter) * 2048);
                cpc_line_display_table[yy]=offset;
        }




	for (y=0;y<total_alto;y++){
		yfinal=y*2;
		offset_tabla=cpc_line_display_table[y];
		//direccion_pixel=offset_tabla+crtc_offset_videoram;

        //sumar teniendo en cuenta los wraparound
        direccion_pixel=offset_tabla;

        direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,crtc_offset_videoram);


		//puntero=cpc_ram_mem_table[3]+offset_tabla;

		//x lo incrementa cada modo por separado
		//for (x=0;x<640;) {
		for (x=offset_x;x<ancho_total+offset_x;) {
			switch (modo_video) {
		                case 0:
                		        //printf ("Mode 0, 160x200 resolution, 16 colours\n");
					//Cada pixel por cuaduplicado
					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb

					//if (crtc_offset_videoram) direccion_pixel=cpc_refresca_ajusta_offet(direccion_pixel);
					//else direccion_pixel++;

					direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

					color0=(byte_leido&128)>>7 | (byte_leido&8)>>2 | (byte_leido&32)>>3 | (byte_leido&2)<<2;
					color1=(byte_leido&64)>>6  | (byte_leido&4)>>1 | (byte_leido&16)>>2 | (byte_leido&1)<<3;

					color0=cpc_palette_table[color0];
                    color0 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom(x++,yfinal,color0);
                    cpc_putpixel_zoom(x++,yfinal,color0);
                    cpc_putpixel_zoom(x++,yfinal,color0);
                    cpc_putpixel_zoom(x++,yfinal,color0);

					color1=cpc_palette_table[color1];
                    color1 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom(x++,yfinal,color1);
                    cpc_putpixel_zoom(x++,yfinal,color1);
                    cpc_putpixel_zoom(x++,yfinal,color1);
                    cpc_putpixel_zoom(x++,yfinal,color1);


		                break;

                		case 1:

/*
Mode 1, 320×200, 4 colors (each byte of video memory represents 4 pixels):
bit 7	        bit 6	        bit 5	        bit 4	        bit 3	        bit 2	        bit 1	        bit 0
pixel 0 (bit 1)	pixel 1 (bit 1)	pixel 2 (bit 1)	pixel 3 (bit 1)	pixel 0 (bit 0)	pixel 1(bit 0)	pixel 2 (bit 0)	pixel 3 (bit 0)
*/
		                        //printf ("Mode 1, 320x200 resolution, 4 colours\n");
					//Duplicamos cada pixel en ancho
					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb

					//if (crtc_offset_videoram) direccion_pixel=cpc_refresca_ajusta_offet(direccion_pixel);
					//else direccion_pixel++;
					direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

					color0=(byte_leido&128)>>7 | ((byte_leido&8)>>2);
					//if (cpc_palette_table[color0]!=4) printf ("color0: %d valor: %d\n",color0,cpc_palette_table[color0]);
					color0=cpc_palette_table[color0];
					color0 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom(x++,yfinal,color0);
					cpc_putpixel_zoom(x++,yfinal,color0);


					color1=(byte_leido&64)>>6 | ((byte_leido&4)>>1);
					color1=cpc_palette_table[color1];
					color1 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom(x++,yfinal,color1);
					cpc_putpixel_zoom(x++,yfinal,color1);


					color2=(byte_leido&32)>>5 | ((byte_leido&2));
					color2=cpc_palette_table[color2];
					color2 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom(x++,yfinal,color2);
					cpc_putpixel_zoom(x++,yfinal,color2);


					color3=(byte_leido&16)>>4 | ((byte_leido&1)<<1   );
					color3=cpc_palette_table[color3];
					color3 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom(x++,yfinal,color3);
					cpc_putpixel_zoom(x++,yfinal,color3);


                		break;

		                case 2:
                		        //printf ("Mode 2, 640x200 resolution, 2 colours\n");
					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb

					//if (crtc_offset_videoram) direccion_pixel=cpc_refresca_ajusta_offet(direccion_pixel);
					//else direccion_pixel++;
					direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

					for (bit=0;bit<8;bit++) {
						color0=(byte_leido&128)>>7;
						color0=cpc_palette_table[color0];
                        color0 +=CPC_INDEX_FIRST_COLOR;
                        cpc_putpixel_zoom(x++,yfinal,color0);
						byte_leido=byte_leido<<1;
					}
		                break;

                		case 3:
		                        //printf ("Mode 3, 160x200 resolution, 4 colours (undocumented)\n");
					//temp
					//http://cpctech.cpc-live.com/docs/graphics.html
					//if (crtc_offset_videoram) direccion_pixel=cpc_refresca_ajusta_offet(direccion_pixel);
					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb
                    direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

                    color0=(byte_leido&128)>>7 | ((byte_leido&8)>>2);
                    //if (cpc_palette_table[color0]!=4) printf ("color0: %d valor: %d\n",color0,cpc_palette_table[color0]);
                    color0=cpc_palette_table[color0];
                    color0 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom(x++,yfinal,color0);
                    cpc_putpixel_zoom(x++,yfinal,color0);
                    cpc_putpixel_zoom(x++,yfinal,color0);
                    cpc_putpixel_zoom(x++,yfinal,color0);


                    color1=(byte_leido&64)>>6 | ((byte_leido&4)>>1);
                    color1=cpc_palette_table[color1];
                    color1 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom(x++,yfinal,color1);
                    cpc_putpixel_zoom(x++,yfinal,color1);
                    cpc_putpixel_zoom(x++,yfinal,color1);
                    cpc_putpixel_zoom(x++,yfinal,color1);

                    break;
		        }

		}
	}


}




void scr_refresca_pantalla_y_border_cpc_no_rainbow(void)
{

    //Refrescar border si conviene
    if (border_enabled.v) {
        if (modificado_border.v) {
            //Dibujar border. Color 0
            unsigned int color=cpc_border_color;
            //color=cpc_palette_table[color];
            color +=CPC_INDEX_FIRST_COLOR;

            scr_refresca_border_cpc(color);
            modificado_border.v=0;
        }
	}


        scr_refresca_pantalla_cpc();
}



//Refresco pantalla con rainbow
void scr_refresca_pantalla_y_border_cpc_rainbow(void)
{



	int ancho,alto;

	ancho=get_total_ancho_rainbow();
	alto=get_total_alto_rainbow();

	int x,y;



	z80_int color_pixel;
	z80_int *puntero;

	puntero=rainbow_buffer;


    //printf("ultima linea al hacer render: %d\n",cpc_last_drawn_line);

    //Si dibujar lineas por debajo del borde inferior en negro
    int drawing_black=0;

    if (cpc_pending_last_drawn_lines_black.v==1 && cpc_pending_last_drawn_lines_black_counter==0) {
        drawing_black=1;
        cpc_pending_last_drawn_lines_black.v=0;
        debug_printf(VERBOSE_DEBUG,"Drawing black lines after lower border (elapsed 1 second since last CRTC vertical changes)");
    }

	for (y=0;y<alto;y++) {


		for (x=0;x<ancho;x++) {


            //Si dibujamos algo que esta por debajo del border inferior y que no estará inicializado con nada
            if (drawing_black && y>=cpc_last_drawn_line) {
                *puntero=0;
            }

            else {
                color_pixel=*puntero;
            }

            puntero++;
            scr_putpixel_zoom_rainbow(x,y,color_pixel);


		}

	}




}



void scr_refresca_pantalla_y_border_cpc(void)
{
    if (rainbow_enabled.v) {
        scr_refresca_pantalla_y_border_cpc_rainbow();
    }
    else {
        scr_refresca_pantalla_y_border_cpc_no_rainbow();
    }
}

void cpc_putpixel_zoom_rainbow(int x,z80_int *puntero_buf_rainbow,int color)
{
    puntero_buf_rainbow[x]=color;
    //Siempre es x2 de alto
    puntero_buf_rainbow[x+get_total_ancho_rainbow()]=color;
}

void screen_store_scanline_rainbow_solo_border_cpc(void)
{
    int ancho_total,total_alto,offset_x;



            //sacar los limites pero sin fijar a 640x200 como en el caso de no rainbow

        //alto_caracter=cpc_crtc_get_height_character();

        ancho_total=cpc_crtc_get_total_pixels_horizontal();
        total_alto=cpc_crtc_get_total_pixels_vertical();



        offset_x=cpc_crtc_get_total_left_border();


/*
#define CPC_LEFT_BORDER_NO_ZOOM 64
#define CPC_TOP_BORDER_NO_ZOOM 72

#define CPC_LEFT_BORDER CPC_LEFT_BORDER_NO_ZOOM*zoom_x
#define CPC_TOP_BORDER CPC_TOP_BORDER_NO_ZOOM*zoom_y

#define CPC_DISPLAY_WIDTH 640
#define CPC_DISPLAY_HEIGHT 400
*/

    int ancho_maximo=CPC_DISPLAY_WIDTH+CPC_LEFT_BORDER_NO_ZOOM*2;
    int alto_maximo=(CPC_DISPLAY_HEIGHT+CPC_TOP_BORDER_NO_ZOOM*2)/2;

    //printf("ancho total: %d\n",ancho_total);

    if (ancho_total>ancho_maximo) ancho_total=ancho_maximo;
    if (total_alto>alto_maximo) total_alto=alto_maximo;
    if (offset_x<0) offset_x=0;
    if (offset_x+ancho_total>ancho_maximo) offset_x=ancho_maximo-ancho_total;



    int inicio_pantalla;
    int final_pantalla;


    //printf("ancho total despues limite: %d ancho maximo %d borde_izqu %d offset_x: %d\n",ancho_total,ancho_maximo,borde_izq,offset_x);


    //int total_scanlines=(CPC_DISPLAY_HEIGHT+CPC_TOP_BORDER_NO_ZOOM*2)/2;

    int borde_superior=cpc_crtc_get_top_border_height();

    inicio_pantalla=borde_superior;



    final_pantalla=inicio_pantalla+total_alto;



  //Si en zona pantalla (no border superior ni inferior)
  //printf("margenes: %d %d\n",inicio_pantalla,final_pantalla);
  //Borde superior o inferior

  if (cpc_convert_scanline_to_final_y()<inicio_pantalla || cpc_convert_scanline_to_final_y()>=final_pantalla) {



        //linea en coordenada display (no border) que se debe leer
        //int y_display=t_scanline_draw-inicio_pantalla;

 			unsigned int color=cpc_border_color;
			//color=cpc_palette_table[color];
			color +=CPC_INDEX_FIRST_COLOR;


    if (cpc_debug_borders.v) {
        if (cpc_convert_scanline_to_final_y()<inicio_pantalla) {

            color=1;
        }

        else {
            color=2;
        }

    }





    //Nos ubicamos ya en la zona de pixeles, saltando el border
    //En esta capa, si color=0, no lo ponemos como transparente sino como color negro
    z80_int *puntero_buf_rainbow;

    //printf("%d\n",t_scanline_draw);

   int y_destino_rainbow=cpc_convert_scanline_to_final_y()*2;

    if (y_destino_rainbow>cpc_last_drawn_line) {
        cpc_last_drawn_line=y_destino_rainbow;
    }
   //printf("ultima linea: %d\n",cpc_last_drawn_line);
   //printf("y destino: %d\n",y_destino_rainbow);
   //printf("vsync position: %d\n",cpc_crtc_get_vsync_position());


    //Nota: en cpc rom, vsync esta en 240 (posicion y=480). Pero tenemos una ventana de 544 de alto para soportar overscan
    //Por tanto, a no ser que el juego use modo overscan, la parte de abajo normalmente va a estar sin usar

    //TODO: calculo con border desactivado


    //Limite inferior y superior. Sobretodo el inferior, pues puede ser negativo (en zona border invisible)
    //En teoria superior no deberia ser mayor, pero por si acaso
    int max_y=get_total_alto_rainbow();

    if (y_destino_rainbow<0 || y_destino_rainbow>=max_y) {
        //printf("y por encima del maximo: %d\n",y_destino_rainbow);
        return;
    }

    puntero_buf_rainbow=&rainbow_buffer[y_destino_rainbow*get_total_ancho_rainbow()];






    int x;

    for (x=0;x<cpc_get_maximum_width_window();x++) {
       cpc_putpixel_zoom_rainbow(x,puntero_buf_rainbow,color);
    }

  }


  //Borde izquierdo y derecho

  if (cpc_convert_scanline_to_final_y()>=inicio_pantalla && cpc_convert_scanline_to_final_y()<final_pantalla) {
      //if (t_scanline_draw>=final_pantalla) {




        //linea en coordenada display (no border) que se debe leer
        //int y_display=t_scanline_draw-inicio_pantalla;



        int left_border=cpc_crtc_get_total_left_border();
        int right_border=cpc_crtc_get_total_right_border();


        //printf("borders: %d %d\n",left_border,right_border);


    //Nos ubicamos ya en la zona de pixeles, saltando el border
    //En esta capa, si color=0, no lo ponemos como transparente sino como color negro
    z80_int *puntero_buf_rainbow;

    //printf("%d\n",t_scanline_draw);

   int y_destino_rainbow=cpc_convert_scanline_to_final_y()*2;
   //printf("y destino: %d\n\n",y_destino_rainbow);

    //TODO: calculo con border desactivado

    //Limite inferior y superior. Sobretodo el inferior, pues puede ser negativo (en zona border invisible)
    //En teoria superior no deberia ser mayor, pero por si acaso
    int max_y=get_total_alto_rainbow();

    if (y_destino_rainbow<0 || y_destino_rainbow>=max_y) {
        //printf("y por encima del maximo: %d\n",y_destino_rainbow);
        return;
    }

    puntero_buf_rainbow=&rainbow_buffer[y_destino_rainbow*get_total_ancho_rainbow()];

    unsigned int color=cpc_border_color;
    //color=cpc_palette_table[color];
    color +=CPC_INDEX_FIRST_COLOR;



    if (cpc_debug_borders.v) {
        color=3;
    }

    int x;

    for (x=0;x<left_border;x++) {
       cpc_putpixel_zoom_rainbow(x,puntero_buf_rainbow,color);
    }


    if (cpc_debug_borders.v) {
        color=4;
    }

    int offset_right=left_border+cpc_crtc_get_total_pixels_horizontal();
    for (x=0;x<right_border;x++) {
       cpc_putpixel_zoom_rainbow(x+offset_right,puntero_buf_rainbow,color);
    }

  }



}




//Guardar en buffer rainbow la linea actual.
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
//void screen_store_scanline_rainbow_solo_display_cpc(z80_int *scanline_buffer,z80_byte *vram_memory_pointer)
void screen_store_scanline_rainbow_solo_display_cpc(void)
{
    int alto_caracter,ancho_total,total_alto,offset_x;



    //sacar los limites pero sin fijar a 640x200 como en el caso de no rainbow

    alto_caracter=cpc_crtc_get_height_character();

    ancho_total=cpc_crtc_get_total_pixels_horizontal();
    total_alto=cpc_crtc_get_total_pixels_vertical();

    //printf("Bordes: %d %d\n",cpc_crtc_get_total_left_border(),cpc_crtc_get_total_right_border() );




    offset_x=cpc_crtc_get_total_left_border();




    int ancho_maximo=CPC_DISPLAY_WIDTH+CPC_LEFT_BORDER_NO_ZOOM*2;
    int alto_maximo=(CPC_DISPLAY_HEIGHT+CPC_TOP_BORDER_NO_ZOOM*2)/2;

    //printf("ancho total: %d\n",ancho_total);

    if (ancho_total>ancho_maximo) ancho_total=ancho_maximo;
    if (total_alto>alto_maximo) total_alto=alto_maximo;
    if (offset_x<0) offset_x=0;
    if (offset_x+ancho_total>ancho_maximo) offset_x=ancho_maximo-ancho_total;



    int inicio_pantalla;
    int final_pantalla;


    //printf("ancho total despues limite: %d ancho maximo %d borde_izqu %d offset_x: %d\n",ancho_total,ancho_maximo,borde_izq,offset_x);


    //int total_scanlines=(CPC_DISPLAY_HEIGHT+CPC_TOP_BORDER_NO_ZOOM*2)/2;

    int borde_superior=cpc_crtc_get_top_border_height();

    inicio_pantalla=borde_superior;



    final_pantalla=inicio_pantalla+total_alto;

  //Si en zona pantalla (no border superior ni inferior)
  //printf("margenes: %d %d\n",inicio_pantalla,final_pantalla);
  if (cpc_convert_scanline_to_final_y()>=inicio_pantalla && cpc_convert_scanline_to_final_y()<final_pantalla) {

        //if (cpc_convert_scanline_to_final_y()==inicio_pantalla) printf("Render inicio pantalla t_scanline_draw %d\n",t_scanline_draw);
        //linea en coordenada display (no border) que se debe leer
        int y_display=cpc_convert_scanline_to_final_y()-inicio_pantalla;






//Renderiza una linea de display (pantalla y sprites, pero no border)



    //Nos ubicamos ya en la zona de pixeles, saltando el border
    //En esta capa, si color=0, no lo ponemos como transparente sino como color negro
    z80_int *puntero_buf_rainbow;

    //printf("%d\n",t_scanline_draw);

   int y_destino_rainbow=cpc_convert_scanline_to_final_y()*2;


    //TODO: calculo con border desactivado




    //Limite inferior y superior. Sobretodo el inferior, pues puede ser negativo (en zona border invisible)
    //En teoria superior no deberia ser mayor, pero por si acaso
    int max_y=get_total_alto_rainbow();

    if (y_destino_rainbow<0 || y_destino_rainbow>=max_y) return;

    puntero_buf_rainbow=&rainbow_buffer[y_destino_rainbow*get_total_ancho_rainbow()];


    //*puntero_buf_rainbow=99;



	z80_byte modo_video=cpc_gate_registers[2] &3;

	if (cpc_forzar_modo_video.v) {
		modo_video=cpc_forzar_modo_video_modo;
	}



    z80_byte byte_leido;



    int x;
    int color0,color1,color2,color3;

        z80_int crtc_offset_videoram=cpc_ctrc_get_offset_videoram();
        z80_byte crtc_video_page=(cpc_crtc_registers[12]>>4)&3;
        //printf ("offset: %d video page: %d\n",crtc_offset_videoram,crtc_video_page);






        int bit;


        z80_int direccion_pixel;
        int yfinal;
        z80_int offset_tabla;


        yfinal=y_display;
        //offset_tabla=cpc_line_display_table[yfinal];

        int char_row=yfinal / alto_caracter;
        int char_scanline=yfinal % alto_caracter;

        offset_tabla=(char_row * cpc_crtc_registers[1]*2) + (char_scanline * 2048);


        //sumar teniendo en cuenta los wraparound
        direccion_pixel=offset_tabla;

        direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,crtc_offset_videoram);




        //printf("yfinal: %d char_row: %d char_scanline: %d crtc_offset_videoram: %d (%04XH) offset_tabla: %d (%04XH) direccion_pixel: %d (%04XH)\n",
        //    yfinal,char_row,char_scanline,crtc_offset_videoram,crtc_offset_videoram,offset_tabla,offset_tabla,direccion_pixel,direccion_pixel);







    	for (x=offset_x;x<ancho_total+offset_x;) {
			switch (modo_video) {
		                case 0:  //160x200



                		        //printf ("Mode 0, 160x200 resolution, 16 colours\n");
					//Cada pixel por cuaduplicado
					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb


					direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

					color0=(byte_leido&128)>>7 | (byte_leido&8)>>2 | (byte_leido&32)>>3 | (byte_leido&2)<<2;
					color1=(byte_leido&64)>>6  | (byte_leido&4)>>1 | (byte_leido&16)>>2 | (byte_leido&1)<<3;

					color0=cpc_palette_table[color0];
                    color0 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);

					color1=cpc_palette_table[color1];
                    color1 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);


		    break;


               		case 1:

/*
Mode 1, 320×200, 4 colors (each byte of video memory represents 4 pixels):
bit 7	        bit 6	        bit 5	        bit 4	        bit 3	        bit 2	        bit 1	        bit 0
pixel 0 (bit 1)	pixel 1 (bit 1)	pixel 2 (bit 1)	pixel 3 (bit 1)	pixel 0 (bit 0)	pixel 1(bit 0)	pixel 2 (bit 0)	pixel 3 (bit 0)
*/
		                        //printf ("Mode 1, 320x200 resolution, 4 colours\n");
					//Duplicamos cada pixel en ancho
					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb


                    direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

					color0=(byte_leido&128)>>7 | ((byte_leido&8)>>2);
					//if (cpc_palette_table[color0]!=4) printf ("color0: %d valor: %d\n",color0,cpc_palette_table[color0]);
					color0=cpc_palette_table[color0];
					color0 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);


					color1=(byte_leido&64)>>6 | ((byte_leido&4)>>1);
					color1=cpc_palette_table[color1];
					color1 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);


					color2=(byte_leido&32)>>5 | ((byte_leido&2));
					color2=cpc_palette_table[color2];
					color2 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color2);
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color2);


					color3=(byte_leido&16)>>4 | ((byte_leido&1)<<1   );
					color3=cpc_palette_table[color3];
					color3 +=CPC_INDEX_FIRST_COLOR;
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color3);
					cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color3);


                		break;


		                case 2:
                		        //printf ("Mode 2, 640x200 resolution, 2 colours\n");
					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb


					direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

					for (bit=0;bit<8;bit++) {
						color0=(byte_leido&128)>>7;
						color0=cpc_palette_table[color0];
	                                        color0 +=CPC_INDEX_FIRST_COLOR;
        	                                cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
						byte_leido=byte_leido<<1;
					}
		                break;



                		case 3:
		                        //printf ("Mode 3, 160x200 resolution, 4 colours (undocumented)\n");
					//temp
					//http://cpctech.cpc-live.com/docs/graphics.html
					//if (crtc_offset_videoram) direccion_pixel=cpc_refresca_ajusta_offet(direccion_pixel);


					byte_leido=*(cpc_ram_mem_table[crtc_video_page]+(direccion_pixel&16383) ); //Solo offset dentro de 16kb


                    direccion_pixel=cpc_sumar_direccion_pantalla(direccion_pixel,1);

                    color0=(byte_leido&128)>>7 | ((byte_leido&8)>>2);
                    //if (cpc_palette_table[color0]!=4) printf ("color0: %d valor: %d\n",color0,cpc_palette_table[color0]);
                    color0=cpc_palette_table[color0];
                    color0 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color0);


                    color1=(byte_leido&64)>>6 | ((byte_leido&4)>>1);
                    color1=cpc_palette_table[color1];
                    color1 +=CPC_INDEX_FIRST_COLOR;
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);
                    cpc_putpixel_zoom_rainbow(x++,puntero_buf_rainbow,color1);

                		break;



            default:

            //Aqui no deberia llegar nunca. pero por si acaso

            x++;
            break;

    }
    }



  }


}



void screen_store_scanline_rainbow_cpc_border_and_display(void)
{
    //Renderizar border y pantalla en buffer rainbow

    screen_store_scanline_rainbow_solo_display_cpc();
    screen_store_scanline_rainbow_solo_border_cpc();
}

void cpc_reset(void)
{
		cpc_gate_registers[0]=cpc_gate_registers[1]=cpc_gate_registers[2]=cpc_gate_registers[3]=0;
        cpc_port_df=0;
		cpc_set_memory_pages();
		cpc_scanline_counter=0;
        cpc_crt_pending_interrupt.v=0;
        cpc_reset_last_drawn_line();
}

void cpc_if_autoenable_realvideo(void)
{
    if (rainbow_enabled.v==0 && autodetect_rainbow.v) {
        //cpc_crtc_get_total_pixels_horizontal(),
        int alto_zona_pixeles=cpc_crtc_get_total_pixels_vertical();
        if (alto_zona_pixeles>200 || alto_zona_pixeles<192) {
            debug_printf(VERBOSE_INFO,"Autoenabling realvideo because video height not standard");
            //printf("Autoenabling realvideo because video height not standard\n");
            enable_rainbow();
        }
    }
}


void cpc_if_autoenable_realvideo_on_changemodes(void)
{
    if (rainbow_enabled.v==0 && autodetect_rainbow.v) {
        if (cpc_video_modes_change_frame_counter>=2) {
            debug_printf(VERBOSE_INFO,"Autoenabling realvideo because 2 or mode video mode changes in a frame");
            //printf("Autoenabling realvideo because 2 or mode video mode changes in a frame (%d)\n",cpc_video_modes_change_frame_counter);
            enable_rainbow();
        }
    }
}


void cpc_if_autoenable_realvideo_on_changeborder(void)
{
    if (rainbow_enabled.v==0 && autodetect_rainbow.v) {
        if (cpc_border_change_frame_counter>=3) {
            debug_printf(VERBOSE_INFO,"Autoenabling realvideo because 3 or more border changes in a frame");
            //printf("Autoenabling realvideo because 3 or more border changes in a frame\n");
            enable_rainbow();
        }
    }
}


void cpc_out_port_fa7e(z80_byte value)
{

    //Port FA7Eh - Floppy Motor On/Off Flipflop
    //Writing 00h to Port FA7Eh turns all disk drive motors off,
    //writing 01h turns all motors on. It is not possible to turn on/off the motor of a specific drive separately.


    if (value&1) {
        dsk_show_activity();
        pd765_motor_on();
    }
    else {
        //Pues realmente si motor va a off, no hay actividad
        pd765_motor_off();
    }

}