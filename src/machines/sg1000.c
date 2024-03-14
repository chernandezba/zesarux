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

#include "sg1000.h"
#include "vdp_9918a.h"
#include "cpu.h"
#include "debug.h"
//#include "ay38912.h"
#include "screen.h"
#include "audio.h"
#include "sn76489an.h"
#include "joystick.h"
#include "settings.h"



//Memory Map - Development - SMS Power!
//https://www.smspower.org/Development/MemoryMap

//https://github.com/mikebenfield/euphrates/blob/master/euphrates/src/hardware/sms_io.rs

z80_byte *sg1000_vram_memory=NULL;





const char *sg1000_string_memory_type_rom="ROM";
const char *sg1000_string_memory_type_ram="RAM";
const char *sg1000_string_memory_type_empty="EMPTY";

z80_bit sg1000_cartridge_inserted={0};

char *sg1000_get_string_memory_type(int tipo)
{



    switch (tipo) {

        case SG1000_SLOT_MEMORY_TYPE_ROM:
            return (char *)sg1000_string_memory_type_rom;
        break;

        case SG1000_SLOT_MEMORY_TYPE_RAM:
            return (char *)sg1000_string_memory_type_ram;
        break;

        default:
            return (char *)sg1000_string_memory_type_empty;
        break;

    }
}


//Retorna direccion de memoria donde esta mapeada la ram y su tipo
z80_byte *sg1000_return_segment_address(z80_int direccion,int *tipo)
{

/*
Region	Maps to
$0000-$bfff	Cartridge (ROM/RAM/etc)
$c000-$c3ff	System RAM
$c400-$ffff	System RAM (mirrored every 1KB)
*/

    //ROM
    if (direccion<=0xbfff) {
        *tipo=SG1000_SLOT_MEMORY_TYPE_ROM;
        return &memoria_spectrum[direccion];
    }

    //RAM 1 KB
    else {
        *tipo=SG1000_SLOT_MEMORY_TYPE_RAM;
        return &memoria_spectrum[0xc000 + (direccion & 1023)];

        //Para SMS asi:
        //return &memoria_spectrum[0xc000 + (direccion & 8191)];
        /*
Master System/Mark III (assuming Sega mapper)
Region	Maps to
$0000-$03ff	ROM (unpaged)
$0400-$3fff	ROM mapper slot 0
$4000-$7fff	ROM mapper slot 1
$8000-$bfff	ROM/RAM mapper slot 2
$c000-$dfff	System RAM
$e000-$ffff	System RAM (mirror)
$fff8	3D glasses control
$fff9-$fffb	3D glasses control (mirrors)
$fffc	Cartridge RAM mapper control
$fffd	Mapper slot 0 control
$fffe	Mapper slot 1 control
$ffff	Mapper slot 2 control

        */
    }






}


void sg1000_init_memory_tables(void)
{




}


void sg1000_reset(void)
{

    //Resetear vram
    int i;

    for (i=0;i<16384;i++) sg1000_vram_memory[i]=0;

}

void sg1000_out_port_vdp_data(z80_byte value)
{
    vdp_9918a_out_vram_data(sg1000_vram_memory,value);
}


z80_byte sg1000_in_port_vdp_data(void)
{
    return vdp_9918a_in_vram_data(sg1000_vram_memory);
}



z80_byte sg1000_in_port_vdp_status(void)
{
    return vdp_9918a_in_vdp_status();
}

void sg1000_out_port_vdp_command_status(z80_byte value)
{
    vdp_9918a_out_command_status(value);
}





void sg1000_alloc_vram_memory(void)
{
    if (sg1000_vram_memory==NULL) {
        sg1000_vram_memory=malloc(16384);
        if (sg1000_vram_memory==NULL) cpu_panic("Cannot allocate memory for sg1000 vram");
    }
}


z80_byte sg1000_read_vram_byte(z80_int address)
{
    //Siempre leer limitando a 16 kb
    return sg1000_vram_memory[address & 16383];
}



void sg1000_insert_rom_cartridge(char *filename)
{

	debug_printf(VERBOSE_INFO,"Inserting sg1000 rom cartridge %s",filename);

    if (!si_existe_archivo(filename)) {
        debug_printf(VERBOSE_ERR,"File %s not found",filename);
        return;
    }

    long tamanyo_archivo=get_file_size(filename);

    if (tamanyo_archivo>49152) {
        debug_printf(VERBOSE_ERR,"Cartridges bigger than 48K are not allowed");
        return;
    }

    FILE *ptr_cartridge;
    ptr_cartridge=fopen(filename,"rb");

    if (!ptr_cartridge) {
    debug_printf (VERBOSE_ERR,"Unable to open cartridge file %s",filename);
            return;
    }



    int leidos=fread(memoria_spectrum,1,tamanyo_archivo,ptr_cartridge);

    debug_printf(VERBOSE_INFO,"Loaded %d bytes of cartridge rom",leidos);


    fclose(ptr_cartridge);


    if (noautoload.v==0) {
            debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
            reset_cpu();
    }

    sg1000_cartridge_inserted.v=1;


}


void sg1000_empty_romcartridge_space(void)
{

//poner a 0
    int i;
    for (i=0;i<65536;i++) {
        memoria_spectrum[i]=0;
    }

    sg1000_cartridge_inserted.v=0;
}





//Refresco pantalla sin rainbow
void scr_refresca_pantalla_y_border_sg1000_no_rainbow(void)
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
            vdp_9918a_render_ula_no_rainbow(sg1000_vram_memory);
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
        vdp_9918a_render_sprites_no_rainbow(sg1000_vram_memory);
    }




}


void scr_refresca_pantalla_y_border_sg1000(void)
{
    if (rainbow_enabled.v) {
        vdp_9918a_scr_refresca_pantalla_y_border_rainbow();
    }
    else {
        scr_refresca_pantalla_y_border_sg1000_no_rainbow();
    }
}







//Almacenaje temporal de render de la linea actual
z80_int sg1000_scanline_buffer[512];


void screen_store_scanline_rainbow_sg1000_border_and_display(void)
{

    screen_store_scanline_rainbow_vdp_9918a_border_and_display(sg1000_scanline_buffer,sg1000_vram_memory);


}




/*




            Mascara de puertos 0b11000001 = 193 = 0xC1





            //puerto DC =  1101 1100  - mask 0b11000001 (193 decimal) = 1100000000 -> joypad A

 Player 1                        A       B
            up down left right fire/space Z

Player 2
             q   a    o    p     m       n


             A B cont reset

             Z X   C    R


             //Puerto DE = 1101 1110 - mask 0b11000001 = 1100 0000

*/


z80_byte sg1000_get_joypad_a(void)
{


    //si estamos en el menu, no devolver tecla
    if (zxvision_key_not_sent_emulated_mach() ) return 255;

    z80_byte valor_joystick=255;

/*
joypad_a (value after mask 0b11000000 = 192)
JOYPAD2_DOWN:   = 0b10000000;
JOYPAD2_UP:     = 0b01000000;
JOYPAD1_B:      = 0b00100000;
JOYPAD1_A:      = 0b00010000;
JOYPAD1_RIGHT:  = 0b00001000;
JOYPAD1_LEFT:   = 0b00000100;
JOYPAD1_DOWN:   = 0b00000010;
JOYPAD1_UP:     = 0b00000001;
}
*/

//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4

			//z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

			if ((puerto_especial_joystick&1)) valor_joystick &=(255-8);
			if ((puerto_especial_joystick&2)) valor_joystick &=(255-4);
			if ((puerto_especial_joystick&4)) valor_joystick &=(255-2);
			if ((puerto_especial_joystick&8)) valor_joystick &=(255-1);

			if ((puerto_especial_joystick&16)) valor_joystick &=(255-16);

            //Espacio tambien vale como Fire/A
            //puerto_32766    db              255  ; B    N    M    Simb Space ;7
            if ((puerto_32766 & 1)==0) valor_joystick &=(255-16);

            //B = Tecla Z

            //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
            if ((puerto_65278 & 2)==0) valor_joystick &=(255-32);


            //Player 2. Q
            //puerto_64510    db              255  ; T    R    E    W    Q     ;2
            if ((puerto_64510 & 1)==0) valor_joystick &=(255-64);




            //Player 2. A
            //puerto_65022   db    255  ; G    F    D    S    A     ;1
            if ((puerto_65022 & 1)==0) valor_joystick &=(255-128);





    return valor_joystick;
}




z80_byte sg1000_get_joypad_b(void)
{

    //si estamos en el menu, no devolver tecla
    if (zxvision_key_not_sent_emulated_mach() ) return 255;


    z80_byte valor_joystick=255;

/*
value after mask port = 0b11000001 = 193
B_TH:           = 0b10000000;
A_TH:           = 0b01000000;
CONT:           = 0b00100000;
RESET:          = 0b00010000;
JOYPAD2_B:      = 0b00001000;
JOYPAD2_A:      = 0b00000100;
JOYPAD2_RIGHT:  = 0b00000010;
JOYPAD2_LEFT:   = 0b00000001;
}
*/

//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4

			//z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right


            //Player 2. O
            //puerto_57342    db              255  ; Y    U    I    O    P     ;5
            if ((puerto_57342 & 2)==0) valor_joystick &=(255-1);


            //Player 2. P
            //puerto_57342    db              255  ; Y    U    I    O    P     ;5
            if ((puerto_57342 & 1)==0) valor_joystick &=(255-2);


            //Player 2. M
            //puerto_32766    db              255  ; B    N    M    Simb Space ;7
            if ((puerto_32766 & 4)==0) valor_joystick &=(255-4);


            //Player 2. N
            //puerto_32766    db              255  ; B    N    M    Simb Space ;7
            if ((puerto_32766 & 8)==0) valor_joystick &=(255-8);

/*
             A B cont reset

             Z X   C    R
*/

            //Player 2. Reset (R)
            //puerto_64510    db              255  ; T    R    E    W    Q     ;2
            if ((puerto_64510 & 8)==0) valor_joystick &=(255-16);


            //Player 2. Cont (C)
            //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
            if ((puerto_65278 & 8)==0) valor_joystick &=(255-32);


            //A  (Z)
            //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
            if ((puerto_65278 & 2)==0) valor_joystick &=(255-64);

            //B  (X)
            //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
            if ((puerto_65278 & 4)==0) valor_joystick &=(255-128);


    return valor_joystick;
}