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
char file_dest[PATH_MAX];
char file_plane_dest[PATH_MAX];

#define MAX_ANCHO_PANTALLA 256
#define MAX_ALTO_PANTALLA 192

//unsigned char sprites[MAX_ALTO_SPRITE][MAX_ANCHO_SPRITE/2];

/*unsigned char tabla_conversion_colour_bmp[16]={
	0, 1, 7, 3, 8, 2, 15,10,
	8, 9, 10,11,12, 2, 14,15
};
*/

unsigned char tabla_conversion_colour_bmp[16]={
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10,11,12,13,14,15
};


unsigned char source_bmp[MAX_ALTO_PANTALLA][MAX_ANCHO_PANTALLA/2];


unsigned char target_plane_zero[6144];
unsigned char target_plane_one[6144];
unsigned char target_plane_two[6144];
unsigned char target_plane_three[6144];


int devuelve_direccion_pantalla_no_table(unsigned char x,unsigned char y)
{
        
        unsigned char linea,high,low;
        
        linea=y/8;
        
        low=x+ ((linea & 7 )<< 5);
        high= (linea  & 24 )+ (y%8);
        


        return low+high*256;
}


void putpixel(unsigned char x,unsigned char y,unsigned char *target_plane,unsigned char value)
{
	int address=devuelve_direccion_pantalla_no_table(x/8,y);
	//Obtener numero de bit
	int bit_number=x%8;

	if (value) value=128;

	//rotar ese value tantas veces como diga el numero de bit
	for (;bit_number>0;bit_number--) value=value>>1;

	unsigned char mascara=255-value;

	//leer valor
	unsigned char valor_orig=target_plane[address];

	//aplicar mascara
	valor_orig=valor_orig&mascara;

	//Y meterle pixel
	valor_orig=valor_orig|value;

	target_plane[address]=valor_orig;

}


void write_file_plane(unsigned char *datos,char *archivo) {

                        FILE *ptr_epromfile;
                        ptr_epromfile=fopen(archivo,"wb");

                        fwrite(datos,1,6144,ptr_epromfile);

                        fclose(ptr_epromfile);

}






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

	if (argc!=3) {
		printf ("%s input output\n",argv[0]);
		exit(1);
	}

	sprintf (file_orig,"%s",argv[1]);
	sprintf (file_dest,"%s",argv[2]);
	ancho=256;
	alto=192;



	printf (";Converting file %s with size %dX%d\n",file_orig,ancho,alto);

	

	//Leemos todo el bmp en array de sprites
        FILE *ptr_inputfile;
        ptr_inputfile=fopen(file_orig,"rb");

        if (ptr_inputfile==NULL) {
		printf ("Error opening %s\n",file_orig);
		exit(1);
        }

	//Leemos 76h bytes cabecera (118)
	char buffer_cabecera[118];
	fread(buffer_cabecera,1,118,ptr_inputfile);

	int indice=alto;

	for (;indice>0;indice--) {
		fread(&source_bmp[indice-1][0],1,ancho/2,ptr_inputfile);
	}

        fclose(ptr_inputfile);

/*
	Escribimos paleta
*/

	sprintf(file_plane_dest,"%s_palette",file_dest);

                        FILE *ptr_epromfile;
                        ptr_epromfile=fopen(file_plane_dest,"wb");

                        fwrite(&buffer_cabecera[0x36],1,64,ptr_epromfile);

                        fclose(ptr_epromfile);




	int x,y;

	//Mostrar cada grupo de sprite de 8x8
	for (y=0;y<alto;y++) {
		for (x=0;x<ancho/2;x++) {

			unsigned char dato=source_bmp[y][x];

			//Separar byte en los dos pixeles
			unsigned char izq=(dato>>4)&15;
			unsigned char der=(dato&15);

			//Hacer putpixel en cada buffer para cada color
			putpixel(x*2,y,target_plane_zero,izq&1);
			putpixel(x*2,y,target_plane_one,izq&2);
			putpixel(x*2,y,target_plane_two,izq&4);
			putpixel(x*2,y,target_plane_three,izq&8);

			putpixel(x*2+1,y,target_plane_zero,der&1);
			putpixel(x*2+1,y,target_plane_one,der&2);
			putpixel(x*2+1,y,target_plane_two,der&4);
			putpixel(x*2+1,y,target_plane_three,der&8);
		}
	}


	sprintf(file_plane_dest,"%s_0",file_dest);
	write_file_plane(target_plane_zero,file_plane_dest);

	sprintf(file_plane_dest,"%s_1",file_dest);
	write_file_plane(target_plane_one,file_plane_dest);

	sprintf(file_plane_dest,"%s_2",file_dest);
	write_file_plane(target_plane_two,file_plane_dest);

	sprintf(file_plane_dest,"%s_3",file_dest);
	write_file_plane(target_plane_three,file_plane_dest);


	

return 0;

}
