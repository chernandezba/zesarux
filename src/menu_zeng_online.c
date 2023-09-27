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

void menu_zeng_join_room_aux(int room_number)
{


    //Lanzar el thread de join room
    zeng_online_client_join_room(room_number);

    contador_menu_zeng_connect_print=0;


    zxvision_simple_progress_window("ZENG Join Room", menu_zeng_online_join_room_cond,menu_zeng_online_join_room_print );

    if (zeng_online_client_join_room_thread_running) {
        menu_warn_message("Connection has not finished yet");
    }


}

//int contador_menu_zeng_online_list_rooms_print=0;


void menu_zeng_online_list_rooms_print(zxvision_window *w)
{
    char buf_temp[NETWORK_MAX_URL+256];
    sprintf(buf_temp,"Connecting to %s",zeng_online_server);

    menu_common_connect_print(w,buf_temp);
}



int menu_zeng_online_list_rooms_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_list_rooms_thread_running;
}



int menu_zeng_online_list_rooms(int *room_number,int *created,int *current_players,int *max_players,char *room_name)
{

    room_name[0]=0;

    //Lanzar el thread de listar rooms
    zeng_online_client_list_rooms();

    contador_menu_zeng_connect_print=0;


    zxvision_simple_progress_window("ZENG Online connection", menu_zeng_online_list_rooms_cond,menu_zeng_online_list_rooms_print );

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


        generic_message_tooltip_return retorno_ventana;
        zxvision_generic_message_tooltip("Rooms", 0,0, 0, 1, &retorno_ventana, 1, "%s", zeng_remote_list_rooms_buffer);

        //zxvision_generic_message_tooltip("About" , 0 ,0,0,0,&retorno_ventana,0,mensaje_about);
        //void zxvision_generic_message_tooltip(char *titulo, int return_after_print_text,int volver_timeout, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, int resizable, const char * texto_format , ...)
        //zxvision_generic_message_tooltip("Tape viewer" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser)

        //printf("Despues ventana rooms\n");
        //Si se sale con ESC
        if (retorno_ventana.estado_retorno==0) return -1;

        //Linea seleccionada es 1? quiere decir que se selecciona texto "--- edition"
    /*
        Por defecto, linea seleccionada es 0, incluso aunque no se haya habilitado linea de cursor, por ejemplo
        al buscar texto con f y n
        Como la que buscamos es la 1, no hay problema de falso positivo
    */
        int linea=retorno_ventana.linea_seleccionada;

        //printf("linea: %d\n",linea);
        printf("Texto seleccionado: [%s]\n",retorno_ventana.texto_seleccionado);


        //Parsear valoes
        /*

                escribir_socket(misocket,"N.  Created Players Max Name                           \n");

        for (i=0;i<zeng_online_current_max_rooms;i++) {
            escribir_socket_format(misocket,"%3d %d     %3d       %3d %s\n",
                i,
                zeng_online_rooms_list[i].created,
                zeng_online_rooms_list[i].current_players,
                zeng_online_rooms_list[i].max_players,
                zeng_online_rooms_list[i].name
            );
        }
        */

        //Si linea cabecera
        if (strstr(retorno_ventana.texto_seleccionado,"Created Players")!=NULL) {
            printf("Seleccionada cabecera. no hacer nada\n");
            return -1;

        }

        if (retorno_ventana.texto_seleccionado[0]==0) {
            printf("Seleccionada linea vacia. No hacer nada\n");
            return -1;
        }

        //TODO: si ventana poco ancho, puede seleccionar linea entre medio solo con espacios

        //Vamos a parsear 4 valores y luego una string
        int valores[4];
        int total_valores=0;

        int indice_string=0;

        int empezado_numero=-1;

        int salir=0;

        while (retorno_ventana.texto_seleccionado[indice_string]!=0 && !salir) {
            //Movernos al espacio siguiente

            //No ha empezado a leer un numero
            if (empezado_numero==-1) {
                if (retorno_ventana.texto_seleccionado[indice_string]!=' ') {
                    if (total_valores==4) salir=1;
                    else empezado_numero=indice_string;
                }
            }

            //Estaba leyendo un numero
            else {
                if (retorno_ventana.texto_seleccionado[indice_string]==' ') {
                    //fin numero
                    valores[total_valores]=atoi(&retorno_ventana.texto_seleccionado[empezado_numero]);
                    total_valores++;
                    empezado_numero=-1;
                }
            }

            if (!salir) indice_string++;
        }

        printf("Fin escaneo\n");
        int i;
        for (i=0;i<4;i++) {
            printf("%d: %d\n",i,valores[i]);
        }

        printf("4: [%s]\n",&retorno_ventana.texto_seleccionado[indice_string]);


        *room_number=valores[0];
        *created=valores[1];
        *current_players=valores[2];
        *max_players=valores[3];
        strcpy(room_name,&retorno_ventana.texto_seleccionado[indice_string]);

        return 0;
    }

    return -1;

}

void menu_zeng_online_create_room_print(zxvision_window *w)
{

    menu_common_connect_print(w,"Creating room");
}

int menu_zeng_online_create_room_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_online_client_create_room_thread_running;
}


void menu_zeng_online_create_room(MENU_ITEM_PARAMETERS)
{
    char texto_linea[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];


    int room_number,created,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    int retorno=menu_zeng_online_list_rooms(&room_number,&created,&current_players,&max_players,room_name);

    if (retorno>=0) {


        if (created==1) {
            debug_printf(VERBOSE_ERR,"Room is already created");
            return;
        }

        if (menu_ventana_scanf("Room name?",room_name,ZENG_ONLINE_MAX_ROOM_NAME+1)<0) {
            return;
        }

        //Create room


        //Lanzar el thread de crear room
        zeng_online_client_create_room(room_number,room_name);

        contador_menu_zeng_connect_print=0;


        zxvision_simple_progress_window("ZENG Online Create Room", menu_zeng_online_create_room_cond,menu_zeng_online_create_room_print );

        //join room
        menu_zeng_join_room_aux(room_number);
        //flag que indique a nivel global que somos master, no slave
        zeng_online_i_am_master.v=1;

        zeng_online_connected.v=1;

        //inicio thread de envio de snapshot, cada x milisegundos
        //zoc_start_snapshot_sending();

        //inicio thread de envio de teclas, cada x milisegundos
        //en principio esto lo enviamos desde envio de snapshot zoc_start_snapshot_sending
        //zoc_start_keys_sending();

        //inicio thread de recepcion de teclas, cada x milisegundos
        //zoc_start_keys_receiving();

        zoc_start_master_thread();

        //TODO: inicio thread de envio de eventos al pulsar teclas (igual en master que slave)
        //TODO: en menu no debe dejar crear room o join


    }
}

void menu_zeng_online_destroy_room(MENU_ITEM_PARAMETERS)
{
    //TODO
}

void menu_zeng_online_join_room(MENU_ITEM_PARAMETERS)
{
    char texto_linea[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];


    int room_number,created,current_players,max_players;
    char room_name[ZENG_ONLINE_MAX_ROOM_NAME+1];

    int retorno=menu_zeng_online_list_rooms(&room_number,&created,&current_players,&max_players,room_name);

    if (retorno>=0) {


        if (created==0) {
            debug_printf(VERBOSE_ERR,"Room is not created");
            return;
        }

        //join room
        menu_zeng_join_room_aux(room_number);

        //flag que indique a nivel global que somos slave , no master
        zeng_online_i_am_master.v=0;
        zeng_online_connected.v=1;

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


        //Create room + join / destroy room
        if (zeng_online_i_am_joined.v==0) {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_create_room,NULL,
            "~~Create room","~~Crear habitación","~~Crear habitació");
            menu_add_item_menu_shortcut(array_menu_common,'c');

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_join_room,NULL,
            "Jo~~in to room","Un~~irse a habitación","Un~~ir-se a habitació");
            menu_add_item_menu_shortcut(array_menu_common,'i');

        }
        else {
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,
            "Joined to room: ","Unido a habitación: ","Unit a habitació: ");
            menu_add_item_menu_sufijo_format(array_menu_common,"%d",zeng_online_joined_to_room_number);

            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_destroy_room,NULL,
            "~~Destroy room","~~Destruir habitación","~~Destruir habitació");
            menu_add_item_menu_shortcut(array_menu_common,'d');
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
