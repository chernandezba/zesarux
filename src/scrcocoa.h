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

#ifndef SCRCOCOA_H
#define SCRCOCOA_H

#include "cpu.h"


extern void scrcocoa_refresca_pantalla(void);
extern void scrcocoa_refresca_pantalla_solo_driver(void);
extern int scrcocoa_init (void);
extern void scrcocoa_end(void);
extern z80_byte scrcocoa_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrcocoa_actualiza_tablas_teclado(void);
extern void scrcocoa_debug_registers(void);
extern void scrcocoa_messages_debug(char *s);
//estos valores son arbitrarios, solo son para meterlos en el array y luego leerlos, deben ser >255 y no repetirse
#define COCOA_KEY_RETURN 256
#define COCOA_KEY_TAB 257
#define COCOA_KEY_BACKSPACE 258
#define COCOA_KEY_ESCAPE 259
#define COCOA_KEY_RMETA 260
#define COCOA_KEY_LMETA 261
#define COCOA_KEY_LSHIFT 262
#define COCOA_KEY_CAPSLOCK 263
#define COCOA_KEY_LALT 264
#define COCOA_KEY_LCTRL 265
#define COCOA_KEY_RSHIFT 266
#define COCOA_KEY_RALT 267
#define COCOA_KEY_RCTRL 268
#define COCOA_KEY_KP_MULTIPLY 269
#define COCOA_KEY_KP_PLUS 270
#define COCOA_KEY_NUMLOCK 271
#define COCOA_KEY_KP_DIVIDE 272
#define COCOA_KEY_KP_ENTER 273
#define COCOA_KEY_KP_MINUS 274
#define COCOA_KEY_KP_EQUALS 275
#define COCOA_KEY_KP0 276
#define COCOA_KEY_KP1 277
#define COCOA_KEY_KP2 278
#define COCOA_KEY_KP3 279
#define COCOA_KEY_KP4 280
#define COCOA_KEY_KP5 281
#define COCOA_KEY_KP6 282
#define COCOA_KEY_KP7 283
#define COCOA_KEY_KP8 284
#define COCOA_KEY_KP9 285
#define COCOA_KEY_F5 286
#define COCOA_KEY_F6 287
#define COCOA_KEY_F7 288
#define COCOA_KEY_F3 289
#define COCOA_KEY_F8 290
#define COCOA_KEY_F9 291
#define COCOA_KEY_F11 292
#define COCOA_KEY_PRINT 293
#define COCOA_KEY_SCROLLOCK 294
#define COCOA_KEY_F10 295
#define COCOA_KEY_F12 296
#define COCOA_KEY_PAUSE 297
#define COCOA_KEY_INSERT 298
#define COCOA_KEY_HOME 299
#define COCOA_KEY_PAGEUP 300
#define COCOA_KEY_DELETE 301
#define COCOA_KEY_F4 302
#define COCOA_KEY_END 303
#define COCOA_KEY_F2 304
#define COCOA_KEY_PAGEDOWN 305
#define COCOA_KEY_F1 306
#define COCOA_KEY_LEFT 307
#define COCOA_KEY_RIGHT 308
#define COCOA_KEY_DOWN 309
#define COCOA_KEY_UP 310


#endif
