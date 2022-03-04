#include <stdio.h>
#include <stdlib.h>

/*

Programa que lee un archivo de log de frecuencias de sonido y genera un archivo de audio de salida
El archivo de log lee frecuencias de 16 bits, esas frecuencias son para cada 1/10 de segundo. Archivo de log de frecuencias se puede generar
 desde el emulador
Archivo de salida es 8 bit, unsigned, mono, 15600 Hz

*/

void main (void)
{
	FILE *infile,*outfile;

	infile=fopen("pruebafreq.log","r");

	if (infile==NULL) {
		printf ("error opening infile\n");
		exit (1);
	}

        outfile=fopen("pruebafreq.rwa","w");

        if (outfile==NULL) {
                printf ("error opening outfile\n");
                exit (1);
        }


	int frecuencia_leida;
	int contador_frecuencia;
	unsigned char valor=255;

	while (!feof(infile)) {
		contador_frecuencia=0;
		unsigned char buffer[2];
		fread(buffer, 1, 2, infile);
		frecuencia_leida=buffer[0]+256*buffer[1];
	
		printf ("frecuencia leida: %d\n",frecuencia_leida);

		//Generar onda de frecuencia leida durante 1560 bytes, a 15600 hz
		
		int total;

		//Archivo de salida es 15600 Hz. Como tenemos aqui 1/10s, sera 1560 bytes
	 	for (total=0;total<1560;total++) {
			fwrite (&valor,1,1,outfile);
			contador_frecuencia +=frecuencia_leida;
			if (contador_frecuencia>=15600/2) {
				valor=valor ^ 255;
				contador_frecuencia -=15600/2;
			}
		}	

	}

	fclose(infile);
	fclose(outfile);


}
