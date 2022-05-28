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

#ifndef HILOW_AUDIO_H
#define HILOW_AUDIO_H

#include "cpu.h"

extern int hilow_read_audio_modo_verbose;

extern int hilow_read_audio_modo_verbose_extra;

extern int hilow_read_audio_directo_a_pista;

extern int hilow_read_audio_ejecutar_sleep;

extern int hilow_read_audio_completamente_automatico;

extern int hilow_read_audio_leer_cara_dos;

extern int hilow_read_audio_autoajustar_duracion_bits;

extern int hilow_read_audio_lee_sector_unavez(int posicion,int *repetir,int *total_bytes_leidos);

extern int hilow_read_audio_lee_sector(int posicion,int *total_bytes_leidos);

extern int hilow_read_audio_lee_byte(int posicion,z80_byte *byte_salida);

extern int hilow_read_audio_buscar_inicio_sector(int posicion);

extern long int hilow_read_audio_tamanyo_archivo_audio;

extern z80_byte *hilow_read_audio_read_hilow_memoria_audio;

extern z80_byte *hilow_read_audio_hilow_ddh;

extern void hilow_read_audio_pausa(int segundos);

extern long int hilow_read_audio_get_file_size(char *nombre);

extern void hilow_read_audio_espejar_sonido(z80_byte *puntero,int tamanyo);


#endif
