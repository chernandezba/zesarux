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

/*
   Sega master system emulation
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sms.h"
#include "vdp_9918a.h"
#include "vdp_9918a_sms.h"
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

z80_byte *sms_vram_memory=NULL;


z80_byte sms_mapper_type=SMS_MAPPER_TYPE_NONE;

z80_byte sms_mapper_FFFC;
z80_byte sms_mapper_FFFD;
z80_byte sms_mapper_FFFE;
z80_byte sms_mapper_FFFF;

//bits significativos
z80_byte sms_mapper_mask_bits=0x3F;



const char *sms_string_memory_type_rom="ROM";
const char *sms_string_memory_type_ram="RAM";
const char *sms_string_memory_type_empty="EMPTY";

z80_bit sms_cartridge_inserted={0};

//Tamanyo del cartucho insertado
int sms_cartridge_size=0;



char *sms_get_string_memory_type(int tipo)
{



    switch (tipo) {

        case SMS_SLOT_MEMORY_TYPE_ROM:
            return (char *)sms_string_memory_type_rom;
        break;

        case SMS_SLOT_MEMORY_TYPE_RAM:
            return (char *)sms_string_memory_type_ram;
        break;

        default:
            return (char *)sms_string_memory_type_empty;
        break;

    }
}


//Retorna direccion de memoria donde esta mapeada la ram y su tipo
z80_byte *sms_return_segment_address(z80_int direccion,int *tipo)
{

    z80_byte bloque_entra;

    switch (sms_mapper_type) {

        case SMS_MAPPER_TYPE_SEGA:

        /*
        Addresses $fffd-$ffff: ROM mapping
        Control register	ROM bank select for slot
        $fffd	0 ($0000-$3fff)
        $fffe	1 ($4000-$7fff)
        $ffff	2 ($8000-$bfff)

        */

        //ROM
        if (direccion<=0xbfff) {
            *tipo=SMS_SLOT_MEMORY_TYPE_ROM;

            if (direccion<=0x3fff) {
                // primer 1kb es siempre bloque 0
                if (direccion<=1023) {
                    bloque_entra=0;
                }
                else {
                    bloque_entra=sms_mapper_FFFD;
                }
            }

            else if (direccion<=0x7fff) {
                bloque_entra=sms_mapper_FFFE;
            }

            else {
                bloque_entra=sms_mapper_FFFF;
            }

            //printf("dir=%d bloque_entra=%d\n",direccion,bloque_entra);

            int offset=(bloque_entra & sms_mapper_mask_bits) * 16384;

            return &memoria_spectrum[offset+(direccion & 16383)];
        }

        //TODO registro FFFC
        /*
        Address $fffc: RAM mapping and miscellaneous functions
Bit	Function
7	"ROM write" enable
6-5	Unused
4	RAM enable ($c000-$ffff)
3	RAM enable ($8000-$bfff)
2	RAM bank select
1-0	Bank shift

        */
        //RAM 8 KB
        else {
            *tipo=SMS_SLOT_MEMORY_TYPE_RAM;

            //total 1 MByte ROM + 8 kb RAM

            //Esto sin mapper:
            return &memoria_spectrum[SMS_MAX_ROM_SIZE + (direccion & 8191)];
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

        break;

        case SMS_MAPPER_TYPE_CODEMASTERS:

        /*
        ROM mapping
        This mapper presents three 16KB slots to the address bus as before.
        However, the control registers are mapped to the first byte of each bank and the first 1KB is no longer protected.
        Control register	Slot
        $0000	0 ($0000-$3fff)
        $4000	1 ($4000-$7fff)
        $8000	2 ($8000-$bfff)
        The mapper is initialised with banks 0, 1 and 0 in slots 0, 1 and 2 respectively.
        When using slot 0, care must be taken to replace interrupt vectors appropriately.
        Also note that there is no RAM mirroring of mapper writes so there is no way to retrieve the last value written.
        */

        //ROM
        if (direccion<=0xbfff) {
            *tipo=SMS_SLOT_MEMORY_TYPE_ROM;

            if (direccion<=0x3fff) {
                bloque_entra=sms_mapper_FFFD;
            }

            else if (direccion<=0x7fff) {
                bloque_entra=sms_mapper_FFFE;
            }

            else {
                bloque_entra=sms_mapper_FFFF;
            }

            //printf("dir=%d bloque_entra=%d\n",direccion,bloque_entra);

            int offset=(bloque_entra & sms_mapper_mask_bits) * 16384;

            return &memoria_spectrum[offset+(direccion & 16383)];
        }

        //TODO registro FFFC
        //RAM 8 KB
        else {
            *tipo=SMS_SLOT_MEMORY_TYPE_RAM;

            //total 1 MByte ROM + 8 kb RAM

            //Esto sin mapper:
            return &memoria_spectrum[SMS_MAX_ROM_SIZE + (direccion & 8191)];
        }

        break;

        //NONE o cualquier otro
        default:

        //El orden dentro de toda la memoria asignada es: primero toda la posible ROM y luego los 8 KB de RAM

        //ROM
        if (direccion<=0xbfff) {
            *tipo=SMS_SLOT_MEMORY_TYPE_ROM;
            return &memoria_spectrum[direccion];
        }

        //RAM 8 KB
        else {
            *tipo=SMS_SLOT_MEMORY_TYPE_RAM;

            //total 1 MByte ROM + 8 kb RAM

            //Esto sin mapper:
            return &memoria_spectrum[SMS_MAX_ROM_SIZE + (direccion & 8191)];
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

        break;



    }




}


void sms_init_memory_tables(void)
{




}


void sms_reset(void)
{

    //Resetear vram
    int i;

    for (i=0;i<16384;i++) sms_vram_memory[i]=0;

    //reset mappers
    sms_mapper_FFFC=0;
    sms_mapper_FFFD=0;
    sms_mapper_FFFE=1;
    sms_mapper_FFFF=2;

    //FFFC=00, FFFD=00, FFFE=01, FFFF=02

}

void sms_out_port_vdp_data(z80_byte value)
{
    if (sms_writing_cram) {
        //printf("Escribiendo cram indice %d valor %d\n",index_sms_escritura_cram & 31,value);

        vdp_9918a_sms_cram[index_sms_escritura_cram & (VDP_9918A_SMS_MODE4_MAPPED_PALETTE_COLOURS-1) ]=value;
        index_sms_escritura_cram++;
    }

    else {
        vdp_9918a_out_vram_data(sms_vram_memory,value);
    }
}


z80_byte sms_in_port_vdp_data(void)
{
    return vdp_9918a_in_vram_data(sms_vram_memory);
}



z80_byte sms_in_port_vdp_status(void)
{
    //Solo nos quedamos con los 3 bits superiores
    //TODO: creo que en todas las maquinas con este VDP deberia ser asi
    return (vdp_9918a_in_vdp_status() & (128+64+32));
}

void sms_out_port_vdp_command_status(z80_byte value)
{
    vdp_9918a_out_command_status(value);
}





void sms_alloc_vram_memory(void)
{
    if (sms_vram_memory==NULL) {
        sms_vram_memory=malloc(16384);
        if (sms_vram_memory==NULL) cpu_panic("Cannot allocate memory for sms vram");
    }
}


z80_byte sms_read_vram_byte(z80_int address)
{
    //Siempre leer limitando a 16 kb
    return sms_vram_memory[address & 16383];
}

void sms_set_mapper_mask_bits(void)
{
    //Ajustar mascara
    //Por defecto. Hasta 64 bloques = 1 MByte
    sms_mapper_mask_bits=0x3F;

    if (sms_cartridge_size<=65536) {
        //4 bloques. hasta 64 KB
        sms_mapper_mask_bits=0x03;
    }

    else if (sms_cartridge_size<=65536*2) {
        //8 bloques. hasta 128 KB
        sms_mapper_mask_bits=0x07;
    }

    else if (sms_cartridge_size<=65536*4) {
        //16 bloques. Hasta 256 KB
        sms_mapper_mask_bits=0x0F;
    }

    else if (sms_cartridge_size<=65536*8) {
        //32 bloques. Hasta 512 KB
        sms_mapper_mask_bits=0x1F;
    }


    //printf("Mapper type: %d mask %d\n",sms_mapper_type,sms_mapper_mask_bits);

}

void sms_set_mapper_type_from_size(void)
{
    //Asumimos no mapper
    sms_mapper_type=SMS_MAPPER_TYPE_NONE;


    //Si mayor de 48kb, mapper type sega
    if (sms_cartridge_size>49152) {
        sms_mapper_type=SMS_MAPPER_TYPE_SEGA;
    }
}

void sms_insert_rom_cartridge(char *filename)
{

	debug_printf(VERBOSE_INFO,"Inserting sms rom cartridge %s",filename);

    if (!si_existe_archivo(filename)) {
        debug_printf(VERBOSE_ERR,"File %s not found",filename);
        return;
    }

    long tamanyo_archivo=get_file_size(filename);


    if (tamanyo_archivo>SMS_MAX_ROM_SIZE) {
        debug_printf(VERBOSE_ERR,"Cartridges bigger than %d KB are not allowed",SMS_MAX_ROM_SIZE/1024);
        return;
    }

    sms_cartridge_size=tamanyo_archivo;


    sms_set_mapper_type_from_size();

    sms_set_mapper_mask_bits();


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

    sms_cartridge_inserted.v=1;


}


void sms_empty_romcartridge_space(void)
{

//poner a 0
//Esto ya no hace falta dado que al hacer set_machine se asigna memoria de nuevo
/*
    int i;
    for (i=0;i<65536;i++) {
        memoria_spectrum[i]=0;
    }
    */

    sms_cartridge_inserted.v=0;

    //Seleccionar de nuevo maquina. Esto resetea y carga de nuevo la rom bios de la master system
    set_machine(NULL);
    cold_start_cpu_registers();
    reset_cpu();

}





//Refresco pantalla sin rainbow
void scr_refresca_pantalla_y_border_sms_no_rainbow(void)
{

    //Renderizar border

    //Si se desactiva el layer de border, lo que hara sera mostrarlo con color 0
    if (border_enabled.v) {
            //ver si hay que refrescar border
            if (modificado_border.v)
            {
                    vdp_9918a_refresca_border();
                    modificado_border.v=0;
            }

    }

    //En no rainbow, realmente las dos capas de tiles van a la misma capa: background
    if (vdp_9918a_force_disable_layer_tile_bg.v==0) {

        //Capa activada. Pero tiene reveal?

        if (vdp_9918a_reveal_layer_tile_bg.v) {
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
            //Render SMS tiles background
            //El parametro vdp_9918a_force_disable_layer_tile_bg aqui realmente no se usa,
            //pues si la capa esta desactivada, ya no entra en esta condicion
            //Se gestiona con la condicion de mas abajo, esto es por compatiblidad con ocultar dicha capa de tile
            //en modos no SMS
            if (vdp_9918a_si_sms_video_mode4()) vdp_9918a_render_ula_no_rainbow_sms(sms_vram_memory,0,vdp_9918a_reveal_layer_tile_bg.v,vdp_9918a_force_disable_layer_tile_bg.v);

            //Render modos no SMS
            else vdp_9918a_render_ula_no_rainbow(sms_vram_memory);

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

    //Capa de foreground mostrada en background?
    //Si es asi, dibujarlos antes que los Sprites
    if (vdp_9918a_si_sms_video_mode4()) {
        if (vdp_9918a_force_bg_tiles.v) {
            vdp_9918a_render_ula_no_rainbow_sms(sms_vram_memory,1,vdp_9918a_reveal_layer_tile_fg.v,vdp_9918a_force_disable_layer_tile_fg.v);
        }
    }

    //Renderizar Sprites

    if (vdp_9918a_force_disable_layer_sprites.v==0) {
        vdp_9918a_render_sprites_no_rainbow(sms_vram_memory);
    }


    //Renderizar tiles foreground. Esto solo para sms modo video 4
    if (vdp_9918a_si_sms_video_mode4()) {
        if (vdp_9918a_force_bg_tiles.v==0) {
            vdp_9918a_render_ula_no_rainbow_sms(sms_vram_memory,1,vdp_9918a_reveal_layer_tile_fg.v,vdp_9918a_force_disable_layer_tile_fg.v);
        }
    }

}


void scr_refresca_pantalla_y_border_sms(void)
{
    if (rainbow_enabled.v) {
        vdp_9918a_scr_refresca_pantalla_y_border_rainbow();
    }
    else {
        scr_refresca_pantalla_y_border_sms_no_rainbow();
    }
}







//Almacenaje temporal de render de la linea actual
z80_int sms_scanline_buffer[512];


void screen_store_scanline_rainbow_sms_border_and_display(void)
{

    screen_store_scanline_rainbow_vdp_9918a_border_and_display(sms_scanline_buffer,sms_vram_memory);


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


z80_byte sms_get_joypad_a(void)
{


    //si estamos en el menu, no devolver tecla
    if (zxvision_key_not_sent_emulated_mach() ) return 255;

    z80_byte valor_joystick=255;

/*
Port $DC: I/O port A and B
Bit	Function
7	Port B Down pin input
6	Port B Up pin input
5	Port A TR pin input
4	Port A TL pin input
3	Port A Right pin input
2	Port A Left pin input
1	Port A Down pin input
0	Port A Up pin input

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

            //Z tambien vale como Fire/A
            //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
            if ((puerto_65278 & 2)==0) valor_joystick &=(255-16);

            //Boton 2 = Tecla X

            //puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
            if ((puerto_65278 & 4)==0) valor_joystick &=(255-32);




            //Player 2. Q
            //puerto_64510    db              255  ; T    R    E    W    Q     ;2
            if ((puerto_64510 & 1)==0) valor_joystick &=(255-64);




            //Player 2. A
            //puerto_65022   db    255  ; G    F    D    S    A     ;1
            if ((puerto_65022 & 1)==0) valor_joystick &=(255-128);





    return valor_joystick;
}




z80_byte sms_get_joypad_b(void)
{

    //si estamos en el menu, no devolver tecla
    if (zxvision_key_not_sent_emulated_mach() ) return 255;


    z80_byte valor_joystick=255;

/*
Port $DD: I/O port B and miscellaneous
Bit	Function
7	Port B TH pin input
6	Port A TH pin input
5	Cartridge slot CONT pin *
4	Reset button (1= not pressed, 0= pressed) *
3	Port B TR pin input
2	Port B TL pin input
1	Port B Right pin input
0	Port B Left pin input

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


            //Player 2. N
            //puerto_32766    db              255  ; B    N    M    Simb Space ;7
            if ((puerto_32766 & 8)==0) valor_joystick &=(255-4);

            //Player 2. M
            //puerto_32766    db              255  ; B    N    M    Simb Space ;7
            if ((puerto_32766 & 4)==0) valor_joystick &=(255-8);

/*
             A B cont reset

             Z X   C    R
*/

/*
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
*/

    return valor_joystick;
}