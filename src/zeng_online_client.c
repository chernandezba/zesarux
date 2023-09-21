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

Functions starting with zoc_ means: zeng online client

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
#include "zeng_online.h"
#include "zeng_online_client.h"
#include "remote.h"
#include "snap_zsf.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng_online_client_list_rooms;
pthread_t thread_zeng_online_client_create_room;
pthread_t thread_zeng_online_client_join_room;
pthread_t thread_zoc_snapshot_sending;
pthread_t thread_zoc_snapshot_receiving;


#endif

//Variables y funciones que no son de pthreads
int zeng_online_client_list_rooms_thread_running=0;
int zeng_online_client_create_room_thread_running=0;
int zeng_online_client_join_room_thread_running=0;

z80_bit zeng_online_i_am_master={0};
z80_bit zeng_online_i_am_joined={0};
int zeng_online_joined_to_room_number=0;

int zoc_pending_send_snapshot=0;

//Si esta conectado
z80_bit zeng_online_connected={0};

char zeng_online_server[NETWORK_MAX_URL+1]="localhost";
int zeng_online_server_port=10000;

//Buffer donde guardar listado de rooms remotas
char *zeng_remote_list_rooms_buffer=NULL;

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




        //Calculo aproximado de memoria necesaria para listado de habitaciones
        //escribir_socket(misocket,"N.  Name                           Created Players Max\n");
        int espacio_por_room=ZENG_ONLINE_MAX_ROOM_NAME+1024; //Margen mas que suficiente
        int total_listado_rooms_memoria=espacio_por_room*ZENG_ONLINE_MAX_ROOMS;


		debug_printf(VERBOSE_DEBUG,"ZENG Online: Getting zeng-online rooms");
        if (zeng_remote_list_rooms_buffer==NULL) {

            zeng_remote_list_rooms_buffer=util_malloc(total_listado_rooms_memoria,"Can not allocate memory for getting list-rooms");


            /*
                    for (i=0;i<zeng_online_current_max_rooms;i++) {
            escribir_socket_format(misocket,"%3d %s %d     %3d       %3d\n",
                i,
                zeng_online_rooms_list[i].name,
                zeng_online_rooms_list[i].created,
                zeng_online_rooms_list[i].current_players,
                zeng_online_rooms_list[i].max_players
            );
            }
            */

        }
        zeng_remote_list_rooms_buffer[0]=0;

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
            return 0;
        }


        //Obtener rooms



		escritos=z_sock_write_string(indice_socket,"zeng-online list-rooms\n");

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online list-rooms: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_list_rooms_buffer,total_listado_rooms_memoria,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online list-rooms (length %d): \n[\n%s\n]",leidos,zeng_remote_list_rooms_buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online list-rooms: %s %s",zeng_remote_list_rooms_buffer,z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			zeng_remote_list_rooms_buffer[posicion_command-1]=0;
			debug_printf(VERBOSE_DEBUG,"ZENG: Received zeng-online list-rooms: %s",zeng_remote_list_rooms_buffer);
		}
		else {
			debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online list-rooms");
			return 0;
		}

        printf("Habitaciones: %s\n",zeng_remote_list_rooms_buffer);

		//finalizar conexion
        z_sock_close_connection(indice_socket);



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

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_list_rooms, NULL, &zeng_online_client_list_rooms_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online list rooms pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_list_rooms);


}

//El creator password de una room que hemos creado
char created_room_creator_password[ZENG_ROOM_PASSWORD_LENGTH+1]; //+1 para el 0 del final. un password simple, para las operaciones

//El user password de una room que nos hemos unido
char created_room_user_password[ZENG_ROOM_PASSWORD_LENGTH+1];

int param_join_room_number;

//Devuelve 0 si no conectado
int zeng_online_client_join_room_connect(void)
{




        zeng_remote_list_rooms_buffer[0]=0;

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

		debug_printf(VERBOSE_DEBUG,"ZENG: Sending join-room");

        char buffer_enviar[1024];
        sprintf(buffer_enviar,"zeng-online join %d\n",param_join_room_number);

		int escritos=z_sock_write_string(indice_socket,buffer_enviar);

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online join: %s",z_sock_get_error(escritos));
			return 0;
		}


		//reintentar
		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online join (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online join: %s",z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",buffer);
		}
		else {
			debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online join");
			return 0;
		}

        printf("Retorno join-room: [%s]\n",buffer);
        //Si hay ERROR
        if (strstr(buffer,"ERROR")!=NULL) {
            debug_printf(VERBOSE_ERR,"Error joining room: %s",buffer);
            return 0;
        }
        strcpy(created_room_user_password,buffer);

        printf("User password: [%s]\n",created_room_user_password);

		//finalizar conexion
        z_sock_close_connection(indice_socket);

        zeng_online_i_am_joined.v=1;

        zeng_online_joined_to_room_number=param_join_room_number;



	//zeng_remote_socket=indice_socket;

	return 1;
}

void *zeng_online_client_join_room_function(void *nada GCC_UNUSED)
{

	zeng_online_client_join_room_thread_running=1;



	//Conectar a remoto

	if (!zeng_online_client_join_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_join_room_thread_running=0;
		return 0;
	}






	//zeng_enabled.v=1;


	zeng_online_client_join_room_thread_running=0;

	return 0;

}

void zeng_online_client_join_room(int room_number)
{

	//Inicializar thread
    //Paso de parametro mediante variable estatica
    param_join_room_number=room_number;

	if (pthread_create( &thread_zeng_online_client_join_room, NULL, &zeng_online_client_join_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online join room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_join_room);


}

char param_create_room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];
int param_create_room_number;



//Devuelve 0 si no conectado
int zeng_online_client_create_room_connect(void)
{



        //zeng_remote_list_rooms_buffer[0]=0;

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

		debug_printf(VERBOSE_DEBUG,"ZENG: Sending create-room");

        char buffer_enviar[1024];
        sprintf(buffer_enviar,"zeng-online create-room %d \"%s\"\n",param_create_room_number,param_create_room_name);

		int escritos=z_sock_write_string(indice_socket,buffer_enviar);

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online create-room: %s",z_sock_get_error(escritos));
			return 0;
		}


		//reintentar
		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online create-room (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online create-room: %s",z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",buffer);
		}
		else {
			debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online create-room");
			return 0;
		}

        printf("Retorno crear-room: [%s]\n",buffer);
        //Si hay ERROR
        if (strstr(buffer,"ERROR")!=NULL) {
            debug_printf(VERBOSE_ERR,"Error creating room: %s",buffer);
            return 0;
        }
        strcpy(created_room_creator_password,buffer);

        printf("Creator password: [%s]\n",created_room_creator_password);

		//finalizar conexion
        z_sock_close_connection(indice_socket);



	//zeng_remote_socket=indice_socket;

	return 1;
}



void *zeng_online_client_create_room_function(void *nada GCC_UNUSED)
{

	zeng_online_client_create_room_thread_running=1;



	//Conectar a remoto

	if (!zeng_online_client_create_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_create_room_thread_running=0;
		return 0;
	}


	//zeng_enabled.v=1;


	zeng_online_client_create_room_thread_running=0;

	return 0;

}

void zeng_online_client_create_room(int room_number,char *room_name)
{
	//Inicializar thread
    //Parametros, dado que tienen que ser estaticos, ni me preocupo en crear una estructura, se asignan a dos variables
    //estaticas y listo
    param_create_room_number=room_number;
    strcpy(param_create_room_name,room_name);

	if (pthread_create( &thread_zeng_online_client_create_room, NULL, &zeng_online_client_create_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online create room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_create_room);
}

int zoc_frames_video_cada_snapshot=1;
int zoc_contador_envio_snapshot=0;
int zoc_snapshots_not_sent=0;
char *zoc_send_snapshot_mem_hexa=NULL;

int zoc_send_snapshot(int indice_socket)
{
	//Enviar snapshot cada 20*250=5000 ms->5 segundos
		debug_printf (VERBOSE_DEBUG,"ZENG: Sending snapshot");

		int posicion_command;
		int escritos,leidos;


        char buffer_comando[200];
        //printf ("Sending put-snapshot\n");
        //put-snapshot creator_pass n data
        sprintf(buffer_comando,"zeng-online put-snapshot %s %d ",created_room_creator_password,zeng_online_joined_to_room_number);
        escritos=z_sock_write_string(indice_socket,buffer_comando);
        //printf("after z_sock_write_string 1\n");
        if (escritos<0) return escritos;


        //TODO esto es ineficiente y que tiene que calcular la longitud. hacer otra z_sock_write sin tener que calcular
        //printf("before z_sock_write_string 2\n");
        escritos=z_sock_write_string(indice_socket,zoc_send_snapshot_mem_hexa);
        //printf("after z_sock_write_string 2\n");



        if (escritos<0) return escritos;



        z80_byte buffer[200];
        //Leer hasta prompt
        //printf("before zsock_read_all_until_command\n");
        leidos=zsock_read_all_until_command(indice_socket,buffer,199,&posicion_command);
        //printf("after zsock_read_all_until_command\n");
        printf("Recibido respuesta despues de put-snapshot: [%s]\n",buffer);
        return leidos;


}



void *zoc_snapshot_sending_function(void *nada GCC_UNUSED)
{

    //conectar a remoto

    //zeng_remote_list_rooms_buffer[0]=0;

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

    //bucle continuo de si hay snapshot de final de frame, enviarlo a remoto
    //TODO: ver posible manera de salir de aqui??
    while (1) {
        if (!zoc_pending_send_snapshot) {
            //Esperar algo. 10 ms, suficiente porque es un mitad de frame
            usleep(10000); //dormir 10 ms
        }
        else {
            int error=zoc_send_snapshot(indice_socket);

            if (error<0) {
                //TODO
                printf("Error sending snapshot to zeng online server\n");
            }

            //Enviado. Avisar no pendiente
            zoc_pending_send_snapshot=0;
            printf("Snapshot sent\n");
        }
    }

	return 0;

}

char *zoc_get_snapshot_mem_hexa=NULL;

int zoc_receive_snapshot(int indice_socket)
{
	//Enviar snapshot cada 20*250=5000 ms->5 segundos
		debug_printf (VERBOSE_DEBUG,"ZENG: Receiving snapshot");

		int posicion_command;
		int escritos,leidos;


        char buffer_comando[200];
        //printf ("Sending put-snapshot\n");
        //get-snapshot user_pass n
        sprintf(buffer_comando,"zeng-online get-snapshot %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);
        escritos=z_sock_write_string(indice_socket,buffer_comando);
        //printf("after z_sock_write_string 1\n");
        if (escritos<0) return escritos;

	    int sock=get_socket_number(indice_socket);




        while (1) {
            if (zsock_available_data(sock)) {
        //Ver si hay datos disponibles

                if (zoc_get_snapshot_mem_hexa==NULL) {
                    zoc_get_snapshot_mem_hexa=util_malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2,"Can not allocate memory for getting snapshot"); //16 MB es mas que suficiente
                }

                //Leer hasta prompt
                //printf("before zsock_read_all_until_command\n");
                leidos=zsock_read_all_until_newline(indice_socket,(z80_byte *)      zoc_get_snapshot_mem_hexa,ZRCP_GET_PUT_SNAPSHOT_MEM*2,&posicion_command);
                //printf("after zsock_read_all_until_command\n");
                printf("Recibido respuesta despues de get-snapshot: [%s]\n",zoc_get_snapshot_mem_hexa);
                //return leidos;

            }
            else {
                printf("get-snapshot no disponible. esperar\n");
                //Esperar algo. 10 ms, suficiente porque es un mitad de frame
                usleep(10000); //dormir 10 ms
            }



        }


}



void *zoc_snapshot_receiving_function(void *nada GCC_UNUSED)
{

    //conectar a remoto

    //zeng_remote_list_rooms_buffer[0]=0;

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

    //bucle continuo de si hay snapshot de final de frame, leerlo de remoto
    //TODO: ver posible manera de salir de aqui??
    while (1) {
        /*if (!zoc_pending_send_snapshot) {
            //Esperar algo. 10 ms, suficiente porque es un mitad de frame
            usleep(10000); //dormir 10 ms
        }
        else {*/
            int error=zoc_receive_snapshot(indice_socket);

            if (error<0) {
                //TODO
                printf("Error sending snapshot to zeng online server\n");
                usleep(10000); //dormir 10 ms
            }

            //Enviado. Avisar no pendiente
            zoc_pending_send_snapshot=0;
            printf("Snapshot sent\n");
        //}
    }

	return 0;

}

//Inicio del thread que como master va enviando snapshot a servidor zeng online
void zoc_start_snapshot_sending(void)
{
	if (pthread_create( &thread_zoc_snapshot_sending, NULL, &zoc_snapshot_sending_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online send snapshot pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_snapshot_sending);
}

//Inicio del thread que como slave va recibiendo snapshot de servidor zeng online
void zoc_start_snapshot_receiving(void)
{
	if (pthread_create( &thread_zoc_snapshot_receiving, NULL, &zoc_snapshot_receiving_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online receive snapshot pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_snapshot_receiving);
}


void zeng_online_client_prepare_snapshot_if_needed(void)
{

	if (zeng_online_connected.v==0) return;

	if (zeng_online_i_am_master.v) {
		zoc_contador_envio_snapshot++;
		//printf ("%d %d\n",contador_envio_snapshot,(contador_envio_snapshot % (50*zeng_segundos_cada_snapshot) ));
		if (zoc_contador_envio_snapshot>=zoc_frames_video_cada_snapshot) {
                zoc_contador_envio_snapshot=0;
				//Si esta el anterior snapshot aun pendiente de enviar
				if (zoc_pending_send_snapshot) {
                    zoc_snapshots_not_sent++;
					debug_printf (VERBOSE_DEBUG,"ZENG: Last snapshot has not been sent yet. Total unsent: %d",zoc_snapshots_not_sent);

                    //Si llegado a un limite, reconectar. Suele suceder con ZENG en slave windows
                    //Sucede que se queda la operacion de send a socket que no acaba
                    /*
                    Aun con esto, parece que a veces va enviando snapshots pero el remoto no parece procesarlos,
                    con lo que aqui lo da como bueno y no incrementa el contador de retries
                    */

                   //TODO
                   /*
                    if (zeng_force_reconnect_failed_retries.v) {
                        if (zoc_snapshots_not_sent>=3*50) { //Si pasan mas de 3 segundos y no ha enviado aun el ultimo snapshot
                            debug_printf (VERBOSE_INFO,"ZENG: Forcing reconnect");
                            //printf("Before forcing ZENG reconnect\n");
                            zeng_force_reconnect();
                            //printf("After forcing ZENG reconnect\n");
                        }
                    }
                    */
				}
				else {
                    zoc_snapshots_not_sent=0;

					//zona de memoria donde se guarda el snapshot pero sin pasar a hexa
					z80_byte *buffer_temp;
					buffer_temp=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM); //16 MB es mas que suficiente

					if (buffer_temp==NULL) cpu_panic("Can not allocate memory for sending snapshot");

					int longitud;

  					save_zsf_snapshot_file_mem(NULL,buffer_temp,&longitud);


                    //temp_memoria_asignada++;
                    //printf("Asignada: %d liberada: %d\n",temp_memoria_asignada,temp_memoria_liberada);

					zoc_send_snapshot_mem_hexa=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2); //16 MB es mas que suficiente

					int char_destino=0;

					int i;

					for (i=0;i<longitud;i++,char_destino +=2) {
						sprintf (&zoc_send_snapshot_mem_hexa[char_destino],"%02X",buffer_temp[i]);
					}

					//metemos salto de linea y 0 al final
					strcpy (&zoc_send_snapshot_mem_hexa[char_destino],"\n");

					debug_printf (VERBOSE_DEBUG,"ZENG Online: Queuing snapshot to send, length: %d",longitud);

                    printf ("ZENG Online: Queuing snapshot to send, length: %d\n",longitud);


					//Liberar memoria que ya no se usa
					free(buffer_temp);


					zoc_pending_send_snapshot=1;


				}
		}
	}
}


#else

//Funciones sin pthreads. ZENG no se llama nunca cuando no hay pthreads, pero hay que crear estas funciones vacias
//para evitar errores de compilacion cuando no hay pthreads

void zeng_online_client_list_rooms(void)
{
}

void zeng_online_client_create_room(int room_number,char *room_name)
{
}

void zoc_start_snapshot_sending(void)
{
}

#endif