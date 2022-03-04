#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif



char file_orig[PATH_MAX];

#define MAX_ANCHO_SPRITE 256
#define MAX_ALTO_SPRITE 192

unsigned char sprites[MAX_ALTO_SPRITE][MAX_ANCHO_SPRITE/2];

/*unsigned char tabla_conversion_colour_bmp[16]={
	0, 1, 7, 3, 8, 2, 15,10,
	8, 9, 10,11,12, 2, 14,15
};
*/

unsigned char tabla_conversion_colour_bmp[16]={
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10,11,12,13,14,15
};

int main(int argc,char *argv[])
{
/*
Formato BMP:

76h bytes header
Linea a linea empezando por la ultima


Formato tabla sprites:
8 bytes cabecera
A partir de aqui, datos sprite. 16 colores por pixel. Color 0 indica transparente. Color 8 se puede usar para color negro en paleta standard
Primer byte:
Bits 7654: Color pixel de mas a la izquierda
Bits 3210: Color pixel de su derecha
Etc...

*/

	int ancho,alto;

	if (argc!=4) {
		printf ("%s input width height\n",argv[0]);
		exit(1);
	}

	sprintf (file_orig,"%s",argv[1]);
	ancho=atoi(argv[2]);
	alto=atoi(argv[3]);

	printf (";Converting file %s with size %dX%d\n",file_orig,ancho,alto);

	

	//Leemos todo el bmp en array de sprites
        FILE *ptr_inputfile;
        ptr_inputfile=fopen(file_orig,"rb");

        if (ptr_inputfile==NULL) {
		printf ("Error opening %s\n",file_orig);
		exit(1);
        }

	//Saltamos 76h bytes cabecera (118)
	char buffer_nada[118];
	fread(buffer_nada,1,118,ptr_inputfile);

	int indice=alto;

	for (;indice>0;indice--) {
		fread(&sprites[indice-1][0],1,ancho/2,ptr_inputfile);
	}

        fclose(ptr_inputfile);

	int x,y;

	//ancho en sprites
	ancho=ancho/8;

	//alto en sprites
	alto=alto/8;

	//Mostrar cada grupo de sprite de 8x8
	for (y=0;y<alto;y++) {
		for (x=0;x<ancho;x++) {

			//Cabecera de cada sprite
			printf (";Sprite at %d,%d\n",x,y);
			printf ("\tdefb 0,0,0,0,0,0,0,0\n");

			int ancho_en_sprite,alto_en_sprite;
			//Cada sprite son 8x8. Ancho en bytes es 4
			for (alto_en_sprite=0;alto_en_sprite<8;alto_en_sprite++) {
				printf ("\tdefb ");
				for (ancho_en_sprite=0;ancho_en_sprite<4;ancho_en_sprite++) {
					//Mostrar los 4 bytes correspondientes
					int yarray=y*8+alto_en_sprite;
					int xarray=x*4+ancho_en_sprite;
					unsigned char dato=sprites[yarray][xarray];

					//Separar byte en los dos pixeles y pasar tabla de conversion de colores bmp
					unsigned char izq=(dato>>4)&15;
					unsigned char der=(dato&15);
					//printf ("(0x%X %d %d) ",dato,izq,der);
					
					//Pasar tabla conversion
					izq=tabla_conversion_colour_bmp[izq];
					der=tabla_conversion_colour_bmp[der];

					dato=(izq<<4)|der;
					//printf ("(C 0x%X ) ",dato);

					printf ("%d%c",dato,(ancho_en_sprite!=3 ? ',' : ' ') );
				}

				printf ("\n");
			}

		}
	}



	

return 0;

}
