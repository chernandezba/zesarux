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
extern z80_bit hsync_generator_active;
extern z80_bit video_fast_mode_emulation;
extern z80_byte video_fast_mode_next_frame_black;

//extern z80_bit video_zx8081_shows_vsync_on_display;

extern z80_bit simulate_lost_vsync;



extern z80_int ramtop_zx8081;

extern z80_bit zx8081_vsync_sound;

extern int timeout_linea_vsync;



extern z80_bit zx8081_detect_vsync_sound;

extern int zx8081_detect_vsync_sound_counter;

#define ZX8081_DETECT_VSYNC_SOUND_COUNTER_MAX 7

//extern z80_bit beta_zx8081_video;

//extern int temp_final_linea;


//a partir de cuantos frames refrescados con hsync=0 hay que decir que la pantalla queda en negro
#define LIMIT_FAST_FRAME_BLACK 5

//normalmente 400
//309 para wrx/nucinv16.p
#define NORMAL_TIMEOUT_LINEA_VSYNC 400

//normalmente 280
#define MINIMA_LINEA_ADMITIDO_VSYNC 280


//VSYNC pulse tiene que ser al menos 400us
//VSYNC dura 6.25 lineas:
//64 us-> una linea -> 207 t_estados
//400 us-> 6.25 lineas -> 1293 t_estados
//tomamos 1100 t_estados como buenos

//#define MINIMO_DURACION_VSYNC 1100

//Normal. Para manic miner, minimo 800
//#define DEFAULT_MINIMO_DURACION_VSYNC 800

/*Nuevo valor estandard, en teoria son
A true VSync basically has a pulse of 2.5 scan lines (2.5 * 64us = 160us = 517.5 T-states at 3.25MHz). Either side are scan lines containing pre and post equalizing pulses, but these can be ignored. A VSync of 160us worked for my analogue TV using the RF connection. Since the ZX81 generates the pulse in software, it can produce any length VSync it wants. It is then a matter of whether the TV is tolerant enough to accept it.
*/

#define DEFAULT_MINIMO_DURACION_VSYNC 518



//Para ilena
//#define DEFAULT_MINIMO_DURACION_VSYNC 550

//Para HERO, 167
//#define MINIMO_DURACION_VSYNC 160



//TODO en teoria esto son 7.5 lineas
#define ZX8081_LINEAS_SUP_NO_USABLES 8

extern void generar_zx8081_vsync(void);
extern void generar_zx8081_horiz_sync(void);

//8k de RAM en 8192-163783
extern z80_bit ram_in_8192;

extern z80_bit ram_in_49152;

extern z80_bit ram_in_32768;

//WRX hi-res mode
extern z80_bit wrx_present;

//extern z80_bit wrx_mueve_primera_columna;


//extern z80_bit hrg_enabled;


//extern int offset_zx8081_t_estados;
extern int offset_zx8081_t_coordx;
extern z80_bit video_zx8081_lnctr_adjust;

//extern z80_bit manic_miner_game;

//Para el estabilizador de imagen
extern int video_zx8081_caracter_en_linea_actual;

//extern int video_zx8081_decremento_x_cuando_mayor;

extern z80_bit video_zx8081_estabilizador_imagen;

extern void enable_wrx(void);
extern void disable_wrx(void);

extern void enable_ram_in_32768(void);

extern void enable_ram_in_49152(void);

extern z80_int get_ramtop_with_rampacks(void);

extern void set_ramtop_with_rampacks(void);

extern z80_int zx8081_get_total_ram_with_rampacks(void);

extern int da_amplitud_speaker_zx8081(void);

extern int inicio_pulso_vsync_t_estados;

extern int minimo_duracion_vsync;

//extern z80_bit ejecutado_zona_pantalla;

extern int vsync_per_second;
extern int last_vsync_per_second;

extern z80_bit autodetect_wrx;

extern void set_zx8081_ramtop(z80_byte valor);


extern z80_bit chroma81;

extern z80_bit autodetect_chroma81;

extern z80_byte chroma81_port_7FEF;

extern void enable_chroma81(void);

extern void disable_chroma81(void);

extern void chroma81_return_mode1_colour(z80_int dir,z80_byte *colortinta,z80_byte *colorpapel);

extern int color_es_chroma(void);

extern z80_byte zx8081_last_port_write_value;

extern z80_byte ascii_to_zx81(z80_byte c);
extern z80_byte ascii_to_zx80(z80_byte c);
extern z80_int zx8081_get_standard_ram(void);

#endif
