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


#include "vdp_9918a_sms.h"
#include "vdp_9918a.h"
#include "cpu.h"
#include "debug.h"
#include "screen.h"
#include "settings.h"


//Rutinas del VDP de Sega Master System que extiende el vdp 9918a
//La mayoría de cosas de aquí solo se aplican cuando se activa el modo 4 de la SMS

int sms_writing_cram=0;

z80_byte index_sms_escritura_cram=0;

z80_byte vdp_9918a_sms_cram[VDP_9918A_SMS_MODE4_MAPPED_PALETTE_COLOURS];


//Forzar siempre columna 0
z80_bit vdp_9918a_sms_force_show_column_zero={0};

//Bloqueo de scrolls
z80_bit vdp_9918a_sms_lock_scroll_horizontal={0};
z80_bit vdp_9918a_sms_lock_scroll_vertical={0};

//Desactivar/Reveal de capas de sms

z80_bit vdp_9918a_force_disable_layer_tile_bg={0};
z80_bit vdp_9918a_force_disable_layer_tile_fg={0};
z80_bit vdp_9918a_reveal_layer_tile_fg={0};
z80_bit vdp_9918a_reveal_layer_tile_bg={0};

//Decimos que los tiles en foreground (con priority) pasen a background
z80_bit vdp_9918a_force_bg_tiles={0};

z80_bit sms_disable_raster_interrupt={0};

//Setting para decir solo disparar 1 interrupcion por frame
z80_bit sms_only_one_raster_int_frame={0};

const char *s_vdp_9918a_video_mode_sms_4="4 - SMS Graphic 256x192";

//Siguiente valor para scroll vertical que se actualiza a final de frame
/*
 The vertical scroll value cannot be changed during the active display
 period, any changes made will be stored in a temporary location and
 used only when the active display period ends (prematurely blanking the
 screen with bit #6 of register #1 doesn't count).

*/
z80_byte sms_next_scroll_vertical_value=0;

//int sms_pending_line_interrupt=0;

//Fix feo para el scroll del wonderboy
z80_bit sms_wonderboy_scroll_hack={0};

int vdp_9918a_sms_get_cram_color(int index)
{
    return vdp_9918a_sms_cram[index % VDP_9918A_SMS_MODE4_MAPPED_PALETTE_COLOURS];
}



void vdp_9918a_sms_reset(void)
{
    int i;
    //Y resetear tabla de colores de sms
    //16+16 colores (tiles + sprites)

    for (i=0;i<2;i++) {

        vdp_9918a_sms_cram[0+i*16]=0x00;
        vdp_9918a_sms_cram[1+i*16]=0x00;
        vdp_9918a_sms_cram[2+i*16]=0x08;
        vdp_9918a_sms_cram[3+i*16]=0x0C;
        vdp_9918a_sms_cram[4+i*16]=0x10;
        vdp_9918a_sms_cram[5+i*16]=0x30;
        vdp_9918a_sms_cram[6+i*16]=0x01;
        vdp_9918a_sms_cram[7+i*16]=0x3C;
        vdp_9918a_sms_cram[8+i*16]=0x02;
        vdp_9918a_sms_cram[9+i*16]=0x03;
        vdp_9918a_sms_cram[10+i*16]=0x05;
        vdp_9918a_sms_cram[11+i*16]=0x0f;
        vdp_9918a_sms_cram[12+i*16]=0x04;
        vdp_9918a_sms_cram[13+i*16]=0x33;
        vdp_9918a_sms_cram[14+i*16]=0x15;
        vdp_9918a_sms_cram[15+i*16]=0x3f;

    }

    sms_next_scroll_vertical_value=0;
    //sms_pending_line_interrupt=0;

    //En SMS los registros se resetean asi:
    vdp_9918a_registers[0]=0x36;
    vdp_9918a_registers[1]=0xA0;
    vdp_9918a_registers[2]=0xFF;
    vdp_9918a_registers[3]=0xFF;
    vdp_9918a_registers[4]=0xFF;
    vdp_9918a_registers[5]=0xFF;
    vdp_9918a_registers[6]=0xFB;
    vdp_9918a_registers[7]=0x00;
    vdp_9918a_registers[8]=0x00;
    vdp_9918a_registers[9]=0x00;
    vdp_9918a_registers[10]=0xFF;

}

int vdp_9918a_si_sms_video_mode4(void)
{
    if (MACHINE_IS_SMS && (vdp_9918a_registers[0] & 4)) return 1;

    else return 0;
}

z80_int vdp_9918a_get_sprite_pattern_table_sms_mode4(void)
{
    z80_int sprite_pattern_table=(vdp_9918a_registers[6] & 4) * 0x800;

    return sprite_pattern_table;
}

z80_int vdp_9918a_sms_get_pattern_name_table(void)
{

    return (vdp_9918a_registers[2]&14) * 0x400;

}

z80_byte vdp_9918a_sms_get_scroll_horizontal(void)
{
    return vdp_9918a_registers[8];
}

z80_byte vdp_9918a_sms_get_scroll_vertical(void)
{
    return vdp_9918a_registers[9];
}

//Dice si renderizamos la parte de tiles de foreground (1) o background (0)
void vdp_9918a_render_ula_no_rainbow_sms(z80_byte *vram,int render_tiles_foreground,int reveal,int forzada_negro)
{
    //Modo 4 SMS. high-res mode, 256x192


	int x,y,bit;
	z80_int direccion_name_table;
    z80_byte byte_color;

	z80_int pattern_base_address;
	z80_int pattern_name_table;

	int char_width=8;



    z80_byte byte_leido1,byte_leido2,byte_leido3,byte_leido4;

    pattern_name_table=vdp_9918a_get_pattern_name_table();

    pattern_base_address=vdp_9918a_get_pattern_base_address();


    direccion_name_table=pattern_name_table;

    //scroll y
    z80_byte scroll_y=vdp_9918a_sms_get_scroll_vertical();

    if (vdp_9918a_sms_lock_scroll_vertical.v) scroll_y=0;


    //fila
    /*
    Register $09 can be divided into two parts, the upper five bits are the
starting row, and the lower three bits are the fine scroll value.
.
    */

    z80_byte fila_scroll_y=((scroll_y>>3)&31);

    z80_byte scroll_y_sublinea=scroll_y&7;

    //TODO: la gestion de scroll fino en vertical y horizontal en modo rainbow, sera
    //algo distinto de este no rainbow (o deberia ser)

    //1 fila mas si hay scroll vertical
    int total_filas=24;

    if (scroll_y_sublinea) total_filas++;

    for (y=0;y<total_filas;y++) {

        //Maximo 28 en Y
        z80_byte final_y=(y+fila_scroll_y) % 28;

            //scroll x
            z80_byte scroll_x=vdp_9918a_sms_get_scroll_horizontal();

            if (vdp_9918a_sms_lock_scroll_horizontal.v) scroll_x=0;

            /*
                If bit #6 of VDP register $00 is set, horizontal scrolling will be fixed
at zero for scanlines zero through 15. This is commonly used to create
a fixed status bar at the top of the screen for horizontally scrolling
games.

Register $00 - Mode Control No. 1

D7 - 1= Disable vertical scrolling for columns 24-31
D6 - 1= Disable horizontal scrolling for rows 0-1
D5 - 1= Mask column 0 with overscan color from register #7
D4 - (IE1) 1= Line interrupt enable
D3 - (EC) 1= Shift sprites left by 8 pixels
D2 - (M4) 1= Use Mode 4, 0= Use TMS9918 modes (selected with M1, M2, M3)
D1 - (M2) Must be 1 for M1/M3 to change screen height in Mode 4.
Otherwise has no effect.
D0 - 1= No sync, display is monochrome, 0= Normal display

            */

            //Las 2 primeras lineas no tendran scroll si bit D6
            //Esto lo usa juego Astro Flash
            if (vdp_9918a_registers[0] & 64 && y<2) scroll_x=0;


            int columna_scroll_x;
            z80_byte scroll_x_fino;


            //scroll_x_fino=(255-scroll_x) & 7;

            //En alguna documentacion he leido que esto se restaba de 32, en vez de 31, cosa que no tiene sentido:
            //en sonic por ejemplo provocaria que cuando se hace el primer desplazamiento, salta la columna de golpe a 1,
            //en vez de simplemente quedarse en columna 0 aunque scroll_x_fino logicamente pasa a 1


//columna
            /*
                The starting column value gives the first column in the name table to use,
calculated by subtracting it from the value 32. So if the starting column
value was $1D, the difference of it from 32 would be $02, hence the first
column drawn is number 2 from the name table.
            */

            //Creo que esto es la manera mas logica pese a la documentación

            z80_byte scroll_negado=256-scroll_x;

            scroll_x_fino=scroll_negado&7;

            columna_scroll_x=(scroll_negado >>3)&31;






            //TODO: la gestion de scroll fino en vertical y horizontal en modo rainbow, sera
            //algo distinto de este no rainbow (o deberia ser)

            //1 columna mas si hay scroll horizontal

            int total_columnas=32;

            if (scroll_x_fino) total_columnas++;

        //printf("%d\n",total_columnas);

        for (x=0;x<total_columnas;x++) {



            int final_x;


            final_x=(x+columna_scroll_x) % 32;



            int offset_tile=final_x*2+final_y*64;
            direccion_name_table=pattern_name_table+offset_tile;


            /*if (y==0 && x>=25) {
                printf("x %d scroll reg: %d columna_scroll_x %d scroll_x_fino %d final_x %d offset_tile %d\n",
                    x,vdp_9918a_sms_get_scroll_horizontal(),columna_scroll_x,scroll_x_fino,final_x,offset_tile);
            }*/
            //if (y==0 && x==28) printf("---\n");

            z80_int pattern_word=vdp_9918a_read_vram_byte(vram,direccion_name_table)+256*vdp_9918a_read_vram_byte(vram,direccion_name_table+1);


            /*
            MSB          LSB
---pcvhn nnnnnnnn

- = Unused. Some games use these bits as flags for collision and damage
zones. (such as Wonderboy in Monster Land, Zillion 2)
p = Priority flag. When set, sprites will be displayed underneath the
background pattern in question.
c = Palette select.
v = Vertical flip flag.
h = Horizontal flip flag.
n = Pattern index, any one of 512 patterns in VRAM can be selected.
            */

            int mirror_x=(pattern_word & 0x0200);

            int mirror_y=(pattern_word & 0x0400);

            z80_int caracter=pattern_word & 511;

            int palette_offset=(pattern_word & 0x0800 ? 16 : 0);

            int priority_tile_bit=(pattern_word & 0x1000);

            int scanline;

            //z80_int pattern_address=(caracter*32+2048*tercio) ;
            z80_int pattern_address=(caracter*32) ; //32 bytes cada tile
            pattern_address +=pattern_base_address;


            //Si tiene mirror vertical, empezamos con la ultima linea del pattern
            if (mirror_y) {
                pattern_address +=(8*4)-4;
            }



            for (scanline=0;scanline<8;scanline++) {

                //TODO: no se si esta es la mejor manera de gestionar el scroll
                int ydestino=y*8+scanline-scroll_y_sublinea;
                //printf("%d\n",ydestino);

                byte_leido1=vdp_9918a_read_vram_byte(vram,pattern_address);
                byte_leido2=vdp_9918a_read_vram_byte(vram,pattern_address+1);
                byte_leido3=vdp_9918a_read_vram_byte(vram,pattern_address+2);
                byte_leido4=vdp_9918a_read_vram_byte(vram,pattern_address+3);

                if (mirror_y) {
                    pattern_address -=4;
                }
                else {
                    pattern_address +=4;
                }

                //No dibujar si y < 0. Esto sucede cuando se aplica scroll vertical

                //Similar para mayor de 192 cuando hay scroll y hacemos 25 filas (parte de la ultima 25)
                if (ydestino>=0 && ydestino<=191) {

                for (bit=0;bit<char_width;bit++) {

                    int priority_pixel=priority_tile_bit;

                    int xdestino=x*char_width+bit-scroll_x_fino;

                    if (mirror_x) {
                        byte_color=((byte_leido1)&1) | ((byte_leido2<<1)&2) | ((byte_leido3<<2)&4) | ((byte_leido4<<3)&8);
                    }
                    else {

                        byte_color=((byte_leido1>>7)&1) | ((byte_leido2>>6)&2) | ((byte_leido3>>5)&4) | ((byte_leido4>>4)&8);

                    }

                    //Si color 0 y capa en foreground, se pasa a background
                        /*
                        When a tile has its priority bit set, all pixels with index greater than 0
                        will be drawn on top of sprites. You must therefore choose a single colour
                        in palette position 0 to be the background colour for such tiles, and they
                        will have a "blank" background. Careful use of tile priority can make the
                        graphics seem more multi-layered.
                        */
                    if (priority_pixel && byte_color==0) priority_pixel=0;


                    int color_paleta=vdp_9918a_sms_cram[palette_offset+ (byte_color & 15)];

                    //maximo 64 colores de paleta
                    color_paleta &=63;




                    //No dibujar si x < 0. Esto sucede cuando se aplica scroll horizontal
                    //Similar para mayor de 255 cuando hay scroll x  y  hacemos 33 filas (parte de la ultima 33)


                    if (xdestino>=0 && xdestino<=255) {

                        //Register $00 - Mode Control No. 1
                        //D5 - 1= Mask column 0 with overscan color from register #7
                        //Esto lo usa sonic. La primera columna es para usar para el scroll
                        if (xdestino<=7 && (vdp_9918a_registers[0] & 32)) {
                            //TODO: que color? debe ser el del border
                            //TODO: esto creo que aplica tambien a sprites, en sonic por ejemplo los enemigos que se van
                            //por la izquierda se ven en esta columna oculta

                            //Y no ocultarlo si tenemos el setting de mostrar forzado columna 0
                            if (vdp_9918a_sms_force_show_column_zero.v==0) {
                                color_paleta=vdp_9918a_sms_get_cram_color(vdp_9918a_get_border_color());
                            }
                        }

                        if (reveal) {
                            int posx=xdestino&1;
                            int posy=ydestino&1;

                            //0,0: 0
                            //0,1: 1
                            //1,0: 1
                            //1,0: 0
                            //Es un xor

                            int si_blanco_negro=posx ^ posy;

                            //Color 0 o 63 (negro / blanco)
                            color_paleta=si_blanco_negro*(SMS_TOTAL_PALETTE_COLOURS-1);
                        }

                        int dibujar=0;
                        if (priority_pixel && render_tiles_foreground) dibujar=1;
                        if (!priority_pixel && !render_tiles_foreground) dibujar=1;

                        //Si capa forzada a negro
                        if (forzada_negro) color_paleta=0;

                        if (dibujar) scr_putpixel_zoom(xdestino,ydestino,SMS_INDEX_FIRST_COLOR+color_paleta);
                    }


                    if (mirror_x) {
                        byte_leido1=byte_leido1>>1;
                        byte_leido2=byte_leido2>>1;
                        byte_leido3=byte_leido3>>1;
                        byte_leido4=byte_leido4>>1;
                    }

                    else {
                        byte_leido1=byte_leido1<<1;
                        byte_leido2=byte_leido2<<1;
                        byte_leido3=byte_leido3<<1;
                        byte_leido4=byte_leido4<<1;
                    }


                }

                }

            }


        }


    }


}

int vdp_9918a_sms_get_sprite_height(void)
{
    /*
     Register $01 - Mode Control No. 2

 D7 - No effect
 D6 - (BLK) 1= Display visible, 0= display blanked.
 D5 - (IE0) 1= Frame interrupt enable.
 D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
 D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
 D2 - No effect
 D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
 D0 - Sprite pixels are doubled in size.
    */


    return (vdp_9918a_registers[1] & 2 ? 16 : 8);



}


//Render sprites en modo 4 Sega Master System
void vdp_9918a_render_sprites_sms_video_mode4_no_rainbow(z80_byte *vram)
{

    z80_int sprite_pattern_table=vdp_9918a_get_sprite_pattern_table_sms_mode4();

    z80_byte byte_leido1,byte_leido2,byte_leido3,byte_leido4;


    //int sprite_size=vdp_9918a_get_sprite_size();
    int sprite_double=vdp_9918a_get_sprite_double();



    //TODO temp
    //sprite_size=8;
    sprite_double=1;

/*
TODO
 Register $01 - Mode Control No. 2

 D7 - No effect
 D6 - (BLK) 1= Display visible, 0= display blanked.
 D5 - (IE0) 1= Frame interrupt enable.
 D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
 D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
 D2 - No effect
 D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
 D0 - Sprite pixels are doubled in size.
 */

    //printf ("Sprite size: %d double: %d\n",sprite_size,sprite_double);
    //printf("Sprite size: 8x%d\n",(vdp_9918a_registers[1] & 2 ? 16 : 8));


    int sprite_height=vdp_9918a_sms_get_sprite_height();

    //TODO: si coordenada Y=208, fin tabla sprites
    //    z80_int sprite_attribute_table=(vdp_9918a_registers[5]) * 0x80;

    //z80_int sprite_pattern_table=(vdp_9918a_registers[6]) * 0x800;

    int sprite;
    int salir=0;

    //En boundary de 128
    //sprite_attribute_table &=(65535-128);

    z80_int sprite_attribute_table=vdp_9918a_get_sprite_attribute_table();

    //Empezar por la del final
    //Ver si hay alguno con coordenada 208 que indica final

    int primer_sprite_final; //=VDP_9918A_SMS_MODE4_MAX_SPRITES-1;


    // buscar ultimo sprite
    /*
        If the Y coordinate is set to $D0, then the sprite in question and all
remaining sprites of the 64 available will not be drawn. This only works
in the 192-line display mode, in the 224 and 240-line modes a Y coordinate
of $D0 has no special meaning.
    */

    for (primer_sprite_final=0;primer_sprite_final<VDP_9918A_SMS_MODE4_MAX_SPRITES && !salir;primer_sprite_final++) {
        int offset_sprite=sprite_attribute_table+primer_sprite_final;

        z80_byte vert_pos=vdp_9918a_read_vram_byte(vram,offset_sprite);
        if (vert_pos==208) salir=1;

    }


    //Siempre estara al siguiente
    primer_sprite_final--;



        //Empezar desde final hacia principio
/*
 Each sprite is defined in the sprite attribute table (SAT), a 256-byte
 table located in VRAM. The SAT has the following layout:

    00: yyyyyyyyyyyyyyyy
    10: yyyyyyyyyyyyyyyy
    20: yyyyyyyyyyyyyyyy
    30: yyyyyyyyyyyyyyyy
    40: ????????????????
    50: ????????????????
    60: ????????????????
    70: ????????????????
    80: xnxnxnxnxnxnxnxn
    90: xnxnxnxnxnxnxnxn
    A0: xnxnxnxnxnxnxnxn
    B0: xnxnxnxnxnxnxnxn
    C0: xnxnxnxnxnxnxnxn
    D0: xnxnxnxnxnxnxnxn
    E0: xnxnxnxnxnxnxnxn
    F0: xnxnxnxnxnxnxnxn

 y = Y coordinate + 1
 x = X coordinate
 n = Pattern index
 ? = Unused

*/

    for (sprite=primer_sprite_final;sprite>=0;sprite--) {
        int vert_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+sprite);
        //printf("sprite %d pos %d\n",sprite,sprite_attribute_table+sprite);

        int horiz_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+0x80+sprite*2);
        z80_byte sprite_name=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+0x80+sprite*2+1);


        //printf("Sprite %d Pattern %d X %d Y %d\n",sprite,sprite_name,horiz_pos,vert_pos);

        /*
        TODO
            The pattern index selects one of 256 patterns to use. Bit 2 of register #6
acts like a global bit 8 in addition to this value, allowing sprite patterns
to be taken from the first 256 or last 256 of the 512 available patterns.
        */



        vert_pos++; //255->coordenada 0
        if (vert_pos==256) vert_pos=0;

        //Entre 255 y 256-32-> son coordenadas negativas
        if (vert_pos>=256-32) {
            //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);
            //printf ("Sprite Y negative: %d\n",vert_pos-256);
            vert_pos=vert_pos-256;

        }

        //Siguiente sprite. El precedente
        //sprite_attribute_table -=4;

        //Si early clock, x-=32

        /*if (attr_color_etc & 128) {
            //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);
            horiz_pos -=32;
        }*/

        //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);




        //Si coord Y no esta en el borde inferior
        if (vert_pos<192) {
            //int offset_pattern_table=sprite_name*bytes_per_sprite+sprite_pattern_table;
                int offset_pattern_table=sprite_name*32+sprite_pattern_table;
            //z80_byte color=attr_color_etc & 15;

            int x,y;



            //Sprites de 8x16 y 8x8


                for (y=0;y<sprite_height;y++) {

                    byte_leido1=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                    byte_leido2=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                    byte_leido3=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                    byte_leido4=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);


                    for (x=0;x<8;x++) {

                        int pos_x_final;
                        int pos_y_final;

                        pos_x_final=horiz_pos+x*sprite_double;
                        pos_y_final=vert_pos+y*sprite_double;

                        if (pos_x_final>=0 && pos_x_final<=255 && pos_y_final>=0 && pos_y_final<=191) {

                            //Si bit a 1
                            if (1) {
                                //Y si ese color no es transparente

                                //TODO
                                //if (color!=0) {
                                if (1) {
                                    //printf ("putpixel sprite x %d y %d\n",pos_x_final,pos_y_final);

                                    z80_byte byte_color=((byte_leido1>>7)&1) | ((byte_leido2>>6)&2) | ((byte_leido3>>5)&4) | ((byte_leido4>>4)&8);



                                    //TODO: segunda paleta??
                                    z80_byte color_sprite=vdp_9918a_sms_cram[16 + (byte_color & 15)];

                                    //maximo 64 colores de paleta
                                    color_sprite &=63;

                                    if (vdp_9918a_reveal_layer_sprites.v) {
                                        int posx=pos_x_final&1;
                                        int posy=pos_y_final&1;

                                        //0,0: 0
                                        //0,1: 1
                                        //1,0: 1
                                        //1,0: 0
                                        //Es un xor

                                        int si_blanco_negro=posx ^ posy;

                                        //Color 0 o 63 (negro / blanco)
                                        color_sprite=si_blanco_negro*(SMS_TOTAL_PALETTE_COLOURS-1);
                                    }

                                    //transparencia
                                    if (byte_color!=0) {

                                        //if (x==0 && y==0) printf("Dibujando sprite %d\n",sprite);


                                        int mostrar=1;
                                        //Si ocultar primera columna
                                        if (pos_x_final<=7 && (vdp_9918a_registers[0] & 32)) {
                                            //en sonic por ejemplo los enemigos que se van
                                            //por la izquierda se ven en esta columna oculta

                                            //Y no ocultarlo si tenemos el setting de mostrar forzado columna 0
                                            if (vdp_9918a_sms_force_show_column_zero.v==0) {
                                                mostrar=0;
                                            }
                                        }



                                        if (mostrar) {
                                            scr_putpixel_zoom(pos_x_final,  pos_y_final,  SMS_INDEX_FIRST_COLOR+color_sprite);
                                            if (sprite_double==2) {
                                                scr_putpixel_zoom(pos_x_final+1,  pos_y_final,    SMS_INDEX_FIRST_COLOR+color_sprite);
                                                scr_putpixel_zoom(pos_x_final,    pos_y_final+1,  SMS_INDEX_FIRST_COLOR+color_sprite);
                                                scr_putpixel_zoom(pos_x_final+1,  pos_y_final+1,  SMS_INDEX_FIRST_COLOR+color_sprite);
                                            }
                                        }

                                    }
                                }
                            }
                        }



                        byte_leido1=byte_leido1<<1;
                        byte_leido2=byte_leido2<<1;
                        byte_leido3=byte_leido3<<1;
                        byte_leido4=byte_leido4<<1;
                    }


                }


        }


    }


}



//Renderizado pixeles en modo 4 de sms en rainbow

//scanline_buffer en este caso es la capa de tiles de background

void vdp_9918a_render_rainbow_display_line_sms(int scanline,z80_int *scanline_buffer,z80_int *scanline_buffer_foreground,z80_byte *vram)
{
   //Modo 4 SMS. high-res mode, 256x192
    z80_int *destino_scanline_buffer;
    //printf("border: %d\n",screen_total_borde_izquierdo);
    destino_scanline_buffer=&scanline_buffer[screen_total_borde_izquierdo];

    z80_int *destino_scanline_buffer_foreground;
    //printf("border: %d\n",screen_total_borde_izquierdo);
    destino_scanline_buffer_foreground=&scanline_buffer_foreground[screen_total_borde_izquierdo];

	int x,bit;
	z80_int direccion_name_table;
    z80_byte byte_color;

	z80_int pattern_base_address;
	z80_int pattern_name_table;


    pattern_base_address=vdp_9918a_get_pattern_base_address();



	int char_width=8;



    z80_byte byte_leido1,byte_leido2,byte_leido3,byte_leido4;





    pattern_name_table=vdp_9918a_get_pattern_name_table();


    direccion_name_table=pattern_name_table;


    //scroll y
    z80_byte scroll_y_total=vdp_9918a_sms_get_scroll_vertical();

    if (vdp_9918a_sms_lock_scroll_vertical.v) scroll_y_total=0;

    //z80_byte scroll_y_signo=256-scroll_y;

    scanline +=scroll_y_total;

    // In the regular 192-line display mode, the name table is 32x28 so the vertical scroll register wraps past 223
    scanline = scanline  % 224;


    //El tratamiento del scroll en vertical es ligeramente diferente aqui en funcion rainbow o en funcion no rainbow



    //Sumar el offset por linea

    int y=scanline/8;




    //fila
    /*
    Register $09 can be divided into two parts, the upper five bits are the
starting row, and the lower three bits are the fine scroll value.
.
    */

    //z80_byte fila_scroll_y=((scroll_y>>3)&31);

    //z80_byte scroll_y_sublinea=scroll_y&7;

    //entre 0 y 7 dentro de la fila



    int scanline_fila=scanline % 8;


    //TODO: la gestion de scroll fino en vertical y horizontal en modo rainbow, sera
    //algo distinto de este no rainbow (o deberia ser)

    //1 fila mas si hay scroll vertical
    //int total_filas=24;

    //if (scroll_y_sublinea) total_filas++;

    //for (y=0;y<total_filas;y++) {


        //Maximo 28 en Y
        z80_byte final_y=(y+0) % 28;

        //printf("y: %d final_y: %d\n",y,final_y);

            //scroll x
            z80_byte scroll_x=vdp_9918a_sms_get_scroll_horizontal();

            if (vdp_9918a_sms_lock_scroll_horizontal.v) scroll_x=0;

            /*
                If bit #6 of VDP register $00 is set, horizontal scrolling will be fixed
at zero for scanlines zero through 15. This is commonly used to create
a fixed status bar at the top of the screen for horizontally scrolling
games.

Register $00 - Mode Control No. 1

D7 - 1= Disable vertical scrolling for columns 24-31
D6 - 1= Disable horizontal scrolling for rows 0-1
D5 - 1= Mask column 0 with overscan color from register #7
D4 - (IE1) 1= Line interrupt enable
D3 - (EC) 1= Shift sprites left by 8 pixels
D2 - (M4) 1= Use Mode 4, 0= Use TMS9918 modes (selected with M1, M2, M3)
D1 - (M2) Must be 1 for M1/M3 to change screen height in Mode 4.
Otherwise has no effect.
D0 - 1= No sync, display is monochrome, 0= Normal display

            */

            //Las 2 primeras lineas no tendran scroll si bit D6
            //Esto lo usa juego Astro Flash
            if (vdp_9918a_registers[0] & 64 && y<2) scroll_x=0;


            int columna_scroll_x;
            z80_byte scroll_x_fino;


            //scroll_x_fino=(255-scroll_x) & 7;

            //En alguna documentacion he leido que esto se restaba de 32, en vez de 31, cosa que no tiene sentido:
            //en sonic por ejemplo provocaria que cuando se hace el primer desplazamiento, salta la columna de golpe a 1,
            //en vez de simplemente quedarse en columna 0 aunque scroll_x_fino logicamente pasa a 1


//columna
            /*
                The starting column value gives the first column in the name table to use,
calculated by subtracting it from the value 32. So if the starting column
value was $1D, the difference of it from 32 would be $02, hence the first
column drawn is number 2 from the name table.
            */

            //Creo que esto es la manera mas logica pese a la documentación

            z80_byte scroll_negado=256-scroll_x;

            scroll_x_fino=scroll_negado&7;

            columna_scroll_x=(scroll_negado >>3)&31;






            //TODO: scroll_x fino se ve raro en boot de master system por ejemplo

            //1 columna mas si hay scroll horizontal

            int total_columnas=32;

            if (scroll_x_fino) total_columnas++;

        //printf("%d\n",total_columnas);

        for (x=0;x<total_columnas;x++) {



            int final_x;


            final_x=(x+columna_scroll_x) % 32;



            int offset_tile=final_x*2+final_y*64;
            direccion_name_table=pattern_name_table+offset_tile;


            /*if (y==0 && x>=25) {
                printf("x %d scroll reg: %d columna_scroll_x %d scroll_x_fino %d final_x %d offset_tile %d\n",
                    x,vdp_9918a_sms_get_scroll_horizontal(),columna_scroll_x,scroll_x_fino,final_x,offset_tile);
            }*/
            //if (y==0 && x==28) printf("---\n");

            z80_int pattern_word=vdp_9918a_read_vram_byte(vram,direccion_name_table)+256*vdp_9918a_read_vram_byte(vram,direccion_name_table+1);


            /*
            MSB          LSB
---pcvhn nnnnnnnn

- = Unused. Some games use these bits as flags for collision and damage
zones. (such as Wonderboy in Monster Land, Zillion 2)
p = Priority flag. When set, sprites will be displayed underneath the
background pattern in question.
c = Palette select.
v = Vertical flip flag.
h = Horizontal flip flag.
n = Pattern index, any one of 512 patterns in VRAM can be selected.
            */

            int mirror_x=(pattern_word & 0x0200);

            int mirror_y=(pattern_word & 0x0400);

            z80_int caracter=pattern_word & 511;

            int palette_offset=(pattern_word & 0x0800 ? 16 : 0);

            int priority_tile_bit=(pattern_word & 0x1000);

            if (vdp_9918a_force_bg_tiles.v) priority_tile_bit=0;


            //z80_int pattern_address=(caracter*32+2048*tercio) ;
            z80_int pattern_address=(caracter*32) ; //32 bytes cada tile
            pattern_address +=pattern_base_address;



            //scroll_y_sublinea

            //Si tiene mirror vertical, empezamos con la ultima linea del pattern
            if (mirror_y) {
                pattern_address +=(7-scanline_fila)*4;
            }
            else {
                //Y sumar el scanline. Cada linea de tile son 4 bytes
                pattern_address +=scanline_fila*4;
            }



            //for (scanline=0;scanline<8;scanline++) {

                //TODO: no se si esta es la mejor manera de gestionar el scroll
                //int ydestino=y*8+scanline-scroll_y_sublinea;
                //printf("%d\n",ydestino);

                byte_leido1=vdp_9918a_read_vram_byte(vram,pattern_address);
                byte_leido2=vdp_9918a_read_vram_byte(vram,pattern_address+1);
                byte_leido3=vdp_9918a_read_vram_byte(vram,pattern_address+2);
                byte_leido4=vdp_9918a_read_vram_byte(vram,pattern_address+3);

                if (mirror_y) {
                    pattern_address -=4;
                }
                else {
                    pattern_address +=4;
                }

                //No dibujar si y < 0. Esto sucede cuando se aplica scroll vertical

                //Similar para mayor de 192 cuando hay scroll y hacemos 25 filas (parte de la ultima 25)
                //if (ydestino>=0 && ydestino<=191) {

                for (bit=0;bit<char_width;bit++) {

                    int priority_pixel=priority_tile_bit;

                    int xdestino=x*char_width+bit-scroll_x_fino;

                    if (mirror_x) {
                        byte_color=((byte_leido1)&1) | ((byte_leido2<<1)&2) | ((byte_leido3<<2)&4) | ((byte_leido4<<3)&8);
                    }
                    else {

                        byte_color=((byte_leido1>>7)&1) | ((byte_leido2>>6)&2) | ((byte_leido3>>5)&4) | ((byte_leido4>>4)&8);

                    }

                    //Si color 0 y capa en foreground, se pasa a background
                        /*
                        When a tile has its priority bit set, all pixels with index greater than 0
                        will be drawn on top of sprites. You must therefore choose a single colour
                        in palette position 0 to be the background colour for such tiles, and they
                        will have a "blank" background. Careful use of tile priority can make the
                        graphics seem more multi-layered.
                        */
                    if (priority_pixel && byte_color==0) priority_pixel=0;


                    int color_paleta=vdp_9918a_sms_cram[palette_offset+ (byte_color & 15)];

                    //maximo 64 colores de paleta
                    color_paleta &=63;




                    //No dibujar si x < 0. Esto sucede cuando se aplica scroll horizontal
                    //Similar para mayor de 255 cuando hay scroll x  y  hacemos 33 filas (parte de la ultima 33)


                    if (xdestino>=0 && xdestino<=255) {

                        //Register $00 - Mode Control No. 1
                        //D5 - 1= Mask column 0 with overscan color from register #7
                        //Esto lo usa sonic. La primera columna es para usar para el scroll
                        if (xdestino<=7 && (vdp_9918a_registers[0] & 32)) {
                            //TODO: que color? debe ser el del border
                            //TODO: esto creo que aplica tambien a sprites, en sonic por ejemplo los enemigos que se van
                            //por la izquierda se ven en esta columna oculta

                            //Y no ocultarlo si tenemos el setting de mostrar forzado columna 0
                            if (vdp_9918a_sms_force_show_column_zero.v==0) {
                                color_paleta=vdp_9918a_sms_get_cram_color(vdp_9918a_get_border_color());
                            }
                        }

                        //printf("xdestino: %d\n",xdestino);
                        //scr_putpixel_zoom(xdestino,ydestino,SMS_INDEX_FIRST_COLOR+color_paleta);
                        //Tile con prioridad (arriba) y solo cuando su color no es 0

                        if (priority_pixel) {
                            if (vdp_9918a_force_disable_layer_tile_fg.v==0) {
                                if (vdp_9918a_reveal_layer_tile_fg.v) {
                                    int posx=xdestino&1;
                                    int posy=scanline&1;

                                    //0,0: 0
                                    //0,1: 1
                                    //1,0: 1
                                    //1,0: 0
                                    //Es un xor

                                    int si_blanco_negro=posx ^ posy;

                                    //Color 0 o 63 (negro / blanco)
                                    color_paleta=si_blanco_negro*(SMS_TOTAL_PALETTE_COLOURS-1);
                                }

                                *destino_scanline_buffer_foreground=SMS_INDEX_FIRST_COLOR+color_paleta;


                            }
                        }

                        else {
                            if (vdp_9918a_force_disable_layer_tile_bg.v==0) {

                                if (vdp_9918a_reveal_layer_tile_bg.v) {
                                    int posx=xdestino&1;
                                    int posy=scanline&1;

                                    //0,0: 0
                                    //0,1: 1
                                    //1,0: 1
                                    //1,0: 0
                                    //Es un xor

                                    int si_blanco_negro=posx ^ posy;

                                    //Color 0 o 63 (negro / blanco)
                                    color_paleta=si_blanco_negro*(SMS_TOTAL_PALETTE_COLOURS-1);
                                }

                                *destino_scanline_buffer=SMS_INDEX_FIRST_COLOR+color_paleta;
                            }
                        }
                        destino_scanline_buffer++;
                        destino_scanline_buffer_foreground++;

                    }




                    if (mirror_x) {
                        byte_leido1=byte_leido1>>1;
                        byte_leido2=byte_leido2>>1;
                        byte_leido3=byte_leido3>>1;
                        byte_leido4=byte_leido4>>1;
                    }

                    else {
                        byte_leido1=byte_leido1<<1;
                        byte_leido2=byte_leido2<<1;
                        byte_leido3=byte_leido3<<1;
                        byte_leido4=byte_leido4<<1;
                    }


                }

                //}

            //}


        }




}



//Renderiza una linea de sprites en modo rainbow
void vdp_9918a_render_rainbow_sprites_line_post_sms(int scanline,z80_int *destino_scanline_buffer,z80_byte *vram)
{



   z80_int sprite_pattern_table=vdp_9918a_get_sprite_pattern_table_sms_mode4();

    z80_byte byte_leido1,byte_leido2,byte_leido3,byte_leido4;


    //int sprite_size=vdp_9918a_get_sprite_size();
    int sprite_double=vdp_9918a_get_sprite_double();

    int sprites_en_linea=0;

    //Asumimos que se resetea el bit 5S del status register
    //Bit  6    5S         1 if more than 4 sprites on a horizontal line
    vdp_9918a_status_register&= (255-64);


    //TODO temp
    //sprite_size=8;
    sprite_double=1;

/*
TODO
 Register $01 - Mode Control No. 2

 D7 - No effect
 D6 - (BLK) 1= Display visible, 0= display blanked.
 D5 - (IE0) 1= Frame interrupt enable.
 D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
 D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
 D2 - No effect
 D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
 D0 - Sprite pixels are doubled in size.
 */

    //printf ("Sprite size: %d double: %d\n",sprite_size,sprite_double);
    //printf("Sprite size: 8x%d\n",(vdp_9918a_registers[1] & 2 ? 16 : 8));


    int sprite_height=vdp_9918a_sms_get_sprite_height();

    //TODO: si coordenada Y=208, fin tabla sprites
    //    z80_int sprite_attribute_table=(vdp_9918a_registers[5]) * 0x80;

    //z80_int sprite_pattern_table=(vdp_9918a_registers[6]) * 0x800;

    int sprite;
    int salir=0;

    //En boundary de 128
    //sprite_attribute_table &=(65535-128);

    z80_int sprite_attribute_table=vdp_9918a_get_sprite_attribute_table();

    //Empezar por la del final
    //Ver si hay alguno con coordenada 208 que indica final

    int primer_sprite_final; //=VDP_9918A_SMS_MODE4_MAX_SPRITES-1;


    // buscar ultimo sprite
    /*
        If the Y coordinate is set to $D0, then the sprite in question and all
remaining sprites of the 64 available will not be drawn. This only works
in the 192-line display mode, in the 224 and 240-line modes a Y coordinate
of $D0 has no special meaning.
    */

    int maximo_sprites_por_linea=VDP_9918A_SMS_MAX_SPRITES_PER_LINE;

    //Si hay setting de no limite sprites por linea
    if (vdp_9918a_unlimited_sprites_line.v) {
        maximo_sprites_por_linea=9999;
    }

    for (primer_sprite_final=0;primer_sprite_final<VDP_9918A_SMS_MODE4_MAX_SPRITES && !salir;primer_sprite_final++) {

        int offset_sprite=sprite_attribute_table+primer_sprite_final;

        z80_byte vert_pos=vdp_9918a_read_vram_byte(vram,offset_sprite);
        if (vert_pos==208) salir=1;

    }


    //Siempre estara al siguiente
    primer_sprite_final--;



        //Empezar desde final hacia principio
/*
 Each sprite is defined in the sprite attribute table (SAT), a 256-byte
 table located in VRAM. The SAT has the following layout:

    00: yyyyyyyyyyyyyyyy
    10: yyyyyyyyyyyyyyyy
    20: yyyyyyyyyyyyyyyy
    30: yyyyyyyyyyyyyyyy
    40: ????????????????
    50: ????????????????
    60: ????????????????
    70: ????????????????
    80: xnxnxnxnxnxnxnxn
    90: xnxnxnxnxnxnxnxn
    A0: xnxnxnxnxnxnxnxn
    B0: xnxnxnxnxnxnxnxn
    C0: xnxnxnxnxnxnxnxn
    D0: xnxnxnxnxnxnxnxn
    E0: xnxnxnxnxnxnxnxn
    F0: xnxnxnxnxnxnxnxn

 y = Y coordinate + 1
 x = X coordinate
 n = Pattern index
 ? = Unused

*/

    for (sprite=primer_sprite_final;sprite>=0 && sprites_en_linea<maximo_sprites_por_linea;sprite--) {
        int vert_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+sprite);
        //printf("sprite %d pos %d\n",sprite,sprite_attribute_table+sprite);

        int horiz_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+0x80+sprite*2);
        z80_byte sprite_name=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+0x80+sprite*2+1);


        //printf("Sprite %d Pattern %d X %d Y %d\n",sprite,sprite_name,horiz_pos,vert_pos);

        /*
        TODO
            The pattern index selects one of 256 patterns to use. Bit 2 of register #6
acts like a global bit 8 in addition to this value, allowing sprite patterns
to be taken from the first 256 or last 256 of the 512 available patterns.
        */



        vert_pos++; //255->coordenada 0
        if (vert_pos==256) vert_pos=0;

        //Entre 255 y 256-32-> son coordenadas negativas
        if (vert_pos>=256-32) {
            //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);
            //printf ("Sprite Y negative: %d\n",vert_pos-256);
            vert_pos=vert_pos-256;

        }

        //Siguiente sprite. El precedente
        //sprite_attribute_table -=4;

        //Si early clock, x-=32

        /*if (attr_color_etc & 128) {
            //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);
            horiz_pos -=32;
        }*/

        //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);




        //Si posicion Y sprite esta en el margen
        //if (vert_pos<192) {
        if (scanline>=vert_pos && scanline<vert_pos+sprite_height*sprite_double) {
            //int offset_pattern_table=sprite_name*bytes_per_sprite+sprite_pattern_table;
                int offset_pattern_table=sprite_name*32+sprite_pattern_table;

            //z80_byte color=attr_color_etc & 15;

            int x;

            //linea 0..7 dentro del sprite
            int fila_sprites=(scanline-vert_pos)/sprite_double;

            offset_pattern_table +=fila_sprites*4;



            //Sprites de 8x16 y 8x8


                //for (y=0;y<sprite_height;y++) {

                    byte_leido1=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                    byte_leido2=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                    byte_leido3=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                    byte_leido4=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);


                    for (x=0;x<8;x++) {

                        int pos_x_final;
                        //int pos_y_final;

                        pos_x_final=horiz_pos+x*sprite_double;
                        //pos_y_final=vert_pos+y*sprite_double;

                        if (pos_x_final>=0 && pos_x_final<=255 /*&& pos_y_final>=0 && pos_y_final<=191*/) {

                            //Si bit a 1
                            if (1) {
                                //Y si ese color no es transparente

                                //TODO
                                //if (color!=0) {
                                if (1) {
                                    //printf ("putpixel sprite x %d y %d\n",pos_x_final,pos_y_final);

                                    z80_byte byte_color=((byte_leido1>>7)&1) | ((byte_leido2>>6)&2) | ((byte_leido3>>5)&4) | ((byte_leido4>>4)&8);



                                    //TODO: segunda paleta??
                                    z80_byte color_sprite=vdp_9918a_sms_cram[16 + (byte_color & 15)];

                                    //maximo 64 colores de paleta
                                    color_sprite &=63;

                                    if (vdp_9918a_reveal_layer_sprites.v) {
                                        int posx=pos_x_final&1;
                                        int posy=scanline&1;

                                        //0,0: 0
                                        //0,1: 1
                                        //1,0: 1
                                        //1,0: 0
                                        //Es un xor

                                        int si_blanco_negro=posx ^ posy;

                                        //Color 0 o 63 (negro / blanco)
                                        color_sprite=si_blanco_negro*(SMS_TOTAL_PALETTE_COLOURS-1);
                                    }

                                    //transparencia
                                    if (byte_color!=0) {

                                        //if (x==0 && y==0) printf("Dibujando sprite %d\n",sprite);


                                        int mostrar=1;
                                        //Si ocultar primera columna
                                        if (pos_x_final<=7 && (vdp_9918a_registers[0] & 32)) {
                                            //en sonic por ejemplo los enemigos que se van
                                            //por la izquierda se ven en esta columna oculta

                                            //Y no ocultarlo si tenemos el setting de mostrar forzado columna 0
                                            if (vdp_9918a_sms_force_show_column_zero.v==0) {
                                                mostrar=0;
                                            }
                                        }



                                        if (mostrar) {
                                            //scr_putpixel_zoom(pos_x_final,  pos_y_final,  SMS_INDEX_FIRST_COLOR+color_sprite);

                                            vdp9918a_put_sprite_pixel(&destino_scanline_buffer[pos_x_final],SMS_INDEX_FIRST_COLOR+color_sprite);
                                            if (sprite_double==2) {
                                                //scr_putpixel_zoom(pos_x_final+1,  pos_y_final,    SMS_INDEX_FIRST_COLOR+color_sprite);
                                                //scr_putpixel_zoom(pos_x_final,    pos_y_final+1,  SMS_INDEX_FIRST_COLOR+color_sprite);
                                                //scr_putpixel_zoom(pos_x_final+1,  pos_y_final+1,  SMS_INDEX_FIRST_COLOR+color_sprite);

                                                //TODO. Verificar esto
                                                vdp9918a_put_sprite_pixel(&destino_scanline_buffer[pos_x_final+1],SMS_INDEX_FIRST_COLOR+color_sprite);
                                            }
                                        }

                                    }
                                }
                            }
                        }



                        byte_leido1=byte_leido1<<1;
                        byte_leido2=byte_leido2<<1;
                        byte_leido3=byte_leido3<<1;
                        byte_leido4=byte_leido4<<1;
                    }


                //}


        }


    }


}




//Renderiza una linea de sprites, primero en buffer temporal así para poder gestionar colisiones
void vdp_9918a_render_rainbow_sprites_line_sms(int scanline,z80_int *scanline_buffer,z80_byte *vram)
{



    int i;

    //Inicializar buffer temporal a color 0 transparente
    for (i=0;i<256;i++) {
        vdp_9918a_buffer_render_sprites[i]=0;
    }


    vdp_9918a_render_rainbow_sprites_line_post_sms(scanline,vdp_9918a_buffer_render_sprites,vram);

    //Y copiar al buffer inicial


    for (i=0;i<256;i++) {
        //TODO. hacer esto con memcpy

        //Copiar en destino, saltando border izquierdo, solo si color no es transparente
        z80_int color_pixel=vdp_9918a_buffer_render_sprites[i];

        if (color_pixel!=0) {
            scanline_buffer[screen_total_borde_izquierdo+i]=color_pixel;
        }
    }

}


//Almacenaje temporal de render de la linea de sprites actual
z80_int sms_scanline_buffer_sprites[512];

//Almacenaje temporal de render de la linea de tiles background actual
z80_int sms_scanline_buffer_tiles_background[512];

//Almacenaje temporal de render de la linea de tiles foreground actual
z80_int sms_scanline_buffer_tiles_foreground[512];

#define SMS_3LAYERS_TRANSPARENT_COLOUR 65535

//Guardar en buffer rainbow la linea actual. Para SMS. solo display
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
//Mezcla las 2 capas de tile y la de sprites
void screen_store_scanline_rainbow_solo_display_vdp_9918a_sms_3layer(z80_int *scanline_buffer,z80_byte *vram_memory_pointer,int y_display)
{

    //Inicializamos las 3 capas a transparente
    int i;

    for (i=0;i<512;i++) {
        sms_scanline_buffer_sprites[i]=sms_scanline_buffer_tiles_background[i]=sms_scanline_buffer_tiles_foreground[i]=SMS_3LAYERS_TRANSPARENT_COLOUR;
    }


    //Render las dos capas de tiles
    vdp_9918a_render_rainbow_display_line_sms(y_display,sms_scanline_buffer_tiles_background,sms_scanline_buffer_tiles_foreground,vram_memory_pointer);



    //Render sprites
    if (vdp_9918a_force_disable_layer_sprites.v==0) {
        vdp_9918a_render_rainbow_sprites_line_sms(y_display,sms_scanline_buffer_sprites,vram_memory_pointer);
    }


    //Mezclar las 3 capas
    z80_int color_capa_sprites;
    z80_int color_capa_tiles_foreground;
    z80_int color_capa_tiles_background;

    //No tocar zona izquierda y derecha que es el border.
    //Border no entra como un layer, sino que se renderiza directamente en scanline_buffer
    for (i=screen_total_borde_izquierdo;i<screen_total_borde_izquierdo+256;i++) {
        //Prioridades:
        //Arriba: capa tiles foreground
        //Medio: capa sprites
        //Abajo: capa tiles background

        //Capa tiles foreground no transparente?
        color_capa_tiles_foreground=sms_scanline_buffer_tiles_foreground[i];
        if (color_capa_tiles_foreground!=SMS_3LAYERS_TRANSPARENT_COLOUR) {
            //printf("Tile en foreground\n");
            scanline_buffer[i]=color_capa_tiles_foreground;
        }

        else {
            //Capa sprites no transparente?
            color_capa_sprites=sms_scanline_buffer_sprites[i];
            if (color_capa_sprites!=SMS_3LAYERS_TRANSPARENT_COLOUR) scanline_buffer[i]=color_capa_sprites;
            else {
                //Capa background no transparente?
                color_capa_tiles_background=sms_scanline_buffer_tiles_background[i];
                if (color_capa_tiles_background!=SMS_3LAYERS_TRANSPARENT_COLOUR) {
                    scanline_buffer[i]=color_capa_tiles_background;
                }

                //Finalmente sera color negro
                else {
                    scanline_buffer[i]=SMS_INDEX_FIRST_COLOR;
                }


            }
        }


    }


}


void vdp9918a_sms_set_scroll_vertical(z80_byte valor)
{

    //En SMS este registro se actualiza a final de frame
    //printf("Cambio scroll vertical: %d\n",vdp_9918a_last_command_status_bytes[0]);
    sms_next_scroll_vertical_value=valor;

}

void vdp_9918a_sms_set_writing_cram(z80_byte valor)
{
    //printf("Write palette. Index: %d byte2: %d\n",vdp_9918a_last_command_status_bytes[0],vdp_9918a_last_command_status_bytes[1] & 63);

    sms_writing_cram=1;

    index_sms_escritura_cram=valor;
}


int vdp_9918a_sms_get_final_color_border(void)
{
    z80_byte border_registro=vdp_9918a_get_border_color();
    return vdp_9918a_sms_get_cram_color(border_registro & 15)+SMS_INDEX_FIRST_COLOR;
}

z80_byte vdp_9918a_sms_raster_line_counter=0;

void vdp_9918a_sms_raster_line_reset(void)
{
    vdp_9918a_sms_raster_line_counter=vdp_9918a_registers[10];
    //printf("reset rasterline to %3d on PC=%04XH A=%02XH BC=%02XH\n",vdp_9918a_sms_raster_line_counter,reg_pc,reg_a,BC);
}


void vdp_9918a_sms_handle_raster_interrupt(void)
{

    if (sms_disable_raster_interrupt.v) return;

    //Solo en zona de pantalla+1, no border
    int linea_actual_interrupcion=t_scanline_draw-screen_invisible_borde_superior-screen_borde_superior;
    if (linea_actual_interrupcion>=1 && linea_actual_interrupcion<=191) {

        //printf("linea_actual_interrupcion %d\n",linea_actual_interrupcion);
        //printf("rasterline %d\n",vdp_9918a_sms_raster_line_counter);

        if (vdp_9918a_sms_raster_line_counter==0) {
            //printf("raster counter is 0. set to new value %d\n",vdp_9918a_registers[10]);

            //temp
            //if (vdp_9918a_registers[10]==0xFF) vdp_9918a_registers[0] &=(255-0x10);


            //else
            vdp_9918a_sms_raster_line_reset();



            if (sms_only_one_raster_int_frame.v) {
                //Si setting de solo una interrupcion por frame,
                //cuando se dispara la primera lo ponemos a valor mas alto, asi
                //no llegara nunca a 0 en este frame (se decrementa 191 veces por frame, o sea no tiene tiempo a llegar a 0)
                vdp_9918a_sms_raster_line_counter=255;
            }

            //printf("Possible Fire interrupt. Look iff1.v=%d vdp_9918a_registers[0] & 0x10 %d\n",iff1.v,vdp_9918a_registers[0] & 0x10);

            if (vdp_9918a_registers[0] & 0x10) {
                //TODO $FF turns off the interrupt requests
                //master of madness no parece ir bien con esta condicion (scroll mal)

                int disparar_interrupcion=0;
                if (iff1.v) disparar_interrupcion=1;

                /*
                if (sms_wonderboy_scroll_hack.v && vdp_9918a_registers[10]==0xFF) {
                    printf("---valor FF. Deshabilitadas\n");
                    disparar_interrupcion=0;
                }
                */

               //if (vdp_9918a_registers[10]==0xFF) disparar_interrupcion=0;

                if (disparar_interrupcion) {
                    //printf("Fired Line interrupt enabled. Reg10: %d tscanline: %d\n",vdp_9918a_registers[10],t_scanline_draw);
                    //sms_pending_line_interrupt=1;
                    interrupcion_maskable_generada.v=1;
                }
            }
        }

        else {
            vdp_9918a_sms_raster_line_counter--;
            //printf("Decrementada linea: %d\n",vdp_9918a_sms_raster_line_counter);
        }

    }


}


z80_byte vdp_9918a_sms_pre_write_reg(z80_byte vdp_register,z80_byte next_value)
{

    /*if (vdp_register==0) {
        if ((next_value & 0x10)==0) {
            printf("Disabling raster interrupt t_scanline_draw=%d on PC=%04XH A=%02XH BC=%02XH\n",
                            t_scanline_draw,reg_pc,reg_a,BC);
        }
        else {
            printf("Enabling raster interrupt t_scanline_draw=%d on PC=%04XH A=%02XH BC=%02XH\n",
                            t_scanline_draw,reg_pc,reg_a,BC);
        }
    }
    */

   /*
    if (vdp_register==8) printf("Change scroll horizontal to %3d t_scanline_draw=%d on PC=%04XH A=%02XH BC=%02XH\n",
                            next_value,t_scanline_draw,reg_pc,reg_a,BC);
    */

    if (vdp_register==9) {
        vdp9918a_sms_set_scroll_vertical(next_value);
        //Para dejarlo igual
        next_value=vdp_9918a_registers[9];
    }

    if (vdp_register==10) {
        /*printf("change raster register to %3d (previous %3d) t_scanline_draw=%d raster_counter %d on PC=%04XH A=%02XH BC=%02XH\n",
                        next_value,vdp_9918a_registers[10],t_scanline_draw,vdp_9918a_sms_raster_line_counter,
                        reg_pc,reg_a,BC);
        */


        /*
        //quiza si es 255 no altera su valor, solo desactiva la raster interrupt
        if (next_value==0xFF && sms_wonderboy_scroll_hack.v) {
            //printf("Do not change raster line value as it is 0xFF\n");
            //vdp_9918a_registers[0] &=(255-0x10);
            //Dejar mismo valor
            //master of madness no parece ir bien con esto (scroll mal)
            next_value=vdp_9918a_registers[vdp_register];
        }
        */

        //Si estaba en FF, cambio instantaneo??
        //Este hack o el justo anterior, pero no poner los dos a la vez
        if (vdp_9918a_registers[10]==0xFF && sms_wonderboy_scroll_hack.v) {
            vdp_9918a_sms_raster_line_counter=next_value;
        }

    }

    return next_value;
}
