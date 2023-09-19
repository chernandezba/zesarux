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

/*

Online Network Play (using a central server) Client related code

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>



#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "network.h"
#include "compileoptions.h"
#include "zeng_online_client.h"
#include "remote.h"
#include "snap_zsf.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng_online_client_list_rooms;


#endif

//Variables y funciones que no son de pthreads
int zeng_online_client_list_rooms_thread_running=0;


char zeng_online_server[NETWORK_MAX_URL+1]="localhost";
int zeng_online_server_port=10000;

#ifdef USE_PTHREADS

//Funciones que usan pthreads



//Devuelve 0 si no conectado
int zeng_online_client_list_rooms_connect(void)
{

    //Inicialmente desconectado
    //zeng_remote_socket=-1;

    //Almacenar aqui hostname hasta la ,
    /*char buffer_hostname[MAX_ZENG_HOSTNAME];

    //Almacenar aqui la copia del campo entero, para facilitar cortar
    char copia_zeng_remote_hostname[MAX_ZENG_HOSTNAME];

    strcpy(copia_zeng_remote_hostname,zeng_remote_hostname);


    char *puntero_hostname=copia_zeng_remote_hostname;

    zeng_total_remotes=0;*/







		int indice_socket=z_sock_open_connection(zeng_online_server,zeng_online_server_port,0,"");

		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
                zeng_online_server,zeng_online_server_port,
                z_sock_get_error(indice_socket));
			return 0;
		}

		 int posicion_command;

#define ZENG_BUFFER_INITIAL_CONNECT 199

		//Leer algo
		char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

		//int leidos=z_sock_read(indice_socket,buffer,199);
		int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			//printf("Received text (length: %d):\n[\n%s\n]\n",leidos,buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
			return 0;
		}

		//zsock_wait_until_command_prompt(indice_socket);

		debug_printf(VERBOSE_DEBUG,"ZENG: Sending get-version");

		//Enviar un get-version
		int escritos=z_sock_write_string(indice_socket,"get-version\n");

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send get-version: %s",z_sock_get_error(escritos));
			return 0;
		}


		//reintentar
		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text for get-version (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't receive version: %s",z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			debug_printf(VERBOSE_DEBUG,"ZENG: Received version: %s",buffer);
		}
		else {
			debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX remote version");
			return 0;
		}

		//Comprobar que version remota sea como local
        char myversion[30];
        util_get_emulator_version_number(myversion);
        if (strcasecmp(myversion,buffer)) {
			debug_printf (VERBOSE_ERR,"Local and remote ZEsarUX versions do not match");
            //printf("Local %s remote %s\n",myversion,buffer);
			return 0;
		}


		//Comprobar que, si nosotros somos master, el remoto no lo sea
		//El usuario puede activarlo, aunque no es recomendable. Yo solo compruebo y ya que haga lo que quiera


        //ver si zeng online enabled
		debug_printf(VERBOSE_DEBUG,"ZENG Online: Getting zeng-online status");

		escritos=z_sock_write_string(indice_socket,"zeng-online is-enabled\n");

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online is-enabled: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online is-enabled (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online is-enabled: %s",z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			debug_printf(VERBOSE_DEBUG,"ZENG: Received zeng-online is-enabled: %s",buffer);
		}
		else {
			debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online is-enabled");
			return 0;
		}

        //Si somos master, que el remoto no lo sea tambien
        int esta_activado=parse_string_to_number(buffer);
        if (!esta_activado) {
            debug_printf (VERBOSE_ERR,"ZENG Online is not enabled on remote server");
        }


		//TODO: fin conexion



	//zeng_remote_socket=indice_socket;

	return 1;
}


void *zeng_online_client_list_rooms_function(void *nada GCC_UNUSED)
{

	zeng_online_client_list_rooms_thread_running=1;



	//Conectar a remoto

	if (!zeng_online_client_list_rooms_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_list_rooms_thread_running=0;
		return 0;
	}






	//zeng_enabled.v=1;


	zeng_online_client_list_rooms_thread_running=0;

	return 0;

}


void zeng_online_client_list_rooms(void)
{

	//ya  inicializado
	/*if (zeng_enabled.v) return;

	if (zeng_remote_hostname[0]==0) return;*/



	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_list_rooms, NULL, &zeng_online_client_list_rooms_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online list rooms pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_list_rooms);





}


#else

//Funciones sin pthreads. ZENG no se llama nunca cuando no hay pthreads, pero hay que crear estas funciones vacias
//para evitar errores de compilacion cuando no hay pthreads

void zeng_online_client_list_rooms(void)
{
}


#endif