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

#ifndef TAPE_H
#define TAPE_H

#include "cpu.h"
#include "mem128.h"
#include "zvfs.h"

extern int tap_open(void);
extern int tap_close(void);
extern int tap_out_open(void);
extern int tap_out_close(void);
extern void tap_load(void);
extern void tap_save(void);

extern char *tapefile;
extern char *tape_out_file;
extern z80_bit initial_tap_load;
extern int initial_tap_sequence;
extern z80_bit noautoload;
extern z80_bit tape_any_flag_loading;

extern int is_tape_inserted(void);

//extern int tape_loading_counter;

extern int (*tape_block_open)(void);
extern int (*tape_block_read)(void *dir,int longitud);
extern int (*tape_block_readlength)(void);
extern int (*tape_block_seek)(int longitud,int direccion);

extern int (*tape_out_block_open)(void);
extern int (*tape_out_block_close)(void);

extern int (*tape_block_save)(void *dir,int longitud);
extern void (*tape_block_begin_save)(int longitud,z80_byte flag);

extern void tape_init(void);
extern void tape_out_init(void);

extern FILE *ptr_mycinta;

extern int tap_load_detect(void);
extern int tap_save_detect(void);



//extern z80_bit tape_load_inserted;
//extern z80_bit tape_save_inserted;
extern int tape_loadsave_inserted;
#define TAPE_LOAD_INSERTED 1
#define TAPE_SAVE_INSERTED 2


extern void insert_tape_load(void);
extern void insert_tape_save(void);
extern void eject_tape_load(void);
extern void eject_tape_save(void);

extern int tape_pause;

extern int tape_loading_counter;
extern void draw_tape_text(void);
//extern void delete_tape_text(void);

extern void gestionar_autoload_spectrum(void);
extern void gestionar_autoload_cpc(void);
extern void gestionar_autoload_sam(void);

extern int autoload_spectrum_loadpp_mode;

extern FILE *ptr_realtape;

extern char realtape_name_rwa[];

extern void realtape_get_byte(void);

extern char realtape_last_value;

extern void realtape_insert(void);
extern void realtape_eject(void);

extern z80_bit realtape_inserted;
extern z80_bit realtape_playing;
extern z80_bit realtape_loading_sound;

extern void realtape_rewind_five(void);
extern void realtape_ffwd_five(void);
extern void realtape_rewind_one(void);
extern void realtape_ffwd_one(void);
extern void realtape_rewind_begin(void);

extern int realtape_visual_detected_tape_type;
extern char *realtape_name;

extern char realtape_volumen;

extern char realtape_wave_offset;

//extern z80_bit autodetect_loaders;
extern z80_bit accelerate_loaders;
extern z80_bit tape_auto_rewind;

extern int tap_load_detect_ace(void);
extern void tap_load_ace(void);
extern int tap_save_detect_ace(void);
extern void tap_save_ace(void);

extern void realtape_start_playing(void);
extern void realtape_stop_playing(void);

extern z80_bit standard_to_real_tape_fallback;

extern void tape_detectar_realtape(void);

typedef enum acceleration_mode_t {
  ACCELERATION_MODE_NONE = 0,
  ACCELERATION_MODE_INCREASING,
  ACCELERATION_MODE_DECREASING,
} acceleration_mode_t;

extern acceleration_mode_t acceleration_mode;
extern size_t acceleration_pc;

extern void tape_check_known_loaders(void);

extern void draw_tape_text_top_speed(void);

extern void realtape_print_footer(void);

extern long long int realtape_file_size;
extern long long int realtape_file_size_counter;



extern void realtape_pause_unpause(void);
extern int realtape_get_current_bit_playing(void);
extern int realtape_algorithm_new_noise_reduction;

#define REALTAPE_VISUAL_MAX_SIZE 4096

//almacenar los datos que se veran en la ventana de visual real tape. Valores maximo y minimo ([0] minimo, [1] maximo)
extern z80_byte realtape_visual_data[REALTAPE_VISUAL_MAX_SIZE*2][2];

extern int realtape_visual_total_used;

extern int realtape_get_seconds_numbytes(long long int numero);
extern int realtape_get_elapsed_seconds(void);
extern int realtape_get_total_seconds(void);
extern void init_visual_real_tape(void);

extern char visual_realtape_textbrowse[];
extern long visual_realtape_array_positions[];

#endif
