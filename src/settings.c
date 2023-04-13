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
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>

#include "settings.h"

#include "zxvision.h"
#include "menu_items.h"
#include "screen.h"
#include "cpu.h"
#include "debug.h"
#include "zx8081.h"
#include "ay38912.h"
#include "tape.h"
#include "audio.h"
#include "timer.h"
#include "snap.h"
#include "operaciones.h"
#include "disassemble.h"
#include "utils.h"
#include "contend.h"
#include "joystick.h"
#include "ula.h"
#include "printers.h"
#include "realjoystick.h"
#include "scrstdout.h"
#include "z88.h"
#include "ulaplus.h"
#include "autoselectoptions.h"
#include "zxuno.h"
#include "charset.h"
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
#include "pd765.h"
#include "tbblue.h"
#include "dandanator.h"
#include "superupgrade.h"
#include "m68k.h"
#include "remote.h"
#include "snap_rzx.h"
#include "multiface.h"
#include "scmp.h"
#include "esxdos_handler.h"
#include "tsconf.h"
#include "kartusho.h"
#include "ifrom.h"
#include "spritefinder.h"
#include "snap_spg.h"
#include "betadisk.h"
#include "tape_tzx.h" 
#include "snap_zsf.h"
#include "compileoptions.h"


#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif


//
// Archivo para settings que se pueden cambiar del emulador, generalmente aquellos del menu y tambien del command line
// Aunque aun falta mucho por mover, la mayoria de settings estan esparcidos por todos los archivos de codigo fuente
//



//
// Debug Settings
//

z80_bit menu_debug_registers_if_showscan={0};

//Si muestra en que scanline se ha disparado un halt
z80_bit debug_settings_show_fired_halt={0};

//Si borra el menu a cada pulsacion y muestra la pantalla de la maquina emulada debajo
z80_bit debug_settings_show_screen={0};

int debug_registers=0;

//Si se muestra por verbose debug los opcodes incorrectos
z80_bit debug_shows_invalid_opcode={0};

//Comportamiento al saltar un breakpoint:
//0: salta el breakpoint siempre que se cumpla condicion
//1: salta el breakpoint cuando la condicion pasa de false a true
z80_bit debug_breakpoints_cond_behaviour={1};

//Opciones al hacer debug, en este caso, al ejecutar comandos paso a paso
//Ver ayuda de comando set-debug-settings para entender significado
int remote_debug_settings=1;


//Si mostrar aviso cuando se cumple un breakpoint
int debug_show_fired_breakpoints_type=0;
//0: siempre
//1: solo cuando condicion no es tipo "PC=XXXX"
//2: nunca


int verbose_level=0;


int remote_protocol_port=DEFAULT_REMOTE_PROTOCOL_PORT;
z80_bit remote_protocol_enabled={0};
char remote_prompt_command_string[REMOTE_MAX_PROMPT_LENGTH]="command";

//Puerto de debug ZEsarUX activo o no
z80_bit hardware_debug_port={0};

char zesarux_zxi_hardware_debug_file[PATH_MAX]="";


//Si se muestra visualmem grafico en drivers grafico. Si no, muestra visualmem de texto en drivers graficos
z80_bit setting_mostrar_visualmem_grafico={1}; 

//Si se muestra direccion en cada linea en View Basic
z80_bit debug_view_basic_show_address={0};

//
// Snapshot Settings
//


//Autograbar snapshot al salir
z80_bit autosave_snapshot_on_exit;

//Autocargar snapshot al iniciar
z80_bit autoload_snapshot_on_start;

//ruta de autoguardado
char autosave_snapshot_path_buffer[PATH_MAX];

//Sincronizar reloj del sistema al Z88 despues cargar snapshot
z80_bit sync_clock_to_z88={1};


//
// Tape Settings
//

//Si el autoload (reset machine, load"") se lanza con top speed
z80_bit fast_autoload={0};

//Suprimir pausas de los archivos tzx
z80_bit tzx_suppress_pause={0};

//Nuevo algoritmo de carga real tape
z80_bit realtape_algorithm_new={0};


//
// PD765 Settings
//

//Si al activar proteccion de escritura en dsk, la controladora no retorna error a la cpu (silent mode)
z80_bit pd765_silent_write_protection={0};


//
// Audio Settings
//

//Si se muestra piano grafico en drivers grafico. Si no, muestra piano de texto en drivers graficos
z80_bit setting_mostrar_ay_piano_grafico={1}; 

//Zoom usado en Audio Chip piano y Wave Piano
int audiochip_piano_zoom_x=3;
int audiochip_piano_zoom_y=3;

//SDL usar callback de audio antiguo o nuevo
#ifdef MINGW
//En Windows usar el nuevo, para evitar clicks
z80_bit audiosdl_use_new_callback={1};
#else
//En Linux usar el viejo, porque el nuevo produce clicks
z80_bit audiosdl_use_new_callback={0};
#endif

//
// Hardware Settings
//

//Sensibilidad usada al leer kempston mouse desde spectrum
int kempston_mouse_factor_sensibilidad=1;


//z80_bit ql_replace_underscore_dot={1};
//z80_bit ql_replace_underscore_dot_only_one={1};

z80_bit ql_flp1_follow_mdv1={0};

//
// Display Settings
//

//Si ocultar legenda de teclas del Z88
z80_bit z88_hide_keys_shortcuts={0};

//Si debe enviar un espacio al final de cada palabra del adventure keyboard
int adventure_keyboard_send_final_spc=0;

//Tiempo que dura la tecla total (mitad de esto pulsada, mitad no pulsada). En 1/50 de segundo
int adventure_keyboard_key_length=DEFAULT_ADV_KEYBOARD_KEY_LENGTH;


//Usar caracteres extendidos de cursesw. La opcion se puede usar aunque no este compilado cursesw
//(se cargará y grabará de config aunque no tenga ningún efecto)
z80_bit use_scrcursesw={0};


//Si se ha preguntado ya para descargar la imagen SD al seleccionar maquina tbblue
z80_bit tbblue_autoconfigure_sd_asked={0};



//Guardar scanlines en pixel/atributos en tbblue, requerido para demos hi-res (de spectrum , no de next necesariamente)
//Por defecto, desactivado
z80_bit tbblue_store_scanlines={0};

//Lo mismo pero para el border
z80_bit tbblue_store_scanlines_border={0};

//Sin limites de sprites por linea en chip vdp 9918a
z80_bit vdp_9918a_unlimited_sprites_line={0};

//Cambiar color paper en carga de msx
z80_bit msx_loading_stripes={0};

//Simular parpadeo cursor QL
//z80_bit ql_simular_parpadeo_cursor={1};


//Reduccion ruido
z80_bit msx_loading_noise_reduction={0};

//
// Windows Settings
//

//Si permitimos o no ventanas en background al pulsar F6
int menu_allow_background_windows=0;

//Forzar siempre overlay_visible_when_menu_closed al cerrar el menu
int always_force_overlay_visible_when_menu_closed=0;

//Reabrir ventanas al iniciar el emulador. Por defecto esto se hace siempre ya desde la version 10.2
z80_bit menu_reopen_background_windows_on_start={1};


//
// GUI Settings
//


//Si se selecciona maquina con listado ordenado por nombre, en vez de por fabricante
z80_bit setting_machine_selection_by_name={0};


//Si el visor de archivos siempre muestra en hexadecimal
z80_bit menu_file_viewer_always_hex={0};

//Si mostrar items avanzados de menu
z80_bit menu_show_advanced_items={0};

//
// ZX Desktop Settings
//

//Desactivar zx desktop al pasar a full screen
int zxdesktop_disable_on_full_screen=0;

//Desactivar mostrar recuadro alrededor pantalla emulada
int zxdesktop_disable_show_frame_around_display=0;

z80_bit zxdesktop_configurable_icons_enabled={1};

z80_bit zxdesktop_empty_trash_on_exit={0};


z80_bit zxdesktop_icon_show_app_open={1};

//
// General Settings
//

int gui_language=GUI_LANGUAGE_DEFAULT;

//Ruta de guardado de descargas juegos
char online_download_path[PATH_MAX]="";