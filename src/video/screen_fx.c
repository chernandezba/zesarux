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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "screen_fx.h"
#include "debug.h"
#include "screen.h"
#include "cpu.h"
#include "utils_math.h"
#include "joystick.h"
#include "sensors.h"
#include "menu_bitmaps.h"

z80_int *screen_special_effects_functions(z80_int *origen,int ancho,int alto);


int screen_rainbow_effect_rotate_grados=SCREEN_FX_ROTATE_DEFAULT_ANGLE;
z80_bit screen_rainbow_effect_rotate_follow_mouse={0};
z80_bit screen_rainbow_effect_remolino_follow_mouse={0};

int screen_rainbow_effect_pixelate_size=SCREEN_FX_PIXELATE_DEFAULT_INTENSITY;
z80_bit screen_rainbow_effect_pixelate_follow_mouse={0};


int screen_rainbow_effect_improved_waves_intensity=SCREEN_FX_WAVES_DEFAULT_INTENSITY;
z80_bit screen_rainbow_effect_improved_waves_follow_mouse={0};

int screen_rainbow_effect_shear_intensity=SCREEN_FX_SHEAR_DEFAULT_INTENSITY;
z80_bit screen_rainbow_effect_shear_intensity_follow_mouse={0};

z80_bit screen_rainbow_effect_sepia_follow_mouse={0};
int screen_rainbow_effect_blur_intensity=SCREEN_FX_BLUR_DEFAULT_INTENSITY;
z80_bit screen_rainbow_effect_blur_follow_mouse={0};

int screen_rainbow_effect_contrast_intensity=SCREEN_FX_CONTRAST_DEFAULT_INTENSITY;
z80_bit screen_rainbow_effect_contrast_follow_mouse={0};

int screen_rainbow_effect_brightness_intensity=SCREEN_FX_BRIGHTNESS_DEFAULT_INTENSITY;
z80_bit screen_rainbow_effect_brightness_follow_mouse={0};

int screen_rainbow_effect_scroll_horizontal_offset=SCREEN_FX_SCROLL_HORIZONTAL_DEFAULT_OFFSET;
z80_bit screen_rainbow_effect_scroll_horizontal_circular={0};
z80_bit screen_rainbow_effect_scroll_horizontal_follow_mouse={0};

int screen_rainbow_effect_scroll_vertical_offset=SCREEN_FX_SCROLL_VERTICAL_DEFAULT_OFFSET;
z80_bit screen_rainbow_effect_scroll_vertical_circular={0};
z80_bit screen_rainbow_effect_scroll_vertical_follow_mouse={0};

int screen_rainbow_effect_attraction_intensity=SCREEN_FX_ATTRACTION_DEFAULT_INTENSITY;
int screen_rainbow_effect_attraction_atrac_repulse=+1;



z80_bit screen_rainbow_effect_shaderborder_leftright_enable={1};
z80_bit screen_rainbow_effect_shaderborder_updown_enable={1};
int screen_rainbow_effect_shaderborder_factor_zoom_leftright=2000;
int screen_rainbow_effect_shaderborder_factor_zoom_updown=2000;
int screen_rainbow_effect_shaderborder_blur_intensity_leftright=4;
int screen_rainbow_effect_shaderborder_blur_intensity_updown=4;



//0.2 segundos. O sea, 10 frames
int screen_rainbow_effect_persistence_total_frames=10;

//A,B,C canales AY o 0
char screen_special_effects_fisheye_follow_music_channel=0;


//Fisheye
// intensidad del efecto
//k → intensidad del efecto
//Es un número pequeño (flotante), por ejemplo:
// k = 0.00001f;
//controla lo fuerte que es el ojo de pez:
//k > 0 → efecto tipo ojo de pez (convexo)
//k < 0 → efecto tipo lente inversa (cóncavo)
//Lo tenemos multiplicado por 100 para evitar usar flotantes
int screen_rainbow_effect_fisheye_factor_k = SCREEN_FX_LENS_DEFAULT_INTENSITY;
z80_bit screen_special_effects_fisheye_automatic_factor={0};
z80_bit screen_special_effects_fisheye_follow_mouse={0};



z80_bit screen_special_effects_enabled={0};



screen_effect_type_name screen_effect_type_list[MAX_SCREEN_EFFECTS]={
    {SCREEN_EFFECT_TYPE_NONE,"None",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_REDUCE,"Reduce",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_UNSTEADY,"Unsteady",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_FLIP_VERTICAL,"Flip Vertical",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_FLIP_HORIZONTAL,"Flip Horizontal",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_ROTATE,"Rotate",&screen_rainbow_effect_rotate_follow_mouse,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_TWIRL,"Twirl",&screen_rainbow_effect_remolino_follow_mouse,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_INTERFERENCES,"Interferences",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_SEA,"Sea",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_WAVES,"Waves",&screen_rainbow_effect_improved_waves_follow_mouse,&screen_rainbow_effect_improved_waves_intensity,SCREEN_FX_WAVES_DEFAULT_INTENSITY,2,20},
    {SCREEN_EFFECT_TYPE_MAGNETIC_FIELD,"Magnetic Field",NULL,&screen_rainbow_effect_attraction_intensity,SCREEN_FX_ATTRACTION_DEFAULT_INTENSITY,1,20},
    {SCREEN_EFFECT_TYPE_SHEAR,"Shear",&screen_rainbow_effect_shear_intensity_follow_mouse,&screen_rainbow_effect_shear_intensity,SCREEN_FX_SHEAR_DEFAULT_INTENSITY,1,49},
    {SCREEN_EFFECT_TYPE_LENS,"Lens",&screen_special_effects_fisheye_follow_mouse,&screen_rainbow_effect_fisheye_factor_k,SCREEN_FX_LENS_DEFAULT_INTENSITY,-600,600},
    {SCREEN_EFFECT_TYPE_RADAR,"Radar",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_ZOOM_MOUSE,"Zoom Mouse",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_PIXELATE,"Pixelate",&screen_rainbow_effect_pixelate_follow_mouse,&screen_rainbow_effect_pixelate_size,SCREEN_FX_PIXELATE_DEFAULT_INTENSITY,2,SCREEN_EFFECT_PIXELATE_MAX_SIZE},
    {SCREEN_EFFECT_TYPE_BLUR,"Blur",&screen_rainbow_effect_blur_follow_mouse,&screen_rainbow_effect_blur_intensity,SCREEN_FX_BLUR_DEFAULT_INTENSITY,1,16},
    {SCREEN_EFFECT_TYPE_SHADERBORDER,"Shader Border",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_LED,"LED",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_HSYNC_LOST,"Hsync lost",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_VSYNC_LOST,"Vsync lost",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_SCROLL_HORIZONTAL,"Scroll Horizontal",&screen_rainbow_effect_scroll_horizontal_follow_mouse,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_SCROLL_VERTICAL,"Scroll Vertical",&screen_rainbow_effect_scroll_vertical_follow_mouse,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_FADEIN,"Fade In",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_FADEOUT,"Fade Out",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_FADEINOUT,"Fade InOut",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_SCANLINES,"Scanlines",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_SEPIA,"Sepia",&screen_rainbow_effect_sepia_follow_mouse,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_PERSISTENCE,"Persistence",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_CONTRAST,"Contrast",&screen_rainbow_effect_contrast_follow_mouse,&screen_rainbow_effect_contrast_intensity,SCREEN_FX_CONTRAST_DEFAULT_INTENSITY,0,1000},
    {SCREEN_EFFECT_TYPE_BRIGHTNESS,"Brightness",&screen_rainbow_effect_brightness_follow_mouse,&screen_rainbow_effect_brightness_intensity,SCREEN_FX_BRIGHTNESS_DEFAULT_INTENSITY,0,256*100},
    {SCREEN_EFFECT_TYPE_NAGRAVISION,"Nagravision",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_RANDOMLINES,"Random Lines",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_DECODENAGRAVISION,"Decode Nagravision",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_SORTALIKE,"Sortalike",NULL,NULL,0,0,0},
    {SCREEN_EFFECT_TYPE_LOGOREBOUND,"Logo Rebound",NULL,NULL,0,0,0}
};

char *screen_effect_name_unknown="Unknown";


z80_int *new_scalled_rainbow_buffer=NULL;


//Comunes a escalado normal y escalado con gigascreen
int scalled_rainbow_ancho=0;
int scalled_rainbow_alto=0;

//Punteros de escalado 0.75 para gigascreen
z80_int *new_scalled_rainbow_buffer_gigascren_one=NULL;
z80_int *new_scalled_rainbow_buffer_gigascren_two=NULL;

void screen_special_effects_free_buffers(void)
{
    if (new_scalled_rainbow_buffer!=NULL) {
        debug_printf(VERBOSE_DEBUG,"Freeing previous scaled rainbow buffer");
        free (new_scalled_rainbow_buffer);
        new_scalled_rainbow_buffer=NULL;
    }

    if (new_scalled_rainbow_buffer_gigascren_one!=NULL) {
            debug_printf(VERBOSE_DEBUG,"Freeing previous scaled gigascreen rainbow buffers");
            free (new_scalled_rainbow_buffer_gigascren_one);
            free (new_scalled_rainbow_buffer_gigascren_two);
            new_scalled_rainbow_buffer_gigascren_one=NULL;
            new_scalled_rainbow_buffer_gigascren_two=NULL;
    }
}

z80_int *screen_special_effects_alloc_buffer(int ancho,int alto)
{
    int tamanyo=ancho*alto*2;
    z80_int *buffer=util_malloc(ancho*alto*2,"Can not allocate scalled rainbow buffer"); //*2 por que son valores de 16 bits
    memset(buffer,0,tamanyo);
    return buffer;
}

void screen_special_effects_functions_pre(int ancho,int alto)
{

    //Liberar buffer anterior
    screen_special_effects_free_buffers();


    new_scalled_rainbow_buffer=screen_special_effects_functions(rainbow_buffer,ancho,alto);

}



char *screen_effect_get_name(enum enum_screen_effect_types type)
{
    int i;
    for (i=0;i<MAX_SCREEN_EFFECTS;i++) {
        if (screen_effect_type_list[i].type==type) return screen_effect_type_list[i].name;
    }
    return screen_effect_name_unknown;
}

void screen_effect_print_names(void)
{
    int i;
    for (i=0;i<MAX_SCREEN_EFFECTS;i++) {
        printf("%s",screen_effect_type_list[i].name);
        if (i!=MAX_SCREEN_EFFECTS-1) printf(",");
    }

}

//Retorna id de efecto segun el texto buscado. -1 si no existe
//Nota: dado que los enum son unsigned, uso variable int en vez de enum_screen_effect_types
int screen_effect_get_type(char *efecto)
{
    int i;
    for (i=0;i<MAX_SCREEN_EFFECTS;i++) {
        if (!strcasecmp(screen_effect_type_list[i].name,efecto)) return screen_effect_type_list[i].type;
    }
    return -1;
}

screen_effect_applied screen_effect_applied_list[MAX_SCREEN_LIST_EFFECTS];

void set_screen_effect(int position,enum enum_screen_effect_types type,int enabled)
{
    if (position<0 || position>=MAX_SCREEN_LIST_EFFECTS) {
        debug_printf(VERBOSE_ERR,"Invalid position %d for effect",position);
        return;
    }

    screen_effect_applied_list[position].type=type;
    screen_effect_applied_list[position].enabled=(enabled ? 1 : 0);
}

//Retorna <0 si efecto no tiene follow mouse
int set_screen_follow_mouse_effect(enum enum_screen_effect_types type)
{
    int i;
    for (i=0;i<MAX_SCREEN_EFFECTS;i++) {
        if (screen_effect_type_list[i].type==type && screen_effect_type_list[i].follow_mouse_setting!=NULL) {
            z80_bit *follow_mouse=screen_effect_type_list[i].follow_mouse_setting;
            follow_mouse->v=1;
            return 0;
        }
    }
    return -1;
}

//Retorna 0 si ok
//Retorna <0 si efecto no tiene intensidad
//Retorna >1 si fuera de rango
int set_screen_intensity_effect(enum enum_screen_effect_types type,int intensidad)
{
    int i;
    for (i=0;i<MAX_SCREEN_EFFECTS;i++) {
        if (screen_effect_type_list[i].type==type && screen_effect_type_list[i].intensity_setting!=NULL) {
            int *intensity_setting=screen_effect_type_list[i].intensity_setting;
            if (intensidad<screen_effect_type_list[i].lower_intensity || intensidad>screen_effect_type_list[i].higher_intensity) return 1;
            *intensity_setting=intensidad;
            return 0;
        }
    }
    return -1;
}

void init_screen_effects_table(void)
{
    int i;
    for (i=0;i<MAX_SCREEN_LIST_EFFECTS;i++) {
        screen_effect_applied_list[i].enabled=0;
        //if (i<MAX_SCREEN_EFFECTS) {
        //    screen_effect_applied_list[i].type=SCREEN_EFFECT_TYPE_NONE+i;
        //}
        //else {
            screen_effect_applied_list[i].type=SCREEN_EFFECT_TYPE_NONE;
        //}
    }
}

//Aplicar efectos a modo rainbow
z80_int *screen_rainbow_effects(z80_int *puntero,int ancho,int alto)
{
    puntero=rainbow_buffer;

    //Si se aplican efectos a la pantalla
    if (screen_special_effects_enabled.v) {
        screen_special_effects_functions_pre(ancho,alto);
        puntero=new_scalled_rainbow_buffer;
    }

    return puntero;
}







//Mezclar dos colores si estan en rango spectrum 0-15, retornando el gigascreen. Si no, devolver el primero
z80_int screen_scale_075_050_mix_two(z80_int color1, z80_int color2)
{
    if (color1<16 && color2<16 && screen_reduce_antialias.v && gigascreen_enabled.v==0) return get_gigascreen_color(color1,color2);
    else return color1;
}

void screen_scale_rainbow_43(z80_int *orig,int ancho,int alto,z80_int *dest)
{

    int x,y;

    int ancho_destino=(ancho*3)/4;

    int diferencia_ancho=ancho-ancho_destino;



    z80_int color_izq;
    z80_int color_der;
    z80_int color_arr;
    z80_int color_aba;
/*

La reducción funciona de la siguiente manera, se divide la imagen origen en bloques de 4x4 pixeles, y cada una de las de destino será de 3x3
Se parte de la imagen origen:

abcd
efgh
ijkl
mnop

A la de destino:

ab3
ef6
789

De la imagen origen, el primer bloque de 2x2 se traspasa tal cual, así:

ab
ef

Se traspasa a:

ab
ef

Luego, las primeras dos filas, se escalan asi:
Las ultimas dos columnas se mezclan los colores, mediante la funcion de mezclado: si hay antialias, se saca el color medio de los dos pixeles, el de la izquierda y derecha.
Si no hay antialas, se escoge el primer pixel.
Así:

cd   -> 3
gh   -> 6


Luego las ultimas dos filas, se escalan así:

Las primeras 3 columnas se mezclan los colores, de manera similar a la anterior, pero mezclando el pixel de arriba y abajo.
Así:

i
   ->  7
m


j
   -> 8
n


k
   -> 9
o

De esto se ve que siempre se descarta dos pixeles como minimo, el l y p

*/

    for (y=0;y<alto;y++) {


        for (x=0;x<ancho;x+=4) {

            //Las dos primeras lineas, las dos primeras columnas, color tal cual. La tercera columna, se mezclan
            if ( (y%4)<2) {
            *dest=*orig;
            dest++;
            orig++;

            *dest=*orig;
            dest++;
            orig++;

            //Mezclar los ultimos dos
            color_izq=*orig;
            orig++;
            color_der=*orig;
            orig++;

            *dest=screen_scale_075_050_mix_two(color_izq,color_der);
            dest++;
            }

            //Las ultimas dos lineas, mezclamos arriba y abajo en las tres primeras columnas. La cuarta columna se descarta
            if ( (y%4)==2) {
            color_arr=*orig;
            color_aba=orig[ancho];

            *dest=screen_scale_075_050_mix_two(color_arr,color_aba);
            dest++;
            orig++;


            color_arr=*orig;
            color_aba=orig[ancho];

            *dest=screen_scale_075_050_mix_two(color_arr,color_aba);
            dest++;
            orig++;


            //Mezclar los ultimos dos
            /*color_izq=*orig;
            orig++;
            color_der=*orig;
            orig++;

            *dest=screen_scale_075_050_mix_two(color_izq,color_der);
            dest++;*/

            color_arr=*orig;
            color_aba=orig[ancho];

            *dest=screen_scale_075_050_mix_two(color_arr,color_aba);
            dest++;
            orig++;

            orig++;


            }



        }


        if ( (y%4)==2) {
            //Saltar la cuarta linea
            orig+=ancho;
            y++;
        }

        dest+=diferencia_ancho;
    }

}

//Escalado a la mitad (2:1)
void screen_scale_rainbow_21(z80_int *orig,int ancho,int alto,z80_int *dest)
{

    int x,y;

    int ancho_destino=ancho/2;


    int diferencia_ancho=ancho-ancho_destino;




    z80_int color_1;
    z80_int color_2;



    for (y=0;y<alto;y+=2) {

        for (x=0;x<ancho;x+=2) {

            //Hacemos gigascreen de pixel arriba-izq y de abajo-der
            color_1=*orig;
            orig++;

            color_2=orig[ancho];

            orig++;


            *dest=screen_scale_075_050_mix_two(color_1,color_2);
            //*dest=color_1;
            dest++;


        }

        dest+=diferencia_ancho;


        //Saltar la segunda linea
        orig+=ancho;



    }

}


//Escalado a una cuarta parte (4:1)
void screen_scale_rainbow_41(z80_int *orig,int ancho,int alto,z80_int *dest)
{
    z80_int *buffer_intermedio=util_malloc_fill(sizeof(z80_int)*ancho*alto,"Can not allocate memory for image reduction",0);


    //Cutre pero efectivo. Reducimos dos veces
    screen_scale_rainbow_21(orig,ancho,alto,buffer_intermedio);
    screen_scale_rainbow_21(buffer_intermedio,ancho,alto,dest);

    free(buffer_intermedio);

}



enum SCREEN_REDUCTIONS screen_reduction_factor=SCREEN_REDUCE_075;

//Antialias al reducir
z80_bit screen_reduce_antialias={1};



int screen_rainbow_effect_temblar_frame=0;

void screen_rainbow_effect_temblar(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    int x,y;

    for (y=0;y<alto;y++) {

        for (x=ancho-2;x>=0;x--) {
            int offset=(ancho*y)+x;
            if ((y%2)==screen_rainbow_effect_temblar_frame) {
                destino[offset]=origen[offset+1];
            }
            else {
                destino[offset]=origen[offset];
            }
        }

    }

    screen_rainbow_effect_temblar_frame ^=1;

}

void screen_rainbow_effect_flip_vertical(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    int x,y;

    for (y=0;y<alto/2;y++) {

        for (x=0;x<ancho;x++) {
            //Obtener pixel de arriba y abajo y meterlos en destino intercambiados
            int offset1=(ancho*y)+x;
            int offset2=(ancho*(alto-1-y))+x;

            int color1=origen[offset1];
            int color2=origen[offset2];
            destino[offset1]=color2;
            destino[offset2]=color1;
        }

    }

}

void screen_rainbow_effect_flip_horizontal(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    int x,y;

    for (y=0;y<alto;y++) {

        for (x=0;x<ancho/2;x++) {
            //Obtener pixel de arriba y abajo y meterlos en destino intercambiados
            int offset1=(ancho*y)+x;
            int offset2=(ancho*y)+ancho-1-x;

            int color1=origen[offset1];
            int color2=origen[offset2];
            destino[offset1]=color2;
            destino[offset2]=color1;
        }

    }

}



void screen_rainbow_effect_rotate(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    int x,y;
    int centro_x=ancho/2;
    int centro_y=alto/2;

    if (screen_rainbow_effect_rotate_follow_mouse.v) {
        centro_x=mouse_x/zoom_x;
        centro_y=mouse_y/zoom_y;
    }

    for (y=0;y<alto;y++) {

        for (x=0;x<ancho;x++) {

            // trasladar al centro
            int dx = x - centro_x;
            int dy = y - centro_y;

            // aplicar rotación
            int src_x =  (util_get_cosine(screen_rainbow_effect_rotate_grados) * dx)/10000 - (util_get_sine(screen_rainbow_effect_rotate_grados) * dy)/10000 + centro_x;
            int src_y = (util_get_sine(screen_rainbow_effect_rotate_grados) * dx)/10000 + (util_get_cosine(screen_rainbow_effect_rotate_grados) * dy)/10000 + centro_y;

            if (src_x>=0 && src_y>=0 && src_x<ancho && src_y<alto) {
                int offset_origen=(ancho*src_y)+src_x;

                int color=origen[offset_origen];

                int offset_destino=(ancho*y)+x;

                destino[offset_destino]=color;
            }
        }

    }

}




void screen_rainbow_effect_remolino(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    int x,y;
    int centro_x=ancho/2;
    int centro_y=alto/2;

    if (screen_rainbow_effect_remolino_follow_mouse.v) {
        centro_x=mouse_x/zoom_x;
        centro_y=mouse_y/zoom_y;
    }

    for (y=0;y<alto;y++) {

        for (x=0;x<ancho;x++) {

            // trasladar al centro
            int dx = x - centro_x;
            int dy = y - centro_y;

            //grados segun distancia al centro. mayor distancia, menos grados
            //nota: no hace falta sacar la raiz cuadrada, solo quiero algo proporcional a la distancia
            int distancia=dx*dx+dy*dy;

            int grados;

            if (distancia==0) grados=0;
            else grados=100000/distancia;

            int src_x =  (util_get_cosine(grados) * dx)/10000 - (util_get_sine(grados) * dy)/10000 + centro_x;
            int src_y = (util_get_sine(grados) * dx)/10000 + (util_get_cosine(grados) * dy)/10000 + centro_y;

            if (src_x>=0 && src_y>=0 && src_x<ancho && src_y<alto) {
                int offset_origen=(ancho*src_y)+src_x;

                int color=origen[offset_origen];

                int offset_destino=(ancho*y)+x;

                destino[offset_destino]=color;
            }
        }

    }

}


int effect_nagravision_seed=0;

int effect_nagravision_get_rnd(void)
{
/*
;Seguimos la misma formula RND del spectrum:
;0..1 -> n=(75*(n+1)-1)/65536
;0..65535 -> n=65536/(75*(n+1)-1)
generar_random_noise:
*/

        int resultado;
        int r;

        r=effect_nagravision_seed;

        resultado=(75*(r+1)-1);

        effect_nagravision_seed=resultado & 0xFFFF;

        return effect_nagravision_seed;


}

//Intercambiar dos lineas
void screen_rainbow_effect_nagravision_swap(z80_int *destino,int ancho,int alto,int y1,int y2)
{
    int x;

    if (y1<0 || y2<0 || y1>=alto || y2>=alto) return;

    for (x=0;x<ancho;x++) {
        int y1_offset=y1*ancho+x;
        int y2_offset=y2*ancho+x;

        int color1=destino[y1_offset];
        int color2=destino[y2_offset];
        destino[y1_offset]=color2;
        destino[y2_offset]=color1;
    }
}


#define SCREEN_EFFECT_NAGRAVISION_GROUP_LINES 32
int screen_rainbow_effect_nagravision_initial_seed=0;

void screen_rainbow_effect_nagravision(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    //Se inicializa el seed en cada frame
    effect_nagravision_seed=screen_rainbow_effect_nagravision_initial_seed;

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    //Y mezclar lineas

    int y;



    for (y=0;y<alto;y++) {

        int valor_random=effect_nagravision_get_rnd() % SCREEN_EFFECT_NAGRAVISION_GROUP_LINES;

        valor_random -=(SCREEN_EFFECT_NAGRAVISION_GROUP_LINES/2);

        //desde -16 hasta +15 con SCREEN_EFFECT_NAGRAVISION_GROUP_LINES=32
        //if (valor_random>=15) printf("%d\n",valor_random);

        int y2=y+valor_random;

        screen_rainbow_effect_nagravision_swap(destino,ancho,alto,y,y2);
    }



    screen_rainbow_effect_nagravision_initial_seed++;
    //A cada segundo, se usa mismo seed
    if (screen_rainbow_effect_nagravision_initial_seed==50) screen_rainbow_effect_nagravision_initial_seed=0;

}

//Algoritmo malo que divide exactamente los bloques en 32
void screen_rainbow_effect_nagravision_bad(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    //Se inicializa el seed en cada frame
    effect_nagravision_seed=screen_rainbow_effect_nagravision_initial_seed;

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    //Y mezclar lineas

    int y;
    int ygrupo;

    for (ygrupo=0;ygrupo<alto;ygrupo+=SCREEN_EFFECT_NAGRAVISION_GROUP_LINES) {

        for (y=ygrupo;y<alto && y<ygrupo+SCREEN_EFFECT_NAGRAVISION_GROUP_LINES;y++) {

            int rango_random=SCREEN_EFFECT_NAGRAVISION_GROUP_LINES;
            if (ygrupo+rango_random>alto) rango_random=alto-ygrupo;

            int valor_random=effect_nagravision_get_rnd() % rango_random;

            int y2=ygrupo+valor_random;

            screen_rainbow_effect_nagravision_swap(destino,ancho,alto,y,y2);
        }

    }

    screen_rainbow_effect_nagravision_initial_seed++;
    //A cada segundo, se usa mismo seed
    if (screen_rainbow_effect_nagravision_initial_seed==50) screen_rainbow_effect_nagravision_initial_seed=0;

}


void screen_rainbow_effect_randomlines(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    //Se inicializa el seed en cada frame
    effect_nagravision_seed=screen_rainbow_effect_nagravision_initial_seed;

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    //Y mezclar lineas

    int y;

    for (y=0;y<alto;y++) {

        int y2=effect_nagravision_get_rnd() % alto;

        screen_rainbow_effect_nagravision_swap(destino,ancho,alto,y,y2);

    }

    screen_rainbow_effect_nagravision_initial_seed++;
    //A cada segundo, se usa mismo seed
    if (screen_rainbow_effect_nagravision_initial_seed==50) screen_rainbow_effect_nagravision_initial_seed=0;

}

int screen_rainbow_effect_sortalike_compare(z80_int *origen,int ancho,int y1,int y2)
{
    int x;
    int similares=0;

    for (x=0;x<ancho;x++) {
        int y1_offset=y1*ancho+x;
        int y2_offset=y2*ancho+x;

        int color_1_1=origen[y1_offset];

        int color_2_1=origen[y2_offset];


        if (color_1_1==color_2_1) similares++;
    }

    return similares;
}

//Usando la division incorrecta de bloques de 32
void screen_rainbow_effect_decodenagra_group_bad(z80_int *destino,int ancho,int alto,int y)
{

    int fingrupo=y+SCREEN_EFFECT_NAGRAVISION_GROUP_LINES;

    //Siempre comparar con la linea anterior
    y--;
    if (y<0) y=0;


    for (;y<alto && y<fingrupo;y++) {
        //y: linea a comparar con las de abajo (empezando en y+1)
        int y2_cambiar=y+1;
        int max_similar=0;

        //printf("Inicio buscar y %d\n",y);

        int y2;

        //Comparar con cada una
        for (y2=y+1;y2<alto && y2<fingrupo;y2++) {

            int similares=screen_rainbow_effect_sortalike_compare(destino,ancho,y,y2);
            //printf("- y %3d y2 %3d similares %d max_similar %d\n",y,y2,similares,max_similar);
            if (similares>max_similar) {
                max_similar=similares;
                y2_cambiar=y2;
                //printf("- y %3d y2 %3d similares %d max_similar %d\n",y,y2,similares,max_similar);
            }

        }

        //Si es la misma linea la que se pretende intercambiar, no hacer nada
        if (y+1!=y2_cambiar) {
            //printf("Cambiar - y %3d y2 %3d max_similar %d\n",y+1,y2_cambiar,max_similar);
            screen_rainbow_effect_nagravision_swap(destino,ancho,alto,y+1,y2_cambiar);
        }
    }

}


//Usando la division incorrecta de bloques de 32
void screen_rainbow_effect_decodenagravision_bad(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    int ygrupo;

    for (ygrupo=0;ygrupo<alto;ygrupo+=SCREEN_EFFECT_NAGRAVISION_GROUP_LINES) {


        screen_rainbow_effect_decodenagra_group_bad(destino,ancho,alto,ygrupo);

    }


}

//array para saber la linea original que tiene cada una antes de moverla, para evitar moverlas demasiado lejos
#define SCREEN_RAINBOW_EFFECT_DECODENAGRA_MAX_ORIG_LINES 1000
int screen_rainbow_effect_decodenagra_linea_original[SCREEN_RAINBOW_EFFECT_DECODENAGRA_MAX_ORIG_LINES];

void screen_rainbow_effect_decodenagra_group(z80_int *destino,int ancho,int alto,int y)
{

    int fingrupo=y+SCREEN_EFFECT_NAGRAVISION_GROUP_LINES;


    for (;y<alto && y<fingrupo;y++) {
        //y: linea a comparar con las de abajo (empezando en y+1)
        int y2_cambiar=y+1;
        int max_similar=0;

        //printf("Inicio buscar y %d\n",y);

        int y2;

        //Comparar con cada una
        for (y2=y+1;y2<alto && y2<fingrupo;y2++) {

            int similares=screen_rainbow_effect_sortalike_compare(destino,ancho,y,y2);
            //printf("- y %3d y2 %3d similares %d max_similar %d\n",y,y2,similares,max_similar);
            if (similares>max_similar) {
                max_similar=similares;
                y2_cambiar=y2;
                //printf("- y %3d y2 %3d similares %d max_similar %d\n",y,y2,similares,max_similar);
            }

        }

        //Si es la misma linea la que se pretende intercambiar, no hacer nada
        if (y+1!=y2_cambiar) {

            int en_limite_lineas=1;
            int linea1=0;
            int linea2=0; //inicializar con algo para que no se queje el compilador


            int cambiar=0;
            //ver si en el rango de array de lineas originales
            if (y2_cambiar>=SCREEN_RAINBOW_EFFECT_DECODENAGRA_MAX_ORIG_LINES || y+1>=SCREEN_RAINBOW_EFFECT_DECODENAGRA_MAX_ORIG_LINES) {
                en_limite_lineas=0;
                cambiar=1;
            }

            if (en_limite_lineas) {
                //ver si no esta muy lejos de su pos inicial
                linea1=screen_rainbow_effect_decodenagra_linea_original[y2_cambiar];
                linea2=screen_rainbow_effect_decodenagra_linea_original[y+1];

                if (util_get_absolute(y+1-linea1)<=SCREEN_EFFECT_NAGRAVISION_GROUP_LINES && util_get_absolute(y2_cambiar-linea2)<=SCREEN_EFFECT_NAGRAVISION_GROUP_LINES) cambiar=1;

            }


            //printf("Cambiar - y %3d y2 %3d max_similar %d\n",y+1,y2_cambiar,max_similar);
            if (cambiar) {
                //printf("Cambiar - %d %d diferencia %d\n",linea1,linea2,util_get_absolute(y+1-linea1));
                screen_rainbow_effect_nagravision_swap(destino,ancho,alto,y+1,y2_cambiar);
                if (en_limite_lineas) {
                    screen_rainbow_effect_decodenagra_linea_original[y2_cambiar]=linea2;
                    screen_rainbow_effect_decodenagra_linea_original[y+1]=linea1;
                }
            }

        }
    }

}

void screen_rainbow_effect_decodenagravision(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    int i;
    for (i=0;i<SCREEN_RAINBOW_EFFECT_DECODENAGRA_MAX_ORIG_LINES;i++) screen_rainbow_effect_decodenagra_linea_original[i]=i;

    int y;

    for (y=0;y<alto;y++) {

        //buscar lineas arriba o abajo a ver cual se parece mas

        screen_rainbow_effect_decodenagra_group(destino,ancho,alto,y);

    }

    //for (i=0;i<320;i++) printf("%d %d\n",i,screen_rainbow_effect_decodenagra_linea_original[i]);
    //printf("-----\n");

}

void screen_rainbow_effect_sortalike(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    int y,y2;

    for (y=0;y<alto;y++) {

        int y2_cambiar=y+1;
        int max_similar=0;

        for (y2=y+1;y2<alto;y2++) {

            int similares=screen_rainbow_effect_sortalike_compare(destino,ancho,y,y2);
            //printf("- y %3d y2 %3d similares %d max_similar %d\n",y,y2,similares,max_similar);
            if (similares>max_similar) {
                max_similar=similares;
                y2_cambiar=y2;
            }

        }

        //printf("Cambiar - y %3d y2 %3d max_similar %d\n",y+1,y2_cambiar,max_similar);
        screen_rainbow_effect_nagravision_swap(destino,ancho,alto,y+1,y2_cambiar);

    }


}


int screen_rainbow_effect_logorebound_x=0;
int screen_rainbow_effect_logorebound_x_inc=+1;
int screen_rainbow_effect_logorebound_y=0;
int screen_rainbow_effect_logorebound_y_inc=+1;

void screen_rainbow_effect_logorebound(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    screen_put_watermark_generic(destino,screen_rainbow_effect_logorebound_x,screen_rainbow_effect_logorebound_y,ancho,alto,screen_generic_putpixel_indexcolour);

    screen_rainbow_effect_logorebound_x +=screen_rainbow_effect_logorebound_x_inc;
    if (screen_rainbow_effect_logorebound_x==0 || screen_rainbow_effect_logorebound_x==ancho-ZESARUX_ASCII_LOGO_ANCHO) {
        screen_rainbow_effect_logorebound_x_inc=-screen_rainbow_effect_logorebound_x_inc;
    }

    screen_rainbow_effect_logorebound_y +=screen_rainbow_effect_logorebound_y_inc;
    if (screen_rainbow_effect_logorebound_y==0 || screen_rainbow_effect_logorebound_y==alto-ZESARUX_ASCII_LOGO_ALTO) {
        screen_rainbow_effect_logorebound_y_inc=-screen_rainbow_effect_logorebound_y_inc;
    }




}

void screen_rainbow_effect_interferences(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);


    int x,y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int valor=util_get_random() % 10000;

            if (valor<1000) {

                int offset=y*ancho+x;

                destino[offset]=15; //blanco

            }

        }

    }

}

#define SCREEN_EFFECT_ZOOM_MOUSE_ZOOM_FACTOR 4
#define SCREEN_EFFECT_ZOOM_MOUSE_SIZE 20

void screen_rainbow_effect_zoom_mouse_putpixel(z80_int *destino,int ancho,int alto,int dest_x,int dest_y,int color)
{
    int zx,zy;

    for (zy=0;zy<SCREEN_EFFECT_ZOOM_MOUSE_ZOOM_FACTOR;zy++) {
        for (zx=0;zx<SCREEN_EFFECT_ZOOM_MOUSE_ZOOM_FACTOR;zx++) {
            if (dest_x+zx>=0 && dest_y+zy>=0 && dest_x+zx<ancho && dest_y+zy<alto) {
                int offset_dest=(dest_y+zy)*ancho+dest_x+zx;
                destino[offset_dest]=color;
            }
        }
    }
}

void screen_rainbow_effect_zoom_mouse(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    int orig_y=mouse_y/zoom_y;
    int dest_y=orig_y;

    orig_y -=SCREEN_EFFECT_ZOOM_MOUSE_SIZE/2;
    dest_y -=(SCREEN_EFFECT_ZOOM_MOUSE_SIZE/2)*SCREEN_EFFECT_ZOOM_MOUSE_ZOOM_FACTOR;

    int inc_x,inc_y;

    for (inc_y=0;inc_y<SCREEN_EFFECT_ZOOM_MOUSE_SIZE;inc_y++,orig_y++) {

        int orig_x=mouse_x/zoom_x;
        int dest_x=orig_x;

        orig_x -=SCREEN_EFFECT_ZOOM_MOUSE_SIZE/2;
        dest_x -=(SCREEN_EFFECT_ZOOM_MOUSE_SIZE/2)*SCREEN_EFFECT_ZOOM_MOUSE_ZOOM_FACTOR;

        for (inc_x=0;inc_x<SCREEN_EFFECT_ZOOM_MOUSE_SIZE;inc_x++,orig_x++) {

            z80_int color=0;

            if (orig_x>=0 && orig_y>=0 && orig_x<ancho && orig_y<alto) {
                int offset_orig=orig_y*ancho+orig_x;
                color=origen[offset_orig];
            }

            else {
                //Pixeles que en origen salen de rango, mostrarlos como tramado
                int sum=orig_x+orig_y;
                if ((sum % 2)==0) color=15;
            }

            screen_rainbow_effect_zoom_mouse_putpixel(destino,ancho,alto,dest_x,dest_y,color);

            dest_x +=SCREEN_EFFECT_ZOOM_MOUSE_ZOOM_FACTOR;
        }

        dest_y +=SCREEN_EFFECT_ZOOM_MOUSE_ZOOM_FACTOR;

    }


}

int screen_rainbow_effect_radar_grados=0;

z80_int *screen_rainbow_effect_radar_putpixel_origen;
z80_int *screen_rainbow_effect_radar_putpixel_destino;
int screen_rainbow_effect_radar_putpixel_ancho;
int screen_rainbow_effect_radar_putpixel_alto;

void screen_rainbow_effect_radar_putpixel(zxvision_window *w GCC_UNUSED,int x,int y,int incremento_verde)
{

    if (x<0 || y<0 || x>=screen_rainbow_effect_radar_putpixel_ancho || y>=screen_rainbow_effect_radar_putpixel_alto) return;

    int color=screen_rainbow_effect_radar_putpixel_origen[y*screen_rainbow_effect_radar_putpixel_ancho + x];

    unsigned int color32=spectrum_colortable[color];
    int red=(color32 >> 16) & 0xFF;
    int green=(color32 >> 8) & 0xFF;
    int blue=(color32   ) & 0xFF;

    red=(red>>3) & 0x1F;
    green=(green>>3) & 0x1F;
    blue=(blue>>3) & 0x1F;

    //Sumamos color verde
    green=green+incremento_verde;
    if (green>0x1f) green=0x1f;

    //Usamos tabla de TSCONF_INDEX_FIRST_COLOR que tiene 5 bits por componente
    int rgb15=(red<<10) | (green<<5) | blue;
    color=TSCONF_INDEX_FIRST_COLOR+rgb15;

    screen_rainbow_effect_radar_putpixel_destino[y*screen_rainbow_effect_radar_putpixel_ancho + x] = color;
}

void screen_rainbow_effect_radar(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    screen_rainbow_effect_radar_putpixel_origen=origen;
    screen_rainbow_effect_radar_putpixel_destino=destino;
    screen_rainbow_effect_radar_putpixel_ancho=ancho;
    screen_rainbow_effect_radar_putpixel_alto=alto;

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    int centro_x=ancho/2;
    int centro_y=alto/2;

    int longitud_radar;

    if (ancho<alto) longitud_radar=ancho/2;
    else longitud_radar=alto/2;

    int grados=screen_rainbow_effect_radar_grados;

    int i;

    //Incrementar componente verde hasta el maximo y en cada grado del radar
    for (i=1;i<=0x1f;i++,grados++) {

        int punto_linea_radar_x=centro_x+(longitud_radar*util_get_cosine(grados)/10000);
        int punto_linea_radar_y=centro_y-(longitud_radar*util_get_sine(grados)/10000);

        //Usamos el parametro de color para indicarle cuanto incrementamos el componente verde
        zxvision_draw_line(NULL,centro_x,centro_y,punto_linea_radar_x,punto_linea_radar_y,i,screen_rainbow_effect_radar_putpixel);

    }

    screen_rainbow_effect_radar_grados++;


}

int screen_rainbow_effect_hsync_lost_x_inicial=0;

void screen_rainbow_effect_hsync_lost(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int xorigen=screen_rainbow_effect_hsync_lost_x_inicial;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {
            int offset_origen=y*ancho+xorigen;
            int offset_destino=y*ancho+x;

            int color=origen[offset_origen];

            //ultimas columnas tienen color negro como se aprecian en los hsync
            if (xorigen>=ancho-20) color=0;

            destino[offset_destino]=color;

            xorigen++;
            if (xorigen==ancho) xorigen=0;

        }


    }

    screen_rainbow_effect_hsync_lost_x_inicial++;
    if (screen_rainbow_effect_hsync_lost_x_inicial==ancho) screen_rainbow_effect_hsync_lost_x_inicial=0;


}


int screen_rainbow_effect_vsync_lost_y_inicial=0;

void screen_rainbow_effect_vsync_lost(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int yorigen=screen_rainbow_effect_vsync_lost_y_inicial;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {
            int offset_origen=yorigen*ancho+x;
            int offset_destino=y*ancho+x;

            int color=origen[offset_origen];

            //ultimas lineas tienen color negro como se aprecian en los vsync
            if (yorigen>=alto-20) color=0;

            destino[offset_destino]=color;

        }

        yorigen++;
        if (yorigen==alto) yorigen=0;
    }

    screen_rainbow_effect_vsync_lost_y_inicial++;
    if (screen_rainbow_effect_vsync_lost_y_inicial==alto) screen_rainbow_effect_vsync_lost_y_inicial=0;


}



void screen_rainbow_effect_attraction(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    //int radio=alto/2; // radio de efecto (ej: 100)


    int mx=mouse_x/zoom_x;
    int my=mouse_y/zoom_y;


    //int radio2 = radio * radio;

    for (y = 0; y < alto; y++) {
        for (x = 0; x < ancho; x++) {

            int dx = x - mx;
            int dy = y - my;

            int signo_dx=(dx<0 ? -1 : +1 );
            int signo_dy=(dy<0 ? -1 : +1 );

            int dx2=dx*dx;
            int dy2=dy*dy;

            int dist2 = dx2 + dy2;

            int atraccion=screen_rainbow_effect_attraction_atrac_repulse;

            if (1/*dist2 < radio2*/) {

                int factor=20000*screen_rainbow_effect_attraction_intensity;

                int srcX;
                int srcY;
                if (dist2==0) {
                    srcX=x+atraccion*signo_dx*factor*100;
                    srcY=y+atraccion*signo_dy*factor*100;
                }

                else {
                    srcX = x+atraccion*signo_dx*factor/dist2;
                    srcY = y+atraccion*signo_dy*factor/dist2;
                }

                // clamp
                if (srcX < 0) srcX = 0;
                if (srcX >= ancho) srcX = ancho - 1;
                if (srcY < 0) srcY = 0;
                if (srcY >= alto) srcY = alto - 1;

                destino[y*ancho + x] = origen[srcY*ancho + srcX];


            }
            else {
                destino[y*ancho + x] = origen[y*ancho + x];
            }
        }
    }


}




#define SCREEN_PIXELATE_ARRAY_LIST_LENGTH (SCREEN_EFFECT_PIXELATE_MAX_SIZE*SCREEN_EFFECT_PIXELATE_MAX_SIZE)

int screen_pixelate_array_list[SCREEN_PIXELATE_ARRAY_LIST_LENGTH];

int screen_rainbow_effect_pixelate_get_color(z80_int *origen,int ancho,int x,int y,int pixelate_size)
{

    int zx,zy;

    int offset_array=0;

    for (zy=0;zy<pixelate_size;zy++) {
        for (zx=0;zx<pixelate_size;zx++) {
            int offset_origen=(y+zy)*ancho+x+zx;
            screen_pixelate_array_list[offset_array++]=origen[offset_origen];
        }
    }

    //Ahora recorremos la lista y vemos cual es el color que mas se repite
    int color_mas_repetido=0;
    int veces_mas_repetido=0;

    int i,j;

    for (i=0;i<pixelate_size;i++) {

        int color_buscar=screen_pixelate_array_list[i];

        int veces_leido=0;

        //Cuantas veces aparece ese color
        for (j=0;j<pixelate_size;j++) {
            if (screen_pixelate_array_list[j]==color_buscar) veces_leido++;
        }

        if (veces_leido>veces_mas_repetido) {
            veces_mas_repetido=veces_leido;
            color_mas_repetido=color_buscar;
        }
    }

    return color_mas_repetido;

}



void screen_rainbow_effect_pixelate(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {
            int pixelate_size=screen_rainbow_effect_pixelate_size;

            if (screen_rainbow_effect_pixelate_follow_mouse.v) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                int max_intensity=SCREEN_EFFECT_PIXELATE_MAX_SIZE;

                pixelate_size=dist/200;

                pixelate_size=max_intensity-pixelate_size;

                if (pixelate_size>SCREEN_EFFECT_PIXELATE_MAX_SIZE) pixelate_size=max_intensity;

                if (pixelate_size<1) pixelate_size=1;


            }

            int yorigen=y/pixelate_size;
            int xorigen=x/pixelate_size;

            yorigen *=pixelate_size;
            xorigen *=pixelate_size;

            //int offset_origen=yorigen*ancho+xorigen;
            int offset_destino=y*ancho+x;

            int color;

            //color=origen[offset_origen];



            //Nota: aunque nuestros buffers no son rgb, vamos a obtener el color que mas se repite
            //Lo ideal seria tener un buffer rgb y sacar un promedio de color de todo el recuadro a pixelar
            color=screen_rainbow_effect_pixelate_get_color(origen,ancho,xorigen,yorigen,pixelate_size);

            destino[offset_destino]=color;

        }

    }


}

int screen_rainbow_effect_improved_waves_tiempo=0;

void screen_rainbow_effect_improved_waves(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            //Cada 30 pixeles en alto, una vuelta entera 360 grados
            int off=((y+screen_rainbow_effect_improved_waves_tiempo/1) % 30)*(360/30);

            int intensity=screen_rainbow_effect_improved_waves_intensity;

            if (screen_rainbow_effect_improved_waves_follow_mouse.v) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                int max_intensity=20;

                intensity=dist/200;

                intensity=max_intensity-intensity;

                if (intensity>max_intensity) intensity=max_intensity;
                if (intensity<0) intensity=0;

            }


            int offset=intensity*util_get_cosine(off)/10000;


            int sx = (x + offset);


            int color;
            if (sx<0) sx=0;
            if (sx>=ancho) sx=ancho-1;

            color=origen[y*ancho +sx];

            destino[y*ancho + x] = color;


        }

    }

    screen_rainbow_effect_improved_waves_tiempo++;


}



void screen_rainbow_effect_shear(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int shear_factor=screen_rainbow_effect_shear_intensity;

            if (screen_rainbow_effect_shear_intensity_follow_mouse.v) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                int max_intensity=50;

                shear_factor=dist/500;


                if (shear_factor>max_intensity) shear_factor=max_intensity;
                if (shear_factor<1) shear_factor=1;

            }


            // shear / "cizalla", "cisallament"
            int sx = x + y / shear_factor;

            int color;

            if (sx<0 || sx>=ancho) color=0;
            else color=origen[y*ancho + sx];

            destino[y*ancho + x] = color;


        }

    }



}




int screen_rainbow_effect_fadeinout_percentage=0;
int screen_rainbow_effect_fadeinout_sign=+1;

void screen_rainbow_effect_fadeinout(z80_int *origen,z80_int *destino,int ancho,int alto,int fadein)
{

    int x,y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int color=origen[y*ancho + x];

            unsigned int color32=spectrum_colortable[color];
            int red=(color32 >> 16) & 0xFF;
            int green=(color32 >> 8) & 0xFF;
            int blue=(color32   ) & 0xFF;

            red=(red>>3) & 0x1F;
            green=(green>>3) & 0x1F;
            blue=(blue>>3) & 0x1F;

            //Aplicar %
            red=(red*screen_rainbow_effect_fadeinout_percentage)/100;
            green=(green*screen_rainbow_effect_fadeinout_percentage)/100;
            blue=(blue*screen_rainbow_effect_fadeinout_percentage)/100;

            //Usamos tabla de TSCONF_INDEX_FIRST_COLOR que tiene 5 bits por componente
            int rgb15=(red<<10) | (green<<5) | blue;
            color=TSCONF_INDEX_FIRST_COLOR+rgb15;

            destino[y*ancho + x] = color;

        }

    }

    if (fadein) {
        screen_rainbow_effect_fadeinout_percentage+=3;
        if (screen_rainbow_effect_fadeinout_percentage>100) screen_rainbow_effect_fadeinout_percentage=0;
    }
    else if (fadein<0) {
        screen_rainbow_effect_fadeinout_percentage-=3;
        if (screen_rainbow_effect_fadeinout_percentage<0) screen_rainbow_effect_fadeinout_percentage=100;
    }
    else { //In-Out
        if (screen_rainbow_effect_fadeinout_sign>0) {
            screen_rainbow_effect_fadeinout_percentage+=3;
            if (screen_rainbow_effect_fadeinout_percentage>100) screen_rainbow_effect_fadeinout_sign=-1;
        }
        else {
            screen_rainbow_effect_fadeinout_percentage-=3;
            if (screen_rainbow_effect_fadeinout_percentage<0) screen_rainbow_effect_fadeinout_sign=+1;
        }
    }

}


void screen_rainbow_effect_scanlines(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int color=origen[y*ancho + x];

            //Bajar brillo lineas pares
            if (y%2) {

                unsigned int color32=spectrum_colortable[color];
                int red=(color32 >> 16) & 0xFF;
                int green=(color32 >> 8) & 0xFF;
                int blue=(color32   ) & 0xFF;

                red=(red>>3) & 0x1F;
                green=(green>>3) & 0x1F;
                blue=(blue>>3) & 0x1F;


                red /=2;
                green /=2;
                blue /=2;

                //Usamos tabla de TSCONF_INDEX_FIRST_COLOR que tiene 5 bits por componente
                int rgb15=(red<<10) | (green<<5) | blue;
                color=TSCONF_INDEX_FIRST_COLOR+rgb15;
            }


            destino[y*ancho + x] = color;

        }

    }


}



void screen_rainbow_effect_sepia(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int color=origen[y*ancho + x];
            unsigned int color32=spectrum_colortable[color];

            int red=(color32 >> 16) & 0xFF;
            int green=(color32 >> 8) & 0xFF;
            int blue=(color32   ) & 0xFF;

            int orig_red=red;
            int orig_green=green;
            int orig_blue=blue;


            red=(red>>3) & 0x1F;
            green=(green>>3) & 0x1F;
            blue=(blue>>3) & 0x1F;


            int tr = (393 * red + 769 * green + 189 * blue);
            int tg = (349 * red + 686 * green + 168 * blue);
            int tb = (272 * red + 534 * green + 131 * blue);


            if (screen_rainbow_effect_sepia_follow_mouse.v) {
                int mx=mouse_x/zoom_x;
                int my=mouse_y/zoom_y;

                int dx=mx-x;
                int dy=my-y;

                //Nota: estos factores de division y multiplicacion están puestos a ojo para que quede mas o menos bien visualmente
                int dist=(dx*dx+dy*dy)/1000;

                if (dist!=0) {

                    tr=(tr/dist+orig_red)*10;
                    tg=(tg/dist+orig_green)*10;
                    tb=(tb/dist+orig_blue)*10;

                }
            }

            tr /=1000;
            tg /=1000;
            tb /=1000;

            red = (tr > 31) ? 31 : tr;
            green = (tg > 31) ? 31 : tg;
            blue = (tb > 31) ? 31 : tb;

            int rgb15=(red<<10) | (green<<5) | blue;


            color=TSCONF_INDEX_FIRST_COLOR+rgb15;

            destino[y*ancho + x] = color;

        }

    }


}

//Retorna color rgb15 del pixel indicado, sumandolo en variables de entrada
void screen_rainbow_effect_blur_media_rgb_pixeles(z80_int *origen,int orig_x,int orig_y,int ancho,int alto,int total_ancho,int total_alto,int x,int y,int *red,int *green,int *blue)
{
    if (
        x<0 || y<0 || x>=total_ancho || y>=total_alto ||
        x<orig_x || y<orig_y || x>=orig_x+ancho || y>=orig_y+alto
        ) {
        return;
    }

    int color=origen[y*total_ancho + x];
    unsigned int color32=spectrum_colortable[color];

    *red +=((color32 >> 16) >> 3) & 0x1F;
    *green +=((color32 >> 8) >> 3) & 0x1F;
    *blue +=((color32) >> 3  ) & 0x1F;

}

//media de color del pixel indicado y los de alrededor, retorna en rgb15
//Si intensity=0 devolverá el color del pixel indicado, tal cual. O sea, no hace blur
int screen_rainbow_effect_blur_media(z80_int *origen,int orig_x,int orig_y,int ancho,int alto,int x,int y,int total_ancho,int total_alto,int intensity)
{
    int red=0;
    int green=0;
    int blue=0;

    int total_pixeles=0;

    int dx,dy;

    for (dy=-intensity;dy<=+intensity;dy++) {
        for (dx=-intensity;dx<=+intensity;dx++) {
            screen_rainbow_effect_blur_media_rgb_pixeles(origen,orig_x,orig_y,ancho,alto,total_ancho,total_alto,x+dx,y+dy,&red,&green,&blue);
            total_pixeles++;
        }
    }


    if (total_pixeles!=0) {
        red /=total_pixeles;
        green /=total_pixeles;
        blue /=total_pixeles;
    }

    int rgb15=(red<<10) | (green<<5) | blue;

    return rgb15;

}

void screen_rainbow_effect_blur_zone(z80_int *origen,z80_int *destino,int orig_x,int orig_y,int ancho,int alto,int total_ancho,int total_alto,int intensity,int follow_mouse)
{

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=orig_y;y<orig_y+alto;y++) {
        for (x=orig_x;x<orig_x+ancho;x++) {

            if (follow_mouse) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                //con 10 ya se usa mucha cpu
                int max_intensity=10;

                intensity=dist/500;

                intensity=max_intensity-intensity;

                if (intensity>max_intensity) intensity=max_intensity;
                if (intensity<0) intensity=0;

            }

            int color=TSCONF_INDEX_FIRST_COLOR+screen_rainbow_effect_blur_media(origen,orig_x,orig_y,ancho,alto,x,y,total_ancho,total_alto,intensity);

            destino[y*total_ancho + x] = color;

        }

    }


}



void screen_rainbow_effect_blur(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    screen_rainbow_effect_blur_zone(origen,destino,0,0,ancho,alto,ancho,alto,screen_rainbow_effect_blur_intensity,screen_rainbow_effect_blur_follow_mouse.v);

}



void screen_rainbow_effect_shaderborder_putpixel(z80_int *destino,int ancho,int alto,int x,int y,int color)
{
    if (x<0 || y<0 || x>=ancho || y>=alto) return;

    destino[y*ancho+x]=color;

}

int screen_rainbow_effect_shaderborder_getpixel(z80_int *origen,int ancho,int alto,int x,int y)
{
    if (x<0 || y<0 || x>=ancho || y>=alto) return 0;

    return origen[y*ancho+x];

}

void screen_rainbow_effect_shaderborder_copy(z80_int *origen,z80_int *destino,int ancho_total,int alto_total,int factor_zoom_x,int factor_zoom_y,
    int ancho,int alto,int orig_x,int orig_y,int dest_x,int dest_y)
{
    //Copiamos trozo de pantalla hacia border
    int x,y;


    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int color=screen_rainbow_effect_shaderborder_getpixel(origen,ancho_total,alto_total,orig_x+x*factor_zoom_x/1000,orig_y+y*factor_zoom_y/1000);
            screen_rainbow_effect_shaderborder_putpixel(destino,ancho_total,alto_total,dest_x+x,y+dest_y,color);

        }
    }

}


void screen_rainbow_effect_shaderborder_lateral(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    z80_int *temp_bufferdestino=screen_special_effects_alloc_buffer(ancho,alto);

    //Copiamos trozo de pantalla hacia border
    //int x,y;

    screen_rainbow_effect_shaderborder_copy(origen,temp_bufferdestino,ancho,alto,screen_rainbow_effect_shaderborder_factor_zoom_leftright,1000,
        LEFT_BORDER_NO_ZOOM,192,LEFT_BORDER_NO_ZOOM,TOP_BORDER_NO_ZOOM,0,TOP_BORDER_NO_ZOOM);

    screen_rainbow_effect_shaderborder_copy(origen,temp_bufferdestino,ancho,alto,screen_rainbow_effect_shaderborder_factor_zoom_leftright,1000,
        LEFT_BORDER_NO_ZOOM,192,
        LEFT_BORDER_NO_ZOOM+(256-LEFT_BORDER_NO_ZOOM*screen_rainbow_effect_shaderborder_factor_zoom_leftright/1000),TOP_BORDER_NO_ZOOM, //origen
        LEFT_BORDER_NO_ZOOM+256,TOP_BORDER_NO_ZOOM  //destino
    );



    //y blur en el borde izquierdo
    screen_rainbow_effect_blur_zone(temp_bufferdestino,destino,0,TOP_BORDER_NO_ZOOM,LEFT_BORDER_NO_ZOOM,192,
        ancho,alto,screen_rainbow_effect_shaderborder_blur_intensity_leftright,0);

    //y en el derecho
    screen_rainbow_effect_blur_zone(temp_bufferdestino,destino,LEFT_BORDER_NO_ZOOM+256,TOP_BORDER_NO_ZOOM,LEFT_BORDER_NO_ZOOM,192,
        ancho,alto,screen_rainbow_effect_shaderborder_blur_intensity_leftright,0);

    free(temp_bufferdestino);


}

void screen_rainbow_effect_shaderborder_infsup(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    z80_int *temp_bufferdestino=screen_special_effects_alloc_buffer(ancho,alto);

    //Copiamos trozo de pantalla hacia border
    //int x,y;

    screen_rainbow_effect_shaderborder_copy(origen,temp_bufferdestino,ancho,alto,1000,screen_rainbow_effect_shaderborder_factor_zoom_updown,
        ancho,TOP_BORDER_NO_ZOOM,
        0,TOP_BORDER_NO_ZOOM,
        0,0
    );

    screen_rainbow_effect_shaderborder_copy(origen,temp_bufferdestino,ancho,alto,1000,screen_rainbow_effect_shaderborder_factor_zoom_updown,
        ancho,TOP_BORDER_NO_ZOOM,
        0,TOP_BORDER_NO_ZOOM+192-TOP_BORDER_NO_ZOOM*screen_rainbow_effect_shaderborder_factor_zoom_updown/1000,
        0,TOP_BORDER_NO_ZOOM+192);



    //y blur en el borde superior
    screen_rainbow_effect_blur_zone(temp_bufferdestino,destino,0,0,ancho,TOP_BORDER_NO_ZOOM,
        ancho,alto,screen_rainbow_effect_shaderborder_blur_intensity_updown,0);

    //y en el borde inferior
    screen_rainbow_effect_blur_zone(temp_bufferdestino,destino,0,TOP_BORDER_NO_ZOOM+192,ancho,TOP_BORDER_NO_ZOOM,
        ancho,alto,screen_rainbow_effect_shaderborder_blur_intensity_updown,0);

    free(temp_bufferdestino);


}

void screen_rainbow_effect_shaderborder(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero copiar tal cual de origen a destino
    int tamanyo=ancho*alto*2;
    memcpy(destino,origen,tamanyo);

    if (screen_rainbow_effect_shaderborder_leftright_enable.v) screen_rainbow_effect_shaderborder_lateral(origen,destino,ancho,alto);
    if (screen_rainbow_effect_shaderborder_updown_enable.v) screen_rainbow_effect_shaderborder_infsup(origen,destino,ancho,alto);
}




void screen_rainbow_effect_contrast(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int factor=screen_rainbow_effect_contrast_intensity;

            if (screen_rainbow_effect_contrast_follow_mouse.v) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                int max_intensity=1000;

                if (dist==0) factor=max_intensity;
                else factor=1000*max_intensity/dist;

                if (factor>max_intensity) factor=max_intensity;

                if (factor<0) factor=0;

            }

            int color=origen[y*ancho + x];
            unsigned int color32=spectrum_colortable[color];

            int red=(color32 >> 16) & 0xFF;
            int green=(color32 >> 8) & 0xFF;
            int blue=(color32   ) & 0xFF;


            /*
| factor | efecto                |
| ------ | --------------------- |
| 0      | gris plano (todo 128) |
| 128    | bajo contraste        |
| 256    | normal (sin cambio)   |
| 384    | alto contraste        |
| 512    | muy fuerte (clipping) |

            */


            int val;

            val = ((factor * (red - 128)) >> 8) + 128;
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            red = val;

            val = ((factor * (green - 128)) >> 8) + 128;
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            green = val;

            val = ((factor * (blue - 128)) >> 8) + 128;
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            blue = val;

            red=(red>>3) & 0x1F;
            green=(green>>3) & 0x1F;
            blue=(blue>>3) & 0x1F;

            int rgb15=(red<<10) | (green<<5) | blue;


            color=TSCONF_INDEX_FIRST_COLOR+rgb15;

            destino[y*ancho + x] = color;

        }

    }


}



void screen_rainbow_effect_brightness(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int brightness=screen_rainbow_effect_brightness_intensity;


            if (screen_rainbow_effect_brightness_follow_mouse.v) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                int max_intensity=1000;

                if (dist==0) brightness=max_intensity;
                else brightness=50*max_intensity/dist;

                //brightness=max_intensity-brightness;

                if (brightness>max_intensity) brightness=max_intensity;

                if (brightness<0) brightness=0;

            }




            int color=origen[y*ancho + x];
            unsigned int color32=spectrum_colortable[color];

            int red=(color32 >> 16) & 0xFF;
            int green=(color32 >> 8) & 0xFF;
            int blue=(color32   ) & 0xFF;

            //permitir color negro incrementarse a partir de cierto factor
            if (brightness>1000) {
                if (!red) red=1;
                if (!green) green=1;
                if (!blue) blue=1;
            }


            red=(red*brightness)/100;
            if (red>255) red=255;

            green=(green*brightness)/100;
            if (green>255) green=255;

            blue=(blue*brightness)/100;
            if (blue>255) blue=255;

            red=(red>>3) & 0x1F;
            green=(green>>3) & 0x1F;
            blue=(blue>>3) & 0x1F;

            int rgb15=(red<<10) | (green<<5) | blue;


            color=TSCONF_INDEX_FIRST_COLOR+rgb15;

            destino[y*ancho + x] = color;

        }

    }


}




//Punteros de los frames anteriores
//Los guardamos en formato RGB15 bits

//frame [0] es el mas antiguo
z80_int *screen_rainbow_effect_persistence_frames_mem[SCREEN_RAINBOW_EFFECT_PERSISTENCE_MAX_FRAMES];
int screen_rainbow_effect_persistence_ancho=-1;
int screen_rainbow_effect_persistence_alto=-1;


void screen_rainbow_effect_persistence_check_mem(int ancho,int alto)
{
    int asignar_mem=0;

    //Liberar si ha cambiado el ancho o alto
    if (screen_rainbow_effect_persistence_ancho!=-1 && screen_rainbow_effect_persistence_alto!=-1) {
        if (screen_rainbow_effect_persistence_ancho!=ancho || screen_rainbow_effect_persistence_alto!=alto) {
            int i;
            for (i=0;i<SCREEN_RAINBOW_EFFECT_PERSISTENCE_MAX_FRAMES;i++) {
                free(screen_rainbow_effect_persistence_frames_mem[i]);
            }
            asignar_mem=1;
        }
    }

    if (screen_rainbow_effect_persistence_ancho==-1 || screen_rainbow_effect_persistence_alto==-1) asignar_mem=1;

    if (asignar_mem) {
        int i;
        for (i=0;i<SCREEN_RAINBOW_EFFECT_PERSISTENCE_MAX_FRAMES;i++) {
            screen_rainbow_effect_persistence_frames_mem[i]=screen_special_effects_alloc_buffer(ancho,alto);
            screen_rainbow_effect_persistence_ancho=ancho;
            screen_rainbow_effect_persistence_alto=alto;
        }
    }

}

void screen_rainbow_effect_persistence(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    //Primero ver si hay que asignar o liberar memoria
    screen_rainbow_effect_persistence_check_mem(ancho,alto);

    int x,y;

    //Renderizar vista actual como mezcla de las anteriores

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {
            int offset=y*ancho + x;

            //Frame actual
            int color=origen[offset];
            unsigned int color32=spectrum_colortable[color];

            int red=(color32 >> 16) & 0xFF;
            int green=(color32 >> 8) & 0xFF;
            int blue=(color32   ) & 0xFF;

            red=(red>>3) & 0x1F;
            green=(green>>3) & 0x1F;
            blue=(blue>>3) & 0x1F;

            int red_original=red;
            int green_original=green;
            int blue_original=blue;


            //Frames anteriores
            int i;
            for (i=0;i<screen_rainbow_effect_persistence_total_frames;i++) {
                z80_int *puntero=screen_rainbow_effect_persistence_frames_mem[i];
                int color_frame_antes=puntero[offset];
                int color_frame_antes_red=(color_frame_antes >> 10) & 0x1F;
                int color_frame_antes_green=(color_frame_antes >> 5) & 0x1F;
                int color_frame_antes_blue=(color_frame_antes) & 0x1F;

                red +=color_frame_antes_red;
                green +=color_frame_antes_green;
                blue +=color_frame_antes_blue;
            }

            //media de todos.
            red /=(screen_rainbow_effect_persistence_total_frames+1);
            green /=(screen_rainbow_effect_persistence_total_frames+1);
            blue /=(screen_rainbow_effect_persistence_total_frames+1);

            int rgb15=(red<<10) | (green<<5) | blue;

            color=TSCONF_INDEX_FIRST_COLOR+rgb15;

            destino[y*ancho + x] = color;


            //Y ahora rotamos los frames. Primero los que tenemos anteriores, que son en formato RGB15
            for (i=0;i<screen_rainbow_effect_persistence_total_frames-1;i++) {
                z80_int *puntero_dest=screen_rainbow_effect_persistence_frames_mem[i];
                z80_int *puntero_orig=screen_rainbow_effect_persistence_frames_mem[i+1];
                int color_orig=puntero_orig[offset];

                //Mientras frame mas antiguo, mas oscuro
                int color_orig_red=(color_orig >>10) & 0x1f;
                int color_orig_green=(color_orig >>5) & 0x1f;
                int color_orig_blue=(color_orig) & 0x1f;

                //frame 0 el mas antiguo, que tenga menos brillo

                //15 para persistencia 10 frames
                //7.5 para persistencia 20 frames
                //30 para persistencia 5 frames
                //int porcentaje_reduccion=(15*10)/screen_rainbow_effect_persistence_total_frames;

                int factor_apagar=100-((screen_rainbow_effect_persistence_total_frames-i)*15)/screen_rainbow_effect_persistence_total_frames;

                if (factor_apagar<0) factor_apagar=0;
                if (factor_apagar>100) factor_apagar=100;

                //if (x==0 && y==0) printf("i %2d factor apagar %d\n",i,factor_apagar);

                color_orig_red=(color_orig_red*factor_apagar)/100;
                color_orig_green=(color_orig_green*factor_apagar)/100;
                color_orig_blue=(color_orig_blue*factor_apagar)/100;

                color_orig=(color_orig_red<<10) | (color_orig_green<<5) | color_orig_blue;

                puntero_dest[offset]=color_orig;
            }

            //Y el ultimo frame, que sale del buffer original, convirtiendo a RGB15
            z80_int *puntero_dest=screen_rainbow_effect_persistence_frames_mem[i];

            rgb15=(red_original<<10) | (green_original<<5) | blue_original;

            puntero_dest[offset]=rgb15;

        }

    }


}

void screen_rainbow_effect_led(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {
            //De cada 4 pixeles solo mostrar 1
            int mx=(x%2);
            int my=(y%2);
            if (mx==0 && my==0) {
                int color=origen[y*ancho + x];
                destino[y*ancho + x] = color;
            }

        }

    }


}



void screen_rainbow_effect_scroll_horizontal(z80_int *origen,z80_int *destino,int ancho,int alto)
{

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {

            int scroll=screen_rainbow_effect_scroll_horizontal_offset;

            if (screen_rainbow_effect_scroll_horizontal_follow_mouse.v) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                int max_intensity=ancho;

                if (dist==0) scroll=max_intensity;
                else scroll=1000*max_intensity/dist;

                if (scroll>max_intensity) scroll=max_intensity;

                if (scroll<0) scroll=0;

            }

            int orig_x = x+scroll;

            int color;

            if (screen_rainbow_effect_scroll_horizontal_circular.v) {
                if (orig_x<0) orig_x=ancho-1+(orig_x % ancho);
                else orig_x=orig_x % ancho;

                //if (orig_x<0 || orig_x>=ancho) printf("ERROR %d ancho: %d\n",orig_x,ancho);

                color=origen[y*ancho + orig_x];
            }

            else {
                if (orig_x<0 || orig_x>=ancho) color=0;
                else color=origen[y*ancho + orig_x];
            }


            destino[y*ancho + x] = color;

        }

    }

}



void screen_rainbow_effect_scroll_vertical(z80_int *origen,z80_int *destino,int ancho,int alto)
{
    //printf("%d %d %d %d\n",(-1 % 3),(-2 % 3),(-3 % 3),(-4 % 3));

    int x,y;

    int cx=mouse_x/zoom_x;
    int cy=mouse_y/zoom_y;

    for (y=0;y<alto;y++) {
        for (x=0;x<ancho;x++) {
            int scroll=screen_rainbow_effect_scroll_vertical_offset;

            if (screen_rainbow_effect_scroll_vertical_follow_mouse.v) {

                //distancia al raton
                int dx=cx-x;
                int dy=cy-y;

                int dist=dx*dx+dy*dy;

                int max_intensity=ancho;

                if (dist==0) scroll=max_intensity;
                else scroll=1000*max_intensity/dist;

                if (scroll>max_intensity) scroll=max_intensity;

                if (scroll<0) scroll=0;

            }

            int orig_y = y+scroll;

            int color;

            if (screen_rainbow_effect_scroll_vertical_circular.v) {
                if (orig_y<0) orig_y=alto-1+(orig_y % alto);
                else orig_y=orig_y % alto;

                //if (orig_y<0 || orig_y>=alto) printf("ERROR %d alto: %d\n",orig_y,alto);

                color=origen[orig_y*ancho + x];
            }

            else {
                if (orig_y<0 || orig_y>=alto) color=0;
                else color=origen[orig_y*ancho + x];
            }



            destino[y*ancho + x] = color;

        }

    }

}







void screen_rainbow_effect_fisheye_change_factor(void)
{
    screen_rainbow_effect_fisheye_factor_k +=50;
    if (screen_rainbow_effect_fisheye_factor_k>600) screen_rainbow_effect_fisheye_factor_k=-600;
}

void screen_rainbow_effect_fisheye_change_factor_slow(void)
{
    screen_rainbow_effect_fisheye_factor_k +=1;
    if (screen_rainbow_effect_fisheye_factor_k>400) screen_rainbow_effect_fisheye_factor_k=-600;
}


int screen_rainbow_effect_fisheye_frames=0;

void screen_rainbow_effect_fisheye(z80_int *origen,z80_int *destino,int ancho,int alto)
{


    int x,y;


    //printf("%d %d\n",screen_rainbow_effect_sea_offsets[0],screen_rainbow_effect_sea_offsets[1]);

    int cx=ancho/2;
    int cy=alto/2;

    //printf("%d\n",cy);

    if (screen_special_effects_fisheye_follow_mouse.v) {

        cx=mouse_x/zoom_x;
        cy=mouse_y/zoom_y;
        //printf("- %d\n",cy);
    }

    // radio máximo al cuadrado (hasta la esquina)
    int maxr2 = cx*cx + cy*cy;


    //Ajustar factor k segun musica
    if (screen_special_effects_fisheye_follow_music_channel) {
        char sensor_name[SENSORS_MAX_SHORT_NAME];
        sprintf(sensor_name,"ay_vol_chip0_chan_%c",screen_special_effects_fisheye_follow_music_channel);
        int sensor_id=sensor_find(sensor_name);

        if (sensor_id<0) return;


        int media_cpu_perc=sensor_get_percentaje_value_by_id(sensor_id);


        //total 12
        int nuevo_k=(12*media_cpu_perc);
        nuevo_k -=600;

        screen_rainbow_effect_fisheye_factor_k=nuevo_k;
    }




    for (y=0;y<alto;y++) {

        for (x=0;x<ancho;x++) {

            //distancias al centro
            int dx = x - cx;
            int dy = y - cy;

            //distancia al centro


            // distancia al cuadrado normalizada [0,1]  -> usamos 0...1000
            int r2;

            if (maxr2==0) r2=9999999;
            else r2 = (1000*(dx*dx + dy*dy)) / maxr2;

            //printf("%f\n",r2);

            // "lente real" (barrel distortion)
            //factor Si factor = 100000 → no hay deformación
            //Si factor > 100000 → estiras hacia afuera
            //Si factor < 100000 → comprimes hacia el centro
            int scale = 100000;  // factor de escala

            int mult = screen_rainbow_effect_fisheye_factor_k * r2;

            // factor = 1 + mult/100000
            int factor_fixed = scale + mult;

            int sx = cx + ((z80_64bit)dx * scale) / factor_fixed;
            int sy = cy + ((z80_64bit)dy * scale) / factor_fixed;



            if (sx<0) sx=0;
            if (sy<0) sy=0;
            if (sx>=ancho) sx=ancho-1;
            if (sy>=alto) sy=alto-1;





            int offset_origen=(sy*ancho)+sx;

            int offset_destino=y*ancho+x;
            destino[offset_destino]=origen[offset_origen];
        }


    }

    screen_rainbow_effect_fisheye_frames++;

    if (screen_special_effects_fisheye_automatic_factor.v) screen_rainbow_effect_fisheye_change_factor_slow();


}





#define SCREEN_EFFECT_WAVES_MAX_LINES 10000

int screen_rainbow_effect_sea_offsets[SCREEN_EFFECT_WAVES_MAX_LINES];

#define SCREEN_EFFECT_WAVES_MAX_OFFSET 4

int screen_rainbow_effect_sea_offset=SCREEN_EFFECT_WAVES_MAX_OFFSET/2;
int screen_rainbow_effect_sea_frames=0;

void screen_rainbow_effect_sea(z80_int *origen,z80_int *destino,int ancho,int alto)
{


    int x,y;

    int valor_random=util_get_random() % 30000;


    //printf("%d %d\n",screen_rainbow_effect_sea_offsets[0],screen_rainbow_effect_sea_offsets[1]);


    for (y=0;y<alto;y++) {

        for (x=0;x<ancho-SCREEN_EFFECT_WAVES_MAX_OFFSET;x++) {
            int offset_onda;

            if (y<SCREEN_EFFECT_WAVES_MAX_LINES) {
                offset_onda=screen_rainbow_effect_sea_offsets[y];
            }
            else offset_onda=0;

            int offset_origen=y*ancho+x+offset_onda;

            int offset_destino=y*ancho+x;
            destino[offset_destino]=origen[offset_origen];
        }

        //Cada cuantos frames moverse
        if ((screen_rainbow_effect_sea_frames%10)==0) {
            if (y>0 && y<SCREEN_EFFECT_WAVES_MAX_LINES) {
                int anterior_offset=screen_rainbow_effect_sea_offsets[y-1];
                int current_offset=screen_rainbow_effect_sea_offsets[y];

                //Lineas impares: copiar offset de la anterior linea
                //Lineas pares: offset segun random
                if ((y%2)==0) anterior_offset=current_offset;

                screen_rainbow_effect_sea_offsets[y]=anterior_offset;

                //33% de probabilidad de irse a la izquierda
                if (valor_random<10000) {
                    if (anterior_offset>0) {
                        screen_rainbow_effect_sea_offsets[y]=anterior_offset-1;
                    }
                }

                //33% de probabilidad de irse a la derecha
                else if (valor_random>20000) {
                    if (anterior_offset<SCREEN_EFFECT_WAVES_MAX_OFFSET) {
                        screen_rainbow_effect_sea_offsets[y]=anterior_offset+1;
                    }
                }

                //33% de probabilidad de no moverse
            }

            //Olas de 15 pixeles de alto
            if ((y%15)==0) valor_random=util_get_random() % 30000;

        }

    }

    screen_rainbow_effect_sea_frames++;


}



z80_int *screen_special_effects_functions(z80_int *origen,int ancho,int alto)
{
    z80_int *inicial_origen=origen;

    int aplicado_algo=0;
    z80_int *destino;

    int i;

    for (i=0;i<MAX_SCREEN_LIST_EFFECTS;i++) {
        if (screen_effect_applied_list[i].enabled && screen_effect_applied_list[i].type!=SCREEN_EFFECT_TYPE_NONE) {
            destino=screen_special_effects_alloc_buffer(ancho,alto);

            switch (screen_effect_applied_list[i].type) {
                case SCREEN_EFFECT_TYPE_NONE:
                break;

                case SCREEN_EFFECT_TYPE_REDUCE:
                    if (screen_reduction_factor==SCREEN_REDUCE_050) {
                        screen_scale_rainbow_21(origen,ancho,alto,destino);
                    }

                    else if (screen_reduction_factor==SCREEN_REDUCE_025) {
                        screen_scale_rainbow_41(origen,ancho,alto,destino);
                    }

                    else if (screen_reduction_factor==SCREEN_REDUCE_075) {
                        screen_scale_rainbow_43(origen,ancho,alto,destino);
                    }

                break;

                case SCREEN_EFFECT_TYPE_UNSTEADY:
                    screen_rainbow_effect_temblar(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_FLIP_VERTICAL:
                    screen_rainbow_effect_flip_vertical(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_FLIP_HORIZONTAL:
                    screen_rainbow_effect_flip_horizontal(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_ROTATE:
                    screen_rainbow_effect_rotate(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_TWIRL:
                    screen_rainbow_effect_remolino(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_INTERFERENCES:
                    screen_rainbow_effect_interferences(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_ZOOM_MOUSE:
                    screen_rainbow_effect_zoom_mouse(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_NAGRAVISION:
                    screen_rainbow_effect_nagravision(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_DECODENAGRAVISION:
                    screen_rainbow_effect_decodenagravision(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_RANDOMLINES:
                    screen_rainbow_effect_randomlines(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_SORTALIKE:
                    screen_rainbow_effect_sortalike(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_LOGOREBOUND:
                    screen_rainbow_effect_logorebound(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_SEA:
                    screen_rainbow_effect_sea(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_WAVES:
                    screen_rainbow_effect_improved_waves(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_SHEAR:
                    screen_rainbow_effect_shear(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_LENS:
                    screen_rainbow_effect_fisheye(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_RADAR:
                    screen_rainbow_effect_radar(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_MAGNETIC_FIELD:
                    screen_rainbow_effect_attraction(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_PIXELATE:
                    screen_rainbow_effect_pixelate(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_BLUR:
                    screen_rainbow_effect_blur(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_SHADERBORDER:
                    screen_rainbow_effect_shaderborder(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_LED:
                    screen_rainbow_effect_led(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_HSYNC_LOST:
                    screen_rainbow_effect_hsync_lost(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_VSYNC_LOST:
                    screen_rainbow_effect_vsync_lost(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_SCROLL_HORIZONTAL:
                    screen_rainbow_effect_scroll_horizontal(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_SCROLL_VERTICAL:
                    screen_rainbow_effect_scroll_vertical(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_FADEIN:
                    screen_rainbow_effect_fadeinout(origen,destino,ancho,alto,1);
                break;

                case SCREEN_EFFECT_TYPE_FADEOUT:
                    screen_rainbow_effect_fadeinout(origen,destino,ancho,alto,-1);
                break;

                case SCREEN_EFFECT_TYPE_FADEINOUT:
                    screen_rainbow_effect_fadeinout(origen,destino,ancho,alto,0);
                break;

                case SCREEN_EFFECT_TYPE_SCANLINES:
                    screen_rainbow_effect_scanlines(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_SEPIA:
                    screen_rainbow_effect_sepia(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_CONTRAST:
                    screen_rainbow_effect_contrast(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_BRIGHTNESS:
                    screen_rainbow_effect_brightness(origen,destino,ancho,alto);
                break;

                case SCREEN_EFFECT_TYPE_PERSISTENCE:
                    screen_rainbow_effect_persistence(origen,destino,ancho,alto);
                break;

            }

            aplicado_algo=1;
            if (origen!=inicial_origen) free(origen);
            origen=destino;
        }
    }


    //Si no se ha aplicado ningun efecto especial, tal cual copiar de origen a destino
    if (!aplicado_algo) {
        destino=screen_special_effects_alloc_buffer(ancho,alto);
        int tamanyo=ancho*alto*sizeof(z80_int);
        memcpy(destino,origen,tamanyo);
    }

    return destino;

}





void screen_scale_075_050_gigascreen_function(int ancho,int alto)
{

return;
                //solo asignar buffer la primera vez o si ha cambiado el tamanyo
                int asignar=0;

                //Si ha cambiado el tamanyo
                if (scalled_rainbow_ancho!=ancho || scalled_rainbow_alto!=alto) {
                        //Liberar si existia
                        screen_special_effects_free_buffers();

                        asignar=1;
                }

                //O si no hay buffer asignado
                if (new_scalled_rainbow_buffer_gigascren_one==NULL) asignar=1;

                if (asignar) {
                        debug_printf(VERBOSE_DEBUG,"Allocating scaled gigascreen rainbow buffers");
                        new_scalled_rainbow_buffer_gigascren_one=malloc(ancho*alto*2); //*2 por que son valores de 16 bits
                        new_scalled_rainbow_buffer_gigascren_two=malloc(ancho*alto*2); //*2 por que son valores de 16 bits

                        if (new_scalled_rainbow_buffer_gigascren_one==NULL || new_scalled_rainbow_buffer_gigascren_two==NULL) cpu_panic("Can not allocate scalled gigascreen rainbow buffers");

                        //Llenarlo de cero
                        int i;
                        for (i=0;i<ancho*alto;i++) {
                            new_scalled_rainbow_buffer_gigascren_one[i]=0;
                            new_scalled_rainbow_buffer_gigascren_two[i]=0;
                        }

                        scalled_rainbow_ancho=ancho;
                        scalled_rainbow_alto=alto;
                }

                //TODO rehacer esto
                //screen_scale_075_050_and_watermark_function(rainbow_buffer_one,new_scalled_rainbow_buffer_gigascren_one,ancho,alto);
                //screen_scale_075_050_and_watermark_function(rainbow_buffer_two,new_scalled_rainbow_buffer_gigascren_two,ancho,alto);
}


