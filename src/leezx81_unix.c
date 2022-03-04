/*
    leezx81_unix 
    Copyright (C) 2013 Cesar Hernandez Bano

    leezx81_unix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
Ficheros .P del emulador XTENDER:
Contienen los bytes tal cual fueron grabados en el ZX81
exceptuando los bytes del nombre, que vienen al principio, y
el byte del nombre final tiene el bit 7 alzado
*/

char caracteres[]=" ??????????\x22?$:?()<>=+-*/;,."
						"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

unsigned int fic_leido;

int sensibilidad_cambio=3,longitud_cambio=3;


//unsigned char *memoria;
//unsigned char *memoria_original;
//unsigned long bytes_leidos=0;

//char fichero[255],fichero_p[255];
char *fichero,*fichero_p;

char ceros=18,unos=33;

FILE *fic,*fic_p;
unsigned char byte_cambio;
char final_fichero=0;

int debugonda=0;

int da_codigo81(int codigo)
{
  if (codigo>127) codigo-=128;
  return (codigo<64 ? caracteres[codigo] : '?');
}

int da_abs(int valor)
{
  if (valor>=0) return valor;
  else return -valor;
}

int lee_byte(void)
//Funcion que lee byte del fichero
//Pone final_fichero a 1 si se llega al final del fichero
{

  byte_cambio=fgetc(fic);
  fic_leido++;
 //printf (" %d\n",fic_leido);
	 
  if (feof(fic)) {
	 final_fichero=1;
	 return 0;
  }
	 
  return byte_cambio;
}

int lee_onda(unsigned char *longitud)
//Funcion que lee una onda completa de sonido
//Devuelve -1 si se llega al final del fichero
//Se lee a 11111hz, 8 bit, mono, unsigned
{
  
  unsigned char veces;
  int byte,byte_ant;

  //Primero posicionarse en una onda de sonido
  //Ver si la onda cambia bruscamente (mas de 2) en mas de 2 bytes

  
  byte_ant=lee_byte();

  veces=0;

  do {
	if (debugonda) printf ("S");
	 if (final_fichero) return -1;
	 byte=lee_byte();
	 if (da_abs(byte-byte_ant)>=sensibilidad_cambio) veces++;
	 else veces=0;
	 byte_ant=byte;
  } while (veces<longitud_cambio);

  *longitud=veces+1;

  //A partir de ahora leer la longitud hasta que el cambio no sea brusco
  //durante 3 bytes normalmente (longitud_cambio)

  //valor longitud_cambio depende de frecuencia muestreo
  //valor sensibilidad_cambio depende del volumen y/o bits (8 o 16)

  //Se tiene byte
  veces=0;
  
  byte_ant=byte;
  do {
	if (debugonda) printf ("O");
	 byte=lee_byte();
	 if (final_fichero) return -1;
	 if (da_abs(byte-byte_ant)<sensibilidad_cambio) {
		veces++;
		if (veces>=longitud_cambio) break;
	 }
	 else veces=0;
	 (*longitud)++;
	 byte_ant=byte;
  } while (1);
  return 0;

}

int dice_margen(int n,int valor,int izq,int der)
//Funcion que dice si el valor n esta entre [valor-izq,valor+der]
{

  return (n>=valor-izq && n<=valor+der);

}

int dice_bit(char numero)
//Dice si el bit es 0 o 1 segun su numero de ondas
//Devuelve -1 si no es un bit aceptado
{
  if (dice_margen(numero,ceros,7,7)) {
	if (debugonda) printf ("0");
	return 0;
	}
  if (dice_margen(numero,unos,7,7)) {
	if (debugonda) printf ("1");
	return 1;
	}
  unsigned int n=(unsigned int)numero;
  printf ("Value %d not accepted\n",n);
  return -1;
  
}

int lee_1_bit(void)
//Funcion que lee 1 bit
{

  unsigned char longitud;

  if (lee_onda(&longitud)==-1) return -1;

  return dice_bit(longitud);
}	
  
int lee_8_bits(void/*char si_longitud_anterior*/)
//Devuelve 8 bits leidos 
//Devuelve -1 si se llega al final de los datos
{
  char bit;
  int n,byte=0;

  for (n=0;n<8;n++) {
	 if ((bit=lee_1_bit())==-1) return -1;
	 byte=byte*2+bit;
  }
  return byte;
}

int escribe_nombre(unsigned char *m,int leidos)
//Funcion que escribe el nombre del fichero y retorna la longitud del nombre
{
  unsigned char n;
  int l=0;

  do {
	 if (!leidos) break;
	 n=*m++;
	 leidos--;
	 l++;
	 putchar(da_codigo81(n));
  } while (n<128);


  return l;

}

void input_num(int *v)
{
  char buffer[80];
  int tecla;

  printf (" : %u\nDo you want to change the value ? (Y/N) ",*v);
  scanf("%s",buffer);
  tecla=buffer[0];
  printf  ("\n");
  if (tecla=='Y' || tecla=='y') {
	 printf ("Write the new value: ");
	 scanf("%d",v);
  }
} 


int lee_todos_bytes(unsigned char *m)
{

  if ((fic=fopen(fichero,"rb"))==NULL) {
         printf ("\nError opening file %s\n",fichero);
         exit(0);
  }
  fic_leido=0;
  final_fichero=0;


  int retorno;
  int bytes_leidos=0;
  unsigned char byte_leido;

  do {
         retorno=lee_8_bits(/*-1*/);
         if (retorno==-1) break;

	 byte_leido=retorno;

         *m++=byte_leido;
         //putchar(da_codigo81(byte_leido));
         bytes_leidos++;
  } while (1);


  fclose(fic);

 return bytes_leidos;

}
 

int main(int argc,char *argv[])
{

  //int byte;
  int tecla;
  char buffer[80];
  int bytes_leidos;
  int auto_parametros=0;

  unsigned char *buffer_memoria;
  unsigned char *buffer_memoria_orig;

  printf ("\nLEEZX81 V1.1\n"
			 "(c) Cesar Hernandez Bano (10/09/1998),(02/09/2013)\n\n");


  if (argc<3) {
	printf ("Syntax: %s [smpfile] [pfile] --automatic\n",argv[0]);
	printf ("smpfile must be: raw, 11111hz, 8 bit, mono, unsigned\n");
	exit(1);
  }


  if (argc==4) {
	if (!strcmp(argv[3],"--automatic")) {
		auto_parametros=1;
	}
	else {
		printf ("Unknown parameter %s\n",argv[3]);
		exit(1);
	}
  }

  if (argc>4) {
	printf ("Error. Too much parameters\n");
	exit(1);
  }

  fichero=argv[1];
  fichero_p=argv[2];



  //printf("\nIntroduce el nombre del fichero SMP:");
  //scanf("%s",fichero);

  //printf("\nIntroduce el nombre del fichero P:");
  //scanf("%s",fichero_p);


  if (auto_parametros==0) {

	  printf ("\nThreshold of wave change is (Note: increase the value if load error)");
	  input_num(&sensibilidad_cambio);
	  printf ("\nThe lenght of wave change is (Note: do not change if frequency is the default : 11111 hz)");
	  input_num(&longitud_cambio);
  	  printf ("\nDebug wave? (Y/N)\n");
	  scanf("%s",buffer);
	tecla=buffer[0];
	if (tecla=='y' || tecla=='Y') debugonda=1;
  }



  
  //Asignar memoria
  if ((buffer_memoria=(unsigned char *)malloc(65536L))==NULL) {
	 printf ("Error allocating memory\n");
	 exit(0);
  }

  buffer_memoria_orig=buffer_memoria;
  

  //memoria=memoria_original;
  
  printf ("Reading data...\n\n");


  if (auto_parametros==0) {
	bytes_leidos=lee_todos_bytes(buffer_memoria);
  }


  else {
	int i;
	int mejor_sensibilidad_cambio=3;
	int mejor_bytes_leidos=0;
	int mejor_fic_leido=0;

	sensibilidad_cambio=3;

	for (i=0;i<30;i++) {
		printf ("Testing with Threshold of wave change: %d\n",sensibilidad_cambio);
		bytes_leidos=lee_todos_bytes(buffer_memoria);
		printf ("Bytes read: %d\n",bytes_leidos);
		if (bytes_leidos>mejor_bytes_leidos) {
			mejor_bytes_leidos=bytes_leidos;
			mejor_sensibilidad_cambio=sensibilidad_cambio;
			mejor_fic_leido=fic_leido;
		}
	
		sensibilidad_cambio++;
	}

	printf ("Best Threshold of wave change: %d Bytes read: %d Sound Bytes read: %d\n",mejor_sensibilidad_cambio,mejor_bytes_leidos,mejor_fic_leido);

	//bytes_leidos=mejor_bytes_leidos;
	//fic_leido=mejor_fic_leido;
	//Relanzamos lectura con el mejor parametro de sensibilidad
	sensibilidad_cambio=mejor_sensibilidad_cambio;
	//printf ("sensi: %d\n",sensibilidad_cambio);
        bytes_leidos=lee_todos_bytes(buffer_memoria);
        printf ("Bytes read: %d\n",bytes_leidos);


  }
	
  
  if (bytes_leidos) {

	 int i;
	 printf ("Bytes read:\n");
	 for (i=0;i<bytes_leidos;i++) {
		putchar(da_codigo81(buffer_memoria[i]));
	 }

	 printf ("\nProgram name:");
	 int longitud_nombre=escribe_nombre(buffer_memoria,bytes_leidos);

	//Descartar nombre
	 bytes_leidos -=longitud_nombre;
	 buffer_memoria +=longitud_nombre;

	 printf ("\nSound Bytes read: %u\nProgram length (without the name):%u\n",
				fic_leido,bytes_leidos);
	 if (bytes_leidos) {
		printf ("\nWrite N and Enter if you don't want to save the program\n");
		scanf("%s",buffer);
		tecla=buffer[0];
		if (!(tecla=='n' || tecla=='N')) {

			printf ("Saving program...\n");

			if ((fic_p=fopen(fichero_p,"a+b"))==NULL) {
				printf ("\nError opening file %s\n",fichero_p);
				exit(0);
			}      


		  fwrite(buffer_memoria,1,bytes_leidos,fic_p);
		  fclose(fic_p);

			printf ("End saving\n");
		  //if (ferror(fic_p)) perror("Error:");
		}
	 }
  }
  if (!bytes_leidos) printf ("\nError: Program length is zero\n");

  free(buffer_memoria_orig);

return 0;
  
}  

