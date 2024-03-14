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

#ifndef SCRSDL_H
#define SCRSDL_H

#include "cpu.h"


extern void scrsdl_refresca_pantalla(void);
extern void scrsdl_refresca_pantalla_solo_driver(void);
extern int scrsdl_init (void);
extern void scrsdl_end(void);
extern z80_byte scrsdl_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrsdl_actualiza_tablas_teclado(void);
extern void scrsdl_debug_registers(void);
extern void scrsdl_messages_debug(char *s);


#define ZESARUX_SDL_SCANCODE_ESCAPE 9
#define ZESARUX_SDL_SCANCODE_1 10
#define ZESARUX_SDL_SCANCODE_2 11
#define ZESARUX_SDL_SCANCODE_3 12
#define ZESARUX_SDL_SCANCODE_4 13
#define ZESARUX_SDL_SCANCODE_5 14
#define ZESARUX_SDL_SCANCODE_6 15
#define ZESARUX_SDL_SCANCODE_7 16
#define ZESARUX_SDL_SCANCODE_8 17
#define ZESARUX_SDL_SCANCODE_9 18
#define ZESARUX_SDL_SCANCODE_0 19

#define ZESARUX_SDL_SCANCODE_MINUS 20
#define ZESARUX_SDL_SCANCODE_EQUALS 21

#define ZESARUX_SDL_SCANCODE_BACKSPACE 22

#define ZESARUX_SDL_SCANCODE_Q 24
#define ZESARUX_SDL_SCANCODE_W 25
#define ZESARUX_SDL_SCANCODE_E 26
#define ZESARUX_SDL_SCANCODE_R 27
#define ZESARUX_SDL_SCANCODE_T 28
#define ZESARUX_SDL_SCANCODE_Y 29
#define ZESARUX_SDL_SCANCODE_U 30
#define ZESARUX_SDL_SCANCODE_I 31
#define ZESARUX_SDL_SCANCODE_O 32
#define ZESARUX_SDL_SCANCODE_P 33

#define ZESARUX_SDL_SCANCODE_LEFTBRACKET 34
#define ZESARUX_SDL_SCANCODE_RIGHTBRACKET 35

#define ZESARUX_SDL_SCANCODE_RETURN 36
#define ZESARUX_SDL_SCANCODE_LCTRL 37

#define ZESARUX_SDL_SCANCODE_A 38
#define ZESARUX_SDL_SCANCODE_S 39
#define ZESARUX_SDL_SCANCODE_D 40
#define ZESARUX_SDL_SCANCODE_F 41
#define ZESARUX_SDL_SCANCODE_G 42
#define ZESARUX_SDL_SCANCODE_H 43
#define ZESARUX_SDL_SCANCODE_J 44
#define ZESARUX_SDL_SCANCODE_K 45
#define ZESARUX_SDL_SCANCODE_L 46

#define ZESARUX_SDL_SCANCODE_SEMICOLON 47
#define ZESARUX_SDL_SCANCODE_APOSTROPHE 48

#define ZESARUX_SDL_SCANCODE_LSHIFT 50

#define ZESARUX_SDL_SCANCODE_Z 52
#define ZESARUX_SDL_SCANCODE_X 53
#define ZESARUX_SDL_SCANCODE_C 54
#define ZESARUX_SDL_SCANCODE_V 55
#define ZESARUX_SDL_SCANCODE_B 56
#define ZESARUX_SDL_SCANCODE_N 57
#define ZESARUX_SDL_SCANCODE_M 58

#define ZESARUX_SDL_SCANCODE_COMMA 59
#define ZESARUX_SDL_SCANCODE_PERIOD 60
#define ZESARUX_SDL_SCANCODE_SLASH 61

#define ZESARUX_SDL_SCANCODE_SPACE 65

#define ZESARUX_SDL_SCANCODE_F1 67
#define ZESARUX_SDL_SCANCODE_F2 68
#define ZESARUX_SDL_SCANCODE_F3 69
#define ZESARUX_SDL_SCANCODE_F4 70
#define ZESARUX_SDL_SCANCODE_F5 71
#define ZESARUX_SDL_SCANCODE_F6 72
#define ZESARUX_SDL_SCANCODE_F7 73
#define ZESARUX_SDL_SCANCODE_F8 74
#define ZESARUX_SDL_SCANCODE_F9 75
#define ZESARUX_SDL_SCANCODE_F10 76

#define ZESARUX_SDL_SCANCODE_F11 95
#define ZESARUX_SDL_SCANCODE_F12 96
#define ZESARUX_SDL_SCANCODE_F13 97
#define ZESARUX_SDL_SCANCODE_F14 98
#define ZESARUX_SDL_SCANCODE_F15 99

#define ZESARUX_SDL_SCANCODE_LEFT 113
#define ZESARUX_SDL_SCANCODE_RIGHT 114
#define ZESARUX_SDL_SCANCODE_DOWN 116
#define ZESARUX_SDL_SCANCODE_UP 111



#define ZESARUX_SDL_SCANCODE_KP_4 83
#define ZESARUX_SDL_SCANCODE_KP_6 85
#define ZESARUX_SDL_SCANCODE_KP_2 88
#define ZESARUX_SDL_SCANCODE_KP_8 80

#define ZESARUX_SDL_SCANCODE_DELETE 119

#endif
