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




int main(int argc,char *argv[])
{


	int ancho,alto;

	if (argc!=4) {
		printf ("%s input ancho alto\n",argv[0]);
		exit(1);
	}

	sprintf (file_orig,"%s",argv[1]);

	ancho=atoi(argv[2]);  
	alto=atoi(argv[3]);



	//printf (";Converting file %s with size %dX%d\n",file_orig,ancho,alto);



        FILE *ptr_inputfile;
        ptr_inputfile=fopen(file_orig,"rb");

        if (ptr_inputfile==NULL) {
		printf ("Error opening %s\n",file_orig);
		exit(1);
        }

//Escribir ancho, alto
printf ("%d,%d,\n",ancho,alto);

  int leidos;
  int tamanyo=ancho*alto;
  int salto_linea=0;
  while (tamanyo) {
    unsigned char byte_leido;
    leidos=fread(&byte_leido,1,1,ptr_inputfile);
    printf ("0x%02X",byte_leido);
    if (tamanyo!=1) printf(",");
    salto_linea++;
    if (salto_linea==ancho) {
      printf ("\n");
      salto_linea=0;
    }
    tamanyo--;
  };

	fclose(ptr_inputfile);




return 0;

}
