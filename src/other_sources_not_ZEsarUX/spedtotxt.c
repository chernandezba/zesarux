#include <stdio.h>
#include <stdlib.h>
 
//Sped file format to txt converter

int main(int argc, char *argv[]) {
	if (argc<2) {
		printf ("%s file\n",argv[0]);
		exit(1);
        }


	char *filename=argv[1];

	FILE *ptr_file;

	ptr_file=fopen(filename,"rb");

	if (ptr_file==NULL) {
		printf ("Error opening file\n");
		exit(1);
	}

	int x=0;

	while (!feof(ptr_file)) {
		unsigned char caracter;
		fread(&caracter,1,1,ptr_file);

		if (caracter==13) {
			caracter=10;
		}

		else if (caracter==127) {
			//(C)
			caracter='c';
		}

		else if (caracter>=128) {
			caracter -=128;	
			int tabcolumn;
			if (x<7) tabcolumn=7;
			else tabcolumn=12;

			while (x<tabcolumn) {
				printf (" ");
				x++;
			}

		}

		printf ("%c",caracter);
		x++;
		if (caracter==10) x=0;
			 
	}


	fclose(ptr_file);

	return 0;

}
