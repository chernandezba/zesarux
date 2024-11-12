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

#ifndef MENU_BITMAPS_H
#define MENU_BITMAPS_H


#define ZESARUX_ASCII_LOGO_ANCHO 26
#define ZESARUX_ASCII_LOGO_ALTO 26
extern char *zesarux_ascii_logo[];
extern char **get_zesarux_ascii_logo(void);
extern char **alter_zesarux_ascii_logo(char **p);
extern char *zesarux_ascii_logo_whitebright[];
extern char **get_zesarux_ascii_logo_whitebright(void);

extern char *bitmap_button_ext_desktop_smartload[];
extern char *bitmap_button_ext_desktop_snapshot[];
extern char *bitmap_button_ext_desktop_machine[];
extern char *bitmap_button_ext_desktop_set_machine[];
extern char *bitmap_button_ext_desktop_set_machine_only_arrow[];
extern char *bitmap_button_ext_desktop_audio[];
extern char *bitmap_button_ext_desktop_display[];
extern char *bitmap_button_ext_desktop_storage[];
extern char *bitmap_button_ext_desktop_help[];
extern char *bitmap_button_ext_desktop_debug[];
extern char *bitmap_button_ext_desktop_network[];
extern char *bitmap_button_ext_desktop_windows[];
extern char *bitmap_button_ext_desktop_settings[];
extern char *bitmap_button_ext_desktop_close_all_menus[];
extern char *bitmap_button_ext_desktop_exit[];

extern char *bitmap_button_ext_desktop_userdefined[];
extern char *bitmap_button_ext_desktop_fullscreen[];
extern char *bitmap_button_ext_desktop_waveform[];
extern char *bitmap_button_ext_desktop_audioregisters[];
extern char *bitmap_button_ext_desktop_audiosheet[];
extern char *bitmap_button_ext_desktop_zxeyes[];
extern char *bitmap_button_ext_desktop_watches[];
extern char *bitmap_button_ext_desktop_visualmem[];
extern char *bitmap_button_ext_desktop_memorycheat[];
extern char *bitmap_button_ext_desktop_mdvrawmap[];
extern char *bitmap_button_ext_desktop_zxlife[];
extern char *bitmap_button_ext_desktop_view_sensors[];
extern char *bitmap_button_ext_desktop_visualfloppy[];
extern char *bitmap_button_ext_desktop_audiopiano[];
extern char *bitmap_button_ext_desktop_wavepiano[];
extern char *bitmap_button_ext_desktop_aymixer[];
extern char *bitmap_button_ext_desktop_ayplayer[];
extern char *bitmap_button_ext_desktop_colour_palettes[];
extern char *bitmap_button_ext_desktop_reset[];
extern char *bitmap_button_ext_desktop_hardreset[];
extern char *bitmap_button_ext_desktop_reloadmmc[];
extern char *bitmap_button_ext_desktop_loadbinary[];
extern char *bitmap_button_ext_desktop_savebinary[];
extern char *bitmap_button_ext_desktop_snapinramrewind[];
extern char *bitmap_button_ext_desktop_snapinramffw[];
extern char *bitmap_button_ext_desktop_pause[];
extern char *bitmap_button_ext_desktop_debugcpu[];
extern char *bitmap_button_ext_desktop_ioports[];
extern char *bitmap_button_ext_desktop_geneneralsoundregisters[];
extern char *bitmap_button_ext_desktop_cpustatistics[];
extern char *bitmap_button_ext_desktop_corestatistics[];
extern char *bitmap_button_ext_desktop_debugcpu_view_adventure[];
extern char *bitmap_button_ext_desktop_text_adventure_map[];
extern char *bitmap_button_ext_desktop_hexeditor[];
extern char *bitmap_button_ext_desktop_viewsprites[];
extern char *bitmap_button_ext_desktop_tsconftbbluespritenav[];
extern char *bitmap_button_ext_desktop_tsconftbbluetilenav[];
extern char *bitmap_button_ext_desktop_shortcutshelper[];
extern char *bitmap_button_ext_desktop_debugconsole[];
extern char *bitmap_button_ext_desktop_fileutils[];
extern char *bitmap_button_ext_desktop_pauseunpausetape[];
extern char *bitmap_button_ext_desktop_reinserttape[];
extern char *bitmap_button_ext_desktop_reinsertrealtape[];
extern char *bitmap_button_ext_desktop_ocr[];
extern char *bitmap_button_ext_desktop_switchborder[];
extern char *bitmap_button_ext_desktop_switchfooter[];
extern char *bitmap_button_ext_desktop_topspeed[];
extern char *bitmap_button_ext_desktop_osdkeyboard[];
extern char *bitmap_button_ext_desktop_osdadvkeyboard[];
extern char *bitmap_button_ext_desktop_quickload[];
extern char *bitmap_button_ext_desktop_quicksave[];
extern char *bitmap_button_ext_desktop_quicksave_scr[];
extern char *bitmap_button_ext_desktop_nmi[];
extern char *bitmap_button_ext_desktop_zxunoprismswitch[];
extern char *bitmap_button_ext_desktop_nothing[];
extern char *bitmap_button_ext_desktop_zengmessage[];
extern char *bitmap_button_ext_desktop_zengonlinemessage[];
extern char *bitmap_button_ext_desktop_rewindtape[];
extern char *bitmap_button_ext_desktop_ffwdtape[];
extern char *bitmap_button_ext_desktop_joyleftright[];
extern char *bitmap_button_ext_desktop_trash[];
extern char *bitmap_button_ext_desktop_trash_not_empty[];
extern char *bitmap_button_ext_desktop_trash_open[];
extern char *bitmap_button_ext_desktop_trash_open_not_empty[];
extern char *bitmap_button_ext_desktop_file_snapshot[];
extern char *bitmap_button_ext_desktop_file_tape[];
extern char *bitmap_button_ext_desktop_file_generic_smartload[];
extern char *bitmap_button_ext_desktop_speccy_online[];
extern char *bitmap_button_ext_desktop_zx81_online[];
extern char *bitmap_button_ext_desktop_visualrealtape[];
extern char *bitmap_button_ext_desktop_poke[];
extern char *bitmap_button_ext_desktop_openwindow[];
extern char *bitmap_button_ext_desktop_processmanagement[];
extern char *bitmap_button_ext_desktop_processswitcher[];
extern char *bitmap_button_ext_desktop_videoinfo[];
extern char *bitmap_button_ext_desktop_textadvlocimage[];
extern char *bitmap_button_ext_desktop_asciitable[];
extern char *bitmap_button_ext_desktop_record_input[];
extern char *bitmap_button_ext_desktop_inspectrumanalyzer[];
extern char *bitmap_button_ext_desktop_networktrafiic[];
extern char *bitmap_button_ext_desktop_videooutput[];

extern char **zxdesktop_buttons_bitmaps[];


extern char *bitmap_lowericon_ext_desktop_cassette_active[];
extern char *bitmap_lowericon_ext_desktop_cassette_active_frametwo[];
extern char *bitmap_lowericon_ext_desktop_cassette_active_framethree[];
extern char *bitmap_lowericon_ext_desktop_cassette_active_framefour[];
extern char *bitmap_lowericon_ext_desktop_cassette_inactive[];

extern char *bitmap_lowericon_ext_desktop_cassette_std_active[];
extern char *bitmap_lowericon_ext_desktop_cassette_std_inactive[];

extern char *bitmap_lowericon_ext_desktop_mmc_active[];
extern char *bitmap_lowericon_ext_desktop_mmc_inactive[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_nonumber_active[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_nonumber_inactive[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_one_active[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_one_inactive[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_two_active[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_two_inactive[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_three_active[];
extern char *bitmap_lowericon_ext_desktop_z88_slot_three_inactive[];
extern char *bitmap_lowericon_ext_desktop_mdv_active[];
extern char *bitmap_lowericon_ext_desktop_mdv_inactive[];
extern char *bitmap_lowericon_ext_desktop_flp_active[];
extern char *bitmap_lowericon_ext_desktop_flp_inactive[];
extern char *bitmap_lowericon_ext_desktop_msx_cart_active[];
extern char *bitmap_lowericon_ext_desktop_msx_cart_inactive[];
extern char *bitmap_lowericon_ext_desktop_svi_active[];
extern char *bitmap_lowericon_ext_desktop_svi_inactive[];
extern char *bitmap_lowericon_ext_desktop_coleco_active[];
extern char *bitmap_lowericon_ext_desktop_coleco_inactive[];
extern char *bitmap_lowericon_ext_desktop_sg1000_active[];
extern char *bitmap_lowericon_ext_desktop_sg1000_inactive[];
extern char *bitmap_lowericon_ext_desktop_sms_active[];
extern char *bitmap_lowericon_ext_desktop_sms_inactive[];
extern char *bitmap_button_ext_desktop_visualmicrodrive[];

extern char *bitmap_lowericon_ext_desktop_plus3_flp_active[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_framezero[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_frameone[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_frametwo[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_framethree[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_framezero[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_frameone[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_frametwo[];
extern char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_framethree[];

extern char *bitmap_lowericon_ext_desktop_plus3_flp_inactive[];

extern char *bitmap_lowericon_ext_desktop_betadisk_active[];
extern char *bitmap_lowericon_ext_desktop_betadisk_inactive[];

extern char *bitmap_lowericon_ext_desktop_cart_timex_inactive[];
extern char *bitmap_lowericon_ext_desktop_cart_timex_active[];

extern char *bitmap_lowericon_ext_desktop_ide_active[];
extern char *bitmap_lowericon_ext_desktop_ide_inactive[];


extern char *bitmap_lowericon_ext_desktop_dandanator_active[];
extern char *bitmap_lowericon_ext_desktop_dandanator_inactive[];

extern char *bitmap_lowericon_ext_desktop_zxunoflash[];

extern char *bitmap_lowericon_ext_desktop_hilow_active[];
extern char *bitmap_lowericon_ext_desktop_hilow_inactive[];
extern char *bitmap_button_ext_desktop_hilow_visual_datadrive[];
extern char *bitmap_lowericon_ext_desktop_hilow_convert[];

extern char *bitmap_button_ext_desktop_helpkeyboard[];

extern char *bitmap_button_ext_desktop_my_machine_generic[];
extern char *bitmap_button_ext_desktop_my_machine_gomas[];
extern char *bitmap_button_ext_desktop_my_machine_zx80[];
extern char *bitmap_button_ext_desktop_my_machine_tk80[];
extern char *bitmap_button_ext_desktop_my_machine_tk82[];
extern char *bitmap_button_ext_desktop_my_machine_zx81[];
extern char *bitmap_button_ext_desktop_my_machine_ts1000[];
extern char *bitmap_button_ext_desktop_my_machine_ts1500[];

extern char *bitmap_button_ext_desktop_my_machine_cz1000[];
extern char *bitmap_button_ext_desktop_my_machine_cz1500[];
extern char *bitmap_button_ext_desktop_my_machine_cz1000_plus[];
extern char *bitmap_button_ext_desktop_my_machine_cz1500_plus[];
extern char *bitmap_button_ext_desktop_my_machine_cz2000[];
extern char *bitmap_button_ext_desktop_my_machine_cz_spectrum[];
extern char *bitmap_button_ext_desktop_my_machine_cz_spectrum_plus[];

extern char *bitmap_button_ext_desktop_my_machine_tk82c[];
extern char *bitmap_button_ext_desktop_my_machine_tk83[];
extern char *bitmap_button_ext_desktop_my_machine_tk85[];
extern char *bitmap_button_ext_desktop_my_machine_ace[];
extern char *bitmap_button_ext_desktop_my_machine_ql[];
extern char *bitmap_button_ext_desktop_my_machine_spectrum_128_spa[];
extern char *bitmap_button_ext_desktop_my_machine_spectrum_128_eng[];
extern char *bitmap_button_ext_desktop_my_machine_spectrum_p2[];
extern char *bitmap_button_ext_desktop_my_machine_spectrum_p2a[];
extern char *bitmap_button_ext_desktop_my_machine_spectrum_p3[];
extern char *bitmap_button_ext_desktop_my_machine_spectrum_48_spa[];
extern char *bitmap_button_ext_desktop_my_machine_inves[];
extern char *bitmap_button_ext_desktop_my_machine_cpc_464[];
extern char *bitmap_button_ext_desktop_my_machine_pcw_8256[];
extern char *bitmap_button_ext_desktop_my_machine_cpc_6128[];
extern char *bitmap_button_ext_desktop_my_machine_cpc_664[];
extern char *bitmap_button_ext_desktop_my_machine_coleco[];
extern char *bitmap_button_ext_desktop_my_machine_sms[];
extern char *bitmap_button_ext_desktop_my_machine_sg1000[];
extern char *bitmap_button_ext_desktop_my_machine_tk90x[];
extern char *bitmap_button_ext_desktop_my_machine_tk95[];
extern char *bitmap_button_ext_desktop_my_machine_msx[];
extern char *bitmap_button_ext_desktop_my_machine_svi318[];
extern char *bitmap_button_ext_desktop_my_machine_svi328[];
extern char *bitmap_button_ext_desktop_my_machine_timex_ts2068[];
extern char *bitmap_button_ext_desktop_my_machine_timex_tc2048[];
extern char *bitmap_button_ext_desktop_my_machine_z88[];
extern char *bitmap_button_ext_desktop_my_machine_zxuno[];
extern char *bitmap_button_ext_desktop_my_machine_tsconf[];
extern char *bitmap_button_ext_desktop_my_machine_baseconf[];
extern char *bitmap_button_ext_desktop_my_machine_mk14[];
extern char *bitmap_button_ext_desktop_my_machine_pentagon[];
extern char *bitmap_button_ext_desktop_my_machine_spectrum_next[];
extern char *bitmap_button_ext_desktop_my_machine_sam[];

#define EXT_DESKTOP_TOTAL_BUTTONS 14

#define EXT_DESKTOP_BUTTONS_ANCHO 26
#define EXT_DESKTOP_BUTTONS_ALTO 26

#define EXT_DESKTOP_BUTTONS_TOTAL_SIZE 32

#define EXT_DESKTOP_BUTTON_CLOSE_ALL_ID 12

#endif
