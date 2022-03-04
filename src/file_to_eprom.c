#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char buffer[32768];

char origen[16384];

int main(int argc,char *argv[])
{

	if (argc!=4) {
		printf ("%s file epromfile filenameoneprom\n",argv[0]);
		exit(1);	
	}

	char *nombredestino;
	nombredestino=argv[3];
	

	char *nombreeprom;
	nombreeprom=argv[2];

	char *nombreorigen;
	nombreorigen=argv[1];
	

                        FILE *ptr_readfile;
                        ptr_readfile=fopen(nombreorigen,"rb");
			int tamanyo=fread(origen,1,16384,ptr_readfile);
                        fclose(ptr_readfile);

			if (tamanyo>16384) {
				printf ("file can not be more than 16kb");
				exit(0);
			}


/*

https://cambridgez88.jira.com/wiki/display/DN/Miscellaneous+useful+information

File Card format

The following is the format used in file Eprom and Flash Cards. Unfortunately this is not the conventions used by the DOR system. However, it is sufficient for traversing through the file contents. With this information you can program an application that can retreave previously 'deleted' (overwritten) files. The format is as follows:

$0000       File entry
...         File entry

... 

...         Latest file entry
...         $FF's until
$3FC0       $00's until
$3FF7       $01
$3FF8       4 byte random id
$3FFC       size of card in banks (2=32K, 8=128K, 16=256K)
$3FFD       sub-type, $7E for 32K cards, and $7C for 128K (or larger) cards
$3FFE       'o'
$3FFF       'z' (file eprom identifier, lower case 'oz')

A file entry has the form:

1 byte      n           length of filename
1 byte      x           '/' for latest version, $00 for old version (deleted)
n-1 bytes   'xxxx'      filename
4 bytes     m           length of file (least significant byte first)
m bytes                 body of file
*/	

//Nota: Parece que el $3FF8       4 byte random id lo escribe al guardar un archivo la primera vez
//Cuando se tiene una eprom vacia, el Filer cuando la reconoce guarda los bytes del final "oz" etc... pero estos 4 bytes random no, hasta que 
//se escribe un archivo por primera vez
                        int i;

	for (i=0;i<32768;i++) buffer[i]=255;

	//nombre archivo

	int longitud_nombre;

	longitud_nombre=strlen(nombredestino);
	//sumamos 1 para la /
	longitud_nombre++;

	buffer[0]=longitud_nombre;
	buffer[1]='/';
	strcpy(&buffer[2],nombredestino);
	//buffer[2]='A';

	int p=longitud_nombre+1;

	
	buffer[p++]=tamanyo%256;
	buffer[p++]=tamanyo/256;
	buffer[p++]=0;
	buffer[p++]=0;

	//en buffer[7] archivo
	memcpy(&buffer[p],origen,tamanyo);


	for (i=0x7FC0;i<0x7FF7;i++) buffer[i]=0;


	buffer[0x7ff7]=1;


	//Bytes aleatorios
	buffer[0x7ff8]=1;
	buffer[0x7ff9]=2;
	buffer[0x7ffa]=3;
	buffer[0x7ffb]=4;

	buffer[0x7ffc]=2;
	buffer[0x7ffd]=0x7e;
	buffer[0x7ffe]='o';
	buffer[0x7fff]='z';


                        FILE *ptr_epromfile;
                        ptr_epromfile=fopen(nombreeprom,"wb");

			fwrite(buffer,1,32768,ptr_epromfile);

                        fclose(ptr_epromfile);

	return 0;

}
