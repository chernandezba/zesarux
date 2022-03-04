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
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>



#include "cpu.h"
#include "spritechip.h"
#include "debug.h"
#include "utils.h"
#include "screen.h"
#include "operaciones.h"
#include "ula.h"
#include "ulaplus.h"
#include "mem128.h"

z80_byte spritechip_last_command_sent;

//Registros internos del chip de sprites
//Primer registro a 0
z80_byte spritechip_registers[256]={0};

z80_bit spritechip_enabled={0};

void spritechip_enable(void)
{
	if (!MACHINE_IS_SPECTRUM) {
		debug_printf(VERBOSE_INFO,"Can not enable ZGX Sprintechip on non Spectrum machine");
		return;
	}

	debug_printf (VERBOSE_INFO,"Enabling Sprite Chip");
	spritechip_enabled.v=1;

	enable_rainbow();
}

void spritechip_disable(void)
{
	if (spritechip_enabled.v) {
		debug_printf (VERBOSE_INFO,"Disabling Sprite Chip");
		spritechip_enabled.v=0;
	}
}


void spritechip_write(z80_int port, z80_byte value)
{
	//printf ("Escribiendo puerto 0x%X valor %d\n",port,value);
	if (port==SPRITECHIP_COMMAND_PORT) {
		spritechip_last_command_sent=value;
	}

	if (port==SPRITECHIP_DATA_PORT) {
		spritechip_registers[spritechip_last_command_sent]=value;

		//Gestionar comandos segun last_command_sent y value

			//Guardar coordenada y original usada en scroll vertical abajo
/*
REG[4]: Posicion y de base de scroll.
REG[10]: Ultimo valor enviado a REG[4]. Se mantiene fijo durante el scroll
*/

		if (spritechip_last_command_sent==4) {
			spritechip_registers[10]=value;
		}

	}
}

z80_byte spritechip_read(z80_int port)
{
	//printf ("Leyendo puerto 0x%X\n",port);
	if (port==SPRITECHIP_COMMAND_PORT) return spritechip_last_command_sent;
	if (port==SPRITECHIP_DATA_PORT) return spritechip_registers[spritechip_last_command_sent];
	return 0;
}


z80_int spritechip_da_color_final(z80_int colorleido,z80_byte tipocolor,z80_byte paletacolor)
{
	z80_int color_final=colorleido;
	//Bit 0,1: tipo de color: standard(0), spectra(1), ulaplus(2), valor 3 no usado

	//Sumar desplazamiento de paleta
	if (tipocolor==1 || tipocolor==2) {
		paletacolor*=16;
		color_final +=paletacolor;

		if (tipocolor==1) color_final +=SPECTRA_INDEX_FIRST_COLOR;
		if (tipocolor==2) color_final=ulaplus_palette_table[color_final]+ULAPLUS_INDEX_FIRST_COLOR;
	}

	return color_final;
}


//Hacer espejo horizontal de los 8 pixeles del sprite
void sprite_mirror_horizontal(z80_byte *buf_origen,z80_byte *buf_destino)
{

	int i;
	z80_byte pixel_izq,pixel_der;

	z80_byte byte_final;


	for (i=0;i<4;i++) {
		//leemos los dos pixeles
		pixel_izq=(buf_origen[i]>>4)&15;
		pixel_der=(buf_origen[i])&15;

		//componer byte final
		byte_final=(pixel_der<<4)|pixel_izq;

		//escribir byte final
		buf_destino[3-i]=byte_final;

	}


}

void spritechip_do_overlay(void)
{

	//spritechip presente?
	if (spritechip_enabled.v==0) return;

	//spritechip activo o no?
	if ( (spritechip_registers[0]&1)==0) return;

	z80_int sprite_table;

	sprite_table=value_8_to_16(spritechip_registers[2],spritechip_registers[1]);

	//printf ("sprite_table at %d\n",sprite_table);


	z80_int total_sprites=value_8_to_16(peek_byte_no_time(sprite_table+1),peek_byte_no_time(sprite_table) );

	//printf ("total sprites: %d\n",total_sprites);

	//Maximo sprites=1638
	//Cada sprite ocupa 40 bytes
	//65536/40=1638.4
	if (total_sprites>1638) total_sprites=1638;

	sprite_table +=2;



	int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;
	int y=t_scanline_draw-screen_invisible_borde_superior;
        if (border_enabled.v==0) y=y-screen_borde_superior;
	z80_int *puntero_buf_rainbow;
        puntero_buf_rainbow=&rainbow_buffer[ y*get_total_ancho_rainbow() ];
        puntero_buf_rainbow +=screen_total_borde_izquierdo*border_enabled.v;

	//Bucle para cada sprite
	int conta_sprites;
	for (conta_sprites=0;conta_sprites<total_sprites;conta_sprites++) {

		//Obtener atributos sprite
		//Primero coordenada y
		z80_int sprite_y;
		sprite_y=value_8_to_16(peek_byte_no_time(sprite_table+5),peek_byte_no_time(sprite_table+4) );
		//printf ("sprite %d coordenada y %d\n",conta_sprites,sprite_y);

		//alto sprite=8
		z80_int alto_sprite=8;

		//z80_int ancho_sprite=8;


		z80_byte atributosprite=(peek_byte_no_time(sprite_table+6));
		z80_byte atributosprite2=(peek_byte_no_time(sprite_table+7));
		z80_byte sprite_activo=(atributosprite) & 1;

		z80_byte parpadeo_sprite=(atributosprite)&32;

		z80_byte zoom_x_sprite=(atributosprite)&8;
		z80_byte zoom_y_sprite=(atributosprite)&16;

		//si hay zoom y, alto*2
		if (zoom_y_sprite) alto_sprite *=2;


		//Si tiene parpadeo el sprite y estado parpadeo activo
		if (estado_parpadeo.v && parpadeo_sprite) parpadeo_sprite=1;

		//sino, no hay parpadeo
		else parpadeo_sprite=0;

		//Atributos2
		z80_byte tipocolor=(atributosprite2)&3;
		/*
Offset 7: Byte atributos2:
Bit 0,1: tipo de color: standard(0), spectra(1), ulaplus(2), valor 3 no usado
Bit 2,3: Aunque color es 0-15, cada sprite tiene dos bits que indica, para todo el sprite, qué subgrupo de color (en ulaplus y spectra) usa:
00: de 0 a 15
01: de 16 a 31
10: de 32 a 47
11: de 48 a 63
		*/

		z80_byte paletacolor=(atributosprite2>>2)&3;



		//Si coordenada y esta en margen y sprite activo
		int diferencia=scanline_copia-sprite_y;

		//Pintar el sprite si esta en rango de coordenada y, si esta el sprite activo, y siempre que con parpadeo activo no este inverso en este momento
		if (diferencia>=0 && diferencia<alto_sprite && scanline_copia<192 && sprite_activo && !parpadeo_sprite) {


/*
Offset 6: Byte atributos1:
Bit 0 Activo o no activo
Bit 1 Espejo x
Bit 2 Espejo y
*/
			//Poner puntero a donde esta esa linea
			//saltamos cabecera
			z80_int sprite_particular=sprite_table+8;


			//Si zoom y activo, incremento en tabla
			int salto_y=(zoom_y_sprite ? diferencia/2 : diferencia);

			//saltamos y*4 (cada linea ocupa 4 bytes)
			//Ver si hay espejo y
			if (atributosprite&4) {
				//Situarse desde abajo hasta arriba
				//yendose hasta la linea 7 y restando
				sprite_particular=sprite_particular+(4*7)-salto_y*4;
			}

			else {
				sprite_particular +=salto_y*4;
			}

			//aumentamos puntero rainbow con coordenada x
			z80_int sprite_x;
			sprite_x=value_8_to_16(peek_byte_no_time(sprite_table+3),peek_byte_no_time(sprite_table+2) );
			//printf ("sprite %d coordenada x %d\n",conta_sprites,sprite_x);

			z80_int *puntero_buf_rainbow_sprite;
			puntero_buf_rainbow_sprite=puntero_buf_rainbow+sprite_x;

			z80_byte byte_leido;
			z80_int colorleido;
			z80_int colorfinal;


			//Meter los siguientes 4 bytes (8 pixeles de ancho de sprite) en buffer temporal. Para poder hacer espejo horizontal si conviene
			z80_byte buf_sprite[4];

			int i;
			for (i=0;i<4;i++) {
				byte_leido=peek_byte_no_time(sprite_particular++);
				buf_sprite[i]=byte_leido;
			}

			z80_byte *puntero_buf_sprite=buf_sprite;

			z80_byte buf_sprite_mirror[4];

			//Si esta atributo de espejo horizontal
			if (atributosprite&2) {

				sprite_mirror_horizontal(buf_sprite,buf_sprite_mirror);
				puntero_buf_sprite=buf_sprite_mirror;

			}


        	              int ancho;
				//int indice_sprite_ancho=0;
                	      for (ancho=0;ancho<8 && sprite_x<256;ancho+=2) {
					//byte_leido=peek_byte_no_time(sprite_particular++);
					byte_leido=*puntero_buf_sprite;
					puntero_buf_sprite++;
					colorleido=(byte_leido>>4)&15;

					colorfinal=spritechip_da_color_final(colorleido,tipocolor,paletacolor);

					//Color 0 transparente
					if (colorleido) *puntero_buf_rainbow_sprite=colorfinal;
					puntero_buf_rainbow_sprite++;
					sprite_x++;

					if (zoom_x_sprite) {
						if (sprite_x<256) {
							if (colorleido) *puntero_buf_rainbow_sprite=colorfinal;
							puntero_buf_rainbow_sprite++;
							sprite_x++;
						}
					}


					if (sprite_x==256) break;

					colorleido=byte_leido&15;
					colorfinal=spritechip_da_color_final(colorleido,tipocolor,paletacolor);

					//Color 0 transparente

					if (colorleido) *puntero_buf_rainbow_sprite=colorfinal;
					puntero_buf_rainbow_sprite++;
					sprite_x++;

					if (zoom_x_sprite) {
						if (sprite_x<256) {
							if (colorleido) *puntero_buf_rainbow_sprite=colorfinal;
							puntero_buf_rainbow_sprite++;
							sprite_x++;
						}
					}

				}
		}


		//8 bytes de la cabecera y 32 bytes de lo que ocupa el sprite
		sprite_table=sprite_table+8+32;

	}

}




//Max 8 lineas
z80_byte buffer_rotar_vertical[32*8];

//Rotar o rellenar con 0 las lineas que quedan por debajo
void sprite_chip_scroll_vertical_arr_rotar_metebuffer(z80_int x,z80_int y,z80_int ancho,z80_int alto,z80_byte bit_relleno,z80_byte cuantos_pixeles)
{
        //Mientras haya ancho y coordenada x no se salga de rango
        if (x+ancho>32) ancho=32-x;

        if (cuantos_pixeles>8) cuantos_pixeles=8;

        z80_byte *screen=get_base_mem_pantalla();
        z80_int direccion;
        z80_byte byte_destino;

        if (bit_relleno==2) {
                //rotar
                z80_byte *puntero_destino;
                puntero_destino=buffer_rotar_vertical;
                z80_int inicial_y=y+alto-cuantos_pixeles;
                for (;cuantos_pixeles>0;cuantos_pixeles--,inicial_y++,y++) {
                        //direccion=screen_addr_table[(inicial_y*32)+x];
                        //z80_byte *puntero_destino;
                        //puntero_destino=&screen[direccion];

                        direccion=screen_addr_table[(y*32)+x];
                        z80_byte *puntero_origen;
                        puntero_origen=&screen[direccion];

                        int j;

                        for (j=0;j<ancho;j++) {
                                byte_destino=*puntero_origen;
                                *puntero_destino=byte_destino;
                                puntero_origen++;
                                puntero_destino++;
                        }
                }
        }


}

//Rotar o rellenar con 0 las lineas que quedan por arriba
void sprite_chip_scroll_vertical_aba_rotar_metebuffer(z80_int x,z80_int y,z80_int ancho,z80_int alto,z80_byte bit_relleno,z80_byte cuantos_pixeles)
{
        //Mientras haya ancho y coordenada x no se salga de rango
        if (x+ancho>32) ancho=32-x;

        if (cuantos_pixeles>8) cuantos_pixeles=8;

        z80_byte *screen=get_base_mem_pantalla();
        z80_int direccion;
        z80_byte byte_destino;

        if (bit_relleno==2) {
                //rotar
                z80_byte *puntero_destino;
                puntero_destino=buffer_rotar_vertical;
		y=y+alto-1;
                for (;cuantos_pixeles>0;cuantos_pixeles--,y--) {

                        direccion=screen_addr_table[(y*32)+x];
                        z80_byte *puntero_origen;
                        puntero_origen=&screen[direccion];

                        int j;

                        for (j=0;j<ancho;j++) {
                                byte_destino=*puntero_origen;
                                *puntero_destino=byte_destino;
                                puntero_origen++;
                                puntero_destino++;
                        }
                }
        }


}



//Rotar o rellenar con 0 las lineas que quedan por debajo
void sprite_chip_scroll_vertical_arr_rotar(z80_int x,z80_int y,z80_int ancho,z80_int alto,z80_byte bit_relleno,z80_byte cuantos_pixeles)
{

        //Mientras haya ancho y coordenada x no se salga de rango
        if (x+ancho>32) ancho=32-x;

	z80_byte *screen=get_base_mem_pantalla();
	z80_int direccion;
	z80_byte byte_destino;

	if (bit_relleno==0 || bit_relleno==1) {
		z80_int inicial_y=y+alto-cuantos_pixeles;
		for (;cuantos_pixeles>0;cuantos_pixeles--,inicial_y++) {
			direccion=screen_addr_table[(inicial_y*32)+x];
			z80_byte *puntero;
			puntero=&screen[direccion];

			int j;

			for (j=0;j<ancho;j++) {
				byte_destino=(bit_relleno ? 255 : 0);
				*puntero=byte_destino;
				puntero++;
			}
		}
	}

	if (bit_relleno==2) {
		//rotar
		z80_byte *puntero_origen;
		puntero_origen=buffer_rotar_vertical;
		z80_int inicial_y=y+alto-cuantos_pixeles;
                for (;cuantos_pixeles>0;cuantos_pixeles--,inicial_y++,y++) {
                        direccion=screen_addr_table[(inicial_y*32)+x];
                        z80_byte *puntero_destino;
                        puntero_destino=&screen[direccion];

			//direccion=screen_addr_table[(y*32)+x];
			//z80_byte *puntero_origen;
			//puntero_origen=&screen[direccion];

			int j;

                        for (j=0;j<ancho;j++) {
                                byte_destino=*puntero_origen;
                                *puntero_destino=byte_destino;
                                puntero_origen++;
				puntero_destino++;
                        }
                }
        }

}



//Rotar o rellenar con 0 las lineas que quedan por arriba
void sprite_chip_scroll_vertical_aba_rotar(z80_int x,z80_int y,z80_int ancho,z80_int alto GCC_UNUSED,z80_byte bit_relleno,z80_byte cuantos_pixeles)
{

        //Mientras haya ancho y coordenada x no se salga de rango
        if (x+ancho>32) ancho=32-x;

        z80_byte *screen=get_base_mem_pantalla();
        z80_int direccion;
        z80_byte byte_destino;

        if (bit_relleno==0 || bit_relleno==1) {
                z80_int inicial_y=y+cuantos_pixeles-1;
                for (;cuantos_pixeles>0;cuantos_pixeles--,inicial_y--) {
                        direccion=screen_addr_table[(inicial_y*32)+x];
                        z80_byte *puntero;
                        puntero=&screen[direccion];

                        int j;

                        for (j=0;j<ancho;j++) {
                                byte_destino=(bit_relleno ? 255 : 0);
                                *puntero=byte_destino;
                                puntero++;
                        }
                }
        }

        if (bit_relleno==2) {
                //rotar
                z80_byte *puntero_origen;
                puntero_origen=buffer_rotar_vertical;
                z80_int inicial_y=y+cuantos_pixeles-1;
                for (;cuantos_pixeles>0;cuantos_pixeles--,inicial_y--) {
                        direccion=screen_addr_table[(inicial_y*32)+x];
                        z80_byte *puntero_destino;
                        puntero_destino=&screen[direccion];

                        int j;

                        for (j=0;j<ancho;j++) {
                                byte_destino=*puntero_origen;
				*puntero_destino=byte_destino;
                                puntero_origen++;
                                puntero_destino++;
                        }
                }
        }

}



//Scroll de la memoria de video en vertical y hacia arriba de una linea concreta
//Y en coordenadas de pixel (0..191)
//X en coordenadas de fila (0..31)
//ancho en coordenadas de fila (0..31)
//Byte relleno si se mete 0 o 1 en byte final
void sprite_chip_scroll_vertical_arr(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno GCC_UNUSED,z80_byte cuantos_pixeles)
{
	z80_byte *screen=get_base_mem_pantalla();

	z80_int direccion;

	direccion=screen_addr_table[(y*32)+x];

        z80_byte *puntero_destino;

        puntero_destino=&screen[direccion];


	direccion=screen_addr_table[((y+cuantos_pixeles)*32)+x];

	z80_byte *puntero_origen;

	puntero_origen=&screen[direccion];


	//Mover (ancho) bytes de origen a destino

        //Mientras haya ancho y coordenada x no se salga de rango
        if (x+ancho>32) ancho=32-x;

	int i;

	z80_byte byte_actual;

	for (i=0;i<ancho;i++) {
		byte_actual=*puntero_origen;
		*puntero_destino=byte_actual;

		puntero_origen++;
		puntero_destino++;
	}
}

//Scroll de la memoria de video en vertical y hacia abajo de una linea concreta
//Y en coordenadas de pixel (0..191)
//X en coordenadas de fila (0..31)
//ancho en coordenadas de fila (0..31)
//Byte relleno si se mete 0 o 1 en byte final
void sprite_chip_scroll_vertical_aba(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno GCC_UNUSED,z80_byte cuantos_pixeles)
{

	//printf ("sprite_chip_scroll_vertical_aba y: %d\n",y);

        z80_byte *screen=get_base_mem_pantalla();

        z80_int direccion;

        direccion=screen_addr_table[(y*32)+x];

        z80_byte *puntero_destino;

        puntero_destino=&screen[direccion];


        direccion=screen_addr_table[((y-cuantos_pixeles)*32)+x];

        z80_byte *puntero_origen;

        puntero_origen=&screen[direccion];


        //Mover (ancho) bytes de origen a destino

        //Mientras haya ancho y coordenada x no se salga de rango
        if (x+ancho>32) ancho=32-x;

        int i;

        z80_byte byte_actual;

        for (i=0;i<ancho;i++) {
                byte_actual=*puntero_origen;
                *puntero_destino=byte_actual;

                puntero_origen++;
                puntero_destino++;
        }
}




//Scroll de la memoria de video en horizontal y hacia la izquierda de una linea concreta
//Y en coordenadas de pixel (0..191)
//X en coordenadas de fila (0..31)
//ancho en coordenadas de fila (0..31)
//Byte relleno si se mete 0 o 1 en byte final
void sprite_chip_scroll_horizontal_izq(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno,z80_byte cuantos_pixeles)
{
        z80_byte *screen=get_base_mem_pantalla();

	z80_int direccion;

	//Mientras haya ancho y coordenada x no se salga de rango
	if (x+ancho>32) ancho=32-x;

        direccion=screen_addr_table[(y*32)+x];

	//76543210 76543210 76543210
	//scroll:
	//65432107 65432107 65432100

	z80_byte byte_actual,byte_siguiente;

	z80_byte *puntero;

	puntero=&screen[direccion];

	byte_actual=*puntero;

	//Guarda el valor del byte que entra en el caso de scroll circular
	z80_byte byte_circular=0;

	int j;
	for (j=0;j<cuantos_pixeles;j++) {
		byte_circular=byte_circular<<1;
		if (byte_actual&128) byte_circular |=1;
		byte_actual=(byte_actual<<1)&254;
	}



	for (;ancho>0;ancho--) {
		//rotar byte actual a la izquierda borrando bit 0
		//byte_actual=(byte_actual<<1)&254;

		z80_byte byte_temporal=0;


		int total_pixeles;
		//Leer byte actual y siguiente
		byte_siguiente=*(puntero+1);

		for (total_pixeles=0;total_pixeles<cuantos_pixeles;total_pixeles++) {

			byte_temporal=byte_temporal<<1;

			//Y meter bit 0 de siguiente byte
			if (ancho>1) {
				if (byte_siguiente&128) byte_temporal |=1;
				byte_siguiente=(byte_siguiente<<1)&254;
			}

			else {
				if (bit_relleno==1) byte_temporal |=1;
			}

		}
		if (ancho==1 && bit_relleno==2) byte_temporal=byte_circular;
		byte_actual |=byte_temporal;


		//Escribir byte
		*puntero=byte_actual;

		//Byte siguiente
		byte_actual=byte_siguiente;

		puntero++;

		x++;
	}
}

//Scroll de la memoria de video en horizontal y hacia la derecha de una linea concreta
//Y en coordenadas de pixel (0..191)
//X en coordenadas de fila (0..31)
//ancho en coordenadas de fila (0..31)
//Byte relleno si se mete 0 o 1 en byte final
void sprite_chip_scroll_horizontal_der(z80_int x,z80_int y,z80_int ancho,z80_byte bit_relleno,z80_byte cuantos_pixeles)
{
        z80_byte *screen=get_base_mem_pantalla();

	z80_int direccion;

	//Mientras haya ancho y coordenada x no se salga de rango
	if (x+ancho>32) ancho=32-x;

        direccion=screen_addr_table[(y*32)+x+ancho-1];

	//76543210 76543210 76543210
	//scroll:
	//65432107 65432107 65432100

	z80_byte byte_actual,byte_siguiente;

	z80_byte *puntero;

	puntero=&screen[direccion];

	byte_actual=*puntero;

	//Guarda el valor del byte que entra en el caso de scroll circular
	z80_byte byte_circular=0;

	int j;
	for (j=0;j<cuantos_pixeles;j++) {
		byte_circular=byte_circular>>1;
		if (byte_actual&1) byte_circular |=128;
		byte_actual=(byte_actual>>1)&127;
	}



	for (;ancho>0;ancho--) {
		//rotar byte actual a la izquierda borrando bit 0
		//byte_actual=(byte_actual<<1)&254;

		z80_byte byte_temporal=0;


		int total_pixeles;
		//Leer byte actual y siguiente
		byte_siguiente=*(puntero-1);

		for (total_pixeles=0;total_pixeles<cuantos_pixeles;total_pixeles++) {

			byte_temporal=byte_temporal>>1;

			//Y meter bit 7 de siguiente byte
			if (ancho>1) {
				if (byte_siguiente&1) byte_temporal |=128;
				byte_siguiente=(byte_siguiente>>1)&127;
			}

			else {
				if (bit_relleno==1) byte_temporal |=128;
			}

		}
		if (ancho==1 && bit_relleno==2) byte_temporal=byte_circular;
		byte_actual |=byte_temporal;


		//Escribir byte
		*puntero=byte_actual;

		//Byte siguiente
		byte_actual=byte_siguiente;

		puntero--;

		x--;
	}
}




void spritechip_do_scroll(void)
{
        //spritechip presente?
        if (spritechip_enabled.v==0) return;

        //spritechip activo o no?
        if ( (spritechip_registers[0]&1)==0) return;

	//Si hay un scroll en curso
	if (spritechip_registers[9]==0) return;


                int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;
                //int y=t_scanline_draw-screen_invisible_borde_superior;
                //if (border_enabled.v==0) y=y-screen_borde_superior;

		int y=scanline_copia;

/*
CMD3: Posicion x de base de scroll. x en coordenadas de fila (0..31)
CMD4: Posicion y de base de scroll. y en coordenadas de pixel (0..191). Se va incrementando en scroll horizontal
CMD5: Ancho de scroll. En coordenadas de fila (1..32).
CMD6: Alto de scroll. En coordenadas de pixel (0..191). Se va decrementando en scroll horizontal y vertical
CMD7: Pixeles a desplazar en scroll
CMD8: Byte de relleno en scroll. 0=meter 0. 1=meter 1. 2=meter circular
CMD9: Iniciar scroll. Tipo de scroll enviar a puerto de datos: 1: Arriba, 2: Abajo, 3: Izquierda, 4: Derecha
*/

	//Si linea actual = linea base scroll
	z80_int origen_y=spritechip_registers[4];
	if (y!=origen_y) return;

	//Scroll para la linea actual
	z80_int origen_x=spritechip_registers[3];

	z80_int ancho_scroll=spritechip_registers[5];

	z80_byte alto_scroll=spritechip_registers[6];

	//Controlar si alto se va de rango
	if (origen_y+alto_scroll>192) alto_scroll=192-origen_y;

	z80_byte inicial_y=spritechip_registers[10];

	//Tipo scroll
	z80_byte tipo_scroll=spritechip_registers[9];
	switch (tipo_scroll) {
		case 1:
			//printf ("scroll vert arr de x %d y %d ancho %d relleno %d cuantos pixeles %d\n",origen_x,y,ancho_scroll,spritechip_registers[8],spritechip_registers[7]);
                        //Ver si esta en la primera linea, rotar o llenar con 0 o con 1 las lineas finales
                        if (y==inicial_y) {
                                sprite_chip_scroll_vertical_arr_rotar_metebuffer(origen_x,y,ancho_scroll,alto_scroll,spritechip_registers[8],spritechip_registers[7]);
                        }




			//Ver si esta en las lineas inferiores que se pierden
			if (alto_scroll>spritechip_registers[7]) {
				sprite_chip_scroll_vertical_arr(origen_x,y,ancho_scroll,spritechip_registers[8],spritechip_registers[7]);
			}

			else {
				if (alto_scroll==spritechip_registers[7])
                                sprite_chip_scroll_vertical_arr_rotar(origen_x,y,ancho_scroll,alto_scroll,spritechip_registers[8],spritechip_registers[7]);
			}

			//TODO. Rotar o llenar con 0 o con 1
		break;

		case 2:
			//printf ("scroll vert aba de x %d y %d ancho %d alto %d relleno %d cuantos pixeles %d\n",origen_x,y,ancho_scroll,alto_scroll,spritechip_registers[8],spritechip_registers[7]);
                        //Ver si esta en la primera linea, rotar o llenar con 0 o con 1 las lineas finales
                        if (y==inicial_y) {
                                sprite_chip_scroll_vertical_aba_rotar_metebuffer(origen_x,y,ancho_scroll,alto_scroll,spritechip_registers[8],spritechip_registers[7]);
                        }

			//Ver si esta en las lineas inferiores que se pierden
			if (alto_scroll>spritechip_registers[7]) {
				sprite_chip_scroll_vertical_aba(origen_x,inicial_y+alto_scroll-1,ancho_scroll,spritechip_registers[8],spritechip_registers[7]);
			}

			else {
				if (alto_scroll==spritechip_registers[7])
				sprite_chip_scroll_vertical_aba_rotar(origen_x,inicial_y,ancho_scroll,alto_scroll,spritechip_registers[8],spritechip_registers[7]);
                        }


			//TODO. Rotar o llenar con 0 o con 1
		break;



		case 3:
			//printf ("scroll horiz izq de x %d y %d ancho %d relleno %d cuantos pixeles %d\n",origen_x,y,ancho_scroll,spritechip_registers[8],spritechip_registers[7]);
			sprite_chip_scroll_horizontal_izq(origen_x,y,ancho_scroll,spritechip_registers[8],spritechip_registers[7]);
		break;
		case 4:
			//printf ("scroll horiz der de x %d y %d ancho %d relleno %d cuantos pixeles %d\n",origen_x,y,ancho_scroll,spritechip_registers[8],spritechip_registers[7]);
			sprite_chip_scroll_horizontal_der(origen_x,y,ancho_scroll,spritechip_registers[8],spritechip_registers[7]);
		break;
	}

	//Incrementar posicion y
	origen_y++;
	spritechip_registers[4]=origen_y;

	//Fin y decrementar alto
	alto_scroll--;
	spritechip_registers[6]=alto_scroll;

	//Fin scroll
	if (alto_scroll==0) {
		spritechip_registers[9]=0;
	}

	//Si alto iba mas alla de la pantalla, fin scroll
	if (y>=191) {
		spritechip_registers[9]=0;
	}

}
