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

//Maximo total de habitaciones que se pueden crear. Valor hardcoded, no se puede fijar mas que este limite (pero si marcar un limite inferior)
#define ZENG_ONLINE_MAX_ROOMS 40

//Maximo total de jugadores permitidos en una habitacion. Valor hardcoded, no se puede ir mas alla pero si menos
#define ZENG_ONLINE_MAX_PLAYERS_PER_ROOM 100

#define ZENG_ROOM_PASSWORD_LENGTH 10

//Maximo de eventos (tecla, joystick) que se mantiene en cola de una habitacion
//TODO: cual es el mejor valor para esto? Consideramos algo multiplicado por ZENG_ONLINE_MAX_PLAYERS_PER_ROOM?
//Aunque no asumo que todos los usuarios (100??) de una sala vayan a pulsar las teclas a la vez
#define ZENG_ONLINE_MAX_EVENTS 30

extern void init_zeng_online_rooms(void);
extern void zeng_online_parse_command(int misocket,int comando_argc,char **comando_argv);

extern int zengonline_get_snapshot(int room,z80_byte *destino);
extern void zengonline_put_snapshot(int room,z80_byte *origen,int longitud);

extern int zeng_online_enabled;
extern void enable_zeng_online(void);

#endif
