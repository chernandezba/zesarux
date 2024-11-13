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

#ifndef MENU_ITEMS_STORAGE_H
#define MENU_ITEMS_STORAGE_H

#include "cpu.h"

extern char menu_hilow_convert_audio_last_audio_sample;
extern int menu_hilow_convert_lento;
extern int menu_hilow_convert_paused;
extern int menu_hilow_convert_audio_has_been_opened;
extern int hilow_convert_audio_thread_running;
//extern char enviado_hilow_sample_convirtiendo;
extern void menu_hilow_convert_get_audio_buffer(void);

extern int menu_storage_string_root_dir(char *string_root_dir);
extern void menu_visual_floppy_buffer_add(int pista,int sector,int byte_en_sector);
extern void menu_visual_floppy_buffer_add_persistent(int pista,int sector,int byte_en_sector);
extern void menu_visualfloppy_increment_rotation(void);
extern void menu_visual_floppy_buffer_reset(void);

extern void menu_kartusho(MENU_ITEM_PARAMETERS);
extern void menu_superupgrade(MENU_ITEM_PARAMETERS);
extern void menu_ifrom(MENU_ITEM_PARAMETERS);
extern void menu_storage_hilow_insert(MENU_ITEM_PARAMETERS);
extern void menu_hilow(MENU_ITEM_PARAMETERS);
extern void menu_samram(MENU_ITEM_PARAMETERS);
extern void menu_timexcart(MENU_ITEM_PARAMETERS);
extern void menu_realtape_open(MENU_ITEM_PARAMETERS);
extern int menu_realtape_inserted_cond(void);
extern void menu_timexcart(MENU_ITEM_PARAMETERS);
extern void menu_dandanator(MENU_ITEM_PARAMETERS);
extern void menu_hilow_barbanegra(MENU_ITEM_PARAMETERS);
extern void menu_hilow_visual_datadrive(MENU_ITEM_PARAMETERS);
extern void menu_hilow_convert_audio(MENU_ITEM_PARAMETERS);
extern void menu_interface1(MENU_ITEM_PARAMETERS);
extern void menu_visual_microdrive(MENU_ITEM_PARAMETERS);
extern void menu_microdrive_raw_map(MENU_ITEM_PARAMETERS);
extern void menu_betadisk(MENU_ITEM_PARAMETERS);
extern void menu_zxpand(MENU_ITEM_PARAMETERS);
extern void menu_esxdos_traps(MENU_ITEM_PARAMETERS);
extern void menu_visual_floppy(MENU_ITEM_PARAMETERS);




#endif

