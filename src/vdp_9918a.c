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


#include "vdp_9918a.h"
#include "cpu.h"
#include "debug.h"
#include "screen.h"
#include "settings.h"
#include "vdp_9918a_sms.h" 

/*
Nota: parece que en el documento chipstms9918 la numeracion de bits está al revés, por ejemplo,
registro 1 bit 6 (BL disables the screen display when reseted.VDP's commands work a bit faster as well. Screen display is displayed by default.)
en el documento aparece mal como bit 1
*/


//Antes de agregar emulacion de sms, esto era un array de [8]
z80_byte vdp_9918a_registers[VDP_9918A_TOTAL_REGISTERS];

z80_byte vdp_9918a_status_register=255;


//Ultimos dos bytes enviados al puerto de comando/status
z80_byte vdp_9918a_last_command_status_bytes[2];
//Contador a ultima posicion
z80_byte vdp_9918a_last_command_status_bytes_counter=0;

//Ultimos 3 bytes enviados al puerto de datos. Realmente el 0 y 1 es el address pointer actual, y el 3 el ultimo byte enviado
z80_byte vdp_9918a_last_vram_bytes[3];
//Contador a ultima posicion
//int vdp_9918a_last_vram_bytes_counter=0;

z80_int vdp_9918a_last_vram_position;


//Forzar desde menu a desactivar capas 
z80_bit vdp_9918a_force_disable_layer_ula={0};
z80_bit vdp_9918a_force_disable_layer_sprites={0};
z80_bit vdp_9918a_force_disable_layer_border={0};


//Forzar a dibujar capa con color fijo, para debug
z80_bit vdp_9918a_reveal_layer_ula={0};
z80_bit vdp_9918a_reveal_layer_sprites={0};


void vdp_9918a_reset(void)
{
    int i;

    for (i=0;i<VDP_9918A_TOTAL_REGISTERS;i++) vdp_9918a_registers[i]=0;


    if (MACHINE_IS_SMS) vdp_9918a_sms_reset();

    vdp_9918a_last_command_status_bytes_counter=0;

}

void vdp_9918a_out_vram_data(z80_byte *vram_memory,z80_byte value)
{

//printf ("%c",(value>=32 && value<=126 ? value : '?') );
    int posicion_escribir=vdp_9918a_last_vram_position & 16383;

    vram_memory[posicion_escribir]=value;

    vdp_9918a_last_vram_position++;

    //printf("Escribiendo en posicion vram %d\n",posicion_escribir);

    //temp. simular retardo al escribir en vram
    //t_estados +=7;


}


z80_byte vdp_9918a_in_vram_data(z80_byte *vram_memory)
{

//printf ("%c",(value>=32 && value<=126 ? value : '?') );
    int posicion_leer=vdp_9918a_last_vram_position & 16383;

    z80_byte value=vram_memory[posicion_leer];

    vdp_9918a_last_vram_position++;

    //temp. simular retardo al leer de vram
    //t_estados +=7;    

    return value;


}

z80_byte vdp_9918a_in_vdp_status(void)
{
    //7 6  5 43210
    //F 5S C Fifth sprite number
    //F: interrupt pending flag
    //5S: fifth sprite flag
    //C: sprite colision (coincidence) flag
    //fifth sprite number


    //Activar real video al leer este registro, si esta autoenable
    if (rainbow_enabled.v==0 && autodetect_rainbow.v) {
        //Activar realvideo. Ya que asi podrá haber lectura de maximo sprites en linea
        debug_printf (VERBOSE_INFO,"Enabling realvideo due to VDP status register reading");

        enable_rainbow();
    }

    z80_byte retorno=vdp_9918a_status_register;


    //The IRQ flag (bit 7) and the collision flag (bit 5) get cleared after reading Status register
    vdp_9918a_status_register&= (255-32-128);

    //Pero... al parecer si borramos tambien bit 7, en MSX Rom no lee teclado
    //vdp_9918a_status_register&= (255-32);    

    return retorno;

}

z80_byte vdp_9918a_previous_video_mode_splash=255;

void vdp_9918a_out_command_status(z80_byte value)
{
    //printf ("vdp_9918a write status: %02XH position: %d\n",value,vdp_9918a_last_command_status_bytes_counter);

    //por defecto
    sms_writing_cram=0;

    switch (vdp_9918a_last_command_status_bytes_counter) {
        case 0:
            vdp_9918a_last_command_status_bytes[0]=value;
            vdp_9918a_last_command_status_bytes_counter=1;
        break;

        case 1:
            vdp_9918a_last_command_status_bytes[1]=value;
            vdp_9918a_last_command_status_bytes_counter=0;

            //Recibido los dos bytes. Ver que comando es
            if ( (vdp_9918a_last_command_status_bytes[1] &  (128+64)) == 64 ) {
                //printf ("Write VDP Address setup.\n");

                //vdp_9918a_last_vram_position=(vdp_9918a_last_command_status_bytes[1] & 63) | (vdp_9918a_last_command_status_bytes[0]<<6);


                vdp_9918a_last_vram_position=(vdp_9918a_last_command_status_bytes[0]) | ((vdp_9918a_last_command_status_bytes[1] & 63)<<8);
                //printf ("Write VDP Address setup to VRAM write. address: %04XH\n",vdp_9918a_last_vram_position);
            }

            if ( (vdp_9918a_last_command_status_bytes[1] &  (128+64)) == 0 ) {
                

                vdp_9918a_last_vram_position=(vdp_9918a_last_command_status_bytes[0]) | ((vdp_9918a_last_command_status_bytes[1] & 63)<<8);
                //printf ("Write VDP Address setup to VRAM read. address: %04XH\n",vdp_9918a_last_vram_position);
            }            

            if ( (vdp_9918a_last_command_status_bytes[1] &  (128+64)) == 128 ) {
                //printf ("Write VDP Register setup.\n");

                //vdp_9918a_last_vram_position=(vdp_9918a_last_command_status_bytes[1] & 63) | (vdp_9918a_last_command_status_bytes[0]<<6);
                z80_byte vdp_register=vdp_9918a_last_command_status_bytes[1] & (VDP_9918A_TOTAL_REGISTERS-1); 

                z80_byte next_value=vdp_9918a_last_command_status_bytes[0];
                
                //Casos para Master System modo video 4
                if (vdp_9918a_si_sms_video_mode4() ) next_value=vdp_9918a_sms_pre_write_reg(vdp_register,next_value);

                vdp_9918a_registers[vdp_register]=next_value;

                //Cambio color o bits de modo, actualizar border
                if (vdp_register==0 || vdp_register==1 || vdp_register==7) {
                    modificado_border.v=1;
                    //printf ("modificado border: %d\n",vdp_9918a_registers[7] &15);
                }

                //Splash de cambio de modo
                if (vdp_register==0 || vdp_register==1) {
                    z80_byte video_mode=vdp_9918a_get_video_mode();
                    if (video_mode!=vdp_9918a_previous_video_mode_splash) {
                        vdp_9918a_previous_video_mode_splash=video_mode;
                        char buffer_mensaje[256];
                        sprintf(buffer_mensaje,"Setting video mode %s",get_vdp_9918_string_video_mode());
                        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);

                    }
                }
                

                //printf ("Write VDP Register register: %02XH value %02XH\n",vdp_register,vdp_9918a_last_command_status_bytes[0]);

	//z80_byte video_mode_m3=(vdp_9918a_registers[0]>>1)&1;

	//z80_byte video_mode_m12=(vdp_9918a_registers[1]>>2)&(2+4);

	//z80_byte video_mode=video_mode_m12 | video_mode_m3;

	//printf ("video_mode: %d\n",video_mode);  

            }     


            //Paleta colores SMS
            if ( (vdp_9918a_last_command_status_bytes[1] &  (128+64)) == 192  && MACHINE_IS_SMS) {
                vdp_9918a_sms_set_writing_cram(vdp_9918a_last_command_status_bytes[0]);
            }       
        break;
    }


}

/*
  M1 M2 M3 Screen format
  0  0  0  Half text  32x24             (Mode 0 - Graphic 1)
  1  0  0  Text       40x24             (Mode 1 - Text)
  0  0  1  Hi resolution 256x192        (Mode 2 - Graphic 2)
  0  1  0  Multicolour  4x4pix blocks   (Mode 3 - Multicolor)
*/

//                              01234567890123456789012345678901
const char *s_vdp_9918a_video_mode_0="0 - Text 40x24";
const char *s_vdp_9918a_video_mode_1="1 - Text 32x24";
const char *s_vdp_9918a_video_mode_2="2 - Graphic 256x192";
const char *s_vdp_9918a_video_mode_3="3 - Graphic 64x48";


z80_byte vdp_9918a_get_video_mode(void)
{

	z80_byte video_mode_m3=(vdp_9918a_registers[0]>>1)&1;

	z80_byte video_mode_m12=(vdp_9918a_registers[1]>>2)&(2+4);

	z80_byte video_mode=video_mode_m12 | video_mode_m3;

    //Modo "especial" de SMS llamado 4, aqui se retorna como 128
    if (vdp_9918a_si_sms_video_mode4() ) {
        //printf("Modo 4 SMS\n");
        return 128;
    }


    return video_mode;
}


z80_int vdp_9918a_get_pattern_name_table(void)
{
    if (vdp_9918a_si_sms_video_mode4() ) {
        return vdp_9918a_sms_get_pattern_name_table();
    }
    else return (vdp_9918a_registers[2]&15) * 0x400; 
}



char *get_vdp_9918_string_video_mode(void) 
{


	//Por defecto
	const char *string_mode=s_vdp_9918a_video_mode_0;


	z80_byte video_mode=vdp_9918a_get_video_mode();

	
	switch(video_mode) {

		case 0:
            string_mode=s_vdp_9918a_video_mode_1;
        break;


		case 1:

			string_mode=s_vdp_9918a_video_mode_2;
		break;


		case 2:
			string_mode=s_vdp_9918a_video_mode_3;
		break;

		case 128:
			string_mode=s_vdp_9918a_video_mode_sms_4;
		break;        
    }

    return (char *)string_mode;

}

//Funciones que se usan en el tile navigator
int vdp_9918a_get_tile_width(void)
{
    z80_byte video_mode=vdp_9918a_get_video_mode();

    //por defecto
    int width=40;

	switch(video_mode) {

		case 0:
        case 1:
        case 2:
        //Incluso en 64x48, la definicion del tile es 32x24

            width=32;
		break;

        //SMS video mode 4
        case 128:
            width=32;
        break;


    }

    return width;   

    
}

//Color border/background
z80_byte vdp_9918a_get_border_color(void)
{

    return vdp_9918a_registers[7] & 15;
}

z80_byte vdp_9918a_get_foreground_color(void)
{
    z80_byte ink=(vdp_9918a_registers[7]>>4)&15;
    
    return ink;
}

//Usado en el visor de tiles
int vdp_9918a_get_tile_heigth(void)
{
    //z80_byte video_mode=vdp_9918a_get_video_mode();

    //por defecto
    int height=24;

    if (vdp_9918a_si_sms_video_mode4()) {
        height=28;
    }

    return height;

    //Incluso en 64x48, la definicion del tile es 32x24

/*
	switch(video_mode) {


		case 2:
			heigth=48;
		break;
    }

    return heigth;   
    */

    
}




z80_byte vdp_9918a_read_vram_byte(z80_byte *vram,z80_int address)
{
    //Siempre leer limitando a 16 kb
    return vram[address & 16383];
}

z80_int vdp_9918a_get_pattern_color_table(void)
{
    z80_int pattern_color_table=(vdp_9918a_registers[3]) * 0x40;

    return pattern_color_table;
}

z80_int vdp_9918a_get_pattern_base_address(void)
{

    z80_int pattern_base_address=(vdp_9918a_registers[4]&7) * 0x800; 

    if (vdp_9918a_si_sms_video_mode4()) {
        //TODO. esto siempre asi??
        pattern_base_address=0;
    }

    return pattern_base_address;
}

int vdp_9918a_get_sprite_size(void)
{
/*
This register can be only written. Use the MSX-BASIC instruction VDP(1) to access it. The instruction reads the system variable REG1SAV (0F3E0h) to return the requested value.

        Bit 7	Bit 6	Bit 5	Bit 4	Bit 3	Bit 2	Bit 1	Bit 0	
R#1:	4/16K	BL	    IE0	    M2	    M1	    0	    SI  	MAG	

4/16K selects VRAM configuration. Write 1 if the VDP is not a V9938 nor V9958.

BL disables the screen display when reseted.VDP's commands work a bit faster as well. Screen display is displayed by default.

IE0 enables (1) or disable (0) the vertical retrace interrupts that occur at just after each display of the screen (foreground).

M1-2 are bits are used with M3-5 bits of register 0 to define the VDP screen mode. (See here for detail)

SI defines the sprite size. Write 1 to use 16x16 sprites, 0 to usinge 8x8 sprites.

MAG enlarges the sprites when 1 is written. (0 by default)
*/
   int sprite_size=(vdp_9918a_registers[1] & 2 ? 16 : 8);



   return sprite_size; 
}

int vdp_9918a_get_sprite_double(void)
{
    int sprite_double=(vdp_9918a_registers[1] & 1 ? 2 : 1);

    //Bank Panic por ejemplo de SG1000 utiliza X2 en el titulo

    return sprite_double;
}

z80_int vdp_9918a_get_sprite_pattern_table(void)
{
    z80_int sprite_pattern_table=(vdp_9918a_registers[6]) * 0x800;

    return sprite_pattern_table;
}


z80_int vdp_9918a_get_sprite_attribute_table(void)
{

    z80_int sprite_attribute_table=(vdp_9918a_registers[5]) * 0x80;

    //En boundary de 128
    sprite_attribute_table &=(65535-128);

    return sprite_attribute_table;
}

void vdp_9918a_render_ula_no_rainbow(z80_byte *vram)
{


	z80_byte video_mode=vdp_9918a_get_video_mode();


	//printf ("video_mode: %d\n",video_mode);


	int x,y,bit; 
	z80_int direccion_name_table;
	z80_byte byte_leido;
    z80_byte byte_color;
	int color=0;
	
    //int zx,zy;

	z80_byte ink,paper;


    z80_int pattern_color_table;
	z80_int pattern_base_address; 
	z80_int pattern_name_table; 

	pattern_name_table=vdp_9918a_get_pattern_name_table(); 


	pattern_color_table=vdp_9918a_get_pattern_color_table();


    pattern_base_address=vdp_9918a_get_pattern_base_address();



	int chars_in_line;
	int char_width;

	switch(video_mode) {
		
		case 4:
		case 0:
		//"screen 0": Text, characters of 6 x 8	40 x 24 characters
		//video_mode: 4		

	

		//pattern_base_address=0; //TODO: Puesto a pelo		
		//"screen 1": Text, characters of 8 x 8	, 32 x 24 characters
		//video_mode: 0	



		if (video_mode==4) {
			chars_in_line=40;
			char_width=6;

			//En modo texto 40x24, color tinta y papel fijos

			//ink=(vdp_9918a_registers[7]>>4)&15;
			//paper=(vdp_9918a_registers[7])&15;			


			ink=vdp_9918a_get_foreground_color();
			paper=vdp_9918a_get_border_color();          
		}

		else {
			chars_in_line=32;
			char_width=8;
		}


		direccion_name_table=pattern_name_table;  

        for (y=0;y<24;y++) {
			for (x=0;x<chars_in_line;x++) {  
       
            		
				z80_byte caracter=vdp_9918a_read_vram_byte(vram,direccion_name_table);
                
				if (video_mode==0) {
					int posicion_color=caracter/8;

					z80_byte byte_color=vdp_9918a_read_vram_byte(vram,pattern_color_table+posicion_color);

					ink=(byte_color >> 4) & 15;
					paper=(byte_color ) & 15;
				}


				int scanline;

				z80_int pattern_address=pattern_base_address+caracter*8;

				for (scanline=0;scanline<8;scanline++) {

					byte_leido=vdp_9918a_read_vram_byte(vram,pattern_address++);
	                       

                    for (bit=0;bit<char_width;bit++) {

						//int fila=(x*char_width+bit)/8;
						
						
						//Ver en casos en que puede que haya menu activo y hay que hacer overlay
						//if (scr_ver_si_refrescar_por_menu_activo(fila,y)) {
							color= ( byte_leido & 128 ? ink : paper );
							scr_putpixel_zoom(x*char_width+bit,y*8+scanline,VDP_9918_INDEX_FIRST_COLOR+color);
						//}

						byte_leido=byte_leido<<1;
        	        }
				}


				direccion_name_table++;

			}
			

   		 }

		break;

		case 2:
			//Screen 3. multicolor mode. 64x48
			//video_mode: 2


			direccion_name_table=pattern_name_table;  

			for (y=0;y<24;y++) {
				for (x=0;x<32;x++) {  
		
							
					z80_byte caracter=vdp_9918a_read_vram_byte(vram,direccion_name_table++);
					
								
					int incremento_byte=(y&3)*2;


					pattern_base_address &=(65536-1023); //Cae offsets de 1kb

					//printf ("pattern_address: %d\n",pattern_base_address);

					z80_int pattern_address=pattern_base_address+caracter*8+incremento_byte;

					int row;
					for (row=0;row<2;row++) {

						byte_leido=vdp_9918a_read_vram_byte(vram,pattern_address++);
						
						int col;
						for (col=0;col<2;col++) {
											
							//Ver en casos en que puede que haya menu activo y hay que hacer overlay
							//if (scr_ver_si_refrescar_por_menu_activo(x,y)) {

								//Primera columna usa color en parte parte alta y luego baja
								color=(byte_leido>>4)&15;

								byte_leido=byte_leido << 4;

								
								int subpixel_x,subpixel_y;

								int xfinal=x*8+col*4;
								int yfinal=y*8+row*4;							

								for (subpixel_y=0;subpixel_y<4;subpixel_y++) {
									for (subpixel_x=0;subpixel_x<4;subpixel_x++) {
								
										scr_putpixel_zoom(xfinal+subpixel_x,  yfinal+subpixel_y,  VDP_9918_INDEX_FIRST_COLOR+color);
									}

								}
								
							//}
						
						}

					}

				}

			}

		break;	        


		//Screen 2. high-res mode, 256x192
		//video_mode: 1
		case 1:
        default:

			chars_in_line=32;
			char_width=8;

            //printf ("pattern base address before mask: %d\n",pattern_base_address);

            //printf ("pattern color table before mask:  %d\n",pattern_color_table);            


			pattern_base_address &=8192; //Cae en offset 0 o 8192
          
			pattern_color_table &=8192; //Cae en offset 0 o 8192


            //printf ("pattern base address after mask: %d\n",pattern_base_address);

            //printf ("pattern color table after mask:  %d\n",pattern_color_table);

			direccion_name_table=pattern_name_table;  

			for (y=0;y<24;y++) {

				int tercio=y/8;

				for (x=0;x<chars_in_line;x++) {  
					
					
					z80_byte caracter=vdp_9918a_read_vram_byte(vram,direccion_name_table);
					

					int scanline;

					z80_int pattern_address=(caracter*8+2048*tercio) ;
					pattern_address +=pattern_base_address;
					
					


					z80_int color_address=(caracter*8+2048*tercio) ;
					color_address +=pattern_color_table;

	
			

					for (scanline=0;scanline<8;scanline++) {

						byte_leido=vdp_9918a_read_vram_byte(vram,pattern_address++);

						byte_color=vdp_9918a_read_vram_byte(vram,color_address++);


						ink=(byte_color>>4) &15;
						paper=byte_color &15;

							
						for (bit=0;bit<char_width;bit++) {

							//int fila=(x*char_width+bit)/8;

													
							//Ver en casos en que puede que haya menu activo y hay que hacer overlay
							//if (scr_ver_si_refrescar_por_menu_activo(fila,y)) {
								color= ( byte_leido & 128 ? ink : paper );
								scr_putpixel_zoom(x*char_width+bit,y*8+scanline,VDP_9918_INDEX_FIRST_COLOR+color);
							//}

							byte_leido=byte_leido<<1;
						}
					}

						
					direccion_name_table++;

				}
		   }

		break;






	}    
}



void vdp_9918a_render_sprites_no_rainbow(z80_byte *vram)
{
 
    if (vdp_9918a_si_sms_video_mode4()) {
        //printf("Render sprites modo 4 sms\n");
        vdp_9918a_render_sprites_sms_video_mode4_no_rainbow(vram);
        return;
    }

    z80_byte video_mode=vdp_9918a_get_video_mode();




    //En modo 40x24 no permitimos sprites
    if (video_mode==4) return;



    z80_int sprite_pattern_table=vdp_9918a_get_sprite_pattern_table();
    
    z80_byte byte_leido;

        
        int sprite_size=vdp_9918a_get_sprite_size();
        int sprite_double=vdp_9918a_get_sprite_double();

        //printf ("Sprite size: %d double: %d\n",sprite_size,sprite_double);



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

        int primer_sprite_final=VDP_9918A_MAX_SPRITES-1;

        //int offset_sprite=sprite_attribute_table;

        //int i;
        for (primer_sprite_final=0;primer_sprite_final<32 && !salir;primer_sprite_final++) {
            int offset_sprite=sprite_attribute_table+primer_sprite_final*4;

            z80_byte vert_pos=vdp_9918a_read_vram_byte(vram,offset_sprite);
            if (vert_pos==208) salir=1;

        }

        //Siempre estara al siguiente
        primer_sprite_final--;

        sprite_attribute_table +=(primer_sprite_final*4);

        //Empezar desde final hacia principio

        for (sprite=primer_sprite_final;sprite>=0;sprite--) {
            int vert_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table);
            int horiz_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+1);
            z80_byte sprite_name=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+2);
            z80_byte attr_color_etc=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+3);

/*
  0: Y-pos, Vertical position (FFh is topmost, 00h is second line, etc.)
  1: X-pos, Horizontal position (00h is leftmost)
  2: Pattern number
  3: Attributes. b0-3:Color, b4-6:unused, b7:EC (Early Clock)            
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
            sprite_attribute_table -=4;

            //Si early clock, x-=32

            if (attr_color_etc & 128) {
                //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);                
                horiz_pos -=32;
            }

            //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);

       
                

                //Si coord Y no esta en el borde inferior
                if (vert_pos<192) {
                    //int offset_pattern_table=sprite_name*bytes_per_sprite+sprite_pattern_table;
                      int offset_pattern_table=sprite_name*8+sprite_pattern_table;
                    z80_byte color=attr_color_etc & 15;

                    int x,y;

                    //Sprites de 16x16
                    if (sprite_size==16) {
                        int quad_x,quad_y;

                        for (quad_x=0;quad_x<2;quad_x++) {
                            for (quad_y=0;quad_y<2;quad_y++) {
                                for (y=0;y<8;y++) {
                                
                                    byte_leido=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                                    for (x=0;x<8;x++) {

                                        int pos_x_final;
                                        int pos_y_final;

                                        pos_x_final=horiz_pos+((quad_x*8)+x)*sprite_double;
                                        pos_y_final=vert_pos+((quad_y*8)+y)*sprite_double;
                                        
                                        //Si dentro de limites
                                        if (pos_x_final>=0 && pos_x_final<=255 && pos_y_final>=0 && pos_y_final<=191) {

                                            //Si bit a 1
                                            if (byte_leido & 128) {
                                                //Y si ese color no es transparente 
                                                if (color!=0) {
                                                    //printf ("putpixel sprite x %d y %d\n",pos_x_final,pos_y_final);

                                                    z80_byte color_sprite=color;

                                                    if (vdp_9918a_reveal_layer_sprites.v) {
                                                        int posx=pos_x_final&1;
                                                        int posy=pos_y_final&1;

                                                        //0,0: 0
                                                        //0,1: 1
                                                        //1,0: 1
                                                        //1,0: 0
                                                        //Es un xor

                                                        int si_blanco_negro=posx ^ posy;
                                                        //printf ("si_blanco_negro: %d\n",si_blanco_negro);
                                                        color_sprite=si_blanco_negro*15;
                                                        //printf ("color: %d\n",color);
                                                    }


                                                    scr_putpixel_zoom(pos_x_final,  pos_y_final,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                    if (sprite_double==2) {
                                                        scr_putpixel_zoom(pos_x_final+1,  pos_y_final,    VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                        scr_putpixel_zoom(pos_x_final,    pos_y_final+1,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                        scr_putpixel_zoom(pos_x_final+1,  pos_y_final+1,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                    }
                                                }
                                            }

                                            byte_leido = byte_leido << 1;
                                        }
                                    }
                                }
                            }
                        }                        
                    }

                    //Sprites de 8x8
                    else {

                        for (y=0;y<8;y++) {

                                byte_leido=vdp_9918a_read_vram_byte(vram,offset_pattern_table++);
                                for (x=0;x<8;x++) {

                                    int pos_x_final;
                                    int pos_y_final;

                                    pos_x_final=horiz_pos+x*sprite_double;
                                    pos_y_final=vert_pos+y*sprite_double;
                                    
                                    if (pos_x_final>=0 && pos_x_final<=255 && pos_y_final>=0 && pos_y_final<=191) {

                                        //Si bit a 1
                                        if (byte_leido & 128) {
                                            //Y si ese color no es transparente
                                            if (color!=0) {
                                                //printf ("putpixel sprite x %d y %d\n",pos_x_final,pos_y_final);

                                                z80_byte color_sprite=color;

                                                if (vdp_9918a_reveal_layer_sprites.v) {
                                                    int posx=pos_x_final&1;
                                                    int posy=pos_y_final&1;

                                                    //0,0: 0
                                                    //0,1: 1
                                                    //1,0: 1
                                                    //1,0: 0
                                                    //Es un xor

                                                    int si_blanco_negro=posx ^ posy;
                                                    color_sprite=si_blanco_negro*15;
                                                }                                            
                                                scr_putpixel_zoom(pos_x_final,  pos_y_final,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                if (sprite_double==2) {
                                                    scr_putpixel_zoom(pos_x_final+1,  pos_y_final,    VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                    scr_putpixel_zoom(pos_x_final,    pos_y_final+1,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                    scr_putpixel_zoom(pos_x_final+1,  pos_y_final+1,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                }                                                
                                            }
                                        }
                                    }

                                    byte_leido = byte_leido << 1;
                                }
                            
                        }
                    }

                }
            

        }   
}

int vdp_9918a_get_final_border_if_disable(void)
{

    unsigned int color_final_border;

    if (vdp_9918a_si_sms_video_mode4()) {
        color_final_border=vdp_9918a_sms_get_final_color_border();

        if (vdp_9918a_force_disable_layer_border.v) color_final_border=SMS_INDEX_FIRST_COLOR; //color 0 de su paleta de colores
    }    
    
    else {
        color_final_border=vdp_9918a_get_border_color()+VDP_9918_INDEX_FIRST_COLOR;

        if (vdp_9918a_force_disable_layer_border.v) color_final_border=VDP_9918_INDEX_FIRST_COLOR; //color 0 de su paleta de colores
    }

    return color_final_border;
}

void vdp_9918a_refresca_border(void)
{

    unsigned int color_final_border;

    color_final_border=vdp_9918a_get_final_border_if_disable();





        int x,y;




	//Top border cambia en spectrum y zx8081 y ace
	int topborder=VDP_9918A_TOP_BORDER;
	

	//color +=spectrum_palette_offset;


        //parte superior
        for (y=0;y<topborder;y++) {
                for (x=0;x<VDP_9918A_ANCHO_PANTALLA*zoom_x+VDP_9918A_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,y,color_final_border);
                }
        }

        //parte inferior
        for (y=0;y<VDP_9918A_BOTTOM_BORDER;y++) {
                for (x=0;x<VDP_9918A_ANCHO_PANTALLA*zoom_x+VDP_9918A_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,topborder+y+VDP_9918A_ALTO_PANTALLA*zoom_y,color_final_border);


                }
        }






        for (y=0;y<VDP_9918A_ALTO_PANTALLA*zoom_y;y++) {
                for (x=0;x<VDP_9918A_LEFT_BORDER;x++) {
                        scr_putpixel(x,topborder+y,color_final_border);
                }

        

        }

        int ancho_pantalla=VDP_9918A_ANCHO_PANTALLA;
        int ancho_border_derecho=VDP_9918A_LEFT_BORDER;

        //laterales. En modo 0, 40x24, border derecho es 16 pixeles mas ancho
        z80_byte video_mode=vdp_9918a_get_video_mode();        

        if (video_mode==4) {
            ancho_pantalla -=16;
            ancho_border_derecho +=16*zoom_x;
        }

        for (y=0;y<VDP_9918A_ALTO_PANTALLA*zoom_y;y++) {

                for (x=0;x<ancho_border_derecho;x++) {
                        scr_putpixel(VDP_9918A_LEFT_BORDER+ancho_pantalla*zoom_x+x,topborder+y,color_final_border);
                }                

        }




}



//Renderiza una linea de display (pantalla y sprites, pero no border)
void vdp_9918a_render_rainbow_display_line(int scanline,z80_int *scanline_buffer,z80_byte *vram)
{


    //Nos ubicamos ya en la zona de pixeles, saltando el border
    //En esta capa, si color=0, no lo ponemos como transparente sino como color negro
    z80_int *destino_scanline_buffer;
    destino_scanline_buffer=&scanline_buffer[screen_total_borde_izquierdo];


	z80_byte video_mode=vdp_9918a_get_video_mode();

	//printf ("video_mode: %d\n",video_mode);


	int x,bit; 
	z80_int direccion_name_table;
	z80_byte byte_leido;
    z80_byte byte_color;
	int color=0;
	
	//int zx,zy;

	z80_byte ink,paper;


	z80_int pattern_base_address; //=2048; //TODO: Puesto a pelo
	z80_int pattern_name_table; //=0; //TODO: puesto a pelo

	pattern_name_table=vdp_9918a_get_pattern_name_table(); //(vdp_9918a_registers[2]&15) * 0x400; 



	pattern_base_address=vdp_9918a_get_pattern_base_address();


	z80_int pattern_color_table=vdp_9918a_get_pattern_color_table();


    //Sumar el offset por linea

    int fila=scanline/8;

    //entre 0 y 7 dentro de la fila
    int scanline_fila=scanline % 8;    

    int offset_sumar_linea;


	int chars_in_line;
	int char_width;

	switch(video_mode) {

		case 4:
		case 0:
		//"screen 0": Text, characters of 6 x 8	40 x 24 characters
		//video_mode: 4		

	

		//pattern_base_address=0; //TODO: Puesto a pelo		
		//"screen 1": Text, characters of 8 x 8	, 32 x 24 characters
		//video_mode: 0	


		if (video_mode==4) {
			chars_in_line=40;
			char_width=6;

			//En modo texto 40x24, color tinta y papel fijos

			ink=(vdp_9918a_registers[7]>>4)&15;
			paper=(vdp_9918a_registers[7])&15;			
		}

		else {
			chars_in_line=32;
			char_width=8;
		}






		direccion_name_table=pattern_name_table;  



        offset_sumar_linea=chars_in_line*fila;

        //printf ("offset: %d\n",offset_sumar_linea);

        direccion_name_table +=offset_sumar_linea;

       
			for (x=0;x<chars_in_line;x++) {  
       
            		
				z80_byte caracter=vdp_9918a_read_vram_byte(vram,direccion_name_table);
                //printf ("%d ",caracter);
                
				if (video_mode==0) {
					int posicion_color=caracter/8;

					z80_byte byte_color=vdp_9918a_read_vram_byte(vram,pattern_color_table+posicion_color);

					ink=(byte_color >> 4) & 15;
					paper=(byte_color ) & 15;
				}




				z80_int pattern_address=pattern_base_address+caracter*8+scanline_fila;



					byte_leido=vdp_9918a_read_vram_byte(vram,pattern_address);
	                       

                    for (bit=0;bit<char_width;bit++) {

						//int columna=(x*char_width+bit)/8;
						
						
						//Ver en casos en que puede que haya menu activo y hay que hacer overlay
						//if (scr_ver_si_refrescar_por_menu_activo(columna,fila)) {
							color= ( byte_leido & 128 ? ink : paper );
							//scr_putpixel_zoom(x*char_width+bit,y*8+scanline,VDP_9918_INDEX_FIRST_COLOR+color);
                            *destino_scanline_buffer=VDP_9918_INDEX_FIRST_COLOR+color;
                            destino_scanline_buffer++;
						//}

						byte_leido=byte_leido<<1;
        	        }

                direccion_name_table++;

			}
			

		break;



		case 2:
			//Screen 3. multicolor mode. 64x48
			//video_mode: 2

			chars_in_line=32;
			char_width=8;

			direccion_name_table=pattern_name_table;  

        offset_sumar_linea=chars_in_line*fila;

        //printf ("offset: %d\n",offset_sumar_linea);

        direccion_name_table +=offset_sumar_linea;            

			//for (y=0;y<24;y++) {
				for (x=0;x<32;x++) {  
		
							
					z80_byte caracter=vdp_9918a_read_vram_byte(vram,direccion_name_table++);
					
								
					int incremento_byte=(fila&3)*2;


					pattern_base_address &=(65536-1023); //Cae offsets de 1kb

					//printf ("pattern_address: %d\n",pattern_base_address);

					z80_int pattern_address=pattern_base_address+caracter*8+incremento_byte+scanline_fila/4;

					//int row;
					//for (row=0;row<2;row++) {

						byte_leido=vdp_9918a_read_vram_byte(vram,pattern_address++);
						
						int col;
						for (col=0;col<2;col++) {
											
							//Ver en casos en que puede que haya menu activo y hay que hacer overlay
							//if (scr_ver_si_refrescar_por_menu_activo(x,fila)) {

								//Primera columna usa color en parte parte alta y luego baja
								color=(byte_leido>>4)&15;

								byte_leido=byte_leido << 4;

								
								int subpixel_x;
                                //int subpixel_y;

								int xfinal=x*8+col*4;
								//int yfinal=y*8+row*4;							

								//for (subpixel_y=0;subpixel_y<4;subpixel_y++) {
									for (subpixel_x=0;subpixel_x<4;subpixel_x++) {
								
										//scr_putpixel_zoom(xfinal+subpixel_x,  yfinal+subpixel_y,  VDP_9918_INDEX_FIRST_COLOR+color);

                                        destino_scanline_buffer[xfinal+subpixel_x]=VDP_9918_INDEX_FIRST_COLOR+color;

                                        //destino_scanline_buffer++;
                            
									}

								//}
								
							//}
						
						}

					//}

				}

			//}

		break;	        


		//Screen 2. high-res mode, 256x192
		//video_mode: 1
		case 1:
        default:


			chars_in_line=32;
			char_width=8;

            //printf ("pattern base address before mask: %d\n",pattern_base_address);

            //printf ("pattern color table before mask:  %d\n",pattern_color_table);            


			pattern_base_address &=8192; //Cae en offset 0 o 8192
          
			pattern_color_table &=8192; //Cae en offset 0 o 8192


            //printf ("pattern base address after mask: %d\n",pattern_base_address);

            //printf ("pattern color table after mask:  %d\n",pattern_color_table);

			direccion_name_table=pattern_name_table;  

            offset_sumar_linea=chars_in_line*fila;

            direccion_name_table +=offset_sumar_linea;            

			//for (y=0;y<24;y++) {

				int tercio=fila/8;

				for (x=0;x<chars_in_line;x++) {  
					
					
					z80_byte caracter=vdp_9918a_read_vram_byte(vram,direccion_name_table);
					

					//int scanline;

					z80_int pattern_address=(caracter*8+2048*tercio) ;
					pattern_address +=pattern_base_address+scanline_fila;
					
					


					z80_int color_address=(caracter*8+2048*tercio) ;
					color_address +=pattern_color_table+scanline_fila;

	
			

					//for (scanline=0;scanline<8;scanline++) {

						byte_leido=vdp_9918a_read_vram_byte(vram,pattern_address);

						byte_color=vdp_9918a_read_vram_byte(vram,color_address);


						ink=(byte_color>>4) &15;
						paper=byte_color &15;

							
						for (bit=0;bit<char_width;bit++) {

							//int columna=(x*char_width+bit)/8;

													
							//Ver en casos en que puede que haya menu activo y hay que hacer overlay
							//if (scr_ver_si_refrescar_por_menu_activo(columna,fila)) {
								color= ( byte_leido & 128 ? ink : paper );
								//scr_putpixel_zoom(x*char_width+bit,y*8+scanline,VDP_9918_INDEX_FIRST_COLOR+color);
                                *destino_scanline_buffer=VDP_9918_INDEX_FIRST_COLOR+color;
                                destino_scanline_buffer++;                                
							//}

							byte_leido=byte_leido<<1;
						}
					//}

						
					direccion_name_table++;

				}
		   //}

		break;


	




	}    

}


//Color 0 es transparente
z80_int vdp_9918a_buffer_render_sprites[256];


//Pone sprite en buffer y activa bit de colision si hace falta
void vdp9918a_put_sprite_pixel(z80_int *destino,z80_int color)
{

    if ( (*destino) !=0 ) {
        //printf ("Colision. Color anterior: %d VDP_9918_INDEX_FIRST_COLOR: %d\n",*destino,VDP_9918_INDEX_FIRST_COLOR);

    //7 6  5 43210
    //F 5S C Fifth sprite number
    //F: interrupt pending flag
    //5S: fifth sprite flag
    //C: sprite colision (coincidence) flag
    //fifth sprite number

    
        vdp_9918a_status_register |=32;

    }

    *destino=color;
}


//Renderiza una linea de sprites 
void vdp_9918a_render_rainbow_sprites_line_post(int scanline,z80_int *destino_scanline_buffer,z80_byte *vram)
{



	//z80_int pattern_base_address; //=2048; //TODO: Puesto a pelo
	//z80_int pattern_name_table; //=0; //TODO: puesto a pelo

	//pattern_name_table=vdp_9918a_get_pattern_name_table(); //(vdp_9918a_registers[2]&15) * 0x400; 



	//pattern_base_address=vdp_9918a_get_pattern_base_address();





    //Sumar el offset por linea

    //int fila=scanline/8;

    //entre 0 y 7 dentro de la fila
    //int scanline_fila=scanline % 8;    



    z80_int sprite_pattern_table=vdp_9918a_get_sprite_pattern_table();
    
    z80_byte byte_leido;

    int sprites_en_linea=0;


    //Asumimos que se resetea el bit 5S del status register
    //Bit  6    5S         1 if more than 4 sprites on a horizontal line
    vdp_9918a_status_register&= (255-64);

        
        int sprite_size=vdp_9918a_get_sprite_size();
        int sprite_double=vdp_9918a_get_sprite_double();


        //printf ("Sprite size: %d double: %d\n",sprite_size,sprite_double);

        /*
        int bytes_per_sprite;
        int bytes_per_line;

        if (sprite_size==8) {
            bytes_per_sprite=8;
            bytes_per_line=1;
        }

        else {
            bytes_per_sprite=32;
            bytes_per_line=2;
        }
        */

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

        int primer_sprite_final=VDP_9918A_MAX_SPRITES-1;

        //int offset_sprite=sprite_attribute_table;

        //int i;
        for (primer_sprite_final=0;primer_sprite_final<32 && !salir;primer_sprite_final++) {
            int offset_sprite=sprite_attribute_table+primer_sprite_final*4;

            z80_byte vert_pos=vdp_9918a_read_vram_byte(vram,offset_sprite);
            if (vert_pos==208) salir=1;

        }

        //Siempre estara al siguiente
        primer_sprite_final--;

        sprite_attribute_table +=(primer_sprite_final*4);

        //Empezar desde final hacia principio
        //printf ("Sprite final: %d\n",primer_sprite_final);

        int maximo_sprites_por_linea=VDP_9918A_MAX_SPRITES_PER_LINE;

        //Si hay setting de no limite sprites por linea
        if (vdp_9918a_unlimited_sprites_line.v) {
            maximo_sprites_por_linea=9999;
        }

        for (sprite=primer_sprite_final;sprite>=0 && sprites_en_linea<maximo_sprites_por_linea;sprite--) {
            int vert_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table);
            int horiz_pos=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+1);
            z80_byte sprite_name=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+2);
            z80_byte attr_color_etc=vdp_9918a_read_vram_byte(vram,sprite_attribute_table+3);

/*
  0: Y-pos, Vertical position (FFh is topmost, 00h is second line, etc.)
  1: X-pos, Horizontal position (00h is leftmost)
  2: Pattern number
  3: Attributes. b0-3:Color, b4-6:unused, b7:EC (Early Clock)            
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
            sprite_attribute_table -=4;

            //Si early clock, x-=32

            if (attr_color_etc & 128) {
                //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);                
                horiz_pos -=32;
            }

            //printf ("sprite number: %d X: %d Y: %d Name: %d color_etc: %d\n",sprite,horiz_pos,vert_pos,sprite_name,attr_color_etc);

       
                

                //Si posicion Y sprite esta en el margen
                if (scanline>=vert_pos && scanline<vert_pos+sprite_size*sprite_double) {
                //if (vert_pos<192) {
                    //int offset_pattern_table=sprite_name*bytes_per_sprite+sprite_pattern_table;

                    

                      int offset_pattern_table=sprite_name*8+sprite_pattern_table;
                    z80_byte color=attr_color_etc & 15;

                    int x;

                    int dibujado_sprite=0;

                    //Sprites de 16x16
                    if (sprite_size==16) {
                        int quad_x;
                        //int quad_y;

                        //linea 0..15 dentro del sprite
                        int fila_sprites_16=(scanline-vert_pos)/sprite_double;
                        //printf ("fila: %d\n",fila_sprites_16);

                        //Cuadrante 0...1
                        int quadrante_y=fila_sprites_16/8;

                        //Sumar 8 bytes si cuadrante 1
                        offset_pattern_table +=quadrante_y*8;

                        //Sumar 0..7 segun linea
                        offset_pattern_table +=fila_sprites_16 % 8;

                        //Nota: seguro que estas sumas se pueden simplificar, pero asi queda mas claro

                        for (quad_x=0;quad_x<2;quad_x++) {
                            //for (quad_y=0;quad_y<2;quad_y++) {
                                //for (y=0;y<8;y++) {
                                
                                    byte_leido=vdp_9918a_read_vram_byte(vram,offset_pattern_table);
                                    for (x=0;x<8;x++) {

                                        int pos_x_final;
                                        //int pos_y_final;

                                        pos_x_final=horiz_pos+((quad_x*8)+x)*sprite_double;
                                        //pos_y_final=vert_pos+(quad_y*8)+y;
                                        
                                        //Si dentro de limites
                                        if (pos_x_final>=0 && pos_x_final<=255 /*&& pos_y_final>=0 && pos_y_final<=191*/) {



                                            //Si bit a 1
                                            if (byte_leido & 128) {
                                                //Y si ese color no es transparente 
                                                if (color!=0) {
                                                    //printf ("putpixel sprite x %d y %d\n",pos_x_final,pos_y_final);

                                                    //Al menos hay un pixel dentro de pantalla, se incrementara contador de sprites por linea
                                                    //Solo lo hago si el pixel no es transparente, esto esta bien??
                                                    dibujado_sprite=1;

                                                    z80_byte color_sprite=color;

                                                    if (vdp_9918a_reveal_layer_sprites.v) {
                                                        int posx=pos_x_final&1;
                                                        int posy=scanline&1;

                                                        //0,0: 0
                                                        //0,1: 1
                                                        //1,0: 1
                                                        //1,0: 0
                                                        //Es un xor

                                                        int si_blanco_negro=posx ^ posy;
                                                        //printf ("si_blanco_negro: %d\n",si_blanco_negro);
                                                        color_sprite=si_blanco_negro*15;
                                                        //printf ("color: %d\n",color);
                                                    }


                                                    //scr_putpixel_zoom(pos_x_final,  pos_y_final,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                    //destino_scanline_buffer[pos_x_final]=VDP_9918_INDEX_FIRST_COLOR+color_sprite;


                                                    vdp9918a_put_sprite_pixel(&destino_scanline_buffer[pos_x_final],VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                    if (sprite_double==2) {
                                                        vdp9918a_put_sprite_pixel(&destino_scanline_buffer[pos_x_final+1],VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                    }
                                
                                                }
                                            }

                                            byte_leido = byte_leido << 1;
                                        }
                                    }
                                //}
                            //}

                            offset_pattern_table+=16; //byte del cuadrante derecho
                        }                        
                    }

                    //Sprites de 8x8
                    else {

                        //printf("Sprites de 8x8\n");

                        //for (y=0;y<8;y++) {
                        //linea 0..7 dentro del sprite
                        int fila_sprites=(scanline-vert_pos)/sprite_double;

                            offset_pattern_table +=fila_sprites;

                                byte_leido=vdp_9918a_read_vram_byte(vram,offset_pattern_table);
                                for (x=0;x<8;x++) {

                                    int pos_x_final;
                                    //int pos_y_final;

                                    pos_x_final=horiz_pos+x*sprite_double;
                                    //pos_y_final=vert_pos+y;
                                    
                                    if (pos_x_final>=0 && pos_x_final<=255 /*&& pos_y_final>=0 && pos_y_final<=191*/) {



                                        //Si bit a 1
                                        if (byte_leido & 128) {
                                            //Y si ese color no es transparente
                                            if (color!=0) {
                                                //printf ("putpixel sprite x %d y %d\n",pos_x_final,pos_y_final);

                                                //Al menos hay un pixel dentro de pantalla, se incrementara contador de sprites por linea
                                                //Solo lo hago si el pixel no es transparente, esto esta bien??
                                                dibujado_sprite=1;

                                                z80_byte color_sprite=color;

                                                if (vdp_9918a_reveal_layer_sprites.v) {
                                                    int posx=pos_x_final&1;
                                                    int posy=scanline&1;

                                                    //0,0: 0
                                                    //0,1: 1
                                                    //1,0: 1
                                                    //1,0: 0
                                                    //Es un xor

                                                    int si_blanco_negro=posx ^ posy;
                                                    color_sprite=si_blanco_negro*15;
                                                }                                            
                                                //scr_putpixel_zoom(pos_x_final,  pos_y_final,  VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                //destino_scanline_buffer[pos_x_final]=VDP_9918_INDEX_FIRST_COLOR+color_sprite;

                                                vdp9918a_put_sprite_pixel(&destino_scanline_buffer[pos_x_final],VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                if (sprite_double==2) {
                                                    vdp9918a_put_sprite_pixel(&destino_scanline_buffer[pos_x_final+1],VDP_9918_INDEX_FIRST_COLOR+color_sprite);
                                                }
                                            }
                                        }
                                    }

                                    byte_leido = byte_leido << 1;
                                }
                            
                        //}
                    }


                    if (dibujado_sprite) sprites_en_linea++; 

                }
            

        }   


        //Si llega al maximo de sprites
        if (sprites_en_linea>=maximo_sprites_por_linea) {
            //Indicamos justo el anterior en el status register
            //Bit 0-4  SP4-0      Number for the 5th sprite (9th in screen 4-8) on a line (b0=SP4, b4=SP0)
            vdp_9918a_status_register&= 128+64+32;


            vdp_9918a_status_register |=(sprite-1);

            //Y el flag 5S
            vdp_9918a_status_register |=64;
        }



}


//Renderiza una linea de sprites, primero en buffer temporal así para poder gestionar colisiones
void vdp_9918a_render_rainbow_sprites_line(int scanline,z80_int *scanline_buffer,z80_byte *vram)
{

    z80_byte video_mode=vdp_9918a_get_video_mode();



    //En modo 40x24 no permitimos sprites
    if (video_mode==4) return;



    int i;

    //Inicializar buffer temporal a color 0 transparente
    for (i=0;i<256;i++) {
        vdp_9918a_buffer_render_sprites[i]=0;
    }


    vdp_9918a_render_rainbow_sprites_line_post(scanline,vdp_9918a_buffer_render_sprites,vram);

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





void screen_store_scanline_rainbow_solo_border_vdp_9918a_section(z80_int *buffer,int lenght)
{
    int i;

    z80_int color_final;

    color_final=vdp_9918a_get_final_border_if_disable();


    for (i=0;i<lenght;i++) {
        *buffer=color_final;
        buffer++;
    }
}


//Nota: no se va a tener en cuenta dibujado completamente real, es decir, que el electron empieza donde la zona de pantalla,
//a la derecha del borde izquierdo,
//sino que cada scanline empieza a la izquierda del borde izquierdo
void screen_store_scanline_rainbow_solo_border_vdp_9918a(z80_int *scanline_buffer)
{

    

	int ancho_pantalla=256;

    //zona de border superior o inferior. Dibujar desde posicion x donde acaba el ancho izquierdo de borde, linea horizontal
	//hasta derecha del todo, y luego trozo de ancho izquiero del borde de linea siguiente
    if ( (t_scanline_draw>=screen_invisible_borde_superior && t_scanline_draw<screen_indice_inicio_pant) ||
             (t_scanline_draw>=screen_indice_fin_pant && t_scanline_draw<screen_indice_fin_pant+screen_total_borde_inferior)
	   ) {
 

        screen_store_scanline_rainbow_solo_border_vdp_9918a_section(scanline_buffer,
            screen_total_borde_izquierdo+ancho_pantalla+screen_total_borde_derecho);
		
    }

        //zona de border + pantalla + border

    else if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {


        //Borde izquierdo
        screen_store_scanline_rainbow_solo_border_vdp_9918a_section(scanline_buffer,screen_total_borde_izquierdo);

        //Borde detecho
        int ancho_border_derecho=screen_total_borde_derecho;

        //laterales. En modo 0, 40x24, border derecho es 16 pixeles mas ancho
        z80_byte video_mode=vdp_9918a_get_video_mode();        

        if (video_mode==4) {
            ancho_pantalla -=16;
            ancho_border_derecho +=16*zoom_x;
        }


        screen_store_scanline_rainbow_solo_border_vdp_9918a_section(&scanline_buffer[screen_total_borde_izquierdo+ancho_pantalla],
            ancho_border_derecho);

    }





}


//Guardar en buffer rainbow la linea actual. Para MSX. solo display
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
void screen_store_scanline_rainbow_solo_display_vdp_9918a(z80_int *scanline_buffer,z80_byte *vram_memory_pointer)
{


  //Si en zona pantalla (no border superior ni inferior)
  if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {


        //linea en coordenada display (no border) que se debe leer
        int y_display=t_scanline_draw-screen_indice_inicio_pant;

        //Si modo 4 sms
        if (vdp_9918a_si_sms_video_mode4() ) {
            screen_store_scanline_rainbow_solo_display_vdp_9918a_sms_3layer(scanline_buffer,vram_memory_pointer,y_display);
            return;
        }

 
        //Render pixeles
        if (vdp_9918a_force_disable_layer_ula.v==0 && vdp_9918a_reveal_layer_ula.v==0) {
            vdp_9918a_render_rainbow_display_line(y_display,scanline_buffer,vram_memory_pointer);
        }

        else {
            //Capa desactivada o reveal
            //Nos ubicamos en zona central
            int inicio_buffer=screen_total_borde_izquierdo;

            int i;



            for (i=0;i<256;i++) {

                z80_int color=0;

                if (vdp_9918a_reveal_layer_ula.v) {
                    int posx=i&1;
                    int posy=t_scanline_draw&1;

                    int si_blanco_negro=posx ^ posy;

                    //color 0 o 15
                    color=si_blanco_negro*15;                    
                }

                scanline_buffer[inicio_buffer+i]=VDP_9918_INDEX_FIRST_COLOR+color;
            }

        }





        //Render sprites
        if (vdp_9918a_force_disable_layer_sprites.v==0) {
            vdp_9918a_render_rainbow_sprites_line(y_display,scanline_buffer,vram_memory_pointer);
        }


        

  }    

}



void screen_store_scanline_rainbow_vdp_9918a_border_and_display(z80_int *scanline_buffer,z80_byte *vram_memory) 
{

    //Renderizar zonas de border y display
    screen_store_scanline_rainbow_solo_border_vdp_9918a(scanline_buffer);
    screen_store_scanline_rainbow_solo_display_vdp_9918a(scanline_buffer,vram_memory);


    //Y transferir a rainbow buffer

    z80_int *puntero_buf_rainbow;

    int y_destino_rainbow;

    y_destino_rainbow=t_scanline_draw-screen_invisible_borde_superior;
    if (border_enabled.v==0) y_destino_rainbow=y_destino_rainbow-screen_borde_superior;

    //Limite inferior y superior. Sobretodo el inferior, pues puede ser negativo (en zona border invisible)
    //En teoria superior no deberia ser mayor, pero por si acaso
    int max_y=get_total_alto_rainbow();

    if (y_destino_rainbow<0 || y_destino_rainbow>=max_y) return;

    puntero_buf_rainbow=&rainbow_buffer[ y_destino_rainbow*get_total_ancho_rainbow() ];


    int limite=get_total_ancho_rainbow();

    z80_int *origen_scanline_buffer;
    origen_scanline_buffer=scanline_buffer;

    if (border_enabled.v==0) origen_scanline_buffer +=screen_total_borde_izquierdo;

    int i;


    int blank_color=0;
    int blanking=0;

    //Bit de blanking
    //Sonic suele habilitar este bit de blank en los primeros scanline en el juego
    //supongo que para ocultar cambios en los elementos del juego: sprites, tiles, paletas
    if ((vdp_9918a_registers[1] & 64)==0) {
        //printf ("BLANK: %d\n",vdp_9918a_registers[1] & 2);
        //En este caso mostrar solamente color del border en toda la pantalla

        blank_color=vdp_9918a_get_final_border_if_disable();

        blanking=1;
    }

    for (i=0;i<limite;i++) {
        if (blanking) {
            *puntero_buf_rainbow=blank_color;
        }
        else {
        *puntero_buf_rainbow=*origen_scanline_buffer;
        }

        origen_scanline_buffer++;
        puntero_buf_rainbow++;
    }

}



//Refresco pantalla con rainbow
//Transfiere el contenido del rainbow buffer a pantalla
//Comun a todas las maquinas que usan este chip VDP
void vdp_9918a_scr_refresca_pantalla_y_border_rainbow(void)
{

	int ancho,alto;

	ancho=get_total_ancho_rainbow();
	alto=get_total_alto_rainbow();

	int x,y;


	z80_int color_pixel;
	z80_int *puntero;

	puntero=rainbow_buffer;


	for (y=0;y<alto;y++) {
		
		for (x=0;x<ancho;x++) {

            color_pixel=*puntero++;
            scr_putpixel_zoom_rainbow(x,y,color_pixel);

		}
		
	}

}