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
z80_byte   caracteres_zx80_no_artistic[64]=" \"???????????$:?()-+*/=><;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//Un poco mas artistico
z80_byte   caracteres_zx80[64]=" \"|v''../#_^f$:?()-+*/=><;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


//Real
z80_byte   caracteres_zx81_no_artistic[64]=" ??????????\"?$:?()><=+-*/;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//Un poco mas artistico
z80_byte   caracteres_zx81[64]=" ''^.|/p#_^\"f$:?()><=+-*/;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


//Caracteres sin interrogantes ni nada, para funciones ocr a speech
z80_byte   caracteres_zx80_solo_letras[64]=
			   " \"           $:?()-+*/=><;,."
                           "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

z80_byte   caracteres_zx81_solo_letras[64]=
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

//indica si se simula la pantalla negra del modo fast
z80_bit video_fast_mode_emulation;

//indica si el siguiente frame estara en negro. es un contador con un limite
z80_byte video_fast_mode_next_frame_black;

//Esto solo sirve para mostrar en menu debug i/o ports
z80_byte zx8081_last_port_write_value;



z80_int ramtop_zx8081;

//forzar habilitar sonido de vsync/cinta en zx80/81
z80_bit zx8081_vsync_sound;


//deteccion de perdida de vsync y por tanto se oye sonido
z80_bit zx8081_detect_vsync_sound={0};

//en cuanto un frame no tiene vsync, incrementamos aqui, y si vale>0, se activa vsync sound
//cuando se recibe un vsync completo de frame, se decrementa esto. cuando llega a 0 , se desactiva vsync sound
//maximo valor de esto: ZX8081_DETECT_VSYNC_SOUND_COUNTER_MAX
int zx8081_detect_vsync_sound_counter=ZX8081_DETECT_VSYNC_SOUND_COUNTER_MAX;




z80_bit autodetect_wrx;

//chroma enabled
z80_bit chroma81={0};

z80_bit autodetect_chroma81={1};

//chroma port
z80_byte chroma81_port_7FEF;


//modo real(beta) de video para zx80/81
//z80_bit beta_zx8081_video;

//opcion de mostrar vsync en pantalla (franjas de carga / grabacion)
//z80_bit video_zx8081_shows_vsync_on_display;

//Opcion para simular perdida de vsync
z80_bit simulate_lost_vsync;



//Esto indica que se ha llegado a final de linea en rainbow, y siguiente escritura de "pantalla" debe tener lnctr=0
//int temp_final_linea=1;

//8k de RAM en 8192-16383
z80_bit ram_in_8192;

//16k de RAM en 49152-65535
z80_bit ram_in_49152;

//16k de RAM en 32767-49152
z80_bit ram_in_32768;

//WRX hi-res mode
z80_bit wrx_present;

//z80_bit wrx_mueve_primera_columna;

//HRG hi-res mode
//z80_bit hrg_enabled;

//offset ajustable para t_estados a final de linea con wrx. normalmente 8
//int offset_zx8081_t_estados;

//offset ajustable para coordenada x con wrx. normalmente 0
int offset_zx8081_t_coordx;


//A partir de que linea hay un timeout y se fuerza vsync
int timeout_linea_vsync;

//Duracion minima de vsync para que se tenga en cuenta
int minimo_duracion_vsync;


z80_bit video_zx8081_lnctr_adjust;

//Ajuste solo para manic miner y para tetrishr. Desplazar coordenada x que normalmente solo se hace en WRX
//z80_bit manic_miner_game;

z80_bit video_zx8081_estabilizador_imagen;

//int video_zx8081_decremento_x_cuando_mayor;

//Inicio del pulso vsync. Para saber si al final del pulso se admite como vsync o no
int inicio_pulso_vsync_t_estados;


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
		screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling WRX video mode");
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

	if (MACHINE_IS_ZX80) return;

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

  if (MACHINE_IS_ZX80) {
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

  if (MACHINE_IS_ZX80) {
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



//se usa solo para el estabilizador de imagen
int video_zx8081_caracter_en_linea_actual=0;

z80_bit video_zx8081_estabilizador_imagen;


void generar_zx8081_vsync(void) {



	//printf ("vsync total de zx81 t_estados: %d\n",t_estados);


	t_scanline_draw=0;
	t_scanline_draw_timeout=0;


        //Cuadrar t_estados a cada linea multiple de 207
        //Esto sirve para tener una imagen estable en horizontal....
        //sino no habria manera posible de sincronizar la imagen en zx80 .. (en zx81 se ayuda de la nmi)
	//lo ideal seria tener un contador de tiempo para la ULA separado del de la cpu... pero para no complicarlo mas,
	//nos ayudamos del contador de tiempo de la cpu

        int t_estados_en_linea=t_estados%screen_testados_linea;
        t_estados -=t_estados_en_linea;




}

void generar_zx8081_horiz_sync(void) {


                        if (video_zx8081_linecntr_enabled.v==1)
                        	video_zx8081_linecntr++;


                        //Cuadrar t_estados a cada linea multiple de 207
                        //Esto sirve para tener una imagen estable en horizontal.... sino no hay manera

                        if (video_zx8081_estabilizador_imagen.v) {
                                int t_estados_en_linea=t_estados%screen_testados_linea;
                                t_estados -=t_estados_en_linea;

				//printf ("t_estados sobran: %d\n",t_estados_en_linea);
                        }


                        //siguiente linea

                        video_zx8081_caracter_en_linea_actual=0;

                        t_scanline_draw++;


                        t_scanline_draw_timeout++;

                        //si han pasado muchas lineas, resetear
                        if (t_scanline_draw_timeout>=timeout_linea_vsync) {
                                //printf ("Reset scanline por timeout. linea=%d\n",t_scanline_draw_timeout);
                                generar_zx8081_vsync();
                                video_zx8081_linecntr_enabled.v=1;
                        }


                       //Generar NMI si conviene
                       if (MACHINE_IS_ZX81) {
                                 if (nmi_generator_active.v==1) {
					generate_nmi();
                                 }
                       }

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
		if (chroma81.v==0) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling Chroma81 video mode");
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


void chroma81_return_mode1_colour(z80_int dir,z80_byte *colortinta,z80_byte *colorpapel)
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
