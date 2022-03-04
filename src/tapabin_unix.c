#include <stdio.h>
#include <stdlib.h>


char *tipos_fichero[]={
  "Programa",
  "Matriz Num.",
  "Matriz Car.",
  "Bytes",
  "Flag"
};


unsigned char *memoria;

int tecla;

char fichero_bin[255],fichero_tap[255];

FILE *fic_bin,*fic_tap;

unsigned int longitud;

int da_ascii(int codigo)
{
  return (codigo<127 && codigo>31 ? codigo : '.');
}


void pon_tipo(void)
//Escribe tipo de fichero
{
  int n;
  unsigned short *p;
  unsigned char tipo;                     
                               
  if (longitud!=19) {
	 printf ("%s:%u\nLongitud: %u\n",
		tipos_fichero[4],*memoria,longitud-2);
	 return;
  }                          
  tipo=memoria[1];
  printf ("%s:",tipos_fichero[tipo]);
  for (n=0;n<10;n++) putchar(da_ascii(memoria[2+n]));
  p=(unsigned short *)&memoria[12];
  printf ("\nLongitud:%u\n",*p);
  if (tipo==3) printf ("Inicio:%u\n",p[1]);
  if (!tipo) {
	 printf ("Variables:%u\nAutorun:",*p-p[2]);
	 if (p[1]<32767) printf ("%u\n",p[1]);
	 else printf("Ninguno\n");
  }
}

void error_abrir(void)
{
  
  printf ("\nError al abrir fichero\a\n");
  exit(0);               
	 
}


int main(void)
{
  char buffer_pregunta[80];

  printf ("\nTAPABIN V1.0\n"        
			 "(c) Cesar Hernandez Bano (15/10/1998),(17/09/2013)\n\n"
			 "Extension del programa ZXSPECTR para convertir ficheros de cinta\n"
			 ".TAP a ficheros binarios .BIN\n\n");
  printf("\nIntroduce el nombre del fichero TAP:");
  scanf("%s",fichero_tap);
  if ((fic_tap=fopen(fichero_tap,"rb"))==NULL) error_abrir();
  
  printf("\nIntroduce el nombre del fichero BIN:");
  scanf("%s",fichero_bin);
  if ((fic_bin=fopen(fichero_bin,"a+b"))==NULL) error_abrir();

  //Asignar memoria
  if ((memoria=(unsigned char *)malloc(65536L))==NULL) {
	 printf ("No hay memoria\a\n");
	 exit(0);
  }

  printf ("\n");

  while (!feof(fic_tap)) {
                                          
	 //Leer longitud
	 if (!fread(&longitud,1,2,fic_tap)) break;
	 //Esto se hace asi porque la feof no detecta bien el final de fichero
	 
	 
	 fread(memoria,longitud,1,fic_tap);
	 
	 //En la longitud se incluye flag y checksum
	 pon_tipo();
	
	 printf ("\nEscriba N si no desea grabar el bloque\n\n");
	 scanf("%s",buffer_pregunta);
	 tecla=buffer_pregunta[0];
	 if (!(tecla=='n' || tecla=='N')) fwrite(memoria+1,1,longitud-2,fic_bin);
  }

  //fcloseall();
fclose(fic_bin);
fclose(fic_tap);

return 0;

}
