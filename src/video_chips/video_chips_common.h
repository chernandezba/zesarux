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

#ifndef VIDEO_CHIPS_COMMON_H
#define VIDEO_CHIPS_COMMON_H


#define MENU_VIEW_SPRITES_MAX_USED_SPRITES_IN_FRAME 1000

extern void video_chips_common_init_used_sprites_in_frame(void);
extern void menu_debug_draw_sprites_set_sprite_used_in_frame(int sprite);
extern int menu_debug_draw_sprites_is_sprite_used_in_frame(int sprite);

#endif
