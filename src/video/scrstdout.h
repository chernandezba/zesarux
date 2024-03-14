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

#ifndef SCRSTDOUT_H
#define SCRSTDOUT_H

#include "cpu.h"

extern int scrstdout_init (void);
extern void scrstdout_end(void);
extern void scrstdout_refresca_pantalla(void);
extern void scrstdout_refresca_pantalla_solo_driver(void);
extern z80_byte scrstdout_lee_puerto(z80_byte puerto_h,z80_byte puerto_l);
extern void scrstdout_actualiza_tablas_teclado(void);
extern void scrstdout_debug_registers(void);
extern void scrstdout_messages_debug(char *s);
extern void scr_stdout_printchar(void);





extern void scr_stdout_debug_char_table_routines_peek(z80_int dir);
extern void scr_stdout_debug_char_table_routines_poke(z80_int dir);

extern void scr_stdout_debug_print_char_routine(void);

extern z80_bit scrstdout_also_send_speech_debug_messages;



#endif
