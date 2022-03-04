#include <stdio.h>

int main (int argc,char *argv[]) {
        FILE *ptr_file;
        ptr_file=fopen(argv[1],"rb");


        if (!ptr_file) {
                printf("Unable to open file %s\n",argv[1]);
                return 1;
        }

	while (!feof(ptr_file)) {
		unsigned char byte_leido;
	        int leidos=fread(&byte_leido,1,1,ptr_file);
		if (leidos>0) printf ("%02X",byte_leido);
	}

        fclose(ptr_file);

        return 0;
}

