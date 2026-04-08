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

#ifndef SCREEN_FX_H
#define SCREEN_FX_H

#include "cpu.h"


enum SCREEN_REDUCTIONS {
//    SCREEN_REDUCE_NONE,
    SCREEN_REDUCE_075,
    SCREEN_REDUCE_050,
    SCREEN_REDUCE_025
};

extern enum SCREEN_REDUCTIONS screen_reduction_factor;

#define SCREEN_EFFECT_PIXELATE_MAX_SIZE 32

#define SCREEN_FX_WAVES_DEFAULT_INTENSITY 8
#define SCREEN_FX_SHEAR_DEFAULT_INTENSITY 4
#define SCREEN_FX_BLUR_DEFAULT_INTENSITY 1
#define SCREEN_FX_CONTRAST_DEFAULT_INTENSITY 100
#define SCREEN_FX_BRIGHTNESS_DEFAULT_INTENSITY 50
#define SCREEN_FX_ATTRACTION_DEFAULT_INTENSITY 4
#define SCREEN_FX_PIXELATE_DEFAULT_INTENSITY 2
#define SCREEN_FX_LENS_DEFAULT_INTENSITY 100
#define SCREEN_FX_ROTATE_DEFAULT_ANGLE 45
#define SCREEN_FX_SCROLL_HORIZONTAL_DEFAULT_OFFSET 1
#define SCREEN_FX_SCROLL_VERTICAL_DEFAULT_OFFSET 1

extern void screen_special_effects_free_buffers(void);
extern z80_int *screen_special_effects_alloc_buffer(int ancho,int alto);
extern z80_bit screen_special_effects_enabled;



extern int screen_rainbow_effect_fisheye_factor_k;
extern void screen_rainbow_effect_fisheye_change_factor(void);

extern int screen_rainbow_effect_pixelate_size;
extern z80_bit screen_rainbow_effect_pixelate_follow_mouse;

extern int screen_rainbow_effect_improved_waves_intensity;
extern z80_bit screen_rainbow_effect_improved_waves_follow_mouse;

extern z80_bit screen_special_effects_fisheye_automatic_factor;
extern z80_bit screen_special_effects_fisheye_follow_mouse;
extern char screen_special_effects_fisheye_follow_music_channel;
extern int screen_rainbow_effect_shear_intensity;
extern z80_bit screen_rainbow_effect_shear_intensity_follow_mouse;

extern int screen_rainbow_effect_attraction_intensity;
extern int screen_rainbow_effect_attraction_atrac_repulse;

extern z80_bit screen_rainbow_effect_sepia_follow_mouse;

extern int screen_rainbow_effect_scroll_horizontal_offset;
extern z80_bit screen_rainbow_effect_scroll_horizontal_follow_mouse;
extern int screen_rainbow_effect_scroll_vertical_offset;
extern z80_bit screen_rainbow_effect_scroll_vertical_follow_mouse;
extern z80_bit screen_rainbow_effect_scroll_horizontal_circular;
extern z80_bit screen_rainbow_effect_scroll_vertical_circular;

extern int screen_rainbow_effect_contrast_intensity;
extern z80_bit screen_rainbow_effect_contrast_follow_mouse;
extern int screen_rainbow_effect_brightness_intensity;
extern z80_bit screen_rainbow_effect_brightness_follow_mouse;
extern int screen_rainbow_effect_rotate_grados;
extern z80_bit screen_rainbow_effect_remolino_follow_mouse;
extern z80_bit screen_rainbow_effect_rotate_follow_mouse;
extern int screen_rainbow_effect_shaderborder_factor_zoom_leftright;
extern int screen_rainbow_effect_shaderborder_factor_zoom_updown;
extern int screen_rainbow_effect_shaderborder_blur_intensity_leftright;
extern int screen_rainbow_effect_shaderborder_blur_intensity_updown;
extern z80_bit screen_rainbow_effect_shaderborder_leftright_enable;
extern z80_bit screen_rainbow_effect_shaderborder_updown_enable;
extern int screen_rainbow_effect_blur_intensity;
extern z80_bit screen_rainbow_effect_blur_follow_mouse;
extern int screen_rainbow_effect_persistence_total_frames;

extern void init_screen_effects_table(void);

#define SCREEN_RAINBOW_EFFECT_PERSISTENCE_MAX_FRAMES 25

//Total de efectos que se pueden aplicar
#define MAX_SCREEN_LIST_EFFECTS 50

//Cantidad de efectos diferentes
#define MAX_SCREEN_EFFECTS 37

enum enum_screen_effect_types {
    SCREEN_EFFECT_TYPE_NONE, //Este siempre el primero en este enum
    SCREEN_EFFECT_TYPE_REDUCE,
    SCREEN_EFFECT_TYPE_FLIP_HORIZONTAL,
    SCREEN_EFFECT_TYPE_FLIP_VERTICAL,
    SCREEN_EFFECT_TYPE_ROTATE,
    SCREEN_EFFECT_TYPE_TWIRL,
    SCREEN_EFFECT_TYPE_SCROLL_HORIZONTAL,
    SCREEN_EFFECT_TYPE_SCROLL_VERTICAL,
    SCREEN_EFFECT_TYPE_HSYNC_LOST,
    SCREEN_EFFECT_TYPE_VSYNC_LOST,
    SCREEN_EFFECT_TYPE_UNSTEADY,
    SCREEN_EFFECT_TYPE_INTERFERENCES,
    SCREEN_EFFECT_TYPE_WAVES,
    SCREEN_EFFECT_TYPE_SEA,
    SCREEN_EFFECT_TYPE_MAGNETIC_FIELD,
    SCREEN_EFFECT_TYPE_SHEAR,
    SCREEN_EFFECT_TYPE_LENS,
    SCREEN_EFFECT_TYPE_RADAR,
    SCREEN_EFFECT_TYPE_ZOOM_MOUSE,
    SCREEN_EFFECT_TYPE_PIXELATE,
    SCREEN_EFFECT_TYPE_BLUR,
    SCREEN_EFFECT_TYPE_SHADERBORDER,
    SCREEN_EFFECT_TYPE_LED,
    SCREEN_EFFECT_TYPE_FADEIN,
    SCREEN_EFFECT_TYPE_FADEOUT,
    SCREEN_EFFECT_TYPE_FADEINOUT,
    SCREEN_EFFECT_TYPE_SCANLINES,
    SCREEN_EFFECT_TYPE_SEPIA,
    SCREEN_EFFECT_TYPE_RGB,
    SCREEN_EFFECT_TYPE_CONTRAST,
    SCREEN_EFFECT_TYPE_BRIGHTNESS,
    SCREEN_EFFECT_TYPE_PERSISTENCE,
    SCREEN_EFFECT_TYPE_NAGRAVISION,
    SCREEN_EFFECT_TYPE_RANDOMLINES,
    SCREEN_EFFECT_TYPE_DECODENAGRAVISION,
    SCREEN_EFFECT_TYPE_SORTALIKE,
    SCREEN_EFFECT_TYPE_LOGOREBOUND
};

typedef struct {
    enum enum_screen_effect_types type;
    char name[40];
    z80_bit *follow_mouse_setting;
    int *intensity_setting;
    int default_intensity;
    int lower_intensity,higher_intensity;
} screen_effect_type_name;

extern char *screen_effect_get_name(enum enum_screen_effect_types type);
extern int screen_effect_get_type(char *efecto);
extern void set_screen_effect(int position,enum enum_screen_effect_types type,int enabled);
extern int set_screen_follow_mouse_effect(enum enum_screen_effect_types type);
extern int set_screen_intensity_effect(enum enum_screen_effect_types type,int intensidad);
extern void screen_effect_print_names(void);

typedef struct {
    int enabled;
    enum enum_screen_effect_types type;
} screen_effect_applied;

extern screen_effect_applied screen_effect_applied_list[];

extern screen_effect_type_name screen_effect_type_list[];






#endif
