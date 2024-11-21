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


#if defined(linux) || defined(__APPLE__)
#include <execinfo.h>
#endif


#include "cpu.h"
#include "debug.h"
#include "mem128.h"
#include "screen.h"
#include "zxvision.h"
#include "zx8081.h"
#include "operaciones.h"
#include "core_spectrum.h"
#include "core_zx8081.h"
#include "core_z88.h"
#include "core_ace.h"
#include "core_cpc.h"
#include "core_pcw.h"
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
#include "ifrom.h"
#include "diviface.h"
#include "betadisk.h"

#include "tsconf.h"

#include "core_reduced_spectrum.h"

#include "remote.h"
#include "charset.h"
#include "settings.h"
#include "expression_parser.h"
#include "atomic.h"
#include "core_msx.h"
#include "core_coleco.h"
#include "msx.h"
#include "coleco.h"
#include "core_sg1000.h"
#include "core_sms.h"
#include "sn76489an.h"
#include "core_svi.h"
#include "svi.h"
#include "vdp_9918a.h"
#include "ql_zx8302.h"
#include "ide.h"
#include "mmc.h"
#include "esxdos_handler.h"
#include "sms.h"
#include "menu_debug_cpu.h"
#include "transtape.h"
#include "hilow_barbanegra.h"
#include "specmate.h"
#include "phoenix.h"
#include "defcon.h"
#include "ramjet.h"
#include "dinamid3.h"
#include "interface007.h"
#include "pd765.h"
#include "pcw.h"
#include "if1.h"
#include "microdrive.h"
#include "lec.h"

struct timeval debug_timer_antes, debug_timer_ahora;




z80_bit menu_breakpoint_exception={0};

z80_bit debug_breakpoints_enabled={0};

//breakpoints de condiciones. nuevo formato para nuevo parser de tokens
token_parser debug_breakpoints_conditions_array_tokens[MAX_BREAKPOINTS_CONDITIONS][MAX_PARSER_TOKENS_NUM];

//watches. nuevo formato con parser de tokens
token_parser debug_watches_array[DEBUG_MAX_WATCHES][MAX_PARSER_TOKENS_NUM];


//Ultimo breakpoint activo+1 (o sea, despues del ultimo activo) para optimizar la comprobacion de breakpoints,
//asi solo se comprueba hasta este en vez de comprobarlos todos
int last_active_breakpoint=0;

//acciones a ejecutar cuando salta un breakpoint
char debug_breakpoints_actions_array[MAX_BREAKPOINTS_CONDITIONS][MAX_BREAKPOINT_CONDITION_LENGTH];

//A 0 si ese breakpoint no ha saltado. A 1 si ya ha saltado
int debug_breakpoints_conditions_saltado[MAX_BREAKPOINTS_CONDITIONS];

//A 1 si ese breakpoint esta activado. A 0 si no
int debug_breakpoints_conditions_enabled[MAX_BREAKPOINTS_CONDITIONS];



optimized_breakpoint optimized_breakpoint_array[MAX_BREAKPOINTS_CONDITIONS];



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

//Todos estos valores podrian ser z80_byte o z80_int, pero dado que al arrancar el ordenador
//estarian a 0, una condicion tipo MRA=0 o MWV=0 etc haria saltar esos breakpoints
//por tanto los inicializo a un valor fuera de rango de z80_int incluso

unsigned int debug_mmu_mrv=65536; //Memory Read Value (valor leido en peek)
unsigned int debug_mmu_mwv=65536; //Memory Write Value (valor escrito en poke)
unsigned int debug_mmu_prv=65536; //Port Read Value (valor leido en lee_puerto)
unsigned int debug_mmu_pwv=65536; //Port Write Value (valor escrito en out_port)

unsigned int debug_mmu_pra=65536; //Port Read Address (direccion usada en lee_puerto)
unsigned int debug_mmu_pwa=65536; //Port Write Address (direccion usada en out_port)

//Inicializar a algo invalido porque si no podria saltar el primer MRA=0 al leer de la rom,
//por tanto a -1 (y si no fuera por ese -1, podria ser un tipo z80_int en vez de int)
unsigned int debug_mmu_mra=65536; //Memory Read Addres (direccion usada peek)
unsigned int debug_mmu_mwa=65536; //Memory Write Address (direccion usada en poke)

//Anteriores valores para mra y mwa y mrv y mwv.
//Si es -1, no hay valor anterior
unsigned int anterior_debug_mmu_mra=65536;
unsigned int anterior_debug_mmu_mwa=65536;
unsigned int anterior_debug_mmu_mrv=65536;
unsigned int anterior_debug_mmu_mwv=65536;

//Array usado en memory-breakpoints
/*
Es equivalente a MRA o MWA pero mucho mas rapido
Valores:
0: no hay mem breakpoint para esa direccion
1: hay mem breakpoint de lectura para esa direccion
2: hay mem breakpoint de escritura para esa direccion
3: hay mem breakpoint de lectura o escritura para esa direccion
*/
z80_byte mem_breakpoint_array[65536];

char *mem_breakpoint_types_strings[]={
	"Disabled",
	"Read",
	"Write",
	"Read & Write"
};

/*
Pruebas de uso de cpu entre el parser clasico (versiones anteriores a la 7.1), el parser optimizado para PC=, MRA=, MWA, y
los mem_breakpoints:

mra=32768

Con zesarux de siempre: 66% de cpu
Con MRA optimizado: 44% de cpu
Con memory breakpoint: 44% de cpu
Mismo uso de cpu en los dos casos anteriores sin breakpoints, solo habilitando breakpoints: 44% cpu


Con 10 mra en optimizado: 48%
Con 10 memory breakpoints: 44%

*/

//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
int debug_exitrom=0;


//Avisa que el ultimo opcode ha sido un out a puerto, para poder hacer breakpoints con esto
int debug_fired_out=0;
//Avisa que el ultimo opcode ha sido un in a puerto, para poder hacer breakpoints con esto
int debug_fired_in=0;
//Avisa que se ha generado interrupcion, para poder hacer breakpoints con esto
int debug_fired_interrupt=0;

//Mensaje que ha hecho saltar el breakpoint
char catch_breakpoint_message[MAX_MESSAGE_CATCH_BREAKPOINT];

//Id indice breakpoint que ha saltado
//Si -1, no ha saltado por indice, quiza por un membreakpoint
int catch_breakpoint_index=0;

//Dice si para el core de spectrum en ese scanline se ha disparado interrupcion, para mostrarlo en el boder
int core_spectrum_executed_halt_in_this_scanline=0;

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



void debug_dump_nested_print(char *string_inicial, char *string_to_print);


//Usados en la carga de archivo de codigo fuente. Archivo crudo original
char *remote_raw_source_code_pointer=NULL;
int remote_tamanyo_archivo_raw_source_code=0;
//Puntero a indices en archivo raw source
int *remote_raw_source_code_indexes_pointer=NULL;
//Tamanyo de ese array
int remote_raw_source_code_indexes_total;

//Puntero a indices en archivo parsed source (lineas sin comentarios, con codigo real)
int *remote_parsed_source_code_indexes_pointer=NULL;
//Tamanyo de ese array
int remote_parsed_source_code_indexes_total;

//ruta al último código fuente cargado
char last_source_code_file[PATH_MAX];


//mostrar ademas mensajes de debug en consola con printf, adicionalmente de donde lo muestre ya (en curses, aa, caca salen en dentro ventana)
z80_bit debug_always_show_messages_in_console={0};

//Si volcar snapshot zsf cuando hay cpu_panic
z80_bit debug_dump_zsf_on_cpu_panic={0};

//Si ya se ha volcado snapshot zsf cuando hay cpu_panic, para evitar un segundo volcado (y siguientes) si se genera otro panic al hacer el snapshot
z80_bit dumped_debug_dump_zsf_on_cpu_panic={0};

//nombre del archivo volcado
char dump_snapshot_panic_name[PATH_MAX]="";


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


struct s_z88_basic_rom_tokens {
	z80_byte index;
	char token[20];
};


struct s_z88_basic_rom_tokens z88_basic_rom_tokens[]={
{0x80,"AND"},
{0x94,"ABS"},
{0x95,"ACS"},
{0x96,"ADVAL"},
{0x97,"ASC"},
{0x98,"ASN"},
{0x99,"ATN"},
{0xC6,"AUTO"},
{0x9A,"BGET"},
{0xD5,"BPUT"},
{0xFB,"COLOUR"},
{0xFB,"COLOR"},
{0xD6,"CALL"},
{0xD7,"CHAIN"},
{0xBD,"CHR$"},
{0xD8,"CLEAR"},
{0xD9,"CLOSE"},
{0xDA,"CLG"},
{0xDB,"CLS"},
{0x9B,"COS"},
{0x9C,"COUNT"},
{0xDC,"DATA"},
{0x9D,"DEG"},
{0xDD,"DEF"},
{0xC7,"DELETE"},
{0x81,"DIV"},
{0xDE,"DIM"},
{0xDF,"DRAW"},
{0xE1,"ENDPROC"},
{0xE0,"END"},
{0xE2,"ENVELOPE"},
{0x8B,"ELSE"},
{0xA0,"EVAL"},
{0x9E,"ERL"},
{0x85,"ERROR"},
{0xC5,"EOF"},
{0x82,"EOR"},
{0x9F,"ERR"},
{0xA1,"EXP"},
{0xA2,"EXT"},
{0xE3,"FOR"},
{0xA3,"FALSE"},
{0xA4,"FN"},
{0xE5,"GOTO"},
{0xBE,"GET$"},
{0xA5,"GET"},
{0xE4,"GOSUB"},
{0xE6,"GCOL"},
{0x93,"HIMEM"},
{0xE8,"INPUT"},
{0xE7,"IF"},
{0xBF,"INKEY$"},
{0xA6,"INKEY"},
{0xA8,"INT"},
{0xA7,"INSTR("},
{0xC9,"LIST"},
{0x86,"LINE"},
{0xC8,"LOAD"},
{0x92,"LOMEM"},
{0xEA,"LOCAL"},
{0xC0,"LEFT$("},
{0xA9,"LEN"},
{0xE9,"LET"},
{0xAB,"LOG"},
{0xAA,"LN"},
{0xC1,"MID$("},
{0xEB,"MODE"},
{0x83,"MOD"},
{0xEC,"MOVE"},
{0xED,"NEXT"},
{0xCA,"NEW"},
{0xAC,"NOT"},
{0xCB,"OLD"},
{0xEE,"ON"},
{0x87,"OFF"},
{0x84,"OR"},
{0x8E,"OPENIN"},
{0xAE,"OPENOUT"},
{0xAD,"OPENUP"},
{0xFF,"OSCLI"},
{0xF1,"PRINT"},
{0x90,"PAGE"},
{0x8F,"PTR"},
{0xAF,"PI"},
{0xF0,"PLOT"},
{0xB0,"POINT("},
{0xF2,"PROC"},
{0xB1,"POS"},
{0xCE,"PUT"},
{0xF8,"RETURN"},
{0xF5,"REPEAT"},
{0xF6,"REPORT"},
{0xF3,"READ"},
{0xF4,"REM"},
{0xF9,"RUN"},
{0xB2,"RAD"},
{0xF7,"RESTORE"},
{0xC2,"RIGHT$("},
{0xB3,"RND"},
{0xCC,"RENUMBER"},
{0x88,"STEP"},
{0xCD,"SAVE"},
{0xB4,"SGN"},
{0xB5,"SIN"},
{0xB6,"SQR"},
{0x89,"SPC"},
{0xC3,"STR$"},
{0xC4,"STRING$("},
{0xD4,"SOUND"},
{0xFA,"STOP"},
{0xB7,"TAN"},
{0x8C,"THEN"},
{0xB8,"TO"},
{0x8A,"TAB("},
{0xFC,"TRACE"},
{0x91,"TIME"},
{0xB9,"TRUE"},
{0xFD,"UNTIL"},
{0xBA,"USR"},
{0xEF,"VDU"},
{0xBB,"VAL"},
{0xBC,"VPOS"},
{0xFE,"WIDTH"},
{0xD3,"HIMEM"},
{0xD2,"LOMEM"},
{0xD0,"PAGE"},
{0xCF,"PTR"},
{0xD1,"TIME"},
{0x01,""}  //Importante este 01 final
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
  sprintf (buffer,"PC=%04x SP=%04x AF=%04x BC=%04x HL=%04x DE=%04x IX=%04x IY=%04x AF'=%04x BC'=%04x HL'=%04x DE'=%04x I=%02x R=%02x  "
                  "F=%c%c%c%c%c%c%c%c F'=%c%c%c%c%c%c%c%c MEMPTR=%04x IM%d IFF%c%c VPS: %d "
                  "MMU=%04x%04x%04x%04x%04x%04x%04x%04x",
  reg_pc,reg_sp,(reg_a<<8)|Z80_FLAGS,(reg_b<<8)|reg_c,(reg_h<<8)|reg_l,(reg_d<<8)|reg_e,reg_ix,reg_iy,(reg_a_shadow<<8)|Z80_FLAGS_SHADOW,(reg_b_shadow<<8)|reg_c_shadow,
  (reg_h_shadow<<8)|reg_l_shadow,(reg_d_shadow<<8)|reg_e_shadow,reg_i,(reg_r&127)|(reg_r_bit7&128),DEBUG_STRING_FLAGS,
  DEBUG_STRING_FLAGS_SHADOW,memptr,im_mode, DEBUG_STRING_IFF12 ,last_vsync_per_second,
  debug_paginas_memoria_mapeadas[0],debug_paginas_memoria_mapeadas[1],debug_paginas_memoria_mapeadas[2],debug_paginas_memoria_mapeadas[3],
  debug_paginas_memoria_mapeadas[4],debug_paginas_memoria_mapeadas[5],debug_paginas_memoria_mapeadas[6],debug_paginas_memoria_mapeadas[7]
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

void clear_mem_breakpoints(void)
{

	int i;

	for (i=0;i<65536;i++) {
		mem_breakpoint_array[i]=0;
	}
}

void init_breakpoints_table(void)
{
	int i;

	//for (i=0;i<MAX_BREAKPOINTS;i++) debug_breakpoints_array[i]=-1;

	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
		//debug_breakpoints_conditions_array[i][0]=0;


		debug_breakpoints_conditions_array_tokens[i][0].tipo=TPT_FIN;


	    	debug_breakpoints_actions_array[i][0]=0;
		debug_breakpoints_conditions_saltado[i]=0;
		debug_breakpoints_conditions_enabled[i]=0;

		optimized_breakpoint_array[i].optimized=0;
	}

        //for (i=0;i<MAX_BREAKPOINTS_PEEK;i++) debug_breakpoints_peek_array[i]=-1;


	clear_mem_breakpoints();
	last_active_breakpoint=0;


}


void init_watches_table(void)
{
	int i;

	for (i=0;i<DEBUG_MAX_WATCHES;i++) {
		debug_watches_array[i][0].tipo=TPT_FIN;
	}


}



//Dibuja la pantalla de panico
void screen_show_panic_screen(int xmax, int ymax)
{

    //int colores_rainbow[]={2+8,6+8,4+8,5+8,0};

	int x,y;


	int total_colores=5;
	int grueso_colores=8; //grueso de 8 pixeles cada franja

	//printf ("Filling colour bars up to %dX%d\n",xmax,ymax);


	for (x=0;x<xmax;x++) {
        int color=0;
		for (y=0;y<ymax;y++) {
			//scr_putpixel(x,y,(color&15) );
            scr_putpixel(x,y,screen_colores_rainbow[(color%total_colores)] );

			if ((y%grueso_colores)==grueso_colores-1) color++;

		}
	}
}

//Funcion para mostrar traza de llamadas, muy util para debugar
//Compilar sin optimizaciones, porque si no, hay llamadas que no se ven en la traza
//Compile with -g -rdynamic to show function names
//In Mac, with -g
//These functions on Mac OS X are available starting from Mac OS 10.5
void debug_exec_show_backtrace(void)
{

#if defined(linux) || defined(__APPLE__)
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

	//Liberar bloqueo de semaforo de print, por si acaso
	debug_printf_sem_init();

	//por si acaso, antes de hacer nada mas, vamos con el printf, para que muestre el error (si es que el driver de video lo permite)
	//hacemos pantalla de panic en xwindows y fbdev, y despues de finalizar el driver, volvemos a mostrar error
	cpu_panic_printf_mensaje(mensaje);

	debug_exec_show_backtrace();

	snap_dump_zsf_on_cpu_panic();

    cpu_panic_last_x=cpu_panic_last_y=0;

    cpu_panic_current_tinta=6;
    cpu_panic_current_papel=1;


	if (scr_end_pantalla!=NULL) {

		//si es xwindows o fbdev, mostramos panic mas mono
		if (si_complete_video_driver() ) {
			//quitar splash text por si acaso
			menu_splash_segundos=1;
			reset_welcome_message();




			//no tiene sentido tener el menu overlay abierto... o si?

			menu_overlay_activo=0;

            cpu_panic_xmax=screen_get_emulated_display_width_zoom_border_en();
            cpu_panic_ymax=screen_get_emulated_display_height_zoom_border_en();

            //Extenderlo al zx desktop
            if (screen_ext_desktop_enabled && scr_driver_can_ext_desktop() ) {
                cpu_panic_xmax +=screen_get_ext_desktop_width_zoom();
                cpu_panic_ymax +=screen_get_ext_desktop_height_zoom();
            }

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

            cpu_panic_printstring("\n");

			if (dumped_debug_dump_zsf_on_cpu_panic.v) {
				cpu_panic_printstring("\nDumped cpu panic snapshot:\n");
				cpu_panic_printstring(dump_snapshot_panic_name);
				cpu_panic_printstring("\non current directory");
				printf ("Dumped cpu panic snapshot: %s on current directory\n",dump_snapshot_panic_name);
			}
                                     //01234567890123456789012345678901
            cpu_panic_printstring("\nHave a nice day ;)\n");


            int cuenta_atras;

            for (cuenta_atras=20;cuenta_atras>=-1;cuenta_atras--) {

                char buffer_texto[32];

                if (cuenta_atras>=0) {
                    sprintf(buffer_texto,"%d ",cuenta_atras);
                    cpu_panic_printstring(buffer_texto);
                }
                else {
                    //En el -1, otro texto mas artistico
                    cpu_panic_printstring("Sayonara baby");
                }

                //Cambiamos coordenada a 0 para sobreescribir contador
                cpu_panic_last_x=0;

                scr_refresca_pantalla_solo_driver();

                //Para xwindows hace falta esto, sino no refresca
                scr_actualiza_tablas_teclado();

                sleep(1);
            }


			scr_end_pantalla();
		}

		else {
			scr_end_pantalla();
		}
	}


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

z_atomic_semaphore debug_printf_semaforo;

void debug_printf_sem_init(void)
{
	z_atomic_reset(&debug_printf_semaforo);
}


//Funciones de consola debug adicional en entorno gráfico
//muestra lo mismo que debug_printf en consola de texto por ejemplo
//de momento se llama: unnamed_console

char *debug_unnamed_console_memory_pointer=NULL;
int debug_unnamed_console_current_x=0;
int debug_unnamed_console_current_y=0;
int debug_unnamed_console_refresh=0;
int debug_unnamed_console_new_messages=0;

z80_bit debug_unnamed_console_enabled={1};



void debug_unnamed_console_end(void)
{
    //printf("on debug_unnamed_console_end\n");
    if (debug_unnamed_console_memory_pointer!=NULL) {
        //printf("unalloc debug_unnamed_console_memory_pointer\n");
        free (debug_unnamed_console_memory_pointer);
        debug_unnamed_console_memory_pointer=NULL;
    }
}

//Ancho de la ventana, que se va actualizando cuando el usuario la redimensiona
int ancho_ventana_unnamed_console=DEBUG_UNNAMED_CONSOLE_VISIBLE_INITIAL_WIDTH-2;

void debug_unnamed_console_scroll(void)
{

    int x,y;

    for (y=0;y<DEBUG_UNNAMED_CONSOLE_HEIGHT-1;y++) {
        for (x=0;x<DEBUG_UNNAMED_CONSOLE_LIMIT_WIDTH;x++) {
            int offset_linea_debajo=(y+1)*DEBUG_UNNAMED_CONSOLE_LIMIT_WIDTH+x;
            char c=debug_unnamed_console_memory_pointer[offset_linea_debajo];

            int offset_linea_actual=y*DEBUG_UNNAMED_CONSOLE_LIMIT_WIDTH+x;
            debug_unnamed_console_memory_pointer[offset_linea_actual]=c;
        }
    }

    //Y meter ultima linea con espacios
    for (x=0;x<DEBUG_UNNAMED_CONSOLE_LIMIT_WIDTH;x++) {
        int offset_linea_actual=y*DEBUG_UNNAMED_CONSOLE_LIMIT_WIDTH+x;
        debug_unnamed_console_memory_pointer[offset_linea_actual]=' ';
    }
}

void debug_unnamed_console_new_line(void)
{

    debug_unnamed_console_current_x=0;

    if (debug_unnamed_console_current_y<DEBUG_UNNAMED_CONSOLE_HEIGHT-1) {
        debug_unnamed_console_current_y++;
    }
    else {
        debug_unnamed_console_scroll();
    }
}



//Escribir caracter en posicion cursor
void debug_unnamed_console_printchar(char c)
{

    //Si no esta inicializado
    if (debug_unnamed_console_memory_pointer==NULL) return;

    //decir que se ha modificado
    debug_unnamed_console_new_messages=1;
    //y que se tiene que refrescar
    debug_unnamed_console_refresh=1;

    if (c==10) {
        //siguiente linea
        debug_unnamed_console_new_line();
        return;
    }

    //no hacemos este filtro, asi podemos meter caracteres utf, etc
    //if (c<32 || c>126) c='?';

    int offset=(debug_unnamed_console_current_y*DEBUG_UNNAMED_CONSOLE_LIMIT_WIDTH)+debug_unnamed_console_current_x;

    debug_unnamed_console_memory_pointer[offset]=c;

    debug_unnamed_console_current_x++;

    if (debug_unnamed_console_current_x>=ancho_ventana_unnamed_console) {
        debug_unnamed_console_new_line();
    }

}


void debug_unnamed_console_print(char *s)
{
    while (*s) {
        debug_unnamed_console_printchar(*s);
        s++;
    }

    //Y salto de linea
    debug_unnamed_console_printchar('\n');
}

void debug_unnamed_console_init(void)
{

    if (debug_unnamed_console_enabled.v==0) return;

    //printf("on debug_unnamed_console_init\n");

    int total_mem=DEBUG_UNNAMED_CONSOLE_LIMIT_WIDTH*DEBUG_UNNAMED_CONSOLE_HEIGHT;

    debug_unnamed_console_memory_pointer=malloc(total_mem);

    if (debug_unnamed_console_memory_pointer==NULL) cpu_panic("Can not allocate memory for unnamed console");

    //Inicializar coordenadas
    debug_unnamed_console_current_x=debug_unnamed_console_current_y=0;

    //Escribir espacios
    int i;
    for (i=0;i<total_mem;i++) debug_unnamed_console_memory_pointer[i]=' ';


    //Metemos texto ya inicial llenando la ventana asi los primeros mensajes estaran abajo
    //debug_unnamed_console_print("Scroll to the bottom to read first messages");

    //hacemos salto de linea de ventana asi los primeros mensajes estaran abajo
    for (i=0;i<DEBUG_UNNAMED_CONSOLE_HEIGHT;i++) {
        //debug_unnamed_console_print("");
        debug_unnamed_console_new_line();
    }

}

char *debug_mask_beyond_should_not_happen="BEYONDLIMIT";

debug_masks_class debug_masks_class_list[]={
    {"DSK",VERBOSE_CLASS_DSK},
    {"PD765",VERBOSE_CLASS_PD765},
    {"PCW",VERBOSE_CLASS_PCW},
    {"ZENG_ONLINE",VERBOSE_CLASS_ZENG_ONLINE},
    {"ZENG_ONLINE_CLIENT",VERBOSE_CLASS_ZENG_ONLINE_CLIENT},
    {"INTERFACE1",VERBOSE_CLASS_IF1},
    {"MICRODRIVE",VERBOSE_CLASS_MDR},
    {"ZXVISION_EVENTS",VERBOSE_CLASS_ZXVISION_EVENTS},
    {"ANYTHINGELSE",VERBOSE_CLASS_ANYTHINGELSE},
    {"",0}  //Siempre este al final
};

int debug_get_total_class_masks(void)
{
    int i;

    //limite 10000 al que nunca se deberia llegar
    for (i=0;i<10000 && debug_masks_class_list[i].name[0]!=0;i++);

    return i;
}

char *debug_get_class_mask_name(int i)
{
    int total=debug_get_total_class_masks();

    if (i>=total) {
        debug_printf(VERBOSE_ERR,"Querying mask name beyond masks limit");
        return debug_mask_beyond_should_not_happen;
    }

    return debug_masks_class_list[i].name;
}

int debug_get_class_mask_value(int i)
{
    int total=debug_get_total_class_masks();

    if (i>=total) {
        debug_printf(VERBOSE_ERR,"Querying mask value beyond masks limit");
        return 0;
    }

    return debug_masks_class_list[i].value;
}

//Parametros de inclusion/exclusion de clases de mensajes (dsk, pd765, etc)
//Por defecto modo excluir
int debug_mascara_modo_exclude_include=VERBOSE_MASK_CLASS_TYPE_EXCLUDE;

//Por defecto no excluimos nada
int debug_mascara_clase_exclude=0;

//Por defecto incluimos todo
int debug_mascara_clase_include=0x7FFFFF00; //31 bits todos marcados, excepto ultimos 8 bits que ahi no se mete valor de mascara y bit de signo



int debug_printf_check_exclude_include(unsigned int clase_mensaje)
{
    if (debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_EXCLUDE) {
        //Excluimos tipos mensaje
        clase_mensaje &=debug_mascara_clase_exclude;

        if (clase_mensaje) {
            //queda el bit activo, por tanto, se trata de ese mensaje que queremos excluir
            return 0; //lo excluimos
        }
        else {
            return 1; //lo incluimos
        }
    }
    else {
        //Incluimos tipos mensaje
        clase_mensaje &=debug_mascara_clase_include;

        if (clase_mensaje) {
            //queda el bit activo, por tanto, se trata de ese mensaje que queremos incluir
            return 1; //lo incluimos
        }
        else {
            return 0; //lo excluimos
        }
    }
}

void debug_printf (int debuglevel, const char * format , ...)
{
	//Adquirir lock
	while(z_atomic_test_and_set(&debug_printf_semaforo)) {
		//printf("Esperando a liberar lock en debug_printf\n");
	}



    int clase_mensaje;

    if (debuglevel<256){
        clase_mensaje=VERBOSE_CLASS_ANYTHINGELSE;
    }
    else {
        clase_mensaje=debuglevel & 0xFFFFFF00;
    }

    //Y nuestro level realmente son los 8 bits inferiores
    debuglevel &=0xFF;

    //Validar exclude/include y si hay que mostrar ese mensaje entonces

    //Clase de mensaje le llega dentro de debuglevel, si tiene mascara de bits a partir de valores 256
    int mostrar_mensaje;
    mostrar_mensaje=debug_printf_check_exclude_include(clase_mensaje);

    //Mensajes con VERBOSE_ERR, siempre se ven, independientemente de cual sea la class
    if (debuglevel==VERBOSE_ERR) mostrar_mensaje=1;

  	int copia_verbose_level;

  	copia_verbose_level=verbose_level;

    //VERBOSE_ONLY_DEBUG_CONSOLE_WINDOW siempre muestra el mensaje y no indica en el texto la prioridad (Error, Warning, etc del mensaje)

  	if ((mostrar_mensaje) && (debuglevel<=copia_verbose_level || debuglevel==VERBOSE_ONLY_DEBUG_CONSOLE_WINDOW)) {
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

        if (debuglevel==VERBOSE_ONLY_DEBUG_CONSOLE_WINDOW) {
            sprintf (buffer_final,"%s",buffer_inicial);
        }

    	else sprintf (buffer_final,"%s%s",verbose_message,buffer_inicial);

        //lanzarlo a consola a traves del driver de video. Siempre que verbose no sea only_debug_console_window
        if (debuglevel!=VERBOSE_ONLY_DEBUG_CONSOLE_WINDOW) {
    	    if (scr_messages_debug!=NULL) scr_messages_debug (buffer_final);
    	    else printf ("%s\n",buffer_final);
        }

		//Si tambien queremos mostrar log en consola,
		//esto es un caso un tanto especial pues la mayoria de drivers ya muestra mensajes en consola,
		//excepto curses, caca y aa lib, pues muestran 1 solo mensaje dentro de la interfaz del emulador
		//En esos casos puede ser necesario que el mensaje salga tal cual en consola, con scroll, aunque se desplace toda la interfaz
		//pero ayudara a que se vean los mensajes
		if (debug_always_show_messages_in_console.v) printf ("%s\n",buffer_final);


        //Y mostrarlo tambien en la unnamed_console
        debug_unnamed_console_print(buffer_final);

    	//Hacer aparecer menu, siempre que el driver no sea null ni.. porque no inicializado tambien? no inicializado
    	if (debuglevel==VERBOSE_ERR) {

			//en el caso de simpletext y null, no aparecera ventana igualmente, pero el error ya se vera por consola
			//en stdout si que "dibuja" la ventana por consola, para que se envie a speech
        	if (
				//!strcmp(scr_new_driver_name,"stdout") ||
        		!strcmp(scr_new_driver_name,"simpletext") ||
        		!strcmp(scr_new_driver_name,"null")
			)
			{
				//nada
			}

        	else {
	        	sprintf (pending_error_message,"%s",buffer_inicial);
    	    	if_pending_error_message=1;
        		menu_fire_event_open_menu();
			}
    	}

	}


	//Liberar lock
	z_atomic_reset(&debug_printf_semaforo);

}


//igual que debug_printf pero mostrando nombre archivo fuente y linea
//util para debug con modo debug o paranoid. mensajes de info o warn no tienen sentido mostrar archivo fuente
//Usar con, ejemplo:
//debug_printf_source (VERBOSE_DEBUG, __FILE__, __LINE__, __FUNCTION__, "Probando mensaje");
//o usando un macro que he definido:
//debug_printf_source (VERBOSE_DEBUG_SOURCE, "Probando mensaje");
void debug_printf_source (int debuglevel, char *archivo, int linea, const char *funcion, const char * format , ...)
{
  int copia_verbose_level;

  copia_verbose_level=verbose_level;

  if (debuglevel<=copia_verbose_level) {
        //tamaño del buffer bastante mas grande que el valor constante definido
    char buffer_inicial[DEBUG_MAX_MESSAGE_LENGTH*2+64];
    va_list args;
    va_start (args, format);
    vsprintf (buffer_inicial,format, args);
    va_end (args);
    debug_printf (debuglevel,"%s:%d (%s) %s",archivo,linea,funcion,buffer_inicial);
  }


}


int debug_nested_id_poke_byte;
int debug_nested_id_poke_byte_no_time;
int debug_nested_id_peek_byte;
int debug_nested_id_peek_byte_no_time;




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

	anterior_debug_mmu_mra=debug_mmu_mra;
	debug_mmu_mra=dir;

	//valor=peek_byte_no_time_no_debug(dir);
	valor=debug_nested_peek_byte_no_time_call_previous(debug_nested_id_peek_byte_no_time,dir);

    anterior_debug_mmu_mrv=debug_mmu_mrv;
	debug_mmu_mrv=valor; //Memory Read Value (valor leido en peek)



	return valor;
}


z80_byte peek_byte_debug (z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor;

	anterior_debug_mmu_mra=debug_mmu_mra;
	debug_mmu_mra=dir;

        //valor=peek_byte_no_debug(dir);
	valor=debug_nested_peek_byte_call_previous(debug_nested_id_peek_byte,dir);

    anterior_debug_mmu_mrv=debug_mmu_mrv;
	debug_mmu_mrv=valor; //Memory Read Value (valor leido en peek)


	//cpu_core_loop_debug_check_breakpoints();


	return valor;

}


z80_byte poke_byte_no_time_debug(z80_int dir,z80_byte value)
{
    anterior_debug_mmu_mwv=debug_mmu_mwv;
	debug_mmu_mwv=value;
	anterior_debug_mmu_mwa=debug_mmu_mwa;
	debug_mmu_mwa=dir;

	//poke_byte_no_time_no_debug(dir,value);
	debug_nested_poke_byte_no_time_call_previous(debug_nested_id_poke_byte_no_time,dir,value);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}

z80_byte poke_byte_debug(z80_int dir,z80_byte value)
{
    anterior_debug_mmu_mwv=debug_mmu_mwv;
	debug_mmu_mwv=value;
	anterior_debug_mmu_mwa=debug_mmu_mwa;
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


void do_breakpoint_exception(char *message)
{
	if (strlen(message)>MAX_MESSAGE_CATCH_BREAKPOINT-1) {
		cpu_panic("do_breakpoint_exception: strlen>MAX_MESSAGE_CATCH_BREAKPOINT");
	}

	sprintf(catch_breakpoint_message,"%s",message);
	debug_printf (VERBOSE_INFO,"Catch breakpoint: %s",message);
}


//Mostrar mensaje que ha hecho saltar el breakpoint y ejecutar accion (por defecto abrir menu)
void cpu_core_loop_debug_breakpoint(char *message)
{
	//menu_abierto=1;
	do_breakpoint_exception(message);

    if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
        menu_breakpoint_exception.v=1;

        //Si el breakpoint no ha saltado mientras estamos en cpu step de ZRCP
        if (menu_event_remote_protocol_enterstep.v==0) {
            zxvision_open_menu_with_window("debugcpu");
        }

    }


    else {
        //Gestionar acciones. Se gestionan desde aqui mismo y ya no escalan a menu

        //Gestion acciones
        //printf("Handling action %s\n",debug_breakpoints_actions_array[catch_breakpoint_index]);
        debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
    }


}










#define BREAKPOINT_CONDITION_OP_AND 0
#define BREAKPOINT_CONDITION_OP_OR 1
#define BREAKPOINT_CONDITION_OP_XOR 2

#define BREAKPOINT_MAX_OPERADORES 3

char *breakpoint_cond_operadores[BREAKPOINT_MAX_OPERADORES]={
	" and ", " or ", " xor "
};








/*
Sobre el parser optimizado y otras optimizaciones. Usos de cpu antes y ahora:

-hasta ayer.
--noconfigfile --set-breakpoint 1 bc=4444 ; 52 %  cpu

-con nuevo parser que permite meter registros y valores en ambos lados del operador de comparación
--noconfigfile --set-breakpoint 1 bc=4444 ; 70 % cpu

-con optimizaron si valor empieza por dígito:
54%

-con optimizacion metiendo comparación de “pc” arriba del todo y condición:
51% cpu. Con codigo de ayer 51%


-si valor no empieza por dígito, por ejemplo PC=CCCCH. Uso cpu 70%



——————

Optimizando expresiones PC=XXXX, MRA=XXXX, MWA=XXXX, optimizado basado en lo comentado con Thomas Busse

**1 breakpoint

--noconfigfile --set-breakpoint 1 pc=4444
-con optimizado
 43% cpu

-con parser anterior
55%cpu


** Con 10 breakpoints.
./zesarux --noconfigfile --set-breakpoint 1 pc=40000 --set-breakpoint 2 pc=40001 --set-breakpoint 3 pc=40002 --set-breakpoint 4 pc=40003 --set-breakpoint 5 pc=40005 --set-breakpoint 6 pc=40006 --set-breakpoint 7 pc=40007 --set-breakpoint 8 pc=40008 --set-breakpoint 9 pc=40009 --set-breakpoint 10 pc=40010

-con optimizado  45 %

-con parser anterior   85% cpu. (Desactivando auto frameskip. con autoframeskip, hace 75% cpu, 2 FPS


*/

//Parsea un breakpoint optimizado, basado en codigo de Thomas Busse
int debug_breakpoint_condition_optimized(int indice)
{

	int tipo_optimizacion;

    tipo_optimizacion=optimized_breakpoint_array[indice].operator;

	unsigned int valor;
	unsigned int valor_variable;

	valor=optimized_breakpoint_array[indice].valor;

	//Segun el tipo
	switch (tipo_optimizacion) {
		case OPTIMIZED_BRK_TYPE_PC:
			valor_variable=reg_pc;
		break;

		case OPTIMIZED_BRK_TYPE_MRA:
			valor_variable=debug_mmu_mra;
		break;

		case OPTIMIZED_BRK_TYPE_MWA:
			valor_variable=debug_mmu_mwa;
		break;

		default:
			return 0;
		break;
	}

	if (valor_variable==valor) {
		debug_printf (VERBOSE_DEBUG,"Fired optimized breakpoint. Optimizer type: %d value: %04XH",tipo_optimizacion,valor);
		return 1;
	}

	//printf ("NOT return variable is ok from optimizer tipo: %d valor: %d\n",tipo_optimizacion,valor);

	return 0;
}

void debug_set_mem_breakpoint(z80_int dir,z80_byte brkp_type)
{
	mem_breakpoint_array[dir]=brkp_type;
}



//Ver si salta breakpoint y teniendo en cuenta setting de saltar siempre o con cambio
int cpu_core_loop_debug_check_mem_brkp_aux(unsigned int dir, z80_byte tipo_mascara, unsigned int anterior_dir)
{

	//dir no deberia estar fuera de rango 0...65535. Pero por si acaso...
	if (dir<0 || dir>65535) return 0;

	if (mem_breakpoint_array[dir] & tipo_mascara) {
		//Coincide condicion

		int saltar_breakpoint=0;

                //Setting de saltar siempre
                if (debug_breakpoints_cond_behaviour.v==0) saltar_breakpoint=1;

                else {
                        //Solo saltar con cambio
                        if (dir != anterior_dir) saltar_breakpoint=1;
                }

		return saltar_breakpoint;
	}
	else return 0;
}

void cpu_core_loop_debug_check_mem_breakpoints(void)
{


//	mem_breakpoint_array
//Ver si coincide mra o mwa
/*
z80_int debug_mmu_mra; //Memory Read Addres (direccion usada peek)
z80_int debug_mmu_mwa; //Memory Write Address (direccion usada en poke)

//Anteriores valores para mra y mwa. De momento usado en los nuevos memory-breakpoints
//Si es -1, no hay valor anterior
int anterior_debug_mmu_mra=-1;
int anterior_debug_mmu_mwa=-1;
*/

	//Probar mra
	if (cpu_core_loop_debug_check_mem_brkp_aux(debug_mmu_mra,1,anterior_debug_mmu_mra)) {
		//Hacer saltar breakpoint MRA
		//printf ("Saltado breakpoint MRA en %04XH PC=%04XH\n",debug_mmu_mra,reg_pc);
		catch_breakpoint_index=-1;


		char buffer_mensaje[100];
		sprintf(buffer_mensaje,"Memory Breakpoint Read Address: %04XH",debug_mmu_mra);
                cpu_core_loop_debug_breakpoint(buffer_mensaje);

		//Cambiar esto tambien aqui porque si no escribiera en los siguientes opcodes, no llamaria a peek_debug y por tanto no
		//cambiaria esto. Aunque es absurdo porque al leer opcodes siempre esta cambiando MRA. por tanto lo comento, solo
		//tiene sentido para mwa
		//anterior_debug_mmu_mra=debug_mmu_mra;

	}

	//Probar mwa
    if (cpu_core_loop_debug_check_mem_brkp_aux(debug_mmu_mwa,2,anterior_debug_mmu_mwa)) {
                //Hacer saltar breakpoint MWA
                //printf ("Saltado breakpoint MWA en %04XH PC=%04XH\n",debug_mmu_mwa,reg_pc);
		catch_breakpoint_index=-1;

		char buffer_mensaje[100];
		sprintf(buffer_mensaje,"Memory Breakpoint Write Address: %04XH",debug_mmu_mwa);
                cpu_core_loop_debug_breakpoint(buffer_mensaje);

		//Cambiar esto tambien aqui porque si no escribiera en los siguientes opcodes, no llamaria a poke_debug y por tanto no
		//cambiaria esto
		anterior_debug_mmu_mwa=debug_mmu_mwa;

        }



}

//Establece variable al ultimo activo+1
void debug_set_last_active_breakpoint(void)
{
	int i;
	for (i=MAX_BREAKPOINTS_CONDITIONS-1;i>=0;i--) {
		if (debug_breakpoints_conditions_enabled[i]) {
			//Esta activado, pero tiene contenido?



			if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {
				last_active_breakpoint=i+1;
				debug_printf (VERBOSE_DEBUG,"Last active breakpoint +1: %d",last_active_breakpoint);
				return;
			}




		}

	}

	last_active_breakpoint=0; //no hay breakpoints activos
	debug_printf (VERBOSE_DEBUG,"Last active breakpoint +1: %d",last_active_breakpoint);
}


//conmutar enabled/disabled
void debug_breakpoints_conditions_toggle(int indice)
{
	//printf ("Ejecutada funcion para espacio, condicion: %d\n",valor_opcion);

	debug_breakpoints_conditions_enabled[indice] ^=1;

	//si queda activo, decir que no ha saltado aun ese breakpoint
	if (debug_breakpoints_conditions_enabled[indice]) {
		debug_breakpoints_conditions_saltado[indice]=0;
	}

	debug_set_last_active_breakpoint();
}


void debug_breakpoints_conditions_enable(int indice)
{
  debug_breakpoints_conditions_enabled[indice]=1;
  debug_set_last_active_breakpoint();
}

void debug_breakpoints_conditions_disable(int indice)
{
  debug_breakpoints_conditions_enabled[indice]=0;
  debug_set_last_active_breakpoint();
}




//Comprobar condiciones. Usando nuevo breakpoint parser.  Solo lo hacemos en core_loop
void cpu_core_loop_debug_check_breakpoints(void)
{
	//Condicion de debug
	if (debug_breakpoints_enabled.v) {

		//Comprobar los mem-breakpoints
		cpu_core_loop_debug_check_mem_breakpoints();

		int i;

		//Breakpoint de condicion
		//for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
		for (i=0;i<last_active_breakpoint;i++) {
			//Si ese breakpoint esta activo
			if (debug_breakpoints_conditions_enabled[i]) {
				if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {

					int se_cumple_breakpoint;
					//printf ("Checking breakpoint %d\n",i);
					//Si esta optimizado

					if (optimized_breakpoint_array[i].optimized) {
						//printf ("Parsing optimized breakpoint\n");
						se_cumple_breakpoint=debug_breakpoint_condition_optimized(i);
					}
					else {
						//se_cumple_breakpoint=debug_breakpoint_condition_loop(&debug_breakpoints_conditions_array[i][0],0);
						int error_code;
						se_cumple_breakpoint=exp_par_evaluate_token(debug_breakpoints_conditions_array_tokens[i],MAX_PARSER_TOKENS_NUM,&error_code);
						//Nota: aqui no comprobamos error_code, gestionamos el valor de retorno tal cual vuelve, haya habido o no error
					}

					if ( se_cumple_breakpoint ) {
						//Si condicion pasa de false a true o bien el comportamiento por defecto es saltar siempre
						if (debug_breakpoints_cond_behaviour.v==0 || debug_breakpoints_conditions_saltado[i]==0) {
                            //printf("breakpoint saltado\n");
                            //printf("PC=%04XH\n",reg_pc);

							debug_breakpoints_conditions_saltado[i]=1;

							char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
							exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);

	        	        	char buffer_mensaje[MAX_BREAKPOINT_CONDITION_LENGTH+64];
        	    	    	sprintf(buffer_mensaje,"%s",buffer_temp);

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

}





//int debug_watches_mostrado_frame=0;
//char debug_watches_texto_destino[1024];

//Misma limitacion de longitud que un breakpoint.
//Si cadena vacia, no hay breakpoint
//char debug_watches_text_to_watch[MAX_BREAKPOINT_CONDITION_LENGTH]="";

//z80_byte debug_watches_y_position=0;

//void cpu_core_loop_debug(void)

int debug_nested_id_core;
z80_byte cpu_core_loop_debug(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

	debug_fired_out=0;
	//Si se ejecuta un out en el core (justo despues que esto) se activara dicha variable

	debug_fired_in=0;
	//Si se ejecuta un in en el core (justo despues que esto) se activara dicha variable

	debug_fired_interrupt=0;
	//Si se lanza una interrupcion en el core (justo despues que esto) se activara dicha variable


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
				debug_printf(VERBOSE_WARN,"Setting REDUCED Spectrum CPU core, the following features will NOT be available or will NOT be properly emulated: Debug t-states, Char detection, PLUS3DOS traps, Save to tape, Divide, Divmmc, RZX, Raster interrupts, TBBlue Copper, Audio DAC, Video out to file");
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

                case CPU_CORE_PCW:
                        debug_printf(VERBOSE_INFO,"Setting PCW core");
                        cpu_core_loop=cpu_core_loop_pcw;
                        cpu_core_loop_name="PCW";
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

    case CPU_CORE_MSX:
      debug_printf(VERBOSE_INFO,"Setting MSX CPU core");
      cpu_core_loop=cpu_core_loop_msx;
      cpu_core_loop_name="MSX";
    break;

    case CPU_CORE_COLECO:
      debug_printf(VERBOSE_INFO,"Setting COLECO CPU core");
      cpu_core_loop=cpu_core_loop_coleco;
      cpu_core_loop_name="COLECO";
    break;

    case CPU_CORE_SG1000:
      debug_printf(VERBOSE_INFO,"Setting SG1000 CPU core");
      cpu_core_loop=cpu_core_loop_sg1000;
      cpu_core_loop_name="SG1000";
    break;

    case CPU_CORE_SMS:
      debug_printf(VERBOSE_INFO,"Setting Master System CPU core");
      cpu_core_loop=cpu_core_loop_sms;
      cpu_core_loop_name="SMS";
    break;

    case CPU_CORE_SVI:
      debug_printf(VERBOSE_INFO,"Setting SVI CPU core");
      cpu_core_loop=cpu_core_loop_svi;
      cpu_core_loop_name="SVI";
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

z80_bit cpu_history_enabled={0};
int cpu_history_nested_id_core;

z80_bit cpu_code_coverage_enabled={0};
int cpu_code_coverage_nested_id_core;


z80_bit extended_stack_enabled={0};
int extended_stack_nested_id_core;

//Array para el code coverage. De momento solo tiene el contenido:
//0: no ha ejecutado la cpu esa dirección
//diferente de 0: ha ejecutado la cpu esa dirección
//en el futuro se pueden usar mas bits de cada elemento
z80_byte cpu_code_coverage_array[65536];

FILE *ptr_transaction_log=NULL;

char transaction_log_filename[PATH_MAX];


//Tamanyo del archivo de transaction log. Para leer desde aqui en vez de usar ftell para saber que tamanyo tiene, que es mas rapido
long transaction_log_tamanyo_escrito=0;

//Lineas del archivo de transaction log
long transaction_log_tamanyo_lineas=0;

char transaction_log_line_to_store[2048];


//campos a guardar en el transaction log
z80_bit cpu_transaction_log_store_datetime={0};
z80_bit cpu_transaction_log_store_address={1};
z80_bit cpu_transaction_log_store_tstates={0};
z80_bit cpu_transaction_log_store_opcode={1};
z80_bit cpu_transaction_log_store_registers={0};

//Si se habilita rotacion del transaction log
z80_bit cpu_transaction_log_rotate_enabled={0};
//Numero de archivos rotados
int cpu_transaction_log_rotated_files=10;
//Tamanyo maximo antes de rotar archivo, en MB. Si es 0, no rotar
int cpu_transaction_log_rotate_size=100;

//Lineas maximas antes de rotar archivo. Si es 0, no rotar
int cpu_transaction_log_rotate_lines=1000000;


int transaction_log_nested_id_core;




//Para tener una memory zone que apunte a un archivo
char memory_zone_by_file_name[PATH_MAX];
z80_byte *memory_zone_by_file_pointer;
int memory_zone_by_file_size=0;

//Si ultima instruccion era HALT. Para ignorar hasta lo que no sea HALT. Contar al menos 1
//Usados en cpu transaction log y cpu-history
int cpu_trans_log_last_was_halt=0;

//Si ultima instruccion era LDIR o LDDR. Para ignorar hasta lo que no sea LDIR o LDDR. Contar al menos 1
int cpu_trans_log_last_was_ldxr=0;

//Para halt
z80_bit cpu_trans_log_ignore_repeated_halt={0};
//Para ldir,lddr
z80_bit cpu_trans_log_ignore_repeated_ldxr={0};



//TODO: esto podria usar funcion util_rotate_files
void transaction_log_rotate_files(int archivos)
{
	//Primero cerrar archivo
	transaction_log_close_file();

	//Borrar el ultimo
	char buffer_last_file[PATH_MAX];

	sprintf(buffer_last_file,"%s.%d",transaction_log_filename,archivos);

	debug_printf (VERBOSE_DEBUG,"Erasing oldest transaction log file %s",buffer_last_file);

	util_delete(buffer_last_file);

	//Y renombrar, el penultimo al ultimo, el antepenultimo al penultimo, etc
	//con 10 archivos:
	//ren file.9 file.10
	//ren file.8 file.9
	//ren file.7 file.8
	//...
	//ren file.1 file.2
	//ren file file.1 esto a mano
	int i;

	for (i=archivos-1;i>=0;i--) {
		char buffer_file_orig[PATH_MAX];
		char buffer_file_dest[PATH_MAX];

		//Caso especial ultimo (seria el .0)
		if (i==0) {
			strcpy(buffer_file_orig,transaction_log_filename);
		}
		else {
			sprintf(buffer_file_orig,"%s.%d",transaction_log_filename,i);
		}

		sprintf(buffer_file_dest,"%s.%d",transaction_log_filename,i+1);

		debug_printf (VERBOSE_DEBUG,"Renaming transaction log file %s -> %s",buffer_file_orig,buffer_file_dest);
		rename(buffer_file_orig,buffer_file_dest);
	}


	//Y finalmente truncar
	transaction_log_truncate();

	//Y tenemos que abrirlo a mano
	transaction_log_open_file();
}

void transaction_log_rotate_by_size(void)
{


	if (cpu_transaction_log_rotate_enabled.v==0) return;

	if (cpu_transaction_log_rotate_size==0) return; //no rotar si vale 0

	//Obtener tamanyo archivo a ver si hay que rotar o no
	//nota: dado que el flush en mac por ejemplo se hace muy de vez en cuando, ver el tamanyo del archivo
	//tal cual con la estructura en memoria, no mirando el archivo en disco

	//ftell es muy lento
	//long tamanyo=ftell(ptr_transaction_log);


	long tamanyo=transaction_log_tamanyo_escrito;

	//printf ("posicion: (tamanyo) %ld\n",tamanyo);

	//Si hay que rotar


	long tamanyo_a_rotar=cpu_transaction_log_rotate_size;

	//Pasar tamanyo a bytes
	tamanyo_a_rotar *=1024;
	tamanyo_a_rotar *=1024;

	if (tamanyo>=tamanyo_a_rotar) {
		debug_printf (VERBOSE_DEBUG,"Rotating transaction log. File size %ld exceeds maximum %ld",tamanyo,tamanyo_a_rotar);
		transaction_log_rotate_files(cpu_transaction_log_rotated_files);
	}
}

void transaction_log_rotate_by_lines(void)
{


	if (cpu_transaction_log_rotate_enabled.v==0) return;

	if (cpu_transaction_log_rotate_lines==0) return; //no rotar si vale 0


	long tamanyo=transaction_log_tamanyo_lineas;

	//printf ("posicion: (tamanyo) %ld\n",tamanyo);

	//Si hay que rotar


	long tamanyo_a_rotar=cpu_transaction_log_rotate_lines;

	if (tamanyo>=tamanyo_a_rotar) {
		debug_printf (VERBOSE_DEBUG,"Rotating transaction log. File lines %ld exceeds maximum %ld",tamanyo,tamanyo_a_rotar);
		transaction_log_rotate_files(cpu_transaction_log_rotated_files);
	}
}



int transaction_log_set_rotate_number(int numero)
{
	if (numero<1 || numero>999) {
        return 1;
	}


	cpu_transaction_log_rotated_files=numero;
	return 0;
}


int transaction_log_set_rotate_size(int numero)
{
	if (numero<0 || numero>9999) {
        return 1;
	}


	cpu_transaction_log_rotate_size=numero;
	return 0;
}

int transaction_log_set_rotate_lines(int numero)
{
	if (numero<0 || numero>2147483647) { //maximo 2^31-1
        return 1;
	}


	cpu_transaction_log_rotate_lines=numero;
	return 0;
}



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

			/
			*/

			int longitud=debug_get_timestamp(&transaction_log_line_to_store[index]);

			index +=longitud;

			//Agregar espacio
			transaction_log_line_to_store[index++]=' ';
			transaction_log_line_to_store[index++]=0;

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


		//Si es halt lo ultimo
		if (cpu_trans_log_ignore_repeated_halt.v) {
			if (CPU_IS_Z80) {
				z80_byte opcode=peek_byte_no_time(registro_pc);
				if (opcode==118) {
					if (cpu_trans_log_last_was_halt<2) cpu_trans_log_last_was_halt++;
					//printf ("halts %d\n",cpu_trans_log_last_was_halt);

				}
				else {
					cpu_trans_log_last_was_halt=0;
				}

			}
			else {
				cpu_trans_log_last_was_halt=0;
			}
		}

		if (cpu_transaction_log_store_registers.v) {
			print_registers(&transaction_log_line_to_store[index]);
                	int len=strlen(&transaction_log_line_to_store[index]);
	                index +=len;
        	        transaction_log_line_to_store[index++]=' ';
	        }

		//salto de linea
		transaction_log_line_to_store[index++]=10;

		//Si esta NULL es que no se ha abierto correctamente, y aqui no deberiamos llegar nunca
		if (ptr_transaction_log!=NULL) {

			//Si era halt los dos ultimos y hay que ignorarlo
			if (cpu_trans_log_ignore_repeated_halt.v && cpu_trans_log_last_was_halt>1) {
				//no hacer log
			}
			else {

				fwrite(transaction_log_line_to_store,1,index,ptr_transaction_log);

				transaction_log_tamanyo_escrito +=index;

				transaction_log_tamanyo_lineas++;
			}
		}




		//Rotar log si conviene por tamanyo
		transaction_log_rotate_by_size();

		//Rotar log si conviene por lineas
		transaction_log_rotate_by_lines();

	}

	//Llamar a core anterior
	debug_nested_core_call_previous(transaction_log_nested_id_core);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos

	return 0;
}

void transaction_log_close_file(void)
{
	if (ptr_transaction_log!=NULL) {
		fclose(ptr_transaction_log);
		ptr_transaction_log=NULL;
	}
}

//Retorna 1 si error
int transaction_log_open_file(void)
{

  transaction_log_tamanyo_escrito=0;
  transaction_log_tamanyo_lineas=0;

  //Si el archivo existia, inicializar tamanyo, no poner a 0

  if (si_existe_archivo(transaction_log_filename)) {
	 transaction_log_tamanyo_escrito=get_file_size(transaction_log_filename);


	 //tiempo antes
	//char tiempo[200];
	//debug_get_timestamp(tiempo);
	//printf ("tiempo antes leer archivo: %s\n",tiempo);

	transaction_log_tamanyo_lineas=get_file_lines(transaction_log_filename);

	//debug_get_timestamp(tiempo);
	//printf ("tiempo despues leer archivo: %s\n",tiempo);

	 //tiempo despues

  }

  debug_printf (VERBOSE_DEBUG,"Transaction log file size: %ld lines: %ld",transaction_log_tamanyo_escrito,transaction_log_tamanyo_lineas);

  ptr_transaction_log=fopen(transaction_log_filename,"ab");
  if (!ptr_transaction_log) {
 		debug_printf (VERBOSE_ERR,"Unable to open Transaction log");
		debug_nested_core_del(transaction_log_nested_id_core);
		return 1;
	}



	return 0;
}

void transaction_log_truncate(void)
{

 	if (ptr_transaction_log) {
        transaction_log_close_file();
        util_truncate_file(transaction_log_filename);
        transaction_log_open_file();
    }
    else {
        util_truncate_file(transaction_log_filename);
    }

}

//Truncar los logs rotados
void transaction_log_truncate_rotated(void)
{



	int archivos=cpu_transaction_log_rotated_files;
	int i;

	for (i=1;i<=archivos;i++) {

		char buffer_file_dest[PATH_MAX];

		sprintf(buffer_file_dest,"%s.%d",transaction_log_filename,i);

		debug_printf (VERBOSE_DEBUG,"Truncating rotated transaction log file %s",buffer_file_dest);
		util_truncate_file(buffer_file_dest);
	}


}

void set_cpu_core_transaction_log(void)
{
        debug_printf(VERBOSE_INFO,"Enabling Transaction Log");
	if (cpu_transaction_log_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}

	transaction_log_nested_id_core=debug_nested_core_add(cpu_core_loop_transaction_log,"Transaction Log Core");


	if (transaction_log_open_file()) return;



	cpu_transaction_log_enabled.v=1;

}

void reset_cpu_core_transaction_log(void)
{
  debug_printf(VERBOSE_INFO,"Disabling Transaction Log");
	if (cpu_transaction_log_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_core_del(transaction_log_nested_id_core);
	cpu_transaction_log_enabled.v=0;

	transaction_log_close_file();

}

void cpu_code_coverage_clear(void)
{
  int i;
  for (i=0;i<65536;i++) cpu_code_coverage_array[i]=0;
}

z80_byte cpu_core_loop_code_coverage(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{


	//hacer cosas antes...
	//printf ("running cpu code coverage addr: %04XH\n",reg_pc);

	int indice=reg_pc & 0xffff;

	cpu_code_coverage_array[indice]=1;

	//Llamar a core anterior
	debug_nested_core_call_previous(cpu_code_coverage_nested_id_core);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos

	return 0;
}



//punto de entrada alternativo que lo activa sin borrar datos
//util si se llama desde cambio velocidad cpu
void set_cpu_core_code_coverage_enable(void)
{
    debug_printf(VERBOSE_INFO,"Enabling Cpu code coverage");

	if (cpu_code_coverage_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}

	cpu_code_coverage_nested_id_core=debug_nested_core_add(cpu_core_loop_code_coverage,"CPU code coverage Core");



	cpu_code_coverage_enabled.v=1;



}

void set_cpu_core_code_coverage(void)
{

  set_cpu_core_code_coverage_enable();
  cpu_code_coverage_clear();

}


void reset_cpu_core_code_coverage(void)
{
  debug_printf(VERBOSE_INFO,"Disabling Cpu code coverage");
	if (cpu_code_coverage_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_core_del(cpu_code_coverage_nested_id_core);
	cpu_code_coverage_enabled.v=0;


}


// Codigo para extended stack




//Array con todo el extended stack
#define EXTENDED_STACK_SIZE 65536
struct s_extended_stack_item extended_stack_array_items[EXTENDED_STACK_SIZE];

//Retornar el tipo de valor de extended stack
char *extended_stack_get_string_type(z80_byte tipo)
{

	//Algunas comprobaciones por si acaso
	if (tipo>=TOTAL_PUSH_VALUE_TYPES) {
		//Si se sale de rango, devolver default
		return push_value_types_strings[0];
	}

	else return push_value_types_strings[tipo];

}

void extended_stack_clear(void)
{

	int i;

	for (i=0;i<EXTENDED_STACK_SIZE;i++) {
		extended_stack_array_items[i].valor=0;
		extended_stack_array_items[i].tipo=0;
	}

}


z80_byte push_valor_extended_stack(z80_int valor,z80_byte tipo)
{

	//printf ("Putting in stack value: %04XH type: %d (%s) SP=%04XH\n",valor,tipo,extended_stack_get_string_type(tipo),reg_sp);

	extended_stack_array_items[reg_sp-1].valor=value_16_to_8h(valor);
	extended_stack_array_items[reg_sp-1].tipo=tipo;

	extended_stack_array_items[reg_sp-2].valor=value_16_to_8l(valor);
	extended_stack_array_items[reg_sp-2].tipo=tipo;


	debug_nested_push_valor_call_previous(extended_stack_nested_id_core,valor,tipo);

	//Para que no se queje el compilador
	return 0;

}

void set_extended_stack(void)
{
    debug_printf(VERBOSE_INFO,"Enabling Extended stack");

	if (extended_stack_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}

	extended_stack_nested_id_core=debug_nested_push_valor_add(push_valor_extended_stack,"Extended stack");



	extended_stack_enabled.v=1;


}

void reset_extended_stack(void)
{
  debug_printf(VERBOSE_INFO,"Disabling Extended stack");
	if (extended_stack_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_push_valor_del(extended_stack_nested_id_core);
	extended_stack_enabled.v=0;


}

// Fin codigo para extended stack






//Dado un puntero z80_byte, con contenido de registros en binario, retorna valores registros
//Registros 16 bits guardados en little endian
//NOTA: este comando se mantiene solo por compatibilidad con Dezog, no ampliarlo ni modificarlo para no romper la conexión con dicho programa
void cpu_history_legacy_regs_bin_to_string(z80_byte *p,char *destino)
{

    //para memoria modificada
    char buffer_temp_memoria[100];
    //por defecto vacio
    buffer_temp_memoria[0]=0;


    z80_int direccion1=value_8_to_16(p[52],p[51]);
    z80_byte valor1=p[53];
    char buffer_addr_1[32];
    sprintf(buffer_addr_1,"(%04X)=%X",direccion1,valor1);

    z80_int direccion2=value_8_to_16(p[55],p[54]);
    z80_byte valor2=p[56];
    char buffer_addr_2[32];
    sprintf(buffer_addr_2,"(%04X)=%X",direccion2,valor2);


    int flags_direcciones=p[50] & 3;

    if (flags_direcciones>0) {
        sprintf(buffer_temp_memoria,"%s %s",
        (flags_direcciones>=1 ? buffer_addr_1 : ""),
        (flags_direcciones>=2 ? buffer_addr_2 : "")
        );
    }


	//Nota: funcion print_registers escribe antes BC que AF. Aqui ponemos AF antes, que es mas lógico
  sprintf (destino,"PC=%02x%02x SP=%02x%02x AF=%02x%02x BC=%02x%02x HL=%02x%02x DE=%02x%02x IX=%02x%02x IY=%02x%02x "
  				   "AF'=%02x%02x BC'=%02x%02x HL'=%02x%02x DE'=%02x%02x "
				   "I=%02x R=%02x IM%d IFF%c%c (PC)=%02x%02x%02x%02x (SP)=%02x%02x "
				   "MMU=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %s",
  p[1],p[0], 	//pc
  p[3],p[2], 	//sp
  p[5],p[4], 	//af
  p[7],p[6], 	//bc
  p[9],p[8], 	//hl
  p[11],p[10], 	//de
  p[13],p[12], 	//ix
  p[15],p[14], 	//iy
  p[17],p[16], 	//af'
  p[19],p[18], 	//bc'
  p[21],p[20], 	//hl'
  p[23],p[22], 	//de'
  p[24], 		//I
  p[25], 		//R
  p[26], 		//IM
  DEBUG_STRING_IFF12_PARAM(p[27]),  //IFF1,2
  //contenido (pc) 4 bytes
  p[28],p[29],p[30],p[31],
  //contenido (sp) 2 bytes
  p[33],p[32],
  //MMU. Las paginas de debug_paginas_memoria_mapeadas, son valores de 16 bits escritas en Little Endian
  p[35],p[34], p[37],p[36], p[39],p[38], p[41],p[40],
  p[43],p[42], p[45],p[44], p[47],p[46], p[49],p[48],
  buffer_temp_memoria
  );

  //50: flags
  //Bits 0-1: sobre direcciones modificadas: 0, ninguna, 1: una direccion, 2: dos direcciones
  //51-52: primera direccion modificada
  //53: valor antes de modificar primera direccion
  //54-55: segunda direccion modificada
  //56: valor antes de modificar segunda direccion

}

//Dado un puntero z80_byte, con contenido de registros en binario, retorna valores registros
//Registros 16 bits guardados en little endian
void cpu_history_extended_regs_bin_to_string(z80_byte *p,char *destino)
{

    //para memoria modificada
    char buffer_temp_memoria[100];
    //por defecto vacio
    buffer_temp_memoria[0]=0;


    z80_int direccion1=value_8_to_16(p[52],p[51]);
    z80_byte valor1=p[53];
    char buffer_addr_1[32];
    sprintf(buffer_addr_1,"(%04X)=%X",direccion1,valor1);

    z80_int direccion2=value_8_to_16(p[55],p[54]);
    z80_byte valor2=p[56];
    char buffer_addr_2[32];
    sprintf(buffer_addr_2,"(%04X)=%X",direccion2,valor2);


    int flags_direcciones=p[50] & 3;

    if (flags_direcciones>0) {
        sprintf(buffer_temp_memoria,"%s %s",
        (flags_direcciones>=1 ? buffer_addr_1 : ""),
        (flags_direcciones>=2 ? buffer_addr_2 : "")
        );
    }


	//Nota: funcion print_registers escribe antes BC que AF. Aqui ponemos AF antes, que es mas lógico
  sprintf (destino,"PC=%02x%02x SP=%02x%02x AF=%02x%02x BC=%02x%02x HL=%02x%02x DE=%02x%02x IX=%02x%02x IY=%02x%02x "
  				   "AF'=%02x%02x BC'=%02x%02x HL'=%02x%02x DE'=%02x%02x "
				   "I=%02x R=%02x IM%d IFF%c%c (PC)=%02x%02x%02x%02x (SP)=%02x%02x "
				   "MMU=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x 3FFD=%02x 1FFD=%02x %s",
  p[1],p[0], 	//pc
  p[3],p[2], 	//sp
  p[5],p[4], 	//af
  p[7],p[6], 	//bc
  p[9],p[8], 	//hl
  p[11],p[10], 	//de
  p[13],p[12], 	//ix
  p[15],p[14], 	//iy
  p[17],p[16], 	//af'
  p[19],p[18], 	//bc'
  p[21],p[20], 	//hl'
  p[23],p[22], 	//de'
  p[24], 		//I
  p[25], 		//R
  p[26], 		//IM
  DEBUG_STRING_IFF12_PARAM(p[27]),  //IFF1,2
  //contenido (pc) 4 bytes
  p[28],p[29],p[30],p[31],
  //contenido (sp) 2 bytes
  p[33],p[32],
  //MMU. Las paginas de debug_paginas_memoria_mapeadas, son valores de 16 bits escritas en Little Endian
  p[35],p[34], p[37],p[36], p[39],p[38], p[41],p[40],
  p[43],p[42], p[45],p[44], p[47],p[46], p[49],p[48],
  p[57],p[58],
  buffer_temp_memoria
  );

  //50: flags
  //Bits 0-1: sobre direcciones modificadas: 0, ninguna, 1: una direccion, 2: dos direcciones
  //51-52: primera direccion modificada
  //53: valor antes de modificar primera direccion
  //54-55: segunda direccion modificada
  //56: valor antes de modificar segunda direccion
  //57: puerto 32765
  //58: puerto 8189

}

//Dado un puntero z80_byte, con contenido de registros en binario, retorna valor registro PC
//Registros 16 bits guardados en little endian
void cpu_history_reg_pc_bin_to_string(z80_byte *p,char *destino)
{

//Nota: funcion print_registers escribe antes BC que AF. Aqui ponemos AF antes, que es mas lógico
  sprintf (destino,"%02x%02x",
  p[1],p[0] 	//pc
  );
}

//Funcion para leer byte preservando variable MRA
z80_byte peek_byte_no_time_no_change_mra(z80_int dir)
{
	unsigned int antes_debug_mmu_mra=debug_mmu_mra;

	z80_byte value=peek_byte_no_time(dir);

	debug_mmu_mra=antes_debug_mmu_mra;

	return value;

}

void cpu_history_regs_to_bin_aux_exsp(z80_byte *p)
{
    //EX (SP),HL  , EX(SP),IX/IY
    z80_byte value1,value2;

    z80_int puntero;

    puntero=reg_sp;
    value1=peek_byte_no_time_no_change_mra(puntero);
    value2=peek_byte_no_time_no_change_mra(puntero+1);
    p[50]=2;
    p[51]=value_16_to_8l(puntero);
    p[52]=value_16_to_8h(puntero);
    p[53]=value1;
    p[54]=value_16_to_8l(puntero+1);
    p[55]=value_16_to_8h(puntero+1);
    p[56]=value2;

    //printf("Storing on history %XH with value %02X%02XH coming from opcode type EX (SP),HL/IX/IY\n",puntero,value2,value1);

}

//Guarda en puntero z80_byte en con contenido de registros en binario
//Registros 16 bits guardados en little endian
void cpu_history_regs_to_bin(z80_byte *p)
{

	p[0]=value_16_to_8l(reg_pc);
	p[1]=value_16_to_8h(reg_pc);

	p[2]=value_16_to_8l(reg_sp);
	p[3]=value_16_to_8h(reg_sp);

	p[4]=Z80_FLAGS;
	p[5]=reg_a;

	p[6]=reg_c;
	p[7]=reg_b;

	p[8]=reg_l;
	p[9]=reg_h;

	p[10]=reg_e;
	p[11]=reg_d;

	p[12]=value_16_to_8l(reg_ix);
	p[13]=value_16_to_8h(reg_ix);

	p[14]=value_16_to_8l(reg_iy);
	p[15]=value_16_to_8h(reg_iy);

	p[16]=Z80_FLAGS_SHADOW;
	p[17]=reg_a_shadow;

	p[18]=reg_c_shadow;
	p[19]=reg_b_shadow;

	p[20]=reg_l_shadow;
	p[21]=reg_h_shadow;

	p[22]=reg_e_shadow;
	p[23]=reg_d_shadow;

	p[24]=reg_i;
  	p[25]=(reg_r&127)|(reg_r_bit7&128);
  	p[26]=im_mode;
	p[27]=iff1.v | (iff2.v<<1);

    p[28]=peek_byte_no_time_no_change_mra(reg_pc);
    p[29]=peek_byte_no_time_no_change_mra(reg_pc+1);
    p[30]=peek_byte_no_time_no_change_mra(reg_pc+2);
    p[31]=peek_byte_no_time_no_change_mra(reg_pc+3);

    p[32]=peek_byte_no_time_no_change_mra(reg_sp);
    p[33]=peek_byte_no_time_no_change_mra(reg_sp+1);

	//MMU. Desde p34
	int i;

	for (i=0;i<8;i++) {
		//Low byte
		p[34+i*2]=value_16_to_8l(debug_paginas_memoria_mapeadas[i]);
		//High byte
		p[34+i*2+1]=value_16_to_8h(debug_paginas_memoria_mapeadas[i]);
	}

    //57,58: puertos paginacion 32765, 8189
    p[57]=puerto_32765;
    p[58]=puerto_8189;

  //50: flags
  //Bits 0-1: sobre direcciones modificadas: 0, ninguna, 1: una direccion, 2: dos direcciones
  //51-52: primera direccion modificada
  //53: valor antes de modificar primera direccion
  //54-55: segunda direccion modificada
  //56: valor antes de modificar segunda direccion

    //por defecto , no modifica direcciones dicho opcode
    p[50]=0;

    z80_byte opcode=peek_byte_no_time_no_change_mra(reg_pc);

    z80_byte opcode1=peek_byte_no_time_no_change_mra(reg_pc+1);

    z80_byte opcode2;

    z80_byte value1,value2;

    z80_int puntero;


	z80_byte *pref203_registro;
	//z80_byte pref203_numerobit;

    z80_int *cual_registro_ixiy;

    z80_int desp16;
    z80_byte desp;

    //Esto se podria hacer con una tabla pero dado que solo lo utilizo aqui, lo hago con switch
    switch (opcode) {

        case 2: //LD (BC),A
            value1=peek_byte_no_time_no_change_mra(BC);
            p[50]=1;
            p[51]=reg_c;
            p[52]=reg_b;
            p[53]=value1;
            //printf("Storing on history %XH with value %XH coming from opcode %XH modifying (BC)\n",HL,value1,opcode);

        break;


        case 18: //LD (DE),A
            value1=peek_byte_no_time_no_change_mra(DE);
            p[50]=1;
            p[51]=reg_e;
            p[52]=reg_d;
            p[53]=value1;
            //printf("Storing on history %XH with value %XH coming from opcode %XH modifying (DE)\n",DE,value1,opcode);

        break;

        case 34: //LD (NN),HL
            puntero=value_8_to_16(peek_byte_no_time_no_change_mra(reg_pc+2),peek_byte_no_time_no_change_mra(reg_pc+1));
            value1=peek_byte_no_time_no_change_mra(puntero);
            value2=peek_byte_no_time_no_change_mra(puntero+1);
            p[50]=2;
            p[51]=value_16_to_8l(puntero);
            p[52]=value_16_to_8h(puntero);
            p[53]=value1;
            p[54]=value_16_to_8l(puntero+1);
            p[55]=value_16_to_8h(puntero+1);
            p[56]=value2;
            //printf("Storing on history %XH with value %02X%02XH coming from opcode %XH modifying 16 bits (NN)\n",puntero,value2,value1,opcode);
        break;

        case 50: //LD (NN),A
            puntero=value_8_to_16(peek_byte_no_time_no_change_mra(reg_pc+2),peek_byte_no_time_no_change_mra(reg_pc+1));
            value1=peek_byte_no_time_no_change_mra(puntero);
            p[50]=1;
            p[51]=value_16_to_8l(puntero);
            p[52]=value_16_to_8h(puntero);
            p[53]=value1;

            //printf("Storing on history %XH with value %02XH coming from opcode %XH modifying 8 bits (NN)\n",puntero,value1,opcode);
        break;


        case 52: //INC (HL)
        case 53: //DEC (HL)
        case 54: //LD (HL),N
        case 112: //LD (HL),B
        case 113: //LD (HL),C
        case 114: //LD (HL),D
        case 115: //LD (HL),E
        case 116: //LD (HL),H
        case 117: //LD (HL),L
        case 119: //LD (HL),A

            value1=peek_byte_no_time_no_change_mra(HL);
            p[50]=1;
            p[51]=reg_l;
            p[52]=reg_h;
            p[53]=value1;
            //printf("Storing on history %XH with value %XH coming from opcode %XH modifying (HL)\n",HL,value1,opcode);

        break;

        case 227: //EX (SP),HL

            cpu_history_regs_to_bin_aux_exsp(p);


        break;



        //Funciones CALL con condicion, si la condicion no se cumple, no se llamara a la funcion y por tanto
        //no se modificaria stack. Pero lo guardamos igualmente, da igual, si no se modifica, al hacer backwards, se dejara tal cual ya estaba
        case 196: //CALL NZ,NN
        case 197: //PUSH BC
        case 199: //RST 0

        case 204: //CALL Z,NN
        case 205: //CALL NN
        case 207: //RST 8

        case 212: //CALL NC,NN
        case 213: //PUSH DE
        case 215: //RST 16

        case 220: //CALL C,NN
        case 223: //RST 24

        case 228: //CALL PO,NN
        case 229: //PUSH HL
        case 231: //RST 32

        case 236: //CALL PE,NN
        case 239: //RST 40

        case 244: //CALL P,NN
        case 245: //PUSH AF
        case 247: //RST 48

        case 252: //CALL M,NN
        case 255: //RST 56

            puntero=reg_sp-2;
            value1=peek_byte_no_time_no_change_mra(puntero);
            value2=peek_byte_no_time_no_change_mra(puntero+1);
            p[50]=2;
            p[51]=value_16_to_8l(puntero);
            p[52]=value_16_to_8h(puntero);
            p[53]=value1;
            p[54]=value_16_to_8l(puntero+1);
            p[55]=value_16_to_8h(puntero+1);
            p[56]=value2;

            //printf("Storing on history %XH with value %02X%02XH coming from opcode %XH type PUSH/CALL/RST\n",puntero,value2,value1,opcode);
        break;


        //Prefijo ED
        case 237:
            switch (opcode1) {

                case 67: //LD (NN),BC
                case 83: //LD (NN),DE
                case 99: //LD (NN),HL
                case 115: //LD (NN),SP

                    puntero=value_8_to_16(peek_byte_no_time_no_change_mra(reg_pc+3),peek_byte_no_time_no_change_mra(reg_pc+2));
                    value1=peek_byte_no_time_no_change_mra(puntero);
                    value2=peek_byte_no_time_no_change_mra(puntero+1);
                    p[50]=2;
                    p[51]=value_16_to_8l(puntero);
                    p[52]=value_16_to_8h(puntero);
                    p[53]=value1;
                    p[54]=value_16_to_8l(puntero+1);
                    p[55]=value_16_to_8h(puntero+1);
                    p[56]=value2;
                    //printf("Storing on history %XH with value %02X%02XH coming from opcode ED%XH modifying 16 bits (NN)\n",puntero,value2,value1,opcode1);
                break;


                case 103: //RRD
                case 111: //RLD
                    value1=peek_byte_no_time_no_change_mra(HL);
                    p[50]=1;
                    p[51]=reg_l;
                    p[52]=reg_h;
                    p[53]=value1;
                    //printf("Storing on history %XH with value %XH coming from opcode ED%XH type RRD/RLD\n",HL,value1,opcode1);

                break;


                case 160: //LDI
                case 168: //LDD
                case 176: //LDIR
                case 184: //LDDR

                    value1=peek_byte_no_time_no_change_mra(DE);
                    p[50]=1;
                    p[51]=reg_e;
                    p[52]=reg_d;
                    p[53]=value1;
                    //printf("Storing on history %XH with value %XH coming from opcode ED%XH type LDI/LDD\n",DE,value1,opcode1);

                break;

                case 162: //INI
                case 170: //IND
                case 178: //INIR
                case 186: //INDR

                    value1=peek_byte_no_time_no_change_mra(HL);
                    p[50]=1;
                    p[51]=reg_l;
                    p[52]=reg_h;
                    p[53]=value1;
                    //printf("Storing on history %XH with value %XH coming from opcode ED%XH type INI/IND\n",HL,value1,opcode1);

                break;

            }



        break;

        //Prefijo 203
        case 203:
            switch (opcode1 & 192) {

                case 64:
                        //Nada
                break;

                case 128: //RES
                case 192: //SET

                    pref203_registro=devuelve_reg_offset(opcode1 & 7);
                    //pref203_numerobit=(opcode1 >> 3) & 7;

                    //registro ficticio 0 indica (HL)
                    if (pref203_registro==0) {
                        value1=peek_byte_no_time_no_change_mra(HL);
                        p[50]=1;
                        p[51]=reg_l;
                        p[52]=reg_h;
                        p[53]=value1;
                        //printf("Storing on history %XH with value %XH coming from opcode CB%XH type SET/RES (HL)\n",HL,value1,opcode1);
                    }

                break;

                default:
				switch(opcode1 & 56) {

					case 0:
                    case 8:
                    case 16:
                    case 24:
                    case 32:
                    case 40:
                    case 48:
                    case 56:
                        pref203_registro=devuelve_reg_offset(opcode1 & 7);

                        //registro ficticio 0 indica (HL)
                        if (pref203_registro==0) {
                            value1=peek_byte_no_time_no_change_mra(HL);
                            p[50]=1;
                            p[51]=reg_l;
                            p[52]=reg_h;
                            p[53]=value1;
                            //printf("Storing on history %XH with value %XH coming from opcode CB%XH type ROTATION (HL)\n",HL,value1,opcode1);
                        }

					break;


				}
			break;
            }


        break;

        //Prefijo 221, 253
        case 221:
        case 253:
            if (opcode==221) cual_registro_ixiy=&reg_ix;
            else cual_registro_ixiy=&reg_iy;

            switch (opcode1) {
                case 34: //LD (NN),IX
                    puntero=value_8_to_16(peek_byte_no_time_no_change_mra(reg_pc+3),peek_byte_no_time_no_change_mra(reg_pc+2));
                    value1=peek_byte_no_time_no_change_mra(puntero);
                    value2=peek_byte_no_time_no_change_mra(puntero+1);
                    p[50]=2;
                    p[51]=value_16_to_8l(puntero);
                    p[52]=value_16_to_8h(puntero);
                    p[53]=value1;
                    p[54]=value_16_to_8l(puntero+1);
                    p[55]=value_16_to_8h(puntero+1);
                    p[56]=value2;
                    //printf("Storing on history %XH with value %02X%02XH coming from opcode DD/FD%02XH modifying 16 bits (NN)\n",puntero,value2,value1,opcode1);

                break;

                case 52: //INC (IX+d)
                case 53: //DEC (IX+d)
                case 54: //LD (IX+d),N
                case 112: //LD (IX+d),B
                case 113: //LD (IX+d),C
                case 114: //LD (IX+d),D
                case 115: //LD (IX+d),E
                case 116: //LD (IX+d),H
                case 117: //LD (IX+d),L
                case 119: //LD (IX+d),A

                    desp=peek_byte_no_time_no_change_mra(reg_pc+2);

                    desp16=desp8_to_16(desp);
                    puntero=*cual_registro_ixiy + desp16;

                    value1=peek_byte_no_time_no_change_mra(puntero);

                    p[50]=1;
                    p[51]=value_16_to_8l(puntero);
                    p[52]=value_16_to_8h(puntero);
                    p[53]=value1;

                    //printf("Storing on history %XH with value %02XH coming from opcode DD/FD%02XH modifying 8 bits (IX+d)\n",puntero,value1,opcode1);


                break;

                case 227: //EX (SP),IX

                    cpu_history_regs_to_bin_aux_exsp(p);


                break;

                case 203:
                    //SET, RES, ROTACIONES... USANDO (IX/IY+d)
                    //Excepto instrucciones bit (del opcode 64 al 127), todas modifican (IX/IY+d)
                    //Formato DD/FB + CB + d + Opcode

                    opcode2=peek_byte_no_time_no_change_mra(reg_pc+3);
                    if (opcode2<64 || opcode2>127) {

                        desp=peek_byte_no_time_no_change_mra(reg_pc+2);
                        desp16=desp8_to_16(desp);
                        puntero=*cual_registro_ixiy + desp16;

                        value1=peek_byte_no_time_no_change_mra(puntero);

                        p[50]=1;
                        p[51]=value_16_to_8l(puntero);
                        p[52]=value_16_to_8h(puntero);
                        p[53]=value1;

                        //printf("Storing on history %XH with value %02XH coming from opcode DD/FD CB %02XH modifying 8 bits (IX+d)\n",puntero,value1,opcode2);
                        }
                break;
            }



        break;

    }

}



int cpu_history_max_elements=1000000; //1 millon
//multiplicado por 28 bytes, esto da que ocupa aproximadamente 28 MB por defecto

/*
Historial se guarda como un ring buffer
Tenemos indice que apunta a primer elemento en el ring. Esta inicializado a 0
Tenemos contador de total elementos en el ring. Inicializado a 0
Tenemos indice de siguiente posicion a insertar. Inicializado a 0
*/

int cpu_history_primer_elemento=0;
int cpu_history_total_elementos=0;
int cpu_history_siguiente_posicion=0;

z80_byte *cpu_history_memory_buffer=NULL;

z80_bit cpu_history_started={0};

int cpu_history_increment_pointer(int indice)
{
	//Si va mas alla del final, retornar 0
	indice++;

	if (indice>=cpu_history_max_elements) indice=0;
	return indice;
}

void cpu_history_init_buffer(void)
{
	cpu_history_primer_elemento=0;
	cpu_history_total_elementos=0;
	cpu_history_siguiente_posicion=0;


	if (cpu_history_memory_buffer!=NULL) {
		free(cpu_history_memory_buffer);
		//TODO: liberar buffer al inicializar cpu_core en set_machine
	}


	cpu_history_memory_buffer=malloc(cpu_history_max_elements*CPU_HISTORY_REGISTERS_SIZE);
	if (cpu_history_memory_buffer==NULL) cpu_panic("Can not allocate memory for cpu history");

}

long long int cpu_history_get_offset_index(int indice)
{
	return indice*CPU_HISTORY_REGISTERS_SIZE;
}

//int temp_conta=0;

void cpu_history_add_element(void)
{

	//-Insertar elemento: Meter contenido en posicion indicada por indice de siguiente posicion. Incrementar indice y si va mas alla del final, poner a 0
	//printf ("Insertando elemento en posicion %d. Primer elemento: %d Total_elementos: %d\n",
	//		cpu_history_siguiente_posicion,cpu_history_primer_elemento,cpu_history_total_elementos);


	//Obtener posicion en memoria
	long long int offset_memoria;
	offset_memoria=cpu_history_get_offset_index(cpu_history_siguiente_posicion);

	//printf ("Offset en memoria: %ld\n",offset_memoria);

	//Meter registros en memoria
	cpu_history_regs_to_bin(&cpu_history_memory_buffer[offset_memoria]);


	cpu_history_siguiente_posicion=cpu_history_increment_pointer(cpu_history_siguiente_posicion);

	//Si total elementos es menor que maximo, incrementar
	if (cpu_history_total_elementos<cpu_history_max_elements) cpu_history_total_elementos++;

	//Si total elementos es igual que maximo, no incrementar y aumentar posicion de indice del primer elemento. Si va mas alla del final, poner a 0
	else {
		cpu_history_primer_elemento=cpu_history_increment_pointer(cpu_history_primer_elemento);
	}

	//temp_conta++;
	//if (temp_conta==100) cpu_history_started.v=0;


}

int cpu_history_get_array_pos_element(int indice)
{
	//Retorna indice a posicion de un elemento teniendo en cuenta que el primero (indice=0) sera donde apunte cpu_history_primer_elemento
	//Aplicar retorno a 0 si se "da la vuelta"

	if (cpu_history_primer_elemento+indice<cpu_history_max_elements) {
		//No da la vuelta. Retornar tal cual
		//TODO: ver si no pide por un elemento mas alla del total escritos
		return cpu_history_primer_elemento+indice;
	}
	else {
		//Ha dado la vuelta.
		int indice_final=cpu_history_primer_elemento+indice-cpu_history_max_elements;
		return indice_final;
		//Ejemplo:
		//array de 3. primero es el 1 y pedimos el 2
		//tiene que retornar el 0:
		//1+2-3=0
		//array de 3. primero es el 2 y pedimos el 2
		//tiene que retornar el 1:
		//2+2-3=1
		//primero+indice-maximo
	}
}

//Nota: este comando se mantiene solo por compabilidad con Dezog (comando cpu-history get)
void cpu_history_legacy_get_registers_element(int indice,char *string_destino)
{

	if (indice<0) {
		strcpy(string_destino,"ERROR: index out of range");
		return;
	}

	if (indice>=cpu_history_total_elementos) {
		sprintf(string_destino,"ERROR: index beyond total elements (%d)",cpu_history_total_elementos);
		return;
	}

	int posicion=cpu_history_get_array_pos_element(indice);

	long long int offset_memoria=cpu_history_get_offset_index(posicion);

	cpu_history_legacy_regs_bin_to_string(&cpu_history_memory_buffer[offset_memoria],string_destino);
}

void cpu_history_get_registers_extended_element(int indice,char *string_destino)
{

	if (indice<0) {
		strcpy(string_destino,"ERROR: index out of range");
		return;
	}

	if (indice>=cpu_history_total_elementos) {
		sprintf(string_destino,"ERROR: index beyond total elements (%d)",cpu_history_total_elementos);
		return;
	}

	int posicion=cpu_history_get_array_pos_element(indice);

	long long int offset_memoria=cpu_history_get_offset_index(posicion);

	cpu_history_extended_regs_bin_to_string(&cpu_history_memory_buffer[offset_memoria],string_destino);
}

void cpu_history_get_pc_register_element(int indice,char *string_destino)
{

	if (indice<0) {
		strcpy(string_destino,"ERROR: index can't be negative");
		return;
	}

	if (indice>=cpu_history_total_elementos) {
		sprintf(string_destino,"ERROR: index beyond total elements (%d)",cpu_history_total_elementos);
		return;
	}

	int posicion=cpu_history_get_array_pos_element(indice);

	long long int offset_memoria=cpu_history_get_offset_index(posicion);

	cpu_history_reg_pc_bin_to_string(&cpu_history_memory_buffer[offset_memoria],string_destino);
}

//Dado un puntero z80_byte, con contenido de registros en binario, restaura los registros
//Registros 16 bits guardados en little endian
//El indice 0 es el elemento mas antiguo
void cpu_history_regs_bin_restore(int indice)
{

	if (indice<0) {
		//strcpy(string_destino,"ERROR: index out of range");
		return;
	}

	if (indice>=cpu_history_total_elementos) {
		//sprintf(string_destino,"ERROR: index beyond total elements (%d)",cpu_history_total_elementos);
		return;
	}



	int posicion=cpu_history_get_array_pos_element(indice);

	long long int offset_memoria=cpu_history_get_offset_index(posicion);


    z80_byte *p;

    p=&cpu_history_memory_buffer[offset_memoria];



	//Nota: funcion print_registers escribe antes BC que AF. Aqui ponemos AF antes, que es mas lógico
  /*sprintf (destino,"PC=%02x%02x SP=%02x%02x AF=%02x%02x BC=%02x%02x HL=%02x%02x DE=%02x%02x IX=%02x%02x IY=%02x%02x "
  				   "AF'=%02x%02x BC'=%02x%02x HL'=%02x%02x DE'=%02x%02x "
				   "I=%02x R=%02x IM%d IFF%c%c (PC)=%02x%02x%02x%02x (SP)=%02x%02x "
				   "MMU=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",*/

  reg_pc=value_8_to_16(p[1],p[0]);  	//pc
  reg_sp=value_8_to_16(p[3],p[2]), 	//sp

  reg_a=p[5]; //af
  Z80_FLAGS=p[4];

  reg_bc=value_8_to_16(p[7],p[6]); 	//bc
  reg_hl=value_8_to_16(p[9],p[8]); 	//hl
  reg_de=value_8_to_16(p[11],p[10]); 	//de
  reg_ix=value_8_to_16(p[13],p[12]); 	//ix
  reg_iy=value_8_to_16(p[15],p[14]); 	//iy

  reg_a_shadow=p[17]; //af'
  Z80_FLAGS_SHADOW=p[16];

  reg_b_shadow=p[19]; //bc'
  reg_c_shadow=p[18];

  reg_h_shadow=p[21]; //hl'
  reg_l_shadow=p[20];


  reg_d_shadow=p[23]; //de'
  reg_e_shadow=p[22];

  reg_i=p[24]; 		//I
  reg_r=p[25]; 		//R
  reg_r_bit7=reg_r&128;

  im_mode=p[26]; 	//IM

  iff1.v=p[27]&1;
  iff2.v=(p[27]>>1)&1;


  /*

  //contenido (pc) 4 bytes
  p[28],p[29],p[30],p[31],
  */
  //contenido (sp) 2 bytes.
  //Creo que esto solo es necesario por ejemplo para restaurar el stack cuando ha saltado una interrupcion
  //En este caso ahi apareceria la direccion de retorno, al saltar la interrupcion
  //Estrictamente hablando esto no es necesario para poder ir a la direccion anterior,
  //pues eso ya se restaura siempre con el registro PC
  //Pero para poder dejar el stack como estaba en cada caso, si es necesario.
  //Esto por ejemplo se puede ver con el juego Chase HQ, si no restauro esto y hago un Back Run,
  //veremos como parte de la pantalla no se está restaurando correctamente, porque probablemente
  //el juego utiliza el stack para dibujar la pantalla
  //En el resto de casos de modificaciones de stack (push, call, rst etc) ya se restaura con los bytes 50-56
  //TODO: esto no haria falta si cuando saltase una interrupcion, generasemos una entrada
  //en el historial donde se guardase el estado de (sp-1) y (sp-2) antes de saltar la interrupcion
  //como si fuese una instruccion CALL normal
  poke_byte_no_time(reg_sp,p[32]);
  poke_byte_no_time(reg_sp+1,p[33]);

  /*
  //MMU. Las paginas de debug_paginas_memoria_mapeadas, son valores de 16 bits escritas en Little Endian
  p[35],p[34], p[37],p[36], p[39],p[38], p[41],p[40],
  p[43],p[42], p[45],p[44], p[47],p[46], p[49],p[48]
  */

  //Para 128k/+2/+2a, restaurar pagina ram del segmento C000H
    //57,58: puertos paginacion 32765, 8189
    //Nota: podriamos llegar a usar la linea de MMU pero es mas comodo, al menos para 128k, usar estos dos
    z80_byte nuevo_puerto_32765=p[57];
    z80_byte nuevo_puerto_8189=p[58];


  if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
    if (puerto_32765!=nuevo_puerto_32765) {
        //printf("Hay cambio de ram y/o rom en %04XH. puerto 32765=%02XH. indice=%d\n",reg_pc,nuevo_puerto_32765,indice);

        //Quitar el bit de bloqueo de paginacion si es que hay alguno ahora mismo, o no podriamos paginar
        puerto_32765 &= (255-32);

        if (MACHINE_IS_SPECTRUM_128_P2) {
            puerto_32765=nuevo_puerto_32765;

            //asignar ram
            mem_page_ram_128k();

            //asignar rom
            mem_page_rom_128k();
        }

        else if (MACHINE_IS_SPECTRUM_P2A_P3) {
            //asignar ram
            mem128_p2a_write_page_port(32765,nuevo_puerto_32765);
        }
    }
  }



    if (MACHINE_IS_SPECTRUM_P2A_P3) {
        if (puerto_8189!=nuevo_puerto_8189) {
            //printf("Hay cambio de rom en %04XH. puerto 8189=%02XH. indice=%d\n",reg_pc,nuevo_puerto_8189,indice);

            //asignar rom
            mem128_p2a_write_page_port(8189,nuevo_puerto_8189);
        }
    }



  //50: flags
  //Bits 0-1: sobre direcciones modificadas: 0, ninguna, 1: una direccion, 2: dos direcciones
  //51-52: primera direccion modificada
  //53: valor antes de modificar primera direccion
  //54-55: segunda direccion modificada
  //56: valor antes de modificar segunda direccion
  int flags_direcciones=p[50] & 3;

  //1 o mas direcciones modificadas
  if (flags_direcciones>=1) {
      z80_int direccion=value_8_to_16(p[52],p[51]);
      z80_byte valor=p[53];
      poke_byte_no_time(direccion,valor);
      //printf("modifying first address %X value %X\n",direccion,valor);
  }

  //2 direcciones modificadas
  if (flags_direcciones>=2) {
      z80_int direccion=value_8_to_16(p[55],p[54]);
      z80_byte valor=p[56];
      poke_byte_no_time(direccion,valor);
      //printf("modifying second address %X value %X\n",direccion,valor);
  }


}


int cpu_history_get_total_elements(void)
{

	return cpu_history_total_elementos;
}

int cpu_history_get_max_size(void)
{
	return cpu_history_max_elements;
}

int cpu_history_set_max_size(int total)
{
	if (total<1 || total>CPU_HISTORY_MAX_ALLOWED_ELEMENTS) return -1;
	else {
		cpu_history_max_elements=total;
		cpu_history_init_buffer();
		return 0;
	}
}

z80_byte cpu_core_loop_history(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{


	//hacer cosas antes...
	//printf ("running cpu history addr: %04XH\n",reg_pc);



	if (cpu_history_started.v) {

		//Prueba comparar legacy registers con nuevo
		/*
		printf ("array elemento en posicion %d. Primer elemento: %d Total_elementos: %d\n",cpu_history_siguiente_posicion,cpu_history_primer_elemento,cpu_history_total_elementos);


		char registros_string_legacgy[1024];
		print_registers(registros_string_legacgy);
		printf ("Legacy registers: %s\n",registros_string_legacgy);


		//Guardar en binario y obtener de nuevo
		char registros_history_string[1024];
		z80_byte registers_history_binary[CPU_HISTORY_REGISTERS_SIZE];

		//Guardar en binario
		cpu_history_regs_to_bin(registers_history_binary);
		//Obtener en string
		cpu_history_legacy_regs_bin_to_string(registers_history_binary,registros_history_string);
		printf ("Newbin registers: %s\n",registros_history_string);
		*/


		//Ver si hay que ignorar repetidos

		//Si es halt lo ultimo
		if (cpu_trans_log_ignore_repeated_halt.v) {
			if (CPU_IS_Z80) {
					z80_byte opcode=peek_byte_no_time(reg_pc);
					if (opcode==118) {
							if (cpu_trans_log_last_was_halt<2) cpu_trans_log_last_was_halt++;
							//printf ("halts %d\n",cpu_trans_log_last_was_halt);

					}
					else {
							cpu_trans_log_last_was_halt=0;
					}

			}
			else {
					cpu_trans_log_last_was_halt=0;
			}
		}

		//Si es ldir o lddr lo ultimo
		if (cpu_trans_log_ignore_repeated_ldxr.v) {
			if (CPU_IS_Z80) {
					z80_byte op_preffix=peek_byte_no_time(reg_pc);
					z80_byte opcode=peek_byte_no_time(reg_pc+1);
					if (op_preffix==237 && (opcode==176 || opcode==184)) {
							if (cpu_trans_log_last_was_ldxr<2) cpu_trans_log_last_was_ldxr++;
					}
					else {
							cpu_trans_log_last_was_ldxr=0;
					}

			}
			else {
					cpu_trans_log_last_was_ldxr=0;
			}
		}

		int ignorar=0;

		//Si era halt los dos ultimos y hay que ignorarlo
		if (cpu_trans_log_ignore_repeated_halt.v && cpu_trans_log_last_was_halt>1) {
			//no hacer log
			ignorar=1;
		}


		//Si era ldir/lddr los dos ultimos y hay que ignorarlo
		if (cpu_trans_log_ignore_repeated_ldxr.v && cpu_trans_log_last_was_ldxr>1) {
			//no hacer log
			ignorar=1;
		}


		if (!ignorar) {

		cpu_history_add_element();

		}

		else {
			//printf ("Ignorando instruccion repetida en pc=%04XH %02X%02X\n",reg_pc,peek_byte_no_time(reg_pc),peek_byte_no_time(reg_pc+1));
		}

		//printf ("\n");
	}

	//Llamar a core anterior
	debug_nested_core_call_previous(cpu_history_nested_id_core);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos

	return 0;
}


//Punto de entrada alternativo util desde cambio velocidad cpu
//para reactivarlo sin perder los datos
void set_cpu_core_history_enable(void)
{
    debug_printf(VERBOSE_INFO,"Enabling Cpu history");

	if (cpu_history_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}



	cpu_history_nested_id_core=debug_nested_core_add(cpu_core_loop_history,"CPU history Core");

	cpu_history_enabled.v=1;




}

void set_cpu_core_history(void)
{

  set_cpu_core_history_enable();
  cpu_history_init_buffer();

}

void reset_cpu_core_history(void)
{
	debug_printf(VERBOSE_INFO,"Disabling Cpu history");
	if (cpu_history_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_core_del(cpu_history_nested_id_core);
	cpu_history_enabled.v=0;

	/*if (cpu_history_memory_buffer!=NULL) {
		free(cpu_history_memory_buffer);
		cpu_history_memory_buffer=NULL;

		//TODO: liberar buffer al inicializar cpu_core en set_machine
	}*/

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
                sprintf (buffer,"Element: %p (%d) id: %d name: %s pointer function: %p previous: %p next: %p\n",e,contador,e->id,e->function_name,e->funcion, e->previous,e->next);
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
                printf ("elemento: %p (%d) id: %d nombre: %s puntero_funcion: %p previous: %p next: %p\n",e,contador,e->id,e->function_name,e->funcion,
                        e->previous,e->next);
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
Los siguientes 5 secciones generados mediante:
cat template_nested_peek.tpl | sed 's/NOMBRE_FUNCION/peek_byte/g' > debug_nested_functions.c
cat template_nested_peek.tpl | sed 's/NOMBRE_FUNCION/peek_byte_no_time/g' >> debug_nested_functions.c
cat template_nested_poke.tpl | sed 's/NOMBRE_FUNCION/poke_byte/g' >> debug_nested_functions.c
cat template_nested_poke.tpl | sed 's/NOMBRE_FUNCION/poke_byte_no_time/g' >> debug_nested_functions.c
cat template_nested_push.tpl | sed 's/NOMBRE_FUNCION/push_valor/g' >> debug_nested_functions.c

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
		nested_list_push_valor=NULL;
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

	if (nested_list_push_valor!=NULL && push_valor==push_valor_nested_handler) {
		debug_dump_nested_print (result,"\nNested push_valor functions\n");
		debug_test_needed_adelante(nested_list_push_valor,result);
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

	//Parsear valor, es una expresion tal cual
	//valor_registro=parse_string_to_number(&texto[i]);
    valor_registro=exp_par_evaluate_expression_to_number(&texto[i]);

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

void debug_set_breakpoint_optimized(int breakpoint_index,char *condicion)
{
	//de momento suponemos que no esta optimizado
	optimized_breakpoint_array[breakpoint_index].optimized=0;

	//Si no es Z80, no optimizar
	if (!CPU_IS_Z80) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: CPU is not Z80. Not optimized");
		return;
	}

	//Aqui asumimos los siguientes:
	//PC=VALOR
	//MWA=VALOR
	//MRA=VALOR

	//Minimo 4 caracteres
	int longitud=strlen(condicion);
	if (longitud<4) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: length<4. Not optimized");
		return;
	}

	//Copiamos los 3 primeros caracteres
	char variable[4];
	int i;
	for (i=0;i<3;i++) variable[i]=condicion[i];

	/*
	+#define OPTIMIZED_BRK_TYPE_PC 0
+#define OPTIMIZED_BRK_TYPE_MWA 1
+#define OPTIMIZED_BRK_TYPE_MRA 2
*/
	int tipo_optimizacion=OPTIMIZED_BRK_TYPE_NINGUNA;
	int posicion_igual;

	//Ver si variable de 2 o 3 caracteres
	if (variable[2]=='=') {
		posicion_igual=2;
		variable[2]=0;

		//Comparar con admitidos
		if (!strcasecmp(variable,"PC")) tipo_optimizacion=OPTIMIZED_BRK_TYPE_PC;

	}

	else if (condicion[3]=='=') {
		//3 caracteres
		posicion_igual=3;
		variable[3]=0;

		//Comparar con admitidos
		if (!strcasecmp(variable,"MWA")) tipo_optimizacion=OPTIMIZED_BRK_TYPE_MWA;
		if (!strcasecmp(variable,"MRA")) tipo_optimizacion=OPTIMIZED_BRK_TYPE_MRA;

	}

	else {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: not detected = on 3th or 4th position. Not optimized");
		return; //Volver sin mas, no se puede optimizar
	}

	if (tipo_optimizacion==OPTIMIZED_BRK_TYPE_NINGUNA) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: not detected known optimizable register/variable. Not optimized");
		return;
	}

	//Sabemos el tipo de optimizacion
	debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Detected possible optimized type=%d",tipo_optimizacion);

	//Ver si lo que hay al otro lado es un valor y nada mas
	//Buscar si hay un espacio copiando en destino
	char valor_comparar[MAX_BREAKPOINT_CONDITION_LENGTH];

	int index_destino=0;

	for (i=posicion_igual+1;condicion[i]!=' ' && condicion[i];i++,index_destino++) {
		valor_comparar[index_destino]=condicion[i];
	}

	valor_comparar[index_destino]=0;

	//Si ha acabado con un espacio, no optimizar
	if (condicion[i]==' ') {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Space after number. Not optimized");
		return;
    }

	//Ver si eso que hay a la derecha del igual es una variable
	//int si_cond_opcode=0;
	unsigned int valor;

    //old parser valor=cpu_core_loop_debug_registro(valor_comparar,&si_cond_opcode);
	int final_numero;
	//printf ("Comprobar si [%s] es numero\n",valor_comparar);
	int result_is_number;
	result_is_number=exp_par_is_number(valor_comparar,&final_numero);
	debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Testing expression [%s] to see if it's a single number",valor_comparar);

	if (result_is_number<=0) {
			//Resulta que es una variable, no un numero . no optimizar
			debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Value is a variable. Not optimized");
			return;
    }

	//Ver si el final del numero ya es el final de texto
	if (valor_comparar[final_numero]!=0) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: More characters left after the number. Not optimized");
		return;
	}

	//Pues tenemos que suponer que es un valor. Parsearlo y meterlo en array de optimizacion
	valor=parse_string_to_number(valor_comparar);

	optimized_breakpoint_array[breakpoint_index].optimized=1;
	optimized_breakpoint_array[breakpoint_index].operator=tipo_optimizacion;
	optimized_breakpoint_array[breakpoint_index].valor=valor;

	debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Set optimized breakpoint operator index %d type %d value %04XH",
				breakpoint_index,tipo_optimizacion,valor);


}



//Indice entre 0 y MAX_BREAKPOINTS_CONDITIONS-1
//Retorna 0 si ok
int debug_set_breakpoint(int breakpoint_index,char *condicion)
{

    if (breakpoint_index<0 || breakpoint_index>MAX_BREAKPOINTS_CONDITIONS-1) {
      debug_printf(VERBOSE_ERR,"Index out of range setting breakpoint");
      return 1;
    }


	int result=exp_par_exp_to_tokens(condicion,debug_breakpoints_conditions_array_tokens[breakpoint_index]);
	if (result<0) {
		debug_breakpoints_conditions_array_tokens[breakpoint_index][0].tipo=TPT_FIN; //Inicializarlo vacio
		debug_printf (VERBOSE_ERR,"Error adding breakpoint [%s]",condicion);
		return 1;
	}

	//Ver si se puede evaluar la expresion resultante. Aqui basicamente generara error
	//cuando haya un parentesis sin cerrar
	int error_evaluate;

	//Si no es token vacio
	if (debug_breakpoints_conditions_array_tokens[breakpoint_index][0].tipo!=TPT_FIN) {
		exp_par_evaluate_token(debug_breakpoints_conditions_array_tokens[breakpoint_index],MAX_PARSER_TOKENS_NUM,&error_evaluate);
		if (error_evaluate) {
			debug_breakpoints_conditions_array_tokens[breakpoint_index][0].tipo=TPT_FIN; //Inicializarlo vacio
			debug_printf (VERBOSE_ERR,"Error adding breakpoint, can not be evaluated [%s]",condicion);
			return 1;
		}
	}



  	debug_breakpoints_conditions_saltado[breakpoint_index]=0;
  	debug_breakpoints_conditions_enabled[breakpoint_index]=1;

	//Llamamos al optimizador
	debug_set_breakpoint_optimized(breakpoint_index,condicion);

	//Miramos cual es el ultimo breakpoint activo
	debug_set_last_active_breakpoint();

	return 0;

}


void debug_set_watch(int watch_index,char *condicion)
{

    if (watch_index<0 || watch_index>DEBUG_MAX_WATCHES-1) {
      debug_printf(VERBOSE_ERR,"Index out of range setting watch");
      return;
    }


	int result=exp_par_exp_to_tokens(condicion,debug_watches_array[watch_index]);
	if (result<0) {
		debug_watches_array[watch_index][0].tipo=TPT_FIN; //Inicializarlo vacio
		debug_printf (VERBOSE_ERR,"Error adding watch [%s]",condicion);
	}


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


//Borra todas las apariciones de un breakpoint concreto
void debug_delete_all_repeated_breakpoint(char *texto)
{

	int posicion=0;

	//char breakpoint_add[64];

	//debug_get_daad_breakpoint_string(breakpoint_add);

	do {
		//Si hay breakpoint ahi, quitarlo
		posicion=debug_find_breakpoint_activeornot(texto);
		if (posicion>=0) {
			debug_printf (VERBOSE_DEBUG,"Clearing breakpoint at index %d",posicion);
			debug_clear_breakpoint(posicion);
		}
	} while (posicion>=0);

	//Y salir
}

//Poner un breakpoint si no estaba como existente y activo y ademas activar breakpoints
//Nota: quiza tendria que haber otra funcion que detecte que existe pero si no esta activo, que solo lo active sin agregar otro repetido
void debug_add_breakpoint_ifnot_exists(char *breakpoint_add)
{
	//Si no hay breakpoint ahi, ponerlo y
	int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion<0) {

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,"");
	}
}


//tipo: tipo maquina: 0: spectrum. 1: zx80. 2: zx81
void debug_view_basic_from_memory(char *results_buffer,int dir_inicio_linea,int final_basic,char **dir_tokens,
int inicio_tokens,z80_byte (*lee_byte_function)(z80_int dir), int tipo, int show_address, int show_current_line )
{

	  	z80_int dir;

  	debug_printf (VERBOSE_INFO,"Start Basic: %d. End Basic: %d",dir_inicio_linea,final_basic);

          int index_buffer;



          index_buffer=0;

          int salir=0;

  	z80_int numero_linea;

  	z80_int longitud_linea;

  	//deberia ser un byte, pero para hacer tokens de pi,rnd, inkeys en zx81, que en el array estan en posicion al final
  	z80_int byte_leido;

  	int lo_ultimo_es_un_token;

    if (show_current_line) {
        //Indicar linea actual y sentencia
        z80_int ppc,subppc;

        if (MACHINE_IS_ZX8081) {
            if (MACHINE_IS_ZX80_TYPE) ppc=peek_word_no_time(16386);

            //ZX81
            else ppc=peek_word_no_time(16391);

            sprintf (&results_buffer[index_buffer],"Current line: %5d\n",ppc);
            index_buffer +=20;
        }

        else {
            //Spectrum
            ppc=peek_word_no_time(23621);
            subppc=peek_byte_no_time(23623);

            sprintf (&results_buffer[index_buffer],"Current line: %5d:%3d\n",ppc,subppc);
            index_buffer +=24;
        }
    }


  	while (dir_inicio_linea<final_basic && salir==0) {
  		lo_ultimo_es_un_token=0;
  		dir=dir_inicio_linea;
  		//obtener numero linea. orden inverso
  		//numero_linea=(peek_byte_no_time(dir++))*256 + peek_byte_no_time(dir++);

        //agregar direccion si hay el setting habilitado para esto
        if (show_address) {
  		    sprintf (&results_buffer[index_buffer],";%5d\n",dir);
  		    index_buffer +=7;
        }

  		numero_linea=(lee_byte_function(dir++))*256;
  		numero_linea +=lee_byte_function(dir++);

  		//escribir numero linea
  		sprintf (&results_buffer[index_buffer],"%4d",numero_linea);
  		index_buffer +=4;

  		//obtener longitud linea. orden normal. zx80 no tiene esto
  		if (tipo!=1) {

  			//longitud_linea=(peek_byte_no_time(dir++))+256*peek_byte_no_time(dir++);
  			longitud_linea=(lee_byte_function(dir++));
  			longitud_linea += 256*lee_byte_function(dir++);

  			debug_printf (VERBOSE_DEBUG,"Line length: %d",longitud_linea);

  		}

  		else longitud_linea=65535;

  		//asignamos ya siguiente direccion.
  		dir_inicio_linea=dir+longitud_linea;

  		while (longitud_linea>0) {
  			byte_leido=lee_byte_function(dir++);
  			longitud_linea--;

  			if (tipo==1 || tipo==2) {
  				//numero
  				if (byte_leido==126) byte_leido=14;

  				else if (byte_leido==118) byte_leido=13;


  				//Convertimos a ASCII
  				else {

  					if (tipo==2) {
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
  						if (tipo==2) byte_leido=da_codigo_zx81_no_artistic(byte_leido);
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

  				if (tipo==0 || tipo==1) {
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
  				if (tipo==1) {
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


void debug_view_z88_print_token(z80_byte index,char *texto_destino)
{

	int salir=0;
	int i;

	for (i=0;!salir;i++) {
		if (z88_basic_rom_tokens[i].index==1) {
			sprintf (texto_destino,"?TOKEN%02XH?",index);
			salir=1;
		}

		if (z88_basic_rom_tokens[i].index==index) {
			strcpy (texto_destino,z88_basic_rom_tokens[i].token);
			salir=1;
		}
	}

}

void debug_view_z88_basic_from_memory(char *results_buffer,int dir_inicio_linea,int final_basic,
	z80_byte (*lee_byte_function)(z80_int dir) )
{

	  	z80_int dir;

  	debug_printf (VERBOSE_INFO,"Start Basic: %d. End Basic: %d",dir_inicio_linea,final_basic);

          int index_buffer;



          index_buffer=0;

          int salir=0;

  	z80_int numero_linea;

  	z80_byte longitud_linea;

  	//deberia ser un byte, pero para hacer tokens de pi,rnd, inkeys en zx81, que en el array estan en posicion al final
  	z80_int byte_leido;

  	//int lo_ultimo_es_un_token;


  	while (dir_inicio_linea<final_basic && salir==0) {
  		//lo_ultimo_es_un_token=0;
  		dir=dir_inicio_linea;
  		//obtener numero linea. orden inverso
  		//numero_linea=(peek_byte_no_time(dir++))*256 + peek_byte_no_time(dir++);
		longitud_linea=(lee_byte_function(dir++));
		//debug_printf (VERBOSE_DEBUG,"Line length: %d",longitud_linea);

  		numero_linea=lee_byte_function(dir++);
  		numero_linea +=(lee_byte_function(dir++))*256;

		if (numero_linea==65535) {
			salir=1;
		}

		else {

  			//escribir numero linea
  			sprintf (&results_buffer[index_buffer],"%5d ",numero_linea);
  			index_buffer +=6;


	  		//asignamos ya siguiente direccion.
  			dir_inicio_linea=dir+longitud_linea;

			//descontar los 3 bytes
			longitud_linea -=3;
			dir_inicio_linea -=3;

  			while (longitud_linea>0) {
  				byte_leido=lee_byte_function(dir++);
  				longitud_linea--;

				if (byte_leido>=32 && byte_leido<=126) {
					results_buffer[index_buffer++]=byte_leido;
				}

				else if (byte_leido>=128) {
					//token
					char buffer_token[20];
					debug_view_z88_print_token(byte_leido,buffer_token);
	  				sprintf (&results_buffer[index_buffer],"%s",buffer_token);
  					index_buffer +=strlen(buffer_token);
  					//lo_ultimo_es_un_token=1;
  				}


	  			else if (byte_leido==13) {
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



void debug_view_basic(char *results_buffer)
{

  	char **dir_tokens;
  	int inicio_tokens;




  	int dir_inicio_linea;
  	int final_basic;

	int tipo=0; //Asumimos spectrum



  	if (MACHINE_IS_SPECTRUM) {
  		//Spectrum

  		//PROG
  		dir_inicio_linea=peek_byte_no_time(23635)+256*peek_byte_no_time(23636);

  		//VARS
  		final_basic=peek_byte_no_time(23627)+256*peek_byte_no_time(23628);

  		dir_tokens=spectrum_rom_tokens;

  		inicio_tokens=163;

  	}

  	else if (MACHINE_IS_ZX81_TYPE) {
  		//ZX81
  		dir_inicio_linea=16509;

  		//D_FILE
  		final_basic=peek_byte_no_time(0x400C)+256*peek_byte_no_time(0x400D);

  		dir_tokens=zx81_rom_tokens;

  		inicio_tokens=192;

		tipo=2;
  	}

          //else if (MACHINE_IS_ZX80) {
    else  {
  		//ZX80
                  dir_inicio_linea=16424;

                  //VARS
                  final_basic=peek_byte_no_time(0x4008)+256*peek_byte_no_time(0x4009);

                  dir_tokens=zx80_rom_tokens;

                  inicio_tokens=213;

		tipo=1;
    }


	debug_view_basic_from_memory(results_buffer,dir_inicio_linea,final_basic,dir_tokens,inicio_tokens,peek_byte_no_time,tipo,debug_view_basic_show_address.v,1);

}


void debug_get_ioports(char *stats_buffer)
{

          //int index_op,
  	int index_buffer;



          //margen suficiente para que quepan lineas largas
          char buf_linea[100];

          index_buffer=0;

    if (CPU_IS_Z80) {
        sprintf (buf_linea,"ULA Data Bus value: %02XH\n",get_ula_databus_value() );
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
    }


  	if (MACHINE_IS_SPECTRUM) {
  		sprintf (buf_linea,"Spectrum FE port: %02X\n",out_254_original_value);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        if (if1_enabled.v) {
            sprintf (buf_linea,"Interface1:\n");
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"Port EF read:  %02XH\n",interface1_last_read_status_ef);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"%s %s %s %s %s\n",
                (interface1_last_read_status_ef & 0x10 ? "Busy" : "    "),
                (interface1_last_read_status_ef & 0x08 ? "DTR" :  "   "),
                (interface1_last_read_status_ef & 0x04 ? "Gap" :  "   "),
                (interface1_last_read_status_ef & 0x02 ? "    " : "Sync"),
                (interface1_last_read_status_ef & 0x01 ? "  "   : "WP")
            );
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"Port EF write: %02XH\n",interface1_last_value_port_ef);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);



            sprintf (buf_linea,"%s %s %s %s %s %s %s\n",
                (interface1_last_value_port_ef & 0x20 ? "    " :    "Wait"),
                (interface1_last_value_port_ef & 0x10 ? "   " :     "CTS"),
                (interface1_last_value_port_ef & 0x08 ? "     " :   "Erase"),
                (interface1_last_value_port_ef & 0x04 ? "R"     :   "W"),

                //Si es Erase sin Write, esta escribiendo GAP
                //Porque solo borra la cinta sin escribir ningun dato en ella
                ((interface1_last_value_port_ef & 0x08)==0 && (interface1_last_value_port_ef & 0x04) ? "GAPW" : "    "),

                (interface1_last_value_port_ef & 0x02 ? "    " :    "CCLK"),
                (interface1_last_value_port_ef & 0x01 ? "    " :    "CDAT")
            );
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"Port E7 read:  %02XH\n",interface1_last_read_e7);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"Port E7 write: %02XH\n",interface1_last_value_port_e7);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            int i;

            for (i=0;i<MAX_MICRODRIVES_BY_CONFIG;i++) {

                if (microdrive_status[i].microdrive_enabled) {
                    if (microdrive_status[i].raw_format) {
                        sprintf (buf_linea,"MDV%d current position: %d\n",i+1,microdrive_status[i].raw_current_position);
                        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
                    }
                    else {
                        sprintf (buf_linea,"MDV%d current sector: %3d\n",i+1,microdrive_status[i].mdr_current_sector);
                        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

                        //Dos saltos de linea porque es la del final
                        sprintf (buf_linea,"MDV%d offset sector:  %3d\n",i+1,microdrive_status[i].mdr_current_offset_in_sector);
                        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
                    }
                }

            }

        }

        if (hilow_enabled.v) {
            sprintf (buf_linea,"Hilow Datadrive:\n");
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
            /*
            OUT FFh
Bit 7 - Reset the Cassette Change bit (1 = reset, 0 = do nothing) (writing a 1 here will set bit 3 of IN FFh back to a 1)
Bit 6 - (Not used?)
Bit 5 - Motor On (1 = On, 0 = Stop)
Bit 4 - Write Gate (1 = Write Enabled, 0 = Write Disabled)
Bit 3 - Fast (1 = Fast, 0 = Slow)
Bit 2 - Track Select (1 = Side 1, 0 = Side 2)
Bit 1 - Forward (1 = Forward, 0 = Reverse)
Bit 0 - Data Bit Out (saving)
            */

            sprintf (buf_linea,"Port FF written:  %02XH\n",last_hilow_port_value);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
            /*
            //Bit 5 - Motor On (1 = On, 0 = Stop)
#define HILOW_PORT_MASK_MOTOR_ON 0x20

//Bit 4 - Write Gate (1 = Write Enabled, 0 = Write Disabled)
#define HILOW_PORT_MASK_WRITE_EN 0x10

//Bit 3 - Fast (1 = Fast, 0 = Slow)
#define HILOW_PORT_MASK_FAST 0x08

//Bit 2 - Track Select (1 = Side 1, 0 = Side 2)
#define HILOW_PORT_MASK_TRACK 0x04

//Bit 1 - Forward (1 = Forward, 0 = Reverse)
#define HILOW_PORT_MASK_FORWARD 0x02

//Bit 0 - Data Bit Out (saving)
#define HILOW_PORT_MASK_BIT_OUT 0x01
            */

            sprintf (buf_linea,"[%s] [%s] [%s] [%s] [%s] [%s]\n",
                (last_hilow_port_value & HILOW_PORT_MASK_MOTOR_ON ? "Motor On " :    "Motor Off"),
                (last_hilow_port_value & HILOW_PORT_MASK_WRITE_EN ? "Write" :     "Read "),
                (last_hilow_port_value & HILOW_PORT_MASK_FAST ? "40x" :   "10x"),
                (last_hilow_port_value & HILOW_PORT_MASK_FORWARD ? "FWD" : "BCK"),
                (last_hilow_port_value & HILOW_PORT_MASK_TRACK ? "Side A"     :   "Side B"),
                (last_hilow_port_value & HILOW_PORT_MASK_BIT_OUT ? "Out1" :    "Out0")
            );
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            z80_byte last_value_read;
            //En el caso de raw, leemos el estado del puerto instantaneo. Si no, leemos el ultimo valor
            if (hilow_rom_traps.v) last_value_read=hilow_read_port_ff_ddh(0xFF);
            else last_value_read=hilow_read_port_ff_raw(0xFF);

            sprintf (buf_linea,"Port FF read:  %02XH\n",last_value_read);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
            /*
IN FFh
Bit 7 - (Not used?)
Bit 6 - Data Bit In (loading)
Bit 5 - (Not used?)
Bit 4 - (Not used?)

Bit 3 - Cassette Change (0 = Cassette Changed, 1 = Not Changed) (it is "changed" if the Cassette Sense bit has ever gone low).
O sea, A 0 si se ha abierto la tapa en algun momento

Bit 2 - Cassette Sense (1 = Cassette in place, 0 = No cassette)
Bit 1 - Reverse (1 = Reverse, 0 = Forward) (Inverted last bit written to bit 1 of OUT FFh, used in "Start the tape" routine)
Bit 0 - Cassette Motion (0 = Moving, 1 = Stopped)
            */

            sprintf (buf_linea,"[%s] [%s] [%s] [%s] [%s]\n",
                (last_value_read & 0x40 ? "In1" :    "In0"),
                (last_value_read & 0x08 ? "NotOpned" :  "Opened  " ),
                (last_value_read & 0x04 ? "CasPre" :   "CasAbs"),
                (last_value_read & 0x02 ? "BCK" : "FWD"),
                (last_value_read & 0x01 ? "Stopped"     :   "Moving ")

            );
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            if (hilow_rom_traps.v) {
                sprintf (buf_linea,"Last sector:  %02XH\n",debug_hilow_last_sector);
                sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
            }

            else {

                int porcentaje=hilow_raw_transcurrido_cinta_porc();

                sprintf (buf_linea,"Position: %9d/%9d (%3d %%)\n",hilow_posicion_cabezal,hilow_raw_device_buffer_total_size,porcentaje);
                sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

                //Y en minutos, segundos, samples
                int samples=hilow_posicion_cabezal % HILOW_RAW_SAMPLE_FREQ;
                int segundos=hilow_posicion_cabezal / HILOW_RAW_SAMPLE_FREQ;
                int minutos=segundos / 60;
                int segundos_mostrar=segundos % 60;

                int minutos_total_cinta=hilow_raw_get_minutes_tape();

                sprintf (buf_linea,"      (%02d:%02d.%05d)/C%d\n",minutos,segundos_mostrar,samples,minutos_total_cinta);
                sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
            }
        }

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

  	if (MACHINE_IS_TIMEX_TS_TC_2068 || MACHINE_IS_CHLOE_280SE || MACHINE_IS_PRISM) {
  		sprintf (buf_linea,"Timex F4 port: %02X\n",timex_port_f4);
                  sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
          }

  	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED || MACHINE_IS_PRISM || MACHINE_IS_CHLOE || superupgrade_enabled.v || MACHINE_IS_CHROME || TBBLUE_MACHINE_128_P2 || TBBLUE_MACHINE_P2A) {

		//En el caso de zxuno, no mostrar si paginacion desactivada por DI7FFD
		int mostrar=1;

		if (MACHINE_IS_ZXUNO && zxuno_get_devcontrol_di7ffd()) mostrar=0;

		if (mostrar) {
            sprintf (buf_linea,"Spectrum 7FFD port: %02X\n",puerto_32765);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
		}
    }

  	if (MACHINE_IS_SPECTRUM_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED || MACHINE_IS_PRISM || superupgrade_enabled.v || MACHINE_IS_CHROME || TBBLUE_MACHINE_P2A) {
		//En el caso de zxuno, no mostrar si paginacion desactivada por DI1FFD
		int mostrar=1;

		if (MACHINE_IS_ZXUNO && zxuno_get_devcontrol_di1ffd()) mostrar=0;

		if (mostrar) {
  			sprintf (buf_linea,"Spectrum 1FFD port: %02X\n",puerto_8189);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
		}
  	}

    if (MACHINE_IS_PCW) {
        int i;
        for (i=0;i<4;i++) {
            sprintf (buf_linea,"PCW port %02XH: %02X <- Bank for %04XH\n",0x80+i,pcw_bank_registers[i],i*16384);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
        }
        sprintf (buf_linea,"PCW port F4H: %02X Lock: %s %s %s %s\n",pcw_port_f4_value,
            (pcw_port_f4_value & 0x80 ? "C000" : "    "),
            (pcw_port_f4_value & 0x40 ? "8000" : "    "),
            (pcw_port_f4_value & 0x20 ? "4000" : "    "),
            (pcw_port_f4_value & 0x10 ? "0000" : "    ")
        );

        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


        //Address of roller RAM. b7-5: bank (0-7). b4-1: address / 512.
        z80_byte roller_ram_bank=(pcw_port_f5_value >> 5) & 0x07;

        z80_int roller_ram_offset=(pcw_port_f5_value & 0x1F) * 512;
        sprintf (buf_linea,"PCW port F5H: %02X Roller RAM: Bank: %02XH Offset: %04XH\n",
            pcw_port_f5_value,roller_ram_bank,roller_ram_offset);

        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"PCW port F6H: %02X <- Scroll line\n",pcw_port_f6_value);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"PCW port F7H: %02X Display: %s %s\n",pcw_port_f7_value,
            (pcw_port_f7_value & 0x80 ? "(INV)" : "     "),
            (pcw_port_f7_value & 0x40 ? "(ON) " : "(OFF)")
        );

        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        //b6: 1 line flyback, read twice in succession indicates frame flyback. b5: FDC interrupt. b4: indicates 32-line screen.
        //b3-0: 300Hz interrupt counter: stays at 1111 until reset by in a,(&F4) (see above). &FC-&FD
        z80_byte valor_f8=pcw_get_port_f8_value();
        sprintf (buf_linea,"PCW port F8H: %02X %s %s %s INTCNT: %2d\n",valor_f8,
            (valor_f8 & 0x40 ? "(FFB)" : "     "),
            (valor_f8 & 0x20 ? "(FIN)" : "     "),
            (valor_f8 & 0x10 ? "(32L)" : "     "),
            valor_f8 & 0xF
        );
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        //sprintf (buf_linea,"PCW interrupt counter: %02X\n",pcw_interrupt_counter);
        //sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


    }

    if (pd765_enabled.v) {
  		sprintf (buf_linea,"\nPD765 status:\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"Motor: %s\n",(pd765_motor_status ? "On" : "Off"));
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"%s command: %s\n",
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_EXM_MASK ? "Running" : "Last   "),
            pd765_last_command_name() );
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"Main status register: %02XH\n",pd765_main_status_register);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"(%s %s %s %s %s %s %s %s)\n",
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_RQM_MASK ? "RQM" : "   "),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_DIO_MASK ? "DIO" : "   "),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_EXM_MASK ? "EXM" : "   "),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_CB_MASK  ? "CB " : "   "),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D3B_MASK ? "D3B" : "   "),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D2B_MASK ? "D2B" : "   "),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D1B_MASK ? "D1B" : "   "),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D0B_MASK ? "D0B" : "   ")
        );
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"Pending interrupt: %s\n",(pd765_interrupt_pending ? "Yes" : "No"));
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


/*
        sprintf (buf_linea,"Seeking: %s\n",(signal_se.running ? "Yes" : "No"));
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        //Chapuza. Cambiar esto por algo mejor, tipo "estado=reading data"
        sprintf (buf_linea,"Reading: %s\n",(
                 ((pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_EXM_MASK) && pd765_command_received==PD765_COMMAND_READ_DATA) ? "Yes" : "No"));
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"Writing: %s\n",(
                 ((pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_EXM_MASK) && pd765_command_received==PD765_COMMAND_WRITE_DATA) ? "Yes" : "No"));
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

*/

        sprintf (buf_linea,"Current Track: %d\n",pd765_pcn);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"Last Sector Read: %d\n",pd765_debug_last_sector_read);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"Last Sector ID Read: (CHRN) %02XH %02XH %02XH %02XH\n",
            pd765_debug_last_sector_id_c_read,pd765_debug_last_sector_id_h_read,pd765_debug_last_sector_id_r_read,pd765_debug_last_sector_id_n_read);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


        sprintf (buf_linea,"Read Bytes/sec: %d\n",pd765_read_stats_bytes_sec);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"Write Bytes/sec: %d\n",pd765_write_stats_bytes_sec);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


        //Salto de linea final
        sprintf (buf_linea,"\n");
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
    }

	if (diviface_enabled.v) {
  		sprintf (buf_linea,"Diviface control port: %02X\n",diviface_control_register);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		sprintf (buf_linea,"Diviface automatic paging: %s\n",(diviface_paginacion_automatica_activa.v ? "Yes" : "No")  );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
	}

    if (ide_enabled.v) {
  		sprintf (buf_linea,"ATA Registers\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Data:          %02X\n",ide_get_data_register() );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Error:         %02X\n",ide_get_error_register() );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Sector Count:  %02X\n",ide_register_sector_count );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Sector Number: %02X\n",ide_register_sector_number );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Cylinder Low:  %02X\n",ide_register_cylinder_low );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Cylinder High: %02X\n",ide_register_cylinder_high );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Drive/Head:    %02X\n",ide_register_drive_head );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Status:        %02X\n",ide_status_register );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

    }

    if (mmc_enabled.v) {
  		sprintf (buf_linea,"MMC Registers\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," State:        %s\n",(mmc_r1&1 ? "Idle" : "Not Idle") );
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea," Last Command: %02X\n",mmc_last_command);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
    }

    if (superupgrade_enabled.v) {
      sprintf (buf_linea,"Superupgrade 43B port: %02X\n",superupgrade_puerto_43b);
      sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
    }

  	if (MACHINE_IS_TBBLUE) {
		sprintf (buf_linea,"\nTBBlue port 123b:   %02X\n",tbblue_port_123b);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

		sprintf (buf_linea,"TBBlue port 123b_2: %02X\n",tbblue_port_123b_second_byte);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  								sprintf (buf_linea,"TBBlue last register: %02X\n",tbblue_last_register);
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


  								sprintf (buf_linea,"TBBlue Registers:\n");
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  								int index_ioport;
  								for (index_ioport=0;index_ioport<256;index_ioport++) {
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





  	if (sn_chip_present.v) {

  			sprintf (buf_linea,"\nSN76489AN chip:\n");
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


			int i;
			for (i=0;i<10;i++) {
					sprintf (buf_linea,"%02X:  %02X\n",i,sn_chip_registers[i]);
					sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
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

        //Segundos y minutos transcurridos segun el tim
        int doscienteavo_segundo=blink_tim[0];
        int segundos=blink_tim[1];
        int minutos=blink_tim[2]+(blink_tim[3]*256)+(blink_tim[4]*65536);

        sprintf (buf_linea,"TIM[FULL]: %d:%02d.%03d\n",minutos,segundos,doscienteavo_segundo);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


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

		//Registros DMA
  		sprintf (buf_linea,"\nZX-Uno DMA Registers:\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMASRC:  %02X%02X\n",zxuno_dmareg[0][1],zxuno_dmareg[0][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMADST:  %02X%02X\n",zxuno_dmareg[1][1],zxuno_dmareg[1][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMAPRE:  %02X%02X\n",zxuno_dmareg[2][1],zxuno_dmareg[2][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMALEN:  %02X%02X\n",zxuno_dmareg[3][1],zxuno_dmareg[3][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMAPROB: %02X%02X\n",zxuno_dmareg[4][1],zxuno_dmareg[4][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMACTRL: %02X\n",zxuno_ports[0xa0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMASTAT: %02X\n",zxuno_ports[0xa6]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  	}

  	if (MACHINE_IS_ZX8081) {
  		sprintf (buf_linea,"ZX80/81 last out port value: %02X\n",zx8081_last_port_write_value);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

  	if (MACHINE_IS_MSX) {
  		sprintf (buf_linea,"PPI Port A: %02X\n",msx_ppi_register_a);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"PPI Port B: %02X\n",msx_ppi_register_b);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"PPI Port C: %02X\n",msx_ppi_register_c);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"PPI Mode Port: %02X\n",msx_ppi_mode_port);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

  	if (MACHINE_IS_SVI) {
  		sprintf (buf_linea,"PPI Port A: %02X\n",svi_ppi_register_a);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"PPI Port B: %02X\n",svi_ppi_register_b);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"PPI Port C: %02X\n",svi_ppi_register_c);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

	if (MACHINE_HAS_VDP_9918A) {
  			sprintf (buf_linea,"\nVDP 9918A chip:\n");
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


			int i;
			for (i=0;i<16;i++) {
					sprintf (buf_linea,"%02X:  %02X\n",i,vdp_9918a_registers[i]);
					sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
			}
	}

	if (MACHINE_IS_SMS) {
        sprintf (buf_linea,"\nMapper registers:\n");
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"FFFC:  %02X\n",sms_mapper_FFFC);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"FFFD:  %02X\n",sms_mapper_FFFD);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"FFFE:  %02X\n",sms_mapper_FFFE);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"FFFF:  %02X\n",sms_mapper_FFFF);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

    }

	if (MACHINE_IS_CPC) {
  			sprintf (buf_linea,"\nCRTC Registers:\n");
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


			int i;
			for (i=0;i<32;i++) {
					sprintf (buf_linea,"%02X:  %02X\n",i,cpc_crtc_registers[i]);
					sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
			}


            sprintf (buf_linea,"\nPPI Port A:  %02X\n",cpc_ppi_ports[0]);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"PPI Port B:  %02X\n",cpc_ppi_ports[1]);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"PPI Port C:  %02X\n",cpc_ppi_ports[2]);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

            sprintf (buf_linea,"PPI Control:  %02X\n",cpc_ppi_ports[3]);
            sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


			sprintf (buf_linea,"\nGate Registers:\n");
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);



			for (i=0;i<4;i++) {
					sprintf (buf_linea,"%02X:  %02X\n",i,cpc_gate_registers[i]);
					sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
			}

			sprintf (buf_linea,"\nPort DF: %02XH\n",cpc_port_df);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

	}



    if (MACHINE_IS_QL) {
        int value_rtc=(ql_zx8032_readbyte(0x18000)<<24) | (ql_zx8032_readbyte(0x18001)<<16) | (ql_zx8032_readbyte(0x18002)<<8) | ql_zx8032_readbyte(0x18003);
  		sprintf (buf_linea,"PC_CLOCK: %0d\n",value_rtc);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"PC_INTR: %02X\n",ql_pc_intr);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

        sprintf (buf_linea,"MC_STAT: %02X\n",ql_mc_stat);
        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
    }

  	if (ay_chip_present.v && (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_MSX1 || MACHINE_IS_SVI || MACHINE_IS_CPC)) {
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


    stats_buffer[index_buffer]=0;

}

int debug_if_breakpoint_action_menu(int index)
{

 //Si indice -1 quiza ha saltado por un membreakpoint
 if (index==-1) return 1;

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

        direccion=exp_par_evaluate_expression_to_number(breakpoint_action_command_argv[0]);
        valor=exp_par_evaluate_expression_to_number(breakpoint_action_command_argv[1]);

        debug_printf (VERBOSE_DEBUG,"Running write command address %d value %d",direccion,valor);

        poke_byte_z80_moto(direccion,valor);

      }
    }

    else if (!strcmp(comando_sin_parametros,"call")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        unsigned int direccion;

        direccion=exp_par_evaluate_expression_to_number(breakpoint_action_command_argv[0]);

        debug_printf (VERBOSE_DEBUG,"Running call command address : %d",direccion);
        if (CPU_IS_MOTOROLA) debug_printf (VERBOSE_DEBUG,"Unimplemented call command for motorola");
        else {
          push_valor(reg_pc,PUSH_VALUE_TYPE_CALL);
          reg_pc=direccion;
        }
      }
    }


    else if (!strcmp(comando_sin_parametros,"disassemble")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        unsigned int direccion;

        direccion=exp_par_evaluate_expression_to_number(breakpoint_action_command_argv[0]);


        char string_direccion[10];

        menu_debug_print_address_memory_zone(string_direccion, direccion);

        size_t longitud_opcode;
        char buffer[100];


        debugger_disassemble(buffer,99,&longitud_opcode,direccion);

        printf("%s %s\n",string_direccion,buffer);


      }
    }



    else if (!strcmp(comando_sin_parametros,"printc")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        unsigned int caracter;

        caracter=exp_par_evaluate_expression_to_number(breakpoint_action_command_argv[0]);

        debug_printf (VERBOSE_DEBUG,"Running printc command character: %d",caracter);

        printf ("%c",caracter);
      }
    }

    else if (!strcmp(comando_sin_parametros,"printe")) {
      if (parametros[0]==0) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running printe command : %s",parametros);
        //char resultado_expresion[256];
        //debug_watches_loop(parametros,resultado_expresion);
  		char salida[MAX_BREAKPOINT_CONDITION_LENGTH];
		char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

		exp_par_evaluate_expression(parametros,salida,string_detoken);

        printf ("%s\n",salida);
      }
    }


    else if (!strcmp(comando_sin_parametros,"printregs")) {
        char buffer[2048];
        print_registers(buffer);

        printf ("%s\n",buffer);
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

    else if (!strcmp(comando_sin_parametros,"putv")) {
      if (parametros[0]==0) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running putv command : %s",parametros);
		z80_byte resultado;
		resultado=exp_par_evaluate_expression_to_number(parametros);
        debug_memory_zone_debug_write_value(resultado);
      }
    }

    else if (!strcmp(comando_sin_parametros,"reset-tstatp")) {
      debug_printf (VERBOSE_DEBUG,"Running reset-tstatp command");
      debug_t_estados_parcial=0;
    }

    //save-binary file addr len
    else if (!strcmp(comando_sin_parametros,"save-binary")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<3) debug_printf (VERBOSE_DEBUG,"Command needs three parameters");
      else {

        char *archivo=breakpoint_action_command_argv[0];
        int direccion=exp_par_evaluate_expression_to_number(breakpoint_action_command_argv[1]);
        int longitud=exp_par_evaluate_expression_to_number(breakpoint_action_command_argv[2]);

        debug_printf (VERBOSE_DEBUG,"Running save-binary command file %s address %d length %d",archivo,direccion,longitud);

        save_binary_file(archivo,direccion,longitud);

      }
    }


    else if (!strcmp(comando_sin_parametros,"start-transaction-log")) {
        debug_printf (VERBOSE_DEBUG,"Running start-transaction-log command");
        if (transaction_log_filename[0]!=0) {
            if (cpu_transaction_log_enabled.v==0) {
                set_cpu_core_transaction_log();
            }
        }
    }

    else if (!strcmp(comando_sin_parametros,"stop-transaction-log")) {
        debug_printf (VERBOSE_DEBUG,"Running stop-transaction-log command");
        if (cpu_transaction_log_enabled.v) {
            reset_cpu_core_transaction_log();
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

        //Si es ifrom
        if (segmento==0 && ifrom_enabled.v==1) {
                sprintf (texto_pagina_short,"IB%d",ifrom_active_bank);
                sprintf (texto_pagina,"iFrom Block %d",ifrom_active_bank);
                return;
        }

        //Si es betadisk
        if (segmento==0 && betadisk_enabled.v && betadisk_active.v) {
                sprintf (texto_pagina_short,"BDSK");
                sprintf (texto_pagina,"Betadisk ROM");
                return;
        }

        //Si es transtape
        if (segmento==0 && transtape_enabled.v && transtape_mapped_rom_memory.v) {
                sprintf (texto_pagina_short,"TRAN");
                sprintf (texto_pagina,"Transtape MEM");
                return;
        }

        //Si es specmate
        if (segmento==0 && specmate_enabled.v && specmate_mapped_rom_memory.v) {
                sprintf (texto_pagina_short,"SPMT");
                sprintf (texto_pagina,"Spec-Mate MEM");
                return;
        }

        //Si es phoenix
        if (segmento==0 && phoenix_enabled.v && phoenix_mapped_rom_memory.v) {
                sprintf (texto_pagina_short,"PHOE");
                sprintf (texto_pagina,"Phoenix MEM");
                return;
        }

        //Si es defcon
        if (segmento==0 && defcon_enabled.v && defcon_mapped_rom_memory.v) {
                sprintf (texto_pagina_short,"DEFC");
                sprintf (texto_pagina,"Defcon MEM");
                return;
        }

        //Si es ramjet
        if (segmento==0 && ramjet_enabled.v && ramjet_mapped_rom_memory.v) {
                sprintf (texto_pagina_short,"RAMJ");
                sprintf (texto_pagina,"Ramjet MEM");
                return;
        }

        //Si es interface007
        if (segmento==0 && interface007_enabled.v && interface007_mapped_rom_memory.v) {
                sprintf (texto_pagina_short,"I007");
                sprintf (texto_pagina,"I007 MEM");
                return;
        }

        //Si es dinamid3
        if (segmento==0 && dinamid3_enabled.v && dinamid3_mapped_rom_memory.v) {
                sprintf (texto_pagina_short,"DIN3");
                sprintf (texto_pagina,"Dinamid3 MEM");
                return;
        }


        //Si es barbanegra
        if (segmento==0 && hilow_bbn_enabled.v && hilow_bbn_mapped_memory.v) {
                sprintf (texto_pagina_short,"BBN");
                sprintf (texto_pagina,"Barbanegra MEM");
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

        //Si es lec memory. Segmento alto
        /*
        Memoria >=32768, siempre es lec. El chip de memoria "original" del spectrum se extrae
        Memoria <=32767, si esta mapeada all-ram, es memoria lec (y si hay una escritura, solo ira a lec).
        Y si no esta mapeada all-ram, es memoria original del spectrum
        */
        if (lec_enabled.v) {
            if (segmento==0) {
                if (lec_all_ram() ) {
                    sprintf (texto_pagina_short,"LE%d",debug_paginas_memoria_mapeadas[0]);
                    sprintf (texto_pagina,"LEC RAM %d",debug_paginas_memoria_mapeadas[0]);
                }

                //Si no, lo dejamos tal cual estaba por defecto (ROM)
                return;
            }

            if (segmento==1) {
                sprintf (texto_pagina_short,"LE%d",debug_paginas_memoria_mapeadas[1]);
                sprintf (texto_pagina,"LEC RAM %d",debug_paginas_memoria_mapeadas[1]);
                return;
            }

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

                        //Si lec y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && lec_enabled.v) {
                            debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                            debug_registers_get_mem_page_extended(1,segmentos[1].longname,segmentos[1].shortname);
                        }

                        //Si kartusho y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && kartusho_enabled.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si ifrom y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && ifrom_enabled.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si betadisk y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && betadisk_enabled.v && betadisk_active.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si transtape
                        if (MACHINE_IS_SPECTRUM && transtape_enabled.v && transtape_mapped_rom_memory.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si specmate
                        if (MACHINE_IS_SPECTRUM && specmate_enabled.v && specmate_mapped_rom_memory.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si phoenix
                        if (MACHINE_IS_SPECTRUM && phoenix_enabled.v && phoenix_mapped_rom_memory.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si defcon
                        if (MACHINE_IS_SPECTRUM && defcon_enabled.v && defcon_mapped_rom_memory.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si ramjet
                        if (MACHINE_IS_SPECTRUM && ramjet_enabled.v && ramjet_mapped_rom_memory.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si interface007
                        if (MACHINE_IS_SPECTRUM && interface007_enabled.v && interface007_mapped_rom_memory.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si dinamid3
                        if (MACHINE_IS_SPECTRUM && dinamid3_enabled.v && dinamid3_mapped_rom_memory.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si barbanegra
                        if (MACHINE_IS_SPECTRUM && hilow_bbn_enabled.v && hilow_bbn_mapped_memory.v) {
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
                          if (MACHINE_IS_ZXUNO && !zxuno_is_chloe_mmu() ) {
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
	  				sprintf (segmentos[pagina].shortname,"B%02X",blink_mapped_memory_banks[pagina]);
	  				sprintf (segmentos[pagina].longname,"BANK %02X",blink_mapped_memory_banks[pagina]);
	                                segmentos[pagina].length=16384;
	                                segmentos[pagina].start=16384*pagina;
				}
  			}


  			//MSX
  			if (MACHINE_IS_MSX) {
				int pagina;
				segmentos_totales=4;
				z80_byte mapping_register=msx_ppi_register_a;

				for (pagina=0;pagina<4;pagina++) {
	  				sprintf (segmentos[pagina].shortname,"SL%d",mapping_register & 3);
	  				sprintf (segmentos[pagina].longname,"Slot %02X",mapping_register & 3);
					segmentos[pagina].length=16384;
					segmentos[pagina].start=16384*pagina;

					mapping_register=mapping_register >> 2;
				}
  			}

  			//SVI
  			if (MACHINE_IS_SVI) {

				segmentos_totales=2;

				//Por defecto: bank01 rom basic, bank02 ram
				char low_type='O';
				z80_byte low_number=1;

				char high_type='A';
				z80_byte high_number=2;

    z80_byte page_config=ay_3_8912_registros[ay_chip_selected][15];

/*
PSG Port B Output

Bit Name    Description
1   /CART   Memory bank 11, ROM 0000-7FFF (Cartridge /CCS1, /CCS2)
2   /BK21   Memory bank 21, RAM 0000-7FFF
3   /BK22   Memory bank 22, RAM 8000-FFFF
4   /BK31   Memory bank 31, RAM 0000-7FFF

5   /BK32   Memory bank 32, RAM 8000-FFFF
6   CAPS    Caps-Lock diod
7   /ROMEN0 Memory bank 12, ROM 8000-BFFF* (Cartridge /CCS3)
8   /ROMEN1 Memory bank 12, ROM C000-FFFF* (Cartridge /CCS4)
*/


				if (page_config!=0xFF) {

					//Ver bits activos
					//Memory bank 11, ROM 0000-7FFF (Cartridge /CCS1, /CCS2)
					if ((page_config & 1)==0) {
						low_number=11;
					}

					//Memory bank 21, RAM 0000-7FFF
					if ((page_config & 2)==0) {
						low_number=21;
						low_type='A';
					}

					//Memory bank 22, RAM 8000-FFFF
					if ((page_config & 4)==0) {
						high_number=22;
					}

					//Memory bank 31, RAM 0000-7FFF
					if ((page_config & 8)==0) {
						low_number=31;
						low_type='A';
					}

					//Memory bank 32, RAM 8000-FFFF
					if ((page_config & 16)==0) {
						high_number=32;
					}

					//TODO bits 6,7
				}



				sprintf (segmentos[0].shortname,"R%c%02d",low_type,low_number);
				sprintf (segmentos[0].longname,"R%cM %02d",low_type,low_number);

				sprintf (segmentos[1].shortname,"R%c%02d",high_type,high_number);
				sprintf (segmentos[1].longname,"R%cM %02d",high_type,high_number);


				segmentos[0].length=32768;
				segmentos[0].start=0;

				segmentos[1].length=32768;
				segmentos[1].start=32768;



  			}

			if (MACHINE_IS_COLECO) {
				segmentos_totales=8;

  				int pagina;
  				for (pagina=0;pagina<segmentos_totales;pagina++) {

					segmentos[pagina].length=8192;
					segmentos[pagina].start=8192*pagina;
				}

				//0-3

				strcpy (segmentos[0].shortname,"BIO");
				strcpy (segmentos[0].longname,"BIOS ROM");

				strcpy (segmentos[1].shortname,"EXP");
				strcpy (segmentos[1].longname,"Expansion port");

				strcpy (segmentos[2].shortname,"EXP");
				strcpy (segmentos[2].longname,"Expansion port");

				strcpy (segmentos[3].shortname,"RAM");
				strcpy (segmentos[3].longname,"RAM (1 KB)");

				//4-7
				for (pagina=4;pagina<8;pagina++) {
						strcpy (segmentos[pagina].shortname,"CR");
						strcpy (segmentos[pagina].longname,"Cartridge ROM");
				}

				/*
0000-1FFF = BIOS ROM
2000-3FFF = Expansion port
4000-5FFF = Expansion port
6000-7FFF = RAM (1K mapped into an 8K spot)
8000-9FFF = Cart ROM
A000-BFFF = Cart ROM
C000-DFFF = Cart ROM
E000-FFFF = Cart ROM
				*/
			}

			if (MACHINE_IS_SG1000) {
				/*
$0000-$bfff	Cartridge (ROM/RAM/etc)
$c000-$c3ff	System RAM
$c400-$ffff	System RAM (mirrored every 1KB)
				*/
				segmentos_totales=2;

				//0
				segmentos[0].length=0xc000;
				segmentos[0].start=0;
				strcpy (segmentos[0].shortname,"ROM");
				strcpy (segmentos[0].longname,"Cartridge ROM");

				//1
				segmentos[1].length=16384;
				segmentos[1].start=0xc000;
				strcpy (segmentos[1].shortname,"RAM");
				strcpy (segmentos[1].longname,"RAM (1 KB)");
			}

            //TODO SMS


  			//Paginas RAM en CHLOE
  			if (MACHINE_IS_CHLOE || is_zxuno_chloe_mmu() ) {
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
                          if (MACHINE_IS_TIMEX_TS_TC_2068) {
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


							//Si es ifrom
        					if (pagina==0 && ifrom_enabled.v==1) {
                				sprintf (segmentos[pagina].shortname,"IB%d",ifrom_active_bank);
                				sprintf (segmentos[pagina].longname,"iFrom Block %d",ifrom_active_bank);
							}


  							segmentos[pagina].length=16384;
	                        segmentos[pagina].start=16384*pagina;

                        }

            }

  			if (MACHINE_IS_PCW) {

                segmentos_totales=4;
                int pagina;
                for (pagina=0;pagina<4;pagina++) {

                    sprintf (segmentos[pagina].shortname,"RAM%d",pcw_banks_paged_read[pagina]);
                    sprintf (segmentos[pagina].longname,"RAM %d",pcw_banks_paged_read[pagina]);

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


			//Caso tbblue para paginas de 8 kb
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

  //En el caso de Spectrum, si es un RST 8 y en caso de esxdos, hay que descartar el byte siguiente
  if (MACHINE_IS_SPECTRUM) {
      if (esxdos_handler_enabled.v) {
        z80_byte opcode=peek_byte_no_time(direccion);
        if (opcode==207) {
            //Y siempre que byte siguiente sea un comando de esxdos (>=128)
            z80_byte comando_esxdos=peek_byte_no_time(direccion+1);
            if (comando_esxdos>=128) {
                direccion_final++;
                debug_printf(VERBOSE_DEBUG,"Skipping the next byte after RST8 because it is a esxdos call");
            }
        }
      }
  }

  direccion_final=adjust_address_space_cpu(direccion_final);


  //Parar hasta volver de la instruccion actual o cuando se produzca algun evento de apertura de menu, como un breakpoint
  int antes_menu_abierto=menu_abierto;
  menu_abierto=0;
  int salir=0;
  while (get_pc_register()!=direccion_final && !salir) {
    //printf("Running until step over ends. Addr=%04XH final=%04XH\n",get_pc_register(),direccion_final);
    debug_core_lanzado_inter.v=0;
    cpu_core_loop();

    if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
        debug_run_until_return_interrupt();
    }

    if (menu_abierto) salir=1;
  }

  //Si antes no estaba menu abierto, dejar el estado final (sea que se haya abierto el menu o no)
  //Si antes estaba el menu abierto, dejarlo abierto
  if (antes_menu_abierto) menu_abierto=antes_menu_abierto;


  debug_printf(VERBOSE_DEBUG,"End Step over");
}



int debug_get_opcode_length(unsigned int direccion)
{
  char buffer_retorno[101];
  size_t longitud_opcode;

  debugger_disassemble(buffer_retorno,100,&longitud_opcode,direccion);

  return longitud_opcode;

}

//Retorna si el texto indicado es de tipo PC=XXXX
//Retorna 0 si no
//Retorna 1 si es
int debug_text_is_pc_condition(char *cond)
{
	if (cond[0]=='P' || cond[0]=='p') {
				if (cond[1]=='C' || cond[1]=='c') {
					if (cond[2]=='=') {
						//Ahora a partir de aqui ver que no haya ningun espacio
						int j;

						for (j=3;cond[j];j++) {
							if (cond[j]==' ') return 0;
						}
						return 1;
					}
				}
	}
	return 0;
}

//Retorna si el breakpoint indicado es de tipo PC=XXXX y action=""
//Retorna 0 si no
//Retorna 1 si es
int debug_return_brk_pc_condition(int indice)
{
	if (debug_breakpoints_enabled.v==0) return -1;

	char *cond;

	int i=indice;

		if (debug_breakpoints_conditions_enabled[i]) {
			if (debug_breakpoints_actions_array[i][0]!=0) return 0;


			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
			cond=buffer_temp;


			return debug_text_is_pc_condition(cond);
		}


	return 0;
}

//Retorna si hay breakpoint tipo PC=XXXX donde XXXX coincide con direccion y action=""
//Teniendo en cuenta que breakpoints tiene que estar enable, y ese breakpoint tambien tiene que estar activado
//Retorna -1 si no
//Retorna indice a breakpoint si coincide
int debug_return_brk_pc_dir_condition(menu_z80_moto_int direccion)
{

	if (debug_breakpoints_enabled.v==0) return -1;

	char *cond;

	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
			if (debug_return_brk_pc_condition(i)) {


			//TODO: esto se podria mejorar analizando los tokens
			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
			cond=buffer_temp;


				menu_z80_moto_int valor=parse_string_to_number(&cond[3]);
				if (valor==direccion) return i;
			}
	}

	return -1;
}

//Retorna lista de breakpoints de tipo PC=dir
int debug_return_brk_pc_dir_list(menu_z80_moto_int *lista)
{

	if (debug_breakpoints_enabled.v==0) return 0;

	char *cond;

    int total_breakpoints=0;

	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
			if (debug_return_brk_pc_condition(i)) {


			//TODO: esto se podria mejorar analizando los tokens
			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
			cond=buffer_temp;


				menu_z80_moto_int valor=parse_string_to_number(&cond[3]);
				//printf("direccion: %d\n",valor);
                lista[total_breakpoints]=valor;
                total_breakpoints++;
			}
	}

	return total_breakpoints;
}

//Retorna primera posicion en array de breakpoint libres. -1 si no hay
int debug_find_free_breakpoint(void)
{
	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {

			if (debug_breakpoints_conditions_array_tokens[i][0].tipo==TPT_FIN) return i;

	}

	return -1;
}

//Retorna primera posicion en array que coindice con breakpoint y que este activado
int debug_find_breakpoint(char *to_find)
{

	if (debug_breakpoints_enabled.v==0) return -1;

	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
		if (debug_breakpoints_conditions_enabled[i]) {

			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
			if  (!strcasecmp(buffer_temp,to_find)) return i;

		}
	}

	return -1;
}

//Retorna primera posicion en array que coindice con breakpoint,este activo o no
int debug_find_breakpoint_activeornot(char *to_find)
{

	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {

			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);

			//printf ("%d temp: [%s] comp: [%s]\n",i,buffer_temp,to_find);

			if (!strcasecmp(buffer_temp,to_find)) return i;

	}

	return -1;
}



//Agrega un breakpoint, con action en la siguiente posicion libre. -1 si no hay
//Retorna indice posicion si hay libre

int debug_add_breakpoint_free(char *breakpoint, char *action)
{
	int posicion=debug_find_free_breakpoint();
	if (posicion<0) {
		debug_printf (VERBOSE_ERR,"No free breakpoint entry");
		return -1;
	}

	debug_set_breakpoint(posicion,breakpoint);
	debug_set_breakpoint_action(posicion,action);

	return posicion;

}

void debug_clear_breakpoint(int indice)
{
	//Elimina una linea de breakpoint. Pone condicion vacia y enabled a 0
	debug_set_breakpoint(indice,"");
	debug_set_breakpoint_action(indice,"");
	//debug_breakpoints_conditions_enabled[indice]=0;
	debug_breakpoints_conditions_disable(indice);
}

void debug_get_stack_moto(menu_z80_moto_int p,int items, char *texto)
{
	int i;
  	for (i=0;i<items;i++) {
		//menu_z80_moto_int valor=16777216*peek_byte_z80_moto(p)+65536*peek_byte_z80_moto(p+1)+256*peek_byte_z80_moto(p+2)+256*peek_byte_z80_moto(p+3);
		sprintf(&texto[i*9],"%02X%02X%02X%02X ",peek_byte_z80_moto(p+i*4),peek_byte_z80_moto(p+i*4+1),
            peek_byte_z80_moto(p+i*4+2),peek_byte_z80_moto(p+i*4+3) );
	}
}

z80_int debug_get_stack_z80_value(int i)
{
    return peek_byte_z80_moto(reg_sp+i*2)+256*peek_byte_z80_moto(reg_sp+1+i*2);
}

//Retorna valores en el stack separados por espacios
//Para Z80: retorna 16 bits
//Para scmp: no implementado aun
void debug_get_stack_values(int items, char *texto)
{

	//Por si acaso, por defecto
	texto[0]=0;

	if (CPU_IS_Z80) {
		int i;
  		for (i=0;i<items;i++) {
			z80_int valor=debug_get_stack_z80_value(i);
			sprintf(&texto[i*5],"%04X ",valor);
		  }

	}

	if (CPU_IS_MOTOROLA) {
		//int i;
		menu_z80_moto_int p=m68k_get_reg(NULL, M68K_REG_SP);
		debug_get_stack_moto(p,items,texto);
	}


}

//Retornar el user stack de motorola
void debug_get_user_stack_values(int items, char *texto)
{

	//Por si acaso, por defecto
	texto[0]=0;

	if (CPU_IS_MOTOROLA) {
		//int i;
		menu_z80_moto_int p=m68k_get_reg(NULL, M68K_REG_USP);
		debug_get_stack_moto(p,items,texto);
	}


}


void debug_get_t_estados_parcial(char *buffer_estadosparcial)
{

			int estadosparcial=debug_t_estados_parcial;

			if (estadosparcial>999999999) sprintf (buffer_estadosparcial,"%s","OVERFLOW");
			else sprintf (buffer_estadosparcial,"%09u",estadosparcial);
}


z80_byte *memory_zone_debug_ptr=NULL;

int memory_zone_current_size=0;

void debug_memory_zone_debug_reset(void)
{
	memory_zone_current_size=0;
}

void debug_memory_zone_debug_write_value(z80_byte valor)
{
	if (memory_zone_debug_ptr==NULL) {
		debug_printf (VERBOSE_DEBUG,"Allocating memory for debug memory zone");
		memory_zone_debug_ptr=malloc(MEMORY_ZONE_DEBUG_MAX_SIZE);
		if (memory_zone_debug_ptr==NULL) {
			cpu_panic ("Can not allocate memory for debug memory zone");
		}
	}

	//Si aun hay espacio disponible
	if (memory_zone_current_size<MEMORY_ZONE_DEBUG_MAX_SIZE) {
		memory_zone_debug_ptr[memory_zone_current_size]=valor;
		memory_zone_current_size++;
	}
	//else {
	//	printf ("Memory zone full\n");
	//}
}


//Obtener fecha, hora , minutos y microsegundos
//Retorna longitud del texto
int debug_get_timestamp(char *destino)
{


	struct timeval tv;
	struct tm* ptm;
	long microseconds;


	// 2015/01/01 11:11:11.999999"
	// 12345678901234567890123456
	const int longitud_timestamp=26;

	/* Obtain the time of day, and convert it to a tm struct. */
	gettimeofday (&tv, NULL);
	ptm = localtime (&tv.tv_sec);
	/* Format the date and time, down to a single second. */
	char time_string[40];

	strftime (time_string, sizeof(time_string), "%Y/%m/%d %H:%M:%S", ptm);

	microseconds = tv.tv_usec;
		/* Print the formatted time, in seconds, followed by a decimal point and the microseconds. */
	sprintf (destino,"%s.%06ld ", time_string, microseconds);


	return longitud_timestamp;


}


int remote_is_number_or_letter(char c)
{
	if (c>='0' && c<='9') return 1;
	if (c>='A' && c<='Z') return 1;
	if (c>='a' && c<='z') return 1;
	return 0;

}


int remote_string_contains_label(char *string, char *label)
{

	char *coincide;
	coincide=util_strcasestr(string, label);

	if (coincide==string) return 1;
	else return 0;


}

/*
Busca etiqueta en archivo codigo fuente
Consideramos formato:
LXXXXX  (para QL)
LXXXX   (para Z80)
Se hace busqueda sin tener en cuenta mayusculas/minusculas
Retorna numero de linea en parsed source code
Retorna -1 si no encontrado

Nota: estas funciones remote_ antes eran parte de remote.c, se separaron para poder
cargar codigo fuente desde fuera de ZRCP, e incluso cuando no hay soporte de threads compilado y por tanto ZRCP no esta compilado

*/

int debug_load_source_code_skip_columns=0;

int remote_find_label_source_code(char *label_to_find)
{
	int linea=0;
	/*
//Puntero a indices en archivo parsed source (lineas sin comentarios, con codigo real)
int *remote_parsed_source_code_indexes_pointer=NULL;
//Tamanyo de ese array
int remote_parsed_source_code_indexes_total;
	*/

	for (linea=0;linea<remote_parsed_source_code_indexes_total;linea++) {
		//Comparar label
		int indice=remote_parsed_source_code_indexes_pointer[linea];
		char *puntero=&remote_raw_source_code_pointer[indice+debug_load_source_code_skip_columns];
		if (remote_string_contains_label(puntero,label_to_find)) {
		     //temp
        	     //printf ("%s\n",puntero);
		     return linea;
		}
	}

	return -1;

}

char remote_get_raw_source_code_char(int posicion)
{
	if (posicion>remote_tamanyo_archivo_raw_source_code) return 0;
	else return remote_raw_source_code_pointer[posicion];
}

int remote_is_char_10_or_13(char c)
{
	if (c==10 || c==13) return 1;
	return 0;
}

//Retorna indice a array de lineas. -1 si no existe
int remote_disassemble_find_label(unsigned int direccion)
{
    int pos_source=-1;

    char buffer_label[128];

    if (remote_debug_settings & 4) {
        if (CPU_IS_MOTOROLA) sprintf(buffer_label,"%05X",direccion);
        else sprintf(buffer_label,"%04X",direccion);
    }
    else {
        if (CPU_IS_MOTOROLA) sprintf(buffer_label,"L%05X",direccion);
        else sprintf(buffer_label,"L%04X",direccion);
    }

    pos_source=remote_find_label_source_code(buffer_label);

    return pos_source;

}

//expulsar/desactivar source code
void load_source_code_eject(void)
{
    remote_tamanyo_archivo_raw_source_code=0;
}


//Retorna 0 si no hay error
int remote_load_source_code(char *archivo)
{

	if (!si_existe_archivo(archivo)) {
		debug_printf(VERBOSE_ERR,"ERROR. File %s not found",archivo);
		return 1;
	}

    strcpy(last_source_code_file,archivo);

	remote_tamanyo_archivo_raw_source_code=0;

	//Desasignar memoria si conviene
	if (remote_raw_source_code_pointer!=NULL) free(remote_raw_source_code_pointer);

	//Ver tamanyo archivo
	long long int tamanyo;

	tamanyo=get_file_size(archivo);

	remote_raw_source_code_pointer=malloc(tamanyo+1); //y el 0 del final

	if (remote_raw_source_code_pointer==NULL) {
		debug_printf(VERBOSE_ERR,"ERROR. Can not allocate memory to load source code file\n");
		return 1;
	}

	FILE *ptr_sourcecode;
	int leidos;

	ptr_sourcecode=fopen(archivo,"rb");

	if (ptr_sourcecode==NULL) {
		debug_printf(VERBOSE_ERR,"ERROR. Can not open source code file\n");
		return 1;
	}

	leidos=fread(remote_raw_source_code_pointer,1,tamanyo,ptr_sourcecode);
	fclose(ptr_sourcecode);

	//El 0 del final
	remote_raw_source_code_pointer[tamanyo]=0;

	if (leidos!=tamanyo) {
		debug_printf(VERBOSE_ERR,"ERROR reading source code file\n");
		return 1;
	}


	remote_tamanyo_archivo_raw_source_code=tamanyo;


	//Contar maximo de lineas aproximadas. Segun cuantos codigos 10 o 13
	int remote_raw_max_source_code_lineas=1;

	int puntero;
	char c;

	for (puntero=0;puntero<remote_tamanyo_archivo_raw_source_code;puntero++) {
		c=remote_get_raw_source_code_char(puntero);
		if (remote_is_char_10_or_13(c) ) remote_raw_max_source_code_lineas++;
	}

	debug_printf(VERBOSE_DEBUG,"Maximum raw source code lines: %d",remote_raw_max_source_code_lineas);

	//Crear indice a lineas en raw source

	//Asignamos memoria para esos indices
	//Desasignar si conviene
	if (remote_raw_source_code_indexes_pointer!=NULL) {
		debug_printf(VERBOSE_DEBUG,"Freeing previous memory to hold indexes to raw source code file");
		free (remote_raw_source_code_indexes_pointer);
	}

	remote_raw_source_code_indexes_pointer=malloc(sizeof(int)*remote_raw_max_source_code_lineas);

	if (remote_raw_source_code_indexes_pointer==NULL) cpu_panic("Can not allocate memory to index source code file");

	remote_raw_source_code_indexes_total=0;
	//Primera linea
	remote_raw_source_code_indexes_pointer[remote_raw_source_code_indexes_total++]=0;

	int puntero_raw=0;

	while (puntero_raw<remote_tamanyo_archivo_raw_source_code) {
		//Buscar primer codigo 10 o 13
		while (puntero_raw<remote_tamanyo_archivo_raw_source_code && !remote_is_char_10_or_13(remote_get_raw_source_code_char(puntero_raw))) {

            //Quitar codigos no deseados, como tabulador
            if (remote_raw_source_code_pointer[puntero_raw]==9) remote_raw_source_code_pointer[puntero_raw]=' ';

			puntero_raw++;
		}

		if (puntero_raw<remote_tamanyo_archivo_raw_source_code) {
			//Buscar siguiente codigo no 10 o 13
			while (puntero_raw<remote_tamanyo_archivo_raw_source_code && remote_is_char_10_or_13(remote_get_raw_source_code_char(puntero_raw))) {
					//Metemos 0 en esa posicion
					remote_raw_source_code_pointer[puntero_raw]=0;
                        		puntero_raw++;
                	}

			if (puntero_raw<remote_tamanyo_archivo_raw_source_code) {
				//Aqui tenemos puntero a siguiente linea
				remote_raw_source_code_indexes_pointer[remote_raw_source_code_indexes_total++]=puntero_raw;
			}
		}
	}

	debug_printf(VERBOSE_DEBUG,"Total effective raw source code lines: %d",remote_raw_source_code_indexes_total);

	//Mostramos cada linea
	int i;
	for (i=0;i<remote_raw_source_code_indexes_total;i++) {
		int indice=remote_raw_source_code_indexes_pointer[i];
		debug_printf (VERBOSE_DEBUG,"Full source line %d : index: %d contents: %s",i,indice,&remote_raw_source_code_pointer[indice]);
	}

	//Crear indice a lineas en source code efectivas (lineas con codigo no comentarios)
	/*

int *remote_parsed_source_code_indexes_pointer=NULL;
int remote_parsed_source_code_indexes_total;

	*/
        //Asignamos memoria para esos indices
        //Desasignar si conviene
        if (remote_parsed_source_code_indexes_pointer!=NULL) {
                debug_printf(VERBOSE_DEBUG,"Freeing previous memory to hold indexes to parsed source code file");
                free (remote_parsed_source_code_indexes_pointer);
        }

        remote_parsed_source_code_indexes_pointer=malloc(sizeof(int)*remote_raw_source_code_indexes_total); //Habra un maximo igual a lineas raw


        if (remote_parsed_source_code_indexes_pointer==NULL) cpu_panic("Can not allocate memory to index source code file parsed");

        remote_parsed_source_code_indexes_total=0;
        //Primera linea
        //remote_parsed_source_code_indexes_pointer[remote_parsed_source_code_indexes_total++]=0;

	//linea leida raw
	int linea_raw=0;

	//Bucle hasta maximo de lineas raw (remote_raw_source_code_indexes_total)
	//Por cada linea, saltar las que no empiezan por alfanumerico (e ignorando primeros espacios o tabs)

	for (linea_raw=0;linea_raw<remote_raw_source_code_indexes_total;linea_raw++) {
		//Saltar primeros espacios o tabs
		int puntero_raw=remote_raw_source_code_indexes_pointer[linea_raw];

		char *texto=&remote_raw_source_code_pointer[puntero_raw];
		while (*texto==' ' || *texto=='\t') {
			texto++;
		}

		if (*texto!=0) {

			//Ver si empieza con alfanumerico
			if (remote_is_number_or_letter(*texto)) {
				remote_parsed_source_code_indexes_pointer[remote_parsed_source_code_indexes_total++]=puntero_raw;
			}

		}
	}

        debug_printf(VERBOSE_DEBUG,"Total effective parsed source code lines: %d",remote_parsed_source_code_indexes_total);

        //Mostramos cada linea
        for (i=0;i<remote_parsed_source_code_indexes_total;i++) {
                int indice=remote_parsed_source_code_indexes_pointer[i];
                debug_printf (VERBOSE_DEBUG,"Parsed source line %d : index: %d contents: %s",i,indice,&remote_raw_source_code_pointer[indice]);
        }

    return 0;

}

/*
z80_byte debug_view_basic_variables_util_invert_nibble(z80_byte valor)
{
    z80_byte high=(valor>>4)& 0xF;
    z80_byte low=valor & 0xF;

    return high|(low<<4);
}
*/


void  debug_view_basic_variables_util_final_division(char *buffer,int exponente_final,int total_mantissa,int signo_exponente,int signo_valor_final)
{

    int entero;


    int decimales;

    if (signo_exponente>0) {



        int multiplicado=exponente_final*total_mantissa;

        //Sacar decimales
        entero=multiplicado/10000;

        decimales=entero*10000;

        decimales=multiplicado-decimales;


    }

    else {
        int division=total_mantissa/exponente_final;

        entero=division/10000;


        decimales=entero*10000;

        decimales=division-decimales;

    }

    //printf("entero: %d\n",entero);

    if (signo_valor_final<0) sprintf(buffer,"-%d.%04d",entero,decimales);
    else sprintf(buffer,"%d.%04d",entero,decimales);

}

/*
Funcion para mostrar el valor numérico en pantalla

Toda la parte de coma flotante se puede mejorar y corregir mucho, he evitado usar variables tipo float de C,
esta todo obtenido mediante enteros de 32 bits, trabajando con tablas de mantisa multiplicadas por 10000
Además sólo tengo en cuenta 16 de los 32 bits posibles de la mantisa, por tanto los valores muchas veces son aproximados
NO se debe tomar como una función perfecta sino como algo que nos da un indicativo del valor APROXIMADO de la variable en coma flotante
y no da siempre un valor exacto
*/
void debug_view_basic_variables_print_number(z80_int dir,char *buffer_linea)
{

    //Si es de 16 bits entera o ZX80, primer byte a 0
    z80_byte number_type=peek_byte_no_time(dir);
    if (number_type==0 || MACHINE_IS_ZX80_TYPE) {
        int variable_value;

        if (MACHINE_IS_ZX80_TYPE) {
            variable_value=peek_word_no_time(dir);
            if (variable_value>32767) {
                //negativo
                variable_value=-(65536-variable_value);
            }
        }

        else {

            variable_value=peek_word_no_time(dir+2);

            //negativo
            if (peek_byte_no_time(dir+1)==0xFF) {
                variable_value=-(65536-variable_value);
            }
        }
        sprintf(buffer_linea,"(int)%d",variable_value);
    }
    else {
        //floating

        int signo_valor_final=+1;

        //Aproximacion muy bruta
        int total_mantissa=5000;  //0.5

        z80_byte mant1=peek_byte_no_time(dir+1);

        //total_mantissa valores multiplicados por 10000

        //Estos valores obtenidos desde el propio basic del Spectrum, manipulando los bytes de valores de variables
        //y viendo el valor que obtienen en pantalla


        if (mant1 & 128) signo_valor_final=-1;  //Valor negativo

        //                              0.0000
        if (mant1 & 64) total_mantissa += 2500;  //10000 * 0.25
        if (mant1 & 32) total_mantissa += 1250;  //10000 * 0.125
        if (mant1 & 16) total_mantissa +=  625;  //10000 * 0.0625
        if (mant1 & 8)  total_mantissa +=  312;  //10000 * 0.03125
        if (mant1 & 4)  total_mantissa +=  156;  //10000 * 0.015625
        if (mant1 & 2)  total_mantissa +=   78;  //10000 * 0.0078125
        if (mant1 & 1)  total_mantissa +=   39;  //10000 * 0.00390625

        z80_byte mant2=peek_byte_no_time(dir+2);

        //                               0.0000
        if (mant2 & 128) total_mantissa +=   19;  //10000 * 0.00195
        if (mant2 & 64) total_mantissa  +=   10;  //10000 * 0.00097
        if (mant2 & 32) total_mantissa  +=    5;  //10000 * 0.00048
        if (mant2 & 16) total_mantissa  +=    2;  //10000 * 0.00024
        if (mant2 & 8)  total_mantissa  +=    1;  //10000 * 0.00012

        //Cualquiera de los otros bits son aproximaciones de 0.00xx y no tenemos precision (contando enteros X 10000) para usarlos
        //por tanto los descarto


        int exponente=peek_byte_no_time(dir);

        int signo_exponente;

        if (exponente>=0x80) {
            signo_exponente=+1;
            exponente -=0x80;
        }
        else {
            signo_exponente=-1;
            exponente=0x80-exponente;
        }


        int exponente_final=1;
        int i;


        //mirar si esto excede un valor final mayor de 31 bits aprox
        //mantisa maxima 4000 aprox
        //2^31 / 4000 = 536870. usa hasta bit 24
        //printf("exponente %d mantissa %d signo %d\n",exponente,total_mantissa,signo_valor_final);
        if (exponente>18) {
            if (signo_exponente>0) {
                if (signo_valor_final<0) sprintf(buffer_linea,"(float)(-%d X 2^%d)/10000",total_mantissa,exponente);
                else sprintf(buffer_linea,"(float)(%d X 2^%d)/10000",total_mantissa,exponente);
            }
            else {
                if (signo_valor_final<0) sprintf(buffer_linea,"(float)(-%d / 2^%d)/10000",total_mantissa,exponente);
                else sprintf(buffer_linea,"(float)(%d / 2^%d)/10000",total_mantissa,exponente);
            }
            return;
        }



        //rotar 1<<
        for (i=0;i<exponente;i++) {
            exponente_final=exponente_final<<1;
        }


        //int valor_total;

        char buffer_valor_total[100];
        debug_view_basic_variables_util_final_division(buffer_valor_total,exponente_final,total_mantissa,signo_exponente,signo_valor_final);

        //valor_total=(exponente_final*total_mantissa)/10000;

        /*printf("(float) Exp:%02XH Mant: %02X%02X%02X%02XH Aprox: %s\n",
            peek_byte_no_time(dir),
            peek_byte_no_time(dir+1),
            peek_byte_no_time(dir+2),
            peek_byte_no_time(dir+3),
            peek_byte_no_time(dir+4),
            buffer_valor_total
        );*/

        sprintf(buffer_linea,"(float)%s",buffer_valor_total);

    }
}

#define MAX_DEBUG_BASIC_VARIABLES_LINE_LENGTH 300

//Convertir caracter a espacio ascii segun si spectrum o zx81
z80_byte debug_view_basic_variables_getchar(z80_byte caracter)
{
    if (MACHINE_IS_ZX8081) {
        if (caracter>=64) caracter='.';
        else caracter=da_codigo_zx81_no_artistic(caracter);
    }

    if (caracter<32 || caracter>126) caracter='.';

    return caracter;
}


//da la letra de la primera variable seugn si spectrum o zx81
z80_byte debug_view_basic_variables_letra_variable(z80_byte first_byte_letter)
{
    z80_byte letra_variable;

    if (MACHINE_IS_ZX8081) {
        letra_variable=first_byte_letter+59;
        if (letra_variable<'A' || letra_variable>'Z') letra_variable='?';
    }

    else {
        letra_variable=first_byte_letter+96;
        if (letra_variable<'a' || letra_variable>'z') letra_variable='?';
    }

    return letra_variable;
}

int debug_view_basic_variables_print_string(z80_int dir,int longitud_variable,char *results_buffer,int maxima_longitud_texto)
{

    char buffer_linea[MAX_DEBUG_BASIC_VARIABLES_LINE_LENGTH+1];
    sprintf (buffer_linea,"\"");
    util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

    int resultado=0;

    int maximo_mostrar=longitud_variable;

    int limite_alcanzado=0;

    //Para que quepa a$=""
    if (maximo_mostrar>MAX_DEBUG_BASIC_VARIABLES_LINE_LENGTH-10) {
        maximo_mostrar=MAX_DEBUG_BASIC_VARIABLES_LINE_LENGTH-10;
        limite_alcanzado=1;
    }

    int i;

    for (i=0;i<maximo_mostrar;i++) {
        z80_byte caracter=peek_byte_no_time(dir+i);
        caracter=debug_view_basic_variables_getchar(caracter);

        buffer_linea[i]=caracter;
    }

    if (limite_alcanzado) {
        buffer_linea[i]='.';
        i++;
        buffer_linea[i]='.';
        i++;
        buffer_linea[i]='.';
        i++;
    }

    buffer_linea[i]='"';
    buffer_linea[i+1]='\n';
    buffer_linea[i+2]=0;

    resultado=util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

    return resultado;
}

int debug_view_basic_variables_print_dim_alpha(char *results_buffer,z80_int puntero,int total_dimensiones,
    int dimensiones[],int indice,int posicion_actual[],z80_int total_offset,int maxima_longitud_texto,int es_array_numero)
{
    /*
    Esta funcion recursiva seguro que se puede optimizar pero... de momento funciona
    Recorre un array alfanumerico de varias dimensiones

    Variables:
    results_buffer: string donde ir guardando el texto
    puntero: apunta a direccion inicial donde esta el primer caracter
    total_dimensiones: de cuantas dimensiones es el array. por ejemplo DIM a$(9,8,7). el total_dimensiones es 3
    dimensiones: tamanyo de cada dimension, con el ejemplo anterior: dimensiones[0]=9. dimensiones[1]=8. dimensiones[2]=7.
    indice: en que dimension nos fijamos. se incrementar cada vez que se llama a la funcion recursivamente
    array posicion_actual: dice en que elemento nos fijamos. por ejempoo si miramos el elemento (2,5,7), el array contiene:
        posicion_actual[0]=2;
        posicion_actual[1]=5;
        posicion_actual[2]=7;
    total_offset: indica el desplazamiento en memoria donde esta ubicada la letra que mostraremos en pantalla
    maxima_longitud_texto: maxima longitud que permite escribir en el string results_buffer
     */

    int i;

    /*
    La idea es recorrer todos los elementos de cada dimension, o sea, para un array de DIM A$(2,3) haremos:
    A$(1,1)
    A$(1,2)
    A$(1,3)
    A$(2,1)
    A$(2,2)
    A$(2,3)
    Para un array de DIM B$(2,3,4) haremos:
    A$(1,1,1)
    A$(1,1,2)
    A$(1,1,3)
    A$(1,1,4)

    A$(1,2,1)
    A$(1,2,2)
    A$(1,2,3)
    A$(1,2,4)

    A$(1,3,1)
    A$(1,3,2)
    A$(1,3,3)
    A$(1,3,4)

    A$(2,1,1)
    A$(2,1,2)
    A$(2,1,3)
    A$(2,1,4)

    A$(2,2,1)
    A$(2,2,2)
    A$(2,2,3)
    A$(2,2,4)

    A$(2,3,1)
    A$(2,3,2)
    A$(2,3,3)
    A$(2,3,4)
    */

    //Nota: aqui no gestionamos el resultado de error de util_concat_string por no hacer mas complicada la llamada recursiva
    //aunque logicamente el limite se controla, si esta al maximo de longitud permitida, util_concat_string no escribira nada
    //printf("dimensiones[indice] :%d total_dimensiones: %d\n",dimensiones[indice],total_dimensiones);

    //controlar maximo dimensiones
    if (total_dimensiones>30) {
        debug_printf(VERBOSE_ERR,"Maximum dimension for an array reached: 30");
        total_dimensiones=30;
    }

    //Controlar tambien tamaño de cada dimension
    int tamanyo_dimension=dimensiones[indice];

    if (tamanyo_dimension>100) {
        debug_printf(VERBOSE_ERR,"Maximum size for an array dimension reached: 100");
        tamanyo_dimension=100;
    }

    for (i=0;i<tamanyo_dimension;i++) {
        //printf("%d(%d) ",i,indice);
        posicion_actual[indice]=i;
        if (indice<total_dimensiones-1) {
            total_offset=debug_view_basic_variables_print_dim_alpha(results_buffer,puntero,total_dimensiones,dimensiones,
                        indice+1,posicion_actual,total_offset,maxima_longitud_texto,es_array_numero);
        }
        else {
            //
            //printf(".");
            char buffer_linea[100];

            //printf("(");
            util_concat_string(results_buffer,"(",maxima_longitud_texto);
            int j;
            for (j=0;j<total_dimensiones;j++) {

                char final_char=(j<total_dimensiones-1 ? ',' : ')');
                //printf("%d%c",posicion_actual[j]+1,final_char);

                sprintf(buffer_linea,"%d%c",posicion_actual[j]+1,final_char);
                util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);
            }


            if (es_array_numero) {
                util_concat_string(results_buffer,"=",maxima_longitud_texto);
                int tamanyo_numero=5;

                if (MACHINE_IS_ZX80_TYPE) tamanyo_numero=2;

                z80_int offset_numero=puntero+total_offset*tamanyo_numero;
                debug_view_basic_variables_print_number(offset_numero,buffer_linea);

                util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

                util_concat_string(results_buffer,"\n",maxima_longitud_texto);
            }
            else {
                z80_byte letra_leida=peek_byte_no_time(puntero+total_offset);
                letra_leida=debug_view_basic_variables_getchar(letra_leida);
                if (letra_leida<32 || letra_leida>126) letra_leida='?';


                //printf(" (offset=%d) (=%c)\n",total_offset,letra_leida);


                sprintf(buffer_linea,"=\"%c\"\n",letra_leida);

                util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

            }

            total_offset++;
        }
        //printf("\n");
    }

    //printf("\n");

    return total_offset;
}

//2,3=
//1,1
//1,2
//1,3
//2,1
//2,2
//2,3

//2,3,4=
//1,1,1,
//1,1,2
//1,1,3
//1,1,4

//1,2,1
//1,2,2
//...

//tipo: tipo maquina: 0: spectrum. 1: zx80. 2: zx81
void debug_view_basic_variables(char *results_buffer,int maxima_longitud_texto)
{

    //Cadena vacia inicialmente
    results_buffer[0]=0;


    z80_int vars_pointer=23627;

    if (MACHINE_IS_ZX81_TYPE) vars_pointer=16400;
    if (MACHINE_IS_ZX80_TYPE) vars_pointer=16392;

	z80_int dir;

  	dir=peek_word_no_time(vars_pointer);
    char buffer_linea[MAX_DEBUG_BASIC_VARIABLES_LINE_LENGTH+1];

    sprintf(buffer_linea,"VARS(%d)=%d\n\n",vars_pointer,dir);

    util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

    //int index_buffer=0;

    z80_byte letra_variable;

    //z80_byte number_type;

    int salir=0;
    int i;

    int resultado=0;

    z80_int longitud_variable;

    int dimensiones[256];
    int posicion_actual[256];

    int inicio_texto;

    char buf_numero[100];


    z80_byte total_dimensiones;


    z80_byte id_variable_alfanum=2;
    z80_byte id_variable_num=3;
    z80_byte id_matriz_num=4;
    z80_byte id_variable_num_mascar=5;
    z80_byte id_matriz_alfanum=6;
    z80_byte id_variable_fornext=7;

    /*
    Nota: funciones (DEF FN) no se definen como variables, solo estan definidas en la linea en la que estan escritas
    Asi cuando invocamos a una función (por ejemplo PRINT FN x(3)), el interprete Basic recorre todas las lineas
    desde el principio hasta que encuentra la DEF FN correspondiente
    */

    if (MACHINE_IS_ZX80_TYPE) {
        id_variable_alfanum=4;
        id_matriz_num=5;
        id_variable_num_mascar=2;
        //no hay matrices alfanum en zx80??
    }



  	while (peek_byte_no_time(dir)!=128 && !salir) {
        z80_int dir_antes=dir;
        sprintf (buffer_linea,"%d: ",dir);
        util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

        z80_byte first_byte=peek_byte_no_time(dir++);
        z80_byte first_byte_letter=first_byte & 31;
        z80_byte variable_type=((first_byte>>5))&7;

        //printf("dir: %d %d\n",dir,variable_type);



            if (variable_type==id_variable_alfanum) {
                //Variable alfanumerica (p ej A$)
                letra_variable=debug_view_basic_variables_letra_variable(first_byte_letter);

                sprintf (buffer_linea,"%c$=",letra_variable);
                util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

                //En zx80, no indica longitud sino que acaba con byte 1
                if (MACHINE_IS_ZX80_TYPE) {
                    util_concat_string(results_buffer,"\"",maxima_longitud_texto);
                    while (peek_byte_no_time(dir)!=1) {
                        z80_byte letra=peek_byte_no_time(dir++);
                        letra=debug_view_basic_variables_getchar(letra);

                        sprintf (buffer_linea,"%c",letra);
                        util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);
                    }

                    util_concat_string(results_buffer,"\"\n",maxima_longitud_texto);
                    dir++;

                }

                else {
                    longitud_variable=peek_word_no_time(dir);

                    dir +=2;


                    resultado=debug_view_basic_variables_print_string(dir,longitud_variable,results_buffer,maxima_longitud_texto);



                    //Siguiente variable
                    dir +=longitud_variable;
                }


            }

            else if (variable_type==id_variable_num) {
                //Variable numérica identificada con un solo caracter
                letra_variable=debug_view_basic_variables_letra_variable(first_byte_letter);


                debug_view_basic_variables_print_number(dir,buf_numero);

                sprintf(buffer_linea,"%c=%s\n",letra_variable,buf_numero);

                if (MACHINE_IS_ZX80_TYPE) dir+=2;
                else dir +=5;

                resultado=util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);


            }

            else if (variable_type==id_matriz_num || variable_type==id_matriz_alfanum) {
                //Matriz numérica(4) o alfanumerica(6)
                letra_variable=debug_view_basic_variables_letra_variable(first_byte_letter);
                //if (letra_variable<'a' || letra_variable>'z') letra_variable='?';

                if (variable_type==id_matriz_num) {
                    sprintf (buffer_linea,"DIM %c(",letra_variable);
                }
                else {
                    sprintf (buffer_linea,"DIM %c$(",letra_variable);
                }
                util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

                //hacerla de 64 bits asi podemos controlar tranquilamente cuando nos pasamos del rango admitido de 64kb
                z80_64bit total_tamanyo;

                //En zx80 solo tienen una dimension
                if (MACHINE_IS_ZX80_TYPE) {
                    total_dimensiones=1;

                    z80_byte dimension;

                    //En zx80, un dim a(0), indica 1 de tamanyo, y el primer dato esta en a(0)
                    if (variable_type==id_matriz_num) {
                        dimension=1+peek_byte_no_time(dir++);
                        total_tamanyo=dimension;
                        dimensiones[0]=dimension;
                        //2 ceros mas
                        //dir +=2;
                        inicio_texto=dir;
                    }

                    if (variable_type==id_matriz_alfanum) {
                        dimension=1+peek_byte_no_time(dir++);
                        total_tamanyo=dimension;
                        dimensiones[0]=dimension;
                        //2 ceros mas
                        //dir +=2;
                        inicio_texto=dir;
                    }

                    sprintf (buffer_linea,"%d)",dimension);
                    util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);
                    longitud_variable=dimension*2;
                }
                else {
                    longitud_variable=peek_word_no_time(dir);

                    dir +=2;


                    total_dimensiones=peek_byte_no_time(dir);
                    //printf("total_dimensiones: %d\n",total_dimensiones);



                    total_tamanyo=1;
                    int salir_calculo_dimensiones=0;

                    for (i=0;i<total_dimensiones && !salir_calculo_dimensiones;i++) {
                        z80_int dimension=peek_word_no_time(dir+1+(i*2));
                        dimensiones[i]=dimension;

                        if (dimension>0 && total_tamanyo<65536) {
                            total_tamanyo*=dimension;
                            //printf("tama %d\n",total_tamanyo);
                        }

                        //demasiado grande? salimos sin calcular todo tamaño
                        if (total_tamanyo>65535) salir_calculo_dimensiones=1;

                        char relleno=(i<total_dimensiones-1 ? ',' : ')');
                        sprintf (buffer_linea,"%d%c",dimension,relleno);
                        util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);
                    }
                    //Y ahora imprimir cadenas de texto / valores numericos
                    inicio_texto=dir+1+(i*2);
                }



                sprintf (buffer_linea,"=\n");
                util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);




                int es_numerico=0;
                if (variable_type==id_matriz_num) es_numerico=1;

                //printf("total:%d\n",total_tamanyo);
                //sleep(5);

                if (total_tamanyo>65535) {
                    debug_printf(VERBOSE_ERR,"Array exceeds 64k (%lld)",total_tamanyo);
                    salir=1;
                }

                else {
                    //printf("total_tamanyo: %lld\n",total_tamanyo);
                    debug_view_basic_variables_print_dim_alpha(results_buffer,inicio_texto,total_dimensiones,&dimensiones[0],
                        0,&posicion_actual[0],0,maxima_longitud_texto,es_numerico);

                }

                //Siguiente variable
                dir +=longitud_variable;

                resultado=util_concat_string(results_buffer,"\n",maxima_longitud_texto);


            }



            else if (variable_type==id_variable_num_mascar) {
                //Variable numérica identificada de mas de un solo caracter
                letra_variable=debug_view_basic_variables_letra_variable(first_byte_letter);

                char buf_nombre_variable[256];
                buf_nombre_variable[0]=letra_variable;

                int indice_nombre=1;

                //dir++;

                int fin_nombre=0;

                while (!fin_nombre) {
                    letra_variable=peek_byte_no_time(dir++);
                    if (letra_variable&128) {
                        fin_nombre=1;
                        letra_variable &=127;
                    }
                    letra_variable=debug_view_basic_variables_getchar(letra_variable);
                    //if (letra_variable<32 || letra_variable>126) letra_variable='?';

                    buf_nombre_variable[indice_nombre++]=letra_variable;

                }

                buf_nombre_variable[indice_nombre]=0;


                debug_view_basic_variables_print_number(dir,buf_numero);

                sprintf(buffer_linea,"%s=%s\n",buf_nombre_variable,buf_numero);

                if (MACHINE_IS_ZX80_TYPE) dir+=2;
                else dir +=5;

                resultado=util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);


            }

            else if (variable_type==id_variable_fornext) {
                //Iterador en bloque FOR/NEXT
                letra_variable=debug_view_basic_variables_letra_variable(first_byte_letter);

                int tamanyo_numero=5;

                if (MACHINE_IS_ZX80_TYPE) tamanyo_numero=2;

                //Inicio, final, step, linea, sentencia
                char buf_inicio[100];
                debug_view_basic_variables_print_number(dir,buf_inicio);
                dir +=tamanyo_numero;

                char buf_final[100];
                debug_view_basic_variables_print_number(dir,buf_final);
                dir +=tamanyo_numero;

                char buf_step[100];
                if (!MACHINE_IS_ZX80_TYPE) {
                    debug_view_basic_variables_print_number(dir,buf_step);
                    dir +=tamanyo_numero;
                }

                z80_int linea=peek_word_no_time(dir);
                dir+=2;

                if (MACHINE_IS_ZX8081) {
                    //printf("FOR %c=%s TO %s STEP %s LINE %d\n",letra_variable,buf_inicio,buf_final,buf_step,linea);
                    if (MACHINE_IS_ZX80_TYPE) {
                        sprintf(buffer_linea,"FOR %c=%s TO %s LINE %d\n",letra_variable,buf_inicio,buf_final,linea);
                    }
                    else sprintf(buffer_linea,"FOR %c=%s TO %s STEP %s LINE %d\n",letra_variable,buf_inicio,buf_final,buf_step,linea);
                }
                else {
                    z80_byte sentencia=peek_byte_no_time(dir);
                    dir++;

                    //printf("FOR %c=%s TO %s STEP %s LINE %d:%d\n",letra_variable,buf_inicio,buf_final,buf_step,linea,sentencia);

                    sprintf(buffer_linea,"FOR %c=%s TO %s STEP %s LINE %d:%d\n",letra_variable,buf_inicio,buf_final,buf_step,linea,sentencia);
                }

                resultado=util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);


            }

            else {
                sprintf (buffer_linea,"Unknown variable type %d\n",variable_type);
                resultado=util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);

            }



        //controlar maximo
        if (resultado) {
            debug_printf(VERBOSE_ERR,"Reached maximum text size. Showing only allowed text");
                //forzar salir
                salir=1;
        }

        //si dir ahora es menor, es que ha "dado la vuelta" a la memoria
        if (dir<dir_antes) {
            debug_printf(VERBOSE_ERR,"Reading beyond memory limits");
            salir=1;
        }


  	}

    sprintf (buffer_linea,"%d: End variables",dir);
    util_concat_string(results_buffer,buffer_linea,maxima_longitud_texto);



}

//Para hacer peek de una direccion de todo el espacio de memoria de una maquina
//realmente no es que podamos acceder a todo la memoria asignada de esa memoria, sino que vemos la SRAM
//Especialmente util para Spectrum Next o ZXUno por ejemplo
z80_byte far_peek_byte(int dir)
{

    //Si esto entra antes de inicializar una maquina (al inicio del emulador), volver

    if (memoria_spectrum==NULL) return 0;

    if (MACHINE_IS_QL) {
        //El QL es algo especial, ya que puede direccionar toda la memoria, retornamos un peek tal cual
        //En este caso no estamos limitados a la zona de SRAM, sino que es toda la rom, ram y zona de entrada/salida de puertos
        return peek_byte_z80_moto(dir);
    }
    if (MACHINE_IS_TBBLUE) {
        //2 MB Maximo
        //ROMS, RAM... Esto es el espacio de memoria lineal tal cual de Spectrum Next. la SRAM
        int maximo=TBBLUE_TOTAL_RAM-1;

        dir &=maximo;
        return memoria_spectrum[dir];
	}

    if (MACHINE_IS_ZXUNO) {
        //512 KB SRAM
        int maximo=(512*1024)-1;

        dir &=maximo;

        //saltamos los 16kb de boot rom de nuestra asignacion de ram

        return memoria_spectrum[16384+dir];
    }

    if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
        //Aunque sé que la ram asignada en estos casos empieza siempre en ram_mem_table[0] y podria
        //ir desde ahi y sumarle simplemente dir, voy a hacer lo mas estandar y sacar el banco y el offset en el banco
        //para obtener la dirección, por si por alguna extraña razon, los bancos de memoria no estuvieran ordenados desde el 0 en adelante
        //(que actualmente lo están, pero si en un futuro me da por cambiarlo...)
        //Nota para mi yo del futuro: seguro que no lo cambiaré esto, pero queda mejor así

        //Limitamos a 128kb
        int maximo=(128*1024)-1;

        dir &=maximo;

        int ram_bank;
        z80_byte *puntero;
        ram_bank=dir / 16384;

        dir = dir & 16383;
        puntero=ram_mem_table[ram_bank]+dir;

        //printf("banco: %d dir: %d\n",ram_bank,dir);
        return *puntero;
    }

    //TODO: resto de maquinas con mas de 64 kb: TSConf, etc

	//Cualquier otra cosa, un peek normal
    return peek_byte_z80_moto(dir);
}



struct s_machine_family_names family_names[]={
    {MACHINE_FAMILY_SPECTRUM,"Spectrum"}, //Este siempre debe ser el primero
    {MACHINE_FAMILY_ZX80,"ZX80"},
    {MACHINE_FAMILY_ZX81,"ZX81"},
    {MACHINE_FAMILY_COLECO,"ColecoVision"},
    {MACHINE_FAMILY_SG1000,"SG1000"},
    {MACHINE_FAMILY_SVI,"Spectravideo"},
    {MACHINE_FAMILY_SMS,"Master System"},
    {MACHINE_FAMILY_MSX,"MSX"},
    {MACHINE_FAMILY_ACE,"Jupiter Ace"},
    {MACHINE_FAMILY_Z88,"Z88"},
    {MACHINE_FAMILY_CPC,"CPC"},
    {MACHINE_FAMILY_PCW,"PCW"},
    {MACHINE_FAMILY_QL,"QL"},
    {MACHINE_FAMILY_MK14,"MK14"},

    {MACHINE_FAMILY_EOF,""}
};

struct s_machine_family machine_family_list[]={
    {MACHINE_ID_ZX80,MACHINE_FAMILY_ZX80},
    {MACHINE_ID_MICRODIGITAL_TK80,MACHINE_FAMILY_ZX80},
    {MACHINE_ID_MICRODIGITAL_TK82,MACHINE_FAMILY_ZX80},

    {MACHINE_ID_ZX81,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_TIMEX_TS1000,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_TIMEX_TS1500,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_MICRODIGITAL_TK82C,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_MICRODIGITAL_TK83,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_MICRODIGITAL_TK85,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_MICRODIGITAL_TK85,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_CZ_1000,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_CZ_1500,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_CZ_1000_PLUS,MACHINE_FAMILY_ZX81},
    {MACHINE_ID_CZ_1500_PLUS,MACHINE_FAMILY_ZX81},

    {MACHINE_ID_COLECO,MACHINE_FAMILY_COLECO},
    {MACHINE_ID_SG1000,MACHINE_FAMILY_SG1000},
    {MACHINE_ID_SVI_318,MACHINE_FAMILY_SVI},
    {MACHINE_ID_SVI_328,MACHINE_FAMILY_SVI},
    {MACHINE_ID_SMS,MACHINE_FAMILY_SMS},
    {MACHINE_ID_MSX1,MACHINE_FAMILY_MSX},
    {MACHINE_ID_ACE,MACHINE_FAMILY_ACE},
    {MACHINE_ID_Z88,MACHINE_FAMILY_Z88},
    {MACHINE_ID_CPC_464,MACHINE_FAMILY_CPC},
    {MACHINE_ID_CPC_4128,MACHINE_FAMILY_CPC},
    {MACHINE_ID_CPC_664,MACHINE_FAMILY_CPC},
    {MACHINE_ID_CPC_6128,MACHINE_FAMILY_CPC},
    {MACHINE_ID_PCW_8256,MACHINE_FAMILY_PCW},
    {MACHINE_ID_PCW_8512,MACHINE_FAMILY_PCW},
    {MACHINE_ID_QL_STANDARD,MACHINE_FAMILY_QL},
    {MACHINE_ID_MK14_STANDARD,MACHINE_FAMILY_MK14},

    {0,MACHINE_FAMILY_EOF}

};


//Buscar el texto del nombre de familia. Si no encontrado, retorna siempre "Spectrum"
char *debug_machine_info_family_get_family(enum machine_families_list family_id)
{
    int i=0;

    while (family_names[i].family_id!=MACHINE_FAMILY_EOF) {
        if (family_names[i].family_id==family_id) {
            return family_names[i].family_name;
        }
        i++;
    }

    //Por defecto Spectrum
    return family_names[0].family_name;
}


//Retorna en familia nombre de maquina
char *debug_machine_info_family(int machine_id)
{
    int i=0;

    while (machine_family_list[i].family_id!=MACHINE_FAMILY_EOF) {
        if (machine_family_list[i].machine_id==machine_id) {
            return debug_machine_info_family_get_family(machine_family_list[i].family_id);
        }
        i++;
    }

    //Por defecto
    return debug_machine_info_family_get_family(MACHINE_FAMILY_SPECTRUM);
}


//Rutinas de timesensors. Agrega un define TIMESENSORS_ENABLED en compileoptions.h para activarlo y lanza make
#ifdef TIMESENSORS_ENABLED

#include "timer.h"

struct s_timesensor_entry timesensors_array[MAX_TIMESENSORS];

int timesensors_started=0;

void timesensor_call_pre(enum timesensor_id id)
{
	if (!timesensors_started) return;

	timer_stats_current_time(&timesensors_array[id].tiempo_antes);
}

void timesensor_call_post(enum timesensor_id id)
{

	if (!timesensors_started) return;


	long diferencia=timer_stats_diference_time(&timesensors_array[id].tiempo_antes,&timesensors_array[id].tiempo_despues);


	//Y agregar

	int indice=timesensors_array[id].index_metrics;

	if (indice<MAX_TIMESENSORS_METRICS) {
		timesensors_array[id].metrics[indice]=diferencia;

		timesensors_array[id].index_metrics++;

		printf ("Agregando metrics valor: %ld\n",diferencia);
	}
}

long timesensor_call_mediatime(enum timesensor_id id)
{

	long acumulado=0;
	int total=timesensors_array[id].index_metrics;

	int i;

	printf ("Calculando la media para id. %d total: %d\n",id,total);

	if (total==0) return 0;

	//Sumar todos
	for (i=0;i<total;i++) {
		printf ("Sumando %ld\n",timesensors_array[id].metrics[i]);
		acumulado +=timesensors_array[id].metrics[i];
	}

	printf ("suma. %ld\n",acumulado);

	//y dividir
	acumulado /=total;

	printf ("total. %ld\n",acumulado);

	return acumulado;
}


long timesensor_call_maxtime(enum timesensor_id id)
{

	long maximo=0;
	int total=timesensors_array[id].index_metrics;

	int i;

	printf ("Calculando maximo para id. %d total: %d\n",id,total);

	for (i=0;i<total;i++) {
		long actual=timesensors_array[id].metrics[i];;
		if (actual>maximo) maximo=actual;
	}

	printf ("maximo. %ld\n",maximo);



	return maximo;
}

void timesensor_call_init(void)
{
	int i;

	for (i=0;i<MAX_TIMESENSORS;i++) {
		timesensors_array[i].index_metrics=0;
	}
}


#endif
