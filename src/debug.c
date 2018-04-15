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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#ifdef linux
#include <execinfo.h>
#endif



#include "cpu.h"
#include "debug.h"
#include "mem128.h"
#include "screen.h"
#include "menu.h"
#include "zx8081.h"
#include "operaciones.h"
#include "core_spectrum.h"
#include "core_zx8081.h"
#include "core_z88.h"
#include "core_ace.h"
#include "core_cpc.h"
#include "core_sam.h"
#include "core_ql.h"
#include "disassemble.h"
#include "utils.h"
#include "prism.h"

#include "spectra.h"
#include "tbblue.h"
#include "zxuno.h"
#include "ulaplus.h"
#include "timex.h"
#include "ay38912.h"
#include "ula.h"
#include "ql.h"
#include "m68k.h"
#include "superupgrade.h"
#include "core_mk14.h"
#include "scmp.h"

#include "dandanator.h"
#include "multiface.h"
#include "chloe.h"
#include "cpc.h"
#include "sam.h"

#include "snap.h"
#include "kartusho.h"
#include "diviface.h"
#include "betadisk.h"

#include "tsconf.h"

#include "core_reduced_spectrum.h"

#include "remote.h"
#include "charset.h"


struct timeval debug_timer_antes, debug_timer_ahora;

int verbose_level=0;


z80_bit menu_breakpoint_exception={0};

z80_bit debug_breakpoints_enabled={0};

//breakpoints de direcciones PC. Si vale -1, no hay breakpoint
//int debug_breakpoints_array[MAX_BREAKPOINTS];

//breakpoints de condiciones
char debug_breakpoints_conditions_array[MAX_BREAKPOINTS_CONDITIONS][MAX_BREAKPOINT_CONDITION_LENGTH];

//acciones a ejecutar cuando salta un breakpoint
char debug_breakpoints_actions_array[MAX_BREAKPOINTS_CONDITIONS][MAX_BREAKPOINT_CONDITION_LENGTH];

//A 0 si ese breakpoint no ha saltado. A 1 si ya ha saltado
int debug_breakpoints_conditions_saltado[MAX_BREAKPOINTS_CONDITIONS];

//A 1 si ese breakpoint esta activado. A 0 si no
int debug_breakpoints_conditions_enabled[MAX_BREAKPOINTS_CONDITIONS];

//Comportamiento al saltar un breakpoint:
//0: salta el breakpoint siempre que se cumpla condicion
//1: salta el breakpoint cuando la condicion pasa de false a true
z80_bit debug_breakpoints_cond_behaviour={1};



//Para hacer breakpoints de lectura de direcciones (cuando se hace peek de alguna direccion). Si vale -1, no hay breakpoint
//int debug_breakpoints_peek_array[MAX_BREAKPOINTS_PEEK];

//Punteros a las funciones originales
//z80_byte (*peek_byte_no_time_no_debug)(z80_int dir);
//z80_byte (*peek_byte_no_debug)(z80_int dir);
//void (*poke_byte_no_time_no_debug)(z80_int dir,z80_byte valor);
//void (*poke_byte_no_debug)(z80_int dir,z80_byte valor);

z80_byte (*lee_puerto_no_debug)(z80_byte puerto_h,z80_byte puerto_l);
void (*out_port_no_debug)(z80_int puerto,z80_byte value);


/*Variables de lectura/escritura en direcciones/puertos
Memory/port
Read/write
Address/value
ejemplo : MRA: memory read address
*/


z80_byte debug_mmu_mrv; //Memory Read Value (valor leido en peek)
z80_byte debug_mmu_mwv; //Memory Write Value (valor escrito en poke)
z80_byte debug_mmu_prv; //Port Read Value (valor leido en lee_puerto)
z80_byte debug_mmu_pwv; //Port Write Value (valor escrito en out_port)

z80_int debug_mmu_mra; //Memory Read Addres (direccion usada peek)
z80_int debug_mmu_mwa; //Memory Write Address (direccion usada en poke)
z80_int debug_mmu_pra; //Port Read Address (direccion usada en lee_puerto)
z80_int debug_mmu_pwa; //Port Write Address (direccion usada en out_port)

//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
int debug_exitrom=0;


//Mensaje que ha hecho saltar el breakpoint
char catch_breakpoint_message[MAX_MESSAGE_CATCH_BREAKPOINT];

//Id indice breakpoint que ha saltado
int catch_breakpoint_index=0;


//Core loop actual
int cpu_core_loop_active;

//puntero a cpu core actual
void (*cpu_core_loop) (void);

//nombre identificativo del core. De momento solo usado para mostrar nombre en comando de remote command
char *cpu_core_loop_name=NULL;

//puntero a cpu core actual, sin degug, usado en la funcion de debug
//void (*cpu_core_loop_no_debug) (void);

//si se hace debug en el core (para breakpoints, y otras condiciones)
//z80_bit debug_cpu_core_loop={0};

void cpu_core_loop_debug_check_breakpoints(void);

void debug_dump_nested_print(char *string_inicial, char *string_to_print);


//empieza en el 163
char *spectrum_rom_tokens[]={
"SPECTRUM","PLAY",
"RND","INKEY$","PI","FN","POINT","SCREEN$","ATTR","AT","TAB",
"VAL$","CODE","VAL","LEN","SIN","COS","TAN","ASN","ACS",
"ATN","LN","EXP","INT","SQR","SGN","ABS","PEEK","IN",
"USR","STR$","CHR$","NOT","BIN","OR","AND","<=",">=",
"<>","LINE","THEN","TO","STEP","DEF FN","CAT","FORMAT","MOVE",
"ERASE","OPEN #","CLOSE #","MERGE","VERIFY","BEEP","CIRCLE","INK","PAPER",
"FLASH","BRIGHT","INVERSE","OVER","OUT","LPRINT","LLIST","STOP","READ",
"DATA","RESTORE","NEW","BORDER","CONTINUE","DIM","REM","FOR","GO TO",
"GO SUB","INPUT","LOAD","LIST","LET","PAUSE","NEXT","POKE","PRINT",
"PLOT","RUN","SAVE","RANDOMIZE","IF","CLS","DRAW","CLEAR","RETURN","COPY"
};



char *zx81_rom_tokens[]={
//estos a partir de 192
"\"","AT","TAB","?","CODE","VAL","LEN","SIN","COS","TAN","ASN","ACS",
"ATN","LN","EXP","INT","SQR","SGN","ABS","PEEK","USR",
"STR$","CHR$","NOT","**"," OR"," AND","<=",">=","<>",
" THEN"," TO"," STEP"," LPRINT"," LLIST"," STOP",
" SLOW"," FAST"," NEW"," SCROLL"," CONT"," DIM",
" REM"," FOR"," GOTO"," GOSUB"," INPUT"," LOAD",
" LIST"," LET"," PAUSE"," NEXT"," POKE"," PRINT",
" PLOT"," RUN"," SAVE"," RAND"," IF"," CLS"," UNPLOT"," CLEAR"," RETURN"," COPY",

//estos en el 64,65,66
"RND","INKEY$","PI"

};

char *zx80_rom_tokens[]={
"THEN","TO",";", "," ,")" ,"(","NOT","-","+",
"*","/","AND","OR","**","=","<",">","LIST",
"RETURN","CLS","DIM","SAVE","FOR","GO TO",
"POKE","INPUT","RANDOMISE","LET","'?'",
"'?'","NEXT","PRINT","'?'","NEW",
"RUN","STOP","CONTINUE","IF","GO SUB","LOAD",
"CLEAR","REM","?"
};
 
//Rutina auxiliar que pueden usar los drivers de video para mostrar los registros. Mete en una string los registros
void print_registers(char *buffer)
{

  if (CPU_IS_SCMP) {
    char buffer_flags[9];
    scmp_get_flags_letters(scmp_m_SR,buffer_flags);
    sprintf (buffer,"PC=%04x P1=%04x P2=%04x P3=%04x AC=%02x ER=%02x SR=%s",

      get_pc_register(),scmp_m_P1.w.l,scmp_m_P2.w.l,scmp_m_P3.w.l,
      scmp_m_AC, scmp_m_ER,buffer_flags);

  }

  else if (CPU_IS_MOTOROLA) {

unsigned int registro_sr=m68k_get_reg(NULL, M68K_REG_SR);

      sprintf (buffer,"PC: %05X SP: %05X USP: %05X SR: %04X : %c%c%c%c%c%c%c%c%c%c "
            "A0: %08X A1: %08X A2: %08X A3: %08X A4: %08X A5: %08X A6: %08X A7: %08X "
            "D0: %08X D1: %08X D2: %08X D3: %08X D4: %08X D5: %08X D6: %08X D7: %08X "



      ,get_pc_register(),m68k_get_reg(NULL, M68K_REG_SP),m68k_get_reg(NULL, M68K_REG_USP),registro_sr,
      (registro_sr&32768 ? 'T' : ' '),
      (registro_sr&8192  ? 'S' : ' '),
      (registro_sr&1024  ? '2' : ' '),
      (registro_sr&512   ? '1' : ' '),
      (registro_sr&256   ? '0' : ' '),
          (registro_sr&16 ? 'X' : ' '),
          (registro_sr&8  ? 'N' : ' '),
          (registro_sr&4  ? 'Z' : ' '),
          (registro_sr&2  ? 'V' : ' '),
          (registro_sr&1  ? 'C' : ' '),
          m68k_get_reg(NULL, M68K_REG_A0),m68k_get_reg(NULL, M68K_REG_A1),m68k_get_reg(NULL, M68K_REG_A2),m68k_get_reg(NULL, M68K_REG_A3),
          m68k_get_reg(NULL, M68K_REG_A4),m68k_get_reg(NULL, M68K_REG_A5),m68k_get_reg(NULL, M68K_REG_A6),m68k_get_reg(NULL, M68K_REG_A7),
          m68k_get_reg(NULL, M68K_REG_D0),m68k_get_reg(NULL, M68K_REG_D1),m68k_get_reg(NULL, M68K_REG_D2),m68k_get_reg(NULL, M68K_REG_D3),
          m68k_get_reg(NULL, M68K_REG_D4),m68k_get_reg(NULL, M68K_REG_D5),m68k_get_reg(NULL, M68K_REG_D6),m68k_get_reg(NULL, M68K_REG_D7)

    );
  }

  else {
  sprintf (buffer,"PC=%04x SP=%04x BC=%04x A=%02x HL=%04x DE=%04x IX=%04x IY=%04x A'=%02x BC'=%04x HL'=%04x DE'=%04x I=%02x R=%02x  "
                  "F=%c%c%c%c%c%c%c%c F'=%c%c%c%c%c%c%c%c MEMPTR=%04x IM%d %s %s VPS: %d ",
  reg_pc,reg_sp, (reg_b<<8)|reg_c,reg_a,(reg_h<<8)|reg_l,(reg_d<<8)|reg_e,reg_ix,reg_iy,reg_a_shadow,(reg_b_shadow<<8)|reg_c_shadow,
  (reg_h_shadow<<8)|reg_l_shadow,(reg_d_shadow<<8)|reg_e_shadow,reg_i,(reg_r&127)|(reg_r_bit7&128),DEBUG_STRING_FLAGS,
  DEBUG_STRING_FLAGS_SHADOW,memptr,im_mode,( iff1.v ? "IFF1" : "   "),( iff2.v ? "IFF2" : "   "),last_vsync_per_second
                        );
  }

			//printf ("128k. p32765=%d p8189=%d\n\r",puerto_32765,puerto_8189);


}


//Para poder saltar los step-to-step
//Evitar en step to step las rutinas de interrupciones maskable/nmi
//z80_bit debug_core_evitamos_inter={0};


//Se ha entrado en una rutina de maskable/nmi
z80_bit debug_core_lanzado_inter={0};

//Valores registro PC de retorno
z80_int debug_core_lanzado_inter_retorno_pc_nmi=0;
z80_int debug_core_lanzado_inter_retorno_pc_maskable=0;


void init_breakpoints_table(void)
{
	int i;

	//for (i=0;i<MAX_BREAKPOINTS;i++) debug_breakpoints_array[i]=-1;

	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
		debug_breakpoints_conditions_array[i][0]=0;
    debug_breakpoints_actions_array[i][0]=0;
		debug_breakpoints_conditions_saltado[i]=0;
		debug_breakpoints_conditions_enabled[i]=0;
	}

        //for (i=0;i<MAX_BREAKPOINTS_PEEK;i++) debug_breakpoints_peek_array[i]=-1;


}





//Dibuja la pantalla de panico
void screen_show_panic_screen(int xmax, int ymax)
{
    //rojo, amarillo, verde, azul,negro
    int colores_rainbow[]={2+8,6+8,4+8,1+8,0};

	int x,y;


	int total_colores=5;
	int grueso_colores=8; //grueso de 8 pixeles cada franja

	//printf ("Filling colour bars up to %dX%d\n",xmax,ymax);


	for (x=0;x<xmax;x++) {
        int color=0;
		for (y=0;y<ymax;y++) {
			//scr_putpixel(x,y,(color&15) );
            scr_putpixel(x,y,colores_rainbow[(color%total_colores)] );

			if ((y%grueso_colores)==grueso_colores-1) color++;

		}
	}
}

//Compile with -g -rdynamic to show function names
void exec_show_backtrace(void) {
#ifdef linux
  int max_items=50;
  void *array[max_items];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, max_items);

  // print out all the frames to stderr
  //fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
}


void cpu_panic_printf_mensaje(char *mensaje)
{

	char buffer[1024];

        printf ("\n\n ZEsarUX kernel panic: %s \n",mensaje);
        print_registers(buffer);
        printf ("%s\n",buffer);

}





int cpu_panic_last_x;
int cpu_panic_last_y;

int cpu_panic_xmax;
int cpu_panic_ymax;

int cpu_panic_current_tinta;
int cpu_panic_current_papel;

int cpu_panic_pixel_zoom=1;

//Escribir caracter en pantalla, teniendo coordenadas en pixeles. Colores sobre tabla de colores de spectrum
//Pixeles de 2x2 en caso de que la ventana sea al menos de 512x384
void cpu_panic_printchar_lowlevel(int x,int y,int tinta,int papel,unsigned char c)
{
    //Detectar caracteres fuera de rango
    if (c<32 || c>127) c='?';

    int indice_charset=(c-32)*8;
    //char_set_spectrum[indice_charset]

    int scanline;
    int nbit;


    for (scanline=0;scanline<8;scanline++) {
        z80_byte byte_leido=char_set_spectrum[indice_charset++];
        for (nbit=0;nbit<8;nbit++) {
            int color;
            color=(byte_leido & 128 ? tinta : papel);

            if (cpu_panic_pixel_zoom==2){
                scr_putpixel(x+nbit*2,y+scanline*2,color);
                scr_putpixel(x+nbit*2,y+scanline*2+1,color);
                scr_putpixel(x+nbit*2+1,y+scanline*2,color);
                scr_putpixel(x+nbit*2+1,y+scanline*2+1,color);
            }

            else scr_putpixel(x+nbit,y+scanline,color);

            byte_leido=byte_leido<<1;
        }
    }
}


void cpu_panic_printchar_newline(void)
{
    cpu_panic_last_x=0;
    cpu_panic_last_y+=8*cpu_panic_pixel_zoom;

    //Si llega al final
    if (cpu_panic_last_y>cpu_panic_ymax-8) cpu_panic_last_y=cpu_panic_ymax-8;
}

void cpu_panic_printchar_nextcolumn(void)
{
    cpu_panic_last_x+=8*cpu_panic_pixel_zoom;

    //Final de linea
    if (cpu_panic_last_x>cpu_panic_xmax-8) cpu_panic_printchar_newline();

}

void cpu_panic_printchar(unsigned char c)
{
    if (c==10 || c==13) cpu_panic_printchar_newline();
    else {
        cpu_panic_printchar_lowlevel(cpu_panic_last_x,cpu_panic_last_y,cpu_panic_current_tinta,cpu_panic_current_papel,c);
        cpu_panic_printchar_nextcolumn();
    }
}

void cpu_panic_printstring(char *message)
{
	while (*message) {
		cpu_panic_printchar(*message);
		message++;
	}
}

//Abortar ejecucion del emulador con kernel panic
void cpu_panic(char *mensaje)
{
	char buffer[1024];

	//por si acaso, antes de hacer nada mas, vamos con el printf, para que muestre el error (si es que el driver de video lo permite)
	//hacemos pantalla de panic en xwindows y fbdev, y despues de finalizar el driver, volvemos a mostrar error
	cpu_panic_printf_mensaje(mensaje);

    cpu_panic_last_x=cpu_panic_last_y=0;

    cpu_panic_current_tinta=6;
    cpu_panic_current_papel=1;


	if (scr_end_pantalla!=NULL) {

		//si es xwindows o fbdev, mostramos panic mas mono
		if (si_complete_video_driver() ) {
			//quitar splash text por si acaso
			menu_splash_segundos=1;
			reset_splash_text();


			cls_menu_overlay();

			set_menu_overlay_function(normal_overlay_texto_menu);

            cpu_panic_xmax=screen_get_emulated_display_width_zoom_border_en();
            cpu_panic_ymax=screen_get_emulated_display_height_zoom_border_en();

            //Determinar si hacemos zoom 1 o 2, segun tamanyo total ventana
            int desired_width=32*8*2;
            int desired_height=24*8*2;

            if (cpu_panic_xmax>=desired_width && cpu_panic_ymax>=desired_height) cpu_panic_pixel_zoom=2;

			screen_show_panic_screen(cpu_panic_xmax,cpu_panic_ymax);

			print_registers(buffer);

            //Maximo 32 caracteres, aunque aprovechamos todo (border incluso) pero hay que considerar
            //por ejemplo pantalla sin border con zoom 1, en ese caso habra un minimo de 256 de ancho (32 caracteres de ancho)
                                 //01234567890123456789012345678901
            cpu_panic_printstring("******************************\n");
			cpu_panic_printstring("*  ZEsarUX kernel panic  :-( *\n");
            cpu_panic_printstring("******************************\n");
            cpu_panic_printstring("\n\n");
            cpu_panic_printstring("Panic message:\n");
			cpu_panic_printstring(mensaje);

            cpu_panic_printstring("\n\nCPU registers:\n");


			//los registros los mostramos dos lineas por debajo de la ultima usada
			cpu_panic_printstring(buffer);


			scr_refresca_pantalla_solo_driver();

			//Para xwindows hace falta esto, sino no refresca
			scr_actualiza_tablas_teclado();


			sleep(20);
			scr_end_pantalla();
		}

		else {
			scr_end_pantalla();
		}
	}

	cpu_panic_printf_mensaje(mensaje);

	exec_show_backtrace();

	exit(1);
}


//Para calcular tiempos funciones. Iniciar contador antes
void debug_tiempo_inicial(void)
{

	gettimeofday(&debug_timer_antes, NULL);

}

//Para calcular tiempos funciones. Contar contador despues e imprimir tiempo por pantalla
void debug_tiempo_final(void)
{

	long debug_timer_mtime, debug_timer_seconds, debug_timer_useconds;

	gettimeofday(&debug_timer_ahora, NULL);

        debug_timer_seconds  = debug_timer_ahora.tv_sec  - debug_timer_antes.tv_sec;
        debug_timer_useconds = debug_timer_ahora.tv_usec - debug_timer_antes.tv_usec;

        debug_timer_mtime = ((debug_timer_seconds) * 1000 + debug_timer_useconds/1000.0) + 0.5;

        printf("Elapsed time: %ld milliseconds\n\r", debug_timer_mtime);
}


void debug_printf (int debuglevel, const char * format , ...)
{
  int copia_verbose_level;

  copia_verbose_level=verbose_level;

  if (debuglevel<=copia_verbose_level) {
	//tamaño del buffer bastante mas grande que el valor constante definido
    char buffer_final[DEBUG_MAX_MESSAGE_LENGTH*2];
    char buffer_inicial[DEBUG_MAX_MESSAGE_LENGTH*2+64];
    char *verbose_message;
    va_list args;
    va_start (args, format);
    vsprintf (buffer_inicial,format, args);
    va_end (args);

	//TODO: controlar maximo mensaje

    switch (debuglevel) {
	case VERBOSE_ERR:
		verbose_message=VERBOSE_MESSAGE_ERR;
	break;

	case VERBOSE_WARN:
		verbose_message=VERBOSE_MESSAGE_WARN;
	break;

	case VERBOSE_INFO:
		verbose_message=VERBOSE_MESSAGE_INFO;
	break;

	case VERBOSE_DEBUG:
		verbose_message=VERBOSE_MESSAGE_DEBUG;
	break;

        case VERBOSE_PARANOID:
                verbose_message=VERBOSE_MESSAGE_PARANOID;
        break;


	default:
		verbose_message="UNKNOWNVERBOSELEVEL";
	break;

    }

    sprintf (buffer_final,"%s%s",verbose_message,buffer_inicial);

    if (scr_messages_debug!=NULL) scr_messages_debug (buffer_final);
    else printf ("%s\n",buffer_final);

    //Hacer aparecer menu, siempre que el driver no sea null ni.. porque no inicializado tambien? no inicializado
    if (debuglevel==VERBOSE_ERR) {

	//en el caso de stdout, no aparecera ventana igualmente, pero el error ya se vera por consola
        if (!strcmp(scr_driver_name,"stdout")) return;
        if (!strcmp(scr_driver_name,"simpletext")) return;
        if (!strcmp(scr_driver_name,"null")) return;
        //if (!strcmp(scr_driver_name,"")) return;
        sprintf (pending_error_message,"%s",buffer_inicial);
        if_pending_error_message=1;
        menu_abierto=1;
    }

}

}


int debug_nested_id_poke_byte;
int debug_nested_id_poke_byte_no_time;
int debug_nested_id_peek_byte;
int debug_nested_id_peek_byte_no_time;


void do_breakpoint_exception(char *message)
{
	if (strlen(message)>MAX_MESSAGE_CATCH_BREAKPOINT-1) {
		cpu_panic("do_breakpoint_exception: strlen>MAX_MESSAGE_CATCH_BREAKPOINT");
	}

	menu_breakpoint_exception.v=1;
	sprintf(catch_breakpoint_message,"%s",message);
	debug_printf (VERBOSE_INFO,"Catch breakpoint: %s",message);
}

void set_peek_byte_function_debug(void)
{

	debug_printf(VERBOSE_INFO,"Enabling debug on MMU");

	//peek_byte_no_time
	//peek_byte_no_time_no_debug=peek_byte_no_time;
	//peek_byte_no_time=peek_byte_no_time_debug;

	//peek_byte_time
	//peek_byte_no_debug=peek_byte;
	//peek_byte=peek_byte_debug;

	//poke_byte_no_time
	//poke_byte_no_time_no_debug=poke_byte_no_time;
	//poke_byte_no_time=poke_byte_no_time_debug;

	//poke_byte
	//poke_byte_no_debug=poke_byte;
	//poke_byte=poke_byte_debug;

	//out port
	//TODO. funciones out y lee_puerto de aqui habra que meterlas en una lista nested si se empiezan a usar en mas sitios...
	out_port_no_debug=out_port;
	out_port=out_port_debug;

	//lee puerto
	lee_puerto_no_debug=lee_puerto;
	lee_puerto=lee_puerto_debug;

        debug_nested_id_poke_byte=debug_nested_poke_byte_add(poke_byte_debug,"Debug poke_byte");
        debug_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(poke_byte_no_time_debug,"Debug poke_byte_no_time");
        debug_nested_id_peek_byte=debug_nested_peek_byte_add(peek_byte_debug,"Debug peek_byte");
        debug_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(peek_byte_no_time_debug,"Debug peek_byte_no_time");


}

void reset_peek_byte_function_debug(void)
{
	debug_printf(VERBOSE_INFO,"Clearing debug on MMU");

	//peek_byte_no_time=peek_byte_no_time_no_debug;
	//peek_byte=peek_byte_no_debug;

	//poke_byte_no_time=poke_byte_no_time_no_debug;
	//poke_byte=poke_byte_no_debug;

	out_port=out_port_no_debug;
	lee_puerto=lee_puerto_no_debug;


        debug_nested_poke_byte_del(debug_nested_id_poke_byte);
        debug_nested_poke_byte_no_time_del(debug_nested_id_poke_byte_no_time);
        debug_nested_peek_byte_del(debug_nested_id_peek_byte);
        debug_nested_peek_byte_no_time_del(debug_nested_id_peek_byte_no_time);
}



z80_byte peek_byte_no_time_debug (z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor;

	debug_mmu_mra=dir;

	//valor=peek_byte_no_time_no_debug(dir);
	valor=debug_nested_peek_byte_no_time_call_previous(debug_nested_id_peek_byte_no_time,dir);

	debug_mmu_mrv=valor; //Memory Read Value (valor leido en peek)



	return valor;
}


z80_byte peek_byte_debug (z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor;

	debug_mmu_mra=dir;

        //valor=peek_byte_no_debug(dir);
	valor=debug_nested_peek_byte_call_previous(debug_nested_id_peek_byte,dir);

	debug_mmu_mrv=valor; //Memory Read Value (valor leido en peek)


	//cpu_core_loop_debug_check_breakpoints();


	return valor;

}


z80_byte poke_byte_no_time_debug(z80_int dir,z80_byte value)
{
	debug_mmu_mwv=value;
	debug_mmu_mwa=dir;

	//poke_byte_no_time_no_debug(dir,value);
	debug_nested_poke_byte_no_time_call_previous(debug_nested_id_poke_byte_no_time,dir,value);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}

z80_byte poke_byte_debug(z80_int dir,z80_byte value)
{
	debug_mmu_mwv=value;
	debug_mmu_mwa=dir;

	//poke_byte_no_debug(dir,value);
	debug_nested_poke_byte_call_previous(debug_nested_id_poke_byte,dir,value);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;
}

void out_port_debug(z80_int puerto,z80_byte value)
{
        debug_mmu_pwv=value;
        debug_mmu_pwa=puerto;

	out_port_no_debug(puerto,value);
}

z80_byte lee_puerto_debug(z80_byte puerto_h,z80_byte puerto_l)
{
	z80_byte valor;

        debug_mmu_pra=value_8_to_16(puerto_h,puerto_l);

        valor=lee_puerto_no_debug(puerto_h,puerto_l);

        debug_mmu_prv=valor;


	return valor;
}



//Mostrar mensaje que ha hecho saltar el breakpoint y ejecutar accion (por defecto abrir menu)
void cpu_core_loop_debug_breakpoint(char *message)
{
	menu_abierto=1;
	do_breakpoint_exception(message);
}


//devuelve valor registro
//devuelve 0xFFFFFFFF si no reconoce
//activa cond_opcode si condicion es de opcode
unsigned int cpu_core_loop_debug_registro(char *registro,int *si_cond_opcode)
{

//Lo siguiente siempre al principio!!
  	*si_cond_opcode=0;


	//printf ("cpu_core_loop_debug_registro: registro: -%s-\n",registro);

  if (CPU_IS_SCMP) {


    if (!strcasecmp(registro,"pc")) return scmp_m_PC.w.l;

    //En caso contrario, fin

    //TODO. En caso de motorola se puede dar ese valor para algun registro. Hacer que devuelva otra cosa diferente en caso de no reconocimiento de registro
    return 0xFFFFFFFF;
  }

	else if (CPU_IS_MOTOROLA) {

		if (!strcasecmp(registro,"pc")) return get_pc_register();

    if (!strcasecmp(registro,"d0")) return m68k_get_reg(NULL, M68K_REG_D0);
    if (!strcasecmp(registro,"d1")) return m68k_get_reg(NULL, M68K_REG_D1);
    if (!strcasecmp(registro,"d2")) return m68k_get_reg(NULL, M68K_REG_D2);
    if (!strcasecmp(registro,"d3")) return m68k_get_reg(NULL, M68K_REG_D3);
    if (!strcasecmp(registro,"d4")) return m68k_get_reg(NULL, M68K_REG_D4);
    if (!strcasecmp(registro,"d5")) return m68k_get_reg(NULL, M68K_REG_D5);
    if (!strcasecmp(registro,"d6")) return m68k_get_reg(NULL, M68K_REG_D6);
    if (!strcasecmp(registro,"d7")) return m68k_get_reg(NULL, M68K_REG_D7);

    if (!strcasecmp(registro,"a0")) return m68k_get_reg(NULL, M68K_REG_A0);
    if (!strcasecmp(registro,"a1")) return m68k_get_reg(NULL, M68K_REG_A1);
    if (!strcasecmp(registro,"a2")) return m68k_get_reg(NULL, M68K_REG_A2);
    if (!strcasecmp(registro,"a3")) return m68k_get_reg(NULL, M68K_REG_A3);
    if (!strcasecmp(registro,"a4")) return m68k_get_reg(NULL, M68K_REG_A4);
    if (!strcasecmp(registro,"a5")) return m68k_get_reg(NULL, M68K_REG_A5);
    if (!strcasecmp(registro,"a6")) return m68k_get_reg(NULL, M68K_REG_A6);
    if (!strcasecmp(registro,"a7")) return m68k_get_reg(NULL, M68K_REG_A7);

    if (!strcasecmp(registro,"tstates")) return t_estados; //Este deberia ser comun a motorola y z80


		//En caso contrario, fin

		//TODO. En caso de motorola se puede dar ese valor para algun registro. Hacer que devuelva otra cosa diferente en caso de no reconocimiento de registro
		return 0xFFFFFFFF;
	}



	if (!strcasecmp(registro,"a")) return reg_a;
	if (!strcasecmp(registro,"b")) return reg_b;
	if (!strcasecmp(registro,"c")) return reg_c;
	if (!strcasecmp(registro,"d")) return reg_d;
	if (!strcasecmp(registro,"e")) return reg_e;
	if (!strcasecmp(registro,"f")) return Z80_FLAGS;
	if (!strcasecmp(registro,"h")) return reg_h;
	if (!strcasecmp(registro,"l")) return reg_l;
	if (!strcasecmp(registro,"i")) return reg_i;
	if (!strcasecmp(registro,"r")) return (reg_r&127)|(reg_r_bit7&128);

        if (!strcasecmp(registro,"af")) return REG_AF;
        if (!strcasecmp(registro,"bc")) return reg_bc;
        if (!strcasecmp(registro,"de")) return reg_de;
        if (!strcasecmp(registro,"hl")) return reg_hl;


/*
#define REG_AF (value_8_to_16(reg_a,Z80_FLAGS))

#define REG_AF_SHADOW (value_8_to_16(reg_a_shadow,Z80_FLAGS_SHADOW))
#define REG_HL_SHADOW (value_8_to_16(reg_h_shadow,reg_l_shadow))
#define REG_BC_SHADOW (value_8_to_16(reg_b_shadow,reg_c_shadow))
#define REG_DE_SHADOW (value_8_to_16(reg_d_shadow,reg_e_shadow))
*/
      if (!strcasecmp(registro,"af'")) return REG_AF_SHADOW;
      if (!strcasecmp(registro,"bc'")) return REG_BC_SHADOW;
      if (!strcasecmp(registro,"de'")) return REG_DE_SHADOW;
      if (!strcasecmp(registro,"hl'")) return REG_HL_SHADOW;

      if (!strcasecmp(registro,"a'")) return reg_a_shadow;
    	if (!strcasecmp(registro,"b'")) return reg_b_shadow;
    	if (!strcasecmp(registro,"c'")) return reg_c_shadow;
    	if (!strcasecmp(registro,"d'")) return reg_d_shadow;
    	if (!strcasecmp(registro,"e'")) return reg_e_shadow;
    	if (!strcasecmp(registro,"f'")) return Z80_FLAGS_SHADOW;
    	if (!strcasecmp(registro,"h'")) return reg_h_shadow;
    	if (!strcasecmp(registro,"l'")) return reg_l_shadow;



        if (!strcasecmp(registro,"sp")) return reg_sp;
        if (!strcasecmp(registro,"pc")) return reg_pc;
        if (!strcasecmp(registro,"ix")) return reg_ix;
        if (!strcasecmp(registro,"iy")) return reg_iy;

        if (!strcasecmp(registro,"fs")) return ( Z80_FLAGS & FLAG_S ? 1 : 0);
        if (!strcasecmp(registro,"fz")) return ( Z80_FLAGS & FLAG_Z ? 1 : 0);
        if (!strcasecmp(registro,"fp") || !strcasecmp(registro,"fv"))
            return ( Z80_FLAGS & FLAG_PV ? 1 : 0);
        if (!strcasecmp(registro,"fh")) return ( Z80_FLAGS & FLAG_H ? 1 : 0);
        if (!strcasecmp(registro,"fn")) return ( Z80_FLAGS & FLAG_N ? 1 : 0);
        if (!strcasecmp(registro,"fc")) return ( Z80_FLAGS & FLAG_C ? 1 : 0);


	if (!strcasecmp(registro,"opcode")) {
		*si_cond_opcode=1;
		//el valor de retorno aqui da igual. se evalua despues
		return 0;
	}

	if (!strcasecmp(registro,"(bc)")) return peek_byte_no_time(reg_bc);

        if (!strcasecmp(registro,"(de)")) return peek_byte_no_time(reg_de);

        if (!strcasecmp(registro,"(hl)")) return peek_byte_no_time(reg_hl);

        if (!strcasecmp(registro,"(sp)")) return peek_byte_no_time(reg_sp);

        if (!strcasecmp(registro,"(pc)")) return peek_byte_no_time(reg_pc);

        if (!strcasecmp(registro,"(ix)")) return peek_byte_no_time(reg_ix);

        if (!strcasecmp(registro,"(iy)")) return peek_byte_no_time(reg_iy);


	//si (NN)
	if (registro[0]=='(') {
		int s=strlen(registro);
		if (s>2) {
			if (registro[s-1]==')') {
				char buf_direccion[MAX_BREAKPOINT_CONDITION_LENGTH];
				//copiar saltando parentesis inicial
				sprintf (buf_direccion,"%s",&registro[1]);
				//quitar parentesis final
				//(16384) -> s=7 -> buf_direccion=16384). () -> s=2 ->buf_direccion=) .
				buf_direccion[s-2]=0;
				//printf ("buf_direccion: %s\n",buf_direccion);
				z80_int direccion=parse_string_to_number(buf_direccion);
				return peek_byte_no_time(direccion);
			}
		}
	}

	//ram mapeada en 49152-65535 de Spectrum
	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		if (!strcasecmp(registro,"ram")) return debug_paginas_memoria_mapeadas[3];

		//rom mapeada en Spectrum
		if (!strcasecmp(registro,"rom")) return (debug_paginas_memoria_mapeadas[0] & 127);

		//TODO. condiciones especiales para mapeo de paginas del +2A tipo ram en rom
	}

	//ram mapeada en 49152-65535 de Prism
        if (MACHINE_IS_PRISM) {
                if (!strcasecmp(registro,"ram")) return prism_retorna_ram_entra()*2;
	}

	//bancos memoria Z88
	if (MACHINE_IS_Z88) {
		if (!strcasecmp(registro,"seg0")) return blink_mapped_memory_banks[0];
		if (!strcasecmp(registro,"seg1")) return blink_mapped_memory_banks[1];
		if (!strcasecmp(registro,"seg2")) return blink_mapped_memory_banks[2];
		if (!strcasecmp(registro,"seg3")) return blink_mapped_memory_banks[3];
	}

	//Variables de la MMU
	//Memoria
	if (!strcasecmp(registro,"mrv")) return debug_mmu_mrv;
	if (!strcasecmp(registro,"mra")) return debug_mmu_mra;

	if (!strcasecmp(registro,"mwv")) return debug_mmu_mwv;
	if (!strcasecmp(registro,"mwa")) return debug_mmu_mwa;

	//Puertos
	if (!strcasecmp(registro,"prv")) return debug_mmu_prv;
	if (!strcasecmp(registro,"pra")) return debug_mmu_pra;

	if (!strcasecmp(registro,"pwv")) return debug_mmu_pwv;
	if (!strcasecmp(registro,"pwa")) return debug_mmu_pwa;

	//T-estados
	if (!strcasecmp(registro,"tstates")) return t_estados;
	if (!strcasecmp(registro,"tstatesl")) return t_estados % screen_testados_linea;
	if (!strcasecmp(registro,"tstatesp")) return debug_t_estados_parcial;

	if (!strcasecmp(registro,"scanline")) return t_scanline_draw;



	//interrupciones
	if (!strcasecmp(registro,"iff1")) return iff1.v;
	if (!strcasecmp(registro,"iff2")) return iff2.v;

	//enterrom, exitrom
/*
//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
int debug_exitrom=0;
*/

	if (!strcasecmp(registro,"enterrom")) {
		if (debug_enterrom==1) {
			debug_enterrom++;
			return 1;
		}
		return 0;
	}

	if (!strcasecmp(registro,"exitrom")) {
		if (debug_exitrom==1) {
			debug_exitrom++;
			return 1;
		}
		return 0;
	}



        return 0xFFFFFFFF;
}


//con la cadena de entrada de condicion, retorna el registro a mirar (o sea, lo que hay antes del =, < o > o /
//retorna registro en string registro. codigo de retorno es puntero a =, < , >, /. si no hay operador, retorna NULL
char *cpu_core_loop_debug_get_registro(char *entrada, char *registro)
{

	//int salir=0;
	char c;

	do {
		c=*entrada;
		if (c=='=' || c=='>' || c=='<' || c=='/') {
			//poner fin de cadena
			*registro=0;
			//y volver
			return entrada;
		}


		if (c==0) {
			//fin de cadena
			*registro=0;
			return NULL;
		}

		*registro=c;
		registro++;
		entrada++;
	} while(1);

}


//Ver longitud de valor (o sea, ignorando ceros a la izquierda)
//Con esto saber cuantos bytes representa dicho valor de condicion
//y comparar tantos bytes desde reg_pc
//Tener en cuenta que ceros a la izquierda no cuentan. Y que condicion para NOP (00H) cuenta como longitud 1
//Ejemplos de valor:
//00000000H  -> condicion para opcode NOP. longitud 1
//00000001H -> longitud 1
//0000ED03H longitud 2
//00DDCB01H  long 3
//DDCB0103H long 4

int debug_breakpoint_cond_opcode(unsigned int valor)
{
	int mascara=0x00FFFFFF;

	//buscar primer valor desde la izquierda que no es cero
	int i;
	int longitud;

	for (longitud=4;longitud>=1;longitud--) {
		if ( (valor&mascara) != valor) break;
		mascara=mascara>>8;
	}

	//Para condicion nop
	if (longitud==0) longitud=1;

	//printf ("longitud en bytes de la condicion opcode: %d\n",longitud);
	z80_int copia_reg_pc=reg_pc;

	for (i=longitud;i>=1;i--) {
		z80_byte valor_comparar;
		valor_comparar=valor>>((i-1)*8);
		//printf ("valor comparar: 0x%X\n",valor_comparar);

		if (valor_comparar!=peek_byte_no_time(copia_reg_pc) ) return 0;

		copia_reg_pc++;
	}



	return 1;

}



//Determina si una condicion es valida o no, hasta final de condicion o que se encuentre un operador: and, or, xor

int debug_breakpoint_condition(char *texto_total,int debug)
{

	char buffer_texto[MAX_BREAKPOINT_CONDITION_LENGTH];


	char *texto;
	texto=buffer_texto;


	//copiar a buffer_texto, hasta espacio final o 0
	//esto sirve para copiar solo el texto hasta final de condicion o que se encuentre un operador: and, or, xor
	int i;
	for (i=0;texto_total[i]!=0 && texto_total[i]!=' ';i++) {
		buffer_texto[i]=texto_total[i];
	}
	buffer_texto[i]=0;


	//registro a mirar . no deberia ser mas alla de 10 caracteres, pero por si acaso
	char registro[MAX_BREAKPOINT_CONDITION_LENGTH];
	char operador;

	//aunque normalmente valores no pasan de 16 bits,
	//en el caso de condicion de opcode queremos que sea de 32 bits
	unsigned int valor;


	//ver si se cumple condicion
	//formato: [registro][operador][valor]
	//donde:
	//[registro] puede ser: A,BC,DE,HL,SP,PC, IX, IY, (BC),(DE),(HL),(SP),(PC), (IX), (IY), B, C, D, E, H, L, R, OPCODE,
  // A0,A1.. A7, D0,D1,...D7
	//[operador] puede ser: < , > , = , /  (/ significa: diferente, o sea, no igual)
	//[valor] es un valor numerico

	texto=cpu_core_loop_debug_get_registro(texto,registro);
	if (texto==NULL) {
		//error evaluando expresion
		return 0;
	}


	/*
	int parentesis_inicial=0;

	//Ver si hay parentesis inicial
	if (texto[0]=='(' ) {
		//Y lo saltamos
		texto++;
		parentesis_inicial=1;
	}

	//obtenemos registro
	registro[0]=texto[0];

	//si hay operador en posicion 1, fin registro
	char c;
	c=texto[1];

	int indice_valor;

	if (c=='=' || c=='<' || c=='>' || c=='/') {
		registro[1]=0;
		operador=c;

		indice_valor=2;
	}

	else {
		//registro tiene 2 caracteres. puede haber parentesis
		registro[1]=c;
		registro[2]=0;
		int posicion_operador=2;
		indice_valor=3;

		if (parentesis_inicial) {
			posicion_operador++;
			indice_valor++;
			//cambiamos string para que este dentro del parentesis
			char registro_temp[3];
			sprintf (registro_temp,"%s",registro);
			sprintf (registro,"(%s)",registro_temp);
		}


		operador=texto[posicion_operador];

	}


	//Obtener valor de despues del operador <, > o = o /
	valor=parse_string_to_number(&texto[indice_valor]);

	*/

	operador=*texto;
	texto++;
	if ( *texto==0 ) {
		 //error evaluando expresion. no hay valor a comparar
                return 0;
        }

	//Obtener valor de despues del operador <, > o = o /
	valor=parse_string_to_number(texto);

        if (debug) {
                debug_printf (VERBOSE_DEBUG,"Parsed condition: %s %c %d",registro,operador,valor);
        }


	int si_cond_opcode;

	unsigned int v_reg=cpu_core_loop_debug_registro(registro,&si_cond_opcode);

	if (v_reg==0xFFFFFFFF) {
		if (debug) debug_printf (VERBOSE_DEBUG,"Invalid register %s",registro);
		return 0;
	}

	unsigned int valor_registro=v_reg;


	//gestionar condicion

	//por defecto error
	int valor_retorno=0;

	switch (operador) {
		case '=':
			//Ver si es condicion de opcode, que es especial
			if (si_cond_opcode) {
				//Ver longitud de valor (o sea, ignorando ceros a la izquierda)
				//Con esto saber cuantos bytes representa dicho valor de condicion
				//y comparar tantos bytes desde reg_pc
				return debug_breakpoint_cond_opcode(valor);
			}

			else {
				//resto de condiciones
				if ( valor_registro == valor ) valor_retorno=1;
			}
		break;

		case '>':
			if ( valor_registro > valor ) valor_retorno=1;
		break;

		case '<':
			if ( valor_registro < valor ) valor_retorno=1;
		break;

		case '/':
			if ( valor_registro != valor ) valor_retorno=1;
		break;

	}


	if (debug) {
		debug_printf (VERBOSE_DEBUG,"Condition: %s Result: %d",buffer_texto,valor_retorno);
	}

	return valor_retorno;

}





//devuelve valor registro en valor_final
//devuelve 0xFFFFFFFF si no reconoce
//activa cond_opcode si condicion es de opcode
//Retorna puntero a siguiente variable
char *debug_watches_get_value_variable_condition(char *texto,unsigned int *valor_final,char *registro)
{
        //registro a mirar . no deberia ser mas alla de 10 caracteres, pero por si acaso
        //char registro[MAX_BREAKPOINT_CONDITION_LENGTH];

			int i;
			for (i=0;i<MAX_BREAKPOINT_CONDITION_LENGTH;i++) {
				if (texto[i]==' ') {
					registro[i]=0;
					i++; //suponemos que el siguiente caracter no es un espacio
					break;
				}
				else if (texto[i]==0) {
					registro[i]=0;
					break;
				}

				else registro[i]=texto[i];
			}

			//no debería suceder nunca esto
			if (i==MAX_BREAKPOINT_CONDITION_LENGTH) registro[i]=0;
	//printf ("registro: %s\n",registro);

	int si_cond_opcode;
	unsigned int valor=cpu_core_loop_debug_registro(registro,&si_cond_opcode);
	//printf ("valor: %u\n",valor);
	*valor_final=valor;
	//printf ("*valor_final: %u\n",*valor_final);

	return &texto[i];
}




void debug_watches_loop(char *texto,char *texto_destino)
{
	//formato entrada: variable[espacio]variable[espacio]variable  ....

        //registro a mirar . no deberia ser mas alla de 10 caracteres, pero por si acaso
        char registro[MAX_BREAKPOINT_CONDITION_LENGTH];

	unsigned int valor_final;
	int len;

	while (*texto!=0) {

		texto=debug_watches_get_value_variable_condition(texto,&valor_final,registro);

		if (valor_final==0xFFFFFFFF) sprintf (texto_destino,"%s=UNK ",registro);
		else sprintf (texto_destino,"%s=%04X ",registro,valor_final);

		len=strlen(texto_destino);
		texto_destino +=len;

		//printf ("valor_final: %u\n",valor_final);

	}
}



#define BREAKPOINT_CONDITION_OP_AND 0
#define BREAKPOINT_CONDITION_OP_OR 1
#define BREAKPOINT_CONDITION_OP_XOR 2

#define BREAKPOINT_MAX_OPERADORES 3

char *breakpoint_cond_operadores[BREAKPOINT_MAX_OPERADORES]={
	" and ", " or ", " xor "
};

//retorna que valor de operador tiene en base a su texto. -1 si ninguno
int cpu_core_loop_debug_breakpoint_return_operator(char *string_op)
{

	int i;

	for (i=0;i<BREAKPOINT_MAX_OPERADORES;i++) {
		if (!strcasecmp(string_op,breakpoint_cond_operadores[i])) {
			//printf ("cpu_core_loop_debug_breakpoint_return_operator. operador leido: %d (%s)\n",i,breakpoint_cond_operadores[i]);
			return i;
		}
	}

	return -1;

}

//buscar dentro de toda la expresion si el primer operador coincide con buscar. no tener en cuenta mayusculas/minusculas
char *si_get_cond_operator(char *cadena, char *buscar)
{

	//localizar el primer espacio
	for (;*cadena!=0;cadena++) {
		if (*cadena==' ') {
			//comparar desde aqui hasta final de cadena buscar
			int i;

			for (i=0;cadena[i]!=0 && buscar[i]!=0;i++) {

				//si difieren. pasar letra cadena a minusculas
				char letra=cadena[i];
                		if (letra>='A' && letra<='Z') letra=letra+('a'-'A');

				if (letra!=buscar[i]) return NULL;
			}

			//se ha comparado todo y coincide
			if (buscar[i]==0) return cadena;
		}
	}

	//no se ha encontrado primer espacio en la cadena

	return NULL;

}

//busca si hay un operador " and " o " or " en la cadena de entrada
//retorno:
//valor de retorno : inicio del operador->apunta al primer espacio del operador
//final del operador->apunta a despues del espacio final del operador
//operador leido, incluyendo espacios principio y final : " and " o " or "
//si no encontrado, retorna NULL
char *debug_breakpoint_condition_loop_find_op(char *cadena_entrada,char **final_operador, char *operador_leido)
{

	char *inicio_cadena,*final_cadena;

	//solo para debug esto
	//char *orig_operador_leido;
	//orig_operador_leido=operador_leido;


	int i;

	for (i=0;i<BREAKPOINT_MAX_OPERADORES;i++) {

		char *s;
		s=breakpoint_cond_operadores[i];

		//buscar primer operador y ver si coincide
		//inicio_cadena=strstr(cadena_entrada,s);
		inicio_cadena=si_get_cond_operator(cadena_entrada,s);

		//printf ("debug_breakpoint_condition_loop_find_op. comparando %s con %s result=%p\n",cadena_entrada,s,inicio_cadena);

		if (inicio_cadena!=NULL) {
			//vamos incrementando final_cadena. inicio_cadena lo dejamos fijo y sera valor de retorno
			final_cadena=inicio_cadena;

			//printf ("leido substring %s\n",final_cadena);

			//copiar operador a destino
			//primer espacio
			*operador_leido=' ';
			operador_leido++;

			final_cadena++;
			//hasta que se lea el espacio final
			while (*final_cadena!=' ') {
				//printf ("cadena: %c\n",*final_cadena);

				*operador_leido=*final_cadena;
				operador_leido++;
				final_cadena++;
			}

			//meter espacio del final y codigo 0
			*operador_leido=' ';
			operador_leido++;

			*operador_leido=0;

			//printf ("debug_breakpoint_condition_loop_find_op. operador leido: %s\n",orig_operador_leido);

			//final_cadena apuntaba al espacio final. incrementar
			final_cadena++;

			//return final_cadena;

			//retornar valores adecuadamente
			*final_operador=final_cadena;

			return inicio_cadena;

		}
	}

	return NULL;

}

//ejecuta operacion logica and , or, etc con operador
int debug_breakpoint_condition_run_operator(int operador,int valor_anterior, int valor_siguiente)
{

	int valor_final;

        //hacer operacion con operador anterior
        switch (operador) {
                case BREAKPOINT_CONDITION_OP_OR:
                        valor_final=valor_anterior | valor_siguiente;
                break;

                case BREAKPOINT_CONDITION_OP_AND:
                        valor_final=valor_anterior & valor_siguiente;
                break;

                case BREAKPOINT_CONDITION_OP_XOR:
                        valor_final=valor_anterior ^ valor_siguiente;
                break;

                default:
			//aqui en teoria no deberia entrar, porque el valor del operador ya entra con un valor valido
                        debug_printf (VERBOSE_DEBUG,"Invalid operator on breakpoint condition");
                        return 0;
                break;

        }

	return valor_final;

}

//Determina si una condicion es valida o no, incluyendo operadores and y or de separacion
//llama repetidamente a debug_breakpoint_condition para cada condicion separada por and o or

//Nota: aqui se habla de condicion entendiendo una expresion asi: [registro][operador][valor]], por ejemplo: A>3
//y un operador es alguno de los siguientes: or, and, xor, etc

int debug_breakpoint_condition_loop(char *texto,int debug)
{
	//formato entrada: condicion and/or/xor condicion and/or/xor condicion ....
	//tambien vale una sola condicion:   condicion

	//valor anterior leido.
	int valor_anterior;
	//previo operador leido
	int previo_operador;

	int valor_final;

	//buffer donde guardar operador " and ", " or ", " xor ", etc
	char buffer_operador_leido[10];

	char *siguiente_condicion;
	char *inicio_operador;

	//siguiente_condicion=texto;
//printf ("en debug_breakpoint_condition_loop\n");
	//buscar siguiente operador
	inicio_operador=debug_breakpoint_condition_loop_find_op(texto,&siguiente_condicion,buffer_operador_leido);

	//como es primera condicion, no aplicar operador, resultado es tal cual
	valor_final=debug_breakpoint_condition(texto,debug);
  //printf ("valor final: %d condicion: [%s]\n",valor_final,texto);
	valor_anterior=valor_final;

  //printf ("Inicio operador: %p\n",inicio_operador);

	while (inicio_operador!=NULL) {
		texto=siguiente_condicion;
		previo_operador=cpu_core_loop_debug_breakpoint_return_operator(buffer_operador_leido);
		//si operador desconocido,volver
		if (previo_operador<0) {
			debug_printf (VERBOSE_DEBUG,"Unknown operator %s",buffer_operador_leido);
			return 0;
		}

		//buscar hasta siguiente operador
		inicio_operador=debug_breakpoint_condition_loop_find_op(texto,&siguiente_condicion,buffer_operador_leido);
		//Si no hay mas operadores, inicio_operador sera NULL y hara salir del bucle while

		//evaluar resultado condicion
		valor_final=debug_breakpoint_condition(texto,debug);

		//y aplicar operador logico
		int result;
		result=debug_breakpoint_condition_run_operator(previo_operador,valor_anterior,valor_final);
		if (debug) debug_printf (VERBOSE_DEBUG,"%d%s%d = %d",valor_anterior,breakpoint_cond_operadores[previo_operador],valor_final,result);

		valor_final=result;
		valor_anterior=valor_final;

	}

	if (debug) debug_printf (VERBOSE_DEBUG,"Final condition: %d",valor_final);
	return valor_final;
}


//Comprobar condiciones. Solo lo hacemos en core_loop
void cpu_core_loop_debug_check_breakpoints(void)
{
        //Condicion de debug
        if (debug_breakpoints_enabled.v) {

          //printf ("core con debug\n");

                //Tabla de breakpoints PC. No usada. porque se puede emular con condicion PC=

                int i;

                /*
                for (i=0;i<MAX_BREAKPOINTS;i++) {
                        if (debug_breakpoints_array[i]!=-1) {
                                if (debug_breakpoints_array[i]==reg_pc) {
                                        char buffer_mensaje[32];
                                        sprintf(buffer_mensaje,"PC Address is: %d",reg_pc);
                                        cpu_core_loop_debug_breakpoint(buffer_mensaje);
                                }
                        }
                }
                */

                //Breakpoint de condicion
                for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
                        if (debug_breakpoints_conditions_array[i][0]!=0) {
                                if (
					debug_breakpoint_condition_loop(&debug_breakpoints_conditions_array[i][0],0) &&
					debug_breakpoints_conditions_enabled[i]

				) {
					//Si condicion pasa de false a true o bien el comportamiento por defecto es saltar siempre
					if (debug_breakpoints_cond_behaviour.v==0 || debug_breakpoints_conditions_saltado[i]==0) {
						debug_breakpoints_conditions_saltado[i]=1;
	                                        char buffer_mensaje[MAX_BREAKPOINT_CONDITION_LENGTH+64];
        	                                sprintf(buffer_mensaje,"Condition: %s",&debug_breakpoints_conditions_array[i][0]);

                                          //Ejecutar accion, por defecto es abrir menu
						catch_breakpoint_index=i;
                	                        cpu_core_loop_debug_breakpoint(buffer_mensaje);
					}
                                }
				else {
					//No se cumple condicion. Indicarlo que esa condicion esta false
					debug_breakpoints_conditions_saltado[i]=0;
				}
                        }
                }

        }

}

int debug_watches_mostrado_frame=0;
char debug_watches_texto_destino[1024];

//Misma limitacion de longitud que un breakpoint.
//Si cadena vacia, no hay breakpoint
char debug_watches_text_to_watch[MAX_BREAKPOINT_CONDITION_LENGTH]="";

z80_byte debug_watches_y_position=0;

//void cpu_core_loop_debug(void)

int debug_nested_id_core;
z80_byte cpu_core_loop_debug(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

  	//Llamamos al core normal
	debug_nested_core_call_previous(debug_nested_id_core);


  //Evaluamos condiciones debug

	//Condiciones enterrom y exitrom
/*
//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
int debug_exitrom=0;
*/

	if (reg_pc<16384) {
		//no ha salido de rom
		debug_exitrom=0;

		//ver si hay que avisar de un enterrom
		if (debug_enterrom==0) debug_enterrom=1;
	}

	if (reg_pc>16383) {
		//no esta en rom
		debug_enterrom=0;

		//ver si hay que avisar de un exitrom
		if (debug_exitrom==0) debug_exitrom=1;
	}

	cpu_core_loop_debug_check_breakpoints();

	//mostrar watches
	//frames_total dice el numero de frame del driver de video
	if (frames_total!=debug_watches_mostrado_frame) {
		debug_watches_mostrado_frame=frames_total;
		if (debug_watches_text_to_watch[0]!=0) {
			debug_watches_loop(debug_watches_text_to_watch,debug_watches_texto_destino);
			screen_print_splash_text(debug_watches_y_position,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,debug_watches_texto_destino);
		}
	}



	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;

}



void set_cpu_core_loop(void)
{
        switch (cpu_core_loop_active) {

                case CPU_CORE_SPECTRUM:
                        debug_printf(VERBOSE_INFO,"Setting Spectrum CPU core");
			if (core_spectrum_uses_reduced.v==0) {
	                        cpu_core_loop=cpu_core_loop_spectrum;
			}
			else {
				debug_printf(VERBOSE_WARN,"Setting REDUCED Spectrum CPU core, the following features will NOT be available or will NOT be properly emulated: Debug t-states, Char detection, +3 Disk, Save to tape, Divide, Divmmc, RZX, Raster interrupts, TBBlue Copper, Audio DAC, Video out to file");
				cpu_core_loop=cpu_core_loop_reduced_spectrum;
			}
                        cpu_core_loop_name="Spectrum";
                break;

                case CPU_CORE_ZX8081:
                        debug_printf(VERBOSE_INFO,"Setting ZX80/81 CPU core");
                        cpu_core_loop=cpu_core_loop_zx8081;
                        cpu_core_loop_name="ZX80/81";
                break;

                case CPU_CORE_ACE:
                        debug_printf(VERBOSE_INFO,"Setting Jupiter ACE core");
                        cpu_core_loop=cpu_core_loop_ace;
                        cpu_core_loop_name="Jupiter ACE";
                break;

                case CPU_CORE_CPC:
                        debug_printf(VERBOSE_INFO,"Setting CPC core");
                        cpu_core_loop=cpu_core_loop_cpc;
                        cpu_core_loop_name="CPC";
                break;

                case CPU_CORE_Z88:
                        debug_printf(VERBOSE_INFO,"Setting Z88 CPU core");
                        cpu_core_loop=cpu_core_loop_z88;
                        cpu_core_loop_name="Z88";
                break;

		case CPU_CORE_SAM:
			debug_printf(VERBOSE_INFO,"Setting Sam Coupe CPU core");
			cpu_core_loop=cpu_core_loop_sam;
      cpu_core_loop_name="Sam Coupe";
		break;

		case CPU_CORE_QL:
			debug_printf(VERBOSE_INFO,"Setting QL CPU core");
			cpu_core_loop=cpu_core_loop_ql;
      cpu_core_loop_name="QL";
		break;

    case CPU_CORE_MK14:
      debug_printf(VERBOSE_INFO,"Setting MK14 CPU core");
      cpu_core_loop=cpu_core_loop_mk14;
      cpu_core_loop_name="MK14";
    break;


                default:
                        cpu_panic("Unknown cpu core");
                break;

        }

	/*
        //Activar core de debug si es necesario
        if (debug_cpu_core_loop.v) {
                debug_printf(VERBOSE_INFO,"Enabling debug on cpu core");
                cpu_core_loop_no_debug=cpu_core_loop;
                cpu_core_loop=cpu_core_loop_debug;
        }
	*/

}


void set_cpu_core_loop_debug(void)
{
	debug_printf(VERBOSE_INFO,"Enabling debug on cpu core");
	debug_nested_id_core=debug_nested_core_add(cpu_core_loop_debug,"Debug core");
}

void reset_cpu_core_loop_debug(void)
{
	debug_printf(VERBOSE_INFO,"Disabling debug on cpu core");
	debug_nested_core_del(debug_nested_id_core);
}



void breakpoints_enable(void)
{
	debug_breakpoints_enabled.v=1;
	//debug_cpu_core_loop.v=1;
	set_peek_byte_function_debug();

        set_cpu_core_loop_debug();


}

void breakpoints_disable(void)
{
	debug_breakpoints_enabled.v=0;
	//debug_cpu_core_loop.v=0;
	reset_peek_byte_function_debug();

        reset_cpu_core_loop_debug();

}


void machine_emulate_memory_refresh_debug_counter(void)
{
        debug_printf (VERBOSE_DEBUG,"Emulate_memory_refresh_counter: %d Limit: %d Max: %d %s",machine_emulate_memory_refresh_counter,MAX_EMULATE_MEMORY_REFRESH_LIMIT,MAX_EMULATE_MEMORY_REFRESH_COUNTER,(machine_emulate_memory_refresh_counter>MAX_EMULATE_MEMORY_REFRESH_LIMIT ? "Beyond Limit" : "") );

}



//Puntero a la funcion original
//void (*cpu_core_loop_no_transaction_log) (void);

z80_bit cpu_transaction_log_enabled={0};
char transaction_log_dumpassembler[32];
size_t transaction_log_longitud_opcode;


FILE *ptr_transaction_log=NULL;

char transaction_log_filename[PATH_MAX];

char transaction_log_line_to_store[2048];


//campos a guardar en el transaction log
z80_bit cpu_transaction_log_store_datetime={0};
z80_bit cpu_transaction_log_store_address={1};
z80_bit cpu_transaction_log_store_tstates={0};
z80_bit cpu_transaction_log_store_opcode={1};
z80_bit cpu_transaction_log_store_registers={0};


int transaction_log_nested_id_core;


//Para tener una memory zone que apunte a un archivo
char memory_zone_by_file_name[PATH_MAX];
z80_byte *memory_zone_by_file_pointer;
int memory_zone_by_file_size=0;

///void cpu_core_loop_transaction_log(void)
z80_byte cpu_core_loop_transaction_log(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

	//Si la cpu ha acabado un ciclo y esta esperando final de frame, no hacer nada
	if (esperando_tiempo_final_t_estados.v==0) {

		int index=0;

                if (cpu_transaction_log_store_datetime.v) {

	                //fecha grabacion
			/*
        	        time_t tiempo = time(NULL);
                	struct tm tm = *localtime(&tiempo);

			//funciones localtime no son tan precisas como gettimeofday
			//parece que localtime tarda unos milisegundos en actualizar los segundos
			//y aparecen tiempos como
			//10:00:01.9999
			//10:00:01.0003   <-aqui deberia haber saltado el segundo
			//10:00:01.0005
			//10:00:02.0008

			//microsegundos
			long trans_useconds;

			struct timeval trans_timer;


		        gettimeofday(&trans_timer, NULL);

        		trans_useconds = trans_timer.tv_usec;
			sprintf (&transaction_log_line_to_store[index],"%04d/%02d/%02d %02d:%02d:%02d.%06ld ",
				tm.tm_year + 1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,trans_useconds);
			*/


			struct timeval tv;
			struct tm* ptm;
			long microseconds;


                        // 2015/01/01 11:11:11.999999 "
                        // 123456789012345678901234567
			const int longitud_timestamp=27;

			/* Obtain the time of day, and convert it to a tm struct. */
			gettimeofday (&tv, NULL);
			ptm = localtime (&tv.tv_sec);
			/* Format the date and time, down to a single second. */
			char time_string[40];

			strftime (time_string, sizeof(time_string), "%Y/%m/%d %H:%M:%S", ptm);

			microseconds = tv.tv_usec;
			 /* Print the formatted time, in seconds, followed by a decimal point and the microseconds. */
			sprintf (&transaction_log_line_to_store[index],"%s.%06ld ", time_string, microseconds);

			index +=longitud_timestamp;

                }

		if (cpu_transaction_log_store_tstates.v) {
			sprintf (&transaction_log_line_to_store[index],"%05d ",t_estados);
			index +=6;
		}

    menu_z80_moto_int registro_pc=get_pc_register();
    registro_pc=adjust_address_space_cpu(registro_pc);

		if (cpu_transaction_log_store_address.v) {
      if (CPU_IS_MOTOROLA) {
        sprintf (&transaction_log_line_to_store[index],"%05X ",registro_pc);
        index +=6;
      }
			else {
        sprintf (&transaction_log_line_to_store[index],"%04X ",registro_pc);
			  index +=5;
      }
		}


	        if (cpu_transaction_log_store_opcode.v) {
			debugger_disassemble(&transaction_log_line_to_store[index],32,&transaction_log_longitud_opcode,registro_pc);
			int len=strlen(&transaction_log_line_to_store[index]);
			index +=len;
			transaction_log_line_to_store[index++]=' ';
        	}


		if (cpu_transaction_log_store_registers.v) {
			print_registers(&transaction_log_line_to_store[index]);
                	int len=strlen(&transaction_log_line_to_store[index]);
	                index +=len;
        	        transaction_log_line_to_store[index++]=' ';
	        }

		//salto de linea
		transaction_log_line_to_store[index++]=10;

		fwrite(transaction_log_line_to_store,1,index,ptr_transaction_log);
	}


	//cpu_core_loop_no_transaction_log();
	//Llamar a core anterior
	debug_nested_core_call_previous(transaction_log_nested_id_core);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;
}


void set_cpu_core_transaction_log(void)
{
        debug_printf(VERBOSE_INFO,"Enabling Transaction Log");

        //cpu_core_loop_no_transaction_log=cpu_core_loop;
	//cpu_core_loop=cpu_core_loop_transaction_log;

	transaction_log_nested_id_core=debug_nested_core_add(cpu_core_loop_transaction_log,"Transaction Log Core");


	cpu_transaction_log_enabled.v=1;

                              ptr_transaction_log=fopen(transaction_log_filename,"ab");
                                  if (!ptr_transaction_log)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open Transaction log");

                                  }

}

void reset_cpu_core_transaction_log(void)
{
        debug_printf(VERBOSE_INFO,"Setting normal cpu core loop");
	//cpu_core_loop=cpu_core_loop_no_transaction_log;
	debug_nested_core_del(transaction_log_nested_id_core);
	cpu_transaction_log_enabled.v=0;
	if (ptr_transaction_log!=NULL) fclose(ptr_transaction_log);
}


int debug_antes_t_estados_parcial=0;

void debug_get_t_stados_parcial_pre(void)
{
debug_antes_t_estados_parcial=t_estados;
}

void debug_get_t_stados_parcial_post(void)
{
//Incrementar variable debug_t_estados_parcial segun lo que haya incrementado
                       if (t_estados<debug_antes_t_estados_parcial) {
                               //Contador ha dado la vuelta
                               int dif_hasta_final_frame=screen_testados_total-debug_antes_t_estados_parcial;
                               debug_t_estados_parcial+=dif_hasta_final_frame+t_estados;
                       }
                       else {
                               debug_t_estados_parcial+=(t_estados-debug_antes_t_estados_parcial);
                       }
}


//Para saltar los step by step
void debug_anota_retorno_step_nmi(void)
{
	debug_core_lanzado_inter.v=1;
	debug_core_lanzado_inter_retorno_pc_nmi=reg_pc;
}

void debug_anota_retorno_step_maskable(void)
{
        debug_core_lanzado_inter.v=1;
        debug_core_lanzado_inter_retorno_pc_maskable=reg_pc;
}



/*

Pruebas stack trace

Linkar con librerias unwind

#include <unwind.h>
#include <libunwind.h>

int getFileAndLine (unw_word_t addr, char *file, size_t flen, int *line)
{
	static char buf[256];
	char *p;

	// prepare command to be executed
	// our program need to be passed after the -e parameter
	sprintf (buf, "/usr/bin/addr2line -C -e zesarux -f -i %lx", addr);

	printf("%s\n",buf);

	FILE* f = popen (buf, "r");

	if (f == NULL)
	{
		perror (buf);
		return 0;
	}

	// get function name
	fgets (buf, 256, f);

	// get file and line
	fgets (buf, 256, f);

	if (buf[0] != '?')
	{
		int l;
		char *p = buf;

		// file name is until ':'
		while (*p != ':')
		{
			p++;
		}

		*p++ = 0;
		// after file name follows line number
		strcpy (file , buf);
		sscanf (p,"%d", line);
	}
	else
	{
		strcpy (file,"unkown");
		*line = 0;
	}

	pclose(f);
}

void show_backtrace (void)
{
	char name[256];
	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip, sp, offp;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	while (unw_step(&cursor) > 0)
	{
		char file[256];
		int line = 0;

		name[0] = '\0';
		unw_get_proc_name(&cursor, name, 256, &offp);
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		//printf ("%s ip = %lx, sp = %lx\n", name, (long) ip, (long) sp);
		getFileAndLine((long)ip, file, 256, &line);
		printf("%s in file %s line %d\n", name, file, line);
	}
}

*/




//Funciones para gestion de listas de funciones
/*
Gestión de funciones anidadas para el core, peek, etc
Rutina común: devuelve z80, admite dir y z80
Es responsabilidad de cada rutina trap el llamar a la anterior
Funciones para agregar, quitar y llamar a anterior
Al agregar se le asigna un id, o bien la rutina que llama lo hace con un id fijo o un string identificativo
Si se agrega y no hay ninguna, se crea una de cero
Lista de anidados mediante estructura con un puntero a la siguiente

Funciones de peek y poke deben llamar a las anteriores normalmente, para hacer que se gestione la contienda o los breakpoints de debug
En caso de funciones con su propia contienda, estas no llamaran a la anterior pero entonces tampoco llamaran a debug y no funcionarán bien los breakpoints y habrá un problema... hay de esas?

Funciones de core también llamaran a las anteriores

Resumiendo: todas deberían llamar a las anteriores
*/

//Asignar memoria para elemento.
//Retorna: puntero a elemento asignado
debug_nested_function_element *debug_nested_alloc_element(void)
{
	debug_nested_function_element *puntero;
	//Asignar memoria
	puntero=malloc(sizeof(debug_nested_function_element));
        if (puntero==NULL) {
                cpu_panic ("No enough memory to create nested element");
        }

	return puntero;

}


//Llenar valores de la estructura
void debug_nested_fill(debug_nested_function_element *estructura,char *function_name, int id, debug_nested_function funcion, debug_nested_function_element *next,debug_nested_function_element *previous)
{

	if (strlen(function_name)>MAX_DEBUG_FUNCTION_NAME) {
		cpu_panic("Nested function name too large");
	}

	strcpy(estructura->function_name,function_name);
	estructura->id=id;
	estructura->funcion=funcion;
	estructura->next=next;
	estructura->previous=previous;

	debug_printf (VERBOSE_DEBUG,"Filling nested function. ID: %d Name: %s",id,function_name);
	//printf ("Filling nested function. ID: %d Name: %s\n",id,function_name);

}

//Buscar un identificador dentro de una lista
debug_nested_function_element *debug_nested_find_id(debug_nested_function_element *e,int id)
{

	if (e==NULL) {
		debug_printf (VERBOSE_DEBUG,"Pointer is NULL when calling to debug_nested_find_id");
		return NULL;
	}

	int salir=0;
	do {
		if (e->id==id) return e;

		//Hay siguiente?
		if (e->next!=NULL) e=e->next;
		else salir=1;
	} while (!salir);


	//No encontrado
	return NULL;
}

//Buscar un nombre de funcion dentro de una lista
debug_nested_function_element *debug_nested_find_function_name(debug_nested_function_element *e,char *nombre)
{
        int salir=0;
        do {
                if (!strcmp(e->function_name,nombre)) return e;

                //Hay siguiente?
                if (e->next!=NULL) e=e->next;
                else salir=1;
        } while (!salir);


        //No encontrado
        return NULL;
}


//Buscar primer identificador libre. Empezando desde 0
int debug_nested_find_free_id(debug_nested_function_element *e)
{
	int id;

	for (id=0;id<MAX_DEBUG_NESTED_ELEMENTS;id++) {
		if (debug_nested_find_id(e,id)==NULL) {
			//ID no encontrado. retornamos ese
			return id;
		}

	}

	//Si no hay ids libres, cpu_panic
	cpu_panic("Maximum nested elements reached");

	//Para que no se queje el compilador. Aqui no llega nunca
	return 0;
}

debug_nested_function_element *debug_nested_find_last(debug_nested_function_element *e)
{
	//debug_nested_function_element *last;

	//last=e;

	while (e->next!=NULL) {
		if (e->next!=NULL) e=e->next;
	}

	return e;
}


//Agregar un elemento a la lista. Retorna id
int debug_nested_add(debug_nested_function_element *e,char *function_name, debug_nested_function funcion)
{
	int id;
	debug_nested_function_element *last_element;
	debug_nested_function_element *new_element;

	//Obtener id libre
	id=debug_nested_find_free_id(e);
	//printf ("New id on add: %d\n",id);

	//Buscar ultimo elemento
	last_element=debug_nested_find_last(e);

	//Asignar uno nuevo
	new_element=debug_nested_alloc_element();

	//Indicar puntero del anterior hacia el siguiente
	last_element->next=new_element;

	//Y llenar valores del actual
	debug_nested_fill(new_element,function_name, id, funcion, NULL, last_element);

	debug_printf (VERBOSE_DEBUG,"Adding nested function id: %d name: %d",id,function_name);

	return id;
}


//Pide puntero al puntero inicial de la lista
void debug_nested_del(debug_nested_function_element **puntero,int id)
{
	debug_nested_function_element *e;

	e=*puntero;

	//Si puntero es nulo, no hacer nada
	if (e==NULL) {
		debug_printf (VERBOSE_DEBUG,"Nested pointer NULL calling to debug_nested_del. Not deleting anything");
		return;
	}

	//Primero buscar elemento
	debug_nested_function_element *borrar;

	//Elemento anterior al que buscamos
	debug_nested_function_element *anterior;
	//Elemento siguiente al que buscamos
	debug_nested_function_element *siguiente;

	borrar=debug_nested_find_id(e,id);

	//Si NULL, no encontrado
	if (borrar==NULL) {
		debug_printf (VERBOSE_DEBUG,"Nested element to delete with id %d not found",id);
		//printf ("Nested element to delete with id %d not found\n",id);
		return;
	}

	//Anterior
	anterior=borrar->previous;

	//Siguiente
	siguiente=borrar->next;

	//Si hay anterior, asignarle el siguiente
	if (anterior) {
		anterior->next=siguiente;
	}

	//Si no hay anterior, quiere decir que es el inicial. Reasignar puntero inicial
	else {
		*puntero=siguiente;
	}

	//Si hay siguiente, asignarle el anterior
	if (siguiente) {
		siguiente->previous=anterior;
	}

	//Liberar memoria
	debug_printf (VERBOSE_DEBUG,"Freeing element id %d name %s",borrar->id,borrar->function_name);
	free(borrar);
}

//Funcion generica que gestiona las llamadas a los elementos anidados
z80_byte debug_nested_generic_handler(debug_nested_function_element *e,z80_int dir,z80_byte value)
{
       //Buscar el ultimo
        debug_nested_function_element *last;

        last=debug_nested_find_last(e);

        //Y llamar a su funcion. Dado que son funciones genericas, enviar parametros sin sentido
        //if (t_estados<20) printf ("Calling last element function name: %s\n",last->function_name);
	z80_byte resultado;
        resultado=last->funcion(dir,value);

        return resultado;

}



//
//Para testeo
//
z80_byte debug_test_funcion(z80_int dir, z80_byte value)
{
	//Prueba simple
	return dir+value*2;

}

//Testeo recorrer adelante
void debug_test_needed_adelante(debug_nested_function_element *e,char *result)
{
char buffer[1024];

	//printf ("Recorriendo list adelante\n");
        int contador=0;
        do {
                sprintf (buffer,"Element: %p (%d) id: %d name: %s pointer function: %p previous: %p next: %p\n", (void *)e, contador, e->id, e->function_name, e->funcptr, (void *)e->previous, (void *)e->next);
                debug_dump_nested_print(result,buffer);

                contador++;
                e=e->next;
        } while (e!=NULL);

}

void debug_test_needed_atras(debug_nested_function_element *e,int contador)
{

	printf ("Recorriendo list atras\n");
        //Buscar ultimo
        e=debug_nested_find_last(e);

        do {
                printf ("elemento: %p (%d) id: %d nombre: %s puntero_funcion: %p previous: %p next: %p\n", (void *)e, contador, e->id, e->function_name, e->funcptr, (void *)e->previous, (void *)e->next);
                contador--;
                e=e->previous;
        } while (e!=NULL);
}

//Prueba crear unos cuantos elementos
//Se puede llamar aqui desde donde se quiera, para testear
const int debug_test_needed_max=100;
void debug_test_nested(void)
{

	printf ("Allocating list\n");

	//Creamos el inicial
	debug_nested_function_element *lista_inicial;
	//debug_nested_function_element *e;
	lista_inicial=debug_nested_alloc_element();
	//e=lista_inicial;

	//Le metemos datos

	//Primer identificador cero
	debug_nested_fill(lista_inicial,"Pruebas",0, debug_test_funcion, NULL, NULL);

	//Asignar otros mas
	int i;
	char nombre_funcion[30];
	for (i=0;i<debug_test_needed_max-1;i++) {
		sprintf (nombre_funcion,"Pruebas%d",i);
		int nuevo_id=debug_nested_add(lista_inicial,nombre_funcion,debug_test_funcion);
		printf ("contador: %d nuevo_id: %d\n",i,nuevo_id);
	}

	//Recorrer array hacia adelante e ir mostrando
	debug_test_needed_adelante(lista_inicial,NULL);

	//Recorrer array hacia atras e ir mostrando
	debug_test_needed_atras(lista_inicial,debug_test_needed_max-1);


	//Borrar a peticion de usuario
	int elemento_a_borrar;
	int salir=0;

	do {
		printf ("Id a borrar: ");
		scanf ("%d",&elemento_a_borrar);
		if (elemento_a_borrar<0 || elemento_a_borrar>debug_test_needed_max-1) salir=1;
		else {
			debug_nested_del(&lista_inicial,elemento_a_borrar);
			debug_test_needed_adelante(lista_inicial,NULL);
		}
	} while (!salir);


	printf ("\n\nEliminando elemento con id 4\n\n");
	debug_nested_del(&lista_inicial,4);

	//Y volver a recorrer array
        //Recorrer array hacia adelante e ir mostrando
        debug_test_needed_adelante(lista_inicial,NULL);

        //Recorrer array hacia atras e ir mostrando
        debug_test_needed_atras(lista_inicial,debug_test_needed_max-1);


	//Prueba llamar a funcion
	z80_byte resultado=lista_inicial->funcion(10,2);
	printf ("Resultado funcion asignada: %d\n",resultado);

	//Ir eliminando todos los ids
	for (i=0;i<debug_test_needed_max;i++) {
		printf ("\nEliminando id %d\n",i);
		debug_nested_del(&lista_inicial,i);
		//Mostrar
		if (lista_inicial!=NULL) debug_test_needed_adelante(lista_inicial,NULL);
		else printf ("(lista vacia)\n");
	}
}


//
//Fin testeo
//


//
//Punteros de funciones nested
//
//
//Para Core
//
//puntero a cpu core normal sin lista
void (*cpu_core_loop_no_nested) (void);
//puntero a primer item en lista de funciones de core
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_core;
//
//Para peek_byte
//
//....




//
//INICIO Funciones de anidacion de core mediante listas nested
//

//Funcion que gestiona las llamadas a los cores anidados
void cpu_core_loop_nested_handler(void)
{
	debug_nested_generic_handler(nested_list_core,0,0);
}


//Agregar un core sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_core_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de core
	//if (nested_list_core==NULL) {
	if (cpu_core_loop!=cpu_core_loop_nested_handler) {

		debug_printf (VERBOSE_DEBUG,"Adding first core to nested list");

        	//Creamos el inicial
	        nested_list_core=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_core,nombre,0,funcion, NULL, NULL);

		cpu_core_loop_no_nested=cpu_core_loop;
		cpu_core_loop=cpu_core_loop_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_core,nombre,funcion);
	}

}

void debug_nested_core_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

	//Si esta a NULL, no hacer nada
	if (cpu_core_loop!=cpu_core_loop_nested_handler) {
		debug_printf (VERBOSE_DEBUG,"Core nested is not enabled. Not deleting anything");
		return;
	}

	debug_nested_del(&nested_list_core,id);

	if (nested_list_core==NULL) {
		//lista vacia. asignar core normal
		debug_printf (VERBOSE_DEBUG,"Core nested empty. Assign normal core");
		cpu_core_loop=cpu_core_loop_no_nested;
	}
}


//Llama a core anterior, llamando por numero de id
void debug_nested_core_call_previous(int id)
{

	//if (t_estados<20) printf ("Calling previous core to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al core original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_core->next==NULL) {
		//Solo un elemento. Llamar al core original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		cpu_core_loop_no_nested();
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_core,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("Core id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al core original
	                cpu_core_loop_no_nested();
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
        	}

		else {
			//Hay anterior. Llamarlo
			//if (t_estados<20) printf ("Calling previous element in list. Name: %s\n",actual->function_name);
			actual->funcion(0,0); //los parametros 0,0 no se usan, se hace solo porque es una funcion generica de dos variables
		}
	}
}


//
//FIN Funciones de anidacion de core mediante listas nested
//

/*
Los siguientes 4 secciones generados mediante:
cat template_nested_peek.tpl | sed 's/NOMBRE_FUNCION/peek_byte/g' > debug_nested_functions.c
cat template_nested_peek.tpl | sed 's/NOMBRE_FUNCION/peek_byte_no_time/g' >> debug_nested_functions.c
cat template_nested_poke.tpl | sed 's/NOMBRE_FUNCION/poke_byte/g' >> debug_nested_functions.c
cat template_nested_poke.tpl | sed 's/NOMBRE_FUNCION/poke_byte_no_time/g' >> debug_nested_functions.c

*/

#include "debug_nested_functions.c"




//Inicializar punteros de cambio de funciones a NULL
void debug_nested_cores_pokepeek_init(void)
{
        nested_list_core=NULL;
        nested_list_poke_byte=NULL;
        nested_list_poke_byte_no_time=NULL;
        nested_list_peek_byte=NULL;
        nested_list_peek_byte_no_time=NULL;
}


void debug_dump_nested_add_string(char *string_inicial, char *string_to_add)
{
  //Agregar string_to_add a string. Suponemos que si cadena vacia, habra un 0 al inicio
  //No comprobamos overflow

  int longitud=strlen(string_inicial);

  strcpy(&string_inicial[longitud],string_to_add);
}

void debug_dump_nested_print(char *string_inicial, char *string_to_print)
{
  if (string_inicial==NULL) {
    printf ("%s",string_to_print);
  }
  else {
    debug_dump_nested_add_string(string_inicial,string_to_print);
  }
}

//Si result es NULL, lo muestra por salida standard. Sino, lo muestra por pantalla
void debug_dump_nested_functions(char *result)
{

  if (result!=NULL) {
    result[0]=0;
  }
  /*Ver en cada caso que haya algo en la lista y que ademas,
  el handler (por ejemplo, cpu_core_loop) apunte a handler nested
  Sucede que si por ejemplo activamos kartusho, y luego hacemos un smartload,
  el kartusho se desactiva, pero la lista contiene funciones nested, aunque los handler de peek y poke
  apuntan a los normales y no a kartusho (como debe ser)
  */

	if (nested_list_core!=NULL && cpu_core_loop==cpu_core_loop_nested_handler) {
		debug_dump_nested_print (result,"\nNested Core functions\n");
		debug_test_needed_adelante(nested_list_core,result);
	}

	if (nested_list_poke_byte!=NULL && poke_byte==poke_byte_nested_handler) {
		debug_dump_nested_print (result,"\nNested poke_byte functions\n");
		debug_test_needed_adelante(nested_list_poke_byte,result);
	}

	if (nested_list_poke_byte_no_time!=NULL && poke_byte_no_time==poke_byte_no_time_nested_handler) {
		debug_dump_nested_print (result,"\nNested poke_byte_no_time functions\n");
		debug_test_needed_adelante(nested_list_poke_byte_no_time,result);
	}

	if (nested_list_peek_byte!=NULL && peek_byte==peek_byte_nested_handler) {
		debug_dump_nested_print (result,"\nNested peek_byte functions\n");
		debug_test_needed_adelante(nested_list_peek_byte,result);
	}

	if (nested_list_peek_byte_no_time!=NULL && peek_byte_no_time==peek_byte_no_time_nested_handler) {
		debug_dump_nested_print (result,"\nNested peek_byte_no_time functions\n");
		debug_test_needed_adelante(nested_list_peek_byte_no_time,result);
	}
}


//Funcion de debug para cambiar valor registro
//Entrada: cadena de texto. Tipo DE=0234H
//Salida: 0 si ok. Diferente de 0 si error

int debug_change_register(char *texto)
{
	//Primero buscar hasta caracter =
	char texto_registro[100];
	unsigned int valor_registro;

	texto_registro[0]=0;

	int i;

	for (i=0;texto[i] && texto[i]!='=';i++) {
		texto_registro[i]=texto[i];
	}

	if (texto[i]==0) return 1; //Llegado hasta final de cadena y no hay igual
	texto_registro[i]=0;

	//Saltamos el =
	i++;

	if (texto[i]==0) return 2; //No hay nada despues del igual

	//Parsear valor
	valor_registro=parse_string_to_number(&texto[i]);

  if (CPU_IS_SCMP) {

  }

	//Cambiar registros
  //Motorola
	else if (CPU_IS_MOTOROLA) {


    if (!strcasecmp(texto_registro,"PC")) {
			m68k_set_reg(M68K_REG_PC,valor_registro);
			return 0;
		}

    else if (!strcasecmp(texto_registro,"D0")) {
			m68k_set_reg(M68K_REG_D0,valor_registro);
			return 0;
		}

    else if (!strcasecmp(texto_registro,"D1")) {
			m68k_set_reg(M68K_REG_D1,valor_registro);
			return 0;
		}

    else if (!strcasecmp(texto_registro,"D2")) {
      m68k_set_reg(M68K_REG_D2,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D3")) {
      m68k_set_reg(M68K_REG_D3,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D4")) {
      m68k_set_reg(M68K_REG_D4,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D5")) {
      m68k_set_reg(M68K_REG_D5,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D6")) {
      m68k_set_reg(M68K_REG_D6,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D7")) {
      m68k_set_reg(M68K_REG_D7,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A0")) {
      m68k_set_reg(M68K_REG_A0,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A1")) {
      m68k_set_reg(M68K_REG_A1,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A2")) {
      m68k_set_reg(M68K_REG_A2,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A3")) {
      m68k_set_reg(M68K_REG_A3,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A4")) {
      m68k_set_reg(M68K_REG_A4,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A5")) {
      m68k_set_reg(M68K_REG_A5,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A6")) {
      m68k_set_reg(M68K_REG_A6,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A7")) {
      m68k_set_reg(M68K_REG_A7,valor_registro);
      return 0;
    }

//TODO el resto de registros...

	}

  //Z80
	else {
		if (!strcasecmp(texto_registro,"PC")) {
			reg_pc=valor_registro;
			return 0;
		}

		else if (!strcasecmp(texto_registro,"SP")) {
                        reg_sp=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IX")) {
                        reg_ix=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IY")) {
                        reg_iy=valor_registro;
                        return 0;
                }

    else if (!strcasecmp(texto_registro,"AF")) {
      reg_a=value_16_to_8h(valor_registro);
      Z80_FLAGS=value_16_to_8l(valor_registro);
      return 0;
    }


              


		else if (!strcasecmp(texto_registro,"BC")) {
                        reg_bc=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"DE")) {
                        reg_de=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"HL")) {
                        reg_hl=valor_registro;
                        return 0;
                }

                else if (!strcasecmp(texto_registro,"AF'")) {
                  reg_a_shadow=value_16_to_8h(valor_registro);
                  Z80_FLAGS_SHADOW=value_16_to_8l(valor_registro);
                  return 0;
                }

                else if (!strcasecmp(texto_registro,"BC'")) {
                  reg_b_shadow=value_16_to_8h(valor_registro);
                  reg_c_shadow=value_16_to_8l(valor_registro);
                  return 0;
                }

                else if (!strcasecmp(texto_registro,"DE'")) {
                  reg_d_shadow=value_16_to_8h(valor_registro);
                  reg_e_shadow=value_16_to_8l(valor_registro);
                  return 0;
                }

                else if (!strcasecmp(texto_registro,"HL'")) {
                  reg_h_shadow=value_16_to_8h(valor_registro);
                  reg_l_shadow=value_16_to_8l(valor_registro);
                  return 0;
                }


      


		else if (!strcasecmp(texto_registro,"A")) {
                        reg_a=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"B")) {
                        reg_b=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"C")) {
                        reg_c=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"D")) {
                        reg_d=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"E")) {
                        reg_e=valor_registro;
                        return 0;
                }

                else if (!strcasecmp(texto_registro,"F")) {
                                    Z80_FLAGS=valor_registro;
                                    return 0;
                            }

		else if (!strcasecmp(texto_registro,"H")) {
                        reg_h=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"L")) {
                        reg_l=valor_registro;
                        return 0;
                }



else if (!strcasecmp(texto_registro,"A'")) {
                    reg_a_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"B'")) {
                    reg_b_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"C'")) {
                    reg_c_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"D'")) {
                    reg_d_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"E'")) {
                    reg_e_shadow=valor_registro;
                    return 0;
            }

            else if (!strcasecmp(texto_registro,"F'")) {
                                Z80_FLAGS_SHADOW=valor_registro;
                                return 0;
                        }

else if (!strcasecmp(texto_registro,"H'")) {
                    reg_h_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"L'")) {
                    reg_l_shadow=valor_registro;
                    return 0;
            }





		else if (!strcasecmp(texto_registro,"I")) {
                        reg_i=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"R")) {
                        reg_r=(valor_registro&127);
			reg_r_bit7=(valor_registro&128);
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IFF1")) {
                    iff1.v=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IFF2")) {
                    iff2.v=valor_registro;
                        return 0;
                }				


	}

	return 3;

}

//Indice entre 0 y MAX_BREAKPOINTS_CONDITIONS-1
void debug_set_breakpoint(int breakpoint_index,char *condicion)
{

    if (breakpoint_index<0 || breakpoint_index>MAX_BREAKPOINTS_CONDITIONS-1) {
      debug_printf(VERBOSE_ERR,"Index out of range setting breakpoint");
      return;
    }


    strcpy(debug_breakpoints_conditions_array[breakpoint_index],condicion);

  	debug_breakpoints_conditions_saltado[breakpoint_index]=0;
  	debug_breakpoints_conditions_enabled[breakpoint_index]=1;

}

//Indice entre 0 y MAX_BREAKPOINTS_CONDITIONS-1
void debug_set_breakpoint_action(int breakpoint_index,char *accion)
{

    if (breakpoint_index<0 || breakpoint_index>MAX_BREAKPOINTS_CONDITIONS-1) {
      debug_printf(VERBOSE_ERR,"Index out of range setting breakpoint action");
      return;
    }


    strcpy(debug_breakpoints_actions_array[breakpoint_index],accion);

}

void debug_view_basic(char *results_buffer)
{

  	char **dir_tokens;
  	int inicio_tokens;
  /*
                  dir_tokens=zx80_rom_tokens;

                  inicio_tokens=213;

  	int i=inicio_tokens;

  	while (i<256) {
  		printf ("%s\n",dir_tokens[i-inicio_tokens]);
  		i++;
  	}
  	return;

  */



  	z80_int dir;

  	int dir_inicio_linea;
  	int final_basic;



  	if (MACHINE_IS_SPECTRUM) {
  		//Spectrum

  		//PROG
  		dir_inicio_linea=peek_byte_no_time(23635)+256*peek_byte_no_time(23636);

  		//VARS
  		final_basic=peek_byte_no_time(23627)+256*peek_byte_no_time(23628);

  		dir_tokens=spectrum_rom_tokens;

  		inicio_tokens=163;

  	}

  	else if (MACHINE_IS_ZX81) {
  		//ZX81
  		dir_inicio_linea=16509;

  		//D_FILE
  		final_basic=peek_byte_no_time(0x400C)+256*peek_byte_no_time(0x400D);

  		dir_tokens=zx81_rom_tokens;

  		inicio_tokens=192;
  	}

          //else if (MACHINE_IS_ZX80) {
          else  {
  		//ZX80
                  dir_inicio_linea=16424;

                  //VARS
                  final_basic=peek_byte_no_time(0x4008)+256*peek_byte_no_time(0x4009);

                  dir_tokens=zx80_rom_tokens;

                  inicio_tokens=213;
          }


  	debug_printf (VERBOSE_INFO,"Start Basic: %d. End Basic: %d",dir_inicio_linea,final_basic);

          int index_buffer;



          index_buffer=0;

          int salir=0;

  	z80_int numero_linea;

  	z80_int longitud_linea;

  	//deberia ser un byte, pero para hacer tokens de pi,rnd, inkeys en zx81, que en el array estan en posicion al final
  	z80_int byte_leido;

  	int lo_ultimo_es_un_token;


  	while (dir_inicio_linea<final_basic && salir==0) {
  		lo_ultimo_es_un_token=0;
  		dir=dir_inicio_linea;
  		//obtener numero linea. orden inverso
  		//numero_linea=(peek_byte_no_time(dir++))*256 + peek_byte_no_time(dir++);
  		numero_linea=(peek_byte_no_time(dir++))*256;
  		numero_linea +=peek_byte_no_time(dir++);

  		//escribir numero linea
  		sprintf (&results_buffer[index_buffer],"%4d",numero_linea);
  		index_buffer +=4;

  		//obtener longitud linea. orden normal. zx80 no tiene esto
  		if (!MACHINE_IS_ZX80) {

  			//longitud_linea=(peek_byte_no_time(dir++))+256*peek_byte_no_time(dir++);
  			longitud_linea=(peek_byte_no_time(dir++));
  			longitud_linea += 256*peek_byte_no_time(dir++);

  			debug_printf (VERBOSE_DEBUG,"Line length: %d",longitud_linea);

  		}

  		else longitud_linea=65535;

  		//asignamos ya siguiente direccion.
  		dir_inicio_linea=dir+longitud_linea;

  		while (longitud_linea>0) {
  			byte_leido=peek_byte_no_time(dir++);
  			longitud_linea--;

  			if (MACHINE_IS_ZX8081) {
  				//numero
  				if (byte_leido==126) byte_leido=14;

  				else if (byte_leido==118) byte_leido=13;


  				//Convertimos a ASCII
  				else {

  					if (MACHINE_IS_ZX81) {
  						if (byte_leido>=64 && byte_leido<=66) {
  							//tokens EN ZX81, 64=RND, 65=PI, 66=INKEY$

  							//para que no haga conversion de byte leido, sino token
  							byte_leido=byte_leido-64+256;
  						}
  					}



  					if (byte_leido>=128 && byte_leido<=191) {
  						//inverso
  						byte_leido-=128;
  					}



  					if (byte_leido<=63) {
  						if (MACHINE_IS_ZX81) byte_leido=da_codigo_zx81_no_artistic(byte_leido);
  						else byte_leido=da_codigo_zx80_no_artistic(byte_leido);
  					}

  					//Entre 64 y 127, es codigo desconocido
  					else if (byte_leido>=64 && byte_leido<=127) {
  						byte_leido='?';
  					}
  				}



  			}


  			if (byte_leido>=32 && byte_leido<=127) {
  				results_buffer[index_buffer++]=byte_leido;
  				lo_ultimo_es_un_token=0;
  			}

  			else if (byte_leido>=inicio_tokens) {

  				if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX80) {
  					//si lo de antes no es un token, meter espacio
  					if (lo_ultimo_es_un_token==0) {
  						results_buffer[index_buffer++]=' ';
  					}
  				}


  				int indice_token=byte_leido-inicio_tokens;
  				//printf ("byte_leido: %d inicio_tokens: %d indice token: %d\n",byte_leido,inicio_tokens,indice_token);
  				sprintf (&results_buffer[index_buffer],"%s ",dir_tokens[indice_token]);
  				index_buffer +=strlen(dir_tokens[indice_token])+1;
  				lo_ultimo_es_un_token=1;
  			}



  			else if (byte_leido==14) {
  				//representacion numero. saltar
  				dir +=5;
  				longitud_linea -=5;
  				lo_ultimo_es_un_token=0;
  			}


  			else if (byte_leido==13) {
  				//ignorar salto de linea excepto en zx80
  				if (MACHINE_IS_ZX80) {
  					longitud_linea=0;
  					dir_inicio_linea=dir;
  				}
  			}

  			else {
  				results_buffer[index_buffer++]='?';
  			}



  			//controlar maximo
  			//1024 bytes de margen
  			if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-1024) {
                          	debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first ones");
                  	        //forzar salir
  				longitud_linea=0;
          	                salir=1;
  	                }


  		}


                  //controlar maximo
                  //1024 bytes de margen
                  if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-1024) {
                          debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first ones");
                          //forzar salir
                          salir=1;
                  }


  		//meter dos saltos de linea
  		results_buffer[index_buffer++]='\n';
  		results_buffer[index_buffer++]='\n';

  	}


          results_buffer[index_buffer]=0;

}

void debug_get_ioports(char *stats_buffer)
{

          //int index_op,
  	int index_buffer;



          //margen suficiente para que quepa una linea y un contador int de 32 bits...
          //aunque si pasa el ancho de linea, la rutina de generic_message lo troceara
          char buf_linea[64];

          index_buffer=0;

  	if (MACHINE_IS_SPECTRUM) {
  		sprintf (buf_linea,"Spectrum FE port: %02X\n",out_254_original_value);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		//Spectra
  		if (spectra_enabled.v) {
  			sprintf (buf_linea,"Spectra video mode register: %02X\n",spectra_display_mode_register);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}

  		//ULAplus
  		if (ulaplus_enabled.v) {
  			sprintf (buf_linea,"ULAplus video mode register: %02X\n",ulaplus_mode);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  			sprintf (buf_linea,"ULAplus extended video mode register: %02X\n",ulaplus_extended_mode);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}

  		//Timex Video
  		if (timex_video_emulation.v) {
  			sprintf (buf_linea,"Timex FF port: %02X\n",timex_port_ff);
                          sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
                  }


  	}

  	if (MACHINE_IS_TIMEX_TS2068 || MACHINE_IS_CHLOE_280SE || MACHINE_IS_PRISM) {
  		sprintf (buf_linea,"Timex F4 port: %02X\n",timex_port_f4);
                  sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
          }

  	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED || MACHINE_IS_PRISM || MACHINE_IS_CHLOE || superupgrade_enabled.v || MACHINE_IS_CHROME || TBBLUE_MACHINE_128_P2 || TBBLUE_MACHINE_P2A) {
                  sprintf (buf_linea,"Spectrum 7FFD port: %02X\n",puerto_32765);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
          }

  	if (MACHINE_IS_SPECTRUM_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED || MACHINE_IS_PRISM || superupgrade_enabled.v || MACHINE_IS_CHROME || TBBLUE_MACHINE_P2A) {
  		sprintf (buf_linea,"Spectrum 1FFD port: %02X\n",puerto_8189);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

    if (superupgrade_enabled.v) {
      sprintf (buf_linea,"Superupgrade 43B port: %02X\n",superupgrade_puerto_43b);
      sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
    }

  	if (MACHINE_IS_TBBLUE) {
		sprintf (buf_linea,"\nTBBlue port 123b: %02X\n",tbblue_port_123b);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);  

  								sprintf (buf_linea,"\nTBBlue last register: %02X\n",tbblue_last_register);
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


  								sprintf (buf_linea,"TBBlue Registers:\n");
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  								int index_ioport;
  								for (index_ioport=0;index_ioport<99;index_ioport++) {
  									//sprintf (buf_linea,"%02X : %02X \n",index_ioport,tbblue_registers[index_ioport]);
  									sprintf (buf_linea,"%02X : %02X \n",index_ioport,tbblue_get_value_port_register(index_ioport) );
  									sprintf (&stats_buffer[index_buffer],"%s",buf_linea);
  									index_buffer +=strlen(buf_linea);
  								}
  	}

	if (MACHINE_IS_TSCONF) {

  								sprintf (buf_linea,"TSConf Registers:\n");
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  								int index_ioport;
  								for (index_ioport=0;index_ioport<256;index_ioport++) {
  									sprintf (buf_linea,"%02X : %02X \n",index_ioport,tsconf_af_ports[index_ioport] );
  									sprintf (&stats_buffer[index_buffer],"%s",buf_linea);
  									index_buffer +=strlen(buf_linea);
  								}
  	}

  	//Registros ULA2 de Prism, paginacion, etc
  	if (MACHINE_IS_PRISM) {

  		sprintf (buf_linea,"\nPrism EE3B port: %02X\n",prism_rom_page);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"ULA2:\n");
                  sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

                  int i;
                  for (i=0;i<16;i++) {
                          sprintf (buf_linea,"%02X:  %02X\n",i,prism_ula2_registers[i]);
                          sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
                  }


          }



  	//Esto no en Z88
  	if (ay_chip_present.v && (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081)) {
  		int chips=ay_retorna_numero_chips();
  		int j;
  		for (j=0;j<chips;j++) {
  			sprintf (buf_linea,"\nAY-3-8912 chip %d:\n",j);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


                  	int i;
  	                for (i=0;i<16;i++) {
          	                sprintf (buf_linea,"%02X:  %02X\n",i,ay_3_8912_registros[j][i]);
                  	        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	                }

  		}


  	}

  	if (MACHINE_IS_Z88) {
  		sprintf (buf_linea,"Z88 Blink:\n\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"SBR:  %04X\n",blink_sbr);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		int i;
  		for (i=0;i<4;i++) {
  			sprintf (buf_linea,"PB%d:  %04X\n",i,blink_pixel_base[i]);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}

  		for (i=0;i<5;i++) {
  			sprintf (buf_linea,"TIM%d: %04X\n",i,blink_tim[i]);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}



  		sprintf (buf_linea,"COM:  %02X\n",blink_com);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"INT:  %02X\n",blink_int);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"STA:  %02X\n",blink_sta);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"EPR:  %02X\n",blink_epr);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"TMK:  %02X\n",blink_tmk);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"TSTA: %02X\n",blink_tsta);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

  	if (MACHINE_IS_ZXUNO) {
                  sprintf (buf_linea,"\nZX-Uno FC3B port: %02X\n",last_port_FC3B);
                  sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"ZX-Uno Registers:\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		int index_ioport;
  		for (index_ioport=0;index_ioport<256;index_ioport++) {
  			sprintf (buf_linea,"%02X : %02X \n",index_ioport,zxuno_ports[index_ioport]);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea);
  			index_buffer +=strlen(buf_linea);
  		}

  	}

  	if (MACHINE_IS_ZX8081) {
  		sprintf (buf_linea,"ZX80/81 last out port value: %02X\n",zx8081_last_port_write_value);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

          stats_buffer[index_buffer]=0;

}

int debug_if_breakpoint_action_menu(int index)
{
  //Si accion nula o menu o break
  if (debug_breakpoints_actions_array[index][0]==0 ||
    !strcmp(debug_breakpoints_actions_array[index],"menu") ||
    !strcmp(debug_breakpoints_actions_array[index],"break")
  )  return 1;

  return 0;
}


//Parseo de parametros de comando.
#define ACTION_MAX_PARAMETERS_COMMAND 10
//array de punteros a comando y sus argumentos
char *breakpoint_action_command_argv[ACTION_MAX_PARAMETERS_COMMAND];
int breakpoint_action_command_argc;

//Separar comando con codigos 0 y rellenar array de parametros
void breakpoint_action_parse_commands_argvc(char *texto)
{
  breakpoint_action_command_argc=util_parse_commands_argvc(texto, breakpoint_action_command_argv, ACTION_MAX_PARAMETERS_COMMAND);

}



void debug_run_action_breakpoint(char *comando)
{
                                //Gestion acciones
                        debug_printf (VERBOSE_DEBUG,"Full command: %s",comando);

int i;

                                                                //Interpretar comando hasta espacio o final de linea
                                                                char comando_sin_parametros[1024];

                                                                for (i=0;comando[i] && comando[i]!=' ' && comando[i]!='\n' && comando[i]!='\r';i++) {
                                                                        comando_sin_parametros[i]=comando[i];
                                                                }

                                                                comando_sin_parametros[i]=0;

        debug_printf (VERBOSE_DEBUG,"Command without parameters: [%s]",comando_sin_parametros);


        char parametros[1024];
        parametros[0]=0;
        int pindex=0;
        if (comando[i]==' ') {
                i++;
                for (;comando[i] && comando[i]!='\n' && comando[i]!='\r';i++,pindex++) {
                        parametros[pindex]=comando[i];
                }
        }

        parametros[pindex]=0;


        debug_printf (VERBOSE_DEBUG,"Action parameters: [%s]",parametros);

        //A partir de aqui se tiene:
        //variable comando_sin_parametros: comando tal cual inicial, sin parametros
        //variable parametros: todos los comandos tal cual se han escrito, son sus espacios y todos

        //Luego los comandos que necesiten parsear parametros pueden hacer:
        //llamar a breakpoint_action_parse_commands_argvc para los comandos de 1 o mas parametros
        //comandos de 1 solo parametro pueden usar tal cual la variable parametros. Util tambien para 1 solo parametro con espacios


        //Separar parametros
	       //breakpoint_action_parse_commands_argvc(parametros);

	//debug_printf (VERBOSE_DEBUG,"Total parameters: %d",breakpoint_action_command_argc);

	//for (i=0;i<breakpoint_action_command_argc;i++) {
	//	debug_printf (VERBOSE_DEBUG,"Parameter %d : [%s]",i,breakpoint_action_command_argv[i]);
	//}

    //Gestion parametros
    if (!strcmp(comando_sin_parametros,"write")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<2) debug_printf (VERBOSE_DEBUG,"Command needs two parameters");
      else {
        unsigned int direccion;
        z80_byte valor;

        direccion=parse_string_to_number(breakpoint_action_command_argv[0]);
        valor=parse_string_to_number(breakpoint_action_command_argv[1]);

        debug_printf (VERBOSE_DEBUG,"Running write command address %d value %d",direccion,valor);

        poke_byte_z80_moto(direccion,valor);
      }
    }

    else if (!strcmp(comando_sin_parametros,"call")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        unsigned int direccion;

        direccion=parse_string_to_number(breakpoint_action_command_argv[0]);

        debug_printf (VERBOSE_DEBUG,"Running call command address : %d",direccion);
        if (CPU_IS_MOTOROLA) debug_printf (VERBOSE_DEBUG,"Unimplemented call command for motorola");
        else {
          push_valor(reg_pc);
          reg_pc=direccion;
        }
      }
    }

    else if (!strcmp(comando_sin_parametros,"printc")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        unsigned int caracter;

        caracter=parse_string_to_number(breakpoint_action_command_argv[0]);

        debug_printf (VERBOSE_DEBUG,"Running printc command character: %d",caracter);

        printf ("%c",caracter);
      }
    }

    else if (!strcmp(comando_sin_parametros,"printe")) {
      if (parametros[0]==0) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running printe command : %s",parametros);
        char resultado_expresion[256];
        debug_watches_loop(parametros,resultado_expresion);
        printf ("%s\n",resultado_expresion);
      }
    }

    else if (!strcmp(comando_sin_parametros,"prints")) {
      if (parametros[0]==0) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running prints command : %s",parametros);
        printf ("%s\n",parametros);
      }
    }


    else if (!strcmp(comando_sin_parametros,"quicksave")) {
      debug_printf (VERBOSE_DEBUG,"Running quicksave command");
      snapshot_quick_save(NULL);
    }

    else if (!strcmp(comando_sin_parametros,"set-register")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running set-register command : %s",breakpoint_action_command_argv[0]);
        debug_change_register(breakpoint_action_command_argv[0]);
      }
    }

    else {
      debug_printf (VERBOSE_DEBUG,"Unknown command %s",comando_sin_parametros);
    }

}


//Estas funciones de debug_registers_get_mem_page_XXXX sin extended, son a borrar tambien



void debug_run_until_return_interrupt(void)
{
        //Ejecutar hasta que registro PC vuelva a su valor anterior o lleguemos a un limite
        //873600 instrucciones es 50 frames de instrucciones de 4 t-estados (69888/4*50)
        int limite_instrucciones=0;
        int salir=0;
        while (limite_instrucciones<873600 && salir==0) {
                if (reg_pc==debug_core_lanzado_inter_retorno_pc_nmi ||
                reg_pc==debug_core_lanzado_inter_retorno_pc_maskable) {
                        debug_printf (VERBOSE_DEBUG,"PC=0x%04X is now on the interrupt return address. Returning",reg_pc);
                        salir=1;
                }
                else {
                        debug_printf (VERBOSE_DEBUG,"Running and step over interrupt handler. PC=0x%04X TSTATES=%d",reg_pc,t_estados);
                        cpu_core_loop();
                        limite_instrucciones++;
                }
        }
}


//Retorna la pagina mapeada para el segmento
void debug_registers_get_mem_page_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{

        //Si es dandanator
        if (segmento==0 && dandanator_enabled.v && dandanator_switched_on.v==1) {
                sprintf (texto_pagina_short,"DB%d",dandanator_active_bank);
                sprintf (texto_pagina,"Dandanator Block %d",dandanator_active_bank);
                return;
        }

        //Si es kartusho
        if (segmento==0 && kartusho_enabled.v==1) {
                sprintf (texto_pagina_short,"KB%d",kartusho_active_bank);
                sprintf (texto_pagina,"Kartusho Block %d",kartusho_active_bank);
                return;
        }

        //Si es betadisk
        if (segmento==0 && betadisk_enabled.v && betadisk_active.v) {
                sprintf (texto_pagina_short,"BDSK");
                sprintf (texto_pagina,"Betadisk ROM");
                return;
        }

        //Si es superupgrade
        if (superupgrade_enabled.v) {
                if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                        //ROM
                        sprintf (texto_pagina_short,"RO%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                        sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                }

                else {
                        //RAM
                        sprintf (texto_pagina_short,"RA%d",debug_paginas_memoria_mapeadas[segmento]);
                        sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
                }
                return;

        }

        //Con multiface
        if (segmento==0 && multiface_enabled.v && multiface_switched_on.v) {
                strcpy(texto_pagina_short,"MLTF");
                strcpy(texto_pagina,"Multiface ROM");
                return;
        }

        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM
                sprintf (texto_pagina_short,"RO%X",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %X",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"RA%X",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %X",debug_paginas_memoria_mapeadas[segmento]);
        }
}



//Retorna la pagina mapeada para el segmento en zxuno
void debug_registers_get_mem_page_zxuno_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"RO%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"RA%02d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %02d",debug_paginas_memoria_mapeadas[segmento]);
        }

}


//Retorna la pagina mapeada para el segmento en tbblue
void debug_registers_get_mem_page_tbblue_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"O%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"A%d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
        }

}


//Retorna la pagina mapeada para el segmento en tsconf
void debug_registers_get_mem_page_tsconf_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"O%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"A%d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
        }

}

//Retorna la pagina mapeada para el segmento en baseconf
void debug_registers_get_mem_page_baseconf_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"O%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"A%d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
        }

}

//Retorna numero de segmentos en uso
int debug_get_memory_pages_extended(debug_memory_segment *segmentos)
{

	//Por si caso, inicializamos todos los strings a ""
	//debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
	int i;
	for (i=0;i<MAX_DEBUG_MEMORY_SEGMENTS;i++) {
		segmentos[i].longname[0]=0;
		segmentos[i].shortname[0]=0;
	}

	//Por defecto
	int segmentos_totales=2;

	strcpy(segmentos[0].longname,"System ROM");
	strcpy(segmentos[0].shortname,"ROM");
	segmentos[0].start=0;
	segmentos[0].length=16384;

        strcpy(segmentos[1].longname,"System RAM");
        strcpy(segmentos[1].shortname,"RAM");
        segmentos[1].start=16384;
        segmentos[1].length=49152;



/*

#define MAX_DEBUG_MEMORY_SEGMENTS 8

struct s_debug_memory_segment {
        //texto largo del nombre del segmento
        char longname[100];

	//texto corto
	char shortname[32];

	//Primera direccion del segmento
	int start;

	//Longitud del segmento
	int length;


};

typedef struct s_debug_memory_segment debug_memory_segment;
*/



   //Paginas memoria
      if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 ||  superupgrade_enabled.v || MACHINE_IS_CHROME) {
		segmentos_totales=4;
                                  int pagina;

        for (pagina=0;pagina<4;pagina++) {

                           debug_registers_get_mem_page_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
				segmentos[pagina].length=16384;
				segmentos[pagina].start=16384*pagina;

          }

    }



	if (MACHINE_IS_TBBLUE) {
                                                      int pagina;
                                                      //4 paginas, texto 5 caracteres max
				segmentos_totales=8;

                            for (pagina=0;pagina<8;pagina++) {

                                                            //Caso tbblue y modo config en pagina 0
                                                            if (pagina==0 || pagina==1) {
                                                                    //z80_byte maquina=(tbblue_config1>>6)&3;
                                                                    z80_byte maquina=(tbblue_registers[3])&3;
                                                                    if (maquina==0){
                                                                            if (tbblue_bootrom.v) {
											strcpy (segmentos[pagina].shortname,"RO");
											strcpy (segmentos[pagina].longname,"ROM");
										}
                                                                            else {
                                                                                    z80_byte romram_page=(tbblue_registers[4]&31);
                                                                                    sprintf (segmentos[pagina].shortname,"SR%d",romram_page);
                                                                                    sprintf (segmentos[pagina].longname,"SRAM %d",romram_page);
                                                                            }
                                                                    }
                                                                    else debug_registers_get_mem_page_tbblue_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                                                            }
                                                            else {
                                                                    debug_registers_get_mem_page_tbblue_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                                                            }

				segmentos[pagina].length=8192;
                                segmentos[pagina].start=8192*pagina;
                            }


                      //D5


      }


                 //Si dandanator y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && dandanator_enabled.v && dandanator_switched_on.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si kartusho y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && kartusho_enabled.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si betadisk y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && betadisk_enabled.v && betadisk_active.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si multiface y maquina 48kb. TODO. Si esta dandanator y tambien multiface, muestra siempre dandanator
                        if (MACHINE_IS_SPECTRUM_16_48 && multiface_enabled.v && multiface_switched_on.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }



/*
   if (tbblue_bootrom.v) {
											strcpy (segmentos[pagina].shortname,"RO");
											strcpy (segmentos[pagina].longname,"ROM");
										}
                                                                            else {
                                                                                    z80_byte romram_page=(tbblue_registers[4]&31);
                                                                                    sprintf (segmentos[pagina].shortname,"SR%d",romram_page);
                                                                                    sprintf (segmentos[pagina].longname,"SRAM %d",romram_page);
                                                                            }
                                                                    }
                                                                    else debug_registers_get_mem_page_tbblue_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
*/


//Paginas memoria
                          if (MACHINE_IS_ZXUNO ) {
                                  int pagina;
                                  //4 paginas, texto 6 caracteres max
                                  //char texto_paginas[4][7];
				segmentos_totales=4;

                                  for (pagina=0;pagina<4;pagina++) {
                                          debug_registers_get_mem_page_zxuno_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);

				segmentos[pagina].length=16384;
                                segmentos[pagina].start=16384*pagina;
                                  }
				if (ZXUNO_BOOTM_ENABLED) {
					sprintf (segmentos[0].shortname,"%s","RO");
					sprintf (segmentos[0].longname,"%s","Boot ROM");
				}


                          }


  			//BANK PAGES
  			if (MACHINE_IS_Z88) {
				int pagina;
				segmentos_totales=4;
				for (pagina=0;pagina<4;pagina++) {
	  				sprintf (segmentos[pagina].shortname,"BANK%02X",blink_mapped_memory_banks[pagina]);
	  				sprintf (segmentos[pagina].longname,"BANK %02X",blink_mapped_memory_banks[pagina]);
	                                segmentos[pagina].length=16384;
	                                segmentos[pagina].start=16384*pagina;
				}
  			}



  			//Paginas RAM en CHLOE
  			if (MACHINE_IS_CHLOE) {
  				//char texto_paginas[8][3];
  				//char tipo_memoria[3];
  				int pagina;
				segmentos_totales=8;
  				for (pagina=0;pagina<8;pagina++) {
  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_ROM)  {
						sprintf (segmentos[pagina].shortname,"R%d",debug_chloe_paginas_memoria_mapeadas[pagina]);
						sprintf (segmentos[pagina].longname,"ROM %d",debug_chloe_paginas_memoria_mapeadas[pagina]);
					}

  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_HOME) {
						sprintf (segmentos[pagina].shortname,"H%d",debug_chloe_paginas_memoria_mapeadas[pagina]);
						sprintf (segmentos[pagina].longname,"HOME %d",debug_chloe_paginas_memoria_mapeadas[pagina]);
					}
  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_DOCK) {
						sprintf (segmentos[pagina].shortname,"%s","DO");
						sprintf (segmentos[pagina].longname,"%s","DOCK");
					}
  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_EX)   {
						sprintf (segmentos[pagina].shortname,"%s","EX");
						sprintf (segmentos[pagina].longname,"%s","EX");
					}

        	                        segmentos[pagina].length=8192;
	                                segmentos[pagina].start=8192*pagina;
  				}

  			}




  			if (MACHINE_IS_PRISM) {
  				segmentos_totales=8;        
  				//Si modo ram en rom
          			if (puerto_8189 & 1) {



  		                  //Paginas RAM en PRISM
                                  //char texto_paginas[8][4];
                                  		//char tipo_memoria[3];
  		                         int pagina;
  		                        

  				   for (pagina=0;pagina<8;pagina++) {
                                                  sprintf (segmentos[pagina].shortname,"A%02d",debug_prism_paginas_memoria_mapeadas[pagina]);
                                                  sprintf (segmentos[pagina].longname,"RAM %02d",debug_prism_paginas_memoria_mapeadas[pagina]);

                                                  segmentos[pagina].length=8192;
	                               		  segmentos[pagina].start=8192*pagina;
  				  }

                                  


  				}



  			//Informacion VRAM en PRISM
  			  else {
  				//char texto_vram[32];




  				//Paginas RAM en PRISM
                                  //char texto_paginas[8][4];
                                  //char tipo_memoria[3];
                                  int pagina;
  				//TODO. como mostrar texto reducido aqui para paginas 2 y 3 segun vram aperture/no aperture??
                                  for (pagina=0;pagina<8;pagina++) {
                                          if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_ROM)  {
                                          		sprintf (segmentos[pagina].shortname,"O%02d",debug_prism_paginas_memoria_mapeadas[pagina]);
                                          		sprintf (segmentos[pagina].longname,"ROM %02d",debug_prism_paginas_memoria_mapeadas[pagina]);
                                          }

                                          if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_HOME) {
  						sprintf (segmentos[pagina].shortname,"H%02d",debug_prism_paginas_memoria_mapeadas[pagina]);
  						sprintf (segmentos[pagina].longname,"HOME %02d",debug_prism_paginas_memoria_mapeadas[pagina]);
  						if (pagina==2 || pagina==3) {
  							//La info de segmentos 2 y 3 (vram aperture si/no) se muestra de info anterior
  							sprintf (segmentos[pagina].shortname,"VRA");
  							sprintf (segmentos[pagina].longname,"VRAM");
  						}
  					}
            				if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_DOCK) {
            					sprintf (segmentos[pagina].shortname,"%s","DO");
            					sprintf (segmentos[pagina].longname,"%s","DOCK");
            				}

            				if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_EX)   {
            					sprintf (segmentos[pagina].shortname,"%s","EX");
            					sprintf (segmentos[pagina].longname,"%s","EX");
            				}

  					//Si pagina rom failsafe
  					if (prism_failsafe_mode.v) {
  						if (pagina==0 || pagina==1) {
  							sprintf (segmentos[pagina].shortname,"%s","FS");
  							sprintf (segmentos[pagina].longname,"%s","Failsafe ROM");
  						}
  					}


  					//Si paginando zona alta c000h con paginas 10,11 (que realmente son vram0 y 1) o paginas 14,15 (que realmente son vram 2 y 3)
  					if (pagina==6 || pagina==7) {
  						int pagina_mapeada=debug_prism_paginas_memoria_mapeadas[pagina];
  						int vram_pagina=-1;
  						switch (pagina_mapeada) {
  							case 10:
  								vram_pagina=0;
  							break;

  							case 11:
  								vram_pagina=1;
  							break;

  							case 14:
  								vram_pagina=2;
  							break;

  							case 15:
  								vram_pagina=3;
  							break;
  						}

  						if (vram_pagina!=-1) {
  							sprintf (segmentos[pagina].shortname,"V%d",vram_pagina);
  							sprintf (segmentos[pagina].longname,"VRAM %d",vram_pagina);
  						}
  					}

  					 segmentos[pagina].length=8192;
	                               	 segmentos[pagina].start=8192*pagina;
                                          //sprintf (texto_paginas[pagina],"%c%d",tipo_memoria,debug_prism_paginas_memoria_mapeadas[pagina]);
                                  }

                              


          }
  			}

  			  //Paginas RAM en TIMEX
                          if (MACHINE_IS_TIMEX_TS2068) {
                          		segmentos_totales=8;        
                                  //char texto_paginas[8][3];
                                  //char tipo_memoria;
                                  int pagina;
                                  for (pagina=0;pagina<8;pagina++) {
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_ROM)  {
                                          	sprintf (segmentos[pagina].shortname,"%s","RO");
                                          	sprintf (segmentos[pagina].longname,"%s","ROM");
                                          }
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_HOME) {
                                          	sprintf (segmentos[pagina].shortname,"%s","HO");
                                          	sprintf (segmentos[pagina].longname,"%s","HOME");
                                          }
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_DOCK) {
                                          	sprintf (segmentos[pagina].shortname,"%s","DO");
                                          	sprintf (segmentos[pagina].longname,"%s","DOCK");
                                          }
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_EX)   {
                                          	sprintf (segmentos[pagina].shortname,"%s","EX");
                                          	sprintf (segmentos[pagina].longname,"%s","EX");
                                          }
                                          //sprintf (texto_paginas[pagina],"%c%d",tipo_memoria,debug_timex_paginas_memoria_mapeadas[pagina]);


  					 segmentos[pagina].length=8192;
	                               	 segmentos[pagina].start=8192*pagina;
                                  }

                           
                          }

  			//Paginas RAM en CPC
  //#define CPC_MEMORY_TYPE_ROM 0
  //#define CPC_MEMORY_TYPE_RAM 1

  //extern z80_byte debug_cpc_type_memory_paged_read[];
  //extern z80_byte debug_cpc_paginas_memoria_mapeadas_read[];
  			if (MACHINE_IS_CPC) {
                        //char texto_paginas[4][5];
                        segmentos_totales=4;
                        int pagina;
                        for (pagina=0;pagina<4;pagina++) {
                            if (debug_cpc_type_memory_paged_read[pagina]==CPC_MEMORY_TYPE_ROM) {
  								sprintf (segmentos[pagina].shortname,"ROM%d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  								sprintf (segmentos[pagina].longname,"ROM %d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  					
  					   		}

                            if (debug_cpc_type_memory_paged_read[pagina]==CPC_MEMORY_TYPE_RAM) {
  								sprintf (segmentos[pagina].shortname,"RAM%d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  								sprintf (segmentos[pagina].longname,"RAM %d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  					  		}

							//Si es kartusho
        					if (pagina==0 && kartusho_enabled.v==1) {
                				sprintf (segmentos[pagina].shortname,"KB%d",kartusho_active_bank);
                				sprintf (segmentos[pagina].longname,"Kartusho Block %d",kartusho_active_bank);
							}


  							segmentos[pagina].length=16384;
	                        segmentos[pagina].start=16384*pagina;

                        }

            }

  			//Paginas RAM en SAM
  			if (MACHINE_IS_SAM) {
  				//char texto_paginas[4][6];
  				segmentos_totales=4;
                                  int pagina;
                                  for (pagina=0;pagina<4;pagina++) {
                                          if (sam_memory_paged_type[pagina]==0) {
                                                  sprintf (segmentos[pagina].shortname,"RAM%02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                                  sprintf (segmentos[pagina].longname,"RAM %02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                          }

                                          if (sam_memory_paged_type[pagina]) {
                                                  sprintf (segmentos[pagina].shortname,"ROM%02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                                  sprintf (segmentos[pagina].longname,"ROM %02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                          }

                                          segmentos[pagina].length=16384;
	                               	 segmentos[pagina].start=16384*pagina;

                                  }

                                 
                          }

                          if (MACHINE_IS_QL) {
                          	segmentos_totales=3;

                          		strcpy(segmentos[0].longname,"System ROM");
					strcpy(segmentos[0].shortname,"ROM");
					segmentos[0].start=0;
					segmentos[0].length=49152;


        				strcpy(segmentos[1].longname,"I/O Space");
        				strcpy(segmentos[1].shortname,"I/O");
        				segmentos[1].start=0x18000;
        				segmentos[1].length=16384;


        				strcpy(segmentos[2].longname,"System RAM");
        				strcpy(segmentos[2].shortname,"RAM");
        				segmentos[2].start=0x20000;
        				segmentos[2].length=QL_MEM_LIMIT+1-0x20000;

                          }


                            if (MACHINE_IS_MK14) {
                          	segmentos_totales=5;

                          		strcpy(segmentos[0].longname,"System ROM");
					strcpy(segmentos[0].shortname,"ROM");
					segmentos[0].start=0;
					segmentos[0].length=512;

					strcpy(segmentos[1].longname,"Shadow ROM");
					strcpy(segmentos[1].shortname,"SROM");
					segmentos[1].start=0x200;
					segmentos[1].length=512*3;


        				strcpy(segmentos[2].longname,"I/O Space");
        				strcpy(segmentos[2].shortname,"I/O");
        				segmentos[2].start=0x800;
        				segmentos[2].length=512;


        				strcpy(segmentos[3].longname,"Extended RAM");
        				strcpy(segmentos[3].shortname,"ERAM");
        				segmentos[3].start=0xb00;
        				segmentos[3].length=256;

        				strcpy(segmentos[4].longname,"Standard RAM");
        				strcpy(segmentos[4].shortname,"RAM");
        				segmentos[4].start=0xf00;
        				segmentos[4].length=256;

                          }



                          if (MACHINE_IS_TSCONF) {
                                                      int pagina;
                                                      //4 paginas, texto 5 caracteres max
				segmentos_totales=4;

                            	for (pagina=0;pagina<4;pagina++) {

                                   debug_registers_get_mem_page_tsconf_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                              

					segmentos[pagina].length=16384;
                                	segmentos[pagina].start=16384*pagina;
                           	 }


      			}

          if (MACHINE_IS_BASECONF) {
                                                      int pagina;
                                                      //4 paginas, texto 5 caracteres max
				segmentos_totales=4;

                            	for (pagina=0;pagina<4;pagina++) {

                                   debug_registers_get_mem_page_baseconf_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                              

					segmentos[pagina].length=16384;
                                	segmentos[pagina].start=16384*pagina;
                           	 }


      			}


  			//Fin paginas ram


      	//Caso divmmc
      			

      	if (diviface_enabled.v) {
      		if ( !   ( (diviface_control_register&128)==0 && diviface_paginacion_automatica_activa.v==0) )  {


			//Caso tbblue
			int div_segment_zero=1;
			int div_segment_one=1;

			if (MACHINE_IS_TBBLUE) {
				if (!(debug_paginas_memoria_mapeadas[0] & DEBUG_PAGINA_MAP_ES_ROM)) div_segment_zero=0;
				if (!(debug_paginas_memoria_mapeadas[1] & DEBUG_PAGINA_MAP_ES_ROM)) div_segment_one=0;
			}

			if (div_segment_zero) {

	      			strcpy(segmentos[0].longname,"Diviface");
				strcpy(segmentos[0].shortname,"DIV");	

			}

			if (div_segment_one) {

				//En maquinas de 8 segmentos, bloque 1 es 8192-16383
				if (segmentos_totales==8) {
				      	strcpy(segmentos[1].longname,"Diviface");
					strcpy(segmentos[1].shortname,"DIV");		
				}

			}
      		}
      	}


	return segmentos_totales;

}


//Devuelve texto estado pagina video(5/7) y si paginacion esta activa o no. Solo para maquinas spectrum y que no sean 16/48
void debug_get_paging_screen_state(char *s)
{

	//por defecto
	*s=0;

	if (!MACHINE_IS_SPECTRUM) return;

	if (MACHINE_IS_SPECTRUM_16_48) return;


	sprintf (s,"SCR%d %s", ( (puerto_32765&8) ? 7 : 5) ,  ( (puerto_32765&32) ? "PDI" : "PEN"  ) );


}



int si_cpu_step_over_jpret(void)
{
        if (CPU_IS_MOTOROLA || CPU_IS_SCMP) return 0;
        z80_byte opcode=peek_byte_no_time(reg_pc);

	debug_printf(VERBOSE_DEBUG,"cpu step over, first opcode at %04XH is %02XH",reg_pc,opcode);

        switch (opcode)
        {

                case 0xC3: // JP
                case 0xCA: // JP Z
                case 0xD2: // JP NC
                case 0xDA: // JP C
                case 0xE2: // JP PO
                case 0xE9: // JP (HL)
                case 0xEA: // JP PE
                case 0xF2: // JP P
                case 0xFA: // JP M

                case 0xC0: // RET NZ
                case 0xC8: // RET Z
                case 0xC9: // RET
                case 0xD0: // RET NC
                case 0xD8: // RET C
                case 0xE0: // RET PO
                case 0xE8: // RET PE
                case 0xF0: // RET P
                case 0xF8: // RET M

                        return 1;
                break;
        }

        return 0;

}


void debug_cpu_step_over(void)
{
  unsigned int direccion=get_pc_register();
  int longitud_opcode=debug_get_opcode_length(direccion);

  unsigned int direccion_final=direccion+longitud_opcode;
  direccion_final=adjust_address_space_cpu(direccion_final);


  //Parar hasta volver de la instruccion actual o cuando se produzca algun evento de apertura de menu, como un breakpoint
  menu_abierto=0;
  int salir=0;
  while (get_pc_register()!=direccion_final && !salir) {
    debug_core_lanzado_inter.v=0;
    cpu_core_loop();

    if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
        debug_run_until_return_interrupt();
    }

    if (menu_abierto) salir=1;
  }
}



int debug_get_opcode_length(unsigned int direccion)
{
  char buffer_retorno[101];
  size_t longitud_opcode;

  debugger_disassemble(buffer_retorno,100,&longitud_opcode,direccion);

  return longitud_opcode;

}
