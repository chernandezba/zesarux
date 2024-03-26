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

#ifndef SN76489AN_H
#define SN76489AN_H

#include "cpu.h"




extern z80_byte sn_chip_registers[];

extern z80_bit sn_chip_present;

extern z80_byte sn_last_audio_channel_frequency;

extern void sn_set_register_port(z80_byte value);

extern void sn_set_value_register(z80_byte value);

extern char da_output_sn(void);

extern void sn_chip_siguiente_ciclo(void);

extern void sn_set_noise_type(z80_byte tipo);

extern void sn_set_volume_noise(z80_byte volume);

extern void sn_set_volume_tone_channel(z80_byte canal,z80_byte volumen_final);

extern int sn_retorna_frecuencia(int registro);
extern int sn_retorna_frecuencia_valor_registro(z80_byte value_l,z80_byte value_h);

extern void sn_out_port_sound(z80_byte value);

extern void sn_establece_frecuencias_todos_canales(void);


//1'7734*1000000 (Hz) en Spectrum
//En Amstrad, 1 MHz
//En zonx zx81 , 1625000
//En zxpand 1625 tambien?
//#define FRECUENCIA_SN   1773400


#define FRECUENCIA_SN (sn_chip_frequency)

//#define FRECUENCIA_COLECO_SN (3500000/32)


#define FRECUENCIA_COLECO_SN 3579545
#define FRECUENCIA_SG1000_SN 3579545
#define FRECUENCIA_SMS_SN 3579545

#define SN_DIVISOR_FRECUENCIA 32


extern int sn_chip_frequency;



#define SN_FRECUENCIA_NOISE (FRECUENCIA_SN/2)

//#define FRECUENCIA_ENVELOPE 6927*10


extern void sn_chip_siguiente_ciclo(void);
extern void init_chip_sn(void);



extern char da_output_sn_izquierdo(void);
extern char da_output_sn_derecho(void);





extern void sn_init_filters(void);

#endif
