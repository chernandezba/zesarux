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
#include <string.h>

#include "zx8081.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "audio.h"
#include "core_zx8081.h"
#include "operaciones.h"
#include "zxvision.h"
#include "ula.h"
#include "zxpand.h"
#include "tape.h"
#include "tv.h"

//http://nocash.emubase.de/zxdocs.htm#zx80zx81videointerruptsintsandnmis

//http://www.user.dccnet.com/wrigter/index_files/ZX%20Video%20Tutorial.htm

/*

Info general sobre ZX80 y ZX81

Dibujan la pantalla mediante rutinas de software. En el emulador no se hace asi, el pintado de pantalla es tarea del driver de video,
que lee los caracteres de pantalla del zx80/81 y los dibuja. Por tanto, no se soportan modos de video de alta resolucion

Diferencias entre ZX80 y ZX81:
-ZX81 tiene un generador de NMI, que genera NMI cuando está activo cada 64 microsegundos (cada final de frame de pantalla).Podria ignorar el generador, pero sirve para bajar la velocidad real del zx81 (en modo slow). Tambien he visto que el mazogs, cuando genera el laberinto, si no hay nmis no acaba nunca
-ZX80 siempre esta en modo fast, y por tanto presenta parpadeo?¿

*/

/*

Teclado:

Igual que spectrum excepto que no hay symbol shift. este esta reemplazado por "."
The ZX81/ZX80 Keyboard Matrix

  Port____Line____Bit__0____1____2____3____4__
  FEFEh  0  (A8)     SHIFT  Z    X    C    V
  FDFEh  1  (A9)       A    S    D    F    G
  FBFEh  2  (A10)      Q    W    E    R    T
  F7FEh  3  (A11)      1    2    3    4    5
  EFFEh  4  (A12)      0    9    8    7    6
  DFFEh  5  (A13)      P    O    I    U    Y
  BFFEh  6  (A14)    ENTER  L    K    J    H
  7FFEh  7  (A15)     SPC   .    M    N    B


*/


//Real
z80_byte   caracteres_zx80_no_artistic[]=" \"???????????$:?()-+*/=><;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//Un poco mas artistico
z80_byte   caracteres_zx80[]=" \"|v''../#_^f$:?()-+*/=><;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


//Real
z80_byte   caracteres_zx81_no_artistic[]=" ??????????\"?$:?()><=+-*/;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//Un poco mas artistico
z80_byte   caracteres_zx81[]=" ''^.|/p#_^\"f$:?()><=+-*/;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


//Caracteres sin interrogantes ni nada, para funciones ocr a speech
z80_byte   caracteres_zx80_solo_letras[]=
               " \"           $:?()-+*/=><;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

z80_byte   caracteres_zx81_solo_letras[]=
              //01234567890 12345678901234567
               "           \" $:?()><=+-*/;,."
              //890123456789012345678901234567890123
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//Pasar de ascii a zx81. Empezando por el 32

z80_byte caracteres_ascii_zx81[]={
//  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9
0, 15,11,15,13,15,15,15,16,17,23,21,26,22,27,24,28,29,30,31,32,33,34,35,36,37,

// :   ;  <  =  >  ?  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
   14,25,19,20,18,15,15,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,

//     [  \  ]  ^  _  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
      15,15,15,15,15,15,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,

// {  |  }  ~  127
   15,15,15,15,15
};


//Pasar de ascii a zx80. Empezando por el 32

z80_byte caracteres_ascii_zx80[]={
//  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9
0, 15,11,15,13,15,15,15,16,17,20,19,26,18,27,21,28,29,30,31,32,33,34,35,36,37,

// :   ;  <  =  >  ?  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
   14,25,24,22,23,15,15,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,

//     [  \  ]  ^  _  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
      15,15,15,15,15,15,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,

// {  |  }  ~  127
   15,15,15,15,15
};

/*
Diferencias entre set de caracteres del zx81 y zx80
zx80 tiene en la posicion 1 , el "
Despues de ?(), en posicion 18:

       18 19 20 21 22 23 24
-zx80: -  +  *  /  =  >  <

-zx81: >  <  =  +  -  *  /
*/


z80_bit nmi_generator_active;
z80_bit hsync_generator_active;
z80_bit zx8081_vsync_generator={0};

//Tiempo total de hsync:
//12 microsegundos:
//1.65 front porch
//4.7 hsync pulse
//5.7 back porch
//TODO: esto deberia ser 39 t-estados (12 microsec)  pero entonces algunos juegos están muy a la izquierda
//tiempo en t-estados. Para calcularla: (207/64)*valor en microsegundos
int hsync_total_duration=39;

int hsync_duration_counter=0;

//Para lanzar los hsync del hsync generator
int ula_zx80_position_x_testados=0;

//indica si se simula la pantalla negra del modo fast
z80_bit video_fast_mode_emulation;

//indica si el siguiente frame estara en negro. es un contador con un limite
z80_byte video_fast_mode_next_frame_black;

//Esto solo sirve para mostrar en menu debug i/o ports
z80_byte zx8081_last_port_write_value;

//Para que al hacer hotswap de ZX80 a ZX81, el ZX81 resultante no tenga nmi generator
//Eso permite que juegos como Breakout y otros se pueda hacer hotswap a ZX81
z80_bit hotswapped_zx80_to_zx81={0};

z80_int ramtop_zx8081;

//forzar habilitar sonido de vsync/cinta en zx80/81
z80_bit zx8081_vsync_sound;


//deteccion de perdida de vsync y por tanto se oye sonido
z80_bit zx8081_detect_vsync_sound={0};

//en cuanto un frame no tiene vsync, incrementamos aqui, y si vale>0, se activa vsync sound
//cuando se recibe un vsync completo de frame, se decrementa esto. cuando llega a 0 , se desactiva vsync sound
//maximo valor de esto: ZX8081_DETECT_VSYNC_SOUND_COUNTER_MAX
int zx8081_detect_vsync_sound_counter=ZX8081_DETECT_VSYNC_SOUND_COUNTER_MAX;

z80_bit force_zx81_chr_128={0};


z80_bit autodetect_wrx;

//chroma enabled
z80_bit chroma81={0};

z80_bit autodetect_chroma81={1};

//chroma port
z80_byte chroma81_port_7FEF;




//Opcion para simular perdida de vsync
z80_bit simulate_lost_vsync;




//8k de RAM en 8192-16383
z80_bit ram_in_8192;

//16k de RAM en 49152-65535
z80_bit ram_in_49152;

//16k de RAM en 32767-49152
z80_bit ram_in_32768;

//WRX hi-res mode
z80_bit wrx_present;






int vsync_per_second;
int last_vsync_per_second;

z80_byte ascii_to_zx81(z80_byte c)
{
    //?
    if (c<32 || c>127) return 15;

    else return caracteres_ascii_zx81[c-32];
}

z80_byte ascii_to_zx80(z80_byte c)
{
    //?
    if (c<32 || c>127) return 15;

    else return caracteres_ascii_zx80[c-32];
}

//z80_bit ejecutado_zona_pantalla;


//Activar wrx y offset de t_estados
void enable_wrx(void)
{
  if (!MACHINE_IS_ZX8081) {
        debug_printf (VERBOSE_INFO,"ZXpand can only be enabled on ZX80/81");
        return;
    }

    if (wrx_present.v==0) {
        screen_print_splash_text_center_no_if_previous(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling WRX video mode");
        debug_printf (VERBOSE_INFO,"Enabling WRX video mode");
    }

    wrx_present.v=1;
}

//Desactivar wrx
void disable_wrx(void)
{
    if (wrx_present.v==1) {
            wrx_present.v=0;
        debug_printf (VERBOSE_INFO,"Disabling WRX video mode");
    }
}




void enable_ram_in_32768(void)
{
    ram_in_32768.v=1;

    //decimos la RAM para BASIC
    //debug_printf (VERBOSE_INFO,"Setting BASIC RAMTOP to 49152");
    //memoria_spectrum[16389]=192;
}

void enable_ram_in_49152(void)
{

    //si habilitamos esta, tambien la de 32768
    ram_in_32768.v=1;
    enable_ram_in_32768();
        ram_in_49152.v=1;

        //decimos la RAM para BASIC
    //debug_printf (VERBOSE_INFO,"Setting BASIC RAMTOP to 65535");
        //memoria_spectrum[16389]=255;
}

z80_int zx8081_get_standard_ram(void)
{
  return (ramtop_zx8081-16383)/1024;
}

z80_int zx8081_get_total_ram_with_rampacks(void)
{
    z80_int total_ram=zx8081_get_standard_ram();

    if (ram_in_8192.v) total_ram +=8;
    if (ram_in_49152.v==1) total_ram +=16;
    if (ram_in_32768.v==1) total_ram +=16;

    return total_ram;

}

z80_int get_ramtop_with_rampacks(void)
{
    //retorna la ramtop teniendo en cuenta los rampack
    if (ram_in_49152.v==1) return 65535;
    if (ram_in_32768.v==1) return 49151;
    return ramtop_zx8081;
}


//ajusta variable ramtop del sistema si hay algun pack activo
//esto se hace al cargar programas zx81 solo
void set_ramtop_with_rampacks(void)
{

    z80_int r;

    if (MACHINE_IS_ZX80_TYPE) return;

    //Solo si hay expansiones de memoria
    if (ram_in_32768.v==1 || ram_in_49152.v==1) {
        r=get_ramtop_with_rampacks();
        r++;
        if (r==0) r=65535;

        debug_printf (VERBOSE_INFO,"Setting BASIC RAMTOP to %d",r);
        memoria_spectrum[16389]=value_16_to_8h(r);
        memoria_spectrum[16388]=value_16_to_8l(r);
    }
}


z80_byte da_codigo81(z80_byte codigo,z80_bit *inverse)
{

  if (codigo>127) {
        inverse->v=1;
        codigo-=128;
  }
  else inverse->v=0;

  if (MACHINE_IS_ZX80_TYPE) {
    //zx80
    if (texto_artistico.v==1) return (codigo<64 ? caracteres_zx80[codigo] : '~');
    else return (codigo<64 ? caracteres_zx80_no_artistic[codigo] : '~');
    }
  else {
    //zx81
    if (texto_artistico.v==1) return (codigo<64 ? caracteres_zx81[codigo] : '~');
    else return (codigo<64 ? caracteres_zx81_no_artistic[codigo] : '~');
  }

}



z80_byte da_codigo81_solo_letras(z80_byte codigo,z80_bit *inverse)
{

  if (codigo>127) {
        inverse->v=1;
        codigo-=128;
  }
  else inverse->v=0;

  if (MACHINE_IS_ZX80_TYPE) {
        //zx80
    return (codigo<64 ? caracteres_zx80_solo_letras[codigo] : ' ');
        }
  else {
        //zx81
    return (codigo<64 ? caracteres_zx81_solo_letras[codigo] : ' ');
  }

}

z80_byte da_codigo_zx80_no_artistic(z80_byte codigo)
{
    return caracteres_zx80_no_artistic[codigo];
}

z80_byte da_codigo_zx81_no_artistic(z80_byte codigo)
{
        return caracteres_zx81_no_artistic[codigo];
}



int da_amplitud_speaker_zx8081(void)
{
                                if (bit_salida_sonido_zx8081.v) return amplitud_speaker_actual_zx8081;
                                else return -amplitud_speaker_actual_zx8081;
}


//Establece tamanyo ram (sin contar ram packs) de zx80/81. Valor entre 1 y 16
void set_zx8081_ramtop(z80_byte valor)
{
    if (valor<1 || valor>16) {
        cpu_panic("Cannot set ZX80/81 RAM");
    }

    ramtop_zx8081=16383+1024*valor;
}


void enable_chroma81(void)
{

    if (!MACHINE_IS_ZX8081) return;


    //en drivers stdout o curses, no habilitar colores, aunque dejamos si que dejamos habilitar las paginas de ram y el rainbow
    if (!strcmp(scr_new_driver_name,"curses") || !strcmp(scr_new_driver_name,"stdout")) {
        debug_printf (VERBOSE_WARN,"Chroma 81 is not supported on curses or stdout drivers");
    }
    else {
        if (chroma81.v==0) screen_print_splash_text_center_no_if_previous(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling Chroma81 video mode");
        chroma81.v=1;
    }


        ram_in_8192.v=1;
        enable_ram_in_49152();
    enable_rainbow();
}

void disable_chroma81(void)
{
    chroma81.v=0;
}


void chroma81_return_mode1_colour(z80_int dir,int *colortinta,int *colorpapel)
{
                        //1 attribute file
            z80_byte c=peek_byte_no_time(dir|0x8000);
                        *colortinta=c&15;
                        *colorpapel=(c>>4)&15;
}

int color_es_chroma(void)
{
    return (chroma81.v && (chroma81_port_7FEF & 32) ? 1 : 0);
}





z80_byte zx80801_last_sprite_video=0;
int zx80801_last_sprite_video_tinta=0;
int zx80801_last_sprite_video_papel=0;


void screen_store_scanline_char_zx8081(z80_byte byte_leido,z80_byte caracter,int inverse)
{

    zx80801_last_sprite_video_tinta=0;
    zx80801_last_sprite_video_papel=15;



    //Si modo chroma81 y lo ha activado

    if (color_es_chroma() ) {
        //ver modo
        if ((chroma81_port_7FEF&16)!=0) {
            //1 attribute file
            chroma81_return_mode1_colour(reg_pc,&zx80801_last_sprite_video_tinta,&zx80801_last_sprite_video_papel);
            //printf ("modo 1\n");
        }
        else {
            //0 character code
            //tablas colores van primero para los 64 normales y luego para los 64 inversos
            if (inverse) {
                caracter +=64;
            }

            z80_int d=caracter*8+0xc000;
            d=d+(video_zx8081_lcntr&7);
            z80_byte leido=peek_byte_no_time(d);
            zx80801_last_sprite_video_tinta=leido&15;
            zx80801_last_sprite_video_papel=(leido>>4)&15;

            //printf ("modo 0\n");

        }
    }


    zx80801_last_sprite_video=byte_leido;
    return;

}

//Tratamiento de los gráficos zx81 al hacer fetch
z80_byte fetch_opcode_zx81_graphics(void)
{

    z80_byte op;

    op=peek_byte_zx80_no_time(reg_pc&0x7fff);

    if( (reg_pc&0x8000) ) {
        //se esta ejecutando la zona de pantalla




        z80_byte caracter;

        if (op&64 || z80_halt_signal.v) {
            caracter=0;
            //Otros caracteres no validos (y el HALT) generan video display a 0

            return op;
        }

        else {
            caracter=op;
            op=0;
        }


        z80_byte sprite;



        //poner caracter en pantalla de video highmem
        z80_bit caracter_inverse;
        z80_int direccion_sprite;


        if (caracter&128) {
            caracter_inverse.v=1;
            caracter=caracter&127;
        }
        else caracter_inverse.v=0;




        //Posible modo wrx, y excluir valores de I usados en chr$128 y udg
        if (reg_i>=33 && reg_i!=0x31 && reg_i!=0x30 && wrx_present.v==0 && autodetect_wrx.v) {
            //posible modo wrx
            debug_printf(VERBOSE_INFO,"Autoenabling wrx so the program seems to need it (I register>32). Also enable 8K RAM in 2000H");
            enable_wrx();

            //algunos juegos requieren que este ram pack este presente antes de activar wrx... sino no funcionara
            //pero igualmente, por si acaso, lo activamos aqui
            ram_in_8192.v=1;

        }


        //Modos WRX
        if (wrx_present.v==1 && reg_i>=32 ) {

            //printf ("reg_i en zona WRX\n");

            direccion_sprite=(reg_i<<8) | (reg_r_bit7 & 128) | ((reg_r) & 127);

            sprite=memoria_spectrum[direccion_sprite];

            if (caracter_inverse.v) sprite=sprite^255;

        }


        else {

            //chr$128
            if (reg_i==0x31) {
                if (caracter_inverse.v) {
                    //El bit de inverse es para acceder a los 64 caracteres siguientes
                    caracter=caracter | 64;
                    //Pero sigue indicando inverse
                    //caracter_inverse.v=0;

                }
            }

            //Otros interfaces que tambien hacen 128 caracteres aunque no siguen la norma del registro I,
            //como el SD81 Booster de Alejandro Valero
            if (caracter_inverse.v && force_zx81_chr_128.v) {
                caracter=caracter | 64;
            }


            direccion_sprite=((reg_i&254)*256)+caracter*8+( (video_zx8081_lcntr) & 7);


            //Obteniendo tipo de letra de rom de zxpand en el caso del zx80
            if (zxpand_enabled.v && MACHINE_IS_ZX80_TYPE && direccion_sprite<8192) {
                sprite=zxpand_memory_pointer[direccion_sprite];
            }

            else {
                sprite=memoria_spectrum[direccion_sprite];
            }

            //aunque este en modo zxpand, la tabla de caracteres siempre sale de la rom principal
            //por eso hacemos sprite=memoria_spectrum[direccion_sprite]; y zxpand rom esta en otro puntero de memoria
            //sprite=peek_byte_zx80_no_time(direccion_sprite);

            if (caracter_inverse.v) sprite=sprite^255;

        }


        screen_store_scanline_char_zx8081(sprite,caracter,caracter_inverse.v);


        //Si no modo real video
        if (rainbow_enabled.v==0) {
            //Intentar autodetectar si hay que activar realvideo
            if (autodetect_rainbow.v) {
                if (MACHINE_IS_ZX80_TYPE) {
                    //ZX80
                    if (reg_i!=0x0e) {
                        debug_printf(VERBOSE_INFO,"Autoenabling realvideo so the program seems to need it (I register on ZX80 != 0x0e)");
                        enable_rainbow();
                    }
                }
                if (MACHINE_IS_ZX81_TYPE) {
                    //ZX81
                    if (reg_i!=0x1e) {
                        debug_printf(VERBOSE_INFO,"Autoenabling realvideo so the program seems to need it (I register on ZX81 != 0x1e)");
                        enable_rainbow();
                    }
                }
            }

        }
    }

    return op;

}

int ula_zx81_time_event_t_estados=0;


int pending_disable_hsync=0;

void generar_zx80_hsync(void)
{

    tv_enable_hsync();
    pending_disable_hsync=1;
    hsync_duration_counter=0;


    //Necesario poner a 0 para imagen correcta en breakout y space invaders 1k y 3k se ven mal la primera linea de sprites de cada caracter
    //ula_zx80_position_x_testados=0;

    video_zx8081_lcntr++;


}


void generar_zx81_hsync(void)
{

    tv_enable_hsync();
    pending_disable_hsync=1;
    hsync_duration_counter=0;


    //ula_zx81_time_event_t_estados=0;

    video_zx8081_lcntr++;


}




void ula_zx80_time_event(int delta)
{

    int i;

    for (i=0;i<delta;i++) {

        ula_zx80_position_x_testados +=1;

        //printf("delta %d ula_zx80_position_x_testados %d\n",delta,ula_zx80_position_x_testados);


        if (ula_zx80_position_x_testados>=screen_testados_linea-hsync_total_duration) {
            //printf("Tiempo previo hsync en t_estados %6d (%d) y: %4d\n",t_estados,t_estados % screen_testados_linea,tv_get_y());
        }


        if (ula_zx80_position_x_testados>=screen_testados_linea) {

            //if (hsync_generator_active.v  && vsync_generator_active.v==0) printf("Fin de linea en %d\n",ula_zx80_position_x_testados);

            ula_zx80_position_x_testados -=screen_testados_linea;

            //Lanzamos hsync
            //NO. Esto se hace desde ACK de la interrupción

            if (0) {
            if (hsync_generator_active.v) {
                //printf("generate hsync en t_estados %6d (%d) ula_zx80_position_x_testados %3d delta %3d y: %4d\n",
                //    t_estados,t_estados % screen_testados_linea,ula_zx80_position_x_testados,1,tv_get_y());
                generar_zx80_hsync();

                //Y desactivamos hsync al momento
                pending_disable_hsync=0;
                tv_disable_hsync();
            }
            }



        }

        if (zx8081_vsync_generator.v) video_zx8081_lcntr=0;


        tv_time_event(1);


    }


}


void ula_zx81_time_event(int delta)
{

    int i;

    for (i=0;i<delta;i++) {

        ula_zx81_time_event_t_estados+=1;




        if (ula_zx81_time_event_t_estados>=screen_testados_linea) {
            ula_zx81_time_event_t_estados -=screen_testados_linea;


            if (nmi_generator_active.v) {
                //printf("Generate nmi\n");
                generate_nmi();
            }

            /*
            if (hsync_generator_active.v && nmi_generator_active.v) {
                generar_zx81_hsync();

                //Y desactivamos hsync al momento
                pending_disable_hsync=0;
                tv_disable_hsync();
            }
            */
            /*
            Al generar nmi,
            /Wait is also pulled low, to ensure the Z80 is in the correct T-State when the NMI is serviced.
            This is gated by the /Halt signal, which would always be high as that is normally triggered by processing
            the End of Line character, which will not happen on the non-visible lines.
            ->Esto se resuelve haciendo que el HALT en Z80 tarde 1 t-estado
            */

        }


        if (zx8081_vsync_generator.v) {
            //printf("Set LCNTR to zero\n");
            //printf("Set LCNTR to zero on time_event tv_y=%d\n",tv_get_y() );
            video_zx8081_lcntr=0;
        }

        tv_time_event(1);

    }


}

void zx81_enable_nmi_generator(void)
{
    if (hotswapped_zx80_to_zx81.v) return;

    //printf("   nmi on   en t_estados %6d y: %4d\n",t_estados,tv_get_y());
    nmi_generator_active.v=1;




}

void zx81_disable_nmi_generator(void)
{
    //printf("   nmi off  en t_estados %6d y: %4d\n",t_estados,tv_get_y());
    nmi_generator_active.v=0;



}



int zx8081_read_port_a0_low(z80_byte puerto_h)
{
    z80_byte valor;

    //Solo se lanzan vsync cuando el nmi generator está off. Si no fuese así, se pretendería enviar vsync
    //en algunos juegos cuando se lee el teclado y el electrón está en la zona del borde superior o inferior
    if (nmi_generator_active.v==0) {
        tv_enable_vsync();
        zx8081_vsync_generator.v=1;
        //printf("Set vsync generator tv_y=%d\n",tv_get_y() );
    }


    //video_zx8081_lcntr=0;


    //Para debug saber donde hay posible inicio vsync
    //O es inicio vsync o simplemente se lee teclado
    //Valor un tanto random que genera punteado en pantalla
    //video_zx8081_ula_video_output=0x11;

    if (nmi_generator_active.v==0 && hsync_generator_active.v) {
        hsync_generator_active.v=0;
        //printf("hsync generator off en t_scanline_draw=%d t_estados: %d\n",t_scanline_draw,t_estados);
    }


    //printf("Disabling the HSYNC generator t_scanline_draw=%d\n",t_scanline_draw);

    //printf("Disabling the HSYNC generator on t-state %d t-states %d scanline_draw %d contador_segundo %d\n",
    //    t_estados % screen_testados_linea,t_estados,t_scanline_draw,contador_segundo);

    modificado_border.v=1;


    //y ponemos a low la salida del altavoz
    bit_salida_sonido_zx8081.v=0;

    set_value_beeper_on_array(da_amplitud_speaker_zx8081() );


    if (zx8081_vsync_sound.v==1) {
        //solo resetea contador de silencio cuando esta activo el vsync sound - beeper
        reset_beeper_silence_detection_counter();
    }

    //Teclado

    //probamos a enviar lo mismo que con teclado de spectrum
    valor=lee_puerto_teclado(puerto_h);

/*
Bit  Expl.
0-4  Keyboard column bits (0=Pressed)
5    Not used             (1)
6    Display Refresh Rate (0=60Hz, 1=50Hz)
7    Cassette input       (0=Normal, 1=Pulse)
*/
    //decimos que no hay pulso de carga. 50 Hz refresco

    valor = (valor & 31) | (32+64);


    //printf ("valor: %d\n",valor);

    int leer_cinta_real=0;

    if (realtape_inserted.v && realtape_playing.v) leer_cinta_real=1;

    if (audio_can_record_input()) {
        if (audio_is_recording_input) {
            leer_cinta_real=1;
        }
    }

    if (leer_cinta_real) {
        if (realtape_get_current_bit_playing()) {
                        valor=valor|128;
                        //printf ("1 ");
                }
                else {
                        valor=(valor & (255-128));
                        //printf ("0 ");
                }
    }


    return valor;



}

void zx8081_out_any_port_video_stuff(void)
{

    //printf ("Sending vsync with hsync_generator_active : %d video_zx8081_ula_video_output: %d\n",hsync_generator_active.v,video_zx8081_ula_video_output);


    //printf("Enabling the HSYNC generator t_scanline_draw=%d\n",t_scanline_draw);

    if (hsync_generator_active.v==0) {
        hsync_generator_active.v=1;
    }


    if (MACHINE_IS_ZX80_TYPE) {
        ula_zx80_position_x_testados=0;
    }

    //Para que la imagen esté centrada. En slow:
    //ula_zx81_time_event_t_estados=screen_total_borde_izquierdo;

    //En fast:
    //Deberia ser este el valor que se asigna
    //ula_zx81_time_event_t_estados=0;

    //temporal. valor algo arbitrario
    //ula_zx81_time_event_t_estados=screen_total_borde_izquierdo/2;

    ula_zx81_time_event_t_estados=0;


    tv_disable_vsync();
    zx8081_vsync_generator.v=0;



    modificado_border.v=1;


    //reseteamos contador de deteccion de modo fast-pantalla negra. Para modo no-realvideo
    video_fast_mode_next_frame_black=0;


    //y ponemos a high la salida del altavoz
    bit_salida_sonido_zx8081.v=1;

    set_value_beeper_on_array(da_amplitud_speaker_zx8081() );


    if (zx8081_vsync_sound.v==1) {
        //solo resetea contador de silencio cuando esta activo el vsync sound - beeper
        reset_beeper_silence_detection_counter();
    }


}