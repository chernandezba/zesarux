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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "cpu.h"

#define MAX_KMOUSE_SENSITIVITY 10

#define DEFAULT_ADV_KEYBOARD_KEY_LENGTH 50

extern z80_bit menu_debug_registers_if_showscan;

extern z80_bit debug_settings_show_screen;

extern z80_bit debug_settings_show_fired_halt;

extern int debug_registers;

extern z80_bit debug_shows_invalid_opcode;

extern z80_bit debug_breakpoints_cond_behaviour;

extern int remote_debug_settings;

extern int debug_show_fired_breakpoints_type;

extern int verbose_level;

extern z80_bit remote_protocol_enabled;
extern int remote_protocol_port;
#define REMOTE_MAX_PROMPT_LENGTH 100
extern char remote_prompt_command_string[];
extern z80_bit remote_protocol_char_mode;

extern z80_bit hardware_debug_port;

extern char zesarux_zxi_hardware_debug_file[];
extern z80_bit setting_mostrar_visualmem_grafico;


extern z80_bit autosave_snapshot_on_exit;
extern z80_bit autoload_snapshot_on_start;
extern char autosave_snapshot_path_buffer[];

extern z80_bit fast_autoload;
extern z80_bit tzx_suppress_pause;
extern z80_bit realtape_algorithm_new;

extern z80_bit pd765_silent_write_protection;

extern z80_bit setting_mostrar_ay_piano_grafico;

extern int kempston_mouse_factor_sensibilidad;

extern int adventure_keyboard_key_length;

extern int adventure_keyboard_send_final_spc;

extern z80_bit use_scrcursesw;

extern z80_bit tbblue_autoconfigure_sd_asked;

extern int menu_allow_background_windows;

extern int always_force_overlay_visible_when_menu_closed;

extern z80_bit tbblue_store_scanlines;

extern z80_bit tbblue_store_scanlines_border;

extern z80_bit menu_reopen_background_windows_on_start;

extern z80_bit vdp_9918a_unlimited_sprites_line;

extern z80_bit msx_loading_stripes;

extern z80_bit msx_loading_noise_reduction;

#define MACHINE_SELECTION_TYPE_BY_MANUFACTURER 0
#define MACHINE_SELECTION_TYPE_BY_NAME 1
#define MACHINE_SELECTION_TYPE_BY_FAMILY 2

extern int setting_machine_selection_type;

//extern z80_bit ql_replace_underscore_dot;

//extern z80_bit ql_replace_underscore_dot_only_one;

//extern z80_bit ql_simular_parpadeo_cursor;

extern z80_bit ql_flp1_follow_mdv1;

extern z80_bit ql_win1_alias_mdv1;

extern z80_bit menu_file_viewer_always_hex;

extern z80_bit debug_view_basic_show_address;

extern z80_bit debug_watches_show_decimal;

extern int gui_language;

extern int zxdesktop_disable_on_full_screen;
extern int disable_border_on_full_screen;
extern int disable_footer_on_full_screen;

extern int setting_set_machine_enable_custom_rom;
extern char custom_romfile[];

extern int zxdesktop_restore_windows_after_full_screen;
extern int zxdesktop_disable_show_frame_around_display;

extern int audiochip_piano_zoom_x;
extern int audiochip_piano_zoom_y;

extern z80_bit audiosdl_use_new_callback;

extern char online_download_path[];

extern z80_bit zxdesktop_configurable_icons_enabled;

extern z80_bit zxdesktop_empty_trash_on_exit;

extern z80_bit zxdesktop_icon_show_app_open;

extern z80_bit z88_hide_keys_shortcuts;

extern z80_bit sync_clock_to_z88;

extern z80_bit menu_show_advanced_items;

extern z80_bit menu_show_simple_items;

extern z80_bit menu_show_simple_items_by_config;

extern z80_bit keyboard_swap_enter_return;

extern z80_bit zsf_snap_save_rom;

extern z80_bit xanniversary_logo;

extern z80_bit setting_process_switcher_immutable;

extern z80_bit setting_process_switcher_always_visible;

extern z80_bit setting_process_switcher_force_left_bottom;

extern z80_bit accessibility_enable_gui_sounds;

extern z80_bit zeng_online_allow_room_creation_from_any_ip;

extern z80_bit zeng_online_destroy_rooms_without_players;

extern z80_bit zeng_online_server_allow_zrcp_only_zeng_online;

extern z80_bit zeng_online_zip_compress_snapshots;

extern z80_bit zeng_online_show_footer_lag_indicator;

extern z80_bit zeng_online_allow_instant_keys;

extern z80_bit do_no_show_david_in_memoriam;

extern char alsa_capture_device[];

extern z80_bit noautoload;

extern z80_bit automount_esxdos_nex;

extern z80_bit index_menu_enabled;

extern z80_bit opcion_fast_welcome_message;

extern z80_bit opcion_no_welcome_message;

extern z80_bit zxvision_topbar_menu_enabled;

extern z80_bit zxvision_setting_use_speccy_keys;

extern z80_bit sms_swap_controls;

#endif

