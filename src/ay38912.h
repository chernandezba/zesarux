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

#ifndef AY38912_H
#define AY38912_H

#include "cpu.h"

//Maximo Chips ay presentes
#define MAX_AY_CHIPS 3

extern z80_bit ay_chip_present;
extern void out_port_ay(z80_int puerto,z80_byte value);
extern z80_byte in_port_ay(z80_byte puerto_h);


extern z80_bit ay_speech_enabled;
extern z80_bit ay_envelopes_enabled;

extern z80_byte ay_3_8912_registro_sel[];


extern z80_byte ay_3_8912_registros[][16];

//1'7734*1000000 (Hz) en Spectrum
//En Amstrad, 1 MHz
//En zonx zx81 , 1625000
//En zxpand 1625 tambien?
//#define FRECUENCIA_AY   1773400


//#define FRECUENCIA_AY (MACHINE_IS_CPC ? 1000000 : 1773400)

extern z80_bit aymidi_rs232_enabled;

#define FRECUENCIA_AY (ay_chip_frequency)

#define FRECUENCIA_SPECTRUM_AY 1773400
#define FRECUENCIA_CPC_AY      1000000
#define FRECUENCIA_ZX81_AY     1625000
#define FRECUENCIA_MSX_AY      1789772
#define FRECUENCIA_SVI_AY      1789772


extern int ay_chip_frequency;

//#define FRECUENCIA_NOISE 886700


#define AY_DIVISOR_FRECUENCIA 16

#define FRECUENCIA_NOISE (FRECUENCIA_AY/2)

#define FRECUENCIA_ENVELOPE 6927*10

extern char da_output_ay(void);
extern void ay_chip_siguiente_ciclo(void);
extern void init_chip_ay(void);

extern void ay_randomize(int chip);
extern z80_int randomize_noise[];

extern void activa_ay_chip_si_conviene(void);
extern z80_bit autoenable_ay_chip;

extern void return_envelope_name(int value,char *string);

//extern z80_bit turbosound_enabled;
extern int total_ay_chips;

extern void turbosound_disable(void);
extern void turbosound_enable(void);
extern int ay_retorna_numero_chips(void);
extern void set_total_ay_chips(int total);

extern int ay_chip_selected;

extern int ay_retorna_frecuencia(int registro,int chip);
extern int ay_retorna_frecuencia_valor_registro(z80_byte value_l,z80_byte value_h);

extern char da_output_ay_izquierdo(void);
extern char da_output_ay_derecho(void);

extern int ay3_stereo_mode;

extern int ay3_custom_stereo_A;
extern int ay3_custom_stereo_B;
extern int ay3_custom_stereo_C;

extern z80_byte ay_retorna_mixer_register(int chip);

extern z80_byte ay_filtros[];
extern void ay_init_filters(void);

extern void ay_establece_frecuencias_todos_canales(void);

#endif
