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
#include <string.h>

#include "cpu.h"
#include "tbblue.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "utils.h"
#include "zxvision.h"
#include "divmmc.h"
#include "diviface.h"
#include "screen.h"

#include "timex.h"
#include "ula.h"
#include "audio.h"

#include "datagear.h"
#include "ay38912.h"
#include "multiface.h"
#include "uartbridge.h"
#include "chardevice.h"
#include "settings.h"
#include "joystick.h"



//Punteros a los bloques de 8kb de ram de spectrum
z80_byte *tbblue_ram_memory_pages[TBBLUE_MAX_SRAM_8KB_BLOCKS];

//2MB->224 bloques (16+16+64*3) (3 bloques extra de 512)
//1.5 MB->160 bloques (2 bloques extra de 512)
//1 MB->96 bloques (1 bloque extra de 512)
//512 KB->32 bloques de (0 bloque extra de 512)

z80_byte tbblue_extra_512kb_blocks=3;

int tbblue_use_rtc_traps=1;

//Establece numero de bloques extra de 512kb en base a memoria indicada en KB
void tbblue_set_ram_blocks(int memoria_kb)
{
	if (memoria_kb>=1536) tbblue_extra_512kb_blocks=3;
	else if (memoria_kb>=1024) tbblue_extra_512kb_blocks=2;
	else if (memoria_kb>=512) tbblue_extra_512kb_blocks=1;
	else tbblue_extra_512kb_blocks=0;

	//printf ("tbblue_extra_512kb_blocks: %d\n",tbblue_extra_512kb_blocks);
}

//Autoactivar real video solo la primera vez que se entra en set_machine con maquina tbblue
int tbblue_already_autoenabled_rainbow=0;

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


//Por defecto denegado
z80_bit tbblue_deny_turbo_rom={1};

//Maximo turbo permitido al habilitar tbblue_deny_turbo_rom. Por defecto maximo 2X
int tbblue_deny_turbo_rom_max_allowed=2;


//
//Inicio Variables, memoria etc de estado de la máquina. Se suelen guardar/cargar en snapshot ZSF
//


//'bootrom' takes '1' on hard-reset and takes '0' if there is any writing on the i/o port 'config1'. It can not be read.
z80_bit tbblue_bootrom={1};

//Copper
z80_byte tbblue_copper_memory[TBBLUE_COPPER_MEMORY];


//Indice al opcode copper a ejecutar
z80_int tbblue_copper_pc=0;

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
//     011 = Tilemap first palette
//     111 = Tilemap second palette
//Paletas de 256 colores formato RGB9 RRRGGGBBB
//Valores son de 9 bits por tanto lo definimos con z80_int que es de 16 bits
z80_int tbblue_palette_ula_first[256];
z80_int tbblue_palette_ula_second[256];
z80_int tbblue_palette_layer2_first[256];
z80_int tbblue_palette_layer2_second[256];
z80_int tbblue_palette_sprite_first[256];
z80_int tbblue_palette_sprite_second[256];
z80_int tbblue_palette_tilemap_first[256];
z80_int tbblue_palette_tilemap_second[256];





//64 patterns de Sprites
/*
In the palette each byte represents the colors in the RRRGGGBB format, and the pink color, defined by standard 1110011, is reserved for the transparent color.
*/
//z80_byte tbsprite_patterns[TBBLUE_MAX_PATTERNS][TBBLUE_SPRITE_SIZE];
z80_byte tbsprite_new_patterns[TBBLUE_SPRITE_ARRAY_PATTERN_SIZE];


/*
[0] 1st: X position (bits 7-0).
[1] 2nd: Y position (0-255).
[2] 3rd: bits 7-4 is palette offset, bit 3 is X MSB, bit 2 is X mirror, bit 1 is Y mirror and bit 0 is visible flag.
[3] 4th: bits 7-6 is reserved, bits 5-0 is Name (pattern index, 0-63).
[4] 5th: if 4bpp pattern, anchor, relative, etc
*/
z80_byte tbsprite_new_sprites[TBBLUE_MAX_SPRITES][TBBLUE_SPRITE_ATTRIBUTE_SIZE];

//Indica el ultimo sprite en la lista que esta visible, para solo renderizar hasta este
int tbsprite_last_visible_sprite=-1;

//Hay opcion de desactivar esto, pero por defecto vendra habilitado
z80_bit tbblue_disable_optimized_sprites={0};

//Indices al indicar paleta, pattern, sprites. Subindex indica dentro de cada pattern o sprite a que posicion (0..3 en sprites o 0..255 en pattern ) apunta
//z80_byte tbsprite_index_palette;
z80_byte tbsprite_index_pattern,tbsprite_index_pattern_subindex;
z80_byte tbsprite_index_sprite,tbsprite_index_sprite_subindex;
z80_byte tbsprite_nr_index_sprite;

/*
Port 0x303B, if read, returns some information:

Bits 7-2: Reserved, must be 0.
Bit 1: max sprites per line flag.
Bit 0: Collision flag.
Port 0x303B, if written, defines the sprite slot to be configured by ports 0x55 and 0x57, and also initializes the address of the palette.

*/

z80_byte tbblue_port_303b;


//Asumimos 256 registros
z80_byte tbblue_registers[256];

//Ultimo registro seleccionado
z80_byte tbblue_last_register;



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


z80_byte tbblue_port_123b_second_byte;


//
//FIN Variables, memoria etc de estado de la máquina. Se suelen guardar/cargar en snapshot ZSF
//

//valor inicial para tbblue_port_123b en caso de fast boot mode
int tbblue_initial_123b_port=-1;

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


//Layers con el indice al color final en la paleta RGB9 (0..511)
z80_int tbblue_layer_ula[TBBLUE_LAYERS_PIXEL_WIDTH];
z80_int tbblue_layer_layer2[TBBLUE_LAYERS_PIXEL_WIDTH];
z80_int tbblue_layer_sprites[TBBLUE_LAYERS_PIXEL_WIDTH];


//Indice a la posicion de 16 bits a escribir
//z80_int tbblue_copper_index_write=0;


z80_byte tbblue_machine_id=8;

struct s_tbblue_machine_id_definition tbblue_machine_id_list[]=
{
	{1,               "DE-1"},
	{2,               "DE-2"},
	{5,               "FBLabs"},
	{6,               "VTrucco"},
	{7,               "WXEDA"},
	{8,               "Emulators"},
	{10,              "ZX Spectrum Next"},
	{11,              "Multicore"},
	{0xea,            "ZX DOS"},
	{250,             "ZX Spectrum Next Antibrick"},

 	{255,""}
};


z80_byte tbblue_core_current_version_major=TBBLUE_CORE_DEFAULT_VERSION_MAJOR;
z80_byte tbblue_core_current_version_minor=TBBLUE_CORE_DEFAULT_VERSION_MINOR;
z80_byte tbblue_core_current_version_subminor=TBBLUE_CORE_DEFAULT_VERSION_SUBMINOR;

////return (TBBLUE_CORE_VERSION_MAJOR<<4 | TBBLUE_CORE_VERSION_MINOR);

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
	tbblue_registers[97]=posicion&0xFF;

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

//Escribe dato copper en posicion de escritura, 16 bits
void tbblue_copper_write_data_16b(z80_byte value1, z80_byte value2)
{
	z80_int posicion=tbblue_copper_get_write_position();

	posicion &=(TBBLUE_COPPER_MEMORY-1);

	//After a write to an odd address, the entire 16-bits are written to Copper memory at once.
	if (posicion&1) {	
		tbblue_copper_memory[posicion-1]=value1;
		tbblue_copper_memory[posicion]=value2;
		//printf ("Writing copper 16b data index %d data %02X%02XH\n",posicion-1,value1,value2);
	}

	posicion++;
	tbblue_copper_set_write_position(posicion);

}

//Devuelve el byte donde apunta indice
z80_byte tbblue_copper_get_byte(z80_int posicion)
{
	posicion &=(TBBLUE_COPPER_MEMORY-1);
	return tbblue_copper_memory[posicion];
}

//Devuelve el valor de copper
z80_int tbblue_copper_get_pc(void)
{
	return tbblue_copper_pc & (TBBLUE_COPPER_MEMORY-1);
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


//z80_bit tbblue_copper_ejecutando_halt={0};

//Ejecuta opcodes del copper // hasta que se encuentra un wait
void tbblue_copper_run_opcodes(void)
{

	z80_byte byte_leido=tbblue_copper_get_byte_pc();
	z80_byte byte_leido2=tbblue_copper_get_byte(tbblue_copper_pc+1);

	//Asumimos que no
	//tbblue_copper_ejecutando_halt.v=0;

	//if (tbblue_copper_get_pc()==0x24) printf ("%02XH %02XH\n",byte_leido,byte_leido2);

    //Special case of "value 0 to port 0" works as "no operation" (duration 1 CLOCK)
	/*
	Dado que el registro 0 es de solo lectura, no pasa nada si escribe en el: al leerlo se obtiene un valor calculado y no el del array
    if (byte_leido==0 && byte_leido2==0) {
      //printf("NOOP at %04XH\n",tbblue_copper_pc);
	  tbblue_copper_next_opcode();
      return;
    }
	*/

    //Special case of "WAIT 63,511" works as "halt" instruction
	/*
    if (byte_leido==255 && byte_leido2==255) {
	  //printf("HALT at %04XH\n",tbblue_copper_pc);
	  tbblue_copper_ejecutando_halt.v=1;

      return;
    }
	*/

    if ( (byte_leido&128)==0) {
        //Es un move
        z80_byte indice_registro=byte_leido&127;
        //tbblue_copper_pc++;
        
        //tbblue_copper_pc++;
        //printf ("Executing MOVE register %02XH value %02XH\n",indice_registro,valor_registro);
        tbblue_set_value_port_position(indice_registro,byte_leido2);

        tbblue_copper_next_opcode();

    }
    else {
        //Es un wait
        //Si se cumple, saltar siguiente posicion
        //z80_int linea, horiz;
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
int tbblue_copper_wait_cond_fired(void)
{
	//int scanline_actual=t_scanline;


	int current_horizontal=tbblue_get_current_raster_horiz_position();

	//Obtener parametros de instruccion wait
	z80_int linea, horiz;
	tbblue_copper_get_wait_opcode_parameters(&linea,&horiz);

	int current_raster=tbblue_get_current_raster_position();

	//511, 63
	//if (tbblue_copper_get_pc()==0x24) 
	// printf ("Waiting until raster %d horiz %d. current %d on copper_pc=%04X\n",linea,horiz,current_raster,tbblue_copper_get_pc() );

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


/*
void tbblue_if_copper_halt(void)
{
	//Si esta activo copper
    z80_byte copper_control_bits=tbblue_copper_get_control_bits();
    if (copper_control_bits != TBBLUE_RCCH_COPPER_STOP) {
        //printf ("running copper %d\n",tbblue_copper_pc);
		if (tbblue_copper_ejecutando_halt.v) {
			//liberar el halt
			//printf ("copper was on halt (copper_pc=%04XH). Go to next opcode\n",tbblue_copper_get_pc() );
			tbblue_copper_next_opcode();
			//printf ("copper was on halt (copper_pc after=%04XH)\n",tbblue_copper_get_pc() );
		}
	}	
}
*/					
 

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

int tbblue_get_current_raster_position(void)
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



/* 
Clip window registers

(R/W) 0x18 (24) => Clip Window Layer 2
  bits 7-0 = Coords of the clip window
  1st write - X1 position
  2nd write - X2 position
  3rd write - Y1 position
  4rd write - Y2 position
  Reads do not advance the clip position
  The values are 0,255,0,191 after a Reset

(R/W) 0x19 (25) => Clip Window Sprites
  bits 7-0 = Cood. of the clip window
  1st write - X1 position
  2nd write - X2 position
  3rd write - Y1 position
  4rd write - Y2 position
  The values are 0,255,0,191 after a Reset
  Reads do not advance the clip position
  When the clip window is enabled for sprites in "over border" mode,
  the X coords are internally doubled and the clip window origin is
  moved to the sprite origin inside the border.

(R/W) 0x1A (26) => Clip Window ULA/LoRes
  bits 7-0 = Coord. of the clip window
  1st write = X1 position
  2nd write = X2 position
  3rd write = Y1 position
  4rd write = Y2 position
  The values are 0,255,0,191 after a Reset
  Reads do not advance the clip position

(R/W) 0x1B (27) => Clip Window Tilemap
  bits 7-0 = Coord. of the clip window
  1st write = X1 position
  2nd write = X2 position
  3rd write = Y1 position
  4rd write = Y2 position
  The values are 0,159,0,255 after a Reset, Reads do not advance the clip position, The X coords are internally doubled (in 40x32 mode, quadrupled in 80x32)

0x1C (28) => Clip Window control
(R) (may change)
  bits 7:6 = Tilemap clip index
  bits 5:4 = ULA/Lores clip index
  bits 3:2 = Sprite clip index
  bits 1:0 = Layer 2 clip index
(W) (may change)
  bits 7:4 = Reserved, must be 0
  bit 3 = Reset the tilemap clip index
  bit 2 = Reset the ULA/LoRes clip index
  bit 1 = Reset the sprite clip index
  bit 0 = Reset the Layer 2 clip index
*/

z80_byte clip_windows[4][4];                    // memory array to store actual clip windows

void tbblue_inc_clip_window_index(const z80_byte index_mask) {
    const z80_byte inc_one = (index_mask<<1) ^ index_mask;   // extract bottom bit of mask (+garbage in upper bits)
    const z80_byte inc_index = (tbblue_registers[28] + inc_one) & index_mask;
    tbblue_registers[28] &= ~index_mask;        // clear old index value
    tbblue_registers[28] |= inc_index;          // set new index value
}

// shifts and masks how the clip-window index is stored in tbblue_registers[28]
#define TBBLUE_CLIP_WINDOW_LAYER2_INDEX_SHIFT   0
#define TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK    (3<<TBBLUE_CLIP_WINDOW_LAYER2_INDEX_SHIFT)

#define TBBLUE_CLIP_WINDOW_SPRITES_INDEX_SHIFT  2
#define TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK   (3<<TBBLUE_CLIP_WINDOW_SPRITES_INDEX_SHIFT)

#define TBBLUE_CLIP_WINDOW_ULA_INDEX_SHIFT      4
#define TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK       (3<<TBBLUE_CLIP_WINDOW_ULA_INDEX_SHIFT)

#define TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_SHIFT  6
#define TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK   (3<<TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_SHIFT)

z80_byte tbblue_get_clip_window_layer2_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_LAYER2_INDEX_SHIFT;
}

z80_byte tbblue_get_clip_window_sprites_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_SPRITES_INDEX_SHIFT;
}

z80_byte tbblue_get_clip_window_ula_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_ULA_INDEX_SHIFT;
}

z80_byte tbblue_get_clip_window_tilemap_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_SHIFT;
}

void tbblue_inc_clip_window_layer2_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK);
}

void tbblue_reset_clip_window_layer2_index(void) {
    tbblue_registers[28] &= ~TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK;
}

void tbblue_inc_clip_window_sprites_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK);
}

void tbblue_reset_clip_window_sprites_index(void) {
    tbblue_registers[28] &= ~(TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK);
}

void tbblue_inc_clip_window_ula_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK);
}

void tbblue_reset_clip_window_ula_index(void) {
    tbblue_registers[28] &= ~(TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK);
}

void tbblue_inc_clip_window_tilemap_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK);
}

void tbblue_reset_clip_window_tilemap_index(void) {
    tbblue_registers[28] &= ~(TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK);
}

//Forzar desde menu a desactivar capas 
z80_bit tbblue_force_disable_layer_ula={0};
z80_bit tbblue_force_disable_layer_tilemap={0};
z80_bit tbblue_force_disable_layer_sprites={0};
z80_bit tbblue_force_disable_layer_layer_two={0};

z80_bit tbblue_allow_layer2_priority_bit={1};

//Forzar a desactivar cooper
z80_bit tbblue_force_disable_cooper={0};

//Damos la paleta que se esta leyendo o escribiendo en una operacion de I/O
//Para ello mirar bits 6-4  de reg 0x43
z80_int *tbblue_get_palette_rw(void)
{
/*
(R/W) 0x43 (67) => Palette Control
  bit 7 = '1' to disable palette write auto-increment.
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

		case 3:
			return tbblue_palette_tilemap_first;
		break;		

		case 7:
			return tbblue_palette_tilemap_second;
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


//Damos el valor del color de la paleta que se esta mostrando en pantalla para tiles
z80_int tbblue_get_palette_active_tilemap(z80_byte index)
{
/*
Bit	Function
7	1 to enable the tilemap
6	0 for 40x32, 1 for 80x32
5	1 to eliminate the attribute entry in the tilemap
4	palette select (0 = first Tilemap palette, 1 = second)
3	(core 3.0) enable "text mode"
2	Reserved, must be 0
1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector
*/
	if (tbblue_registers[0x6B] & 16) return tbblue_palette_tilemap_second[index];
    else return tbblue_palette_tilemap_first[index];

}

//XXX indica desplazamiento de 1 pattern de 4bits
int tbsprite_pattern_get_offset_index_4bpp(z80_byte sprite,z80_byte index_in_sprite)
{
	//return sprite*TBBLUE_SPRITE_8BPP_SIZE+index_in_sprite;
    return sprite*TBBLUE_SPRITE_4BPP_SIZE+index_in_sprite;
}

z80_byte tbsprite_pattern_get_value_index_4bpp(z80_byte sprite,z80_byte index_in_sprite)
{
	return tbsprite_new_patterns[tbsprite_pattern_get_offset_index_4bpp(sprite,index_in_sprite)];
}



int tbsprite_pattern_get_offset_index_8bpp(z80_byte sprite,z80_byte index_in_sprite)
{
	return sprite*TBBLUE_SPRITE_8BPP_SIZE+index_in_sprite;
}

z80_byte tbsprite_pattern_get_value_index_8bpp(z80_byte sprite,z80_byte index_in_sprite)
{
	return tbsprite_new_patterns[tbsprite_pattern_get_offset_index_8bpp(sprite,index_in_sprite)];
}

void tbsprite_pattern_put_value_index_8bpp(z80_byte sprite,z80_byte index_in_sprite,z80_byte value)
{
	tbsprite_new_patterns[tbsprite_pattern_get_offset_index_8bpp(sprite,index_in_sprite)]=value;
}




int tbsprite_is_lockstep()
{
	return (tbblue_registers[9]&0x10);
}


void tbsprite_increment_index_303b() {	
	// increment the "port" index
	tbsprite_index_sprite_subindex=0;
	tbsprite_index_sprite++;
	tbsprite_index_sprite %= TBBLUE_MAX_SPRITES;
}




int tbblue_write_on_layer2(void)
{
	if (tbblue_port_123b &1) return 1;


	return 0;
}

int tbblue_read_on_layer2(void)
{
	if (tbblue_port_123b & 4) return 1;


	return 0;
}

//Retorna si hay mapeados 16kb o 48kb en espacio de memoria
int tbblue_layer2_size_mapped(void)
{
    z80_byte region=tbblue_port_123b&(64+128);

    if (region==128+64) {
        //printf("mapeados 48kb\n");
        return 49152;
    }
    else return 16384;
}

int tbblue_is_active_layer2(void)
{
	if (tbblue_port_123b & 2) return 1;

	return 0;
}

int tbblue_get_offset_start_layer2_reg(z80_byte register_value)
{
	//since core3.0 the NextRegs 0x12 and 0x13 are 7bit.
	int offset=register_value&127;
	//due to 7bit the value can leak outside of 2MiB
	// in HW the reads outside of SRAM module are "unspecified result", writes are ignored (!)	

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

int tbblue_get_offset_start_layer2(void)
{


    if (tbblue_port_123b & 8 ) return tbblue_get_offset_start_layer2_reg(tbblue_registers[19]);
    else return tbblue_get_offset_start_layer2_reg(tbblue_registers[18]);


}

int tbblue_get_offset_start_tilemap(void)
{
	return tbblue_registers[110]&63;
}


int tbblue_get_offset_start_tiledef(void)
{
	return tbblue_registers[111]&63;
}

int tbblue_get_tilemap_width(void)
{
	if (tbblue_registers[107]&64) return 80;
	else return 40;
}

int tbblue_if_ula_is_enabled(void)
{
	/*
(R/W) 0x68 (104) => ULA Control
  bit 7    = 1 to disable ULA output
  bit 6    = 0 to select the ULA colour for blending in SLU modes 6 & 7
           = 1 to select the ULA/tilemap mix for blending in SLU modes 6 & 7
  bits 5-1 = Reserved must be 0
  bit 0    = 1 to enable stencil mode when both the ULA and tilemap are enabled
            (if either are transparent the result is transparent otherwise the
             result is a logical AND of both colours)
						 */

	if (tbblue_registers[104]&128) return 0;
	else return 1;
}

void tbblue_reset_sprites(void)
{

	int i;

	

	//Resetear patterns todos a transparente
	for (i=0;i<TBBLUE_MAX_PATTERNS;i++) {
		int j;
		for (j=0;j<256;j++) {
			//tbsprite_patterns[i][j]=TBBLUE_DEFAULT_TRANSPARENT;
			tbsprite_pattern_put_value_index_8bpp(i,j,TBBLUE_DEFAULT_TRANSPARENT);
		}
	}

	//Poner toda info de sprites a 0. Seria quiza suficiente con poner bit de visible a 0
	for (i=0;i<TBBLUE_MAX_SPRITES;i++) {
		int j;
		for (j=0;j<TBBLUE_SPRITE_ATTRIBUTE_SIZE;j++) {
			tbsprite_new_sprites[i][j]=0;
		}
	}

    tbsprite_last_visible_sprite=-1;


	//tbsprite_index_palette=tbsprite_index_pattern=tbsprite_index_sprite=0;
	tbsprite_index_pattern=tbsprite_index_pattern_subindex=0;
	tbsprite_index_sprite=tbsprite_index_sprite_subindex=0;
	tbsprite_nr_index_sprite=0;	

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
			int colorpaleta=tbblue_default_ula_colours[i];


	//bright magenta son colores transparentes por defecto (1C7H y 1C6H  / 2 = E3H)
	//lo cambio a 1CF, que es un color FF24FFH, que no es magenta puro, pero evita el problema de transparente por defecto
	//esto lo corrige igualmente nextos al arrancar, pero si arrancamos tbblue en modo fast-boot, pasaria que los bright
	//magenta se verian transparentes
			if (i==11) colorpaleta=0x1CF;


			tbblue_palette_ula_first[j*16+i]=colorpaleta;
			tbblue_palette_ula_second[j*16+i]=colorpaleta;

		}
	}



}


/*void tbblue_out_port_sprite_index(z80_byte value)
{
	//printf ("Out tbblue_out_port_sprite_index %02XH\n",value);
	tbsprite_index_palette=tbsprite_index_pattern=tbsprite_index_sprite=value;

	tbsprite_index_pattern_subindex=tbsprite_index_sprite_subindex=0;
}*/


void tbblue_out_port_sprite_index(z80_byte value)
{
	/*
Port 0x303B (W)
X S S S S S S S
N6 X N N N N N N
A write to this port has two effects.
One is it selects one of 128 sprites for writing sprite attributes via port 0x57.
The other is it selects one of 128 4-bit patterns in pattern memory for writing 
sprite patterns via port 0x5B. The N6 bit shown is the least significant in the 7-bit 
pattern number and should always be zero when selecting one of 64 8-bit patterns indicated by N.	
	*/
	//printf ("Out tbblue_out_port_sprite_index %02XH\n",value);
	tbsprite_index_pattern=value % TBBLUE_MAX_PATTERNS;

	//De esta manera permitimos escrituras de 4bpp patterns en "medio" de un pattern de 8bpp (256 / 2 = 128 = 0x80)
	//casualmente (o no) nuestro incremento es de 128 y ese bit N6 está en la posicion de bit 7 = 128
	tbsprite_index_pattern_subindex=value & 0x80;

	tbsprite_index_sprite=value % TBBLUE_MAX_SPRITES;
	tbsprite_index_sprite_subindex=0;
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
//(R/W) 0x43 (67) => Palette Control
//  bit 7 = '1' to disable palette write auto-increment.

	if ((tbblue_registers[0x43] & 128)==0) {
		z80_byte indice=tbblue_registers[0x40];
		indice++;
		tbblue_registers[0x40]=indice;
	}

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

(R/W) 0x43 (67) => Palette Control
  bit 7 = '1' to disable palette write auto-increment.


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

//printf ("Escribir paleta\n");


}

#define TBBLUE_LAYER2_PRIORITY 0x8000

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

If writing the Layer 2 palette colour, in the second byte, bit 7 is "priority" bit. 
Priority colour will be always on top (drawn above all other layers), even on a priority arrangement like "USL" . 
If you need the exact same colour with priority and non priority, you will need to program the same colour twice, 
changing bit 7 to 0 for the non priority colour alternative.

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

    if ((valor&128) && (tbblue_registers[0x43]&0x30) == 0x10) {
        // layer 2 palette has extra priority bit in color (must be removed while mixing layers)
        color_actual |= TBBLUE_LAYER2_PRIORITY;
    }    

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

/*
Once a pattern number is selected via port 0x303B, the 256-byte or 128-byte pattern can be written to this port. 
The internal pattern pointer auto-increments after each write so as many sequential patterns as desired can be written. 
The internal pattern pointer will roll over from pattern 127 to pattern 0 (4-bit patterns) or from pattern 63 to pattern 0 
(8-bit patterns) automatically.
->esto del rollover es automático. Siempre resetea a 64. La diferencia es que podemos escribir “en medio” de un pattern 
de 256 (8 bit) cuando N6 es 1 (lo que indicaría un pattern de 128 - 4 bit). 
Así también, si N6 es 0 puede ser pattern de 4 bits, aunque da igual. N6 lo tratamos siempre como “sumar 1/2 pattern”
*/


	tbsprite_pattern_put_value_index_8bpp(tbsprite_index_pattern,tbsprite_index_pattern_subindex,value);
	//tbsprite_patterns[tbsprite_index_pattern][tbsprite_index_pattern_subindex]=value;



	if (tbsprite_index_pattern_subindex==255) {
		tbsprite_index_pattern_subindex=0;
		tbsprite_index_pattern++;
		if (tbsprite_index_pattern>=TBBLUE_MAX_PATTERNS) tbsprite_index_pattern=0;
	}
	else tbsprite_index_pattern_subindex++;

}

/*void tbblue_out_sprite_sprite(z80_byte value)
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
}*/

//Para escribir en el array de sprites
void tbblue_write_sprite_value(z80_byte indice,z80_byte subindice,z80_byte value)
{

    if (indice>=TBBLUE_MAX_SPRITES) {
        //Esto no deberia pasar pero... por si acaso
        return;
    }

    tbsprite_new_sprites[indice][subindice]=value;

    //tbsprite_last_visible_sprite
    if (subindice==3 && (value&128)) {
        //registro 3 e indica que sprite esta visible
        if (indice>tbsprite_last_visible_sprite) {
            tbsprite_last_visible_sprite=indice;
            //printf("last visible sprite: %d\n",tbsprite_last_visible_sprite);

            //TODO: logicamente si se escribe un sprite que no está visible, habria que recorrer todo el array para ver el ultimo,
            //y actualizarlo tambien, pero creo que no vale la pena, de momento solo ponemos el indice del mayor sprite que se 
            //ha escrito que esta visible, y listo
        }

    }
}

void tbblue_out_sprite_sprite(z80_byte value)
{
	//printf ("Out tbblue_out_sprite_sprite. Index: %d subindex: %d %02XH\n",tbsprite_index_sprite,tbsprite_index_sprite_subindex,value);

/*
Once a sprite is selected via port 0x303B, 
its attributes can be written to this port one byte after another. 
Sprites can have either four or five attribute bytes and the internal attribute pointer 
will move onto the next sprite after those four or five attribute bytes are written. 
This means you can select a sprite via port 0x303B and write attributes for as many sequential sprites as desired. 
The attribute pointer will roll over from sprite 127 to sprite 0.
*/


	//Indices al indicar paleta, pattern, sprites. Subindex indica dentro de cada pattern o sprite a que posicion 
	//(0..3/4 en sprites o 0..255 en pattern ) apunta
	//z80_byte tbsprite_index_sprite,tbsprite_index_sprite_subindex;

	//tbsprite_sprites[tbsprite_index_sprite][tbsprite_index_sprite_subindex]=value;
    tbblue_write_sprite_value(tbsprite_index_sprite,tbsprite_index_sprite_subindex,value);

	if (tbsprite_index_sprite_subindex == 3 && (value & 0x40) == 0) {			
		// 4-byte type, add 0 as fifth
		tbsprite_index_sprite_subindex++;
		//tbsprite_sprites[tbsprite_index_sprite][tbsprite_index_sprite_subindex]=0;
        tbblue_write_sprite_value(tbsprite_index_sprite,tbsprite_index_sprite_subindex,0);
	}

	tbsprite_index_sprite_subindex++;

	// Fin de ese sprite
	if (tbsprite_index_sprite_subindex == TBBLUE_SPRITE_ATTRIBUTE_SIZE) {
		tbsprite_increment_index_303b();
	}
}


//Guarda scanline actual y el pattern (los indices a colores) sobre la paleta activa de sprites
//z80_byte sprite_line[MAX_X_SPRITE_LINE];

#define TBBLUE_SPRITE_TRANS_FICT 65535

//Dice si un color de la capa de sprites es igual al color transparente ficticio inicial
int tbblue_si_sprite_transp_ficticio(z80_int color)
{
    if (color==TBBLUE_SPRITE_TRANS_FICT) return 1;
    return 0;
}

//Dice si ese color es de layer2 con priority bit
int tbblue_color_is_layer2_priority(z80_int color)
{
    if (!tbblue_si_sprite_transp_ficticio(color) && (color&TBBLUE_LAYER2_PRIORITY) ) return 1;
    else return 0;
}


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
	int clipxmin=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0]+TBBLUE_SPRITE_BORDER;
	int clipxmax=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1]+TBBLUE_SPRITE_BORDER;
	z80_byte sprites_over_border=tbblue_registers[21]&2;
	if (sprites_over_border==0 && (x<clipxmin || x>clipxmax)) return;

	//Si index de color es transparente, no hacer nada
/*
The sprites have now a new register for sprite transparency. Unlike the Global Transparency Colour register this refers to an index and  should be set when using indices other than 0xE3:

(R/W) 0x4B (75) => Transparency index for Sprites
bits 7-0 = Set the index value. (0XE3 after a reset)
	*/
	//if (color==tbblue_registers[75]) return;

	z80_int color_final=tbblue_get_palette_active_sprite(color);


	int xfinal=x;

	xfinal +=screen_total_borde_izquierdo*border_enabled.v;
	xfinal -=TBBLUE_SPRITE_BORDER;

	xfinal *=2; //doble de ancho


	//Ver si habia un color y activar bit colision
	z80_int color_antes=tbblue_layer_sprites[xfinal];

	//if (!tbblue_si_transparent(color_antes)) {
	if (!tbblue_si_sprite_transp_ficticio(color_antes) ) {
		//colision
		tbblue_port_303b |=1;
		//printf ("set colision flag. result value: %d\n",tbblue_port_303b);

        //Si invertir prioridad sprites
        // NextReg 0x15 bit6 = rendering priority: 1 = Sprite 0 on top, 0 = Sprite 0 at bottom

        if (tbblue_registers[21] & 64) return;  // keep pixel of previous sprite rendered
	}
	

	//sprite_line[x]=color;
	tbblue_layer_sprites[xfinal]=color_final;
	tbblue_layer_sprites[xfinal+1]=color_final; //doble de ancho

	//if (xfinal<0 || xfinal>TBBLUE_LAYERS_PIXEL_WIDTH) printf ("out of range x sprites: %d\n",xfinal);

}

z80_byte tbsprite_do_overlay_get_pattern_xy_8bpp(z80_byte index_pattern,z80_byte sx,z80_byte sy)
{

	//return tbsprite_patterns[index_pattern][sy*TBBLUE_SPRITE_WIDTH+sx];
	return tbsprite_pattern_get_value_index_8bpp(index_pattern,sy*TBBLUE_SPRITE_WIDTH+sx);
}

z80_byte tbsprite_do_overlay_get_pattern_xy_4bpp(z80_byte index_pattern,z80_byte sx,z80_byte sy)
{
	z80_byte valor=tbsprite_pattern_get_value_index_4bpp(index_pattern, (sy*TBBLUE_SPRITE_WIDTH+sx)/2  );

	if ( (sx % 2) == 0) valor=valor>>4;

	valor &= 0xF;

	return valor;

}


z80_int tbsprite_return_color_index(z80_byte index)
{
	//z80_int color_final=tbsprite_palette[index];

	z80_int color_final=tbblue_get_palette_active_sprite(index);
	//return RGB9_INDEX_FIRST_COLOR+color_final;
	return color_final;
}

int tbblue_if_sprites_enabled(void) 
{

	return tbblue_registers[21]&1;

}


int tbblue_if_tilemap_enabled(void) 
{

	return tbblue_registers[107]&128;

}

void tbsprite_do_overlay(void)
{


    if (!tbblue_if_sprites_enabled() ) return;

    //printf ("tbblue sprite chip activo\n");


    //int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;
    int y=t_scanline_draw; //0..63 es border (8 no visibles)

    int border_no_visible=screen_indice_inicio_pant-TBBLUE_SPRITE_BORDER;

    y -=border_no_visible;

    //Ejemplo: scanline_draw=32 (justo donde se ve sprites). border_no_visible=64-32 =32
    //y=y-32 -> y=0


    //Situamos el 0 32 pixeles por encima de dentro de pantalla, tal cual como funcionan las cordenadas de sprite de tbblue


    //Calculos exclusivos para puntero buffer rainbow
    int rainbowy=t_scanline_draw-screen_invisible_borde_superior;
    if (border_enabled.v==0) rainbowy=rainbowy-screen_borde_superior;
        

    //Aqui tenemos el y=0 arriba del todo del border

    //Bucle para cada sprite
    int conta_sprites;
    z80_byte index_pattern;

    int i;
    //int offset_pattern;

    z80_byte sprites_over_border=tbblue_registers[21]&2;


    int rangoxmin, rangoxmax;
    int rangoymin, rangoymax;

    if (sprites_over_border) {
        rangoxmin=0;
        rangoxmax=TBBLUE_SPRITE_BORDER+256+TBBLUE_SPRITE_BORDER-1;
        rangoymin=0;
        rangoymax=TBBLUE_SPRITE_BORDER+192+TBBLUE_SPRITE_BORDER-1;			

        if (tbblue_registers[21]&0x20) {
                // sprite clipping "over border" enabled, double the X coordinate of clip window
                rangoxmin=2*clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0];
                rangoxmax=2*clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1]+1;
                rangoymin=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2];
                rangoymax=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3];
                if (rangoxmax > TBBLUE_SPRITE_BORDER+256+TBBLUE_SPRITE_BORDER-1) {
                    // clamp rangoxmax to 319
                    rangoxmax = TBBLUE_SPRITE_BORDER+256+TBBLUE_SPRITE_BORDER-1;
                }
        }
    }

    else {
        // take clip window coordinates, but limit them to [0,0]->[255,191] (and offset them +32,+32)
        rangoxmin=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0] + TBBLUE_SPRITE_BORDER;
        rangoxmax=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1] + TBBLUE_SPRITE_BORDER;
        rangoymin=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2] + TBBLUE_SPRITE_BORDER;
        rangoymax=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3] + TBBLUE_SPRITE_BORDER;
        if (rangoymax > TBBLUE_SPRITE_BORDER+192-1) {
            // clamp rangoymax to 32+191 (bottom edge of PAPER)
            rangoymax = TBBLUE_SPRITE_BORDER+192-1;
        }
    }

    if (y<rangoymin || y>rangoymax) return;


    int total_sprites=0;

    int sprite_visible;

    int anchor_x;
    int anchor_y;
    z80_byte anchor_palette_offset;
    z80_byte anchor_index_pattern;
    int anchor_visible;
    int anchor_sprite_es_4bpp;

    anchor_x=anchor_y=anchor_palette_offset=anchor_index_pattern=anchor_visible=anchor_sprite_es_4bpp=0;

    int anchor_mirror_x=0;
    int anchor_mirror_y=0;
    int anchor_rotate=0;
    int anchor_sprite_zoom_x=0;
    int anchor_sprite_zoom_y=0;

    //int sprite_has_5_bytes;


    //int relative_sprite=0;

    //Esto se conserva el valor anterior del anterior anchor
    //int sprite_es_relative_composite=0; 
    int sprite_es_relative_unified=0;        

    int ultimo_sprite=TBBLUE_MAX_SPRITES-1;


    //Si optimizamos, que es lo habitual
    /*
    Con Delta shadow, me ahorro un 1% de mi CPU optimizando esto
    */
    if (tbblue_disable_optimized_sprites.v==0) ultimo_sprite=tbsprite_last_visible_sprite;

    //printf("renderizando hasta %d\n",ultimo_sprite);
    

    //for (conta_sprites=0;conta_sprites<TBBLUE_MAX_SPRITES && total_sprites<MAX_SPRITES_PER_LINE;conta_sprites++) {

    //recorremos array no a todos los sprites sino hasta el ultimo visible o bien hasta el maximo de lineas
    for (conta_sprites=0;conta_sprites<=ultimo_sprite && total_sprites<MAX_SPRITES_PER_LINE;conta_sprites++) {
        int sprite_x;
        int sprite_y;

        z80_byte transparency_colour=tbblue_registers[75];

        //Esto se lee de cada sprite
        int relative_sprite=0;

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
        

        z80_byte spr_attr_0=tbsprite_new_sprites[conta_sprites][0];
        z80_byte spr_attr_1=tbsprite_new_sprites[conta_sprites][1];
        z80_byte spr_attr_2=tbsprite_new_sprites[conta_sprites][2];
        z80_byte spr_attr_3=tbsprite_new_sprites[conta_sprites][3];
        z80_byte spr_attr_4=tbsprite_new_sprites[conta_sprites][4];

        sprite_visible=spr_attr_3 & 128;


        if ((spr_attr_3 & 64)==0) {
            //sprite es de 4 bytes. entonces sera como un anchor de modo composite
            //Sprite attribute 3 V E N5 N4 N3 N2 N1 N0
            //If E=0, the sprite is fully described by sprite attributes 0-3. 
            //The sprite pattern is an 8-bit one identified by pattern N=0-63. 
            //The sprite is an anchor and cannot be made relative. 
            
            //The sprite is displayed as if sprite attribute 4 is zero.
            spr_attr_4=0;                      
        }
        


            
        //Relative sprites
        //H N6 T X X Y Y Y8
        //{H,N6} must not equal {0,1} as this combination is used to indicate a relative sprite.
        //z80_byte spr_attr_4=tbsprite_sprites[conta_sprites][4];

        if ((spr_attr_4 & (128+64))==64) {

            relative_sprite=1;

                    

            //printf ("Relative sprite number %d\n",conta_sprites);
            /*
            The sprite module records the following information from the anchor:

            Anchor.visible
            Anchor.X
            Anchor.Y
            Anchor.palette_offset
            Anchor.N (pattern number)
            Anchor.H (indicates if the sprite uses 4-bit patterns)
            */

            //Relative sprites is visible if anchor and this sprite are both visibles
            //The visibility of a particular relative sprite is the result of ANDing the anchor’s visibility 
            //with the relative sprite’s visibility. In other words, if the anchor is invisible then so are all its relatives.
            if (sprite_visible && anchor_visible) sprite_visible=128; 
            //Realmente con 1 valdria pero lo hago para que coincida con el valor normal cuando es visible


            else sprite_visible=0;
            
            //sprite_visible=anchor_visible;

            //printf("visible: %d\n",sprite_visible);
        }

        else {
            relative_sprite=0;
            //No es relativo. Guardar la visibilidad del ultimo anchor
            anchor_visible=sprite_visible;

            //El anchor define el tipo de "relatividad" de los sprites asociados

            if (spr_attr_4 & 32) {
            
            //con la condicion al reves para que se arregla un poco 
            //if (!(spr_attr_4 & 32)) {                                
                sprite_es_relative_unified=1;
                //sprite_es_relative_composite=0;
                //printf("sprite unified en %d\n",conta_sprites);
            }
            else {
                sprite_es_relative_unified=0;
                //sprite_es_relative_composite=1;
            }                               
        }

        



        z80_byte mirror_x=spr_attr_2 & 8;
        //[2] 3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is rotate flag and bit 0 is X MSB.
        z80_byte mirror_y=spr_attr_2 & 4;			

        z80_byte sprite_rotate;

        sprite_rotate=spr_attr_2 & 2;                    			
        
        //if (sprite_visible) {

        sprite_x=spr_attr_0 | ((spr_attr_2 & 1)<<8);

        //printf ("sprite %d x: %d \n",conta_sprites,sprite_x);

        sprite_y=spr_attr_1;

        if (!relative_sprite) {
            //Sprite Attribute 4
            //A. Extended Anchor Sprite
            //H N6 T X X Y Y Y8
            //Y8 = Ninth bit of the sprite’s Y coordinate

            sprite_y |= ((spr_attr_4 & 1)<<8);
        }

        //Posicionamos esa y teniendo en cuenta que nosotros contamos 0 arriba del todo del border en cambio sprites aqui
        //Considera y=32 dentro de pantalla y y=0..31 en el border
        //sprite_y +=screen_borde_superior-32;

        //Si y==32-> y=32+48-32=32+16=48
        //Si y==0 -> y=48-32=16


        //3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is rotate flag and bit 0 is X MSB.
        //Offset paleta se lee tal cual sin rotar valor
        z80_byte palette_offset=spr_attr_2 & 0xF0;

        index_pattern=spr_attr_3 & 63;
        
        //Sprite Attribute 4
        //0 1 N6 X X Y Y PO
        //TODO: solo para relative composite sprite, no unified
        z80_byte sprite_zoom_x=(spr_attr_4 >> 3)&3;
        z80_byte sprite_zoom_y=(spr_attr_4 >> 1)&3;




        int sprite_es_4bpp=0;

        z80_byte mask_index_pattern=63;

            /*


        //En caso de anchor:
        //H N6 T X X Y Y Y8
        //H = 1 if the sprite pattern is 4-bit
        //N6 = 7th pattern bit if the sprite pattern is 4-bit
        //N6 es un nombre confuso, es el bit que dice si sumamos 128 o no
        Lo que hago para sprites de 4bpp es:
        multiplico *2 valor que teniamos de index pattern, de los bits N5 N4 N3 N2 N1 N0
        el bit llamado N6 sera el bit 0
        el valor resultante, multiplicado por 128 (sprites de 4bpp ocupan 128),
        
        sera el offset al sprite
        asi, el index pattern en sprites de 4bpp va de 0 a 127
        en el caso de 8bpp, va de 0 a 63
        */

        

        if (!relative_sprite) {

            if (spr_attr_4 & 128) sprite_es_4bpp=1;

            if (sprite_es_4bpp) {
            index_pattern *=2;
                if (spr_attr_4 & 64) {
                    //offset_4bpp_N6=1;
                    index_pattern +=1;
                }
            }

            anchor_sprite_es_4bpp=sprite_es_4bpp;
        }

        else {


            //En caso de relative sprites, el valor de H viene del anchor
            /*
            B. Relative Sprite, Composite Type
            0 1 N6 X X Y Y PO
            C. Relative Sprite, Unified Type
            0 1 N6 0 0 0 0 PO

            Ver que el bit N6 se desplaza respecto a cuando es un anchor
            */

            sprite_es_4bpp=anchor_sprite_es_4bpp;

            if (sprite_es_4bpp) {
                index_pattern *=2;
                if (spr_attr_4 & 32) {
                    //offset_4bpp_N6=1;
                    index_pattern +=1;
                }
            }

            
        }

            
        


        if (sprite_es_4bpp) {
            mask_index_pattern=127;
            transparency_colour &=0xF; //solo se coge los 4 bits inferiores del registro de indice de transparencia
        }



        //Si era sprite relativo, asignar valores del ultimo anchor
        if (relative_sprite) {
            //printf("Using the last anchor values\n");
            //Relative sprites only have 8-bit X and Y coordinates (the ninth bits are taken for other purposes)
            //These are signed offsets from the anchor’s X,Y coordinate

            sprite_x=spr_attr_0;
            if (sprite_x & 128) sprite_x=-(256-sprite_x);

            sprite_y=spr_attr_1; 
            if (sprite_y & 128) sprite_y=-(256-sprite_y);                       

                                    

            /*
            If the relative sprite has its PR bit set in sprite attribute 2, 
            then the anchor’s palette offset is added to the relative sprite’s to determine the active 
            palette offset for the relative sprite. Otherwise the relative sprite uses its own palette 
            offset as usual.

            If the relative sprite has its PO bit set in sprite attribute 4, then the anchor’s pattern 
            number is added to the relative sprite’s to determine the pattern used for display. Otherwise 
            the relative sprite uses its own pattern number as usual. The intention is to supply a method 
            to easily animate a large sprite by manipulating the pattern number in the anchor.
            */
            //P P P P XM YM R X8/PR
            if (spr_attr_2 &1) {
                palette_offset=(palette_offset+anchor_palette_offset)& 0xF0;
            }

            //0 1 N6 X X Y Y PO
            if (spr_attr_4&1) {
                index_pattern=(index_pattern+anchor_index_pattern)&mask_index_pattern;
            }

            /*
            These recorded items are not used by composite sprites:

            Anchor.rotate
            Anchor.xmirror
            Anchor.ymirror
            Anchor.xscale
            Anchor.yscale

            PUES... esto diria que en la wiki esta al reves
            Si no, los pies y brazos del deltashadow, cuando va a la izquierda, se ven mal
            */

            if (sprite_es_relative_unified) {

                sprite_zoom_x=anchor_sprite_zoom_x;
                sprite_zoom_y=anchor_sprite_zoom_y;

                //printf("unified sprite sprite %d\n",conta_sprites);
                /*
                The difference is the collection of anchor and relatives is treated as 
                if it were a single 16×16 sprite. The anchor’s rotation, mirror, and 
                scaling bits apply to all its relatives. Rotating the anchor causes all the relatives to 
                rotate around the anchor. Mirroring the anchor causes the relatives to mirror around the anchor.
                    The sprite hardware will automatically adjust X,Y coords and rotation, scaling and mirror bits of all 
                    relatives according to settings in the anchor.

                Unified sprites should be defined as if all its parts are 16×16 in size with the anchor controlling
                the look of the whole.

                A unified sprite is like a big version of an individual 16×16 sprite controlled by the anchor.                            
                */

                if (anchor_rotate) {
                    sprite_rotate = !sprite_rotate;
                    int old_x = sprite_x;
                    sprite_x = -sprite_y;
                    sprite_y = old_x;
                    z80_byte old_v = mirror_x;
                    mirror_x = sprite_rotate ? mirror_y : !mirror_y;
                    mirror_y = sprite_rotate ? old_v : !old_v;
                }                           

                if (anchor_mirror_x) {
                    mirror_x = !mirror_x;
                    sprite_x = -sprite_x;
                }     

                if (anchor_mirror_y) {
                    mirror_y = !mirror_y;
                    sprite_y = -sprite_y;
                }      

                sprite_x <<= sprite_zoom_x;
                sprite_y <<= sprite_zoom_y;                                                  

                                                            
            }     


            sprite_x=(sprite_x+anchor_x) & 0x1FF;
            sprite_y=(sprite_y+anchor_y) & 0x1FF;
            
    
        }

        else {
            //Guardamos estos valores como el ultimo anchor
            //if (sprite_x > 512-128) sprite_x -= 512;                // -127 .. +384 (cover 8x scaleX)
            //if (conta_sprites==9) printf("anchor sprite on %d\n",conta_sprites);
            //printf("anchor on %d\n",conta_sprites);

            anchor_x=sprite_x;
            anchor_y=sprite_y;
            anchor_palette_offset=palette_offset;
            anchor_index_pattern=index_pattern;
            anchor_mirror_x=mirror_x;
            anchor_mirror_y=mirror_y;    
            anchor_rotate=sprite_rotate;
            anchor_sprite_zoom_x=sprite_zoom_x;
            anchor_sprite_zoom_y=sprite_zoom_y;

                                
        }

        //if (mirror_x) printf("mirror on sprite %d\n",conta_sprites);

        //hasta aqui no lo miramos pues hay que leer variables de anchor si hay un sprite relativo

        //Alterar visibilidad de sprites segun ventana debug-tbblues-sprites-sprite disable
        sprite_visible *=debug_tbblue_sprite_visibility[conta_sprites];

    

        if (sprite_visible) {    

            //if (conta_sprites==0 || conta_sprites==1) {
            //    printf("ANTES %d index_pattern: %d\n",conta_sprites,index_pattern);
            //}                                

            //Si coordenada y esta en margen y sprite activo
            int diferencia=(y-sprite_y)>>sprite_zoom_y;


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



                //Dibujar linea x

                //Cambiar offset si mirror x, ubicarlo a la derecha del todo
                if (mirror_x) {
                    //offset_pattern=offset_pattern+TBBLUE_SPRITE_WIDTH-1;
                    sx=TBBLUE_SPRITE_WIDTH-1;
                    incx=-1;
                }

                //z80_byte sprite_rotate;

                //sprite_rotate=tbsprite_sprites[conta_sprites][2]&2;
                //if (sprite_rotate) printf("rotate on %d\n",conta_sprites);

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
                    z80_byte index_color;

                    if (sprite_es_4bpp) {
                        //printf("es 4bpp\n");
                        //index_color=tbsprite_do_overlay_get_pattern_xy_4bpp(index_pattern,offset_4bpp_N6,sx,sy);
                        //index_pattern +=TBBLUE_SPRITE_4BPP_SIZE*offset_4bpp_N6;


                        index_color=tbsprite_do_overlay_get_pattern_xy_4bpp(index_pattern,sx,sy);

                        //index_color +=7;
                    }
                    else {
                        //printf("es 8 bpp\n");
                        //printf("pattern %d\n",index_pattern);
                        index_color=tbsprite_do_overlay_get_pattern_xy_8bpp(index_pattern,sx,sy);

                        //index_color +=2;
                    }

                        //Si index de color es transparente, no hacer nada
/*
The sprites have now a new register for sprite transparency. Unlike the Global Transparency Colour register this refers to an index and  should be set when using indices other than 0xE3:

(R/W) 0x4B (75) => Transparency index for Sprites
bits 7-0 = Set the index value. (0XE3 after a reset)
*/

                    sx=sx+incx;
                    sy=sy+incy;

                    int sumar_x=1<<sprite_zoom_x;




                    if (index_color!=transparency_colour) {

                    //Sumar palette offset. Logicamente si es >256 el resultado, dará la vuelta el contador
                    index_color +=palette_offset;

                    //printf ("index color: %d\n",index_color);
                    //printf ("palette offset: %d\n",palette_offset);
                    

                    if (sprite_zoom_x==0) {
                        tbsprite_put_color_line(sprite_x,index_color,rangoxmin,rangoxmax);
                    }
                    else {
                        int zz=0;
                        for (zz=0;zz<sumar_x;zz++) {
                            tbsprite_put_color_line(sprite_x+zz,index_color,rangoxmin,rangoxmax);
                        }
                    }

                    }
                    sprite_x+=sumar_x;


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


}

z80_byte tbblue_get_port_layer2_value(void)
{
	return tbblue_port_123b;
}

void tbblue_out_port_layer2_value(z80_byte value)
{

    if ((value & 16)==0) {

	tbblue_port_123b=value;

	//Sincronizar bit layer2
		

    /*
    (W) 0x69 (105) => DISPLAY CONTROL 1 REGISTER

    Bit	Function
    7	Enable the Layer 2 (alias for Layer 2 Access Port ($123B) bit 1)
    6	Enable ULA shadow (bank 7) display (alias for Memory Paging Control ($7FFD) bit 3)
    5-0	alias for Timex Sinclair Video Mode Control ($xxFF) bits 5:0

    */
    tbblue_registers[105] &= (255-128);
    if (value&2) tbblue_registers[105]|=128;

	//printf ("valor a 123b: %02XH\n",value);
    }

    else {
        tbblue_port_123b_second_byte=value;
    }
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





void tbblue_init_memory_tables(void)
{
/*


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


Nuevo mapeo para multiface:

-- 0x000000 - 0x00FFFF (64K)  => ZX Spectrum ROM         A20:A16 = 00000
   -- 0x010000 - 0x011FFF ( 8K)  => divMMC ROM              A20:A16 = 00001,000
   -- 0x012000 - 0x013FFF ( 8K)  => unused                  A20:A16 = 00001,001
   -- 0x014000 - 0x017FFF (16K)  => Multiface ROM,RAM       A20:A16 = 00001,01
   -- 0x018000 - 0x01BFFF (16K)  => Alt ROM0 128k           A20:A16 = 00001,10
   -- 0x01c000 - 0x01FFFF (16K)  => Alt ROM1 48k            A20:A16 = 00001,11
   -- 0x020000 - 0x03FFFF (128K) => divMMC RAM              A20:A16 = 00010
   -- 0x040000 - 0x05FFFF (128K) => ZX Spectrum RAM         A20:A16 = 00100
   -- 0x060000 - 0x07FFFF (128K) => Extra RAM
   -- 0x080000 - 0x0FFFFF (512K) => 1st Extra IC RAM (if present)
   -- 0x100000 - 0x17FFFF (512K) => 2nd Extra IC RAM (if present)
   -- 0x180000 - 0x1FFFFF (512K) => 3rd Extra IC RAM (if present)



*/





	int i,indice;

	//Los 8 KB de la fpga ROM estan al final
	tbblue_fpga_rom=&memoria_spectrum[2*1024*1024];

	//224 Paginas RAM spectrum 512k
	for (i=0;i<TBBLUE_MAX_SRAM_8KB_BLOCKS;i++) {
		indice=0x040000+8192*i;
		tbblue_ram_memory_pages[i]=&memoria_spectrum[indice];
	}

	//4 Paginas ROM de 16kb (8 paginas de 8kb)
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

	//Guardar el valor tal cual, antes de ver si la pagina excede el limite
	debug_paginas_memoria_mapeadas[segment]=reg_value;		

	//tbblue_memory_paged[segment]=tbblue_ram_memory_pages[page];
	reg_value=tbblue_get_limit_sram_page(reg_value);
	tbblue_memory_paged[segment]=tbblue_ram_memory_pages[reg_value];

}


void tbblue_set_rom_page_no_255(z80_byte segment)
{
    z80_byte tbblue_register=80+segment;
    z80_byte reg_value=tbblue_registers[tbblue_register];

	//Guardar el valor tal cual, antes de ver si la pagina excede el limite
	debug_paginas_memoria_mapeadas[segment]=reg_value;			

	reg_value=tbblue_get_limit_sram_page(reg_value);
	tbblue_memory_paged[segment]=tbblue_ram_memory_pages[reg_value];

}

int tbblue_get_altrom(void)
{
/*
0x8C (140) => Alternate ROM 
(R/W) (hard reset = 0)
IMMEDIATE
bit 7 = 1 to enable alt rom
bit 6 = 1 to make alt rom visible only during writes, otherwise replaces rom during reads
bit 5 = 1 to lock ROM1 (48K rom)
bit 4 = 1 to lock ROM0 (128K rom)
AFTER SOFT RESET (copied into bits 7-4)
bit 3 = 1 to enable alt rom
bit 2 = 1 to make alt rom visible only during writes, otherwise replaces rom during reads
bit 1 = 1 to lock ROM1 (48K rom)
bit 0 = 1 to lock ROM0 (128K rom)
*/

/*
   -- 0x018000 - 0x01BFFF (16K)  => Alt ROM0 128k           A20:A16 = 00001,10
   -- 0x01c000 - 0x01FFFF (16K)  => Alt ROM1 48k            A20:A16 = 00001,11
   */

/*


   nr_8c_altrom_lock_rom1 <= nr_8c_altrom(5);
   nr_8c_altrom_lock_rom0 <= nr_8c_altrom(4);
   ....

   port_1ffd_rom <= port_1ffd_reg(2) & port_7ffd_reg(4);

   ....

      signal machine_type_config    : std_logic;
   signal machine_type_48        : std_logic;
   signal machine_type_128       : std_logic;
   signal machine_type_p3        : std_logic;


   machine_type_config <= '1' when nr_03_machine_type = "000" else '0';   -- 48k config
   machine_type_48 <= '1' when nr_03_machine_type(2 downto 1) = "00" else '0';   -- 48k
   machine_type_128 <= '1' when nr_03_machine_type = "010" or nr_03_machine_type = "100" else '0';  -- 128k or pentagon
   machine_type_p3 <= '1' when nr_03_machine_type = "011" else '0';   -- +3

(R/W) 0x03 (03) => Set machine type
...
bits 2-0 = Machine type (writable in config mode only):
000 = Config mode
001 = ZX 48K
010 = ZX 128K/+2 (Grey)
011 = ZX +2A-B/+3e/Next Native
100 = Pentagon 128K      

...
   process (machine_type_48, machine_type_p3, nr_8c_altrom_lock_rom1, nr_8c_altrom_lock_rom0, port_1ffd_rom)
   begin
      if machine_type_48 = '1' then
         sram_active_rom <= "00";
         sram_alt_128 <= (not nr_8c_altrom_lock_rom1) and nr_8c_altrom_lock_rom0;


      elsif machine_type_p3 = '1' then
         if nr_8c_altrom_lock_rom1 = '1' or nr_8c_altrom_lock_rom0 = '1' then
            sram_active_rom <= nr_8c_altrom_lock_rom1 & nr_8c_altrom_lock_rom0;
            sram_alt_128 <= not nr_8c_altrom_lock_rom1;
         else
            sram_active_rom <= port_1ffd_rom;
            sram_alt_128 <= not port_1ffd_rom(0);   -- behave like a 128k machine
         end if;


      else
         if nr_8c_altrom_lock_rom1 = '1' or nr_8c_altrom_lock_rom0 = '1' then
            sram_active_rom <= '0' & nr_8c_altrom_lock_rom1;
            sram_alt_128 <= not nr_8c_altrom_lock_rom1;
         else
            sram_active_rom <= '0' & port_1ffd_rom(0);
            sram_alt_128 <= not port_1ffd_rom(0);
         end if;
      end if;


   end process;   

   Acerca de sram_alt_128:
Allen Albright Oh yeah -- the next is beginning with a +3 port implementation but it can still 
behave as a 48k, 128k, pentagon and it does that by disabling port 0x1ffd when running that code. 
So the hardware still internally operates as a +3 but when behaving as a 48k, 128k, pentagon it's not 
possible to write to port 0x1ffd and the machine behaves like a 48k, 128k, pentagon.
This enabling or disabling of hardware is done by nextzxos (or the user) when running legacy 
programs through nextreg 0x85 - 0x82 and through nextreg 0x89 - 0x86. The latter "expansion bus decodes" 
only come into force when the expansion bus is enabled, in which case the internal port decode and 
the expansion bus decodes are ANDed together. For a 48k load, you'd turn off most things and change the 
video timing in nextreg 0x03 to generate the 48k video frame.   

-> O sea que quien lo entienda que me lo explique :(

*/

	int altrom;

	/*
bit 5 = 1 to lock ROM1 (48K rom)
bit 4 = 1 to lock ROM0 (128K rom)

altrom=0 -> ROM0
altrom=1 -> ROM01
*/

    if ( (tbblue_registers[0x8c] & 32) == 32) {
        //printf ("ROM1\n");
        altrom=1;
    }
    //128k rom
    else if ( (tbblue_registers[0x8c] & 16) == 16) {
        altrom=0;
        //printf ("ROM0\n");
    }

    //a 0 los dos . paginado +3.
    else {
        
        z80_byte rom_entra=((puerto_32765>>4)&1);
        altrom=rom_entra;	
        //printf ("alt rom segun 7ffd (%d)\n",altrom);
    }

	return altrom;
}


int tbblue_get_altrom_offset_dir(int altrom,z80_int dir)
{
	int offset;
	/*
	   -- 0x018000 - 0x01BFFF (16K)  => Alt ROM0 128k           A20:A16 = 00001,10
   -- 0x01c000 - 0x01FFFF (16K)  => Alt ROM1 48k            A20:A16 = 00001,11
   */

    if (altrom==1) {
        offset=0x01c000+dir;
    }
    else {
        offset=0x018000+dir;
    }

	return offset;
}

void tbblue_set_rom_page(z80_byte segment,z80_byte page)
{
	z80_byte tbblue_register=80+segment;
	z80_byte reg_value=tbblue_registers[tbblue_register];

	if (reg_value==255) {

		//Guardar el valor tal cual, antes de ver si la pagina excede el limite
		debug_paginas_memoria_mapeadas[segment]=DEBUG_PAGINA_MAP_ES_ROM+page;

		page=tbblue_get_limit_sram_page(page);

		//Si esta altrom en read
		//Altrom.
		//bit 6 =0 , only for read. bit 6=1, only for write
		if (  (tbblue_registers[0x8c] & 192) ==128)    {
			
			int altrom;

			//TODO: tener en cuenta altrom si maquina es distinta de machine_type_p3,
			//que es como en teoria lo estoy haciendo. Ver codigo vhdl para salir de dudas
			altrom=tbblue_get_altrom();

			//printf ("Enabling alt rom on read. altrom=%d\n",altrom);


			int offset=tbblue_get_altrom_offset_dir(altrom,8192*segment);

			tbblue_memory_paged[segment]=&memoria_spectrum[offset];

		}

		else tbblue_memory_paged[segment]=tbblue_rom_memory_pages[page];


		
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
			//debug_printf (VERBOSE_DEBUG,"Pages 0,1,2,3");
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
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);

			contend_pages_actual[0]=contend_pages_128k_p2a[0];
			contend_pages_actual[1]=contend_pages_128k_p2a[1];
			contend_pages_actual[2]=contend_pages_128k_p2a[2];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];



			break;

		case 1:
			//debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");

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
			//debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,3");

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
			//debug_printf (VERBOSE_DEBUG,"Pages 4,7,6,3");

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

/*void tbblue_set_emulator_setting_multiface(void)
{
	
	//(R/W) 0x06 (06) => Peripheral 2 setting:
  //bit 3 = Enable Multiface (1 = enabled)(0 after a PoR or Hard-reset)
	

	//de momento nada
	//return;

	multiface_type=MULTIFACE_TYPE_THREE; //Vamos a suponer este tipo
	z80_byte multisetting=tbblue_registers[6]&8;

	if (multisetting) {
		//printf ("Enabling multiface\n");
		//sleep (1);
		//temp multiface_enable();
	}
	else {
		//printf ("Disabling multiface\n");
		//sleep (1);
		//temp multiface_disable();
	}
}
*/

z80_byte tbblue_get_diviface_enabled(void)
{
    /*
(W)		06 => Peripheral 2 setting, only in bootrom or config mode:
			bit 7 = Enable turbo mode (0 = disabled, 1 = enabled)
			bit 6 = DAC chip mode (0 = I2S, 1 = JAP)
			bit 5 = Enable Lightpen  (1 = enabled)
			bit 4 = Enable DivMMC (1 = enabled) -> divmmc automatic paging. divmmc memory is supported using manual
		*/
    return tbblue_registers[6]&16;    
}

void tbblue_set_emulator_setting_divmmc(void)
{


    z80_byte diven=tbblue_get_diviface_enabled();
    debug_printf (VERBOSE_INFO,"Apply config.divmmc change: %s",(diven ? "enabled" : "disabled") );
    //printf ("Apply config2.divmmc change: %s\n",(diven ? "enabled" : "disabled") );

    if (diven) {
        //printf ("Activando diviface automatic paging\n");
        divmmc_diviface_enable();
        diviface_allow_automatic_paging.v=1;
    }


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
	bit 1-0 = Set CPU speed (soft reset = %00)

%00 = 3.5MHz
%01 = 7MHz
%10 = 14MHz
%11 = 28MHz (works since core 3.0	  


	*/

	z80_byte t=tbblue_registers[7] & 3;

	//printf ("Setting turbo: value %d on pc %04XH\n",t,reg_pc);			

	if (t==0) cpu_turbo_speed=1;
	else if (t==1) cpu_turbo_speed=2;
	else if (t==2) cpu_turbo_speed=4;
	else if (t==3) cpu_turbo_speed=8;

	if (tbblue_deny_turbo_rom.v && reg_pc<16384) {
		//Ver el maximo permitido
		if (cpu_turbo_speed>tbblue_deny_turbo_rom_max_allowed) cpu_turbo_speed=tbblue_deny_turbo_rom_max_allowed;
	}


	cpu_set_turbo_speed();
}

void tbblue_set_emulator_setting_reg_8(void)
{
/*
(R/W) 0x08 (08) => Peripheral 3 setting:
  bit 7 = 128K paging enable (inverse of port 0x7ffd, bit 5)
          Unlike the paging lock in port 0x7ffd,
          this may be enabled or disabled at any time.
          Use "1" to disable the locked paging.
  bit 6 = "1" to disable RAM contention. (0 after a reset)
  bit 5 = Stereo mode (0 = ABC, 1 = ACB)(0 after a PoR or Hard-reset)
  bit 4 = Enable internal speaker (1 = enabled)(1 after a PoR or Hard-reset)
  bit 3 = Enable Specdrum/Covox (1 = enabled)(0 after a PoR or Hard-reset)
  bit 2 = Enable Timex modes (1 = enabled)(0 after a PoR or Hard-reset)
  bit 1 = Enable TurboSound (1 = enabled)(0 after a PoR or Hard-reset)
  bit 0 = Reserved, must be 0
*/
	z80_byte value=tbblue_registers[8];

	debug_printf (VERBOSE_DEBUG,"Setting register 8 to %02XH from PC=%04XH",value,reg_pc);

	//bit 6 = "1" to disable RAM contention. (0 after a reset)
	if (value&64) {
		//Desactivar contention. Solo hacerlo cuando hay cambio
		if (contend_enabled.v) {
			debug_printf (VERBOSE_DEBUG,"Disabling contention");
        	contend_enabled.v=0;
	        inicializa_tabla_contend();
		}
	}

	else {
		//Activar contention. Solo hacerlo cuando hay cambio
		if (contend_enabled.v==0) {
			debug_printf (VERBOSE_DEBUG,"Enabling contention");
        	contend_enabled.v=1;
	        inicializa_tabla_contend();
		}		

	}

  	//bit 5 = Stereo mode (0 = ABC, 1 = ACB)(0 after a PoR or Hard-reset)
	//ay3_stereo_mode;
	//1=ACB Stereo (Canal A=Izq,Canal C=Centro,Canal B=Der)
    //2=ABC Stereo (Canal A=Izq,Canal B=Centro,Canal C=Der)	  
	if (value&32) {
		//ACB
		ay3_stereo_mode=1;
		debug_printf (VERBOSE_DEBUG,"Setting ACB stereo");
	}
	else {
		//ABC
		ay3_stereo_mode=2;
		debug_printf (VERBOSE_DEBUG,"Setting ABC stereo");
	}


  
  	//bit 4 = Enable internal speaker (1 = enabled)(1 after a PoR or Hard-reset)
	if (value&16) {
		beeper_enabled.v=1;
		debug_printf (VERBOSE_DEBUG,"Enabling beeper");
	}
	else {
		beeper_enabled.v=0;
		debug_printf (VERBOSE_DEBUG,"Disabling beeper");
	}

  	//bit 3 = Enable Specdrum/Covox (1 = enabled)(0 after a PoR or Hard-reset)
	if (value&8) {
		audiodac_enabled.v=1;
		audiodac_selected_type=0;
		debug_printf (VERBOSE_DEBUG,"Enabling audiodac Specdrum");
	}
	else {
		audiodac_enabled.v=0;
		debug_printf (VERBOSE_DEBUG,"Disabling audiodac Specdrum");
	}


  	//bit 2 = Enable Timex modes (1 = enabled)(0 after a PoR or Hard-reset)
	if (value&4) {

		/*
		Desactivamos esto, pues NextOS al arrancar activa modo timex, y por tanto, el real video
		Con real video activado, usa mucha mas cpu
		Quitando esto, arrancara NextOS sin forzar a activar modo timex ni real video y por tanto usara menos cpu
		Si alguien quiere modo timex y/o real video, que lo habilite a mano
		debug_printf (VERBOSE_DEBUG,"Enabling timex video");
		enable_timex_video();
		*/
	}
	else {
		/*
		debug_printf (VERBOSE_DEBUG,"Disabling timex video");
		disable_timex_video();
		*/
	}
  	
	//bit 1 = Enable TurboSound (1 = enabled)(0 after a PoR or Hard-reset)
	if (value &2) set_total_ay_chips(3);
	else set_total_ay_chips(1);



}

//Para poder alterar visibilidad de sprites desde un menu
int debug_tbblue_sprite_visibility[TBBLUE_MAX_SPRITES];

void reset_debug_tbblue_sprite_visibility(void)
{
    int i;
    for (i=0;i<TBBLUE_MAX_SPRITES;i++) {
        debug_tbblue_sprite_visibility[i]=1; 
    }    
}

void tbblue_reset_common(void)
{


	tbblue_registers[18]=8;
	tbblue_registers[19]=11;

	tbblue_registers[20]=TBBLUE_DEFAULT_TRANSPARENT;

	tbblue_registers[21]=0;
	tbblue_registers[22]=0;
	tbblue_registers[23]=0;

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
	tbblue_registers[75]=0xE3;

/*
(R/W) 0x4C (76) => Transparency index for the tilemap
  bits 7-4 = Reserved, must be 0
  bits 3-0 = Set the index value (0xF after reset)
	*/

	tbblue_registers[76]=0xF;


	tbblue_registers[97]=0;
	tbblue_registers[98]=0;

	//Aunque no esté especificado como tal, ponemos este a 0
	/*
Bit	Function
7	1 to enable the tilemap
6	0 for 40x32, 1 for 80x32
5	1 to eliminate the attribute entry in the tilemap
4	palette select (0 = first Tilemap palette, 1 = second)
3	(core 3.0) enable "text mode"
2	Reserved, must be 0
1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector
	*/

	tbblue_registers[107]=0;


	tbblue_registers[112]=0;

	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1]=255;
	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]=191;

	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1]=255;
	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3]=191;

	clip_windows[TBBLUE_CLIP_WINDOW_ULA][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_ULA][1]=255;
	clip_windows[TBBLUE_CLIP_WINDOW_ULA][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]=191;

	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1]=159;
	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]=255;



	tbblue_copper_pc=0;
	
	tbblue_set_mmu_128k_default();

	tbblue_was_in_p2a_ram_in_rom.v=0;

    reset_debug_tbblue_sprite_visibility();


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




 /*
 0x8C (140) => Alternate ROM
 (R/W) (hard reset = 0)
 IMMEDIATE
   bit 7 = 1 to enable alt rom
   bit 6 = 1 to make alt rom visible only during writes, otherwise replaces rom during reads
   bit 5 = 1 to lock ROM1 (48K rom)
   bit 4 = 1 to lock ROM0 (128K rom)
 AFTER SOFT RESET (copied into bits 7-4)
   bit 3 = 1 to enable alt rom
   bit 2 = 1 to make alt rom visible only during writes, otherwise replaces rom during reads
   bit 1 = 1 to lock ROM1 (48K rom)
   bit 0 = 1 to lock ROM0 (128K rom)
 The locking mechanism also applies if the alt rom is not enabled. For the +3 and zx next, if the two lock bits are not
 zero, then the corresponding rom page is locked in place. Other models use the bits to preferentially lock the corresponding
 48K rom or the 128K rom.

 */

    z80_byte reg8c_low=tbblue_registers[0x8c];

    //Moverlo a bits altos
    reg8c_low = reg8c_low << 4;

    //Quitar de origen los bits altos

    tbblue_registers[0x8c] &=0xF;

    //Y meterle los altos

    tbblue_registers[0x8c] |=reg8c_low;	


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
	tbblue_registers[5]=1;
	tbblue_registers[6]=0;
	tbblue_registers[7]=0;
	tbblue_registers[8]=16;
	tbblue_registers[9]=0;


	tbblue_registers[0x8c]=0;

	tbblue_reset_common();


	tbblue_reset_palette_write_state();

	tbblue_port_123b=0;
    tbblue_port_123b_second_byte=0;


	if (tbblue_fast_boot_mode.v) {
		tbblue_registers[3]=3;

		tbblue_registers[8]=2+8+16; //turbosound 3 chips, specdrum, internal speaker
		
		/*
		0x08 (08) => Peripheral 3 Setting
(R/W)
  bit 7 = Unlock port 0x7ffd (read 1 indicates port 0x7ffd is not locked)
  bit 6 = Disable ram and port contention (soft reset = 0)
  bit 5 = AY stereo mode (0 = ABC, 1 = ACB) (hard reset = 0)
  bit 4 = Enable internal speaker (hard reset = 1)
  bit 3 = Enable 8-bit DACs (A,B,C,D) (hard reset = 0)
  bit 2 = Enable port 0xff Timex video mode read (hides floating bus on 0xff) (hard reset = 0)
  bit 1 = Enable turbosound (currently selected AY is frozen when disabled) (hard reset = 0)
  bit 0 = Implement issue 2 keyboard (hard reset = 0)
		*/

		set_total_ay_chips(3);

		audiodac_enabled.v=1;
		audiodac_selected_type=0;


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

z80_byte tbblue_get_register_port(void)
{
	return tbblue_last_register;
}

void tbblue_splash_monitor_mode(void)
{

	char buffer_mensaje[100];

	int refresco=50;

	if (tbblue_registers[5] & 4) refresco=60;

	int vga_mode=1;

	if ( (tbblue_registers[17] & 7) ==7 ) vga_mode=0;

	if (vga_mode) sprintf(buffer_mensaje,"Setting monitor VGA mode %d %d Hz",tbblue_registers[17] & 7,refresco);
	else sprintf(buffer_mensaje,"Setting monitor HDMI mode %d Hz",refresco);



	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);
}

void tbblue_splash_layer2_mode(void)
{

    char buffer_mensaje[100];

    sprintf(buffer_mensaje,"Setting Layer 2 mode %s",tbblue_get_layer2_mode_name() );


    screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);
}

void tbblue_get_string_palette_format(char *texto)
{
//if (value&128) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling lores video mode. 128x96 256 colours");
	/*
	(R/W) 0x43 (67) => Palette Control
	bit 0 = Disable the standard Spectrum flash feature to enable the extra colours.
  (Reset to 0 after a reset)

	(R/W) 0x42 (66) => Palette Format
  bits 7-0 = Number of the last ink colour entry on palette. (Reset to 15 after a Reset)
  This number can be 1, 3, 7, 15, 31, 63, 127 or 255.
	

	*/


	if ((tbblue_registers[67]&1)==0) strcpy (texto,"Normal Color palette");
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

		sprintf (texto,"Extra colors %d inks %d papers",tintas,papeles);

	}
		

}


void tbblue_splash_palette_format(void)
{
//if (value&128) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling lores video mode. 128x96 256 colours");
	/*
	(R/W) 0x43 (67) => Palette Control
	bit 0 = Disable the standard Spectrum flash feature to enable the extra colours.
  (Reset to 0 after a reset)

	(R/W) 0x42 (66) => Palette Format
  bits 7-0 = Number of the last ink colour entry on palette. (Reset to 15 after a Reset)
  This number can be 1, 3, 7, 15, 31, 63, 127 or 255.
	

	*/

	char mensaje[200];
	char videomode[100];

	tbblue_get_string_palette_format(videomode);

	sprintf (mensaje,"Setting %s",videomode);

	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);

	

	/*
	if ((tbblue_registers[67]&1)==0) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling extra colour palette");
	else {

		z80_byte palformat=tbblue_registers[66];

		
		//Ejemplo: mascara 3:   00000011
		//Son 4 tintas
		//64 papeles

		//Para pasar de tintas a papeles :    00000011 -> inverso -> 11111100
		//Dividimos 11111100 entre tintas, para rotar el valor 2 veces a la derecha = 252 / 4 = 63   -> +1 -> 64
		

		int tintas=palformat+1;

		int papeles=255-palformat;

		//Dado que tintas siempre es +1, nunca habra division por 0. Pero por si acaso

		if (tintas==0) papeles=0;
		else {
			papeles=(papeles/tintas)+1;
		}

		char mensaje[200];
		sprintf (mensaje,"Enabling extra colour palette: %d inks %d papers",tintas,papeles);
		screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);
	}
	*/

}

//Sincronizar los bits de registro 0x69/105 hacia layer2, ula shadow bank, puerto ff timex
void tbblue_sync_display1_reg_to_others(z80_byte value)
{
/*
(W) 0x69 (105) => DISPLAY CONTROL 1 REGISTER

Bit	Function
7	Enable the Layer 2 (alias for Layer 2 Access Port ($123B) bit 1)
6	Enable ULA shadow (bank 7) display (alias for Memory Paging Control ($7FFD) bit 3)
5-0	alias for Timex Sinclair Video Mode Control ($xxFF) bits 5:0

*/


	//123B bit 1 = Layer2 ON or OFF set=ON,
	tbblue_port_123b &=(255-2);
	if (value & 128) tbblue_port_123b |=2;

	//Bit shadow
	puerto_32765 &= (255-8);
	if (value&64) puerto_32765|=8;

	//Puerto timex
	z80_byte temp_timex_ff=timex_port_ff;
	temp_timex_ff &= (128+64); //Solo conservar bits altos
	temp_timex_ff |= (value&63); //Y del valor de entrada enviar los 6 bits bajos
	timex_port_ff=temp_timex_ff;
	//TODO: esto no genera splash de cambio de modo. Se podria llamar a set_timex_port_ff, donde este si que hace los splash,
	//pero al final sincroniza valor de timex hacia registro 105 (lo inverso de aqui), que aunque no pasaria nada, es
	//redundante
}


void tbblue_paging_128k_reg_142(void)
{
	z80_byte value=tbblue_registers[142];

	/*
	0x8E (142) => Spectrum 128K Memory Mapping
(R/W)
  bit 7 = port 0xdffd bit 0         \  RAM
  bits 6:4 = port 0x7ffd bits 2:0   /  bank 0-15

R bit 3 = 1
W bit 3 = 1 to change RAM bank, 0 = no change to mmu6 / mmu7 / RAM bank in ports 0x7ffd, 0xdffd


  bit 2 = port 0x1ffd bit 0            paging mode

If bit 2 = paging mode = 0 (normal)
  bit 1 = port 0x1ffd bit 2         \  ROM
  bit 0 = port 0x7ffd bit 4         /  select

If bit 2 = paging mode = 1 (special allRAM)
  bit 1 = port 0x1ffd bit 2         \  all
  bit 0 = port 0x1ffd bit 1         /  RAM

Writes can affect all ports 0x7ffd, 0xdffd, 0x1ffd (in Profi mapping mode, bit 3 of port 0xdffd is unaffected)
Writes always change the ROM / allRAM mapping
Writes immediately change the current mmu mapping as if by port write
	*/

	z80_byte valor_final_puerto_32765=0;
	z80_byte valor_final_puerto_8189=0;


	//W bit 3 = 1 to change RAM bank, 0 = no change to mmu6 / mmu7 / RAM bank in ports 0x7ffd, 0xdffd
	if (value & 8) {
		valor_final_puerto_32765 |=(value>>4) & 7;
	}

	else {
		//Conservar valor anterior de pagina ram
		valor_final_puerto_32765=puerto_32765 & 7;
	}


	//bit 2 = port 0x1ffd bit 0            paging mode

	//If bit 2 = paging mode = 1 (special allRAM)
	if (value & 4) {
		//All RAM
		//printf ("all ram\n");
		//sleep(2);

		//bit 2 = port 0x1ffd bit 0            paging mode
		valor_final_puerto_8189 |=1;

  		//bit 1 = port 0x1ffd bit 2         \  all
  		//bit 0 = port 0x1ffd bit 1         /  RAM
		valor_final_puerto_8189 |=(value&3)<<1;


		  
	}

	else {
		//If bit 2 = paging mode = 0 (normal)
		//bit 1 = port 0x1ffd bit 2         \  ROM
		//bit 0 = port 0x7ffd bit 4         /  select
		valor_final_puerto_8189 |=(value & 2 ? 4 : 0);
		valor_final_puerto_32765 |=(value & 1 ? 16 : 0);
	}


	//printf ("Puerto 32765: %d puerto 8189: %d\n",valor_final_puerto_32765,valor_final_puerto_8189);


	tbblue_out_port_32765(valor_final_puerto_32765);
	tbblue_out_port_8189(valor_final_puerto_8189);

	//TODO puerto 0xdffd


		//R bit 3 = 1
	//En lectura siempre a uno, por tanto metemos siempre ese bit a 1 en el registro
	//lo hacemos al final del todo por si acaso nos da por leer antes de nuevo ese registro tbblue_registers[142];
	tbblue_registers[142] |=8;
}


void tbblue_set_joystick_1_mode(void)
{

	z80_byte joystick_mode=((tbblue_registers[5] >>6) & 3 ) | ((tbblue_registers[5] >> 1) & 4) ;

	//printf("joystick mode: %d\n",joystick_mode);

	/*

	0x05 (05) => Peripheral 1 Setting
(R/W)
  bits 7:6 = Joystick 1 mode (LSB)
  bits 5:4 = Joystick 2 mode (LSB)
  bit 3 = Joystick 1 mode (MSB)
  bit 2 = 50/60 Hz mode (0 = 50Hz, 1 = 60Hz, Pentagon is always 50Hz)
  bit 1 = Joystick 2 mode (MSB)
  bit 0 = Enable scandoubler (1 = enabled)
Joystick modes:
  000 = Sinclair 2 (12345)
  001 = Kempston 1 (port 0x1F)
  010 = Cursor (56780)
  011 = Sinclair 1 (67890)
  100 = Kempston 2 (port 0x37)
  101 = MD 1 (3 or 6 button joystick port 0x1F)
  110 = MD 2 (3 or 6 button joystick port 0x37)
  111 = I/O Mode
Both joysticks are placed in I/O Mode if either is set to I/O Mode. The underlying
joystick type is not changed and reads of this register will continue to return
the last joystick type. Whether the joystick is in io mode or not is invisible
but this state can be cleared either through reset or by re-writing the register
with joystick type not equal to 111. Recovery time for a normal joystick read after
leaving I/O Mode is at most 64 scan lines.

	*/

	//TODO: que es I/O Mode ???

	//Solo hacemos caso a estos:

/*
  000 = Sinclair 2 (12345)
  001 = Kempston 1 (port 0x1F)
  010 = Cursor (56780)
  011 = Sinclair 1 (67890)
  100 = Kempston 2 (port 0x37)
  101 = MD 1 (3 or 6 button joystick port 0x1F) (como si fuera kempston)
  */

 //cualquier otro, dejarlo como estaba
 //printf ("joy mode: %d\n",joystick_mode);

 switch (joystick_mode) {
	 case 0:
	 	joystick_emulation=JOYSTICK_SINCLAIR_2;
		debug_printf(VERBOSE_DEBUG,"Setting joystick 1 emulation to Sinclair 2");
	 break;

	 case 1:
	 case 4:
     case 5:
	 	joystick_emulation=JOYSTICK_KEMPSTON;
		debug_printf(VERBOSE_DEBUG,"Setting joystick 1 emulation to Kempston");
	 break;

	 case 2:
	 	joystick_emulation=JOYSTICK_CURSOR_WITH_SHIFT;
		debug_printf(VERBOSE_DEBUG,"Setting joystick 1 emulation to Cursor");
	 break;

	 case 3:
	 	joystick_emulation=JOYSTICK_SINCLAIR_1;
		debug_printf(VERBOSE_DEBUG,"Setting joystick 1 emulation to Sinclair 1");
	 break;

     default:
        debug_printf(VERBOSE_DEBUG,"Unemulated joystick mode: %d",joystick_mode);
    break;

 }


}

	
//tbblue_last_register
//void tbblue_set_value_port(z80_byte value)
void tbblue_set_value_port_position(z80_byte index_position,z80_byte value)
{

	//Nota: algunos registros como el 0 y el 1, que son read only, deja escribirlos en el array,
	//pero luego cuando se van a leer, mediante la funcion tbblue_get_value_port_register, se obtienen de otro sitio, no del array
	//por lo que todo ok



	//printf ("register port %02XH value %02XH\n",index_position,value);

	z80_byte last_register_5=tbblue_registers[5];
	z80_byte last_register_6=tbblue_registers[6];
	z80_byte last_register_7=tbblue_registers[7];
	z80_byte last_register_8=tbblue_registers[8];
	z80_byte last_register_17=tbblue_registers[17];
	z80_byte last_register_21=tbblue_registers[21];
	z80_byte last_register_66=tbblue_registers[66];
	z80_byte last_register_67=tbblue_registers[67];
	z80_byte last_register_99=tbblue_registers[99];
    z80_byte last_register_112=tbblue_registers[112];
	
	//z80_byte aux_divmmc;

    z80_byte previous_machine_type=tbblue_registers[3]&7;

	if (index_position==3) {

        //printf ("Cambiando registro tipo maquina 3: valor: %02XH\n",value);

        //Controlar caso especial
        //(W) 0x03 (03) => Set machine type, only in IPL or config mode
        //   		bits 2-0 = Machine type:
        //      		000 = Config mode
        

        if (!(previous_machine_type==0 || tbblue_bootrom.v)) {
            debug_printf(VERBOSE_DEBUG,"Can not change machine type (to %02XH) while in non config mode or non IPL mode",value);
            //printf("Can not change machine type (to %02XH) while in non config mode or non IPL mode\n",value);

            //Preservar bits de maquina
            value &=(255-7);
            value |=previous_machine_type;
        }
    }

	if (index_position==28) {
        /*
        (W) 0x1C (28) => Clip Window control
            bits 7-4 = Reserved, must be 0
            bit 3 - reset the Tilemap clip index.
            bit 2 - reset the ULA/LoRes clip index.
            bit 1 - reset the sprite clip index.
            bit 0 - reset the Layer 2 clip index.
        */
			if (value&1) tbblue_reset_clip_window_layer2_index();
			if (value&2) tbblue_reset_clip_window_sprites_index();
			if (value&4) tbblue_reset_clip_window_ula_index();
			if (value&8) tbblue_reset_clip_window_tilemap_index();
            return;
    }

	tbblue_registers[index_position]=value;


	switch(index_position)
	{

		case 2:
		/*
0x02 (02) => Reset
(R)
  bit 7 = Indicates the reset signal to the expansion bus and esp is asserted
  bits 6:2 = Reserved
  bit 1 = Indicates the last reset was a hard reset
  bit 0 = Indicates the last reset was a soft reset
  * Only one of bits 1:0 will be set
(W)
  bit 7 = Assert and hold reset to the expansion bus and the esp wifi (hard reset = 0)
  bits 6:2 = Reserved, must be 0
  bit 1 = Generate a hard reset (reboot)
  bit 0 = Generate a soft reset
  * Hard reset has precedence
  
					*/
            if (value&2) {
                
                tbblue_bootrom.v=1;
                
                tbblue_registers[3]=0;
                
                tbblue_set_memory_pages();
                reg_pc=0;
            }

            //Hard reset has precedence. Entonces esto es un else, si hay hard reset, no haremos soft reset
            else if (value&1) {
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

        
            if (previous_machine_type==0 || tbblue_bootrom.v) {
                

                //Pentagon not supported yet. TODO
                //last_value=tbblue_config1;
                tbblue_bootrom.v=0;
                //printf ("----setting bootrom to 0\n");

                //printf ("Writing register 3 value %02XH\n",value);

                tbblue_set_memory_pages();


                //Solo cuando hay cambio
                //if ( last_register_3 != value )
                tbblue_set_emulator_setting_timing();
            }
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

		case 5:
			if ((last_register_5&4)!=(value&4)) tbblue_splash_monitor_mode();

			//joystick 1 mode
			/*
			  bits 7:6 = Joystick 1 mode (LSB)
  			bits 5:4 = Joystick 2 mode (LSB)
  			bit 3 = Joystick 1 mode (MSB)
  			bit 2 = 50/60 Hz mode (0 = 50Hz, 1 = 60Hz, Pentagon is always 50Hz)
  			bit 1 = Joystick 2 mode (MSB)
  			bit 0 = Enable scandoubler (1 = enabled)
			*/

			if ((last_register_5&(8+64+128))!=(value&(8+64+128))) {
                //printf("antes: %d\n",last_register_5&(8+64+128));
				tbblue_set_joystick_1_mode();
                //printf("despues: %d\n",value&(8+64+128));
			}
		
		break;

		

		case 6:

			//Bit 7 no me afecta, solo afecta a cambios por teclado en maquina real
			//bit 7 = Enable turbo mode (0 = disabled, 1 = enabled)(0 after a PoR or Hard-reset)

			//Si hay cambio en DivMMC
			/*
			(W)		06 => Peripheral 2 setting, only in bootrom or config mode:

						bit 4 = Enable DivMMC (1 = enabled)
						bit 3 = Enable Multiface (1 = enabled)(0 after a PoR or Hard-reset)
					*/
			if ( (last_register_6&16) != (value&16)) tbblue_set_emulator_setting_divmmc();
			//if ( (last_register_6&8) != (value&8)) tbblue_set_emulator_setting_multiface();
		break;


		case 7:
		/*
		(R/W)	07 => Turbo mode
					*/
					if ( last_register_7 != value ) tbblue_set_emulator_setting_turbo();
		break;

		case 8:
/*
(R/W) 0x08 (08) => Peripheral 3 setting:
  bit 7 = 128K paging enable (inverse of port 0x7ffd, bit 5) 
          Unlike the paging lock in port 0x7ffd, 
          this may be enabled or disabled at any time.
          Use "1" to disable the locked paging.
  bit 6 = "1" to disable RAM contention. (0 after a reset) 
  bit 5 = Stereo mode (0 = ABC, 1 = ACB)(0 after a PoR or Hard-reset)
  bit 4 = Enable internal speaker (1 = enabled)(1 after a PoR or Hard-reset)
  bit 3 = Enable Specdrum/Covox (1 = enabled)(0 after a PoR or Hard-reset)
  bit 2 = Enable Timex modes (1 = enabled)(0 after a PoR or Hard-reset)
  bit 1 = Enable TurboSound (1 = enabled)(0 after a PoR or Hard-reset)
  bit 0 = Reserved, must be 0
*/

			if ( last_register_8 != value ) tbblue_set_emulator_setting_reg_8();

		break;


		case 9:
		
/*
0x09 (09) => Peripheral 4 Setting
(R/W)
  bit 7 = Place AY 2 in mono mode (hard reset = 0)
  bit 6 = Place AY 1 in mono mode (hard reset = 0)
  bit 5 = Place AY 0 in mono mode (hard reset = 0)
  bit 4 = Sprite id lockstep (nextreg 0x34 and port 0x303B are in lockstep) (soft reset = 0)
  bit 3 = Reset divmmc mapram bit (port 0xe3 bit 6) (read returns 0)
  bit 2 = 1 to silence hdmi audio (hard reset = 0)
  bits 1:0 = Scanline weight
    00 = scanlines off
    01 = scanlines 50%
    10 = scanlines 25%
    11 = scanlines 12.5%

*/		
			
			if (value & 8) {   
				
                diviface_control_register &=(255-64);
				
				
			}
    
   		
			//printf ("out reg 9: %02XH\n",value);
		break;


		case 17:
			if ((last_register_17&7)!=(value&7)) tbblue_splash_monitor_mode();
		break;		

		case 21:
			//modo lores
			if ( (last_register_21&128) != (value&128)) {
				if (value&128) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling lores video mode. 128x96 256 colours");
				else screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling lores video mode");
			}
		break;




		case 24:
			//(W) 0x18 (24) => Clip Window Layer 2
			clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][tbblue_get_clip_window_layer2_index()]=value;
            tbblue_inc_clip_window_layer2_index();

			//debug
			//printf ("layer2 %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]);
		break;



		case 25:
			//((W) 0x19 (25) => Clip Window Sprites
			clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][tbblue_get_clip_window_sprites_index()]=value;
            tbblue_inc_clip_window_sprites_index();

			//debug
			//printf ("sprites %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3]);
		break;



		case 26:
			//(W) 0x1A (26) => Clip Window ULA/LoRes
			clip_windows[TBBLUE_CLIP_WINDOW_ULA][tbblue_get_clip_window_ula_index()]=value;
            tbblue_inc_clip_window_ula_index();

			//debug
			//printf ("ula %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_ULA][0],clip_windows[TBBLUE_CLIP_WINDOW_ULA][1],clip_windows[TBBLUE_CLIP_WINDOW_ULA][2],clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]);
		break;



		case 27:
			//(W) 0x1B (27) => Clip Window Tilemap
			clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][tbblue_get_clip_window_tilemap_index()]=value;
            tbblue_inc_clip_window_tilemap_index();

			//debug
			//printf ("tilemap %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]);
		break;



/*
(W) 0x2D (45) => SoundDrive (SpecDrum) port 0xDF mirror
 bits 7-0 = Data to be written at Soundrive
 this port can be used to send data to the SoundDrive using the Copper co-processor
*/

		case 45:

		if (audiodac_enabled.v) {
            audiodac_send_sample_value(value);
        }

		break;


		case 52:	//0x34 - sprite index
			if (tbsprite_is_lockstep()) {
				tbblue_out_port_sprite_index(value);
			} else {
				tbsprite_nr_index_sprite=value%TBBLUE_MAX_SPRITES;
			}
		break;

		// sprite attribute registers
		case 53:	case 54:	case 55:	case 56:	case 57:	//0x35, 0x36, 0x37, 0x38, 0x39
		case 117:	case 118:	case 119:	case 120:	case 121:	//0x75, 0x76, 0x77, 0x78, 0x79
		{
			int attribute_id = (index_position-0x35)&7;				//0..4
			int sprite_id = tbsprite_is_lockstep() ? tbsprite_index_sprite : tbsprite_nr_index_sprite;
			//tbsprite_sprites[sprite_id][attribute_id] = value;
            tbblue_write_sprite_value(sprite_id,attribute_id,value);
			if (index_position < 0x70) break;	//0x35, 0x36, 0x37, 0x38, 0x39 = done
			//0x75, 0x76, 0x77, 0x78, 0x79 = increment sprite id
			if (tbsprite_is_lockstep()) {
				tbsprite_increment_index_303b();
			} else {
				++tbsprite_nr_index_sprite;
				tbsprite_nr_index_sprite%=TBBLUE_MAX_SPRITES;
			}
		}
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


		case 99:
/*
(W) 0x63 (99) => Copper Data 16-bit Write

Similar to Copper Data ($60), allows to upload Copper instructions to the copper memory, 
but the difference is that writes are committed to copper memory in 16-bit words 
(only half-written instructions by using NextReg $60 may get executed, $63 prevents that).

The first write to this register is MSB of Copper Instruction destined for even copper instruction address.
The second write to this register is LSB of Copper Instruction destined for odd copper instruction address.
After any write, the copper address is auto-incremented to the next memory position.
After a write to an odd address, the entire 16-bits are written to Copper memory at once.

*/

		tbblue_copper_write_data_16b(last_register_99,value);

		break;		


		case 105:
/*
(W) 0x69 (105) => DISPLAY CONTROL 1 REGISTER

Bit	Function
7	Enable the Layer 2 (alias for Layer 2 Access Port ($123B) bit 1)
6	Enable ULA shadow (bank 7) display (alias for Memory Paging Control ($7FFD) bit 3)
5-0	alias for Timex Sinclair Video Mode Control ($xxFF) bits 5:0

*/
		//printf ("Registro 105 valor %02XH\n",value);


		tbblue_sync_display1_reg_to_others(value);

		break; 


        case 112:
            if ((last_register_112&(16+32))!=(value&(16+32))) tbblue_splash_layer2_mode();
        break;


		case 140:
			//printf ("Write to 140 (8c) register value: %02XH PC=%X\n",value,reg_pc);
			tbblue_set_memory_pages();
		break;


		case 142:
			tbblue_paging_128k_reg_142();
		break;


		default:
			/*if 
			(
			(index_position>=0x35 && index_position<=0x39)  ||
			(index_position>=0x75 && index_position<=0x79)
			) 
			{
				printf ("debug tbblue register %02XH (%d decimal) sending value %02XH\n",index_position,index_position,value);
			}*/
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

	//Casos especiales. Registros que no se obtienen leyendo del array de registros. En principio todos estos están marcados
	//como read-only en la documentacion de tbblue
	/*
	(R) 0x00 (00) => Machine ID

	(R) 0x01 (01) => Version (Nibble most significant = Major, Nibble less significant = Minor)
	*/

	//printf ("leer registro %02XH\n",registro);

	z80_int *paleta;
	z80_byte indice_paleta;

	

	switch(registro)
	{
		case 0:

/*
hardware numbers
#define HWID_DE1A               1               DE-1 
#define HWID_DE2A               2               DE-2  
#define HWID_DE2N               3               DE-2 (new) 
#define HWID_DE1N               4               DE-1 (new) 
#define HWID_FBLABS             5               FBLabs 
#define HWID_VTRUCCO   				 	6               VTrucco 
#define HWID_WXEDA              7               WXEDA 
#define HWID_EMULATORS  				8               Emulators 
#define HWID_ZXNEXT             10              ZX Spectrum Next 
#define HWID_MC                 11              Multicore 
#define HWID_ZXNEXT_AB  				250             ZX Spectrum Next Anti-brick 
*/


			return tbblue_machine_id; //8;
		break;


/*
(R) 0x01 (01) => Core Version 
  bits 7-4 = Major version number
  bits 3-0 = Minor version number
  (see register 0x0E for sub minor version number)


#define TBBLUE_CORE_VERSION_MAJOR     1 
#define TBBLUE_CORE_VERSION_MINOR     10
#define TBBLUE_CORE_VERSION_SUBMINOR  31

  */
		case 1:
			//return (TBBLUE_CORE_VERSION_MAJOR<<4 | TBBLUE_CORE_VERSION_MINOR);
			return (tbblue_core_current_version_major<<4 | tbblue_core_current_version_minor);
		break;


		case 7:
		/*
		Read:

Bit	Function
7-6	Reserved
5-4	Current actual CPU speed
3-2	Reserved
1-0	Programmed CPU speed

		*/
			return ( (tbblue_registers[7] &3) | ((tbblue_registers[7] &3)<<4) );


		break;

		case 0xE:
			//return TBBLUE_CORE_VERSION_SUBMINOR;
			return tbblue_core_current_version_subminor;
		break;		

		case 24:
			//(W) 0x18 (24) => Clip Window Layer 2
            return clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][tbblue_get_clip_window_layer2_index()];
		break;

		case 25:
			//((W) 0x19 (25) => Clip Window Sprites
            return clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][tbblue_get_clip_window_sprites_index()];
		break;

		case 26:
			//(W) 0x1A (26) => Clip Window ULA/LoRes
			return clip_windows[TBBLUE_CLIP_WINDOW_ULA][tbblue_get_clip_window_ula_index()];
		break;

		case 27:
			//(W) 0x1B (27) => Clip Window Tilemap
			return clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][tbblue_get_clip_window_tilemap_index()];
		break;

		case 28:
/*
0x1C (28) => Clip Window control
(R) (may change)
  bits 7:6 = Tilemap clip index
  bits 5:4 = ULA/Lores clip index
  bits 3:2 = Sprite clip index
  bits 1:0 = Layer 2 clip index
*/		
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


		case 65:
		//lectura paleta 
/*
0x41 (65) => Palette Value (8 bit colour)
(R/W)
  bits 7:0 = Colour for the palette index selected by nextreg 0x40.
    The format is RRRGGGBB -  the lower blue bit of the 9-bit colour will be the logical
    OR of blue bits 1 and 0 of this 8-bit value.
    After the write, the palette index is auto-incremented to the next index if the
    auto-increment is enabled in nextreg 0x43.  Reads do not auto-increment.
    Any other bits associated with the index will be zeroed.
*/		
			indice_paleta=tbblue_registers[64];

			paleta=tbblue_get_palette_rw();	

			return (paleta[indice_paleta]>>1) & 0xFF;
		break;

		case 68:
		//lectura paleta
/*
0x44 (68) => Palette Value (9 bit colour)
(R/W)
  Two consecutive writes are needed to write the 9 bit colour
  1st write:
    bits 7:0 = RRRGGGBB
  2nd write:
    bits 7:1 = Reserved, must be 0
    bit 0 = lsb B
    If writing to an L2 palette
    bit 7 = 1 for L2 priority colour, 0 for normal.
      An L2 priority colour moves L2 above all layers.  If you need the same
      colour in both priority and normal modes, you will need to have two
      different entries with the same colour one with and one without priority.
  After two consecutive writes the palette index is auto-incremented if
  auto-increment is enabled in nextreg 0x43.
  Reads only return the 2nd byte and do not auto-increment.
*/		
			indice_paleta=tbblue_registers[64];

			paleta=tbblue_get_palette_rw();	

            z80_int color=paleta[indice_paleta];

            if (color & TBBLUE_LAYER2_PRIORITY) {
                    return (color & 1) | 0x80;
            }            

			return color & 1;		
		
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


struct s_tbblue_priorities_names {
	char layers[3][20];
};


struct s_tbblue_priorities_names tbblue_priorities_names[8]={
	{ { "Sprites" ,  "Layer 2"  ,  "ULA&Tiles" } },
	{ { "Layer 2" ,  "Sprites"  ,  "ULA&Tiles" } },
	{ { "Sprites" ,  "ULA&Tiles"  ,  "Layer 2" } },
	{ { "Layer 2" ,  "ULA&Tiles"  ,  "Sprites" } },
	{ { "ULA&Tiles" ,  "Sprites"  ,  "Layer 2" } },
	{ { "ULA&Tiles" ,  "Layer 2"  ,  "Sprites" } },

//TODO:
	{ { "Invalid" ,  "Invalid"  ,  "Invalid" } },
	{ { "Invalid" ,  "Invalid"  ,  "Invalid" } },
};

//Retorna el texto de la capa que corresponde segun el byte de prioridad y la capa demandada en layer
//La capa de arriba del todo, es capa 0. La de en medio, la 1, etc
char *tbblue_get_string_layer_prio(int layer,z80_byte prio)
{
/*
     Reset default is 000, sprites over the Layer 2, over the ULA graphics
     000 - S L U
     001 - L S U
     010 - S U L
     011 - L U S
     100 - U S L
     101 - U L S

	 TODO:
	110 - (U|T)S(T|U)(B+L) Blending layer and Layer 2 combined, colours clamped to [0,7]
    111 - (U|T)S(T|U)(B+L-5) Blending layer and Layer 2 combined, colours clamped to [0,7]
*/

	//por si acaso. capa entre 0 y 7
	prio = prio & 7;

	//layer entre 0 y 2
	layer = layer % 3;

	return tbblue_priorities_names[prio].layers[layer];

}



//Inicializa punteros a los 3 layers
z80_int *p_layer_first;
z80_int *p_layer_second;
z80_int *p_layer_third;



//+int tbblue_si_sprite_transp_ficticio(z80_int color)

//z80_byte (*peek_byte_no_time)(z80_int dir);

int (*tbblue_fn_pixel_layer_transp_first)(z80_int color);
int (*tbblue_fn_pixel_layer_transp_second)(z80_int color);
int (*tbblue_fn_pixel_layer_transp_third)(z80_int color);

z80_byte tbblue_get_layers_priorities(void)
{
	return (tbblue_registers[0x15] >> 2)&7;
}

void tbblue_set_layer_priorities(void)
{
	//Por defecto
	//sprites over the Layer 2, over the ULA graphics
	p_layer_first=tbblue_layer_sprites;
	p_layer_second=tbblue_layer_layer2;
	p_layer_third=tbblue_layer_ula;



	tbblue_fn_pixel_layer_transp_first=tbblue_si_sprite_transp_ficticio;
	tbblue_fn_pixel_layer_transp_third=tbblue_si_sprite_transp_ficticio;
	tbblue_fn_pixel_layer_transp_second=tbblue_si_sprite_transp_ficticio;

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
	//z80_byte prio=(tbblue_registers[0x15] >> 2)&7;
	z80_byte prio=tbblue_get_layers_priorities();

	//printf ("prio: %d\n",prio);

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

		default:
			p_layer_first=tbblue_layer_sprites;
			p_layer_second=tbblue_layer_layer2;
			p_layer_third=tbblue_layer_ula;

		break;	
	}





}

z80_int tbblue_get_border_color(z80_int color)
{
    int flash_disabled = tbblue_registers[0x43]&1;  //flash_disabled se llamaba antes. ahora indica "enable ulanext"
    int is_timex_hires = timex_video_emulation.v && ((timex_port_ff&7) == 6);

    // 1) calculate correct color index into palette
	if (is_timex_hires) {
        // Timex HiRes 512x256 enforces border color by the FF port value, with priority over other methods
        color=get_timex_paper_mode6_color();        //0..7 PAPER index
        if (flash_disabled) color += 128;           // current HW does not bother with Bright in ULANext ON mode
        else color += 8 + 16;                       // +8 for BRIGHT 1, +16 for PAPER color in ULANext OFF mode
	}
    else if (flash_disabled) {   // ULANext mode ON

        if (tbblue_registers[0x42] == 255) {    // full-ink mode takes border colour from "fallback"
        //    // in this case this is final result, just return it (no further processing needed)
            return RGB9_INDEX_FIRST_COLOR + tbblue_get_9bit_colour(tbblue_registers[0x4A]);
        }

        // other ULANext modes take border color from palette starting at 128..135
        color += 128;
    }

    else {  // ULANext mode OFF (border colors are 16..23)
        color += 16;
    }

    // 2) convert index to actual color from palette
    color = tbblue_get_palette_active_ula(color);

    // 3) check for transparent colour -> use fallback colour if border is "transparent"
    if (tbblue_si_transparent(color)) {
        color = tbblue_get_9bit_colour(tbblue_registers[0x4A]);
    }
    return color + RGB9_INDEX_FIRST_COLOR;
}

void get_pixel_color_tbblue(z80_byte attribute,z80_int *tinta_orig, z80_int *papel_orig)
{

	/*

(R/W) 0x43 (67) => Palette Control
  bit 0 = Enabe ULANext mode if 1. (0 after a reset)

	*/

	z80_byte ink=*tinta_orig;
	z80_byte paper=*papel_orig;

	z80_byte palette_format=tbblue_registers[0x42];
	z80_byte flash_disabled=tbblue_registers[0x43]&1; //flash_disabled se llamaba antes. ahora indica "enable ulanext"


    z80_byte bright,flash;
    z80_int aux;



	if (!flash_disabled) {

/*
(R/W) 0x40 (64) => Palette Index
  bits 7-0 = Select the palette index to change the associated colour.

  For the ULA only, INKs are mapped to indices 0-7, Bright INKS to indices 8-15,
   PAPERs to indices 16-23 and Bright PAPERs to indices 24-31.

  In ULANext mode, INKs come from a subset of indices 0-127 and PAPERs come from
   a subset of indices 128-255.  The number of active indices depends on the number
   of attribute bits assigned to INK and PAPER out of the attribute byte.
  The ULA always takes border colour from paper.
*/

        ink=attribute &7; 
        paper=((attribute>>3) &7)+16; //colores papel empiezan en 16
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

Nuevo:
(R/W) 0x42 (66) => ULANext Attribute Byte Format
  bits 7-0 = Mask indicating which bits of an attribute byte are used to
             represent INK.  The rest will represent PAPER.  (15 on reset)
             The mask can only indicate a solid sequence of bits on the right
             side of the attribute byte (1, 3, 7, 15, 31, 63, 127 or 255).
             The 255 value enables the full ink colour mode and all the the palette entries 
             will be inks with all paper colours mapping to position 128.

OLD:
(R/W) 0x42 (66) => Palette Format
  bits 7-0 = Number of the last ink colour entry on palette. (Reset to 15 after a Reset)
  This number can be 1, 3, 7, 15, 31, 63, 127 or 255.
  The 255 value enables the full ink colour mode and
  all the the palette entries are inks but the paper will be the colour at position 128.
  (only applies to ULANext palette. Layer 2 and Sprite palettes works as "full ink")

TODO: el significado es el mismo antes que ahora?
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

z80_int tbblue_tile_return_color_index(z80_byte index)
{
    z80_int color_final=tbblue_get_palette_active_tilemap(index);
    return color_final;
}

void tbblue_do_tile_putpixel(z80_byte pixel_color,z80_byte transparent_colour,z80_byte tpal,z80_int *puntero_a_layer,int ula_over_tilemap)
{

    if (pixel_color!=transparent_colour) {
        //No es color transparente el que ponemos
        pixel_color |=tpal;

        //Vemos lo que hay en la capa
        z80_int color_previo_capa;
        color_previo_capa=*puntero_a_layer;

        //Poner pixel tile si color de ula era transparente o bien la ula está por debajo
        if (tbblue_si_sprite_transp_ficticio(color_previo_capa) || !ula_over_tilemap) { 
            *puntero_a_layer=tbblue_tile_return_color_index(pixel_color);
        }

    }


}

//Devuelve el color del pixel dentro de un tilemap
z80_byte tbblue_get_pixel_tile_xy_4bpp(int x,int y,z80_byte *puntero_this_tiledef)
{
	//4bpp
	int offset_x=x/2;

	int pixel_a_derecha=x%2;

	int offset_y=y*4; //Cada linea ocupa 4 bytes

	int offset_final=offset_y+offset_x;


	z80_byte byte_leido=puntero_this_tiledef[offset_final];
	if (pixel_a_derecha) {
		return byte_leido & 0xF;
	}

	else {
		return (byte_leido>>4) & 0xF;
	}

}

int tbblue_tiles_are_monocrome(void)
{
/*
Registro 6BH


  
    (R/W) 0x6B (107) => Tilemap Control


Bit	Function
7	1 to enable the tilemap
6	0 for 40x32, 1 for 80x32
5	1 to eliminate the attribute entry in the tilemap
4	palette select (0 = first Tilemap palette, 1 = second)
3	(core 3.0) enable "text mode"
2	Reserved, must be 0
1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector

0	1 to enforce "tilemap over ULA" layer priority

Bits 7 & 6 enable the tilemap and select resolution.

Bit 5 changes the structure of the tilemap so that it contains only 8-bit tilemap-id 
entries instead of 16-bit tilemap-id and tile-attribute entries.

If 8-bit tilemap is selected, the tilemap contains only tile numbers and the attributes are taken 
from Default Tilemap Attribute Register ($6C).

Bit 4 selects one of two tilemap palettes used for final colour lookup.

Bit 1 enables the 512-tile-mode when the tile attribute (either global in $6C or per tile in map data) 
contains ninth bit of tile-id value. 
In this mode the tiles are drawn under ULA pixels, unless bit 0 is used to force whole tilemap over ULA.

Bit 0 can enforce tilemap over ULA either in 512-tile-mode, or even override the per-tile bit selector from tile attributes. 
If zero, the tilemap priority is either decided by attribute bit or in 512-tile-mode it is under ULA.


*/


	return tbblue_registers[0x6b] & 8; //bit "text mode"/monocromo

}

/*
int tbblue_tiles_512_mode(void)
{
	
// (R/W) 0x6B (107) => Tilemap Control
//   bit 7    = 1 to enable the tilemap
//   bit 6    = 0 for 40x32, 1 for 80x32
//   bit 5    = 1 to eliminate the attribute entry in the tilemap
//   bit 4    = palette select
//   bits 3-2 = Reserved set to 0
//   bit 1    = 1 to activate 512 tile mode
//   bit 0    = 1 to force tilemap on top of ULA
// 
// 512 tile mode is solely entered via bit 1.  Whether the ula is enabled or not makes no difference
// 
// when in 512 tile mode, the ULA is on top of the tilemap.  You can change this by setting bit 0	
	

	return tbblue_registers[0x6b] & 1;
}
*/


//Devuelve el color del pixel dentro de un tilemap
z80_byte tbblue_get_pixel_tile_xy_monocromo(int x,int y,z80_byte *puntero_this_tiledef)
{

	//Cada linea ocupa 1 bytes

	z80_byte byte_leido=puntero_this_tiledef[y];


	return (byte_leido>> (7-x) ) & 0x1;


}


z80_byte tbblue_get_pixel_tile_xy(int x,int y,z80_byte *puntero_this_tiledef)
{
	//si monocromo
	if (tbblue_tiles_are_monocrome() ) {
		return tbblue_get_pixel_tile_xy_monocromo(x,y,puntero_this_tiledef);
	}

	else {
		return tbblue_get_pixel_tile_xy_4bpp(x,y,puntero_this_tiledef);
	}
}



void tbblue_do_tile_overlay(int scanline)
{
	//Gestion scroll vertical
	int scroll_y=tbblue_registers[49];

	//Renderizar en array tbblue_layer_ula el scanline indicado
	//leemos del tile y indicado, sumando scroll vertical
	int scanline_efectivo=scanline+scroll_y;
	scanline_efectivo %=256; 
	

	int posicion_y=scanline_efectivo/8;

	int linea_en_tile=scanline_efectivo %8;

    int tbblue_bytes_per_tile=2;

	int tilemap_width=tbblue_get_tilemap_width();

	int multiplicador_ancho=1;
	if (tilemap_width==40) multiplicador_ancho=2;
/*
//borde izquierdo + pantalla + borde derecho
#define TBBLUE_LAYERS_PIXEL_WIDTH (48+256+48)

z80_int tbblue_layer_ula[TBBLUE_LAYERS_PIXEL_WIDTH];
*/

	z80_int *puntero_a_layer;
	puntero_a_layer=&tbblue_layer_ula[(48-32)*2]; //Inicio de pantalla es en offset 48, restamos 32 pixeles que es donde empieza el tile
																								//*2 porque es doble de ancho

	z80_int *orig_puntero_a_layer;
	orig_puntero_a_layer=puntero_a_layer;

  /*
Bit	Function
7	1 to enable the tilemap
6	0 for 40x32, 1 for 80x32
5	1 to eliminate the attribute entry in the tilemap
4	palette select (0 = first Tilemap palette, 1 = second)
3	(core 3.0) enable "text mode"
2	Reserved, must be 0
1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector
   */
	z80_byte tbblue_tilemap_control=tbblue_registers[107];

	if (tbblue_tilemap_control&32) tbblue_bytes_per_tile=1;




	z80_byte *puntero_tilemap;	
	z80_byte *puntero_tiledef;

	//Gestion scroll
/*(R/W) 0x2F (47) => Tilemap Offset X MSB
  bits 7-2 = Reserved, must be 0
  bits 1-0 = MSB X Offset
  Meaningful Range is 0-319 in 40 char mode, 0-639 in 80 char mode

(R/W) 0x30 (48) => Tilemap Offset X LSB
  bits 7-0 = LSB X Offset
  Meaningful range is 0-319 in 40 char mode, 0-639 in 80 char mode

(R/W) 0x31 (49) => Tilemap Offset Y
  bits 7-0 = Y Offset (0-191)
*/
	int scroll_x=tbblue_registers[48]+256*(tbblue_registers[47] & 3);

	//Llevar control de posicion x pixel en destino dentro del rango (0..40*8, 0..80*8)
	int max_destino_x_pixel=tilemap_width*8;

	scroll_x %=max_destino_x_pixel;

	int destino_x_pixel=0;

	int offset_sumar=0;
	if (scroll_x) {
		//Si hay scroll_x, no que hacemos es empezar a escribir por la parte final derecha
		destino_x_pixel=max_destino_x_pixel-scroll_x;
		offset_sumar=destino_x_pixel;
	}


	offset_sumar *=multiplicador_ancho;
	puntero_a_layer +=offset_sumar;


	//Clipwindow horizontal. Limites
	
				/*
				The tilemap display surface extends 32 pixels around the central 256×192 display.
The origin of the clip window is the top left corner of this area 32 pixels to the left and 32 pixels above 
the central 256×192 display. The X coordinates are internally doubled to cover the full 320 pixel width of the surface.
 The clip window indicates the portion of the tilemap display that is non-transparent and its indicated extent is inclusive; 
 it will extend from X1*2 to X2*2+1 horizontally and from Y1 to Y2 vertically.
			*/




	int clipwindow_min_x=clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0]*2;
	int clipwindow_max_x=(clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1]+1)*2;




	//Para controlar clipwindow. Coordenadas de destino_x_pixel van de 0 a 319 en modo 40 columnas, o de 0 a 639 en modo 80 columnas
	if (tilemap_width==80) {
		clipwindow_min_x *=2;
		clipwindow_max_x *=2;
	}

	//printf ("clipwindow_min_x %d clipwindow_max_x %d\n",clipwindow_min_x,clipwindow_max_x);

	//Inicio del tilemap
	puntero_tilemap=tbblue_ram_memory_pages[5*2]+(256*tbblue_get_offset_start_tilemap());

	//Obtener offset sobre tilemap
	int offset_tilemap=tbblue_bytes_per_tile*tilemap_width*posicion_y;


	puntero_tilemap +=offset_tilemap;  //Esto apuntara al primer tile de esa posicion y y con x=0


	//Inicio del tiledef
	puntero_tiledef=tbblue_ram_memory_pages[5*2]+(256*tbblue_get_offset_start_tiledef());

	//puntero_a_layer -=scroll_x; //temp chapuza


	int x;

	int xmirror,ymirror,rotate;
	z80_byte tpal;

	z80_byte byte_first;
	z80_byte byte_second;

	int ula_over_tilemap;

    // 0 when tilemap-over-ULA is enforced, 1 when attribute ULA-over-tilemap bit should be used
    int ula_over_tilemap_mask = (tbblue_tilemap_control&1)^1;	

	//tilemap_width=40;
/*
(R/W) 0x4C (76) => Transparency index for the tilemap
  bits 7-4 = Reserved, must be 0
  bits 3-0 = Set the index value (0xF after reset)
Defines the transparent colour index for tiles. The 4-bit pixels of a tile definition are compared to this value to determine if they are transparent.
*/
	z80_byte transparent_colour=tbblue_registers[76] & 0xF;



	//printf ("y: %d t_scanline_draw: %d rainbowy:%d sprite_y: %d\n",y,t_scanline_draw,rainbowy,sprite_y);
	z80_byte tbblue_default_tilemap_attr=tbblue_registers[108];


// Bloque mejorado del fork de Peter Ped Helcmanovsky

        for (x=0;x<tilemap_width;x++) {
                //TODO stencil mode
                byte_first=*puntero_tilemap;
                puntero_tilemap++;
                if (tbblue_bytes_per_tile==2) {
                        byte_second=*puntero_tilemap;
                        puntero_tilemap++;
                } else {
                        byte_second = tbblue_default_tilemap_attr;
                }

                int tnum=byte_first;

/*
  bits 15-12 : palette offset
  bit     11 : x mirror
  bit     10 : y mirror
  bit      9 : rotate
  bit      8 : ULA over tilemap OR bit 8 of tile number (512 tile mode)
  bits   7-0 : tile number
  */

                if (tbblue_tiles_are_monocrome()) {
					//En modo texto:
// bits 15-9: palette offset (7 bits)
// bit 8 : ULA over tilemap (in 512 tile mode, bit 8 of the tile number)
// bits 7-0 : tile number
// 
// The tiles are defined like UDGs (1 bit per pixel) and that 1 bit is combined with 
// the 7-bit palette offset to form the 8-bit pixel that gets looked up in the tilemap palette.

						//Que mal documentado esta el tema de paleta... no se rota bits a la derecha
                        tpal=(byte_second)&0xFE;
                        xmirror=0;
                        ymirror=0;
                        rotate=0;
                } else {
						//Que mal documentado esta el tema de paleta... no se rota bits a la derecha
                        tpal=(byte_second)&0xF0;
                        xmirror=(byte_second>>3)&1;
                        ymirror=(byte_second>>2)&1;
                        rotate=(byte_second>>1)&1;
                }


                if (tbblue_tilemap_control&2) {
                        // 512 tile mode
                        tnum |= (byte_second&1)<<8;
                        ula_over_tilemap = ula_over_tilemap_mask;
                } else {
                        // 256 tile mode, "ULA over tilemap" bit used from attribute (plus "force tilemap")
                        ula_over_tilemap = byte_second & ula_over_tilemap_mask;
                }

                //printf ("Color independiente. tpal:%d byte_second: %02XH\n",tpal,byte_second);

                //Sacar puntero a principio tiledef.
                int offset_tiledef;


                if (tbblue_tiles_are_monocrome()) {
                        offset_tiledef=tnum*TBBLUE_TILE_HEIGHT;
                }
                else {
                        //4 bpp. cada tiledef ocupa 4 bytes * 8 = 32
                        offset_tiledef=tnum*(TBBLUE_TILE_WIDTH/2)*TBBLUE_TILE_HEIGHT;
                }

                //sumar posicion y
                //offset_tiledef += linea_en_tile*4;

                //tiledef

                //printf ("tpal %d\n",tpal);
	


//FIN del bloque mejorado del fork de Peter Ped Helcmanovsky




		//Renderizar los 8 pixeles del tile
		int pixel_tile;
		z80_byte *puntero_this_tiledef;
		puntero_this_tiledef=&puntero_tiledef[offset_tiledef];


		//Incrementos de x e y
		int incx=+1;
		int incy=0;

		z80_byte sx=0,sy=0; //Coordenadas x,y dentro del tile

		//sumar posicion y
		sy += linea_en_tile;		


		//Aplicar mirror si conviene y situarnos en la ultima linea
		if (ymirror) {
			//sy=TBBLUE_TILE_HEIGHT-1-diferencia;
			sy=TBBLUE_TILE_HEIGHT-1-linea_en_tile;
		}
		else {
			//sy=diferencia;
		}

		//Cambiar offset si mirror x, ubicarlo a la derecha del todo
		if (xmirror) {
			sx=TBBLUE_TILE_WIDTH-1;
			incx=-1;
		}


	//Rotacion. Mismo metodo que con sprites
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
		^       ---X----
		|     	---X----

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

			
		if (rotate) {
			z80_byte sy_old=sy;
			sy=(TBBLUE_TILE_HEIGHT-1)-sx;
			sx=sy_old;

			incy=-incx;
			incx=0;
			//printf ("Tiles con rotacion size %d\n",tbblue_bytes_per_tile);
		}


		for (pixel_tile=0;pixel_tile<8;pixel_tile+=2) { //Saltamos de dos en dos porque son 4bpp


			z80_byte pixel_izq,pixel_der;

			//Pixel izquierdo
			pixel_izq=tbblue_get_pixel_tile_xy(sx,sy,puntero_this_tiledef);

			if (destino_x_pixel>=clipwindow_min_x && destino_x_pixel<clipwindow_max_x) {
				tbblue_do_tile_putpixel(pixel_izq,transparent_colour,tpal,puntero_a_layer,ula_over_tilemap);
				if (tilemap_width==40) tbblue_do_tile_putpixel(pixel_izq,transparent_colour,tpal,puntero_a_layer+1,ula_over_tilemap);
			}
			puntero_a_layer++;
			if (tilemap_width==40) puntero_a_layer++;
			destino_x_pixel++;



			sx=sx+incx;
			sy=sy+incy;

			//Controlar si se sale por la derecha (pues hay scroll)
			if (destino_x_pixel==max_destino_x_pixel) {
				destino_x_pixel=0;
				puntero_a_layer=orig_puntero_a_layer;
			}



			//Pixel derecho
			pixel_der=tbblue_get_pixel_tile_xy(sx,sy,puntero_this_tiledef);

			if (destino_x_pixel>=clipwindow_min_x && destino_x_pixel<clipwindow_max_x) {
				tbblue_do_tile_putpixel(pixel_der,transparent_colour,tpal,puntero_a_layer,ula_over_tilemap);
				if (tilemap_width==40) tbblue_do_tile_putpixel(pixel_der,transparent_colour,tpal,puntero_a_layer+1,ula_over_tilemap);
			}
			puntero_a_layer++;
			if (tilemap_width==40) puntero_a_layer++;
			destino_x_pixel++;

			sx=sx+incx;
			sy=sy+incy;

			//Controlar si se sale por la derecha (pues hay scroll)
			if (destino_x_pixel==max_destino_x_pixel) {
				destino_x_pixel=0;
				puntero_a_layer=orig_puntero_a_layer;
			}


		}


  }

}

void tbblue_fast_render_ula_layer(z80_int *puntero_final_rainbow,int estamos_borde_supinf,
	int final_borde_izquierdo,int inicio_borde_derecho,int ancho_rainbow)
{


	int i;
		z80_int color;


	//(R/W) 0x4A (74) => Transparency colour fallback
	//	bits 7-0 = Set the 8 bit colour.
	//	(0 = black on reset on reset)
	z80_int fallbackcolour = RGB9_INDEX_FIRST_COLOR + tbblue_get_9bit_colour(tbblue_registers[74]);

	for (i=0;i<ancho_rainbow;i++) {


		//Primera capa
		color=tbblue_layer_ula[i];
		if (!tbblue_si_sprite_transp_ficticio(color) ) {
			*puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
			//doble de alto
			puntero_final_rainbow[ancho_rainbow]=RGB9_INDEX_FIRST_COLOR+color; 
		}

	
					
        else {
            if (estamos_borde_supinf) {
                //Si estamos en borde inferior o superior, no hacemos nada, dibujar color borde
            }

            else {
                //Borde izquierdo o derecho o pantalla. Ver si estamos en pantalla
                if (i>=final_borde_izquierdo && i<inicio_borde_derecho) {
                    //Poner color indicado por "Transparency colour fallback" registro
                    *puntero_final_rainbow=fallbackcolour;
                    //doble de alto
                    puntero_final_rainbow[ancho_rainbow]=fallbackcolour;								
                }
                else {
                    //Es borde. dejar ese color
                }
            
            }
        }



		puntero_final_rainbow++;

		
	}

}


//Nos situamos en la linea justo donde empiezan los tiles
void tbblue_render_layers_rainbow(int capalayer2,int capasprites)
{


	//(R/W) 0x4A (74) => Transparency colour fallback
		//	bits 7-0 = Set the 8 bit colour.
		//	(0 = black on reset on reset)
	z80_int fallbackcolour = RGB9_INDEX_FIRST_COLOR + tbblue_get_9bit_colour(tbblue_registers[74]);

	int y;
	int diferencia_border_tiles;

	//diferencia_border_tiles=screen_indice_inicio_pant-TBBLUE_TILES_BORDER;
	//Tamaño del border efectivo restando espacio usado por tiles/layer2 en border
	diferencia_border_tiles=screen_borde_superior-TBBLUE_TILES_BORDER;

    y=t_scanline_draw-screen_invisible_borde_superior;


	//y=t_scanline_draw-screen_indice_inicio_pant;
    if (border_enabled.v==0) y=y-screen_borde_superior;


    //if (y<diferencia_border_tiles || y>=(screen_indice_inicio_pant+192+TBBLUE_TILES_BORDER)) {	

    if (y<diferencia_border_tiles || y>=(screen_borde_superior+192+TBBLUE_TILES_BORDER)) {	
        
        //printf ("t_scanline_draw: %d y: %d diferencia_border_tiles: %d screen_indice_inicio_pant: %d screen_invisible_borde_superior: %d TBBLUE_TILES_BORDER: %d\n",
        //	t_scanline_draw,y,diferencia_border_tiles,screen_indice_inicio_pant,screen_invisible_borde_superior,TBBLUE_TILES_BORDER);

        //Si estamos por encima o por debajo de la zona de tiles/layer2,
        //que es la mas alta de todas las capas

        return; 

    }
		

    //Calcular donde hay border
    int final_border_superior=screen_indice_inicio_pant-screen_invisible_borde_superior;
    int inicio_border_inferior=final_border_superior+192;

    //Doble de alto
    y *=2;

    final_border_superior *=2;
    inicio_border_inferior *=2;

    //Vemos si linea esta en zona border
    int estamos_borde_supinf=0;
    if (y<final_border_superior || y>=inicio_border_inferior) estamos_borde_supinf=1;

    //Zona borde izquierdo y derecho
    int final_borde_izquierdo=2*screen_total_borde_izquierdo*border_enabled.v;
    int inicio_borde_derecho=final_borde_izquierdo+TBBLUE_DISPLAY_WIDTH;




    int ancho_rainbow=get_total_ancho_rainbow();

	z80_int *puntero_final_rainbow=&rainbow_buffer[ y*ancho_rainbow ];

	//Por defecto
	//sprites over the Layer 2, over the ULA graphics


	tbblue_set_layer_priorities();

	z80_int color;
	
	//printf ("ancho total: %d size layers: %d\n",get_total_ancho_rainbow(),TBBLUE_LAYERS_PIXEL_WIDTH );

	int i;

	//Si solo hay capa ula, hacer render mas rapido
	//printf ("%d %d %d\n",capalayer2,capasprites,tbblue_get_layers_priorities());
	//if (capalayer2==0 && capasprites==0 && tbblue_get_layers_priorities()==0) {  //prio 0=S L U
	if (capalayer2==0 && capasprites==0) { 	 
		//Hará fast render cuando no haya capa de layer2 o sprites, aunque tambien,
		//estando esas capas, cuando este en zona de border o no visible de dichas capas
		tbblue_fast_render_ula_layer(puntero_final_rainbow,estamos_borde_supinf,final_borde_izquierdo,inicio_borde_derecho,ancho_rainbow);

	}	



	else {

        for (i=0;i<ancho_rainbow;i++) {


            color = tbblue_layer_layer2[i];

            //Si no se permite layer2 con priority bit, resetearlo
            if (tbblue_allow_layer2_priority_bit.v==0) {
                if (tbblue_color_is_layer2_priority(color)) {
                    //Este para el check que viene luego de tbblue_color_is_layer2_priority, que no lo detecte como priority
                    color &= 0x1FF;
                    //Y este ya para el renderizado de capas, dejar un color normal de 9 bits
                    tbblue_layer_layer2[i] &= 0x1FF;
                }
            }

            //Si color de layer2 tiene bit de prioridad y no es el transparente
            //Se mira que no sea el color ficticio de transparente porque este es 65535 (todos los bits a 1) y el
            //TBBLUE_LAYER2_PRIORITY=0x8000, por lo que un color como transparente se podria interpretar como que es de layer2 con prioridad
            if (tbblue_color_is_layer2_priority(color)) {
                //printf("bit con prioridad\n");
                //Tiene prioridad. Quitar ese bit de prioridad y cualquier otro que no es el indice de color
                color &= 0x1FF;

                *puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
                //doble de alto
                puntero_final_rainbow[ancho_rainbow]=RGB9_INDEX_FIRST_COLOR+color;            
            }

            else {

                //Primera capa
                color=p_layer_first[i];
                if (!tbblue_fn_pixel_layer_transp_first(color) ) {
                    *puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
                    //doble de alto
                    puntero_final_rainbow[ancho_rainbow]=RGB9_INDEX_FIRST_COLOR+color;
                }

                else {
                    color=p_layer_second[i];
                    if (!tbblue_fn_pixel_layer_transp_second(color) ) {
                        *puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
                        //doble de alto
                        puntero_final_rainbow[ancho_rainbow]=RGB9_INDEX_FIRST_COLOR+color;				
                    }

                    else {
                        color=p_layer_third[i];
                        if (!tbblue_fn_pixel_layer_transp_third(color) ) {
                            *puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
                            //doble de alto
                            puntero_final_rainbow[ancho_rainbow]=RGB9_INDEX_FIRST_COLOR+color;					
                        }
                            
                        else {
                            if (estamos_borde_supinf) {
                                //Si estamos en borde inferior o superior, no hacemos nada, dibujar color borde
                            }

                            else {
                                //Borde izquierdo o derecho o pantalla. Ver si estamos en pantalla
                                if (i>=final_borde_izquierdo && i<inicio_borde_derecho) {
                                    //Poner color indicado por "Transparency colour fallback" registro:
                                    *puntero_final_rainbow=fallbackcolour;
                                    //doble de alto
                                    puntero_final_rainbow[ancho_rainbow]=fallbackcolour;
                                }
                                else {
                                    //Es borde. dejar ese color
                                }
                            
                            }
                        }
                    }

                }
            }

            puntero_final_rainbow++;

            
        }

	}
}


char *tbblue_layer2_video_modes_names[]={
	"256x192 8bpp",
	"320x256 8bpp",
	"640x256 4bpp",
	"Unknown"
};

char *tbblue_get_layer2_mode_name(void)
{
    //Resolucion si 256x192x8, organizacion en scanlines, o las otras resoluciones que organizan en columnas
    //00=256x192x8. 01=320x256x8, 10=640x256x4
    int layer2_resolution=(tbblue_registers[112]>>4) & 3; 

    return tbblue_layer2_video_modes_names[layer2_resolution];
}


void tbblue_do_layer2_overlay(int linea_render)
{


    if (!tbblue_is_active_layer2() || tbblue_force_disable_layer_layer_two.v) return;

    //Resolucion si 256x192x8, organizacion en scanlines, o las otras resoluciones que organizan en columnas
    //00=256x192x8. 01=320x256x8, 10=640x256x4
    int layer2_resolution=(tbblue_registers[112]>>4) & 3; 


    //Obtener offset paleta color
    int palette_offset=tbblue_registers[112] & 15;

    

    //Obtener inicio pantalla layer2
    //int tbblue_layer2_offset=tbblue_get_offset_start_layer2();

    int tbblue_layer2_offset=tbblue_get_offset_start_layer2_reg(tbblue_registers[18]);


    //Scroll vertical

/*
(R/W) 0x17 (23) => Layer2 Offset Y
bits 7-0 = Y Offset (0-191)(Reset to 0 after a reset)
*/		
    //Mantener el offset y en 0..191
    z80_byte tbblue_reg_23=tbblue_registers[23]; 


    int offset_scroll=tbblue_reg_23+linea_render;

    if (layer2_resolution) {
        offset_scroll %=256;
        tbblue_layer2_offset +=offset_scroll;
    }

    else {
        offset_scroll %=192;
        tbblue_layer2_offset +=offset_scroll*256;
    }



    //Scroll horizontal
/*

(R/W) 22 => Layer2 Offset X
  bits 7-0 = X Offset (0-255)(Reset to 0 after a reset)
0x71 (113) => Layer 2 X Scroll MSB
(R/W)
   bits 7:1 = Reserved, must be 0
   bit 0 = MSB of scroll amount		
*/

    int tbblue_reg_22=tbblue_registers[22] + (tbblue_registers[113]&1)*256;



    //Valor final de scroll x, acotar a valores validos
    if (layer2_resolution) {
        tbblue_reg_22 %=320;
    }

    else {
        tbblue_reg_22 %=256;
    }


    //Para la gestión de la posicion x del pixel
    int pos_x_origen=tbblue_reg_22;

    

    //Inicio de la posicion en el layer final
    int posicion_array_layer=0;


    int borde_no_escribible=screen_total_borde_izquierdo;
    if (layer2_resolution>0) borde_no_escribible-=TBBLUE_LAYER2_12_BORDER;

    posicion_array_layer +=borde_no_escribible*2; //doble de ancho

    int posx;

    //Total pixeles por defecto
    int total_x=256;

    
    int clip_min=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0];
    int clip_max=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1];

    if (layer2_resolution) {
        total_x +=TBBLUE_LAYER2_12_BORDER*2;

        //Se multiplica por 2 siempre en estas resoluciones de 320x256 y 640x256
        clip_min *=2;
        clip_max *=2;			

    }


    for (posx=0;posx<total_x;posx++) {
            

        //printf ("posx: %d pos_x_origen: %d\n",posx,pos_x_origen);
            
        if (posx>=clip_min && posx<=clip_max ) {
        
            int offset_pixel;

            z80_byte pixel_izq,pixel_der;

            if (layer2_resolution) {
                offset_pixel=tbblue_layer2_offset+pos_x_origen*256;
            }

            else {
                offset_pixel=tbblue_layer2_offset+pos_x_origen;
            }


            offset_pixel &=0x1FFFFF;


            

            z80_byte byte_leido=memoria_spectrum[offset_pixel];

            if (layer2_resolution==2) {
                pixel_izq=(byte_leido>>4) & 0xF;
                pixel_der=(byte_leido   ) & 0xF;					

            }

            else {
                pixel_izq=byte_leido;
                pixel_der=pixel_izq;
            }

            z80_int final_color_layer2_izq=tbblue_get_palette_active_layer2(pixel_izq+palette_offset);

            //Ver si color resultante es el transparente de ula, y cambiarlo por el color transparente ficticio
            if (tbblue_si_transparent(final_color_layer2_izq)) final_color_layer2_izq=TBBLUE_SPRITE_TRANS_FICT;


            z80_int final_color_layer2_der=tbblue_get_palette_active_layer2(pixel_der+palette_offset);

            //Ver si color resultante es el transparente de ula, y cambiarlo por el color transparente ficticio
            if (tbblue_si_transparent(final_color_layer2_der)) final_color_layer2_der=TBBLUE_SPRITE_TRANS_FICT;		

            tbblue_layer_layer2[posicion_array_layer]=final_color_layer2_izq;
            tbblue_layer_layer2[posicion_array_layer+1]=final_color_layer2_der;

        }

        //Este incremento se tiene que hacer siempre fuera, para que se aplique siempre, se haga o no clipping
        posicion_array_layer+=2;



        //else {
        //	printf ("fuera rango\n");
        //}

                    

        //Siguiente posicion
        pos_x_origen++;
        if (pos_x_origen>=total_x) {
            pos_x_origen=0;
        }
            

    }


	 
}

void tbblue_reveal_layer_draw(z80_int *layer)
{
	int i;

	for (i=0;i<TBBLUE_LAYERS_PIXEL_WIDTH;i++) {
		z80_int color=*layer;
	
		if (!tbblue_si_sprite_transp_ficticio(color)) {

			//Color de revelado es blanco o negro segun cuadricula:
			// Negro Blanco Negro ...
			// Blanco Negro Blanco ...
			// Negro Blanco Negro ....
			// .....

			//Por tanto tener en cuenta posicion x e y
			int posx=i&1;
			int posy=t_scanline_draw&1;

			//0,0: 0
			//0,1: 1
			//1,0: 1
			//1,0: 0
			//Es un xor

			int si_blanco_negro=posx ^ posy;

			*layer=511*si_blanco_negro; //ultimo de los colores en paleta rgb9 de tbblue -> blanco, negro es 0
		}

		layer++;
	}
}


//Forzar a dibujar capa con color fijo, para debug
z80_bit tbblue_reveal_layer_ula={0};
z80_bit tbblue_reveal_layer_layer2={0};
z80_bit tbblue_reveal_layer_sprites={0};

void tbblue_do_ula_standard_overlay()
{


	//Render de capa standard ULA (normal, timex) 

	//printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

	//linea que se debe leer
	int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;



	int x,bit;
	z80_int direccion;
	z80_byte byte_leido;


	int color=0;
	z80_byte attribute;
	z80_int ink,paper;


	z80_byte *screen=get_base_mem_pantalla();


	/*
0x26 (38) => ULA X Scroll
(R/W)
  bits 7:0 = X Offset (0-255) (soft reset = 0)

	*/

	//entonces sumar 1 posicion por cada 8 del scroll

	z80_byte ula_offset_x=tbblue_registers[38];
	int indice_origen_bytes=ula_offset_x*2; //*2 dado que leemos del puntero_buffer_atributos que guarda 2 bytes: pixel y atributo	

	/*
0x27 (39) => ULA Y Scroll
(R/W)
  bits 7:0 = Y Offset (0-191) (soft reset = 0)


	*/

	z80_byte tbblue_scroll_y=tbblue_registers[39];
	

	scanline_copia +=tbblue_scroll_y;
	scanline_copia=scanline_copia % 192;



	//Usado cuando hay scroll vertical y por tanto los pixeles y atributos salen de la pantalla tal cual (modo sin rainbow)
	int pos_no_rainbow_pix_x;


	//scroll x para modo no rainbow (es decir, cuando hay scroll vertical)
	pos_no_rainbow_pix_x=ula_offset_x;
	pos_no_rainbow_pix_x %=32;	


	//Estos direccion y dir_atributo usados cuando hay scroll vertical y por tanto los pixeles y atributos salen de la pantalla tal cual (modo sin rainbow),
	//y tambien en timex 512x192
	direccion=screen_addr_table[(scanline_copia<<5)];

	int fila=scanline_copia/8;
	int dir_atributo=6144+(fila*32);


	z80_byte *puntero_buffer_atributos;
	z80_byte col6;
	z80_byte tin6, pap6;

	z80_byte timex_video_mode=timex_port_ff&7;
	z80_bit si_timex_hires={0};
	z80_bit si_timex_8_1={0};

	if (timex_video_mode==2) si_timex_8_1.v=1;

	//Por defecto
	puntero_buffer_atributos=scanline_buffer;

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
				//512x192 monocromo. 
				//y color siempre fijo
				/*
	bits D3-D5: Selection of ink and paper color in extended screen resolution mode (000=black/white, 001=blue/yellow, 010=red/cyan, 011=magenta/green, 100=green/magenta, 101=cyan/red, 110=yellow/blue, 111=white/black); these bits are ignored when D2=0

				black, blue, red, magenta, green, cyan, yellow, white
				*/

				//Si D2==0, these bits are ignored when D2=0?? Modo 4 que es??

				tin6=get_timex_ink_mode6_color();


				//Obtenemos color
				pap6=get_timex_paper_mode6_color();
				//printf ("papel: %d\n",pap6);

				//Y con brillo
				col6=((pap6*8)+tin6)+64;

			
				si_timex_hires.v=1;
			break;


		}
	}

	//Capa de destino
	int posicion_array_layer=0;
	posicion_array_layer +=(screen_total_borde_izquierdo*border_enabled.v*2); //Doble de ancho


	int columnas=32;

	if (si_timex_hires.v) {
		columnas=64;
	}

    for (x=0;x<columnas;x++) {

		if (tbblue_scroll_y) {
			//Si hay scroll vertical (no es 0) entonces el origen de los bytes no se obtiene del buffer de pixeles y color en alta resolucion,
			//Si no que se obtiene de la pantalla tal cual
			//TODO: esto es una limitacion de tal y como hace el render el tbblue, en que hago render de una linea cada vez,
			//para corregir esto, habria que tener un buffer destino con todas las lineas de ula y hacer luego overlay con cada
			//capa por separado, algo completamente impensable
			//de todas maneras esto es algo extraño que suceda: que alguien le de por hacer efectos en color en alta resolucion, en capa ula,
			//y activar el scroll vertical. En teoria tambien puede hacer parpadeos en juegos normales, pero quien va a querer cambiar el scroll en juegos
			//que no estan preparados para hacer scroll?
			byte_leido=screen[direccion+pos_no_rainbow_pix_x];


			if (si_timex_8_1.v==0) {
				attribute=screen[dir_atributo+pos_no_rainbow_pix_x];	
			}

			else {
				//timex 8x1
				attribute=screen[direccion+pos_no_rainbow_pix_x+8192];
			}



		}

		else {

			//Modo sin scroll vertical. Permite scroll horizontal. Es modo rainbow

			

			//Pero si no tenemos scanline
			if (tbblue_store_scanlines.v==0) {
				byte_leido=screen[direccion+pos_no_rainbow_pix_x];
				attribute=screen[dir_atributo+pos_no_rainbow_pix_x];	
				indice_origen_bytes+=2;
			}

			else {
				byte_leido=puntero_buffer_atributos[indice_origen_bytes++];
				attribute=puntero_buffer_atributos[indice_origen_bytes++];
			}

		}



		//32 columnas
		//truncar siempre a modulo 64 (2 bytes: pixel y atributo)
		indice_origen_bytes %=64;

		if (si_timex_hires.v) {
			if ((x&1)==0) byte_leido=screen[direccion+pos_no_rainbow_pix_x];
			else byte_leido=screen[direccion+pos_no_rainbow_pix_x+8192];

			attribute=col6;
		}			
			
		get_pixel_color_tbblue(attribute,&ink,&paper);
			
    	for (bit=0;bit<8;bit++) {			
			color= ( byte_leido & 128 ? ink : paper ) ;

			int posx=x*8+bit; //Posicion pixel. Para clip window registers	
			if (si_timex_hires.v) posx /=2;

			//Tener en cuenta valor clip window
			
			//(W) 0x1A (26) => Clip Window ULA/LoRes
			if (posx>=clip_windows[TBBLUE_CLIP_WINDOW_ULA][0] && posx<=clip_windows[TBBLUE_CLIP_WINDOW_ULA][1] && scanline_copia>=clip_windows[TBBLUE_CLIP_WINDOW_ULA][2] && scanline_copia<=clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]) {
				if (!tbblue_force_disable_layer_ula.v) {
					z80_int color_final=tbblue_get_palette_active_ula(color);

					//Ver si color resultante es el transparente de ula, y cambiarlo por el color transparente ficticio
					if (tbblue_si_transparent(color_final)) color_final=TBBLUE_SPRITE_TRANS_FICT;

					tbblue_layer_ula[posicion_array_layer]=color_final;
					if (si_timex_hires.v==0) tbblue_layer_ula[posicion_array_layer+1]=color_final; //doble de ancho

				}
			}

		
			posicion_array_layer++;
			if (si_timex_hires.v==0) posicion_array_layer++; //doble de ancho
        	byte_leido=byte_leido<<1;
				
      	}

		if (si_timex_hires.v) {
				if (x&1) {
					pos_no_rainbow_pix_x++;
					//direccion++;
				}
		}

		else {
			//direccion++;
			pos_no_rainbow_pix_x++;
		}


			
		pos_no_rainbow_pix_x %=32;		

	  }
	
}




void tbblue_do_ula_lores_overlay()
{


	//Render de capa ULA LORES
	//printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

	//linea que se debe leer
	int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;


	int color;

	/* modo lores
	(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  	*/

	  	

	z80_byte *lores_pointer;
	z80_byte posicion_x_lores_pointer;

	
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
  		


	int posicion_array_layer=0;
	posicion_array_layer +=(screen_total_borde_izquierdo*border_enabled.v*2); //Doble de ancho


	int posx;
	z80_int color_final;

	for (posx=0;posx<256;posx++) {
				
		color=lores_pointer[posicion_x_lores_pointer/2];
		//tenemos indice color de paleta
		//transformar a color final segun paleta ula activa
		//color=tbblue_get_palette_active_ula(lorescolor);

		posicion_x_lores_pointer++; 
		//nota: dado que es una variable de 8 bits, automaticamente se trunca al pasar de 255 a 0, por tanto no hay que sacar el modulo de division con 256
		
		//Tener en cuenta valor clip window
		
		//(W) 0x1A (26) => Clip Window ULA/LoRes
		if (posx>=clip_windows[TBBLUE_CLIP_WINDOW_ULA][0] && posx<=clip_windows[TBBLUE_CLIP_WINDOW_ULA][1] && scanline_copia>=clip_windows[TBBLUE_CLIP_WINDOW_ULA][2] && scanline_copia<=clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]) {
			if (!tbblue_force_disable_layer_ula.v) {
				color_final=tbblue_get_palette_active_ula(color);

				//Ver si color resultante es el transparente de ula, y cambiarlo por el color transparente ficticio
				if (tbblue_si_transparent(color_final)) color_final=TBBLUE_SPRITE_TRANS_FICT;

				tbblue_layer_ula[posicion_array_layer]=color_final;
				tbblue_layer_ula[posicion_array_layer+1]=color_final; //doble de ancho

			}
		}

		posicion_array_layer+=2; //doble de ancho
				
    }


}

//Guardar en buffer rainbow la linea actual. 
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
void screen_store_scanline_rainbow_solo_display_tbblue(void)
{

	//si linea no coincide con entrelazado, volvemos
	if (if_store_scanline_interlace(t_scanline_draw)==0) return;

	
	//48% cpu en welcome screen. Alternativa mas lenta sin memfill

	/*
	int i;
	z80_int *clear_p_ula=tbblue_layer_ula;
	z80_int *clear_p_layer2=tbblue_layer_layer2;
	z80_int *clear_p_sprites=tbblue_layer_sprites;

	for (i=0;i<TBBLUE_LAYERS_PIXEL_WIDTH;i++) {

		//Esto es un pelin mas rapido hacerlo asi, con punteros e incrementarlos, en vez de indices a array
		*clear_p_ula=TBBLUE_SPRITE_TRANS_FICT;
		// *clear_p_layer2=TBBLUE_TRANSPARENT_REGISTER_9;
		*clear_p_layer2=TBBLUE_SPRITE_TRANS_FICT;
		*clear_p_sprites=TBBLUE_SPRITE_TRANS_FICT;

		clear_p_ula++;
		clear_p_layer2++;
		clear_p_sprites++;

	}
	*/

	//Alternativa con memfill. Esto solo se puede hacer dado que TBBLUE_SPRITE_TRANS_FICT=65535=0xFFFF y por tanto escribe
	//dos bytes iguales
	//46% cpu en welcome screen

	//Por si acaso en un futuro cambia ese valor
	if (TBBLUE_SPRITE_TRANS_FICT!=65535) cpu_panic("Changed transparent value. Can not do fast layer clear");

	//Tenemos que escribir en array de z80_int (2 bytes)
	int tamanyo_clear=TBBLUE_LAYERS_PIXEL_WIDTH*2;
	memset(tbblue_layer_ula,0xFF,tamanyo_clear);
	memset(tbblue_layer_layer2,0xFF,tamanyo_clear);
	memset(tbblue_layer_sprites,0xFF,tamanyo_clear);
	



	//int bordesupinf=0;

	int capalayer2=0;
	int capasprites=0;
	//int capatiles=0;

  	//En zona visible pantalla (no borde superior ni inferior)
  	if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {

        //int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;


        int tbblue_lores=tbblue_registers[0x15] & 128;
        if (tbblue_lores) tbblue_do_ula_lores_overlay();
        else {
            if (tbblue_if_ula_is_enabled() ) {
                tbblue_do_ula_standard_overlay();
            }
        }

    //Overlay de layer2
                        //Capa layer2
            /*if (tbblue_is_active_layer2() && !tbblue_force_disable_layer_layer_two.v) {
                if (scanline_copia>=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2] && scanline_copia<=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]) {
                    capalayer2=1;
                
                    tbblue_do_layer2_overlay();
                    if (tbblue_reveal_layer_layer2.v) {
                            tbblue_reveal_layer_draw(tbblue_layer_layer2);
                    }
                }
            }*/

	}

	else {
		//bordesupinf=1;
	}

	//Aqui puede ser borde superior o inferior




    //Overlay de layer2
    //Capa layer2
    if (tbblue_is_active_layer2() && !tbblue_force_disable_layer_layer_two.v) {
        int y_layer2=t_scanline_draw; //0..63 es border (8 no visibles);
        int border_no_visible=screen_indice_inicio_pant-TBBLUE_LAYER2_12_BORDER;


        int layer2_resolution=(tbblue_registers[112]>>4) & 3; 

        if (layer2_resolution>0) {
            y_layer2 -=border_no_visible;
        }
        else {
            y_layer2 -=screen_indice_inicio_pant;
        }

        int dibujar=0;


        if (layer2_resolution==0) {
            if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {
                if (y_layer2>=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2] && y_layer2<=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]) {
                    dibujar=1;
                }
            }
        }

        else if (y_layer2>=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2] && y_layer2<=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]) {
            dibujar=1;
        }

        if (dibujar) {
            capalayer2=1;
        
            tbblue_do_layer2_overlay(y_layer2);


        

            if (tbblue_reveal_layer_layer2.v) {
                    tbblue_reveal_layer_draw(tbblue_layer_layer2);
            }
        }

                    
    }

		
    //Capa de tiles. Mezclarla directamente a la capa de ula tbblue_layer_ula


	if ( tbblue_if_tilemap_enabled() && tbblue_force_disable_layer_tilemap.v==0) {
		int y_tile=t_scanline_draw; //0..63 es border (8 no visibles)
		int border_no_visible=screen_indice_inicio_pant-TBBLUE_TILES_BORDER;
		y_tile-=border_no_visible;

				/*
				The tilemap display surface extends 32 pixels around the central 256×192 display.
The origin of the clip window is the top left corner of this area 32 pixels to the left and 32 pixels above 
the central 256×192 display. The X coordinates are internally doubled to cover the full 320 pixel width of the surface.
 The clip window indicates the portion of the tilemap display that is non-transparent and its indicated extent is inclusive; 
 it will extend from X1*2 to X2*2+1 horizontally and from Y1 to Y2 vertically.
			*/

			//Tener en cuenta clip window
		if (y_tile>=clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2] && y_tile<=clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]) {
			//capatiles=1;
			tbblue_do_tile_overlay(y_tile);
		}
	}


    if (tbblue_reveal_layer_ula.v) {
            tbblue_reveal_layer_draw(tbblue_layer_ula);
    }



	//capa sprites. Si clip window y corresponde:
	z80_byte sprites_over_border=tbblue_registers[21]&2;
	//Clip window on Sprites only work when the "over border bit" is disabled
	int mostrar_sprites=1;
	if (sprites_over_border==0) {
		int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;
		if (scanline_copia<clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2] || scanline_copia>clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3]) mostrar_sprites=0;
	}

	
	if (mostrar_sprites && !tbblue_force_disable_layer_sprites.v) {
		capasprites=1;
		tbsprite_do_overlay();

        if (tbblue_reveal_layer_sprites.v) {
                tbblue_reveal_layer_draw(tbblue_layer_sprites);
        }

	}




    //Renderizamos las 3 capas buffer rainbow
	tbblue_render_layers_rainbow(capalayer2,capasprites);



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




void screen_tbblue_refresca_pantalla_comun_tbblue(int x,int y,unsigned int color)
{

    int dibujar=0;

    //if (x>255) dibujar=1;
    //else if (y>191) dibujar=1;
    if (scr_ver_si_refrescar_por_menu_activo(x/8,y/8)) dibujar=1;

    if (dibujar) {
        scr_putpixel_zoom(x,y,color);
        scr_putpixel_zoom(x,y+1,color);
        scr_putpixel_zoom(x+1,y,color);
        scr_putpixel_zoom(x+1,y+1,color);
    }
}


//Refresco pantalla sin rainbow para tbblue
void screen_tbblue_refresca_pantalla_comun(void)
{
    int x,y,bit;
    z80_int direccion,dir_atributo;
    z80_byte byte_leido;
    int color=0;
    int fila;
    //int zx,zy;

    z80_byte attribute,ink,paper,bright,flash,aux;


    z80_byte *screen=get_base_mem_pantalla();

    //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
    z80_byte x_hi;

    for (y=0;y<192;y++) {
        //direccion=16384 | devuelve_direccion_pantalla(0,y);

        //direccion=16384 | screen_addr_table[(y<<5)];
        direccion=screen_addr_table[(y<<5)];


        fila=y/8;
        dir_atributo=6144+(fila*32);
        for (x=0,x_hi=0;x<32;x++,x_hi +=8) {



            byte_leido=screen[direccion];
            attribute=screen[dir_atributo];


            ink=attribute &7;
            paper=(attribute>>3) &7;
                        bright=(attribute) &64;
            flash=(attribute)&128;
            if (flash) {
                    //intercambiar si conviene
                    if (estado_parpadeo.v) {
                            aux=paper;
                            paper=ink;
                            ink=aux;
                    }
            }

            if (bright) {
                    ink +=8;
                    paper +=8;
            }

            for (bit=0;bit<8;bit++) {

                color= ( byte_leido & 128 ? ink : paper );

                //Por cada pixel, hacer *2s en ancho y alto.
                //Esto es muy simple dado que no soporta modo rainbow y solo el estandard 256x192
                screen_tbblue_refresca_pantalla_comun_tbblue((x_hi+bit)*2,y*2,color);


                byte_leido=byte_leido<<1;
            }
    


            direccion++;
            dir_atributo++;
        }

    }

}



void screen_tbblue_refresca_no_rainbow_border(void)
{
	int color;

	if (simulate_screen_zx8081.v==1) color=15;
	else color=out_254 & 7;

	if (scr_refresca_sin_colores.v) color=7;

    int x,y;



    //parte superior
    for (y=0;y<TBBLUE_TOP_BORDER;y++) {
        for (x=0;x<TBBLUE_DISPLAY_WIDTH*zoom_x+TBBLUE_LEFT_BORDER*2;x++) {
            scr_putpixel(x,y,color);


        }
    }

    //parte inferior
    for (y=0;y<TBBLUE_TOP_BORDER;y++) {
        for (x=0;x<TBBLUE_DISPLAY_WIDTH*zoom_x+TBBLUE_LEFT_BORDER*2;x++) {
            scr_putpixel(x,TBBLUE_TOP_BORDER+y+TBBLUE_DISPLAY_HEIGHT*zoom_y,color);


        }
    }


    //laterales
    for (y=0;y<TBBLUE_DISPLAY_HEIGHT*zoom_y;y++) {
        for (x=0;x<TBBLUE_LEFT_BORDER;x++) {
                scr_putpixel(x,TBBLUE_TOP_BORDER+y,color);
                scr_putpixel(TBBLUE_LEFT_BORDER+TBBLUE_DISPLAY_WIDTH*zoom_x+x,TBBLUE_TOP_BORDER+y,color);
        }

    }



}


//Refresco pantalla con rainbow. Nota. esto deberia ser una funcion comun y no tener diferentes para comun, prism, tbblue, etc
void screen_tbblue_refresca_rainbow(void)
{


	//aqui no tiene sentido (o si?) el modo simular video zx80/81 en spectrum
	int ancho,alto;

	ancho=get_total_ancho_rainbow();
	alto=get_total_alto_rainbow();

	int x,y,bit;

	//margenes de zona interior de pantalla. Para overlay menu
	int margenx_izq=TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v;
	int margenx_der=TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v+TBBLUE_DISPLAY_WIDTH;
	int margeny_arr=TBBLUE_TOP_BORDER_NO_ZOOM*border_enabled.v;
	int margeny_aba=TBBLUE_BOTTOM_BORDER_NO_ZOOM*border_enabled.v+TBBLUE_DISPLAY_HEIGHT;

	z80_int color_pixel;
	z80_int *puntero;

	puntero=rainbow_buffer;
	int dibujar;


	//Si se reduce la pantalla 0.75
	if (screen_reduce_075.v) {
		screen_scale_075_function(ancho,alto);
		puntero=new_scalled_rainbow_buffer;
	}
	//Fin reduccion pantalla 0.75





	for (y=0;y<alto;y++) {


		for (x=0;x<ancho;x+=8) {
			dibujar=1;

			//Ver si esa zona esta ocupada por texto de menu u overlay

			if (y>=margeny_arr && y<margeny_aba && x>=margenx_izq && x<margenx_der) {
				if (!scr_ver_si_refrescar_por_menu_activo( (x-margenx_izq)/8, (y-margeny_arr)/8) )
					dibujar=0;
			}


			if (dibujar==1) {
                for (bit=0;bit<8;bit++) {
                    color_pixel=*puntero++;
                    scr_putpixel_zoom_rainbow(x+bit,y,color_pixel);
                }
			}
			else puntero+=8;

		}
		
	}


}





void screen_tbblue_refresca_no_rainbow(void)
{
    //modo clasico. sin rainbow
    if (rainbow_enabled.v==0) {
        if (border_enabled.v) {
            //ver si hay que refrescar border
            if (modificado_border.v)
            {
                //scr_refresca_border();
                screen_tbblue_refresca_no_rainbow_border();
                modificado_border.v=0;
            }

        }

        screen_tbblue_refresca_pantalla_comun();
    }
}


void tbblue_out_port_8189(z80_byte value)
{
    //Puerto tipicamente 8189
    // the hardware will respond to all port addresses with bit 1 reset, bit 12 set and bits 13, 14 and 15 reset).

    //printf ("TBBLUE changing port 8189 value=0x%02XH\n",value);
    puerto_8189=value;

    //En rom entra la pagina habitual de modo 128k, evitando lo que diga la mmu
    tbblue_registers[80]=255;
    tbblue_registers[81]=255;

    tbblue_set_memory_pages();
                        
}

void tbblue_out_port_32765(z80_byte value)
{
    //printf ("TBBLUE changing port 32765 value=0x%02XH\n",value);
    puerto_32765=value;

    //para indicar a la MMU la  pagina en los segmentos 6 y 7
    tbblue_registers[80+6]=(value&7)*2;
    tbblue_registers[80+7]=(value&7)*2+1;

    //En rom entra la pagina habitual de modo 128k, evitando lo que diga la mmu
    tbblue_registers[80]=255;
    tbblue_registers[81]=255;

    tbblue_set_memory_pages();

    //Sincronizar bit shadow


    /*
    (W) 0x69 (105) => DISPLAY CONTROL 1 REGISTER

    Bit	Function
    7	Enable the Layer 2 (alias for Layer 2 Access Port ($123B) bit 1)
    6	Enable ULA shadow (bank 7) display (alias for Memory Paging Control ($7FFD) bit 3)
    5-0	alias for Timex Sinclair Video Mode Control ($xxFF) bits 5:0

    */
    tbblue_registers[105] &= (255-64);
    if (value&8) tbblue_registers[105]|=64;

}


z80_byte tbblue_uartbridge_readdata(void)
{

	return uartbridge_readdata();
}


void tbblue_uartbridge_writedata(z80_byte value)
{
 
	uartbridge_writedata(value);


}

z80_byte tbblue_uartbridge_readstatus(void)
{
	//No dispositivo abierto
	if (!uartbridge_available()) return 0;

	
	int status=chardevice_status(uartbridge_handler);
	

	z80_byte status_retorno=0;

	if (status & CHDEV_ST_RD_AVAIL_DATA) status_retorno |= TBBLUE_UART_STATUS_DATA_READY;

	return status_retorno;
}
