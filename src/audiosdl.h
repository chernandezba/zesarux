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

#ifndef AUDIOSDL_H
#define AUDIOSDL_H

#include "cpu.h"

extern int audiosdl_init(void);
extern int audiosdl_thread_finish(void);
extern void audiosdl_send_frame(char *buffer);
extern void audiosdl_end(void);

#define DEFAULT_AUDIOSDL_SAMPLES 512
extern int audiosdl_samples;

extern void audiosdl_get_buffer_info (int *buffer_size,int *current_buffer_position);

#define MAX_AUDIOSDL_FIFO_MULTIPLIER 10
#define MIN_AUDIOSDL_FIFO_MULTIPLIER 2

extern int audiosdl_fifo_buffer_size_multiplier;


#endif
