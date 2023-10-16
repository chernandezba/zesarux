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

#endif
