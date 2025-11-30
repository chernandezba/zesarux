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
z80_bit vsync_generator_active={0};

int zx8081_video_electron_position_x_testados=0;
int zx8081_video_electron_position_x_testados_testados_antes=0;

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

z80_bit force_zx81_chr_128={0};


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




//A partir de que linea hay un timeout y se fuerza vsync
int timeout_linea_vsync;

//Duracion minima de vsync para que se tenga en cuenta
int minimo_duracion_vsync;






//longitud del pulso vsync en t-estados
int longitud_pulso_vsync=0;
//t-estados anterior de ejecutar un opcode para saber lo que durara el vsync total
int longitud_pulso_vsync_t_estados_antes=0;


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





void zx8081_reset_electron_line_by_vsync(void)
{

//debug_printf(VERBOSE_ERR,"Abrir menu");

	//printf ("vsync total de zx81 t_estados: %d\n",t_estados);

    //printf("Set t_scanline_draw to 0 on generar_zx8081_vsync\n");
	t_scanline_draw=0;
	t_scanline_draw_timeout=0;


    zx8081_video_electron_position_x_testados=0;


    //Cuadrar t_estados a cada linea multiple de 207
    //Esto sirve para tener una imagen estable en horizontal....
    //sino no habria manera posible de sincronizar la imagen en zx80 .. (en zx81 se ayuda de la nmi)
	//lo ideal seria tener un contador de tiempo para la ULA separado del de la cpu... pero para no complicarlo mas,
	//nos ayudamos del contador de tiempo de la cpu

    //Metodo 1 para que la imagen no se desplace continuamente
    //texto desfasado 1 scanline
    //int t_estados_en_linea=t_estados%screen_testados_linea;
    //t_estados -=t_estados_en_linea;

    //Metodo 2 para que la imagen no se desplace continuamente
    //Parece que se vea bien pero hace cosas raras:
    //Caracter "punto" al hacer un reset de zx81, arriba del todo
    //Texto tiembla en vertical con clocktest.p
    //int t_estados_en_linea=t_estados%screen_testados_linea;
    //int sumar=screen_testados_linea-t_estados_en_linea;
    //t_estados +=sumar;

    //Metodo 3 con ajuste de mitad
    //Metodo 3 con ajuste de mitad. Basic estable
    //Breakout va lento
    //doble breakout se ve desplazado
    //dstar tiembla al moverse
    //qs defenda 1 scanline desplazado
    //xtricator tiembla
    //int t_estados_en_linea=t_estados%screen_testados_linea;
    //if (t_estados_en_linea<screen_testados_linea/2) {
    //    t_estados -=t_estados_en_linea;
    //}
    //else {
    //    int sumar=screen_testados_linea-t_estados_en_linea;
    //    t_estados +=sumar;
    //}

    //Otros metodos mas esotéricos
    //t_estados=0;
    //t_estados +=417;


    //417 t_estados desfasado... o tambien se puede ver como 417-207-207=3 t-estados
    //con esto la imagen no se desplaza pero a cada pulsacion de tecla se desplaza
    //t_estados +=3;

}

void generar_zx8081_hsync(void)
{


    //printf("x a 0\n");
    zx8081_video_electron_position_x_testados=0;




    if (1/*video_zx8081_linecntr_enabled.v==1*/) video_zx8081_linecntr++;


    //siguiente linea


    t_scanline_draw++;

    if (t_scanline_draw==56) printf("scanlinedraw : %d\n",t_scanline_draw);


    t_scanline_draw_timeout++;

    //si han pasado muchas lineas, resetear
    if (t_scanline_draw_timeout>=timeout_linea_vsync) {
        printf ("Reset scanline por timeout. linea=%d\n",t_scanline_draw_timeout);
        //printf("vsync 2-\n");
        zx8081_reset_electron_line_by_vsync();
        //video_zx8081_linecntr_enabled.v=1;
    }


}

int zx8081_get_vsync_length(void)
{

    return longitud_pulso_vsync;

}

void zx8081_if_admited_vsync(void)
{

    if (vsync_generator_active.v==0) return;

    video_zx8081_linecntr=0;

    //Calcular cuanto ha tardado el vsync
    int longitud_pulso_vsync=zx8081_get_vsync_length();



        //printf ("escribe puerto. final vsync  t_estados=%d. diferencia: %d t_scanline_draw: %d t_scanline_draw_timeout: %d\n",t_estados,longitud_pulso_vsync,t_scanline_draw,t_scanline_draw_timeout);



		if (longitud_pulso_vsync >= minimo_duracion_vsync) {
			//if (t_scanline_draw_timeout>MINIMA_LINEA_ADMITIDO_VSYNC || t_scanline_draw_timeout<=3) {

			if (1/*t_scanline_draw_timeout>MINIMA_LINEA_ADMITIDO_VSYNC*/) {
				printf ("admitido pulso vsync en linea %3d testados_linea %3d t_estados %6d\n",t_scanline_draw_timeout,t_estados % screen_testados_linea,t_estados);

                if (!simulate_lost_vsync.v) {



					if (zx8081_detect_vsync_sound.v) {
						//printf ("vsync total de zx8081 t_estados: %d\n",t_estados);
						if (zx8081_detect_vsync_sound_counter>0) zx8081_detect_vsync_sound_counter--;

					}

                    //printf("vsync 1\n");
					zx8081_reset_electron_line_by_vsync();
					vsync_per_second++;
				}


			}

            else {
                printf ("no admitido final pulso vsync porque linea es inferior a 280 (%d)\n",t_scanline_draw_timeout);
            }
		}

		else {
			//printf ("no admitimos pulso vsync por duracion menor a esperado, duracion: %d esperado %d\n",longitud_pulso_vsync,minimo_duracion_vsync);
		}


}

void adjust_zx8081_electron_position(void)
{



    int delta=0;

    if (t_estados<zx8081_video_electron_position_x_testados_testados_antes) {
        //printf("Ha dado la vuelta---------\n");
        delta=(screen_testados_total-zx8081_video_electron_position_x_testados_testados_antes)+t_estados;
    }
    else {
        delta=t_estados-zx8081_video_electron_position_x_testados_testados_antes;
        //if (delta<0 || delta>23) printf("delta: %d\n",delta);

    }
    if (delta>100) {
        printf("delta %d\n",delta);
        //sleep(1);
    }

    zx8081_video_electron_position_x_testados +=delta;

    //printf("delta %d zx8081_video_electron_position_x_testados %d\n",delta,zx8081_video_electron_position_x_testados);

    zx8081_video_electron_position_x_testados_testados_antes=t_estados;


    if (zx8081_video_electron_position_x_testados>=screen_testados_linea) {

        //printf("Fin de linea en %d\n",zx8081_video_electron_position_x_testados);

        zx8081_video_electron_position_x_testados -=screen_testados_linea;

        if (hsync_generator_active.v/* && vsync_generator_active.v==0*/) {
            generar_zx8081_hsync();

            //La ULA genera un hsync exactamente cada 64 microsegundos, tanto en ZX80 como ZX81
            //Pero creo que si vsync no esta activo. si vsync activo, tiene preferencia vsync?
            //if (hsync_generator_active.v && vsync_generator_active.v==0) generar_zx8081_hsync();

            //Ademas en ZX81 genera una NMI cada 64 microsegundos
            if (MACHINE_IS_ZX81_TYPE) {
                if (nmi_generator_active.v==1) {
                    //printf("nmi en t_estados %d\n",t_estados);
                    generate_nmi();
                }
            }
        }


    }


    //Si ha pasado mucho rato sin hsync, forzarlo. Valor arbitrario 300
    //Esto sirve en los modos FAST y en SAVE/LOAD
    if (zx8081_video_electron_position_x_testados>300) generar_zx8081_hsync();


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


		//Si no esta el modo real zx8081, no hacer esto
		if (rainbow_enabled.v) {


			z80_byte sprite;
			int x;


            //poner caracter en pantalla de video highmem
            z80_bit caracter_inverse;
            z80_int direccion_sprite;


            if (caracter&128) {
                caracter_inverse.v=1;
                caracter=caracter&127;
            }
            else caracter_inverse.v=0;

            //printf ("fetch caracter: %d\n",caracter);
            int y=t_scanline_draw;

            //TODO
            y -=ZX8081_LINEAS_SUP_NO_USABLES;
            //para evitar las lineas superiores
            //TODO. cuadrar esto con valores de borde invisible superior

            //printf("store graphics to y: %d caracter: %d\n",y,caracter);

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


                x=(zx8081_video_electron_position_x_testados-12)*2;

                //printf ("direccion_sprite: %d\n",direccion_sprite);
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


                direccion_sprite=((reg_i&254)*256)+caracter*8+( (video_zx8081_linecntr) & 7);


                x=(zx8081_video_electron_position_x_testados-12)*2;
                //if (y==48) printf("x: %3d y: %3d zx8081_video_electron_position_x_testados %d\n",x,y,zx8081_video_electron_position_x_testados);

                //if (y==50) printf("0store graphics to y: %d x: %d sprite: %d\n",y,x,sprite);


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


            if (border_enabled.v==0) {
                y=y-screen_borde_superior;
                x=x-screen_total_borde_izquierdo;
            }


            int totalancho=get_total_ancho_rainbow();



            if (y>=0 && y<get_total_alto_rainbow() ) {

                //printf("x: %d\n",x);

                if (x>=0 && x<totalancho )  {

                    //si linea no coincide con entrelazado, volvemos
                    if (if_store_scanline_interlace(y) ) {
                        //if (y==48) printf("store graphics to y: %d x: %d sprite: %d\n",y,x,sprite);
                        screen_store_scanline_char_zx8081(x,y,sprite,caracter,caracter_inverse.v);
                    }


                }
                else {
                    printf("x fuera de rango: %d\n",x);
                }

            }


		}


		//Si no modo real video
		else {
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


int zx8081_read_port_a0_low(z80_byte puerto_h)
{
    z80_byte valor;

    if (vsync_generator_active.v==0) {
        longitud_pulso_vsync=0;
        longitud_pulso_vsync_t_estados_antes=t_estados;
        vsync_generator_active.v=1;
        printf("vsync generator on  en t_scanline_draw=%d\n",t_scanline_draw);
    }


    //video_zx8081_linecntr=0;
    video_zx8081_ula_video_output=255;

    if (nmi_generator_active.v==0 && hsync_generator_active.v) {
        hsync_generator_active.v=0;
        printf("hsync generator off en t_scanline_draw=%d\n",t_scanline_draw);
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

    //valor &= (255-128);


    //decimos que hay pulso de carga, alternado
    //if (reg_b!=0) temp_cinta_zx81=128;
    //else temp_cinta_zx81=0;
    //valor |=temp_cinta_zx81;

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
        printf("hsync generator on  en t_scanline_draw=%d\n",t_scanline_draw);
    }

    if (vsync_generator_active.v) {
        vsync_generator_active.v=0;
        printf("vsync generator off en t_scanline_draw=%d\n",t_scanline_draw);
    }

    //no estoy seguro de esto
    //video_zx8081_linecntr=0;

 	video_zx8081_ula_video_output=0;


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