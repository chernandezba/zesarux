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

#include "cpu.h"
#include "scrxwindows.h"
#include "debug.h"
#include "screen.h"
#include "mem128.h"
#include "compileoptions.h"
#include "zx8081.h"
#include "z88.h"
#include "charset.h"
#include "zxvision.h"
#include "timer.h"
#include "utils.h"
//#include "operaciones.h"
#include "joystick.h"
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


#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>

#ifdef USE_XVIDMODE
//Para full screen
#include <X11/extensions/xf86vmode.h>

/*
	#if defined(__APPLE__)
//En apple no encuentro ese include con la estructura... aunque luego igualmente no tiene la extension XFree86-VidModeExtension y no puede hacer fullscreen
        typedef struct
        {
                unsigned long   flags;
                unsigned long   functions;
                unsigned long   decorations;
                long            inputMode;
                unsigned long   status;
        } PropMwmHints;

	#else
	#include <Xm/MwmUtil.h>
	#endif
*/


//La Xm/MwmUtil.h, presente en motif y openmotif en Linux, solo la uso para esta estructura, que pongo a continuacion,
//para que sea mas facil compilar y no tener que instalarla solo para eso. Es mas, en Mac OS X ni existe

        typedef struct
        {
                unsigned long   flags;
                unsigned long   functions;
                unsigned long   decorations;
                long            inputMode;
                unsigned long   status;
        } PropMwmHints;

#endif


int shm_used = 0;
static XImage *image = NULL;


#ifdef USE_XEXT
#define X_USE_SHM
#endif

#ifdef X_USE_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

static Visual *xdisplay_visual = NULL;
static int xdisplay_depth = -1;

//desactivar shm por linea de comandos
int disable_shm=0;


#ifdef X_USE_SHM
static XShmSegmentInfo shm_info;
int shm_eventtype;

static int try_shm( void );
static int get_shm_id( const int size );
#endif


int xdisplay_end (void);

int xerror_error;
int xerror_expecting;


int ultimo_resize_width=0,ultimo_resize_height=0;

int fullscreen_width,fullscreen_height;

//Resolucion pantalla antes de full screen
int screen_prefull_width,screen_prefull_height;


//Resoluciones de pantalla posibles - para fullscreen

#ifdef USE_XVIDMODE
XF86VidModeModeInfo **videomodes;
#endif


void scrxwindows_resize(int width,int height);



void scrxwindows_messages_debug(char *s)
{
	printf ("%s\n",s);
        //flush de salida standard. normalmente no hace falta esto, pero si ha finalizado el driver curses, deja la salida que no hace flush
        fflush(stdout);

}


//#define SIZE (256 * 192)

//;                               Bits:  4    3    2    1    0     ;desplazamiento puerto
//puerto_65278    db              255  ; V    C    X    Z    Sh    ;0
//puerto_65022    db              255  ; G    F    D    S    A     ;1
//puerto_64510    db              255  ; T    R    E    W    Q     ;2
//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7



//	z80_byte prueba_car=33;


      Display *dpy;


      Window ventana=0;


GC gc;


//Para poder capturar cierre de ventana cuando se le da a la "X" del titulo de ventana
static Atom delete_window_atom;


int scrxwindows_setwindowparms(void)
{

	//Para indicar los incrementos de tamaño de la ventana
	//asi la ventana se incrementa solo en tamaños multiples del tamaño de spectrum real
	//sino, el tamaño podria ser cualquiera
	//De momento solo se usa para eso, aunque se pueden establecer muchos otros parametros
	XWMHints *wmHints;
	XSizeHints *sizeHints;
	XClassHint *classHint;
	XTextProperty windowName, iconName;
	char *windowNameList="ZEsarUX",*iconNameList="ZEsarUX";


	if(!(wmHints = XAllocWMHints())) {
		debug_printf(VERBOSE_ERR,"failure allocating XAllocWMHints");
		return 1;
	}

	if(!(sizeHints = XAllocSizeHints())) {
		debug_printf(VERBOSE_ERR,"failure allocating XAllocSizeHints");
		return 1;
	}

	if(!(classHint = XAllocClassHint())) {
		debug_printf(VERBOSE_ERR,"failure allocating XAllocClassHint");
		return 1;
	}


	if(XStringListToTextProperty(&windowNameList,1,&windowName) == 0 ) {
		debug_printf(VERBOSE_ERR,"structure allocation for windowName failed");
		return 1;
	}

	if(XStringListToTextProperty(&iconNameList,1,&iconName) == 0 ) {
		debug_printf(VERBOSE_ERR,"structure allocation for iconName failed\n");
		return 1;
	}


	/* Set standard window properties */

	sizeHints->flags = PBaseSize | PResizeInc | PMaxSize | PMinSize;

	sizeHints->base_width = 0;
	sizeHints->base_height = 0;

	//Fijamos el minimo de tamaño de la ventana
	sizeHints->min_width = screen_get_window_size_width_no_zoom_border_en() + screen_get_ext_desktop_width_no_zoom();
	sizeHints->min_height = screen_get_window_size_height_no_zoom_border_en() + screen_get_ext_desktop_height_no_zoom();


	//Y se fijan los incrementos de la ventana, para que se amplie en fracciones enteras
	if (ventana_fullscreen==0) {
	        sizeHints->width_inc    = screen_get_window_size_width_no_zoom_border_en() + screen_get_ext_desktop_width_no_zoom();
	        sizeHints->height_inc   = screen_get_window_size_height_no_zoom_border_en() + screen_get_ext_desktop_height_no_zoom();
	}

	else {
		//en full screen ponemos el tamanyo de ventana a la que queramos
		sizeHints->width_inc    = 1;
		sizeHints->height_inc   = 1;
	}

	sizeHints->max_width    =     9999 * sizeHints->width_inc;
	sizeHints->max_height   =     9999 * sizeHints->height_inc;

	sizeHints->flags |= PAspect;
	sizeHints->min_aspect.x = 0;
	sizeHints->min_aspect.y = 1;    //Indica 0.1

	sizeHints->max_aspect.x = 100;
	sizeHints->max_aspect.y = 0;    //Indica 100.0

	wmHints->flags=StateHint | InputHint;

	wmHints->initial_state=NormalState;
	wmHints->input=True;

	classHint->res_name="ZEsarUX";
	classHint->res_class="ZEsarUX";

	XSetWMProperties(dpy, ventana, &windowName, &iconName, NULL, 0, sizeHints, wmHints, classHint);

	//Fin Para indicar los incrementos de tamaño de la ventana


	//Capturar cierre ventana

	delete_window_atom = XInternAtom( dpy, "WM_DELETE_WINDOW", 0 );
	XSetWMProtocols( dpy, ventana, &delete_window_atom, 1 );




	return 0;

}



//Atom wmDelete;




void scrxwindows_reset_fullscreen(void)
{

#ifdef USE_XVIDMODE
	debug_printf (VERBOSE_INFO,"Resetting fullscreen");
        ventana_fullscreen=0;


        //Activar decoraciones de ventana


        PropMwmHints hints;

        Atom    property;

        hints.flags = 2;        // Specify that we're changing the ventana decorations.
        hints.decorations = 1;  // true
        property = XInternAtom(dpy,"_MOTIF_WM_HINTS",True);
        XChangeProperty(dpy,ventana,property,property,32,PropModeReplace,(unsigned char *)&hints,5);


	//Liberar raton
        XMapRaised(dpy,ventana);

        XUngrabPointer(dpy,CurrentTime);

        XUngrabKeyboard(dpy,CurrentTime);


	//Poner resolucion anterior
	XF86VidModeSwitchToMode(dpy,DefaultScreen(dpy),videomodes[0] );


        //Ajustar parametros, concretamente el que indica incrementos de ventana, para ponerlo en modo normal
        scrxwindows_setwindowparms();




#else
        debug_printf(VERBOSE_ERR,"Full screen support not compiled");

#endif


}

void scrxwindows_set_fullscreen(void)
{



#ifdef USE_XVIDMODE
	// http://tonyobryan.com/index.php?article=9

	debug_printf (VERBOSE_INFO,"Setting full screen");


	//https://forums.geforce.com/default/topic/487759/xf86vidmodeswitchtomode-fails-when-changing-to-a-modeline-greater-than-the-current-one-i-39-ve-trie/


	int i  ;

	int vmMajor, vmMinor;

	int modeNum, bestMode;


	int screen;
	screen = DefaultScreen(dpy);


	/* set best mode to current */

	bestMode = 0;

	/* get a connection */






	int vm_event, vm_error;

	//Comprobar si existe esa extension
	if (!XF86VidModeQueryExtension(dpy, &vm_event, &vm_error)) {
		debug_printf (VERBOSE_ERR,"No extension XFree86-VidModeExtension found. Can't switch to full screen");
		return;
	}




	XF86VidModeQueryVersion(dpy, &vmMajor, &vmMinor);

	debug_printf(VERBOSE_INFO,"XF86 VideoMode extension version %d.%d", vmMajor, vmMinor);





	XF86VidModeGetAllModeLines(dpy, screen, &modeNum, &videomodes);




	int bestwidth,bestheight;
	bestMode=-1;
	bestwidth=99999;
	bestheight=99999;

	//prueba a zoom 2. Quiza mejor asi porque el alto total (296 ) multiplicado por dos da 594, que es casi 600, y por tanto,
	//la resolucion que mejor se ajusta es 800x600
	int zoom_futuro_x,zoom_futuro_y;
	zoom_futuro_x=zoom_futuro_y=2;

	int widthspectrum;
	int heightspectrum;

	//Resolucion de ventana
        widthspectrum=screen_get_window_size_width_no_zoom_border_en()*zoom_futuro_x;
				widthspectrum +=(screen_get_ext_desktop_width_no_zoom()*zoom_futuro_x);

        heightspectrum=screen_get_window_size_height_no_zoom_border_en()*zoom_futuro_y;
        heightspectrum +=(screen_get_ext_desktop_height_no_zoom()*zoom_futuro_y);



	//int width=widthspectrum;
	//int height=heightspectrum;

//fullscreen_width,fullscreen_height

	/* save desktop-resolution before switching modes */


	//TODO
	//Supuestamente el modo actual esta en
	//*videomodes[0]
	//Pero esto no me funciona. Lo unico que funciona bien es obtener la resolucion actual mediante DisplayWidth,  etc
	//Obtener resolucion actual
	//Luego la resolucion actual no la encuentro en ninguno de los *videomodes[]...
    	screen_prefull_width  = DisplayWidth( dpy, screen );
	screen_prefull_height = DisplayHeight( dpy, screen );

	//debug_printf(VERBOSE_INFO,"Current video mode: %d x %d",screen_prefull_width,screen_prefull_height);

	/* look for mode with best resolution */
	debug_printf(VERBOSE_INFO,"minimum desired size: %d x %d",widthspectrum,heightspectrum);

	for (i = 0; i < modeNum; i++)

	{

		int leidowidth,leidoheight;

		leidowidth=videomodes[i]->hdisplay;
		leidoheight=videomodes[i]->vdisplay;

		debug_printf(VERBOSE_INFO,"mode %d: %4d x %4d",i,leidowidth,leidoheight);


		//Si modo con mayor o igual resolucion
		if (leidowidth >= widthspectrum && leidoheight >=heightspectrum) {

			//Y si ese modo es menor que el best
			if (leidowidth < bestwidth && leidoheight < bestheight) {
				debug_printf(VERBOSE_INFO,"mode %i is better",i);
				bestMode = i;

				bestwidth=leidowidth;
				bestheight=leidoheight;
			}
		}



	}


	if (bestMode==-1) {
		debug_printf(VERBOSE_ERR,"No valid modes found");
		return;
	}

	zoom_x=zoom_futuro_x;
	zoom_y=zoom_futuro_y;

	set_putpixel_zoom();



	debug_printf(VERBOSE_INFO,"best mode: %d %4d x %4d",bestMode,bestwidth,bestheight);


	//


	//para que sea pantalla completa, tamanyo de ventana tiene que ser el mismo que pantalla
	fullscreen_width=bestwidth;
	fullscreen_height=bestheight;


	ventana_fullscreen=1;


	//Eliminar decoraciones de ventana


	PropMwmHints hints;

	Atom    property;

	hints.flags = 2;        // Specify that we're changing the ventana decorations.
	hints.decorations = 0;  // 0 (false) means that ventana decorations should go bye-bye.
	property = XInternAtom(dpy,"_MOTIF_WM_HINTS",True);
	XChangeProperty(dpy,ventana,property,property,32,PropModeReplace,(unsigned char *)&hints,5);



	//Ajustar parametros, concretamente el que indica incrementos de ventana, para que podamos poner el tamaño que queramos
	scrxwindows_setwindowparms();


	//Llamamos aqui porque necesita asignar algunas estructuras de memoria... sino petara con segmentation fault
	scrxwindows_resize(fullscreen_width,fullscreen_height);


	XF86VidModeSwitchToMode(dpy,DefaultScreen(dpy),videomodes[bestMode] );

	XMoveResizeWindow(dpy,ventana,0,0,fullscreen_width,fullscreen_height);



        //Ajustar centro de la ventana
        int xcentro = (fullscreen_width-widthspectrum)/2;
        int ycentro = (fullscreen_height-heightspectrum)/2;
	debug_printf(VERBOSE_INFO,"Center: x %d y %d",xcentro,ycentro);

        XF86VidModeSetViewPort(dpy,DefaultScreen(dpy),xcentro,ycentro);



	//Mostrar ventana y capturar teclado y mouse
	XMapRaised(dpy,ventana);

	XGrabPointer(dpy,ventana,True,0,GrabModeAsync,GrabModeAsync,ventana,0L,CurrentTime);

	XGrabKeyboard(dpy,ventana,False,GrabModeAsync,GrabModeAsync,CurrentTime);


#else
	debug_printf(VERBOSE_ERR,"Full screen support not compiled");

#endif

}

void scrxwindows_alloc_image(int width,int height)
{

#ifdef X_USE_SHM
if (!disable_shm)
  shm_used = try_shm();
#endif                          /* #ifdef X_USE_SHM */


if( !shm_used ) {

	//free mem if needed
	if (image!=NULL) {
		debug_printf(VERBOSE_INFO,"Free non shm image data");
		free(image->data);
	}


    image = XCreateImage( dpy, xdisplay_visual,
                       xdisplay_depth, ZPixmap, 0, NULL, width,height,8,0);

    if(!image) {
      debug_printf(VERBOSE_ERR,"Couldn't create image");
      exit(1);
    }

    if( ( image->data = malloc( image->bytes_per_line *
                                                 image->height ) ) == NULL ) {
      debug_printf(VERBOSE_ERR," Out of memory for image data");
      exit(1);
    }
        //printf ("despues xcreateimage image=%x\n",image);



}

else {
debug_printf (VERBOSE_INFO,"Using X11 Shared memory");
//printf ("con shm dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
}


}

void scrxwindows_resize(int width,int height)
{

	if (!ventana) return;

	clear_putpixel_cache();

	int zoom_x_calculado,zoom_y_calculado;

	debug_printf (VERBOSE_INFO,"Xwindows resize");

	if (ventana_fullscreen) {
		zoom_x_calculado=zoom_x;
		zoom_y_calculado=zoom_y;

		modificado_border.v=1;



                debug_printf (VERBOSE_INFO,"Calling XResizeWindow on fullscreen");

                scrxwindows_alloc_image(fullscreen_width,fullscreen_height);

                XResizeWindow( dpy, ventana, fullscreen_width, fullscreen_height);

		XMapRaised(dpy,ventana);

	//printf ("resize %d %d\n",width,height);
	scr_reallocate_layers_menu(fullscreen_width,fullscreen_height);	


		return;

	}



	debug_printf (VERBOSE_INFO,"width: %d get_window_width: %d height: %d get_window_height: %d",width,screen_get_window_size_width_no_zoom_border_en(),height,screen_get_window_size_height_no_zoom_border_en());

		//zoom_x_calculado=width/screen_get_window_size_width_no_zoom_border_en();
		zoom_x_calculado=width/(screen_get_window_size_width_no_zoom_border_en()+screen_get_ext_desktop_width_no_zoom() );
		zoom_y_calculado=height/(screen_get_window_size_height_no_zoom_border_en()+screen_get_ext_desktop_height_no_zoom() );


	if (!zoom_x_calculado) zoom_x_calculado=1;
	if (!zoom_y_calculado) zoom_y_calculado=1;

	debug_printf (VERBOSE_INFO,"zoom_x: %d zoom_y: %d zoom_x_calculated: %d zoom_y_calculated: %d",zoom_x,zoom_y,zoom_x_calculado,zoom_y_calculado);

	if (zoom_x_calculado!=zoom_x || zoom_y_calculado!=zoom_y) {
		//resize
		zoom_x=zoom_x_calculado;
		zoom_y=zoom_y_calculado;
		set_putpixel_zoom();
		modificado_border.v=1;

                width=screen_get_window_size_width_zoom_border_en();
								width+=screen_get_ext_desktop_width_zoom();

                height=screen_get_window_size_height_zoom_border_en();
                height+=screen_get_ext_desktop_height_zoom();


		debug_printf (VERBOSE_INFO,"Calling XResizeWindow to %d X %d",width,height);

		scrxwindows_alloc_image(width,height);

		XResizeWindow( dpy, ventana, width, height);

	}

	//printf ("resize %d %d\n",width,height);
	scr_reallocate_layers_menu(width,height);

}


void scrxwindows_debug_registers(void)
{

char buffer[2048];
print_registers(buffer);

printf ("%s\r",buffer);

}


/*


The whole matter becomes at good deal clearer if we look at the screen address in binary.


           High Byte                |               Low Byte

0   1   0   T   T   L   L   L          Cr Cr Cr Cc Cc Cc Cc Cc



I have used some abbreviations to make things a bit clearer:

T - these two bits refer to which third of the screen is being addressed:  00 - Top,  01 - Middle,    10 - Bottom

L - these three bits indicate which line is being addressed:  from 0 - 7, or 000 - 111 in binary

Cr - these three bits indicate which character row is being addressed:  from 0 - 7

Cc - these five bits refer to which character column is being addressed: from 0 - 31


The  top three bits ( 010 ) of the high byte don't change.


*/

//Funcion de poner pixel en pantalla de driver, teniendo como entrada el color en RGB
void scrxwindows_putpixel_final_rgb(int x,int y,unsigned int color_rgb)
{
	XPutPixel(image,x,y,color_rgb);	
}

void scrxwindows_putpixel_final(int x,int y,unsigned int color)
{
	XPutPixel(image,x,y,spectrum_colortable[color]);
}

void scrxwindows_putpixel(int x,int y,unsigned int color)
{

	if (menu_overlay_activo==0) {
                //Putpixel con menu cerrado
                scrxwindows_putpixel_final(x,y,color);
                return;
  }          

  //Metemos pixel en layer adecuado
	buffer_layer_machine[y*ancho_layer_menu_machine+x]=color;        

  //Putpixel haciendo mix  
  screen_putpixel_mix_layers(x,y);   


}

void scrxwindows_refresca_border(void)
{
	scr_refresca_border();

}


void scrxwindows_putchar_zx8081(int x,int y, z80_byte caracter)
{
	scr_putchar_zx8081_comun(x,y, caracter);
}



//Rutina de putchar para menu
void scrxwindows_putchar_menu(int x,int y, z80_byte caracter,int tinta,int papel)
{

        z80_bit inverse;

        inverse.v=0;

	//128 y 129 corresponden a franja de menu y a letra enye minuscula
        if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) caracter='?';

        scr_putchar_menu_comun_zoom(caracter,x,y,inverse,tinta,papel,menu_gui_zoom);


}

//Rutina de putchar para footer window
void scrxwindows_putchar_footer(int x,int y, z80_byte caracter,int tinta,int papel) {


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




void scrxwindows_refresca_pantalla_zx81(void)
{

scr_refresca_pantalla_y_border_zx8081();

}

void scrxwindows_refresca_pantalla_solo_driver(void)
{
   //Dibujar normal toda la pantalla entera

	 int ancho=screen_get_window_size_width_zoom_border_en();

	 ancho +=screen_get_ext_desktop_width_zoom();

	 int alto=screen_get_window_size_height_zoom_border_en();

	 alto +=screen_get_ext_desktop_height_zoom();     

        if( shm_used ) {

#ifdef X_USE_SHM
                //printf ("con shm dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
                //printf ("image=%x\n",image);

                XShmPutImage(dpy, ventana, gc, image, 0, 0, 0, 0, ancho, alto, True);

                //temp probar para ver si esto detiene el uso de cpu incrementandose
                //XSync(dpy, False);

                //o tambien probar esto
                //XFlush(dpy);

    // FIXME: should wait for an ShmCompletion event here

#endif


        }

        else {
                //printf ("sin shm dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
                XPutImage(dpy, ventana, gc, image, 0, 0, 0, 0, ancho, alto );

        }

        /*
        Hacemos flush de todo lo enviado a la pantalla. La docu de XFlush dice:
        The XFlush() function flushes the output buffer. Most client applications need not use this function because 
        the output buffer is automatically flushed as needed by calls to XPending(), XNextEvent(), and XWindowEvent(). 
        Events generated by the server may be enqueued into the library's event queue.

        Me parece perfecto... si no llamo a XFlush lo hace automáticamente, es cierto. PERO lo hace cuando le da la gana,
        y entonces sucede que se ven parpadeos en el menu en los recuadros de ventana, en waveform, en aysheet... en monton de sitios,
        porque en esas ventanas hay texto por ejemplo en blanco que va debajo y luego dibujo encima pixeles, pero como lo refresca
        cuando quiere (si no hay XFlush) eso produce los parpadeos, de mostrar momentos intermedios en que aún no he finalizado de dibujar
        completamente la ventana

        */
        XFlush(dpy);

}

void scrxwindows_refresca_pantalla(void)
{


        if (sem_screen_refresh_reallocate_layers) {
                //printf ("--Screen layers are being reallocated. return\n");
                //debug_exec_show_backtrace();
                return;
        }

        sem_screen_refresh_reallocate_layers=1;



	if (MACHINE_IS_ZX8081) {


		//scr_refresca_pantalla_rainbow_comun();
	        scrxwindows_refresca_pantalla_zx81();
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
				        scrxwindows_refresca_border();
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

	//printf ("%d\n",spectrum_colortable[1]);

	screen_render_menu_overlay_if_active();

	//Escribir footer
	draw_middle_footer();

	scrxwindows_refresca_pantalla_solo_driver();



sem_screen_refresh_reallocate_layers=0;

}


void scrxwindows_end(void)
{

	debug_printf (VERBOSE_INFO,"Closing xwindows video driver");

	//Si hay pantalla completa, desactivarla
	if (ventana_fullscreen) scrxwindows_reset_fullscreen();

	xdisplay_end();
}



//;                    Bits:  4    3    2    1    0     ;desplazamiento puerto
//puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
//puerto_65022   db    255  ; G    F    D    S    A     ;1
//puerto_64510    db              255  ; T    R    E    W    Q     ;2
//puerto_63486    db              255  ; 5    4    3    2    1     ;3
//puerto_61438    db              255  ; 6    7    8    9    0     ;4
//puerto_57342    db              255  ; Y    U    I    O    P     ;5
//puerto_49150    db              255  ; H    J    K    L    Enter ;6
//puerto_32766    db              255  ; B    N    M    Simb Space ;7

//Puertos de teclado Z88
/*
-------------------------------------------------------------------------
         | D7     D6      D5      D4      D3      D2      D1      D0
-------------------------------------------------------------------------
A15 (#7) | RSH    SQR     ESC     INDEX   CAPS    .       /       £
A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
A13 (#5) | [      SPACE   1       Q       A       Z       L       0
A12 (#4) | ]      LFT     2       W       S       X       M       P
A11 (#3) | -      RGT     3       E       D       C       K       9
A10 (#2) | =      DWN     4       R       F       V       J       O
A9  (#1) | \      UP      5       T       G       B       U       I
A8  (#0) | DEL    ENTER   6       Y       H       N       7       8
-------------------------------------------------------------------------


Disposicion del teclado Z88

ESC  1 2 3 4 5 6 7 8 9 0  -_  =+  \|  DEL
TAB q w e r t y u i o p [{  ]}
DIA  a s d f g h j k l ;: '" libra~ ENTER
SHIFT z x c v b n m ,< .> /? SHIFT   UP
INDEX MENU HELP SQU SPACE  CAPSLOCK  LEFT RIGHT DOWN


Teclado PC english
`~ 1 2 3 4 5 6 7 8 9 0  -_  =+  BACKSPACE
TAB q w e r t y u i o p [{ ]}  \|
CAPS a s d f g h j k l  ;:  '"  ENTER
SHIFT z x c v b n m  ,<  .>  /?  SHIFT UP
CTRL ALT SPACE ALT CTRL

Teclado PC Spanish
ºª 1 2 3 4 5 6 7 8 9 0 '?  ¡¿  BACKSPACE
TAB q w e r t y u i o p `^  +*
CAPS a s d f g h j k l  ñ  '"  çÇ  ENTER
SHIFT <>  z x c v b n m  ,;  .:  -_  SHIFT
CTRL ALT SPACE ALT CTRL


*/

//Teclas de Z88 asociadas a cada tecla del teclado fisico
KeySym scrxwindows_keymap_z88_cpc_minus;
KeySym scrxwindows_keymap_z88_cpc_equal;
KeySym scrxwindows_keymap_z88_cpc_backslash;
KeySym scrxwindows_keymap_z88_cpc_bracket_left;
KeySym scrxwindows_keymap_z88_cpc_bracket_right;
KeySym scrxwindows_keymap_z88_cpc_semicolon;
KeySym scrxwindows_keymap_z88_cpc_apostrophe;
KeySym scrxwindows_keymap_z88_cpc_pound;
KeySym scrxwindows_keymap_z88_cpc_comma;
KeySym scrxwindows_keymap_z88_cpc_period;
KeySym scrxwindows_keymap_z88_cpc_slash;

KeySym scrxwindows_keymap_z88_cpc_circunflejo;
KeySym scrxwindows_keymap_z88_cpc_colon;
KeySym scrxwindows_keymap_z88_cpc_arroba;
KeySym scrxwindows_keymap_z88_cpc_leftz; //Tecla a la izquierda de la Z. Solo usada en Chloe


void scrxwindows_z88_cpc_load_keymap(void)
{

	debug_printf (VERBOSE_INFO,"Loading keymap");

	//Teclas se ubican en misma disposicion fisica del Z88, excepto:
	//libra~ -> spanish: cedilla (misma ubicacion fisica del z88). english: acento grave (supuestamente a la izquierda del 1)
	//backslash: en english esta en fila inferior del z88. en spanish, lo ubicamos a la izquierda del 1 (ºª\)

	switch (z88_cpc_keymap_type) {

		case 1:
			if (MACHINE_IS_Z88 || MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI)  {
				scrxwindows_keymap_z88_cpc_minus=XK_apostrophe;
				scrxwindows_keymap_z88_cpc_equal=XK_exclamdown;
				scrxwindows_keymap_z88_cpc_backslash=XK_masculine;

				scrxwindows_keymap_z88_cpc_bracket_left=XK_dead_grave;
				scrxwindows_keymap_z88_cpc_bracket_right=XK_plus;
				scrxwindows_keymap_z88_cpc_semicolon=XK_ntilde;
				scrxwindows_keymap_z88_cpc_apostrophe=XK_dead_acute;
				scrxwindows_keymap_z88_cpc_pound=XK_ccedilla;
				scrxwindows_keymap_z88_cpc_comma=XK_comma;
				scrxwindows_keymap_z88_cpc_period=XK_period;
				scrxwindows_keymap_z88_cpc_slash=XK_minus;
				scrxwindows_keymap_z88_cpc_leftz=XK_less; //Tecla a la izquierda de la Z. Solo usada en Chloe
			}




			else if (MACHINE_IS_CPC) {
                        	scrxwindows_keymap_z88_cpc_minus=XK_apostrophe;
	                        scrxwindows_keymap_z88_cpc_circunflejo=XK_exclamdown;

        	                scrxwindows_keymap_z88_cpc_arroba=XK_dead_grave;
                	        scrxwindows_keymap_z88_cpc_bracket_left=XK_plus;
	                        scrxwindows_keymap_z88_cpc_colon=XK_ntilde;
        	                scrxwindows_keymap_z88_cpc_semicolon=XK_dead_acute;
                	        scrxwindows_keymap_z88_cpc_bracket_right=XK_ccedilla;
                        	scrxwindows_keymap_z88_cpc_comma=XK_comma;
	                        scrxwindows_keymap_z88_cpc_period=XK_period;
        	                scrxwindows_keymap_z88_cpc_slash=XK_minus;

                	        scrxwindows_keymap_z88_cpc_backslash=XK_masculine;
				scrxwindows_keymap_z88_cpc_leftz=XK_less; //Tecla a la izquierda de la Z. Solo usada en Chloe
		        }

			else if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
				scrxwindows_keymap_z88_cpc_minus=XK_apostrophe;
                                scrxwindows_keymap_z88_cpc_equal=XK_exclamdown;
                                scrxwindows_keymap_z88_cpc_backslash=XK_masculine;

                                scrxwindows_keymap_z88_cpc_bracket_left=XK_dead_grave;
                                scrxwindows_keymap_z88_cpc_bracket_right=XK_plus;
                                scrxwindows_keymap_z88_cpc_semicolon=XK_ntilde;
                                scrxwindows_keymap_z88_cpc_apostrophe=XK_dead_acute;
                                scrxwindows_keymap_z88_cpc_pound=XK_ccedilla;
                                scrxwindows_keymap_z88_cpc_comma=XK_comma;
                                scrxwindows_keymap_z88_cpc_period=XK_period;
                                scrxwindows_keymap_z88_cpc_slash=XK_minus;
                                scrxwindows_keymap_z88_cpc_leftz=XK_less; //Tecla a la izquierda de la Z. Solo usada en Chloe
                        }


		break;


		default:
			//0 o default
			if (MACHINE_IS_Z88 || MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI)  {
				scrxwindows_keymap_z88_cpc_minus=XK_minus;
				scrxwindows_keymap_z88_cpc_equal=XK_equal;
				scrxwindows_keymap_z88_cpc_backslash=XK_backslash;

				scrxwindows_keymap_z88_cpc_bracket_left=XK_bracketleft;
				scrxwindows_keymap_z88_cpc_bracket_right=XK_bracketright;
				scrxwindows_keymap_z88_cpc_semicolon=XK_semicolon;
				scrxwindows_keymap_z88_cpc_apostrophe=XK_apostrophe;
				scrxwindows_keymap_z88_cpc_pound=XK_grave;
				scrxwindows_keymap_z88_cpc_comma=XK_comma;
				scrxwindows_keymap_z88_cpc_period=XK_period;
				scrxwindows_keymap_z88_cpc_slash=XK_slash;
                                scrxwindows_keymap_z88_cpc_leftz=XK_less; //Tecla a la izquierda de la Z. Solo usada en Chloe
			}

			else if (MACHINE_IS_CPC) {
				scrxwindows_keymap_z88_cpc_minus=XK_minus;
                                scrxwindows_keymap_z88_cpc_circunflejo=XK_equal;

                                scrxwindows_keymap_z88_cpc_arroba=XK_bracketleft;
                                scrxwindows_keymap_z88_cpc_bracket_left=XK_bracketright;
                                scrxwindows_keymap_z88_cpc_colon=XK_semicolon;
                                scrxwindows_keymap_z88_cpc_semicolon=XK_apostrophe;
                                scrxwindows_keymap_z88_cpc_bracket_right=XK_grave;
                                scrxwindows_keymap_z88_cpc_comma=XK_comma;
                                scrxwindows_keymap_z88_cpc_period=XK_period;
                                scrxwindows_keymap_z88_cpc_slash=XK_slash;
                                scrxwindows_keymap_z88_cpc_backslash=XK_backslash;
                                scrxwindows_keymap_z88_cpc_leftz=XK_less; //Tecla a la izquierda de la Z. Solo usada en Chloe
			}

			else if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
				scrxwindows_keymap_z88_cpc_minus=XK_minus;
                                scrxwindows_keymap_z88_cpc_circunflejo=XK_equal;

                                scrxwindows_keymap_z88_cpc_arroba=XK_bracketleft;
                                scrxwindows_keymap_z88_cpc_bracket_left=XK_bracketright;
                                scrxwindows_keymap_z88_cpc_colon=XK_semicolon;
                                scrxwindows_keymap_z88_cpc_semicolon=XK_apostrophe;
                                scrxwindows_keymap_z88_cpc_bracket_right=XK_grave;
                                scrxwindows_keymap_z88_cpc_comma=XK_comma;
                                scrxwindows_keymap_z88_cpc_period=XK_period;
                                scrxwindows_keymap_z88_cpc_slash=XK_slash;
                                scrxwindows_keymap_z88_cpc_backslash=XK_backslash;
                                scrxwindows_keymap_z88_cpc_leftz=XK_less; //Tecla a la izquierda de la Z. Solo usada en Chloe
			}
		break;
	}
}




void deal_with_keys(XEvent *event,int pressrelease)
{
	KeySym keysym;
	//char tecla;


	KeySym native;

	//printf ("event->xkey :%d\n",event->xkey);

	native = XLookupKeysym( &event->xkey, 0 );
	//aunque esto es una union y se podria poner como native = XLookupKeysym( event, 0 );   pero retorna un warning

	keysym=native;
	//printf ("keysym: 0x%lX\n",native);

	//Suport for numbers on azerty keyboard. You can get them without having to press shift
	if (azerty_keyboard.v) {
		switch (keysym) {
			case XK_ampersand:
				keysym = XK_1;
			break;

			case XK_eacute:
				keysym = XK_2;
		        break;

			case XK_quotedbl:
				keysym = XK_3;
		        break;

			case XK_apostrophe:
			        keysym = XK_4;
			break;

			case XK_parenleft:
			        keysym = XK_5;
		        break;

			case XK_minus:
		                keysym = XK_6;
		        break;

			case XK_egrave:
		                keysym = XK_7;
		        break;

			case XK_underscore:
				keysym = XK_8;
		        break;

			case XK_ccedilla:
				keysym = XK_9;
		        break;

			case XK_agrave:
			        keysym = XK_0;
			break;
		}
	}

     int tecla_gestionada_chloe=0;
        if (MACHINE_IS_SPECTRUM && chloe_keyboard.v) {
                        tecla_gestionada_chloe=1;

                        if (keysym==scrxwindows_keymap_z88_cpc_minus) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_MINUS,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_equal) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_EQUAL,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_backslash) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BACKSLASH,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_left) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BRACKET_LEFT,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_right) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_BRACKET_RIGHT,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_semicolon) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_SEMICOLON,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_apostrophe) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_APOSTROPHE,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_pound) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_POUND,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_comma) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_COMMA,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_period) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_PERIOD,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_slash) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_SLASH,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_leftz) util_set_reset_key_chloe_keymap(UTIL_KEY_CHLOE_LEFTZ,pressrelease);

                        else tecla_gestionada_chloe=0;
        }


        if (tecla_gestionada_chloe) return;



        int tecla_gestionada_sam_ql=0;
        if (MACHINE_IS_SAM || MACHINE_IS_QL || MACHINE_IS_MSX || MACHINE_IS_SVI) {
                tecla_gestionada_sam_ql=1;

                        if (keysym==scrxwindows_keymap_z88_cpc_minus) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_MINUS,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_equal) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_EQUAL,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_backslash) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BACKSLASH,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_left) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BRACKET_LEFT,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_right) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_BRACKET_RIGHT,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_semicolon) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_SEMICOLON,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_apostrophe) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_APOSTROPHE,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_pound) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_POUND,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_comma) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_COMMA,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_period) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_PERIOD,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_slash) util_set_reset_key_common_keymap(UTIL_KEY_COMMON_KEYMAP_SLASH,pressrelease);


                else tecla_gestionada_sam_ql=0;
        }

        if (tecla_gestionada_sam_ql) return;



	switch (keysym) {

			case 32:
				util_set_reset_key(UTIL_KEY_SPACE,pressrelease);
			break;

			case XK_Return:
				util_set_reset_key(UTIL_KEY_ENTER,pressrelease);
			break;

			case XK_Shift_L:
				util_set_reset_key(UTIL_KEY_SHIFT_L,pressrelease);
			break;

                        case XK_Shift_R:
				joystick_possible_rightshift_key(pressrelease);
                        break;

                        case XK_Alt_L:
				util_set_reset_key(UTIL_KEY_ALT_L,pressrelease);
			break;
                        case XK_Alt_R:
				joystick_possible_rightalt_key(pressrelease);
                        break;


			case XK_Control_L:
				util_set_reset_key(UTIL_KEY_CONTROL_L,pressrelease);
                        break;

			case XK_Control_R:
				joystick_possible_rightctrl_key(pressrelease);
                        break;

			case XK_Super_L:
			case XK_Super_R:
				util_set_reset_key(UTIL_KEY_WINKEY,pressrelease);
                        break;

			case XK_Delete:
				util_set_reset_key(UTIL_KEY_DEL,pressrelease);
                        break;

			//Teclas que generan doble pulsacion
			case XK_BackSpace:
				util_set_reset_key(UTIL_KEY_BACKSPACE,pressrelease);
                        break;

			case XK_Home:
				joystick_possible_home_key(pressrelease);
                        break;

			case XK_Left:
				util_set_reset_key(UTIL_KEY_LEFT,pressrelease);
                        break;

                        case XK_Right:
				util_set_reset_key(UTIL_KEY_RIGHT,pressrelease);
                        break;

                        case XK_Down:
				util_set_reset_key(UTIL_KEY_DOWN,pressrelease);
                        break;

                        case XK_Up:
				util_set_reset_key(UTIL_KEY_UP,pressrelease);
                        break;

			case XK_Tab:
				util_set_reset_key(UTIL_KEY_TAB,pressrelease);
                        break;

			case XK_Caps_Lock:
				util_set_reset_key(UTIL_KEY_CAPS_LOCK,pressrelease);
                        break;

			case XK_comma:
				util_set_reset_key(UTIL_KEY_COMMA,pressrelease);
                        break;

			case XK_period:
				util_set_reset_key(UTIL_KEY_PERIOD,pressrelease);
                        break;

			case XK_minus:
			case XK_KP_Subtract:
				util_set_reset_key(UTIL_KEY_MINUS,pressrelease);
                        break;

			case XK_plus:
			case XK_KP_Add:
				util_set_reset_key(UTIL_KEY_PLUS,pressrelease);
                        break;

                        case XK_slash:
                        case XK_KP_Divide:
				util_set_reset_key(UTIL_KEY_SLASH,pressrelease);
                        break;

                        case XK_asterisk:
                        case XK_KP_Multiply:
				util_set_reset_key(UTIL_KEY_ASTERISK,pressrelease);
                        break;



			//F1 pulsado
                        case XK_F1:
				util_set_reset_key(UTIL_KEY_F1,pressrelease);
                        break;

			//F2 pulsado
                        case XK_F2:
				util_set_reset_key(UTIL_KEY_F2,pressrelease);
                        break;

			//F3 pulsado
                        case XK_F3:
				util_set_reset_key(UTIL_KEY_F3,pressrelease);
                        break;

			//F4 pulsado
                        case XK_F4:
				util_set_reset_key(UTIL_KEY_F4,pressrelease);
                        break;

                        //F5 pulsado
                        case XK_F5:
                                util_set_reset_key(UTIL_KEY_F5,pressrelease);
                        break;


                        //F6 pulsado
                        case XK_F6:
                                util_set_reset_key(UTIL_KEY_F6,pressrelease);
                        break;

                        //F7 pulsado
                        case XK_F7:
                                util_set_reset_key(UTIL_KEY_F7,pressrelease);
                        break;


			//F8 pulsado. osdkeyboard
			case XK_F8:
				util_set_reset_key(UTIL_KEY_F8,pressrelease);
                        break;

			//F9 pulsado. quickload
			case XK_F9:
				util_set_reset_key(UTIL_KEY_F9,pressrelease);
                        break;


                        //F10 pulsado
                        case XK_F10:
				util_set_reset_key(UTIL_KEY_F10,pressrelease);
                        break;


                        //F11 pulsado
                        case XK_F11:
                                util_set_reset_key(UTIL_KEY_F11,pressrelease);
                        break;

                        //F12 pulsado
                        case XK_F12:
                                util_set_reset_key(UTIL_KEY_F12,pressrelease);
                        break;

                        //F13 pulsado
                        case XK_F13:
                                util_set_reset_key(UTIL_KEY_F13,pressrelease);
                        break;

                        //F14 pulsado
                        case XK_F14:
                                util_set_reset_key(UTIL_KEY_F14,pressrelease);
                        break;

                        //F15 pulsado
                        case XK_F15:
                                util_set_reset_key(UTIL_KEY_F15,pressrelease);
                        break;

			//ESC pulsado
			case XK_Escape:
				util_set_reset_key(UTIL_KEY_ESC,pressrelease);
			break;

			//PgUp
                        case XK_Page_Up:
				util_set_reset_key(UTIL_KEY_PAGE_UP,pressrelease);
                        break;

                        //PgDn
                        case XK_Page_Down:
				util_set_reset_key(UTIL_KEY_PAGE_DOWN,pressrelease);
                        break;


			//Teclas del keypad
			case XK_KP_Insert:
                                util_set_reset_key(UTIL_KEY_KP0,pressrelease);
                        break;

                        case XK_KP_End:
                                util_set_reset_key(UTIL_KEY_KP1,pressrelease);
                        break;

                        case XK_KP_Down:
                                util_set_reset_key(UTIL_KEY_KP2,pressrelease);
                        break;

                        case XK_KP_Page_Down:
                                util_set_reset_key(UTIL_KEY_KP3,pressrelease);
                        break;

                        case XK_KP_Left:
                                util_set_reset_key(UTIL_KEY_KP4,pressrelease);
                        break;

                        case XK_KP_Begin:
                                util_set_reset_key(UTIL_KEY_KP5,pressrelease);
                        break;

                        case XK_KP_Right:
                                util_set_reset_key(UTIL_KEY_KP6,pressrelease);
                        break;

                        case XK_KP_Home:
                                util_set_reset_key(UTIL_KEY_KP7,pressrelease);
                        break;

                        case XK_KP_Up:
                                util_set_reset_key(UTIL_KEY_KP8,pressrelease);
                        break;

                        case XK_KP_Page_Up:
                                util_set_reset_key(UTIL_KEY_KP9,pressrelease);
                        break;

                        case XK_KP_Delete:
                                util_set_reset_key(UTIL_KEY_KP_COMMA,pressrelease);
                        break;

                        case XK_KP_Enter:
                                util_set_reset_key(UTIL_KEY_KP_ENTER,pressrelease);
                        break;




			default:
				//convert_numeros_letras_puerto_teclado(keysym,pressrelease);
				if (keysym<256) util_set_reset_key(keysym,pressrelease);
			break;

		}


	//Fuera del switch

//Teclas que necesitan conversion de teclado para CPC
        if (MACHINE_IS_CPC) {

                        if (keysym==scrxwindows_keymap_z88_cpc_minus) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_MINUS,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_circunflejo) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_CIRCUNFLEJO,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_arroba) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_ARROBA,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_left) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BRACKET_LEFT,pressrelease);



                        else if (keysym==scrxwindows_keymap_z88_cpc_colon) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_COLON,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_semicolon) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_SEMICOLON,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_right) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BRACKET_RIGHT,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_comma) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_COMMA,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_period) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_PERIOD,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_slash) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_SLASH,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_backslash) util_set_reset_key_cpc_keymap(UTIL_KEY_CPC_BACKSLASH,pressrelease);


        }


//Teclas que necesitan conversion de teclado para Z88
	if (!MACHINE_IS_Z88) return;

                        if (keysym==scrxwindows_keymap_z88_cpc_minus) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_MINUS,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_equal) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_EQUAL,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_backslash) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BACKSLASH,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_left) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BRACKET_LEFT,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_bracket_right) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_BRACKET_RIGHT,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_semicolon) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_SEMICOLON,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_apostrophe) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_APOSTROPHE,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_pound) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_POUND,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_comma) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_COMMA,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_period) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_PERIOD,pressrelease);

                        else if (keysym==scrxwindows_keymap_z88_cpc_slash) util_set_reset_key_z88_keymap(UTIL_KEY_Z88_SLASH,pressrelease);





}


void scrxwindows_actualiza_tablas_teclado(void)
{


	XEvent event;
	//quiza lo mejor es mediante eventos sincronos, de tal manera que cuando se pulse una tecla se actualice la tabla...
//	printf ("lee\n ");
        //XNextEvent(dpy,&event);



	//Capturar cierre ventana
	//Desactivado porque con esto, el menu sin multitask no va
	//quiza porque en ese caso, la lectura de teclas se hace "asincrona" y no se lee... o no se libera nunca tecla... hay que revisarlo
	//por eso de momento esta desactivado. aunque no se captura evento de cierre aqui, pero si que se envia una señal,
	//y al no capturarse, al darle con el raton a la X de cierre de ventana, no hara nada
	/*
	XNextEvent(dpy, &event);

      if (event.type == ClientMessage &&
          event.xclient.data.l[0] == delete_window_atom)
	{
		debug_printf (VERBOSE_INFO,"Received window close event");
		//Hacemos que aparezca el menu con opcion de salir del emulador
		menu_abierto=1;
		menu_button_exit_emulator.v=1;

      }
	*/
	//Fin capturar cierre ventana




	while (XCheckWindowEvent(dpy,ventana,KeyPressMask|KeyReleaseMask|StructureNotifyMask|ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask,&event)) {
	  //printf ("evento tipo: %d\n",event.type);
          switch(event.type){

		//Mouse
		case ButtonPress:

			if ( event.xbutton.button == 1 ) {
				//mouse_left=1;
				util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,1);
			}
			if ( event.xbutton.button == 3 ) {
				//mouse_right=1;
				util_set_reset_mouse(UTIL_MOUSE_RIGHT_BUTTON,1);
			}

			//Botones 4 y 5 en X11 es scroll arriba y abajo... ciertamente un tanto confuso
     		if ( event.xbutton.button == 4 ) {
                mouse_wheel_vertical=1;
            }

            if ( event.xbutton.button == 5 ) {
                mouse_wheel_vertical=-1;
            }			

			gunstick_x=event.xbutton.x;
			gunstick_y=event.xbutton.y;
			gunstick_x=gunstick_x/zoom_x;
			gunstick_y=gunstick_y/zoom_y;

			debug_printf (VERBOSE_PARANOID,"Mouse Button press. x=%d y=%d. gunstick: x: %d y: %d", event.xbutton.x, event.xbutton.y,gunstick_x,gunstick_y);

		break;

                case ButtonRelease:
                        debug_printf (VERBOSE_PARANOID,"Mouse Button release. x=%d y=%d", event.xbutton.x, event.xbutton.y);
                        if ( event.xbutton.button == 1 ) {
				//mouse_left=0;
				util_set_reset_mouse(UTIL_MOUSE_LEFT_BUTTON,0);
			}
                        if ( event.xbutton.button == 3 ) {
				//mouse_right=0;
				util_set_reset_mouse(UTIL_MOUSE_RIGHT_BUTTON,0);
			}

			//mouse_left=0;

                break;

		case MotionNotify:
		    mouse_x=event.xbutton.x;
		    mouse_y=event.xbutton.y;



		    kempston_mouse_x=mouse_x/zoom_x;
		    kempston_mouse_y=255-mouse_y/zoom_y;
		    //printf("Mouse is at (%d,%d)\n", kempston_mouse_x, kempston_mouse_y);

			debug_printf (VERBOSE_PARANOID,"Mouse motion. X: %d Y:%d kempston x: %d y: %d",mouse_x,mouse_y,kempston_mouse_x,kempston_mouse_y);
		break;


		case KeyPress:
	//		printf ("key press ");
			notificar_tecla_interrupcion_si_z88();
	            deal_with_keys(&event,1);
            break;

		case KeyRelease:
	//		printf ("release key ");
		    deal_with_keys(&event,0);
		break;

		case ConfigureNotify:
			//      xdisplay_configure_notify(event.xconfigure.width,
                        //        event.xconfigure.height);
			debug_printf(VERBOSE_INFO,"XWindows event ConfigureNotify width: %d height: %d",event.xconfigure.width,event.xconfigure.height);

		//ver si tamaño es el mismo que el actual, no hacer resize
		//en mac os x pasa que esto se queda en un bucle muchas veces al redimensionar y se queda continuamente ampliando y reduciendo

//                if (ultimo_resize_width!=event.xconfigure.width || ultimo_resize_height!=event.xconfigure.height) {
                        ultimo_resize_width=event.xconfigure.width;
                        ultimo_resize_height=event.xconfigure.height;

			scrxwindows_resize(event.xconfigure.width,event.xconfigure.height);

			//Redibujar zona inferior Z88 si conviene
			screen_z88_draw_lower_screen();
			menu_init_footer();

			//si en menu y no esta multitask, hay que refrescar a mano
			if (menu_abierto && !menu_multitarea) {
				debug_printf (VERBOSE_INFO,"Force refresh screen because in menu and multitask is disabled");
				scr_refresca_pantalla();
			}


			//vaciar eventos. para evitar que en mac se quede en bucle
			while (XCheckWindowEvent(dpy,ventana,KeyPressMask|KeyReleaseMask|StructureNotifyMask|ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask,&event));
			//TODO: quiza en vez de esto lo mejor seria poner una pausa aqui...
  //              }
    //            else {
      //                  debug_printf (VERBOSE_INFO,"Not resizing window so size is the same");
        //        }

		break;

		case ExposureMask:
			debug_printf(VERBOSE_INFO,"XWindows event ExposureMask x: %d y: %d height: %d width: %d",event.xexpose.x, event.xexpose.y, event.xexpose.width, event.xexpose.height);
		break;

		//case ClientMessage:
		//
		//	if (event.xclient.message_type == wm_protocols && event.xclient.data.l[0] == wm_delete_window) {
		//		end_emulator();
		//	}
		//break;

	  }
	}
	//printf("\n");

}


//Esta funcion estaba pensada para leer tecla al momento de leer el puerto... Esto de momento se hace a cada final de frame de pantalla
z80_byte scrxwindows_lee_puerto(z80_byte puerto_h,z80_byte puerto_l)
{

        //Para evitar warnings al compilar de "unused parameter"
        puerto_h=puerto_l;
        puerto_l=puerto_h;


//de momento nada
	return 255;
}


static int xdisplay_find_visual (void)
{
  XVisualInfo visual_tmpl;
  XVisualInfo *vis;
  int nvis, i;
  int sel_v = -1;
  int sel_v_depth = -1;
  int sel_v_class = -1;

int xui_screenNum = DefaultScreen(dpy);


  visual_tmpl.screen = xui_screenNum;
  vis = XGetVisualInfo( dpy,
                             VisualScreenMask,
                             &visual_tmpl, &nvis );
  if( vis != NULL ) {
    for( i = 0; i < nvis; i++ ) {
            /*
             * Save the visual index and its depth, if this is the first
             * truecolor visual, or a visual that is 'preferred' over the
             * previous 'best' visual.
             */
      if( ( sel_v_depth == -1 && vis[i].depth >= 4 ) || /* depth >= 4  */
          ( vis[i].depth > sel_v_depth &&
                vis[i].depth <= 16 ) ||                 /* depth up to 16 */
          ( vis[i].depth <= 8 && vis[i].depth == sel_v_depth &&
                vis[i].class != sel_v_class &&
                    vis[i].class == PseudoColor ) || /* indexed changable palette */
          ( vis[i].depth > 8 && vis[i].depth == sel_v_depth &&
                vis[i].class != sel_v_class &&
                    vis[i].class == TrueColor ) /* decomposed constatant colors */
      ) {
        sel_v = i;
        sel_v_depth = vis[i].depth;
        sel_v_class = vis[i].class;
      }
    }

    if ( sel_v != -1 ) {
      xdisplay_visual = vis[sel_v].visual;
      xdisplay_depth = vis[sel_v].depth;
    }
XFree(vis);
  }
  return sel_v == -1 ? 1 : 0;
}




#define NIL (0)       // A name for the void pointer



void scrxwindows_hide_mouse_pointer(void)
{


	Cursor invisibleCursor;
	Pixmap bitmapNoData;
	XColor black;
	static char noData[] = { 0,0,0,0,0,0,0,0 };
	black.red = black.green = black.blue = 0;

	bitmapNoData = XCreateBitmapFromData(dpy, ventana, noData, 8, 8);
	invisibleCursor = XCreatePixmapCursor(dpy, bitmapNoData, bitmapNoData,
                                     &black, &black, 0, 0);
	XDefineCursor(dpy,ventana, invisibleCursor);
	XFreeCursor(dpy, invisibleCursor);

}

void scrxwindows_detectedchar_print(z80_byte caracter)
{
        printf ("%c",caracter);
        //flush de salida standard
        fflush(stdout);

}


//Estos valores no deben ser mayores de OVERLAY_SCREEN_MAX_WIDTH y OVERLAY_SCREEN_MAX_HEIGTH
int scrxwindows_get_menu_width(void)
{
        int max=screen_get_emulated_display_width_no_zoom_border_en();

        max +=screen_get_ext_desktop_width_no_zoom();

        max=max/menu_char_width/menu_gui_zoom;


        if (max>OVERLAY_SCREEN_MAX_WIDTH) max=OVERLAY_SCREEN_MAX_WIDTH;

                //printf ("max x: %d %d\n",max,screen_get_emulated_display_width_no_zoom_border_en());

        return max;
}


int scrxwindows_get_menu_height(void)
{
        int max=screen_get_emulated_display_height_no_zoom_border_en();

        max +=screen_get_ext_desktop_height_no_zoom();

        max=max/menu_char_height/menu_gui_zoom;

        if (max>OVERLAY_SCREEN_MAX_HEIGTH) max=OVERLAY_SCREEN_MAX_HEIGTH;

                //printf ("max y: %d %d\n",max,screen_get_emulated_display_height_no_zoom_border_en());
        return max;
}


int scrxwindows_driver_can_ext_desktop (void)
{
        return 1;
}


int scrxwindows_init (void) {

	debug_printf (VERBOSE_INFO,"Init XWindows Video Driver");

	// Open the display

	dpy = XOpenDisplay(NIL);

	if (!dpy) {
		debug_printf (VERBOSE_ERR,"xwindows driver. Error opening display");
		return 1;
	}
	//assert(dpy);

	// Get some colors

	int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

	// Create the window

	//Esto asignarlo antes para que el driver vea correctamente el tamaño
	scr_driver_can_ext_desktop=scrxwindows_driver_can_ext_desktop;

int ancho,alto;
ancho=screen_get_window_size_width_zoom_border_en();

ancho +=screen_get_ext_desktop_width_zoom();

alto=screen_get_window_size_height_zoom_border_en();

alto +=screen_get_ext_desktop_height_zoom();

	ventana = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, ancho, alto,0, blackColor, blackColor);

	//printf ("crear ventana %d %d\n",screen_get_window_size_width_zoom_border_en(), screen_get_window_size_height_zoom_border_en() );
	debug_printf (VERBOSE_INFO,"Create XWindows Window %d X %d",ancho,alto);



	if (scrxwindows_setwindowparms()) return 1;


	// We want to get MapNotify events

	XSelectInput(dpy, ventana, KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	// "Map" the window (that is, make it appear on the screen)

	XMapWindow(dpy, ventana);

	// Create a "Graphics Context"

	gc = XCreateGC(dpy, ventana, 0, NIL);

	// Tell the GC we draw using the white color

	XSetForeground(dpy, gc, whiteColor);

	// Wait for the MapNotify event

	for(;;) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    if (e.type == MapNotify)
		  break;
	}

	if ( xdisplay_find_visual() ) exit(1);









	scrxwindows_alloc_image(ancho, alto);

scr_reallocate_layers_menu(ancho,alto);

	if (!shm_used) debug_printf (VERBOSE_WARN,"No X11 Shared memory. Expect poor performance");


	XFlush(dpy);

	char *list[2];
	char buffer[20];
	XTextProperty text;

	list[0] = buffer;
	list[1] = 0;

	sprintf( buffer, "ZEsarUX "EMULATOR_VERSION );

	XStringListToTextProperty( list, 1, &text);
	XSetWMName( dpy, ventana, &text );



	//Inicializaciones necesarias
	scr_putpixel=scrxwindows_putpixel;
  scr_putpixel_final=scrxwindows_putpixel_final;
  scr_putpixel_final_rgb=scrxwindows_putpixel_final_rgb;
        scr_get_menu_width=scrxwindows_get_menu_width;
        scr_get_menu_height=scrxwindows_get_menu_height;	
	

	scr_putchar_zx8081=scrxwindows_putchar_zx8081;
        scr_debug_registers=scrxwindows_debug_registers;
        scr_messages_debug=scrxwindows_messages_debug;
	scr_putchar_menu=scrxwindows_putchar_menu;
	scr_putchar_footer=scrxwindows_putchar_footer;
	scr_set_fullscreen=scrxwindows_set_fullscreen;
	scr_reset_fullscreen=scrxwindows_reset_fullscreen;
	scr_z88_cpc_load_keymap=scrxwindows_z88_cpc_load_keymap;
	scr_detectedchar_print=scrxwindows_detectedchar_print;
	scr_tiene_colores=1;
	screen_refresh_menu=1;


	//Esta funcion quiza no iba antes, pero en un xorg reciente funciona
	if (mouse_pointer_shown.v==0) {
		debug_printf (VERBOSE_INFO,"Hiding mouse pointer");
		scrxwindows_hide_mouse_pointer();
	}


	//Otra inicializacion necesaria
	//Esto debe estar al final, para que funcione correctamente desde menu, cuando se selecciona un driver, y no va, que pueda volver al anterior
	scr_set_driver_name("xwindows");


        scr_z88_cpc_load_keymap();


	if (ventana_fullscreen) {
		scr_set_fullscreen();
	}

	return 0;


}



#ifdef X_USE_SHM
static int try_shm (void)
{
  int id;
  int error;

  if( !XShmQueryExtension( dpy ) ) return 0;

  shm_eventtype = XShmGetEventBase( dpy ) + ShmCompletion;

int ancho=screen_get_window_size_width_zoom_border_en();
ancho +=screen_get_ext_desktop_width_zoom();

int alto=screen_get_window_size_height_zoom_border_en();
alto +=screen_get_ext_desktop_height_zoom();

image = XShmCreateImage( dpy, xdisplay_visual,
                           xdisplay_depth, ZPixmap,
                           NULL, &shm_info,
ancho, alto );


/*
   we allocate extra space after the screen for status bar icons
   status bar icons total width always smaller than 3xDISPLAY_ASPECT_WIDTH
*/
  if( !image ) return 0;

  /* Get an SHM to work with */
  id = get_shm_id( image->bytes_per_line * image->height );
  if( id == -1 ) return 0;

  /* Attempt to attach to the shared memory */
  shm_info.shmid = id;
  image->data = shm_info.shmaddr = shmat( id, 0, 0 );

  /* If we couldn't attach, remove the chunk and give up */
  if( image->data == (void*)-1 ) {
    shmctl( id, IPC_RMID, NULL );
    image->data = NULL;
    return 0;
  }

  /* This may generate an X error */
  xerror_error = 0; xerror_expecting = 1;
  error = !XShmAttach( dpy, &shm_info );

/* Force any X errors to occur before we disable traps */
  XSync( dpy, False );
  xerror_expecting = 0;

  /* If we caught an error, don't use SHM */
  if( error || xerror_error ) {
    printf ("shm error. disabling\n");
    shmctl( id, IPC_RMID, NULL );
    shmdt( image->data ); image->data = NULL;
    return 0;
  }

  /* Now flag the chunk for deletion; this will take effect when
     everything has detached from it */
  shmctl( id, IPC_RMID, NULL );

  return 1;
}

/* Get an SHM ID; also attempt to reclaim any stale chunks we find */
static int get_shm_id (const int size)
{
  key_t key = 'Z' << 24 | 'E' << 16 | 'U' << 8 | 'X';
  struct shmid_ds shm;

  int id;

  int pollution = 5;

  do {
    /* See if a chunk already eimagests with this key */
    id = shmget( key, size, 0777 );

    /* If the chunk didn't already eimagest, try and create one for our
       use */
    if( id == -1 ) {
      id = shmget( key, size, IPC_CREAT | 0777 );
      continue;                 /* And then jump to the end of the loop */
    }
    /* If the chunk already eimagests, try and get information about it */
    if( shmctl( id, IPC_STAT, &shm ) != -1 ) {

      /* If something's actively using this chunk, try another key */
      if( shm.shm_nattch ) {
        key++;
      } else {          /* Otherwise, attempt to remove the chunk */

        /* If we couldn't remove that chunk, try another key. If we
           could, just try again */
        if( shmctl( id, IPC_RMID, NULL ) != 0 ) key++;
      }
    } else {            /* Couldn't get info on the chunk, so try next key */
      key++;
    }

    id = -1;            /* To prevent early eimaget from loop */

  } while( id == -1 && --pollution );

  return id;
}
#endif                  /* #ifdef X_USE_SHM */





static void xdisplay_destroy_image (void)
{
  /* Free the XImage used to store screen data; also frees the malloc'd
     data */
#ifdef X_USE_SHM
  if( shm_used ) {
    XShmDetach( dpy, &shm_info );
    shmdt( shm_info.shmaddr );
    image->data = NULL;
    shm_used = 0;
  }
#endif
  if( image ) XDestroyImage( image ); image = NULL;
}



int xdisplay_end (void)
{
  xdisplay_destroy_image();
  /* Free the allocated GC */
  if( gc ) XFreeGC( dpy, gc ); 
  gc = 0;

  XDestroyWindow(dpy,ventana);
  XCloseDisplay(dpy);

  return 0;
}




