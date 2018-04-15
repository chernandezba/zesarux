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
#include "tbblue.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "utils.h"
#include "menu.h"
#include "divmmc.h"
#include "diviface.h"
#include "screen.h"

#include "timex.h"
#include "ula.h"
#include "audio.h"

#define TBBLUE_MAX_SRAM_8KB_BLOCKS 224

//Punteros a los 64 bloques de 8kb de ram de spectrum
z80_byte *tbblue_ram_memory_pages[TBBLUE_MAX_SRAM_8KB_BLOCKS];

//2MB->224 bloques (16+16+64*3) (3 bloques extra de 512)
//1.5 MB->160 bloques (2 bloques extra de 512)
//1 MB->96 bloques (1 bloque extra de 512)
//512 KB->32 bloques de (0 bloque extra de 512)

z80_byte tbblue_extra_512kb_blocks=3;

static int tbblue_copper_wait_cond_fired(void);
static int tbblue_get_current_raster_position(void);

z80_byte tbblue_return_max_extra_blocks(void)
{
	return 32+tbblue_extra_512kb_blocks*64;
}

//Retorna ram total en KB
int tbblue_get_current_ram(void)
{
	return 256+8*tbblue_return_max_extra_blocks();
}

//Punteros a los 8 bloques de 8kb de rom de spectrum
z80_byte *tbblue_rom_memory_pages[8];

z80_byte *tbblue_fpga_rom;

//Memoria mapeada, en 8 bloques de 8 kb
z80_byte *tbblue_memory_paged[8];


//Si arranca rapido sin pasar por el proceso de boot. Va directamente a rom 48k
z80_bit tbblue_fast_boot_mode={0};


//Copper
z80_byte tbblue_copper_memory[TBBLUE_COPPER_MEMORY];

//Indice a la posicion de 16 bits a escribir
//z80_int tbblue_copper_index_write=0;

//Indice al opcode copper a ejecutar
z80_int tbblue_copper_pc=0;

//Obtiene posicion de escritura del copper
z80_int tbblue_copper_get_write_position(void)
{
	z80_int posicion;
	posicion=tbblue_registers[97] | ((tbblue_registers[98]&7)<<8);
	return posicion;
}

//Establece posicion de escritura del copper
void tbblue_copper_set_write_position(z80_int posicion)
{
	tbblue_registers[97]=posicion%0xFF;

	z80_byte msb=(posicion>>8)&7; //3 bits bajos

	z80_byte reg98=tbblue_registers[98];
	reg98 &=(255-7);
	reg98 |=msb;
	tbblue_registers[98]=reg98;
}

void tbblue_copper_increment_write_position(void)
{
	z80_int posicion=tbblue_copper_get_write_position();

	posicion++;
	tbblue_copper_set_write_position(posicion);
}


//Escribe dato copper en posicion de escritura
void tbblue_copper_write_data(z80_byte value)
{
	z80_int posicion=tbblue_copper_get_write_position();

	posicion &=(TBBLUE_COPPER_MEMORY-1);


	//printf ("Writing copper data index %d data %02XH\n",posicion,value);

	tbblue_copper_memory[posicion]=value;

	posicion++;
	tbblue_copper_set_write_position(posicion);

}

//Devuelve el byte donde apunta indice
z80_byte tbblue_copper_get_byte(z80_int posicion)
{
	posicion &=(TBBLUE_COPPER_MEMORY-1);
	return tbblue_copper_memory[posicion];
}

//Devuelve el byte donde apunta pc
z80_byte tbblue_copper_get_byte_pc(void)
{
	return tbblue_copper_get_byte(tbblue_copper_pc);

}

void tbblue_copper_get_wait_opcode_parameters(z80_int *line, z80_int *horiz)
{
	z80_byte byte_a=tbblue_copper_get_byte(tbblue_copper_pc);
	z80_byte byte_b=tbblue_copper_get_byte(tbblue_copper_pc+1);

	*line=byte_b|((byte_a&1)<<8);
	*horiz=((byte_a>>1)&63);
}

void tbblue_copper_reset_pc(void)
{
	tbblue_copper_pc=0;
}

void tbblue_copper_set_stop(void)
{
	tbblue_registers[98] &=63;
}

void tbblue_copper_next_opcode(void)
{
	//Incrementar en 2. 
	tbblue_copper_pc +=2;

  /*
                                                        modos
                                                               01 = Copper start, execute the list, then stop at last adress
       10 = Copper start, execute the list, then loop the list from start
       11 = Copper start, execute the list and restart the list at each frame
                                                        */

                                                   //Si ha ido a posicion 0
                                                   if (tbblue_copper_pc==TBBLUE_COPPER_MEMORY) {
													   z80_byte copper_control_bits=tbblue_copper_get_control_bits();
                                                           switch (copper_control_bits) {
                                                                        case TBBLUE_RCCH_COPPER_STOP:
																			//Se supone que nunca se estara ejecutando cuando el mode sea stop
                                                                           tbblue_copper_set_stop();
                                                                        break;

                                                                        case TBBLUE_RCCH_COPPER_RUN_LOOP:
                                                                                //loop
                                                                                tbblue_copper_pc=0;
                                                                                //printf ("Reset copper on mode TBBLUE_RCCH_COPPER_RUN_LOOP\n");
                                                                        break;

																		case TBBLUE_RCCH_COPPER_RUN_LOOP_RESET:
                                                                                //loop
                                                                                tbblue_copper_pc=0;
                                                                                //printf ("Reset copper on mode TBBLUE_RCCH_COPPER_RUN_LOOP_RESET\n");
                                                                        break;

                                                                        case TBBLUE_RCCH_COPPER_RUN_VBI:
                                                                                //loop??
                                                                                tbblue_copper_pc=0;
                                                                                //printf ("Reset copper on mode RUN_VBI\n");
                                                                        break;
                                                           }
												   }

}

//Ejecuta opcodes del copper // hasta que se encuentra un wait
void tbblue_copper_run_opcodes(void)
{
	z80_byte byte_leido=0;

		byte_leido=tbblue_copper_get_byte_pc();
		if ( (byte_leido&128)==0) {
			//Es un move
			z80_byte indice_registro=byte_leido&127;
			//tbblue_copper_pc++;
			z80_byte valor_registro=tbblue_copper_get_byte(tbblue_copper_pc+1);
			//tbblue_copper_pc++;
			//printf ("Executing MOVE register %02XH value %02XH\n",indice_registro,valor_registro);
			tbblue_set_value_port_position(indice_registro,valor_registro);

			tbblue_copper_next_opcode();

		}
		else {
			//Es un wait
			//Si se cumple, saltar siguiente posicion
			z80_int UNUSED(linea), UNUSED(horiz);
			//tbblue_copper_get_wait_opcode_parameters(&linea,&horiz);
			if (tbblue_copper_wait_cond_fired () ) {
                                                        //printf ("Wait condition positive at copper_pc %02XH scanline %d raster %d\n",tbblue_copper_pc,t_scanline,tbblue_get_current_raster_position() );
                                                        tbblue_copper_next_opcode();
                                                        //printf ("Wait condition positive, after incrementing copper_pc %02XH\n",tbblue_copper_pc);
			}
			//printf ("Waiting until scanline %d horiz %d\n",linea,horiz);
			
		}
	
}

z80_byte tbblue_copper_get_control_bits(void)
{
	//z80_byte control=(tbblue_registers[98]>>6)&3;
	z80_byte control=(tbblue_registers[98])&(128+64);
	/*

# define(`__RCCH_COPPER_STOP', 0x00)
# define(`__RCCH_COPPER_RUN_LOOP_RESET', 0x40)
# define(`__RCCH_COPPER_RUN_LOOP', 0x80)
# define(`__RCCH_COPPER_RUN_VBI', 0xc0)

	*/
	return control;
}



/*int tbblue_copper_is_opcode_wait(void)
{
	z80_byte byte_leido=tbblue_copper_get_byte_pc();
	if ( (byte_leido&128) ) return 1;
	return 0;
}*/

//Si scanline y posicion actual corresponde con instruccion wait
static int tbblue_copper_wait_cond_fired(void)
{
	//int scanline_actual=t_scanline;


	int current_horizontal=tbblue_get_current_raster_horiz_position();

	//Obtener parametros de instruccion wait
	z80_int linea, horiz;
	tbblue_copper_get_wait_opcode_parameters(&linea,&horiz);

	int current_raster=tbblue_get_current_raster_position();

	//printf ("Waiting until raster %d horiz %d. current %d\n",linea,horiz,current_raster);

	//comparar vertical
	if (current_raster==linea) {
		//comparar horizontal
		//printf ("Comparing current %d to %d\n",current_horizontal,horiz);
		if (current_horizontal>=horiz) {
			//printf ("Fired wait condition %d,%d at %d,%d (t-states %d)\n",linea,horiz,current_raster,current_horizontal,
			//		t_estados % screen_testados_linea);
			return 1;
		}
	}

	return 0;
}



void tbblue_copper_handle_next_opcode(void)
{

	//Si esta activo copper
    z80_byte copper_control_bits=tbblue_copper_get_control_bits();
    if (copper_control_bits != TBBLUE_RCCH_COPPER_STOP) {
        //printf ("running copper %d\n",tbblue_copper_pc);
        tbblue_copper_run_opcodes();
	}
}                                           

 

void tbblue_copper_handle_vsync(void)
{
	z80_byte copper_control_bits=tbblue_copper_get_control_bits();
    if (copper_control_bits==TBBLUE_RCCH_COPPER_RUN_VBI) {
    	tbblue_copper_reset_pc();
        //printf ("Reset copper on control bit 3 on vsync\n");
    }
                                                   
}


void tbblue_copper_write_control_hi_byte(z80_byte value)
{
	/*

# define(`__RCCH_COPPER_STOP', 0x00)
# define(`__RCCH_COPPER_RUN_LOOP_RESET', 0x40)
# define(`__RCCH_COPPER_RUN_LOOP', 0x80)
# define(`__RCCH_COPPER_RUN_VBI', 0xc0)

# STOP causes the copper to stop executing instructions
# and hold the instruction pointer at its current position.
#
# RUN_LOOP_RESET causes the copper to reset its instruction
# pointer to 0 and run in LOOP mode (see next).
#
# RUN_LOOP causes the copper to restart with the instruction
# pointer at its current position.  Once the end of the instruction
# list is reached, the copper loops back to the beginning.
#
# RUN_VBI causes the copper to reset its instruction
# pointer to 0 and run in VBI mode.  On vsync interrupt,
# the copper restarts the instruction list from the beginning.

# Note that modes RUN_LOOP_RESET and RUN_VBI will only reset
# the instruction pointer to zero if the mode actually changes
# to RUN_LOOP_RESET or RUN_VBI.  Writing the same mode in a
# second write will not cause the instruction pointer to zero.

# It is possible to write values into the copper's instruction
# space while it is running and since the copper constantly
# refetches a wait instruction it is executing, you can cause
# the wait instruction to end prematurely by changing it to
# something else.

*/

	z80_byte action=value&(128+64);

	switch (action) {
		//Estos dos casos, resetean el puntero de instruccion
		case TBBLUE_RCCH_COPPER_RUN_LOOP_RESET:
			//printf ("Reset copper PC when writing TBBLUE_RCCH_COPPER_RUN_LOOP_RESET to control hi byte\n");
			tbblue_copper_reset_pc();
		break;

		case TBBLUE_RCCH_COPPER_RUN_VBI:
			//printf ("Reset copper PC when writing TBBLUE_RCCH_COPPER_RUN_VBI to control hi byte\n");
			tbblue_copper_reset_pc();
		break;

	}

}

//Fin copper


/*
   //es zona de vsync y borde superior
                                                                //Aqui el contador raster tiene valor (192+56 en adelante)
                                                                //contador de scanlines del core, entre 0 y screen_indice_inicio_pant ,
                                                                if (t_scanline<screen_indice_inicio_pant) {
                                                                        if (t_scanline==linea_raster-192-screen_total_borde_inferior) disparada_raster=1;
                                                                }

                                                                //Esto es zona de paper o borde inferior
                                                                //Aqui el contador raster tiene valor 0 .. <(192+56)
                                                                //contador de scanlines del core, entre screen_indice_inicio_pant y screen_testados_total
                                                                else {
                                                                        if (t_scanline-screen_indice_inicio_pant==linea_raster) disparada_raster=1;
                                                                }

https://github.com/z88dk/z88dk/blob/master/libsrc/_DEVELOPMENT/target/zxn/config/config_zxn_copper.m4#L74

# 50Hz                          60Hz
# Lines                         Lines
#
#   0-191  Display                0-191  Display
# 192-247  Bottom Border        192-223  Bottom Border
# 248-255  Vsync (interrupt)    224-231  Vsync (interrupt)
# 256-311 Top Border 232-261 Top Border


# Horizontally the display is the same in 50Hz or 60Hz mode but it
# varies by model.  It consists of 448 pixels (0-447) in 48k mode
# and 456 pixels (0-455) in 128k mode.  Grouped in eight pixels
# that's screen bytes 0-55 in 48k mode and 0-56 in 128k mode.
#
# 48k mode                      128k mode
# Bytes  Pixels                 Bytes  Pixels
#
#  0-31    0-255  Display        0-31    0-255  Display
# 32-39  256-319  Right Border  32-39  256-319  Right Border
# 40-51  320-415  HBlank        40-51  320-415  HBlank
# 52-55  416-447  Left Border   52-56  416-455  Left Border
#
# The ZXN Copper understands two operations:
#
# (1) Wait for a particular line (0-311 @ 50Hz or 0-261 @ 60Hz)
#     and a horizontal character position (0-55 or 0-56)
#
# (2) Write a value to a nextreg.

int screen_invisible_borde_superior;
//normalmente a 56.
int screen_borde_superior;

//estos dos anteriores se suman aqui. es 64 en 48k, y 63 en 128k. por tanto, uno de los dos valores vale 1 menos
int screen_indice_inicio_pant;

//suma del anterior+192
int screen_indice_fin_pant;

//normalmente a 56
int screen_total_borde_inferior;

zona borde invisible: 0 .. screen_invisible_borde_superior;
zona borde visible: screen_invisible_borde_superior .. screen_invisible_borde_superior+screen_borde_superior
zona visible pantalla: screen_indice_inicio_pant .. screen_indice_inicio_pant+192
zona inferior: screen_indice_fin_pant .. screen_indice_fin_pant+screen_total_borde_inferior 



*/

static int tbblue_get_current_raster_position(void)
{
	int raster;

	if (t_scanline<screen_invisible_borde_superior) {
		//En zona borde superior invisible (vsync)
		//Ajustamos primero a desplazamiento entre 0 y esa zona
		raster=t_scanline;

		//Sumamos offset de la zona raster
		
		raster +=192+screen_total_borde_inferior;
		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
		return raster;
	}

	if (t_scanline<screen_indice_inicio_pant) {
		//En zona borde superior visible
		//Ajustamos primero a desplazamiento entre 0 y esa zona
		raster=t_scanline-screen_invisible_borde_superior;

		//Sumamos offset de la zona raster
		raster +=192+screen_total_borde_inferior+screen_invisible_borde_superior;

		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
		return raster;
	}

	if (t_scanline<screen_indice_fin_pant) {
		//En zona visible pantalla
		//Ajustamos primero a desplazamiento entre 0 y esa zona
		raster=t_scanline-screen_indice_inicio_pant;

		//Sumamos offset de la zona raster
                raster +=0  ; //solo para que quede mas claro

		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
                return raster;
        }

	//Caso final. Zona borde inferior
		//Ajustamos primero a desplazamiento entre 0 y esa zona
                raster=t_scanline-screen_indice_fin_pant;

		//Sumamos offset de la zona raster
                raster +=192;
		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
                return raster;

}


int tbblue_get_current_raster_horiz_position(void)
{
/*
# Horizontally the display is the same in 50Hz or 60Hz mode but it
# varies by model.  It consists of 448 pixels (0-447) in 48k mode
# and 456 pixels (0-455) in 128k mode.  Grouped in eight pixels
# that's screen bytes 0-55 in 48k mode and 0-56 in 128k mode.
#
# 48k mode                      128k mode
# Bytes  Pixels                 Bytes  Pixels
#
#  0-31    0-255  Display        0-31    0-255  Display
# 32-39  256-319  Right Border  32-39  256-319  Right Border
# 40-51  320-415  HBlank        40-51  320-415  HBlank
# 52-55  416-447  Left Border   52-56  416-455  Left Border
*/
	int estados_en_linea=t_estados % screen_testados_linea;
	int horizontal_actual=estados_en_linea;

	//Dividir por la velocidad turbo
	horizontal_actual /=cpu_turbo_speed;

	//Con esto tendremos rango entre 0 y 223. Multiplicar por dos para ajustar a rango 0-448
	horizontal_actual *=2;

	//Dividir entre 8 para ajustar  rango 0-56
	horizontal_actual /=8;

	return horizontal_actual;

}


//Sprites

//Paleta de 256 colores formato RGB9 RRRGGGBBB
//Valores son de 9 bits por tanto lo definimos con z80_int que es de 16 bits
//z80_int tbsprite_palette[256];


//Diferentes paletas
//Total:
//     000 = ULA first palette
//     100 = ULA secondary palette
//     001 = Layer 2 first palette
//    101 = Layer 2 secondary palette
//     010 = Sprites first palette 
//     110 = Sprites secondary palette
//Paletas de 256 colores formato RGB9 RRRGGGBBB
//Valores son de 9 bits por tanto lo definimos con z80_int que es de 16 bits
z80_int tbblue_palette_ula_first[256];
z80_int tbblue_palette_ula_second[256];
z80_int tbblue_palette_layer2_first[256];
z80_int tbblue_palette_layer2_second[256];
z80_int tbblue_palette_sprite_first[256];
z80_int tbblue_palette_sprite_second[256];


//Diferentes layers a componer la imagen final
/*
(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  bits 6-5 = Reserved, must be 0
  bits 4-2 = set layers priorities:
     Reset default is 000, sprites over the Layer 2, over the ULA graphics
     000 - S L U
     001 - L S U
     010 - S U L
     011 - L U S
     100 - U S L
     101 - U L S
 */

//Si en zona pantalla y todo es transparente, se pone un 0
//Layers con el indice al olor final en la paleta RGB9 (0..511)


//Por que ancho 512? Es mas de lo necesario. 
//256 pixeles de ancho + 48 sumando borde izquierdo y derecho dan 304
//es mas, probando a donde llega el indice "posicion_array_layer" es efectivamente 304
//Por lo que con 304 seria mas que suficiente
//Seguramente este 512 ha sido una confusión debido a que en el comentario anterior se habla de colores RGB9 desde 0 hasta 511
z80_int tbblue_layer_ula[512];
z80_int tbblue_layer_layer2[512];
z80_int tbblue_layer_sprites[512];

/* 
Clip window registers

(W) 0x18 (24) => Clip Window Layer 2
  bits 7-0 = Cood. of the clip window
  1st write - X1 position
  2nd write - X2 position
  3rd write - Y1 position
  4rd write - Y2 position
  The values are 0,255,0,191 after a Reset

(W) 0x19 (25) => Clip Window Sprites
  bits 7-0 = Cood. of the clip window
  1st write - X1 position
  2nd write - X2 position
  3rd write - Y1 position
  4rd write - Y2 position
  The values are 0,255,0,191 after a Reset
  Clip window on Sprites only work when the "over border bit" is disabled

(W) 0x1A (26) => Clip Window ULA/LoRes
  bits 7-0 = Coord. of the clip window
  1st write = X1 position
  2nd write = X2 position
  3rd write = Y1 position
  4rd write = Y2 position

(W) 0x1C (28) => Clip Window control
  bits 7-3 = Reserved, must be 0
  bit 2 - reset the ULA/LoRes clip index.
  bit 1 - reset the sprite clip index.
  bit 0 - reset the Layer 2 clip index.

*/

z80_byte clip_window_layer2[4];
z80_byte clip_window_layer2_index;

z80_byte clip_window_sprites[4];
z80_byte clip_window_sprites_index;

z80_byte clip_window_ula[4];
z80_byte clip_window_ula_index;


//Damos la paleta que se esta leyendo o escribiendo en una operacion de I/O
//Para ello mirar bits 6-4  de reg 0x43
z80_int *tbblue_get_palette_rw(void)
{
/*
(R/W) 0x43 (67) => Palette Control
  bit 7 = Reserved, must be 0
  bits 6-4 = Select palette for reading or writing:
     000 = ULA first palette
     100 = ULA secondary palette
     001 = Layer 2 first palette
     101 = Layer 2 secondary palette
     010 = Sprites first palette 
     110 = Sprites secondary palette
  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	z80_byte active_palette=(tbblue_registers[0x43]>>4)&7;

	switch (active_palette) {
		case 0:
			return tbblue_palette_ula_first;
		break;

		case 4:
			return tbblue_palette_ula_second;
		break;

		case 1:
			return tbblue_palette_layer2_first;
		break;

		case 5:
			return tbblue_palette_layer2_second;
		break;

		case 2:
			return tbblue_palette_sprite_first;
		break;

		case 6:
			return tbblue_palette_sprite_second;
		break;

		//por defecto retornar siempre ULA first palette
		default:
			return tbblue_palette_ula_first;
		break;
	}
}

//Damos el valor del color de la paleta que se esta leyendo o escribiendo en una operacion de I/O
z80_int tbblue_get_value_palette_rw(z80_byte index)
{
	z80_int *paleta;

	paleta=tbblue_get_palette_rw();

	return paleta[index];
}


//Modificamos el valor del color de la paleta que se esta leyendo o escribiendo en una operacion de I/O
void tbblue_set_value_palette_rw(z80_byte index,z80_int valor)
{
	z80_int *paleta;

	paleta=tbblue_get_palette_rw();

	paleta[index]=valor;
}

//Damos el valor del color de la paleta que se esta mostrando en pantalla para sprites
//Para ello mirar bit 3 de reg 0x43
z80_int tbblue_get_palette_active_sprite(z80_byte index)
{
/*
(R/W) 0x43 (67) => Palette Control

  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	if (tbblue_registers[0x43]&8) return tbblue_palette_sprite_second[index];
	else return tbblue_palette_sprite_first[index];

}

//Damos el valor del color de la paleta que se esta mostrando en pantalla para layer2
//Para ello mirar bit 2 de reg 0x43
z80_int tbblue_get_palette_active_layer2(z80_byte index)
{
/*
(R/W) 0x43 (67) => Palette Control

  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	if (tbblue_registers[0x43]&4) return tbblue_palette_layer2_second[index];
	else return tbblue_palette_layer2_first[index];

}

//Damos el valor del color de la paleta que se esta mostrando en pantalla para ula
//Para ello mirar bit 1 de reg 0x43
z80_int tbblue_get_palette_active_ula(z80_byte index)
{
/*
(R/W) 0x43 (67) => Palette Control

  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	if (tbblue_registers[0x43]&2) return tbblue_palette_ula_second[index];
	else return tbblue_palette_ula_first[index];

}

//64 patterns de Sprites
/*
In the palette each byte represents the colors in the RRRGGGBB format, and the pink color, defined by standard 1110011, is reserved for the transparent color.
*/
//z80_byte tbsprite_patterns[TBBLUE_MAX_PATTERNS][TBBLUE_SPRITE_SIZE];
z80_byte tbsprite_new_patterns[TBBLUE_MAX_PATTERNS*TBBLUE_SPRITE_SIZE];

int tbsprite_pattern_get_offset_index(z80_byte sprite,z80_byte index_in_sprite)
{
	return sprite*TBBLUE_SPRITE_SIZE+index_in_sprite;
}

z80_byte tbsprite_pattern_get_value_index(z80_byte sprite,z80_byte index_in_sprite)
{
	return tbsprite_new_patterns[tbsprite_pattern_get_offset_index(sprite,index_in_sprite)];
}

void tbsprite_pattern_put_value_index(z80_byte sprite,z80_byte index_in_sprite,z80_byte value)
{
	tbsprite_new_patterns[tbsprite_pattern_get_offset_index(sprite,index_in_sprite)]=value;
}

//64 sprites
/*
[0] 1st: X position (bits 7-0).
[1] 2nd: Y position (0-255).
[2] 3rd: bits 7-4 is palette offset, bit 3 is X MSB, bit 2 is X mirror, bit 1 is Y mirror and bit 0 is visible flag.
[3] 4th: bits 7-6 is reserved, bits 5-0 is Name (pattern index, 0-63).
*/
z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];

//Indices al indicar paleta, pattern, sprites. Subindex indica dentro de cada pattern o sprite a que posicion (0..3 en sprites o 0..255 en pattern ) apunta
z80_byte tbsprite_index_palette;
z80_byte tbsprite_index_pattern,tbsprite_index_pattern_subindex;
z80_byte tbsprite_index_sprite,tbsprite_index_sprite_subindex;

/*
Port 0x303B, if read, returns some information:

Bits 7-2: Reserved, must be 0.
Bit 1: max sprites per line flag.
Bit 0: Collision flag.
Port 0x303B, if written, defines the sprite slot to be configured by ports 0x55 and 0x57, and also initializes the address of the palette.

*/

z80_byte tbblue_port_303b;


/* Informacion relacionada con Layer2. Puede cambiar en el futuro, hay que ir revisando info en web de Next

Registros internos implicados:

(R/W) 0x12 (18) => Layer 2 RAM page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 8 after a Reset)

(R/W) 0x13 (19) => Layer 2 RAM shadow page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 11 after a Reset)

(R/W) 0x14 (20) => Global transparency color
  bits 7-0 = Transparency color value (Reset to 0xE3, after a reset)
  (Note this value is 8-bit only, so the transparency is compared only by the MSB bits of the final colour)



(R/W) 0x16 (22) => Layer2 Offset X
  bits 7-0 = X Offset (0-255)(Reset to 0 after a reset)

(R/W) 0x17 (23) => Layer2 Offset Y
  bits 7-0 = Y Offset (0-191)(Reset to 0 after a reset)




Posiblemente registro 20 aplica a cuando el layer2 esta por detras de pantalla de spectrum, y dice el color de pantalla de spectrum
que actua como transparente
Cuando layer2 esta encima de pantalla spectrum, el color transparente parece que es el mismo que sprites: TBBLUE_TRANSPARENT_COLOR 0xE3

Formato layer2: 256x192, linear, 8bpp, RRRGGGBB (mismos colores que sprites), ocupa 48kb

Se accede en modo escritura en 0000-3fffh mediante puerto:

Banking in Layer2 is out 4667 ($123B)
bit 0 = write enable, which changes writes from 0-3fff to write to layer2,
bit 1 = Layer2 ON or OFF set=ON,
bit 2 = ????
bit 3 = Use register 19 instead of 18 to tell sram page
bit 4 puts layer 2 behind the normal spectrum screen
bit 6 and 7 are to say which 16K section is paged in,
$03 = 00000011b Layer2 on and writable and top third paged in at $0000,
$43 = 01000011b Layer2 on and writable and middle third paged in at $0000,
$C3 = 11000011b Layer2 on and writable and bottom third paged in at $0000,  ?? sera 100000011b??? TODO
$02 = 00000010b Layer2 on and nothing paged in. etc

Parece que se mapea la pagina de sram indicada en registro 19

*/


/*

IMPORTANT!!

Trying some old layer2 demos that doesn't set register 19 is dangerous.
To avoid problems, first do:
out 9275, 19
out 9531,32
To set layer2 to the extra ram:
0x080000 – 0x0FFFFF (512K) => Extra RAM

Then load the demo program and will work

*/

z80_byte tbblue_port_123b;


//valor inicial para tbblue_port_123b en caso de fast boot mode
int tbblue_initial_123b_port=-1;

int tbblue_write_on_layer2(void)
{
	if (tbblue_port_123b &1) return 1;
	return 0;
}

int tbblue_is_active_layer2(void)
{
	if (tbblue_port_123b & 2) return 1;
	return 0;
}


int tbblue_get_offset_start_layer2(void)
{
	int offset=tbblue_registers[18]&63;

	if (tbblue_port_123b & 8 ) offset=tbblue_registers[19]&63;

	//offset=tbblue_registers[18]&63;
	offset*=16384;

	//Y empezar en 0x040000 – 0x05FFFF (128K) => ZX Spectrum RAM
	/*
	recordemos
	    0x040000 – 0x05FFFF (128K) => ZX Spectrum RAM			(16 paginas) 
    0x060000 – 0x07FFFF (128K) => Extra RAM				(16 paginas)

    0x080000 – 0x0FFFFF (512K) => 1st Extra IC RAM (if present)		(64 paginas)
    0x100000 – 0x17FFFF (512K) => 2nd Extra IC RAM (if present)		(64 paginas)
    0x180000 – 0xFFFFFF (512K) => 3rd Extra IC RAM (if present)		(64 paginas)

    0x200000 (2 MB)

    	Con and 63, maximo layer 2 son 64 paginas
    	64*16384=1048576  -> 1 mb total
    	empezando en 0x040000 + 1048576 = 0x140000 y no nos salimos de rango (estariamos en el 2nd Extra IC RAM)
    	Dado que siempre asigno 2 mb para tbblue, no hay problema

    	*/



	offset +=0x040000;

	return offset;
}

void tbblue_reset_sprites(void)
{

	int i;

	

	//Resetear patterns todos a transparente
	for (i=0;i<TBBLUE_MAX_PATTERNS;i++) {
		int j;
		for (j=0;j<256;j++) {
			//tbsprite_patterns[i][j]=TBBLUE_DEFAULT_TRANSPARENT;
			tbsprite_pattern_put_value_index(i,j,TBBLUE_DEFAULT_TRANSPARENT);
		}
	}

	//Poner toda info de sprites a 0. Seria quiza suficiente con poner bit de visible a 0
	for (i=0;i<TBBLUE_MAX_SPRITES;i++) {
		tbsprite_sprites[i][0]=0;
		tbsprite_sprites[i][1]=0;
		tbsprite_sprites[i][2]=0;
		tbsprite_sprites[i][3]=0;
	}


	tbsprite_index_palette=tbsprite_index_pattern=tbsprite_index_sprite=0;

	tbblue_port_303b=0;

	tbblue_registers[22]=0;
	tbblue_registers[23]=0;


}

z80_int tbblue_get_9bit_colour(z80_byte valor)
{
	//Retorna color de 9 bits en base a 8 bits
	z80_int valor16=valor;

	//Bit bajo sale de hacer or de bit 1 y 0
	//z80_byte bit_bajo=valor&1;
	z80_byte bit_bajo=(valor&1)|((valor&2)>>1);

	//rotamos a la izquierda para que sean los 8 bits altos
	valor16=valor16<<1;

	valor16 |=bit_bajo;

	return valor16;	
}

void tbblue_reset_palettes(void)
{
	//Inicializar Paletas
	int i;

//z80_int valor16=tbblue_get_9bit_colour(valor);

	//Paletas layer2 & sprites son los mismos valores del indice*2 y metiendo bit 0 como el bit1 inicial
	//(cosa absurda pues no sigue la logica de mezclar bit 0 y bit 1 usado en registro 41H)
	for (i=0;i<256;i++) {
		z80_int color;
		color=i*2;
		if (i&2) color |=1;

 		tbblue_palette_layer2_first[i]=color;
 		tbblue_palette_layer2_second[i]=color;
 		tbblue_palette_sprite_first[i]=color;
 		tbblue_palette_sprite_second[i]=color;
	}


	//Se repiten los 16 colores en los 256. Para colores sin brillo, componente de color vale 5 (101b)
	const z80_int tbblue_default_ula_colours[16]={
0 ,   //000000000 
5 ,   //000000101 
320 , //101000000 
325 , //101000101 
40 ,  //000101000 
45 ,  //000101101 
360 , //101101000 
365 , //101101101 
0 ,   //000000000 
7 ,   //000000111 
448 , //111000000 
455 , //111000111 
56 ,  //000111000 
63 ,  //000111111 
504 , //111111000 
511   //111111111 
	};
	/*
	Nota: para convertir una lista de valores binarios en decimal:
	LINEA=""

while read LINEA; do
echo -n "$((2#$LINEA))"
echo " , //$LINEA "


done < /tmp/archivo_lista.txt
	*/

	int j;

	for (j=0;j<16;j++) {
		for (i=0;i<16;i++) {
			tbblue_palette_ula_first[j*16+i]=tbblue_default_ula_colours[i];
			tbblue_palette_ula_second[j*16+i]=tbblue_default_ula_colours[i];
		}
	}

	/*

 	
	tbblue_palette_ula_first[8]=tbblue_palette_ula_first[8+128]=0;
	tbblue_palette_ula_first[9]=tbblue_palette_ula_first[9+128]=7;
	tbblue_palette_ula_first[10]=tbblue_palette_ula_first[10+128]=448;
	tbblue_palette_ula_first[11]=tbblue_palette_ula_first[11+128]=455; 

	 //tanto 455 como 454 (1C7H y 1C6H) son colores transparentes por defecto (1C7H y 1C6H  / 2 = E3H)
	tbblue_palette_ula_first[12]=tbblue_palette_ula_first[12+128]=56;
	tbblue_palette_ula_first[13]=tbblue_palette_ula_first[13+128]=63;
	tbblue_palette_ula_first[14]=tbblue_palette_ula_first[14+128]=504;
	tbblue_palette_ula_first[15]=tbblue_palette_ula_first[15+128]=511;*/


}


void tbblue_out_port_sprite_index(z80_byte value)
{
	//printf ("Out tbblue_out_port_sprite_index %02XH\n",value);
	tbsprite_index_palette=tbsprite_index_pattern=tbsprite_index_sprite=value;

	tbsprite_index_pattern_subindex=tbsprite_index_sprite_subindex=0;
}

/*void tbblue_out_sprite_palette(z80_byte value)
{
	//printf ("Out tbblue_out_sprite_palette %02XH\n",value);

	tbsprite_palette[tbsprite_index_palette]=value;
	if (tbsprite_index_palette==255) tbsprite_index_palette=0;
	else tbsprite_index_palette++;
}*/


//Indica si al escribir registro 44h de paleta:
//si 0, se escribe 8 bits superiores
//si no 0, se escribe 1 bit inferior
int tbblue_write_palette_state=0;

void tbblue_reset_palette_write_state(void)
{
	tbblue_write_palette_state=0;
}

void tbblue_increment_palette_index(void)
{

	z80_byte indice=tbblue_registers[0x40];
	indice++;
	tbblue_registers[0x40]=indice;

	tbblue_reset_palette_write_state();
}



//Escribe valor de 8 bits superiores (de total de 9) para indice de color de paleta 
void tbblue_write_palette_value_high8(z80_byte valor)
{
/*
(R/W) 0x40 (64) => Palette Index
  bits 7-0 = Select the palette index to change the default colour. 
  0 to 127 indexes are to ink colours and 128 to 255 indexes are to papers.
  (Except full ink colour mode, that all values 0 to 255 are inks)
  Border colours are the same as paper 0 to 7, positions 128 to 135,
  even at full ink mode. 
  (inks and papers concept only applies to Enhanced ULA palette. 
  Layer 2 and Sprite palettes works as "full ink" mode)

  (R/W) 0x41 (65) => Palette Value (8 bit colour)
  bits 7-0 = Colour for the palette index selected by the register 0x40. Format is RRRGGGBB
  Note the lower blue bit colour will be an OR between bit 1 and bit 0. 
  After the write, the palette index is auto-incremented to the next index. 
  The changed palette remains until a Hard Reset.


(R/W) 0x44 (68) => Palette Value (9 bit colour)
  Two consecutive writes are needed to write the 9 bit colour
  1st write:
     bits 7-0 = RRRGGGBB
  2nd write. 
     bit 7-1 = Reserved, must be 0
     bit 0 = lsb B
  After the two consecutives writes the palette index is auto-incremented.
  The changed palette remais until a Hard Reset.

*/
	z80_byte indice=tbblue_registers[0x40];

	//Obtenemos valor actual y alteramos los 8 bits altos del total de 9
	//z80_int color_actual=tbblue_get_value_palette_rw(indice);

	//Conservamos bit bajo
	//color_actual &=1;
	//Bit bajo es el mismo que bit 1

	/*z80_int valor16=valor;

	//Bit bajo sale de hacer or de bit 1 y 0
	//z80_byte bit_bajo=valor&1;
	z80_byte bit_bajo=(valor&1)|((valor&2)>>1);

	//rotamos a la izquierda para que sean los 8 bits altos
	valor16=valor16<<1;

	valor16 |=bit_bajo;*/

	z80_int valor16=tbblue_get_9bit_colour(valor);

	//y or del valor de 1 bit de B
	//valor16 |=color_actual;

	tbblue_set_value_palette_rw(indice,valor16);




}


//Escribe valor de 1 bit inferior (de total de 9) para indice de color de paleta y incrementa indice
void tbblue_write_palette_value_low1(z80_byte valor)
{
/*
(R/W) 0x40 (64) => Palette Index
  bits 7-0 = Select the palette index to change the default colour. 
  0 to 127 indexes are to ink colours and 128 to 255 indexes are to papers.
  (Except full ink colour mode, that all values 0 to 255 are inks)
  Border colours are the same as paper 0 to 7, positions 128 to 135,
  even at full ink mode. 
  (inks and papers concept only applies to Enhanced ULA palette. 
  Layer 2 and Sprite palettes works as "full ink" mode)

  (R/W) 0x41 (65) => Palette Value (8 bit colour)
  bits 7-0 = Colour for the palette index selected by the register 0x40. Format is RRRGGGBB
  Note the lower blue bit colour will be an OR between bit 1 and bit 0. 
  After the write, the palette index is auto-incremented to the next index. 
  The changed palette remains until a Hard Reset.


(R/W) 0x44 (68) => Palette Value (9 bit colour)
  Two consecutive writes are needed to write the 9 bit colour
  1st write:
     bits 7-0 = RRRGGGBB
  2nd write. 
     bit 7-1 = Reserved, must be 0
     bit 0 = lsb B
  After the two consecutives writes the palette index is auto-incremented.
  The changed palette remais until a Hard Reset.

*/
	z80_byte indice=tbblue_registers[0x40];

	//Bit inferior siempre cambia indice anterior
	//indice--;

	//Obtenemos valor actual y conservamos los 8 bits altos del total de 9
	z80_int color_actual=tbblue_get_value_palette_rw(indice);

	//Conservamos 8 bits altos
	color_actual &=0x1FE;

	//Y valor indicado, solo conservar 1 bit
	valor &= 1;

	color_actual |=valor;

	tbblue_set_value_palette_rw(indice,color_actual);

	tbblue_increment_palette_index();


}


//Escribe valor de paleta de registro 44H, puede que se escriba en 8 bit superiores o en 1 inferior
void tbblue_write_palette_value_high8_low1(z80_byte valor)
{
	if (tbblue_write_palette_state==0) {
		tbblue_write_palette_value_high8(valor);
		tbblue_write_palette_state++;
	}

	else tbblue_write_palette_value_low1(valor);
	
}


void tbblue_out_sprite_pattern(z80_byte value)
{




	tbsprite_pattern_put_value_index(tbsprite_index_pattern,tbsprite_index_pattern_subindex,value);
	//tbsprite_patterns[tbsprite_index_pattern][tbsprite_index_pattern_subindex]=value;



	if (tbsprite_index_pattern_subindex==255) {
		tbsprite_index_pattern_subindex=0;
		tbsprite_index_pattern++;
		if (tbsprite_index_pattern>=TBBLUE_MAX_PATTERNS) tbsprite_index_pattern=0;
	}
	else tbsprite_index_pattern_subindex++;

}

void tbblue_out_sprite_sprite(z80_byte value)
{
	//printf ("Out tbblue_out_sprite_sprite. Index: %d subindex: %d %02XH\n",tbsprite_index_sprite,tbsprite_index_sprite_subindex,value);



	//Indices al indicar paleta, pattern, sprites. Subindex indica dentro de cada pattern o sprite a que posicion (0..3 en sprites o 0..255 en pattern ) apunta
	//z80_byte tbsprite_index_sprite,tbsprite_index_sprite_subindex;

	tbsprite_sprites[tbsprite_index_sprite][tbsprite_index_sprite_subindex]=value;
	if (tbsprite_index_sprite_subindex==3) {
		//printf ("sprite %d [3] pattern: %d\n",tbsprite_index_sprite,tbsprite_sprites[tbsprite_index_sprite][3]&63);
		tbsprite_index_sprite_subindex=0;
		tbsprite_index_sprite++;
		if (tbsprite_index_sprite>=TBBLUE_MAX_SPRITES) tbsprite_index_sprite=0;
	}

	else tbsprite_index_sprite_subindex++;
}


//Guarda scanline actual y el pattern (los indices a colores) sobre la paleta activa de sprites
//z80_byte sprite_line[MAX_X_SPRITE_LINE];


//Dice si un color de la paleta rbg9 es transparente
int tbblue_si_transparent(z80_int color)
{
	//if ( (color&0x1FE)==TBBLUE_TRANSPARENT_COLOR) return 1;
	color=(color>>1)&0xFF;
	if (color==TBBLUE_TRANSPARENT_REGISTER) return 1;
	return 0;
}


/*
Port 0x243B is write-only and is used to set the registry number.

Port 0x253B is used to access the registry value.

Register:
(R/W) 21 => Sprite system
 bits 7-2 = Reserved, must be 0
 bit 1 = Over border (1 = yes)
 bit 0 = Sprites visible (1 = visible)
*/
void tbsprite_put_color_line(int x,z80_byte color,int rangoxmin,int rangoxmax)
{

	//Si coordenadas invalidas, volver
	//if (x<0 || x>=MAX_X_SPRITE_LINE) return;

	//Si coordenadas fuera de la parte visible (border si o no), volver
	if (x<rangoxmin || x>rangoxmax) return;


	//Si fuera del viewport. Clip window on Sprites only work when the "over border bit" is disabled
	int clipxmin=clip_window_sprites[0]+TBBLUE_SPRITE_BORDER;
	int clipxmax=clip_window_sprites[1]+TBBLUE_SPRITE_BORDER;
	z80_byte sprites_over_border=tbblue_registers[21]&2;
	if (sprites_over_border==0 && (x<clipxmin || x>=clipxmax)) return;

	z80_int color_final=tbblue_get_palette_active_sprite(color);

	//Si color transparente, no hacer nada
	if (tbblue_si_transparent(color_final)) return;

	int xfinal=x;

	xfinal +=screen_total_borde_izquierdo*border_enabled.v;
	xfinal -=TBBLUE_SPRITE_BORDER;


	//Ver si habia un color y activar bit colision
	z80_int color_antes=tbblue_layer_sprites[xfinal];

	if (!tbblue_si_transparent(color_antes)) {
		//colision
		tbblue_port_303b |=1;
		//printf ("set colision flag. result value: %d\n",tbblue_port_303b);
	}
	

	//sprite_line[x]=color;
	tbblue_layer_sprites[xfinal]=color_final;

}

z80_byte tbsprite_do_overlay_get_pattern_xy(z80_byte index_pattern,z80_byte sx,z80_byte sy)
{

	//return tbsprite_patterns[index_pattern][sy*TBBLUE_SPRITE_WIDTH+sx];
	return tbsprite_pattern_get_value_index(index_pattern,sy*TBBLUE_SPRITE_WIDTH+sx);
}

z80_int tbsprite_return_color_index(z80_byte index)
{
	//z80_int color_final=tbsprite_palette[index];

	z80_int color_final=tbblue_get_palette_active_sprite(index);
	//return RGB9_INDEX_FIRST_COLOR+color_final;
	return color_final;
}

void tbsprite_do_overlay(void)
{

        //spritechip activo o no?
        if ( (tbblue_registers[21]&1)==0) return;

				//printf ("tbblue sprite chip activo\n");


        //int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;
        int y=t_scanline_draw; //0..63 es border (8 no visibles)

				int border_no_visible=screen_indice_inicio_pant-TBBLUE_SPRITE_BORDER;

				y -=border_no_visible;

				//Ejemplo: scanline_draw=32 (justo donde se ve sprites). border_no_visible=64-32 =32
				//y=y-32 -> y=0


				//Situamos el 0 32 pixeles por encima de dentro de pantalla, tal cual como funcionan las cordenadas de sprite de tbblue


        //if (border_enabled.v==0) y=y-screen_borde_superior;
        //z80_int *puntero_buf_rainbow;
        //puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];


				//Calculos exclusivos para puntero buffer rainbow
		    int rainbowy=t_scanline_draw-screen_invisible_borde_superior;
		    if (border_enabled.v==0) rainbowy=rainbowy-screen_borde_superior;
		    //z80_int *puntero_buf_rainbow;
		    //puntero_buf_rainbow=&rainbow_buffer[ rainbowy*get_total_ancho_rainbow() ];

		    //int puntero_array_layer=0;




        //puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;

				//printf ("overlay t_scanline_draw: %d y: %d\n",t_scanline_draw,y);

				//Aqui tenemos el y=0 arriba del todo del border

        //Bucle para cada sprite
        int conta_sprites;
				z80_byte index_pattern;

		int i;
		//int offset_pattern;

		z80_byte sprites_over_border=tbblue_registers[21]&2;


				//Inicializar linea a transparente
				//for (i=0;i<MAX_X_SPRITE_LINE;i++) {
				//	sprite_line[i]=TBBLUE_TRANSPARENT_COLOR;
				//}

				int rangoxmin, rangoxmax;

				if (sprites_over_border) {
					rangoxmin=0;
					rangoxmax=TBBLUE_SPRITE_BORDER+256+TBBLUE_SPRITE_BORDER-1;
				}

				else {
					rangoxmin=TBBLUE_SPRITE_BORDER;
					rangoxmax=TBBLUE_SPRITE_BORDER+255;
				}


				int total_sprites=0;


        for (conta_sprites=0;conta_sprites<TBBLUE_MAX_SPRITES && total_sprites<MAX_SPRITES_PER_LINE;conta_sprites++) {
					int sprite_x;
					int sprite_y;



					/*

					OLD
					[0] 1st: X position (bits 7-0).
					[1] 2nd: Y position (0-255).
					[2] 3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is visible flag and bit 0 is X MSB.
					[3] 4th: bits 7-6 is reserved, bits 5-0 is Name (pattern index, 0-63).

					NEW
					[0] 1st: X position (bits 7-0).
					[1] 2nd: Y position (0-255).
					[2] 3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is rotate flag and bit 0 is X MSB.
					[3] 4th: bit 7 is visible flag, bit 6 is reserved, bits 5-0 is Name (pattern index, 0-63).


					*/
					/*
					Because sprites can be displayed on top of the ZX Spectrum border, the coordinates of each sprite can range
					from 0 to 319 for the X axis and 0 to 255 for the Y axis. For both axes, values from 0 to 31 are reserved
					for the Left or top border, for the X axis the values 288 to 319 is reserved for the right border and for
					the Y axis values 224 to 255 for the lower border.

If the display of the sprites on the border is disabled, the coordinates of the sprites range from (32,32) to (287,223).
*/

					//Si sprite visible
					if (tbsprite_sprites[conta_sprites][3]&128) {
						sprite_x=tbsprite_sprites[conta_sprites][0] | ((tbsprite_sprites[conta_sprites][2]&1)<<8);

						//printf ("sprite %d x: %d \n",conta_sprites,sprite_x);

						sprite_y=tbsprite_sprites[conta_sprites][1];

						//Posicionamos esa y teniendo en cuenta que nosotros contamos 0 arriba del todo del border en cambio sprites aqui
						//Considera y=32 dentro de pantalla y y=0..31 en el border
						//sprite_y +=screen_borde_superior-32;

						//Si y==32-> y=32+48-32=32+16=48
						//Si y==0 -> y=48-32=16

						z80_byte mirror_x=tbsprite_sprites[conta_sprites][2]&8;
						//[2] 3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is rotate flag and bit 0 is X MSB.
						z80_byte mirror_y=tbsprite_sprites[conta_sprites][2]&4;

						//3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is rotate flag and bit 0 is X MSB.
						//Offset paleta se lee tal cual sin rotar valor
						z80_byte palette_offset=(tbsprite_sprites[conta_sprites][2]) & 0xF0;

						index_pattern=tbsprite_sprites[conta_sprites][3]&63;
						//Si coordenada y esta en margen y sprite activo

						int diferencia=y-sprite_y;


						int rangoymin, rangoymax;

						if (sprites_over_border) {
							rangoymin=0;
							rangoymax=TBBLUE_SPRITE_BORDER+192+TBBLUE_SPRITE_BORDER-1;
						}

						else {
							rangoymin=TBBLUE_SPRITE_BORDER;
							rangoymax=TBBLUE_SPRITE_BORDER+191;
						}


						//Pintar el sprite si esta en rango de coordenada y
						if (diferencia>=0 && diferencia<TBBLUE_SPRITE_HEIGHT && y>=rangoymin && y<=rangoymax) {

							//printf ("y: %d t_scanline_draw: %d rainbowy:%d sprite_y: %d\n",y,t_scanline_draw,rainbowy,sprite_y);
							z80_byte sx=0,sy=0; //Coordenadas x,y dentro del pattern
							//offset_pattern=0;

							//Incrementos de x e y
							int incx=+1;
							int incy=0;

							//Aplicar mirror si conviene y situarnos en la ultima linea
							if (mirror_y) {
								//offset_pattern=offset_pattern+TBBLUE_SPRITE_WIDTH*(TBBLUE_SPRITE_HEIGHT-1);
								sy=TBBLUE_SPRITE_HEIGHT-1-diferencia;
								//offset_pattern -=TBBLUE_SPRITE_WIDTH*diferencia;
							}
							else {
								//offset_pattern +=TBBLUE_SPRITE_WIDTH*diferencia;
								sy=diferencia;
							}


							//index_pattern ya apunta a pattern a pintar en pantalla
							//z80_int *puntero_buf_rainbow_sprite;
							//puntero_buf_rainbow_sprite=puntero_buf_rainbow+sprite_x;

							//Dibujar linea x

							//Cambiar offset si mirror x, ubicarlo a la derecha del todo
							if (mirror_x) {
								//offset_pattern=offset_pattern+TBBLUE_SPRITE_WIDTH-1;
								sx=TBBLUE_SPRITE_WIDTH-1;
								incx=-1;
							}

							z80_byte sprite_rotate;

							sprite_rotate=tbsprite_sprites[conta_sprites][2]&2;

							/*
							Comparar bits rotacion con ejemplo en media/spectrum/tbblue/sprites/rotate_example.png
							*/
							/*
							Basicamente sin rotar un sprite, se tiene (reduzco el tamaño a la mitad aqui para que ocupe menos)


							El sentido normal de dibujado viene por ->, aumentando coordenada X


					->  ---X----
							---XX---
							---XXX--
							---XXXX-
							---X----
							---X----
							---X----
							---X----

							Luego cuando se rota 90 grados, en vez de empezar de arriba a la izquierda, se empieza desde abajo y reduciendo coordenada Y:

							    ---X----
									---XX---
									---XXX--
									---XXXX-
									---X----
									---X----
							^ 	---X----
							|		---X----

							Entonces, al dibujar empezando asi, la imagen queda rotada:

							--------
							--------
							XXXXXXXX
							----XXX-
							----XX--
							----X---
							--------

							De ahi que el incremento y sea -incremento x , incremento x sera 0

							Aplicando tambien el comportamiento para mirror, se tiene el resto de combinaciones

							*/


							if (sprite_rotate) {
								z80_byte sy_old=sy;
								sy=(TBBLUE_SPRITE_HEIGHT-1)-sx;
								sx=sy_old;

								incy=-incx;
								incx=0;
							}


							for (i=0;i<TBBLUE_SPRITE_WIDTH;i++) {
								z80_byte index_color=tbsprite_do_overlay_get_pattern_xy(index_pattern,sx,sy);

								//Sumar palette offset. Logicamente si es >256 el resultado, dará la vuelta el contador
								index_color +=palette_offset;

								//printf ("index color: %d\n",index_color);
								

								sx=sx+incx;
								sy=sy+incy;

								/*
								if (mirror_x) {
									//offset_pattern--;
									sx--;
								}
								else {
									//offset_pattern++;
									sx++;
								}*/
								//z80_byte color=tbsprite_palette[index_color];
								//tbsprite_put_color_line(sprite_x++,color,rangoxmin,rangoxmax);
								tbsprite_put_color_line(sprite_x++,index_color,rangoxmin,rangoxmax);


							}

							total_sprites++;
							//printf ("total sprites in this line: %d\n",total_sprites);
							if (total_sprites==MAX_SPRITES_PER_LINE) {
								//max sprites per line flag
								tbblue_port_303b |=2;
								//printf ("set max sprites per line flag\n");
							}

						}

				}
			}

			//Dibujar linea de sprites en pantalla ignorando color transparente

			//Tener en cuenta que de 0..31 en x es el border
			//Posicionar puntero rainbow en zona interior pantalla-32 pixels border

			//puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;

			//puntero_buf_rainbow -=TBBLUE_SPRITE_BORDER;



			//Inicializar linea a transparente



}

z80_byte tbblue_get_port_layer2_value(void)
{
	return tbblue_port_123b;
}

void tbblue_out_port_layer2_value(z80_byte value)
{
	tbblue_port_123b=value;
}


z80_byte tbblue_get_port_sprite_index(void)
{
	/*
	Port 0x303B, if read, returns some information:

Bits 7-2: Reserved, must be 0.
Bit 1: max sprites per line flag.
Bit 0: Collision flag.
*/
	z80_byte value=tbblue_port_303b;
	//Cuando se lee, se resetean bits 0 y 1
	//printf ("-----Reading port 303b. result value: %d\n",value);
	tbblue_port_303b &=(255-1-2);

	return value;

}



//'bootrom' takes '1' on hard-reset and takes '0' if there is any writing on the i/o port 'config1'. It can not be read.
z80_bit tbblue_bootrom={1};

//Puerto tbblue de maquina/mapeo
/*
- Write in port 0x24DB (config1)

                case cpu_do(7 downto 6) is
                    when "01"    => maquina <= s_speccy48;
                    when "10"    => maquina <= s_speccy128;
                    when "11"    => maquina <= s_speccy3e;
                    when others    => maquina <= s_config;   ---config mode
                end case;
                romram_page <= cpu_do(4 downto 0);                -- rom or ram page
*/

//z80_byte tbblue_config1=0;

/*
Para habilitar las interfaces y las opciones que utilizo el puerto 0x24DD (config2). Para iniciar la ejecución de la máquina elegida escribo la máquina de la puerta 0x24DB y luego escribir 0x01 en puerta 0x24D9 (hardsoftreset), que activa el "SoftReset" y hace PC = 0.



Si el bit 7 es "0":

6 => "Lightpen" activado;
5 => "Multiface" activado;
4 => "PS/2" teclado o el ratón;
3-2 => el modo de joystick1;
1-0 => el modo de joystick2;

Puerta 0x24D9:

bit 1 => realizar un "hard reset" ( "maquina"=0,  y "bootrom" recibe '1' )
bit 0 => realizar un "soft reset" ( PC = 0x0000 )
*/

//z80_byte tbblue_config2=0;
//z80_byte tbblue_hardsoftreset=0;


//Si segmento bajo (0-16383) es escribible
z80_bit tbblue_low_segment_writable={0};


//Indice a lectura del puerto
//z80_byte tbblue_read_port_24d5_index=0;

//port 0x24DF bit 2 = 1 turbo mode (7Mhz)
//z80_byte tbblue_port_24df;


//Asumimos 256 registros
z80_byte tbblue_registers[256];

//Ultimo registro seleccionado
z80_byte tbblue_last_register;


void tbblue_init_memory_tables(void)
{
/*

Primer bloque de ram: memoria interna de tbblue en principio no accesible por el spectrum:

Mapeo viejo

0x000000 – 0x01FFFF (128K) => DivMMC RAM
0x020000 – 0x03FFFF (128K) => Layer2 RAM
0x040000 – 0x05FFFF (128K) => ??????????
0x060000 – 0x06FFFF (64K) => ESXDOS and Multiface RAM
0x060000 – 0x063FFF (16K) => ESXDOS ROM
0x064000 – 0x067FFF (16K) => Multiface ROM
0x068000 – 0x06BFFF (16K) => Multiface extra ROM
0x06c000 – 0x06FFFF (16K) => Multiface RAM
0x070000 – 0x07FFFF (64K) => ZX Spectrum ROM

Segundo bloque de ram: 512K, todo accesible para Spectrum. Se mapean 256 o 512 mediante bit 6 y 7 de puerto 32765
0x080000 - 0x0FFFFF (512K) => Speccy RAM

Luego 8 KB de rom de la fpga
0x100000 - 0x101FFF

Nuevo:

0x000000 – 0x00FFFF (64K) => ZX Spectrum ROM
0x010000 – 0x013FFF (16K) => ESXDOS ROM
0x014000 – 0x017FFF (16K) => Multiface ROM
0x018000 – 0x01BFFF (16K) => Multiface extra ROM
0x01c000 – 0x01FFFF (16K) => Multiface RAM
0x020000 – 0x05FFFF (256K) => divMMC RAM
0x060000 – 0x07FFFF (128K) => ZX Spectrum RAM
0x080000 – 0x0FFFFF (512K) => Extra RAM


Nuevo oct 2017:

    0x000000 – 0x00FFFF (64K) => ZX Spectrum ROM
    0x010000 – 0x013FFF (16K) => ESXDOS ROM
    0x014000 – 0x017FFF (16K) => Multiface ROM
    0x018000 – 0x01BFFF (16K) => Multiface extra ROM
    0x01c000 – 0x01FFFF (16K) => Multiface RAM
    0x020000 – 0x03FFFF (128K) => divMMC RAM
    0x040000 – 0x05FFFF (128K) => ZX Spectrum RAM			(16 paginas) 
    0x060000 – 0x07FFFF (128K) => Extra RAM				(16 paginas)

    0x080000 – 0x0FFFFF (512K) => 1st Extra IC RAM (if present)		(64 paginas)
    0x100000 – 0x17FFFF (512K) => 2nd Extra IC RAM (if present)		(64 paginas)
    0x180000 – 0xFFFFFF (512K) => 3rd Extra IC RAM (if present)		(64 paginas)

    0x200000 (2 MB)


*/





	int i,indice;

	//Los 8 KB de la fpga ROM estan al final
	tbblue_fpga_rom=&memoria_spectrum[2*1024*1024];

	//224 Paginas RAM spectrum 512k
	for (i=0;i<TBBLUE_MAX_SRAM_8KB_BLOCKS;i++) {
		indice=0x040000+8192*i;
		tbblue_ram_memory_pages[i]=&memoria_spectrum[indice];
	}

	//4 Paginas ROM
	for (i=0;i<8;i++) {
		indice=0+8192*i;
		tbblue_rom_memory_pages[i]=&memoria_spectrum[indice];
	}

}

int tbblue_get_limit_sram_page(int page)
{

	z80_byte max=tbblue_return_max_extra_blocks();

	if (page>max-1) page=max-1;

	return page;
}

void tbblue_set_ram_page(z80_byte segment)
{
	z80_byte tbblue_register=80+segment;
	z80_byte reg_value=tbblue_registers[tbblue_register];

	//tbblue_memory_paged[segment]=tbblue_ram_memory_pages[page];
	reg_value=tbblue_get_limit_sram_page(reg_value);
	tbblue_memory_paged[segment]=tbblue_ram_memory_pages[reg_value];

	debug_paginas_memoria_mapeadas[segment]=reg_value;
}


void tbblue_set_rom_page_no_255(z80_byte segment)
{
        z80_byte tbblue_register=80+segment;
        z80_byte reg_value=tbblue_registers[tbblue_register];

	reg_value=tbblue_get_limit_sram_page(reg_value);
	tbblue_memory_paged[segment]=tbblue_ram_memory_pages[reg_value];
	debug_paginas_memoria_mapeadas[segment]=reg_value;
}

void tbblue_set_rom_page(z80_byte segment,z80_byte page)
{
	z80_byte tbblue_register=80+segment;
	z80_byte reg_value=tbblue_registers[tbblue_register];

	if (reg_value==255) {
		page=tbblue_get_limit_sram_page(page);
		tbblue_memory_paged[segment]=tbblue_rom_memory_pages[page];
		debug_paginas_memoria_mapeadas[segment]=DEBUG_PAGINA_MAP_ES_ROM+page;
	}
	else {
		tbblue_set_rom_page_no_255(segment);
	}
}


void tbblue_mem_page_ram_rom(void)
{
	z80_byte page_type;

	page_type=(puerto_8189 >>1) & 3;

	switch (page_type) {
		case 0:
			debug_printf (VERBOSE_DEBUG,"Pages 0,1,2,3");
			tbblue_registers[80]=0*2;
			tbblue_registers[81]=0*2+1;
			tbblue_registers[82]=1*2;
			tbblue_registers[83]=1*2+1;
			tbblue_registers[84]=2*2;
			tbblue_registers[85]=2*2+1;
			tbblue_registers[86]=3*2;
			tbblue_registers[87]=3*2+1;


			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*3);
			tbblue_set_ram_page(3*2+1);

			contend_pages_actual[0]=contend_pages_128k_p2a[0];
			contend_pages_actual[1]=contend_pages_128k_p2a[1];
			contend_pages_actual[2]=contend_pages_128k_p2a[2];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];



			break;

		case 1:
			debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");

			tbblue_registers[80]=4*2;
			tbblue_registers[81]=4*2+1;
			tbblue_registers[82]=5*2;
			tbblue_registers[83]=5*2+1;
			tbblue_registers[84]=6*2;
			tbblue_registers[85]=6*2+1;
			tbblue_registers[86]=7*2;
			tbblue_registers[87]=7*2+1;

			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);


			contend_pages_actual[0]=contend_pages_128k_p2a[4];
			contend_pages_actual[1]=contend_pages_128k_p2a[5];
			contend_pages_actual[2]=contend_pages_128k_p2a[6];
			contend_pages_actual[3]=contend_pages_128k_p2a[7];





			break;

		case 2:
			debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,3");

			tbblue_registers[80]=4*2;
			tbblue_registers[81]=4*2+1;
			tbblue_registers[82]=5*2;
			tbblue_registers[83]=5*2+1;
			tbblue_registers[84]=6*2;
			tbblue_registers[85]=6*2+1;
			tbblue_registers[86]=3*2;
			tbblue_registers[87]=3*2+1;

			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);

			contend_pages_actual[0]=contend_pages_128k_p2a[4];
			contend_pages_actual[1]=contend_pages_128k_p2a[5];
			contend_pages_actual[2]=contend_pages_128k_p2a[6];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];




			break;

		case 3:
			debug_printf (VERBOSE_DEBUG,"Pages 4,7,6,3");

			tbblue_registers[80]=4*2;
			tbblue_registers[81]=4*2+1;
			tbblue_registers[82]=7*2;
			tbblue_registers[83]=7*2+1;
			tbblue_registers[84]=6*2;
			tbblue_registers[85]=6*2+1;
			tbblue_registers[86]=3*2;
			tbblue_registers[87]=3*2+1;

			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);

			contend_pages_actual[0]=contend_pages_128k_p2a[4];
			contend_pages_actual[1]=contend_pages_128k_p2a[7];
			contend_pages_actual[2]=contend_pages_128k_p2a[6];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];




			break;

	}
}

z80_byte disabled_tbblue_mem_get_ram_page(void)
{

	//printf ("Valor 32765: %d\n",puerto_32765);

	z80_byte ram_entra=puerto_32765&7;

	z80_byte bit3=0;
	z80_byte bit4=0;

	//Forzamos a que lea siempre bit 6 y 7 del puerto 32765
	//Dejamos esto asi por si en un futuro hay manera de limitar la lectura de esos bits

	int multiplicador=4; //multiplicamos 128*4

	if (multiplicador==2 || multiplicador==4) {
		bit3=puerto_32765&64;  //Bit 6
		//Lo movemos a bit 3
		bit3=bit3>>3;
	}

  if (multiplicador==4) {
      bit4=puerto_32765&128;  //Bit 7
      //Lo movemos a bit 4
      bit4=bit4>>3;
  }


	ram_entra=ram_entra|bit3|bit4;

	//printf ("ram entra: %d\n",ram_entra);

	return ram_entra;
}


void tbblue_set_mmu_128k_default(void)
{
	//rom default, paginas ram 5,2,0
	tbblue_registers[80]=255;
	tbblue_registers[81]=255;
	tbblue_registers[82]=10;
	tbblue_registers[83]=11;
	tbblue_registers[84]=4;
	tbblue_registers[85]=5;
	tbblue_registers[86]=0;
	tbblue_registers[87]=1;

	debug_paginas_memoria_mapeadas[0]=0;
	debug_paginas_memoria_mapeadas[1]=1;
	debug_paginas_memoria_mapeadas[2]=10;
	debug_paginas_memoria_mapeadas[3]=11;
	debug_paginas_memoria_mapeadas[4]=4;
	debug_paginas_memoria_mapeadas[5]=5;
	debug_paginas_memoria_mapeadas[6]=0;
	debug_paginas_memoria_mapeadas[7]=1;

}

//Indica si estamos en modo ram in rom del +2a
z80_bit tbblue_was_in_p2a_ram_in_rom={0};


void tbblue_set_memory_pages(void)
{
	//Mapeamos paginas de RAM segun config maquina
	z80_byte maquina=(tbblue_registers[3])&7;

	int romram_page;
	//int ram_page;
	int rom_page;
	int indice;

	//Por defecto
	tbblue_low_segment_writable.v=0;

	//printf ("tbblue set memory pages. maquina=%d\n",maquina);
	/*
	bits 1-0 = Machine type:
		00 = Config mode (bootrom)
		01 = ZX 48K
		10 = ZX 128K
		11 = ZX +2/+3e
	*/

	z80_byte contend_page_high_segment=tbblue_registers[86]/2;

	switch (maquina) {
		case 1:
                    //001 = ZX 48K
			tbblue_set_rom_page(0,0*2);
			tbblue_set_rom_page(1,0*2+1);
			tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);
			tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);
			tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);


		        contend_pages_actual[0]=0;
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[0];


			//tbblue_low_segment_writable.v=0;
		break;

		case 2:
                    //010 = ZX 128K
			rom_page=(puerto_32765>>4)&1;
                        tbblue_set_rom_page(0,rom_page*2);
			tbblue_set_rom_page(1,rom_page*2+1);

                        tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);

                        tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);

			//ram_page=tbblue_mem_get_ram_page();
			//tbblue_registers[80+6]=ram_page*2;
			//tbblue_registers[80+7]=ram_page*2+1;


                        tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);



			//tbblue_low_segment_writable.v=0;
		        contend_pages_actual[0]=0;
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[contend_page_high_segment];


		break;

		case 3:
			//011 = ZX +2/+3e
			//Si RAM en ROM
			if (puerto_8189&1) {

				tbblue_mem_page_ram_rom();
				//printf ("setting low segment writeable as port 8189 bit 1\n");
				tbblue_low_segment_writable.v=1;

				tbblue_was_in_p2a_ram_in_rom.v=1;
			}

			else {

				//printf ("NOT setting low segment writeable as port 8189 bit 1\n");

			 	//Si se cambiaba de modo ram in rom a normal
				if (tbblue_was_in_p2a_ram_in_rom.v) {
					debug_printf(VERBOSE_DEBUG,"Going from ram in rom mode to normal mode. Setting default ram pages");
					tbblue_set_mmu_128k_default();
					tbblue_was_in_p2a_ram_in_rom.v=0;
				}

                    //when "11"    => maquina <= s_speccy3e;
                        	rom_page=(puerto_32765>>4)&1;

		        	z80_byte rom1f=(puerto_8189>>1)&2;
		        	z80_byte rom7f=(puerto_32765>>4)&1;

				z80_byte rom_page=rom1f | rom7f;

				//printf ("rom: %d:\n",rom_page);


                        	tbblue_set_rom_page(0,rom_page*2);
				tbblue_set_rom_page(1,rom_page*2+1);

                        	tbblue_set_ram_page(2);
				tbblue_set_ram_page(3);

                        	tbblue_set_ram_page(4);
				tbblue_set_ram_page(5);

                        	//ram_page=tbblue_mem_get_ram_page();
				//tbblue_registers[80+6]=ram_page*2;
				//tbblue_registers[80+7]=ram_page*2+1;
                        	tbblue_set_ram_page(6);
				tbblue_set_ram_page(7);

		        	contend_pages_actual[0]=0;
		        	contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        	contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        	contend_pages_actual[3]=contend_pages_128k_p2a[contend_page_high_segment];


			}
		break;

		case 4:
                    //100 = Pentagon 128K. TODO. de momento tal cual 128kb
			rom_page=(puerto_32765>>4)&1;
                        tbblue_set_rom_page(0,rom_page*2);
			tbblue_set_rom_page(1,rom_page*2+1);

                        tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);

                        tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);

			//ram_page=tbblue_mem_get_ram_page();
			//tbblue_registers[80+6]=ram_page*2;
			//tbblue_registers[80+7]=ram_page*2+1;
                        tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);


			//tbblue_low_segment_writable.v=0;
		        contend_pages_actual[0]=0;
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[contend_page_high_segment];


		break;

		default:

			//Caso maquina 0 u otros no contemplados
			//000 = Config mode

			//printf ("tbblue_bootrom.v=%d\n",tbblue_bootrom.v);

			if (tbblue_bootrom.v==0) {
				/*
When the variable 'bootrom' takes '0', page 0 (0-16383) is mapped to the RAM 1024K,
and the page mapping is configured by bits 5-0 of the I/O port 'config1'.
These 6 bits maps 64 16K pages the start of 1024K SRAM space 0-16363 the Speccy,
which allows you access to all SRAM.

->Ampliado a 7 bits (0..128)
*/
				//romram_page=(tbblue_registers[4]&63);
				romram_page=(tbblue_registers[4]&127);
				indice=romram_page*16384;
				//printf ("page on 0-16383: %d offset: %06X\n",romram_page,indice);
				tbblue_memory_paged[0]=&memoria_spectrum[indice];
				tbblue_memory_paged[1]=&memoria_spectrum[indice+8192];
				tbblue_low_segment_writable.v=1;
				//printf ("low segment writable for machine default\n");

				debug_paginas_memoria_mapeadas[0]=romram_page;
				debug_paginas_memoria_mapeadas[1]=romram_page;
			}
			else {
				//In this setting state, the page 0 repeats the content of the ROM 'loader', ie 0-8191 appear memory contents, and repeats 8192-16383
				//La rom es de 8 kb pero la hemos cargado dos veces
				tbblue_memory_paged[0]=tbblue_fpga_rom;
				tbblue_memory_paged[1]=&tbblue_fpga_rom[8192];
				//tbblue_low_segment_writable.v=0;
				//printf ("low segment NON writable for machine default\n");
				debug_paginas_memoria_mapeadas[0]=0;
				debug_paginas_memoria_mapeadas[1]=0;
			}

			tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);
			tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);

			//En modo config, ram7 esta en segmento 3
			tbblue_registers[80+6]=7*2;
			tbblue_registers[80+7]=7*2+1;


			tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);




		        contend_pages_actual[0]=0; //Suponemos que esa pagina no tiene contienda
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[7];

		break;
	}

}

void tbblue_set_emulator_setting_divmmc(void)
{

/*
(W)		06 => Peripheral 2 setting, only in bootrom or config mode:
			bit 7 = Enable turbo mode (0 = disabled, 1 = enabled)
			bit 6 = DAC chip mode (0 = I2S, 1 = JAP)
			bit 5 = Enable Lightpen  (1 = enabled)
			bit 4 = Enable DivMMC (1 = enabled) -> divmmc automatic paging. divmmc memory is supported using manual
		*/
        //z80_byte diven=tbblue_config2&4;
				z80_byte diven=tbblue_registers[6]&16;
        debug_printf (VERBOSE_INFO,"Apply config.divmmc change: %s",(diven ? "enabled" : "disabled") );
        //printf ("Apply config2.divmmc change: %s\n",(diven ? "enabled" : "disabled") );

				if (diven) {
					//printf ("Activando diviface automatic paging\n");
					divmmc_diviface_enable();
					diviface_allow_automatic_paging.v=1;
				}

        //else divmmc_diviface_disable();
				else {
					//printf ("Desactivando diviface automatic paging\n");
					diviface_allow_automatic_paging.v=0;
					//Y hacer un page-out si hay alguna pagina activa
					diviface_paginacion_automatica_activa.v=0;
				}

}




void tbblue_set_emulator_setting_turbo(void)
{
	/*
	(R/W)	07 => Turbo mode
				bit 1-0 = Turbo (00 = 3.5MHz, 01 = 7MHz, 10 = 14MHz, 11 = 28MHz)(Reset to 00 after a PoR or Hard-reset)

				*/
	z80_byte t=tbblue_registers[7] & 3;
	if (t==0) cpu_turbo_speed=1;
	else if (t==1) cpu_turbo_speed=2;
	else if (t==2) cpu_turbo_speed=4;
	else cpu_turbo_speed=8;


	//printf ("Setting turbo: %d\n",cpu_turbo_speed);

	cpu_set_turbo_speed();
}

void tbblue_reset_common(void)
{


	tbblue_registers[18]=8;
	tbblue_registers[19]=11;

	tbblue_registers[20]=TBBLUE_DEFAULT_TRANSPARENT;

	tbblue_registers[21]=0;
	tbblue_registers[22]=0;
	tbblue_registers[23]=0;

	tbblue_registers[24]=191;
	tbblue_registers[25]=191;
	tbblue_registers[26]=191;
	tbblue_registers[28]=0;

	tbblue_registers[30]=0;
	tbblue_registers[31]=0;
	tbblue_registers[34]=0;
	tbblue_registers[35]=0;
	tbblue_registers[50]=0;
	tbblue_registers[51]=0;
	tbblue_registers[66]=15;
	tbblue_registers[67]=0;
	tbblue_registers[74]=0;
	tbblue_registers[97]=0;
	tbblue_registers[98]=0;


	clip_window_layer2[0]=0;
	clip_window_layer2[1]=255;
	clip_window_layer2[2]=0;
	clip_window_layer2[3]=191;

	clip_window_sprites[0]=0;
	clip_window_sprites[1]=255;
	clip_window_sprites[2]=0;
	clip_window_sprites[3]=191;

	clip_window_ula[0]=0;
	clip_window_ula[1]=255;
	clip_window_ula[2]=0;
	clip_window_ula[3]=191;	


	clip_window_layer2_index=clip_window_sprites_index=clip_window_ula_index=0;



	tbblue_copper_pc=0;
	
	tbblue_set_mmu_128k_default();

	tbblue_was_in_p2a_ram_in_rom.v=0;


}

void tbblue_reset(void)
{

	//Los bits reservados los metemos a 0 también

	/*
	(R/W) 02 => Reset:
  bits 7-3 = Reserved, must be 0
  bit 2 = (R) Power-on reset (PoR)
  bit 1 = (R/W) Reading 1 indicates a Hard-reset. If written 1 causes a Hard Reset.
  bit 0 = (R/W) Reading 1 indicates a Soft-reset. If written 1 causes a Soft Reset.
	*/
	tbblue_registers[2]=1;
	

	tbblue_reset_common();



}

void tbblue_hard_reset(void)
{

	/*
	(R/W) 02 => Reset:
	  bits 7-3 = Reserved, must be 0
	  bit 2 = (R) Power-on reset (PoR)
	  bit 1 = (R/W) Reading 1 indicates a Hard-reset. If written 1 causes a Hard Reset.
	  bit 0 = (R/W) Reading 1 indicates a Soft-reset. If written 1 causes a Soft Reset.
	*/

	//Aqui no estoy distinguiendo entre hard reset y power-on reset, dado que al iniciar maquina siempre llama a hard reset
	tbblue_registers[2]=4+2;


	tbblue_registers[3]=0;
	tbblue_registers[4]=0;
	tbblue_registers[5]=0;
	tbblue_registers[6]=0;
	tbblue_registers[7]=0;
	tbblue_registers[8]=0;

	tbblue_reset_common();


	tbblue_reset_palette_write_state();

	tbblue_port_123b=0;


	if (tbblue_fast_boot_mode.v) {
		tbblue_registers[3]=3;
		tbblue_registers[80]=0xff;
		tbblue_registers[81]=0xff;
		tbblue_set_memory_pages();

		if (tbblue_initial_123b_port>=0) tbblue_port_123b=tbblue_initial_123b_port;
	}

	else {
		tbblue_bootrom.v=1;
	//printf ("----setting bootrom to 1\n");
		tbblue_set_memory_pages();
		tbblue_set_emulator_setting_divmmc();
	}



	tbblue_reset_sprites();
	tbblue_reset_palettes();


}





/*
z80_byte old_tbblue_read_port_24d5(void)
{

//- port 0x24D5: 1st read = return hardware number (below), 2nd read = return firmware version number (first nibble.second nibble) e.g. 0x12 = 1.02
//
//hardware numbers
//1 = DE-1 (old)
//2 = DE-2 (old)
//3 = DE-2 (new)
//4 = DE-1 (new)
//5 = FBLabs
//6 = VTrucco
//7 = WXEDA hardware (placa de desarrollo)
//8 = Emulators
//9 = ZX Spectrum Next


/

}

*/

void tbblue_set_timing_128k(void)
{
        contend_read=contend_read_128k;
        contend_read_no_mreq=contend_read_no_mreq_128k;
        contend_write_no_mreq=contend_write_no_mreq_128k;

        ula_contend_port_early=ula_contend_port_early_128k;
        ula_contend_port_late=ula_contend_port_late_128k;


        screen_testados_linea=228;
        screen_invisible_borde_superior=7;
        screen_invisible_borde_derecho=104;

        port_from_ula=port_from_ula_p2a;
        contend_pages_128k_p2a=contend_pages_p2a;

}


void tbblue_set_timing_48k(void)
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

void tbblue_change_timing(int timing)
{
        if (timing==0) tbblue_set_timing_48k();
        else if (timing==1) tbblue_set_timing_128k();

        screen_set_video_params_indices();
        inicializa_tabla_contend();

}

/*
*/
void tbblue_set_emulator_setting_timing(void)
{
	/*
	(W) 0x03 (03) => Set machine type, only in IPL or config mode:
	A write in this register disables the IPL
	(0x0000-0x3FFF are mapped to the RAM instead of the internal ROM)
	bit 7 = lock timing

	bits 6-4 = Timing:
	000 or 001 = ZX 48K
	010 = ZX 128K
	011 = ZX +2/+3e
	100 = Pentagon 128K

	bit 3 = Reserved, must be 0

	bits 2-0 = Machine type:
	000 = Config mode
	001 = ZX 48K
	010 = ZX 128K
	011 = ZX +2/+3e
	100 = Pentagon 128K
	*/


                //z80_byte t=(tbblue_config1 >> 6)&3;
		z80_byte t=(tbblue_registers[3]>>4)&7;

		//TODO: otros timings

                if (t<=1) {
		//48k
				debug_printf (VERBOSE_INFO,"Apply config.timing. change:48k");
				tbblue_change_timing(0);
		}
		else {
		//128k
				debug_printf (VERBOSE_INFO,"Apply config.timing. change:128k");
				tbblue_change_timing(1);
		}




}


void tbblue_set_register_port(z80_byte value)
{
	tbblue_last_register=value;
}

void tbblue_splash_palette_format(void)
{
//if (value&128) screen_print_splash_text(10,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling lores video mode. 128x96 256 colours");
	/*
	(R/W) 0x43 (67) => Palette Control
	bit 0 = Disable the standard Spectrum flash feature to enable the extra colours.
  (Reset to 0 after a reset)

	(R/W) 0x42 (66) => Palette Format
  bits 7-0 = Number of the last ink colour entry on palette. (Reset to 15 after a Reset)
  This number can be 1, 3, 7, 15, 31, 63, 127 or 255.
	

	*/


	if ((tbblue_registers[67]&1)==0) screen_print_splash_text(10,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling extra colour palette");
	else {

		z80_byte palformat=tbblue_registers[66];

		/*
		Ejemplo: mascara 3:   00000011
		Son 4 tintas
		64 papeles

		Para pasar de tintas a papeles :    00000011 -> inverso -> 11111100
		Dividimos 11111100 entre tintas, para rotar el valor 2 veces a la derecha = 252 / 4 = 63   -> +1 -> 64
		*/

		int tintas=palformat+1;

		int papeles=255-palformat;

		//Dado que tintas siempre es +1, nunca habra division por 0. Pero por si acaso

		if (tintas==0) papeles=0;
		else {
			papeles=(papeles/tintas)+1;
		}

		char mensaje[200];
		sprintf (mensaje,"Enabling extra colour palette: %d inks %d papers",tintas,papeles);
		screen_print_splash_text(10,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);
	}
		

}

	
//tbblue_last_register
//void tbblue_set_value_port(z80_byte value)
void tbblue_set_value_port_position(z80_byte index_position,z80_byte value)
{


	//printf ("register port %02XH value %02XH\n",tbblue_last_register,value);

	z80_byte last_register_6=tbblue_registers[6];
	z80_byte last_register_7=tbblue_registers[7];
	z80_byte last_register_21=tbblue_registers[21];
	z80_byte last_register_66=tbblue_registers[66];
	z80_byte last_register_67=tbblue_registers[67];
	

	if (index_position==3) {
		//Controlar caso especial
		//(W) 0x03 (03) => Set machine type, only in IPL or config mode
		//   		bits 2-0 = Machine type:
		//      		000 = Config mode
		z80_byte machine_type=tbblue_registers[3]&7;

		if (!(machine_type==0 || tbblue_bootrom.v)) {
			debug_printf(VERBOSE_DEBUG,"Can not change machine type (to %02XH) while in non config mode or non IPL mode",value);
			return;
		}
	}

	tbblue_registers[index_position]=value;

	switch(index_position)
	{

		case 2:
		/*
		(R/W)	02 => Reset:
					bits 7-3 = Reserved, must be 0
					bit 2 = (R) Power-on reset
					bit 1 = (R/W) if 1 Hard Reset
					bit 0 = (R/W) if 1 Soft Reset
					*/

						//tbblue_hardsoftreset=value;
						if (value&1) {
							//printf ("Doing soft reset due to writing to port 24D9H\n");
							reg_pc=0;
						}
						if (value&2) {
							//printf ("Doing hard reset due to writing to port 24D9H\n");
							tbblue_bootrom.v=1;
							//printf ("----setting bootrom to 1. when writing register 2 and bit 1\n");
							tbblue_registers[3]=0;
							//tbblue_config1=0;
							tbblue_set_memory_pages();
							reg_pc=0;
						}

					break;


		break;

		case 3:
		/*
		(W) 0x03 (03) => Set machine type, only in IPL or config mode:
   		A write in this register disables the IPL
   		(0x0000-0x3FFF are mapped to the RAM instead of the internal ROM)
   		bit 7 = lock timing
   		bits 6-4 = Timing:
      		000 or 001 = ZX 48K
      		010 = ZX 128K
      		011 = ZX +2/+3e
      		100 = Pentagon 128K
   		bit 3 = Reserved, must be 0
   		bits 2-0 = Machine type:
      		000 = Config mode
      		001 = ZX 48K
      		010 = ZX 128K
      		011 = ZX +2/+3e
      		100 = Pentagon 128K
      		*/

		/*  OLD:
				(W)		03 => Set machine type, only in bootrom or config mode:
							A write in this register disables the bootrom mode (0000 to 3FFF are mapped to the RAM instead of the internal ROM)
							bits 7-5 = Reserved, must be 0
							bits 4-3 = Timing:
								00,
								01 = ZX 48K
								10 = ZX 128K
								11 = ZX +2/+3e
							bit 2 = Reserved, must be 0
							bits 1-0 = Machine type:
								00 = Config mode (bootrom)
								01 = ZX 48K
								10 = ZX 128K
								11 = ZX +2/+3e
								*/
			//Pentagon not supported yet. TODO
			//last_value=tbblue_config1;
			tbblue_bootrom.v=0;
			//printf ("----setting bootrom to 0\n");

			//printf ("Writing register 3 value %02XH\n",value);

			tbblue_set_memory_pages();


			//Solo cuando hay cambio
			//if ( last_register_3 != value )
			tbblue_set_emulator_setting_timing();
		break;


		break;

		case 4:

/*
		(W)		04 => Set page RAM, only in config mode (no bootrom):
					bits 7-5 = Reserved, must be 0
					bits 4-0 = RAM page mapped in 0000-3FFF (32 pages of 16K = 512K)
			*/

			tbblue_set_memory_pages();

		break;

		

		case 6:
			//Si hay cambio en DivMMC
			/*
			(W)		06 => Peripheral 2 setting, only in bootrom or config mode:

						bit 4 = Enable DivMMC (1 = enabled)
					*/
			if ( (last_register_6&16) != (value&16)) tbblue_set_emulator_setting_divmmc();
		break;


		case 7:
		/*
		(R/W)	07 => Turbo mode
					bit 0 = Turbo (0 = 3.5MHz, 1 = 7MHz)
					*/
					if ( last_register_7 != value ) tbblue_set_emulator_setting_turbo();
		break;

		case 21:
			//modo lores
			if ( (last_register_21&128) != (value&128)) {
				if (value&128) screen_print_splash_text(10,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling lores video mode. 128x96 256 colours");
				else screen_print_splash_text(10,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling lores video mode");
			}
		break;


		case 24:
			//(W) 0x18 (24) => Clip Window Layer 2
			clip_window_layer2[clip_window_layer2_index&3]=value;
			clip_window_layer2_index++;

			//debug
			//printf ("layer2 %d %d %d %d\n",clip_window_layer2[0],clip_window_layer2[1],clip_window_layer2[2],clip_window_layer2[3]);
		break;

		case 25:
			//((W) 0x19 (25) => Clip Window Sprites
			clip_window_sprites[clip_window_sprites_index&3]=value;
			clip_window_sprites_index++;

			//debug
			//printf ("sprites %d %d %d %d\n",clip_window_sprites[0],clip_window_sprites[1],clip_window_sprites[2],clip_window_sprites[3]);
		break;	

		case 26:
			//(W) 0x1A (26) => Clip Window ULA/LoRes
			clip_window_ula[clip_window_ula_index&3]=value;
			clip_window_ula_index++;

			//debug
			//printf ("ula %d %d %d %d\n",clip_window_ula[0],clip_window_ula[1],clip_window_ula[2],clip_window_ula[3]);
		break;				

		case 28:
		/*
		(W) 0x1C (28) => Clip Window control
  bits 7-3 = Reserved, must be 0
  bit 2 - reset the ULA/LoRes clip index.
  bit 1 - reset the sprite clip index.
  bit 0 - reset the Layer 2 clip index.
  	*/

			if (value&1) clip_window_layer2_index=0;
			if (value&2) clip_window_sprites_index=0;
			if (value&4) clip_window_ula_index=0;

		break;

/*
(W) 0x2D (45) => SoundDrive (SpecDrum) port 0xDF mirror
 bits 7-0 = Data to be written at Soundrive
 this port cand be used to send data to the SoundDrive using the Copper co-processor
*/

		case 45:

/*
        //DAC Audio
        if (audiodac_enabled.v && puerto_l==audiodac_types[audiodac_selected_type].port) {
                audiodac_last_value_data=value;
                silence_detection_counter=0;
        }
*/

		if (audiodac_enabled.v) audiodac_last_value_data=value;

		break;

		case 64:
			//palette index
			tbblue_reset_palette_write_state();
		break;


		//(R/W) 0x41 (65) => Palette Value
		case 65:
			tbblue_write_palette_value_high8(value);
			tbblue_increment_palette_index();
		break;

		case 66:
			if ( (last_register_66 != value) && tbblue_registers[67]&1) tbblue_splash_palette_format();
		break;

		case 67:
			if ( (last_register_67&1) != (value&1) ) tbblue_splash_palette_format();
		break;

		
		case 68:
			tbblue_write_palette_value_high8_low1(value);
		break;


		//MMU
		case 80:
		case 81:
			tbblue_set_memory_pages();
		break;

		case 82:
		case 83:
		case 84:
		case 85:
		case 86:
		case 87:
			tbblue_set_memory_pages();
		break;

		case 96:
/*
(W) 0x60 (96) => Copper data
  bits 7-0 = Byte to write at "Copper list"
  Note that each copper instruction is composed by two bytes (16 bits).
*/

		//printf ("0x60 (96) => Copper data value %02XH\n",value);

		tbblue_copper_write_data(value);

		break;

		case 97:
/*
(W) 0x61 (97) => Copper control LO bit
  bits 7-0 = Copper list index address LSB.
  After the write, the index is auto-incremented to the next memory position.
  (Index is set to 0 after a reset)
*/

		//printf ("0x61 (97) => Copper control LO bit value %02XH\n",value);

		//tbblue_copper_increment_write_position();
		//sleep(1);

		break;

		case 98:
/*
(W) 0x62 (98) => Copper control HI bit
   bits 7-6 = Start control
       00 = Copper fully stoped
       01 = Copper start, execute the list, then stop at last adress
       10 = Copper start, execute the list, then loop the list from start
       11 = Copper start, execute the list and restart the list at each frame
   bits 2-0 = Copper list index address MSB
*/

		//printf ("0x62 (98) => Copper control HI bit value %02XH\n",value);
		//tbblue_copper_increment_write_position();
		//sleep(1);
		tbblue_copper_write_control_hi_byte(value);

		break;

	}


}


//tbblue_last_register
void tbblue_set_value_port(z80_byte value)
{
	tbblue_set_value_port_position(tbblue_last_register,value);
}

int tbblue_get_raster_line(void)
{
	/*
	Line 0 is first video line. In truth the line is the Y counter, Video is from 0 to 191, borders and hsync is >192
Same this page: http://www.zxdesign.info/vertcontrol.shtml


Row	Start	Row End	Length	Description
0		191	192	Video Display
192	247	56	Bottom Border
248	255	8	Vertical Sync
256	312	56	Top Border

*/
	if (t_scanline>=screen_indice_inicio_pant) return t_scanline-screen_indice_inicio_pant;
	else return t_scanline+192+screen_total_borde_inferior;


}


z80_byte tbblue_get_value_port_register(z80_byte registro)
{

	int linea_raster;

	//Casos especiales
	/*
	(R) 0x00 (00) => Machine ID

	(R) 0x01 (01) => Version (Nibble most significant = Major, Nibble less significant = Minor)
	*/
	switch(registro)
	{
		case 0:
			return 8;
		break;

		case 1:
			return 0x19;
		break;

		/*




		(R) 0x1E (30) => Active video line (MSB)
  bits 7-1 = Reserved, always 0
  bit 0 = Active line MSB (Reset to 0 after a reset)

(R) 0x1F (31) = Active video line (LSB)
  bits 7-0 = Active line LSB (0-255)(Reset to 0 after a reset)
		*/

		case 30:
			linea_raster=tbblue_get_raster_line();
			linea_raster=linea_raster >> 8;
			return (linea_raster&1);
		break;

		case 31:
			linea_raster=tbblue_get_raster_line();
			return (linea_raster&0xFF);
		break;




	}


	return tbblue_registers[registro];
}



z80_byte tbblue_get_value_port(void)
{
	return tbblue_get_value_port_register(tbblue_last_register);
}



//Devuelve puntero a direccion de memoria donde esta el scanline en modo lores para direccion y
z80_byte *get_lores_pointer(int y)
{
	z80_byte *base_pointer;

	//Siempre saldra de ram 5
	base_pointer=tbblue_ram_memory_pages[5*2];	

	//128x96 one byte per pixel in left to right, top to bottom order so that the 
	//top half of the screen is in the first timex display file at 0x4000 
	//and the bottom half is in the second timex display file at 0x6000
	
	z80_int offset=0;

	//int yorig=y;

	const int mitad_alto=96/2;

	//Segunda mitad
	if (y>=mitad_alto) {
		//printf ("segundo bloque. y=%d offset=%d\n",y,offset);
		offset +=0x2000;
		y=y-mitad_alto;
	}

	//Sumamos desplazamiento por y
	offset +=y*128;

	//printf ("y: %d offset: %d\n",yorig,offset);

	base_pointer +=offset;

	return base_pointer;
}



//Inicializa punteros a los 3 layers
z80_int *p_layer_first;
z80_int *p_layer_second;
z80_int *p_layer_third;

void tbblue_set_layer_priorities(void)
{
	//Por defecto
	//sprites over the Layer 2, over the ULA graphics
	p_layer_first=tbblue_layer_sprites;
	p_layer_second=tbblue_layer_layer2;
	p_layer_third=tbblue_layer_ula;

	/*
	(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  bits 6-5 = Reserved, must be 0
  bits 4-2 = set layers priorities:
     Reset default is 000, sprites over the Layer 2, over the ULA graphics
     000 - S L U
     001 - L S U
     010 - S U L
     011 - L U S
     100 - U S L
     101 - U L S
  bit 1 = Over border (1 = yes)(Back to 0 after a reset)
  bit 0 = Sprites visible (1 = visible)(Back to 0 after a reset)
  */
	z80_byte prio=(tbblue_registers[0x15] >> 2)&7;

	switch (prio) {
		case 0:
			p_layer_first=tbblue_layer_sprites;
			p_layer_second=tbblue_layer_layer2;
			p_layer_third=tbblue_layer_ula;
		break;

		case 1:
			p_layer_first=tbblue_layer_layer2;
			p_layer_second=tbblue_layer_sprites;
			p_layer_third=tbblue_layer_ula;
		break;


		case 2:
			p_layer_first=tbblue_layer_sprites;
			p_layer_second=tbblue_layer_ula;
			p_layer_third=tbblue_layer_layer2;
		break;

		case 3:
			p_layer_first=tbblue_layer_layer2;
			p_layer_second=tbblue_layer_ula;
			p_layer_third=tbblue_layer_sprites;
		break;

		case 4:
			p_layer_first=tbblue_layer_ula;
			p_layer_second=tbblue_layer_sprites;
			p_layer_third=tbblue_layer_layer2;
		break;

		case 5:
			p_layer_first=tbblue_layer_ula;
			p_layer_second=tbblue_layer_layer2;
			p_layer_third=tbblue_layer_sprites;
		break;
	}

}

void get_pixel_color_tbblue(z80_byte attribute,z80_int *tinta_orig, z80_int *papel_orig)
{

	/*
(R/W) 0x43 (67) => Palette Control
  bit 0 = Disable the standard Spectrum flash feature to enable the extra colours.
  (Reset to 0 after a reset)
	*/

	z80_byte ink=*tinta_orig;
	z80_byte paper=*papel_orig;

	z80_byte palette_format=tbblue_registers[0x42];
	z80_byte flash_disabled=tbblue_registers[0x43]&1;


        z80_byte bright,flash;
        z80_int aux;

	if (!flash_disabled) {

                        ink=attribute &7; 
                        paper=((attribute>>3) &7)+128; //colores papel empiezan en 128
                        bright=(attribute)&64; 
                        flash=(attribute)&128; 
                        if (flash) { 
                                if (estado_parpadeo.v) { 
                                        aux=paper; 
                                        paper=ink; 
                                        ink=aux; 
                                } 
                        } 
            
            if (bright) {   
                paper+=8; 
                ink+=8; 
            } 

	}

	else {
      /*
(R/W) 0x42 (66) => Palette Format
  bits 7-0 = Number of the last ink colour entry on palette. (Reset to 15 after a Reset)
  This number can be 1, 3, 7, 15, 31, 63, 127 or 255.
  The 255 value enables the full ink colour mode and
  all the the palette entries are inks but the paper will be the colour at position 128.
  (only applies to ULANext palette. Layer 2 and Sprite palettes works as "full ink")
        */
		int rotacion_papel=1;
		int mascara_tinta=palette_format;
		int mascara_papel=255-mascara_tinta;

		//Estos valores se podrian tener ya calculados al llamar desde la funcion de screen_store_scanline_rainbow_solo_display_tbblue
		//o incluso calcularlos en cuanto se modificase el registro 42h o 43h
		//Como realmente son pocas variables a calcular, quiza ni merece la pena

		switch (mascara_tinta) {
			case 1:
				rotacion_papel=1;
			break;
	
			case 3:
				rotacion_papel=2;
			break;

			case 7:
				rotacion_papel=3;
			break;

			case 15:
				rotacion_papel=4;
			break;

			case 31:
				rotacion_papel=5;
			break;

			case 63:
				rotacion_papel=6;
			break;

			case 127:
				rotacion_papel=7;
			break;

		}

		if (mascara_tinta==255) {
			paper=128;
			ink=attribute;
		}

		else {
			ink=attribute & mascara_tinta;
			paper=((attribute & mascara_papel) >> rotacion_papel)+128;
		}	

	}			

	*tinta_orig=ink;		
	*papel_orig=paper;

}


//Guardar en buffer rainbow la linea actual. Para Spectrum. solo display
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
void screen_store_scanline_rainbow_solo_display_tbblue(void)
{

	//si linea no coincide con entrelazado, volvemos
	if (if_store_scanline_interlace(t_scanline_draw)==0) return;

	int i;
	for (i=0;i<512;i++) {
		tbblue_layer_ula[i]=TBBLUE_TRANSPARENT_REGISTER_9;
		tbblue_layer_layer2[i]=TBBLUE_TRANSPARENT_REGISTER_9;
		tbblue_layer_sprites[i]=TBBLUE_TRANSPARENT_REGISTER_9;
	}

	int bordesupinf=0;

  //En zona visible pantalla (no borde superior ni inferior)
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
	//z80_int dir_atributo;
        z80_byte byte_leido;


        int color=0;
        //int fila;

        z80_byte attribute;
        //z80_byte bright,flash;
	z80_int ink,paper;
	//z80_int aux;


        z80_byte *screen=get_base_mem_pantalla();

        direccion=screen_addr_table[(scanline_copia<<5)];

				//Inicializar puntero a layer2 de tbblue, irlo incrementando a medida que se ponen pixeles
				//Layer2 siempre se dibuja desde registro que indique pagina 18. Registro 19 es un backbuffer pero siempre se dibuja desde 18
				//int tbblue_layer2_offset=tbblue_registers[18]&63;

				//tbblue_layer2_offset*=16384;



				int tbblue_layer2_offset=tbblue_get_offset_start_layer2();


				//Mantener el offset y en 0..191
				z80_byte tbblue_reg_23=tbblue_registers[23];


				tbblue_reg_23 +=scanline_copia;
				tbblue_reg_23=tbblue_reg_23 % 192;

				tbblue_layer2_offset +=tbblue_reg_23*256;

				z80_byte tbblue_reg_22=tbblue_registers[22];

/*
(R/W) 22 => Layer2 Offset X
  bits 7-0 = X Offset (0-255)(Reset to 0 after a reset)

(R/W) 0x17 (23) => Layer2 Offset Y
  bits 7-0 = Y Offset (0-191)(Reset to 0 after a reset)
*/


        //fila=scanline_copia/8;
        //dir_atributo=6144+(fila*32);


	z80_byte *puntero_buffer_atributos;


	//Si modo timex 512x192 pero se hace modo escalado
	//Si es modo timex 512x192, llamar a otra funcion
        if (timex_si_modo_512_y_zoom_par() ) {
                //Si zoom x par
                if (timex_mode_512192_real.v) {
                	return;
        	}
        }


	//temporal modo 6 timex 512x192 pero hacemos 256x192
	z80_byte temp_prueba_modo6[SCANLINEBUFFER_ONE_ARRAY_LENGTH];
	z80_byte col6;
	z80_byte tin6, pap6;

	z80_byte timex_video_mode=timex_port_ff&7;
	z80_byte timexhires_resultante;
	z80_int timexhires_origen;

	z80_bit si_timex_hires={0};

	//Por defecto
	puntero_buffer_atributos=scanline_buffer;

	/* modo lores
	(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  bits 6-5 = Reserved, must be 0
  	*/

  	int tbblue_lores=tbblue_registers[0x15] & 128;

  	z80_byte *lores_pointer;
  	z80_byte posicion_x_lores_pointer=0;

  	if (tbblue_lores) {
  		int linea_lores=scanline_copia;  
  		//Sumamos offset y
  		/*
  		(R/W) 0x33 (51) => LoRes Offset Y
  bits 7-0 = Y Offset (0-191)(Reset to 0 after a reset)
  Being only 96 pixels, this allows the display to scroll in "half-pixels",
  at the same resolution and smoothness as Layer 2.
  		*/
  		linea_lores +=tbblue_registers[0x33];

  		linea_lores=linea_lores % 192;
  		//if (linea_lores>=192) linea_lores -=192;

  		lores_pointer=get_lores_pointer(linea_lores/2);  //admite hasta y=95, dividimos entre 2 linea actual



  		//Y scroll horizontal
  		posicion_x_lores_pointer=tbblue_registers[0x32];
  	}


	if (timex_video_emulation.v) {
		//Modos de video Timex
		/*
000 - Video data at address 16384 and 8x8 color attributes at address 22528 (like on ordinary Spectrum);

001 - Video data at address 24576 and 8x8 color attributes at address 30720;

010 - Multicolor mode: video data at address 16384 and 8x1 color attributes at address 24576;

110 - Extended resolution: without color attributes, even columns of video data are taken from address 16384, and odd columns of video data are taken from address 24576
		*/
		switch (timex_video_mode) {

			case 4:
			case 6:
				//512x192 monocromo. aunque hacemos 256x192
				//y color siempre fijo
				/*
bits D3-D5: Selection of ink and paper color in extended screen resolution mode (000=black/white, 001=blue/yellow, 010=red/cyan, 011=magenta/green, 100=green/magenta, 101=cyan/red, 110=yellow/blue, 111=white/black); these bits are ignored when D2=0

				black, blue, red, magenta, green, cyan, yellow, white
				*/

				//Si D2==0, these bits are ignored when D2=0?? Modo 4 que es??

				//col6=(timex_port_ff>>3)&7;
				tin6=get_timex_ink_mode6_color();


				//Obtenemos color
				//tin6=col6;
				pap6=get_timex_paper_mode6_color();

				//Y con brillo
				col6=((pap6*8)+tin6)+64;

				//Nos inventamos un array de colores, con mismo color siempre, correspondiente a lo que dice el registro timex
				//Saltamos de dos en dos
				//De manera similar al buffer scanlines_buffer, hay pixel, atributo, pixel, atributo, etc
				//por eso solo llenamos la parte que afecta al atributo

				puntero_buffer_atributos=temp_prueba_modo6;
				int i;
				for (i=1;i<SCANLINEBUFFER_ONE_ARRAY_LENGTH;i+=2) {
					temp_prueba_modo6[i]=col6;
				}
				si_timex_hires.v=1;
			break;


		}
	}

	int posicion_array_layer=0;

	posicion_array_layer +=screen_total_borde_izquierdo*border_enabled.v;


	int posicion_array_pixeles_atributos=0;
        for (x=0;x<32;x++) {


                        //byte_leido=screen[direccion];
                        byte_leido=puntero_buffer_atributos[posicion_array_pixeles_atributos++];

			//Timex. Reducir 512x192 a 256x192.
			//Obtenemos los dos bytes implicados, metemos en variable de 16 bits,
			//Y vamos comprimiendo cada 2 pixeles. De cada 2 pixeles, si los dos son 0, metemos 0. Si alguno o los dos son 1, metemos 1
			//Esto es muy lento

			if (si_timex_hires.v) {

					//comprimir bytes
					timexhires_resultante=0;
					//timexhires_origen=byte_leido*256+screen[direccion+8192];
					timexhires_origen=screen[direccion]*256+screen[direccion+8192];

					//comprimir pixeles
					int i;
					for (i=0;i<8;i++) {
						timexhires_resultante=timexhires_resultante<<1;
						if ( (timexhires_origen&(32768+16384))   ) timexhires_resultante |=1;
						timexhires_origen=timexhires_origen<<2;
					}

					byte_leido=timexhires_resultante;

			}



            attribute=puntero_buffer_atributos[posicion_array_pixeles_atributos++];

               

			get_pixel_color_tbblue(attribute,&ink,&paper);
			

			//int cambiada_tinta;
			//int cambiada_paper;

			//cambiada_tinta=0;
			//cambiada_paper=0;

            for (bit=0;bit<8;bit++) {

				
				color= ( byte_leido & 128 ? ink : paper ) ;

				if (tbblue_lores) {
					

					z80_byte lorescolor=lores_pointer[posicion_x_lores_pointer/2];
					//tenemos indice color de paleta
					//transformar a color final segun paleta ula activa
					//color=tbblue_get_palette_active_ula(lorescolor);

					color=lorescolor;

					posicion_x_lores_pointer++; 
				}

				int posx=x*8+bit; //Posicion pixel. Para clip window registers							

				//Capa ula
				//Tener en cuenta valor clip window
				
				//(W) 0x1A (26) => Clip Window ULA/LoRes
				if (posx>=clip_window_ula[0] && posx<=clip_window_ula[1] && scanline_copia>=clip_window_ula[2] && scanline_copia<=clip_window_ula[3]) {
				tbblue_layer_ula[posicion_array_layer]=tbblue_get_palette_active_ula(color);
				}

				//Capa layer2
				if (tbblue_is_active_layer2()) {
					if (posx>=clip_window_layer2[0] && posx<=clip_window_layer2[1] && scanline_copia>=clip_window_layer2[2] && scanline_copia<=clip_window_layer2[3]) {
					z80_byte color_layer2=memoria_spectrum[tbblue_layer2_offset+tbblue_reg_22];
					z80_int final_color_layer2=tbblue_get_palette_active_layer2(color_layer2);
					tbblue_layer_layer2[posicion_array_layer]=final_color_layer2;
					}
				}

				posicion_array_layer++;

                                byte_leido=byte_leido<<1;


					
				tbblue_reg_22++;
            }
			direccion++;


        }

		//printf ("posicion_array_layer: %d\n",posicion_array_layer);




	}

	else {
		bordesupinf=1;
	}

	//Aqui puede ser borde superior o inferior

	//capa sprites. Si clip window y corresponde:
	z80_byte sprites_over_border=tbblue_registers[21]&2;
	//Clip window on Sprites only work when the "over border bit" is disabled
	int mostrar_sprites=1;
	if (sprites_over_border==0) {
		int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;
		if (scanline_copia<clip_window_sprites[2] || scanline_copia>clip_window_sprites[3]) mostrar_sprites=0;
	}

	
	if (mostrar_sprites) {
	tbsprite_do_overlay();
	}


	//int i;

	//int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

        //la copiamos a buffer rainbow
        //z80_int *puntero_buf_rainbow;
        //esto podria ser un contador y no hace falta que lo recalculemos cada vez. TODO
        int y;

        y=t_scanline_draw-screen_invisible_borde_superior;
        if (border_enabled.v==0) y=y-screen_borde_superior;

	//puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;
	z80_int *puntero_final_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];

	//Por defecto
	//sprites over the Layer 2, over the ULA graphics

	//p_layer_first=tbblue_layer_sprites;
	//p_layer_second=tbblue_layer_layer2;
	//p_layer_third=tbblue_layer_ula;

	tbblue_set_layer_priorities();




	z80_int color;
	//z80_int color_final=;

	for (i=0;i<get_total_ancho_rainbow();i++) {

		//Primera capa
		color=p_layer_first[i];
		if (!tbblue_si_transparent(color)) {
			*puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
		}

		else {
			color=p_layer_second[i];
			if (!tbblue_si_transparent(color)) {
				*puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
			}

			else {
				color=p_layer_third[i];
				if (!tbblue_si_transparent(color)) {
					*puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
				}
				//TODO: que pasa si las tres capas son transparentes
				else {
					if (bordesupinf) {
					//Si estamos en borde inferior o superior, no hacemos nada, dibujar color borde
					}

					else {
						//Borde izquierdo o derecho o pantalla. Ver si estamos en pantalla
						if (i>=screen_total_borde_izquierdo*border_enabled.v &&
							i<screen_total_borde_izquierdo*border_enabled.v+256) {
							//Poner color negro
							*puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+0;
						}

						else {
						//Es borde. dejar ese color
						}
					
					}
				}
			}

		}

		puntero_final_rainbow++;

		
	}

}




z80_byte return_tbblue_mmu_segment(z80_int dir)
{
        int segmento=dir/8192;
        z80_byte reg_mmu_value=tbblue_registers[80+segmento];
        return reg_mmu_value;
}


//Si la zona de 0-16383 es escribible por mmu (registro 80/81 contiene no 255)
int tbblue_is_writable_segment_mmu_rom_space(z80_int dir)
{
	//En maquina en config mode no tiene sentido
	z80_byte maquina=(tbblue_registers[3])&7;
	if (maquina==0) return 0;

	z80_byte mmu_value=return_tbblue_mmu_segment(dir);
	if (mmu_value!=255) return 1;
	else return 0;
}

