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

Online Network Play (using a central server) Server related code

*/

/*

Snapshots:
-cuando se pide un snapshot desde un slave, se asigna memoria y se copia ahí el contenido del último snapshot.
Mientras se copia se incrementa un contador atómico, de tal manera que si hay dos conexiones pidiendo snapshot al mismo tiempo,
contador será 2 por ejemplo

-cuando un máster envía un slave, primero se envía a una memoria temporal. Y luego se enviará a la memoria de snapshot ,
antes esperando a que el contador atómico esté a 0. problema: puede estar a 0 pero cuando se vaya a enviar el nuevo snapshot,
puede entrar lectura de snapshot desde slave. como solventarlo?



Rooms:
Al crear una habitacion, se genera un password que es el que debe usarse siempre para hacer acciones sobre esa habitacion, como:
-establecer maximo de jugadores
-envio snapshot
-envio eventos teclado/joystick
Para obtener ese password, hay que unirse a la habitacion

Nota: el master hace:
-crear habitacion
-unirse a habitacion creada
-y en bucle: enviar snapshots

El slave hace:
-unirse a una habitacion que se selecione
-y en bucle: enviar eventos, obtener snapshots


TODO: cada nueva conexión a ZRCP ocupa 32MB, según este código en remote.c:
char *buffer_lectura_socket=malloc(MAX_LENGTH_PROTOCOL_COMMAND);

32 MB son muchos MB para cada conexión... con 100 conexiones nos comemos 3 GB. Como solventar esto?

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
#include "remote.h"
#include "snap_zsf.h"
#include "autoselectoptions.h"
#include "ay38912.h"
#include "atomic.h"
#include "stats.h"
#include "textspeech.h"
#include "settings.h"





//Variables, estructuras,funciones etc que se pueden compilar aun sin soporte de pthreads

//Numero maximo de habitaciones para esta sesion que se pueden crear
//Interesa que este valor se pueda bajar o subir (pero no subir nunca mas alla de ZENG_ONLINE_MAX_ROOMS),
//porque segun la potencia del server se puede permitir mas o menos
int zeng_online_current_max_rooms=10;


int zeng_online_enabled=0;

//Estructura de un evento: Tecla, press/release, y uuid del cliente que lo envia
struct s_zeng_online_eventkeys {
    int tecla;
    int pressrelease;
    char uuid[STATS_UUID_MAX_LENGTH];
    int nomenu; //nomenu if set to non 0, tells the key is not sent when menu is open
};

//Maximo de peticiones de join que se pueden quedar en espera
#define ZOC_MAX_JOIN_WAITING 30

//Estructura de una peticion de join en espera
struct s_zoc_join_waiting_request {
    char nickname[ZOC_MAX_NICKNAME_LENGTH+1];
    int waiting;
    int permissions; //TODO: esto hace falta tenerlo aqui o directamente ira hacia el cliente?
};

//Estructura de una habitacion de zeng online
struct zeng_online_room {
    int created;
    int max_players;
    int current_players;
    int do_not_allow_events; //TODO: no permitir envio de eventos en esta room, util para solo transmitir desde el master y que nadie interactue
    char name[ZENG_ONLINE_MAX_ROOM_NAME+1]; //+1 para el 0 del final

    char user_password[ZENG_ROOM_PASSWORD_LENGTH+1]; //+1 para el 0 del final. un password simple, para tener un minimo de seguridad
    //que no se pueden lanzar acciones sobre una habitacion sino se ha unido a dicha habitacion

    char creator_password[ZENG_ROOM_PASSWORD_LENGTH+1]; //+1 para el 0 del final. un password simple, para las operaciones
      //del que ha creado la habitacion

    z80_byte *snapshot_memory; //Donde esta asignado el snapshot
    int snapshot_size;
    int snapshot_id; //Para saber cuando cambia el snapshot

    z_atomic_semaphore mutex_reading_snapshot;
    int reading_snapshot_count;

    z_atomic_semaphore semaphore_writing_snapshot;

    //donde se almacenan los eventos. Es un array circular, si se llega al final se sobreescriben
    //No pasa nada si sobreescribimos la posicion donde un cliente esta leyendo, si se pierde el evento, se pierde...
    //Ya le llegara un snapshot que lo corrija
    //Esto es parecido a la compresión de video: se van enviando deltas de diferencias y cada cierto tiempo se envia una imagen entera
    struct s_zeng_online_eventkeys events[ZENG_ONLINE_MAX_EVENTS];
    int index_event;
    z_atomic_semaphore semaphore_events;

    //Mensajes de broadcast
    int broadcast_message_id;

    //En el mensaje se le agrega:
    //Message from room XX user XX
    //Mas que suficiente el margen de ZOC_MAX_NICKNAME_LENGTH+100
    char broadcast_message[ZENG_ONLINE_MAX_BROADCAST_MESSAGE_SHOWN_LENGTH];
    int broadcast_messages_allowed;

    //Usuarios logueados
    //Si "", indica que no hay user en esa posicion
    z_atomic_semaphore semaphore_joined_users;
    char joined_users[ZENG_ONLINE_MAX_PLAYERS_PER_ROOM][ZOC_MAX_NICKNAME_LENGTH+1];
    char joined_users_uuid[ZENG_ONLINE_MAX_PLAYERS_PER_ROOM][STATS_UUID_MAX_LENGTH+1];

    //Peticiones de join en espera
    int index_waiting_join_first; //donde empieza el primer elemento en espera
    int total_waiting_join; //total de join en espera
    struct s_zoc_join_waiting_request join_waiting_list[ZOC_MAX_JOIN_WAITING];
    int autojoin_enabled;
    int autojoin_permissions;

    //Perfiles de teclas permitidas en usuarios
    //Primer indice [] apunta al id de perfil.
    //Segundo indice [] apunta a la tecla; si vale 0, indica final de perfil. Final de perfil tambien indicado por el ultimo item al llenarse el array
    int allowed_keys[ZOC_MAX_KEYS_PROFILES][ZOC_MAX_KEYS_ITEMS];
    //Perfiles asignados a cada uuid. Si es "", no esta asignado
    char allowed_keys_assigned[ZOC_MAX_KEYS_PROFILES][STATS_UUID_MAX_LENGTH+1];

};

//Array de habitaciones en zeng online
struct zeng_online_room zeng_online_rooms_list[ZENG_ONLINE_MAX_ROOMS];

//Agregar usuario a la lista de joined_users
void zoc_add_user_to_joined_users(int room_number,char *nickname,char *uuid)
{
    //Para evitar escribir dos a la vez
	while(z_atomic_test_and_set(&zeng_online_rooms_list[room_number].semaphore_joined_users)) {
		//printf("Esperando a liberar lock en zoc_add_user_to_joined_users\n");
	}

    int i;
    int agregado=0;

    for (i=0;i<zeng_online_rooms_list[room_number].max_players;i++) {
        if (zeng_online_rooms_list[room_number].joined_users_uuid[i][0]==0) {
            strcpy(zeng_online_rooms_list[room_number].joined_users[i],nickname);
            strcpy(zeng_online_rooms_list[room_number].joined_users_uuid[i],uuid);
            agregado=1;
            break; //para salir del bucle y liberar lock
        }
    }

    zeng_online_rooms_list[room_number].current_players++;


    //Y si llega al final sin haber agregado usuario, es un error aunque no lo reportaremos
    if (!agregado) debug_printf(VERBOSE_DEBUG,"Reached maximum users on join_list names");

	//Liberar lock
	z_atomic_reset(&zeng_online_rooms_list[room_number].semaphore_joined_users);

}

//Quita usuario de la lista de joined_users
void zoc_del_user_to_joined_users(int room_number,char *uuid)
{

    //Para evitar escribir dos a la vez
	while(z_atomic_test_and_set(&zeng_online_rooms_list[room_number].semaphore_joined_users)) {
		//printf("Esperando a liberar lock en zoc_del_user_to_joined_users\n");
	}

    int i;
    int borrado=0;

    for (i=0;i<zeng_online_rooms_list[room_number].max_players;i++) {
        if (!strcmp(zeng_online_rooms_list[room_number].joined_users_uuid[i],uuid)) {
            zeng_online_rooms_list[room_number].joined_users[i][0]=0;
            zeng_online_rooms_list[room_number].joined_users_uuid[i][0]=0;
            break; //para salir del bucle y liberar lock
        }
    }

    //Aunque nunca deberia ser <0, pero por si acaso
    if (zeng_online_rooms_list[room_number].current_players>0) {
        zeng_online_rooms_list[room_number].current_players--;
    }

    //Y si llega al final sin haber encontrado usuario, es un error aunque no lo reportaremos
    if (!borrado) debug_printf(VERBOSE_DEBUG,"Can not find user with uuid %s to delete from join list",uuid);

	//Liberar lock
	z_atomic_reset(&zeng_online_rooms_list[room_number].semaphore_joined_users);

}


int join_list_return_last_element(int room_number)
{
    //TODO meter esto en bloqueo semaforo
    int indice_inicial=zeng_online_rooms_list[room_number].index_waiting_join_first;
    int indice_final=indice_inicial+zeng_online_rooms_list[room_number].total_waiting_join;

    indice_final %=ZOC_MAX_JOIN_WAITING; //dar la vuelta al contador

    return indice_final;
}

void join_delete_first_element(int room_number)
{
    //TODO meter esto en bloqueo semaforo
    int indice_inicial=zeng_online_rooms_list[room_number].index_waiting_join_first;
    indice_inicial++;

    indice_inicial %=ZOC_MAX_JOIN_WAITING; //dar la vuelta al contador

    zeng_online_rooms_list[room_number].index_waiting_join_first=indice_inicial;
    zeng_online_rooms_list[room_number].total_waiting_join--;

}

//Agregar elemento join a lista
int join_list_add_element(int room_number,char *nickname)
{
    //Si llena la lista, esperar
    while (zeng_online_rooms_list[room_number].total_waiting_join==ZOC_MAX_JOIN_WAITING) {
        printf("Waiting queue is full. let's wait\n");
        sleep(1);
    }

    //TODO meter esto en bloqueo semaforo



    //Obtener indice del siguiente
    int indice_final=join_list_return_last_element(room_number);
    zeng_online_rooms_list[room_number].join_waiting_list[indice_final].waiting=1;
    strcpy(zeng_online_rooms_list[room_number].join_waiting_list[indice_final].nickname,nickname);

    zeng_online_rooms_list[room_number].total_waiting_join++;

    //bloqueo hasta aqui

    return indice_final;



}

//Ver si el evento es valido, que no tenga restriccion de teclas para ese uuid
int zengonline_valid_event(int room_number,char *uuid,int tecla)
{
    /*
    //Perfiles de teclas permitidas en usuarios
    //Primer indice [] apunta al id de perfil.
    //Segundo indice [] apunta a la tecla; si vale 0, indica final de perfil. Final de perfil tambien indicado por el ultimo item al llenarse el array
    int allowed_keys[ZOC_MAX_KEYS_PROFILES][ZOC_MAX_KEYS_ITEMS];
    //Perfiles asignados a cada uuid. Si es "", no esta asignado
    char allowed_keys_assigned[ZOC_MAX_KEYS_PROFILES][STATS_UUID_MAX_LENGTH+1];
    */

    int i;

    for (i=0;i<ZOC_MAX_KEYS_PROFILES;i++) {
        //buscar primero el que corresponde al uuid. Solo valido 1 perfil máximo por uuid
        if (!strcmp(uuid,zeng_online_rooms_list[room_number].allowed_keys_assigned[i])) {
            //Ver si esa tecla esta en la lista. Lista finaliza con el ultimo item o cuando item es 0
            int j;
            for (j=0;j<ZOC_MAX_KEYS_ITEMS && zeng_online_rooms_list[room_number].allowed_keys[i][j];j++) {
                if (zeng_online_rooms_list[room_number].allowed_keys[i][j]==tecla) {
                    printf("Tecla %d es valida para el uuid %s\n",tecla,uuid);
                    return 1;
                }
            }

            //no tecla valida
            printf("Tecla %d NO es valida para el uuid %s\n",tecla,uuid);
            return 0;
        }
    }

    printf("No hay restricción de tecla para el uuid %s\n",uuid);
    return 1;
}

//Agregar evento de tecla/joystick
void zengonline_add_event(int room_number,char *uuid,int tecla,int event_type,int nomenu)
{
    //Para evitar escribir dos a la vez
	while(z_atomic_test_and_set(&zeng_online_rooms_list[room_number].semaphore_events)) {
		//printf("Esperando a liberar lock en zengonline_add_event\n");
	}


    int index_event=zeng_online_rooms_list[room_number].index_event;
    zeng_online_rooms_list[room_number].events[index_event].tecla=tecla;
    zeng_online_rooms_list[room_number].events[index_event].pressrelease=event_type;
    zeng_online_rooms_list[room_number].events[index_event].nomenu=nomenu;
    strcpy(zeng_online_rooms_list[room_number].events[index_event].uuid,uuid);

    //Obtenemos siguiente indice
    index_event++;
    if (index_event>=ZENG_ONLINE_MAX_EVENTS) index_event=0;

    zeng_online_rooms_list[room_number].index_event=index_event;



	//Liberar lock
	z_atomic_reset(&zeng_online_rooms_list[room_number].semaphore_events);
}

//Obtiene el snapshot de una habitacion y mirando que no haya nadie escribiendo (o sea un put snapshot en curso)
//Es el problema tipico del readers-writers (aunque en mi caso solo tengo un writer)
//Quiero que puedan leer muchos simultaneamente, pero que solo pueda escribir cuando no hay nadie leyendo
//https://www.tutorialspoint.com/readers-writers-problem
int zengonline_get_snapshot(int room,z80_byte *destino)
{
    //Adquirir lock mutex
	while(z_atomic_test_and_set(&zeng_online_rooms_list[room].mutex_reading_snapshot)) {
		//printf("Esperando a liberar lock mutex_reading_snapshot en zengonline_put_snapshot\n");
	}

    //Incrementar contador de cuantos leen
    zeng_online_rooms_list[room].reading_snapshot_count++;
    if (zeng_online_rooms_list[room].reading_snapshot_count==1) {
        //Si es el primer lector, bloqueamos escritura

    	while(z_atomic_test_and_set(&zeng_online_rooms_list[room].semaphore_writing_snapshot)) {
		    //printf("Esperando a liberar lock semaphore_writing_snapshot en zengonline_get_snapshot\n");
	    }
    }

    //Liberar lock mutex
	z_atomic_reset(&zeng_online_rooms_list[room].mutex_reading_snapshot);

    int longitud_snapshot=zeng_online_rooms_list[room].snapshot_size;

    memcpy(destino,zeng_online_rooms_list[room].snapshot_memory,longitud_snapshot);

    //Adquirir lock mutex
	while(z_atomic_test_and_set(&zeng_online_rooms_list[room].mutex_reading_snapshot)) {
	    //printf("Esperando a liberar lock mutex_reading_snapshot en zengonline_put_snapshot\n");
	}


    zeng_online_rooms_list[room].reading_snapshot_count--;
    //Si somos el ultimo lector, liberar bloqueo escritura

    if (zeng_online_rooms_list[room].reading_snapshot_count==0) {
        z_atomic_reset(&zeng_online_rooms_list[room].semaphore_writing_snapshot);
    }

    //Liberar lock mutex
    z_atomic_reset(&zeng_online_rooms_list[room].mutex_reading_snapshot);

    return longitud_snapshot;


}

//Lo mueve de una memoria a la memoria del snapshot de esa habitacion
//Es el problema tipico del readers-writers (aunque en mi caso solo tengo un writer)
//Quiero que puedan leer muchos simultaneamente, pero que solo pueda escribir cuando no hay nadie leyendo
//https://www.tutorialspoint.com/readers-writers-problem
void zengonline_put_snapshot(int room,z80_byte *origen,int longitud)
{
    z80_byte *destino_snapshot;
    destino_snapshot=util_malloc(longitud,"Can not allocate memory for new snapshot");

    memcpy(destino_snapshot,origen,longitud);

    //Aqui llega la parte exclusiva, parte del problema de writer-readers
	//Adquirir lock
	while(z_atomic_test_and_set(&zeng_online_rooms_list[room].semaphore_writing_snapshot)) {
		//printf("Esperando a liberar lock semaphore_writing_snapshot en zengonline_put_snapshot\n");
	}


    //Aqui cambiamos el snapshot de la habitacion por ese otro
    if (zeng_online_rooms_list[room].snapshot_memory!=NULL) free(zeng_online_rooms_list[room].snapshot_memory);

    zeng_online_rooms_list[room].snapshot_memory=destino_snapshot;
    zeng_online_rooms_list[room].snapshot_size=longitud;
    //Incrementamos el id de snapshot
    zeng_online_rooms_list[room].snapshot_id++;


	//Liberar lock
	z_atomic_reset(&zeng_online_rooms_list[room].semaphore_writing_snapshot);

}

void init_zeng_online_rooms(void)
{

    debug_printf(VERBOSE_INFO,"Initializing ZENG Online rooms");

    int i;

    for (i=0;i<ZENG_ONLINE_MAX_ROOMS;i++) {
        zeng_online_rooms_list[i].created=0;
        zeng_online_rooms_list[i].max_players=ZENG_ONLINE_MAX_PLAYERS_PER_ROOM;
        zeng_online_rooms_list[i].current_players=0;
        strcpy(zeng_online_rooms_list[i].name,"<free>");
        zeng_online_rooms_list[i].snapshot_memory=NULL;
        zeng_online_rooms_list[i].snapshot_size=0;
        zeng_online_rooms_list[i].snapshot_id=0;
        zeng_online_rooms_list[i].reading_snapshot_count=0;
        zeng_online_rooms_list[i].index_event=0;
        zeng_online_rooms_list[i].index_waiting_join_first=0;
        zeng_online_rooms_list[i].total_waiting_join=0;

        z_atomic_reset(&zeng_online_rooms_list[i].mutex_reading_snapshot);
        z_atomic_reset(&zeng_online_rooms_list[i].semaphore_writing_snapshot);
        z_atomic_reset(&zeng_online_rooms_list[i].semaphore_events);
        z_atomic_reset(&zeng_online_rooms_list[i].semaphore_joined_users);


    }
}



void enable_zeng_online(void)
{
    zeng_online_enabled=1;
    //TODO: acciones adicionales al activarlo
}

void disable_zeng_online(void)
{
    zeng_online_enabled=0;
    //TODO: acciones adicionales al desactivarlo
}

void zeng_online_set_room_name(int room,char *room_name)
{

    int i;

    //El nombre, excluyendo el 0 del final, y filtrando caracteres raros
    for (i=0;room_name[i] && i<ZENG_ONLINE_MAX_ROOM_NAME;i++) {
        z80_byte caracter=room_name[i];
        if (caracter<32 || caracter>126) caracter='?';

        zeng_online_rooms_list[room].name[i]=caracter;
    }

    //Y el 0 del final
    zeng_online_rooms_list[room].name[i]=0;

}

void zeng_online_assign_room_passwords(int room)
{

    int i;

    for (i=0;i<ZENG_ROOM_PASSWORD_LENGTH;i++) {
        //letras mayus de la A a la Z (26 caracteres)
        ay_randomize(0);

        int valor_random=randomize_noise[0];

        int letra=valor_random % 26;

        char caracter_password='A'+letra;

        zeng_online_rooms_list[room].user_password[i]=caracter_password;
    }

    for (i=0;i<ZENG_ROOM_PASSWORD_LENGTH;i++) {
        //letras mayus de la A a la Z (26 caracteres)
        ay_randomize(0);

        int valor_random=randomize_noise[0];

        int letra=valor_random % 26;

        char caracter_password='A'+letra;

        zeng_online_rooms_list[room].creator_password[i]=caracter_password;
    }

}

void zeng_online_create_room(int misocket,int room_number,char *room_name)
{
    //comprobaciones
    if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
        escribir_socket_format(misocket,"ERROR. Room number beyond limit");
        return;
    }

    //Ver si esta libre
    if (zeng_online_rooms_list[room_number].created) {
        escribir_socket_format(misocket,"ERROR. Room is already created");
        return;
    }

    zeng_online_set_room_name(room_number,room_name);
    zeng_online_assign_room_passwords(room_number);

    //zeng_online_rooms_list[room_number].max_players=ZENG_ONLINE_MAX_PLAYERS_PER_ROOM;

    zeng_online_rooms_list[room_number].current_players=0;
    zeng_online_rooms_list[room_number].snapshot_memory=NULL;

    zeng_online_rooms_list[room_number].autojoin_enabled=0;

    zeng_online_rooms_list[room_number].broadcast_message_id=0;
    zeng_online_rooms_list[room_number].broadcast_message[0]=0;
    zeng_online_rooms_list[room_number].broadcast_messages_allowed=1;

    zeng_online_rooms_list[room_number].created=1;

    //Inicializar lista de usuarios vacia
    int i;
    for (i=0;i<ZENG_ONLINE_MAX_PLAYERS_PER_ROOM;i++) {
        zeng_online_rooms_list[room_number].joined_users[i][0]=0;
        zeng_online_rooms_list[room_number].joined_users_uuid[i][0]=0;
    }

    //Inicializar perfiles de teclas
    for (i=0;i<ZOC_MAX_KEYS_PROFILES;i++) {
        zeng_online_rooms_list[room_number].allowed_keys[i][0]=0; //0 teclas
        zeng_online_rooms_list[room_number].allowed_keys_assigned[i][0]=0; //Asignado a nadie
    }

    //Retornar el creator password
    escribir_socket(misocket,zeng_online_rooms_list[room_number].creator_password);

}

void zeng_online_destroy_room(int misocket,int room_number)
{
    //comprobaciones
    if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
        escribir_socket_format(misocket,"ERROR. Room number beyond limit");
        return;
    }

    //Ver si esta libre
    if (zeng_online_rooms_list[room_number].created==0) {
        escribir_socket_format(misocket,"ERROR. Room is not created");
        return;
    }

    zeng_online_rooms_list[room_number].max_players=ZENG_ONLINE_MAX_PLAYERS_PER_ROOM;

    zeng_online_rooms_list[room_number].current_players=0;

    strcpy(zeng_online_rooms_list[room_number].name,"<free>");

    if (zeng_online_rooms_list[room_number].snapshot_memory!=NULL) {
        free(zeng_online_rooms_list[room_number].snapshot_memory);
    }

    zeng_online_rooms_list[room_number].created=0;


}

void zeng_online_parse_command(int misocket,int comando_argc,char **comando_argv,char *ip_source_address)
{
    //TODO: si el parse para un comando largo, como put-snapshot, fuese lento, habria que procesarlo diferente:
    //ir hasta el primer espacio, y no procesar los dos parametros

    if (comando_argc<1) {
        escribir_socket(misocket,"ERROR. Needs at least one parameter");
        return;
    }

    //Este comando_argc indicara los parametros del comando zengonline XXX, o sea,
    //zengonline enable : parametros 0
    //zengonline join 0: parametros 1
    comando_argc--;
    //printf("Parametros zeng online comando: %d\n",comando_argc);

    if (!strcmp(comando_argv[0],"is-enabled")) {
        escribir_socket_format(misocket,"%d",zeng_online_enabled);
    }

    else if (!strcmp(comando_argv[0],"enable")) {
        if (zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. Already enabled");
        }
        else {
            enable_zeng_online();
        }
    }

    else if (!strcmp(comando_argv[0],"disable")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. Already disabled");
        }
        else {
            disable_zeng_online();
        }
    }

    else if (!strcmp(comando_argv[0],"list-rooms")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        int i;

        escribir_socket(misocket,"N.  Created Autojoin Players Max Name\n");

        for (i=0;i<zeng_online_current_max_rooms;i++) {
            escribir_socket_format(misocket,"%3d %d       %d      %3d       %3d %s\n",
                i,
                zeng_online_rooms_list[i].created,
                zeng_online_rooms_list[i].autojoin_enabled,
                zeng_online_rooms_list[i].current_players,
                zeng_online_rooms_list[i].max_players,
                zeng_online_rooms_list[i].name
            );
        }
    }

    //create-room, cuando se crea desde menu, debe comprobar que no se retorna mensaje de ERROR, y/o mostrar el error al usuario
    else if (!strcmp(comando_argv[0],"create-room")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        if (zeng_online_allow_room_creation_from_any_ip.v==0) {
            if (strcmp(ip_source_address,"127.0.0.1")) {
                //Realmente el mensaje seria: This server only allows localhost room creation
                //pero no queremos dar muchas pistas a un posible atacante
                escribir_socket(misocket,"ERROR. This server does not allow room creation");
                return;
            }
        }

        int room_number=parse_string_to_number(comando_argv[1]);
        zeng_online_create_room(misocket,room_number,comando_argv[2]);
    }

    //TODO esto solo es temporal
    /*else if (!strcmp(comando_argv[0],"pass-room")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<1) {
            escribir_socket(misocket,"ERROR. Needs one parameter");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[1]);

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }


        escribir_socket_format(misocket,"%s",zeng_online_rooms_list[room_number].user_password);
    }
    */



    //get-join-queue-size creator_pass n
    else if (!strcmp(comando_argv[0],"get-join-queue-size")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar creator_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }


        escribir_socket_format(misocket,"%d",zeng_online_rooms_list[room_number].total_waiting_join);

    }

    //"get-join-first-element-queue creator_pass n  Gets the first element in join waiting queue on room n\n"
    else if (!strcmp(comando_argv[0],"get-join-first-element-queue")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar creator_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }

        //si vacio, retornar "empty"
        if (zeng_online_rooms_list[room_number].total_waiting_join==0) {
            escribir_socket(misocket,"<empty>");
            return;
        }

        //TODO bloqueo de esto
        int indice_primero=zeng_online_rooms_list[room_number].index_waiting_join_first;
        escribir_socket_format(misocket,"%s",zeng_online_rooms_list[room_number].join_waiting_list[indice_primero].nickname);
        //fin bloqueo

    }

    //"authorize-join creator_pass n perm Authorize/deny permissions to client join to room n. perm are permissions, can be:\n"
    else if (!strcmp(comando_argv[0],"authorize-join")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<3) {
            escribir_socket(misocket,"ERROR. Needs three parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar creator_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }

        int id_authorization=zeng_online_rooms_list[room_number].index_waiting_join_first;
        int permissions_client=parse_string_to_number(comando_argv[3]);

        if (zeng_online_rooms_list[room_number].total_waiting_join==0) {
            escribir_socket(misocket,"ERROR. There's not any pending join authorization");
            return;
        }

        //TODO bloqueo desde aqui
        zeng_online_rooms_list[room_number].join_waiting_list[id_authorization].permissions=permissions_client;
        zeng_online_rooms_list[room_number].join_waiting_list[id_authorization].waiting=0;


        join_delete_first_element(room_number);
        //hasta aqui

    }


    //set-max-players user_pass n m  Define max-players (m) for room (n). Requires user_pass of that room\n"
    else if (!strcmp(comando_argv[0],"set-max-players")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<3) {
            escribir_socket(misocket,"ERROR. Needs three parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }


        int max_players=parse_string_to_number(comando_argv[3]);

        if (max_players<1 || max_players>ZENG_ONLINE_MAX_PLAYERS_PER_ROOM) {
            escribir_socket(misocket,"ERROR. Max players beyond limit");
            return;
        }

        zeng_online_rooms_list[room_number].max_players=max_players;

    }

    //"set-autojoin creator_pass n p       Define permissions (p) for autojoin on room (n)
    else if (!strcmp(comando_argv[0],"set-autojoin")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<3) {
            escribir_socket(misocket,"ERROR. Needs three parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }


        int permissions=parse_string_to_number(comando_argv[3]);

        zeng_online_rooms_list[room_number].autojoin_enabled=1;
        zeng_online_rooms_list[room_number].autojoin_permissions=permissions;

    }

    //"set-allow-messages creator_pass n   Allows sending messages (allowed by default)\n"
    else if (!strcmp(comando_argv[0],"set-allow-messages")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }


        zeng_online_rooms_list[room_number].broadcast_messages_allowed=1;

    }

    //"reset-allow-messages creator_pass n   Allows sending messages (allowed by default)\n"
    else if (!strcmp(comando_argv[0],"reset-allow-messages")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }


        zeng_online_rooms_list[room_number].broadcast_messages_allowed=0;

    }

       //send-message user_pass n nickname message
    else if (!strcmp(comando_argv[0],"send-message")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<4) {
            escribir_socket(misocket,"ERROR. Needs four parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }

        if (!zeng_online_rooms_list[room_number].broadcast_messages_allowed) {
            escribir_socket(misocket,"ERROR. Broadcast messages are not allowed in this room");
            return;
        }

        //No hace falta indicar room number dado que solo se mostraran mensajes de nuestro room
        sprintf(zeng_online_rooms_list[room_number].broadcast_message,
            "Message from %s: %s",
            comando_argv[3],
            comando_argv[4]);

        zeng_online_rooms_list[room_number].broadcast_message_id++;

    }

    //"list-users user_pass n               Gets the list of joined users on room n\n"
    else if (!strcmp(comando_argv[0],"list-users")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }

        int i;
        for (i=0;i<zeng_online_rooms_list[room_number].max_players;i++) {
            if (zeng_online_rooms_list[room_number].joined_users[i][0]) {
                escribir_socket_format(misocket,"%s\n",
                    zeng_online_rooms_list[room_number].joined_users[i]);
            }
        }


    }

    //"get-message-id user_pass n          Gets the broadcast message id from room\n"
    else if (!strcmp(comando_argv[0],"get-message-id")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }

        escribir_socket_format(misocket,"%d",zeng_online_rooms_list[room_number].broadcast_message_id);

    }
    //"get-message user_pass n             Gets the broadcast message from room\n"
    else if (!strcmp(comando_argv[0],"get-message")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }

        escribir_socket(misocket,zeng_online_rooms_list[room_number].broadcast_message);

    }

    //"send-keys user_pass n uuid key event nomenu   Simulates sending key press/release to room n.\n"
    else if (!strcmp(comando_argv[0],"send-keys")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<6) {
            escribir_socket(misocket,"ERROR. Needs six parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }


        //uuid key event nomenu

		int tecla=parse_string_to_number(comando_argv[4]);
		int event_type=parse_string_to_number(comando_argv[5]);
		int nomenu=parse_string_to_number(comando_argv[6]);

        if (zengonline_valid_event(room_number,comando_argv[3],tecla)) {
            zengonline_add_event(room_number,comando_argv[3],tecla,event_type,nomenu);
        }


    }

    //get-keys user_pass n
    else if (!strcmp(comando_argv[0],"get-keys")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }

        //Siempre empiezo desde la posicion actual del buffer
        int indice_lectura=zeng_online_rooms_list[room_number].index_event;

        //TODO: ver posible manera de salir de aqui??
        while (1) {
            if (zeng_online_rooms_list[room_number].index_event==indice_lectura) {
                //Esperar algo.
                //TODO: parametro configurable
                usleep(1000); // (20 ms es un frame entero)
            }
            else {
                //Retornar evento de la lista
                //Returned format is: # uuid key event nomenu #". Los # inicial y final es para validar que se recibe bien
                int escritos=escribir_socket_format(misocket,"# %s %d %d %d #\n",
                    zeng_online_rooms_list[room_number].events[indice_lectura].uuid,
                    zeng_online_rooms_list[room_number].events[indice_lectura].tecla,
                    zeng_online_rooms_list[room_number].events[indice_lectura].pressrelease,
                    zeng_online_rooms_list[room_number].events[indice_lectura].nomenu
                );

                /*printf("Retorno key-keys: %s %d %d %d\n",
                    zeng_online_rooms_list[room_number].events[indice_lectura].uuid,
                    zeng_online_rooms_list[room_number].events[indice_lectura].tecla,
                    zeng_online_rooms_list[room_number].events[indice_lectura].pressrelease,
                    zeng_online_rooms_list[room_number].events[indice_lectura].nomenu
                );*/

                indice_lectura++;
                if (indice_lectura>=ZENG_ONLINE_MAX_EVENTS) {
                    indice_lectura=0;
                }

                //printf("Escritos socket: %d\n",escritos);
                if (escritos<0) {
                    debug_printf(VERBOSE_DEBUG,"Error returning zeng-online get-keys. Client connection may be closed");
                    return;
                }

            }
        }


    }

    //"get-snapshot user_pass n          Get a snapshot from room n, returns ERROR if no snapshot there. Requires user_pass\n"
    else if (!strcmp(comando_argv[0],"get-snapshot")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }

        if (!zeng_online_rooms_list[room_number].snapshot_size) {
            escribir_socket(misocket,"ERROR. There is no snapshot on this room");
            return;
        }



        z80_byte *puntero_snapshot=util_malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2,"Can not allocate memory for get snapshot");

        int longitud=zengonline_get_snapshot(room_number,puntero_snapshot);


        int i;
        for (i=0;i<longitud;i++) {
            escribir_socket_format(misocket,"%02X",puntero_snapshot[i]);
        }

        free(puntero_snapshot);




    }

    else if (!strcmp(comando_argv[0],"get-snapshot-id")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }

        if (!zeng_online_rooms_list[room_number].snapshot_size) {
            escribir_socket(misocket,"ERROR. There is no snapshot on this room");
            return;
        }



        escribir_socket_format(misocket,"%d",zeng_online_rooms_list[room_number].snapshot_id);





    }

    //"destroy-room creator_pass n         Destroys room n\n"
    else if (!strcmp(comando_argv[0],"destroy-room")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }

        zeng_online_destroy_room(misocket,room_number);
    }

    else if (!strcmp(comando_argv[0],"reset-autojoin")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<2) {
            escribir_socket(misocket,"ERROR. Needs two parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }

        zeng_online_rooms_list[room_number].autojoin_enabled=0;
    }

    //"put-snapshot creator_pass n data
    else if (!strcmp(comando_argv[0],"put-snapshot")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<3) {
            escribir_socket(misocket,"ERROR. Needs three parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[2]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass. comando_argv[1]
        if (strcmp(comando_argv[1],zeng_online_rooms_list[room_number].creator_password)) {
            escribir_socket(misocket,"ERROR. Invalid creator password for that room");
            return;
        }

        //longitud del snapshot es la longitud del parametro snapshot /2 (porque viene en hexa)
        int longitud_snapshot=strlen(comando_argv[3])/2;

        if (longitud_snapshot<1) {
            escribir_socket(misocket,"ERROR. Received an empty snapshot");
            return;
        }

        char *s=comando_argv[3];
        //int parametros_recibidos=0;

        z80_byte *buffer_destino;
        buffer_destino=malloc(longitud_snapshot);
        if (buffer_destino==NULL) cpu_panic("Can not allocate memory for put-snapshot");

        //z80_byte valor;


        //Se usa un bucle mucho mas rapido que si se usase parse_string_to_number
        //tiempo mas bajo usando version "lenta" del bucle: 52 microsec
        //usando version rapida: 7 microsec


        /*while (*s) {
            char buffer_valor[4];
            buffer_valor[0]=s[0];
            buffer_valor[1]=s[1];
            buffer_valor[2]='H';
            buffer_valor[3]=0;
            //printf ("%s\n",buffer_valor);
            //TODO: quiza este parse como es continuo se puede acelerar de alguna manera
            valor=parse_string_to_number(buffer_valor);
            //printf ("valor: %d\n",valor);

            buffer_destino[parametros_recibidos++]=valor;
            //menu_debug_write_mapped_byte(direccion++,valor);

            s++;
            if (*s) s++;
        }*/

        z80_byte *destino=buffer_destino;
        while (*s) {
            *destino=(util_hex_nibble_to_byte(*s)<<4) | util_hex_nibble_to_byte(*(s+1));
            destino++;

            s++;
            if (*s) s++;
        }

        zengonline_put_snapshot(room_number,buffer_destino,longitud_snapshot);

        free(buffer_destino);


    }

    //leave n user_pass uuid
    //TODO: no hacemos nada con el nickname, solo mostrarlo en footer
    else if (!strcmp(comando_argv[0],"leave")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<3) {
            escribir_socket(misocket,"ERROR. Needs three parameters");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[1]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        //validar user_pass.
        if (strcmp(comando_argv[2],zeng_online_rooms_list[room_number].user_password)) {
            escribir_socket(misocket,"ERROR. Invalid user password for that room");
            return;
        }


        //TODO: seguro que hay que hacer mas cosas en el leave...


        //comando_argv[3] contiene el uuid
        zoc_del_user_to_joined_users(room_number,comando_argv[3]);




    }


    //join n. Aunque el master requiere join tambien, no necesita el user_password que le retorna, pero debe leerlo
    //para quitar esa respuesta del socket
    // "join n nickname uuid [creator_pass]
    else if (!strcmp(comando_argv[0],"join")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<3) {
            escribir_socket(misocket,"ERROR. Needs three parameters at least");
            return;
        }

        int room_number=parse_string_to_number(comando_argv[1]);

        if (room_number<0 || room_number>=zeng_online_current_max_rooms) {
            escribir_socket_format(misocket,"ERROR. Room number beyond limit");
            return;
        }

        if (!zeng_online_rooms_list[room_number].created) {
            escribir_socket(misocket,"ERROR. Room is not created");
            return;
        }

        if (zeng_online_rooms_list[room_number].current_players >= zeng_online_rooms_list[room_number].max_players) {
            escribir_socket(misocket,"ERROR. Maximum players in that room reached");
            return;
        }

        int permissions;

        //Si tiene 4 parámetros, el cuarto es creator_pass para no necesitar autorización del master
        if (comando_argc>3) {
            if (strcmp(comando_argv[4],zeng_online_rooms_list[room_number].creator_password)) {
                escribir_socket(misocket,"ERROR. Invalid creator password for that room");
                return;
            }
            //Damos casi todos permisos al master, excepto read snapshot
            permissions=ZENG_ONLINE_PERMISSIONS_ALL_MASTER;

        }

        else {
            //Esperar hasta recibir autorización del master
            //Ver si hay autojoin
            if (zeng_online_rooms_list[room_number].autojoin_enabled) {
                printf("Autojoin for room %d is enabled with permissions %d\n",room_number,
                zeng_online_rooms_list[room_number].autojoin_permissions);
                permissions=zeng_online_rooms_list[room_number].autojoin_permissions;

            }

            else {

                printf("Waiting to receive authorization from master\n");
                int id_authorization=join_list_add_element(room_number,comando_argv[2]); //nickname agregado a la lista

                int timeout=60;
                while (zeng_online_rooms_list[room_number].join_waiting_list[id_authorization].waiting && timeout) {
                    printf("Waiting authorization...\n");
                    sleep(1);
                }
                if (zeng_online_rooms_list[room_number].join_waiting_list[id_authorization].waiting) {
                    escribir_socket(misocket,"ERROR. Timeout waiting for client join authorization");
                    return;
                }

                permissions=zeng_online_rooms_list[room_number].join_waiting_list[id_authorization].permissions;

            }

            printf("Permissions for this join: %d\n",permissions);
            //Si permisos 0 , denegado join
            if (permissions==0) {
                escribir_socket(misocket,"ERROR. You are not authorized to join");
                return;
            }


            //estos permisos ya es reponsabilidad de ZEsarUX hacerle caso desde las funciones cliente
            //el servidor no va a comprobar dichos permisos, seria demasiado lio (y logicamente esto es un pequeño problema de seguridad)
            //ZEsarUX desde el menu hace el join, obtiene los permisos que le retorna el servidor, y tiene que ser
            //fiel en recibir o no snapshot, en enviar o no teclas, etc


        }

        //TODO: seguro que hay que hacer mas cosas en el join...


        zoc_add_user_to_joined_users(room_number,comando_argv[2],comando_argv[3]);

        //Y retornamos el user_password
        escribir_socket_format(misocket,"%s %d",zeng_online_rooms_list[room_number].user_password,permissions);




    }

    //TODO comandos
    //
    //delete-room: para una room concreta, requiere creator_password.
    //leave: para una room concreta, requiere que este creada, requiere user_password. hay que asegurarse que el leave
    //       es de esta conexion, y no de una nueva. como controlar eso??? el leave decrementara el numero de jugadores conectados, logicamente

    else {
        escribir_socket(misocket,"ERROR. Invalid command for zeng-online");
        return;
    }
}


