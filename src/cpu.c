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
   CPU related functions
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




#include "cpu.h"
#include "start.h"
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
#include "pd765.h"
#include "core_reduced_spectrum.h"
#include "baseconf.h"
#include "settings.h"
#include "datagear.h"
#include "network.h"
#include "stats.h"
#include "zeng.h"
#include "hilow_datadrive.h"
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
#include "hilow_barbanegra.h"
#include "transtape.h"
#include "mhpokeador.h"
#include "specmate.h"
#include "phoenix.h"
#include "defcon.h"
#include "ramjet.h"
#include "interface007.h"
#include "dinamid3.h"
#include "dsk.h"
#include "plus3dos_handler.h"
#include "pcw.h"


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




int zoom_x=2,zoom_y=2;
int zoom_x_original,zoom_y_original;

//Cambiar zoom a 1 cuando cambio a máquina Next, o QL, o cualquiera con GUI zoom a 2
z80_bit autochange_zoom_big_display={1};

struct timeval z80_interrupts_timer_antes, z80_interrupts_timer_ahora;

struct timeval zesarux_start_time;

long z80_timer_difftime, z80_timer_seconds, z80_timer_useconds;



//punteros a funciones de inicio para hacer fallback de video, funciones de set
driver_struct scr_driver_array[MAX_SCR_INIT];
int num_scr_driver_array=0;

//punteros a funciones de inicio para hacer fallback de audio, funciones de set
driver_struct audio_driver_array[MAX_AUDIO_INIT];
int num_audio_driver_array=0;




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




//SDL : Si se lee teclado mediante scancodes raw en vez de usar localizacion de teclado
z80_bit sdl_raw_keyboard_read={0};

//Si se usa core de spectrum reducido o no
#ifdef USE_REDUCED_CORE_SPECTRUM
z80_bit core_spectrum_uses_reduced={1};
#else
z80_bit core_spectrum_uses_reduced={0};
#endif







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

/*
el valor del vector de interrupciones es I*256+FFh simplemente porque durante la INT,
la ULA siempre está generando el borde, por lo que el valor del bus flotante es "todo a alta impedancia" y eso,
en el Spectrum significa "1"
*/
z80_int get_im2_interrupt_vector(void)
{
    return reg_i*256+get_ula_databus_value();
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



//Si no cambiamos parametros de frameskip y otros cuando es maquina lenta (raspberry)
//Desde ZEsarUX 10.3 no cambiar dichos parametros
z80_bit cambio_parametros_maquinas_lentas={0};



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





//Si se sale del emulador despues de X segundos. A 0 para desactivarlo
int exit_emulator_after_seconds=0;

int exit_emulator_after_seconds_counter=0;



//el id de version (por ejemplo "10.2") segun la config
char last_version_text_string[255]="";


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


	do_not_run_init_z88_memory_slots=1;


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

    do_not_run_init_z88_memory_slots=0;

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

    if (esxdos_umount_on_reset.v) {
        if (esxdos_handler_enabled.v) {
            debug_printf(VERBOSE_DEBUG,"Disabling esxdos handler due to reset");
            esxdos_handler_disable();
        }
        esxdos_umount_on_reset.v=0;
    }

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

    if (MACHINE_IS_PCW) {
        pcw_reset();
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

	if (MACHINE_IS_TIMEX_TS_TC_2068) timex_set_memory_pages();

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

	if (hilow_bbn_enabled.v) {
		hilow_bbn_reset();
	}

	if (transtape_enabled.v) {
		transtape_reset();
	}

	if (specmate_enabled.v) {
		specmate_reset();
	}

	if (phoenix_enabled.v) {
		phoenix_reset();
	}

	if (defcon_enabled.v) {
		defcon_reset();
	}

	if (ramjet_enabled.v) {
		ramjet_reset();
	}

	if (interface007_enabled.v) {
		interface007_reset();
	}

	if (dinamid3_enabled.v) {
		dinamid3_reset();
	}

    if (pd765_enabled.v) {
        pd765_reset();
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

                            " TC2048   Timex Computer 2048\n"
                            " TC2068   Timex Computer 2068\n"
                            " TS1000   Timex Sinclair 1000\n"
                            " TS1500   Timex Sinclair 1500\n"
							" TS2068   Timex Sinclair 2068\n"

							" Inves    Inves Spectrum+\n"
                            " 48ks     ZX Spectrum+ 48k (Spanish)\n"
							" 128ks    ZX Spectrum+ 128k (Spanish)\n"


                        " TK80     Microdigital TK80\n"
                        " TK82     Microdigital TK82\n"
                        " TK82C    Microdigital TK82C\n"
                        " TK83     Microdigital TK83\n"
                        " TK85     Microdigital TK85\n"
					    " TK90X    Microdigital TK90X\n"
					    " TK90XS   Microdigital TK90X (Spanish)\n"
					    " TK95     Microdigital TK95\n"
                        " TK95S    Microdigital TK95 (Spanish)\n"

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
                            " CPC664   Amstrad CPC 664\n"
                            " CPC6128  Amstrad CPC 6128\n"
                            " PCW8256  Amstrad PCW 8256\n"
                            " PCW8512  Amstrad PCW 8512\n"

							" MSX1     MSX1\n"
							" Coleco   Colecovision\n"
							" SG1000   Sega SG1000\n"
                            " SMS      Sega Master System\n"
							" SVI318   Spectravideo SVI 318\n"
							" SVI328   Spectravideo SVI 328\n"
							;




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


//Esta lista no tiene por que estar ordenada por id ni por nombre
//puede estar desordenada
//Pero la mantengo ordenada por id de maquina solo por motivo estetico
struct s_machine_names machine_names[]={
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
    {"Timex Sinclair 2068",   			17},
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
    {"Timex Computer 2048",   			MACHINE_ID_TIMEX_TC2048},
    {"Timex Computer 2068",   			MACHINE_ID_TIMEX_TC2068},
    {"Microdigital TK95 (Spanish)",		MACHINE_ID_MICRODIGITAL_TK95_SPA},

    {"MSX1",MACHINE_ID_MSX1},
    {"ColecoVision",MACHINE_ID_COLECO},
    {"SG-1000",MACHINE_ID_SG1000},
    {"Spectravideo 318",MACHINE_ID_SVI_318},
    {"Spectravideo 328",MACHINE_ID_SVI_328},
    {"Master System",MACHINE_ID_SMS},

    {"ZX80",  				120},
    {"ZX81",  				121},
    {"Jupiter Ace",  			122},
    {"Timex Sinclair 1000",   			MACHINE_ID_TIMEX_TS1000},
    {"Timex Sinclair 1500",   			MACHINE_ID_TIMEX_TS1500},
    {"Microdigital TK80",		MACHINE_ID_MICRODIGITAL_TK80},
    {"Microdigital TK82",		MACHINE_ID_MICRODIGITAL_TK82},
    {"Microdigital TK82C",		MACHINE_ID_MICRODIGITAL_TK82C},
    {"Microdigital TK83",		MACHINE_ID_MICRODIGITAL_TK83},
    {"Microdigital TK85",		MACHINE_ID_MICRODIGITAL_TK85},
    {"Z88",  				130},
    {"CPC 464",  			MACHINE_ID_CPC_464},
    {"CPC 4128",  			MACHINE_ID_CPC_4128},
    {"CPC 664",  			MACHINE_ID_CPC_664},
    {"CPC 6128",  			MACHINE_ID_CPC_6128},
    {"PCW 8256",            MACHINE_ID_PCW_8256},
    {"PCW 8512",            MACHINE_ID_PCW_8512},
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


          else if (MACHINE_IS_TIMEX_TS_TC_2068) {

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

	else if (MACHINE_IS_CPC_6128) {

		//48 kb rom
		//128 kb ram.

                malloc_machine(176*1024);
                random_ram(memoria_spectrum,176*1024);

                cpc_init_memory_tables();
                cpc_set_memory_pages();


        }

	else if (MACHINE_IS_CPC_664) {

		//48 kb rom
		//128 kb ram. Asignamos 128kb ram aunque maquina sea de 64 kb de ram->Facilita las funciones

                malloc_machine(176*1024);
                random_ram(memoria_spectrum,176*1024);

                cpc_init_memory_tables();
                cpc_set_memory_pages();


        }

	else if (MACHINE_IS_PCW_8256) {

		//256kb todo ram. Aunque asignamos 2 MB para tener el máximo de memoria ya disponible
                pcw_total_ram=256*1024;

                malloc_machine(2*1024*1024);
                random_ram(memoria_spectrum,2*1024*1024);

                pcw_init_memory_tables();
                pcw_set_memory_pages();


        }

	else if (MACHINE_IS_PCW_8512) {

		//512kb todo ram. Aunque asignamos 2 MB para tener el máximo de memoria ya disponible
                pcw_total_ram=512*1024;

                malloc_machine(2*1024*1024);
                random_ram(memoria_spectrum,2*1024*1024);

                pcw_init_memory_tables();
                pcw_set_memory_pages();


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
31=tk95s
32-39=Reservadas otras Spectrum

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
123=Timex TS1000
124=Timex TS1500
125=Microdigital TK80
126=Microdigital TK82
127=Microdigital TK82C
128=Microdigital TK83
129=Microdigital TK85

130=z88 (old 30)

140=amstrad cpc464
141=amstrad cpc4128
142=amstrad cpc664
143=amstrad cpc6128

150=Sam Coupe (old 50)
151-59 reservado para otros sam (old 51-59)

160=QL Standard
161-179 reservado para otros QL

180=MK14 Standard
181-189 reservado para otros MK14

190=Amstrad PCW 8256
191=Amstrad PCW 8512
192-199 reservado para otros PCW
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



        //Ya que se inicializa la maquina, todas las rutinas de nested core se pierden y hay que desactivar los interfaces asociados
		dandanator_enabled.v=0;
		superupgrade_enabled.v=0;
		kartusho_enabled.v=0;
		ifrom_enabled.v=0;
		betadisk_enabled.v=0;
		hilow_enabled.v=0;
        samram_enabled.v=0;
        hilow_bbn_enabled.v=0;
        mhpokeador_enabled.v=0;
        multiface_enabled.v=0;
        transtape_enabled.v=0;
        specmate_enabled.v=0;
        phoenix_enabled.v=0;
        defcon_enabled.v=0;
        ramjet_enabled.v=0;
        interface007_enabled.v=0;
        dinamid3_enabled.v=0;
        textadv_location_desc_enabled.v=0;

		plus3dos_traps.v=0;
		pd765_enabled.v=0;

        //para que iba a querer desactivar esto ?
		//dskplusthree_emulation.v=0;

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

		else if (MACHINE_IS_PCW) {
			cpu_core_loop_active=CPU_CORE_PCW;
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

        //TODO: de momento core Spectrum
		else if (MACHINE_IS_PCW) {
			cpu_core_loop_active=CPU_CORE_SPECTRUM;
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
                //Habilitar pd765 a no ser que los traps esten activados
                if (plus3dos_traps.v==0) pd765_enable();
				//plus3dos_traps.v=1;
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

               else if (MACHINE_IS_TIMEX_TS_TC_2068) {
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


		else if (MACHINE_IS_CPC) {
                        contend_read=contend_read_cpc;
                        contend_read_no_mreq=contend_read_no_mreq_cpc;
                        contend_write_no_mreq=contend_write_no_mreq_cpc;

                        ula_contend_port_early=ula_contend_port_early_cpc;
                        ula_contend_port_late=ula_contend_port_late_cpc;

                        if (MACHINE_IS_CPC_HAS_FLOPPY) {
                            pd765_enable();
                        }


                }


		else if (MACHINE_IS_PCW) {
			contend_read=contend_read_pcw;
			contend_read_no_mreq=contend_read_no_mreq_pcw;
			contend_write_no_mreq=contend_write_no_mreq_pcw;

			ula_contend_port_early=ula_contend_port_early_pcw;
			ula_contend_port_late=ula_contend_port_late_pcw;

            pd765_enable();

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

                case MACHINE_ID_MICRODIGITAL_TK90X:
                case MACHINE_ID_MICRODIGITAL_TK90X_SPA:
                case MACHINE_ID_MICRODIGITAL_TK95:
                case MACHINE_ID_MICRODIGITAL_TK95_SPA:
                poke_byte=poke_byte_spectrum_48k;
                peek_byte=peek_byte_spectrum_48k;
		peek_byte_no_time=peek_byte_no_time_spectrum_48k;
		poke_byte_no_time=poke_byte_no_time_spectrum_48k;
                lee_puerto=lee_puerto_spectrum;
                break;



                case MACHINE_ID_SPECTRUM_128:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		peek_byte_no_time=peek_byte_no_time_spectrum_128k;
		poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
		ay_chip_present.v=1;
                break;

                case MACHINE_ID_SPECTRUM_128_SPA:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		peek_byte_no_time=peek_byte_no_time_spectrum_128k;
		poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;




                case MACHINE_ID_SPECTRUM_P2:
                case MACHINE_ID_SPECTRUM_P2_FRE:
                case MACHINE_ID_SPECTRUM_P2_SPA:
                poke_byte=poke_byte_spectrum_128k;
                peek_byte=peek_byte_spectrum_128k;
		        peek_byte_no_time=peek_byte_no_time_spectrum_128k;
		        poke_byte_no_time=poke_byte_no_time_spectrum_128k;
                lee_puerto=lee_puerto_spectrum;
                ay_chip_present.v=1;
                break;



                //11=Amstrad +2A (ROM v4.0
                case MACHINE_ID_SPECTRUM_P2A_40:
                case MACHINE_ID_SPECTRUM_P2A_41:
                case MACHINE_ID_SPECTRUM_P2A_SPA:
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




                case MACHINE_ID_ZXUNO:
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


		case MACHINE_ID_ZX80:
        case MACHINE_ID_MICRODIGITAL_TK80:
        case MACHINE_ID_MICRODIGITAL_TK82:
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

		case MACHINE_ID_ZX81:
        case MACHINE_ID_TIMEX_TS1000:
        case MACHINE_ID_TIMEX_TS1500:
        case MACHINE_ID_MICRODIGITAL_TK82C:
        case MACHINE_ID_MICRODIGITAL_TK83:
        case MACHINE_ID_MICRODIGITAL_TK85:
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

		//CPC664
        case MACHINE_ID_CPC_664:
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


        break;

		//CPC6128
        case MACHINE_ID_CPC_6128:
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


        break;


        case MACHINE_ID_PCW_8256:
		poke_byte=poke_byte_pcw;
		peek_byte=peek_byte_pcw;
		peek_byte_no_time=peek_byte_no_time_pcw;
		poke_byte_no_time=poke_byte_no_time_pcw;
		lee_puerto=lee_puerto_pcw;
        out_port=out_port_pcw;
        ay_chip_present.v=1;
        fetch_opcode=fetch_opcode_pcw;

		break;

        case MACHINE_ID_PCW_8512:
		poke_byte=poke_byte_pcw;
		peek_byte=peek_byte_pcw;
		peek_byte_no_time=peek_byte_no_time_pcw;
		poke_byte_no_time=poke_byte_no_time_pcw;
		lee_puerto=lee_puerto_pcw;
        out_port=out_port_pcw;
        ay_chip_present.v=1;
        fetch_opcode=fetch_opcode_pcw;

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



void post_set_mach_reopen_screen(void)
{
				debug_printf(VERBOSE_INFO,"End Screen");
			scr_end_pantalla();
			debug_printf(VERBOSE_INFO,"Creating Screen");
			screen_init_pantalla_and_others_and_realjoystick();

			//scr_init_pantalla();
}


//Reabrir ventana en caso de que maquina seleccionada sea diferente a la anterior
void post_set_machine_no_rom_load_reopen_window(void)
{

    int antes_menu_gui_zoom=menu_gui_zoom;

	set_menu_gui_zoom();

	//printf ("last: %d current: %d\n",last_machine_type,current_machine_type);

	if (last_machine_type!=255 && last_machine_type!=current_machine_type) {
        //Si máquina con gui zoom a 2, cambiar zoom_x y zoom_y a 1, para no exceder tamaños
        if (autochange_zoom_big_display.v) {
            if (antes_menu_gui_zoom !=menu_gui_zoom && menu_gui_zoom==2) {
                if (zoom_x!=1 || zoom_y!=1) {
                    debug_printf (VERBOSE_INFO,"Setting zoom_x and zoom_y to 1 because selected machine has a big display");
                    //printf ("Setting zoom_x and zoom_y to 1\n");
                    zoom_x=zoom_y=1;
                    set_putpixel_zoom();
                }
            }
        }


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
            zxvision_rearrange_background_windows(0,1);
        }

        zxvision_check_all_configurable_icons_positions();

    }

    set_last_dimensiones_ventana();

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

            case MACHINE_ID_MICRODIGITAL_TK90X:
            romfilename="tk90x.rom";
            break;

            case MACHINE_ID_MICRODIGITAL_TK90X_SPA:
            romfilename="tk90xs.rom";
            break;

            case MACHINE_ID_MICRODIGITAL_TK95:
            romfilename="tk95.rom";
            break;

            case MACHINE_ID_MICRODIGITAL_TK95_SPA:
            romfilename="tk95es.rom";
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

            case MACHINE_ID_ZX80:
            case MACHINE_ID_MICRODIGITAL_TK80:
            case MACHINE_ID_MICRODIGITAL_TK82:
            romfilename="zx80.rom";

            break;

            case MACHINE_ID_ZX81:
            case MACHINE_ID_TIMEX_TS1000: //misma rom
            case MACHINE_ID_MICRODIGITAL_TK82C:
            case MACHINE_ID_MICRODIGITAL_TK83:

            romfilename="zx81.rom";

		    break;

            case MACHINE_ID_TIMEX_TS1500:

            romfilename="ts1500.rom";

		    break;

            case MACHINE_ID_MICRODIGITAL_TK85:

            romfilename="tk85.rom";

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

            case MACHINE_ID_CPC_664:
            romfilename="cpc664.rom";
            break;

            case MACHINE_ID_CPC_6128:
            romfilename="cpc6128.rom";
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

            case MACHINE_ID_PCW_8256:
            romfilename="pcw_boot.rom";
            break;

            case MACHINE_ID_PCW_8512:
            romfilename="pcw_boot.rom";
            break;

            default:
            //printf ("ROM for Machine id %d not supported. Exiting\n",machine_type);
            sprintf (mensaje_error,"ROM for Machine id %d not supported. Exiting",current_machine_type);
            cpu_panic(mensaje_error);
            break;



		}
	}

    //Aunque si hay custom rom, sobreescribir esto
    if (setting_set_machine_enable_custom_rom && custom_romfile[0]!=0) {
        debug_printf(VERBOSE_INFO,"Loading custom rom %s",custom_romfile);
        romfilename=custom_romfile;
    }

    open_sharedfile(romfilename,&ptr_romfile);
    if (!ptr_romfile)
    {
        debug_printf(VERBOSE_ERR,"Unable to open rom file %s",romfilename);

        //No hacemos mas un panic por esto. Ayuda a debugar posibles problemas con el path de inicio
        //cpu_panic("Unable to open rom file");

        return;
    }

    int expected_rom_size=get_rom_size(current_machine_type);

		//Caso Inves. ROM esta en el final de la memoria asignada
    if (MACHINE_IS_INVES) {
        //Inves
        leidos=fread(&memoria_spectrum[65536],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }


    else if (MACHINE_IS_SPECTRUM_16_48) {
        //Spectrum 16k rom
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }

    else if (MACHINE_IS_SPECTRUM_128_P2) {
        //Spectrum 32k rom

        leidos=fread(rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }

    }

    else if (MACHINE_IS_SPECTRUM_P2A_P3) {

        //Spectrum 64k rom

        leidos=fread(rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
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

        leidos=fread(chloe_rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
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

        leidos=fread(prism_rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
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
            leidos=fread(tbblue_fpga_rom,1,expected_rom_size,ptr_romfile);
            memcpy(&tbblue_fpga_rom[8192],tbblue_fpga_rom,8192);
            if (leidos!=expected_rom_size) {
                debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
            }
        }

    }

    else if (MACHINE_IS_CHROME) {
        //160 K RAM, 64 K ROM
        leidos=fread(chrome_rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }

    }

    else if (MACHINE_IS_TSCONF) {
        leidos=fread(tsconf_rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }

    }

    else if (MACHINE_IS_BASECONF) {
        leidos=fread(baseconf_rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }

    }


    else if (MACHINE_IS_TIMEX_TS_TC_2068) {

        leidos=fread(timex_rom_mem_table[0],1,16384,ptr_romfile);
        if (leidos!=16384) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: 16384 Loaded: %d",leidos);
        }

        leidos=fread(timex_ex_rom_mem_table[0],1,8192,ptr_romfile);
        if (leidos!=8192) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: 8192 Loaded: %d",leidos);
        }



    }

    else if (MACHINE_IS_COLECO) {
        //coleco 8 kb rom
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }

    else if (MACHINE_IS_SG1000) {
            //no tiene rom. No cargamos nada, aunque mas arriba intenta siempre abrir un archivo de rom,
            //es por eso que es necesario que exista el archivo de rom, aunque no se cargue ni se use para nada

    }

    else if (MACHINE_IS_SMS) {
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }

    else if (MACHINE_IS_MSX1) {
        //msx 32 kb rom
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }


    else if (MACHINE_IS_SVI) {
        //svi 32 kb rom
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }


    else if (MACHINE_IS_ZX80_TYPE) {
        //ZX80
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }


    else if (MACHINE_IS_ZX81_TYPE) {
        //ZX81 y variantes
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }
    }

    else if (MACHINE_IS_ACE) {

        //Jupiter Ace
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
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

        leidos=fread(cpc_rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }

    }

    else if (MACHINE_IS_CPC_6128 || MACHINE_IS_CPC_664) {
        //48k rom

        leidos=fread(cpc_rom_mem_table[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }

    }


    else if (MACHINE_IS_SAM) {
        //32k rom

        leidos=fread(sam_rom_memory[0],1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Expected size: %d Loaded: %d",expected_rom_size,leidos);
        }

    }

    else if (MACHINE_IS_QL) {
        //minimo 16kb,maximo 128k
        leidos=fread(memoria_ql,1,131072,ptr_romfile);

        //Minimo 16kb
        if (leidos<16384) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Minium expected: 16384 Loaded: %d",leidos);
        }

    }


    else if (MACHINE_IS_MK14) {
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Minium expected: 16384 Loaded: %d",leidos);
        }

    }

    //Realmente no es una rom, sino el contenido que carga a la RAM el pcw desde el ¿puerto de impresora?
    else if (MACHINE_IS_PCW_8256 || MACHINE_IS_PCW_8512) {
        leidos=fread(memoria_spectrum,1,expected_rom_size,ptr_romfile);
        if (leidos!=expected_rom_size) {
            debug_printf(VERBOSE_ERR,"Error loading ROM. Minium expected: 16384 Loaded: %d",leidos);
        }

    }


    fclose(ptr_romfile);

}




//char *param_custom_romfile=NULL;
z80_bit opcion_no_welcome_message;




//cuantos botones-joystick a teclas definidas
int joystickkey_definidas=0;




char macos_path_to_executable[PATH_MAX];

//Valor asignado desde BUILDNUMBER
unsigned int buildnumber_int=0;

//Ultima version ejecutada segun la config
unsigned int last_buildnumber_int=0;

z80_bit zesarux_has_been_downgraded={0};

