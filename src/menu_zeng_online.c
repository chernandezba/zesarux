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
- Al ser slave, empieza un thread de lectura de snapshot, cada x milisegundos
- Tambien se activa el thread de envio de eventos al pulsar teclas (igual en master que slave)
*/

int zeng_online_opcion_seleccionada=0;

z80_bit zeng_online_i_am_master={0};
z80_bit zeng_online_i_am_joined={0};
int zeng_online_joined_to_room_number=0;


char zeng_online_server[NETWORK_MAX_URL+1]="localhost";
int zeng_online_server_port=10000;

void menu_zeng_online_server(MENU_ITEM_PARAMETERS)
{
    //TODO
}

void menu_zeng_online_server_port(MENU_ITEM_PARAMETERS)
{
    //TODO
}


int contador_menu_zeng_online_list_rooms_print=0;


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

void menu_zeng_online_list_rooms(void)
{


    //Lanzar el thread de listar rooms
    zeng_online_client_list_rooms();

    contador_menu_zeng_connect_print=0;


    //strcpy(menu_zeng_connect_print_host,zeng_online_server);
    zxvision_simple_progress_window("ZENG Online connection", menu_zeng_online_list_rooms_cond,menu_zeng_online_list_rooms_print );



}



void menu_zeng_online_create_room(MENU_ITEM_PARAMETERS)
{
    menu_zeng_online_list_rooms();
}

void menu_zeng_online_destroy_room(MENU_ITEM_PARAMETERS)
{
    //TODO
}

void menu_zeng_online_join_room(MENU_ITEM_PARAMETERS)
{
    //TODO
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

        menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_server_port,NULL,
            "~~Port","~~Puerto","~~Port");
        menu_add_item_menu_prefijo_format(array_menu_common," [%d] ",zeng_online_server_port);
        menu_add_item_menu_shortcut(array_menu_common,'p');

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
            menu_add_item_menu_en_es_ca(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_online_destroy_room,NULL,
            "~~Destroy room","~~Destruir habitación","~~Destruir habitació");
            menu_add_item_menu_shortcut(array_menu_common,'d');
        }

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
