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

#include "pcw.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "contend.h"
#include "joystick.h"
#include "zxvision.h"
#include "operaciones.h"
#include "utils.h"
#include "audio.h"

#include "dsk.h"
#include "pd765.h"
#include "ula.h"

/*

Info PCW:

Información pcw

Amstrad PCW

Boot

PCW boot sequence

Boot mister

Amstrad-PCW_MiSTer/boot_loader.sv at master · MiSTer-devel/Amstrad-PCW_MiSTer · GitHub

Emulador

JOYCE for UNIX

https://www.habisoft.com/pcwwiki/doku.php?id=en:sistema:indice

Más hardware

https://www.seasip.info/Unix/Joyce/hardware.pdf
￼


*/

//Direcciones donde estan cada pagina de ram. 128 paginas de 16 kb cada una
//asignamos 2 MB para tener el máximo de memoria ya disponible
z80_byte *pcw_ram_mem_table[PCW_MAX_RAM_PAGES];





//Direcciones actuales mapeadas para lectura, bloques de 16 kb
z80_byte *pcw_memory_paged_read[4];

//Direcciones actuales mapeadas para lectura, bloques de 16 kb
z80_byte *pcw_memory_paged_write[4];


//Paginas mapeadas para lectura en los 4 segmentos, usado para lectura de teclado y en debug
z80_byte pcw_banks_paged_read[4];

//Contador de linea para lanzar interrupcion.
z80_byte pcw_scanline_counter;

z80_bit pcw_pending_interrupt={0};


z80_byte pcw_keyboard_table[16]={
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

z80_byte pcw_interrupt_from_pd765_type=0;

//Total ram. Para un 8256, 256k. Para un 8512, 512k
int pcw_total_ram=256*1024;

//Tabla de colores
int pcw_rgb_table[2]={

    0x000000, //negro
    0x41FF00  //verde. Tipico color de green "P1" phosphor
};

//Mostrar colores en blanco y negro
z80_bit pcw_black_white_display={0};


//Siempre ver la pantalla aunque bit de F7 diga lo contrario
z80_bit pcw_always_on_display={0};

//No dejar que se invierta el color
z80_bit pcw_do_not_inverse_display={0};

//
// Inicio de variables necesarias para preservar el estado (o sea las que tienen que ir en un snapshot)
//

//Registros de bancos para F0,F1,F2,F3
z80_byte pcw_bank_registers[4];

//Port F4 If a memory range is set as “locked”, then the “block to read” bits are ignored; memory is read from the “block to write”.
//Bit 7: lock C000..FFFF
//Bit 6: lock 8000..BFFF
//Bit 5: lock 4000..7FFF
//Bit 4: lock 0..3FFF
//Bits 3-0: unused
z80_byte pcw_port_f4_value;

//Address of roller RAM. b7-5: bank (0-7). b4-1: address / 512.
z80_byte pcw_port_f5_value;

//Vertical screen position
z80_byte pcw_port_f6_value;

//b7: reverse video. b6: screen enable.
z80_byte pcw_port_f7_value;

z80_byte pcw_port_f8_value;

//z80_byte pcw_interrupt_counter;

//
// Fin de variables necesarias para preservar el estado (o sea las que tienen que ir en un snapshot)
//


z80_byte *pcw_get_memory_offset_read(z80_int dir)
{
    int segmento=dir/16384;

    int offset=dir & 16383;

    z80_byte *puntero;

    puntero=pcw_memory_paged_read[segmento];

    puntero +=offset;

    return puntero;
}

z80_byte *pcw_get_memory_offset_write(z80_int dir)
{
    int segmento=dir/16384;

    int offset=dir & 16383;

    z80_byte *puntero;

    puntero=pcw_memory_paged_write[segmento];

    puntero +=offset;

    return puntero;
}

z80_byte pcw_get_mask_bank_ram(void)
{
    //Si pcw_total_ram=256*1024, mascara 15
    //Si pcw_total_ram=512*1024, mascara 31
    
    int max_banks=pcw_total_ram/16384;

    return max_banks-1;
}

void pcw_set_memory_pages(void)
{


    z80_byte bank;

    int i;

    //z80_byte port_f4_mask=0x10;

    for (i=0;i<4;i++) {

        //Sending the bank number (with b7 set) to one of ports &F0-&F3 selects that bank for reading and writing. 
        //Sending the bank number for writing to b0-2 of a port and the bank for reading to b4-b6 (with b7 reset) 
        //maps separate banks in for reading and writing: this can only be used for the first 8 banks.

        bank=pcw_bank_registers[i];

        //TODO. algo no he entendido bien....  gonzalezz y otros de opera
        //hacen out F3,3, cuando realmente lo que quieren hacer es paginar en el modo extendido (sin tener el bit 7 alzado)
        //Con eso cargan los de opera, batman y demas...
        //bank |=128;
        

        if (bank & 128) {
            //PCW (“extended”) paging mode
            bank &=pcw_get_mask_bank_ram();
            //printf("mask %d\n",pcw_get_mask_bank_ram());
            pcw_memory_paged_read[i]=pcw_ram_mem_table[bank];
            pcw_memory_paged_write[i]=pcw_ram_mem_table[bank];

            pcw_banks_paged_read[i]=bank;
        }
        else {
            //CPC (“standard”) paging mode
            //Juegos de Opera Soft, Batman etc usan este modo
            z80_byte bank_write=bank & 7;
            z80_byte bank_read=(bank >> 4) & 7;

            //Port F4: If a memory range is set as “locked”, then the “block to read” bits are ignored; memory is read from the “block to write”.
            /*
                    D7              RW3  (CPU bank C000h to FFFFh)
                    D6              RW0  (CPU bank 0000h to 3FFFh)
                    D5              RW2  (CPU hank B000h to BFFFh)
	                D4		RW1  (CPU bank 4000h to 7FFFh)
            */
            z80_byte port_f4_mask;
            switch(i) {
                case 0:
                    port_f4_mask=0x40;
                break;

                case 1:
                    port_f4_mask=0x10;
                break;

                case 2:
                    port_f4_mask=0x20;
                break;

                default:
                    port_f4_mask=0x80;
                break;
            }
            if (pcw_port_f4_value & port_f4_mask) {
                //printf("memory locked\n");
                //sleep(2);
                bank_read=bank_write;
            }

            pcw_memory_paged_read[i]=pcw_ram_mem_table[bank_read];
            pcw_memory_paged_write[i]=pcw_ram_mem_table[bank_write];

            pcw_banks_paged_read[i]=bank_read;

        }

        //port_f4_mask=port_f4_mask<<1;

    }


}

void pcw_reset(void)
{
    //pcw_bank_registers[0]=pcw_bank_registers[1]=pcw_bank_registers[2]=pcw_bank_registers[3]=0;

    pcw_bank_registers[0]=0x80;
    pcw_bank_registers[1]=0x81;
    pcw_bank_registers[2]=0x82;
    pcw_bank_registers[3]=0x83; 

    //Importante esto en arranque, si no, no van juegos de Opera por ejemplo
    pcw_port_f4_value=0xF1;

    pcw_port_f5_value=0;
    pcw_port_f6_value=0;
    pcw_port_f7_value=0;
    pcw_port_f8_value=0;

    pcw_interrupt_from_pd765_type=0;

    //pcw_interrupt_counter=0;

    pd765_set_terminal_count_signal();

    pcw_scanline_counter=0;

    pcw_pending_interrupt.v=0;

    pcw_set_memory_pages();

    //Recargar contenido del boot de RAM
    rom_load(NULL);
}

void pcw_init_memory_tables(void)
{
	debug_printf (VERBOSE_DEBUG,"Initializing pcw memory tables");

    z80_byte *puntero;
    puntero=memoria_spectrum;


    int i;
    for (i=0;i<PCW_MAX_RAM_PAGES;i++) {
            pcw_ram_mem_table[i]=puntero;
            puntero +=16384;
    }

}

void pcw_out_port_bank(z80_byte puerto_l,z80_byte value)
{
/*

&F0	O	Select bank for &0000
&F1	O	Select bank for &4000
&F2	O	Select bank for &8000
&F3	O	Select bank for &C000. Usually &87.
    */

    int bank=puerto_l-0xF0;



    pcw_bank_registers[bank]=value;

    //printf("PCW set bank %d value %02XH\n",bank,value);

    //if (bank==0) sleep(1);

    pcw_set_memory_pages();


}

void pcw_out_port_f4(z80_byte value)
{
    pcw_port_f4_value=value;
    printf("PCW set port F4 value %02XH\n",value);

    pcw_set_memory_pages();
}

void pcw_out_port_f5(z80_byte value)
{
    pcw_port_f5_value=value;
    printf("PCW set port F5 value %02XH\n",value);
}

void pcw_out_port_f6(z80_byte value)
{
    pcw_port_f6_value=value;
    printf("PCW set port F6 value %02XH\n",value);
}


void pcw_out_port_f7(z80_byte value)
{
    /*if ((pcw_port_f7_value & 128) != (value&128)) {
        printf("Cambio inversion\n");
        sleep(1);
    }*/

    pcw_port_f7_value=value;
    //printf("PCW set port F7 value %02XH\n",value);

    
}


void pcw_interrupt_from_pd765(void)
{
    if (pcw_interrupt_from_pd765_type==1) {
        printf("Generate NMI triggered from pd765\n");
        //
        generate_nmi();
        //sleep(2);

        //TODO: se supone que se desactivan todas al recibir una nmi
        /*
        Command 2 will enable non-maskable interrupts from the Floppy
        Disc Controller until Command 3 is issued to enable maskable
        interrupts instead, or Command 4 is issued to disable all
        interrupts from the Floppy Disc Controller, or until the first
        non-maskable interrupt occurs.  On power-up and system reset all
        Floppy Disc Controller Interrupts are disabled.
            */

        //TODO: No estoy seguro de lo siguiente
        //pcw_interrupt_from_pd765_type=0;
        //sleep(2);
        
    }
        
    //TODO Revisar esto. solo se genera cuando esta EI???
    if (pcw_interrupt_from_pd765_type==2) {
        printf("Generate Maskable interrupt triggered from pd765\n");
        //If the Z80 has disabled interrupts, the interrupt line stays high until the Z80 enables them again.
        pcw_pending_interrupt.v=1;

        //temp
        //pcw_interrupt_from_pd765_type=0;
        //sleep(3);
        
    }
}

void pcw_out_port_f8(z80_byte value)
{
    
    printf("PCW set port F8 value %02XH reg_pc %04XH\n",value,reg_pc);

    /*
    0 end bootstrap, 
    1 reboot, 
    2/3/4 connect FDC to NMI/standard interrupts/neither, 
    5/6 set/clear FDC terminal count, 
    7/8 screen on/off (for external video), 
    9/10 disc motor on/off, 
    11/12 beep on/off
    */

    switch (value) {
        case 0:
            printf("End bootstrap\n");
            //sleep(2);
        break;

        case 1:
            printf("Reboot\n");
            //sleep(2);

            reg_pc=0;

            //Recargar contenido del boot de RAM
            rom_load(NULL);            
        break;

        /*
        Command 2 will enable non-maskable interrupts from the Floppy
        Disc Controller until Command 3 is issued to enable maskable
        interrupts instead, or Command 4 is issued to disable all
        interrupts from the Floppy Disc Controller, or until the first
        non-maskable interrupt occurs.  On power-up and system reset all
        Floppy Disc Controller Interrupts are disabled.
        */
        case 2:
            printf("Connect FDC to NMI\n");
            pcw_interrupt_from_pd765_type=1;
            //sleep(2);
        break;

        case 3:
            printf("Connect FDC to standard interrupts\n");
            pcw_interrupt_from_pd765_type=2;
            //sleep(2);
        break;

        case 4:
            printf("Connect FDC to nothing\n");
            pcw_interrupt_from_pd765_type=0;
            //sleep(2);
        break;


        case 5:
            printf("Set Terminal count\n");
            pd765_set_terminal_count_signal();
        break;

        case 6:
            printf("Reset Terminal count\n"); 
            //Tal y como tenemos implementado el set, esto ya no tiene sentido
            //pd765_reset_terminal_count_signal();
        break;


        case 9:
            dsk_show_activity();
            pd765_motor_on();        
        break;

        case 10:
            pd765_motor_off();
        break;

        case 11:
            
            //printf("set beeper\n");
            value_beeper=100;
        break;

        case 12:
            
            //printf("reset beeper\n");
            value_beeper=0;
        break;

    }
   
}

void pcw_increment_interrupt_counter(void)
{
    //printf("scanline: %d F8 port: %02XH\n",t_scanline,pcw_port_f8_value);
    z80_byte pcw_interrupt_counter=pcw_port_f8_value & 0xF;

    if (pcw_interrupt_counter!=0x0F) pcw_interrupt_counter++;

    pcw_port_f8_value &=0xF0;
    pcw_port_f8_value |=pcw_interrupt_counter;

}

z80_byte pcw_get_port_f8_value(void)
{
    z80_byte return_value=pcw_port_f8_value;

    if (pd765_interrupt_pending) {
        
        printf("Return F8 FDC interrupt\n");

        //printf("pd765_terminal_count_signal.v: %d\n",pd765_terminal_count_signal.v);
        return_value|=0x20;
        
        //sleep(1);
    }
    else {
        //printf("Return F8 FDC NO interrupt\n");
    }

    //bit 6 Frame flyback; this is set while the screen is not being drawn
    //TODO: de momento calculo chapucero
    //printf("t_scanline %d\n",t_scanline);
    if (t_scanline_draw<56) return_value |=0x40;

    return return_value;

}

z80_byte pcw_in_port_f8(void)
{
    //printf("LEE puerto F8H\n");
    return pcw_get_port_f8_value();
        

}


z80_byte pcw_in_port_f4(void)
{

    //printf("LEE puerto F4\n");

    z80_byte return_value=pcw_get_port_f8_value();

    pcw_port_f8_value &=0xF0;



    return return_value;

}

z80_byte pcw_in_port_fd(void)
{
    //printf("LEE puerto FDH on PC=%04XH\n",reg_pc);
    //Impresora. TODO completar    
    return 0x40; //bit 6: If 1, printer has finished.
}

int pcw_keyboard_ticker_update_counter;

void pcw_keyboard_ticker_update(void)
{
	pcw_keyboard_ticker_update_counter++;


    //3FFFh bit 6 toggles with each update from the keyboard to the PCW.
    //3FFFh bit 7 is 1 if the keyboard is currently transmitting its state to the PCW, 0 if it is scanning its keys.
	pcw_keyboard_table[15] &= 0x3F;
	if (pcw_keyboard_ticker_update_counter & 1) pcw_keyboard_table[15] |= 0x80;
	if (pcw_keyboard_ticker_update_counter & 2) pcw_keyboard_table[15] |= 0x40;    
}

z80_byte pcw_read_keyboard(z80_int dir)
{
    /*
The PCW's keyboard is directly mapped into the last 16 bytes of bank 3, even when interrupts are disabled. 
Each key is reflected by one bit in bytes &3FF0-&3FFA.
b7:   k2     k1     [+]    .      ,      space  V      X      Z      del<   alt
b6:   k3     k5     1/2    /      M      N      B      C      lock          k.
b5:   k6     k4     shift  ;      K      J      F      D      A             enter
b4:   k9     k8     k7     ¤      L      H      G      S      tab           f8
b3:   paste  copy   #      P      I      Y      T      W      Q             [-]
b2:   f2     cut    return [      O      U      R      E      stop          can
b1:   k0     ptr    ]      -      9      7      5      3      2             extra
b0:   f4     exit   del>   =      0      8      6      4      1             f6
      &3FF0  &3FF1  &3FF2  &3FF3  &3FF4  &3FF5  &3FF6  &3FF7  &3FF8  &3FF9  &3FFA
Bytes &3FFB-&3FFF reflect the keyboard in a different, incomplete way. These bytes are also used by Creative Technology's KeyMouse (in its standard "MicroDesign mode") and the Teqniche 102-key keyboard to provide additional functionality, creating some incompatibilities along the way. Among the more interesting mappings are the following:
&3FFB	Standard keyboard  
KeyMouse	b7-b0 unused (0)
b6-b0 horizontal movement counter.
&3FFC	KeyMouse	b7-b6 high bits of vertical movement counter.
&3FFD	All
Standard keyboard 
KeyMouse	b7 always set; b6 current state of SHIFT LOCK 
b3-b0 cursor keys, b4 matrix key 
b3-b0 low bits of vertical movement counter.
&3FFE	KeyMouse	b7 left button, b6 right button.    
    */

    //Quedarnos con ultimo byte de la direccion
    int fila=dir & 0xF;

    //de momento
    z80_byte return_value=0;

    //Teclas al pulsar activan bit

    
    /*

3FFFh bit 7 is 1 if the keyboard is currently transmitting its state to the PCW, 0 if it is scanning its keys.
If no keyboard is present, all 16 bytes of the memory map are zero.
    */

    //temp
    //if (fila==8) return_value=temp_row_8;
    //if (fila==5) return_value=temp_row_5;


    //si estamos en el menu, no devolver tecla
    if (!zxvision_key_not_sent_emulated_mach() ) {

        return_value=pcw_keyboard_table[fila];

    }

    //if (fila==0xF) sleep(1);

    //3FFDh bit 7 is 0 if LK2 is present, 1 if not.
    
    if (fila==0xD) return_value=128;



    //printf("PCW return read row %XH value %02XH reg_pc=%04XH\n",fila,return_value,reg_pc);
    //sleep(1);


   
    return return_value;
}



void pcw_putpixel_border(int x,int y,unsigned int color)
{

    scr_putpixel(x,y,color);

}

void scr_refresca_border_pcw(unsigned int color)
{


    int ancho_pantalla=PCW_DISPLAY_WIDTH;
    int alto_pantalla=PCW_DISPLAY_HEIGHT;

	int ancho_border=PCW_LEFT_BORDER_NO_ZOOM;
	int alto_border=PCW_TOP_BORDER_NO_ZOOM;

	//printf ("ancho pantalla: %d alto_pantalla: %d offset_x_pantalla: %d anchoborder: %d altoborder: %d\n",ancho_pantalla,alto_pantalla,offset_x_pantalla,ancho_border,alto_border);



    int x,y;

	int x_borde_derecho=(ancho_border+ancho_pantalla)*zoom_x;
	//printf ("x borde derecho: %d total ventana: %d\n",x_borde_derecho,(640+CPC_LEFT_BORDER_NO_ZOOM*2)*zoom_x);


    //parte superior e inferior
    for (y=0;y<alto_border*zoom_y;y++) {
        for (x=0;x<(PCW_DISPLAY_WIDTH+PCW_LEFT_BORDER_NO_ZOOM*2)*zoom_x;x++) {
        //printf ("x: %d y: %d\n",x,y);
        pcw_putpixel_border(x,y,color);
        pcw_putpixel_border(x,(alto_border+alto_pantalla)*zoom_y+y,color);
        }
    }

    //laterales
    
    for (y=0;y<alto_pantalla*zoom_y;y++) {
        for (x=0;x<ancho_border*zoom_x;x++) {
            pcw_putpixel_border(x,alto_border*zoom_y+y,color);
            pcw_putpixel_border(x_borde_derecho+x,alto_border*zoom_y+y,color);
        }

    }
        

}




void scr_refresca_pant_pcw_return_line_pointer(z80_byte roller_ram_bank,z80_int roller_ram_offset,
    z80_byte *address_block, z80_int *address)
{
    z80_byte *puntero_roller_ram;

    puntero_roller_ram=pcw_ram_mem_table[roller_ram_bank];
    puntero_roller_ram +=roller_ram_offset;    

    z80_int valor=*puntero_roller_ram+256*puntero_roller_ram[1];

    //descomponer en bloque y addres

    *address_block=(valor>>13) & 0x07;

    //15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
    //block   |  offset /16                 | offset

    *address=(valor & 7) +2 *(valor & 0x1FF8);
}

int pcw_get_rgb_color(int i)
{
    //0 o 1
    if (pcw_black_white_display.v) {
        return (i ? 7 : 0);
    }

    else return (i ? PCW_COLOUR_GREEN : PCW_COLOUR_BLACK);
}

void pcw_refresca_putpixel(int x,int y,int color)
{
    y*=2;

    int color_final=pcw_get_rgb_color(color);

    scr_putpixel_zoom(x,y,color_final);
    scr_putpixel_zoom(x,y+1,color_final);
}

//Refresco sin rainbow
void scr_refresca_pantalla_pcw(void)
{


    //Address of roller RAM. b7-5: bank (0-7). b4-1: address / 512.
    z80_byte roller_ram_bank=(pcw_port_f5_value >> 5) & 0x07;

    z80_int roller_ram_offset=(pcw_port_f5_value & 0x1F) * 512;

    //printf("Roller ram: bank: %02XH Offset: %02XH\n",roller_ram_bank,roller_ram_offset);

    //TODO Vertical screen position puerto F6
    int x,y,scanline;

    for (y=0;y<256;y+=8) {
        z80_byte address_block;
        z80_int address;


        //roller_ram_offset+=2;
        for (x=0;x<720;x+=8) {
            
            for (scanline=0;scanline<8;scanline++) {
                int yfinal=y+scanline;

                scr_refresca_pant_pcw_return_line_pointer(roller_ram_bank,roller_ram_offset+yfinal*2,&address_block,&address);

                address +=x;


                z80_byte *puntero_byte=pcw_ram_mem_table[address_block]+address;

                z80_byte byte_leido=*puntero_byte;

                int bit;

                for (bit=0;bit<8;bit++) {
                    int pixel;

                    if (byte_leido & 128) pixel=1;
                    else pixel=0;

                    //Reverse video
                    if ((pcw_port_f7_value & 0x80) && pcw_do_not_inverse_display.v==0) {
                        //printf("antes pixel :%d\n",pixel);
                        
                        pixel ^=1;
                        //printf("despues pixel :%d\n",pixel);

                        //sleep(1);
                    }

                    //Si pantalla no activa
                    if (!(pcw_port_f7_value & 0x40) && pcw_always_on_display.v==0) pixel=0;

                    pcw_refresca_putpixel(x+bit,y+scanline,pixel);

                    byte_leido=byte_leido<<1;
                }
            }
        }
    }
}

void scr_refresca_pantalla_y_border_pcw_no_rainbow(void)
{

    //Refrescar border si conviene
    if (border_enabled.v) {
        if (modificado_border.v) {
            //Dibujar border. Color 0
            //unsigned int color=pcw_border_color;

            //TODO
            unsigned int color=0;

            //color=cpc_palette_table[color];
            //color +=CPC_INDEX_FIRST_COLOR;

            scr_refresca_border_pcw(color);
            modificado_border.v=0;
        }
	}


    scr_refresca_pantalla_pcw();
}



void scr_refresca_pantalla_y_border_pcw(void)
{

    //TODO : de momento sin rainbow
    scr_refresca_pantalla_y_border_pcw_no_rainbow();

    /*
    if (rainbow_enabled.v) {
        scr_refresca_pantalla_y_border_pcw_rainbow();
    }
    else {
        scr_refresca_pantalla_y_border_pcw_no_rainbow();
    }
    */
}