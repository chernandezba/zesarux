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

#include "coleco.h"
#include "vdp_9918a.h"
#include "cpu.h"
#include "debug.h"
//#include "ay38912.h"
#include "screen.h"
#include "audio.h"
#include "sn76489an.h"
#include "joystick.h"
#include "settings.h"

z80_byte *coleco_vram_memory=NULL;



//slots asignados, y sus 4 segmentos
//tipos: rom, ram, vacio
//int coleco_memory_slots[4][4];


//Si lee joystick (0) o keypad (1)
int colleco_controller_joystick_mode=1;


const char *coleco_string_memory_type_rom="ROM";
const char *coleco_string_memory_type_ram="RAM";
const char *coleco_string_memory_type_empty="EMPTY";

z80_bit coleco_cartridge_inserted={0};

char *coleco_get_string_memory_type(int tipo)
{


    switch (tipo) {

        case COLECO_SLOT_MEMORY_TYPE_ROM:
            return (char *)coleco_string_memory_type_rom;
        break;

        case COLECO_SLOT_MEMORY_TYPE_RAM:
            return (char *)coleco_string_memory_type_ram;
        break;

        default:
            return (char *)coleco_string_memory_type_empty;
        break;

    }
}


//Retorna direccion de memoria donde esta mapeada la ram y su tipo
z80_byte *coleco_return_segment_address(z80_int direccion,int *tipo)
{

/*
0000-1FFF = BIOS ROM
2000-3FFF = Expansion port
4000-5FFF = Expansion port
6000-7FFF = RAM (1K mapped into an 8K spot)
8000-9FFF = Cart ROM
A000-BFFF = Cart ROM
C000-DFFF = Cart ROM
E000-FFFF = Cart ROM
*/

    //ROM
    if (direccion<=0x1FFF || direccion>=0x8000) {
        *tipo=COLECO_SLOT_MEMORY_TYPE_ROM;
        return &memoria_spectrum[direccion];
    }

    //RAM 1 KB
    else if (direccion>=0x6000 && direccion<=0x7FFF) {
        *tipo=COLECO_SLOT_MEMORY_TYPE_RAM;
        return &memoria_spectrum[0x6000 + (direccion & 1023)];
    }

    //Vacio
    else {
        *tipo=COLECO_SLOT_MEMORY_TYPE_EMPTY;
        return &memoria_spectrum[direccion];
    }







}


void coleco_init_memory_tables(void)
{




}


void coleco_reset(void)
{

    colleco_controller_joystick_mode=1;


    //Resetear vram
    int i;

    for (i=0;i<16384;i++) coleco_vram_memory[i]=0;

}

void coleco_out_port_vdp_data(z80_byte value)
{
    vdp_9918a_out_vram_data(coleco_vram_memory,value);
}


z80_byte coleco_in_port_vdp_data(void)
{
    return vdp_9918a_in_vram_data(coleco_vram_memory);
}



z80_byte coleco_in_port_vdp_status(void)
{
    return vdp_9918a_in_vdp_status();
}

void coleco_out_port_vdp_command_status(z80_byte value)
{
    vdp_9918a_out_command_status(value);
}





void coleco_alloc_vram_memory(void)
{
    if (coleco_vram_memory==NULL) {
        coleco_vram_memory=malloc(16384);
        if (coleco_vram_memory==NULL) cpu_panic("Cannot allocate memory for coleco vram");
    }
}


z80_byte coleco_read_vram_byte(z80_int address)
{
    //Siempre leer limitando a 16 kb
    return coleco_vram_memory[address & 16383];
}



void coleco_insert_rom_cartridge(char *filename)
{

	debug_printf(VERBOSE_INFO,"Inserting coleco rom cartridge %s",filename);

    if (!si_existe_archivo(filename)) {
        debug_printf(VERBOSE_ERR,"File %s not found",filename);
        return;
    }

    long tamanyo_archivo=get_file_size(filename);

    if (tamanyo_archivo!=8192 && tamanyo_archivo!=16384 && tamanyo_archivo!=24576 && tamanyo_archivo!=32768) {
        debug_printf(VERBOSE_ERR,"Only 8k, 16k, 24k and 32k rom cartridges are allowed");
        return;
    }

        FILE *ptr_cartridge;
        ptr_cartridge=fopen(filename,"rb");

        if (!ptr_cartridge) {
		debug_printf (VERBOSE_ERR,"Unable to open cartridge file %s",filename);
                return;
        }



	//Leer cada bloque de 16 kb si conviene. Esto permite tambien cargar cartucho de 8kb como si fuera de 16kb

	int bloque;

    int salir=0;

    int bloques_totales=0;

	for (bloque=0;bloque<2 && !salir;bloque++) {
        /*
        The ROM Header

A ROM needs a header to be auto-executed by the system when the COLECO is initialized.

After finding the RAM and initializing the system variables, the COLECO looks for the ROM headers in all the slots
on the memory pages 4000h-7FFFh and 8000h-FFFh. The search is done in ascending order.
When a primary Slot is expanded, the search is done in the corresponding secondary Slots before going to the next Primary Slot.
When the system finds a header, it selects the ROM slot only on the memory page corresponding to the address specified in INIT then, runs the program in ROM at the same address. (In short, it makes an inter-slot call.)

        */
        int offset=32768+bloque*16384;
		int leidos=fread(&memoria_spectrum[offset],1,16384,ptr_cartridge);
        if (leidos==16384) {
            //coleco_memory_slots[1][1+bloque]=COLECO_SLOT_MEMORY_TYPE_ROM;
            debug_printf (VERBOSE_INFO,"Loaded 16kb bytes of rom at slot 1 block %d",bloque);

            bloques_totales++;

        }
        else {
            salir=1;
        }

	}

/*
    if (bloques_totales==1) {
            //Copiar en los otros 2 segmentos

            //Antes, si es un bloque de 8kb, copiar 8kb bajos en parte alta
            if (tamanyo_archivo==8192) {
                memcpy(&memoria_spectrum[32768+8192],&memoria_spectrum[32768],8192);
            }

            memcpy(&memoria_spectrum[49152],&memoria_spectrum[32768],16384);



    }
    */



    //int i;


        fclose(ptr_cartridge);


        if (noautoload.v==0) {
                debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
                reset_cpu();
        }

    coleco_cartridge_inserted.v=1;

}


void coleco_empty_romcartridge_space(void)
{


//poner a 0 desde 0x2000
    int i;
    for (i=0x2000;i<65536;i++) {
        memoria_spectrum[i]=0;
    }

    coleco_cartridge_inserted.v=0;

}





//Refresco pantalla sin rainbow
void scr_refresca_pantalla_y_border_coleco_no_rainbow(void)
{



    if (border_enabled.v && vdp_9918a_force_disable_layer_border.v==0) {
            //ver si hay que refrescar border
            if (modificado_border.v)
            {
                    vdp_9918a_refresca_border();
                    modificado_border.v=0;
            }

    }


    if (vdp_9918a_force_disable_layer_ula.v==0) {

        //Capa activada. Pero tiene reveal?

        if (vdp_9918a_reveal_layer_ula.v) {
            //En ese caso, poner fondo tramado
            int x,y;
            for (y=0;y<192;y++) {
                for (x=0;x<256;x++) {
                    int posx=x&1;
			        int posy=y&1;
                    int si_blanco_negro=posx ^ posy;
                    int color=si_blanco_negro*15;
                    scr_putpixel_zoom(x,y,  VDP_9918_INDEX_FIRST_COLOR+color);
                }
            }
        }
        else {
            vdp_9918a_render_ula_no_rainbow(coleco_vram_memory);
        }
    }

    else {
        //En ese caso, poner fondo negro
        int x,y;
        for (y=0;y<192;y++) {
            for (x=0;x<256;x++) {
                scr_putpixel_zoom(x,y,  VDP_9918_INDEX_FIRST_COLOR+0);
            }
        }
    }



    if (vdp_9918a_force_disable_layer_sprites.v==0) {
        vdp_9918a_render_sprites_no_rainbow(coleco_vram_memory);
    }




}


void scr_refresca_pantalla_y_border_coleco(void)
{
    if (rainbow_enabled.v) {
        vdp_9918a_scr_refresca_pantalla_y_border_rainbow();
    }
    else {
        scr_refresca_pantalla_y_border_coleco_no_rainbow();
    }
}







//Almacenaje temporal de render de la linea actual
z80_int coleco_scanline_buffer[512];


void screen_store_scanline_rainbow_coleco_border_and_display(void)
{

    screen_store_scanline_rainbow_vdp_9918a_border_and_display(coleco_scanline_buffer,coleco_vram_memory);


}



/*
RO � Ajuste fino del tono, canal A
R1 � Ajuste aproximado del tono, canal A-
R2 � Ajuste fino del tono, canal B
R3 � Ajuste aproximado del tono, canal B
R4 � Ajuste fino del tono, canal C
R5 � Ajuste aproximado del tono, canal C
*/


//Fino: xxxxx|D3|D2|D1|D0|
//Aproximado: |xx|xx|D9|D8|D7|D6|D5|D4|

//Valores de 10 bits
//z80_byte temp_coleco_audio_frecuencies[6];

/*
int coleco_get_frequency_channel(int canal)
{
    if (canal<0 || canal>2) return 0;

    z80_byte fino,aproximado;

    fino=sn_chip_registers[canal*2] & 0xF;
    aproximado=(sn_chip_registers[canal*2+1] & 63);

    int frecuencia=(aproximado<<4) | fino;

    return frecuencia;
}
*/


//Establecer frecuencia del AY con valor entrada de 10 bits. funcion TEMPORAL
/*
void coleco_set_sn_freq(int canal,int frecuencia)
{

    canal=canal % 3; //0,1,2

    z80_byte fino,aproximado;

    fino=frecuencia & 0xF;
    aproximado=(frecuencia >>4) & 63;

    sn_set_channel_fine_tune(canal,fino);

    sn_set_channel_aprox_tune(canal,aproximado);

}

*/






z80_byte coleco_get_joypad_a(void)
{

    z80_byte valor_joystick=255;

/*
static INPUT_PORTS_START( coleco_hand_controller )
	PORT_START("COMMON0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )

	PORT_START("COMMON1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(coleco_hand_controller_device, keypad_r)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )

	PORT_START("KEYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_PLUS_PAD)
INPUT_PORTS_END
*/

//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4

			//z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

			if ((puerto_especial_joystick&1)) valor_joystick &=(255-2);
			if ((puerto_especial_joystick&2)) valor_joystick &=(255-8);
			if ((puerto_especial_joystick&4)) valor_joystick &=(255-4);
			if ((puerto_especial_joystick&8)) valor_joystick &=(255-1);

			if ((puerto_especial_joystick&16)) valor_joystick &=(255-64);

            //Espacio tambien vale como Fire/A
            //puerto_32766    db              255  ; B    N    M    Simb Space ;7
            if ((puerto_32766 & 1)==0) valor_joystick &=(255-64);

            //Custom = Tecla C

            //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
            if ((puerto_65278 & 8)==0) valor_joystick &=(255-128);







    return valor_joystick;
}




z80_byte coleco_get_joypad_b(void)
{

    z80_byte valor_joystick=255;

/*
	PORT_START("COMMON1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(coleco_hand_controller_device, keypad_r)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )

    ?????
*/


    return valor_joystick;
}


z80_byte coleco_get_keypad_a(void)
{
    z80_byte valor_joystick=255;


/*

    Info de mame
    https://github.com/mamedev/mame/blob/master/src/devices/bus/coleco/controller/hand.cpp

	if (!BIT(keypad, 0)) data &= 0x0a;
	if (!BIT(keypad, 1)) data &= 0x0d;
	if (!BIT(keypad, 2)) data &= 0x07;
	if (!BIT(keypad, 3)) data &= 0x0c;
	if (!BIT(keypad, 4)) data &= 0x02;
	if (!BIT(keypad, 5)) data &= 0x03;
	if (!BIT(keypad, 6)) data &= 0x0e;
	if (!BIT(keypad, 7)) data &= 0x05;
	if (!BIT(keypad, 8)) data &= 0x01;
	if (!BIT(keypad, 9)) data &= 0x0b;
	if (!BIT(keypad, 10)) data &= 0x06;
	if (!BIT(keypad, 11)) data &= 0x09;
    */


    //Tecla 0
    //puerto_61438    db              255  ; 6    7    8    9    0     ;4
    if ((puerto_61438 & 1)==0) valor_joystick &=0x0a;   //binario 1010

    //Tecla 1
    //puerto_63486    db              255  ; 5    4    3    2    1     ;3
    if ((puerto_63486 & 1)==0) valor_joystick &=0x0d;  //binario 1101

    //Tecla 2
    //puerto_63486    db              255  ; 5    4    3    2    1     ;3
    if ((puerto_63486 & 2)==0) valor_joystick &=0x07;  //binario 0111

    //Tecla 3
    //puerto_63486    db              255  ; 5    4    3    2    1     ;3
    if ((puerto_63486 & 4)==0) valor_joystick &=0x0c;  //binario 1100

    //Tecla 4
    //puerto_63486    db              255  ; 5    4    3    2    1     ;3
    if ((puerto_63486 & 8)==0) valor_joystick &=0x02;  //binario 0010

    //Tecla 5
    //puerto_63486    db              255  ; 5    4    3    2    1     ;3
    if ((puerto_63486 & 16)==0) valor_joystick &=0x03; //binario 0011

    //Tecla 6
    //puerto_61438    db              255  ; 6    7    8    9    0     ;4
    if ((puerto_61438 & 16)==0) valor_joystick &=0x0e; //binario 1110

    //Tecla 7
    //puerto_61438    db              255  ; 6    7    8    9    0     ;4
    if ((puerto_61438 & 8)==0) valor_joystick &=0x05; //binario 0101

    //Tecla 8
    //puerto_61438    db              255  ; 6    7    8    9    0     ;4
    if ((puerto_61438 & 4)==0) valor_joystick &=0x01;  //binario 0001

    //Tecla 9
    //puerto_61438    db              255  ; 6    7    8    9    0     ;4
    if ((puerto_61438 & 2)==0) valor_joystick &=0x0b;  //binario 1011

    //# -> tecla Z
    //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
    if ((puerto_65278 & 2)==0) valor_joystick &=0x06;  //binario 0110

    //* -> tecla X
    //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
    if ((puerto_65278 & 4)==0) valor_joystick &=0x09;  //binario 1001

    //printf("valor joystick: %02X %02X\n",valor_joystick,valor_joystick^255);

    return valor_joystick;
}

z80_byte coleco_get_keypad_b(void)
{

        //Por que no hay keypad B conectado??
    z80_byte valor_joystick=255;
    return valor_joystick;

}


void coleco_set_keypad_mode(void)
{
    colleco_controller_joystick_mode=0;
}


void coleco_set_joystick_mode(void)
{
    colleco_controller_joystick_mode=1;
}



z80_byte coleco_get_controller_a(void)
{

    //si estamos en el menu, no devolver tecla
    if (zxvision_key_not_sent_emulated_mach() ) return 255;


    if (colleco_controller_joystick_mode) return coleco_get_joypad_a();
    else return coleco_get_keypad_a();
}



z80_byte coleco_get_controller_b(void)
{

    //si estamos en el menu, no devolver tecla
    if (zxvision_key_not_sent_emulated_mach() ) return 255;

    if (colleco_controller_joystick_mode) return coleco_get_joypad_b();
    else return coleco_get_keypad_b();
}