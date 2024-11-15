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
This is a sample code to convert a gimp C image, 24 bit rgb, to 8x8 font raw
The original font is DOS font by Neil Roy
The code also skips vertical/horizontal lines which separes every character

This file is not built in the ZEsarUX code, you must compile it by yourself if you need it
Notice the array data is not complete, is only kept here as an example
*/

/* GIMP RGBA C-Source image dump (dos_8x8_font_green.c) */



/* GIMP RGB C-Source image dump (dos_8x8_font_green.c) */

static const struct {
  int  	 width;
  int  	 height;
  int  	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char 	 pixel_data[145 * 145 * 3 + 1];
} gimp_image = {
  145, 145, 3,
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
//Incomplete data... just removed to avoid large sample file
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377",
};
int get_pixel(int x,int y)
{
    //ancho total de la imagen
	int ancho=145;
	int offset=(y*ancho+x)*3;

    //Color en 24 bit
	unsigned int valor=(gimp_image.pixel_data[offset] << 16) |
			   (gimp_image.pixel_data[offset+1] << 8) |
			   (gimp_image.pixel_data[offset+2]   );

    //Color de fondo tratamos como 0
    if (valor==8553090) return 0;

    //Cualquier otra cosa es un 1
	else return 1;
}

void print_ascii_code(int codigo)
{
    int x,y;

    for (y=0;y<8;y++) {
        unsigned char valor_byte=0;
        for (x=0;x<8;x++) {

            //Imagen es de 16 caracteres de ancho
            int offset_fila=codigo/16;
            int offset_columna=codigo % 16;

            //cara caracter está a 9 pixeles, pues hay lineas de separacion verticales y horizontales
            int offset_y=offset_fila*9;
            int offset_x=offset_columna*9;
            int color=get_pixel(offset_x+x+1,offset_y+y+1); //+1 para saltar las lineas de separacion

            //if (color) printf("X");
            //else printf(" ");
            valor_byte=valor_byte<<1;

            if (color) valor_byte |=1;
            //printf("%d",color);


        }
        //printf("\n");
        printf("0x%02x,",valor_byte);
    }
}

main() {
	int x,y;

    /*
	for (y=0;y<145;y++) {
		for (x=0;x<145;x++) {
			int color=get_pixel(x,y);
			if (!color) printf(" ");
			else printf("X");
			//printf("%u ",color);
		}
		printf("\n");
	}
    */

   int codigo;
   for (codigo=32;codigo<128;codigo++) {
       //printf("codigo: %d\n",codigo);
       print_ascii_code(codigo);
       printf("\n");
   }
}
