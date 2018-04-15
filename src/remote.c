/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
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
#include <fcntl.h>


#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "remote.h"
#include "compileoptions.h"
#include "ql.h"
#include "disassemble.h"
#include "menu.h"
#include "audio.h"
#include "timer.h"
#include "screen.h"
#include "ay38912.h"
#include "charset.h"
#include "diviface.h"
#include "ula.h"
#include "superupgrade.h"
#include "tbblue.h"
#include "textspeech.h"
#include "tsconf.h"
#include "operaciones.h"
#include "snap.h"
#include "kartusho.h"
#include "zxevo.h"



int remote_protocol_port=DEFAULT_REMOTE_PROTOCOL_PORT;
z80_bit remote_protocol_enabled={0};
z80_bit remote_ack_enter_cpu_step={0};


//Si se llama a end_emulator desde aqui
z80_bit remote_calling_end_emulator={0};

//Opciones al hacer debug, en este caso, al ejecutar comandos paso a paso
//Ver ayuda de comando set-debug-settings para entender significado
int remote_debug_settings=1;

#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>

#ifdef MINGW
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

#include <stdarg.h>

struct sockaddr_in adr;
unsigned int long_adr;
int sock_listen,sock_conectat;

int remote_salir_conexion;

z80_bit remote_protocol_ended={0};

//Usados en la carga de archivo de codigo fuente. Archivo crudo original
char *remote_raw_source_code_pointer=NULL;
int remote_tamanyo_archivo_raw_source_code=0;
//Puntero a indices en archivo raw source
int *remote_raw_source_code_indexes_pointer=NULL;
//Tamanyo de ese array
int remote_raw_source_code_indexes_total;

//Puntero a indices en archivo parsed source (lineas sin comentarios, con codigo real)
int *remote_parsed_source_code_indexes_pointer=NULL;
//Tamanyo de ese array
int remote_parsed_source_code_indexes_total;


//Si se envia CR despues de cada sentencia de escritura
int enviar_cr=0;


int remote_find_label_source_code(char *label_to_find);


//Crea un socket TCP per la connexio en xarxa
int crear_socket_TCP(void)
{
	#ifdef MINGW
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(1,1), &wsadata) == SOCKET_ERROR) {
		debug_printf(VERBOSE_ERR,"Error creating socket.");
		return -1;
	}
	#endif


        return socket(PF_INET,SOCK_STREAM,0);
}


int omplir_adr_internet
(struct sockaddr_in *adr,char *host,unsigned short n_port)
{
        struct hostent *h;

        adr->sin_family=AF_INET;
        if (host!=NULL) {
                if ((h=gethostbyname(host))==NULL) return -1;
                adr->sin_addr=*(struct in_addr *)h->h_addr;

                //printf ("\nncdd_util: %s : %d = %lX\n",host,(int)n_port,(unsigned long)adr->sin_addr.s_addr);

        }
        else {
                adr->sin_addr.s_addr=htonl(INADDR_ANY);
  }
        adr->sin_port=htons(n_port);
        return 0;
}

//Assignar l'adre<E7>a al socket
//Host ha de valer NULL si es tracta del servidor
int assignar_adr_internet
(int sock,char *host,unsigned short n_port)
{
        struct sockaddr_in adr;

        if (omplir_adr_internet(&adr,host,n_port)<0) return -1;
        return bind(sock,(struct sockaddr *)&adr,sizeof(adr));
}

pthread_t thread_remote_protocol;

//Si el thread se ha inicializado correctamente
z80_bit thread_remote_inicializado={0};


int escribir_socket(int socket, char *buffer)
{

char cr=13;

int efectivo_enviar_cr=0;

if (enviar_cr) {
	//Si ultimo caracter es 10, agregar un 13
	int longitud=strlen(buffer);
	if (longitud) {
		if (buffer[longitud-1]==10) efectivo_enviar_cr=1;
	}
}

#ifdef MINGW

int smsg=send(socket,buffer,strlen(buffer),0);
	 if(smsg==SOCKET_ERROR){
			 debug_printf(VERBOSE_ERR,"Error writing to socket");
			 return -1;
	 }
	 if (efectivo_enviar_cr) send(socket,&cr,1,0);
	 return smsg;

#else

	int escrito=write(socket,buffer,strlen(buffer));

	if (efectivo_enviar_cr) write(socket,&cr,1);

	return escrito;

#endif

}

//int leidos = read(sock_conectat, buffer_lectura_socket, 1023);
int leer_socket(int s, char *buffer, int longitud)
{
#ifdef MINGW

int leidos=recv(s,buffer,longitud,0);
 if(leidos==SOCKET_ERROR){
	 	debug_printf(VERBOSE_ERR,"Error reading from socket");
		return -1;
 }
 return leidos;

#else
	//int leidos=read(s, buffer, longitud);
	int leidos=recv(s,buffer,longitud,0);
	//printf ("leidos en leer_socket: %d\n",leidos);
	return leidos;

#endif
}


void escribir_socket_format (int misocket, const char * format , ...)
{

    char buffer_final[4096];

    va_list args;
    va_start (args, format);
    vsprintf (buffer_final,format, args);
    va_end (args);

    escribir_socket(misocket,buffer_final);
}



#define EASTER_EGG_TOTAL_SPRITES 13

//Sprite GScape
z80_byte easter_egg_sprite_0[]={
2,26,
0x03,0x80,
0x05,0xC0,
0x07,0x80,
0x04,0x40,
0x03,0x80,
0x0D,0x40,
0x1E,0xE0,
0x1E,0xE0,
0x37,0x40,
0x35,0x40,
0x37,0x40,
0x37,0x40,
0x1A,0x20,
0x19,0xC0,
0x05,0xC0,
0x0D,0x40,
0x13,0x60,
0x1E,0xE0,
0x1E,0xE0,
0x1C,0xC0,
0x3D,0xC0,
0x39,0x80,
0x31,0x80,
0x50,0x40,
0x60,0xE0,
0x30,0x00
};


//Sprite Toi
z80_byte easter_egg_sprite_1[]={
3,32,
0x00,0x7E,0x00,
0x0E,0x81,0x60,
0x11,0x00,0x90,
0x2E,0x81,0x68,
0x2C,0x80,0x34,
0x4C,0x00,0x12,
0x48,0x00,0x12,
0x38,0x00,0x0C,
0x10,0x00,0x08,
0x11,0x86,0x08,
0x13,0xCF,0x04,
0x13,0xCF,0x04,
0x23,0xDF,0x8A,
0x26,0xF7,0x85,
0x24,0x63,0x85,
0x54,0x63,0x89,
0xA4,0x63,0x89,
0xA6,0xF7,0x06,
0x93,0x9E,0x04,
0x90,0x00,0x04,
0x70,0x00,0x08,
0x08,0x01,0x08,
0x09,0x02,0x10,
0x08,0xFC,0x10,
0x14,0x00,0x20,
0x12,0x00,0x50,
0x0B,0x01,0x88,
0x11,0xFF,0x08,
0x10,0x80,0x90,
0x10,0x80,0x60,
0x09,0x00,0x00,
0x06,0x00,0x00
};


//Sprite Sabre
z80_byte easter_egg_sprite_2[]={
6,40,
0x00,0x00,0x01,0x80,0x00,0x00,
0x00,0x00,0x07,0xE0,0x00,0x00,
0x00,0x00,0x1D,0xD8,0x00,0x00,
0x00,0x00,0x7E,0xC6,0x00,0x00,
0x00,0x01,0xBF,0x71,0x80,0x00,
0x00,0x06,0x7B,0xD1,0xE0,0x00,
0x00,0x1D,0x3D,0x77,0x18,0x00,
0x00,0x73,0x8A,0xCC,0x16,0x00,
0x01,0xF7,0xE0,0x00,0x31,0x80,
0x07,0xF9,0xBF,0xD9,0x60,0x60,
0x0D,0x5C,0x7C,0x57,0xC0,0x50,
0x1C,0xBF,0x03,0x57,0x00,0x48,
0x39,0x7E,0xE0,0x02,0x00,0x48,
0x34,0xFD,0x6B,0x6A,0x40,0x84,
0x28,0x7A,0xAE,0xEA,0x02,0x84,
0x33,0x1D,0xB7,0x69,0x27,0x04,
0x17,0x87,0x76,0xD5,0x3E,0x08,
0x1F,0xE0,0x7B,0xD4,0xF8,0x08,
0x0F,0xD4,0x00,0x00,0xC8,0x10,
0x07,0xB7,0xC0,0x02,0x40,0x20,
0x0B,0xEE,0xAB,0xD2,0x44,0xC0,
0x08,0xFB,0x6F,0x52,0x27,0x10,
0x0E,0x1F,0xFB,0x51,0xF8,0x10,
0x0A,0xC0,0xFF,0xFF,0x03,0x50,
0x0A,0xEF,0x00,0x00,0xF7,0x50,
0x0B,0xC7,0x00,0x00,0xF7,0x50,
0x0A,0xFF,0x00,0x00,0xE5,0x50,
0x0E,0xEF,0x00,0x00,0xF7,0x50,
0x0B,0xEF,0x00,0x00,0xE1,0x50,
0x0A,0xEB,0x00,0x00,0xE7,0x50,
0x0F,0xDD,0x00,0x00,0xF1,0x50,
0x0A,0xEF,0x00,0x00,0xF7,0x50,
0x0E,0xDF,0x00,0x00,0xF5,0x50,
0x0A,0xED,0x00,0x00,0xE3,0x50,
0x0B,0xEF,0x00,0x00,0xF7,0x70,
0x0E,0xEF,0x00,0x00,0xE7,0x60,
0x02,0xFF,0x00,0x00,0xF3,0x40,
0x01,0xFA,0x00,0x00,0xF5,0xC0,
0x00,0x6F,0x00,0x00,0xF6,0x00,
0x00,0x06,0x00,0x00,0x60,0x00
};


//Sprite WorldDes
z80_byte easter_egg_sprite_3[]={
6,33,
0x00,0x00,0x00,0x00,0x03,0xFE,
0x00,0x00,0x00,0x00,0x0F,0xFF,
0x00,0x00,0x00,0x00,0x2A,0xAB,
0x00,0x00,0x00,0x00,0xD5,0x57,
0x00,0x00,0x00,0x00,0xFF,0xFB,
0x00,0x00,0x00,0x00,0xFF,0xF7,
0x00,0x00,0x00,0x00,0x24,0x12,
0x00,0x00,0x00,0x01,0x7E,0xBF,
0x00,0x00,0x00,0x00,0x63,0x71,
0x00,0x00,0x00,0x01,0x7E,0xBF,
0x00,0x00,0x00,0x0F,0x7F,0x7F,
0x00,0x00,0x00,0x31,0x7E,0xBF,
0x00,0x00,0x00,0xC1,0x00,0x00,
0x00,0x00,0x01,0x01,0x77,0x77,
0x7F,0xFF,0xFF,0x83,0xAA,0xAA,
0xDF,0xE0,0x01,0xFF,0xDD,0xDD,
0xEF,0xFF,0xFF,0xFF,0x00,0x00,
0x77,0xFF,0xFF,0xFF,0x7E,0xBF,
0x3B,0xFF,0xFF,0xFF,0x63,0x71,
0x0F,0xFF,0xFE,0xAB,0x7E,0xBF,
0x00,0x00,0x1F,0xFF,0x7F,0x7F,
0x00,0x3F,0x00,0x0F,0x7E,0xBF,
0x00,0x1E,0x03,0x80,0x24,0x12,
0x00,0x30,0x0F,0xFF,0xFF,0xFB,
0x00,0x60,0x03,0x80,0x3F,0xF7,
0x00,0xC0,0x00,0x07,0xFF,0xFB,
0x01,0x80,0x00,0x07,0xD5,0x57,
0x03,0x00,0x00,0x01,0xAA,0xAB,
0x06,0x00,0x00,0x00,0xFF,0xFF,
0x0C,0x00,0x00,0x00,0x67,0xFE,
0x3E,0x00,0x00,0x01,0xF8,0x7C,
0x3E,0x00,0x00,0x01,0xF8,0x00,
0x00,0x00,0x00,0x00,0x00,0x00
};

//Sprite NightShift
z80_byte easter_egg_sprite_4[]={
3,56,
0x00,0x00,0x3C,
0x1E,0x01,0xC2,
0x35,0x86,0x12,
0x6A,0xF8,0xA9,
0x5D,0x55,0x75,
0xBE,0xAB,0xF9,
0x9F,0xFF,0xF5,
0xAF,0xAA,0xA9,
0x95,0x00,0x51,
0x4A,0x03,0x01,
0x40,0x0C,0xC3,
0x27,0xF8,0x32,
0x1C,0x00,0x0A,
0x08,0x05,0x44,
0x08,0x0A,0x84,
0x06,0x05,0x08,
0x04,0x0A,0x90,
0x06,0x17,0x10,
0x05,0x0E,0x30,
0x06,0x17,0x50,
0x05,0x0E,0x30,
0x06,0x17,0x50,
0x05,0x2E,0x30,
0x0A,0x1D,0x50,
0x0D,0x3E,0x30,
0x0A,0x1D,0x10,
0x0D,0x2E,0x28,
0x0A,0x1F,0x18,
0x15,0x2E,0x88,
0x1A,0x1F,0x18,
0x14,0x2F,0x88,
0x1A,0x57,0x58,
0x14,0x2F,0x8C,
0x2A,0x57,0xC4,
0x34,0x0B,0xAC,
0x28,0x15,0xC4,
0x34,0x0A,0xA2,
0x28,0x01,0x56,
0x50,0x00,0x02,
0x60,0x1F,0x86,
0x43,0xE0,0x72,
0x4E,0x1F,0x0A,
0x10,0xAF,0x46,
0x21,0x5E,0xB0,
0x00,0xBD,0x76,
0x11,0x5A,0xEC,
0x08,0xBD,0xD8,
0x04,0x5B,0xB0,
0x02,0x35,0x60,
0x01,0x01,0xC0,
0x00,0x3E,0x00,
0x00,0x4F,0x00,
0x00,0xBE,0x80,
0x00,0x4F,0x00,
0x00,0xBE,0x80,
0x00,0x41,0x00
};

//Sprite Franknstein
z80_byte easter_egg_sprite_5[]={
1,16,
0x3C,
0x7A,
0x84,
0xBB,
0xEF,
0xF0,
0x7E,
0x3C,
0x7E,
0x76,
0xF7,
0xFB,
0x3C,
0x76,
0x66,
0x77
};

//Sprite Exolon
z80_byte easter_egg_sprite_6[]={
3,31,
0x00,0x10,0x00,
0x00,0x20,0x00,
0x00,0x21,0xE0,
0x00,0xC7,0xF0,
0x06,0x6F,0x88,
0x0D,0xCF,0x70,
0x0B,0xB7,0x40,
0x1B,0x4B,0x78,
0x16,0xB5,0xB0,
0x02,0x7A,0xC0,
0x0F,0x7B,0x70,
0x1E,0xBB,0x00,
0x0E,0xEB,0x30,
0x01,0xF0,0x7F,
0x01,0xFD,0x86,
0x00,0x7D,0xF8,
0x01,0x81,0xB8,
0x00,0xFC,0x30,
0x02,0x00,0x00,
0x05,0x54,0x00,
0x0A,0xA0,0x00,
0x34,0x16,0x00,
0x03,0xEE,0x00,
0x0F,0x9E,0x00,
0x1F,0x0F,0x00,
0x15,0x0A,0x80,
0x7C,0x07,0x00,
0xF0,0x07,0x00,
0xE0,0x03,0x20,
0x70,0x07,0xE0,
0x3C,0x07,0x80
};

//Sprite Zorro
z80_byte easter_egg_sprite_7[]={
3,23,
0x00,0x0F,0x00,
0x00,0x0F,0x00,
0x00,0x3F,0xC0,
0x00,0x0E,0x00,
0x00,0x0F,0x00,
0x00,0x0F,0x00,
0x00,0x0E,0x00,
0x00,0x1F,0x80,
0x00,0x3F,0xC0,
0x00,0x3F,0xE0,
0x00,0x7F,0xF0,
0x00,0x7F,0x78,
0x00,0xFF,0x10,
0x00,0xFF,0x00,
0x01,0xFF,0x00,
0x01,0xFF,0x80,
0x03,0x9D,0xC0,
0x03,0x18,0xE0,
0x00,0x18,0x60,
0x00,0x30,0xC0,
0x00,0x60,0x80,
0x00,0x61,0xC0,
0x00,0x20,0x60
};


//Sprite SirFred
z80_byte easter_egg_sprite_8[]={
2,12,
0x0F,0x80,
0x07,0xC0,
0xC7,0xF8,
0x6F,0x9E,
0x3F,0xBC,
0x3F,0xF4,
0x3F,0xD0,
0x3F,0x80,
0x3F,0xEA,
0x63,0xFE,
0x47,0x7C,
0x02,0x00
};

//Sprite AquaPlane
z80_byte easter_egg_sprite_9[]={
3,14,
0x1C,0x00,0x00,
0x1F,0xC0,0x00,
0x0F,0xF8,0x00,
0x0F,0xFE,0x00,
0x07,0xFF,0x00,
0x05,0xFD,0xC0,
0x01,0x7C,0xE0,
0x05,0x7F,0xF0,
0x05,0x5F,0xF8,
0x03,0x5F,0xFC,
0x01,0xFF,0xF8,
0x00,0x79,0xC0,
0x00,0x00,0x04,
0x31,0x72,0xD0
};


//Sprite ChaseHQ invirtiendo ceros y unos
z80_byte easter_egg_sprite_10[]={
4,40,
0x00,0x00,0x00,0x00,
0x50,0x00,0x00,0x02,
0x40,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x09,0xA0,0x00,
0x00,0x33,0xA2,0x00,
0x00,0x0B,0xAC,0x80,
0x00,0x0B,0xAC,0x80,
0x00,0x03,0xEC,0x90,
0x00,0x4B,0xDD,0x90,
0x00,0x3F,0xDF,0xF0,
0x00,0x23,0xFF,0x08,
0x00,0xC0,0x3C,0x00,
0x00,0xB0,0x38,0x70,
0x00,0xBC,0x78,0xE8,
0x00,0xF0,0x38,0x10,
0x01,0x42,0x3C,0x20,
0x00,0x9F,0xBC,0xF0,
0x11,0x6B,0x9F,0x58,
0x11,0xFF,0x3F,0xFA,
0x18,0xFE,0xDD,0xFE,
0x19,0x7C,0xFE,0xFA,
0x0C,0xF8,0x1C,0xFE,
0x08,0x78,0x02,0x7A,
0x00,0xA8,0x15,0x16,
0x40,0x51,0x5A,0x8A,
0x60,0x20,0xFE,0x16,
0x40,0x13,0x01,0xE6,
0x40,0x21,0xFF,0xD6,
0x42,0x08,0xFC,0xAE,
0x41,0x25,0xC1,0xCE,
0x62,0x0B,0xFF,0xAE,
0x71,0x05,0xFF,0x4E,
0x42,0x82,0xAA,0x92,
0x35,0x41,0x45,0x1C,
0x77,0x80,0x00,0x5A,
0x67,0x40,0x00,0xDC,
0x67,0xA8,0x00,0xDA,
0x27,0xF5,0x14,0x54,
0x00,0x00,0x00,0x00
};


//Sprite FoxFightsBack
z80_byte easter_egg_sprite_11[]={
4,28,
0x01,0xFF,0xFF,0x80,
0x07,0xF1,0x1F,0xE0,
0x1F,0xF7,0x5F,0xF8,
0x3F,0xF7,0x5F,0xFC,
0x7F,0xF5,0x5F,0xFC,
0x7F,0xF1,0x1F,0xFE,
0xFF,0xFF,0xFF,0xFE,
0xFF,0xFF,0xFF,0xFF,
0xD5,0x54,0x45,0x47,
0xD5,0x46,0xEC,0x5F,
0xC5,0x56,0xED,0x5F,
0xD5,0x56,0xED,0x57,
0xD4,0x56,0xC5,0x47,
0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,
0xA8,0xAE,0x26,0xE3,
0xAA,0xAF,0x6A,0xEF,
0xDA,0xAF,0x6A,0xE7,
0xDA,0xAF,0x6A,0xEF,
0xD8,0x8E,0x26,0x23,
0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFD,0xFF,
0x7F,0x88,0xAD,0xFE,
0x7F,0xBA,0xAD,0xFC,
0x3F,0x9A,0xDD,0xF8,
0x1F,0xBA,0xAF,0xF8,
0x0F,0xB8,0xAD,0xFE,
0x03,0xFF,0xFF,0xFF
};

//Sprite Turrican
z80_byte easter_egg_sprite_12[]={
3,31,
0x00,0x7E,0x00,
0x00,0x83,0x00,
0x00,0x6B,0x80,
0x00,0x43,0x80,
0x00,0x06,0x80,
0x01,0xFC,0x80,
0x00,0xF1,0x00,
0x00,0x02,0x80,
0x81,0x11,0xC0,
0xFF,0xCB,0xE0,
0xFD,0x51,0xE0,
0x02,0x80,0x90,
0x07,0x03,0x70,
0x01,0x6B,0xA0,
0x00,0x01,0x40,
0x00,0x3E,0x00,
0x01,0xBF,0x00,
0x07,0xFF,0x00,
0x0F,0xDF,0x00,
0x17,0xDE,0x03,
0x3B,0x09,0x3F,
0x3C,0x07,0x7F,
0x20,0x0F,0x73,
0x1E,0x06,0xC3,
0x1C,0x00,0x01,
0x0E,0x00,0x00,
0x0E,0x00,0x00,
0x0F,0x00,0x00,
0x07,0x00,0x00,
0x1F,0x00,0x00,
0x3F,0x00,0x00
};


struct s_items_ayuda
{
  char *nombre_comando;
  char *nombres_alternativos;
  char *parametros;
  char *ayuda_comando;
};

struct s_items_ayuda items_ayuda[]={

  {"about",NULL,NULL,"Shows about message"},
  {"cpu-step","|cs",NULL,"Run single opcode cpu step"},
  {"cpu-step-over","|cso",NULL,"Runs until returning from the current opcode. In case if current opcode is RET or JP (with or without flag conditions) it will run a cpu-step instead of cpu-step-over"},
  {"disable-breakpoint","|db","index","Disable specific breakpoint"},
  {"disable-breakpoints",NULL,NULL,"Disable all breakpoints"},
  {"disassemble","|d","[address] [lines]","Disassemble at address. If no address specified, "
                                        "disassemble from PC register. If no lines specified, disassembles one line"},
	{"dump-nested-functions",NULL,NULL,"Shows internal nested core functions"},
  {"enable-breakpoint","|eb","index","Enable specific breakpoint"},
  {"enable-breakpoints",NULL,NULL,"Enable breakpoints"},
  {"enter-cpu-step",NULL,NULL,"Enter cpu step to step mode"},
  {"evaluate","|e","expression","Evaluate expression, can be more than one register separated by spaces. It's the same as using watches on the debug menu"},
  {"exit-cpu-step","|ecs",NULL,"Exit cpu step to step mode"},
  {"exit-emulator",NULL,NULL,"Ends emulator"},
{"find-label",NULL,"label","Finds label on source code"},
  {"generate-nmi",NULL,NULL,"Generates a NMI"},
	{"get-audio-buffer-info",NULL,NULL,"Get audio buffer information"},
  {"get-breakpoints","|gb","[index] [items]","Get breakpoints list. If set index, returns item at index. If set items, returns number of items list starting from index parameter"},
	{"get-breakpointsactions","|gba","[index] [items]","Get breakpoints actions list. If set first, returns item at index. If set items, returns number of items list starting from index parameter"},
	  {"get-buildnumber",NULL,NULL,"Shows build number. Useful on beta version, this build number is the compilation date of ZEsarUX in Unix time format"},
	{"get-cpu-core-name",NULL,NULL,"Get emulation cpu core name"},
  {"get-crc32",NULL,"start_address length","Calculate crc32 checksum starting at address for defined length. It uses current memory zone"},
  {"get-current-machine","|gcm",NULL,"Returns current machine name"},
	{"get-current-memory-zone","|gcmz",NULL,"Returns current memory zone"},
	{"get-debug-settings","|gds",NULL,"Get debug settings on remote command protocol. See command set-debug-settings"},

	{"get-io-ports",NULL,NULL,"Returns currently i/o ports used"},

	{"get-machines",NULL,NULL,"Returns list of emulated machines"},
	{"get-memory-pages","|gmp","[verbose]","Returns current state of memory pages. Default output will be the same as on debug menu; verbose output gives a detailed description of every page"},
	{"get-memory-zones","|gmz",NULL,"Returns list of memory zones of this machine"},
	{"get-ocr",NULL,NULL,"Get OCR output text"},
	{"get-os",NULL,NULL,"Shows emulator operating system"},
  {"get-paging-state",NULL,NULL,"Shows paging state on Spectrum 128k machines: if using screen 5/7 and if paging enabled"},
  {"get-registers","|gr",NULL,"Get CPU registers"},
	{"get-stack-backtrace",NULL,"[items]","Get last 16-bit values from the stack. If no items parameter, it shows 5 by default"},
	  {"get-version",NULL,NULL,"Shows emulator version"},
#ifdef EMULATE_VISUALMEM
  {"get-visualmem-written-dump","|gvmwd","[compact]","Dumps all the visual memory written positions and values. Then, clear its contents. If parameter compact, will compress non zero values on the same line, for a maximum of 16"},
  {"get-visualmem-read-dump","|gvmrd","[compact]","Dumps all the visual memory read positions and values. Then, clear its contents. If parameter compact, will compress non zero values on the same line, for a maximum of 16"},
  {"get-visualmem-opcode-dump","|gvmod","[compact]","Dumps all the visual memory executed positions and values. Then, clear its contents. If parameter compact, will compress non zero values on the same line, for a maximum of 16"},
#endif
  {"hard-reset-cpu",NULL,NULL,"Hard resets the machine"},
  {"help","|?","[command]","Shows help screen or command help"},
	{"hexdump","|h","pointer lenght","Dumps memory at address, showing hex and ascii."},
	{"hexdump-internal",NULL,"pointer lenght [offset]","Dumps internal memory (hexadecimal and ascii) for a given memory pointer. "
							"Pointer can be:\n"
							"diviface_memory: where divide/divmmc firmware and ram is located\n"
							"emulator_memory: usually includes RAM+ROM emulated machine\n"
							"rainbow_buffer: where the real video memory buffer is located\n"
							"superupgrade_flash: where the superupgrade Flash is located\n"
							"superupgrade_ram: where the superupgrade RAM is located\n"
							"\n"
							"Use with care, pointer address is a memory address on the emulator program (not the emulated memory)"},
	{"kartusho-press-button",NULL,NULL,"Press button on the Kartusho interface"},
	{"load-source-code","|lsc","file","Load source file to be used on disassemble opcode functions"},
	{"ls",NULL,NULL,"Minimal command list"},
	{"noop",NULL,NULL,"This command does nothing"},
  {"quit","|exit|logout",NULL,"Closes connection"},
	{"read-memory",NULL,"[address] [length]","Dumps memory at address. "
																				"It not specify address, dumps all memory for current memory zone: 64 KB for mapped memory on Z80, 16 kb for Spectrum 48KB ROM, etc. "
																				"If specify address but not lenght, only 1 byte is read"
	},
  {"reset-cpu",NULL,NULL,"Resets CPU"},
  {"run","|r","[verbose] [limit] [no-stop-on-data]","Run cpu when on cpu step mode. Returns when a breakpoint is fired, data sent (for example keypress) or any other event which opens the menu. Set verbose parameter to get verbose output. limit parameter is a number of opcodes to run before returning. no-stop-on-data tells that the command will not return if data is sent to the socket (for example keypress on telnet client). verbose or limit or no-stop-on-data parameters can be written in different order, for example:\nrun verbose\nor\nrun 100\nor\nrun verbose 100\n"
   "Notice this command does not run the usual cpu loop, instead it is controlled from ZRCP. If you close the connection, the run loop will die"},
	{"save-binary-internal",NULL,"pointer lenght file [offset]","Dumps internal memory to file for a given memory pointer. "
				"Pointer can be any of the hexdump-internal command\n"
				"Use with care, pointer address is a memory address on the emulator program (not the emulated memory)"},
	{"send-keys-ascii",NULL,"time asciichar1 [asciichar2] [asciichar3] ... ","Simulates sending some ascii keys on parameters asciichar, separated by spaces. Every key is separated in time by a non-press time. Time is in miliseconds, a normal value for Basic writing is 100 miliseconds"},

	{"send-keys-string",NULL,"time string","Simulates sending some keys on parameter string. Every key is separated in time by a non-press time. Time is in miliseconds, a normal value for Basic writing is 100 miliseconds"},
	{"set-breakpoint","|sb","index [condition]","Sets a breakpoint at desired index entry with condition. If no condition set, breakpoint will be handled as disabled\n"
	HELP_MESSAGE_CONDITION_BREAKPOINT
	},
	{"set-breakpointaction","|sba","index [condition]","Sets a breakpoint action at desired index entry with condition.\n"
	HELP_MESSAGE_BREAKPOINT_ACTION
	},
	{"set-cr",NULL,NULL,"Sends carriage return to every command output received, useful on Windows environments"},
	{"set-debug-settings","|sds","setting","Set debug settings on remote command protocol. It's a numeric value with bitmask with different meaning: \n"
				"Bit 0: show all cpu registers on cpu stepping or only pc+opcode.\nBit 1: show 8 next opcodes on cpu stepping.\n"
				"Bit 2: Do not add a L preffix when searching source code labels.\n"
				"Bit 3: Show bytes when debugging opcodes.\n"
				"Bit 4: Repeat last command only by pressing enter.\n"
				"Bit 5: Step over interrupt when running cpu-step, cpu-step-over and run verbose. It's the same setting as Step Over Interrupt on menu\n"
		},
	{"set-machine","|sm","machine_name","Set machine"},
	{"set-memory-zone","|smz","zone","Set memory zone number"},
  {"set-register","|sr","register=value","Changes register value. Example: set-register DE=3344H"},

	{"set-text-brightness",NULL,"brightness","Change text render brightness value (0-100)"},
	{"set-verbose-level",NULL,NULL,"Sets verbose level for console output"},
	{"set-window-zoom",NULL,"zoom","Sets window zoom"},
  {"smartload","|sl","file","Smart-loads a file. Use with care, may produce unexpected behaviour when emulator is doing a machine reset for example"},
  {"snapshot-load",NULL,"file","Loads a snapshot"},
  {"snapshot-save",NULL,"file","Saves a snapshot"},
  {"speech-empty-fifo",NULL,NULL,"Empty speech fifo"},
  {"speech-send",NULL,"message","Sends message to speech"},

  {"tbblue-get-clipwindow",NULL,"ula|layer2|sprite","Get clip window parameters. You need to tell which clip window. Only allowed on machine TBBlue"},
  {"tbblue-set-clipwindow",NULL,"ula|layer2|sprite x1 x2 y1 y2","Set clip window parameters. You need to tell which clip window. Only allowed on machine TBBlue"},


 {"tbblue-get-palette",NULL,"ula|layer2|sprite first|second index [items]","Get palette colours at index, if not specified items parameters, returns only one. You need to tell which palette. Returned values are in hexadecimal format. Only allowed on machine TBBlue"},
 {"tbblue-get-pattern",NULL,"index [items]","Get patterns at index, if not specified items parameters, returns only one. Returned values are in hexadecimal format. Only allowed on machine TBBlue"},

{"tbblue-get-register",NULL,"index","Get TBBlue register at index"},

 {"tbblue-get-sprite",NULL,"index [items]","Get sprites at index, if not specified items parameters, returns only one. Returned values are in hexadecimal format. Only allowed on machine TBBlue"},

 {"tbblue-set-palette",NULL,"ula|layer2|sprite first|second index value","Sets palette values starting at desired starting index. You need to tell which palette. Values must be separed by one space each one"},
 {"tbblue-set-pattern",NULL,"index value","Sets pattern values starting at desired pattern index. Values must be separed by one space each one, you can only define one pattern maximum (so 256 values maximum)"},

{"tbblue-set-register",NULL,"index value","Set TBBlue register with value at index"},

 {"tbblue-set-sprite",NULL,"index value","Sets sprite values starting at desired sprite index. Values must be separed by one space each one, you can only define one sprite maximum (so 4 values maximum)"},


 {"tsconf-get-af-port",NULL,"index","Get TSConf XXAF port value"},

 {"tsconf-set-af-port",NULL,"index value","Set TSConf XXAF port value"},

	{"view-basic",NULL,NULL,"Gets Basic program listing"},
	{"write-memory","|wm","address value","Writes a sequence of bytes starting at desired address on memory. Bytes must be separed by one space each one"},
	{"write-memory-raw",NULL,"address values","Writes a sequence of bytes starting at desired address on memory. Bytes must be in hexadecimal and not separed"},

 {"zxevo-get-nvram",NULL,"index","Get ZX-Evo NVRAM value at index"},

  {NULL,NULL,NULL,NULL}
};


//Si longitud=0, devuelve toda la memoria
void remote_get_memory(int misocket,unsigned int inicio,unsigned int longitud)
{

	menu_debug_set_memory_zone_attr();

	if (longitud==0) {
  	longitud=menu_debug_memory_zone_size;
	}

	while (longitud--) {
		//escribir_socket_format(misocket,"%02X",peek_byte_z80_moto(inicio++));
		escribir_socket_format(misocket,"%02X",menu_debug_get_mapped_byte(inicio++));
	}


}

void remote_help_command(int misocket,char *parametros)
{

  //char buffer_temporal[1024];

  int i;
  int encontrado=0;

  for (i=0;items_ayuda[i].nombre_comando!=NULL && !encontrado;i++) {
    if (!strcmp(parametros,items_ayuda[i].nombre_comando)) {
      escribir_socket (misocket,"Syntax: ");
      escribir_socket (misocket,items_ayuda[i].nombre_comando);
      if (items_ayuda[i].nombres_alternativos) escribir_socket (misocket,items_ayuda[i].nombres_alternativos);
      if (items_ayuda[i].parametros) {
        //escribir_socket(misocket,"\nParameters: ");
        escribir_socket(misocket," ");
        escribir_socket(misocket,items_ayuda[i].parametros);
      }
      escribir_socket (misocket,"\n\nDescription\n");
      escribir_socket (misocket,items_ayuda[i].ayuda_comando);
      encontrado=1;
    }
  }

  if (!encontrado) escribir_socket (misocket,"No help for that command");

}

//Inicio contando desde cero
void remote_get_breakpoints(int misocket,int inicio,int items)
{
  int i;

  escribir_socket (misocket,"Breakpoints: ");
  if (debug_breakpoints_enabled.v) escribir_socket (misocket,"On\n");
  else escribir_socket (misocket,"Off\n");

  for (i=inicio;i<MAX_BREAKPOINTS_CONDITIONS && i<inicio+items;i++) {

    if (debug_breakpoints_conditions_enabled[i]==0 || debug_breakpoints_enabled.v==0) {
      escribir_socket_format(misocket,"Disabled %d: ",i+1);
    }
    else {
      escribir_socket_format(misocket,"Enabled %d: ",i+1);
    }

    if (debug_breakpoints_conditions_array[i][0]) {
      escribir_socket_format(misocket,debug_breakpoints_conditions_array[i]);
    }
    else {
      escribir_socket_format(misocket,"None");
    }

    escribir_socket(misocket,"\n");

  }
}


void remote_get_memory_zones(int misocket)
{
  int i;
	char zone_name[1024];
	int readwrite;
	int size;

	escribir_socket(misocket,"Zone: -1 Name: Mapped memory\n");

	for (i=0;i<MACHINE_MAX_MEMORY_ZONES;i++) {
		size=machine_get_memory_zone_attrib(i, &readwrite);
		if (size>0) {
			machine_get_memory_zone_name(i, zone_name);
			escribir_socket_format(misocket,"Zone: %d Name: %s Size: %d R/W: %d\n",i,zone_name,size,readwrite);
		}
	}

}

void remote_get_breakpointsactions(int misocket,int inicio,int items)
{
  int i;


  for (i=inicio;i<MAX_BREAKPOINTS_CONDITIONS && i<inicio+items;i++) {
		escribir_socket_format(misocket,"%d: ",i+1);

    if (debug_breakpoints_actions_array[i][0]==0 ||
			!strcmp(debug_breakpoints_actions_array[i],"menu") ||
			!strcmp(debug_breakpoints_actions_array[i],"break")
		) {
			escribir_socket_format(misocket,"menu");
    }
    else {
      escribir_socket_format(misocket,debug_breakpoints_actions_array[i]);
    }

    escribir_socket(misocket,"\n");

  }
}


void remote_disable_breakpoint(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No index set");
    return;
  }

  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }


  debug_breakpoints_conditions_enabled[indice-1]=0;
}


void remote_enable_breakpoint(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No index set");
    return;
  }

  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }


  debug_breakpoints_conditions_enabled[indice-1]=1;
}

void remote_set_breakpoint(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No parameters set");
    return;
  }
  int i;

  //Evaluar primer parametro
  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }

  //Buscar espacio del segundo parametro

  for (i=0;parametros[i]!=' ' && parametros[i];i++);


  //Si no hay espacio de segundo parametro, segundo parametro es nulo
  //Sera igual escribir:
  //set-breakpoint 1[_espacio_][_fin_linea]
  //o
  //set-breakpoint 1[_fin_linea]
  if (parametros[i]!=0) {
    i++;
  }


//Comprobar longitud condicion. Si condicion vacia, acabara desactivando el breakpoint
  int longitud=strlen(&parametros[i]);
  if (longitud>MAX_BREAKPOINT_CONDITION_LENGTH) {
    escribir_socket(misocket,"Error. Condition too long");
    return;
  }

  debug_set_breakpoint(indice-1,&parametros[i]);
}


void remote_set_breakpointaction(int misocket,char *parametros)
{
  //Separar id indice y condicion
  //Buscar hasta espacio
  if (parametros[0]==0) {
    escribir_socket(misocket,"Error. No parameters set");
    return;
  }
  int i;

  //Evaluar primer parametro
  int indice=atoi(parametros);

  if (indice<1 || indice>MAX_BREAKPOINTS_CONDITIONS) {
    escribir_socket(misocket,"Error. Index out of range");
    return;
  }

  //Buscar espacio del segundo parametro

  for (i=0;parametros[i]!=' ' && parametros[i];i++);


  //Si no hay espacio de segundo parametro, segundo parametro es nulo
  //Sera igual escribir:
  //set-breakpointaction 1[_espacio_][_fin_linea]
  //o
  //set-breakpointaction 1[_fin_linea]
  if (parametros[i]!=0) {
    i++;
  }


//Comprobar longitud condicion. Si condicion vacia, acabara desactivando el breakpoint
  int longitud=strlen(&parametros[i]);
  if (longitud>MAX_BREAKPOINT_CONDITION_LENGTH) {
    escribir_socket(misocket,"Error. Action too long");
    return;
  }

  debug_set_breakpoint_action(indice-1,&parametros[i]);
}


void remote_disassemble(int misocket,unsigned int direccion,int lineas,int mostrar_direccion)
{

	menu_debug_set_memory_zone_attr();

  char buffer_retorno[1024];
	//char buffer_codigo_fuente[1024];
	//char posible_siguiente_linea[1024]="";

  size_t longitud_opcode;

  //int lineas_orig=lineas;

	//int mostrar_codigo_fuente=1;

	int pos_source=-1;

	int guessed_next_pos_source=-1;

  while (lineas) {
		//mostrar_codigo_fuente=1;
    //direccion=adjust_address_space_cpu(direccion);

		direccion=adjust_address_memory_size(direccion);
		//int posicion_final_linea=-1;

		if (remote_tamanyo_archivo_raw_source_code) {
			char buffer_label[128];

			if (remote_debug_settings & 4) {
				if (CPU_IS_MOTOROLA) sprintf(buffer_label,"%05X",direccion);
				else sprintf(buffer_label,"%04X",direccion);
			}
			else {
				if (CPU_IS_MOTOROLA) sprintf(buffer_label,"L%05X",direccion);
				else sprintf(buffer_label,"L%04X",direccion);
			}

			pos_source=remote_find_label_source_code(buffer_label);
			//printf ("posicion para %s: %d\n",buffer_label,pos_source);
			if (pos_source!=-1) guessed_next_pos_source=pos_source;
		}


		int pos=0;
  	if (mostrar_direccion) {
			/*if (CPU_IS_MOTOROLA) {
				sprintf(buffer_retorno,"%05X ",direccion);
				pos+=6;
			}
  		else {
				sprintf(buffer_retorno,"%04X ",direccion);
				pos +=5;
			}*/

			menu_debug_print_address_memory_zone(buffer_retorno, direccion);
			pos +=7;

			//Quitar 0 del final
			//buffer_retorno[pos]=' ';
  	}

		escribir_socket_format(misocket,"%s ",buffer_retorno);
		pos=0;

  	debugger_disassemble(&buffer_retorno[pos],100,&longitud_opcode,direccion);

		//Mostrar bytes del opcode si conviene. Tenemos buffer para 4 bytes en spectrum y para 16 en QL
		if (remote_debug_settings & 8) {
#define MAX_OPCODE_LENGHT_MOTO 12
#define MAX_OPCODE_LENGHT_Z80 4

			char buffer_bytes_opcode[MAX_OPCODE_LENGHT_MOTO*2+1]; //24+1
															//  123456789012345678901234
			strcpy(buffer_bytes_opcode,"                        ");

			if (CPU_IS_MOTOROLA) buffer_bytes_opcode[MAX_OPCODE_LENGHT_MOTO*2]=0;
			else buffer_bytes_opcode[MAX_OPCODE_LENGHT_Z80*2]=0;

			//fijamos maximo
			int buffer_longitud=longitud_opcode;
			if (buffer_longitud>MAX_OPCODE_LENGHT_MOTO) buffer_longitud=MAX_OPCODE_LENGHT_MOTO;

			//printf ("longitud :%d fijo: %d\n",longitud_opcode,buffer_longitud);

			//Copiamos bytes
			int indice;
			for (indice=0;buffer_longitud>0;buffer_longitud--,indice++) {
				char buffer_temp[3];
				sprintf (buffer_temp,"%02X",peek_byte_z80_moto(direccion+indice));
				buffer_bytes_opcode[indice*2]=buffer_temp[0];
				buffer_bytes_opcode[indice*2+1]=buffer_temp[1];
			}

			escribir_socket_format(misocket,"%s ",buffer_bytes_opcode);

		}



  	direccion +=longitud_opcode;



		if (remote_tamanyo_archivo_raw_source_code) {

			char *puntero_source=NULL;

			if (pos_source!=-1 || guessed_next_pos_source!=-1) {

				//Intentamos mostrar la siguiente linea
				if (pos_source!=-1) {
		                        int indice=remote_parsed_source_code_indexes_pointer[pos_source];
                		        puntero_source=&remote_raw_source_code_pointer[indice];
				}

				else {
					//Mostrar guessed
		                        int indice=remote_parsed_source_code_indexes_pointer[guessed_next_pos_source];
                		        puntero_source=&remote_raw_source_code_pointer[indice];

				}
			}

			escribir_socket(misocket,buffer_retorno);

			if (puntero_source!=NULL) {
				//tabular y mostrar source
				int longitud=strlen(buffer_retorno);
				//Tabular hasta columna indicada. TODO configurable
				int espacios=50-longitud;
				while (espacios>0) {
					escribir_socket(misocket," ");
					espacios--;
				}

				escribir_socket(misocket,"|");
				escribir_socket(misocket,puntero_source);
			}

		}

		else escribir_socket(misocket,buffer_retorno);


		//Y apuntamos a la siguiente linea guessed
		if (guessed_next_pos_source!=-1) guessed_next_pos_source++;


  	lineas--;
  	if (lineas) escribir_socket(misocket,"\n");


  }
}

void remote_disable_multitask(void)
{

  menu_multitarea=0;

  audio_playing.v=0;

  timer_reset();
  //Pausa de 0.1 segundo
  usleep(100000);

}

void remote_enter_menu(void)
{
  menu_abierto=1;
}
void remote_disable_multitask_enter_menu(void)
{
  remote_enter_menu();
  remote_disable_multitask();
}

void remote_send_esc_close_menu(void)
{
  //Simulamos pulsar ESC
  salir_todos_menus=1;
  puerto_especial1 &=(255-1);
  //Pausa de 0.1 segundo
  usleep(100000);
  puerto_especial1 |=1;
}


void remote_footer_cpu_step(void)
{
	menu_putstring_footer(WINDOW_FOOTER_ELEMENT_X_CPUSTEP,1,"STEP",WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);
	//Dado que se ha quitado multitask y esta todo parado, refrescar pantalla para mostrar el footer
	scr_refresca_pantalla_solo_driver();

}

void remote_footer_cpu_step_clear(void)
{
	menu_putstring_footer(WINDOW_FOOTER_ELEMENT_X_CPUSTEP,1,"     ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
	//no hace falta forzar refresco pantalla dado que se sale de este estado, menu esta cerrado y por tanto se ejecutara el core tal cual

}

//estado multitarea antes de cpu_step
int menu_multitarea_antes_cpu_step=0;

//Este modo deja la emulacion pausada
void remote_cpu_enter_step(int misocket)
{
  //De momento solo simular pulsacion de tecla de menu, eso hace saltar el step

  //TODO: Pendiente de eliminar esta variable. Tiene sentido???
  //menu_espera_tecla_no_cpu_loop_flag_salir.v=1;

  //guardar estado multitarea
  menu_multitarea_antes_cpu_step=menu_multitarea;

  remote_ack_enter_cpu_step.v=0;


  //Abrir menu si no estaba ya abierto
  remote_disable_multitask_enter_menu();
  //Cerrar menu
  remote_send_esc_close_menu();

  //Pausa de 0.1 segundo. Esperamos a que se haya cerrado el menu
  usleep(100000);

  //Avisar que entramos en paso a paso y Abrir menu
  menu_event_remote_protocol_enterstep.v=1;
  menu_abierto=1;

  int contador_timeout;
  //Esperamos a que se haya recibido la señal de entrada en remote cpu-step
  //20 iteraciones=2 segundos
  for (contador_timeout=0;contador_timeout<20 && remote_ack_enter_cpu_step.v==0;contador_timeout++) {
    usleep(100000); //0.1 segundo
  }

  if (remote_ack_enter_cpu_step.v==0) {
    //Menu no se ha enterado de entrada en este modo. Error
    escribir_socket(misocket,"Error. Can not enter cpu step mode. You can try closing the menu");
    menu_event_remote_protocol_enterstep.v=0;
    return;
  }

	remote_footer_cpu_step();

}


//Muestra salida de registros y opcode cuando se hacen operaciones de cpu step, run, etc
void remote_get_regs_disassemble(int misocket)
{
  char buffer_retorno[2048];

  //Mostrar registros
	if (remote_debug_settings&1) {
  	print_registers(buffer_retorno);
  	escribir_socket (misocket,buffer_retorno);

	//Mostrar T-estados
	escribir_socket_format(misocket," TSTATES: %d",t_estados);

  	//Salto linea
  	escribir_socket (misocket,"\n");
	}

	/*else  {
		if (CPU_IS_MOTOROLA) {
			sprintf (buffer_retorno,"%05X ",get_pc_register());
		}
		else {
			sprintf (buffer_retorno,"%04X ",get_pc_register());
		}

		escribir_socket (misocket,buffer_retorno);
	}*/


  //Y desensamblar direccion donde apunta el PC
	int lineas=1;
	if (remote_debug_settings&2) lineas=8;
  remote_disassemble(misocket,get_pc_register(),lineas,1 );

}


void remote_cpu_exit_step_continue(void)
{
  menu_event_remote_protocol_enterstep.v=0;
  remote_send_esc_close_menu();
	remote_footer_cpu_step_clear();
}

void remote_cpu_exit_step(int misocket)
{
  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You are not in step to step mode");
    return;
  }
	remote_cpu_exit_step_continue();

 //Restaurar estado multitarea
 menu_multitarea=menu_multitarea_antes_cpu_step;
}

void remote_cpu_after_core_loop(void)
{
  if (menu_multitarea==0) {
    audio_playing.v=0;
    //audio_thread_finish();
  }
}

void remote_cpu_step(int misocket) {
  //char buffer_retorno[1024];
  //Ejecutar una instruccion
  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You must first enter cpu-step mode");
    return;
  }
  debug_core_lanzado_inter.v=0;
  cpu_core_loop();

  if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
	  debug_run_until_return_interrupt();
  }


  remote_cpu_after_core_loop();

  remote_get_regs_disassemble(misocket);

}

/*
int si_remote_cpu_step_over_jpret(void)
{
	if (CPU_IS_MOTOROLA || CPU_IS_SCMP) return 0;
	z80_byte opcode=peek_byte_no_time(reg_pc);

	switch (opcode)
	{

		case 0xC3: // JP
                case 0xCA: // JP Z
                case 0xD2: // JP NC
                case 0xDA: // JP C
                case 0xE2: // JP PO
                case 0xE9: // JP (HL)
                case 0xEA: // JP PE
                case 0xF2: // JP P
                case 0xFA: // JP M

                case 0xC0: // RET NZ
                case 0xC8: // RET Z
                case 0xC9: // RET
                case 0xD0: // RET NC
                case 0xD8: // RET C
                case 0xE0: // RET PO
                case 0xE8: // RET PE
                case 0xF0: // RET P
                case 0xF8: // RET M

			return 1;
		break;
	}

	return 0;

}
*/

void remote_cpu_step_over(int misocket) {

  //char buffer_retorno[1024];
  //Si apunta PC a instrucciones RET o JP, hacer un cpu-step
  if (si_cpu_step_over_jpret()) {
	  debug_printf(VERBOSE_DEBUG,"Running only cpu-step as current opcode is JP or RET");
	  remote_cpu_step(misocket);
	  return;
  }


  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You must first enter cpu-step mode");
    return;
  }


debug_cpu_step_over();

/*
  unsigned int direccion=get_pc_register();
  int longitud_opcode=remote_get_opcode_length(direccion);

  unsigned int direccion_final=direccion+longitud_opcode;
  direccion_final=adjust_address_space_cpu(direccion_final);


  //Parar hasta volver de la instruccion actual o cuando se produzca algun evento de apertura de menu, como un breakpoint
  menu_abierto=0;
  int salir=0;
  while (get_pc_register()!=direccion_final && !salir) {
    debug_core_lanzado_inter.v=0;
    cpu_core_loop();

    if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
    	debug_run_until_return_interrupt();
    }

    if (menu_abierto) salir=1;
  }
*/

  remote_cpu_after_core_loop();

  remote_get_regs_disassemble(misocket);


}

void remote_cpu_run_loop(int misocket,int verbose,int limite,int datos_vuelve)
{

	char buf[30];


	//De momento no se como hacer esto en Windows
	//Hacer el socket que no se quede bloqueado si no hay datos.

	if (datos_vuelve) {
	#ifdef MINGW
	  u_long on = 1;
    ioctlsocket(sock_conectat, FIONBIO, &on);
	#else
		int flags = fcntl(sock_conectat, F_GETFL, 0);
		fcntl(sock_conectat, F_SETFL, flags | O_NONBLOCK);
	#endif

	}

	int total_instrucciones=0;

	int salir=0;
	while (!salir) {
	  if (verbose) {
	    remote_get_regs_disassemble(misocket);
	    escribir_socket(misocket,"\n");
	  }
	  debug_core_lanzado_inter.v=0;
	  cpu_core_loop();
	  if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
		debug_run_until_return_interrupt();
	  }


	//Si se vuelve cuando hay datos en el socket. Con esto, usa mucha mas cpu (46% respecto a un 19% si no se esta mirando el socket)
	if (datos_vuelve) {
	#ifdef MINGW
		int leidos=recv(sock_conectat,buf,30,0);
		//int leidos = read(sock_conectat, buf, 30);
		if (leidos>0) {
			salir=1;      	
		}
	#else
		//int leidos = read(sock_conectat, buf, 30);
		int leidos=recv(sock_conectat,buf,30,0);
		if (leidos>0) {
			salir=1;      	
		}
	#endif
	}


	  total_instrucciones++;
	  if (limite) {
	    if (total_instrucciones==limite) {
	      escribir_socket_format(misocket,"Returning after %d opcodes\n",limite);
	      salir=1;
	    }
	  }
	  if (menu_abierto) salir=1;
	}


	//Dejar el socket tranquilito como estaba antes
	if (datos_vuelve) {
	#ifdef MINGW
		u_long on = 0;
    	ioctlsocket(sock_conectat, FIONBIO, &on);
	#else
		int flags = fcntl(sock_conectat, F_GETFL, 0);
		fcntl(sock_conectat, F_SETFL, flags ^ O_NONBLOCK);
	#endif
	}

	//Si se ha saltado un breakpoint, decirlo
	if (menu_breakpoint_exception.v) {
		if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
			escribir_socket_format(misocket,"Breakpoint fired: %s\n",catch_breakpoint_message);
			}
	}


}



//Ejecutar hasta siguiente punto de paro o cualquier otro evento que abra el menu
//Variables: verbose: si se muestra desensamblado en cada instruccion,
//limite: si se ejecutan N instrucciones y se finaliza. Si es 0, no tiene limite (hasta apertura de menu o breakpoint)
void remote_cpu_run(int misocket,int verbose,int limite,int datosvuelve)
{

  if (menu_event_remote_protocol_enterstep.v==0) {
    escribir_socket(misocket,"Error. You must first enter cpu-step mode");
    return;
  }




  //Parar cuando se produzca algun evento de apertura de menu, como un breakpoint
  menu_abierto=0;


//Opcion A. Solo para este caso de run
/*
 #ifdef MINGW
  towindows_remote_cpu_run_misocket=misocket;
  towindows_remote_cpu_run_verbose=verbose;
  towindows_remote_cpu_run_limite=limite;
  towindows_remote_cpu_run_loop=1;

	debug_printf (VERBOSE_DEBUG,"Calling remote_cpu_run_loop from ZRCP to main loop");
  while (towindows_remote_cpu_run_loop)  {
	  sleep (1);
  }
  	debug_printf (VERBOSE_DEBUG,"Exiting remote_cpu_run_loop from ZRCP to main loop");
 #else
  remote_cpu_run_loop(misocket,verbose,limite,datosvuelve);
 #endif
*/

//Opcion B. Hacemos esto normal y luego en menu, el menu_event_remote_protocol_enterstep refresca la pantalla por si solo cada 1 segundo
  remote_cpu_run_loop(misocket,verbose,limite,datosvuelve);




  debug_printf(VERBOSE_DEBUG,"Exiting run command");

  remote_cpu_after_core_loop();
  remote_get_regs_disassemble(misocket);

}

void remote_evaluate(int misocket,char *texto)
{
  char buffer_retorno[1024];


  debug_watches_loop(texto,buffer_retorno);
  escribir_socket(misocket,buffer_retorno);
}

/*void easter_egg_put_char(int x,int y,int color,unsigned char c)
{

  z80_bit inverse,f;

inverse.v=0;
f.v=0;
//128 y 129 corresponden a franja de menu y a letra enye minuscula
if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';
scr_putsprite_comun(&char_set[(caracter-32)*8],x,y,inverse,tinta,papel,f);



y=y*8;

for (line=0;line<8;line++,y++) {
  byte_leido=*puntero++;
  if (inverse.v==1) byte_leido = byte_leido ^255;
  for (bit=0;bit<8;bit++) {
        if (byte_leido & 128 ) color=tinta;
        else color=papel;

        //simular modo fast para zx81
        if (MACHINE_IS_ZX8081) {
                if (fast_mode.v==1 && video_fast_mode_emulation.v==1 && video_fast_mode_next_frame_black==LIMIT_FAST_FRAME_BLACK) color=0;
        }

        byte_leido=(byte_leido&127)<<1;

        //este scr_putpixel_zoom_rainbow tiene en cuenta los timings de la maquina (borde superior, por ejemplo)
        if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow((x*8)+bit+margenx_izq,y+margeny_arr,color);

        else scr_putpixel_zoom((x*8)+bit,y,color);

   }
}



}*/

void remote_easter_egg_refresca_pantalla(void)
{
  //sin color oscuro gris
  menu_abierto=0;
  //printf ("%p\n",scr_refresca_pantalla_solo_driver);
  scr_refresca_pantalla_solo_driver();
  menu_abierto=1;
  scr_actualiza_tablas_teclado(); //Xwindows por ejemplo requiere esto
}

z80_int *easter_egg_front_display;
z80_int *easter_egg_back_display;

void easter_egg_asigna_memoria_pantalla(void)
{
  //Asignamos las dos pantallas para visualizacion del easter egg
  //Una front y la otra back
  easter_egg_front_display=malloc(2*256*192); //16 bits, 256x192 resolucion

  easter_egg_back_display=malloc(2*256*192);

  if (easter_egg_front_display==NULL || easter_egg_back_display==NULL)
  {
    cpu_panic("Can not allocate memory for easter egg");
  }

}

void easter_egg_libera_memoria_pantalla(void)
{
  free(easter_egg_back_display);
  free(easter_egg_front_display);
}

void easter_egg_flush_display(void)
{
  //Mezcla las dos pantallas y las visualiza en la ventana
  //Back es la de fondo
  //Front es la de delante
  //Si color en front es 65535, es transparente y se muestra el back

  z80_int *back,*front;

  back=easter_egg_back_display;
  front=easter_egg_front_display;

  int x,y;
  z80_int color_final;

  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      color_final=*front;
      if (color_final==65535) color_final=*back;
      scr_putpixel_zoom(x,y,color_final);

      front++;
      back++;
    }
  }

  remote_easter_egg_refresca_pantalla();
}

void easter_egg_putpixel_front(int x,int y,z80_int color)
{
  int offset;
  offset=y*256+x;
  easter_egg_front_display[offset]=color;
}

void easter_egg_putpixel_back(int x,int y,z80_int color)
{
  int offset;
  offset=y*256+x;
  easter_egg_back_display[offset]=color;
}

z80_int easter_egg_getpixel_front(int x,int y)
{
  int offset;
  offset=y*256+x;
  return easter_egg_front_display[offset];
}

//Dibuja franjas de colores en el background, de color oscuro. Excluir el negro y azul, para que se vea mejor texto
void easter_egg_rainbow_back(void)
{
  int x,y;
  z80_int color=2;

  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      easter_egg_putpixel_back(x,y,color+16); //+16 porque son los colores oscuros
     }
    color++;
    if (color==8) color=2; //Negro ni azul no
  }

}

void easter_egg_transparent_front(void)
{
  int x,y;
  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      easter_egg_putpixel_front(x,y,65535);
    }
  }
}

void easter_egg_black_front(void)
{
  int x,y;
  for (y=0;y<192;y++) {
    for (x=0;x<256;x++) {
      easter_egg_putpixel_front(x,y,0);
    }
  }
}

z80_int easter_egg_get_random(void)
{
  ay_randomize(0);

  //randomize_noise es valor de 16 bits
  return randomize_noise[0];
}

void easter_egg_random_pixels_front(void)
{
  int x,y;
  z80_int color,color_final;

  int iteraciones;

  int max_iteraciones=50*10; //50 frames, 10 segundos

  int valor_si_transparente=0;

  //5 segundos
  for (iteraciones=0;iteraciones<max_iteraciones;iteraciones++) {

    for (y=0;y<192;y++) {
      for (x=0;x<256;x++) {
        color=easter_egg_get_random() % EMULATOR_TOTAL_PALETTE_COLOURS;
        color_final=color;

        //A partir de mitad de proceso, colores pueden salir transparentes
        if (iteraciones>=max_iteraciones/2) {
          //Formula 1: A medida que avanzan las iteraciones, van desapareciendo zonas de pixeles
          //if ((x+y)/2==valor_si_transparente) color_final=65535;

          //Formula 2: colores van desapareciendo segun random
          if (color_final<200) color_final=65535; //200 puesto a ojo ;)
        }



        //solo poner el pixel si no es transparente el que habia
        if (easter_egg_getpixel_front(x,y)!=65535) easter_egg_putpixel_front(x,y,color_final);
      }

      //Un poco mas de random al tema
      int entropia;
      for (entropia=0;entropia<y;entropia++) easter_egg_get_random();
    }

    if (iteraciones>=max_iteraciones/2) valor_si_transparente++;

    easter_egg_flush_display();
    usleep(1000000/50);
  }
}

//Dibujar solo una linea de un caracter (no los 8 pixeles de alto)
//scanline va de 0 a 7
void easter_scanline_putchar_front(z80_byte caracter,int x,int y,int scanline,z80_int color)
{
    z80_byte *puntero_caracter;
    int offset;
    z80_byte byte_leido;

    if (!si_valid_char(caracter)) caracter='?';

    offset=(caracter-32)*8;

    puntero_caracter=&char_set_spectrum[offset+scanline];
    byte_leido=*puntero_caracter;

    //Hacemos bold
    z80_byte byte_bold=byte_leido<<1;
    byte_leido |=byte_bold;


    int bit;

    for (bit=0;bit<8;bit++) {
      if (byte_leido&128) easter_egg_putpixel_front(x+bit,y,color);

      byte_leido=byte_leido<<1;
    }
}

//x,y en coordenadas pixeles
//ancho en caracteres (1=8 pixeles)
//alto en pixeles
void easter_egg_put_sprite_front(z80_byte *sprite, int x, int y, int ancho, int alto, z80_int color)
{

  int xorig=x;
  int anchoorig=ancho;
  z80_byte byte_leido;

  for (;alto>0;alto--,y++) {
    x=xorig;
    ancho=anchoorig;
    for (;ancho>0;ancho--,x+=8) {
      int bit;

      byte_leido=*sprite;
      sprite++;
      for (bit=0;bit<8;bit++) {
        if (byte_leido&128) {
          int xfinal=x+bit;
          if (xfinal>=0 && xfinal<=255 && y>=0 && y<=191) {  //Podemos jugar con sprites que salen de los limites de la pantalla
            easter_egg_putpixel_front(xfinal,y,color);
          }
        }
        byte_leido=byte_leido<<1;
      }
    }
  }

}

void easter_egg_scroll_vertical_front(void)
{
  int x,y;
  z80_int color;
  for (y=0;y<191;y++) {
    for (x=0;x<256;x++) {
      color=easter_egg_getpixel_front(x,y+1);
      easter_egg_putpixel_front(x,y,color);
    }
  }

  //Ultima linea transparente
  for (x=0;x<256;x++) easter_egg_putpixel_front(x,191,65535);
}

void easter_egg_scroll_horizontal_continuo_front(void)
{
  //Copiar primera linea de la izquierda que va a desaparecer
  z80_int linea_copia[192];
  z80_int color;

  int x,y;
  for (y=0;y<192;y++) linea_copia[y]=easter_egg_getpixel_front(0,y);

  //Scroll
  for (x=0;x<255;x++) {
    for (y=0;y<192;y++) {
      color=easter_egg_getpixel_front(x+1,y);
      easter_egg_putpixel_front(x,y,color);
    }
  }

  //Y poner linea a la derecha
  for (y=0;y<192;y++) easter_egg_putpixel_front(255,y,linea_copia[y]);
}

void easter_egg_star_wars_write_line(char *texto,int longitud_texto,int x,int y,int scanline, z80_int color)
{
  for (;longitud_texto;longitud_texto--) {
    easter_scanline_putchar_front(*texto,x,y,scanline,color);
    x+=8;
    texto++;
  }
}



z80_byte *easter_egg_retorna_numero_sprite(int numero_sprite,int *ancho,int *alto)
{
  z80_byte *sprite;

  switch (numero_sprite)
  {
    case 0:
      sprite=easter_egg_sprite_0;
    break;

    case 1:
      sprite=easter_egg_sprite_1;
    break;

    case 2:
      sprite=easter_egg_sprite_2;
    break;

    case 3:
      sprite=easter_egg_sprite_3;
    break;

    case 4:
      sprite=easter_egg_sprite_4;
    break;

    case 5:
      sprite=easter_egg_sprite_5;
    break;

    case 6:
      sprite=easter_egg_sprite_6;
    break;

    case 7:
      sprite=easter_egg_sprite_7;
    break;

    case 8:
      sprite=easter_egg_sprite_8;
    break;

    case 9:
      sprite=easter_egg_sprite_9;
    break;

    case 10:
      sprite=easter_egg_sprite_10;
    break;

    case 11:
      sprite=easter_egg_sprite_11;
    break;

    case 12:
      sprite=easter_egg_sprite_12;
    break;


    default:
      sprite=easter_egg_sprite_0;
    break;
  }
  *ancho=*sprite;
  sprite++;
  *alto=*sprite;
  sprite++;

  return sprite;

}
int sprite_actual=0;

void easter_egg_show_random_sprite(void)
{
  int x,y;

    x=((easter_egg_get_random() ) % 200)+10;
    y=((easter_egg_get_random() ) % 100)+20;

    //int numero_sprite=easter_egg_get_random() % EASTER_EGG_TOTAL_SPRITES;
    int  numero_sprite=sprite_actual  % EASTER_EGG_TOTAL_SPRITES;
    sprite_actual++;
    //printf ("numero sprite: %d\n",numero_sprite);
    int ancho,alto;

    z80_byte *sprite=easter_egg_retorna_numero_sprite(numero_sprite,&ancho,&alto);

    z80_int color=easter_egg_get_random() % EMULATOR_TOTAL_PALETTE_COLOURS;

    easter_egg_put_sprite_front(sprite,x,y,ancho,alto,color);
}

#define EASTER_TEXT_SCROLL_PAUSE (1000000/7)
#define EASTER_STAR_WARS_TEXT_COLOR 15
#define EASTER_REBOTE_PELOTA_PAUSA (1000000/1000)

void easter_egg_star_wars_text_one_line(char *texto)
{
  int scanline;

    for (scanline=0;scanline<8;scanline++) {
      easter_egg_star_wars_write_line(texto,strlen(texto),0,191,scanline,EASTER_STAR_WARS_TEXT_COLOR);  //Siempre escribir en ultima linea 191
      easter_egg_flush_display();
      usleep(EASTER_TEXT_SCROLL_PAUSE); //0.25 s de pausa
      easter_egg_scroll_vertical_front();
    }

}

int mostrar_sprite_random_contador=0;

z80_bit easter_egg_mostrar_sprite={0};

void easter_egg_star_wars_text(char *texto_orig)
{

    char texto[MAX_TEXTO_GENERIC_MESSAGE];
    strcpy(texto, texto_orig);
  //void menu_generic_message_tooltip(char *titulo, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, const char * texto_format , ...)

    //Funcion derivada de menu_generic_message_tooltip
  	//texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
  	char buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE][MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

  	const int max_ancho_texto=30;

  	//Primera linea que mostramos en la ventana
  	//int primera_linea=0;

  	int indice_linea=0;  //Numero total de lineas??
  	int indice_texto=0;
  	int ultimo_indice_texto=0;
  	int longitud=strlen(texto);

  	//int indice_segunda_linea;

  	//int texto_no_cabe=0;

  	do {
  		indice_texto+=max_ancho_texto;


  		//printf ("indice_linea: %d\n",indice_linea);

  		//Controlar final de texto
  		if (indice_texto>=longitud) indice_texto=longitud;

  		//Si no, miramos si hay que separar por espacios
  		else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

  		//Separamos por salto de linea, filtramos caracteres extranyos
  		indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

  		//copiar texto
  		int longitud_texto=indice_texto-ultimo_indice_texto;


  		//snprintf(buffer_lineas[indice_linea],longitud_texto,&texto[ultimo_indice_texto]);


  		menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
  		buffer_lineas[indice_linea++][longitud_texto]=0;
  		//printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


  		//printf ("texto: %s\n",buffer_lineas[indice_linea-1]);

  		if (indice_linea==MAX_LINEAS_TOTAL_GENERIC_MESSAGE) {
                          //cpu_panic("Max lines on menu_generic_message reached");
  			debug_printf(VERBOSE_INFO,"Max lines on menu_generic_message reached (%d)",MAX_LINEAS_TOTAL_GENERIC_MESSAGE);
  			//finalizamos bucle
  			indice_texto=longitud;
  		}

  		ultimo_indice_texto=indice_texto;
  		//printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);

  	} while (indice_texto<longitud);


  	debug_printf (VERBOSE_INFO,"Read %d lines (word wrapped)",indice_linea);


    int linea_leida;

    for (linea_leida=0;linea_leida<indice_linea;linea_leida++) {
      easter_egg_star_wars_text_one_line(buffer_lineas[linea_leida]);

      //Y si se da el caso, mostrar un sprite al azar
      if ((mostrar_sprite_random_contador%3)==0 && easter_egg_mostrar_sprite.v==1) easter_egg_show_random_sprite();
      mostrar_sprite_random_contador++;
    }



    //easter_egg_star_wars_text_one_line("Hola que tal");
    //easter_egg_star_wars_text_one_line("esto es una prueba");

}


//Derivado de menu_about_read_file
void easter_egg_read_file(char *aboutfile)
{

  char about_file[MAX_TEXTO_GENERIC_MESSAGE];

  debug_printf (VERBOSE_INFO,"Loading %s File",aboutfile);
  FILE *ptr_aboutfile;
//ptr_aboutfile=fopen(aboutfile,"rb");
  open_sharedfile(aboutfile,&ptr_aboutfile);

  if (!ptr_aboutfile)
  {
    easter_egg_star_wars_text("Sorry, I can not read the file... maybe you have deleted it?");
  }
  else {

    int leidos=fread(about_file,1,MAX_TEXTO_GENERIC_MESSAGE,ptr_aboutfile);
    debug_printf (VERBOSE_INFO,"Read %d bytes of file: %s",leidos,aboutfile);

    if (leidos==MAX_TEXTO_GENERIC_MESSAGE) {
      debug_printf (VERBOSE_ERR,"Read max text buffer: %d bytes. Showing only these",leidos);
      leidos--;
    }

    about_file[leidos]=0;


    fclose(ptr_aboutfile);

    easter_egg_star_wars_text(about_file);
  }
}

void easter_egg_star_wars(void)
{

  easter_egg_mostrar_sprite.v=0;

  easter_egg_star_wars_text("A long time ago, in a galaxy far, far away....");
  easter_egg_star_wars_text("\n\n");
  easter_egg_star_wars_text("Just kidding... this is not the usual Star Wars scroller... you have just found the ZEsarUX easter egg! \n"
                            "So, this could be my first demo for the Speccy, but this demo does not run on a real Speccy, "
                            "it runs from the inside of the emulator, this is not a Spectrum binary program. \n"

                            "You may notice every pixel in this demo can have its own color, and I can use here the total "
                            "palette colours of the emulator.\n"
                            "This demo have two planes, one background with the rainbow stripes and one foreground, with the scroller. "
                            "I can also do some fast and curious effects like this:\n\n");



  int contador;
  int veces_scroll_horizontal=1;

  for (contador=0;contador<256*veces_scroll_horizontal;contador++) {  //multiple de 256 para que deje la pantalla como estaba antes
    easter_egg_flush_display();
    usleep(1000000/50); //20 ms de pausa
    easter_egg_scroll_horizontal_continuo_front();
  }

  easter_egg_star_wars_text("\nWell maybe it's not very impressive, but I'm not an expert Spectrum demo programmer, I'm better at making "
                            "Spectrum emulators ;)\n");


  easter_egg_star_wars_text("I can also draw some sprites...");
  easter_egg_mostrar_sprite.v=1;

  easter_egg_star_wars_text("\n\n\n");


  easter_egg_star_wars_text("Do you recognize all the sprites? They all come from different Spectrum games.\n\n");

  easter_egg_star_wars_text("So... I don't want you to stay here reading lots of text, so if you want, you can read the following and the "
                            "usual acknowledgements list... I expect you enjoyed this simple? demo ;)\n"
                            "\nACKNOWLEDGEMENTS:\n\n");
  easter_egg_read_file("ACKNOWLEDGEMENTS");

  //Scroll final
  int y;
  for (y=0;y<192;y++) {
    easter_egg_flush_display();
    usleep(EASTER_TEXT_SCROLL_PAUSE);
    easter_egg_scroll_vertical_front();
  }
}

void easter_egg_rebote_pelota(void)
{

  //Primer flush. Para que cuando se hace putpixel directo, tengamos antes la mezcla de front-back aqui
  easter_egg_flush_display();

  int x,y;
    //Rebote pelota
    x=((get_pc_register() ^ easter_egg_get_random()) +17 ) % 256; //x random segun pc. +17 por meter un random
    y=(get_pc_register() ^ easter_egg_get_random() ) % 192; //y random segun pc
    //mejor x,y fijados asi sabemos como acabara
    //x=33;
    //y=99;

    int incx=+1;
    int incy=+1;
    int contador=0;

    z80_int color=(get_pc_register() ^ easter_egg_get_random() ) % EMULATOR_TOTAL_PALETTE_COLOURS;

    while (contador<50000) //Con esto llenamos la pantalla
    {
      //Hacemos putpixel normal sin front-back
      //easter_egg_putpixel_front(x,y,color);
      scr_putpixel_zoom(x,y,color);
      color++;
      if (color==EMULATOR_TOTAL_PALETTE_COLOURS) color=0;

      x +=incx;
      y +=incy;

      if (x==0 || x==255) {
        incx=-incx;
      }

      if (y==0 || y==191) {
        incy=-incy;
      }


      usleep(EASTER_REBOTE_PELOTA_PAUSA);


      //hacemos refresco normal sin front-back
      //easter_egg_flush_display();

      //Refresco de vez en cuando
      if ( (contador%100)==0) remote_easter_egg_refresca_pantalla();

      contador++;
    }

}

void remote_easter_egg_mostrar(void)
{

  easter_egg_asigna_memoria_pantalla();

  easter_egg_rainbow_back();

  //5 segundos de pixeles aleatorios
  easter_egg_black_front();
  easter_egg_random_pixels_front();


  easter_egg_transparent_front();



  easter_egg_star_wars();

  easter_egg_rebote_pelota();


  easter_egg_libera_memoria_pantalla();

}

void remote_easter_egg(int misocket)
{

  if (!si_complete_video_driver()) {
    escribir_socket(misocket,"Error. You need to try this using a full video driver...");
    return;
  }

    //Entramos en el mismo modo que cpu-step para poder congelar la emulacion
    remote_cpu_enter_step(misocket);
    if (menu_event_remote_protocol_enterstep.v==0) return;



    remote_easter_egg_mostrar();
    sleep(5);


    remote_cpu_exit_step(misocket);

}

//Retorna puntero a fin de comando (0) o salto de linea, o bien, caracter de despues del espacio
char *find_space_or_end(char *s)
{
  while (*s && *s!=' ' && *s!='\r') {
    s++;
  }
  if (*s==' ') s++;
  return s;
}

void remote_cerrar_conexion(void)
{
#ifdef MINGW
	closesocket(sock_conectat);
#else
	close(sock_conectat);
#endif

	//#ifdef MINGW
	//WSACleanup();
	//#endif
}

//Parseo de parametros de comando.
#define REMOTE_MAX_PARAMETERS_COMMAND 10
//array de punteros a comando y sus argumentos
char *remote_command_argv[REMOTE_MAX_PARAMETERS_COMMAND];
int remote_command_argc;

//Separar comando con codigos 0 y rellenar array de parametros
void remote_parse_commands_argvc(char *texto)
{

  remote_command_argc=util_parse_commands_argvc(texto, remote_command_argv, REMOTE_MAX_PARAMETERS_COMMAND);

}

void remote_dump_nested_core_functions(int misocket)
{

	char buffer[10000];


	debug_dump_nested_functions(buffer);

	escribir_socket(misocket,buffer);
}

//Escribe en destino las dos cadenas, la segunda tabulada en la posicion column, y tabulada mediante espacios
/*void remote_print_strings_tabbed(char *destino, char *string1, char *string2, int column)
{
	//Primero llenar de espacios hasta columna
	int i;
	for (i=0;i<column;i++) {
		destino[i]=' ';
	}

	//Escribe las dos cadenas
	sprintf(destino,"%s",string1);

	//Quitar 0 final
	destino[strlen(string1)]=' ';

	//Escribe cadena 2
	sprintf(&destino[column],"%s",string2);
}
*/

void remote_fill_string_spaces(char *string, int length)
{
	int i;

	for (i=0;i<length;i++) {
		*string=' ';
		string++;
	}

	*string=0;
}

int remote_get_max_length_command(void)
{
	int salto_tabulacion=0;
	int i;

	//Calcular primero salto de tabulacion ideal, leyendo mayor longitud de texto
	for (i=0;items_ayuda[i].nombre_comando!=NULL;i++) {
		int l=strlen(items_ayuda[i].nombre_comando);
		if (l>salto_tabulacion) salto_tabulacion=l;
	}

	return salto_tabulacion;
}

void remote_print_no_zero_ending(char *s,char *string_to_add)
{
	while (*string_to_add) {
		*s=*string_to_add;

		s++;
		string_to_add++;
	}
}

//Retorna lista de comandos con ayuda simple en misma linea
void remote_help_no_parameters_command(int misocket)
{

		//Mostrar comando y al lado ayuda del mismo, reducida hasta el siguiente punto

		//Calcular primero maxima longitud comando
		int salto_tabulacion=remote_get_max_length_command();

		//darle dos espacios mas

		salto_tabulacion +=2;

		char buffer_texto[1024];

		char buffer_ayuda[1024];

    int i;
    for (i=0;items_ayuda[i].nombre_comando!=NULL;i++) {

				//llenar string con espacios minimo hasta salto_tabulacion
				remote_fill_string_spaces(buffer_texto,salto_tabulacion);



				//Meter en buffer_ayuda hasta siguiente . o final
				int j;

				for (j=0;items_ayuda[i].ayuda_comando[j]!=0 && items_ayuda[i].ayuda_comando[j]!='.';j++) {
					buffer_ayuda[j]=items_ayuda[i].ayuda_comando[j];
				}

				buffer_ayuda[j]=0;

				//Meter en buffer texto las dos cadenas
				remote_print_no_zero_ending(buffer_texto,items_ayuda[i].nombre_comando);

				sprintf(&buffer_texto[salto_tabulacion],"%s",buffer_ayuda);
				escribir_socket(misocket,buffer_texto);


        escribir_socket (misocket,"\n");
    }

    escribir_socket_format(misocket,"\nTotal commands: %d\n",i);

}

//Retorna lista de comandos tabulados en varias columnas
void remote_simple_help(int misocket)
{


		int salto_tabulacion=remote_get_max_length_command();

		//y darle dos espacios mas
		salto_tabulacion +=2;

		const int columnas_total=4;
		int columna=0;
		char buffer_texto[1024];
		int i;

    for (i=0;items_ayuda[i].nombre_comando!=NULL;i++) {
				if (columna==0) remote_fill_string_spaces(buffer_texto,columnas_total*salto_tabulacion);

				int pos_columna=columna*salto_tabulacion;


				remote_print_no_zero_ending(&buffer_texto[pos_columna],items_ayuda[i].nombre_comando);
				//sprintf(&buffer_texto[pos_columna],"%s",items_ayuda[i].nombre_comando);
				//Quitar 0 del final
				buffer_texto[pos_columna+strlen(items_ayuda[i].nombre_comando)]=32;

				columna++;

				//Si final de columna o final de comandos
				if (columna==columnas_total) {
					escribir_socket(misocket,buffer_texto);
					escribir_socket (misocket,"\n");
					columna=0;
				}


        //escribir_socket(misocket,items_ayuda[i].nombre_comando);
        //escribir_socket (misocket,"\n");
    }

		//Si ha acabado en medio de columnas
		if (columna!=0) {
			escribir_socket(misocket,buffer_texto);
			escribir_socket (misocket,"\n");
		}

}

//Copia de origen hasta destino finalizado con espacio o 0
void remote_copy_string_spc(char *origen, char *destino)
{

  if (origen!=NULL) {
    while (*origen!=0 && *origen!=32) {
      *destino=*origen;

      destino++;
      origen++;
    }
  }

  *destino=0;
}



char remote_get_raw_source_code_char(int posicion)
{
	if (posicion>remote_tamanyo_archivo_raw_source_code) return 0;
	else return remote_raw_source_code_pointer[posicion];
}

int remote_is_number_or_letter(char c)
{
	if (c>='0' && c<='9') return 1;
	if (c>='A' && c<='Z') return 1;
	if (c>='a' && c<='z') return 1;
	return 0;

}



int remote_string_contains_label(char *string, char *label)
{

	char *coincide;
	coincide=util_strcasestr(string, label);

	if (coincide==string) return 1;
	else return 0;


}

/*
Busca etiqueta en archivo codigo fuente
Consideramos formato:
LXXXXX  (para QL)
LXXXX   (para Z80)
Se hace busqueda sin tener en cuenta mayusculas/minusculas
Retorna numero de linea en parsed source code
Retorna -1 si no encontrado

*/

int remote_find_label_source_code(char *label_to_find)
{
	int linea=0;
	/*
//Puntero a indices en archivo parsed source (lineas sin comentarios, con codigo real)
int *remote_parsed_source_code_indexes_pointer=NULL;
//Tamanyo de ese array
int remote_parsed_source_code_indexes_total;
	*/

	for (linea=0;linea<remote_parsed_source_code_indexes_total;linea++) {
		//Comparar label
		int indice=remote_parsed_source_code_indexes_pointer[linea];
		char *puntero=&remote_raw_source_code_pointer[indice];
		if (remote_string_contains_label(puntero,label_to_find)) {
		     //temp
        	     //printf ("%s\n",puntero);
		     return linea;
		}
	}

	return -1;

}


int remote_is_char_10_or_13(char c)
{
	if (c==10 || c==13) return 1;
	return 0;
}

void remote_load_source_code(int misocket,char *archivo)
{

	if (!si_existe_archivo(archivo)) {
		escribir_socket_format(misocket,"ERROR. File %s not found\n",archivo);
		return;
	}

	remote_tamanyo_archivo_raw_source_code=0;

	//Desasignar memoria si conviene
	if (remote_raw_source_code_pointer!=NULL) free(remote_raw_source_code_pointer);

	//Ver tamanyo archivo
	long int tamanyo;

	tamanyo=get_file_size(archivo);

	remote_raw_source_code_pointer=malloc(tamanyo+1); //y el 0 del final

	if (remote_raw_source_code_pointer==NULL) {
		escribir_socket(misocket,"ERROR. Can not allocate memory to load source code file\n");
		return;
	}

	FILE *ptr_sourcecode;
	int leidos;

	ptr_sourcecode=fopen(archivo,"rb");

	if (ptr_sourcecode==NULL) {
		escribir_socket(misocket,"ERROR. Can not open source code file\n");
		return;
	}

	leidos=fread(remote_raw_source_code_pointer,1,tamanyo,ptr_sourcecode);
	fclose(ptr_sourcecode);

	//El 0 del final
	remote_raw_source_code_pointer[tamanyo]=0;

	if (leidos!=tamanyo) {
		escribir_socket(misocket,"ERROR reading source code file\n");
		return;
	}


	remote_tamanyo_archivo_raw_source_code=tamanyo;


	//Contar maximo de lineas aproximadas. Segun cuantos codigos 10 o 13
	int remote_raw_max_source_code_lineas=1;

	int puntero;
	char c;

	for (puntero=0;puntero<remote_tamanyo_archivo_raw_source_code;puntero++) {
		c=remote_get_raw_source_code_char(puntero);
		if (remote_is_char_10_or_13(c) ) remote_raw_max_source_code_lineas++;
	}

	debug_printf(VERBOSE_DEBUG,"Maximum raw source code lines: %d",remote_raw_max_source_code_lineas);

	//Crear indice a lineas en raw source

	//Asignamos memoria para esos indices
	//Desasignar si conviene
	if (remote_raw_source_code_indexes_pointer!=NULL) {
		debug_printf(VERBOSE_DEBUG,"Freeing previous memory to hold indexes to raw source code file");
		free (remote_raw_source_code_indexes_pointer);
	}

	remote_raw_source_code_indexes_pointer=malloc(sizeof(int)*remote_raw_max_source_code_lineas);

	if (remote_raw_source_code_indexes_pointer==NULL) cpu_panic("Can not allocate memory to index source code file");

	remote_raw_source_code_indexes_total=0;
	//Primera linea
	remote_raw_source_code_indexes_pointer[remote_raw_source_code_indexes_total++]=0;

	int puntero_raw=0;

	while (puntero_raw<remote_tamanyo_archivo_raw_source_code) {
		//Buscar primer codigo 10 o 13
		while (puntero_raw<remote_tamanyo_archivo_raw_source_code && !remote_is_char_10_or_13(remote_get_raw_source_code_char(puntero_raw))) {
			puntero_raw++;
		}

		if (puntero_raw<remote_tamanyo_archivo_raw_source_code) {
			//Buscar siguiente codigo no 10 o 13
			while (puntero_raw<remote_tamanyo_archivo_raw_source_code && remote_is_char_10_or_13(remote_get_raw_source_code_char(puntero_raw))) {
					//Metemos 0 en esa posicion
					remote_raw_source_code_pointer[puntero_raw]=0;
                        		puntero_raw++;
                	}

			if (puntero_raw<remote_tamanyo_archivo_raw_source_code) {
				//Aqui tenemos puntero a siguiente linea
				remote_raw_source_code_indexes_pointer[remote_raw_source_code_indexes_total++]=puntero_raw;
			}
		}
	}

	debug_printf(VERBOSE_DEBUG,"Total effective raw source code lines: %d",remote_raw_source_code_indexes_total);

	//Mostramos cada linea
	int i;
	for (i=0;i<remote_raw_source_code_indexes_total;i++) {
		int indice=remote_raw_source_code_indexes_pointer[i];
		debug_printf (VERBOSE_DEBUG,"Full source line %d : index: %d contents: %s",i,indice,&remote_raw_source_code_pointer[indice]);
	}

	//Crear indice a lineas en source code efectivas (lineas con codigo no comentarios)
	/*

int *remote_parsed_source_code_indexes_pointer=NULL;
int remote_parsed_source_code_indexes_total;

	*/
        //Asignamos memoria para esos indices
        //Desasignar si conviene
        if (remote_parsed_source_code_indexes_pointer!=NULL) {
                debug_printf(VERBOSE_DEBUG,"Freeing previous memory to hold indexes to parsed source code file");
                free (remote_parsed_source_code_indexes_pointer);
        }

        remote_parsed_source_code_indexes_pointer=malloc(sizeof(int)*remote_raw_source_code_indexes_total); //Habra un maximo igual a lineas raw


        if (remote_parsed_source_code_indexes_pointer==NULL) cpu_panic("Can not allocate memory to index source code file parsed");

        remote_parsed_source_code_indexes_total=0;
        //Primera linea
        //remote_parsed_source_code_indexes_pointer[remote_parsed_source_code_indexes_total++]=0;

	//linea leida raw
	int linea_raw=0;

	//Bucle hasta maximo de lineas raw (remote_raw_source_code_indexes_total)
	//Por cada linea, saltar las que no empiezan por alfanumerico (e ignorando primeros espacios o tabs)

	for (linea_raw=0;linea_raw<remote_raw_source_code_indexes_total;linea_raw++) {
		//Saltar primeros espacios o tabs
		int puntero_raw=remote_raw_source_code_indexes_pointer[linea_raw];

		char *texto=&remote_raw_source_code_pointer[puntero_raw];
		while (*texto==' ' || *texto=='\t') {
			texto++;
		}

		if (*texto!=0) {

			//Ver si empieza con alfanumerico
			if (remote_is_number_or_letter(*texto)) {
				remote_parsed_source_code_indexes_pointer[remote_parsed_source_code_indexes_total++]=puntero_raw;
			}

		}
	}

        debug_printf(VERBOSE_DEBUG,"Total effective parsed source code lines: %d",remote_parsed_source_code_indexes_total);

        //Mostramos cada linea
        for (i=0;i<remote_parsed_source_code_indexes_total;i++) {
                int indice=remote_parsed_source_code_indexes_pointer[i];
                debug_printf (VERBOSE_DEBUG,"Parsed source line %d : index: %d contents: %s",i,indice,&remote_raw_source_code_pointer[indice]);
        }


}

void remote_hexdump_internal(int misocket,z80_byte *inicio,int longitud,int offset)
{
	//Formato:
	//000284d0  20 20 20 20 20 4d 4f 56  45 2e 57 20 20 20 44 31  |     MOVE.W   D1|
	//printf ("%p %p\n",inicio,memoria_spectrum);
	inicio +=offset;

	for (;longitud>0;longitud -=16) {
		escribir_socket_format(misocket,"%XH ",offset);
		offset +=16;

		//escribir_socket_format(misocket,"%d ",longitud);
		//sleep(1);
		int longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		z80_byte *puntero_linea;
		puntero_linea=inicio;
		//Volcar 16 bytes hexa
		for (;longitud_parcial>0;longitud_parcial--) {
			escribir_socket_format(misocket,"%02X ",*puntero_linea);
			puntero_linea++;
		}
		//z80_byte c=(z80_byte)*memoria_spectrum;

		//escribir_socket_format(misocket,"%02XH ",*puntero_linea);
		//Llenar los que falten

		if (longitud<16) {
			int espacios=16-longitud;
			for (;espacios;espacios--) escribir_socket(misocket,"   ");
		}

		escribir_socket(misocket," |");


		puntero_linea=(z80_byte *) inicio;

		longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		//Volcar 16 bytes ascii
		for (;longitud_parcial>0;longitud_parcial--) {
			unsigned char c=*puntero_linea;
			if (c<32 || c>127) c='.';
			escribir_socket_format(misocket,"%c",c);
			puntero_linea++;
		}

		escribir_socket(misocket,"|\n");

		inicio +=16;

	}
}



void remote_hexdump(int misocket,int inicio,int longitud)
{
	//Formato:
	//000284d0  20 20 20 20 20 4d 4f 56  45 2e 57 20 20 20 44 31  |     MOVE.W   D1|
	//printf ("%p %p\n",inicio,memoria_spectrum);
	//inicio +=offset;

	menu_debug_set_memory_zone_attr();

	for (;longitud>0;longitud -=16) {
		char buffer_direccion[MAX_LENGTH_ADDRESS_MEMORY_ZONE+1];
		menu_debug_print_address_memory_zone(buffer_direccion,inicio);

		escribir_socket_format(misocket,"%sH ",buffer_direccion);
		//offset +=16;

		//escribir_socket_format(misocket,"%d ",longitud);
		//sleep(1);
		int longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		int puntero_linea;
		puntero_linea=inicio;
		//Volcar 16 bytes hexa
		for (;longitud_parcial>0;longitud_parcial--) {
			//escribir_socket_format(misocket,"%02X ",peek_byte_z80_moto(puntero_linea));
			escribir_socket_format(misocket,"%02X ",menu_debug_get_mapped_byte(puntero_linea));
			puntero_linea++;
		}
		//z80_byte c=(z80_byte)*memoria_spectrum;

		//escribir_socket_format(misocket,"%02XH ",*puntero_linea);
		//Llenar los que falten

		if (longitud<16) {
			int espacios=16-longitud;
			for (;espacios;espacios--) escribir_socket(misocket,"   ");
		}

		escribir_socket(misocket," |");


		puntero_linea=inicio;

		longitud_parcial=longitud;

		if (longitud_parcial>16) longitud_parcial=16;

		//Volcar 16 bytes ascii
		for (;longitud_parcial>0;longitud_parcial--) {
			//unsigned char c=peek_byte_z80_moto(puntero_linea);
			unsigned char c=menu_debug_get_mapped_byte(puntero_linea);
			if (c<32 || c>127) c='.';
			escribir_socket_format(misocket,"%c",c);
			puntero_linea++;
		}

		escribir_socket(misocket,"|\n");

		inicio +=16;

	}
}

//Retorna 0 si no reconocido puntero. Retorna puntero en variable "puntero"
int return_internal_pointer(char *s,z80_byte **puntero)
{

	z80_byte *inicio;
	inicio=NULL;
	int retorno=1;

	if (!strcasecmp(s,"emulator_memory")) 				inicio=memoria_spectrum;
	else if (!strcasecmp(s,"diviface_memory")) 		inicio=diviface_memory_pointer;
	else if (!strcasecmp(s,"rainbow_buffer")) 		inicio=(z80_byte *)rainbow_buffer;
	else if (!strcasecmp(s,"superupgrade_ram")) 	inicio=superupgrade_ram_memory_pointer;
	else if (!strcasecmp(s,"superupgrade_flash")) inicio=superupgrade_rom_memory_pointer;

	else retorno=0;

	*puntero=inicio;

	return retorno;
}


//Retorna puntero a array de clip window, segun 
//tipo : ula|layer2|sprite
//Retorna NULL si hay algun error
z80_byte *remote_return_clipwindow(char *tipo)
{

  /*

z80_byte clip_window_layer2[4];

z80_byte clip_window_sprites[4];

z80_byte clip_window_ula[4];

*/

  if (!strcmp(tipo,"ula")) {
    return clip_window_ula;
  }
  else   if (!strcmp(tipo,"layer2")) {
    return clip_window_layer2;
  }
  else   if (!strcmp(tipo,"sprite")) {
    return clip_window_sprites;
  }

  return NULL;

}

//Retorna puntero a array de paleta, segun 
//tipo : ula|layer2|sprite
//firstsecond: first|second
//Retorna NULL si hay algun error
z80_int *remote_return_palette(char *tipo, char *firstsecond)
{

  int segundo=0;

  if (!strcmp(firstsecond,"first")) segundo=0;
  else if (!strcmp(firstsecond,"second")) segundo=1;
  else return NULL;
  /*
  z80_int tbblue_palette_ula_first[256];
z80_int tbblue_palette_ula_second[256];
z80_int tbblue_palette_layer2_first[256];
z80_int tbblue_palette_layer2_second[256];
z80_int tbblue_palette_sprite_first[256];
z80_int tbblue_palette_sprite_second[256];
*/

  if (!strcmp(tipo,"ula")) {
    if (segundo) return tbblue_palette_ula_second;
    else return tbblue_palette_ula_first;
  }
  else   if (!strcmp(tipo,"layer2")) {
    if (segundo) return tbblue_palette_layer2_second;
    else return tbblue_palette_layer2_first;
  }
  else   if (!strcmp(tipo,"sprite")) {
    if (segundo) return tbblue_palette_sprite_second;
    else return tbblue_palette_sprite_first;
  }

  return NULL;

}


#ifdef EMULATE_VISUALMEM

void remote_visualmem_generic_compact(int misocket, z80_byte *buffer, int final_visualmem)
{

	int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);
	int longitud_linea=0;

	int i;
	for (i=0;i<final_visualmem;i++) {
		//Posicion no 0
		if (buffer[i]) {
			if (longitud_linea==0) {
				//Nueva linea
				escribir_socket_format(misocket,"%0*XH ",digitos_max,i);
			}
			escribir_socket_format(misocket,"%u ",buffer[i]);
			buffer[i]=0;

			longitud_linea++;
			if (longitud_linea==16) {
				escribir_socket_format(misocket,"\n");
				longitud_linea=0;
			}
		}

		else {
			//Posicion 0. Si la linea estaba empezada, salto de linea
			if (longitud_linea) {
				escribir_socket_format(misocket,"\n");
				longitud_linea=0;
			}
		}
	}
}
#endif

//Leer todos los parametros en un solo array de char
//Maximo es 65536*4, para permitir comando largo write-mapped-memory que pueda escribir en 64kb de memoria,
//teniendo en cuenta que un numero como maximo ocupa 3 caracteres + 1 espacio
//Damos 10 bytes de mas de margen por si acaso
//char parametros[MAX_LENGTH_PROTOCOL_COMMAND];


char buffer_lectura_socket[MAX_LENGTH_PROTOCOL_COMMAND];

//para poder repetir comando anterior solo pulsando enter
char buffer_lectura_socket_anterior[MAX_LENGTH_PROTOCOL_COMMAND]="";

char *parametros;

void interpreta_comando(char *comando,int misocket)
{

char buffer_retorno[2048];

	debug_printf (VERBOSE_DEBUG,"Remote command: lenght: %d [%s]",strlen(comando),comando);

	//Si enter y setting de repetir comando anterior solo pulsando enter
	if (  (comando[0]==0  || comando[0]=='\n' || comando[0]=='\r')
		&&
		(remote_debug_settings&16)
		) {
		strcpy(comando,buffer_lectura_socket_anterior);
		debug_printf (VERBOSE_DEBUG,"Repeating last command: lenght: %d [%s]",strlen(comando),comando);
		escribir_socket_format(misocket,"Repeating: %s\n",comando);
	}

	else {
		//Si no, copiar a texto comando anterior el actual
		strcpy(buffer_lectura_socket_anterior,comando);
	}

	//printf ("%d %d %d %d",comando[0],comando[1],comando[2],comando[3]);

	//No hacer caso de comandos con enter, cr, o espacios
	//Si comando nulo, volver
	int i;
	int comando_ignorar=1;

	i=0;
	while (comando[i]) {
		char c=comando[i];
		if (c!=10 && c!=13 && c!=10) comando_ignorar=0;
		i++;
	}

	if (comando_ignorar) return;

	if (comando[0]==0 || (comando[0]==10 && comando[1]==0) ) {
		//debug_printf(VERBOSE_DEBUG,"Command is null");
		//sleep (1);
		return;
	}


    //Interpretar comando hasta espacio o final de linea, o sea sin tener en cuenta parametros
    char comando_sin_parametros[1024];

    for (i=0;comando[i] && comando[i]!=' ' && comando[i]!='\n' && comando[i]!='\r';i++) {
      comando_sin_parametros[i]=comando[i];
    }

    comando_sin_parametros[i]=0;

	parametros=&comando[i];

	//parametros[0]=0;
	int pindex=0;
	if (comando[i]==' ') {
		i++;
		parametros++;
		for (;comando[i] && comando[i]!='\n' && comando[i]!='\r';i++,pindex++) {
			//printf ("searching for end command index: %d\n",i);
			//parametros[pindex]=comando[i];
		}
	}

	parametros[pindex]=0;

	debug_printf (VERBOSE_DEBUG,"Remote command without parameters: lenght: %d [%s]",strlen(comando_sin_parametros),comando_sin_parametros);
	debug_printf (VERBOSE_DEBUG,"Remote command parameters: lenght: %d [%s]",strlen(parametros),parametros);


	//Ver cada comando
	if (!strcmp(comando_sin_parametros,"help") || !strcmp(comando_sin_parametros,"?")) {
		if (parametros[0]==0) {
			escribir_socket(misocket,"Available commands:\n");
			remote_help_no_parameters_command(misocket);


			escribir_socket(misocket,"\nYou can get descriptive help for every command with: help command");

			escribir_socket(misocket,"\nNote: When help shows an argument in brackets [], it means the argument is optional, and when written, "
			                  	"you must not write these brackets\n");
    }

		else remote_help_command(misocket,parametros);
	}


	else if (!strcmp(comando_sin_parametros,"about")) {
		escribir_socket (misocket,"ZEsarUX remote command protocol");
	}

  else if (!strcmp(comando_sin_parametros,"cpu-step") || !strcmp(comando_sin_parametros,"cs")) {
    remote_cpu_step(misocket);
  }

  else if (!strcmp(comando_sin_parametros,"cpu-step-over") || !strcmp(comando_sin_parametros,"cso")) {
    remote_cpu_step_over(misocket);
  }

  else if (!strcmp(comando_sin_parametros,"disable-breakpoint") || !strcmp(comando_sin_parametros,"db")) {
    if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
    else remote_disable_breakpoint(misocket,parametros);
  }

  //disable-breakpoints
    else if (!strcmp(comando_sin_parametros,"disable-breakpoints")) {
      if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. Already disabled");
      else {
              debug_breakpoints_enabled.v=0;
              //debug_cpu_core_loop.v=0;
              breakpoints_disable();
      }
    }


  else if (!strcmp(comando_sin_parametros,"disassemble") || !strcmp(comando_sin_parametros,"d")) {
    unsigned int direccion;
    int lineas=1;
    if (parametros[0]==0) {
      direccion=get_pc_register();
    }
    else {

      direccion=parse_string_to_number(parametros);
      //Ver si hay espacio
      char *s=find_space_or_end(parametros);
      if (*s) lineas=parse_string_to_number(s);

    }
    remote_disassemble(misocket,direccion,lineas,1);

  }

	else if (!strcmp(comando_sin_parametros,"dump-nested-functions")) {
		remote_dump_nested_core_functions(misocket);
	}

  else if (!strcmp(comando_sin_parametros,"easter-egg")) {
    remote_easter_egg(misocket);
  }


  else if (!strcmp(comando_sin_parametros,"enable-breakpoint") || !strcmp(comando_sin_parametros,"eb")) {
    if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
    else remote_enable_breakpoint(misocket,parametros);
  }

  //enable-breakpoints
    else if (!strcmp(comando_sin_parametros,"enable-breakpoints")) {
      if (debug_breakpoints_enabled.v==0) {
              debug_breakpoints_enabled.v=1;
              breakpoints_enable();
      }


      else {
        escribir_socket (misocket,"Error. Already enabled");
      }

    }

  else if (!strcmp(comando_sin_parametros,"enter-cpu-step")) {
    remote_cpu_enter_step(misocket);
  }


  else if (!strcmp(comando_sin_parametros,"evaluate") || !strcmp(comando_sin_parametros,"e")) {
    if (parametros[0]==0) {
      escribir_socket(misocket,"Error. No expression");
    }
    else {
      remote_evaluate(misocket,parametros);
    }
  }




  else if (!strcmp(comando_sin_parametros,"exit-cpu-step") || !strcmp(comando_sin_parametros,"ecs")) {
    remote_cpu_exit_step(misocket);
  }

  else if (!strcmp(comando_sin_parametros,"exit-emulator")) {
		escribir_socket (misocket,"Sayonara baby\n");
		sleep(1);

		remote_calling_end_emulator.v=1;
		end_emulator();
  }


  else if (!strcmp(comando_sin_parametros,"find-label")) {
		if (remote_tamanyo_archivo_raw_source_code==0) {
			escribir_socket (misocket,"ERROR. No source code loaded");
		}
		else {
			int linea=remote_find_label_source_code(parametros);
			if (linea!=-1) {
				int indice=remote_parsed_source_code_indexes_pointer[linea];
        char *puntero=&remote_raw_source_code_pointer[indice];
				escribir_socket (misocket,puntero);
			}
			else {
				escribir_socket (misocket,"ERROR. Not found");
			}
		}
  }


  else if (!strcmp(comando_sin_parametros,"generate-nmi")) {
	generate_nmi();
  }


	else if (!strcmp(comando_sin_parametros,"get-audio-buffer-info")) {
		int tamanyo,posicion;
		audio_get_buffer_info(&tamanyo,&posicion);
		escribir_socket_format(misocket,"Total size: %d Current size: %d\n",tamanyo,posicion);
	}

//get-breakpoints, estado global y lista cada uno, si hay y si esta enabled
  else if (!strcmp(comando_sin_parametros,"get-breakpoints") || !strcmp(comando_sin_parametros,"gb")) {
	int inicio=1;
	int items=MAX_BREAKPOINTS_CONDITIONS;

		remote_parse_commands_argvc(parametros);
                if (remote_command_argc>0) {
			inicio=parse_string_to_number(remote_command_argv[0]);
			if (inicio<1 || inicio>MAX_BREAKPOINTS_CONDITIONS) {
				escribir_socket (misocket,"ERROR. Index out of range");
				return;
			}
			items=1;
                }

                if (remote_command_argc>1) {
			items=parse_string_to_number(remote_command_argv[1]);
                }


    remote_get_breakpoints(misocket,inicio-1,items);
  }

	//get-breakpoints, estado global y lista cada uno, si hay y si esta enabled
	  else if (!strcmp(comando_sin_parametros,"get-breakpointsactions") || !strcmp(comando_sin_parametros,"gba")) {
        int inicio=1;
        int items=MAX_BREAKPOINTS_CONDITIONS;

                remote_parse_commands_argvc(parametros);
                if (remote_command_argc>0) {
                        inicio=parse_string_to_number(remote_command_argv[0]);
			if (inicio<1 || inicio>MAX_BREAKPOINTS_CONDITIONS) {
				escribir_socket (misocket,"ERROR. Index out of range");
				return;
			}
			items=1;
                }

                if (remote_command_argc>1) {
                        items=parse_string_to_number(remote_command_argv[1]);
                }


	    remote_get_breakpointsactions(misocket,inicio-1,items);
	  }

	else if (!strcmp(comando_sin_parametros,"get-buildnumber")) {
                escribir_socket (misocket,BUILDNUMBER);
	}


	else if (!strcmp(comando_sin_parametros,"get-cpu-core-name")) {
		if (cpu_core_loop_name!=NULL) escribir_socket(misocket,cpu_core_loop_name);
	}


  else if (!strcmp(comando_sin_parametros,"get-crc32")) {

    remote_parse_commands_argvc(parametros);

    if (remote_command_argc<2) {
      escribir_socket(misocket,"ERROR. Needs two parameters");
      return;
    }

    /*if (return_internal_pointer(remote_command_argv[0],&inicio)==0) {
      escribir_socket(misocket,"ERROR. Unknown pointer");
      return;
    }*/

    int start_address=parse_string_to_number(remote_command_argv[0]);
    int length=parse_string_to_number(remote_command_argv[1]);

    if (length<1) {
      escribir_socket(misocket,"ERROR. Length must be >0");
      return;
    }


    //Copiar contenido memoria segun memory zone activa a buffer de memoria temporal
    z80_byte *memoria_temporal;
    memoria_temporal=malloc(length);
    if (memoria_temporal==NULL) cpu_panic("Can not allocate memory for crc32 calculation");


    menu_debug_set_memory_zone_attr();
    int longitud_copiar=length;
    int i;

    for (i=0;longitud_copiar>0;i++,longitud_copiar--) { 
      z80_byte byte_leido=menu_debug_get_mapped_byte(start_address+i);
      memoria_temporal[i]=byte_leido;
    }


    //z80_long_int crc32=util_crc32_calculation(memoria_spectrum,16384);
    z80_long_int crc32=util_crc32_calculation(0,memoria_temporal,length);
    escribir_socket_format (misocket,"%08x",crc32);

    free(memoria_temporal);

  }




  else if (!strcmp(comando_sin_parametros,"get-current-machine") || !strcmp(comando_sin_parametros,"gcm")) {
    escribir_socket (misocket,get_machine_name(current_machine_type));
  }

	else if (!strcmp(comando_sin_parametros,"get-current-memory-zone") || !strcmp(comando_sin_parametros,"gcmz")) {
		menu_debug_set_memory_zone_attr();

		char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
		int zone=menu_get_current_memory_zone_name_number(zone_name);
		//machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);

		escribir_socket_format (misocket,"Zone number: %d Name: %s Size: %d (%d KB)", zone,zone_name,
			menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);

	}

	else if (!strcmp(comando_sin_parametros,"get-debug-settings") || !strcmp(comando_sin_parametros,"gds")) {
		sprintf(buffer_retorno,"%d",remote_debug_settings);
		escribir_socket(misocket,buffer_retorno);
	}



	else if (!strcmp(comando_sin_parametros,"get-io-ports") ) {
  	char stats_buffer[MAX_TEXTO_GENERIC_MESSAGE];
		debug_get_ioports(stats_buffer);
		escribir_socket(misocket,stats_buffer);
	}

	else if (!strcmp(comando_sin_parametros,"get-machines")) {
		escribir_socket (misocket,string_machines_list_description);
	}

	else if (!strcmp(comando_sin_parametros,"get-memory-pages") || !strcmp(comando_sin_parametros,"gmp")) {

                remote_parse_commands_argvc(parametros);

                debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
                int total_segmentos=debug_get_memory_pages_extended(segmentos);

                int verbose=0;

                if (remote_command_argc>0) {
                 if (!strcmp(remote_command_argv[0],"verbose")) {
                    verbose=1;
                  }
                }


                 int i;

                for (i=0;i<total_segmentos;i++) { 

                  if (verbose) {
                    escribir_socket_format(misocket,"Segment %d\nLong name: %s\nShort name: %s\nStart: %XH\nEnd: %XH\n\n",
            i+1,segmentos[i].longname, segmentos[i].shortname, segmentos[i].start, segmentos[i].start+segmentos[i].length-1);
                  }

                  else {
                    escribir_socket_format(misocket,"%s ",segmentos[i].shortname);
                  }
                }
		

		//char buffer_temporal[MAX_TEXT_DEBUG_GET_MEMORY_PAGES+1];
		//debug_get_memory_pages(buffer_temporal);
		//escribir_socket (misocket,buffer_temporal);
	}

	else if (!strcmp(comando_sin_parametros,"get-memory-zones") || !strcmp(comando_sin_parametros,"gmz")) {
		remote_get_memory_zones(misocket);
	}



	else if (!strcmp(comando_sin_parametros,"get-ocr")) {
		char buffer_ocr[8192];
		ocr_get_text(buffer_ocr);
		escribir_socket (misocket,buffer_ocr);
	}



	else if (!strcmp(comando_sin_parametros,"get-os")) {
		escribir_socket (misocket,COMPILATION_SYSTEM);
	}

  else if (!strcmp(comando_sin_parametros,"get-paging-state")) {
    char buffer[32];
    debug_get_paging_screen_state(buffer);
    escribir_socket (misocket,buffer);
  }

  else if (!strcmp(comando_sin_parametros,"get-registers") || !strcmp(comando_sin_parametros,"gr")) {
    print_registers(buffer_retorno);
    escribir_socket (misocket,buffer_retorno);
  }

	else if (!strcmp(comando_sin_parametros,"get-stack-backtrace")) {

    int items=5;

    remote_parse_commands_argvc(parametros);
    if (remote_command_argc>0) {
      items=parse_string_to_number(remote_command_argv[0]);
      if (items<1) {
        escribir_socket(misocket,"ERROR. Items must be >0");
        return;
      }
    }

		if (CPU_IS_MOTOROLA) {

		}
		else {
			int i;


			for (i=0;i<items;i++) {
				z80_int valor=peek_byte_z80_moto(reg_sp+i*2)+256*peek_byte_z80_moto(reg_sp+1+i*2);
				escribir_socket_format(misocket,"%04XH ",valor);
			}
		}
	}


	else if (!strcmp(comando_sin_parametros,"get-version")) {
		escribir_socket (misocket,EMULATOR_VERSION);
	}

#ifdef EMULATE_VISUALMEM
	else if (!strcmp(comando_sin_parametros,"get-visualmem-written-dump") || !strcmp(comando_sin_parametros,"gvmwd") ) {

		int salida_compacta=0;
		remote_parse_commands_argvc(parametros);
		if (remote_command_argc>0) {
			if (!strcmp(remote_command_argv[0],"compact")) salida_compacta=1;
		}

		int final_visualmem=65536;
		if (MACHINE_IS_QL) final_visualmem=QL_MEM_LIMIT+1;

		if (salida_compacta) {
			remote_visualmem_generic_compact(misocket,visualmem_buffer,final_visualmem);
			return;
		}

		int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);

		int i;
		for (i=0;i<final_visualmem;i++) {
			if (visualmem_buffer[i]) {
				escribir_socket_format(misocket,"%0*XH %u\n",digitos_max,i,visualmem_buffer[i]);
				clear_visualmembuffer(i);
			}
		}
	}

	else if (!strcmp(comando_sin_parametros,"get-visualmem-read-dump") || !strcmp(comando_sin_parametros,"gvmrd")) {

		int salida_compacta=0;
		remote_parse_commands_argvc(parametros);
		if (remote_command_argc>0) {
			if (!strcmp(remote_command_argv[0],"compact")) salida_compacta=1;
		}

		int final_visualmem=65536;
		if (MACHINE_IS_QL) final_visualmem=QL_MEM_LIMIT+1;

		if (salida_compacta) {
			remote_visualmem_generic_compact(misocket,visualmem_read_buffer,final_visualmem);
			return;
		}

		int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);

		int i;
		for (i=0;i<final_visualmem;i++) {
			if (visualmem_read_buffer[i]) {
				escribir_socket_format(misocket,"%0*XH %u\n",digitos_max,i,visualmem_read_buffer[i]);
				clear_visualmemreadbuffer(i);
			}
		}
	}

	else if (!strcmp(comando_sin_parametros,"get-visualmem-opcode-dump") || !strcmp(comando_sin_parametros,"gvmod")) {

		int salida_compacta=0;
		remote_parse_commands_argvc(parametros);
		if (remote_command_argc>0) {
			if (!strcmp(remote_command_argv[0],"compact")) salida_compacta=1;
		}

		int final_visualmem=65536;
		if (MACHINE_IS_QL) final_visualmem=QL_MEM_LIMIT+1;

		if (salida_compacta) {
			remote_visualmem_generic_compact(misocket,visualmem_opcode_buffer,final_visualmem);
			return;
		}

		int digitos_max=menu_debug_get_total_digits_hexa(final_visualmem-1);

		int i;
		for (i=0;i<final_visualmem;i++) {
			if (visualmem_opcode_buffer[i]) {
				escribir_socket_format(misocket,"%0*XH %u\n",digitos_max,i,visualmem_opcode_buffer[i]);
				clear_visualmemopcodebuffer(i);
			}
		}
	}
#endif






  else if (!strcmp(comando_sin_parametros,"hard-reset-cpu")) {
          hard_reset_cpu();
  }


	else if (!strcmp(comando_sin_parametros,"hexdump") || !strcmp(comando_sin_parametros,"h")) {
		remote_parse_commands_argvc(parametros);

		if (remote_command_argc!=2) {
			escribir_socket(misocket,"ERROR. Needs two parameters");
			return;
		}

		int inicio=parse_string_to_number(remote_command_argv[0]);
		int longitud=parse_string_to_number(remote_command_argv[1]);

		remote_hexdump(misocket,inicio,longitud);

	}

	else if (!strcmp(comando_sin_parametros,"hexdump-internal")) {
		remote_parse_commands_argvc(parametros);

		if (remote_command_argc<2) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}


		z80_byte *inicio;


		if (return_internal_pointer(remote_command_argv[0],&inicio)==0) {
			escribir_socket(misocket,"ERROR. Unknown pointer");
			return;
		}

		if (inicio==NULL) {
			escribir_socket(misocket,"ERROR. Pointer is null");
			return;
		}

		int longitud=parse_string_to_number(remote_command_argv[1]);

		int offset=0;
		if (remote_command_argc>2) offset=parse_string_to_number(remote_command_argv[2]);

		remote_hexdump_internal(misocket,inicio,longitud,offset);

	}

	else if (!strcmp(comando_sin_parametros,"kartusho-press-button")) {
			kartusho_press_button();
	}

	else if (!strcmp(comando_sin_parametros,"load-source-code") || !strcmp(comando_sin_parametros,"lsc")) {
					remote_load_source_code(misocket,parametros);
	}

	else if (!strcmp(comando_sin_parametros,"ls")) {
			remote_simple_help(misocket);
	}

	else if (!strcmp(comando_sin_parametros,"noop")) {
		//No hacer absolutamente nada
	}

	else if (!strcmp(comando_sin_parametros,"read-memory")) {
		unsigned int inicio=0;
		unsigned int longitud=0;

		if (parametros[0]!=0) {

			inicio=parse_string_to_number(parametros);
			longitud=1;
			//Ver si hay espacio
      char *s=find_space_or_end(parametros);
      if (*s) longitud=parse_string_to_number(s);

		}
		remote_get_memory(misocket,inicio,longitud);

	}


	else if (!strcmp(comando_sin_parametros,"read-mapped-memory")) {
		escribir_socket(misocket,"This command is no longer supported. Use read-memory command and set memory zone if needed");
	}

  else if (!strcmp(comando_sin_parametros,"reset-cpu")) {
          reset_cpu();
  }

  else if (!strcmp(comando_sin_parametros,"run") || !strcmp(comando_sin_parametros,"r")) {
    int verbose=0;
    int limit=0;

    int par=0;
	int datosvuelve=1;

    remote_parse_commands_argvc(parametros);

    //ver cada parametro. pueden venir en diferente orden
    for (par=0;par<remote_command_argc;par++) {
      if (!strcasecmp(remote_command_argv[par],"verbose")) verbose=1;
      else if (!strcasecmp(remote_command_argv[par],"no-stop-on-data")) datosvuelve=0;
      else limit=parse_string_to_number(remote_command_argv[par]);
    }


                //if (remote_command_argc!=2) {
                //        escribir_socket(misocket,"ERROR. Needs two parameters");
                //        return;
                //}

                //int inicio=parse_string_to_number(remote_command_argv[0]);
                //int longitud=parse_string_to_number(remote_command_argv[1]);

                //remote_hexdump(misocket,inicio,longitud);

    //if (parametros[0]!=0) verbose=1;
	char texto_evento_data[100];
	if (datosvuelve) strcpy(texto_evento_data,"key press or data sent, ");
	else texto_evento_data[0]=0;

    if (limit==0) escribir_socket_format(misocket,"Running until a breakpoint, %smenu opening or other event\n",texto_evento_data);
    else escribir_socket_format(misocket,"Running until a breakpoint, %smenu opening, %d opcodes run, or other event\n",texto_evento_data,limit);
    remote_cpu_run(misocket,verbose,limit,datosvuelve);
  }


	else if (!strcmp(comando_sin_parametros,"save-binary-internal")) {
		remote_parse_commands_argvc(parametros);

		if (remote_command_argc<3) {
			escribir_socket(misocket,"ERROR. Needs three parameters minimum");
			return;
		}


		z80_byte *inicio;

		if (return_internal_pointer(remote_command_argv[0],&inicio)==0) {
			escribir_socket(misocket,"ERROR. Unknown pointer");
			return;
		}

		if (inicio==NULL) {
			escribir_socket(misocket,"ERROR. Pointer is null");
			return;
		}

		int longitud=parse_string_to_number(remote_command_argv[1]);

		char *binaryfile;

		binaryfile=remote_command_argv[2];

		int offset=0;
		if (remote_command_argc>3) offset=parse_string_to_number(remote_command_argv[3]);

		inicio +=offset;

		FILE *ptr_binaryfile;
				ptr_binaryfile=fopen(binaryfile,"wb");
				if (!ptr_binaryfile)
			{
						debug_printf (VERBOSE_ERR,"Unable to open file");
				}
			else {

							int i;
							for (i=0;i<longitud;i++) {
											fwrite(inicio,1,1,ptr_binaryfile);
											inicio++;
							}

						 fclose(ptr_binaryfile);
			}


	}

	else if (!strcmp(comando_sin_parametros,"send-keys-string")) {
		//remote_parse_commands_argvc(parametros);

		/*if (remote_command_argc<2) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}*/

		z80_byte tecla;

		//No separamos en argv porque interesa tener toda la string a enviar, con sus espacios y todo
		int pausa=parse_string_to_number(parametros);

		char *s=find_space_or_end(parametros);
		if ( (*s)==0) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}

		for (;*s;s++) {
			tecla=*s;
			ascii_to_keyboard_port(tecla);
			usleep(pausa*1000);
			reset_keyboard_ports();
			usleep(pausa*1000);
		}


	}

	else if (!strcmp(comando_sin_parametros,"send-keys-ascii")) {
		remote_parse_commands_argvc(parametros);

		if (remote_command_argc<2) {
			escribir_socket(misocket,"ERROR. Needs two parameters minimum");
			return;
		}

		z80_byte tecla;

		int pausa=parse_string_to_number(remote_command_argv[0]);

		int i;
		for (i=1;i<remote_command_argc;i++) {
			tecla=parse_string_to_number(remote_command_argv[i]);
			//printf ("Enviando tecla %d\n",tecla);
			ascii_to_keyboard_port(tecla);
			usleep(pausa*1000);
			reset_keyboard_ports();
			usleep(pausa*1000);
		}


	}

//set-breakpoint - si breakpoint activo. id numero y texto
else if (!strcmp(comando_sin_parametros,"set-breakpoint") || !strcmp(comando_sin_parametros,"sb")) {
  if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
  else remote_set_breakpoint(misocket,parametros);
}

else if (!strcmp(comando_sin_parametros,"set-breakpointaction") || !strcmp(comando_sin_parametros,"sba")) {
  if (debug_breakpoints_enabled.v==0) escribir_socket (misocket,"Error. You must enable breakpoints first");
  else remote_set_breakpointaction(misocket,parametros);
}

else if (!strcmp(comando_sin_parametros,"set-cr")) {
	enviar_cr=1;
}

else if (!strcmp(comando_sin_parametros,"set-debug-settings") || !strcmp(comando_sin_parametros,"sds")) {
	if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	else remote_debug_settings=parse_string_to_number(parametros);
}

else if (!strcmp(comando_sin_parametros,"set-machine") || !strcmp(comando_sin_parametros,"sm")) {
	if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	else {

		//Entramos en el mismo modo que cpu-step para poder congelar la emulacion
		remote_cpu_enter_step(misocket);
		if (menu_event_remote_protocol_enterstep.v==0) return;

		if (set_machine_type_by_name(parametros)) {
						 //Error.
						escribir_socket_format(misocket,"ERROR. Unknown machine %s",parametros);
		}

		else {
			set_machine(NULL);
			cold_start_cpu_registers();
			reset_cpu();
		}

		remote_cpu_exit_step(misocket);
	}

}

else if (!strcmp(comando_sin_parametros,"set-memory-zone") || !strcmp(comando_sin_parametros,"smz") ) {
	if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	else {

		int readwrite;

		int zone=parse_string_to_number(parametros);

		if (zone==-1) {
			menu_debug_show_memory_zones=0;
			menu_debug_memory_zone=-1;
		}

		else {

			menu_debug_show_memory_zones=1;

			unsigned int size=machine_get_memory_zone_attrib(zone,&readwrite);
			if (size<=0) {
						 //Error.
						escribir_socket_format(misocket,"ERROR. Unknown zone %d",zone);
					}

			else {
				//escribir_socket_format(misocket,"Setting zone to %d\n",zone);
				menu_debug_memory_zone=zone;
			}
		}

	}

}

	else if (!strcmp(comando_sin_parametros,"set-text-brightness")) {
		if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
	        else {
			int bri=parse_string_to_number(parametros);
			if (bri<0 || bri>100) {
				escribir_socket_format(misocket,"ERROR. Invalid brightness value %d",bri);
			}

			else screen_text_brightness=bri;
		}
	}

	else if (!strcmp(comando_sin_parametros,"set-register") || !strcmp(comando_sin_parametros,"sr")) {
		if (debug_change_register(parametros)) {
			escribir_socket (misocket,"Error changing register");
		}
		else {
			print_registers(buffer_retorno);
			escribir_socket (misocket,buffer_retorno);
		}
	}

	else if (!strcmp(comando_sin_parametros,"set-verbose-level")) {
		if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");

		else {
			int i=parse_string_to_number(parametros);

			if (i<0 || i>4) {
				escribir_socket (misocket,"ERROR. Invalid Verbose level");
			}

			else verbose_level=i;
		}
	}

	else if (!strcmp(comando_sin_parametros,"set-window-zoom") ) {
		if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
		else {

			//Entramos en el mismo modo que cpu-step para poder congelar la emulacion
			remote_cpu_enter_step(misocket);
			if (menu_event_remote_protocol_enterstep.v==0) return;

			int z=parse_string_to_number(parametros);

			screen_set_window_zoom(z);


			remote_cpu_exit_step(misocket);
		}

	}

  else if (!strcmp(comando_sin_parametros,"smartload") || !strcmp(comando_sin_parametros,"sl")) {

    //Asegurarnos que congelamos el emulador: abrir menu con mutitarea desactivada
    //Entramos en el mismo modo que cpu-step para poder congelar la emulacion
    remote_cpu_enter_step(misocket);
    if (menu_event_remote_protocol_enterstep.v==0) return;

    //remote_disable_multitask_enter_menu();

    if (quickload(parametros)) {
      escribir_socket (misocket,"Error. Unknown file format");
    }

    //remote_send_esc_close_menu();

    remote_cpu_exit_step(misocket);

  }

  else if (!strcmp(comando_sin_parametros,"snapshot-load") ) {
    //Asegurarnos que congelamos el emulador: abrir menu con mutitarea desactivada
    //Entramos en el mismo modo que cpu-step para poder congelar la emulacion
    remote_cpu_enter_step(misocket);
    if (menu_event_remote_protocol_enterstep.v==0) return;


    strcpy(snapshot_load_file,parametros);
    snapfile=snapshot_load_file;
    snapshot_load();


    remote_cpu_exit_step(misocket);
  }

  else if (!strcmp(comando_sin_parametros,"snapshot-save") ) {
    snapshot_save(parametros);
  }

  else if (!strcmp(comando_sin_parametros,"speech-empty-fifo") ) {
  	//Vaciamos cola speech
  	textspeech_empty_speech_fifo();
  }

  else if (!strcmp(comando_sin_parametros,"speech-send") ) {
  	textspeech_print_speech(parametros);
  }

  else if (!strcmp(comando_sin_parametros,"tbblue-get-clipwindow") ) {

    if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
    else {
      remote_parse_commands_argvc(parametros);
      if (remote_command_argc<1) {
        escribir_socket(misocket,"ERROR. Needs one parameter");
        return;
      }


      z80_byte *clipwindow;
      clipwindow=remote_return_clipwindow(remote_command_argv[0]);
      if (clipwindow==NULL) {
        escribir_socket(misocket,"ERROR. Unknown clip window");
        return;
      }

      int i;
      for (i=0;i<4;i++) {
        escribir_socket_format(misocket,"%d ",clipwindow[i]);
      }

      


    }
  }


  else if (!strcmp(comando_sin_parametros,"tbblue-set-clipwindow") ) {

    if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
    else {
      remote_parse_commands_argvc(parametros);
      if (remote_command_argc<5) {
        escribir_socket(misocket,"ERROR. Needs five parameters");
        return;
      }


      z80_byte *clipwindow;
      clipwindow=remote_return_clipwindow(remote_command_argv[0]);
      if (clipwindow==NULL) {
        escribir_socket(misocket,"ERROR. Unknown clip window");
        return;
      }

      int i;
      for (i=0;i<4;i++) {
        clipwindow[i]=parse_string_to_number(remote_command_argv[1+i]);
      }

      


    }
  }

    else if (!strcmp(comando_sin_parametros,"tbblue-get-palette") ) {

		if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
		else {
			remote_parse_commands_argvc(parametros);
			if (remote_command_argc<3) {
				escribir_socket(misocket,"ERROR. Needs three parameter minimum");
				return;
			}

			z80_byte index=parse_string_to_number(remote_command_argv[2]);

      z80_int *paleta;
      paleta=remote_return_palette(remote_command_argv[0],remote_command_argv[1]);
      if (paleta==NULL) {
        escribir_socket(misocket,"ERROR. Unknown palette");
        return;
      }

			int items=1;
			if (remote_command_argc>3) items=parse_string_to_number(remote_command_argv[3]);
			//if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");

			for (;items;items--) {
				//z80_byte color=tbsprite_palette[index++];  
        z80_int color=paleta[index++];  //primera paleta
				escribir_socket_format(misocket,"%03X ",color);
			}

	        }


    }

        else if (!strcmp(comando_sin_parametros,"tbblue-get-pattern") ) {

                if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
                else {

				remote_parse_commands_argvc(parametros);

		                if (remote_command_argc<1) {
                		        escribir_socket(misocket,"ERROR. Needs one parameter minimum");
		                        return;
                		}

                    int index_int=parse_string_to_number(remote_command_argv[0]);

										int totalitems=1;

										if (remote_command_argc>1) totalitems=parse_string_to_number(remote_command_argv[1]);

                    if (index_int<0 || index_int>=TBBLUE_MAX_PATTERNS) escribir_socket(misocket,"ERROR. Out of range");
                    else {
											for (;totalitems;totalitems--) {
												int i;
												for (i=0;i<256;i++) {
                	         	//z80_byte index_color=tbsprite_patterns[index_int][i];
														 z80_byte index_color=tbsprite_pattern_get_value_index(index_int,i);
	                	        escribir_socket_format(misocket,"%02X ",index_color);
												}
												escribir_socket(misocket,"\n");
												index_int++;
												if (index_int==TBBLUE_MAX_PATTERNS) index_int=0;
											}
                    }
                }

        }


        else if (!strcmp(comando_sin_parametros,"tbblue-get-register") ) {

          if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
          else {
            if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
            else {

               int index=parse_string_to_number(parametros);
               if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");
               else {
                 z80_byte value=tbblue_registers[index];
                 escribir_socket_format(misocket,"%02XH",value);
               }
            }
          }

        }


        else if (!strcmp(comando_sin_parametros,"tbblue-get-sprite") ) {

					if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
					else {

							remote_parse_commands_argvc(parametros);

							if (remote_command_argc<1) {
											escribir_socket(misocket,"ERROR. Needs one parameter minimum");
											return;
							}

							int index_int=parse_string_to_number(remote_command_argv[0]);

							int totalitems=1;

							if (remote_command_argc>1) totalitems=parse_string_to_number(remote_command_argv[1]);

							if (index_int<0 || index_int>=TBBLUE_MAX_SPRITES) escribir_socket(misocket,"ERROR. Out of range");
							else {
								for (;totalitems;totalitems--) {
									int i;
									for (i=0;i<4;i++) {
											z80_byte value_sprite=tbsprite_sprites[index_int][i];
											escribir_socket_format(misocket,"%02X ",value_sprite);
									}
									escribir_socket(misocket,"\n");
									index_int++;
									if (index_int==TBBLUE_MAX_SPRITES) index_int=0;
								}
							}
					}

				}



	else if (!strcmp(comando_sin_parametros,"tbblue-set-palette")) {
		z80_byte index;
		z80_byte valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

    //ula|layer2|sprite first|second index value
		else {

      //remote_copy_string_spc
      char nombrepaleta[100];
      char tipopaleta[100];
      char textoindice[100];

      char *s;
      s=parametros;

      remote_copy_string_spc(s,nombrepaleta);
      s=find_space_or_end(s);

      remote_copy_string_spc(s,tipopaleta);
      s=find_space_or_end(s);

      remote_copy_string_spc(s,textoindice);
      s=find_space_or_end(s);

      //printf ("%s-%s-%s\n",nombrepaleta,tipopaleta,textoindice);

      z80_int *paleta;
      paleta=remote_return_palette(nombrepaleta,tipopaleta);
      if (paleta==NULL) {
        escribir_socket(misocket,"ERROR. Unknown palette");
        return;
      }


			index=parse_string_to_number(textoindice);

			//Ver si hay espacio
			//s=find_space_or_end(parametros);
			while (*s) {
				valor=parse_string_to_number(s);

        paleta[index++]=valor; 

				s=find_space_or_end(s);
			}


		}

	}


	else if (!strcmp(comando_sin_parametros,"tbblue-set-pattern")) {
		int index_int;
		z80_byte valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

		else {

			index_int=parse_string_to_number(parametros);

			if (index_int<0 || index_int>=TBBLUE_MAX_PATTERNS) escribir_socket(misocket,"ERROR. Out of range");

			z80_byte i=0;

			//Ver si hay espacio
			char *s=find_space_or_end(parametros);
			while (*s) {
				valor=parse_string_to_number(s);
				//tbsprite_patterns[index_int][i++]=valor;
				tbsprite_pattern_put_value_index(index_int,i,valor);
				i++;

				s=find_space_or_end(s);
			}


		}

	}

   else if (!strcmp(comando_sin_parametros,"tbblue-set-register") ) {

          if (!MACHINE_IS_TBBLUE) escribir_socket(misocket,"ERROR. Machine is not TBBlue");
          else {

              remote_parse_commands_argvc(parametros);

              if (remote_command_argc<2) {
                      escribir_socket(misocket,"ERROR. Needs two parameters");
                      return;
              }

              int index_int=parse_string_to_number(remote_command_argv[0]);

              int value=parse_string_to_number(remote_command_argv[1]);


              if (index_int<0 || index_int>255 || value<0 || value>255) escribir_socket(misocket,"ERROR. Out of range");
              else {
                //tbblue_set_register_port(index_int);
                //tbblue_set_value_port(value);
								tbblue_set_value_port_position(index_int,value);
              }
          }

  }


	else if (!strcmp(comando_sin_parametros,"tbblue-set-sprite")) {
		int index_int;
		z80_byte valor;
		if (parametros[0]==0) {
			escribir_socket(misocket,"ERROR. No parameters set");
		}

		else {

			index_int=parse_string_to_number(parametros);

			if (index_int<0 || index_int>=TBBLUE_MAX_SPRITES) escribir_socket(misocket,"ERROR. Out of range");

			z80_byte i=0;

			//Ver si hay espacio
			char *s=find_space_or_end(parametros);
			while (*s) {
				valor=parse_string_to_number(s);
				tbsprite_sprites[index_int][i++]=valor;
				if (i==4) i=0;

				s=find_space_or_end(s);
			}


		}

	}


				else if (!strcmp(comando_sin_parametros,"tsconf-get-af-port") ) {

			  	if (!MACHINE_IS_TSCONF) escribir_socket(misocket,"ERROR. Machine is not TSConf");
					else {
						if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
						else {

							 int index=parse_string_to_number(parametros);
							 if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");
							 else {
								 z80_byte value=tsconf_get_af_port(index);
								 escribir_socket_format(misocket,"%02XH",value);
							 }
						}
					}

				}

		

				else if (!strcmp(comando_sin_parametros,"tsconf-set-af-port") ) {

			  	if (!MACHINE_IS_TSCONF) escribir_socket(misocket,"ERROR. Machine is not TSConf");
					else {

              remote_parse_commands_argvc(parametros);

              if (remote_command_argc<2) {
                      escribir_socket(misocket,"ERROR. Needs two parameters");
                      return;
              }

							int index_int=parse_string_to_number(remote_command_argv[0]);

            	int value=parse_string_to_number(remote_command_argv[1]);

							if (index_int<0 || index_int>255 || value<0 || value>255) escribir_socket(misocket,"ERROR. Out of range");

							else {
								tsconf_write_af_port(index_int,value);
							}

						
					}

				}				





        else if (!strcmp(comando_sin_parametros,"quit") || !strcmp(comando_sin_parametros,"exit") || !strcmp(comando_sin_parametros,"logout")) {
          if (menu_event_remote_protocol_enterstep.v) remote_cpu_exit_step(misocket);

          escribir_socket (misocket,"Sayonara baby\n");
					remote_salir_conexion=1;
					sleep(1);

					remote_cerrar_conexion();

        }

	else if (!strcmp(comando_sin_parametros,"view-basic")) {
				char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];
				debug_view_basic(results_buffer);
				escribir_socket(misocket,results_buffer);
	}

/*
como generar volcado hexadecimal de archivo binario:
hexdump -v -e '16/1 "%02xH " " "'
*/
else if (!strcmp(comando_sin_parametros,"write-memory") || !strcmp(comando_sin_parametros,"wm")) {
	unsigned int direccion;
	z80_byte valor;
	if (parametros[0]==0) {
		escribir_socket(misocket,"ERROR. No parameters set");
	}

	else {

		direccion=parse_string_to_number(parametros);

		menu_debug_set_memory_zone_attr();

		//Ver si hay espacio
		char *s=find_space_or_end(parametros);
		while (*s) {
			valor=parse_string_to_number(s);
			//poke_byte_z80_moto(direccion++,valor);
			//printf ("poke addr %X value %x\n",direccion,valor);
			menu_debug_write_mapped_byte(direccion++,valor);

			s=find_space_or_end(s);
		}


	}

}

else if (!strcmp(comando_sin_parametros,"write-mapped-memory") || !strcmp(comando_sin_parametros,"wmm")) {
	escribir_socket(misocket,"This command is no longer supported. Use write-memory command and set memory zone if needed");
}

else if (!strcmp(comando_sin_parametros,"write-mapped-memory-raw") ) {
	escribir_socket(misocket,"This command is no longer supported. Use write-memory-raw command and set memory zone if needed");
}

else if (!strcmp(comando_sin_parametros,"write-memory-raw") ) {
	unsigned int direccion;
	z80_byte valor;
	if (parametros[0]==0) {
		escribir_socket(misocket,"ERROR. No parameters set");
	}

	else {

		direccion=parse_string_to_number(parametros);

		menu_debug_set_memory_zone_attr();

		//Ver si hay espacio
		char *s=find_space_or_end(parametros);
		while (*s) {
			char buffer_valor[4];
			buffer_valor[0]=s[0];
			buffer_valor[1]=s[1];
			buffer_valor[2]='H';
			buffer_valor[3]=0;
			//printf ("%s\n",buffer_valor);
			valor=parse_string_to_number(buffer_valor);
			//printf ("valor: %d\n",valor);
			//poke_byte_z80_moto(direccion++,valor);
			menu_debug_write_mapped_byte(direccion++,valor);

			s++;
			if (*s) s++;
		}
		/*else {
			escribir_socket(misocket,"ERROR. No value set");
		}*/

	}

}

		else if (!strcmp(comando_sin_parametros,"zxevo-get-nvram") ) {

			  	if (!MACHINE_IS_ZXEVO) escribir_socket(misocket,"ERROR. Machine is not ZX-Evo");
					else {
						if (parametros[0]==0) escribir_socket(misocket,"ERROR. No parameter set");
						else {

							 int index=parse_string_to_number(parametros);
							 if (index<0 || index>255) escribir_socket(misocket,"ERROR. Out of range");
							 else {
								 z80_byte value=zxevo_nvram[index];
								 escribir_socket_format(misocket,"%02X",value);
							 }
						}
					}

				}

	else {
		escribir_socket (misocket,"Unknown command");
	}




}

//Retorna 0 si ok
//diferente de 0 si error
int remote_initialize_port(void)
{

	sock_listen=crear_socket_TCP();

//#ifndef MINGW
  if (sock_listen<0) {
    debug_printf (VERBOSE_ERR,"Error creating socket TCP for remote command protocol");
    return 1;
  }

	//Evitar errores "Unable to bind: Address already in use" cuando se cierra el emulador con una conexion en curso
	int opcion_socket=1;

	setsockopt(sock_listen,SOL_SOCKET,SO_REUSEADDR,&opcion_socket,sizeof(int));

//#endif


	int result=assignar_adr_internet(sock_listen,NULL,remote_protocol_port);

//#ifndef MINGW
  if (result<0) {
    debug_printf(VERBOSE_ERR,"Error allocating socket address for remote command protocol");
    return 1;
  }
//#endif

	result=listen(sock_listen,1);

//#ifndef MINGW
  if (result<0) {
    debug_printf (VERBOSE_ERR,"Error running listen for remote command protocol");
    return 1;
  }
//#endif


  return 0;
}

void *thread_remote_protocol_function(void *UNUSED(nada))
{
        /*while (1) {
                sleep(1);

                printf ("aqui estoy en el pthread remote\n");

        }*/

        if (remote_initialize_port()) return NULL;

				//printf ("Listening for connections in port %u\n",remote_protocol_port);

				fflush(stdout);

				while (1)
				{
				  long_adr=sizeof(adr);



					if (!remote_protocol_ended.v) {
						//printf ("ANTES accept\n");
						sock_conectat=accept(sock_listen,(struct sockaddr *)&adr,&long_adr);
					}

					else {
						//Si ha finalizado, volver
						return NULL;
					}


					if (sock_conectat<0) {
					  debug_printf (VERBOSE_DEBUG,"Remote command. Error running accept");
						//Esto se dispara cuando desactivamos el protocolo desde el menu, y no queremos que salte error
						//Aunque tambien se puede dar en otros momentos por culpa de fallos de conexion, no queremos que moleste al usuario
						//como mucho lo mostramos en verbose debug

						remote_salir_conexion=1;
						sleep(1);
					}


				else {

					debug_printf (VERBOSE_DEBUG,"Received remote command connection petition");

				          //Enviamos mensaje bienvenida
				          //char bienvenida[1024];
				          //sprintf (bienvenida,"%s","Welcome to ZEsarUX remote protocol\n");
				          //write(sock_conectat,bienvenida,strlen(bienvenida));
                  escribir_socket(sock_conectat,"Welcome to ZEsarUX remote command protocol (ZRCP)\nWrite help for available commands\n");

          //if ( (write(sock,buffer,longitut)) <0 ) return -1;

					remote_salir_conexion=0;

					while (!remote_salir_conexion) {

						char prompt[1024];
            if (menu_event_remote_protocol_enterstep.v) sprintf (prompt,"%s","\ncommand@cpu-step> ");
						else sprintf (prompt,"%s","\ncommand> ");
						if (escribir_socket(sock_conectat,prompt)<0) remote_salir_conexion=1;

						int indice_destino=0;

						if (!remote_salir_conexion) {

							//Leer socket

							//int leidos = read(sock_conectat, buffer_lectura_socket, 1023);
							int leidos;
							int salir_bucle=0;
							do {
								leidos=leer_socket(sock_conectat, &buffer_lectura_socket[indice_destino], MAX_LENGTH_PROTOCOL_COMMAND-1);
								debug_printf (VERBOSE_DEBUG,"Read block %d bytes index: %d",leidos,indice_destino);

								/*
								RETURN VALUES
     These calls return the number of bytes received, or -1 if an error occurred.

     For TCP sockets, the return value 0 means the peer has closed its half side of the connection.
		 						*/

								if (leidos==0) remote_salir_conexion=1;

								if (leidos>0) {

									//temp debug
									/*int j;
									for (j=0;j<leidos;j++) {
										unsigned char letra=buffer_lectura_socket[indice_destino+j];
										if (letra<32 || letra>127) printf ("%02XH\n",letra);
										else printf ("%c\n",letra);
									}*/

									indice_destino +=leidos;
									//Si acaba con final de string, salir



									//printf ("%d %d %d %d\n",buffer_lectura_socket[0],buffer_lectura_socket[1],buffer_lectura_socket[2],buffer_lectura_socket[3]);

									if (buffer_lectura_socket[indice_destino-1]=='\r' || buffer_lectura_socket[indice_destino-1]=='\n' || buffer_lectura_socket[indice_destino-1]==0) {
										//printf ("salir\n");
										salir_bucle=1;
									}
								}


							} while (leidos>0 && salir_bucle==0);


							//if (leidos) {
								buffer_lectura_socket[indice_destino]=0;
								debug_printf (VERBOSE_DEBUG,"Remote command. Lenght Read text: %d",indice_destino);

								//int j;
								//for (j=0;buffer_lectura_socket[j];j++) printf ("%d %c\n",j,buffer_lectura_socket[j]);

								//temp
								//if (indice_destino> DEBUG_MAX_MESSAGE_LENGTH-100)verbose_level=0;

								//Esto probablemente petara el emulador si el verbose level esta al menos 3 y el comando recibido excede 1024... esto es
								//porque el buffer de texto en la funcion debug_printf es precisamente de 1024
								//Entonces por ejemplo el comando write-mapped-memory si se le envia una secuencia de mas de 1024 caracteres, petara en este caso
								debug_printf (VERBOSE_DEBUG,"Remote command. Read text: [%s]",buffer_lectura_socket);

								interpreta_comando(buffer_lectura_socket,sock_conectat);
							//}

						}

					}
			  	}
					//printf ("remote_salir_conexion: %d\n",remote_salir_conexion);
					debug_printf (VERBOSE_DEBUG,"Remote command. Exiting connection");

					//Salir del modo step si estaba activado
					if (menu_event_remote_protocol_enterstep.v) remote_cpu_exit_step_continue();
				}

				//printf ("despues de bucle aqui no se llega nunca\n");

        return NULL;
}

void init_remote_protocol(void)
{

  if (remote_protocol_enabled.v==0) return;

	debug_printf (VERBOSE_INFO,"Starting remote protocol listener on port %d",remote_protocol_port);
	thread_remote_inicializado.v=0;

	if (pthread_create( &thread_remote_protocol, NULL, &thread_remote_protocol_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create remote protocol pthread");
	}


	thread_remote_inicializado.v=1;
	remote_protocol_ended.v=0;


}

void end_remote_protocol(void)
{

  if (remote_protocol_enabled.v==0) return;
  if (thread_remote_inicializado.v==0) return;

	debug_printf(VERBOSE_INFO,"Ending remote command protocol listener");

	remote_salir_conexion=1;
	remote_protocol_ended.v=1;

	remote_cerrar_conexion();
#ifdef MINGW
	closesocket(sock_listen);
	WSACleanup();
#else
	close(sock_listen);
#endif



  pthread_cancel(thread_remote_protocol);

}

#else

//No hacer nada, no hay pthreads disponibles
void init_remote_protocol(void)
{
	if (remote_protocol_enabled.v==0) return;
	debug_printf(VERBOSE_INFO,"Not enabling remote protocol because pthreads support is not compiled");
}

void end_remote_protocol(void)
{

}

#endif
