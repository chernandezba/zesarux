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

#ifndef QL_I8049_H
#define QL_I8049_H

#include "ql.h"

extern int ql_ipc_reading_bit_ready;

extern unsigned char ql_read_ipc(void);
extern void ql_write_ipc(unsigned char Data);
extern char ql_audio_da_output(void);
extern void ql_audio_next_cycle(void);

extern void ql_ipc_reset(void);

extern void qltraps_init_fopen_files_array(void);

extern moto_int ql_current_sound_duration;

extern int ql_mantenido_pulsada_tecla;
extern int ql_mantenido_pulsada_tecla_timer;

extern int ql_pressed_backspace;

extern int ql_pulsado_tecla(void);

extern z80_byte ql_keyboard_table[];

extern int ql_ipc_get_frecuency_sound_current_pitch(void);

extern int i8049_chip_present;

extern int ql_audio_playing;

extern const int ql_i8049_sound_chip_frequency;

extern int ql_ipc_get_frecuency_sound_value(int pitch);

extern void ql_stop_sound(void);


extern unsigned char ql_audio_pitch1;
extern unsigned char ql_audio_pitch2;
extern moto_int ql_audio_grad_x;
extern moto_int ql_audio_duration;
extern unsigned char ql_audio_grad_y;
extern unsigned char ql_audio_wrap;
extern unsigned char ql_audio_randomness_of_step;
extern unsigned char ql_audio_fuziness;


extern int ql_sound_feature_pitch2_enabled;
extern int ql_sound_feature_grad_x_enabled;
extern int ql_sound_feature_grad_y_enabled;
extern int ql_sound_feature_wrap_enabled;
extern int ql_sound_feature_fuzzy_enabled;
extern int ql_sound_feature_random_enabled;

#endif
