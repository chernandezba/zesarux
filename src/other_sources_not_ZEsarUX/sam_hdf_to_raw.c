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

void main (void)
{

	unsigned char *memoria;

	//1 GB
	memoria=malloc(1024*1024*1024);

	if (!memoria) {
		printf ("Error allocating memory\n");
		exit (1);
	}

	char *file_orig="disco.hdf";
	char *file_dest="disco2.hdf";

        FILE *ptr_inputfile;
        ptr_inputfile=fopen(file_orig,"rb");

        if (ptr_inputfile==NULL) {
                printf ("Error opening %s\n",file_orig);
                exit(1);
        }

        int leidos=fread(memoria,1,1024*1024*1024,ptr_inputfile);

	printf ("leidos: %d\n",leidos);

	fclose (ptr_inputfile);


	//Invertir bytes. empezando desde offset 128 donde empieza el disco propiamente
	int i;
	unsigned char h,l;
	for (i=128;i<leidos;i+=2) {
		h=memoria[i];
		l=memoria[i+1];
		memoria[i]=l;
		memoria[i+1]=h;
	}




	//Grabar
                        FILE *ptr_outputfile;
                        ptr_outputfile=fopen(file_dest,"wb");

                        fwrite(&memoria[128],1,leidos-128,ptr_outputfile);

                        fclose(ptr_outputfile);



}
