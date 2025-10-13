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

#ifndef MENU_ITEMS_H
#define MENU_ITEMS_H

#include "cpu.h"
#include "sensors.h"
#include "menu_debug_cpu.h"


extern void menu_poke(MENU_ITEM_PARAMETERS);


extern void menu_zxvision_test(MENU_ITEM_PARAMETERS);

extern int menu_cond_ay_chip(void);
//extern int menu_cond_ay_or_sn_chip(void);
extern int menu_cond_i8049_chip(void);
extern int last_debug_poke_dir;
extern void menu_debug_poke(MENU_ITEM_PARAMETERS);
extern void menu_debug_cpu_resumen_stats(MENU_ITEM_PARAMETERS);
extern void menu_about_core_statistics(MENU_ITEM_PARAMETERS);
extern void menu_about_changelog_common(int show_err_if_big);
extern void menu_ay_registers(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_msx_videoregisters(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_msx_spritenav(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_msx_tilenav(MENU_ITEM_PARAMETERS);
extern void menu_audio_new_waveform(MENU_ITEM_PARAMETERS);
extern void menu_debug_new_visualmem(MENU_ITEM_PARAMETERS);
extern void menu_audio_new_ayplayer(MENU_ITEM_PARAMETERS);
extern int menu_audio_new_ayplayer_si_mostrar(void);
extern void menu_debug_hexdump(MENU_ITEM_PARAMETERS);
extern void menu_osd_adventure_keyboard(MENU_ITEM_PARAMETERS);
extern void menu_osd_adventure_keyboard_next(void);
extern void menu_display_total_palette(MENU_ITEM_PARAMETERS);

extern int menu_sound_wave_llena;

extern int ayplayer_force_refresh;

extern void menu_debug_disassemble(MENU_ITEM_PARAMETERS);
extern void menu_debug_assemble(MENU_ITEM_PARAMETERS);


extern void menu_zxdesktop_trash_empty(MENU_ITEM_PARAMETERS);

extern void menu_toy_follow_mouse(MENU_ITEM_PARAMETERS);
extern void menu_toys_zxlife(MENU_ITEM_PARAMETERS);
extern void process_switcher_sync_immutable_setting(void);
extern void process_switcher_sync_always_visible_setting(void);
extern void process_switcher_sync_always_left_bottom_setting(void);
extern void menu_ascii_table(MENU_ITEM_PARAMETERS);

extern void menu_process_switcher(MENU_ITEM_PARAMETERS);
extern void menu_textadv_loc_image(MENU_ITEM_PARAMETERS);
extern void menu_textadv_loc_image_tell_show_creating_image(void);

extern void menu_settings_display(MENU_ITEM_PARAMETERS);

extern void menu_draw_background_windows(MENU_ITEM_PARAMETERS);
extern void menu_debug_cpu_stats(MENU_ITEM_PARAMETERS);


extern void menu_ext_desktop_settings(MENU_ITEM_PARAMETERS);
extern void menu_cpu_transaction_log(MENU_ITEM_PARAMETERS);

extern void menu_debug_view_sprites(MENU_ITEM_PARAMETERS);

extern void menu_realtape_record_input(MENU_ITEM_PARAMETERS);
extern void menu_realtape_record_input_write_byte(char valor);
extern void menu_input_spectrum_analyzer(MENU_ITEM_PARAMETERS);

extern char *menu_debug_sprites_change_ptr_historial[];


extern char *menu_debug_poke_address_historial[];

extern char *menu_debug_poke_value_historial[];

extern void menu_breakpoint_fired(char *s);


extern void menu_ay_partitura(MENU_ITEM_PARAMETERS);
extern void menu_record_mid(MENU_ITEM_PARAMETERS);

extern void menu_direct_midi_output(MENU_ITEM_PARAMETERS);
extern void menu_ay_mixer(MENU_ITEM_PARAMETERS);
extern void menu_i8049_mixer(MENU_ITEM_PARAMETERS);
extern void menu_uartbridge(MENU_ITEM_PARAMETERS);
extern void menu_network(MENU_ITEM_PARAMETERS);


extern void menu_debug_change_memory_zone_splash(void);

extern void menu_common_connect_print(zxvision_window *w,char *texto);
extern int contador_menu_zeng_connect_print;

extern void menu_zeng_send_message(MENU_ITEM_PARAMETERS);
extern void menu_network_traffic(MENU_ITEM_PARAMETERS);
extern void menu_video_output(MENU_ITEM_PARAMETERS);

extern void menu_mmc_divmmc(MENU_ITEM_PARAMETERS);
extern void menu_storage_diviface_eprom_write_jumper(MENU_ITEM_PARAMETERS);
extern void menu_storage_mmc_autoconfigure_tbblue(MENU_ITEM_PARAMETERS);
extern void menu_storage_mmc_emulation(MENU_ITEM_PARAMETERS);
extern void menu_storage_divmmc_mmc_ports_emulation(MENU_ITEM_PARAMETERS);
extern void menu_storage_divmmc_diviface(MENU_ITEM_PARAMETERS);
extern void menu_ide_divide(MENU_ITEM_PARAMETERS);
extern void menu_audio(MENU_ITEM_PARAMETERS);
extern void menu_debug_main(MENU_ITEM_PARAMETERS);
extern void menu_debug_load_binary(MENU_ITEM_PARAMETERS);
extern void menu_debug_save_binary(MENU_ITEM_PARAMETERS);
extern void menu_snapshot(MENU_ITEM_PARAMETERS);
extern void menu_snapshot_quickload(MENU_ITEM_PARAMETERS);
extern void menu_storage(MENU_ITEM_PARAMETERS);
extern void menu_storage_tape(MENU_ITEM_PARAMETERS);
extern void menu_plusthreedisk(MENU_ITEM_PARAMETERS);

extern void menu_video_layers(MENU_ITEM_PARAMETERS);

extern void menu_specnext_audio_dac(MENU_ITEM_PARAMETERS);

extern void menu_debug_dma(MENU_ITEM_PARAMETERS);

extern void menu_msxcart(MENU_ITEM_PARAMETERS);
extern void menu_z88_slot_insert(MENU_ITEM_PARAMETERS);

extern void menu_zxuno_spi_flash(MENU_ITEM_PARAMETERS);
extern void menu_realtape_pause_unpause(MENU_ITEM_PARAMETERS);
extern void menu_smartload(MENU_ITEM_PARAMETERS);
extern void menu_onscreen_keyboard(MENU_ITEM_PARAMETERS);
extern void menu_principal_salir_emulador(MENU_ITEM_PARAMETERS);
extern void menu_exit_emulator(MENU_ITEM_PARAMETERS);
extern void menu_inicio_bucle_main(void);

extern void menu_reinsert_real_tape(void);
extern int menu_tape_input_insert_cond(void);
extern int menu_realtape_cond(void);

extern void menu_reinsert_std_tape(void);

extern z80_bit save_snapshot_file_on_exit_dialog;

extern int menu_zeng_send_message_cond(void);


extern int menu_zsock_http(char *host, char *url,int *http_code,char **mem,int *t_leidos, char **mem_after_headers,
            int skip_headers,char *add_headers,int use_ssl,char *redirect_url,char *ssl_sni_host_name);

extern int menu_download_file(char *host,char *url,char *archivo_temp,int ssl_use,int estimated_maximum_size,char *ssl_sni_host_name);

extern void menu_display_settings(MENU_ITEM_PARAMETERS);

extern void menu_ay_pianokeyboard(MENU_ITEM_PARAMETERS);

extern void menu_beeper_pianokeyboard(MENU_ITEM_PARAMETERS);

extern void menu_debug_tsconf_tbblue_msx(MENU_ITEM_PARAMETERS);

extern void menu_windows(MENU_ITEM_PARAMETERS);

extern void menu_help_show_keyboard(MENU_ITEM_PARAMETERS);

extern void menu_audio_chip_info(MENU_ITEM_PARAMETERS);



extern void menu_ql_mdv_flp(MENU_ITEM_PARAMETERS);

extern void menu_debug_unnamed_console(MENU_ITEM_PARAMETERS);

extern void menu_memory_cheat(MENU_ITEM_PARAMETERS);

extern void menu_audio_general_sound(MENU_ITEM_PARAMETERS);

extern void menu_debug_ioports(MENU_ITEM_PARAMETERS);

extern void menu_about_new(MENU_ITEM_PARAMETERS);

extern void menu_debug_load_source_code(MENU_ITEM_PARAMETERS);

extern void menu_debug_unload_source_code(MENU_ITEM_PARAMETERS);

extern void menu_snapshot_rewind(MENU_ITEM_PARAMETERS);

extern menu_z80_moto_int menu_debug_hexdump_direccion;



extern int menu_debug_hexdump_with_ascii(char *dumpmemoria,menu_z80_moto_int dir_leida,int bytes_por_linea,z80_byte valor_xor,menu_debug_hexdump_store_differences *diferencias);

extern char *menu_debug_hexdump_change_ptr_historial[];

extern void menu_find(MENU_ITEM_PARAMETERS);

extern void menu_debug_view_basic_variables(MENU_ITEM_PARAMETERS);

extern void menu_debug_view_sensors(MENU_ITEM_PARAMETERS);

extern void menu_display_window_list(MENU_ITEM_PARAMETERS);

extern void menu_accessibility_settings(MENU_ITEM_PARAMETERS);

extern void menu_ext_desk_settings_enable(MENU_ITEM_PARAMETERS);

extern void menu_visual_realtape(MENU_ITEM_PARAMETERS);

extern void menu_machine_selection(MENU_ITEM_PARAMETERS);

extern void menu_debug_machine_info(MENU_ITEM_PARAMETERS);

extern void menu_help(MENU_ITEM_PARAMETERS);

extern void menu_debug_input_file_keyboard(MENU_ITEM_PARAMETERS);

extern void menu_debug_reset(MENU_ITEM_PARAMETERS);

extern void menu_shortcuts_window(MENU_ITEM_PARAMETERS);



extern void menu_debug_file_utils(MENU_ITEM_PARAMETERS);

extern void menu_in_memoriam_david_about(MENU_ITEM_PARAMETERS);

extern void menu_process_f_functions_by_action_name(int id_funcion,int si_pulsado_icono_zxdesktop,int id_tecla_f_pulsada,int si_pulsado_boton_redefinido,int numero_boton);



extern int menu_inicio_opcion_seleccionada;

extern int force_menu_dibuja_menu_recorrer_menus;



extern void menu_realtape_empty_buffer(void);

extern void menu_visual_cassette_tape(MENU_ITEM_PARAMETERS);
extern void menu_first_start_wizard(void);

extern int menu_debug_hexdump_add_follow_expression(char *string_texto);
extern int menu_debug_hexdump_follow_expression_defined(void);
extern void menu_debug_hexdump_get_follow_expression_string(char *string_texto);

extern void menu_view_basic_listing(MENU_ITEM_PARAMETERS);
extern void menu_view_gosub_stack(MENU_ITEM_PARAMETERS);
extern void menu_view_basic_variables(MENU_ITEM_PARAMETERS);

extern int menu_inicio_mostrar_main_menu(int salir_menu);

struct s_menu_debug_view_sensors_list {
    char short_name[SENSORS_MAX_SHORT_NAME];
    int fila;
    int columna;
    //tipo de widget
    int tipo;
    int valor_en_vez_de_perc;
};

typedef struct s_menu_debug_view_sensors_list menu_debug_view_sensors_list;

//5x3
//0=fila 0, columna 0
//1=fila 0, columna 1
//...
//6=fila 1, columna 0
#define MENU_VIEW_SENSORS_TOTAL_COLUMNS 5
#define MENU_VIEW_SENSORS_TOTAL_ROWS 3

#define MENU_VIEW_SENSORS_TOTAL_ELEMENTS (MENU_VIEW_SENSORS_TOTAL_COLUMNS*MENU_VIEW_SENSORS_TOTAL_ROWS)




extern menu_debug_view_sensors_list menu_debug_view_sensors_list_sensors[];

extern struct s_zxdesktop_lowericons_info zdesktop_lowericons_array[];

extern int zxdesktop_icon_tape_inverse;
extern int zxdesktop_icon_tape_real_inverse;
extern int zxdesktop_icon_mmc_inverse;
extern int zxdesktop_icon_mmc_inverse_second;
extern int zxdesktop_icon_plus3_inverse;
extern int zxdesktop_icon_betadisk_inverse;
extern int zxdesktop_icon_ide_inverse;
extern int zxdesktop_icon_zxpand_inverse;
extern int zxdesktop_icon_mdv1_inverse;
extern int zxdesktop_icon_mdv2_inverse;
extern int zxdesktop_icon_mdv3_inverse;
extern int zxdesktop_icon_mdv4_inverse;
extern int zxdesktop_icon_flp1_inverse;
extern int zxdesktop_icon_dandanator_inverse;
extern int zxdesktop_icon_zxunoflash_inverse;
extern int zxdesktop_icon_zxmmcplusflash_inverse;
extern int zxdesktop_icon_hilow_inverse;


#define GENERIC_VISUALTAPE_COLOR_FONDO AMIGAOS_COLOUR_blue
#define VISUALTAPE_COLOR_MARCO 7
#define VISUALTAPE_COLOR_RODILLOS 15

#define GENERIC_VISUALTAPE_ANCHO_CINTA 1000
#define GENERIC_VISUALTAPE_ALTO_CINTA 630
#define VISUALTAPE_RODILLO_ARRASTRE_IZQ_X 280

#define GENERIC_VISUALTAPE_ROLLO_DERECHO_X (GENERIC_VISUALTAPE_ANCHO_CINTA-VISUALTAPE_RODILLO_ARRASTRE_IZQ_X)


//radios para cantidad de cinta enrollada
//para un maximo de una cinta de 90
#define GENERIC_VISUALTAPE_ROLLO_MAX_RADIO 250
#define GENERIC_VISUALTAPE_ROLLO_MIN_RADIO 110


//Rodillos arrastre
#define VISUALTAPE_RODILLO_ARRASTRE_MAX_RADIO 110
#define VISUALTAPE_RODILLO_ARRASTRE_MIN_RADIO 55

#define VISUALTAPE_RODILLO_ARRASTRE_DER_X (GENERIC_VISUALTAPE_ANCHO_CINTA-VISUALTAPE_RODILLO_ARRASTRE_IZQ_X)
#define VISUALTAPE_RODILLO_ARRASTRE_Y 290

#define VISUALTAPE_RODILLO_FIJO_IZQUIERDO_X 50
#define VISUALTAPE_RODILLO_FIJO_DERECHO_X (GENERIC_VISUALTAPE_ANCHO_CINTA-50)
#define VISUALTAPE_RODILLO_FIJO_Y (GENERIC_VISUALTAPE_ALTO_CINTA-100)
#define VISUALTAPE_RODILLO_FIJO_RADIO 10

//Rodillos inferiores
#define VISUALTAPE_RODILLO_MOVIL_IZQUIERDO_X 90
#define VISUALTAPE_RODILLO_MOVIL_DERECHO_X (GENERIC_VISUALTAPE_ANCHO_CINTA-90)
#define VISUALTAPE_RODILLO_MOVIL_Y (GENERIC_VISUALTAPE_ALTO_CINTA-50)
#define VISUALTAPE_RODILLO_MOVIL_RADIO 40
#define VISUALTAPE_RODILLO_MOVIL_RADIO_INTERIOR 6

extern void menu_generic_visualtape(zxvision_window *w,
    int real_width,int real_height,int offset_x,int offset_y,
    int porcentaje_cinta_izquierdo,int porcentaje_cinta_derecha,
    int antes_porcentaje_cinta_izquierdo,int antes_porcentaje_cinta_derecha,
    int grados_rodillos,int antes_grados_rodillos,int redibujar_rollos,int redibujar_parte_estatica,int redibujar_rodillos_arrastre,
    int temblor,int semaforos_hilow,int pestanyas_escritura);


#endif

