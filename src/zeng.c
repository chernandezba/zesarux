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
Network Play (not using a central server) related code
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
#include "zeng.h"
#include "zeng_online_client.h"
#include "remote.h"
#include "snap_zsf.h"
#include "autoselectoptions.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng;
pthread_t zeng_thread_connect;

#endif

//Si el thread se ha inicializado correctamente
//z80_bit thread_zeng_inicializado={0};



//-ZENG: ZEsarUX Network Gaming



//La fifo

zeng_key_presses zeng_key_presses_array[ZENG_FIFO_SIZE];

//Antiguo indice a zsocket cuando se gestionaba solo 1 slave
//int zeng_remote_socket=-1;

int zeng_remote_sockets[ZENG_MAX_REMOTE_HOSTS];

int zeng_total_remotes=0;


//Tamanyo de la fifo
int zeng_fifo_current_size=0;

//Posicion de agregar de la fifo
int zeng_fifo_write_position=0;

//Posicion de leer de la fifo
int zeng_fifo_read_position=0;

//Si esta habilitado zeng
z80_bit zeng_enabled={0};

//Hostnames remoto
char zeng_remote_hostname[MAX_ZENG_HOSTNAME]="";

//Puerto remoto
int zeng_remote_port=10000;

//int zeng_segundos_cada_snapshot=2;

int zeng_frames_video_cada_snapshot=2;

int zeng_i_am_master=0;

//opcion para no enviar eventos de teclas
int zeng_do_not_send_input_events=0;

//Mensaje de envio a footer remoto
//Con margen +100 de sobra para agregar el comando print-footer
char zeng_send_message_footer[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH+100];

int pending_zeng_send_message_footer=0;

//Cantidad de snapshots no enviados porque hay otro pendiente
int zeng_snapshots_not_sent=0;

//Forzar a reconectar si llega a 3 failed snapshots
z80_bit zeng_force_reconnect_failed_retries={0};

//No mostrar error en thread send
int zeng_thread_send_not_show_error=0;

//Indica que ha habido un error en ejecutar zeng_send_snapshot_uno_concreto
int return_zeng_send_snapshot_uno_concreto;

int zeng_enable_thread_running=0;


#ifdef USE_PTHREADS

int zeng_next_position(int pos)
{
	pos++;
	if (pos==ZENG_FIFO_SIZE) pos=0;
	return pos;
}

//Agregar elemento a la fifo
//Retorna 1 si esta llena
int zeng_fifo_add_element(zeng_key_presses *elemento)
{
	//Si esta llena, no hacer nada
	//TODO: esperar a flush
	//TODO: semaforo

	if (zeng_fifo_current_size==ZENG_FIFO_SIZE) return 1;

	//Escribir en la posicion actual
	zeng_key_presses_array[zeng_fifo_write_position].tecla=elemento->tecla;
	zeng_key_presses_array[zeng_fifo_write_position].pressrelease=elemento->pressrelease;

	//Y poner siguiente posicion
	zeng_fifo_write_position=zeng_next_position(zeng_fifo_write_position);

	//Y sumar total elementos
	zeng_fifo_current_size++;

	return 0;

}

//Leer elemento de la fifo
//Retorna 1 si esta vacia
int zeng_fifo_read_element(zeng_key_presses *elemento)
{
	//TODO: semaforo

	if (zeng_fifo_current_size==0) return 1;

	//Leer de la posicion actual
	elemento->tecla=zeng_key_presses_array[zeng_fifo_read_position].tecla;
	elemento->pressrelease=zeng_key_presses_array[zeng_fifo_read_position].pressrelease;

	//Y poner siguiente posicion
	zeng_fifo_read_position=zeng_next_position(zeng_fifo_read_position);

	//Y restar total elementos
	zeng_fifo_current_size--;

	return 0;

}

void zeng_empty_fifo(void)
{
	//Tamanyo de la fifo
	zeng_fifo_current_size=0;

	//Posicion de agregar de la fifo
	zeng_fifo_write_position=0;

	//Posicion de leer de la fifo
	zeng_fifo_read_position=0;
}


void zeng_send_key_event(enum util_teclas tecla,int pressrelease)
{
	//Si esta menu abierto en origen, no enviar
	if (menu_abierto) return;

    int enviar=0;

    //Si hay zeng online, enviar
    if (zeng_online_connected.v) enviar=1;


    //Si no hay zeng (no el online, sino el directo a host) no enviar
	if (zeng_enabled.v) enviar=1;

    if (!enviar) return;



	//teclas F no enviar
	switch (tecla) {
		case UTIL_KEY_F1:
		case UTIL_KEY_F2:
		case UTIL_KEY_F3:
		case UTIL_KEY_F4:
		case UTIL_KEY_F5:
		case UTIL_KEY_F6:
		case UTIL_KEY_F7:
		case UTIL_KEY_F8:
		case UTIL_KEY_F9:
		case UTIL_KEY_F10:
		case UTIL_KEY_F11:
		case UTIL_KEY_F12:
		case UTIL_KEY_F13:
		case UTIL_KEY_F14:
		case UTIL_KEY_F15:
			return;
		break;

		default:
		break;
	}

	zeng_key_presses elemento;

	elemento.tecla=tecla;
	elemento.pressrelease=pressrelease;

	//printf ("Adding zeng key event to fifo\n");

	if (zeng_fifo_add_element(&elemento)) {
		debug_printf (VERBOSE_DEBUG,"Error adding zeng key event. FIFO full");
		return;
	}

}

//Devuelve 0 si no conectado
//Se conecta a los diferentes hosts separados por comas (si somos master y hay varios slaves)
//No se permite conexion a varios master
int zeng_connect_remotes(void)
{

    //Inicialmente desconectado
    //zeng_remote_socket=-1;

    //Almacenar aqui hostname hasta la ,
    char buffer_hostname[MAX_ZENG_HOSTNAME];

    //Almacenar aqui la copia del campo entero, para facilitar cortar
    char copia_zeng_remote_hostname[MAX_ZENG_HOSTNAME];

    strcpy(copia_zeng_remote_hostname,zeng_remote_hostname);


    char *puntero_hostname=copia_zeng_remote_hostname;

    zeng_total_remotes=0;



    while (*puntero_hostname) {

        int puerto=zeng_remote_port;

        //Obtener nombre hasta ","
        char *existe=strstr(puntero_hostname,",");

        if (existe!=NULL) {
            *existe=0; //fijar fin de caracter
        }

        strcpy(buffer_hostname,puntero_hostname);

        //Y siguiente iteración empezará despues
        if (existe!=NULL) {
            existe++;
            puntero_hostname=existe;
        }

        else {
            //puntero apunta al final
            int longitud=strlen(puntero_hostname);
            puntero_hostname=&puntero_hostname[longitud];
        }


        //Ver si se indica puerto con ":"
        char *existe_puerto=strstr(buffer_hostname,":");

        if (existe_puerto!=NULL) {
            *existe_puerto=0; //fijar fin de caracter

            //Y leer puerto
            existe_puerto++;
            puerto=parse_string_to_number(existe_puerto);
        }


        //printf("Conectando a %s:%d\n",buffer_hostname,puerto);




		int indice_socket=z_sock_open_connection(buffer_hostname,puerto,0,"");

		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"Error connecting to %s:%d. %s",
                buffer_hostname,puerto,
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


		if (zeng_i_am_master) {

			debug_printf(VERBOSE_DEBUG,"ZENG: Sending zeng-is-master");

			//Enviar un zeng-is-master
			escritos=z_sock_write_string(indice_socket,"zeng-is-master\n");

			if (escritos<0) {
				debug_printf(VERBOSE_ERR,"ERROR. Can't send zeng-is-master: %s",z_sock_get_error(escritos));
				return 0;
			}


			//reintentar
			leidos=zsock_read_all_until_command(indice_socket,(z80_byte *)buffer,ZENG_BUFFER_INITIAL_CONNECT,&posicion_command);
			if (leidos>0) {
				buffer[leidos]=0; //fin de texto
				debug_printf(VERBOSE_DEBUG,"ZENG: Received text for zeng-is-master (length %d): \n[\n%s\n]",leidos,buffer);
			}

			if (leidos<0) {
				debug_printf(VERBOSE_ERR,"ERROR. Can't receive zeng-is-master: %s",z_sock_get_error(leidos));
				return 0;
			}

			//1 mas para eliminar el salto de linea anterior a "command>"
			if (posicion_command>=1) {
				buffer[posicion_command-1]=0;
				debug_printf(VERBOSE_DEBUG,"ZENG: Received zeng-is-master: %s",buffer);
			}
			else {
				debug_printf (VERBOSE_ERR,"Error receiving ZEsarUX zeng-is-master parameter");
				return 0;
			}

			//Si somos master, que el remoto no lo sea tambien
			int es_master_remoto=parse_string_to_number(buffer);
			if (es_master_remoto) {
				debug_printf (VERBOSE_ERR,"There is one than one ZENG master. That is NOT recommended. Use at your own risk ;)");
			}

		}


		//escribir_socket(misocket,"Waiting until command prompt final");
		//printf("Waiting until command prompt final\n");

		//zsock_wait_until_command_prompt(indice_socket);




       zeng_remote_sockets[zeng_total_remotes++]=indice_socket;

       if (zeng_total_remotes>ZENG_MAX_REMOTE_HOSTS) {
            debug_printf(VERBOSE_ERR,"No more than %d remote hosts are allowed! Disabling ZENG",ZENG_MAX_REMOTE_HOSTS);
            return 0;
       }



    }

	//zeng_remote_socket=indice_socket;

	return 1;
}

//Devuelve 0 si error
int zeng_disconnect_remote(void)
{
    int i;

    for (i=0;i<zeng_total_remotes;i++) {
	    z_sock_close_connection(zeng_remote_sockets[i]);
    }
	return 1;
}

int contador_envio_snapshot=0;



int zeng_send_snapshot_pending=0;


//zona memoria donde se guarda ya el snapshot con comando put-snapshot y valores hexadecimales
char *zeng_send_snapshot_mem_hexa=NULL; //zeng_send_snapshot_mem_hexa

int zeng_send_snapshot(int socket)
{
	//Enviar snapshot cada 20*250=5000 ms->5 segundos
		debug_printf (VERBOSE_DEBUG,"ZENG: Sending snapshot");

		int posicion_command;
		int escritos,leidos;



				//printf ("Sending put-snapshot\n");
                //printf("before z_sock_write_string 1\n");
				escritos=z_sock_write_string(socket,"put-snapshot ");
                //printf("after z_sock_write_string 1\n");
				if (escritos<0) return escritos;


				//TODO esto es ineficiente y que tiene que calcular la longitud. hacer otra z_sock_write sin tener que calcular
                //printf("before z_sock_write_string 2\n");
				escritos=z_sock_write_string(socket,zeng_send_snapshot_mem_hexa);
                //printf("after z_sock_write_string 2\n");



				if (escritos<0) return escritos;



				z80_byte buffer[200];
				//Leer hasta prompt
                //printf("before zsock_read_all_until_command\n");
				leidos=zsock_read_all_until_command(socket,buffer,199,&posicion_command);
                //printf("after zsock_read_all_until_command\n");
				return leidos;


}

//Estructura para el envio de parametros a cada hilo en zeng_send_keys_onehost

struct s_zeng_send_keys_onehost {
    int tecla;
    int pressrelease;
    int finished;
    pthread_t thread;
};

struct s_zeng_send_keys_onehost zeng_send_keys_onehost_array[ZENG_MAX_REMOTE_HOSTS];


//Variable global que indica que la llamada a zeng_send_keys_onehost ha funcionado mal, si es <0
int return_zeng_send_keys_onehost;

void *thread_zeng_send_keys_onehost(void *p_index_socket)
{

    //Realmente el puntero no lo trato como tal, sino como el valor del indice
    int index_socket=(int)p_index_socket;

    char buffer_comando[256];

    int tecla=zeng_send_keys_onehost_array[index_socket].tecla;
    int pressrelease=zeng_send_keys_onehost_array[index_socket].pressrelease;

    sprintf(buffer_comando,"send-keys-event %d %d 1\n",tecla,pressrelease);
    //el 1 del final indica que no se envia la tecla si el menu en remoto esta abierto

    int escritos=z_sock_write_string(zeng_remote_sockets[index_socket],buffer_comando);


    //printf ("despues de enviar send-keys. escritos en write string: %d\n",escritos);


    //Si ha habido error al escribir en socket
    if (escritos<0) {
        return_zeng_send_keys_onehost=escritos;
        zeng_send_keys_onehost_array[index_socket].finished=1;
        return NULL;
    }


    else {

        z80_byte buffer[200];

        //Leer hasta prompt
        int posicion_command;

        //printf ("antes de leer hasta command prompt\n");
        int leidos=zsock_read_all_until_command(zeng_remote_sockets[index_socket],buffer,199,&posicion_command);

        //printf ("despues de leer hasta command prompt\n");

        //Si ha habido error al leer de socket
        if (leidos<0) {
            return_zeng_send_keys_onehost=leidos;
            zeng_send_keys_onehost_array[index_socket].finished=1;
            return NULL;
        }
    }

    zeng_send_keys_onehost_array[index_socket].finished=1;

    return NULL;

}

void zeng_send_keys_onehost(int index_socket)
{

    //struct s_zeng_send_keys_onehost *parametros=&zeng_send_keys_onehost_array[index_socket];

	if (pthread_create( &zeng_send_keys_onehost_array[index_socket].thread, NULL, &thread_zeng_send_keys_onehost, (void *) index_socket) ) {
		debug_printf(VERBOSE_ERR,"Can not create thread_zeng_send_keys_onehost pthread");
        zeng_send_keys_onehost_array[index_socket].finished=1;
        return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(zeng_send_keys_onehost_array[index_socket].thread);


}

//Retorna <0 si error
int zeng_send_keys(zeng_key_presses *elemento)
{

    if (zeng_do_not_send_input_events) return 1;

    int i;

    return_zeng_send_keys_onehost=1;

    for (i=0;i<zeng_total_remotes;i++) {

        zeng_send_keys_onehost_array[i].finished=0;
        zeng_send_keys_onehost_array[i].tecla=elemento->tecla;
        zeng_send_keys_onehost_array[i].pressrelease=elemento->pressrelease;

        zeng_send_keys_onehost(i);

    }

    //esperar a que finalicen


    //printf("\n");

    int finished;

    do {
        finished=1;
        for (i=0;i<zeng_total_remotes;i++) {
            //printf("socket %d finished %d\n",i,zeng_send_keys_onehost_array[i].finished);
            if (zeng_send_keys_onehost_array[i].finished==0) finished=0;
            else {
		    /*
                if (zeng_send_keys_onehost_array[i].finished!=2) {
                    zeng_send_keys_onehost_array[i].finished=2;
                    //liberar memoria
                    pthread_join(zeng_send_keys_onehost_array[i].thread,NULL);
                }
		*/
            }
        }
        if (!finished) usleep(1000); //dormir 1 ms
        //printf("Finished: %d\n",finished);
    } while (!finished && return_zeng_send_keys_onehost>=0);

    //printf("all sockets finished\n");

    if (return_zeng_send_keys_onehost<0) return return_zeng_send_keys_onehost;


	return 1;
}

//Retorna <0 si error
int zeng_send_message(void)
{

	pending_zeng_send_message_footer=0;

    int i;

    for (i=0;i<zeng_total_remotes;i++) {

			int escritos=z_sock_write_string(zeng_remote_sockets[i],zeng_send_message_footer);

			//Si ha habido error al escribir en socket
			if (escritos<0) return escritos;

			else {
				//Leer hasta prompt
				int posicion_command;
				z80_byte buffer[200];
				int leidos=zsock_read_all_until_command(zeng_remote_sockets[i],buffer,199,&posicion_command);



				//Si ha habido error al leer de socket
				if (leidos<0) return leidos;
			}
    }

	return 1;

}



//Estructura para el pase de parametros de envio de threads de envio de snapshots
struct s_zeng_send_snapshot_uno_concreto {
    int finished;
    pthread_t thread;
};

struct s_zeng_send_snapshot_uno_concreto zeng_send_snapshot_uno_concreto_array[ZENG_MAX_REMOTE_HOSTS];

void *thread_zeng_send_snapshot_uno_concreto(void *p_indice_socket)
{

    //Este puntero realmente para mi es un entero, le paso el indice
    int indice_socket=(int) p_indice_socket;

    //printf("before zeng_send_snapshot\n");
    int error=zeng_send_snapshot(zeng_remote_sockets[indice_socket]);
    //printf("after zeng_send_snapshot\n");


    if (error<0) {
        return_zeng_send_snapshot_uno_concreto=1;
    }

    zeng_send_snapshot_uno_concreto_array[indice_socket].finished=1;

    return NULL;

}

void zeng_send_snapshot_uno_concreto(int indice_socket)
{

    if (pthread_create( &zeng_send_snapshot_uno_concreto_array[indice_socket].thread, NULL,
            &thread_zeng_send_snapshot_uno_concreto, (void *) indice_socket) ) {
        debug_printf(VERBOSE_ERR,"Can not create thread_zeng_send_snapshot_uno_concreto pthread");
        zeng_send_snapshot_uno_concreto_array[indice_socket].finished=1;
        return;
    }
	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(zeng_send_snapshot_uno_concreto_array[indice_socket].thread);


}

//int temp_memoria_asignada=0;
//int temp_memoria_liberada=0;

void *thread_zeng_function(void *nada GCC_UNUSED)
{
	/*
Hilo de sincronización de juego:

-si flag de envío de snapshot, se envía. Ese flag lo activa el core al final de frame, cada X segundos, y cuando somos el máster

-si hay que enviar mensaje al otro jugador, enviarlo

-ver la fifo usada en envío de eventos:
*tecla
*press/release

Dicha fifo hay que controlarla mediante semáforos
Se mete elementos en fifo cuando se llama a util send press/release
Se leen y envían eventos de la fifo desde este thread

-dormir durante 5ms - cuarto de frame

Para las rutinas zsock también haría falta semáforos pero como no voy a llamarla desde dos sitios distintos a la vez pues..

Poder enviar mensajes a otros jugadores
	 */


	//int escritos;
	//int leidos;

	//error conectando zeng. desactivarlo si se produce
	int error_desconectar=0;

	//TODO: controlar otros errores de envio de snapshot y mensaje. Al igual que se hace con send-keys

	while (1) {
		usleep(5000); //dormir 5 ms



		//Si hay tecla pendiente de enviar
		zeng_key_presses elemento;
		while (!zeng_fifo_read_element(&elemento) && !error_desconectar) {
			debug_printf (VERBOSE_DEBUG,"ZENG: Read event from zeng fifo and sending it to remote: key %d pressrelease %d",elemento.tecla,elemento.pressrelease);

            debug_printf (VERBOSE_DEBUG,"Info joystick: fire: %d up: %d down: %d left: %d right: %d",
                UTIL_KEY_JOY_FIRE,UTIL_KEY_JOY_UP,UTIL_KEY_JOY_DOWN,UTIL_KEY_JOY_LEFT,UTIL_KEY_JOY_RIGHT);

			//command> help send-keys-event
			//Syntax: send-keys-event key event
				int error=zeng_send_keys(&elemento);

				if (error<0) error_desconectar=1;

		}



		//Si hay mensaje pendiente de enviar
		if (pending_zeng_send_message_footer && !error_desconectar) {
			int error=zeng_send_message();

			if (error<0) {
				error_desconectar=1;

			}

		}



		//Si hay snapshot pendiente de enviar
		if (zeng_i_am_master && !error_desconectar) {
			if (zeng_send_snapshot_pending && zeng_send_snapshot_mem_hexa!=NULL) {
                int i;

                return_zeng_send_snapshot_uno_concreto=0;

                for (i=0;i<zeng_total_remotes;i++) {

                    //printf("before zeng_send_snapshot\n");

                    zeng_send_snapshot_uno_concreto_array[i].finished=0;

                    zeng_send_snapshot_uno_concreto(i);

                    //printf("after zeng_send_snapshot\n");


                    if (return_zeng_send_snapshot_uno_concreto) {
                        error_desconectar=1;
                    }


                }

                //zeng_send_snapshot_pending=0;

                //Esperar a que finalicen

                int finished;

                do {
                    finished=1;
                    for (i=0;i<zeng_total_remotes;i++) {
                        //printf("socket %d finished %d\n",i,zeng_send_snapshot_uno_concreto_array[i].finished);
                        if (zeng_send_snapshot_uno_concreto_array[i].finished==0) finished=0;
                        else {
                            //Si tiene valor 2, es que ya se ha liberado la memoria del thread
			    /*
                           if (zeng_send_snapshot_uno_concreto_array[i].finished!=2) {
                                   zeng_send_snapshot_uno_concreto_array[i].finished=2;
                                   //liberar memoria
                                   pthread_join(zeng_send_snapshot_uno_concreto_array[i].thread,NULL);
                           }
			   */
                        }
                    }
                    if (!finished) usleep(1000); //dormir 1 ms
                    //printf("Finished: %d\n",finished);
                } while (!finished && !error_desconectar);

                //printf("all snapshot sending finished. freeing snapshot memory\n\n");

                //solo liberar memoria si no ha habido error, porque si no corremos el riesgo de liberar
                //memoria que aun pueda estar usando algun thread de envio de snapshot
                if (!error_desconectar) {
                    free(zeng_send_snapshot_mem_hexa);
                    zeng_send_snapshot_mem_hexa=NULL;

                    //Esto liberarlo al final, asi le dice a la funcion que llena el snapshot que ya puede
                    //crear otro snapshot
                    zeng_send_snapshot_pending=0;

                    //temp_memoria_liberada++;
                    //printf("Asignada: %d liberada: %d\n",temp_memoria_asignada,temp_memoria_liberada);

                }

			}
		}


		if (error_desconectar) {
			if (!zeng_thread_send_not_show_error) debug_printf (VERBOSE_ERR,"Error sending to socket. Disabling ZENG");

			//Aqui cerramos el thread desde mismo dentro del thread
			zeng_disable_forced();

			//Parece que en Windows esto no es suficiente para salir del thread desde el mismo pthread. Hacemos un return
			//Por si acaso dejamos el return siempre, si es Windows u otro sistema que no haga caso del pthread_cancel,
			//pues volvera. Y si no, no llegara aqui
			//Y le damos un tiempo para que se cierre con cancel. Al menos asi en Mac no llegara aqui pues se cierra antes el pthread
			sleep(1);

			if (!zeng_thread_send_not_show_error) debug_printf(VERBOSE_ERR,"Error sending to socket. Disabling ZENG. Returning from thread after disabling it");
			return NULL;

		}
	}
}




void zeng_force_reconnect(void)
{

    zeng_thread_send_not_show_error=1;

    zeng_disable_forced();
    //Darle tiempo a que se cierre

    usleep(100000); //0.1 segundo

    zeng_enable();

    zeng_thread_send_not_show_error=0;
}

//Enviar estado actual de la maquina como snapshot a maquina remota


void zeng_send_snapshot_if_needed(void)
{

	if (zeng_enabled.v==0) return;

	if (zeng_i_am_master) {
		contador_envio_snapshot++;
		//printf ("%d %d\n",contador_envio_snapshot,(contador_envio_snapshot % (50*zeng_segundos_cada_snapshot) ));
		if (contador_envio_snapshot>=zeng_frames_video_cada_snapshot) {
                contador_envio_snapshot=0;
				//Si esta el anterior snapshot aun pendiente de enviar
				if (zeng_send_snapshot_pending) {
                    zeng_snapshots_not_sent++;
					debug_printf (VERBOSE_DEBUG,"ZENG: Last snapshot has not been sent yet. Total unsent: %d",zeng_snapshots_not_sent);

                    //Si llegado a un limite, reconectar. Suele suceder con ZENG en slave windows
                    //Sucede que se queda la operacion de send a socket que no acaba
                    /*
                    Aun con esto, parece que a veces va enviando snapshots pero el remoto no parece procesarlos,
                    con lo que aqui lo da como bueno y no incrementa el contador de retries
                    */
                    if (zeng_force_reconnect_failed_retries.v) {
                        if (zeng_snapshots_not_sent>=3*50) { //Si pasan mas de 3 segundos y no ha enviado aun el ultimo snapshot
                            debug_printf (VERBOSE_INFO,"ZENG: Forcing reconnect");
                            //printf("Before forcing ZENG reconnect\n");
                            zeng_force_reconnect();
                            //printf("After forcing ZENG reconnect\n");
                        }
                    }
				}
				else {
                    zeng_snapshots_not_sent=0;

					//zona de memoria donde se guarda el snapshot pero sin pasar a hexa
					z80_byte *buffer_temp;
					buffer_temp=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM); //16 MB es mas que suficiente

					if (buffer_temp==NULL) cpu_panic("Can not allocate memory for sending snapshot");

					int longitud;

  					save_zsf_snapshot_file_mem(NULL,buffer_temp,&longitud);


                    //temp_memoria_asignada++;
                    //printf("Asignada: %d liberada: %d\n",temp_memoria_asignada,temp_memoria_liberada);

					zeng_send_snapshot_mem_hexa=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2); //16 MB es mas que suficiente

					int char_destino=0;

					int i;

					for (i=0;i<longitud;i++,char_destino +=2) {
						sprintf (&zeng_send_snapshot_mem_hexa[char_destino],"%02X",buffer_temp[i]);
					}

					//metemos salto de linea y 0 al final
					strcpy (&zeng_send_snapshot_mem_hexa[char_destino],"\n");

					debug_printf (VERBOSE_DEBUG,"ZENG: Queuing snapshot to send, length: %d",longitud);


					//Liberar memoria que ya no se usa
					free(buffer_temp);


					zeng_send_snapshot_pending=1;


				}
		}
	}
}



void *zeng_enable_thread_function(void *nada GCC_UNUSED)
{

	zeng_enable_thread_running=1;



	//No hay pendiente snapshot
	zeng_send_snapshot_pending=0;

    //Y resetear la cuenta de snapshots no enviados
    zeng_snapshots_not_sent=0;

	//Conectar a remoto
	if (!zeng_connect_remotes()) {
		//Desconectar solo si el socket estaba conectado

		//if (zeng_remote_socket>=0)
        //Desconectar los que esten conectados
        zeng_disconnect_remote();

		zeng_enable_thread_running=0;
		return 0;
	}


	//Inicializar thread

	if (pthread_create( &thread_zeng, NULL, &thread_zeng_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng pthread");
		zeng_enable_thread_running=0;
		return 0;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(thread_zeng);


	zeng_enabled.v=1;


	zeng_enable_thread_running=0;

	return 0;

}



//Cancelar thread de conexion zeng
void zeng_cancel_connect(void)
{

	debug_printf(VERBOSE_DEBUG,"Cancelling ZENG connect");


		pthread_cancel(zeng_thread_connect);


	zeng_enable_thread_running=0;
}

void zeng_enable(void)
{

	//ya  inicializado
	if (zeng_enabled.v) return;

	if (zeng_remote_hostname[0]==0) return;



	//Inicializar thread

	if (pthread_create( &zeng_thread_connect, NULL, &zeng_enable_thread_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng connect pthread");
		return;
	}

	//y pthread en estado detached asi liberara su memoria asociada a thread al finalizar, sin tener que hacer un pthread_join
	pthread_detach(zeng_thread_connect);





}

//desactivar zeng sin cerrar socket pues ha fallado la conexion


void zeng_disable_normal(int forced)
{

	//ya  cerrado
	if (zeng_enabled.v==0) return;




	zeng_enabled.v=0;


	//Finalizar thread
	//printf ("antes de pthread_cancel\n");

	pthread_cancel(thread_zeng);

	//printf ("despues de pthread_cancel\n");


	//Vaciar fifo
	zeng_empty_fifo();

	//Decir que no hay snapshot pendiente
	zeng_send_snapshot_pending=0;

	//Cerrar conexión con ZRCP.
	if (!forced) {
		//TODO: enviarle un "quit"
		zeng_disconnect_remote();
	}
	else {
		//Liberar socket z_sock pero sin desconectarlo realmente
        int i;
        for (i=0;i<zeng_total_remotes;i++) {
		z_sock_free_connection(zeng_remote_sockets[i]);
        }
	}

    //#else
	//sin threads
	//zeng_enabled.v=0;



}

void zeng_disable(void)
{
	zeng_disable_normal(0);
}

void zeng_disable_forced(void)
{
	zeng_disable_normal(1);
}

void zeng_add_pending_send_message_footer(char *mensaje)
{

	if (!pending_zeng_send_message_footer) {
		sprintf(zeng_send_message_footer,"print-footer %s\n",mensaje);
		pending_zeng_send_message_footer=1;
		debug_printf (VERBOSE_DEBUG,"Queuing sending message to remote: %s",mensaje);
	}

}

#else

//Funciones sin pthreads. ZENG no se llama nunca cuando no hay pthreads, pero hay que crear estas funciones vacias
//para evitar errores de compilacion cuando no hay pthreads

void zeng_send_snapshot_if_needed(void)
{
}

void zeng_send_key_event(enum util_teclas tecla,int pressrelease)
{
}

void zeng_add_pending_send_message_footer(char *mensaje)
{
}

void zeng_cancel_connect(void)
{
}

void zeng_disable(void)
{
	zeng_enabled.v=0;
}

void zeng_enable(void)
{
}

int zeng_fifo_add_element(zeng_key_presses *elemento)
{
    return 0;
}

int zeng_fifo_read_element(zeng_key_presses *elemento)
{
    return 1;
}

#endif