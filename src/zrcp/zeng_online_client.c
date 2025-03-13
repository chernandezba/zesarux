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
#include "mem128.h"
#include "ula.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng_online_client_list_rooms;
pthread_t thread_zeng_online_client_create_room;
pthread_t thread_zeng_online_client_join_room;
pthread_t thread_zoc_master_thread;
pthread_t thread_zoc_master_thread_secondary_commands;
pthread_t thread_zoc_master_thread_stream_audio;
pthread_t thread_zoc_slave_thread;
pthread_t thread_zoc_slave_thread_secondary_commands;
pthread_t thread_zoc_slave_thread_stream_audio;
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
pthread_t thread_zeng_online_client_rename_room;
pthread_t thread_zeng_online_client_stop_slave_thread;
pthread_t thread_zeng_online_client_stop_master_thread;


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
int zeng_online_client_rename_room_thread_running=0;
int zeng_online_client_stop_slave_thread_thread_running=0;
int zeng_online_client_stop_master_thread_thread_running=0;

z80_bit zeng_online_i_am_master={0};
//z80_bit zeng_online_i_am_joined={0};
int zeng_online_joined_to_room_number=0;

int zoc_pending_send_snapshot=0;

int zoc_pending_send_streaming_display=0;

//Si esta conectado
z80_bit zeng_online_connected={0};

//Si la ultima habitacion se ha creado.
z80_bit zeng_online_room_was_created={0};

//Servidor zeng online remoto al que conectarse
//Dejamos por defecto el mio para que la gente pueda conectarse a salas ya creadas o crear nuevas
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

//Si tiene modo streaming habilitado en la room que nos hemos unido
int created_room_streaming_mode;

int param_join_room_number;
char param_join_room_creator_password[ZENG_ROOM_PASSWORD_LENGTH+1];

//Se indicara que queremos obtener el snapshot que habia ya que estamos reentrando como master
int zoc_rejoining_as_master=0;

//Crear una habitacion con modo streaming o no
int streaming_enabled_when_creating=1;

//Detectar silencio en sonido en streaming
int streaming_silence_detection=1;

//Perfiles de teclas que se han cargado del servidor como master/manager para la habitacion actual
int allowed_keys[ZOC_MAX_KEYS_PROFILES][ZOC_MAX_KEYS_ITEMS];
//Perfiles asignados a cada uuid. Si es "", no esta asignado
char allowed_keys_assigned[ZOC_MAX_KEYS_PROFILES][STATS_UUID_MAX_LENGTH+1];

char *zeng_remote_list_users_buffer=NULL;


char *zoc_get_snapshot_mem_hexa=NULL;
z80_byte *zoc_get_snapshot_mem_binary=NULL;
int zoc_get_snapshot_mem_binary_longitud=0;
int zoc_pending_apply_received_snapshot=0;
z80_byte *zoc_get_snapshot_mem_binary_comprimido=NULL;

char *zoc_get_streaming_display_mem_hexa=NULL;
z80_byte *zoc_get_streaming_display_mem_binary=NULL;
int zoc_get_streaming_display_mem_binary_longitud=0;
int zoc_pending_apply_received_streaming_display=0;

int zoc_receive_snapshot_last_id=0;

int zoc_ultimo_snapshot_recibido_es_zip=0;

int zeng_online_snapshot_diff=0;

int zeng_online_snapshot_diff_media=0;

int zoc_slave_differential_displays_counter=0;

//Cada cuantas pantallas diferenciales hay que pedir una entera
int zoc_slave_differential_displays_limit_full=2;

//Si autoajustar parametro de zoc_slave_differential_displays_limit_full
z80_bit zoc_slave_differential_displays_limit_full_autoadjust={1};

//FPS del anterior intervalo para autoajuste de diferenciales
int zoc_slave_differential_displays_limit_full_previous_fps=-1;

//Cada cuanto se autoajusta (en segundos)
int zoc_slave_differential_displays_limit_full_autoadjust_seconds_interval=10;
int zoc_slave_differential_displays_limit_full_autoadjust_seconds_counter=0;

//A cada minuto enviar/recibir snapshot en modo streaming
#define ZOC_INTERVAL_SNAPSHOT_ON_STREAMING (50*60)


//Decir a los threads de salir de la habitacion, por ejemplo si hacen kick del usuario
int zoc_exit_from_room=0;


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

//Para dar un margen al principio de conectar antes de salir aviso de dropout
int zoc_veces_audio_no_recibido_contador=-20;


int zoc_veces_audio_no_recibido_segundos=0;

void zec_veces_audio_no_recibido_timer(void)
{
    if (zeng_online_connected.v && zeng_online_i_am_master.v==0 && created_room_streaming_mode) {
        //Contar intervalos cada 10 segundos
        zoc_veces_audio_no_recibido_segundos++;
        if (zoc_veces_audio_no_recibido_segundos>=10) {
            zoc_veces_audio_no_recibido_segundos=0;
            zoc_veces_audio_no_recibido_contador=0;
        }
    }
}


#ifdef USE_PTHREADS

//Funciones que usan pthreads
int zoc_receive_snapshot(int indice_socket);


//Devuelve 0 si no conectado
int zeng_online_client_list_rooms_connect(void)
{



    //Calculo aproximado de memoria necesaria para listado de habitaciones
    //escribir_socket(misocket,"N.  Name                           Created Players Max\n");
    int espacio_por_room=ZENG_ONLINE_MAX_ROOM_NAME+1024; //Margen mas que suficiente
    int total_listado_rooms_memoria=espacio_por_room*ZENG_ONLINE_MAX_ROOMS;


    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Getting zeng-online rooms");
    if (zeng_remote_list_rooms_buffer==NULL) {

        zeng_remote_list_rooms_buffer=util_malloc(total_listado_rooms_memoria,"Can not allocate memory for getting list-rooms");



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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //zsock_wait_until_command_prompt(indice_socket);

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Sending get-version");

    //Enviar un get-version
    int escritos=z_sock_write_string(indice_socket,"get-version\n");

    if (escritos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send get-version: %s",z_sock_get_error(escritos));
        return 0;
    }


    //reintentar
    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text for get-version (length %d): \n[\n%s\n]",leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive version: %s",z_sock_get_error(leidos));
        return 0;
    }

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received version: %s",buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX remote version");
        return 0;
    }

    //Comprobar que version remota sea como local
    char myversion[30];
    util_get_emulator_version_number(myversion);
    if (strcasecmp(myversion,buffer)) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Local and remote ZEsarUX versions do not match");
        //printf("Local %s remote %s\n",myversion,buffer);
        return 0;
    }


    //Comprobar que, si nosotros somos master, el remoto no lo sea
    //El usuario puede activarlo, aunque no es recomendable. Yo solo compruebo y ya que haga lo que quiera


    //ver si zeng online enabled
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: ZENG Online: Getting zeng-online status");

    escritos=z_sock_write_string(indice_socket,"zeng-online is-enabled\n");

    if (escritos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online is-enabled: %s",z_sock_get_error(escritos));
        return 0;
    }

    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online is-enabled (length %d): \n[\n%s\n]",leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online is-enabled: %s",z_sock_get_error(leidos));
        return 0;
    }

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received zeng-online is-enabled: %s",buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online is-enabled");
        return 0;
    }

    //Si somos master, que el remoto no lo sea tambien
    int esta_activado=parse_string_to_number(buffer);
    if (!esta_activado) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ZENG Online is not enabled on remote server");
        return 0;
    }


    //Obtener rooms



    escritos=z_sock_write_string(indice_socket,"zeng-online list-rooms\n");

    if (escritos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online list-rooms: %s",z_sock_get_error(escritos));
        return 0;
    }

    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_list_rooms_buffer,total_listado_rooms_memoria,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online list-rooms (length %d): \n[\n%s\n]",leidos,zeng_remote_list_rooms_buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online list-rooms: %s %s",zeng_remote_list_rooms_buffer,z_sock_get_error(leidos));
        return 0;
    }

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        zeng_remote_list_rooms_buffer[posicion_command-1]=0;
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received zeng-online list-rooms",zeng_remote_list_rooms_buffer);
        //printf("ZENG: Received zeng-online list-rooms: %s\n",zeng_remote_list_rooms_buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online list-rooms");
        return 0;
    }

    //printf("Habitaciones: %s\n",zeng_remote_list_rooms_buffer);

    //finalizar conexion
    z_sock_close_connection(indice_socket);



	//zeng_remote_socket=indice_socket;

	return 1;
}



//Devuelve 0 si no conectado
int zeng_online_client_list_users_connect(void)
{



    //Calculo aproximado de memoria necesaria para listado de usuarios
    int espacio_necesario=ZENG_ONLINE_MAX_PLAYERS_PER_ROOM*(ZOC_MAX_NICKNAME_LENGTH+STATS_UUID_MAX_LENGTH+30); //+30 mas que suficiente margen



    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: ZENG Online: Getting zeng-online users");
    if (zeng_remote_list_users_buffer==NULL) {

        zeng_remote_list_users_buffer=util_malloc(espacio_necesario,"Can not allocate memory for getting list-users");

    }
    zeng_remote_list_users_buffer[0]=0;



    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");




    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    char buffer_comando[200];
    sprintf(buffer_comando,"zeng-online list-users %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);

    int escritos=z_sock_write_string(indice_socket,buffer_comando);

    if (escritos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online list-users: %s",z_sock_get_error(escritos));
        return 0;
    }

    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_list_users_buffer,espacio_necesario,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online list-users (length %d): \n[\n%s\n]",leidos,zeng_remote_list_users_buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online list-users: %s %s",zeng_remote_list_users_buffer,z_sock_get_error(leidos));
        return 0;
    }

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        zeng_remote_list_users_buffer[posicion_command-1]=0;
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received zeng-online list-users: %s",zeng_remote_list_users_buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online list-users");
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
        //Solo mostrar ese mensaje cuando aun no se ha declarado desconectado
        if (zoc_last_snapshot_received_counter) DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online %s: %s",command_name_for_info,z_sock_get_error(escritos));
        return 0;
    }


    int leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,max_buffer,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online %s (length %d): \n[\n%s\n]",command_name_for_info,leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online %s: %s",command_name_for_info,z_sock_get_error(leidos));
        return 0;
    }



    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online %s",command_name_for_info);
        return 0;
    }

    //printf("Retorno %s: [%s]\n",command_name_for_info,buffer);
    //Si hay ERROR
    if (strstr(buffer,"ERROR")!=NULL) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error %s room: %s",command_name_for_info,buffer);
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

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: ZENG Online: Sending %s",command_name_for_info);


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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }



    char buffer_envio_comando[200];
    //authorize-join creator_pass n perm
    sprintf(buffer_envio_comando,"zeng-online authorize-join %s %d %d\n",
        created_room_creator_password,zeng_online_joined_to_room_number,parm_zeng_online_client_authorize_join_permissions);

    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Authorizing by: %s",buffer_envio_comando);
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Authorizing user");

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Permissions: %d",parm_zeng_online_client_authorize_join_permissions);


    int escritos=z_sock_write_string(indice_socket,buffer_envio_comando);

    if (escritos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online authorize-join: %s",z_sock_get_error(escritos));
        return 0;
    }

    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online authorize-join (length %d): \n[\n%s\n]",leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online authorize-join: %s %s",buffer,z_sock_get_error(leidos));
        return 0;
    }

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received zeng-online authorize-join: %s",buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online authorize-join");
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }



    //Obtener join list
    char buffer_envio_comando[200];

    sprintf(buffer_envio_comando,"zeng-online get-join-first-element-queue %s %d\n",created_room_creator_password,zeng_online_joined_to_room_number);


    int escritos=z_sock_write_string(indice_socket,buffer_envio_comando);

    if (escritos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online list-rooms: %s",z_sock_get_error(escritos));
        return 0;
    }

    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zeng_remote_join_list_buffer,1023,&posicion_command);
    if (leidos>0) {
        zeng_remote_join_list_buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online list-rooms (length %d): \n[\n%s\n]",leidos,zeng_remote_join_list_buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online list-rooms: %s %s",zeng_remote_join_list_buffer,z_sock_get_error(leidos));
        return 0;
    }

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        zeng_remote_join_list_buffer[posicion_command-1]=0;
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received zeng-online list-rooms");
        //printf("ZENG: Received zeng-online list-rooms: %s\n",zeng_remote_join_list_buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online list-rooms");
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

char *param_zeng_online_client_rename_room;

//Devuelve 0 si no conectado
int zeng_online_client_rename_room_connect(void)
{

    char buffer_enviar[1024];

    //rename-room creator_pass n name
    sprintf(buffer_enviar,"zeng-online rename-room %s %d \"%s\"\n",
        created_room_creator_password,
        zeng_online_joined_to_room_number,
        param_zeng_online_client_rename_room);

    return zoc_open_command_close(buffer_enviar,"rename-room");

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

void *zeng_online_client_rename_room_function(void *nada GCC_UNUSED)
{
    zeng_online_client_rename_room_thread_running=1;

	//Conectar a remoto

	if (!zeng_online_client_rename_room_connect()) {
		//Desconectar solo si el socket estaba conectado

        //Desconectar los que esten conectados
        //TODO zeng_disconnect_remote();

		zeng_online_client_rename_room_thread_running=0;
		return 0;
	}


	zeng_online_client_rename_room_thread_running=0;

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

    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: ZENG Online: Sending %s",command_name_for_info);


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
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: Allowed key on profile: %d",allowed_keys[i][j]);
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

    //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: ZENG Online: Sending %s",command_name_for_info);


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





	zeng_online_client_authorize_join_thread_running=0;

	return 0;

}


void *zoc_stop_slave_thread_function(void *nada GCC_UNUSED)
//void zoc_stop_slave_thread(void)
{
    zeng_online_client_stop_slave_thread_thread_running=1;

    //Primero intentar que salga de las habitaciones de manera controlada y darle un minimo de tiempo
    //El thread con mas pausa entre comprobaciones es de 1 segundo, por tanto 2 segundos deberia ser suficiente
    zoc_exit_from_room=1;
    sleep(2);

    //Y luego cancelar los threads, si es que siguen vivos
    //En teoria no sucede nada si se intenta cancelar un thread que ya no existe
    //Realmente retorna error de : ESRCH  No thread with the ID thread could be found.
    //Creo que si desde que finaliza el thread, hasta que se cancela, si se crea un nuevo
    //thread en medio, con mismo ID (no se si eso es posible), entonces se cancelaria ese nuevo thread
    //Como en teoria no estoy creando un thread en medio, no sucederá nunca eso
    pthread_cancel(thread_zoc_slave_thread);
    pthread_cancel(thread_zoc_slave_thread_secondary_commands);
    pthread_cancel(thread_zoc_slave_thread_stream_audio);
    zoc_pending_apply_received_snapshot=0;
    zoc_pending_apply_received_streaming_display=0;

    zeng_online_client_stop_slave_thread_thread_running=0;

    return 0;
}

void *zoc_stop_master_thread_function(void *nada GCC_UNUSED)
//void zoc_stop_master_thread(void)
{
    zeng_online_client_stop_master_thread_thread_running=1;
    //Primero intentar que salga de las habitaciones de manera controlada y darle un minimo de tiempo
    //El thread con mas pausa entre comprobaciones es de 1 segundo, por tanto 2 segundos deberia ser suficiente
    zoc_exit_from_room=1;
    sleep(2);

    //Y luego cancelar los threads, si es que siguen vivos
    //En teoria no sucede nada si se intenta cancelar un thread que ya no existe
    pthread_cancel(thread_zoc_master_thread);
    pthread_cancel(thread_zoc_master_thread_secondary_commands);
    pthread_cancel(thread_zoc_master_thread_stream_audio);
    zeng_online_client_stop_master_thread_thread_running=0;

    return 0;
}

void zeng_online_client_list_rooms(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_list_rooms, NULL, &zeng_online_client_list_rooms_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online list rooms pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_list_rooms);


}

void zeng_online_client_list_users(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_list_users, NULL, &zeng_online_client_list_users_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online list users pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_list_users);


}

void zeng_online_client_leave_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_leave_room, NULL, &zeng_online_client_leave_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online leave room pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online autojoin pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_autojoin_room);


}

void zeng_online_client_disable_autojoin_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_disable_autojoin_room, NULL, &zeng_online_client_disable_autojoin_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online disable autojoin pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_disable_autojoin_room);


}

//Hilo que se encarga de detener los hilos de slave
void zeng_online_client_stop_slave_thread(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_stop_slave_thread, NULL, &zoc_stop_slave_thread_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online stop slave thread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_stop_slave_thread);


}

//Hilo que se encarga de detener los hilos de master
void zeng_online_client_stop_master_thread(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_stop_master_thread, NULL, &zoc_stop_master_thread_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online stop master thread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_stop_master_thread);


}


void zeng_online_client_write_message_room(char *message)
{

	//Inicializar thread
    strcpy(param_zeng_online_client_write_message_room_message,message);

	if (pthread_create( &thread_zeng_online_client_write_message_room, NULL, &zeng_online_client_write_message_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online write message pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online kick user pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online set max players room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_max_players_room);


}

void zeng_online_client_rename_room(char *name)
{

	//Inicializar thread
    param_zeng_online_client_rename_room=name;


	if (pthread_create( &thread_zeng_online_client_rename_room, NULL, &zeng_online_client_rename_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online rename room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_rename_room);


}


//Parametro allow_disallow : 1: allow, 0: disallow
void zeng_online_client_allow_message_room(int allow_disallow)
{

    param_zeng_online_client_allow_message_room_allow_disallow=allow_disallow;



	if (pthread_create( &thread_zeng_online_client_allow_message_room, NULL, &zeng_online_client_allow_message_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online allow/disallow message pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_allow_message_room);


}

void zeng_online_client_get_profile_keys(void)
{


	if (pthread_create( &thread_zeng_online_client_get_profile_keys, NULL, &zeng_online_client_get_profile_keys_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online get profile keys pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_get_profile_keys);


}

void zeng_online_client_send_profile_keys(void)
{

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: Sending restricted keys to server");

	if (pthread_create( &thread_zeng_online_client_send_profile_keys, NULL, &zeng_online_client_send_profile_keys_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online send profile keys pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_send_profile_keys);


}

void zeng_online_client_destroy_room(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_destroy_room, NULL, &zeng_online_client_destroy_room_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online destroy room pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_destroy_room);


}

void zeng_online_client_join_list(void)
{

	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_join_list, NULL, &zeng_online_client_join_list_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online join list pthread");
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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online authorize join pthread");
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //zsock_wait_until_command_prompt(indice_socket);

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Sending join-room");

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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online join: %s",z_sock_get_error(escritos));
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
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online join (length %d): \n[\n%s\n]",leidos,buffer);
        }

        if (leidos<0) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online join: %s",z_sock_get_error(leidos));
            return 0;
        }

        if (leidos==0) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: No response read from join yet");
        }

        reintentos--;
    } while (leidos==0 && reintentos>0);

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online join");
        return 0;
    }

    //printf("Retorno join-room: [%s]\n",buffer);
    //Si hay ERROR
    if (strstr(buffer,"ERROR")!=NULL) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error joining room: %s",buffer);
        return 0;
    }

    //Dividir el user_password y los permisos y el streaming mode
    int i;
    for (i=0;buffer[i] && buffer[i]!=' ';i++);

    if (buffer[i]==0) {
        //llegado al final sin que haya espacios. error
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error joining room, no permissions detected: %s",buffer);
        return 0;
    }

    //truncar de momento hasta aqui
    buffer[i]=0;
    strcpy(created_room_user_password,buffer);

    //Y seguir hasta buscar los permisos
    i++;

    int inicio_index_permissions=i;

    for (;buffer[i] && buffer[i]!=' ';i++);

    if (buffer[i]==0) {
        //llegado al final sin que haya espacios. error
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error joining room, no streaming mode detected: %s",buffer);
        return 0;
    }

    //truncar de momento hasta aqui
    buffer[i]=0;

    created_room_user_permissions=parse_string_to_number(&buffer[inicio_index_permissions]);



    i++;

    created_room_streaming_mode=parse_string_to_number(&buffer[i]);

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: User password: [%s]",created_room_user_password);
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: User permissions: [%d]",created_room_user_permissions);
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Streaming mode: [%d]",created_room_streaming_mode);

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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online join room pthread");
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

    int modo_streaming_efectivo=streaming_enabled_when_creating;

    //Si maquina no es spectrum, desactivar modo streaming
    if (!MACHINE_IS_SPECTRUM) modo_streaming_efectivo=0;

    //zeng_remote_list_rooms_buffer[0]=0;

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //zsock_wait_until_command_prompt(indice_socket);

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Sending create-room");

    char buffer_enviar[1024];
    sprintf(buffer_enviar,"zeng-online create-room %d \"%s\" %d\n",param_create_room_number,param_create_room_name,modo_streaming_efectivo);

    int escritos=z_sock_write_string(indice_socket,buffer_enviar);

    if (escritos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't send zeng-online create-room: %s",z_sock_get_error(escritos));
        return 0;
    }


    //reintentar
    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for zeng-online create-room (length %d): \n[\n%s\n]",leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive zeng-online create-room: %s",z_sock_get_error(leidos));
        return 0;
    }

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",buffer);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online create-room");
        return 0;
    }

    //printf("Retorno crear-room: [%s]\n",buffer);
    //Si hay ERROR
    if (strstr(buffer,"ERROR")!=NULL) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error creating room: %s",buffer);
        return 0;
    }
    strcpy(created_room_creator_password,buffer);

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Creator password: [%s]",created_room_creator_password);

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
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online create room pthread");
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
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Sending snapshot");

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
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_snapshot_mem_hexa);
    }

    //printf("Recibido respuesta despues de put-snapshot: [%s]\n",buffer);
    return leidos;


}

int zoc_send_streaming_display_counter=0;

//Para las dos pantallas que se envian cada vez: la diferencial y la completa
char *zoc_send_streaming_display_mem_hexa[2]={NULL,NULL};

int zoc_send_streaming_display(int indice_socket,int slot)
{

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Sending streaming_display");

    int posicion_command;
    int escritos,leidos;


    char buffer_comando[200];
    //printf ("Sending put-streaming_display\n");
    //streaming-put-display creator_pass n s data
    sprintf(buffer_comando,"zeng-online streaming-put-display %s %d %d ",created_room_creator_password,zeng_online_joined_to_room_number,slot);

    //printf("Sending command: [%s]\n",buffer_comando);

    escritos=z_sock_write_string(indice_socket,buffer_comando);
    //printf("after z_sock_write_string 1\n");
    if (escritos<0) return escritos;



    escritos=z_sock_write_string(indice_socket,zoc_send_streaming_display_mem_hexa[slot]);
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
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_streaming_display_mem_hexa);
    }

    zoc_send_streaming_display_counter++;

    //printf("Recibido respuesta despues de put-streaming_display: [%s]\n",buffer);
    return leidos;


}

char *zoc_send_streaming_audio_mem_hexa=NULL;
int zoc_master_pending_send_streaming_audio=0;

int zoc_send_streaming_audio_counter=0;

int zoc_send_streaming_audio(int indice_socket)
{

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Sending streaming_audio");

    int posicion_command;
    int escritos,leidos;


    char buffer_comando[200];
    //printf ("Sending put-streaming_display\n");
    //"streaming-put-audio creator_pass n data
    sprintf(buffer_comando,"zeng-online streaming-put-audio %s %d ",created_room_creator_password,zeng_online_joined_to_room_number );

    //printf("Sending command: [%s]\n",buffer_comando);

    escritos=z_sock_write_string(indice_socket,buffer_comando);
    //printf("after z_sock_write_string 1\n");
    if (escritos<0) return escritos;

    //printf("Length audio: %d\n",strlen(zoc_send_streaming_audio_mem_hexa));

    escritos=z_sock_write_string(indice_socket,zoc_send_streaming_audio_mem_hexa);
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
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_streaming_display_mem_hexa);
    }

    zoc_send_streaming_audio_counter++;

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
    //Si el tamaño del total de eventos permitidos en la fifo excede nuestro buffer, la llamada
    //a zsock_read_all_until_newline seguramente se quedaria esperando hasta hacer timeout porque no encontraria el newline final
    //Unos 30 caracteres por linea. * el maximo de fifo, *2 para margen suficiente
    //char buffer[1024];
    #define ZOC_GET_KEYS_MAX_BUFFER (ZENG_FIFO_SIZE*30*2)
    char buffer[ZOC_GET_KEYS_MAX_BUFFER+1];

    //Leer hasta prompt
    int posicion_command;

    //printf ("antes de leer hasta command prompt\n");
    //int leidos=zsock_read_all_until_newline(indice_socket,(z80_byte *)buffer,1023,&posicion_command);
    int leidos=zsock_read_all_until_newline(indice_socket,(z80_byte *)buffer,ZOC_GET_KEYS_MAX_BUFFER,&posicion_command);
    //printf("leidos on zoc_get_keys: %d\n",leidos);

    if (leidos>0) {
        buffer[leidos]=0; //fin de texto
        //printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
                            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Processing from ZRCP command send-keys-event: key %d event %d",numero_key,numero_event);
                            //printf ("Processing from ZRCP command send-keys-event: key %d event %d\n",numero_key,numero_event);

                            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Info joystick: fire: %d up: %d down: %d left: %d right: %d",
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
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received answer for get_keys with invalid number of fields");
            }


            inicio_linea=i+1;
        }
    }

}

int zoc_pending_apply_received_streaming_audio=0;

int zoc_last_id_stream_audio=-1;



z80_byte *zoc_get_audio_mem_binary=NULL;

z80_byte *zoc_get_audio_mem_binary_second_buffer=NULL;

int zoc_get_audio_mem_binary_longitud=0;

int zoc_streaming_audio_received_counter=0;

int zoc_get_stream_audio_continuous(int indice_socket)
{

    //printf("Pidiendo stream audio continuo\n");

    //int posicion_command;
    //int escritos;
    int leidos;

    //Ver si hay datos disponibles en el socket
	int sock_number=get_socket_number(indice_socket);

	if (sock_number<0) {
		return 0;
	}

    //printf("see if data available on zoc_get_stream_audio_continuous\n");

    if (!zsock_available_data(sock_number)) {
        //printf("no data available on zoc_get_stream_audio_continuous\n");
        return 1;
    }


    //printf("--Obteniendo stream audio remoto %d\n",contador_segundo);


    int max_stream_continuo=ZOC_STREAMING_AUDIO_BUFFER_SIZE*2*100;


    char *zoc_get_audio_mem_hexa=util_malloc(max_stream_continuo,"Can not allocate memory for getting audio");



    leidos=zsock_read_all_until_newline_streamaudio(indice_socket,(z80_byte *)zoc_get_audio_mem_hexa,max_stream_continuo);



    if (leidos>0) {
        zoc_get_audio_mem_hexa[leidos]=0; //fin de texto
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive get-audio: %s",z_sock_get_error(leidos));
        return 0;
    }

    //printf("Buffer audio leido: [%s]\n",zoc_get_audio_mem_hexa);

    //printf("--Despues obtener audio remoto. posicion_command=%d leidos=%d\n",posicion_command,leidos);



    //printf("--Fin recepcion obtener audio remoto. %d\n",contador_segundo);

    //printf("Buffer despues de truncar: [%s]\n",&zoc_get_audio_mem_hexa[inicio_datos_snapshot]);

    //TODO: detectar texto ERROR en respuesta
    //return leidos;

    //Convertir hexa a memoria
    if (zoc_get_audio_mem_binary==NULL) {
        zoc_get_audio_mem_binary=util_malloc(ZOC_STREAMING_AUDIO_BUFFER_SIZE,"Can not allocate memory for apply audio");
    }


    char *s=zoc_get_audio_mem_hexa;
    int parametros_recibidos=0;



    z80_byte *destino;
    destino=zoc_get_audio_mem_binary;

    int total_stream_procesado=0;

    //No leer mas de un stream de audio
    while (*s && total_stream_procesado<ZOC_STREAMING_AUDIO_BUFFER_SIZE && leidos) {
        *destino=(util_hex_nibble_to_byte(*s)<<4) | util_hex_nibble_to_byte(*(s+1));
        destino++;

        parametros_recibidos++;

        s++;
        if (*s) s++;

        total_stream_procesado++;

        leidos--;
    }


    zoc_get_audio_mem_binary_longitud=parametros_recibidos;

    free(zoc_get_audio_mem_hexa);

    if (!zoc_pending_apply_received_streaming_audio) {
        //copiar el buffer recibido al segundo buffer
        //printf("Copiar el buffer recibido al segundo buffer. longitud=%d\n",zoc_get_audio_mem_binary_longitud);


        if (zoc_get_audio_mem_binary_second_buffer==NULL) {
            zoc_get_audio_mem_binary_second_buffer=util_malloc(ZOC_STREAMING_AUDIO_BUFFER_SIZE,"Can not allocate memory for apply audio");
        }

        if (zoc_get_audio_mem_binary_longitud<=2) {
            //Se indica que son dos valores (canales estereo) en periodo de silencio. Hay que escribir ese valor repetido en todo el sample
            //Si en vez de esto metieramos simplemente 0, se escucharian molestos clics por ejemplo al mover el cursor
            //al moverse por el menu del spectrum 128k
            //Esto es debido a que alterariamos el ultimo valor de sample reproducido
            //En cambio con esto repetimos el ultimo valor de sample y asi no se escucha nada extraño
            memset(zoc_get_audio_mem_binary_second_buffer,zoc_get_audio_mem_binary[0],ZOC_STREAMING_AUDIO_BUFFER_SIZE);
            //printf("Periodo silencio %d. write %d\n",contador_segundo,(char) zoc_get_audio_mem_binary[0]);
        }

        else {
            memcpy(zoc_get_audio_mem_binary_second_buffer,zoc_get_audio_mem_binary,ZOC_STREAMING_AUDIO_BUFFER_SIZE);
            //printf("Periodo no silencio %d\n",contador_segundo);
        }

        zoc_pending_apply_received_streaming_audio=1;

        zoc_streaming_audio_received_counter++;
    }

    //printf("--Fin obtener audio remoto. %d\n",contador_segundo);

    return 1;

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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            //TODO return -1;
        }


    }

    return indice_socket;

}

int zoc_start_connection_get_stream_audio(void)
{
    //conectar a remoto

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);
    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }



   char buffer_comando[256];



    //streaming-get-audio-cont user_pass n
    sprintf(buffer_comando,"zeng-online streaming-get-audio-cont %s %d\n",
        created_room_user_password,zeng_online_joined_to_room_number);


    int escritos=z_sock_write_string(indice_socket,buffer_comando);



   //Si ha habido error al escribir en socket
    if (escritos<0) {
        //TODO return escritos;
    }


    else {

        //printf ("before read command sending get-keys on streaming-get-audio-cont\n");
        //1 segundo maximo de reintentos
        //sobretodo util en la primera conexion, dado que el buffer vendra vacio, siempre esperara 1 segundo
        int leidos=zsock_read_all_until_command_max_reintentos(indice_socket,(z80_byte *)buffer_initial,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command,100);
        //printf ("after read command sending get-keys on streaming-get-audio-cont. leidos=%d\n",leidos);

        if (leidos>0) {
            buffer_initial[leidos]=0; //fin de texto
            //printf("Received text after get-keys: (length: %d):\n[\n%s\n]\n",leidos,buffer_initial);
        }

        if (leidos<0) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
            return -1;
        }


    }


    return escritos;
}

int zoc_keys_tecla_conmutada(zeng_key_presses *elemento)
{

    if (!zeng_fifo_get_current_size()) return 0;

    if (elemento->pressrelease!=0) return 0;

    //Lo de ahora es un release. Lo siguiente es un press de la misma tecla?
    //Con esto vemos si la tecla se ha pulsado y liberado, con lo cual, quitaremos esas dos acciones
    zeng_key_presses elemento_siguiente;

    zeng_fifo_begin_lock();

    zeng_fifo_peek_element_no_lock(&elemento_siguiente);

    if (elemento_siguiente.pressrelease && elemento_siguiente.tecla==elemento->tecla) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Removing repeated toggle key %d, possible due to X11",elemento_siguiente.tecla);
        //printf("ZENG Online Client: Removing repeated toggle key %d, possible due to X11\n",elemento_siguiente.tecla);
        //quitar el siguiente elemento
        //por tanto lo que se quedara es el release
        zeng_fifo_read_element_no_lock(&elemento_siguiente);
        zeng_fifo_end_lock();
        return 1;
    }

    zeng_fifo_end_lock();

    return 0;

}

int zoc_keys_send_counter=0;

int zoc_keys_send_pending(int indice_socket,int *enviada_alguna_tecla)
{

    *enviada_alguna_tecla=0;

    int error_desconectar=0;
    zeng_key_presses elemento;

    //No quedarse toda la vida en este bucle. Puede suceder que mientras estamos dentro, se agreguen mas teclas,
    //y no saldriamos nunca de aqui
    //Como maximo enviar la cantidad de teclas que habia en la fifo al entrar
    int tamanyo_cola=zeng_fifo_get_current_size();


    /*if (tamanyo_cola) {
        printf("\n");
        printf("Tamanyo fifo antes enviar a remote: %d\n",tamanyo_cola);
    }*/

    //zeng_fifo_debug_show_fifo();

    while (tamanyo_cola>0 && !error_desconectar) {
        if (!zeng_fifo_read_element(&elemento)) {
            *enviada_alguna_tecla=1;
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Read event from zeng fifo and sending it to remote: key %d pressrelease %d",elemento.tecla,elemento.pressrelease);

            //printf ("ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d\n",elemento.tecla,elemento.pressrelease);

            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Info joystick: fire: %d up: %d down: %d left: %d right: %d",
                UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);


            if (!zoc_keys_tecla_conmutada(&elemento)) {

                //command> help send-keys-event
                //Syntax: send-keys-event key event
                int error=zoc_send_keys(indice_socket,&elemento);
                if (error<0) error_desconectar=1;

                zoc_keys_send_counter++;
            }

            else {
                //printf("No enviar tecla conmutada %d pressrelease %d\n",elemento.tecla,elemento.pressrelease);
            }
        }


        tamanyo_cola--;

        //printf("tamanyo fifo despues send remote: %d\n",zeng_fifo_get_current_size());

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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Received text for get-join-queue-size (length %d)",leidos);
        //printf("ZENG: Received text for get-join-queue-size (length %d): \n[\n%s\n]\n",leidos,buffer);
    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive get-join-queue-size: %s",z_sock_get_error(leidos));
        return 0;
    }


    //printf("after zsock_read_all_until_command\n");
    //printf("Recibido respuesta despues de get-snapshot-id: [%s]\n",buffer);

    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;
        //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_snapshot_mem_hexa);
    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX get-join-queue-size");
        return 0;
    }

    //printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

    //int leer_snap=0;

    //Detectar si error
    if (strstr(buffer,"ERROR")!=NULL) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error getting get-join-queue-size");
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

int zoc_common_alive_user_send_counter=0;

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


        //int return_value=zoc_common_send_command(indice_socket,buffer_enviar,"alive");
        zoc_common_send_command(indice_socket,buffer_enviar,"alive");

        zoc_common_alive_user_send_counter++;


        //if (!return_value) return 0;

    }
}

int zoc_common_get_messages_received_counter=0;

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
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: There is a new message");
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

                zoc_common_get_messages_received_counter++;

            }


            zoc_last_message_id=id_actual;
        }
    }
}

int zoc_sent_snapshots_counter=0;

void master_thread_put_snapshot(int indice_socket)
{
    if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT) {


        if (zoc_pending_send_snapshot) {
            //printf("Putting snapshot\n");
            int error=zoc_send_snapshot(indice_socket);

            if (error<0) {
                //TODO
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error sending snapshot to zeng online server");
            }

            //Enviado. Avisar no pendiente
            zoc_pending_send_snapshot=0;
            //zeng_online_client_reset_scanline_counter();
            //printf("Snapshot sent\n");

            zoc_sent_snapshots_counter++;
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



    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }


    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    zoc_last_message_id=zoc_get_message_id(indice_socket);
    //printf("Initial message id: %d\n",zoc_last_message_id);

    int indice_socket_get_keys=zoc_start_connection_get_keys();

    //bucle continuo de si hay snapshot de final de frame, enviarlo a remoto
    //TODO: ver posible manera de salir de aqui??
    while (!zoc_exit_from_room) {

        if (zeng_online_client_end_frame_reached) {
            zeng_online_client_end_frame_reached=0;

            //Nota: un master normal deberia tener todos permisos, sin necesidad de comprobarlos
            //PERO si queremos un master que solo gestione autorizaciones, cambios de parametros en room, etc,
            //sin que envie snapshots o reciba teclas o envie teclas, se le pueden quitar dichos permisos,
            //y podremos gestionar el resto

            //printf("before get snapshot\n");

            //En modo streaming escribimos display en vez de snapshot

            if (created_room_streaming_mode) {
                //printf("Modo streaming. Escribimos display en vez de snapshot\n");

                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_DISPLAY) {

                    //printf("Putting display\n");

                    if (zoc_pending_send_streaming_display) {

                        //Enviar la incremental y la completa
                        int error=zoc_send_streaming_display(indice_socket,0);

                        if (error<0) {
                            //TODO
                            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error sending streaming_display to zeng online server");
                        }

                        error=zoc_send_streaming_display(indice_socket,1);


                        if (error<0) {
                            //TODO
                            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error sending streaming_display to zeng online server");
                        }

                        //Enviado. Avisar no pendiente
                        zoc_pending_send_streaming_display=0;

                        //printf("streaming_display sent\n");
                    }

                }

                //En modo streaming tambien enviamos snapshot pero mucho menos frecuentemente
                master_thread_put_snapshot(indice_socket);

            }

            else {

                master_thread_put_snapshot(indice_socket);

            }

            //Un master que recibe snapshots
            //Esto ya no se usa, era inicialmente un manager que recibia snapshots
            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT) {

                //printf("Recibiendo snapshot porque somos master manager. contador_segundo=%d\n",contador_segundo);
                int error=zoc_receive_snapshot(indice_socket);
                //TODO gestionar bien este error
                if (error<0) {
                    //TODO
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error getting snapshot from zeng online server");
                    usleep(10000); //dormir 10 ms
                }

            }

            //Si estabamos reentrando como master, obtener el snapshot que habia
            if (zoc_rejoining_as_master) {
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: Receiving first snapshot as we are rejoining as master");
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


            /*

            Muevo todo esto a secondary thread

            //Tambien a final de cada frame, y cada 50 veces (o sea 1 segundo), ver si hay pendientes autorizaciones
            contador_obtener_autorizaciones++;

            if ((contador_obtener_autorizaciones % 50)==0) {
                zoc_get_pending_authorization_size(indice_socket);
            }

            zoc_common_get_messages_slave_master(indice_socket);

            zoc_common_alive_user(indice_socket);

            */

        }

        //TODO: parametro configurable
        usleep(1000); //1 ms (20 ms es un frame de video)

        pthread_testcancel();

    }

    //finalizar conexion
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_master_thread_function");
    z_sock_close_connection(indice_socket);

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_master_thread_function - indice_socket_get_keys");
    z_sock_close_connection(indice_socket_get_keys);

	return 0;

}

void *zoc_master_thread_function_secondary_commands(void *nada GCC_UNUSED)
{


    //zeng_remote_list_rooms_buffer[0]=0;

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");



    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }


    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    zoc_last_message_id=zoc_get_message_id(indice_socket);
    //printf("Initial message id: %d\n",zoc_last_message_id);




    while (!zoc_exit_from_room) {



        zoc_get_pending_authorization_size(indice_socket);

        zoc_common_get_messages_slave_master(indice_socket);

        zoc_common_alive_user(indice_socket);


        //TODO: parametro configurable
        sleep(1);

        pthread_testcancel();

    }

    //finalizar conexion
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_master_thread_function_secondary_commands");
    z_sock_close_connection(indice_socket);

	return 0;

}



void *zoc_master_thread_function_stream_audio(void *nada GCC_UNUSED)
{

    //TODO: este thread se crea siempre, pero si no hemos habilitado el modo streaming, no tiene sentido lanzarlo
    //porque al final no hara nada

    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");



    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }


    while (!zoc_exit_from_room) {

        if (created_room_streaming_mode) {


            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_AUDIO) {


                if (zoc_master_pending_send_streaming_audio) {

                    //printf("Putting audio %d\n",contador_segundo);

                    int error=zoc_send_streaming_audio(indice_socket);

                    //printf("After putting audio %d\n",contador_segundo);


                    if (error<0) {
                        //TODO
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error sending streaming_audio to zeng online server");
                    }


                    zoc_master_pending_send_streaming_audio=0;
                }

            }

        }



        //TODO: parametro configurable
        usleep(100); //0.1 ms (20 ms es un frame de video)

        pthread_testcancel();

    }

    //finalizar conexion
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_master_thread_function_stream_audio");
    z_sock_close_connection(indice_socket);

	return 0;

}




int zoc_receive_snapshot(int indice_socket)
{
    //printf("Inicio zoc_receive_snapshot llamado desde:\n");
    //debug_exec_show_backtrace();

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Receiving snapshot");

    int posicion_command;
    int escritos,leidos;


    char buffer_comando[200];



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
            //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text for get-snapshot-id (length %d): \n[\n%s\n]",leidos,buffer);
        }

        if (leidos<0) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive get-snapshot-id: %s",z_sock_get_error(leidos));
            return 0;
        }


        //printf("after zsock_read_all_until_command\n");
        //printf("Recibido respuesta despues de get-snapshot-id: [%s]\n",buffer);

        //1 mas para eliminar el salto de linea anterior a "command>"
        if (posicion_command>=1) {
            buffer[posicion_command-1]=0;
            //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_snapshot_mem_hexa);
        }
        else {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX get-snapshot-id");
            return 0;
        }

        //printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

        int leer_snap=0;

        //Detectar si error
        if (strstr(buffer,"ERROR")!=NULL) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error getting snapshot-id");
            //printf ("ZENG Online Client: Error getting snapshot-id: [%s]\n",buffer);
        }
        else {
            //Ver si id diferente
            //printf("snapshot id: %s\n",buffer);

            int nuevo_id=parse_string_to_number(buffer);
            if (nuevo_id!=zoc_receive_snapshot_last_id) {
                zeng_online_snapshot_diff=nuevo_id-zoc_receive_snapshot_last_id;
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Snapshot id difference from last snapshot: %d",zeng_online_snapshot_diff);

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
                //Pero en modo streaming no quiero que salte
                //Nota: en modo streaming tambien se reciben snapshots pero solo 1 cada minuto
                //y solo se aplica al hacer un leave
                if (!created_room_streaming_mode && zeng_online_snapshot_diff>alertar_diferencia) {
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
                //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text for get-snapshot (length %d): \n[\n%s\n]",leidos,zoc_get_snapshot_mem_hexa);
            }

            if (leidos<0) {
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive get-snapshot: %s",z_sock_get_error(leidos));
                return 0;
            }


            //printf("after zsock_read_all_until_command\n");
            //printf("Recibido respuesta despues de get-snapshot: [%s]\n",zoc_get_snapshot_mem_hexa);

            //1 mas para eliminar el salto de linea anterior a "command>"
            if (posicion_command>=1) {
                zoc_get_snapshot_mem_hexa[posicion_command-1]=0;
                //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_snapshot_mem_hexa);
            }
            else {
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online get-snapshot");
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



            z80_byte *destino;
            destino=zoc_get_snapshot_mem_binary_comprimido;

            //Uso util_hex_nibble_to_byte porque es mas rapido que no andar haciendo parse_string_to_number a string de cada byte
            while (*s) {
                *destino=(util_hex_nibble_to_byte(*s)<<4) | util_hex_nibble_to_byte(*(s+1));
                destino++;

                parametros_recibidos++;

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

            //Esto es solo para que la ventana de progreso al hacer un leave room vea que ya se ha aplicado el snapshot
            zeng_online_client_get_snapshot_applied_finished=1;

        }

    }
    else {
        //printf("get-snapshot no disponible. esperar\n");

    }



    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: End Receiving snapshot");
    return 1;


}

int zoc_streaming_display_received_counter=0;

int zoc_receive_streaming_display(int indice_socket,int slot)
{


    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Receiving streaming display");

    int posicion_command;
    int escritos,leidos;


    char buffer_comando[200];



    if (!zoc_pending_apply_received_streaming_display) {

        #define ZENG_BUFFER_INITIAL_CONNECT 199

        //Leer algo
        //char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];


        //streaming-get-display user_pass n s
        sprintf(buffer_comando,"zeng-online streaming-get-display %s %d %d\n",created_room_user_password,zeng_online_joined_to_room_number,slot);

        //printf("buffer_comando: [%s]\n",buffer_comando);

        escritos=z_sock_write_string(indice_socket,buffer_comando);
        //printf("after z_sock_write_string 1\n");
        if (escritos<0) return escritos;



//Ver si hay datos disponibles y no esta pendiente aplicar ultimo streaming_display

        if (zoc_get_streaming_display_mem_hexa==NULL) {
            zoc_get_streaming_display_mem_hexa=util_malloc(ZRCP_GET_PUT_STREAMING_DISPLAY_MEM*2,"Can not allocate memory for getting streaming_display"); //16 MB es mas que suficiente
        }

        //Leer hasta prompt
        //printf("before zsock_read_all_until_command\n");
        leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zoc_get_streaming_display_mem_hexa,ZRCP_GET_PUT_STREAMING_DISPLAY_MEM*2,&posicion_command);

        //printf("leidos despues de get-streaming_display: %d\n",leidos);


        if (leidos>0) {
            zoc_get_streaming_display_mem_hexa[leidos]=0; //fin de texto
            //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text for get-streaming_display (length %d): \n[\n%s\n]",leidos,zoc_get_streaming_display_mem_hexa);
            //printf("respuesta leido: [%s]\n",zoc_get_streaming_display_mem_hexa);
        }

        if (leidos<0) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive get-streaming-display: %s",z_sock_get_error(leidos));
            return 0;
        }


        //printf("after zsock_read_all_until_command\n");
        // printf("Recibido respuesta despues de get-streaming_display: [%s]\n",zoc_get_streaming_display_mem_hexa);

        //1 mas para eliminar el salto de linea anterior a "command>"
        if (posicion_command>=1) {
            zoc_get_streaming_display_mem_hexa[posicion_command-1]=0;
            //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_streaming_display_mem_hexa);
        }
        else {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online get-streaming_display");
            return 0;
        }

        //printf("Recibido respuesta despues de truncar: [%s]\n",zoc_get_streaming_display_mem_hexa);


        //printf("Buffer despues de truncar: [%s]\n",&zoc_get_streaming_display_mem_hexa[inicio_datos_streaming_display]);

        //TODO: detectar texto ERROR en respuesta
        //return leidos;

        //Convertir hexa a memoria
        if (zoc_get_streaming_display_mem_binary==NULL) {
            zoc_get_streaming_display_mem_binary=util_malloc(ZRCP_GET_PUT_STREAMING_DISPLAY_MEM,"Can not allocate memory for apply streaming_display");
        }


        char *s=zoc_get_streaming_display_mem_hexa;
        int parametros_recibidos=0;



        z80_byte *destino;
        destino=zoc_get_streaming_display_mem_binary;

        while (*s) {
            *destino=(util_hex_nibble_to_byte(*s)<<4) | util_hex_nibble_to_byte(*(s+1));
            destino++;

            parametros_recibidos++;

            s++;
            if (*s) s++;
        }


        zoc_get_streaming_display_mem_binary_longitud=parametros_recibidos;


        //printf("Apply streaming_display. Compressed %d Uncompressed %d\n",zoc_get_streaming_display_mem_binary_longitud_comprimido,zoc_get_streaming_display_mem_binary_longitud);

        zoc_pending_apply_received_streaming_display=1;

        zoc_streaming_display_received_counter++;



    }
    else {
        //printf("get-streaming_display no disponible. esperar\n");
        //printf("Pantalla pendiente de aplicar aun\n");

    }



    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: End Receiving streaming_display");
    return 1;


}


//Este metodo NO se usa, y se usa zoc_get_stream_audio_continuous
int zoc_receive_streaming_audio(int indice_socket)
{

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Receiving streaming audio");

    int posicion_command;
    int escritos,leidos;


    char buffer_comando[200];


    #define ZENG_BUFFER_INITIAL_CONNECT 199

    //Leer algo
    char buffer[ZENG_BUFFER_INITIAL_CONNECT+1];

    //Ver si el id de audio ha cambiado

    //"streaming-get-audio-id user_pass n
    sprintf(buffer_comando,"zeng-online streaming-get-audio-id %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);
    escritos=z_sock_write_string(indice_socket,buffer_comando);
    //printf("after z_sock_write_string 1\n");
    if (escritos<0) return escritos;

    //Leer hasta prompt
    //printf("before zsock_read_all_until_command\n");
    leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);

    if (leidos>0) {
        buffer[leidos]=0; //fin de texto

    }

    if (leidos<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive streaming-get-audio-id: %s",z_sock_get_error(leidos));
        return 0;
    }




    //1 mas para eliminar el salto de linea anterior a "command>"
    if (posicion_command>=1) {
        buffer[posicion_command-1]=0;

    }
    else {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX get-stream-audio-id");
        return 0;
    }

    //printf("Recibido respuesta despues de truncar: [%s]\n",buffer);

    int leer_audio=0;

    //Detectar si error
    if (strstr(buffer,"ERROR")!=NULL) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error getting streaming-get-audio-id");
    }
    else {
        //Ver si id diferente
        //printf("audio id: %s\n",buffer);

        int nuevo_id=parse_string_to_number(buffer);
        if (nuevo_id!=zoc_last_id_stream_audio) {


            zoc_last_id_stream_audio=nuevo_id;
            leer_audio=1;
        }
        else {
            //printf("stream audio es el mismo que el anterior\n");
        }
    }


    if (leer_audio) {

        //printf("Obteniendo stream audio remoto %d\n",contador_segundo);


        //"streaming-get-audio user_pass n
        sprintf(buffer_comando,"zeng-online streaming-get-audio %s %d\n",created_room_user_password,zeng_online_joined_to_room_number);
        escritos=z_sock_write_string(indice_socket,buffer_comando);
        //printf("after z_sock_write_string 1\n");
        if (escritos<0) return escritos;


        char *zoc_get_audio_mem_hexa=util_malloc(ZOC_STREAMING_AUDIO_BUFFER_SIZE*2+100,"Can not allocate memory for getting audio");


        //Leer hasta prompt
        //printf("before zsock_read_all_until_command\n");
        leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)zoc_get_audio_mem_hexa,ZOC_STREAMING_AUDIO_BUFFER_SIZE*2+100,&posicion_command);




        if (leidos>0) {
            zoc_get_audio_mem_hexa[leidos]=0; //fin de texto

        }

        if (leidos<0) {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't receive get-audio: %s",z_sock_get_error(leidos));
            return 0;
        }


        //printf("after zsock_read_all_until_command\n");


        //1 mas para eliminar el salto de linea anterior a "command>"
        if (posicion_command>=1) {
            zoc_get_audio_mem_hexa[posicion_command-1]=0;
            //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Received text: %s",zoc_get_audio_mem_hexa);
        }
        else {
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error receiving ZEsarUX zeng-online get-audio");
            return 0;
        }

        //printf("Fin recepcion obtener audio remoto. %d\n",contador_segundo);

        //printf("Buffer despues de truncar: [%s]\n",&zoc_get_audio_mem_hexa[inicio_datos_snapshot]);

        //TODO: detectar texto ERROR en respuesta
        //return leidos;

        //Convertir hexa a memoria
        if (zoc_get_audio_mem_binary==NULL) {
            zoc_get_audio_mem_binary=util_malloc(ZOC_STREAMING_AUDIO_BUFFER_SIZE,"Can not allocate memory for apply audio");
        }


        char *s=zoc_get_audio_mem_hexa;
        int parametros_recibidos=0;



        z80_byte *destino;
        destino=zoc_get_audio_mem_binary;

        while (*s) {
            *destino=(util_hex_nibble_to_byte(*s)<<4) | util_hex_nibble_to_byte(*(s+1));
            destino++;

            parametros_recibidos++;

            s++;
            if (*s) s++;
        }


        zoc_get_audio_mem_binary_longitud=parametros_recibidos;

        free(zoc_get_audio_mem_hexa);

        if (!zoc_pending_apply_received_streaming_audio) {
            //copiar el buffer recibido al segundo buffer
            //printf("Copiar el buffer recibido al segundo buffer. longitud=%d\n",zoc_get_audio_mem_binary_longitud);
            if (zoc_get_audio_mem_binary_second_buffer==NULL) {
                zoc_get_audio_mem_binary_second_buffer=util_malloc(ZOC_STREAMING_AUDIO_BUFFER_SIZE,"Can not allocate memory for apply audio");
            }

            memcpy(zoc_get_audio_mem_binary_second_buffer,zoc_get_audio_mem_binary,ZOC_STREAMING_AUDIO_BUFFER_SIZE);
            zoc_pending_apply_received_streaming_audio=1;
        }

        //printf("Fin obtener audio remoto. %d\n",contador_segundo);

    }

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
                //DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: I have been kicked");
                put_footer_first_message("I have been kicked");
                zeng_online_client_leave_room();
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: I have been kicked from the ZENG Online room");
                return 1;
            }
        }

    }

    return 0;
}


void slave_thread_get_snapshot(int indice_socket)
{
    //Como se puede ver es el cliente quien gestiona los permisos en base a lo que el join le retornó el server
    //Pero luego el server no valida que el cliente tenga los permisos que está usando
    if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT) {

        //Recibir snapshot


        int error=zoc_receive_snapshot(indice_socket);
        //TODO gestionar bien este error
        if (error<0) {
            //TODO
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error getting snapshot from zeng online server");
            //printf("ZENG Online Client: Error getting snapshot from zeng online server\n");
            usleep(10000); //dormir 10 ms
        }


    }

}

//int zoc_snapshot_slave_streaming_counter=0;

int zoc_slave_force_get_snapshot=0;

int zeng_online_client_get_snapshot_applied_finished=0;

void *zoc_slave_thread_function(void *nada GCC_UNUSED)
{

    //conectar a remoto
    //Inicializar timeout para no recibir tempranos mensajes de "OFFLINE"
    zoc_last_snapshot_received_counter=ZOC_TIMEOUT_NO_SNAPSHOT;



    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    //zoc_last_message_id=zoc_get_message_id(indice_socket);
    //printf("Initial message id: %d\n",zoc_last_message_id);


    int indice_socket_get_keys;

    //No iniciar conexion para get keys si estamos en modo streaming
    if (!created_room_streaming_mode) {
        indice_socket_get_keys=zoc_start_connection_get_keys();
    }


    //int indice_socket_get_stream_audio=zoc_start_connection_get_stream_audio();

    //Para dar un margen al principio de conectar antes de salir aviso de dropout
    zoc_veces_audio_no_recibido_contador=-20;

    while (!zoc_exit_from_room) {

        if (zeng_online_client_end_frame_reached) {
            zeng_online_client_end_frame_reached=0;

            //En modo streaming solo leemos pantalla, y no leemos ni snapshot ni teclas
            if (created_room_streaming_mode) {
                //printf("Modo streaming. solo leemos pantalla\n");


                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_DISPLAY) {

                    if (!zoc_pending_apply_received_streaming_display) {
                        //Recibir snapshot
                        //printf("Recibir pantalla\n");

                        int slot=0;

                        //slot 0 es diferencial. Cada X frames diferenciales, pedir uno entero

                        if (zoc_slave_differential_displays_counter>=zoc_slave_differential_displays_limit_full) {
                            zoc_slave_differential_displays_counter=0;
                            slot=1;
                            //printf("Pedir pantalla entera\n");
                        }

                        else {
                            //printf("Pedir pantalla diferencial\n");
                            zoc_slave_differential_displays_counter++;
                        }



                        int error=zoc_receive_streaming_display(indice_socket,slot);
                        //TODO gestionar bien este error
                        if (error<0) {
                            //TODO
                            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Error getting streaming display from zeng online server");
                            usleep(10000); //dormir 10 ms
                        }

                        else {

                            //Ver si la pantalla recibida era al final una completa
                            if (zoc_get_streaming_display_mem_binary!=NULL) {
                                //printf("Recibida pantalla longitud %d\n",zoc_get_streaming_display_mem_binary_longitud);
                                if ((zoc_get_streaming_display_mem_binary[0] & 1)==0) {

                                    //Ha venido una pantalla entera, bien porque se ha pedido asi o bien porque
                                    //se ha pedido una diferencial y era demasiado grande y se genera una entera
                                    //Por tanto reiniciar el conteo
                                    zoc_slave_differential_displays_counter=0;

                                    if (slot==0) {
                                        /*printf("------Pedida diferencial pero era pantalla entera------- %d %02X %02X %02X\n",
                                            zoc_get_streaming_display_mem_binary_longitud,
                                            zoc_get_streaming_display_mem_binary[0],
                                            zoc_get_streaming_display_mem_binary[1],
                                            zoc_get_streaming_display_mem_binary[2]
                                        );*/
                                    }
                                }
                            }
                        }
                    }

                }

                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_AUDIO) {
                    // Leo el audio desde otro thread aparte

                }

                //En modo streaming obtendremos snapshot al hacer leave room
                //Nota: hay la posibilidad de que si se acaba de crear la sala en modo streaming, aun no haya entrado
                //un snapshot. En este caso la peticion aqui fallaria aunque no se alerta al usuario
                //Se recibiria un error de ZRCP "ERROR. There is no snapshot on this room"

                if (zoc_slave_force_get_snapshot) {
                    zoc_slave_force_get_snapshot=0;
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: Streaming mode. Getting snapshot to use on leave room");
                    //printf("ZENG Online Client: Streaming mode. Getting snapshot to use on leave room\n");

                    zoc_pending_apply_received_snapshot=0;
                    slave_thread_get_snapshot(indice_socket);
                }

            }

            else {

                slave_thread_get_snapshot(indice_socket);


                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_KEYS) {
                    //recepcion teclas
                    //TODO gestionar error
                    //Tecnicamente no haria falta esto, pues las teclas ya nos llegan
                    //por el volcado de puertos que tiene el snapshot
                    //PERO esto permite que lleguen mas rapido antes que el snapshot
                    zoc_get_keys(indice_socket_get_keys);
                }


            }

            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS) {
                //Enviar teclas
                //TODO gestionar error_desconectar
                int enviada_alguna_tecla;
                //int error_desconectar=
                zoc_keys_send_pending(indice_socket,&enviada_alguna_tecla);


            }


            /*

            Estos comandos los aparto al secondary commands thread

            zoc_common_get_messages_slave_master(indice_socket);

            zoc_common_alive_user(indice_socket);


            if (zoc_check_if_kicked(indice_socket)) {
                //Logicamente el usuario puede intentar entrar de nuevo pero
                //mientras no se haga kick de otro usuario diferente, este primero quedara en la lista de kick
                //y siempre se le echara
                //Ademas como va relacionado por el uuid, aunque cambie su nick, el kick seguirá siendo efectivo
                return NULL;
            }

            */



        }

        //TODO este parametro configurable
        usleep(1000); //1 ms (20 ms es un frame de video)

        //printf("en slave thread %d\n",contador_segundo_infinito);

        pthread_testcancel();

    }


    //finalizar conexion
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_slave_thread_function");
    z_sock_close_connection(indice_socket);

    if (!created_room_streaming_mode) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_slave_thread_function - indice_socket_get_keys");
        z_sock_close_connection(indice_socket_get_keys);
    }

	return 0;

}


//Decirle al slave thread que tiene que traerse un snapshot, en modo streaming, la hacer leave room
//Nota: se podria hacer casi todo desde aqui pero no tenemos el numero de socket con el que está conectado,
//aunque se podria abrir conexión nueva pero mejor hacerlo asi y reaprovechar el socket ya abierto
void zoc_slave_set_force_get_snapshot(void)
{
    if (created_room_streaming_mode) {
        if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_DISPLAY) {
            zoc_slave_force_get_snapshot=1;
            return;
        }
    }

    //No se cumple nada de eso. No esperar
    zeng_online_client_get_snapshot_applied_finished=1;
}

//Apartar comandos como get-message-id y otros que envian y reciben y si hay algo de latencia, molestarian
//en los comandos de recibir streaming de video (ralentizarian la recepcion de video)
void *zoc_slave_thread_function_secondary_commands(void *nada GCC_UNUSED)
{

    //conectar a remoto
    //Inicializar timeout para no recibir tempranos mensajes de "OFFLINE"
    zoc_last_snapshot_received_counter=ZOC_TIMEOUT_NO_SNAPSHOT;



    char server[NETWORK_MAX_URL+1];
    int puerto;
    puerto=zeng_online_get_server_and_port(server);

    int indice_socket=z_sock_open_connection(server,puerto,0,"");

    if (indice_socket<0) {
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
        return 0;
    }

    //Leer id de mensaje de broadcast para saber cuando llega uno nuevo
    zoc_last_message_id=zoc_get_message_id(indice_socket);


    //printf("Initial message id: %d\n",zoc_last_message_id);


    while (!zoc_exit_from_room) {


        zoc_common_get_messages_slave_master(indice_socket);

        zoc_common_alive_user(indice_socket);


        //hacer que salgan todos los threads si hay kick de usuario
        if (zoc_check_if_kicked(indice_socket)) {
            //Logicamente el usuario puede intentar entrar de nuevo pero
            //mientras no se haga kick de otro usuario diferente, este primero quedara en la lista de kick
            //y siempre se le echara
            //Ademas como va relacionado por el uuid, aunque cambie su nick, el kick seguirá siendo efectivo
            zoc_exit_from_room=1;
        }



        //TODO este parametro configurable
        sleep(1);

        //printf("en slave thread secondary commands %d\n",contador_segundo_infinito);

        pthread_testcancel();

    }

    //finalizar conexion
    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_slave_thread_function_secondary_commands");
    z_sock_close_connection(indice_socket);

	return 0;

}




void *zoc_slave_thread_function_stream_audio(void *nada GCC_UNUSED)
{

    int indice_socket_get_stream_audio;

    if (created_room_streaming_mode) {
        indice_socket_get_stream_audio=zoc_start_connection_get_stream_audio();
    }
    //TODO: gestionar errores


    while (!zoc_exit_from_room) {


        if (created_room_streaming_mode) {


            if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_AUDIO) {



                //Metodo sin stream continuo
                //zoc_receive_streaming_audio(indice_socket);

                //Metodo con stream continuo
                zoc_get_stream_audio_continuous(indice_socket_get_stream_audio);


            }

        }


        //TODO este parametro configurable
        usleep(10); //0.01 ms (20 ms es un frame de video)

        //printf("en slave thread stream audio %d\n",contador_segundo_infinito);

        pthread_testcancel();

    }

    if (created_room_streaming_mode) {
        //finalizar conexion
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Closing socket on zoc_slave_thread_function_stream_audio");
        z_sock_close_connection(indice_socket_get_stream_audio);
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Error connecting to %s:%d. %s",
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
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: ERROR. Can't read remote prompt: %s",z_sock_get_error(leidos));
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


//Conteo de veces llamado a  zeng_online_client_end_frame_from_core_functions, y por tanto frames de video
//Usado para obtener FPS en modo streaming
int zoc_client_video_frames=0;

//Conteo de cuantas pantallas recibidas, se usa para obtener el FPS
int zoc_client_streaming_display_count=0;

int zoc_client_streaming_display_fps=0;

//Usado para obtener la media de fps en un intervalo dado
int zoc_client_streaming_display_fps_seconds=0;
int zoc_client_streaming_display_fps_sum=0;
int zoc_client_streaming_display_fps_last_interval=0;

z80_byte *zoc_get_base_mem_pantalla(void)
{
    if (!MACHINE_IS_SPECTRUM) return NULL;

    return get_base_mem_pantalla();
}

//Decir si se bloquea temporalmente la aplicacion de streaming de video
int zoc_slave_forbid_apply_streaming_video=0;

void zeng_online_client_apply_pending_received_streaming_display(void)
{
    if (zeng_online_connected.v==0) return;

    if (zeng_online_i_am_master.v) return;

    if (!zoc_pending_apply_received_streaming_display) {
        //printf("NO apply pending %d\n",contador_segundo);
        return;
    }

    if (zoc_slave_forbid_apply_streaming_video) return;

    //printf("Apply pending %d\n",contador_segundo);

    //load_zsf_streaming_display_file_mem(NULL,zoc_get_streaming_display_mem_binary,zoc_get_streaming_display_mem_binary_longitud,1,1);
    //Aplicarlo a la pantalla
    //Formato: byte 0: flags. bit 0: si pantalla diferencial
    //byte 1: color border
    //byte 2-: datos

    z80_byte current_border_color=out_254 & 7;

    z80_byte streaming_border_color=zoc_get_streaming_display_mem_binary[1] & 7;

    if (current_border_color != streaming_border_color) {
        out_254 &=(255-7);
        out_254 |=streaming_border_color;
        modificado_border.v=1;
        //printf("Modificado border\n");
    }
    z80_byte *screen=zoc_get_base_mem_pantalla();

    if (screen==NULL) {
        //Cliente no es Spectrum. No hacemos nada
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Can not use streaming display on non Spectrum machine");
        return;
    }

    //Pantalla diferencial
    if (zoc_get_streaming_display_mem_binary[0] & 1) {
        //printf("Aplicar pantalla diferencial\n");
        int longitud_pantalla_diferencial=zoc_get_streaming_display_mem_binary_longitud-2;

        longitud_pantalla_diferencial /=3; //Cada entrada son 3 bytes

        //printf("zoc_get_streaming_display_mem_binary_longitud %d\n",zoc_get_streaming_display_mem_binary_longitud);


        //if (longitud_pantalla_diferencial==0) printf("no cambios\n");

        z80_byte *mem_pantalla_diferencial=zoc_get_streaming_display_mem_binary+2;

        int i;
        int offset_pantalla_diferencial=0;
        for (i=0;i<longitud_pantalla_diferencial;i++) {
            z80_int offset_byte=mem_pantalla_diferencial[offset_pantalla_diferencial]+256*mem_pantalla_diferencial[offset_pantalla_diferencial+1];
            z80_byte valor_a_escribir=mem_pantalla_diferencial[offset_pantalla_diferencial+2];
            //printf("Direccion: %d valor: %d\n",offset_byte+16384,valor_a_escribir);
            screen[offset_byte]=valor_a_escribir;
            offset_pantalla_diferencial +=3;
        }

    }

    else {
        //printf("Aplicar pantalla entera\n");

        memcpy(screen,&zoc_get_streaming_display_mem_binary[2],ZOC_STREAM_DISPLAY_SIZE);
    }


    free(zoc_get_streaming_display_mem_binary);
    zoc_get_streaming_display_mem_binary=NULL;


    zoc_pending_apply_received_streaming_display=0;

    zoc_client_streaming_display_count++;

    //Si estaba offline, reactualizamos
    /*if (zoc_last_streaming_display_received_counter==0) {
        zoc_show_bottom_line_footer_connected(); //Para actualizar la linea de abajo del todo con texto ZEsarUX version bla bla - ONLINE
        generic_footertext_print_operating("ONLINE");
    }*/

    //5 segundos de timeout, para aceptar teclas slave si no hay streaming_display
    //zoc_last_streaming_display_received_counter=ZOC_TIMEOUT_NO_streaming_display;

    //zeng_online_client_reset_scanline_counter();

}



//Inicio del thread de master
void zoc_start_master_thread(void)
{
    zoc_exit_from_room=0;

	if (pthread_create( &thread_zoc_master_thread, NULL, &zoc_master_thread_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online master pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_master_thread);

	if (pthread_create( &thread_zoc_master_thread_secondary_commands, NULL, &zoc_master_thread_function_secondary_commands, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online secondary commands master pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_master_thread_secondary_commands);

	if (pthread_create( &thread_zoc_master_thread_stream_audio, NULL, &zoc_master_thread_function_stream_audio, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online streaming audio master pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_master_thread_stream_audio);
}

//Inicio del thread de slave
void zoc_start_slave_thread(void)
{

    zoc_exit_from_room=0;

	if (pthread_create( &thread_zoc_slave_thread, NULL, &zoc_slave_thread_function, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online slave pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_slave_thread);

    //Slave thread secondary commands
	if (pthread_create( &thread_zoc_slave_thread_secondary_commands, NULL, &zoc_slave_thread_function_secondary_commands, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online secondary commands slave pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_slave_thread_secondary_commands);


    //Slave thread streaming audio
	if (pthread_create( &thread_zoc_slave_thread_stream_audio, NULL, &zoc_slave_thread_function_stream_audio, NULL) ) {
		DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_ERR,"ZENG Online Client: Can not create zeng online streaming audio slave pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zoc_slave_thread_stream_audio);
}









void zeng_online_client_prepare_snapshot_if_needed(void)
{

	if (zeng_online_connected.v==0) return;


		zoc_contador_envio_snapshot++;
		//printf ("%d %d\n",contador_envio_snapshot,(contador_envio_snapshot % (50*zeng_segundos_cada_snapshot) ));


		if (zoc_contador_envio_snapshot>=zoc_frames_video_cada_snapshot && !zoc_rejoining_as_master) {
            zoc_contador_envio_snapshot=0;
            //Si esta el anterior snapshot aun pendiente de enviar
            if (zoc_pending_send_snapshot) {
                zoc_snapshots_not_sent++;
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Last snapshot has not been sent yet. Total unsent: %d",zoc_snapshots_not_sent);

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
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: ZENG: Forcing reconnect");
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




                //printf("Asignada: %d liberada: %d\n",temp_memoria_asignada,temp_memoria_liberada);
                //printf("Created snapshot original size %d compressed size %d\n",longitud_sin_comprimir,longitud);

                if (zoc_send_snapshot_mem_hexa==NULL) zoc_send_snapshot_mem_hexa=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2); //16 MB es mas que suficiente



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

                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Queuing snapshot to send, length: %d bytes",longitud);


            }
		}

}


z80_byte *zoc_last_streaming_display=NULL;

z80_byte *zoc_differential_display=NULL;

//Generar pantalla diferencial sobre el frame anterior
//Formato de cada posicion:
//16 bits: offset en pantalla (ejemplo si direccion es 16385, el offset es 1)
//8 bit: valor
//Si pantalla excede de (6912/3)=2304 bytes, se retornara -1 indicando que se generara pantala entera, sin diferenciales
//Si no excede, retorna tamaño pantalla diferencial
int zoc_generate_differential_display(z80_byte *current_screen)
{
    //Si no hay anterior pantalla inicial, retornar 0 para que genere pantalla entera
    if (zoc_last_streaming_display==NULL) {
        zoc_last_streaming_display=util_malloc(ZOC_STREAM_DISPLAY_SIZE,"Can not allocate memory for streaming display");
        return 0;
    }

    if (zoc_differential_display==NULL) {
        zoc_differential_display=util_malloc(ZOC_STREAM_DISPLAY_SIZE,"Can not allocate memory for differential screen");
    }



    int bytes_diferentes=0;

    int i;
    int excede_limite=0;


    for (i=0;i<ZOC_STREAM_DISPLAY_SIZE && !excede_limite;i++) {
        z80_byte readed_byte=current_screen[i];
        if (zoc_last_streaming_display[i]!=readed_byte) {

            if (bytes_diferentes>=ZOC_STREAM_DISPLAY_SIZE/3) {
                //printf("Excede limite\n");
                excede_limite=1;
            }

            else {
                int offset_destino=bytes_diferentes*3;
                zoc_differential_display[offset_destino++]=value_16_to_8l(i);
                zoc_differential_display[offset_destino++]=value_16_to_8h(i);
                zoc_differential_display[offset_destino]=readed_byte;

                bytes_diferentes++;
            }

        }
    }

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Differential display. Different bytes from previous display: %d",bytes_diferentes);



    //Diferencial demasiado grande
    if (excede_limite) return -1;

    else return bytes_diferentes*3;

}

int zoc_generated_differential_displays_counter=0;

int zoc_get_streaming_display(z80_byte *buffer_temp_sin_comprimir,int force_full_display)
{

    int longitud_sin_comprimir;

    z80_byte *screen=zoc_get_base_mem_pantalla();

    if (screen==NULL) {
        //Master no es Spectrum
        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"Can not generate streaming display on non Spectrum machine");
        //Generamos un diferencial sin nada
        buffer_temp_sin_comprimir[0]=1;
        buffer_temp_sin_comprimir[1]=0;
        return 2;
    }


    int longitud_pantalla_diferencial=-1;

    if (!force_full_display) {
        longitud_pantalla_diferencial=zoc_generate_differential_display(screen);
    }

    //Flags byte 0:
    //bit 0: pantalla diferencial o no

    if (longitud_pantalla_diferencial>=0) {

        //printf("Generating differential display\n");

        buffer_temp_sin_comprimir[0]=1;
        longitud_sin_comprimir=longitud_pantalla_diferencial;
        memcpy(&buffer_temp_sin_comprimir[2],zoc_differential_display,longitud_sin_comprimir);
        zoc_generated_differential_displays_counter++;
    }

    else {

        //Generar pantalla entera, ya sea porque se ha pedido asi o bien porque la diferencial ocupa demasiado

        if (!force_full_display) {
            //Generar pantalla entera porque la diferencial ocupa demasiado
            //printf("Generating full display because differential was big\n");
        }

        longitud_sin_comprimir=ZOC_STREAM_DISPLAY_SIZE;
        buffer_temp_sin_comprimir[0]=0;

        memcpy(&buffer_temp_sin_comprimir[2],screen,ZOC_STREAM_DISPLAY_SIZE);

    }

    //byte 1: color border
    buffer_temp_sin_comprimir[1]=out_254 & 7;

    longitud_sin_comprimir +=2;

    //Almacenar la ultima pantalla para comparar luego la diferencial
    memcpy(zoc_last_streaming_display,screen,ZOC_STREAM_DISPLAY_SIZE);

    return longitud_sin_comprimir;

}

void zoc_prepare_streaming_display(int force_full_display,int slot)
{

    //zona de memoria donde se guarda el streaming_display pero sin pasar a hexa. y sin comprimir zip
    z80_byte *buffer_temp_sin_comprimir;
    buffer_temp_sin_comprimir=malloc(ZRCP_GET_PUT_STREAMING_DISPLAY_MEM);

    if (buffer_temp_sin_comprimir==NULL) cpu_panic("Can not allocate memory for sending streaming_display");


    int longitud_sin_comprimir=zoc_get_streaming_display(buffer_temp_sin_comprimir,force_full_display);


    if (zoc_send_streaming_display_mem_hexa[slot]==NULL) {
        zoc_send_streaming_display_mem_hexa[slot]=util_malloc(ZRCP_GET_PUT_STREAMING_DISPLAY_MEM*2,"Can not allocate memory for streaming display");
    }


    int i;

    z80_byte *origen=buffer_temp_sin_comprimir;
    char *destino=zoc_send_streaming_display_mem_hexa[slot];
    z80_byte byte_leido;

    for (i=0;i<longitud_sin_comprimir;i++) {
        byte_leido=*origen++;
        *destino++=util_byte_to_hex_nibble(byte_leido>>4);
        *destino++=util_byte_to_hex_nibble(byte_leido);
    }


    //metemos salto de linea y 0 al final
    *destino++ ='\n';
    *destino++ =0;



    free(buffer_temp_sin_comprimir);

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_PARANOID,"ZENG Online Client: Queuing streaming_display to send, length: %d bytes",longitud_sin_comprimir);


}

void zeng_online_client_prepare_streaming_display_if_needed(void)
{

	if (zeng_online_connected.v==0) return;




    if (!zoc_rejoining_as_master) {

        //Si esta el anterior streaming_display aun pendiente de enviar
        if (zoc_pending_send_streaming_display) {


        }
        else {


                //Slot 0 diferencial
                zoc_prepare_streaming_display(0,0);

                //Slot 1 completo
                zoc_prepare_streaming_display(1,1);


                zoc_pending_send_streaming_display=1;

        }

	}
}

//Modo en el que estamos
//0: inicial o sin cambios
//1: hemos incrementado diferenciales para mejorar fps
//2: hemos decrementado diferenciales para mejorar calidad.
int autoajuste_diferenciales_estado=0;


int ultimo_try_increment_diferenciales_contador_segundo=0;

void zec_increment_differential_display_parameter(void)
{
    //0,2,5,10,15,20...
    if (zoc_slave_differential_displays_limit_full==0) zoc_slave_differential_displays_limit_full=2;
    else if (zoc_slave_differential_displays_limit_full==2) zoc_slave_differential_displays_limit_full=5;
    else zoc_slave_differential_displays_limit_full +=5;

    if (zoc_slave_differential_displays_limit_full>=50) zoc_slave_differential_displays_limit_full=50;

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. New value: %d",zoc_slave_differential_displays_limit_full);

}

void zec_decrement_differential_display_parameter(void)
{
    //0,2,5,10,15,20...
    if (zoc_slave_differential_displays_limit_full==5) zoc_slave_differential_displays_limit_full=2;
    else if (zoc_slave_differential_displays_limit_full==2) zoc_slave_differential_displays_limit_full=0;
    else zoc_slave_differential_displays_limit_full -=5;

    if (zoc_slave_differential_displays_limit_full<=0) zoc_slave_differential_displays_limit_full=0;

    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. New value: %d",zoc_slave_differential_displays_limit_full);
}

#define ZEC_AUTOADJUST_MARGIN_COMPARISON 3

//Dice si el primer valor es menor que el segundo, pero por un margen definido
int zec_autoadjust_differentials_less(int a,int b)
{
    //Si A=10, B=12, margen=3: no es menor
    //Si A=9, B=12, margen=3: es menor
    //Si A=8, B=12, margen=3: es menor
    if (b-a>=ZEC_AUTOADJUST_MARGIN_COMPARISON) return 1;
    else return 0;
}

//Dice si el primer valor es mayor que el segundo, pero por un margen definido
int zec_autoadjust_differentials_greater(int a,int b)
{
    //Si A=12, B=10, margen=3: no es mayor
    //Si A=12, B=9, margen=3: es mayor
    //Si A=12, B=8, margen=3: es mayor
    if (a-b>=ZEC_AUTOADJUST_MARGIN_COMPARISON) return 1;
    else return 0;
}

//Dice si el primer valor es igual que el segundo, pero por un margen definido
int zec_autoadjust_differentials_equal(int a,int b)
{
    //Si A=12, B=10, margen=3: es igual
    //Si A=10, B=12, margen=3: es igual
    //Si A=12, B=9, margen=3: no es igual
    //Si A=9, B=12, margen=3: no es igual
    //Si A=12, B=8, margen=3: no es igual
    if (util_abs(a-b)<ZEC_AUTOADJUST_MARGIN_COMPARISON) return 1;
    else return 0;
}

void zec_autoadjust_differentials_display(void)
{
    zoc_slave_differential_displays_limit_full_autoadjust_seconds_counter++;

    if (zoc_slave_differential_displays_limit_full_autoadjust_seconds_counter>=zoc_slave_differential_displays_limit_full_autoadjust_seconds_interval) {

        zoc_slave_differential_displays_limit_full_autoadjust_seconds_counter=0;

        //printf("Autoadjusting differentials. Previous FPS: %d Last FPS: %d Differentials parameter: %d\n",
        //    zoc_slave_differential_displays_limit_full_previous_fps,zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full);


        //Reseteamos intervalo
        zoc_client_streaming_display_fps_seconds=0;
        zoc_client_streaming_display_fps_sum=0;


        //Recalcular parametro segun algoritmo
        /*
        En momento X cualquiera:
        -calcular puntuación media durante 1 minuto
        -calcular puntuación media durante 1 minuto

        Si tenemos 50 fps de media y 0 diferenciales, no hacer nada

        Si fps menor que antes, aumentar diferenciales (si diferenciales no es 50). Buscamos aumentar fps a costa de reducir calidad


        Si fps mayor o igual que antes, reducir diferenciales (si diferenciales no es 0). Aumentamos calidad a ver si Fps no se altera mucho

        Calcular también puntuación media del último minuto o 5 minutos y actuar igual

        Los aumentos / decrementos de incrementales quizás van de 5 en 5, no de 1 en 1 que se notaría poco

        */

        //Evitar valor inicial
        if (zoc_slave_differential_displays_limit_full_previous_fps>=0) {

            //Si 50 fps y diferenciales 0
            if (zoc_client_streaming_display_fps_last_interval==50 && zoc_slave_differential_displays_limit_full==0) {
                //printf("50 FPS and 0 differentials. Ideal state. Do nothing\n");
            }
            else {

                //Si en estado inicial o de no tocar nada
                if (autoajuste_diferenciales_estado==0) {
                    //printf("Estado: 0 inicial o estable\n");

                    if (zec_autoadjust_differentials_less(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: stable. "
                            "FPS less than before (now: %d before: %d). Increment differentials",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);
                        zec_increment_differential_display_parameter();

                        autoajuste_diferenciales_estado=1;

                    }

                    if (zec_autoadjust_differentials_greater(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: stable. "
                            "Bigger FPS than before (now: %d before: %d). Reduce differentials",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);
                        zec_decrement_differential_display_parameter();

                        autoajuste_diferenciales_estado=2;
                    }

                    //Si no ha variado
                    if (zec_autoadjust_differentials_equal(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {

                        //Si no ha variado, probamos a bajar si es que esta en un valor relativamente alto
                        //Esto antes , mas prioritario que subir incrementales
                        if (zoc_slave_differential_displays_limit_full>0) {
                            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: stable. "
                                "Same or almost same FPS than before (now: %d before: %d). Try reducing differentials",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);
                            zec_decrement_differential_display_parameter();


                            autoajuste_diferenciales_estado=2;
                        }

                        //probamos a subir si es que no esta en un valor relativamente alto
                        else if (zoc_slave_differential_displays_limit_full<5) {
                            //de estos solo hacer 1 cada 5 minuto o asi
                            if (contador_segundo_infinito-ultimo_try_increment_diferenciales_contador_segundo>=5*60*1000) {

                                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: stable. "
                                    "Same or almost same FPS than before (now: %d before: %d). Try increment differentials (this not too often, just to check if it improves)",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);
                                zec_increment_differential_display_parameter();

                                autoajuste_diferenciales_estado=1;
                                ultimo_try_increment_diferenciales_contador_segundo=contador_segundo_infinito;
                            }

                            else {
                                //printf("--Ibamos a hacer un try increment diferenciales pero no ha pasado mas de 1 minuto del anterior: %d\n",
                                //    contador_segundo_infinito-ultimo_try_increment_diferenciales_contador_segundo);
                            }
                        }


                    }

                }

                //Si en estado de que hemos incrementado
                else if (autoajuste_diferenciales_estado==1) {
                    //printf("Estado: 1 de incrementado diferenciales\n");
                    //Si empeora fps, reducir y volvemos a estado 0.
                    if (zec_autoadjust_differentials_less(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: incrementing. "
                            "FPS less than before (now: %d before: %d). Decrement differentials",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);
                        zec_decrement_differential_display_parameter();

                        autoajuste_diferenciales_estado=0;
                    }

                    //Si mejora FPS, aumentar diferenciales
                    if (zec_autoadjust_differentials_greater(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: incrementing. "
                            "Bigger FPS than before (now: %d before: %d). Increment differentials",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);

                        zec_increment_differential_display_parameter();

                        if (zoc_slave_differential_displays_limit_full>=50) {

                            //Estado estable porque hemos llegado al limite
                            autoajuste_diferenciales_estado=0;
                        }

                        else {
                            autoajuste_diferenciales_estado=1;
                        }
                    }

                    //Si mismos FPS, reducir incrementales
                    if (zec_autoadjust_differentials_equal(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: incrementing. "
                            "Same or almost same FPS than before (now: %d before: %d). Reduce differentials and going to stable state",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);
                        autoajuste_diferenciales_estado=0;

                        zec_decrement_differential_display_parameter();
                    }
                }

                //Si en estado de que hemos decrementado. Estamos buscando mantener fps o mejorarlos
                else if (autoajuste_diferenciales_estado==2) {
                    //printf("Estado: 2 de decrementado diferenciales\n");
                    //Si empeora fps, aumentar incrementales y volvemos a estado 0.
                    if (zec_autoadjust_differentials_less(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: decrementing. "
                            "FPS less than before (now: %d before: %d). Increment differentials",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);

                        zec_increment_differential_display_parameter();

                        autoajuste_diferenciales_estado=0;

                    }

                    //Si mejora FPS, disminuir diferenciales
                    if (zec_autoadjust_differentials_greater(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: decrementing. "
                            "Bigger FPS than before (now: %d before: %d). Decrement differentials",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);

                        zec_decrement_differential_display_parameter();
                        if (zoc_slave_differential_displays_limit_full<=0) {
                            //Estado estable porque hemos llegado al limite
                            autoajuste_diferenciales_estado=0;
                        }
                        else {
                            autoajuste_diferenciales_estado=2;
                        }
                    }

                    //Mismos FPS y estabamos reduciendo. No reducir diferenciales
                    if (zec_autoadjust_differentials_equal(zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps)) {
                        DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: Autoadjust differentials. State: decrementing. "
                            "Same or almost same FPS than before (now: %d before: %d). Going to stable state",zoc_client_streaming_display_fps_last_interval,zoc_slave_differential_displays_limit_full_previous_fps);
                        autoajuste_diferenciales_estado=0;
                    }
                }

                //printf("Estado despues de ajustes: %d\n",autoajuste_diferenciales_estado);

            }
        }

        //printf("After algorithm autoadjust differentials. Differentials parameter: %d\n",
        //    zoc_slave_differential_displays_limit_full);



        zoc_slave_differential_displays_limit_full_previous_fps=zoc_client_streaming_display_fps_last_interval;

    }

}

//Calcular los fps del ultimo segundo, la media de los ultimos segundos, y autoajustar parametro de pantallas diferenciales
void zoc_client_manage_fps_diferential_display(void)
{

    zoc_client_video_frames++;
    if (zoc_client_video_frames==50) {

        zoc_client_video_frames=0;

        zoc_client_streaming_display_fps=zoc_client_streaming_display_count;
        zoc_client_streaming_display_count=0;

        //printf("1 segundo. FPS=%d\n",zoc_client_streaming_display_fps);


        zoc_client_streaming_display_fps_sum +=zoc_client_streaming_display_fps;


        zoc_client_streaming_display_fps_seconds++;

        //Por si acaso, aunque incrementamos siempre antes de dividir, podria ser que el contador hiciera overflow
        //y llegase a ser 0

        if (zoc_client_streaming_display_fps_seconds==0) {
            zoc_client_streaming_display_fps_last_interval=0;
        }

        else {
            zoc_client_streaming_display_fps_last_interval=zoc_client_streaming_display_fps_sum/zoc_client_streaming_display_fps_seconds;
        }

        //printf("FPS average in the last %d seconds: %d\n",zoc_client_streaming_display_fps_seconds,zoc_client_streaming_display_fps_last_interval);

        //Nota: este algoritmo de autoajuste del parametro de diferenciales es muy mejorable, funciona pero "de aquella manera"
        if (zoc_slave_differential_displays_limit_full_autoadjust.v) {
            zec_autoadjust_differentials_display();
        }


    }
}

int zoc_snapshot_master_streaming_counter=0;

void zeng_online_client_end_frame_from_core_functions(void)
{

    if (zeng_online_connected.v==0) return;

    zeng_online_client_tell_end_frame();

    //El orden en el caso de rejoin as master es importante,
    //primero aplicaremos el snapshot recibido al entrar como master,
    //luego ya iremos enviando los siguientes como master normal

    if (created_room_streaming_mode) {
        zeng_online_client_apply_pending_received_streaming_display();

        if (zeng_online_i_am_master.v==0) {
            zoc_client_manage_fps_diferential_display();
        }
    }
    else {
        zeng_online_client_apply_pending_received_snapshot();
    }


    //Envio pantalla y snapshot de master
    if (zeng_online_i_am_master.v) {
        if (created_room_streaming_mode) {

            zeng_online_client_prepare_streaming_display_if_needed();

            //En modo streaming enviamos 1 snapshot cada minuto

            zoc_snapshot_master_streaming_counter++;
            if (zoc_snapshot_master_streaming_counter>=ZOC_INTERVAL_SNAPSHOT_ON_STREAMING) {
                zoc_snapshot_master_streaming_counter=0;
                //printf("Preparing snapshot on streaming mode\n");
                DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: Streaming mode. Preparing snapshot to use on leave room on the slave");

                zeng_online_client_prepare_snapshot_if_needed();
                //printf("zoc_pending_send_snapshot: %d\n",zoc_pending_send_snapshot);
            }

        }

        else {

            zeng_online_client_prepare_snapshot_if_needed();

        }
    }



    if (zeng_online_i_am_master.v==0) {
        if (zoc_last_snapshot_received_counter>0) {

            //En modo streaming no haremos saltar el aviso OFFLINE
            if (!created_room_streaming_mode) {

                zoc_last_snapshot_received_counter--;

                if (zoc_last_snapshot_received_counter==0) {
                    DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_INFO,"ZENG Online Client: Timeout receiving snapshots from master. Allowing local key press");
                    zoc_show_bottom_line_footer_connected(); //Para actualizar la linea de abajo del todo con texto ZEsarUX version bla bla - OFFLINE
                    generic_footertext_print_operating("OFFLIN");
                }

            }

        }
    }

}

void zeng_online_client_alter_fps_streaming(void)
{
    if (zeng_online_connected.v && zeng_online_i_am_master.v==0 && created_room_streaming_mode) {
        //Nota: estos FPS son de recepcion, no es un valor completamente real siempre. Por ejemplo:
        //Si master envia 50 FPS y slave recibe 50 FPS, ok, mostrara 50 FPS
        //Si master no tiene mucho ancho de banda y envia a 22 FPS, el slave puede recibir 50 FPS y mostrara 50 FPS
        //Esto es porque el slave no tiene manera de saber si el frame recibido es el mismo que habia antes o no
        //Pero bueno, se tomara este FPS mostrado como una medida de velocidad del slave:
        //Si dice 50 FPS, consideremos que el slave está bien (aunque el master podria estar mal)
        //Si dice menos de 50 FPS, al menos el slave está mal (y el master podria estar mal también)
        ultimo_fps=zoc_client_streaming_display_fps;
    }
}


char zoc_last_audio_value_received=0;

void zeng_online_client_end_audio_frame_stuff(void)
{
    if (zeng_online_connected.v && zeng_online_i_am_master.v && created_room_streaming_mode) {
        //Tareas del master. Poner buffer audio para enviar

        if (!zoc_master_pending_send_streaming_audio) {

            //Ver si el buffer tiene sonido o está en silencio. Comparamos si el valor es el mismo
            int diferente=0;
            int i;
            char valor_inicial=audio_buffer[0];

            int longitud_enviar_audio=ZOC_STREAMING_AUDIO_BUFFER_SIZE;

            if (streaming_silence_detection) {

                for (i=0;i<ZOC_STREAMING_AUDIO_BUFFER_SIZE;i++) {
                    //printf("%d\n",i);
                    if (valor_inicial!=audio_buffer[i]) {
                        //printf("Diferente %d\n",i);
                        diferente=1;
                        break;
                    }
                }
            }

            else {
                diferente=1;
            }

            if (!diferente) {
                //printf("Buffer audio has silence. i=%d\n",i);
                //En ese caso solo enviamos dos valores del sample (canal izquierdo y derecho)
                //que son los que usara el remoto para repetir esos valores en destino
                longitud_enviar_audio=2;
            }

            else {

                //printf("Buffer audio has NO silence. Send it. i=%d\n",i);

            }

            //else {


                z80_byte *zoc_streaming_audio_to_send=util_malloc(longitud_enviar_audio,"Can not allocate memory for stream audio");

                memcpy(zoc_streaming_audio_to_send,audio_buffer,longitud_enviar_audio);

                if (zoc_send_streaming_audio_mem_hexa==NULL) {
                    //Preparo siempre memoria para el maximo, aunque solo se envien los dos bytes identificando silencio
                    zoc_send_streaming_audio_mem_hexa=util_malloc(ZOC_STREAMING_AUDIO_BUFFER_SIZE*2+100,"Can not allocate memory for streaming audio");
                }



                int longitud=longitud_enviar_audio;

                z80_byte *origen=zoc_streaming_audio_to_send;
                char *destino=zoc_send_streaming_audio_mem_hexa;
                z80_byte byte_leido;

                for (i=0;i<longitud;i++) {
                    byte_leido=*origen++;
                    *destino++=util_byte_to_hex_nibble(byte_leido>>4);
                    *destino++=util_byte_to_hex_nibble(byte_leido);
                }


                //metemos salto de linea y 0 al final
                *destino++ ='\n';
                *destino++ =0;



                free(zoc_streaming_audio_to_send);

                zoc_master_pending_send_streaming_audio=1;

            //}
        }

    }

    if (zeng_online_connected.v && zeng_online_i_am_master.v==0 && created_room_streaming_mode) {
        //Tareas del slave. Aplicar audio que esta pendiente

        reset_beeper_silence_detection_counter();

        if (zoc_pending_apply_received_streaming_audio) {
            //printf("End audio frame. Sonido disponible.        %d\n",contador_segundo);

            memcpy(audio_buffer,zoc_get_audio_mem_binary_second_buffer,ZOC_STREAMING_AUDIO_BUFFER_SIZE);

            //Ajustar volumen de todo el frame de audio
            //Nota: el audio viene del cliente master con volumen aplicado, si no tiene 100%
            //lo normal seria que el master pusiera 100% y cada slave lo ajuste en su menu
            if (audiovolume!=100) {
                int i;
                for (i=0;i<ZOC_STREAMING_AUDIO_BUFFER_SIZE;i++) {
                    audio_buffer[i]=audio_adjust_volume(audio_buffer[i]);
                }

            }

            zoc_last_audio_value_received=audio_buffer[ZOC_STREAMING_AUDIO_BUFFER_SIZE-1];

            zoc_pending_apply_received_streaming_audio=0;

            zoc_veces_audio_no_recibido_contador=0;
        }
        else {
            //TODO: Windows alterna a cada frame de audio este perido de no available junto con el available,
            //pero solo cuando es un periodo de silencio. No se por que, quizá no se llama al timer de manera precisa
            //Quiza porque Windows no usa threads en timer sino que utiliza el reloj? Aunque si pruebo timer con threads tampoco va bien
            DBG_PRINT_ZENG_ONLINE_CLIENT VERBOSE_DEBUG,"ZENG Online Client: End audio frame. Sound not available yet");
            //Meter silencio
            //Metemos ultimo valor recibido para que no suene petardeo
            memset(audio_buffer,zoc_last_audio_value_received,ZOC_STREAMING_AUDIO_BUFFER_SIZE);

            //vemos si esto sucede a menudo
            zoc_veces_audio_no_recibido_contador++;


            //Aparte de autoajustar incrementales, vemos si han habido varios cortes de audio en este intervalo,
            //y avisar con un mensaje en el footer
            //Nota: obviamente esto requiere que el auto ajuste de incrementales esté activado
            //Podria hacer una funcion aparte para que no dependiera de eso pero no me parece tan importante este mensaje en el footer
            //printf("zoc_veces_audio_no_recibido_contador %d\n",zoc_veces_audio_no_recibido_contador);
            //Nota2: usando mi zeng server remoto (usando server local no pasa), los periodos de silencio se tardan mas en enviarse desde el master que
            //los periodos de audio normal, no acabo de entender porque. Por tanto se alterna 1 periodo de silencio con 1 de no audio, y asi seguido
            //No saltara el aviso de dropout dado que reseteo el contador cuando se recibe audio (sea o no silencio)
            if (zeng_online_show_footer_lag_indicator.v) {
                if (zoc_veces_audio_no_recibido_contador>5) {
                    generic_footertext_print_operating("DROPOUT");
                    zoc_veces_audio_no_recibido_contador=0;
                }
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

void zeng_online_client_stop_slave_thread(void)
{
}

void zeng_online_client_stop_master_thread(void)
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

void zeng_online_client_rename_room(char *name)
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

void zeng_online_client_alter_fps_streaming(void)
{
}

void zeng_online_client_end_audio_frame_stuff(void)
{
}

void zeng_online_client_apply_pending_received_snapshot(void)
{
}

#endif