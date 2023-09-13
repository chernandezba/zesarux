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

Online Network Play (using a central server) related code

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



#ifdef USE_PTHREADS



#endif

//Variables, estructuras,funciones etc que se pueden compilar aun sin soporte de pthreads

//Numero maximo de habitaciones para esta sesion que se pueden crear
//Interesa que este valor se pueda bajar o subir (pero no subir nunca mas alla de ZENG_ONLINE_MAX_ROOMS),
//porque segun la potencia del server se puede permitir mas o menos
int zeng_online_current_max_rooms=10;


int zeng_online_enabled=0;

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

    //Es un contador atomico, mas que un semaforo realmente. Indica cuantas personas en esa room estan usando un snapshot
    z_atomic_semaphore reading_snapshot;


};

//Array de habitaciones en zeng online
struct zeng_online_room zeng_online_rooms_list[ZENG_ONLINE_MAX_ROOMS];

void init_zeng_online_rooms(void)
{

    debug_printf(VERBOSE_INFO,"Initializing ZENG Online rooms");

    int i;

    for (i=0;i<ZENG_ONLINE_MAX_ROOMS;i++) {
        zeng_online_rooms_list[i].created=0;
        zeng_online_rooms_list[i].max_players=ZENG_ONLINE_MAX_PLAYERS_PER_ROOM;
        zeng_online_rooms_list[i].current_players=0;
        strcpy(zeng_online_rooms_list[i].name,"<free>                        ");
        zeng_online_rooms_list[i].snapshot_memory=NULL;
        zeng_online_rooms_list[i].snapshot_size=0;

        z_atomic_reset(&zeng_online_rooms_list[i].reading_snapshot);


    }
}

//Funciones que usan pthreads
#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>

pthread_t xxxxxxx;

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
    //primero rellenar con espacios, asi queda siempre alineado
    int i;

    for (i=0;i<ZENG_ONLINE_MAX_ROOM_NAME;i++) {
        zeng_online_rooms_list[room].name[i]=' ';
    }

    //Y el 0 del final
    zeng_online_rooms_list[room].name[i]=0;

    //Y ahora el nombre, excluyendo el 0 del final, y filtrando caracteres raros
    for (i=0;room_name[i] && i<ZENG_ONLINE_MAX_ROOM_NAME;i++) {
        z80_byte caracter=room_name[i];
        if (caracter<32 || caracter>126) caracter='?';

        zeng_online_rooms_list[room].name[i]=caracter;
    }

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

    zeng_online_rooms_list[room_number].max_players=ZENG_ONLINE_MAX_PLAYERS_PER_ROOM;
    zeng_online_rooms_list[room_number].current_players=0;
    zeng_online_rooms_list[room_number].snapshot_memory=NULL;

    zeng_online_rooms_list[room_number].created=1;

    //Retornar el creator password
    escribir_socket(misocket,zeng_online_rooms_list[room_number].creator_password);

}

void zeng_online_parse_command(int misocket,int comando_argc,char **comando_argv)
{
    //TODO: si el parse para un comando largo, como put-snapshot, fuese lento, habria que procesarlo diferente:
    //ir hasta el primer espacio, y no procesar los dos parametros

    if (comando_argc<1) {
        escribir_socket(misocket,"ERROR. Needs at least one parameter");
        return;
    }

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

        escribir_socket(misocket,"N.  Name                           Created Players Max\n");

        for (i=0;i<zeng_online_current_max_rooms;i++) {
            escribir_socket_format(misocket,"%3d %s %d     %3d       %3d\n",
                i,
                zeng_online_rooms_list[i].name,
                zeng_online_rooms_list[i].created,
                zeng_online_rooms_list[i].current_players,
                zeng_online_rooms_list[i].max_players
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

        int room_number=parse_string_to_number(comando_argv[1]);
        zeng_online_create_room(misocket,room_number,comando_argv[2]);
    }

    //TODO esto solo es temporal
    else if (!strcmp(comando_argv[0],"pass-room")) {
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



    //join n. Aunque el master requiere join tambien, no necesita el user_password que le retorna, pero debe leerlo
    //para quitar esa respuesta del socket
    else if (!strcmp(comando_argv[0],"join")) {
        if (!zeng_online_enabled) {
            escribir_socket(misocket,"ERROR. ZENG Online is not enabled");
            return;
        }

        if (comando_argc<1) {
            escribir_socket(misocket,"ERROR. Needs one parameter");
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

        //TODO: seguro que hay que hacer mas cosas en el join...
        zeng_online_rooms_list[room_number].current_players++;

        //Y retornamos el user_password
        escribir_socket(misocket,zeng_online_rooms_list[room_number].user_password);

    }

    //TODO comandos
    //
    //delete-room: para una room concreta, requiere creator_password.
    //leave: para una room concreta, requiere que este creada, requiere user_password. hay que asegurarse que el leave
    //       es de esta conexion, y no de una nueva. como controlar eso??? el leave decrementara el numero de jugadores conectados, logicamente
    //put-snapshot: para una room concreta, requiere creator_password.
    //get-snapshot: para una room concreta, requiere user_password.
    //send-keys-event: para una room concreta, requiere user_password.

    else {
        escribir_socket(misocket,"ERROR. Invalid command for zeng-online");
        return;
    }
}


#else

//Funciones sin pthreads. ZENG_ONLINE no se llama nunca cuando no hay pthreads, pero hay que crear estas funciones vacias
//para evitar errores de compilacion cuando no hay pthreads

void zeng_online_xxxxx(void)
{
}


#endif