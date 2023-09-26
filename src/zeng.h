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

#ifndef ZENG_H
#define ZENG_H

#include "utils.h"

//Estructura para la FIFO de eventos de teclas

struct s_zeng_key_presses {
	enum util_teclas tecla;
	int pressrelease;
    //Este contador solo se usa por zeng online, y no por zeng normal
    unsigned int contador_scanline;
};

typedef struct s_zeng_key_presses zeng_key_presses;

//50 teclas en cola, que es una barbaridad
#define ZENG_FIFO_SIZE 50

//Total de hosts al que se conecta un hosts concreto
//Por tanto el total de hosts interconectados entre ellos es este valor +1 (+1 por ser el mismo)
#define ZENG_MAX_REMOTE_HOSTS 15

extern int zeng_fifo_add_element(zeng_key_presses *elemento);

extern int zeng_fifo_read_element(zeng_key_presses *elemento);

extern void zeng_send_key_event(enum util_teclas tecla,int pressrelease);

extern void zeng_empty_fifo(void);

extern z80_bit zeng_enabled;
extern void zeng_enable(void);
extern void zeng_disable(void);
extern void zeng_cancel_connect(void);
extern void zeng_send_snapshot_if_needed(void);


extern char zeng_remote_hostname[];

extern int zeng_do_not_send_input_events;

//Puerto remoto
extern int zeng_remote_port;
extern int zeng_i_am_master;
//extern int zeng_segundos_cada_snapshot;
extern int zeng_frames_video_cada_snapshot;

extern void zeng_add_pending_send_message_footer(char *mensaje);
extern int pending_zeng_send_message_footer;
extern void zeng_disable_forced(void);

extern int zeng_enable_thread_running;

extern z80_bit zeng_force_reconnect_failed_retries;

#define MAX_ZENG_HOSTNAME 256

#endif
