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

//Para usar timeval
#include <sys/time.h>



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
#include "screen.h"
#include "timer.h"
#include "autoselectoptions.h"
#include "textspeech.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng_online_client_list_rooms;
pthread_t thread_zeng_online_client_create_room;
pthread_t thread_zeng_online_client_join_room;
pthread_t thread_zoc_master_thread;
pthread_t thread_zoc_slave_thread;
pthread_t thread_zeng_online_client_join_list;
pthread_t thread_zeng_online_client_authorize_join;
pthread_t thread_zeng_online_client_leave_room;
pthread_t thread_zeng_online_client_destroy_room;
pthread_t thread_zeng_online_client_autojoin_room;
pthread_t thread_zeng_online_client_disable_autojoin_room;
pthread_t thread_zeng_online_client_write_message_room;
pthread_t thread_zeng_online_client_allow_message_room;


#endif

//Variables y funciones que no son de pthreads
int zeng_online_client_list_rooms_thread_running=0;
int zeng_online_client_create_room_thread_running=0;
int zeng_online_client_join_room_thread_running=0;
int zeng_online_client_join_list_thread_running=0;
int zeng_online_client_authorize_join_thread_running=0;
int zeng_online_client_leave_room_thread_running=0;
int zeng_online_client_destroy_room_thread_running=0;
int zeng_online_client_autojoin_room_thread_running=0;
int zeng_online_client_disable_autojoin_room_thread_running=0;
int zeng_online_client_write_message_room_thread_running=0;
int zeng_online_client_allow_message_room_thread_running=0;

z80_bit zeng_online_i_am_master={0};
//z80_bit zeng_online_i_am_joined={0};
int zeng_online_joined_to_room_number=0;

int zoc_pending_send_snapshot=0;

//Si esta conectado
z80_bit zeng_online_connected={0};

char zeng_online_server[NETWORK_MAX_URL+1]="51.83.33.13";
int zeng_online_server_port=10000;

char zeng_online_nickname[ZOC_MAX_NICKNAME_LENGTH+1]="";

//Buffer donde guardar listado de rooms remotas
char *zeng_remote_list_rooms_buffer=NULL;

//Buffer donde guardar listado de list join queue
char zeng_remote_join_list_buffer[1024];

//Contador que se activa al recibir un snapshot. Mientras no sea 0, no se procesan pulsaciones de teclas locales en slave
int zoc_last_snapshot_received_counter=0;

//Tiempo que ha tardado en conectar a un list rooms
long zeng_online_last_list_rooms_latency=0;


//El creator password de una room que hemos creado
char created_room_creator_password[ZENG_ROOM_PASSWORD_LENGTH+1]; //+1 para el 0 del final. un password simple, para las operaciones

//El user password de una room que nos hemos unido
char created_room_user_password[ZENG_ROOM_PASSWORD_LENGTH+1];

//Los permisos de una room que nos hemos unido
int created_room_user_permissions;

int param_join_room_number;
char param_join_room_creator_password[ZENG_ROOM_PASSWORD_LENGTH+1];

//Se indicara que queremos obtener el snapshot que habia ya que estamos reentrando como master
int zoc_rejoining_as_master=0;


//Retornar puerto y hostname del server
int zeng_online_get_server_and_port(char *buffer_hostname)
{

    strcpy(buffer_hostname,zeng_online_server);
    int puerto=zeng_online_server_port;

    //Ver si se indica puerto con ":"
    char *existe_puerto=strstr(buffer_hostname,":");

    if (existe_puerto!=NULL) {
        *existe_puerto=0; //fijar fin de caracter

        //Y leer puerto
        existe_puerto++;
        puerto=parse_string_to_number(existe_puerto);
    }

    return puerto;
}

long zeng_online_get_last_list_rooms_latency(void)
{
    return zeng_online_last_list_rooms_latency;
}



char *string_zoc_return_connected_status_offline="OFFLINE";
char *string_zoc_return_connected_status_online="ONLINE";

//Retorna estado de conexion de zeng, asumiendo que está conectado:
//Si no ha habido timeout recibiendo snapshots, "ONLINE"
//Si ha habido timeout recibiendo snapshots y por tanto ahora permitimos teclas, "OFFLINE"
char *zoc_return_connected_status(void)
{
    if (zoc_last_snapshot_received_counter==0) return string_zoc_return_connected_status_offline;
    else return string_zoc_return_connected_status_online;
}

void zoc_show_bottom_line_footer_connected(void)
{
    menu_footer_clear_bottom_line();
    menu_footer_bottom_line(); //Para actualizar la linea de abajo del todo con texto ZEsarUX version bla bla y ONLINE/OFFLINE si conviene
}


#ifdef USE_PTHREADS

//Funciones que usan pthreads
int zoc_receive_snapshot(int indice_socket);


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

        //Empezar a contar latencia

        struct timeval list_rooms_time_antes,list_rooms_time_despues;

        //calcular tiempo que tarda en dibujarse

	    timer_stats_current_time(&list_rooms_time_antes);

        char server[NETWORK_MAX_URL+1];
        int puerto;
        puerto=zeng_online_get_server_and_port(server);

		int indice_socket=z_sock_open_connection(server,puerto,0,"");


        zeng_online_last_list_rooms_latency=timer_stats_diference_time(&list_rooms_time_antes,&list_rooms_time_despues);

		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
                server,puerto,
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


//Retorna -1 si error.
//Si no error, retorna indice_socket
int zoc_common_open(void)
{

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
            server,puerto,
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


//Funcion comun para enviar un comando con una conexion abierta. Retorna 0 si error
//Guarda resultado en buffer
int zoc_common_send_command_buffer(int indice_socket,char *buffer_enviar,char *command_name_for_info,char *buffer,int max_buffer)
{

    //#define ZENG_BUFFER_INITIAL_CONNECT 199

    //Leer algo
    //char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];
    int posicion_command;

    int escritos=z_sock_write_string(indice_socket,buffer_enviar);


    if (escritos<0) {
        debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online %s: %s",command_name_for_info,z_sock_get_error(escritos));
        return 0;
    }


    int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,max_buffer,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online %s (length %d): \n[\n%s\n]",command_name_for_info,leidos,buffer);
    }

    if (leidos<0) {
        debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online %s: %s",command_name_for_info,z_sock_get_error(leidos));
        return 0;
    }



    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",buffer);
    }
    else {
        debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online %s",command_name_for_info);
        return 0;
    }

    //printf("Retorno %s: [%s]\n",command_name_for_info,buffer);
    //Si hay ERROR
    if (strstr(buffer,"ERROR")!=NULL) {
        debug_printf(VERBOSE_ERR,"Error %s room: %s",command_name_for_info,buffer);
        return 0;
    }



    return 1;

}

//Funcion comun para enviar un comando con una conexion abierta. Retorna 0 si error
int zoc_common_send_command(int indice_socket,char *buffer_enviar,char *command_name_for_info)
{

    #define ZENG_BUFFER_INITIAL_CONNECT 199

    //buffer retorno
    char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

    return zoc_common_send_command_buffer(indice_socket,buffer_enviar,command_name_for_info,buffer,ZENG_BUFFER_INITIAL_CONNECT);


}

//Funcion comun para enviar un comando con una conexion abierta y cerrar conexion. Retorna 0 si error
int zoc_common_send_command_and_close(int indice_socket,char *buffer_enviar,char *command_name_for_info)
{

    int return_value=zoc_common_send_command(indice_socket,buffer_enviar,command_name_for_info);


    if (!return_value) return 0;


    //finalizar conexion
    z_sock_close_connection(indice_socket);

    return 1;

}

//Funcion comun para abrir conexion, enviar comando y cerrar conexion
//Retorna 0 si error
int zoc_open_command_close(char *buffer_enviar,char *command_name_for_info)
{
    int indice_socket=zoc_common_open();
    if (indice_socket<0) {
        return 0;
    }

    debug_printf(VERBOSE_DEBUG,"ZENG Online: Sending %s",command_name_for_info);


    return zoc_common_send_command_and_close(indice_socket,buffer_enviar,command_name_for_info);

}

int zeng_online_client_leave_room_connect(void)
{

    char buffer_enviar[1024];

    sprintf(buffer_enviar,"zeng-online leave %d %s \"%s\"\n",param_join_room_number,created_room_user_password,zeng_online_nickname);

    int return_value=zoc_open_command_close(buffer_enviar,"leave");

    //Desconectar solo si no ha habido error
    if (return_value) {
        printf("Desconexion ok\n");
        zeng_online_connected.v=0;
    }

    return return_value;
}



int parm_zeng_online_client_authorize_join_permissions;

//Devuelve 0 si no conectado
int zeng_online_client_authorize_join_connect(void)
{
           char server[NETWORK_MAX_URL+1];
        int puerto;
        puerto=zeng_online_get_server_and_port(server);

		int indice_socket=z_sock_open_connection(server,puerto,0,"");



		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
                server,puerto,
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



        char buffer_envio_comando[200];
        //authorize-join creator_pass n perm
        sprintf(buffer_envio_comando,"zeng-online authorize-join %s %d %d\n",
            created_room_creator_password,zeng_online_joined_to_room_number,parm_zeng_online_client_authorize_join_permissions);

            printf("Authorizing by: %s\n",buffer_envio_comando);

            printf("Ppermisos: %d\n",parm_zeng_online_client_authorize_join_permissions);


		int escritos=z_sock_write_string(indice_socket,buffer_envio_comando);

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online authorize-join: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online authorize-join (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online authorize-join: %s %s",buffer,z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			debug_printf(VERBOSE_DEBUG,"ZENG: Received zeng-online authorize-join: %s",buffer);
		}
		else {
			debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online authorize-join");
			return 0;
		}

        printf("return authorize-join llist: %s\n",buffer);

		//finalizar conexion
        z_sock_close_connection(indice_socket);



	//zeng_remote_socket=indice_socket;

	return 1;

}

//Devuelve 0 si no conectado
int zeng_online_client_join_list_connect(void)
{




        zeng_remote_join_list_buffer[0]=0;

        //Empezar a contar latencia



        char server[NETWORK_MAX_URL+1];
        int puerto;
        puerto=zeng_online_get_server_and_port(server);

		int indice_socket=z_sock_open_connection(server,puerto,0,"");



		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
                server,puerto,
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






        //Obtener join list
        char buffer_envio_comando[200];

        sprintf(buffer_envio_comando,"zeng-online get-join-first-element-queue %s %d\n",created_room_creator_password,zeng_online_joined_to_room_number);


		int escritos=z_sock_write_string(indice_socket,buffer_envio_comando);

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online list-rooms: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_join_list_buffer,1023,&posicion_command);
		if (leidos>0) {
			zeng_remote_join_list_buffer[leidos]=0; //fin de texto
			debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online list-rooms (length %d): \n[\n%s\n]",leidos,zeng_remote_join_list_buffer);
		}

		if (leidos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online list-rooms: %s %s",zeng_remote_join_list_buffer,z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			zeng_remote_join_list_buffer[posicion_command-1]=0;
			debug_printf(VERBOSE_DEBUG,"ZENG: Received zeng-online list-rooms: %s",zeng_remote_join_list_buffer);
		}
		else {
			debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online list-rooms");
			return 0;
		}

        printf("join llist: %s\n",zeng_remote_join_list_buffer);

		//finalizar conexion
        z_sock_close_connection(indice_socket);



	//zeng_remote_socket=indice_socket;

	return 1;
}

void *zeng_online_client_leave_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_leave_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_leave_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_leave_room_thread_running=0;
		return 0;
	}




	zeng_online_client_leave_room_thread_running=0;

	return 0;

}





int param_zeng_online_client_autojoin_room_permisos=0;

//Devuelve 0 si no conectado
int zeng_online_client_autojoin_room_connect(void)
{
    char buffer_enviar[1024];

    //"set-autojoin creator_pass n p       Define permissions (p) for autojoin on room (n),
    sprintf(buffer_enviar,"zeng-online set-autojoin %s %d %d\n",
        created_room_creator_password,zeng_online_joined_to_room_number,param_zeng_online_client_autojoin_room_permisos);

    return zoc_open_command_close(buffer_enviar,"set-autojoin");

}

int zeng_online_client_destroy_room_connect(void)
{
    char buffer_enviar[1024];

    sprintf(buffer_enviar,"zeng-online destroy-room %s %d\n",created_room_creator_password,zeng_online_joined_to_room_number);

    return zoc_open_command_close(buffer_enviar,"destroy-room");

}



//Devuelve 0 si no conectado
int zeng_online_client_disable_autojoin_room_connect(void)
{

    char buffer_enviar[1024];

    //"reset-autojoin creator_pass n
    sprintf(buffer_enviar,"zeng-online reset-autojoin %s %d\n",
        created_room_creator_password,zeng_online_joined_to_room_number);

    return zoc_open_command_close(buffer_enviar,"reset-autojoin");

}

//Devuelve -1 si error
int zoc_get_message_id(int indice_socket)
{

    char buffer_enviar[1024];

    //get-message-id user_pass n
    sprintf(buffer_enviar,"zeng-online get-message-id %s %d\n",
        created_room_user_password,zeng_online_joined_to_room_number);

    #define ZENG_BUFFER_INITIAL_CONNECT 199

    //buffer retorno
    char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

    int return_value=zoc_common_send_command_buffer(indice_socket,buffer_enviar,"get-message-id",buffer,ZENG_BUFFER_INITIAL_CONNECT);

    //printf("get_message_id: [%s]\n",buffer);

    if (!return_value) {
        return -1;
    }

    return parse_string_to_number(buffer);

}

//Devuelve -1 si error
int zoc_get_message(int indice_socket,char *mensaje)
{
    mensaje[0]=0;

    char buffer_enviar[1024];

    //get-message user_pass n
    sprintf(buffer_enviar,"zeng-online get-message %s %d\n",
        created_room_user_password,zeng_online_joined_to_room_number);

    #define ZENG_BUFFER_INITIAL_CONNECT 199

    //buffer retorno
    char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

    int return_value=zoc_common_send_command_buffer(indice_socket,buffer_enviar,"get-message-id",buffer,ZENG_BUFFER_INITIAL_CONNECT);

    printf("get_message: [%s]\n",buffer);

    if (!return_value) {
        return -1;
    }

    strcpy(mensaje,buffer);

    return 0;

}

char param_zeng_online_client_write_message_room_message[ZENG_ONLINE_MAX_BROADCAST_MESSAGE_LENGTH+1];

//Devuelve 0 si no conectado
int zeng_online_client_write_message_room_connect(void)
{

    char buffer_enviar[1024];

    //"send-message user_pass n nickname message    Sends broadcast message to room\n"
    sprintf(buffer_enviar,"zeng-online send-message %s %d \"%s\" \"%s\"\n",
        created_room_user_password,
        zeng_online_joined_to_room_number,
        zeng_online_nickname,
        param_zeng_online_client_write_message_room_message);

    return zoc_open_command_close(buffer_enviar,"send-message");

}

int param_zeng_online_client_allow_message_room_allow_disallow;

//Devuelve 0 si no conectado
int zeng_online_client_allow_message_room_connect(void)
{

    char buffer_enviar[1024];

    if (param_zeng_online_client_allow_message_room_allow_disallow) {

        //"set-allow-messages creator_pass n
        sprintf(buffer_enviar,"zeng-online set-allow-messages %s %d\n",
            created_room_creator_password,
            zeng_online_joined_to_room_number);

    }

    else {
        //"reset-allow-messages creator_pass n
        sprintf(buffer_enviar,"zeng-online reset-allow-messages %s %d\n",
            created_room_creator_password,
            zeng_online_joined_to_room_number);
    }

    return zoc_open_command_close(buffer_enviar,"set-allow-messages");

}

void *zeng_online_client_autojoin_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_autojoin_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_autojoin_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_autojoin_room_thread_running=0;
		return 0;
	}


	zeng_online_client_autojoin_room_thread_running=0;

	return 0;

}

void *zeng_online_client_disable_autojoin_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_disable_autojoin_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_disable_autojoin_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_disable_autojoin_room_thread_running=0;
		return 0;
	}


	zeng_online_client_disable_autojoin_room_thread_running=0;

	return 0;

}

void *zeng_online_client_write_message_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_write_message_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_write_message_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_write_message_room_thread_running=0;
		return 0;
	}


	zeng_online_client_write_message_room_thread_running=0;

	return 0;

}

void *zeng_online_client_allow_message_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_allow_message_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_allow_message_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_allow_message_room_thread_running=0;
		return 0;
	}


	zeng_online_client_allow_message_room_thread_running=0;

	return 0;

}

void *zeng_online_client_destroy_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_destroy_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_destroy_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_destroy_room_thread_running=0;
		return 0;
	}




	zeng_online_client_destroy_room_thread_running=0;

	return 0;

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


void *zeng_online_client_join_list_function(void *nada GCC_UNUSED)
{

	zeng_online_client_join_list_thread_running=1;



	//Conectar a remoto

	if (!zeng_online_client_join_list_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_join_list_thread_running=0;
		return 0;
	}






	//zeng_enabled.v=1;


	zeng_online_client_join_list_thread_running=0;

	return 0;

}


void *zeng_online_client_authorize_join_function(void *nada GCC_UNUSED)
{

	zeng_online_client_authorize_join_thread_running=1;



	//Conectar a remoto

	if (!zeng_online_client_authorize_join_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_authorize_join_thread_running=0;
		return 0;
	}






	//zeng_enabled.v=1;


	zeng_online_client_authorize_join_thread_running=0;

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

void zeng_online_client_leave_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_leave_room, NULL, &zeng_online_client_leave_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online leave room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_leave_room);


}

void zeng_online_client_autojoin_room(int permisos)
{
    // guardar esos permisos
    param_zeng_online_client_autojoin_room_permisos=permisos;
	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_autojoin_room, NULL, &zeng_online_client_autojoin_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online autojoin pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_autojoin_room);


}

void zeng_online_client_disable_autojoin_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_disable_autojoin_room, NULL, &zeng_online_client_disable_autojoin_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online disable autojoin pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_disable_autojoin_room);


}



void zeng_online_client_write_message_room(char *message)
{

	//Inicializar thread
    strcpy(param_zeng_online_client_write_message_room_message,message);

	if (pthread_create( &thread_zeng_online_client_write_message_room, NULL, &zeng_online_client_write_message_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online write message pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_write_message_room);


}



//Parametro allow_disallow : 1: allow, 0: disallow
void zeng_online_client_allow_message_room(int allow_disallow)
{

    param_zeng_online_client_allow_message_room_allow_disallow=allow_disallow;



	if (pthread_create( &thread_zeng_online_client_allow_message_room, NULL, &zeng_online_client_allow_message_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online allow/disallow message pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_allow_message_room);


}

void zeng_online_client_destroy_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_destroy_room, NULL, &zeng_online_client_destroy_room_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online destroy room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_destroy_room);


}

void zeng_online_client_join_list(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_join_list, NULL, &zeng_online_client_join_list_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online join list pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_join_list);


}

void zeng_online_client_authorize_join(int permissions)
{

	//Inicializar thread
    parm_zeng_online_client_authorize_join_permissions=permissions;

	if (pthread_create( &thread_zeng_online_client_authorize_join, NULL, &zeng_online_client_authorize_join_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online authorize join pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_authorize_join);


}



//Devuelve 0 si no conectado
int zeng_online_client_join_room_connect(void)
{




        zeng_remote_list_rooms_buffer[0]=0;

        char server[NETWORK_MAX_URL+1];
        int puerto;
        puerto=zeng_online_get_server_and_port(server);

		int indice_socket=z_sock_open_connection(server,puerto,0,"");

		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
                server,puerto,
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

        //Join desde el master le envia el creator password
        if (param_join_room_creator_password[0]!=0) {
            sprintf(buffer_enviar,"zeng-online join %d \"%s\" %s\n",param_join_room_number,zeng_online_nickname,param_join_room_creator_password);
        }
        else {
            sprintf(buffer_enviar,"zeng-online join %d \"%s\"\n",param_join_room_number,zeng_online_nickname);
        }

		int escritos=z_sock_write_string(indice_socket,buffer_enviar);

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-online join: %s",z_sock_get_error(escritos));
			return 0;
		}

        //Esperar la autorizacion, hasta numero reintentos
        //TODO: ajustar esto hasta 1 o 2 minutos por ejemplo. dependera de los reintentos y sleeps de zsock_read_all_until_command
        int reintentos=30;

		//reintentar
        do {
            leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
            if (leidos>0) {
                buffer[leidos]=0; //fin de texto
                debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-online join (length %d): \n[\n%s\n]",leidos,buffer);
            }

            if (leidos<0) {
                debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-online join: %s",z_sock_get_error(leidos));
                return 0;
            }

            if (leidos==0) {
                printf("no leida respuesta aun de join\n");
            }

            reintentos--;
        } while (leidos==0 && reintentos>0);

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

        //Dividir el user_password y los permisos
        int i;
        for (i=0;buffer[i] && buffer[i]!=' ';i++);

        if (buffer[i]==0) {
            //llegado al final sin que haya espacios. error
            debug_printf(VERBOSE_ERR,"Error joining room, no permissions detected: %s",buffer);
            return 0;
        }

        //truncar de momento hasta aqui
        buffer[i]=0;
        strcpy(created_room_user_password,buffer);

        //Y seguir hasta buscar los permisos
        i++;
        created_room_user_permissions=parse_string_to_number(&buffer[i]);

        printf("User password: [%s]\n",created_room_user_password);
        printf("User permissions: [%d]\n",created_room_user_permissions);


		//finalizar conexion
        z_sock_close_connection(indice_socket);

        zeng_online_connected.v=1;

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

void zeng_online_client_join_room(int room_number,char *creator_password)
{

	//Inicializar thread
    //Paso de parametro mediante variable estatica
    //TODO: esto es horrible porque como entren dos o mas join a la vez, el parametro esta compartido
    param_join_room_number=room_number;
    strcpy(param_join_room_creator_password,creator_password);

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

        char server[NETWORK_MAX_URL+1];
        int puerto;
        puerto=zeng_online_get_server_and_port(server);

		int indice_socket=z_sock_open_connection(server,puerto,0,"");

		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
                server,puerto,
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

//Cada cuanto enviar frames de video. TODO parametro configurable, aunque deberia ser un valor muy bajo siempre
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

        //printf("Sending command: [%s]\n",buffer_comando);

        escritos=z_sock_write_string(indice_socket,buffer_comando);
        //printf("after z_sock_write_string 1\n");
        if (escritos<0) return escritos;


        //TODO esto es ineficiente y que tiene que calcular la longitud. hacer otra z_sock_write sin tener que calcular
        //printf("before z_sock_write_string 2\n");
        //printf("Sending snapshot data length: %lu\n",strlen(zoc_send_snapshot_mem_hexa));
        //printf("First bytes of snapshot: %c%c%c%c\n",
          //  zoc_send_snapshot_mem_hexa[0],zoc_send_snapshot_mem_hexa[1],zoc_send_snapshot_mem_hexa[2],zoc_send_snapshot_mem_hexa[3]);

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

        //printf("Recibido respuesta despues de put-snapshot: [%s]\n",buffer);
        return leidos;


}




void zoc_get_keys(int indice_socket)
{

    //Ver si hay datos disponibles en el socket
	int sock_number=get_socket_number(indice_socket);

	if (sock_number<0) {
		return;
	}

    //printf("see if data available on zoc_get_keys\n");

    if (!zsock_available_data(sock_number)) {
        //printf("no data available on zoc_get_keys\n");
        return;
    }



        //Ir leyendo cada linea
        //buffer suficientemente grande por si llegan varios eventos de golpe
        char buffer[1024];

        //Leer hasta prompt
        int posicion_command;

        //printf ("antes de leer hasta command prompt\n");
        int leidos=zsock_read_all_until_newline(indice_socket,(z80_byte *)buffer,1023,&posicion_command);
        //printf("leidos on zoc_get_keys: %d\n",leidos);

        if (leidos>0) {
            buffer[leidos]=0; //fin de texto
            //printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer);
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
                //printf("procesar linea: [%s]\n",&buffer[inicio_linea]);
                ////Returned format is: uuid key event nomenu"

                //campo_inicio y final almacenan el caracter "#"

                //La longitud de los campos casi ninguno deberia exceder mas de 4 o 5 caracteres,
                //pero que dado que podemos haber recibido una respuesta parcial (si la anterior llamada a zsock_read_all_until_newline
                //no era completa) los campos pueden haber llegado en mal orden y por tanto tendremos que considerar el maximo
                //ancho de todos los campos, que es STATS_UUID_MAX_LENGTH
                char campo_inicio[STATS_UUID_MAX_LENGTH+1];
                char campo_final[STATS_UUID_MAX_LENGTH+1];

                char received_uuid[STATS_UUID_MAX_LENGTH+1];
                char received_key[STATS_UUID_MAX_LENGTH+1];
                char received_event[STATS_UUID_MAX_LENGTH+1];
                char received_nomenu[STATS_UUID_MAX_LENGTH+1];
                //ir procesando segun espacios
                int campos_leidos=0;

                int j;
                int inicio_campo=inicio_linea;
                for (j=inicio_linea;buffer[j] && campos_leidos<6;j++) {
                    if (buffer[j]==' ') {
                        buffer[j]=0;
                        //printf("read field: campos_leidos: %d\n",campos_leidos);
                        //printf("read field: [%s]\n",&buffer[inicio_campo]);

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
                    //printf("read last field: [%s]\n",&buffer[inicio_campo]);
                    strcpy(campo_final,&buffer[inicio_campo]);

                    //printf("Received event: uuid: [%s] key: [%s] event: [%s] nomenu: [%s]\n",
                      //  received_uuid,received_key,received_event,received_nomenu);

                    //Validar que empiece y acabe con "#"
                    int valido=0;

                    if (!strcmp(campo_inicio,"#") && !strcmp(campo_final,"#")) valido=1;

                    if (!valido) printf("linea no valida\n");
                    else {

                        //Si uuid yo soy mismo, no procesarlo
                        if (!strcmp(received_uuid,stats_uuid)) {
                            //printf("The event is mine. Do not process it\n");
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
                                //printf ("Processing from ZRCP command send-keys-event: key %d event %d\n",numero_key,numero_event);

                                debug_printf (VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
                                    UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);

                                //Si tecla especial de reset todas teclas. usado en driver curses
                                if (numero_key==UTIL_KEY_RESET_ALL) {
                                    //printf("Reset todas teclas\n");
                                    reset_keyboard_ports();
                                }

                                else {
                                    //printf("Recibida tecla desde zeng online. Antes Puerto puerto_61438: %d\n",puerto_61438);
                                    util_set_reset_key_continue_after_zeng(numero_key,numero_event);
                                    //printf("Recibida tecla desde zeng online. Despues Puerto puerto_61438: %d\n",puerto_61438);
                                }
                            }
                        }
                    }
                }

                else {
                    printf("Recibida respuesta incorrecta con diferentes campos de los esperados\n");
                }


                inicio_linea=i+1;
            }
        }

}

//Contador desde el ultimo snapshot enviado/recibido, se incrementa con cada scanline
//int zeng_online_scanline_counter=0;

/*
void zeng_online_client_reset_scanline_counter(void)
{
    zeng_online_scanline_counter=0;
}

void zeng_online_client_increment_scanline_counter(void)
{
    //Aunque es un simple incrementado de contador, solo hacer esto cuando este activado
    //porque aqui se llama desde cada core_ y por si se agrega algo mas aqui, solo hacerlo cuando este activado
    if (zeng_online_connected.v==0) return;

    zeng_online_scanline_counter++;
}
*/

int zeng_online_client_end_frame_reached=0;

void zeng_online_client_tell_end_frame(void)
{
    zeng_online_client_end_frame_reached=1;
}

int zoc_start_connection_get_keys(void)
{
    //conectar a remoto

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);
    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
            server,puerto,
            z_sock_get_error(indice_socket));
        return 0;
    }

        int posicion_command;

#define ZENG_BUFFER_INITIAL_CONNECT 199

    //Leer algo
    char buffer_initial[ZENG_BUFFER_INITIAL_CONNECT+1];

    //int leidos=z_sock_read(indice_socket,buffer,199);
    int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
    printf("leidos on zoc_start_connection_get_keys: %d\n",leidos);
    if (leidos>0) {
        buffer_initial[leidos]=0; //fin de texto
        printf("Received text on zoc_start_connection_get_keys (length: %d):\n[\n%s\n]\n",leidos,buffer_initial);
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



        printf ("before read command sending get-keys on zoc_start_connection_get_keys\n");
        //1 segundo maximo de reintentos
        //sobretodo util en la primera conexion, dado que el buffer vendra vacio, siempre esperara 1 segundo
        int leidos=zsock_read_all_until_command_max_reintentos(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command,100);
        printf ("after read command sending get-keys on zoc_start_connection_get_keys. leidos=%d\n",leidos);

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
            //printf("Received text after send-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer);
        }

        if (leidos<0) {
            debug_printf(VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            return -1;
        }


    }


    return escritos;
}

int zoc_keys_send_pending(int indice_socket,int *enviada_alguna_tecla)
{

    *enviada_alguna_tecla=0;

    int error_desconectar=0;
    zeng_key_presses elemento;
    while (!zeng_fifo_read_element(&elemento) && !error_desconectar) {
        *enviada_alguna_tecla=1;
        debug_printf (VERBOSE_DEBUG,"ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d",elemento.tecla,elemento.pressrelease);

        //printf ("ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d\n",elemento.tecla,elemento.pressrelease);

        debug_printf (VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
            UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);

        //command> help send-keys-event
        //Syntax: send-keys-event key event
            int error=zoc_send_keys(indice_socket,&elemento);

            if (error<0) error_desconectar=1;

    }

    return error_desconectar;
}

int zoc_get_pending_authorization_size_last_queue_size=0;

int zoc_get_pending_authorization_size(int indice_socket)
{
    //printf("Inicio zoc_receive_snapshot llamado desde:\n");
    //debug_exec_show_backtrace();


    int posicion_command;
    int escritos,leidos;


    char buffer_comando[200];



                #define ZENG_BUFFER_INITIAL_CONNECT 199

                //Leer algo
                char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

                //zo get-join-queue-size LPBPPHZPXV 0

                sprintf(buffer_comando,"zeng-online get-join-queue-size %s %d\n",created_room_creator_password,zeng_online_joined_to_room_number);
                escritos=z_sock_write_string(indice_socket,buffer_comando);
                //printf("after z_sock_write_string 1\n");
                if (escritos<0) return escritos;

                //Leer hasta prompt
                //printf("before zsock_read_all_until_command\n");
                leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);

                if (leidos>0) {
                    buffer[leidos]=0; //fin de texto
                    debug_printf(VERBOSE_DEBUG,"ZENG: Received text for get-join-queue-size (length %d): \n[\n%s\n]",leidos,buffer);
                }

                if (leidos<0) {
                    debug_printf(VERBOSE_ERR,"ERROR. Can't receive get-join-queue-size: %s",z_sock_get_error(leidos));
                    return 0;
                }


                //printf("after zsock_read_all_until_command\n");
                //printf("Recibido respuesta despues de get-snapshot-id: [%s]\n",buffer);

                //1 mas para eliminar el salto de linea anterior a "command>"
                if (posicion_command>=1) {
                    buffer[posicion_command-1]=0;
                    //debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                }
                else {
                    debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX get-join-queue-size");
                    return 0;
                }

                //printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

                int leer_snap=0;

                //Detectar si error
                if (strstr(buffer,"ERROR")!=NULL) {
                    printf("Error getting get-join-queue-size\n");
                }
                else {
                    //Ver si id diferente
                    int queue_size=parse_string_to_number(buffer);
                    printf("Waiting room queue size: %d\n",queue_size);

                    if (queue_size) {

                        if (queue_size!=zoc_get_pending_authorization_size_last_queue_size) {
                            //Escribir en footer solo cuando valor anterior cambia
                            //TODO quiza habria que usar algun tipo de bloqueo para esto


                            zoc_get_pending_authorization_size_last_queue_size=queue_size-1;

                            //Y lo mostramos en el footer
                            char mensaje[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH+ZOC_MAX_NICKNAME_LENGTH+1];

                            sprintf(mensaje,"ZOC clients waiting on room %d",zeng_online_joined_to_room_number);

                            //Por si acaso truncar al maximo que permite el footer
                            mensaje[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH]=0;

                            put_footer_first_message(mensaje);

                            //Y enviarlo a speech
                            textspeech_print_speech(mensaje);

                        }


                    }
                }


    return 1;


}
int contador_obtener_mensajes=0;
int zoc_last_message_id=0;

//En el rejoin como master se aplicara el primer snapshot recibido del server
int zoc_pending_apply_received_snapshot_as_rejoin_as_master=0;

void zoc_common_get_messages_slave_master(int indice_socket)
{

    //Tambien a final de cada frame, y cada 50 veces (o sea 1 segundo), ver si hay pendientes mensajes
    contador_obtener_mensajes++;

    if ((contador_obtener_mensajes % 50)==0) {
        int id_actual=zoc_get_message_id(indice_socket);
        if (id_actual!=zoc_last_message_id) {
            printf("Hay nuevo mensaje\n");
            char buffer_mensaje[ZENG_ONLINE_MAX_BROADCAST_MESSAGE_SHOWN_LENGTH+1];
            zoc_get_message(indice_socket,buffer_mensaje);
            if (buffer_mensaje[0]) {

                //Y lo mostramos en el footer
                char mensaje[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH+ZENG_ONLINE_MAX_BROADCAST_MESSAGE_SHOWN_LENGTH+1];

                sprintf(mensaje,"%s",buffer_mensaje);

                //Por si acaso truncar al maximo que permite el footer
                mensaje[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH]=0;

                put_footer_first_message(mensaje);

                //Y enviarlo a speech
                textspeech_print_speech(mensaje);

            }


            zoc_last_message_id=id_actual;
        }
    }
}

void *zoc_master_thread_function(void *nada GCC_UNUSED)
{

    //conectar a remoto
    //Inicializar timeout para no recibir tempranos mensajes de "OFFLINE"
    zoc_last_snapshot_received_counter=ZOC_TIMEOUT_NO_SNAPSHOT;

    //zeng_remote_list_rooms_buffer[0]=0;

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    int contador_obtener_autorizaciones=0;

    if (indice_socket<0) {
        debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
            server,puerto,
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


    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    zoc_last_message_id=zoc_get_message_id(indice_socket);
    printf("Initial message id: %d\n",zoc_last_message_id);

    int indice_socket_get_keys=zoc_start_connection_get_keys();

    //bucle continuo de si hay snapshot de final de frame, enviarlo a remoto
    //TODO: ver posible manera de salir de aqui??
    while (1) {

        if (zeng_online_client_end_frame_reached) {
            zeng_online_client_end_frame_reached=0;

            //Nota: un master normal deberia tener todos permisos, sin necesidad de comprobarlos
            //PERO si queremos un master que solo gestione autorizaciones, cambios de parametros en room, etc,
            //sin que envie snapshots o reciba teclas o envie teclas, se le pueden quitar dichos permisos,
            //y podremos gestionar el resto

            //printf("before get snapshot\n");

            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT) {


                if (zoc_pending_send_snapshot) {
                    int error=zoc_send_snapshot(indice_socket);

                    if (error<0) {
                        //TODO
                        printf("Error sending snapshot to zeng online server\n");
                    }

                    //Enviado. Avisar no pendiente
                    zoc_pending_send_snapshot=0;
                    //zeng_online_client_reset_scanline_counter();
                    //printf("Snapshot sent\n");
                }
            }

            //Un master que recibe snapshots
            //Esto ya no se usa, era inicialmente un manager que recibia snapshots
            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT) {

                printf("Recibiendo snapshot porque somos master manager. contador_segundo=%d\n",contador_segundo);
                int error=zoc_receive_snapshot(indice_socket);
                //TODO gestionar bien este error
                if (error<0) {
                    //TODO
                    printf("Error getting snapshot from zeng online server\n");
                    usleep(10000); //dormir 10 ms
                }

            }

            //Si estabamos reentrando como master, obtener el snapshot que habia
            if (zoc_rejoining_as_master) {
                printf("Receiving first snapshot as we are rejoining as master\n");
                zoc_rejoining_as_master=0;
                zoc_pending_apply_received_snapshot_as_rejoin_as_master=1;

                zoc_receive_snapshot(indice_socket);
            }

            //Enviar teclas
            //TODO gestionar error_desconectar
            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS) {
                int enviada_alguna_tecla;
                int error_desconectar=zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);
            }

            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_KEYS) {
                //recepcion teclas
                zoc_get_keys(indice_socket_get_keys);
            }

            //printf("Contador scanline: %d\n",zeng_online_scanline_counter);




            //Tambien a final de cada frame, y cada 50 veces (o sea 1 segundo), ver si hay pendientes autorizaciones
            contador_obtener_autorizaciones++;

            if ((contador_obtener_autorizaciones % 50)==0) {
                zoc_get_pending_authorization_size(indice_socket);
            }

            zoc_common_get_messages_slave_master(indice_socket);

        }

        //TODO: parametro configurable
        usleep(1000); //1 ms (20 ms es un frame de video)

    }

	return 0;

}

//Pruebas
//int esperar_por_envio_alguna_tecla=0;
/*void temp_esperar_por_envio_alguna_tecla(void)
{
    esperar_por_envio_alguna_tecla=50;
}*/

//int zoc_client_pulsada_alguna_tecla_local=0;

/*void zoc_decir_pulsada_alguna_tecla_local(void)
{
    zoc_client_pulsada_alguna_tecla_local=1;
}*/



char *zoc_get_snapshot_mem_hexa=NULL;
z80_byte *zoc_get_snapshot_mem_binary=NULL;
int zoc_get_snapshot_mem_binary_longitud=0;
int zoc_pending_apply_received_snapshot=0;

int zoc_receive_snapshot_last_id=0;

int zoc_receive_snapshot(int indice_socket)
{
    //printf("Inicio zoc_receive_snapshot llamado desde:\n");
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
                //printf("Recibido respuesta despues de get-snapshot-id: [%s]\n",buffer);

                //1 mas para eliminar el salto de linea anterior a "command>"
                if (posicion_command>=1) {
                    buffer[posicion_command-1]=0;
                    //debug_printf(VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                }
                else {
                    debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX get-snapshot-id");
                    return 0;
                }

                //printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

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
                        //printf("Snapshot es el mismo que el anterior\n");
                    }
                }



                if (leer_snap) {

                    //printf("Obteniendo snapshot\n");

                    //printf("antes enviar get-snaps\n");
                    //get-snapshot user_pass n
                    sprintf(buffer_comando,"zeng-online get-snapshot %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);
                    escritos=z_sock_write_string(indice_socket,buffer_comando);
                    //printf("after z_sock_write_string 1\n");
                    if (escritos<0) return escritos;

                    //int sock=get_socket_number(indice_socket);


                    //printf("despues enviar get-snaps\n");



            //Ver si hay datos disponibles y no esta pendiente aplicar ultimo snapshot

                    if (zoc_get_snapshot_mem_hexa==NULL) {
                        zoc_get_snapshot_mem_hexa=util_malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2,"Can not allocate memory for getting snapshot"); //16 MB es mas que suficiente
                    }

                    //Leer hasta prompt
                    //printf("before zsock_read_all_until_command\n");
                    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zoc_get_snapshot_mem_hexa,ZRCP_GET_PUT_SNAPSHOT_MEM*2,&posicion_command);

                    //printf("leidos despues de get-snapshot: %d\n",leidos);


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
                        debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-online get-snapshot");
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
            //printf("get-snapshot no disponible. esperar\n");

        }

        //Esperar algo. 10 ms, suficiente porque es un mitad de frame
        //usleep(10000); //dormir 10 ms

    //}
    return 1;


}





void *zoc_slave_thread_function(void *nada GCC_UNUSED)
{

    //conectar a remoto
    //Inicializar timeout para no recibir tempranos mensajes de "OFFLINE"
    zoc_last_snapshot_received_counter=ZOC_TIMEOUT_NO_SNAPSHOT;

    //zeng_remote_list_rooms_buffer[0]=0;

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
            server,puerto,
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

    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    zoc_last_message_id=zoc_get_message_id(indice_socket);
    printf("Initial message id: %d\n",zoc_last_message_id);


    int indice_socket_get_keys=zoc_start_connection_get_keys();

    //bucle continuo de recepcion snapshot
    //TODO: ver posible manera de salir de aqui??

    int temppppp=0;

    while (1) {

        if (zeng_online_client_end_frame_reached) {
            zeng_online_client_end_frame_reached=0;

            //Como se puede ver es el cliente quien gestiona los permisos en base a lo que el join le retornó el server
            //Pero luego el server no valida que el cliente tenga los permisos que está usando
            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT) {

                //Recibir snapshot


                int error=zoc_receive_snapshot(indice_socket);
                //TODO gestionar bien este error
                if (error<0) {
                    //TODO
                    printf("Error getting snapshot from zeng online server\n");
                    usleep(10000); //dormir 10 ms
                }


            }

            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_KEYS) {
                //recepcion teclas
                //TODO gestionar error
                //Tecnicamente no haria falta esto, pues las teclas ya nos llegan
                //por el volcado de puertos que tiene el snapshot
                //PERO esto permite que lleguen mas rapido antes que el snapshot
                zoc_get_keys(indice_socket_get_keys);
            }


            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS) {
                //Enviar teclas
                //TODO gestionar error_desconectar
                int enviada_alguna_tecla;
                int error_desconectar=zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);


            }

            zoc_common_get_messages_slave_master(indice_socket);

            //printf("siguiente segundo. contador_segundo=%d. temppppp=%d\n",contador_segundo,temppppp++);


        }

        //TODO este parametro configurable
        usleep(1000); //1 ms (20 ms es un frame de video)

    }

	return 0;

}










int zoc_start_connection_get_snapshot(void)
{
    //conectar a remoto

    //zeng_remote_list_rooms_buffer[0]=0;

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
            server,puerto,
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





void zeng_online_client_apply_pending_received_snapshot(void)
{
    if (zeng_online_connected.v==0) return;

    if (!zoc_pending_apply_received_snapshot) return;

    //Permitir aplicar el primer snapshot que hemos recibido al hacer rejoin as master
    if (zoc_pending_apply_received_snapshot_as_rejoin_as_master) {
        printf("Permitir primer snapshot recibido con rejoin\n");
        zoc_pending_apply_received_snapshot_as_rejoin_as_master=0;
    }

    //O si somos un master con permisos de get snapshot
    //o sea lo que era inicialmente un manager master, que recibia snapshots
    //Esto ya no se usa, un manager no recibe snapshots
    else if (zeng_online_i_am_master.v && (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT)) {
        printf("Permitir aplicar snapshot porque somos solo manager\n");
    }

    else {
        if (zeng_online_i_am_master.v) return;
    }


    //printf("Putting snapshot coming from ZRCP: %p %d\n",zoc_get_snapshot_mem_binary,zoc_get_snapshot_mem_binary_longitud);

    load_zsf_snapshot_file_mem(NULL,zoc_get_snapshot_mem_binary,zoc_get_snapshot_mem_binary_longitud,1,1);

    free(zoc_get_snapshot_mem_binary);
    zoc_get_snapshot_mem_binary=NULL;

    zoc_pending_apply_received_snapshot=0;

    //Si estaba offline, reactualizamos
    if (zoc_last_snapshot_received_counter==0) {
        zoc_show_bottom_line_footer_connected(); //Para actualizar la linea de abajo del todo con texto ZEsarUX version bla bla - ONLINE
        generic_footertext_print_operating("ONLINE");
    }

    //5 segundos de timeout, para aceptar teclas slave si no hay snapshot
    zoc_last_snapshot_received_counter=ZOC_TIMEOUT_NO_SNAPSHOT;

    //zeng_online_client_reset_scanline_counter();

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

void zoc_stop_master_thread(void)
{
    pthread_cancel(thread_zoc_master_thread);
}

void zoc_stop_slave_thread(void)
{
    pthread_cancel(thread_zoc_slave_thread);
    zoc_pending_apply_received_snapshot=0;
}






void zeng_online_client_prepare_snapshot_if_needed(void)
{

	if (zeng_online_connected.v==0) return;

	if (zeng_online_i_am_master.v) {
		zoc_contador_envio_snapshot++;
		//printf ("%d %d\n",contador_envio_snapshot,(contador_envio_snapshot % (50*zeng_segundos_cada_snapshot) ));


		if (zoc_contador_envio_snapshot>=zoc_frames_video_cada_snapshot && !zoc_rejoining_as_master) {
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

  					save_zsf_snapshot_file_mem(NULL,buffer_temp,&longitud,1);


                    //temp_memoria_asignada++;
                    //printf("Asignada: %d liberada: %d\n",temp_memoria_asignada,temp_memoria_liberada);

					if (zoc_send_snapshot_mem_hexa==NULL) zoc_send_snapshot_mem_hexa=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2); //16 MB es mas que suficiente

					int char_destino=0;

					int i;

                    z80_byte *origen=buffer_temp;
                    char *destino=zoc_send_snapshot_mem_hexa;
                    z80_byte byte_leido;

					for (i=0;i<longitud;i++,char_destino +=2) {
                        //En vez de sprintf, que es poco optimo, usar alternativa
						//sprintf (&zoc_send_snapshot_mem_hexa[char_destino],"%02X",buffer_temp[i]);
                        byte_leido=*origen++;
                        *destino++=util_byte_to_hex_nibble(byte_leido>>4);
                        *destino++=util_byte_to_hex_nibble(byte_leido);

					}

					//metemos salto de linea y 0 al final
					strcpy (&zoc_send_snapshot_mem_hexa[char_destino],"\n");

					debug_printf (VERBOSE_DEBUG,"ZENG Online: Queuing snapshot to send, length: %d",longitud);

                    //printf ("ZENG Online: Queuing snapshot to send, length: %d\n",longitud);


					//Liberar memoria que ya no se usa
					free(buffer_temp);


					zoc_pending_send_snapshot=1;


				}
		}
	}
}


void zeng_online_client_end_frame_from_core_functions(void)
{

    if (zeng_online_connected.v==0) return;

    zeng_online_client_tell_end_frame();

    //El orden en el caso de rejoin as master es importante,
    //primero aplicaremos el snapshot recibido al entrar como master,
    //luego ya iremos enviando los siguientes como master normal
    zeng_online_client_apply_pending_received_snapshot();
    zeng_online_client_prepare_snapshot_if_needed();



    if (zeng_online_i_am_master.v==0) {
        if (zoc_last_snapshot_received_counter>0) {
            zoc_last_snapshot_received_counter--;

            if (zoc_last_snapshot_received_counter==0) {
                printf("Timeout receiving snapshots from master. Allowing local key press\n");
                zoc_show_bottom_line_footer_connected(); //Para actualizar la linea de abajo del todo con texto ZEsarUX version bla bla - OFFLINE
                generic_footertext_print_operating("OFFLIN");
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

void zeng_online_client_join_list(void)
{
}

void zeng_online_client_authorize_join(int permissions)
{
}

void zeng_online_client_leave_room(void)
{
}

void zeng_online_client_destroy_room(void)
{
}

void zeng_online_client_create_room(int room_number,char *room_name)
{
}

void zoc_start_master_thread(void)
{
}

//Inicio del thread de slave
void zoc_start_slave_thread(void)
{
}

void zoc_stop_slave_thread(void)
{
}

void zoc_stop_master_thread(void)
{
}

void zeng_online_client_end_frame_from_core_functions(void)
{
}

void zeng_online_client_autojoin_room(int permisos)
{
}

void zeng_online_client_disable_autojoin_room(void)
{
}

void zeng_online_client_write_message_room(char *message)
{
}

void zeng_online_client_allow_message_room(int allow_disallow)
{
}

void zeng_online_client_join_room(int room_number,char *creator_password)
{
}

#endif