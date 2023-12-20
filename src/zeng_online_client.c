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
#include "settings.h"



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
pthread_t thread_zeng_online_client_list_users;
pthread_t thread_zeng_online_client_get_profile_keys;
pthread_t thread_zeng_online_client_send_profile_keys;
pthread_t thread_zeng_online_client_kick_user;
pthread_t thread_zeng_online_client_max_players_room;


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
int zeng_online_client_list_users_thread_running=0;
int zeng_online_client_get_profile_keys_thread_running=0;
int zeng_online_client_send_profile_keys_thread_running=0;
int zeng_online_client_kick_user_thread_running=0;
int zeng_online_client_max_players_room_thread_running=0;

z80_bit zeng_online_i_am_master={0};
//z80_bit zeng_online_i_am_joined={0};
int zeng_online_joined_to_room_number=0;

int zoc_pending_send_snapshot=0;

//Si esta conectado
z80_bit zeng_online_connected={0};

//Si la ultima habitacion se ha creado.
z80_bit zeng_online_room_was_created={0};

char zeng_online_server[NETWORK_MAX_URL+1]="localhost";
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

//Perfiles de teclas que se han cargado del servidor como master/manager para la habitacion actual
int allowed_keys[ZOC_MAX_KEYS_PROFILES][ZOC_MAX_KEYS_ITEMS];
//Perfiles asignados a cada uuid. Si es "", no esta asignado
char allowed_keys_assigned[ZOC_MAX_KEYS_PROFILES][STATS_UUID_MAX_LENGTH+1];


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


		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Getting zeng-online rooms");
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
			return 0;
		}

		//zsock_wait_until_command_prompt(indice_socket);

		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Sending get-version");

		//Enviar un get-version
		int escritos=z_sock_write_string(indice_socket,"get-version\n");

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send get-version: %s",z_sock_get_error(escritos));
			return 0;
		}


		//reintentar
		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text for get-version (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive version: %s",z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received version: %s",buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX remote version");
			return 0;
		}

		//Comprobar que version remota sea como local
        char myversion[30];
        util_get_emulator_version_number(myversion);
        if (strcasecmp(myversion,buffer)) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Local and remote ZEsarUX versions do not match");
            //printf("Local %s remote %s\n",myversion,buffer);
			return 0;
		}


		//Comprobar que, si nosotros somos master, el remoto no lo sea
		//El usuario puede activarlo, aunque no es recomendable. Yo solo compruebo y ya que haga lo que quiera


        //ver si zeng online enabled
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online: Getting zeng-online status");

		escritos=z_sock_write_string(indice_socket,"zeng-online is-enabled\n");

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online is-enabled: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online is-enabled (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online is-enabled: %s",z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received zeng-online is-enabled: %s",buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online is-enabled");
			return 0;
		}

        //Si somos master, que el remoto no lo sea tambien
        int esta_activado=parse_string_to_number(buffer);
        if (!esta_activado) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online is not enabled on remote server");
            return 0;
        }


        //Obtener rooms



		escritos=z_sock_write_string(indice_socket,"zeng-online list-rooms\n");

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online list-rooms: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_list_rooms_buffer,total_listado_rooms_memoria,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online list-rooms (length %d): \n[\n%s\n]",leidos,zeng_remote_list_rooms_buffer);
		}

		if (leidos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online list-rooms: %s %s",zeng_remote_list_rooms_buffer,z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			zeng_remote_list_rooms_buffer[posicion_command-1]=0;
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received zeng-online list-rooms",zeng_remote_list_rooms_buffer);
            //printf("ZENG: Received zeng-online list-rooms: %s\n",zeng_remote_list_rooms_buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online list-rooms");
			return 0;
		}

        //printf("Habitaciones: %s\n",zeng_remote_list_rooms_buffer);

		//finalizar conexion
        z_sock_close_connection(indice_socket);



	//zeng_remote_socket=indice_socket;

	return 1;
}

char *zeng_remote_list_users_buffer=NULL;

//Devuelve 0 si no conectado
int zeng_online_client_list_users_connect(void)
{



        //Calculo aproximado de memoria necesaria para listado de usuarios
        int espacio_necesario=ZENG_ONLINE_MAX_PLAYERS_PER_ROOM*(ZOC_MAX_NICKNAME_LENGTH+STATS_UUID_MAX_LENGTH+30); //+30 mas que suficiente margen



		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online: Getting zeng-online users");
        if (zeng_remote_list_users_buffer==NULL) {

            zeng_remote_list_users_buffer=util_malloc(espacio_necesario,"Can not allocate memory for getting list-users");

        }
        zeng_remote_list_users_buffer[0]=0;



        char server[NETWORK_MAX_URL+1];
        int puerto;
        puerto=zeng_online_get_server_and_port(server);

		int indice_socket=z_sock_open_connection(server,puerto,0,"");




		if (indice_socket<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
			return 0;
		}

        char buffer_comando[200];
        sprintf(buffer_comando,"zeng-online list-users %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);

		int escritos=z_sock_write_string(indice_socket,buffer_comando);

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online list-users: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_list_users_buffer,espacio_necesario,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online list-users (length %d): \n[\n%s\n]",leidos,zeng_remote_list_users_buffer);
		}

		if (leidos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online list-users: %s %s",zeng_remote_list_users_buffer,z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			zeng_remote_list_users_buffer[posicion_command-1]=0;
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received zeng-online list-users: %s",zeng_remote_list_users_buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online list-users");
			return 0;
		}

        //printf("usuarios: %s\n",zeng_remote_list_users_buffer);

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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online %s: %s",command_name_for_info,z_sock_get_error(escritos));
        return 0;
    }


    int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,max_buffer,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online %s (length %d): \n[\n%s\n]",command_name_for_info,leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online %s: %s",command_name_for_info,z_sock_get_error(leidos));
        return 0;
    }



    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text: %s",buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online %s",command_name_for_info);
        return 0;
    }

    //printf("Retorno %s: [%s]\n",command_name_for_info,buffer);
    //Si hay ERROR
    if (strstr(buffer,"ERROR")!=NULL) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error %s room: %s",command_name_for_info,buffer);
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

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online: Sending %s",command_name_for_info);


    return zoc_common_send_command_and_close(indice_socket,buffer_enviar,command_name_for_info);

}

int zeng_online_client_leave_room_connect(void)
{

    char buffer_enviar[1024];

    sprintf(buffer_enviar,"zeng-online leave %d %s \"%s\"\n",param_join_room_number,created_room_user_password,stats_uuid);

    int return_value=zoc_open_command_close(buffer_enviar,"leave");

    //Desconectar aunque haya habido error
    if (return_value) {
        //printf("Desconexion ok\n");
    }


    zeng_online_connected.v=0;


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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
			return 0;
		}



        char buffer_envio_comando[200];
        //authorize-join creator_pass n perm
        sprintf(buffer_envio_comando,"zeng-online authorize-join %s %d %d\n",
            created_room_creator_password,zeng_online_joined_to_room_number,parm_zeng_online_client_authorize_join_permissions);

        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Authorizing by: %s",buffer_envio_comando);

        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Permissions: %d",parm_zeng_online_client_authorize_join_permissions);


		int escritos=z_sock_write_string(indice_socket,buffer_envio_comando);

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online authorize-join: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online authorize-join (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online authorize-join: %s %s",buffer,z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received zeng-online authorize-join: %s",buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online authorize-join");
			return 0;
		}

        //printf("return authorize-join list: %s\n",buffer);

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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
			return 0;
		}






        //Obtener join list
        char buffer_envio_comando[200];

        sprintf(buffer_envio_comando,"zeng-online get-join-first-element-queue %s %d\n",created_room_creator_password,zeng_online_joined_to_room_number);


		int escritos=z_sock_write_string(indice_socket,buffer_envio_comando);

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online list-rooms: %s",z_sock_get_error(escritos));
			return 0;
		}

		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_join_list_buffer,1023,&posicion_command);
		if (leidos>0) {
			zeng_remote_join_list_buffer[leidos]=0; //fin de texto
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online list-rooms (length %d): \n[\n%s\n]",leidos,zeng_remote_join_list_buffer);
		}

		if (leidos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online list-rooms: %s %s",zeng_remote_join_list_buffer,z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			zeng_remote_join_list_buffer[posicion_command-1]=0;
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received zeng-online list-rooms");
            //printf("ZENG: Received zeng-online list-rooms: %s\n",zeng_remote_join_list_buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online list-rooms");
			return 0;
		}

        //printf("join llist: %s\n",zeng_remote_join_list_buffer);

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
int zoc_get_kicked_user(int indice_socket,char *buffer_destino)
{

    char buffer_enviar[1024];

    //get-message-id user_pass n
    sprintf(buffer_enviar,"zeng-online get-kicked-user %s %d\n",
        created_room_user_password,zeng_online_joined_to_room_number);

    #define ZENG_BUFFER_INITIAL_CONNECT 199

    //buffer retorno
    char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

    int return_value=zoc_common_send_command_buffer(indice_socket,buffer_enviar,"get-kicked-user",buffer,ZENG_BUFFER_INITIAL_CONNECT);

    //printf("get_message_id: [%s]\n",buffer);

    if (!return_value) {
        return -1;
    }

    strcpy(buffer_destino,buffer);

    return 0;

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

    //printf("get_message: [%s]\n",buffer);

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

char param_zeng_online_client_kick_user_uuid[STATS_UUID_MAX_LENGTH+1];

//Devuelve 0 si no conectado
int zeng_online_client_kick_user_connect(void)
{

    char buffer_enviar[1024];

    //kick creator_pass n uuid                        Kick user identified by uuid\n"
    sprintf(buffer_enviar,"zeng-online kick %s %d \"%s\"\n",
        created_room_creator_password,
        zeng_online_joined_to_room_number,
        param_zeng_online_client_kick_user_uuid);

    return zoc_open_command_close(buffer_enviar,"kick");

}


int param_zeng_online_client_max_players_room;

//Devuelve 0 si no conectado
int zeng_online_client_max_players_room_connect(void)
{

    char buffer_enviar[1024];

    //set-max-players creator_pass n m   Define max-players (m) for room (n). Requires creator_pass of that room\n"
    sprintf(buffer_enviar,"zeng-online set-max-players %s %d %d\n",
        created_room_creator_password,
        zeng_online_joined_to_room_number,
        param_zeng_online_client_max_players_room);

    return zoc_open_command_close(buffer_enviar,"set-max-players");

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

void *zeng_online_client_kick_user_function(void *nada GCC_UNUSED)
{
    zeng_online_client_kick_user_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_kick_user_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_kick_user_thread_running=0;
		return 0;
	}


	zeng_online_client_kick_user_thread_running=0;

	return 0;

}

void *zeng_online_client_max_players_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_max_players_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_max_players_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_max_players_room_thread_running=0;
		return 0;
	}


	zeng_online_client_max_players_room_thread_running=0;

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

//Devuelve 0 si no conectado
int zeng_online_client_get_profile_keys_connect(void)
{

    char buffer_enviar[1024];





    //return zoc_open_command_close(buffer_enviar,"set-allow-messages");


    int indice_socket=zoc_common_open();
    if (indice_socket<0) {
        return 0;
    }

    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online: Sending %s",command_name_for_info);


    //return zoc_common_send_command_and_close(indice_socket,buffer_enviar,command_name_for_info);


    //int return_value=zoc_common_send_command(indice_socket,buffer_enviar,command_name_for_info);

    #define ZENG_BUFFER_INITIAL_CONNECT 199

    //buffer retorno
    char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

    //Obtener los perfiles asignados a cada uuid y las teclas
    int i;

    for (i=0;i<ZOC_MAX_KEYS_PROFILES;i++) {

        //get-key-profile creator_pass n p
        sprintf(buffer_enviar,"zeng-online get-key-profile %s %d %d\n",
            created_room_creator_password,
            zeng_online_joined_to_room_number,
            i
        );


        int return_value=zoc_common_send_command_buffer(indice_socket,buffer_enviar,"get-key-profile",buffer,ZENG_BUFFER_INITIAL_CONNECT);


        if (!return_value) return 0;

        //printf("buffer recibido para get-key-profile perfil %d: %s\n",i,buffer);

        //Parsear cada valor
        int indice=0;
        int inicio_numero=0;
        int indice_tecla=0;


        while (buffer[indice]) {
            if (buffer[indice]==' ' || buffer[indice+1]==0) {
                if (buffer[indice]==' ') buffer[indice]=0;
                //printf("add tecla %s\n",&buffer[inicio_numero]);
                int valor=parse_string_to_number(&buffer[inicio_numero]);
                allowed_keys[i][indice_tecla++]=valor;
                inicio_numero=indice+1;

            }


            indice++;
        }

        //0 del final si conviene
        if (indice_tecla<ZOC_MAX_KEYS_ITEMS) allowed_keys[i][indice_tecla]=0;

        //mostrarlos
        int j;
        for (j=0;j<ZOC_MAX_KEYS_ITEMS && allowed_keys[i][j];j++) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"Allowed key on profile: %d",allowed_keys[i][j]);
        }


        //get-key-profile-assign creator_pass n p
        sprintf(buffer_enviar,"zeng-online get-key-profile-assign %s %d %d\n",
            created_room_creator_password,
            zeng_online_joined_to_room_number,
            i
        );


        return_value=zoc_common_send_command_buffer(indice_socket,buffer_enviar,"get-key-profile-assign",buffer,ZENG_BUFFER_INITIAL_CONNECT);


        if (!return_value) return 0;

        //printf("buffer recibido para get-key-profile-assign perfil %d: %s\n",i,buffer);

        //Esto tal cual a variable
        strcpy(allowed_keys_assigned[i],buffer);


    }


    //finalizar conexion
    z_sock_close_connection(indice_socket);

    return 1;

}


//Devuelve 0 si no conectado
int zeng_online_client_send_profile_keys_connect(void)
{

    char buffer_enviar[1024];





    //return zoc_open_command_close(buffer_enviar,"set-allow-messages");


    int indice_socket=zoc_common_open();
    if (indice_socket<0) {
        return 0;
    }

    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online: Sending %s",command_name_for_info);


    //return zoc_common_send_command_and_close(indice_socket,buffer_enviar,command_name_for_info);


    //int return_value=zoc_common_send_command(indice_socket,buffer_enviar,command_name_for_info);

    #define ZENG_BUFFER_INITIAL_CONNECT 199

    //buffer retorno
    //char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

    //Obtener los perfiles asignados a cada uuid y las teclas
    int i;

    for (i=0;i<ZOC_MAX_KEYS_PROFILES;i++) {
        char buffer_teclas[ZOC_MAX_KEYS_ITEMS*4]; //3 bytes mas espacio para cada tecla
        buffer_teclas[0]=0;
        int j;
        int total_teclas=0;
        for (j=0;j<ZOC_MAX_KEYS_ITEMS && allowed_keys[i][j];j++) {
            //printf("Tecla escaneada %d\n",allowed_keys[i][j]);
            char buf_tecla[10];
            sprintf(buf_tecla,"%d ",allowed_keys[i][j]);
            util_concat_string(buffer_teclas,buf_tecla,ZOC_MAX_KEYS_ITEMS*4);
            total_teclas++;
        }




        if (total_teclas) {
            //quitar espacio del final
            int longitud=strlen(buffer_teclas);
            if (longitud>0) {
                if (buffer_teclas[longitud-1]==' ') buffer_teclas[longitud-1]=0;
            }

            sprintf(buffer_enviar,"zeng-online set-key-profile %s %d %d %s\n",
                created_room_creator_password,
                zeng_online_joined_to_room_number,
                i,
                buffer_teclas
            );
            //printf("Enviando %s\n",buffer_enviar);
        }

        //No hay teclas. enviar lista vacia
        else {
            sprintf(buffer_enviar,"zeng-online set-key-profile %s %d %d\n",
                created_room_creator_password,
                zeng_online_joined_to_room_number,
                i
            );
            //printf("Enviando %s\n",buffer_enviar);
        }

        int return_value=zoc_common_send_command(indice_socket,buffer_enviar,"set-key-profile");


        if (!return_value) return 0;




        //get-key-profile-assign creator_pass n p
        sprintf(buffer_enviar,"zeng-online set-key-profile-assign %s %d %d %s\n",
            created_room_creator_password,
            zeng_online_joined_to_room_number,
            i,
            allowed_keys_assigned[i]
        );


        return_value=zoc_common_send_command(indice_socket,buffer_enviar,"set-key-profile-assign");


        if (!return_value) return 0;



    }


    //finalizar conexion
    z_sock_close_connection(indice_socket);

    return 1;

}

void *zeng_online_client_get_profile_keys_function(void *nada GCC_UNUSED)
{
    zeng_online_client_get_profile_keys_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_get_profile_keys_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_get_profile_keys_thread_running=0;
		return 0;
	}


	zeng_online_client_get_profile_keys_thread_running=0;

	return 0;

}

void *zeng_online_client_send_profile_keys_function(void *nada GCC_UNUSED)
{
    zeng_online_client_send_profile_keys_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_send_profile_keys_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_send_profile_keys_thread_running=0;
		return 0;
	}


	zeng_online_client_send_profile_keys_thread_running=0;

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


void *zeng_online_client_list_users_function(void *nada GCC_UNUSED)
{

	zeng_online_client_list_users_thread_running=1;



	//Conectar a remoto

	if (!zeng_online_client_list_users_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_list_users_thread_running=0;
		return 0;
	}






	//zeng_enabled.v=1;


	zeng_online_client_list_users_thread_running=0;

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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online list rooms pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_list_rooms);


}

void zeng_online_client_list_users(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_list_users, NULL, &zeng_online_client_list_users_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online list users pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_list_users);


}

void zeng_online_client_leave_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_leave_room, NULL, &zeng_online_client_leave_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online leave room pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online autojoin pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_autojoin_room);


}

void zeng_online_client_disable_autojoin_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_disable_autojoin_room, NULL, &zeng_online_client_disable_autojoin_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online disable autojoin pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online write message pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_write_message_room);


}



void zeng_online_client_kick_user(char *uuid)
{

	//Inicializar thread
    strcpy(param_zeng_online_client_kick_user_uuid,uuid);

	if (pthread_create( &thread_zeng_online_client_kick_user, NULL, &zeng_online_client_kick_user_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online kick user pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_kick_user);


}



void zeng_online_client_max_players_room(int valor)
{

	//Inicializar thread
    param_zeng_online_client_max_players_room=valor;


	if (pthread_create( &thread_zeng_online_client_max_players_room, NULL, &zeng_online_client_max_players_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online set max players room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_max_players_room);


}


//Parametro allow_disallow : 1: allow, 0: disallow
void zeng_online_client_allow_message_room(int allow_disallow)
{

    param_zeng_online_client_allow_message_room_allow_disallow=allow_disallow;



	if (pthread_create( &thread_zeng_online_client_allow_message_room, NULL, &zeng_online_client_allow_message_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online allow/disallow message pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_allow_message_room);


}

void zeng_online_client_get_profile_keys(void)
{


	if (pthread_create( &thread_zeng_online_client_get_profile_keys, NULL, &zeng_online_client_get_profile_keys_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online get profile keys pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_get_profile_keys);


}

void zeng_online_client_send_profile_keys(void)
{

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"Sending restricted keys to server");

	if (pthread_create( &thread_zeng_online_client_send_profile_keys, NULL, &zeng_online_client_send_profile_keys_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online send profile keys pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_send_profile_keys);


}

void zeng_online_client_destroy_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_destroy_room, NULL, &zeng_online_client_destroy_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online destroy room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_destroy_room);


}

void zeng_online_client_join_list(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_join_list, NULL, &zeng_online_client_join_list_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online join list pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online authorize join pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_authorize_join);


}



//Devuelve 0 si no conectado
int zeng_online_client_join_room_connect(void)
{




        //zeng_remote_list_rooms_buffer[0]=0;

        char server[NETWORK_MAX_URL+1];
        int puerto;
        puerto=zeng_online_get_server_and_port(server);

		int indice_socket=z_sock_open_connection(server,puerto,0,"");

		if (indice_socket<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
			return 0;
		}

		//zsock_wait_until_command_prompt(indice_socket);

		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Sending join-room");

        char buffer_enviar[1024];

        //Join desde el master le envia el creator password
        if (param_join_room_creator_password[0]!=0) {
            sprintf(buffer_enviar,"zeng-online join %d \"%s\" %s %s\n",param_join_room_number,zeng_online_nickname,stats_uuid,param_join_room_creator_password);
        }
        else {
            sprintf(buffer_enviar,"zeng-online join %d \"%s\" %s\n",param_join_room_number,zeng_online_nickname,stats_uuid);
        }

		int escritos=z_sock_write_string(indice_socket,buffer_enviar);

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online join: %s",z_sock_get_error(escritos));
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
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online join (length %d): \n[\n%s\n]",leidos,buffer);
            }

            if (leidos<0) {
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online join: %s",z_sock_get_error(leidos));
                return 0;
            }

            if (leidos==0) {
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"No response read from join yet");
            }

            reintentos--;
        } while (leidos==0 && reintentos>0);

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			//DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text: %s",buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online join");
			return 0;
		}

        //printf("Retorno join-room: [%s]\n",buffer);
        //Si hay ERROR
        if (strstr(buffer,"ERROR")!=NULL) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error joining room: %s",buffer);
            return 0;
        }

        //Dividir el user_password y los permisos
        int i;
        for (i=0;buffer[i] && buffer[i]!=' ';i++);

        if (buffer[i]==0) {
            //llegado al final sin que haya espacios. error
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error joining room, no permissions detected: %s",buffer);
            return 0;
        }

        //truncar de momento hasta aqui
        buffer[i]=0;
        strcpy(created_room_user_password,buffer);

        //Y seguir hasta buscar los permisos
        i++;
        created_room_user_permissions=parse_string_to_number(&buffer[i]);

        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"User password: [%s]",created_room_user_password);
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"User permissions: [%d]",created_room_user_permissions);


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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online join room pthread");
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
			return 0;
		}

		//zsock_wait_until_command_prompt(indice_socket);

		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Sending create-room");

        char buffer_enviar[1024];
        sprintf(buffer_enviar,"zeng-online create-room %d \"%s\"\n",param_create_room_number,param_create_room_name);

		int escritos=z_sock_write_string(indice_socket,buffer_enviar);

		if (escritos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't send zeng-online create-room: %s",z_sock_get_error(escritos));
			return 0;
		}


		//reintentar
		leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Received text for zeng-online create-room (length %d): \n[\n%s\n]",leidos,buffer);
		}

		if (leidos<0) {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive zeng-online create-room: %s",z_sock_get_error(leidos));
			return 0;
		}

		//1 mas para eliminar el salto de linea anterior a "command>"
		if (posicion_command>=1) {
			buffer[posicion_command-1]=0;
			//DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text: %s",buffer);
		}
		else {
			DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online create-room");
			return 0;
		}

        //printf("Retorno crear-room: [%s]\n",buffer);
        //Si hay ERROR
        if (strstr(buffer,"ERROR")!=NULL) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error creating room: %s",buffer);
            return 0;
        }
        strcpy(created_room_creator_password,buffer);

        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Creator password: [%s]",created_room_creator_password);

		//finalizar conexion
        z_sock_close_connection(indice_socket);

    zeng_online_room_was_created.v=1;

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

    zeng_online_room_was_created.v=0;

	//Inicializar thread
    //Parametros, dado que tienen que ser estaticos, ni me preocupo en crear una estructura, se asignan a dos variables
    //estaticas y listo
    param_create_room_number=room_number;
    strcpy(param_create_room_name,room_name);

	if (pthread_create( &thread_zeng_online_client_create_room, NULL, &zeng_online_client_create_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online create room pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Sending snapshot");

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
                    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
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
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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

                    if (!valido) {
                        //printf("linea no valida\n");
                    }
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
                                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Processing from ZRCP command send-keys-event: key %d event %d",numero_key,numero_event);
                                //printf ("Processing from ZRCP command send-keys-event: key %d event %d\n",numero_key,numero_event);

                                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
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
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Received answer for get_keys with invalid number of fields");
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
    //printf("leidos on zoc_start_connection_get_keys: %d\n",leidos);
    if (leidos>0) {
        buffer_initial[leidos]=0; //fin de texto
        //printf("Received text on zoc_start_connection_get_keys (length: %d):\n[\n%s\n]\n",leidos,buffer_initial);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //bucle continuo de leer teclas de remoto
    //TODO: ver posible manera de salir de aqui??
    //int error_desconectar=0;



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



        //printf ("before read command sending get-keys on zoc_start_connection_get_keys\n");
        //1 segundo maximo de reintentos
        //sobretodo util en la primera conexion, dado que el buffer vendra vacio, siempre esperara 1 segundo
        int leidos=zsock_read_all_until_command_max_reintentos(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command,100);
        //printf ("after read command sending get-keys on zoc_start_connection_get_keys. leidos=%d\n",leidos);

        if (leidos>0) {
            buffer_initial[leidos]=0; //fin de texto
            //printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer_initial);
        }

        if (leidos<0) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d",elemento.tecla,elemento.pressrelease);

        //printf ("ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d\n",elemento.tecla,elemento.pressrelease);

        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
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
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text for get-join-queue-size (length %d)",leidos);
                    //printf("ZENG: Received text for get-join-queue-size (length %d): \n[\n%s\n]\n",leidos,buffer);
                }

                if (leidos<0) {
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive get-join-queue-size: %s",z_sock_get_error(leidos));
                    return 0;
                }


                //printf("after zsock_read_all_until_command\n");
                //printf("Recibido respuesta despues de get-snapshot-id: [%s]\n",buffer);

                //1 mas para eliminar el salto de linea anterior a "command>"
                if (posicion_command>=1) {
                    buffer[posicion_command-1]=0;
                    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                }
                else {
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX get-join-queue-size");
                    return 0;
                }

                //printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

                //int leer_snap=0;

                //Detectar si error
                if (strstr(buffer,"ERROR")!=NULL) {
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Error getting get-join-queue-size");
                }
                else {
                    //Ver si id diferente
                    int queue_size=parse_string_to_number(buffer);
                    //printf("Waiting room queue size: %d\n",queue_size);

                    if (queue_size) {

                        if (queue_size!=zoc_get_pending_authorization_size_last_queue_size) {
                            //Escribir en footer solo cuando valor anterior cambia
                            //TODO quiza habria que usar algun tipo de bloqueo para esto


                            zoc_get_pending_authorization_size_last_queue_size=queue_size-1;

                            //Y lo mostramos en el footer
                            char mensaje[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH+ZOC_MAX_NICKNAME_LENGTH+1];

                            sprintf(mensaje,"ZOC clients waiting on room");

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
//int contador_obtener_mensajes=0;
int zoc_last_message_id=0;

//En el rejoin como master se aplicara el primer snapshot recibido del server
int zoc_pending_apply_received_snapshot_as_rejoin_as_master=0;

//contiene el valor anterior de contador_segundo_infinito de la anterior consulta de mensajes
int contador_mensajes_anteriorsegundos=0;


int contador_alive_anteriorsegundos=0;
//unsigned int antes_network_traffic_counter_read=0;
//unsigned int antes_network_traffic_counter_write=0;

void zoc_common_alive_user(int indice_socket)
{

    //Cada 10 segundos enviar comando alive
    int diferencia_tiempo=contador_segundo_infinito-contador_alive_anteriorsegundos;

    if (diferencia_tiempo>50*20*10) {
        contador_alive_anteriorsegundos=contador_segundo_infinito;

        char buffer_enviar[1024];
            //alive user_pass n uuid
            sprintf(buffer_enviar,"zeng-online alive %s %d %s\n",
                created_room_user_password,
                zeng_online_joined_to_room_number,
                stats_uuid
            );
            //printf("Enviando %s\n",buffer_enviar);


        int return_value=zoc_common_send_command(indice_socket,buffer_enviar,"alive");


        //if (!return_value) return 0;

    }
}

void zoc_common_get_messages_slave_master(int indice_socket)
{

    //Cada segundo ver si hay pendientes mensajes
    int diferencia_tiempo=contador_segundo_infinito-contador_mensajes_anteriorsegundos;

    if (diferencia_tiempo>50*20) {
        contador_mensajes_anteriorsegundos=contador_segundo_infinito;

        //printf("contador_segundo_infinito: %d\n",contador_segundo_infinito);

        //prueba mostrar trafico
        /*unsigned int diferencia_read=network_traffic_counter_read-antes_network_traffic_counter_read;
        unsigned int diferencia_write=network_traffic_counter_write-antes_network_traffic_counter_write;
        //ha pasado 1 segundo tal cual, por tanto no hay que dividir
        printf("Traffic: %8u kbyte/s read - %8u kbyte/s write\n",diferencia_read/1000,diferencia_write/1000);

        antes_network_traffic_counter_read=network_traffic_counter_read;
        antes_network_traffic_counter_write=network_traffic_counter_write;*/
        //fin prueba mostrar trafico

        int id_actual=zoc_get_message_id(indice_socket);
        if (id_actual!=zoc_last_message_id) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"There is a new message");
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }


    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    zoc_last_message_id=zoc_get_message_id(indice_socket);
    //printf("Initial message id: %d\n",zoc_last_message_id);

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
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Error sending snapshot to zeng online server");
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

                //printf("Recibiendo snapshot porque somos master manager. contador_segundo=%d\n",contador_segundo);
                int error=zoc_receive_snapshot(indice_socket);
                //TODO gestionar bien este error
                if (error<0) {
                    //TODO
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Error getting snapshot from zeng online server");
                    usleep(10000); //dormir 10 ms
                }

            }

            //Si estabamos reentrando como master, obtener el snapshot que habia
            if (zoc_rejoining_as_master) {
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"Receiving first snapshot as we are rejoining as master");
                zoc_rejoining_as_master=0;
                zoc_pending_apply_received_snapshot_as_rejoin_as_master=1;

                zoc_receive_snapshot(indice_socket);
            }

            //Enviar teclas
            //TODO gestionar error_desconectar
            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS) {
                int enviada_alguna_tecla;
                //int error_desconectar=
                zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);
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

            zoc_common_alive_user(indice_socket);

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
z80_byte *zoc_get_snapshot_mem_binary_comprimido=NULL;

int zoc_receive_snapshot_last_id=0;

int zoc_ultimo_snapshot_recibido_es_zip=0;

int zeng_online_snapshot_diff=0;

int zeng_online_snapshot_diff_media=0;

int zoc_receive_snapshot(int indice_socket)
{
    //printf("Inicio zoc_receive_snapshot llamado desde:\n");
    //debug_exec_show_backtrace();

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Receiving snapshot");

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
                    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text for get-snapshot-id (length %d): \n[\n%s\n]",leidos,buffer);
                }

                if (leidos<0) {
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive get-snapshot-id: %s",z_sock_get_error(leidos));
                    return 0;
                }


                //printf("after zsock_read_all_until_command\n");
                //printf("Recibido respuesta despues de get-snapshot-id: [%s]\n",buffer);

                //1 mas para eliminar el salto de linea anterior a "command>"
                if (posicion_command>=1) {
                    buffer[posicion_command-1]=0;
                    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                }
                else {
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX get-snapshot-id");
                    return 0;
                }

                //printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

                int leer_snap=0;

                //Detectar si error
                if (strstr(buffer,"ERROR")!=NULL) {
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Error getting snapshot-id");
                }
                else {
                    //Ver si id diferente
                    //printf("snapshot id: %s\n",buffer);

                    int nuevo_id=parse_string_to_number(buffer);
                    if (nuevo_id!=zoc_receive_snapshot_last_id) {
                        zeng_online_snapshot_diff=nuevo_id-zoc_receive_snapshot_last_id;
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"Snapshot id difference from last snapshot: %d",zeng_online_snapshot_diff);

                        zeng_online_snapshot_diff_media=(zeng_online_snapshot_diff+zeng_online_snapshot_diff_media)/2;

                        int alertar_diferencia=10;

                        //si no es zip lo habitual es que haya mayor diferencia
                        //aunque bueno... 10 snapshots de retraso es mucho,
                        //aunque no sea zip, pero para no alertar continuamente cuando no es zip,
                        //porque es lo habitual, que sea mas de 10 para no zip y conexion contra servidor remoto
                        //Lo normal es que el master esté enviando siempre con compresión zip, a no ser que alguien
                        //lo quiera desactivar por alguna razón que no se me ocurre ahora...
                        if (!zoc_ultimo_snapshot_recibido_es_zip) alertar_diferencia=20;

                        //Si han pasado mas de 10 snapshots, avisar con "algo" el el footer
                        if (zeng_online_snapshot_diff>alertar_diferencia) {
                            if (zeng_online_show_footer_lag_indicator.v) {
                                generic_footertext_print_operating("LAG");
                            }
                        }

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
                        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text for get-snapshot (length %d): \n[\n%s\n]",leidos,zoc_get_snapshot_mem_hexa);
                    }

                    if (leidos<0) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't receive get-snapshot: %s",z_sock_get_error(leidos));
                        return 0;
                    }


                    //printf("after zsock_read_all_until_command\n");
                   // printf("Recibido respuesta despues de get-snapshot: [%s]\n",zoc_get_snapshot_mem_hexa);

                    //1 mas para eliminar el salto de linea anterior a "command>"
                    if (posicion_command>=1) {
                        zoc_get_snapshot_mem_hexa[posicion_command-1]=0;
                        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG: Received text: %s",zoc_get_snapshot_mem_hexa);
                    }
                    else {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error receiving ZEsarUX zeng-online get-snapshot");
                        return 0;
                    }

                    //printf("Recibido respuesta despues de truncar: [%s]\n",zoc_get_snapshot_mem_hexa);

                    //Nos quedamos con la respuesta hasta el ultimo
                    //Es decir, si en get-snapshot el server remoto nos ha dejado en cola 2, 3 o mas snapshots, estamos pillando el ultimo
                    //Recordemos que lo envia de manera continua con saltos de linea despues de cada uno de ellos
                    //TODO: esto esta MAL. Hay que escoger el que tiene el ultimo salto de linea
                    //int i;

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
                    if (zoc_get_snapshot_mem_binary_comprimido==NULL) {
                        zoc_get_snapshot_mem_binary_comprimido=util_malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2,"Can not allocate memory for apply snapshot");
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

                        zoc_get_snapshot_mem_binary_comprimido[parametros_recibidos++]=valor;
                        //menu_debug_write_mapped_byte(direccion++,valor);

                        s++;
                        if (*s) s++;
                    }




                    int zoc_get_snapshot_mem_binary_longitud_comprimido=parametros_recibidos;

                    //"PK" = 50 4b
                    //printf("Firma ZIP: %02XH %02XH\n",zoc_get_snapshot_mem_binary_comprimido[0],zoc_get_snapshot_mem_binary_comprimido[1]);

                    zoc_ultimo_snapshot_recibido_es_zip=0;

                    if (zoc_get_snapshot_mem_binary_longitud_comprimido>1) {
                        if (zoc_get_snapshot_mem_binary_comprimido[0]==0x50 &&
                            zoc_get_snapshot_mem_binary_comprimido[1]==0x4b) {
                            zoc_ultimo_snapshot_recibido_es_zip=1;
                        }
                    }


                    if (zoc_ultimo_snapshot_recibido_es_zip) {
                        //Descomprimir zip
                        //printf("Es snapshot comprimido\n");
                        //calcular el tiempo en descomprimirlo


                        //calcular tiempo que tarda en descomprimir

                        timer_stats_current_time(&zeng_online_uncompress_time_antes);


                        zoc_get_snapshot_mem_binary=util_uncompress_memory_zip(zoc_get_snapshot_mem_binary_comprimido,
                            zoc_get_snapshot_mem_binary_longitud_comprimido,&zoc_get_snapshot_mem_binary_longitud,"snapshot.zsf");

                        zeng_online_uncompress_difftime=timer_stats_diference_time(&zeng_online_uncompress_time_antes,&zeng_online_uncompress_time_despues);

                        //media de tiempo.
                        zeng_online_uncompress_media=(zeng_online_uncompress_media+zeng_online_uncompress_difftime)/2;

                        //printf("Tiempo en descomprimir: %ld us\n",zeng_online_uncompress_difftime);

                        //solo liberamos en este caso que hay el espacio comprimido
                        free(zoc_get_snapshot_mem_binary_comprimido);
                    }

                    else {
                        //printf("No es snapshot comprimido\n");
                        //aqui no haremos free dado que no existe compresion y entonces
                        //zoc_get_snapshot_mem_binary y zoc_get_snapshot_mem_binary_comprimido apuntan al mismo sitio
                        //ya se liberara desde zoc_get_snapshot_mem_binary_comprimido al aplicar el snapshot
                        zoc_get_snapshot_mem_binary=zoc_get_snapshot_mem_binary_comprimido;
                        zoc_get_snapshot_mem_binary_longitud=zoc_get_snapshot_mem_binary_longitud_comprimido;

                    }


                    zoc_get_snapshot_mem_binary_comprimido=NULL;

                    //printf("Apply snapshot. Compressed %d Uncompressed %d\n",zoc_get_snapshot_mem_binary_longitud_comprimido,zoc_get_snapshot_mem_binary_longitud);

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

//contiene el valor anterior de contador_segundo_infinito de la anterior consulta de kick
int contador_kick_anteriorsegundos=0;


//comprueba si nos han echado de la habitacion
int zoc_check_if_kicked(int indice_socket)
{
    //Ver si nos han hecho kick


    //Cada segundo ver
    int diferencia_tiempo=contador_segundo_infinito-contador_kick_anteriorsegundos;

    if (diferencia_tiempo>50*20) {
        contador_kick_anteriorsegundos=contador_segundo_infinito;

        char buffer_kick_user[STATS_UUID_MAX_LENGTH+1];
        int retorno=zoc_get_kicked_user(indice_socket,buffer_kick_user);
        if (retorno>=0) {
            //printf("Kicked user: [%s]\n",buffer_kick_user);
            if (!strcmp(buffer_kick_user,stats_uuid)) {
                //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"I have been kicked");
                put_footer_first_message("I have been kicked");
                zeng_online_client_leave_room();
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"I have been kicked from the ZENG Online room");
                return 1;
            }
        }

    }

    return 0;
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    zoc_last_message_id=zoc_get_message_id(indice_socket);
    //printf("Initial message id: %d\n",zoc_last_message_id);


    int indice_socket_get_keys=zoc_start_connection_get_keys();

    //bucle continuo de recepcion snapshot
    //TODO: ver posible manera de salir de aqui??

    //int temppppp=0;

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
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Error getting snapshot from zeng online server");
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
                //int error_desconectar=
                zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);


            }

            zoc_common_get_messages_slave_master(indice_socket);

            zoc_common_alive_user(indice_socket);


            if (zoc_check_if_kicked(indice_socket)) {
                //Logicamente el usuario puede intentar entrar de nuevo pero
                //mientras no se haga kick de otro usuario diferente, este primero quedara en la lista de kick
                //y siempre se le echara
                //Ademas como va relacionado por el uuid, aunque cambie su nick, el kick seguirá siendo efectivo
                return NULL;
            }



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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
        //printf("Permitir primer snapshot recibido con rejoin\n");
        zoc_pending_apply_received_snapshot_as_rejoin_as_master=0;
    }

    //O si somos un master con permisos de get snapshot
    //o sea lo que era inicialmente un manager master, que recibia snapshots
    //Esto ya no se usa, un manager no recibe snapshots
    else if (zeng_online_i_am_master.v && (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT)) {
        //printf("Permitir aplicar snapshot porque somos solo manager\n");
    }

    else {
        if (zeng_online_i_am_master.v) return;
    }


    //printf("Putting snapshot coming from ZRCP: %p %d\n",zoc_get_snapshot_mem_binary,zoc_get_snapshot_mem_binary_longitud);

    load_zsf_snapshot_file_mem(NULL,zoc_get_snapshot_mem_binary,zoc_get_snapshot_mem_binary_longitud,1,1);

    free(zoc_get_snapshot_mem_binary);
    zoc_get_snapshot_mem_binary=NULL;

/*
#ifdef ZENG_ONLINE_USE_ZIP_SNAPSHOT
    free(zoc_get_snapshot_mem_binary_comprimido);
#endif
    zoc_get_snapshot_mem_binary_comprimido=NULL;
*/

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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online send snapshot pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_master_thread);
}

//Inicio del thread de slave
void zoc_start_slave_thread(void)
{
	if (pthread_create( &thread_zoc_slave_thread, NULL, &zoc_slave_thread_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"Can not create zeng online send snapshot pthread");
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
					DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG: Last snapshot has not been sent yet. Total unsent: %d",zoc_snapshots_not_sent);

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
                            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG: Forcing reconnect");
                            //printf("Before forcing ZENG reconnect\n");
                            zeng_force_reconnect();
                            //printf("After forcing ZENG reconnect\n");
                        }
                    }
                    */
				}
				else {
                    zoc_snapshots_not_sent=0;

					//zona de memoria donde se guarda el snapshot pero sin pasar a hexa. y sin comprimir zip
					z80_byte *buffer_temp_sin_comprimir;
					buffer_temp_sin_comprimir=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM); //16 MB es mas que suficiente

					if (buffer_temp_sin_comprimir==NULL) cpu_panic("Can not allocate memory for sending snapshot");

					int longitud_sin_comprimir;

                    //La compresion nativa del zsf (repeticiones de bytes) la forzamos siempre activa
                    int antes_zsf_force_uncompressed=zsf_force_uncompressed;
                    zsf_force_uncompressed=0;

  					save_zsf_snapshot_file_mem(NULL,buffer_temp_sin_comprimir,&longitud_sin_comprimir,1);

                    zsf_force_uncompressed=antes_zsf_force_uncompressed;

                    int longitud;
                    z80_byte *buffer_temp;

                    //Obtener copia de la variable por si se modifica el setting (cosa muy improbable) antes de la segunda vez de usar el setting
                    int comprimir_zip=zeng_online_zip_compress_snapshots.v;

                    if (comprimir_zip) {

                        timer_stats_current_time(&zeng_online_compress_time_antes);

                        //Comprimir a zip
                        buffer_temp=util_compress_memory_zip(buffer_temp_sin_comprimir,longitud_sin_comprimir,
                                        &longitud,"snapshot.zsf");

                        zeng_online_compress_difftime=timer_stats_diference_time(&zeng_online_compress_time_antes,&zeng_online_compress_time_despues);

                        //media de tiempo.
                        zeng_online_compress_media=(zeng_online_compress_media+zeng_online_compress_difftime)/2;

                    }

                    else {
                        buffer_temp=buffer_temp_sin_comprimir;
                        longitud=longitud_sin_comprimir;
                    }



                    //temp_memoria_asignada++;
                    //printf("Asignada: %d liberada: %d\n",temp_memoria_asignada,temp_memoria_liberada);
                    //printf("Created snapshot original size %d compressed size %d\n",longitud_sin_comprimir,longitud);

					if (zoc_send_snapshot_mem_hexa==NULL) zoc_send_snapshot_mem_hexa=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2); //16 MB es mas que suficiente

					//int char_destino=0;

					int i;

                    z80_byte *origen=buffer_temp;
                    char *destino=zoc_send_snapshot_mem_hexa;
                    z80_byte byte_leido;

					for (i=0;i<longitud;i++/*,char_destino +=2*/) {
                        //En vez de sprintf, que es poco optimo, usar alternativa
                        //Comparativa: con un snapshot con un di y un jp 16384, tarda
                        //con sprintf: 116 microsegundos el tiempo mas bajo
                        //con util_byte_to_hex_nibble: 5 microsegundos el tiempo mas bajo
						//sprintf (&zoc_send_snapshot_mem_hexa[char_destino],"%02X",buffer_temp[i]);
                        byte_leido=*origen++;
                        *destino++=util_byte_to_hex_nibble(byte_leido>>4);
                        *destino++=util_byte_to_hex_nibble(byte_leido);
					}



					//metemos salto de linea y 0 al final
					//strcpy (&zoc_send_snapshot_mem_hexa[char_destino],"\n");
                    *destino++ ='\n';
                    *destino++ =0;



                    //printf ("ZENG Online: Queuing snapshot to send, length: %d\n",longitud);


					//Liberar memoria que ya no se usa
                    if (comprimir_zip) {
					    free(buffer_temp);
                    }
                    free(buffer_temp_sin_comprimir);


					zoc_pending_send_snapshot=1;

                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online: Queuing snapshot to send, length: %d",longitud);


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
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"Timeout receiving snapshots from master. Allowing local key press");
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

void zeng_online_client_kick_user(char *message)
{
}

void zeng_online_client_max_players_room(int valor)
{
}

void zeng_online_client_allow_message_room(int allow_disallow)
{
}

void zeng_online_client_join_room(int room_number,char *creator_password)
{
}

void zeng_online_client_list_users(void)
{
}

void zeng_online_client_get_profile_keys(void)
{
}

void zeng_online_client_send_profile_keys(void)
{
}

#endif