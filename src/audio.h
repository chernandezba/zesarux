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

#ifndef AUDIO_H
#define AUDIO_H

#include <dirent.h>
#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "cpu.h"


//Por el tema de usar PATH_MAX en windows
#ifdef MINGW
#include <stdlib.h>
#define PATH_MAX MAX_PATH
#define NAME_MAX MAX_PATH
#endif



//lineas de cada pantalla. al final de cada linea se guarda el bit a enviar al altavoz
//#define AUDIO_BUFFER_SIZE 312

//Dice cuantos frames enteros de pantalla hay que esperar antes de enviar el audio
//en ZXSpectr usamos buffers de 10 frames
//pero parece que aqui como el sincronismo no es tan bueno, no se puede hacer tanto
#define FRAMES_VECES_BUFFER_AUDIO 5
//#define FRAMES_VECES_BUFFER_AUDIO 10

#define AUDIO_BUFFER_SIZE (312*FRAMES_VECES_BUFFER_AUDIO)

//#define FRECUENCIA_SONIDO AUDIO_BUFFER_SIZE*50

//Frecuencia normal constante sin tener en cuenta velocidad cpu. Se usa en chip AY como referencia y en pocos sitios mas
#define FRECUENCIA_CONSTANTE_NORMAL_SONIDO (312*50)

//temp
//#define FRECUENCIA_SONIDO 312*50

#define FRECUENCIA_SONIDO frecuencia_sonido_variable

//Frecuencia teniendo en cuenta la velocidad de la cpu
extern int frecuencia_sonido_variable;



//duracion en ms de un buffer de sonido, normalmente  (1000.0/((FRECUENCIA_SONIDO)/AUDIO_BUFFER_SIZE))
//usado en audiodsp con phtreads
#define AUDIO_MS_DURACION 100


#define AMPLITUD_BEEPER 50
#define AMPLITUD_BEEPER_GRABACION_ZX8081 25
#define AMPLITUD_TAPE 2

extern int amplitud_speaker_actual_zx8081;
extern int amplitud_speaker_actual_msx;
extern int amplitud_speaker_actual_svi;

extern int (*audio_init) (void);
extern int (*audio_thread_finish) (void);
extern void (*audio_end) (void);
extern void (*audio_send_frame) (char *buffer);
extern void (*audio_get_buffer_info) (int *buffer_size,int *current_size);
extern char *audio_buffer;
extern char *audio_buffer_playback;

extern z80_bit audio_noreset_audiobuffer_full;

extern char audio_new_driver_name[];

extern void audio_set_driver_name(char *nombre);

extern char *audio_buffer_one;
extern char *audio_buffer_two;
//extern char audio_buffer_oneandtwo[AUDIO_BUFFER_SIZE*2];

extern char audio_buffer_one_assigned[];
extern char audio_buffer_two_assigned[];


extern int audio_buffer_indice;
extern z80_bit audio_buffer_switch;
extern void set_active_audio_buffer(void);

extern z80_byte ultimo_altavoz;
extern z80_bit bit_salida_sonido_zx8081;

extern char value_beeper;


extern z80_bit interrupt_finish_sound;

//5 segundos hasta detectar silencio
#define SILENCE_DETECTION_MAX 50*5


extern int silence_detection_counter;
extern int beeper_silence_detection_counter;

//aofile
extern char *aofilename;
extern void init_aofile(void);
extern void close_aofile(void);
extern void aofile_send_frame(char *buffer);
extern z80_bit aofile_inserted;

#define AOFILE_TYPE_RAW 0
#define AOFILE_TYPE_WAV 1
extern int aofile_type;


extern z80_bit audio_playing;
extern void envio_audio(void);

//#define BEEPER_ARRAY_LENGTH 228
#define MAX_BEEPER_ARRAY_LENGTH MAX_STATES_LINE

#define CURRENT_BEEPER_ARRAY_LENGTH MAX_CURRENT_STATES_LINE

extern int buffer_beeper[];
extern int beeper_real_enabled;

extern void beeper_new_line(void);
extern char get_value_beeper_sum_array(void);
extern void set_value_beeper_on_array(char value);

extern int audiovolume;

extern z80_bit output_beep_filter_on_rom_save;

extern z80_bit output_beep_filter_alter_volume;
extern char output_beep_filter_volume;

extern int audio_adjust_volume(int valor_enviar);

struct s_nota_musical
{
        char nombre[4];
	int frecuencia; //aunque tiene decimales, para lo que necesitamos no hacen falta
    //int ql_beep_pitch; //Valor de pitch asociado al sonido de QL. Valor negativo si no tiene relación
};

typedef struct s_nota_musical nota_musical;

#define NOTAS_MUSICALES_OCTAVAS 10
#define NOTAS_MUSICALES_NOTAS_POR_OCTAVA 12
#define MAX_NOTAS_MUSICALES (NOTAS_MUSICALES_OCTAVAS*NOTAS_MUSICALES_NOTAS_POR_OCTAVA)

extern char *get_note_name(int frecuencia);
extern int get_mid_number_note(char *str);
extern void get_note_values(char *texto,int *nota_final,int *si_sostenido,int *octava);
extern char *get_note_name_by_index(int index);
extern int get_note_frequency_from_ql_pitch(int pitch);
extern int get_note_frequency_by_index(int index);
extern int get_note_index_from_ql_pitch(int pitch);
extern int set_audiodriver_null(void);
extern void fallback_audio_null(void);
extern void audio_empty_buffer(void);

extern int mid_mete_cabecera(z80_byte *midi_file,int pistas,int division);
extern int mid_mete_inicio_pista(z80_byte *mem,int division);
extern int mid_mete_evento_final_pista(unsigned char *mem);
extern int mid_mete_nota(z80_byte *mem,int silencio_anterior,int duracion,int canal_midi,int keynote,int velocity);
extern void mid_mete_longitud_pista(z80_byte *mem,int longitud);
extern void mid_frame_event(void);
extern int audio_midi_output_initialized;
extern void audio_midi_output_frame_event(void);

extern int audio_midi_raw_mode;

extern int audio_midi_output_note_on(unsigned char channel, unsigned char note);
extern int audio_midi_set_instrument(unsigned char instrument);
extern void audio_midi_output_raw(z80_byte value);
extern void audio_midi_output_reset(void);

extern char *midi_instrument_list[];
extern int si_audio_silenced(void);
extern int audio_get_total_chips(void);
extern int audio_retorna_frecuencia_canal(int canal,int chip);
extern int audio_si_canal_tono(int chip,int canal);

//Para cuantas notas da esto aprox?
#define MAX_MID_EXPORT_BUFFER 1000000

extern z80_bit mid_is_recording;
extern z80_bit mid_is_paused;
extern char mid_export_file[];
extern int mid_chips_al_start;
//extern int mid_record_at_least_one;
extern int mid_notes_recorded;
extern z80_bit mid_record_noisetone;
extern int mid_has_been_initialized(void);
extern void mid_flush_file(void);
extern void mid_initialize_export(void);
extern void mid_reset_export_buffers(void);
extern int mid_max_buffer(void);
extern void mid_set_cambio_instrumento(void);
extern z80_byte mid_instrument;

extern char audio_valor_enviar_sonido;

extern char audio_valor_enviar_sonido_izquierdo;
extern char audio_valor_enviar_sonido_derecho;

extern int audio_ay_player_load(char *filename);
extern z80_byte *audio_ay_player_mem;
extern z80_byte ay_player_pista_actual;

extern z80_byte ay_player_total_songs(void);

extern int audio_ay_player_play_song(z80_byte song);

extern void ay_player_load_and_play(char *filename);

extern z80_bit ay_player_playing;

extern void ay_player_playing_timer(void);
extern void ay_player_next_track (void);
extern void ay_player_previous_track (void);

extern void ay_player_stop_player(void);


extern z80_int ay_song_length;
extern z80_int ay_song_length_counter;

extern char ay_player_file_song_name[];
extern char ay_player_file_author[];
extern char ay_player_file_misc[];


extern z80_bit ay_player_exit_emulator_when_finish;
extern z80_int ay_player_limit_infinite_tracks;
extern z80_int ay_player_limit_any_track;
extern z80_bit ay_player_repeat_file;

extern z80_bit ay_player_cpc_mode;
extern z80_bit ay_player_show_info_console;

//Gestion de playlist para ay player
struct s_ay_player_playlist_item {
    char nombre[PATH_MAX];

    struct s_ay_player_playlist_item *next_item;
};

typedef struct s_ay_player_playlist_item ay_player_playlist_item;

extern ay_player_playlist_item *ay_player_first_item_playlist;

extern void ay_player_get_duration_current_song(z80_byte *minutos_total,z80_byte *segundos_total);
extern void ay_player_get_elapsed_current_song(z80_byte *minutos,z80_byte *segundos);

extern void ay_player_playlist_init(void);
extern int ay_player_playlist_get_total_elements(void);
extern void ay_player_playlist_add(char *archivo);
extern void ay_player_playlist_remove(int position);
extern void ay_player_playlist_get_item(int position,char *nombre);
extern int ay_player_playlist_item_actual;
extern char ay_player_filename_playing[];

extern z80_byte audiodac_last_value_data;

extern z80_bit audiodac_enabled;

extern int audiodac_selected_type;

//extern z80_int audiodac_custom_port;

struct s_audiodac_type {
  char name[20];
  z80_int port;
};

typedef struct s_audiodac_type audiodac_type;

#define MAX_AUDIODAC_TYPES 5

extern audiodac_type audiodac_types[];

extern z80_bit beeper_enabled;

extern z80_bit audio_resample_1bit;

extern void audiodac_mix(void);

extern int audio_using_sdl2;

extern z80_bit silence_detector_setting;

extern void audiodac_print_types(void);

extern int audiodac_set_type(char *texto);

extern void audiodac_set_custom_port(z80_byte valor);

//extern int audioonebitspeaker_tiempo_espera;

#define TIPO_ALTAVOZ_ONEBITSPEAKER_PCSPEAKER 0
#define TIPO_ALTAVOZ_ONEBITSPEAKER_RPI_GPIO 1

extern int audioonebitspeaker_intensive_cpu_usage;

extern int audioonebitspeaker_agudo_filtro;
extern int audioonebitspeaker_agudo_filtro_limite;
extern int audioonebitspeaker_tipo_altavoz;
extern int audioonebitspeaker_rpi_gpio_pin;

struct s_audiobuffer_stats
{
	int maximo;
	int minimo;
	int medio;
	int frecuencia;
	int volumen;  //entre 0 y 127

	int volumen_escalado;  //escalado entre 0 y 15, como los volumenes del chip AY
};

typedef struct s_audiobuffer_stats audiobuffer_stats;

extern void audio_get_audiobuffer_stats(audiobuffer_stats *audiostats);

extern int audio_tone_generator;
extern char audio_tone_generator_last;
extern char audio_tone_generator_get(void);

extern char audio_change_top_speed_sound(char sonido);

extern void audio_send_mono_sample(char valor_sonido);
extern void audio_send_stereo_sample(char valor_sonido_izquierdo,char valor_sonido_derecho);

//extern z80_bit audio_driver_accepts_stereo;

extern void audiodac_send_sample_value(z80_byte value);

extern void midi_output_frame_event(void);
extern int audio_midi_output_init(void);
extern void audio_midi_output_finish(void);

extern z80_bit midi_output_record_noisetone;

#define FREQ_TOP_SPEED_CHANGE 12800

extern int audio_midi_client;
extern int audio_midi_port;
extern int audio_midi_available(void);

extern char audio_raw_midi_device_out[];

#define MAX_AUDIO_RAW_MIDI_DEVICE_OUT 128


#ifdef MINGW

//extern int windows_midi_midiport;
extern void windows_midi_output_flush_output(void);
extern int windows_mid_initialize_all(void);
extern void windows_mid_finish_all(void);
extern int windows_note_on(unsigned char channel, unsigned char note,unsigned char velocity);
extern int windows_note_off(unsigned char channel, unsigned char note,unsigned char velocity);
extern int windows_change_instrument(unsigned char instrument);
extern void windows_midi_raw(z80_byte value);
extern void windows_midi_output_reset(void);


#endif

#endif
