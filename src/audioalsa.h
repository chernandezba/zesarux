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

#ifndef AUDIOALSA_H
#define AUDIOALSA_H

#include "cpu.h"

extern int audioalsa_init(void);
extern void audioalsa_end(void);
extern int audioalsa_thread_finish(void);
extern void audioalsa_send_frame(char *buffer);
extern int fifo_alsa_buffer_size;
extern int alsa_periodsize;
extern void audioalsa_get_buffer_info (int *buffer_size,int *current_buffer_position);

//extern int alsa_midi_client;
//extern int alsa_midi_port;
extern int alsa_midi_volume;
extern void alsa_midi_output_flush_output(void);
extern int alsa_note_on(unsigned char channel, unsigned char note,unsigned char velocity);
extern int alsa_note_off(unsigned char channel, unsigned char note,unsigned char velocity);
extern int alsa_mid_initialize_all(void);
extern void alsa_mid_finish_all(void);
extern void alsa_mid_initialize_volume(void);

//#define MAX_ALSA_MID_DEVICE_OUT 128
//extern char *alsa_mid_device_out;
extern char alsa_mid_device_out[];

extern int alsa_midi_raw(z80_byte value);
extern void alsa_midi_output_reset(void);
extern int alsa_change_instrument(unsigned char instrument);

//extern int alsa_midi_initialized;

//extern void alsa_midi_output_frame_event(void);

#define ALSA_MID_VELOCITY 127



#endif
