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

#ifndef ZENG_ONLINE_H
#define ZENG_ONLINE_H

#include "atomic.h"
#include "stats.h"

//Maximo nombre para una habitacion
//Si se cambia esto, ajustar nombre con espacios en init_zeng_online_rooms, para que quede 30 de longitud,
//Y en cabecera de list-rooms
#define ZENG_ONLINE_MAX_ROOM_NAME 30

//Maximo nickname
#define ZOC_MAX_NICKNAME_LENGTH 10

//Maximo total de habitaciones que se pueden crear. Valor hardcoded, no se puede fijar mas que este limite (pero si marcar un limite inferior)
#define ZENG_ONLINE_MAX_ROOMS 40

//Maximo total de jugadores permitidos en una habitacion. Valor hardcoded, no se puede ir mas alla pero si menos
#define ZENG_ONLINE_MAX_PLAYERS_PER_ROOM 100

#define ZENG_ROOM_PASSWORD_LENGTH 10

#define ZENG_ONLINE_MAX_BROADCAST_MESSAGE_LENGTH 64

//En el mensaje se le agrega:
//Message from room XX user XX
//Mas que suficiente el margen de ZOC_MAX_NICKNAME_LENGTH+100
#define ZENG_ONLINE_MAX_BROADCAST_MESSAGE_SHOWN_LENGTH (ZENG_ONLINE_MAX_BROADCAST_MESSAGE_LENGTH+ZOC_MAX_NICKNAME_LENGTH+100+1)

//Maximo de eventos (tecla, joystick) que se mantiene en cola de una habitacion
//TODO: cual es el mejor valor para esto? Consideramos algo multiplicado por ZENG_ONLINE_MAX_PLAYERS_PER_ROOM?
//Aunque no asumo que todos los usuarios (100??) de una sala vayan a pulsar las teclas a la vez
#define ZENG_ONLINE_MAX_EVENTS 30



//Maximo de perfiles de teclas permitidos por cada habitacion
#define ZOC_MAX_KEYS_PROFILES 4
//Maximo de teclas permitidas por cada perfil
//OJO! El total en ZENG de parametros es REMOTE_MAX_PARAMETERS_COMMAND, siempre ese valor tendra que ser mayor que ZOC_MAX_KEYS_ITEMS
#define ZOC_MAX_KEYS_ITEMS 10


#define ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT 1
#define ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT 2
#define ZENG_ONLINE_PERMISSIONS_GET_KEYS 4
#define ZENG_ONLINE_PERMISSIONS_SEND_KEYS 8
#define ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE 16

//Usado por ejemplo cuando hay un join y queremos indicar todos permisos para un slave
#define ZENG_ONLINE_PERMISSIONS_ALL_SLAVE (ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT|ZENG_ONLINE_PERMISSIONS_GET_KEYS|ZENG_ONLINE_PERMISSIONS_SEND_KEYS|ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE)

//Damos casi todos permisos al master, excepto read snapshot
#define ZENG_ONLINE_PERMISSIONS_ALL_MASTER (ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT|ZENG_ONLINE_PERMISSIONS_GET_KEYS|ZENG_ONLINE_PERMISSIONS_SEND_KEYS|ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE)


extern void init_zeng_online_rooms(void);
extern void zeng_online_parse_command(int misocket,int comando_argc,char **comando_argv,char *ipsource);

extern int zengonline_get_snapshot(int room,z80_byte *destino);
extern void zengonline_put_snapshot(int room,z80_byte *origen,int longitud);

extern int zeng_online_enabled;
extern void enable_zeng_online(void);
extern void disable_zeng_online(void);

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

    //Ultimo usuario que se le ha hecho "kick"
    char kicked_user[STATS_UUID_MAX_LENGTH+1];

};

extern struct zeng_online_room zeng_online_rooms_list[];

extern int zeng_online_current_max_rooms;

#endif
