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

#ifndef TIMER_H
#define TIMER_H

#include "cpu.h"



extern void timer_pause_waiting_end_frame(void);
extern void timer_check_interrupt(void);
extern void timer_reset(void);
extern void start_timer_thread(void);

extern int timer_sleep_machine;
extern int original_timer_sleep_machine;
extern void timer_sleep(int milisec);

extern int contador_segundo;

extern int timer_condicion_top_speed(void);
extern int top_speed_real_frames;
extern z80_bit top_speed_timer;

extern z80_bit interrupcion_fifty_generada;

extern int timer_get_uptime_seconds(void);

extern void timer_get_texto_time(struct timeval *tv, char *texto);

extern int timer_get_worked_time(void);


extern struct timeval core_cpu_timer_frame_antes;
extern struct timeval core_cpu_timer_frame_despues;
extern long core_cpu_timer_frame_difftime;
extern long core_cpu_timer_frame_media;


extern struct timeval core_cpu_timer_refresca_pantalla_antes;
extern struct timeval core_cpu_timer_refresca_pantalla_despues;
extern long core_cpu_timer_refresca_pantalla_difftime;
extern long core_cpu_timer_refresca_pantalla_media;

extern long timer_stats_diference_time(struct timeval *tiempo_antes, struct timeval *tiempo_despues);
extern void timer_stats_current_time(struct timeval *tiempo);



extern struct timeval core_cpu_timer_each_frame_antes;
extern struct timeval core_cpu_timer_each_frame_despues;
extern long core_cpu_timer_each_frame_difftime;
extern long core_cpu_timer_each_frame_media;


extern struct timeval core_render_menu_overlay_antes;
extern struct timeval core_render_menu_overlay_despues;
extern long core_render_menu_overlay_difftime;
extern long core_render_menu_overlay_media;


extern int timer_on_screen_key;
extern int timer_on_screen_adv_key;

extern void timer_toggle_top_speed_timer(void);

extern void timer_get_elapsed_core_frame_pre(void);
extern void timer_get_elapsed_core_frame_post(void);

extern long timer_get_current_seconds(void);

#endif
