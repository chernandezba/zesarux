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

#ifndef REALJOYSTICK_H
#define REALJOYSTICK_H

#include "cpu.h"


#include "compileoptions.h"


#ifdef USE_LINUXREALJOYSTICK
        //Es un linux con soporte realjoystick

        #include <linux/joystick.h>

#else
        //No es linux con soporte realjoystick
        //En este caso definimos los tipos de datos usados para que no pete al compilar
        //Luego en el init detectamos esto y devuelve que no hay soporte joystick


        //#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
        //#define JS_EVENT_AXIS           0x02    /* joystick moved */
        //#define JS_EVENT_INIT           0x80    /* initial state of device */


typedef unsigned int __u32;
typedef short __s16;
typedef unsigned char __u8;

        struct js_event {
                __u32 time;     /* event timestamp in milliseconds */
                __s16 value;    /* value */
                __u8 type;      /* event type */
                __u8 number;    /* axis/button number */
        };


#endif


//Los hacemos diferentes de JS_EVENT_* de Linux para asegurarnos que el codigo es portable en otras plataformas
        //#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
        //#define JS_EVENT_AXIS           0x02    /* joystick moved */
        //#define JS_EVENT_INIT           0x80    /* initial state of device */

#define REALJOYSTICK_INPUT_EVENT_BUTTON         0x04    /* button pressed/released */
#define REALJOYSTICK_INPUT_EVENT_AXIS           0x08    /* joystick moved */
#define REALJOYSTICK_INPUT_EVENT_INIT           0x40    /* initial state of device */


extern int (*realjoystick_init)(void);
extern void (*realjoystick_main)(void);
//extern int (*realjoystick_hit)(void);

extern int realjoystick_hit;

extern int realjoystick_null_init(void);
extern void realjoystick_null_main(void);
extern int realjoystick_null_hit(void);


extern void realjoystick_linux_main(void);
extern int realjoystick_linux_init(void);
extern int realjoystick_linux_hit(void);

extern z80_bit no_native_linux_realjoystick;
extern int realjoystick_autocalibrate_value;


extern int realjoystick_read_event(int *button,int *type,int *value);

//extern void realjoystick_set_default_functions(void);

extern void realjoystick_new_set_default_functions(void);

extern void realjoystick_init_events_keys_tables(void);
extern void realjoystick_initialize_joystick(void);

extern char string_dev_joystick[];

extern int realjoystick_index;

extern z80_bit realjoystick_present;

extern z80_bit realjoystick_disabled;

#define REALJOYSTICK_EVENT_UP                   0
#define REALJOYSTICK_EVENT_DOWN                 1
#define REALJOYSTICK_EVENT_LEFT                 2
#define REALJOYSTICK_EVENT_RIGHT                3
#define REALJOYSTICK_EVENT_FIRE                 4
#define REALJOYSTICK_EVENT_ESC_MENU             5
#define REALJOYSTICK_EVENT_EXIT_EMULATOR        6
#define REALJOYSTICK_EVENT_ENTER                7
#define REALJOYSTICK_EVENT_MENU_TAB             8
#define REALJOYSTICK_EVENT_QUICKLOAD            9
#define REALJOYSTICK_EVENT_QUICKSAVE            10
#define REALJOYSTICK_EVENT_OSDKEYBOARD          11
#define REALJOYSTICK_EVENT_OSD_TEXT_KEYBOARD    12
#define REALJOYSTICK_EVENT_NUMBERSELECT         13
#define REALJOYSTICK_EVENT_NUMBERACTION         14
#define REALJOYSTICK_EVENT_JOYSELECT            15
#define REALJOYSTICK_EVENT_AUX1                 16
#define REALJOYSTICK_EVENT_AUX2                 17
#define REALJOYSTICK_EVENT_AUX3                 18
#define REALJOYSTICK_EVENT_AUX4                 19
#define REALJOYSTICK_EVENT_REWIND               20
#define REALJOYSTICK_EVENT_FFORWARD             21

//este valor es el numero de ultimo REALJOYSTICK_EVENT_XX +1
#define MAX_EVENTS_JOYSTICK 22

extern char *realjoystick_event_names[];


struct s_realjoystick_event_key_function {

	//si esta asignada esa funcion o no
	z80_bit asignado;

	//numero de boton
	int button;

	//tipo de boton: 0-boton normal, +1 axis positivo, -1 axis negativo
	int button_type;

        //caracter a enviar, usado en array de keys pero no en array de eventos
        z80_byte caracter;

};

typedef struct s_realjoystick_event_key_function realjoystick_events_keys_function;

extern realjoystick_events_keys_function realjoystick_events_array[];

extern int realjoystick_redefine_event(int indice);
extern int realjoystick_redefine_event_no_wait(int indice,int button,int type,int value);
extern int realjoystick_redefine_key(int indice,z80_byte caracter);
extern int realjoystick_redefine_key_no_wait(int indice,z80_byte caracter,int button,int type,int value);


#define MAX_KEYS_JOYSTICK 12

extern realjoystick_events_keys_function realjoystick_keys_array[];

extern void realjoystick_copy_event_button_key(int indice_evento,int indice_tecla,z80_byte caracter);

extern void realjoystick_clear_keys_array(void);

extern void realjoystick_clear_events_array(void);

extern void realjoystick_print_event_keys(void);

extern void realjoystick_get_button_string(char *texto, int *button,int *button_type);

extern int realjoystick_get_event_string(char *texto);

extern z80_bit realjoystick_clear_keys_on_smartload;

extern int realjoystick_set_type(char *tipo);

extern int realjoystick_set_button_event(char *text_button, char *text_event);

extern int realjoystick_set_button_key(char *text_button,char *text_key);

extern int realjoystick_set_event_key(char *text_event,char *text_key);

extern void realjoystick_common_set_event(int button,int type,int value);
extern void realjoystick_common_set_hat(int boton,int direccion);


extern int simulador_joystick;
extern int simulador_joystick_forzado;

//extern int realjoystick_find_event(int indice_inicial,int button,int type,int value);
extern int realjoystick_buscar_evento_en_tabla(int button, int button_type);

extern void realjoystick_reopen_driver(void);
extern int realjoystick_is_linux_native(void);
extern void realjoystick_start_driver(void);

extern int realjoystick_total_buttons;
extern int realjoystick_total_axes;
#define REALJOYSTICK_MAX_NAME 32

extern char realjoystick_joy_name[];

#define REALJOYSTICK_MAX_DRIVER_NAME 40

extern char realjoystick_driver_name[];


#endif
