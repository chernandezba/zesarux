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
antes esperando a que el contador atómico esté a 0


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



#ifdef USE_PTHREADS



#endif

//Variables, estructuras,funciones etc que se pueden compilar aun sin soporte de pthreads

//Numero maximo de habitaciones para esta sesion que se pueden crear
//Interesa que este valor se pueda bajar o subir (pero no subir nunca mas alla de ZENG_ONLINE_MAX_ROOMS),
//porque segun la potencia del server se puede permitir mas o menos
int zeng_online_current_max_rooms=10;

//Numero maximo de jugadores por cada habitacion
int zeng_online_current_max_players=20;

int zeng_online_enabled=0;

//Estructura de una habitacion de zeng online
struct zeng_online_room {
    int created;
    char name[ZENG_ONLINE_MAX_ROOM_NAME+1]; //+1 para el 0 del final
    z80_byte *snapshot_memory; //Donde esta asignado el snapshot
};

//Array de habitaciones en zeng online
struct zeng_online_room zeng_online_rooms_list[ZENG_ONLINE_MAX_ROOMS];

void init_zeng_online_rooms(void)
{

    debug_printf(VERBOSE_INFO,"Initializing ZENG Online rooms");

    int i;

    for (i=0;i<ZENG_ONLINE_MAX_ROOMS;i++) {
        zeng_online_rooms_list[i].created=0;
        strcpy(zeng_online_rooms_list[i].name,"<free>");
        zeng_online_rooms_list[i].snapshot_memory=NULL;
    }
}

//Funciones que usan pthreads
#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>

pthread_t xxxxxxx;




#else

//Funciones sin pthreads. ZENG_ONLINE no se llama nunca cuando no hay pthreads, pero hay que crear estas funciones vacias
//para evitar errores de compilacion cuando no hay pthreads

void zeng_online_xxxxx(void)
{
}


#endif