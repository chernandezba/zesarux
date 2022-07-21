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

/*
   Main and initial ZEsarUX functions
*/


#include <stdlib.h>
#include <stdio.h>
#ifndef MINGW
	#include <unistd.h>
#endif
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <signal.h>

#ifdef MINGW
	//Para llamar a FreeConsole
	#include <windows.h>
#endif


#if defined(__APPLE__)
	//Para _NSGetExecutablePath
	#include <mach-o/dyld.h>
#endif


#include "cpu.h"
#include "scrnull.h"
#include "operaciones.h"
#include "debug.h"
#include "compileoptions.h"
#include "tape.h"
#include "tape_tap.h"
#include "tape_tzx.h"
#include "tape_smp.h"
#include "audio.h"
#include "screen.h"
#include "ay38912.h"
#include "mem128.h"
#include "zx8081.h"
#include "snap.h"
#include "zxvision.h"
#include "menu_debug_cpu.h"
#include "core_spectrum.h"
#include "core_zx8081.h"
#include "timer.h"
#include "contend.h"
#include "utils.h"
#include "utils_text_adventure.h"
#include "ula.h"
#include "printers.h"
#include "joystick.h"
#include "realjoystick.h"
#include "z88.h"
#include "ulaplus.h"
#include "zxuno.h"
#include "chardetect.h"
#include "textspeech.h"
#include "mmc.h"
#include "ide.h"
#include "divmmc.h"
#include "divide.h"
#include "diviface.h"
#include "zxpand.h"
#include "spectra.h"
#include "spritechip.h"
#include "jupiterace.h"
#include "timex.h"
#include "chloe.h"
#include "prism.h"
#include "cpc.h"
#include "sam.h"
#include "atomlite.h"
#include "if1.h"
#include "tbblue.h"
#include "dandanator.h"
#include "superupgrade.h"
#include "ql.h"
#include "m68k.h"
#include "remote.h"
#include "snap_rzx.h"
#include "multiface.h"
#include "chrome.h"
#include "tsconf.h"
#include "scmp.h"
#include "mk14.h"
#include "esxdos_handler.h"
#include "kartusho.h"
#include "ifrom.h"
#include "betadisk.h"
#include "codetests.h"
#include "pd765.h"
#include "core_reduced_spectrum.h"
#include "baseconf.h"
#include "settings.h"
#include "datagear.h"
#include "network.h"
#include "stats.h"
#include "zeng.h"
#include "hilow.h"
#include "ds1307.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "sn76489an.h"
#include "vdp_9918a.h"
#include "svi.h"
#include "ql_qdos_handler.h"
#include "ql_i8049.h"
#include "samram.h"
#include "snap_ram.h"
#include "menu_items.h"
#include "charset.h"
#include "menu_filesel.h"

#ifdef COMPILE_STDOUT
#include "scrstdout.h"
#endif


#ifdef COMPILE_SIMPLETEXT
#include "scrsimpletext.h"
#endif


#ifdef COMPILE_CURSES
#include "scrcurses.h"
#endif

#ifdef COMPILE_AA
#include "scraa.h"
#endif

#ifdef COMPILE_CACA
#include "scrcaca.h"
#endif


#ifdef USE_COCOA
#include "scrcocoa.h"
#endif


#ifdef COMPILE_XWINDOWS
#include "scrxwindows.h"
#endif

#ifdef COMPILE_SDL

	#ifdef COMPILE_SDL2
		#include "scrsdl2.h"
	#else
		#include "scrsdl.h"
	#endif
#endif


#ifdef COMPILE_FBDEV
#include "scrfbdev.h"
#endif


#ifdef COMPILE_DSP
#include "audiodsp.h"
#endif

#ifdef COMPILE_ONEBITSPEAKER
#include "audioonebitspeaker.h"
#endif

#ifdef COMPILE_SDL
#include "audiosdl.h"
#endif


#ifdef COMPILE_ALSA
#include "audioalsa.h"
#endif

#ifdef COMPILE_PULSE
#include "audiopulse.h"
#endif


#ifdef COMPILE_COREAUDIO
#include "audiocoreaudio.h"
#endif


#include "audionull.h"

#ifdef USE_PTHREADS
#include <pthread.h>

pthread_t thread_main_loop;

#endif



#include "autoselectoptions.h"



char *romfilename;

//Maquina actual
z80_byte current_machine_type;

//Ultima maquina seleccionada desde post_set_machine
z80_byte last_machine_type=255;

//Ultimo ancho y alto de ventana desde post_set_machine_no_rom_load_reopen_window
int last_ancho_ventana=99999;
int last_alto_ventana=99999;


//Tipos de CPU Z80 activa
enum z80_cpu_types z80_cpu_current_type=Z80_TYPE_GENERIC;


char *z80_cpu_types_strings[TOTAL_Z80_CPU_TYPES]={
	"Generic",
	"Mostek",
	"CMOS"
};


char *scrfile;

int zoom_x=2,zoom_y=2;
int zoom_x_original,zoom_y_original;

struct timeval z80_interrupts_timer_antes, z80_interrupts_timer_ahora;

struct timeval zesarux_start_time;

long z80_timer_difftime, z80_timer_seconds, z80_timer_useconds;

//Indica si se tiene que probar dispositivos. Solo se hace cuando no se especifica uno concreto por linea de comandos
z80_bit try_fallback_video;
z80_bit try_fallback_audio;

//punteros a funciones de inicio para hacer fallback de video, funciones de set
driver_struct scr_driver_array[MAX_SCR_INIT];
int num_scr_driver_array=0;

//punteros a funciones de inicio para hacer fallback de audio, funciones de set
driver_struct audio_driver_array[MAX_AUDIO_INIT];
int num_audio_driver_array=0;

//Los inicializamos a cadena vacia... No poner NULL, dado que hay varios strcmp que se comparan contra esto
char *driver_screen="";
char *driver_audio="";


//porcentaje de velocidad de la cpu
int porcentaje_velocidad_emulador=100;

int anterior_porcentaje_velocidad_emulador=100;


//parametro pasado por linea de comandos
//int initial_porcentaje_velocidad_emulador=100;




//linea actual (scanline) segun cuantas interrupciones maskables se han generado
//en modo slow del zx81 hay un "offset" para que se ajuste a la pantalla
int video_zx8081_linecntr=0;

z80_bit video_zx8081_linecntr_enabled;

//se supone que es el siguiente valor que tendra una linea entera segun la ULA, util para emular fast/slow
z80_byte video_zx8081_ula_video_output;

//simular franjas de carga y sonido. De momento solo para ZX80/81
z80_bit tape_loading_simulate;

//franjas de carga y sonido a velocidad real o rapido
z80_bit tape_loading_simulate_fast;

//Autoseleccionar opciones de emulacion (audio, realvideo, etc) segun snap o cinta cargada
z80_bit autoselect_snaptape_options;



int argc;
char **argv;
int puntero_parametro;



z80_bit border_enabled;

//contador que indica cuantos frames de pantalla entero se han enviado, para contar cuando enviamos el sonido
int contador_frames_veces_buffer_audio=0;

int final_nombre;


//T-estados totales del frame
int t_estados=0;

//t-estados parcial. Usados solo en debug
unsigned int debug_t_estados_parcial=0;

//Scan line actual. Este siempre indica la linea actual dentro del frame total. No alterable por vsync
int t_scanline=0;

//Scan line actual para dibujar en pantalla. En ZX spectrum siempre va adelante. En ZX80/81 lo altera el vsync
int t_scanline_draw=0;
//Linea actual desde el ultimo vsync para zx80/81. util para controlar timeout y forzar vsync
int t_scanline_draw_timeout=0;


//Si soporte de snow effect esta activo o no
z80_bit snow_effect_enabled;

z80_bit test_config_and_exit={0};


//SDL : Si se lee teclado mediante scancodes raw en vez de usar localizacion de teclado
z80_bit sdl_raw_keyboard_read={0};

//Si se usa core de spectrum reducido o no
#ifdef USE_REDUCED_CORE_SPECTRUM
z80_bit core_spectrum_uses_reduced={1};
#else
z80_bit core_spectrum_uses_reduced={0};
#endif


void add_scr_init_array(char *name,int (*funcion_init) () , int (*funcion_set) () )
{
	if (num_scr_driver_array==MAX_SCR_INIT) {
                cpu_panic("Error. Maximum number of screen drivers");
	}


	strcpy(scr_driver_array[num_scr_driver_array].driver_name,name);
	scr_driver_array[num_scr_driver_array].funcion_init=funcion_init;
	scr_driver_array[num_scr_driver_array].funcion_set=funcion_set;

	num_scr_driver_array++;
}


void add_audio_init_array(char *name,int (*funcion_init) () , int (*funcion_set) () )
{
        if (num_audio_driver_array==MAX_AUDIO_INIT) {
                cpu_panic("Error. Maximum number of audio drivers");
        }


        strcpy(audio_driver_array[num_audio_driver_array].driver_name,name);
        audio_driver_array[num_audio_driver_array].funcion_init=funcion_init;
        audio_driver_array[num_audio_driver_array].funcion_set=funcion_set;

        num_audio_driver_array++;
}


void do_fallback_video(void)
{
	debug_printf(VERBOSE_INFO,"Guessing video driver");

	int i;

	for (i=0;i<num_scr_driver_array;i++) {

		screen_reset_scr_driver_params();

		int (*funcion_init) ();
		int (*funcion_set) ();

		funcion_init=scr_driver_array[i].funcion_init;
		funcion_set=scr_driver_array[i].funcion_set;
		if ( (funcion_init()) ==0) {
			debug_printf(VERBOSE_DEBUG,"Ok video driver i:%d %s",i,scr_new_driver_name);
			funcion_set();
			return;
		}

		debug_printf(VERBOSE_INFO,"Fallback to next video driver");
	}


	printf ("No valid video driver found\n");
	exit(1);
}

void do_fallback_audio(void)
{
        debug_printf(VERBOSE_INFO,"Guessing audio driver");

        int i;

        for (i=0;i<num_audio_driver_array;i++) {
                int (*funcion_init) ();
                int (*funcion_set) ();

                funcion_init=audio_driver_array[i].funcion_init;
                funcion_set=audio_driver_array[i].funcion_set;
                if ( (funcion_init()) ==0) {
                        debug_printf (VERBOSE_DEBUG,"Ok audio driver i:%d %s",i,audio_new_driver_name);
                        funcion_set();
                        return;
                }

		debug_printf(VERBOSE_INFO,"Fallback to next audio driver");
        }


        printf ("No valid audio driver found\n");
        exit(1);
}


int siguiente_parametro(void)
{
	argc--;
	if (argc==0) {
		//printf ("Error sintaxis\n");
		return 1;
	}
	puntero_parametro++;
	return 0;
}

void siguiente_parametro_argumento(void)
{
	if (siguiente_parametro()) {
		printf ("Syntax error. Parameter %s missing value\n",argv[puntero_parametro]);
		exit(1);
	}
}

z80_registro registro_hl;

z80_registro registro_de;

z80_registro registro_bc;

z80_byte reg_h_shadow,reg_l_shadow;
z80_byte reg_b_shadow,reg_c_shadow;
z80_byte reg_d_shadow,reg_e_shadow;
z80_byte reg_a_shadow;


z80_byte reg_i;
z80_byte reg_r,reg_r_bit7;

//usado en zx80/81
z80_byte reg_r_antes_zx8081;

z80_int reg_pc;
z80_byte reg_a;
z80_int reg_sp;
z80_int reg_ix;
z80_int reg_iy;

//Nueva gestion de flags
z80_byte Z80_FLAGS;
z80_byte Z80_FLAGS_SHADOW;

//MEMPTR. Solo se usara si se ha activado en el configure
z80_int memptr;

//Solo se usa si se habilita EMULATE_SCF_CCF_UNDOC_FLAGS
z80_byte scf_ccf_undoc_flags_before;
int scf_ccf_undoc_flags_after_changed;

int z80_ejecutada_instruccion_bloque_ld_cp=0;
int z80_ejecutada_instruccion_bloque_ot_in=0;
z80_byte z80_last_data_transferred_ot_in;

//Emulacion de refresco de memoria.
int machine_emulate_memory_refresh=0;
int machine_emulate_memory_refresh_counter=0;

//Optimizacion de velocidad de LDIR,LDDR:
z80_bit cpu_ldir_lddr_hack_optimized={0};

//A 0 si interrupts disabled
//A 1 si interrupts enabled
//z80_bit interrupts;
z80_bit iff1;
/*
These flip flops are simultaneously set or reset by the EI and DI instructions. IFF1 determines whether interrupts are allowed, but its value cannot be read. The value of IFF2 is copied to the P/V flag by LD A,I and LD A,R. When an NMI occurs, IFF1 is reset, thereby disallowing further [maskable] interrupts, but IFF2 is left unchanged. This enables the NMI service routine to check whether the interrupted program had enabled or disabled maskable interrupts. So, Spectrum snapshot software can only read IFF2, but most emulators will emulate both, and then the one that matters most is IFF1.
*/

z80_bit iff2;

z80_byte im_mode=0;
z80_bit cpu_step_mode;

//border se ha modificado
z80_bit modificado_border;

//Si se genera valor random para cada cold boot en el registro R
z80_bit cpu_random_r_register={0};


z80_bit zxmmc_emulation={0};

//Inves. Poke ROM. valor por defecto
//z80_byte valor_poke_rom=255;


//Decir si hay que volver a hacer fetch en el core, esto pasa con instrucciones FD FD FD ... por ejemplo
int core_refetch=0;


//Decir que ha llegado a final de frame pantalla y tiene que revisar si enviar y recibir snapshots de ZRCP y ZENG
z80_bit core_end_frame_check_zrcp_zeng_snap={0};

//en spectrum, 32. en pentagon, 36
int cpu_duracion_pulso_interrupcion=32;


//Inves. Ultimo valor hecho poke a RAM baja (0...16383) desde menu
z80_byte last_inves_low_ram_poke_menu=255;

z80_bit inves_ula_bright_error={1};

z80_int get_im2_interrupt_vector(void)
{
    return reg_i*256+ula_databus_value;
}

//Inves. Contador ula delay. mas o menos exagerado
//maximo 1: a cada atributo
//z80_byte inves_ula_delay_factor=3;


//Tablas teclado
z80_byte puerto_65278=255; //    db    255  ; V    C    X    Z    Sh    ;0
z80_byte puerto_65022=255; //    db    255  ; G    F    D    S    A     ;1
z80_byte puerto_64510=255; //    db              255  ; T    R    E    W    Q     ;2
z80_byte puerto_63486=255; //    db              255  ; 5    4    3    2    1     ;3
z80_byte puerto_61438=255; //    db              255  ; 6    7    8    9    0     ;4
z80_byte puerto_57342=255; //    db              255  ; Y    U    I    O    P     ;5
z80_byte puerto_49150=255; //    db              255  ; H                J         K      L    Enter ;6
z80_byte puerto_32766=255; //    db              255  ; B    N    M    Simb Space ;7

//puertos especiales no presentes en spectrum
z80_byte puerto_especial1=255; //   ;  .  .  PgDn  PgUp ESC ;
z80_byte puerto_especial2=255; //   F5 F4 F3 F2 F1
z80_byte puerto_especial3=255; //  F10 F9 F8 F7 F6
z80_byte puerto_especial4=255; //  F15 F14 F13 F12 F11

z80_bit chloe_keyboard={0};



//se ha llegado al final de instrucciones en un frame de pantalla y esperamos
//a que se genere una interrupcion de 1/50s
z80_bit esperando_tiempo_final_t_estados;

//se ha generado interrupcion maskable de la cpu
//en spectrum pasa con el timer
//en zx80/zx81 pasa con el cambio de bit 6 en reg_r
z80_bit interrupcion_maskable_generada;


//se ha generado interrupcion non maskable de la cpu.
//pasa en zx81 cada 64 microsegundos y si el nmi generator esta activo
z80_bit interrupcion_non_maskable_generada;


//se ha generado interrupcion de timer 1/50s
z80_bit interrupcion_timer_generada;


z80_bit z80_halt_signal;


z80_byte *memoria_spectrum;


z80_bit quickload_inicial;
char *quickload_nombre;

z80_bit z88_slotcard_inicial;
char *z88_slotcard_inicial_nombre;
int z88_slotcard_inicial_slot;

//Si no cambiamos parametros de frameskip y otros cuando es maquina lenta (raspberry, cocoa mac os x, etc)
z80_bit no_cambio_parametros_maquinas_lentas={0};



//Indica si se repinta automaticamente la pantalla
z80_bit stdout_simpletext_automatic_redraw={0};




//fin valores para stdout


z80_bit windows_no_disable_console={0};


//Si salir rapido del emulador
z80_bit quickexit={0};


//Si soporte azerty para xwindows
z80_bit azerty_keyboard={0};


z80_int ramtop_ace;

//Permitir escritura en ROM
z80_bit allow_write_rom={0};

//0=default
//1=spanish
int z88_cpc_keymap_type=0;

char *realmachine_keymap_strings_types[]={
	"Default",
	"Spanish"
};

//Modo turbo. 1=normal. 2=7 Mhz, 3=14 Mhz, etc
int cpu_turbo_speed=1;


char dump_ram_file[PATH_MAX]="";


//Si se sale del emulador despues de X segundos. A 0 para desactivarlo
int exit_emulator_after_seconds=0;

int exit_emulator_after_seconds_counter=0;


char last_version_string[255]="";


z80_bit do_no_show_changelog_when_update={0};

int cpu_turbo_speed_antes=1;


z80_bit set_machine_empties_audio_buffer={1};

//Texto indicado en el parametro
char parameter_disablebetawarning[100]="";


//parametros de estadisticas
int total_minutes_use=0;


//Aqui solo se llama posteriormente a haber inicializado la maquina, nunca antes
void cpu_set_turbo_speed(void)
{


	debug_printf (VERBOSE_INFO,"Changing turbo mode from %dX to %dX",cpu_turbo_speed_antes,cpu_turbo_speed);

	//Ajustes previos de t_estados. En estos ajustes, solo las variables t_estados, antes_t_estados se usan. Las t_estados_en_linea, t_estados_percx son para debug
	//printf ("Turbo was %d, setting turbo %d\n",cpu_turbo_speed_antes,cpu_turbo_speed);

	//int t_estados_en_linea=t_estados % screen_testados_linea;

	//int t_estados_percx=(t_estados_en_linea*100)/screen_testados_linea;

	//printf ("Before changing turbo, t-scanline: %d, t-states: %d, t-states in line: %d, percentaje column: %d%%\n",t_scanline,t_estados,t_estados_en_linea,t_estados_percx);

	int antes_t_estados=t_estados / cpu_turbo_speed_antes;

	//printf ("Before changing turbo, t-states at turbo 1X: %d\n",antes_t_estados);


	z80_bit antes_debug_breakpoints_enabled;
	antes_debug_breakpoints_enabled.v=debug_breakpoints_enabled.v;

	z80_bit antes_betadisk_enabled;
	antes_betadisk_enabled.v=betadisk_enabled.v;

	z80_bit antes_mutiface_enabled;
	antes_mutiface_enabled.v=multiface_enabled.v;
	
	z80_bit antes_cpu_code_coverage_enabled;
	antes_cpu_code_coverage_enabled.v=cpu_code_coverage_enabled.v;
	
	z80_bit antes_cpu_history_enabled;
	antes_cpu_history_enabled.v=cpu_history_enabled.v;
	
	z80_bit antes_extended_stack_enabled;
	antes_extended_stack_enabled.v=extended_stack_enabled.v;
	
	
	
	

	if (cpu_turbo_speed>MAX_CPU_TURBO_SPEED) {
		debug_printf (VERBOSE_INFO,"Turbo mode higher than maximum. Setting to %d",MAX_CPU_TURBO_SPEED);
		cpu_turbo_speed=MAX_CPU_TURBO_SPEED;
	}

	//Si esta divmmc/divide, volver a aplicar funciones poke
	if (diviface_enabled.v) diviface_restore_peek_poke_functions();




	//Variable turbo se sobreescribe al llamar a set_machine_params. Guardar y restaurar luego
	int speed=cpu_turbo_speed;

	set_machine_empties_audio_buffer.v=0; //para que no vacie buffer de sonido y asi sonido no se oye extraño
	set_machine_params();

	cpu_turbo_speed=speed;

	screen_testados_linea *=cpu_turbo_speed;
        screen_set_video_params_indices();
        inicializa_tabla_contend_cached_change_cpu_speed();

        //Recalcular algunos valores cacheados
        recalcular_get_total_ancho_rainbow();
        recalcular_get_total_alto_rainbow();




	//Ajustes posteriores de t_estados
	//Ajustar t_estados para que se quede en mismo "sitio"
	t_estados=antes_t_estados * cpu_turbo_speed;


	//t_estados_en_linea=t_estados % screen_testados_linea;

	//t_estados_percx=(t_estados_en_linea*100)/screen_testados_linea;

	//printf ("After changing turbo, t-states: %d, t-states in line: %d, percentaje column: %d%%\n",t_estados,t_estados_en_linea,t_estados_percx);
	//printf ("Calculated t-scanline according to t-states: %d\n",t_estados / screen_testados_linea);



        init_rainbow();
        init_cache_putpixel();

	if (diviface_enabled.v) diviface_set_peek_poke_functions();

	//Si estaba modo debug cpu, reactivar
	if (antes_debug_breakpoints_enabled.v) {
		debug_printf(VERBOSE_INFO,"Re-enabling breakpoints because they were enabled before changing turbo mode");
		debug_breakpoints_enabled.v=1;
		breakpoints_enable();
	}

	if (antes_betadisk_enabled.v) betadisk_enable();

	if (antes_mutiface_enabled.v) multiface_enable();
	
	
	
	if (antes_cpu_code_coverage_enabled.v) set_cpu_core_code_coverage_enable();
	
	
	if (antes_cpu_history_enabled.v) set_cpu_core_history_enable();
	
	
	if (antes_extended_stack_enabled.v) set_extended_stack();
	
	

	cpu_turbo_speed_antes=cpu_turbo_speed;



	/*
	Calculos de tiempo en ejecutar esta funcion de cambio de velocidad de cpu, desde metodo antiguo hasta optimizado actual:
--Metodo clasico de obtener tablas contend:

	Con O0:
cpu: X01 tiempo: 1611 us	
cpu: X08 tiempo: 4117 us

	Con O2:
cpu: X01 tiempo: 880 us	
cpu: X08 tiempo: 1031 us	



	--Con rutina contend con memset:
	Con O0:
cpu: X08 tiempo: 863 us	

	Con O2:
cpu: X08 tiempo: 539 us	

	-- Con tabla cacheada al cambiar speed:
	Con O0:
cpu: X01 tiempo: 964 us
cpu: X08 tiempo: 883 us

	Con O2:
cpu: X01 tiempo: 547 us
cpu: X08 tiempo: 438 us
	*/


}

void z80_no_ejecutado_block_opcodes(void)
{
    z80_ejecutada_instruccion_bloque_ld_cp=0;
    z80_ejecutada_instruccion_bloque_ot_in=0;    
}

void z80_adjust_flags_interrupt_block_opcode(void)
{

	//Si estabamos en una instruccion de bloque
    /*
    Basado en info de: https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
    */
    //Esto comun para ld_cp ot_in
    if (z80_ejecutada_instruccion_bloque_ld_cp || z80_ejecutada_instruccion_bloque_ot_in) {
        /*
        LDxR / CPxR interrupted

        When LDxR / CPxR is interrupted, the following flags are modified compared to the non-interrupted LDx / CPx:

        INxR / OTxR interrupted

        When INxR / OTxR is interrupted, the following flags are modified compared to the non-interrupted INx / OUTx:


        YF (flag 5)= PC.13  
        XF (flag 3)= PC.11
        */
        Z80_FLAGS &=(255-FLAG_3-FLAG_5);

        z80_byte high_pc=(reg_pc>>8);
        z80_byte final_35=high_pc & (8+32);
        Z80_FLAGS |=final_35;

    }		

    if (z80_ejecutada_instruccion_bloque_ot_in) {
        /*
        INxR / OTxR interrupted

        When INxR / OTxR is interrupted, the following flags are modified compared to the non-interrupted INx / OUTx:

        YF = PC.13
        XF = PC.11
        if (CF) {
        if (data & 0x80) {
            PF = PF ^ Parity((B - 1) & 0x7) ^ 1;
            HF = (B & 0x0F) == 0x00;
        } else {
            PF = PF ^ Parity((B + 1) & 0x7) ^ 1;
            HF = (B & 0x0F) == 0x0F;
        }
        } else {
        PF = PF ^ Parity(B & 0x7) ^ 1;
        }
        where PC is the address of the start of the instruction (i.e. the 0xED prefix)
    */


        z80_byte PF,HF;

        PF=(Z80_FLAGS & FLAG_PV ? 1 : 0);
        HF=(Z80_FLAGS & FLAG_H ? 1 : 0);

        Z80_FLAGS &=(255-FLAG_PV-FLAG_H);

        if (Z80_FLAGS & FLAG_C) {
            if (z80_last_data_transferred_ot_in & 0x80) {
                PF = PF ^ util_parity((reg_b - 1) & 0x7) ^ 1;
                HF = (reg_b & 0x0F) == 0x00;
            } else {
                PF = PF ^ util_parity((reg_b + 1) & 0x7) ^ 1;
                HF = (reg_b & 0x0F) == 0x0F;
            }
        } else {
            PF = PF ^ util_parity(reg_b & 0x7) ^ 1;
        }        

        if (PF) Z80_FLAGS |=FLAG_PV;
        if (HF) Z80_FLAGS |=FLAG_H;

    }	

}

//Establecer la mayoria de registros a valores indefinidos (a 255), que asi se establecen al arrancar una maquina
//al pulsar el boton de reset aqui no se llama
void cold_start_cpu_registers(void)
{

	//Probar RANDOMIZE USR 46578 y debe aparecer pantalla con colores y al pulsar Espacio, franjas en el borde
	//Si esto se ejecuta despues de un reset, no sucede
	//Ver http://foro.speccy.org/viewtopic.php?f=11&t=2319
	//y http://www.worldofspectrum.org/forums/discussion/comment/539714#Comment_539714

        /*
        AF=BC=DE=HL=IX=IY=SP=FFFFH
        AF'=BC'=DE'=HL'=FFFFH
        IR=0000H
        */

        reg_a=0xff;
        Z80_FLAGS=0xff;
        BC=HL=DE=0xffff;
        reg_ix=reg_iy=reg_sp=0xffff;

        reg_h_shadow=reg_l_shadow=reg_b_shadow=reg_c_shadow=reg_d_shadow=reg_e_shadow=reg_a_shadow=Z80_FLAGS_SHADOW=0xff;
        reg_i=0;
        reg_r=reg_r_bit7=0;

	if (cpu_random_r_register.v) {
		reg_r=value_16_to_8l(randomize_noise[0]) & 127;
		debug_printf (VERBOSE_DEBUG,"R Register set to random value: %02XH",reg_r);
	}

	out_254_original_value=out_254=0xff;

	//Parece que en Inves esto es asi:
	if (MACHINE_IS_INVES) {
		out_254_original_value=out_254=0;
	}

	modificado_border.v=1;

	//Modo BOOTM y cambio de otros settings del menu emulador
	if (MACHINE_IS_ZXUNO) {

		//activar BOOTM
		zxuno_ports[0]=1;

		//metemos registro SCRATCH / COLDBOOT a 0
		zxuno_ports[0xfe]=0;


        	zxuno_ports[0x0B]=0;
        	zxuno_ports[0x0C]=255;
	        zxuno_ports[0x0D]=1;
        	zxuno_ports[0x0E]=0;
	        zxuno_ports[0x0F]=0;
        	zxuno_ports[0x40]=0;



		zxuno_set_emulator_setting_i2kb();
		zxuno_set_emulator_setting_timing();
		zxuno_set_emulator_setting_contend();
		zxuno_set_emulator_setting_diven();
		zxuno_set_emulator_setting_disd();
		zxuno_set_emulator_setting_devcontrol_diay();
		zxuno_set_emulator_setting_devcontrol_ditay();
		zxuno_set_emulator_setting_scandblctrl();
		zxuno_set_emulator_setting_ditimex();
		zxuno_set_emulator_setting_diulaplus();

		//quitamos write enable de la spi flash zxuno
		zxuno_spi_clear_write_enable();

		zxuno_radasoffset_high_byte.v=0;
		zxuno_radasoffset=0;

		//radaspadding
		zxuno_ports[0x42]=0;

		//radaspalbank
		zxuno_ports[0x43]=0;

        //registro prism
        zxuno_ports[0x50]=0;

        zxuno_prism_set_default_palette();

	}

	if (MACHINE_IS_PRISM) {
		hard_reset_cpu_prism();
		prism_set_emulator_setting_cpuspeed();
	}

	if (MACHINE_IS_TBBLUE) {
		tbblue_hard_reset();
	}

	if (MACHINE_IS_TSCONF) {
		tsconf_hard_reset();
		tsconf_set_default_basic_palette();
	}

	if (MACHINE_IS_BASECONF) {
		baseconf_hard_reset();
		//baseconf_set_default_basic_palette();
	}
}


//Para maquinas Z88 y zxuno y prism
void hard_reset_cpu(void)
{
	if (MACHINE_IS_Z88) hard_reset_cpu_z88();

	else if (MACHINE_IS_ZXUNO) {
		hard_reset_cpu_zxuno();
	}

	else if (MACHINE_IS_PRISM) {
		hard_reset_cpu_prism();
		reset_cpu();
	}

	else if (MACHINE_IS_TBBLUE) {
    	tbblue_hard_reset();
		reset_cpu();
  }

	else if (superupgrade_enabled.v) {
		superupgrade_hard_reset();
		reset_cpu();
	}

	else if (MACHINE_IS_TSCONF) {
		tsconf_hard_reset();

	}

	else if (MACHINE_IS_BASECONF) {
		baseconf_hard_reset();

	}

}

void reset_cpu(void)
{

	debug_printf (VERBOSE_INFO,"Reset cpu");

	if (rzx_reproduciendo) {
		eject_rzx_file();
	}

	reg_pc=0;
	reg_i=0;

	//mapear rom 0 en modos 128k y paginas RAM normales
	puerto_32765=0;
	puerto_8189=0;

	zesarux_zxi_last_register=0;
	zesarux_zxi_registers_array[0]=0;

	zesarux_zxi_registers_array[4]=0;
	zesarux_zxi_registers_array[5]=0;


	interrupcion_maskable_generada.v=0;
	interrupcion_non_maskable_generada.v=0;
  interrupcion_timer_generada.v=0;
	iff1.v=iff2.v=0;
  im_mode=0;

	if1_rom_paged.v=0;


	//Algunos otros registros
	reg_a=0xff;
	Z80_FLAGS=0xff;
	reg_sp=0xffff;

	datagear_reset();

    diviface_reset();




	//si no se pone esto a 0, al cambiar de zx80 a zx81 suele colgarse
        z80_halt_signal.v=0;


        esperando_tiempo_final_t_estados.v=0;


	if (MACHINE_IS_ZX8081) {
		//algunos reseteos para zx80/81
		nmi_generator_active.v=0;
                hsync_generator_active.v=0;
		timeout_linea_vsync=NORMAL_TIMEOUT_LINEA_VSYNC;
		chroma81_port_7FEF=0;

		//Prueba ajuste por defecto, para intentar eliminar lnctr
		//if (MACHINE_IS_ZX80) video_zx8081_lnctr_adjust.v=0;
		//else if (MACHINE_IS_ZX81) video_zx8081_lnctr_adjust.v=1;

		//Si zxpand, habilitar overlay rom
		if (zxpand_enabled.v) {
			zxpand_overlay_rom.v=1;
			dragons_lair_hack.v=0;
		}

	}


	if (MACHINE_IS_SPECTRUM_128_P2) {
		mem_set_normal_pages_128k();
	}

	if (MACHINE_IS_SPECTRUM_P2A_P3) {
		mem_set_normal_pages_p2a();
	}

	if (MACHINE_IS_ZXUNO) {
		//mem_set_normal_pages_zxuno();

		//interrupciones raster
		zxuno_ports[0x0c]=0xff;
		zxuno_ports[0x0d]=1;

		//indice de mensaje a coreid
		zxuno_core_id_indice=0;

		zxuno_set_memory_pages();

		//Registros dma
		zxuno_index_nibble_dma_write[0]=zxuno_index_nibble_dma_write[1]=zxuno_index_nibble_dma_write[2]=zxuno_index_nibble_dma_write[3]=zxuno_index_nibble_dma_write[4]=0;
		zxuno_index_nibble_dma_read[0]=zxuno_index_nibble_dma_read[1]=zxuno_index_nibble_dma_read[2]=zxuno_index_nibble_dma_read[3]=zxuno_index_nibble_dma_read[4]=0;

		zxuno_ports[0xa0]=0;
		zxuno_ports[0xa6]=0;


		zxuno_dmareg[0][0]=zxuno_dmareg[0][1]=0;
		zxuno_dmareg[1][0]=zxuno_dmareg[1][1]=0;
		zxuno_dmareg[2][0]=zxuno_dmareg[2][1]=0;
		zxuno_dmareg[3][0]=zxuno_dmareg[3][1]=0;
		zxuno_dmareg[4][0]=zxuno_dmareg[4][1]=0;
	}

	//Modos extendidos ulaplus desactivar, sea en maquina zxuno o no
	zxuno_ports[0x40]=0;


	if (MACHINE_IS_Z88) {
		z88_set_default_memory_pages();
		z88_snooze.v=0;
		z88_coma.v=0;

		blink_tim[0] = 0x98;
		blink_tim[1] = blink_tim[2] = blink_tim[3] = blink_tim[4] = 0;

		//registros de video a 0 tambien
		blink_pixel_base[0]=blink_pixel_base[1]=blink_pixel_base[2]=blink_pixel_base[3]=0;
		blink_sbr=0;

		//resetear speaker. por si acaso se queda activo el sonido a 3200 khz
		//7           SRUN        Speaker source (0=SBIT, 1=TxD or 3200Khz
		blink_com &= (255-128);

	}

	if (MACHINE_IS_MSX) {
		msx_reset();
	}

	if (MACHINE_IS_SVI) {
		svi_reset();
	}	

	if (MACHINE_IS_COLECO) {
		coleco_reset();
	}	

	if (MACHINE_IS_SG1000) {
		sg1000_reset();
	}	

	if (MACHINE_IS_SMS) {
		sms_reset();
	}	    

	vdp_9918a_reset();	

	t_estados=0;
	t_scanline=0;
	t_scanline_draw=0;

        if (MACHINE_IS_INVES) {
		//Inves
		t_scanline_draw=screen_indice_inicio_pant;
        }

	init_chip_ay();

	init_chip_sn();

#ifdef EMULATE_CPU_STATS
util_stats_init();
#endif



	if (MACHINE_IS_SPECTRUM) {
		//Modo spectra 0
		spectra_display_mode_register=0;

		if (ulaplus_presente.v) ulaplus_set_mode(0);
		if (ulaplus_presente.v) ulaplus_set_extended_mode(0);
	}


	//Resetear modo timex
	timex_port_ff=0;

	//Resetear paginacion timex
	timex_port_f4=0;

	//Resetear puerto eff7 pentagon
	puerto_eff7=0;


	if (MACHINE_IS_CHLOE) chloe_set_memory_pages();

	if (MACHINE_IS_PRISM) {
		//Cambiar pagina rom tal cual como si pusiesemos bit de rom de puerto 32765 a 0 y el bit de 8189 tambien a 0
		//Asi por ejemplo, si desde prism vamos a maquina +2A, luego vamos a 48 Basic,
		//haciendo un reset normal, volvemos a menu +2A. Sino cambiasemos la pagina de rom asi,
		//al hacer reset volveria a 48 Basic

		//desactivado
		//porque reset desde maquina 48 (rom page 2) lo envia a prism boot
		//		prism_rom_page &=(255-3);


		prism_set_memory_pages();
	}

	if (MACHINE_IS_CHROME) {
		chrome_set_memory_pages();
	}

	if (MACHINE_IS_TSCONF) {
		tsconf_reset_cpu();
	}

	if (MACHINE_IS_BASECONF) {
		baseconf_reset_cpu();
	}

	if (MACHINE_IS_TBBLUE) {
		tbblue_reset();
		tbblue_set_memory_pages();

		//porque antes estaba el tbblue_reset aqui despues de set_memory_pages???

		//tbblue_read_port_24d5_index=0;
		ds1307_reset();
	}

	if (MACHINE_IS_TIMEX_T2068) timex_set_memory_pages();

	if (MACHINE_IS_CPC) {
        cpc_reset();
	}

	if (MACHINE_IS_SAM) {
		sam_vmpr=sam_hmpr=sam_lmpr=0;

		sam_set_memory_pages();
	}


        if (mmc_enabled.v) mmc_reset();
        if (ide_enabled.v) ide_reset();

	if (superupgrade_enabled.v) superupgrade_set_memory_pages();

	ay_player_playing.v=0;

	if (multiface_enabled.v) {
		multiface_lockout=0;
		multiface_unmap_memory();
	}

	if (MACHINE_IS_SPECTRUM && betadisk_enabled.v) betadisk_reset();

	if (MACHINE_IS_QL) {
        m68k_init();
        m68k_set_cpu_type(M68K_CPU_TYPE_68000);
        m68k_pulse_reset();

		//HWReset();
		//printf ("Reg PC QL: %08XH\n",pc);
		//sleep(2);
		ql_ipc_reset();
        ql_stop_sound();
		qltraps_init_fopen_files_array();
	}

	if (MACHINE_IS_MK14) {
		scmp_reset();
		mk14_reset();
	}


	if (esxdos_handler_enabled.v) {
		esxdos_handler_reset();
	}

	if (hilow_enabled.v) {
		hilow_reset();
	}


	//Inicializar zona memoria de debug
	debug_memory_zone_debug_reset();

}

char *string_machines_list_description=
//Ordenados por fabricante y año. Misma ordenacion en menu machine selection
							" MK14     MK14\n"

							" ZX80     ZX80\n"
							" ZX81     ZX81\n"
              " 16k      ZX Spectrum 16k\n"
              " 48k      ZX Spectrum 48k\n"
              " 48kp     ZX Spectrum+ 48k\n"
							" 128k     ZX Spectrum+ 128k\n"
							" QL       QL\n"

							" P2       ZX Spectrum +2\n"
							" P2F      ZX Spectrum +2 (French)\n"
							" P2S      ZX Spectrum +2 (Spanish)\n"
							" P2A40    ZX Spectrum +2A (ROM v4.0)\n"
							" P2A41    ZX Spectrum +2A (ROM v4.1)\n"
							" P2AS     ZX Spectrum +2A (Spanish)\n"

							" P340     ZX Spectrum +3 (ROM v4.0)\n"
							" P341     ZX Spectrum +3 (ROM v4.1)\n"
							" P3S      ZX Spectrum +3 (Spanish)\n"							

                            " TC2048   Timex TC 2048\n"
                            " TC2068   Timex TC 2068\n"
							" TS2068   Timex TS 2068\n"

							" Inves    Inves Spectrum+\n"
              " 48ks     ZX Spectrum+ 48k (Spanish)\n"
							" 128ks    ZX Spectrum+ 128k (Spanish)\n"
                     

					    " TK90X    Microdigital TK90X\n"
					    " TK90XS   Microdigital TK90X (Spanish)\n"
					    " TK95     Microdigital TK95\n"

							" Z88      Cambridge Z88\n"

							" Sam      Sam Coupe\n"

				    	" Pentagon Pentagon\n"

							" Chloe140 Chloe 140 SE\n"
							" Chloe280 Chloe 280 SE\n"

							" Chrome   Chrome\n"

							" Prism    Prism 512\n"

							" ZXUNO    ZX-Uno\n"

							" BaseConf ZX-Evolution BaseConf\n"

							" TSConf   ZX-Evolution TS-Conf\n"

							" TBBlue   ZX Spectrum Next/TBBlue\n"

					    " ACE      Jupiter Ace\n"

							" CPC464   Amstrad CPC 464\n"
							" CPC4128  Amstrad CPC 4128\n"
							
							" MSX1     MSX1\n"
							" Coleco   Colecovision\n"
							" SG1000   Sega SG1000\n"
                            " SMS      Sega Master System\n"
							" SVI318   Spectravideo SVI 318\n"
							" SVI328   Spectravideo SVI 328\n"
							;


void cpu_help(void)
{
	printf ("Usage:\n"
		"[--tape] file           Insert input standard tape file. Supported formats: Spectrum: .TAP, .TZX -- ZX80: .O, .80, .Z81 -- ZX81: .P, .81, .Z81 -- All machines: .RWA, .SMP, .WAV\n"
		"[--realtape] file       Insert input real tape file. Supported formats: Spectrum: .TAP, .TZX -- ZX80: .O, .80, .Z81 -- ZX81: .P, .81, .Z81 -- All machines: .RWA, .SMP, .WAV\n"
		"[--snap] file           Load snapshot file. Supported formats: Spectrum: .Z80, .ZX, .SP, .SNA -- ZX80: .ZX, .O, .80 -- ZX81: .ZX, .P, .81, .Z81\n"
		"[--slotcard] file       Insert Z88 EPROM/Flash file in the first slot. Supported formats: .EPR, .63, .EPROM, .FLASH\n"
		"Note: if you write a tape/snapshot/card file name without --tape, --realtape, --snap or --slotcard parameters, the emulator will try to guess file type (it's the same as SmartLoad on the menu)\n"
		"\n"
        "--slotcard-num file n   Same as --slotcard but insert card on the slot number n (1,2 or 3)\n"

		"--outtape file          Insert output standard tape file. Supported formats: Spectrum: .TAP, .TZX -- ZX80: .O -- ZX81: .P\n"

		"--zoom n                Total Zoom Factor\n"
		"--vo driver             Video output driver. Valid drivers: ");

#ifdef USE_COCOA
	printf ("cocoa ");
#endif

#ifdef COMPILE_XWINDOWS
	printf ("xwindows ");
#endif

#ifdef COMPILE_SDL
        printf ("sdl ");
#endif


#ifdef COMPILE_FBDEV
	printf ("fbdev ");
#endif


#ifdef COMPILE_CACA
        printf ("caca ");
#endif

#ifdef COMPILE_AA
        printf ("aa ");
#endif

#ifdef COMPILE_CURSES
	printf ("curses ");
#endif


#ifdef COMPILE_STDOUT
	printf ("stdout ");
#endif

#ifdef COMPILE_SIMPLETEXT
        printf ("simpletext ");
#endif


	printf ("null\n");


	printf ("--vofile file           Also output video to raw file\n");
	printf ("--vofilefps n           FPS of the output video [1|2|5|10|25|50] (default:5)\n");



	printf ("--ao driver             Audio output driver. Valid drivers: ");

#ifdef COMPILE_PULSE
        printf ("pulse ");
#endif

#ifdef COMPILE_ALSA
        printf ("alsa ");
#endif

#ifdef COMPILE_SDL
        printf ("sdl ");
#endif

#ifdef COMPILE_DSP
        printf ("dsp ");
#endif

#ifdef COMPILE_ONEBITSPEAKER
        printf ("onebitspeaker ");
#endif

#ifdef COMPILE_COREAUDIO
        printf ("coreaudio ");
#endif


        printf ("null\n");




#ifdef USE_SNDFILE
	printf ("--aofile file           Also output sound to wav or raw file\n");
#else
	printf ("--aofile file           Also output sound to raw file\n");
#endif

	printf ("--version               Get emulator version and exit. Must be the first command line setting\n");

	printf ("\n");

	printf ("--machine               Machine type: \n");

	printf ("%s",string_machines_list_description);



		printf ("\n"
		"--noconfigfile          Do not load configuration file. This parameter must be the first and it's ignored if written on config file\n"
		"--configfile f          Use the specified config file. This parameter must be the first and it's ignored if written on config file\n"
		"--experthelp            Show expert options\n"
		"\n"
		"Any command line setting shown here or on experthelp can be written on a configuration file,\n"
		"this configuration file is on your home directory with name: " DEFAULT_ZESARUX_CONFIG_FILE "\n"

		"\n"

#ifdef MINGW
		"Note: On Windows command prompt, all emulator messages are supressed (except the initial copyright message).\n"
		"To avoid this, you must specify at least one parameter on command line.\n\n"
#endif

              );

}

void cpu_help_expert(void)
{

	printf ("Expert options, in sections: \n"
		"\n"
		"\n"
		"Debugging\n"
		"---------\n"
		"\n"
		"--verbose n                         Verbose level n (0=only errors, 1=warning and errors, 2=info, warning and errors, 3=debug, 4=lots of messages)\n"
        "--disable-debug-console-win         Disable debug console window\n"
		"--verbose-always-console            Always show messages in console (using simple printf) additionally to the default video driver, interesting in some cases as curses, aa or caca video drivers\n"
		"--debugregisters                    Debug CPU Registers on text console\n"
	    "--showcompileinfo                   Show compilation information\n"
		"--debugconfigfile                   Debug parsing of configuration file (and .config files). This parameter must be the first and it's ignored if written on config file\n"
		"--testconfig                        Test configuration and exit without starting emulator\n"
		"--romfile file                      Select custom ROM file\n"
		"--loadbinary file addr len          Load binary file \"file\" at address \"addr\" with length \"len\". Set ln to 0 to load the entire file in memory\n"
		"--loadbinarypath path               Select initial Load Binary path\n"
		"--savebinarypath path               Select initial Save Binary path\n"
		"--keyboardspoolfile file            Insert spool file for keyboard presses\n"
		"--keyboardspoolfile-play            Play spool file right after starting the emulated machine\n"
        "--keyboardspoolfile-keylength n     Length of every key pressed. n is in intervals of 1/50 seconds, from 1 to 100. So, a value of 1 means 20 ms, and 100 means 2000 ms\n"
        "--keyboardspoolfile-nodelay         Do not send delay after every key press\n"

#ifdef USE_PTHREADS
		"--enable-remoteprotocol             Enable ZRCP remote protocol\n"
		"--remoteprotocol-port n             Set remote protocol port (default: 10000)\n"
        "--remoteprotocol-prompt p           Change the command prompt shown on remote protocol\n"
#endif

		"--showfiredbreakpoint n             Tells to show the breakpoint condition when it is fired. Possible values: \n"
		"                                    0: always shows the condition\n"
		"                                    1: only shows conditions that are not like PC=XXXX\n"
		"                                    2: never shows conditions\n"

#ifdef MINGW
		"--nodisableconsole                  Do not disable text output on this console. On Windows, text output is disabled unless you specify "
		"at least one parameter on command line, or this parameter on command line or on configuration file. \n"
#endif

	    "--enable-breakpoints                Enable breakpoints handling.\n"
	    "--brkp-always                       Fire a breakpoint when it is fired always, not only when the condition changes from false to true\n"
		"--show-display-debug                Shows emulated screen on every key action on debug registers menu\n"
		"--show-electron-debug               Shows TV electron position when debugging, using a coloured line\n"
        "--show-basic-address                Shows location address of every basic line on menu View Basic\n"
	    "--show-invalid-opcode               If running invalid cpu opcodes will generate a warning message\n"

);

        printf(

        "--cpu-history-max-items n           Maximum items allowed on cpu history feature. Each item uses %d bytes of memory\n",
                                    CPU_HISTORY_REGISTERS_SIZE);



printf(
		"--set-breakpoint n s                Set breakpoint with string s at position n. n must be between 1 and %d. string s must be written in \"\" if has spaces. Used normally with --enable-breakpoints\n",MAX_BREAKPOINTS_CONDITIONS
);

printf(
		"--set-breakpointaction n s          Set breakpoint action with string s at position n. n must be between 1 and %d. string s must be written in \"\" if has spaces. Used normally with --enable-breakpoints\n",MAX_BREAKPOINTS_CONDITIONS
);

printf(
		"--set-watch n s                     Set watch with string s at position n. n must be between 1 and %d. string s must be written in \"\" if has spaces\n",DEBUG_MAX_WATCHES
);

printf (
	    "--set-mem-breakpoint a n            Set memory breakpoint at address a for type n\n"
        "--load-source-code f                Load source code from file\n"
	    "--hardware-debug-ports              These ports are used to interact with ZEsarUX, for example showing a ASCII character on console, read ZEsarUX version, etc. "
		"Read file extras/docs/zesarux_zxi_registers.txt for more information\n"
	    "--hardware-debug-ports-byte-file f  Sets the file used on register HARDWARE_DEBUG_BYTE_FILE\n"
	    "--dump-ram-to-file f                Dump memory from 4000h to ffffh to a file, when exiting emulator\n"
	    "--dump-snapshot-panic               Dump .zsf snapshot when a cpu panic is fired\n"


		"\n"
		"\n"
		"CPU Settings\n"
		"------------\n"
		"\n"

		"--cpuspeed n                Set CPU speed in percentage\n"
		"--denyturbozxunoboot        Deny setting turbo mode on ZX-Uno boot\n"
		"--denyturbotbbluerom        Limit setting turbo mode on TBBlue ROM (default setting denied)\n"
        "--allowturbotbbluerom       Do not limit setting turbo mode on TBBlue ROM\n"
		"--tbblue-max-turbo-rom n    Max allowed turbo speed mode on TBBlue ROM when enabling --denyturbotbbluerom (default value: 2)\n"
		"--tbblue-fast-boot-mode     Boots tbblue directly to a 48 rom but with all the Next features enabled (except divmmc)\n"
		//no uso esto de momento "--tbblue-123b-port n        Sets the initial value for port 123b on hard reset, for tbblue-fast-boot-mode\n"
		"--random-r-register         Generate random value for R register on every cold start, instead of the normal 0 value. Useful to avoid same R register in the start of games, when they use that register as a random value\n"
		"--spectrum-reduced-core     Use Spectrum reduced core. It uses less cpu, ideal for slow devices like Raspberry Pi One and Zero\n"
		"                            The following features will NOT be available or will NOT be properly emulated when using this core:\n"
		"                            Debug t-states, Char detection, +3 Disk, Save to tape, Divide, Divmmc, RZX, Raster interrupts, TBBlue Copper, Audio DAC, Video out to file\n"
		"--no-spectrum-reduced-core  Do not use Spectrum reduced core\n"  

		"\n"
		"\n"
		"ULA Settings\n"
		"------------\n"
		"\n"

		"--ula-data-bus n            Sets the ula data bus value\n"


		"\n"
		"\n"
		"Tapes, Snapshots\n"
		"----------------\n"
		"\n"
		"--noautoload                No autoload tape file on Spectrum, ZX80 or ZX81\n"
		"--fastautoload              Do the autoload process at top speed\n"
		"--noautoselectfileopt       Do not autoselect emulation options for known snap and tape files\n"
        "--no-fallbacktorealtape     Disable fallback to real tape setting\n"
        "--anyflagloading            Enables tape load routine to load without knowing block flag\n"
        "--autorewind                Autorewind standard tape when reaching end of tape\n"
		"--simulaterealload          Simulate real tape loading\n"
		"--simulaterealloadfast      Enable fast simulate real tape loading\n"
        "--deletetzxpauses           Do not follow pauses on TZX tapes\n"
		"--realloadfast              Fast loading of real tape\n"
		"--smartloadpath path        Select initial smartload path\n"
		"--addlastfile file          Add a file to the last files used\n"
		"--quicksavepath path        Select path for quicksave & continous autosave\n" 
		"--autoloadsnap              Load last snapshot on start\n"
		"--autosavesnap              Save snapshot on exit\n"
		"--autosnappath path         Folder to save/load automatic snapshots\n"
		"--tempdir path              Folder to save temporary files. Folder must exist and have read and write permissions\n"
		"--snap-no-change-machine    Do not change machine when loading sna or z80 snapshots. Just load it on memory\n"
		"--no-close-after-smartload  Do not close menu after SmartLoad\n"
        "--snapram-interval n        Generate a snapshot in ram every n seconds\n"
        "--snapram-max n             Maximum snapshots to keep in memory\n"
        "--snapram-rewind-timeout n  After this time pressed rewind action, the rewind position is reset to current\n"


		"\n"
		"\n"
		"Audio Features\n"
		"--------------\n"
		"\n"

		"--disableayspeech         Disable AY Speech sounds\n"
		"--disableenvelopes        Disable AY Envelopes\n"
		"--disablebeeper           Disable Beeper\n"
        "--disablerealbeeper       Disable real Beeper sound\n"
		"--totalaychips  n         Number of ay chips. Default 1\n"
		"--ay-stereo-mode n        Mode of AY stereo emulated: 0=Mono, 1=ACB, 2=ABC, 3=BAC, 4=Custom. Default Mono\n"
		"--ay-stereo-channel X n   Position of AY channel X (A, B or C) in case of Custom Stereo Mode. 0=Left, 1=Center, 2=Right\n"
		"--enableaudiodac          Enable DAC emulation. By default Specdrum\n"
		"--audiodactype type       Select one of audiodac types: "
);
		audiodac_print_types();

printf (
		"\n"
		"--audiovolume n           Sets the audio output volume to percentage n\n"
		"--zx8081vsyncsound        Enable vsync/tape sound on ZX80/81\n"
		"--ayplayer-end-exit       Exit emulator when end playing current ay file\n"
		"--ayplayer-end-no-repeat  Do not repeat playing from the beginning when end playing current ay file\n"
		"--ayplayer-inf-length n   Limit to n seconds to ay tracks with infinite length\n"
		"--ayplayer-any-length n   Limit to n seconds to all ay tracks\n"
		"--ayplayer-cpc            Set AY Player to CPC mode (default: Spectrum)\n"
        "--audiopiano-zoom n       Set zoom for Audio Chip Piano and Wave Piano (1-3)\n"
		"--enable-midi             Enable midi output\n"
		"--midi-client n           Set midi client value to n. Needed only on Linux with Alsa audio driver\n"
		"--midi-port n             Set midi port value to n. Needed on Windows and Linux with Alsa audio driver\n"
		"--midi-raw-device s       Set midi raw device to s. Needed on Linux with Alsa audio driver\n"
		"--midi-allow-tone-noise   Allow tone+noise channels on midi\n"
		"--midi-no-raw-mode        Do not use midi in raw mode. Raw mode is required on Linux to emulate AY midi registers\n"


		"\n"
		"\n"
		"Audio Driver Settings\n"
		"---------------------\n"
		"\n"

		"--noreset-audiobuffer-full  Do not reset audio buffer when it's full. By default it does reset the buffer when full, it helps reducing latency\n"
		"--enable-silencedetector    Enable silence detector. Silence detector is disabled by default\n"
		"--disable-silencedetector   Disable silence detector. Silence detector is disabled by default\n"


#ifdef COMPILE_ONEBITSPEAKER
        "--onebitspeaker-type t                   Define Speaker type for One Bit Speaker driver (0=PC Speaker,1=Raspberry PI GPIO)\n"
        "--onebitspeaker-improved                 Improved One Bit Speaker sound but uses more cpu\n"
        "--onebitspeaker-hifreq-filter            Enable filter on One Bit Speaker to avoid high frequency sounds\n"
        "--onebitspeaker-hifreq-filter-divider n  Set the divider value for the One Bit Speaker hi freq filter. Accepted values from 1 to 15. Final frequency will be 15600/2/n Hz\n"

#endif



#ifdef COMPILE_ALSA
		"--alsaperiodsize n          Alsa audio periodsize multiplier (2 or 4). Default 2. Lower values reduce latency but can increase cpu usage\n"


#ifdef USE_PTHREADS

		"--fifoalsabuffersize n      Alsa fifo buffer size multiplier (4 to 10). Default 4. Lower values reduce latency but can increase cpu usage\n"

#endif
#endif



#ifdef COMPILE_PULSE
#ifdef USE_PTHREADS
        "--pulseperiodsize n         Pulse audio periodsize multiplier (1 to 4). Default 2. Lower values reduce latency but can increase cpu usage\n"
        "--fifopulsebuffersize n     Pulse fifo buffer size multiplier (4 to 10). Default 10. Lower values reduce latency but can increase cpu usage\n"
#endif
#endif

#ifdef COMPILE_COREAUDIO
        "--fifocorebuffersize n      Coreaudio fifo buffer size multiplier (2 to 10). Default 2. Lower values reduce latency but can increase cpu usage\n"
#endif

#ifdef COMPILE_SDL
		);


		//Cerramos el printf para que quede mas claro que hacemos un printf con un parametro
		printf (
		"--sdlsamplesize n           SDL audio sample size (128 to 2048). Default %d. Lower values reduce latency but can increase cpu usage\n",DEFAULT_AUDIOSDL_SAMPLES);
		printf (
		"--fifosdlbuffersize n       SDL fifo buffer size multiplier (2 to 10). Default 2. Lower values reduce latency but can increase cpu usage\n"
		"--sdlrawkeyboard            SDL read keyboard in raw mode, needed for ZX Recreated to work well\n");


		printf (
#endif



		"\n"
		"\n"
		"Display Settings\n"
		"----------------\n"
		"\n"

		"--realvideo                      Enable real video display - for Spectrum (rainbow and other advanced effects) and ZX80/81 (non standard & hi-res modes)\n"
		"--no-detect-realvideo            Disable real video autodetection\n"
		"--tbblue-legacy-hicolor          Allow legacy hi-color effects on pixel/attribute display zone\n"
		"--tbblue-legacy-border           Allow legacy border effects on tbblue machine\n"
        "--tbblue-no-sprite-optimization  Disable tbblue sprite render optimization\n"

		//"--tsconf-fast-render       Enables fast render of Tiles and Sprites for TSConf. Uses less host cpu but it's less realistic: doesn't do scanline render but full frame render\n"

		"--snoweffect                     Enable snow effect support for Spectrum\n"
		"--enablegigascreen               Enable Gigascreen video support\n"
		"--enableinterlaced               Enable Interlaced video support\n"
		"--enableulaplus                  Enable ULAplus video modes\n"
		"--enablespectra                  Enable Spectra video modes\n"
		"--enabletimexvideo               Enable Timex video modes\n"
		"--disablerealtimex512            Disable real Timex mode 512x192. In this case, it's scalled to 256x192 but allows scanline effects\n"
		"--enable16c                      Enable 16C video mode support\n"
		"--enablezgx                      Enable ZGX Sprite chip\n"
		"--autodetectwrx                  Enable WRX autodetect setting on ZX80/ZX81\n"
		"--wrx                            Enable WRX mode on ZX80/ZX81\n"
		"--vsync-minimum-length n         Set ZX80/81 Vsync minimum length in t-states (minimum 100, maximum 999)\n"
		"--chroma81                       Enable Chroma81 support on ZX80/ZX81\n"
		"--videozx8081 n                  Emulate ZX80/81 Display on Spectrum. n=pixel threshold (1..16. 4=normal)\n"
		"--videofastblack                 Emulate black screen on fast mode on ZX80/ZX81\n"
		"--no-ocr-alternatechars          Disable looking for an alternate character set other than the ROM default on OCR functions\n"
		"--scr file                       Load Screen File at startup\n"
	    "--arttextthresold n              Pixel threshold for artistic emulation for curses & stdout & simpletext (1..16. 4=normal)\n"
	    "--disablearttext                 Disable artistic emulation for curses & stdout & simpletext\n"
		"--allpixeltotext                 Enable all pixel to text mode\n"
		"--allpixeltotext-scale n         All pixel to text mode scale\n"
		"--allpixeltotext-invert          All pixel to text mode invert mode\n"		
        "--allpixeltotext-width n         All pixel to text max width (in chars) (minimum 1, maximum 9999)\n"
        "--allpixeltotext-x-offset n      All pixel to text X-Offset (in chars) (minimum 0, maximum 9999)\n"
        "--allpixeltotext-height n        All pixel to text max height (in chars) (minimum 1, maximum 9999)\n"
        "--allpixeltotext-y-offset n      All pixel to text Y-Offset (in chars) (minimum 0, maximum 9999)\n"    
		"--text-keyboard-add text         Add a string to the Adventure Text OSD Keyboard. The first addition erases the default text keyboard.\n"
		" You can use hotkeys by using double character ~~ just before the letter, for example:\n"
		" --text-keyboard-add ~~north   --text-keyboard-add e~~xamine\n");

printf (
		"--text-keyboard-length n         Define the duration for every key press on the Adventure Text OSD Keyboard, in 1/50 seconds (default %d). Minimum 10, maximum 100\n"
		"The half of this value, the key will be pressed, the other half, released. Example: --text-keyboard-length 50 to last 1 second\n",
		DEFAULT_ADV_KEYBOARD_KEY_LENGTH);

printf (
		"--text-keyboard-finalspc         Sends a space after every word on the Adventure Text OSD Keyboard\n"            
		"--red                            Force display mode with red colour\n"
		"--green                          Force display mode with green colour\n"
		"--blue                           Force display mode with blue colour\n"
		"  Note: You can combine colours, for example, --red --green for Yellow display, or --red --green --blue for Gray display\n"
		"--inversevideo                   Inverse display colours\n"
		"--realpalette                    Use real Spectrum colour palette according to info by Richard Atkinson\n"

#ifdef COMPILE_AA
        "--aaslow                         Use slow rendering on aalib\n"
#endif



#ifdef COMPILE_CURSESW
		"--curses-ext-utf                 Use extended utf characters to have 64x48 display, only on Spectrum and curses drivers\n"
#endif


		"--autoredrawstdout               Enable automatic display redraw for stdout & simpletext drivers\n"
		"--sendansi                       Sends ANSI terminal control escape sequences for stdout & simpletext drivers, to use colours and cursor control\n"
		"--textfps n                      Sets FPS for stdout and simpletext text drivers\n"        


		"\n"
		"\n"
		"Video Driver Settings\n"
		"---------------------\n"
		"\n"


#ifdef USE_XEXT
        "--disableshm                Disable X11 Shared Memory\n"
#endif

		"--nochangeslowparameters    Do not change any performance parameters (frameskip, realvideo, etc) "
		"on slow machines like raspberry, etc\n"


#ifdef COMPILE_FBDEV
        "--no-use-ttyfbdev           Do not use a tty on fbdev driver. It disables keyboard\n"
        "--no-use-ttyrawfbdev        Do not use keyboard on raw mode for fbdev driver\n"
        "--use-all-res-fbdev         Use all virtual resolution on fbdev driver. Experimental feature\n"
        "--decimal-full-scale-fbdev  Use non integer zoom to fill the display with full screen mode on fbdev driver\n"
        "--fbdev-double-buffer       Use double buffer to avoid flickering on menu but uses more cpu\n"
#ifdef EMULATE_RASPBERRY
        "--fbdev-no-res-change       Avoid resolution change on Raspberry Pi full screen mode\n"
        "--fbdev-margin-width n      Increment fbdev width size on n pixels on Raspberry Pi full screen mode\n"
        "--fbdev-margin-height n     Increment fbdev width height on n pixels on Raspberry Pi full screen mode\n"
#endif

#endif


		"\n"
		"\n"
		"File Browser Settings\n"
		"---------------------\n"
		"\n"

		"--filebrowser-hide-dirs            Do not show directories on file selector menus\n"
        "--filebrowser-hide-size            Do not show file sizes on file selector menus\n"
        "--filebrowser-allow-folder-delete  Allows deleting folders on the file utilities browser. Enable it AT YOUR OWN RISK\n"
        "--fileviewer-hex                   File viewer always shows file contents in hexadecimal+ascii\n"
		"--no-file-previews                 Do not show file previews on file selector menus\n"
        "--reduce-file-previews             Reduce file previews to half size\n"


		"\n"
		"\n"
		"Function Keys Settings\n"
		"----------------------\n"
		"\n"


		);

        int i;
		printf (
	    "--def-f-function key action  Define F key to do an action. action can be: ");


        for (i=0;i<MAX_F_FUNCTIONS;i++) {
            printf ("%s ",defined_direct_functions_array[i].texto_funcion);
        }



		printf (
		"\n"


		"\n"
		"\n"
		"OSD Settings\n"
		"------------\n"
		"\n"


		"--enable-watermark      Adds a watermark to the display. Needs realvideo\n"
		"--watermark-position n  Where to put watermark. 0: Top left, 1: Top right. 2: Bottom left. 3: Bottom right\n"
        "--nosplash              Disable all splash texts\n"


		"\n"
		"\n"
		"General Settings\n"
		"----------------\n"
		"\n"

		"--zoomx n                   Horizontal Zoom Factor\n"
		"--zoomy n                   Vertical Zoom Factor\n"
		
		"--reduce-075                Reduce display size 4/3 (divide by 4, multiply by 3)\n"
		"--reduce-075-no-antialias   Disable antialias for reduction, enabled by default\n"
		"--reduce-075-offset-x n     Destination offset x on reduced display\n"
		"--reduce-075-offset-y n     Destination offset y on reduced display\n"

		"--frameskip n               Set frameskip (0=none, 1=25 FPS, 2=16 FPS, etc)\n"
		"--disable-autoframeskip     Disable autoframeskip\n"
        "--autoframeskip-moving-win  Autoframeskip even when moving windows\n"
        "--disable-flash             Disable flash\n"
		"--fullscreen                Enable full screen\n"
		"--disableborder             Disable Border\n"   
        "--disablefooter             Disable window footer\n"             
        "--ignoremouseclickopenmenu  Ignore mouse clicking to open menu or ZX Desktop buttons\n" 
        "--limitopenmenu             Limit the action to open menu (F5 by default, joystick button). To open it, you must press the key 3 times in one second\n"               
        "--language language         Select alternate language for menu. Available languages: es (Spanish), ca (Catalan). Default language if not set: English\n"
        "--online-download-path p    Where to download files from the speccy and zx81 online browser. If not set, they are download to a temporary folder\n"

#ifndef MINGW
		"--no-cpu-usage              Do not show host CPU usage on footer\n"
#endif

		"--no-cpu-temp               Do not show host CPU temperature on footer\n"
		"--no-fps                    Do not show FPS on footer\n"
        "--nowelcomemessage          Disable welcome message\n"
        "--disablemenufileutils      Disable File Utilities menu\n"        

		"\n"
		"\n"
		"ZX Desktop Settings\n"
		"-------------------\n"
		"\n"


		"--enable-zxdesktop                             Enable ZX Desktop space\n"
		"--zxdesktop-width n                            ZX Desktop width\n"
        "--zxdesktop-height n                           ZX Desktop height\n"
		"--zxdesktop-fill-type n                        ZX Desktop fill type (0,1,2,3,4 or 5)\n"
		"--zxdesktop-fill-primary-color n               ZX Desktop primary fill color (0-15)\n"
		"--zxdesktop-fill-secondary-color n             ZX Desktop secondary fill color (0-15)\n"
        "--zxdesktop-fill-degraded-inverted             ZX Desktop inverted colours on degraded fill type\n"
		"--zxdesktop-new-items                          Try to place new menu items on the ZX Desktop space\n"
		"--zxdesktop-disable-buttons                    Disable ZX Desktop direct access buttons\n"
		"--zxdesktop-transparent-upper-buttons          Make ZX Desktop upper buttons transparent\n"
		"--zxdesktop-transparent-lower-buttons          Make ZX Desktop lower buttons transparent\n"
        "--zxdesktop-disable-box-upper-buttons          Disable box around ZX Desktop upper buttons\n"
        "--zxdesktop-disable-box-lower-buttons          Disable box around ZX Desktop lower buttons\n"  
        "--zxdesktop-disable-footer-switch              Disable ZX Desktop footer enlarge/reduce buttons\n"
		"--zxdesktop-disable-on-fullscreen              Disable ZX Desktop when going to full screen\n"
        "--zxdesktop-disable-frame-emulated-display     Disable showing a frame around the emulated machine display\n"
        "--zxdesktop-scr-file f                         Set ZX Desktop SCR background file\n"
        "--zxdesktop-scr-enable                         Enable ZX Desktop SCR background file\n"
        "--zxdesktop-scr-centered                       Center ZX Desktop SCR background\n"
        "--zxdesktop-scr-fillscale                      Scale automatic for ZX Desktop SCR background\n"
        "--zxdesktop-scr-mixbackground                  Mix SCR image with background\n"
        "--zxdesktop-scr-scalefactor n                  Scale manually for ZX Desktop SCR background\n"
        "--zxdesktop-scr-disable-flash                  Disable flash for ZX Desktop SCR background\n"
        "--zxdesktop-disable-configurable-icons         Disable configurable icons on ZX Desktop\n"  
        "--zxdesktop-no-transparent-configurable-icons  Make ZX Desktop configurable icons non transparent\n"
        "--zxdesktop-no-configurable-icons-text-bg      Disable background on configurable icons text\n"         

        "--zxdesktop-add-icon x y a n e s               Add icon to position x,y, to action a, icon name n, extra parameters e, status s. "
          "Icon name and extra parameters are mandatory, so if they are blank, just write it as \"\". status can be: exists or deleted. action can be: ");     
     

        for (i=0;i<MAX_F_FUNCTIONS;i++) {
            printf ("%s ",defined_direct_functions_array[i].texto_funcion);
        }


		printf (
		"\n"

	    "--def-button-function button action    Define Button to do an action. action can be: ");


			for (i=0;i<MAX_F_FUNCTIONS;i++) {
				printf ("%s ",defined_direct_functions_array[i].texto_funcion);
			}



        printf("\n"        


		"\n"
		"\n"
		"ZX Vision Settings\n"
		"-------------------\n"
		"\n"


		//"--invespokerom n           Inves rom poke value\n"

		"--menucharwidth n                        Character size width for menus valid values: 8,7,6 or 5\n"
		"--hidemousepointer                       Hide Mouse Pointer. Not all video drivers support this\n"
		"--disablemenumouse                       Disable mouse on emulator menu\n"
        
		//"--overlayinfo              Overlay on screen some machine info, like when loading tape\n"
	
		"--disablemultitaskmenu                   When multitask is disabled, both emulation, background windows and other menu features are stopped when opening the menu\n"
		//"--disablebw-no-multitask   Disable changing to black & white colours on the emulator machine when menu open and multitask is off\n"
        "--stopemulationmenu                      When multitask is enabled, you can disable emulation when opening the menu\n"
		"--hide-menu-percentage-bar               Hides vertical percentaje bar on the right of text windows and file selector\n"
        "--hide-menu-submenu-indicator            Hides submenu indicator character (>) on menu items with submenus\n"
		"--hide-menu-minimize-button              Hides minimize button on the title window\n"
        "--hide-menu-maximize-button              Hides maximize button on the title window\n"
		"--hide-menu-close-button                 Hides close button on the title window\n"
        "--show-menu-background-button            Shows background button on inactive windows\n"
		"--invert-menu-mouse-scroll               Inverts mouse scroll movement\n"
        "--right-mouse-esc                        Right button mouse simulates ESC key and not secondary actions\n"
		"--allow-background-windows               Allow putting windows in background\n"
        "--allow-background-windows-closed-menu   Allow these background windows even when menu closed\n"
		);

	printf (
		"--menu-mix-method s                      How to mix menu and the layer below. s should be one of:");

		
		for (i=0;i<MAX_MENU_MIX_METHODS;i++) {
			printf ("%s ",screen_menu_mix_methods_strings[i]);
		}

		printf ("\n");



printf (

		"--menu-transparency-perc n               Transparency percentage to apply to menu\n"
		//"--menu-darken-when-open    Darken layer below menu when menu open\n"
		"--menu-bw-multitask                      Grayscale layer below menu when menu opened and multitask is disabled\n"		
		"--disabletooltips                        Disable tooltips on menu\n"
		"--no-first-aid s                         Disable first aid message s. Do not throw any error if invalid\n"
		"--disable-all-first-aid                  Disable all first aid messages\n" 
		"--forcevisiblehotkeys                    Force always show hotkeys. By default it will only be shown after a timeout or wrong key pressed\n"
		"--forceconfirmyes                        Force confirmation dialogs yes/no always to yes\n"
		"--gui-style s                            Set GUI style. Available: ");

		estilo_gui_retorna_nombres();


printf (
		"\n"
        "--charset s                              Set Charset. This setting must be after --gui-style (if used). Available: ");

        charset_retorna_nombres();

printf (
		"\n"


		"--setmachinebyname                       Select machine by name instead of manufacturer\n"
		"--disablemenu                            Disable menu\n"
		"--disablemenuandexit                     Disable menu. Any event that opens the menu will exit the emulator\n"
		
		//"--text-keyboard-clear      Clear all entries of the Adventure Text Keyboard\n"

		//"--windowgeometry s x y w h Set window geometry. Parameters: window name (s), x coord, y coord, width (w), height (h)\n"
        //"--windowgeometry-ext s x y w h m Set window geometry, extended version (old version only kept for compatibility). Parameters: window name (s), x coord, y coord, width (w), height (h), is minimized (m)\n"
        //"--windowgeometry-full s x y w h wb hb m  Set window geometry, full version (old versions only kept for compatibility). "
        "--window-geometry s x y w h wb hb mn mx  Set window geometry. "
        " Parameters: window name (s), x coord, y coord, width (w), height (h), width before minimize/maximize (wb), height before minimize/maximize (hb), is minimized (mn), is maximized (mx)\n"
		"--disable-restore-windows                Disable restore windows on start\n"
        "--restorewindow s                        Restore window s on start\n"
		"--clear-all-windowgeometry               Clear all windows geometry thay may be loaded from the configuration file\n"
        "--restore-all-known-windows              Restore all known windows on start. Use ONLY for debugging!\n"


		"\n"
		"\n"
		"Hardware Settings\n"
		"-----------------\n"
		"\n"

		"--printerbitmapfile f   Sends printer output to image file. Supported formats: pbm, txt\n"
		"--printertextfile f     Sends printer output to text file using OCR method. Printer output is saved to a text file using OCR method to guess text.\n"
		"--redefinekey src dest  Redefine key scr to be key dest. You can write maximum 10 redefined keys\n"
        "                        Key must be ascii character numbers or a character included in escaped quotes, like: 97 (for 'a') or \\'q\\'\n"
        "                        (the escaped quotes are used only in command line; on configuration file, they are normal quotes '')\n"
        "--recreatedzx           Enable support for Recreated ZX Spectrum Keyboard\n"
		"--keymap n              Which kind of physical keyboard you have. Default 0 (English) or 1 (Spanish)\n"
		"--enablekempstonmouse   Enable kempston mouse emulation\n"
		"--kempstonmouse-sens n  Set kempston mouse sensitivity (1-%d)\n",MAX_KMOUSE_SENSITIVITY);

		printf (


        "\n"
        "Memory Settings\n"
        "---------------\n"
        "\n"

        "--zx8081mem n       Emulate 1,2,...16 kb of memory on ZX80/ZX81\n"
        "--zx8081ram8K2000   Emulate 8K RAM in 2000H for ZX80/ZX81\n"
        "--zx8081ram16K8000  Emulate 16K RAM in 8000H for ZX80/ZX81\n"
        "--zx8081ram16KC000  Emulate 16K RAM in C000H for ZX80/ZX81\n"
		"--acemem n          Emulate 3, 19, 35 or 51 kb of memory on Jupiter Ace\n"
		"--128kmem n         Set more than 128k RAM for 128k machines. Allowed values: 128, 256, 512"


		"\n"
		"\n"
		"Print char traps & Text to Speech\n"
		"---------------------------------\n"
		"\n"

		"--enableprintchartrap      Enable traps for standard ROM print char calls and non standard second & third traps. On Spectrum, ZX80, ZX81 machines, standard ROM calls are those using RST10H. On Z88, are those using OS_OUT and some other functions. Note: it is enabled by default on stdout & simpletext drivers\n"
		"--disableprintchartrap     Disable traps for ROM print char calls and second & third traps.\n"
        "--chardetectcompatnum      Enable ROM trap compatibility for printing numbers (but not good for printing from games, like PAWS)\n"
		"--automaticdetectchar      Enable automatic detection & try all method to find print character routines, for games not using RST 10H\n"
		"--secondtrapchar n         Print Char second trap address\n"
		"--secondtrapsum32          Print Char second trap sum 32 to character\n"
		"--thirdtrapchar n          Print Char third trap address\n"
        "--chardetectignorenl       Ignore new line characters (13, 10) on char detection\n"
        "--linewidth n              Print char line width\n"        

        //estos dos settings tienen tambien opcion para desactivar el setting por cambios en valor por defecto a partir ZEsarUX 9.3
		"--linewidthwaitspace       Text will be sent to speech when line is larger than line width and a space, comma or semicolon is detected\n"
        "--linewidthnowaitspace     Just disable previous setting\n"
        "--linewidthwaitdot         Text will be sent to speech when line is larger than line width and dot is detected\n"
        "--linewidthnowaitdot       Just disable previous setting\n"
		"--textspeechprogram p      Specify a path to a program or script to be sent the emulator text shown. For example, for text to speech: speech_filters/festival_filter.sh or speech_filters/macos_say_filter.sh\n"
		"--textspeechstopprogram p  Specify a path to a program or script in charge of stopping the running speech program. For example, speech_filters/stop_festival_filter.sh\n"
        "--textspeechgetstdout      Send stdout from script to debug console window\n"
		"--textspeechwait           Wait and pause the emulator until the Speech program returns\n"
		"--textspeechmenu           Also send text menu entries to Speech program\n"
		"--textspeechtimeout n      After some seconds the text will be sent to the Speech program when no new line is sent. Between 0 and 99. 0 means never\n"
        

		"\n"
		"\n"
		"Storage Settings\n"
		"----------------\n"
		"\n"

		"--mmc-file f                    Set mmc image file\n"
		"--enable-mmc                    Enable MMC emulation. Usually requires --mmc-file\n"
		"--mmc-write-protection          Enable MMC write protection\n"
		"--mmc-no-persistent-writes      Disable MMC persistent writes\n");

        printf(
            "--copy-file-to-mmc source dest  Add file from local filesystem to the mmc, before starting ZEsarUX. That copies the files in "
            "the mmc image and syncs the changes. "
            "You can use that setting up to %d times. Destination must not include 0:/ prefix. mmc file is set with setting --mmc-file\n",
            MAX_COPY_FILES_TO_MMC);
        
        printf(
		"--enable-divmmc-ports           Enable DIVMMC emulation ports only, but not paging. Usually requires --enable-mmc\n"
		"--enable-divmmc-paging          Enable DIVMMC paging only\n"
		"--enable-divmmc                 Enable DIVMMC emulation: ports & paging. Usually requires --enable-mmc\n"
		"--divmmc-rom f                  Sets divmmc firmware rom. If not set, uses default file\n"
		"--enable-zxmmc                  Enable ZXMMC emulation. Usually requires --enable-mmc\n"
		"--ide-file f                    Set ide image file\n"
		"--enable-ide                    Enable IDE emulation. Usually requires --ide-file\n"
		"--ide-write-protection          Enable IDE write protection\n"
		"--ide-no-persistent-writes      Disable IDE persistent writes\n"		
		"--enable-divide-ports           Enable DIVIDE emulation ports only, but not paging. Usually requires --enable-ide\n"
		"--enable-divide-paging          Enable DIVIDE paging only\n"
		"--enable-divide                 Enable DIVIDE emulation. Usually requires --enable-ide\n"
		"--divide-rom f                  Sets divide firmware rom. If not set, uses default file\n"
		"--enable-8bit-ide               Enable 8-bit simple IDE emulation. Requires --enable-ide\n"
		"--diviface-ram-size n           Sets divide/divmmc ram size in kb. Allowed values: 32, 64, 128, 256 or 512\n"
		"--enable-esxdos-handler         Enable ESXDOS traps handler. Requires divmmc or divide paging emulation\n"
		"--esxdos-root-dir p             Set ESXDOS root directory for traps handler. Uses current directory by default.\n"
        "--esxdos-readonly               Forbid write operations on ESXDOS handler\n"
        "--esxdos-local-dir p            Set ESXDOS local directory for traps handler. This is the relative directory used inside esxdos.\n"
		"--enable-zxpand                 Enable ZXpand emulation\n"
		"--zxpand-root-dir p             Set ZXpand root directory for sd/mmc filesystem. Uses current directory by default.\n"
		"                                Note: ZXpand does not use --mmc-file setting\n"
		"--zxunospifile path             File to use on ZX-Uno as SPI Flash. Default: zxuno.flash\n"
		"--zxunospi-write-protection     Enable ZX-Uno SPI Flash write protection\n"
		"--zxunospi-persistent-writes    Enable ZX-Uno SPI Flash persistent writes\n"
        "--zxuno-initial-64k f           Load a 64kb file that will be written on the initial 64kb space\n"
        "--dandanator-rom f              Set ZX Dandanator rom file\n"
        "--enable-dandanator             Enable ZX Dandanator emulation. Requires --dandanator-rom\n"
        "--dandanator-press-button       Simulates pressing button on ZX Dandanator. Requires --enable-dandanator\n"
		"--superupgrade-flash f          Set Superupgrade flash file\n"
		"--enable-superupgrade           Enable Superupgrade emulation. Requires --superupgrade-flash\n"
        "--kartusho-rom f                Set Kartusho rom file\n"
        "--enable-kartusho               Enable Kartusho emulation. Requires --kartusho-rom\n"
        "--ifrom-rom f                   Set iFrom rom file\n"
        "--enable-ifrom                  Enable iFrom emulation. Requires --ifrom-rom\n"
        "--dsk-file f                    Set +3 DSK image file\n"
        "--enable-dsk                    Enable +3 DSK emulation. Usually requires --dsk-file\n"
        "--dsk-write-protection          Enable +3 DSK write protection\n"
		"--dsk-no-persistent-writes      Disable +3 DSK persistent writes\n"
        "--enable-betadisk               Enable Betadisk emulation\n"
        "--trd-file f                    Set trd image file\n"
        "--enable-trd                    Enable TRD emulation. Usually requires --trd-file\n"
        "--trd-write-protection          Enable TRD write protection\n"
		"--trd-no-persistent-writes      Disable TRD persistent writes\n"
        "--hilow-file f                  Set HiLow Data Drive image file\n"
        "--enable-hilow                  Enable HiLow Data Drive. Usually requires --hilow-file\n"
		"--hilow-write-protection        Enable HiLow Data Drive write protection\n"
		"--hilow-no-persistent-writes    Disable HiLow Data Drive persistent writes\n"        
		"--enable-ql-mdv-flp             Enable QL Microdrive & Floppy emulation\n"
		"--ql-mdv1-root-dir p            Set QL mdv1 root directory\n"
		"--ql-mdv2-root-dir p            Set QL mdv2 root directory\n"
		"--ql-flp1-root-dir p            Set QL flp1 root directory\n"
        "--ql-mdv1-read-only             Mark mdv1 as read only\n"
        "--ql-mdv2-read-only             Mark mdv2 as read only\n"
        "--ql-flp1-read-only             Mark flp1 as read only\n"


		"\n"
        "\n"
        "External Tools\n"
        "--------------\n"
        "\n"

        "--tool-sox-path p     Set external tool sox path. Path can not include spaces\n"
        "--tool-gunzip-path p  Set external tool gunzip path. Path can not include spaces\n"
        "--tool-tar-path p     Set external tool tar path. Path can not include spaces\n"
        "--tool-unrar-path p   Set external tool unrar path. Path can not include spaces\n"


		"\n"
		"\n"
		"Joystick\n"
		"--------\n"
		"\n"

		"--joystickemulated type     Type of joystick emulated. Type can be one of: ");

	joystick_print_types();
		printf (" (default: %s).\n",joystick_texto[joystick_emulation]);
		printf ("  Note: if a joystick type has spaces in its name, you must write it between \"\"\n");


	printf(
        "--joystickfirekey n         Define which key triggers the fire function for the joystick: 0=Home, 1=RightAlt, 2=RightCtrl, 3=RightShift\n"
		"--disablerealjoystick       Disable real joystick emulation\n"
		"--realjoystickpath f        Change default real joystick device path (used on Linux)\n"
        "--realjoystickindex n       Change default real joystick device id (used on Windows and other OS with SDL driver)\n"
		"--realjoystick-calibrate n  Parameter to autocalibrate joystick axis. Axis values read from joystick less than n and greater than -n are considered as 0. Default: 16384. Not used on native linux real joystick\n"

#ifdef USE_LINUXREALJOYSTICK
		"--no-native-linux-realjoy  Do not use native linux real joystick support. Instead use the video driver joystick support (currently only SDL)\n"
#endif		
        "--joystickevent but evt     Set a joystick button or axis to an event (changes joystick to event table)\n"
        "                            If it's a button (not axis), must be specified with its number, without sign, for example: 2\n"
        "                            If it's axis, must be specified with its number and sign, for example: +2 or -2\n"
        "                            Event must be one of: ");

        realjoystick_print_event_keys();

	printf ("\n"
		"--joystickkeybt but key     Define a key pressed when a joystick button pressed (changes joystick to key table)\n"
		"                            If it's a button (not axis), must be specified with its number, without sign, for example: 2\n"
        "                            If it's axis, must be specified with its number and sign, for example: +2 or -2\n"
        "                            Key must be an ascii character number or a character included in escaped quotes, like: 13 (for enter) or \\'q\\'\n"
		"                            (the escaped quotes are used only in command line; on configuration file, they are normal quotes '')\n"
		"                            Note: to simulate Caps shift, use key value 128, and to simulate Symbol shift, use key value 129\n"
        "--joystickkeyev evt key     Define a key pressed when a joystick event is generated (changes joystick to key table)\n"
        "                            Event must be one of: ");

        realjoystick_print_event_keys();


	printf ("\n"
		"                            Key must be an ascii character number or a character included in escaped quotes, like: 13 (for enter) or \\'q\\' \n"
		"                            (the escaped quotes are used only in command line; on configuration file, they are normal quotes '')\n"
		"                            Note: to simulate Caps shift, use key value 128, and to simulate Symbol shift, use key value 129\n"

        	"\n"
		"  Note: As you may see, --joystickkeyev is not dependent on the real joystick type you use, because it sets an event to a key, "
		"and --joystickkeybt and --joystickevent are dependent on the real joystick type, because they set a button/axis number to "
		"an event or key, and button/axis number changes depending on the joystick (the exception here is the axis up/down/left/right "
		"which are the same for all joysticks: up: -1, down: +1, left: -1, right: +1)\n"

		"\n"
		"--clearkeylistonsmart       Clear all joystick (events and buttons) to keys table every smart loading.\n"
		"                            Joystick to events table is never cleared using this setting\n"
		"--cleareventlist            Clears joystick to events table\n"
		"--enablejoysticksimulator   Enable real joystick simulator. Only useful on development\n"


		"\n"
		"\n"
		"Network\n"
		"-------\n"
		"\n"
		
		"--zeng-remote-hostname s    ZENG last remote hostname\n"
		"--zeng-remote-port n        ZENG last remote port\n"
		"--zeng-snapshot-interval n  ZENG snapshot interval\n"
		"--zeng-iam-master           Tells this machine is a ZENG master\n"


		"\n"
		"\n"
		"Statistics\n"
		"----------\n"
		"\n"
		
		"--total-minutes-use n                 Total minutes of use of ZEsarUX\n"
		"--stats-send-already-asked            Do not ask to send statistics\n"
		"--stats-send-enabled                  Enable send statistics\n"
		"--stats-uuid s                        UUID to send statistics\n"
		"--stats-disable-check-updates         Disable checking of available ZEsarUX updates\n"
		"--stats-disable-check-yesterday-users Disable checking ZEsarUX yesterday users\n"
		"--stats-last-avail-version s          ZEsarUX last available version to download\n"
		"--stats-speccy-queries n              Total queries on the speccy online browser\n"
		"--stats-zx81-queries n                Total queries on the zx81 online browser\n"
		
			
		"\n"
		"\n"
		"Miscellaneous\n"
		"-------------\n"
		"\n"

		"--saveconf-on-exit                       Always save configuration when exiting emulator\n"
		"--helpcustomconfig                       Show help for autoconfig files\n"
		"--quickexit                              Exit emulator quickly: no yes/no confirmation and no fadeout\n"
		"--exit-after n                           Exit emulator after n seconds\n"
		"--last-version s                         String which identifies last version run. Usually doesnt need to change it, used to show the start popup of the new version changes\n"
		"--no-show-changelog                      Do not show changelog when updating version\n"
		"--disablebetawarning text                Do not pause beta warning message on boot for version named as that parameter text\n"
		"--tbblue-autoconfigure-sd-already-asked  Do not ask to autoconfigure tbblue initial SD image\n"
		
		//Esto no hace falta que lo vea un usuario, solo lo uso yo para probar partes del emulador 
		//"--codetests                Run develoment code tests\n"
		"--tonegenerator n                        Enable tone generator. Possible values: 1: generate max, 2: generate min, 3: generate min/max at 50 Hz\n");

        printf(
        "--sensor-set position type               Set sensor for menu View sensors. position must be 0 to %d. type must be one of:\n",MENU_VIEW_SENSORS_TOTAL_ELEMENTS-1);

        sensor_list_print();

        printf("\n"
        "--sensor-set-widget position type        Set widget type sensor for menu View sensors. position must be 0 to %d. type must be one of:\n",MENU_VIEW_SENSORS_TOTAL_ELEMENTS-1);

        widget_list_print();

        printf("\n"
        "--sensor-set-abs position                Set widget type absolute instead of percentaje for menu View sensors. position must be 0 to %d\n",MENU_VIEW_SENSORS_TOTAL_ELEMENTS-1);
 
        printf(
		"\n\n"

		"One-time actions\n"
		"----------------\n"
		"\n"
        "The following are actions that are executed from the console and don't start ZEsarUX:"
        "\n\n"
        "--convert-tap-tzx source destination           Convert tap source file to destination tzx\n"
        "--convert-tap-tzx-turbo-rg source destination  Convert tap source file to destination tzx turbo (4000 bauds) for use with Rodolfo Guerra ROMS\n"
        "--convert-tap-pzx source destination           Convert tap source file to destination pzx\n"
        "--convert-tzx-tap source destination           Convert tzx source file to destination tap\n"
        "--convert-pzx-tap source destination           Convert pzx source file to destination tap\n"


        "\n\n"        

	);

}

#ifdef USE_COCOA
int set_scrdriver_cocoa(void)
{
                        scr_refresca_pantalla=scrcocoa_refresca_pantalla;
												scr_refresca_pantalla_solo_driver=scrcocoa_refresca_pantalla_solo_driver;
                        scr_init_pantalla=scrcocoa_init;
                        scr_end_pantalla=scrcocoa_end;
                        scr_lee_puerto=scrcocoa_lee_puerto;
                        scr_actualiza_tablas_teclado=scrcocoa_actualiza_tablas_teclado;
        return 0;
}

#endif


#ifdef COMPILE_XWINDOWS
int set_scrdriver_xwindows(void)
{
                        scr_refresca_pantalla=scrxwindows_refresca_pantalla;
												scr_refresca_pantalla_solo_driver=scrxwindows_refresca_pantalla_solo_driver;
                        scr_init_pantalla=scrxwindows_init;
                        scr_end_pantalla=scrxwindows_end;
                        scr_lee_puerto=scrxwindows_lee_puerto;
                        scr_actualiza_tablas_teclado=scrxwindows_actualiza_tablas_teclado;
                        //scr_debug_registers=scrxwindows_debug_registers;
			//scr_messages_debug=scrxwindows_messages_debug;
	return 0;
}

#endif

#ifdef COMPILE_SDL
int set_scrdriver_sdl(void)
{
                        scr_refresca_pantalla=scrsdl_refresca_pantalla;
												scr_refresca_pantalla_solo_driver=scrsdl_refresca_pantalla_solo_driver;
                        scr_init_pantalla=scrsdl_init;
                        scr_end_pantalla=scrsdl_end;
                        scr_lee_puerto=scrsdl_lee_puerto;
                        scr_actualiza_tablas_teclado=scrsdl_actualiza_tablas_teclado;
        return 0;
}

#endif



#ifdef COMPILE_FBDEV
int set_scrdriver_fbdev(void)
{
                        scr_refresca_pantalla=scrfbdev_refresca_pantalla;
												scr_refresca_pantalla_solo_driver=scrfbdev_refresca_pantalla_solo_driver;
                        scr_init_pantalla=scrfbdev_init;
                        scr_end_pantalla=scrfbdev_end;
                        scr_lee_puerto=scrfbdev_lee_puerto;

			//la rutina de tabla teclado la establecemos en el init... pues se cambia segun si modo raw o no
                        //scr_actualiza_tablas_teclado=scrfbdev_actualiza_tablas_teclado;
                        //scr_debug_registers=scrfbdev_debug_registers;
                        //scr_messages_debug=scrfbdev_messages_debug;
        return 0;
}

#endif


#ifdef COMPILE_CURSES
int set_scrdriver_curses(void)
{
//asignar pantalla curses
scr_refresca_pantalla=scrcurses_refresca_pantalla;
scr_refresca_pantalla_solo_driver=scrcurses_refresca_pantalla_solo_driver;
scr_init_pantalla=scrcurses_init;
scr_end_pantalla=scrcurses_end;
scr_lee_puerto=scrcurses_lee_puerto;
scr_actualiza_tablas_teclado=scrcurses_actualiza_tablas_teclado;
//scr_debug_registers=scrcurses_debug_registers;
//			scr_messages_debug=scrcurses_messages_debug;
	return 0;
}
#endif

#ifdef COMPILE_AA
int set_scrdriver_aa(void)
{
//asignar pantalla aa
scr_refresca_pantalla=scraa_refresca_pantalla;
scr_refresca_pantalla_solo_driver=scraa_refresca_pantalla_solo_driver;
scr_init_pantalla=scraa_init;
scr_end_pantalla=scraa_end;
scr_lee_puerto=scraa_lee_puerto;
scr_actualiza_tablas_teclado=scraa_actualiza_tablas_teclado;


//scr_debug_registers=scraa_debug_registers;
//scr_messages_debug=scraa_messages_debug;
	return 0;
}
#endif

#ifdef COMPILE_CACA
int set_scrdriver_caca(void)
{
//asignar pantalla caca
scr_refresca_pantalla=scrcaca_refresca_pantalla;
scr_refresca_pantalla_solo_driver=scrcaca_refresca_pantalla_solo_driver;
scr_init_pantalla=scrcaca_init;
scr_end_pantalla=scrcaca_end;
scr_lee_puerto=scrcaca_lee_puerto;
scr_actualiza_tablas_teclado=scrcaca_actualiza_tablas_teclado;
//scr_debug_registers=scrcaca_debug_registers;
//			scr_messages_debug=scrcaca_messages_debug;
	return 0;
}
#endif




int set_scrdriver_null(void) {
scr_refresca_pantalla=scrnull_refresca_pantalla;
scr_refresca_pantalla_solo_driver=scrnull_refresca_pantalla_solo_driver;
scr_init_pantalla=scrnull_init;
scr_end_pantalla=scrnull_end;
scr_lee_puerto=scrnull_lee_puerto;
scr_actualiza_tablas_teclado=scrnull_actualiza_tablas_teclado;
//scr_debug_registers=scrnull_debug_registers;
//			scr_messages_debug=scrnull_messages_debug;
	return 0;
}

#ifdef COMPILE_STDOUT

int set_scrdriver_stdout(void) {
scr_refresca_pantalla=scrstdout_refresca_pantalla;
scr_refresca_pantalla_solo_driver=scrstdout_refresca_pantalla_solo_driver;
scr_init_pantalla=scrstdout_init;
scr_end_pantalla=scrstdout_end;
scr_lee_puerto=scrstdout_lee_puerto;
scr_actualiza_tablas_teclado=scrstdout_actualiza_tablas_teclado;
//scr_debug_registers=scrstdout_debug_registers;
//                      scr_messages_debug=scrstdout_messages_debug;
        return 0;
}

#endif

#ifdef COMPILE_SIMPLETEXT

int set_scrdriver_simpletext(void) {
scr_refresca_pantalla=scrsimpletext_refresca_pantalla;
scr_refresca_pantalla_solo_driver=scrsimpletext_refresca_pantalla_solo_driver;
scr_init_pantalla=scrsimpletext_init;
scr_end_pantalla=scrsimpletext_end;
scr_lee_puerto=scrsimpletext_lee_puerto;
scr_actualiza_tablas_teclado=scrsimpletext_actualiza_tablas_teclado;

        return 0;
}

#endif



#ifdef COMPILE_DSP
int set_audiodriver_dsp(void) {
                        audio_init=audiodsp_init;
                        audio_send_frame=audiodsp_send_frame;
			audio_thread_finish=audiodsp_thread_finish;
			audio_end=audiodsp_end;
			audio_get_buffer_info=audiodsp_get_buffer_info;
			return 0;

                }
#endif


#ifdef COMPILE_ONEBITSPEAKER
int set_audiodriver_onebitspeaker(void) {
                        audio_init=audioonebitspeaker_init;
                        audio_send_frame=audioonebitspeaker_send_frame;
			audio_thread_finish=audioonebitspeaker_thread_finish;
			audio_end=audioonebitspeaker_end;
			audio_get_buffer_info=audioonebitspeaker_get_buffer_info;
			return 0;

                }
#endif



#ifdef COMPILE_SDL
int set_audiodriver_sdl(void) {
                        audio_init=audiosdl_init;
                        audio_send_frame=audiosdl_send_frame;
                        audio_thread_finish=audiosdl_thread_finish;
                        audio_end=audiosdl_end;
												audio_get_buffer_info=audiosdl_get_buffer_info;
                        return 0;

                }
#endif


#ifdef COMPILE_ALSA
int set_audiodriver_alsa(void) {
                        audio_init=audioalsa_init;
                        audio_send_frame=audioalsa_send_frame;
			audio_thread_finish=audioalsa_thread_finish;
			audio_end=audioalsa_end;
			audio_get_buffer_info=audioalsa_get_buffer_info;
			return 0;

                }
#endif

#ifdef COMPILE_PULSE
int set_audiodriver_pulse(void) {
                        audio_init=audiopulse_init;
                        audio_send_frame=audiopulse_send_frame;
                        audio_thread_finish=audiopulse_thread_finish;
			audio_end=audiopulse_end;
			audio_get_buffer_info=audiopulse_get_buffer_info;
                        return 0;

                }
#endif


#ifdef COMPILE_COREAUDIO
int set_audiodriver_coreaudio(void) {
                        audio_init=audiocoreaudio_init;
                        audio_send_frame=audiocoreaudio_send_frame;
                        audio_thread_finish=audiocoreaudio_thread_finish;
			audio_end=audiocoreaudio_end;
			audio_get_buffer_info=audiocoreaudio_get_buffer_info;
                        return 0;

                }
#endif

//Randomize Usando segundos del reloj
//Usado en AY Chip y Random ram
void init_randomize_noise_value(void)
{

                int chips=ay_retorna_numero_chips();

                int i;

                for (i=0;i<chips;i++) {

	                gettimeofday(&z80_interrupts_timer_antes, NULL);
        	        randomize_noise[i]=z80_interrupts_timer_antes.tv_sec & 0xFFFF;
                	//printf ("randomize vale: %d\n",randomize_noise);

		}
}


void random_ram(z80_byte *puntero,int longitud)
{

	//Cada vez que se inicializa una maquina, reasignar valor randomize
	init_randomize_noise_value();

	z80_byte valor;

	z80_byte valor_h, valor_l;

	for (;longitud;longitud--,puntero++) {


		ay_randomize(0);

		//randomize_noise es valor de 16 bits. sacar uno de 8 bits
		valor_h=value_16_to_8h(randomize_noise[0]);
		valor_l=value_16_to_8l(randomize_noise[0]);

		//valor=randomize_noise & 0xFF;

		//Solo hacemos random en determinadas maquinas
		if (MACHINE_IS_SPECTRUM_16_48) {
			//hacemos xor con los dos
			valor=valor_h ^ valor_l;
		}

		else {
			valor=0;
		}

		//printf ("random:%d\n",valor);
		*puntero=valor;
	}

}


void avisar_opcion_obsoleta(char *texto){
	//Solo avisa si no hay guardado de configuracion
	//Porque si hay guardado, al guardarlo ya lo grabara como toca
	if (save_configuration_file_on_exit.v==0) debug_printf (VERBOSE_ERR,"%s",texto);
}

//Patron de llenado para inves: FF,00,FF,00, etc...
void random_ram_inves(z80_byte *puntero,int longitud)
{

        z80_byte valor=255;

        for (;longitud;longitud--,puntero++) {

                //printf ("random:%d\n",valor);
                *puntero=valor;

		valor = valor ^255;
        }

	//asumimos que el ultimo valor enviado desde menu sera el por defecto (255)
	last_inves_low_ram_poke_menu=255;

}



struct s_machine_names machine_names[]={

//char *machine_names[]={
    {"ZX Spectrum 16k",              	0},
    {"ZX Spectrum 48k", 			1},
    {"Inves Spectrum+",			2},
    {"Microdigital TK90X",		3},
    {"Microdigital TK90X (Spanish)",	4},
    {"Microdigital TK95",		5},
    {"ZX Spectrum+ 128k",			6},
    {"ZX Spectrum+ 128k (Spanish)",		7},
    {"ZX Spectrum +2",			8},
    {"ZX Spectrum +2 (French)",		9},
    {"ZX Spectrum +2 (Spanish)",		10},
    {"ZX Spectrum +2A (ROM v4.0)",		11},
    {"ZX Spectrum +2A (ROM v4.1)",		12},
    {"ZX Spectrum +2A (Spanish)",		13},
    {"ZX-Uno",         			14},
    {"Chloe 140SE",    			15},
    {"Chloe 280SE",    			16},
    {"Timex TS2068",   			17},
    {"Prism 512",       			18},
    {"ZX Spectrum Next",   			19},
    {"ZX Spectrum+ 48k (Spanish)",		20},
    {"Pentagon",		21},
    {"Chrome", MACHINE_ID_CHROME},
    {"ZX-Evolution TS-Conf", MACHINE_ID_TSCONF},
    {"ZX-Evolution BaseConf", MACHINE_ID_BASECONF},


    {"ZX Spectrum +3 (ROM v4.0)",		MACHINE_ID_SPECTRUM_P3_40},
    {"ZX Spectrum +3 (ROM v4.1)",		MACHINE_ID_SPECTRUM_P3_41},
    {"ZX Spectrum +3 (Spanish)",		MACHINE_ID_SPECTRUM_P3_SPA},

    {"ZX Spectrum+ 48k",		MACHINE_ID_SPECTRUM_48_PLUS_ENG},
    {"Timex TC2048",   			MACHINE_ID_TIMEX_TC2048},
    {"Timex TC2068",   			MACHINE_ID_TIMEX_TC2068},

    {"MSX1",MACHINE_ID_MSX1},
    {"ColecoVision",MACHINE_ID_COLECO},
    {"SG-1000",MACHINE_ID_SG1000},
    {"Spectravideo 318",MACHINE_ID_SVI_318},
    {"Spectravideo 328",MACHINE_ID_SVI_328},
    {"Master System",MACHINE_ID_SMS},

    {"ZX80",  				120},
    {"ZX81",  				121},
    {"Jupiter Ace",  			122},
    {"Z88",  				130},
    {"CPC 464",  			MACHINE_ID_CPC_464},
    {"CPC 4128",  			MACHINE_ID_CPC_4128},
    {"Sam Coupe", 			150},
    {"QL",				160},
    {"MK14", MACHINE_ID_MK14_STANDARD},

    //Indicador de final
    {"",0}

};




char *get_machine_name(z80_byte machine)
{
	int i;
    //printf("Current machine: %d\n",machine);

	for (i=0;i<99999;i++) {
		if (machine_names[i].nombre_maquina[0]==0) {
			char mensaje[200];
			sprintf (mensaje,"No machine name found for machine id: %d",machine);
			cpu_panic(mensaje);
		}

		if (machine_names[i].id==machine) return machine_names[i].nombre_maquina;
	}

	//Aunque aqui no se llega nunca, para que no se queje el compilador
	return NULL;
}

//int maquina_anterior_cambio_cpu_speed=-1;

//ajustar timer de final de cada frame de pantalla. En vez de lanzar un frame cada 20 ms, hacerlo mas o menos rapido
void set_emulator_speed(void)
{

/*
	int forzarcambio=0;

	//Si se cambia entre maquina Z88 y cualquier otra, forzar
	if (MACHINE_IS_Z88 && maquina_anterior_cambio_cpu_speed!=30 ||
	    !MACHINE_IS_Z88 && maquina_anterior_cambio_cpu_speed==30) {
		forzarcambio=1;
		debug_printf ("
	}


	maquina_anterior_cambio_cpu_speed=machine_type;
*/

	/*
	if (porcentaje_velocidad_emulador==100) {
		//Si velocidad es la normal, no hacer ningun recalculo
		timer_sleep_machine=original_timer_sleep_machine;
	}

	else {
		timer_sleep_machine=original_timer_sleep_machine*100/porcentaje_velocidad_emulador;
		if (timer_sleep_machine==0) timer_sleep_machine=1;

	}
	*/

	//Calcular velocidad. caso normal que porcentaje=100, valor queda timer_sleep_machine=original_timer_sleep_machine*100/100 = original_timer_sleep_machine
	timer_sleep_machine=original_timer_sleep_machine*100/porcentaje_velocidad_emulador;
	if (timer_sleep_machine==0) timer_sleep_machine=1;

	//Si ha cambiado velocidad, reiniciar driver audio con frecuencia adecuada
	if (anterior_porcentaje_velocidad_emulador!=porcentaje_velocidad_emulador) {
		if (audio_end!=NULL) audio_end();
		frecuencia_sonido_variable=FRECUENCIA_CONSTANTE_NORMAL_SONIDO*porcentaje_velocidad_emulador/100;
		if (audio_init!=NULL) {
			if (audio_init()) {
				//Error
				fallback_audio_null();
			}
		}
	}

	anterior_porcentaje_velocidad_emulador=porcentaje_velocidad_emulador;

	debug_printf (VERBOSE_INFO,"Setting timer_sleep_machine to %d us",timer_sleep_machine);

	/*
	Lo anterior cubre los casos:
	-cambio de maquina de spectrum a z88: se recalcula timer_sleep_machine, sin tener que reiniciar driver audio
	-cambio de porcentaje cpu: se recalcula timer_sleep_machine y se reinicia driver audio si hay cambio porcentaje velocidad
	*/

}

/*
void set_emulator_speed(void)
{
	//ajuste mediante t estados por linea. requiere desactivar realvideo
	//ademas la tabla de memoria contended se saldra de rango...
	screen_testados_linea=screen_testados_linea*porcentaje_velocidad_emulador/100;

	printf ("t estados por linea: %d\n",screen_testados_linea);

}
*/


//ajusta max_cpu_cycles segun porcentaje cpu
//desactivado de momento
void old_set_emulator_speed(void)
{

	return ;

	//Esto se hacia antes cuando la cpu no estaba sincronizada y solo habia el contador de maximo de instrucciones
	//ahora habria que cuadrar muchos contadores en base a la velocidad: total_testates, testates de border, etc....

	//if (machine_type==20) max_cpu_cycles=MAX_CPU_CYCLES_ZX80;
	//else if (machine_type==21) max_cpu_cycles=MAX_CPU_CYCLES_ZX81;
	//else max_cpu_cycles=MAX_CPU_CYCLES_SPECTRUM;


	if (porcentaje_velocidad_emulador==100) return;

	//controlar valores. por ejemplo, un porcentaje de 1, da un floating point exception en alguna parte del codigo...
	if (porcentaje_velocidad_emulador<10 || porcentaje_velocidad_emulador>1000) {
		debug_printf (VERBOSE_ERR,"Invalid value for cpu speed: %d",porcentaje_velocidad_emulador);
		return;
	}

	debug_printf (VERBOSE_INFO,"Setting cpu speed to: %d%%",porcentaje_velocidad_emulador);

	//lo hacemos multiple de 312
	//max_cpu_cycles=max_cpu_cycles/312;

	//aplicamos porcentaje
	//max_cpu_cycles=max_cpu_cycles*porcentaje_velocidad_emulador;
	//max_cpu_cycles=max_cpu_cycles/100;

	//y lo volvemos a dejar *312
	//max_cpu_cycles=max_cpu_cycles*312;

}


//asignar memoria de maquina, liberando memoria antes si conviene
void malloc_machine(int tamanyo)
{
	if (memoria_spectrum!=NULL) {
		debug_printf(VERBOSE_INFO,"Freeing previous Machine memory");
		free(memoria_spectrum);
	}

	debug_printf(VERBOSE_INFO,"Allocating %d bytes for Machine memory",tamanyo);
	memoria_spectrum=malloc(tamanyo);

	if (memoria_spectrum==NULL) {
		cpu_panic ("Error. Cannot allocate Machine memory");
	}


	//En Z88, el puntero que se usa es realmente z88_puntero_memoria
	z88_puntero_memoria=memoria_spectrum;
}

void malloc_mem_machine(void) {

	//Caso Inves. Asignamos 64 kb ram+16kb para rom

	if (MACHINE_IS_INVES) {
		malloc_machine(65536+16384);
		random_ram_inves(memoria_spectrum,65536);
	}

        else if (MACHINE_IS_SPECTRUM_16_48 || MACHINE_IS_ZX8081 || MACHINE_IS_ACE) {
                //total 64kb
                malloc_machine(65536);
                random_ram(memoria_spectrum+16384,49152);

        }

        else if (MACHINE_IS_SPECTRUM_128_P2) {

                //32 kb rom, 128-1024 ram
                malloc_machine((32+1024)*1024);
                random_ram(memoria_spectrum+32768,1024*1024);

		mem_init_memory_tables_128k();
                mem_set_normal_pages_128k();

        }

	 else if (MACHINE_IS_SPECTRUM_P2A_P3) {

                //64 kb rom, 128-1024 ram
                malloc_machine((64+1024)*1024);
                random_ram(memoria_spectrum+65536,1024*1024);

		mem_init_memory_tables_p2a();
                mem_set_normal_pages_p2a();

        }

	else if (MACHINE_IS_ZXUNO) {
		//16 KB rom
		//512 KB SRAM
		//1024 FLASH
        //4 paginas vram adicionales de 8192 kb
		malloc_machine((ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE+ZXUNO_SPI_SIZE+8*4)*1024);
		random_ram(memoria_spectrum,(ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE+ZXUNO_SPI_SIZE+8*4)*1024);


		zxuno_init_memory_tables();
		zxuno_set_memory_pages();



                //mem_set_normal_pages_zxuno();



	}

	  else if (MACHINE_IS_CHLOE) {

		//Necesita 32 kb rom, 256 kb ram (EX 64, DOCK 64, HOME 128)

                //32 kb rom, 256 ram
                malloc_machine((32+256)*1024);
                random_ram(memoria_spectrum+32768,256*1024);

		chloe_init_memory_tables();
		chloe_set_memory_pages();

        }

	else if (MACHINE_IS_PRISM) {

                //Necesita 4096 KB rom, 512 kb ram (EX 64, DOCK 64, HOME 128)

                //4096 kb rom, 512 ram
                malloc_machine((4096+512)*1024);

                random_ram(memoria_spectrum+(4096*1024),512*1024);


		//Y paginas VRAM
		prism_malloc_vram();

                prism_init_memory_tables();
                prism_set_memory_pages();

        }

        else if (MACHINE_IS_TBBLUE) {

		//2048 KB RAM + 16 KB FPGA ROM (8 kb repetido dos veces)
                malloc_machine( TBBLUE_TOTAL_MEMORY_USED*1024);

                random_ram(memoria_spectrum,TBBLUE_TOTAL_MEMORY_USED*1024);


                tbblue_init_memory_tables();
                tbblue_set_memory_pages();

        }

				else if (MACHINE_IS_CHROME) {

				//Necesita 64 kb rom, 160 kb ram
				malloc_machine((64+160)*1024);
				random_ram(memoria_spectrum+65536,160*1024);

				chrome_init_memory_tables();
				chrome_set_memory_pages();

						}

						else if (MACHINE_IS_TSCONF) {

		                //512 kb rom, 4096 ram
		                malloc_machine((512+4096)*1024);
		                //temp de momento nada de random random_ram(memoria_spectrum+512*1024,4096*1024);

				tsconf_init_memory_tables();
				tsconf_set_memory_pages();


		        }

					else if (MACHINE_IS_BASECONF) {

		                //512 kb rom, 4096 ram
		                malloc_machine((512+4096)*1024);
		                //temp de momento nada de random random_ram(memoria_spectrum+512*1024,4096*1024);

				baseconf_init_memory_tables();
				baseconf_set_memory_pages();


		        }

						else if (MACHINE_IS_MK14) {

										//64 kb ram/rom
										malloc_machine(65536);

						}


          else if (MACHINE_IS_TIMEX_T2068) {

                //Necesita 192KB de memoria (EX 64, DOCK 64, HOME 48, ROM 16)

                malloc_machine((192)*1024);
                random_ram(memoria_spectrum,192*1024);

                timex_init_memory_tables();
                timex_set_memory_pages();

		//Inicializar memoria DOCK con 255
		timex_empty_dock_space();

        }

	else if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) {

		//32 kb rom
		//128 kb ram. Asignamos 128kb ram aunque maquina sea de 64 kb de ram->Facilita las funciones

                malloc_machine(160*1024);
                random_ram(memoria_spectrum,160*1024);

                cpc_init_memory_tables();
                cpc_set_memory_pages();


        }
        
        else if (MACHINE_IS_MSX1) {
                //total 64kb * 4
                malloc_machine(65536*4);
                random_ram(memoria_spectrum+32768,32768);


				//y 16kb para vram
				msx_alloc_vram_memory();


				msx_init_memory_tables();

        }


        else if (MACHINE_IS_SVI) {
			//Total:  5 RAMS de 32 kb, 3 ROMS de 32 kb -> 5*32 + 3*32 = 160 + 96 = 256
                
                malloc_machine(256*1024);
                random_ram(memoria_spectrum,256*1024);


				//y 16kb para vram
				svi_alloc_vram_memory();


				svi_init_memory_tables();

        }		

        else if (MACHINE_IS_COLECO) {
                //total 64kb 
                malloc_machine(65536);
                random_ram(memoria_spectrum+32768,32768);


				//y 16kb para vram
				coleco_alloc_vram_memory();


				coleco_init_memory_tables();

        }		

        else if (MACHINE_IS_SG1000) {
                //total 64kb 
                malloc_machine(65536);
                random_ram(memoria_spectrum,65536);


				//y 16kb para vram
				sg1000_alloc_vram_memory();


				sg1000_init_memory_tables();

        }			

        else if (MACHINE_IS_SMS) {
                //total 1 MByte ROM + 8 kb RAM 
                malloc_machine(SMS_MAX_ROM_SIZE+8192);
                random_ram(memoria_spectrum,SMS_MAX_ROM_SIZE+8192);


				//y 16kb para vram
				sms_alloc_vram_memory();


				sms_init_memory_tables();

        }	        


	else if (MACHINE_IS_Z88) {
		//Asignar 4 MB
		//z88_puntero_memoria=malloc(4*1024*1024);
		malloc_machine(4*1024*1024);
		random_ram(memoria_spectrum,4*1024*1024);
	}

	else if (MACHINE_IS_SAM) {
		//Asignar 512 kb+32 kb de rom
		//Incluso si la maquina es de 256kb, nosotros asignamos el maximo (512)
		malloc_machine((512+32)*1024);
		random_ram(memoria_spectrum,(512+32)*1024);

                sam_init_memory_tables();
                sam_set_memory_pages();
	}

	else if (MACHINE_IS_QL) {
		//Asignar 256 kb de ram
		malloc_machine(QL_MEM_LIMIT+1);
		memoria_ql=memoria_spectrum;
		//random_ram(memoria_ql,QL_MEM_LIMIT+1);
	}

	else {
		cpu_panic("Do not know how to allocate mem for active machine");
	}

}



void set_machine_params(void)
{

/*
0=Sinclair 16k
1=Sinclair 48k
2=Inves Spectrum+
3=tk90x
4=tk90xs
5=tk95
6=Sinclair 128k
7=Sinclair 128k Español
8=Amstrad +2
9=Amstrad +2 - Frances
10=Amstrad +2 - Espa�ol
11=Amstrad +2A (ROM v4.0)
12=Amstrad +2A (ROM v4.1)
13=Amstrad +2A - Espa�ol
14=ZX-Uno
15=Chloe 140SE
16=Chloe 280SE
17=Timex TS2068
18=Prism
19=TBBlue
20=Spectrum + Spanish
21=Pentagon
22=Chrome
23=ZX-Evolution TS-Conf
24=ZX-Evolution BaseConf (no implementado aun)

25=Amstrad +3 (ROM v4.0)
26=Amstrad +3 (ROM v4.1)
27=Amstrad +3 - Espa�ol

28=Spectrum + English
29=Timex TC2048
30=Timex TC2068
31-39=Reservadas otras Spectrum
100=colecovision
101=sega sg1000
102=Spectravideo 318
103=Spectravideo 328
104=sega Master System
110-119 msx:
110 msx1
120=zx80 (old 20)
121=zx81 (old 21)
122=jupiter ace (old 22)
130=z88 (old 30)
140=amstrad cpc464 
141=amstrad cpc4128 
150=Sam Coupe (old 50)
151-59 reservado para otros sam (old 51-59)
160=QL Standard
161-179 reservado para otros QL

180=MK14 Standard
181-189 reservado para otros MK14
*/

		char mensaje_error[200];

		//defaults
		ay_chip_present.v=0;
		sn_chip_present.v=0;
        i8049_chip_present=0;

		if (!MACHINE_IS_Z88) {
			//timer_sleep_machine=original_timer_sleep_machine=20000;
			original_timer_sleep_machine=20000;
        	        set_emulator_speed();
		}

		allow_write_rom.v=0;

		multiface_enabled.v=0;

		dandanator_enabled.v=0;
		superupgrade_enabled.v=0;
		kartusho_enabled.v=0;
		ifrom_enabled.v=0;
		betadisk_enabled.v=0;
		hilow_enabled.v=0;
        samram_enabled.v=0;

		plus3dos_traps.v=0;
		pd765_enabled.v=0;
		dskplusthree_emulation.v=0;

		//nota: combiene que allow_write_rom.v sea 0 al desactivar superupgrade
		//porque si estaba activo allow_write_rom.v antes, y desactivamos superupgrade,
		//al intentar desactivar allow_write, se produce segmentation fault

		//Si maquina anterior era pentagon, desactivar timing pentagon y activar contended memory
		if (last_machine_type==MACHINE_ID_PENTAGON) {
			debug_printf(VERBOSE_DEBUG,"Disabling pentagon timing and enabling contended memory because previous machine was Pentagon");
			pentagon_timing.v=0;
			contend_enabled.v=1;
		}

		chloe_keyboard.v=0;


		input_file_keyboard_turbo.v=0;

		//Modo turbo no. No, parece que cambiarlo aqui sin mas no provoca cambios cuando se hace smartload
		//no tocarlo, dejamos que el usuario se encargue de ponerlo a 1 si quiere
		//printf ("Setting cpu speed to 1\n");
		//sleep (3);
		//cpu_turbo_speed=1;

		//en spectrum, 32. en pentagon, 36
		cpu_duracion_pulso_interrupcion=32;


		z80_cpu_current_type=Z80_TYPE_GENERIC;		


		//cpu_core_loop=cpu_core_loop_spectrum;
		if (MACHINE_IS_SPECTRUM) {
			cpu_core_loop_active=CPU_CORE_SPECTRUM;
		}

		else if (MACHINE_IS_ZX8081) {
			cpu_core_loop_active=CPU_CORE_ZX8081;
		}

		else if (MACHINE_IS_ACE) {
			cpu_core_loop_active=CPU_CORE_ACE;
		}

		else if (MACHINE_IS_CPC) {
			cpu_core_loop_active=CPU_CORE_CPC;
		}

		else if (MACHINE_IS_SAM) {
			cpu_core_loop_active=CPU_CORE_SAM;
		}

		else if (MACHINE_IS_QL) {
			cpu_core_loop_active=CPU_CORE_QL;
		}

		else if (MACHINE_IS_MK14) {
			cpu_core_loop_active=CPU_CORE_MK14;
		}

		else if (MACHINE_IS_MSX) {
			cpu_core_loop_active=CPU_CORE_MSX;
		}		

		else if (MACHINE_IS_SVI) {
			cpu_core_loop_active=CPU_CORE_SVI;
		}			

		else if (MACHINE_IS_COLECO) {
			cpu_core_loop_active=CPU_CORE_COLECO;
		}	

		else if (MACHINE_IS_SG1000) {
			cpu_core_loop_active=CPU_CORE_SG1000;
		}		

		else if (MACHINE_IS_SMS) {
			cpu_core_loop_active=CPU_CORE_SMS;
		}	        	


		else {
			cpu_core_loop_active=CPU_CORE_Z88;
		}

		debug_breakpoints_enabled.v=0;
		set_cpu_core_loop();

		//esto ya establece el cpu core. Debe estar antes de establecer peek, poke, lee_puerto, out_port
		//dado que "restaura" dichas funciones a sus valores originales... pero la primera vez, esos valores originales valen 0
		//breakpoints_disable();

		out_port=out_port_spectrum;


		//desactivar ram refresh emulation, cpu transaction log y cualquier otra funcion que altere el cpu_core, el peek_byte, etc
		cpu_transaction_log_enabled.v=0;
		cpu_code_coverage_enabled.v=0;
		cpu_history_enabled.v=0;
		machine_emulate_memory_refresh=0;

		push_valor=push_valor_default;
		extended_stack_enabled.v=0;


		//Valores usados en real video
		//TODO. para 128k hay 1 linea menos superior. ZX80,ZX81??
		screen_invisible_borde_superior=8;
		screen_borde_superior=56;

		screen_total_borde_inferior=56;

		screen_total_borde_izquierdo=48;
		screen_total_borde_derecho=48;
		screen_invisible_borde_derecho=96;
		screen_testados_linea=224;

		//Reseteos para ZX80/81

                //offset_zx8081_t_coordx=0;
		video_zx8081_lnctr_adjust.v=0;

		video_zx8081_estabilizador_imagen.v=1;

		disable_wrx();
                //wrx_present.v=0;
		//wrx_mueve_primera_columna.v=1;
                zx8081_vsync_sound.v=0;
		//video_zx8081_decremento_x_cuando_mayor=8;
		minimo_duracion_vsync=DEFAULT_MINIMO_DURACION_VSYNC;

		//normalmente excepto +2a
		port_from_ula=port_from_ula_48k;

		fetch_opcode=fetch_opcode_spectrum;



		//2 para ZX81 mejor
		//0 para spectrum mejor
		realtape_volumen=0;
        realtape_algorithm_new_noise_reduction=0;


		//Resetear a zona memoria por defecto. Evita cuelgues al intentar usar una zona de memoria que ya no esta disponible,
		//ejemplo: iniciar maquina msx. abrir view sprites->activar hardware. F5 y cambiar a spectravideo. F5. Floating point exception.
		//menu_debug_set_memory_zone_mapped();
		//En principio esto ya no hace falta, desde menu_debug_set_memory_zone_attr, menu_debug_get_mapped_byte y menu_debug_write_mapped_byte
		//ya se está conmutando correctamente a memory mapped cuando la zona anterior ya no está disponible

		screen_set_parameters_slow_machines();

		//Cuando se viene aqui desde cambio modo turbo, no interesa vaciar buffer, si no, se oye fatal. Ejemplo: uwol en tsconf
		if (set_machine_empties_audio_buffer.v) audio_empty_buffer();

		//Inicializar paletas de colores. Colores basicos de spectrum y algunos derivados (gigascreen, etc) dependen de si paleta real activa y segun que maquina
		//Cuando se viene aqui desde cambio modo turbo, no interesa reinicializar paleta, si no, se realentiza todo mucho. Ejemplo: uwol en tsconf
		//Nota: Se le podria cambiar el nombre a la variable set_machine_empties_audio_buffer.v
		if (set_machine_empties_audio_buffer.v) screen_init_colour_table();

		set_machine_empties_audio_buffer.v=1;

		if (MACHINE_IS_CHLOE) chloe_keyboard.v=1;

		//Cargar keymap de manera generica. De momento solo se usa en Z88, CPC y Chloe
		if (scr_z88_cpc_load_keymap!=NULL) scr_z88_cpc_load_keymap();


		if (MACHINE_IS_TBBLUE) datagear_dma_enable();
		//else datagear_dma_disable();

		//Pentagon no tiene modo Timex
		if (MACHINE_IS_PENTAGON) {
			if (timex_video_emulation.v) disable_timex_video();
		}

		//Quitar mensajes de footer establecidos con autoselectoptions.c
		//Desactivado. Esto provoca:
		//Al cambiar de maquina, si hay un mensaje pendiente, seguira desplazandose hasta que acabe
		//Si en cambio activasemos esta linea comentada, pasaria que al cargar un snapshot que tenga .config,
		//se estableceria el mensaje del programname del .config, y esta linea borraria el mensaje y no se veria
		//temp tape_options_set_first_message_counter=tape_options_set_second_message_counter=0;


		if (MACHINE_IS_SPECTRUM_16_48) {
			contend_read=contend_read_48k;
			contend_read_no_mreq=contend_read_no_mreq_48k;
			contend_write_no_mreq=contend_write_no_mreq_48k;

			ula_contend_port_early=ula_contend_port_early_48k;
			ula_contend_port_late=ula_contend_port_late_48k;

			//Ajustes para Inves
			if (MACHINE_IS_INVES) {
	                        screen_testados_linea=228;
        	                screen_invisible_borde_superior=7;
                	        screen_invisible_borde_derecho=104;
			}
		}

		else if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
                        contend_read=contend_read_128k;
                        contend_read_no_mreq=contend_read_no_mreq_128k;
                        contend_write_no_mreq=contend_write_no_mreq_128k;

			ula_contend_port_early=ula_contend_port_early_128k;
			ula_contend_port_late=ula_contend_port_late_128k;


	                screen_testados_linea=228;
        	        screen_invisible_borde_superior=7;
                	screen_invisible_borde_derecho=104;

			contend_pages_128k_p2a=contend_pages_128k;

		        if (MACHINE_IS_SPECTRUM_P2A_P3) {
                		port_from_ula=port_from_ula_p2a;
				contend_pages_128k_p2a=contend_pages_p2a;
			}

			if (MACHINE_IS_PENTAGON) {
				contend_enabled.v=0;
				ula_enable_pentagon_timing_no_common();
			}

			if (MACHINE_IS_SPECTRUM_P3) {
				pd765_enable();
				plus3dos_traps.v=1;
			}

		}

                else if (MACHINE_IS_ZXUNO) {
			zxuno_set_timing_48k();
                }


		else if (MACHINE_IS_ZX8081) {

		        screen_invisible_borde_superior=16;
			screen_borde_superior=48;

                	contend_read=contend_read_zx8081;
	                contend_read_no_mreq=contend_read_no_mreq_zx8081;
        	        contend_write_no_mreq=contend_write_no_mreq_zx8081;

			ula_contend_port_early=ula_contend_port_early_zx8081;
			ula_contend_port_late=ula_contend_port_late_zx8081;


			//2 para ZX81 mejor
			//0 para spectrum mejor
			realtape_volumen=2;
            realtape_algorithm_new_noise_reduction=2;

		}

		else if (MACHINE_IS_ACE) {
		        screen_invisible_borde_superior=16;
			screen_borde_superior=48;

                        contend_read=contend_read_ace;
                        contend_read_no_mreq=contend_read_no_mreq_ace;
                        contend_write_no_mreq=contend_write_no_mreq_ace;

                        ula_contend_port_early=ula_contend_port_early_ace;
                        ula_contend_port_late=ula_contend_port_late_ace;


                }


	       else if (MACHINE_IS_CHLOE) {
                        contend_read=contend_read_chloe;
                        contend_read_no_mreq=contend_read_no_mreq_chloe;
                        contend_write_no_mreq=contend_write_no_mreq_chloe;

                        ula_contend_port_early=ula_contend_port_early_chloe;
                        ula_contend_port_late=ula_contend_port_late_chloe;


                }

		else if (MACHINE_IS_PRISM) {
                        contend_read=contend_read_prism;
                        contend_read_no_mreq=contend_read_no_mreq_prism;
                        contend_write_no_mreq=contend_write_no_mreq_prism;

                        ula_contend_port_early=ula_contend_port_early_prism;
                        ula_contend_port_late=ula_contend_port_late_prism;

			screen_invisible_borde_superior=45;
			screen_borde_superior=48;
			screen_total_borde_inferior=48;

			screen_total_borde_izquierdo=64;
			screen_total_borde_derecho=64;
			screen_invisible_borde_derecho=158;
			screen_testados_linea=133;
                }

                if (MACHINE_IS_TBBLUE) {
			tbblue_set_timing_48k();

		        //divmmc arranca desactivado, lo desactivamos asi para que no cambie las funciones peek/poke
			//esto ya se desactiva en set_machine
		        /*if (divmmc_enabled.v) {
		                divmmc_enabled.v=0;
		                diviface_enabled.v=0;
		        }
			*/


                }


								else if (MACHINE_IS_CHROME) {
															 contend_read=contend_read_chrome;
															 contend_read_no_mreq=contend_read_no_mreq_chrome;
															 contend_write_no_mreq=contend_write_no_mreq_chrome;

															 ula_contend_port_early=ula_contend_port_early_chrome;
															 ula_contend_port_late=ula_contend_port_late_chrome;


			 	                		screen_testados_linea=228;
			         	        		screen_invisible_borde_superior=7;
			                 			screen_invisible_borde_derecho=104;


											 }

						else if (MACHINE_IS_TSCONF) {
							contend_read=contend_read_tsconf;
							contend_read_no_mreq=contend_read_no_mreq_tsconf;
					        contend_write_no_mreq=contend_write_no_mreq_tsconf;

				 			ula_contend_port_early=ula_contend_port_early_tsconf;
				 			ula_contend_port_late=ula_contend_port_late_tsconf;
					        screen_testados_linea=224;


		
							screen_invisible_borde_superior	=32; //para que sumen 320
							screen_borde_superior=48;

							screen_total_borde_inferior=48;

							/* Tiempos de tsconf:
							Line, pixels: 
blank - left border - pixels - right border
256x192: 88-52-256-52
320x200,
320x240: 88-20-320-20
360x288: 88-0-360-0

Frame, lines:
blank - upper border - pixels - lower border
256x192: 32-48-192-48
320x200: 32-44-200-44
320x240: 32-24-240-24
360x288: 32-0-288-0

You don't need timings for H/V sync =)

							*/


                        	//los timings son realmente estos :
                        	screen_total_borde_izquierdo=64;
                        	screen_total_borde_derecho=64;
                        	screen_invisible_borde_derecho=64;

							z80_cpu_current_type=Z80_TYPE_CMOS;	


							
				}

					else if (MACHINE_IS_BASECONF) {
							contend_read=contend_read_baseconf;
							contend_read_no_mreq=contend_read_no_mreq_baseconf;
					        contend_write_no_mreq=contend_write_no_mreq_baseconf;

				 			ula_contend_port_early=ula_contend_port_early_baseconf;
				 			ula_contend_port_late=ula_contend_port_late_baseconf;

							 z80_cpu_current_type=Z80_TYPE_CMOS;	

                                        //Temp timings 128k
                        screen_testados_linea=228;
                        screen_invisible_borde_superior=7;
                        screen_invisible_borde_derecho=104;


							
				}

               else if (MACHINE_IS_TIMEX_T2068) {
                        contend_read=contend_read_timex;
                        contend_read_no_mreq=contend_read_no_mreq_timex;
                        contend_write_no_mreq=contend_write_no_mreq_timex;

                        ula_contend_port_early=ula_contend_port_early_timex;
                        ula_contend_port_late=ula_contend_port_late_timex;


                }




                else if (MACHINE_IS_Z88) {
                        contend_read=contend_read_z88;
                        contend_read_no_mreq=contend_read_no_mreq_z88;
                        contend_write_no_mreq=contend_write_no_mreq_z88;

                        ula_contend_port_early=ula_contend_port_early_z88;
                        ula_contend_port_late=ula_contend_port_late_z88;

						z80_cpu_current_type=Z80_TYPE_CMOS;		

			//timer_sleep_machine=original_timer_sleep_machine=5000;
			original_timer_sleep_machine=5000;
			set_emulator_speed();

                }


		else if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) {
                        contend_read=contend_read_cpc;
                        contend_read_no_mreq=contend_read_no_mreq_cpc;
                        contend_write_no_mreq=contend_write_no_mreq_cpc;

                        ula_contend_port_early=ula_contend_port_early_cpc;
                        ula_contend_port_late=ula_contend_port_late_cpc;


                }

		else if (MACHINE_IS_MSX) {
			contend_read=contend_read_msx1;
			contend_read_no_mreq=contend_read_no_mreq_msx1;
			contend_write_no_mreq=contend_write_no_mreq_msx1;

			ula_contend_port_early=ula_contend_port_early_msx1;
			ula_contend_port_late=ula_contend_port_late_msx1;

			
			// 3579545 / 312 / 50
			screen_testados_linea=229;

            //TODO
            //Parece que MSX usa CMOS tambien
            //https://faqwiki.zxnet.co.uk/wiki/Z80#Differences_between_NMOS_and_CMOS_Z80s
            //https://www.msx.org/forum/development/msx-development/bug-z80-emulation-or-tr-hw
            //z80_cpu_current_type=Z80_TYPE_CMOS;
            //Pero a falta de confirmar, de momento no lo toco

		}		

		else if (MACHINE_IS_SVI) {
			contend_read=contend_read_svi;
			contend_read_no_mreq=contend_read_no_mreq_svi;
			contend_write_no_mreq=contend_write_no_mreq_svi;

			ula_contend_port_early=ula_contend_port_early_svi;
			ula_contend_port_late=ula_contend_port_late_svi;


			screen_testados_linea=228;

		}			

		else if (MACHINE_IS_COLECO) {
			contend_read=contend_read_coleco;
			contend_read_no_mreq=contend_read_no_mreq_coleco;
			contend_write_no_mreq=contend_write_no_mreq_coleco;

			ula_contend_port_early=ula_contend_port_early_coleco;
			ula_contend_port_late=ula_contend_port_late_coleco;

			
			screen_testados_linea=215;

		}		

		else if (MACHINE_IS_SG1000) {
			contend_read=contend_read_sg1000;
			contend_read_no_mreq=contend_read_no_mreq_sg1000;
			contend_write_no_mreq=contend_write_no_mreq_sg1000;

			ula_contend_port_early=ula_contend_port_early_sg1000;
			ula_contend_port_late=ula_contend_port_late_sg1000;

			
			screen_testados_linea=228;

		}		

		else if (MACHINE_IS_SMS) {
			contend_read=contend_read_sms;
			contend_read_no_mreq=contend_read_no_mreq_sms;
			contend_write_no_mreq=contend_write_no_mreq_sms;

			ula_contend_port_early=ula_contend_port_early_sms;
			ula_contend_port_late=ula_contend_port_late_sms;

			
			screen_testados_linea=228;

		}	        				

		else if (MACHINE_IS_SAM) {
			contend_read=contend_read_sam;
                        contend_read_no_mreq=contend_read_no_mreq_sam;
                        contend_write_no_mreq=contend_write_no_mreq_sam;

                        ula_contend_port_early=ula_contend_port_early_sam;
                        ula_contend_port_late=ula_contend_port_late_sam;
		}


		else if (MACHINE_IS_MK14) {
																	 contend_read=contend_read_mk14;
																	 contend_read_no_mreq=contend_read_no_mreq_mk14;
																	 contend_write_no_mreq=contend_write_no_mreq_mk14;

								ula_contend_port_early=ula_contend_port_early_mk14;
								ula_contend_port_late=ula_contend_port_late_mk14;
		}




	switch (current_machine_type) {

		case 0:
		poke_byte=poke_byte_spectrum_16k;
		peek_byte=peek_byte_spectrum_16k;
		peek_byte_no_time=peek_byte_no_time_spectrum_16k;
		poke_byte_no_time=poke_byte_no_time_spectrum_16k;
		lee_puerto=lee_puerto_spectrum;
		break;

		case 1:
		case MACHINE_ID_SPECTRUM_48_PLUS_SPA:
        case MACHINE_ID_SPECTRUM_48_PLUS_ENG:
        case MACHINE_ID_TIMEX_TC2048:
		poke_byte=poke_byte_spectrum_48k;
		peek_byte=peek_byte_spectrum_48k;
		peek_byte_no_time=peek_byte_no_time_spectrum_48k;
		poke_byte_no_time=poke_byte_no_time_spectrum_48k;
		lee_puerto=lee_puerto_spectrum;

        if (MACHINE_IS_TIMEX_TC2048) {
            enable_timex_video();
        }

		break;

                case 2:
                poke_byte=poke_byte_spectrum_inves;
                peek_byte=peek_byte_spectrum_inves;
		peek_byte_no_time=peek_byte_no_time_spectrum_inves;
		poke_byte_no_time=poke_byte_no_time_spectrum_inves;
		lee_puerto=lee_puerto_spectrum;
                break;

                case 3:
                poke_byte=poke_byte_spectrum_48k;
                peek_byte=peek_byte_spectrum_48k;
		peek_byte_no_time=peek_byte_no_time_spectrum_48k;
		poke_byte_no_time=poke_byte_no_time_spectrum_48k;
                lee_puerto=lee_puerto_spectrum;
                break;

                case 4:
                poke_byte=poke_byte_spectrum_48k;
                peek_byte=peek_byte_spectrum_48k;
		peek_byte_no_time=peek_byte_no_time_spectrum_48k;
		poke_byte_no_time=poke_byte_no_time_spectrum_48k;
                lee_puerto=lee_puerto_spectrum;
                break;

                case 5:
                poke_byte=poke_byte_spectrum_48k;
                peek_byte=peek_byte_spectrum_48k;
		peek_byte_no_time=peek_byte_no_time_spectrum_48k;
		poke_byte_no_time=poke_byte_no_time_spectrum_48k;
                lee_puerto=lee_puerto_spectrum;
                break;




                case 6:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		peek_byte_no_time=peek_byte_no_time_spectrum_128k;
		poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
		ay_chip_present.v=1;
                break;

                case 7:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		peek_byte_no_time=peek_byte_no_time_spectrum_128k;
		poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;




                case 8:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		peek_byte_no_time=peek_byte_no_time_spectrum_128k;
		poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;

                case 9:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		peek_byte_no_time=peek_byte_no_time_spectrum_128k;
		poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;

                case 10:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		peek_byte_no_time=peek_byte_no_time_spectrum_128k;
        poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;


                //11=Amstrad +2A (ROM v4.0
                case 11:
                case 12:
                case 13:
                case MACHINE_ID_SPECTRUM_P3_40:
                case MACHINE_ID_SPECTRUM_P3_41:
                case MACHINE_ID_SPECTRUM_P3_SPA:
                poke_byte=poke_byte_spectrum_128kp2a;
                peek_byte=peek_byte_spectrum_128kp2a;
		peek_byte_no_time=peek_byte_no_time_spectrum_128kp2a;
		poke_byte_no_time=poke_byte_no_time_spectrum_128kp2a;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;

                //12=Amstrad +2A (ROM v4.1)
                /*case 12:
                poke_byte=poke_byte_spectrum_128kp2a;
                peek_byte=peek_byte_spectrum_128kp2a;
		peek_byte_no_time=peek_byte_no_time_spectrum_128kp2a;
		poke_byte_no_time=poke_byte_no_time_spectrum_128kp2a;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;

                //13=Amstrad +2A - Espa�ol
                case 13:
                poke_byte=poke_byte_spectrum_128kp2a;
                peek_byte=peek_byte_spectrum_128kp2a;
		peek_byte_no_time=peek_byte_no_time_spectrum_128kp2a;
		poke_byte_no_time=poke_byte_no_time_spectrum_128kp2a;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;*/


                case 14:
                poke_byte=poke_byte_zxuno;
                peek_byte=peek_byte_zxuno;
                peek_byte_no_time=peek_byte_no_time_zxuno;
                poke_byte_no_time=poke_byte_no_time_zxuno;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;


		case 15:
		case 16:
                poke_byte=poke_byte_chloe;
                peek_byte=peek_byte_chloe;
                peek_byte_no_time=peek_byte_no_time_chloe;
                poke_byte_no_time=poke_byte_no_time_chloe;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
		enable_rainbow();
		enable_ulaplus();
		enable_timex_video();

		//Chloe 280SE lleva turbosound
		if (MACHINE_IS_CHLOE_280SE) {
		        ay_chip_selected=0;
		        //turbosound_enabled.v=1;
						total_ay_chips=2;
		}


                break;

                case MACHINE_ID_TIMEX_TS2068:
                case MACHINE_ID_TIMEX_TC2068:
                poke_byte=poke_byte_timex;
                peek_byte=peek_byte_timex;
                peek_byte_no_time=peek_byte_no_time_timex;
                poke_byte_no_time=poke_byte_no_time_timex;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                enable_rainbow();
                enable_timex_video();

                break;

		case 18:
                poke_byte=poke_byte_prism;
                peek_byte=peek_byte_prism;
                peek_byte_no_time=peek_byte_no_time_prism;
                poke_byte_no_time=poke_byte_no_time_prism;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                enable_rainbow();
                enable_ulaplus();
                enable_timex_video();

                break;

                case 19:
                poke_byte=poke_byte_tbblue;
                peek_byte=peek_byte_tbblue;
                peek_byte_no_time=peek_byte_no_time_tbblue;
                poke_byte_no_time=poke_byte_no_time_tbblue;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;

				//Solo forzar real video una vez al entrar aquí. Para poder dejar real video desactivado si el usuario lo quiere,
				//pues aqui se entra siempre al cambiar velocidad cpu (y eso pasa en la rom cada vez que te mueves por el menu del 128k por ejemplo)
				//TODO: siempre que el usuario entre al emulador se activara la primera vez
				if (!tbblue_already_autoenabled_rainbow) {
					enable_rainbow();
					//lo mismo para timex video
					enable_timex_video();
				}

				tbblue_already_autoenabled_rainbow=1;

				multiface_type=MULTIFACE_TYPE_THREE;

								//Si maquina destino es tbblue, forzar a activar border. De momento no se ve bien con border desactivado
								border_enabled.v=1;				

                break;


                case MACHINE_ID_PENTAGON:

				//Pentagon
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
                peek_byte_no_time=peek_byte_no_time_spectrum_128k;
                poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;


				//en spectrum, 32. en pentagon, 36
				cpu_duracion_pulso_interrupcion=36;				
                break;


								case MACHINE_ID_CHROME:
														poke_byte=poke_byte_chrome;
														peek_byte=peek_byte_chrome;
														peek_byte_no_time=peek_byte_no_time_chrome;
														poke_byte_no_time=poke_byte_no_time_chrome;
														lee_puerto=lee_puerto_spectrum;
														ay_chip_present.v=1;
								break;


								case MACHINE_ID_TSCONF:
								poke_byte=poke_byte_tsconf;
								peek_byte=peek_byte_tsconf;
								peek_byte_no_time=peek_byte_no_time_tsconf;
								poke_byte_no_time=poke_byte_no_time_tsconf;
								lee_puerto=lee_puerto_spectrum;
								ay_chip_present.v=1;

								//TSConf hacemos que active siempre realvideo (siempre que setting de autoactivar este yes)
								//por conveniencia, dado que se verá todo mejor asi que no con real video off
								if (autodetect_rainbow.v) enable_rainbow();

								break;

									case MACHINE_ID_BASECONF:
								poke_byte=poke_byte_baseconf;
								peek_byte=peek_byte_baseconf;
								peek_byte_no_time=peek_byte_no_time_baseconf;
								poke_byte_no_time=poke_byte_no_time_baseconf;
								lee_puerto=lee_puerto_spectrum;
								ay_chip_present.v=1;

								//baseConf hacemos que active siempre realvideo (siempre que setting de autoactivar este yes)
								//por conveniencia, dado que se verá todo mejor asi que no con real video off
								//if (autodetect_rainbow.v) enable_rainbow();

								break;

		case MACHINE_ID_COLECO:
                poke_byte=poke_byte_coleco;
                peek_byte=peek_byte_coleco;
				peek_byte_no_time=peek_byte_no_time_coleco;
				poke_byte_no_time=poke_byte_no_time_coleco;
                lee_puerto=lee_puerto_coleco;
				out_port=out_port_coleco;
				fetch_opcode=fetch_opcode_coleco;
				sn_chip_present.v=1;
        break;

		case MACHINE_ID_SG1000:
                poke_byte=poke_byte_sg1000;
                peek_byte=peek_byte_sg1000;
				peek_byte_no_time=peek_byte_no_time_sg1000;
				poke_byte_no_time=poke_byte_no_time_sg1000;
                lee_puerto=lee_puerto_sg1000;
				out_port=out_port_sg1000;
				fetch_opcode=fetch_opcode_sg1000;
				sn_chip_present.v=1;
        break;

		case MACHINE_ID_SMS:
                poke_byte=poke_byte_sms;
                peek_byte=peek_byte_sms;
				peek_byte_no_time=peek_byte_no_time_sms;
				poke_byte_no_time=poke_byte_no_time_sms;
                lee_puerto=lee_puerto_sms;
				out_port=out_port_sms;
				fetch_opcode=fetch_opcode_sms;
				sn_chip_present.v=1;
        break;        

		case MACHINE_ID_MSX1:
                poke_byte=poke_byte_msx1;
                peek_byte=peek_byte_msx1;
				peek_byte_no_time=peek_byte_no_time_msx1;
				poke_byte_no_time=poke_byte_no_time_msx1;
                lee_puerto=lee_puerto_msx1;
				out_port=out_port_msx1;
				fetch_opcode=fetch_opcode_msx;
				ay_chip_present.v=1;
				ay_chip_selected=0;
				total_ay_chips=1;
        break;


		case MACHINE_ID_SVI_318:
		case MACHINE_ID_SVI_328:
                poke_byte=poke_byte_svi;
                peek_byte=peek_byte_svi;
				peek_byte_no_time=peek_byte_no_time_svi;
				poke_byte_no_time=poke_byte_no_time_svi;
                lee_puerto=lee_puerto_svi;
				out_port=out_port_svi;
				fetch_opcode=fetch_opcode_svi;
				ay_chip_present.v=1;
				ay_chip_selected=0;
				total_ay_chips=1;
        break;


		case 120:
                poke_byte=poke_byte_zx80;
                peek_byte=peek_byte_zx80;
		peek_byte_no_time=peek_byte_zx80_no_time;
		poke_byte_no_time=poke_byte_zx80_no_time;
		lee_puerto=lee_puerto_zx80;


		out_port=out_port_zx80;

		nmi_generator_active.v=0;
		hsync_generator_active.v=0;

		debug_printf (VERBOSE_INFO,"Emulating ZX80 with %d KB (ramtop=%d)",(ramtop_zx8081-16383)/1024,ramtop_zx8081);
		//printf ("ramtop: %d\n",ramtop_zx8081);

		screen_testados_linea=207;
                //offset_zx8081_t_estados=0;

		fetch_opcode=fetch_opcode_zx81;



		//video_zx8081_lnctr_adjust.v=0;



		break;

		case 121:
                poke_byte=poke_byte_zx80;
                peek_byte=peek_byte_zx80;
		peek_byte_no_time=peek_byte_zx80_no_time;
		poke_byte_no_time=poke_byte_zx80_no_time;

		lee_puerto=lee_puerto_zx81;

		out_port=out_port_zx81;

		nmi_generator_active.v=0;
		hsync_generator_active.v=0;

		debug_printf (VERBOSE_INFO,"Emulating ZX81 with %d KB (ramtop=%d)",(ramtop_zx8081-16383)/1024,ramtop_zx8081);
		//printf ("ramtop: %d\n",ramtop_zx8081);
		screen_testados_linea=207;

                //offset_zx8081_t_estados=0;

		fetch_opcode=fetch_opcode_zx81;


		//video_zx8081_lnctr_adjust.v=1;

		break;

		case 122:
		//Jupiter Ace
                poke_byte=poke_byte_ace;
                peek_byte=peek_byte_ace;
                peek_byte_no_time=peek_byte_ace_no_time;
                poke_byte_no_time=poke_byte_ace_no_time;

                lee_puerto=lee_puerto_ace;

                out_port=out_port_ace;

                //nmi_generator_active.v=0;
                //hsync_generator_active.v=0;

                debug_printf (VERBOSE_INFO,"Emulating Jupiter Ace with %d KB (ramtop=%d)",(ramtop_ace-16383)/1024+3,ramtop_ace);
                screen_testados_linea=208;

                //offset_zx8081_t_estados=0;

                fetch_opcode=fetch_opcode_ace;



                break;


                case 130:
                poke_byte=poke_byte_z88;
                peek_byte=peek_byte_z88;
                peek_byte_no_time=peek_byte_no_time_z88;
                poke_byte_no_time=poke_byte_no_time_z88;
                lee_puerto=lee_puerto_z88;
		out_port=out_port_z88;

		init_z88_memory_slots();

		disable_rainbow();

		//zx81
		//screen_testados_linea=207;
		//spectrum 3.5 Mhz
		//screen_testados_linea=224;

		//Z88 3,2768 MHz
		screen_testados_linea=210;




		break;


		//CPC464
                case MACHINE_ID_CPC_464:
                poke_byte=poke_byte_cpc;
                peek_byte=peek_byte_cpc;
                peek_byte_no_time=peek_byte_no_time_cpc;
                poke_byte_no_time=poke_byte_no_time_cpc;
                lee_puerto=lee_puerto_cpc;
		out_port=out_port_cpc;
                ay_chip_present.v=1;
                fetch_opcode=fetch_opcode_cpc;
		//4Mhz
		screen_testados_linea=256;

		//temp
		//screen_testados_linea=228;

                //CBA Stereo
                ay3_stereo_mode=4;

                break;


		//CPC4128
                case MACHINE_ID_CPC_4128:
                poke_byte=poke_byte_cpc;
                peek_byte=peek_byte_cpc;
                peek_byte_no_time=peek_byte_no_time_cpc;
                poke_byte_no_time=poke_byte_no_time_cpc;
                lee_puerto=lee_puerto_cpc;
		out_port=out_port_cpc;
                ay_chip_present.v=1;
                fetch_opcode=fetch_opcode_cpc;
		//4Mhz
		screen_testados_linea=256;

		//temp
		//screen_testados_linea=228;
                break;

		case 150:
                poke_byte=poke_byte_sam;
                peek_byte=peek_byte_sam;
                peek_byte_no_time=peek_byte_no_time_sam;
                poke_byte_no_time=poke_byte_no_time_sam;
                lee_puerto=lee_puerto_sam;
                out_port=out_port_sam;
                fetch_opcode=fetch_opcode_sam;
		screen_testados_linea=384; //6 MHZ aprox
		ay_chip_present.v=1; //Simulacion del chip SAA mediante el AY

		ay_chip_selected=0;
                total_ay_chips=2;


                break;

		case MACHINE_ID_QL_STANDARD: //QL 160


		poke_byte=poke_byte_legacy_ql;
		peek_byte=peek_byte_legacy_ql;
		peek_byte_no_time=peek_byte_no_time_legacy_ql;
		poke_byte_no_time=poke_byte_no_time_legacy_ql;
		lee_puerto=lee_puerto_legacy_ql;
		out_port=out_port_legacy_ql;
		fetch_opcode=fetch_opcode_legacy_ql;

		ql_readbyte_no_ports_function=ql_readbyte_no_ports;


							//Hagamoslo mas lento
								screen_testados_linea=80;

            i8049_chip_present=1;
		break;


		case MACHINE_ID_MK14_STANDARD:
		poke_byte=poke_byte_mk14;
		peek_byte=peek_byte_mk14;
		peek_byte_no_time=peek_byte_no_time_mk14;
		poke_byte_no_time=poke_byte_no_time_mk14;
		lee_puerto=lee_puerto_legacy_mk14;
		break;




		default:
			//printf ("Init Machine id %d not supported. Exiting\n",current_machine_type);
			sprintf (mensaje_error,"Init Machine id %d not supported. Exiting",current_machine_type);
			cpu_panic(mensaje_error);
		break;
	}


	if (MACHINE_IS_SPECTRUM) {
                //Activar deteccion automatica de rutina de impresion de caracteres, si conviene
                if (chardetect_detect_char_enabled.v) {
                        chardetect_init_automatic_char_detection();
                }

		//Reactivar poke de spectra
		if (spectra_enabled.v) spectra_set_poke();

	}


	debug_printf(VERBOSE_INFO,"Setting machine %s",get_machine_name(current_machine_type));

	//Recalcular algunos valores cacheados
	recalcular_get_total_ancho_rainbow();
	recalcular_get_total_alto_rainbow();



}

void set_menu_gui_zoom(void)
{
	//Ajustar zoom del gui. por defecto 1
	menu_gui_zoom=1;
	//printf ("calling set_menu_gui_zoom. driver: %s\n",scr_new_driver_name);

	if (si_complete_video_driver() ) {
		if (MACHINE_IS_QL || MACHINE_IS_TSCONF || MACHINE_IS_CPC || MACHINE_IS_PRISM || MACHINE_IS_SAM || MACHINE_IS_TBBLUE) menu_gui_zoom=2;
	}

	debug_printf (VERBOSE_INFO,"Setting GUI menu zoom to %d",menu_gui_zoom);
}

void post_set_mach_reopen_screen(void)
{
				debug_printf(VERBOSE_INFO,"End Screen");
			scr_end_pantalla();
			debug_printf(VERBOSE_INFO,"Creating Screen");
			screen_init_pantalla_and_others_and_realjoystick();

			//scr_init_pantalla();
}

void set_last_dimensiones_ventana(void)
{
    last_ancho_ventana=screen_get_total_width_window_plus_zxdesktop();
    last_alto_ventana=screen_get_total_height_window_no_footer_plus_zxdesktop();

    //printf("Setting antes_ancho_total %d antes_alto_total %d\n",last_ancho_ventana,last_alto_ventana);
}

//Reabrir ventana en caso de que maquina seleccionada sea diferente a la anterior
void post_set_machine_no_rom_load_reopen_window(void)
{

    int antes_menu_gui_zoom=menu_gui_zoom;

	set_menu_gui_zoom();

	//printf ("last: %d current: %d\n",last_machine_type,current_machine_type);

	if (last_machine_type!=255 && last_machine_type!=current_machine_type) {
		debug_printf (VERBOSE_INFO,"Reopening window so current machine is different and may have different window size");
		//printf ("Reopening window so current machine is different and may have different window size\n");
		post_set_mach_reopen_screen();

		//Rearrange de ventanas en segundo plano, por si la maquina actual es una ventana de ZEsarUX mas pequeña 
		//y se saldrian las ventanas zxvision de rango
		//debug_printf (VERBOSE_DEBUG,"Rearrange zxvision windows so current machine is different and may have different window size");
		//zxvision_rearrange_background_windows(		


        //printf("antes_ancho_total %d antes_alto_total %d ancho actual %d alto actual %d\n",last_ancho_ventana,last_alto_ventana,
        //    screen_get_total_width_window_plus_zxdesktop(),screen_get_total_height_window_no_footer_plus_zxdesktop() );

        if (screen_get_total_width_window_plus_zxdesktop() < last_ancho_ventana || 
            screen_get_total_height_window_no_footer_plus_zxdesktop() < last_alto_ventana ||
            antes_menu_gui_zoom !=menu_gui_zoom
            ) {

            debug_printf (VERBOSE_DEBUG,"Rearrange zxvision windows so current machine has smaller window size or gui zoom different");

            //printf ("Rearrange zxvision windows so current machine has smaller window size or gui zoom different\n");
            zxvision_rearrange_background_windows();
        }

        zxvision_check_all_configurable_icons_positions();

    }

    set_last_dimensiones_ventana();
        
}

/*
Vieja funcion
Reabrir ventana en caso de que maquina seleccionada tenga tamanyo diferente que la anterior
TODO: Quiza se podria simplificar esto, se empezó con Z88 a spectrum y se han ido agregando,
se exponen todos los casos de maquinas con diferentes tamanyos de ventana,
pero quiza simplemente habria que ver que el tamanyo anterior fuera diferente al actual y entonces reabrir ventana
O que la maquina actual es diferente de la anterior
*/
void old_post_set_machine_no_rom_load_reopen_window(void)
{

	set_menu_gui_zoom();


	//si se cambia de maquina Z88 o a maquina Z88, redimensionar ventana
	if (last_machine_type!=255) {

		if ( (MACHINE_IS_Z88 && last_machine_type!=130)  || (last_machine_type==130 && !(MACHINE_IS_Z88) ) ) {
			debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing Z88 to/from other machine)");
			post_set_mach_reopen_screen();
			return;
		}
	}


	//si se cambia de maquina CPC o a maquina CPC, redimensionar ventana

							if (last_machine_type!=255) {

											if ( (MACHINE_IS_CPC && !(last_machine_type>=140 && last_machine_type<=149) )  || (  (last_machine_type>=140 && last_machine_type<=149) && !(MACHINE_IS_CPC) ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing CPC to/from other machine)");

															post_set_mach_reopen_screen();
															return;
											}
							}

							//si se cambia de maquina SAM o a maquina SAM, redimensionar ventana

							if (last_machine_type!=255) {

											if ( (MACHINE_IS_SAM && !(last_machine_type>=150 && last_machine_type<=159) )  || (  (last_machine_type>=150 && last_machine_type<=159) && !(MACHINE_IS_SAM) ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing SAM to/from other machine)");

															post_set_mach_reopen_screen();
															return;
											}
							}

							//si se cambia de maquina QL o a maquina QL, redimensionar ventana

							if (last_machine_type!=255) {

											if ( (MACHINE_IS_QL && !(last_machine_type>=160 && last_machine_type<=179) )  || (  (last_machine_type>=160 && last_machine_type<=179) && !(MACHINE_IS_QL) ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing QL to/from other machine)");

															post_set_mach_reopen_screen();
															return;
											}
							}



							//si se cambia de maquina Prism o a maquina Prism, redimensionar ventana

							if (last_machine_type!=255) {

											if ( (MACHINE_IS_PRISM && last_machine_type!=18)   || (last_machine_type==18 && !(MACHINE_IS_PRISM)  ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing PRISM to/from other machine)");

															post_set_mach_reopen_screen();
															return;
											}
							}

							//si se cambia de maquina TBBLUE o a maquina TBBLUE, redimensionar ventana

							if (last_machine_type!=255) {

											if ( (MACHINE_IS_TBBLUE && last_machine_type!=MACHINE_ID_TBBLUE)   || (last_machine_type==MACHINE_ID_TBBLUE && !(MACHINE_IS_TBBLUE)  ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing TBBLUE to/from other machine)");

															post_set_mach_reopen_screen();
															return;
											}
							}

							//si se cambia de maquina TSconf o a maquina tsconf, redimensionar ventana

							if (last_machine_type!=255) {

											if ( (MACHINE_IS_TSCONF && last_machine_type!=MACHINE_ID_TSCONF)   || (last_machine_type==MACHINE_ID_TSCONF && !(MACHINE_IS_TSCONF)  ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing TSCONF to/from other machine)");

															post_set_mach_reopen_screen();
															return;
											}
							}


							//si se cambia de maquina MK14 o a maquina mk14, redimensionar ventana

							if (last_machine_type!=255) {

											if ( (MACHINE_IS_MK14 && last_machine_type!=MACHINE_ID_MK14_STANDARD)   || (last_machine_type==MACHINE_ID_MK14_STANDARD && !(MACHINE_IS_MK14)  ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing MK14 to/from other machine)");

															post_set_mach_reopen_screen();
															return;
											}
							}

							//Si se cambia de maquina Spectrum o a maquina Spectrum - afecta especialmente a spectrum
							if (last_machine_type!=255) {

											if ( (MACHINE_IS_SPECTRUM && !(last_machine_type<30) )  || (  (last_machine_type<30) && !(MACHINE_IS_SPECTRUM) ) ) {
															debug_printf (VERBOSE_INFO,"Reopening window so machine has different size (changing Spectrum to/from other machine)");
															post_set_mach_reopen_screen();
															return;
											}
							}

}

void post_set_machine_no_rom_load(void)
{

		screen_set_video_params_indices();
		inicializa_tabla_contend();

		init_rainbow();
		init_cache_putpixel();


		post_set_machine_no_rom_load_reopen_window();

		//printf ("antes init layers\n");
		scr_init_layers_menu();
		//printf ("despues init layers\n");
		scr_clear_layer_menu();		


		last_machine_type=current_machine_type;
		menu_init_footer();



}

void post_set_machine(char *romfile)
{

                //leer rom
                debug_printf (VERBOSE_INFO,"Loading ROM");
                rom_load(romfile);

		post_set_machine_no_rom_load();
}


void set_machine(char *romfile)
{

	//Si estaba divmmc o divide activo, desactivarlos
	//if (divmmc_enabled.v) divmmc_disable();
	//if (divide_enabled.v) divide_disable();
	if (diviface_enabled.v) diviface_disable();

                //Si se cambia de maquina zxuno a otra no zxuno, desactivar divmmc
		/*
                if (last_machine_type!=255) {
                        if (last_machine_type==14 && machine_type!=14 && divmmc_enabled.v) {
                                debug_printf (VERBOSE_INFO,"Disabling divmmc because it was enabled on ZX-Uno");
                                divmmc_disable();
                        }
                }
		*/



	set_machine_params();
	malloc_mem_machine();

	//Si hay activo divmmc o divide, reactivar. Se pone aqui porque en caso de zxuno es necesario que esten inicializadas las paginas de memoria
	//if (divmmc_enabled.v) divmmc_enable();
	//if (divide_enabled.v) divide_enable();

	post_set_machine(romfile);
}



//Punto de entrada para error al cargar rom de cualquier modelo de spectrum, dado que se lee longitud de la rom diferente de la esperada
//panic
//void rom_load_cpu_panic(char *romfilename,int leidos)
//{
//	debug_printf(VERBOSE_ERR,"Error loading ROM");
//}


void rom_load(char *romfilename)
{
                FILE *ptr_romfile;
		int leidos;

	char mensaje_error[200];


	if (romfilename==NULL) {
		switch (current_machine_type) {
                case 0:
                romfilename="48.rom";
                break;

                case 1:
                case MACHINE_ID_SPECTRUM_48_PLUS_ENG:
                romfilename="48.rom";
                break;

                case 2:
                romfilename="inves.rom";
                break;

                case 3:
                romfilename="tk90x.rom";
                break;

                case 4:
                romfilename="tk90xs.rom";
                break;

                case 5:
                romfilename="tk95.rom";
                break;




                case 6:
                romfilename="128.rom";
                break;

                case 7:
                romfilename="128s.rom";
                break;




                case 8:
                romfilename="p2.rom";
                break;

                case 9:
                romfilename="p2f.rom";
                break;

                case 10:
                romfilename="p2s.rom";
                break;



                case 11:
                case MACHINE_ID_SPECTRUM_P3_40:
                romfilename="p2a40.rom";
                break;

                case MACHINE_ID_SPECTRUM_P3_41:
                case 12:
                romfilename="p2a41.rom";
                break;

                case MACHINE_ID_SPECTRUM_P3_SPA:
                case 13:
                romfilename="p2as.rom";
                break;


		case 14:
		romfilename="zxuno_bootloader.rom";
		break;

		case 15:
		case 16:
		romfilename="se.rom";
		break;


		case MACHINE_ID_TIMEX_TS2068:
        case MACHINE_ID_TIMEX_TC2068:
		romfilename="ts2068.rom";
		break;

		case 18:
		romfilename="prism.rom";
		break;

		case 19:
		if (tbblue_fast_boot_mode.v) romfilename="48.rom";
		else romfilename="tbblue_loader.rom";
		break;

                case 20:
                romfilename="48es.rom";
                break;

                case 21:
                romfilename="pentagon.rom";
                break;

                case MACHINE_ID_COLECO:
                romfilename="coleco.rom";
                break;	

                case MACHINE_ID_SG1000:
                romfilename="sg1000.rom"; 
                break;		

                case MACHINE_ID_SMS:
                romfilename="sms.rom"; 
                break;	                					
                
                case MACHINE_ID_MSX1:
                romfilename="msx.rom";
                break;

                case MACHINE_ID_SVI_318:
				case MACHINE_ID_SVI_328:
                romfilename="svi.rom";
                break;				

								case MACHINE_ID_CHROME:
								romfilename="chrome.rom";
								break;

								case MACHINE_ID_TSCONF:
								romfilename="zxevo_tsconf.rom";
								break;

								case MACHINE_ID_BASECONF:
								romfilename="zxevo_baseconf.rom";
								break;

                case MACHINE_ID_TIMEX_TC2048:
                romfilename="tc2048.rom";
                break;

                case 120:
                romfilename="zx80.rom";

                break;

                case 121:

                romfilename="zx81.rom";

		break;

                case 122:

                romfilename="ace.rom";

                break;

		case 130:

		romfilename="Z88OZ47.rom";


		break;


		case MACHINE_ID_CPC_464:
		romfilename="cpc464.rom";
		break;

		case MACHINE_ID_CPC_4128:
		romfilename="cpc464.rom";
		break;		

		case 150:
		if (atomlite_enabled.v) romfilename="atomlite.rom";
		else romfilename="samcoupe.rom";
		break;

		case MACHINE_ID_QL_STANDARD:
		romfilename="ql_js.rom";

		//romfilename="MIN189.rom";
		//romfilename="ql_jm.rom";
		break;

		case MACHINE_ID_MK14_STANDARD:
		romfilename="mk14.rom";
		break;

                        default:
                                //printf ("ROM for Machine id %d not supported. Exiting\n",machine_type);
                                sprintf (mensaje_error,"ROM for Machine id %d not supported. Exiting",current_machine_type);
				cpu_panic(mensaje_error);
                        break;



		}
	}

		open_sharedfile(romfilename,&ptr_romfile);
                if (!ptr_romfile)
                {
                    debug_printf(VERBOSE_ERR,"Unable to open rom file %s",romfilename);

					//No hacemos mas un panic por esto. Ayuda a debugar posibles problemas con el path de inicio
					//cpu_panic("Unable to open rom file");

					return;
                }

		//Caso Inves. ROM esta en el final de la memoria asignada
		if (MACHINE_IS_INVES) {
			//Inves
			leidos=fread(&memoria_spectrum[65536],1,16384,ptr_romfile);
                                if (leidos!=16384) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
                                }
                }


		else if (MACHINE_IS_SPECTRUM_16_48) {
			//Spectrum 16k rom
                        	leidos=fread(memoria_spectrum,1,16384,ptr_romfile);
				if (leidos!=16384) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
				}
		}

		else if (MACHINE_IS_SPECTRUM_128_P2) {
			//Spectrum 32k rom

                                leidos=fread(rom_mem_table[0],1,32768,ptr_romfile);
                                if (leidos!=32768) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
	                         }

		}

		else if (MACHINE_IS_SPECTRUM_P2A_P3) {

			//Spectrum 64k rom

                                leidos=fread(rom_mem_table[0],1,65536,ptr_romfile);
                                if (leidos!=65536) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }
		}

		else if (MACHINE_IS_ZXUNO) {
			//107 bytes rom
                        //leidos=fread(memoria_spectrum,1,56,ptr_romfile);
                        //if (leidos!=56) {
                        //                debug_printf(VERBOSE_ERR,"Error loading ROM");
                        //}

			//Max 8kb rom
                        leidos=fread(memoria_spectrum,1,8192,ptr_romfile);
			//Un minimo de rom...
                        if (leidos<1) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                        }

			debug_printf (VERBOSE_DEBUG,"Read %d bytes of rom file %s",leidos,romfilename);

			zxuno_load_spi_flash();


            zxuno_load_additional_64k_block();
		}

	       else if (MACHINE_IS_CHLOE) {
                        //SE Basic IV 32k rom

                                leidos=fread(chloe_rom_mem_table[0],1,32768,ptr_romfile);
                                if (leidos!=32768) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }

                }

               else if (MACHINE_IS_PRISM) {
                        //320k rom
/*
ROM page	file	size
0		48.rom					16k
1		48.rom					16k
2		48.rom					16k
3		48.rom					16k
4		se.rom					32k
6		se.rom  				32k
8		128.rom 				32k
10		128.rom 				32k
12		128.rom 				32k
14		128.rom 				32k
16		alternaterom_plus3e_mmcen3eE.rom	64k
20 end

Total 20 pages=320 Kb
*/

                                leidos=fread(prism_rom_mem_table[0],1,320*1024,ptr_romfile);
                                if (leidos!=320*1024) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }


				prism_load_failsafe_rom();

                }

		else if (MACHINE_IS_TBBLUE) {
			
			//Cargamos solo la rom de 48 4 veces 
			if (tbblue_fast_boot_mode.v) {
			leidos=fread(&memoria_spectrum[0],1,16384,ptr_romfile);
			memcpy(&memoria_spectrum[16384],&memoria_spectrum[0],16384);
			memcpy(&memoria_spectrum[32768],&memoria_spectrum[0],16384);
			memcpy(&memoria_spectrum[49152],&memoria_spectrum[0],16384);
			
                                if (leidos!=16384) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }				
			}
			else {
				leidos=fread(tbblue_fpga_rom,1,8192,ptr_romfile);
				memcpy(&tbblue_fpga_rom[8192],tbblue_fpga_rom,8192);
                                if (leidos!=8192) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }
			}

                }

								else if (MACHINE_IS_CHROME) {
									//160 K RAM, 64 K ROM
									leidos=fread(chrome_rom_mem_table[0],1,65536,ptr_romfile);
									if (leidos!=65536) {
									     debug_printf(VERBOSE_ERR,"Error loading ROM");
									}

								}

								else if (MACHINE_IS_TSCONF) {
									leidos=fread(tsconf_rom_mem_table[0],1,512*1024,ptr_romfile);
									if (leidos!=512*1024) {
											debug_printf(VERBOSE_ERR,"Error loading ROM");
									}

								}

			else if (MACHINE_IS_BASECONF) {
									leidos=fread(baseconf_rom_mem_table[0],1,512*1024,ptr_romfile);
									if (leidos!=512*1024) {
											debug_printf(VERBOSE_ERR,"Error loading ROM");
									}

								}


              else if (MACHINE_IS_TIMEX_T2068) {

                                leidos=fread(timex_rom_mem_table[0],1,16384,ptr_romfile);
                                if (leidos!=16384) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }

                                leidos=fread(timex_ex_rom_mem_table[0],1,8192,ptr_romfile);
                                if (leidos!=8192) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }



                }

                else if (MACHINE_IS_COLECO) {
			//coleco 8 kb rom
                        	leidos=fread(memoria_spectrum,1,8192,ptr_romfile);
				if (leidos!=8192) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
				}
		}				
                
                else if (MACHINE_IS_SG1000) {
					//no tiene rom. No cargamos nada, aunque mas arriba intenta siempre abrir un archivo de rom,
					//es por eso que es necesario que exista el archivo de rom, aunque no se cargue ni se use para nada
			
		}	

                else if (MACHINE_IS_SMS) {
                        	leidos=fread(memoria_spectrum,1,8192,ptr_romfile);
				if (leidos!=8192) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
				}
		}	        

                else if (MACHINE_IS_MSX1) {
			//msx 32 kb rom
                        	leidos=fread(memoria_spectrum,1,32768,ptr_romfile);
				if (leidos!=32768) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
				}
		}


                else if (MACHINE_IS_SVI) {
			//svi 32 kb rom
                        	leidos=fread(memoria_spectrum,1,32768,ptr_romfile);
				if (leidos!=32768) {
				 	debug_printf(VERBOSE_ERR,"Error loading ROM");
				}
		}


		else if (MACHINE_IS_ZX80) {
			//ZX80
                                leidos=fread(memoria_spectrum,1,4096,ptr_romfile);
                                if (leidos!=4096) {
					debug_printf(VERBOSE_ERR,"Error loading ROM");
                                }
		}


		else if (MACHINE_IS_ZX81) {

			//ZX81
                                leidos=fread(memoria_spectrum,1,8192,ptr_romfile);
                                if (leidos!=8192) {
					debug_printf(VERBOSE_ERR,"Error loading ROM");
                                }
		}

                else if (MACHINE_IS_ACE) {

                        //Jupiter Ace
                                leidos=fread(memoria_spectrum,1,8192,ptr_romfile);
                                if (leidos!=8192) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                }
                }


		else if (MACHINE_IS_Z88) {
			//Z88
			//leer maximo 512 kb de ROM
			leidos=fread(z88_puntero_memoria,1,512*1024,ptr_romfile);
                                if (leidos<=0) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                }

			z88_internal_rom_size=leidos-1;

		}


	      else if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) {
                        //32k rom

                                leidos=fread(cpc_rom_mem_table[0],1,32768,ptr_romfile);
                                if (leidos!=32768) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }

                }


              else if (MACHINE_IS_SAM) {
                        //32k rom

                                leidos=fread(sam_rom_memory[0],1,32768,ptr_romfile);
                                if (leidos!=32768) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }

                }

		else if (MACHINE_IS_QL) {
			//minimo 16kb,maximo 128k
			        leidos=fread(memoria_ql,1,131072,ptr_romfile);

																//Minimo 16kb
                                if (leidos<16384) {
                                        debug_printf(VERBOSE_ERR,"Error loading ROM");
                                 }

		}


		else if (MACHINE_IS_MK14) {
			leidos=fread(memoria_spectrum,1,512,ptr_romfile);
			if (leidos!=512) {
					debug_printf(VERBOSE_ERR,"Error loading ROM");
			}

		}


                fclose(ptr_romfile);

}


//Para Windows con pthreads. En todos los sistemas, se permite main loop en pthread, excepto en Windows
#ifdef USE_PTHREADS
int si_thread_main_loop;


void ver_si_enable_thread_main_loop(void)
{

#ifdef MINGW
        si_thread_main_loop=0;
#else
        si_thread_main_loop=1;
#endif

}

#endif



void segint_signal_handler(int sig)
{

	debug_printf (VERBOSE_INFO,"Sigint (CTRL+C) received");

        //para evitar warnings al compilar
        sig++;

//Primero de todo detener el pthread del emulador, que no queremos que siga activo el emulador con el pthread de fondo mientras
//se ejecuta el end_emulator
#ifdef USE_PTHREADS
        if (si_thread_main_loop) {
        	debug_printf (VERBOSE_INFO,"Ending main loop thread");
		pthread_cancel(thread_main_loop);
	}
#endif

    //salir sin fadeout, no queremos hacerlo en caso de salir desde aqui para que sea mas rapido
    //nota: dado que estamos saliendo sin guardar config, podemos alterar parametros de configuracion
    //desde aqui sin que se graben en el archivo de configuración
    quickexit.v=1;

    //salir sin guardar config
	end_emulator_saveornot_config(0);


}

void segterm_signal_handler(int sig)
{

        debug_printf (VERBOSE_INFO,"Sigterm received");

        //para evitar warnings al compilar
        sig++;

//Primero de todo detener el pthread del emulador, que no queremos que siga activo el emulador con el pthread de fondo mientras
//se ejecuta el end_emulator
#ifdef USE_PTHREADS
        if (si_thread_main_loop) {
        	debug_printf (VERBOSE_INFO,"Ending main loop thread");
		pthread_cancel(thread_main_loop);
	}
#endif


        //salir sin guardar config
        end_emulator_saveornot_config(0);


}



void segfault_signal_handler(int sig)
{
	//para evitar warnings al compilar
	sig++;

	cpu_panic("Segmentation fault");
}


void sigbus_signal_handler(int sig)
{
	//Saltara por ejemplo si empezamos a escribir en un puntero que no se ha inicializado
	//para evitar warnings al compilar
	sig++;

	cpu_panic("Bus error");
}


void sigpipe_signal_handler(int sig)
{
	//Saltara por ejemplo cuando se escribe en un socket que se ha cerrado
	//para evitar warnings al compilar
	sig++;
	
	debug_printf (VERBOSE_DEBUG,"Received signal sigpipe");


}


void floatingpoint_signal_handler(int sig)
{
        //para evitar warnings al compilar
        sig++;

        cpu_panic("Floating point exception");
}


void main_init_video(void)
{
		//Video init
                debug_printf (VERBOSE_INFO,"Initializing Video Driver");

                //gestion de fallback y con driver indicado

                //scr_init_pantalla=NULL;

#ifdef USE_COCOA
                add_scr_init_array("cocoa",scrcocoa_init,set_scrdriver_cocoa);
                if (!strcmp(driver_screen,"cocoa")) {
                        set_scrdriver_cocoa();
                }
#endif


#ifdef COMPILE_XWINDOWS
                add_scr_init_array("xwindows",scrxwindows_init,set_scrdriver_xwindows);
                if (!strcmp(driver_screen,"xwindows")) {
                        set_scrdriver_xwindows();
                }
#endif


#ifdef COMPILE_SDL
                add_scr_init_array("sdl",scrsdl_init,set_scrdriver_sdl);
                if (!strcmp(driver_screen,"sdl")) {
                        set_scrdriver_sdl();
                }
#endif



#ifdef COMPILE_FBDEV
		add_scr_init_array("fbdev",scrfbdev_init,set_scrdriver_fbdev);
                if (!strcmp(driver_screen,"fbdev")) {
                        set_scrdriver_fbdev();
		}
#endif

#ifdef COMPILE_CACA
                add_scr_init_array("caca",scrcaca_init,set_scrdriver_caca);
                if (!strcmp(driver_screen,"caca")) {
                        set_scrdriver_caca();
                }
#endif

#ifdef COMPILE_AA
                add_scr_init_array("aa",scraa_init,set_scrdriver_aa);
                if (!strcmp(driver_screen,"aa")) {
                        set_scrdriver_aa();
                }
#endif


#ifdef COMPILE_CURSES
                add_scr_init_array("curses",scrcurses_init,set_scrdriver_curses);

                if (!strcmp(driver_screen,"curses")) {
                        set_scrdriver_curses();
                }
#endif

#ifdef COMPILE_STDOUT
                add_scr_init_array("stdout",scrstdout_init,set_scrdriver_stdout);
                if (!strcmp(driver_screen,"stdout")) {
                        set_scrdriver_stdout();
                }
#endif

#ifdef COMPILE_SIMPLETEXT
                add_scr_init_array("simpletext",scrsimpletext_init,set_scrdriver_simpletext);
                if (!strcmp(driver_screen,"simpletext")) {
                        set_scrdriver_simpletext();
                }
#endif


                //Y finalmente null video driver
                add_scr_init_array("null",scrnull_init,set_scrdriver_null);
                if (!strcmp(driver_screen,"null")) {
                        set_scrdriver_null();
                }



                if (try_fallback_video.v==1) {
                        //probar drivers de video
                        do_fallback_video();
                }

                //no probar. Inicializar driver indicado. Si falla, fallback a null
                else {
                        if (screen_init_pantalla_and_others() ) {
                                debug_printf (VERBOSE_ERR,"Error using video output driver %s. Fallback to null",driver_screen);
				set_scrdriver_null();
				screen_init_pantalla_and_others();
                        }
                }


}

void main_init_audio(void)
{
		debug_printf (VERBOSE_INFO,"Initializing Audio");
                //Audio init
                audio_init=NULL;
                audio_buffer_switch.v=0;

                interrupt_finish_sound.v=0;
                audio_playing.v=1;

                audio_buffer_one=audio_buffer_one_assigned;
                audio_buffer_two=audio_buffer_two_assigned;

                set_active_audio_buffer();

		audio_empty_buffer();

                //gestion de fallback y con driver indicado.

#ifdef COMPILE_PULSE
                add_audio_init_array("pulse",audiopulse_init,set_audiodriver_pulse);
                if (!strcmp(driver_audio,"pulse")) {
                        set_audiodriver_pulse();

                }
#endif


#ifdef COMPILE_ALSA
                add_audio_init_array("alsa",audioalsa_init,set_audiodriver_alsa);
                if (!strcmp(driver_audio,"alsa")) {
                        set_audiodriver_alsa();

                }
#endif


#ifdef COMPILE_COREAUDIO
                add_audio_init_array("coreaudio",audiocoreaudio_init,set_audiodriver_coreaudio);
                if (!strcmp(driver_audio,"coreaudio")) {
                        set_audiodriver_coreaudio();

                }
#endif


#ifdef COMPILE_SDL
                add_audio_init_array("sdl",audiosdl_init,set_audiodriver_sdl);
                if (!strcmp(driver_audio,"sdl")) {
                        set_audiodriver_sdl();

                }
#endif


#ifdef COMPILE_DSP
                add_audio_init_array("dsp",audiodsp_init,set_audiodriver_dsp);
                if (!strcmp(driver_audio,"dsp")) {
                        set_audiodriver_dsp();

                }
#endif

#ifdef COMPILE_ONEBITSPEAKER
                add_audio_init_array("onebitspeaker",audioonebitspeaker_init,set_audiodriver_onebitspeaker);
                if (!strcmp(driver_audio,"onebitspeaker")) {
                        set_audiodriver_onebitspeaker();

                }
#endif




                //Y finalmente null audio driver
                add_audio_init_array("null",audionull_init,set_audiodriver_null);
                if (!strcmp(driver_audio,"null")) {
                        set_audiodriver_null();

                }





                if (try_fallback_audio.v==1) {
                        //probar drivers de audio
                        do_fallback_audio();
                }

                //no probar. Inicializar driver indicado. Si falla, fallback a null
                else {
                        if (audio_init()) {
				fallback_audio_null();
                        }
                }

}

char *param_custom_romfile=NULL;
z80_bit opcion_no_welcome_message;


z80_bit command_line_zx8081_vsync_sound={0};
z80_bit command_line_wrx={0};
z80_bit command_line_spectra={0};
z80_bit command_line_timex_video={0};
z80_bit command_line_spritechip={0};
z80_bit command_line_ulaplus={0};
z80_bit command_line_gigascreen={0};
z80_bit command_line_16c={0};
z80_bit command_line_interlaced={0};
z80_bit command_line_chroma81={0};
z80_bit command_line_zxpand={0};
z80_bit command_line_esxdos_handler={0};
z80_bit command_line_mmc={0};
z80_bit command_line_zxmmc={0};
z80_bit command_line_divmmc={0};
z80_bit command_line_divmmc_ports={0};


z80_bit command_line_ide={0};
z80_bit command_line_divide={0};
z80_bit command_line_divide_ports={0};

z80_bit command_line_divide_paging={0};
z80_bit command_line_divmmc_paging={0};
z80_bit command_line_8bitide={0};

z80_bit command_line_dandanator={0};
z80_bit command_line_dandanator_push_button={0};
z80_bit command_line_superupgrade={0};
z80_bit command_line_kartusho={0};
z80_bit command_line_ifrom={0};
z80_bit command_line_betadisk={0};
z80_bit command_line_trd={0};
z80_bit command_line_dsk={0};
z80_bit command_line_hilow={0};
z80_bit command_line_load_source_code={0};
char *command_line_load_source_code_file;

z80_bit command_line_set_breakpoints={0};

z80_bit command_line_enable_midi={0};

int command_line_vsync_minimum_lenght=0;

char *command_line_load_binary_file=NULL;
int command_line_load_binary_address;
int command_line_load_binary_length;

char *command_line_esxdos_local_dir_path;
z80_bit command_line_esxdos_local_dir={0};

int command_line_chardetect_printchar_enabled=-1;


z80_bit added_some_osd_text_keyboard={0};



//cuantos botones-joystick a teclas definidas
int joystickkey_definidas=0;


//void parse_cmdline_options(int argc,char *argv[]) {
int parse_cmdline_options(void) {

		while (!siguiente_parametro()) {
			if (!strcmp(argv[puntero_parametro],"--help")) {
				cpu_help();
				exit(1);
			}

                        if (!strcmp(argv[puntero_parametro],"--experthelp")) {
                                cpu_help_expert();
                                exit(1);
                        }

			if (!strcmp(argv[puntero_parametro],"--helpcustomconfig")) {
				customconfig_help();
				exit(1);
			}


			if (!strcmp(argv[puntero_parametro],"--showcompileinfo")) {
                                show_compile_info();
                                exit(1);
                        }



			if (!strcmp(argv[puntero_parametro],"--debugconfigfile")) {
				//Este parametro aqui se ignora, solo se lee antes del parseo del archivo de configuracion
                        }

			else if (!strcmp(argv[puntero_parametro],"--noconfigfile")) {
                                //Este parametro aqui se ignora, solo se lee antes del parseo del archivo de configuracion
                        }

			else if (!strcmp(argv[puntero_parametro],"--configfile")) {
                                //Este parametro aqui se ignora, solo se lee antes del parseo del archivo de configuracion
					siguiente_parametro_argumento();
                        }						

			else if (!strcmp(argv[puntero_parametro],"--saveconf-on-exit")) {
				save_configuration_file_on_exit.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zoomx")) {
				siguiente_parametro_argumento();
				zoom_x=atoi(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--zoomy")) {
				siguiente_parametro_argumento();
				zoom_y=atoi(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--reduce-075")) {
				screen_reduce_075.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--reduce-075-no-antialias")) {
				screen_reduce_075_antialias.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--reduce-075-offset-x")) {
				siguiente_parametro_argumento();
				screen_reduce_offset_x=atoi(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--reduce-075-offset-y")) {
				siguiente_parametro_argumento();
				screen_reduce_offset_y=atoi(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--enable-watermark")) {
				screen_watermark_enabled.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enable-zxdesktop")) {
				screen_ext_desktop_enabled=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-width")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<128 || valor>9999) {
					printf ("Invalid value for ZX Desktop width\n");
					exit(1);
				}
				screen_ext_desktop_width=valor;
			}		

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-height")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<0 || valor>9999) {
					printf ("Invalid value for ZX Desktop height\n");
					exit(1);
				}
				screen_ext_desktop_height=valor;
			}	            

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-fill-type")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<0 || valor>MENU_MAX_EXT_DESKTOP_FILL_NUMBER) {
					printf ("Invalid value for ZX Desktop fill type\n");
					exit(1);
				}
				menu_ext_desktop_fill=valor;
			}		

			//Deprecated --zxdesktop-fill-solid-color
			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-fill-solid-color") ||
					!strcmp(argv[puntero_parametro],"--zxdesktop-fill-primary-color")
			
			) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<0 || valor>15) {
					printf ("Invalid value for ZX Desktop primary fill solid color\n");
					exit(1);
				}
				menu_ext_desktop_fill_first_color=valor;
			}		

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-fill-secondary-color")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<0 || valor>15) {
					printf ("Invalid value for ZX Desktop seconday fill solid color\n");
					exit(1);
				}
				menu_ext_desktop_fill_second_color=valor;
			}						

            else if (!strcmp(argv[puntero_parametro],"--zxdesktop-fill-degraded-inverted")) {
                menu_ext_desktop_degraded_inverted.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-new-items")) {
				screen_ext_desktop_place_menu=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-disable-buttons")) {
				menu_zxdesktop_buttons_enabled.v=0;
			} 

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-transparent-upper-buttons")) {
				menu_ext_desktop_transparent_upper_icons.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-transparent-lower-buttons")) {
				menu_ext_desktop_transparent_lower_icons.v=1;
			}

            //Esta opcion va al reves de otras de transparencia, porque por defecto estos iconos son transparentes
			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-no-transparent-configurable-icons")) {
				menu_ext_desktop_transparent_configurable_icons.v=0;
			}   

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-no-configurable-icons-text-bg")) {
				menu_ext_desktop_configurable_icons_text_background.v=0;
			}                       

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-disable-box-upper-buttons")) {
				menu_ext_desktop_disable_box_upper_icons.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-disable-box-lower-buttons")) {
				menu_ext_desktop_disable_box_lower_icons.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-disable-footer-switch")) {
				zxdesktop_switch_button_enabled.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-disable-on-fullscreen")) {
				zxdesktop_disable_on_full_screen=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-disable-frame-emulated-display")) {
				zxdesktop_disable_show_frame_around_display=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-scr-enable")) {
				zxdesktop_draw_scrfile_enabled=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-scr-file")) {
				siguiente_parametro_argumento();
                strcpy(zxdesktop_draw_scrfile_name,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-scr-centered")) {
				zxdesktop_draw_scrfile_centered=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-scr-fillscale")) {
				zxdesktop_draw_scrfile_fill_scale=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-scr-mixbackground")) {
				zxdesktop_draw_scrfile_mix_background=1;
			}            


			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-scr-scalefactor")) {
                siguiente_parametro_argumento();

				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<1 || valor>5) {
					printf ("Invalid value for ZX Desktop SCR scale factor\n");
					exit(1);
				}
				zxdesktop_draw_scrfile_scale_factor=valor;
			}

            else if (!strcmp(argv[puntero_parametro],"--zxdesktop-scr-disable-flash")) {
                zxdesktop_draw_scrfile_disable_flash=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--zxdesktop-disable-configurable-icons")) {
                zxdesktop_configurable_icons_enabled.v=0;
            }

            else if (!strcmp(argv[puntero_parametro],"--zxdesktop-add-icon")) {
                //get_defined_direct_functions
                //"--zxdesktop-add-icon x y a n e s                  Add icon to position x,y, to function f, icon name n, extra parameters e, status s\n"
                siguiente_parametro_argumento();
                int x=parse_string_to_number(argv[puntero_parametro]);
                siguiente_parametro_argumento();
                int y=parse_string_to_number(argv[puntero_parametro]);
                siguiente_parametro_argumento();
                int indice_funcion=get_defined_direct_functions(argv[puntero_parametro]);
                siguiente_parametro_argumento();

                if (indice_funcion<0) {
                    printf ("Invalid action for icon: %s\n",argv[puntero_parametro]);
                    exit(1);
                }

                char *text_icon=argv[puntero_parametro];
                siguiente_parametro_argumento();

                char *extra_info=argv[puntero_parametro];
                siguiente_parametro_argumento();

                enum zxdesktop_custom_icon_status_ids status;

                if (!strcasecmp("exists",argv[puntero_parametro])) {
                    status=ZXDESKTOP_CUSTOM_ICON_EXISTS;
                }

                else if (!strcasecmp("deleted",argv[puntero_parametro])) {
                    status=ZXDESKTOP_CUSTOM_ICON_DELETED;
                }

                else {
                    printf("Invalid icon status %s\n",argv[puntero_parametro]);
                    exit(1);
                }

                
                
                int indice_icono=zxvision_add_configurable_icon_no_add_position(indice_funcion);
                if (indice_icono<0) {
                    printf("Can not add more icons, limit reached: %d\n",MAX_ZXDESKTOP_CONFIGURABLE_ICONS);
                    exit(1);
                }

                //Asignamos x,y a mano dado que aqui aun no sabemos el tamaño de la ventana y por tanto no sabemos si sale de rango
                zxdesktop_configurable_icons_list[indice_icono].pos_x=x;
                zxdesktop_configurable_icons_list[indice_icono].pos_y=y;

                zxvision_set_configurable_icon_text(indice_icono,text_icon);
                zxvision_set_configurable_icon_extra_info(indice_icono,extra_info);

                zxdesktop_configurable_icons_list[indice_icono].status=status;


            }

			else if (!strcmp(argv[puntero_parametro],"--watermark-position")) {
				siguiente_parametro_argumento();
				screen_watermark_position=atoi(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--menucharwidth")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor!=5 && valor!=6 && valor!=7 && valor!=8) {
					printf ("Invalid value for character width\n");
					exit(1);
				}
				menu_char_width=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--zoom")) {
				siguiente_parametro_argumento();
				zoom_y=zoom_x=atoi(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--frameskip")) {
                                siguiente_parametro_argumento();
                                frameskip=atoi(argv[puntero_parametro]);
				if (frameskip>49 || frameskip<0) {
					printf ("Frameskip out of range\n");
					exit(1);
				}
    }

			else if (!strcmp(argv[puntero_parametro],"--disable-autoframeskip")) {
					autoframeskip.v=0;
				}

            else if (!strcmp(argv[puntero_parametro],"--autoframeskip-moving-win")) {
                auto_frameskip_even_when_movin_windows.v=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--disable-flash")) {
                disable_change_flash.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--testconfig")) {
				test_config_and_exit.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--nochangeslowparameters")) {
				no_cambio_parametros_maquinas_lentas.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--fullscreen")) {
				ventana_fullscreen=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--verbose")) {
                                siguiente_parametro_argumento();
                                verbose_level=atoi(argv[puntero_parametro]);
				if (verbose_level<0 || verbose_level>4) {
					printf ("Invalid Verbose level\n");
					exit(1);
				}
            }

            else if (!strcmp(argv[puntero_parametro],"--disable-debug-console-win")) {
                //Por defecto esta habilitado, por tanto lo desactivamos
                debug_unnamed_console_end();
                debug_unnamed_console_enabled.v=0;
            }

            else if (!strcmp(argv[puntero_parametro],"--verbose-always-console")) {
                debug_always_show_messages_in_console.v=1;
            }			

			else if (!strcmp(argv[puntero_parametro],"--nodisableconsole")) {
				//Parametro que solo es de Windows, pero lo admitimos en cualquier sistema
				windows_no_disable_console.v=1;
			}

			/*
		        else if (!strcmp(argv[puntero_parametro],"--invespokerom")) {
                                siguiente_parametro_argumento();
                                valor_poke_rom=atoi(argv[puntero_parametro]);
                        }
			*/



                        else if (!strcmp(argv[puntero_parametro],"--cpuspeed")) {
                                siguiente_parametro_argumento();
                                porcentaje_velocidad_emulador=atoi(argv[puntero_parametro]);
                                if (porcentaje_velocidad_emulador<1 || porcentaje_velocidad_emulador>9999) {
                                        printf ("Invalid CPU percentage\n");
                                        exit(1);
                                }
                        }

			else if (!strcmp(argv[puntero_parametro],"--denyturbozxunoboot")) {
					zxuno_deny_turbo_bios_boot.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--denyturbotbbluerom")) {
					tbblue_deny_turbo_rom.v=1;
			}		

            //Contemplamos los dos setting de deny y allow para compatibilidad con versiones previas a ZEsarUX 9.3, donde antes
            //estaba permitido por defecto
			else if (!strcmp(argv[puntero_parametro],"--allowturbotbbluerom")) {
					tbblue_deny_turbo_rom.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--tbblue-max-turbo-rom")) {
				
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<1 || valor>8) {
						printf ("Invalid value for tbblue-max-turbo-rom\n");
						exit(1);
				}
				tbblue_deny_turbo_rom_max_allowed=valor;					
			}					

			else if (!strcmp(argv[puntero_parametro],"--tbblue-fast-boot-mode")) {
				tbblue_fast_boot_mode.v=1;
			}  

			else if (!strcmp(argv[puntero_parametro],"--random-r-register")) {
				cpu_random_r_register.v=1;
			}

			/* no uso esto de momento
			else if (!strcmp(argv[puntero_parametro],"--tbblue-123b-port")) {
				siguiente_parametro_argumento();
				tbblue_initial_123b_port=atoi(argv[puntero_parametro]);
			}*/

                        else if (!strcmp(argv[puntero_parametro],"--zx8081mem")) {
                                siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor<1 || valor>16) {
					printf ("Invalid RAM value\n");
					exit(1);
				}
				set_zx8081_ramtop(valor);
                        }

			else if (!strcmp(argv[puntero_parametro],"--acemem")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor!=3 && valor!=19 && valor!=35 && valor!=51) {
                                        printf ("Invalid RAM value\n");
                                        exit(1);
                                }
                                set_ace_ramtop(valor);
                        }

                        else if (!strcmp(argv[puntero_parametro],"--videozx8081")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<1 || valor>16) {
                                        printf ("Invalid threshold value\n");
                                        exit(1);
                                }
				simulate_screen_zx8081.v=1;
				umbral_simulate_screen_zx8081=valor;
                        }


			else if (!strcmp(argv[puntero_parametro],"--128kmem")) {
                                siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				z80_byte multiplicador;
				switch (valor) {

					case 128:
						multiplicador=1;
					break;

					case 256:
						multiplicador=2;
					break;

					case 512:
						multiplicador=4;
					break;

					case 1024:
						multiplicador=8;
					break;					


					default:
						printf ("Invalid RAM value\n");
						exit(1);
					break;
				}

				mem_set_multiplicador_128(multiplicador);
      }


			else if (!strcmp(argv[puntero_parametro],"--scr")) {
				siguiente_parametro_argumento();
				//cargar pantalla
				scrfile=argv[puntero_parametro];
			}



			else if (!strcmp(argv[puntero_parametro],"--tape")) {
				siguiente_parametro_argumento();
				insert_tape_cmdline(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--realtape")) {
                                siguiente_parametro_argumento();
				realtape_name=argv[puntero_parametro];
			}



                        else if (!strcmp(argv[puntero_parametro],"--snap")) {
				siguiente_parametro_argumento();
				insert_snap_cmdline(argv[puntero_parametro]);
                        }


                        else if (!strcmp(argv[puntero_parametro],"--outtape")) {
                                siguiente_parametro_argumento();
                                tape_out_file=argv[puntero_parametro];
                        }


                        else if (!strcmp(argv[puntero_parametro],"--noautoload")) {
				noautoload.v=1;
                        }

			else if (!strcmp(argv[puntero_parametro],"--fastautoload")) {
                                fast_autoload.v=1;
			}


			//eprom y flash cards de z88 hacen lo mismo que quickload
			else if (!strcmp(argv[puntero_parametro],"--slotcard")) {
                                siguiente_parametro_argumento();
				//ver si extension valida
				if (
				      !util_compare_file_extension(argv[puntero_parametro],"epr")
				 ||   !util_compare_file_extension(argv[puntero_parametro],"63")
				 ||   !util_compare_file_extension(argv[puntero_parametro],"eprom")
				 ||   !util_compare_file_extension(argv[puntero_parametro],"flash")

				 ){
					quickload_inicial.v=1;
					quickload_nombre=argv[puntero_parametro];
				}
				else {
                    printf ("Invalid extension for eprom/flash card\n");
					exit(1);
				}

            }

			else if (!strcmp(argv[puntero_parametro],"--slotcard-num")) {
                siguiente_parametro_argumento();
				//ver si extension valida
				if (
				      !util_compare_file_extension(argv[puntero_parametro],"epr")
				 ||   !util_compare_file_extension(argv[puntero_parametro],"63")
				 ||   !util_compare_file_extension(argv[puntero_parametro],"eprom")
				 ||   !util_compare_file_extension(argv[puntero_parametro],"flash")

				 ){
					z88_slotcard_inicial.v=1;
					z88_slotcard_inicial_nombre=argv[puntero_parametro];

                    siguiente_parametro_argumento();
                    z88_slotcard_inicial_slot=parse_string_to_number(argv[puntero_parametro]);
                    if (z88_slotcard_inicial_slot<1 || z88_slotcard_inicial_slot>3) {
                        printf("Invalid slot number\n");
                        exit(1);
                    }
				}
				else {
                    printf ("Invalid extension for eprom/flash card\n");
					exit(1);
				}

            }            


                        else if (!strcmp(argv[puntero_parametro],"--vo")) {
				int drivervook=0;
				try_fallback_video.v=0;

                                siguiente_parametro_argumento();
                                driver_screen=argv[puntero_parametro];

#ifdef USE_COCOA
				if (!strcmp(driver_screen,"cocoa")) drivervook=1;
#endif

#ifdef COMPILE_XWINDOWS
				if (!strcmp(driver_screen,"xwindows")) drivervook=1;
#endif

#ifdef COMPILE_SDL
                                if (!strcmp(driver_screen,"sdl")) drivervook=1;
#endif


#ifdef COMPILE_FBDEV
				if (!strcmp(driver_screen,"fbdev")) drivervook=1;
#endif

#ifdef COMPILE_CURSES
				if (!strcmp(driver_screen,"curses")) drivervook=1;
#endif

#ifdef COMPILE_AA
				if (!strcmp(driver_screen,"aa")) drivervook=1;
#endif

#ifdef COMPILE_CACA
				if (!strcmp(driver_screen,"caca")) drivervook=1;
#endif

#ifdef COMPILE_STDOUT
				if (!strcmp(driver_screen,"stdout")) drivervook=1;
#endif

#ifdef COMPILE_SIMPLETEXT
                                if (!strcmp(driver_screen,"simpletext")) drivervook=1;
#endif


				if (!strcmp(driver_screen,"null")) drivervook=1;

				if (!drivervook) {
					printf ("Video Driver %s not supported\n",driver_screen);
					exit(1);
				}



                        }

#ifdef COMPILE_AA
                        else if (!strcmp(argv[puntero_parametro],"--aaslow")) {
                                scraa_fast=0;
                        }
#endif




                        else if (!strcmp(argv[puntero_parametro],"--ao")) {
				try_fallback_audio.v=0;
                                int driveraook=0;
                                siguiente_parametro_argumento();
                                driver_audio=argv[puntero_parametro];



#ifdef COMPILE_DSP
                                if (!strcmp(driver_audio,"dsp")) driveraook=1;
#endif

#ifdef COMPILE_ONEBITSPEAKER
                                if (!strcmp(driver_audio,"onebitspeaker")) driveraook=1;
#endif

#ifdef COMPILE_SDL
                                if (!strcmp(driver_audio,"sdl")) driveraook=1;
#endif


#ifdef COMPILE_ALSA
                                if (!strcmp(driver_audio,"alsa")) driveraook=1;
#endif

#ifdef COMPILE_PULSE
                                if (!strcmp(driver_audio,"pulse")) driveraook=1;
#endif


#ifdef COMPILE_COREAUDIO
                                if (!strcmp(driver_audio,"coreaudio")) driveraook=1;
#endif

                                if (!strcmp(driver_audio,"null")) driveraook=1;

                                if (!driveraook) {
                                        printf ("Audio Driver %s not supported\n",driver_audio);
                                        exit(1);
                                }



                        }

                        else if (!strcmp(argv[puntero_parametro],"--aofile")) {
                                siguiente_parametro_argumento();
                                aofilename=argv[puntero_parametro];
                        }

                        else if (!strcmp(argv[puntero_parametro],"--vofile")) {
                                siguiente_parametro_argumento();
                                vofilename=argv[puntero_parametro];
                        }

                        else if (!strcmp(argv[puntero_parametro],"--vofilefps")) {
                                siguiente_parametro_argumento();

                                int valor=atoi(argv[puntero_parametro]);
                                if (valor==1 || valor==2 || valor==5 || valor==10 || valor==25 || valor==50) {
					vofile_fps=50/valor;
				}

				else {
				        printf ("Invalid FPS value\n");
                                        exit(1);
                                }

                        }


                        else if (!strcmp(argv[puntero_parametro],"--disableayspeech")) {
				ay_speech_enabled.v=0;
                        }

			else if (!strcmp(argv[puntero_parametro],"--disableenvelopes")) {
                                ay_envelopes_enabled.v=0;
                        }



                        else if (!strcmp(argv[puntero_parametro],"--debugregisters")) {
				debug_registers=1;
                        }

#ifdef USE_XEXT
			else if (!strcmp(argv[puntero_parametro],"--disableshm"))
				disable_shm=1;
#endif

			else if (!strcmp(argv[puntero_parametro],"--disableborder")) {
				border_enabled.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--hidemousepointer")) {
				mouse_pointer_shown.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--limitopenmenu")) {
				menu_limit_menu_open.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--setmachinebyname")) {
				setting_machine_selection_by_name.v=1;
			}			

            //soportar opcion antigua tambien
			else if (!strcmp(argv[puntero_parametro],"--hide-dirs") ||
                    !strcmp(argv[puntero_parametro],"--filebrowser-hide-dirs")
            ) {
				menu_filesel_hide_dirs.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--filebrowser-hide-size")) {
				menu_filesel_hide_size.v=1;
			}            

            else if (!strcmp(argv[puntero_parametro],"--filebrowser-allow-folder-delete")) {
                menu_filesel_utils_allow_folder_delete.v=1;
            }

  			else if (!strcmp(argv[puntero_parametro],"--fileviewer-hex")) {
				menu_file_viewer_always_hex.v=1;
			}          

			else if (!strcmp(argv[puntero_parametro],"--no-file-previews")) {
				menu_filesel_show_previews.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--reduce-file-previews")) {
				menu_filesel_show_previews_reduce.v=1;
			}            


			else if (!strcmp(argv[puntero_parametro],"--disablemenumouse")) {
				mouse_menu_disabled.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--ignoremouseclickopenmenu")) {
                mouse_menu_ignore_click_open.v=1;
            }
/*
                "--text-keyboard-add        Add a string to the Adventure Text Keyboard\n"
                "--text-keyboard-clear      Clear all entries of the Adventure Text Keyboard\n"
*/

		       else if (!strcmp(argv[puntero_parametro],"--text-keyboard-add")) {
				if (added_some_osd_text_keyboard.v==0) {
					util_clear_text_adventure_kdb();
					added_some_osd_text_keyboard.v=1;
					//printf ("Clearing text keyboard\n");
				}
                                siguiente_parametro_argumento();
				//printf ("Adding text keyboard %s\n",argv[puntero_parametro]);
				util_add_text_adventure_kdb(argv[puntero_parametro]);
                        }

				else if (!strcmp(argv[puntero_parametro],"--text-keyboard-length")) {
						siguiente_parametro_argumento();
						int valor=parse_string_to_number(argv[puntero_parametro]);
						if (valor<10 || valor>100) {
                                        printf ("Invalid text-keyboard-length value\n");
                                        exit(1);
                                }
						adventure_keyboard_key_length=valor;
				}
						
				else if (!strcmp(argv[puntero_parametro],"--text-keyboard-finalspc")) {
						adventure_keyboard_send_final_spc=1;

				}


			/*else if (!strcmp(argv[puntero_parametro],"--overlayinfo")) {
				enable_second_layer();
			}*/

			else if (!strcmp(argv[puntero_parametro],"--disablefooter")) {
				disable_footer();
			}

			else if (!strcmp(argv[puntero_parametro],"--disablemultitaskmenu")) {
                menu_multitarea=0;
			}

            else if (!strcmp(argv[puntero_parametro],"--stopemulationmenu")) {
                menu_emulation_paused_on_menu=1;
            }
		
			else if (!strcmp(argv[puntero_parametro],"--disablebw-no-multitask")) {
				//Obsoleto
            			//screen_bw_no_multitask_menu.v=0;
			}


			else if (!strcmp(argv[puntero_parametro],"--machine")) {
				char *machine_name;
				siguiente_parametro_argumento();
                                machine_name=argv[puntero_parametro];

				if (set_machine_type_by_name(machine_name)) {
					exit(1);
                                }

			}

			else if (!strcmp(argv[puntero_parametro],"--videofastblack")) {
				video_fast_mode_emulation.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--no-ocr-alternatechars")) {
				ocr_settings_not_look_23606.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--allpixeltotext")) {
				screen_text_all_refresh_pixel.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--allpixeltotext-scale")) {
					siguiente_parametro_argumento();
					int valor=parse_string_to_number(argv[puntero_parametro]);
					if (valor<1 || valor>99) {
									printf ("Invalid --allpixeltotext-scale value\n");
									exit(1);
							}
					screen_text_all_refresh_pixel_scale=valor;
			}


			else if (!strcmp(argv[puntero_parametro],"--allpixeltotext-invert")) {
				screen_text_all_refresh_pixel_invert.v=1;
			}			

			else if (!strcmp(argv[puntero_parametro],"--allpixeltotext-width")) {
                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);
                if (valor<1 || valor>9999) {
                                printf ("Invalid --allpixeltotext-width value\n");
                                exit(1);
                        }				
                scr_refresca_pantalla_tsconf_text_max_ancho=valor;
			}		

			else if (!strcmp(argv[puntero_parametro],"--allpixeltotext-x-offset")) {
                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);
                if (valor<0 || valor>9999) {
                                printf ("Invalid --allpixeltotext-x-offset value\n");
                                exit(1);
                        }				
                scr_refresca_pantalla_tsconf_text_offset_x=valor;
			}	

			else if (!strcmp(argv[puntero_parametro],"--allpixeltotext-height")) {
                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);
                if (valor<1 || valor>9999) {
                                printf ("Invalid --allpixeltotext-height value\n");
                                exit(1);
                        }				
                scr_refresca_pantalla_tsconf_text_max_alto=valor;
			}		

			else if (!strcmp(argv[puntero_parametro],"--allpixeltotext-y-offset")) {
                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);
                if (valor<0 || valor>9999) {
                                printf ("Invalid --allpixeltotext-y-offset value\n");
                                exit(1);
                        }				
                scr_refresca_pantalla_tsconf_text_offset_y=valor;
			}	



			else if (!strcmp(argv[puntero_parametro],"--zx8081vsyncsound")) {
				command_line_zx8081_vsync_sound.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zx8081ram8K2000")) {
                                ram_in_8192.v=1;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--zx8081ram16K8000")) {
                                enable_ram_in_32768();
                        }

                        else if (!strcmp(argv[puntero_parametro],"--zx8081ram16KC000")) {
                                enable_ram_in_49152();
                        }

			else if (!strcmp(argv[puntero_parametro],"--autodetectwrx")) {
                                autodetect_wrx.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--wrx")) {
                                command_line_wrx.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--vsync-minimum-length")) {
		                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<100 || valor>999) {
                                        printf ("Invalid vsync length value\n");
                                        exit(1);
                                }
                                command_line_vsync_minimum_lenght=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--chroma81")) {
                                command_line_chroma81.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--romfile")) {
                                siguiente_parametro_argumento();
                                param_custom_romfile=argv[puntero_parametro];
			}

                        else if (!strcmp(argv[puntero_parametro],"--smartloadpath")) {
                                siguiente_parametro_argumento();
				sprintf(quickload_file,"%s/",argv[puntero_parametro]);

                                quickfile=quickload_file;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--addlastfile")) {
                                siguiente_parametro_argumento();
				last_filesused_insert(argv[puntero_parametro]);
                        }

			else if (!strcmp(argv[puntero_parametro],"--quicksavepath")) {
                                siguiente_parametro_argumento();
				sprintf(snapshot_autosave_interval_quicksave_directory,"%s/",argv[puntero_parametro]);
                        }


			else if (!strcmp(argv[puntero_parametro],"--loadbinarypath")) {
                                siguiente_parametro_argumento();
                                sprintf(binary_file_load,"%s/",argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--savebinarypath")) {
                                siguiente_parametro_argumento();
                                sprintf(binary_file_save,"%s/",argv[puntero_parametro]);
			}			

			else if (!strcmp(argv[puntero_parametro],"--zxunospifile")) {
                                siguiente_parametro_argumento();
				sprintf(zxuno_flash_spi_name,"%s",argv[puntero_parametro]);
                        }

			else if (!strcmp(argv[puntero_parametro],"--zxuno-initial-64k")) {
                                siguiente_parametro_argumento();
				sprintf(zxuno_initial_64k_file,"%s",argv[puntero_parametro]);
                        }
                       

			else if (
			         !strcmp(argv[puntero_parametro],"--zxunospi-persistent-writes")
			      ||
				 !strcmp(argv[puntero_parametro],"--zxunospiwriteenable")     //opcion obsoleta
				) {
                                zxuno_flash_persistent_writes.v=1;
			}


                        else if (!strcmp(argv[puntero_parametro],"--zxunospi-write-protection")) {
				zxuno_flash_write_protection.v=1;
                        }


			else if (!strcmp(argv[puntero_parametro],"--printerbitmapfile")) {
				siguiente_parametro_argumento();
                                zxprinter_enabled.v=1;
				zxprinter_bitmap_filename=argv[puntero_parametro];
				zxprinter_file_bitmap_init();
			}

			else if (!strcmp(argv[puntero_parametro],"--printertextfile")) {
                                siguiente_parametro_argumento();
                                zxprinter_enabled.v=1;
                                zxprinter_ocr_filename=argv[puntero_parametro];
                                zxprinter_file_ocr_init();
                        }

			else if (!strcmp(argv[puntero_parametro],"--redefinekey")) {
				z80_byte tecla_original, tecla_redefinida;
				siguiente_parametro_argumento();
				tecla_original=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				tecla_redefinida=parse_string_to_number(argv[puntero_parametro]);

				if (util_add_redefinir_tecla(tecla_original,tecla_redefinida)) {
					exit(1);
				}
			}

			else if (!strcmp(argv[puntero_parametro],"--recreatedzx")) {
				recreated_zx_keyboard_support.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--keymap")) {
				siguiente_parametro_argumento();
                int valor=atoi(argv[puntero_parametro]);
                if (valor<0 || valor>1) {
               		printf ("Invalid Keymap value\n");
                    exit(1);
                }
                z88_cpc_keymap_type=valor;
			}

			

			else if (!strcmp(argv[puntero_parametro],"--enablekempstonmouse")) {
				kempston_mouse_emulation.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--kempstonmouse-sens")) {
				siguiente_parametro_argumento();
                int valor=atoi(argv[puntero_parametro]);
                if (valor<1 || valor>MAX_KMOUSE_SENSITIVITY) {
               		printf ("Invalid Kempston Mouse Sensitivity value\n");
                    exit(1);
                }
                kempston_mouse_factor_sensibilidad=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--spectrum-reduced-core")) {
                                core_spectrum_uses_reduced.v=1;
                        }

			else if (!strcmp(argv[puntero_parametro],"--no-spectrum-reduced-core")) {
                                core_spectrum_uses_reduced.v=0;
                        }

            else if (!strcmp(argv[puntero_parametro],"--ula-data-bus")) {
                siguiente_parametro_argumento();
                ula_databus_value=parse_string_to_number(argv[puntero_parametro]);
            }

			else if (!strcmp(argv[puntero_parametro],"--def-f-function")) {
				siguiente_parametro_argumento();
				if (argv[puntero_parametro][0]!='F' && argv[puntero_parametro][0]!='f') {
					printf ("Unknown key\n");
					exit(1);
				}

				int valor=atoi(&argv[puntero_parametro][1]);

				if (valor<1 || valor>MAX_F_FUNCTIONS_KEYS) {
					printf ("Invalid key\n");
					exit(1);
				}

				siguiente_parametro_argumento();

				if (menu_define_key_function(valor,argv[puntero_parametro])) {
					printf ("Invalid f-function action: %s\n",argv[puntero_parametro]);
					exit(1);
				}


			}

			else if (!strcmp(argv[puntero_parametro],"--def-button-function")) {
				siguiente_parametro_argumento();
	

				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<0 || valor>=MAX_USERDEF_BUTTONS) {
					printf ("Invalid button\n");
					exit(1);
				}

				siguiente_parametro_argumento();

				if (menu_define_button_function(valor,argv[puntero_parametro])) {
					printf ("Invalid button action: %s\n",argv[puntero_parametro]);
					exit(1);
				}


			}            


			else if (!strcmp(argv[puntero_parametro],"--autoloadsnap")) {
                        	autoload_snapshot_on_start.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--autosavesnap")) {
                                autosave_snapshot_on_exit.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--autosnappath")) {
                                siguiente_parametro_argumento();
                                sprintf(autosave_snapshot_path_buffer,"%s/",argv[puntero_parametro]);
                        }


			else if (!strcmp(argv[puntero_parametro],"--tempdir")) {
                                siguiente_parametro_argumento();
                                sprintf(emulator_tmpdir_set_by_user,"%s/",argv[puntero_parametro]);
                        }

			//--sna-no-change-machine deprecated
			else if (!strcmp(argv[puntero_parametro],"--sna-no-change-machine") || !strcmp(argv[puntero_parametro],"--snap-no-change-machine")
			) {
				sna_setting_no_change_machine.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--no-close-after-smartload")) {
				no_close_menu_after_smartload.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--snapram-interval")) {
                siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>99) {
					printf ("Invalid snapram-interval value. Must be between 1 and 99\n");
					exit(1);
				}
				snapshot_in_ram_interval_seconds=valor;
            }

            else if (!strcmp(argv[puntero_parametro],"--snapram-max")) {
                siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>MAX_TOTAL_SNAPSHOTS_IN_RAM) {
					printf ("Invalid snapram-max value. Must be between 1 and %d\n",MAX_TOTAL_SNAPSHOTS_IN_RAM);
					exit(1);
				}
				snapshots_in_ram_maximum=valor;
            }

            else if (!strcmp(argv[puntero_parametro],"--snapram-rewind-timeout")) {
                siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>99) {
					printf ("Invalid snapram-rewind-timeout value. Must be between 1 and 99\n");
					exit(1);
				}
				snapshot_in_ram_enabled_timer_timeout=valor;
            }



			else if (!strcmp(argv[puntero_parametro],"--loadbinary")) {
				siguiente_parametro_argumento();
				command_line_load_binary_file=argv[puntero_parametro];
				siguiente_parametro_argumento();
				command_line_load_binary_address=parse_string_to_number(argv[puntero_parametro]);
				siguiente_parametro_argumento();
				command_line_load_binary_length=parse_string_to_number(argv[puntero_parametro]);

				//Y decimos cual ha sido ultimo archivo binario cargado
				sprintf(binary_file_load,"%s",command_line_load_binary_file);
			}


			else if (!strcmp(argv[puntero_parametro],"--disablearttext")) {
				texto_artistico.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--curses-ext-utf")) {
				use_scrcursesw.v=1;
			}			


                        else if (!strcmp(argv[puntero_parametro],"--arttextthresold")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<1 || valor>16) {
                                        printf ("Invalid threshold value\n");
                                        exit(1);
                                }
                                umbral_arttext=valor;
                        }


		        else if (!strcmp(argv[puntero_parametro],"--disableprintchartrap")) {
				command_line_chardetect_printchar_enabled=0;
			}

                        else if (!strcmp(argv[puntero_parametro],"--enableprintchartrap")) {
				command_line_chardetect_printchar_enabled=1;
                        }

                else if (!strcmp(argv[puntero_parametro],"--chardetectignorenl")) {
                        chardetect_ignore_newline.v=1;
                }

                else if (!strcmp(argv[puntero_parametro],"--chardetectcompatnum")) {
                    chardetect_rom_compat_numbers.v=1;
                }


		        else if (!strcmp(argv[puntero_parametro],"--autoredrawstdout")) {
				stdout_simpletext_automatic_redraw.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--textfps")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                scr_set_fps_stdout_simpletext(valor);
                        }



			else if (!strcmp(argv[puntero_parametro],"--sendansi")) {
                                screen_text_accept_ansi=1;
			}

                        else if (!strcmp(argv[puntero_parametro],"--linewidth")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                chardetect_line_width=valor;
                        }


		        else if (!strcmp(argv[puntero_parametro],"--automaticdetectchar")) {
				trap_char_detection_routine_number=TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC;
				chardetect_detect_char_enabled.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--secondtrapchar")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
                               	chardetect_second_trap_char_dir=valor;
			}

                        else if (!strcmp(argv[puntero_parametro],"--thirdtrapchar")) {
				siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                chardetect_third_trap_char_dir=valor;
                        }

			else if (!strcmp(argv[puntero_parametro],"--linewidthwaitspace")) {
				chardetect_line_width_wait_space.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--linewidthnowaitspace")) {
				chardetect_line_width_wait_space.v=0;
			}            

			else if (!strcmp(argv[puntero_parametro],"--linewidthwaitdot")) {
				chardetect_line_width_wait_dot.v=1;
			}           

			else if (!strcmp(argv[puntero_parametro],"--linewidthnowaitdot")) {
				chardetect_line_width_wait_dot.v=0;
			}                  

			else if (!strcmp(argv[puntero_parametro],"--secondtrapsum32")) {
				chardetect_second_trap_sum32.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--textspeechprogram")) {
				siguiente_parametro_argumento();
                                textspeech_filter_program=argv[puntero_parametro];
				//Si es ruta relativa, poner ruta absoluta
				if (!si_ruta_absoluta(textspeech_filter_program)) {
					//printf ("es ruta relativa\n");
					//lo metemos en el buffer de texto del menu usado para esto
					//TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
					//no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
					//directorio de archivo
				        char directorio_actual[PATH_MAX];
				        getcwd(directorio_actual,PATH_MAX);

					sprintf (menu_buffer_textspeech_filter_program,"%s/%s",directorio_actual,textspeech_filter_program);
		                        textspeech_filter_program=menu_buffer_textspeech_filter_program;

					//printf ("ruta final: %s\n",textspeech_filter_program);

				}

				//Validar para windows que no haya espacios en la ruta
				textspeech_filter_program_check_spaces();

				//Y si es NULL es que han habido espacios, y salir del emulador
				if (textspeech_filter_program==NULL) exit(1);

			}

                        else if (!strcmp(argv[puntero_parametro],"--textspeechstopprogram")) {
                                siguiente_parametro_argumento();
                                textspeech_stop_filter_program=argv[puntero_parametro];
                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(textspeech_stop_filter_program)) {
                                        //printf ("es ruta relativa\n");
                                        //lo metemos en el buffer de texto del menu usado para esto
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (menu_buffer_textspeech_stop_filter_program,"%s/%s",directorio_actual,textspeech_stop_filter_program);
                                        textspeech_stop_filter_program=menu_buffer_textspeech_stop_filter_program;

                                        //printf ("ruta final: %s\n",textspeech_stop_filter_program);


                                        //Validar para windows que no haya espacios en la ruta
                                        textspeech_stop_filter_program_check_spaces();

                                        //Y si es NULL es que han habido espacios, y salir del emulador
                                        if (textspeech_stop_filter_program==NULL) exit(1);
                                }
                        }

			else if (!strcmp(argv[puntero_parametro],"--textspeechwait")) {
				textspeech_filter_program_wait.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--textspeechmenu")) {
                                textspeech_also_send_menu.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--textspeechgetstdout")) {
                textspeech_get_stdout.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--textspeechtimeout")) {
                                siguiente_parametro_argumento();
				textspeech_timeout_no_enter=parse_string_to_number(argv[puntero_parametro]);
				if (textspeech_timeout_no_enter<0 || textspeech_timeout_no_enter>99) {
					printf ("Invalid value for textspeechtimeout\n");
					exit(1);
				}
			}

                        else if (!strcmp(argv[puntero_parametro],"--tool-sox-path")) {
                                siguiente_parametro_argumento();
                                sprintf (external_tool_sox,"%s",argv[puntero_parametro]);
			}

						//deprecated
                        else if (!strcmp(argv[puntero_parametro],"--tool-unzip-path")) {
                                siguiente_parametro_argumento();
                                //sprintf (external_tool_unzip,"%s",argv[puntero_parametro]);
			}

                        else if (!strcmp(argv[puntero_parametro],"--tool-gunzip-path")) {
                                siguiente_parametro_argumento();
                                sprintf (external_tool_gunzip,"%s",argv[puntero_parametro]);
			}

                        else if (!strcmp(argv[puntero_parametro],"--tool-tar-path")) {
                                siguiente_parametro_argumento();
                                sprintf (external_tool_tar,"%s",argv[puntero_parametro]);
			}

                        else if (!strcmp(argv[puntero_parametro],"--tool-unrar-path")) {
                                siguiente_parametro_argumento();
                                sprintf (external_tool_unrar,"%s",argv[puntero_parametro]);
			}



			else if (!strcmp(argv[puntero_parametro],"--dsk-file")) {
				siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (dskplusthree_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

				else {
					sprintf (dskplusthree_file_name,"%s",argv[puntero_parametro]);
				}

			}


                        else if (!strcmp(argv[puntero_parametro],"--enable-dsk")) {
                                command_line_dsk.v=1;
                        }


                        else if (!strcmp(argv[puntero_parametro],"--dsk-write-protection")) {
				dskplusthree_write_protection.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--dsk-no-persistent-writes")) {
				dskplusthree_persistent_writes.v=0;
			}


			else if (!strcmp(argv[puntero_parametro],"--hilow-file")) {
				siguiente_parametro_argumento();

                //Si es ruta relativa, poner ruta absoluta
                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                        //printf ("es ruta relativa\n");

                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                        //directorio de archivo
                        char directorio_actual[PATH_MAX];
                        getcwd(directorio_actual,PATH_MAX);

                        sprintf (hilow_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                }

				else {
					sprintf (hilow_file_name,"%s",argv[puntero_parametro]);
				}

			}

                        else if (!strcmp(argv[puntero_parametro],"--enable-hilow")) {
                                command_line_hilow.v=1;
                        }

			else if (!strcmp(argv[puntero_parametro],"--hilow-write-protection")) {
				hilow_write_protection.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--hilow-no-persistent-writes")) {
				hilow_persistent_writes.v=0;
			}                        

            else if (!strcmp(argv[puntero_parametro],"--load-source-code")) {
                    command_line_load_source_code.v=1;

                    siguiente_parametro_argumento();

                    command_line_load_source_code_file=argv[puntero_parametro];
            }                        

			else if (!strcmp(argv[puntero_parametro],"--trd-file")) {
				siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (trd_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

				else {
					sprintf (trd_file_name,"%s",argv[puntero_parametro]);
				}

			}


                        else if (!strcmp(argv[puntero_parametro],"--enable-trd")) {
                                command_line_trd.v=1;
                        }


                        else if (!strcmp(argv[puntero_parametro],"--trd-write-protection")) {
				trd_write_protection.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--trd-no-persistent-writes")) {
				trd_persistent_writes.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--mmc-file")) {
				siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (mmc_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

				else {
					sprintf (mmc_file_name,"%s",argv[puntero_parametro]);
				}

			}

			else if (!strcmp(argv[puntero_parametro],"--enable-mmc")) {
				command_line_mmc.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--copy-file-to-mmc")) {
                siguiente_parametro_argumento();
				char *source;
                source=argv[puntero_parametro];

                siguiente_parametro_argumento();
				char *destination;
                destination=argv[puntero_parametro];

                if (util_copy_files_to_mmc_addlist(source,destination)) {
                    exit(1);
                }
            }

			else if (!strcmp(argv[puntero_parametro],"--mmc-write-protection")) {
				mmc_write_protection.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--mmc-no-persistent-writes")) {
				mmc_persistent_writes.v=0;
			}


			else if (!strcmp(argv[puntero_parametro],"--enable-divmmc-ports")) {
				command_line_divmmc_ports.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enable-divmmc-paging")) {
				command_line_divmmc_paging.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enable-divmmc")) {
				command_line_divmmc.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--divmmc-rom")) {
				siguiente_parametro_argumento();
				strcpy(divmmc_rom_name,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--enable-zxmmc")) {
				command_line_zxmmc.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enable-8bit-ide")) {
                                command_line_8bitide.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--diviface-ram-size")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				/*
				int diviface_current_ram_memory_bits=4; //Por defecto 128 KB

				-using 2 bits: 32 kb
				-using 3 bits: 64 kb
				-using 4 bits: 128 kb (default)
				-using 5 bits: 256 kb
				-using 6 bits: 512 kb
				*/

				switch (valor) {
					case 32:
						diviface_current_ram_memory_bits=2;
					break;

					case 64:
						diviface_current_ram_memory_bits=3;
					break;

					case 128:
						diviface_current_ram_memory_bits=4;
					break;

					case 256:
						diviface_current_ram_memory_bits=5;
					break;

					case 512:
						diviface_current_ram_memory_bits=6;
					break;

					default:
						printf ("Invalid value for diviface ram size\n");
						exit(1);
					break;
				}

			}



			else if (!strcmp(argv[puntero_parametro],"--enable-zxpand")) {
				command_line_zxpand.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxpand-root-dir")) {
      	siguiente_parametro_argumento();

        //Si es ruta relativa, poner ruta absoluta
        if (!si_ruta_absoluta(argv[puntero_parametro])) {
        	//printf ("es ruta relativa\n");
					convert_relative_to_absolute(argv[puntero_parametro],zxpand_root_dir);
        }

				else {
          sprintf (zxpand_root_dir,"%s",argv[puntero_parametro]);
				}
			}



			else if (!strcmp(argv[puntero_parametro],"--enable-esxdos-handler")) {
			  command_line_esxdos_handler.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--esxdos-root-dir")) {
			  siguiente_parametro_argumento();

			  //Si es ruta relativa, poner ruta absoluta
			  if (!si_ruta_absoluta(argv[puntero_parametro])) {
			    //printf ("es ruta relativa\n");
			    convert_relative_to_absolute(argv[puntero_parametro],esxdos_handler_root_dir);
			  }

			  else {
			    sprintf (esxdos_handler_root_dir,"%s",argv[puntero_parametro]);
			  }
			}

            else if (!strcmp(argv[puntero_parametro],"--esxdos-readonly")) {
                esxdos_handler_readonly.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--esxdos-local-dir")) {
                          siguiente_parametro_argumento();

			  command_line_esxdos_local_dir_path=argv[puntero_parametro];
			  command_line_esxdos_local_dir.v=1;
			}

/*
				"--enable-ql-mdv-flp        Enable QL Microdrive & Floppy emulation\n"
				"--ql-mdv1-root-dir p       Set QL mdv1 root directory\n"
				"--ql-mdv2-root-dir p       Set QL mdv2 root directory\n"
				"--ql-flp1-root-dir p       Set QL flp1 root directory\n"
				*/
			else if (!strcmp(argv[puntero_parametro],"--enable-ql-mdv-flp")) {
				ql_microdrive_floppy_emulation=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--ql-mdv1-root-dir")) {
			  siguiente_parametro_argumento();

			  //Si es ruta relativa, poner ruta absoluta
			  if (!si_ruta_absoluta(argv[puntero_parametro])) {
			    //printf ("es ruta relativa\n");
			    convert_relative_to_absolute(argv[puntero_parametro],ql_mdv1_root_dir);
			  }

			  else {
			    sprintf (ql_mdv1_root_dir,"%s",argv[puntero_parametro]);
				}
			}

			else if (!strcmp(argv[puntero_parametro],"--ql-mdv2-root-dir")) {
			  siguiente_parametro_argumento();

			  //Si es ruta relativa, poner ruta absoluta
			  if (!si_ruta_absoluta(argv[puntero_parametro])) {
			    //printf ("es ruta relativa\n");
			    convert_relative_to_absolute(argv[puntero_parametro],ql_mdv2_root_dir);
			  }

			  else {
			    sprintf (ql_mdv2_root_dir,"%s",argv[puntero_parametro]);
				}
			}

			else if (!strcmp(argv[puntero_parametro],"--ql-flp1-root-dir")) {
			  siguiente_parametro_argumento();

			  //Si es ruta relativa, poner ruta absoluta
			  if (!si_ruta_absoluta(argv[puntero_parametro])) {
			    //printf ("es ruta relativa\n");
			    convert_relative_to_absolute(argv[puntero_parametro],ql_flp1_root_dir);
			  }

			  else {
			    sprintf (ql_flp1_root_dir,"%s",argv[puntero_parametro]);
				}
			}


            else if (!strcmp(argv[puntero_parametro],"--ql-mdv1-read-only")) {
                ql_device_mdv1_readonly=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--ql-mdv2-read-only")) {
                ql_device_mdv2_readonly=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--ql-flp1-read-only")) {
                ql_device_flp1_readonly=1;
            }



                        else if (!strcmp(argv[puntero_parametro],"--ide-file")) {
                                siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (ide_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

                                else {
                                        sprintf (ide_file_name,"%s",argv[puntero_parametro]);
                                }

                        }

                        else if (!strcmp(argv[puntero_parametro],"--enable-ide")) {
                                command_line_ide.v=1;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--ide-write-protection")) {
				ide_write_protection.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--ide-no-persistent-writes")) {
				ide_persistent_writes.v=0;
			}

                        else if (!strcmp(argv[puntero_parametro],"--enable-divide")) {
                                command_line_divide.v=1;
                        }

												else if (!strcmp(argv[puntero_parametro],"--enable-divide-ports")) {
													command_line_divide_ports.v=1;
												}

												else if (!strcmp(argv[puntero_parametro],"--enable-divide-paging")) {
													command_line_divide_paging.v=1;
												}

												else if (!strcmp(argv[puntero_parametro],"--divide-rom")) {
													siguiente_parametro_argumento();
													strcpy(divide_rom_name,argv[puntero_parametro]);
												}


		        else if (!strcmp(argv[puntero_parametro],"--dandanator-rom")) {
                                siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (dandanator_rom_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

                                else {
                                        sprintf (dandanator_rom_file_name,"%s",argv[puntero_parametro]);
                                }

                        }

                        else if (!strcmp(argv[puntero_parametro],"--enable-dandanator")) {
                                command_line_dandanator.v=1;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--dandanator-press-button")) {
                                command_line_dandanator_push_button.v=1;
                        }


              else if (!strcmp(argv[puntero_parametro],"--kartusho-rom")) {
                                siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (kartusho_rom_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

                                else {
                                        sprintf (kartusho_rom_file_name,"%s",argv[puntero_parametro]);
                                }

                        }

                        else if (!strcmp(argv[puntero_parametro],"--enable-kartusho")) {
                                command_line_kartusho.v=1;
                        }

						else if (!strcmp(argv[puntero_parametro],"--ifrom-rom")) {
                                siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (ifrom_rom_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

                                else {
                                        sprintf (ifrom_rom_file_name,"%s",argv[puntero_parametro]);
                                }

                        }

                        else if (!strcmp(argv[puntero_parametro],"--enable-ifrom")) {
                                command_line_ifrom.v=1;
                        }						

                         else if (!strcmp(argv[puntero_parametro],"--enable-betadisk")) {
                                command_line_betadisk.v=1;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--superupgrade-flash")) {
                                siguiente_parametro_argumento();

                                //Si es ruta relativa, poner ruta absoluta
                                if (!si_ruta_absoluta(argv[puntero_parametro])) {
                                        //printf ("es ruta relativa\n");

                                        //TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
                                        //no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
                                        //directorio de archivo
                                        char directorio_actual[PATH_MAX];
                                        getcwd(directorio_actual,PATH_MAX);

                                        sprintf (superupgrade_rom_file_name,"%s/%s",directorio_actual,argv[puntero_parametro]);

                                }

                                else {
                                        sprintf (superupgrade_rom_file_name,"%s",argv[puntero_parametro]);
                                }

                        }

                        else if (!strcmp(argv[puntero_parametro],"--enable-superupgrade")) {
                                command_line_superupgrade.v=1;
                        }




#ifdef COMPILE_FBDEV
                	else if (!strcmp(argv[puntero_parametro],"--no-use-ttyfbdev")) {
				fbdev_no_uses_tty=1;
			}

                	else if (!strcmp(argv[puntero_parametro],"--no-use-ttyrawfbdev")) {
				fbdev_no_uses_ttyraw=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--use-all-res-fbdev")) {
                                fbdev_use_all_virtual_res=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--fbdev-double-buffer")) {
                    fbdev_double_buffer_enabled.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--decimal-full-scale-fbdev")) {
				ventana_fullscreen=1;
                                fbdev_decimal_full_scale_fbdev=1;
			}

#ifdef EMULATE_RASPBERRY

            else if (!strcmp(argv[puntero_parametro],"--fbdev-no-res-change")) {
                fbdev_no_res_change.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--fbdev-margin-height")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor<0) {
					printf ("Invalid margin height value\n");
					exit(1);
				}
				fbdev_margin_height=valor;
			}
			else if (!strcmp(argv[puntero_parametro],"--fbdev-margin-width")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor<0) {
					printf ("Invalid margin width value\n");
					exit(1);
				}
				fbdev_margin_width=valor;
			}
#endif


#endif

                        else if (!strcmp(argv[puntero_parametro],"--noautoselectfileopt")) {
                                autoselect_snaptape_options.v=0;
                        }

 							


			else if (!strcmp(argv[puntero_parametro],"--nosplash")) {
                                screen_show_splash_texts.v=0;
			}

			//Por defecto esta activo. Se mantiene solo por compatibilidad
			else if (!strcmp(argv[puntero_parametro],"--cpu-usage")) {
				screen_show_cpu_usage.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--no-cpu-usage")) {
				screen_show_cpu_usage.v=0;
			}			

			else if (!strcmp(argv[puntero_parametro],"--no-cpu-temp")) {
				screen_show_cpu_temp.v=0;
			}		


			else if (!strcmp(argv[puntero_parametro],"--no-fps")) {
				screen_show_fps.v=0;
			}							

			else if (!strcmp(argv[puntero_parametro],"--nowelcomemessage")) {
                                opcion_no_welcome_message.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--hide-menu-percentage-bar")) {
                                menu_hide_vertical_percentaje_bar.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--hide-menu-minimize-button")) {
                                menu_hide_minimize_button.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--hide-menu-maximize-button")) {
                                menu_hide_maximize_button.v=1;
			}  

			else if (!strcmp(argv[puntero_parametro],"--show-menu-background-button")) {
                                menu_hide_background_button_on_inactive.v=0;
			}                      

			else if (!strcmp(argv[puntero_parametro],"--hide-menu-submenu-indicator")) {
                menu_hide_submenu_indicator.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--language")) {
				siguiente_parametro_argumento();

                if (!strcasecmp(argv[puntero_parametro],"es")) {
                    gui_language=GUI_LANGUAGE_SPANISH;
                }
                else if (!strcasecmp(argv[puntero_parametro],"ca")) {
                    gui_language=GUI_LANGUAGE_CATALAN;
                }                
                else {
                    printf("Invalid language\n");
                    exit(1);
                }
			}

            else if (!strcmp(argv[puntero_parametro],"--online-download-path")) {
                siguiente_parametro_argumento();
                strcpy(online_download_path,argv[puntero_parametro]);
            }

			else if (!strcmp(argv[puntero_parametro],"--menu-mix-method")) {
				siguiente_parametro_argumento();
				int i;
				int encontrado=0;
				for (i=0;i<MAX_MENU_MIX_METHODS;i++) {
					if (!strcasecmp(screen_menu_mix_methods_strings[i],argv[puntero_parametro])) {
						screen_menu_mix_method=i;
						encontrado=1;
					}
				}

				if (!encontrado) {
						printf ("Invalid menu mix method\n");
						exit (1);
										
				}
			}

			
			else if (!strcmp(argv[puntero_parametro],"--menu-transparency-perc")) {

			int valor;

					siguiente_parametro_argumento();
					valor=atoi(argv[puntero_parametro]);

					if (valor<0 || valor>95) {
						printf ("Invalid menu transparency value\n");
						exit (1);
					}

        screen_menu_mix_transparency=valor;

			}


			else if (!strcmp(argv[puntero_parametro],"--menu-darken-when-open")) {
                //Lo desactivo. Esto da problemas con footer
                //mantengo opcion por compatibilidad
				//screen_menu_reduce_bright_machine.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--menu-bw-multitask")) {
				screen_machine_bw_no_multitask.v=1;
			}					


			else if (!strcmp(argv[puntero_parametro],"--hide-menu-close-button")) {
                                menu_hide_close_button.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--invert-menu-mouse-scroll")) {
                                menu_invert_mouse_scroll.v=1;
			}

            
			else if (!strcmp(argv[puntero_parametro],"--right-mouse-esc")) {
                menu_mouse_right_send_esc.v=1;
			}            

			else if (!strcmp(argv[puntero_parametro],"--allow-background-windows")) {
                                menu_allow_background_windows=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--allow-background-windows-closed-menu")) {
                                always_force_overlay_visible_when_menu_closed=1;
			}            

			else if (!strcmp(argv[puntero_parametro],"--realvideo")) {
				enable_rainbow();
			}

			else if (!strcmp(argv[puntero_parametro],"--no-detect-realvideo")) {
				autodetect_rainbow.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--tbblue-legacy-hicolor")) {
				tbblue_store_scanlines.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--tbblue-legacy-border")) {
				tbblue_store_scanlines_border.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--tbblue-no-sprite-optimization")) {
				tbblue_disable_optimized_sprites.v=1;
			}            


			/*else if (!strcmp(argv[puntero_parametro],"--tsconf-fast-render")) {
				tsconf_si_render_spritetile_rapido.v=1;
			}*/


			else if (!strcmp(argv[puntero_parametro],"--enableulaplus")) {
				command_line_ulaplus.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enablegigascreen")) {
				command_line_gigascreen.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enable16c")) {
				command_line_16c.v=1;
			}			

			else if (!strcmp(argv[puntero_parametro],"--enableinterlaced")) {
				command_line_interlaced.v=1;
			}						

			else if (!strcmp(argv[puntero_parametro],"--enablespectra")) {
				command_line_spectra.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enabletimexvideo")) {
                                command_line_timex_video.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disablerealtimex512")) {
				timex_mode_512192_real.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--enablezgx")) {
                                command_line_spritechip.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disablerealbeeper")) {
				beeper_real_enabled=0;
      }

			else if (!strcmp(argv[puntero_parametro],"--disablebeeper")) {
				beeper_enabled.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--enableturbosound")) {

				int valor;

					//TODO. No aparece en el menu el error
					avisar_opcion_obsoleta("--enableturbosound setting is obsolete since version 5.1. Use --totalaychips");
					valor=2;

				set_total_ay_chips(valor);

			}

			else if (!strcmp(argv[puntero_parametro],"--totalaychips")) {

				int valor;

					siguiente_parametro_argumento();
					valor=atoi(argv[puntero_parametro]);

					if (valor>MAX_AY_CHIPS || valor<1) {
						printf ("Invalid ay chip value\n");
						exit (1);
					}

        set_total_ay_chips(valor);

			}

			else if (!strcmp(argv[puntero_parametro],"--ay-stereo-mode")) {
				int valor;

					siguiente_parametro_argumento();
					valor=atoi(argv[puntero_parametro]);

					if (valor>5 || valor<0) {
						printf ("Invalid ay stereo mode value\n");
						exit (1);
					}
				ay3_stereo_mode=valor;
			}			

			else if (!strcmp(argv[puntero_parametro],"--ay-stereo-channel")) {
				char canal;
				siguiente_parametro_argumento();

				canal=argv[puntero_parametro][0];
				canal=letra_mayuscula(canal);

				if (canal!='A' && canal!='B' && canal!='C') {
					printf ("Invalid ay stereo channel\n");
					exit(1);
				}

				int valor;
				siguiente_parametro_argumento();
				valor=atoi(argv[puntero_parametro]);

				if (valor<0 || valor>2) {
					printf ("Invalid ay stereo channel position value\n");
					exit(1);
				}

				if (canal=='A') ay3_custom_stereo_A=valor;
				if (canal=='B') ay3_custom_stereo_B=valor;
				if (canal=='C') ay3_custom_stereo_C=valor;

			}

			else if (!strcmp(argv[puntero_parametro],"--enablespecdrum")) {
				//TODO. No aparece en el menu el error
				avisar_opcion_obsoleta("--enablespecdrum setting is obsolete since version 5.1. Use --enableaudiodac");
																audiodac_enabled.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--enableaudiodac")) {
																audiodac_enabled.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--audiodactype")) {
					siguiente_parametro_argumento();

					if (!audiodac_set_type(argv[puntero_parametro]) ) {
						printf ("Invalid audiodactype\n");
						exit (1);
					}

			}


			else if (!strcmp(argv[puntero_parametro],"--snoweffect")) {
				snow_effect_enabled.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--audiovolume")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);

			        if (valor>100 || valor<0) {
			                printf ("Invalid volume value\n");
					exit (1);
				}

        			audiovolume=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-end-exit")) {
				ay_player_exit_emulator_when_finish.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-end-no-repeat")) {
				ay_player_repeat_file.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-inf-length")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor<1 || valor>1310) {
					printf ("Invalid length value. Must be between 1 and 1310\n");
					exit(1);
				}
				ay_player_limit_infinite_tracks=valor*50;
			}

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-any-length")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor<1 || valor>1310) {
					printf ("Invalid length value. Must be between 1 and 1310\n");
					exit(1);
				}
				ay_player_limit_any_track=valor*50;
			}

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-cpc")) {
				ay_player_cpc_mode.v=1;
			}

            
			else if (!strcmp(argv[puntero_parametro],"--audiopiano-zoom")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>3) {
					printf ("Invalid audiopiano-zoom value. Must be between 1 and 3\n");
					exit(1);
				}
				audiochip_piano_zoom_x=valor;
                audiochip_piano_zoom_y=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--enable-midi")) {
				command_line_enable_midi.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--midi-client")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<0 || valor>255) {
					printf ("Invalid client value. Must be between 0 and 255\n");
					exit(1);
				}
				audio_midi_client=valor;
			}			

			else if (!strcmp(argv[puntero_parametro],"--midi-port")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<0 || valor>255) {
					printf ("Invalid port value. Must be between 0 and 255\n");
					exit(1);
				}
				audio_midi_port=valor;
			}	

			else if (!strcmp(argv[puntero_parametro],"--midi-raw-device")) {
				siguiente_parametro_argumento();
				strcpy(audio_raw_midi_device_out,argv[puntero_parametro]);
			}	

			else if (!strcmp(argv[puntero_parametro],"--midi-allow-tone-noise")) {
				midi_output_record_noisetone.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--midi-no-raw-mode")) {
				audio_midi_raw_mode=0;
			}


			else if (!strcmp(argv[puntero_parametro],"--noreset-audiobuffer-full")) {
				audio_noreset_audiobuffer_full.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--enable-silencedetector")) {
				silence_detector_setting.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disable-silencedetector")) {
				silence_detector_setting.v=0;
			}

			//opciones obsoletas pc speaker. no hacer nada. solo leer valor de parametro
			else if (!strcmp(argv[puntero_parametro],"--pcspeaker-wait-time")) {
				siguiente_parametro_argumento();
			}

            else if (!strcmp(argv[puntero_parametro],"--pcspeaker-improved")) {
            }

            else if (!strcmp(argv[puntero_parametro],"--pcspeaker-hifreq-filter")) {
            }

            else if (!strcmp(argv[puntero_parametro],"--pcspeaker-hifreq-filter-divider")) {
                siguiente_parametro_argumento();
            }

            else if (!strcmp(argv[puntero_parametro],"--pcspeaker-type")) {
                siguiente_parametro_argumento();
            }
            //fin opciones obsoletas pc speaker


            //Settings de pcspeaker siempre compilados, simplemente si no estan, no salen en la ayuda
            else if (!strcmp(argv[puntero_parametro],"--onebitspeaker-improved")) {
                audioonebitspeaker_intensive_cpu_usage=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--onebitspeaker-hifreq-filter")) {
                audioonebitspeaker_agudo_filtro=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--onebitspeaker-hifreq-filter-divider")) {

                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);

                if (valor<1 || valor>15) {
                        printf ("Invalid value for onebitspeaker-hifreq-filter-divider. Accepted values from 1 to 15\n");
                        exit(1);
                }
                audioonebitspeaker_agudo_filtro_limite=valor;
            }

            else if (!strcmp(argv[puntero_parametro],"--onebitspeaker-type")) {
                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);

                if (valor!=TIPO_ALTAVOZ_ONEBITSPEAKER_PCSPEAKER && valor !=TIPO_ALTAVOZ_ONEBITSPEAKER_RPI_GPIO) {
                        printf ("Invalid value for --onebitspeaker-type\n");
                        exit(1);
                }             

                audioonebitspeaker_tipo_altavoz=valor;
            }
			


#ifdef COMPILE_ALSA
			else if (!strcmp(argv[puntero_parametro],"--alsaperiodsize")) {
		                 siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor!=2 && valor!=4) {
                                        printf ("Invalid Alsa Period size\n");
                                        exit(1);
                                }
                                alsa_periodsize=AUDIO_BUFFER_SIZE*valor;
			}


#ifdef USE_PTHREADS
                        else if (!strcmp(argv[puntero_parametro],"--fifoalsabuffersize")) {
                                 siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<4 || valor>10) {
                                        printf ("Invalid Alsa Fifo Buffer size\n");
                                        exit(1);
                                }
				fifo_alsa_buffer_size=AUDIO_BUFFER_SIZE*valor;
                        }


#endif
#endif


#ifdef COMPILE_PULSE
#ifdef USE_PTHREADS
                        else if (!strcmp(argv[puntero_parametro],"--pulseperiodsize")) {
                                 siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                //if (valor!=2 && valor!=4 && valor!=1) {
                                if (valor<1 || valor>4) {
                                        printf ("Invalid Pulse Period size\n");
                                        exit(1);
                                }
                                pulse_periodsize=AUDIO_BUFFER_SIZE*valor;
                        }


                        else if (!strcmp(argv[puntero_parametro],"--fifopulsebuffersize")) {
                                 siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<4 || valor>10) {
                                        printf ("Invalid Pulse Fifo Buffer size\n");
                                        exit(1);
                                }
                                fifo_pulse_buffer_size=AUDIO_BUFFER_SIZE*valor;
                        }


#endif
#endif


#ifdef COMPILE_COREAUDIO

	else if (!strcmp(argv[puntero_parametro],"--fifocorebuffersize")) {
				 siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor<MIN_AUDIOCOREAUDIO_FIFO_MULTIPLIER || valor>MAX_AUDIOCOREAUDIO_FIFO_MULTIPLIER) {
								printf ("Invalid Coreaudio Fifo Buffer size\n");
								exit(1);
				}
				audiocoreaudio_fifo_buffer_size_multiplier=valor;
			}
#endif

#ifdef COMPILE_SDL
			else if (!strcmp(argv[puntero_parametro],"--sdlsamplesize")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor<128 || valor>2048) {
					printf ("Invalid SDL audio sample size\n");
					exit(1);
				}
				audiosdl_samples=valor;
			}


			else if (!strcmp(argv[puntero_parametro],"--fifosdlbuffersize")) {
						 siguiente_parametro_argumento();
						int valor=atoi(argv[puntero_parametro]);
						if (valor<MIN_AUDIOSDL_FIFO_MULTIPLIER || valor>MAX_AUDIOSDL_FIFO_MULTIPLIER) {
										printf ("Invalid SDL Fifo Buffer size\n");
										exit(1);
						}
						audiosdl_fifo_buffer_size_multiplier=valor;
					}

#endif


			//Este setting lo permitimos siempre, aunque no se haya compilado driver sdl, pues es una variable global, aunque no se verá en la ayuda
			else if (!strcmp(argv[puntero_parametro],"--sdlrawkeyboard")) {
					sdl_raw_keyboard_read.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--no-fallbacktorealtape")) {
                standard_to_real_tape_fallback.v=0;
            }

			else if (!strcmp(argv[puntero_parametro],"--anyflagloading")) {
                                tape_any_flag_loading.v=1;
                        }			

			else if (!strcmp(argv[puntero_parametro],"--autorewind")) {
                tape_auto_rewind.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--simulaterealload")) {
                                tape_loading_simulate.v=1;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--simulaterealloadfast")) {
                                tape_loading_simulate_fast.v=1;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--deletetzxpauses")) {
                                tzx_suppress_pause.v=1;
                        }


                        else if (!strcmp(argv[puntero_parametro],"--realloadfast")) {
							    accelerate_loaders.v=1;
                        }						

                        else if (!strcmp(argv[puntero_parametro],"--blue")) {
                                screen_gray_mode |=1;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--green")) {
                                screen_gray_mode |=2;
                        }

                        else if (!strcmp(argv[puntero_parametro],"--red")) {
                                screen_gray_mode |=4;
                        }

			else if (!strcmp(argv[puntero_parametro],"--inversevideo")) {
                                inverse_video.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--realpalette")) {
                                spectrum_1648_use_real_palette.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disabletooltips")) {
				tooltip_enabled.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--disable-all-first-aid")) {
				menu_disable_first_aid.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--no-first-aid")) {
				siguiente_parametro_argumento();
				menu_first_aid_disable(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--disablemenu")) {
				menu_desactivado.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disablemenuandexit")) {
				menu_desactivado_andexit.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disablemenufileutils")) {
				menu_desactivado_file_utilities.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--forcevisiblehotkeys")) {
                                menu_force_writing_inverse_color.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--forceconfirmyes")) {
				force_confirm_yes.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--gui-style")) {
				siguiente_parametro_argumento();
                int indice=menu_get_gui_index_by_name(argv[puntero_parametro]);
                if (indice<0) {
					printf ("Invalid GUI style: %s\n",argv[puntero_parametro]);
					exit(1);
				}

                estilo_gui_activo=indice;
                set_charset_from_gui();                

                /*

				int i;
				for (i=0;i<ESTILOS_GUI;i++) {
					if (!strcasecmp(argv[puntero_parametro],definiciones_estilos_gui[i].nombre_estilo)) {
						estilo_gui_activo=i;
						set_charset_from_gui();
						break;
					}
				}
				if (i==ESTILOS_GUI) {
					printf ("Invalid GUI style: %s\n",argv[puntero_parametro]);
					exit(1);
				}

                */
            }

            else if (!strcmp(argv[puntero_parametro],"--charset")) {
				siguiente_parametro_argumento();
                int indice=get_charset_id_by_name(argv[puntero_parametro]);
                if (indice<0) {
					printf ("Invalid charset: %s\n",argv[puntero_parametro]);
					exit(1);
				}

                user_charset=indice;
                set_user_charset();
               
            }



			else if (!strcmp(argv[puntero_parametro],"--keyboardspoolfile")) {
                                siguiente_parametro_argumento();
				//sprintf(input_file_keyboard_name_buffer,"%s",argv[puntero_parametro]);
				input_file_keyboard_name=argv[puntero_parametro];
			        input_file_keyboard_init();
			}

			else if (!strcmp(argv[puntero_parametro],"--keyboardspoolfile-play")) {
                input_file_keyboard_playing.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--keyboardspoolfile-keylength")) {
                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);

                if (valor>100 || valor<1) {
                    printf ("Invalid keylength\n");
                   exit (1);
                }

                input_file_keyboard_delay=valor;
			 
            }

            else if (!strcmp(argv[puntero_parametro],"--keyboardspoolfile-nodelay")) {
                input_file_keyboard_send_pause.v=0;
            }

//Si no hay soporte de pthreads, estas opciones las permitimos pero luego no hace nada

			else if (!strcmp(argv[puntero_parametro],"--enable-remoteprotocol")) {
					remote_protocol_enabled.v=1;
		 }

		 else if (!strcmp(argv[puntero_parametro],"--remoteprotocol-port")) {
            siguiente_parametro_argumento();
            int valor=atoi(argv[puntero_parametro]);

            if (valor>65535 || valor<1) {
                printf ("Invalid port value\n");
                exit (1);
            }

            remote_protocol_port=valor;
		 }

		 else if (!strcmp(argv[puntero_parametro],"--remoteprotocol-prompt")) {
            siguiente_parametro_argumento();
            strcpy(remote_prompt_command_string,argv[puntero_parametro]);
		 }


		 else if (!strcmp(argv[puntero_parametro],"--showfiredbreakpoint")) {
			 siguiente_parametro_argumento();
            int valor=parse_string_to_number(argv[puntero_parametro]);
			if (valor<0 || valor>2) {
				 printf ("Invalid port value\n");
				 exit (1);
			 }

			debug_show_fired_breakpoints_type=valor;
		 }

		 else if (!strcmp(argv[puntero_parametro],"--set-breakpoint")) {
			 siguiente_parametro_argumento();
			 int valor=atoi(argv[puntero_parametro]);
			 valor--;

			 siguiente_parametro_argumento();


			 if (valor<0 || valor>MAX_BREAKPOINTS_CONDITIONS-1) {
				 printf("Index %d out of range setting breakpoint \"%s\"\n",valor+1,argv[puntero_parametro]);
				 exit(1);
			 }

			 debug_set_breakpoint(valor,argv[puntero_parametro]);

		 }

		 else if (!strcmp(argv[puntero_parametro],"--set-watch")) {
			 siguiente_parametro_argumento();
			 int valor=atoi(argv[puntero_parametro]);
			 valor--;

			 siguiente_parametro_argumento();


			 if (valor<0 || valor>DEBUG_MAX_WATCHES-1) {
				 printf("Index %d out of range setting watch \"%s\"\n",valor+1,argv[puntero_parametro]);
				 exit(1);
			 }

			 debug_set_watch(valor,argv[puntero_parametro]);

		 }		 

		 else if (!strcmp(argv[puntero_parametro],"--set-mem-breakpoint")) {
			 siguiente_parametro_argumento();
			 int direccion=parse_string_to_number(argv[puntero_parametro]);
			 if (direccion<0 || direccion>65535) {
				 printf("Address %d out of range setting memory breakpoint\n",direccion);
				 exit(1);
			 }

			siguiente_parametro_argumento();
			 int valor=parse_string_to_number(argv[puntero_parametro]);
			 if (valor<0 || valor>255) {
				 printf("Type %d out of range setting memory breakpoint at address %04XH\n",valor,direccion);
				 exit(1);
			 }			

			 debug_set_mem_breakpoint(direccion,valor);

		 }		 


		 else if (!strcmp(argv[puntero_parametro],"--set-breakpointaction")) {
			 siguiente_parametro_argumento();
			 int valor=atoi(argv[puntero_parametro]);
			 valor--;

			 siguiente_parametro_argumento();


			 if (valor<0 || valor>MAX_BREAKPOINTS_CONDITIONS-1) {
				 printf("Index %d out of range setting breakpoint action \"%s\"\n",valor+1,argv[puntero_parametro]);
				 exit(1);
			 }

			 debug_set_breakpoint_action(valor,argv[puntero_parametro]);



		 }

		 else if (!strcmp(argv[puntero_parametro],"--enable-breakpoints")) {
		 			 command_line_set_breakpoints.v=1;
		 }

		else if (!strcmp(argv[puntero_parametro],"--show-invalid-opcode")) {
	  				debug_shows_invalid_opcode.v=1;
		}

        else if (!strcmp(argv[puntero_parametro],"--cpu-history-max-items")) {
            siguiente_parametro_argumento();

            int max_items=parse_string_to_number(argv[puntero_parametro]);


            if (max_items<1 || max_items>CPU_HISTORY_MAX_ALLOWED_ELEMENTS) {
                printf("Value for max history elements out of range\n");
                exit(1);
            }

            cpu_history_max_elements=max_items;
        }


		else if (!strcmp(argv[puntero_parametro],"--brkp-always")) {
	  				debug_breakpoints_cond_behaviour.v=0;
		}

		else if (!strcmp(argv[puntero_parametro],"--show-display-debug")) {
	  				debug_settings_show_screen.v=1;

		}


		else if (!strcmp(argv[puntero_parametro],"--show-electron-debug")) {
	  				menu_debug_registers_if_showscan.v=1;
		}

        else if (!strcmp(argv[puntero_parametro],"--show-basic-address")) {
	  				debug_view_basic_show_address.v=1;
        }


		 else if (!strcmp(argv[puntero_parametro],"--hardware-debug-ports")) {
			 hardware_debug_port.v=1;
		 }

		 else if (!strcmp(argv[puntero_parametro],"--hardware-debug-ports-byte-file")) {
			siguiente_parametro_argumento();
			strcpy(zesarux_zxi_hardware_debug_file,argv[puntero_parametro]);
		 }

		 else if (!strcmp(argv[puntero_parametro],"-—dump-ram-to-file")) {
                                siguiente_parametro_argumento();
				strcpy(dump_ram_file,argv[puntero_parametro]);
                 }

		else if (!strcmp(argv[puntero_parametro],"--dump-snapshot-panic")) {
				 debug_dump_zsf_on_cpu_panic.v=1;
		}

			else if (!strcmp(argv[puntero_parametro],"--joystickemulated")) {
                                siguiente_parametro_argumento();
				if (realjoystick_set_type(argv[puntero_parametro])) {
                                        exit(1);
				}

			}

            else if (!strcmp(argv[puntero_parametro],"--joystickfirekey")) {
                siguiente_parametro_argumento();
                int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<0 || valor>3) {
					printf ("Invalid value %d for setting --joystickfirekey\n",valor);
                    exit(1);				
				}
				joystick_defined_key_fire=valor;
            }


			else if (!strcmp(argv[puntero_parametro],"--disablerealjoystick")) {
				//realjoystick_present.v=0;
				realjoystick_disabled.v=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--no-native-linux-realjoy")) {
				no_native_linux_realjoystick.v=1;
			}
			
			else if (!strcmp(argv[puntero_parametro],"--realjoystickpath")) {
				siguiente_parametro_argumento();
				strcpy(string_dev_joystick,argv[puntero_parametro]);
				
			}

			else if (!strcmp(argv[puntero_parametro],"--realjoystickindex")) {
				siguiente_parametro_argumento();
                realjoystick_index=parse_string_to_number(argv[puntero_parametro]);	
			}            

			else if (!strcmp(argv[puntero_parametro],"--realjoystick-calibrate")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<0 || valor>32000) {
					printf ("Invalid value %d for setting --realjoystick-calibrate\n",valor);
                    exit(1);				
				}
				realjoystick_autocalibrate_value=valor;
			}


			else if (!strcmp(argv[puntero_parametro],"--joystickevent")) {
				char *text_button;
				char *text_event;

				//obtener boton
				siguiente_parametro_argumento();
				text_button=argv[puntero_parametro];

				//obtener evento
				siguiente_parametro_argumento();
				text_event=argv[puntero_parametro];

				//Y definir el evento
				if (realjoystick_set_button_event(text_button,text_event)) {
                                        exit(1);
                                }


			}

                        else if (!strcmp(argv[puntero_parametro],"--joystickkeybt")) {

				char *text_button;
                                char *text_key;

                                //obtener boton
                                siguiente_parametro_argumento();
                                text_button=argv[puntero_parametro];

                                //obtener tecla
                                siguiente_parametro_argumento();
                                text_key=argv[puntero_parametro];

				//Y definir el evento
                                if (realjoystick_set_button_key(text_button,text_key)) {
                                        exit(1);
                                }

			}


			else if (!strcmp(argv[puntero_parametro],"--joystickkeyev")) {

				char *text_event;
                                char *text_key;

                                //obtener evento
				siguiente_parametro_argumento();
				text_event=argv[puntero_parametro];

				//Y obtener tecla
				siguiente_parametro_argumento();
				text_key=argv[puntero_parametro];

	                        //Y definir el evento
                                if (realjoystick_set_event_key(text_event,text_key)) {
					exit (1);
                                }


			}

			else if (!strcmp(argv[puntero_parametro],"--clearkeylistonsmart")) {
				realjoystick_clear_keys_on_smartload.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--cleareventlist")) {
                          	realjoystick_clear_events_array();
			}


			else if (!strcmp(argv[puntero_parametro],"--enablejoysticksimulator")) {
				simulador_joystick=1;
			}


			else if (!strcmp(argv[puntero_parametro],"--quickexit")) {
				quickexit.v=1;
			}


	                 else if (!strcmp(argv[puntero_parametro],"--exit-after")) {
                         	siguiente_parametro_argumento();
	                         int valor=atoi(argv[puntero_parametro]);
				if (valor<=0) {
					printf ("Invalid value %d for setting --exit-after\n",valor);
                                 	exit(1);
				}
				exit_emulator_after_seconds=valor;
                         }


			else if (!strcmp(argv[puntero_parametro],"--zeng-remote-hostname")) {
				siguiente_parametro_argumento();
				strcpy(zeng_remote_hostname,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--zeng-remote-port")) {
				siguiente_parametro_argumento();

				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>65535) {
						printf ("Invalid value %d for setting --zeng-remote-port\n",valor);
						exit(1);
				}

				zeng_remote_port=valor;
			}


			else if (!strcmp(argv[puntero_parametro],"--zeng-snapshot-interval")) {
				siguiente_parametro_argumento();

				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>9) {
						printf ("Invalid value %d for setting --zeng-snapshot-interval\n",valor);
						exit(1);
				}

				zeng_segundos_cada_snapshot=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--zeng-iam-master")) {
				zeng_i_am_master=1;
			}



        	else if (!strcmp(argv[puntero_parametro],"--total-minutes-use")) {
				siguiente_parametro_argumento();
				total_minutes_use=parse_string_to_number(argv[puntero_parametro]);	

			}	    

			else if (!strcmp(argv[puntero_parametro],"--stats-send-already-asked")) {
				stats_asked.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--tbblue-autoconfigure-sd-already-asked")) {
				tbblue_autoconfigure_sd_asked.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--stats-send-enabled")) {
				stats_enabled.v=1;
			}		

			else if (!strcmp(argv[puntero_parametro],"--stats-uuid")) {
				siguiente_parametro_argumento();
				strcpy(stats_uuid,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--stats-disable-check-updates")) {
				stats_check_updates_enabled.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--stats-disable-check-yesterday-users")) {
				stats_check_yesterday_users_enabled.v=0;
			}			
	
			else if (!strcmp(argv[puntero_parametro],"--stats-last-avail-version")) {
				siguiente_parametro_argumento();
				strcpy(stats_last_remote_version,argv[puntero_parametro]);
			}			
			
			else if (!strcmp(argv[puntero_parametro],"--stats-speccy-queries")) {
				siguiente_parametro_argumento();
				stats_total_speccy_browser_queries=parse_string_to_number(argv[puntero_parametro]);
			}	
			
			else if (!strcmp(argv[puntero_parametro],"--stats-zx81-queries")) {
				siguiente_parametro_argumento();
				stats_total_zx81_browser_queries=parse_string_to_number(argv[puntero_parametro]);
			}	
			
			
			

                         
			
			
			
			else if (!strcmp(argv[puntero_parametro],"--last-version")) {
				siguiente_parametro_argumento();
				strcpy(last_version_string,argv[puntero_parametro]);
			}	

			else if (!strcmp(argv[puntero_parametro],"--no-show-changelog")) {
				do_no_show_changelog_when_update.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disablebetawarning")) {
				siguiente_parametro_argumento();
				strcpy(parameter_disablebetawarning,argv[puntero_parametro]);
			}	

            //Mantenido por compatibilidad con versiones antiguas
			else if (!strcmp(argv[puntero_parametro],"--windowgeometry")) {
				siguiente_parametro_argumento();
				char *nombre;
				int x,y,ancho,alto;

				nombre=argv[puntero_parametro];

				siguiente_parametro_argumento();
				x=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				y=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				ancho=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				alto=parse_string_to_number(argv[puntero_parametro]);

				if (x<0 || y<0 || ancho<0 || alto<0) {
					printf ("Invalid window geometry\n");
					exit(1);
				}

				util_add_window_geometry(nombre,x,y,ancho,alto,0,0,ancho,alto);  //mantenido por compatibilidad minimizado=maximizado=0, ancho alto antes de minimizado

			}	


			else if (!strcmp(argv[puntero_parametro],"--windowgeometry-ext")) {
				siguiente_parametro_argumento();
				char *nombre;
				int x,y,ancho,alto,is_minimized;

				nombre=argv[puntero_parametro];

				siguiente_parametro_argumento();
				x=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				y=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				ancho=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				alto=parse_string_to_number(argv[puntero_parametro]);

				if (x<0 || y<0 || ancho<0 || alto<0) {
					printf ("Invalid window geometry\n");
					exit(1);
				}

				siguiente_parametro_argumento();
				is_minimized=parse_string_to_number(argv[puntero_parametro]);

				util_add_window_geometry(nombre,x,y,ancho,alto,is_minimized,0,ancho,alto); //mantenido por compatibilidad ancho y alto antes de minimizado

			}

			else if (!strcmp(argv[puntero_parametro],"--windowgeometry-full")) {
				siguiente_parametro_argumento();
				char *nombre;
				int x,y,ancho,alto,ancho_antes_minimized,alto_antes_minimized,is_minimized;

				nombre=argv[puntero_parametro];

				siguiente_parametro_argumento();
				x=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				y=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				ancho=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				alto=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				ancho_antes_minimized=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				alto_antes_minimized=parse_string_to_number(argv[puntero_parametro]);                

				if (x<0 || y<0 || ancho<0 || alto<0) {
					printf ("Invalid window geometry\n");
					exit(1);
				}

				siguiente_parametro_argumento();
				is_minimized=parse_string_to_number(argv[puntero_parametro]);

				util_add_window_geometry(nombre,x,y,ancho,alto,is_minimized,0,ancho_antes_minimized,alto_antes_minimized); //mantenido por compatibilidad

			}            


			else if (!strcmp(argv[puntero_parametro],"--window-geometry")) {
				siguiente_parametro_argumento();
				char *nombre;
				int x,y,ancho,alto,ancho_antes_minimized,alto_antes_minimized,is_minimized,is_maximized;

				nombre=argv[puntero_parametro];

				siguiente_parametro_argumento();
				x=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				y=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				ancho=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				alto=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				ancho_antes_minimized=parse_string_to_number(argv[puntero_parametro]);

				siguiente_parametro_argumento();
				alto_antes_minimized=parse_string_to_number(argv[puntero_parametro]);                

				if (x<0 || y<0 || ancho<0 || alto<0) {
					printf ("Invalid window geometry\n");
					exit(1);
				}

				siguiente_parametro_argumento();
				is_minimized=parse_string_to_number(argv[puntero_parametro]);
                
                siguiente_parametro_argumento();
                is_maximized=parse_string_to_number(argv[puntero_parametro]);

				util_add_window_geometry(nombre,x,y,ancho,alto,is_minimized,is_maximized,ancho_antes_minimized,alto_antes_minimized); 

			}              


			else if (!strcmp(argv[puntero_parametro],"--clear-all-windowgeometry")) {
				util_clear_all_windows_geometry();
			}

			else if (!strcmp(argv[puntero_parametro],"--restorewindow")) {
				siguiente_parametro_argumento();

				//strcpy(restore_window_array[total_restore_window_array_elements++],argv[puntero_parametro]);
                add_window_to_restore(argv[puntero_parametro]);

			}

			else if (!strcmp(argv[puntero_parametro],"--restore-all-known-windows")) {
				zxvision_add_all_windows_to_restore();
			}            

            //Mantenida por compatibilidad hacia atras. esto es ahora por defecto desde version 10.2
			else if (!strcmp(argv[puntero_parametro],"--enable-restore-windows")) {
				//nada
			}

			else if (!strcmp(argv[puntero_parametro],"--disable-restore-windows")) {
				menu_reopen_background_windows_on_start.v=0;
			}            

			else if (!strcmp(argv[puntero_parametro],"--tonegenerator")) {
				siguiente_parametro_argumento();
                                 int valor=atoi(argv[puntero_parametro]);
                                if (valor<1 || valor>3) {
                                        printf ("Invalid value %d for setting --tonegenerator\n",valor);
                                        exit(1);
                                }
				audio_tone_generator=valor;
			}

            //Opcion oculta para probar modo navidad
            else if (!strcmp(argv[puntero_parametro],"--enable-christmas-mode")) {
                christmas_mode.v=1;
            }

            //sensor-set position type
			else if (!strcmp(argv[puntero_parametro],"--sensor-set")) {
				siguiente_parametro_argumento();
                int numero_sensor=parse_string_to_number(argv[puntero_parametro]); 
                if (numero_sensor<0 || numero_sensor>=MENU_VIEW_SENSORS_TOTAL_ELEMENTS) {
                    printf ("Invalid value %d for setting --sensor-set\n",numero_sensor);
                    exit(1);
                }

                siguiente_parametro_argumento();
                char *sensor_type=argv[puntero_parametro];

                int sensor_id=sensor_find(sensor_type);
                if (sensor_id<0) {
                    printf ("Invalid sensor type %s for setting --sensor-set\n",sensor_type);
                    exit(1);                    
                }

				strcpy (menu_debug_view_sensors_list_sensors[numero_sensor].short_name,sensor_type);
			}

            //sensor-set-widget position type
			else if (!strcmp(argv[puntero_parametro],"--sensor-set-widget")) {
				siguiente_parametro_argumento();
                int numero_sensor=parse_string_to_number(argv[puntero_parametro]); 
                if (numero_sensor<0 || numero_sensor>=MENU_VIEW_SENSORS_TOTAL_ELEMENTS) {
                    printf ("Invalid value %d for setting --sensor-set-widget\n",numero_sensor);
                    exit(1);
                }

                siguiente_parametro_argumento();
                char *sensor_type=argv[puntero_parametro];

                int widget_id=zxvision_widget_find_name_type(sensor_type);
                if (widget_id<0) {
                    printf ("Invalid sensor widget type %s for setting --sensor-set-widget\n",sensor_type);
                    exit(1);                    
                }

				menu_debug_view_sensors_list_sensors[numero_sensor].tipo=widget_id;
			}        

            //mostrar valor en vez de porcentaje
           //sensor-set-abs position 
			else if (!strcmp(argv[puntero_parametro],"--sensor-set-abs")) {
				siguiente_parametro_argumento();
                int numero_sensor=parse_string_to_number(argv[puntero_parametro]); 
                if (numero_sensor<0 || numero_sensor>=MENU_VIEW_SENSORS_TOTAL_ELEMENTS) {
                    printf ("Invalid value %d for setting --sensor-set-abs\n",numero_sensor);
                    exit(1);
                }

                menu_debug_view_sensors_list_sensors[numero_sensor].valor_en_vez_de_perc=1;

			}      

            else if (!strcmp(argv[puntero_parametro],"--convert-tap-tzx")) {
                siguiente_parametro_argumento();
                char *origen=argv[puntero_parametro];
                siguiente_parametro_argumento();
                char *destino=argv[puntero_parametro];

                printf("Converting from TAP file %s to TZX file %s\n",origen,destino);

                if (util_extract_tap(origen,NULL,destino,0)) {
                    printf("Error executing conversion\n");
                    exit(1);
                }

                printf("Conversion finished. Exiting\n");
                exit(0);
            }

            else if (!strcmp(argv[puntero_parametro],"--convert-tap-tzx-turbo-rg")) {
                siguiente_parametro_argumento();
                char *origen=argv[puntero_parametro];
                siguiente_parametro_argumento();
                char *destino=argv[puntero_parametro];

                printf("Converting from TAP file %s to TZX Rodolfo Guerra Turbo file %s\n",origen,destino);

                if (util_extract_tap(origen,NULL,destino,1)) {
                    printf("Error executing conversion\n");
                    exit(1);
                }

                printf("Conversion finished. Exiting\n");
                exit(0);
            }            

            else if (!strcmp(argv[puntero_parametro],"--convert-tap-pzx")) {
                siguiente_parametro_argumento();
                char *origen=argv[puntero_parametro];
                siguiente_parametro_argumento();
                char *destino=argv[puntero_parametro];

                printf("Converting from TAP file %s to PZX file %s\n",origen,destino);

                if (util_extract_tap(origen,NULL,destino,0)) {
                    printf("Error executing conversion\n");
                    exit(1);
                }

                printf("Conversion finished. Exiting\n");
                exit(0);
            }            

            else if (!strcmp(argv[puntero_parametro],"--convert-tzx-tap")) {
                siguiente_parametro_argumento();
                char *origen=argv[puntero_parametro];
                siguiente_parametro_argumento();
                char *destino=argv[puntero_parametro];

                printf("Converting from TZX file %s to TAP file %s\n",origen,destino);

                if (util_extract_tzx(origen,NULL,destino)) {
                    printf("Error executing conversion\n");
                    exit(1);
                }

                printf("Conversion finished. Exiting\n");
                exit(0);
            }

            else if (!strcmp(argv[puntero_parametro],"--convert-pzx-tap")) {
                siguiente_parametro_argumento();
                char *origen=argv[puntero_parametro];
                siguiente_parametro_argumento();
                char *destino=argv[puntero_parametro];

                printf("Converting from PZX file %s to TAP file %s\n",origen,destino);

                if (util_extract_pzx(origen,NULL,destino)) {
                    printf("Error executing conversion\n");
                    exit(1);
                }

                printf("Conversion finished. Exiting\n");
                exit(0);
            }            


			//autodetectar que el parametro es un snap o cinta. Esto tiene que ser siempre el ultimo else if
			else if (quickload_valid_extension(argv[puntero_parametro])) {
			        quickload_inicial.v=1;
                		quickload_nombre=argv[puntero_parametro];
		        }

			else {

				//parametro desconocido
				debug_printf (VERBOSE_ERR,"Unknown parameter : %s . Stopping parsing the rest of parameters",argv[puntero_parametro]);
				return 1;
				//cpu_help();
				//exit(1);
			}


		}

		//Fin de interpretacion de parametros
		return 0;

}



void emulator_main_loop(void)
{
	while (1) {
		if (menu_abierto==1) menu_inicio();

    		//Bucle principal de ejecución de la cpu
    		while (menu_abierto==0) {
    			cpu_core_loop();
    		}


	}


}

#ifdef USE_PTHREADS
void *thread_main_loop_function(void *nada)
{
        emulator_main_loop();

	//aqui no llega nunca, lo hacemos solo para que no se queje el compilador
        nada=0;
        nada++;
	return NULL;
}
#endif


void print_funny_message(void)
{
	//Mensaje gracioso de arranque que empezó con la ZXSpectr edition (ZEsarUX 4.1)
	//El primero era: Detected SoundBlaster at A220 I5 D1 T2

	//printf ("random: %d\n",randomize_noise[0]);

	//mensajes random de broma
	#define MAX_RANDOM_FUNNY_MESSAGES 23
	char *random_funny_messajes[MAX_RANDOM_FUNNY_MESSAGES]={
		"Detected SoundBlaster at A220 I5 D1 T2",
		"DOS/4GW Protected Mode Run-time  Version 1.97",		//2
		"Detected 4 MB expanded memory (EMS)",
		"64K High Memory Area is available",								//4
		"Detected Hercules Video Card 720x350",
		"Detected Enhanced Graphics Adapter (EGA) 640x350",	//6
		"Uncompressing Linux... done, booting the kernel",
		"PhoenixBIOS 4.0 Release 6.0",											//8
		"301-Keyboard not detected. Press F1 to continue",
		"Error: IRQL NOT LESS OR EQUAL",   //10
		"R Tape loading error, 0:1",
		"Software Failure. Press left mouse button to continue. Guru Meditation #00000004.000AAC0",
		"RAMTOP no good",
		"   < Sistema preparado >   ",
		"Sorry, a system error ocurred. unimplemented trap",
		"Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(179,2)",
		"Invalid MSX-DOS call",
		"B Integer out of range, 0:1",
		"Your System ate a SPARC! Gah!",
		"CMOS checksum error - Defaults loaded",
        "Proudly Made on Earth",
        "Made From 100% Recycled Pixels",
        "You have died of dysentery"
      
	};


	int mensaje_gracioso=randomize_noise[0] % MAX_RANDOM_FUNNY_MESSAGES;
	//printf ("indice mensaje gracioso: %d\n",mensaje_gracioso);
	printf ("%s ... Just kidding ;)\n\n",random_funny_messajes[mensaje_gracioso]);
							/*
							printf ("386 Processor or higher detected\n"
											"Using expanded memory (EMS)\n");
							*/
}


void check_christmas_mode(void)
{
    time_t tiempo = time(NULL);
    struct tm tm = *localtime(&tiempo);

    //printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    int dia=tm.tm_mday;
    int mes=tm.tm_mon+1; 

    //printf("Mes: %d Dia: %d\n",mes,dia);


    //Activarlo si mes 12 y dia > 19, o si mes 1 y dia < 9
    //O sea, del 20 de Diciembre hasta el 8 de Enero
    if ((mes==12 && dia>19) || (mes==1 && dia<9)) {
        debug_printf(VERBOSE_DEBUG,"Enabling christmas mode");
        christmas_mode.v=1;
    }
}


char macos_path_to_executable[PATH_MAX];

//Proceso inicial
int zesarux_main (int main_argc,char *main_argv[]) {

	if (main_argc>1) {
		if (!strcmp(main_argv[1],"--version")) {
		//	printf ("ZEsarUX Version: " EMULATOR_VERSION " Date: " EMULATOR_DATE " - " EMULATOR_EDITION_NAME "\n");
			printf ("ZEsarUX v." EMULATOR_VERSION " - " EMULATOR_EDITION_NAME ". " EMULATOR_DATE  "\n");
			exit(0);
		}
	}


    if (main_argc>1) {
            if (!strcmp(main_argv[1],"--codetests")) {
                    codetests_main(main_argc,main_argv);
                    exit(1);
            }

    }


	//de momento ponemos esto a null y los mensajes siempre saldran por un printf normal
	scr_messages_debug=NULL;

#if defined(__APPLE__)
	//Si estamos en Mac y estamos ejecutando desde bundle de la App, cambiar carpeta a directorio de trabajo
	//Esto antes estaba en el zesarux.sh, pero ahora se llama al binario para poder usar los permisos de Documents, Downloads etc
	//de MacOS Catalina

	//Cambiar a la carpeta donde estamos ejecutando el binario

	//por si acaso, por defecto a cadena vacia
	macos_path_to_executable[0]=0;

	uint32_t bufsize=PATH_MAX;

	_NSGetExecutablePath(macos_path_to_executable, &bufsize);

	if (macos_path_to_executable[0]!=0) { 

			char dir[PATH_MAX];
			util_get_dir(macos_path_to_executable,dir);

			printf ("Changing to Mac App bundle directory: %s\n",dir);
			chdir(dir);
		
	}
	/*
	Para testeo, para eliminar permisos de acceso en Catalina, ejecutar:
	tccutil reset SystemPolicyDocumentsFolder com.cesarhernandez.zesarux
	tccutil reset SystemPolicyDownloadsFolder com.cesarhernandez.zesarux
	tccutil reset SystemPolicyDesktopFolder com.cesarhernandez.zesarux
	*/
#endif


/*
Note for developers: If you are doing modifications to ZEsarUX, you should follow the rules from GPL license, as well as 
the licenses that cover all the external modules
Also, you should keep the following copyright message, beginning with "Begin Copyright message" and ending with "End Copyright message"
*/

//Begin Copyright message

	printf ("ZEsarUX - ZX Second-Emulator And Released for UniX\n"
	"https://github.com/chernandezba/zesarux\n\n"
    "Copyright (C) 2013 Cesar Hernandez Bano\n"
	"\n"
    "ZEsarUX is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n"
	"\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
	"\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n"
	"\n"
	);

	printf ("Please read the other licenses used in ZEsarUX, from the menu Help->Licenses or just open files from folder licenses/\n\n\n");

		

    //printf ("ZEsarUX Version: " EMULATOR_VERSION " Date: " EMULATOR_DATE " - " EMULATOR_EDITION_NAME "\n"
    printf ("ZEsarUX v." EMULATOR_VERSION " - " EMULATOR_EDITION_NAME ". " EMULATOR_DATE  "\n"
    
            "\n");


//End Copyright message





#ifdef DEBUG_SECOND_TRAP_STDOUT
    printf ("\n\nWARNING!!!! DEBUG_SECOND_TRAP_STDOUT enabled!!\n"
        "Enable this only when you want to find printing routines\n\n");
    sleep (3);
#endif



	//Unos cuantos valores por defecto
	cpu_step_mode.v=0;
	current_machine_type=1;
	noautoload.v=0;
	ay_speech_enabled.v=1;
	ay_envelopes_enabled.v=1;
	tapefile=NULL;
	realtape_name=NULL;
	tape_out_file=NULL;
	aofilename=NULL;
	aofile_inserted.v=0;
	vofilename=NULL;
	vofile_inserted.v=0;
	input_file_keyboard_inserted.v=0;
	input_file_keyboard_playing.v=0;
	input_file_keyboard_send_pause.v=1;


	/*
		OLD : Activado por defecto. La versión de windows, tanto pthreads, como sin threads, y validado en 5.0, 6.0 y 6,1.
		hace un clic continuo a veces. Y activando el detector, como se apaga el audio, se deja de escuchar

		Desactivado por defecto: Version windows sin pthreads, no hace click continuo aun sin tener detector de silencio
		Version windows con pthreads, hace clicks independientemente del detector de silencio
	*/

	silence_detector_setting.v=0;



	scr_z88_cpc_load_keymap=NULL;



	scrfile=NULL;
	snapfile=NULL;
	modificado_border.v=1;
	border_enabled.v=1;

	scr_putpixel=NULL;
    scr_driver_can_ext_desktop=NULL;
	//scr_putpixel_final=NULL;

	simulate_screen_zx8081.v=0;
	keyboard_issue2.v=0;
	tape_any_flag_loading.v=0;

	video_interlaced_mode.v=0;

	tape_loadsave_inserted=0;

	//tape_load_inserted.v=0;
	//tape_save_inserted.v=0;

	menu_splash_text_active.v=0;
	opcion_no_welcome_message.v=0;
	spec_smp_memory=NULL;

	autoselect_snaptape_options.v=1;

	tape_loading_simulate.v=0;
	tape_loading_simulate_fast.v=0;


    //Valores de ZX80/81
    //ZX80/81 con 16 kb
    ramtop_zx8081=16383+16384;
    ram_in_8192.v=0;
    ram_in_32768.v=0;
    ram_in_49152.v=0;
    wrx_present.v=0;
    zx8081_vsync_sound.v=0;
    //video_zx8081_shows_vsync_on_display.v=0;
    video_zx8081_estabilizador_imagen.v=1;
    //video_zx8081_decremento_x_cuando_mayor=8;

    //19KB (3+16)
    ramtop_ace=16383+16384;

    try_fallback_video.v=1;
    try_fallback_audio.v=1;

    video_fast_mode_emulation.v=0;

    simulate_lost_vsync.v=0;


    last_x_atributo=0;

    snow_effect_enabled.v=0;

    inverse_video.v=0;

    kempston_mouse_emulation.v=0;


    scr_set_driver_name("");
    audio_set_driver_name("");

    transaction_log_filename[0]=0;

    debug_printf_sem_init();

    debug_unnamed_console_init();


#ifdef COMPILE_XWINDOWS
	#ifdef USE_XEXT
	#else
	disable_shm=1;
	#endif
#endif


    texto_artistico.v=1;


    rainbow_enabled.v=0;
    autodetect_rainbow.v=1;
    autodetect_wrx.v=0;

    contend_enabled.v=1;

    zxprinter_enabled.v=0;
    zxprinter_motor.v=0;
    zxprinter_power.v=0;

    tooltip_enabled.v=1;

	autosave_snapshot_on_exit.v=0;
	autoload_snapshot_on_start.v=0;
	autosave_snapshot_path_buffer[0]=0;

	audio_ay_player_mem=NULL;

	menu_first_aid_startup=1;

	//Inicializar rutinas de cpu core para que, al parsear breakpoints del config file, donde aun no hay inicializada maquina,
	//funciones como opcode1=XX , peek(x), etc no peten porque utilizan funciones peek. Inicializar también las de puerto por si acaso
	poke_byte=poke_byte_vacio;
	poke_byte_no_time=poke_byte_vacio;
	peek_byte=peek_byte_vacio;
	peek_byte_no_time=peek_byte_vacio;	
	lee_puerto=lee_puerto_vacio;
	//lee_puerto_no_time=lee_puerto_vacio;
	out_port=out_port_vacio;	
	fetch_opcode=fetch_opcode_vacio;
	realjoystick_init=realjoystick_null_init;
	realjoystick_main=realjoystick_null_main;

	ql_readbyte_no_ports_function=ql_readbyte_no_ports_vacio;
	//realjoystick_hit=realjoystick_null_hit;

	//Inicializo tambien la de push
	push_valor=push_valor_default;



	clear_lista_teclas_redefinidas();

	debug_nested_cores_pokepeek_init();


	//esto va aqui, asi podemos parsear el establecer set-breakpoint desde linea de comandos
	init_breakpoints_table();
	init_watches_table();

	extended_stack_clear();

	last_filesused_clear();
	menu_first_aid_init();

    init_zxdesktop_configurable_icons();

	//estos dos se inicializan para que al hacer set_emulator_speed, que se ejecuta antes de init audio,
	//si no hay driver inicializado, no llamarlos
	audio_end=NULL;
	audio_init=NULL;



	quickload_inicial.v=0;
    z88_slotcard_inicial.v=0;

//Establecer rutas de utilidades externas
#if defined(__APPLE__)
    sprintf (external_tool_tar,"/usr/bin/tar");
    sprintf (external_tool_gunzip,"/usr/bin/gunzip");
#endif

	//antiguo
	//realjoystick_new_set_default_functions();

	//nuevo:
	realjoystick_init_events_keys_tables();


    //por si lanzamos un cpu_panic antes de inicializar video, que esto este a NULL y podamos detectarlo para no ejecutarlo
	scr_end_pantalla=NULL;
	memoria_spectrum=NULL;


	//Primero parseamos archivo de configuracion
	debug_parse_config_file.v=0;

    //Si hay parametro de debug parse config file...
    //main_argc,char *main_argv[])
    //printf ("%d\n",main_argc);
    if (main_argc>1) {
        if (!strcmp(main_argv[1],"--debugconfigfile")) {
            debug_parse_config_file.v=1;
        }
    }


	int noconfigfile=0;

    if (main_argc>1) {
        if (!strcmp(main_argv[1],"--noconfigfile")) {
            noconfigfile=1;
        }

        if (!strcmp(main_argv[1],"--configfile")) {
            customconfigfile=main_argv[2];
        }

        //Si help es el primer parametro, procesarlo aquí y no parsear config file ni hacer nada mas
        //Nota: esto no sería estrictamente necesario, el help también se procesa en parse_cmdline_options,
        //pero si tenemos setting verbose en el archivo de config, antes del help se ven por consola
        //varios mensajes referentes a insert recent file, setting joystick type, etc
        //asi mejor el help lo proceso aqui y evito todos esos mensajes por consola al usuario
        if (!strcmp(main_argv[1],"--help")) {
            cpu_help();
            exit(1);
        }

        //Lo mismo para experthelp
        if (!strcmp(main_argv[1],"--experthelp")) {
            cpu_help_expert();
            exit(1);
        }

    }



	if (noconfigfile==0) {
        //parametros del archivo de configuracion
        configfile_parse();

        argc=configfile_argc;
        argv=configfile_argv;
        puntero_parametro=0;

        if (parse_cmdline_options()) {
            //debug_printf(VERBOSE_ERR,"Error parsing configuration file. Disabling autosave feature");
            //Desactivamos autoguardado para evitar que se genere una configuración incompleta
            save_configuration_file_on_exit.v=0;
        }
	}


  	//Luego parseamos parametros por linea de comandos
  	argc=main_argc;
  	argv=main_argv;
  	puntero_parametro=0;

  	if (parse_cmdline_options()) {
		printf ("\n\n");
        cpu_help();
        exit(1);
	}

	if (test_config_and_exit.v) exit(0);

	//Init random value. Usado en AY Chip y Random ram y mensajes "kidding"
    init_randomize_noise_value();

#ifdef SNAPSHOT_VERSION
	printf ("Build number: " BUILDNUMBER "\n");

	printf ("WARNING. This is a Snapshot version and not a stable one\n"
			 "Some features may not work, random crashes could happen, abnormal CPU use, or lots of debug messages on console\n\n");

	//Si no coincide ese parametro, hacer pausa
	if (strcmp(parameter_disablebetawarning,EMULATOR_VERSION)) {
		sleep (3);
	}
#endif


	print_funny_message();



#ifdef MINGW
        //Si no se ha pasado ningun parametro, ni parametro --nodisableconsole, sea en consola o en archivo de configuracion, liberar consola, con pausa de 2 segundos para que se vea un poco :P
        if (main_argc==1 && windows_no_disable_console.v==0) {
                sleep(2);
                printf ("Disabling text printing on this console. Specify --nodisableconsole or any other command line setting to avoid it\n");
                FreeConsole();
        }
#endif



    //guardamos zoom original. algunos drivers, como fbdev, lo modifican.
    zoom_x_original=zoom_x;
    zoom_y_original=zoom_y;


    //Pausa para leer texto de inicio, copyright, etc
    //desactivada sleep(1);

    //Inicializacion maquina



	init_cpu_tables();

	//movemos esto antes, asi podemos parsear el establecer set-breakpoint desde linea de comandos
	//init_breakpoints_table();


	init_ulaplus_table();
	init_prism_palettes();
		//init_cpc_rgb_table();
	screen_init_colour_table();

    screen_init_ext_desktop();
    init_visual_real_tape();
	init_screen_addr_table();

	init_cpc_line_display_table();

#ifdef EMULATE_VISUALMEM
	init_visualmembuffer();
#endif

	menu_debug_daad_init_flagobject();

	inicializa_tabla_contend_speed_higher();



#ifdef USE_LINUXREALJOYSTICK

	//Soporte nativo de linux joystick
	if (no_native_linux_realjoystick.v==0) {
		realjoystick_init=realjoystick_linux_init;
		realjoystick_main=realjoystick_linux_main;
	}
#endif	


	TIMESENSOR_INIT();


	debug_printf (VERBOSE_INFO,"Starting emulator");




#ifdef USE_PTHREADS
		debug_printf (VERBOSE_INFO,"Using phtreads");
#else
		debug_printf (VERBOSE_INFO,"Not using phtreads");
#endif

	if (silence_detector_setting.v) debug_printf (VERBOSE_INFO,"Enabling Silence Detector");
	else debug_printf (VERBOSE_INFO,"Disabling Silence Detector");


	debug_printf (VERBOSE_INFO,"Initializing Machine");

	//Si hemos especificado una rom diferente por linea de comandos


	if (param_custom_romfile!=NULL) {
		set_machine(param_custom_romfile);
	}


	else {
		set_machine(NULL);
	}

	cold_start_cpu_registers();

	reset_cpu();

	//Algun parametro que se resetea con reset_cpu y/o set_machine y se puede haber especificado por linea de comandos
	if (command_line_zx8081_vsync_sound.v) zx8081_vsync_sound.v=1;


    //Inicializamos Video antes que el resto de cosas.
    main_init_video();

    //llamar a set_menu_gui_zoom para establecer zoom menu. Ya se ha llamado desde set_machine pero como no hay driver de video aun ahi,
    //no se aplica zoom de gui dado que eso solo es para driver xwindows, sdl etc y no para curses y otros
    set_menu_gui_zoom();

  //Activar deteccion automatica de rutina de impresion de caracteres, si conviene
	//Esto se hace tambien al inicializar cpu... Pero como al inicializar cpu aun no hemos inicializado driver video,
	//y por tanto no se sabe si hay stdout... Entonces hacemos esto justo despues de inicializar video
  //Activar deteccion automatica de rutina de impresion de caracteres, si conviene
    if (chardetect_detect_char_enabled.v) {
        chardetect_init_automatic_char_detection();
    }



    set_putpixel_zoom();
	menu_init_footer();

    //Muy al principio cargar scr file de fondo, para que al entrar ya se vea
    zxdesktop_draw_scrfile_load();


	//Despues de inicializar video, llamar a esta funcion, por si hay que cambiar frameskip (especialmente en cocoa)
	screen_set_parameters_slow_machines();


	tape_init();

    tape_out_init();

	//Si hay realtape insertado
	if (realtape_name!=NULL) realtape_insert();


	main_init_audio();



	init_chip_ay();
	init_chip_sn();
	ay_init_filters();
	sn_init_filters();
	
	mid_reset_export_buffers();



	//Inicializar joystick en caso de linux native o simulador
	if (realjoystick_is_linux_native() || simulador_joystick) {
		realjoystick_initialize_joystick();
	}


	if (aofilename!=NULL) {
			init_aofile();
	}

    if (vofilename!=NULL) {
        init_vofile();
    }



	//Load Screen
	if (scrfile!=NULL) {
		load_screen(scrfile);
	}


    //ajustar estilo del gui, si driver video permite ese estilo o si hay que ir al primero que permita
    menu_adjust_gui_style_to_driver();

    //Crear iconos de ejemplo justo aqui despues que ya esta definido zxdesktop, los anchos de pantalla, etc etc
    create_default_zxdesktop_configurable_icons();

    //despues de leer los iconos de la config, e inicializar pantalla, ver si algun icono esta en alguna posicion que no debe y reasignar
    zxvision_check_all_configurable_icons_positions();

    set_last_dimensiones_ventana();

	scr_refresca_pantalla();



	//Capturar segmentation fault
	//desactivado normalmente en versiones snapshot
	signal(SIGSEGV, segfault_signal_handler);

	//Capturar floating point exception
	//desactivado normalmente en versiones snapshot
	signal(SIGFPE, floatingpoint_signal_handler);

  //Capturar sigbus. 
  //desactivado normalmente en versiones snapshot
#ifndef MINGW	
    signal(SIGBUS, sigbus_signal_handler);	
#endif

	//Capturar segint (CTRL+C)
	signal(SIGINT, segint_signal_handler);

	//Capturar segterm
	signal(SIGTERM, segterm_signal_handler);

#ifndef MINGW	
	//Capturar sigpipe
	signal(SIGPIPE, sigpipe_signal_handler);	
#endif


	//Restaurar ventanas, si conviene. Hacerlo aqui y no mas tarde, para evitar por ejemplo que al salir el logo de splash
	//aparezcan las ventanas en background
	zxvision_restore_windows_on_startup();

	//Inicio bucle principal
	reg_pc=0;
	interrupcion_maskable_generada.v=0;
	interrupcion_non_maskable_generada.v=0;
	interrupcion_timer_generada.v=0;

	z80_halt_signal.v=0;

	esperando_tiempo_final_t_estados.v=0;
	framescreen_saltar=0;

    //Si modo navidad
    check_christmas_mode();


	if (opcion_no_welcome_message.v==0) {
		set_welcome_message();
	}

	else {
		//Cuando hay splash, la propia funcion set_welcome_message llama a cls_menu_overlay y esta llama a menu_draw_ext_desktop
        //y luego a show_all_windows_startup
		menu_draw_ext_desktop();
        show_all_windows_startup();
	}



	//Algun parametro que se resetea con reset_cpu y/o set_machine y se puede haber especificado por linea de comandos
	if (command_line_wrx.v) enable_wrx();

	if (command_line_load_binary_file!=NULL) {
		load_binary_file(command_line_load_binary_file,command_line_load_binary_address,command_line_load_binary_length);
	}



	if (command_line_chardetect_printchar_enabled != -1) {
		chardetect_printchar_enabled.v=command_line_chardetect_printchar_enabled;
	}

	if (command_line_spectra.v) spectra_enable();

	if (command_line_ulaplus.v) enable_ulaplus();

	if (command_line_gigascreen.v) enable_gigascreen();

	if (command_line_16c.v) enable_16c_mode();

	if (command_line_interlaced.v) enable_interlace();		

	if (command_line_timex_video.v) enable_timex_video();

	if (command_line_spritechip.v) spritechip_enable();


	if (command_line_chroma81.v) enable_chroma81();

	//MMC
    //Antes ver si hay que copiar archivos
    util_copy_files_to_mmc_doit();

	if (command_line_mmc.v) mmc_enable();

	if (command_line_divmmc_ports.v) {
		divmmc_mmc_ports_enable();
	}

	if (command_line_divmmc_paging.v) {
		divmmc_diviface_enable();
	}

	if (command_line_divmmc.v) {
		divmmc_mmc_ports_enable();
		divmmc_diviface_enable();
	}

	if (command_line_zxmmc.v) zxmmc_emulation.v=1;
	if (command_line_8bitide.v) eight_bit_simple_ide_enable();


	//IDE
	if (command_line_ide.v) ide_enable();

	if (command_line_divide_ports.v) {
		divide_ide_ports_enable();
	}

	if (command_line_divide_paging.v) {
		divide_diviface_enable();
	}

	if (command_line_divide.v) {
		divide_ide_ports_enable();
		divide_diviface_enable();
    }



	if (command_line_esxdos_handler.v) {
		esxdos_handler_enable();

		// Solo meter el directorio local si esta habilitado esxdos
        	if (command_line_esxdos_local_dir.v) {
	                // Esto lo hacemos aqui porque antes en el esxdos_handler_enable se inicializa tambien esxdos y por tanto se borra esxdos_handler_cwd
        	        strcpy(esxdos_handler_cwd,command_line_esxdos_local_dir_path);
        	}
	}


	if (command_line_zxpand.v) zxpand_enable();


	//Dandanator
	if (command_line_dandanator.v) dandanator_enable();
	if (command_line_dandanator_push_button.v) dandanator_press_button();

	//Superupgrade
	if (command_line_superupgrade.v) superupgrade_enable(1);

	//Kartusho
	if (command_line_kartusho.v) kartusho_enable();

	//iFrom
	if (command_line_ifrom.v) ifrom_enable();	

	//Betadisk
	if (command_line_betadisk.v) {
		betadisk_enable();
		//Gestionar autoboot. Si este betadisk_enable estuviese antes del hacer el reset_cpu desde aqui,
		//no haria falta este truco
		betadisk_reset();
	}

	if (command_line_trd.v) trd_enable();

	if (command_line_dsk.v) dskplusthree_enable();

    //hilow
    if (command_line_hilow.v) {
        hilow_enable();
    }

    //load source code
    if (command_line_load_source_code.v) {
        int retorno=remote_load_source_code(command_line_load_source_code_file);
        if (retorno) {
            debug_printf(VERBOSE_ERR,"Error loading source code from file %s",command_line_load_source_code_file);
        }
    }    

	if (command_line_set_breakpoints.v) {
		if (debug_breakpoints_enabled.v==0) {
		        debug_breakpoints_enabled.v=1;
		        breakpoints_enable();
		}

	}

	if (command_line_enable_midi.v) {
		if (audio_midi_output_init() ) debug_printf (VERBOSE_ERR,"Error initializing midi device");
	}


	//Si la version actual es mas nueva que la anterior, eso solo si el autoguardado de config esta activado
	if (save_configuration_file_on_exit.v && do_no_show_changelog_when_update.v==0) {
		//if (strcmp(last_version_string,EMULATOR_VERSION)) {  //Si son diferentes
		if (strcmp(last_version_string,BUILDNUMBER) && last_version_string[0]!=0) {  //Si son diferentes y last_version_string no es nula
			//Y si driver permite menu normal
			if (si_normal_menu_video_driver()) {
				menu_event_new_version_show_changes.v=1;
				menu_set_menu_abierto(1);
				//menu_abierto=1;
			}
		}
	}




	start_timer_thread();

	gettimeofday(&z80_interrupts_timer_antes, NULL);


	//Apuntar momento de inicio para estadisticas-uptime
	gettimeofday(&zesarux_start_time, NULL);


	//antes de cargar otros snapshots por linea de comandos, ver si hay autocarga
	//si hay autocarga, pero luego se indica otro snapshot, se cargara el del autoload pero despues el indicado...
	//y por tanto la autocarga no permanece
	if (autoload_snapshot_on_start.v) {
		autoload_snapshot();
	}


	//Ver si hay que cargar snapshot. considerar quickload
	if (quickload_inicial.v==1) {
		debug_printf(VERBOSE_INFO,"Smartloading %s",quickload_nombre);
		quickload(quickload_nombre);
	}

	else {
		debug_printf(VERBOSE_INFO,"See if we have to load snapshot...");
	    snapshot_load();
	}

    //Ver si hay que insertar slot de z88
    if (z88_slotcard_inicial.v) {
        if (!MACHINE_IS_Z88) {
            debug_printf(VERBOSE_ERR,"Trying to insert a Z88 slot but current machine is not Z88");
        }
        else {
            z88_load_eprom_card(z88_slotcard_inicial_nombre,z88_slotcard_inicial_slot);
        }
    }

	//Poner esto aqui porque se resetea al establecer parametros maquina en set_machine_params
	if (command_line_vsync_minimum_lenght) {
		minimo_duracion_vsync=command_line_vsync_minimum_lenght;
	}


	init_network_tables();

	//Iniciar ZRCP
	init_remote_protocol();

	//Funciones de red en background
	stats_check_updates();
	send_stats_server();
	stats_check_yesterday_users();


	//printf("menu abierto: %d menu_overlay_activo: %d\n",menu_abierto,menu_overlay_activo);


	//Restaurar ventanas, si conviene
	//zxvision_restore_windows_on_startup();	

	//printf("menu abierto: %d menu_overlay_activo: %d\n",menu_abierto,menu_overlay_activo);

	//Inicio bucle de emulacion


//En SDL2 y SDL1, rutinas de refresco de pantalla se deben lanzar desde el thread principal. Por tanto:
#ifdef COMPILE_SDL
	if (!strcmp(driver_screen,"sdl")) {
		debug_printf (VERBOSE_INFO,"Calling main loop emulator on the main thread as it is required by SDL");
		emulator_main_loop();

		//Aqui no se llega nunca pero por si acaso
		return 0;
	}
#endif

	//si no tenemos pthreads, entrar en el bucle principal tal cual
	//si hay pthreads, lanzarlo como thread aparte y quedarnos en un bucle con sleep

#ifdef USE_PTHREADS


	//Esto deberia estar disponible en todos menos en Windows. Logicamente si USE_PTHREADS esta habilitado
	ver_si_enable_thread_main_loop();


	if (si_thread_main_loop) {
		debug_printf (VERBOSE_INFO,"Calling main loop emulator on a thread");
                if (pthread_create( &thread_main_loop, NULL, &thread_main_loop_function, NULL) ) {
                        cpu_panic("Can not create main loop pthread");
                }
	}

	else {
		debug_printf (VERBOSE_INFO,"Calling main loop emulator without threads (although pthreads are available)");
		emulator_main_loop();
		//De aqui hacia abajo no se deberia llegar nunca... ya que esto es para pthreads y windows
		//(y lo de abajo es para cocoa y mas abajo para sistemas sin pthreads)
	}



	#ifdef USE_COCOA

    //Si hay soporte COCOA, dejar solo el thread con el main loop y volver a main (de scrcocoa)

	#else
    //Bucle cerrado con sleep. El bucle main se ha lanzado como thread
    while (1) {
        timer_sleep(1000);
        //printf ("bucle con sleep\n");
    }
	#endif



#else
	debug_printf (VERBOSE_INFO,"Calling main loop emulator without threads");
	emulator_main_loop();
#endif

	//Aqui solo se llega en caso de cocoa

	return 0;

}

void dump_ram_file_on_exit(void)
{
	if (dump_ram_file[0]) {

			debug_printf (VERBOSE_INFO,"Dumping ram contents to file %s",dump_ram_file);

                        //Crear archivo vacio
                        FILE *ptr_ramfile;
                        ptr_ramfile=fopen(dump_ram_file,"wb");

                        int totalsize=49152;

                        z80_byte valor_grabar;
			z80_int dir=16384;

                        if (ptr_ramfile!=NULL) {
                                while (totalsize) {
					valor_grabar=peek_byte_no_time(dir++);
                                        totalsize--;

                                        fwrite(&valor_grabar,1,1,ptr_ramfile);
                                }
                                fclose(ptr_ramfile);
                        }

			else {
				debug_printf(VERBOSE_ERR,"Error writing dump ram file");
			}

	}
}

int ending_emulator_flag=0;

//Se pasa parametro que dice si guarda o no la configuración.
//antes se guardaba siempre, pero ahora en casos de recepcion de señales de terminar, no se guarda,
//pues generaba segfaults en las rutinas de guardar ventanas (--restorewindow)
void end_emulator_saveornot_config(int saveconfig)
{
	debug_printf (VERBOSE_INFO,"End emulator");
	
    ending_emulator_flag=1;
	
	
	

	dump_ram_file_on_exit();

	top_speed_timer.v=0;

//Si se ha llamado aqui desde otro sitio que no sea el pthread del main_loop_emulator, hay que destruir antes el pthread con:
//#ifdef USE_PTHREADS
// if (si_thread_main_loop) {
//        debug_printf (VERBOSE_INFO,"Ending main loop thread");
//        pthread_cancel(thread_main_loop);
// }
//#endif

	menu_abierto=0;

	if (saveconfig && save_configuration_file_on_exit.v) {
		int uptime_seconds=timer_get_uptime_seconds();
  
  		total_minutes_use +=uptime_seconds/60;
		util_write_configfile();
	}

	//end_remote_protocol(); porque si no, no se puede finalizar el emulador desde el puerto telnet
	if (!remote_calling_end_emulator.v) {
		end_remote_protocol();
	}

	reset_menu_overlay_function();

	cls_menu_overlay();

	close_aofile();
	close_vofile();
	close_zxprinter_bitmap_file();
	close_zxprinter_ocr_file();

	//Flush write devices
	zxuno_flush_flash_to_disk();	
	z88_flush_eprom_or_flash_to_disk();
	mmc_flush_flash_to_disk();
	ide_flush_flash_to_disk();
	trd_flush_contents_to_disk();
	superupgrade_flush_flash_to_disk();
    hilow_flush_contents_to_disk();

	audio_midi_output_finish();


	audio_thread_finish();
	audio_playing.v=0;
	audio_end();

	//printf ("footer: %d\n",menu_footer);

	//Desactivo footer para que no se actualice, sino a veces aparece el footer (cpu, fps, etc) en color grisaceo mientras hace el fade
	//menu_footer=0;

	//Parece ser que el fadeout y en particular el refresco de pantalla no sienta muy bien
	//cuando se ejecuta desde remote protocol y con el driver cocoa gl. No se muy bien porque,
	//probado a poner un check de que no se refresque dos veces simultaneamente la pantalla y aun asi peta con segmentation fault
	if (!remote_calling_end_emulator.v) {
		if (!no_fadeout_exit.v) {
			scr_fadeout();
		}
	}


  scr_end_pantalla();

	//Borrar archivos de speech en Windows
	//Borrar archivo de lock
	textspeech_borrar_archivo_windows_lock_file();
	//Borrar archivo de speech
	textspeech_borrar_archivo_windows_speech_file();


	if (remote_calling_end_emulator.v) end_remote_protocol();

  exit(0);

}


void end_emulator(void)
{
	end_emulator_saveornot_config(1);
}

void end_emulator_autosave_snapshot(void)
{

    if (autosave_snapshot_on_exit.v) autosave_snapshot();

    end_emulator();
}
