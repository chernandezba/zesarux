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


#include "printers.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "screen.h"
#include "menu.h"

//Si zx printer esta activa
z80_bit zxprinter_enabled;
//Si motor activo
z80_bit zxprinter_motor;
//Si potencia (bit a 1) o sin potencia (bit a 0)
z80_bit zxprinter_power;
//Velocidad
int zxprinter_speed=1;
//Posicion x de impresion
int zxprinter_x;


//Buffer de imagen (256 pixeles - 32 bytes de ancho * 8 lineas de alto)
z80_byte zxprinter_image_buffer[32*8];

//Posicion dentro del bit de impresion (0..7)
z80_byte zxprinter_x_bit;

//Posicion y de impresion dentro del buffer de imagen
int zxprinter_y;

//Puntero que indica nombre de archivo donde se genera el ocr - conversion de imagen a texto
//NULL si no esta activo
char *zxprinter_ocr_filename=NULL;

//Puntero a archivo ocr
FILE *ptr_zxprinter_ocr;

//Puntero que indica nombre de archivo donde se genera el archivo bitmap: jpg, pnm, txt...
//NULL si no esta activo
char *zxprinter_bitmap_filename=NULL;

//Puntero a archivo bitmap
FILE *ptr_zxprinter_bitmap;

//Alto total de la imagen desde que se abre archivo hasta que se hace close
int zxprinter_alto_total;


//si hay impresion en curso
//contador se decrementa a cada segundo
//sirve para indicar mediante overlay en pantalla que se esta imprimiendo
//despues de imprimir, permanece durante x segundos en pantalla
//int printing_counter=0;

void draw_print_text(void)
{


		generic_footertext_print_operating("PRINT");

}



//Escribir bit en buffer de impresion
void zxprinter_write_bit_to_buffer(z80_bit value)
{
	//Obtenemos byte del buffer
	z80_byte byte_buffer=zxprinter_image_buffer[zxprinter_y*32+zxprinter_x/8];

	z80_byte mascara_bit=128;
	z80_byte valor_byte=128*value.v;

	if (zxprinter_x_bit>0) {
		mascara_bit=mascara_bit>>zxprinter_x_bit;
		valor_byte=valor_byte>>zxprinter_x_bit;
	}

	mascara_bit ^=255;

	byte_buffer=(byte_buffer&mascara_bit)|valor_byte;

	zxprinter_image_buffer[zxprinter_y*32+zxprinter_x/8]=byte_buffer;

	//siguiente bit
	zxprinter_x_bit++;
	if (zxprinter_x_bit==8) zxprinter_x_bit=0;

}

//Escribir cabecera pbm
void zxprinter_write_pbm_header(void)
{
	//17 ocupa la cabecera, guardamos 1 mas para el codigo 0 de sprintf pero que no quedara en la cabecera
	char pbm_header[18];

	//hacemos SEEK al principio
	if (fseek(ptr_zxprinter_bitmap, 0L, SEEK_SET)!=0) {
		debug_printf (VERBOSE_ERR,"Error seeking to the start of file to update header");
		return;
	}


	sprintf (pbm_header,"P4\n256%10d\n",zxprinter_alto_total);

	fwrite(pbm_header,1,17,ptr_zxprinter_bitmap);
}


//Escribir imagen bitmap pbm
void zxprinter_write_bitmap_pbm(void)
{

	debug_printf(VERBOSE_DEBUG,"Writing one line of bitmap to pbm file");

	int x;
	z80_byte valor;


	for (x=0;x<32;x++) {
		valor=zxprinter_image_buffer[zxprinter_y*32+x];
		fwrite(&valor,1,1,ptr_zxprinter_bitmap);
	}


	fflush(ptr_zxprinter_bitmap);

}

//Escribir imagen bitmap pero en archivo de texto
void zxprinter_write_bitmap_txt(void)
{

	debug_printf(VERBOSE_DEBUG,"Writing one line of bitmap to txt file");

	int x, bit_x;
	z80_byte valor;
	unsigned char caracter_salida;

	for (x=0;x<32;x++) {
		valor=zxprinter_image_buffer[zxprinter_y*32+x];
		//printf ("C%x",valor);
		for (bit_x=0;bit_x<8;bit_x++) {
			if (valor&128) caracter_salida='#';
			else caracter_salida=' ';

			valor=valor<<1;

			fwrite(&caracter_salida,1,1,ptr_zxprinter_bitmap);
		}
	}


	caracter_salida='\n';
	fwrite(&caracter_salida,1,1,ptr_zxprinter_bitmap);

	//y hacemos flush
	fflush(ptr_zxprinter_bitmap);


}



//Escribir linea de impresion hacia archivo de disco
void zxprinter_write_buffer(void)
{

	/* debug

	int i;

	for (i=0;i<32;i++) {
		printf ("%02x",zxprinter_image_buffer[zxprinter_y*32+i]);
	}


	printf("  ");

	z80_byte c;
	for (i=0;i<32;i++) {
		c=zxprinter_image_buffer[zxprinter_y*32+i];
		if (c>31 && c<128) printf ("%c",c);
		else printf (".");
	}

	printf("\n");

	*/



	//Si tipo de archivo bitmap
	if (zxprinter_bitmap_filename!=NULL) {
		//Segun tipo de archivo

		//Bitmap TXT
		if (!util_compare_file_extension(zxprinter_bitmap_filename,"txt")) {
			zxprinter_write_bitmap_txt();
		}

		else if (!util_compare_file_extension(zxprinter_bitmap_filename,"pbm")) {
                        zxprinter_write_bitmap_pbm();
                }


		else {
			debug_printf (VERBOSE_DEBUG,"Unknown bitmap printer file");
		}
	}
}

void zxprinter_write_buffer_ocr(void)
{
	//Convierte todo el buffer de imagen de impresion a texto
	debug_printf(VERBOSE_DEBUG,"Writing one character line to txt file");
	int i;

	z80_byte buffer_salida[33];

	z80_byte caracter;
	z80_byte inverse;

	for (i=0;i<32;i++) {
		//printf ("%x ",zxprinter_image_buffer[i]);

		caracter=compare_char_step(&zxprinter_image_buffer[i],&inverse,32);
		if (!caracter) caracter='?';

		buffer_salida[i]=caracter;

	}

	buffer_salida[i]='\n';

	fwrite(buffer_salida,1,33,ptr_zxprinter_ocr);

	fflush(ptr_zxprinter_ocr);
	
}

//salto de linea
void zxprinter_cr(void)
{

	//-1 porque parece que nos "sobra" el primer pixel de linea
	zxprinter_x=-1;


	zxprinter_write_buffer();

	zxprinter_y++;
	zxprinter_alto_total++;


	//printf ("x: %d y: %d\n");

	if (zxprinter_y==8) {

		if (zxprinter_ocr_filename!=NULL) zxprinter_write_buffer_ocr();
		
		zxprinter_y=0;
	}
}


z80_byte zxprinter_get_port(void)
{                        //temp
                        /*
                        (D6) Will be read as low if the printer is there, high if it is not, and is used solely to check if the printer is connected.
                        (D0) This is high when the printer is ready for the next bit.
                        (D7) This line is high for the start of a new line.
                        */
                        return  255-64;
}

void zxprinter_write_port(z80_byte value)
{

			draw_print_text();

                        //En realidad esto es mucho mas complicado, pero para las rutinas de la zxprinter de la rom ya funciona
                        //habria que tener en cuenta que el motor se pone en marcha y va avanzando...
                        //Desde la rom en cambio siempre se va enviando 0 o 1 segun el bit que toque, aunque sea todo el rato a 1
                        /*
                        (D1) Speed
                        (D2) High level means stop the motor, low means start it.
                        (D7) High level applies power to the print head.
                        */
                        //printf ("out zxprinter: %x\n",value);

                        if ((value&2)==0) {
                                zxprinter_speed=2;
                                //printf ("speed fast\n");
                        }
                        else {
                                zxprinter_speed=1;
                                //printf ("speed slow\n");
                        }


                        if ((value&4)==0) {
                                zxprinter_motor.v=1;
                                //printf ("motor on\n");
                        }
                        else {
                                zxprinter_motor.v=0;
                                //printf ("motor off\n");
                        }


                        if ((value&128)==0) {
                                zxprinter_power.v=0;
                                //printf ("power off\n");
                                //temp
                        }
                        else {
                                zxprinter_power.v=1;
                                //printf ("power on\n");
                        }



                        if (zxprinter_motor.v) {
                                if (zxprinter_x>=0 && zxprinter_x<256) {
					zxprinter_write_bit_to_buffer(zxprinter_power);
                                        //if (zxprinter_power.v) printf ("#");
                                        //else printf (" ");
                                }
                                zxprinter_x++;

			        //en zx81, menos
			        if (MACHINE_IS_ZX8081) {
					if (zxprinter_x>=257) zxprinter_cr();
				}

				else {

	                                if (zxprinter_x>=256) {
        	                                //printf ("\n");
		
						zxprinter_cr();
                        	        }
				}
                        }

}


void eject_zxprinter_bitmap_file(void)
{
	zxprinter_bitmap_filename=NULL;
}

void eject_zxprinter_ocr_file(void)
{
        zxprinter_ocr_filename=NULL;
}

void zxprinter_init_buffer(void)
{
	zxprinter_x_bit=0;
	zxprinter_x=-1;

	zxprinter_y=0;
	zxprinter_alto_total=0;
}


int zxprinter_file_bitmap_init(void)
{

	//append, binary
        ptr_zxprinter_bitmap=fopen(zxprinter_bitmap_filename,"wb");

        if (!ptr_zxprinter_bitmap) {
                debug_printf(VERBOSE_ERR,"Unable to open zxprinter bitmap file %s",zxprinter_bitmap_filename);
		
		
                eject_zxprinter_bitmap_file();
                return 1;
        }

	zxprinter_init_buffer();


	//Inicializar archivo pbm si conviene
	if (!util_compare_file_extension(zxprinter_bitmap_filename,"pbm")) {
		zxprinter_write_pbm_header();
        }


        return 0;
                        
}  

int zxprinter_file_ocr_init(void)
{

        ptr_zxprinter_ocr=fopen(zxprinter_ocr_filename,"wb");

        if (!ptr_zxprinter_ocr) {
                debug_printf(VERBOSE_ERR,"Unable to open zxprinter OCR file %s",zxprinter_ocr_filename);


                eject_zxprinter_ocr_file();
                return 1;
        }

	zxprinter_init_buffer();

        return 0;

}



//Cerrar archivo bitmap
void close_zxprinter_bitmap_file(void)
{

	if (zxprinter_bitmap_filename==NULL) {
                debug_printf (VERBOSE_INFO,"Closing zx printer bitmap file. But already closed");
        }

	else {
		debug_printf (VERBOSE_INFO,"Closing zx printer bitmap file");

		if (!util_compare_file_extension(zxprinter_bitmap_filename,"pbm")) {
			zxprinter_write_pbm_header();
		}

		fclose(ptr_zxprinter_bitmap);

		eject_zxprinter_bitmap_file();
	}

}

//Cerrar archivo ocr
void close_zxprinter_ocr_file(void)
{

        if (zxprinter_ocr_filename==NULL) {
                debug_printf (VERBOSE_INFO,"Closing zx printer OCR file. But already closed");
        }

        else {
                debug_printf (VERBOSE_INFO,"Closing zx printer OCR file");
                fclose(ptr_zxprinter_ocr);
                eject_zxprinter_ocr_file();
        }

}

