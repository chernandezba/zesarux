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
   Real joystick functions
*/


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef MINGW
#include <sys/ioctl.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "realjoystick.h"
#include "cpu.h"
#include "debug.h"
#include "joystick.h"
#include "zxvision.h"
#include "utils.h"
#include "screen.h"
#include "compileoptions.h"
#include "snap_ram.h"





//#define STRING_DEV_JOYSTICK "/dev/input/js0"

char string_dev_joystick[1024]="/dev/input/js0";

int realjoystick_index=0;

int ptr_realjoystick;

//Por defecto dice que esta habilitado. Luego cada driver (o el null) diran si realmente existe o no
z80_bit realjoystick_present={1};


//Si se desactiva por opcion de linea de comandos
z80_bit realjoystick_disabled={0};

//Si cada vez que se llama a smartload, se resetea tabla de botones a teclas
z80_bit realjoystick_clear_keys_on_smartload={0};


//si no usamos un joystick real sino un simulador.
int simulador_joystick=0;


//se activa evento del simulador con F8 en scrxwindows
//se puede forzar, por ejemplo, al redefinir tecla
//pero como al redefinir tecla, esta en un bucle esperando,
//se puede pulsar ahi F8 y se activara el forzado... y no hay que tenerlo forzado antes
int simulador_joystick_forzado=0;

//tecla que envia con la accion de numselect
//desde '0','1'.. hasta '9'.. y otras adicionales, como espacio
z80_byte realjoystick_numselect='1';



//Que funcion gestiona el inicio
int (*realjoystick_init)(void);
//Funcion de poll
void (*realjoystick_main)(void);

//Funcion que dice si se ha pulsado algo en el joystick
//int (*realjoystick_hit)(void);

int realjoystick_hit=0;


int realjoystick_total_buttons=0;
int realjoystick_total_axes=0;
char realjoystick_joy_name[REALJOYSTICK_MAX_NAME+1]=""; //+1 por si acaso no me acuerdo del 0 final...
char realjoystick_driver_name[REALJOYSTICK_MAX_DRIVER_NAME+1]="";


//Parametro de "autocalibrado". Valores de axis entre -VALOR y +VALOR, se consideran 0
//v>-realjoystick_autocalibrate_value && v<realjoystick_autocalibrate_value ->0
//De momento solo se usa en driver SDL. En nativo linux no usarlo: hasta ahora nunca ha hecho falta, mejor no meterlo
int realjoystick_autocalibrate_value=16383;


//No usar soporte nativo de real joystick de linux y dejar que use el del driver de video (actualmente solo SDL)
//Ponemos esto aqui como variable global y no dentro de realjoystick_linux
z80_bit no_native_linux_realjoystick={0};

//asignacion de botones de joystick a funciones
//posicion 0 del array es para REALJOYSTICK_EVENT_UP
//posicion 1 del array es para REALJOYSTICK_EVENT_DOWN
//etc...
realjoystick_events_keys_function realjoystick_events_array[MAX_EVENTS_JOYSTICK];

//asignacion de botones de joystick a letras de teclado
//la posicion dentro del array no indica nada
realjoystick_events_keys_function realjoystick_keys_array[MAX_KEYS_JOYSTICK];

char *realjoystick_event_names[]={
    "Up",
    "Down",
    "Left",
    "Right",
    "Fire",
    "EscMenu",
    "ExitEmulator",
    "Enter",
    "MenuTab",
    "Smartload",
	"Quicksave",
	"Osdkeyboard",
	"Osdtextkb",
	"NumSelect",
	"NumAction",
	"JoySelect",
	"Aux1",
	"Aux2",
	"Aux3",
	"Aux4",
    "Rewind",
    "FForward"
};

void realjoystick_print_event_keys(void)
{
	int i;

	for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
		printf ("%s ",realjoystick_event_names[i]);
	}
}

//Retorna numero de evento para texto evento indicado, sin tener en cuenta mayusculas
//-1 si no encontrado
int realjoystick_get_event_string(char *texto)
{

        int i;

        for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
			if (!strcasecmp(texto,realjoystick_event_names[i])) {
				debug_printf (VERBOSE_DEBUG,"Event %s has event number: %d",texto,i);
				return i;
			}
        }


	//error no encontrado
	debug_printf (VERBOSE_DEBUG,"Event %s unknown",texto);
	return -1;
}

//Retorna numero de boton y su tipo para texto dado
//tipo de boton: 0-boton normal, +1 axis positivo, -1 axis negativo
void realjoystick_get_button_string(char *texto, int *button,int *button_type)
{
	//Ver si hay signo
	if (texto[0]!='+' && texto[0]!='-') {
		*button=parse_string_to_number(texto);
		*button_type=0;
		debug_printf (VERBOSE_DEBUG,"Button/Axis %s is button number %d",texto,*button);
		return;
	}

	if (texto[0]=='+') *button_type=+1;
	else *button_type=-1;

	*button=parse_string_to_number(&texto[1]);

	debug_printf (VERBOSE_DEBUG,"Button/Axis %s is axis number %d and sign %d",texto,*button,*button_type);
}


void realjoystick_clear_keys_array(void)
{

	debug_printf (VERBOSE_INFO,"Clearing joystick to keys table");

	int i;
        for (i=0;i<MAX_KEYS_JOYSTICK;i++) {
                realjoystick_keys_array[i].asignado.v=0;
        }

}

void realjoystick_clear_events_array(void)
{

	debug_printf (VERBOSE_INFO,"Clearing joystick to events table");

        int i;
        for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
                realjoystick_events_array[i].asignado.v=0;
        }

}

void realjoystick_init_events_keys_tables(void)
{
	//eventos
	realjoystick_clear_events_array();

	//y teclas
	realjoystick_clear_keys_array();
}


void realjoystick_new_set_default_functions(void)
{
	//primero desasignamos todos
	//eventos
	realjoystick_init_events_keys_tables();


	//y luego asignamos algunos por defecto
	//se pueden redefinir por linea de comandos previo a esto, o luego por el menu
/*

Botones asignados por defecto:
Arriba, Abajo, Izquierda, Derecha: direcciones
Fire: Boton normal
EscMenu: Boton normal
Enter. Quickload: botones pequenyos
Numberselect, Numberaction: botones pequenyos
Aux1: Boton grande (left)
Osdkeyboard: Boton grande (right)
Aux2: desasignado

*/


	realjoystick_events_array[REALJOYSTICK_EVENT_UP].asignado.v=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_UP].button=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_UP].button_type=-1;

	realjoystick_events_array[REALJOYSTICK_EVENT_DOWN].asignado.v=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_DOWN].button=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_DOWN].button_type=+1;

        realjoystick_events_array[REALJOYSTICK_EVENT_LEFT].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_LEFT].button=0;
        realjoystick_events_array[REALJOYSTICK_EVENT_LEFT].button_type=-1;

        realjoystick_events_array[REALJOYSTICK_EVENT_RIGHT].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_RIGHT].button=0;
        realjoystick_events_array[REALJOYSTICK_EVENT_RIGHT].button_type=+1;

        realjoystick_events_array[REALJOYSTICK_EVENT_FIRE].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_FIRE].button=3;
        realjoystick_events_array[REALJOYSTICK_EVENT_FIRE].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_ESC_MENU].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_ESC_MENU].button=0;
        realjoystick_events_array[REALJOYSTICK_EVENT_ESC_MENU].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_ENTER].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_ENTER].button=8;
        realjoystick_events_array[REALJOYSTICK_EVENT_ENTER].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].button=9;
        realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].button_type=0;
	//normalmente las acciones no de axis van asociadas a botones... pero tambien pueden ser a axis
	//realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].button_type=-1;

        realjoystick_events_array[REALJOYSTICK_EVENT_OSDKEYBOARD].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_OSDKEYBOARD].button=5;
        realjoystick_events_array[REALJOYSTICK_EVENT_OSDKEYBOARD].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERSELECT].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERSELECT].button=2;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERSELECT].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERACTION].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERACTION].button=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERACTION].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_AUX1].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_AUX1].button=4;
        realjoystick_events_array[REALJOYSTICK_EVENT_AUX1].button_type=0;

	//En mi joystick de test me faltan botones para poder asignar tambien este
        //realjoystick_events_array[REALJOYSTICK_EVENT_AUX2].asignado.v=1;
        //realjoystick_events_array[REALJOYSTICK_EVENT_AUX2].button=5;
        //realjoystick_events_array[REALJOYSTICK_EVENT_AUX2].button_type=0;


	//prueba
        //realjoystick_keys_array[0].asignado.v=1;
        //realjoystick_keys_array[0].button=20;
        //realjoystick_keys_array[0].button_type=0;
	//realjoystick_keys_array[0].caracter='a';



}


int realjoystick_null_init(void)
{
	//No inicializa nada. Salir y decir que no hay joystick
	return 1;
}

//Null se encarga de driver de joystick cuando no hay joystick pero tambien de gestionar el simulador de joystick
void realjoystick_null_main(void)
{
	if (realjoystick_present.v==0) return;
	//printf ("realjoystick_null_main\n");



	//El null al final le hacemos que desactive el joystick, para que no aparezca joystick en menu
	//El tema es que se podria hacer cuando se llama a null_init, pero no se llama a realjoystick_null_init
	//dado que el init del joystick lo tiene que hacer el driver de video (caso de sdl ejemplo), o el linux joystick nativo
	//en caso del null no llama nadie al init
	debug_printf (VERBOSE_DEBUG,"Disabling joystick support as we are using the default null driver");
	realjoystick_present.v=0;


}


int realjoystick_simulador_init(void)
{
	debug_printf (VERBOSE_DEBUG,"realjoystick_simulador_init");

	strcpy(realjoystick_joy_name,"Joystick simulator");
	realjoystick_total_axes=255;
	realjoystick_total_buttons=255;
	strcpy(realjoystick_driver_name,"Simulator");
	return 0;
}


void read_simulador_joystick(void)
{
	simulador_joystick_forzado=0;

	int value,type,button;

	printf ("Button number: ");
	scanf ("%d",&button);
	if (button<0 || button>255) {
		printf ("Invalid button number\n");
		return;
	}

	printf ("Button type: (%d=button, %d=axis)",REALJOYSTICK_INPUT_EVENT_BUTTON,REALJOYSTICK_INPUT_EVENT_AXIS);
	scanf ("%d",&type);

	printf ("Button value: ");
	scanf ("%d",&value);

	if (value<-32767 || value>32767) {
		printf ("Invalid value\n");
		return;
	}

	printf ("OK simulating joystick button/axis: button: %d type: %d value: %d\n",button,type,value);

	realjoystick_hit=1;

	menu_info_joystick_last_raw_value=value;

	realjoystick_common_set_event(button,type,value);






}


//Null se encarga de driver de joystick cuando no hay joystick pero tambien de gestionar el simulador de joystick
void realjoystick_simulador_main(void)
{
	if (realjoystick_present.v==0) return;
	//printf ("realjoystick_simulador_main\n");

	if (simulador_joystick_forzado) {
		read_simulador_joystick();
	}

}





//Retorna indice o -1 si no encontrado
int realjoystick_find_event_or_key(int indice_inicial,realjoystick_events_keys_function *tabla,int maximo,int button,int type,int value)
{
        int i;
        for (i=indice_inicial;i<maximo;i++) {
		if (tabla[i].asignado.v==1) {
			if (tabla[i].button==button) {

				//boton normal. no axis
				if (type==REALJOYSTICK_INPUT_EVENT_BUTTON && tabla[i].button_type==0) return i;


				if (type==REALJOYSTICK_INPUT_EVENT_AXIS) {

					//ver si coindice el axis
					if (tabla[i].button_type==+1) {
						if (value>0) return i;
					}

                                	if (tabla[i].button_type==-1) {
                                        	if (value<0) return i;
	                                }

					//si es 0, representara poner 0 en eje izquierdo y derecho por ejemplo

					if
					(
						(tabla[i].button_type==+1 || tabla[i].button_type==-1)
						&& value==0
					)
					return i;

					//if (value==0) return i;
				}
			}

		}
        }

	return -1;

}

int realjoystick_find_event(int indice_inicial,int button,int type,int value)
{
	return realjoystick_find_event_or_key(indice_inicial,realjoystick_events_array,MAX_EVENTS_JOYSTICK,button,type,value);
}

int realjoystick_find_key(int indice_inicial,int button,int type,int value)
{
        return realjoystick_find_event_or_key(indice_inicial,realjoystick_keys_array,MAX_KEYS_JOYSTICK,button,type,value);
}

//Busca evento en tabla segun numero de boton y tipo
int realjoystick_buscar_evento_en_tabla(int button, int button_type)
{
/*
	//numero de boton
	int button;

	//tipo de boton: 0-boton normal, +1 axis positivo, -1 axis negativo
	int button_type;
*/
//realjoystick_events_keys_function realjoystick_events_array[MAX_EVENTS_JOYSTICK];
	int i;

	for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
		if (realjoystick_events_array[i].asignado.v &&
			realjoystick_events_array[i].button==button && realjoystick_events_array[i].button_type==button_type) return i;
	}

	return -1;
}


void realjoystick_print_char(z80_byte caracter)
{

	//if (menu_footer==0) return;

	char buffer_mensaje[33];


	//si ya hay second layer (quiza fijo) no hacerlo por contador
	/*
	if (menu_footer==0) {
		menu_second_layer_counter=seconds;
		enable_second_layer();
		menu_overlay_activo=1;
	}
	*/

	sprintf (buffer_mensaje,"Key: %c",caracter);

	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);


}

void realjoystick_print_joyselect(void)
{
	char buffer_mensaje[64];
	sprintf (buffer_mensaje,"Set joystick type: %s",joystick_texto[joystick_emulation]);
	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);


}

//si value=0, es reset
//si value != no, es set
void realjoystick_set_reset_key(int index,int value)
{

	z80_byte tecla=realjoystick_keys_array[index].caracter;

	//printf ("key: %c\n",tecla);

	if (value) {
		debug_printf (VERBOSE_DEBUG,"set key %c",tecla);
		ascii_to_keyboard_port(tecla);
	}


        else {
		debug_printf (VERBOSE_DEBUG,"reset key %c",tecla);
		ascii_to_keyboard_port_set_clear(tecla,0);
	}

}

void realjoystick_send_f_function(int accion)
{
		//printf ("pulsada tecl de funcion\n");

      menu_button_f_function.v=1;
      menu_button_f_function_action=accion;
      menu_abierto=1;

}

//si value=0, es reset
//si value != no, es set
void realjoystick_set_reset_action(int index,int value)
{

	switch (index) {
		case REALJOYSTICK_EVENT_UP:
			if (value) joystick_set_up(1);
			else joystick_release_up(1);
		break;

		case REALJOYSTICK_EVENT_DOWN:
			 if (value) joystick_set_down(1);
			 else joystick_release_down(1);
		break;

		case REALJOYSTICK_EVENT_LEFT:
			if (value) joystick_set_left(1);
			else joystick_release_left(1);
		break;

		case REALJOYSTICK_EVENT_RIGHT:
			if (value) joystick_set_right(1);
			else joystick_release_right(1);
		break;

		case REALJOYSTICK_EVENT_FIRE:
			if (value) joystick_set_fire(1);
			else joystick_release_fire(1);
		break;


		//Evento de ESC representa ESC para navegar entre menus y tambien abrir el menu (lo que ahora es con F5 y antes era ESC)
		//NO activa ESC de Z88
		case REALJOYSTICK_EVENT_ESC_MENU:
            if (value) {
                    puerto_especial1 &=255-1;
                    if (util_if_open_just_menu() ) menu_fire_event_open_menu();
            }
            else {
                    puerto_especial1 |=1;
            }

		break;

		case REALJOYSTICK_EVENT_EXIT_EMULATOR:
			if (value) realjoystick_send_f_function(F_FUNCION_EXITEMULATOR);
		break;


        case REALJOYSTICK_EVENT_MENU_TAB:
    //printf ("Event menutab %d\n",value);
            if (value) {
                menu_tab.v=1;
            }
            else {
                menu_tab.v=0;
            }

        break;

		case REALJOYSTICK_EVENT_NUMBERSELECT:

			//cambiar valor solo cuando libera
			//lo hacemos asi porque en axis, aqui se envia muchas veces y saltaria demasiado rapido

			if (!value) {

				//Liberar tecla pulsada. Esto sirve para eventos asociados a botones de tipo axis,
				//si es que hemos puesto numselect y numaction a mismos botones de axis (uno + y otro -)
				//Dado que al liberar numaction, lo que interpreta es que numselect vale 0 y entra aqui
				//Aun asi con esto lo que provoca es que cuando se pulse numaction, se llame tambien a numselect de golpe
				//por lo general no se debian usar los botones de axis para mas que para direcciones
				ascii_to_keyboard_port_set_clear(realjoystick_numselect,0);


				if (realjoystick_numselect=='9') realjoystick_numselect=' ';
				else if (realjoystick_numselect==' ') realjoystick_numselect='0';

				else realjoystick_numselect++;
				debug_printf (VERBOSE_DEBUG,"numberselect: %d",realjoystick_numselect);
				realjoystick_print_char(realjoystick_numselect);

			}
		break;

		case REALJOYSTICK_EVENT_NUMBERACTION:

			if (value) {
				debug_printf (VERBOSE_DEBUG,"send key %c",realjoystick_numselect);
				ascii_to_keyboard_port(realjoystick_numselect);
				realjoystick_print_char(realjoystick_numselect);
			}

			else {
				ascii_to_keyboard_port_set_clear(realjoystick_numselect,0);
			}
		break;


		case REALJOYSTICK_EVENT_JOYSELECT:
			if (value) {
				joystick_cycle_next_type();
				realjoystick_print_joyselect();
			}
		break;

		case REALJOYSTICK_EVENT_ENTER:
			if (value) puerto_49150 &=255-1;
            else puerto_49150 |=1;

        break;


		case REALJOYSTICK_EVENT_QUICKLOAD:
			if (value) {
				menu_abierto=1;
				menu_button_smartload.v=1;
			}
		break;

		case REALJOYSTICK_EVENT_REWIND:
			if (value) {
                realjoystick_send_f_function(F_FUNCION_REWIND);
			}
		break;

		case REALJOYSTICK_EVENT_FFORWARD:
			if (value) {
				realjoystick_send_f_function(F_FUNCION_FFW);
			}
		break;


		case REALJOYSTICK_EVENT_QUICKSAVE:
			if (value) {
				//printf ("boton quicksave\n");
				realjoystick_send_f_function(F_FUNCION_QUICKSAVE);
		      	//Activar funcion f en menu. Primera funcion que llamo desde joystick de la misma manera que cuando se pulsa tecla F de funcion
			}
		break;

		case REALJOYSTICK_EVENT_OSDKEYBOARD:
            if (value) {
                menu_abierto=1;
                menu_button_osdkeyboard.v=1;
            }
        break;

		case REALJOYSTICK_EVENT_OSD_TEXT_KEYBOARD:
            if (value) {
                menu_abierto=1;
                menu_button_osd_adv_keyboard_openmenu.v=1;
            }
        break;


	}

}







int realjoystick_find_if_already_defined_button(realjoystick_events_keys_function *tabla,int maximo,int button,int type)
{
	int i;
        for (i=0;i<maximo;i++) {
                if (tabla[i].asignado.v==1) {
			if (tabla[i].button==button && tabla[i].button_type==type) return i;
		}
	}
	return -1;
}


//redefinir evento
//Devuelve 1 si ok
int realjoystick_redefine_event_key_no_wait(realjoystick_events_keys_function *tabla,int indice,int maximo,int button,int type,int value)
{


			button=menu_info_joystick_last_button;

			type=menu_info_joystick_last_type;
			value=menu_info_joystick_last_value;

	//if (1) {
	//if (realjoystick_read_event(&button,&type,&value) ==1 ) {
		debug_printf (VERBOSE_DEBUG,"redefine for button: %d type: %d value: %d",button,type,value);
		//printf ("redefine for button: %d type: %d value: %d\n",button,type,value);
                //eventos de init no hacerles caso, de momento
		if ( (type&REALJOYSTICK_INPUT_EVENT_INIT)!=REALJOYSTICK_INPUT_EVENT_INIT) {
			debug_printf (VERBOSE_DEBUG,"redefine for button: %d type: %d value: %d",button,type,value);

			int button_type=0;


                        if (type==REALJOYSTICK_INPUT_EVENT_BUTTON) {
                                //tabla[indice].button_type=0;
				button_type=0;
                        }

                        if (type==REALJOYSTICK_INPUT_EVENT_AXIS) {
                                //if (value<0) tabla[indice].button_type=-1;
                                //else tabla[indice].button_type=+1;
                                if (value<0) button_type=-1;
                                else button_type=+1;
                        }

			//Antes de asignarlo, ver que no exista uno antes
			//desasignamos primero el actual
			tabla[indice].asignado.v=0;

			int existe_evento=realjoystick_find_if_already_defined_button(tabla,maximo,button,button_type);

			if (existe_evento!=-1) {
				debug_printf (VERBOSE_ERR,"Button already mapped");
				return 0;
			}

			tabla[indice].asignado.v=1;
			tabla[indice].button=button;
			tabla[indice].button_type=button_type;

			}


    //}

	return 1;
}



//redefinir evento
//Devuelve 1 si ok
//0 si salimos con tecla

/*
int realjoystick_redefine_event_key(realjoystick_events_keys_function *tabla,int indice,int maximo)
{

	menu_espera_no_tecla();

	realjoystick_hit=0;

	//leemos boton
	int button,type,value;

	debug_printf (VERBOSE_DEBUG,"Redefine action: %d",indice);

	//printf ("Redefine action: %d\n",indice);

	simulador_joystick_forzado=1;

	menu_espera_tecla_o_joystick();



	simulador_joystick_forzado=1;

	//si no se ha pulsado joystick, pues se habra pulsado tecla
	if (!realjoystick_hit ) {
		debug_printf (VERBOSE_DEBUG,"Pressed key, not joystick");
		//printf ("Pressed key, not joystick\n");
		return 0;
	}


	debug_printf (VERBOSE_DEBUG,"Pressed joystick");


			button=menu_info_joystick_last_button;

			type=menu_info_joystick_last_type;
			value=menu_info_joystick_last_value;

	//if (1) {
	//if (realjoystick_read_event(&button,&type,&value) ==1 ) {
		debug_printf (VERBOSE_DEBUG,"redefine for button: %d type: %d value: %d",button,type,value);
		//printf ("redefine for button: %d type: %d value: %d\n",button,type,value);
                //eventos de init no hacerles caso, de momento
		if ( (type&REALJOYSTICK_INPUT_EVENT_INIT)!=REALJOYSTICK_INPUT_EVENT_INIT) {
			debug_printf (VERBOSE_DEBUG,"redefine for button: %d type: %d value: %d",button,type,value);

			int button_type=0;


                        if (type==REALJOYSTICK_INPUT_EVENT_BUTTON) {
                                //tabla[indice].button_type=0;
				button_type=0;
                        }

                        if (type==REALJOYSTICK_INPUT_EVENT_AXIS) {
                                //if (value<0) tabla[indice].button_type=-1;
                                //else tabla[indice].button_type=+1;
                                if (value<0) button_type=-1;
                                else button_type=+1;
                        }

			//Antes de asignarlo, ver que no exista uno antes
			//desasignamos primero el actual
			tabla[indice].asignado.v=0;

			int existe_evento=realjoystick_find_if_already_defined_button(tabla,maximo,button,button_type);

			if (existe_evento!=-1) {
				debug_printf (VERBOSE_ERR,"Button already mapped");
				return 0;
			}

			tabla[indice].asignado.v=1;
			tabla[indice].button=button;
			tabla[indice].button_type=button_type;

			}


    //}

	return 1;
}
*/

//redefinir evento
//Devuelve 1 si ok
//0 si salimos con ESC
/*
int realjoystick_redefine_event(int indice)
{
        return realjoystick_redefine_event_key(realjoystick_events_array,indice,MAX_EVENTS_JOYSTICK);
}
*/

//redefinir evento
//Devuelve 1 si ok
int realjoystick_redefine_event_no_wait(int indice,int button,int type,int value)
{
        return realjoystick_redefine_event_key_no_wait(realjoystick_events_array,indice,MAX_EVENTS_JOYSTICK,button,type,value);
}



//redefinir tecla
//Devuelve 1 si ok
//0 si salimos con ESC
/*
int realjoystick_redefine_key(int indice,z80_byte caracter)
{
	if (realjoystick_redefine_event_key(realjoystick_keys_array,indice,MAX_KEYS_JOYSTICK)) {
		realjoystick_keys_array[indice].caracter=caracter;
		return 1;
	}

	return 0;
}
*/

//redefinir tecla
//Devuelve 1 si ok
int realjoystick_redefine_key_no_wait(int indice,z80_byte caracter,int button,int type,int value)
{
    if (realjoystick_redefine_event_key_no_wait(realjoystick_keys_array,indice,MAX_KEYS_JOYSTICK,button,type,value)) {
        //Ya se ha asignado el evento/key dado que la estructura es del mismo tipo (aunque diferentes estructuras asignadas)

        //Asignar la tecla
        realjoystick_keys_array[indice].caracter=caracter;

        return 1;
    }

    else return 0;
}




//copiar boton asociado a un evento hacia boton-tecla
void realjoystick_copy_event_button_key(int indice_evento,int indice_tecla,z80_byte caracter)
{
	if (realjoystick_events_array[indice_evento].asignado.v) {
		debug_printf (VERBOSE_DEBUG,"Setting event %d to key %c on index %d (button %d button_type: %d)",
			indice_evento,caracter,indice_tecla,realjoystick_events_array[indice_evento].button,
			realjoystick_events_array[indice_evento].button_type);
		realjoystick_keys_array[indice_tecla].asignado.v=1;
		realjoystick_keys_array[indice_tecla].button=realjoystick_events_array[indice_evento].button;
		realjoystick_keys_array[indice_tecla].button_type=realjoystick_events_array[indice_evento].button_type;
		realjoystick_keys_array[indice_tecla].caracter=caracter;
	}

	else {
		debug_printf (VERBOSE_DEBUG,"On realjoystick_copy_event_button_key, event %d is not assigned",indice_evento);
	}
}

//Devuelve 0 si ok
int realjoystick_set_type(char *tipo) {

				debug_printf (VERBOSE_INFO,"Setting joystick type %s",tipo);

                                int i;
                                for (i=0;i<=JOYSTICK_TOTAL;i++) {
                                        if (!strcasecmp(tipo,joystick_texto[i])) break;
                                }
                                if (i>JOYSTICK_TOTAL) {
                                        debug_printf (VERBOSE_ERR,"Invalid joystick type %s",tipo);
					return 1;
                                }


                                joystick_emulation=i;
	return 0;
}


//Devuelve 0 si ok
int realjoystick_set_button_event(char *text_button, char *text_event)
{

				debug_printf (VERBOSE_INFO,"Setting button %s to event %s",text_button,text_event);

//--joystickevent but evt    Set a joystick button or axis to an event (changes joystick to event table)

				//obtener boton
				int button,button_type;
				realjoystick_get_button_string(text_button,&button,&button_type);

				//obtener evento
				int evento=realjoystick_get_event_string(text_event);
				if (evento==-1) {
					debug_printf (VERBOSE_ERR,"Unknown event %s",text_event);
					return 1;
				}


				//Y definir el evento
				realjoystick_events_array[evento].asignado.v=1;
				realjoystick_events_array[evento].button=button;
				realjoystick_events_array[evento].button_type=button_type;

	return 0;
}



//Devuelve 0 si ok
int realjoystick_set_button_key(char *text_button,char *text_key)
{
				debug_printf (VERBOSE_INFO,"Setting button %s to key %s",text_button,text_key);

//"--joystickkeybt but key    Define a key pressed when a joystick button pressed (changes joystick to key table)\n"

                                //ver si maximo definido
                                if (joystickkey_definidas==MAX_KEYS_JOYSTICK) {
                                        debug_printf (VERBOSE_ERR,"Maximum defined joystick to keys defined (%d)",joystickkey_definidas);
                                        return 1;
                                }

                                //obtener boton
                                int button,button_type;
                                realjoystick_get_button_string(text_button,&button,&button_type);

                                z80_byte caracter=parse_string_to_number(text_key);


                                //realjoystick_copy_event_button_key(evento,joystickkey_definidas,caracter);
                                realjoystick_keys_array[joystickkey_definidas].asignado.v=1;
                                realjoystick_keys_array[joystickkey_definidas].button=button;
                                realjoystick_keys_array[joystickkey_definidas].button_type=button_type;
                                realjoystick_keys_array[joystickkey_definidas].caracter=caracter;

                                joystickkey_definidas++;


 return 0;
}


//Devuelve 0 si ok
int realjoystick_set_event_key(char *text_event,char *text_key)
{
				debug_printf (VERBOSE_INFO,"Setting event %s to key %s",text_event,text_key);

//    "--joystickkeyev evt key    Define a key pressed when a joystick event is generated (changes joystick to key table)\n"



                                //ver si maximo definido
                                if (joystickkey_definidas==MAX_KEYS_JOYSTICK) {
                                        debug_printf (VERBOSE_ERR,"Maximum defined joystick to keys defined (%d)",joystickkey_definidas);
                                        return 1;
                                }


                                //obtener evento

                                int evento=realjoystick_get_event_string(text_event);
                                if (evento==-1) {
                                        debug_printf (VERBOSE_ERR,"Unknown event %s",text_event);
                                        return 1;
                                }

                                //Y obtener tecla

                                z80_byte caracter=parse_string_to_number(text_key);

                                realjoystick_copy_event_button_key(evento,joystickkey_definidas,caracter);

                                joystickkey_definidas++;
	return 0;
}





//lectura de evento de joystick y conversion a movimiento de joystick spectrum
void realjoystick_common_set_event(int button,int type,int value)
{


		//eventos de init no hacerles caso, de momento
		if ( (type&REALJOYSTICK_INPUT_EVENT_INIT)!=REALJOYSTICK_INPUT_EVENT_INIT) {




			menu_info_joystick_last_button=button;

			menu_info_joystick_last_type=type;
			menu_info_joystick_last_value=value;

			menu_info_joystick_last_index=-1; //de momento suponemos ningun evento


			//buscamos el evento. En axis busca tanto la direccion como la opuesta
			int index=-1;
			do  {

				index=realjoystick_find_event(index+1,button,type,value);
				//menu_info_joystick_last_index=index;
				//printf ("last index: %d\n",menu_info_joystick_last_index);
				if (index>=0) {
					debug_printf (VERBOSE_DEBUG,"Event found on index: %d",index);

					menu_info_joystick_last_index=index;

					//ver tipo boton normal

					if (type==REALJOYSTICK_INPUT_EVENT_BUTTON) {
						realjoystick_set_reset_action(index,value);
					}


					//ver tipo axis
					if (type==REALJOYSTICK_INPUT_EVENT_AXIS) {
						switch (index) {
							case REALJOYSTICK_EVENT_UP:
								//reset abajo
								joystick_release_down(1);
								realjoystick_set_reset_action(index,value);
							break;

							case REALJOYSTICK_EVENT_DOWN:
									//reset arriba
									joystick_release_up(1);
									realjoystick_set_reset_action(index,value);
							break;

							case REALJOYSTICK_EVENT_LEFT:
									//reset derecha
									joystick_release_right(1);
									realjoystick_set_reset_action(index,value);
							break;

							case REALJOYSTICK_EVENT_RIGHT:
									//reset izquierda
									joystick_release_left(1);
									realjoystick_set_reset_action(index,value);
							break;



							default:
								//acciones que no son de axis
								realjoystick_set_reset_action(index,value);
							break;
						}
					}

					//gestionar si es 0, poner 0 en izquierda y derecha por ejemplo (solo para acciones de left/right/up/down)

					//si es >0, hacer que la accion de -1 se resetee (solo para acciones de left/right/up/down)
					//si es <0, hacer que la accion de +1 se resetee (solo para acciones de left/right/up/down)
				}
			} while (index>=0);

			//despues de evento, buscar boton a tecla
			//buscamos el evento
			index=-1;
			do {
                        index=realjoystick_find_key(index+1,button,type,value);
                        if (index>=0) {
                                debug_printf (VERBOSE_DEBUG,"Event found on index: %d. key=%c value:%d",index,realjoystick_keys_array[index].caracter,value);

                                //ver tipo boton normal o axis

                                if (type==REALJOYSTICK_INPUT_EVENT_BUTTON || type==REALJOYSTICK_INPUT_EVENT_AXIS) {
                                        realjoystick_set_reset_key(index,value);
                                }
			}
			} while (index>=0);



		}



}





void realjoystick_initialize_joystick(void)
{



	//Si viene desactivado por config, decir que no esta
	if (realjoystick_disabled.v) {
        realjoystick_present.v=0;
    }

	else {

		//Si tenemos el simulador de joystick, decir que esta presente y usar funciones del simulador
		if (simulador_joystick) {
			realjoystick_init=realjoystick_simulador_init;
			realjoystick_main=realjoystick_simulador_main;
		}


			if (realjoystick_init()) {
				realjoystick_present.v=0;
			}
	}
}


int realjoystick_is_linux_native(void)
{
#ifdef USE_LINUXREALJOYSTICK

	if (no_native_linux_realjoystick.v==0) return 1;

#endif

	return 0;
}



void realjoystick_common_set_hat(int boton,int direccion)
{

    //int valorfinalaxis=0;

    //Left, Right en valor de boton
    //Down, UP en valor de boton+1
    //TODO: esto es completamente arbitrario y solo para poder usar bien funcion realjoystick_common_set_event como si fuera un AXIS


    //Valores deducidos mediante dos joystick con hat:
    //0,1,2,3,4,5,6,7 empezando con direccion arriba y yendo en las agujas del reloj
    //15 es no movimiento (o centro?). Para el caso, cualquier otro valor diferente de 0-7, equivale a no movimiento
    //aunque se elige el 15 porque es el mismo que se recibe desde la lectura de un hat de joystick

    //Primero reseteamos movimiento en los dos ejes simulados
    realjoystick_common_set_event(boton,REALJOYSTICK_INPUT_EVENT_AXIS,0);
    realjoystick_common_set_event(boton+1,REALJOYSTICK_INPUT_EVENT_AXIS,0);
    menu_info_joystick_last_raw_value=0;


    int bitmask_direction=0;  //Up 8 Down 4 Left 2 Right 1

    #define BITMASK_DIR_UP    8
    #define BITMASK_DIR_DOWN  4
    #define BITMASK_DIR_LEFT  2
    #define BITMASK_DIR_RIGHT 1

    //TODO: podria enviar las direcciones directamente desde este switch pero queda mas limpio enviarlas abajo,
    //tambien me ahorro sentencias de realjoystick_common_set_event repetidas para las diagonales

    switch(direccion) {
        case 0: //up
            bitmask_direction |= BITMASK_DIR_UP;
        break;

        case 1: //up+right
            bitmask_direction |= BITMASK_DIR_UP | BITMASK_DIR_RIGHT;
        break;

        case 2: //right
            bitmask_direction |= BITMASK_DIR_RIGHT;
        break;

        case 3: //down+right
            bitmask_direction |= BITMASK_DIR_RIGHT | BITMASK_DIR_DOWN;
        break;

        case 4: //down
            bitmask_direction |= BITMASK_DIR_DOWN;
        break;

        case 5: //down+left
            bitmask_direction |= BITMASK_DIR_DOWN | BITMASK_DIR_LEFT;
        break;

        case 6: //left
            bitmask_direction |= BITMASK_DIR_LEFT;
        break;

        case 7: //left+up
            bitmask_direction |= BITMASK_DIR_LEFT | BITMASK_DIR_UP;
        break;


    }



    //boton: up : -32767 down: 32767
    //boton+1 left: -32767, right: 32767

    if (bitmask_direction) {


        if (bitmask_direction&BITMASK_DIR_LEFT) {
            realjoystick_common_set_event(boton,REALJOYSTICK_INPUT_EVENT_AXIS,-32767);
            menu_info_joystick_last_raw_value=-32767;
        }

        if (bitmask_direction&BITMASK_DIR_RIGHT) {
            realjoystick_common_set_event(boton,REALJOYSTICK_INPUT_EVENT_AXIS,+32767);
            menu_info_joystick_last_raw_value=+32767;
        }

        if (bitmask_direction&BITMASK_DIR_UP) {
            realjoystick_common_set_event(boton+1,REALJOYSTICK_INPUT_EVENT_AXIS,-32767);
            menu_info_joystick_last_raw_value=-32767;
        }

        if (bitmask_direction&BITMASK_DIR_DOWN) {
            realjoystick_common_set_event(boton+1,REALJOYSTICK_INPUT_EVENT_AXIS,+32767);
            menu_info_joystick_last_raw_value=+32767;
        }


        realjoystick_hit=1;

    }

    //y si es 0, reseteamos movimiento
    /*else {
        realjoystick_common_set_event(boton,REALJOYSTICK_INPUT_EVENT_AXIS,0);
        realjoystick_common_set_event(boton+1,REALJOYSTICK_INPUT_EVENT_AXIS,0);
    }*/


}