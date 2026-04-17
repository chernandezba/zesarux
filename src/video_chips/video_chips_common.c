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



#include <stdio.h>


#include "video_chips_common.h"
#include "cpu.h"
#include "debug.h"
#include "screen.h"
#include "settings.h"



int menu_debug_draw_sprites_used_sprites_in_frame[MENU_VIEW_SPRITES_MAX_USED_SPRITES_IN_FRAME];

void init_view_sprites_used_sprites_in_frame(void)
{
    int i;

    for (i=0;i<MENU_VIEW_SPRITES_MAX_USED_SPRITES_IN_FRAME;i++) {
        menu_debug_draw_sprites_used_sprites_in_frame[i]=0;
    }

}

int menu_debug_draw_sprites_is_sprite_used_in_frame(int sprite)
{
    if (sprite<0 || sprite>=MENU_VIEW_SPRITES_MAX_USED_SPRITES_IN_FRAME) return 0;

    //if (menu_debug_draw_sprites_used_sprites_in_frame[sprite]) printf("is %d %d\n",sprite,menu_debug_draw_sprites_used_sprites_in_frame[sprite]);

    return menu_debug_draw_sprites_used_sprites_in_frame[sprite];

}

void menu_debug_draw_sprites_set_sprite_used_in_frame(int sprite)
{
    if (sprite<0 || sprite>=MENU_VIEW_SPRITES_MAX_USED_SPRITES_IN_FRAME) return;
    //printf("set sprite %d\n",sprite);
    menu_debug_draw_sprites_used_sprites_in_frame[sprite]=1;

}