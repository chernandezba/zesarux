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
#include "stats.h"
#include "settings.h"


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
int zeng_online_restricted_keys_opcion_seleccionada=0;

//Puede ser: Master, Manager o Slave
char zeng_online_joined_as_text[30]="";




void menu_zeng_online_set_server(MENU_ITEM_PARAMETERS)
{
    menu_ventana_scanf("Server",zeng_online_server,NETWORK_MAX_URL+1);

}

void menu_zeng_online_nickname(MENU_ITEM_PARAMETERS)
{
    char buf_nickname[ZOC_MAX_NICKNAME_LENGTH+1];
    strcpy(buf_nickname,zeng_online_nickname);

    menu_ventana_scanf("Nickname",buf_nickname,ZOC_MAX_NICKNAME_LENGTH+1);

    //No permitir nickname con la palabra “error” en el nombre, porque
    //puede hacer que respuestas que respuestas que retornen su nombre, se interpreten como un error

    if (strcasestr(buf_nickname,"ERROR")!=NULL) {
        menu_error_message("Nickname not allowed");
        return;
    }

    strcpy(zeng_online_nickname,buf_nickname);
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

int menu_zeng_online_write_message_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_write_message_room_thread_running;
}

int menu_zeng_online_kick_user_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_kick_user_thread_running;
}

int menu_zeng_online_allow_message_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_allow_message_room_thread_running;
}

int menu_zeng_online_get_profile_keys_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_get_profile_keys_thread_running;
}

int menu_zeng_online_send_profile_keys_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_send_profile_keys_thread_running;
}

int menu_zeng_online_destroy_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_destroy_room_thread_running;
}

int menu_zeng_online_list_rooms_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_list_rooms_thread_running;
}

int menu_zeng_online_list_users_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_list_users_thread_running;
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


    zxvision_simple_progress_window("Get room list", menu_zeng_online_list_rooms_cond,menu_zeng_online_connecting_common_print );

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

void menu_zeng_online_ask_custom_permissions_send_message(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_ask_custom_permissions_value ^= ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE;
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

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

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

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_ask_custom_permissions_send_message,NULL,
            "Write Message","Escribir Mensaje","Escriure Missatge");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
            (menu_zeng_online_ask_custom_permissions_value & ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE ? 'X' : ' ')
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

int menu_zeng_online_ask_permissions(void)
{
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

    return permisos;
}

void menu_zeng_online_join_list(MENU_ITEM_PARAMETERS)
{


    //Lanzar el thread de obtener ultimo join pendiente
    zeng_online_client_join_list();

    //return;
    //pendiente con zeng_online_client_join_list_thread_running
    //buffer de zeng_remote_join_list_buffer

    contador_menu_zeng_connect_print=0;


    zxvision_simple_progress_window("Get join list", menu_zeng_online_join_list_cond,menu_zeng_online_connecting_common_print );

    if (zeng_online_client_join_list_thread_running) {
        menu_warn_message("Connection has not finished yet");
    }

    menu_generic_message_format("ZENG join list","%s",zeng_remote_join_list_buffer);

    //Si no vacio
    if (strcmp(zeng_remote_join_list_buffer,"<empty>")) {
        int permisos=menu_zeng_online_ask_permissions();


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

    strcpy(zeng_online_joined_as_text,"Master");
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

        menu_generic_message_splash("Create room","Created room and joined as master");

        zoc_show_bottom_line_footer_connected();


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

        menu_generic_message_splash("Rejoin as master","Rejoined as master");

        zoc_show_bottom_line_footer_connected();


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

        //Esta variable indica que hay que recibir snapshot inicial (si esta a 1)
        //como somos un manager, no queremos snapshot inicial
        //zoc_rejoining_as_master=0;

        //join room
        menu_zeng_join_room_aux(room_number,created_room_creator_password);
        //flag que indique a nivel global que somos master, no slave
        zeng_online_i_am_master.v=1;

        //Y me asigno permisos de nada, ni recibir snapshot ni envio de snapshot, ni de envio ni recepcion de teclas
        //solo manager, eso va implicito en el thread de master
        //created_room_user_permissions=ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT;
        created_room_user_permissions=ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE;

        zoc_start_master_thread();

        strcpy(zeng_online_joined_as_text,"Manager");


    }
}

void menu_zeng_online_list_rooms_menu_item(MENU_ITEM_PARAMETERS)
{

    int room_number,created,autojoin,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    //int retorno=
    menu_zeng_online_list_rooms(&room_number,&created,&autojoin,&current_players,&max_players,room_name);


}

//menu comun para pedir usuario y retornar un uuid
//ocultar_master vale 1 si no quieremos que aparezca el usuario que es el master
//agregar_none indica si quieremos agregar el None al final
//retorna 0 si sale con esc
int menu_zeng_online_ask_user_get_uuid(char *uuid,int ocultar_master,int agregar_none)
{
    //TODO: pedir esto desde la lista de usuarios, mas comodo
    //menu_ventana_scanf("UUID",allowed_keys_assigned[valor_opcion],STATS_UUID_MAX_LENGTH+1);


    //Lanzar el thread de listar users
    zeng_online_client_list_users();

    contador_menu_zeng_connect_print=0;


    zxvision_simple_progress_window("Get users list", menu_zeng_online_list_users_cond,menu_zeng_online_connecting_common_print );

    if (zeng_online_client_list_users_thread_running) {
        menu_warn_message("Connection has not finished yet");
    }

    if (zeng_remote_list_users_buffer[0]==0) {
        menu_error_message("Empty response");
        return 0;
    }


    //menu_generic_message("Joined users",zeng_remote_list_users_buffer);

    //Cada linea:
    //nickname
    //uuid
    //nickname
    //uuid
    //Por tanto saltamos la linea del uuid porque al usuario no le interesa

    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    int opcion_seleccionada=0;
    //do {

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        int i=0;
        int linea=0;
        int inicio_linea=0;

        while (zeng_remote_list_users_buffer[i]) {
            if (zeng_remote_list_users_buffer[i]=='\n') {
                zeng_remote_list_users_buffer[i]=0;

                int agregar=1;

                //Las dos primeras lineas es el nickname del master y su uuid
                if (ocultar_master && linea<2) agregar=0;

                if (agregar) {
                    //agregar en linea par el nickname
                    if ((linea%2)==0) {
                        menu_add_item_menu(array_menu_common,&zeng_remote_list_users_buffer[inicio_linea],MENU_OPCION_NORMAL,NULL,NULL);
                    }
                    else {
                        //uuid lo metemos como texto misc en opcion
                        menu_add_item_menu_misc(array_menu_common,&zeng_remote_list_users_buffer[inicio_linea]);
                    }

                }

                inicio_linea=i+1;
                linea++;
            }
            i++;
        }


        //Para poder desasignar
        if (agregar_none) {
            menu_add_item_menu(array_menu_common,"None",MENU_OPCION_NORMAL,NULL,NULL);
            menu_add_item_menu_misc(array_menu_common,"");
        }


        retorno_menu=menu_dibuja_menu(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Users");

    //No quedarnos en bucle como un menu. Al seleccionar usuario, se establece el uuid, y siempre que no se salga con ESC
    if ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus) {

        printf("Usuario %s UUID %s\n",item_seleccionado.texto_opcion,item_seleccionado.texto_misc);

        strcpy(uuid,item_seleccionado.texto_misc);
        return 1;

    }

    return 0;


}

void menu_zeng_online_list_users(MENU_ITEM_PARAMETERS)
{


    char buffer_uuid[STATS_UUID_MAX_LENGTH+1];

    //no hacemos nada con el uuid en este caso
    menu_zeng_online_ask_user_get_uuid(buffer_uuid,0,0);


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

        printf("Estado conexion: %d\n",zeng_online_connected.v);

        zoc_show_bottom_line_footer_connected();


    }
}

void menu_zeng_online_leave_room_master(MENU_ITEM_PARAMETERS)
{

    if (menu_confirm_yesno("Leave room?")) {

        menu_zeng_online_leave_room_master_aux();

        menu_generic_message_splash("Leave room","Left room");

        zoc_show_bottom_line_footer_connected(); //Para actualizar la linea de abajo del todo con texto ZEsarUX version bla bla

    }
}

void menu_zeng_online_autojoin_room(MENU_ITEM_PARAMETERS)
{

    //int permisos=menu_zeng_online_ask_custom_permissions();

    int permisos=menu_zeng_online_ask_permissions();

    zeng_online_client_autojoin_room(permisos);

    zxvision_simple_progress_window("Autojoin room", menu_zeng_online_autojoin_room_cond,menu_zeng_online_connecting_common_print );

}

void menu_zeng_online_disable_autojoin_room(MENU_ITEM_PARAMETERS)
{

    zeng_online_client_disable_autojoin_room();

    zxvision_simple_progress_window("Disable Autojoin room", menu_zeng_online_disable_autojoin_room_cond,menu_zeng_online_connecting_common_print );

}

void menu_zeng_online_write_message_room(MENU_ITEM_PARAMETERS)
{

    //Porque aqui se puede llegar desde un boton F/icono
    if (!zeng_online_connected.v) return;

    if (!(created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE)) {
        menu_error_message("You don't have permissions to send messages");
        return;
    }

    char message[ZENG_ONLINE_MAX_BROADCAST_MESSAGE_LENGTH+1];
    message[0]=0;

    if (menu_ventana_scanf("Message?",message,ZENG_ONLINE_MAX_BROADCAST_MESSAGE_LENGTH+1)<0) {
        return;
    }

    zeng_online_client_write_message_room(message);

    zxvision_simple_progress_window("Write message room", menu_zeng_online_write_message_room_cond,menu_zeng_online_connecting_common_print );

    //Cerrar menus. es una accion tipica que escribes un mensaje y quieres volver al momento al juego
    salir_todos_menus=1;

}

void menu_zeng_online_disallow_message_room(MENU_ITEM_PARAMETERS)
{

    zeng_online_client_allow_message_room(0);

    zxvision_simple_progress_window("Disallow messages", menu_zeng_online_allow_message_room_cond,menu_zeng_online_connecting_common_print );

}

void menu_zeng_online_allow_message_room(MENU_ITEM_PARAMETERS)
{

    zeng_online_client_allow_message_room(1);

    zxvision_simple_progress_window("Allow messages", menu_zeng_online_allow_message_room_cond,menu_zeng_online_connecting_common_print );

}

void menu_zeng_online_join_room(MENU_ITEM_PARAMETERS)
{

    if (zeng_online_nickname[0]==0) {
        debug_printf(VERBOSE_ERR,"You must set your nickname");
        return;
    }


    //char texto_linea[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];


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


        if (zeng_online_connected.v) {
            menu_generic_message_splash("Join room","Room joined");
            zoc_show_bottom_line_footer_connected();
            strcpy(zeng_online_joined_as_text,"Slave");
        }



    }
}

//Estructura para las teclas mostradas en el teclado de restricted keys
struct s_menu_zoc_keys_restricted {
    int tecla_marcada;
    int x,y; //posición de la tecla en el menu tabulado
    char texto[10]; //texto de la tecla
    enum util_teclas valor_tecla; //valor correspondiente de la tecla
};

struct s_menu_zoc_keys_restricted menu_zoc_keys_restricted[]={
    {0, 0,0,"ESC",UTIL_KEY_ESC},

    {0, 4,1,"1",'1'},
    {0, 4+2,1,"2",'2'},
    {0, 4+4,1,"3",'3'},
    {0, 4+6,1,"4",'4'},
    {0, 4+8,1,"5",'5'},
    {0, 4+10,1,"6",'6'},
    {0, 4+12,1,"7",'7'},
    {0, 4+14,1,"8",'8'},
    {0, 4+16,1,"9",'9'},
    {0, 4+18,1,"0",'0'},
    {0, 4+20,1,"BCK",UTIL_KEY_BACKSPACE},

    {0, 0,2,"TAB",UTIL_KEY_TAB},
    {0, 4,2,"Q",'q'},
    {0, 4+2,2,"W",'w'},
    {0, 4+4,2,"E",'e'},
    {0, 4+6,2,"R",'r'},
    {0, 4+8,2,"T",'t'},
    {0, 4+10,2,"Y",'y'},
    {0, 4+12,2,"U",'u'},
    {0, 4+14,2,"I",'i'},
    {0, 4+16,2,"O",'o'},
    {0, 4+18,2,"P",'p'},

    {0, 0,3,"CS",UTIL_KEY_CAPS_LOCK},
    {0, 4,3,"A",'a'},
    {0, 4+2,3,"S",'s'},
    {0, 4+4,3,"D",'d'},
    {0, 4+6,3,"F",'f'},
    {0, 4+8,3,"G",'g'},
    {0, 4+10,3,"H",'h'},
    {0, 4+12,3,"J",'j'},
    {0, 4+14,3,"K",'k'},
    {0, 4+16,3,"L",'l'},
    {0, 4+20,3,"ENT",UTIL_KEY_ENTER},

    {0, 0,4,"SH",UTIL_KEY_SHIFT_L},
    {0, 4,4,"Z",'z'},
    {0, 4+2,4,"X",'x'},
    {0, 4+4,4,"C",'c'},
    {0, 4+6,4,"V",'v'},
    {0, 4+8,4,"B",'b'},
    {0, 4+10,4,"N",'n'},
    {0, 4+12,4,"M",'m'},
    {0, 4+20,4,"SH",UTIL_KEY_SHIFT_R},

    {0, 0,5,"CTR",UTIL_KEY_CONTROL_L},
    {0, 4,5,"ALT",UTIL_KEY_ALT_L},
    {0, 4+4,5,"SPC",UTIL_KEY_SPACE},
    {0, 4+8,5,"ALT",UTIL_KEY_ALT_R},
    {0, 4+12,5,"CTR",UTIL_KEY_CONTROL_R},

    {0, 10,7,"JOY UP",UTIL_KEY_JOY_UP},
    {0, 3,8,"JOY LF",UTIL_KEY_JOY_LEFT},
    {0, 11,8,"FIRE",UTIL_KEY_JOY_FIRE},
    {0, 17,8,"JOY RG",UTIL_KEY_JOY_RIGHT},
    {0, 10,9,"JOY DN",UTIL_KEY_JOY_DOWN},


    //la del final, texto ""
    {0, 0,0,"",0}
};

//busca un id de tecla dentro de menu_zoc_keys_restricted
//retorna -1 si no encontrado
int menu_zeng_online_buscar_tecla(int tecla_buscar)
{
    int i;

    for (i=0;menu_zoc_keys_restricted[i].texto[0];i++) {
        int valor_tecla=menu_zoc_keys_restricted[i].valor_tecla;
        if (valor_tecla==tecla_buscar) return i;
    }

    return -1;
}

int menu_zoc_keys_restricted_get_marcadas(void)
{
    int i;
    int marcadas=0;

    for (i=0;menu_zoc_keys_restricted[i].texto[0];i++) {
        if (menu_zoc_keys_restricted[i].tecla_marcada) marcadas++;
    }

    return marcadas;
}




void menu_zeng_online_restricted_keys_assign_to(MENU_ITEM_PARAMETERS)
{

    char buffer_uuid[STATS_UUID_MAX_LENGTH+1];

    if (menu_zeng_online_ask_user_get_uuid(buffer_uuid,0,1)) {
        strcpy(allowed_keys_assigned[valor_opcion],buffer_uuid);
    }

}

void menu_zeng_online_kick_user(MENU_ITEM_PARAMETERS)
{

    char buffer_uuid[STATS_UUID_MAX_LENGTH+1];

    if (menu_zeng_online_ask_user_get_uuid(buffer_uuid,1,1)) {
        zeng_online_client_kick_user(buffer_uuid);
        zxvision_simple_progress_window("Kick user", menu_zeng_online_kick_user_cond,menu_zeng_online_connecting_common_print );
    }

}

void menu_zeng_online_restricted_keys_click(MENU_ITEM_PARAMETERS)
{
    //validar que no se marca mas de un maximo de ZOC_MAX_KEYS_ITEMS
    if (menu_zoc_keys_restricted[valor_opcion].tecla_marcada==0) {
        int total_marcadas=menu_zoc_keys_restricted_get_marcadas();
        if (total_marcadas>=ZOC_MAX_KEYS_ITEMS) {
            menu_error_message("Too many selected keys");
            return;
        }
    }


    menu_zoc_keys_restricted[valor_opcion].tecla_marcada ^=1;
}

#define MENU_ZOC_PROFILE_KEYBOARD_X 0
#define MENU_ZOC_PROFILE_KEYBOARD_Y 0
#define MENU_ZOC_PROFILE_KEYBOARD_ANCHO 30
#define MENU_ZOC_PROFILE_KEYBOARD_ALTO 12

void menu_zeng_online_restricted_keys_edit_keyboard(MENU_ITEM_PARAMETERS)
{

    //rellenar el teclado con los valores de int allowed_keys[ZOC_MAX_KEYS_PROFILES][ZOC_MAX_KEYS_ITEMS];
    int perfil_teclado=valor_opcion;

    int i;
    //primero inicializar todo a teclas no marcadas
    for (i=0;menu_zoc_keys_restricted[i].texto[0];i++) {
        menu_zoc_keys_restricted[i].tecla_marcada=0;
    }

    //buscar cada tecla recibida del servidor en nuestro teclado visual
    for (i=0;i<ZOC_MAX_KEYS_ITEMS && allowed_keys[perfil_teclado][i];i++) {
        int tecla=allowed_keys[perfil_teclado][i];
        int indice_en_teclado=menu_zeng_online_buscar_tecla(tecla);
        if (indice_en_teclado>=0) {
            menu_zoc_keys_restricted[indice_en_teclado].tecla_marcada=1;
        }
    }

    zxvision_window ventana;

    int ancho_ventana=MENU_ZOC_PROFILE_KEYBOARD_ANCHO;
    int alto_ventana=MENU_ZOC_PROFILE_KEYBOARD_ALTO;

    int x=menu_center_x()-ancho_ventana/2;
    int y=menu_center_y()-alto_ventana/2;

    zxvision_new_window(&ventana,x,y,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Allowed Keys");
    zxvision_draw_window(&ventana);

    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    int opcion_seleccionada=0;
    do {


        //Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);



        for (i=0;menu_zoc_keys_restricted[i].texto[0];i++) {


            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_restricted_keys_click,NULL,menu_zoc_keys_restricted[i].texto);
            menu_add_item_menu_tabulado(array_menu_common,menu_zoc_keys_restricted[i].x+1,menu_zoc_keys_restricted[i].y);
            menu_add_item_menu_marcar_opcion(array_menu_common,menu_zoc_keys_restricted[i].tecla_marcada);
            menu_add_item_menu_valor_opcion(array_menu_common,i);


        }

		//Nombre de ventana solo aparece en el caso de stdout
        retorno_menu=menu_dibuja_menu(&opcion_seleccionada,&item_seleccionado,array_menu_common,"OSD Adventure KB" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana

        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            //llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("Item seleccionado: %d\n",item_seleccionado.valor_opcion);
                //printf ("actuamos por funcion\n");


                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);


            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



    //printf ("en final de funcion\n");
    zxvision_destroy_window(&ventana);

    //cerrar esta ventana y no queremos que cierre todos los submenus
    salir_todos_menus=0;


    //Actualizar array de teclas que se le enviara al servidor con lo que hay marcado en nuestro teclado en pantalla
    int indice_destino=0;


    for (i=0;menu_zoc_keys_restricted[i].texto[0];i++) {
        if (menu_zoc_keys_restricted[i].tecla_marcada) {
            allowed_keys[perfil_teclado][indice_destino++]=menu_zoc_keys_restricted[i].valor_tecla;
        }
    }

    //0 del final si aun hay sitio
    if (indice_destino<ZOC_MAX_KEYS_ITEMS) allowed_keys[perfil_teclado][indice_destino++]=0;

}

char *menu_zeng_online_get_nickname_uuid(char *uuid)
{
    char *encontrado=strstr(zeng_remote_list_users_buffer,uuid);

    if (encontrado==NULL) return NULL;

    //Formato es
    //nickname
    //uuid
    //..

    //Por tanto retornar la linea anterior
    //Y no irnos mas alla del principio
    encontrado-=2; //Nos ponemos en la linea anterior (saltamos caracter actual y el \n de la linea anterior).

    //Vamos al inicio de la linea
    while (encontrado>=zeng_remote_list_users_buffer) {
        if (*encontrado=='\n') return encontrado+1;

        //si hemos llegado al principio
        if (encontrado==zeng_remote_list_users_buffer) return encontrado;

        encontrado--;
    }

    return NULL;
}

void menu_zeng_online_restricted_keys(MENU_ITEM_PARAMETERS)
{

    if (zeng_online_client_send_profile_keys_thread_running) {
        menu_error_message("Send thread has not finished yet");
        return;
    }

    //Obtener
    zeng_online_client_get_profile_keys();

    zxvision_simple_progress_window("Get keys", menu_zeng_online_get_profile_keys_cond,menu_zeng_online_connecting_common_print );



    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

        //Para poder hacer la conversion de uuid a nickname, obtener listado de usuarios
        //Lanzar el thread de listar users
        zeng_online_client_list_users();

        contador_menu_zeng_connect_print=0;


        zxvision_simple_progress_window("Get users list", menu_zeng_online_list_users_cond,menu_zeng_online_connecting_common_print );

        if (zeng_online_client_list_users_thread_running) {
            menu_warn_message("Connection has not finished yet");
        }

        menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

        int i;

        for (i=0;i<ZOC_MAX_KEYS_PROFILES;i++) {

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,
                "---Profile","---Perfil","---Perfil");
            menu_add_item_menu_sufijo_format(array_menu_common," %d--- ",i+1);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_restricted_keys_edit_keyboard,NULL,
                "Edit keys","Editar teclas","Editar tecles");
            menu_add_item_menu_valor_opcion(array_menu_common,i);


            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_restricted_keys_assign_to,NULL,
                "Assigned to: ","Asignado a: ","Assignat a: ");

            if (allowed_keys_assigned[i][0]==0) {
               menu_add_item_menu_sufijo(array_menu_common,"None");
            }

            else {

                //Obtener a que nickname corresponde ese uuid
                char *nickname=menu_zeng_online_get_nickname_uuid(allowed_keys_assigned[i]);
                if (nickname==NULL) {
                    //agregarlo tal cual como uuid
                    menu_add_item_menu_sufijo_format(array_menu_common,"UUID: %s",allowed_keys_assigned[i]);
                }
                else {
                    //Agregar como nickname
                    char buff_nickname[ZOC_MAX_NICKNAME_LENGTH+1];
                    int j=0;
                    while (*nickname!=0 && *nickname!='\n') {
                        buff_nickname[j++]=*nickname;

                        nickname++;
                    }
                    buff_nickname[j++]=0;
                    menu_add_item_menu_sufijo_format(array_menu_common,"%s",buff_nickname);
                }
            }

            menu_add_item_menu_valor_opcion(array_menu_common,i);

            menu_add_item_menu_separator(array_menu_common);

        }

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "---Close menu to apply to server---","---Cierra menu para aplicar al servidor---","--Tanca menu per aplicar al servidor---");
        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu(&zeng_online_restricted_keys_opcion_seleccionada,&item_seleccionado,array_menu_common,"Restricted keys");


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




    //Enviar teclas al salir del menu
    //Lo enviamos en background sin molestar al usuario con ventanas de progreso
    zeng_online_client_send_profile_keys();

}

int menu_zeng_online_not_connected_cond(void)
{
    return !zeng_online_connected.v;
}

void menu_zeng_online_zip_snapshot(MENU_ITEM_PARAMETERS)
{
    zeng_online_zip_compress_snapshots.v ^=1;
}


void menu_zeng_online_lag_indicator(MENU_ITEM_PARAMETERS)
{
    zeng_online_show_footer_lag_indicator.v ^=1;
}

void menu_zeng_online_allow_instant_keys(MENU_ITEM_PARAMETERS)
{
    zeng_online_allow_instant_keys.v ^=1;
}

void menu_zeng_online(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;
    do {


        menu_add_item_menu_en_es_ca_inicial(&array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_set_server,menu_zeng_online_not_connected_cond,
            "~~Server","~~Servidor","~~Servidor");
        char string_zeng_online_server[16];
        menu_tape_settings_trunc_name(zeng_online_server,string_zeng_online_server,16);
        menu_add_item_menu_prefijo_format(array_menu_common,"[%s] ",string_zeng_online_server);
        menu_add_item_menu_shortcut(array_menu_common,'s');

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_nickname,menu_zeng_online_not_connected_cond,
            "~~Nickname","~~Nickname","~~Nickname");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%s] ",zeng_online_nickname);
        menu_add_item_menu_shortcut(array_menu_common,'n');

        if (zeng_online_connected.v) {
            //Texto de connected con parpadeo
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
                "^^Connected^^ to server","^^Conectado^^ al servidor","^^Connectat^^ al servidor");
        }

        menu_add_item_menu_separator(array_menu_common);

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_list_rooms_menu_item,NULL,
            "List rooms","Listar habitaciones","Llistar habitacions");

        //Create room + join / destroy room
        if (zeng_online_connected.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_join_room,NULL,
            "Jo~~in to room","Un~~irse a habitación","Un~~ir-se a habitació");
            menu_add_item_menu_shortcut(array_menu_common,'i');

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_create_room,NULL,
            "~~Create room & Join as Master","~~Crear habitación & Unir como master","~~Crear habitació & Unir com master");
            menu_add_item_menu_shortcut(array_menu_common,'c');

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_rejoin_master,NULL,
            "Rejoin as Master","Reunir como master","Reunir com master");
            menu_add_item_menu_es_avanzado(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_rejoin_manager_master,NULL,
            "Rejoin as Manager","Reunir como manager","Reunir com manager");
            menu_add_item_menu_es_avanzado(array_menu_common);


        }
        else {

            menu_add_item_menu_separator(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "--Info--","--Info--","--Info--");

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "Joined to room: ","Unido a habitación: ","Unit a habitació: ");
            menu_add_item_menu_sufijo_format(array_menu_common,"%d",zeng_online_joined_to_room_number);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "Joined as: ","Unido como: ","Unit com: ");
            menu_add_item_menu_sufijo_format(array_menu_common,"%s",zeng_online_joined_as_text);
            menu_add_item_menu_es_avanzado(array_menu_common);

            if (zeng_online_i_am_master.v) {
                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
                "Creator password: ","Password creador: ","Password creador: ");
                menu_add_item_menu_sufijo_format(array_menu_common,"[%s]",created_room_creator_password);
                menu_add_item_menu_es_avanzado(array_menu_common);
            }


            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "My room permissions: ","Mis permisos habitación: ","Meus permisos habitació: ");
            menu_add_item_menu_sufijo_format(array_menu_common,"%s-%s-%s-%s-%s",
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT ? "RS" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT ? "WS" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_KEYS ? "RK" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS ? "WK" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE ? "WM" : "  ")
            );
            menu_add_item_menu_es_avanzado(array_menu_common);


            menu_add_item_menu_separator(array_menu_common);


            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_list_users,NULL,
            "List users","Listar usuarios","Llistar usuaris");
            menu_add_item_menu_es_avanzado(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_write_message_room,NULL,
                "Broadcast message","Mensaje difusión","Missatge difusió");

            if (zeng_online_i_am_master.v) {
                menu_add_item_menu_separator(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_join_list,NULL,
                "Join Waiting List","Lista de espera unirse","Llista d'espera unir-se");


                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_autojoin_room,NULL,
                "Set autojoin","Activar autounir","Activar autounir");
                menu_add_item_menu_es_avanzado(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_disable_autojoin_room,NULL,
                "Reset autojoin","Desactivar autounir","Desactivar autounir");
                menu_add_item_menu_es_avanzado(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_restricted_keys,NULL,
                "Restricted keys","Teclas restringidas","Tecles restringides");
                menu_add_item_menu_tiene_submenu(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_kick_user,NULL,
                "Kick user","Expulsar usuario","Expulsar usuari");

                menu_add_item_menu_separator(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_allow_message_room,NULL,
                "Allow broadcast messages","Permitir mensajes difusión","Permetre missatges difusió");
                menu_add_item_menu_es_avanzado(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_disallow_message_room,NULL,
                "Disallow broadcast messages","No permitir mensajes difusión","No permetre missatges difusió");
                menu_add_item_menu_es_avanzado(array_menu_common);

                menu_add_item_menu_separator(array_menu_common);
                menu_add_item_menu_es_avanzado(array_menu_common);


                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_leave_room_master,NULL,
                "Leave room","Abandonar habitación","Abandonar habitació");

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_destroy_room,NULL,
                "~~Destroy room","~~Destruir habitación","~~Destruir habitació");
                menu_add_item_menu_shortcut(array_menu_common,'d');


                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT) {
                    menu_add_item_menu_separator(array_menu_common);
                    menu_add_item_menu_es_avanzado(array_menu_common);
                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_zip_snapshot,NULL,
                    "Zip compress snapshot","Comprimir snapshot zip","Comprimir snapshot zip");
                    menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
                        (zeng_online_zip_compress_snapshots.v ? 'X' : ' '));
                    menu_add_item_menu_es_avanzado(array_menu_common);

                }

            }

            else {
                menu_add_item_menu_separator(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_leave_room_slave,NULL,
                "Leave room","Abandonar habitación","Abandonar habitació");

                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT) {
                    menu_add_item_menu_separator(array_menu_common);
                    menu_add_item_menu_es_avanzado(array_menu_common);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_lag_indicator,NULL,
                    "Footer lag indicator","Indicador lag en footer","Indicador lag al footer");
                    menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
                        (zeng_online_show_footer_lag_indicator.v ? 'X' : ' '));
                    menu_add_item_menu_es_avanzado(array_menu_common);

                }

                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS) {
                    menu_add_item_menu_separator(array_menu_common);
                    menu_add_item_menu_es_avanzado(array_menu_common);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_allow_instant_keys,NULL,
                    "Allow instant keys","Permitir teclas instantáneas","Permetre tecles instantànies");
                    menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
                        (zeng_online_allow_instant_keys.v ? 'X' : ' '));
                    menu_add_item_menu_tooltip(array_menu_common,"Key presses are sent instantly to the emulated machine, "
                        "instead of waiting for the event to go to the server and come back ");
                    menu_add_item_menu_tooltip(array_menu_common,"Key presses are sent instantly to the emulated machine, "
                        "instead of waiting for the event to go to the server and come back. This can be useful in some cases");

                    menu_add_item_menu_es_avanzado(array_menu_common);

                }


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
