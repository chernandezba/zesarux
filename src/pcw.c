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

//Direcciones donde estan cada pagina de ram. 16 paginas de 16 kb cada una
z80_byte *pcw_ram_mem_table[16];





//Direcciones actuales mapeadas para lectura, bloques de 16 kb
z80_byte *pcw_memory_paged_read[4];

//Direcciones actuales mapeadas para lectura, bloques de 16 kb
z80_byte *pcw_memory_paged_write[4];


//Paginas mapeadas para lectura en los 4 segmentos, usado para lectura de teclado
z80_byte pcw_banks_paged_read[4];

//Contador de linea para lanzar interrupcion.
z80_byte pcw_scanline_counter;

z80_bit pcw_pending_interrupt={0};


z80_byte pcw_keyboard_table[16]={
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

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

z80_byte pcw_interrupt_from_pd765_type=0;

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


void pcw_set_memory_pages(void)
{


    z80_byte bank;

    int i;

    z80_byte port_f4_mask=0x10;

    for (i=0;i<4;i++) {

        //Sending the bank number (with b7 set) to one of ports &F0-&F3 selects that bank for reading and writing. 
        //Sending the bank number for writing to b0-2 of a port and the bank for reading to b4-b6 (with b7 reset) 
        //maps separate banks in for reading and writing: this can only be used for the first 8 banks.

        bank=pcw_bank_registers[i];

        if (bank & 128) {
            //PCW (“extended”) paging mode
            bank &=15;
            pcw_memory_paged_read[i]=pcw_ram_mem_table[bank];
            pcw_memory_paged_write[i]=pcw_ram_mem_table[bank];

            pcw_banks_paged_read[i]=bank;
        }
        else {
            //CPC (“standard”) paging mode
            z80_byte bank_write=bank & 7;
            z80_byte bank_read=(bank >> 4) & 7;

            //Port F4: If a memory range is set as “locked”, then the “block to read” bits are ignored; memory is read from the “block to write”.
            if (pcw_port_f4_value & port_f4_mask) {
                bank_read=bank_write;
            }

            pcw_memory_paged_read[i]=pcw_ram_mem_table[bank_read];
            pcw_memory_paged_write[i]=pcw_ram_mem_table[bank_write];

            pcw_banks_paged_read[i]=bank_read;

        }

        port_f4_mask=port_f4_mask<<1;

    }


}

void pcw_reset(void)
{
    pcw_bank_registers[0]=pcw_bank_registers[1]=pcw_bank_registers[2]=pcw_bank_registers[3]=0;
    pcw_port_f4_value=0;

    pcw_interrupt_from_pd765_type=0;

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
    for (i=0;i<16;i++) {
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

    printf("PCW set bank %d value %02XH\n",bank,value);

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
    pcw_port_f7_value=value;
    printf("PCW set port F7 value %02XH\n",value);
}


void pcw_interrupt_from_pd765(void)
{
    if (pcw_interrupt_from_pd765_type==1) {
        printf("Generate NMI triggered from pd765\n");
        generate_nmi();
        //sleep(2);
        
    }
        
    //TODO Revisar esto. solo se genera cuando esta EI???
    if (pcw_interrupt_from_pd765_type==2 && iff1.v) {
        printf("Generate Maskable interrupt triggered from pd765\n");
        interrupcion_maskable_generada.v=1;
        //sleep(2);
        
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

        case 2:
            printf("Connect FDC to NMI\n");
            pcw_interrupt_from_pd765_type=1;
            //sleep(5);
        break;

        case 3:
            printf("Connect FDC to standard interrupts\n");
            pcw_interrupt_from_pd765_type=2;
            //sleep(5);
        break;

        case 4:
            printf("Connect FDC to nothing\n");
            pcw_interrupt_from_pd765_type=0;
            //sleep(5);
        break;


        case 5:
            printf("Set Terminal count\n");
            pd765_set_terminal_count_signal();
        break;

        case 6:
            printf("Reset Terminal count\n"); 
            pd765_reset_terminal_count_signal();
        break;


        case 9:
            dsk_show_activity();
            pd765_motor_on();        
        break;

        case 10:
            pd765_motor_off();
        break;

        case 11:
            //temporal
            printf("set beeper\n");
            value_beeper=100;
        break;

        case 12:
            //temporal
            printf("reset beeper\n");
            value_beeper=0;
        break;

    }
   
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

    return_value=pcw_keyboard_table[fila];

    if (fila==0xF) return_value=128;



    printf("PCW return read row %XH value %02XH reg_pc=%04XH\n",fila,return_value,reg_pc);
    //sleep(1);


   
    return return_value;
}


void scr_refresca_border_pcw(unsigned int color)
{
    /*
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
*/
}


void scr_refresca_pant_pcw_return_line_pointer(z80_byte roller_ram_bank,z80_int roller_ram_offset,
    z80_byte *address_block, z80_int *address)
{
    z80_byte *puntero_roller_ram;

    puntero_roller_ram=pcw_memory_paged_read[roller_ram_bank];
    puntero_roller_ram +=roller_ram_offset;    

    z80_int valor=*puntero_roller_ram+256*puntero_roller_ram[1];

    //descomponer en bloque y addres

    *address_block=(valor>>13) & 0x07;

    //15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
    //block   |  offset /16                 | offset

    *address=(valor & 7) +2 *(valor & 0x1FF8);
}

//Refresco sin rainbow
void scr_refresca_pantalla_pcw(void)
{

    

    //Address of roller RAM. b7-5: bank (0-7). b4-1: address / 512.
    z80_byte roller_ram_bank=(pcw_port_f5_value >> 5) & 0x07;

    z80_int roller_ram_offset=(pcw_port_f5_value & 0x1F) * 512;

    printf("Roller ram: bank: %02XH Offset: %02XH\n",roller_ram_bank,roller_ram_offset);



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

                
                if (x<9) {
                    printf("Line %d x %d Block %02XH Address %04XH\n",yfinal,x,address_block,address);
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