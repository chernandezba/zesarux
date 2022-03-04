/*
	 
Cabecera ficheros SP:             

Offset    Longitud    Descripci¢n
------   ----------  -------------------
  0       2 bytes    "SP" (53h, 50h) Signatura.
  2       1 palabra  Longitud del programa en bytes (el emulador actualmente
							s¢lo genera programas de 49152 bytes)
  4       1 palabra  Posici¢n inicial del programa (el emulador actualmente
							s¢lo genera programas que comiencen en la pos. 16384)
  6       1 palabra  Registro BC del Z80
  8       1 palabra  Registro DE del Z80
 10       1 palabra  Registro HL del Z80
 12       1 palabra  Registro AF del Z80
 14       1 palabra  Registro IX del Z80
 16       1 palabra  Registro IY del Z80
 18       1 palabra  Registro BC' del Z80
 20       1 palabra  Registro DE' del Z80
 22       1 palabra  Registro HL' del Z80
 24       1 palabra  Registro AF' del Z80
 26       1 byte     Registro R (de refresco) del Z80
 27       1 byte     Registro I (de interrupciones) del Z80
 28       1 palabra  Registro SP del Z80
 30       1 palabra  Registro PC del Z80
 32       1 palabra  Reservada para uso futuro, siempre 0
 34       1 byte     Color del borde al comenzar
 35       1 byte     Reservado para uso futuro, siempre 0
 36       1 palabra  Palabra de estado codificada por bits. Formato:

							Bit     Descripci¢n
							---     -----------
							15-8    Reservados para uso futuro
							7-6     Reservados para uso interno, siempre 0
							5       Estado del Flash: 0 - tinta INK, papel PAPER
															  1 - tinta PAPER, papel INK
							4       Interrupci¢n pendiente de ejecutarse
							3       Reservado para uso futuro
							2       Biestable IFF2 (uso interno)
							1       Modo de interrupci¢n: 0=IM1; 1=IM2
							0       Biestable IFF1 (estado de interrupci¢n):
											0 - Interrupciones desactivadas (DI)
											1 - Interrupciones activadas (EI)

Cabecera ficheros Z80:

	 The old .Z80 snapshot format (for version 1.45 and below) looks like
	 this:

		  Byte    Length  Description
		  0       1       A register
		  1       1       F register
		  2       2       BC register pair (LSB, i.e.  C, first)
        4       2       HL register pair
        6       2       Program counter
        8       2       Stack pointer
		  10      1       Interrupt register
        11      1       Refresh register (Bit 7 is not significant!)
        12      1       Bit 0  : Bit 7 of the R-register
                        Bit 1-3: Border colour
                        Bit 4  : 1=Basic SamRom switched in
                        Bit 5  : 1=Block of data is compressed
                        Bit 6-7: No meaning
        13      2       DE register pair
        15      2       BC' register pair
        17      2       DE' register pair
        19      2       HL' register pair
        21      1       A' register
        22      1       F' register
        23      2       IY register (Again LSB first)
		  25      2       IX register
		  27      1       Interrupt flipflop, 0=DI, otherwise EI
		  28      1       IFF2 (not particularly important...)
        29      1       Bit 0-1: Interrupt mode (0, 1 or 2)
								Bit 2  : 1=Issue 2 emulation
                        Bit 3  : 1=Double interrupt frequency
                        Bit 4-5: 1=High video synchronisation
                                 3=Low video synchronisation
                                 0,2=Normal
								Bit 6-7: 0=Cursor/Protek/AGF joystick
                                 1=Kempston joystick
                                 2=Sinclair 1 joystick
                                 3=Sinclair 2 joystick

    Because of compatibility, if byte 12 is 255, it has to be regarded as
    being 1.  After this header block of 30 bytes the 48K bytes of Spectrum
    memory follows in a compressed format (if bit 5 of byte 12 is one). The
    compression method is very simple: it replaces repetitions of at least
    five equal bytes by a four-byte code ED ED xx yy, which stands for "byte
    yy repeated xx times".  Only sequences of length at least 5 are coded.
    The exception is sequences consisting of ED's; if they are encountered,
    even two ED's are encoded into ED ED 02 ED.  Finally, every byte
    directly following a single ED is not taken into a block, for example
	 ED 6*00 is not encoded into ED ED ED 06 00 but into ED 00 ED ED 05 00.
	 The block is terminated by an end marker, 00 ED ED 00.

    That's the format of .Z80 files as used by versions up to 1.45.  Since
	 version 2.01 emulates the Spectrum 128 too, there was a need for a new
    format.

    The first 30 bytes are almost the same as the old versions' header.  Of
    the flag byte, bit 4 and 5 have got no meaning anymore, and the program
	 counter (bytes 6 and 7) are zero to signal a version 2.01 .Z80 file.  So
    loading a new style .Z80 file into an old emulator will cause an error
    or a reset at the most.

    After the first 30 bytes, an additional header follows:

        Byte    Length  Description
        30      2       Length of additional header block (contains 23)
        32      2       Program counter
        34      1       Hardware mode: 0=Spectrum 48K, 1=0+interface I,
                        2=SamRam, 3=Spectrum 128K, 4=3+interface I.
        35      1       If in SamRam mode, bitwise state of 74ls259.
                        For example, bit 6=1 after an OUT 31,13 (=2*6+1)
                        If in 128 mode, contains last OUT to 7ffd
        36      1       Contains 0FF if Interface I rom paged
		  37      1       Bit 0: 1 if R register emulation on
								Bit 1: 1 if LDIR emulation on
        38      1       Last OUT to fffd (soundchip register number)
		  39      16      Contents of the sound chip registers

    Hereafter a number of memory blocks follow, each containing the
    compressed data of a 16K block.  The compression is according to the old
    scheme, except for the end-marker, which is now absent.  The structure
	 of a memory block is:

        Byte    Length  Description
        0       2       Length of data (without this 3-byte header)
        2       1       Page number of block
        3       [0]     Compressed data

    The pages are numbered, depending on the hardware mode, in the following
    way:

        Page    In '48 mode     In '128 mode    In SamRam mode

         0      48K rom         rom (basic)     48K rom
         1      Interf. I rom   Interf. I rom   Interf. I rom
         2      -               rom (reset)     samram rom (basic)
			3      -               page 0          samram rom (monitor,..)
			4      8000-bfff       page 1          Normal 8000-bfff
         5      c000-ffff       page 2          Normal c000-ffff
			6      -               page 3          Shadow 8000-bfff
         7      -               page 4          Shadow c000-ffff
         8      4000-7fff       page 5          4000-7fff
         9      -               page 6          -
        10      -               page 7          -

    In 48K mode, pages 4,5 and 8 are saved.  In SamRam mode, pages 4 to 8
    are saved.  In 128 mode, all pages from 3 to 10 are saved.  This
    version saves the pages in numerical order.  There is no end marker.
    end marker.




*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM_CACHE 10000
#define MAXPATH 256

//typedef unsigned int miuint;
typedef unsigned short miuint;
typedef unsigned char uchar;

struct SP {
  char signatura[2];
  miuint longitud;
  miuint posicion_inicial;
  miuint BC;
  miuint DE;
  miuint HL;
  miuint AF;
  miuint IX;
  miuint IY;
  miuint BC_;
  miuint DE_;
  miuint HL_;
  miuint AF_;
  uchar R;
  uchar I;
  miuint SP;
  miuint PC;
  miuint reservada;
  uchar borde;
  uchar reservado;
  struct {
	 unsigned IFF1:1;
	 unsigned modo_int:1;
	 unsigned IFF2:1;
	 unsigned reserv:1;
    unsigned interrupcion_pendiente:1;
	 unsigned flash:1;
	 unsigned reservados:2; 
  } flags;
  uchar reservados;
};

struct Z80 {
  uchar A;
  uchar F;
  miuint BC;
  miuint HL;
  miuint PC_old;
  miuint SP;
  uchar I;
  uchar R;
  struct {
	 unsigned R7:1;
	 unsigned borde:3;
	 unsigned sisamrom:1;
	 unsigned comprimido:1;
	 unsigned reservado1:2;
  } flags1;
  miuint DE;
  miuint BC_;
  miuint DE_;
  miuint HL_;
  uchar A_;
  uchar F_;
  miuint IY;
  miuint IX;
  uchar IFF1;
  uchar IFF2;
  struct {
	 unsigned modo_int:2;
	 unsigned issue2:1;
	 unsigned doble_interrupt:1;
	 unsigned video:2;
	 unsigned joystick:2;
  } flags2;
};

struct Z802 {
  miuint longit_cabecera;
  miuint PC;
  uchar hardware;
  uchar nada1;
  uchar nada2;
  uchar RLDIR;
  uchar FFFD;
  uchar chip[16];
};


struct SP sp;
struct Z80 z80;
struct Z802 z802;
FILE *input;
FILE *output;
FILE *temp8,*temp4,*temp5;

struct _cache {
  FILE *fichero;
  char *memoria;
  miuint posicion;
} cache[5];

int caches=0;

char *ftemporales[]={
  "8.tmp",
  "4.tmp",
  "5.tmp"
};

int dasegmento(int numero)
// Seg£n numero (8,4 o 5) devuelve segmento 0,1 o 2
{
  if (numero==8) return 0;
  return numero-3;
} 


FILE **dapuntero(int segmento)
//Da puntero a &temp8,&temp4,o &temp5 seg£n si segmento es 0, 1 o 2
{
  if (!segmento) return &temp8;
  if (segmento==1) return &temp4;
  return &temp5;
}


void errordisco(void)
{
  printf ("\nError de disco!\a\n");
  exit(0);
}

void fabre(char *f,char *m,FILE **p)
{
  if ((*p=fopen(f,m))==NULL) errordisco();
}


void pon_porcentaje(unsigned long n)    
{                                                 
  if (!(n%1024)) printf ("Convertido en %lu%%\r",n*100/49152); 
}

void setcache(FILE *f,uchar modo) //w para escritura, r para lectura
{
  int n;

  for (n=0;n<5 && cache[n].memoria!=NULL;n++);

  if ((cache[n].memoria=malloc(TAM_CACHE))==NULL) {
	 printf("\nMemoria insuficiente\n\a");
	 exit(0);
  }
  cache[n].fichero=f;
  if (modo=='r') cache[n].posicion=TAM_CACHE;
  else cache[n].posicion=0;
}

struct _cache *dacache(FILE *f)
{
  int n;

  for (n=0;n<=5 && cache[n].fichero!=f;n++);
  return &cache[n];
}

int fdarc(FILE *f)
{
  struct _cache *c;

  c=dacache(f);
  if (c->posicion==TAM_CACHE) {
	 fread(c->memoria,TAM_CACHE,1,f);
	 c->posicion=0;
  }
  return c->memoria[c->posicion++];
}

void fponc(uchar car,FILE *f)
{
  struct _cache *c;

  c=dacache(f);
  if (c->posicion==TAM_CACHE) {
	 fwrite(c->memoria,TAM_CACHE,1,f);
	 c->posicion=0;
  }
  c->memoria[c->posicion++]=car;
}

void fponc2(uchar c,int segmento)
{
  fponc(c,*dapuntero(segmento));
}

void libera_cache(FILE *f)
{
  struct _cache *c;

  c=dacache(f);
  free(c->memoria);
  c->memoria=NULL;

}

void fleer(char *p,int bytes,FILE *f)
{
  for (;bytes;bytes--)
	 *p++=fdarc(f);
}

void fescr(char *p,int bytes,FILE *f)
{
  for (;bytes;bytes--)
	 fponc(*p++,f);
}

void setcachetemp(int segmento,uchar c)
{
  if (!segmento) setcache(temp8,c);            
  else if (segmento==1) setcache(temp4,c);
  else setcache(temp5,c);     
}

void escribe_cache(FILE *f)
{
  struct _cache *c;

  c=dacache(f);

  fwrite(c->memoria,c->posicion,1,f);   
  libera_cache(f);
}

/*
void strupr(char *string)
{
  char letra;
  while (*string) {
	letra=*string;
	if (letra>='a' && letra <='z') letra = letra - ('a' - 'A');
	*string=letra;

	string++;
  }
}
*/

int main(void)
{

  //miuint n,m,segmento;
  unsigned int n,m,segmento;

  uchar byte,byte_anterior,cuenta;       
  char origen[MAXPATH];
  char destino[MAXPATH];
  unsigned long bytes_escritos;
  char error_conv=0; //Se pone a 1 si no se puede convertir (Z80 de 128k)
  int dest=0; //0 si SP a Z80, 1 si Z80 a SP

  printf ("SP_Z80 V1.1\n"
			 "(c) Cesar Hernandez Bano  (17/11/1998),(17/09/2013)\n\n"
			 "Extension del programa ZXSPECTR para convertir "
			 "ficheros SP y Z80\n\n");
			 
  printf ("Introduce fichero origen:");
  gets(origen);

  printf ("\nIntroduce fichero destino:");
  gets(destino);

  //strupr(origen);

  if (strstr(origen,".Z80")!=NULL || strstr(origen,".z80")!=NULL) dest=1;
  printf ("\nConvirtiendo a formato %s\n",(dest ? "SP" : "Z80"));
  fabre(origen,"rb",&input);
  fabre(destino,"wb",&output);
  setcache(input,'r');
  setcache(output,'w');
  if (dest) {
	 fleer ((char * )&z80,sizeof(struct Z80),input);
	 if (!z80.PC_old) fleer((char * )&z802,sizeof(struct Z802),input);
  }
  else fleer((char * )&sp,sizeof(struct SP),input);
  if (dest) {
	 if (!z80.PC_old && z802.hardware) {
		printf ("\nSolo se pueden convertir ficheros de 48K\n\n\a");
		error_conv=1;
	 }
	 else {
		sp.signatura[0]='S';
		sp.signatura[1]='P';
		sp.longitud=49152;
		sp.posicion_inicial=16384;
		sp.BC=z80.BC;
		sp.DE=z80.DE;
		sp.HL=z80.HL;
		sp.BC_=z80.BC_;
		sp.DE_=z80.DE_;
		sp.HL_=z80.HL_;
		sp.AF=z80.F+((miuint)z80.A << 8);
		sp.AF_=z80.F_+((miuint)z80.A_ << 8);
		sp.IX=z80.IX;
		sp.IY=z80.IY;
		if (!z80.PC_old) sp.PC=z802.PC;
		else sp.PC=z80.PC_old;
		sp.SP=z80.SP;
		sp.I=z80.I;
		sp.R=z80.R+(z80.flags1.R7 << 7);
		sp.borde=z80.flags1.borde;
		sp.flags.IFF1=(z80.IFF1!=0);
		sp.flags.IFF2=(z80.IFF2!=0);
		sp.flags.modo_int=(z80.flags2.modo_int==2);
		fescr((char * )&sp,sizeof(struct SP),output);
		bytes_escritos=0;
		for (m=0;m<3;m++) {
		  segmento=m;
		  if (!z80.PC_old) { 
			 fdarc(input);
			 fdarc(input);
			 segmento=dasegmento(fdarc(input));
		  }
		  byte_anterior=0;
		  cuenta=0;
		  fabre(ftemporales[segmento],"wb",dapuntero(segmento));
		  setcachetemp(segmento,'w');
		  for (n=0;n<16384;n++) {
			 pon_porcentaje((bytes_escritos++)/2);
			 if (!z80.PC_old || z80.flags1.comprimido) {
				if (cuenta) {
				  fponc2(byte_anterior,segmento);
				  cuenta--;
				}                    
				else {
				  byte=fdarc(input);
				  if (byte!=237) {
					 if (byte_anterior==237) fponc2(byte_anterior,segmento);
					 fponc2(byte,segmento);
					 byte_anterior=0;
				  }
				  else if (byte_anterior!=237) byte_anterior=237;
				  else {
					 cuenta=fdarc(input);
					 byte_anterior=fdarc(input);
					 n-=2;
				  }
				}
			 }
			 else {
				byte=fdarc(input);
				fponc2(byte,segmento);
			 }
		  }
		}
		for (m=0;m<3;m++) {
		  escribe_cache(*dapuntero(m));
		  fclose (*dapuntero(m));
		  fabre(ftemporales[m],"rb",dapuntero(m));
		  setcachetemp(m,'r');
		  for (n=0;n<16384;n++) {
			 byte=fdarc(*dapuntero(m));
			 fponc(byte,output);
			 pon_porcentaje((bytes_escritos++)/2);
		  }
		}
		for (n=0;n<3;n++)	{
		  libera_cache(*dapuntero(n));
		  fclose(*dapuntero(n)); 
		  remove(ftemporales[n]);
		}
	 }
  }
  else {
	 z80.BC=sp.BC;
	 z80.DE=sp.DE;
	 z80.HL=sp.HL;
	 z80.BC_=sp.BC_;
	 z80.DE_=sp.DE_;
	 z80.HL_=sp.HL_;
	 z80.A=(sp.AF >> 8);
	 z80.F=(uchar) sp.AF;
	 z80.A_=(sp.AF_ >> 8);
	 z80.F_=(uchar) sp.AF_;
	 z80.IX=sp.IX;
	 z80.IY=sp.IY;
	 z80.PC_old=sp.PC;
	 z80.SP=sp.SP;
	 z80.I=sp.I;
	 z80.R=sp.R;
	 z80.flags1.borde=sp.borde;
	 z80.flags1.R7=(sp.R>>7)&1;
	 z80.IFF1=sp.flags.IFF1;
	 z80.IFF2=sp.flags.IFF2;
	 z80.flags2.modo_int=sp.flags.modo_int+1;
	 fescr((char * )&z80,sizeof(struct Z80),output);
	 for (n=0;n<49152;n++) {
		pon_porcentaje(n);
		byte=fdarc(input);
		fponc(byte,output);
	 }

  }
	 
  if (!error_conv)  escribe_cache(output);
  fclose(input);
  fclose (output);
  libera_cache(input);
  if (!error_conv) {
	 pon_porcentaje(49152);
	 printf  ("\n\nConversion concluida\n\n");
  }

return 0;

}	 
