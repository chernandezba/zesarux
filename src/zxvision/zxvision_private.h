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

#ifndef ZXVISION_PRIVATE_H
#define ZXVISION_PRIVATE_H



#include "cpu.h"

//Para usar PATH_MAX
#include "zesarux.h"


extern int zxvision_si_icono_cerca(int x,int y);
extern int zxvision_get_minimum_y_icon_position(void);
extern void zxvision_get_start_valid_positions_icons(int *p_xinicial,int *p_xfinal,int *p_yinicial,int *p_yfinal);
extern int zxvision_search_trash_configurable_icon(void);
extern int if_zxdesktop_trash_not_empty(void);
extern int zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_width(void);
extern int zxvision_if_mouse_in_lower_button_reduce_zxdesktop_width(void);
extern int zxvision_if_mouse_in_lower_button_enlarge_zxdesktop_height(void);
extern int zxvision_if_mouse_in_lower_button_reduce_zxdesktop_height(void);
extern int zxvision_if_lower_button_switch_zxdesktop_visible(void);
extern int zxvision_if_lower_button_switch_zxdesktop_enabled(void);
extern void zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_common(int *p_x,int *p_y,int *p_xboton,int *p_yboton);
extern int zxvision_if_mouse_in_lower_button_enlarge_reduce_zxdesktop_width(int ampliar_reducir_ancho);

extern z80_bit switchzxdesktop_button_visible;

#endif
