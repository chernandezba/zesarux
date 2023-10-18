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

//Para uso de funciones va_
#include <stdarg.h>


#ifdef MINGW
	#include <winsock2.h>
#endif



#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "network.h"
#include "compileoptions.h"
#include "chardevice.h"
#include "atomic.h"


/*
Estos ya vienen de network.h
#ifdef USE_PTHREADS
	#include <pthread.h>
#endif


#include <sys/types.h>

#ifdef MINGW
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#endif
*/


//Inicio includes SSL
#ifdef COMPILE_SSL

#include <openssl/ssl.h>
#include <openssl/err.h>

#endif


//Estructura para guardar sockets
#define MAX_Z_SOCKETS 30

struct s_z_sockets_struct {
	int used;
	struct sockaddr_in adr;
	int socket_number;
	z80_bit use_ssl;

#ifdef COMPILE_SSL
	SSL_CTX *ssl_ctx;
	SSL *ssl_conn;
    //handle server name indication (SNI)
    char ssl_sni_hostname[NETWORK_MAX_URL];
#endif

};


//funciones zsock no permito usarlas sin pthreads. Por que? Porque no tiene sentido realizar funciones de red si no se pueden meter
//las funciones en background, que se quedaria el emulador congelado al usar el speccy browser (por ejemplo, aunque seria un mal menor),
//o al usar ZENG (ahi es imposible usarlo si no hay threads, como todo ZRCP). Al final se trata de simplificar las rutinas:
//Si no hay soporte de threads, no hay funciones de red. Y punto


typedef struct s_z_sockets_struct z_sockets_struct;

//array de sockets
z_sockets_struct sockets_list[MAX_Z_SOCKETS];



//Mensajes de error de red

char *z_err_msg_generic="Network error";
char *z_err_msg_tcp_sock="Can't create TCP socket";
char *z_err_msg_host_not_found="Host not found";
char *z_err_msg_sta_conn="Error establishing connection with destination";
char *z_err_msg_ssl_unavail="SSL requested but ssl libraries unavailable";
char *z_err_msg_sock_not_open="Socket is not open";
char *z_err_msg_write_socket="Error writing to socket";
char *z_err_msg_read_socket="Error reading from socket";
char *z_err_msg_too_many_sockets="Too many open sockets";

char *z_sock_get_error(int error)
{
	switch (error) {
		case Z_ERR_NUM_TCP_SOCK:
			return z_err_msg_tcp_sock;
		break;

		case Z_ERR_NUM_HOST_NOT_FOUND:
			return z_err_msg_host_not_found;
		break;

		case Z_ERR_NUM_STA_CONN:
			return z_err_msg_sta_conn;
		break;

		case Z_ERR_NUM_SSL_UNAVAIL:
			return z_err_msg_ssl_unavail;
		break;

		case Z_ERR_NUM_SOCK_NOT_OPEN:
        	return z_err_msg_sock_not_open;
		break;

		case Z_ERR_NUM_WRITE_SOCKET:
			return z_err_msg_write_socket;
		break;

		case Z_ERR_NUM_READ_SOCKET:
			return z_err_msg_read_socket;
		break;

		case Z_ERR_NUM_TOO_MANY_SOCKETS:
			return z_err_msg_too_many_sockets;
		break;

		default:
			return z_err_msg_generic;
		break;
	}
}

void z_sock_increment_traffic_counter_read(int total_read);
void z_sock_increment_traffic_counter_write(int total_write);


//Inicio funciones SSL
#ifdef COMPILE_SSL


void z_init_ssl(void)
{
	SSL_load_error_strings ();
	SSL_library_init ();
}


int z_connect_ssl(int indice_tabla)
{
	debug_printf (VERBOSE_DEBUG,"Connecting SSL");

	debug_printf (VERBOSE_DEBUG,"SSL_CTX_new");
	sockets_list[indice_tabla].ssl_ctx = SSL_CTX_new (SSLv23_client_method ());
    //sockets_list[indice_tabla].ssl_ctx = SSL_CTX_new (TLSv1_client_method() );

	// create an SSL connection and attach it to the socket
	sockets_list[indice_tabla].ssl_conn = SSL_new(sockets_list[indice_tabla].ssl_ctx);

	if (sockets_list[indice_tabla].ssl_conn==NULL) {
		debug_printf (VERBOSE_DEBUG,"Error creating SSL context");

		//mostrar error
		//ERR_print_errors_fp(stderr);

		return -1;
	}

	debug_printf (VERBOSE_DEBUG,"SSL_set_fd %d %d",sockets_list[indice_tabla].ssl_conn,sockets_list[indice_tabla].socket_number);
	SSL_set_fd(sockets_list[indice_tabla].ssl_conn, sockets_list[indice_tabla].socket_number);

	// perform the SSL/TLS handshake with the server - when on the
	// server side, this would use SSL_accept()

    //Activate SNI if needed
    debug_printf(VERBOSE_DEBUG,"ssl_sni_hostname: [%s]",sockets_list[indice_tabla].ssl_sni_hostname);
    if (sockets_list[indice_tabla].ssl_sni_hostname[0]!=0) {
        int resultado_sni = SSL_set_tlsext_host_name(sockets_list[indice_tabla].ssl_conn,sockets_list[indice_tabla].ssl_sni_hostname);
        if(resultado_sni!=1) {
            debug_printf (VERBOSE_DEBUG,"Error SSL_set_tlsext_host_name");
            //printf ("Error SSL_set_tlsext_host_name\n");
            return -1;
        }
    }

	debug_printf (VERBOSE_DEBUG,"Running SSL_connect");
    	ERR_clear_error();
	int err = SSL_connect(sockets_list[indice_tabla].ssl_conn);
	if (err != 1) {
        //debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d",err);
        int final_err=SSL_get_error(sockets_list[indice_tabla].ssl_conn,err);
        switch(final_err) {
            case SSL_ERROR_NONE:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_NONE error",final_err);
            break;

            case SSL_ERROR_ZERO_RETURN:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_ZERO_RETURN error",final_err);
            break;

            case SSL_ERROR_WANT_READ:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_WANT_READ error",final_err);
            break;

            case SSL_ERROR_WANT_WRITE:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_WANT_WRITE error",final_err);
            break;

            case SSL_ERROR_WANT_CONNECT:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_WANT_CONNECT error",final_err);
            break;

            case SSL_ERROR_WANT_ACCEPT:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_WANT_ACCEPT error",final_err);
            break;

            case SSL_ERROR_WANT_X509_LOOKUP:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_WANT_X509_LOOKUP error",final_err);
            break;

            case SSL_ERROR_SYSCALL:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_SYSCALL error",final_err);
            break;

            case SSL_ERROR_SSL:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. SSL_ERROR_SSL error",final_err);
                int final_ssl_err=1;

                //por si acaso, doy un maximo de iteraciones
                int current_line=0;
                while (final_ssl_err!=0 && current_line<10) {
                    final_ssl_err=ERR_get_error();
                    debug_printf(VERBOSE_DEBUG,"SSL error: %s",ERR_error_string(final_ssl_err,NULL));
                    current_line++;
                }
            break;

            default:
                debug_printf(VERBOSE_DEBUG,"ERROR SSL_connect: %d. Unknown error",final_err);
            break;
        }


   		return -1;
	}

	return 0;
}

void z_disconnect_ssl(int indice_tabla)
{
	debug_printf (VERBOSE_DEBUG,"Disconnecting SSL");

	debug_printf (VERBOSE_DEBUG,"SSL_CTX_free");
	SSL_CTX_free(sockets_list[indice_tabla].ssl_ctx);

}



#endif
//Fin funciones SSL







#ifndef NETWORKING_DISABLED


//Si se envia CR despues de cada sentencia de escritura
int enviar_cr=0;


//Crea un socket TCP per la connexio en xarxa
int crear_socket_TCP(void)
{
	#ifdef MINGW
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(1,1), &wsadata) == SOCKET_ERROR) {
		return Z_ERR_NUM_TCP_SOCK;
	}
	#endif


    return socket(PF_INET,SOCK_STREAM,0);
}

z_atomic_semaphore omplir_adr_internet_semaforo;

void omplir_adr_internet_semaforo_init(void)
{
	z_atomic_reset(&omplir_adr_internet_semaforo);
}



int omplir_adr_internet(struct sockaddr_in *adr,char *host,unsigned short n_port)
{

	//Adquirir lock
	//Parece ser que si aqui se entra simultaneamente desde varios sitios,
	//como h->h_addr tiene mismo valor cuando host es el mismo (ejemplo 51.83.33.13 al
	//enviar estadisticas, comprobar update y obtener usuarios de ayer),
	//si se acceden todos a la vez acaba petando la sentencia
	//adr->sin_addr=*(struct in_addr *)h->h_addr;
	//con segfault
	//Parece que con este semaforo ya no peta
	//El por qué real de esto lo desconozco,
	//está claro que si se lanzasen montones de acciones por red simultaneamente, el uso
	//de este bloqueo no seria para nada eficiente
	while(z_atomic_test_and_set(&omplir_adr_internet_semaforo)) {
		//printf("Esperando a liberar lock en omplir_adr_internet_semaforo\n");
	}

        struct hostent *h;

        adr->sin_family=AF_INET;
        if (host!=NULL) {


			h=gethostbyname(host);

			if (h==NULL) {

				//Liberar lock
				z_atomic_reset(&omplir_adr_internet_semaforo);
				return -1;
			}


                adr->sin_addr=*(struct in_addr *)h->h_addr;

                //printf ("\nncdd_util: %s : %d = %lX\n",host,(int)n_port,(unsigned long)adr->sin_addr.s_addr);

        }
        else {
                adr->sin_addr.s_addr=htonl(INADDR_ANY);
  }
        adr->sin_port=htons(n_port);

	//Liberar lock
	z_atomic_reset(&omplir_adr_internet_semaforo);

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





//Retorna bytes escritos
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
			 return Z_ERR_NUM_WRITE_SOCKET;
	 }
	 if (efectivo_enviar_cr) send(socket,&cr,1,0);

     z_sock_increment_traffic_counter_write(smsg);
	 return smsg;

#else

    //printf("before write\n");
	int escrito=write(socket,buffer,strlen(buffer));
    //printf("after write\n");

	if (efectivo_enviar_cr) write(socket,&cr,1);

    z_sock_increment_traffic_counter_write(escrito);

	return escrito;

#endif

}

//int leidos = read(sock_conectat, buffer_lectura_socket, 1023);
int leer_socket(int s, char *buffer, int longitud)
{


#ifdef MINGW

int leidos=recv(s,buffer,longitud,0);
 if(leidos==SOCKET_ERROR){
		return Z_ERR_NUM_READ_SOCKET;
 }

 z_sock_increment_traffic_counter_read(leidos);
 return leidos;

#else
	//int leidos=read(s, buffer, longitud);
	int leidos=recv(s,buffer,longitud,0);
	//printf ("leidos en leer_socket: %d\n",leidos);
    z_sock_increment_traffic_counter_read(leidos);
	return leidos;

#endif
}


int connectar_socket(int s,struct sockaddr_in *adr)
{
	//TODO: como funciona esto en Windows?
	int retorno=connect(s,(struct sockaddr *)adr,sizeof(struct sockaddr_in));

	//El error ya lo muestra zsock_open_connection, que es el unico sitio que lo usa
    //if (retorno<0) {
    //    debug_printf (VERBOSE_ERR,"Error stablishing connection with host");
    //}

	return retorno;

}

int cerrar_socket(int s)
{


#ifdef MINGW
	return closesocket(s);
	//desactivo esto ya que esto implica que no se va a usar mas los windows sockets, cosa no cierta (se pueden usar en zeng por ejemplo)
	//ademas no estamos llamando a WSAStartup al inicio
	//Se deberia hacer el WSACleanup al finalizar el emulador
	//WSACleanup();
#else
	return close(s);
#endif

}


int zsock_available_data(int socket)
{
			//TODO: en windows siempre retorna datos disponibles. lo cual seria un problema por que si no hay datos,
			//la conexion se queda en read colgada

//-En Windows para saber si un socket tiene datos para leer,
//metemos primero el socket en modo no bloqueante,
//luego usar llamada a read con 0 bytes y devolverá error si no hay bytes para leer
//Es la mejor manera esta? No estoy seguro, pero funciona

#ifdef MINGW
//En Windows
int longitud_leer=0;

char buffer;


int iResult;
u_long iMode = 1; //no bloquear

iResult = ioctlsocket(socket, FIONBIO, &iMode);
//if (iResult != NO_ERROR) {
  //printf("ioctlsocket failed with error: %ld\n", iResult);
//}



int leidos=recv(socket,&buffer,longitud_leer,0);

iMode = 0; //bloquear, como por defecto

iResult = ioctlsocket(socket, FIONBIO, &iMode);
//if (iResult != NO_ERROR) {
  //printf("ioctlsocket failed with error: %ld\n", iResult);
//}



 if(leidos==SOCKET_ERROR){
		return 0;
 }
 return 1;



#else
//En Unix

	if (chardevice_status(socket) & CHDEV_ST_RD_AVAIL_DATA) return 1;
	else return 0;
#endif
}



//Fin de funciones CON pthreads
#else



//No hacer nada, no hay pthreads disponibles
//funciones nulas de socket. Nadie deberia usarlas si no hay pthreads pero por si acaso...



//Crea un socket TCP per la connexio en xarxa
int crear_socket_TCP(void)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;
}


int omplir_adr_internet(struct sockaddr_in *adr GCC_UNUSED,char *host GCC_UNUSED,unsigned short n_port GCC_UNUSED)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;

}

//Assignar l'adre<E7>a al socket
//Host ha de valer NULL si es tracta del servidor
int assignar_adr_internet(int sock GCC_UNUSED,char *host GCC_UNUSED,unsigned short n_port GCC_UNUSED)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;
}



int escribir_socket(int socket GCC_UNUSED, char *buffer GCC_UNUSED)
{

	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;

}

//int leidos = read(sock_conectat, buffer_lectura_socket, 1023);
int leer_socket(int s GCC_UNUSED, char *buffer GCC_UNUSED, int longitud GCC_UNUSED)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;
}



int connectar_socket(int s GCC_UNUSED,struct sockaddr_in *adr GCC_UNUSED)
{

	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;

}


int cerrar_socket(int s GCC_UNUSED)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;
}


int zsock_available_data(int socket GCC_UNUSED)
{
	//Esto no se usara sin pthreads pero bueno, retornamos 1
	return 1;
}





#endif
//Fin de funciones SIN pthreads



//Funciones (pocas) que no necesitan pthreads

int escribir_socket_format(int misocket, const char * format , ...)
{

    char buffer_final[4096];

    va_list args;
    va_start (args, format);
    vsprintf (buffer_final,format, args);
    va_end (args);

    return escribir_socket(misocket,buffer_final);
}


z_atomic_semaphore network_semaforo;

//Semaforo y contadores para llevar el total de trafico leido/escrito
z_atomic_semaphore network_traffic_counter_semaforo;
unsigned int network_traffic_counter_read=0;
unsigned int network_traffic_counter_write=0;

void init_network_tables(void)
{
	int i;
	for (i=0;i<MAX_Z_SOCKETS;i++) {
		sockets_list[i].used=0;
		sockets_list[i].use_ssl.v=0;

#ifdef COMPILE_SSL
    //Y por si acaso ponemos el ssl_sni_host_name a vacio, por si hay alguna llamada en que se le olvida indicarlo
    sockets_list[i].ssl_sni_hostname[0]=0;
#endif


	}

	z_atomic_reset(&network_semaforo);
    z_atomic_reset(&network_traffic_counter_semaforo);

#ifdef COMPILE_SSL
	z_init_ssl();
#endif

}

void z_sock_increment_traffic_counter_read(int total_read)
{
    //Si <0 sera algun error al leer/escribir
    if (total_read<=0) return;

	//Adquirir lock
	while(z_atomic_test_and_set(&network_traffic_counter_semaforo)) {
		//printf("Esperando a liberar lock en network_traffic_counter_semaforo en z_sock_increment_traffic_counter_read\n");
	}

	network_traffic_counter_read +=total_read;

	//Liberar lock
	z_atomic_reset(&network_traffic_counter_semaforo);

}

void z_sock_increment_traffic_counter_write(int total_write)
{

    //Si <0 sera algun error al leer/escribir
    if (total_write<=0) return;

	//Adquirir lock
	while(z_atomic_test_and_set(&network_traffic_counter_semaforo)) {
		//printf("Esperando a liberar lock en network_traffic_counter_semaforo en z_sock_increment_traffic_counter_write\n");
	}

	network_traffic_counter_write +=total_write;

	//Liberar lock
	z_atomic_reset(&network_traffic_counter_semaforo);

}

int find_free_socket(void)
{
	int i;
	for (i=0;i<MAX_Z_SOCKETS;i++) {
		if (sockets_list[i].used==0) {
			debug_printf (VERBOSE_PARANOID,"Found free socket at index %d",i);
			return i;
		}
	}

	return -1;
}

int z_sock_assign_socket(void)
{
	//Adquirir lock
	while(z_atomic_test_and_set(&network_semaforo)) {
		//printf("Esperando a liberar lock en network_semaforo en z_sock_assign_socket\n");
	}

	int indice_tabla=find_free_socket();
	if (indice_tabla<0) {
		//Dado que es un error grave, que lo muestre como error y por ventana. Tambien retornamos numero error
		debug_printf(VERBOSE_ERR,"Too many ZEsarUX open sockets (%d)",MAX_Z_SOCKETS);
		indice_tabla=Z_ERR_NUM_TOO_MANY_SOCKETS;
	}
	else {
		//Asignar el socket
		sockets_list[indice_tabla].used=1;
		//Y de momento con numero socket invalido
		sockets_list[indice_tabla].socket_number=-1;
	}

	//Liberar lock
	z_atomic_reset(&network_semaforo);

	return indice_tabla;
}


/*
Supuestamente usaria esto en z_sock_close_connection pero dado que ahi solo hay que decir que el socket esta disponible, se puede modificar tal cual,
no hace falta lock
void z_sock_free_socket(int indice_tabla)
{

	//Adquirir lock
	while(z_atomic_test_and_set(&network_semaforo)) {
		//printf("Esperando a liberar lock en network_semaforo en z_sock_free_socket\n");
	}

	sockets_list[indice_tabla].used=0;

	//Liberar lock
	z_atomic_reset(&network_semaforo);


}
*/

//Retorna indice a la tabla de sockets. <0 si error
//si sni_host_name es "", no usar
int z_sock_open_connection(char *host,int port,int use_ssl,char *ssl_sni_host_name)
{


	int indice_tabla=z_sock_assign_socket();
	if (indice_tabla<0) {
		return -1;
	}

	int test_socket;

	//Aqui ya se ha asignado socket en la lista. Si hay error posterior, liberar dicho socket
	int error=0;
	int error_num=-1;

	if ((test_socket=crear_socket_TCP())<0) {
		error=1;
		error_num=Z_ERR_NUM_TCP_SOCK;
    }

	else {

        if (omplir_adr_internet(&sockets_list[indice_tabla].adr,host,port)<0) {
                error=1;
				error_num=Z_ERR_NUM_HOST_NOT_FOUND;
        }

		else {

			if (connectar_socket(test_socket,&sockets_list[indice_tabla].adr)<0) {
					debug_printf(VERBOSE_DEBUG,"%s: %s:%d",z_sock_get_error(Z_ERR_NUM_STA_CONN),host,port);
					error=1;
					error_num=Z_ERR_NUM_STA_CONN;
        	}
		}
	}

	if (error) {
		sockets_list[indice_tabla].used=0;
		return error_num;
	}

	sockets_list[indice_tabla].socket_number=test_socket;

	sockets_list[indice_tabla].use_ssl.v=use_ssl;

	if (use_ssl) {

#ifdef COMPILE_SSL
        strcpy(sockets_list[indice_tabla].ssl_sni_hostname,ssl_sni_host_name);
		int ret=z_connect_ssl(indice_tabla);
		if (ret!=0) {
			debug_printf (VERBOSE_DEBUG,"Error connecting ssl");
			return -1;
		}
#else
		//printf ("SSL requested but ssl libraries unavailable\n");
		return Z_ERR_NUM_SSL_UNAVAIL;

#endif
	}

	return indice_tabla;

}

int get_socket_number(int indice_tabla)
{

	if (indice_tabla<0 || indice_tabla>=MAX_Z_SOCKETS) {
		//printf ("indice fuera de rango\n");
		return Z_ERR_NUM_SOCK_NOT_OPEN;
	}

	if (!sockets_list[indice_tabla].used) {
		//printf ("socket indice %d no esta usado\n",indice_tabla);
		return Z_ERR_NUM_SOCK_NOT_OPEN;
	}

	else {
		return sockets_list[indice_tabla].socket_number;
	}
}


int z_sock_close_connection(int indice_tabla)
{

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
				return sock;
	}


	sockets_list[indice_tabla].used=0;

	if (sockets_list[indice_tabla].use_ssl.v) {

#ifdef COMPILE_SSL
		z_disconnect_ssl(indice_tabla);
#else
		//printf ("SSL requested but ssl libraries unavailable\n");
		return Z_ERR_NUM_SSL_UNAVAIL;

#endif

	}



	return cerrar_socket(sock);
}


//Liberar el socket de la lista pero sin desconectar
int z_sock_free_connection(int indice_tabla)
{

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
				return sock;
	}


	sockets_list[indice_tabla].used=0;

	return 0;
}

int z_sock_read(int indice_tabla, z80_byte *buffer, int longitud)
{

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
				return sock;
	}

	if (sockets_list[indice_tabla].use_ssl.v) {

#ifdef COMPILE_SSL
		//int SSL_read(SSL *ssl, void *buf, int num);
        z_sock_increment_traffic_counter_read(longitud);
		return SSL_read(sockets_list[indice_tabla].ssl_conn,buffer,longitud);
#else
		//printf ("SSL requested but ssl libraries unavailable\n");
		return Z_ERR_NUM_SSL_UNAVAIL;

#endif
	}


	return leer_socket(sock,(char *)buffer,longitud);
}


int z_sock_write_string(int indice_tabla, char *buffer)
{

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
				return sock;
	}

	if (sockets_list[indice_tabla].use_ssl.v) {

#ifdef COMPILE_SSL
		//int SSL_write(SSL *ssl, const void *buf, int num);
		int longitud=strlen(buffer);
        z_sock_increment_traffic_counter_write(longitud);
		return SSL_write(sockets_list[indice_tabla].ssl_conn,buffer,longitud);
#else
		//printf ("SSL requested but ssl libraries unavailable\n");
		return Z_ERR_NUM_SSL_UNAVAIL;

#endif
	}

    //printf("before escribir_socket\n");
	return escribir_socket(sock,buffer);
    //printf("after escribir_socket\n");
}



int zsock_wait_until_command_prompt(int indice_tabla)
{
	//Leemos hasta que no haya mas datos para leer. Idealmente estara el "command> "
	int leidos;

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
				return sock;
	}



		char buffer[100];

		do {
			//if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
			if (zsock_available_data(sock)) {
				leidos=z_sock_read(indice_tabla,(z80_byte *)buffer,100);
				//printf ("leidos en zsock_wait_until_command_prompt: %d\n",leidos);
				if (leidos<0) return -1;
			}
			else {
				leidos=0;
			}
		} while (leidos>0);

		return 0;

	}


//Esta funcion no la usa nadie de momento
//Si se activa alguna vez, comparar con zsock_read_all_until_command pues son casi iguales (esta ultima comprueba el prompt final, y zsock_read_all no)
/*
int zsock_read_all(int indice_tabla,z80_byte *buffer,int max_buffer)
{

	//Leemos hasta que no haya mas datos para leer. Idealmente estara el "command> "
	int leidos;

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
		return sock;
	}


	int pos_destino=0;
	int total_leidos=0;

		do {
			//if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
			if (zsock_available_data(sock)) {
				leidos=z_sock_read(indice_tabla,&buffer[pos_destino],max_buffer);
				//printf ("leidos en zsock_wait_until_command_prompt: %d\n",leidos);
				if (leidos<0) return -1;
				else {
					max_buffer -=leidos;
					total_leidos +=leidos;
					pos_destino +=leidos;
				}
			}
			else {
				leidos=0;
			}
		} while (leidos>0 && max_buffer>0);

	return total_leidos;

}
*/

void zsock_debug_dump_ascii(char *buffer,int total_leidos)
{
	for (;total_leidos>0;total_leidos--,buffer++) {
		printf ("%c",util_printable_char(*buffer));
	}
}

//devuelve en posicion_command donde empieza el "command>", si es >=0
//permite establecer maximo reintentos
int zsock_read_all_until_command_max_reintentos(int indice_tabla,z80_byte *buffer,int max_buffer,int *posicion_command,int max_reintentos)
{

	//printf ("inicio zsock_read_all_until_command\n");

	//Leemos hasta que no haya mas datos para leer. Idealmente estara el "command> "
	int leidos;

	//Inicialmente no sabemos la posicion del command>
	*posicion_command=-1;

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
		return sock;
	}


	int pos_destino=0;
	int total_leidos=0;
	int leido_command_prompt=0;
	int reintentos=0;
	//printf ("entrando bucle zsock_read_all_until_command\n");
	do {

		do {
			//if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
			if (zsock_available_data(sock)) {
				leidos=z_sock_read(indice_tabla,&buffer[pos_destino],max_buffer);
				//printf ("leidos en zsock_wait_until_command_prompt: %d\n",leidos);
				if (leidos<0) return -1;
				else {
					max_buffer -=leidos;
					total_leidos +=leidos;
					pos_destino +=leidos;
				}
			}
			else {
				leidos=0;
			}
		} while (leidos>0 && max_buffer>0);

		//Ver si hay "command> al final"
		if (total_leidos>=9) { //"command> " ocupa 9
			//printf ("Leido al final: %02XH %02XH [%c%c]\n",buffer[total_leidos-2],buffer[total_leidos-1],
			//util_printable_char(buffer[total_leidos-2]),util_printable_char(buffer[total_leidos-1]));

			//printf ("Leido todo: ");
			//zsock_debug_dump_ascii(buffer,total_leidos);
			//printf ("\n");

			if (buffer[total_leidos-1]==' ' && buffer[total_leidos-2]=='>') {
				leido_command_prompt=1;
				*posicion_command=total_leidos-9;
				//printf ("Recibido command prompt\n");
			}
		}

		else {
			//printf ("total leidos: %d\n",total_leidos);
		}


		if (!leido_command_prompt) {
			//printf ("NO recibido command prompt. Reintentar\n");
			usleep(10000); //10 ms
		}

		reintentos++;


	} while (!leido_command_prompt && reintentos<max_reintentos);


	//TODO: si se llega aqui sin haber recibido command prompt. Se gestionara en retorno mediante posicion_command
	return total_leidos;

}

//devuelve en posicion_command donde empieza el "command>", si es >=0
int zsock_read_all_until_command(int indice_tabla,z80_byte *buffer,int max_buffer,int *posicion_command)
{

    //5 segundos de reintentos
    return zsock_read_all_until_command_max_reintentos(indice_tabla,buffer,max_buffer,posicion_command,500);



}


//devuelve en posicion_command donde empieza el salto de linea, si es >=0
int zsock_read_all_until_newline(int indice_tabla,z80_byte *buffer,int max_buffer,int *posicion_command)
{

	//Leemos hasta que no haya mas datos para leer. Idealmente estara el "\n"
	int leidos;

	//Inicialmente no sabemos la posicion del command>
	*posicion_command=-1;

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
		return sock;
	}


	int pos_destino=0;
	int total_leidos=0;
	int leido_command_prompt=0;
	int reintentos=0;

	do {

		do {
			//if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
			if (zsock_available_data(sock)) {
				leidos=z_sock_read(indice_tabla,&buffer[pos_destino],max_buffer);
				//printf ("leidos en zsock_wait_until_command_prompt: %d\n",leidos);
				if (leidos<0) return -1;
				else {
					max_buffer -=leidos;
					total_leidos +=leidos;
					pos_destino +=leidos;
				}
			}
			else {
				leidos=0;
			}
		} while (leidos>0 && max_buffer>0);

		//Ver si hay "\n" al final
		if (total_leidos>=1) { //"\n" ocupa 9
			//printf ("Leido al final: %02XH %02XH [%c%c]\n",buffer[total_leidos-2],buffer[total_leidos-1],
			//util_printable_char(buffer[total_leidos-2]),util_printable_char(buffer[total_leidos-1]));

			//printf ("Leido todo: ");
			//zsock_debug_dump_ascii(buffer,total_leidos);
			//printf ("\n");

			if (buffer[total_leidos-1]=='\n') {
				leido_command_prompt=1;
				*posicion_command=total_leidos-1;
				//printf ("Recibido command prompt\n");
			}
		}

		else {
			//printf ("total leidos: %d\n",total_leidos);
		}


		if (!leido_command_prompt) {
			//printf ("NO recibido command prompt. Reintentar\n");
			usleep(10000); //10 ms
		}

		reintentos++;

		//TODO controlar maximo reintentos
	} while (!leido_command_prompt && reintentos<500);
	//5 segundos de reintentos

	//TODO: si se llega aqui sin haber recibido command prompt. Se gestionara en retorno mediante posicion_command
	return total_leidos;

}

char *zsock_http_skip_headers(char *mem,int total_leidos,int *http_code,char *redirect)
{
//leer linea a linea hasta fin cabecera
	char buffer_linea[1024];
	int i=0;
	int salir=0;
	int linea=0;

	//de momento
	redirect[0]=0;

	int redireccion=0;

	do {
		int leidos;
		char *next_mem;
		if (*mem=='\n') {
			//esto puede que no pase, linea con solo salto linea tendra un cr antes,
			//por tanto la deteccion de esa linea se leera abajom cuando buffer linea vacia
			salir=1;
			mem++;
			//printf ("salir con salto linea inicial\n");
		}
		else {
			next_mem=util_read_line(mem,buffer_linea,total_leidos,1024,&leidos);
			total_leidos -=leidos;

			//si linea primera http code
			if (linea==0) {
				char *existe;
				existe=strstr(buffer_linea," ");
				if (existe!=NULL) {
					//Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
					*http_code=parse_string_to_number(&existe[1]);
					if ((*http_code)==302) redireccion=1;
				}
			}

			linea++;

			if (buffer_linea[0]==0) {
				salir=1;
				//printf ("salir con linea vacia final\n");
				mem=next_mem;
			}
			else {
				debug_printf (VERBOSE_PARANOID,"header %d: %s",i,buffer_linea);

				//Ver si redirect
				if (redireccion) {
					char *existe;
					char *existe_minus;
					char *pref_location="Location: ";

					//Windows no tiene strcasestr
					//existe=strcasestr(buffer_linea,"Location: ");


					existe=strstr(buffer_linea,"Location: ");
					existe_minus=strstr(buffer_linea,"location: ");

					if (existe!=NULL || existe_minus!=NULL) {
						int longitud=strlen(pref_location);
						debug_printf (VERBOSE_DEBUG,"zsock_http_skip_headers Detected redirect %s",buffer_linea);
						strcpy(redirect,&buffer_linea[longitud]);
					}
				}

				i++;
				mem=next_mem;
			}

			if (total_leidos<=0) salir=1;
		}
	} while (!salir);

	//printf ("respuesta despues cabeceras:\n%s\n",mem)
	return mem;
}


int zsock_http(char *host, char *url,int *http_code,char **mem,int *t_leidos, char **mem_after_headers,int skip_headers,
    char *add_headers,int use_ssl,char *redirect_url,int estimated_maximum_size, char *ssl_sni_host_name)
{

	*mem=NULL;
	*mem_after_headers=NULL;
	*t_leidos=0;
	*http_code=200; //asumimos ok por defecto
	redirect_url[0]=0;

	int puerto=80;

	if (use_ssl) puerto=443;

	int indice_socket=z_sock_open_connection(host,puerto,use_ssl,ssl_sni_host_name);

	if (indice_socket<0) {
		//printf ("retornamos desde zsock http. errnum: %d\n",indice_socket);
		return indice_socket;
	}

		int sock=get_socket_number(indice_socket);

	if (sock<0) {
		return sock;
	}

	char request[1024];

	sprintf(request,"GET %s HTTP/1.0\r\n"
					"Host: %s\r\n"
					"User-Agent: ZEsarUX " EMULATOR_VERSION " " COMPILATION_SYSTEM "\r\n"
					"Accept-Encoding: identity\r\n"
					"%s"
					"\r\n",
					url,host,add_headers);

		//Agregamos siempre "Accept-Encoding: identity\r\n" para decir que no aceptamos contenido comprimido
		//Esto es especialmente sensible en zxinfo.dk, pues habia a veces que retornaba contenido gz
		//Ejemplo: query "target renegade" y luego seleccionar el juego target renegade
		/*
		Info del RFC
		https://tools.ietf.org/html/rfc2616#section-14

If no Accept-Encoding field is present in a request, the server MAY
   assume that the client will accept any content coding. In this case,
   if "identity" is one of the available content-codings, then the
   server SHOULD use the "identity" content-coding, unless it has
   additional information that a different content-coding is meaningful
   to the client.
		*/

	//Cuidado que este debug_printf no exceda el valor de DEBUG_MAX_MESSAGE_LENGTH, que no llegará, pero por si acaso...
	debug_printf (VERBOSE_DEBUG,"zsock_http Request:\n%s",request);


	int escritos=z_sock_write_string(indice_socket,request);


	if (escritos<0) {
		//Error escribiendo en el socket
		return Z_ERR_NUM_WRITE_SOCKET;
	}

	//todo buffer asignar
	char *response;
	int max_buffer=MAX_ZSOCK_HTTP_BUFFER; //1024*1024; //1 mb max por defecto

	if (estimated_maximum_size>max_buffer) max_buffer=estimated_maximum_size;


	debug_printf (VERBOSE_DEBUG,"Allocating max block for download: %d",max_buffer);


	response=malloc(max_buffer);
	if (response==NULL) cpu_panic ("Can not allocate memory for http response");


	//char response[65535];

	//int leido_content_length=0;
	int pos_destino=0;

	int leidos;
	int salir=0;
	int total_leidos=0;

	//todo leer hasta content-length o hasta cierre de socket
	//todo usar funcion parecida a zsock_read_all_until_command pero con condicion redefinible
	//todo ver si el socket se ha cerrado
	int reintentos=0;

	int max_reintentos=ZSOCK_HTTP_DEFAULT_MINIMUM_SEGMENTS; //500;

	if (estimated_maximum_size>0) {
        //Calculamos estimacion de segmentos en base a estimated_maximum_size
        //calculo muy pesimista de 1024 bytes por bloque
        max_reintentos=estimated_maximum_size/1024;

        //Y no bajar del minimo por default
        if (max_reintentos<ZSOCK_HTTP_DEFAULT_MINIMUM_SEGMENTS) max_reintentos=ZSOCK_HTTP_DEFAULT_MINIMUM_SEGMENTS;
	}

	do {
		do {
			//if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
			if (zsock_available_data(sock)) {
				leidos=z_sock_read(indice_socket,(z80_byte *)&response[pos_destino],max_buffer);

				//Print un poco mas inteligente
				int print_total_leidos_mb=0;
				if (total_leidos>1024*1024) print_total_leidos_mb=1;

				int print_estimated_maximum_size_mb=0;
				if (estimated_maximum_size>1024*1024) print_estimated_maximum_size_mb=1;

				debug_printf (VERBOSE_DEBUG,"Read data on zsock_http (z_sock_read): %d (current %d%s estimated max %d%s) chunk count: %d (max %d)",leidos,
											(print_total_leidos_mb ? total_leidos/1024/1024: total_leidos),
											(print_total_leidos_mb ? "MB": "B"),

											(print_estimated_maximum_size_mb ? estimated_maximum_size/1024/1024: estimated_maximum_size),
											(print_estimated_maximum_size_mb ? "MB": "B"),

											reintentos,max_reintentos);
				if (leidos<0) salir=1;
				else if (leidos==0) {
					salir=1; //si lee 0, ha llegado al final
					//todo aplicar lo mismo a funciones zeng
				}
				else {
					max_buffer -=leidos;
					total_leidos +=leidos;
					pos_destino +=leidos;
				}
			}
			else {
				leidos=0;
			}
		} while (leidos>0 && max_buffer>0 && !salir);
		//int leidos=z_sock_read(indice_socket,&response[pos_destino],65535);

		if (!salir) {
			usleep(10000); //10 ms. Suponiendo max_reintentos=500 y 10 ms de pausa, esto nos lleva 5 segundos continuos de pausa
			reintentos++;
		}

		//controlar maximo reintentos
		//TODO: realmente hace falta? Realmente estos reintentos son, mas bien, trozos en que se divide la descarga (o sea, veces que lanzamos la peticion http)
	} while (reintentos<max_reintentos && !salir);

	debug_printf (VERBOSE_PARANOID,"zsock_http: Retries: %d",reintentos);

	if (total_leidos>0) {
		response[total_leidos]=0;
		debug_printf (VERBOSE_DEBUG,"zsock_http: Total read data: %d",total_leidos);
		*t_leidos=total_leidos;
		//printf ("respuesta:\n%s\n",response);
		z_sock_close_connection(indice_socket);
		*mem=response;

		if (skip_headers) {

			*mem_after_headers=zsock_http_skip_headers(*mem,total_leidos,http_code,redirect_url);

		}
		return 0;
	}

	else return -1;


}
