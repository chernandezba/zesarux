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

//Este programa para convertir un charset de 6 bytes por caracter en uno de 8
//Usado por ejemplo para convertir el charset Tom Thumb

char file_orig[PATH_MAX];



unsigned char charset[576];


int main(int argc,char *argv[])
{


	int ancho,alto;

	if (argc!=2) {
		printf ("%s input\n",argv[0]);
		exit(1);
	}

	sprintf (file_orig,"%s",argv[1]);


	printf ("Converting file %s\n",file_orig);



	//Leemos todo el bmp en array de sprites
        FILE *ptr_inputfile;
        ptr_inputfile=fopen(file_orig,"rb");

        if (ptr_inputfile==NULL) {
		printf ("Error opening %s\n",file_orig);
		exit(1);
        }

	fread(charset,1,576,ptr_inputfile);

	int i;

    for (i=0;i<576;i++) {
        printf("0x%02X,",charset[i]);
        if ((i%6)==5) {
            printf("0x00,0x00,\n");
        }
    }



return 0;

}
