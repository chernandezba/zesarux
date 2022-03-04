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

#ifndef TEXTSPEECH_H
#define TEXTSPEECH_H

#include "cpu.h"


extern char *textspeech_filter_program;
extern z80_bit textspeech_filter_program_wait;

extern void scrstdout_menu_print_speech(char *texto);


//esto luego ira en speech
//Linea actual en buffer
#define MAX_BUFFER_SPEECH 1500
//+1 para guardar el 0 del final
extern char buffer_speech[];
extern int index_buffer_speech;

extern void textspeech_print_speech(char *texto);

extern int textspeech_finalizado_hijo_speech(void);

extern int textspeech_get_stdout_childs(void);

extern void scrtextspeech_filter_run_pending(void);

extern void textspeech_empty_speech_fifo(void);

extern void textspeech_add_speech_fifo(void);
extern void textspeech_add_speech_fifo_debugconsole_yesno(int also_send_to_debug_console);
extern z80_bit textspeech_get_stdout;
extern int textspeech_fds_output_initialized;

extern int scrtextspeech_filter_counter;

extern int textspeech_timeout_no_enter;

extern void textspeech_add_character(z80_byte c);

extern void textspeech_send_new_line(void);

//extern int textspeech_operating_counter;

extern void textspeech_print_operating(void);
extern void textspeech_clear_operating(void);

extern z80_bit textspeech_also_send_menu;

extern void textspeech_borrar_archivo_windows_lock_file(void);
extern void textspeech_borrar_archivo_windows_speech_file(void);

extern void textspeech_enviar_speech_pantalla(void);

extern int fifo_buffer_speech_size;

extern void textspeech_filter_program_check_spaces(void);

extern void textspeech_stop_filter_program_check_spaces(void);

extern char *textspeech_stop_filter_program;

extern void ocr_get_text(char *s);

#endif
