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
   Funciones menu de ZENG Online
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>



#include "zxvision.h"
#include "menu_items.h"
#include "compileoptions.h"
#include "screen.h"
#include "cpu.h"
#include "debug.h"
#include "menu_zeng_online.h"
#include "zeng_online_client.h"
#include "zeng_online.h"
#include "network.h"


/*
----ZENG online

Funcionamiento:

- El server ZENG online, se le tiene que hacer un "zengonline enable"
- Un ZEsarUX, el que hara de master, menu ZENG Online -> Create room, esto lo convierte en master. Se listan las habitaciones
y tiene que escoger una que este vacia. Esto hace un create-room (nos guardamos el creator_password), y un join-room (nos guardamos user_password)
- Al ser master, empieza un thread de envio de snapshot, cada x milisegundos
- Tambien se activa el thread de envio de eventos al pulsar teclas (igual en master que slave)

- Los clientes ZEsarUX, menu ZENG Online -> Join room. Esto nos convierte en slave. Nos guardamos el user_password
- Al ser slave, empieza un thread de lectura de snapshot, de manera continua recibe snaps
- Al ser slave, empieza un thread de lectura de keys, de manera continua recibe keys
- Tambien se activa el thread de envio de eventos al pulsar teclas (igual en master que slave)
*/

int zeng_online_opcion_seleccionada=0;





void menu_zeng_online_server(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf("Server",zeng_online_server,NETWORK_MAX_URL+1);

}

void menu_zeng_online_nickname(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf("Nickname",zeng_online_nickname,ZOC_MAX_NICKNAME_LENGTH+1);
}

void menu_zeng_online_join_room_print(zxvision_window *w)
{
    char buf_temp[NETWORK_MAX_URL+256];
    sprintf(buf_temp,"Connecting to %s",zeng_online_server);

    menu_common_connect_print(w,buf_temp);
}



int menu_zeng_online_join_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_join_room_thread_running;
}

void menu_zeng_join_room_aux(int room_number,char *creator_password)
{


    //Lanzar el thread de join room
    zeng_online_client_join_room(room_number,creator_password);

    contador_menu_zeng_connect_print=0;


    zxvision_simple_progress_window("ZENG Join Room", menu_zeng_online_join_room_cond,menu_zeng_online_join_room_print );

    if (zeng_online_client_join_room_thread_running) {
        menu_warn_message("Connection has not finished yet");
    }


}



void menu_zeng_online_connecting_common_print(zxvision_window *w)
{
    char buf_temp[NETWORK_MAX_URL+256];
    sprintf(buf_temp,"Connecting to %s",zeng_online_server);

    menu_common_connect_print(w,buf_temp);
}




int menu_zeng_online_leave_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_leave_room_thread_running;
}

int menu_zeng_online_autojoin_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_autojoin_room_thread_running;
}

int menu_zeng_online_disable_autojoin_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_disable_autojoin_room_thread_running;
}

int menu_zeng_online_destroy_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_destroy_room_thread_running;
}

int menu_zeng_online_list_rooms_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_list_rooms_thread_running;
}

int menu_zeng_online_join_list_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_join_list_thread_running;
}


int menu_zeng_online_authorize_join_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_authorize_join_thread_running;
}

int menu_zeng_online_list_rooms(int *room_number,int *created,int *autojoin,int *current_players,int *max_players,char *room_name)
{

    room_name[0]=0;

    //Lanzar el thread de listar rooms
    zeng_online_client_list_rooms();

    contador_menu_zeng_connect_print=0;


    zxvision_simple_progress_window("ZENG Online connection", menu_zeng_online_list_rooms_cond,menu_zeng_online_connecting_common_print );

    if (zeng_online_client_list_rooms_thread_running) {
        menu_warn_message("Connection has not finished yet");
    }

    if (zeng_remote_list_rooms_buffer[0]!=0) {
        //menu_generic_message("Rooms",zeng_remote_list_rooms_buffer);

        //Mostrar latencia
        long latencia=zeng_online_get_last_list_rooms_latency();

        char buffer_latencia[30];
        //Es en microsegundos. Si es >1000, mostrar en ms
        if (latencia>=1000) {
            sprintf(buffer_latencia,"%ld ms",latencia/1000);
        }
        else {
            sprintf(buffer_latencia,"%ld us",latencia);
        }

        menu_generic_message_format("ZENG online latency","Latency is: %s",buffer_latencia );


    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    int opcion_seleccionada=0;

    menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

    //Ir agregando lineas hasta final
    int i;
    int inicio_linea=0;

    int linea=0;

    for (i=0;zeng_remote_list_rooms_buffer[i];i++) {
        if (zeng_remote_list_rooms_buffer[i]=='\n') {
            zeng_remote_list_rooms_buffer[i]=0;
            //El primero es cabecera y lo metemos como tipo separador
            if (linea==0) {
                printf("Primer item\n");
                menu_add_item_menu(array_menu_common,&zeng_remote_list_rooms_buffer[inicio_linea],MENU_OPCION_SEPARADOR,NULL,NULL);
            }
            else {
                menu_add_item_menu(array_menu_common,&zeng_remote_list_rooms_buffer[inicio_linea],MENU_OPCION_NORMAL,NULL,NULL);
                //Al menos hay dos items de menu, por tanto opcion inicial es la segunda
                opcion_seleccionada=1;
            }

            inicio_linea=i+1;

            linea++;
        }
    }

    //Y el del final. Siempre que no sea texto en blanco
    if (zeng_remote_list_rooms_buffer[inicio_linea] && zeng_remote_list_rooms_buffer[inicio_linea]!='\n') {
        menu_add_item_menu(array_menu_common,&zeng_remote_list_rooms_buffer[inicio_linea],MENU_OPCION_NORMAL,NULL,NULL);
    }


        //menu_add_item_menu_separator(array_menu_common);

        //menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Rooms");

        //Si no seleccionada linea valida
        if (!((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0)) {
            return -1;

            /*
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                }
            */
        }

    //} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

//printf(item_seleccionado.texto_opcion);
        //generic_message_tooltip_return retorno_ventana;
        //zxvision_generic_message_tooltip("Rooms", 0,0, 0, 1, &retorno_ventana, 1, "%s", zeng_remote_list_rooms_buffer);


        //printf("Despues ventana rooms\n");
        //Si se sale con ESC
        //if (retorno_ventana.estado_retorno==0) return -1;

        //Linea seleccionada es 1? quiere decir que se selecciona texto "--- edition"
    /*
        Por defecto, linea seleccionada es 0, incluso aunque no se haya habilitado linea de cursor, por ejemplo
        al buscar texto con f y n
        Como la que buscamos es la 1, no hay problema de falso positivo
    */
        //int linea=retorno_ventana.linea_seleccionada;

        //printf("linea: %d\n",linea);
        printf("Texto seleccionado: [%s]\n",item_seleccionado.texto_opcion);



        //Si linea cabecera
        if (strstr(item_seleccionado.texto_opcion,"Created Players")!=NULL) {
            printf("Seleccionada cabecera. no hacer nada\n");
            return -1;

        }

        if (item_seleccionado.texto_opcion[0]==0) {
            printf("Seleccionada linea vacia. No hacer nada\n");
            return -1;
        }

        //TODO: si ventana poco ancho, puede seleccionar linea entre medio solo con espacios

        //Vamos a parsear 4 valores y luego una string
        #define ROOM_LIST_CAMPOS_LEER 5
        int valores[ROOM_LIST_CAMPOS_LEER];
        int total_valores=0;

        int indice_string=0;

        int empezado_numero=-1;

        int salir=0;

        while (item_seleccionado.texto_opcion[indice_string]!=0 && !salir) {
            //Movernos al espacio siguiente

            //No ha empezado a leer un numero
            if (empezado_numero==-1) {
                if (item_seleccionado.texto_opcion[indice_string]!=' ') {
                    if (total_valores==ROOM_LIST_CAMPOS_LEER) salir=1;
                    else empezado_numero=indice_string;
                }
            }

            //Estaba leyendo un numero
            else {
                if (item_seleccionado.texto_opcion[indice_string]==' ') {
                    //fin numero
                    valores[total_valores]=atoi(&item_seleccionado.texto_opcion[empezado_numero]);
                    total_valores++;
                    empezado_numero=-1;
                }
            }

            if (!salir) indice_string++;
        }

        printf("Fin escaneo\n");

        for (i=0;i<ROOM_LIST_CAMPOS_LEER;i++) {
            printf("%d: %d\n",i,valores[i]);
        }

        printf("%d: [%s]\n",i,&item_seleccionado.texto_opcion[indice_string]);


        *room_number=valores[0];
        *created=valores[1];
        *autojoin=valores[2];
        *current_players=valores[3];
        *max_players=valores[4];

        //Truncar nombre ventana hasta espacio final
        int inicio_nombre=strlen(item_seleccionado.texto_opcion)-1;
        for (i=inicio_nombre;i>=indice_string;i--) {
            if (item_seleccionado.texto_opcion[i]!=' ') {
                item_seleccionado.texto_opcion[i+1]=0;
                break;
            }
        }

        strcpy(room_name,&item_seleccionado.texto_opcion[indice_string]);

        return 0;
    }

    return -1;

}



int menu_zeng_online_ask_custom_permissions_value=0;
int menu_zeng_online_ask_custom_permissions_set_apply=0;

void menu_zeng_online_ask_custom_permissions_get_snapshot(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_ask_custom_permissions_value ^= ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT;
}

void menu_zeng_online_ask_custom_permissions_get_keys(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_ask_custom_permissions_value ^= ZENG_ONLINE_PERMISSIONS_GET_KEYS;
}


void menu_zeng_online_ask_custom_permissions_send_keys(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_ask_custom_permissions_value ^= ZENG_ONLINE_PERMISSIONS_SEND_KEYS;
}

void menu_zeng_online_ask_custom_permissions_apply(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_ask_custom_permissions_set_apply=1;
}

int menu_zeng_online_ask_custom_permissions(void)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    int opcion_seleccionada=0;

    //Inicializar siempre permisos a 0
    menu_zeng_online_ask_custom_permissions_value=0;

    menu_zeng_online_ask_custom_permissions_set_apply=0;

    do {

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,menu_zeng_online_ask_custom_permissions_get_snapshot,NULL);

        menu_add_item_menu_en_es_ca_inicial(&array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_ask_custom_permissions_get_snapshot,NULL,
            "Read Snapshot","Leer Snapshot","Llegir Snapshot");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
            (menu_zeng_online_ask_custom_permissions_value & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT ? 'X' : ' ')
        );

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_ask_custom_permissions_get_keys,NULL,
            "Read Keys","Leer Teclas","Llegir Tecles");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
            (menu_zeng_online_ask_custom_permissions_value & ZENG_ONLINE_PERMISSIONS_GET_KEYS ? 'X' : ' ')
        );

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_ask_custom_permissions_send_keys,NULL,
            "Write Keys","Escribir Teclas","Escriure Tecles");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
            (menu_zeng_online_ask_custom_permissions_value & ZENG_ONLINE_PERMISSIONS_SEND_KEYS ? 'X' : ' ')
        );
        menu_add_item_menu_separator(array_menu_common);

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_ask_custom_permissions_apply,NULL,
            "Apply","Aplicar","Aplicar");

        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Custom Permissions");


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus
                && !menu_zeng_online_ask_custom_permissions_set_apply);

    if (menu_zeng_online_ask_custom_permissions_set_apply) return menu_zeng_online_ask_custom_permissions_value;

    //Salir con ESC
    return -1;
}


void menu_zeng_online_join_list(MENU_ITEM_PARAMETERS)
{


    //Lanzar el thread de obtener ultimo join pendiente
    zeng_online_client_join_list();

    //return;
    //pendiente con zeng_online_client_join_list_thread_running
    //buffer de zeng_remote_join_list_buffer

    contador_menu_zeng_connect_print=0;


    zxvision_simple_progress_window("ZENG Online connection", menu_zeng_online_join_list_cond,menu_zeng_online_connecting_common_print );

    if (zeng_online_client_join_list_thread_running) {
        menu_warn_message("Connection has not finished yet");
    }

    menu_generic_message_format("ZENG join list","%s",zeng_remote_join_list_buffer);

    //Si no vacio
    if (strcmp(zeng_remote_join_list_buffer,"<empty>")) {
        int permisos=-1;
        while (permisos==-1) {
            int opcion=menu_simple_four_choices("Permissions","Kind","None","Read only","All","Custom");

            switch (opcion) {
                case 0: //ESC es 0 y por tanto ESC es lo mismo que no dar permisos
                case 1:
                    permisos=0;
                break;

                case 2:
                    permisos=ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT | ZENG_ONLINE_PERMISSIONS_GET_KEYS;
                break;

                case 3:
                    permisos=ZENG_ONLINE_PERMISSIONS_ALL_SLAVE;
                break;


                case 4:

                    //Devuelve -1 si salido con ESC
                    permisos=menu_zeng_online_ask_custom_permissions();


                break;
            }
        }

        printf("Permisos: %d\n",permisos);

        //Enviar comando de autorizacion con esos permisos
        zeng_online_client_authorize_join(permisos);
    }

}



void menu_zeng_online_create_room_print(zxvision_window *w)
{

    menu_common_connect_print(w,"Creating room");
}

int menu_zeng_online_create_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_create_room_thread_running;
}

void menu_zoc_join_master_aux(int room_number)
{
    //join room
    menu_zeng_join_room_aux(room_number,created_room_creator_password);
    //flag que indique a nivel global que somos master, no slave
    zeng_online_i_am_master.v=1;


    zoc_start_master_thread();
}

void menu_zeng_online_create_room(MENU_ITEM_PARAMETERS)
{
    if (zeng_online_nickname[0]==0) {
        debug_printf(VERBOSE_ERR,"You must set your nickname");
        return;
    }


    int room_number,created,autojoin,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    int retorno=menu_zeng_online_list_rooms(&room_number,&created,&autojoin,&current_players,&max_players,room_name);

    if (retorno>=0) {


        if (created==1) {
            debug_printf(VERBOSE_ERR,"Room is already created");
            return;
        }

        strcpy(room_name,"<used>");

        if (menu_ventana_scanf("Room name?",room_name,ZENG_ONLINE_MAX_ROOM_NAME+1)<0) {
            return;
        }

        //Create room


        //Lanzar el thread de crear room
        zeng_online_client_create_room(room_number,room_name);

        contador_menu_zeng_connect_print=0;


        zxvision_simple_progress_window("ZENG Online Create Room", menu_zeng_online_create_room_cond,menu_zeng_online_connecting_common_print );

        menu_zoc_join_master_aux(room_number);


    }
}

void menu_zeng_online_rejoin_master(MENU_ITEM_PARAMETERS)
{
    if (zeng_online_nickname[0]==0) {
        debug_printf(VERBOSE_ERR,"You must set your nickname");
        return;
    }


    int room_number,created,autojoin,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    int retorno=menu_zeng_online_list_rooms(&room_number,&created,&autojoin,&current_players,&max_players,room_name);

    if (retorno>=0) {


        if (created==0) {
            debug_printf(VERBOSE_ERR,"Room is not created");
            return;
        }


        if (menu_ventana_scanf("Creator password?",created_room_creator_password,ZENG_ROOM_PASSWORD_LENGTH+1)<0) {
            return;
        }

        if (created_room_creator_password[0]==0) {
            menu_error_message("Password can not be blank");
            return;
        }

        zoc_rejoining_as_master=1;
        menu_zoc_join_master_aux(room_number);


    }
}

//Unirse como un tipo de "manager" pero que no envia snapshots
void menu_zeng_online_rejoin_manager_master(MENU_ITEM_PARAMETERS)
{
    if (zeng_online_nickname[0]==0) {
        debug_printf(VERBOSE_ERR,"You must set your nickname");
        return;
    }


    int room_number,created,autojoin,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    int retorno=menu_zeng_online_list_rooms(&room_number,&created,&autojoin,&current_players,&max_players,room_name);

    if (retorno>=0) {


        if (created==0) {
            debug_printf(VERBOSE_ERR,"Room is not created");
            return;
        }


        if (menu_ventana_scanf("Creator password?",created_room_creator_password,ZENG_ROOM_PASSWORD_LENGTH+1)<0) {
            return;
        }

        if (created_room_creator_password[0]==0) {
            menu_error_message("Password can not be blank");
            return;
        }

        zoc_rejoining_as_master=1;

        //join room
        menu_zeng_join_room_aux(room_number,created_room_creator_password);
        //flag que indique a nivel global que somos master, no slave
        zeng_online_i_am_master.v=1;

        //Y me asigno permisos solo de recibir snapshot pero no de envio de snapshot, ni de envio ni recepcion de teclas
        created_room_user_permissions=ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT;

        zoc_start_master_thread();


    }
}

void menu_zeng_online_list_rooms_menu_item(MENU_ITEM_PARAMETERS)
{

    int room_number,created,autojoin,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    int retorno=menu_zeng_online_list_rooms(&room_number,&created,&autojoin,&current_players,&max_players,room_name);


}

void menu_zeng_online_leave_room_master_aux(void)
{



        //Detener el thread de slave
        zoc_stop_master_thread();

        zeng_online_client_leave_room();

        zxvision_simple_progress_window("Leave room", menu_zeng_online_leave_room_cond,menu_zeng_online_connecting_common_print );


}

void menu_zeng_online_destroy_room(MENU_ITEM_PARAMETERS)
{
    if (menu_confirm_yesno("Destroy room?")) {

        //Leave room
        menu_zeng_online_leave_room_master_aux();

        //Destroy room

        zeng_online_client_destroy_room();

        zxvision_simple_progress_window("Destroy room", menu_zeng_online_destroy_room_cond,menu_zeng_online_connecting_common_print );

        menu_generic_message_splash("Destroy room","Room destroyed");

    }
}

void menu_zeng_online_leave_room_slave(MENU_ITEM_PARAMETERS)
{

    if (menu_confirm_yesno("Leave room?")) {
        //Detener el thread de slave
        zoc_stop_slave_thread();


        zeng_online_client_leave_room();

        zxvision_simple_progress_window("Leave room", menu_zeng_online_leave_room_cond,menu_zeng_online_connecting_common_print );

        menu_generic_message_splash("Leave room","Left room");


    }
}

void menu_zeng_online_leave_room_master(MENU_ITEM_PARAMETERS)
{

    if (menu_confirm_yesno("Leave room?")) {

        menu_zeng_online_leave_room_master_aux();

        menu_generic_message_splash("Leave room","Left room");

    }
}

void menu_zeng_online_autojoin_room(MENU_ITEM_PARAMETERS)
{

    int permisos=menu_zeng_online_ask_custom_permissions();

    zeng_online_client_autojoin_room(permisos);

    zxvision_simple_progress_window("Autojoin room", menu_zeng_online_autojoin_room_cond,menu_zeng_online_connecting_common_print );

}

void menu_zeng_online_disable_autojoin_room(MENU_ITEM_PARAMETERS)
{

    zeng_online_client_disable_autojoin_room();

    zxvision_simple_progress_window("Disable Autojoin room", menu_zeng_online_disable_autojoin_room_cond,menu_zeng_online_connecting_common_print );

}

void menu_zeng_online_join_room(MENU_ITEM_PARAMETERS)
{

    if (zeng_online_nickname[0]==0) {
        debug_printf(VERBOSE_ERR,"You must set your nickname");
        return;
    }


    char texto_linea[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];


    int room_number,created,autojoin,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    int retorno=menu_zeng_online_list_rooms(&room_number,&created,&autojoin,&current_players,&max_players,room_name);

    if (retorno>=0) {


        if (created==0) {
            debug_printf(VERBOSE_ERR,"Room is not created");
            return;
        }

        //join room
        menu_zeng_join_room_aux(room_number,"");

        //flag que indique a nivel global que somos slave , no master
        zeng_online_i_am_master.v=0;
        //zeng_online_connected.v=1;

        //TODO: en menu no debe dejar crear room o join

        //inicio thread de recepcion de snapshot, cada x milisegundos
        //Esto en principio lo hacemos desde recepcion de teclas zoc_start_keys_receiving()
        //zoc_start_snapshot_receiving();

        //inicio thread de envio de teclas, cada x milisegundos
        //zoc_start_keys_sending();

        //inicio thread de recepcion de teclas, cada x milisegundos
        //zoc_start_keys_receiving();

        zoc_start_slave_thread();


    }
}

void menu_zeng_online(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


        menu_add_item_menu_en_es_ca_inicial(&array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_server,NULL,
            "~~Server","~~Servidor","~~Servidor");
        char string_zeng_online_server[16];
        menu_tape_settings_trunc_name(zeng_online_server,string_zeng_online_server,16);
        menu_add_item_menu_prefijo_format(array_menu_common,"[%s] ",string_zeng_online_server);
        menu_add_item_menu_shortcut(array_menu_common,'s');

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_nickname,NULL,
            "~~Nickname","~~Nickname","~~Nickname");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%s] ",zeng_online_nickname);
        menu_add_item_menu_shortcut(array_menu_common,'n');

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_list_rooms_menu_item,NULL,
            "List rooms","Listar habitaciones","Llistar habitacions");

        //Create room + join / destroy room
        if (zeng_online_connected.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_create_room,NULL,
            "~~Create room & Join as Master","~~Crear habitación & Unir como master","~~Crear habitació & Unir com master");
            menu_add_item_menu_shortcut(array_menu_common,'c');

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_rejoin_master,NULL,
            "Rejoin as Master","Reunir como master","Reunir com master");
            menu_add_item_menu_es_avanzado(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_rejoin_manager_master,NULL,
            "Rejoin as Manager","Reunir como manager","Reunir com manager");
            menu_add_item_menu_es_avanzado(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_join_room,NULL,
            "Jo~~in to room","Un~~irse a habitación","Un~~ir-se a habitació");
            menu_add_item_menu_shortcut(array_menu_common,'i');

        }
        else {

            menu_add_item_menu_separator(array_menu_common);


            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "Joined to room: ","Unido a habitación: ","Unit a habitació: ");
            menu_add_item_menu_sufijo_format(array_menu_common,"%d",zeng_online_joined_to_room_number);

            if (zeng_online_i_am_master.v) {
                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
                "Creator password: ","Password creador: ","Password creador: ");
                menu_add_item_menu_sufijo_format(array_menu_common,"[%s]",created_room_creator_password);
                menu_add_item_menu_es_avanzado(array_menu_common);
            }

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "My room permissions: ","Mis permisos habitación: ","Meus permisos habitació: ");

            //RS: Read snapshot
            //RK: Read keys
            //WK: Write keys

            menu_add_item_menu_sufijo_format(array_menu_common,"%s-%s-%s-%s",
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT ? "RS" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT ? "WS" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_KEYS ? "RK" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS ? "WK" : "  ")
            );



            if (zeng_online_i_am_master.v) {
                menu_add_item_menu_separator(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_join_list,NULL,
                "Join List","Lista join","Llista join");

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_autojoin_room,NULL,
                "Set autojoin","Activar autounir","Activar autounir");

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_disable_autojoin_room,NULL,
                "Reset autojoin","Desactivar autounir","Desactivar autounir");


                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_leave_room_master,NULL,
                "Leave room","Abandonar habitación","Abandonar habitació");

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_destroy_room,NULL,
                "~~Destroy room","~~Destruir habitación","~~Destruir habitació");
                menu_add_item_menu_shortcut(array_menu_common,'d');

            }

            else {
                menu_add_item_menu_separator(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_leave_room_slave,NULL,
                "Leave room","Abandonar habitación","Abandonar habitació");
            }
        }

        //TODO: desconectar con zeng_online_connected.v=0;

        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&zeng_online_opcion_seleccionada,&item_seleccionado,array_menu_common,"ZENG Online");


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}
