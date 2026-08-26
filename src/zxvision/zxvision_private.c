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
   Private menu interface functions, also known as "ZX Vision"
*/

//
// Archivo para funciones auxiliares de soporte de menu, funciones y objetos privados que no tienen por que verlo desde los menu*.c
//


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



#include "zxvision.h"
#include "zxvision_private.h"
#include "zxvision_topbar.h"
#include "menu_items.h"
#include "menu_items_settings.h"
#include "menu_items_storage.h"
#include "menu_bitmaps.h"
#include "menu_debug_cpu.h"
#include "menu_file_viewer_browser.h"
#include "menu_filesel.h"
#include "menu_zeng_online.h"
#include "screen.h"
#include "cpu.h"
#include "start.h"
#include "debug.h"
#include "ay38912.h"
#include "tape.h"
#include "audio.h"
#include "timer.h"
#include "operaciones.h"
#include "utils.h"
#include "utils_math.h"
#include "joystick.h"
#include "ula.h"
#include "realjoystick.h"
#include "scrstdout.h"
#include "autoselectoptions.h"
#include "charset.h"
#include "chardetect.h"
#include "textspeech.h"
#include "prism.h"
#include "cpc.h"
#include "sam.h"
#include "tbblue.h"
#include "remote.h"
#include "tsconf.h"
#include "settings.h"
#include "stats.h"
#include "network.h"
#include "ql.h"
#include "zvfs.h"
#include "pcw.h"
#include "pd765.h"
#include "dsk.h"
#include "zeng_online_client.h"
#include "ql_qdos_handler.h"
#include "ql_i8049.h"

#if defined(__APPLE__)
    #include <sys/syslimits.h>

    #include <sys/resource.h>

#endif

#include "compileoptions.h"

#ifdef COMPILE_CURSES
    #include "scrcurses.h"
#endif

#ifdef COMPILE_AA
    #include "scraa.h"
#endif

#ifdef COMPILE_STDOUT
    #include "scrstdout.h"
//macro llama a funcion real
    #define scrstdout_menu_print_speech_macro scrstdout_menu_print_speech
//funcion llama
#else
//funcion no llama a nada
    #define scrstdout_menu_print_speech_macro(x)
#endif


#ifdef COMPILE_XWINDOWS
    #include "scrxwindows.h"
#endif