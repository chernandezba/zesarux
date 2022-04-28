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

#ifndef AUDIOONEBITSPEAKER_H
#define AUDIOONEBITSPEAKER_H

#include "cpu.h"

extern int audioonebitspeaker_init(void);
extern void audiopcspeaker_end(void);
extern int audiopcspeaker_thread_finish(void);
extern void audiopcspeaker_send_frame(char *buffer);
extern void audiopcspeaker_get_buffer_info (int *buffer_size,int *current_size);
extern void audiopcspeaker_calibrate_tiempo_espera(void);


#endif
