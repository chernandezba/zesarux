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

#ifndef AUDIO_AYPLAYER_H
#define AUDIO_AYPLAYER_H

//Para usar PATH_MAX
#include "zesarux.h"

extern int audio_ay_player_load(char *filename);
extern z80_byte *audio_ay_player_mem;
extern z80_byte ay_player_pista_actual;
extern void ay_player_pause_unpause(void);
extern z80_bit ay_player_paused;

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
extern z80_int ay_player_add_to_track;
extern z80_bit ay_player_repeat_file;

extern z80_bit ay_player_cpc_mode;
extern z80_bit ay_player_show_info_console;
extern z80_bit ay_player_shuffle_mode;
extern z80_bit ay_player_silence_detection;

extern int ay_player_silence_detection_counter;

//Gestion de playlist para ay player
struct s_ay_player_playlist_item {
    char nombre[PATH_MAX];

    int marcado;

    struct s_ay_player_playlist_item *next_item;
};

typedef struct s_ay_player_playlist_item ay_player_playlist_item;

extern ay_player_playlist_item *ay_player_first_item_playlist;

extern void ay_player_get_duration_current_song(z80_byte *minutos_total,z80_byte *segundos_total,z80_byte *decimas_segundos_total);
extern void ay_player_get_elapsed_current_song(z80_byte *minutos,z80_byte *segundos,z80_byte *decimas_segundos);
extern z80_byte ay_player_version(void);
extern z80_byte ay_player_release_file_version(void);
extern int ay_player_size(void);
extern void ay_player_playlist_init(void);
extern int ay_player_playlist_get_total_elements(void);
extern void ay_player_playlist_add(char *archivo);
extern void ay_player_playlist_remove(int position);
extern void ay_player_playlist_remove_all(void);
extern void ay_player_playlist_get_item(int position,char *nombre);
extern void ay_player_load_playlist(char *archivo_playlist);
extern void ay_player_save_playlist(char *destination_file,int marked_items,int append);
extern int ay_player_playlist_item_actual;
extern char ay_player_filename_playing[];
extern void ay_player_play_current_item(void);
extern void ay_player_add_directory_playlist(char *directorio);
extern void ay_player_add_file(char *archivo);
extern void ay_player_next_file(void);
extern void ay_player_previous_file(void);
extern void ay_player_play_this_item(int item);
extern void ay_player_start_playing_all_items(void);
extern ay_player_playlist_item *ay_player_search_item(int position);
extern void ay_player_mark_unmark_this_item(int position);


#endif