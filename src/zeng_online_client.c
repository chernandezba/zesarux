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
#include "zeng_online_client.h"
#include "remote.h"
#include "snap_zsf.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng_online_client_list_rooms;


#endif

//Variables y funciones que no son de pthreads
int zeng_online_client_list_rooms_thread_running=0;

#ifdef USE_PTHREADS

//Funciones que usan pthreads


void *zeng_online_client_list_rooms_function(void *nada GCC_UNUSED)
{

	zeng_online_client_list_rooms_thread_running=1;



	//Conectar a remoto
    /*
	if (!zeng_connect_remotes()) {
		//Desconectar solo si el socket estaba conectado

		//if (zeng_remote_socket>=0)
        //Desconectar los que esten conectados
        zeng_disconnect_remote();

		zeng_online_client_list_rooms_thread_running=0;
		return 0;
	}
    */
   sleep(5);





	//zeng_enabled.v=1;


	zeng_online_client_list_rooms_thread_running=0;

	return 0;

}


void zeng_online_client_list_rooms(void)
{

	//ya  inicializado
	/*if (zeng_enabled.v) return;

	if (zeng_remote_hostname[0]==0) return;*/



	//Inicializar thread

	if (pthread_create( &thread_zeng_online_client_list_rooms, NULL, &zeng_online_client_list_rooms_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng online list rooms pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng_online_client_list_rooms);





}


#else

//Funciones sin pthreads. ZENG no se llama nunca cuando no hay pthreads, pero hay que crear estas funciones vacias
//para evitar errores de compilacion cuando no hay pthreads

void zeng_online_client_list_rooms(void)
{
}


#endif