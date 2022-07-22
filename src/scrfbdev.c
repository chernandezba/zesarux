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


/* Referencias:

http://man7.org/linux/man-pages/man3/termios.3.html
http://linux.die.net/man/3/cfmakeraw
http://man7.org/linux/man-pages/man4/console_codes.4.html
http://pubs.opengroup.org/onlinepubs/009695399/basedefs/termios.h.html

sobre key press & release:
http://www.linuxjournal.com/article/1080

list scancodes
http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

The scancodes in translated scancode set 2 are given in hex. Between parentheses the keycap on a US keyboard. The scancodes are given in order, grouped according to groups of keys that are usually found next to each other.

00 is normally an error code

01 (Esc)

02 (1!), 03 (2@), 04 (3#), 05 (4$), 06 (5%E), 07 (6^), 08 (7&), 09 (8*), 0a (9(), 0b (0)), 0c (-_), 0d (=+), 0e (Backspace)

0f (Tab), 10 (Q), 11 (W), 12 (E), 13 (R), 14 (T), 15 (Y), 16 (U), 17 (I), 18 (O), 19 (P), 1a ([{), 1b (]})

1c (Enter)

1d (LCtrl)

1e (A), 1f (S), 20 (D), 21 (F), 22 (G), 23 (H), 24 (J), 25 (K), 26 (L), 27 (;:), 28 ('")

29 (`~)

2a (LShift)

2b (\|), on a 102-key keyboard

2c (Z), 2d (X), 2e (C), 2f (V), 30 (B), 31 (N), 32 (M), 33 (,<), 34 (.>), 35 (/?), 36 (RShift)

37 (Keypad-*) or ( * / PrtScn) on a 83/84-key keyboard

38 (LAlt), 39 (Space bar),

3a (CapsLock)

3b (F1), 3c (F2), 3d (F3), 3e (F4), 3f (F5), 40 (F6), 41 (F7), 42 (F8), 43 (F9), 44 (F10)

45 (NumLock)

46 (ScrollLock)

47 (Keypad-7/Home), 48 (Keypad-8/Up), 49 (Keypad-9/PgUp)

4a (Keypad--)

4b (Keypad-4/Left), 4c (Keypad-5), 4d (Keypad-6/Right), 4e (Keypad-+)

4f (Keypad-1/End), 50 (Keypad-2/Down), 51 (Keypad-3/PgDn)

52 (Keypad-0/Ins), 53 (Keypad-./Del)

54 (Alt-SysRq) on a 84+ key keyboard

55 is less common; occurs e.g. as F11 on a Cherry G80-0777 keyboard, as F12 on a Telerate keyboard, as PF1 on a Focus 9000 keyboard, and as FN on an IBM ThinkPad.

56 mostly on non-US keyboards. It is often an unlabelled key to the left or to the right of the left Alt key.

*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>

#include "scrfbdev.h"
#include "debug.h"
#include "screen.h"
#include "cpu.h"
#include "zxvision.h"
#include "charset.h"
#include "utils.h"
#include "joystick.h"
#include "z88.h"
#include "cpc.h"
#include "prism.h"
#include "sam.h"
#include "ql.h"
#include "settings.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "svi.h"

//donde apunta el puntero de doble buffer. O si no hay doble buffer, apunta directamente a memoria de pantalla
/*
Pruebas en pc virtual linux 32 bits. zoom 1
Sin doble buffer: 24% cpu
Con doble buffer: 30% cpu

En raspberry. frameskip 5.
Sin doble buffer: 65% cpu
Con doble buffer: 65% cpu. 12 FPS
*/
z80_byte *double_buffer_pointer;


z80_bit fbdev_double_buffer_enabled={0};

//si no cambia la resolucion en raspberry con modo fullscreen
z80_bit fbdev_no_res_change={0};


z80_byte *fbdev_pointer = 0;

long int fbdev_screensize = 0;
long int offset_centrado=0;

  int fbdev_filedescriptor = 0;
int fbdev_line_length;
int fbdev_ancho,fbdev_alto;
int bpp;

int fbdev_margin_width=0;
int fbdev_margin_height=0;

z80_bit fbdev_sends_release;

int fbdev_use_all_virtual_res=0;

int fbdev_decimal_full_scale_fbdev=0;


int fbdev_tty;

struct DEVS {
    char *fb0;
    char *fbnr;
    char *ttynr;
};

struct DEVS devs_default = {
    fb0:   "/dev/fb0",
    fbnr:  "/dev/fb%d",
    ttynr: "/dev/tty%d",
};
struct DEVS devs_devfs = {
    fb0:   "/dev/fb/0",
    fbnr:  "/dev/fb/%d",
    ttynr: "/dev/vc/%d",
};
struct DEVS *devices;

struct termios termios_valores_orig;

void (*putpixel_fbdev_lowlevel) (int x,int y,z80_byte r,z80_byte g,z80_byte b);
void fbdev_tty_makecooked(struct termios *termops_p);

//para el escalado especial
//escalado x a factor
int x_scale_factor_mult;
int x_scale_factor_div;

//escalado y a factor
int y_scale_factor_mult;
int y_scale_factor_div;


#define MAX_CONTADOR_NOTECLA 5

unsigned long fbdev_initial_tty_mode;


//no utilizar tty. y por tanto no lee teclado
int fbdev_no_uses_tty=0;

//no usar teclado en raw mode
int fbdev_no_uses_ttyraw=0;

//contador que se activa cuando no hay tecla pulsada. Cuando se llegue a max, notificar realmente a core z80 que no hay tecla pulsada
//Esto es debido a que el driver de fbdev, cuando se deja tecla pulsada, retorna alternativamente ERR y tecla, segun un factor de repeticion
//Para que ese ERR no se interprete como no tecla pulsada, damos un minimo de veces que se debe suceder para que realmente se considere no tecla
int scrfbdev_contador_notecla=0;


//Teclas de Z88 asociadas a cada tecla del teclado fisico
unsigned char scrfbdev_keymap_z88_cpc_minus;
unsigned char scrfbdev_keymap_z88_cpc_equal;
unsigned char scrfbdev_keymap_z88_cpc_backslash;
unsigned char scrfbdev_keymap_z88_cpc_bracket_left;
unsigned char scrfbdev_keymap_z88_cpc_bracket_right;
unsigned char scrfbdev_keymap_z88_cpc_semicolon;
unsigned char scrfbdev_keymap_z88_cpc_apostrophe;
unsigned char scrfbdev_keymap_z88_cpc_pound;
unsigned char scrfbdev_keymap_z88_cpc_comma;
unsigned char scrfbdev_keymap_z88_cpc_period;
unsigned char scrfbdev_keymap_z88_cpc_slash;

unsigned char scrfbdev_keymap_z88_cpc_circunflejo;
unsigned char scrfbdev_keymap_z88_cpc_colon;
unsigned char scrfbdev_keymap_z88_cpc_arroba;
unsigned char scrfbdev_keymap_z88_cpc_leftz; //Tecla a la izquierda de la Z. Solo usada en Chloe


void scrfbdev_z88_cpc_load_keymap(void)
{

	debug_printf (VERBOSE_INFO,"Loading keymap");

	//Teclas se ubican en misma disposicion fisica del Z88, excepto:
	//libra~ -> spanish: cedilla (misma ubicacion fisica del z88). english: acento grave (supuestamente a la izquierda del 1)
	//backslash: en english esta en fila inferior del z88. en spanish, lo ubicamos a la izquierda del 1 (ºª\)

	//en modo raw, las teclas en driver fbdev retornan los mismos valores que un teclado english,
	//por tanto con esto podriamos mapear cualquier teclado fisico, sea ingles, spanish, danes o lo que sea,
	//y los codigos raw de retorno siempre son los mismos.
	//por tanto, devolvemos lo mismo que con keymap english siempre:

	if (MACHINE_IS_Z88 || MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI) {
		scrfbdev_keymap_z88_cpc_minus=RAWKEY_minus;
		scrfbdev_keymap_z88_cpc_equal=RAWKEY_equal;
		scrfbdev_keymap_z88_cpc_backslash=RAWKEY_grave;

		scrfbdev_keymap_z88_cpc_bracket_left=RAWKEY_bracket_left;
		scrfbdev_keymap_z88_cpc_bracket_right=RAWKEY_bracket_right;
		scrfbdev_keymap_z88_cpc_semicolon=RAWKEY_semicolon;
		scrfbdev_keymap_z88_cpc_apostrophe=RAWKEY_apostrophe;
		scrfbdev_keymap_z88_cpc_pound=RAWKEY_backslash;
		scrfbdev_keymap_z88_cpc_comma=RAWKEY_comma;
		scrfbdev_keymap_z88_cpc_period=RAWKEY_period;
		scrfbdev_keymap_z88_cpc_slash=RAWKEY_slash;
		scrfbdev_keymap_z88_cpc_leftz=RAWKEY_leftz;
	}

	else if (MACHINE_IS_CPC) {
		scrfbdev_keymap_z88_cpc_minus=RAWKEY_minus;
		scrfbdev_keymap_z88_cpc_circunflejo=RAWKEY_equal;

		scrfbdev_keymap_z88_cpc_arroba=RAWKEY_bracket_left;
                scrfbdev_keymap_z88_cpc_bracket_left=RAWKEY_bracket_right;
                scrfbdev_keymap_z88_cpc_colon=RAWKEY_semicolon;
                scrfbdev_keymap_z88_cpc_semicolon=RAWKEY_apostrophe;
                scrfbdev_keymap_z88_cpc_bracket_right=RAWKEY_backslash;
                scrfbdev_keymap_z88_cpc_comma=RAWKEY_comma;
                scrfbdev_keymap_z88_cpc_period=RAWKEY_period;
                scrfbdev_keymap_z88_cpc_slash=RAWKEY_slash;

		scrfbdev_keymap_z88_cpc_backslash=RAWKEY_grave;
		scrfbdev_keymap_z88_cpc_leftz=RAWKEY_leftz;
        }

	else if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
		scrfbdev_keymap_z88_cpc_minus=RAWKEY_minus;
                scrfbdev_keymap_z88_cpc_equal=RAWKEY_equal;
                scrfbdev_keymap_z88_cpc_backslash=RAWKEY_grave;
                scrfbdev_keymap_z88_cpc_bracket_left=RAWKEY_bracket_left;
                scrfbdev_keymap_z88_cpc_bracket_right=RAWKEY_bracket_right;
                scrfbdev_keymap_z88_cpc_semicolon=RAWKEY_semicolon;
                scrfbdev_keymap_z88_cpc_apostrophe=RAWKEY_apostrophe;
                scrfbdev_keymap_z88_cpc_pound=RAWKEY_backslash;
                scrfbdev_keymap_z88_cpc_comma=RAWKEY_comma;
                scrfbdev_keymap_z88_cpc_period=RAWKEY_period;
                scrfbdev_keymap_z88_cpc_slash=RAWKEY_slash;
		scrfbdev_keymap_z88_cpc_leftz=RAWKEY_leftz;
        }





	scrfbdev_keymap_z88_cpc_colon=RAWKEY_semicolon;

	return;



}

int scrfbdev_kbhit()
{
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv);
}

int scrfbdev_getch()
{
	int r;
	unsigned char c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	} else {
		return c;
	}
}




void scrfbdev_refresca_border(void)
{
	scr_refresca_border();

}


void scrfbdev_putchar_zx8081(int x,int y, z80_byte caracter)
{

	scr_putchar_zx8081_comun(x,y, caracter);


}


//Rutina de putchar para menu
void scrfbdev_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{


	z80_bit inverse;

	//caracter=da_codigo81(caracter,&inverse);
	//printf ("%c",caracter);

	inverse.v=0;

        //128 y 129 corresponden a franja de menu y a letra enye minuscula
        if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';

        scr_putchar_menu_comun_zoom(caracter,x,y,inverse,tinta,papel,menu_gui_zoom);


}

void scrfbdev_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel)
{

	int yorigen;

	yorigen=screen_get_emulated_display_height_no_zoom_bottomborder_en()/8;




	//scr_putchar_menu(x,yorigen+y,caracter,tinta,papel);
        y +=yorigen;
        z80_bit inverse;

        inverse.v=0;

        //128 y 129 corresponden a franja de menu y a letra enye minuscula
        if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';
        //scr_putchar_menu_comun_zoom(caracter,x,y,inverse,tinta,papel,1);
		scr_putchar_footer_comun_zoom(caracter,x,y,inverse,tinta,papel);

}




void scrfbdev_refresca_pantalla_zx81(void)
{

	scr_refresca_pantalla_y_border_zx8081();

}

void scrfbdev_refresca_pantalla_solo_driver(void)
{

    if (fbdev_double_buffer_enabled.v) {
        //Hacer flush del doble buffer a la pantalla

        //En este caso se vuelca siempre toda la pantalla
        if (ventana_fullscreen && fbdev_decimal_full_scale_fbdev) {
            memcpy(fbdev_pointer,double_buffer_pointer,fbdev_screensize);
        }

        else {

        //int total_lineas=100; //temp
        int totalspectrumalto=screen_get_window_size_height_zoom_border_en();
        int y;


        z80_byte *origen;
        z80_byte *destino;        

        for (y=0;y<totalspectrumalto;y++) {


            origen=double_buffer_pointer+offset_centrado+fbdev_line_length*y;
            destino=fbdev_pointer+offset_centrado+fbdev_line_length*y;

            memcpy(destino,origen,fbdev_line_length);

        }
        }
    }
}



void scrfbdev_refresca_pantalla(void)
{


        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;


	if (MACHINE_IS_ZX8081) {


		//scr_refresca_pantalla_rainbow_comun();
		scrfbdev_refresca_pantalla_zx81();
	}

        else if (MACHINE_IS_PRISM) {
                screen_prism_refresca_pantalla();
        }

        else if (MACHINE_IS_TBBLUE) {
                screen_tbblue_refresca_pantalla();
        }		


	else if (MACHINE_IS_SPECTRUM) {

		if (MACHINE_IS_TSCONF)	screen_tsconf_refresca_pantalla();

		  else { //Spectrum no TSConf

		//modo clasico. sin rainbow
		if (rainbow_enabled.v==0) {
			if (border_enabled.v) {
				//ver si hay que refrescar border
				if (modificado_border.v)
				{
					scrfbdev_refresca_border();
					modificado_border.v=0;
				}

			}

			scr_refresca_pantalla_comun();
		}

		else {
			//modo rainbow - real video
			scr_refresca_pantalla_rainbow_comun();
		}
		}
	}

	else if (MACHINE_IS_Z88) {
		screen_z88_refresca_pantalla();
	}

        else if (MACHINE_IS_ACE) {
                scr_refresca_pantalla_y_border_ace();
        }

	else if (MACHINE_IS_CPC) {
                scr_refresca_pantalla_y_border_cpc();
        }

        else if (MACHINE_IS_SAM) {
                scr_refresca_pantalla_y_border_sam();
        }

        else if (MACHINE_IS_QL) {
                scr_refresca_pantalla_y_border_ql();
        }

        else if (MACHINE_IS_MK14) {
                scr_refresca_pantalla_y_border_mk14();
        }

	else if (MACHINE_IS_MSX) {
		scr_refresca_pantalla_y_border_msx();
	}    

	else if (MACHINE_IS_SVI) {
		scr_refresca_pantalla_y_border_svi();
	}    		


	else if (MACHINE_IS_COLECO) {
		scr_refresca_pantalla_y_border_coleco();
	}    

	else if (MACHINE_IS_SG1000) {
		scr_refresca_pantalla_y_border_sg1000();
	}  

	else if (MACHINE_IS_SMS) {
		scr_refresca_pantalla_y_border_sms();
	}         


	screen_render_menu_overlay_if_active();


        //Escribir footer
        draw_middle_footer();

    scrfbdev_refresca_pantalla_solo_driver();


	sem_screen_refresh_reallocate_layers=0;


}





void scrfbdev_set_fullscreen(void)
{
	//debug_printf (VERBOSE_ERR,"fbdev: Full screen mode not supported on this video driver");

	scrfbdev_end();
	ventana_fullscreen=1;
	scrfbdev_init();

	modificado_border.v=1;

}

void scrfbdev_reset_fullscreen(void)
{
	//debug_printf (VERBOSE_ERR,"fbdev: Full screen mode not supported on this video driver");

	scrfbdev_end();
	ventana_fullscreen=0;

	zoom_x=zoom_x_original;
	zoom_y=zoom_y_original;

	set_putpixel_zoom();

	scrfbdev_init();

	modificado_border.v=1;
}

int fbdev_setvt(int vtno)
{
	struct vt_stat vts;
	char vtname[12];

	if (vtno < 0) {
		if (-1 == ioctl(fbdev_tty,VT_OPENQRY, &vtno) || vtno == -1) {
			debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_OPENQRY");
			return 1;
		}
	}

	vtno &= 0xff;
	sprintf(vtname, devices->ttynr, vtno);
	chown(vtname, getuid(), getgid());
	if (-1 == access(vtname, R_OK | W_OK)) {
		debug_printf(VERBOSE_ERR,"fbdev: access %s: %s\n",vtname,strerror(errno));
		return 1;
	}

	if (-1 == ioctl(fbdev_tty,VT_GETSTATE, &vts)) {
		debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_GETSTATE");
		return 1;
	}
	//orig_vt_no = vts.v_active;
	if (-1 == ioctl(fbdev_tty,VT_ACTIVATE, vtno)) {
		debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_ACTIVATE");
		return 1;
	}
	if (-1 == ioctl(fbdev_tty,VT_WAITACTIVE, vtno)) {
		debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_WAITACTIVE");
		return 1;
	}

	return 0;
}

/* Hmm. radeonfb needs this. matroxfb doesn't. */
static int fbdev_activate_current(int tty)
{
	struct vt_stat vts;

	if (-1 == ioctl(tty,VT_GETSTATE, &vts)) {
		debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_GETSTATE");
		return 1;
	}
	if (-1 == ioctl(tty,VT_ACTIVATE, vts.v_active)) {
		debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_ACTIVATE");
		return 1;
	}
	if (-1 == ioctl(tty,VT_WAITACTIVE, vts.v_active)) {
		debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_WAITACTIVE");
		return 1;
	}
	return 0;
}

void fbdev_cls(void)
{

	int x,y;

	//Borrar pantalla
	for (y=0;y<fbdev_alto;y++) {
		for (x=0;x<fbdev_ancho;x++) {
			putpixel_fbdev_lowlevel(x,y,0,0,0);
		}
	}

}


void scrfbdev_end(void)
{
	debug_printf (VERBOSE_INFO,"Closing video driver");
	//establecer consola texto
	ioctl(fbdev_tty, KDSETMODE, KD_TEXT);

	//system ("/bin/stty cooked");

	//restaurar valores terminal
	if (!fbdev_no_uses_tty) {
		tcsetattr(fbdev_tty, TCSANOW, &termios_valores_orig);

		if (fbdev_sends_release.v) {
			if (ioctl(0, KDSKBMODE, fbdev_initial_tty_mode)) {
				debug_printf(VERBOSE_ERR,"fbdev: Couldn't restore tty original mode");
			}
		}

	}



	/*
	 * struct termios termios_valores;
	 * if (tcgetattr(fbdev_tty,&termios_valores)==-1)
	 * {
	 * debug_printf(VERBOSE_ERR,"fbdev: couldn't set tty raw mode");
	 * //return 1;
}

fbdev_tty_makecooked(&termios_valores);


if (tcsetattr(fbdev_tty, TCSANOW, &termios_valores)==-1) {
	debug_printf(VERBOSE_ERR,"fbdev: couldn't set tty raw mode");
	//return 1;
}
*/
	fbdev_cls();

	munmap(fbdev_pointer, fbdev_screensize);
	close(fbdev_filedescriptor);

    if (fbdev_double_buffer_enabled.v) {
        free(double_buffer_pointer);
    }


}

z80_byte scrfbdev_lee_puerto(z80_byte puerto_h,z80_byte puerto_l){

	//Para evitar warnings al compilar de "unused parameter"
	puerto_h=puerto_l;
	puerto_l=puerto_h;


	return 255;
}

z80_byte scrfbdev_convert_ordinary_scancodes(z80_byte scancode)
{
	/*
	 *
	 * http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
	 *
	 * The scancodes in translated scancode set 2 are given in hex. Between parentheses the keycap on a US keyboard. The scancodes are given in order, grouped according to groups of keys that are usually found next to each other.
	 *
	 * 00 is normally an error code
	 * 01 (Esc)
	 * 02 (1!), 03 (2@), 04 (3#), 05 (4$), 06 (5%E), 07 (6^), 08 (7&), 09 (8*), 0a (9(), 0b (0)), 0c (-_), 0d (=+), 0e (Backspace)
	 * 0f (Tab), 10 (Q), 11 (W), 12 (E), 13 (R), 14 (T), 15 (Y), 16 (U), 17 (I), 18 (O), 19 (P), 1a ([{), 1b (]})
	 * 1c (Enter)
	 * 1d (LCtrl)
	 * 1e (A), 1f (S), 20 (D), 21 (F), 22 (G), 23 (H), 24 (J), 25 (K), 26 (L), 27 (;:), 28 ('")
	 *
	 * 29 (`~)
	 * 2a (LShift)
	 * 2b (\|), on a 102-key keyboard
	 * 2c (Z), 2d (X), 2e (C), 2f (V), 30 (B), 31 (N), 32 (M), 33 (,<), 34 (.>), 35 (/?), 36 (RShift)
	 * 37 (Keypad-*) or ( * /PrtScn) on a 83/84-key keyboard
	 *
	 * 38 (LAlt), 39 (Space bar),
	 * 3a (CapsLock)
	 * 3b (F1), 3c (F2), 3d (F3), 3e (F4), 3f (F5), 40 (F6), 41 (F7), 42 (F8), 43 (F9), 44 (F10)
	 * 45 (NumLock)
	 * 46 (ScrollLock)
	 *
	 * 47 (Keypad-7/Home), 48 (Keypad-8/Up), 49 (Keypad-9/PgUp)
	 * 4a (Keypad--)
	 * 4b (Keypad-4/Left), 4c (Keypad-5), 4d (Keypad-6/Right), 4e (Keypad-+)
	 * 4f (Keypad-1/End), 50 (Keypad-2/Down), 51 (Keypad-3/PgDn)
	 * 52 (Keypad-0/Ins), 53 (Keypad-./Del)
	 *
	 * 54 (Alt-SysRq) on a 84+ key keyboard
	 * 55 is less common; occurs e.g. as F11 on a Cherry G80-0777 keyboard, as F12 on a Telerate keyboard, as PF1 on a Focus 9000 keyboard, and as FN on an IBM ThinkPad.
	 * 56 mostly on non-US keyboards. It is often an unlabelled key to the left or to the right of the left Alt key.
	 */


	char teclas[]={
		0x00,
		0x00,
		'1','2','3','4','5','6','7','8','9','0','-','=',0x00,
		0x00,'q','w','e','r','t','y','u','i','o','p','[',']',
		0x00, //Enter
		0x00, //LCtrl
		'a','s','d','f','g','h','j','k','l',';','\'',
		0x00,
		0x00,
		0x00,
		'z','x','c','v','b','n','m',',','.','/'
	};

	if (scancode>=sizeof(teclas) ) return 0;

	else return teclas[scancode];

}


unsigned char temp_buffer_teclas[5];
char temp_index_buffer_teclas=0;

//Rutina de teclado pero para modo raw, donde se envian send + release
//Nota: estos valores de teclado son identicos a los leidos desde assembler,
//e incluso a los que se leerian en un msdos
//por ejemplo, en mi emulador ZXSpectr, la rutina es equivalente
//https://github.com/chernandezba/zxspectr
void scrfbdev_actualiza_tablas_teclado_rawmode(void){

	int pressrelease;
	z80_byte teclaraw;
	z80_byte tecla;

	//if (scrfbdev_kbhit()) {
	while (scrfbdev_kbhit()) {


		pressrelease=1;

		//scrfbdev_contador_notecla=0;

		//printf ("tecla pulsada\n");
		teclaraw=scrfbdev_getch();


		/*
 		if (teclaraw==0x60) {
		 			//Para cursores, etc. NO del keypad.
		 			//estos vienen precedidos por el codigo 0x60, y luego viene el mismo codigo
		 			//que se generaria en el keypad. por tanto, este 0x60 simplemente la rutina lo ignorara

		}
		*/


		//Para hacer debug de teclas pulsadas, descomentar las lineas siguientes hasta el debug_printf (VERBOSE_ERR,texto_teclas); }

		/*
		if (temp_index_buffer_teclas<5) {
			temp_buffer_teclas[temp_index_buffer_teclas++]=teclaraw;
		}

		else {
			char texto_teclas[80];
			sprintf (texto_teclas,"%x %x %x %x %x",temp_buffer_teclas[0],temp_buffer_teclas[1],temp_buffer_teclas[2],
					temp_buffer_teclas[3],temp_buffer_teclas[4]);
			temp_index_buffer_teclas=0;

			debug_printf (VERBOSE_ERR,texto_teclas);
		}
		*/
		//Fin Debug





		if (teclaraw & 128 ) {
			pressrelease=0;
			teclaraw=teclaraw & 127;
		}


		if (pressrelease) notificar_tecla_interrupcion_si_z88();

		//debug_printf (VERBOSE_ERR,"Tecla pulsada: %u",teclaraw);

		//temp
		//modo raw, 'f' fin es 33
		//if (teclaraw==33) {
		//scrfbdev_end();
		//exit(0);
		//}


        //Teclas que necesitan conversion de teclado para Chloe
        int tecla_gestionada_chloe=0;
        if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
                        tecla_gestionada_chloe=1;

                        if (teclaraw==scrfbdev_keymap_z88_cpc_minus) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_MINUS,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_equal) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_EQUAL,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_backslash) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BACKSLASH,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_left) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BRACKET_LEFT,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_right) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BRACKET_RIGHT,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_semicolon) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_SEMICOLON,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_apostrophe) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_APOSTROPHE,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_pound) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_POUND,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_comma) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_COMMA,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_period) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_PERIOD,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_slash) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_SLASH,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_leftz) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_LEFTZ,pressrelease);

                        else tecla_gestionada_chloe=0;
        }


        if (tecla_gestionada_chloe) return;



        int tecla_gestionada_sam_ql=0;
        if (MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI) {
                tecla_gestionada_sam_ql=1;

                        if (teclaraw==scrfbdev_keymap_z88_cpc_minus) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_MINUS,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_equal) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_EQUAL,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_backslash) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BACKSLASH,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_left) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BRACKET_LEFT,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_right) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BRACKET_RIGHT,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_semicolon) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_SEMICOLON,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_apostrophe) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_APOSTROPHE,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_pound) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_POUND,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_comma) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_COMMA,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_period) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_PERIOD,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_slash) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_SLASH,pressrelease);


                else tecla_gestionada_sam_ql=0;
        }

        if (tecla_gestionada_sam_ql) return;



		switch (teclaraw) {

			case 0x00:
				break;

			case RAWKEY_Space:
				util_set_reset_key(UTIL_KEY_SPACE,pressrelease);
				break;

			case RAWKEY_Return:
				util_set_reset_key(UTIL_KEY_ENTER,pressrelease);
				break;

			case RAWKEY_Shift_L:
				util_set_reset_key(UTIL_KEY_SHIFT_L,pressrelease);
				break;

			case RAWKEY_Shift_R:
				joystick_possible_rightshift_key(pressrelease);
				break;

			case RAWKEY_Alt_L:
				util_set_reset_key(UTIL_KEY_ALT_L,pressrelease);
				break;

				//case RAWKEY_Alt_R:
				//        joystick_possible_rightalt_key(pressrelease);
				//break;

			case RAWKEY_Control_L:
				util_set_reset_key(UTIL_KEY_CONTROL_L,pressrelease);
				break;

				//case RAWKEY_Control_R:
				//	joystick_possible_rightctrl_key(pressrelease);
				//break;


				//Teclas que generan doble pulsacion
			case RAWKEY_BackSpace:
				util_set_reset_key(UTIL_KEY_BACKSPACE,pressrelease);
				break;

			case RAWKEY_Keypad_Home:
				joystick_possible_home_key(pressrelease);
				break;

			case RAWKEY_Keypad_Left:
				util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
				break;

			case RAWKEY_Keypad_Right:
				util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
				break;


			case RAWKEY_Keypad_Down:
				util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
				break;

			case RAWKEY_Keypad_Up:
				util_set_reset_key(UTIL_KEY_UP,pressrelease);
				break;

			case RAWKEY_Tab:
				util_set_reset_key(UTIL_KEY_TAB,pressrelease);
				break;

			case RAWKEY_Caps_Lock:
				util_set_reset_key(UTIL_KEY_CAPS_LOCK,pressrelease);
				break;

			case RAWKEY_comma:
				//util_set_reset_key(UTIL_KEY_COMMA,pressrelease);
				util_set_reset_key(',',pressrelease);
				break;

			case RAWKEY_period:
				//util_set_reset_key(UTIL_KEY_PERIOD,pressrelease);
				util_set_reset_key('.',pressrelease);
				break;


			case RAWKEY_KP_Subtract:
				util_set_reset_key(UTIL_KEY_MINUS,pressrelease);
				break;

			case RAWKEY_KP_Add:
				util_set_reset_key(UTIL_KEY_PLUS,pressrelease);
				break;

			case RAWKEY_KP_Divide:
				//util_set_reset_key(UTIL_KEY_SLASH,pressrelease);
				util_set_reset_key('/',pressrelease);
				break;

			case RAWKEY_KP_Multiply:
				util_set_reset_key(UTIL_KEY_ASTERISK,pressrelease);
				break;


				//F1 pulsado
			case RAWKEY_F1:
				util_set_reset_key(UTIL_KEY_F1,pressrelease);
				break;

				//F2 pulsado
			case RAWKEY_F2:
				util_set_reset_key(UTIL_KEY_F2,pressrelease);
				break;

				//F3 pulsado
			case RAWKEY_F3:
				util_set_reset_key(UTIL_KEY_F3,pressrelease);
				break;

				//F4 pulsado
			case RAWKEY_F4:
				util_set_reset_key(UTIL_KEY_F4,pressrelease);
				break;

				//F5 pulsado
			case RAWKEY_F5:
				util_set_reset_key(UTIL_KEY_F5,pressrelease);
				break;

        //F6 pulsado
			case RAWKEY_F6:
				util_set_reset_key(UTIL_KEY_F6,pressrelease);
				break;

        //F7 pulsado
			case RAWKEY_F7:
				util_set_reset_key(UTIL_KEY_F7,pressrelease);
				break;


				//F8 pulsado
			case RAWKEY_F8:
				util_set_reset_key(UTIL_KEY_F8,pressrelease);
				break;

				//F9 pulsado
			case RAWKEY_F9:
				util_set_reset_key(UTIL_KEY_F9,pressrelease);
				break;

				//F10 pulsado
			case RAWKEY_F10:
				util_set_reset_key(UTIL_KEY_F10,pressrelease);
				break;



				//ESC pulsado
			case RAWKEY_Escape:
				util_set_reset_key(UTIL_KEY_ESC,pressrelease);
				break;

				//PgUp
			case RAWKEY_Keypad_Page_Up:
				util_set_reset_key(UTIL_KEY_PAGE_UP,pressrelease);
				break;


				//PgDn
			case RAWKEY_Keypad_Page_Down:
				util_set_reset_key(UTIL_KEY_PAGE_DOWN,pressrelease);
				break;

			//Cursores en raspberry
#ifdef EMULATE_RASPBERRY

			case RAWKEY_RPI_Home:
				joystick_possible_home_key(pressrelease);
			break;

			case RAWKEY_RPI_Up:
				util_set_reset_key(UTIL_KEY_UP,pressrelease);
			break;

			case RAWKEY_RPI_Left:
				util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
			break;

			case RAWKEY_RPI_Right:
				util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
			break;

			case RAWKEY_RPI_Down:
				util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
			break;

#endif


			default:

				//tecla ordinaria
				tecla=scrfbdev_convert_ordinary_scancodes(teclaraw);
				//printf (" parseada: %u '%c' \n",tecla, ( tecla>31 && tecla<128 ? tecla : '.' ) );

				//convert_numeros_letras_puerto_teclado(tecla,pressrelease);
				if (tecla<256) util_set_reset_key(tecla,pressrelease);
				break;

		}

		//Fuera del switch

        //Teclas que necesitan conversion de teclado para CPC
        if (MACHINE_IS_CPC) {

                        if (teclaraw==scrfbdev_keymap_z88_cpc_minus) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_MINUS,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_circunflejo) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_CIRCUNFLEJO,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_arroba) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_ARROBA,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_left) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BRACKET_LEFT,pressrelease);



                        else if (teclaraw==scrfbdev_keymap_z88_cpc_colon) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_COLON,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_semicolon) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_SEMICOLON,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_right) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BRACKET_RIGHT,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_comma) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_COMMA,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_period) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_PERIOD,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_slash) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_SLASH,pressrelease);

                        else if (teclaraw==scrfbdev_keymap_z88_cpc_backslash) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BACKSLASH,pressrelease);


        }


		//Teclas que necesitan conversion de teclado para Z88
		if (!MACHINE_IS_Z88) return;

		if (teclaraw==scrfbdev_keymap_z88_cpc_minus) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_MINUS,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_equal) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_EQUAL,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_backslash) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BACKSLASH,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_left) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BRACKET_LEFT,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_bracket_right) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BRACKET_RIGHT,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_semicolon) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_SEMICOLON,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_apostrophe) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_APOSTROPHE,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_pound) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_POUND,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_comma) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_COMMA,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_period) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_PERIOD,pressrelease);

		else if (teclaraw==scrfbdev_keymap_z88_cpc_slash) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_SLASH,pressrelease);




	}


}




//Rutina de teclado pero NO modo raw, y NO se envian send + release
void scrfbdev_actualiza_tablas_teclado(void){


	if (fbdev_no_uses_tty) {
		reset_keyboard_ports();
		return;
	}

	if (!scrfbdev_kbhit()) {
		if (scrfbdev_contador_notecla<=MAX_CONTADOR_NOTECLA) {
			scrfbdev_contador_notecla++;
			//hasta que no pasen unas cuantas veces, no liberar esa tecla
			return;
		}
	}


	reset_keyboard_ports();


	unsigned char tecla;



	//if (scrfbdev_kbhit()) {
	while (scrfbdev_kbhit()) {

		notificar_tecla_interrupcion_si_z88();
		scrfbdev_contador_notecla=0;

		//printf ("tecla pulsada\n");
		tecla=scrfbdev_getch();
		//printf (" tecla %u \n",tecla);
		//debug_printf (VERBOSE_ERR,"Tecla pulsada: %u",tecla);


		if (tecla==13) tecla=10;

		//ESC
		if (tecla==27) {

			if (scrfbdev_kbhit() ) {
				unsigned char tecla2;
				tecla2=scrfbdev_getch();
				//printf (" tecla2 %u \n",tecla2);

				switch (tecla2) {
					case '[':
						if (scrfbdev_kbhit() ) {
							unsigned char tecla3;
							tecla3=scrfbdev_getch();
							//printf (" tecla3 %u \n",tecla3);

							switch (tecla3) {




								//cursor up
								case 'A':
									util_set_reset_key(UTIL_KEY_UP,1);
									break;

									//cursor down
								case 'B':
									util_set_reset_key(UTIL_KEY_DOWN,1);
									break;

									//cursor right
								case 'C':
									util_set_reset_key(UTIL_KEY_RIGHT,1);
									break;

									//cursor left
								case 'D':
									util_set_reset_key(UTIL_KEY_LEFT,1);
									break;

									//doble  esc. F1, F2
								case '[':
									if (scrfbdev_kbhit() ) {
										unsigned char tecla4;
										tecla4=scrfbdev_getch();
										//printf (" tecla4 %u \n",tecla4);

										switch (tecla4) {
											//F1
											case 'A':
												util_set_reset_key(UTIL_KEY_F1,1);
												break;

												//F2
											case 'B':
												util_set_reset_key(UTIL_KEY_F2,1);
												break;

												//F3
											case 'C':
												util_set_reset_key(UTIL_KEY_F3,1);
												break;

												//F4
											case 'D':
												util_set_reset_key(UTIL_KEY_F4,1);
												break;

												//F5
											case 'E':
												util_set_reset_key(UTIL_KEY_F5,1);
												break;


										}
									}

									break;

									//para F10
											case 50:
												if (scrfbdev_kbhit() ) {
													unsigned char tecla4;
													tecla4=scrfbdev_getch();
													//printf (" tecla4 %u \n",tecla4);

													switch (tecla4) {

														//F10
														case 49:
															util_set_reset_key(UTIL_KEY_F10,1);
															break;

													}
												}

												break;

												//Para Home
														case 49:
															if (scrfbdev_kbhit() ) {
																unsigned char tecla4;
																tecla4=scrfbdev_getch();
																//printf (" tecla4 %u \n",tecla4);

																switch (tecla4) {

																	case 126:
																		joystick_possible_home_key(1);
																		break;

																}
															}

															break;


															//Para PgUp
																	case 53:
																		if (scrfbdev_kbhit() ) {
																			unsigned char tecla4;
																			tecla4=scrfbdev_getch();
																			//printf (" tecla4 %u \n",tecla4);

																			switch (tecla4) {

																				case 126:
																					util_set_reset_key(UTIL_KEY_PAGE_UP,1);
																					break;

																			}
																		}

																		break;


																		//Para PgDn
																				case 54:
																					if (scrfbdev_kbhit() ) {
																						unsigned char tecla4;
																						tecla4=scrfbdev_getch();
																						//printf (" tecla4 %u \n",tecla4);

																						switch (tecla4) {

																							//PgDn
																							case 126:
																								util_set_reset_key(UTIL_KEY_PAGE_DOWN,1);
																								break;

																						}
																					}

																					break;








							}
						}

						break;


																							default:
																								//Cualquier otra cosa entendemos que es ALT+letra
																								set_symshift();

																								ascii_to_keyboard_port(tecla2);
																								break;

				}
			}

			else {
				//solo escape
				util_set_reset_key(UTIL_KEY_ESC,1);
			}

		}

		//algunas otras teclas

		//CTRL+1. Edit
		else if (tecla==28) {
			puerto_65278 &=255-1;
			puerto_63486 &=255-1;

		}

		//TAB emula sym+shift
		else if (tecla==9) {
			util_set_reset_key(UTIL_KEY_TAB,1);
		}

		//BACKSPACE
		else if (tecla==127) {
			util_set_reset_key(UTIL_KEY_BACKSPACE,1);
		}


		else {
			if (tecla!=0) {
				ascii_to_keyboard_port(tecla);
			}

		}
	}



}



void scrfbdev_debug_registers(void){

char buffer[2048];

print_registers(buffer);

printf ("%s\r",buffer);

}



void scrfbdev_messages_debug(char *s)
{
	printf ("%s\n",s);
	//flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
	fflush(stdout);

}




//Formato habitual de raspberry . el formato de los colores no se comporta como en pc
void putpixel_fbdev_lowlevel_32bpp_rgb(int x,int y,z80_byte r,z80_byte g,z80_byte b)
{
	z80_byte *puntero;
	int offset;

	//offset=((fbdev_ancho*y)+x)*4;
	offset=y*fbdev_line_length + x*4;

	puntero=double_buffer_pointer+offset+offset_centrado;


	//red
	*puntero++=r;

	//green
	*puntero++=g;


	//blue
	*puntero++=b;

	//Alpha. Valor 0?
	*puntero++=0x00;
}

//Formato habitual de PC
void putpixel_fbdev_lowlevel_32bpp_bgr(int x,int y,z80_byte r,z80_byte g,z80_byte b)
{
	z80_byte *puntero;
	unsigned int offset;

	//offset=((fbdev_ancho*y)+x)*4;
	offset=y*fbdev_line_length + x*4;

	puntero=double_buffer_pointer+offset+offset_centrado;

	//blue
	*puntero++=b;

	//green
	*puntero++=g;

	//red
	*puntero++=r;

	//Alpha. Valor 0?
	*puntero++=0x00;
}

//Formato habitual de raspberry. El formato de los colores no se comporta como en pc
void putpixel_fbdev_lowlevel_24bpp_rgb(int x,int y,z80_byte r,z80_byte g,z80_byte b)
{
	z80_byte *puntero;
	unsigned int offset;

	//offset=((fbdev_ancho*y)+x)*3;
	offset=y*fbdev_line_length + x*3;
	puntero=double_buffer_pointer+offset+offset_centrado;

	//red
	*puntero++=r;

	//green
	*puntero++=g;

	//blue
	*puntero++=b;


}

//Formato habitual de PC
void putpixel_fbdev_lowlevel_24bpp_bgr(int x,int y,z80_byte r,z80_byte g,z80_byte b)
{
	z80_byte *puntero;
	unsigned int offset;

	//offset=((fbdev_ancho*y)+x)*3;
	offset=y*fbdev_line_length + x*3;
	puntero=double_buffer_pointer+offset+offset_centrado;

	//blue
	*puntero++=b;

	//green
	*puntero++=g;

	//red
	*puntero++=r;


}


void putpixel_fbdev_lowlevel_16bpp(int x,int y,z80_byte r,z80_byte g,z80_byte b)
{
	z80_byte *puntero;
	unsigned int offset;

	//offset=((fbdev_ancho*y)+x)*2;
	offset=y*fbdev_line_length + x*2;
	puntero=double_buffer_pointer+offset+offset_centrado;

	//r5g6b5: A 16bpp format.
	//6 bits: entre 0 y 63
	//5 bits: entre 0 y 31
	//TODO. esto es correcto???
	r=r>>3;
	b=b>>3;
	g=g>>2;


	unsigned short int t = r<<11 | g << 5 | b;
	*puntero++=t & 0xFF;
	*puntero++=(t>>8) & 0xFF;

}

void scrfbdev_putpixel_final_rgb(int x,int y,unsigned int color_rgb)
{
	z80_byte r,g,b;

	b=color_rgb;
	color_rgb=color_rgb>>8;

	g=color_rgb;
	color_rgb=color_rgb>>8;

	r=color_rgb;

	putpixel_fbdev_lowlevel(x,y,r,g,b);	
}


void scrfbdev_putpixel_final(int x,int y,unsigned int color)
{

	int c;
	c=spectrum_colortable[color];

	scrfbdev_putpixel_final_rgb(x,y,c);

}


void scrfbdev_putpixel(int x,int y,unsigned int color)
{
    if (menu_overlay_activo==0) {
                //Putpixel con menu cerrado
                scrfbdev_putpixel_final(x,y,color);
                return;
        }          

        //Metemos pixel en layer adecuado
	buffer_layer_machine[y*ancho_layer_menu_machine+x]=color;        

        //Putpixel haciendo mix  
        screen_putpixel_mix_layers(x,y);   
}

void scrfbdev_special_scale_putpixel(int x,int y,unsigned int color)
{

	z80_byte r,g,b;
	int c;
	c=spectrum_colortable[color];

	b=c;
	c=c>>8;

	g=c;
	c=c>>8;

	r=c;


	int xdest=x*x_scale_factor_mult/x_scale_factor_div;

	int restox=(((x+1)*x_scale_factor_mult) / x_scale_factor_div)-xdest;


	int ydest=y*y_scale_factor_mult/y_scale_factor_div;

	int restoy=(((y+1)*y_scale_factor_mult) / y_scale_factor_div)-ydest;

	//putpixel_fbdev_lowlevel(xdest,ydest,r,g,b);

	int xdestorig=xdest;
	int restoxorig=restox;

	for (;restoy;restoy--,ydest++) {
		xdest=xdestorig;
		restox=restoxorig;

		for (;restox;restox--,xdest++) {
			putpixel_fbdev_lowlevel(xdest,ydest,r,g,b);
		}
	}

	//printf ("restox: %d \n",restox);
	//printf ("restoy: %d \n",restoy);

}



//Esto es equivalente a la funcion cfmakeraw(), pero por si acaso no existe en todos los unix
void fbdev_tty_makeraw(struct termios *termios_p) {

	//termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	termios_p->c_iflag &= ~(PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	termios_p->c_oflag &= ~OPOST;
	//termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG );
	termios_p->c_cflag &= ~(CSIZE | PARENB);
	termios_p->c_cflag |= CS8;
}


//Lo contrario de makeraw
/*
 * void fbdev_tty_makecooked(struct termios *termios_p)
 * {
 *	termios_p->c_iflag |= (IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
 *	termios_p->c_oflag |= OPOST;
 *	termios_p->c_lflag |= (ECHO | ECHONL | ICANON | ISIG | IEXTEN);
 *	termios_p->c_cflag |= (CSIZE | PARENB);
 *	termios_p->c_cflag &= ~CS8;
 * }
 */

int fbdev_init_tty(void)
{

	struct stat dummy;

	if (devices==NULL) {
		if (0 == stat("/dev/.devfsd",&dummy))
			devices = &devs_devfs;
		else
			devices = &devs_default;
	}


	//dev_init();
	fbdev_tty = 0;

	//normal
	int vt=0;

	//prueba usar otra virtual terminal
	//int vt=2;

	if (vt != 0) {
		if (fbdev_setvt(vt)) return 1;
	}

	struct vt_stat vts;

	if (-1 == ioctl(fbdev_tty,VT_GETSTATE, &vts)) {
		debug_printf(VERBOSE_ERR,"fbdev: ioctl VT_GETSTATE: %s (not a linux console?)",strerror(errno));
		return 1;
	}


	ioctl(fbdev_tty, KDSETMODE, KD_GRAPHICS);

	if (vt != 0) fbdev_activate_current(fbdev_tty);

	int flags;
	if((flags=fcntl(fbdev_tty,F_GETFL))==-1)
	{
		debug_printf(VERBOSE_ERR,"fbdev: couldn't get flags from tty device: %s",strerror(errno));
		return 1;
	}

	//no bloqueo de lectura
	flags &= ~O_NONBLOCK;


	//system ("/bin/stty raw");
	//Establecer raw keyboard, para no esperar enter


	if(fcntl(fbdev_tty,F_SETFL,flags)==-1)
	{
		debug_printf(VERBOSE_ERR,"fbdev: couldn't set tty device non-blocking: %s",strerror(errno));
		return 1;
	}

	struct termios termios_valores;


	if (tcgetattr(fbdev_tty,&termios_valores)==-1)
	{
		debug_printf(VERBOSE_ERR,"fbdev: couldn't set tty raw mode");
		return 1;
	}

	memcpy(&termios_valores_orig,&termios_valores,sizeof( struct termios ) );


	fbdev_tty_makeraw(&termios_valores);


	if (tcsetattr(fbdev_tty, TCSANOW, &termios_valores)==-1) {
		debug_printf(VERBOSE_ERR,"fbdev: couldn't set tty raw mode");
		return 1;
	}



	//Intentar meter terminal en raw mode, recibiendo press y release events
	//obtener
	if (fbdev_no_uses_ttyraw==0) {

		//printf ("poniendo terminal en modo teclado raw\n");

		if (ioctl(0, KDGKBMODE, &fbdev_initial_tty_mode)) {
			debug_printf(VERBOSE_ERR,"fbdev: Couldn't get tty mode");
		}

		else {

			//establecer
			if (ioctl(0, KDSKBMODE, K_RAW)) {
				debug_printf(VERBOSE_ERR,"fbdev: Couldn't set tty raw mode");
			}

			else {

				fbdev_sends_release.v=1;

				//cambiamos rutina de lectura de teclado
				scr_actualiza_tablas_teclado=scrfbdev_actualiza_tablas_teclado_rawmode;
			}
		}

	}


	//Fin inicializar tty
	return 0;

}

void scrfbdev_detectedchar_print(z80_byte caracter)
{

	//No enviar a pantalla

        //printf ("%c",caracter);
        //flush de salida standard
        //fflush(stdout);

	//para que el compilador no se queje de variable no usada
	caracter++;

}

//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrfbdev_get_menu_width(void)
{
        int max=screen_get_emulated_display_width_no_zoom_border_en()/menu_char_width/menu_gui_zoom;
        if (max>OVERLAY_SCREEN_MAX_WIDTH) max=OVERLAY_SCREEN_MAX_WIDTH;

                //printf ("max x: %d %d\n",max,screen_get_emulated_display_width_no_zoom_border_en());

        return max;
}


int scrfbdev_get_menu_height(void)
{
        int max=screen_get_emulated_display_height_no_zoom_border_en()/8/menu_gui_zoom;
        if (max>OVERLAY_SCREEN_MAX_HEIGTH) max=OVERLAY_SCREEN_MAX_HEIGTH;

                //printf ("max y: %d %d\n",max,screen_get_emulated_display_height_no_zoom_border_en());
        return max;
}

int scrfbdev_driver_can_ext_desktop (void)
{
        return 0;
}


//Fbdev video drivers
int scrfbdev_init (void){

	debug_printf (VERBOSE_INFO,"Init Fbdev Video Driver");

	scr_debug_registers=scrfbdev_debug_registers;
	scr_messages_debug=scrfbdev_messages_debug;

	scr_putchar_menu=scrfbdev_putchar_menu;
	scr_putchar_footer=scrfbdev_putchar_footer;

        scr_get_menu_width=scrfbdev_get_menu_width;
        scr_get_menu_height=scrfbdev_get_menu_height;	
	//scr_driver_can_ext_desktop=scrfbdev_driver_can_ext_desktop;


	scr_set_fullscreen=scrfbdev_set_fullscreen;
	scr_reset_fullscreen=scrfbdev_reset_fullscreen;
	scr_z88_cpc_load_keymap=scrfbdev_z88_cpc_load_keymap;
	scr_detectedchar_print=scrfbdev_detectedchar_print;


	//int ancho_minimo=ANCHO_PANTALLA+LEFT_BORDER_NO_ZOOM*2*border_enabled.v;
	//int alto_minimo=ALTO_PANTALLA+TOP_BORDER_NO_ZOOM*border_enabled.v+BOTTOM_BORDER_NO_ZOOM*border_enabled.v;

	int ancho_minimo=screen_get_window_size_width_no_zoom_border_en();
	int alto_minimo=screen_get_window_size_height_no_zoom_border_en();

	scr_putchar_zx8081=scrfbdev_putchar_zx8081;
	scr_tiene_colores=1;
	screen_refresh_menu=1;

	fbdev_sends_release.v=0;

	//en modo no raw es esta
	scr_actualiza_tablas_teclado=scrfbdev_actualiza_tablas_teclado;



	struct fb_var_screeninfo varinfo;
	struct fb_fix_screeninfo fixinfo;

	// Open the file for reading and writing
	fbdev_filedescriptor = open("/dev/fb0", O_RDWR);
	if (!fbdev_filedescriptor) {
		debug_printf(VERBOSE_ERR,"fbdev: Error: cannot open framebuffer device.\n");
		return 1;
	}

	//printf("The framebuffer device was opened successfully.\n");

	// Get fixed screen information
	if (ioctl(fbdev_filedescriptor, FBIOGET_FSCREENINFO, &fixinfo)) {
	//ponemos este mensaje en debug y no en error para que no se active menu y 
	//salga error cuando hace autodeteccion de driver de video. es especialmente molesto si el driver es stdout
	//mensaje de abajo de variable information tambien seria susceptible de cambiar a debug
		debug_printf(VERBOSE_DEBUG,"fbdev: Error reading fixed information.");
		return 1;
	}

	// Get variable screen information
	if (ioctl(fbdev_filedescriptor, FBIOGET_VSCREENINFO, &varinfo)) {
		debug_printf(VERBOSE_ERR,"fbdev: Error reading variable information.");
		return 1;
	}




	//Si estamos en fullscreen, en raspberry, cambiar resolucion de fbdev. Zoom 1 y ancho y alto segun si hay border o no
	#ifdef EMULATE_RASPBERRY
	if (ventana_fullscreen && fbdev_no_res_change.v==0) {
		zoom_x=zoom_y=1;

		//Con unos pixeles de mas de margen. Importante esto en minipantalla
		varinfo.xres=ancho_minimo+fbdev_margin_width;
		varinfo.yres=alto_minimo+fbdev_margin_height;
		//varinfo.xres=ancho_minimo;
		//varinfo.yres=alto_minimo;

		varinfo.bits_per_pixel=24;

		debug_printf (VERBOSE_INFO,"Setting framebuffer size to: %dx%d",varinfo.xres,varinfo.yres);

		// Set variable screen information
		if (ioctl(fbdev_filedescriptor, FBIOPUT_VSCREENINFO, &varinfo)) {
			debug_printf(VERBOSE_ERR,"fbdev: Error Setting variable information.");
			return 1;
		}


		// Get fixed screen information
		if (ioctl(fbdev_filedescriptor, FBIOGET_FSCREENINFO, &fixinfo)) {
			debug_printf(VERBOSE_ERR,"fbdev: Error reading fixed information.");
			return 1;
		}



	}
	#endif


	// map framebuffer to user memory
	fbdev_screensize = fixinfo.smem_len;


	if (fbdev_use_all_virtual_res) {
		//usar toda resolucion virtual
		fbdev_ancho=varinfo.xres_virtual;
		fbdev_alto=varinfo.yres_virtual;
	}

	else {

		//normal
		fbdev_ancho=varinfo.xres;
		fbdev_alto=varinfo.yres;
	}


	fbdev_line_length=fixinfo.line_length;


	bpp=varinfo.bits_per_pixel;

	debug_printf(VERBOSE_INFO,"fbdev info: real %dx%d, virtual %dX%d, %d bpp fbdev_screensize: %ld", varinfo.xres, varinfo.yres, varinfo.xres_virtual, varinfo.yres_virtual, bpp, fbdev_screensize );


	//Esta variable la leeremos posteriormente, al establecer putpixel_fbdev_lowlevel
	int color_rgb=0;


	//Solo asumimos que existe color RGB o BGR, nada mas
	//PC se comporta BGR. Raspberry se comporta RGB

	if (varinfo.red.offset==0) {
		debug_printf (VERBOSE_INFO,"Red colour component offset is 0. Assume color mode RGB");
		color_rgb=1;
	}

	else {
		debug_printf (VERBOSE_INFO,"Red colour component offset is not 0. Assume color mode BGR");
		color_rgb=0;
	}



	//normal putpixel en el caso de no fullscreen, o sea , no llenar toda la pantalla
	//sin zoom escalado especial, y luego en raspberry (y/o con opcion adicional) cambiar previamente la resolucion a la similar a spectrum
	scr_putpixel=scrfbdev_putpixel;

    scr_putpixel_final=scrfbdev_putpixel_final;
    scr_putpixel_final_rgb=scrfbdev_putpixel_final_rgb;	

	if (ventana_fullscreen && fbdev_decimal_full_scale_fbdev) {


		//putpixel con escalado "especial", o sea, no numero entero
		//en el caso de fullscreen, llenar toda la pantalla, zoom puede ser un factor decimal
		//o sea, --fullscreen hara que vaya mas lento. es casi mejor no hacer fullscreen, zoom 1 y en el caso de raspberry, se
		//ajustara la resolucion del framebuffer a la mas cercana con zoom 1
		//TODO: quiza hacer que si factores de multiplicacion son normales, hacer escalado normal, no este especial

		scr_putpixel=scrfbdev_special_scale_putpixel;

		zoom_x=zoom_y=1;
		set_putpixel_zoom();

		//x_scale_factor_mult=3;
		//x_scale_factor_div=2;

		x_scale_factor_mult=fbdev_ancho;
		x_scale_factor_div=ancho_minimo;

		//escalado y a factor
		//y_scale_factor_mult=4;
		//y_scale_factor_div=3;
		y_scale_factor_mult=fbdev_alto;
		y_scale_factor_div=alto_minimo;

		//proporcional:
		//y_scale_factor_mult=x_scale_factor_mult;
		//y_scale_factor_div=x_scale_factor_div;

	}


	//establecer consola grafica
	//Inicializar tty
	if (!fbdev_no_uses_tty) {
		if (fbdev_init_tty()) {
			//debug_printf (VERBOSE_ERR,"fbdev: Error initializing tty
			return 1;
		}
	}



	//validar zoom


	int zoom_max_x=fbdev_ancho/ancho_minimo;
	int zoom_max_y=fbdev_alto/alto_minimo;

	//printf ("zoom_max_x: %d zoom_max_y: %d \n",zoom_max_x,zoom_max_y);

	if (zoom_max_x==0 || zoom_max_y==0) {
		debug_printf (VERBOSE_ERR,"fbdev: No minimum size found");
		return 1;
	}

	if (zoom_x>zoom_max_x || zoom_y>zoom_max_y) {
		debug_printf (VERBOSE_ERR,"fbdev: zoom too big");
		zoom_x=zoom_y=1;
		set_putpixel_zoom();
	}

	//Obtener Orden componentes R G B Alpha
	debug_printf (VERBOSE_DEBUG,"Framebuffer: Red: Offset: %d Length: %d",varinfo.red.offset,varinfo.red.length);
	debug_printf (VERBOSE_DEBUG,"Framebuffer: Green: Offset: %d Length: %d",varinfo.green.offset,varinfo.green.length);
	debug_printf (VERBOSE_DEBUG,"Framebuffer: Blue: Offset: %d Length: %d",varinfo.blue.offset,varinfo.blue.length);
	debug_printf (VERBOSE_DEBUG,"Framebuffer: Alpha: Offset: %d Length: %d",varinfo.transp.offset,varinfo.transp.length);

	/*
	 *
	 * struct fb_bitfield {
	 *	__u32 offset;                    beginning of bitfield
	 *	__u32 length;                    length of bitfield
	 *	__u32 msb_right;                 != 0 : Most significant bit is
	 *					right
};


*/


	switch (bpp) {
		case 32:
			if (color_rgb) putpixel_fbdev_lowlevel=putpixel_fbdev_lowlevel_32bpp_rgb;
			else putpixel_fbdev_lowlevel=putpixel_fbdev_lowlevel_32bpp_bgr;
			break;

		case 24:
			if (color_rgb) putpixel_fbdev_lowlevel=putpixel_fbdev_lowlevel_24bpp_rgb;
			else putpixel_fbdev_lowlevel=putpixel_fbdev_lowlevel_24bpp_bgr;
			break;

		case 16:
			putpixel_fbdev_lowlevel=putpixel_fbdev_lowlevel_16bpp;
			break;

		default:
			debug_printf(VERBOSE_ERR,"fbdev: Bpp %d not supported\n",bpp);
			return 1;
			break;
	}



	fbdev_pointer = (char*)mmap(0,
				    fbdev_screensize,
			     PROT_READ | PROT_WRITE,
			     MAP_SHARED,
			     fbdev_filedescriptor, 0);

	if (fbdev_pointer == -1) {
		debug_printf(VERBOSE_ERR,"fbdev: Failed to mmap.\n");
		return 1;
	}

	else {


        //asignar la memoria del doble buffer
        if (fbdev_double_buffer_enabled.v) {
            double_buffer_pointer=malloc(fbdev_screensize);

            if (double_buffer_pointer==NULL) {
                debug_printf(VERBOSE_ERR,"fbdev: Can not allocate double buffer\n");
                return 1;
            }
        }

        else {
            double_buffer_pointer=fbdev_pointer;
        }



		//Borrar pantalla
		fbdev_cls();


		if (!ventana_fullscreen) {

			//centrar imagen. cambiar puntero inicial

			int offsetx,offsety,offset;

			//offset en pixeles de pantalla

			//int totalspectrumancho=ANCHO_PANTALLA*zoom_x+LEFT_BORDER*2*border_enabled.v;
			//int totalspectrumalto=ALTO_PANTALLA*zoom_y+TOP_BORDER*border_enabled.v+BOTTOM_BORDER*border_enabled.v;

			int totalspectrumancho=screen_get_window_size_width_zoom_border_en();
			int totalspectrumalto=screen_get_window_size_height_zoom_border_en();

			//offsetx=50;
			//offsety=50;

			offsetx=(fbdev_ancho-totalspectrumancho)/2;
			offsety=(fbdev_alto-totalspectrumalto)/2;

			offset=offsety*fbdev_line_length + offsetx*bpp/8;

			offset_centrado=offset;

		}

        else {
            offset_centrado=0;
        }



	}

	//printf("%dx%d, %d bpp fbdev_screensize: %ld\n", fbdev_ancho,fbdev_alto, bpp, fbdev_screensize );
	//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
	scr_set_driver_name("fbdev");

	scr_z88_cpc_load_keymap();

    scr_reallocate_layers_menu(fbdev_ancho,fbdev_alto);    	

	return 0;


}
