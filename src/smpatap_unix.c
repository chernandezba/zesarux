#include <stdio.h>
#include <stdlib.h>

#define SPEC_NO_RUIDO 		2
#define SPEC_ONDAS_GUIA 	10


char *spec_tipos_fichero[]={
  "Program",
  "Number Array",
  "Character Array",
  "Bytes",
  "Flag"
};

//int spec_ondas_leidas;

unsigned char spec_carry;

unsigned char *memoria;
unsigned char *memoria_original;
unsigned long spec_bytes_leidos;

//char fichero_smp[1024],fichero_tap[1024];
char *fichero_smp,*fichero_tap;

char spec_tono_guia=14,spec_ceros=6,spec_unos=12;

FILE *ptr_mycinta_smp,*fic_tap;

char spec_byte_cambio,spec_cambio=0;
char spec_final_fichero=0;

int spec_da_ascii(int codigo)
{
  return (codigo<127 && codigo>31 ? codigo : '.');
}

int spec_da_abs(int valor)
{
  if (valor>=0) return valor;
  else return -valor;
}

int spec_lee_byte(void)
//Funcion que lee byte del fichero
//Mira si hay un byte de cambio de onda, en cuyo caso lo devuelve
//Pone spec_final_fichero a 1 si se llega al final del fichero
{
  if (spec_cambio) {
	 spec_cambio=0;
	 return spec_byte_cambio;
  }
	 
  else {
	 spec_byte_cambio=fgetc(ptr_mycinta_smp);
  }
	                                
  if (feof(ptr_mycinta_smp)) {
	 spec_final_fichero=1;
	 return 0;
  }
	 
  return spec_byte_cambio;
}

char spec_da_signo(char valor)
//Devuelve el signo de valor: -1,+1 o 0
{

  if (valor>=0) return 1;
  if (valor<0) return -1;

}

int spec_lee_onda(unsigned char *longitud,unsigned char *amplitud)
//Funcion que lee una onda completa de sonido
//Da la maxima amplitud (en positivo) y la longitud
//de esa onda
//Devuelve -1 si se llega al final del fichero
{
  char byte,byte_anterior,veces=0;

  *longitud=1;
  *amplitud=0;

  byte_anterior=spec_lee_byte();
  *amplitud=spec_da_abs(byte_anterior);

  
  if (spec_final_fichero) return -1;
  //signo=spec_da_signo(byte_anterior);
  do {
	 byte=spec_lee_byte();
	 if (spec_final_fichero) return -1;
	 
	 if (spec_da_abs(byte)>*amplitud) *amplitud=spec_da_abs(byte);
	 if (spec_da_signo(byte)!=spec_da_signo(byte_anterior)
		  //&& spec_da_abs(byte)>=SPEC_NO_RUIDO
		  ) {
		if (veces==1) {
		  spec_cambio=1;

		  return 0;
		}
		veces++;
	 }
	 (*longitud)++;
	 byte_anterior=byte;
  } while (1);
}

int spec_dice_bit(char longitud)
//Dice si el bit es 0 o 1 segun su amplitud
//Devuelve -1 si no es un bit aceptado
{
  if (longitud>=spec_ceros-2 && longitud<=spec_ceros+2) return 0;
  if (longitud>=spec_unos-2 && longitud<=spec_unos+2) return 1;
  return -1;
}

int spec_lee_8_bits(void/*char si_longitud_anterior*/)
//Devuelve 8 bits leidos 


//No usado:
//Se puede entrar la longitud anterior leida, si no entrarlo con -1

//Devuelve -1 si se llega al final del fichero 
//Devuelve -2 si se encuentra ruido
//Devuelve -3 si se encuentran datos sin sentido
{
  unsigned char longitud,amplitud;
  char bit;
  int n,byte=0;

  for (n=0;n<8;n++) {
	 /*if (si_longitud_anterior!=-1) {
		longitud=si_longitud_anterior;
		si_longitud_anterior=-1;
		amplitud=10; //Hacer que se entre amplitud_anterior
	 }*/
	 /*else*/
	 if (spec_lee_onda(&longitud,&amplitud)==-1) return -1;
	 
	 if (amplitud<SPEC_NO_RUIDO) return -2;
	 bit=spec_dice_bit(longitud);
	 if (bit==-1) return -3;

	 byte=byte*2+bit;
  }
  return byte;
}

int spec_pon_tipo(void)
//Escribe tipo de fichero
{
  int n;
  unsigned short *p;
  unsigned char tipo;

  if (spec_bytes_leidos!=19) {
	 printf ("%s:%u\nLength: ",
		spec_tipos_fichero[4],*memoria_original);
	 if (spec_bytes_leidos>2) printf ("%lu+2\n",spec_bytes_leidos-2);
	 else printf ("%lu\n",spec_bytes_leidos);
	 return 0;
  }
  tipo=memoria_original[1];
  printf ("%s:",spec_tipos_fichero[tipo]);
  for (n=0;n<10;n++) putchar(spec_da_ascii(memoria_original[2+n]));
  p=(unsigned short *) &memoria_original[12];
  printf ("\nLength:%u\n",*p);
  if (tipo==3) printf ("Start:%u\n",p[1]);
  if (!tipo) {
	 printf ("Variables:%u\nAutorun:",*p-p[2]);
	 if (p[1]<32767) printf ("%u\n",p[1]);
	 else printf("None\n");
  }


//TODO: aqui se llega alguna vez?
return 0;
}
  
int main(int argc,char *argv[])
{

  unsigned char amplitud,longitud;
  int byte,byte2,tecla;
  unsigned int n;
  char buffer_pregunta[80];

  printf ("\nSMPATAP V1.1\n"
			 "(c) Cesar Hernandez Bano (10/09/1998),(31/03/2014)\n\n"
			 "SMP to TAP file converter\n\n");


 if (argc!=3) {
        printf ("Syntax: %s [smpfile] [tapfile]\n",argv[0]);
        printf ("smpfile must be: raw, 11111hz, 8 bit, mono, unsigned\n");
        exit(1);
  }


  fichero_smp=argv[1];
  fichero_tap=argv[2];


/*
  printf("\nIntroduce el nombre del fichero SMP:");
  scanf("%s",fichero);

  printf("\nIntroduce el nombre del fichero TAP:");
  scanf("%s",fichero_tap);
*/

  if ((ptr_mycinta_smp=fopen(fichero_smp,"rb"))==NULL) {
         printf ("\nError opening smp file\n");
         exit(0);
  }


  if ((fic_tap=fopen(fichero_tap,"a+b"))==NULL) {
	 printf ("\nError opening tap file\n");
	 exit(0);
  }

  //Asignar memoria
  if ((memoria_original=(unsigned char *)malloc(65536L))==NULL) {
	 printf ("Error allocating memory\n");
	 exit(0);
  }
  
  do {
	  spec_carry=0;

	  spec_cambio=0;
	  spec_final_fichero=0;

	  spec_bytes_leidos=0;


	 memoria=memoria_original;
  
	 //Leer unas ondas de tono guia
	 n=0;
	 do {
		if (spec_lee_onda(&longitud,&amplitud)==-1) goto fin;
		if (amplitud<SPEC_NO_RUIDO || (!(longitud>=spec_tono_guia-2 && longitud<=spec_tono_guia+2))
			 ) {
		  n=0;
		  continue;
		}
		n++;
	 } while (n<SPEC_ONDAS_GUIA);	 
	 printf ("\n\nReading pilot tone...\n");
	 do {
	  if (spec_lee_onda(&longitud,&amplitud)==-1) goto fin;
	 } while (amplitud>=SPEC_NO_RUIDO && (longitud>=spec_tono_guia-2 && 
				 longitud<=spec_tono_guia+2));
	 //Hay que saber si se esta en mitad o al final de la onda falsa
	 if (longitud>6) { //en mitad de la onda falsa
		spec_cambio=0;
		byte=spec_byte_cambio;
		do {
		  byte2=spec_lee_byte();
		  if (spec_final_fichero) goto fin;
		} while (spec_da_signo(byte)==spec_da_signo(byte2));
	 }
		
	 printf ("Reading data...\n\n");

	 
	 //Despues del tono guia viene una onda falsa, no utilizable,
	 //parecida a un bit 0
  
	 do {
		byte=spec_lee_8_bits(/*-1*/);
		//if (byte==-1) goto fin;
		if (byte==-1) break;
		if (byte<0) break;
		*memoria++=byte;
		spec_carry^=byte;
		spec_bytes_leidos++;
		//printf ("memoria: %p spec_bytes_leidos: %lu\n",memoria,spec_bytes_leidos);
	 } while (1);


	 if (spec_bytes_leidos) {
		if (!spec_pon_tipo()) {
		  n=spec_bytes_leidos;

		//antiguamente esto limitaba a 19 lineas en msdos 
		  if (n>80*19) n=80*19;
		  memoria=memoria_original;
		  printf ("\n");
		  for (;n;n--) {
			 putchar(spec_da_ascii(*memoria++));
		  }
		  printf ("\n");
		}
	 }

         if (spec_carry) {
                printf ("Loading Error. Invalid end carry\n");
                getchar();
         }


	 if (spec_bytes_leidos) {
		printf ("\nWrite N and Enter if you don't want to save the program\n");
		scanf("%s",buffer_pregunta);
		tecla=buffer_pregunta[0];
		if (!(tecla=='n' || tecla=='N')) {
		  fwrite(&spec_bytes_leidos,1,2,fic_tap);
		  fwrite(memoria_original,1,spec_bytes_leidos,fic_tap);
		  if (ferror(fic_tap)) perror("Error:");
		}
	 }
	 else if (spec_bytes_leidos) getchar();

  } while (1);
  
fin:
  //fcloseall();
  fclose(ptr_mycinta_smp);
  fclose(fic_tap);
  free(memoria_original);

return 0;
  
}  
