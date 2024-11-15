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

int progress_bar_seq=0;

char progress_bar_string[]="|/-\\";

void put_progress_bar(void)
{
	printf ("%c",progress_bar_string[progress_bar_seq/100]);
	fflush(stdout);

	progress_bar_seq++;
	if (progress_bar_seq==400) progress_bar_seq=0;
}

int main (void)
{

	int leidos;

	unsigned char buffer_lectura[1024];

	char *file_orig="input.hdf";
	char *file_dest="output.ide";

        FILE *ptr_inputfile;
        ptr_inputfile=fopen(file_orig,"rb");

        if (ptr_inputfile==NULL) {
                printf ("Error opening %s\n",file_orig);
                exit(1);
        }

	FILE *ptr_outputfile;
	ptr_outputfile=fopen(file_dest,"wb");

        if (ptr_outputfile==NULL) {
                printf ("Error opening %s\n",file_dest);
                exit(1);
        }



	// Leer offset a datos raw del byte de cabecera:
	//0x09 DOFS WORD Image data offset This is the absolute offset in the HDF file where the actual hard-disk data dump starts.
	//In HDF version 1.1 this is 0x216.

	//Leemos 10 bytes de la cabecera
        fread(buffer_lectura,1,10,ptr_inputfile);

	int offset_raw=buffer_lectura[9]+256*buffer_lectura[10];

	printf ("Offset to raw data: %d\n",offset_raw);


	//Ya hemos leido 10 bytes del principio
	int saltar_bytes=offset_raw-10;

	//Saltar esos bytes
	fread(buffer_lectura,1,saltar_bytes,ptr_inputfile);

	//Y vamos leyendo bloques de 1024
	int escritos=0;

	do {
	        leidos=fread(buffer_lectura,1,1024,ptr_inputfile);
		if (leidos>0) {
			fwrite(buffer_lectura,1,leidos,ptr_outputfile);
			escritos +=leidos;
			printf ("Writing data %dKB ",escritos/1024);
			put_progress_bar();
			printf ("\r");
		}
	} while (leidos>0);

	fclose (ptr_inputfile);

                        fclose(ptr_outputfile);

	return 0;

}
