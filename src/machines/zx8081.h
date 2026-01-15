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

#ifndef ZX8081_H
#define ZX8081_H

#include "cpu.h"

extern z80_byte caracteres_zx80[];
extern z80_byte caracteres_zx81[];
extern z80_byte da_codigo81(z80_byte codigo,z80_bit *inverse);
extern z80_byte da_codigo_zx80_no_artistic(z80_byte codigo);
extern z80_byte da_codigo_zx81_no_artistic(z80_byte codigo);
extern z80_byte da_codigo81_solo_letras(z80_byte codigo,z80_bit *inverse);

extern z80_bit nmi_generator_active;
//extern z80_bit hsync_generator_active;
//extern z80_bit vsync_generator_active;
extern z80_bit zx8081_vsync_generator;
extern z80_bit video_fast_mode_emulation;
extern z80_byte video_fast_mode_next_frame_black;
extern int zx8081_get_vsync_length(void);

extern z80_bit simulate_lost_vsync;
extern z80_bit simulate_lost_hsync;

extern z80_byte fetch_opcode_zx81_graphics(void);

extern z80_int ramtop_zx8081;

extern z80_bit zx8081_vsync_sound;

extern int timeout_linea_vsync;

extern z80_bit force_zx81_chr_128;


extern z80_bit zx8081_detect_vsync_sound;

extern int zx8081_detect_vsync_sound_counter;


extern z80_bit hotswapped_zx80_to_zx81;


//a partir de cuantos frames refrescados con hsync=0 hay que decir que la pantalla queda en negro
#define LIMIT_FAST_FRAME_BLACK 5

//normalmente 400
//309 para wrx/nucinv16.p
//#define NORMAL_TIMEOUT_LINEA_VSYNC 400

//normalmente 280

//A partir de vsync si inicia en 304 , mazogs da imagen estable
//nucinv16.p necesita 304 maximo. mas , pierde vsync
//#define MINIMA_LINEA_ADMITIDO_VSYNC 304



//#define MINIMA_LINEA_ADMITIDO_VSYNC 100


//TODO en teoria esto son 7.5 lineas
#define ZX8081_LINEAS_SUP_NO_USABLES 8

extern void zx8081_reset_electron_line_by_vsync(void);
extern void generar_zx8081_hsync(void);
extern int zx8081_read_port_a0_low(z80_byte puerto_h);
extern void zx8081_out_any_port_video_stuff(void);
extern void zx8081_if_admited_vsync(void);

extern z80_byte zx80801_last_sprite_video;
extern int zx80801_last_sprite_video_tinta;
extern int zx80801_last_sprite_video_papel;

extern void ula_zx8081_time_event(int delta);

extern int ula_zx81_time_event_t_estados;

extern void generar_zx80_hsync(void);
extern void generar_zx81_hsync(int tiempo);

extern void generate_zx81_delayed_hsync(int tiempo,int generate_after);



//8k de RAM en 8192-163783
extern z80_bit ram_in_8192;

extern z80_bit ram_in_49152;

extern z80_bit ram_in_32768;

//WRX hi-res mode
extern z80_bit wrx_present;

extern void adjust_zx8081_electron_position(int delta);


extern void enable_wrx(void);
extern void disable_wrx(void);

extern void enable_ram_in_32768(void);

extern void enable_ram_in_49152(void);

extern z80_int get_ramtop_with_rampacks(void);

extern void set_ramtop_with_rampacks(void);

extern z80_int zx8081_get_total_ram_with_rampacks(void);

extern int da_amplitud_speaker_zx8081(void);

//extern int inicio_pulso_vsync_t_estados;
extern int longitud_pulso_vsync;
extern int longitud_pulso_vsync_t_estados_antes;

extern int minimo_duracion_vsync;


extern int vsync_per_second;
extern int last_vsync_per_second;

extern z80_bit autodetect_wrx;

extern void set_zx8081_ramtop(z80_byte valor);


extern z80_bit chroma81;

extern z80_bit autodetect_chroma81;

extern z80_byte chroma81_port_7FEF;

extern void enable_chroma81(void);

extern void disable_chroma81(void);


extern int color_es_chroma(void);

extern z80_byte zx8081_last_port_write_value;

extern z80_byte ascii_to_zx81(z80_byte c);
extern z80_byte ascii_to_zx80(z80_byte c);
extern z80_int zx8081_get_standard_ram(void);

extern void ula_zx80_time_event(int delta);
extern void ula_zx81_time_event(int delta);
extern void zx81_enable_nmi_generator(void);
extern void zx81_disable_nmi_generator(void);

extern void zx8081_timer_vsync_detection(void);

#define WAITMAP_SIZE 207
extern int get_waitmap_value(int pos);

#endif
