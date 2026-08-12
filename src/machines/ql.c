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
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "ql.h"
#include "m68k.h"
#include "debug.h"
#include "utils.h"
#include "zxvision.h"
#include "operaciones.h"
#include "screen.h"
#include "settings.h"
#include "ay38912.h"
#include "ql_i8049.h"
#include "ql_zx8302.h"
#include "compileoptions.h"
#include "ula.h"
#include "ql_qdos_handler.h"
#include "autoselectoptions.h"
#include "joystick.h"


#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif



unsigned char *memoria_ql;

//ultima direccion de memoria válida
//128k de rom + 128k de ram por defecto
unsigned int ql_mem_limit=(1024*(128+128))-1;


//Aunque el ql no tiene realmente border, metemos uno pequeño artificial y le asignamos un color
int ql_border_color=0;

int ql_pantalla_proporcion_real=QL_SIZE_TYPE_1476;

int ql_use_visionql_color_palette=0;

/*
"The pixels of the original QL's 512×256 screen are 1.355 times as high as they are wide.
Combined with the 2:1 ratio of the 512×256 resolution, the aspect ratio of the QL screen will then be 2/1.355 ≈ 1.476.
This is wider than 4:3 due to the overscan."
https://theqlforum.com/viewtopic.php?start=20&t=2799
*/

int ql_get_display_width_with_proportion(void)
{
    switch (ql_pantalla_proporcion_real) {
        case QL_SIZE_TYPE_133:
            return QL_133_DISPLAY_WIDTH;
        break;

        case QL_SIZE_TYPE_1476:
            return QL_1476_DISPLAY_WIDTH;
        break;
    }

    return QL_DISPLAY_WIDTH;
}




//Define el total de RAM del QL
void ql_set_memory_size(int kb_ram)
{
    ql_mem_limit=(1024*(128+kb_ram))-1;
}

int ql_get_current_ram_kb(void)
{
    return ((ql_mem_limit+1)/1024)-128;
}

//Retorna el maximo asignable de RAM
int ql_get_maximum_ram_kb(void)
{
    return ((QL_MAXIMUM_MEM_LIMIT+1)/1024)-128;
}



//Si la emulacion esta habilitada
int ql_qimi_mouse_enabled=0;



int ql_qimi_irq_enabled=0;

unsigned char ql_qimi_movement_register_114620=0;

int ql_qimi_mouse_x_antes=0;
int ql_qimi_mouse_y_antes=0;

/*

QIMI Mouse

QIMI Mouse Interface document

Updated: 26.04.00

Dave Westbury has kindly supplied some programming information and register details for the QIMI mouse interface. His notes are reproduced below.

Circuit Diagram - Click here to download the circuit diagram (a 512x256 QL _PIC graphics file)

QIMI interrupts are enabled by reading or writing to address 114622

Mouse buttons read from address 114588:
bit 4 = right button
bit 5 = left button
0 = button on.

Movement read from address 114620:
bit 0 = up/down direction
bit 2 = right/left movement
bit 4 = right/left direction
bit 5 = up/down movement
0=left & down, 1=right & up.


*/

void ql_qimi_reset(void)
{
    ql_qimi_irq_enabled=0;
}

void ql_qimi_writebyte(unsigned int Address, unsigned char Data)
{
    if (Address==114622) {
        printf("Enabled IRQ\n");
        ql_qimi_irq_enabled=1;
    }
}


void ql_qimi_handle_irq(void)
{
    if (!ql_qimi_mouse_enabled) return;

    if (!ql_qimi_irq_enabled) return;

    ql_qimi_movement_register_114620=0;

    if (zxvision_key_not_sent_emulated_mach()) return;

    int hay_movimiento=0;

    if (mouse_x!=ql_qimi_mouse_x_antes) {

        ql_qimi_movement_register_114620 |=4; //movimiento horizontal

        if (mouse_x>ql_qimi_mouse_x_antes) ql_qimi_movement_register_114620 |=16; //derecha


        hay_movimiento=1;
    }

    if (mouse_y!=ql_qimi_mouse_y_antes) {

        ql_qimi_movement_register_114620 |=32; //movimiento vertical

        if (mouse_y<ql_qimi_mouse_y_antes) ql_qimi_movement_register_114620 |=1; //arriba


        hay_movimiento=1;
    }

    if (hay_movimiento) {
            printf("enviada irq mouse\n");

            //mouse
            ql_pc_intr |=16;

            m68k_set_irq(2);
    }


    ql_qimi_mouse_x_antes=mouse_x;
    ql_qimi_mouse_y_antes=mouse_y;
}

unsigned char ql_qimi_readbyte(unsigned int Address)
{



    if (Address==0x1BFBC || Address==0x1BFBE) {
        printf("QIMI read Address %X %d\n",Address,Address);
    }

    if (Address==114588) {
        unsigned char return_value=255;
        if (!zxvision_key_not_sent_emulated_mach() ) {
            if (mouse_left) return_value -=32;
            if (mouse_right) return_value -=16;
        }
        return return_value;
    }
    else if (Address==114620) return ql_qimi_movement_register_114620;
    else if (Address==114622) ql_qimi_irq_enabled=1;

    return 255;
}


void ql_writebyte(unsigned int Address, unsigned char Data)
{
    Address %=(ql_mem_limit+1);

    //QIMI 114622=0x1BFBE
    //QIMI 114620=0x1BFBC)
    //QIMI 114588=0x1BF9C)
    if (ql_qimi_mouse_enabled && (Address==0x1BFBC || Address==0x1BF9C || Address==0x1BFBE)) {
        printf("QIMI write Address %XH %d value %02X\n",Address,Address,Data);
        ql_qimi_writebyte(Address,Data);
        return;
    }

    if (Address>=0x18000 && Address<=0x1BFFF) {
        ql_zx8032_write(Address,Data);


        #ifdef EMULATE_VISUALMEM

        //Escribimos en visualmem a partir de direccion 18000H
        set_visualmembuffer(Address);

        #endif

        return; //Espacio i/o
    }

    if (Address<0x18000 || Address>ql_mem_limit) return;


    unsigned char valor=Data;

    memoria_ql[Address]=valor;

    #ifdef EMULATE_VISUALMEM

    //Escribimos en visualmem a partir de direccion 18000H
    set_visualmembuffer(Address);

    #endif

}

unsigned char ql_readbyte(unsigned int Address)
{
    Address %=(ql_mem_limit+1);

    //QIMI 114622=0x1BFBE
    //QIMI 114620=0x1BFBC)
    //QIMI 114588=0x1BF9C)
    if (ql_qimi_mouse_enabled && (Address==0x1BFBC || Address==0x1BF9C || Address==0x1BFBE)) {
        return ql_qimi_readbyte(Address);
    }

    if (Address>=0x18000 && Address<=0x1BFFF) {


        unsigned char valor=ql_zx8032_readbyte(Address);

        #ifdef EMULATE_VISUALMEM

        //Escribimos en visualmem a partir de direccion 18000H
        set_visualmemreadbuffer(Address);

        #endif
        return valor;
    }


    if (Address>ql_mem_limit) return(0);

    #ifdef EMULATE_VISUALMEM

    //Escribimos en visualmem a partir de direccion 18000H
    set_visualmemreadbuffer(Address);

    #endif


    unsigned char valor=memoria_ql[Address];
    return valor;
}

//Puntero a la funcion final que se modifica cuando se asigna maquina QL. Al inicio, se apunta a funcion vacia para
//que el parser de breakpoints desde configfile no pete
//Podia petar con --machine QL --set-breakpoint 1 "OPCODE1=207"
unsigned char (*ql_readbyte_no_ports_function)(unsigned int Address);

unsigned char ql_readbyte_no_ports_vacio(unsigned int Address GCC_UNUSED)
{
    return 0;
}

unsigned char ql_readbyte_no_ports(unsigned int Address)
{
    Address %=(ql_mem_limit+1);
    unsigned char valor=memoria_ql[Address];
    return valor;

}

void ql_writebyte_no_ports(unsigned int Address,unsigned char valor)
{
    Address %=(ql_mem_limit+1);
    memoria_ql[Address]=valor;

}





unsigned int GetMemB(unsigned int address)
{
    return(ql_readbyte(address));
}


/* Fetch word, address may not be word-aligned */
unsigned int  GetMemW(unsigned int address)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 1);
#endif
    return((ql_readbyte(address)<<8)|ql_readbyte(address+1));
}


/* Fetch dword, address may not be dword-aligned */
unsigned int GetMemL(unsigned int address)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 1);
#endif
    return((GetMemW(address)<<16) | GetMemW(address+2));
}


/* Write byte to address */
void SetMemB (unsigned int address, unsigned int value)
{
    ql_writebyte(address,value);
}


/* Write word, address may not be word-aligned */
void SetMemW(unsigned int address, unsigned int value)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 0);
#endif
    ql_writebyte(address,(value>>8)&255);
    ql_writebyte(address+1, (value&255));
}
/* Write dword, address may not be dword-aligned */
void SetMemL(unsigned int address, unsigned int value)
{
#ifdef CHKADDRESSERR
    if (address & 0x1) ExceptionGroup0(ADDRESSERR, address, 0);
#endif
    SetMemW(address, (value>>16)&65535);
    SetMemW(address+2, (value&65535));
}


unsigned int m68k_read_disassembler_16 (unsigned int address)
{
    return GetMemW(address);
}


unsigned int m68k_read_disassembler_32 (unsigned int address)
{
    return GetMemL(address);
}




//Funciones legacy solo para interceptar posibles llamadas a poke, peek etc en caso de motorola
//la mayoria de estas vienen del menu, lo ideal es que en el menu se usen peek_byte_z80_moto , etc

void poke_byte_legacy_ql(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED)
{
    debug_printf(VERBOSE_ERR,"Calling poke_byte function on a QL machine. TODO fix it!");
}

void poke_byte_no_time_legacy_ql(z80_int dir GCC_UNUSED,z80_byte valor GCC_UNUSED)
{
    debug_printf(VERBOSE_ERR,"Calling poke_byte_no_time function on a QL machine. TODO fix it!");
}

z80_byte peek_byte_legacy_ql(z80_int dir GCC_UNUSED)
{
    debug_printf(VERBOSE_ERR,"Calling peek_byte function on a QL machine. TODO fix it!");
    return 0;
}

z80_byte peek_byte_no_time_legacy_ql(z80_int dir GCC_UNUSED)
{
    //debug_printf(VERBOSE_ERR,"Calling peek_byte_no_time function on a QL machine. TODO fix it!");
    return 0;
}

z80_byte lee_puerto_legacy_ql(z80_byte h GCC_UNUSED,z80_byte l GCC_UNUSED)
{
    debug_printf(VERBOSE_ERR,"Calling lee_puerto function on a QL machine. TODO fix it!");
    return 0;
}

void out_port_legacy_ql(z80_int puerto GCC_UNUSED,z80_byte value GCC_UNUSED)
{
    debug_printf(VERBOSE_ERR,"Calling out_port function on a QL machine. TODO fix it!");
}

z80_byte fetch_opcode_legacy_ql(void)
{
    debug_printf(VERBOSE_ERR,"Calling fetch_opcode function on a QL machine. TODO fix it!");
    return 0;
}





void motorola_get_flags_string(char *texto)
{

    unsigned int registro_sr=m68k_get_reg(NULL, M68K_REG_SR);

    sprintf (texto,"%c%c%c%c%c%c%c%c%c%c",
        (registro_sr&32768 ? 'T' : '-'),
        (registro_sr&8192  ? 'S' : '-'),
        (registro_sr&1024  ? '2' : '-'),
        (registro_sr&512   ? '1' : '-'),
        (registro_sr&256   ? '0' : '-'),
        (registro_sr&16 ? 'X' : '-'),
        (registro_sr&8  ? 'N' : '-'),
        (registro_sr&4  ? 'Z' : '-'),
        (registro_sr&2  ? 'V' : '-'),
        (registro_sr&1  ? 'C' : '-')  );
}


void ql_putpixel_rainbow(int x,int y,int color)
{
    int ancho=get_total_ancho_rainbow();
    int alto=get_total_alto_rainbow();

    if (x>=0 && y>=0 && x<ancho && y<alto) {
        int indice;

        indice=(ancho*y)+x;

        rainbow_buffer[indice]=color;

    }

}


//Hace putpixel en x,y doblando en alto
void ql_putpixel_zoom(int x,int y,unsigned int color)
{

    if (rainbow_enabled.v) {
        ql_putpixel_rainbow(x+QL_LEFT_BORDER_NO_ZOOM*border_enabled.v,y+QL_TOP_BORDER_NO_ZOOM*border_enabled.v,QL_INDEX_FIRST_COLOR+color);
        ql_putpixel_rainbow(x+QL_LEFT_BORDER_NO_ZOOM*border_enabled.v,y+QL_TOP_BORDER_NO_ZOOM*border_enabled.v+1,QL_INDEX_FIRST_COLOR+color);
    }
    else {
        scr_putpixel_zoom(x,y,QL_INDEX_FIRST_COLOR+color);
        scr_putpixel_zoom(x,y+1,QL_INDEX_FIRST_COLOR+color);
    }

}

//Para poder escalar en ancho a las relaciones de aspecto que no son 1:1
void scr_refresca_pantalla_ql_putpixel_aspect_ratio(int x,int y,int color,int *xdestino,int *acumulado_escala_1476)
{
    //Para proporcion 1.476
    //1.476=1476/1000
    //Por cada 1000 píxeles de entrada, produces 1476 píxeles de salida.
    //Debes insertar 476 píxeles extra por cada 1000 originales.


    if (ql_pantalla_proporcion_real==QL_SIZE_TYPE_133 && (x%3)==0) ql_putpixel_zoom((*xdestino)++,y*2,color);


    (*acumulado_escala_1476) +=476;
    if (ql_pantalla_proporcion_real==QL_SIZE_TYPE_1476) {
        while (*acumulado_escala_1476 >= 1000) {
            ql_putpixel_zoom((*xdestino)++,y*2,color);
            (*acumulado_escala_1476) -= 1000;
        }
    }
}


//Refresco de pantalla ql sin rainbow
void scr_refresca_pantalla_ql(void)
{


/*
$18063	MC_STAT		Master chip status register
Bit	Purpose
1	0 = Screen on
    1 = Screen off

3	0 = 4 colour (mode 4)
    1 = 8 colour (mode 8)

7	0 = Use screen 0 (allegedly at $20000)
    1 = Use screen 1 (allegedly at $280000)

*/

    z80_byte mc_stat=ql_mc_stat;
    int video_mode=(mc_stat>>3)&1;
    //printf ("mc_stat: %02XH video_mode: %d\n",mc_stat,video_mode);

    int pantalla_apagada=(mc_stat & 2);

    //if (pantalla_apagada) printf("Pantalla apagada %d\n",contador_segundo);

    int total_alto;
    int total_ancho;
    int x,y;

    unsigned int color1;


    z80_byte green,red,blue;

    z80_byte byte_leido_h,byte_leido_l;

    unsigned char *memoria_pantalla_ql;

    memoria_pantalla_ql=&memoria_ql[0x20000 + ((mc_stat & 0x80) << 8)];



    total_alto=256;
    total_ancho=512;

    int flashing_color;

    for (y=0;y<total_alto;y++){
        //Al principio de cada linea, flash es siempre 0
        int ql_linea_flashing=0;
        int xdestino=0;

        int acumulado_escala_1476=0;

        for (x=0;x<total_ancho;) {

/*
In 512-pixel mode, two bits per pixel are used, and the GREEN and BLUE signals are tied together, giving a choice of four colours:
black, white, green and red. On a monochrome screen, this will translate as a four level greyscale.
In 256-pixel mode, four bits per pixel are used: one bit each for Red, Green and Blue, and one bit for flashing.
The flash bit operates as a toggle: when set for the first time, it freezes the background colour at the value set by R, G and B,
and starts flashing at the next bit in the line; when set for the second time, it stops flashing.
Flashing is always cleared at the beginning of a raster line.


Addressing for display memory starts at the bottom of dynamic RAM and progresses in the order of the raster
scan - from left to right and from top to bottom of the picture. Each word in display memory is formatted as follows:

High byte (A0=0)						Low Byte (A0=1)						Mode
D7 D6 D5 D4 D3 D2 D1 D0			D7 D6 D5 D4 D3 D2 D1 D0
G7 G6 G5 G4 G3 G2 G1 G0			R7 R6 R5 R4 R3 R2 R1 R0		512-pixel
G3 F3 G2 F2 G1 F1 G0 F0			R3 B3 R2 B2 R1 B1 R0 B0		256-pixel


R, G, Band F in the above refer to Red, Green, Blue and Flash. The numbering is such that a binary
word appears written as it will appear on the display: ie R0 is the value of Red for the rightmost pixel,
that is the last pixel to be shifted out onto the raster.
10.3 Display Control Register
This is a write-only register, which is at $18063 in the QL .
One of its bits is available through the Qdos MT.DMODE trap: bit 3, which is 0 for 512-pixel mode and 1 for 256-pixel mode.
The other two bits of the display control register are not supported by Qdos, these being bit 1 of the display
control register, which can be used to blank the display completely, and bit 7, which can be used to switch the base of
screen memory from $20000 to $28000. Future versions of Qdos may allow the system variables to be
initialised at $30000 to take advantage of this dual- screen feature: the present version does not.
Bits 0,2,4,5 and 6 of the display control register should never be set to anything other than zero, as they are
reserved and may have unpredictable results in future versions of the QL hardware.
*/
            //En modo 256x256 hay parpadeo


            byte_leido_h=*memoria_pantalla_ql;
            memoria_pantalla_ql++;

            byte_leido_l=*memoria_pantalla_ql;
            memoria_pantalla_ql++;

            if (video_mode==1) {

                int npixel;
                for (npixel=7;npixel>=0;npixel-=2) {

                    //G3 F3 G2 F2 G1 F1 G0 F0                 R3 B3 R2 B2 R1 B1 R0 B0         256-pixel

                    green=((byte_leido_h)>>npixel)&1;
                    red=((byte_leido_l)>>npixel)&1;
                    blue=((byte_leido_l)>>(npixel-1))&1;


                    /*
                    //colores para QL
                    const int ql_colortable_original[8]={
                    0x000000, //Negro
                    0x0000ff, //Azul
                    0xff0000, //Rojo
                    0xff00ff, //Magenta
                    0x00ff00, //Verde
                    0x00ffff, //Cyan
                    0xffff00, //Amarillo
                    0xffffff  //Blanco
                    };
                    */

                    color1=green*4+red*2+blue;	// GRB
                    //printf ("estado parpadeo: %d\n",estado_parpadeo.v);

                    if (ql_linea_flashing && estado_parpadeo.v) {
                        color1=flashing_color;
                    }

                    if (pantalla_apagada) color1=0;

                    ql_putpixel_zoom(xdestino++,y*2,color1);
                    ql_putpixel_zoom(xdestino++,y*2,color1);

                    x++;

                    scr_refresca_pantalla_ql_putpixel_aspect_ratio(x,y,color1,&xdestino,&acumulado_escala_1476);

                    x++;

                    scr_refresca_pantalla_ql_putpixel_aspect_ratio(x,y,color1,&xdestino,&acumulado_escala_1476);



                    //Ver si cambia valor bit flash
                    int bit_flashing=((byte_leido_h)>>(npixel-1))&1;
                    if (bit_flashing) {
                        ql_linea_flashing ^=1;
                        flashing_color=color1;
                    }

                }



            }

            //Al arrancar, esta mc_stat=2A=0010 1010 -> modo 4 colores
            //512x256. 4 colours per pixel (2 bits per byte)


            if (video_mode==0) {

                int npixel;

                for (npixel=7;npixel>=0;npixel--) {
                    //G7 G6 G5 G4 G3 G2 G1 G0			R7 R6 R5 R4 R3 R2 R1 R0		512-pixel


                    green=((byte_leido_h))&128;
                    red=((byte_leido_l))&128;

                    byte_leido_h=byte_leido_h<<1;
                    byte_leido_l=byte_leido_l<<1;

                    if (green==0 && red==0) color1=0;
                    else if (green && red==0) color1=4;
                    else if (green==0 && red) color1=2;
                    else color1=7;

                    if (pantalla_apagada) color1=0;

                    ql_putpixel_zoom(xdestino++,y*2,color1);

                    x++;

                    scr_refresca_pantalla_ql_putpixel_aspect_ratio(x,y,color1,&xdestino,&acumulado_escala_1476);

                }

            }

        }

        //if (y==0) printf ("final x: %d\n",xdestino);
    }
}

void scr_refresca_border_rainbow_ql(unsigned int color)
{


    int x,y;

    int ancho=ql_get_display_width_with_proportion();

    //parte superior
    for (y=0;y<QL_TOP_BORDER_NO_ZOOM;y++) {
        for (x=0;x<ancho+QL_LEFT_BORDER_NO_ZOOM*2;x++) {
            ql_putpixel_rainbow(x,y,color);
        }
    }

    //parte inferior
    for (y=0;y<QL_TOP_BORDER_NO_ZOOM;y++) {
        for (x=0;x<ancho+QL_LEFT_BORDER_NO_ZOOM*2;x++) {
            ql_putpixel_rainbow(x,QL_TOP_BORDER_NO_ZOOM+y+QL_DISPLAY_HEIGHT,color);
        }
    }


    //laterales
    for (y=0;y<QL_DISPLAY_HEIGHT;y++) {
        for (x=0;x<QL_LEFT_BORDER_NO_ZOOM;x++) {
            ql_putpixel_rainbow(x,QL_TOP_BORDER_NO_ZOOM+y,color);
            ql_putpixel_rainbow(QL_LEFT_BORDER_NO_ZOOM+ancho+x,QL_TOP_BORDER_NO_ZOOM+y,color);
        }

    }


}

void scr_refresca_border_ql(unsigned int color)
{


    int x,y;

    int ancho=ql_get_display_width_with_proportion();

    //parte superior
    for (y=0;y<QL_TOP_BORDER;y++) {
        for (x=0;x<ancho*zoom_x+QL_LEFT_BORDER*2;x++) {
            scr_putpixel(x,y,color);
        }
    }

    //parte inferior
    for (y=0;y<QL_TOP_BORDER;y++) {
        for (x=0;x<ancho*zoom_x+QL_LEFT_BORDER*2;x++) {
            scr_putpixel(x,QL_TOP_BORDER+y+QL_DISPLAY_HEIGHT*zoom_y,color);
        }
    }


    //laterales
    for (y=0;y<QL_DISPLAY_HEIGHT*zoom_y;y++) {
        for (x=0;x<QL_LEFT_BORDER;x++) {
            scr_putpixel(x,QL_TOP_BORDER+y,color);
            scr_putpixel(QL_LEFT_BORDER+ancho*zoom_x+x,QL_TOP_BORDER+y,color);
        }

    }


}


void scr_refresca_pantalla_y_border_ql(void)
{
    if (rainbow_enabled.v) {
        //De momento es un falso realvideo. Las mismas rutinas de render escriben en pixeles o buffer rainbow segun si rainbow enabled o no

        //Refrescar border si conviene
        if (border_enabled.v) {

            //Dibujar border. Color 0
            scr_refresca_border_rainbow_ql(ql_border_color);
            modificado_border.v=0;


        }

        scr_refresca_pantalla_ql();

        //Y si conviene watermark
        //Lo hacemos aqui tambien porque habitualmente se hace desde cpu_loop_refresca_pantalla pero eso seguramente va antes de este refresco y se sobreescribe
        screen_add_watermark_rainbow();


        scr_refresca_pantalla_rainbow_comun();
    }

    else {

        //Refrescar border si conviene
        if (border_enabled.v) {
            if (modificado_border.v) {
                //Dibujar border. Color 0
                scr_refresca_border_ql(ql_border_color);
                modificado_border.v=0;
            }

        }

        scr_refresca_pantalla_ql();
    }
}





char ql_nombre_autorun[1024]="";

void ql_handle_boot_file(char *filename)
{

    char dir[PATH_MAX];
    util_get_dir(filename,dir);
    ql_microdrive_floppy_emulation=1;

    ql_insert_mdv_flp(QL_QDOS_UNIT_MDV1,dir);

    set_snaptape_fileoptions(filename);

    //Indicar archivo boot para hacer lrun mdv1_archivo
    if (noautoload.v==0) {
        char nombre_boot[PATH_MAX];
        util_get_file_no_directory(filename,nombre_boot);
        //Si no es boot automatico, meter nombre a cargar
        if (strcasecmp(nombre_boot,"boot")) {
            debug_printf(VERBOSE_INFO,"Autoload %s",nombre_boot);

            //Si archivo tiene . cambiar por "_"
            int i;
            for (i=0;nombre_boot[i];i++) {
                if (nombre_boot[i]=='.') nombre_boot[i]='_';
            }

            if (
                   !util_compare_file_extension_ql(filename,"exe") ||
                   !util_compare_file_extension_ql(filename,"exec") ||
                   !util_compare_file_extension(filename,"exe") ||
                   !util_compare_file_extension(filename,"exec") ) {
                        sprintf(ql_nombre_autorun,"exec mdv1_%s\x0a",nombre_boot);

            }


            else sprintf(ql_nombre_autorun,"lrun mdv1_%s\x0a",nombre_boot);
        }
        else {
            ql_nombre_autorun[0]=0;
        }
    }

}




void ql_load_and_execute(char *filename)
{

    //de momento hacer un exec tal cual, parecido a lrun
    ql_handle_boot_file(filename);
    return;


    int valor_leido_longitud=get_file_size(filename);



    if (!si_existe_archivo(filename)) {
        debug_printf(VERBOSE_ERR,"Can't find file %s",filename);
        return;
    }



    if (valor_leido_longitud==0) valor_leido_longitud=4194304; //4 MB max

    int direccion=QL_MAXIMUM_MEM_LIMIT-valor_leido_longitud-4096; //4096 de reserva


    m68k_set_reg(M68K_REG_PC,direccion);

    //algo por debajo del inicio de carga del programa
    m68k_set_reg(M68K_REG_USP,direccion-8192);

    FILE *ptr_binaryfile_load;
    ptr_binaryfile_load=fopen(filename,"rb");
    if (!ptr_binaryfile_load) {

        debug_printf (VERBOSE_ERR,"Unable to open Binary file %s",filename);


    }

    else {

        int leidos=1;
        z80_byte byte_leido;
        while (valor_leido_longitud>0 && leidos>0) {
            leidos=fread(&byte_leido,1,1,ptr_binaryfile_load);
            if (leidos>0) {

                    poke_byte_z80_moto(direccion,byte_leido);

                    direccion++;
                    valor_leido_longitud--;
            }
        }


        fclose(ptr_binaryfile_load);

    }

    char dir[PATH_MAX];
    util_get_dir(filename,dir);
    ql_microdrive_floppy_emulation=1;

    //que no haga reset etc
    int antes_no_autoload=noautoload.v;
    noautoload.v=1;

    ql_insert_mdv_flp(QL_QDOS_UNIT_MDV1,dir);
    noautoload.v=antes_no_autoload;

    set_snaptape_fileoptions(filename);


}