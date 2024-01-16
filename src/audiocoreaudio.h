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

#ifndef AUDIOCOREAUDIO_H
#define AUDIOCOREAUDIO_H

#include "cpu.h"

extern int audiocoreaudio_init(void);
extern int audiocoreaudio_thread_finish(void);
extern void audiocoreaudio_send_frame(char *buffer);
extern void audiocoreaudio_end(void);

extern void audiocoreaudio_get_buffer_info (int *buffer_size,int *current_buffer_position);
extern int audiocoreaudio_can_record_input(void);
extern void audiocoreaudio_start_record_input(void);
extern void audiocoreaudio_stop_record_input(void);

extern void coreaudio_mid_raw_send(z80_byte value);
extern void coreaudio_midi_output_reset(void);

#define MAX_AUDIOCOREAUDIO_FIFO_MULTIPLIER 10
#define MIN_AUDIOCOREAUDIO_FIFO_MULTIPLIER 2

extern int audiocoreaudio_fifo_buffer_size_multiplier;

extern int coreaudio_note_on(unsigned char channel, unsigned char note,unsigned char velocity);
extern int coreaudio_note_off(unsigned char channel, unsigned char note,unsigned char velocity);
extern int coreaudio_change_instrument(unsigned char instrument);
extern void coreaudio_midi_output_flush_output(void);
extern int coreaudio_mid_initialize_all(void);
extern void coreaudio_mid_finish_all(void);

#endif
