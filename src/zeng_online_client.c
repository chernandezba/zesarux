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
#include "zeng.h"
#include "remote.h"
#include "snap_zsf.h"
#include "stats.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng_online_client_list_rooms;
pthread_t thread_zeng_online_client_create_room;
pthread_t thread_zeng_online_client_join_room;
pthread_t thread_zoc_snapshot_sending;
pthread_t thread_zoc_snapshot_receiving;
pthread_t thread_zoc_keys_sending;
pthread_t thread_zoc_keys_receiving;
pthread_t thread_zoc_master_thread;
pthread_t thread_zoc_slave_thread;


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

char zeng_online_server[NETWORK_MAX_URL+1]="51.83.33.13";
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
        printf("Sending command: [%s]\n",buffer_comando);
        escritos=z_sock_write_string(indice_socket,buffer_comando);
        //printf("after z_sock_write_string 1\n");
        if (escritos<0) return escritos;


        //TODO esto es ineficiente y que tiene que calcular la longitud. hacer otra z_sock_write sin tener que calcular
        //printf("before z_sock_write_string 2\n");
        printf("Sending snapshot data length: %lu\n",strlen(zoc_send_snapshot_mem_hexa));
        printf("First bytes of snapshot: %c%c%c%c\n",
            zoc_send_snapshot_mem_hexa[0],zoc_send_snapshot_mem_hexa[1],zoc_send_snapshot_mem_hexa[2],zoc_send_snapshot_mem_hexa[3]);
        escritos=z_sock_write_string(indice_socket,zoc_send_snapshot_mem_hexa);
        //printf("after z_sock_write_string 2\n");



        if (escritos<0) return escritos;



        z80_byte buffer[200];
        //buffer[0]=0; //temp para tener buffer limpio
        //Leer hasta prompt
        //printf("before zsock_read_all_until_command\n");
        leidos=zsock_read_all_until_command(indice_socket,buffer,199,&posicion_command);
        //printf("after zsock_read_all_until_command\n");

                if (posicion_command>=1) {
                    buffer[posicion_command-1]=0;
                    //debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                }

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

        //Enviar teclas
        //TODO gestionar error_desconectar
        int enviada_alguna_tecla;
        int error_desconectar=zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);
    }

	return 0;

}

void zoc_get_keys(int indice_socket)
{

    //Ver si hay datos disponibles en el socket
	int sock_number=get_socket_number(indice_socket);

	if (sock_number<0) {
		return;
	}

    if (!zsock_available_data(sock_number)) return;



        //Ir leyendo cada linea
        //buffer suficientemente grande por si llegan varios eventos de golpe
        char buffer[1024];

        //Leer hasta prompt
        int posicion_command;

        //printf ("antes de leer hasta command prompt\n");
        int leidos=zsock_read_all_until_newline(indice_socket,(z80_byte *)buffer,1023,&posicion_command);

        if (leidos>0) {
            buffer[leidos]=0; //fin de texto
            printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer);
        }

        if (leidos<0) {
            debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            //TODO return -1;
        }

        //Ir leyendo a cada final de linea
        int i;
        int inicio_linea=0;

        for (i=0;buffer[i];i++) {
            if (buffer[i]=='\n') {
                //Lo cambiamos a fin de cadena con 0
                buffer[i]=0;
                printf("procesar linea: [%s]\n",&buffer[inicio_linea]);
                ////Returned format is: uuid key event nomenu"

                char campo_inicio[10];
                char campo_final[10];
                char received_uuid[STATS_UUID_MAX_LENGTH+1];
                char received_key[10];
                char received_event[10];
                char received_nomenu[10];
                //ir procesando segun espacios
                int campos_leidos=0;

                int j;
                int inicio_campo=inicio_linea;
                for (j=inicio_linea;buffer[j] && campos_leidos<6;j++) {
                    if (buffer[j]==' ') {
                        buffer[j]=0;
                        printf("read field: [%s]\n",&buffer[inicio_campo]);

                        switch(campos_leidos) {
                            case 0:
                                strcpy(campo_inicio,&buffer[inicio_campo]);
                            break;

                            case 1:
                                strcpy(received_uuid,&buffer[inicio_campo]);
                            break;

                            case 2:
                                strcpy(received_key,&buffer[inicio_campo]);
                            break;

                            case 3:
                                strcpy(received_event,&buffer[inicio_campo]);
                            break;

                            case 4:
                                strcpy(received_nomenu,&buffer[inicio_campo]);
                            break;
                        }

                        campos_leidos++;
                        inicio_campo=j+1;
                    }
                }

                //campo final?
                if (campos_leidos==5) {
                    printf("read last field: [%s]\n",&buffer[inicio_campo]);
                    strcpy(campo_final,&buffer[inicio_campo]);

                    printf("Received event: uuid: [%s] key: [%s] event: [%s] nomenu: [%s]\n",
                        received_uuid,received_key,received_event,received_nomenu);

                    //Validar que empiece y acabe con "#"
                    int valido=0;

                    if (!strcmp(campo_inicio,"#") && !strcmp(campo_final,"#")) valido=1;

                    if (!valido) printf("linea no valida\n");
                    else {

                        //Si uuid yo soy mismo, no procesarlo
                        if (!strcmp(received_uuid,stats_uuid)) {
                            printf("The event is mine. Do not process it\n");
                        }
                        else {
                            int numero_key=parse_string_to_number(received_key);
                            int numero_event=parse_string_to_number(received_event);
                            int numero_nomenu=parse_string_to_number(received_nomenu);

                            int enviar=1;
                            if (numero_nomenu && menu_abierto) enviar=0;


                            //Enviar la tecla pero que no vuelva a entrar por zeng
                            if (enviar) {
                                debug_printf (VERBOSE_DEBUG,"Processing from ZRCP command send-keys-event: key %d event %d",numero_key,numero_event);
                                printf ("Processing from ZRCP command send-keys-event: key %d event %d\n",numero_key,numero_event);

                                debug_printf (VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
                                    UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);

                                //Si tecla especial de reset todas teclas. usado en driver curses
                                if (numero_key==UTIL_KEY_RESET_ALL) {
                                    //printf("Reset todas teclas\n");
                                    reset_keyboard_ports();
                                }

                                else {
                                    util_set_reset_key_continue_after_zeng(numero_key,numero_event);
                                }
                            }
                        }
                    }
                }

                else {
                    printf("Incorrect answer received\n");
                }


                inicio_linea=i+1;
            }
        }

}

int zeng_online_client_end_frame_reached=0;

void zeng_online_client_tell_end_frame(void)
{
    zeng_online_client_end_frame_reached=1;
}

void *zoc_master_thread_function(void *nada GCC_UNUSED)
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

    int indice_socket_get_keys=zoc_start_connection_get_keys();

    //bucle continuo de si hay snapshot de final de frame, enviarlo a remoto
    //TODO: ver posible manera de salir de aqui??
    while (1) {

        if (zeng_online_client_end_frame_reached) {
            zeng_online_client_end_frame_reached=0;
            if (!zoc_pending_send_snapshot) {
                //Esperar algo. 10 ms, suficiente porque es un mitad de frame
                //usleep(10000); //dormir 10 ms
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

            //Enviar teclas
            //TODO gestionar error_desconectar
            int enviada_alguna_tecla;
            int error_desconectar=zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);

            //recepcion teclas
            zoc_get_keys(indice_socket_get_keys);
        }

            usleep(10000); //dormir 10 ms

    }

	return 0;

}

//Pruebas
int esperar_por_envio_alguna_tecla=0;
/*void temp_esperar_por_envio_alguna_tecla(void)
{
    esperar_por_envio_alguna_tecla=50;
}*/
void *zoc_slave_thread_function(void *nada GCC_UNUSED)
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

    int indice_socket_get_keys=zoc_start_connection_get_keys();

    //bucle continuo de recepcion snapshot
    //TODO: ver posible manera de salir de aqui??
int temppppp;



    while (1) {
        if (zeng_online_client_end_frame_reached) {
            zeng_online_client_end_frame_reached=0;



            //recepcion teclas
            //TODO gestionar error
            zoc_get_keys(indice_socket_get_keys);


            //Enviar teclas
            //TODO gestionar error_desconectar
            int enviada_alguna_tecla;
            int error_desconectar=zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);

            if (enviada_alguna_tecla) {
                //esperar 1 segundo
                esperar_por_envio_alguna_tecla=50;
            }

            //Recibir snapshot
            //Siempre que no acabemos de enviar teclas. En ese caso dejar pasar unos segundos??
            if (esperar_por_envio_alguna_tecla>0) {
                esperar_por_envio_alguna_tecla--;
            }

            if (!esperar_por_envio_alguna_tecla) {
                //temppppp++;
                //if ((temppppp%50)==0) {
                    printf("llamando a zoc_receive_snapshot\n");
                    int error=zoc_receive_snapshot(indice_socket);
                    //TODO gestionar bien este error
                    if (error<0) {
                        //TODO
                        printf("Error getting snapshot from zeng online server\n");
                        usleep(10000); //dormir 10 ms
                    }
                //}
            }
        }

        usleep(10000); //dormir 10 ms

    }

	return 0;

}

int zoc_send_keys(int indice_socket,zeng_key_presses *elemento)
{


    char buffer_comando[256];

    int tecla=elemento->tecla;
    int pressrelease=elemento->pressrelease;

    //"send-keys user_pass n uuid key event nomenu
    //el 1 del final indica que no se envia la tecla si el menu en remoto esta abierto

    sprintf(buffer_comando,"zeng-online send-keys %s %d %s %d %d 1\n",
        created_room_user_password,zeng_online_joined_to_room_number,stats_uuid,
    tecla,pressrelease);


    int escritos=z_sock_write_string(indice_socket,buffer_comando);

   //Si ha habido error al escribir en socket
    if (escritos<0) {
        return escritos;
    }


    else {

        z80_byte buffer[200];

        //Leer hasta prompt
        int posicion_command;

        //printf ("antes de leer hasta command prompt\n");
        int leidos=zsock_read_all_until_command(indice_socket,buffer,199,&posicion_command);

        if (leidos>0) {
            buffer[leidos]=0; //fin de texto
            printf("Received text after send-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer);
        }

        if (leidos<0) {
            debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            return -1;
        }


    }


    return escritos;
}

int zoc_send_keys_avisado_final_frame=0;

int zoc_keys_send_pending(int indice_socket,int *enviada_alguna_tecla)
{

    *enviada_alguna_tecla=0;

    int error_desconectar=0;
    zeng_key_presses elemento;
    while (!zeng_fifo_read_element(&elemento) && !error_desconectar) {
        *enviada_alguna_tecla=1;
        debug_printf (VERBOSE_DEBUG,"ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d",elemento.tecla,elemento.pressrelease);

        printf ("ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d\n",elemento.tecla,elemento.pressrelease);

        debug_printf (VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
            UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);

        //command> help send-keys-event
        //Syntax: send-keys-event key event
            int error=zoc_send_keys(indice_socket,&elemento);

            if (error<0) error_desconectar=1;

    }

    return error_desconectar;
}

void *zoc_keys_sending_function(void *nada GCC_UNUSED)
{

    //conectar a remoto


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

    //bucle continuo de enviar teclas a remoto
    //TODO: ver posible manera de salir de aqui??
int error_desconectar=0;

    while (1) {

		//Si hay tecla pendiente de enviar. Enviarlas a final de frame
        if (zoc_send_keys_avisado_final_frame) {
            int enviada_alguna_tecla;
            error_desconectar=zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);


            zoc_send_keys_avisado_final_frame=0;

        }
        else {
        }


        //Esperar algo. 10 ms, suficiente porque es un mitad de frame
        usleep(10000); //dormir 10 ms
    }

	return 0;

}

void zeng_online_client_tell_send_keys_end_frame(void)
{
    zoc_send_keys_avisado_final_frame=1;
}

int zoc_start_connection_get_snapshot(void)
{
    //conectar a remoto

    //zeng_remote_list_rooms_buffer[0]=0;

    int indice_socket=z_sock_open_connection(zeng_online_server,zeng_online_server_port,0,"");

    if (indice_socket<0) {
        debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
            zeng_online_server,zeng_online_server_port,
            z_sock_get_error(indice_socket));
        return -1;
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
        return -1;
    }

    return indice_socket;
}

int zoc_start_connection_get_keys(void)
{
    //conectar a remoto


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
    char buffer_initial[ZENG_BUFFER_INITIAL_CONNECT+1];

    //int leidos=z_sock_read(indice_socket,buffer,199);
    int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
    if (leidos>0) {
        buffer_initial[leidos]=0; //fin de texto
        //printf("Received text (length: %d):\n[\n%s\n]\n",leidos,buffer);
    }

    if (leidos<0) {
        debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //bucle continuo de leer teclas de remoto
    //TODO: ver posible manera de salir de aqui??
int error_desconectar=0;



   char buffer_comando[256];



    ////command> zo get-keys OCAAYKWGYA 0

    sprintf(buffer_comando,"zeng-online get-keys %s %d\n",
        created_room_user_password,zeng_online_joined_to_room_number);


    int escritos=z_sock_write_string(indice_socket,buffer_comando);



   //Si ha habido error al escribir en socket
    if (escritos<0) {
        //TODO return escritos;
    }


    else {



        //printf ("antes de leer hasta command prompt\n");
        int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);

        if (leidos>0) {
            buffer_initial[leidos]=0; //fin de texto
            printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer_initial);
        }

        if (leidos<0) {
            debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            //TODO return -1;
        }


    }

    return indice_socket;

}

void *zoc_keys_receiving_function(void *nada GCC_UNUSED)
{

    //conectar a remoto


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
    char buffer_initial[ZENG_BUFFER_INITIAL_CONNECT+1];

    //int leidos=z_sock_read(indice_socket,buffer,199);
    int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
    if (leidos>0) {
        buffer_initial[leidos]=0; //fin de texto
        //printf("Received text (length: %d):\n[\n%s\n]\n",leidos,buffer);
    }

    if (leidos<0) {
        debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //bucle continuo de leer teclas de remoto
    //TODO: ver posible manera de salir de aqui??
int error_desconectar=0;



   char buffer_comando[256];



    ////command> zo get-keys OCAAYKWGYA 0

    sprintf(buffer_comando,"zeng-online get-keys %s %d\n",
        created_room_user_password,zeng_online_joined_to_room_number);


    int escritos=z_sock_write_string(indice_socket,buffer_comando);



   //Si ha habido error al escribir en socket
    if (escritos<0) {
        //TODO return escritos;
    }


    else {



        //printf ("antes de leer hasta command prompt\n");
        int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);

        if (leidos>0) {
            buffer_initial[leidos]=0; //fin de texto
            printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer_initial);
        }

        if (leidos<0) {
            debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            //TODO return -1;
        }


    }


    int indice_socket_receive_snapshot=zoc_start_connection_get_snapshot();
    //TODO. si error indice_socket_receive_snapshot<0

    while (1) {

        //Leer snapshot
        if (zeng_online_i_am_master.v==0) {
            int error=zoc_receive_snapshot(indice_socket_receive_snapshot);
            //TODO gestionar bien este error
            if (error<0) {
                //TODO
                printf("Error getting snapshot from zeng online server\n");
                usleep(10000); //dormir 10 ms
            }
        }

        //Ir leyendo cada linea
        //buffer suficientemente grande por si llegan varios eventos de golpe
        char buffer[1024];

        //Leer hasta prompt
        int posicion_command;

        //printf ("antes de leer hasta command prompt\n");
        int leidos=zsock_read_all_until_newline(indice_socket,(z80_byte *)buffer,1023,&posicion_command);

        if (leidos>0) {
            buffer[leidos]=0; //fin de texto
            printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer);
        }

        if (leidos<0) {
            debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            //TODO return -1;
        }

        //Ir leyendo a cada final de linea
        int i;
        int inicio_linea=0;

        for (i=0;buffer[i];i++) {
            if (buffer[i]=='\n') {
                //Lo cambiamos a fin de cadena con 0
                buffer[i]=0;
                printf("procesar linea: [%s]\n",&buffer[inicio_linea]);
                ////Returned format is: uuid key event nomenu"

                char campo_inicio[10];
                char campo_final[10];
                char received_uuid[STATS_UUID_MAX_LENGTH+1];
                char received_key[10];
                char received_event[10];
                char received_nomenu[10];
                //ir procesando segun espacios
                int campos_leidos=0;

                int j;
                int inicio_campo=inicio_linea;
                for (j=inicio_linea;buffer[j] && campos_leidos<6;j++) {
                    if (buffer[j]==' ') {
                        buffer[j]=0;
                        printf("read field: [%s]\n",&buffer[inicio_campo]);

                        switch(campos_leidos) {
                            case 0:
                                strcpy(campo_inicio,&buffer[inicio_campo]);
                            break;

                            case 1:
                                strcpy(received_uuid,&buffer[inicio_campo]);
                            break;

                            case 2:
                                strcpy(received_key,&buffer[inicio_campo]);
                            break;

                            case 3:
                                strcpy(received_event,&buffer[inicio_campo]);
                            break;

                            case 4:
                                strcpy(received_nomenu,&buffer[inicio_campo]);
                            break;
                        }

                        campos_leidos++;
                        inicio_campo=j+1;
                    }
                }

                //campo final?
                if (campos_leidos==5) {
                    printf("read last field: [%s]\n",&buffer[inicio_campo]);
                    strcpy(campo_final,&buffer[inicio_campo]);

                    printf("Received event: uuid: [%s] key: [%s] event: [%s] nomenu: [%s]\n",
                        received_uuid,received_key,received_event,received_nomenu);

                    //Validar que empiece y acabe con "#"
                    int valido=0;

                    if (!strcmp(campo_inicio,"#") && !strcmp(campo_final,"#")) valido=1;

                    if (!valido) printf("linea no valida\n");
                    else {

                        //Si uuid yo soy mismo, no procesarlo
                        if (!strcmp(received_uuid,stats_uuid)) {
                            printf("The event is mine. Do not process it\n");
                        }
                        else {
                            int numero_key=parse_string_to_number(received_key);
                            int numero_event=parse_string_to_number(received_event);
                            int numero_nomenu=parse_string_to_number(received_nomenu);

                            int enviar=1;
                            if (numero_nomenu && menu_abierto) enviar=0;


                            //Enviar la tecla pero que no vuelva a entrar por zeng
                            if (enviar) {
                                debug_printf (VERBOSE_DEBUG,"Processing from ZRCP command send-keys-event: key %d event %d",numero_key,numero_event);
                                printf ("Processing from ZRCP command send-keys-event: key %d event %d\n",numero_key,numero_event);

                                debug_printf (VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
                                    UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);

                                //Si tecla especial de reset todas teclas. usado en driver curses
                                if (numero_key==UTIL_KEY_RESET_ALL) {
                                    //printf("Reset todas teclas\n");
                                    reset_keyboard_ports();
                                }

                                else {
                                    util_set_reset_key_continue_after_zeng(numero_key,numero_event);
                                }
                            }
                        }
                    }
                }

                else {
                    printf("Incorrect answer received\n");
                }


                inicio_linea=i+1;
            }
        }



        //Esperar algo. 10 ms, suficiente porque es un mitad de frame
        usleep(10000); //dormir 10 ms
    }

	return 0;

}

char *zoc_get_snapshot_mem_hexa=NULL;
z80_byte *zoc_get_snapshot_mem_binary=NULL;
int zoc_get_snapshot_mem_binary_longitud=0;
int zoc_pending_apply_received_snapshot=0;

int zoc_receive_snapshot_last_id=0;

int zoc_receive_snapshot(int indice_socket)
{
    printf("Inicio zoc_receive_snapshot llamado desde:\n");
    //debug_exec_show_backtrace();

    debug_printf (VERBOSE_DEBUG,"ZENG: Receiving snapshot");

    int posicion_command;
    int escritos,leidos;


    char buffer_comando[200];

    //while (1) {

            if (!zoc_pending_apply_received_snapshot) {

                #define ZENG_BUFFER_INITIAL_CONNECT 199

                //Leer algo
                char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

                //Ver si el id de snapshot ha cambiado

                sprintf(buffer_comando,"zeng-online get-snapshot-id %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);
                escritos=z_sock_write_string(indice_socket,buffer_comando);
                //printf("after z_sock_write_string 1\n");
                if (escritos<0) return escritos;

                //Leer hasta prompt
                //printf("before zsock_read_all_until_command\n");
                leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);

                if (leidos>0) {
                    buffer[leidos]=0; //fin de texto
                    debug_printf(VERBOSE_DEBUG,"ZENG: Received text for get-snapshot-id (length %d): \n[\n%s\n]",leidos,buffer);
                }

                if (leidos<0) {
                    debug_printf(VERBOSE_ERR,"ERROR. Can't receive get-snapshot-id: %s",z_sock_get_error(leidos));
                    return 0;
                }


                //printf("after zsock_read_all_until_command\n");
                printf("Recibido respuesta despues de get-snapshot-id: [%s]\n",buffer);

                //1 mas para eliminar el salto de linea anterior a "command>"
                if (posicion_command>=1) {
                    buffer[posicion_command-1]=0;
                    //debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                }
                else {
                    debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX get-snapshot-id");
                    return 0;
                }

                printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

                int leer_snap=0;

                //Detectar si error
                if (strstr(buffer,"ERROR")!=NULL) {
                    printf("Error getting snapshot-id\n");
                }
                else {
                    //Ver si id diferente
                    int nuevo_id=parse_string_to_number(buffer);
                    if (nuevo_id!=zoc_receive_snapshot_last_id) {
                        zoc_receive_snapshot_last_id=nuevo_id;
                        leer_snap=1;
                    }
                    else {
                        printf("Snapshot es el mismo que el anterior\n");
                    }
                }



                if (leer_snap) {

                    printf("Obteniendo snapshot\n");

                    printf("antes enviar get-snaps\n");
                    //get-snapshot user_pass n
                    sprintf(buffer_comando,"zeng-online get-snapshot %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);
                    escritos=z_sock_write_string(indice_socket,buffer_comando);
                    //printf("after z_sock_write_string 1\n");
                    if (escritos<0) return escritos;

                    //int sock=get_socket_number(indice_socket);


                    printf("despues enviar get-snaps\n");



            //Ver si hay datos disponibles y no esta pendiente aplicar ultimo snapshot

                    if (zoc_get_snapshot_mem_hexa==NULL) {
                        zoc_get_snapshot_mem_hexa=util_malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2,"Can not allocate memory for getting snapshot"); //16 MB es mas que suficiente
                    }

                    //Leer hasta prompt
                    //printf("before zsock_read_all_until_command\n");
                    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zoc_get_snapshot_mem_hexa,ZRCP_GET_PUT_SNAPSHOT_MEM*2,&posicion_command);

                    if (leidos>0) {
                        zoc_get_snapshot_mem_hexa[leidos]=0; //fin de texto
                        debug_printf(VERBOSE_DEBUG,"ZENG: Received text for get-snapshot (length %d): \n[\n%s\n]",leidos,zoc_get_snapshot_mem_hexa);
                    }

                    if (leidos<0) {
                        debug_printf(VERBOSE_ERR,"ERROR. Can't receive get-snapshot: %s",z_sock_get_error(leidos));
                        return 0;
                    }


                    //printf("after zsock_read_all_until_command\n");
                   // printf("Recibido respuesta despues de get-snapshot: [%s]\n",zoc_get_snapshot_mem_hexa);

                    //1 mas para eliminar el salto de linea anterior a "command>"
                    if (posicion_command>=1) {
                        zoc_get_snapshot_mem_hexa[posicion_command-1]=0;
                        //debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                    }
                    else {
                        debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online create-room");
                        return 0;
                    }

                    //printf("Recibido respuesta despues de truncar: [%s]\n",zoc_get_snapshot_mem_hexa);

                    //Nos quedamos con la respuesta hasta el ultimo
                    //Es decir, si en get-snapshot el server remoto nos ha dejado en cola 2, 3 o mas snapshots, estamos pillando el ultimo
                    //Recordemos que lo envia de manera continua con saltos de linea despues de cada uno de ellos
                    //TODO: esto esta MAL. Hay que escoger el que tiene el ultimo salto de linea
                    int i;

                    /*int inicio_datos_snapshot=0;
                    //int leidos_saltos_linea=0;

                    for (i=0;i<leidos;i++) {
                        if (zoc_get_snapshot_mem_hexa[i]=='\n') {
                            //Y quitar ese salto de linea, si es el final no queremos que se procese
                            zoc_get_snapshot_mem_hexa[i]=0;

                            //Si hay algo mas despues del salto de linea
                            if (i!=leidos-1) inicio_datos_snapshot=i+1;
                        }
                    }
                    */

                    //printf("Buffer despues de truncar: [%s]\n",&zoc_get_snapshot_mem_hexa[inicio_datos_snapshot]);

                    //TODO: detectar texto ERROR en respuesta
                    //return leidos;

                    //Convertir hexa a memoria
                    if (zoc_get_snapshot_mem_binary==NULL) {
                        zoc_get_snapshot_mem_binary=util_malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2,"Can not allocate memory for apply snapshot");
                    }


                    char *s=zoc_get_snapshot_mem_hexa;
                    int parametros_recibidos=0;
                    z80_byte valor;


                    while (*s) {
                        char buffer_valor[4];
                        buffer_valor[0]=s[0];
                        buffer_valor[1]=s[1];
                        buffer_valor[2]='H';
                        buffer_valor[3]=0;
                        //printf ("%s\n",buffer_valor);
                        valor=parse_string_to_number(buffer_valor);
                        //printf ("valor: %d\n",valor);

                        zoc_get_snapshot_mem_binary[parametros_recibidos++]=valor;
                        //menu_debug_write_mapped_byte(direccion++,valor);

                        s++;
                        if (*s) s++;
                    }


                    zoc_get_snapshot_mem_binary_longitud=parametros_recibidos;

                    zoc_pending_apply_received_snapshot=1;

                }

        }
        else {
            printf("get-snapshot no disponible. esperar\n");

        }

        //Esperar algo. 10 ms, suficiente porque es un mitad de frame
        //usleep(10000); //dormir 10 ms

    //}
    return 1;


}

void zeng_online_client_apply_pending_received_snapshot(void)
{
    if (zeng_online_connected.v==0) return;

    if (zeng_online_i_am_master.v) return;

    if (!zoc_pending_apply_received_snapshot) return;


    printf("Putting snapshot coming from ZRCP: %p %d\n",zoc_get_snapshot_mem_binary,zoc_get_snapshot_mem_binary_longitud);

    load_zsf_snapshot_file_mem(NULL,zoc_get_snapshot_mem_binary,zoc_get_snapshot_mem_binary_longitud,1);

    free(zoc_get_snapshot_mem_binary);
    zoc_get_snapshot_mem_binary=NULL;

    zoc_pending_apply_received_snapshot=0;

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
            //zoc_pending_send_snapshot=0;
            //printf("Snapshot sent\n");
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


//Inicio del thread de master
void zoc_start_master_thread(void)
{
	if (pthread_create( &thread_zoc_master_thread, NULL, &zoc_master_thread_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online send snapshot pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_master_thread);
}

//Inicio del thread de slave
void zoc_start_slave_thread(void)
{
	if (pthread_create( &thread_zoc_slave_thread, NULL, &zoc_slave_thread_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online send snapshot pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_slave_thread);
}

//Inicio del thread que va enviando teclas a servidor zeng online
void zoc_start_keys_sending(void)
{
	if (pthread_create( &thread_zoc_keys_sending, NULL, &zoc_keys_sending_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online send keys pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_keys_sending);
}

//Inicio del thread que va leyendo teclas del servidor zeng online
void zoc_start_keys_receiving(void)
{
	if (pthread_create( &thread_zoc_keys_receiving, NULL, &zoc_keys_receiving_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online receive keys pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_keys_receiving);
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

					if (zoc_send_snapshot_mem_hexa==NULL) zoc_send_snapshot_mem_hexa=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2); //16 MB es mas que suficiente

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