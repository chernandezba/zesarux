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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "cpu.h"

#define JOYSTICK_TOTAL 16
#define JOYSTICK_NONE 0
#define JOYSTICK_KEMPSTON 1
#define JOYSTICK_SINCLAIR_1 2
#define JOYSTICK_SINCLAIR_2 3
#define JOYSTICK_CURSOR 4
#define JOYSTICK_CURSOR_WITH_SHIFT 5
#define JOYSTICK_OPQA_SPACE 6
#define JOYSTICK_FULLER 7
#define JOYSTICK_ZEBRA 8
#define JOYSTICK_MIKROGEN 9
#define JOYSTICK_ZXPAND 10
#define JOYSTICK_CURSOR_SAM 11
#define JOYSTICK_CPC_1 12
#define JOYSTICK_MSX 13
#define JOYSTICK_SVI 14
#define JOYSTICK_PCW_CASCADE 15
#define JOYSTICK_PCW_DKTRONICS 16


extern z80_byte puerto_especial_joystick;

extern int joystick_emulation;
extern int joystick_autofire_frequency;
extern int joystick_autofire_counter;

extern int joystick_autoleftright_enabled;
extern int joystick_autoleftright_frequency;
extern int joystick_autoleftright_counter;
extern int joystick_autoleftright_status;

extern int joystick_barato;

extern char *joystick_texto[];

extern void joystick_set_right(int si_enviar_zeng_event);
extern void joystick_release_right(int si_enviar_zeng_event);
extern void joystick_set_left(int si_enviar_zeng_event);
extern void joystick_release_left(int si_enviar_zeng_event);
extern void joystick_set_down(int si_enviar_zeng_event);
extern void joystick_release_down(int si_enviar_zeng_event);
extern void joystick_set_up(int si_enviar_zeng_event);
extern void joystick_release_up(int si_enviar_zeng_event);

extern void joystick_set_fire(int si_enviar_zeng_event,int fire_button);
extern void joystick_release_fire(int si_enviar_zeng_event,int fire_button);

extern z80_bit lightgun_emulation_enabled;
extern int lightgun_emulation_type;
extern z80_bit lightgun_scope;

#define LIGHTGUN_TOTAL 7
#define GUNSTICK_SINCLAIR_1 0
#define GUNSTICK_SINCLAIR_2 1
#define GUNSTICK_KEMPSTON 2
#define MAGNUM_AUX 3
#define DEFENDER_LIGHTGUN 4
#define STACK_LIGHT_RIFLE 5
#define TROJAN_LIGHT_PEN_EAR 6


extern void lightgun_draw_scope(void);

extern char *lightgun_types_list[];
extern void lightgun_print_types(void);

extern int lightgun_set_type(char *tipo);


extern int lightgun_x;
extern int lightgun_y;
extern int lightgun_view_white(void);
extern int lightgun_view_electron(void);

//extern int lightgun_range_x;
//extern int lightgun_range_y;
//extern int lightgun_y_offset;
//extern int lightgun_solo_brillo;



extern int mouse_x,mouse_y;
extern int mouse_wheel_vertical,mouse_wheel_horizontal;
extern z80_bit kempston_mouse_emulation;
extern int mouse_left,mouse_right,mouse_pressed_close_window,mouse_pressed_background_window;

extern int mouse_pressed_hotkey_window;
extern int mouse_pressed_hotkey_window_key;

extern z80_byte kempston_mouse_x,kempston_mouse_y;

extern void joystick_print_types(void);

extern void joystick_cycle_next_type(void);
extern void joystick_cycle_next_type_autofire(void);

#define JOYSTICK_KEY_FIRE_TOTAL 8

#define JOYSTICK_KEY_FIRE_IS_HOME 0
#define JOYSTICK_KEY_FIRE_IS_RIGHTALT 1
#define JOYSTICK_KEY_FIRE_IS_RIGHTCTRL 2
#define JOYSTICK_KEY_FIRE_IS_RIGHTSHIFT 3
#define JOYSTICK_KEY_FIRE_IS_LEFTALT 4
#define JOYSTICK_KEY_FIRE_IS_LEFTCTRL 5
#define JOYSTICK_KEY_FIRE_IS_LEFTSHIFT 6
#define JOYSTICK_KEY_FIRE_IS_TAB 7

extern char *joystick_defined_fire_texto[];
//extern int joystick_defined_key_fire;
//extern int joystick_defined_key_fire2;
//extern int joystick_defined_key_fire3;
//extern int joystick_defined_key_fire4;

extern int joystick_defined_key_fire_array[];

extern void joystick_possible_home_key(int pressrelease);
extern void joystick_possible_rightshift_key(int pressrelease);
extern void joystick_possible_rightalt_key(int pressrelease);
extern void joystick_possible_rightctrl_key(int pressrelease);
extern void joystick_possible_leftshift_key(int pressrelease);
extern void joystick_possible_leftalt_key(int pressrelease);
extern void joystick_possible_leftctrl_key(int pressrelease);
extern void joystick_possible_tab_key(int pressrelease);

#endif
