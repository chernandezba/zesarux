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

    if (util_strcasestr(buf_nickname,"ERROR")!=NULL) {
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

void menu_zeng_online_get_snapshot_common_print(zxvision_window *w)
{

    menu_common_connect_print(w,"Getting snapshot");
}

void menu_zeng_online_stop_threads_common_print(zxvision_window *w)
{

    menu_common_connect_print(w,"Stopping threads");
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

int menu_zeng_online_stop_master_thread_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_stop_master_thread_thread_running;
}

//Contador por si el snapshot pedido nunca llega (por ejemplo que el master aun no lo haya enviado y de error al recibirlo)
int menu_zeng_online_get_snapshot_applied_cond_counter=0;

int menu_zeng_online_get_snapshot_applied_cond(zxvision_window *w GCC_UNUSED)
{


    menu_zeng_online_get_snapshot_applied_cond_counter+= ZXVISION_SIMPLE_PROGRESS_WINDOW_FRAMES_REFRESH;

    //en 5 segundos, timeout
    if (menu_zeng_online_get_snapshot_applied_cond_counter>5*50) return 1;

	return zeng_online_client_get_snapshot_applied_finished;
}

int menu_zeng_online_stop_slave_thread_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_stop_slave_thread_thread_running;
}

int menu_zeng_online_write_message_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_write_message_room_thread_running;
}

int menu_zeng_online_kick_user_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_kick_user_thread_running;
}

int menu_zeng_online_max_players_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_max_players_room_thread_running;
}

int menu_zeng_online_rename_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_rename_room_thread_running;
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
        if (menu_show_advanced_items.v) {
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

        }


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
                //printf("Primer item\n");
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

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Rooms");

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
        //zxvision_generic_message_tooltip("Rooms", 0,0, 0, 1, &retorno_ventana, 1, 0, "%s", zeng_remote_list_rooms_buffer);


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
        //printf("Texto seleccionado: [%s]\n",item_seleccionado.texto_opcion);



        //Si linea cabecera
        if (strstr(item_seleccionado.texto_opcion,"Created Players")!=NULL) {
            //printf("Seleccionada cabecera. no hacer nada\n");
            return -1;

        }

        if (item_seleccionado.texto_opcion[0]==0) {
            //printf("Seleccionada linea vacia. No hacer nada\n");
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

        //printf("Fin escaneo\n");

        /*for (i=0;i<ROOM_LIST_CAMPOS_LEER;i++) {
            printf("%d: %d\n",i,valores[i]);
        }*/

        //printf("%d: [%s]\n",i,&item_seleccionado.texto_opcion[indice_string]);


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

void menu_zeng_online_ask_custom_permissions_get_display(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_ask_custom_permissions_value ^= ZENG_ONLINE_PERMISSIONS_GET_DISPLAY;
}

void menu_zeng_online_ask_custom_permissions_get_audio(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_ask_custom_permissions_value ^= ZENG_ONLINE_PERMISSIONS_GET_AUDIO;
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

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_ask_custom_permissions_get_display,NULL,
            "Read Display","Leer Pantalla","Llegir Pantalla");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
            (menu_zeng_online_ask_custom_permissions_value & ZENG_ONLINE_PERMISSIONS_GET_DISPLAY ? 'X' : ' ')
        );

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_ask_custom_permissions_get_audio,NULL,
            "Read Audio","Leer Audio","Llegir Audio");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
            (menu_zeng_online_ask_custom_permissions_value & ZENG_ONLINE_PERMISSIONS_GET_AUDIO ? 'X' : ' ')
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

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Custom Permissions");


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
                permisos=ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT | ZENG_ONLINE_PERMISSIONS_GET_DISPLAY | ZENG_ONLINE_PERMISSIONS_GET_AUDIO | ZENG_ONLINE_PERMISSIONS_GET_KEYS;
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


        //printf("Permisos: %d\n",permisos);

        //Enviar comando de autorizacion con esos permisos
        zeng_online_client_authorize_join(permisos);
    }

    salir_todos_menus=1;

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

        //detectar que realmente se haya creado y no hacer join ni nada mas si no se ha creado
        if (zeng_online_room_was_created.v) {

            menu_zoc_join_master_aux(room_number);

            if (zeng_online_connected.v) {
                menu_generic_message_splash("Create room","Created room and joined as master");

                zoc_show_bottom_line_footer_connected();
            }

        }


    }

    salir_todos_menus=1;
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

    salir_todos_menus=1;
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

    salir_todos_menus=1;
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


        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&opcion_seleccionada,&item_seleccionado,array_menu_common,"Users");

    //No quedarnos en bucle como un menu. Al seleccionar usuario, se establece el uuid, y siempre que no se salga con ESC
    if ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus) {

        //printf("Usuario %s UUID %s\n",item_seleccionado.texto_opcion,item_seleccionado.texto_misc);

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

    salir_todos_menus=1;


}

void menu_zeng_online_leave_room_master_aux(void)
{



        //Detener el thread de master
        //zoc_stop_master_thread();
        zeng_online_client_stop_master_thread();
        zxvision_simple_progress_window("Stopping master thread", menu_zeng_online_stop_master_thread_cond,menu_zeng_online_stop_threads_common_print );

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
        //Al salir, en modo streaming, aplicamos un snapshot entero que se van recibiendo menos frecuentemente
        if (created_room_streaming_mode && zeng_online_i_am_master.v==0) {
            zeng_online_client_get_snapshot_applied_finished=0;

            //Obtenemos el ultimo snapshot
            //Nota: ese snapshot es el que envia el master cada 1 minuto, por tanto tampoco va a ser
            //un snapshot muy reciente, pero es mejor esta alternativa a la que teniamos antes, donde era el thread de slave
            //quien se traia el snapshot a cada minuto, esa manera de antes de hacerlo podia comportar dos cosas:
            //1. Si hace join y leave room en menos de 1 minuto, no se ha traido ningun snapshot
            //2. Entre el envio del snapshot del master (cada 1 minuto) y la recepcion de slave (cada 1 minuto), al final
            //el snapshot puede tener hasta 2 minutos de retraso

            //Le decimos que no aplique recepcion de streaming de video, no vaya a ser que entre
            //que desde que se recibe el snapshot y se hace leave, entre algo de streaming de video
            //y queda nuestra pantalla local algo inconsistente

            zoc_slave_forbid_apply_streaming_video=1;

            zoc_slave_set_force_get_snapshot();

            menu_zeng_online_get_snapshot_applied_cond_counter=0;
            zxvision_simple_progress_window("Getting last full snapshot", menu_zeng_online_get_snapshot_applied_cond,menu_zeng_online_get_snapshot_common_print );

            debug_printf(VERBOSE_INFO,"Apply last snapshot on leave");
            zeng_online_client_apply_pending_received_snapshot();
        }


        //Detener el thread de slave
        //zoc_stop_slave_thread();
        zeng_online_client_stop_slave_thread();
        zxvision_simple_progress_window("Stopping slave thread", menu_zeng_online_stop_slave_thread_cond,menu_zeng_online_stop_threads_common_print );


        zeng_online_client_leave_room();
        zxvision_simple_progress_window("Leave room", menu_zeng_online_leave_room_cond,menu_zeng_online_connecting_common_print );


        menu_generic_message_splash("Leave room","Left room");

        //printf("Estado conexion: %d\n",zeng_online_connected.v);

        zoc_show_bottom_line_footer_connected();

        //Importante siempre desbloquear esto, sino no se recibiria luego ningun streaming de video
        zoc_slave_forbid_apply_streaming_video=0;


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

    salir_todos_menus=1;

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

            //Si es modo streaming y no es maquina spectrum, cambiar a spectrum
            if (created_room_streaming_mode && !MACHINE_IS_SPECTRUM) {

                debug_printf(VERBOSE_INFO,"Changing to Spectrum machine because room has streaming enabled");

                current_machine_type=MACHINE_ID_SPECTRUM_48;

                set_machine(NULL);
            }
        }



    }

    salir_todos_menus=1;
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

    salir_todos_menus=1;

}

void menu_zeng_online_max_players_room(MENU_ITEM_PARAMETERS)
{

    //El valor actual no lo sabemos. Indicamos 10 por ejemplo
    int valor=10;

    menu_ventana_scanf_numero_enhanced("Max players",&valor,3,+1,1,ZENG_ONLINE_MAX_PLAYERS_PER_ROOM,0);

    zeng_online_client_max_players_room(valor);
    zxvision_simple_progress_window("Max players", menu_zeng_online_max_players_room_cond,menu_zeng_online_connecting_common_print);


}

void menu_zeng_online_rename_room(MENU_ITEM_PARAMETERS)
{

    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];


    strcpy(room_name,"<used>");

    if (menu_ventana_scanf("Room name?",room_name,ZENG_ONLINE_MAX_ROOM_NAME+1)<0) {
        return;
    }


    zeng_online_client_rename_room(room_name);
    zxvision_simple_progress_window("Rename room", menu_zeng_online_rename_room_cond,menu_zeng_online_connecting_common_print);


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
        retorno_menu=menu_dibuja_menu_no_title_lang(&opcion_seleccionada,&item_seleccionado,array_menu_common,"OSD Adventure KB" );


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

        retorno_menu=menu_dibuja_menu_dialogo_no_title_lang(&zeng_online_restricted_keys_opcion_seleccionada,&item_seleccionado,array_menu_common,"Restricted keys");


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

void menu_zeng_online_streaming_enabled_when_creating(MENU_ITEM_PARAMETERS)
{
    streaming_enabled_when_creating ^=1;
}



void menu_zeng_online_full_display_interval_autoadjust(MENU_ITEM_PARAMETERS)
{
    zoc_slave_differential_displays_limit_full_autoadjust.v ^=1;
}

//English, Spanish, Catalan
char *s_display_quality_perfect_en[]={"Perfect","Perfecta","Perfecta"};
char *s_display_quality_good_en[]={"Good","Buena","Bona"};
char *s_display_quality_medium_en[]={"Medium","Media","Mitja"};
char *s_display_quality_low_en[]={"Low","Baja","Baixa"};



char *get_menu_zoc_display_quality_string(int interval)
{

    int indice;

    if (gui_language==GUI_LANGUAGE_SPANISH) indice=1;
    else if (gui_language==GUI_LANGUAGE_CATALAN) indice=2;
    else indice=0;

    if (interval==0) return s_display_quality_perfect_en[indice];
    else if (interval<=5) return s_display_quality_good_en[indice];
    else if (interval<=10) return s_display_quality_medium_en[indice];
    else return s_display_quality_low_en[indice];

}

void menu_zeng_online_full_display_interval(MENU_ITEM_PARAMETERS)
{

    int opcion=menu_simple_five_choices("Interval?","One of:",
        get_menu_zoc_display_quality_string(0),
        get_menu_zoc_display_quality_string(2),
        get_menu_zoc_display_quality_string(10),
        get_menu_zoc_display_quality_string(20),
        "Custom");


    switch (opcion) {
        case 1:
            zoc_slave_differential_displays_limit_full=0;
        break;

        case 2:
            zoc_slave_differential_displays_limit_full=2;
        break;

        case 3:
            zoc_slave_differential_displays_limit_full=10;
        break;

        case 4:
            zoc_slave_differential_displays_limit_full=20;
        break;

        case 5:

            menu_ventana_scanf_numero_enhanced("Full display interval",&zoc_slave_differential_displays_limit_full,3,+1,0,50,0);

        break;



    }

}

void menu_zeng_online_streaming_silence_detection(MENU_ITEM_PARAMETERS)
{
    streaming_silence_detection ^=1;
}


//max_length sin contar el 0 del final
void menu_zoc_status_print_link_bar(char *destino,int max_length,int position_cursor,char caracter_cursor,char caracter_enlace)
{
    int i;

    for (i=0;i<max_length;i++) {
        if (i==position_cursor) destino[i]=caracter_cursor;
        else destino[i]=caracter_enlace;
    }
    destino[i]=0;
}


zxvision_window *menu_zeng_online_status_window_window;

#define ZOC_STATUS_LENGTH_STRING_LINK_BAR 20
#define ZOC_STATUS_MULTIPLIER_CURSOR 2



struct s_menu_zoc_status_common_link {
    int direccion_derecha;
    char caracter_cursor;

    int valor_variable_estadistica_anterior;
    int pos_cursor;
    int temporizador_enlace;
};

struct s_menu_zoc_status_common_link menu_zoc_status_vars_send_streaming_display={
    1,'>',
    0,0,0
};

struct s_menu_zoc_status_common_link menu_zoc_status_vars_send_streaming_audio={
    1,'>',
    0,0,0
};

struct s_menu_zoc_status_common_link menu_zoc_status_vars_send_snapshots={
    1,'>',
    0,0,0
};

struct s_menu_zoc_status_common_link menu_zoc_status_vars_received_streaming_display={
    0,'<',
    0,(ZOC_STATUS_LENGTH_STRING_LINK_BAR-1)*2,0
};

struct s_menu_zoc_status_common_link menu_zoc_status_vars_received_streaming_audio={
    0,'<',
    0,(ZOC_STATUS_LENGTH_STRING_LINK_BAR-1)*2,0
};

struct s_menu_zoc_status_common_link menu_zoc_status_vars_received_snapshots={
    0,'<',
    0,(ZOC_STATUS_LENGTH_STRING_LINK_BAR-1)*2,0
};



//Para barras de enlace que se desplazan a la izquierda y con multiplicador
//Como streaming displays, streaming audio etc
void menu_zoc_status_common_link(char *buffer_texto,int variable_estadistica,struct s_menu_zoc_status_common_link *vars)
{
    char caracter_enlace='-';


    //Ver diferencia entre contador anterior y actual
    int diff=variable_estadistica-(vars->valor_variable_estadistica_anterior);

    if (diff!=0) {
        //Aguantarlo hasta 1 segundo (50 frames) despues de que haya actividad
        //1 segundo suponiendo que esta ventana refresca 50 veces por segundo, que la hemos definido ya asi
        vars->temporizador_enlace=50;
    }

    int max_pos=ZOC_STATUS_LENGTH_STRING_LINK_BAR*ZOC_STATUS_MULTIPLIER_CURSOR;

    if (vars->direccion_derecha) {
        (vars->pos_cursor) +=diff;

        if (vars->pos_cursor>max_pos) {
            //Aparecer por la izquierda
            vars->pos_cursor -=max_pos;

            vars->pos_cursor %=max_pos;

        }
    }

    else {

        (vars->pos_cursor) -=diff;

        if (vars->pos_cursor<0) {
            //Aparecer por la derecha
            vars->pos_cursor +=max_pos;

            vars->pos_cursor %=max_pos;

        }
    }

    vars->valor_variable_estadistica_anterior=variable_estadistica;


    if (vars->temporizador_enlace) {
        vars->temporizador_enlace--;
        caracter_enlace='=';
    }

    menu_zoc_status_print_link_bar(buffer_texto,ZOC_STATUS_LENGTH_STRING_LINK_BAR,
        (vars->pos_cursor)/ZOC_STATUS_MULTIPLIER_CURSOR,vars->caracter_cursor,caracter_enlace);
}


struct s_menu_zoc_status_common_link_no_multiplier {
    int direccion_derecha;
    char caracter_cursor;

    int valor_variable_estadistica_anterior;
    int pos_cursor;
    int cantidad_movimiento;
    int temporizador_enlace;
};

struct s_menu_zoc_status_common_link_no_multiplier menu_zoc_status_vars_pending_authorization={
    0,'<',
    0,ZOC_STATUS_LENGTH_STRING_LINK_BAR-1,
    0,0
};

struct s_menu_zoc_status_common_link_no_multiplier menu_zoc_status_vars_keys_send={
    1,'>',
    0,0,
    0,0
};

struct s_menu_zoc_status_common_link_no_multiplier menu_zoc_status_vars_keys_received={
    0,'<',
    0,ZOC_STATUS_LENGTH_STRING_LINK_BAR-1,
    0,0
};

struct s_menu_zoc_status_common_link_no_multiplier menu_zoc_status_vars_alive_sent={
    1,'>',
    0,0,
    0,0
};

struct s_menu_zoc_status_common_link_no_multiplier menu_zoc_status_vars_broadcast_messages={
    0,'<',
    0,ZOC_STATUS_LENGTH_STRING_LINK_BAR-1,
    0,0
};

struct s_menu_zoc_status_common_link_no_multiplier menu_zoc_status_vars_send_snapshots_no_multiplier={
    1,'>',
    0,0,
    0,0
};


//Para barras de enlace que se desplazan izquierda/derecha sin multiplicador, como keys sent o broadcast messages received
//y que se quedan moviendo hasta que llegan a un extremo
void menu_zoc_status_common_link_no_multiplier(char *buffer_texto,int variable_estadistica,
    struct s_menu_zoc_status_common_link_no_multiplier *vars)
{
    char caracter_enlace='-';


    if (vars->cantidad_movimiento) {

        //Aguantarlo hasta 1 segundo (50 frames) despues de que haya actividad
        //1 segundo suponiendo que esta ventana refresca 50 veces por segundo, que la hemos definido ya asi
        vars->temporizador_enlace=50;

        if (vars->direccion_derecha) {
            //Mueve hacia la derecha
            (vars->pos_cursor)++;
            if ((vars->pos_cursor)==ZOC_STATUS_LENGTH_STRING_LINK_BAR) {
                //ha enviado una tecla. meter cursor a posicion inicial y decrementar teclas restantes
                (vars->cantidad_movimiento)--;
                vars->pos_cursor=0;
            }
        }

        else {
            //Mueve hacia la izquierda
            (vars->pos_cursor)--;
            if ((vars->pos_cursor)<0) {
                //ha enviado una tecla. meter cursor a posicion inicial y decrementar teclas restantes
                (vars->cantidad_movimiento)--;
                vars->pos_cursor=ZOC_STATUS_LENGTH_STRING_LINK_BAR-1;
            }
        }
    }


    if (variable_estadistica!=vars->valor_variable_estadistica_anterior) {
        //Activar movimiento
        //Decir cuantas teclas hay que enviar
        vars->cantidad_movimiento +=variable_estadistica-(vars->valor_variable_estadistica_anterior);

        //Limite visual de 10 en cola, para que no se quede mucho rato moviendose el cursor despues de haber liberado teclas
        if ((vars->cantidad_movimiento)>10) vars->cantidad_movimiento=10;

        //Contador anterior para saber cuando se envian nuevas
        vars->valor_variable_estadistica_anterior=variable_estadistica;
    }

    if (vars->temporizador_enlace) {
        vars->temporizador_enlace--;
        caracter_enlace='=';
    }

    menu_zoc_status_print_link_bar(buffer_texto,ZOC_STATUS_LENGTH_STRING_LINK_BAR,
        vars->pos_cursor,vars->caracter_cursor,caracter_enlace);
}




void menu_zeng_online_status_window_overlay(void)
{

    menu_speech_set_tecla_pulsada(); //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_zeng_online_status_window_window->is_minimized) return;


    //Print....
    //Tambien contar si se escribe siempre o se tiene en cuenta contador_segundo...

    zxvision_window *w=menu_zeng_online_status_window_window;

    if (zeng_online_connected.v==0) {
        zxvision_cls(w);
        zxvision_print_string_defaults_fillspc(w,1,0,"ZENG Online not connected");
    }
    else {
        //Quitar posible mensaje de not connected
        zxvision_print_string_defaults_fillspc(w,1,0,"");

        int linea=0;

        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Joined as: %s. Streaming mode: %s",
            zeng_online_joined_as_text,
            (created_room_streaming_mode ? "Yes" : "No")
        );

        if (zeng_online_i_am_master.v) {
            //Master
            if (created_room_streaming_mode) {
                //Modo streaming
                int porcentaje_full_displays=0;
                if (zoc_send_streaming_display_counter!=0) {
                    porcentaje_full_displays=(zoc_generated_full_displays_counter*100)/zoc_send_streaming_display_counter;
                }
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Streaming displays sent: %d",zoc_send_streaming_display_counter);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," Full: %d (%d %%)",zoc_generated_full_displays_counter,porcentaje_full_displays);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," Differential: %d (%d %%)",zoc_generated_differential_displays_counter,100-porcentaje_full_displays);

                int porcentaje_no_silence=0;
                if (zoc_send_streaming_audio_counter!=0) {
                    porcentaje_no_silence=(zoc_sent_streaming_audio_no_silence_counter*100)/zoc_send_streaming_audio_counter;
                }
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Streaming audios sent: %d",zoc_send_streaming_audio_counter);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," No Silence: %d (%d %%)",zoc_sent_streaming_audio_no_silence_counter,porcentaje_no_silence);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," Silence: %d (%d %%)",zoc_sent_streaming_audio_silence_counter,100-porcentaje_no_silence);
            }
            else {
                //Modo no streaming
                //Aqui vendria: envio de snapshots (cubierto mas abajo), envio de teclas (cubierto mas abajo)
            }

            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Snapshots sent: %d",zoc_sent_snapshots_counter);
            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Pending authorizations received: %d",zoc_get_pending_authorization_counter);
        }
        else {
            //Slave
            if (created_room_streaming_mode) {
                //Modo streaming
                int porcentaje_full_displays=0;
                if (zoc_streaming_display_received_counter!=0) {
                    porcentaje_full_displays=(zoc_streaming_display_full_received_counter*100)/zoc_streaming_display_received_counter;
                }
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Streaming displays received: %d",zoc_streaming_display_received_counter);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," Full: %d (%d %%)",zoc_streaming_display_full_received_counter,porcentaje_full_displays);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," Differential: %d (%d %%)",zoc_streaming_display_differential_received_counter,100-porcentaje_full_displays);

                int porcentaje_no_silence=0;
                if (zoc_streaming_audio_received_counter!=0) {
                    porcentaje_no_silence=(zoc_streaming_audio_no_silence_received_counter*100)/zoc_streaming_audio_received_counter;
                }
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Streaming audios received: %d",zoc_streaming_audio_received_counter);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," No Silence: %d (%d %%)",zoc_streaming_audio_no_silence_received_counter,porcentaje_no_silence);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++," Silence: %d (%d %%)",zoc_streaming_audio_silence_received_counter,100-porcentaje_no_silence);

            }
            else {
                //Modo no streaming
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Snapshots received: %d",zoc_received_snapshot_counter);
            }
        }

        //Solo envia teclas:
        //Slave (en modo streaming o no streaming)
        //Master (en modo no streaming)
        if (zeng_online_i_am_master.v==0 || (zeng_online_i_am_master.v && !created_room_streaming_mode)) {
            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Keys sent: %d",zoc_keys_send_counter);
        }

        //Solo recibe teclas:
        //Master (en modo streaming o no streaming)
        //Slave (en modo no streaming)
        if (zeng_online_i_am_master.v || (zeng_online_i_am_master.v==0 && !created_room_streaming_mode)) {
            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Keys received: %d",zoc_get_keys_counter);
        }
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Alive packets sent: %d",zoc_common_alive_user_send_counter);
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Broadcast Messages received: %d",zoc_common_get_messages_received_counter);

        //Linea en blanco. Lo hacemos asi por borrar posibles restos
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"");

        //
        //
        //Zona de barras de enlaces
        //
        //

        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Flows status:");
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"");

        char buffer_texto[ZOC_STATUS_LENGTH_STRING_LINK_BAR+1];
                                                                // 0123456789012345678901234567890123456
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"This ZEsarUX          ZENG Online Server",buffer_texto);
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"    |                          |",buffer_texto);
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"    |                          |",buffer_texto);

        if (zeng_online_i_am_master.v) {
            //Master
            if (created_room_streaming_mode) {
                //Modo streaming
                //Barra de streaming displays sent
                menu_zoc_status_common_link(buffer_texto,zoc_send_streaming_display_counter,&menu_zoc_status_vars_send_streaming_display);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Display  %s  |",buffer_texto);


                //Barra de streaming audios sent
                menu_zoc_status_common_link(buffer_texto,zoc_send_streaming_audio_counter,&menu_zoc_status_vars_send_streaming_audio);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Audio    %s  |",buffer_texto);

            }
            else {
                //Modo no streaming
                //Aqui vendria: envio de snapshots (cubierto mas abajo), envio de teclas (cubierto mas abajo)
            }


            //Barra de snapshots sent
            //Si es modo streaming, usamos movimiento como el de send keys, o sea, que se mueve de punta a punta cuando hay 1 envio
            if (created_room_streaming_mode) {
                menu_zoc_status_common_link_no_multiplier(buffer_texto,zoc_sent_snapshots_counter,&menu_zoc_status_vars_send_snapshots_no_multiplier);
            }
            else {
                menu_zoc_status_common_link(buffer_texto,zoc_sent_snapshots_counter,&menu_zoc_status_vars_send_snapshots);
            }
            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Snapshot %s  |",buffer_texto);


            //Barra de Pending authorizations received
            menu_zoc_status_common_link_no_multiplier(buffer_texto,zoc_get_pending_authorization_counter,&menu_zoc_status_vars_pending_authorization);
            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Pnd auth %s  |",buffer_texto);

        }
        else {
            //Slave
            if (created_room_streaming_mode) {

                //Barra de streaming displays received
                menu_zoc_status_common_link(buffer_texto,zoc_streaming_display_received_counter,&menu_zoc_status_vars_received_streaming_display);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Display  %s  |",buffer_texto);


                //Barra de streaming audios received
                menu_zoc_status_common_link(buffer_texto,zoc_streaming_audio_received_counter,&menu_zoc_status_vars_received_streaming_audio);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Audio    %s  |",buffer_texto);

            }
            else {
                //Modo no streaming

                //Barra de snapshots received
                menu_zoc_status_common_link(buffer_texto,zoc_received_snapshot_counter,&menu_zoc_status_vars_received_snapshots);
                zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Snapshot %s  |",buffer_texto);
            }
        }

        //Solo envia teclas:
        //Slave (en modo streaming o no streaming)
        //Master (en modo no streaming)

        if (zeng_online_i_am_master.v==0 || (zeng_online_i_am_master.v && !created_room_streaming_mode)) {
            //Keys sent
            menu_zoc_status_common_link_no_multiplier(buffer_texto,zoc_keys_send_counter,&menu_zoc_status_vars_keys_send);
            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Keyboard %s  |",buffer_texto);
        }

        //Solo recibe teclas:
        //Master (en modo streaming o no streaming)
        //Slave (en modo no streaming)
        if (zeng_online_i_am_master.v || (zeng_online_i_am_master.v==0 && !created_room_streaming_mode)) {

            //Keys received
            menu_zoc_status_common_link_no_multiplier(buffer_texto,zoc_get_keys_counter,&menu_zoc_status_vars_keys_received);
            zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Keyboard %s  |",buffer_texto);
        }

        //Alive packets
        menu_zoc_status_common_link_no_multiplier(buffer_texto,zoc_common_alive_user_send_counter,&menu_zoc_status_vars_alive_sent);
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Alive    %s  |",buffer_texto);


        //Broadcast messages
        menu_zoc_status_common_link_no_multiplier(buffer_texto,zoc_common_get_messages_received_counter,&menu_zoc_status_vars_broadcast_messages);
        zxvision_print_string_defaults_fillspc_format(w,1,linea++,"Messages %s  |",buffer_texto);

    }


    //Mostrar contenido
    zxvision_draw_window_contents(menu_zeng_online_status_window_window);

}




//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_zeng_online_status_window;


void menu_zeng_online_status_window(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();

    if (!menu_multitarea) {
        menu_warn_message("This window needs multitask enabled");
        return;
    }

    zxvision_window *ventana;
    ventana=&zxvision_window_zeng_online_status_window;

	//IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
	//si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
	//la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
	//zxvision_delete_window_if_exists(ventana);

    //Crear ventana si no existe
    if (!zxvision_if_window_already_exists(ventana)) {
        int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

        if (!util_find_window_geometry("zengonlinestatus",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
            ancho_ventana=43;
            alto_ventana=29;

            xventana=menu_center_x()-ancho_ventana/2;
            yventana=menu_center_y()-alto_ventana/2;
        }


        zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"ZENG Online Status",
            "zengonlinestatus",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

        ventana->can_be_backgrounded=1;

    }

    //Si ya existe, activar esta ventana
    else {
        zxvision_activate_this_window(ventana);
    }

	zxvision_draw_window(ventana);

	z80_byte tecla;


	int salir=0;


    menu_zeng_online_status_window_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_zeng_online_status_window_overlay);


    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
        //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
        return;
    }

    do {


		tecla=zxvision_common_getkey_refresh();


        switch (tecla) {

            //Salir con ESC
            case 2:
                salir=1;
            break;

            //O tecla background
            case 3:
                salir=1;
            break;
        }


    } while (salir==0);


	util_add_window_geometry_compact(ventana);

	if (tecla==3) {
		zxvision_message_put_window_background();
	}

	else {
		zxvision_destroy_window(ventana);
	}


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
        menu_add_item_menu_sufijo_format(array_menu_common," [%s]",string_zeng_online_server);
        menu_add_item_menu_shortcut(array_menu_common,'s');

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_nickname,menu_zeng_online_not_connected_cond,
            "~~Nickname","~~Nickname","~~Nickname");
        menu_add_item_menu_sufijo_format(array_menu_common," [%s]",zeng_online_nickname);
        menu_add_item_menu_shortcut(array_menu_common,'n');

        if (zeng_online_connected.v) {
            //Texto de connected con parpadeo
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
                "^^Connected^^ to server","^^Conectado^^ al servidor","^^Connectat^^ al servidor");
        }

        menu_add_item_menu_separator(array_menu_common);

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_list_rooms_menu_item,NULL,
            "List rooms","Listar habitaciones","Llistar habitacions");
        menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

        //Create room + join / destroy room
        if (zeng_online_connected.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_join_room,NULL,
            "Jo~~in to room","Un~~irse a habitación","Un~~ir-se a habitació");
            menu_add_item_menu_shortcut(array_menu_common,'i');
            menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_create_room,NULL,
            "~~Create room & Join as Master","~~Crear habitación & Unir como master","~~Crear habitació & Unir com master");
            menu_add_item_menu_shortcut(array_menu_common,'c');
            menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_rejoin_master,NULL,
            "Rejoin as Master","Reunir como master","Reunir com master");
            menu_add_item_menu_es_avanzado(array_menu_common);
            menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_rejoin_manager_master,NULL,
            "Rejoin as Manager","Reunir como manager","Reunir com manager");
            menu_add_item_menu_es_avanzado(array_menu_common);
            menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);


            menu_add_item_menu_separator(array_menu_common);
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
            menu_add_item_menu_sufijo_format(array_menu_common,"%s-%s-%s-%s-%s-%s-%s-%s-%s",
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT ? "RS" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT ? "WS" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_DISPLAY ? "RD" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_AUDIO ? "RA" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_DISPLAY ? "WD" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_AUDIO ? "WA" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_KEYS ? "RK" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS ? "WK" : "  "),
                (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_MESSAGE ? "WM" : "  ")
            );
            menu_add_item_menu_es_avanzado(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "Streaming mode:","Modo Streaming:","Mode Streaming:");

            menu_add_item_menu_sufijo_format(array_menu_common," %s",(created_room_streaming_mode ? "Yes" : "No"));
            menu_add_item_menu_es_avanzado(array_menu_common);


            menu_add_item_menu_separator(array_menu_common);


            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_list_users,NULL,
            "List users","Listar usuarios","Llistar usuaris");
            menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);
            menu_add_item_menu_es_avanzado(array_menu_common);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_write_message_room,NULL,
                "Broadcast message","Mensaje difusión","Missatge difusió");
            menu_add_item_menu_genera_ventana(array_menu_common);

            if (zeng_online_i_am_master.v) {
                menu_add_item_menu_separator(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_join_list,NULL,
                "Entry Waiting List","Lista de espera entrada","Llista d'espera d'entrada");
                menu_add_item_menu_genera_ventana(array_menu_common);


                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_autojoin_room,NULL,
                "Set autojoin","Activar autounir","Activar autounir");
                menu_add_item_menu_es_avanzado(array_menu_common);
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_disable_autojoin_room,NULL,
                "Reset autojoin","Desactivar autounir","Desactivar autounir");
                menu_add_item_menu_es_avanzado(array_menu_common);
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_rename_room,NULL,
                "Rename room","Renombrar habitación","Renombrar habitació");
                menu_add_item_menu_es_avanzado(array_menu_common);
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_restricted_keys,NULL,
                "Restricted keys","Teclas restringidas","Tecles restringides");
                menu_add_item_menu_es_avanzado(array_menu_common);
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_kick_user,NULL,
                "Kick user","Expulsar usuario","Expulsar usuari");
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_max_players_room,NULL,
                "Set max players per room","Definir max jugadores por hab.","Definir max jugadors per hab.");
                menu_add_item_menu_es_avanzado(array_menu_common);
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_separator(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_allow_message_room,NULL,
                "Allow broadcast messages","Permitir mensajes difusión","Permetre missatges difusió");
                menu_add_item_menu_es_avanzado(array_menu_common);
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_disallow_message_room,NULL,
                "Disallow broadcast messages","No permitir mensajes difusión","No permetre missatges difusió");
                menu_add_item_menu_es_avanzado(array_menu_common);
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_separator(array_menu_common);
                menu_add_item_menu_es_avanzado(array_menu_common);


                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_leave_room_master,NULL,
                "Leave room","Abandonar habitación","Abandonar habitació");
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_destroy_room,NULL,
                "~~Destroy room","~~Destruir habitación","~~Destruir habitació");
                menu_add_item_menu_shortcut(array_menu_common,'d');
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_status_window,NULL,
                "Status Window","Ventana de Estado","Finestra d'Estat");
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_PUT_SNAPSHOT) {

                    menu_add_item_menu_separator(array_menu_common);
                    menu_add_item_menu_es_avanzado(array_menu_common);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
                        "--------","--------","--------");
                    menu_add_item_menu_es_avanzado(array_menu_common);

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
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_status_window,NULL,
                "Status Window","Ventana de Estado","Finestra d'Estat");
                menu_add_item_menu_add_flags(array_menu_common,MENU_ITEM_FLAG_GENERA_VENTANA | MENU_ITEM_FLAG_SE_CERRARA);

                menu_add_item_menu_separator(array_menu_common);
                menu_add_item_menu_es_avanzado(array_menu_common);

                menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
                    "--------","--------","--------");
                menu_add_item_menu_es_avanzado(array_menu_common);

                menu_add_item_menu_separator(array_menu_common);
                menu_add_item_menu_es_avanzado(array_menu_common);



                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_SNAPSHOT) {
                    //menu_add_item_menu_separator(array_menu_common);
                    //menu_add_item_menu_es_avanzado(array_menu_common);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_lag_indicator,NULL,
                    "Footer lag/dropout indicator","Indicador lag/mal sonido en footer","Indicador lag/so dolent al footer");
                    menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
                        (zeng_online_show_footer_lag_indicator.v ? 'X' : ' '));
                    menu_add_item_menu_es_avanzado(array_menu_common);

                }

                if (created_room_streaming_mode && (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_GET_DISPLAY)) {
                    //menu_add_item_menu_separator(array_menu_common);
                    //menu_add_item_menu_es_avanzado(array_menu_common);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_full_display_interval_autoadjust,NULL,
                    "Autoadjust Display quality","Autoajustar Calidad pantalla","Autoajustar Qualitat pantalla");
                    menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
                        (zoc_slave_differential_displays_limit_full_autoadjust.v ? 'X' : ' ' ));


                    menu_add_item_menu_es_avanzado(array_menu_common);


                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_full_display_interval,NULL,
                    "Display quality","Calidad pantalla","Qualitat pantalla");
                    menu_add_item_menu_sufijo_format(array_menu_common," [%s] (%d)",
                        get_menu_zoc_display_quality_string(zoc_slave_differential_displays_limit_full),
                        zoc_slave_differential_displays_limit_full);
                    menu_add_item_menu_prefijo_format(array_menu_common,"    ");

                    menu_add_item_menu_tooltip(array_menu_common,"How many differential displays have to ask to get a full display frame");
                    menu_add_item_menu_ayuda(array_menu_common,"How many differential displays have to ask to get a full display frame. "
                        "\n"
                        "Example values:"
                        "\n"
                        "0:  will always get a full display, so no glitches on the screen. "
                        "Set it when you have high bandwidth and very low latency to the ZENG Online Server\n"
                        "\n"
                        "5:  will get a full display every 5 differential displays, some glitches on the screen when you don't have 50 FPS "
                        "(due to latency to the server or low bandwidth\n"
                        "\n"
                        "50: will get a full display every 50 differential displays, many glitches on the screen, set it if you have very low bandwidth"
                    );
                    menu_add_item_menu_es_avanzado(array_menu_common);

                }

                if (created_room_user_permissions & ZENG_ONLINE_PERMISSIONS_SEND_KEYS) {
                    //menu_add_item_menu_separator(array_menu_common);
                    //menu_add_item_menu_es_avanzado(array_menu_common);

                    menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_allow_instant_keys,NULL,
                    "Allow instant keys","Permitir teclas instantáneas","Permetre tecles instantànies");
                    menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
                        (zeng_online_allow_instant_keys.v ? 'X' : ' '));
                    menu_add_item_menu_tooltip(array_menu_common,"Key presses are sent instantly to the emulated machine, "
                        "instead of waiting for the event to go to the server and come back ");
                    menu_add_item_menu_ayuda(array_menu_common,"Key presses are sent instantly to the emulated machine, "
                        "instead of waiting for the event to go to the server and come back. This can be useful in some cases");

                    menu_add_item_menu_es_avanzado(array_menu_common);

                }


            }
        }


        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_streaming_enabled_when_creating,menu_zeng_online_not_connected_cond,
        "Streaming mode on create","Modo streaming al crear","Mode streaming al crear");
        menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
            (streaming_enabled_when_creating ? 'X' : ' '));
        menu_add_item_menu_tooltip(array_menu_common,"Stream mode runs a game only on the master and the slaves only receive display and audio. "
            "Only available on Spectrum");
        menu_add_item_menu_ayuda(array_menu_common,"Stream mode runs a game only on the master and the slaves only receive display and audio. "
            "Only available on Spectrum");
        menu_add_item_menu_es_avanzado(array_menu_common);

        if (zeng_online_connected.v && zeng_online_i_am_master.v && streaming_enabled_when_creating) {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_streaming_silence_detection,NULL,
            "Streaming silence detection","Detección silencio en Streaming","Detecció silenci en Streaming");
            menu_add_item_menu_prefijo_format(array_menu_common,"[%c] ",
                (streaming_silence_detection ? 'X' : ' '));
            menu_add_item_menu_tooltip(array_menu_common,"On master with streaming, do not send audio if there's silence");
            menu_add_item_menu_ayuda(array_menu_common,"On master with streaming, do not send audio if there's silence. "
                "Actually it sends audio but not a full audio frame, just a few bytes");
            menu_add_item_menu_es_avanzado(array_menu_common);

        }


        menu_add_item_menu_separator(array_menu_common);

        menu_add_ESC_item(array_menu_common);

        retorno_menu=menu_dibuja_menu_no_title_lang(&zeng_online_opcion_seleccionada,&item_seleccionado,array_menu_common,"ZENG Online");


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                //llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                        //printf ("actuamos por funcion\n");
                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}
