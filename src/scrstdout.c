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

#include <unistd.h>
//#include <termios.h>

#include <fcntl.h>
//#include <sys/ioctl.h>
#include <sys/select.h>
#include <errno.h>

#include <string.h>

#include <sys/wait.h> 


#include "scrstdout.h"
#include "debug.h"
#include "screen.h"
#include "mem128.h"
#include "zx8081.h"
#include "operaciones.h"
#include "cpu.h"
#include "utils.h"
#include "menu.h"
#include "joystick.h"
#include "ula.h"
#include "disassemble.h"
#include "z88.h"
#include "timer.h"
#include "chardetect.h"
#include "textspeech.h"
#include "tsconf.h"
#include "settings.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "svi.h"

void scrstdout_establece_tablas_teclado(int c);
void scrstdout_reset_teclas(void);
void scrstdout_repinta_pantalla(void);
void scrstdout_menu_kbhit();

//A 1 si se ha recibido el caracter "1" de escape y se espera siguiente valor para saber longitud
//A 2 hay que ignorar longitud de caracter escape
//int z88_escape_caracter=0;
//int z88_escape_caracter_longitud=0;


//Si se envian a speech tambien messages de debug
z80_bit scrstdout_also_send_speech_debug_messages={0};

//int stdout_x_position=0;

#define STDOUT_IZQ_BORDER 4
#define STDOUT_TOP_BORDER 4

int tecla_activa=0;
int anterior_tecla=0;








//buffer de comando introducido
char buffer_tecla_comando[256];

int pos_buffer_tecla_comando=0;



	

//Usado en funciones de print del menu, para que hagan speech y pausa
void scrstdout_menu_print_speech(char *texto_orig)
{

	//Para que el texto que se ha enviado a consola se fuerce a mostrar con fflush antes de enviar a speech	
	fflush(stdout);

	if (textspeech_filter_program==NULL) return;
	if (textspeech_also_send_menu.v==0) return;

	//pasar filtros de conversion de corchetes de menu [ ] [X] en "Disabled" o "Enabled"
	char texto[MAX_BUFFER_SPEECH+1];
	menu_textspeech_filter_corchetes(texto_orig,texto);	

	textspeech_print_speech(texto);
	
	//scrtextspeech_filter_run_pending();
	do {
		if (textspeech_finalizado_hijo_speech() ) scrtextspeech_filter_run_pending();
		usleep(10000);
		scrstdout_menu_kbhit();	

	} while (!textspeech_finalizado_hijo_speech() || fifo_buffer_speech_size);
	//Estar aqui mientras se este reproduciendo el proceso hijo y si hay mas texto en la cola para reproducir
	
}




void scrstdout_putpixel_final_rgb(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color_rgb GCC_UNUSED)
{
}

void scrstdout_putpixel_final(int x GCC_UNUSED,int y GCC_UNUSED,unsigned int color GCC_UNUSED)
{
}


//Rutina de putchar para menu
void scrstdout_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{
	
	//Para evitar warnings al compilar de "unused parameter"
	x=y;
	y=x;
	caracter=tinta=papel;
	tinta=papel=caracter;
	
}

void scrstdout_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel)
{
	
	//Para evitar warnings al compilar de "unused parameter"
	x=y;
	y=x;
	caracter=tinta=papel;
	tinta=papel=caracter;
	
}




void scrstdout_set_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scrstdout_reset_fullscreen(void)
{
	debug_printf (VERBOSE_ERR,"Full screen mode not supported on this video driver");
}

void scrstdout_z88_cpc_load_keymap(void)
{
	debug_printf (VERBOSE_INFO,"Loading keymap");
}


void scrstdout_textspeech_filter_welcome_message(void)
{

	char texto_welcome[40];
	sprintf(texto_welcome," Welcome to ZEsarUX v." EMULATOR_VERSION " ");
	textspeech_print_speech(texto_welcome);

	char texto_edition[40];
	sprintf(texto_edition," " EMULATOR_EDITION_NAME " ");
	textspeech_print_speech(texto_edition);	

	
	textspeech_print_speech("Press opening curly bracket to manual redraw screen. Press closing curly bracket to automatic redraw screen. Write 'menu' to open the menu. Write 'esc' to simulate scape key on some menu dialogs");
	
	
}

void scrstdout_detectedchar_print(z80_byte caracter)
{
	printf ("%c",caracter);
        //flush de salida standard
        fflush(stdout);

}


//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrstdout_get_menu_width(void)
{
        return 32;
}


int scrstdout_get_menu_height(void)
{
        return 24;
}

int scrstdout_driver_can_ext_desktop (void)
{
        return 0;
}

//Null video drivers
int scrstdout_init (void){ 
	
	debug_printf (VERBOSE_INFO,"Init stdout Video Driver"); 
	
	
	printf ("Press { to manual redraw screen. Press } to automatic redraw screen\nWrite 'menu' to open the menu\nWrite 'esc' to simulate ESC key on some menu dialogs\n");
	
	
	//Mismos mensajes de bienvenida a traves de filtro texto
	if (opcion_no_welcome_message.v==0) scrstdout_textspeech_filter_welcome_message();
	
	
	if (textspeech_filter_program!=NULL) {
		char *mensaje_stop="You can stop listening to menu entries by pressing ENTER.";
		printf("%s\n",mensaje_stop);
		if (opcion_no_welcome_message.v==0) textspeech_print_speech(mensaje_stop);

		char *mensaje_stoptext="Write 'stoptext' to cancel pending filter texts";
		printf ("%s\n",mensaje_stoptext);
		
		//Mismo mensaje de stoptext a traves de filtro texto
		if (opcion_no_welcome_message.v==0) textspeech_print_speech(mensaje_stoptext);
	}
	
	scr_debug_registers=scrstdout_debug_registers;
	scr_messages_debug=scrstdout_messages_debug;
	
	scr_putchar_menu=scrstdout_putchar_menu;
	scr_putchar_footer=scrstdout_putchar_footer;

	scr_putpixel_final=scrstdout_putpixel_final;
	scr_putpixel_final_rgb=scrstdout_putpixel_final_rgb;

        scr_get_menu_width=scrstdout_get_menu_width;
        scr_get_menu_height=scrstdout_get_menu_height;
	scr_driver_can_ext_desktop=scrstdout_driver_can_ext_desktop;
	
	
	scr_set_fullscreen=scrstdout_set_fullscreen;
	scr_reset_fullscreen=scrstdout_reset_fullscreen;
	scr_z88_cpc_load_keymap=scrstdout_z88_cpc_load_keymap;

	scr_detectedchar_print=scrstdout_detectedchar_print;

	//activamos esto en stdout, para que se capture el texto, pero si no tenemos el automatic redraw activado, 
	//para evitar hacer redraw y a la vez hacer print de trap
	if (stdout_simpletext_automatic_redraw.v==0) {
		debug_printf(VERBOSE_DEBUG,"Enabling print char trap as the --autoredrawstdout setting is off");
		chardetect_printchar_enabled.v=1;
	}

	//tambien activar que los textos de menus se envien a filtro (si es que hay filtro)
	//textspeech_also_send_menu.v=1;
	
	
	scr_set_driver_name("stdout");
	
	screen_stdout_driver=1;
	
	
	return 0; 
	
}

void scrstdout_end(void)
{
	
	debug_printf (VERBOSE_INFO,"Closing stdout video driver");
	
	
}

void scrstdout_refresca_pantalla_solo_driver(void)
{
        //Como esto solo lo uso de momento para drivers graficos, de momento lo dejo vacio
}


void scrstdout_refresca_pantalla(void){}
z80_byte scrstdout_lee_puerto(z80_byte puerto_h,z80_byte puerto_l){
	
	//Para evitar warnings al compilar de "unused parameter"
	puerto_h=puerto_l;
	puerto_l=puerto_h;
	
	return 255;
}



int scrstdout_kbhit()
{
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv);
}


//Si se pulsa tecla con menu leyendose. Avisar de parar, e incluso borrar contenido fifo
void scrstdout_menu_kbhit()
{
	if (textspeech_filter_program==NULL) return;
	
	if (scrstdout_kbhit() ) {
		menu_speech_tecla_pulsada=1;
		textspeech_empty_speech_fifo();
		//printf ("tecla pulsada\n");
	}
	
}

int scrstdout_getch()
{
	int r;
	unsigned char c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	} else {
		return c;
	}
}



/*
 * char scrscrstdout_getch() {
 *	char buf = 0;
 *	struct termios old = {0};
 *	if (tcgetattr(0, &old) < 0)
 *		perror("tcsetattr()");
 *	old.c_lflag &= ~ICANON;
 *	old.c_lflag &= ~ECHO;
 *	old.c_cc[VMIN] = 1;
 *	old.c_cc[VTIME] = 0;
 *	if (tcsetattr(0, TCSANOW, &old) < 0)
 *		perror("tcsetattr ICANON");
 *	if (read(0, &buf, 1) < 0)
 *		perror ("read()");
 *	old.c_lflag |= ICANON;
 *	old.c_lflag |= ECHO;
 *	if (tcsetattr(0, TCSADRAIN, &old) < 0)
 *		perror ("tcsetattr ~ICANON");
 *	return (buf);
 * }
 */




void scrstdout_actualiza_tablas_teclado(void){
	
	
	z80_byte c;
	
	//solo hacerlo 1 vez por segundo
	//if (stdout_simpletext_automatic_redraw.v && contador_segundo==0) {
	
	//hacerlo 5 veces por segundo
	//if (stdout_simpletext_automatic_redraw.v && (contador_segundo%200)==0) {

	//10 veces por segundo
	//if (stdout_simpletext_automatic_redraw.v && (contador_segundo%100)==0) {	
	if (stdout_simpletext_automatic_redraw.v && (contador_segundo%(20*scrstdout_simpletext_refresh_factor))==0) {
		scrstdout_repinta_pantalla();
	}
	
	
	if (tecla_activa>10 && tecla_activa<=20) {
		scrstdout_establece_tablas_teclado(anterior_tecla);	
		tecla_activa--;
		return;
	}
	
	if (tecla_activa>0 && tecla_activa<=10) {
		scrstdout_reset_teclas();
		tecla_activa--;
		return;
	}
	
	if (scrstdout_kbhit()) {
		//printf ("tecla pulsada\n");
		c=scrstdout_getch();
		
		buffer_tecla_comando[pos_buffer_tecla_comando]=c;
		if (c!=10 && pos_buffer_tecla_comando!=254) pos_buffer_tecla_comando++;
		
		if (c=='{') {
			debug_printf (VERBOSE_INFO,"Redrawing Screen");
			scrstdout_repinta_pantalla();
			//se supone que el siguiente es enter, no enviarlo
			if (scrstdout_kbhit()) c=scrstdout_getch();
		}
		
		if (c=='}') {
			stdout_simpletext_automatic_redraw.v ^=1;
			if (stdout_simpletext_automatic_redraw.v) debug_printf (VERBOSE_INFO,"Setting automatic Screen Redraw");
			else debug_printf (VERBOSE_INFO,"Disabling automatic Screen Redraw");
			
			//se supone que el siguiente es enter, no enviarlo
			if (scrstdout_kbhit()) c=scrstdout_getch();
		}
		
		
		else {
			notificar_tecla_interrupcion_si_z88();
			//printf ("tecla: %d\n",c);
			//scrstdout_establece_tablas_teclado(c);
			
			//mantener la tecla por 1/10 segundo y liberarla por 1/10 segundo
			tecla_activa=20;
			anterior_tecla=c;
			
			//si es enter, hacer un printf adicional para refrescar buffer
			if (c==10) {
                                printf ("\n");
                                buffer_tecla_comando[pos_buffer_tecla_comando]=0;
				//printf ("comando: %s\n",buffer_tecla_comando);
				textspeech_send_new_line();
				
				
				//hacer acciones segun comando introducido
				if (strlen(buffer_tecla_comando)>0) {
					if (!strcmp(buffer_tecla_comando,"menu")) {
						menu_fire_event_open_menu();
					}


					if (!strcmp(buffer_tecla_comando,"esc")) {
						anterior_tecla=2;
					}
					
					if (!strcmp(buffer_tecla_comando,"stoptext")) {
						//Vaciamos cola speech
						textspeech_empty_speech_fifo();

						if (textspeech_filter_program==NULL) printf ("There is no configured text filter program\n");
					}	
					
					//reseteamos buffer
					pos_buffer_tecla_comando=0;
					buffer_tecla_comando[0]=0;
				}
			}
		}
	}
	
	else {
		//printf ("tecla no pulsada\n");
		scrstdout_reset_teclas();
	}
	
	
/*
Pasado a timer	
	//Si hay texto ahi acumulado pero no se ha recibido salto de linea, al cabo de un segundo, saltar
	scrtextspeech_filter_counter++;
	if (scrtextspeech_filter_counter==50 && index_buffer_speech!=0) {
		debug_printf (VERBOSE_DEBUG,"Forcing sending filter text although there is no carriage return");
		textspeech_add_speech_fifo();
	}
	
	
	//Si hay pendiente speech
	if (textspeech_filter_program!=NULL) {
		//Si hay finalizado el proceso hijo
		//printf ("esperar\n");
		if (textspeech_finalizado_hijo_speech() ) {
			scrtextspeech_filter_run_pending();
		}
	}
*/
	
	
}
void scrstdout_debug_registers(void){

char buffer[2048];

print_registers(buffer);

printf ("%s\r",buffer);

}



void scrstdout_messages_debug(char *s)
{
	printf ("%s\n",s);
	//flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
	fflush(stdout);
	
	if (scrstdout_also_send_speech_debug_messages.v) textspeech_print_speech(s);
	
}

void scrstdout_reset_teclas(void)
{
	//inicializar todas las teclas a nada - 255
	reset_keyboard_ports();
	
}

void scrstdout_establece_tablas_teclado(int c)
{
	//primero inicializar todas las teclas a nada - 255
	scrstdout_reset_teclas();
	
	if (c!=0) {

		//tecla ESC
		if (c==2) {
			puerto_especial1 &=(255-1);
		}

		else ascii_to_keyboard_port(c);
	}
	
}

void stdout_common_fun_color(z80_byte atributo GCC_UNUSED,int *brillo GCC_UNUSED, int *parpadeo GCC_UNUSED)
{

        if (!screen_text_accept_ansi)  return;
        int paper,ink;
        int copia_paper;


        ink = atributo & 7;
        paper = ( atributo >> 3 ) & 7;


        if (atributo & 128) {
                //hay parpadeo
                if (estado_parpadeo.v) {
                        //estado de inversion de color
                        copia_paper=paper;
                        paper=ink;
                        ink=copia_paper;
                }
        }

        //printf ("\x1b[H");
        if (atributo & 64) {
                ink +=8;
                paper +=8;
        }

        screen_text_set_ansi_color_fg(ink);
        screen_text_set_ansi_color_bg(paper);



}

void stdout_common_fun_caracter (int x GCC_UNUSED,int y GCC_UNUSED,int brillo GCC_UNUSED, unsigned char inv GCC_UNUSED,z80_byte caracter )
{
	//printf ("%d",caracter);
	printf ("%c",caracter);
}

void stdout_common_fun_saltolinea (void)
{
	printf ("\n");
}

void scrstdout_repinta_pantalla(void)
{

        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;
	
	//enviar Ansi inicio pantalla
	screen_text_send_ansi_go_home();
	
	
	 //si todo de pixel a ascii art
     if (rainbow_enabled.v && screen_text_all_refresh_pixel.v) {
     
scr_refresca_pantalla_tsconf_text(stdout_common_fun_color,stdout_common_fun_caracter,stdout_common_fun_saltolinea,screen_text_all_refresh_pixel_scale);
     
     }
	
	else if (MACHINE_IS_ZX8081) {
		screen_text_repinta_pantalla_zx81();
	}
	
	else if (MACHINE_IS_Z88) {
		//Si esta vofile activo, hay que dibujar dentro del buffer rainbow
		//Aqui se llama 5 veces por segundo
		//Aunque vofile_send_frame se llama desde core_z88.c,
		//si se genera un video de mas de 5 FPS, el video tendra correctamente los FPS que toquen,
		//pero apareceran frames repetidos, ya que estamos generando nueva imagen en rainbow solo 5 veces por segundo
		if (vofile_inserted.v) {
			set_z88_putpixel_zoom_function();
			screen_z88_refresca_pantalla_comun();
		}
		
		screen_text_repinta_pantalla_z88();
	}

	else if (MACHINE_IS_ACE) {
		screen_text_repinta_pantalla_ace();
	}

	else if (MACHINE_IS_SAM) {
		screen_text_repinta_pantalla_sam();
	}

	else if (MACHINE_IS_CPC) {
                screen_text_repinta_pantalla_cpc();
        }

	else if (MACHINE_IS_CHLOE) {
		screen_text_repinta_pantalla_chloe();
	}

	else if (MACHINE_IS_TSCONF) {
		//Si es modo texto, hacer este refresh:
		z80_byte modo_video=tsconf_get_video_mode_display();
		if (modo_video==3) {
			scr_refresca_pantalla_tsconf_text_textmode(stdout_common_fun_color,stdout_common_fun_caracter,stdout_common_fun_saltolinea,12);
			sem_screen_refresh_reallocate_layers=0;
			return;
		}


		//con rainbow
		if (rainbow_enabled.v) {
			scr_refresca_pantalla_tsconf_text(stdout_common_fun_color,stdout_common_fun_caracter,stdout_common_fun_saltolinea,12);
		}

		//sin rainbow, como spectrum
		else {
                	screen_text_repinta_pantalla_spectrum();
		}

	}
	
	
	
	else {
		//Refresco en Spectrum
		screen_text_repinta_pantalla_spectrum();
		
	}

sem_screen_refresh_reallocate_layers=0;
	
	
}









