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

#ifndef NETWORK_H
#define NETWORK_H

#include "compileoptions.h"

#ifdef USE_PTHREADS
	#include <pthread.h>
#endif


#include <sys/types.h>

#ifdef MINGW
//	#include <winsock2.h>
// Si lo activamos aqui, da error al compilar menu.h con:
// including winsock2.h after winsock.h is unsupported
// se supone que lo carga antes en otro sitio...
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#endif


#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

//Maxima longitud de una url (valor inventado por mi)
#define NETWORK_MAX_URL 1024


//por defecto 1 mb
#define MAX_ZSOCK_HTTP_BUFFER 1*1024*1024

extern int enviar_cr;

extern int crear_socket_TCP(void);
extern int escribir_socket(int socket, char *buffer);

extern int omplir_adr_internet(struct sockaddr_in *adr,char *host,unsigned short n_port);
extern int connectar_socket(int s,struct sockaddr_in *adr);

extern int leer_socket(int s, char *buffer, int longitud);
extern int escribir_socket_format (int misocket, const char * format , ...);
extern int assignar_adr_internet(int sock,char *host,unsigned short n_port);
extern int cerrar_socket(int s);
extern void init_network_tables(void);

extern int z_sock_open_connection(char *host,int port,int use_ssl,char *ssl_sni_host_name);
extern int z_sock_close_connection(int indice_tabla);
extern int z_sock_free_connection(int indice_tabla);
extern int z_sock_read(int indice_tabla, z80_byte *buffer, int longitud);
extern int z_sock_write_string(int indice_tabla, char *buffer);
extern int zsock_wait_until_command_prompt(int indice_tabla);
extern int zsock_read_all_until_command(int indice_tabla,z80_byte *buffer,int max_buffer,int *posicion_command);
extern int zsock_read_all_until_command_max_reintentos(int indice_tabla,z80_byte *buffer,int max_buffer,int *posicion_command,int max_reintentos);
extern int zsock_read_all_until_newline(int indice_tabla,z80_byte *buffer,int max_buffer,int *posicion_command);
extern int get_socket_number(int indice_tabla);
extern int zsock_available_data(int socket);

extern int zsock_http(char *host, char *url,int *http_code,char **mem,int *t_leidos, char **mem_after_headers,int skip_headers,
    char *add_headers,int use_ssl,char *redirect_url,int estimated_maximum_size, char *ssl_sni_host_name);

extern char *z_sock_get_error(int error);
extern int z_sock_assign_socket(void);
extern void omplir_adr_internet_semaforo_init(void);

extern unsigned int network_traffic_counter_read;
extern unsigned int network_traffic_counter_write;

//el -1 es un error generico
#define Z_ERR_NUM_TCP_SOCK -2
#define Z_ERR_NUM_HOST_NOT_FOUND -3
#define Z_ERR_NUM_STA_CONN -4
#define Z_ERR_NUM_SSL_UNAVAIL -5
#define Z_ERR_NUM_SOCK_NOT_OPEN -6
#define Z_ERR_NUM_WRITE_SOCKET -7
#define Z_ERR_NUM_READ_SOCKET -8
#define Z_ERR_NUM_TOO_MANY_SOCKETS -9

#define ZSOCK_HTTP_DEFAULT_MINIMUM_SEGMENTS 500

#endif
