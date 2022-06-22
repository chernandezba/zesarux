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

#ifndef MENU_ITEMS_SETTINGS_H
#define MENU_ITEMS_SETTINGS_H

#include "cpu.h"

extern void menu_settings(MENU_ITEM_PARAMETERS);
extern void menu_settings_config_file(MENU_ITEM_PARAMETERS);
extern int menu_interface_border_cond(void);
extern void menu_interface_border(MENU_ITEM_PARAMETERS);
extern void menu_interface_fullscreen(MENU_ITEM_PARAMETERS);
extern void menu_interface_flash(MENU_ITEM_PARAMETERS);
extern void menu_interface_multitask(MENU_ITEM_PARAMETERS);
extern void menu_hardware_realjoystick(MENU_ITEM_PARAMETERS);
extern void menu_debug_verbose(MENU_ITEM_PARAMETERS);
extern void menu_settings_tape(MENU_ITEM_PARAMETERS);
extern void menu_interface_change_gui_style_apply(MENU_ITEM_PARAMETERS);
extern void menu_settings_snapshot(MENU_ITEM_PARAMETERS);
extern void menu_cpu_settings(MENU_ITEM_PARAMETERS);

extern void menu_zxdesktop_set_configurable_icons_modify(MENU_ITEM_PARAMETERS);
extern void menu_zxdesktop_add_configurable_icons(MENU_ITEM_PARAMETERS);
extern void menu_zxdesktop_set_configurable_icons(MENU_ITEM_PARAMETERS);

extern int menu_hardware_advanced_input_value(int minimum,int maximum,char *texto,int *variable);

extern int menu_inves_cond(void);
extern int menu_cond_stdout(void);
extern int menu_cond_zx8081(void);
extern void menu_interface_charwidth_after_width_change(void);
extern void menu_interface_allow_background_windows_delete_windows(void);

extern void menu_zxdesktop_set_userdef_buttons_functions(MENU_ITEM_PARAMETERS);
extern void menu_hardware_set_f_functions(MENU_ITEM_PARAMETERS);


#endif

