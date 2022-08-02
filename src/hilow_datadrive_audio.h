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

#ifndef HILOW_DATADRIVE_AUDIO_H
#define HILOW_DATADRIVE_AUDIO_H

#include "cpu.h"

extern int hilow_read_audio_modo_verbose;
extern int hilow_read_audio_modo_verbose_extra;
extern int hilow_read_audio_directo_a_pista;
extern int hilow_read_audio_ejecutar_sleep;
extern int hilow_read_audio_leer_cara_dos;
extern int hilow_read_audio_autoajustar_duracion_bits;
extern int hilow_read_audio_invertir_senyal;
extern int hilow_read_audio_minimo_variacion;
extern int hilow_read_audio_autocorrect;

extern long long int hilow_read_audio_tamanyo_archivo_audio;
extern z80_byte *hilow_read_audio_read_hilow_memoria_audio;
extern z80_byte *hilow_read_audio_hilow_ddh;
extern z80_byte hilow_read_audio_buffer_result[];
extern z80_byte hilow_read_audio_buffer_sector_five_byte[];
extern z80_byte hilow_read_audio_buffer_label[];


extern int hilow_read_audio_lee_sector(int posicion,int *total_bytes_leidos,int *p_sector);
extern int hilow_read_audio_buscar_inicio_sector(int posicion);
extern void hilow_read_audio_pausa(int segundos);
extern long long int hilow_read_audio_get_file_size(char *nombre);
extern void hilow_read_audio_espejar_sonido(z80_byte *puntero,int tamanyo);
extern void hilow_read_audio_write_sector_to_memory(int sector);
extern int hilow_read_audio_warn_if_sector_mismatch(int sector);
extern void hilow_read_audio_print_mostrar_ids_sector(void);

extern void hilow_read_audio_reset_buffer_label(void);
extern void hilow_read_audio_reset_buffer_sector_five_byte(void);

extern int hilow_read_audio_lee_sector_bytes_leidos;

//Funcion callback para algunas acciones
typedef void (*hilow_read_audio_callback_function)(int value,int posicion);

extern hilow_read_audio_callback_function hilow_read_audio_byteread_callback;
extern hilow_read_audio_callback_function hilow_read_audio_byte_output_write_callback;
extern hilow_read_audio_callback_function hilow_read_audio_bit_output_write_callback;
extern hilow_read_audio_callback_function hilow_read_audio_probably_sync_error_callback;
extern int hilow_read_audio_current_phase;


#define HILOW_LONGITUD_FINAL_SECTOR 28    
extern z80_byte hilow_read_audio_buffer_end_sector[];


//Fases en las que esta
#define HILOW_READ_AUDIO_PHASE_NONE                     0
#define HILOW_READ_AUDIO_PHASE_SEARCHING_SECTOR_MARKS   1
#define HILOW_READ_AUDIO_PHASE_READING_SECTOR_MARKS     2
#define HILOW_READ_AUDIO_PHASE_SEARCHING_SECTOR_LABEL   3
#define HILOW_READ_AUDIO_PHASE_READING_SECTOR_LABEL     4
#define HILOW_READ_AUDIO_PHASE_SEARCHING_SECTOR_DATA    5

//Este realmente se activa despues de que se pase del searching sector data
#define HILOW_READ_AUDIO_PHASE_READING_SECTOR_DATA      6

#endif
