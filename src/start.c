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


#include "start.h"
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
#include "zeng_online.h"
#include "zeng_online_client.h"


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

z80_bit test_config_and_exit={0};

char *scrfile;
z80_bit quickload_inicial;
char *quickload_nombre;

z80_bit z88_slotcard_inicial;
char *z88_slotcard_inicial_nombre;
int z88_slotcard_inicial_slot;

//Indica si se tiene que probar dispositivos. Solo se hace cuando no se especifica uno concreto por linea de comandos
z80_bit try_fallback_video;
z80_bit try_fallback_audio;

//Los inicializamos a cadena vacia... No poner NULL, dado que hay varios strcmp que se comparan contra esto
char *driver_screen="";
char *driver_audio="";

z80_bit added_some_osd_text_keyboard={0};
char dump_ram_file[PATH_MAX]="";

//el build number segun la config
char last_version_string[255]="";

int argc;
char **argv;
int puntero_parametro;


//Si activado el homenaje para David
z80_bit activated_in_memoriam_david={0};

//Inicio command_line flags
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

z80_bit command_line_ayplayer_start_playlist={0};

z80_bit command_line_start_zeng_online_server={0};

//Fin command_line flags

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



void emulator_main_loop(void)
{
	while (1) {
		if (menu_abierto==1) menu_inicio();

    		//Bucle principal de ejecución de la cpu
    		while (menu_abierto==0 && ending_emulator_flag==0) {
    			cpu_core_loop();
    		}

			//Nos quedamos aqui cerrados cuando se ha salido con ctrl-c, donde este
			//thread aun sigue vivo, pero no hay que andar refrescando ya nada del emulador, para
			//no generar segfault en cuanto se cierre el driver de video
			while (ending_emulator_flag) {
				//printf("Finalizando\n");
				sleep(1);
			}

	}


}

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


void set_last_dimensiones_ventana(void)
{
    last_ancho_ventana=screen_get_total_width_window_plus_zxdesktop();
    last_alto_ventana=screen_get_total_height_window_no_footer_plus_zxdesktop();

    //printf("Setting antes_ancho_total %d antes_alto_total %d\n",last_ancho_ventana,last_alto_ventana);
}

void avisar_opcion_obsoleta(char *texto){
	//Solo avisa si no hay guardado de configuracion
	//Porque si hay guardado, al guardarlo ya lo grabara como toca
	if (save_configuration_file_on_exit.v==0) debug_printf (VERBOSE_ERR,"%s",texto);
}


void print_funny_message(void)
{
	//Mensaje gracioso de arranque que empezó con la ZXSpectr edition (ZEsarUX 4.1)
	//El primero era: Detected SoundBlaster at A220 I5 D1 T2

	//printf ("random: %d\n",randomize_noise[0]);

	//mensajes random de broma
	#define MAX_RANDOM_FUNNY_MESSAGES 32
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
		"Software Failure. Press left mouse button to continue. Guru Meditation #00000004.000AAC0", //12
		"RAMTOP no good",
		"   < Sistema preparado >   ", //14
		"Sorry, a system error ocurred. unimplemented trap",
		"Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(179,2)", //16
		"Invalid MSX-DOS call",
		"B Integer out of range, 0:1", //18
		"Your System ate a SPARC! Gah!",
		"CMOS checksum error - Defaults loaded", //20
        "Proudly Made on Earth",
        "Made From 100% Recycled Pixels", //22
        "You have died of dysentery",
        "Not ready reading drive A. Abort, Retry, Fail?", //24
        "lp0 on fire",
        "Does not compute", //26
        "Shannon and Bill say this can't happen",
        "Z80 panic: shut her down Scotty, she's sucking mud again", //28
        "Not enough memory to display the error m",
        "ERROR 1164 HOW IN THE HELL DID YOU GET HERE", //30
        "Good afternoon, gentelman, I'm a HAL 9000 Computer",
        "Neo-Geo. MAX 330 MEGA - PRO GEAR SPEC"
	};


	int mensaje_gracioso=randomize_noise[0] % MAX_RANDOM_FUNNY_MESSAGES;
	//printf ("indice mensaje gracioso: %d\n",mensaje_gracioso);
	printf ("%s ... Just kidding ;)\n\n",random_funny_messajes[mensaje_gracioso]);
							/*
							printf ("386 Processor or higher detected\n"
											"Using expanded memory (EMS)\n");
							*/
}

char os_release_name[MAX_OS_RELEASE_NAME+1]=""; //Por si acaso definida inicialmente en blanco
void get_os_release(void)
{
    util_get_operating_system_release(os_release_name,MAX_OS_RELEASE_NAME);
    if (os_release_name[0]==0) {
        //En caso de no detectar, le ponemos nombre de OS de compilación
        strcpy(os_release_name,COMPILATION_SYSTEM);
    }
    debug_printf(VERBOSE_INFO,"Running OS Release: %s",os_release_name);
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


void zesarux_cmdline_help(void)
{
	printf ("Usage:\n"
		"[--tape] file           Insert input standard tape file. Supported formats: Spectrum: .PZX, .TAP, .TZX -- ZX80: .O, .80, .Z81 -- ZX81: .P, .81, .Z81 -- Jupiter Ace: .TAP -- All machines: .RWA, .SMP, .WAV\n"
		"[--realtape] file       Insert input real tape file. Supported formats: Spectrum: .PZX, .TAP, .TZX -- ZX80: .O, .80, .Z81 -- ZX81: .P, .81, .Z81 -- CPC: .CDT -- All machines: .RWA, .SMP, .WAV\n"
		"[--snap] file           Load snapshot file. Supported formats: Spectrum: .ZSF, .ZX, .NEX, .SNA, .SNX, .SP, .SPG, .RZX, .Z80 -- ZX80: .ZSF, .ZX, .O, .80 -- ZX81: .ZSF, .ZX, .P, .81, .Z81 -- Jupiter Ace: .ACE\n"
		"[--slotcard] file       Insert Z88 EPROM/Flash file in the first slot. Supported formats: .EPR, .63, .EPROM, .FLASH\n"
		"Note: if you write a tape/snapshot/card file name without --tape, --realtape, --snap or --slotcard parameters, ZEsarUX will try to guess file type (it's the same as SmartLoad on the menu)\n"
		"\n"
        "--slotcard-num file n   Same as --slotcard but insert card on the slot number n (1,2 or 3)\n"

		"--outtape file          Insert output standard tape file. Supported formats: Spectrum: .TAP, .TZX, .PZX -- ZX80: .O -- ZX81: .P\n"

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

	printf ("--machine id            Machine type: \n\n"
            " ID       Description\n"
            " -------- -----------\n"
            "\n");

	printf ("%s",string_machines_list_description);



		printf ("\n"
		"--noconfigfile          Do not load configuration file. This parameter must be the first and it's ignored if written on config file\n"
		"--configfile f          Use the specified config file. This parameter must be the first and it's ignored if written on config file\n"
		"--experthelp            Show expert options\n"
        "--helpcustomconfig      Show help for autoconfig files\n"
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

void zesarux_cmdline_help_expert(void)
{

	printf ("Expert options: \n"




		"\n"
		"\n"
		"Accessibility - Print char traps\n"
		"--------------------------------\n"
		"\n"

		"--enableprintchartrap      Enable traps for standard ROM print char calls and non standard second & third traps. On Spectrum, ZX80, ZX81 machines, standard ROM calls are those using RST10H. On Z88, are those using OS_OUT and some other functions. Note: it is enabled by default on stdout & simpletext drivers\n"
		"--disableprintchartrap     Disable traps for ROM print char calls and second & third traps.\n"
        "--chardetectcompatnum      Enable ROM trap compatibility for printing numbers (but not good for printing from games, like PAWS)\n"
		"--automaticdetectchar      Enable automatic detection & try all method to find print character routines, for games not using RST 10H\n"
		"--secondtrapchar n         Print Char second trap address\n"
		"--secondtrapsum32          Print Char second trap sum 32 to character\n"
		"--thirdtrapchar n          Print Char third trap address\n"
        "--chartrapfilter s         Set Print Char trap filter, to a one of: ");

        charfilter_print_list();

    printf(
        "\n"
        "--chardetectignorenl       Ignore new line characters (13, 10) on char detection\n"
        "--linewidth n              Print char line width\n"

        //estos dos settings tienen tambien opcion para desactivar el setting por cambios en valor por defecto a partir ZEsarUX 9.3
		"--linewidthwaitspace       Text will be sent to speech when line is larger than line width and a space, comma or semicolon is detected\n"
        "--linewidthnowaitspace     Just disable previous setting\n"
        "--linewidthwaitdot         Text will be sent to speech when line is larger than line width and dot is detected\n"
        "--linewidthnowaitdot       Just disable previous setting\n"

		"\n"
		"\n"
		"Accessibility - Text to Speech\n"
		"------------------------------\n"
		"\n"


		"--textspeechprogram p      Specify a path to a program or script to be sent the emulator text shown. For example, for text to speech: speech_filters/festival_filter.sh or speech_filters/macos_say_filter.sh\n"
		"--textspeechstopprogram p  Specify a path to a program or script in charge of stopping the running speech program. For example, speech_filters/stop_festival_filter.sh\n"
        "--textspeechgetstdout      Send stdout from script to debug console window\n"
		"--textspeechwait           Wait and pause the emulator until the Speech program returns\n"
		"--textspeechmenu           Also send text menu entries to Speech program\n"
		"--textspeechtimeout n      After some seconds the text will be sent to the Speech program when no new line is sent. Between 0 and 99. 0 means never\n"

		"\n"
		"\n"
		"Accessibility - Others\n"
		"------------------------------\n"
		"\n"

        "--accessibility-gui-sounds  Enable sounds for GUI events\n"


		"\n"
		"\n"
		"Audio Features\n"
		"--------------\n"
		"\n"

		"--disableayspeech                Disable AY Speech sounds\n"
		"--disableenvelopes               Disable AY Envelopes\n"
		"--disablebeeper                  Disable Beeper\n"
        "--disablerealbeeper              Disable real Beeper sound\n"
		"--totalaychips  n                Number of ay chips. Default 1\n"
		"--ay-stereo-mode n               Mode of AY stereo emulated: 0=Mono, 1=ACB, 2=ABC, 3=BAC, 4=Custom. Default Mono\n"
		"--ay-stereo-channel X n          Position of AY channel X (A, B or C) in case of Custom Stereo Mode. 0=Left, 1=Center, 2=Right\n"
		"--enableaudiodac                 Enable DAC emulation. By default Specdrum\n"
		"--audiodactype type              Select one of audiodac types: "
);
		audiodac_print_types();

printf (
		"\n"
		"--audiovolume n                  Sets the audio output volume to percentage n\n"
		"--zx8081vsyncsound               Enable vsync/tape sound on ZX80/81\n"
        "--ayplayer-add-dir d             Add directory containing AY files to playlist\n"
        "--ayplayer-start-playlist        Start playing playlist when start ZEsarUX\n"
        "--ayplayer-shuffle               Random playback\n"
        "--ayplayer-no-silence-detection  Do not jump to next track if silence detected during 10 seconds\n"
		"--ayplayer-end-exit              Exit emulator when end playing all ay files in playlist\n"
		"--ayplayer-end-repeat            Repeat playing from the beginning when end playing current ay file\n"
		"--ayplayer-inf-length n          Limit to n seconds to ay tracks with infinite length\n"
		"--ayplayer-any-length n          Limit to n seconds to all ay tracks\n"
		"--ayplayer-cpc                   Set AY Player to CPC mode (default: Spectrum)\n"
        "--ayplayer-show-info-console     Show AY Player information about current file and song on console\n"
        "--audiopiano-zoom n              Set zoom for Audio Chip Piano and Wave Piano (1-3)\n"
		"--enable-midi                    Enable midi output\n"
		"--midi-client n                  Set midi client value to n. Needed only on Linux with Alsa audio driver\n"
		"--midi-port n                    Set midi port value to n. Needed on Windows and Linux with Alsa audio driver\n"
		"--midi-raw-device s              Set midi raw device to s. Needed on Linux with Alsa audio driver\n"
		"--midi-allow-tone-noise          Allow tone+noise channels on midi\n"
		"--midi-no-raw-mode               Do not use midi in raw mode. Raw mode is required on Linux to emulate AY midi registers\n"


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
        "--sdl-use-callback-new      SDL audio use new callback (usually better results on Windows)\n"
        "--sdl-use-callback-old      SDL audio use old callback\n"
		"--sdlrawkeyboard            SDL read keyboard in raw mode, needed for ZX Recreated to work well\n");


		printf (
#endif



		"\n"
		"\n"
		"CPU Settings\n"
		"------------\n"
		"\n"

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
		"Debugging\n"
		"---------\n"
		"\n"
		"--verbose n                         Verbose level n (0=only errors, 1=warning and errors, 2=info, warning and errors, 3=debug, 4=lots of messages)\n"
        "--debug-filter s                    Filter type for debug messages, can be exclude or include\n"
        "--debug-filter-exclude-mask n       Filter mask for excluding debug messages\n"
        "--debug-filter-include-mask n       Filter mask for including debug messages\n"
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

        "--zeng-online-nickname s                        Sets nickname for ZENG Online\n"
        "--zeng-online-no-zip-snapshots                  Do not compress snapshots with zip format\n"
        "--zeng-online-no-footer-lag-indicator           Show lag indicator on footer when snapshots are coming late\n"
        "--enable-zeng-online-server                     Enable ZENG Online server. Requires ZRCP\n"
        "--zeng-online-server-allow-create               Allows this ZENG Online server to allow create rooms from any ip address. By default, only creation from localhost is allowed\n"
        "--zeng-online-server-max-rooms n                Set maximum rooms for this ZENG Online server\n"
        "--zeng-online-server-max-players-room n         Set maximum players per room for this ZENG Online server\n"
        "--zeng-online-server-destroy-rooms-no-players   This ZENG Online server destroys rooms without players\n"
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
        "--textadvmap-zoom n                 Text adventure map: zoom level\n"
        "--textadvmap-follow                 Text adventure map: follow the current position on the map\n"
        "--textadvmap-show-unconnected       Text adventure map: show unconnected rooms\n"
        "--textadvmap-no-show-unvisited      Text adventure map: no not show unvisited rooms\n"
        "--textadvmap-no-show-objects        Text adventure map: no not show objects\n"
        "--textadvmap-no-show-pictures       Text adventure map: no not show pictures\n"



		"\n"
		"\n"
		"Display Settings\n"
		"----------------\n"
		"\n"

		"--realvideo                            Enable real video display - for Spectrum (rainbow and other advanced effects) and ZX80/81 (non standard & hi-res modes)\n"
		"--no-detect-realvideo                  Disable real video autodetection\n"
		"--tbblue-legacy-hicolor                Allow legacy hi-color effects on pixel/attribute display zone\n"
		"--tbblue-legacy-border                 Allow legacy border effects on tbblue machine\n"
        "--tbblue-no-sprite-optimization        Disable tbblue sprite render optimization\n"

		//"--tsconf-fast-render       Enables fast render of Tiles and Sprites for TSConf. Uses less host cpu but it's less realistic: doesn't do scanline render but full frame render\n"

		"--snoweffect                           Enable snow effect support for Spectrum\n"
		"--enablegigascreen                     Enable Gigascreen video support\n"
		"--enableinterlaced                     Enable Interlaced video support\n"
		"--enableulaplus                        Enable ULAplus video modes\n"
		"--enablespectra                        Enable Spectra video modes\n"
		"--enabletimexvideo                     Enable Timex video modes\n"
		"--disablerealtimex512                  Disable real Timex mode 512x192. In this case, it's scalled to 256x192 but allows scanline effects\n"
		"--enable16c                            Enable 16C video mode support\n"
		"--enablezgx                            Enable ZGX Sprite chip\n"
		"--autodetectwrx                        Enable WRX autodetect setting on ZX80/ZX81\n"
		"--wrx                                  Enable WRX mode on ZX80/ZX81\n"
		"--vsync-minimum-length n               Set ZX80/81 Vsync minimum length in t-states (minimum 100, maximum 999)\n"
		"--chroma81                             Enable Chroma81 support on ZX80/ZX81\n"
		"--videozx8081 n                        Emulate ZX80/81 Display on Spectrum. n=pixel threshold (1..16. 4=normal)\n"
		"--videofastblack                       Emulate black screen on fast mode on ZX80/ZX81\n"
		"--no-ocr-alternatechars                Disable looking for an alternate character set other than the ROM default on OCR functions\n"
        "--z88-hide-shortcuts                   Hide Z88 shortcuts from the display\n"
		"--scr file                             Load Screen File at startup\n"
	    "--arttextthresold n                    Pixel threshold for artistic emulation for curses & stdout & simpletext (1..16. 4=normal)\n"
	    "--disablearttext                       Disable artistic emulation for curses & stdout & simpletext\n"
		"--allpixeltotext                       Enable all pixel to text mode\n"
		"--allpixeltotext-scale n               All pixel to text mode scale\n"
		"--allpixeltotext-invert                All pixel to text mode invert mode\n"
        "--allpixeltotext-width n               All pixel to text max width (in chars) (minimum 1, maximum 9999)\n"
        "--allpixeltotext-x-offset n            All pixel to text X-Offset (in chars) (minimum 0, maximum 9999)\n"
        "--allpixeltotext-height n              All pixel to text max height (in chars) (minimum 1, maximum 9999)\n"
        "--allpixeltotext-y-offset n            All pixel to text Y-Offset (in chars) (minimum 0, maximum 9999)\n"
		"--text-keyboard-add text               Add a string to the Adventure Text OSD Keyboard. The first addition erases the default text keyboard.\n"
		" You can use hotkeys by using double character ~~ just before the letter, for example:\n"
		" --text-keyboard-add ~~north   --text-keyboard-add e~~xamine\n");

printf (
		"--text-keyboard-length n               Define the duration for every key press on the Adventure Text OSD Keyboard, in 1/50 seconds (default %d). Minimum 10, maximum 100\n"
		"The half of this value, the key will be pressed, the other half, released. Example: --text-keyboard-length 50 to last 1 second\n",
		DEFAULT_ADV_KEYBOARD_KEY_LENGTH);

printf (
		"--text-keyboard-finalspc               Sends a space after every word on the Adventure Text OSD Keyboard\n"
        "--textimageprogram p                   Specify a path to a program or script to be sent the emulator text shown to generate images\n"
        "--textimage-method-location s          Set the method to detect location text, one of: ");

        textadv_location_print_method_strings();

printf("\n"
        "--textimage-min-time-between-images n  Minimum time (in miliseconds) between every image to avoid too much cost usage by external API\n"
        "--textimage-min-no-char-time n         After that time (in miliseconds) without receiving any character, we can guess it's the end of the location description. Increase it if the descriptions are not full read\n"
        "--textimage-min-after-room-time n      After change room and after that time (in miliseconds), we can guess it's the end of the location description. Increase it if the descriptions are blank or not full read\n"
        "--textimage-total-count n              Define the total executions of the textimageprogram, used only for your own information\n"
		"--red                                  Force display mode with red colour\n"
		"--green                                Force display mode with green colour\n"
		"--blue                                 Force display mode with blue colour\n"
		"  Note: You can combine colours, for example, --red --green for Yellow display, or --red --green --blue for Gray display\n"
		"--inversevideo                         Inverse display colours\n"
		"--realpalette                          Use real Spectrum colour palette according to info by Richard Atkinson\n"

#ifdef COMPILE_AA
        "--aaslow                               Use slow rendering on aalib\n"
#endif



#ifdef COMPILE_CURSESW
		"--curses-ext-utf                       Use extended utf characters to have 64x48 display, only on Spectrum and curses drivers\n"
#endif


		"--autoredrawstdout                     Enable automatic display redraw for stdout & simpletext drivers\n"
		"--sendansi                             Sends ANSI terminal control escape sequences for stdout & simpletext drivers, to use colours and cursor control\n"
		"--textfps n                            Sets FPS for stdout and simpletext text drivers\n"





#ifdef COMPILE_FBDEV
        "--no-use-ttyfbdev                      Do not use a tty on fbdev driver. It disables keyboard\n"
        "--no-use-ttyrawfbdev                   Do not use keyboard on raw mode for fbdev driver\n"
        "--use-all-res-fbdev                    Use all virtual resolution on fbdev driver. Experimental feature\n"
        "--decimal-full-scale-fbdev             Use non integer zoom to fill the display with full screen mode on fbdev driver\n"
        "--fbdev-double-buffer                  Use double buffer to avoid flickering on menu but uses more cpu\n"
#ifdef EMULATE_RASPBERRY
        "--fbdev-no-res-change                  Avoid resolution change on Raspberry Pi full screen mode\n"
        "--fbdev-margin-width n                 Increment fbdev width size on n pixels on Raspberry Pi full screen mode\n"
        "--fbdev-margin-height n                Increment fbdev width height on n pixels on Raspberry Pi full screen mode\n"
#endif

#endif




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
	    "--def-f-function key action                 Define F key to do an action. action can be: ");


        for (i=0;i<MAX_F_FUNCTIONS;i++) {
            printf ("%s ",defined_direct_functions_array[i].texto_funcion);
        }



		printf (
		"\n"

		"--def-f-function-parameters key extra-info  Define extra info associated to an action of a F key"


		"\n"
		"\n"
		"General Settings\n"
		"----------------\n"
		"\n"

		"--zoomx n                                      Horizontal Zoom Factor\n"
		"--zoomy n                                      Vertical Zoom Factor\n"
        "--no-autochange-zoom-big-display               No autochange to zoom 1 when switching to machine with big display (Next, QL, CPC, ...)\n"

		"--reduce-075                                   Reduce display size 4/3 (divide by 4, multiply by 3)\n"
		"--reduce-075-no-antialias                      Disable antialias for reduction, enabled by default\n"
		"--reduce-075-offset-x n                        Destination offset x on reduced display\n"
		"--reduce-075-offset-y n                        Destination offset y on reduced display\n"

		"--frameskip n                                  Set frameskip (0=none, 1=25 FPS, 2=16 FPS, etc)\n"
        "--no-frameskip-zxdesktop-back                  Disable apply frameskip drawing ZX Desktop Background\n"
		"--disable-autoframeskip                        Disable autoframeskip\n"
        "--no-autoframeskip-moving-win                  Disable autoframeskip even when moving windows\n"
        "--disable-flash                                Disable flash\n"
		"--fullscreen                                   Enable full screen\n"
		"--zxdesktop-disable-on-fullscreen              Disable ZX Desktop when going to full screen\n"
        "--zxdesktop-no-restore-win-after-fullscreen    Do not restore windows after disabling full screen, when --zxdesktop-disable-on-fullscreen setting is set\n"
		"--disable-border-on-fullscreen                 Disable Border when going to full screen\n"
        "--disable-footer-on-fullscreen                 Disable Footer when going to full screen\n"
		"--disableborder                                Disable Border\n"
        "--disablefooter                                Disable window footer\n"
        "--ignoremouseclickopenmenu                     Ignore mouse clicking to open menu or ZX Desktop buttons\n"
        "--limitopenmenu                                Limit the action to open menu (F5 by default, joystick button). To open it, you must press the key 3 times in one second\n"
		"--advancedmenus                                Show advanced menu items\n"
        "--language language                            Select alternate language for menu. Available languages: es (Spanish), ca (Catalan). Default language if not set: English\n"
        "--online-download-path p                       Where to download files from the speccy and zx81 online browser. If not set, they are download to a temporary folder\n"

#ifndef MINGW
		"--no-cpu-usage                                 Do not show host CPU usage on footer\n"
#endif

		"--no-cpu-temp                                  Do not show host CPU temperature on footer\n"
		"--no-fps                                       Do not show FPS on footer\n"
        "--nowelcomemessage                             Disable welcome message\n"
        "--enable-xanniversary-logo                     Enable X Anniversary logo (enabled by default only on X version)\n"
        "--disable-xanniversary-logo                    Disable X Anniversary logo. It's the default behaviour, only kept here for backwards compatibility\n"
        "--disablemenufileutils                         Disable File Utilities menu\n"


		"\n"
		"\n"
		"Hardware Settings\n"
		"-----------------\n"
		"\n"

        "--emulatorspeed n           Set Emulator speed in percentage\n"
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
		"\n"
		"Hardware - Joystick\n"
		"-------------------\n"
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
        "Hardware - Memory Settings\n"
        "--------------------------\n"
        "\n"

        "--zx8081mem n       Emulate 1,2,...16 kb of memory on ZX80/ZX81\n"
        "--zx8081ram8K2000   Emulate 8K RAM in 2000H for ZX80/ZX81\n"
        "--zx8081ram16K8000  Emulate 16K RAM in 8000H for ZX80/ZX81\n"
        "--zx8081ram16KC000  Emulate 16K RAM in C000H for ZX80/ZX81\n"
		"--acemem n          Emulate 3, 19, 35 or 51 kb of memory on Jupiter Ace\n"
		"--128kmem n         Set more than 128k RAM for 128k machines. Allowed values: 128, 256, 512\n"


		"\n"
		"\n"
		"Network\n"
		"-------\n"
		"\n"

		"--zeng-remote-hostname s             ZENG last remote hostname\n"
		"--zeng-remote-port n                 ZENG last remote port\n"
		"--zeng-snapshot-interval-frames n    ZENG snapshot interval, on video frames\n"
		"--zeng-iam-master                    Tells this machine is a ZENG master\n"
        "--zeng-not-send-input-events         Do not send ZENG input events (keyboard, joystick) to other hosts\n"




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
		"Snapshot Settings\n"
		"-----------------\n"
		"\n"

		"--smartloadpath path        Select initial smartload path\n"
		"--addlastfile file          Add a file to the last files used\n"
		"--quicksavepath path        Select path for quicksave & continous autosave\n"
		"--autoloadsnap              Load last snapshot on start\n"
		"--autosavesnap              Save snapshot on exit\n"
		"--autosnappath path         Folder to save/load automatic snapshots\n"
		"--tempdir path              Folder to save temporary files. Folder must exist and have read and write permissions\n"
		"--snap-no-change-machine    Do not change machine when loading sna or z80 snapshots. Just load it on memory\n"
        "--zsf-save-rom              Include ROM contents in saved ZSF snapshot. Useful when running custom roms. Only available for Spectrum/Clones models 16k/48k/128k/+2/+2A/+3\n"
		"--no-close-after-smartload  Do not close menu after SmartLoad\n"
        "--z88-not-sync-clock-snap   Do not sync PC clock to Z88 clock after loading a snapshot\n"
        "--snapram-interval n        Generate a snapshot in ram every n seconds\n"
        "--snapram-max n             Maximum snapshots to keep in memory\n"
        "--snapram-rewind-timeout n  After this time pressed rewind action, the rewind position is reset to current\n"


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
		"Storage - +3 Disk Settings\n"
		"--------------------------\n"
		"\n"

        "--dsk-file f                             Set 3\" CF2 Floppy DSK image file\n"
        "--enable-dsk                             Enable 3\" CF2 Floppy DSK emulation. Usually requires --dsk-file\n"
        "--dsk-write-protection                   Enable 3\" CF2 Floppy DSK write protection\n"
        "--pd765-silent-write-protection          When write protect is enabled, do not notify the cpu, so behave as it is not write protected (but the data is not written)\n"
        "--dsk-persistent-writes                  Enable 3\" CF2 Floppy DSK persistent writes\n"
		"--dsk-no-persistent-writes               Disable 3\" CF2 Floppy DSK persistent writes\n"

		"--dsk-pcw-no-boot-reinsert-previous-dsk  Do not reinsert previous dsk after booting CP/M\n"
		"--dsk-pcw-no-failback-cpm-when-no-boot   Do not insert CP/M disk if selected disk is not bootable\n"

		"\n"
		"\n"
		"Storage - Betadisk Settings\n"
		"---------------------------\n"
		"\n"

        "--enable-betadisk               Enable Betadisk emulation\n"
        "--trd-file f                    Set trd image file\n"
        "--enable-trd                    Enable TRD emulation. Usually requires --trd-file\n"
        "--trd-write-protection          Enable TRD write protection\n"
		"--trd-no-persistent-writes      Disable TRD persistent writes\n"


		"\n"
		"\n"
		"Storage - Dandanator Settings\n"
		"-----------------------------\n"
		"\n"

        "--dandanator-rom f              Set ZX Dandanator rom file\n"
        "--enable-dandanator             Enable ZX Dandanator emulation. Requires --dandanator-rom\n"
        "--dandanator-press-button       Simulates pressing button on ZX Dandanator. Requires --enable-dandanator\n"


		"\n"
		"\n"
		"Storage - ESXDOS Handler Settings\n"
		"---------------------------------\n"
		"\n"

		"--enable-esxdos-handler         Enable ESXDOS traps handler. Requires divmmc or divide paging emulation\n"
		"--esxdos-root-dir p             Set ESXDOS root directory for traps handler. Uses current directory by default.\n"
        "--esxdos-readonly               Forbid write operations on ESXDOS handler\n"
        "--esxdos-local-dir p            Set ESXDOS local directory for traps handler. This is the relative directory used inside esxdos.\n"


		"\n"
		"\n"
		"Storage - HiLow Settings\n"
		"------------------------\n"
		"\n"

        "--hilow-file f                  Set HiLow Data Drive image file\n"
        "--enable-hilow                  Enable HiLow Data Drive. Usually requires --hilow-file\n"
		"--hilow-write-protection        Enable HiLow Data Drive write protection\n"
		"--hilow-no-persistent-writes    Disable HiLow Data Drive persistent writes\n"


		"\n"
		"\n"
		"Storage - IDE Settings\n"
		"----------------------\n"
		"\n"

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


		"\n"
		"\n"
		"Storage - iFrom Settings\n"
		"------------------------\n"
		"\n"

        "--ifrom-rom f                   Set iFrom rom file\n"
        "--enable-ifrom                  Enable iFrom emulation. Requires --ifrom-rom\n"


		"\n"
		"\n"
		"Storage - Kartusho Settings\n"
		"---------------------------\n"
		"\n"

        "--kartusho-rom f                Set Kartusho rom file\n"
        "--enable-kartusho               Enable Kartusho emulation. Requires --kartusho-rom\n"


		"\n"
		"\n"
		"Storage - MMC Settings\n"
		"----------------------\n"
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


		"\n"
		"\n"
		"Storage - QL MDV & FLP Settings\n"
		"-------------------------------\n"
		"\n"

		"--enable-ql-mdv-flp             Enable QL Microdrive & Floppy emulation\n"
		"--ql-mdv1-root-dir p            Set QL mdv1 root directory\n"
		"--ql-mdv2-root-dir p            Set QL mdv2 root directory\n"
		"--ql-flp1-root-dir p            Set QL flp1 root directory\n"
        "--ql-mdv1-read-only             Mark mdv1 as read only\n"
        "--ql-mdv2-read-only             Mark mdv2 as read only\n"
        "--ql-flp1-read-only             Mark flp1 as read only\n"


		"\n"
		"\n"
		"Storage - Superupgrade Settings\n"
		"-------------------------------\n"
		"\n"

		"--superupgrade-flash f          Set Superupgrade flash file\n"
		"--enable-superupgrade           Enable Superupgrade emulation. Requires --superupgrade-flash\n"


		"\n"
		"\n"
		"Storage - Tape Settings\n"
		"-----------------------\n"
		"\n"
		"--noautoload                No autoload tape file on Spectrum, ZX80 or ZX81\n"
		"--fastautoload              Do the autoload process at top speed\n"
		"--noautoselectfileopt       Do not autoselect emulation options for known snap and tape files\n"
        "--no-fallbacktorealtape     Disable fallback to real tape setting\n"
        "--anyflagloading            Enables tape load routine to load without knowing block flag\n"
        "--autorewind                Autorewind tape when reaching end of tape\n"
		"--simulaterealload          Simulate real tape loading\n"
		"--simulaterealloadfast      Enable fast simulate real tape loading\n"
        "--deletetzxpauses           Do not follow pauses on TZX tapes\n"
		"--realloadfast              Fast loading of real tape\n"


		"\n"
		"\n"
		"Storage - ZXPand Settings\n"
		"-------------------------\n"
		"\n"

		"--enable-zxpand                 Enable ZXpand emulation\n"
		"--zxpand-root-dir p             Set ZXpand root directory for sd/mmc filesystem. Uses current directory by default.\n"
		"                                Note: ZXpand does not use --mmc-file setting\n"


		"\n"
		"\n"
		"Storage - ZX-Uno Flash Settings\n"
		"-------------------------------\n"
		"\n"

		"--zxunospifile path             File to use on ZX-Uno as SPI Flash. Default: zxuno.flash\n"
		"--zxunospi-write-protection     Enable ZX-Uno SPI Flash write protection\n"
		"--zxunospi-persistent-writes    Enable ZX-Uno SPI Flash persistent writes\n"
        "--zxuno-initial-64k f           Load a 64kb file that will be written on the initial 64kb space\n"


		"\n"
		"\n"
		"ULA Settings\n"
		"------------\n"
		"\n"

		"--ula-data-bus n            Sets the ula data bus value\n"



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

		"--changeslowparameters      Change some performance parameters (frameskip, realvideo, etc) "
		"on slow machines like raspberry, etc\n"



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
        "--zxdesktop-disable-frame-emulated-display     Disable showing a frame around the emulated machine display\n"
        "--zxdesktop-scr-file f                         Set ZX Desktop SCR background file\n"
        "--zxdesktop-scr-enable                         Enable ZX Desktop SCR background file\n"
        "--zxdesktop-scr-centered                       Center ZX Desktop SCR background\n"
        "--zxdesktop-scr-fillscale                      Scale automatic for ZX Desktop SCR background\n"
        "--zxdesktop-scr-mixbackground                  Mix SCR image with background\n"
        "--zxdesktop-scr-scalefactor n                  Scale manually for ZX Desktop SCR background\n"
        "--zxdesktop-scr-disable-flash                  Disable flash for ZX Desktop SCR background\n"
        "--zxdesktop-disable-configurable-icons         Disable configurable icons on ZX Desktop\n"
        "--zxdesktop-empty-trash-on-exit                Empty Trash on exit ZEsarUX\n"
        "--zxdesktop-no-show-indicators-open-apps       Show icon indicators for open apps\n"
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

        "--def-button-function-parameters button extra-info  Define extra info associated to an action of a button\n"


		"\n"
		"\n"
		"ZX Vision Settings\n"
		"-------------------\n"
		"\n"


		//"--invespokerom n           Inves rom poke value\n"

		"--menucharwidth n                        Character size width for menus valid values: 8,7,6 or 5\n"
        "--menucharheight n                       Character size height for menus valid values: 8,7 or 6\n"
		"--hidemousepointer                       Hide Mouse Pointer. Not all video drivers support this\n"
		"--disablemenumouse                       Disable mouse on emulator menu\n"

		//"--overlayinfo              Overlay on screen some machine info, like when loading tape\n"

		"--disablemultitaskmenu                   When multitask is disabled, both emulation, background windows and other menu features are stopped when opening the menu\n"
		//"--disablebw-no-multitask   Disable changing to black & white colours on the emulator machine when menu open and multitask is off\n"
        "--stopemulationmenu                      When multitask is enabled, you can disable emulation when opening the menu\n"
        "--old-behaviour-menu-esc-etc             Old menu behaviour: ESC go back, Apps go back to previous menu\n"
		"--hide-menu-percentage-bar               Hides vertical percentaje bar on the right of text windows and file selector\n"
        "--hide-menu-submenu-indicator            Hides submenu indicator character (>) on menu items with submenus\n"
		"--hide-menu-minimize-button              Hides minimize button on the title window\n"
        "--hide-menu-maximize-button              Hides maximize button on the title window\n"
		"--hide-menu-close-button                 Hides close button on the title window\n"
        "--show-menu-background-button            Shows background button on inactive windows\n"
        "--no-change-frame-resize-zone            Do not change frame window when mouse if over resize zone\n"
		"--invert-menu-mouse-scroll               Inverts mouse scroll movement\n"
        "--right-mouse-esc                        Right button mouse simulates ESC key and not secondary actions\n"
        "--process-switcher-immutable             Massive actions on menu Windows, like minimize all, cascade, etc, don't affect the Process switcher window\n"
        "--process-switcher-always-visible        Process switcher is always visible (on top of all windows)\n"
		"--allow-background-windows               Allow putting windows in background\n"
        "--allow-background-windows-closed-menu   Allow these background windows even when menu closed\n"
		);

	printf (
		"--menu-mix-method s                      How to mix menu and the layer below. s should be one of: ");


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
        "--charset-customfile f                   Set file name for customfile charset\n"


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
		"Miscellaneous\n"
		"-------------\n"
		"\n"

		"--saveconf-on-exit                       Always save configuration when exiting emulator\n"
		"--quickexit                              Exit emulator quickly: no yes/no confirmation and no fadeout\n"
		"--exit-after n                           Exit emulator after n seconds\n"
		"--last-version s                         String which identifies last build version run. Usually doesnt need to change it, used to show the start popup of the new version changes\n"
        "--last-version-text s                    String which identifies last version run. Usually doesnt need to change it, used to show the start popup of the new version changes\n"
		"--no-show-changelog                      Do not show changelog when updating version\n"
        "--no-show-david-in-memoriam              Do not show David in memoriam message\n"
		"--machinelist                            Get machines list names whitespace separated, and exit\n"
		"--disablebetawarning text                Do not pause beta warning message on boot for version named as that parameter text\n"
		"--tbblue-autoconfigure-sd-already-asked  Do not ask to autoconfigure tbblue initial SD image\n"

		//Esto no hace falta que lo vea un usuario, solo lo uso yo para probar partes del emulador
		//"--codetests                Run develoment code tests\n"
		"--tonegenerator n                        Enable tone generator. Possible values: 1: generate max, 2: generate min, 3: generate min/max at 50 Hz\n");

        printf(
        "--sensor-set position type               Set sensor for menu View sensors. Position must be 0 to %d. Type must be one of:\n",MENU_VIEW_SENSORS_TOTAL_ELEMENTS-1);

        sensor_list_print();

        printf("\n"
        "--sensor-set-widget position type        Set widget type sensor for menu View sensors. Position must be 0 to %d. Type must be one of:\n",MENU_VIEW_SENSORS_TOTAL_ELEMENTS-1);

        widget_list_print();

        printf("\n"
        "--sensor-set-abs position                Set widget type absolute instead of percentaje for menu View sensors. Position must be 0 to %d\n",MENU_VIEW_SENSORS_TOTAL_ELEMENTS-1);


        printf(
        "--history-item-add-debugcpu-ptr s        Add string as history for debug cpu change pointer\n"
        "--history-item-add-hexeditor-ptr s       Add string as history for hexeditor change pointer\n"
        "--history-item-add-sprites-ptr s         Add string as history for sprites change pointer\n"
		"--history-item-add-poke-ptr s            Add string as history for poke pointer\n"
		"--history-item-add-poke-value s          Add string as history for poke value\n"


		"\n\n"

		"One-time actions\n"
		"----------------\n"
		"\n"
        "The following are actions that are executed from the console and don't start ZEsarUX:"
        "\n\n"
        "--convert-tap-tzx source destination           Convert tap source file to destination tzx\n"
        "--convert-tap-tzx-turbo-rg source destination  Convert tap source file to destination tzx turbo (4000 bauds) for use with Rodolfo Guerra ROMS\n"
        "--convert-tap-pzx source destination           Convert tap source file to destination pzx\n"
        "--convert-tap-scr source destination           Convert tap source file to destination scr\n"
        "--convert-tzx-tap source destination           Convert tzx source file to destination tap\n"
        "--convert-pzx-tap source destination           Convert pzx source file to destination tap\n"


        "\n\n"

	);

}



void segint_signal_handler(int sig)
{

	debug_printf (VERBOSE_INFO,"Sigint (CTRL+C) received");

        //para evitar warnings al compilar
        sig++;

    //No detener threads, porque si no, el end_emulator puede fallar por miles de razones...

//Primero de todo detener el pthread del emulador, que no queremos que siga activo el emulador con el pthread de fondo mientras
//se ejecuta el end_emulator
/*
#ifdef USE_PTHREADS
        if (si_thread_main_loop) {
        	debug_printf (VERBOSE_INFO,"Ending main loop thread");
		pthread_cancel(thread_main_loop);
	}
#endif
*/
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

        //No detener threads, porque si no, el end_emulator puede fallar por miles de razones...

//Primero de todo detener el pthread del emulador, que no queremos que siga activo el emulador con el pthread de fondo mientras
//se ejecuta el end_emulator
/*
#ifdef USE_PTHREADS
        if (si_thread_main_loop) {
        	debug_printf (VERBOSE_INFO,"Ending main loop thread");
		pthread_cancel(thread_main_loop);
	}
#endif
*/

    //salir sin fadeout, no queremos hacerlo en caso de salir desde aqui para que sea mas rapido
    //nota: dado que estamos saliendo sin guardar config, podemos alterar parametros de configuracion
    //desde aqui sin que se graben en el archivo de configuración
    quickexit.v=1;

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

        screen_este_driver_permite_ext_desktop=0;

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

#ifdef USE_COCOA
int set_scrdriver_cocoa(void)
{
    scr_refresca_pantalla=scrcocoa_refresca_pantalla;
    scr_refresca_pantalla_solo_driver=scrcocoa_refresca_pantalla_solo_driver;
    scr_init_pantalla=scrcocoa_init;
    scr_end_pantalla=scrcocoa_end;
    scr_lee_puerto=scrcocoa_lee_puerto;
    scr_actualiza_tablas_teclado=scrcocoa_actualiza_tablas_teclado;

    //Esto se tiene que cambiar antes incluso de inicializar el driver video, para evitar leer mal el tamaño total ventana
    //screen_este_driver_permite_ext_desktop=1;
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

    //Esto se tiene que cambiar antes incluso de inicializar el driver video, para evitar leer mal el tamaño total ventana
    //screen_este_driver_permite_ext_desktop=1;
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

    //Esto se tiene que cambiar antes incluso de inicializar el driver video, para evitar leer mal el tamaño total ventana
    //screen_este_driver_permite_ext_desktop=1;
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




int set_scrdriver_null(void)
{
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

int set_scrdriver_stdout(void)
{
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

int set_scrdriver_simpletext(void)
{
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
int set_audiodriver_dsp(void)
{
    audio_init=audiodsp_init;
    audio_send_frame=audiodsp_send_frame;
    audio_thread_finish=audiodsp_thread_finish;
    audio_end=audiodsp_end;
    audio_get_buffer_info=audiodsp_get_buffer_info;
    return 0;

}
#endif


#ifdef COMPILE_ONEBITSPEAKER
int set_audiodriver_onebitspeaker(void)
{
    audio_init=audioonebitspeaker_init;
    audio_send_frame=audioonebitspeaker_send_frame;
    audio_thread_finish=audioonebitspeaker_thread_finish;
    audio_end=audioonebitspeaker_end;
    audio_get_buffer_info=audioonebitspeaker_get_buffer_info;
    return 0;

}
#endif



#ifdef COMPILE_SDL
int set_audiodriver_sdl(void)
{
    audio_init=audiosdl_init;
    audio_send_frame=audiosdl_send_frame;
    audio_thread_finish=audiosdl_thread_finish;
    audio_end=audiosdl_end;
    audio_get_buffer_info=audiosdl_get_buffer_info;
    return 0;

}
#endif


#ifdef COMPILE_ALSA
int set_audiodriver_alsa(void)
{
    audio_init=audioalsa_init;
    audio_send_frame=audioalsa_send_frame;
    audio_thread_finish=audioalsa_thread_finish;
    audio_end=audioalsa_end;
    audio_get_buffer_info=audioalsa_get_buffer_info;
    return 0;

}
#endif

#ifdef COMPILE_PULSE
int set_audiodriver_pulse(void)
{
    audio_init=audiopulse_init;
    audio_send_frame=audiopulse_send_frame;
    audio_thread_finish=audiopulse_thread_finish;
    audio_end=audiopulse_end;
    audio_get_buffer_info=audiopulse_get_buffer_info;
    return 0;

}
#endif


#ifdef COMPILE_COREAUDIO
int set_audiodriver_coreaudio(void)
{
    audio_init=audiocoreaudio_init;
    audio_send_frame=audiocoreaudio_send_frame;
    audio_thread_finish=audiocoreaudio_thread_finish;
    audio_end=audiocoreaudio_end;
    audio_get_buffer_info=audiocoreaudio_get_buffer_info;
    return 0;

}
#endif


void main_init_video(void)
{
    //Video init
    debug_printf (VERBOSE_INFO,"Initializing Video Driver");

    //Asumimos video driver no soporta ext desktop
    screen_este_driver_permite_ext_desktop=0;

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


//desde_commandline: si parsea desde commandline (1) o desde archivo de config (0)
int parse_cmdline_options(int desde_commandline) {

		while (!siguiente_parametro()) {
			if (!strcmp(argv[puntero_parametro],"--help")) {
				zesarux_cmdline_help();
				exit(1);
			}

                        if (!strcmp(argv[puntero_parametro],"--experthelp")) {
                                zesarux_cmdline_help_expert();
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

                int valor=atoi(argv[puntero_parametro]);

				if (valor<1 || valor>9) {
					printf ("Invalid value for zoom\n");
					exit(1);
				}

                zoom_x=valor;

			}

			else if (!strcmp(argv[puntero_parametro],"--zoomy")) {
				siguiente_parametro_argumento();

                int valor=atoi(argv[puntero_parametro]);

				if (valor<1 || valor>9) {
					printf ("Invalid value for zoom\n");
					exit(1);
				}

                zoom_y=valor;
			}

            else if (!strcmp(argv[puntero_parametro],"--no-autochange-zoom-big-display")) {
                autochange_zoom_big_display.v=0;
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
				zxdesktop_width=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-height")) {
				siguiente_parametro_argumento();
				int valor=parse_string_to_number(argv[puntero_parametro]);

				if (valor<0 || valor>9999) {
					printf ("Invalid value for ZX Desktop height\n");
					exit(1);
				}
				zxdesktop_height=valor;
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

			else if (!strcmp(argv[puntero_parametro],"--zxdesktop-no-restore-win-after-fullscreen")) {
				zxdesktop_restore_windows_after_full_screen=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--disable-border-on-fullscreen")) {
				disable_border_on_full_screen=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--disable-footer-on-fullscreen")) {
				disable_footer_on_full_screen=1;
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

            else if (!strcmp(argv[puntero_parametro],"--zxdesktop-empty-trash-on-exit")) {
                zxdesktop_empty_trash_on_exit.v=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--zxdesktop-no-show-indicators-open-apps")) {
                zxdesktop_icon_show_app_open.v=0;
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

			else if (!strcmp(argv[puntero_parametro],"--menucharheight")) {
				siguiente_parametro_argumento();
				int valor=atoi(argv[puntero_parametro]);
				if (valor!=6 && valor!=7 && valor!=8) {
					printf ("Invalid value for character height\n");
					exit(1);
				}
				menu_char_height=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--zoom")) {
				siguiente_parametro_argumento();
                int valor=atoi(argv[puntero_parametro]);

				if (valor<1 || valor>9) {
					printf ("Invalid value for zoom\n");
					exit(1);
				}

				zoom_y=zoom_x=valor;
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

            //Mantenida por compatibilidad hacia atras. El valor por defecto es 1
            else if (!strcmp(argv[puntero_parametro],"--autoframeskip-moving-win")) {
                auto_frameskip_even_when_movin_windows.v=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--no-autoframeskip-moving-win")) {
                auto_frameskip_even_when_movin_windows.v=0;
            }


            else if (!strcmp(argv[puntero_parametro],"--no-frameskip-zxdesktop-back")) {
                frameskip_draw_zxdesktop_background.v=0;
            }

            else if (!strcmp(argv[puntero_parametro],"--disable-flash")) {
                disable_change_flash.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--testconfig")) {
				test_config_and_exit.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--nochangeslowparameters")) {
				cambio_parametros_maquinas_lentas.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--changeslowparameters")) {
				cambio_parametros_maquinas_lentas.v=1;
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

            else if (!strcmp(argv[puntero_parametro],"--debug-filter")) {
                siguiente_parametro();
                if (!strcasecmp(argv[puntero_parametro],"exclude")) {
                    debug_mascara_modo_exclude_include=VERBOSE_MASK_CLASS_TYPE_EXCLUDE;
                }

                else if (!strcasecmp(argv[puntero_parametro],"include")) {
                    debug_mascara_modo_exclude_include=VERBOSE_MASK_CLASS_TYPE_INCLUDE;
                }

                else {
                    printf("Invalid debug filter type\n");
                    exit(1);
                }
            }



            else if (!strcmp(argv[puntero_parametro],"--debug-filter-exclude-mask")) {
                siguiente_parametro();

                int valor=parse_string_to_number(argv[puntero_parametro]);
                debug_mascara_clase_exclude=valor;
            }

            else if (!strcmp(argv[puntero_parametro],"--debug-filter-include-mask")) {
                siguiente_parametro();

                int valor=parse_string_to_number(argv[puntero_parametro]);
                debug_mascara_clase_include=valor;
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


                        //Mantenida por compatibilidad hacia atras (ZEsarUX versiones anterior a 11)
                        else if (!strcmp(argv[puntero_parametro],"--cpuspeed")) {
                                siguiente_parametro_argumento();
                                porcentaje_velocidad_emulador=atoi(argv[puntero_parametro]);
                                if (porcentaje_velocidad_emulador<1 || porcentaje_velocidad_emulador>9999) {
                                        printf ("Invalid CPU percentage\n");
                                        exit(1);
                                }
                        }


            else if (!strcmp(argv[puntero_parametro],"--emulatorspeed")) {
                    siguiente_parametro_argumento();
                    porcentaje_velocidad_emulador=atoi(argv[puntero_parametro]);
                    if (porcentaje_velocidad_emulador<1 || porcentaje_velocidad_emulador>9999) {
                            printf ("Invalid Emulator speed\n");
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

			else if (!strcmp(argv[puntero_parametro],"--advancedmenus")) {
				menu_show_advanced_items.v=1;
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

            else if (!strcmp(argv[puntero_parametro],"--old-behaviour-menu-esc-etc")) {
                menu_old_behaviour_close_menus.v=1;
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

            else if (!strcmp(argv[puntero_parametro],"--z88-hide-shortcuts")) {
                z88_hide_keys_shortcuts.v=1;
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
                                setting_set_machine_enable_custom_rom=1;
                                strcpy(custom_romfile,argv[puntero_parametro]);
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

			else if (!strcmp(argv[puntero_parametro],"--def-f-function-parameters")) {
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

				if (menu_define_key_function_extra_info(valor,argv[puntero_parametro])) {
					printf ("Invalid f-function action extra info: %s\n",argv[puntero_parametro]);
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

			else if (!strcmp(argv[puntero_parametro],"--def-button-function-parameters")) {
				siguiente_parametro_argumento();

				int valor=atoi(argv[puntero_parametro]);

				if (valor<0 || valor>=MAX_USERDEF_BUTTONS) {
					printf ("Invalid button\n");
					exit(1);
				}

				siguiente_parametro_argumento();

                strcpy(defined_buttons_functions_array_parameters[valor],argv[puntero_parametro]);


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

            else if (!strcmp(argv[puntero_parametro],"--zsf-save-rom")) {
                zsf_snap_save_rom.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--no-close-after-smartload")) {
				no_close_menu_after_smartload.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--z88-not-sync-clock-snap")) {
                sync_clock_to_z88.v=0;
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

            else if (!strcmp(argv[puntero_parametro],"--chartrapfilter")) {
                siguiente_parametro_argumento();
                if (charfilter_set(argv[puntero_parametro])) {
                    printf ("Unknown char filter\n");
                    exit(1);
                }
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

            else if (!strcmp(argv[puntero_parametro],"--accessibility-gui-sounds")) {
                accessibility_enable_gui_sounds.v=1;
            }

		else if (!strcmp(argv[puntero_parametro],"--textimageprogram")) {
				siguiente_parametro_argumento();
                strcpy(textimage_filter_program,argv[puntero_parametro]);

				//Si es ruta relativa, poner ruta absoluta
				if (!si_ruta_absoluta(textimage_filter_program)) {
					//printf ("es ruta relativa\n");
					//lo metemos en el buffer de texto del menu usado para esto
					//TODO: quiza hacer esto con convert_relative_to_absolute pero esa funcion es para directorios,
					//no para directorios con archivo, por tanto quiza habria que hacer un paso intermedio separando
					//directorio de archivo
                    char directorio_actual[PATH_MAX];
                    getcwd(directorio_actual,PATH_MAX);

                    char ruta_temp[PATH_MAX];

					sprintf (ruta_temp,"%s/%s",directorio_actual,textimage_filter_program);
                    strcpy(textimage_filter_program,ruta_temp);

					//printf ("ruta final: %s\n",textimage_filter_program);

				}

				//Validar para windows que no haya espacios en la ruta
				if (textimage_filter_program_check_spaces()) {

				    //han habido espacios, y salir del emulador
				    exit(1);
                }

			}



			else if (!strcmp(argv[puntero_parametro],"--textimage-method-location")) {
                siguiente_parametro_argumento();
                if (textadv_location_set_method_by_string(argv[puntero_parametro])) {
                    printf ("Invalid method for detecting location text\n");
					exit(1);
				}
            }



			else if (!strcmp(argv[puntero_parametro],"--textimage-min-time-between-images")) {
                siguiente_parametro_argumento();
				textadv_location_desc_last_image_generated_min=parse_string_to_number(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--textimage-min-no-char-time")) {
                siguiente_parametro_argumento();
				max_textadv_location_desc_no_char_counter=parse_string_to_number(argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--textimage-min-after-room-time")) {
                siguiente_parametro_argumento();
				max_textadv_location_desc_counter=parse_string_to_number(argv[puntero_parametro]);
			}


			else if (!strcmp(argv[puntero_parametro],"--textimage-total-count")) {
                siguiente_parametro_argumento();
				textadv_location_total_conversions=parse_string_to_number(argv[puntero_parametro]);
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

			else if (!strcmp(argv[puntero_parametro],"--pd765-silent-write-protection")) {
				pd765_silent_write_protection.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--dsk-no-persistent-writes")) {
				dskplusthree_persistent_writes.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--dsk-persistent-writes")) {
				dskplusthree_persistent_writes.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--dsk-pcw-no-boot-reinsert-previous-dsk")) {
				pcw_boot_reinsert_previous_dsk.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--dsk-pcw-no-failback-cpm-when-no-boot")) {
				pcw_failback_cpm_when_no_boot.v=0;
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

            else if (!strcmp(argv[puntero_parametro],"--enable-xanniversary-logo")) {
                xanniversary_logo.v=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--disable-xanniversary-logo")) {
                xanniversary_logo.v=0;
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

            else if (!strcmp(argv[puntero_parametro],"--no-change-frame-resize-zone")) {
                menu_change_frame_when_resize_zone.v=0;
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

            else if (!strcmp(argv[puntero_parametro],"--process-switcher-immutable")) {
                setting_process_switcher_immutable.v=1;
            }

            else if (!strcmp(argv[puntero_parametro],"--process-switcher-always-visible")) {
                setting_process_switcher_always_visible.v=1;
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

            else if (!strcmp(argv[puntero_parametro],"--ayplayer-add-dir")) {
                siguiente_parametro_argumento();
                ay_player_add_directory_playlist(argv[puntero_parametro]);
            }

            else if (!strcmp(argv[puntero_parametro],"--ayplayer-start-playlist")) {
                command_line_ayplayer_start_playlist.v=1;
            }

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-end-exit")) {
				ay_player_exit_emulator_when_finish.v=1;
			}

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-shuffle")) {
				ay_player_shuffle_mode.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--ayplayer-no-silence-detection")) {
                ay_player_silence_detection.v=0;
            }

            //Por compatibilidad con versiones < 10.10
			else if (!strcmp(argv[puntero_parametro],"--ayplayer-end-no-repeat")) {
				ay_player_repeat_file.v=0;
			}

			else if (!strcmp(argv[puntero_parametro],"--ayplayer-end-repeat")) {
				ay_player_repeat_file.v=1;
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

            else if (!strcmp(argv[puntero_parametro],"--ayplayer-show-info-console")) {
                ay_player_show_info_console.v=1;
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

            //Este setting lo permitimos siempre, aunque no se haya compilado driver sdl, pues es una variable global, aunque no se verá en la ayuda
            else if (!strcmp(argv[puntero_parametro],"--sdl-use-callback-new")) {
                audiosdl_use_new_callback.v=1;
            }

            //Este setting lo permitimos siempre, aunque no se haya compilado driver sdl, pues es una variable global, aunque no se verá en la ayuda
            else if (!strcmp(argv[puntero_parametro],"--sdl-use-callback-old")) {
                audiosdl_use_new_callback.v=0;
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

            else if (!strcmp(argv[puntero_parametro],"--charset-customfile")) {
				siguiente_parametro_argumento();

                strcpy(char_set_customfile_path,argv[puntero_parametro]);

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

         else if (!strcmp(argv[puntero_parametro],"--enable-zeng-online-server")) {
            command_line_start_zeng_online_server.v=1;
         }


         else if (!strcmp(argv[puntero_parametro],"--zeng-online-nickname")) {
            siguiente_parametro_argumento();
            if (strlen(argv[puntero_parametro])>ZOC_MAX_NICKNAME_LENGTH) {
                printf("Nickname too long. Maximum: %d characters\n",ZOC_MAX_NICKNAME_LENGTH);
                exit(1);
            }

            strcpy(zeng_online_nickname,argv[puntero_parametro]);
         }

         else if (!strcmp(argv[puntero_parametro],"--zeng-online-server-allow-create")) {
            zeng_online_allow_room_creation_from_any_ip.v=1;
         }

         else if (!strcmp(argv[puntero_parametro],"--zeng-online-server-max-rooms")) {
            siguiente_parametro_argumento();
            int valor=parse_string_to_number(argv[puntero_parametro]);
			if (valor<1 || valor>ZENG_ONLINE_MAX_ROOMS) {
                printf ("Invalid max rooms value\n");
				 exit (1);
			 }
            zeng_online_current_max_rooms=valor;
         }


         else if (!strcmp(argv[puntero_parametro],"--zeng-online-server-max-players-room")) {
            siguiente_parametro_argumento();
            int valor=parse_string_to_number(argv[puntero_parametro]);
			if (valor<1 || valor>ZENG_ONLINE_MAX_PLAYERS_PER_ROOM) {
                printf ("Invalid max players per rooms value\n");
				 exit (1);
			 }
            zeng_online_current_max_players_per_room=valor;
         }


        else if (!strcmp(argv[puntero_parametro],"--zeng-online-server-destroy-rooms-no-players")) {
            zeng_online_destroy_rooms_without_players.v=1;
        }

        else if (!strcmp(argv[puntero_parametro],"--zeng-online-no-zip-snapshots")) {
            zeng_online_zip_compress_snapshots.v=0;
        }

        else if (!strcmp(argv[puntero_parametro],"--zeng-online-no-footer-lag-indicator")) {
            zeng_online_show_footer_lag_indicator.v=0;
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

        else if (!strcmp(argv[puntero_parametro],"--textadvmap-follow")) {
            menu_debug_textadventure_map_connections_center_current=1;
        }

        else if (!strcmp(argv[puntero_parametro],"--textadvmap-show-unconnected")) {
            menu_debug_textadventure_map_connections_show_rooms_no_connections=1;
        }

        else if (!strcmp(argv[puntero_parametro],"--textadvmap-no-show-unvisited")) {
            menu_debug_textadventure_map_connections_show_unvisited=0;
        }

        else if (!strcmp(argv[puntero_parametro],"--textadvmap-no-show-objects")) {
            menu_debug_textadventure_map_connections_show_objects=0;
        }

        else if (!strcmp(argv[puntero_parametro],"--textadvmap-no-show-pictures")) {
            menu_debug_textadventure_map_connections_show_pictures=0;
        }


        else if (!strcmp(argv[puntero_parametro],"--textadvmap-zoom")) {
            siguiente_parametro_argumento();

            int valor=parse_string_to_number(argv[puntero_parametro]);


            if (valor<0 || valor>MAX_TEXTADVENTURE_MAP_ZOOM) {
                printf("Text adventure zoom out of range\n");
                exit(1);
            }

            menu_debug_textadventure_map_connections_zoom=valor;

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

            //por compatibilidad hacia atras
			else if (!strcmp(argv[puntero_parametro],"--zeng-snapshot-interval")) {
				siguiente_parametro_argumento();

				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>9) {
						printf ("Invalid value %d for setting --zeng-snapshot-interval\n",valor);
						exit(1);
				}

				zeng_frames_video_cada_snapshot=valor*50;
			}

			else if (!strcmp(argv[puntero_parametro],"--zeng-snapshot-interval-frames")) {
				siguiente_parametro_argumento();

				int valor=parse_string_to_number(argv[puntero_parametro]);
				if (valor<1 || valor>9*50) {
						printf ("Invalid value %d for setting --zeng-snapshot-interval-frames\n",valor);
						exit(1);
				}

				zeng_frames_video_cada_snapshot=valor;
			}

			else if (!strcmp(argv[puntero_parametro],"--zeng-iam-master")) {
				zeng_i_am_master=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--zeng-not-send-input-events")) {
                zeng_do_not_send_input_events=1;
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





			else if (!strcmp(argv[puntero_parametro],"--last-version-text")) {
				siguiente_parametro_argumento();
				strcpy(last_version_text_string,argv[puntero_parametro]);
			}


			else if (!strcmp(argv[puntero_parametro],"--last-version")) {
				siguiente_parametro_argumento();
				strcpy(last_version_string,argv[puntero_parametro]);

                last_buildnumber_int=atoi(last_version_string);

                if (buildnumber_int<last_buildnumber_int) {
                    printf("It seems you have downgraded ZEsarUX from %s to %s\n"
                    "If there is any unknown parameter on the configuration file, from the moment that parameter is detected, the rest of the parameters are tried to be read\n",
                        last_version_text_string,EMULATOR_VERSION);
                    zesarux_has_been_downgraded.v=1;
                    sleep(3);
                }
			}

			else if (!strcmp(argv[puntero_parametro],"--no-show-changelog")) {
				do_no_show_changelog_when_update.v=1;
			}

            else if (!strcmp(argv[puntero_parametro],"--no-show-david-in-memoriam")) {
                do_no_show_david_in_memoriam.v=1;
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



			else if (!strcmp(argv[puntero_parametro],"--history-item-add-debugcpu-ptr")) {
				siguiente_parametro_argumento();
                util_scanf_history_insert(menu_debug_registers_change_ptr_historial,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--history-item-add-hexeditor-ptr")) {
				siguiente_parametro_argumento();
                util_scanf_history_insert(menu_debug_hexdump_change_ptr_historial,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--history-item-add-sprites-ptr")) {
				siguiente_parametro_argumento();
                util_scanf_history_insert(menu_debug_sprites_change_ptr_historial,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--history-item-add-poke-ptr")) {
				siguiente_parametro_argumento();
                util_scanf_history_insert(menu_debug_poke_address_historial,argv[puntero_parametro]);
			}

			else if (!strcmp(argv[puntero_parametro],"--history-item-add-poke-value")) {
				siguiente_parametro_argumento();
                util_scanf_history_insert(menu_debug_poke_value_historial,argv[puntero_parametro]);
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

            else if (!strcmp(argv[puntero_parametro],"--convert-tap-scr")) {
                siguiente_parametro_argumento();
                char *origen=argv[puntero_parametro];
                siguiente_parametro_argumento();
                char *destino=argv[puntero_parametro];

                printf("Converting from TAP file %s to SCR file %s\n",origen,destino);

                if (util_convert_any_to_scr(origen,destino)) {
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


                if (desde_commandline) {
				    //parametro desconocido por linea de comandos, avisar con error
				    debug_printf (VERBOSE_ERR,"Unknown parameter : %s . Stopping parsing the rest of parameters",argv[puntero_parametro]);
                    return 1;
                }

                else {


                    //si en cambio estamos parseando archivo de configuracion, hacerlo mas tolerante, arrancar pero con aviso
                    debug_printf (VERBOSE_ERR,"Unknown parameter : %s",argv[puntero_parametro]);

                    //Nos vamos hasta siguiente parametro que empiece con "--"
                    int salir=0;

                    do {

                        //Tenemos que situarnos justo antes del siguiente parametro, pues el bucle es tal cual lo parsea

                        if (argc<=1) {
                            //printf("Fin desde argc<=1\n");
                            salir=1;
                        }

                        else {
                            if (argv[puntero_parametro+1][0]=='-' && argv[puntero_parametro+1][1]=='-') {
                                //printf("Fin desde encontrado siguiente --\n");
                                salir=1; //encontrado siguiente
                            }
                            else {
                                argc--;
                                puntero_parametro++;
                            }
                        }

                    } while (!salir);

                }



			}


		}

		//Fin de interpretacion de parametros
		return 0;

}

//Proceso inicial
int zesarux_main (int main_argc,char *main_argv[]) {

	if (main_argc>1) {
		if (!strcmp(main_argv[1],"--version")) {
		//	printf ("ZEsarUX Version: " EMULATOR_VERSION " Date: " EMULATOR_DATE " - " EMULATOR_EDITION_NAME "\n");
			printf ("ZEsarUX v." EMULATOR_VERSION " - " EMULATOR_EDITION_NAME ". " EMULATOR_DATE  "\n");
			exit(0);
		}

		if (!strcmp(main_argv[1],"--machinelist")) {
			get_machine_list_whitespace();
			printf("\n");
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

    //conversion de valor BUILDNUMBER a entero
    buildnumber_int=atoi(BUILDNUMBER);
    //printf("build number %u\n",buildnumber_int);


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
    //scr_driver_can_ext_desktop=NULL;
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

    //TODO: ZX81 de serie tiene 1 kb, TS1000 tiene 2 kb, y TS1500 tiene 16 kb.
    //En cambio estamos haciendo que todos ellos inicien con 16 kb

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


#ifndef NETWORKING_DISABLED
	omplir_adr_internet_semaforo_init();
#endif


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

    zxvision_index_menu_init();
    zxvision_index_load_from_disk();

    ay_player_playlist_init();

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
            zesarux_cmdline_help();
            exit(1);
        }

        //Lo mismo para experthelp
        if (!strcmp(main_argv[1],"--experthelp")) {
            zesarux_cmdline_help_expert();
            exit(1);
        }

    }



	if (noconfigfile==0) {
        //parametros del archivo de configuracion
        configfile_parse();

        argc=configfile_argc;
        argv=configfile_argv;
        puntero_parametro=0;

        //Desde parseo de archivo de config no se genera error nunca, se es mas tolerante, avisando del error, pero
        //parseando siguientes parametros

        parse_cmdline_options(0);

        /*

        if (parse_cmdline_options(0)) {
            //Desactivamos autoguardado para evitar que se genere una configuración incompleta
            //Pero solo si no ha habido downgrade
            //Si hay un downgrade, se avisara al usuario
            if (zesarux_has_been_downgraded.v==0) {
                save_configuration_file_on_exit.v=0;
            }
        }
        */
	}


  	//Luego parseamos parametros por linea de comandos
  	argc=main_argc;
  	argv=main_argv;
  	puntero_parametro=0;

  	if (parse_cmdline_options(1)) {
		printf ("\n\n");
        zesarux_cmdline_help();
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

    get_os_release();



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


	/*if (param_custom_romfile!=NULL) {
		set_machine(param_custom_romfile);
	}*/


	//else {
		set_machine(NULL);
	//}

	cold_start_cpu_registers();

	reset_cpu();

	//Algun parametro que se resetea con reset_cpu y/o set_machine y se puede haber especificado por linea de comandos
	if (command_line_zx8081_vsync_sound.v) zx8081_vsync_sound.v=1;

    //llamar a set_menu_gui_zoom para establecer zoom menu. Ya se ha llamado desde set_machine pero como no hay driver de video aun ahi,
    //no se aplica zoom de gui dado que eso solo es para driver xwindows, sdl etc y no para curses y otros
	//Esto tiene que ir justo aqui antes de init driver video pues al cambiar a veces gui zoom a 2 (caso tbblue o cpc por ejemplo),
	//el tamaño de zx desktop se multiplica por el gui zoom y entonces la memoria a asignar del driver de video es mayor
    set_menu_gui_zoom();


    //Inicializamos Video antes que el resto de cosas.
    main_init_video();


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

    //Texto recordatorio de David, solo la primera vez
	//solo si el autoguardado de config esta activado
    //Al salir, se activara la opcion de no mostrar de nuevo el recordatorio
	if (save_configuration_file_on_exit.v && do_no_show_david_in_memoriam.v==0) {

        if (!strcmp(EMULATOR_GAME_EDITION,"David")) {

			//Y si driver permite menu normal
			if (si_normal_menu_video_driver()) {
                activated_in_memoriam_david.v=1;
			}

            //Solo mostrarlo una vez, a la siguiente ya no se vera
            do_no_show_david_in_memoriam.v=1;
		}
	}


	if (opcion_no_welcome_message.v==0) {
		set_welcome_message();
	}

	else {
		//Cuando hay splash, la propia funcion set_welcome_message llama a cls_menu_overlay y esta llama a menu_draw_ext_desktop
        //y luego a show_all_windows_startup
		menu_draw_ext_desktop();
        show_all_windows_startup();


        //no hay splash, si hay In Memoriam David, forzar aparecer menu
        if (activated_in_memoriam_david.v) {
             menu_set_menu_abierto(1);
        }
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

    if (command_line_ayplayer_start_playlist.v) {
        ay_player_start_playing_all_items();
    }



	//Si la version actual es mas nueva que la anterior mostrar changelog
    //eso solo si el autoguardado de config esta activado
    //Y no hacer saltar esto cuando sale el In Memoriam de David
	if (save_configuration_file_on_exit.v && do_no_show_changelog_when_update.v==0 && activated_in_memoriam_david.v==0) {
		//if (strcmp(last_version_string,EMULATOR_VERSION)) {  //Si son diferentes
		if (strcmp(last_version_string,BUILDNUMBER) && last_version_string[0]!=0) {  //Si son diferentes y last_version_string no es nula
			//Y si driver permite menu normal
			if (si_normal_menu_video_driver()) {
                //Y si version actual es mayor que la anterior
                if (buildnumber_int>last_buildnumber_int) {
                    menu_event_new_version_show_changes.v=1;
                    menu_set_menu_abierto(1);
                }
				//menu_abierto=1;
			}
		}
	}



    //Si la version actual es mas vieja, aviso del downgrade
    if (zesarux_has_been_downgraded.v) {
       menu_set_menu_abierto(1);
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

    //Inicializar ZENG online
    init_zeng_online_rooms();

	//Iniciar ZRCP
	init_remote_protocol();

    //Habilitar zeng online si conviene
    if (command_line_start_zeng_online_server.v) {
        if (remote_protocol_enabled.v) {
            printf("Iniciando zeng online server\n");
            enable_zeng_online();
        }
    }


    generate_stats_uuid();

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


int ending_emulator_flag=0;

//Se pasa parametro que dice si guarda o no la configuración.
//antes se guardaba siempre, pero ahora en casos de recepcion de señales de terminar, no se guarda,
//pues generaba segfaults en las rutinas de guardar ventanas (--restorewindow)
void end_emulator_saveornot_config(int saveconfig)
{
	debug_printf (VERBOSE_INFO,"End emulator");

	//Para indicar al thread de emulacion que tiene que salir, esto es valido cuando se llega aqui con ctrl-c
	//Si no, se quedaria el loop de emulacion por debajo y en cuanto aqui cerramos el driver de video,
	//petaria con segfault al intentar refrescar la pantalla o similar
    ending_emulator_flag=1;
	//Dejamos un ligero tiempo para que el thread se entere
	//1 milisegundo mas que suficiente
	usleep(1000);



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

    //Guardar indice de busqueda de menu
    zxvision_index_save_to_disk();

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