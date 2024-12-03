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
   Several utilities functions
*/

//para strcasestr
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>



#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#ifdef MINGW
//Para usar GetLogicalDrives
#include <winbase.h>
#endif


#include "cpu.h"
#include "start.h"
#include "utils.h"
#include "debug.h"
#include "joystick.h"
#include "compileoptions.h"
#include "tape.h"
#include "snap.h"
#include "screen.h"
#include "realjoystick.h"
#include "zxvision.h"
#include "menu_file_viewer_browser.h"
#include "menu_filesel.h"
#include "menu_debug_cpu.h"
#include "z88.h"
#include "audio.h"
#include "playtzx.h"
#include "tape_tzx.h"
#include "tape_pzx.h"
#include "operaciones.h"
#include "zx8081.h"
#include "ay38912.h"
#include "autoselectoptions.h"
#include "ulaplus.h"
#include "textspeech.h"
#include "spectra.h"
#include "zxuno.h"
#include "spritechip.h"
#include "timex.h"
#include "cpc.h"
#include "timer.h"
#include "sam.h"
#include "divmmc.h"
#include "divide.h"
#include "superupgrade.h"
#include "chardetect.h"
#include "zx8081.h"
#include "jupiterace.h"
#include "mmc.h"
#include "ide.h"
#include "zxpand.h"
#include "diviface.h"
#include "divmmc.h"
#include "divide.h"
#include "dandanator.h"
#include "ql.h"
#include "m68k.h"
#include "remote.h"
#include "ula.h"
#include "snap_rzx.h"
#include "scmp.h"
#include "mk14.h"
#include "esxdos_handler.h"
#include "tbblue.h"
#include "tsconf.h"
#include "kartusho.h"
#include "ifrom.h"
#include "mdvtool.h"
#include "betadisk.h"
#include "multiface.h"
#include "pd765.h"
#include "baseconf.h"
#include "settings.h"
#include "chloe.h"
#include "zeng.h"
#include "network.h"
#include "stats.h"
#include "scl2trd.h"
#include "zip.h"
#include "msx.h"
#include "coleco.h"
#include "sg1000.h"
#include "sms.h"
#include "svi.h"
#include "charset.h"
#include "snap_zsf.h"
#include "ql_qdos_handler.h"
#include "ql_i8049.h"
#include "samram.h"

#include "ff.h"
#include "diskio.h"
#include "zvfs.h"
#include "snap_ram.h"
#include "menu_items.h"
#include "menu_items_storage.h"
#include "hilow_datadrive.h"
#include "tape_smp.h"
#include "utils_text_adventure.h"
#include "hilow_datadrive_audio.h"
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
#include "menu_bitmaps.h"
#include "zeng_online.h"
#include "zeng_online_client.h"
#include "if1.h"
#include "microdrive.h"
#include "microdrive_raw.h"
#include "lec.h"

//Archivo usado para entrada de teclas
FILE *ptr_input_file_keyboard;
//Nombre archivo
char input_file_keyboard_name_buffer[PATH_MAX];
//Puntero que apunta al nombre
char *input_file_keyboard_name=NULL;
//Si esta insertado
z80_bit input_file_keyboard_inserted;
//Si esta en play (y no pausado)
z80_bit input_file_keyboard_playing;

//Pausa en valores de 1/50 segundos
int input_file_keyboard_delay=5;
//Contador actual
int input_file_keyboard_delay_counter=0;
//Pendiente de leer siguiente tecla
z80_bit input_file_keyboard_pending_next;
//Si lo siguiente es pausa o tecla
z80_bit input_file_keyboard_is_pause;
//Si hay que enviar pausa despues de cada pulsacion o no
z80_bit input_file_keyboard_send_pause;

//modo turbo de spool file
z80_bit input_file_keyboard_turbo={0};

//Si se guarda la configuracion al salir del programa
z80_bit save_configuration_file_on_exit={0};

//ultima tecla leida
unsigned char input_file_keyboard_last_key;

void parse_customfile_options(void);


char *customconfigfile=NULL;

int write_rom_nested_id_poke_byte;
int write_rom_nested_id_poke_byte_no_time;


//Utilidades externas
char external_tool_sox[PATH_MAX]="/usr/bin/sox";
//char external_tool_unzip[PATH_MAX]="/usr/bin/unzip";
char external_tool_gunzip[PATH_MAX]="/bin/gunzip";
char external_tool_tar[PATH_MAX]="/bin/tar";
char external_tool_unrar[PATH_MAX]="/usr/bin/unrar";

//Para interfaz con FatFS
char fatfs_disk_zero_path[PATH_MAX]="/Users/cesarhernandez/Desktop/pruebas_fatfs.img";


//tablas de conversion de teclado
struct x_tabla_teclado tabla_teclado_numeros[]={
//0
                                        {&puerto_61438, 1},
                                        {&puerto_63486, 1},
                                        {&puerto_63486, 2},
                                        {&puerto_63486, 4},
//4
                                        {&puerto_63486, 8},
                                        {&puerto_63486, 16},
                                        {&puerto_61438, 16},
                                        {&puerto_61438, 8},
//8
                                        {&puerto_61438, 4},
                                        {&puerto_61438, 2}
};

struct x_tabla_teclado tabla_teclado_letras[]={
        {&puerto_65022,1},   //A
        {&puerto_32766,16},
        {&puerto_65278,8},
        {&puerto_65022,4},   //D

                                        {&puerto_64510, 4},
                                        {&puerto_65022, 8},
                                        {&puerto_65022, 16},
                                        {&puerto_49150, 16},
                                        {&puerto_57342, 4},
                                        {&puerto_49150, 8},
                                        {&puerto_49150, 4},
                                        {&puerto_49150, 2},
                                        {&puerto_32766, 4},
//M
                                        {&puerto_32766, 8},
                                        {&puerto_57342, 2},
                                        {&puerto_57342, 1},
                                        {&puerto_64510, 1},
                                        {&puerto_64510, 8},
                                        {&puerto_65022, 2},
                                        {&puerto_64510, 16},
//U
                                        {&puerto_57342, 8},
                                        {&puerto_65278, 16},
                                        {&puerto_64510, 2},
                                        {&puerto_65278, 4},
                                        {&puerto_57342, 16},
                                        {&puerto_65278, 2}


};

struct x_tabla_teclado z88_tabla_teclado_numeros[]={
	{&blink_kbd_a13,1}, //0
	{&blink_kbd_a13,32},
	{&blink_kbd_a12,32},
	{&blink_kbd_a11,32},
	{&blink_kbd_a10,32},
	{&blink_kbd_a9,32},	//5
	{&blink_kbd_a8,32},
	{&blink_kbd_a8,2},
	{&blink_kbd_a8,1},
	{&blink_kbd_a11,1}

};

struct x_tabla_teclado cpc_tabla_teclado_numeros[]={
        {&cpc_keyboard_table[4],1}, //0
        {&cpc_keyboard_table[8],1},
        {&cpc_keyboard_table[8],2},
        {&cpc_keyboard_table[7],2},
        {&cpc_keyboard_table[7],1},
        {&cpc_keyboard_table[6],2},     //5
        {&cpc_keyboard_table[6],1},
        {&cpc_keyboard_table[5],2},
        {&cpc_keyboard_table[5],1},
        {&cpc_keyboard_table[4],2}

};

//Teclado de pcw y cpc iguales...
struct x_tabla_teclado pcw_tabla_teclado_numeros[]={
        {&pcw_keyboard_table[4],1}, //0
        {&pcw_keyboard_table[8],1},
        {&pcw_keyboard_table[8],2},
        {&pcw_keyboard_table[7],2},
        {&pcw_keyboard_table[7],1},
        {&pcw_keyboard_table[6],2},     //5
        {&pcw_keyboard_table[6],1},
        {&pcw_keyboard_table[5],2},
        {&pcw_keyboard_table[5],1},
        {&pcw_keyboard_table[4],2}

};


struct x_tabla_teclado z88_tabla_teclado_letras[]={
	{&blink_kbd_a13,8}, //A
	{&blink_kbd_a9,4},
	{&blink_kbd_a11,4},
	{&blink_kbd_a11,8}, //D
	{&blink_kbd_a11,16},
	{&blink_kbd_a10,8},
	{&blink_kbd_a9,8},
	{&blink_kbd_a8,8}, //H
	{&blink_kbd_a9,1},
	{&blink_kbd_a10,2},
	{&blink_kbd_a11,2},
	{&blink_kbd_a13,2}, //L
	{&blink_kbd_a12,2},
	{&blink_kbd_a8,4}, //N
	{&blink_kbd_a10,1},
	{&blink_kbd_a12,1}, //P
	{&blink_kbd_a13,16},
	{&blink_kbd_a10,16},
	{&blink_kbd_a12,8},
	{&blink_kbd_a9,16}, //T
	{&blink_kbd_a9,2},
	{&blink_kbd_a10,4},
	{&blink_kbd_a12,16},
	{&blink_kbd_a12,4}, //X
	{&blink_kbd_a8,16},
	{&blink_kbd_a13,4}
};


struct x_tabla_teclado cpc_tabla_teclado_letras[]={
	{&cpc_keyboard_table[8],32}, //A
	{&cpc_keyboard_table[6],64},
        {&cpc_keyboard_table[7],64},
        {&cpc_keyboard_table[7],32}, //D
        {&cpc_keyboard_table[7],4},
        {&cpc_keyboard_table[6],32},
        {&cpc_keyboard_table[6],16},
        {&cpc_keyboard_table[5],16}, //H
        {&cpc_keyboard_table[4],8},
        {&cpc_keyboard_table[5],32},
        {&cpc_keyboard_table[4],32},
        {&cpc_keyboard_table[4],16}, //L
        {&cpc_keyboard_table[4],64},
        {&cpc_keyboard_table[5],64}, //N
        {&cpc_keyboard_table[4],4},
        {&cpc_keyboard_table[3],8}, //P
        {&cpc_keyboard_table[8],8},
        {&cpc_keyboard_table[6],4},
        {&cpc_keyboard_table[7],16},
        {&cpc_keyboard_table[6],8}, //T
        {&cpc_keyboard_table[5],4},
        {&cpc_keyboard_table[6],128},
        {&cpc_keyboard_table[7],8},
        {&cpc_keyboard_table[7],128}, //X
        {&cpc_keyboard_table[5],8},
        {&cpc_keyboard_table[8],128}
};

//Teclado de pcw y cpc iguales...
struct x_tabla_teclado pcw_tabla_teclado_letras[]={
	{&pcw_keyboard_table[8],32}, //A
	{&pcw_keyboard_table[6],64},
        {&pcw_keyboard_table[7],64},
        {&pcw_keyboard_table[7],32}, //D
        {&pcw_keyboard_table[7],4},
        {&pcw_keyboard_table[6],32},
        {&pcw_keyboard_table[6],16},
        {&pcw_keyboard_table[5],16}, //H
        {&pcw_keyboard_table[4],8},
        {&pcw_keyboard_table[5],32},
        {&pcw_keyboard_table[4],32},
        {&pcw_keyboard_table[4],16}, //L
        {&pcw_keyboard_table[4],64},
        {&pcw_keyboard_table[5],64}, //N
        {&pcw_keyboard_table[4],4},  //O
        {&pcw_keyboard_table[3],8}, //P
        {&pcw_keyboard_table[8],8},
        {&pcw_keyboard_table[6],4},
        {&pcw_keyboard_table[7],16},
        {&pcw_keyboard_table[6],8}, //T
        {&pcw_keyboard_table[5],4},
        {&pcw_keyboard_table[6],128},
        {&pcw_keyboard_table[7],8},
        {&pcw_keyboard_table[7],128}, //X
        {&pcw_keyboard_table[5],8},
        {&pcw_keyboard_table[8],128}
};

struct x_tabla_teclado msx_tabla_teclado_letras[]={
	{&msx_keyboard_table[2],64}, //A
	{&msx_keyboard_table[2],128},
        {&msx_keyboard_table[3],1},
        {&msx_keyboard_table[3],2}, //D
        {&msx_keyboard_table[3],4},
        {&msx_keyboard_table[3],8},
        {&msx_keyboard_table[3],16},
        {&msx_keyboard_table[3],32}, //H
        {&msx_keyboard_table[3],64},
        {&msx_keyboard_table[3],128},
        {&msx_keyboard_table[4],1},
        {&msx_keyboard_table[4],2}, //L
        {&msx_keyboard_table[4],4},
        {&msx_keyboard_table[4],8}, //N
        {&msx_keyboard_table[4],16},
        {&msx_keyboard_table[4],32}, //P
        {&msx_keyboard_table[4],64},
        {&msx_keyboard_table[4],128},
        {&msx_keyboard_table[5],1},
        {&msx_keyboard_table[5],2}, //T
        {&msx_keyboard_table[5],4},
        {&msx_keyboard_table[5],8},
        {&msx_keyboard_table[5],16},
        {&msx_keyboard_table[5],32}, //X
        {&msx_keyboard_table[5],64},
        {&msx_keyboard_table[5],128}
};


struct x_tabla_teclado msx_tabla_teclado_numeros[]={
        {&msx_keyboard_table[0],1}, //0
        {&msx_keyboard_table[0],2},
        {&msx_keyboard_table[0],4},
        {&msx_keyboard_table[0],8},
        {&msx_keyboard_table[0],16},
        {&msx_keyboard_table[0],32},     //5
        {&msx_keyboard_table[0],64},
        {&msx_keyboard_table[0],128},
        {&msx_keyboard_table[1],1},
        {&msx_keyboard_table[1],2}

};



struct x_tabla_teclado svi_tabla_teclado_letras[]={
	{&svi_keyboard_table[2],2}, //A
	{&svi_keyboard_table[2],4},
        {&svi_keyboard_table[2],8},
        {&svi_keyboard_table[2],16}, //D
        {&svi_keyboard_table[2],32},
        {&svi_keyboard_table[2],64},
        {&svi_keyboard_table[2],128},
        {&svi_keyboard_table[3],1}, //H
        {&svi_keyboard_table[3],2},
        {&svi_keyboard_table[3],4},
        {&svi_keyboard_table[3],8},
        {&svi_keyboard_table[3],16}, //L
        {&svi_keyboard_table[3],32},
        {&svi_keyboard_table[3],64}, //N
        {&svi_keyboard_table[3],128},
        {&svi_keyboard_table[4],1}, //P
        {&svi_keyboard_table[4],2},
        {&svi_keyboard_table[4],4},
        {&svi_keyboard_table[4],8},
        {&svi_keyboard_table[4],16}, //T
        {&svi_keyboard_table[4],32},
        {&svi_keyboard_table[4],64},
        {&svi_keyboard_table[4],128},
        {&svi_keyboard_table[5],1}, //X
        {&svi_keyboard_table[5],2},
        {&svi_keyboard_table[5],4}
};


struct x_tabla_teclado svi_tabla_teclado_numeros[]={
        {&svi_keyboard_table[0],1}, //0
        {&svi_keyboard_table[0],2},
        {&svi_keyboard_table[0],4},
        {&svi_keyboard_table[0],8},
        {&svi_keyboard_table[0],16},
        {&svi_keyboard_table[0],32},     //5
        {&svi_keyboard_table[0],64},
        {&svi_keyboard_table[0],128},
        {&svi_keyboard_table[1],1},
        {&svi_keyboard_table[1],2}

};


// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 0|    F4     F1      5     F2     F3     F5      4      7
// 1|   Ret   Left     Up    Esc  Right      \  Space   Down
// 2|     ]      z      .      c      b  Pound      m      '
// 3|     [   Caps      k      s      f      =      g      ;
// 4|     l      3      h      1      a      p      d      j
// 5|     9      w      i    Tab      r      -      y      o
// 6|     8      2      6      q      e      0      t      u
// 7| Shift   Ctrl    Alt      x      v      /      n      ,

//89l6ihverantyd
struct x_tabla_teclado ql_tabla_teclado_letras[]={
	      {&ql_keyboard_table[4],16}, //A
	      {&ql_keyboard_table[2],16},
        {&ql_keyboard_table[2],8},
        {&ql_keyboard_table[4],64}, //D
        {&ql_keyboard_table[6],16},//E
        {&ql_keyboard_table[3],16},
        {&ql_keyboard_table[3],64}, //G
        {&ql_keyboard_table[4],4}, //H
        {&ql_keyboard_table[5],4}, //I
        {&ql_keyboard_table[4],128}, //J
        {&ql_keyboard_table[3],4}, //k
        {&ql_keyboard_table[4],1}, //L
        {&ql_keyboard_table[2],64},
        {&ql_keyboard_table[7],64}, //N
        {&ql_keyboard_table[5],128},
        {&ql_keyboard_table[4],32}, //P
        {&ql_keyboard_table[6],8},
        {&ql_keyboard_table[5],16}, //R
        {&ql_keyboard_table[3],8},
        {&ql_keyboard_table[6],64}, //T
        {&ql_keyboard_table[6],128},
        {&ql_keyboard_table[7],16}, //V
        {&ql_keyboard_table[5],2},
        {&ql_keyboard_table[7],8}, //X
        {&ql_keyboard_table[5],64}, //Y
        {&ql_keyboard_table[2],2}
};

// ================================== matrix ============================
//        0      1      2      3      4      5      6      7
//  +-------------------------------------------------------
// 0|    F4     F1      5     F2     F3     F5      4      7
// 1|   Ret   Left     Up    Esc  Right      \  Space   Down
// 2|     ]      z      .      c      b  Pound      m      '
// 3|     [   Caps      k      s      f      =      g      ;
// 4|     l      3      h      1      a      p      d      j
// 5|     9      w      i    Tab      r      -      y      o
// 6|     8      2      6      q      e      0      t      u
// 7| Shift   Ctrl    Alt      x      v      /      n      ,

struct x_tabla_teclado ql_tabla_teclado_numeros[]={
	      {&ql_keyboard_table[6],32}, //0
        {&ql_keyboard_table[4],8},
        {&ql_keyboard_table[6],2},
        {&ql_keyboard_table[4],2}, //3
        {&ql_keyboard_table[0],64},
        {&ql_keyboard_table[0],4}, //5
        {&ql_keyboard_table[6],4},
        {&ql_keyboard_table[0],128}, //7
        {&ql_keyboard_table[6],1},
        {&ql_keyboard_table[5],1} //9
};


//El MK14 usa los 4 bits superiores de cada mk14_keystatus para almacenar el estado de las teclas
//pero yo lo almaceno en los 4 bits inferiores porque me facilita la vida tenerlo ahi en la ventana de Keyboard Help
struct x_tabla_teclado mk14_tabla_teclado_numeros[]={
        {&mk14_keystatus[0],128>>4}, //0
        {&mk14_keystatus[1],128>>4},
        {&mk14_keystatus[2],128>>4},
        {&mk14_keystatus[3],128>>4},
        {&mk14_keystatus[4],128>>4},
        {&mk14_keystatus[5],128>>4},     //5
        {&mk14_keystatus[6],128>>4},
        {&mk14_keystatus[7],128>>4},
        {&mk14_keystatus[0],64>>4},
        {&mk14_keystatus[1],64>>4}

};


//Para guardar geometria de ventanas
int total_config_window_geometry=0;
saved_config_window_geometry saved_config_window_geometry_array[MAX_CONFIG_WINDOW_GEOMETRY];

/*

Lista fabricantes
Science of Cambridge
Sinclair
Amstrad
Timex Sinclair
Investronica
Microdigital Eletronica
Zxuno team
Chloe Corporation
New Horizons (Jeff Braine)
Cambridge computers
Jupiter cantab
Miles Gordon Tech
Pentagon
Czerweny Electronica


*/

char *array_fabricantes_hotkey[]={
        "~~Amstrad",
        "Ascii C~~orp",
        "Cam~~bridge Computers",
        "~~Chloe Corporation",
        "Coleco In~~dustries",
        "Czerweny Electronica",
        "~~Investronica",
        "J~~upiter Cantab",
        "Ma~~rio Prato",
        "~~Microdigital Eletronica",
        "Miles Gordon Technolog~~y",
        "N~~edoPC",
        "Ne~~w Horizons",
        "~~Pentagon",
        "Scie~~nce of Cambridge",
        "Se~~ga",
        "~~Sinclair Research",
        "Spectravideo Intl",
        "Timex Computer",
        "~~Timex Sinclair",
        "TS ~~Labs",
        "~~VTrucco/FB Labs",
        "~~ZXUno Team"
};

//Si letra es espacio->no hay letra. spectravideo o timex computer no hay letras libres
char array_fabricantes_hotkey_letra[]="aobcd iurmyewpngs  tlvz";



//Array de maquinas por fabricante, acabado en 255
int array_maquinas_sinclair[]={
	120,121,0,1,MACHINE_ID_SPECTRUM_48_PLUS_ENG,6,160,255
};

int array_maquinas_timex_sinclair[]={
	MACHINE_ID_TIMEX_TS1000,MACHINE_ID_TIMEX_TS1500,MACHINE_ID_TIMEX_TS2068,255
};

int array_maquinas_timex_computers[]={
	MACHINE_ID_TIMEX_TC2048,MACHINE_ID_TIMEX_TC2068,255
};

int array_maquinas_cambridge_computers[]={
	130,255
};

int array_maquinas_investronica[]={
	2,20,7,255
};

int array_maquinas_microdigital_electronica[]={
	MACHINE_ID_MICRODIGITAL_TK80,MACHINE_ID_MICRODIGITAL_TK82,MACHINE_ID_MICRODIGITAL_TK82C,MACHINE_ID_MICRODIGITAL_TK83,MACHINE_ID_MICRODIGITAL_TK85,MACHINE_ID_MICRODIGITAL_TK90X,MACHINE_ID_MICRODIGITAL_TK90X_SPA,MACHINE_ID_MICRODIGITAL_TK95,MACHINE_ID_MICRODIGITAL_TK95_SPA,255
};

int array_maquinas_ascii_corp[]={
	MACHINE_ID_MSX1,255
};

int array_maquinas_spectravideo_international[]={
	MACHINE_ID_SVI_318,MACHINE_ID_SVI_328,255
};

int array_maquinas_coleco_industries[]={
	MACHINE_ID_COLECO,255
};

int array_maquinas_sega[]={
	MACHINE_ID_SG1000,MACHINE_ID_SMS,255
};

int array_maquinas_amstrad[]={
	8,9,10,11,12,13,MACHINE_ID_SPECTRUM_P3_40,MACHINE_ID_SPECTRUM_P3_41,MACHINE_ID_SPECTRUM_P3_SPA,
    MACHINE_ID_CPC_464,MACHINE_ID_CPC_4128,MACHINE_ID_CPC_664,MACHINE_ID_CPC_6128,MACHINE_ID_PCW_8256,MACHINE_ID_PCW_8512,255
};

int array_maquinas_jupiter_cantab[]={
	122,255
};

int array_maquinas_zxuno_team[]={
	14,255
};

int array_maquinas_chloe_corporation[]={
	15,16,255
};

int array_maquinas_jeff_braine[]={
	18,255
};

int array_maquinas_tbblue[]={
	19,255
};

int array_maquinas_miles_gordon[]={
	150,255
};

int array_maquinas_pentagon[]={
	21,255
};

int array_maquinas_marioprato[]={
	22,255
};

int array_maquinas_tslabs[]={
	MACHINE_ID_TSCONF,255
};

int array_maquinas_nedopc[]={
	MACHINE_ID_BASECONF,255
};

int array_maquinas_science_of_cambridge[]={
	MACHINE_ID_MK14_STANDARD,255
};

int array_maquinas_czerweny_electronica[]={
	MACHINE_ID_CZ_1000,MACHINE_ID_CZ_1500,MACHINE_ID_CZ_1000_PLUS,MACHINE_ID_CZ_1500_PLUS,MACHINE_ID_CZ_2000,MACHINE_ID_CZ_SPECTRUM,MACHINE_ID_CZ_SPECTRUM_PLUS,255
};

//Retorna array a maquinas segun fabricante
int *return_maquinas_fabricante(int fabricante)
{
	switch (fabricante) {
		case FABRICANTE_SINCLAIR:
			return array_maquinas_sinclair;
		break;

		case FABRICANTE_TIMEX_SINCLAIR:
			return array_maquinas_timex_sinclair;
		break;

		case FABRICANTE_TIMEX_COMPUTERS:
			return array_maquinas_timex_computers;
		break;

		case FABRICANTE_CAMBRIDGE_COMPUTERS:
			return array_maquinas_cambridge_computers;
		break;

		case FABRICANTE_INVESTRONICA:
			return array_maquinas_investronica;
		break;

		case FABRICANTE_MICRODIGITAL_ELECTRONICA:
			return array_maquinas_microdigital_electronica;
		break;

		case FABRICANTE_ASCII_CORP:
			return array_maquinas_ascii_corp;
		break;

		case FABRICANTE_SPECTRAVIDEO_INTERNATIONAL:
			return array_maquinas_spectravideo_international;
		break;

		case FABRICANTE_COLECO_INDUSTRIES:
			return array_maquinas_coleco_industries;
		break;

		case FABRICANTE_SEGA:
			return array_maquinas_sega;
		break;

		case FABRICANTE_AMSTRAD:
			return array_maquinas_amstrad;
		break;

		case FABRICANTE_JUPITER_CANTAB:
			return array_maquinas_jupiter_cantab;
		break;

		case FABRICANTE_ZXUNO_TEAM:
			return array_maquinas_zxuno_team;
		break;

		case FABRICANTE_CHLOE_CORPORATION:
			return array_maquinas_chloe_corporation;
		break;

		case FABRICANTE_NEW_HORIZONS:
			return array_maquinas_jeff_braine;
		break;

		case FABRICANTE_TBBLUE:
			return array_maquinas_tbblue;
		break;

		case FABRICANTE_MILES_GORDON:
			return array_maquinas_miles_gordon;
		break;

		case FABRICANTE_PENTAGON:
			return array_maquinas_pentagon;
		break;

    case FABRICANTE_MARIOPRATO:
      return array_maquinas_marioprato;
    break;

    case FABRICANTE_TSLABS:
      return array_maquinas_tslabs;
    break;

        case FABRICANTE_NEDOPC:
      return array_maquinas_nedopc;
    break;

    case FABRICANTE_SCIENCE_OF_CAMBRIDGE:
      return array_maquinas_science_of_cambridge;
    break;

    case FABRICANTE_CZERWENY_ELECTRONICA:
        return array_maquinas_czerweny_electronica;
    break;

		default:
			cpu_panic("Unknown machines made by manufacturer");
		break;
	}

	//Aqui no se llega nunca. Pero para evitar warning del compilador
	return NULL;
}


//Retorna fabricante segun tipo maquina
int return_fabricante_maquina(int maquina)
{
	switch (maquina) {
		case 0:
		case 1:
		case 6:
        case MACHINE_ID_SPECTRUM_48_PLUS_ENG:
		case 120:
		case 121:
		case 160:
			return FABRICANTE_SINCLAIR;
		break;

        case MACHINE_ID_TIMEX_TS1000:
        case MACHINE_ID_TIMEX_TS1500:
		case MACHINE_ID_TIMEX_TS2068:
			return FABRICANTE_TIMEX_SINCLAIR;
		break;

		case 130:
			return FABRICANTE_CAMBRIDGE_COMPUTERS;
		break;

		case 2:
		case 7:
		case 20:
			return FABRICANTE_INVESTRONICA;
		break;

        case MACHINE_ID_MICRODIGITAL_TK80:
        case MACHINE_ID_MICRODIGITAL_TK82:
        case MACHINE_ID_MICRODIGITAL_TK82C:
        case MACHINE_ID_MICRODIGITAL_TK83:
        case MACHINE_ID_MICRODIGITAL_TK85:
		case MACHINE_ID_MICRODIGITAL_TK90X:
		case MACHINE_ID_MICRODIGITAL_TK90X_SPA:
		case MACHINE_ID_MICRODIGITAL_TK95:
        case MACHINE_ID_MICRODIGITAL_TK95_SPA:
			return FABRICANTE_MICRODIGITAL_ELECTRONICA;
		break;

		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case MACHINE_ID_SPECTRUM_P3_40:
		case MACHINE_ID_SPECTRUM_P3_41:
		case MACHINE_ID_SPECTRUM_P3_SPA:
		case MACHINE_ID_CPC_464:
        case MACHINE_ID_CPC_4128:
        case MACHINE_ID_CPC_664:
        case MACHINE_ID_CPC_6128:
        case MACHINE_ID_PCW_8256:
        case MACHINE_ID_PCW_8512:
			return FABRICANTE_AMSTRAD;
		break;

        case MACHINE_ID_TIMEX_TC2048:
        case MACHINE_ID_TIMEX_TC2068:
            return FABRICANTE_TIMEX_COMPUTERS;
        break;

                case MACHINE_ID_COLECO:
			return FABRICANTE_COLECO_INDUSTRIES;
		break;

        case MACHINE_ID_SG1000:
        case MACHINE_ID_SMS:
			return FABRICANTE_SEGA;
		break;

                case MACHINE_ID_MSX1:
			return FABRICANTE_ASCII_CORP;
		break;

                case MACHINE_ID_SVI_318:
                case MACHINE_ID_SVI_328:
			return FABRICANTE_SPECTRAVIDEO_INTERNATIONAL;
		break;

		case 122:
			return FABRICANTE_JUPITER_CANTAB;
		break;

		case 14:
			return FABRICANTE_ZXUNO_TEAM;
		break;

		case 15:
		case 16:
			return FABRICANTE_CHLOE_CORPORATION;
		break;

		case 18:
			return FABRICANTE_NEW_HORIZONS;
		break;

		case 19:
			return FABRICANTE_TBBLUE;
		break;

		case 150:
			return FABRICANTE_MILES_GORDON;
		break;

		case 21:
			return FABRICANTE_PENTAGON;
		break;

    case MACHINE_ID_CHROME:
      return FABRICANTE_MARIOPRATO;
    break;

    case MACHINE_ID_TSCONF:
      return FABRICANTE_TSLABS;
    break;

        case MACHINE_ID_BASECONF:
      return FABRICANTE_NEDOPC;
    break;

    case MACHINE_ID_MK14_STANDARD:
      return FABRICANTE_SCIENCE_OF_CAMBRIDGE;
    break;

            case MACHINE_ID_CZ_1000:
            case MACHINE_ID_CZ_1500:
            case MACHINE_ID_CZ_1000_PLUS:
            case MACHINE_ID_CZ_1500_PLUS:
            case MACHINE_ID_CZ_2000:
            case MACHINE_ID_CZ_SPECTRUM:
            case MACHINE_ID_CZ_SPECTRUM_PLUS:
                return FABRICANTE_CZERWENY_ELECTRONICA;
            break;

		default:
			cpu_panic ("Unknown manufacturer for machine id");
		break;
	}

	//Aqui no se llega nunca. Pero para evitar warning del compilador
	return 0;
}


//Retorna posicion dentro del array de maquinas indicado, posicion en array. 255 si no se encuentra
int return_machine_position(int *array_maquinas,int id_maquina)
{
	int index=0;
	while (array_maquinas[index]!=255) {
		if (id_maquina==array_maquinas[index]) return index;
		index++;
	}

	return 255;
}

z80_bit debug_parse_config_file;


#ifdef EMULATE_CPU_STATS
//Arrays de contadores de numero de veces instrucciones usadas
unsigned int stats_codsinpr[256];
unsigned int stats_codpred[256];
unsigned int stats_codprcb[256];
unsigned int stats_codprdd[256];
unsigned int stats_codprfd[256];
unsigned int stats_codprddcb[256];
unsigned int stats_codprfdcb[256];
//codprddfd.c  codprddfd.o  codpred.c    codpred.o

void util_stats_increment_counter(unsigned int *stats_array,int index)
{
	stats_array[index]++;
}

unsigned int util_stats_get_counter(unsigned int *stats_array,int index)
{
	return stats_array[index];
}

void util_stats_set_counter(unsigned int *stats_array,int index,unsigned int value)
{
	stats_array[index]=value;
}


//Buscar dentro del indice el de mayor valor
int util_stats_find_max_counter(unsigned int *stats_array)
{
	int index_max=0;
	unsigned int value_max=0;

	int i;
	unsigned int value;

	for (i=0;i<256;i++) {
		value=util_stats_get_counter(stats_array,i);
		if (value>value_max) {
			value_max=value;
			index_max=i;
		}
	}

	return index_max;
}


void util_stats_init(void)
{

        debug_printf (VERBOSE_INFO,"Initializing CPU Statistics Counter Array");

        int i;

        for (i=0;i<256;i++) {
                util_stats_set_counter(stats_codsinpr,i,0);
                util_stats_set_counter(stats_codpred,i,0);
                util_stats_set_counter(stats_codprcb,i,0);
                util_stats_set_counter(stats_codprdd,i,0);
                util_stats_set_counter(stats_codprfd,i,0);
                util_stats_set_counter(stats_codprddcb,i,0);
                util_stats_set_counter(stats_codprfdcb,i,0);
        }
}

//Retornar la suma de todos los contadores
unsigned int util_stats_sum_all_counters(void)
{

	unsigned int total=0;
	int i;

        for (i=0;i<256;i++) {
                total +=util_stats_get_counter(stats_codsinpr,i);
                total +=util_stats_get_counter(stats_codpred,i);
                total +=util_stats_get_counter(stats_codprcb,i);
                total +=util_stats_get_counter(stats_codprdd,i);
                total +=util_stats_get_counter(stats_codprfd,i);
                total +=util_stats_get_counter(stats_codprddcb,i);
                total +=util_stats_get_counter(stats_codprfdcb,i);
        }


	return total;

}


#endif








//Obtiene la extension de filename y la guarda en extension. Extension sin el punto
//filename debe ser nombre de archivo, sin incluir directorio
void util_get_file_extension(char *filename,char *extension)
{

        //obtener extension del nombre
        //buscar ultimo punto
        //parar si se encuentra / o \ de division de carpeta

        char caracter_carpeta='/';
#ifdef MINGW
        caracter_carpeta='\\';
#endif


        int j;
        j=strlen(filename);
        if (j==0) extension[0]=0;
        else {
                for (;j>=0 && filename[j]!='.' && filename[j]!=caracter_carpeta;j--);

                if (filename[j]==caracter_carpeta) {
                        extension[0]=0; //no hay extension
                }

                else {
                        if (j>=0) strcpy(extension,&filename[j+1]);
                        else extension[0]=0;
                }
        }

        debug_printf (VERBOSE_DEBUG,"Filename: [%s] Extension: [%s]",filename,extension);
}

//Obtiene el nombre de filename sin extension y la guarda en filename_without_extension.
//filename debe ser nombre de archivo, sin incluir directorio
//TODO: busca hasta el primer punto. quiza deberia tener en cuenta el punto desde el final... o no..
void util_get_file_without_extension(char *filename,char *filename_without_extension)
{

	int i;

	for (i=0;filename[i]!=0 && filename[i]!='.';i++) {
		filename_without_extension[i]=filename[i];
	}

	filename_without_extension[i]=0;

        //printf ("Filename: %s without Extension: %s\n",filename,filename_without_extension);
}



//Obtiene el nombre y extension (quitamos el directorio)
void util_get_file_no_directory(char *filename,char *file_no_dir)
{
	//buscamos / o barra invertida windows

        int j;
        j=strlen(filename);
        if (j==0) file_no_dir[0]=0;
	else {
		for (;j>=0 && filename[j]!='/' && filename[j]!='\\' ;j--);

		strcpy(file_no_dir,&filename[j+1]);
	}
}

//Compara la extension indicada con la del archivo, sin distinguir mayusculas
//Devuelve valor de strcasecmp -> 0 igual, otros->diferente
//Extension sin el punto
int util_compare_file_extension(char *filename,char *extension_compare)
{
	char extension[NAME_MAX];

	util_get_file_extension(filename,extension);

	return strcasecmp(extension,extension_compare);
}



//Retorna el directorio de una ruta completa, ignorando barras repetidas del final
void util_get_dir(char *ruta,char *directorio)
{
        int i;

        if (ruta==NULL) {
                debug_printf (VERBOSE_DEBUG,"ruta NULL");
                directorio[0]=0;
                return;
        }

	//buscar barra final / (o \ de windows)
        for (i=strlen(ruta)-1;i>=0;i--) {
                if (ruta[i]=='/' || ruta[i]=='\\') break;
        }

        //Ubicarse en el primer caracter no /
        for (;i>=0;i--) {
                if (ruta[i]!='/' && ruta[i]!='\\') break;
        }

        if (i>=0) {

                //TODO. porque esto no va con strcpy??? Da segmentation fault en mac, en cintas de revenge.p
                int j;
                for (j=0;j<=i;j++) {
                        directorio[j]=ruta[j];
                }
                //sleep(1);
                //printf ("despues de strncpy\n");
                directorio[i+1]='/';
                directorio[i+2]=0;
                //printf ("directorio final: %s\n",directorio);
        }

        else {
                //printf ("directorio vacio\n");
                //directorio vacio
                directorio[0]=0;
        }

}

//Compone path completo a un archivo teniendo el directorio y nombre
//tiene en cuenta si directorio es vacio
//lo mete en fullpath
void util_get_complete_path(char *dir,char *name,char *fullpath)
{
	if (dir[0]==0) {
		//directorio nulo
		sprintf (fullpath,"%s",name);
	}
	else {
		sprintf (fullpath,"%s/%s",dir,name);
	}
}

void util_press_menu_symshift(int pressrelease)
{
        if (pressrelease) menu_symshift.v=1;
        else menu_symshift.v=0;
}


//En spectrum, desactiva symbol shift. En zx80/81, desactiva mayusculas
void clear_symshift(void)
{
        if (MACHINE_IS_ZX8081) puerto_65278 |=1;
        else puerto_32766 |=2;

        //Desactivamos tecla symshift para menu
        menu_symshift.v=0;

}


//En spectrum, activa symbol shift. En zx80/81, activa mayusculas
void set_symshift(void)
{
//puerto_65278   db    255  ; V    C    X    Z    Sh    ;0
//puerto_32766    db              255  ; B    N    M    Simb Space ;7

        if (MACHINE_IS_ZX8081) puerto_65278  &=255-1;
        else puerto_32766 &=255-2;

        //Activamos tecla symshift para menu
        menu_symshift.v=1;

}

void set_shift_z88(void)
{
	//A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
	blink_kbd_a14 &= 255-64;
}

void clear_shift_z88(void)
{
        //A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
        blink_kbd_a14 |= 64;
}




//Abre un archivo en solo lectura buscandolo en las rutas:
//1) ruta actual
//2) ../Resources/
//3) INSTALLPREFIX/
//normalmente usado para cargar roms
//modifica puntero FILE adecuadamente
void old_open_sharedfile(char *archivo,FILE **f)
{
        char buffer_nombre[1024];
	strcpy(buffer_nombre,archivo);

        //ruta actual
        debug_printf(VERBOSE_INFO,"Looking for file %s at current dir",buffer_nombre);
        *f=fopen(buffer_nombre,"rb");

        //sino, en ../Resources
        if (!(*f)) {
                sprintf(buffer_nombre,"../Resources/%s",archivo);
                debug_printf(VERBOSE_INFO,"Looking for file %s",buffer_nombre);
                *f=fopen(buffer_nombre,"rb");

                //sino, en INSTALLPREFIX/share/zesarux
                if (!(*f)) {
                        sprintf(buffer_nombre,"%s/%s/%s",INSTALL_PREFIX,"/share/zesarux/",archivo);
                        debug_printf(VERBOSE_INFO,"Looking for file %s",buffer_nombre);
                        *f=fopen(buffer_nombre,"rb");
                }
        }


}


//Busca un archivo o carpeta buscandolo en las rutas:
//1) ruta actual
//2) ../Resources/
//3) INSTALLPREFIX/
//normalmente usado para cargar roms
//Retorna ruta archivo en ruta_final (siempre que no sea NULL), valor retorno no 0. Si no existe, valor retorno 0
int find_sharedfile(char *archivo,char *ruta_final)
{
        char buffer_nombre[PATH_MAX];
	strcpy(buffer_nombre,archivo);

	int existe;

        //ruta actual
        debug_printf(VERBOSE_INFO,"Looking for file %s from current dir",buffer_nombre);
        existe=si_existe_archivo(buffer_nombre);

        //sino, en ../Resources
        if (!existe) {
                sprintf(buffer_nombre,"../Resources/%s",archivo);
                debug_printf(VERBOSE_INFO,"Looking for file %s",buffer_nombre);
                existe=si_existe_archivo(buffer_nombre);

                //sino, en INSTALLPREFIX/share/zesarux
                if (!existe) {
                        sprintf(buffer_nombre,"%s/%s/%s",INSTALL_PREFIX,"/share/zesarux/",archivo);
                        debug_printf(VERBOSE_INFO,"Looking for file %s",buffer_nombre);
                        existe=si_existe_archivo(buffer_nombre);
                }
        }

	if (existe) {
		if (ruta_final!=NULL) strcpy(ruta_final,buffer_nombre);
		debug_printf(VERBOSE_INFO,"Found on path %s",buffer_nombre);
	}

	return existe;
}


//Abre un archivo en solo lectura buscandolo en las rutas:
//1) ruta actual
//2) ../Resources/
//3) INSTALLPREFIX/
//normalmente usado para cargar roms
//modifica puntero FILE adecuadamente
void open_sharedfile(char *archivo,FILE **f)
{
        char buffer_nombre[PATH_MAX];

	int existe=find_sharedfile(archivo,buffer_nombre);
	if (existe) {
		*f=fopen(buffer_nombre,"rb");
	}

	else *f=NULL;

}


//Devuelve puntero a archivo si ha podido encontrar el archivo y abrirlo en escritura.
//NULL
void open_sharedfile_write_open(char *archivo,FILE **f)
{
	//Asumimos NULL
	*f=NULL;

	if (!si_existe_archivo(archivo)) return;

	*f=fopen(archivo,"wb");

}

//Abre un archivo en escritura buscandolo en las rutas:
//1) ruta actual
//2) ../Resources/
//3) INSTALLPREFIX/
//normalmente usado para cargar roms
//modifica puntero FILE adecuadamente
//Primero mira que el archivo exista y luego intenta abrirlo para escritura
void open_sharedfile_write(char *archivo,FILE **f)
{
        char buffer_nombre[1024];

        //ruta actual
        debug_printf(VERBOSE_INFO,"Looking for file %s at current dir",archivo);
        open_sharedfile_write_open(archivo,f);

        //sino, en ../Resources
        if (!(*f)) {
                sprintf(buffer_nombre,"../Resources/%s",archivo);
                debug_printf(VERBOSE_INFO,"Looking for file %s",buffer_nombre);
                open_sharedfile_write_open(buffer_nombre,f);

                //sino, en INSTALLPREFIX/share/zesarux
                if (!(*f)) {
                        sprintf(buffer_nombre,"%s/%s/%s",INSTALL_PREFIX,"/share/zesarux/",archivo);
                        debug_printf(VERBOSE_INFO,"Looking for file %s",buffer_nombre);
                        open_sharedfile_write_open(buffer_nombre,f);
                }
        }

	//printf ("file: %d\n",*f);

}

void get_compile_info(char *s)
{
        sprintf (s,"%s",
        "Compilation date: " COMPILATION_DATE "\n"
	"\n"
	"Compilation system: " COMPILATION_SYSTEM "\n"
	"\n"
	"Compilation system release: " COMPILATION_SYSTEM_RELEASE "\n"
	"\n"
        "Configure command: " CONFIGURE_OPTIONS "\n"
	"\n"
        "Compile variables: " COMPILE_VARIABLES "\n"
	"\n"
        "Compile INITIALCFLAGS: " COMPILE_INITIALCFLAGS "\n"
        "Compile INITIALLDFLAGS: " COMPILE_INITIALLDFLAGS "\n"
        "Compile FINALCFLAGS: " COMPILE_FINALCFLAGS "\n"
        "Compile FINALLDFLAGS: " COMPILE_FINALLDFLAGS "\n"
	"\n"
        "Install PREFIX: " INSTALL_PREFIX "\n"
        );

	if (strlen(s)>=MAX_COMPILE_INFO_LENGTH) cpu_panic("Error. MAX_COMPILE_INFO_LENGTH reached");
}

void show_compile_info(void)
{
        char buffer[MAX_COMPILE_INFO_LENGTH];
        get_compile_info(buffer);
        printf ("%s",buffer);
}


//Segun tecla de entrada, genera puerto de teclado, lo pone o lo borra
void ascii_to_keyboard_port_set_clear(unsigned tecla,int pressrelease)
{

	//printf ("ascii_to_keyboard_port_set_clear: tecla: %d pressrelease: %d\n",tecla,pressrelease);

    if (tecla>='A' && tecla<='Z') {
					if (MACHINE_IS_SPECTRUM || MACHINE_IS_ACE) {
                                        	//mayus. para Spectrum
						if (pressrelease) {
	                                        	puerto_65278 &=255-1;
						}
						else {
							puerto_65278 |=1;
						}
					}

					if (MACHINE_IS_Z88) {
						//mayus. para Z88
						//A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
						if (pressrelease) {
							blink_kbd_a14 &=255-64;
						}
						else {
							blink_kbd_a14 |=64;
						}
					}

                                        //mayus para MSX
					if (MACHINE_IS_MSX) {
						if (pressrelease) {
							msx_keyboard_table[6] &=255-1;
						}
						else {
							msx_keyboard_table[6] |=1;
						}
					}

					if (MACHINE_IS_SVI) {
						if (pressrelease) {
							svi_keyboard_table[6] &=255-1;
						}
						else {
							svi_keyboard_table[6] |=1;
						}
					}

					if (MACHINE_IS_CPC) {
						if (pressrelease) {
							cpc_keyboard_table[2] &=255-32;
						}
						else {
							cpc_keyboard_table[2] |=32;
						}
					}

					if (MACHINE_IS_PCW) {
						if (pressrelease) {
                            pcw_keyboard_table[2] &=255-32;

						}
						else {
                            pcw_keyboard_table[2] |=32;
						}
					}

       	                                tecla=tecla+('a'-'A');
    }


        //printf ("Tecla buena: %d  \n",c);
	switch(tecla) {

                                case 27:
                                        //printf ("Alt\n");
                                break;

				case 32:
                                	if (pressrelease) {
						puerto_32766 &=255-1;
                                        	blink_kbd_a13 &= (255-64);
						cpc_keyboard_table[5] &= (255-128);
                                                msx_keyboard_table[8] &= (255-1);
                                                svi_keyboard_table[8] &= (255-1);
					}
	                                else {
						puerto_32766 |=1;
                                        	blink_kbd_a13 |= 64;
						cpc_keyboard_table[5] |= 128;
                                                msx_keyboard_table[8] |= 1;
                                                svi_keyboard_table[8] |= 1;
					}

        	                break;


	                        case 13:
	                        case 10:
        	                        if (pressrelease) {
						puerto_49150 &=255-1;
						blink_kbd_a8 &= (255-64);
						//Enter "grande" del cpc
						cpc_keyboard_table[2] &= (255-4);
                                                msx_keyboard_table[7] &= (255-128);
                                                svi_keyboard_table[6] &= (255-64);
					}

                	                else {
						puerto_49150 |=1;
						blink_kbd_a8 |= 64;
						cpc_keyboard_table[2] |= 4;
                                                msx_keyboard_table[7] |= 128;
                                                svi_keyboard_table[6] |= 64;
					}


                        	break;

                                default:
					convert_numeros_letras_puerto_teclado(tecla,pressrelease);
                                break;
	}


	//simbolos para ZX Spectrum
	if (MACHINE_IS_SPECTRUM) {
		switch (tecla) {
                                case '!':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-1;
					}
					else {
						clear_symshift();
						puerto_63486 |=1;
					}
                                break;

                                //Enviar caps shift con 128
                                case 128:
                                        if (pressrelease) {
						//mayus
						puerto_65278 &=255-1;
                                        }
                                        else {
						//mayus
						puerto_65278 |=1;
                                        }
				break;

                                //Enviar symbol shift con 129
                                case 129:
                                        if (pressrelease) {
						set_symshift();
                                        }
                                        else {
						clear_symshift();
                                        }
				break;

				//Enviar Shift + 1 (edit).
				/*
				En curses va bien
				Es stdout habria que hacer el | , luego el texto y enter, todo a la vez, en la misma linea,
				porque si se hace | y enter, se edita la linea y al dar enter se entra la linea sin modificar
				*/
				case '|':
                                        if (pressrelease) {
						//mayus
						puerto_65278 &=255-1;
                                                puerto_63486 &=255-1;
                                        }
                                        else {
						//mayus
						puerto_65278 |=1;
                                                puerto_63486 |=1;
                                        }
				break;


                                //Symbol + 2
                                case '@':
                                        if (pressrelease) {
						set_symshift();
	                                        puerto_63486 &=255-2;
					}
					else {
						clear_symshift();
	                                        puerto_63486 |=2;
					}
                                break;

                                 //Symbol + 3
                                case '#':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_63486 |=4;
					}

                                break;

                                case '$':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_63486 |=8;
					}
                                break;


                                case '%':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_63486 |=16;
					}
                                break;

                                case '&':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=16;
					}
                                break;

                                //Comilla simple
                                case '\'':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=8;
					}
                                break;

                                case '(':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=4;
					}
                                break;

                                case ')':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=2;
					}
                                break;

                                case '_':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-1;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=1;
					}
                                break;

                                case '*':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_32766 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_32766 |=16;
					}
                                break;

                                case '?':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=8;
					}
                                break;

                                //Esto equivale al simbolo de "flecha" arriba - exponente
                                case '^':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=16;
					}
                                break;

                                case '-':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=8;
					}
                                break;


                                case '+':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=4;
					}
                                break;

                                case '=':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=2;
					}
                                break;

                                case ';':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=2;
					}
                                break;

                                //Comilla doble
                                case '"':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-1;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=1;
					}
                                break;

                                case '<':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_64510 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_64510 |=8;
					}
                                break;

                                case '>':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_64510 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_64510 |=16;
					}
                                break;

                                case '/':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=16;
					}
                                break;

                                case ':':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=2;
					}
                                break;

                                case '.':
					if (pressrelease) {
						puerto_32766 &=255-2-4;
					}
					else {
						puerto_32766 |=2+4;
					}

                                break;

                                case ',':
					if (pressrelease) {
	                                        puerto_32766 &=255-2-8;
					}
					else {
	                                        puerto_32766 |=2+8;
					}
				break;


		}
	}

	//simbolos para Jupiter Ace
	else if (MACHINE_IS_ACE) {
		switch (tecla) {
                                case '!':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-1;
					}
					else {
						clear_symshift();
						puerto_63486 |=1;
					}
                                break;



                                case '@':
                                        if (pressrelease) {
						set_symshift();
	                                        puerto_63486 &=255-2;
					}
					else {
						clear_symshift();
	                                        puerto_63486 |=2;
					}
                                break;

                                case '#':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_63486 |=4;
					}

                                break;

                                case '$':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_63486 |=8;
					}
                                break;


                                case '%':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_63486 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_63486 |=16;
					}
                                break;

                                case '&':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=16;
					}
                                break;

                                //Comilla simple
                                case '\'':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=8;
					}
                                break;

                                case '(':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=4;
					}
                                break;

                                case ')':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=2;
					}
                                break;

                                case '_':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_61438 &=255-1;
					}
					else {
	                                        clear_symshift();
        	                                puerto_61438 |=1;
					}
                                break;

                                case '*':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_32766 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_32766 |=16;
					}
                                break;

                                case '?':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=8;
					}
                                break;

				case '~':
					if (pressrelease) {
                                                set_symshift();
                                                puerto_65022 &=255-1;
                                        }
                                        else {
                                                clear_symshift();
                                                puerto_65022 |=1;
                                        }
                                break;

                                case '|':
                                        if (pressrelease) {
                                                set_symshift();
                                                puerto_65022 &=255-2;
                                        }
                                        else {
                                                clear_symshift();
                                                puerto_65022 |=2;
                                        }
                                break;

                                case '\\':
                                        if (pressrelease) {
                                                set_symshift();
                                                puerto_65022 &=255-4;
                                        }
                                        else {
                                                clear_symshift();
                                                puerto_65022 |=4;
                                        }
                                break;

                                case '{':
                                        if (pressrelease) {
                                                set_symshift();
                                                puerto_65022 &=255-8;
                                        }
                                        else {
                                                clear_symshift();
                                                puerto_65022 |=8;
                                        }
                                break;

                                case '}':
                                        if (pressrelease) {
                                                set_symshift();
                                                puerto_65022 &=255-16;
                                        }
                                        else {
                                                clear_symshift();
                                                puerto_65022 |=16;
                                        }
                                break;


                                //Esto equivale al simbolo de "flecha" arriba - exponente
                                case '^':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=16;
					}
                                break;

                                case '-':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=8;
					}
                                break;


                                case '+':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=4;
					}
                                break;

                                case '=':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=2;
					}
                                break;

                                case '[':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=16;
					}
                                break;

                                case ']':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=8;
					}
                                break;


				/* Simbolo copyright (C) . TODO
                                case '(C)':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=4;
					}
                                break;
				*/

                                case ';':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=2;
					}
                                break;

                                //Comilla doble
                                case '"':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-1;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=1;
					}
                                break;

                                case '<':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_64510 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_64510 |=8;
					}
                                break;

                                case '>':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_64510 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_64510 |=16;
					}
                                break;

                                case '/':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=16;
					}
                                break;

                                case ':':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=2;
					}
                                break;

                                case '.':
					if (pressrelease) {
						puerto_32766 &=255-2-4;
					}
					else {
						puerto_32766 |=2+4;
					}

                                break;

                                case ',':
					if (pressrelease) {
	                                        puerto_32766 &=255-2-8;
					}
					else {
	                                        puerto_32766 |=2+8;
					}
				break;


		}

	}


	//simbolos para ZX80
	else if (MACHINE_IS_ZX80_TYPE) {
                switch (tecla) {
                                case '$':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=8;
					}
                                break;


                                case '(':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=4;
					}
                                break;

                                case ')':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=2;
					}
                                break;

                                case '*':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-1;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=1;
					}
                                break;

                                case '?':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=8;
					}

                                break;

                                //Esto equivale al simbolo de ** - exponente
                                case '^':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=16;
					}
                                break;

                                case '-':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=8;
					}
                                break;

                                case '+':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=4;
					}
                                break;

                                case '=':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=2;
					}
                                break;

                                case ';':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=4;
					}
                                break;

                                //Comilla doble
                                case '"':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=16;
					}
                                break;

                                case '<':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_32766 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_32766 |=8;
					}
                                break;

                                case '>':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_32766 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_32766 |=4;
					}
                                break;

                                case '/':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=16;
					}
                                break;

                                case ':':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=2;
					}
                                break;

                                case '.':
					if (pressrelease) {
						puerto_32766 &=255-2;
					}
					else {
						puerto_32766 |=2;
					}
                                break;

                                case ',':
                                        if (pressrelease) {
						set_symshift();
						puerto_32766 &=255-2;
					}
					else {
						clear_symshift();
						puerto_32766 |=2;
					}

                                break;

                }
        }

        //simbolos para ZX81
        else if (MACHINE_IS_ZX81_TYPE) {
                switch (tecla) {

                                case '$':
					if (pressrelease) {
	                                        set_symshift();
						puerto_57342 &=255-8;
					}
					else {
	                                        clear_symshift();
						puerto_57342 |=8;
					}
                                break;


                                case '(':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=4;
					}
                                break;

                                case ')':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=2;
					}
                                break;

                                case '*':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_32766 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_32766 |=16;
					}
                                break;

                                case '?':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=8;
					}
                                break;

                                //Esto equivale al simbolo de ** - exponente
                                case '^':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=16;
					}
                                break;

                                case '-':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=8;
					}
                                break;

                                case '+':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=4;
					}
                                break;

                                case '=':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_49150 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_49150 |=2;
					}
                                break;

                                case ';':
					if (pressrelease) {
	                                        set_symshift();
	                                        puerto_65278 &=255-4;
					}
					else {
	                                        clear_symshift();
	                                        puerto_65278 |=4;
					}
                                break;

                                //Comilla doble
                                case '"':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_57342 &=255-1;
					}
					else {
	                                        clear_symshift();
        	                                puerto_57342 |=1;
					}
                                break;

                                case '<':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_32766 &=255-8;
					}
					else {
	                                        clear_symshift();
        	                                puerto_32766 |=8;
					}
                                break;

                                case '>':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_32766 &=255-4;
					}
					else {
	                                        clear_symshift();
        	                                puerto_32766 |=4;
					}
                                break;

                                case '/':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-16;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=16;
					}
                                break;

                                case ':':
					if (pressrelease) {
	                                        set_symshift();
        	                                puerto_65278 &=255-2;
					}
					else {
	                                        clear_symshift();
        	                                puerto_65278 |=2;
					}
                                break;

                                case '.':
					if (pressrelease) {
	                                        puerto_32766 &=255-2;
					}
					else {
	                                        puerto_32766 |=2;
					}

                                break;

                                case ',':
					if (pressrelease) {
	                                        set_symshift();
						puerto_32766 &=255-2;
					}
					else {
	                                        clear_symshift();
	                                        puerto_32766 |=2;
					}
				break;

                }
        }

        if (MACHINE_IS_Z88) {
                switch (tecla) {
                                case '!':
                                        if (pressrelease) {
						set_shift_z88();
						//A13 (#5) [      SPACE   1       Q       A       Z       L       0
						blink_kbd_a13 &=255-32;
                                        }
                                        else {
						clear_shift_z88();
						blink_kbd_a13 |=32;
                                        }
                                break;

				case '@':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a12 &=255-32;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a12 |=32;
                                        }
                                break;

                                case '#':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a11 &=255-32;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a11 |=32;
                                        }
                                break;

                                case '$':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a10 &=255-32;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a10 |=32;
                                        }
                                break;

                                case '%':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a9 &=255-32;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a9 |=32;
                                        }
                                break;

                                case '^':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a8 &=255-32;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a8 |=32;
                                        }
                                break;

				//A8  (#0) | DEL    ENTER   6       Y       H       N       7       8
                                case '&':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a8 &=255-2;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a8 |=2;
                                        }
                                break;

                                case '*':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a8 &=255-1;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a8 |=1;
                                        }
                                break;

				//A11 (#3) | -      RGT     3       E       D       C       K       9
                                case '(':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a11 &=255-1;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a11 |=1;
                                        }
                                break;

				//A13 (#5) | [      SPACE   1       Q       A       Z       L       0
                                case ')':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a13 &=255-1;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a13 |=1;
                                        }
                                break;

				//A11 (#3) | -      RGT     3       E       D       C       K       9
				//A10 (#2) | =      DWN     4       R       F       V       J       O
				//A9  (#1) | \      UP      5       T       G       B       U       I
                                case '-':
                                        if (pressrelease) {
                                                blink_kbd_a11 &=255-128;
                                        }
                                        else {
                                                blink_kbd_a11 |=128;
                                        }
                                break;


                                case '_':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a11 &=255-128;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a11 |=128;
                                        }
                                break;

                                case '=':
                                        if (pressrelease) {
                                                blink_kbd_a10 &=255-128;
                                        }
                                        else {
                                                blink_kbd_a10 |=128;
                                        }
                                break;


                                case '+':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a10 &=255-128;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a10 |=128;
                                        }
                                break;

                                case '\\':
                                        if (pressrelease) {
                                                blink_kbd_a9 &=255-128;
                                        }
                                        else {
                                                blink_kbd_a9 |=128;
                                        }
                                break;


                                case '|':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a9 &=255-128;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a9 |=128;
                                        }
                                break;

				//A13 (#5) | [      SPACE   1       Q       A       Z       L       0
				//A12 (#4) | ]      LFT     2       W       S       X       M
                                case '[':
                                        if (pressrelease) {
                                                blink_kbd_a13 &=255-128;
                                        }
                                        else {
                                                blink_kbd_a13 |=128;
                                        }
                                break;


                                case '{':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a13 &=255-128;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a13 |=128;
                                        }
                                break;

                                case ']':
                                        if (pressrelease) {
                                                blink_kbd_a12 &=255-128;
                                        }
                                        else {
                                                blink_kbd_a12 |=128;
                                        }
                                break;


                                case '}':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a12 &=255-128;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a12 |=128;
                                        }
                                break;


				//A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
                                case ';':
                                        if (pressrelease) {
                                                blink_kbd_a14 &=255-2;
                                        }
                                        else {
                                                blink_kbd_a14 |=2;
                                        }
                                break;


                                case ':':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a14 &=255-2;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a14 |=2;
                                        }
                                break;

                                //A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
                                case '\'':
                                        if (pressrelease) {
                                                blink_kbd_a14 &=255-1;
                                        }
                                        else {
                                                blink_kbd_a14 |=1;
                                        }
                                break;


                                case '"':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a14 &=255-1;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a14 |=1;
                                        }
                                break;



				//A15 (#7) | RSH    SQR     ESC     INDEX   CAPS    .       /       £
				//TODO Libra se envia como ...?
				/*
                                case 'XXXX':
                                        if (pressrelease) {
                                                blink_kbd_a15 &=255-1;
                                        }
                                        else {
                                                blink_kbd_a15 |=1;
                                        }
                                break;
				*/


                                case '~':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a15 &=255-1;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a15 |=1;
                                        }
                                break;


                                //A14 (#6) | HELP   LSH     TAB     DIA     MENU    ,       ;       '
                                case ',':
                                        if (pressrelease) {
                                                blink_kbd_a14 &=255-4;
                                        }
                                        else {
                                                blink_kbd_a14 |=4;
                                        }
                                break;


                                case '<':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a14 &=255-4;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a14 |=4;
                                        }
                                break;

				//A15 (#7) | RSH    SQR     ESC     INDEX   CAPS    .       /       £
                                case '.':
                                        if (pressrelease) {
                                                blink_kbd_a15 &=255-4;
                                        }
                                        else {
                                                blink_kbd_a15 |=4;
                                        }
                                break;


                                case '>':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a15 &=255-4;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a15 |=4;
                                        }
                                break;

                                //A15 (#7) | RSH    SQR     ESC     INDEX   CAPS    .       /       £
                                case '/':
                                        if (pressrelease) {
                                                blink_kbd_a15 &=255-2;
                                        }
                                        else {
                                                blink_kbd_a15 |=2;
                                        }
                                break;


                                case '?':
                                        if (pressrelease) {
                                                set_shift_z88();
                                                blink_kbd_a15 &=255-2;
                                        }
                                        else {
                                                clear_shift_z88();
                                                blink_kbd_a15 |=2;
                                        }
                                break;



		}
	}



}

//Segun tecla de entrada, genera puerto de teclado, lo activa
void ascii_to_keyboard_port(unsigned tecla)
{
	ascii_to_keyboard_port_set_clear(tecla,1);
}

int input_file_keyboard_is_playing(void)
{
        if (input_file_keyboard_inserted.v && input_file_keyboard_playing.v) return 1;
        else return 0;
}

void insert_input_file_keyboard(void)
{
	input_file_keyboard_inserted.v=1;
        input_file_keyboard_playing.v=0;
}

void eject_input_file_keyboard(void)
{
        input_file_keyboard_inserted.v=0;
        input_file_keyboard_playing.v=0;

        //Si modo turbo, quitar
        if (input_file_keyboard_turbo.v) {
                reset_peek_byte_function_spoolturbo();
                input_file_keyboard_turbo.v=0;
        }
}

int input_file_keyboard_init(void)
{
	ptr_input_file_keyboard=fopen(input_file_keyboard_name,"rb");

	if (!ptr_input_file_keyboard) {
		debug_printf(VERBOSE_ERR,"Unable to open keyboard input file %s",input_file_keyboard_name);
		eject_input_file_keyboard();
		return 1;
	}

	insert_input_file_keyboard();

	return 0;

}

void input_file_keyboard_close(void)
{
	eject_input_file_keyboard();
	fclose(ptr_input_file_keyboard);
}


void reset_keyboard_ports(void)
{
        //inicializar todas las teclas a nada - 255
        puerto_65278=255;
        puerto_65022=255;
        puerto_64510=255;
        puerto_63486=255;
        puerto_61438=255;
        puerto_57342=255;
        puerto_49150=255;
        puerto_32766=255;
        puerto_especial1=255;
        puerto_especial2=255;
        puerto_especial3=255;
        puerto_especial4=255;

        puerto_especial_joystick=0;

        //De Sam coupe

        puerto_65534=255;

        puerto_teclado_sam_fef9=255;
        puerto_teclado_sam_fdf9=255;
        puerto_teclado_sam_fbf9=255;
        puerto_teclado_sam_f7f9=255;
        puerto_teclado_sam_eff9=255;
        puerto_teclado_sam_dff9=255;
        puerto_teclado_sam_bff9=255;
        puerto_teclado_sam_7ff9=255;

	//De Z88
	blink_kbd_a15=255;
	blink_kbd_a14=255;
	blink_kbd_a13=255;
	blink_kbd_a12=255;
	blink_kbd_a11=255;
	blink_kbd_a10=255;
	blink_kbd_a9=255;
	blink_kbd_a8=255;

	//De CPC
	int i=0;
	for (i=0;i<16;i++) cpc_keyboard_table[i]=255;

	//De MSX
	for (i=0;i<16;i++) msx_keyboard_table[i]=255;

	//De Spectravideo
	for (i=0;i<16;i++) svi_keyboard_table[i]=255;

	//De QL
	for (i=0;i<8;i++) ql_keyboard_table[i]=255;

    //De pcw
    for (i=0;i<16;i++) pcw_keyboard_table[i]=255;

        menu_symshift.v=0;
        menu_capshift.v=0;
        menu_backspace.v=0;
        menu_tab.v=0;

    //De MK14
	for (i=0;i<MK14_DIGITS;i++) {
        mk14_keystatus[i] = 0xFF;                            // Keys not pressed
    }


}


void input_file_keyboard_get_key(void)
{
                        if (input_file_keyboard_pending_next.v==1) {
                                input_file_keyboard_pending_next.v=0;
                                //leer siguiente tecla o enviar pausa (nada)
				if (input_file_keyboard_send_pause.v==1) {
					if (input_file_keyboard_is_pause.v==1) {
						reset_keyboard_ports();
						return;
					}
				}

                                int leidos=fread(&input_file_keyboard_last_key,1,1,ptr_input_file_keyboard);
				if (leidos==0) {
					debug_printf (VERBOSE_INFO,"Read 0 bytes of Input File Keyboard. End of file");
					eject_input_file_keyboard();
					reset_keyboard_ports();
					return;
				}

                        }


                        //Puertos a 0
                        reset_keyboard_ports();
                        //solo habilitar tecla indicada
			if (input_file_keyboard_send_pause.v==0) ascii_to_keyboard_port(input_file_keyboard_last_key);

			//Se envia pausa. Ver si ahora es pausa o tecla
			else {
				if (input_file_keyboard_is_pause.v==0) ascii_to_keyboard_port(input_file_keyboard_last_key);
			}


}

//Devuelve tamanyo minimo de ROM para el tipo de maquina indicado
int get_rom_size(int machine)
{
    //Por defecto
    int rom_size=16384;

    //Truco. Utilizo macro de MACHINE_IS_ porque me facilita mucho la vida

    //Guardo antes valor de current_machine_type

    z80_byte original_current_machine_type=current_machine_type;

    current_machine_type=machine;


    if (MACHINE_IS_SPECTRUM_16_48) rom_size=16384;

    else if (MACHINE_IS_SPECTRUM_128_P2) rom_size=32768;

    else if (MACHINE_IS_SPECTRUM_P2A_P3) rom_size=65536;

    else if (MACHINE_IS_ZXUNO) rom_size=214;

    else if (MACHINE_IS_CHLOE) rom_size=32768;

    else if (MACHINE_IS_PRISM)  rom_size=320*1024;

    else if (MACHINE_IS_TBBLUE) rom_size=8192;

    else if (MACHINE_IS_CHROME)  rom_size=65536;

    else if (MACHINE_IS_TSCONF)  rom_size=512*1024;

    else if (MACHINE_IS_BASECONF)  rom_size=512*1024;

    else if (MACHINE_IS_TIMEX_TS_TC_2068)  rom_size=24576;

    else if (MACHINE_IS_COLECO)  rom_size=8192;

    //else if (MACHINE_IS_SG1000) rom_size=

    else if (MACHINE_IS_SMS)  rom_size=8192;

    else if (MACHINE_IS_MSX1) rom_size=32768;

    else if (MACHINE_IS_SVI)  rom_size=32768;

    else if (MACHINE_IS_ZX80_TYPE) rom_size=4096;

    else if (MACHINE_IS_MICRODIGITAL_TK85) rom_size=10*1024;

    else if (MACHINE_IS_ZX81_TYPE)  rom_size=8192;

    else if (MACHINE_IS_ACE) rom_size=8192;

    else if (MACHINE_IS_Z88) rom_size=131072;

    else if (MACHINE_IS_CPC_464 || MACHINE_IS_CPC_4128) rom_size=32768;

    else if (MACHINE_IS_CPC_6128 || MACHINE_IS_CPC_664) rom_size=49152;

    else if (MACHINE_IS_SAM) rom_size=32768;

    else if (MACHINE_IS_QL) rom_size=49152;

    else if (MACHINE_IS_MK14)  rom_size=512;

    else if (MACHINE_IS_PCW_8256 || MACHINE_IS_PCW_8512) rom_size=275;


    //Restaurar current_machine_type
    current_machine_type=original_current_machine_type;

    return rom_size;

}


//Retorna la cantidad de ram para la maquina en curso
//En principio esto solo es a caracter informativo, no se llama desde ninguna funcion importante
int get_ram_size(void)
{
    //Memoria
    //Por defecto
    int total_ram=48*1024;

    //TODO:sumar posibles extensiones de memoria en spectrum
    if (MACHINE_IS_MK14) {
        total_ram=256;
    }

    else if (MACHINE_IS_COLECO || MACHINE_IS_SG1000) {
        total_ram=1024;
    }

    else if (MACHINE_IS_SMS) {
        total_ram=8192;
    }

    else if (MACHINE_IS_INVES) {
        total_ram=65536;
    }

    else if (MACHINE_IS_SPECTRUM_16 || MACHINE_IS_SVI_318) {
        total_ram=16*1024;
    }

    else if (MACHINE_IS_CPC_HAS_64K) {
        total_ram=64*1024;
    }

    else if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_QL || MACHINE_IS_CPC_HAS_128K || MACHINE_IS_CHLOE_140SE) {
        total_ram=128*1024;
    }

    else if (MACHINE_IS_PCW) {
        total_ram=pcw_total_ram;
    }

    else if (MACHINE_IS_SVI_328) {
        //Total:  3 ROMS de 32 kb, 5 RAMS de 32 kb, en SVI328.
        //En 318, solo 1 pagina de 16 kb ram?
        total_ram=5*32*1024;
    }

    //TODO: posibles cartuchos de RAM?
    else if (MACHINE_IS_MSX) {
        total_ram=128*1024;
    }

    else if (MACHINE_IS_CHROME) {
        total_ram=160*1024;
    }

    else if (MACHINE_IS_CHLOE_280SE) {
        total_ram=256*1024;
    }

    else if (MACHINE_IS_PRISM || MACHINE_IS_ZXUNO) {
        total_ram=512*1024;
    }

    else if (MACHINE_IS_TBBLUE) {
        total_ram=tbblue_get_current_ram()*1024;
    }

    else if (MACHINE_IS_ZXEVO) {
        total_ram=4*1024*1024;
    }

    else if (MACHINE_IS_SAM) {
        total_ram=get_sam_ram()*1024;
    }

    else if (MACHINE_IS_Z88) {
        total_ram=z88_get_total_ram();
    }

    else if (MACHINE_IS_ZX8081) {
        total_ram=zx8081_get_total_ram_with_rampacks()*1024;
    }

    else if (MACHINE_IS_ACE) {
        total_ram=get_ram_ace()*1024;
    }

    return total_ram;

}

//1 si ok
//0 si error
int configfile_read_aux(char *configfile,char *mem)
{

        //Avisar si tamanyo grande
        if (get_file_size(configfile) > (long long int)MAX_SIZE_CONFIG_FILE) cpu_panic("Configuration file is larger than maximum size allowed");

	FILE *ptr_configfile;
        ptr_configfile=fopen(configfile,"rb");


        if (!ptr_configfile) {
                printf("Unable to open configuration file %s\n",configfile);
                return 0;
        }

        int leidos=fread(mem,1,MAX_SIZE_CONFIG_FILE,ptr_configfile);

        //Poner un codigo 0 al final
        mem[leidos]=0;


        fclose(ptr_configfile);

        return 1;

}

//Devuelve 1 si ok
//0 si error
int util_get_configfile_name_aux(char *configfile,char *file_to_find)
{
  if (customconfigfile!=NULL) {
        sprintf(configfile,"%s",customconfigfile);
        return 1;
  }

  #ifndef MINGW
  	char *directorio_home;
  	directorio_home=getenv("HOME");
  	if ( directorio_home==NULL) {
                  //printf("Unable to find $HOME environment variable to open configuration file\n");
  		return 0;
  	}

  	sprintf(configfile,"%s/%s",directorio_home,file_to_find);

  #else
  	char *homedrive;
  	char *homepath;
  	homedrive=getenv("HOMEDRIVE");
  	homepath=getenv("HOMEPATH");

  	if (homedrive==NULL || homepath==NULL) {
  		//printf("Unable to find HOMEDRIVE or HOMEPATH environment variables to open configuration file\n");
                  return 0;
          }

  	sprintf(configfile,"%s\\%s\\%s",homedrive,homepath,file_to_find);

  #endif

  return 1;

}

int util_get_configfile_name(char *configfile)
{
  return util_get_configfile_name_aux(configfile,DEFAULT_ZESARUX_CONFIG_FILE);
}

int util_get_devconfigfile_name(char *configfile)
{
  return util_get_configfile_name_aux(configfile,DEFAULT_ZESARUX_DEVCONFIG_FILE);
}

//Devuelve 1 si ok
//0 si error
int util_create_sample_configfile(int additional)
{

char configfile[PATH_MAX];
	FILE *ptr_configfile;

if (util_get_configfile_name(configfile)==0)  {
  printf("Unable to find $HOME, or HOMEDRIVE or HOMEPATH environment variables to open configuration file\n");
  return 0;
}

  ptr_configfile=fopen(configfile,"wb");
              if (!ptr_configfile) {
                      printf("Unable to create sample configuration file %s\n",configfile);
                      return 0;
              }


    char creator_buffer[255];


    //fecha grabacion
    time_t tiempo = time(NULL);
    struct tm tm = *localtime(&tiempo);


    sprintf(creator_buffer,";ZEsarUX configuration file created from ZEsarUX %s on %02d/%02d/%04d %02d:%02d:%02d\n\n",
        EMULATOR_VERSION,tm.tm_mday,tm.tm_mon+1,tm.tm_year + 1900,tm.tm_hour,tm.tm_min,tm.tm_sec);


    fwrite(creator_buffer, 1, strlen(creator_buffer), ptr_configfile);


  char *sample_config=
    ";Lines beginning with ; or # are ignored\n"
    ";You can specify here the same options passed on command line, for example:\n"
    ";--verbose 2\n"
    ";Options can be written in quotation marks, for example:\n"
    ";--joystickemulated \"OPQA Space\"\n"
    ";Options can be written on the same line or different lines, like:\n"
    ";--verbose 2 --machine 128k\n"
    ";Or:\n"
    ";--verbose 2\n"
    ";--machine 128k\n"
    ";Or even like this:\n"
    ";--verbose\n"
    ";2\n"
    ";--machine\n"
    ";128k\n"
    "\n"
    ";Run zesarux with --help or --experthelp to see all the options\n"
    "\n"
    ;

  fwrite(sample_config, 1, strlen(sample_config), ptr_configfile);

  if (additional)  {
	char *sample_config_additional="--saveconf-on-exit"
	    "\n"
	    "\n"
	;

	fwrite(sample_config_additional, 1, strlen(sample_config_additional), ptr_configfile);
   }


              fclose(ptr_configfile);

              return 1;

}

/*int util_write_config_add_string(char *config_settings,char *string_add)
{
    int longitud=strlen(string_add)+2; //Agregar texto y salto linea
    sprintf (config_settings,"%s \n",string_add);

    return longitud;
}
*/

#define MAX_CONFIG_SETTING (PATH_MAX*2)

int util_write_config_add_string (char *config_settings, const char * format , ...)
{

	//tamaño del buffer bastante mas grande que el valor constante definido
    char string_add[MAX_CONFIG_SETTING+100];


    va_list args;
    va_start (args, format);
    vsprintf (string_add,format, args);
    va_end (args);

    int longitud=strlen(string_add)+2; //Agregar texto y salto linea
    sprintf (config_settings,"%s \n",string_add);

    return longitud;

}


//Retorna un string con valor X,+X o -X segun button_type
void util_write_config_aux_realjoystick(int button_type, int button, char *texto)
{
  		if (button_type==0) {
  			sprintf(texto,"%d",button);
  		}
  		else if (button_type<0) {
  			sprintf(texto,"-%d",button);
  		}

  		else {
  			sprintf(texto,"+%d",button);
  		}
}

void util_copy_path_delete_last_slash(char *origen, char *destino)
{
    //Quitar barras del final si las hay
    strcpy(destino,origen);
    int i=strlen(destino);
    i--;
    while (i>0 &&
        (destino[i]=='/' || destino[i]=='\\')
        ) {
            destino[i]=0;
            i--;
    }

}

//void util_save_config_normalize_path


//1 si ok
//0 si error
int util_write_configfile(void)
{
  //Sobreescribimos archivos con settings actuales
  debug_printf (VERBOSE_INFO,"Writing configuration file");

  //Creamos archivo de configuracion de ejemplo y agregamos settings aparte

  if (util_create_sample_configfile(0)==0) {
    debug_printf(VERBOSE_ERR,"Cannot write configuration file");
    return 0;
  }

  //Agregamos contenido
  //char config_settings[20000]; //Esto es mas que suficiente

  char *config_settings=util_malloc(MAX_SIZE_CONFIG_FILE+1,"Can not allocate memory for config file");

  char buffer_temp[MAX_CONFIG_SETTING]; //Para cadenas temporales
  //char buffer_temp2[MAX_CONFIG_SETTING]; //Para cadenas temporales

  int indice_string=0;

  int i;



//Macro para no repetir tantas veces lo mismo
#define ADD_STRING_CONFIG indice_string +=util_write_config_add_string(&config_settings[indice_string]

  //sprintf (buffer_temp,"--zoom %d",zoom_x);
  if (save_configuration_file_on_exit.v)      ADD_STRING_CONFIG,"--saveconf-on-exit");


    //Estos dos justo al principio para poder avisar si se ha hecho downgrade
    //Ademas el --last-version-text antes
    ADD_STRING_CONFIG,"--last-version-text \"%s\"",EMULATOR_VERSION);
    ADD_STRING_CONFIG,"--last-version \"%s\"",BUILDNUMBER);



  //TODO: por que no estamos guardando zoom_x y zoom_y??
                                              ADD_STRING_CONFIG,"--zoom %d",zoom_x);

  if (autochange_zoom_big_display.v==0)       ADD_STRING_CONFIG,"--no-autochange-zoom-big-display");

  if (frameskip)                              ADD_STRING_CONFIG,"--frameskip %d",frameskip);


  if (menu_char_width!=8)		      ADD_STRING_CONFIG,"--menucharwidth %d",menu_char_width);
  if (menu_char_height!=8)		      ADD_STRING_CONFIG,"--menucharheight %d",menu_char_height);

  if (screen_reduce_075.v)			ADD_STRING_CONFIG,"--reduce-075");
  if (screen_reduce_075_antialias.v==0)		ADD_STRING_CONFIG,"--reduce-075-no-antialias");



						ADD_STRING_CONFIG,"--reduce-075-offset-x %d",screen_reduce_offset_x);
						ADD_STRING_CONFIG,"--reduce-075-offset-y %d",screen_reduce_offset_y);

  if (screen_watermark_enabled.v)               ADD_STRING_CONFIG,"--enable-watermark");


                                                ADD_STRING_CONFIG,"--watermark-position %d",screen_watermark_position);


  if (screen_ext_desktop_enabled)             ADD_STRING_CONFIG,"--enable-zxdesktop");
                                              ADD_STRING_CONFIG,"--zxdesktop-width %d",zxdesktop_width);
                                              ADD_STRING_CONFIG,"--zxdesktop-height %d",zxdesktop_height);



                                              ADD_STRING_CONFIG,"--zxdesktop-fill-type %d",menu_ext_desktop_fill);
                                              ADD_STRING_CONFIG,"--zxdesktop-fill-primary-color %d",menu_ext_desktop_fill_first_color);
                                              ADD_STRING_CONFIG,"--zxdesktop-fill-secondary-color %d",menu_ext_desktop_fill_second_color);

    if (menu_ext_desktop_degraded_inverted.v)	  ADD_STRING_CONFIG,"--zxdesktop-fill-degraded-inverted");


    if (screen_ext_desktop_place_menu)          ADD_STRING_CONFIG,"--zxdesktop-new-items");

    if (menu_zxdesktop_upper_buttons_enabled.v==0)    ADD_STRING_CONFIG,"--zxdesktop-disable-upper-buttons");
    if (menu_zxdesktop_lower_buttons_enabled.v==0)    ADD_STRING_CONFIG,"--zxdesktop-disable-lower-buttons");


    if (menu_ext_desktop_transparent_upper_icons.v)   ADD_STRING_CONFIG,"--zxdesktop-transparent-upper-buttons");

    if (menu_ext_desktop_transparent_lower_icons.v)   ADD_STRING_CONFIG,"--zxdesktop-transparent-lower-buttons");

	if (menu_ext_desktop_transparent_configurable_icons.v==0)  ADD_STRING_CONFIG,"--zxdesktop-no-transparent-configurable-icons");

    if (menu_ext_desktop_configurable_icons_text_background.v==0)   ADD_STRING_CONFIG,"--zxdesktop-no-configurable-icons-text-bg");

    if (menu_ext_desktop_disable_box_upper_icons.v)   ADD_STRING_CONFIG,"--zxdesktop-disable-box-upper-buttons");

    if (menu_ext_desktop_disable_box_lower_icons.v)   ADD_STRING_CONFIG,"--zxdesktop-disable-box-lower-buttons");

    if (zxdesktop_switch_button_enabled.v==0)         ADD_STRING_CONFIG,"--zxdesktop-disable-footer-switch");


	if (zxdesktop_disable_on_full_screen)             ADD_STRING_CONFIG,"--zxdesktop-disable-on-fullscreen");

    if (disable_border_on_full_screen)                  ADD_STRING_CONFIG,"--disable-border-on-fullscreen");
    if (disable_footer_on_full_screen)                  ADD_STRING_CONFIG,"--disable-footer-on-fullscreen");

    if (zxdesktop_restore_windows_after_full_screen==0) ADD_STRING_CONFIG,"--zxdesktop-no-restore-win-after-fullscreen");

    if (zxdesktop_disable_show_frame_around_display)  ADD_STRING_CONFIG,"--zxdesktop-disable-frame-emulated-display");


    if (zxdesktop_draw_scrfile_enabled)                 ADD_STRING_CONFIG,"--zxdesktop-scr-enable");

    if (zxdesktop_draw_scrfile_name[0]!=0)              ADD_STRING_CONFIG,"--zxdesktop-scr-file \"%s\"",zxdesktop_draw_scrfile_name);

    if (zxdesktop_draw_scrfile_centered)                ADD_STRING_CONFIG,"--zxdesktop-scr-centered");

    if (zxdesktop_draw_scrfile_fill_scale)              ADD_STRING_CONFIG,"--zxdesktop-scr-fillscale");

    if (zxdesktop_draw_scrfile_mix_background)          ADD_STRING_CONFIG,"--zxdesktop-scr-mixbackground");

                                                        ADD_STRING_CONFIG,"--zxdesktop-scr-scalefactor %d",zxdesktop_draw_scrfile_scale_factor);

    if (zxdesktop_draw_scrfile_disable_flash)           ADD_STRING_CONFIG,"--zxdesktop-scr-disable-flash");

    if (zxdesktop_configurable_icons_enabled.v==0)      ADD_STRING_CONFIG,"--zxdesktop-disable-configurable-icons");

    if (zxdesktop_empty_trash_on_exit.v)                ADD_STRING_CONFIG,"--zxdesktop-empty-trash-on-exit");

    if (zxdesktop_icon_show_app_open.v==0)              ADD_STRING_CONFIG,"--zxdesktop-no-show-indicators-open-apps");

    if (zxdesktop_configurable_icons_enabled.v) {
        for (i=0;i<MAX_ZXDESKTOP_CONFIGURABLE_ICONS;i++) {
            if (zxdesktop_configurable_icons_list[i].status!=ZXDESKTOP_CUSTOM_ICON_NOT_EXISTS) {
                char buffer_status[30];
                //por defecto
                strcpy(buffer_status,"exists");

                if (zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_DELETED) {
                    strcpy(buffer_status,"deleted");
                }

                char texto_funcion[MAX_DEFINED_F_FUNCION_NAME_LENGTH];
                int indice_funcion=zxdesktop_configurable_icons_list[i].indice_funcion;
                strcpy(texto_funcion,defined_direct_functions_array[indice_funcion].texto_funcion);

                //--zxdesktop-add-icon x y a n e s    Add icon to position x,y, to function f, icon name n, extra parameters e, status s
                //Si opcion de vaciar papelera al salir, significa no grabar items que esten borrados

                int saveicon=1;

                if (zxdesktop_empty_trash_on_exit.v && zxdesktop_configurable_icons_list[i].status==ZXDESKTOP_CUSTOM_ICON_DELETED) {
                    saveicon=0;
                }

                if (saveicon) {

                    ADD_STRING_CONFIG,"--zxdesktop-add-icon %d %d \"%s\" \"%s\" \"%s\" \"%s\"",
                        zxdesktop_configurable_icons_list[i].pos_x,zxdesktop_configurable_icons_list[i].pos_y,
                        texto_funcion,zxdesktop_configurable_icons_list[i].text_icon,zxdesktop_configurable_icons_list[i].extra_info,
                        buffer_status);

                }
            }
        }
    }

  if (autoframeskip.v==0)                     ADD_STRING_CONFIG,"--disable-autoframeskip");

  if (auto_frameskip_even_when_movin_windows.v==0)         ADD_STRING_CONFIG,"--no-autoframeskip-moving-win");

  if (frameskip_draw_zxdesktop_background.v==0)    ADD_STRING_CONFIG,"--no-frameskip-zxdesktop-back");

  if (disable_change_flash.v)                 ADD_STRING_CONFIG,"--disable-flash");

  if (cambio_parametros_maquinas_lentas.v)    ADD_STRING_CONFIG,"--changeslowparameters");
  else                                        ADD_STRING_CONFIG,"--nochangeslowparameters");
  if (ventana_fullscreen)                     ADD_STRING_CONFIG,"--fullscreen");
  if (verbose_level)                          ADD_STRING_CONFIG,"--verbose %d",verbose_level);


  if (debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_EXCLUDE) ADD_STRING_CONFIG,"--debug-filter exclude");
  if (debug_mascara_modo_exclude_include==VERBOSE_MASK_CLASS_TYPE_INCLUDE) ADD_STRING_CONFIG,"--debug-filter include");


                                                ADD_STRING_CONFIG,"--debug-filter-exclude-mask %d",debug_mascara_clase_exclude);
                                                ADD_STRING_CONFIG,"--debug-filter-include-mask %d",debug_mascara_clase_include);


  if (debug_unnamed_console_enabled.v==0)     ADD_STRING_CONFIG,"--disable-debug-console-win");

  if (debug_always_show_messages_in_console.v) ADD_STRING_CONFIG,"--verbose-always-console");
  if (windows_no_disable_console.v)           ADD_STRING_CONFIG,"--nodisableconsole");
  if (porcentaje_velocidad_emulador!=100)     ADD_STRING_CONFIG,"--emulatorspeed %d",porcentaje_velocidad_emulador);
  if (zxuno_deny_turbo_bios_boot.v)           ADD_STRING_CONFIG,"--denyturbozxunoboot");

  if (tbblue_deny_turbo_rom.v) {
    ADD_STRING_CONFIG,"--denyturbotbbluerom");
  }

  else {
    ADD_STRING_CONFIG,"--allowturbotbbluerom");
  }

                                              ADD_STRING_CONFIG,"--tbblue-max-turbo-rom %d",tbblue_deny_turbo_rom_max_allowed);

  if (tbblue_deny_turbo_everywhere.v)         ADD_STRING_CONFIG,"--denyturbotbblueeverywhere");

                                              ADD_STRING_CONFIG,"--tbblue-max-turbo-everywhere %d",tbblue_deny_turbo_everywhere_max_allowed);


  if (tbblue_fast_boot_mode.v)                ADD_STRING_CONFIG,"--tbblue-fast-boot-mode");


  if (cpu_random_r_register.v)                ADD_STRING_CONFIG,"--random-r-register");


  //no uso esto de momento if (tbblue_initial_123b_port>=0)            ADD_STRING_CONFIG,"--tbblue-123b-port %d",tbblue_initial_123b_port);

  if (zx8081_get_standard_ram()!=16)          ADD_STRING_CONFIG,"--zx8081mem %d",zx8081_get_standard_ram());
  if (get_ram_ace()!=19)                      ADD_STRING_CONFIG,"--acemem %d",get_ram_ace() );
  if (mem128_multiplicador!=1)                ADD_STRING_CONFIG,"--128kmem %d",mem128_multiplicador*128);

  if (simulate_screen_zx8081.v)               ADD_STRING_CONFIG,"--videozx8081 %d",umbral_simulate_screen_zx8081);
                                              ADD_STRING_CONFIG,"--ao %s",audio_new_driver_name);
                                              ADD_STRING_CONFIG,"--vo %s",scr_new_driver_name);
  if (noautoload.v)                           ADD_STRING_CONFIG,"--noautoload");

  if (fast_autoload.v)			      ADD_STRING_CONFIG,"--fastautoload");


  if (ay_speech_enabled.v==0)                 ADD_STRING_CONFIG,"--disableayspeech");
  if (ay_envelopes_enabled.v==0)              ADD_STRING_CONFIG,"--disableenvelopes");
  if (audio_noreset_audiobuffer_full.v)       ADD_STRING_CONFIG,"--noreset-audiobuffer-full");


  if (silence_detector_setting.v)	      ADD_STRING_CONFIG,"--enable-silencedetector");
  else 					      ADD_STRING_CONFIG,"--disable-silencedetector");

  if (audioonebitspeaker_intensive_cpu_usage)     ADD_STRING_CONFIG,"--onebitspeaker-improved");
  if (audioonebitspeaker_agudo_filtro)            ADD_STRING_CONFIG,"--onebitspeaker-hifreq-filter");
                                              ADD_STRING_CONFIG,"--onebitspeaker-hifreq-filter-divider %d",audioonebitspeaker_agudo_filtro_limite);
                                              ADD_STRING_CONFIG,"--onebitspeaker-type %d",audioonebitspeaker_tipo_altavoz);

#ifdef COMPILE_ALSA
                                              ADD_STRING_CONFIG,"--alsacapturedevice %s",alsa_capture_device);
#endif

  if (border_enabled.v==0)                    ADD_STRING_CONFIG,"--disableborder");
  if (mouse_pointer_shown.v==0)               ADD_STRING_CONFIG,"--hidemousepointer");
  if (mouse_menu_disabled.v)                  ADD_STRING_CONFIG,"--disablemenumouse");
  if (mouse_menu_ignore_click_open.v)         ADD_STRING_CONFIG,"--ignoremouseclickopenmenu");

  if (kempston_mouse_emulation.v)             ADD_STRING_CONFIG,"--enablekempstonmouse");

                                              ADD_STRING_CONFIG,"--kempstonmouse-sens %d",kempston_mouse_factor_sensibilidad);


  if (core_spectrum_uses_reduced.v)           ADD_STRING_CONFIG,"--spectrum-reduced-core");
  else                                        ADD_STRING_CONFIG,"--no-spectrum-reduced-core");


                                              ADD_STRING_CONFIG,"--ula-data-bus %d",ula_databus_value);

  if (menu_footer==0)                         ADD_STRING_CONFIG,"--disablefooter");
  if (menu_multitarea==0)                     ADD_STRING_CONFIG,"--disablemultitaskmenu");
  if (menu_emulation_paused_on_menu)          ADD_STRING_CONFIG,"--stopemulationmenu");

  if (menu_old_behaviour_close_menus.v)       ADD_STRING_CONFIG,"--old-behaviour-menu-esc-etc");

  //if (screen_bw_no_multitask_menu.v==0)       ADD_STRING_CONFIG,"--disablebw-no-multitask");

  get_machine_config_name_by_number(buffer_temp,current_machine_type);

  if (buffer_temp[0]!=0) {
                                              ADD_STRING_CONFIG,"--machine %s",buffer_temp);
  }
  if (video_fast_mode_emulation.v)            ADD_STRING_CONFIG,"--videofastblack");

  if (ocr_settings_not_look_23606.v)          ADD_STRING_CONFIG,"--no-ocr-alternatechars");

  if (z88_hide_keys_shortcuts.v)                ADD_STRING_CONFIG,"--z88-hide-shortcuts");

  if (screen_text_all_refresh_pixel.v)        ADD_STRING_CONFIG,"--allpixeltotext");

                                              ADD_STRING_CONFIG,"--allpixeltotext-scale %d",screen_text_all_refresh_pixel_scale);

  if (screen_text_all_refresh_pixel_invert.v) ADD_STRING_CONFIG,"--allpixeltotext-invert");

                                            ADD_STRING_CONFIG,"--allpixeltotext-width %d",scr_refresca_pantalla_tsconf_text_max_ancho);
                                            ADD_STRING_CONFIG,"--allpixeltotext-x-offset %d",scr_refresca_pantalla_tsconf_text_offset_x);
                                            ADD_STRING_CONFIG,"--allpixeltotext-height %d",scr_refresca_pantalla_tsconf_text_max_alto);
                                            ADD_STRING_CONFIG,"--allpixeltotext-y-offset %d",scr_refresca_pantalla_tsconf_text_offset_y);

  if (zx8081_vsync_sound.v)                   ADD_STRING_CONFIG,"--zx8081vsyncsound");
  if (ram_in_8192.v)                          ADD_STRING_CONFIG,"--zx8081ram8K2000");
  if (ram_in_32768.v)                         ADD_STRING_CONFIG,"--zx8081ram16K8000");
  if (ram_in_49152.v)                         ADD_STRING_CONFIG,"--zx8081ram16KC000");
  if (autodetect_wrx.v)                       ADD_STRING_CONFIG,"--autodetectwrx");
  if (wrx_present.v)                          ADD_STRING_CONFIG,"--wrx");
                                              ADD_STRING_CONFIG,"--vsync-minimum-length %d",minimo_duracion_vsync);
  if (chroma81.v)                             ADD_STRING_CONFIG,"--chroma81");


  if (setting_set_machine_enable_custom_rom && custom_romfile[0]!=0) {
        ADD_STRING_CONFIG,"--romfile \"%s\"",custom_romfile);
  }


  if (quickload_file[0]!=0) {
	util_get_dir(quickload_file,buffer_temp);
 	ADD_STRING_CONFIG,"--smartloadpath \"%s\"",buffer_temp);
  }

  //Archivos recientes de smartload. Recorremos desde abajo hasta arriba
	for (i=MAX_LAST_FILESUSED-1;i>=0;i--) {
		if (last_files_used_array[i][0]!=0) ADD_STRING_CONFIG,"--addlastfile \"%s\"",last_files_used_array[i]);
	}



  if (snapshot_autosave_interval_quicksave_directory[0]!=0) {
        //Rutas que son directorios, llamar a util_copy_path_delete_last_slash
        //Rutas que apuntan a archivos (o directorios por linea de comandos pero que en menu almacenan archivos), llamar a util_get_dir
        //printf ("dir quicksave: %s\n",snapshot_autosave_interval_quicksave_directory);
        util_copy_path_delete_last_slash(snapshot_autosave_interval_quicksave_directory,buffer_temp);
        //printf ("dir quicksave final: %s\n",buffer_temp);

 	ADD_STRING_CONFIG,"--quicksavepath \"%s\"",buffer_temp);
  }


  if (binary_file_load[0]!=0) {
        //printf ("dir binary_file_load: %s\n",binary_file_load);
	util_get_dir(binary_file_load,buffer_temp);
        //printf ("dir binary_file_load final: %s\n",buffer_temp);
 	ADD_STRING_CONFIG,"--loadbinarypath \"%s\"",buffer_temp);
  }

  if (binary_file_save[0]!=0) {
        util_get_dir(binary_file_save,buffer_temp);
 	ADD_STRING_CONFIG,"--savebinarypath \"%s\"",buffer_temp);
  }

  if (zxuno_flash_spi_name[0])                ADD_STRING_CONFIG,"--zxunospifile \"%s\"",zxuno_flash_spi_name);
  if (zxuno_flash_persistent_writes.v)     ADD_STRING_CONFIG,"--zxunospi-persistent-writes");


  if (zxuno_flash_write_protection.v)         ADD_STRING_CONFIG,"--zxunospi-write-protection");

  //TODO printerbitmapfile
  //TODO printertextfile
  //TODO redefinekey

  if (recreated_zx_keyboard_support.v)	      ADD_STRING_CONFIG,"--recreatedzx");

  if (keyboard_issue2.v)                        ADD_STRING_CONFIG,"--keyboard-issue2");

                                                ADD_STRING_CONFIG,"--keymap %d",z88_cpc_keymap_type);

  if (autoload_snapshot_on_start.v)           ADD_STRING_CONFIG,"--autoloadsnap");
  if (autosave_snapshot_on_exit.v)            ADD_STRING_CONFIG,"--autosavesnap");
  if (autosave_snapshot_path_buffer[0]!=0) {
    //Quitar barra del final si la hay
    //strcpy(buffer_temp,autosave_snapshot_path_buffer);
    //i=strlen(buffer_temp);
    //if (i>1 && buffer_temp[i-1]=='/') buffer_temp[i-1]=0;
    util_copy_path_delete_last_slash(autosave_snapshot_path_buffer,buffer_temp);
        ADD_STRING_CONFIG,"--autosnappath \"%s\"",buffer_temp);
  }


  if (emulator_tmpdir_set_by_user[0]!=0) {
	//Quitar barra del final si la hay
	util_copy_path_delete_last_slash(emulator_tmpdir_set_by_user,buffer_temp);
 	ADD_STRING_CONFIG,"--tempdir \"%s\"",buffer_temp);
  }

  if (sna_setting_no_change_machine.v)          ADD_STRING_CONFIG,"--snap-no-change-machine");

  if (automount_esxdos_nex.v==0)                ADD_STRING_CONFIG,"--nex-no-automount-esxdos");


  if (zsf_snap_save_rom.v)                      ADD_STRING_CONFIG,"--zsf-save-rom");

  if (no_close_menu_after_smartload.v)          ADD_STRING_CONFIG,"--no-close-after-smartload");

  if (sync_clock_to_z88.v==0)                   ADD_STRING_CONFIG,"--z88-not-sync-clock-snap");

                                                ADD_STRING_CONFIG,"--snapram-interval %d",snapshot_in_ram_interval_seconds);
                                                ADD_STRING_CONFIG,"--snapram-max %d",snapshots_in_ram_maximum);
                                                ADD_STRING_CONFIG,"--snapram-rewind-timeout %d",snapshot_in_ram_enabled_timer_timeout);


  if (texto_artistico.v==0)                     ADD_STRING_CONFIG,"--disablearttext");
                                                ADD_STRING_CONFIG,"--arttextthresold %d",umbral_arttext);

  if (use_scrcursesw.v)                         ADD_STRING_CONFIG,"--curses-ext-utf");


  if (chardetect_printchar_enabled.v)         ADD_STRING_CONFIG,"--enableprintchartrap");
  if (stdout_simpletext_automatic_redraw.v)   ADD_STRING_CONFIG,"--autoredrawstdout");
  if (screen_text_accept_ansi)                ADD_STRING_CONFIG,"--sendansi");

                                              ADD_STRING_CONFIG,"--textfps %d",50/scrstdout_simpletext_refresh_factor);


  if (chardetect_line_width)                  ADD_STRING_CONFIG,"--linewidth %d",chardetect_line_width);
  if (chardetect_detect_char_enabled.v && trap_char_detection_routine_number==TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC)
                                              ADD_STRING_CONFIG,"--automaticdetectchar");
  if (chardetect_second_trap_char_dir)        ADD_STRING_CONFIG,"--secondtrapchar %d",chardetect_second_trap_char_dir);
  if (chardetect_third_trap_char_dir)         ADD_STRING_CONFIG,"--thirdtrapchar %d",chardetect_third_trap_char_dir);
                                              ADD_STRING_CONFIG,"--chartrapfilter \"%s\"",chardetect_char_filter_names[chardetect_char_filter]);
  if (chardetect_ignore_newline.v)            ADD_STRING_CONFIG,"--chardetectignorenl");

  if (chardetect_rom_compat_numbers.v)        ADD_STRING_CONFIG,"--chardetectcompatnum");

  if (textspeech_get_stdout.v) ADD_STRING_CONFIG,"--textspeechgetstdout");

  if (chardetect_line_width_wait_space.v)     ADD_STRING_CONFIG,"--linewidthwaitspace");
  else                                          ADD_STRING_CONFIG,"--linewidthnowaitspace");

  if (chardetect_line_width_wait_dot.v)     ADD_STRING_CONFIG,"--linewidthwaitdot");
  else                                      ADD_STRING_CONFIG,"--linewidthnowaitdot");

  if (chardetect_second_trap_sum32.v)         ADD_STRING_CONFIG,"--secondtrapsum32");
  if (textspeech_filter_program!=NULL)        ADD_STRING_CONFIG,"--textspeechprogram \"%s\"",textspeech_filter_program);
  if (textspeech_stop_filter_program!=NULL)   ADD_STRING_CONFIG,"--textspeechstopprogram \"%s\"",textspeech_stop_filter_program);
  if (textspeech_filter_program_wait.v)       ADD_STRING_CONFIG,"--textspeechwait");
  if (textspeech_also_send_menu.v)            ADD_STRING_CONFIG,"--textspeechmenu");
  if (textspeech_timeout_no_enter)            ADD_STRING_CONFIG,"--textspeechtimeout %d",textspeech_timeout_no_enter);

  if (accessibility_enable_gui_sounds.v)      ADD_STRING_CONFIG,"--accessibility-gui-sounds");

  if (textimage_filter_program[0])          ADD_STRING_CONFIG,"--textimageprogram \"%s\"",textimage_filter_program);

                                            ADD_STRING_CONFIG,"--textimage-method-location %s",
                                                textadv_location_additional_room_change_method_strings[textadv_location_additional_room_change_method]);
                                            ADD_STRING_CONFIG,"--textimage-min-time-between-images %d",textadv_location_desc_last_image_generated_min);
                                            ADD_STRING_CONFIG,"--textimage-min-no-char-time %d",max_textadv_location_desc_no_char_counter);
                                            ADD_STRING_CONFIG,"--textimage-min-after-room-time %d",max_textadv_location_desc_counter);
                                            ADD_STRING_CONFIG,"--textimage-total-count %d",textadv_location_total_conversions);

    if (menu_debug_textadventure_map_connections_center_current)            ADD_STRING_CONFIG,"--textadvmap-follow");
    if (menu_debug_textadventure_map_connections_show_rooms_no_connections) ADD_STRING_CONFIG,"--textadvmap-show-unconnected");
    if (menu_debug_textadventure_map_connections_show_unvisited==0)         ADD_STRING_CONFIG,"--textadvmap-no-show-unvisited");
    if (menu_debug_textadventure_map_connections_show_objects==0)           ADD_STRING_CONFIG,"--textadvmap-no-show-objects");
    if (menu_debug_textadventure_map_connections_show_pictures==0)          ADD_STRING_CONFIG,"--textadvmap-no-show-pictures");

                                            ADD_STRING_CONFIG,"--textadvmap-zoom %d",menu_debug_textadventure_map_connections_zoom);


                                              ADD_STRING_CONFIG,"--tool-sox-path \"%s\"",external_tool_sox);
                                              //ADD_STRING_CONFIG,"--tool-unzip-path \"%s\"",external_tool_unzip);
                                              ADD_STRING_CONFIG,"--tool-gunzip-path \"%s\"",external_tool_gunzip);
                                              ADD_STRING_CONFIG,"--tool-tar-path \"%s\"",external_tool_tar);
                                              ADD_STRING_CONFIG,"--tool-unrar-path \"%s\"",external_tool_unrar);
  if (mmc_file_name[0]!=0)                    ADD_STRING_CONFIG,"--mmc-file \"%s\"",mmc_file_name);
  if (mmc_enabled.v)                          ADD_STRING_CONFIG,"--enable-mmc");
  if (divmmc_mmc_ports_enabled.v)             ADD_STRING_CONFIG,"--enable-divmmc-ports");

  if (mmc_write_protection.v)		      ADD_STRING_CONFIG,"--mmc-write-protection");
  if (mmc_persistent_writes.v==0)	      ADD_STRING_CONFIG,"--mmc-no-persistent-writes");

  if (if1_enabled.v)                        ADD_STRING_CONFIG,"--enable-interface1");

  for (i=0;i<MAX_MICRODRIVES_BY_CONFIG;i++) {
    if (microdrive_status[i].microdrive_file_name[0]!=0)  ADD_STRING_CONFIG,"--zx-mdv-file %d \"%s\"",i+1,microdrive_status[i].microdrive_file_name);
    if (microdrive_status[i].microdrive_enabled) ADD_STRING_CONFIG,"--zx-mdv-enable %d",i+1);
    if (microdrive_status[i].microdrive_persistent_writes==0) ADD_STRING_CONFIG,"--zx-mdv-no-persistent-writes %d",i+1);
  }


  //Los settings de  mmc paging no guardarlo si maquina es tbblue, pues acaba activando el divmmc paging en tbblue y entonces esto provoca que no arranque la tbblue rom
  if (!MACHINE_IS_TBBLUE) {
    if (divmmc_diviface_enabled.v)            ADD_STRING_CONFIG,"--enable-divmmc-paging");
    if (divmmc_mmc_ports_enabled.v && divmmc_diviface_enabled.v)
                                              ADD_STRING_CONFIG,"--enable-divmmc");
  }

  if (divmmc_rom_name[0]!=0)                  ADD_STRING_CONFIG,"--divmmc-rom \"%s\"",divmmc_rom_name);

  if (hilow_file_name[0]!=0)                    ADD_STRING_CONFIG,"--hilow-file \"%s\"",hilow_file_name);
  if (hilow_enabled.v)                          ADD_STRING_CONFIG,"--enable-hilow");
  if (hilow_write_protection.v)		            ADD_STRING_CONFIG,"--hilow-write-protection");
  if (hilow_persistent_writes.v==0)	            ADD_STRING_CONFIG,"--hilow-no-persistent-writes");

  if (z88_eprom_or_flash_persistent_writes.v==0)    ADD_STRING_CONFIG,"--z88-no-persistent-writes");

  if (ide_file_name[0]!=0)                    ADD_STRING_CONFIG,"--ide-file \"%s\"",ide_file_name);
  if (ide_enabled.v)                          ADD_STRING_CONFIG,"--enable-ide");
  if (ide_write_protection.v)		      ADD_STRING_CONFIG,"--ide-write-protection");
  if (ide_persistent_writes.v==0)	      ADD_STRING_CONFIG,"--ide-no-persistent-writes");
  if (divide_ide_ports_enabled.v)             ADD_STRING_CONFIG,"--enable-divide-ports");
  if (divide_diviface_enabled.v)              ADD_STRING_CONFIG,"--enable-divide-paging");

  if (divide_ide_ports_enabled.v && divide_diviface_enabled.v)
                                              ADD_STRING_CONFIG,"--enable-divide");
  if (divide_rom_name[0]!=0)                  ADD_STRING_CONFIG,"--divide-rom \"%s\"",divide_rom_name);


  if (zxmmc_emulation.v)                      ADD_STRING_CONFIG,"--enable-zxmmc");
  if (eight_bit_simple_ide_enabled.v)         ADD_STRING_CONFIG,"--enable-8bit-ide");

			                      ADD_STRING_CONFIG,"--diviface-ram-size %d",get_diviface_total_ram());

  if (zxpand_enabled.v)                       ADD_STRING_CONFIG,"--enable-zxpand");
  if (zxpand_root_dir[0]!=0)                  ADD_STRING_CONFIG,"--zxpand-root-dir \"%s\"",zxpand_root_dir);

  //no decir que esta habilitado si se va a desactivar al resetear
  if (esxdos_handler_enabled.v && esxdos_umount_on_reset.v==0)               ADD_STRING_CONFIG,"--enable-esxdos-handler");
  if (esxdos_handler_root_dir[0]!=0)          ADD_STRING_CONFIG,"--esxdos-root-dir \"%s\"",esxdos_handler_root_dir);

  if (esxdos_handler_enabled.v && esxdos_handler_cwd[0]!=0)  ADD_STRING_CONFIG,"--esxdos-local-dir \"%s\"",esxdos_handler_cwd);

  if (esxdos_handler_readonly.v)              ADD_STRING_CONFIG,"--esxdos-readonly");

  if (ql_microdrive_floppy_emulation)         ADD_STRING_CONFIG,"--enable-ql-mdv-flp");
  if (ql_mdv1_root_dir[0]!=0)                 ADD_STRING_CONFIG,"--ql-mdv1-root-dir \"%s\"",ql_mdv1_root_dir);
  if (ql_mdv2_root_dir[0]!=0)                 ADD_STRING_CONFIG,"--ql-mdv2-root-dir \"%s\"",ql_mdv2_root_dir);
  if (ql_flp1_root_dir[0]!=0)                 ADD_STRING_CONFIG,"--ql-flp1-root-dir \"%s\"",ql_flp1_root_dir);

  if (ql_device_mdv1_enabled)                 ADD_STRING_CONFIG,"--ql-mdv1-enable");
  if (ql_device_mdv2_enabled)                 ADD_STRING_CONFIG,"--ql-mdv2-enable");
  if (ql_device_flp1_enabled)                 ADD_STRING_CONFIG,"--ql-flp1-enable");


  if (ql_device_mdv1_readonly)                ADD_STRING_CONFIG,"--ql-mdv1-read-only");
  if (ql_device_mdv2_readonly)                ADD_STRING_CONFIG,"--ql-mdv2-read-only");
  if (ql_device_flp1_readonly)                ADD_STRING_CONFIG,"--ql-flp1-read-only");

  if (ql_flp1_follow_mdv1.v)                  ADD_STRING_CONFIG,"--ql-flp1-dir-copied-mdv1");

  if (dandanator_rom_file_name[0]!=0)         ADD_STRING_CONFIG,"--dandanator-rom \"%s\"",dandanator_rom_file_name);
  if (dandanator_enabled.v)                   ADD_STRING_CONFIG,"--enable-dandanator");
  if (kartusho_rom_file_name[0]!=0)         ADD_STRING_CONFIG,"--kartusho-rom \"%s\"",kartusho_rom_file_name);
  if (kartusho_enabled.v)                   ADD_STRING_CONFIG,"--enable-kartusho");

  if (ifrom_rom_file_name[0]!=0)         ADD_STRING_CONFIG,"--ifrom-rom \"%s\"",ifrom_rom_file_name);
  if (ifrom_enabled.v)                   ADD_STRING_CONFIG,"--enable-ifrom");

  if (betadisk_enabled.v)                   ADD_STRING_CONFIG,"--enable-betadisk");

  if (trd_file_name[0]!=0)                    ADD_STRING_CONFIG,"--trd-file \"%s\"",trd_file_name);
  if (trd_enabled.v)                          ADD_STRING_CONFIG,"--enable-trd");
  if (trd_write_protection.v)		      ADD_STRING_CONFIG,"--trd-write-protection");
  if (trd_persistent_writes.v==0)	      ADD_STRING_CONFIG,"--trd-no-persistent-writes");



  if (dskplusthree_file_name[0]!=0)           ADD_STRING_CONFIG,"--dsk-file \"%s\"",dskplusthree_file_name);
  if (dskplusthree_emulation.v)               ADD_STRING_CONFIG,"--enable-dsk");
  if (dskplusthree_write_protection.v)	      ADD_STRING_CONFIG,"--dsk-write-protection");

  if (pd765_silent_write_protection.v)        ADD_STRING_CONFIG,"--pd765-silent-write-protection");

  if (dskplusthree_persistent_writes.v==0)    ADD_STRING_CONFIG,"--dsk-no-persistent-writes");
  else                                        ADD_STRING_CONFIG,"--dsk-persistent-writes");

  if (pcw_boot_reinsert_previous_dsk.v==0)    ADD_STRING_CONFIG,"--dsk-pcw-no-boot-reinsert-previous-dsk");
  if (pcw_failback_cpm_when_no_boot.v==0)     ADD_STRING_CONFIG,"--dsk-pcw-no-failback-cpm-when-no-boot");

  if (superupgrade_rom_file_name[0]!=0)       ADD_STRING_CONFIG,"--superupgrade-flash \"%s\"",superupgrade_rom_file_name);
  if (superupgrade_enabled.v)                 ADD_STRING_CONFIG,"--enable-superupgrade");


                                              ADD_STRING_CONFIG,"--showfiredbreakpoint %d",debug_show_fired_breakpoints_type);


  if (debug_breakpoints_enabled.v)            ADD_STRING_CONFIG,"--enable-breakpoints");

  if (debug_shows_invalid_opcode.v)           ADD_STRING_CONFIG,"--show-invalid-opcode");


  if (debug_breakpoints_cond_behaviour.v==0)  ADD_STRING_CONFIG,"--brkp-always");


  if (debug_settings_show_screen.v)           ADD_STRING_CONFIG,"--show-display-debug");


  if (menu_debug_registers_if_showscan.v)     ADD_STRING_CONFIG,"--show-electron-debug");

  if (debug_view_basic_show_address.v)        ADD_STRING_CONFIG,"--show-basic-address");

                                              ADD_STRING_CONFIG,"--cpu-history-max-items %d",cpu_history_max_elements);

  if (remote_tamanyo_archivo_raw_source_code) ADD_STRING_CONFIG,"--load-source-code %s",last_source_code_file);


  for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {



			if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {

				//nuevo parser de breakpoints
				char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
				exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
                                ADD_STRING_CONFIG,"--set-breakpoint %d \"%s\"",i+1,buffer_temp);

                if (!debug_breakpoints_conditions_enabled[i]) {
                    ADD_STRING_CONFIG,"--disable-breakpoint %d",i+1);
                }
			}


	}

  for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
    if (debug_breakpoints_actions_array[i][0]!=0)
                                              ADD_STRING_CONFIG,"--set-breakpointaction %d \"%s\"",i+1,debug_breakpoints_actions_array[i]);
  }



  for (i=0;i<DEBUG_MAX_WATCHES;i++) {

			if (debug_watches_array[i][0].tipo!=TPT_FIN) {

				char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
				exp_par_tokens_to_exp(debug_watches_array[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
                                ADD_STRING_CONFIG,"--set-watch %d \"%s\"",i+1,buffer_temp);
			}

	}



  for (i=0;i<65536;i++) {
		if (mem_breakpoint_array[i]!=0)
                                              ADD_STRING_CONFIG,"--set-mem-breakpoint %04XH %d",i,mem_breakpoint_array[i]);
	}

  if (hardware_debug_port.v)                  ADD_STRING_CONFIG,"--hardware-debug-ports");
  if (zesarux_zxi_hardware_debug_file[0]!=0)  ADD_STRING_CONFIG,"--hardware-debug-ports-byte-file \"%s\"",zesarux_zxi_hardware_debug_file);

  if (debug_dump_zsf_on_cpu_panic.v)          ADD_STRING_CONFIG,"--dump-snapshot-panic");

  if (autoselect_snaptape_options.v==0)       ADD_STRING_CONFIG,"--noautoselectfileopt");
  if (screen_show_splash_texts.v==0)          ADD_STRING_CONFIG,"--nosplash");
  if (screen_show_cpu_usage.v==0)             ADD_STRING_CONFIG,"--no-cpu-usage");
  if (screen_show_cpu_temp.v==0)              ADD_STRING_CONFIG,"--no-cpu-temp");
  if (screen_show_fps.v==0)                   ADD_STRING_CONFIG,"--no-fps");
  if (opcion_no_welcome_message.v)                     ADD_STRING_CONFIG,"--nowelcomemessage");
  if (opcion_fast_welcome_message.v)          ADD_STRING_CONFIG,"--fastwelcomemessage");

  if (xanniversary_logo.v)                    ADD_STRING_CONFIG,"--enable-xanniversary-logo");

  if (menu_hide_vertical_percentaje_bar.v) ADD_STRING_CONFIG,"--hide-menu-percentage-bar");

  if (menu_hide_submenu_indicator.v)            ADD_STRING_CONFIG,"--hide-menu-submenu-indicator");
  if (menu_show_submenus_tree.v==0)             ADD_STRING_CONFIG,"--no-show-previous-submenus");

  if (menu_hide_minimize_button.v)           ADD_STRING_CONFIG,"--hide-menu-minimize-button");
  if (menu_hide_maximize_button.v)           ADD_STRING_CONFIG,"--hide-menu-maximize-button");
  if (menu_hide_close_button.v)              ADD_STRING_CONFIG,"--hide-menu-close-button");

  if (menu_hide_background_button_on_inactive.v==0) ADD_STRING_CONFIG,"--show-menu-background-button");

  if (menu_change_frame_when_resize_zone.v==0)      ADD_STRING_CONFIG,"--no-change-frame-resize-zone");

  if (gui_language==GUI_LANGUAGE_SPANISH)   ADD_STRING_CONFIG,"--language es");
  if (gui_language==GUI_LANGUAGE_CATALAN)   ADD_STRING_CONFIG,"--language ca");

  if (online_download_path[0]!=0)           ADD_STRING_CONFIG,"--online-download-path \"%s\"",online_download_path);

  if (menu_invert_mouse_scroll.v)             ADD_STRING_CONFIG,"--invert-menu-mouse-scroll");

  if (menu_mouse_right_send_esc.v)          ADD_STRING_CONFIG,"--right-mouse-esc");

  if (setting_process_switcher_immutable.v)     ADD_STRING_CONFIG,"--process-switcher-immutable");

  if (setting_process_switcher_always_visible.v)     ADD_STRING_CONFIG,"--process-switcher-always-visible");

  if (setting_process_switcher_force_left_bottom.v==0)  ADD_STRING_CONFIG,"--process-switcher-no-left-bottom");

  if (menu_allow_background_windows)          ADD_STRING_CONFIG,"--allow-background-windows");

  if (always_force_overlay_visible_when_menu_closed) ADD_STRING_CONFIG,"--allow-background-windows-closed-menu");

                                              ADD_STRING_CONFIG,"--menu-mix-method \"%s\"",screen_menu_mix_methods_strings[screen_menu_mix_method]);


                                                ADD_STRING_CONFIG,"--menu-transparency-perc %d",screen_menu_mix_transparency);



  //lo desactivo. Esto da problemas con footer
  //if (screen_menu_reduce_bright_machine.v)   ADD_STRING_CONFIG,"--menu-darken-when-open");

  if (screen_machine_bw_no_multitask.v)      ADD_STRING_CONFIG,"--menu-bw-multitask");



  if (rainbow_enabled.v)                      ADD_STRING_CONFIG,"--realvideo");

  if (autodetect_rainbow.v==0)                ADD_STRING_CONFIG,"--no-detect-realvideo");

  if (tbblue_store_scanlines.v)               ADD_STRING_CONFIG,"--tbblue-legacy-hicolor");
  if (tbblue_store_scanlines_border.v)        ADD_STRING_CONFIG,"--tbblue-legacy-border");
  if (tbblue_disable_optimized_sprites.v)     ADD_STRING_CONFIG,"--tbblue-no-sprite-optimization");

  //if (tsconf_si_render_spritetile_rapido.v)   ADD_STRING_CONFIG,"--tsconf-fast-render");


  if (gigascreen_enabled.v)                   ADD_STRING_CONFIG,"--enablegigascreen");
  if (video_interlaced_mode.v)                ADD_STRING_CONFIG,"--enableinterlaced");

  if (ulaplus_presente.v)                     ADD_STRING_CONFIG,"--enableulaplus");
  if (spectra_enabled.v)                      ADD_STRING_CONFIG,"--enablespectra");
  if (timex_video_emulation.v)                ADD_STRING_CONFIG,"--enabletimexvideo");
  if (timex_mode_512192_real.v==0)	      ADD_STRING_CONFIG,"--disablerealtimex512");
  if (pentagon_16c_mode_available.v)          ADD_STRING_CONFIG,"--enable16c");


  if (spritechip_enabled.v)                   ADD_STRING_CONFIG,"--enablezgx");
  if (beeper_enabled.v==0)                    ADD_STRING_CONFIG,"--disablebeeper");
  if (beeper_real_enabled==0)                 ADD_STRING_CONFIG,"--disablerealbeeper");
  if (ay_retorna_numero_chips()>1)            ADD_STRING_CONFIG,"--totalaychips %d",ay_retorna_numero_chips() );

  if (ay3_stereo_mode>0)                      ADD_STRING_CONFIG,"--ay-stereo-mode %d",ay3_stereo_mode);

                                              ADD_STRING_CONFIG,"--ay-stereo-channel A %d",ay3_custom_stereo_A);
                                              ADD_STRING_CONFIG,"--ay-stereo-channel B %d",ay3_custom_stereo_B);
                                              ADD_STRING_CONFIG,"--ay-stereo-channel C %d",ay3_custom_stereo_C);

  if (audiodac_enabled.v)                     ADD_STRING_CONFIG,"--enableaudiodac");
                                              ADD_STRING_CONFIG,"--audiodactype \"%s\"",audiodac_types[audiodac_selected_type].name);

  if (snow_effect_enabled.v)                  ADD_STRING_CONFIG,"--snoweffect");
  if (audiovolume!=100)                       ADD_STRING_CONFIG,"--audiovolume %d",audiovolume);
  if (ay_player_exit_emulator_when_finish.v)  ADD_STRING_CONFIG,"--ayplayer-end-exit");
  if (ay_player_shuffle_mode.v)               ADD_STRING_CONFIG,"--ayplayer-shuffle");
  if (ay_player_silence_detection.v==0)       ADD_STRING_CONFIG,"--ayplayer-no-silence-detection");
  if (ay_player_repeat_file.v)                ADD_STRING_CONFIG,"--ayplayer-end-repeat");
  if (ay_player_limit_infinite_tracks!=0)     ADD_STRING_CONFIG,"--ayplayer-inf-length %d",ay_player_limit_infinite_tracks/50);
  if (ay_player_limit_any_track!=0)           ADD_STRING_CONFIG,"--ayplayer-any-length %d",ay_player_limit_any_track/50);
  if (ay_player_cpc_mode.v)                   ADD_STRING_CONFIG,"--ayplayer-cpc");
  if (ay_player_show_info_console.v)          ADD_STRING_CONFIG,"--ayplayer-show-info-console");

                                              ADD_STRING_CONFIG,"--audiopiano-zoom %d",audiochip_piano_zoom_x);


  if (audio_midi_output_initialized)          ADD_STRING_CONFIG,"--enable-midi");
                                              ADD_STRING_CONFIG,"--midi-client %d",audio_midi_client);
                                              ADD_STRING_CONFIG,"--midi-port %d",audio_midi_port);

                                              ADD_STRING_CONFIG,"--midi-raw-device %s",audio_raw_midi_device_out);
  if (audio_midi_raw_mode==0)                 ADD_STRING_CONFIG,"--midi-no-raw-mode");


  if (midi_output_record_noisetone.v)         ADD_STRING_CONFIG,"--midi-allow-tone-noise");



  //Este setting lo permitimos siempre, aunque no se haya compilado driver sdl, pues es una variable global, aunque no se verá en la ayuda,
  if (sdl_raw_keyboard_read.v)                ADD_STRING_CONFIG,"--sdlrawkeyboard");


  //Este setting lo permitimos siempre, aunque no se haya compilado driver sdl, pues es una variable global, aunque no se verá en la ayuda,
  if (audiosdl_use_new_callback.v)              ADD_STRING_CONFIG,"--sdl-use-callback-new");
  //Este setting lo permitimos siempre, aunque no se haya compilado driver sdl, pues es una variable global, aunque no se verá en la ayuda,
  if (audiosdl_use_new_callback.v==0)           ADD_STRING_CONFIG,"--sdl-use-callback-old");


  if (standard_to_real_tape_fallback.v==0)    ADD_STRING_CONFIG,"--no-fallbacktorealtape");
  if (tape_any_flag_loading.v)                ADD_STRING_CONFIG,"--anyflagloading");
  if (tape_auto_rewind.v)                     ADD_STRING_CONFIG,"--autorewind");

  if (tape_loading_simulate.v)                ADD_STRING_CONFIG,"--simulaterealload");
  if (tape_loading_simulate_fast.v)           ADD_STRING_CONFIG,"--simulaterealloadfast");
  if (tzx_suppress_pause.v)                   ADD_STRING_CONFIG,"--deletetzxpauses");

  if (accelerate_loaders.v)                   ADD_STRING_CONFIG,"--realloadfast");


  if (screen_gray_mode&1)                     ADD_STRING_CONFIG,"--blue");
  if (screen_gray_mode&2)                     ADD_STRING_CONFIG,"--green");
  if (screen_gray_mode&4)                     ADD_STRING_CONFIG,"--red");
  if (inverse_video.v)                        ADD_STRING_CONFIG,"--inversevideo");
  if (spectrum_1648_use_real_palette.v)       ADD_STRING_CONFIG,"--realpalette");

  if (tooltip_enabled.v==0)                   ADD_STRING_CONFIG,"--disabletooltips");


  if (menu_disable_first_aid.v)               ADD_STRING_CONFIG,"--disable-all-first-aid");

	for (i=0;i<total_first_aid;i++) {
                int *opcion;
                opcion=first_aid_list[i].puntero_setting;

                if (*opcion) ADD_STRING_CONFIG,"--no-first-aid %s",first_aid_list[i].config_name);
	}


  if (menu_limit_menu_open.v)                 ADD_STRING_CONFIG,"--limitopenmenu");

  if (menu_show_advanced_items.v)               ADD_STRING_CONFIG,"--advancedmenus");

  if (setting_machine_selection_by_name.v)    ADD_STRING_CONFIG,"--setmachinebyname");

  if (menu_filesel_hide_dirs.v)         ADD_STRING_CONFIG,"--filebrowser-hide-dirs");

  if (menu_filesel_hide_size.v)             ADD_STRING_CONFIG,"--filebrowser-hide-size");

  if (menu_filesel_utils_allow_folder_delete.v)      ADD_STRING_CONFIG,"--filebrowser-allow-folder-delete");


  if (menu_file_viewer_always_hex.v)            ADD_STRING_CONFIG,"--fileviewer-hex");

  if (menu_filesel_show_previews.v==0)         ADD_STRING_CONFIG,"--no-file-previews");

  if (menu_filesel_show_previews_reduce.v)      ADD_STRING_CONFIG,"--reduce-file-previews");

  if (menu_desactivado.v)                     ADD_STRING_CONFIG,"--disablemenu");

  if (menu_desactivado_andexit.v)              ADD_STRING_CONFIG,"--disablemenuandexit");


  if (menu_desactivado_file_utilities.v)      ADD_STRING_CONFIG,"--disablemenufileutils");

  //if (index_menu_enabled.v==0)                  ADD_STRING_CONFIG,"--disable-search-menu");
  if (index_menu_enabled.v==1)                  ADD_STRING_CONFIG,"--enable-search-menu");

  if (menu_force_writing_inverse_color.v)     ADD_STRING_CONFIG,"--forcevisiblehotkeys");
  if (force_confirm_yes.v)                    ADD_STRING_CONFIG,"--forceconfirmyes");
                                              ADD_STRING_CONFIG,"--gui-style \"%s\"",definiciones_estilos_gui[estilo_gui_activo].nombre_estilo);


  if (user_charset>=0)                        ADD_STRING_CONFIG,"--charset \"%s\"",charset_list[user_charset].nombre);

  if (char_set_customfile_path[0]!=0)         ADD_STRING_CONFIG,"--charset-customfile \"%s\"",char_set_customfile_path);


  if (zeng_remote_hostname[0]!=0)             ADD_STRING_CONFIG,"--zeng-remote-hostname %s",zeng_remote_hostname);
                                              ADD_STRING_CONFIG,"--zeng-remote-port %d",zeng_remote_port);
                                              //ADD_STRING_CONFIG,"--zeng-snapshot-interval %d",zeng_segundos_cada_snapshot);
                                              ADD_STRING_CONFIG,"--zeng-snapshot-interval-frames %d",zeng_frames_video_cada_snapshot);
  if (zeng_i_am_master)                       ADD_STRING_CONFIG,"--zeng-iam-master");
  if (zeng_do_not_send_input_events)          ADD_STRING_CONFIG,"--zeng-not-send-input-events");




                                              ADD_STRING_CONFIG,"--total-minutes-use %d",total_minutes_use);


  if (stats_asked.v)                          ADD_STRING_CONFIG,"--stats-send-already-asked");
  if (stats_enabled.v)                        ADD_STRING_CONFIG,"--stats-send-enabled");

  if (tbblue_autoconfigure_sd_asked.v)             ADD_STRING_CONFIG,"--tbblue-autoconfigure-sd-already-asked");



  if (stats_uuid[0]!=0)                      ADD_STRING_CONFIG,"--stats-uuid %s",stats_uuid);

  			ADD_STRING_CONFIG,"--stats-speccy-queries %d",stats_total_speccy_browser_queries);

			ADD_STRING_CONFIG,"--stats-zx81-queries %d",stats_total_zx81_browser_queries);





  if (stats_check_updates_enabled.v==0)                 ADD_STRING_CONFIG,"--stats-disable-check-updates");

  if (stats_check_yesterday_users_enabled.v==0)         ADD_STRING_CONFIG,"--stats-disable-check-yesterday-users");




  if (stats_last_remote_version[0]!=0)       ADD_STRING_CONFIG,"--stats-last-avail-version %s",stats_last_remote_version);


  if (avoid_christmas_mode.v)               ADD_STRING_CONFIG,"--avoid-christmas-mode");


  if (do_no_show_changelog_when_update.v)     ADD_STRING_CONFIG,"--no-show-changelog");

  if (do_no_show_david_in_memoriam.v)         ADD_STRING_CONFIG,"--no-show-david-in-memoriam");


  if (parameter_disablebetawarning[0])        ADD_STRING_CONFIG,"--disablebetawarning \"%s\"",parameter_disablebetawarning);

  if (parameter_disable_allbetawarningsleep.v)  ADD_STRING_CONFIG,"--disableallbetawarningpause");

  for (i=0;i<total_config_window_geometry;i++) {
       ADD_STRING_CONFIG,"--window-geometry %s %d %d %d %d %d %d %d %d", saved_config_window_geometry_array[i].nombre,
       saved_config_window_geometry_array[i].x,saved_config_window_geometry_array[i].y,
        saved_config_window_geometry_array[i].ancho,saved_config_window_geometry_array[i].alto,
        saved_config_window_geometry_array[i].width_before_max_min_imize,saved_config_window_geometry_array[i].height_before_max_min_imize,
        saved_config_window_geometry_array[i].is_minimized,saved_config_window_geometry_array[i].is_maximized
        );
  }

  if (menu_reopen_background_windows_on_start.v==0) ADD_STRING_CONFIG,"--disable-restore-windows");


  if (menu_allow_background_windows && menu_reopen_background_windows_on_start.v) {
        //Guardar lista de ventanas activas
	//Empezamos una a una, desde la de mas abajo
	zxvision_window *ventana;

	ventana=zxvision_current_window;

	if (ventana!=NULL) {

                ventana=zxvision_find_first_window_below_this(ventana);

                while (ventana!=NULL) {

                        if (ventana->geometry_name[0]!=0 && ventana->can_be_backgrounded) {
                                ADD_STRING_CONFIG,"--restorewindow \"%s\"",ventana->geometry_name);
                        }


                        ventana=ventana->next_window;

                }
        }

  }


    //Sensores en la ventana de debug view sensors
    for (i=0;i<MENU_VIEW_SENSORS_TOTAL_ELEMENTS;i++) {
        if (menu_debug_view_sensors_list_sensors[i].short_name[0]) {
            ADD_STRING_CONFIG,"--sensor-set %d \"%s\" ",i,menu_debug_view_sensors_list_sensors[i].short_name);
            int widget_id=menu_debug_view_sensors_list_sensors[i].tipo;
            //printf("id: %d\n",widget_id);
            ADD_STRING_CONFIG,"--sensor-set-widget %d \"%s\" ",i,zxvision_widget_types_names[widget_id]);

            if (menu_debug_view_sensors_list_sensors[i].valor_en_vez_de_perc) ADD_STRING_CONFIG,"--sensor-set-abs %d",i);
        }
    }


    //Historiales de varios campos de texto

    //Debug cpu change ptr
    //Obtener de ultimo a mas reciente, para que al insertarlo queden en el orden correcto
    int history_length=util_scanf_history_get_total_lines(menu_debug_registers_change_ptr_historial);
    for (i=history_length-1;i>=0;i--) {
        ADD_STRING_CONFIG,"--history-item-add-debugcpu-ptr \"%s\" ",menu_debug_registers_change_ptr_historial[i]);
    }

    //Hex editor
    //Obtener de ultimo a mas reciente, para que al insertarlo queden en el orden correcto
    history_length=util_scanf_history_get_total_lines(menu_debug_hexdump_change_ptr_historial);
    for (i=history_length-1;i>=0;i--) {
        ADD_STRING_CONFIG,"--history-item-add-hexeditor-ptr \"%s\" ",menu_debug_hexdump_change_ptr_historial[i]);
    }

    //Sprites
    //Obtener de ultimo a mas reciente, para que al insertarlo queden en el orden correcto
    history_length=util_scanf_history_get_total_lines(menu_debug_sprites_change_ptr_historial);
    for (i=history_length-1;i>=0;i--) {
        ADD_STRING_CONFIG,"--history-item-add-sprites-ptr \"%s\" ",menu_debug_sprites_change_ptr_historial[i]);
    }

    //Poke addr
    //Obtener de ultimo a mas reciente, para que al insertarlo queden en el orden correcto
    history_length=util_scanf_history_get_total_lines(menu_debug_poke_address_historial);
    for (i=history_length-1;i>=0;i--) {
        ADD_STRING_CONFIG,"--history-item-add-poke-ptr \"%s\" ",menu_debug_poke_address_historial[i]);
    }

    //Poke value
    //Obtener de ultimo a mas reciente, para que al insertarlo queden en el orden correcto
    history_length=util_scanf_history_get_total_lines(menu_debug_poke_value_historial);
    for (i=history_length-1;i>=0;i--) {
        ADD_STRING_CONFIG,"--history-item-add-poke-value \"%s\" ",menu_debug_poke_value_historial[i]);
    }


  for (i=0;i<MAX_F_FUNCTIONS_KEYS;i++) {
    int indice=defined_f_functions_keys_array[i];
    enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice);
    if (accion!=F_FUNCION_DEFAULT) {
                                              ADD_STRING_CONFIG,"--def-f-function F%d \"%s\"",i+1,defined_direct_functions_array[indice].texto_funcion);
    }
  }

  for (i=0;i<MAX_F_FUNCTIONS_KEYS;i++) {
    if (defined_f_functions_keys_array_parameters[i][0]!=0) {
                                              ADD_STRING_CONFIG,"--def-f-function-parameters F%d \"%s\"",i+1,defined_f_functions_keys_array_parameters[i]);
    }
  }


  for (i=0;i<MAX_USERDEF_BUTTONS;i++) {
    int indice=defined_buttons_functions_array[i];
    enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice);
    if (accion!=F_FUNCION_DEFAULT) {
                                              ADD_STRING_CONFIG,"--def-button-function %d \"%s\"",i,defined_direct_functions_array[indice].texto_funcion);
    }
  }

  for (i=0;i<MAX_USERDEF_BUTTONS;i++) {
    if (defined_buttons_functions_array_parameters[i][0]!=0) {
                                              ADD_STRING_CONFIG,"--def-button-function-parameters %d \"%s\"",i,defined_buttons_functions_array_parameters[i]);
    }
  }


  if (input_file_keyboard_name!=NULL && input_file_keyboard_inserted.v)         ADD_STRING_CONFIG,"--keyboardspoolfile \"%s\"",input_file_keyboard_name);

  if (input_file_keyboard_playing.v)           ADD_STRING_CONFIG,"--keyboardspoolfile-play");

                                                ADD_STRING_CONFIG,"--keyboardspoolfile-keylength %d",input_file_keyboard_delay);

  if (input_file_keyboard_send_pause.v==0)      ADD_STRING_CONFIG,"--keyboardspoolfile-nodelay");

                                              ADD_STRING_CONFIG,"--joystickemulated \"%s\"",joystick_texto[joystick_emulation]);


                                            ADD_STRING_CONFIG,"--joystickfirekey %d",joystick_defined_key_fire);


  if (remote_protocol_enabled.v)                ADD_STRING_CONFIG,"--enable-remoteprotocol");
                                                ADD_STRING_CONFIG,"--remoteprotocol-port %d",remote_protocol_port);
                                                ADD_STRING_CONFIG,"--remoteprotocol-prompt \"%s\"",remote_prompt_command_string);

  if (zeng_online_enabled)                      ADD_STRING_CONFIG,"--enable-zeng-online-server");


                                                ADD_STRING_CONFIG,"--zeng-online-hostname \"%s\"",zeng_online_server);
  if (zeng_online_nickname[0])                  ADD_STRING_CONFIG,"--zeng-online-nickname \"%s\"",zeng_online_nickname);


  if (zeng_online_zip_compress_snapshots.v==0)  ADD_STRING_CONFIG,"--zeng-online-no-zip-snapshots");
  if (zeng_online_show_footer_lag_indicator.v==0)   ADD_STRING_CONFIG,"--zeng-online-no-footer-lag-indicator");
  if (zeng_online_allow_room_creation_from_any_ip.v)    ADD_STRING_CONFIG,"--zeng-online-server-allow-create");
  if (zeng_online_destroy_rooms_without_players.v)      ADD_STRING_CONFIG,"--zeng-online-server-destroy-rooms-no-players");
                                                ADD_STRING_CONFIG,"--zeng-online-server-max-rooms %d",zeng_online_current_max_rooms);
                                                ADD_STRING_CONFIG,"--zeng-online-server-max-players-room %d",zeng_online_current_max_players_per_room);

  if (zeng_online_server_allow_zrcp_only_zeng_online.v==0)  ADD_STRING_CONFIG,"--zeng-online-server-allow-all-zrcp");

  if (realjoystick_disabled.v==1)              ADD_STRING_CONFIG,"--disablerealjoystick");


  if (realjoystick_index!=0)                ADD_STRING_CONFIG,"--realjoystickindex %d",realjoystick_index);

  if (no_native_linux_realjoystick.v)          ADD_STRING_CONFIG,"--no-native-linux-realjoy");



                          ADD_STRING_CONFIG,"--realjoystickpath %s",string_dev_joystick);

                        ADD_STRING_CONFIG,"--realjoystick-calibrate %d",realjoystick_autocalibrate_value);



  //real joystick buttons to events. Siempre este antes que el de events/buttons to keys
  for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
  	if (realjoystick_events_array[i].asignado.v) {
  		char texto_button[20];
  		int button_type;
  		button_type=realjoystick_events_array[i].button_type;

  		util_write_config_aux_realjoystick(button_type, realjoystick_events_array[i].button, texto_button);

  		ADD_STRING_CONFIG,"--joystickevent %s %s",texto_button,realjoystick_event_names[i]);
  	}
  }





  //real joystick buttons to keys
  for (i=0;i<MAX_KEYS_JOYSTICK;i++) {
  	if (realjoystick_keys_array[i].asignado.v) {
  		char texto_button[20];
  		int button_type;
  		z80_byte caracter;
  		caracter=realjoystick_keys_array[i].caracter;
  		button_type=realjoystick_keys_array[i].button_type;

		util_write_config_aux_realjoystick(button_type, realjoystick_keys_array[i].button, texto_button);

  		ADD_STRING_CONFIG,"--joystickkeybt %s %d",texto_button,caracter);
  	}
  }




  //joystickkeyev no lo estoy autoguardando, esto es mas indicado para archivos .config
  if (realjoystick_clear_keys_on_smartload.v) ADD_STRING_CONFIG,"--clearkeylistonsmart");



  //text osd keyboard
  for (i=0;i<osd_adv_kbd_defined;i++) {
          //Truco para poder poner " en el texto. Con barra invertida
          if (!strcmp(osd_adv_kbd_list[i],"\"")) ADD_STRING_CONFIG,"--text-keyboard-add \\");
	else ADD_STRING_CONFIG,"--text-keyboard-add \"%s\"",osd_adv_kbd_list[i]);
  }

                                        ADD_STRING_CONFIG,"--text-keyboard-length %d",adventure_keyboard_key_length);

     if (adventure_keyboard_send_final_spc) ADD_STRING_CONFIG,"--text-keyboard-finalspc");


  if (quickexit.v)                            ADD_STRING_CONFIG,"--quickexit");

  //Guardar si hay algo que Guardar
  if (indice_string) {
    char configfile[PATH_MAX];
  	 FILE *ptr_configfile;

     if (util_get_configfile_name(configfile)==0)  {
       debug_printf(VERBOSE_ERR,"Unable to find $HOME, or HOMEDRIVE or HOMEPATH environment variables to open configuration file");
       free(config_settings);
       return 0;
     }

     ptr_configfile=fopen(configfile,"a+");
     if (!ptr_configfile) {
                        debug_printf(VERBOSE_ERR,"Cannot write configuration file %s",configfile);
                        free(config_settings);
                        return 0;
      }

    fwrite(config_settings, 1, strlen(config_settings), ptr_configfile);


      fclose(ptr_configfile);
    }


    free(config_settings);

    return 1;

}

//Leer archivo de configuracion en buffer
//Devuelve 0 si no existe
//Detecta que no exceda el limite
int configfile_read(char *mem,int limite)
{



	char configfile[PATH_MAX];

  if (util_get_configfile_name(configfile)==0)  {
    printf("Unable to find $HOME, or HOMEDRIVE or HOMEPATH environment variables to open configuration file\n");
    return 0;
  }



	//Ver si archivo existe

	if (!si_existe_archivo(configfile)) {
		printf("Configuration file %s not found\nCreating a new one\n",configfile);

    if (util_create_sample_configfile(1)==0) return 0;

	}

        int tamanyo_archivo=get_file_size(configfile);

        if (tamanyo_archivo>limite) {
             cpu_panic("Configuration file larger than maximum allowed size. Exiting");
        }

	return configfile_read_aux(configfile,mem);

}

int devconfigfile_read(char *mem,int limite)
{



	char configfile[PATH_MAX];

  if (util_get_devconfigfile_name(configfile)==0)  {
    debug_printf(VERBOSE_INFO,"Unable to find $HOME, or HOMEDRIVE or HOMEPATH environment variables to open dev configuration file");
    return 0;
  }



	//Ver si archivo existe

	if (!si_existe_archivo(configfile)) {
		debug_printf(VERBOSE_INFO,"Additional configuration file %s not found",configfile);

                return 0;

	}

        int tamanyo_archivo=get_file_size(configfile);

        if (tamanyo_archivo>limite) {
             cpu_panic("Configuration file larger than maximum allowed size. Exiting");
        }

	return configfile_read_aux(configfile,mem);

}

//Retorna puntero a caracter leido que hace de final (sea final de linea o de archivo)
char *configfile_end_line(char *m)
{

	while ( (*m!='\n') && (*m!=0) ) {
		m++;
	}

	return m;
}

//Retorna puntero a caracter leido que hace de final (sea final de linea o de archivo)
char *configfile_next_field(char *m,int comillas_iniciales)
{

	//Hasta final de linea, o final de texto, o espacio (y no comilla previa), o comillas
       while (
		(*m!='\n') &&
		(*m!='\r') &&
		(*m!=0) &&
		(*m!='"')
	 ) {

		if (comillas_iniciales==0 && *m==' ') break;

                //Dejamos que se puedan escapar unas " si van precedidas de barra invertida
                //if ( (*m)=='\\') {
                //        if (*(m+1)=='"') m++;
                //}

                m++;
        }

        return m;
}


void configfile_parse_lines(char *mem,char *p_argv[],int *p_argc)
{
	char caracter;
	int argumentos=*p_argc;

	//para compatibilidad con rutinas de leer parametros, argv[0] no lo usamos
	p_argv[0]="";
	argumentos++;

	do {
		caracter=*mem;

		//Fin
		if (caracter==0) {
			*p_argc=argumentos;
			return;
		}

		//Comentarios
		if (caracter==';' || caracter=='#') {
			mem=configfile_end_line(mem);
			if (*mem) mem++;
		}

		//Estamos en salto de linea o espacio o retorno de carro, saltamos
		else if (caracter=='\n' || caracter==' ' || caracter=='\r' ) mem++;

		else {

			int comillas_iniciales=0;

			//Parametro correcto
			//Ver si comillas iniciales
			if (caracter=='"') {
				comillas_iniciales=1;
				mem++;
				//printf ("comillas iniciales en %s\n",mem);
			}

            //Indicar cada argumento con su puntero a memoria
			p_argv[argumentos]=mem;
			argumentos++;
                        if (argumentos>MAX_PARAMETERS_CONFIG_FILE) {
                                cpu_panic("Maximum parameters in config file reached");
                        }

			mem=configfile_next_field(mem,comillas_iniciales);

			//Poner codigo 0 al final
			if (*mem) {
				*mem=0;
				mem++;
			}
		}

	} while (1);

}


char *configfile_argv[MAX_PARAMETERS_CONFIG_FILE];
int configfile_argc=0;

//Convertir archivo de configuracion leido en argv
//Ignorar lineas que empiecen con ; o #
//Separar lineas y espacios en diferentes parametros; tener en cuenta texto que haya entre comillas, no separar por espacios
//Cada final de parametro tendra codigo 0
void configfile_parse(void)
{
    //Esta memoria no la liberamos nunca, pues se usara despues
    //en varios sitios, como en parse_cmdline_options(0), y tambien se dejan fijados punteros a muchos parametros
    //que se leen del archivo de config, como realtape_name=argv[puntero_parametro];
	char *mem_config=malloc(MAX_SIZE_CONFIG_FILE+1);



    if (mem_config==NULL) {
		cpu_panic("Unable to allocate memory for configuration file");
	}

	if (configfile_read(mem_config,MAX_SIZE_CONFIG_FILE)==0) {
		//No hay archivo de configuracion. Parametros vacios
		configfile_argv[0]="";
		configfile_argc=1;
		return;
	}


	configfile_parse_lines(mem_config,&configfile_argv[0],&configfile_argc);

	//Mostrar cada parametro. Ignoramos el primero (numero 0) pues es vacio
	if (debug_parse_config_file.v) {
		int i;
		for (i=1;i<configfile_argc;i++) {
			printf ("Debug: Configfile, parameter %d = [%s]\n",i,configfile_argv[i]);
		}
	}

	return;
}


char *devconfigfile_argv[MAX_PARAMETERS_CONFIG_FILE];
int devconfigfile_argc=0;


//Devuelve 0 si no existe el archivo o error
int devconfigfile_parse(void)
{
    //Esta memoria no la liberamos nunca, pues se usara despues
    //en varios sitios, como en parse_cmdline_options(0), y tambien se dejan fijados punteros a muchos parametros
    //que se leen del archivo de config, como realtape_name=argv[puntero_parametro];
	char *mem_devconfig=malloc(MAX_SIZE_CONFIG_FILE+1);



    if (mem_devconfig==NULL) {
		cpu_panic("Unable to allocate memory for configuration file");
	}

	if (devconfigfile_read(mem_devconfig,MAX_SIZE_CONFIG_FILE)==0) {
		//No hay archivo de configuracion. Parametros vacios
		return 0;
	}


	configfile_parse_lines(mem_devconfig,&devconfigfile_argv[0],&devconfigfile_argc);

        return 1;

}

char *custom_config_mem_pointer=NULL;

//Parsear archivo indicado de configuracion
void configfile_parse_custom_file(char *archivo)
{

	//Ver si hay que asignar memoria
	//Debe quedar esta memoria asignada al salir de este procedimiento, pues lo usa en parse_customfile_options
	if (custom_config_mem_pointer==NULL) {
		debug_printf (VERBOSE_DEBUG,"Allocating memory to read custom config file");
		custom_config_mem_pointer=malloc(MAX_SIZE_CONFIG_FILE+1);
	}

	else {
		debug_printf (VERBOSE_DEBUG,"No need to allocate memory to read custom config file, we allocated it before");
	}


        if (custom_config_mem_pointer==NULL) {
                cpu_panic("Unable to allocate memory for configuration file");
        }

	//Valor inicial
	configfile_argc=0;


        if (configfile_read_aux(archivo,custom_config_mem_pointer)==0) {
                //No hay archivo de configuracion. Parametros vacios
                configfile_argv[0]="";
                configfile_argc=1;

                return;
        }


        configfile_parse_lines(custom_config_mem_pointer,&configfile_argv[0],&configfile_argc);

        //Mostrar cada parametro. Ignoramos el primero (numero 0) pues es vacio
        if (debug_parse_config_file.v) {
                int i;
                for (i=1;i<configfile_argc;i++) {
                        printf ("Debug: Custom Configfile, parameter %d = [%s]\n",i,configfile_argv[i]);
                }
        }


        return;
}


//Parsear archivo de configuracion indicado, usado en insertar cinta, snap, etc
void parse_custom_file_config(char *archivo)
{

	configfile_parse_custom_file(archivo);
	argc=configfile_argc;
	argv=configfile_argv;
	puntero_parametro=0;

	parse_customfile_options();
}


tecla_redefinida lista_teclas_redefinidas[MAX_TECLAS_REDEFINIDAS];


void clear_lista_teclas_redefinidas(void)
{
	int i;
	for (i=0;i<MAX_TECLAS_REDEFINIDAS;i++) {
		lista_teclas_redefinidas[i].tecla_original=0;
	}
	debug_printf (VERBOSE_DEBUG,"Cleared redefined keys table");
}

//Poder redefinir una tecla. Retorna 0 si no hay redefinicion. Retorna 1 si lo hay
z80_byte util_redefinir_tecla(z80_byte tecla)
{

	int i;
	z80_byte tecla_original,tecla_redefinida;
	for (i=0;i<MAX_TECLAS_REDEFINIDAS;i++) {
		tecla_original=lista_teclas_redefinidas[i].tecla_original;
		tecla_redefinida=lista_teclas_redefinidas[i].tecla_redefinida;
		if (tecla_original) {
			if (tecla_original==tecla) return tecla_redefinida;
		}
	}

	return 0;


}

//Agregar una tecla a la lista de redefinicion. Retorna 0 si ok, 1 si error (lista llena por ejemplo)
int util_add_redefinir_tecla(z80_byte p_tecla_original, z80_byte p_tecla_redefinida)
{

	int i;
        z80_byte tecla_original;
	//z80_byte tecla_redefinida;

	for (i=0;i<MAX_TECLAS_REDEFINIDAS;i++) {
		tecla_original=lista_teclas_redefinidas[i].tecla_original;
                //tecla_redefinida=lista_teclas_redefinidas[i].tecla_redefinida;
		if (tecla_original==0) {
			lista_teclas_redefinidas[i].tecla_original=p_tecla_original;
			lista_teclas_redefinidas[i].tecla_redefinida=p_tecla_redefinida;
			debug_printf (VERBOSE_DEBUG,"Added key %d to be %d",p_tecla_original,p_tecla_redefinida);
			return 0;
		}
	}

	debug_printf (VERBOSE_ERR,"Maximum redefined keys (%d)",MAX_TECLAS_REDEFINIDAS);
	return 1;
}


void convert_numeros_letras_puerto_teclado_continue(z80_byte tecla,int pressrelease)
{


       //Si teclado recreated, y menu cerrado
       /*if (!menu_abierto && recreated_zx_keyboard_support.v) {

               enum util_teclas tecla_final;
               int pressrelease_final;
               //printf ("Entrada a convertir tecla recreated desde convert_numeros_letras_puerto_teclado_continue\n");
               recreated_zx_spectrum_keyboard_convert(tecla, &tecla_final, &pressrelease_final);
               if (tecla_final) {
               		//printf ("redefinicion de tecla. antes: %d despues: %d\n",tecla,tecla_final);
                       tecla=tecla_final;
                       pressrelease=pressrelease_final;

			if (tecla==UTIL_KEY_CAPS_SHIFT) {
				//printf ("Pulsada caps shift (%d)\n",UTIL_KEY_CAPS_SHIFT);
				util_set_reset_key(tecla,pressrelease);
				return;
			}

                       //Si sigue estando entre a-z y 0-9 enviar tal cual. Si no, llamar a util_set_reset_key
                       if (
                       	(tecla>='a' && tecla<='z') ||
                       	(tecla>='0' && tecla<='9')
                       	)
                       {
                       	//Nada. Prefiero escribir la condicion asi que no poner un negado
                       	//printf("Tecla final es entre az y 09. generar puerto spectrum\n");
                       }


                       else {
                       	//util_set_reset_key(tecla,pressrelease);
                       	//printf ("Enviar a util_set_reset_key_convert_recreated_yesno sin convertir\n");
                       	util_set_reset_key_convert_recreated_yesno(tecla,pressrelease,0);
                       	return;
                       }
               }
       }*/

       convert_numeros_letras_puerto_teclado_continue_after_recreated(tecla,pressrelease);
}



void convert_numeros_letras_puerto_teclado_continue_after_recreated(z80_byte tecla,int pressrelease)
{

	//Redefinicion de teclas
	z80_byte tecla_redefinida;
	tecla_redefinida=util_redefinir_tecla(tecla);
	if (tecla_redefinida) tecla=tecla_redefinida;

  if (tecla>='a' && tecla<='z') {
      int indice=tecla-'a';

      z80_byte *puerto;
      puerto=tabla_teclado_letras[indice].puerto;

      z80_byte mascara;
      mascara=tabla_teclado_letras[indice].mascara;

                                        //printf ("asignamos tecla mediante array indice=%d puerto=%x mascara=%d - ",indice,puerto,mascara);

      if (pressrelease) *puerto &=255-mascara;
      else *puerto |=mascara;


        if (MACHINE_IS_Z88) {
                puerto=z88_tabla_teclado_letras[indice].puerto;
                mascara=z88_tabla_teclado_letras[indice].mascara;

                if (pressrelease) *puerto &=255-mascara;
                else *puerto |=mascara;
        }

        if (MACHINE_IS_CPC) {
                puerto=cpc_tabla_teclado_letras[indice].puerto;
                mascara=cpc_tabla_teclado_letras[indice].mascara;

                if (pressrelease) *puerto &=255-mascara;
                else *puerto |=mascara;
         }

        if (MACHINE_IS_PCW) {
                puerto=pcw_tabla_teclado_letras[indice].puerto;
                mascara=pcw_tabla_teclado_letras[indice].mascara;

                if (pressrelease) *puerto &=255-mascara;
                else *puerto |=mascara;

         }

        if (MACHINE_IS_MSX) {
                puerto=msx_tabla_teclado_letras[indice].puerto;
                mascara=msx_tabla_teclado_letras[indice].mascara;

                if (pressrelease) *puerto &=255-mascara;
                else *puerto |=mascara;
         }

        if (MACHINE_IS_SVI) {
                puerto=svi_tabla_teclado_letras[indice].puerto;
                mascara=svi_tabla_teclado_letras[indice].mascara;

                if (pressrelease) *puerto &=255-mascara;
                else *puerto |=mascara;
         }

          if (MACHINE_IS_QL) {
            puerto=ql_tabla_teclado_letras[indice].puerto;
            mascara=ql_tabla_teclado_letras[indice].mascara;

            if (pressrelease) *puerto &=255-mascara;
            else *puerto |=mascara;
          }

          if (MACHINE_IS_MK14) {
            //Solo algunas letras
            /*
            mk14_keystatus

               Matriz teclado
               128 64  32  16

                0  8   -   A	mk14_keystatus[0]
            	1  9   -   B	mk14_keystatus[1]
            	2  -   GO  C	mk14_keystatus[2]
            	3  -   MEM D	mk14_keystatus[3]
            	4  -   ABR -	mk14_keystatus[4]
            	5  -   -   -	mk14_keystatus[5]
            	6  -   -   E	mk14_keystatus[6]
            	7  -   TER F	mk14_keystatus[7]

            GO: mapeado a G
            MEM: mapeado a M
            ABR: mapeado a Z
            TERM: mapeado a T

            //El MK14 usa los 4 bits superiores de cada mk14_keystatus para almacenar el estado de las teclas
            //pero yo lo almaceno en los 4 bits inferiores porque me facilita la vida tenerlo ahi en la ventana de Keyboard Help
            */

            mascara=255;

            if (tecla=='a') {
              puerto=&mk14_keystatus[0];
              mascara=16>>4;
            }

            if (tecla=='b') {
              puerto=&mk14_keystatus[1];
              mascara=16>>4;
            }

            if (tecla=='c') {
              puerto=&mk14_keystatus[2];
              mascara=16>>4;
            }

            if (tecla=='d') {
              puerto=&mk14_keystatus[3];
              mascara=16>>4;
            }

            if (tecla=='e') {
              puerto=&mk14_keystatus[6];
              mascara=16>>4;
            }

            if (tecla=='f') {
              puerto=&mk14_keystatus[7];
              mascara=16>>4;
            }

            if (tecla=='g') {
              puerto=&mk14_keystatus[2];
              mascara=32>>4;
            }

            if (tecla=='m') {
              puerto=&mk14_keystatus[3];
              mascara=32>>4;
            }

            if (tecla=='z') {
              puerto=&mk14_keystatus[4];
              mascara=32>>4;
            }

            if (tecla=='t') {
              puerto=&mk14_keystatus[7];
              mascara=32>>4;
            }


            if (mascara!=255) {
              if (pressrelease) *puerto &=255-mascara;
              else *puerto |=mascara;
            }
          }


    }

  if (tecla>='0' && tecla<='9') {
  int indice=tecla-'0';

  z80_byte *puerto;
  puerto=tabla_teclado_numeros[indice].puerto;

  z80_byte mascara;
  mascara=tabla_teclado_numeros[indice].mascara;

                                        //printf ("asignamos tecla mediante array indice=%d puerto=%x mascara=%d - ",indice,puerto,mascara);

                                        if (pressrelease) *puerto &=255-mascara;
                                        else *puerto |=mascara;



  if (MACHINE_IS_Z88) {
                                                puerto=z88_tabla_teclado_numeros[indice].puerto;
                                                mascara=z88_tabla_teclado_numeros[indice].mascara;

                                                if (pressrelease) *puerto &=255-mascara;
                                                else *puerto |=mascara;
  }

	if (MACHINE_IS_CPC) {
                                                puerto=cpc_tabla_teclado_numeros[indice].puerto;
                                                mascara=cpc_tabla_teclado_numeros[indice].mascara;

                                                if (pressrelease) *puerto &=255-mascara;
                                                else *puerto |=mascara;
  }

	if (MACHINE_IS_PCW) {
                                                puerto=pcw_tabla_teclado_numeros[indice].puerto;
                                                mascara=pcw_tabla_teclado_numeros[indice].mascara;

                                                if (pressrelease) *puerto &=255-mascara;
                                                else *puerto |=mascara;

  }

	if (MACHINE_IS_MSX) {
                                                puerto=msx_tabla_teclado_numeros[indice].puerto;
                                                mascara=msx_tabla_teclado_numeros[indice].mascara;

                                                if (pressrelease) *puerto &=255-mascara;
                                                else *puerto |=mascara;
  }

	if (MACHINE_IS_SVI) {
                                                puerto=svi_tabla_teclado_numeros[indice].puerto;
                                                mascara=svi_tabla_teclado_numeros[indice].mascara;

                                                if (pressrelease) *puerto &=255-mascara;
                                                else *puerto |=mascara;
  }

  if (MACHINE_IS_QL) {
                                          puerto=ql_tabla_teclado_numeros[indice].puerto;
                                          mascara=ql_tabla_teclado_numeros[indice].mascara;

                                          if (pressrelease) *puerto &=255-mascara;
                                          else *puerto |=mascara;
  }

  if (MACHINE_IS_MK14) {
                                                puerto=mk14_tabla_teclado_numeros[indice].puerto;
                                                mascara=mk14_tabla_teclado_numeros[indice].mascara;

                                                if (pressrelease) *puerto &=255-mascara;
                                                else *puerto |=mascara;
  }


 }

}




void insert_tape_cmdline(char *s)
{
        tapefile=s;
}


void insert_snap_cmdline(char *s)
{
        snapfile=s;
}

//retorna 1 si ok
int quickload_valid_extension(char *nombre) {
	if (
                   !util_compare_file_extension(nombre,"zx")
                || !util_compare_file_extension(nombre,"sp")
                || !util_compare_file_extension(nombre,"zsf")
                || !util_compare_file_extension(nombre,"spg")
                || !util_compare_file_extension(nombre,"nex")
                || !util_compare_file_extension(nombre,"trd")
                || !util_compare_file_extension(nombre,"mdr")
                || !util_compare_file_extension(nombre,"rmd")
                || !util_compare_file_extension(nombre,"ddh")
                || !util_compare_file_extension(nombre,"dsk")
                || !util_compare_file_extension(nombre,"z80")
                || !util_compare_file_extension(nombre,"tzx")
                || !util_compare_file_extension(nombre,"pzx")
                || !util_compare_file_extension(nombre,"sna")
                || !util_compare_file_extension(nombre,"snx")
                || !util_compare_file_extension(nombre,"tap")

                || !util_compare_file_extension(nombre,"smp")
                || !util_compare_file_extension(nombre,"wav")
                || !util_compare_file_extension(nombre,"rwa")


                || !util_compare_file_extension(nombre,"o")
                || !util_compare_file_extension(nombre,"80")
                || !util_compare_file_extension(nombre,"p")
                || !util_compare_file_extension(nombre,"81")
                || !util_compare_file_extension(nombre,"p81")
                || !util_compare_file_extension(nombre,"z81")
                || !util_compare_file_extension(nombre,"epr")
                || !util_compare_file_extension(nombre,"63")
                || !util_compare_file_extension(nombre,"eprom")
		|| !util_compare_file_extension(nombre,"flash")
		|| !util_compare_file_extension(nombre,"ace")
		|| !util_compare_file_extension(nombre,"dck")
		|| !util_compare_file_extension(nombre,"cdt")
    || !util_compare_file_extension(nombre,"ay")
    || !util_compare_file_extension(nombre,"scr")
    || !util_compare_file_extension(nombre,"rzx")
    || !util_compare_file_extension(nombre,"rom")
    || !util_compare_file_extension(nombre,"col")
    || !util_compare_file_extension(nombre,"sg")
    || !util_compare_file_extension(nombre,"sms")
    || !util_compare_file_extension(nombre,"bin")
    || !util_compare_file_extension(nombre,"sc")
    || !util_compare_file_extension(nombre,"cas")

    || !util_compare_file_extension(nombre,"zmenu")
	) {
		return 1;
	}


	return 0;


}


//Indica que quickload esta adivinando tipo tzx (si real o standard)
//Se lee desde rutina tape_tzx para meter error en pantalla o no
z80_bit quickload_guessing_tzx_type={0};

void quickload_standard_tape(char *nombre)
{
                if (noautoload.v==0 && !MACHINE_IS_TBBLUE) { //TODO: desactivamos autoload en TBBLUE

			//De momento sam coupe solo carga en real tape
			if (MACHINE_IS_SAM) {
		                realtape_name=nombre;
		                realtape_insert();
		                return;

			}

			//Extension tap tambien es de jupiter ace
			if (!MACHINE_IS_ACE) {

	                        //Si no estamos en spectrum, cambiar maquina
        	                if (!(MACHINE_IS_SPECTRUM)) {
                	                //spectrum 128k
                        	        current_machine_type=6;
                                	set_machine(NULL);
	                                //el reset machine viene desde tape_init
        	                }
			}

                        //establecer parametros por defecto
                        set_machine_params();
                }

                insert_tape_cmdline(nombre);
                tape_init();


}

void quickload_real_tape(char *nombre)
{
                if (noautoload.v==0) {
                        //Si no estamos en spectrum, cambiar maquina
                        if (!(MACHINE_IS_SPECTRUM)) {
                                //spectrum 128k
                                current_machine_type=6;
                                set_machine(NULL);
                                //el reset machine viene desde tape_init
                        }

                        //establecer parametros por defecto
                        set_machine_params();
                }

                realtape_name=nombre;

                realtape_insert();

}

void quickload_common_set_spectrum(void)
{
                if (noautoload.v==0) {
                        //Si no estamos en spectrum, cambiar maquina
                        if (!(MACHINE_IS_SPECTRUM)) {
                                //spectrum 128k
                                current_machine_type=6;
                                set_machine(NULL);
                                //el reset machine viene desde tape_init
                        }

                        //establecer parametros por defecto
                        set_machine_params();
                }

}

//Funcionamiento de quickload:
//Desactivar realvideo
//Limpiar asignacion de joystick a teclas (pero no de eventos), si hay opcion de que lo haga
//Si es snapshot, detecta parametros de ese snap(mediante ssl) y carga el snapshot con los parametros de maquina indicados en el archivo snap
//Si es cinta:
//Si tiene autoload:
//  Si es cinta de spectrum y maquina seleccionada no es spectrum, reset a spectrum 128k
//  si es cinta de zx80 o zx81, cambiar a maquina zx80 o zx81 correspondiente
//  establecer parametros por defecto de la maquina actual
//Insertar cinta. Detectar parametros por defecto de esa cinta(mediante ssl) y detectar tambien si se debe ejecutar en maquina concreta, como Robocop2
//Si tiene autoload, resetear antes maquina para que se pueda ejecutar comando LOAD

//En el caso de Z88, si esta en Z88 activo, se inserta cartucho en slot 1 sin alterar resto de slots. Si no esta Z88 activo, se cambia a Z88

int quickload_continue(char *nombre) {

	//Si hay cinta realtape insertada, quitarla
    if (realtape_inserted.v) realtape_eject();

	//Si hay cartucho de timex insertado, expulsarlo
	//Realmente no comprobamos si hay cartucho pues no hay ningun flag que indique insertado o no. Simplemente liberamos memoria dock
	if (MACHINE_IS_TIMEX_TS_TC_2068) {
		timex_empty_dock_space();
	}

	//Quitar top speed si es que estabamos cargando algo antes a maxima velocidad
	top_speed_timer.v=0;

	//Poner modo turbo a 1. En Z88 no hacer esto, pues desasigna los slots , porque llama a set_machine_params y este a init_z88_memory_slots
	if (!MACHINE_IS_Z88 && cpu_turbo_speed!=1) {
		cpu_turbo_speed=1;
		cpu_set_turbo_speed();
	}



#ifdef EMULATE_RASPBERRY
	//desactivar realvideo por defecto en raspberry. Si hace falta, el realvideo se activara desde el archivo snapshot o en la deteccion por ssl
	if (cambio_parametros_maquinas_lentas.v==0) {
		debug_printf (VERBOSE_INFO,"Parameter changeslowparameters not enabled. Do not change any frameskip or realvideo parameters");
	}
    else {
	    disable_rainbow();
    }
#endif

	//expulsar cinta insertada
	eject_tape_load();
	tapefile=NULL;


	if (realjoystick_clear_keys_on_smartload.v) {
	        //Eliminar asignacion de eventos joystick a teclas.
		realjoystick_clear_keys_array();
	}

    if (
            !util_compare_file_extension(nombre,"zx")
        || !util_compare_file_extension(nombre,"sp")
        || !util_compare_file_extension(nombre,"zsf")
        || !util_compare_file_extension(nombre,"nex")
        || !util_compare_file_extension(nombre,"spg")
        || !util_compare_file_extension(nombre,"z80")
        || !util_compare_file_extension(nombre,"sna")
        || !util_compare_file_extension(nombre,"snx")
        || !util_compare_file_extension(nombre,"ace")
        || !util_compare_file_extension(nombre,"rzx")
    ) {


        insert_snap_cmdline(nombre);
        snapshot_load();

        return 0;
    }

    //cinta de spectrum tzx, la mayoria (excepto rwa)
    else if (
                !util_compare_file_extension(nombre,"tzx")

    ) {

        debug_printf (VERBOSE_INFO,"Smartload. Guessing if standard tzx tape or real tape");

        //Ver si la cinta es estandard (ver si todos los tags id tzx son reconocidos)
        //o meterla como real tape

        //Mete la cinta como standard tape. Si da error de tzx id no reconocido, la meteremos como real tape

        insert_tape_cmdline(nombre);
        tape_init();

        //Leer hasta final
        char buffer_temp[65536];
        int final=0;
        quickload_guessing_tzx_type.v=1;
        while (!final) {
            z80_int cinta_longitud;
                        cinta_longitud=tape_block_readlength();
                        if (cinta_longitud==0) final=1;
            else {
                tape_block_read(buffer_temp,cinta_longitud);
            }
        }

        //expulsar cinta insertada
        eject_tape_load();
        tapefile=NULL;

        quickload_guessing_tzx_type.v=0;

        debug_printf (VERBOSE_INFO,"Smartload. End guessing process");

        if (tzx_read_returned_unknown_id.v) {
            debug_printf (VERBOSE_INFO,"Tzx tape will be loaded as real tape");
            quickload_real_tape(nombre);
        }

        else {
            debug_printf (VERBOSE_INFO,"Tzx tape will be loaded as standard tape");
            quickload_standard_tape(nombre);
        }

        return 0;
    }

    //cinta de spectrum pzx
    else if (
                !util_compare_file_extension(nombre,"pzx")

    ) {

        debug_printf (VERBOSE_INFO,"Smartload. Guessing if standard pzx tape or real tape");

        //Ver si la cinta es estandard
        //o meterla como real tape



        insert_tape_cmdline(nombre);
        tape_init();

        int es_cinta_estandard=tape_pzx_see_if_standard_tape();

        //expulsar cinta insertada
        eject_tape_load();
        tapefile=NULL;


        if (!es_cinta_estandard) {
            debug_printf (VERBOSE_INFO,"PZX tape will be loaded as real tape");
            //printf ("PZX tape will be loaded as real tape\n");
            quickload_real_tape(nombre);
        }

        else {
            debug_printf (VERBOSE_INFO,"PZX tape will be loaded as standard tape");
            //printf("PZX tape will be loaded as standard tape\n");
            quickload_standard_tape(nombre);
        }

        return 0;
    }


//cintas de spectrum, la mayoria (excepto rwa)
    else if (
                !util_compare_file_extension(nombre,"tap")
            || !util_compare_file_extension(nombre,"rwa")

    ) {

        quickload_standard_tape(nombre);

        return 0;
    }

//cintas de msx
    else if (
                !util_compare_file_extension(nombre,"cas")


    ) {

        //Aqui el autoload da igual. cambiamos siempre a msx si conviene
        if (!MACHINE_IS_MSX && !MACHINE_IS_SVI) {
            current_machine_type=MACHINE_ID_MSX1;
            set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }

        insert_tape_cmdline(nombre);
        tape_init();

        return 0;
    }


//wav, smp y pzx suponemos real audio tape Spectrum
    else if (
                !util_compare_file_extension(nombre,"wav")
            || !util_compare_file_extension(nombre,"smp")
            || !util_compare_file_extension(nombre,"pzx")

    ) {
        quickload_real_tape(nombre);

        return 0;
    }




    //cintas de zx80, las cargamos como cintas
    else if (
                !util_compare_file_extension(nombre,"o")
            || !util_compare_file_extension(nombre,"80")

    ) {


        if (noautoload.v==0) {
        //Si no estamos en zx80, cambiar maquina
            if (!(MACHINE_IS_ZX80_TYPE)) {
                    current_machine_type=120;
                    set_machine(NULL);
            }

            //establecer parametros por defecto
            set_machine_params();
        }


        insert_tape_cmdline(nombre);
        tape_init();

        return 0;
    }

    //cintas de zx81, las cargamos como cintas
    else if (
                !util_compare_file_extension(nombre,"p")
            || !util_compare_file_extension(nombre,"81")
            || !util_compare_file_extension(nombre,"p81")
            || !util_compare_file_extension(nombre,"z81")

    ) {

        if (noautoload.v==0) {
            //Si no estamos en zx81, cambiar maquina
            if (!(MACHINE_IS_ZX81_TYPE)) {
                    current_machine_type=121;
                    set_machine(NULL);
                    //el reset machine viene desde tape_init
            }

            //establecer parametros por defecto
            set_machine_params();
        }


        insert_tape_cmdline(nombre);
        tape_init();

        return 0;
    }

	//dock cartridges de Timex
	else if (
		!util_compare_file_extension(nombre,"dck")
	) {
		//Aqui el autoload da igual. cambiamos siempre a timex si conviene
		if (!MACHINE_IS_TIMEX_TS_TC_2068) {
			current_machine_type=MACHINE_ID_TIMEX_TS2068;
			set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }

		timex_insert_dck_cartridge(nombre);

		return 0;

	}

	//TRD
	else if (
		!util_compare_file_extension(nombre,"trd")
	) {
		//Aqui el autoload da igual. cambiamos siempre a Pentagon si conviene
		if (!MACHINE_IS_SPECTRUM) {
			current_machine_type=MACHINE_ID_PENTAGON;
			set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();
            reset_cpu();


        }

        betadisk_enable();
        trd_insert_disk(nombre);


		return 0;

	}

	//DDH
	else if (
        !util_compare_file_extension(nombre,"ddh")
	) {
		//Aqui el autoload da igual. cambiamos siempre a Spectrum si conviene
		if (!MACHINE_IS_SPECTRUM) {
			current_machine_type=MACHINE_ID_SPECTRUM_48;
			set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();
            reset_cpu();

        }

        hilow_disable();

        strcpy(hilow_file_name,nombre);

        hilow_enable();

		return 0;

	}

	//MDR y RMD
	else if (
		!util_compare_file_extension(nombre,"mdr") ||
        !util_compare_file_extension(nombre,"rmd")
	) {
		//Aqui el autoload da igual. cambiamos siempre a Spectrum si conviene
		if (!MACHINE_IS_SPECTRUM) {
			current_machine_type=MACHINE_ID_SPECTRUM_48;
			set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();
            reset_cpu();

        }

        enable_if1();

        microdrive_eject(0);

        strcpy(microdrive_status[0].microdrive_file_name,nombre);

        microdrive_insert(0);


		return 0;

	}


	//DSK
	else if (
		!util_compare_file_extension(nombre,"dsk")
	) {
		//Aqui el autoload da igual. cambiamos siempre a P3 si conviene
		if (!MACHINE_IS_SPECTRUM_P3 && !MACHINE_IS_CPC_HAS_FLOPPY && !MACHINE_IS_PCW) {
			current_machine_type=MACHINE_ID_SPECTRUM_P3_40;
			set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();
            reset_cpu();


        }

        dskplusthree_disable();
        dsk_insert_disk(nombre);

        set_snaptape_fileoptions(nombre);

        //strcpy(dskplusthree_file_name,nombre);

        dskplusthree_enable();

        //Habilitar pd765 a no ser que los traps esten activados
        if (MACHINE_IS_SPECTRUM_P3) {
            if (plus3dos_traps.v==0) pd765_enable();
        }
        else {
            pd765_enable();
        }
		//plus3dos_traps.v=1;



		return 0;

	}


    //Archivos ay
    else if (
        !util_compare_file_extension(nombre,"ay")
    ) {
        ay_player_add_file(nombre);
        //ay_player_load_and_play(nombre);
        return 0;

    }

    else if (
        !util_compare_file_extension(nombre,"scr")
    ) {
        quickload_common_set_spectrum();
        load_screen(nombre);
        return 0;

    }


	//cintas de CPC
	else if (
                !util_compare_file_extension(nombre,"cdt")
        ) {
		//Aqui el autoload da igual. cambiamos siempre a cpc si conviene
            if (!MACHINE_IS_CPC) {
			    current_machine_type=140;
                set_machine(NULL);

                    //establecer parametros por defecto. Incluido quitar slots de memoria
                set_machine_params();

                reset_cpu();
            }

            realtape_name=nombre;

            realtape_insert();

            return 0;

        }

	//Archivos .rom
	else if (
            !util_compare_file_extension(nombre,"rom")
        ) {


        //En MSX o SVI, cargar la rom como un cartucho de juegos
        if (MACHINE_IS_MSX) msx_insert_rom_cartridge(nombre);
        else if (MACHINE_IS_SVI) svi_insert_rom_cartridge(nombre);
        else {
            //En cualquier otro, cargar la rom como la rom de la maquina (como seleccionar custom machine pero sin dejar la rom definida para siempre)
            set_machine(nombre);
            cold_start_cpu_registers();
            reset_cpu();
        }


        return 0;

    }

	//Cartuchos de Coleco
	else if (
            !util_compare_file_extension(nombre,"col")
        ) {
		//Aqui el autoload da igual. cambiamos siempre a coleco si conviene
        if (!MACHINE_IS_COLECO) {
			current_machine_type=MACHINE_ID_COLECO;
            set_machine(NULL);

                //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }

        coleco_insert_rom_cartridge(nombre);


        return 0;

    }

	//Cartuchos de SG1000
	else if (
                !util_compare_file_extension(nombre,"sg")
             || !util_compare_file_extension(nombre,"sc")
        ) {
		//Aqui el autoload da igual. cambiamos siempre a sg1000 si conviene
        if (!MACHINE_IS_SG1000 && !MACHINE_IS_SMS) {
            current_machine_type=MACHINE_ID_SG1000;
            set_machine(NULL);

                //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }

        sg1000_insert_rom_cartridge(nombre);


        return 0;

    }

    //Cartuchos de SMS
	else if (
                !util_compare_file_extension(nombre,"sms")
             || !util_compare_file_extension(nombre,"bin")

        ) {
		//Aqui el autoload da igual. cambiamos siempre a sms si conviene
        if (!MACHINE_IS_SMS) {
			current_machine_type=MACHINE_ID_SMS;
            set_machine(NULL);

                //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }

        sms_insert_rom_cartridge(nombre);


        return 0;

    }

	//eprom cards de Z88
    else if (
            !util_compare_file_extension(nombre,"epr")
		|| !util_compare_file_extension(nombre,"63")
		|| !util_compare_file_extension(nombre,"eprom")

        ) {

		//Aqui el autoload da igual. cambiamos siempre a Z88 si conviene

        //Si no estamos en z88, cambiar maquina
        if (!(MACHINE_IS_Z88)) {
            current_machine_type=130;
            set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }


		//Insertar rom en slot libre
		debug_printf (VERBOSE_INFO,"Finding free slots");

		int slot;
		for (slot=1;slot<=3 && z88_memory_slots[slot].size!=0;slot++);


		//si no hay ninguno libre, meterlo en slot 1
		if (slot==4) {
			//debug_printf (VERBOSE_INFO,"No free slots. Using slot 1");
			debug_printf (VERBOSE_ERR,"No free slots to use. Inserting new card on slot 1");
			slot=1;
		}

		z88_load_eprom_card(nombre,slot);


        return 0;
    }


	//flash cards de Z88
    else if (
                   !util_compare_file_extension(nombre,"flash")

        ) {

        //Aqui el autoload da igual. cambiamos siempre a Z88 si conviene

        //Si no estamos en z88, cambiar maquina
        if (!(MACHINE_IS_Z88)) {
            current_machine_type=130;
            set_machine(NULL);

            //establecer parametros por defecto. Incluido quitar slots de memoria
            set_machine_params();

            reset_cpu();
        }

        //Insertar flash en slot 3
        z88_load_flash_intel_card(nombre,3);


        return 0;
    }


	//Archivos de menus
    else if (
                   !util_compare_file_extension(nombre,"zmenu")

        ) {

        strcpy(menu_open_zmenu_file_path,nombre);

        if (menu_abierto) {
            menu_event_pending_zmenu_file_menu_open.v=1;
        }

        menu_event_open_zmenu_file.v=1;

        menu_set_menu_abierto(1);


        return 0;
    }


    //Error. no reconocido
    return 1;

}



int quickload(char *nombre) {

	//Recordar estado de algunos interfaces para activarlos despues
	z80_bit antes_divmmc_diviface_enabled;
	antes_divmmc_diviface_enabled.v=divmmc_diviface_enabled.v;

	z80_bit antes_divide_diviface_enabled;
	antes_divide_diviface_enabled.v=divide_diviface_enabled.v;

	z80_bit antes_superupgrade_enabled;
	antes_superupgrade_enabled.v=superupgrade_enabled.v;

    z80_bit antes_debug_breakpoints_enabled;
    antes_debug_breakpoints_enabled.v=debug_breakpoints_enabled.v;

	z80_bit antes_cpu_history_enabled;
	antes_cpu_history_enabled.v=cpu_history_enabled.v;


	int retorno=quickload_continue(nombre);

	//En tbblue, setting de divmmc desactivado se gestiona mediante bit de registro
	/*
	z80_byte diven=tbblue_registers[6]&16;
	*/
	if (MACHINE_IS_TBBLUE) {
		if ((tbblue_registers[6]&16)==0) antes_divmmc_diviface_enabled.v=0;
	}

    if (antes_divmmc_diviface_enabled.v)  {
        debug_printf (VERBOSE_DEBUG,"Reenabling divmmc as it was enabled before quickload");
        divmmc_diviface_disable();
        divmmc_diviface_enable();
    }

    if (antes_divide_diviface_enabled.v)  {
        debug_printf (VERBOSE_DEBUG,"Reenabling divide as it was enabled before quickload");
        divide_diviface_disable();
        divide_diviface_enable();
    }

	if (antes_superupgrade_enabled.v && superupgrade_enabled.v==0) {
		debug_printf (VERBOSE_DEBUG,"Reenabling superupgrade as it was enabled before quickload");
		superupgrade_enable(0);
	}

	if (antes_cpu_history_enabled.v) set_cpu_core_history_enable();

    //Si estaba modo debug cpu, reactivar
    //Asegurarme que no este ya habilitado debug_breakpoints_enabled.v, puede suceder simplemente
    //si hacemos smartload y no tenemos habilitado el autoload medium, con lo que no resetea la maquina y no se desactivan los breakpoints
    //y entonces se habilitaria breakpoints con los breakpoints y activados con lo que genera segfault
    //porque las rutinas de core breakpoints se llaman a si mismas en bucle
    if (antes_debug_breakpoints_enabled.v && debug_breakpoints_enabled.v==0) {
        debug_printf(VERBOSE_DEBUG,"Re-enabling breakpoints because they were enabled before quickload");
        breakpoints_enable();
    }

	return retorno;

}


//Retorna 1 si existe archivo o directorio
//0 si no
int si_existe_archivo(char *nombre)
{

    //printf("si_existe_archivo %s\n",nombre);

    if (util_path_is_mmc_fatfs(nombre)) {
        //printf("si_existe_archivo for %s using FatFS\n",nombre);
        FRESULT fr;
        FILINFO fno;


        //printf("Test for 'file.txt'...\n");

        fr = f_stat(nombre, &fno);
        if (fr==FR_OK) {
            return 1;
        }

        //desconocido
        else return 0;
    }

    else {
                //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(nombre, &buf_stat)==0) {

			return 1;

                }

	return 0;
    }

}

//Retorna sufijo y unidades para un tamaño de bytes
//B esta con espacios para quede alineado con el resto
long long int get_size_human_friendly(long long int tamanyo,char *sufijo)
{
    strcpy(sufijo,"B  ");

    //kibibyte (KiB)
    if (tamanyo >= 1024) {
        tamanyo /= 1024;
        strcpy(sufijo,"KiB");
    }

    if (tamanyo >= 1024) {
        tamanyo /= 1024;
        strcpy(sufijo,"MiB");
    }

    if (tamanyo >= 1024) {
        tamanyo /= 1024;
        strcpy(sufijo,"GiB");
    }

    if (tamanyo >= 1024) {
        tamanyo /= 1024;
        strcpy(sufijo,"TiB");
    }

    return tamanyo;
}


//Retorna valor, sufijo y unidades para un tamaño de bits per second
void get_size_bps_human_friendly(long long int tamanyo,char *texto)
{

    char sufijo[10];

    tamanyo *=8;

    strcpy(sufijo,"bps");

    if (tamanyo >= 1000) {
        tamanyo /= 1000;
        strcpy(sufijo,"Kbps");
    }

    if (tamanyo >= 1000) {
        tamanyo /= 1000;
        strcpy(sufijo,"Mbps");
    }

    if (tamanyo >= 1000) {
        tamanyo /= 1000;
        strcpy(sufijo,"Gbps");
    }

    if (tamanyo >= 1000) {
        tamanyo /= 1000;
        strcpy(sufijo,"Tbps");
    }

    sprintf(texto,"%lld %s",tamanyo,sufijo);

}

//Retorna tamanyo archivo
long long int get_file_size(char *nombre)
{

    //Si es archivo de la mmc
    //if (util_path_is_prefix_mmc_fatfs(nombre)) {
    if (util_path_is_mmc_fatfs(nombre)) {
        //printf("get_file_size for %s using FatFS\n",nombre);
        FRESULT fr;
        FILINFO fno;


        //printf("Test for 'file.txt'...\n");

        fr = f_stat(nombre, &fno);
        if (fr==FR_OK) {
            return fno.fsize;
        }

        //desconocido
        else return 0;
    }

    else {


#if __MINGW32__
        //Para que funcione la llamada en windows 32 bit. Si no, trata tamanyos de archivos con enteros de 32 bits
        //y mostrara mal cualquier tamanyo mayor de 2 GB
        struct __stat64 buf_stat;
        if (_stat64(nombre, &buf_stat) != 0) {
#else

        struct stat buf_stat;
        if (stat(nombre, &buf_stat)!=0) {
#endif
            debug_printf(VERBOSE_INFO,"Unable to get status of file %s",nombre);
            return 0;
        }

        else {
			//printf ("file size: %ld\n",buf_stat.st_size);
			return buf_stat.st_size;
        }
    }
}





//Retorna numero lineas archivo.
int get_file_lines(char *filename)
{

	int leidos;
        int total_lineas=0;

        //Leemos primero todo el archivo en memoria
        long long int tamanyo_archivo=get_file_size(filename);

        z80_byte *buffer_memoria;
        z80_byte *buffer_memoria_copia;

        if (tamanyo_archivo>0) {
                //printf ("asignando memoria\n");
                buffer_memoria=malloc(tamanyo_archivo);
                if (buffer_memoria==NULL) cpu_panic("Can not allocate memory for counting file lines");
                buffer_memoria_copia=buffer_memoria;
        }

        FILE *ptr_archivo;
        ptr_archivo=fopen(filename,"rb");
        if (!ptr_archivo) {
                debug_printf (VERBOSE_DEBUG,"Can not open %s",filename);
                return 0;
        }

        leidos=fread(buffer_memoria,1,tamanyo_archivo,ptr_archivo);
        fclose(ptr_archivo);

        /*
        Nota: leyendo de fread todo de golpe, o byte a byte, suele tardar lo mismo, EXCEPTO, en el caso que el archivo este en cache
        del sistema operativo, parece que leer byte a byte no tira de cache. En cambio, leyendo todo de golpe, si que usa la cache
        Ejemplo:
        Leer archivo de 371 MB. lineas total: 14606365
        25 segundos aprox en leer y contar lineas, usando fread byte a byte
        Si uso fread leyendo todo de golpe, tarda menos de 1 segundo
        */

        z80_byte byte_leido;

        while (leidos>0) {
                byte_leido=*buffer_memoria_copia;
                buffer_memoria_copia++;
                leidos--;

                if (byte_leido=='\n') total_lineas++;

                //if (leidos<5) printf ("leyendo de memoria\n");
        }

        if (tamanyo_archivo>0) {
                //printf ("liberando memoria\n");
                free(buffer_memoria);
        }

        //printf ("lineas total: %d\n",total_lineas);

	return total_lineas;

}



//Retorna -1 si hay algun error
//retorna bytes leidos si ok
int lee_archivo(char *nombre,char *buffer,int max_longitud)
{

	int leidos;

                FILE *ptr_archivo;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(nombre,&in_fatfs,&ptr_archivo,&fil)<0) {
        debug_printf (VERBOSE_DEBUG,"Can not open %s",nombre);
        return -1;
    }

    /*
                ptr_archivo=fopen(nombre,"rb");
                if (!ptr_archivo) {
                        debug_printf (VERBOSE_DEBUG,"Can not open %s",nombre);
                        return -1;
                }
    */
                leidos=zvfs_fread(in_fatfs,(z80_byte *)buffer,max_longitud,ptr_archivo,&fil);
                //leidos=fread(buffer,1,max_longitud,ptr_archivo);

                zvfs_fclose(in_fatfs,ptr_archivo,&fil);
                //fclose(ptr_archivo);


		return leidos;

}

/*
void util_set_reset_key_msx_keymap(enum util_teclas_msx_keymap tecla,int pressrelease)
{
        switch (tecla) {
			case UTIL_KEY_MSX_MINUS:
                                if (pressrelease) {
                                        msx_keyboard_table[1] &=255-4;
                                }
                                else {
                                        msx_keyboard_table[1] |=4;
                                }
                        break;
        }

}
*/

void util_set_reset_key_cpc_keymap(enum util_teclas_cpc_keymap tecla,int pressrelease)
{
        switch (tecla) {
			case UTIL_KEY_CPC_MINUS:
                                if (pressrelease) {
                                        cpc_keyboard_table[3] &=255-2;
                                }
                                else {
                                        cpc_keyboard_table[3] |=2;
                                }
                        break;

			case UTIL_KEY_CPC_CIRCUNFLEJO:
                                if (pressrelease) {
                                        cpc_keyboard_table[3] &=255-1;
                                }
                                else {
                                        cpc_keyboard_table[3] |=1;
                                }
                        break;

			case UTIL_KEY_CPC_ARROBA:
                                if (pressrelease) {
                                        cpc_keyboard_table[3] &=255-4;
                                }
                                else {
                                        cpc_keyboard_table[3] |=4;
                                }
                        break;

			case UTIL_KEY_CPC_BRACKET_LEFT:
                                if (pressrelease) {
                                        cpc_keyboard_table[2] &=255-2;
                                }
                                else {
                                        cpc_keyboard_table[2] |=2;
                                }
                        break;

			case UTIL_KEY_CPC_COLON:
                                if (pressrelease) {
                                        cpc_keyboard_table[3] &=255-32;
                                }
                                else {
                                        cpc_keyboard_table[3] |=32;
                                }
                        break;

			case UTIL_KEY_CPC_SEMICOLON:
                                if (pressrelease) {
                                        cpc_keyboard_table[3] &=255-16;
                                }
                                else {
                                        cpc_keyboard_table[3] |=16;
                                }
                        break;

			case UTIL_KEY_CPC_BRACKET_RIGHT:
                                if (pressrelease) {
                                        cpc_keyboard_table[2] &=255-8;
                                }
                                else {
                                        cpc_keyboard_table[2] |=8;
                                }
                        break;

			case UTIL_KEY_CPC_COMMA:
                                if (pressrelease) {
                                        cpc_keyboard_table[4] &=255-128;
                                }
                                else {
                                        cpc_keyboard_table[4] |=128;
                                }
                        break;

			case UTIL_KEY_CPC_PERIOD:


                                if (pressrelease) {
                                        cpc_keyboard_table[3] &=255-128;
                                }
                                else {
                                        cpc_keyboard_table[3] |=128;
                                }
                        break;

			case UTIL_KEY_CPC_SLASH:
                                if (pressrelease) {
                                        cpc_keyboard_table[3] &=255-64;
                                }
                                else {
                                        cpc_keyboard_table[3] |=64;
                                }
                        break;

			case UTIL_KEY_CPC_BACKSLASH:
                                if (pressrelease) {
                                        cpc_keyboard_table[2] &=255-64;
                                }
                                else {
                                        cpc_keyboard_table[2] |=64;
                                }
                        break;

	}

}

void util_set_reset_key_z88_keymap(enum util_teclas_z88_keymap tecla,int pressrelease)
{
	switch (tecla) {
                        case UTIL_KEY_Z88_MINUS:
                                if (pressrelease) {
                                        blink_kbd_a11 &= 255-128;
                                }
                                else {
                                        blink_kbd_a11 |= 128;
                                }
                        break;

                        case UTIL_KEY_Z88_EQUAL:
                                if (pressrelease) {
                                        blink_kbd_a10 &= 255-128;
                                }
                                else {
                                        blink_kbd_a10 |= 128;
                                }
                        break;

                        case UTIL_KEY_Z88_BACKSLASH:
                                if (pressrelease) {
                                        blink_kbd_a9 &= 255-128;
                                }
                                else {
                                        blink_kbd_a9 |= 128;
                                }
                        break;



                        case UTIL_KEY_Z88_BRACKET_LEFT:
                                if (pressrelease) {
                                        blink_kbd_a13 &= 255-128;
                                }
                                else {
                                        blink_kbd_a13 |= 128;
                                }
                        break;

                        case UTIL_KEY_Z88_BRACKET_RIGHT:
                                if (pressrelease) {
                                        blink_kbd_a12 &= 255-128;
                                }
                                else {
                                        blink_kbd_a12 |= 128;
                                }
                        break;

                        case UTIL_KEY_Z88_SEMICOLON:
                                if (pressrelease) {
                                        blink_kbd_a14 &= 255-2;
                                }
                                else {
                                        blink_kbd_a14 |= 2;
                                }
                        break;

                        case UTIL_KEY_Z88_APOSTROPHE:
                                if (pressrelease) {
                                        blink_kbd_a14 &= 255-1;
                                }
                                else {
                                        blink_kbd_a14 |= 1;
                                }
                        break;

                        case UTIL_KEY_Z88_POUND:
                                if (pressrelease) {
                                        blink_kbd_a15 &= 255-1;
                                }
                                else {
                                        blink_kbd_a15 |= 1;
                                }
                        break;

                        case UTIL_KEY_Z88_COMMA:
                                if (pressrelease) {

                                        blink_kbd_a14 &= (255-4);
                                }
                                else {

                                        blink_kbd_a14 |= 4;
                                }
                        break;

                        case UTIL_KEY_Z88_PERIOD:
                                 if (pressrelease) {
                                                blink_kbd_a15 &= (255-4);
                                        }
                                        else {
                                                blink_kbd_a15 |= 4;
                                        }
                        break;

                        case UTIL_KEY_Z88_SLASH:
                                 if (pressrelease) {
                                                blink_kbd_a15 &= (255-2);
                                        }
                                        else {
                                                blink_kbd_a15 |= 2;
                                        }
                        break;

	}
}


void util_set_reset_key_common_keymap(enum util_teclas_common_keymap tecla,int pressrelease)
{
        switch (tecla) {
                        case UTIL_KEY_COMMON_KEYMAP_MINUS:
                                if (pressrelease) {
                                        puerto_teclado_sam_eff9 &= 255-32;
                                        ql_keyboard_table[5] &= 255-32;
                                        msx_keyboard_table[1] &=255-4;
                                        svi_keyboard_table[2] &=255-1;
                                        //printf ("tecla minus\n");
                                        pcw_keyboard_table[3] &=255-2;
                                }
                                else {
                                        puerto_teclado_sam_eff9 |= 32;
                                        ql_keyboard_table[5] |= 32;
                                        msx_keyboard_table[1] |=4;
                                        svi_keyboard_table[2] |=1;
                                        pcw_keyboard_table[3] |=2;
                                }
                        break;

                        case UTIL_KEY_COMMON_KEYMAP_EQUAL:
                                if (pressrelease) {
                                        puerto_teclado_sam_eff9 &= 255-64;
                                        ql_keyboard_table[3] &= 255-32;
                                        msx_keyboard_table[1] &= 255-8;
                                        svi_keyboard_table[1] &=255-32;
                                        pcw_keyboard_table[3] &=(255-1);
                                }
                                else {
                                        puerto_teclado_sam_eff9 |= 64;
                                        ql_keyboard_table[3] |= 32;
                                        msx_keyboard_table[1] |=8;
                                        svi_keyboard_table[1] |=32;
                                        pcw_keyboard_table[3] |=1;
                                }
                        break;

                        //En mi teclado, tecla a la izquierda del 1
                        case UTIL_KEY_COMMON_KEYMAP_BACKSLASH:
                                if (pressrelease) {
                                  // 6|   Ret   Left     Up    Esc  Right      \  Space   Down     ql_keyboard_table[1]
                                  ql_keyboard_table[1] &=255-32;
                                  msx_keyboard_table[1] &= 255-16;
                                  svi_keyboard_table[5] &= 255-16;

                                }
                                else {
                                  ql_keyboard_table[1] |=32;
                                  msx_keyboard_table[1] |=16;
                                  svi_keyboard_table[5] |= 16;

                                }
                        break;



                        case UTIL_KEY_COMMON_KEYMAP_BRACKET_LEFT:
                                if (pressrelease) {
					                             puerto_teclado_sam_dff9 &= 255-32;
                                       //// 4|     [   Caps      k      s      f      =      g      ;     ql_keyboard_table[3]
                                       ql_keyboard_table[3] &=255-1;
                                       msx_keyboard_table[1] &= 255-32;
                                       svi_keyboard_table[5] &= 255-8;
                                       pcw_keyboard_table[3] &=(255-4);
                                }
                                else {
					                             puerto_teclado_sam_dff9 |= 32;
                                       ql_keyboard_table[3] |=1;
                                       msx_keyboard_table[1] |=32;
                                       svi_keyboard_table[5] |= 8;
                                       pcw_keyboard_table[3] |=4;
                                }
                        break;


			case UTIL_KEY_COMMON_KEYMAP_BRACKET_RIGHT:
                                if (pressrelease) {
					                             puerto_teclado_sam_dff9 &= 255-64;
                                       // 5|     ]      z      .      c      b  Pound      m      '     ql_keyboard_table[2]
                                       ql_keyboard_table[2] &=255-1;
                                       msx_keyboard_table[1] &= 255-64;
                                       svi_keyboard_table[5] &= 255-32;
                                       pcw_keyboard_table[2] &=(255-2);
                                }
                                else {
                                        puerto_teclado_sam_dff9 |= 64;
                                        ql_keyboard_table[2] |=1;
                                        msx_keyboard_table[1] |=64;
                                        svi_keyboard_table[5] |= 32;
                                        pcw_keyboard_table[2] |=2;
                                }
                        break;

                        case UTIL_KEY_COMMON_KEYMAP_SEMICOLON:
                                if (pressrelease) {
                                      // 4|     [   Caps      k      s      f      =      g      ;     ql_keyboard_table[3]
                                      ql_keyboard_table[3] &=255-128;
                                      msx_keyboard_table[1] &= 255-128;
                                      svi_keyboard_table[1] &= 255-4;
                                      pcw_keyboard_table[3] &=(255-32);
                                }
                                else {
                                      ql_keyboard_table[3] |=128;
                                      msx_keyboard_table[1] |=128;
                                      svi_keyboard_table[1] |= 4;
                                      pcw_keyboard_table[3] |=32;
                                }
                        break;

                        case UTIL_KEY_COMMON_KEYMAP_APOSTROPHE:
                                if (pressrelease) {
                                        puerto_teclado_sam_bff9 &= 255-32;
                                        // 5|     ]      z      .      c      b  Pound      m      '     ql_keyboard_table[2]
                                        ql_keyboard_table[2] &=255-128;
                                        msx_keyboard_table[2] &= 255-1;
                                        svi_keyboard_table[1] &= 255-8;
                                        pcw_keyboard_table[3] &=(255-16);
                                }
                                else {
                                        puerto_teclado_sam_bff9 |= 32;
                                        ql_keyboard_table[2] |=128;
                                         msx_keyboard_table[2] |= 1;
                                        svi_keyboard_table[1] |= 8;
                                        pcw_keyboard_table[3] |=16;
                                }
                        break;

                        case UTIL_KEY_COMMON_KEYMAP_POUND:
                                if (pressrelease) {
                                        puerto_teclado_sam_bff9 &= 255-64;
                                        //// 5|     ]      z      .      c      b  Pound      m      '     ql_keyboard_table[2]
                                        ql_keyboard_table[2] &=255-32;
                                        msx_keyboard_table[2] &= 255-2;
                                        pcw_keyboard_table[2] &=(255-8);
                                }
                                else {
                                        puerto_teclado_sam_bff9 |= 64;
                                        ql_keyboard_table[2] |=32;
                                        msx_keyboard_table[2] |= 2;
                                        pcw_keyboard_table[2] |=8;
                                }
                        break;


			case UTIL_KEY_COMMON_KEYMAP_COMMA:
                                if (pressrelease) {
					                              puerto_teclado_sam_7ff9 &= 255-32;
                                        ql_keyboard_table[7] &=(255-128);
                                        msx_keyboard_table[2] &= 255-4;
                                        svi_keyboard_table[1] &= 255-16;
                                        pcw_keyboard_table[4] &=(255-128);

                                }
                                else {
					                             puerto_teclado_sam_7ff9 |= 32;
                                       ql_keyboard_table[7] |=128;
                                       msx_keyboard_table[2] |= 4;
                                       svi_keyboard_table[1] |= 16;
                                       pcw_keyboard_table[4] |=128;
                                }
                        break;

                        case UTIL_KEY_COMMON_KEYMAP_PERIOD:
                                 if (pressrelease) {
					                              puerto_teclado_sam_7ff9 &= 255-64;
                                        ql_keyboard_table[2] &=(255-4);
                                        msx_keyboard_table[2] &= 255-8;
                                        svi_keyboard_table[1] &= 255-64;
                                        pcw_keyboard_table[3] &=(255-128);
                                 }
                                 else {
					                              puerto_teclado_sam_7ff9 |= 64;
                                        ql_keyboard_table[2] |=4;
                                        msx_keyboard_table[2] |= 8;
                                        svi_keyboard_table[1] |= 64;
                                        pcw_keyboard_table[3] |=128;
                                 }
                        break;

                        case UTIL_KEY_COMMON_KEYMAP_SLASH:
                                 if (pressrelease) {
					                              puerto_teclado_sam_7ff9 &= 255-128;
                                        // 0| Shift   Ctrl    Alt      x      v      /      n      ,     ql_keyboard_table[7]
                                        ql_keyboard_table[7] &=255-32;
                                        msx_keyboard_table[2] &= 255-16;
                                        svi_keyboard_table[1] &= 255-128;
                                        pcw_keyboard_table[3] &=(255-64);
                                 }
                                 else {
					                              puerto_teclado_sam_7ff9 |= 128;
                                        ql_keyboard_table[7] |=32;
                                        msx_keyboard_table[2] |= 16;
                                        svi_keyboard_table[1] |= 128;
                                        pcw_keyboard_table[3] |=64;
                                 }
                        break;

                        case UTIL_KEY_COMMON_KEYMAP_LEFTZ:
                                 if (pressrelease) {

                                        // tecla 1/2 cerca del shift derecho
                                        pcw_keyboard_table[2] &=(255-64);
                                 }
                                 else {

                                        pcw_keyboard_table[2] |=64;
                                 }
                        break;

        }
}



z80_bit chloe_keyboard_pressed_shift={0};
//z80_bit chloe_keyboard_pressed_alt={0};
z80_bit chloe_keyboard_pressed_winkey={0};

//Tecla pulsada. Valor de util_teclas
enum util_teclas chloe_keyboard_pressed_tecla=UTIL_KEY_NONE;

//Valor en ascii
z80_byte chloe_keyboard_pressed_tecla_ascii=0;


//Tabla de teclas a enviar con modificador en chloe
//Todas se corresponen a symbol shift + algo

struct s_teclas_chloe_numeros {
	z80_byte ascii_origen;
	z80_byte ascii_destino;
};

typedef struct s_teclas_chloe_numeros teclas_chloe_numeros;

#define TOTAL_TECLAS_CHLOE_NUMEROS 10
teclas_chloe_numeros tabla_teclas_chloe_numeros[TOTAL_TECLAS_CHLOE_NUMEROS]={
	{'1','1'},
	{'2','2'},
	{'3','3'},
	{'4','4'},
	{'5','5'},
	{'6','h'},
	{'7','6'},
	{'8','b'},
	{'9','8'},
	{'0','9'},
};

void util_set_reset_key_chloe_keymap(enum util_teclas_chloe_keymap tecla,int pressrelease)
{
	//printf ("envio de tecla extendida por keymap. tecla: %d pressrelease: %d\n",tecla,pressrelease);

	//todas generan sym shift
	if (pressrelease==0) {
		reset_keyboard_ports();
		return;
	}


	//Se pulsan las teclas
	//Teclas que vienen de keymap, como -_ etc
	switch (tecla) {
		case UTIL_KEY_CHLOE_MINUS:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('0',1);
			else  convert_numeros_letras_puerto_teclado_continue('j',1);
		break;

		case UTIL_KEY_CHLOE_EQUAL:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('k',1);
			else convert_numeros_letras_puerto_teclado_continue('l',1);
		break;


		case UTIL_KEY_CHLOE_BRACKET_LEFT:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('f',1);
			else convert_numeros_letras_puerto_teclado_continue('y',1);
		break;

		case UTIL_KEY_CHLOE_BRACKET_RIGHT:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('g',1);
			else convert_numeros_letras_puerto_teclado_continue('u',1);
		break;

		case UTIL_KEY_CHLOE_SEMICOLON:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('z',1);
			else convert_numeros_letras_puerto_teclado_continue('o',1);
		break;

		case UTIL_KEY_CHLOE_APOSTROPHE:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('p',1);
			else convert_numeros_letras_puerto_teclado_continue('7',1);
		break;

		case UTIL_KEY_CHLOE_POUND:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('s',1);
			else convert_numeros_letras_puerto_teclado_continue('d',1);
		break;

		case UTIL_KEY_CHLOE_SLASH:
			set_symshift();
			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('c',1);
			else convert_numeros_letras_puerto_teclado_continue('v',1);
		break;

		case UTIL_KEY_CHLOE_COMMA:
			set_symshift();
                        if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('r',1);
                        else convert_numeros_letras_puerto_teclado_continue('n',1);
		break;

		case UTIL_KEY_CHLOE_PERIOD:
			set_symshift();
                        if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('t',1);
                        else convert_numeros_letras_puerto_teclado_continue('m',1);
		break;

		case UTIL_KEY_CHLOE_LEFTZ:
			set_symshift();
                        if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('a',1);
                        else convert_numeros_letras_puerto_teclado_continue('x',1);
		break;

		case UTIL_KEY_CHLOE_BACKSLASH:
			//Esto se mapea a true video (shift +3) e inverse video (shift+4)
			//Activar mayus
			puerto_65278  &=255-1;

			if (chloe_keyboard_pressed_shift.v) convert_numeros_letras_puerto_teclado_continue('4',1);
			else convert_numeros_letras_puerto_teclado_continue('3',1);
		break;




	}
}


void util_set_reset_key_chloe(void)
{
	//printf ("Shift: %d non ascii key: %d ascii: %d\n",chloe_keyboard_pressed_shift.v,chloe_keyboard_pressed_tecla,chloe_keyboard_pressed_tecla_ascii);

	if (chloe_keyboard_pressed_winkey.v) {
		set_symshift();
	}

				if (chloe_keyboard_pressed_shift.v==0) {
					//Tecla sin shift
					//Enviar tecla tal cual sin modificador
					//printf ("Enviar tecla sin shift\n");
					if (chloe_keyboard_pressed_tecla!=UTIL_KEY_NONE) {
						//printf ("tecla no es ascii: %d\n",chloe_keyboard_pressed_tecla);
						util_set_reset_key_continue(chloe_keyboard_pressed_tecla,1);
					}
					else {
						//printf ("tecla es ascii: %d\n",chloe_keyboard_pressed_tecla_ascii);
						convert_numeros_letras_puerto_teclado_continue(chloe_keyboard_pressed_tecla_ascii,1);
					}
					return;
				}


				//Tecla con shift

                                if (chloe_keyboard_pressed_tecla==UTIL_KEY_NONE && chloe_keyboard_pressed_tecla_ascii==0) {
					//No tecla pulsada. Volver
                                        return;
                                }

                                else {
                                        //Tecla con shift
					//printf ("Pulsado shift y tecla %d o %d ascii\n",chloe_keyboard_pressed_tecla,chloe_keyboard_pressed_tecla_ascii);

					//Ver si es tecla ascii
					if (chloe_keyboard_pressed_tecla_ascii) {
						//printf ("Es tecla ascii\n");
						int i;

						//Ver si es tecla numerica
						for (i=0;i<TOTAL_TECLAS_CHLOE_NUMEROS;i++) {
							if (tabla_teclas_chloe_numeros[i].ascii_origen==chloe_keyboard_pressed_tecla_ascii) {
								set_symshift();
								convert_numeros_letras_puerto_teclado_continue(tabla_teclas_chloe_numeros[i].ascii_destino,1);
								return;
							}
						}

						//No es tecla numerica. Enviar mayusculas y ascii
						util_set_reset_key_continue(UTIL_KEY_SHIFT_L,1);
						convert_numeros_letras_puerto_teclado_continue(chloe_keyboard_pressed_tecla_ascii,1);
						return;
					}

					//No es tecla ascii
					//printf ("No es tecla ascii\n");


					if (chloe_keyboard_pressed_tecla!=UTIL_KEY_NONE) util_set_reset_key_continue(chloe_keyboard_pressed_tecla,1);
					else convert_numeros_letras_puerto_teclado_continue(chloe_keyboard_pressed_tecla_ascii,1);



                                        return;
                                }
}


//Decir si el raton funciona sobre el menu
int si_menu_mouse_activado(void)
{
  //if (kempston_mouse_emulation.v) return 0;
  if (gunstick_emulation) return 0;
  if (mouse_menu_disabled.v) return 0;

  return 1;
}


//Numero de veces que se ha pulsado la tecla de abrir el menu
int util_if_open_just_menu_times=0;

//Contador que se incrementa cada vez desde timer.c (50 veces por segundo)
unsigned int util_if_open_just_menu_counter=0;

//Contador inicial de la primera pulsacion de tecla de abrir menu
unsigned int util_if_open_just_menu_initial_counter=0;

//Accion de abrir menu (F5, boton joystick) que ademas controla si el boton esta limitado para que abra el menu solo cuando se pulsa 3 veces seguidas, por ejemplo, en 1 segundo
int util_if_open_just_menu(void)
{
	if (menu_limit_menu_open.v==0) return 1;

	//esta limitado el uso de F5, hay que pulsar 3 veces en el ultimo segundo
	if (util_if_open_just_menu_times==0) {
		util_if_open_just_menu_initial_counter=util_if_open_just_menu_counter;
	}

	util_if_open_just_menu_times++;
	debug_printf (VERBOSE_DEBUG,"Pressed open menu key: %d times",util_if_open_just_menu_times);

	//Si llega a 3 veces, ver si la diferencia del contador es menor o igual que 50
	if (util_if_open_just_menu_times==3) {
		util_if_open_just_menu_times=0;
		int diferencia=util_if_open_just_menu_counter-util_if_open_just_menu_initial_counter;
		debug_printf (VERBOSE_DEBUG,"Time elapsed between the third keypress and the first one (in 1/50s): %d",diferencia);
		if (diferencia<=50) return 1;
	}

	return 0;
}


//Si esta emulacion de kempston mouse activa, no se puede usar el raton para el menu
void util_set_reset_mouse(enum util_mouse_buttons boton,int pressrelease)
{

    util_generate_random_noise(pressrelease);

  switch(boton)
  {
    case UTIL_MOUSE_LEFT_BUTTON:
      if (pressrelease) {

        //printf ("Leido press release mouse\n");
        if (si_menu_mouse_activado() ) {
          //Si no esta menu abierto, hace accion de abrir menu, siempre que no este kempston
          if (menu_abierto==0) {
                  if (kempston_mouse_emulation.v==0) {
                      if (mouse_menu_ignore_click_open.v==0) {
                          menu_fire_event_open_menu();
                          menu_was_open_by_left_mouse_button.v=1;
                      }
                  }
          }
          else {
            //Si esta menu abierto, es como enviar enter, pero cuando no esta la ventana en background
            if (zxvision_keys_event_not_send_to_machine) {
                    //y si se pulsa dentro de ventana
                    if (si_menu_mouse_en_ventana() ) {
                    //util_set_reset_key(UTIL_KEY_ENTER,1);  //Ya no enviamos enter al pulsar boton mouse
                    }
            }
          }
        }

        mouse_left=1;
        //printf ("left button pressed\n");

      }
      else {

        if (si_menu_mouse_activado()) {

          if (menu_abierto==1) {
            //Si esta menu abierto, es como enviar enter
            //util_set_reset_key(UTIL_KEY_ENTER,0); //Ya no enviamos enter al pulsar boton mouse
          }
        }

        //printf ("left button released\n");
        mouse_left=0;
        //printf ("reseteamos mouse_pressed_close_window desde utils ventana\n");
        //puerto_especial1 |=1;
        mouse_pressed_close_window=0;
        mouse_pressed_background_window=0;
        mouse_pressed_hotkey_window=0;
        mouse_pressed_hotkey_window_key=0;

      }
    break;

    case UTIL_MOUSE_RIGHT_BUTTON:
      if (pressrelease) {
        mouse_right=1;
        //Si esta menu abierto, hace como ESC

        if (si_menu_mouse_activado()) {
          //Si no esta menu abierto, hace accion de abrir menu, siempre que no este kempston
          if (menu_abierto==0) {
                  if (kempston_mouse_emulation.v==0) {
                      if (mouse_menu_ignore_click_open.v==0) {
                        menu_fire_event_open_menu();

                        if (menu_mouse_right_send_esc.v==0) menu_was_open_by_right_mouse_button.v=1;
                      }
                  }
          }
          else {
            //Si esta menu abierto, es como enviar ESC si setting menu_mouse_right_send_esc.v
            if (zxvision_keys_event_not_send_to_machine && menu_mouse_right_send_esc.v) {
                    //y si se pulsa dentro de ventana
                    //no ver dentro ventana
                    //if (si_menu_mouse_en_ventana() ) {
                    util_set_reset_key(UTIL_KEY_ESC,1);
                    //}
            }
          }
        }

      }
      else {
        mouse_right=0;
        //Si esta menu abierto, hace como ESC si setting menu_mouse_right_send_esc.v
        if (si_menu_mouse_activado() && menu_mouse_right_send_esc.v) {
          if (menu_abierto) util_set_reset_key(UTIL_KEY_ESC,0);
        }

        mouse_pressed_close_window=0;
        mouse_pressed_background_window=0;
        mouse_pressed_hotkey_window=0;
        mouse_pressed_hotkey_window_key=0;
      }
    break;

  }


}


void util_set_reset_key_handle_chloe_ascii(enum util_teclas tecla,int pressrelease)
{

                //printf ("teclado es chloe. tecla=%d\n",tecla);
                //tecla ascii
       //Tecla no es shift. comentario absurdo???
                        //Ver si se pulsa o se libera
                        if (pressrelease==0) {
                                //Se libera
				chloe_keyboard_pressed_tecla_ascii=0;
                                //Liberar todas teclas de puertos
                                reset_keyboard_ports();
                        }
                        else {
                                //Se pulsa
                                //Si solo pulsado shift pero no una tecla diferente, no hacer nada, solo activar modificador
                                chloe_keyboard_pressed_tecla_ascii=tecla;
                                util_set_reset_key_chloe();
                        }

}

void util_set_reset_key_convert_recreated_yesno(enum util_teclas tecla,int pressrelease,int convertrecreated)
{


	//Para poder evitar que se vuelva a convertir cuando se llama desde convert_numeros_letras_puerto_teclado_continue
	if (convertrecreated) {
		//Si teclado recreated, y menu cerrado
		if (!menu_abierto && recreated_zx_keyboard_support.v) {

			enum util_teclas tecla_final;
			int pressrelease_final;

			//Si es mayusculas
			if (tecla==UTIL_KEY_SHIFT_L) {
				//printf ("Pulsada shift L real\n");
				if (pressrelease) recreated_zx_keyboard_pressed_caps.v=1;
				else recreated_zx_keyboard_pressed_caps.v=0;
				return;
			}

			//printf ("Entrada a convertir tecla recreated desde util_set_reset_key\n");
			recreated_zx_spectrum_keyboard_convert(tecla, &tecla_final, &pressrelease_final);
			if (tecla_final) {
				tecla=tecla_final;
				pressrelease=pressrelease_final;
			}
		}
	}

	if (chloe_keyboard.v) {
		//Ver si tecla que se pulsa es shift
		if (tecla==UTIL_KEY_SHIFT_L || tecla==UTIL_KEY_SHIFT_R) {
			//Ver si se pulsa o se libera
			if (pressrelease==0) {
				//Se libera
				chloe_keyboard_pressed_shift.v=0;
				util_set_reset_key_continue(tecla,pressrelease);
				return;
			}
			else {
				//Se pulsa
				//Si solo pulsado shift pero no una tecla diferente, no hacer nada, solo activar modificador
				chloe_keyboard_pressed_shift.v=1;
				util_set_reset_key_chloe();
				return;
			}
		}

		//Ver si se pulsa tecla Windows (o cmd en mac)
		else if (tecla==UTIL_KEY_WINKEY_L) {
			//Ver si se pulsa o se libera
                        if (pressrelease==0) {
				//printf ("Liberada winkey\n");
				chloe_keyboard_pressed_winkey.v=0;
				clear_symshift();
				//Liberar todas teclas de puertos en caso de driver cocoa
				//Esto sucede porque esto es tecla cmd y en cocoa,
				//cuando se libera cmd, no se envian liberacion de otras teclas pulsadas
				if (strcmp(scr_new_driver_name,"cocoa")==0) {
					//printf ("Liberando todas teclas en driver cocoa\n");
					chloe_keyboard_pressed_tecla=UTIL_KEY_NONE;
					chloe_keyboard_pressed_tecla_ascii=0;
					reset_keyboard_ports();
	                                return;
				}
				util_set_reset_key_continue(tecla,pressrelease);
                                return;
                        }
                        else {
                                //Se pulsa
				//printf ("utils. enviando winkey - symbol shift\n");
				chloe_keyboard_pressed_winkey.v=1;
				util_set_reset_key_chloe();
                                return;
                        }
		}

		else if (tecla==UTIL_KEY_ALT_L || tecla==UTIL_KEY_ALT_R) {
			//Se pulsa alt
			if (pressrelease==0) {
				//Liberar todas teclas de puertos
                                reset_keyboard_ports();
                                return;
                        }
                        else {
                                //Se pulsa
				//Enviar sym+shift
				set_symshift();
				puerto_65278  &=255-1;
				return;
			}
		}

		else if (tecla==UTIL_KEY_CONTROL_L || tecla==UTIL_KEY_CONTROL_R)  {
			//En Chloe, Control es shift+9
			if (pressrelease==0) {
                                //Liberar todas teclas de puertos
                                reset_keyboard_ports();
                                return;
                        }
                        else {
                                //Se pulsa
                                //Shift
                                puerto_65278  &=255-1;
                                //9
                                puerto_61438  &=255-2;
				return;
			}
		}


		else if (tecla==UTIL_KEY_TAB && menu_abierto==0) {
                       //En Chloe, TAB es shift+1. Pero con menu cerrado
			if (pressrelease==0) {
                                //Liberar todas teclas de puertos
                                reset_keyboard_ports();
                                return;
                        }
                        else {
                                //Shift
                                puerto_65278  &=255-1;
                                //1
                                puerto_63486  &=255-1;
				return;
			}
                }


		else if (tecla==UTIL_KEY_ESC) {
		      //ESC en Chloe es BREAK
			if (pressrelease==0) {
                                //Liberar todas teclas de puertos
                                reset_keyboard_ports();
                                return;
                        }
                        else {
                        	puerto_65278 &=255-1;
				puerto_32766 &=255-1;
				//Y indicar tecla ESC y otra funcion de esc (liberar cola speech)
				util_set_reset_key_continue(UTIL_KEY_ESC,1);
                                return;
                        }
		}



		else {
			//Ver si se pulsa o se libera
                        if (pressrelease==0) {
                                //Se libera
                                chloe_keyboard_pressed_tecla=UTIL_KEY_NONE;
				//Liberar todas teclas de puertos
				reset_keyboard_ports();
                                return;
                        }
                        else {
                                //Se pulsa
                                //Si solo pulsado shift pero no una tecla diferente, no hacer nada, solo activar modificador
                                chloe_keyboard_pressed_tecla=tecla;
                                util_set_reset_key_chloe();
				return;
                        }
                }


	}

	else {
		util_set_reset_key_continue(tecla,pressrelease);
	}

}


//componente de "ruido" para random consistente en
//-tiempo entre pulsacion y liberacion de una tecla y de boton de raton
int util_random_noise=0;
int util_random_noise_last_time=0;

void util_generate_random_noise(int pressrelease)
{
    //componente random noise
    if (pressrelease) util_random_noise_last_time=contador_segundo_infinito;
    else {
        util_random_noise=contador_segundo_infinito-util_random_noise_last_time;
    }
}

void util_set_reset_key(enum util_teclas tecla,int pressrelease)
{

    util_generate_random_noise(pressrelease);

        if (chloe_keyboard.v && tecla>=32 && tecla<=127) {
                util_set_reset_key_handle_chloe_ascii(tecla,pressrelease);
                return;
        }
        else {
        //printf ("util_set_reset_key tecla: %d pressrelease: %d\n",tecla,pressrelease);
       util_set_reset_key_convert_recreated_yesno(tecla,pressrelease,1);
        }
}

void convert_numeros_letras_puerto_teclado(z80_byte tecla,int pressrelease)
{

        convert_numeros_letras_puerto_teclado_continue(tecla,pressrelease);
}






//Retorna no 0 si hay redefinicion de tecla F (no es default)
int util_set_reset_key_continue_f_functions(enum util_teclas tecla,int pressrelease)
{

  //printf ("tecla: %d pressrelease: %d menu_abierto: %d\n",tecla,pressrelease,menu_abierto);

  //Aunque este el menu abierto, procesar estas teclas. Asi esto permite que la tecla de OSD aparezca
  //desde el menu
  //if (menu_abierto) return 0;
  //Ver si la tecla F esta asignada
  //enum defined_f_function_ids defined_f_functions_keys_array[MAX_F_FUNCTIONS_KEYS]

  int indice;

  switch (tecla) {
    case UTIL_KEY_F1:
      indice=0;
    break;

    case UTIL_KEY_F2:
      indice=1;
    break;

    case UTIL_KEY_F3:
      indice=2;
    break;

    case UTIL_KEY_F4:
      indice=3;
    break;

    case UTIL_KEY_F5:
      indice=4;
      //printf ("F5!!\n");
    break;

    case UTIL_KEY_F6:
      indice=5;
    break;

    case UTIL_KEY_F7:
      indice=6;
    break;

    case UTIL_KEY_F8:
      indice=7;
    break;

    case UTIL_KEY_F9:
      indice=8;
    break;

    case UTIL_KEY_F10:
      indice=9;
    break;

    case UTIL_KEY_F11:
      indice=10;
    break;

    case UTIL_KEY_F12:
      indice=11;
    break;

    case UTIL_KEY_F13:
      indice=12;
    break;

    case UTIL_KEY_F14:
      indice=13;
    break;

    case UTIL_KEY_F15:
      indice=14;
    break;

    default:
      return 0;
    break;
  }

  int indice_tabla=defined_f_functions_keys_array[indice];
  debug_printf (VERBOSE_DEBUG,"Key: F%d Action: %s",indice+1,defined_direct_functions_array[indice_tabla].texto_funcion);

  enum defined_f_function_ids accion=menu_da_accion_direct_functions_indice(indice_tabla);

  //printf ("Key: F%d Action: %s indice_tabla: %d accion: %d\n",indice+1,defined_direct_functions_array[indice_tabla].texto_funcion,indice_tabla,accion);

  //Abrir menu si funcion no es defecto y no es background window
  //if (accion!=F_FUNCION_DEFAULT && accion!=F_FUNCION_BACKGROUND_WINDOW) {
  if (accion!=F_FUNCION_DEFAULT) {
    if (pressrelease) {
      //Activar funcion f en menu
      menu_button_f_function.v=1;
      //printf("Indice accion: %d\n",indice);
      menu_button_f_function_index=indice;
      menu_abierto=1;

/* Pruebas abrir ventana directa
zxvision_switch_to_window_on_open_menu=1;
strcpy(zxvision_switch_to_window_on_open_menu_name,"debugcpu");
menu_event_open_menu.v=1;
*/

    }
    return 1;
  }

  return 0;
}


//Aqui solo se activan/desactivan bits de puerto_especial para teclas F
void util_set_reset_key_continue_tecla_f(enum util_teclas tecla,int pressrelease)
{

switch(tecla)
{

  //F1 pulsado
  case UTIL_KEY_F1:

          if (pressrelease) {
                  puerto_especial2 &=255-1;
          }
          else {
                  puerto_especial2 |=1;
          }
  break;

  //F2 pulsado
  case UTIL_KEY_F2:

          if (pressrelease) {
                  puerto_especial2 &=255-2;
          }
          else {
                  puerto_especial2 |=2;
          }
  break;


  case UTIL_KEY_F3:

          if (pressrelease) {
                  puerto_especial2 &=255-4;
          }
          else {
                  puerto_especial2 |=4;
          }
  break;


       case UTIL_KEY_F4:
           if (pressrelease) {
             puerto_especial2 &=255-8;
           }

           else {
             puerto_especial2 |=8;
           }
      break;


  case UTIL_KEY_F5:

          if (pressrelease) {
                  puerto_especial2 &=255-16;
          }
          else {
                  puerto_especial2 |=16;
          }
  break;


  case UTIL_KEY_F6:

          if (pressrelease) {
                  puerto_especial3 &=255-1;
          }
          else {
                  puerto_especial3 |=1;
          }
  break;


  case UTIL_KEY_F7:

          if (pressrelease) {
                  puerto_especial3 &=255-2;
          }
          else {
                  puerto_especial3 |=2;
          }
  break;


  case UTIL_KEY_F8:
        if (pressrelease) {
               puerto_especial3 &=255-4;
        }
        else {
              puerto_especial3 |=4;
        }

  break;


  case UTIL_KEY_F9:

          if (pressrelease) {
              puerto_especial3 &=255-8;
          }
          else {
              puerto_especial3 |=8;
          }

  break;


  case UTIL_KEY_F10:

          if (pressrelease) {
                  puerto_especial3 &=255-16;
          }
          else {
                  puerto_especial3 |=16;
          }
  break;


  case UTIL_KEY_F11:

          if (pressrelease) {
                  puerto_especial4 &=255-1;
          }
          else {
                  puerto_especial4 |=1;
          }
  break;


  case UTIL_KEY_F12:

          if (pressrelease) {
                  puerto_especial4 &=255-2;
          }
          else {
                  puerto_especial4 |=2;
          }
  break;


  case UTIL_KEY_F13:

          if (pressrelease) {
                  puerto_especial4 &=255-4;
          }
          else {
                  puerto_especial4 |=4;
          }
  break;


  case UTIL_KEY_F14:

          if (pressrelease) {
                  puerto_especial4 &=255-8;
          }
          else {
                  puerto_especial4 |=8;
          }
  break;


  case UTIL_KEY_F15:

          if (pressrelease) {
                  puerto_especial4 &=255-16;
          }
          else {
                  puerto_especial4 |=16;
          }
  break;

  default:
    //Para que no se queje el compilador
  break;

}

//printf ("puerto especial 4 en set: %d\n",puerto_especial4);

}

void util_set_reset_key_continue(enum util_teclas tecla,int pressrelease)
{

  //Activar bits de puertos_especiales para teclas F
  //Hay que hacerlo asi para que el menu se vea notificado de pulsacion o no pulsacion de esas teclas, estén o no asignadas a funciones
  //Sirve por ejemplo para que si tenemos mapeado F13 a hard reset, al pulsar F13, cuando se llama a menu_espera_no_tecla, que funcione
  util_set_reset_key_continue_tecla_f(tecla,pressrelease);

  //Ver si hay teclas F redefinidas
  if (util_set_reset_key_continue_f_functions(tecla,pressrelease)) return;


  //No estoy seguro que este sea el mejor sitio para llamar a zeng, pero...

  //Enviar tecla si no es cursor (esto se trata como joystick aparte)
  if (tecla!=UTIL_KEY_FIRE && tecla!=UTIL_KEY_LEFT && tecla!=UTIL_KEY_RIGHT && tecla!=UTIL_KEY_DOWN && tecla!=UTIL_KEY_UP) {
    //printf("Enviando tecla %3d pressrelease %d desde utils\n",tecla,pressrelease);
        zeng_send_key_event(tecla,pressrelease);
  }

//Si esta zeng online y somos slave, no enviamos teclas locales nuestras
//ya nos llegara el cambio debido al snapshot y de la recepcion de teclas

z80_byte antes_puerto_65278=puerto_65278;
z80_byte antes_puerto_65022=puerto_65022;
z80_byte antes_puerto_64510=puerto_64510;
z80_byte antes_puerto_63486=puerto_63486;
z80_byte antes_puerto_61438=puerto_61438;
z80_byte antes_puerto_57342=puerto_57342;
z80_byte antes_puerto_49150=puerto_49150;
z80_byte antes_puerto_32766=puerto_32766;


z80_byte antes_puerto_especial_joystick=puerto_especial_joystick;


    unsigned char antes_ql_keyboard_table[8];
    z80_byte antes_cpc_keyboard_table[16];
    z80_byte antes_msx_keyboard_table[16];
    z80_byte antes_svi_keyboard_table[16];
    z80_byte antes_pcw_keyboard_table[16];
    z80_byte antes_sam_keyboard_table[8];
    z80_byte antes_z88_keyboard_table[8];
    z80_byte antes_mk14_keyboard_table[8];
    int i;
    if (MACHINE_IS_QL) {
        for (i=0;i<8;i++) antes_ql_keyboard_table[i]=ql_keyboard_table[i];
    }

    if (MACHINE_IS_CPC) {
        for (i=0;i<16;i++) antes_cpc_keyboard_table[i]=cpc_keyboard_table[i];
    }

    if (MACHINE_IS_MSX) {
        for (i=0;i<16;i++) antes_msx_keyboard_table[i]=msx_keyboard_table[i];
    }

    if (MACHINE_IS_SVI) {
        for (i=0;i<16;i++) antes_svi_keyboard_table[i]=svi_keyboard_table[i];
    }

    if (MACHINE_IS_PCW) {
        for (i=0;i<16;i++) antes_pcw_keyboard_table[i]=pcw_keyboard_table[i];
    }

    if (MACHINE_IS_SAM) {
        antes_sam_keyboard_table[0]=puerto_teclado_sam_fef9;
        antes_sam_keyboard_table[1]=puerto_teclado_sam_fdf9;
        antes_sam_keyboard_table[2]=puerto_teclado_sam_fbf9;
        antes_sam_keyboard_table[3]=puerto_teclado_sam_f7f9;
        antes_sam_keyboard_table[4]=puerto_teclado_sam_eff9;
        antes_sam_keyboard_table[5]=puerto_teclado_sam_dff9;
        antes_sam_keyboard_table[6]=puerto_teclado_sam_bff9;
        antes_sam_keyboard_table[7]=puerto_teclado_sam_7ff9;
    }

    if (MACHINE_IS_Z88) {
        antes_z88_keyboard_table[0]=blink_kbd_a15;
        antes_z88_keyboard_table[1]=blink_kbd_a14;
        antes_z88_keyboard_table[2]=blink_kbd_a13;
        antes_z88_keyboard_table[3]=blink_kbd_a12;
        antes_z88_keyboard_table[4]=blink_kbd_a11;
        antes_z88_keyboard_table[5]=blink_kbd_a10;
        antes_z88_keyboard_table[6]=blink_kbd_a9;
        antes_z88_keyboard_table[7]=blink_kbd_a8;
    }

    if (MACHINE_IS_MK14) {
        for (i=0;i<8;i++) antes_mk14_keyboard_table[i]=mk14_keystatus[i];
    }

  util_set_reset_key_continue_after_zeng(tecla,pressrelease);


  if (zeng_online_connected.v && zeng_online_i_am_master.v==0 && !menu_abierto && zoc_last_snapshot_received_counter>0 && zeng_online_allow_instant_keys.v==0) {
    //printf("Antes Puerto puerto_61438: %d\n",puerto_61438);
    puerto_65278=antes_puerto_65278;
    puerto_65022=antes_puerto_65022;
    puerto_64510=antes_puerto_64510;
    puerto_63486=antes_puerto_63486;
    puerto_61438=antes_puerto_61438;
    puerto_57342=antes_puerto_57342;
    puerto_49150=antes_puerto_49150;
    puerto_32766=antes_puerto_32766;


    puerto_especial_joystick=antes_puerto_especial_joystick;
    //printf("Despues Puerto puerto_61438: %d\n",puerto_61438);

    if (MACHINE_IS_QL) {
        for (i=0;i<8;i++) ql_keyboard_table[i]=antes_ql_keyboard_table[i];
    }

    if (MACHINE_IS_CPC) {
        for (i=0;i<16;i++) cpc_keyboard_table[i]=antes_cpc_keyboard_table[i];
    }

    if (MACHINE_IS_MSX) {
        for (i=0;i<16;i++) msx_keyboard_table[i]=antes_msx_keyboard_table[i];
    }

    if (MACHINE_IS_SVI) {
        for (i=0;i<16;i++) svi_keyboard_table[i]=antes_svi_keyboard_table[i];
    }

    if (MACHINE_IS_PCW) {
        for (i=0;i<16;i++) pcw_keyboard_table[i]=antes_pcw_keyboard_table[i];
    }

    if (MACHINE_IS_SAM) {
        puerto_teclado_sam_fef9=antes_sam_keyboard_table[0];
        puerto_teclado_sam_fdf9=antes_sam_keyboard_table[1];
        puerto_teclado_sam_fbf9=antes_sam_keyboard_table[2];
        puerto_teclado_sam_f7f9=antes_sam_keyboard_table[3];
        puerto_teclado_sam_eff9=antes_sam_keyboard_table[4];
        puerto_teclado_sam_dff9=antes_sam_keyboard_table[5];
        puerto_teclado_sam_bff9=antes_sam_keyboard_table[6];
        puerto_teclado_sam_7ff9=antes_sam_keyboard_table[7];
    }

    if (MACHINE_IS_Z88) {
        blink_kbd_a15=antes_z88_keyboard_table[0];
        blink_kbd_a14=antes_z88_keyboard_table[1];
        blink_kbd_a13=antes_z88_keyboard_table[2];
        blink_kbd_a12=antes_z88_keyboard_table[3];
        blink_kbd_a11=antes_z88_keyboard_table[4];
        blink_kbd_a10=antes_z88_keyboard_table[5];
        blink_kbd_a9=antes_z88_keyboard_table[6];
        blink_kbd_a8=antes_z88_keyboard_table[7];
    }

    if (MACHINE_IS_MK14) {
        for (i=0;i<8;i++) mk14_keystatus[i]=antes_mk14_keyboard_table[i];
    }

    //zoc_decir_pulsada_alguna_tecla_local();
  }
}

void util_set_reset_key_continue_after_zeng(enum util_teclas tecla,int pressrelease)
{



    switch (tecla) {
        case UTIL_KEY_SPACE:
        case 32:
            if (pressrelease) {
                    puerto_32766 &=255-1;
                    blink_kbd_a13 &= (255-64);
                    cpc_keyboard_table[5] &= (255-128);
                    ql_keyboard_table[1] &= (255-64);
                    msx_keyboard_table[8] &= (255-1);
                    svi_keyboard_table[8] &= (255-1);
                    pcw_keyboard_table[5] &=(255-128);
            }
            else {
                    puerto_32766 |=1;
                    blink_kbd_a13 |= 64;
                                cpc_keyboard_table[5] |= 128;
                    ql_keyboard_table[1] |= 64;
                    msx_keyboard_table[8] |= 1;
                    svi_keyboard_table[8] |= 1;
                    pcw_keyboard_table[5] |=128;
            }
        break;

                        case UTIL_KEY_ENTER:
                                if (pressrelease) {
                                        puerto_49150 &=255-1;
                                        blink_kbd_a8 &= (255-64);

                                        ql_keyboard_table[1] &= (255-1);
                                        msx_keyboard_table[7] &= (255-128);
                                        svi_keyboard_table[6] &= (255-64);

                                        if (keyboard_swap_enter_return.v==0) {
                                            cpc_keyboard_table[2] &= (255-4);
                                            pcw_keyboard_table[2] &=(255-4);
                                        }
                                        else {
                                            cpc_keyboard_table[0] &=(255-64);
                                            pcw_keyboard_table[0xA] &=(255-32);
                                        }

					//Avisar de envio enter especial para rutinas de speech, para que envien sonido
					textspeech_send_new_line();

                                }

                                else {
                                        puerto_49150 |=1;
                                        blink_kbd_a8 |=64;

                                        ql_keyboard_table[1] |= 1;
                                        msx_keyboard_table[7] |= 128;
                                        svi_keyboard_table[6] |= 64;

                                        if (keyboard_swap_enter_return.v==0) {
                                            cpc_keyboard_table[2] |= 4;
                                            pcw_keyboard_table[2] |=4;
                                        }
                                        else {
                                            cpc_keyboard_table[0] |=64;
                                            pcw_keyboard_table[0xA] |=32;
                                        }
                                }


					//if (MACHINE_IS_CPC) {
					//	printf ("Temporal en CPC Enter es Fire\n");
//&49     DEL     Joy 1 Fire 3 (CPC only) Joy 1 Fire 2    Joy1 Fire 1     Joy1 right      Joy1 left       Joy1 down       Joy1 up
					//	if (pressrelease) cpc_keyboard_table[9] &=(255-16);
					//	else cpc_keyboard_table[9] |=16;
					//}



                        break;


                        case UTIL_KEY_HOME:
                        //printf("pressed home\n");

                                if (pressrelease) {
                                        puerto_especial1 &=255-16;
                                }
                                else {
                                        puerto_especial1 |=16;
                                }

                        break;

                        case UTIL_KEY_END:
                                if (pressrelease) {
                                    puerto_especial1 &=255-8;
                                        //End en cpc es copy
                                        cpc_keyboard_table[1] &=(255-2);
                                }
                                else {
                                    puerto_especial1 |=8;
                                        //End en cpc es copy
                                        cpc_keyboard_table[1] |=2;
                                }
                        break;


                        case UTIL_KEY_DEL:
                                //printf("util press del\n");
                                if (pressrelease) {
                                        //del en cpc es del
                                        //&49	DEL	Joy 1 Fire 3 (CPC only)	Joy 1 Fire 2	Joy1 Fire 1	Joy1 right	Joy1 left	Joy1 down	Joy1 up
                                        //En cpc hacemos que tanto del y backspace del teclado sean el DEL del cpc
                                        cpc_keyboard_table[9] &=(255-128);

                                        //En pcw tecla borrar hacia derecha
                                        pcw_keyboard_table[2] &=(255-1);
                                }
                                else {
                                        //del en cpc es del
                                        cpc_keyboard_table[9] |=128;

                                        pcw_keyboard_table[2] |=1;
                                }
                                //printf("%d\n",cpc_keyboard_table[9]);
                        break;


                        case UTIL_KEY_SHIFT_L:
                        case UTIL_KEY_CAPS_SHIFT:

                                if (pressrelease) menu_capshift.v=1;
                                else menu_capshift.v=0;

                                if (pressrelease) {
                                        puerto_65278  &=255-1;
                                        blink_kbd_a14 &= (255-64);
					                              cpc_keyboard_table[2] &=255-32;
                                        ql_keyboard_table[7] &= (255-1);
                                        msx_keyboard_table[6] &=(255-1);
                                        svi_keyboard_table[6] &=(255-1);
                                        pcw_keyboard_table[2] &=(255-32);
                                }
                                else  {
                                        puerto_65278 |=1;
                                        blink_kbd_a14 |= 64;
                                        ql_keyboard_table[7] |= 1;
					cpc_keyboard_table[2] |=32;
                                        msx_keyboard_table[6] |=1;
                                        svi_keyboard_table[6] |=1;
                                        pcw_keyboard_table[2] |=32;

                                    //Si se libera left shift, reiniciamos contador de conmutacion de ventanas
                                    menu_contador_conmutar_ventanas=0;
                                    //printf("reset contador conmutacion\n");
                                }
                        break;

                        case UTIL_KEY_SHIFT_R:
                                if (pressrelease) menu_capshift.v=1;
                                else menu_capshift.v=0;

                                if (pressrelease) {
                                        puerto_65278  &=255-1;
                                        blink_kbd_a15 &= (255-128);
					                              cpc_keyboard_table[2] &=255-32;
                                        ql_keyboard_table[7] &= (255-1);
                                        msx_keyboard_table[6] &=(255-1);
                                        svi_keyboard_table[6] &=(255-1);
                                        pcw_keyboard_table[2] &=(255-32);
                                }
                                else  {
                                        puerto_65278 |=1;
                                        blink_kbd_a15 |= 128;
					                              cpc_keyboard_table[2] |=32;
                                        ql_keyboard_table[7] |= 1;
                                        msx_keyboard_table[6] |=1;
                                        svi_keyboard_table[6] |=1;
                                        pcw_keyboard_table[2] |=32;
                                }
                        break;

        case UTIL_KEY_ALT_L:

            util_press_menu_symshift(pressrelease);

            //printf ("Pulsado ctrl o Alt");
            if (MACHINE_IS_ZX8081) {
                //para zx80/81
                //aqui hace lo mismo que mayusculas
                if (pressrelease) puerto_65278  &=255-1;
                else  puerto_65278 |=1;
            }


            else {

                //puerto_32766    db              255  ; B    N    M    Simb Space ;7

                if (pressrelease) {
                    puerto_32766  &=255-2;
                    blink_kbd_a15 &= (255-64);

                    //ALT en CPC es CLR
                    cpc_keyboard_table[2] &=(255-1);
                    ql_keyboard_table[7] &= (255-4);

                    //ALT en MSX es stop
                    msx_keyboard_table[7] &= (255-16);
                    svi_keyboard_table[6] &= (255-32);
                    pcw_keyboard_table[0xA] &=(255-128);
                }


                else  {
                    puerto_32766 |=2;
                    blink_kbd_a15 |= 64;

                    //ALT en CPC es CLR
                    cpc_keyboard_table[2] |=1;
                    ql_keyboard_table[7] |= 4;

                    //ALT en MSX es stop
                    msx_keyboard_table[7] |= 16;
                    svi_keyboard_table[6] |= 32;
                    pcw_keyboard_table[0xA] |=128;
                }
            }
        break;

        case UTIL_KEY_ALT_R:
            util_press_menu_symshift(pressrelease);

            //printf ("Pulsado ALT R\n");
            if (MACHINE_IS_ZX8081) {
                //para zx80/81
                //aqui hace lo mismo que mayusculas
                if (pressrelease) puerto_65278  &=255-1;
                else  puerto_65278 |=1;
            }


            //En SAM coupe, alt derecha es Edit
            else if (MACHINE_IS_SAM) {
                if (pressrelease) {
                    puerto_teclado_sam_bff9 &= 255-128;
                }
                else	puerto_teclado_sam_bff9 |=128;
            }


            else {

                //puerto_32766    db              255  ; B    N    M    Simb Space ;7

                if (pressrelease) {
                    puerto_32766  &=255-2;
                    blink_kbd_a15 &= (255-64);

                    //ALT en CPC es CLR
                    cpc_keyboard_table[2] &=(255-1);
                    ql_keyboard_table[7] &= (255-4);
                    pcw_keyboard_table[1] &=(255-2);
                    //printf("PTR\n");
                }


                else  {
                    puerto_32766 |=2;
                    blink_kbd_a15 |= 64;

                    //ALT en CPC es CLR
                    cpc_keyboard_table[2] |=1;
                    ql_keyboard_table[7] |= 4;
                    pcw_keyboard_table[1] |=2;
                }
            }
        break;


        case UTIL_KEY_CONTROL_L:

            util_press_menu_symshift(pressrelease);

            //printf ("Pulsado ctrl");
            if (MACHINE_IS_ZX8081) {
                //para zx80/81
                //aqui hace lo mismo que mayusculas
                if (pressrelease) puerto_65278  &=255-1;
                else  puerto_65278 |=1;
            }

            //En sam es otra tecla

            else if (MACHINE_IS_SAM) {
                if (pressrelease) puerto_65534 &=255-1;
                else puerto_65534 |=1;
            }


            else {



                //puerto_32766    db              255  ; B    N    M    Simb Space ;7

                if (pressrelease) {
                    puerto_32766  &=255-2;
                    blink_kbd_a14 &= (255-16);
                    cpc_keyboard_table[2] &=(255-128);
                    msx_keyboard_table[6] &=(255-2);
                    ql_keyboard_table[7] &= (255-2);
                    pcw_keyboard_table[0xA] &=(255-2);


                }


                else  {
                    puerto_32766 |=2;
                    blink_kbd_a14 |= 16;
                    cpc_keyboard_table[2] |=128;
                    msx_keyboard_table[6] |=2;
                    ql_keyboard_table[7] |= 2;
                    pcw_keyboard_table[0xA] |=2;

                }
            }
        break;

        case UTIL_KEY_CONTROL_R:

            util_press_menu_symshift(pressrelease);

            //printf ("Pulsado ctrl");
            if (MACHINE_IS_ZX8081) {
                //para zx80/81
                //aqui hace lo mismo que mayusculas
                if (pressrelease) puerto_65278  &=255-1;
                else  puerto_65278 |=1;
            }

            //En sam es otra tecla

            else if (MACHINE_IS_SAM) {
                if (pressrelease) puerto_65534 &=255-1;
                else puerto_65534 |=1;
            }


            else {



                //puerto_32766    db              255  ; B    N    M    Simb Space ;7

                if (pressrelease) {
                    puerto_32766  &=255-2;
                    blink_kbd_a14 &= (255-16);
                    cpc_keyboard_table[2] &=(255-128);
                    msx_keyboard_table[6] &=(255-2);
                    ql_keyboard_table[7] &= (255-2);
                    pcw_keyboard_table[1] &=(255-1);


                }


                else  {
                    puerto_32766 |=2;
                    blink_kbd_a14 |= 16;
                    cpc_keyboard_table[2] |=128;
                    msx_keyboard_table[6] |=2;
                    ql_keyboard_table[7] |= 2;
                    pcw_keyboard_table[1] |=1;

                }
            }
        break;

        case UTIL_KEY_WINKEY_L:
            //printf("winkey L\n");
            if (pressrelease) {
                pcw_keyboard_table[2] &=(255-128);
            }
            else {
                pcw_keyboard_table[2] |=128;
            }

        break;

        case UTIL_KEY_WINKEY_R:
            //printf("winkey R\n");
            if (pressrelease) {
                pcw_keyboard_table[0xA] &=(255-8);
            }
            else {
                pcw_keyboard_table[0xA] |=8;
            }

        break;


                        //Teclas que generan doble pulsacion
                        case UTIL_KEY_BACKSPACE:
                                if (pressrelease) menu_backspace.v=1;
                                else menu_backspace.v=0;

                                if (MACHINE_IS_SAM) {
                                        if (pressrelease) puerto_teclado_sam_eff9 &= (255-128);
                                        else              puerto_teclado_sam_eff9 |= 128;
                                }

				else {


// 6|   Ret   Left     Up    Esc  Right      \  Space   Down     ql_keyboard_table[1]
// 5|     ]      z      .      c      b  Pound      m      '     ql_keyboard_table[2]
// 4|     [   Caps      k      s      f      =      g      ;     ql_keyboard_table[3]
// 3|     l      3      h      1      a      p      d      j     ql_keyboard_table[4]
// 2|     9      w      i    Tab      r      -      y      o     ql_keyboard_table[5]
// 1|     8      2      6      q      e      0      t      u     ql_keyboard_table[6]
// 0| Shift   Ctrl    Alt      x      v      /      n      ,     ql_keyboard_table[7]

	                                if (pressrelease) {
        	                                puerto_65278 &=255-1;
                	                        puerto_61438 &=255-1;
                        	                blink_kbd_a8 &= (255-128);

                                                        //En cpc hacemos que tanto del y backspace del teclado sean el DEL del cpc
						                              cpc_keyboard_table[9] &=(255-128);
                                                msx_keyboard_table[7] &=(255-32);
                                                svi_keyboard_table[5] &=(255-64);
                                          //ql_pressed_backspace=1;
                                          ql_keyboard_table[7] &=(255-2);
                                          ql_keyboard_table[1] &=(255-2);
                                          pcw_keyboard_table[9] &=(255-128);

	                                }
        	                        else {
                	                        puerto_65278 |=1;
                        	                puerto_61438 |=1;
                                	        blink_kbd_a8 |= 128;
						                              cpc_keyboard_table[9] |=128;
                                                msx_keyboard_table[7] |=32;
                                                svi_keyboard_table[5] |=64;
                                          //ql_pressed_backspace=0;
                                          ql_keyboard_table[7] |=2;
                                          ql_keyboard_table[1] |=2;
                                          pcw_keyboard_table[9] |=128;
	                                }
				}
                        break;


                        case UTIL_KEY_FIRE:
                                if (pressrelease) {
                                        joystick_set_fire(1);


                                }
                                else {
                                        joystick_release_fire(1);


                                }
                        break;

                        case UTIL_KEY_LEFT:
                                if (pressrelease) {
                                        //puerto_65278 &=255-1;
                                        //puerto_63486 &=255-16;
                                        joystick_set_left(1);
                                        blink_kbd_a12 &= (255-64);
					                    cpc_keyboard_table[1] &=(255-1);
                                        msx_keyboard_table[8] &=(255-16);
                                        svi_keyboard_table[6] &=(255-128);

          // 1|   Ret   Left     Up    Esc  Right      \  Space   Down
                                        ql_keyboard_table[1] &= (255-2);
                                        pcw_keyboard_table[1] &=(255-128);
                                }
                                else {
                                        //puerto_65278 |=1;
                                        //puerto_63486 |=16;
                                        joystick_release_left(1);
                                        blink_kbd_a12 |= 64;
					                    cpc_keyboard_table[1] |=1;
                                        msx_keyboard_table[8] |=16;
                                        svi_keyboard_table[6] |=128;
                                        ql_keyboard_table[1] |= 2;
                                        pcw_keyboard_table[1] |=128;
                                }
                        break;
                        case UTIL_KEY_RIGHT:
                                if (pressrelease) {
                                        //puerto_65278 &=255-1;
                                        //puerto_61438 &=255-4;
                                        joystick_set_right(1);
                                        blink_kbd_a11 &= (255-64);
					                              cpc_keyboard_table[0] &=(255-2);
                                        msx_keyboard_table[8] &=(255-128);
                                        svi_keyboard_table[8] &=(255-128);
                                        ql_keyboard_table[1] &= (255-16);
                                        pcw_keyboard_table[0] &=(255-64);
                                }
                                else {
                                        //puerto_65278 |=1;
                                        //puerto_61438 |=4;
                                        joystick_release_right(1);
                                        blink_kbd_a11 |= 64;
                                        cpc_keyboard_table[0] |=2;
                                        msx_keyboard_table[8] |=128;
                                        svi_keyboard_table[8] |=128;
                                        ql_keyboard_table[1] |= 16;
                                        pcw_keyboard_table[0] |=64;
                                }
                        break;

                        case UTIL_KEY_DOWN:
                                if (pressrelease) {
                                        //puerto_65278 &=255-1;
                                        //puerto_61438 &=255-16;
                                        joystick_set_down(1);

                                        blink_kbd_a10 &= (255-64);
					cpc_keyboard_table[0] &=(255-4);
                                        msx_keyboard_table[8] &=(255-64);
                                        svi_keyboard_table[7] &=(255-128);
                                        ql_keyboard_table[1] &= (255-128);
                                        pcw_keyboard_table[0xA] &=(255-64);

                                }
                                else {
                                        //puerto_65278 |=1;
                                        //puerto_61438 |=16;
                                        joystick_release_down(1);

                                        blink_kbd_a10 |= 64;
					cpc_keyboard_table[0] |=4;
                                        msx_keyboard_table[8] |=64;
                                        svi_keyboard_table[7] |=128;
                                        ql_keyboard_table[1] |= 128;
                                        pcw_keyboard_table[0xA] |=64;
                                }
                        break;

                        case UTIL_KEY_UP:
                                if (pressrelease) {
                                        //puerto_65278 &=255-1;
                                        //puerto_61438 &=255-8;
                                        joystick_set_up(1);
                                        blink_kbd_a9 &= (255-64);
					cpc_keyboard_table[0] &=(255-1);
                                        msx_keyboard_table[8] &=(255-32);
                                        svi_keyboard_table[5] &=(255-128);
                                        ql_keyboard_table[1] &= (255-4);
                                        pcw_keyboard_table[1] &=(255-64);
                                }
                                else {
                                        //puerto_65278 |=1;
                                        //puerto_61438 |=8;
                                        joystick_release_up(1);
                                        blink_kbd_a9 |= 64;
					cpc_keyboard_table[0] |=1;
                                        msx_keyboard_table[8] |=32;
                                        svi_keyboard_table[5] |=128;
                                        ql_keyboard_table[1] |=4;
                                        pcw_keyboard_table[1] |=64;
                                }
                        break;


                        //las 5 son botones de joystick que vienen desde ZENG (entrade de comando por ZRCP) exclusivamente.
                        //Se diferencian de las anteriores en que no vuelven a generar evento ZENG de nuevo
                        case UTIL_KEY_JOY_FIRE:
                                if (pressrelease) {
                                        joystick_set_fire(0);


                                }
                                else {
                                        joystick_release_fire(0);


                                }
                        break;

                        case UTIL_KEY_JOY_LEFT:
                                if (pressrelease) {
                                        joystick_set_left(0);
                                        blink_kbd_a12 &= (255-64);
					cpc_keyboard_table[1] &=(255-1);

          // 1|   Ret   Left     Up    Esc  Right      \  Space   Down
                                        ql_keyboard_table[1] &= (255-2);
                                }
                                else {
                                        joystick_release_left(0);
                                        blink_kbd_a12 |= 64;
					                              cpc_keyboard_table[1] |=1;
                                        ql_keyboard_table[1] |= 2;
                                }
                        break;
                        case UTIL_KEY_JOY_RIGHT:
                                if (pressrelease) {
                                        joystick_set_right(0);
                                        blink_kbd_a11 &= (255-64);
					                              cpc_keyboard_table[0] &=(255-2);
                                        ql_keyboard_table[1] &= (255-16);
                                }
                                else {
                                        joystick_release_right(0);
                                        blink_kbd_a11 |= 64;
                                        cpc_keyboard_table[0] |=2;
                                        ql_keyboard_table[1] |= 16;
                                }
                        break;

                        case UTIL_KEY_JOY_DOWN:
                                if (pressrelease) {
                                        joystick_set_down(0);

                                        blink_kbd_a10 &= (255-64);
					cpc_keyboard_table[0] &=(255-4);
                                        ql_keyboard_table[1] &= (255-128);

                                }
                                else {
                                        joystick_release_down(0);

                                        blink_kbd_a10 |= 64;
					cpc_keyboard_table[0] |=4;
                                        ql_keyboard_table[1] |= 128;
                                }
                        break;

                        case UTIL_KEY_JOY_UP:
                                if (pressrelease) {
                                        joystick_set_up(0);
                                        blink_kbd_a9 &= (255-64);
					cpc_keyboard_table[0] &=(255-1);
                                        ql_keyboard_table[1] &= (255-4);
                                }
                                else {
                                        joystick_release_up(0);
                                        blink_kbd_a9 |= 64;
					cpc_keyboard_table[0] |=1;
                                        ql_keyboard_table[1] |=4;
                                }
                        break;





                        case UTIL_KEY_TAB:
                                if (pressrelease) {
                                        menu_tab.v=1;
                                        //printf ("Pulsado TAB\n");
                                }
                                else {
                                        menu_tab.v=0;
                                        //printf ("Liberado TAB\n");
                                }

				if (MACHINE_IS_SAM) {
					if (pressrelease) puerto_teclado_sam_f7f9 &= (255-64);
					else              puerto_teclado_sam_f7f9 |= 64;
				}

				else {
	                                if (pressrelease) {
                                        	puerto_65278 &=255-1;
                                	        puerto_32766 &=255-2;
                        	                blink_kbd_a14 &= (255-32);
						                              cpc_keyboard_table[8]&= (255-16);
                                          // 2|     9      w      i    Tab      r      -      y      o     ql_keyboard_table[5]

                                          msx_keyboard_table[7] &=(255-8);
                                          ql_keyboard_table[5] &=(255-8);

                                          pcw_keyboard_table[8]&= (255-16);
        	                        }



	                                else {
                                	        puerto_65278 |=1;
                        	                puerto_32766 |=2;
                	                        blink_kbd_a14 |= 32;
						                              cpc_keyboard_table[8]|= 16;
                                        msx_keyboard_table[7] |=8;
                                          ql_keyboard_table[5] |=8;

                                          pcw_keyboard_table[8]|= 16;
	                                }
				}
                        break;

                        case UTIL_KEY_CAPS_LOCK:
				if (MACHINE_IS_SAM) {
                                        if (pressrelease) puerto_teclado_sam_f7f9 &= (255-128);
                                        else              puerto_teclado_sam_f7f9 |= 128;
                                }

				else {

	                                if (pressrelease) {
        	                                puerto_65278 &=255-1;
                	                        puerto_63486 &=255-2;
                        	                blink_kbd_a15 &= (255-8);
                                                cpc_keyboard_table[8] &=(255-64);
                                                // 4|     [   Caps      k      s      f      =      g      ;     ql_keyboard_table[3]
                                                msx_keyboard_table[6] &=(255-8);
                                                svi_keyboard_table[8] &=(255-8);
                                                ql_keyboard_table[3] &=(255-2);
                                                pcw_keyboard_table[8] &=(255-64);

                                                //Tambien el led del shift lock
                                                pcw_keyboard_table[0xD] &=(255-64);

                                                //printf("CAPS\n");
	                                }
        	                        else {
                	                        puerto_65278 |=1;
                        	                puerto_63486 |=2;
                                	        blink_kbd_a15 |= 8;
                                                cpc_keyboard_table[8] |=64;
                                                msx_keyboard_table[6] |=8;
                                                svi_keyboard_table[8] |=8;
                                                ql_keyboard_table[3] |=2;
                                                pcw_keyboard_table[8] |=64;

                                                pcw_keyboard_table[0xD] |=64;
	                                }
				}
                        break;

                        case UTIL_KEY_COMMA:
                        case ',':
                                util_press_menu_symshift(pressrelease);

                                int actuar_como_zx8081=0;

                                if (MACHINE_IS_ZX8081) {
                                    actuar_como_zx8081=1;

                                    //Con menu abierto, se genera tecla sym+n como en spectrum
                                    if (menu_abierto && zxvision_keys_event_not_send_to_machine) {
                                        actuar_como_zx8081=0;
                                    }
                                }

				if (actuar_como_zx8081) {
					if (pressrelease) {
						puerto_32766 &=255-2;
						puerto_65278 &=255-1;
					}
					else {
						puerto_32766 |=2;
						puerto_65278 |=1;
					}
				}


				else {


	                                if (pressrelease) {
        	                                puerto_32766 &=255-2-8;

                	                }
                        	        else {
                                	        puerto_32766 |=2+8;

	                                }


					//Temp para CPC. Quiza hay que tener en cuenta keymap
					if (pressrelease) {
                                                cpc_keyboard_table[4] &=255-128;
                                                pcw_keyboard_table[4] &=255-128;
                                        }
                                        else {
                                                cpc_keyboard_table[4] |=128;
                                                pcw_keyboard_table[4] |=128;
                                        }
				}
                        break;


                        //Punto
                        case UTIL_KEY_PERIOD:
                        case '.':

                                if (MACHINE_IS_PCW) {
                                    //printf("Period\n");
                                    if (pressrelease) {
                                        pcw_keyboard_table[3] &=(255-128);
                                    }
                                    else {
                                        pcw_keyboard_table[3] |=128;
                                    }
                                }
                                else {
                                    util_press_menu_symshift(pressrelease);

                                    int actuar_como_zx8081=0;

                                    if (MACHINE_IS_ZX8081) {
                                        actuar_como_zx8081=1;

                                        //Con menu abierto, se genera tecla sym+m como en spectrum
                                        if (menu_abierto && zxvision_keys_event_not_send_to_machine) {
                                            actuar_como_zx8081=0;
                                        }
                                    }


                                    if (actuar_como_zx8081) {

                                            //para zx80/81
                                                    if (pressrelease) puerto_32766  &=255-2;
                                            else  puerto_32766 |=2;
                                    }

                                    else {
                                            if (pressrelease) {
                                                    puerto_32766 &=255-2-4;

                                            }
                                            else {
                                                    puerto_32766 |=2+4;

                                            }
                                    }
                                }



                        break;

                        //F1 pulsado
                        case UTIL_KEY_F1:

                                if (pressrelease) {
                                        blink_kbd_a14 &= (255-128);
                                        ql_keyboard_table[0] &= (255-2);
                                        msx_keyboard_table[6] &= (255-32);
                                        svi_keyboard_table[7] &=(255-1);
                                        pcw_keyboard_table[0] &=(255-4);


                                }
                                else {
                                        blink_kbd_a14 |= 128;
                                        ql_keyboard_table[0] |= 2;
                                        msx_keyboard_table[6] |= 32;
                                        svi_keyboard_table[7] |=1;
                                        pcw_keyboard_table[0] |=4;


                                }
                        break;

                        //F2 pulsado
                        case UTIL_KEY_F2:

                                if (pressrelease) {
                                        blink_kbd_a15 &= (255-16);
                                        ql_keyboard_table[0] &= (255-8);
                                        msx_keyboard_table[6] &= (255-64);
                                        svi_keyboard_table[7] &=(255-2);
                                        //F2 es F1 + shift en pcw
                                        pcw_keyboard_table[0] &=(255-4);
                                        pcw_keyboard_table[2] &=(255-32); //shift


                                }
                                else {
                                        blink_kbd_a15 |= 16;
                                        ql_keyboard_table[0] |= 8;
                                        msx_keyboard_table[6] |= 64;
                                        svi_keyboard_table[7] |=2;

                                        pcw_keyboard_table[0] |=4;
                                        pcw_keyboard_table[2] |=32; //shift


                                }
                        break;

//// 7|    F4     F1      5     F2     F3     F5      4      7
                        //F3 pulsado
                        case UTIL_KEY_F3:

                                if (pressrelease) {
                                        blink_kbd_a14 &= (255-8);
                                        ql_keyboard_table[0] &= (255-16);
                                        msx_keyboard_table[6] &= (255-128);
                                        svi_keyboard_table[7] &=(255-4);
                                        pcw_keyboard_table[0] &=(255-1);
                                }
                                else {
                                        blink_kbd_a14 |= 8;
                                        ql_keyboard_table[0] |= 16;
                                        msx_keyboard_table[6] |= 128;
                                        svi_keyboard_table[7] |=4;
                                        pcw_keyboard_table[0] |=1;
                                }
                        break;

			//F4 pulsado. Volcar pantalla a speech si el menu esta cerrado
			case UTIL_KEY_F4:
			if (pressrelease) {
                                   if (!menu_abierto) textspeech_enviar_speech_pantalla();

                                   ql_keyboard_table[0] &= (255-1);
                                   msx_keyboard_table[7] &= (255-1);
                                   svi_keyboard_table[7] &=(255-8);
                                   //F4 en pcw es F3+Shift
                                   pcw_keyboard_table[0] &=(255-1);
                                   pcw_keyboard_table[2] &=(255-32); //shift
                                 }

                                 else {
                                   ql_keyboard_table[0] |= 1;
                                   msx_keyboard_table[7] |= 1;
                                   svi_keyboard_table[7] |=8;

                                   pcw_keyboard_table[0] |=1;
                                   pcw_keyboard_table[2] |=32;
                                 }
		        break;

                        //F5 pulsado
                        case UTIL_KEY_F5:

                                if (pressrelease) {
					                    if (util_if_open_just_menu() )  {
                                            //printf("Disparar evento abrir menu\n");
                                            menu_fire_event_open_menu();
                                        }
                                        //printf("Pulsado F5\n");
                                        ql_keyboard_table[0] &= (255-32); //Caso especial F5 y QL

                                        msx_keyboard_table[7] &= (255-2);
                                        svi_keyboard_table[7] &=(255-16);

                                        pcw_keyboard_table[0xA] &=(255-1);
                                }
                                else {
                                        ql_keyboard_table[0] |= 32;
                                        msx_keyboard_table[7] |= 2;
                                        svi_keyboard_table[7] |=16;

                                        pcw_keyboard_table[0xA] |=1;
                                }
                        break;

                        //z80_byte puerto_especial3=255; //  F10 F9 F8 F7 F6

                        //F6 pulsado. De momento no hace nada
                        case UTIL_KEY_F6:

                                if (pressrelease) {
                                   //F6 en pcw es F5+Shift
                                   pcw_keyboard_table[0xA] &=(255-1);
                                   pcw_keyboard_table[2] &=(255-32); //shift
                                }
                                else {
                                   pcw_keyboard_table[0xA] |=1;
                                   pcw_keyboard_table[2] |=32; //shift
                                }
                        break;

                        //F7 pulsado. Uso del simulador de joystick si esta habilitado
                        case UTIL_KEY_F7:

                                if (pressrelease) {
					if (simulador_joystick) {
						simulador_joystick_forzado=1;
					}
                                    pcw_keyboard_table[0xA] &=(255-16);

                                }
                                else {
                                    pcw_keyboard_table[0xA] |=16;
                                }
                        break;


			//F8 pulsado. On Screen keyboard, pero en pcw no
                        case UTIL_KEY_F8:

                            if (MACHINE_IS_PCW) {
                                if (pressrelease) {
                                    //F8 en pcw es F7+Shift
                                    pcw_keyboard_table[0xA] &=(255-16);
                                    pcw_keyboard_table[2] &=(255-32); //shift
                                }
                                else {
                                    pcw_keyboard_table[0xA] |=16;
                                    pcw_keyboard_table[2] |=32; //shift
                                }
                            }

                            else {

                                if (pressrelease) {
                                    menu_abierto=1;
                                    menu_button_osdkeyboard.v=1;
                                }

                            }

                        break;


                        //F9 pulsado. quickload
                        case UTIL_KEY_F9:

                                if (pressrelease) {
                                        menu_abierto=1;
                                        menu_button_smartload.v=1;
                                }
                                else {
                                }

                        break;

                        //F10 pulsado
                        case UTIL_KEY_F10:

                                if (pressrelease) {
                                }
                                else {
                                }
                        break;


                        //z80_byte puerto_especial3=255; //  F10 F9 F8 F7 F6


                        //z80_byte puerto_especial4=255; //  F15 F14 F13 F12 F11

                        //F11 pulsado. De momento no hace nada
                        case UTIL_KEY_F11:

                                if (pressrelease) {
                                }
                                else {
                                }
                        break;

                        //F12 pulsado. De momento no hace nada
                        case UTIL_KEY_F12:

                                if (pressrelease) {
                                }
                                else {
                                }
                        break;

                        //F13 pulsado. De momento no hace nada
                        case UTIL_KEY_F13:

                                if (pressrelease) {
                                }
                                else {
                                }
                        break;

                        //F14 pulsado. De momento no hace nada
                        case UTIL_KEY_F14:

                                if (pressrelease) {
                                }
                                else {
                                }
                        break;

                        //F15 pulsado. De momento no hace nada
                        case UTIL_KEY_F15:

                                if (pressrelease) {
                                }
                                else {
                                }
                        break;




                        //ESC pulsado. Para Z88 y tecla menu
                        case UTIL_KEY_ESC:
//A15 (#7) | RSH    SQR     ESC     INDEX   CAPS    .       /       £
                          //ESC para Spectrum es BREAK, siempre que no este text to speech habilitado
                            if (MACHINE_IS_SPECTRUM && textspeech_filter_program==NULL) {
                                if (pressrelease) {
                                    puerto_65278 &=255-1;
                                    puerto_32766 &=255-1;
                                }
                                else {
                                    puerto_65278 |=1;
                                    puerto_32766 |=1;
                                }
                            }


                            if (pressrelease) {
                                blink_kbd_a15 &= (255-32);
                                puerto_especial1 &=255-1;
                                cpc_keyboard_table[8] &=(255-4);
                                msx_keyboard_table[7] &=(255-4);
                                svi_keyboard_table[6] &= (255-16);
                                puerto_teclado_sam_f7f9 &= (255-32);
                                // 6|   Ret   Left     Up    Esc  Right      \  Space   Down     ql_keyboard_table[1]
                                ql_keyboard_table[1] &= (255-8);

                                //En PCW su tecla STOP es nuestro ESC
                                pcw_keyboard_table[8] &=(255-4);

                                //Vaciamos cola speech
                                textspeech_empty_speech_fifo();

                            }
                            else {
                                blink_kbd_a15 |=32;
                                puerto_especial1 |=1;
                                cpc_keyboard_table[8] |=4;
                                msx_keyboard_table[7] |=4;
                                svi_keyboard_table[6] |= 16;
                                puerto_teclado_sam_f7f9 |= 32;
                                ql_keyboard_table[1] |= 8;

                                pcw_keyboard_table[8] |=4;
                            }



			            break;

                        //PgUP
                        case UTIL_KEY_PAGE_UP:

                                if (pressrelease) {
                                        puerto_especial1 &=255-2;
                                }
                                else {
                                        puerto_especial1 |=2;
                                }
                        break;

                        //PgDn
                        case UTIL_KEY_PAGE_DOWN:

                                if (pressrelease) {
                                        puerto_especial1 &=255-4;
                                }
                                else {
                                        puerto_especial1 |=4;
                                }
                        break;


			//Teclas del keypad
            //Los de arriba del numero 7, en keypad UK
            //numlock  /  *  -

                        case UTIL_KEY_KP_NUMLOCK:
                                if (pressrelease) {
                                    pcw_keyboard_table[0xA] &=(255-4);
                                }
                                else {
                                    pcw_keyboard_table[0xA] |=4;
                                }
                        break;


                        case UTIL_KEY_KP_DIVIDE:
                                if (MACHINE_IS_PCW) {
                                    if (pressrelease) {
                                        pcw_keyboard_table[1] &=(255-4);
                                    }
                                    else {
                                        pcw_keyboard_table[1] |=4;
                                    }
                                }

                                else {

                                    if (pressrelease) {
                                            set_symshift();
                                            puerto_65278 &=255-16;
                                    }
                                    else {
                                            clear_symshift();
                                            puerto_65278 |=16;
                                    }
                                }
                        break;

                        case UTIL_KEY_KP_MULTIPLY:
                                if (MACHINE_IS_PCW) {
                                    if (pressrelease) {
                                        pcw_keyboard_table[1] &=(255-8);
                                    }
                                    else {
                                        pcw_keyboard_table[1] |=8;
                                    }
                                }

                                else {
                                    if (pressrelease) {
                                            set_symshift();
                                            puerto_32766 &=255-16;
                                    }
                                    else {
                                            clear_symshift();
                                            puerto_32766 |=16;
                                    }
                                }
                        break;

                        case UTIL_KEY_KP_MINUS:
                                if (MACHINE_IS_PCW) {
                                    if (pressrelease) {
                                        pcw_keyboard_table[0] &=(255-8);
                                    }
                                    else {
                                        pcw_keyboard_table[0] |=8;
                                    }
                                }

                                else {
                                    if (pressrelease) {
                                            set_symshift();
                                            puerto_49150 &=255-8;
                                    }
                                    else {
                                            clear_symshift();
                                            puerto_49150 |=8;
                                    }
                                }
                        break;



            case UTIL_KEY_KP_PLUS:
                    if (pressrelease) {
                            set_symshift();
                            puerto_49150 &=255-4;
                    }
                    else {
                            clear_symshift();
                            puerto_49150 |=4;
                    }
            break;

			case UTIL_KEY_KP0:
				if (pressrelease) {
					cpc_keyboard_table[1] &=(255-128);
					puerto_teclado_sam_dff9 &=(255-128);
                    pcw_keyboard_table[0] &=(255-2);
				}
				else {
					cpc_keyboard_table[1] |=128;
					puerto_teclado_sam_dff9 |=128;
                    pcw_keyboard_table[0] |=2;
				}
			break;

			case UTIL_KEY_KP1:
				if (pressrelease) {
					cpc_keyboard_table[1] &=(255-32);
					puerto_teclado_sam_fef9 &=(255-32);
                    pcw_keyboard_table[1] &=(255-128);
				}
				else {
					cpc_keyboard_table[1] |=32;
					puerto_teclado_sam_fef9 |=32;
                    pcw_keyboard_table[1] |=128;
				}
			break;

			case UTIL_KEY_KP2:
				if (pressrelease) {
					cpc_keyboard_table[1] &=(255-64);
					puerto_teclado_sam_fef9 &=(255-64);
                    pcw_keyboard_table[0] &=(255-128);
				}
				else {
					cpc_keyboard_table[1] |=64;
					puerto_teclado_sam_fef9 |=64;
                    pcw_keyboard_table[0] |=128;
				}
			break;

			case UTIL_KEY_KP3:
				if (pressrelease) {
					cpc_keyboard_table[0] &=(255-32);
					puerto_teclado_sam_fef9 &=(255-128);
                    pcw_keyboard_table[0] &=(255-64);
				}
				else {
					cpc_keyboard_table[0] |=32;
					puerto_teclado_sam_fef9 |=128;
                    pcw_keyboard_table[0] |=64;
				}
			break;

			case UTIL_KEY_KP4:
				if (pressrelease) {
					cpc_keyboard_table[2] &=(255-16);
					puerto_teclado_sam_fdf9 &=(255-32);
                    pcw_keyboard_table[1] |=(255-32);
				}
				else {
					cpc_keyboard_table[2] |=16;
					puerto_teclado_sam_fdf9 |=32;
                    pcw_keyboard_table[1] |=32;
				}
			break;

			case UTIL_KEY_KP5:
				if (pressrelease) {
					cpc_keyboard_table[1] &=(255-16);
					puerto_teclado_sam_fdf9 &=(255-64);
                    pcw_keyboard_table[1] &=(255-64);
				}
				else {
					cpc_keyboard_table[1] |=16;
					puerto_teclado_sam_fdf9 |=64;
                    pcw_keyboard_table[1] |=64;
				}
			break;

			case UTIL_KEY_KP6:
				if (pressrelease) {
					cpc_keyboard_table[0] &=(255-16);
					puerto_teclado_sam_fdf9 &=(255-128);
                    pcw_keyboard_table[0] &=(255-32);
				}
				else {
					cpc_keyboard_table[0] |=16;
					puerto_teclado_sam_fdf9 |=128;
                    pcw_keyboard_table[0] |=32;
				}
			break;

			case UTIL_KEY_KP7:
				if (pressrelease) {
					cpc_keyboard_table[1] &=(255-4);
					puerto_teclado_sam_fbf9 &=(255-32);
                    pcw_keyboard_table[2] &=(255-16);
				}
				else {
					cpc_keyboard_table[1] |=4;
					puerto_teclado_sam_fbf9 |=32;
                    pcw_keyboard_table[2] |=16;
				}
			break;

			case UTIL_KEY_KP8:
				if (pressrelease) {
					cpc_keyboard_table[1] &=(255-8);
					puerto_teclado_sam_fbf9 &=(255-64);
                    pcw_keyboard_table[1] &=(255-16);
				}
				else {
					cpc_keyboard_table[1] |=8;
					puerto_teclado_sam_fbf9 |=64;
                    pcw_keyboard_table[1] |=16;
				}
			break;

			case UTIL_KEY_KP9:
				if (pressrelease) {
					cpc_keyboard_table[0] &=(255-8);
					puerto_teclado_sam_fbf9 &=(255-128);
                    pcw_keyboard_table[0] &=(255-16);
				}
				else {
					cpc_keyboard_table[0] |=8;
					puerto_teclado_sam_fbf9 |=128;
                    pcw_keyboard_table[0] |=16;
				}
			break;

			case UTIL_KEY_KP_COMMA:
				if (pressrelease) {
					cpc_keyboard_table[0] &=(255-128);
                    pcw_keyboard_table[0xA] &=(255-64);
				}
				else {
					cpc_keyboard_table[0] |=128;
                    pcw_keyboard_table[0xA] |=64;
				}
			break;

			case UTIL_KEY_KP_ENTER:
                if (pressrelease) {
                    if (keyboard_swap_enter_return.v==0) {
                        cpc_keyboard_table[0] &=(255-64);
                        pcw_keyboard_table[0xA] &=(255-32);
                    }
                    else {
                        cpc_keyboard_table[2] &= (255-4);
                        pcw_keyboard_table[2] &=(255-4);
                    }
                }


                else {
                    if (keyboard_swap_enter_return.v==0) {
                        cpc_keyboard_table[0] |=64;
                        pcw_keyboard_table[0xA] |=32;
                    }

                    else {
                        cpc_keyboard_table[2] |= 4;
                        pcw_keyboard_table[2] |= 4;
                    }
                }
			break;


			case UTIL_KEY_NONE:
				//Esto no se usa aqui, solo lo usa en la rutina de Chloe. Lo pongo para que no se queje de Warning al compilar
				//Ademas esto identifica cuando no habia tecla previa en Chloe, no tiene sentido aqui en el switch gestionarlo
			break;

			//NO: Chapuza. Para gestion de z y b del teclado recreated
			//La solucion elegante seria que para activar un puerto de tecla, se usase solo util_set_reset_keys,
			//y nadie llamase a convert_numeros_letras_puerto_teclado . Para ello, todas las teclas fuera de lo ascii (ctrl, alt, etc)
			//deben tener una numeracion fuera de lo ascii normal (128 en adelante?)
			//Esta chapuza que hago tambien esta provocando que se llame a la funcion de recreated_zx_spectrum_keyboard_convert
			//desde dos sitios, cuando deberia llamarse solo desde un sitio
			//case 'z':
			//case 'b':
			//	convert_numeros_letras_puerto_teclado_continue_after_recreated(tecla,pressrelease);
			//break;


			default:
				//Caso entre a-z y 0-9
			if (
                        (tecla>='a' && tecla<='z') ||
                        (tecla>='0' && tecla<='9')
			)
				{
					convert_numeros_letras_puerto_teclado_continue_after_recreated(tecla,pressrelease);
				}
			break;




	}
}

//Memoria para labels
labeltree *parse_string_labeltree=NULL;

unsigned int parse_string_to_number_label(char *texto)
{
    labeltree *found=labeltree_find_element(parse_string_labeltree,texto);

    if (found==NULL) {
        //No hay labels cargadas
        return 0;
    }

    //Si encontrado exacto
    if (!strcasecmp(texto,found->name)) {
        return found->value;
    }

    //Si no encontrado exacto, retornar 0
    return 0;
}

void parse_string_to_number_add_label(char *texto,int numero)
{
    labeltree *l=labeltree_add_element(parse_string_labeltree,texto,numero);

    //primer elemento
    if (l!=NULL) parse_string_labeltree=l;
}

void labels_load_parse_one_line(z80_byte *memoria)
{
    //Separar linea en
    //LABEL[espacios, tabs, dos puntos :]VALOR
    //printf("Parseando linea: [%s]\n",memoria);

    z80_byte *label=memoria;
    int longitud=strlen((char *)memoria);

    //buscar separador
    while ( longitud && *memoria!=9 && *memoria!=32 && *memoria!=':') {
        memoria++;
        longitud--;
    }

    if (!longitud) {
        //fin de linea sin leer separador
        return;
    }

    *memoria=0;

    memoria++;
    longitud--;

    //saltar separadores hasta valor
    while ( longitud && (*memoria==9 || *memoria==32 || *memoria==':') ) {
        memoria++;
        longitud--;
    }

    z80_byte *valor=memoria;

    //printf("Parseamos label: [%s] valor: [%s]\n",label,valor);

    int valor_numero=parse_string_to_number((char *)valor);

    parse_string_to_number_add_label((char *)label,valor_numero);

}

void labels_load_parse_lines(z80_byte *memoria,int longitud)
{
    z80_byte *inicio_linea=memoria;

    //Separar cada linea
    while (1) {
        //printf("longitud: %d\n",longitud);
        //Buscar cr/lf o final de archivo
        if ((*memoria==10 || *memoria==13) || longitud==0) {
            *memoria=0;
            labels_load_parse_one_line(inicio_linea);

            //Siguiente caracter
            memoria++;
            longitud--;

            if (longitud>0) {

                //buscar siguiente linea
                while ( (*memoria==10 || *memoria==13) && longitud>0) {
                    memoria++;
                    longitud--;
                }

            }

            //fin de archivo
            if (longitud<=0) return;

            inicio_linea=memoria;
        }

        else {
            memoria++;
            longitud--;
        }
    }


}

//Cargar archivo de etiquetas
void labels_load(char *archivo)
{

    //Liberar si habia alguno
    labeltree_free(parse_string_labeltree);

    parse_string_labeltree=NULL;

    if (!si_existe_archivo(archivo)) {
        debug_printf(VERBOSE_ERR,"File %s does not exist",archivo);
        return;
    }

    long long int longitud=get_file_size(archivo);

    //Si longitud 0 (cosa extraña) volver sin mas
    if (!longitud) return;

    //Asignamos 1 byte mas para el 0 del final
    z80_byte *buffer_labels=util_malloc(longitud+1,"Can not allocate memory to read labels file");

    int total_leidos=util_load_file_bytes(buffer_labels,archivo,longitud);

    if (!total_leidos) return;

    //Ir procesando linea a linea
    labels_load_parse_lines(buffer_labels,longitud);

    free(buffer_labels);
}

//Retorna numero parseado. Si acaba en H, se supone que es hexadecimal
//Si acaba en
//Si empieza por ' o "" es un caracter (1 solo caracter, no un string)
//Valor de retorno unsigned de 32 bits
//Da tipo valor segun:
/*
enum token_parser_formato {
	TPF_DECIMAL,
	TPF_HEXADECIMAL,
	TPF_BINARIO,
	TPF_ASCII
}; */

int parse_string_is_hexa(char *texto,int longitud)
{
    int i;

    for (i=0;i<longitud;i++) {
        if (!
            (
            (texto[i]>='a' && texto[i]<='f') ||
            (texto[i]>='A' && texto[i]<='F') ||
            (texto[i]>='0' && texto[i]<='9')

            )
        ) {
            return 0;
        }
    }

    return 1;

}



unsigned int parse_string_to_number_get_type(char *texto,enum token_parser_formato *tipo_valor)
{
	int value;

        //Asumimos decimal
        *tipo_valor=TPF_DECIMAL;

	int l=strlen(texto);
	if (l==0) return 0;


	//Si empieza por ' o ""
	if (texto[0]=='\'' || texto[0]=='"') {
                *tipo_valor=TPF_ASCII;
                return texto[1];
        }

	//sufijo. Buscar ultimo caracter antes de final de cadena o espacio o parentesis de cierre. Asi podemos parsear cosas como "20H 32 34", y se interpretara solo el 20H
    //Nota: ese parentesis de cierre se usa, en principio, solamente desde assemble.c cuando el valor a parsear es de tipo
    //ASM_PARM_PARENTHESIS_N o ASM_PARM_PARENTHESIS_NN. Esto no deberia dar ningun problema, pero por si acaso...
        int posicion_sufijo=0;
        for (;texto[posicion_sufijo]!=0 && texto[posicion_sufijo]!=' ' && texto[posicion_sufijo]!=')';posicion_sufijo++);
        posicion_sufijo--;

  //int posicion_sufijo=l-1;


	char sufijo=texto[posicion_sufijo];
	if (sufijo=='H' || sufijo=='h') {
		//hexadecimal
        //Vamos a comprobar que todo lo que haya este en el rango 0..9 a..f A...F
        //Por que si no, un label que se llame por ejemplo "Research" pensara que es un valor hexadecimal, porque acaba en h
        if (parse_string_is_hexa(texto,posicion_sufijo)) {
            //quitamos sufijo y parseamos
            texto[posicion_sufijo]=0;
            value=strtol(texto, NULL, 16);
            //volvemos a dejar sufijo tal cual
            texto[posicion_sufijo]=sufijo;

                    *tipo_valor=TPF_HEXADECIMAL;
            return value;
        }
	}

        if (sufijo=='%') {
		//binario
		//quitamos sufijo y parseamos
		texto[posicion_sufijo]=0;
		value=strtol(texto, NULL, 2);
		//volvemos a dejar sufijo tal cual
		texto[posicion_sufijo]=sufijo;

                *tipo_valor=TPF_BINARIO;
		return value;
	}

    //Si empieza por letra, es un label
    if ( (texto[0]>='A' && texto[0]<='Z') ||
         (texto[0]>='a' && texto[0]<='z')
        ) {
            //printf("Paseamos [%s] con parse_string_to_number_label\n",texto);
        return parse_string_to_number_label(texto);
    }

	//decimal
        return atoi(texto);

}


//Retorna numero parseado. Si acaba en H, se supone que es hexadecimal
//Si acaba en
//Si empieza por ' o "" es un caracter (1 solo caracter, no un string)
//Valor de retorno unsigned de 32 bits
unsigned int parse_string_to_number(char *texto)
{
        enum token_parser_formato tipo_valor;
        return parse_string_to_number_get_type(texto,&tipo_valor);
}

/*int ascii_to_hexa(char c)
{
  if (c>='0' && c<='9') return c-'0';
  if (c>='a' && c<='f') return c-'a'+10;
  if (c>='A' && c<='F') return c-'A'+10;
  return 0;
}*/

//Retorna numero grande parseado. Si acaba en H, se supone que es hexadecimal
//Si empieza por ' o "" es un caracter (1 solo caracter, no un string)
/*//Valor de retorno unsigned de 64 bits
long long int parse_string_to_long_number(char *texto)
{
	long long int value;

  while (*texto && *texto!=' ') {
    printf("t: [%c]",*texto);
    value=value*16;
    value +=ascii_to_hexa(*texto);
    texto++;
  }

  return value;


}*/


//carpeta temporal usada por el emulador
char emulator_tmpdir[PATH_MAX]="";

//carpeta temporal que ha especificado el usuario
char emulator_tmpdir_set_by_user[PATH_MAX]="";

//Retorna directorio temporal para el usuario y lo crea
//Formato: /tmp/zesarux-<uid>
//donde <uid> es el uid del usuario
char *get_tmpdir_base(void)
{

	//Si lo ha cambiado el usuario
	if (emulator_tmpdir_set_by_user[0]!=0) strcpy(emulator_tmpdir,emulator_tmpdir_set_by_user);


	if (emulator_tmpdir[0]==0) {

#ifndef MINGW
		int uid=getuid();
		sprintf (emulator_tmpdir,"/tmp/zesarux-%d",uid);
#else
		//Obtener ruta temporal raiz c:\windows
		char windows_temp[PATH_MAX];

		char *env_tmp,*env_temp,*env_userprofile,*env_systemroot;
        	env_tmp=getenv("TMP");
        	env_temp=getenv("TEMP");
		env_userprofile=getenv("USERPROFILE");
		env_systemroot=getenv("SystemRoot");


		//Ir probando uno a uno
		if (env_tmp!=NULL) sprintf (windows_temp,"%s",env_tmp);
		else if (env_temp!=NULL) sprintf (windows_temp,"%s",env_temp);
		else if (env_userprofile!=NULL) sprintf (windows_temp,"%s",env_userprofile);
		else if (env_systemroot!=NULL) sprintf (windows_temp,"%s",env_systemroot);
		else {
			//como ultima instancia, c:\windows\temp
			sprintf (windows_temp,"%s","C:\\windows\\temp\\");
		}

		debug_printf (VERBOSE_DEBUG,"Windows Temporary dir: %s",windows_temp);

		//Obtener un uid unico para usuario
		char template_dir[PATH_MAX];
		sprintf (template_dir,"%s\\zesarux-XXXXXX",windows_temp);

		char *dir;
		dir=mkdtemp(template_dir);

                if (dir==NULL) {
                        debug_printf (VERBOSE_DEBUG,"Error getting temporary directory: %s",strerror(errno) );
                }
                else {
                        sprintf(emulator_tmpdir,"%s",dir);
			debug_printf (VERBOSE_DEBUG,"ZEsarUX Temporary dir: %s",emulator_tmpdir);
                }

#endif


		debug_printf (VERBOSE_INFO,"Creating new temporary directory %s",emulator_tmpdir);


#ifndef MINGW
	      mkdir(emulator_tmpdir,S_IRWXU);
#else
	      mkdir(emulator_tmpdir);
#endif


	}

	return emulator_tmpdir;
}

#define FRECUENCIA_SONIDO_RWA_FILES 15600

int convert_smp_to_rwa(char *origen, char *destino)
{

	FILE *ptr_origen;
	ptr_origen=fopen(origen,"rb");
	if (ptr_origen==NULL) {
		debug_printf (VERBOSE_ERR,"Error reading source file");
		return 1;
	}

        FILE *ptr_destino;
        ptr_destino=fopen(destino,"wb");
        if (ptr_destino==NULL) {
		debug_printf (VERBOSE_ERR,"Error creating target file: %s",destino);
		return 1;
	}

	unsigned char last_value=0;
	unsigned char previous_last_value=0;

	#define FREQ_SMP 11111

	int contador=FRECUENCIA_SONIDO_RWA_FILES;
	int frecuencia_origen=FREQ_SMP;

	while (!feof(ptr_origen)) {

		//"Sampleamos" frecuencia original a 15600

		contador=contador-frecuencia_origen;
		if (contador<=0) {
			fread(&last_value, 1,1 , ptr_origen);
			contador +=FRECUENCIA_SONIDO_RWA_FILES;
		}



		//fwrite(&last_value,1,1,ptr_destino);
		//en vez de escribir ultimo valor tal cual, hacer la media de los dos anteriores
		int media=(previous_last_value+last_value)/2;
		unsigned char media8=media;
		fwrite(&media8,1,1,ptr_destino);

		previous_last_value=last_value;
	}


	fclose(ptr_origen);
	fclose(ptr_destino);

	return 0;

}

//Convertir wav a raw: 44100 hz, 8 bits, mono, unsigned
int convert_wav_to_raw(char *origen, char *destino)
{



	char sox_command[PATH_MAX];

	char sox_program[PATH_MAX];


	sprintf (sox_program,"%s",external_tool_sox);
	sprintf (sox_command,"%s \"%s\" -t .raw -r %d -b 8 -e unsigned -c 1 \"%s\"",external_tool_sox,origen,44100,destino);




	if (!si_existe_archivo(sox_program)) {
		debug_printf(VERBOSE_ERR,"Unable to find sox program: %s",sox_program);
		return 1;
	}


	debug_printf (VERBOSE_DEBUG,"Running %s command",sox_command);

	if (system (sox_command)==-1) {
		debug_printf (VERBOSE_DEBUG,"Error running command %s",sox_command);
		return 1;
	}

	return 0;

}

//Convertir wav a rwa: 15600 hz, 8 bits, mono, unsigned
int convert_wav_to_rwa(char *origen, char *destino)
{



	char sox_command[PATH_MAX];

	char sox_program[PATH_MAX];



	//sprintf (sox_program,"/usr/bin/sox");
	//sprintf (sox_command,"/usr/bin/sox \"%s\" -t .raw -r %d -b 8 -e unsigned -c 1 \"%s\"",origen,FRECUENCIA_SONIDO_RWA_FILES,destino);
	sprintf (sox_program,"%s",external_tool_sox);
	sprintf (sox_command,"%s \"%s\" -t .raw -r %d -b 8 -e unsigned -c 1 \"%s\"",external_tool_sox,origen,FRECUENCIA_SONIDO_RWA_FILES,destino);




	if (!si_existe_archivo(sox_program)) {
		debug_printf(VERBOSE_ERR,"Unable to find sox program: %s",sox_program);
		return 1;
	}


	debug_printf (VERBOSE_DEBUG,"Running %s command",sox_command);

	if (system (sox_command)==-1) {
		debug_printf (VERBOSE_DEBUG,"Error running command %s",sox_command);
		return 1;
	}

	return 0;

}

int convert_rwa_to_wav(char *origen, char *destino)
{



        char sox_command[PATH_MAX];

        char sox_program[PATH_MAX];



        sprintf (sox_program,"%s",external_tool_sox);
        //sprintf (sox_command,"%s \"%s\" -t .raw -r %d -b 8 -e unsigned -c 1 \"%s\"",external_tool_sox,origen,FRECUENCIA_SONIDO_RWA_FILES,destino);
        sprintf (sox_command,"%s -t .raw -r %d -b 8 -e unsigned -c 1 \"%s\" \"%s\"",external_tool_sox,FRECUENCIA_SONIDO_RWA_FILES,origen,destino);




        if (!si_existe_archivo(sox_program)) {
                debug_printf(VERBOSE_ERR,"Unable to find sox program: %s",sox_program);
                return 1;
        }


        debug_printf (VERBOSE_DEBUG,"Running %s command",sox_command);

        if (system (sox_command)==-1) {
                debug_printf (VERBOSE_DEBUG,"Error running command %s",sox_command);
                return 1;
        }

        return 0;

}

char tzx_frecuencia_sonido[10];

int convert_tzx_to_rwa(char *origen, char *destino)
{

        //char nombre_origen[NAME_MAX];
        //util_get_file_no_directory(origen,nombre_origen);

        //sprintf (destino,"%s/tmp_%s.rwa",get_tmpdir_base(),nombre_origen);
        //debug_printf (VERBOSE_INFO,"Creating temporary file %s",destino);

	//playtzx-0.12c/playtzx -au -freq 15600 Rambo.tzx Rambo.rwa
	char *argumentos[]={
		"playtzx","-au","-freq","",
        "-sil","",
        "",""
	};

	//sprintf (tzx_frecuencia_sonido,"%d",FRECUENCIA_SONIDO);
	//frecuencia de sonido a generar archivo siempre 15600, independientemente del % de cpu
	sprintf (tzx_frecuencia_sonido,"%d",FRECUENCIA_SONIDO_RWA_FILES);


	argumentos[3]=tzx_frecuencia_sonido;

    //Agregar 2 segundos de silencio al principio
    //Sin este silencio, la autocarga de Spectrum +3 no funciona, por que no le da tiempo entre que inicia la rom y se pone a cargar
    char buffer_silencio[20];
    sprintf(buffer_silencio,"%d",FRECUENCIA_SONIDO_RWA_FILES*2);
    argumentos[5]=buffer_silencio;

	argumentos[6]=origen;
	argumentos[7]=destino;

	return main_playtzx(8,argumentos);
}

void convert_o_p_to_rwa_write_silence(FILE *ptr_archivo,int segundos)
{
        int i;
        unsigned char escrito;

        escrito=128;
        for (i=0;i<FRECUENCIA_SONIDO_RWA_FILES*segundos;i++) {
                fwrite(&escrito,1,1,ptr_archivo);
        }

}


//escribe 1 o 0
void convert_o_p_to_rwa_write_bit(FILE *ptr_archivo,int bit,unsigned char valor_high,unsigned char valor_low)
{

/*
  0:  /\/\/\/\________
  1:  /\/\/\/\/\/\/\/\/\________

        //150 us high: byte:  1 1
	//byte 0 de punto medio
        //150 us low: byte: 0 0

        //silencio: 1300 us -> 22 bytes a 0


	Lo unico que depende de la frecuencia es esos 2 bytes a 1 y los dos bytes a 0 y los 22 bytes de silencio de final de bit

*/



	int longitud;

	//Bit a 1 son 9 ondas. Bit a 0 son 4 ondas
	if (bit) longitud=9;
	else longitud=4;

	z80_byte escrito;
	int i;
	for (i=0;i<longitud;i++) {

		//2 bytes arriba
		fwrite(&valor_high,1,1,ptr_archivo);
		fwrite(&valor_high,1,1,ptr_archivo);

		//1 byte 0. esto podria ser -1 a efectos practicos. pero se pone 0 para que "visualmente" quede mejor
		escrito=128;
		fwrite(&escrito,1,1,ptr_archivo);

		//2 bytes -1
		fwrite(&valor_low,1,1,ptr_archivo);
		fwrite(&valor_low,1,1,ptr_archivo);
	}

	escrito=128;
	for (i=0;i<22;i++) {
		fwrite(&escrito,1,1,ptr_archivo);
	}
}


void convert_o_p_to_rwa_write_byte(FILE *ptr_archivo,unsigned char leido)
{

#define ZX8081_AMPLITUD_BIT 50
                //meter los 8 bits. en orden, primero el mas alto
                int numerobit,bit;
                for (numerobit=0;numerobit<8;numerobit++) {
                        if (leido&128) bit=1;
                        else bit=0;

                        leido=leido<<1;


                        convert_o_p_to_rwa_write_bit(ptr_archivo,bit,128+ZX8081_AMPLITUD_BIT,128-ZX8081_AMPLITUD_BIT);
                }
}

int convert_o_p_to_rwa(char *origen, char *destino,int si_p)
{


/*
Bits and Bytes
Each byte consists of 8 bits (MSB first) without any start and stop bits, directly followed by the next byte. A "0" bit consists of four high pulses, a "1" bit of nine pulses, either one followed by a silence period.

  0:  /\/\/\/\________
  1:  /\/\/\/\/\/\/\/\/\________

Each pulse is split into a 150us High period, and 150us Low period. The duration of the silence between each bit is 1300us. The baud rate is thus 400 bps (for a "0" filled area) downto 250 bps (for a "1" filled area). Average medium transfer rate is approx. 307 bps (38 bytes/sec) for files that contain 50% of "0" and "1" bits each.

*/

	//en senyal rwa a 15600 hz, cada pulso de 150+150 us dura aprox 5 bytes -> 0,00032 s
	//300 us -> 1/1000000 * 300 = 0,0003

	//150 us high: byte:  1 1
	//byte 0 de punto medio
	//150 us low: byte: -1 -1

	//silencio: 1300 us -> 22 bytes a 0


        FILE *ptr_origen;
        ptr_origen=fopen(origen,"rb");
        if (ptr_origen==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading source file");
                return 1;
        }

        FILE *ptr_destino;
        ptr_destino=fopen(destino,"wb");
        if (ptr_destino==NULL) {
                debug_printf (VERBOSE_ERR,"Error creating target file: %s",destino);
                return 1;
        }

	//metemos 7 segundos de silencio
	convert_o_p_to_rwa_write_silence(ptr_destino,7);

        unsigned char leido;


	//Si ZX81, metemos nombre al principio.
    //En los P no hay el nombre al principio. En el audio si lo hay

    //Pero si es un P81, el nombre ya nos viene
    int si_p81=0;

    if (!util_compare_file_extension(origen,"p81")) si_p81=1;

	if (si_p && !si_p81) {


        //obtenemos nombre archivo y directorio por separado
        char nombre[NAME_MAX];
        char nombre_sin_ext[NAME_MAX];

        util_get_file_no_directory(origen,nombre);
        util_get_file_without_extension(nombre,nombre_sin_ext);

        debug_printf(VERBOSE_INFO,"Adding filename [%s] at the beginning of audio",nombre_sin_ext);

        int i;
        int longitud=strlen(nombre_sin_ext);

        for (i=0;i<longitud;i++) {

            //Metemos nombre
            //unsigned char nombre='A';
            unsigned char letra=ascii_to_zx81(nombre_sin_ext[i]); //38; //38 = A en ZX81
            if (i==longitud-1) letra |=128; //caracter del final

            convert_o_p_to_rwa_write_byte(ptr_destino,letra);
        }
	}


        //metemos datos
        while (!feof(ptr_origen)) {
		fread(&leido, 1,1 , ptr_origen);
		convert_o_p_to_rwa_write_byte(ptr_destino,leido);
        }

	//metemos 3 segundos de silencio
	convert_o_p_to_rwa_write_silence(ptr_destino,3);

        fclose(ptr_origen);
        fclose(ptr_destino);

        return 0;
}


int convert_o_to_rwa(char *origen, char *destino)
{
	return convert_o_p_to_rwa(origen,destino,0);
}

int convert_p_to_rwa(char *origen, char *destino)
{
        return convert_o_p_to_rwa(origen,destino,1);
}


void convert_tap_to_rwa_write_silence(FILE *ptr_archivo,int segundos)
{
        int i;
        unsigned char escrito;

        escrito=128;
        for (i=0;i<FRECUENCIA_SONIDO_RWA_FILES*segundos;i++) {
                fwrite(&escrito,1,1,ptr_archivo);
        }

}


#define SPEC_AMPLITUD_BIT 50

//Para acelerar las escrituras, tablas pregeneradas
unsigned char convert_tap_to_rwa_wave_high[]={
    128+SPEC_AMPLITUD_BIT,128+SPEC_AMPLITUD_BIT,
    128+SPEC_AMPLITUD_BIT,128+SPEC_AMPLITUD_BIT,
    128+SPEC_AMPLITUD_BIT,128+SPEC_AMPLITUD_BIT,
    128+SPEC_AMPLITUD_BIT,128+SPEC_AMPLITUD_BIT
};

unsigned char convert_tap_to_rwa_wave_low[]={
    128-SPEC_AMPLITUD_BIT,128-SPEC_AMPLITUD_BIT,
    128-SPEC_AMPLITUD_BIT,128-SPEC_AMPLITUD_BIT,
    128-SPEC_AMPLITUD_BIT,128-SPEC_AMPLITUD_BIT,
    128-SPEC_AMPLITUD_BIT,128-SPEC_AMPLITUD_BIT
};

//escribe 1 o 0
void convert_tap_to_rwa_write_bit(FILE *ptr_archivo,int bit)
{

	int longitud;

	//Bit a 1 son 4 bytes high , 4 low
    longitud=(bit==0 ? 4 : 8);


    //bytes arriba
    fwrite(&convert_tap_to_rwa_wave_high,1,longitud,ptr_archivo);


    //bytes abajo
    fwrite(&convert_tap_to_rwa_wave_low,1,longitud,ptr_archivo);


}


void convert_tap_to_rwa_write_byte(FILE *ptr_archivo,unsigned char leido)
{


    //meter los 8 bits. en orden, primero el mas alto
    int numerobit,bit;
    for (numerobit=0;numerobit<8;numerobit++) {
        bit=(leido&128 ? 1 : 0);
        //if (leido&128) bit=1;
        //else bit=0;

        leido=leido<<1;


        convert_tap_to_rwa_write_bit(ptr_archivo,bit);
    }
}

void convert_tap_to_rwa_write_pilot(FILE *ptr_archivo,z80_byte flag_bloque)
{

#define SPEC_AMPLITUD_PILOT 50

////Tono guia: 10 bytes high. 10 bytes low.

	int ondas;

        int longitud=10;

        z80_byte escrito;
        int i;

//8063 (Header) or 3223 (Data) Pilot Pulses
//flag 0: 8063/2
//flag 255: 3223/2

	int totalondas;

	int diferenciamaxmin=8063-3223;

	flag_bloque=255-flag_bloque;

	totalondas=3223+(diferenciamaxmin*flag_bloque/256);

	debug_printf (VERBOSE_DEBUG,"Number of pilot tone waves: %d",totalondas);

	for (ondas=0;ondas<totalondas/2;ondas++) {

	        for (i=0;i<longitud;i++) {
			escrito=128+SPEC_AMPLITUD_PILOT;
                	//bytes arriba
	                fwrite(&escrito,1,1,ptr_archivo);

        	}

	        for (i=0;i<longitud;i++) {
			escrito=128-SPEC_AMPLITUD_PILOT;

	                //bytes abajo
        	        fwrite(&escrito,1,1,ptr_archivo);

	        }
	}
}

void convert_tap_to_rwa_write_sync_false(FILE *ptr_archivo)
{

#define SPEC_AMPLITUD_SYNC_FALSE 50
        //Metemos onda "falsa" de sincronismo
        //convert_tap_to_rwa_write_sync_false(ptr_destino);
	//        //Primero sync pulse (high): 190us-> 3 bytes
        //Segundo sync pulse (low): 210us->3 bytes

	z80_byte escrito;
	escrito=128+SPEC_AMPLITUD_SYNC_FALSE;

	fwrite(&escrito,1,1,ptr_archivo);
	fwrite(&escrito,1,1,ptr_archivo);
	fwrite(&escrito,1,1,ptr_archivo);

        escrito=128-SPEC_AMPLITUD_SYNC_FALSE;

        fwrite(&escrito,1,1,ptr_archivo);
        fwrite(&escrito,1,1,ptr_archivo);
        fwrite(&escrito,1,1,ptr_archivo);
}




//Convierte archivo tap a rwa en destino indicado
int convert_tap_to_rwa(char *origen, char *destino)
{
        //char nombre_origen[NAME_MAX];
        //util_get_file_no_directory(origen,nombre_origen);

        //sprintf (destino,"%s/tmp_%s.rwa",get_tmpdir_base(),nombre_origen);
        //debug_printf (VERBOSE_INFO,"Creating temporary file %s",destino);




        FILE *ptr_origen;
        ptr_origen=fopen(origen,"rb");
        if (ptr_origen==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading source file");
                return 1;
        }

        FILE *ptr_destino;
        ptr_destino=fopen(destino,"wb");
        if (ptr_destino==NULL) {
                debug_printf (VERBOSE_ERR,"Error creating target file: %s",destino);
                return 1;
        }


/*
	//Tono guia: 10 bytes high. 10 bytes low. cada pulso entero (+1 y -1 ) dura: 0.001280 segundos. medio pulso dura 0.000641 -> aprox 619us/pulse. cada byte son 64.102564 us
	//Primero sync pulse (high): 190us-> 3 bytes
	//Segundo sync pulse (low): 210us->3 bytes

	//bit a 0: 244 us -> 3.8 bytes-> 4 bytes a high. 4 bytes a low -> 8 total onda que dura el bit 0
	//bit a 1: 488 us-> 7.6 bytes -> 8 bytes a high. 8 bytes a low ->16 total onda que dura el bit 1


Spectrum Cassette Blocks
  Silence (low level)
  8063 (Header) or 3223 (Data) Pilot Pulses (2168 clks/pulse) (619us/pulse)-> 0.000619
  1st Sync Pulse (667 clks/pulse) (190us/pulse)
  2nd Sync Pulse (735 clks/pulse) (210us/pulse)
  Blocktype Byte (00h=Header, FFh=Data)
  Data Byte(s) (two 855 or 1710 clks/pulse per bit) (244 or 488 us/pulse)
  Checksum Byte (above Blocktype and Data Bytes XORed with each other)
  End pulse (opposite level of last pulse)
  Silence (low level)

*/

	//Bucle hasta que acabe tap
	//metemos 2 segundos de silencio
	convert_tap_to_rwa_write_silence(ptr_destino,2);

	while (!feof(ptr_origen)) {

        	unsigned char leido;



		//Leer longitud de lo que viene
		z80_int longitud_bloque;

		fread(&leido, 1,1 , ptr_origen);

		//aqui puede haber feof
		if (!feof(ptr_origen)) {

			longitud_bloque=leido;

		        fread(&leido, 1,1 , ptr_origen);
        		longitud_bloque +=leido*256;

			z80_byte flag_bloque;
			fread(&flag_bloque, 1,1 , ptr_origen);

			debug_printf (VERBOSE_INFO,"Generating audio tape block. Length (without flag and checksum): %d, flag: %d",longitud_bloque-2,flag_bloque);

			//Meter tono guia, longitud de tono guia depende de flag
			convert_tap_to_rwa_write_pilot(ptr_destino,flag_bloque);

			//Metemos onda "falsa" de sincronismo
			convert_tap_to_rwa_write_sync_false(ptr_destino);

			//Metemos flag
			convert_tap_to_rwa_write_byte(ptr_destino,flag_bloque);
			//Meter datos
			//bucle de longitud bloque.

			//restamos el flag que ya esta escrito
			longitud_bloque--;

			while (longitud_bloque>0) {


				fread(&leido, 1,1 , ptr_origen);
				convert_tap_to_rwa_write_byte(ptr_destino,leido);

				longitud_bloque--;

			}

			//fin bucle longitud bloque

			//metemos 1 segundo de silencio
			convert_tap_to_rwa_write_silence(ptr_destino,1);

			//seguir a siguiente bloque dentro de tap
		}

	}


        fclose(ptr_origen);
        fclose(ptr_destino);

        return 0;
}

//cada cuantos estados escribimos un sample de audio
//224 significa a final de cada scanline -> 312*50=15600 hz
#define CONVERT_PZX_TSTATES_AUDIO_SAMPLE 224
#define CONVERT_PZX_AMPLITUD_PULSE 50

void convert_pzx_to_rwa_write_one_pulse(int valor_pulso,FILE *ptr_destino)
{
     	z80_byte escrito;

        if (valor_pulso) escrito=128+CONVERT_PZX_AMPLITUD_PULSE;
        else escrito=128-CONVERT_PZX_AMPLITUD_PULSE;

	fwrite(&escrito,1,1,ptr_destino);
}

void convert_pzx_to_rwa_write_pulses(int *p_t_estado_actual,int duracion_pulsos,int *p_valor_pulso_inicial,FILE *ptr_destino)
{



        //Usar los valores iniciales pasandolos a variables
        int t_estado_actual=*p_t_estado_actual;
        int valor_pulso_inicial=*p_valor_pulso_inicial;

        t_estado_actual %=CONVERT_PZX_TSTATES_AUDIO_SAMPLE;

        int final_final_estado=t_estado_actual+duracion_pulsos;


       //printf ("convert_pzx_to_rwa_write_pulses: state %d length %d initial_pulse %d\n",t_estado_actual,duracion_pulsos,valor_pulso_inicial);


/*
Bucle while (
mientras contador >=224
meter siguiente byte sample audio
contador -=224
)
*/


        while (t_estado_actual<final_final_estado) {
                //meter siguiente byte sample audio
                convert_pzx_to_rwa_write_one_pulse(valor_pulso_inicial,ptr_destino);

                t_estado_actual +=CONVERT_PZX_TSTATES_AUDIO_SAMPLE;
        }




       //Retornar los valores finales
       *p_t_estado_actual=final_final_estado; //t_estado_actual;
       *p_valor_pulso_inicial=valor_pulso_inicial;

}

//Nota: block_size entra como z80_long_int pero aqui tratamos con int con signo, para evitar que en
//el bucle while de abajo pueda irse a menor que 0 y nunca acabaria (esto pasaria en bloques corruptos)
int convert_pzx_to_rwa_tag_pzxt(z80_byte *memoria,int block_size)
{
        debug_printf (VERBOSE_DEBUG,"PZX: Start PZXT block");
/*
PZXT - PZX header block
-----------------------

offset type     name   meaning
0      u8       major  major version number (currently 1).
1      u8       minor  minor version number (currently 0).
2      u8[?]    info   tape info, see below.

This block distinguishes the PZX files from other files and may provide
additional info about the file as well. This block must be always present as
the first block of any PZX file.

Any implementation should check the version number before processing the
rest of the PZX file or even the rest of this block itself. Any
implementation should accept only files whose major version it implements
and reject anything else. However an implementation may report a warning in
case it encounters minor greater than it implements for given major, if the
implementor finds it convenient to do so.

Note that this block also allows for simple concatenation of PZX files. Any
implementation should thus check the version number not only in case of the
first block, but anytime it encounters this block anywhere in the file. This
in fact suggests that an implementation might decide not to treat the first
block specially in any way, except checking the file starts with this block
type, which is usually done because of file type recognition anyway.

The rest of the block data may provide additional info about the tape. Note
that an implementation is not required to process any of this information in
any way and may as well safely ignore it.

The additional information consists of sequence of strings, each terminated
either by character 0x00 or end of the block, whichever comes first.
This means the last string in the sequence may or may not be terminated.

The first string (if there is any) in the sequence is always the title of
the tape. The following strings (if there are any) form key and value pairs,
each value providing particular type of information according to the key.
In case the last value is missing, it should be treated as empty string.
The following keys are defined (for reference, the value in brackets is the
corresponding type byte as specified by the TZX standard):

Publisher  [0x01] - Software house/publisher
Author     [0x02] - Author(s)
Year       [0x03] - Year of publication
Language   [0x04] - Language
Type       [0x05] - Game/utility type
Price      [0x06] - Original price
Protection [0x07] - Protection scheme/loader
Origin     [0x08] - Origin
Comment    [0xFF] - Comment(s)

Note that some keys (like Author or Comment) may be used more than once.

Any encoding implementation must use any of the key names as they are listed
above, including the case. For any type of information not covered above, it
should use either the generic Comment field or invent new sensible key name
following the style used above. This allows any decoding implementation to
classify and/or localize any of the key names it understands, and use any
others verbatim.

Overall, same rules as for use in TZX files apply, for example it is not
necessary to specify the Language field in case all texts are in English.

*/


/*
PZXT - PZX header block
-----------------------

offset type     name   meaning
0      u8       major  major version number (currently 1).
1      u8       minor  minor version number (currently 0).
2      u8[?]    info   tape info, see below.
*/

        z80_byte pzx_version_major=memoria[0];
        z80_byte pzx_version_minor=memoria[1];

        debug_printf (VERBOSE_DEBUG,"PZX: file version: %d.%d",pzx_version_major,pzx_version_minor);

/*
 Any
implementation should accept only files whose major version it implements
and reject anything else. However an implementation may report a warning in
case it encounters minor greater than it implements for given major, if the
implementor finds it convenient to do so.
*/

        if (pzx_version_major>PZX_CURRENT_MAJOR_VERSION) {
                debug_printf (VERBOSE_ERR,"PZX: Can not handle this PZX version");
                return 1;
        }


        block_size -=2;
        memoria +=2;

        //Los strings los vamos guardando en un array de char separado. Asumimos que ninguno ocupa mas de 1024. Si es asi, esto petara...

        char text_string[1024];
        int index_string=0;

        while (block_size>0) {
                char caracter=*memoria;

                if (caracter==0) {
                        text_string[index_string++]=0;
                        //fin de cadena
                        debug_printf (VERBOSE_DEBUG,"PZX: info: %s",text_string);
                        index_string=0;
                }

                else {
                        text_string[index_string++]=util_return_valid_ascii_char(caracter);
                }

                memoria++;
                block_size--;

        }

        //Final puede haber acabado con byte 0 o no. Lo metemos por si acaso
        if (index_string!=0) {
                text_string[index_string++]=0;
                debug_printf (VERBOSE_DEBUG,"PZX: info: %s",text_string);
        }

        return 0;

}


//Nota: block_size entra como z80_long_int pero aqui tratamos con int con signo, para evitar que en
//el bucle while de abajo pueda irse a menor que 0 y nunca acabaria (esto pasaria en bloques corruptos)
void convert_pzx_to_rwa_tag_puls(z80_byte *memoria,int block_size,FILE *ptr_destino,int *p_t_estado_actual)
{


        debug_printf (VERBOSE_DEBUG,"PZX: Start PULS block");

/*
PULS - Pulse sequence
---------------------

offset type   name      meaning
0      u16    count     bits 0-14 optional repeat count (see bit 15), always greater than zero
                        bit 15 repeat count present: 0 not present 1 present
2      u16    duration1 bits 0-14 low/high (see bit 15) pulse duration bits
                        bit 15 duration encoding: 0 duration1 1 ((duration1<<16)+duration2)
4      u16    duration2 optional low bits of pulse duration (see bit 15 of duration1)
6      ...    ...       ditto repeated until the end of the block

This block is used to represent arbitrary sequence of pulses. The sequence
consists of pulses of given duration, with each pulse optionally repeated
given number of times. The duration may be up to 0x7FFFFFFF T cycles,
however longer durations may be achieved by concatenating the pulses by use
of zero pulses. The repeat count may be up to 0x7FFF times, however more
repetitions may be achieved by simply storing the same pulse again together
with another repeat count.

The optional repeat count is stored first. When present, it is stored as
16 bit value with bit 15 set. When not present, the repeat count is considered to be 1.
Note that the stored repeat count must be always greater than zero, so when
decoding, a value 0x8000 is not a zero repeat count, but prefix indicating the
presence of extended duration, see below.

The pulse duration itself is stored next. When it fits within 15 bits, it is
stored as 16 bit value as it is, with bit 15 not set. Otherwise the 15 high
bits are stored as 16 bit value with bit 15 set, followed by 16 bit value
containing the low 16 bits. Note that in the latter case the repeat count
must be present unless the duration fits within 16 bits, otherwise the
decoding implementation would treat the high bits as a repeat count.

The above can be summarized with the following pseudocode for decoding:

    count = 1 ;
    duration = fetch_u16() ;
    if ( duration > 0x8000 ) {
        count = duration & 0x7FFF ;
        duration = fetch_u16() ;
    }
    if ( duration >= 0x8000 ) {
        duration &= 0x7FFF ;
        duration <<= 16 ;
        duration |= fetch_u16() ;
    }

The pulse level is low at start of the block by default. However initial
pulse of zero duration may be easily used to make it high. Similarly, pulse
of zero duration may be used to achieve pulses lasting longer than
0x7FFFFFFF T cycles. Note that if the repeat count is present in case of
zero pulse for some reason, any decoding implementation must consistently
behave as if there was one zero pulse if the repeat count is odd and as if
there was no such pulse at all if it is even.

For example, the standard pilot tone of Spectrum header block (leader < 128)
may be represented by following sequence:

0x8000+8063,2168,667,735

The standard pilot tone of Spectrum data block (leader >= 128) would be:

0x8000+3223,2168,667,735

For the record, the standard ROM save routines create the pilot tone in such
a way that the level of the first sync pulse is high and the level of the
second sync pulse is low. The bit pulses then follow, each bit starting with
high pulse. The creators of the PZX files should use this information to
determine if they got the polarity of their files right. Note that although
most loaders are not polarity sensitive and would work even if the polarity
is inverted, there are some loaders which won't, so it is better to always
stick to this scheme.

*/


        z80_int count;
        int duration;

        int valor_pulso_inicial=0;
        int t_estado_actual=*p_t_estado_actual;



        while (block_size>0) {

                count = 1 ;

                //duration = fetch_u16() ;
                duration = (*memoria)|((memoria[1])<<8);
                memoria +=2;
                block_size -=2;

                if ( duration > 0x8000 ) {
                        count = duration & 0x7FFF ;
                        //duration = fetch_u16() ;
                        duration = (*memoria)|((memoria[1])<<8);
                        memoria +=2;
                        block_size -=2;
                }
                if ( duration >= 0x8000 ) {
                        duration &= 0x7FFF ;
                        duration <<= 16 ;
                        //duration |= fetch_u16() ;
                        duration |= (*memoria)|((memoria[1])<<8);
                        memoria +=2;
                        block_size -=2;
                }

                //printf ("count: %d duration: %d\n",count,duration);
                debug_printf(VERBOSE_DEBUG,"PZX: PULS: count: %d duration: %d",count,duration);
                while (count) {
                        //printf ("count=%d\n",count);
                        convert_pzx_to_rwa_write_pulses(&t_estado_actual,duration,&valor_pulso_inicial,ptr_destino);
                        count--;
                        //invertir pulso
                        valor_pulso_inicial=!valor_pulso_inicial;

                        //Truncar estados a multiple de scanline
                        t_estado_actual %=CONVERT_PZX_TSTATES_AUDIO_SAMPLE;

                }
        }

        *p_t_estado_actual=t_estado_actual;


}

void convert_pzx_to_rwa_tag_data(z80_byte *memoria,z80_long_int block_size GCC_UNUSED,FILE *ptr_destino,int *p_t_estado_actual)
{
        debug_printf (VERBOSE_DEBUG,"Start DATA block");
/*
DATA - Data block
-----------------

offset      type             name  meaning
0           u32              count bits 0-30 number of bits in the data stream
                                   bit 31 initial pulse level: 0 low 1 high
4           u16              tail  duration of extra pulse after last bit of the block
6           u8               p0    number of pulses encoding bit equal to 0.
7           u8               p1    number of pulses encoding bit equal to 1.
8           u16[p0]          s0    sequence of pulse durations encoding bit equal to 0.
8+2*p0      u16[p1]          s1    sequence of pulse durations encoding bit equal to 1.
8+2*(p0+p1) u8[ceil(bits/8)] data  data stream, see below.

This block is used to represent binary data using specified sequences of
pulses. The data bytes are processed bit by bit, most significant bits first.
Each bit of the data is represented by one of the sequences, s0 if its value
is 0 and s1 if its value is 1, respectively. Each sequence consists of
pulses of specified durations, p0 pulses for sequence s0 and p1 pulses for
sequence s1, respectively.

The initial pulse level is specified by bit 31 of the count field. For data
saved with standard ROM routines, it should be always set to high, as
mentioned in PULS description above. Also note that pulse of zero duration
may be used to invert the pulse level at start and/or the end of the
sequence. It would be also possible to use it for pulses longer than 65535 T
cycles in the middle of the sequence, if it was ever necessary.

For example, the sequences for standard ZX Spectrum bit encoding are:
(initial pulse level is high):

bit 0: 855,855
bit 1: 1710,1710

The sequences for ZX81 encoding would be (initial pulse level is high):

bit 0: 530, 520, 530, 520, 530, 520, 530, 4689
bit 1: 530, 520, 530, 520, 530, 520, 530, 520, 530, 520, 530, 520, 530, 520, 530, 520, 530, 4689

The sequence for direct recording at 44100kHz would be (assuming initial pulse level is low):

bit 0: 79,0
bit 1: 0,79

The sequence for direct recording resampled to match the common denominator
of standard pulse width would be (assuming initial pulse level is low):

bit 0: 855,0
bit 1: 0,855

After the very last pulse of the last bit of the data stream is output, one
last tail pulse of specified duration is output. Non zero duration is
usually necessary to terminate the last bit of the block properly, for
example for data block saved with standard ROM routine the duration of the
tail pulse is 945 T cycles and only then goes the level low again. Of course
the specified duration may be zero as well, in which case this pulse has no
effect on the output. This is often the case when multiple data blocks are
used to represent continuous stream of pulses.
*/


/*
offset      type             name  meaning
0           u32              count bits 0-30 number of bits in the data stream
                                   bit 31 initial pulse level: 0 low 1 high
4           u16              tail  duration of extra pulse after last bit of the block
6           u8               p0    number of pulses encoding bit equal to 0.
7           u8               p1    number of pulses encoding bit equal to 1.
8           u16[p0]          s0    sequence of pulse durations encoding bit equal to 0.
8+2*p0      u16[p1]          s1    sequence of pulse durations encoding bit equal to 1.
8+2*(p0+p1) u8[ceil(bits/8)] data  data stream, see below.
*/

        int initial_pulse;

        z80_long_int count;

        int t_estado_actual=*p_t_estado_actual;


        count=memoria[0]+
                (memoria[1]*256)+
                (memoria[2]*65536)+
                ((memoria[3]&127)*16777216);

        initial_pulse=(memoria[3]&128)>>7;

        memoria +=4;

        z80_int tail=memoria[0]+
                (memoria[1]*256);

        memoria +=2;

        z80_byte num_pulses_zero=*memoria;
        memoria++;

        z80_byte num_pulses_one=*memoria;
        memoria++;

        //Secuencias que identifican a un cero y un uno
        z80_int seq_pulses_zero[256];
        z80_int seq_pulses_one[256];

        //Metemos las secuencias de 0 y 1 en array
        int i;
        for (i=0;i<num_pulses_zero;i++) {
               seq_pulses_zero[i]=memoria[0]+(memoria[1]*256);

                memoria +=2;
        }


        for (i=0;i<num_pulses_one;i++) {
               seq_pulses_one[i]=memoria[0]+(memoria[1]*256);

                memoria +=2;
        }
        debug_printf (VERBOSE_DEBUG,"PZX: count: %d initial_pulse %d tail %d num_pulses_0 %d num_pulses_1 %d",
               count,initial_pulse,tail,num_pulses_zero,num_pulses_one);

        /*printf ("Secuence 0: ");
        for (i=0;i<num_pulses_zero;i++) {
               printf ("%d ",seq_pulses_zero[i]);
        }
        printf ("\n");

        printf ("Secuence 1: ");
        for (i=0;i<num_pulses_one;i++) {
               printf ("%d ",seq_pulses_one[i]);
        }
        printf ("\n");*/


        //Procesar el total de bits
        int bit_number=7;
        z80_byte processing_byte;

        z80_int *sequence_bit;
        int longitud_sequence_bit;

        z80_long_int total_bits_read;

        for (total_bits_read=0;total_bits_read<count;total_bits_read+=8) {
        //for (i=0;i<count;i+=8) {
                processing_byte=*memoria;
                for (bit_number=7;bit_number>=0;bit_number--) {

                        if (processing_byte & 128) {
                                //Writing bit to 1
                                //printf ("Writing bit to 1\n");
                                sequence_bit=&seq_pulses_one[0];
                                longitud_sequence_bit=num_pulses_one;
                        }
                        else {
                                //Writing bit to 0
                                //printf ("Writing bit to 0\n");
                                sequence_bit=&seq_pulses_zero[0];
                                longitud_sequence_bit=num_pulses_zero;
                        }


                        int contador_seq=0;
                        while (longitud_sequence_bit) {
                                z80_int duration=sequence_bit[contador_seq++];
                                convert_pzx_to_rwa_write_pulses(&t_estado_actual,duration,&initial_pulse,ptr_destino);
                                longitud_sequence_bit--;

                                //invertir pulso
                                initial_pulse=!initial_pulse;
                        }

                        processing_byte=processing_byte<<1;

                }
                memoria++;

                //Truncar estados a multiple de scanline
                t_estado_actual %=CONVERT_PZX_TSTATES_AUDIO_SAMPLE;

        }


        //Generar tail.
        convert_pzx_to_rwa_write_pulses(&t_estado_actual,tail,&initial_pulse,ptr_destino);

        *p_t_estado_actual=t_estado_actual;

}


void convert_pzx_to_rwa_tag_paus(z80_byte *memoria,z80_long_int block_size GCC_UNUSED,FILE *ptr_destino,int *p_t_estado_actual)
{
        debug_printf (VERBOSE_DEBUG,"PZX: Start PAUS block");
/*
offset type   name      meaning
0      u32    duration  bits 0-30 duration of the pause
                        bit 31 initial pulse level: 0 low 1 high

This block may be used to produce pauses during which the pulse level is not
particularly important. The pause consists of pulse of given duration and
given level. However note that some emulators may choose to simulate random
noise during this period, occasionally toggling the pulse level. In such case
the level must not be toggled during the first 70000 T cycles and then more
than once during each 70000 T cycles.

For example, in case of ZX Spectrum program saved by standard SAVE command
there is a low pulse of about one second (49-50 frames) between the header
and data blocks. On the other hand, there is usually no pause between the
blocks necessary at all, as long as the tail pulse of the preceding block was
used properly.

*/

        int initial_pulse;

        z80_long_int count;

        int t_estado_actual=*p_t_estado_actual;


        count=memoria[0]+
                (memoria[1]*256)+
                (memoria[2]*65536)+
                ((memoria[3]&127)*16777216);

        initial_pulse=(memoria[3]&128)>>7;

        memoria +=4;

        debug_printf(VERBOSE_DEBUG,"PZX: PAUS: count: %d",count);
        convert_pzx_to_rwa_write_pulses(&t_estado_actual,count,&initial_pulse,ptr_destino);


        *p_t_estado_actual=t_estado_actual;

}



//Convierte archivo pzx a rwa en destino indicado
int convert_pzx_to_rwa(char *origen, char *destino)
{


        //Leemos archivo en memoria
        z80_byte *pzx_file_mem;
        long long int bytes_to_load=get_file_size(origen);

        pzx_file_mem=malloc(bytes_to_load);
        if (pzx_file_mem==NULL) cpu_panic("Can not allocate memory for loading PZX file");

        FILE *ptr_origen;
        ptr_origen=fopen(origen,"rb");
        if (ptr_origen==NULL) {
                debug_printf (VERBOSE_ERR,"PZX: Error reading source file");
                return 1;
        }

        //int leidos=fread(pzx_file_mem,1,bytes_to_load,ptr_origen);
        fread(pzx_file_mem,1,bytes_to_load,ptr_origen);

        fclose(ptr_origen);

        FILE *ptr_destino;
        ptr_destino=fopen(destino,"wb");
        if (ptr_destino==NULL) {
                debug_printf (VERBOSE_ERR,"PZX: Error creating target file: %s",destino);
                return 1;
        }

        int estado_actual=0;

        //Ir leyendo hasta llegar al final del archivo
        //z80_long_int puntero_lectura=0;
        long long int puntero_lectura=0;

        while (puntero_lectura<bytes_to_load) {
                /*
                Leer datos identificador de bloque
                offset type     name   meaning
                0      u32      tag    unique identifier for the block type.
                4      u32      size   size of the block in bytes, excluding the tag and size fields themselves.
                8      u8[size] data   arbitrary amount of block data.
                */

                char tag_name[5];
                tag_name[0]=pzx_file_mem[puntero_lectura++];
                tag_name[1]=pzx_file_mem[puntero_lectura++];
                tag_name[2]=pzx_file_mem[puntero_lectura++];
                tag_name[3]=pzx_file_mem[puntero_lectura++];
                tag_name[4]=0;

                z80_long_int block_size;



                block_size=pzx_file_mem[puntero_lectura]+
                          (pzx_file_mem[puntero_lectura+1]*256)+
                          (pzx_file_mem[puntero_lectura+2]*65536)+
                          (pzx_file_mem[puntero_lectura+3]*16777216);
                puntero_lectura +=4;



                //printf ("Block tag name: [%s] size: [%u]\n",tag_name,block_size);


                //Tratar cada tag
                if (!strcmp(tag_name,"PZXT")) {
                      int ret=convert_pzx_to_rwa_tag_pzxt(&pzx_file_mem[puntero_lectura],block_size);
                      if (ret!=0) {
                              //La version es superior a la que gestionamos. Salir
                              return 1;
                      }
                }

                else if (!strcmp(tag_name,"PULS")) {
                      convert_pzx_to_rwa_tag_puls(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);
                }

                else if (!strcmp(tag_name,"DATA")) {
                      convert_pzx_to_rwa_tag_data(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);
                }

                else if (!strcmp(tag_name,"PAUS")) {
                      convert_pzx_to_rwa_tag_paus(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);
                }

                else {
                        debug_printf (VERBOSE_DEBUG,"PZX: Unknown block type: %02XH %02XH %02XH %02XH. Skipping it",
                        tag_name[0],tag_name[1],tag_name[2],tag_name[3]);
                }


                //Y saltar al siguiente bloque
                puntero_lectura +=block_size;

                //Truncar estados a multiple de scanline
                estado_actual %=CONVERT_PZX_TSTATES_AUDIO_SAMPLE;
        }



        fclose(ptr_destino);
        free(pzx_file_mem);

        return 0;
}


int convert_any_to_wav(char *origen, char *destino)
{
        //Primero pasar a rwa y luego a wav
        char rwa_temp_file[PATH_MAX];

	int result_first_convert;

	if (!util_compare_file_extension(origen,"tap")) result_first_convert=convert_tap_to_rwa_tmpdir(origen,rwa_temp_file);
	else if (!util_compare_file_extension(origen,"tzx")) result_first_convert=convert_tzx_to_rwa_tmpdir(origen,rwa_temp_file);
	else if (!util_compare_file_extension(origen,"smp")) result_first_convert=convert_smp_to_rwa_tmpdir(origen,rwa_temp_file);
	else if (!util_compare_file_extension(origen,"o")) result_first_convert=convert_o_to_rwa_tmpdir(origen,rwa_temp_file);
	else if (!util_compare_file_extension(origen,"p") || !util_compare_file_extension(origen,"p81")) result_first_convert=convert_p_to_rwa_tmpdir(origen,rwa_temp_file);
        else if (!util_compare_file_extension(origen,"pzx")) result_first_convert=convert_pzx_to_rwa_tmpdir(origen,rwa_temp_file);
	else return 1;

         if (result_first_convert) {
			//Error
                        return 1;
                }

                if (!si_existe_archivo(rwa_temp_file)) {
                        //debug_printf(VERBOSE_ERR,"Error converting input file. Target file not found");
                        return 1;
                }

        if (convert_rwa_to_wav(rwa_temp_file,destino)) {
                return 1;
        }


        return 0;
}

int convert_rmd_to_mdr_find_end_sync(int pos_raw,int *total_leidos,int total_size,z80_byte *microdrive_buffer_datos)
{
    z80_byte sync_lista[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

    while ((*total_leidos)<total_size) {


        z80_byte current_value=microdrive_buffer_datos[pos_raw++];
        if (pos_raw>=total_size) pos_raw=0; //soportar dar la vuelta

        (*total_leidos)++;

        //Rotar los anteriores
        sync_lista[7]=sync_lista[6];
        sync_lista[6]=sync_lista[5];
        sync_lista[5]=sync_lista[4];
        sync_lista[4]=sync_lista[3];
        sync_lista[3]=sync_lista[2];
        sync_lista[2]=sync_lista[1];
        sync_lista[1]=sync_lista[0];
        sync_lista[0]=current_value;

        if (sync_lista[0]==0xFF &&
            sync_lista[1]==0xFF &&
            sync_lista[2]==0x00 &&
            sync_lista[3]==0x00 &&
            sync_lista[4]==0x00 &&
            sync_lista[5]==0x00 &&
            sync_lista[6]==0x00 &&
            sync_lista[7]==0x00

        ) {
            return pos_raw;
        }

    }

    return pos_raw;
}

int convert_rmd_to_mdr_find_end_gap(int pos_raw,int *total_leidos,int total_size,z80_byte *microdrive_buffer_info,int *inicio_gap)
{
    //Buscar primer gap

    while ((microdrive_buffer_info[pos_raw] & 0x01)  && *total_leidos<total_size) {
        (*total_leidos)++;
        pos_raw++;
        if (pos_raw>=total_size) pos_raw=0; //soportar dar la vuelta
    }

    *inicio_gap=pos_raw;

    if (*total_leidos>=total_size) return pos_raw;

    //Buscar fin gap
    while ((microdrive_buffer_info[pos_raw] & 0x01)==0  && *total_leidos<total_size) {
        (*total_leidos)++;
        pos_raw++;
        if (pos_raw>=total_size) pos_raw=0; //soportar dar la vuelta
    }

    //printf("End gap en %d\n",pos_raw);
    return pos_raw;
}

int convert_rmd_to_mdr(char *origen, char *destino)
{

/*
    Microdrive cartridge
    GAP      PREAMBLE      15 byte      GAP      PREAMBLE      15 byte    512     1
    [-----][00 00 ... ff ff][BLOCK HEAD][-----][00 00 ... ff ff][REC HEAD][ DATA ][CHK]
    Preamble = 10 * 0x00 + 2 * 0xff (12 byte)

*algo como:
paso1:
buscar gap inicio gap y final gap. saltar 10 bytes 00 + 2 ff.
leer 15 bytes
saltar 10 bytes 00 + 2 ff.
leer 15 bytes+512+1
--esto es fin de sector
saltar a paso 1
*/

    //Leer tamanyo
    int tamanyo_archivo=get_file_size(origen);

    //restar cabecera
    tamanyo_archivo -=MICRODRIVE_RAW_HEADER_SIZE;

    //Si es tamaño invalido o impar (porque son registros de 16 bits y por tanto par)
    if (tamanyo_archivo<=0 || (tamanyo_archivo &1) ) {
        debug_printf(VERBOSE_ERR,"Invalid size for RAW microdrive file");
        return 1;
    }

	//int leidos;

	char buffer_cabecera[MICRODRIVE_RAW_HEADER_SIZE];

    FILE *ptr_inputfile;
    ptr_inputfile=fopen(origen,"rb");

    if (ptr_inputfile==NULL) {
            debug_printf (VERBOSE_ERR,"Error opening %s",origen);
            return 1;
    }




	//Leemos la cabecera
    fread(buffer_cabecera,1,MICRODRIVE_RAW_HEADER_SIZE,ptr_inputfile);

    int total_size=tamanyo_archivo/2;

    //leemos datos e info datos aparte
    z80_byte *microdrive_buffer_datos=util_malloc(total_size,"No enough memory for Microdrive buffer");
    z80_byte *microdrive_buffer_info=util_malloc(total_size,"No enough memory for Microdrive buffer");

    fread(microdrive_buffer_datos,1,total_size,ptr_inputfile);
    fread(microdrive_buffer_info,1,total_size,ptr_inputfile);



	fclose (ptr_inputfile);

    //Y buffer para mdr
    //Al menos espacio ocupado en raw
    z80_byte *microdrive_buffer_mdr=util_malloc(total_size,"No enough memory for Microdrive buffer");

    int pos_raw=0;
    int pos_mdr=0;

    int total_leidos=0;

/*
paso1:
buscar gap inicio gap y final gap. saltar 10 bytes 00 + 2 ff.
leer 15 bytes
buscar gap inicio gap y final gap
saltar 10 bytes 00 + 2 ff.
leer 15 bytes+512+1
--esto es fin de sector
saltar a paso 1
*/

    //Primero ver donde empieza el primer gap para ver todo lo que "descuenta" del principio
    int primer_gap_encontrado=0;


    while (total_leidos<total_size) {

        int inicio_gap;

        //printf("pos_raw: %d total_leidos: %d\n",pos_raw,total_leidos);

        //buscar gap inicio gap y final gap.
        pos_raw=convert_rmd_to_mdr_find_end_gap(pos_raw,&total_leidos,total_size,microdrive_buffer_info,&inicio_gap);
        if (total_leidos>=total_size) break;

        if (!primer_gap_encontrado) {
            //printf("primer gap en %d. pos_raw: %d total_leidos: %d\n",inicio_gap,pos_raw,total_leidos);
            primer_gap_encontrado=1;
            total_leidos -=inicio_gap; //para descontar lo que no es gap del principio
            //total_leidos--;
            if (total_leidos<0) total_leidos=0;

            //printf("despues primer gap: pos_raw: %d total_leidos: %d\n",pos_raw,total_leidos);

            //y poder dar la vuelta
            //TODO: realmente habria que buscar gap y ademas que fuese inicio de sector realmente, y ademas gap durante cierto rato
            //no vale con solo gap porque podria ser el gap que hay entre cabecera y datos
        }

        //saltar 10 bytes 00 + 2 ff.
        pos_raw=convert_rmd_to_mdr_find_end_sync(pos_raw,&total_leidos,total_size,microdrive_buffer_datos);
        if (total_leidos>=total_size) break;

        //leer 15 bytes
        int i;
        for (i=0;i<15 && total_leidos<total_size;i++) {
            microdrive_buffer_mdr[pos_mdr++]=microdrive_buffer_datos[pos_raw++];
            if (pos_raw>=total_size) pos_raw=0; //soportar dar la vuelta
            total_leidos++;
        }
        if (total_leidos>=total_size) break;

        //buscar gap inicio gap y final gap
        pos_raw=convert_rmd_to_mdr_find_end_gap(pos_raw,&total_leidos,total_size,microdrive_buffer_info,&inicio_gap);
        if (total_leidos>=total_size) break;

        //saltar 10 bytes 00 + 2 ff.
        pos_raw=convert_rmd_to_mdr_find_end_sync(pos_raw,&total_leidos,total_size,microdrive_buffer_datos);
        if (total_leidos>=total_size) break;

        //leer 15 bytes+512+1
        for (i=0;i<15+512+1 && total_leidos<total_size;i++) {
            microdrive_buffer_mdr[pos_mdr++]=microdrive_buffer_datos[pos_raw++];
            if (pos_raw>=total_size) pos_raw=0; //soportar dar la vuelta
            total_leidos++;
        }
        if (total_leidos>=total_size) break;
    }




	FILE *ptr_outputfile;
	ptr_outputfile=fopen(destino,"wb");

    if (ptr_outputfile==NULL) {
            debug_printf (VERBOSE_ERR,"Error opening %s",destino);
            return 1;
    }

    fwrite(microdrive_buffer_mdr,1,pos_mdr,ptr_outputfile);

    //Agregar el byte final que indica proteccion escritura o no. Lo ponemos como protegido
    z80_byte proteccion=1;

    fwrite(&proteccion,1,1,ptr_outputfile);

    fclose(ptr_outputfile);


    return 0;
}



int convert_mdr_to_rmd(char *origen, char *destino)
{

/*
    Microdrive cartridge
    GAP      PREAMBLE      15 byte      GAP      PREAMBLE      15 byte    512     1
    [-----][00 00 ... ff ff][BLOCK HEAD][-----][00 00 ... ff ff][REC HEAD][ DATA ][CHK]
    Preamble = 10 * 0x00 + 2 * 0xff (12 byte)


*/

    int gap_length=59;

    int sync_length=8;

    int end_sector_relleno=6*16; //valores a fc al final de sector de relleno, basurilla, podria ser cualquier cosa

    z80_byte sync_bytes[8]={0,0,0,0,0,0,0xff,0xff};


    int sector_raw_length=MDR_BYTES_PER_SECTOR+gap_length+sync_length+gap_length+sync_length+end_sector_relleno;

    //Leer tamanyo
    int tamanyo_archivo=get_file_size(origen);


    //Si es tamaño invalido o impar (porque son registros de 16 bits y por tanto par)
    if (tamanyo_archivo<=0) {
        debug_printf(VERBOSE_ERR,"Invalid size for MDR microdrive file");
        return 1;
    }

	//int leidos;


    //buffer para mdr
    z80_byte *microdrive_buffer_mdr=util_malloc(tamanyo_archivo,"No enough memory for Microdrive buffer");

    int total_sectors=tamanyo_archivo/MDR_BYTES_PER_SECTOR;




    FILE *ptr_inputfile;
    ptr_inputfile=fopen(origen,"rb");

    if (ptr_inputfile==NULL) {
            debug_printf (VERBOSE_ERR,"Error opening %s",origen);
            return 1;
    }

    fread(microdrive_buffer_mdr,1,tamanyo_archivo,ptr_inputfile);

	fclose (ptr_inputfile);


    //Espacio para datos e info
    int size_raw=total_sectors*sector_raw_length;

    z80_byte *microdrive_buffer_datos=util_malloc(size_raw,"No enough memory for Microdrive buffer");
    z80_byte *microdrive_buffer_info=util_malloc(size_raw,"No enough memory for Microdrive buffer");


    int pos_mdr=0;

    int pos_raw=0;


    while (total_sectors>0) {
        //printf("Remaining sectors: %d\n",total_sectors);

        //Meter gap
        int i;
        for (i=0;i<gap_length;i++) {
            microdrive_buffer_datos[pos_raw]=0;
            microdrive_buffer_info[pos_raw]=0;

            pos_raw++;
        }

        //Meter sync
        for (i=0;i<sync_length;i++) {
            microdrive_buffer_datos[pos_raw]=sync_bytes[i];
            microdrive_buffer_info[pos_raw]=(MICRODRIVE_RAW_INFO_BYTE_MASK_DATA>>8) & 0xFF;

            pos_raw++;
        }

        //Meter 15 bytes
        for (i=0;i<15;i++) {
            microdrive_buffer_datos[pos_raw]=microdrive_buffer_mdr[pos_mdr];
            microdrive_buffer_info[pos_raw]=(MICRODRIVE_RAW_INFO_BYTE_MASK_DATA>>8) & 0xFF;

            pos_raw++;
            pos_mdr++;
        }


        //Meter gap
        for (i=0;i<gap_length;i++) {
            microdrive_buffer_datos[pos_raw]=0;
            microdrive_buffer_info[pos_raw]=0;

            pos_raw++;
        }

        //Meter sync
        for (i=0;i<sync_length;i++) {
            microdrive_buffer_datos[pos_raw]=sync_bytes[i];
            microdrive_buffer_info[pos_raw]=(MICRODRIVE_RAW_INFO_BYTE_MASK_DATA>>8) & 0xFF;

            pos_raw++;
        }

        //Meter 15+512+1 bytes
        for (i=0;i<15+512+1;i++) {
            microdrive_buffer_datos[pos_raw]=microdrive_buffer_mdr[pos_mdr];
            microdrive_buffer_info[pos_raw]=(MICRODRIVE_RAW_INFO_BYTE_MASK_DATA>>8) & 0xFF;

            pos_raw++;
            pos_mdr++;
        }

        //Meter relleno
        for (i=0;i<end_sector_relleno;i++) {
            microdrive_buffer_datos[pos_raw]=0xFC;
            microdrive_buffer_info[pos_raw]=(MICRODRIVE_RAW_INFO_BYTE_MASK_DATA>>8) & 0xFF;

            pos_raw++;
        }


        total_sectors--;
    }




	FILE *ptr_outputfile;
	ptr_outputfile=fopen(destino,"wb");

    if (ptr_outputfile==NULL) {
            debug_printf (VERBOSE_ERR,"Error opening %s",destino);
            return 1;
    }


    z80_byte buffer_cabecera[MICRODRIVE_RAW_HEADER_SIZE];
    microdrive_raw_create_header(buffer_cabecera,0);

    fwrite(buffer_cabecera,1,MICRODRIVE_RAW_HEADER_SIZE,ptr_outputfile);

    fwrite(microdrive_buffer_datos,1,pos_raw,ptr_outputfile);
    fwrite(microdrive_buffer_info,1,pos_raw,ptr_outputfile);


    fclose(ptr_outputfile);


    return 0;
}

int convert_hdf_to_raw(char *origen, char *destino)
{

	int leidos;

	unsigned char buffer_lectura[1024];

        FILE *ptr_inputfile;
        ptr_inputfile=fopen(origen,"rb");

        if (ptr_inputfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error opening %s",origen);
                return 1;
        }

	FILE *ptr_outputfile;
	ptr_outputfile=fopen(destino,"wb");

        if (ptr_outputfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error opening %s",destino);
                return 1;
        }


	// Leer offset a datos raw del byte de cabecera:
	//0x09 DOFS WORD Image data offset This is the absolute offset in the HDF file where the actual hard-disk data dump starts.
	//In HDF version 1.1 this is 0x216.

	//Leemos 10 bytes de la cabecera
        fread(buffer_lectura,1,10,ptr_inputfile);

	int offset_raw=buffer_lectura[9]+256*buffer_lectura[10];

	debug_printf (VERBOSE_DEBUG,"Offset to raw data: %d",offset_raw);


	//Ya hemos leido 10 bytes del principio
	int saltar_bytes=offset_raw-10;

	//Saltar esos bytes
	fread(buffer_lectura,1,saltar_bytes,ptr_inputfile);

	//Y vamos leyendo bloques de 1024
	int escritos=0;

	do {
	        leidos=fread(buffer_lectura,1,1024,ptr_inputfile);
		if (leidos>0) {
			fwrite(buffer_lectura,1,leidos,ptr_outputfile);
			escritos +=leidos;
			debug_printf (VERBOSE_DEBUG,"Writing data %dKB ",escritos/1024);
			//put_progress_bar();
			//printf ("\r");
		}
	} while (leidos>0);

	fclose (ptr_inputfile);

        fclose(ptr_outputfile);


        return 0;
}


//Para el conversor de scr a txt
FILE *convert_scr_to_txt_ptr_destino;
//Soporte para FatFS
FIL convert_scr_to_txt_fil;        /* File object */
int convert_scr_to_txt_in_fatfs;

void convert_scr_to_txt_printf (z80_byte c)
{
    zvfs_fwrite(convert_scr_to_txt_in_fatfs,&c,1,convert_scr_to_txt_ptr_destino,&convert_scr_to_txt_fil);

}

int convert_scr_to_txt(char *origen, char *destino)
{


    z80_byte *buffer_lectura;
    buffer_lectura=malloc(6912);

    if (buffer_lectura==NULL) cpu_panic("Cannot allocate memory for file read");

    lee_archivo(origen,(char *)buffer_lectura,6912);


    if (zvfs_fopen_write(destino,&convert_scr_to_txt_in_fatfs,&convert_scr_to_txt_ptr_destino,&convert_scr_to_txt_fil)<0) {
        debug_printf (VERBOSE_ERR,"Can not open %s",destino);
        return 1;
    }


    screen_text_repinta_pantalla_spectrum_comun_addr(0,convert_scr_to_txt_printf,1,0,buffer_lectura);

    zvfs_fclose(convert_scr_to_txt_in_fatfs,convert_scr_to_txt_ptr_destino,&convert_scr_to_txt_fil);

    return 0;
}


int convert_scr_to_tap(char *origen, char *destino)
{

	int leidos;

        int tamanyo_origen=get_file_size(origen);

        if (tamanyo_origen<0) {
                debug_printf (VERBOSE_ERR,"Error getting size for %s",origen);
                return 1;
        }

        z80_byte *buffer_lectura;
        buffer_lectura=malloc(tamanyo_origen);

        if (buffer_lectura==NULL) cpu_panic("Cannot allocate memory for file read");





	//unsigned char buffer_lectura[1024];

        FILE *ptr_inputfile;
        ptr_inputfile=fopen(origen,"rb");

        if (ptr_inputfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error opening %s",origen);
                return 1;
        }


        leidos=fread(buffer_lectura,1,tamanyo_origen,ptr_inputfile);

        if (tamanyo_origen!=leidos) {
              debug_printf (VERBOSE_ERR,"Error reading %s",origen);
                return 1;
        }


        fclose (ptr_inputfile);



	FILE *ptr_outputfile;
	ptr_outputfile=fopen(destino,"wb");

        if (ptr_outputfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error opening %s",destino);
                return 1;
        }

        //Metemos bloque cabecera
        //Primero indicar longitud bloque que viene
        z80_byte buf_tap_cab[2];
        buf_tap_cab[0]=19;
        buf_tap_cab[1]=0;

        //Escribir esto
        fwrite(buf_tap_cab,1,2,ptr_outputfile);

        //Cabecera spectrum
        z80_byte bloque_cabecera[19]={
                0, //flag 0,
                3, //bloque bytes,
                'S','C','R','E','E','N',' ',' ',' ',' ', //nombre
                0x00,0x1b, //longitud 6912
                0x00,0x40, //inicio 16384
                0x00,0x80, //unused
                00  //checksum. meter despues
        };

        z80_byte crc=get_memory_checksum_spectrum(0,bloque_cabecera,18);

        bloque_cabecera[18]=crc;

        //Escribir esto
        fwrite(bloque_cabecera,1,19,ptr_outputfile);


	//Escribir bloque de datos propiamente

        //indicar longitud bloque que viene. 6912+2  (+2 del flag y crc)
        buf_tap_cab[0]=0x02;
        buf_tap_cab[1]=0x1b;

        //Escribir esto
        fwrite(buf_tap_cab,1,2,ptr_outputfile);

        //flag 255
        z80_byte byte_flag=255;
        //Escribir esto
        fwrite(&byte_flag,1,1,ptr_outputfile);

        //Los datos propiamente
        fwrite(buffer_lectura,1,6912,ptr_outputfile);

        //Y meter crc. Calculamos. Tenemos que byte inicial es el de flag
        crc=get_memory_checksum_spectrum(byte_flag,buffer_lectura,6912);

        //Escribir esto
        fwrite(&crc,1,1,ptr_outputfile);


        fclose(ptr_outputfile);

        free(buffer_lectura);


        return 0;
}


//Crea carpeta temporal y asigna nombre para archivo temporal rwa
void convert_to_rwa_common_tmp(char *origen, char *destino)
{
        char nombre_origen[NAME_MAX];
        util_get_file_no_directory(origen,nombre_origen);

        sprintf (destino,"%s/tmp_%s.rwa",get_tmpdir_base(),nombre_origen);
        debug_printf (VERBOSE_INFO,"Creating temporary file %s",destino);
}


//Crea carpeta temporal y asigna nombre para archivo temporal raw
void convert_to_raw_common_tmp(char *origen, char *destino)
{
        char nombre_origen[NAME_MAX];
        util_get_file_no_directory(origen,nombre_origen);

        sprintf (destino,"%s/tmp_%s.raw",get_tmpdir_base(),nombre_origen);
        debug_printf (VERBOSE_INFO,"Creating temporary file %s",destino);
}

//Convierte archivo tap a rwa en carpeta temporal, generando nombre de archivo destino
int convert_tap_to_rwa_tmpdir(char *origen, char *destino)
{
	convert_to_rwa_common_tmp(origen,destino);

	return convert_tap_to_rwa(origen,destino);
}

//Convierte archivo pzx a rwa en carpeta temporal, generando nombre de archivo destino
int convert_pzx_to_rwa_tmpdir(char *origen, char *destino)
{
	convert_to_rwa_common_tmp(origen,destino);

	return convert_pzx_to_rwa(origen,destino);
}

//Convierte archivo tzx a rwa en carpeta temporal, generando nombre de archivo destino
int convert_tzx_to_rwa_tmpdir(char *origen, char *destino)
{
	convert_to_rwa_common_tmp(origen,destino);

        return convert_tzx_to_rwa(origen,destino);

}

int convert_smp_to_rwa_tmpdir(char *origen, char *destino)
{
        convert_to_rwa_common_tmp(origen,destino);

        return convert_smp_to_rwa(origen,destino);

}

int convert_wav_to_rwa_tmpdir(char *origen, char *destino)
{
        convert_to_rwa_common_tmp(origen,destino);

        return convert_wav_to_rwa(origen,destino);

}

int convert_wav_to_raw_tmpdir(char *origen, char *destino)
{
        convert_to_raw_common_tmp(origen,destino);

        return convert_wav_to_raw(origen,destino);

}

int convert_o_to_rwa_tmpdir(char *origen, char *destino)
{
        convert_to_rwa_common_tmp(origen,destino);

        return convert_o_to_rwa(origen,destino);

}

int convert_p_to_rwa_tmpdir(char *origen, char *destino)
{
        convert_to_rwa_common_tmp(origen,destino);

        return convert_p_to_rwa(origen,destino);

}



//Retorna 0 si ok
//1 si error al abrir archivo
//Tiene en cuenta zonas de memoria
int save_binary_file(char *filename,int valor_leido_direccion,int valor_leido_longitud)
{


	menu_debug_set_memory_zone_attr();

        char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
	menu_get_current_memory_zone_name_number(zone_name);

        if (valor_leido_longitud==0) valor_leido_longitud=menu_debug_memory_zone_size;


	debug_printf(VERBOSE_INFO,"Saving %s file at %d address at zone %s with %d bytes",filename,valor_leido_direccion,zone_name,valor_leido_longitud);

	FILE *ptr_binaryfile_save;
	ptr_binaryfile_save=fopen(filename,"wb");
	if (!ptr_binaryfile_save) {

		debug_printf (VERBOSE_ERR,"Unable to open Binary file %s",filename);
		return 1;
	}

	else {

		int escritos=1;
		z80_byte byte_leido;
		while (valor_leido_longitud>0 && escritos>0) {

			byte_leido=menu_debug_get_mapped_byte(valor_leido_direccion);

			escritos=fwrite(&byte_leido,1,1,ptr_binaryfile_save);
			valor_leido_direccion++;
			valor_leido_longitud--;
		}


		fclose(ptr_binaryfile_save);
	}

	return 0;

}

//Retorna 0 si ok
//1 si archivo no encontrado
//2 si error leyendo
//Tiene en cuenta zonas de memoria
int load_binary_file(char *filename,int valor_leido_direccion,int valor_leido_longitud)
{
	int returncode=0;

	if (!si_existe_archivo(filename)) return 1;

	if (valor_leido_longitud==0) valor_leido_longitud=4194304; //4 MB max


	menu_debug_set_memory_zone_attr();


	char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
	menu_get_current_memory_zone_name_number(zone_name);

	debug_printf(VERBOSE_INFO,"Loading %s file at %d address at zone %s with maximum %d bytes",filename,valor_leido_direccion,zone_name,valor_leido_longitud);


	FILE *ptr_binaryfile_load;
	ptr_binaryfile_load=fopen(filename,"rb");
	if (!ptr_binaryfile_load) {

		debug_printf (VERBOSE_ERR,"Unable to open Binary file %s",filename);
		returncode=2;

	}

	else {

		int leidos=1;
		z80_byte byte_leido;
		while (valor_leido_longitud>0 && leidos>0) {
			leidos=fread(&byte_leido,1,1,ptr_binaryfile_load);
			if (leidos>0) {

					menu_debug_write_mapped_byte(valor_leido_direccion,byte_leido);

					valor_leido_direccion++;
					valor_leido_longitud--;
			}
		}


		fclose(ptr_binaryfile_load);

	}

    return returncode;

}






//extern char *mostrar_footer_game_name;

//Opciones para configuracion custom de archivo cinta/snap
void parse_customfile_options(void)
{

	//Si se ha leido algun parametro de --joystickkeybt o --joystickkeyev
	//Cuando se lee el primero, se inicializa la tabla a 0
	int leido_config_joystick_a_key=0;

	z80_bit added_some_osd_text_keyboard={0};

        while (!siguiente_parametro()) {

		debug_printf (VERBOSE_DEBUG,"Parsing setting %s",argv[puntero_parametro]);

                if (!strcmp(argv[puntero_parametro],"--realvideo")) {
                                enable_rainbow();
                }

		else if (!strcmp(argv[puntero_parametro],"--snoweffect")) {
                                snow_effect_enabled.v=1;
		}

		else if (!strcmp(argv[puntero_parametro],"--wrx")) {
			enable_wrx();
		}

		else if (!strcmp(argv[puntero_parametro],"--chroma81")) {
                        enable_chroma81();
		}

                else if (!strcmp(argv[puntero_parametro],"--vsync-minimum-length")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<100 || valor>999) {
                                        debug_printf (VERBOSE_ERR,"Invalid vsync length value");
                                        return;
                                }
                                minimo_duracion_vsync=valor;
                }

		else if (!strcmp(argv[puntero_parametro],"--no-horiz-stabilization")) {
                        video_zx8081_estabilizador_imagen.v=0;
		}

		else if (!strcmp(argv[puntero_parametro],"--programname")) {
			siguiente_parametro_argumento();
			mostrar_footer_game_name=argv[puntero_parametro];
		}

		//Para que aparezca --programsettingsinfo tiene que haber --programname
		else if (!strcmp(argv[puntero_parametro],"--programsettingsinfo")) {
                        siguiente_parametro_argumento();
                        mostrar_footer_second_message=argv[puntero_parametro];

                        //Parche para poder soportar la letra eñe en ZEsarUX 10.10
                        //Cambiar la # por el caracter 129 que es nuestro caracter interno
                        int jj;
                        int longitud=strlen(mostrar_footer_second_message);
                        for (jj=0;jj<longitud;jj++) {
                            if (mostrar_footer_second_message[jj]=='#') {
                                if (si_complete_video_driver()) mostrar_footer_second_message[jj]=0x81;
                                else mostrar_footer_second_message[jj]='n';
                            }
                        }

                }


		else if (!strcmp(argv[puntero_parametro],"--disableborder")) {
			debug_printf(VERBOSE_INFO,"End Screen");
	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);


			disable_border();
			screen_init_pantalla_and_others_and_realjoystick();
                        screen_restart_pantalla_restore_overlay(previous_function,menu_antes);
			debug_printf(VERBOSE_INFO,"Creating Screen");
		}

                else if (!strcmp(argv[puntero_parametro],"--enableborder")) {
                        debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);

                        enable_border();
			screen_init_pantalla_and_others_and_realjoystick();
                        screen_restart_pantalla_restore_overlay(previous_function,menu_antes);
                        debug_printf(VERBOSE_INFO,"Creating Screen");
                }

		else if (!strcmp(argv[puntero_parametro],"--enableinterlace")) {
			enable_interlace();
		}

		else if (!strcmp(argv[puntero_parametro],"--disableinterlace")) {
			disable_interlace();
		}

		else if (!strcmp(argv[puntero_parametro],"--enableulaplus")) {
			enable_ulaplus();
		}

		else if (!strcmp(argv[puntero_parametro],"--disableulaplus")) {
			disable_ulaplus();
		}

                else if (!strcmp(argv[puntero_parametro],"--enablespectra")) {
                        spectra_enable();
                }

                else if (!strcmp(argv[puntero_parametro],"--disablespectra")) {
                        spectra_disable();
                }

		else if (!strcmp(argv[puntero_parametro],"--enabletimexvideo")) {
                        enable_timex_video();
		}

		else if (!strcmp(argv[puntero_parametro],"--disabletimexvideo")) {
                        disable_timex_video();
		}

		else if (!strcmp(argv[puntero_parametro],"--enablezgx")) {
                        spritechip_enable();
		}

		else if (!strcmp(argv[puntero_parametro],"--redefinekey")) {
				z80_byte tecla_original, tecla_redefinida;
                                siguiente_parametro_argumento();
                                tecla_original=parse_string_to_number(argv[puntero_parametro]);

                                siguiente_parametro_argumento();
                                tecla_redefinida=parse_string_to_number(argv[puntero_parametro]);

                                if (util_add_redefinir_tecla(tecla_original,tecla_redefinida)) {
					//Error volver sin leer mas parametros
                                        return;
                                }
		}

		else if (!strcmp(argv[puntero_parametro],"--clearredefinekey")) {
			clear_lista_teclas_redefinidas();
		}



		else if (!strcmp(argv[puntero_parametro],"--joystickemulated")) {
			siguiente_parametro_argumento();
			if (realjoystick_set_type(argv[puntero_parametro])) {
			//Error. Volver sin leer mas parametros
				return;
			}

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
					//Error. Volver sin leer mas parametros
                                        return;
                                }


		}

		else if (!strcmp(argv[puntero_parametro],"--joystickkeybt")) {

				if (leido_config_joystick_a_key==0) {
					//Vaciar tabla
					realjoystick_clear_keys_array();
					//Puntero a 0
					joystickkey_definidas=0;
					//Decir que para las siguientes no se borra
					leido_config_joystick_a_key=1;
				}

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
					//Error. Volver sin leer mas parametros
                                        return;
                                }

		}

                else if (!strcmp(argv[puntero_parametro],"--joystickkeyev")) {

                                if (leido_config_joystick_a_key==0) {
                                        //Vaciar tabla
                                        realjoystick_clear_keys_array();
                                        //Puntero a 0
                                        joystickkey_definidas=0;
                                        //Decir que para las siguientes no se borra
                                        leido_config_joystick_a_key=1;
                                }

                                char *text_event;
                                char *text_key;

                                //obtener boton
                                siguiente_parametro_argumento();
                                text_event=argv[puntero_parametro];

                                //obtener tecla
                                siguiente_parametro_argumento();
                                text_key=argv[puntero_parametro];

                                //Y definir el evento
                                if (realjoystick_set_event_key(text_event,text_key)) {
					//Error. Volver sin leer mas parametros
                                        return;
                                }

                }

                else if (!strcmp(argv[puntero_parametro],"--cleareventlist")) {
			realjoystick_clear_events_array();
		}


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
                                        debug_printf (VERBOSE_ERR,"Invalid text-keyboard-length value\n");
                                        return;
                                }
						adventure_keyboard_key_length=valor;
				}

				else if (!strcmp(argv[puntero_parametro],"--text-keyboard-finalspc")) {
						adventure_keyboard_send_final_spc=1;

				}

		 else if (!strcmp(argv[puntero_parametro],"--machine")) {
                                char *machine_name;
                                siguiente_parametro_argumento();
                                machine_name=argv[puntero_parametro];

                                if (set_machine_type_by_name(machine_name)) {
					                               //Error. Volver sin leer mas parametros
                                        return;
                                }

                                set_machine(NULL);
                                cold_start_cpu_registers();
                                reset_cpu();

		}

		else if (!strcmp(argv[puntero_parametro],"--zx8081vsyncsound")) {
			zx8081_vsync_sound.v=1;
		}

		else if (!strcmp(argv[puntero_parametro],"--zx8081mem")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<1 || valor>16) {
                                        debug_printf (VERBOSE_ERR,"Invalid RAM value");
                                        return;
                                }
                                set_zx8081_ramtop(valor);
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

		else if (!strcmp(argv[puntero_parametro],"--gigascreen")) {
                                enable_gigascreen();
		}


		else if (!strcmp(argv[puntero_parametro],"--enablezx8081lnctradjust")) {
			video_zx8081_lnctr_adjust.v=1;
		}

		else if (!strcmp(argv[puntero_parametro],"--disablezx8081lnctradjust")) {
			video_zx8081_lnctr_adjust.v=0;
		}

		else if (!strcmp(argv[puntero_parametro],"--frameskip")) {
                                siguiente_parametro_argumento();
                                int valor=atoi(argv[puntero_parametro]);
                                if (valor<0 || valor>50) {
                                        debug_printf (VERBOSE_ERR,"Invalid frameskip value");
                                        return;
                                }
                                frameskip=valor;
                }

		else if (!strcmp(argv[puntero_parametro],"--aychip")) {
                                ay_chip_present.v=1;
		}



		//
		//Opcion no reconocida. Error
		//
                else {
                        debug_printf (VERBOSE_ERR,"Setting %s not allowed on custom config file",argv[puntero_parametro]);
                        return;
                }
        }


}

void zmenufiles_help(void)
{
	printf ("zmenu files help:\n"
		"\n"
		"zmenu files are configuration files to open a menu inside ZEsarUX.\n"
		"The idea for that files is to have a launcher for different programs, they are opened by the Smartloard or just writing them as a ZEsarUX parameter.\n"
		"zmenu files are text files and have extension .zmenu.\n"
		"\n"
		"The following are the allowed parameters on a zmenu file:\n"
		"\n"

        "--set-launcher text                                 Set the name of the launcher window\n"
        "--add-launcher-entry-smartload name path_to_file    Defines a program to by run like a smartload entry. Path is searched on the current directory, and also on the common installed path\n"
        "--add-launcher-entry-ql-mdv    name path_to_folder  Defines a directory to by mounted on QL as a mdv1 path. Path is searched on the current directory, and also on the common installed path\n"

        "\n"
	);
}

void customconfig_help(void)
{
	printf ("Custom config help:\n"
		"\n"
		"Custom config files are configuration files for every program or game you want.\n"
		"When you load a tape/snapshot, the emulator will search for a file, identical name of the file you load,\n"
		"but adding extension .config, and it will apply every parameter contained in the file.\n"
		"\n"
		"So, if you load for example: game.tap, it will be searched a file game.tap.config\n"
		"This config file is a text file you can edit; the format is like the " DEFAULT_ZESARUX_CONFIG_FILE " main configuration file,\n"
		"but not every command line parameter allowed on main configuration file is allowed on a custom config file.\n"
		"There are some .config files included in the ZEsarUX extras package, you can view them to have some examples."
		"\n"
		"The following are the allowed parameters on a custom config file, I don't describe the parameters that are the same as on command line:\n"
		"\n"

	"--machine id \n"
	"--programname text          Set the name of the tape/snapshot, shown on footer\n"
	"--programsettingsinfo text  Set the text for settings information, shown on footer\n"
	"--frameskip n\n"
	"--disableborder\n"
	"--enableborder              Enable border\n"
	"--aychip                    Enable AY-Chip\n"
	"--zx8081mem n\n"
	"--zx8081ram8K2000\n"
	"--zx8081ram16K8000\n"
	"--zx8081ram16KC000\n"
	"--zx8081vsyncsound\n"
	"--realvideo\n"
	"--snoweffect\n"
	"--enableinterlace           Enable interlace video mode\n"
	"--disableinterlace          Disable interlace video mode\n"
	"--enableulaplus             Enable ULAplus video modes\n"
	"--disableulaplus            Disable ULAplus video modes\n"
	"--enablespectra             Enable Spectra video modes\n"
	"--disablespectra            Disable Spectra video modes\n"
	"--enabletimexvideo          Enable Timex video modes\n"
	"--disabletimexvideo         Disable Timex video modes\n"
	"--enablezgx\n"
	"--wrx\n"
	"--chroma81\n"
	"--vsync-minimum-length n \n"
	"--no-horiz-stabilization    Disable Horizontal Stabilization\n"
	"--gigascreen                Enable GigaScreen emulation\n"
	"--enablezx8081lnctradjust   Enable LNCTR adjust on ZX80/81\n"
	"--disablezx8081lnctradjust  Disable LNCTR adjust on ZX80/81\n"
	"--redefinekey src dest\n"
	"--clearredefinekey          Clear redefine key table\n"
	"--joystickemulated type\n"
	"--joystickevent but evt\n"
	"--joystickkeybt but key\n"
	"--joystickkeyev evt key\n"
	"--text-keyboard-add text\n"
        "--text-keyboard-length n\n"
        "--text-keyboard-finalspc\n"
	"--cleareventlist\n"

	"\n"
	);
}

//Nombres cortos de maquinas y sus id y su icono
struct s_machines_short_names_id {
        char machine_name[32];
        int machine_id;
        char **bitmap;
};

//Finaliza con machine_id -1
struct s_machines_short_names_id machines_short_names_id[]={
   {"16k",0,bitmap_button_ext_desktop_my_machine_gomas},
   {"48k",1,bitmap_button_ext_desktop_my_machine_gomas},
   {"48kp",MACHINE_ID_SPECTRUM_48_PLUS_ENG,bitmap_button_ext_desktop_my_machine_spectrum_48_spa},
   {"48ks",20,bitmap_button_ext_desktop_my_machine_spectrum_48_spa},
   {"Inves",2,bitmap_button_ext_desktop_my_machine_inves},
   {"TK90X",MACHINE_ID_MICRODIGITAL_TK90X,bitmap_button_ext_desktop_my_machine_tk90x},
   {"TK90XS",MACHINE_ID_MICRODIGITAL_TK90X_SPA,bitmap_button_ext_desktop_my_machine_tk90x},
   {"TK95",MACHINE_ID_MICRODIGITAL_TK95,bitmap_button_ext_desktop_my_machine_tk95},
   {"TK95S",MACHINE_ID_MICRODIGITAL_TK95_SPA,bitmap_button_ext_desktop_my_machine_tk95},
   {"128k",6,bitmap_button_ext_desktop_my_machine_spectrum_128_eng},
   {"128ks",7,bitmap_button_ext_desktop_my_machine_spectrum_128_spa},
   {"P2",8,bitmap_button_ext_desktop_my_machine_spectrum_p2},
   {"P2F",9,bitmap_button_ext_desktop_my_machine_spectrum_p2},
   {"P2S",10,bitmap_button_ext_desktop_my_machine_spectrum_p2},

   {"P2A40",11,bitmap_button_ext_desktop_my_machine_spectrum_p2a},
   {"P2A41",12,bitmap_button_ext_desktop_my_machine_spectrum_p2a},
   {"P2AS",13,bitmap_button_ext_desktop_my_machine_spectrum_p2a},
   {"ZXUNO",14,bitmap_button_ext_desktop_my_machine_zxuno},
   {"Chloe140",15,bitmap_button_ext_desktop_my_machine_generic},
   {"Chloe280",16,bitmap_button_ext_desktop_my_machine_generic},
   {"TC2048",MACHINE_ID_TIMEX_TC2048,bitmap_button_ext_desktop_my_machine_timex_tc2048},
   {"TC2068",MACHINE_ID_TIMEX_TC2068,bitmap_button_ext_desktop_my_machine_timex_ts2068},
   {"TS2068",MACHINE_ID_TIMEX_TS2068,bitmap_button_ext_desktop_my_machine_timex_ts2068},
   {"Prism",18,bitmap_button_ext_desktop_my_machine_generic},
   {"TBBlue",19,bitmap_button_ext_desktop_my_machine_spectrum_next},

   {"Pentagon",21,bitmap_button_ext_desktop_my_machine_pentagon},
   {"Chrome",MACHINE_ID_CHROME,bitmap_button_ext_desktop_my_machine_generic},
   {"BaseConf",MACHINE_ID_BASECONF,bitmap_button_ext_desktop_my_machine_baseconf},
   {"TSConf",MACHINE_ID_TSCONF,bitmap_button_ext_desktop_my_machine_tsconf},
   {"P340",MACHINE_ID_SPECTRUM_P3_40,bitmap_button_ext_desktop_my_machine_spectrum_p3},
   {"P341",MACHINE_ID_SPECTRUM_P3_41,bitmap_button_ext_desktop_my_machine_spectrum_p3},
   {"P3S",MACHINE_ID_SPECTRUM_P3_SPA,bitmap_button_ext_desktop_my_machine_spectrum_p3},
   {"ZX80",MACHINE_ID_ZX80,bitmap_button_ext_desktop_my_machine_zx80},
   {"ZX81",121,bitmap_button_ext_desktop_my_machine_zx81},
   {"ACE",122,bitmap_button_ext_desktop_my_machine_ace},
   {"TS1000",MACHINE_ID_TIMEX_TS1000,bitmap_button_ext_desktop_my_machine_ts1000},
   {"TS1500",MACHINE_ID_TIMEX_TS1500,bitmap_button_ext_desktop_my_machine_ts1500},


    {"CZ1000",MACHINE_ID_CZ_1000,bitmap_button_ext_desktop_my_machine_cz1000},
    {"CZ1500",MACHINE_ID_CZ_1500,bitmap_button_ext_desktop_my_machine_cz1500},
    {"CZ1000p",MACHINE_ID_CZ_1000_PLUS,bitmap_button_ext_desktop_my_machine_cz1000_plus},
    {"CZ1500p",MACHINE_ID_CZ_1500_PLUS,bitmap_button_ext_desktop_my_machine_cz1500_plus},
    {"CZ2000",MACHINE_ID_CZ_2000,bitmap_button_ext_desktop_my_machine_cz2000},
    {"CZSPEC",MACHINE_ID_CZ_SPECTRUM,bitmap_button_ext_desktop_my_machine_cz_spectrum},
    {"CZSPECp",MACHINE_ID_CZ_SPECTRUM_PLUS,bitmap_button_ext_desktop_my_machine_cz_spectrum_plus},



   {"TK80",MACHINE_ID_MICRODIGITAL_TK80,bitmap_button_ext_desktop_my_machine_tk80},
   {"TK82",MACHINE_ID_MICRODIGITAL_TK82,bitmap_button_ext_desktop_my_machine_tk82},
   {"TK82C",MACHINE_ID_MICRODIGITAL_TK82C,bitmap_button_ext_desktop_my_machine_tk82c},
   {"TK83",MACHINE_ID_MICRODIGITAL_TK83,bitmap_button_ext_desktop_my_machine_tk83},
   {"TK85",MACHINE_ID_MICRODIGITAL_TK85,bitmap_button_ext_desktop_my_machine_tk85},

   {"Z88",130,bitmap_button_ext_desktop_my_machine_z88},
   {"CPC464",MACHINE_ID_CPC_464,bitmap_button_ext_desktop_my_machine_cpc_464},
   {"CPC4128",MACHINE_ID_CPC_4128,bitmap_button_ext_desktop_my_machine_cpc_464},
   {"CPC664",MACHINE_ID_CPC_664,bitmap_button_ext_desktop_my_machine_cpc_664},
   {"CPC6128",MACHINE_ID_CPC_6128,bitmap_button_ext_desktop_my_machine_cpc_6128},
   {"PCW8256",MACHINE_ID_PCW_8256,bitmap_button_ext_desktop_my_machine_pcw_8256},
   {"PCW8512",MACHINE_ID_PCW_8512,bitmap_button_ext_desktop_my_machine_pcw_8256},
   {"SAM",150,bitmap_button_ext_desktop_my_machine_sam},
   {"QL",160,bitmap_button_ext_desktop_my_machine_ql},
   {"MK14",MACHINE_ID_MK14_STANDARD,bitmap_button_ext_desktop_my_machine_mk14},

   {"MSX1",MACHINE_ID_MSX1,bitmap_button_ext_desktop_my_machine_msx},
   {"COLECO",MACHINE_ID_COLECO,bitmap_button_ext_desktop_my_machine_coleco},
   {"SG1000",MACHINE_ID_SG1000,bitmap_button_ext_desktop_my_machine_sg1000},
   {"SMS",MACHINE_ID_SMS,bitmap_button_ext_desktop_my_machine_sms},
   {"SVI318",MACHINE_ID_SVI_318,bitmap_button_ext_desktop_my_machine_svi318},
   {"SVI328",MACHINE_ID_SVI_328,bitmap_button_ext_desktop_my_machine_svi328},

   //Fin
   {"",-1,bitmap_button_ext_desktop_my_machine_generic}
};


//Devuelve -1 si desconocida
int get_machine_id_by_name(char *machine_name)
{

        int i=0;

        while (machines_short_names_id[i].machine_id>=0) {

                if (!strcasecmp(machines_short_names_id[i].machine_name,machine_name)) {
                        return machines_short_names_id[i].machine_id;
                }

                i++;
        }

        //no encontrado
        debug_printf (VERBOSE_ERR,"Unknown machine %s",machine_name);
        return -1;

}

char **get_machine_icon_by_name(char *machine_name)
{

        int i=0;

        while (machines_short_names_id[i].machine_id>=0) {

                if (!strcasecmp(machines_short_names_id[i].machine_name,machine_name)) {
                        return machines_short_names_id[i].bitmap;
                }

                i++;
        }

        //no encontrado
        //debug_printf (VERBOSE_ERR,"Unknown machine %s",machine_name);
        return bitmap_button_ext_desktop_my_machine_generic;

}


//Devuelve listado de ids de maquinas separado por espacios
void get_machine_list_whitespace(void)
{

        int i=0;

        while (machines_short_names_id[i].machine_id>=0) {
                printf("%s ",machines_short_names_id[i].machine_name);

                i++;
        }


}

//Devuelve 0 si ok
int set_machine_type_by_name(char *machine_name)
{

  int maquina=get_machine_id_by_name(machine_name);
  if (maquina==-1) return 1;

  current_machine_type=maquina;


	return 0;
}

//Esta es la funcion inversa de la anterior. Devuelve "" si no se sabe numero de maquina
void get_machine_config_name_by_number(char *machine_name,int machine_number)
{

        int i=0;

        while (machines_short_names_id[i].machine_id>=0) {

                if (machine_number==machines_short_names_id[i].machine_id) {
                        strcpy(machine_name,machines_short_names_id[i].machine_name);
                        return;
                }

                i++;
        }

        //no encontrado
        machine_name[0]=0;
}



//Alternativa a scandir para sistemas Mingw, que no implementan dicha funcion
int scandir_mingw(const char *dir, struct dirent ***namelist,
              int (*filter)(const struct dirent *),
              int (*compar)(const struct dirent **, const struct dirent **))
{

	#define MAX_ARCHIVOS_SCANDIR_MINGW 20000
        int archivos=0;

        //Puntero a cada archivo leido
        struct dirent *memoria_archivos;

        //Array de punteros.
        struct dirent **memoria_punteros;

        //Asignamos memoria
        memoria_punteros=malloc(sizeof(struct dirent *)*MAX_ARCHIVOS_SCANDIR_MINGW);


        if (memoria_punteros==NULL) {
                cpu_panic("Error allocating memory when reading directory");
        }

        *namelist=memoria_punteros;

        //int indice_puntero;


       struct dirent *dp;
       DIR *dfd;

       if ((dfd = opendir(dir)) == NULL) {
           debug_printf(VERBOSE_ERR,"Can't open directory %s", dir);
           return -1;
       }

       while ((dp = readdir(dfd)) != NULL) {

                debug_printf(VERBOSE_DEBUG,"scandir_mingw: file: %s",dp->d_name);

		if (filter(dp)) {

		        //Asignar memoria para ese fichero
		        memoria_archivos=malloc(sizeof(struct dirent));

		      if (memoria_archivos==NULL) {
                		cpu_panic("Error allocating memory when reading directory");
		        }

		        //Meter puntero
		        memoria_punteros[archivos]=memoria_archivos;

		        //Meter datos
		        memcpy(memoria_archivos,dp,sizeof( struct dirent ));
		        archivos++;

		        if (archivos>=MAX_ARCHIVOS_SCANDIR_MINGW) {
                		debug_printf(VERBOSE_ERR,"Error. Maximum files in directory reached: %d",MAX_ARCHIVOS_SCANDIR_MINGW);
		                return archivos;
		        }

		}


       }
       closedir(dfd);

	//lanzar qsort
	int (*funcion_compar)(const void *, const void *);

	funcion_compar=( int (*)(const void *, const void *)  )compar;

	qsort(memoria_punteros,archivos,sizeof( struct dirent *), funcion_compar);

        return archivos;
}


//Devuelve la letra en mayusculas
char letra_mayuscula(char c)
{
	if (c>='a' && c<='z') c=c-('a'-'A');
	return c;
}

//Devuelve la letra en minusculas
char letra_minuscula(char c)
{
        if (c>='A' && c<='Z') c=c+('a'-'A');
        return c;
}

//Devuelve la letra en minusculas, pero considerando valores int
int int_minuscula(int c)
{
        if (c>='A' && c<='Z') c=c+('a'-'A');
        return c;
}

//Convierte una string en minusculas
void string_a_minusculas(char *origen, char *destino)
{
	char letra;

	for (;*origen;origen++,destino++) {
		letra=*origen;
		letra=letra_minuscula(letra);
		*destino=letra;
	}

	*destino=0;
}

//Convierte una string en mayusculas
void string_a_mayusculas(char *origen, char *destino)
{
	char letra;

	for (;*origen;origen++,destino++) {
		letra=*origen;
		letra=letra_mayuscula(letra);
		*destino=letra;
	}

	*destino=0;
}

//Dice si ruta es absoluta. 1 si es absoluta. 0 si no
int si_ruta_absoluta(char *ruta)
{

#ifdef MINGW
	//En Windows
	//Si empieza por '\' es absoluta
        if (ruta[0]=='\\') return 1;

	//Si empieza por letra unidad: es absoluta
	if (strlen(ruta)>2) {
		//Solo mirar los :
		if (ruta[1]==':') return 1;
	}

        return 0;

#else
	//En Unix
	//Si empieza por '/' es absoluta
	if (ruta[0]=='/') return 1;
	else return 0;
#endif

}

//Retorna tipo de archivo
//0: desconocido
//1: archivo normal (o symbolic link)
//2: directorio


int get_file_type_from_stat(struct stat *f)
{
  if (f->st_mode & S_IFDIR) return 2;
  else return 1;
}


//0 desconocido o inexistente
//1 normal
//2 directorio
int get_file_type_from_name(char *nombre)
{
  struct stat buf_stat;

          if (stat(nombre, &buf_stat)!=0) {
                  debug_printf(VERBOSE_INFO,"Unable to get status of file %s",nombre);
return 0;
          }

          else {
//printf ("file size: %ld\n",buf_stat.st_size);
return get_file_type_from_stat(&buf_stat);
          }
}

int file_is_directory(char *nombre)
{
    if (get_file_type_from_name(nombre)==2) return 1;
    else return 0;
}

//Retorna fecha de un archivo en valores de punteros
//Devuelve 1 si error
//anyo tal cual: 2017, etc
int get_file_date_from_stat(struct stat *buf_stat,int *hora,int *minuto,int *segundo,int *dia,int *mes,int *anyo)
{



          struct tm *foo;

#if defined(__APPLE__)
          struct timespec *d;
          d=&buf_stat->st_mtimespec;
          //foo = gmtime((const time_t *)d);
          foo = localtime((const time_t *)d);
#else
          time_t *d;
          d=&buf_stat->st_mtime;
          //foo = gmtime((const time_t *)d);
          foo = localtime((const time_t *)d);
#endif

//printf("Year: %d\n", foo->tm_year);
//printf("Month: %d\n", foo->tm_mon);
//printf("Day: %d\n", foo->tm_mday);
//printf("Hour: %d\n", foo->tm_hour);
//printf("Minute: %d\n", foo->tm_min);
//printf("Second: %d\n", foo->tm_sec);

*hora=foo->tm_hour;
*minuto=foo->tm_min;
*segundo=foo->tm_sec;

*dia=foo->tm_mday;
*mes=foo->tm_mon+1;
*anyo=foo->tm_year+1900;

          return 0;


}

//Retorna fecha de un archivo en valores de punteros
//Devuelve 1 si error
//anyo tal cual: 2017, etc
int get_file_date_from_name(char *nombre,int *hora,int *minuto,int *segundo,int *dia,int *mes,int *anyo)
{

    if (util_path_is_mmc_fatfs(nombre)) {
        //printf("get_file_date_from_name for %s using FatFS\n",nombre);
        FRESULT fr;
        FILINFO fno;


        //printf("Test for 'file.txt'...\n");

        fr = f_stat(nombre, &fno);
        if (fr==FR_OK) {
            /*
fdate
The date when the file was modified or the directory was created.
bit15:9
Year origin from 1980 (0..127)

bit8:5
Month (1..12)

bit4:0
Day (1..31)

ftime
The time when the file was modified or the directory was created.

bit15:11
Hour (0..23)

bit10:5
Minute (0..59)

bit4:0
Second / 2 (0..29)

            */

           *anyo=1980+((fno.fdate >> 9) &127);

           *mes=((fno.fdate >> 5) & 15);

           *dia=((fno.fdate ) & 31);

           *hora=((fno.ftime >> 11) &31);

           *minuto=((fno.ftime >> 5) &63);

           *segundo=((fno.ftime ) &31);

           return 0;
        }

        //desconocido
        else return 1;
    }
    else {

    struct stat buf_stat;

    if (stat(nombre, &buf_stat)!=0) {
        debug_printf(VERBOSE_INFO,"Unable to get status of file %s",nombre);
        return 1;
    }


    get_file_date_from_stat(&buf_stat,hora,minuto,segundo,dia,mes,anyo);


    return 0;

    }
}




//Retorna tipo de archivo segun valor d_type
//Funcion nueva que usa st_mode en vez de d_type. d_type no valia para Windows ni para Haiku
//de hecho, hay que EVITAR usar esa propiedad pues no está en todos los sistemas operativos,
//haiku por ejemplo no la tiene

/*
The only fields in the dirent structure that are mandated by
    POSIX.1 are d_name and d_ino.  The other fields are
    unstandardized, and not present on all systems; see NOTES below
    for some further details.
*/
//0: desconocido
//1: archivo normal (o symbolic link)
//2: directorio
//Entrada: nombre archivo. requisito es que el archivo se encuentre en directorio actual


int get_file_type(char *nombre)
{


    //Si es archivo de la mmc
    //if (util_path_is_prefix_mmc_fatfs(nombre)) {
    if (util_path_is_mmc_fatfs(nombre)) {
        //printf("f_stat for %s using FatFS\n",nombre);
        FRESULT fr;
        FILINFO fno;


        //printf("Test for 'file.txt'...\n");

        fr = f_stat(nombre, &fno);
        if (fr==FR_OK) {
            if (fno.fattrib & AM_DIR) {
                //if (!strcmp(nombre,"..")) printf(".. es directorio\n");
                return 2;
            }
            else {
                return 1;
            }
        }

        //desconocido
        else return 0;
    }

    struct stat buf_stat;

    if (stat(nombre, &buf_stat)==0) {
		debug_printf (VERBOSE_DEBUG,"Name: %s st_mode: %d constants: S_IFDIR: %d",nombre,buf_stat.st_mode,S_IFDIR);

		if (buf_stat.st_mode & S_IFDIR) return 2;
		else return 1;
	}


	//desconocido
    return 0;



}


void convert_relative_to_absolute(char *relative_path,char *final_path)
{
	//Directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//Cambiamos a lo que dice el path relativo
	zvfs_chdir(relative_path);

	//Y en que directorio acabamos?
	getcwd(final_path,PATH_MAX);

	//Volver a directorio inicial
	zvfs_chdir(directorio_actual);

	//printf ("Convert relative to absolute path: relative: %s absolute: %s\n",relative_path,final_path);

	debug_printf (VERBOSE_DEBUG,"Convert relative to absolute path: relative: %s absolute: %s",relative_path,final_path);

}

int util_read_file_lines(char *memoria,char **lineas,int max_lineas)
{

	int linea=0;
	//char letra;

	while (linea<max_lineas && *memoria!=0) {
		lineas[linea++]=memoria;

		while (*memoria!='\n' && *memoria!='\r' && *memoria!=0) {
			//printf ("1 %d %c ",linea,*memoria);
			memoria++;
		}

		//meter un 0
		if (*memoria!=0) {
			*memoria=0;
			memoria++;
		}

		//saltar saltos de lineas sobrantes
		while ( (*memoria=='\n' || *memoria=='\r') && *memoria!=0) {
			//printf ("2 %d %c ",linea,*memoria);
			memoria++;
		}

		//printf (" memoria %p\n",memoria);
	}

	return linea;

}

int util_read_line_fields(char *memoria,char **campos)
{
	int campo_actual=0;
	while (*memoria!=0) {
		campos[campo_actual++]=memoria;

		while (*memoria!=' ' && *memoria!=0) {
			memoria++;
		}

                //meter un 0
                if (*memoria!=0) {
                        *memoria=0;
                        memoria++;
                }

		//saltar espacios adicionales
		while ( *memoria==' ' && *memoria!=0) {
			memoria++;
		}
	}

	return campo_actual;
}


int util_parse_pok_mem(char *memoria,struct s_pokfile **tabla_pokes)
{
/*
'N' means: this is the Next trainer,
'Y' means: this is the last line of the file (the rest of the file is ignored).
After the 'N' follows the name/description of this specific trainer. This string
may be up to 30 characters. There is no space between the 'N' and the string.
Emulator authors can use these strings to set up the selection entries.

The following lines, up to the next 'N' or 'Z' hold the POKEs to be applied for
this specific trainer. Again, the first character determines the content.
'M' means: this is not the last POKE (More)
'Z' means: this is the last POKE
The rest of a POKE line is built from

  bbb aaaaa vvv ooo

All values are decimal, separation is done by one or more spaces.

The field 'bbb' (128K memory bank) is built from
  bit 0-2 : bank value
        3 : ignore bank (1=yes, always set for 48K games)



Ejemplo:
NImmunity
M  8 44278  58   0
Z  8 44285  58   0
Y
*/


/*
Estructura:

int indice_accion: numero de orden de la accion poke. 0 para la primera, 1 para la segunda, etc
Un poke de inmunidad que sean dos pokes, tendran este numero igual (y si es el primero, sera 0)
char texto[40]: lo que hace ese poke
z80_byte banco;
z80_int direccion;
z80_byte valor;
z80_byte valor_orig;

*/

	//int indice_accion=0;

	//char *lineas[MAX_LINEAS_POK_FILE];

        char **lineas;

        lineas=malloc(sizeof(char *) * MAX_LINEAS_POK_FILE);

        if (lineas==NULL) cpu_panic("Can not allocate memory for pok file reading");

	char ultimo_nombre_poke[MAX_LENGTH_LINE_POKE_FILE+1]="";

	int lineas_leidas=util_read_file_lines(memoria,lineas,MAX_LINEAS_POK_FILE);

	int i,j;

	int total_pokes=0;

	int final=0;

	int destino=0;

	for (i=0;i<lineas_leidas && final==0;i++) {
		debug_printf (VERBOSE_DEBUG,"line: %d text: %s",i,lineas[i]);
		char *campos[10];

		//Si linea pasa de 1024, limitar
		if (strlen(lineas[i]) >=1024 ) {
			debug_printf (VERBOSE_DEBUG,"Line %d too large. Truncating",i);
			lineas[i][1023]=0;
		}

		//copiar linea actual antes de trocear para recuperar luego nombre de poke en N si conviene

		char ultima_linea[1024];
		sprintf (ultima_linea,"%s",lineas[i]);
		//si linea execede maximo, truncar
		if (strlen(ultima_linea) > MAX_LENGTH_LINE_POKE_FILE) ultima_linea[MAX_LENGTH_LINE_POKE_FILE]=0;

		int total_campos=util_read_line_fields(lineas[i],campos);
		for (j=0;j<total_campos;j++) {
			//printf ("campo %d : [%s] ",j,campos[j]);
			//Si campo excede maximo, truncar
			if ( strlen(campos[j]) > MAX_LENGTH_LINE_POKE_FILE) campos[j][MAX_LENGTH_LINE_POKE_FILE]=0;
		}

		//Casos
		switch (campos[0][0]) {
			case 'N':
				//Texto de accion puede ser de dos palabras
				//sprintf (ultimo_nombre_poke,"%s %s",&campos[0][1], (total_campos>1 ? campos[1] : "")   );
				sprintf (ultimo_nombre_poke,"%s",&ultima_linea[1]);
			break;

			case 'M':
			case 'Z':
				//printf ("i: %d puntero %p\n",i,&tabla_pokes[i]->indice_accion);
				tabla_pokes[destino]->indice_accion=total_pokes;
				sprintf (tabla_pokes[destino]->texto,"%s",ultimo_nombre_poke);
				if (total_campos>=4) {
					tabla_pokes[destino]->banco=atoi(campos[1]);
					tabla_pokes[destino]->direccion=atoi(campos[2]);
					tabla_pokes[destino]->valor=atoi(campos[3]);
				}

				if (total_campos>=5) {
					tabla_pokes[destino]->valor_orig=atoi(campos[4]);
				}

				//Si era final de poke de accion, incrementar
				if (campos[0][0]=='Z') total_pokes++;

				destino++;

			break;


			case 'Y':
				final=1;
			break;

			default:
				//printf ("error format\n");
				return -1;
			break;
		}

		if (!final) {

		}


		//printf ("\n");
	}

        free(lineas);

	//return lineas_leidas;
	return destino;

}


//Retorna numero elementos del array
int util_parse_pok_file(char *file,struct s_pokfile **tabla_pokes)
{
	char *mem;

        int max_size=(MAX_LINEAS_POK_FILE*MAX_LENGTH_LINE_POKE_FILE)-1; //1 para el 0 del final


        if (get_file_size(file) > max_size) {
                debug_printf (VERBOSE_ERR,"File too large");
                return 0;
        }

	mem=malloc(max_size);


                FILE *ptr_pok;
                ptr_pok=fopen(file,"rb");

                if (!ptr_pok) {
                        debug_printf (VERBOSE_ERR,"Unable to open file %s",file);
                        return 0;
                }


                int leidos=fread(mem,1,max_size,ptr_pok);

		//Fin de texto
		mem[leidos]=0;

		fclose(ptr_pok);


	int total=util_parse_pok_mem(mem,tabla_pokes);



	free(mem);

	return total;


}


//Usado en pok file
int util_poke(z80_byte banco,z80_int direccion,z80_byte valor)
{
	//Si estamos en maquina 48k
	if (MACHINE_IS_SPECTRUM_16_48) {
		if (banco<8) {
			debug_printf (VERBOSE_ERR,"This poke is for a 128k machine and we are not in 128k machine (poke bank: %d)",banco);
			return -1;
		}

		debug_printf (VERBOSE_DEBUG,"util_spectrum_poke. pokeing address %d with value %d",direccion,valor);
		poke_byte_no_time(direccion,valor);
	}


	//si maquina 128k
	else if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		//ram_mem_table
		//Si banco >7, hacer poke normal
		if (banco>7) {
			debug_printf (VERBOSE_DEBUG,"util_spectrum_poke. pokeing address %d with value %d",direccion,valor);
			poke_byte_no_time(direccion,valor);
		}
		else {
			//Poke con banco de memoria
			direccion = direccion & 16383;
			z80_byte *puntero;
			puntero=ram_mem_table[banco]+direccion;
			debug_printf (VERBOSE_DEBUG,"util_spectrum_poke. pokeing bank %d address %d with value %d",banco,direccion,valor);
			*puntero=valor;
		}
	}

	else if (MACHINE_IS_ZXUNO)  {
		if (ZXUNO_BOOTM_ENABLED) {
			debug_printf (VERBOSE_ERR,"Can not poke in ZX-Uno mode with BOOTM enabled");
			return -1;
		}

                if (banco>7) {
                        debug_printf (VERBOSE_DEBUG,"util_spectrum_poke. pokeing address %d with value %d",direccion,valor);
                        poke_byte_no_time(direccion,valor);
                }
                else {
                        //Poke con banco de memoria
                        direccion = direccion & 16383;
                        z80_byte *puntero;
                        puntero=zxuno_sram_mem_table_new[banco]+direccion;
			debug_printf (VERBOSE_DEBUG,"util_spectrum_poke. pokeing bank %d address %d with value %d",banco,direccion,valor);
                        *puntero=valor;
                }
	}

	//resto de maquinas
	else if (MACHINE_IS_ZX8081 || MACHINE_IS_Z88) {
		debug_printf (VERBOSE_DEBUG,"util_spectrum_poke. pokeing address %d with value %d",direccion,valor);
		poke_byte_no_time(direccion,valor);
	}

	return 0;


}



//Busca archivo "archivo" en directorio, sin distinguir mayusculas/minusculas
//Si encuentra, lo mete en nombreencontrado, y devuelve no 0
//Si no encuentra, devuelve 0
int util_busca_archivo_nocase(char *archivo,char *directorio,char *nombreencontrado)
{

	debug_printf (VERBOSE_DEBUG,"Searching file %s in directory %s",archivo,directorio);

	//Saltar todas las / iniciales del archivo
	while ( (*archivo=='/' || *archivo=='\\') && (*archivo!=0) ) {
		archivo++;
	}


	struct dirent *busca_archivo_dirent;
	DIR *busca_archivo_dir=NULL;



	busca_archivo_dir = opendir(directorio);


	if (busca_archivo_dir==NULL) {
		return 0;
        }

        do {

                busca_archivo_dirent = readdir(busca_archivo_dir);

                if (busca_archivo_dirent == NULL) {
                        closedir(busca_archivo_dir);
			return 0;
                }

		if (!strcasecmp(archivo,busca_archivo_dirent->d_name)) {
			//Encontrado
			sprintf (nombreencontrado,"%s",busca_archivo_dirent->d_name);
			debug_printf (VERBOSE_DEBUG,"Found file %s",nombreencontrado);
			return 1;
		}


        } while(1);

	//Aqui no llega nunca
	return 0;


}


//Funciones Spool Turbo para modelos Spectrum
void peek_byte_spoolturbo_check_key(z80_int dir)
{
	//si dir=23560, enviar tecla de spool file
	z80_int lastk=23560;

    /* pruebas modo turbo en jupiter ace
    if (MACHINE_IS_ACE) {
        lastk=15398;
    //KEYCOD	3C26	15398	 1 byte. The ASCII code of the last key pressed.

        //15399	1 byte. Used by the routine that reads the keyboard.
        //Conteo de tiempo pulsada tecla??
        if (input_file_keyboard_is_playing() && dir==15399) {
            //poke_byte_no_time(dir,1);
            //return;
        }
    }
    */

        if (input_file_keyboard_is_playing() && dir==lastk) {
	                        z80_byte input_file_keyboard_last_key;

                                int leidos=fread(&input_file_keyboard_last_key,1,1,ptr_input_file_keyboard);
                                if (leidos==0) {
                                        debug_printf (VERBOSE_INFO,"Read 0 bytes of Input File Keyboard. End of file");
                                        eject_input_file_keyboard();
                                        reset_keyboard_ports();
                                }

                                //conversion de salto de linea
                                if (input_file_keyboard_last_key==10) input_file_keyboard_last_key=13;

                                poke_byte_no_time(lastk,input_file_keyboard_last_key);


        }
}

//Punteros a las funciones originales
z80_byte (*peek_byte_no_time_no_spoolturbo)(z80_int dir);
z80_byte (*peek_byte_no_spoolturbo)(z80_int dir);

z80_byte peek_byte_spoolturbo(z80_int dir)
{

	peek_byte_spoolturbo_check_key(dir);

	return peek_byte_no_spoolturbo(dir);
}

z80_byte peek_byte_no_time_spoolturbo(z80_int dir)
{

        peek_byte_spoolturbo_check_key(dir);

        return peek_byte_no_time_no_spoolturbo(dir);
}




void set_peek_byte_function_spoolturbo(void)
{

    debug_printf(VERBOSE_INFO,"Enabling spoolturbo on peek_byte");


	if (MACHINE_IS_SPECTRUM) {
		//Cambiar valores de repeticion de teclas
		poke_byte_no_time(23561,1);
		poke_byte_no_time(23562,1);
	}


    peek_byte_no_time_no_spoolturbo=peek_byte_no_time;
    peek_byte_no_time=peek_byte_no_time_spoolturbo;

    peek_byte_no_spoolturbo=peek_byte;
    peek_byte=peek_byte_spoolturbo;

}

void reset_peek_byte_function_spoolturbo(void)
{
	debug_printf(VERBOSE_INFO,"Resetting spoolturbo on peek_byte");

    if (MACHINE_IS_SPECTRUM) {
	    //Restaurar valores de repeticion de teclas
	    poke_byte_no_time(23561,35);
	    poke_byte_no_time(23562,5);
    }

    peek_byte_no_time=peek_byte_no_time_no_spoolturbo;
    peek_byte=peek_byte_no_spoolturbo;
}


//Funciones de Escritura en ROM
//Permitido en Spectrum 48k, Spectrum 16k, ZX80, ZX81, Jupiter Ace
//Punteros a las funciones originales
//void (*poke_byte_no_time_no_writerom)(z80_int dir,z80_byte value);
//void (*poke_byte_no_writerom)(z80_int dir,z80_byte value);


void poke_byte_writerom_exec(z80_int dir,z80_byte value)
{
	if (MACHINE_IS_SPECTRUM) {
		if (dir<16384) memoria_spectrum[dir]=value;
	}

	else if (MACHINE_IS_ZX80_TYPE) {
                if (dir<4096) memoria_spectrum[dir]=value;
        }

	else if (MACHINE_IS_ZX81_TYPE) {
                if (dir<8192) memoria_spectrum[dir]=value;
        }

	else if (MACHINE_IS_ACE) {
                if (dir<8192) memoria_spectrum[dir]=value;
        }

	else if (MACHINE_IS_SAM) {

                z80_byte *puntero;
                puntero=sam_return_segment_memory(dir);

                dir = dir & 16383;

                puntero=puntero+dir;
                *puntero=value;

	}
}

z80_byte poke_byte_writerom(z80_int dir,z80_byte value)
{

	poke_byte_writerom_exec(dir,value);

        //poke_byte_no_writerom(dir,value);
	debug_nested_poke_byte_call_previous(write_rom_nested_id_poke_byte,dir,value);

	//Para que no se queje el compilador
	return 0;

}

z80_byte poke_byte_no_time_writerom(z80_int dir,z80_byte value)
{

	poke_byte_writerom_exec(dir,value);

        //poke_byte_no_time_no_writerom(dir,value);
	debug_nested_poke_byte_no_time_call_previous(write_rom_nested_id_poke_byte_no_time,dir,value);

	//Para que no se queje el compilador
	return 0;
}




void set_poke_byte_function_writerom(void)
{

        debug_printf(VERBOSE_INFO,"Enabling Write on ROM on poke_byte");

        //poke_byte_no_time_no_writerom=poke_byte_no_time;
        //poke_byte_no_time=poke_byte_no_time_writerom;

        //poke_byte_no_writerom=poke_byte;
        //poke_byte=poke_byte_writerom;

        write_rom_nested_id_poke_byte=debug_nested_poke_byte_add(poke_byte_writerom,"Write rom poke_byte");
        write_rom_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(poke_byte_no_time_writerom,"Write rom poke_byte_no_time");

}

void reset_poke_byte_function_writerom(void)
{
        debug_printf(VERBOSE_INFO,"Resetting Write on ROM on poke_byte");
        //poke_byte_no_time=poke_byte_no_time_no_writerom;
        //poke_byte=poke_byte_no_writerom;

        debug_nested_poke_byte_del(write_rom_nested_id_poke_byte);
        debug_nested_poke_byte_no_time_del(write_rom_nested_id_poke_byte_no_time);

}

//Retorna nombre de cinta spectrum (10 bytes)
void util_tape_get_name_header(z80_byte *tape,char *texto)
{
	int i;
	z80_byte caracter;

	for (i=0;i<10;i++) {
		caracter=*tape++;
		if (caracter<32 || caracter>126) caracter='.';

		*texto++=caracter;
	}

	*texto=0;
}

void util_tape_get_info_tapeblock_ace(z80_byte *tape,z80_byte flag,z80_int longitud,char *texto)
{
	char buffer_nombre[11];

	z80_byte first_byte=tape[0];
		if (flag==0 && (first_byte==0 || first_byte==32) && longitud==27) {
			//Posible cabecera
			util_tape_get_name_header(&tape[1],buffer_nombre);

                        z80_int cabecera_longitud;
                        z80_int cabecera_inicio;

                        cabecera_longitud=value_8_to_16(tape[12],tape[11]);
                        cabecera_inicio=value_8_to_16(tape[14],tape[13]);



			//Evitar el uso del caracter ":" para evitar generar nombres (en el expansor de archivos) con ":" que pueden dar problemas en windows
			switch (first_byte) {
				case 0:
					sprintf(texto,"Dict %s [%d]",buffer_nombre,cabecera_longitud);
				break;

				case 32:
					sprintf(texto,"Bytes %s [%d,%d]",buffer_nombre,cabecera_inicio,cabecera_longitud);
				break;



			}
		}


		else sprintf(texto,"Flag %d Length %d",flag,longitud-2); //Saltar los 2 bytes de flag y checksum
}


void util_tape_get_info_tapeblock(z80_byte *tape,z80_byte flag,z80_int longitud,char *texto)
{

    //42 caracteres maximo de la info?
	char buffer_nombre[11];

	z80_byte first_byte=tape[0];
		if (flag==0 && first_byte<=4 && longitud==19) {
			//Posible cabecera
			util_tape_get_name_header(&tape[1],buffer_nombre);

            z80_int cabecera_longitud;
            z80_int cabecera_inicio;

            cabecera_longitud=value_8_to_16(tape[12],tape[11]);
            cabecera_inicio=value_8_to_16(tape[14],tape[13]);

            char array_var_name='a'+(tape[14] & 0x1F)-1;



			//Evitar el uso del caracter ":" para evitar generar nombres (en el expansor de archivos) con ":" que pueden dar problemas en windows
			switch (first_byte) {
				case 0:
                    //Si tiene autoinicio
                    if (cabecera_inicio<32768) sprintf(texto,"Program %s [size %d,LINE %d]",buffer_nombre,cabecera_longitud,cabecera_inicio);
					else sprintf(texto,"Program %s [size %d]",buffer_nombre,cabecera_longitud);
				break;

				case 1:
					sprintf(texto,"Num array %s [size %d,%c()]",buffer_nombre,cabecera_longitud,array_var_name);
				break;

				case 2:
					sprintf(texto,"Char array %s [size %d,%c$()]",buffer_nombre,cabecera_longitud,array_var_name);
				break;

				case 3:
                                        if (cabecera_longitud==6912 && cabecera_inicio==16384) sprintf(texto,"Screen$ %s",buffer_nombre);
                                                          //01234567890123456789012345678901
                                                          //Code: 1234567890 [16384,49152]
					else sprintf(texto,"Code %s [%d,%d]",buffer_nombre,cabecera_inicio,cabecera_longitud);
				break;

                //Esto solo soportado por Hilow
				case 4:
					sprintf(texto,"NMI %s",buffer_nombre);
				break;
			}
		}

		else if (flag==0 && first_byte==3 && longitud==36) {
			//Bloque de codigo fuente SPED
			util_tape_get_name_header(&tape[1],buffer_nombre);

                                     //01234567890123456789012345678901
                                     //SPED: 1234567890 (12/12/2099)
                        z80_byte sped_day=tape[18];
                        z80_byte sped_month=tape[19];
                        z80_byte sped_year=tape[20];

			sprintf(texto,"SPED %s (%d-%d-%d)",buffer_nombre,sped_day,sped_month,sped_year+1980);

		}


		else sprintf(texto,"Flag %d Length %d",flag,longitud-2); //Saltar los 2 bytes de flag y checksum
}

//Retorna texto descriptivo de informacion de cinta en texto. Cinta tipo tap o tzx
//Retorna longitud del bloque
//origin_tap es importante cuando se viene de un tap y maquina activa es jupiter ace, pues esos tap no tienen flag
int util_tape_tap_get_info(z80_byte *tape,char *texto,int origin_tap)
{
	int longitud=value_8_to_16(tape[1],tape[0]);

    //Soporte para cintas TAP de Jupiter Ace, que no tienen flag
    if (MACHINE_IS_ACE) {
        tape+=2;

        int flag;

        int longitud_aparente=longitud;

        if (!origin_tap) {
            flag=tape[0];
            tape++;
            //printf("flag leido: %d\n",flag);
        }
        else {
            //es un tap, flag no se usa. Asumimos que es 0 si es bloque de cabecera (longitud 16)
            if (longitud_aparente==26) flag=0;
            else flag=255;

            //Y le aumentamos artificialmente la longitud para que funcione bien la llamada a util_tape_get_info_tapeblock_ace
            longitud_aparente++;
        }

        //printf("longitud %d flag %d\n",longitud_aparente,flag);


        //Si longitud<2, es error
        if (longitud_aparente<2) strcpy(texto,"Corrupt tape");
        else {
            //Maximo 25 bytes
            z80_byte buffer_temp[25];
            util_memcpy_protect_origin(buffer_temp, tape, 25, 0, 25);

            //de momento
            util_tape_get_info_tapeblock_ace(buffer_temp,flag,longitud_aparente,texto);


        }
    }

    else {

        int flag=tape[2];
        tape+=3;


        //Si longitud<2, es error
        if (longitud<2) strcpy(texto,"Corrupt tape");
        else {
            //Maximo 36 bytes (en una cabecera tipo SPED). Copiamos a buffer temporal para evitar que se salga puntero de sitio
            z80_byte buffer_temp[36];
            util_memcpy_protect_origin(buffer_temp, tape, 36, 0, 36);
            util_tape_get_info_tapeblock(buffer_temp,flag,longitud,texto);


        }

    }


	return longitud+2;
}


//Peek byte multiple para Z80 y Motorola
//Esta deberia ser la rutina preferida para llamarse desde menu y otros sitios
z80_byte peek_byte_z80_moto(unsigned int address)
{
  address=adjust_address_space_cpu(address);

  //if (MACHINE_IS_QL) return ql_readbyte_no_ports(address);
  if (MACHINE_IS_QL) return ql_readbyte_no_ports_function(address);

  else return peek_byte_no_time(address);
}

void poke_byte_z80_moto(unsigned int address,z80_byte valor)
{
  address=adjust_address_space_cpu(address);
  if (MACHINE_IS_QL) ql_writebyte_no_ports(address,valor);
  else poke_byte_no_time(address,valor);
}

unsigned int get_ql_pc(void)
{
  return m68k_get_reg(NULL, M68K_REG_PC);
}

unsigned int get_pc_register(void)
{
  if (CPU_IS_SCMP) return scmp_m_PC.w.l;
  else if (CPU_IS_MOTOROLA) return get_ql_pc();
  else return reg_pc;
}

unsigned int adjust_address_space_cpu(unsigned int direccion)
{
  if (!CPU_IS_MOTOROLA) direccion &=0xFFFF;
  else {
        //direccion &=0xFFFFF; //20 bits direccion
        direccion &=QL_MEM_LIMIT; //si son 256k totales, esto vale 0x3FFFF
  }

  return direccion;
}


void convert_signed_unsigned(char *origen, unsigned char *destino,int longitud)
{
        int i;
        z80_byte leido;

        for (i=0;i<longitud;i++) {
                leido=*origen;
                leido=128+leido;

                *destino=leido;
                destino++;

                origen++;
        }

}

//Retorna 0 si ok
/*
Al descomprimir bloques de datos de RZX suele decir:
gunzip: invalid compressed data--crc error
Aunque descomprime bien. Sera porque en caso del RZX le agrego una cabecera manualmente
*/
int uncompress_gz(char *origen,char *destino)
{

  debug_printf(VERBOSE_INFO,"Uncompressing %s to %s",origen,destino);

//Descomprimir
char uncompress_command[PATH_MAX];
char uncompress_program[PATH_MAX];


sprintf (uncompress_program,"%s",external_tool_gunzip);
sprintf (uncompress_command,"cat %s | %s -c  > \"%s\" ",origen,external_tool_gunzip,destino);

if (system (uncompress_command)==-1) return 1;

return 0;
}

char *mingw_strcasestr(const char *arg1, const char *arg2)
{
   const char *a, *b;

   for(;*arg1;arg1++) {

     a = arg1;
     b = arg2;

     while((*a | 32) == (*b | 32)) {
	a++;
	b++;
       if(!*b)
         return (char *)arg1;
     }

   }

   return NULL;
}

char *util_strcasestr(char *string, char *string_a_buscar)
{

        //Debe coincidir desde el principio de string
#ifdef MINGW
        //En Windows no esta la funcion strcasestr
        char *coincide=mingw_strcasestr(string,string_a_buscar);
#else
        char *coincide=strcasestr(string,string_a_buscar);
#endif

        return coincide;

}


void util_sprintf_address_hex(menu_z80_moto_int p,char *string_address)
{
  if (CPU_IS_MOTOROLA) {
    sprintf (string_address,"%06XH",p);
  }

  else {
    sprintf (string_address,"%04XH",p);
  }
}



//Separar comando con codigos 0 y rellenar array de parametros
int util_parse_commands_argvc(char *texto, char *parm_argv[], int maximo)
{

        int args=0;

        while (*texto) {
                //Inicio parametro
                parm_argv[args++]=texto;
                if (args==maximo) {
                        debug_printf(VERBOSE_DEBUG,"Max parameters reached (%d)",maximo);
                        return args;
                }

                //Ir hasta espacio o final
                while (*texto && *texto!=' ') {
                        texto++;
                }

                if ( (*texto)==0) return args;

                *texto=0; //Separar cadena
                texto++;
        }

        return args;
}


//Separar comando con codigos 0 y rellenar array de parametros, parecido a
//util_parse_commands_argvc pero tienendo en cuenta si hay comillas
int util_parse_commands_argvc_comillas(char *texto, char *parm_argv[], int maximo)
{

        int args=0;

        //Si se han leido en algun momento al parsear un parametro
        int comillas_algun_momento=0;


        while (*texto) {
                //Inicio parametro
                parm_argv[args++]=texto;
                if (args==maximo) {
                        debug_printf(VERBOSE_DEBUG,"Max parameters reached (%d)",maximo);
                        return args;
                }

                //Ir hasta espacio o final

                comillas_algun_momento=0;

                //Variable que va conmutando segun se lee
                int comillas_leidas=0;

                int antes_escape=0;

                //TODO: al escapar comillas , incluye el caracter de escape tambien
                //esto sucede porque estamos escribiendo en el mismo sitio que leemos
                //deberia tener una cadena origen y una destino distintas

                //TODO: controlar parametros como : 123"456 -> en este caso resulta: 23"45

                while (*texto && (*texto!=' ' || comillas_leidas)  ) {
                        //Si son comillas y no escapadas
                        if ((*texto)=='"' && !antes_escape) {
                                //printf ("Leemos comillas\n");
                                comillas_leidas ^=1;

                                comillas_algun_momento=1;
                        }

                        if ( (*texto)=='\\') antes_escape=1;
                        else antes_escape=0;

                        texto++;
                }

                if (comillas_algun_momento) {
                        //Se ha leido en algun momento. Quitar comillas iniciales y finales
                        //Lo que hacemos es cambiar el inicio de parametro a +1

                        //printf ("Ajustar parametro porque esta entre comillas\n");

                        char *inicio_parametro;
                        inicio_parametro=parm_argv[args-1];

                        inicio_parametro++;

                        parm_argv[args-1]=inicio_parametro;

                        //Y quitar comillas del final
                        *(texto-1)=0;

                        //printf ("Parametro ajustado sin comillas: [%s]\n",parm_argv[args-1]);

                }

                if ( (*texto)==0) return args;



                *texto=0; //Separar cadena
                texto++;
        }

        return args;
}

//Retorna 0 si ok. No 0 si error. Ancho expresado en pixeles. Alto expresado en pixeles
//Source es en crudo bytes monocromos. ppb sera 8 siempre
int util_write_pbm_file(char *archivo, int ancho, int alto, int ppb, z80_byte *source)
{

  FILE *ptr_destino;
  ptr_destino=fopen(archivo,"wb");

  if (ptr_destino==NULL) {
    debug_printf (VERBOSE_ERR,"Error writing pbm file");
    return 1;
  }


    //Escribir cabecera pbm
  	char pbm_header[20]; //Suficiente

  	sprintf (pbm_header,"P4\n%d %0d\n",ancho,alto);

  	fwrite(pbm_header,1,strlen(pbm_header),ptr_destino);


  int x,y;
  for (y=0;y<alto;y++) {
    for (x=0;x<ancho/ppb;x++) {
      fwrite(source,1,1,ptr_destino);
      source++;
    }
  }

  fclose(ptr_destino);

  return 0;
}

int util_write_stl_file(char *archivo, int ancho, int alto, z80_byte *source,int incluir_base,int alto_base,int exponente,int alto_solido)
{

    FILE *ptr_destino;
    ptr_destino=fopen(archivo,"wb");

    if (ptr_destino==NULL) {
        debug_printf (VERBOSE_ERR,"Error writing pbm file");
        return 1;
    }


    //Escribir cabecera stl
    char stl_header[256]; //Suficiente

    sprintf (stl_header,"solid ZEsarUX\n");

    fwrite(stl_header,1,strlen(stl_header),ptr_destino);


    //int exponente=0; //Escala: 1 pixel=1 mm
    int exponente_z=0;

    int z=0; //empezar en
    //int alto_base=1;
    if (incluir_base) z+=alto_base;

    //int alto_solido=10; //Cuanto de altura z, esta valor multiplicado *10 exponente

    //para determinar recuadro
    int max_y=0;
    int min_y=191;
    int max_x=0;
    int min_x=255;

    int x,y,bit;
        for (y=alto-1;y>0;y--) {
            for (x=0;x<ancho;x+=8) {
            z80_byte byte_leido=*source;
            for (bit=0;bit<8;bit++) {

                //if (incluir_base) util_stl_cube(ptr_destino,x+bit,alto-1-y,0,exponente,exponente_z,1);

                if (byte_leido&128) {
                    util_stl_cube(ptr_destino,x+bit,y,z,exponente,exponente_z,1,1,alto_solido);

                    if (x+bit>max_x) max_x=x+bit;
                    if (y>max_y) max_y=y;

                    if (x+bit<min_x) min_x=x+bit;
                    if (y<min_y) min_y=y;
                }
                byte_leido=byte_leido<<1;
            }
            source++;
        }
    }

    //generar la base si conviene
    if (incluir_base) {
        util_stl_cube(ptr_destino,min_x,min_y,0,
        exponente,0,
        max_x+1-min_x,max_y+1-min_y,alto_base);
    }

    //Escribir fin stl
  	sprintf (stl_header,"endsolid\n");

  	fwrite(stl_header,1,strlen(stl_header),ptr_destino);

    fclose(ptr_destino);

    return 0;
}

//Retorna 0 si ok. No 0 si error. Ancho expresado en pixeles. Alto expresado en pixeles
//Source es en crudo bytes monocromos. ppb sera 8 siempre
int util_write_sprite_c_file(char *archivo, int ancho, int alto, int ppb, z80_byte *source)
{

  FILE *ptr_destino;
  ptr_destino=fopen(archivo,"wb");

  if (ptr_destino==NULL) {
    debug_printf (VERBOSE_ERR,"Error writing pbm file");
    return 1;
  }


    //Escribir cabecera pbm
        char *c_header="//Created by ZEsarUX emulator\n\nunsigned char mysprite[]={\n";
        char *c_end_header="};\n";

  	fwrite(c_header,1,strlen(c_header),ptr_destino);

          char buffer_linea[30]; // "0xFF," (5 caracteres. Damos mas por si acaso extendemos)


  int x,y;
  for (y=0;y<alto;y++) {
    for (x=0;x<ancho/ppb;x++) {
            sprintf (buffer_linea,"0x%02X,",*source);
        fwrite(buffer_linea,1,strlen(buffer_linea),ptr_destino);
      source++;
    }

    //Meter salto de linea cada cambio de posicion y
    char nl='\n';
    fwrite(&nl,1,1,ptr_destino);

  }

        //final de archivo
fwrite(c_end_header,1,strlen(c_end_header),ptr_destino);
  fclose(ptr_destino);

  return 0;
}


void util_truncate_file(char *filename)
{

	debug_printf(VERBOSE_INFO,"Truncating file %s",filename);

	FILE *ptr_destino;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_write(filename,&in_fatfs,&ptr_destino,&fil)<0) {
        debug_printf (VERBOSE_ERR,"Error truncating file");
        return;
    }


    /*
	ptr_destino=fopen(filename,"wb");

  	if (ptr_destino==NULL) {
    		debug_printf (VERBOSE_ERR,"Error truncating file");
    		return;
    	}
    */

   zvfs_fclose(in_fatfs,ptr_destino,&fil);
    	//fclose(ptr_destino);

}


//Retorna tamanyo de zona y actualiza puntero a memoria indicada
//Si es 0, no existe
//Attrib: bit 0: read, bit 1: write

unsigned int machine_get_memory_zone_attrib(int zone, int *readwrite)
{

  //Por defecto
  int size=0;
  *readwrite=1; //1 read only

  //Zona 0, ram speccy
  switch (zone) {
    case 0:


      *readwrite=3; //1 read, 2 write
      //como fallback, por defecto en spectrum 48kb ram
      size=49152;

      if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
        size=131072*mem128_multiplicador;
      }

      if (MACHINE_IS_CHLOE) {
        size=131072;
      }

      //Vigilar condiciones que pueden cumplir mas de una maquina a la vez

      if (MACHINE_IS_ZXUNO) {
        size=ZXUNO_SRAM_PAGES*16384;
      }

      if (MACHINE_IS_TBBLUE) {
        size=2*1024*1024; //Retornamos siempre zona memoria 2 MB
      }

      if (MACHINE_IS_TSCONF) {
        size=TSCONF_RAM_PAGES*16384;
      }

        if (MACHINE_IS_BASECONF) {
        size=BASECONF_RAM_PAGES*16384;
      }

      if (MACHINE_IS_QL) {
        size=QL_MEM_LIMIT+1-131072;
      }

      if (MACHINE_IS_CPC_HAS_64K) {
        size=65536;
      }

      if (MACHINE_IS_CPC_HAS_128K) {
        size=131072;
      }

      if (MACHINE_IS_PCW) {
        size=pcw_total_ram;
      }

      if (MACHINE_IS_INVES) {
	size=65536;
      }

      if (MACHINE_IS_SAM) {
	size=512*1024; //512 kb para simplificar->siempre el maximo
      }

      if (MACHINE_IS_Z88) {
      	size=0; //Solo zona memoria de 4 mb en caso de z88
      }

      if (MACHINE_IS_MSX) {
      	size=0; //Mostrar zona de memoria de 256kb en caso de msx
      }

       if (MACHINE_IS_SVI) {
      	size=0; //Mostrar zona de memoria de 256kb en caso de msx
      }

      if (MACHINE_IS_SMS) {
          size=8192;
      }

    break;

    //Zona rom
    case 1:


      *readwrite=1; //1 read, 2 write
      //como fallback por defecto en spectrum 16kb rom
      size=16384;

      if (MACHINE_IS_SPECTRUM_128_P2) {
        size=32768;
      }

      if (MACHINE_IS_SPECTRUM_P2A_P3) {
        size=65536;
      }

	if (MACHINE_IS_CHLOE) {
		size=32768;
	}

      //Vigilar condiciones que pueden cumplir mas de una maquina a la vez

      if (MACHINE_IS_ZXUNO) {
        size=16384;
      }

      if (MACHINE_IS_TBBLUE) {
        size=8192;
      }

      if (MACHINE_IS_TSCONF) {
        size=TSCONF_ROM_PAGES*16384;
      }

      if (MACHINE_IS_BASECONF) {
        size=BASECONF_ROM_PAGES*16384;
      }

      if (MACHINE_IS_ZX81_TYPE) {
        size=8192;
      }

      if (MACHINE_IS_ZX80_TYPE) {
        size=4096;
      }

      if (MACHINE_IS_QL) {
        size=49152;
      }

      if (MACHINE_IS_CPC_464) {
        size=32768;
      }

      if (MACHINE_IS_CPC_4128) {
        size=32768;
      }

      if (MACHINE_IS_CPC_HAS_FLOPPY) {
        size=49152;
      }


      if (MACHINE_IS_SAM) {
	size=32768;
      }

      if (MACHINE_IS_MSX) {
	size=32768;
      }
      if (MACHINE_IS_SVI) {
	size=32768;
      }

      if (MACHINE_IS_Z88) {
      	size=0; //Solo zona memoria de 4 mb en caso de z88
      }

      if (MACHINE_IS_SMS) {
          size=sms_cartridge_size;
      }


    break;

    //diviface rom
    case 2:
      if (diviface_enabled.v) {
        *readwrite=1;
        size=DIVIFACE_FIRMWARE_KB*1024;
      }
    break;

    //diviface ram
    case 3:
      if (diviface_enabled.v) {
        *readwrite=3;
        size=(get_diviface_total_ram())*1024;
      }
    break;

    //zxuno flash
    case 4:
      if (MACHINE_IS_ZXUNO) {
        *readwrite=3;
        size=ZXUNO_SPI_SIZE*1024;
      }
    break;

    //tsconf fmaps
    case 5:
      if (MACHINE_IS_TSCONF) {
        *readwrite=3;
        size=TSCONF_FMAPS_SIZE;
      }
    break;

    //kartusho
    case 6:
	if (kartusho_enabled.v) {
          *readwrite=1;
	  size=KARTUSHO_SIZE;
	}
    break;

    //dandanator
    case 7:
        if (dandanator_enabled.v) {
          *readwrite=1;
          size=DANDANATOR_SIZE;
        }
    break;


    //Superupgrade ram
    case 8:
        if (superupgrade_enabled.v) {
          *readwrite=3;
          size=SUPERUPGRADE_RAM_SIZE;
        }
    break;

    //Superupgrade rom
    case 9:
        if (superupgrade_enabled.v) {
          *readwrite=1;
          size=SUPERUPGRADE_ROM_SIZE;
        }
    break;

    //Z88 memory
    case 10:
    	if (MACHINE_IS_Z88) {
                *readwrite=1;
    		size=4*1024*1024; //Zona entera de los 4 MB
    	}
    break;



    //Betadisk
    case 11:
    	if (betadisk_enabled.v) {
                *readwrite=1;
    		size=BETADISK_SIZE;
    	}
    break;


    //Multiface rom
    case 12:
    	if (multiface_enabled.v) {
                *readwrite=1;
    		size=8192;
    	}
    break;

    //Multiface ram
    case 13:
    	if (multiface_enabled.v) {
                *readwrite=3;
    		size=8192;
    	}
    break;

    //tbblue sprites
    case 14:
      if (MACHINE_IS_TBBLUE) {
        *readwrite=3;
        size=TBBLUE_SPRITE_ARRAY_PATTERN_SIZE;
      }
    break;

    //tsconf sprites
    case 15:
      if (MACHINE_IS_TSCONF) {
        *readwrite=3;
        size=4*1024*1024; //Puede ser toda la ram
      }
    break;

	//memory zone by file. 16
    case MEMORY_ZONE_NUM_FILE_ZONE:
      if (memory_zone_by_file_size>0) {
        *readwrite=3;
        size=memory_zone_by_file_size;
      }
    break;

    //tbblue copper
    case MEMORY_ZONE_NUM_TBBLUE_COPPER:
      if (MACHINE_IS_TBBLUE) {
        *readwrite=3;
        size=TBBLUE_COPPER_MEMORY;
      }
    break;

    case MEMORY_ZONE_NUM_TIMEX_EX:
      if (MACHINE_IS_TIMEX_TS_TC_2068) {
        *readwrite=1;
        size=8192;
      }
      if (MACHINE_IS_CHLOE_280SE) {
	*readwrite=1;
	size=65536;
      }
    break;

    case MEMORY_ZONE_NUM_TIMEX_DOCK:
      if (MACHINE_IS_TIMEX_TS_TC_2068) {
        *readwrite=1;
        size=65536;
      }
      if (MACHINE_IS_CHLOE_280SE) {
	*readwrite=1;
	size=65536;
      }
    break;


        case MEMORY_ZONE_NUM_DAAD_CONDACTS:
                if (MACHINE_IS_SPECTRUM) {
                        if (util_daad_detect()) size=65536;
                }

                if (MACHINE_IS_CPC) {
                        if (util_daad_detect()) size=65536;
                }

        break;

        case MEMORY_ZONE_NUM_PAWS_CONDACTS:
                if (MACHINE_IS_SPECTRUM) {
                        if (util_textadv_detect_paws_quill()) size=65536;
                }


        break;

        case MEMORY_ZONE_DEBUG:
                if (memory_zone_current_size) {
                        size=memory_zone_current_size;
                }
        break;


    //ifrom
    case MEMORY_ZONE_IFROM:
	if (ifrom_enabled.v) {
          *readwrite=1;
	  size=IFROM_SIZE;
	}
    break;


    case MEMORY_ZONE_MSX_VRAM:
        if (MACHINE_IS_MSX) {
              *readwrite=3; //read+write
              size=16384;
        }
    break;

    case MEMORY_ZONE_MSX_ALL_MEM:
        if (MACHINE_IS_MSX) {
              *readwrite=1;
              size=256*1024;  //Zona entera de los 256kb
        }
    break;


    case MEMORY_ZONE_SVI_VRAM:
        if (MACHINE_IS_SVI) {
              *readwrite=3; //read+write
              size=16384;
        }
    break;

    case MEMORY_ZONE_SVI_ALL_MEM:
        if (MACHINE_IS_SVI) {
              *readwrite=1;
              size=256*1024;  //Zona entera de los 256kb
        }
    break;

    case MEMORY_ZONE_COLECO_VRAM:
        if (MACHINE_IS_COLECO) {
              *readwrite=3; //read+write
              size=16384;
        }
    break;


    case MEMORY_ZONE_SG1000_VRAM:
        if (MACHINE_IS_SG1000) {
              *readwrite=3; //read+write
              size=16384;
        }
    break;

    case MEMORY_ZONE_SMS_VRAM:
        if (MACHINE_IS_SMS) {
              *readwrite=3; //read+write
              size=16384;
        }
    break;

    case MEMORY_ZONE_SAMRAM:
        if (samram_enabled.v) {
              *readwrite=3; //read+write
              size=SAMRAM_SIZE;
        }
    break;

    case MEMORY_ZONE_NUM_TBBLUE_SPRITES:
        if (MACHINE_IS_TBBLUE) {
            *readwrite=3; //read+write
            size=TBBLUE_MAX_SPRITES*TBBLUE_SPRITE_ATTRIBUTE_SIZE;
        }
    break;

    //hilow rom
    case MEMORY_ZONE_HILOW_ROM:
        if (hilow_enabled.v) {
            *readwrite=1;
            size=HILOW_ROM_SIZE;
        }
    break;


    //hilow ram
    case MEMORY_ZONE_HILOW_RAM:
        if (hilow_enabled.v) {
            *readwrite=3; //read+write
            size=HILOW_RAM_SIZE;
        }
    break;

    //hilow device
    case MEMORY_ZONE_HILOW_DEVICE:
        if (hilow_enabled.v && hilow_rom_traps.v) {
            *readwrite=3; //read+write
            size=HILOW_DEVICE_SIZE;
        }
    break;

    //hilow buffer intermedio de conversion
    case MEMORY_ZONE_HILOW_CONVERT_READ:
        if (menu_hilow_convert_audio_has_been_opened) {
            *readwrite=3; //read+write
            size=HILOW_SECTOR_SIZE;
        }
    break;

    //hilow barbanegra rom
    case MEMORY_ZONE_HILOW_BARBANEGRA_ROM:
        if (hilow_bbn_enabled.v) {
            *readwrite=1;
            size=HILOW_BARBANEGRA_ROM_SIZE;
        }
    break;

    //hilow barbanegra ram
    case MEMORY_ZONE_HILOW_BARBANEGRA_RAM:
        if (hilow_bbn_enabled.v) {
            *readwrite=3; //read+write
            size=HILOW_BARBANEGRA_RAM_SIZE;
        }
    break;


    //transtape rom
    case MEMORY_ZONE_TRANSTAPE_ROM:
        if (transtape_enabled.v) {
            *readwrite=1;
            size=TRANSTAPE_ROM_SIZE;
        }
    break;

    //transtape ram
    case MEMORY_ZONE_TRANSTAPE_RAM:
        if (transtape_enabled.v) {
            *readwrite=3; //read+write
            size=TRANSTAPE_RAM_SIZE;
        }
    break;

    case MEMORY_ZONE_MHPOKEADOR_RAM:
        if (mhpokeador_enabled.v) {
            *readwrite=3; //read+write
            size=MHPOKEADOR_RAM_SIZE;
        }
    break;

    //specmate rom
    case MEMORY_ZONE_SPECMATE_ROM:
        if (specmate_enabled.v) {
            *readwrite=1;
            size=SPECMATE_ROM_SIZE;
        }
    break;

    //phoenix rom
    case MEMORY_ZONE_PHOENIX_ROM:
        if (phoenix_enabled.v) {
            *readwrite=1;
            size=PHOENIX_ROM_SIZE;
        }
    break;

    //defcon rom
    case MEMORY_ZONE_DEFCON_ROM:
        if (defcon_enabled.v) {
            *readwrite=1;
            size=DEFCON_ROM_SIZE;
        }
    break;

    //ramjet rom
    case MEMORY_ZONE_RAMJET_ROM:
        if (ramjet_enabled.v) {
            *readwrite=1;
            size=RAMJET_ROM_SIZE;
        }
    break;

    //interface007 rom
    case MEMORY_ZONE_INTERFACE007_ROM:
        if (interface007_enabled.v) {
            *readwrite=1;
            size=INTERFACE007_ROM_SIZE;
        }
    break;

    //dinamid3 rom
    case MEMORY_ZONE_DINAMID3_ROM:
        if (dinamid3_enabled.v) {
            *readwrite=1;
            size=DINAMID3_ROM_SIZE;
        }
    break;

    case MEMORY_ZONE_DSK_SECTOR:
        if (dsk_memory_zone_dsk_sector_enabled.v) {
            *readwrite=1;
            size=dsk_memory_zone_dsk_sector_size;
        }
    break;

    case MEMORY_ZONE_LEC_MEMORY:
        if (lec_enabled.v) {
            *readwrite=1;
            size=lec_get_total_memory_size();
        }
    break;

    case MEMORY_ZONE_MDV1:
        if (microdrive_status[0].microdrive_enabled) {
            *readwrite=1;
            if (microdrive_status[0].raw_format) {
                size=microdrive_status[0].raw_total_size;
            }
            else size=microdrive_status[0].mdr_total_sectors*MDR_BYTES_PER_SECTOR;
        }
    break;

    case MEMORY_ZONE_MDV2:
        if (microdrive_status[1].microdrive_enabled) {
            *readwrite=1;
            if (microdrive_status[1].raw_format) {
                size=microdrive_status[1].raw_total_size;
            }
            else size=microdrive_status[1].mdr_total_sectors*MDR_BYTES_PER_SECTOR;
        }
    break;

    case MEMORY_ZONE_MDV3:
        if (microdrive_status[2].microdrive_enabled) {
            *readwrite=1;
            if (microdrive_status[2].raw_format) {
                size=microdrive_status[2].raw_total_size;
            }
            else size=microdrive_status[2].mdr_total_sectors*MDR_BYTES_PER_SECTOR;
        }
    break;

    case MEMORY_ZONE_MDV4:
        if (microdrive_status[3].microdrive_enabled) {
            *readwrite=1;
            if (microdrive_status[3].raw_format) {
                size=microdrive_status[3].raw_total_size;
            }
            else size=microdrive_status[3].mdr_total_sectors*MDR_BYTES_PER_SECTOR;
        }
    break;

  }

  return size;

}


z80_byte *machine_get_memory_zone_pointer(int zone, int address)
{

  z80_byte *p=NULL;

  //Zona 0, ram speccy
  switch (zone) {
    case 0:

    //Caso normal 48k como fallback
          p=&memoria_spectrum[address+16384];




      if (MACHINE_IS_SPECTRUM_128_P2) {
        //Saltar los 32kb de rom
        p=&memoria_spectrum[address+32768];
      }

      if (MACHINE_IS_SPECTRUM_P2A_P3) {
        //Saltar los 64kb de rom
        p=&memoria_spectrum[address+65536];
      }

	if (MACHINE_IS_CHLOE) {
	z80_byte *start=chloe_home_ram_mem_table[0];
	p=&start[address];
	}

      //Vigilar condiciones que pueden cumplir mas de una maquina a la vez

      if (MACHINE_IS_ZXUNO) {
        //saltar los primeros 16 kb de la rom del zxuno
        p=&memoria_spectrum[address+16384];

      }

      if (MACHINE_IS_TBBLUE) {
        p=&memoria_spectrum[address];
      }

      if (MACHINE_IS_TSCONF) {
        z80_byte *start=tsconf_ram_mem_table[0];
        p=&start[address];
      }

      if (MACHINE_IS_BASECONF) {
        z80_byte *start=baseconf_ram_mem_table[0];
        p=&start[address];
      }

      if (MACHINE_IS_QL) {
        p=&memoria_spectrum[address+131072];
      }

      if (MACHINE_IS_CPC) {
        z80_byte *start=cpc_ram_mem_table[0];
        p=&start[address];
      }

      if (MACHINE_IS_PCW) {
        z80_byte *start=pcw_ram_mem_table[0];
        p=&start[address];
      }

      if (MACHINE_IS_INVES) {
	p=&memoria_spectrum[address];
      }

      if (MACHINE_IS_SAM) {
	z80_byte *start=sam_ram_memory[0];
        p=&start[address];
      }

      if (MACHINE_IS_SMS) {
          p=&memoria_spectrum[SMS_MAX_ROM_SIZE + address];
      }



    break;


    //Zona 1, rom speccy
    case 1:

      //Caso normal 48k como fallback
      p=&memoria_spectrum[address];

      //En resto de maquinas suele tambien estar al principio del puntero memoria_spectrum

      if (MACHINE_IS_TBBLUE) {
        p=&tbblue_fpga_rom[address];
      }

      if (MACHINE_IS_INVES) {
	p=&memoria_spectrum[65536+address];
      }

    break;


    //diviface rom
    case 2:
      if (diviface_enabled.v) {
        p=&diviface_memory_pointer[address];
      }
    break;

    //diviface ram
    case 3:
      if (diviface_enabled.v) {
        p=&diviface_ram_memory_pointer[address];
      }
    break;

    //zxuno flash
    case 4:
      if (MACHINE_IS_ZXUNO) {
        p=&memoria_spectrum[ (ZXUNO_ROM_SIZE+ZXUNO_SRAM_SIZE)*1024 + address ];
      }
    break;

    case 5:
      if (MACHINE_IS_TSCONF) {
        p=&tsconf_fmaps[address];
      }
    break;


    //kartusho
    case 6:
        if (kartusho_enabled.v) {
	p=&kartusho_memory_pointer[address];
      }
    break;

    //dandanator
    case 7:
        if (dandanator_enabled.v) {
        p=&dandanator_memory_pointer[address];
      }
    break;


    //superupgrade ram
    case 8:
        if (superupgrade_enabled.v) {
        p=&superupgrade_ram_memory_pointer[address];
      }
    break;

    //superupgrade rom
    case 9:
        if (superupgrade_enabled.v) {
        p=&superupgrade_rom_memory_pointer[address];
      }
    break;

    case 10:
    	if (MACHINE_IS_Z88) {
    		p=&memoria_spectrum[address];
    	}
    break;

    //Betadisk
    case 11:
	if (betadisk_enabled.v) {
    		p=&betadisk_memory_pointer[address];
    	}
    break;


    //Multiface rom
    case 12:
    	if (multiface_enabled.v) {
    		p=&multiface_memory_pointer[address];
    	}
    break;

    //Multiface ram
    case 13:
    	if (multiface_enabled.v) {
    		p=&multiface_memory_pointer[address+8192];
    	}
    break;


    //tbblue sprites
    case 14:
      if (MACHINE_IS_TBBLUE) {
        p=tbsprite_new_patterns;
        p=p+address;
      }
    break;


    //tsconf sprites
    case 15:
      if (MACHINE_IS_TSCONF) {
        p=tsconf_ram_mem_table[0];
        int offset=tsconf_return_spritesgraphicspage();
        offset+=address;

        //limitar a 4 mb
        offset &=0x3FFFFF;
        p +=offset;
      }
    break;

	//memory zone by file. 16
	case MEMORY_ZONE_NUM_FILE_ZONE:
		if (memory_zone_by_file_size>0) {
			p=&memory_zone_by_file_pointer[address];
		}
	break;

    //tbblue copper
    case MEMORY_ZONE_NUM_TBBLUE_COPPER:
      if (MACHINE_IS_TBBLUE) {
        p=tbblue_copper_memory;
        p=p+address;
      }
    break;

    case MEMORY_ZONE_NUM_TIMEX_EX:
      if (MACHINE_IS_TIMEX_TS_TC_2068) {
        z80_byte *start=timex_ex_rom_mem_table[0];
        p=&start[address];
      }
      if (MACHINE_IS_CHLOE_280SE) {
        z80_byte *start=chloe_ex_ram_mem_table[0];
        p=&start[address];
      }
    break;

    case MEMORY_ZONE_NUM_TIMEX_DOCK:
      if (MACHINE_IS_TIMEX_TS_TC_2068) {
        z80_byte *start=timex_dock_rom_mem_table[0];
        p=&start[address];
      }
      if (MACHINE_IS_CHLOE_280SE) {
        z80_byte *start=chloe_dock_ram_mem_table[0];
        p=&start[address];
      }
    break;

        case MEMORY_ZONE_NUM_DAAD_CONDACTS:
                if (MACHINE_IS_SPECTRUM) {
                        if (util_daad_detect()) {
                                //La direccion está en la zona de memoria ram (zona 0)
                                //No tiene sentido evaluar entre 0-16383. En ese caso sera igual que 16384-32767
                                if (address<16384) address +=16384;
                                z80_byte *start=machine_get_memory_zone_pointer(0,address-16384);
                                //Nota: la direccion dentro de la zona de memoria sera la misma que la direccion real en memoria mapeada
                                //restamos 16384 pues la zona 0 de ram empieza a contar desde ahi
                                p=start;
                        }
                }

                if (MACHINE_IS_CPC) {
                        if (util_daad_detect()) {
                                //La direccion está en la zona de memoria ram (zona 0)
                                z80_byte *start=machine_get_memory_zone_pointer(0,address);
                                p=start;
                        }
                }

        break;


        case MEMORY_ZONE_NUM_PAWS_CONDACTS:
                if (MACHINE_IS_SPECTRUM) {
                        if (util_textadv_detect_paws_quill()) {
                                //La direccion está en la zona de memoria ram (zona 0)
                                //No tiene sentido evaluar entre 0-16383. En ese caso sera igual que 16384-32767
                                if (address<16384) address +=16384;
                                z80_byte *start=machine_get_memory_zone_pointer(0,address-16384);
                                //Nota: la direccion dentro de la zona de memoria sera la misma que la direccion real en memoria mapeada
                                //restamos 16384 pues la zona 0 de ram empieza a contar desde ahi
                                p=start;
                        }
                }



        break;

        case MEMORY_ZONE_DEBUG:
                if (memory_zone_current_size) {
                        p=&memory_zone_debug_ptr[address];
                }
        break;

    //ifrom
    case MEMORY_ZONE_IFROM:
        if (ifrom_enabled.v) {
	p=&ifrom_memory_pointer[address];
      }
    break;


    case MEMORY_ZONE_MSX_VRAM:
        if (MACHINE_IS_MSX) {
                p=&msx_vram_memory[address];
        }
    break;

    case MEMORY_ZONE_MSX_ALL_MEM:
        if (MACHINE_IS_MSX) {
                p=&memoria_spectrum[address];
        }
    break;


    case MEMORY_ZONE_SVI_VRAM:
        if (MACHINE_IS_SVI) {
                p=&svi_vram_memory[address];
        }
    break;

    case MEMORY_ZONE_SVI_ALL_MEM:
        if (MACHINE_IS_SVI) {
                p=&memoria_spectrum[address];
        }
    break;


    case MEMORY_ZONE_COLECO_VRAM:
        if (MACHINE_IS_COLECO) {
                p=&coleco_vram_memory[address];
        }
    break;

    case MEMORY_ZONE_SG1000_VRAM:
        if (MACHINE_IS_SG1000) {
                p=&sg1000_vram_memory[address];
        }
    break;

    case MEMORY_ZONE_SMS_VRAM:
        if (MACHINE_IS_SMS) {
                p=&sms_vram_memory[address];
        }
    break;

    case MEMORY_ZONE_SAMRAM:
        if (samram_enabled.v) {
              p=&samram_memory_pointer[address];
        }
    break;

    case MEMORY_ZONE_NUM_TBBLUE_SPRITES:
        if (MACHINE_IS_TBBLUE) {
            int sprite_offset=address / TBBLUE_SPRITE_ATTRIBUTE_SIZE;
            int sprite_attr_index=address % TBBLUE_SPRITE_ATTRIBUTE_SIZE;
            p=&tbsprite_new_sprites[sprite_offset][sprite_attr_index];

            //Por si alguien escribe desde aqui, que indique el ultimo sprite del optimizador, en este caso, el maximo
            //dado que esta funcion vale para lectura y escritura, no sabemos realmente cuando escribe ahi
            //entonces, la solucion pasa por indicarle aqui el maximo y listo
            tbsprite_last_visible_sprite=TBBLUE_MAX_SPRITES-1;

        }
    break;

    //hilow rom
    case MEMORY_ZONE_HILOW_ROM:
        if (hilow_enabled.v) {
            p=&hilow_memory_pointer[address];
        }
    break;


    //hilow ram
    case MEMORY_ZONE_HILOW_RAM:
        if (hilow_enabled.v) {
	        //La RAM esta despues de los 8kb de rom
            p=&hilow_memory_pointer[8192+address];
        }
    break;

    //hilow device
    case MEMORY_ZONE_HILOW_DEVICE:
        if (hilow_enabled.v && hilow_rom_traps.v) {
            p=&hilow_device_buffer[address];
        }
    break;

    //hilow buffer intermedio de conversion
    case MEMORY_ZONE_HILOW_CONVERT_READ:
        if (menu_hilow_convert_audio_has_been_opened) {
            p=&hilow_read_audio_buffer_result[address+1]; //Saltar el primer byte del sector
        }
    break;


    //hilow barbanegra rom
    case MEMORY_ZONE_HILOW_BARBANEGRA_ROM:
        if (hilow_bbn_enabled.v) {
            p=&hilow_bbn_memory_pointer[address];
        }
    break;


    //hilow barbanegra ram
    case MEMORY_ZONE_HILOW_BARBANEGRA_RAM:
        if (hilow_bbn_enabled.v) {
	        //La RAM esta despues de los 8kb de rom
            p=&hilow_bbn_memory_pointer[8192+address];
        }
    break;

    //transtape rom
    case MEMORY_ZONE_TRANSTAPE_ROM:
        if (transtape_enabled.v) {
            p=&transtape_memory_pointer[address];
        }
    break;


    //transtape ram
    case MEMORY_ZONE_TRANSTAPE_RAM:
        if (transtape_enabled.v) {
	        //La RAM esta despues de los kb de rom
            p=&transtape_memory_pointer[TRANSTAPE_ROM_SIZE+address];
        }
    break;

    case MEMORY_ZONE_MHPOKEADOR_RAM:
        if (mhpokeador_enabled.v) {
            p=&mhpokeador_memory_pointer[address];
        }
    break;

    //specmate rom
    case MEMORY_ZONE_SPECMATE_ROM:
        if (specmate_enabled.v) {
            p=&specmate_memory_pointer[address];
        }
    break;

    //phoenix rom
    case MEMORY_ZONE_PHOENIX_ROM:
        if (phoenix_enabled.v) {
            p=&phoenix_memory_pointer[address];
        }
    break;

    //defcon rom
    case MEMORY_ZONE_DEFCON_ROM:
        if (defcon_enabled.v) {
            p=&defcon_memory_pointer[address];
        }
    break;

    //ramjet rom
    case MEMORY_ZONE_RAMJET_ROM:
        if (ramjet_enabled.v) {
            p=&ramjet_memory_pointer[address];
        }
    break;

    //interface007 rom
    case MEMORY_ZONE_INTERFACE007_ROM:
        if (interface007_enabled.v) {
            p=&interface007_memory_pointer[address];
        }
    break;

    //dinamid3 rom
    case MEMORY_ZONE_DINAMID3_ROM:
        if (dinamid3_enabled.v) {
            p=&dinamid3_memory_pointer[address];
        }
    break;

    case MEMORY_ZONE_DSK_SECTOR:
        if (dsk_memory_zone_dsk_sector_enabled.v) {
            p=&p3dsk_buffer_disco[dsk_memory_zone_dsk_sector_start+address];
        }
    break;

    case MEMORY_ZONE_LEC_MEMORY:
        if (lec_enabled.v) {
            p=&lec_ram_memory_pointer[address];
        }
    break;

    case MEMORY_ZONE_MDV1:
        if (microdrive_status[0].microdrive_enabled) {
            if (microdrive_status[0].raw_format) {
                p=(z80_byte *) &microdrive_status[0].raw_microdrive_buffer[address];
            }
            else p=&microdrive_status[0].if1_microdrive_buffer[address];
        }
    break;

    case MEMORY_ZONE_MDV2:
        if (microdrive_status[1].microdrive_enabled) {
            if (microdrive_status[1].raw_format) {
                p=(z80_byte *) &microdrive_status[1].raw_microdrive_buffer[address];
            }
            else p=&microdrive_status[1].if1_microdrive_buffer[address];
        }
    break;

    case MEMORY_ZONE_MDV3:
        if (microdrive_status[2].microdrive_enabled) {
            if (microdrive_status[2].raw_format) {
                p=(z80_byte *) &microdrive_status[2].raw_microdrive_buffer[address];
            }
            else p=&microdrive_status[2].if1_microdrive_buffer[address];
        }
    break;

    case MEMORY_ZONE_MDV4:
        if (microdrive_status[3].microdrive_enabled) {
            if (microdrive_status[3].raw_format) {
                p=(z80_byte *) &microdrive_status[3].raw_microdrive_buffer[address];
            }
            else p=&microdrive_status[3].if1_microdrive_buffer[address];
        }
    break;

  }

  return p;

}

subzone_info subzone_info_zxuno[]={
        {0x000000,0x01FFFF,"ZX Spectrum RAM"},
        {0x020000,0x02FFFF,"ZX Spectrum ROM"},
        {0x030000,0x033FFF,"ESXDOS ROM"},
        {0x034000,0x03FFFF,"Unassigned"},
        {0x040000,0x05FFFF,"divMMC RAM"}, //y spectranet flash rom
        {0x060000,0x06FFFF,"Chloe EXT"}, //y spectranet ram
        {0x070000,0x07FFFF,"Chloe DOC"}, //y spectranet ram
  {0,0,""}

};

subzone_info subzone_info_tbblue[]={
        {0x000000,0x00FFFF,"ZX Spectrum ROM"},
        {0x010000,0x013FFF,"ESXDOS ROM"},
        {0x014000,0x017FFF,"Multiface ROM"},
        {0x018000,0x01BFFF,"Multiface extra ROM"},
        {0x01c000,0x01FFFF,"Multiface RAM"},
        {0x020000,0x03FFFF,"divMMC RAM"},
        {0x040000,0x05FFFF,"ZX Spectrum RAM"},
        {0x060000,0x07FFFF,"Extra RAM"},
        {0x080000,0x0FFFFF,"1st Extra IC RAM"},
        {0x100000,0x17FFFF,"2nd Extra IC RAM"},
        {0x180000,0xFFFFFF,"3rd Extra IC RAM"},
  {0,0,""}
/*
    0x000000 – 0x00FFFF (64K) => ZX Spectrum ROM
    0x010000 – 0x013FFF (16K) => ESXDOS ROM
    0x014000 – 0x017FFF (16K) => Multiface ROM

    0x018000 – 0x01BFFF (16K) => Multiface extra ROM
    0x01c000 – 0x01FFFF (16K) => Multiface RAM
    0x020000 – 0x03FFFF (128K) => divMMC RAM
    0x040000 – 0x05FFFF (128K) => ZX Spectrum RAM
    0x060000 – 0x07FFFF (128K) => Extra RAM

    0x080000 – 0x0FFFFF (512K) => 1st Extra IC RAM (if present)
    0x100000 – 0x17FFFF (512K) => 2nd Extra IC RAM (if present)
    0x180000 – 0xFFFFFF (512K) => 3rd Extra IC RAM (if present)
    */
};


subzone_info subzone_info_tsconf_fmaps[]={
        {0x000000,0x0001ff,"CRAM: Color Palette RAM"},
        {0x000200,0x0003ff,"SFILE: Sprite Descriptors"},

/*
TSConf fmaps
#000	000x	CRAM	Color Palette RAM, 256 cells 16 bit wide, 16 bit access
#200	001x	SFILE	Sprite Descriptors, 85 cells 48 bit wide, 16 bit access
#400	0100	REGS	TS-Conf Registers, 8 bit access, adressing is the same as by #nnAF port
*/
  {0,0,""}
};

//Busca la subzona de memoria en la tabla indicada, retorna indice
int machine_seach_memory_subzone_name(subzone_info *tabla,int address)
{
        int i;

        for (i=0;tabla[i].nombre[0]!=0;i++) if (address>=tabla[i].inicio && address<=tabla[i].fin) break;

        return i;
}


subzone_info *machine_get_memory_subzone_array(int zone, int machine_id)
{


  switch (machine_id) {
          case MACHINE_ID_TBBLUE:
                if (zone==0) {
                        return subzone_info_tbblue;

                }
          break;


        case MACHINE_ID_ZXUNO:
                if (zone==0) {
                        return subzone_info_zxuno;

                }
        break;

        case MACHINE_ID_TSCONF:
                if (zone==5) {
                        return subzone_info_tsconf_fmaps;
                }
        break;

  }

        return NULL;

}


//Maximo texto: 32 de longitud
void machine_get_memory_subzone_name(int zone, int machine_id, int address, char *name)
{

  //Por defecto
  strcpy(name,"");

        subzone_info *puntero;
        puntero=machine_get_memory_subzone_array(zone,machine_id);
        if (puntero==NULL) return;

        int indice=machine_seach_memory_subzone_name(puntero,address);
        strcpy(name,puntero[indice].nombre);

}




//Maximo texto longitud: MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT

void machine_get_memory_zone_name(int zone, char *name)
{

  //Por defecto
  strcpy(name,"Unknown zone");

  //Zona 0, ram speccy
  switch (zone) {
    case 0:
		 //123456789012345678901234567890
      strcpy(name,"Machine RAM");


    break;


    case 1:
		 //123456789012345678901234567890
      strcpy(name,"Machine ROM");


    break;

    case 2:
      if (diviface_enabled.v) {
		   //123456789012345678901234567890
        strcpy(name,"Diviface eprom");
      }
    break;

    case 3:
      if (diviface_enabled.v) {
		   //123456789012345678901234567890
        strcpy(name,"Diviface ram");
      }
    break;

    //zxuno flash
    case 4:
      if (MACHINE_IS_ZXUNO) {
		   //123456789012345678901234567890
        strcpy(name,"ZX-Uno flash");
      }
    break;

    case 5:
      if (MACHINE_IS_TSCONF) {
		   //123456789012345678901234567890
        strcpy(name,"TSConf Fmaps");
      }
    break;

    //kartusho
    case 6:
        if (kartusho_enabled.v) {
		           //123456789012345678901234567890
		strcpy(name,"Kartusho rom");
	}
    break;

    //dandanator
    case 7:
        if (dandanator_enabled.v) {
		           //123456789012345678901234567890
                strcpy(name,"Dandanator rom");
        }
    break;


	//superupgrade ram
    case 8:
	if (superupgrade_enabled.v) {
		           //123456789012345678901234567890
		strcpy(name,"Superupgr. ram");
	}
    break;


        //superupgrade rom
    case 9:
        if (superupgrade_enabled.v) {
		           //123456789012345678901234567890
                strcpy(name,"Superupgr. rom");
        }
    break;


    case 10:
    	if (MACHINE_IS_Z88) {
    		strcpy(name,"Full 4 MB");
    	}
    break;

    //Betadisk
    case 11:
	if (betadisk_enabled.v) {
			   //123456789012345678901234567890
		strcpy(name,"Betadisk rom");
	}
    break;


    //Multiface rom
    case 12:
    	if (multiface_enabled.v) {
    			   //123456789012345678901234567890
		strcpy(name,"Multiface rom");
        }
    break;


    //Multiface ram
    case 13:
    	if (multiface_enabled.v) {
    			   //123456789012345678901234567890
		strcpy(name,"Multiface ram");
        }
    break;

    case 14:
        if (MACHINE_IS_TBBLUE) {
          		   //123456789012345678901234567890
		strcpy(name,"TBBlue patterns");
        }
    break;

    //tsconf sprites
    case 15:
      if (MACHINE_IS_TSCONF) {
          		   //123456789012345678901234567890
		strcpy(name,"TSConf sprites");
      }
        break;

	//memory zone by file. 16
	case MEMORY_ZONE_NUM_FILE_ZONE:
		if (memory_zone_by_file_size>0) {
          		   //123456789012345678901234567890
		strcpy(name,"File zone");
		}
	break;

    //tbblue copper. 17
    case MEMORY_ZONE_NUM_TBBLUE_COPPER:
      if (MACHINE_IS_TBBLUE) {
          		   //123456789012345678901234567890
		strcpy(name,"TBBlue copper");
      }
    break;

    case MEMORY_ZONE_NUM_TIMEX_EX:
      if (MACHINE_IS_TIMEX_TS_TC_2068) {
        strcpy(name,"Timex EXROM");
      }

      if (MACHINE_IS_CHLOE_280SE) {
        strcpy(name,"Chloe EX");
      }

    break;

    case MEMORY_ZONE_NUM_TIMEX_DOCK:
      if (MACHINE_IS_TIMEX_TS_TC_2068) {
        strcpy(name,"Timex Dock");
      }
      if (MACHINE_IS_CHLOE_280SE) {
        strcpy(name,"Chloe Dock");
      }
    break;


        case MEMORY_ZONE_NUM_DAAD_CONDACTS:
                if (MACHINE_IS_SPECTRUM) {
                        strcpy(name,"Daad Condacts");
                }

                if (MACHINE_IS_CPC) {
                        strcpy(name,"Daad Condacts");
                }

        break;


        case MEMORY_ZONE_NUM_PAWS_CONDACTS:
                //if (MACHINE_IS_SPECTRUM) {
                        strcpy(name,"Paws Condacts");
                //}


        break;


        case MEMORY_ZONE_DEBUG:
                if (memory_zone_current_size) {
                        strcpy(name,"Debug");
                }
        break;

    //ifrom
    case MEMORY_ZONE_IFROM:
        if (ifrom_enabled.v) {
		           //123456789012345678901234567890
		strcpy(name,"iFrom rom");
	}
    break;


    case MEMORY_ZONE_MSX_VRAM:
        if (MACHINE_IS_MSX) {
               strcpy(name,"MSX VRAM");
        }
    break;


    case MEMORY_ZONE_MSX_ALL_MEM:
        if (MACHINE_IS_MSX) {
                          //123456789012345678901234567890
               strcpy(name,"MSX All Mem");
        }
    break;

   case MEMORY_ZONE_SVI_VRAM:
        if (MACHINE_IS_SVI) {
               strcpy(name,"SVI VRAM");
        }
    break;


    case MEMORY_ZONE_SVI_ALL_MEM:
        if (MACHINE_IS_SVI) {
                          //123456789012345678901234567890
               strcpy(name,"SVI All Mem");
        }
    break;

    case MEMORY_ZONE_COLECO_VRAM:
        if (MACHINE_IS_COLECO) {
               strcpy(name,"Coleco VRAM");
        }
    break;

    case MEMORY_ZONE_SG1000_VRAM:
        if (MACHINE_IS_SG1000) {
               strcpy(name,"SG1000 VRAM");
        }
    break;

    case MEMORY_ZONE_SMS_VRAM:
        if (MACHINE_IS_SMS) {
               strcpy(name,"SMS VRAM");
        }
    break;

    case MEMORY_ZONE_SAMRAM:
        if (samram_enabled.v) {
              strcpy(name,"SamRAM");
        }
    break;

    case MEMORY_ZONE_NUM_TBBLUE_SPRITES:
        if (MACHINE_IS_TBBLUE) {
            strcpy(name,"TBBlue sprites");
        }
    break;

    //hilow rom
    case MEMORY_ZONE_HILOW_ROM:
        if (hilow_enabled.v) {
            strcpy(name,"HiLow ROM");
        }
    break;

    //hilow ram
    case MEMORY_ZONE_HILOW_RAM:
        if (hilow_enabled.v) {
            strcpy(name,"HiLow RAM");
        }
    break;

    //hilow ram
    case MEMORY_ZONE_HILOW_DEVICE:
        if (hilow_enabled.v && hilow_rom_traps.v) {
            strcpy(name,"HiLow Device");
        }
    break;

    //hilow buffer intermedio de conversion
    case MEMORY_ZONE_HILOW_CONVERT_READ:
        if (menu_hilow_convert_audio_has_been_opened) {
            strcpy(name,"HiLow Convert");
        }
    break;


    //hilow barbanegra rom
    case MEMORY_ZONE_HILOW_BARBANEGRA_ROM:
        if (hilow_bbn_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"HiLow Barbanegra ROM");
        }
    break;

    //hilow barbanegra ram
    case MEMORY_ZONE_HILOW_BARBANEGRA_RAM:
        if (hilow_bbn_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"HiLow Barbanegra RAM");
        }
    break;

    //transtape rom
    case MEMORY_ZONE_TRANSTAPE_ROM:
        if (transtape_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Transtape ROM");
        }
    break;

    //transtape ram
    case MEMORY_ZONE_TRANSTAPE_RAM:
        if (transtape_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Transtape RAM");
        }
    break;

    case MEMORY_ZONE_MHPOKEADOR_RAM:
        if (mhpokeador_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"MH Pokeador RAM");
        }
    break;

    //specmate rom
    case MEMORY_ZONE_SPECMATE_ROM:
        if (specmate_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Spec-Mate ROM");
        }
    break;

    //phoenix rom
    case MEMORY_ZONE_PHOENIX_ROM:
        if (phoenix_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Phoenix ROM");
        }
    break;

    //defcon rom
    case MEMORY_ZONE_DEFCON_ROM:
        if (defcon_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Defcon ROM");
        }
    break;

    //ramjet rom
    case MEMORY_ZONE_RAMJET_ROM:
        if (ramjet_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Ramjet ROM");
        }
    break;

    //interface007 rom
    case MEMORY_ZONE_INTERFACE007_ROM:
        if (interface007_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Interface007 ROM");
        }
    break;

    //dinamid3 rom
    case MEMORY_ZONE_DINAMID3_ROM:
        if (dinamid3_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"Dinamid3 ROM");
        }
    break;

    case MEMORY_ZONE_DSK_SECTOR:
        if (dsk_memory_zone_dsk_sector_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"DSK Sector");
        }
    break;

    case MEMORY_ZONE_LEC_MEMORY:
        if (lec_enabled.v) {
                       //123456789012345678901234567890
            strcpy(name,"LEC Memory");
        }
    break;

    case MEMORY_ZONE_MDV1:
        if (microdrive_status[0].microdrive_enabled) {
                       //123456789012345678901234567890
            strcpy(name,"MDV1 Device");
        }
    break;

    case MEMORY_ZONE_MDV2:
        if (microdrive_status[1].microdrive_enabled) {
                       //123456789012345678901234567890
            strcpy(name,"MDV2 Device");
        }
    break;

    case MEMORY_ZONE_MDV3:
        if (microdrive_status[2].microdrive_enabled) {
                       //123456789012345678901234567890
            strcpy(name,"MDV3 Device");
        }
    break;

    case MEMORY_ZONE_MDV4:
        if (microdrive_status[3].microdrive_enabled) {
                       //123456789012345678901234567890
            strcpy(name,"MDV4 Device");
        }
    break;

  }


}



int machine_get_next_available_memory_zone(int zone)
{
  //Dado una zona actual, busca primera disponible. Si llega al final, retorna -1
  //char nombre[1024];
  int readwrite;

  do {
    //printf ("Zone: %d\n",zone);
    if (zone>=MACHINE_MAX_MEMORY_ZONES) return -1;
    unsigned int size=machine_get_memory_zone_attrib(zone,&readwrite);
    if (size>0) return zone;
    zone++;
  } while (1);

}

void util_delete(char *filename)
{
	unlink(filename);
}



//Extraido de http://rosettacode.org/wiki/CRC-32#C
z80_long_int util_crc32_calculation(z80_long_int crc, z80_byte *buf, size_t len)
{
        static z80_long_int table[256];
        static int have_table = 0;
        z80_long_int rem;
        z80_byte octet;
        int i, j;
        z80_byte *p, *q;

        /* This check is not thread safe; there is no mutex. */
        if (have_table == 0) {
                /* Calculate CRC table. */
                for (i = 0; i < 256; i++) {
                        rem = i;  /* remainder from polynomial division */
                        for (j = 0; j < 8; j++) {
                                if (rem & 1) {
                                        rem >>= 1;
                                        rem ^= 0xedb88320;
                                } else
                                        rem >>= 1;
                        }
                        table[i] = rem;
                }
                have_table = 1;
        }

        crc = ~crc;
        q = buf + len;
        for (p = buf; p < q; p++) {
                octet = *p;  /* Cast to unsigned octet. */
                crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
        }
        return ~crc;
}


//Retorna cuantos bits estan a 0 en un byte
int util_return_ceros_byte(z80_byte valor)
{
	int i;
	int ceros=0;
	for (i=0;i<8;i++) {
		if ((valor&1)==0) ceros++;

		valor=valor>>1;
	}

	return ceros;
}


void util_byte_to_binary(z80_byte value,char *texto)
{
	int i;

	for (i=0;i<8;i++) {
		if (value&128) *texto='1';
		else *texto='0';

		texto++;

		value=value<<1;
	}

	*texto=0; //fin cadena
}

void util_save_file(z80_byte *origin, long long int tamanyo_origen, char *destination_file)
{


    FILE *ptr_destination_file;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    if (zvfs_fopen_write(destination_file,&in_fatfs,&ptr_destination_file,&fil)<0) {
        debug_printf (VERBOSE_ERR,"Can not open %s",destination_file);
        return;
    }


    zvfs_fwrite(in_fatfs,origin,tamanyo_origen,ptr_destination_file,&fil);

    zvfs_fclose(in_fatfs,ptr_destination_file,&fil);


}

//Leer hasta numero de bytes concreto
int util_load_file_bytes(z80_byte *taperead,char *filename,int total_leer)
{
    FILE *ptr_tapebrowser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Can not open %s",filename);
        return 0;
    }


    int leidos=zvfs_fread(in_fatfs,taperead,total_leer,ptr_tapebrowser,&fil);


	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading data");
        return 0;
    }

    zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);

    return leidos;
}


void util_copy_file(char *source_file, char *destination_file)
{

	long long int tamanyo_origen=get_file_size(source_file);

	FILE *ptr_source_file;

    //Soporte para FatFS
    FIL fil_source;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs_source;

    //printf("copiar %s a %s. tamanyo=%ld\n",source_file,destination_file,tamanyo_origen);


    if (zvfs_fopen_read(source_file,&in_fatfs_source,&ptr_source_file,&fil_source)<0) {
        debug_printf (VERBOSE_ERR,"Can not open %s",source_file);
        return;
    }




    FILE *ptr_destination_file;


    //Soporte para FatFS
    FIL fil_destination;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs_destination;


    if (zvfs_fopen_write(destination_file,&in_fatfs_destination,&ptr_destination_file,&fil_destination)<0) {
        debug_printf (VERBOSE_ERR,"Can not open %s",destination_file);
        return;
    }



        z80_byte byte_buffer;

        //Leer byte a byte... Si, es poco eficiente
        while (tamanyo_origen) {

            zvfs_fread(in_fatfs_source,&byte_buffer,1,ptr_source_file,&fil_source);

        	//fread(&byte_buffer,1,1,ptr_source_file);


            zvfs_fwrite(in_fatfs_destination,&byte_buffer,1,ptr_destination_file,&fil_destination);

        	//fwrite(&byte_buffer,1,1,ptr_destination_file);
            //if ((tamanyo_origen % 1024)==0) printf("bytes restantes por copiar: %ld\n",tamanyo_origen);

        	tamanyo_origen--;
	}

    zvfs_fclose(in_fatfs_source,ptr_source_file,&fil_source);
    zvfs_fclose(in_fatfs_destination,ptr_destination_file,&fil_destination);




}

int si_existe_editionnamegame(char *nombre_final)
{
    //Dependiendo del que queramos para esa edicion, es un tzx o un tap
    return find_sharedfile(EDITION_NAME_GAME,nombre_final);
}

//Carga el juego designado como edition name game. Devuelve 0 si no encontrado (en caso por ejemplo que el juego
//esté denegada la distribución
int util_load_editionnamegame(void)
{
	char nombre_final[PATH_MAX];
	if (si_existe_editionnamegame(nombre_final)) {
		debug_printf(VERBOSE_INFO,"Loading name edition game %s",EMULATOR_GAME_EDITION);
		strcpy(quickload_file,nombre_final);
		quickfile=quickload_file;
		//Forzar autoload
		z80_bit pre_noautoload;
		pre_noautoload.v=noautoload.v;
		noautoload.v=0;
		quickload(quickload_file);

		noautoload.v=pre_noautoload.v;
		return 1;
	}
	else return 0;
}


int util_extract_mdv(char *mdvname, char *dest_dir)
{
	//echo "./mdvtool /Users/chernandezba/Downloads/psion/ABACUS.MDV export_all /tmp/"

	//qlay mdv must be file size 174930
	if (get_file_size(mdvname)!=174930) {
		debug_printf(VERBOSE_ERR,"I can only open QLAY mdv file format - must be exactly 174930 bytes in size");
		return 1;
	}

	char *argumentos[]={
                "mdvtool","","export_all",""
        };

        argumentos[1]=mdvname;
        argumentos[3]=dest_dir;

	return main_mdvtool(4,argumentos);
}

int util_extract_hdf(char *hdfname, char *dest_dir)
{

	int leidos;

	//unsigned char buffer_lectura[1024];
        //Asignamos bloque de 64kb de lectura
        z80_byte *buffer_lectura;

        buffer_lectura=malloc(65536);
        if (buffer_lectura==NULL) {
                cpu_panic("Unable to assign read buffer");
        }

        //Obtener archivo
        char archivo[PATH_MAX];
        char archivo_sin_extension[PATH_MAX];
        char archivo_destino[PATH_MAX];

        //Solo nombre sin directorio
        util_get_file_no_directory(hdfname,archivo);

        //Solo nombre sin extension
        util_get_file_without_extension(archivo,archivo_sin_extension);

        //De un hdf puede salir un mmc o un ide
        sprintf (archivo_destino,"%s/%s.mmcide",dest_dir,archivo_sin_extension);

        FILE *ptr_inputfile;
        ptr_inputfile=fopen(hdfname,"rb");

        if (ptr_inputfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error opening input file %s",hdfname);
                return 1;
        }

	FILE *ptr_outputfile;
	ptr_outputfile=fopen(archivo_destino,"wb");

        if (ptr_outputfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error opening output file %s",archivo_destino);
                return 1;
        }

        //printf ("Input: %s output: %s\n",hdfname,archivo_destino);

	// Leer offset a datos raw del byte de cabecera:
	//0x09 DOFS WORD Image data offset This is the absolute offset in the HDF file where the actual hard-disk data dump starts.
	//In HDF version 1.1 this is 0x216.

	//Leemos 11 bytes de la cabecera
        fread(buffer_lectura,1,11,ptr_inputfile);

	int offset_raw=buffer_lectura[9]+256*buffer_lectura[10];

	debug_printf (VERBOSE_DEBUG,"HDF Offset to raw data: %d",offset_raw);


	//Ya hemos leido 11 bytes del principio
	int saltar_bytes=offset_raw-11;

	//Saltar esos bytes
	fread(buffer_lectura,1,saltar_bytes,ptr_inputfile);

	//Y vamos leyendo bloques de 1024
	long long int escritos=0;

	do {
	        leidos=fread(buffer_lectura,1,65536,ptr_inputfile);
		if (leidos>0) {
			fwrite(buffer_lectura,1,leidos,ptr_outputfile);
			escritos +=leidos;
			debug_printf (VERBOSE_DEBUG,"Writing temporary data %dKB",escritos/1024);
		}
	} while (leidos>0);

        //Y luego rellenar archivo a siguiente valor valido de archivo .ide
        //Nota: para .mmc ya nos sirve, dado que .mmc necesita que sea multiple de 256 kb
        long long int valid_ide_sizes[]={
		 8*1024*1024,   //0
		 16*1024*1024,
		 32*1024*1024,
		 64*1024*1024,
		 128*1024*1024,
		 256*1024*1024, //5
		 512*1024*1024,
		 1024*1024*1024,  //7. 1 GB
                 2147483648L, //8. 2 GB
        };

        int total_sizes=9;
        //Ver si coincide con alguno exactamente
        int i;
        int coincide=0;
        for (i=0;i<total_sizes && !coincide;i++) {
                if (escritos==valid_ide_sizes[i]) coincide=1;
        }

        if (!coincide) {
                //No coincide. Ver en que tramo esta
                for (i=0;i<total_sizes;i++) {
                        if (escritos<valid_ide_sizes[i]) break;
                }

                if (i==total_sizes) {
                        debug_printf(VERBOSE_ERR,"Final IDE image too big");
                }

                else {
                        long long int final_size=valid_ide_sizes[i];
                        long long int rellenar=final_size-escritos;
                        debug_printf (VERBOSE_DEBUG,"Adding %d KB until normal image size (%d KB)",rellenar/1024,final_size/1024);
                        z80_byte byte_relleno=0xFF;
                        while (rellenar) {
                            fwrite(&byte_relleno,1,1,ptr_outputfile);
                            rellenar--;
                        }
                }
        }


	fclose (ptr_inputfile);

        fclose(ptr_outputfile);

        free(buffer_lectura);

	return 0;




}

//Cambiar en cadena s, caracter orig por dest
void util_string_replace_char(char *s,char orig,char dest)
{
	while (*s) {
		if ( (*s)==orig) *s=dest;
		s++;
	}
}


//Agrega una cadena de texto a otra con salto de linea al final. Retorna longitud texto agregado contando salto de linea
int util_add_string_newline(char *destination,char *text_to_add)
{
	int longitud_texto=strlen(text_to_add)+1; //Agregar salto de linea
	sprintf (destination,"%s\n",text_to_add);
 	return longitud_texto;

}


//Agrega al texto "original" la cadena "string_to_add", comprobando que no se exceda el limite de longitud de original
//limite hay que pasarlo como el total del buffer (incluyendo el 0 final)
//Retorna 1 si no cabe
int util_concat_string(char *original,char *string_to_add,int limite)
{
        int longitud_original=strlen(original);

        int longitud_to_add=strlen(string_to_add);

        int longitud_final=longitud_original+longitud_to_add+1; //el +1 del byte 0 final

        if (longitud_final>limite) {
            //printf("!!!!!!!!!!Overflow util_concat_string: limit %d, asked %d\n",limite,longitud_final);
            return 1;
        }

        char *offset_add;
        offset_add=&original[longitud_original];
        //printf("before strcpy [%s] [%s]\n",original,string_to_add);
        strcpy(offset_add,string_to_add);
        //printf("after strcpy\n");

        return 0;
}

//De una cadena de bytes, lo muestra como hexadecimal, sin espacios. Retorna cadena acabada en 0
//Completa con espacios hasta longitud
void util_binary_to_hex(z80_byte *origen, char *destino, int longitud_max, int longitud)
{
	int i;

	for (i=0;i<longitud_max && i<longitud;i++) {
		sprintf(destino,"%02X",*origen);

		origen++;
		destino+=2;
	}

	for (;i<longitud_max;i++) {
		*destino=' ';
		destino++;
		*destino=' ';
		destino++;
	}

	*destino=0;
}



//De una cadena de bytes, lo muestra como ascii, sin espacios. si hay caracter no imprimible, muestra . . . Retorna cadena acabada en 0
//Completa con espacios
void util_binary_to_ascii(z80_byte *origen, char *destino, int longitud_max, int longitud)
{
	int i;
	z80_byte caracter;

	for (i=0;i<longitud_max && i<longitud;i++) {
		caracter=*origen;
		if (caracter<32 || caracter>126) caracter='.';
		*destino=caracter;

		origen++;
		destino++;
	}

	for (;i<longitud_max;i++) {
                *destino=' ';
        }

        *destino=0;
}

//Imprime numero binario de 32 bits acabado con prefijo "%"
//longitud_max maximo de la longitud de la cadena incluyendo "%"
//No imprimir ceros al inicio
void util_ascii_to_binary(int valor_origen,char *destino,int longitud_max)
{

        //Si es cero tal cual
        if (valor_origen==0 && longitud_max>=2) {
                strcpy(destino,"0%");
                return;
        }

        //Usamos sin signo por si acaso
        unsigned int valor=valor_origen;
        const unsigned int mascara=(unsigned int)2147483648;

        longitud_max--; //hemos de considerar prefijo final

        int nbit;
        int primer_uno=0;

        for (nbit=0;nbit<32 && longitud_max>0;nbit++) {
                //printf ("nbit %d valor %u\n",nbit,valor);
                if (valor & mascara) {
                        //printf ("primer uno en nbit %d valor %d\n",nbit,valor);
                        *destino='1';
                        primer_uno=1;
                }

                else {
                        if (primer_uno) *destino='0';
                }

                if (primer_uno) {
                        destino++;
                        longitud_max--;
                }

                valor=valor<<1;
        }



        *destino='%';
        destino++;

        *destino=0;

}




/*void util_file_save(char *filename,z80_byte *puntero, long int tamanyo);
{
	           FILE *ptr_filesave;
                                  ptr_filesave=fopen(filename,"wb");
                                  if (!ptr_filesave)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open file %s",filename);

                                  }
                                else {

                                        fwrite(puntero,1,tamanyo,ptr_filesave);

                                  fclose(ptr_filesave);


                                }

}*/

void util_file_append(char *filename,z80_byte *puntero, int tamanyo)
{
                   FILE *ptr_filesave;
                                  ptr_filesave=fopen(filename,"ab");
                                  if (!ptr_filesave)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open file %s",filename);

                                  }
                                else {

                                        fwrite(puntero,1,tamanyo,ptr_filesave);

                                  fclose(ptr_filesave);


                                }

}

/*
Funcion usada en compresion de datos
Dado un puntero de entrada, y la longitud del bloque, dice cuantas veces aparecer el primer byte, y que byte es
Retorna: numero de repeticiones (minimo 1) y el byte que es (modifica contenido puntero *byte_repetido)
*/

int util_get_byte_repetitions(z80_byte *memoria,int longitud,z80_byte *byte_repetido)
{
	int repeticiones=1;

	//Primer byte
	z80_byte byte_anterior;
	byte_anterior=*memoria;
	//printf ("byte anterior: %d\n",byte_anterior);

	memoria++;
	longitud--;

	while (longitud>0 && (*memoria)==byte_anterior) {
		//printf ("longitud: %d memoria: %p\n",longitud,memoria);
		repeticiones++;
		memoria++;
		longitud--;
	}

	*byte_repetido=byte_anterior;
	return repeticiones;

}

void util_write_repeated_byte(z80_byte *memoria,z80_byte byte_a_repetir,int longitud)
{
	while (longitud>0) {
		*memoria=byte_a_repetir;

		longitud--;
		memoria++;
	}
}


/*
Funcion de compresion de datos
Dado un puntero de entrada, la longitud del bloque, y el byte "magico" de repeticion (normalmente DD), y el puntero de salida, retorna esos datos comprimidos, mediante:
siendo XX el byte magico, cuando hay un bloque con repeticion (minimo 5 repeticiones) se retorna como:
XX XX YY ZZ, siendo YY el byte a repetir y ZZ el numero de repeticiones (0 veces significa 256 veces)
Si no hay repeticiones, se retornan los bytes tal cual
*casos a considerar:
**si en entrada hay byte XX repetido al menos 2 veces, se trata como repeticion, sin tener que llegar al minimo de 5
**si el bloque de salida ocupa mas que el de entrada (que hubiesen muchos XX XX en la entrada) , el proceso que llama aqui lo debe considerar, para dejar el bloque sin comprimir
**si se repite mas de 256 veces, trocear en varios

Retorna: longitud del bloque destino


Considerar caso DD 00 00 00 00 00 ...  -> genera DD    DD DD 00 5
Habria que ver al generar repeticion, si anterior era DD, hay que convertir el DD anterior en DD DD DD 01
Debe generar:
DD DD DD 01  DD DD 00 05

O sea, tenemos el DD anterior, agregamos DD DD 01

con zxuno.flash es uno de estos casos


Peor de los casos:

A)
DD DD 00  DD DD 00  ....  6 bytes
DD DD DD 02     DD DD DD 02   8 bytes

B)
DD 00 00 00 00 00   DD 00 00 00 00 00  ... 12 bytes
DD DD DD 01  DD DD 00 5    DD DD DD 01  DD DD 00 5   .... 16 bytes

consideramos lo peor que sea el doble

*/

int util_compress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte)
{

	int longitud_destino=0;

	int antes_es_magic_aislado=0; //Si el de antes era un byte magic (normalmente DD) asilado

	while (longitud) {
		z80_byte byte_repetido;
		int repeticiones=util_get_byte_repetitions(origen,longitud,&byte_repetido);
		//printf ("Remaining size: %d Byte: %02X Repetitions: %d\n",longitud,byte_repetido,repeticiones);
		//if (longitud<0) exit(1);

		origen +=repeticiones;
		longitud -=repeticiones;

		//Hay repeticiones
		if (repeticiones>=5
			||
		(byte_repetido==magic_byte && repeticiones>1)
		) {
			//Escribir magic byte dos veces, byte a repetir, y numero repeticiones
			//Si repeticiones > 256, trocear
			while (repeticiones>0) {
				if (antes_es_magic_aislado) {
                    antes_es_magic_aislado=0;

                    //Antes hay un DD. Convertirlo en DD DD DD 01
					destino[0]=magic_byte;
					destino[1]=magic_byte;
					destino[2]=1;

					destino +=3;
					longitud_destino +=3;

                    //printf("antes_es_magic_aislado\n");

                    //printf("--Repeticion byte_a_repetir %d longitud_repeticion %d\n",magic_byte,1);
				}

				destino[0]=magic_byte;
				destino[1]=magic_byte;
				destino[2]=byte_repetido;
				z80_byte brep;
				if (repeticiones>255) brep=0;
				else brep=repeticiones;

				destino[3]=brep;

				//printf ("%d %02X %02X %02X %02X\n",longitud,magic_byte,magic_byte,byte_repetido,brep);

                //printf("--Repeticion byte_a_repetir %d longitud_repeticion %d\n",byte_repetido,(brep==0 ? 256 : brep));

				destino +=4;
				longitud_destino +=4;

				repeticiones -=256;
			}

			//antes_es_magic_aislado=0;
		}

		else {
			//No hay repeticiones
			if (repeticiones==1 && byte_repetido==magic_byte) antes_es_magic_aislado=1;
			else antes_es_magic_aislado=0;


			util_write_repeated_byte(destino,byte_repetido,repeticiones);
			//printf ("%d %02X(%d)\n",longitud,byte_repetido,repeticiones);

			destino +=repeticiones;
			longitud_destino +=repeticiones;



		}
	}

	return longitud_destino;

}



/*
Funcion de descompresion de datos
siendo XX el byte magico, cuando hay un bloque con repeticion (minimo 5 repeticiones) se retorna como:
XX XX YY ZZ, siendo YY el byte a repetir y ZZ el numero de repeticiones (0 veces significa 256 veces)

Retorna: longitud del bloque destino
*/

int util_uncompress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte)
{
        int longitud_destino=0;

        while (longitud) {
            //printf("util_uncompress_data_repetitions. longitud %d longitud_destino %d\n",longitud,longitud_destino);
		//Si primer y segundo byte son el byte magic
		int repeticion=0;
		if (longitud>=4) {
			if (origen[0]==magic_byte && origen[1]==magic_byte) {
				repeticion=1;

				z80_byte byte_a_repetir=origen[2];

				int longitud_repeticion;
				longitud_repeticion=origen[3];

				if (longitud_repeticion==0) longitud_repeticion=256;

                //printf("Read %02X\n",origen[0]);
                //printf("Read %02X\n",origen[1]);
                //printf("Read %02X\n",origen[2]);
                //printf("Read %02X\n",origen[3]);

                //printf("--Repeticion byte_a_repetir %d longitud_repeticion %d\n",byte_a_repetir,longitud_repeticion);

				util_write_repeated_byte(destino,byte_a_repetir,longitud_repeticion);

				origen+=4;
				longitud-=4;

				destino+=longitud_repeticion;
				longitud_destino+=longitud_repeticion;
			}
		}

		if (!repeticion) {
            //printf("Read %02X\n",*origen);
            //printf("Antes de escribir en destino %p (memoria_spectrum: %p)\n",destino,memoria_spectrum);
			*destino=*origen;
            //printf("Despues de escribir en destino\n");

			origen++;
			longitud--;

			destino++;
			longitud_destino++;
		}
	}

	return longitud_destino;
}



//Obtener coordenada x,y de una direccion de pantalla dada
//x entre 0..255 (aunque sera multiple de 8)
//y entre 0..191
void util_spectrumscreen_get_xy(z80_int dir,int *xdest,int *ydest)
{
        //De momento para ir rapido, buscamos direccion en array de scanline
        //screen_addr_table

        //dir -=16384;  //TODO: quiza en vez de hacer una resta, hacer un AND para quitar ese bit. Quiza incluso quitar todos los bits 15,14,13
        //asi quitaria offsets 32768, 16384 y 8192

        dir &=(65535-32768-16384-8192);


        int indice=0;
        int x,y;
        for (y=0;y<192;y++) {
                for (x=0;x<32;x++) {

                        if (dir==screen_addr_table[indice]) {
                                *xdest=x*8;
                                *ydest=y;
                                return;
                        }

                        indice++;
                }
        }

}


//Convierte una pantalla de tipo spectrum (con sus particulares direcciones) a un sprite
void util_convert_scr_sprite(z80_byte *origen,z80_byte *destino)
{
	int x,y;
	z80_byte *linea_origen;

	for (y=0;y<192;y++) {
		int offset_origen=screen_addr_table[y*32] & 8191;
		linea_origen=&origen[offset_origen];
		for (x=0;x<32;x++) {
			*destino=*linea_origen;
			destino++;
			linea_origen++;
		}
	}
}

//Dado una zona de memoria con formato scr, devuelve el color del pixel en posicion x,y
//Utilizar codigo optimizado para que sea lo mas rapido posible
int util_get_pixel_color_scr(z80_byte *scrfile,int x,int y)
{

    //comprobar margenes antes
    if (x<0 || x>255 || y<0 || y>191) return 0;

    int byte_x=x>>3;  // Divide entre 8

    int offset_origen=(screen_addr_table[y*32] & 8191)+byte_x;

    z80_byte byte_leido=scrfile[offset_origen];

    //Sacar el pixel indicado. Evitar bucles para que sea mas rapido
    int pixel_on;
    switch (x & 7) {
        case 0:
            pixel_on=(byte_leido & 128 ? 1 : 0);
        break;

        case 1:
            pixel_on=(byte_leido & 64  ? 1 : 0);
        break;

        case 2:
            pixel_on=(byte_leido & 32  ? 1 : 0);
        break;

        case 3:
            pixel_on=(byte_leido & 16  ? 1 : 0);
        break;

        case 4:
            pixel_on=(byte_leido & 8   ? 1 : 0);
        break;

        case 5:
            pixel_on=(byte_leido & 4   ? 1 : 0);
        break;

        case 6:
            pixel_on=(byte_leido & 2   ? 1 : 0);
        break;

        case 7:
            pixel_on=(byte_leido & 1   ? 1 : 0);
        break;


    }

    //falta atributo

    /*int byte_y=y>>3; //Divide entre 8
    //luego multiplica por 32
    int offset_y=byte_y*32;*/

    //Es lo mismo que multiplicar por 4 quitando los 3 bits inferiores
    int offset_y=(y & (255-7))<<2;

    int offset_attribute=6144+offset_y+byte_x;
    z80_byte atributo=scrfile[offset_attribute];

    //Invertir si parpadeo y estado parpadeo activo
    if (atributo & 128) {
        if (estado_parpadeo.v) pixel_on ^=1;
    }

    int color_pixel=(pixel_on ? atributo &7 : (atributo>>3)&7); //Retornar tinta o papel

    if (atributo & 64) color_pixel +=8; //Sumar brillo

    return color_pixel;


}

//Devolver valor sin signo
int util_get_absolute(int valor)
{
        if (valor<0) valor=-valor;

        return valor;
}


//Devolver signo de valor
int util_get_sign(int valor)
{
	if (valor<0) return -1;

	return +1;
}



//Rutina para extraer TAP pero tambien para convertir a TZX o PZX
//Si tzxfile !=NULL, lo convierte a tzx o pzx, en vez de expandir
//Si tzx_turbo_rg, convierte a tzx turbo de Rodolfo Guerra (4000 bauds)
int util_extract_tap(char *filename,char *tempdir,char *tzxfile,int tzx_turbo_rg)
{


	//tapefile
	if (util_compare_file_extension(filename,"tap")!=0) {
		debug_printf(VERBOSE_ERR,"Tape expander not supported for this tape type");
		return 1;
	}

    int es_pzx=0;

	//Leemos cinta en memoria
	int total_file_size=get_file_size(filename);

	z80_byte *taperead;



        FILE *ptr_tapebrowser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open tape %s for extracting tap",filename);
        return 1;
    }

    /*
        ptr_tapebrowser=fopen(filename,"rb");

        if (!ptr_tapebrowser) {
		debug_printf(VERBOSE_ERR,"Unable to open tape %s for extracting tap",filename);
		return 1;
	}
    */

	taperead=malloc(total_file_size);
	if (taperead==NULL) cpu_panic("Error allocating memory for tape browser");

	//z80_byte *puntero_lectura;
	//puntero_lectura=taperead;
    int nuevo_puntero_lectura=0;

        //Abrir fichero tzxfile si conviene convertir
        FILE *ptr_tzxfile;

                //Soporte para FatFS
                FIL fil_tzxfile;        /* File object */
                //FRESULT fr_tzxfile;     /* FatFs return code */

                int in_fatfs_tzxfile;


        if (tzxfile!=NULL) {



                if (zvfs_fopen_write(tzxfile,&in_fatfs_tzxfile,&ptr_tzxfile,&fil_tzxfile)<0) {
                    debug_printf (VERBOSE_ERR,"Can not open %s",tzxfile);
                    return 1;
                }

                /*
                ptr_tzxfile=fopen(tzxfile,"wb");

                if (!ptr_tzxfile) {
                                debug_printf (VERBOSE_ERR,"Can not open %s",tzxfile);
                                return 1;
                }
                */

               //Ver si archivo es PZX
               if (!util_compare_file_extension(tzxfile,"pzx")) es_pzx=1;


            if (es_pzx) {
                tape_write_pzx_header_ptr(ptr_tzxfile,in_fatfs_tzxfile,&fil_tzxfile);
            }
            else {
                tape_write_tzx_header_ptr(ptr_tzxfile,in_fatfs_tzxfile,&fil_tzxfile);
            }
        }


        int leidos;


         leidos=zvfs_fread(in_fatfs,taperead,total_file_size,ptr_tapebrowser,&fil);


	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
                return 1;
        }

        zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);
        //fclose(ptr_tapebrowser);

	char buffer_texto[32*4]; //4 lineas mas que suficiente

	int longitud_bloque;



	int filenumber=0;

	int previo_longitud_segun_cabecera=-1; //Almacena longitud de bloque justo anterior
	z80_byte previo_flag=255; //Almacena flag de bloque justo anterior
	z80_byte previo_tipo_bloque=255; //Almacena previo tipo bloque anterior (0, program, 3 bytes etc)

    int remaining_file_size=total_file_size;

	while(remaining_file_size>0) {
		//z80_byte *copia_puntero=puntero_lectura;
        int nuevo_copia_puntero=nuevo_puntero_lectura;
        //Buffer temporal para evitar que se salga de sitio
        z80_byte buffer_temp[36]; //36 maximo en una cabecera tipo sped
        util_memcpy_protect_origin(buffer_temp,taperead,total_file_size,nuevo_puntero_lectura,36);
		longitud_bloque=util_tape_tap_get_info(buffer_temp,buffer_texto,1);
                //printf("longitud bloque: %d\n",longitud_bloque);
                //printf("nombre: %s\n",buffer_texto);

                //Eliminar posibles / del nombre
                int i;
                for (i=0;buffer_texto[i];i++) {
                        if (buffer_texto[i]=='/') buffer_texto[i]=' ';
                }

		remaining_file_size-=longitud_bloque;

		nuevo_puntero_lectura +=longitud_bloque;
		//debug_printf (VERBOSE_DEBUG,"Tape browser. Block: %s",buffer_texto);


     //printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);

		char buffer_temp_file[PATH_MAX];
		int longitud_final=longitud_bloque-2-2; //Saltar los dos de cabecera, el de flag y el crc

                if (longitud_final>=0) {
                        z80_byte tipo_bloque=255;

                        //Si bloque de flag 0 y longitud 17 o longitud 34 (sped)
                        z80_byte flag=util_get_byte_protect(taperead,total_file_size,nuevo_copia_puntero+2);

                        //printf ("flag %d previo_flag %d previolong %d longitud_final %d\n",flag,previo_flag,previo_longitud_segun_cabecera,longitud_final);

                        int longitud_segun_cabecera=-1;

                        if (flag==0 && (longitud_final==17 || longitud_final==34) ) {
                                if (tzxfile==NULL) sprintf (buffer_temp_file,"%s/%02d-header-%s",tempdir,filenumber,buffer_texto);

                                tipo_bloque=util_get_byte_protect(taperead,total_file_size,nuevo_copia_puntero+3); //0, program, 3 bytes etc

                                //printf ("%s : tipo %d\n",buffer_temp_file,tipo_bloque);

                                //Longitud segun cabecera
                                longitud_segun_cabecera=value_8_to_16(util_get_byte_protect(taperead,total_file_size,nuevo_copia_puntero+15),
                                util_get_byte_protect(taperead,total_file_size,nuevo_copia_puntero+14));

                        }
                        else {
                                char extension_agregar[10];
                                extension_agregar[0]=0; //Por defecto

                                int era_pantalla=0;

                                //Si bloque de flag 255, ver si corresponde al bloque anterior de flag 0
                                if (flag==255 && previo_flag==0 && previo_longitud_segun_cabecera==longitud_final) {
                                        //Corresponde. Agregar extensiones bas o scr segun el caso
                                        if (previo_tipo_bloque==0) {
                                                //Basic
                                                strcpy(extension_agregar,".bas");
                                        }


                                }

                                //Consideramos que es pantalla siempre que tenga longitud 6912,
                                //sin tener en cuenta que haya cabecera antes o no
                                if (longitud_final==6912) {
                                    //archivo a expandir es pantalla scr
                                    strcpy(extension_agregar,".scr");
                                    //y ademas decimos que ese es el achivo de pantalla usado en los previews
                                    era_pantalla=1;
                                }



                                if (tzxfile==NULL) {
                                        sprintf (buffer_temp_file,"%s/%02d-data-%d%s",tempdir,filenumber,longitud_final,extension_agregar);

                                        if (era_pantalla) {
                                                //Indicar con un archivo en la propia carpeta cual es el archivo de pantalla
                                                //usado en los previews
                                                char buff_preview_scr[PATH_MAX];
                                                sprintf(buff_preview_scr,"%s/%s",tempdir,MENU_SCR_INFO_FILE_NAME);

                                                //Meter en archivo MENU_SCR_INFO_FILE_NAME la ruta al archivo de pantalla
                                                util_save_file((z80_byte *)buffer_temp_file,strlen(buffer_temp_file)+1,buff_preview_scr);
                                        }
                                }
                        }


                        if (tzxfile==NULL) {
                                //Generar bloque con datos, saltando los dos de cabecera y el flag
                                //Generamos bloque temporal para evitar segfaults en archivos corruptos


                                z80_byte *temp_save_mem;

                                temp_save_mem=malloc(longitud_final);
                                //printf("\nAsignando %d bytes\n",longitud_final);
                                if (temp_save_mem==NULL) cpu_panic("Can not allocate memory for file save");

                                //printf("offset %d total_memoria %d longitud_bloque %d\n",nuevo_copia_puntero+3,total_file_size,longitud_final);
                                //if (nuevo_copia_puntero+3+longitud_final>total_file_size) printf("POSIBLE ERROR\n");

                                //printf("util_memcpy_protect_origin destino %p origen %p total_mem %d offset %d total_copiar %d\n",
                                //       temp_save_mem,taperead,total_file_size,nuevo_copia_puntero+3,longitud_final);

                                util_memcpy_protect_origin(temp_save_mem,taperead,total_file_size,nuevo_copia_puntero+3,longitud_final);

                                util_save_file(temp_save_mem,longitud_final,buffer_temp_file);

                                free(temp_save_mem);


                        }

                        //Convertir a tzx o pzx
                        else {

                            if (es_pzx) {



                                //Generamos bloque temporal para evitar segfaults en archivos corruptos
                                z80_byte *temp_save_mem=malloc(longitud_bloque);
                                if (temp_save_mem==NULL) cpu_panic("Can not allocate memory for file save");
                                util_memcpy_protect_origin(temp_save_mem,taperead,total_file_size,nuevo_copia_puntero,longitud_bloque);


                                z80_byte flag_bloque=temp_save_mem[2];

                                int longitud_bloque_pzx=longitud_bloque-2; //saltar los 2 bytes iniciales que dicen longitud
                                tape_block_pzx_begin_save_ptr(ptr_tzxfile,longitud_bloque_pzx,flag_bloque,in_fatfs_tzxfile,&fil_tzxfile);

                                //int longitud_bloque_pzx=longitud_bloque-2; //saltar los 2 bytes iniciales que dicen longitud

                                zvfs_fwrite(in_fatfs_tzxfile,&temp_save_mem[2],longitud_bloque_pzx,ptr_tzxfile,&fil_tzxfile);


                                free(temp_save_mem);
                            }

                            else {


                                if (tzx_turbo_rg) {
                                    //Bloque turbo de 4000 bauds para usar con roms de Rodolfo Guerra
                                    z80_byte buffer_tzx[19];
                                    buffer_tzx[0]=0x11;

                                    buffer_tzx[1]=value_16_to_8l(1408);
                                    buffer_tzx[2]=value_16_to_8h(1408);

                                    buffer_tzx[3]=value_16_to_8l(397);
                                    buffer_tzx[4]=value_16_to_8h(397);

                                    buffer_tzx[5]=value_16_to_8l(317);
                                    buffer_tzx[6]=value_16_to_8h(317);

                                    buffer_tzx[7]=value_16_to_8l(325);
                                    buffer_tzx[8]=value_16_to_8h(325);

                                    buffer_tzx[9]=value_16_to_8l(649);
                                    buffer_tzx[10]=value_16_to_8h(649);

                                    buffer_tzx[11]=value_16_to_8l(4835);
                                    buffer_tzx[12]=value_16_to_8h(4835);

                                    buffer_tzx[13]=8;

                                    buffer_tzx[14]=value_16_to_8l(318);
                                    buffer_tzx[15]=value_16_to_8h(318);

                                    //Length of data that follow. 3 bytes
                                    buffer_tzx[16]=value_16_to_8l(longitud_bloque-2); //Sin contar propiamente los 2 bytes iniciales del bloque tap
                                    buffer_tzx[17]=value_16_to_8h(longitud_bloque-2); //Sin contar propiamente los 2 bytes iniciales del bloque tap
                                    buffer_tzx[18]=0; //Valor de 24 bits. Byte mas alto a 0

                                    zvfs_fwrite(in_fatfs_tzxfile,buffer_tzx,19,ptr_tzxfile,&fil_tzxfile);

                                    //Generamos bloque temporal para evitar segfaults en archivos corruptos
                                    z80_byte *temp_save_mem=malloc(longitud_bloque-2);
                                    if (temp_save_mem==NULL) cpu_panic("Can not allocate memory for file save");
                                    util_memcpy_protect_origin(temp_save_mem,&taperead[2],total_file_size,nuevo_copia_puntero,longitud_bloque-2);


                                    zvfs_fwrite(in_fatfs_tzxfile,temp_save_mem,longitud_bloque-2,ptr_tzxfile,&fil_tzxfile);


                                    free(temp_save_mem);
                                }

                                else {
                                    //Generar bloque con datos. TZX ID 10h, word pausa de 500ms, longitud
                                    z80_byte buffer_tzx[5];
                                    buffer_tzx[0]=0x10;
                                    buffer_tzx[1]=244;
                                    buffer_tzx[2]=1;

                                    zvfs_fwrite(in_fatfs_tzxfile,buffer_tzx,3,ptr_tzxfile,&fil_tzxfile);



                                    //Meter datos tal cual de tap: longitud, flag, datos, crc

                                    //Generar bloque con datos, saltando los dos de cabecera y el flag


                                    //Generamos bloque temporal para evitar segfaults en archivos corruptos
                                    z80_byte *temp_save_mem=malloc(longitud_bloque);
                                    if (temp_save_mem==NULL) cpu_panic("Can not allocate memory for file save");
                                    util_memcpy_protect_origin(temp_save_mem,taperead,total_file_size,nuevo_copia_puntero,longitud_bloque);


                                    zvfs_fwrite(in_fatfs_tzxfile,temp_save_mem,longitud_bloque,ptr_tzxfile,&fil_tzxfile);


                                    free(temp_save_mem);

                                }

                            }


                        }

                        filenumber++;

                        previo_flag=flag;
                        previo_longitud_segun_cabecera=longitud_segun_cabecera;
                        previo_tipo_bloque=tipo_bloque;

                }
	}


	free(taperead);

        if (tzxfile!=NULL) {
            zvfs_fclose(in_fatfs_tzxfile,ptr_tzxfile,&fil_tzxfile);
            //fclose(ptr_tzxfile);
        }

	return 0;

}


//Rutina para extraer MDR
int util_extract_mdr(char *filename,char *tempdir)
{


	if (util_compare_file_extension(filename,"mdr")!=0) {
		debug_printf(VERBOSE_ERR,"MDR expander not supported for this microdrive type");
		return 1;
	}

	//Leemos cinta en memoria
	int total_file_size=get_file_size(filename);

	z80_byte *taperead;



    FILE *ptr_tapebrowser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open tape %s for extracting mdr",filename);
        return 1;
    }


	taperead=util_malloc(total_file_size,"Can not allocate memory for expand mdr");


    int leidos;


    leidos=zvfs_fread(in_fatfs,taperead,total_file_size,ptr_tapebrowser,&fil);


	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
        return 1;
    }

    zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);


   //int menu_microdrive_map_browse(zxvision_window *ventana,int tipo,int microdrive_seleccionado,int y_ventana_inicial,
//z80_byte (*f_get_byte)(int microdrive_seleccionado,int sector,int sector_offset),
//int total_sectors

    int total_sectors=leidos/MDR_BYTES_PER_SECTOR;
    //printf("total sectors: %d\n",total_sectors);

    struct s_mdr_file_cat *catalogo;

    //printf("Before catalogo\n");

    catalogo=mdr_get_file_catalogue(taperead,total_sectors);

    //printf("After catalogo\n");

    int i;

    for (i=0;i<catalogo->total_files;i++) {
        //printf("%d [%s]\n",i,catalogo->file[i].name);

        //mdr_get_file_from_catalogue(taperead,total_sectors,nombre,tamanyo,destino,&frag,&nofrag);

        int tamanyo=catalogo->file[i].file_size;


        //printf("tamanyo: %d\n",tamanyo);



        //buffer para archivo de testino
        z80_byte *destino=util_malloc(tamanyo,"Can not allocate memory to get file");


        mdr_get_file_from_catalogue(taperead,&catalogo->file[i],destino,tamanyo,total_sectors);



        char buffer_temp_file[PATH_MAX];

        char nombre_escapado[11];

        mdr_get_file_name_escaped(catalogo->file[i].name,nombre_escapado);

        //Si es program, agregar extension .bas
        if (catalogo->file[i].header_info[0]==0) {
            sprintf (buffer_temp_file,"%s/%s.bas",tempdir,nombre_escapado);
        }

        else sprintf (buffer_temp_file,"%s/%s",tempdir,nombre_escapado);

        util_save_file(destino,tamanyo,buffer_temp_file);


        //Si longitud era 6912, indicar la pantalla para los previews
        if (catalogo->file[i].file_size==6912) {
            //Indicar con un archivo en la propia carpeta cual es el archivo de pantalla
            //usado en los previews
            char buff_preview_scr[PATH_MAX];
            sprintf(buff_preview_scr,"%s/%s",tempdir,MENU_SCR_INFO_FILE_NAME);

            //Meter en archivo MENU_SCR_INFO_FILE_NAME la ruta al archivo de pantalla
            util_save_file((z80_byte *)buffer_temp_file,strlen(buffer_temp_file)+1,buff_preview_scr);
        }

        free(destino);

    }

    free(catalogo);

	free(taperead);

	return 0;

}

//Rutina para extraer RMD. Pasamos a MDR y luego llamar a util_extract_mdr
int util_extract_rmd(char *filename,char *tempdir)
{
    char tmpfile[PATH_MAX];

    char buf_file_no_dir[PATH_MAX];
    util_get_file_no_directory(filename,buf_file_no_dir);

    //Agregamos extension temp para evitar posible coincidencia con un directorio igual
    //(generado de un preview al abrir este archivo .rmd.mdr)
    //Ejemplo: expandir archivo bug.rmd.mdr genera carpeta bug.rmd.mdr
    //Pero si expandieramos archivo bug.rmd generaria archivo temporal bug.rmd.mdr, de mismo nombre que la carpeta anterior,
    //por eso el archivo temporal sera bug.rmd.temp.mdr

    sprintf (tmpfile,"%s/%s.temp.mdr",get_tmpdir_base(),buf_file_no_dir);

    convert_rmd_to_mdr(filename,tmpfile);

    return util_extract_mdr(tmpfile,tempdir);
}

//Rutina para extraer DDH
int util_extract_ddh(char *filename,char *tempdir)
{

	if (util_compare_file_extension(filename,"ddh")!=0) {
		debug_printf(VERBOSE_ERR,"DDH expander not supported for this image type");
		return 1;
	}

	//Leemos cinta en memoria
	int total_file_size=HILOW_DEVICE_SIZE;

	z80_byte *taperead;



    FILE *ptr_tapebrowser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open tape %s for extracting tap",filename);
        return 1;
    }


	taperead=util_malloc(total_file_size,"Can not allocate memory for expand ddh");


    int leidos;


    leidos=zvfs_fread(in_fatfs,taperead,total_file_size,ptr_tapebrowser,&fil);


	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
        return 1;
    }

    zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);


	char buffer_texto[32*4]; //4 lineas mas que suficiente

	//int longitud_bloque;


    int id_archivo;

    //Ver que sector usamos, si el 0 o el 1
    int sector;
    z80_int usage_counter_zero=hilow_util_get_usage_counter(1,taperead);
    z80_int usage_counter_one=hilow_util_get_usage_counter(2,taperead);

    if (usage_counter_zero>usage_counter_one) sector=1;
    else sector=2;

    int total_archivos=hilow_util_get_total_files(sector,taperead);

    if (total_archivos>HILOW_MAX_FILES_DIRECTORY) total_archivos=HILOW_MAX_FILES_DIRECTORY;

    for (id_archivo=0;id_archivo<total_archivos;id_archivo++) {


        hilow_util_get_file_name(sector,taperead,id_archivo,buffer_texto);


        z80_int longitud=hilow_util_get_file_length(sector,taperead,id_archivo);

        //printf("nombre: %s longitud: %d\n",buffer_texto,longitud);

        z80_byte *mem_archivo;

        mem_archivo=util_malloc(longitud,"Can not allocate memory for expand ddh");

        hilow_util_get_file_contents(sector,taperead,id_archivo,mem_archivo);

        //Si es program, agregar extension .bas
        z80_byte tipo_archivo=hilow_util_get_file_type(sector,taperead,id_archivo);

        char buffer_temp_file[PATH_MAX];

        if (tipo_archivo==0) {
            sprintf (buffer_temp_file,"%s/%s.bas",tempdir,buffer_texto);
        }
        else sprintf (buffer_temp_file,"%s/%s",tempdir,buffer_texto);


        util_save_file(mem_archivo,longitud,buffer_temp_file);

        //Si longitud era 6912, indicar la pantalla para los previews
        if (longitud==6912) {
            //Indicar con un archivo en la propia carpeta cual es el archivo de pantalla
            //usado en los previews
            char buff_preview_scr[PATH_MAX];
            sprintf(buff_preview_scr,"%s/%s",tempdir,MENU_SCR_INFO_FILE_NAME);

            //Meter en archivo MENU_SCR_INFO_FILE_NAME la ruta al archivo de pantalla
            util_save_file((z80_byte *)buffer_temp_file,strlen(buffer_temp_file)+1,buff_preview_scr);
        }

        free(mem_archivo);
    }


	free(taperead);

	return 0;

}


//Rutina para extraer TZX pero tambien para convertir a TAP
//Si tapfile !=NULL, lo convierte a tap, en vez de expandir
int util_extract_tzx(char *filename,char *tempdirectory,char *tapfile)
{


	//tapefile
	if (
        util_compare_file_extension(filename,"tzx")!=0 &&
        util_compare_file_extension(filename,"cdt")!=0

    ) {
		debug_printf(VERBOSE_ERR,"Tape expander not supported for this tape type");
		return 1;
	}

	//Leemos cinta en memoria
	int total_mem=get_file_size(filename);

	z80_byte *taperead;



        FILE *ptr_tapebrowser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;



    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open tape for extracting tzx");
        return 1;
    }

    /*
        ptr_tapebrowser=fopen(filename,"rb");

        if (!ptr_tapebrowser) {
		debug_printf(VERBOSE_ERR,"Unable to open tape for extracting tzx");
		return 1;
	}
    */

	taperead=malloc(total_mem);
	if (taperead==NULL) cpu_panic("Error allocating memory for tape browser/convert");

	z80_byte *puntero_lectura;
	puntero_lectura=taperead;

        //Abrir fichero tapfile si conviene convertir
        FILE *ptr_tapfile;

    //Soporte para FatFS
    FIL fil_tapfile;        /* File object */
    //FRESULT fr_tapfile;     /* FatFs return code */

    int in_fatfs_tapfile;




        if (tapfile!=NULL) {



                if (zvfs_fopen_write(tapfile,&in_fatfs_tapfile,&ptr_tapfile,&fil_tapfile)<0) {
                    debug_printf (VERBOSE_ERR,"Can not open %s",tapfile);
                    return 1;
                }

                /*
                ptr_tapfile=fopen(tapfile,"wb");

                if (!ptr_tapfile) {
                                debug_printf (VERBOSE_ERR,"Can not open %s",tapfile);
                                return 1;
                }
                */
        }


        int leidos;

        leidos=zvfs_fread(in_fatfs,taperead,total_mem,ptr_tapebrowser,&fil);

        //leidos=fread(taperead,1,total_mem,ptr_tapebrowser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
                return 1;
        }

        zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);
        //fclose(ptr_tapebrowser);

	char buffer_texto[32*4]; //4 lineas mas que suficiente

	int longitud_bloque;



	int filenumber=0;

	int previo_longitud_segun_cabecera=-1; //Almacena longitud de bloque justo anterior
	z80_byte previo_flag=255; //Almacena flag de bloque justo anterior
	z80_byte previo_tipo_bloque=255; //Almacena previo tipo bloque anterior (0, program, 3 bytes etc)

	puntero_lectura +=10; //Saltar cabecera (version tzx etc)

	int salir=0;

		z80_byte *copia_puntero;

	while(total_mem>0 && !salir) {

		z80_byte tzx_id=*puntero_lectura;

		puntero_lectura++;

		switch (tzx_id) {

		case 0x10:
                case 0x11:

                //ID 10 - Standard Speed Data Block
                //ID 11 - Turbo Speed Data Block

                if (tzx_id==0x11) {
                        copia_puntero=puntero_lectura+16;
                        //copia_puntero hay que dejarlo justo donde quedan 2 bytes de la longitud y luego el siguiente es el flag
                        //realmente los de longitud no se leeran luego, el flag si. Esto es por compatibilidad con la gestion del bloque tipo 0x10

                        longitud_bloque=puntero_lectura[15]+256*puntero_lectura[16]+65536*puntero_lectura[17];
                        puntero_lectura+=18;

                        //bloque simulado para poder obtener nombre del bloque
                        //39 bytes: 2 de longitud, 1 de flag, y 36 de maximo cabecera tipo sped
                        z80_byte buffer_temp_cabecera[39];

                        //2 primeros bytes de longitud
                        buffer_temp_cabecera[0]=longitud_bloque % 0xFF;
                        buffer_temp_cabecera[1]=(longitud_bloque % 0xFF00)>>8;
                        util_memcpy_protect_origin(&buffer_temp_cabecera[2], puntero_lectura, 37, 0, 37);

                        util_tape_tap_get_info(buffer_temp_cabecera,buffer_texto,0);

                }

                else {
		        puntero_lectura +=2;
		        total_mem -=2;
                        copia_puntero=puntero_lectura;
                        longitud_bloque=util_tape_tap_get_info(puntero_lectura,buffer_texto,0);
                }


		total_mem-=longitud_bloque;
		puntero_lectura +=longitud_bloque;
		//debug_printf (VERBOSE_DEBUG,"Tape browser. Block: %s",buffer_texto);


     //printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);

		char buffer_temp_file[PATH_MAX];
		int longitud_final;

                if (tzx_id==0x11) longitud_final=longitud_bloque-2;

                else longitud_final=longitud_bloque-2-2; //Saltar los dos de cabecera, el de flag y el crc

		z80_byte tipo_bloque=255;

		//Si bloque de flag 0 y longitud 17 o longitud 34 (sped)
		z80_byte flag=copia_puntero[2];

		//printf ("flag %d previo_flag %d previolong %d longitud_final %d\n",flag,previo_flag,previo_longitud_segun_cabecera,longitud_final);

		int longitud_segun_cabecera=-1;

		if (flag==0 && (longitud_final==17 || longitud_final==34) ) {
			if (tapfile==NULL) sprintf (buffer_temp_file,"%s/%02d-header-%s",tempdirectory,filenumber,buffer_texto);

			tipo_bloque=copia_puntero[3]; //0, program, 3 bytes etc

			//printf ("%s : tipo %d\n",buffer_temp_file,tipo_bloque);

			//Longitud segun cabecera
			longitud_segun_cabecera=value_8_to_16(copia_puntero[15],copia_puntero[14]);

		}
		else {
			char extension_agregar[10];
			extension_agregar[0]=0; //Por defecto

                        int era_pantalla=0;

			//Si bloque de flag 255, ver si corresponde al bloque anterior de flag 0
			if (flag==255 && previo_flag==0 && previo_longitud_segun_cabecera==longitud_final) {
				//Corresponde. Agregar extensiones bas o scr segun el caso
				if (previo_tipo_bloque==0) {
					//Basic
					strcpy(extension_agregar,".bas");
				}


			}

            //Consideramos que es pantalla siempre que tenga longitud 6912,
            //sin tener en cuenta que haya cabecera antes o no
            if (longitud_final==6912) {
                //archivo a expandir es pantalla scr
                strcpy(extension_agregar,".scr");
                //y ademas decimos que ese es el achivo de pantalla usado en los previews
                era_pantalla=1;
            }


			if (tapfile==NULL) {
                                sprintf (buffer_temp_file,"%s/%02d-data-%d%s",tempdirectory,filenumber,longitud_final,extension_agregar);

                                if (era_pantalla) {
                                        //Indicar con un archivo en la propia carpeta cual es el archivo de pantalla
                                        //usado en los previews
                                        char buff_preview_scr[PATH_MAX];
                                        sprintf(buff_preview_scr,"%s/%s",tempdirectory,MENU_SCR_INFO_FILE_NAME);

                                        //Meter en archivo MENU_SCR_INFO_FILE_NAME la ruta al archivo de pantalla
                                        util_save_file((z80_byte *)buffer_temp_file,strlen(buffer_temp_file)+1,buff_preview_scr);
                                }
                        }
		}


                //Si expandir
                if (tapfile==NULL) {
		        //Generar bloque con datos, saltando los dos de cabecera y el flag
		        util_save_file(copia_puntero+3,longitud_final,buffer_temp_file);
                }

                //Convertir a tap
                else {
                        //Generar bloque con datos
                        //Meter longitud, flag
                        z80_byte buffer_tap[3];
                        z80_int longitud_cabecera_tap=longitud_final+2;
                        buffer_tap[0]=value_16_to_8l(longitud_cabecera_tap);
                        buffer_tap[1]=value_16_to_8h(longitud_cabecera_tap);
                        buffer_tap[2]=flag;



                        zvfs_fwrite(in_fatfs_tapfile,buffer_tap,3,ptr_tapfile,&fil_tapfile);

                        //fwrite(buffer_tap,1,3,ptr_tapfile);


                        //Meter datos
                        zvfs_fwrite(in_fatfs_tapfile,copia_puntero+3,longitud_final,ptr_tapfile,&fil_tapfile);
                        //fwrite(copia_puntero+3,1,longitud_final,ptr_tapfile);

                        //Agregar CRC
                        z80_byte byte_crc=*(copia_puntero+3+longitud_final);

                        buffer_tap[0]=byte_crc;

                        zvfs_fwrite(in_fatfs_tapfile,buffer_tap,1,ptr_tapfile,&fil_tapfile);
                        //fwrite(buffer_tap,1,1,ptr_tapfile);

                }

		filenumber++;

		previo_flag=flag;
		previo_longitud_segun_cabecera=longitud_segun_cabecera;
		previo_tipo_bloque=tipo_bloque;

		break;


		case 0x20:


                                //sprintf(buffer_texto,"ID 20 - Pause");
                                //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


                                puntero_lectura+=2;
				total_mem -=2;



                        break;

                        case 0x30:
                                //sprintf(buffer_texto,"ID 30 Text description:");
                                //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                longitud_bloque=*puntero_lectura;
                                //printf ("puntero: %d longitud: %d\n",puntero,longitud_bloque);
                                //util_binary_to_ascii(&tzx_file_mem[puntero+1],buffer_bloque,longitud_bloque,longitud_bloque);
                                //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

                                puntero_lectura+=1;
                                puntero_lectura+=longitud_bloque;
                                //printf ("puntero: %d\n",puntero);

				total_mem -=(longitud_bloque+1);
                        break;

                        case 0x31:
                                //sprintf(buffer_texto,"ID 31 Message block:");
                                //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                longitud_bloque=puntero_lectura[1];
                                //util_binary_to_ascii(&tzx_file_mem[puntero+2],buffer_bloque,longitud_bloque,longitud_bloque);
                                //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

                                puntero_lectura+=2;
                                puntero_lectura+=longitud_bloque;

				total_mem -=(longitud_bloque+2);
                        break;

                        case 0x32:
                                //sprintf(buffer_texto,"ID 32 Archive info:");
                                //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                longitud_bloque=puntero_lectura[0]+256*puntero_lectura[1];
                                puntero_lectura+=2;


                                z80_byte nstrings=*puntero_lectura;
				puntero_lectura++;

				total_mem -=3;

				//char buffer_text_id[256];
                                //char buffer_text_text[256];

                                for (;nstrings;nstrings--) {
                                        //z80_byte text_type=*puntero_lectura;
					puntero_lectura++;
                                        z80_byte longitud_texto=*puntero_lectura;
					puntero_lectura++;

					total_mem -=2;

                                        //tape_tzx_get_archive_info(text_type,buffer_text_id);
                                        //util_binary_to_ascii(&tzx_file_mem[puntero],buffer_text_text,longitud_texto,longitud_texto);
                                        //sprintf (buffer_texto,"%s: %s",buffer_text_id,buffer_text_text);

                                        //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                        puntero_lectura +=longitud_texto;

					total_mem -=longitud_texto;
                                }

                        break;

		default:
                        debug_printf(VERBOSE_DEBUG,"Unhandled TZX id %02XH. Stopping",tzx_id);
			salir=1;
		break;

	   }
	}


	free(taperead);

        if (tapfile!=NULL) {
            zvfs_fclose(in_fatfs_tapfile,ptr_tapfile,&fil_tapfile);
            //fclose(ptr_tapfile);
        }

	return 0;

}



//Rutina para extraer PZX pero tambien para convertir a TAP
//Si tapfile !=NULL, lo convierte a tap, en vez de expandir
int util_extract_pzx(char *filename,char *tempdirectory,char *tapfile)
{


	//tapefile
	if (util_compare_file_extension(filename,"pzx")!=0) {
		debug_printf(VERBOSE_ERR,"Tape expander not supported for this tape type");
		return 1;
	}

	//Leemos cinta en memoria
	int total_file_size=get_file_size(filename);

	z80_byte *taperead;



    FILE *ptr_tapebrowser;

    //Soporte para FatFS
    FIL fil;        /* File object */

    int in_fatfs;

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open tape for extracting pzx");
        return 1;
    }



	taperead=malloc(total_file_size);
	if (taperead==NULL) cpu_panic("Error allocating memory for tape browser/convert");

	//z80_byte *puntero_lectura;

	//puntero_lectura=taperead;
    int puntero_lectura=0;

    //Abrir fichero tapfile si conviene convertir
    FILE *ptr_tapfile;

    //Soporte para FatFS
    FIL fil_tapfile;        /* File object */

    int in_fatfs_tapfile;



    if (tapfile!=NULL) {

            if (zvfs_fopen_write(tapfile,&in_fatfs_tapfile,&ptr_tapfile,&fil_tapfile)<0) {
                debug_printf (VERBOSE_ERR,"Can not open %s",tapfile);
                return 1;
            }

    }


    int leidos;

    leidos=zvfs_fread(in_fatfs,taperead,total_file_size,ptr_tapebrowser,&fil);
    //leidos=fread(taperead,1,total_mem,ptr_tapebrowser);

	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
        return 1;
    }

    zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);
    //fclose(ptr_tapebrowser);

	char buffer_texto[32*4]; //4 lineas mas que suficiente

	int longitud_bloque;



	int filenumber=0;

	int previo_longitud_segun_cabecera=-1; //Almacena longitud de bloque justo anterior
	z80_byte previo_flag=255; //Almacena flag de bloque justo anterior
	z80_byte previo_tipo_bloque=255; //Almacena previo tipo bloque anterior (0, program, 3 bytes etc)



	int salir=0;

    //z80_byte *copia_puntero;
    int copia_puntero;

    int remaining_file_size=total_file_size;

	while(remaining_file_size>0 && !salir) {

        //Comprobaciones para pzx corruptos
        //total_file_size,remaining_file_size,puntero_lectura)

        if (puntero_lectura<0 || puntero_lectura>=total_file_size) {
            debug_printf(VERBOSE_DEBUG,"Finishing extracting pzx file as it seems corrupted");
            //printf("total_size %d remaining_file_size %d puntero_lectura %d\n",total_file_size,remaining_file_size,puntero_lectura);
            salir=1;
        }

        else {

            char tag_name[5];
            tag_name[0]=util_get_byte_protect(taperead,total_file_size,puntero_lectura+0);
            tag_name[1]=util_get_byte_protect(taperead,total_file_size,puntero_lectura+1);
            tag_name[2]=util_get_byte_protect(taperead,total_file_size,puntero_lectura+2);
            tag_name[3]=util_get_byte_protect(taperead,total_file_size,puntero_lectura+3);
            tag_name[4]=0;

            puntero_lectura +=4;

            //printf("PZX TAG: [%s]\n",tag_name);


            z80_long_int block_size;



            block_size=util_get_byte_protect(taperead,total_file_size,puntero_lectura+0)+
                        (util_get_byte_protect(taperead,total_file_size,puntero_lectura+1)*256)+
                        (util_get_byte_protect(taperead,total_file_size,puntero_lectura+2)*65536)+
                        (util_get_byte_protect(taperead,total_file_size,puntero_lectura+3)*16777216);
            puntero_lectura +=4;

            remaining_file_size -=8;


            //Tratar cada tag

            if (!strcmp(tag_name,"DATA")) {
                        //convert_pzx_to_rwa_tag_data(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);

                //z80_byte *memoria;
                //memoria=&taperead[puntero_lectura];

                int memoria_lectura=puntero_lectura;

                //int initial_pulse;

                z80_long_int count;

                //int t_estado_actual=*p_t_estado_actual;


                count=util_get_byte_protect(taperead,total_file_size,memoria_lectura+0)+
                        (util_get_byte_protect(taperead,total_file_size,memoria_lectura+1)*256)+
                        (util_get_byte_protect(taperead,total_file_size,memoria_lectura+2)*65536)+
                        ((util_get_byte_protect(taperead,total_file_size,memoria_lectura+3)&127)*16777216);

                //initial_pulse=(util_get_byte_protect(taperead,total_file_size,memoria_lectura+3)&128)>>7;

                memoria_lectura +=4;

                //z80_int tail=memoria[0]+(memoria[1]*256);

                memoria_lectura +=2;

                z80_byte num_pulses_zero=util_get_byte_protect(taperead,total_file_size,memoria_lectura);
                memoria_lectura++;

                z80_byte num_pulses_one=util_get_byte_protect(taperead,total_file_size,memoria_lectura);
                memoria_lectura++;

                //Secuencias que identifican a un cero y un uno
                //z80_int seq_pulses_zero[256];
                //z80_int seq_pulses_one[256];

                //Metemos las secuencias de 0 y 1 en array
                int i;
                for (i=0;i<num_pulses_zero;i++) {
                    //seq_pulses_zero[i]=util_get_byte_protect(taperead,total_file_size,memoria_lectura+0)+
                    //    (util_get_byte_protect(taperead,total_file_size,memoria_lectura+1)*256);

                    memoria_lectura +=2;
                }


                for (i=0;i<num_pulses_one;i++) {
                    //seq_pulses_one[i]=util_get_byte_protect(taperead,total_file_size,memoria_lectura+0)+
                    //    (util_get_byte_protect(taperead,total_file_size,memoria_lectura+1)*256);

                    memoria_lectura +=2;
                }




                //Procesar el total de bits
                //int bit_number=7;
                //z80_byte processing_byte;

                //z80_int *sequence_bit;
                //int longitud_sequence_bit;

                //z80_long_int total_bits_read;



                //z80_byte *puntero_lectura_copia=memoria;
                int memoria_lectura_copia=memoria_lectura;

                //util_get_byte_protect(taperead,total_file_size,memoria_lectura_copia+


                //36 que suficiente por si da la casualidad de cabecera sped (34 bytes)
                //TODO: esto son los primeros pasos para evitar un segfault al hacer un preview de un pzx corrupto
                /*


                longitud_bloque=util_tape_tap_get_info(buffer_temp,buffer_texto);
                */

                z80_byte buffer_temp[36];
                util_memcpy_protect_origin(buffer_temp,taperead,total_file_size,puntero_lectura,36);
                longitud_bloque=util_tape_tap_get_info(buffer_temp,buffer_texto,0);



                copia_puntero=memoria_lectura_copia-2;
                longitud_bloque=count/8;



                char buffer_temp_file[PATH_MAX];
                int longitud_final;

                longitud_final=longitud_bloque-2;

                //else longitud_final=longitud_bloque-2-2; //Saltar los dos de cabecera, el de flag y el crc

                z80_byte tipo_bloque=255;

                //Si bloque de flag 0 y longitud 17 o longitud 34 (sped)
                z80_byte flag=util_get_byte_protect(taperead,total_file_size,copia_puntero+2);

                //printf ("flag %d previo_flag %d previolong %d longitud_final %d\n",flag,previo_flag,previo_longitud_segun_cabecera,longitud_final);

                int longitud_segun_cabecera=-1;

                if (longitud_final>=0) {

                    if (flag==0 && (longitud_final==17 || longitud_final==34) ) {
                        if (tapfile==NULL) {

                            //buffer temporal 34 bytes maximo
                            z80_byte buffer_temp[34];
                            util_memcpy_protect_origin(buffer_temp,taperead,total_file_size,copia_puntero+3,34);


                            util_tape_get_info_tapeblock(buffer_temp,flag,longitud_final+2,buffer_texto);


                            sprintf (buffer_temp_file,"%s/%02d-header-%s",tempdirectory,filenumber,buffer_texto);
                            //printf ("%s/%02d-header-%s\n",tempdirectory,filenumber,buffer_texto);
                        }

                        tipo_bloque=util_get_byte_protect(taperead,total_file_size,copia_puntero+3); //0, program, 3 bytes etc

                        //printf ("%s : tipo %d\n",buffer_temp_file,tipo_bloque);

                        //Longitud segun cabecera
                        longitud_segun_cabecera=value_8_to_16(util_get_byte_protect(taperead,total_file_size,copia_puntero+15),
                                util_get_byte_protect(taperead,total_file_size,copia_puntero+14));

                    }
                    else {
                        char extension_agregar[10];
                        extension_agregar[0]=0; //Por defecto

                        int era_pantalla=0;

                        //Si bloque de flag 255, ver si corresponde al bloque anterior de flag 0
                        if (flag==255 && previo_flag==0 && previo_longitud_segun_cabecera==longitud_final) {
                            //Corresponde. Agregar extensiones bas o scr segun el caso
                            if (previo_tipo_bloque==0) {
                                //Basic
                                strcpy(extension_agregar,".bas");
                            }


                        }

                        //Consideramos que es pantalla siempre que tenga longitud 6912,
                        //sin tener en cuenta que haya cabecera antes o no
                        if (longitud_final==6912) {
                            //archivo a expandir tiene pantalla scr
                            strcpy(extension_agregar,".scr");
                            //y ademas decimos que ese es el achivo de pantalla usado en los previews
                            era_pantalla=1;
                        }


                        if (tapfile==NULL) {
                            sprintf (buffer_temp_file,"%s/%02d-data-%d%s",tempdirectory,filenumber,longitud_final,extension_agregar);

                            if (era_pantalla) {
                                //Indicar con un archivo en la propia carpeta cual es el archivo de pantalla
                                //usado en los previews
                                char buff_preview_scr[PATH_MAX];
                                sprintf(buff_preview_scr,"%s/%s",tempdirectory,MENU_SCR_INFO_FILE_NAME);

                                //Meter en archivo MENU_SCR_INFO_FILE_NAME la ruta al archivo de pantalla
                                util_save_file((z80_byte *)buffer_temp_file,strlen(buffer_temp_file)+1,buff_preview_scr);
                            }

                        }
                    }


                    //Si expandir
                    if (tapfile==NULL) {
                        //Generar bloque con datos, saltando los dos de cabecera y el flag

                        //memoria temporal para ello
                        z80_byte *buffer_temp=malloc(longitud_final);
                        if (buffer_temp==NULL) cpu_panic("Can not allocate memory for pzx expansion");

                        util_memcpy_protect_origin(buffer_temp,taperead,total_file_size,copia_puntero+3,longitud_final);

                        util_save_file(buffer_temp,longitud_final,buffer_temp_file);

                        free(buffer_temp);
                    }

                    //Convertir a tap
                    else {
                        //Generar bloque con datos
                        //Meter longitud, flag
                        z80_byte buffer_tap[3];
                        z80_int longitud_cabecera_tap=longitud_final+2;
                        buffer_tap[0]=value_16_to_8l(longitud_cabecera_tap);
                        buffer_tap[1]=value_16_to_8h(longitud_cabecera_tap);
                        buffer_tap[2]=flag;

                        zvfs_fwrite(in_fatfs_tapfile,buffer_tap,3,ptr_tapfile,&fil_tapfile);
                        //fwrite(buffer_tap,1,3,ptr_tapfile);


                        //Meter datos
                        //memoria temporal para ello
                        z80_byte *buffer_temp=malloc(longitud_final);
                        if (buffer_temp==NULL) cpu_panic("Can not allocate memory for pzx expansion");
                        util_memcpy_protect_origin(buffer_temp,taperead,total_file_size,copia_puntero+3,longitud_final);

                        zvfs_fwrite(in_fatfs_tapfile,buffer_temp,longitud_final,ptr_tapfile,&fil_tapfile);
                        //fwrite(copia_puntero+3,1,longitud_final,ptr_tapfile);

                        //Agregar CRC
                        z80_byte byte_crc=util_get_byte_protect(taperead,total_file_size,copia_puntero+3+longitud_final);


                        buffer_tap[0]=byte_crc;
                        zvfs_fwrite(in_fatfs_tapfile,buffer_tap,1,ptr_tapfile,&fil_tapfile);
                        //fwrite(buffer_tap,1,1,ptr_tapfile);

                        free(buffer_temp);

                    }

                    filenumber++;

                    previo_flag=flag;
                    previo_longitud_segun_cabecera=longitud_segun_cabecera;
                    previo_tipo_bloque=tipo_bloque;

                }

            }



            //Y saltar al siguiente bloque
            puntero_lectura +=block_size;

            remaining_file_size -=block_size;




        }
    }


    free(taperead);

    if (tapfile!=NULL) {
        zvfs_fclose(in_fatfs_tapfile,ptr_tapfile,&fil_tapfile);
        //fclose(ptr_tapfile);
    }

	return 0;

}

int util_convert_sna_to_scr(char *filename,char *archivo_destino)
{
        //snapshot a SCR


        z80_byte sna_48k_header[SNA_48K_HEADER_SIZE];


        FILE *ptr_snafile;

        int filesize=get_file_size(filename);



        if (filesize==49179 || filesize==131103 || filesize==147487) {

                //Soporte para FatFS
                FIL fil_origen;        /* File object */

                int in_fatfs_origen;


                if (zvfs_fopen_read(filename,&in_fatfs_origen,&ptr_snafile,&fil_origen)<0) {
                        debug_printf(VERBOSE_ERR,"Error opening %s",filename);
                        return 1;
                }

                //int leidos;

                //Load File
                /*
                ptr_snafile=fopen(filename,"rb");
                if (ptr_snafile==NULL) {
                        debug_printf(VERBOSE_ERR,"Error opening %s",filename);
                        return 1;
                }
                */

                //48k y 128k tienen misma cabecera al principio
                //leidos=zvfs_fread(in_fatfs_origen,sna_48k_header,SNA_48K_HEADER_SIZE,ptr_snafile,&fil_origen);
                zvfs_fread(in_fatfs_origen,sna_48k_header,SNA_48K_HEADER_SIZE,ptr_snafile,&fil_origen);
                //leidos=fread(sna_48k_header,1,SNA_48K_HEADER_SIZE,ptr_snafile);



                //Leer byte a byte y pasarlo a archivo destino

                FILE *ptr_destination_file;

                FIL fil_destino;        /* File object */

                int in_fatfs_destino;

                if (zvfs_fopen_write(archivo_destino,&in_fatfs_destino,&ptr_destination_file,&fil_destino)<0) {
                    debug_printf (VERBOSE_ERR,"Can not open %s",archivo_destino);
                    return 1;
                }

                /*
                ptr_destination_file=fopen(archivo_destino,"wb");

                        if (!ptr_destination_file) {
                                debug_printf (VERBOSE_ERR,"Can not open %s",archivo_destino);
                                return 1;
                }
                */

                int i;

                //TODO: En el caso de snapshots .sna de 128k, y con juegos que usan la pagina 7 para pantalla, no se vera esa ram 7,
                //porque asumimos siempre la ram 5 (que es lo primero que se lee en el snapshot)
                //debido al poco uso de estos snapshots y la poca posibilidad que use ram 7 (ejemplo con Abadia del crimen)
                //esto se queda pendiente
                //La ordenacion de paginas ram en 128k en este formato es terrible y no me merece el esfuerzo corregirlo

                for (i=0;i<6912;i++) {
                        z80_byte byte_leido;

                        zvfs_fread(in_fatfs_origen,&byte_leido,1,ptr_snafile,&fil_origen);
                        //fread(&byte_leido,1,1,ptr_snafile);

                        zvfs_fwrite(in_fatfs_destino,&byte_leido,1,ptr_destination_file,&fil_destino);
                        //fwrite(&byte_leido,1,1,ptr_destination_file);
                }

                zvfs_fclose(in_fatfs_origen,ptr_snafile,&fil_origen);
                //fclose(ptr_snafile);

                zvfs_fclose(in_fatfs_destino,ptr_destination_file,&fil_destino);
                //fclose(ptr_destination_file);
        }

        return 0;
}


int util_convert_sp_to_scr(char *filename,char *archivo_destino)
{
        //snapshot sp a SCR

        int snap_64kb=0;

        //Soportar snapshot que empieza en rom, como shadow of unicorn
        int tamanyo_archivo=get_file_size(filename);

        if (tamanyo_archivo==65574) snap_64kb=1;


        z80_byte sp_header[SP_HEADER_SIZE];


        FILE *ptr_spfile;



        //int leidos;


        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_spfile,&fil)<0) {
                debug_printf(VERBOSE_ERR,"Error opening %s",filename);
                return 1;
        }

        /*
        //Load File
        ptr_spfile=fopen(filename,"rb");
        if (ptr_spfile==NULL) {
                debug_printf(VERBOSE_ERR,"Error opening %s",filename);
                return 1;
        }
        */

        //leidos=zvfs_fread(in_fatfs,sp_header,SP_HEADER_SIZE,ptr_spfile,&fil);
        zvfs_fread(in_fatfs,sp_header,SP_HEADER_SIZE,ptr_spfile,&fil);
        //leidos=fread(sp_header,1,SP_HEADER_SIZE,ptr_spfile);



        //Leer byte a byte y pasarlo a archivo destino

        FILE *ptr_destination_file;

    //Soporte para FatFS
    FIL fil_destination;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs_destination;


    if (zvfs_fopen_write(archivo_destino,&in_fatfs_destination,&ptr_destination_file,&fil_destination)<0) {
                        debug_printf (VERBOSE_ERR,"Can not open %s",archivo_destino);
                        return 1;
    }

        /*
        ptr_destination_file=fopen(archivo_destino,"wb");

                if (!ptr_destination_file) {
                        debug_printf (VERBOSE_ERR,"Can not open %s",archivo_destino);
                        return 1;
        }
        */

        int i;

        if (snap_64kb) {
            //Saltar los primeros 16kb

            for (i=0;i<16384;i++) {
                    z80_byte byte_leido;

                    zvfs_fread(in_fatfs,&byte_leido,1,ptr_spfile,&fil);

            }
        }

        for (i=0;i<6912;i++) {
                z80_byte byte_leido;

                zvfs_fread(in_fatfs,&byte_leido,1,ptr_spfile,&fil);
                //fread(&byte_leido,1,1,ptr_spfile);

                zvfs_fwrite(in_fatfs_destination,&byte_leido,1,ptr_destination_file,&fil_destination);
                //fwrite(&byte_leido,1,1,ptr_destination_file);
        }

        zvfs_fclose(in_fatfs,ptr_spfile,&fil);
        //fclose(ptr_spfile);

        zvfs_fclose(in_fatfs_destination,ptr_destination_file,&fil_destination);
        //fclose(ptr_destination_file);

        return 0;

}

//Retorna 0 si ok
int util_convert_any_to_scr(char *filename,char *archivo_destino)
{
    //Extraer imagen de cualquier archivo (cinta, snap, disco, etc) a scr

    //Extraer scr de esa cinta, snapshot, etc



    char tmpdir[PATH_MAX];

    char buf_file_no_dir[PATH_MAX];
    util_get_file_no_directory(filename,buf_file_no_dir);

    sprintf (tmpdir,"%s/%s_extract_scr",get_tmpdir_base(),buf_file_no_dir);

    menu_filesel_mkdir(tmpdir);

    int retorno;

    //Tipos de archivos que hay que extraer todos y buscar un archivo de pantalla

    if (!util_compare_file_extension(filename,"tap")) {
        retorno=util_extract_tap(filename,tmpdir,NULL,0);
    }
    else if (!util_compare_file_extension(filename,"tzx")) {
        retorno=util_extract_tzx(filename,tmpdir,NULL);
    }
    else if (!util_compare_file_extension(filename,"pzx") ) {
            retorno=util_extract_pzx(filename,tmpdir,NULL);
    }
    else if (!util_compare_file_extension(filename,"trd") ) {
            retorno=util_extract_trd(filename,tmpdir);
    }
    else if (!util_compare_file_extension(filename,"ddh") ) {
            retorno=util_extract_ddh(filename,tmpdir);
    }
    else if (!util_compare_file_extension(filename,"mdr") ) {
            retorno=util_extract_mdr(filename,tmpdir);
    }
    else if (!util_compare_file_extension(filename,"rmd") ) {
            retorno=util_extract_rmd(filename,tmpdir);
    }
    else if (!util_compare_file_extension(filename,"dsk") ) {
            //Ejemplos de DSK que muestran pantalla: CASTLE MASTER.DSK , Drazen Petrovic Basket.dsk
            retorno=util_extract_dsk(filename,tmpdir);
    }

    //Tipos de archivos en que es conversion directa
    else if (!util_compare_file_extension(filename,"zsf")) {
        util_convert_zsf_to_scr(filename,archivo_destino);
        //No hay que buscar archivo de pantalla. ya lo extrae directo. volvemos
        return 0;
    }

	//Si es sna
	else if (!util_compare_file_extension(filename,"sna")) {
        util_convert_sna_to_scr(filename,archivo_destino);
        //No hay que buscar archivo de pantalla. ya lo extrae directo. volvemos
        return 0;
	}

	//Si es sp
	else if (!util_compare_file_extension(filename,"sp")) {
        util_convert_sp_to_scr(filename,archivo_destino);
        //No hay que buscar archivo de pantalla. ya lo extrae directo. volvemos
        return 0;
	}

	//Si es z80
	else if (!util_compare_file_extension(filename,"z80")) {
		util_convert_z80_to_scr(filename,archivo_destino);
        //No hay que buscar archivo de pantalla. ya lo extrae directo. volvemos
        return 0;
	}

	//Si es P/P81
	else if (!util_compare_file_extension(filename,"p") || !util_compare_file_extension(filename,"p81")) {
		util_convert_p_to_scr(filename,archivo_destino);
        //No hay que buscar archivo de pantalla. ya lo extrae directo. volvemos
        return 0;
	}


    else {
        debug_printf(VERBOSE_ERR,"Unknown input file to extract scr");
        return 1;
    }

    char archivo_info_pantalla[PATH_MAX];
    sprintf(archivo_info_pantalla,"%s/%s",tmpdir,MENU_SCR_INFO_FILE_NAME);
    //printf("archivo_info_pantalla %s\n",archivo_info_pantalla);

    char buf_archivo_scr[PATH_MAX];

    if (si_existe_archivo(archivo_info_pantalla)) {
        lee_archivo(archivo_info_pantalla,buf_archivo_scr,PATH_MAX-1);

        //strcpy(final_scrfile_name,buf_archivo_scr);
        //printf("Leyendo archivo scr %s\n",final_scrfile_name);

        //Finalmente movemos el scr generado a la ruta final
        zvfs_rename(buf_archivo_scr,archivo_destino);

        return 0;
    }

    else {
        debug_printf(VERBOSE_ERR,"File has no SCR screen");

        return 1;
    }



    return 0;
}

int util_convert_zsf_to_scr(char *filename,char *archivo_destino)
//,z80_byte *origin_memory,int longitud_memoria,int load_fast_mode)
{

    FILE *ptr_zsf_file;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("menu_file_p_browser_show %s\n",filename);

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_zsf_file,&fil)<0) {
        debug_printf (VERBOSE_DEBUG,"Error reading snapshot file %s",filename);
        return 1;
    }

    /*
    ptr_zsf_file=fopen(filename,"rb");
    if (!ptr_zsf_file) {
        debug_printf (VERBOSE_DEBUG,"Error reading snapshot file %s",filename);
        return 1;
    }
    */


    //Verificar que la cabecera inicial coincide
    //zsf_magic_header

    char buffer_magic_header[256];

    int longitud_magic=strlen(zsf_magic_header);



    int leidos;

    leidos=zvfs_fread(in_fatfs,(z80_byte *)buffer_magic_header,longitud_magic,ptr_zsf_file,&fil);
    //leidos=fread(buffer_magic_header,1,longitud_magic,ptr_zsf_file);

    if (leidos!=longitud_magic) {
        debug_printf (VERBOSE_DEBUG,"Invalid ZSF file, small magic header");
        zvfs_fclose(in_fatfs,ptr_zsf_file,&fil);
        //fclose(ptr_zsf_file);
        return 1;
    }


    //Comparar texto
    buffer_magic_header[longitud_magic]=0;

    if (strcmp(buffer_magic_header,zsf_magic_header)) {
        debug_printf (VERBOSE_DEBUG,"Invalid ZSF file, invalid magic header");
        zvfs_fclose(in_fatfs,ptr_zsf_file,&fil);
        //fclose(ptr_zsf_file);
        return 1;
    }


    z80_byte block_header[6];

    //Read blocks
    //while (!feof(ptr_zsf_file)) {

    int salir=0;

    //La pagina habitual de pantalla
    int pagina_pantalla=5;

    //Ultimo machine id leido
    z80_byte last_machine_id_read=0;

    while (!zvfs_feof(in_fatfs,ptr_zsf_file,&fil) && !salir) {
        //Read header block
        unsigned int leidos;

        leidos=zvfs_fread(in_fatfs,block_header,6,ptr_zsf_file,&fil);
        //leidos=fread(block_header,1,6,ptr_zsf_file);


        if (leidos==0) break; //End while

        if (leidos!=6) {
            debug_printf(VERBOSE_DEBUG,"Error reading snapshot file. Read: %u Expected: 6",leidos);
            return 1;
        }

        z80_int block_id;
        block_id=value_8_to_16(block_header[1],block_header[0]);
        unsigned int block_lenght_zsf=block_header[2]+(block_header[3]*256)+(block_header[4]*65536)+(block_header[5]*16777216);

        debug_printf (VERBOSE_INFO,"Block id: %u (%s) Length: %u",block_id,zsf_get_block_id_name(block_id),block_lenght_zsf);

        //printf ("Block id: %u (%s) Length: %u\n",block_id,zsf_get_block_id_name(block_id),block_lenght_zsf);

        z80_byte *block_data;

        //Evitar bloques de longitud cero
        //Por si acaso inicializar a algo
        z80_byte buffer_nothing;
        block_data=&buffer_nothing;


        if (block_lenght_zsf) {
            block_data=malloc(block_lenght_zsf);

            if (block_data==NULL) cpu_panic("Can not allocate memory for zsf convert");

            //Read block data
            leidos=zvfs_fread(in_fatfs,block_data,block_lenght_zsf,ptr_zsf_file,&fil);
            //leidos=fread(block_data,1,block_lenght_zsf,ptr_zsf_file);


            if (leidos!=block_lenght_zsf) {
                debug_printf(VERBOSE_DEBUG,"Error reading snapshot file. Read: %u Expected: %u",leidos,block_lenght_zsf);
                return 1;
            }
        }


        z80_byte block_flags;
        z80_int block_start;

        int block_lenght;


        z80_byte ram_page;

        int longitud_original=block_lenght_zsf;

        int i;

        switch(block_id) {

            case ZSF_MACHINEID:
                last_machine_id_read=block_data[0];
            break;

            case ZSF_SPEC128_MEMCONF:
                    /*

                -Block ID 5: ZSF_SPEC128_MEMCONF
                Byte Fields:
                0: Port 32765 contents
                1: Port 8189 contents
                2: Total memory multiplier: 1 for 128kb ram, 2 for 256 kb ram, 4 for 512 kb ram
                    */

                //Ver si usa pantalla en RAM 7
                if (block_data[0] & 8) {
                    pagina_pantalla=7;
                }
            break;




            case ZSF_RAMBLOCK:
                /*
                A ram binary block
                Byte Fields:
                0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
                YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
                1,2: Block start address
                3,4: Block lenght (if 0, means 65536. Value 0 only used on Inves)
                5 and next bytes: data bytes
                */

                //Ya tenemos pantalla
                salir=1;

                i=0;
                block_flags=block_data[i];

                //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes


                i++;
                block_start=value_8_to_16(block_data[i+1],block_data[i]);
                i +=2;
                block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
                i+=2;

                if (block_lenght==0) block_lenght=65536;

                debug_printf (VERBOSE_DEBUG,"Block start: %d Length: %d Compressed: %s Length_source: %d",block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);
                //printf ("Block start: %d Length: %d Compressed: %d Length_source: %d\n",block_start,block_lenght,block_flags&1,longitud_original);


                longitud_original -=5;

                //Asignamos memoria temporal para el bloque
                z80_byte *buffer_memoria;

                //Por si acaso, el doble de lo que en teoria se necesita
                buffer_memoria=malloc(block_lenght*2);

                if (buffer_memoria==NULL) cpu_panic("Can not allocate memory for zsf convert");


                load_zsf_snapshot_block_data_addr(&block_data[i],buffer_memoria,block_lenght,longitud_original,block_flags&1);

                int offset_pantalla=0;

                //printf("lenght: %d\n",block_lenght);

                //Caso de Inves, saltar los primeros 16k de ram baja
                //compruebo por si acaso que block_lenght sea suficiente
                if (last_machine_id_read==MACHINE_ID_INVES && block_lenght>16384+6912) offset_pantalla=16384;

                util_save_file(&buffer_memoria[offset_pantalla],6912,archivo_destino);

                free(buffer_memoria);


            break;

            case ZSF_SPEC128_RAMBLOCK:
            case ZSF_ZXUNO_RAMBLOCK:
            case ZSF_TSCONF_RAMBLOCK:
            case ZSF_CHROME_RAMBLOCK:
                //Codigo compatible tanto para 128k, zxuno, tsconf o chrome, los bloques con iguales excepto por el id
                /*
                -Block ID 6: ZSF_SPEC128_RAMBLOCK
                A ram binary block for a spectrum 128, p2 or p2a machine
                Byte Fields:
                0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
                YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
                1,2: Block start address (currently unused)
                3,4: Block lenght
                5: ram block id (0..7) for a spectrum 128k for example
                6 and next bytes: data bytes

                -Block ID 10: ZSF_ZXUNO_RAMBLOCK
                A ram binary block for a zxuno
                Byte Fields:
                0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
                    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
                1,2: Block start address (currently unused)
                3,4: Block lenght
                5: ram block id
                6 and next bytes: data bytes

                -Block ID 14: ZSF_TSCONF_RAMBLOCK
                A ram binary block for a tsconf
                Byte Fields:
                0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
                    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
                1,2: Block start address (currently unused)
                3,4: Block lenght
                5: ram block id
                6 and next bytes: data bytes


                -Block ID 49: ZSF_CHROME_RAMBLOCK
                A ram binary block for a chrome
                Byte Fields:
                0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
                    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
                1,2: Block start address (currently unused)
                3,4: Block lenght
                5: ram block id
                6 and next bytes: data bytes


                */



                //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes


                i=0;
                block_flags=block_data[i];

                //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

                i++;
                block_start=value_8_to_16(block_data[i+1],block_data[i]);
                i +=2;
                block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
                i+=2;

                ram_page=block_data[i];
                i++;

                //Paginas 5 o 7 de RAM
                if (ram_page==pagina_pantalla) {
                    //Ya tenemos pantalla
                    salir=1;

                    //Asignamos memoria temporal para el bloque
                    z80_byte *buffer_memoria;

                    //Por si acaso, el doble de lo que en teoria se necesita
                    buffer_memoria=malloc(block_lenght*2);

                    if (buffer_memoria==NULL) cpu_panic("Can not allocate memory for zsf convert");

                    debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


                    longitud_original -=6;


                    load_zsf_snapshot_block_data_addr(&block_data[i],buffer_memoria,block_lenght,longitud_original,block_flags&1);

                    util_save_file(buffer_memoria,6912,archivo_destino);

                    free(buffer_memoria);
                }



            break;

            case ZSF_TBBLUE_RAMBLOCK:
                /*

                -Block ID 21: ZSF_TBBLUE_RAMBLOCK
                A ram binary block for a tbblue. We store all the 2048 MB  (memoria_spectrum pointer). Total pages: 128
                Byte Fields:
                0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
                    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
                1,2: Block start address (currently unused)
                3,4: Block lenght
                5: ram block id (in blocks of 16kb)
                6 and next bytes: data bytes
                */

                i=0;
                block_flags=block_data[i];

                //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

                i++;
                block_start=value_8_to_16(block_data[i+1],block_data[i]);
                i +=2;
                block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
                i+=2;

                ram_page=block_data[i];
                i++;

                //Paginas de RAM en Next empiezan en offset 0x040000. 0x040000 / 16384 = 16. 16 + 5 = 21 (el 5 es de pagina 5 de ram)
                if (ram_page==16+pagina_pantalla) {
                    //Ya tenemos pantalla
                    salir=1;

                    //Asignamos memoria temporal para el bloque
                    z80_byte *buffer_memoria;

                    //Por si acaso, el doble de lo que en teoria se necesita
                    buffer_memoria=malloc(block_lenght*2);

                    if (buffer_memoria==NULL) cpu_panic("Can not allocate memory for zsf convert");

                    debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


                    longitud_original -=6;


                    load_zsf_snapshot_block_data_addr(&block_data[i],buffer_memoria,block_lenght,longitud_original,block_flags&1);

                    util_save_file(buffer_memoria,6912,archivo_destino);

                    free(buffer_memoria);
                }

            break;


            case ZSF_PRISM_VRAMBLOCK:
                /*
                A vram binary block for a prism. blocks of 8kb size
                Byte Fields:
                0: Flags. Currently: bit 0: if compressed with repetition block DD DD YY ZZ, where
                    YY is the byte to repeat and ZZ the number of repetitions (0 means 256)
                1,2: Block start address (currently unused)
                3,4: Block lenght
                5: ram block id
                6 and next bytes: data bytes

                */



                //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes


                i=0;
                block_flags=block_data[i];

                //longitud_original : tamanyo que ocupa todo el bloque con la cabecera de 5 bytes

                i++;
                block_start=value_8_to_16(block_data[i+1],block_data[i]);
                i +=2;
                block_lenght=value_8_to_16(block_data[i+1],block_data[i]);
                i+=2;

                ram_page=block_data[i];
                i++;

                //En este caso buscamos la vram0 o la vram2 de prism
                if ((pagina_pantalla==5 && ram_page==0) || (pagina_pantalla==7 && ram_page==2)) {
                    //Ya tenemos pantalla
                    salir=1;

                    //Asignamos memoria temporal para el bloque
                    z80_byte *buffer_memoria;

                    //Por si acaso, el doble de lo que en teoria se necesita
                    buffer_memoria=malloc(block_lenght*2);

                    if (buffer_memoria==NULL) cpu_panic("Can not allocate memory for zsf convert");

                    debug_printf (VERBOSE_DEBUG,"Block ram_page: %d start: %d Length: %d Compressed: %s Length_source: %d",ram_page,block_start,block_lenght,(block_flags&1 ? "Yes" : "No"),longitud_original);


                    longitud_original -=6;


                    load_zsf_snapshot_block_data_addr(&block_data[i],buffer_memoria,block_lenght,longitud_original,block_flags&1);

                    util_save_file(buffer_memoria,6912,archivo_destino);

                    free(buffer_memoria);
                }



            break;


        }


        if (block_lenght_zsf) free(block_data);

    }

    zvfs_fclose(in_fatfs,ptr_zsf_file,&fil);
    //fclose(ptr_zsf_file);

    return 0;

}



void util_convert_p_to_scr_putchar(z80_byte caracter,int x,int y,z80_byte *pantalla_destino)
{

        //Validar origen
        if (y<0 || x<0 || y>23 || x>32) {
                //printf ("x o y fuera de rango\n");
                return;
        }

        //printf ("x %d y %d car %d\n",x,y,caracter);

        z80_int direccion=screen_addr_table[(y*8*32)+x];

        //direccion +=x;

        int inverso=0;

        if (caracter & 128) {
                caracter -=128;
                inverso=1;
        }


        int scanline;

        int offset_caracter=caracter*8;


        for (scanline=0;scanline<8;scanline++) {
                //Validar origen
                if (offset_caracter>=512) {
                        //printf("offset caracter %d fuera de rango\n",caracter);
                        return;
                }

                z80_byte byte_leido=char_set_zx81_no_ascii[offset_caracter++];

                //Validar destino
                if (direccion>=6912) {
                        //printf("offset destino %d fuera de rango\n",direccion);
                        return;
                }

                if (inverso) byte_leido ^=255;

                //printf ("destino %d byte %d\n",direccion,byte_leido);

                pantalla_destino[direccion]=byte_leido;


                direccion +=256;


        }
}

int util_convert_p_to_scr(char *filename,char *archivo_destino)
{
        //snapshot .P a SCR
        z80_byte *buffer_lectura;

        long long int bytes_to_load=get_file_size(filename);

        if (bytes_to_load<20) return 1; //Tamaño muy pequeño

        buffer_lectura=malloc(bytes_to_load);

        if (buffer_lectura==NULL) cpu_panic("Can not allocate memory for snapshot reading");


        FILE *ptr_pfile;



        //int leidos;


        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;


        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_pfile,&fil)<0) {
                debug_printf(VERBOSE_ERR,"Error opening %s",filename);
                return 1;
        }

        //Saltar el nombre del principio si es un .p81
        if (!util_compare_file_extension(filename,"p81")) {
            z80_byte byte_leido=0;
            while (bytes_to_load>0 && (byte_leido&128)==0) {
                //printf("saltando byte\n");
                zvfs_fread(in_fatfs,&byte_leido,1,ptr_pfile,&fil);
                bytes_to_load--;
            }
        }


        //Load File

        /*
        ptr_pfile=fopen(filename,"rb");
        if (ptr_pfile==NULL) {
                debug_printf(VERBOSE_ERR,"Error opening %s",filename);
                return 1;
        }
        */

        //leidos=zvfs_fread(in_fatfs,buffer_lectura,bytes_to_load,ptr_pfile,&fil);
        zvfs_fread(in_fatfs,buffer_lectura,bytes_to_load,ptr_pfile,&fil);
        //leidos=fread(buffer_lectura,1,bytes_to_load,ptr_pfile);


        zvfs_fclose(in_fatfs,ptr_pfile,&fil);
        //fclose(ptr_pfile);

//        //puntero pantalla en DFILE
        /*
        video_pointer=peek_word_no_time(0x400C);


        y snap se carga en:

        puntero_inicio=memoria_spectrum+0x4009;

        por tanto desde el offset de un .p file es en la posicion 3

        Luego restar a eso 9 bytes


        el primer byte es 118 . saltarlo
        */

       int normal_snapshot_load_addr=0x4009;

       int offset_puntero=0x400C-normal_snapshot_load_addr;

       z80_int video_pointer=buffer_lectura[offset_puntero]+256*buffer_lectura[offset_puntero+1];

       //Nota: parece que los snapshot de zx80 (.o) no guardan nunca la pantalla, por lo que un conversor de .o a .scr no tiene sentido
       //aunque en ese caso bastaria con poner ormal_snapshot_load_addr=0x4000


       video_pointer -=normal_snapshot_load_addr;

       //char_set_zx81_no_ascii

       //Asignamos 6912 bytes para la pantalla
        z80_byte *buffer_pantalla;

        buffer_pantalla=malloc(6912);

        if (buffer_pantalla==NULL) cpu_panic("Can not allocate memory for snapshot reading");

        //Pixeles a 0. Atributos a papel 7, tinta 0, brillo 1
        int i;

        for (i=0;i<6144;i++) {
                buffer_pantalla[i]=0;
        }

        for (;i<6912;i++) {
                buffer_pantalla[i]=56+64;
        }


       //se supone que el primer byte es 118 . saltarlo
        video_pointer++;
        int y=0;
        int x=0;
        z80_byte caracter;
        while (y<24) {
                //printf ("y: %d\n",y);

                //Ver rango
                if (video_pointer>=bytes_to_load) {
                    //printf("video pointer incorrecto\n");
                    return 1;
                }

                caracter=buffer_lectura[video_pointer++];
                if (caracter==118) {
                        //rellenar con espacios hasta final de linea. Dado que ya hemos borrado el buffer de pantalla a 0 , esto no hace falta
                                /*for (;x<32;x++) {
                                        printf (" ");
                                        util_convert_p_to_scr_putchar(' ' ,x,y,buffer_pantalla);
					//puntero_printchar_caracter(' ');
                                }*/
                                y++;



                                //printf ("\n");
				//puntero_printchar_caracter('\n');


                                x=0;
                }
                else {
                        //z80_bit inverse;



                        util_convert_p_to_scr_putchar(caracter,x,y,buffer_pantalla);

                        //caracter=da_codigo81(caracter,&inverse);
                        //printf ("%c",caracter);

                        x++;

                        if (x==32) {
                                //Ver rango
                                if (video_pointer>=bytes_to_load) return 1;

                                if (buffer_lectura[video_pointer]!=118) {
                                        //debug_printf (VERBOSE_DEBUG,"End of line %d is not 118 opcode. Is: 0x%x",y,memoria_spectrum[video_pointer]);
								}
                                //saltamos el HALT que debe haber en el caso de linea con 32 caracteres
                                video_pointer++;
                                x=0;
                                y++;


                                //printf ("\n");
				//puntero_printchar_caracter('\n');

                        }

                }


    }

        //Grabar pantalla
        util_save_file(buffer_pantalla,6912,archivo_destino);

        free(buffer_pantalla);
        free(buffer_lectura);

        return 0;

}



int util_convert_z80_to_scr(char *filename,char *archivo_destino)
{
    //snapshot z80 a SCR

	z80_byte z80_header[Z80_MAIN_HEADER_SIZE];

	//Cabecera adicional

	z80_byte z80_header_adicional[Z80_AUX_HEADER_SIZE];

	//Cabecera de cada bloque de datos en version 2 o 3
	z80_byte z80_header_bloque[3];

    z80_byte *buffer_lectura;
    buffer_lectura=malloc(1024*1024); //1 MB, mas que suficiente
    if (buffer_lectura==NULL) cpu_panic("Can not allocate memory for snapshot read");

    z80_byte *buffer_destino;
    buffer_destino=malloc(1024*1024); //1 MB, mas que suficiente
    if (buffer_destino==NULL) cpu_panic("Can not allocate memory for snapshot read");


    FILE *ptr_z80file;

    z80_int direccion_destino;



    int leidos;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_z80file,&fil)<0) {
                debug_printf(VERBOSE_ERR,"Error opening %s",filename);
                return 1;
    }



    leidos=zvfs_fread(in_fatfs,z80_header,Z80_MAIN_HEADER_SIZE,ptr_z80file,&fil);


    int pagina_pantalla=5;


    int comprimido;
    comprimido=(z80_header[12]>>5)&1;

    if (z80_header[6]==0 && z80_header[7]==0) {


        //Z80 version 2 o 3

        //leemos longitud de la cabecera adicional
        leidos=zvfs_fread(in_fatfs,z80_header_adicional,2,ptr_z80file,&fil);
        //leidos=fread(z80_header_adicional,1,2,ptr_z80file);
        z80_int long_cabecera_adicional=value_8_to_16(z80_header_adicional[1],z80_header_adicional[0]);
        if (long_cabecera_adicional!= 23 && long_cabecera_adicional!= 54 && long_cabecera_adicional!= 55) {
                debug_printf(VERBOSE_DEBUG,"Header with %d bytes unknown",long_cabecera_adicional);
                //printf("Header with %d bytes unknown\n",long_cabecera_adicional);
                return 1;

        }

        else {


            //leemos esa cabecera adicional
            debug_printf(VERBOSE_DEBUG,"Reading %d bytes of additional header",long_cabecera_adicional);
            leidos=zvfs_fread(in_fatfs,&z80_header_adicional[2],long_cabecera_adicional,ptr_z80file,&fil);
            //leidos=fread(&z80_header_adicional[2],1,long_cabecera_adicional,ptr_z80file);

            //Ver si snapshot de 128k y obtener pagina activa (5 o 7) de pantalla
            int es_128k=0;
            z80_byte snap_machine_type=z80_header_adicional[4];
            //printf("machine type: %d\n",snap_machine_type);
            //Asumimos version 2
            int version_file=2;
            if (long_cabecera_adicional==54 || long_cabecera_adicional==55) version_file=3;

            if (version_file==2) {
                //V2
                if (snap_machine_type==3 || snap_machine_type==4) es_128k=1;
            }
            else {
                //V3
                if (snap_machine_type==4 || snap_machine_type==5 || snap_machine_type==6) es_128k=1;
            }

            if (snap_machine_type==7 || snap_machine_type==9 || snap_machine_type==12 || snap_machine_type==13) es_128k=1;

            if (es_128k) {
                z80_byte snap_puerto_32765=z80_header_adicional[5];
                if (snap_puerto_32765 & 8) pagina_pantalla=7;
            }

            //printf("pagina pantalla: %d\n",pagina_pantalla);

            int salir=0;

            z80_byte numerobloque;
            //z80_byte valor_puerto_32765;
            z80_int longitudbloque;

            do {

                //leer datos
                //cabecera del bloque de 16kb
                leidos=zvfs_fread(in_fatfs,z80_header_bloque,3,ptr_z80file,&fil);
                //leidos=fread(z80_header_bloque,1,3,ptr_z80file);

                comprimido=1;
                //printf("leidos: %d\n",leidos);
                if (leidos>0) {
                    numerobloque=z80_header_bloque[2];
                    longitudbloque=value_8_to_16(z80_header_bloque[1],z80_header_bloque[0]);
                    if (longitudbloque==65535) {
                            //If length=0xffff, data is 16384 bytes long and not compressed
                            longitudbloque=16384;
                            comprimido=0;
                    }


                    debug_printf(VERBOSE_DEBUG,"Reading %d bytes of data block %d",longitudbloque,numerobloque);
                    leidos=zvfs_fread(in_fatfs,buffer_lectura,longitudbloque,ptr_z80file,&fil);
                    //leidos=fread(buffer_lectura,1,longitudbloque,ptr_z80file);

                    direccion_destino=0;
                    load_z80_snapshot_bytes(buffer_lectura,leidos,direccion_destino,comprimido,buffer_destino);



                    if (numerobloque==pagina_pantalla+3) {
                            //Pagina de la pantalla. No hace falta leer mas
                            salir=1;
                    }
                }

            } while (leidos>0 && !salir);
        }

    }

    else {
        leidos=zvfs_fread(in_fatfs,buffer_lectura,65536,ptr_z80file,&fil);
        //leidos=fread(buffer_lectura,1,65536,ptr_z80file);
        //printf ("despues de carga\n");
        debug_printf(VERBOSE_DEBUG,"Read %d bytes of data",leidos);
        //printf ("despues de debug_printf\n");

        //z80_byte byte_leido;

        direccion_destino=0;

        load_z80_snapshot_bytes(buffer_lectura,leidos,direccion_destino,comprimido,buffer_destino);
    }


    //Grabar 6912 bytes a archivo destino
    util_save_file(buffer_destino,6912,archivo_destino);

    zvfs_fclose(in_fatfs,ptr_z80file,&fil);
    //fclose(ptr_z80file);

    free(buffer_lectura);
    free(buffer_destino);

    return 0;

}



//Realmente lo que hacemos es copiar el .p en .baszx81 con un offset , saltando datos iniciales para situarnos en el programa basic
int util_extract_p(char *filename,char *tempdir)
{


	//tapefile
	if (util_compare_file_extension(filename,"p")!=0 && util_compare_file_extension(filename,"p81")!=0) {
		debug_printf(VERBOSE_ERR,"Expander not supported for this file type");
		return 1;
	}


	//Leemos archivo en memoria
	int total_mem=get_file_size(filename);

	z80_byte *taperead;



        FILE *ptr_tapebrowser;

        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;

        //printf("menu_file_p_browser_show %s\n",filename);

        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return 1;
        }

        /*

        ptr_tapebrowser=fopen(filename,"rb");

        if (!ptr_tapebrowser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return 1;
	}
        */


    char nombre_programa[256];
    strcpy(nombre_programa,"basic-data");

    //Saltar el nombre del principio si es un .p81, guardando en un buffer
    if (!util_compare_file_extension(filename,"p81")) {
        z80_byte byte_leido=0;
        int i=0;
        while (total_mem>0 && (byte_leido&128)==0) {
            //printf("saltando byte\n");
            zvfs_fread(in_fatfs,&byte_leido,1,ptr_tapebrowser,&fil);

            z80_bit inverse;
            z80_byte caracter_ascii=da_codigo81_solo_letras(byte_leido,&inverse);
            nombre_programa[i]=caracter_ascii;
            i++;

            total_mem--;
        }
        nombre_programa[i]=0;
    }

	taperead=malloc(total_mem);
	if (taperead==NULL) cpu_panic("Error allocating memory for expander");


        int leidos;

        leidos=zvfs_fread(in_fatfs,taperead,total_mem,ptr_tapebrowser,&fil);
        //leidos=fread(taperead,1,total_mem,ptr_tapebrowser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
                return 1;
        }

        zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);
        //fclose(ptr_tapebrowser);



        char buffer_temp_file[PATH_MAX];
        sprintf (buffer_temp_file,"%s/%s.baszx81",tempdir,nombre_programa);

        int offset=116;
        //116 = 16509-0x4009

        int longitud_final=total_mem-offset;

        util_save_file(&taperead[offset],longitud_final,buffer_temp_file);


        free(taperead);

	return 0;

}


//Realmente lo que hacemos es copiar el .o en .baszx80 con un offset , saltando datos iniciales para situarnos en el programa basic
int util_extract_o(char *filename,char *tempdir)
{


	//tapefile
	if (util_compare_file_extension(filename,"o")!=0) {
		debug_printf(VERBOSE_ERR,"Expander not supported for this file type");
		return 1;
	}


	//Leemos archivo en memoria
	int total_mem=get_file_size(filename);

	z80_byte *taperead;



        FILE *ptr_tapebrowser;

        //Soporte para FatFS
        FIL fil;        /* File object */
        //FRESULT fr;     /* FatFs return code */

        int in_fatfs;


        if (zvfs_fopen_read(filename,&in_fatfs,&ptr_tapebrowser,&fil)<0) {
            debug_printf(VERBOSE_ERR,"Unable to open file");
            return 1;
        }

        /*

        ptr_tapebrowser=fopen(filename,"rb");

        if (!ptr_tapebrowser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return 1;
	}
    */

	taperead=malloc(total_mem);
	if (taperead==NULL) cpu_panic("Error allocating memory for expander");


        int leidos;

        leidos=zvfs_fread(in_fatfs,taperead,total_mem,ptr_tapebrowser,&fil);
        //leidos=fread(taperead,1,total_mem,ptr_tapebrowser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
                return 1;
        }


        zvfs_fclose(in_fatfs,ptr_tapebrowser,&fil);
        //fclose(ptr_tapebrowser);



        char buffer_temp_file[PATH_MAX];
        sprintf (buffer_temp_file,"%s/basic-data.baszx80",tempdir);

        int offset=40;
        //40 = 16424-16384

        int longitud_final=total_mem-offset;

        util_save_file(&taperead[offset],longitud_final,buffer_temp_file);


        free(taperead);

	return 0;

}







int util_extract_trd(char *filename,char *tempdir)
{

	//tapefile
	if (util_compare_file_extension(filename,"trd")!=0) {
		debug_printf(VERBOSE_ERR,"Expander not supported for this file type");
		return 1;
	}

	//Leemos trd en memoria
	int bytes_to_load=get_file_size(filename);


	int tamanyo_trd_entry=16;


	z80_byte *trd_file_memory;
	trd_file_memory=malloc(bytes_to_load);
	if (trd_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return 0;
	}

	//Leemos cabecera archivo trd
    FILE *ptr_file_trd_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */

    int in_fatfs;

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_trd_browser,&fil)<0) {
        debug_printf(VERBOSE_ERR,"Unable to open file");
        free(trd_file_memory);
        return 0;
    }



    int leidos;

    leidos=zvfs_fread(in_fatfs,trd_file_memory,bytes_to_load,ptr_file_trd_browser,&fil);
    //leidos=fread(trd_file_memory,1,bytes_to_load,ptr_file_trd_browser);

	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading file");
        return 0;
    }

    zvfs_fclose(in_fatfs,ptr_file_trd_browser,&fil);
    //fclose(ptr_file_trd_browser);




	char buffer_texto[64]; //2 lineas, por si acaso


	int start_track_8=256*8;


    z80_int files_on_disk=util_get_byte_protect(trd_file_memory,bytes_to_load,start_track_8+228);

    z80_int deleted_files_on_disk=util_get_byte_protect(trd_file_memory,bytes_to_load,start_track_8+244);




	int puntero,i;

	puntero=0;

    char buffer_temp_file[PATH_MAX];

    //TODO: Total de archivos incluye los borrados??
	for (i=0;i<files_on_disk+deleted_files_on_disk;i++) {
        z80_byte buffer_nombre[10];
        util_memcpy_protect_origin(buffer_nombre,trd_file_memory,bytes_to_load,puntero,9);
		menu_file_mmc_browser_show_file(buffer_nombre,buffer_texto,1,9);
		if (buffer_texto[0]!='?') {

            z80_byte start_sector=util_get_byte_protect(trd_file_memory,bytes_to_load,puntero+14);
            z80_byte start_track=util_get_byte_protect(trd_file_memory,bytes_to_load,puntero+15);
            z80_int longitud_final=util_get_byte_protect(trd_file_memory,bytes_to_load,puntero+11)+256*util_get_byte_protect(trd_file_memory,bytes_to_load,puntero+12);
            debug_printf (VERBOSE_DEBUG,"File %s starts at track %d sector %d size %d",buffer_texto,start_track,start_sector,longitud_final);

            //calcular offset
            /*
            TR-DOS disk specs v1.0 - last revised on 9-28-1997

            - Max disk sides are 2
            - Max logical tracks per side are 80
            - Logical sectors per track are 16
            - Sector dimension is 256 bytes
            - Root directory is 8 sectors long starting from track 0, sector 1
            - Max root entries are 128
            - Root entry dimension is 16 bytes
            - Logical sector 8 (9th physical) holds disc info
            - Logical sectors from 0 to 15 are unused
            - Files are *NOT* fragmented

            */
            int offset=16*256*start_track+256*start_sector;

            //grabar archivo
            sprintf (buffer_temp_file,"%s/%s",tempdir,buffer_texto);

            if (longitud_final==6912) {
                //Indicar con un archivo en la propia carpeta cual es el archivo de pantalla
                //usado en los previews
                char buff_preview_scr[PATH_MAX];
                sprintf(buff_preview_scr,"%s/%s",tempdir,MENU_SCR_INFO_FILE_NAME);

                //Meter en archivo MENU_SCR_INFO_FILE_NAME la ruta al archivo de pantalla
                util_save_file((z80_byte *)buffer_temp_file,strlen(buffer_temp_file)+1,buff_preview_scr);
            }

            //Creamos buffer temporal para esto
            z80_byte *buffer_temp_memoria;
            buffer_temp_memoria=malloc(longitud_final);
            if (buffer_temp_memoria==NULL) cpu_panic("Can not allocate memory for trd extract");
            util_memcpy_protect_origin(buffer_temp_memoria,trd_file_memory,bytes_to_load,offset,longitud_final);

            util_save_file(buffer_temp_memoria,longitud_final,buffer_temp_file);
		}


		puntero +=tamanyo_trd_entry;


	}


	free(trd_file_memory);

    return 0;

}


//Retorna el sector fisico que corresponde a un sector logico
//-1 si no se encuentra
int util_dsk_getphysical_track_sector(z80_byte *dsk_memoria,int total_pistas,int pista_buscar,int sector_buscar,int longitud_dsk)
{

/*
sectores van alternados:
00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

1,6,2,7,3,8


0 1 2 3 4 5 6 7 8
0,5,1,6,2,7,3,8,4

*/

	int pista;
	int sector;

	//int iniciopista_orig=256;

    //printf("util_dsk_getphysical_track_sector. pista_buscar=%d sector_buscar=%d\n",pista_buscar,sector_buscar);

	//Buscamos en todo el archivo dsk
	for (pista=0;pista<total_pistas;pista++) {

        //TODO: de momento cara 0
        int iniciopista_orig=menu_dsk_get_start_track(dsk_memoria,longitud_dsk,pista_buscar,0);

        //printf("before getting sectores_en_pista iniciopista_orig=%XH\n",iniciopista_orig);

		//int sectores_en_pista=dsk_memoria[iniciopista_orig+0x15];

        int sectores_en_pista=util_get_byte_protect(dsk_memoria,longitud_dsk,iniciopista_orig+0x15);
		//debug_printf(VERBOSE_DEBUG,"Iniciopista: %XH (%d). Sectores en pista %d: %d. IDS pista:  ",iniciopista_orig,iniciopista_orig,pista,sectores_en_pista);

        //printf("Iniciopista: %XH (%d). Sectores en pista %d: %d. IDS pista:  \n",iniciopista_orig,iniciopista_orig,pista,sectores_en_pista);

		//int iniciopista_orig=traps_plus3dos_getoff_start_trackinfo(pista);
		int iniciopista=iniciopista_orig;
		//saltar 0x18
		iniciopista +=0x18;

		for (sector=0;sector<sectores_en_pista;sector++) {
			int offset_tabla_sector=sector*8;

			//printf("before getting pista_id sumando %d %d\n",iniciopista,offset_tabla_sector);

			int offset_leer_dsk;

			offset_leer_dsk=iniciopista+offset_tabla_sector;
			//Validar offset
			if (offset_leer_dsk>=longitud_dsk) return -1;

            //printf("before getting pistaid\n");
			//z80_byte pista_id=dsk_memoria[offset_leer_dsk]; //Leemos pista id

            z80_byte pista_id=util_get_byte_protect(dsk_memoria,longitud_dsk,offset_leer_dsk); //Leemos pista id


			//printf("after getting pistaid\n");


			//Validar offset
			offset_leer_dsk=iniciopista+offset_tabla_sector+2;
			if (offset_leer_dsk>=longitud_dsk) return -1;


            //printf("before getting sector_id\n");
			//z80_byte sector_id=dsk_memoria[offset_leer_dsk]; //Leemos c1, c2, etc

            z80_byte sector_id=util_get_byte_protect(dsk_memoria,longitud_dsk,offset_leer_dsk); //Leemos c1, c2, etc

			//printf("after getting sector_id\n");

			//debug_printf(VERBOSE_DEBUG,"%02X ",sector_id);


            //printf("C: %02XH R: %02X \n",pista_id,sector_id);

			sector_id &=0xF;

			sector_id--;  //empiezan en 1...

			if (pista_id==pista_buscar && sector_id==sector_buscar) {
				//printf("Found sector %d/%d at %d/%d\n",pista_buscar,sector_buscar,pista,sector);
		                return sector;
			}

            //printf("after if\n");

		}

        //printf("after for\n");

		//debug_printf(VERBOSE_DEBUG,"");

		iniciopista_orig +=256;
		iniciopista_orig +=512*sectores_en_pista;
	}

    //printf("Not found sector %d/%d\n",pista_buscar,sector_buscar);

	debug_printf(VERBOSE_DEBUG,"Not found sector %d/%d",pista_buscar,sector_buscar);

	//retornamos offset fuera de rango
	//printf("returning -1\n");
	return -1;


}

//Retorna los dos sectores (fisicos, no logicos) que ocupa un bloque de 1kb
void util_dsk_getsectors_block(z80_byte *dsk_file_memory,int longitud_dsk,int bloque,int *sector1,int *pista1,int *sector2,int *pista2,int incremento_pista)
{
			//int total_pistas=longitud_dsk/4864;
            int total_pistas=menu_dsk_get_total_pistas(dsk_file_memory,longitud_dsk);
            //printf("total_pistas: %d\n",total_pistas);
			int pista;
			int sector_en_pista;

			int sector_total;

			sector_total=bloque*2; //cada bloque es de 2 sectores

            //Incremento_pista indica sumar x pistas de desplazamiento al bloque
            sector_total +=9*incremento_pista;

			//tenemos sector total en variable bloque
			//sacar pista
			pista=sector_total/9; //9 sectores por pista
			sector_en_pista=sector_total % 9;

			//printf ("pista: %d sector en pista: %d\n",pista,sector_en_pista);

			//offset a los datos dentro del dsk
			//int offset=pista*4864+sector_en_pista*512;


			//printf("before getting offset1\n");
            *pista1=pista;
			*sector1=util_dsk_getphysical_track_sector(dsk_file_memory,total_pistas,pista,sector_en_pista,longitud_dsk);

			sector_total++;
			pista=sector_total/9; //9 sectores por pista
			sector_en_pista=sector_total % 9;

			//printf("before getting offset2\n");
            *pista2=pista;
			*sector2=util_dsk_getphysical_track_sector(dsk_file_memory,total_pistas,pista,sector_en_pista,longitud_dsk);
			//printf("after getting offset2\n");

}

//parecido memcmp pero usando funciones protegidas de acceso
int util_dsk_memcmp(z80_byte *dsk_file_memory,int longitud_dsk,int puntero1,int puntero2)
{
    //printf("util_dsk_memcmp %XH to %XH\n",puntero1,puntero2);

    int i;

    int resta=0;

    for (i=0;i<11;i++) {
        int byte1=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero1);
        int byte2=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero2);



        int parcial=byte2-byte1;

        resta +=parcial;

        //printf("i: %d %02XH %02XH resta: %d\n",i,byte1,byte2,resta);

        puntero1++;
        puntero2++;

        //no hace falta comparar la string entera. En el momento que 1 caracter cambie, volver
        if (resta!=0) return resta;
    }

    return resta;
}

//Obtener los bloques que usa una entrada del directorio
//Bloques de CP/M de 128 bytes cada uno
//Retorna cantidad de bloques como valor de retorno
//En array de bloques rellena los bloques
int util_dsk_get_blocks_entry_file(z80_byte *dsk_file_memory,int longitud_dsk,z80_byte *bloques,int entrada_obtener)
{
    int pista_filesystem;
    int puntero=menu_dsk_get_start_filesystem(dsk_file_memory,longitud_dsk,&pista_filesystem);

    //Saltar a la entrada indicada
    int saltar=32*entrada_obtener;

    puntero +=saltar;

    int records_entry;

    int total_bloques=0;

    int puntero_a_nombre=puntero+1;

    int mismo_nombre_comp;

    do {
        records_entry=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+15);

        //printf("Entry %d Total bloques %d\n",total_bloques,records_entry);

        //Nos ubicamos en la posicion del primer bloque
        puntero +=16;

        int restantes_records_entry=records_entry;

        int puntero_bloques=puntero;
        while (restantes_records_entry>0)  {
            z80_byte block_id=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero_bloques);

            //printf("Entry %d block %02XH\n",total_bloques,block_id);

            bloques[total_bloques++]=block_id;



            puntero_bloques++;
            //Cada record son de 128 bytes. Cada bloque son de 1024 bytes
            //Por tanto cada bloque son 8 records

            restantes_records_entry -=8;
        }

        puntero+=16;

        //Comparar si siguiente entrada es mismo archivo
        mismo_nombre_comp=util_dsk_memcmp(dsk_file_memory,longitud_dsk,puntero+1,puntero_a_nombre);

        puntero_a_nombre=puntero+1;

    } while (mismo_nombre_comp==0);


    return total_bloques;
}

void util_extract_dsk_get_filename(z80_byte *origen,char *destino,int sipuntoextension,int longitud)
{
	int i;

	for (i=0;i<longitud;i++) {
		char caracter;
		caracter=*origen;


        origen++;
        if (caracter<32 || caracter>126) {
            //Primer caracter si fuera de rango siempre sera ?
            if (i==0) caracter='?';
            else {
                //meterlo en el rango valido de 32...127
                caracter &=127;

                if (caracter==127) caracter=126;

                if (caracter<32) caracter=32+caracter;

            }
        }


        *destino=caracter;
        destino++;

        if (sipuntoextension && i==7) {
            *destino='.';
            destino++;
        }


	}

	*destino=0;
}


int util_extract_dsk(char *filename,char *tempdir)  {


	int tamanyo_dsk_entry=32;

	int max_entradas_dsk=64;

	//Asignamos para 16 entradas
	//int bytes_to_load=tamanyo_dsk_entry*max_entradas_dsk;

	//Max 300kb
	int bytes_to_load=300000;  //temp. 4096

	z80_byte *dsk_file_memory;
	dsk_file_memory=malloc(bytes_to_load);
	if (dsk_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return 0;
	}

    int longitud_dsk=bytes_to_load;

	//Leemos archivo dsk
    FILE *ptr_file_dsk_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;


    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_dsk_browser,&fil)<0) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(dsk_file_memory);
		return 0;
    }


    int leidos;

    leidos=zvfs_fread(in_fatfs,dsk_file_memory,bytes_to_load,ptr_file_dsk_browser,&fil);
    //leidos=fread(dsk_file_memory,1,bytes_to_load,ptr_file_dsk_browser);

	if (leidos==0) {
        debug_printf(VERBOSE_ERR,"Error reading file");
        return 0;
}


    zvfs_fclose(in_fatfs,ptr_file_dsk_browser,&fil);
    //fclose(ptr_file_dsk_browser);




	char buffer_texto[64]; //2 lineas, por si acaso



 	//sprintf(buffer_texto,"DSK disk image");
	//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00000000  45 58 54 45 4e 44 45 44  20 43 50 43 20 44 53 4b  |EXTENDED CPC DSK|
00000010  20 46 69 6c 65 0d 0a 44  69 73 6b 2d 49 6e 66 6f  | File..Disk-Info|
00000020  0d 0a 43 50 44 52 65 61  64 20 76 33 2e 32 34 00  |..CPDRead v3.24.|
00000030  2d 01 00 00 13 13 13 13  13 13 13 13 13 13 13 13  |-...............|
00000040  13 13 13 13 13 13 13 13  13 13 13 13 13 13 13 13  |................|
00000050  13 13 13 13 13 13 13 13  13 13 13 13 00 00 00 00  |................|
00000060  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000200  00 43 4f 4d 50 49 4c 45  52 c2 49 4e 00 00 00 80  |.COMPILER.IN....|
00000210  02 03 04 05 06 07 08 09  0a 0b 0c 0d 0e 0f 10 11  |................|
00000220  00 43 4f 4d 50 49 4c 45  52 c2 49 4e 01 00 00 59  |.COMPILER.IN...Y|
00000230  12 13 14 15 16 17 18 19  1a 1b 1c 1d 00 00 00 00  |................|
00000240  00 4b 49 54 31 32 38 4c  44 c2 49 4e 00 00 00 03  |.KIT128LD.IN....|
00000250  1e 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000260  00 4b 49 54 31 32 38 20  20 c2 49 4e 00 00 00 80  |.KIT128  .IN....|
00000270  1f 20 21 22 23 24 25 26  27 28 29 2a 2b 2c 2d 2e  |. !"#$%&'()*+,-.|
00000280  00 4b 49 54 31 32 38 20  20 c2 49 4e 01 00 00 59  |.KIT128  .IN...Y|
*/

	//La extension es de 1 byte


/*
 	sprintf(buffer_texto,"Disk information:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	util_binary_to_ascii(&dsk_file_memory[0], buffer_texto, 34, 34);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


 	sprintf(buffer_texto,"\nCreator:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	util_binary_to_ascii(&dsk_file_memory[0x22], buffer_texto, 14, 14);

	sprintf(buffer_texto,"\nTracks: %d",dsk_file_memory[0x30]);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Sides: %d",dsk_file_memory[0x31]);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


        sprintf(buffer_texto,"\nFirst PLUS3 entries:");
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
*/


	int i;

    int pista_filesystem;

    int puntero=menu_dsk_get_start_filesystem(dsk_file_memory,bytes_to_load,&pista_filesystem);



/*
en teoria , el directorio empieza en pista 0 sector 0, aunque esta info dice otra cosa:

                TRACK 0          TRACK 1             TRACK 2

SPECTRUM +3    Reserved          Directory             -
                               (sectors 0-3)


Me encuentro con algunos discos en que empiezan en pista 1 y otros en pista 0 ??

*/

/*
TODO. supuestamente entradas del directorio pueden ocupar 4 sectores. Actualmente solo hago 1
*/


    /*

    puntero=menu_dsk_get_start_filesystem(dsk_file_memory,bytes_to_load);

    //printf("puntero: %d\n",puntero);

    //int total_pistas=bytes_to_load/4864;
    int total_pistas=util_get_byte_protect(dsk_file_memory,longitud_dsk,0x30);
    //printf("total pistas: %d\n",total_pistas);


	if (puntero==-1) {
		//printf ("Filesystem track/sector 0/0 not found. Guessing it\n");
		//no encontrado. probar con lo habitual
		puntero=0x200;
	}



    int pista_buscar;

    for (pista_buscar=1;pista_buscar<=2;pista_buscar++) {

        printf("Pista: %d\n",pista_buscar);
		//Si contiene e5 en el nombre, nos vamos a pista 1
        //O si segundo caracter no es ascii

        z80_byte byte_name=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+1);

		if (byte_name==0xe5 || byte_name<32 || byte_name>127) {

            puntero=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,pista_buscar,0,longitud_dsk);

			if (puntero==-1) {
                printf ("Filesystem track/sector not found. Guessing it\n");
                //no encontrado. probar con lo habitual
                puntero=0x200;
			}

		}

        else break;

    }

    printf("Found filesystem at track %d. Puntero=%X\n",pista_buscar,puntero);
    */



	puntero++; //Saltar el primer byte en la entrada de filesystem

/*
TODO
Pista 1 suele empezar en offset 1400H, aunque me encuentro con algunos discos, como el http://www.worldofspectrum.org/infoseekid.cgi?id=0011456 (4 Top Games),
en que empieza en 1300H. Porque??
*/


	z80_byte *buffer_temp;
	buffer_temp=malloc(80000);


	for (i=0;i<max_entradas_dsk;i++) {

                //printf("entrada %d\n",i);

        z80_byte file_is_deleted=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero-1);
        //printf("before menu_file_mmc_browser_show_file\n");
        z80_byte buffer_nombre[12];
        util_memcpy_protect_origin(buffer_nombre,dsk_file_memory,longitud_dsk,puntero,11);
		//menu_file_mmc_browser_show_file(buffer_nombre,buffer_texto,1,11);

        //Emilio Butragueno Futbol.dsk tiene 4 archivos llamados BUITRE.XXX donde XXX son caracteres no validos
        //Si no se muestran esos XXX, se cambian por espacios, y entonces hay 4 archivos todos iguales , y uno de ellos es la pantalla
        //y por tanto no se muestra la pantalla
        //por tanto hacemos que esos XXX sean caracteres visibles
        util_extract_dsk_get_filename(buffer_nombre,buffer_texto,1,11);

        //printf("after menu_file_mmc_browser_show_file\n");
        /*printf("nombre orig: [");
        int kk;
        for (kk=0;kk<11;kk++) printf("%02X ",buffer_nombre[kk]);
        printf("]\n");
        printf("nombre final: [%s]\n",buffer_texto);*/


		if (buffer_texto[0]!='?') {

            if (file_is_deleted==0xE5) debug_printf (VERBOSE_DEBUG,"File %s is deleted. Skipping",buffer_texto);
            else {
                //indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                debug_printf (VERBOSE_DEBUG,"File %s",buffer_texto);

                            //printf("puntero: %d\n",puntero);

                //printf ("File %s\n",buffer_texto);

                z80_byte continuation_marker=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+12-1); //-1 porque empezamos el puntero en primera posicion

                //Averiguar inicio de los datos
                //Es un poco mas complejo ya que hay que localizar cada sector donde esta ubicado
                int total_bloques=1;
                int bloque;

                bloque=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+15);

                //printf ("Bloque %02XH\n",bloque);

                //printf("after dsk_file_memory[puntero+15];\n");

                //Este bloque indica el primer bloque de 1k del archivo. Esta ubicado en el principio de cada entrada de archivo+16
                //(aqui hay 15 porque ya empezamos desplazados en 1 - 0x201)
                //Luego se pueden guardar hasta 16 bloques en esa entrada
                //Si el archivo ocupa mas de 16kb, se genera otra entrada con mismo nombre de archivo, con los siguientes bloques
                //Si ocupa mas de 32 kb, otra entrada mas, etc
                //Notas: archivo de 16 kb exactos, genera dos entradas de archivo, incluso ocupando un bloque del siguiente,
                //dado que al principio esta la cabecera de plus3dos
                //Desde la cabecera de plus3dos a los datos hay 0x80 bytes

                z80_int longitud_real_archivo=0;

                int destino_en_buffer_temp=0;

                do {

                    int offset1,offset2;

                    //printf("before menu_dsk_getoff_block\n");
                    //Si filesystem ha empezado en pista 1, pista_buscar-1=0 por tanto no desplazamos nada
                    //Si filesystem ha empezado en pista 2, pista_buscar-1=1 por tanto desplazamos una pista


                    menu_dsk_getoff_block(dsk_file_memory,bytes_to_load,bloque,&offset1,&offset2,pista_filesystem);

                    //printf("offset1 %XH offset2 %XH\n",offset1,offset2);


                    if (offset1<0 || offset2<0) {
                        debug_printf(VERBOSE_DEBUG,"Error reading dsk offset block");
                        return 0; //TODO: O retornar error?
                    }

                    //Sacar longitud real, de cabecera plus3dos. Solo el primer sector contiene cabecera plus3dos y la primera entrada del archivo,
                    //por tanto el primer sector contiene 512-128=384 datos, mientras que los siguientes,
                    //contienen 512 bytes de datos. El sector final puede contener 512 bytes o menos
                    if (total_bloques==1 && continuation_marker==0) {
                        int offset_a_longitud=offset1+16;
                        //printf("Offset a longitud: %XH\n",offset_a_longitud);
                        longitud_real_archivo=util_get_byte_protect(dsk_file_memory,longitud_dsk,offset_a_longitud)+256*util_get_byte_protect(dsk_file_memory,longitud_dsk,offset_a_longitud+1);
                        debug_printf (VERBOSE_DEBUG,"Real length file %s read from PLUS3DOS header: %d",buffer_texto,longitud_real_archivo);

                        //printf ("Real length file %s read from PLUS3DOS header: %d\n",buffer_texto,longitud_real_archivo);

                        util_memcpy_protect_origin(buffer_temp,dsk_file_memory,longitud_dsk,offset1+128,512-128);
                        //memcpy(buffer_temp,&dsk_file_memory[offset1+128],512-128);

                        destino_en_buffer_temp=destino_en_buffer_temp + (512-128);

                        //Siguiente sector
                        util_memcpy_protect_origin(&buffer_temp[destino_en_buffer_temp],dsk_file_memory,longitud_dsk,offset2,512);
                        //memcpy(&buffer_temp[destino_en_buffer_temp],&dsk_file_memory[offset2],512);
                        //printf ("Escribiendo sector 2\n");

                        //printf("destino_en_buffer_temp 1 %d\n",destino_en_buffer_temp);

                        destino_en_buffer_temp +=512;

                                            //printf("after memcpy 1\n");


                    }

                    else {

                        //Los dos sectores
                        //printf("destino_en_buffer_temp 2 %d\n",destino_en_buffer_temp);

                        util_memcpy_protect_origin(&buffer_temp[destino_en_buffer_temp],dsk_file_memory,longitud_dsk,offset1,512);
                        //memcpy(&buffer_temp[destino_en_buffer_temp],&dsk_file_memory[offset1],512);
                        destino_en_buffer_temp +=512;

                        util_memcpy_protect_origin(&buffer_temp[destino_en_buffer_temp],dsk_file_memory,longitud_dsk,offset2,512);
                        //memcpy(&buffer_temp[destino_en_buffer_temp],&dsk_file_memory[offset2],512);
                        destino_en_buffer_temp +=512;

                        //printf("after memcpy 2\n");
                    }


                    //printf ("b:%02XH of:%XH %XH  ",bloque,offset1,offset2);
                    //Cada offset es un sector de 512 bytes

                    total_bloques++;
                    bloque=util_get_byte_protect(dsk_file_memory,longitud_dsk,puntero+15-1+total_bloques);



                } while (bloque!=0 && total_bloques<=16);

                //Grabar archivo
                char buffer_nombre_destino[PATH_MAX];
                sprintf (buffer_nombre_destino,"%s/%s",tempdir,buffer_texto);

			    //Ver si es primer entrada de archivo (con lo que sobreescribimos) o si es segunda y siguientes, hacer append
/*

Byte 12
    _______________            Continuation marker.  Each directory entry
   | | | | | | | | |           provides space for 16 data blocks
   | | | |*| | | | |           identifying where data is stored on the
   | | | | | | | | |           disk.  Files exceeding 16K use additional
   |_|_|_|_|_|_|_|_|           entries to record data blocks.  For
                               files up to 16K and for the initial entry
   of a longer file this byte value is 00.  For the fist continuation
   it is 01 for the second 02 and so on.
*/

			    int longitud_en_bloques=destino_en_buffer_temp; //(total_bloques-1)*1024;


			    if (continuation_marker==0) {
                    int longitud_final=longitud_en_bloques;

                    //printf("longitud_final: %d\n",longitud_final);

                    //En este caso, se ha grabado el archivo inicial y se sabe la longitud real segun cabecera plus3dos
                    //si resulta que la longitud en bloques a guardar ahora es mayor que la cabecera, reducir
                    if (longitud_final>longitud_real_archivo) longitud_final=longitud_real_archivo;

                    //printf("longitud_final despues ajustar: %d\n",longitud_final);

                    debug_printf (VERBOSE_DEBUG,"File entry is the first. Saving %d bytes on file %s",longitud_final,buffer_nombre_destino);
                    util_save_file(buffer_temp,longitud_final,buffer_nombre_destino);


                    if (longitud_final==6912) {
                        //Indicar con un archivo en la propia carpeta cual es el archivo de pantalla
                        //usado en los previews
                        char buff_preview_scr[PATH_MAX];
                        sprintf(buff_preview_scr,"%s/%s",tempdir,MENU_SCR_INFO_FILE_NAME);

                        //Meter en archivo MENU_SCR_INFO_FILE_NAME la ruta al archivo de pantalla
                        util_save_file((z80_byte *)buffer_nombre_destino,strlen(buffer_nombre_destino)+1,buff_preview_scr);
                    }
			    }

                else {
                    debug_printf (VERBOSE_DEBUG,"File entry is not the first. Adding %d bytes to the file %s",longitud_en_bloques,buffer_nombre_destino);
                    util_file_append(buffer_nombre_destino,buffer_temp,longitud_en_bloques);
                    //TODO: al guardar entradas de archivo diferentes a la primera,
                    //se agregan siempre longitudes en bloques de 1kb
                    //por lo que el archivo acabará ocupando mas de lo que deberia
                }


			debug_printf (VERBOSE_DEBUG,"Saving file %s",buffer_nombre_destino);

		}
    }

    puntero +=tamanyo_dsk_entry;


    /*
    Como saber, de cada archivo, donde está ubicado en disco, y su longitud?
    Ejemplo:

00000200  00 4c 20 20 20 20 20 20  20 a0 a0 20 00 00 00 03  |.L       .. ....|
00000210  02 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000220  00 44 49 53 4b 20 20 20  20 a0 20 20 00 00 00 02  |.DISK    .  ....|
00000230  03 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000240  00 50 41 4e 54 20 20 20  20 42 41 4b 00 00 00 38  |.PANT    BAK...8|
00000250  04 05 06 07 08 09 0a 00  00 00 00 00 00 00 00 00  |................|
00000260  00 50 41 4e 54 20 20 20  20 20 20 20 00 00 00 38  |.PANT       ...8|
00000270  0b 0c 0d 0e 0f 10 11 00  00 00 00 00 00 00 00 00  |................|
00000280  00 50 41 4e 54 32 20 20  20 20 20 20 00 00 00 38  |.PANT2      ...8|
00000290  12 13 14 15 16 17 18 00  00 00 00 00 00 00 00 00  |................|
000002a0  00 50 41 4e 54 32 20 20  20 42 41 4b 00 00 00 38  |.PANT2   BAK...8|
000002b0  19 1a 1b 1c 1d 1e 1f 00  00 00 00 00 00 00 00 00  |................|
000002c0  00 50 41 4e 54 33 20 20  20 42 41 4b 00 00 00 38  |.PANT3   BAK...8|
000002d0  20 21 22 23 24 25 26 00  00 00 00 00 00 00 00 00  | !"#$%&.........|
000002e0  00 50 41 4e 54 33 20 20  20 20 20 20 00 00 00 38  |.PANT3      ...8|
000002f0  27 28 29 2a 2b 2c 2d 00  00 00 00 00 00 00 00 00  |'()*+,-.........|
00000300  00 42 41 53 49 43 20 20  20 20 20 20 00 00 00 03  |.BASIC      ....|
00000310  2e 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000320  e5 e5 e5 e5 e5 e5 e5 e5  e5 e5 e5 e5 e5 e5 e5 e5  |................|

Ese PANT es un code 16384,6913

Este es el DISK?:
00000800  50 4c 55 53 33 44 4f 53  1a 01 00 b3 00 00 00 00  |PLUS3DOS........|
00000810  33 00 0a 00 33 00 00 00  00 00 00 00 00 00 00 00  |3...3...........|
00000820  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000870  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 9b  |................|
00000880  00 0a 2f 00 e7 c3 a7 3a  f4 32 33 36 39 33 0e 00  |../....:.23693..|
00000890  00 8d 5c 00 2c c3 a7 3a  fd b0 22 32 33 39 39 39  |..\.,..:.."23999|
000008a0  22 3a ef 22 6c 22 af 3a  f9 c0 b0 22 32 34 30 30  |":."l".:..."2400|
000008b0  30 22 0d 00 00 00 00 00  00 00 00 00 00 00 00 00  |0"..............|


Este el PANT:

00001000  50 4c 55 53 33 44 4f 53  1a 01 00 81 1b 00 00 03  |PLUS3DOS........|
00001010  01 1b 00 40 00 80 00 00  00 00 00 00 00 00 00 00  |...@............|
00001020  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00001070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 f3  |................|
00001080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00001180  00 00 00 18 00 7c 7e 42  00 00 00 00 00 00 00 00  |.....|~B........|


Fijarse en la linea:
00001010  01 1b 00 40

1b01H=6913
4000H=16384

Aqui supuestamente esta PANT2:
(19968=4e00h)
00004e00  50 4c 55 53 33 44 4f 53  1a 01 00 82 1b 00 00 03  |PLUS3DOS........|
00004e10  02 1b 00 40 00 80 00 00  00 00 00 00 00 00 00 00  |...@............|
00004e20  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00004e70  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 f5  |................|
00004e80  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

PANT2 en la tabla de asignacion:
00000280  00 50 41 4e 54 32 20 20  20 20 20 20 00 00 00 38  |.PANT2      ...8|
00000290  12 13 14 15 16 17 18 00  00 00 00 00 00 00 00 00  |................|

Ese 12h indica de alguna manera que empieza en 4e00h
19968-512=19456
19456/1024=19 =18+1=12H+1


PANT en la tabla de asignacion:
00000260  00 50 41 4e 54 20 20 20  20 20 20 20 00 00 00 38  |.PANT       ...8|
00000270  0b 0c 0d 0e 0f 10 11 00  00 00 00 00 00 00 00 00  |................|



Este "Basic" es:
00000300  00 42 41 53 49 43 20 20  20 20 20 20 00 00 00 03  |.BASIC      ....|
00000310  2e 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

2e+1=2f
2fh*1024=48128=0xBC00

BC00H+512=0xBE00

Aunque el basic esta en:

0000c800  50 4c 55 53 33 44 4f 53  1a 01 00 18 01 00 00 00  |PLUS3DOS........|
0000c810  98 00 00 80 98 00 00 00  00 00 00 00 00 00 00 00  |................|
0000c820  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
0000c870  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 41  |...............A|
0000c880  00 01 0e 00 ea 68 6f 6c  61 20 71 75 65 20 74 61  |.....hola que ta|
0000c890  6c 0d 00 02 02 00 f0 0d  00 03 21 00 f8 22 61 3a  |l.........!.."a:|
0000c8a0  70 61 6e 74 22 af 31 36  33 38 34 0e 00 00 00 40  |pant".16384....@|

c800h/1024=32h


		*/
	}




	free(dsk_file_memory);

	return 0;

}



int util_extract_z88_card(char *filename,char *tempdir)
{

        long long int bytes_to_load=get_file_size(filename);

        z80_byte *flash_file_memory;
        flash_file_memory=malloc(bytes_to_load);
        if (flash_file_memory==NULL) {
                debug_printf(VERBOSE_ERR,"Unable to assign memory");
                return 1;
        }

        //Leemos archivo
        FILE *ptr_file_flash_browser;
        ptr_file_flash_browser=fopen(filename,"rb");

        if (!ptr_file_flash_browser) {
                debug_printf(VERBOSE_ERR,"Unable to open file");
                free(flash_file_memory);
                return 1;
        }


        int leidos=fread(flash_file_memory,1,bytes_to_load,ptr_file_flash_browser);

        if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return 1;
        }


        fclose(ptr_file_flash_browser);

        //El segundo byte tiene que ser 0 (archivo borrado) o '/'
        if (flash_file_memory[1]!=0 && flash_file_memory[1]!='/') {
                //No es archivo epr, eprom o flash con archivos. Salir
                debug_printf (VERBOSE_INFO,"This is not a Z88 File Card");
        	free(flash_file_memory);
        	return 1;
        }


        z88_eprom_flash_file file;


        z80_byte *dir;

        dir=flash_file_memory;

        char buffer_nombre[Z88_MAX_CARD_FILENAME+1];

        int retorno;


        do {
                z80_byte *dir_current;
                dir_current=dir;

                retorno=z88_eprom_new_ptr_flash_find_next(&dir,&file);
                if (retorno) {
                        z88_eprom_flash_get_file_name(&file,buffer_nombre);

			if (buffer_nombre[0]=='.') buffer_nombre[0]='D'; //archivo borrado

                        z80_long_int tamanyo=file.size[0]+(file.size[1]*256)+(file.size[2]*65536)+(file.size[3]*16777216);
                        debug_printf (VERBOSE_INFO,"Name: %s size: %d",buffer_nombre,tamanyo);

                        //Datos empieza en dir+(byte longitud nombre)+longitud nombre+4(de tamanyo archivo)
                        z80_byte *data_ptr=dir_current+1+file.namelength+4;

                        //Si nombre empieza por /, evitar primer byte
                        int index_nombre=0;
                        if (buffer_nombre[0]=='/') index_nombre++;


                        //Grabar archivo
			char buffer_nombre_destino[PATH_MAX];
			sprintf (buffer_nombre_destino,"%s/%s",tempdir,&buffer_nombre[index_nombre]);

			util_save_file(data_ptr,tamanyo,buffer_nombre_destino);

                }

        } while (retorno!=0);


        free(flash_file_memory);


        return 0;
}



//Archivos de basic z88 acaban con 3 bytes: 00 FF FF
int file_is_z88_basic(char *filename)
{
        long long int bytes_to_load=get_file_size(filename);

        z80_byte *flash_file_memory;
        flash_file_memory=malloc(bytes_to_load);
        if (flash_file_memory==NULL) {
                debug_printf(VERBOSE_ERR,"Unable to assign memory");
                return 0;
        }

        //Leemos archivo
        FILE *ptr_file_flash_browser;

    //Soporte para FatFS
    FIL fil;        /* File object */
    //FRESULT fr;     /* FatFs return code */

    int in_fatfs;

    //printf("file_is_z88_basic %s\n",filename);

    if (zvfs_fopen_read(filename,&in_fatfs,&ptr_file_flash_browser,&fil)<0) {
        //debug_printf(VERBOSE_ERR,"Unable to open file");
        free(flash_file_memory);
        return 0;
    }

/*
        ptr_file_flash_browser=fopen(filename,"rb");

        if (!ptr_file_flash_browser) {
                debug_printf(VERBOSE_ERR,"Unable to open file");
                free(flash_file_memory);
                return 0;
        }
*/

        //int leidos;

        //leidos=zvfs_fread(in_fatfs,flash_file_memory,bytes_to_load,ptr_file_flash_browser,&fil);
        zvfs_fread(in_fatfs,flash_file_memory,bytes_to_load,ptr_file_flash_browser,&fil);

        //leidos=fread(flash_file_memory,1,bytes_to_load,ptr_file_flash_browser);

        //if (leidos==0) {
        //        debug_printf(VERBOSE_ERR,"Error reading file");
        //        return 0;
        //}


        zvfs_fclose(in_fatfs,ptr_file_flash_browser,&fil);
        //fclose(ptr_file_flash_browser);

        int esbasic=0;

        if (bytes_to_load>3) {
           if (flash_file_memory[bytes_to_load-3]==0x00 &&
                flash_file_memory[bytes_to_load-2]==0xFF &&
                flash_file_memory[bytes_to_load-1]==0xFF) {

                debug_printf (VERBOSE_INFO,"File is probably Z88 Basic");

                esbasic=1;
           }
        }

        free(flash_file_memory);

        return esbasic;
}



void util_save_game_config(char *filename)
{
  //Sobreescribimos archivos con settings actuales
  debug_printf (VERBOSE_INFO,"Writing configuration file");

  //Agregamos contenido
  char config_settings[20000]; //Esto es mas que suficiente

  char buffer_temp[MAX_CONFIG_SETTING]; //Para cadenas temporales
  //char buffer_temp2[MAX_CONFIG_SETTING]; //Para cadenas temporales

  int indice_string=0;

  int i;


  //Al menos generamos una linea en blanco inicial
                                              ADD_STRING_CONFIG,";Autogenerated autoconfig file");



  //Y empezamos a meter opciones

  get_machine_config_name_by_number(buffer_temp,current_machine_type);
  if (buffer_temp[0]!=0) {
                                              ADD_STRING_CONFIG,"--machine %s",buffer_temp);
  }

  if (frameskip)                              ADD_STRING_CONFIG,"--frameskip %d",frameskip);


  if (border_enabled.v==0)                    ADD_STRING_CONFIG,"--disableborder");


//Estas siguientes solo si es ZX80/81
  if (MACHINE_IS_ZX8081) {
        if (zx8081_get_standard_ram()!=16)          ADD_STRING_CONFIG,"--zx8081mem %d",zx8081_get_standard_ram());
        if (zx8081_vsync_sound.v)                   ADD_STRING_CONFIG,"--zx8081vsyncsound");
        if (ram_in_8192.v)                          ADD_STRING_CONFIG,"--zx8081ram8K2000");
        if (ram_in_32768.v)                         ADD_STRING_CONFIG,"--zx8081ram16K8000");
        if (ram_in_49152.v)                         ADD_STRING_CONFIG,"--zx8081ram16KC000");
        if (wrx_present.v)                          ADD_STRING_CONFIG,"--wrx");
        if (chroma81.v)                             ADD_STRING_CONFIG,"--chroma81");
                                                    ADD_STRING_CONFIG,"--vsync-minimum-length %d",minimo_duracion_vsync);
        if (video_zx8081_estabilizador_imagen.v==0) ADD_STRING_CONFIG,"--no-horiz-stabilization");
        if (video_zx8081_lnctr_adjust.v)            ADD_STRING_CONFIG,"--enablezx8081lnctradjust");
  }


  if (rainbow_enabled.v)                      ADD_STRING_CONFIG,"--realvideo");
  if (video_interlaced_mode.v)                ADD_STRING_CONFIG,"--enableinterlaced");




//Estas solo si es Spectrum
  if (MACHINE_IS_SPECTRUM) {
        if (snow_effect_enabled.v)                  ADD_STRING_CONFIG,"--snoweffect");
        if (ulaplus_presente.v)                     ADD_STRING_CONFIG,"--enableulaplus");
        if (spectra_enabled.v)                      ADD_STRING_CONFIG,"--enablespectra");
        if (timex_video_emulation.v)                ADD_STRING_CONFIG,"--enabletimexvideo");
  }


  if (gigascreen_enabled.v)                   ADD_STRING_CONFIG,"--enablegigascreen");

/*
--redefinekey src dest
--joystickkeyev evt key
*/

					ADD_STRING_CONFIG,"--clearredefinekey");

                                        //Esto conviene meterlo en el .config, asi el usuario
                                        //sabe que sus botones a eventos se inicializan en el juego
                                        //y se establecen un poco mas abajo
                                        //en caso que no quisiera, que lo quite

					ADD_STRING_CONFIG,"--cleareventlist");


						ADD_STRING_CONFIG,"--joystickemulated \"%s\"",joystick_texto[joystick_emulation]);


  //real joystick buttons to events. Siempre este antes que el de events/buttons to keys
  for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
        if (realjoystick_events_array[i].asignado.v) {
                char texto_button[20];
                int button_type;
                button_type=realjoystick_events_array[i].button_type;

                util_write_config_aux_realjoystick(button_type, realjoystick_events_array[i].button, texto_button);

                ADD_STRING_CONFIG,"--joystickevent %s %s",texto_button,realjoystick_event_names[i]);
        }
  }


  //real joystick buttons to keys
  for (i=0;i<MAX_KEYS_JOYSTICK;i++) {
        if (realjoystick_keys_array[i].asignado.v) {
                char texto_button[20];
                int button_type;
                z80_byte caracter;
                caracter=realjoystick_keys_array[i].caracter;
                button_type=realjoystick_keys_array[i].button_type;

                //Si hay evento en vez de numero de boton, meter --joystickkeyev

                //printf ("Buscando evento para boton %d tipo %d\n",realjoystick_keys_array[i].button,button_type);
                int buscar_evento_index=realjoystick_buscar_evento_en_tabla(realjoystick_keys_array[i].button,button_type);
                if (buscar_evento_index>=0) {
                        //printf ("Encontrado evento %d para boton %d tipo %d\n",buscar_evento_index,realjoystick_keys_array[i].button,button_type);
                    ADD_STRING_CONFIG,"--joystickkeyev %s %d",realjoystick_event_names[buscar_evento_index],caracter);
                }

                else {
                        util_write_config_aux_realjoystick(button_type, realjoystick_keys_array[i].button, texto_button);
                        ADD_STRING_CONFIG,"--joystickkeybt %s %d",texto_button,caracter);
                }
        }
  }


  //text osd keyboard
  for (i=0;i<osd_adv_kbd_defined;i++) {
          //Truco para poder poner " en el texto. Con barra invertida
          if (!strcmp(osd_adv_kbd_list[i],"\"")) ADD_STRING_CONFIG,"--text-keyboard-add \\");
        else ADD_STRING_CONFIG,"--text-keyboard-add \"%s\"",osd_adv_kbd_list[i]);
  }

        ADD_STRING_CONFIG,"--text-keyboard-length %d",adventure_keyboard_key_length);

        if (adventure_keyboard_send_final_spc) ADD_STRING_CONFIG,"--text-keyboard-finalspc");



         FILE *ptr_configfile;

     ptr_configfile=fopen(filename,"wb");
     if (!ptr_configfile) {
                        debug_printf(VERBOSE_ERR,"Cannot write configuration file %s",filename);
                        return;
      }

    fwrite(config_settings, 1, strlen(config_settings), ptr_configfile);


      fclose(ptr_configfile);


}


int util_find_right_most_space(char *texto)
{
        int i=strlen(texto)-1;

        for (;i>=0;i--) {
                if (texto[i]!=' ') return i;
        }

        return i;
}

void util_clear_final_spaces(char *orig,char *destination)
{
        //Quitar espacios al final del texto
        //Primero localizar primer espacio de la derecha

        int indice=0;

        int espacio_derecha=util_find_right_most_space(orig);

        if (espacio_derecha>=0) {
            for (indice=0;indice<=espacio_derecha && orig[indice];indice++) {
                    destination[indice]=orig[indice];
            }
        }

        destination[indice]=0;
}


int util_is_digit(char c)
{
        if (c>='0' && c<='9') return 1;
        return 0;
}

int util_is_letter(char c)
{
        if (letra_minuscula(c)>='a' && letra_minuscula(c)<='z') return 1;
        return 0;
}


int util_get_available_drives(char *texto)
{
#ifdef MINGW
	int bitmask_unidades=GetLogicalDrives(); //Mascara de bits. bit inferior si unidad A disponible, etc
#else
	int bitmask_unidades=0;
#endif

	int unidades_detectadas=0;

	char letra_actual='A';

	for (;letra_actual<='Z';letra_actual++) {
		//Ver bit inferior
		if (bitmask_unidades&1) {
			//printf ("letra actual: %d unidades_detectadas: %d\n",letra_actual,unidades_detectadas);
			texto[unidades_detectadas]=letra_actual;
			unidades_detectadas++;
		}

		bitmask_unidades >>=1;
	}

	//y final de texto
	texto[unidades_detectadas]=0;


	return unidades_detectadas;
}

int get_cpu_frequency(void)
{
        int cpu_hz=screen_testados_total*50;

        return cpu_hz;
}




//Retorna 0 si no encontrado
int util_find_window_geometry(char *nombre,int *x,int *y,int *ancho,int *alto,int *is_minimized,int *is_maximized,
    int *width_before_max_min_imize,int *height_before_max_min_imize)
{
        int i;

        for (i=0;i<total_config_window_geometry;i++) {
                if (!strcasecmp(nombre,saved_config_window_geometry_array[i].nombre)) {
                        *x=saved_config_window_geometry_array[i].x;
                        *y=saved_config_window_geometry_array[i].y;
                        *ancho=saved_config_window_geometry_array[i].ancho;
                        *alto=saved_config_window_geometry_array[i].alto;
                        *is_minimized=saved_config_window_geometry_array[i].is_minimized;
                        *is_maximized=saved_config_window_geometry_array[i].is_maximized;
                        *width_before_max_min_imize=saved_config_window_geometry_array[i].width_before_max_min_imize;
                        *height_before_max_min_imize=saved_config_window_geometry_array[i].height_before_max_min_imize;
                        debug_printf (VERBOSE_DEBUG,"Returning window geometry %s from index %d, %d,%d %dX%d (%dX%d) min %d",
                        nombre,i,*y,*y,*ancho,*alto,*width_before_max_min_imize,*height_before_max_min_imize,*is_minimized);
                        return 1;
                }
        }

        //Si no se encuentra, meter geometria por defecto
        *x=menu_origin_x();
        *y=0;
        *ancho=ZXVISION_MAX_ANCHO_VENTANA;
        *alto=ZXVISION_MAX_ALTO_VENTANA;
        *width_before_max_min_imize=*ancho;
        *height_before_max_min_imize=*alto;
        *is_minimized=0;
        *is_maximized=0;
        debug_printf (VERBOSE_DEBUG,"Returning default window geometry for %s",nombre);
        return 0;
}





//Retorna 0 si error. Lo agrega si no existe. Si existe, lo modifica
/*
Hay que tener en cuenta que puede agregar cualquier nombre, exista o no dicha ventana
Esto permite que si en el futuro se borra alguna ventana por código, pero el usuario la estaba guardando por configuración,
con --windowgeometry, no dará error si es de una ventana que ya no existe
*/
int util_add_window_geometry(char *nombre,int x,int y,int ancho,int alto,int is_minimized,int is_maximized,int width_before_max_min_imize,int height_before_max_min_imize)
{

        int destino=total_config_window_geometry;
        int sustituir=0;

        int i;

        //Buscar si se encuentra, sustituir
        for (i=0;i<total_config_window_geometry;i++) {
                if (!strcasecmp(nombre,saved_config_window_geometry_array[i].nombre)) {
                        destino=i;
                        sustituir=1;
                        break;
                }
        }

        if (!sustituir) {
                if (total_config_window_geometry==MAX_CONFIG_WINDOW_GEOMETRY) {
                        debug_printf (VERBOSE_ERR,"Maximum window geometry config reached (%d)",MAX_CONFIG_WINDOW_GEOMETRY);
                        return 0;
                }
        }

        debug_printf (VERBOSE_DEBUG,"Storing window geometry at %d index array, name %s, %d,%d %dX%d (%dX%d before minimize)",
                destino,nombre,x,y,ancho,alto,width_before_max_min_imize,height_before_max_min_imize);

        strcpy(saved_config_window_geometry_array[destino].nombre,nombre);
        saved_config_window_geometry_array[destino].x=x;
        saved_config_window_geometry_array[destino].y=y;
        saved_config_window_geometry_array[destino].ancho=ancho;
        saved_config_window_geometry_array[destino].alto=alto;
        saved_config_window_geometry_array[destino].is_minimized=is_minimized;
        saved_config_window_geometry_array[destino].is_maximized=is_maximized;
        saved_config_window_geometry_array[destino].width_before_max_min_imize=width_before_max_min_imize;
        saved_config_window_geometry_array[destino].height_before_max_min_imize=height_before_max_min_imize;


        if (!sustituir) total_config_window_geometry++;

        return 1;

}

//Misma funcion que la anterior pero indicando solo puntero a ventana
//El nombre lo obtiene de la estructura de zxvision_window
void util_add_window_geometry_compact(zxvision_window *ventana)
{

        char *nombre;

        nombre=ventana->geometry_name;
        if (nombre[0]==0) {
                //printf ("Trying to save window geometry but name is blank\n");
                return;
        }

        util_add_window_geometry(nombre,ventana->x,ventana->y,ventana->visible_width,ventana->visible_height,
                ventana->is_minimized,ventana->is_maximized, ventana->width_before_max_min_imize,ventana->height_before_max_min_imize);
}

void util_clear_all_windows_geometry(void)
{
        total_config_window_geometry=0;
}

//Agregar una letra en el string en la posicion indicada
void util_str_add_char(char *texto,int posicion,char letra)
{


        if (posicion<0) return; //error

        //Primero hacer hueco, desplazar todo a la derecha
        int longitud=strlen(texto);



        //Si se intenta meter mas alla de la posicion del 0 final
        if (posicion>longitud) {
                //printf ("intentando escribir mas alla del 0 final\n");
                posicion=longitud;
        }

        int i;


        //Meter un 0 despues del 0 ultimo
        texto[longitud+1]=0;

        for (i=longitud;i>posicion;i--) {
                texto[i]=texto[i-1];
        }

        texto[posicion]=letra;
}

//Quitar una letra en el string en la posicion indicada
void util_str_del_char(char *texto,int posicion)
{

         if (posicion<0) return; //error

        //desplazar todo a la izquierda
        int longitud=strlen(texto);

        if (longitud==0) return; //cadena vacia


        //Si se intenta borrar mas alla de la longitud
        if (posicion>=longitud) {
                //printf ("intentando borrar mas alla del final\n");
                posicion=longitud-1;
        }

        int i;


        for (i=posicion;i<longitud;i++) {
                texto[i]=texto[i+1];
        }

}


char util_printable_char(char c)
{
        if (c<32 || c>126) return '?';
        else return c;
}

//funcion para leer una linea desde origen, hasta codigo 10 NL
//se puede limitar max en destino
//retorna puntero a byte despues de salto linea
char *util_read_line(char *origen,char *destino,int size_orig,int max_size_dest,int *leidos)
{
	max_size_dest --;
	*leidos=0;
	for (;*origen && size_orig>0 && max_size_dest>0;origen++,size_orig--,(*leidos)++) {
		//ignorar cr
		if ( *origen=='\r' ) continue;
		if ( *origen=='\n' ) {
			origen++;
            (*leidos)++;
			break;
		}
		*destino=*origen;
		destino++;
		max_size_dest--;

	}
	*destino=0;
	return origen;
}

//Retorna el codigo http o <0 si otros errores
int util_download_file(char *hostname,char *url,char *archivo,int use_ssl,int estimated_maximum_size,char *ssl_sni_host_name)
{
  int http_code;
	char *mem;
	char *orig_mem;
	char *mem_after_headers;
	int total_leidos;
	int retorno;
    char redirect_url[NETWORK_MAX_URL];



    retorno=zsock_http(hostname,url,&http_code,&mem,&total_leidos,&mem_after_headers,1,"",use_ssl,redirect_url,estimated_maximum_size,ssl_sni_host_name);


    if (http_code==302 && redirect_url[0]!=0) {
        debug_printf (VERBOSE_DEBUG,"util_download_file: detected redirect to %s",redirect_url);
        //TODO: solo gestiono 1 redirect

        //obtener protocolo
        use_ssl=util_url_is_https(redirect_url);

        //obtener host
        char nuevo_host[NETWORK_MAX_URL];
        util_get_host_url(redirect_url,nuevo_host);

        //obtener nueva url (sin host)
        char nueva_url[NETWORK_MAX_URL];
        util_get_url_no_host(redirect_url,nueva_url);

        //liberar memoria anterior
        if (mem!=NULL) free(mem);

        debug_printf (VERBOSE_DEBUG,"util_download_file: querying again host %s (SSL=%d) url %s",nuevo_host,use_ssl,nueva_url);

        //El redirect sucede con las url a archive.org

        retorno=zsock_http(nuevo_host,nueva_url,&http_code,&mem,&total_leidos,
                &mem_after_headers,1,"",use_ssl,redirect_url,estimated_maximum_size,ssl_sni_host_name);

	}


	orig_mem=mem;

	if (mem_after_headers!=NULL && http_code==200) {
		//temp limite
		//mem_after_headers[10000]=0;
		//menu_generic_message("Games",mem_after_headers);
		//char texto_final[30000];

		//int indice_destino=0;

		int dif_header=mem_after_headers-mem;
		total_leidos -=dif_header;
		mem=mem_after_headers;
		//grabar a disco
		//todo usar funcion de utils comun, existe?

		FILE *ptr_destino;
        ptr_destino=fopen(archivo,"wb");

        if (ptr_destino==NULL) {
                debug_printf (VERBOSE_ERR,"Error writing output file");
                return -1;
        }



        fwrite(mem_after_headers,1,total_leidos,ptr_destino);


        fclose(ptr_destino);
        free(orig_mem);
    }

    //Fin resultado http correcto
    else {
        free(orig_mem);

        if (retorno<0) {
                //printf ("Error: %d %s\n",retorno,z_sock_get_error(retorno));
                return retorno;
        }
        else {
                //No hacemos VERBOSE_ERR porque sera responsabilidad del que llame a la funcion util_download_file
                //de mostrar mensaje en ventana
                debug_printf(VERBOSE_DEBUG,"Error downloading file. Return code: %d",http_code);
        }
    }

    return http_code;
}

void util_normalize_name(char *texto)
{
	while (*texto) {
		char c=*texto;
		if (c=='(' || c==')') {
			*texto='_';
		}
		texto++;
	}
}

void util_normalize_query_http(char *orig,char *dest)
{

	while (*orig) {
		char c=*orig;
                if (c<=32 || c>=127 || c=='/') {
                        sprintf (dest,"%%%02x",c);
                        dest+=3;
                }

                else {
                        *dest=c;
			dest++;
		}


		orig++;
	}

	*dest=0;


}



int util_extract_scl(char *sclname, char *dest_dir)
{
        //Archivo orig
        char name[PATH_MAX];
        //char dir[PATH_MAX];
        util_get_file_no_directory(sclname,name);
        //util_get_dir(sclname,dir);

        char destname[PATH_MAX];
        sprintf(destname,"%s/%s.trd",dest_dir,name);
        debug_printf (VERBOSE_INFO,"Calling scl2trd_main %s %s",sclname,destname);
        scl2trd_main(sclname,destname);
        return 0;
}




int on_extract_entry(const char *filename, void *arg) {
    static int i = 0;
    int n = *(int *)arg;
    debug_printf (VERBOSE_INFO,"Internal zip decompressor: Extracted: %s (%d of %d)", filename, ++i, n);

    return 0;
}

int util_extract_zip(char *zipname, char *dest_dir)
{
        int arg = 2;
        zip_extract(zipname, dest_dir, on_extract_entry, &arg);

        return 0;
}

//Retorna puntero a memoria comprimida
z80_byte *util_compress_memory_zip(z80_byte *memoria_input,int longitud,int *longitud_comprimido,char *nombre_archivo_interno)
{
    char *outbuf = NULL;
    size_t outbufsize = 0;

    //const char *inbuf = "Append some data here...\0";
    struct zip_t *zip = zip_stream_open(NULL, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    {
        zip_entry_open(zip, nombre_archivo_interno);
        {
            zip_entry_write(zip, memoria_input, longitud);
        }
        zip_entry_close(zip);

        /* copy compressed stream into outbuf */
        zip_stream_copy(zip, (void **)&outbuf, &outbufsize);
    }
    zip_stream_close(zip);

    *longitud_comprimido=outbufsize;

    //free(outbuf);
    return (z80_byte *)outbuf;
}

z80_byte *util_uncompress_memory_zip(z80_byte *memoria_input,int longitud,int *longitud_descomprimido,char *nombre_archivo_interno)
{

    char *buf = NULL;
    size_t bufsize = 0;

    struct zip_t *zip = zip_stream_open((char *)memoria_input, longitud, 0, 'r');
    {
        zip_entry_open(zip, nombre_archivo_interno);
        {
            zip_entry_read(zip, (void **)&buf, &bufsize);
        }
        zip_entry_close(zip);
    }
    zip_stream_close(zip);

    //free(buf);

    *longitud_descomprimido=bufsize;

    return (z80_byte *)buf;

}

int util_url_is_https(char *url)
{

        char *encontrado;

        encontrado=strstr(url,"https://");
        if (encontrado==url) return 1;
        else return 0;
}

void util_get_host_url(char *url, char *host)
{
        //buscar primero si hay ://

        char *encontrado;
        //char *inicio_host;

        encontrado=strstr(url,"://");
        if (encontrado!=NULL) {
                encontrado +=3; //saltar 3 caracteres ://
        }
        else encontrado=url;

        //Copiar hasta la primera / o final de string
        int i=0;

        for (i=0;(*encontrado)!=0 && (*encontrado)!='/';i++,host++,encontrado++) {
                *host=*encontrado;
        }

        *host=0;

}

void util_get_url_no_host(char *url, char *url_no_host)
{
        //buscar primero si hay ://

        char *encontrado;
        //char *inicio_host;

        encontrado=strstr(url,"://");
        if (encontrado!=NULL) {
                encontrado +=3; //saltar 3 caracteres ://
        }
        else encontrado=url;

        //Buscar hasta la primera / o final de string
        for (;(*encontrado)!=0 && (*encontrado)!='/';encontrado++);

        //copiar desde ahi hasta final de string
        for (;(*encontrado)!=0;encontrado++,url_no_host++) {
                *url_no_host=*encontrado;
        }

        *url_no_host=0;

}


//Retorna ? si caracter esta fuera de rango. Si no , retorna caracter
char util_return_valid_ascii_char(char c)
{
        if (c>=32 && c<=126) return c;
        else return '?';
}


//Retorna checksum de bloque igual que hace spectrum. Podemos indicar valor inicial distinto de 0 para empezar a calcular ya
//con byte de flag calculado
z80_byte get_memory_checksum_spectrum(z80_byte crc,z80_byte *origen,int longitud)
{
        //z80_byte crc=0;

        while (longitud>0) {
                crc ^=*origen;
                origen++;
                longitud--;
        }

        return crc;
}

//Guardar valor de 16 bits en direccion de memoria
void util_store_value_little_endian(z80_byte *destination,z80_int value)
{
        destination[0]=value_16_to_8l(value);
        destination[1]=value_16_to_8h(value);
}


//Recuperar valor de 16 bits de direccion de memoria
z80_int util_get_value_little_endian(z80_byte *origin)
{
        return value_8_to_16(origin[1],origin[0]);
}



void util_get_home_dir(char *homedir)
{

        //asumimos vacio
        homedir[0]=0;

  #ifndef MINGW
        char *directorio_home;
        directorio_home=getenv("HOME");
        if ( directorio_home==NULL) {
                  //printf("Unable to find $HOME environment variable to open configuration file\n");
                return;
        }

        sprintf(homedir,"%s/",directorio_home);

  #else
        char *homedrive;
        char *homepath;
        homedrive=getenv("HOMEDRIVE");
        homepath=getenv("HOMEPATH");

        if (homedrive==NULL || homepath==NULL) {
                //printf("Unable to find HOMEDRIVE or HOMEPATH environment variables to open configuration file\n");
                  return;
          }

        sprintf(homedir,"%s\\%s\\",homedrive,homepath);

  #endif


}

 #define UTIL_BMP_HEADER_SIZE (14+40)

int util_bmp_get_file_size(int ancho,int alto)
{
        //File size
        //cabecera + ancho*alto*3 (*3 porque es 24 bit de color)
        int file_size=(ancho*alto*3)+UTIL_BMP_HEADER_SIZE;

        return file_size;
}

//Asignar memoria para un archivo bmp de 24 bits y meter algunos valores en cabecera
z80_byte *util_bmp_new(int ancho,int alto)
{

        //http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm


        int memoria_necesaria=(ancho*alto*3)+UTIL_BMP_HEADER_SIZE;

        z80_byte *puntero;

        puntero=malloc(memoria_necesaria);

        if (puntero==NULL) cpu_panic("Can not allocate memory for bmp file");

        puntero[0]='B';
        puntero[1]='M';

        //File size
        //cabecera + ancho*alto*3 (*3 porque es 24 bit de color)
        int file_size=util_bmp_get_file_size(ancho,alto);
        puntero[2]=file_size & 0xFF;
        file_size >>=8;

        puntero[3]=file_size & 0xFF;
        file_size >>=8;

        puntero[4]=file_size & 0xFF;
        file_size >>=8;

        puntero[5]=file_size & 0xFF;
        file_size >>=8;

        //Unused
        puntero[6]=puntero[7]=puntero[8]=puntero[9];

        //Data offset
        puntero[10]=value_16_to_8l(UTIL_BMP_HEADER_SIZE);
        puntero[11]=value_16_to_8h(UTIL_BMP_HEADER_SIZE);
        puntero[12]=puntero[13]=0;

        //Size of info header = 40
        puntero[14]=40;
        puntero[15]=puntero[16]=puntero[17]=0;

        //Ancho
        puntero[18]=value_16_to_8l(ancho);
        puntero[19]=value_16_to_8h(ancho);
        puntero[20]=puntero[21]=0;

        //Alto
        puntero[22]=value_16_to_8l(alto);
        puntero[23]=value_16_to_8h(alto);
        puntero[24]=puntero[25]=0;

        //Planes = 1
        puntero[26]=1;
        puntero[27]=0;

        //Bits per pixel -> 24
        puntero[28]=24;
        puntero[29]=0;

        //Compression -> 0 no compression
        puntero[30]=puntero[31]=puntero[32]=puntero[33]=0;

        //Image size. If compression -> 0
        puntero[34]=puntero[35]=puntero[36]=puntero[37]=0;

        //horizontal resolution: Pixels/meter. 2835. valor de referencia de otro bmp
        puntero[38]=value_16_to_8l(2835);
        puntero[39]=value_16_to_8h(2835);
        puntero[40]=puntero[41]=0;


        //vertical resolution: Pixels/meter
        puntero[42]=value_16_to_8l(2835);
        puntero[43]=value_16_to_8h(2835);
        puntero[44]=puntero[45]=0;

        //Number of actually used colors. For a 8-bit / pixel bitmap this will be 100h or 256.
        //-> 1 ^24
        puntero[46]=puntero[47]=puntero[48]=0;
        puntero[49]=1;

        //Number of important colors  0 = all
        puntero[50]=puntero[51]=puntero[52]=puntero[53]=0;

        return puntero;
}

void util_bmp_putpixel(z80_byte *puntero,int x,int y,int r,int g,int b)
{
        int ancho=value_8_to_16(puntero[19],puntero[18]);
        int alto=value_8_to_16(puntero[23],puntero[22]);

        //Se dibuja de abajo a arriba
        int yfinal=(alto-1)-y;

        int tamanyo_linea=ancho*3;
        int offset_x=x*3;
//TODO: determinados anchos de imagen parece que el offset de linea no siempre es este
        int offset=(yfinal*tamanyo_linea)+offset_x+UTIL_BMP_HEADER_SIZE;

        //Each 3-byte triplet in the bitmap array represents the relative intensities of blue, green, and red, respectively, for a pixel.

        puntero[offset++]=b;
        puntero[offset++]=g;
        puntero[offset++]=r;

}





void util_write_screen_bmp(char *archivo)
{

        enable_rainbow();

        int ancho,alto;


        ancho=screen_get_emulated_display_width_no_zoom_border_en();
        alto=screen_get_emulated_display_height_no_zoom_border_en();


        FILE *ptr_scrfile;
        ptr_scrfile=fopen(archivo,"wb");
        if (!ptr_scrfile) {
                debug_printf (VERBOSE_ERR,"Unable to open Screen file");
        }

        else {


                z80_byte *puntero_bitmap;

                puntero_bitmap=util_bmp_new(ancho,alto);


//pixeles...



                //Derivado de vofile_send_frame


                z80_int *original_buffer;

                original_buffer=rainbow_buffer;


                z80_int color_leido;

                int r,g,b;

                int x,y;


                for (y=0;y<alto;y++) {
                        for (x=0;x<ancho;x++) {
                                color_leido=*original_buffer;
                                convertir_color_spectrum_paleta_to_rgb(color_leido,&r,&g,&b);

                                util_bmp_putpixel(puntero_bitmap,x,y,r,g,b);

                                original_buffer++;
                        }
                }



                int file_size=util_bmp_get_file_size(ancho,alto);

                fwrite(puntero_bitmap,1,file_size,ptr_scrfile);




                fclose(ptr_scrfile);

                free(puntero_bitmap);
        }




}

//Indica que la paleta primaria o la secundaria se ha cambiado
//util para detectar en algunos sitios si paleta se ha cambiado y hay que cargarla de nuevo
int util_bmp_load_palette_changed_palette_primary=0;
int util_bmp_load_palette_changed_palette_second=0;

//Cargar 256 colores de paleta de un archivo bmp (de memoria) en un indice de nuestra paleta interna
void util_bmp_load_palette(z80_byte *mem,int indice_inicio_color)
{

		//Cargar la paleta bmp. A partir del offset 36h??
/*
ColorTable	4 * NumColors bytes	0036h	present only if Info.BitsPerPixel less than 8
colors should be ordered by importance
 		Red	1 byte	 	Red intensity
Green	1 byte	 	Green intensity
Blue	1 byte	 	Blue intensity
reserved	1 byte	 	unused (=0)

*/

    int i;
    int indice_paleta;

    //En posicion 0E, dword que indica tamaño de la segunda paleta. Luego vendra la paleta de colores
    int size_info_header;

    //TODO: asumimos tamaños pequeños de esa cabecera y por tanto solo leo 2 bytes de ese dword
    size_info_header=mem[0x0E]+256*mem[0x0F];

    indice_paleta=size_info_header;
    indice_paleta +=0x0E;

    //Orden BGR0
    for (i=0;i<256;i++) {
        int red=mem[indice_paleta+2];
        int green=mem[indice_paleta+1];
        int blue=mem[indice_paleta];

        int color=(red<<16) | (green<<8) | blue;
        debug_printf(VERBOSE_DEBUG,"Loading BMP palette. Index %d Value %06XH",i,color);


        screen_set_colour_normal(indice_inicio_color+i,color);

        indice_paleta +=4;
    }

    if (indice_inicio_color==BMP_INDEX_FIRST_COLOR) util_bmp_load_palette_changed_palette_primary=1;
    else util_bmp_load_palette_changed_palette_second=1;

    //printf("Palette has been changed\n");
    //debug_exec_show_backtrace();
    //printf("\n");
}

void util_load_bmp_file_palette(z80_byte *puntero,int id_paleta)
{
    //Cargar la paleta bmp.
    int paleta_destino=(id_paleta==0 ? BMP_INDEX_FIRST_COLOR : BMP_SECOND_INDEX_FIRST_COLOR);
    util_bmp_load_palette(puntero,paleta_destino);
}

//Cargar un archivo bmp en memoria. Retorna puntero
//id_paleta indica si se carga en paleta de colores 0 o 1
z80_byte *util_load_bmp_file_no_palette(char *archivo)
{
    z80_byte *puntero;

    //Asignar memoria
    int tamanyo=get_file_size(archivo);
    puntero=malloc(tamanyo);

    if (puntero==NULL) cpu_panic("Can not allocate memory for bmp file");

    //cargarlo en memoria
    FILE *ptr_bmpfile;
    ptr_bmpfile=fopen(archivo,"rb");

    if (!ptr_bmpfile) {
            debug_printf(VERBOSE_ERR,"Unable to open bmp file %s",archivo);
            return NULL;
    }

    fread(puntero,1,tamanyo,ptr_bmpfile);
    fclose(ptr_bmpfile);


    return puntero;
}

//Cargar un archivo bmp en memoria. Retorna puntero
//id_paleta indica si se carga en paleta de colores 0 o 1
z80_byte *util_load_bmp_file(char *archivo,int id_paleta)
{
    z80_byte *puntero=util_load_bmp_file_no_palette(archivo);

    if (puntero==NULL) return NULL;

    util_load_bmp_file_palette(puntero,id_paleta);

    return puntero;
}

void util_rotate_file(char *filename,int archivos)
{

	//Borrar el ultimo
	char buffer_last_file[PATH_MAX];

	sprintf(buffer_last_file,"%s.%d",filename,archivos);

	debug_printf (VERBOSE_DEBUG,"Erasing oldest file %s",buffer_last_file);

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
			strcpy(buffer_file_orig,filename);
		}
		else {
			sprintf(buffer_file_orig,"%s.%d",filename,i);
		}

		sprintf(buffer_file_dest,"%s.%d",filename,i+1);

		debug_printf (VERBOSE_DEBUG,"Renaming log file %s -> %s",buffer_file_orig,buffer_file_dest);
		rename(buffer_file_orig,buffer_file_dest);
	}


}



//Funcion comun que puede indicar si queremos convertir utf a caracteres en pantalla, o bien,
//convertir caracteres con acentos etc a caracteres sin acentos
int util_convert_utf_charset_common(char *origen,z80_byte *final,int longitud_texto,int forzar_no_mostrar_utf)
{

    //Esto es un poco chapuza porque alteramos temporalmente el comportamiento de la funcion que convierte utf
    //Esto daria problemas si se accediera a dicha funcion desde varios threads a la vez, cosa que no deberia pasar
    int antes_forzar_no_mostrar_caracteres_extendidos=forzar_no_mostrar_caracteres_extendidos;

    if (forzar_no_mostrar_utf) forzar_no_mostrar_caracteres_extendidos=1;


        int longitud_final=0;

        int era_utf=0;

        z80_byte letra;

        while (longitud_texto>0) {

                letra=*origen;
                origen++;

            if (era_utf) {
                letra=menu_escribe_texto_convert_utf(era_utf,letra);
                era_utf=0;

                //Caracter final utf
                *final=letra;

            }


            //Si no, ver si entra un prefijo utf
            else {
                //printf ("letra: %02XH\n",letra);
                //Prefijo utf
                            if (menu_es_prefijo_utf(letra)) {
                                    era_utf=letra;
                    //printf ("activado utf\n");
                            }

                else {
                    //Caracter normal
                *final=letra;

                }
            }




        //if (x>=32) {
        //  printf ("Escribiendo caracter [%c] en x: %d\n",letra,x);
        //}


        if (!era_utf) {
                                final++;

                longitud_final++;

        }


        longitud_texto--;

        }

        //Caracter 0 del final
        *final=0;


    if (forzar_no_mostrar_utf) forzar_no_mostrar_caracteres_extendidos=antes_forzar_no_mostrar_caracteres_extendidos;

        return longitud_final;
}


//Convertir una string de origen a caracteres charset, sustituyendo graficos en cirilicos
int util_convert_utf_charset(char *origen,z80_byte *final,int longitud_texto)
{
    return util_convert_utf_charset_common(origen,final,longitud_texto,0);
}

//Convertir una string de origen a caracteres sin acentos etc
int util_convert_utf_no_utf(char *origen,char *final,int longitud_texto)
{
    return util_convert_utf_charset_common(origen,(z80_byte *)final,longitud_texto,1);
}

const char *spectrum_colour_names[16]={
        "Black",
        "Blue",
        "Red",
        "Magenta",
        "Green",
        "Cyan",
        "Yellow",
        "White",
        "Black",
        "BrightBlue",
        "BrightRed",
        "BrightMagenta",
        "BrightGreen",
        "BrightCyan",
        "BrightYellow",
        "BrightWhite",
};


//Guardar en la posicion indicada el valor de 32 bits, little endian
void util_write_long_value(z80_byte *destino,unsigned int valor)
{
    destino[0]=valor         & 0xFF;
    destino[1]=(valor >> 8)  & 0xFF;
    destino[2]=(valor >> 16) & 0xFF;
    destino[3]=(valor >> 24) & 0xFF;
}

//Lee de la posicion indicada el valor de 32 bits, little endian
unsigned int util_read_long_value(z80_byte *origen)
{
    return (origen[0])|(origen[1]<<8)|(origen[2]<<16)|(origen[3]<<24);
}

int util_get_input_file_keyboard_ms(void)
{
    return input_file_keyboard_delay*1000/50;
}

//Dice si una ruta es de una mmc montada
//Realmente chan fat fs soporta otras rutas como validas, pero aqui establecemos siempre
// "X:/", donde X es un numero de unidad de 0 al 9, para poder distinguir de rutas locales de filesystem
//Ejemplo:   0:/SYS/ESXDOS.SYS
int util_path_is_prefix_mmc_fatfs(char *dir)
{
    //Al menos 3 de longitud
    if (strlen(dir)<3) return 0;

    if (util_is_digit(dir[0]) && dir[1]==':' && dir[2]=='/') {
        //printf("util_path_is_prefix_mmc_fatfs ruta %s es de mmc montado\n",dir);
        return 1;
    }

    return 0;
}

//Dice si un archivo esta en una ruta de mmc fat fs o no , teniendo en cuenta la unidad actual
int util_path_is_mmc_fatfs(char *dir)
{
    /*
Cambios de ruta al estilo unix:

Si 0:/xxxx, cambiamos a unidad mmc
Si /xxxx o \xxxxx o X:\XXXXX o X:/XXXXX, no es unidad mmc

Cualquier otra cosa, chdir sin alterar unidad mmc activa o no

TODO: indicar C: sin barra (C:/) provoca que crea que es ruta relativa, aunque no afecta a ningun sitio
porque desde el menu de Drives en file selector ya agrego la / final
*/

    int ruta_es_mmc=util_path_is_prefix_mmc_fatfs(dir);

    int ruta_es_relativa=1;
    if (dir[0]=='/' || dir[0]=='\\' || util_path_is_windows_with_drive(dir)) ruta_es_relativa=0;

    //Si ruta es de mmc o ruta relativa
    //printf("ruta: [%s]ruta_es_mmc %d ruta_es_relativa %d\n",dir,ruta_es_mmc,ruta_es_relativa);

    int usar_chdir_mmc=0;

    if (ruta_es_mmc && menu_mmc_image_montada) usar_chdir_mmc=1;

    else {
        if (ruta_es_relativa) {
            //ruta relativa. Conservar unidad actual
            if (menu_current_drive_mmc_image.v) usar_chdir_mmc=1;
            else usar_chdir_mmc=0;
        }
        else {
            //usar_chdir_mmc=0; //ya es por defecto
        }
    }

    return usar_chdir_mmc;
}

//Si ruta empieza por X:\ o X:/ donde X es una letra
int util_path_is_windows_with_drive(char *dir)
{
    if (strlen(dir)<3) return 0;

    if (util_is_letter(dir[0]) &&
        dir[1]==':' &&
        (dir[2]=='/' || dir[2]=='\\')
    )
    {
        return 1;
    }

    return 0;

}

//Funciones para poder copiar en una imagen mmc una cantidad de X archivos, al iniciar el emulador

//origenes
char copy_files_to_mmc_source[MAX_COPY_FILES_TO_MMC][PATH_MAX];
//destinos
char copy_files_to_mmc_destination[MAX_COPY_FILES_TO_MMC][1024];
//Cuantos se han indicado
int copy_files_to_mmc_total=0;

//Agregar un archivo a la lista
//Retorna 0 si ok
int util_copy_files_to_mmc_addlist(char *source, char *destination)
{
    if (copy_files_to_mmc_total==MAX_COPY_FILES_TO_MMC) {
        debug_printf(VERBOSE_ERR,"Maximum files reached: (%d)",MAX_COPY_FILES_TO_MMC);
        return 1;
    }

    //destino no debe incluir 0:/, lo agregaremos nosotros
    if (util_path_is_prefix_mmc_fatfs(destination)) {
        debug_printf(VERBOSE_ERR,"Destination must not include 0:/ prefix");
        return 1;
    }

    strcpy(copy_files_to_mmc_source[copy_files_to_mmc_total],source);

    strcpy(copy_files_to_mmc_destination[copy_files_to_mmc_total],destination);

    copy_files_to_mmc_total++;

    return 0;

}


//Esto va fuera porque todas las operaciones lo usan. Lo uso en el copy inicial
FATFS FatFs_util_copy_files_to_mmc;   /* Work area (filesystem object) for logical drive */

//Copiarlos
void util_copy_files_to_mmc_doit(void)
{
    if (!copy_files_to_mmc_total) return;

    if (mmc_file_name[0]==0) {
        debug_printf(VERBOSE_ERR,"No mmc file name set using --copy-file-to-mmc");
        return;
    }


    strcpy(fatfs_disk_zero_path,mmc_file_name);

    printf("Mounting mmc image %s\n",mmc_file_name);

    /* Gives a work area to the default drive */
    FRESULT resultado=f_mount(&FatFs_util_copy_files_to_mmc, "", 1);

    if (resultado!=FR_OK) {
        debug_printf(VERBOSE_ERR,"Error %d mounting image %s: %s",resultado,mmc_file_name,zvfs_get_strerror(resultado));
        return;
    }

    menu_mmc_image_montada=1;



    //copiar
    int i;

    for (i=0;i<copy_files_to_mmc_total;i++) {

        //meter prefijo 0:// en destino
        char destination[1024];
        sprintf(destination,"0://%s",copy_files_to_mmc_destination[i]);

        printf("Copying file %s to %s\n",copy_files_to_mmc_source[i],destination);


        util_copy_file(copy_files_to_mmc_source[i],destination);
    }

    printf("Syncing changes to the mmc image\n");

    diskio_sync();

    //desmontar
    //Decir que no montado y cambiar drive a local
    menu_mmc_image_montada=0;
    menu_current_drive_mmc_image.v=0;

    resultado=f_mount(0, "", 0);

    if (resultado!=FR_OK) {
        debug_printf(VERBOSE_ERR,"Error desmontando imagen : %d\n",resultado);
        return;
    }

}
//Solicitar lectura de memoria controlando si offset fuera de rango
//para evitar segfaults con DSK protegidos con speedlock por ejemplo
//(Mercenary - Escape From Targ & The Second City (1988)(Novagen Software))
//o DSK de MSX: (Pocky. Pokey (1989)(Pony Tail Soft)(ja)(Disk 1 of 3).dsk)
z80_byte util_get_byte_protect(z80_byte *memoria,int total_size,int offset)
{
    if (offset<0 || offset>=total_size) return 0;
    else return memoria[offset];
}


//Funcion memcpy pero comprobando que origen no se salga del maximo permitido
void util_memcpy_protect_origin(z80_byte *destino,z80_byte *memoria,int total_size,int offset,int total_copiar)
{

    for (;total_copiar>0;total_copiar--) {
        *destino=util_get_byte_protect(memoria,total_size,offset);

        destino++;
        offset++;
    }

}

int util_abs(int v)
{
    if (v<0) return -v;
    else return v;
}

int util_sign(int v)
{
    if (v<0) return -1;
    else return +1;
}


int util_cosine_table[91]={

10000,  //0
 9998,
 9994,
 9986,
 9976,
 9962,
 9945,
 9925,
 9903,
 9877,
 9848,
 9816,
 9781,
 9744,
 9703,

 9659, //15
 9613,
 9563,
 9511,
 9455,
 9397,
 9336,
 9272,
 9205,
 9135,
 9063,
 8988,
 8910,
 8829,
 8746,

 8660, //30,
 8572,
 8480,
 8387,
 8290,
 8192,
 8090,
 7986,
 7880,
 7771,
 7660,
 7547,
 7431,
 7314,
 7193,

 7071, //45
 6947,
 6820,
 6691,
 6561,
 6428,
 6293,
 6157,
 6018,
 5878,
 5736,
 5592,
 5446,
 5299,
 5150,

 5000, //60
 4848,
 4695,
 4540,
 4384,
 4226,
 4067,
 3907,
 3746,
 3584,
 3420,
 3256,
 3090,
 2924,
 2756,

 2588, //75
 2419,
 2250,
 2079,
 1908,
 1736, //80
 1564,
 1392,
 1219,
 1045,
  872,
  698,
  523,
  349,
  175,
    0  //90
};


//Retorna el coseno de un grado, multiplicado por 10000
int util_get_cosine(int degrees)
{

    //Ajustar a 360
    degrees = degrees % 360;

    //Hacerlo positivo
    if (degrees<0) degrees=360+degrees;


    if (degrees>=91 && degrees<=180) {
        return -util_cosine_table[180-degrees];
    }
    else if (degrees>=181 && degrees<=270) {
        return -util_cosine_table[degrees-180];
    }
    else if (degrees>=271 && degrees<=359) {
        return util_cosine_table[360-degrees];
    }
    else return util_cosine_table[degrees];
}

//Retorna el seno de un grado, multiplicado por 10000
int util_get_sine(int degrees)
{
    return util_get_cosine(90-degrees);
}

//Retorna los grados para un coseno dado
int util_get_acosine(int cosine)
{
    int i;
    int acosine;

    for (i=1;i<91;i++) {
        if (util_abs(cosine)==util_cosine_table[i]) {
            acosine=i;
            break;
        }

        if (util_abs(cosine)>util_cosine_table[i]) {
            acosine=i-1;
            break;
        }
    }

    //Si negativo
    if (cosine<0) {
        acosine=180-acosine;
    }

    return acosine;
}

//Antiguo algoritmo raiz cuadrada que recorre todos los elementos
int old_util_sqrt(int number)
{
    int resultado=0;
    while (resultado<=number) {
        if (resultado*resultado==number) return resultado; //exacto

        if (resultado*resultado>number) return resultado-1; //nos pasamos. restar 1 para no esceder

        resultado++;
    }

    return resultado;
}

//calcula la raiz cuadrada con valores enteros
//Algoritmo mejorado que va dividiendo conjunto total/2 cada vez, siendo mucho mas rapido
//Por ejemplo, 13 iteraciones para calcular raiz de 10000, mientras con el antiguo, serian 100 iteraciones
//(con el antiguo siempre son X iteraciones siendo X el resultado de la raiz cuadrada)
//Tipo resultado: 0 exacto, 1 aproximado, -1 valor negativo
//Si result_type==NULL, no guardar tipo resultado
//
//Nota para mi yo del futuro (o cualquiera que lea esto):
//Podria haber usado la funcion sqrt que utiliza libreria math del sistema (y probablemente directo a cpu),
//pero quiero hacerlo asi, asi aprendo un algoritmo simple
//Ademas, no necesito decimales (y si necesito algo de precision, podria usar un valor de entrada multiplicado por 10000 por ejemplo)
z80_64bit util_sqrt(z80_64bit number,int *result_type)
{
    //printf("-------\n");
    //z80_64bit resultado;
    z80_64bit final=number;
    z80_64bit inicio=0;

    int result_type_when_null;

    //Cuando parametro es NULL, lo cambiamos para que apunte a una variable local que no uso
    if (result_type==NULL) {
        result_type=&result_type_when_null;
    }

    while (inicio<=final) {
        //printf("Inicio %d final %d\n",inicio,final);

        //Si hay dos

        z80_64bit medio=(inicio+final)/2;

        z80_64bit cuadrado=medio*medio;

        if (cuadrado==number) {
            //printf("Exact result #1\n");
            *result_type=0;
            return medio; //exacto
        }

        z80_64bit delta=final-inicio;
        //si hay dos o 1 numeros en nuestro conjunto, hay que optar por el primero o segundo, sin que exceda
        if (delta<=1) {

            if (final*final==number) {
                //printf("Exact result #2\n");  //creo que esta solucion nunca se da
                *result_type=0;
                return final;
            }
            else {
                if (inicio*inicio==number) {
                    //printf("Exact result #3\n"); //creo que esta solucion nunca se da
                    *result_type=0;
                }
                else {
                    //printf("Aproximate result\n");
                    *result_type=1;
                }

                return inicio;
            }
        }

        if (cuadrado>number) {
            //Nos quedamos con la parte izquierda
            //printf("izq\n");
            final=medio;
        }

        else {
            //cuadrado<number
            //Nos quedamos con la parte derecha
            //printf("der\n");
            inicio=medio;
        }


    }

    //realmente por aqui solo deberiamos salir si inicio>final, o sea , si final es negativo
    //printf("Fallback return\n");
    *result_type=-1;
    return inicio;
}

//Dice si la direccion de memoria dir contiene los bytes de la lista.
int util_compare_bytes_address(menu_z80_moto_int dir,int *lista,int total_items,int case_insensitive)
{
    int i;

    for (i=0;i<total_items;i++) {
        z80_byte byte_leido=peek_byte_z80_moto(dir+i);
        int byte_en_lista=lista[i];

        if (case_insensitive) {
            byte_leido=int_minuscula(byte_leido);
            byte_en_lista=int_minuscula(byte_en_lista);
        }
        if (byte_leido!=byte_en_lista) {
            //printf("Not equal address %d\n",dir);
            return 0;
        }
    }

    return 1;
}

//Convierte una cinta real (wav, rwa, smp) a texto indicando los bloques que tiene de cinta,
//convirtiendo sonido a bits de Spectrum
//O también genera un tap de salida

//Si texto_browser!=NULL, genera el texto de browser
//Si tap_output!=NULL, genera el tap de salida
//array_block_positions y max_array_block_positions se utilizan para guardar las posiciones de cada bloque, segun la posicion en la cinta
//primer bloque: posicion 0
//segundo bloque: posicion XXX
//etc...
//Si no se quiere usar, indicar array_block_positions a NULL. Indice finaliza con -1
void util_realtape_browser(char *filename, char *texto_browser,int maxima_longitud_texto,char *tap_output,
                        long *array_block_positions,int max_array_block_positions,int *codigo_retorno)
{

    /*
    Este codigo es un poco chapuza porque para llamar a la rutina que convierte audio en datos de cinta,
    hay variables que se modifican de manera global, y hay que preservarlas, pues dichas variables
    contienen datos de una posible cinta convertida de wav/rwa/smp a tap en memoria, desde Standard tape
    */

   //printf("Inicio util_realtape_browser\n");


    //Preservar valores anteriores
    int antes_spec_smp_read_index_tap=spec_smp_read_index_tap;
    int antes_spec_smp_write_index_tap=spec_smp_write_index_tap;
    int antes_spec_smp_total_read=spec_smp_total_read;

    z80_byte *antes_spec_smp_memory=spec_smp_memory;
    FILE *antes_ptr_mycinta_smp=ptr_mycinta_smp;
    int antes_lee_smp_ya_convertido=lee_smp_ya_convertido;




    //Para que lo asigne la rutina main_spec_rwaatap
    spec_smp_memory=NULL;




    if (texto_browser!=NULL) {
        //iniciar con cadena vacia pues lo que hace es concatenar strings
        texto_browser[0]=0;

        main_spec_rwaatap_pointer_print=texto_browser;
    }


    main_spec_rwaatap_pointer_print_max=maxima_longitud_texto;

    //Hay que convertir el nombre del archivo (si no es rwa)
    char nombre_origen[NAME_MAX];
    util_get_file_no_directory(filename,nombre_origen);



    char file_to_open[PATH_MAX];
    file_to_open[0]=0; //de momento

    //si es rwa, archivo tal cual
    if (!util_compare_file_extension(filename,"rwa")) {
        strcpy(file_to_open,filename);
    }

    //convertir
    if (!util_compare_file_extension(filename,"smp")) {
        if (convert_smp_to_rwa_tmpdir(filename,file_to_open)) {
			debug_printf(VERBOSE_ERR,"Error converting input file");
			return;
		}
    }

    //convertir
    if (!util_compare_file_extension(filename,"wav")) {
        if (convert_wav_to_rwa_tmpdir(filename,file_to_open)) {
			debug_printf(VERBOSE_ERR,"Error converting input file");
			return;
		}
    }



    if (file_to_open[0]==0) {
        debug_printf(VERBOSE_ERR,"Do not know how to browse this file");
        return;
    }

    int longitud_archivo_smp=get_file_size(file_to_open);

    ptr_mycinta_smp=fopen(file_to_open,"rb");



    if (ptr_mycinta_smp==NULL) {
        debug_printf(VERBOSE_ERR,"Error opening file (on util_realtape_browser): %s",file_to_open);
    }

    else {


        //avisar que no se ha abierto aun el archivo. Esto se hace porque en la rutina de Autodetectar,
        //cada vez se abre el archivo de nuevo, y evitar que se tenga que convertir (por ejemplo de wav) una y otra vez
        lee_smp_ya_convertido=0;


        main_spec_rwaatap(array_block_positions,max_array_block_positions,codigo_retorno,longitud_archivo_smp);



        //generar el tap de salida si conviene
        if (tap_output!=NULL) {
            util_save_file(spec_smp_memory,spec_smp_total_read,tap_output);
        }



        //Este lo ha asignado la rutina main_spec_rwaatap
        free(spec_smp_memory);
    }



    main_spec_rwaatap_pointer_print=NULL;

    //restaurar

    spec_smp_read_index_tap=antes_spec_smp_read_index_tap;


    //Preservar valores anteriores

    //puntero de escritura de archivo generado en memoria
    spec_smp_write_index_tap=antes_spec_smp_write_index_tap;

    //total de bytes de archivo generado en memoria
    spec_smp_total_read=antes_spec_smp_total_read;

    //donde se guarda el archivo generado en memoria
    spec_smp_memory=antes_spec_smp_memory;


    ptr_mycinta_smp=antes_ptr_mycinta_smp;

    lee_smp_ya_convertido=antes_lee_smp_ya_convertido;



}



//Convierte una cinta real (wav, rwa, smp) a zx80/81 P/O
//tambien tape browser
void convert_realtape_to_po(char *filename, char *archivo_destino, char *texto_info_output,int si_load)
{

    /*
    Este codigo es un poco chapuza porque para llamar a la rutina que convierte audio en datos de cinta,
    hay variables que se modifican de manera global, y hay que preservarlas, pues dichas variables
    contienen datos de una posible cinta convertida de wav/rwa/smp a tap en memoria, desde Standard tape
    */


    //Preservar valores anteriores
    int antes_lee_smp_ya_convertido=lee_smp_ya_convertido;
    FILE *antes_ptr_mycinta_smp=ptr_mycinta_smp;
    char *antes_tapefile=tapefile;


    //Hay que convertir el nombre del archivo (si no es rwa)
    char nombre_origen[NAME_MAX];
    util_get_file_no_directory(filename,nombre_origen);



    char file_to_open[PATH_MAX];
    file_to_open[0]=0; //de momento

    //si es rwa, archivo tal cual
    if (!util_compare_file_extension(filename,"rwa")) {
        strcpy(file_to_open,filename);
    }

    //convertir
    if (!util_compare_file_extension(filename,"smp")) {
        if (convert_smp_to_rwa_tmpdir(filename,file_to_open)) {
			debug_printf(VERBOSE_ERR,"Error converting input file");
			return;
		}
    }

    //convertir
    if (!util_compare_file_extension(filename,"wav")) {
        if (convert_wav_to_rwa_tmpdir(filename,file_to_open)) {
			debug_printf(VERBOSE_ERR,"Error converting input file");
			return;
		}
    }



    if (file_to_open[0]==0) {
        debug_printf(VERBOSE_ERR,"Do not know how to convert this file to P/O");
        return;
    }


    tapefile=file_to_open;

    main_leezx81(archivo_destino,texto_info_output,si_load);



    tapefile=antes_tapefile;

    ptr_mycinta_smp=antes_ptr_mycinta_smp;

    lee_smp_ya_convertido=antes_lee_smp_ya_convertido;

}

//Escribe en string de destino texto tipo MM:SS segun segundos de entrada
void util_print_minutes_seconds(int segundos_totales, char *texto)
{
    int minutos=segundos_totales/60;

    int segundos=segundos_totales % 60;

    //Si son menos de 60 minutos, formato MM:SS. Si no, MMM:SS

    if (minutos<60) {
        sprintf(texto,"%02d:%02d",minutos,segundos);
    }
    else {
        sprintf(texto,"%d:%02d",minutos,segundos);
    }
}

void toggle_flash_state(void)
{
    if (disable_change_flash.v) {
        estado_parpadeo.v=0;
    }
    else {
        estado_parpadeo.v ^=1;
    }
}

//Asigna memoria y si falla, genera un mensaje de panic
void *util_malloc(int total,char *mensaje_panic)
{
    char *memoria;
    memoria=malloc(total);
    if (memoria==NULL) {
        cpu_panic(mensaje_panic);
    }

    return memoria;
}

//Asigna memoria y si falla, genera un mensaje de panic
//Llena la memoria con el byte indicado
void *util_malloc_fill(int total,char *mensaje_panic,z80_byte value)
{
    char *memoria;
    memoria=util_malloc(total,mensaje_panic);

    int i;
    for (i=0;i<total;i++) memoria[i]=value;

    return memoria;
}

//Esta funcion y la siguiente se usan para poder asignar memoria char en mensajes para ventana
//antes se hacia con char texto[MAX_TEXTO_GENERIC_MESSAGE] pero dado que ha aumentado de tamaño bastante MAX_TEXTO_GENERIC_MESSAGE,
//ya no se puede asignar dentro de una funcion (exceded el stack) y hay que hacerlo por malloc
void *util_malloc_max_texto_generic_message(char *mensaje_panic)
{
    return util_malloc(MAX_TEXTO_GENERIC_MESSAGE,mensaje_panic);
}


void *util_malloc_max_texto_browser(void)
{
    return util_malloc(MAX_TEXTO_BROWSER,"Can not allocate memory for browser");
}

//Cortar un texto por la derecha con ...
//max_length es la longitud efectiva en caracteres que queremos. Por ejemplo para un maximo de 3 caracteres, indicar un 3
//char_buffer_size es el tamaño del buffer de char, por ejemplo si es char buffer[10], le pasamos un 10
void util_trunc_name_right(char *texto,int max_length,int char_buffer_size)
{

    int longitud_texto=strlen(texto);

    if (longitud_texto>max_length && max_length>=3 && char_buffer_size>=4) {
        texto[max_length]=0;
        texto[max_length-1]='.';
        texto[max_length-2]='.';
        texto[max_length-3]='.';
    }

    //Ejemplo:
    //1234567890 <-longitud
    //HOLA QUE TAL  <- texto

    //01234567890   <- indice array
    //Cortar a 10
    //          0
    //         .
    //        .
    //       .
}

//Inicializar una string con un caracter y finalizarlo con 0
//longitud es la cantidad de caracter a repetir, si se trata de llenar toda la string, deberia ser el tamaño del buffer-1
void util_fill_string_character(char *buffer_linea,int longitud,z80_byte caracter)
{
    int j;
    for (j=0;j<longitud;j++) buffer_linea[j]=caracter;
    buffer_linea[j]=0;
}

//Hacer drag & drop de un archivo
//Esto solo tiene sentido lanzarlo desde un thread aparte, como el del driver cocoa
//pues necesita que el menu se entere (si es que esta el menu abierto) de la simulacion de pulsar tecla ESC
//TODO: esto es todo un poco chapuza, en vez de simular pulsaciones de teclas, tendria que haber alguna manera
//de notificar al menu y decirle que cerrase todo
void util_drag_drop_file(char *filepath)
{
    debug_printf(VERBOSE_INFO,"Smartloading by drag & drop: %s",filepath);

    strcpy(quickload_file,filepath);


    //Simulamos pulsar ESC. solo si menu estaba abierto
    if (menu_abierto) {
        debug_printf(VERBOSE_INFO,"Simulating pressing ESC and closing all menus before smartloading");
        //salir_todos_menus=1;


        //Si estamos en un menu, enviar ESC. Si tenemos background permitido y estamos en una ventana que permite background, enviar F6

        //Asumimos ESC
        int enviar_esc=1;

        if (menu_allow_background_windows) {
            if (zxvision_current_window!=NULL) {
                if (zxvision_current_window->can_be_backgrounded) {
                    enviar_esc=0;
                }
            }
        }

        //printf("Enviar ESC: %d\n",enviar_esc);

        if (enviar_esc) {
            //Simular ESC
            salir_todos_menus=1;
            puerto_especial1 &=(255-1);
        }

        else {
            //Simular F6
            menu_pressed_close_all_menus.v=1;
            //z80_byte puerto_especial3=255; //  F10 F9 F8 F7 F6
            puerto_especial3 &=(255-1);
        }


        //Pausa de 0.1 segundo
        usleep(100000);


        if (enviar_esc) {
            //Liberar ESC
            puerto_especial1 |=1;
        }

        else {
            //Liberar F6
            puerto_especial3 |=1;
        }


        //Y cerrar de nuevo por si hay algun tooltip abierto
        if (enviar_esc) {
            //Pausa de 0.1 segundo
            usleep(100000);

            //Simular ESC
            salir_todos_menus=1;
            puerto_especial1 &=(255-1);

            //Pausa de 0.1 segundo
            usleep(100000);

            //Liberar ESC
            puerto_especial1 |=1;
        }

        menu_event_pending_drag_drop_menu_open.v=1;
    }


    //printf("Decir de abrir menu para drag & drop\n");

    menu_set_menu_abierto(1);
    menu_event_drag_drop.v=1;
}

//Dice si un filesystem es de tipo plusidedos (los usados en roms de +3e)
//total_size debe contener tamanyo total de buffer de memoria
int util_if_filesystem_plusidedos(z80_byte *memoria,int total_size)
{
    char filesystem[11];

    util_memcpy_protect_origin((z80_byte *) filesystem,memoria,total_size,0,10);

	filesystem[10]=0;
	if (!strcmp(filesystem,"PLUSIDEDOS")) {
        //printf("es plusidedos\n");
        return 1;
    }
    else return 0;
}


//Dice si un filesystem es de tipo fat16
//total_size debe contener tamanyo total de buffer de memoria
int util_if_filesystem_fat16(z80_byte *memoria,int total_size)
{
    char filesystem[6];

    util_memcpy_protect_origin((z80_byte *) filesystem,memoria,total_size,0x100036,5);

	filesystem[5]=0;
	if (!strcmp(filesystem,"FAT16")){
        //printf("es fat16\n");
        return 1;
    }
    else return 0;


	//memcpy(filesystem,&mmc_file_memory[0x100036],5);

	//filesystem[5]=0;
	//if (!strcmp(filesystem,"FAT16")) {
}

//Retorna texto %Y-%m-%d-%H-%M-%S, usado en quicksave y en dump zsf on panic
//texto tiene que tener tamanyo 40, aunque cabe con menos, pero mejor asi  .   char time_string[40];
void snapshot_get_date_time_string_common(char *texto,int todos_guiones)
{
struct timeval tv;
  struct tm* ptm;
  //long microseconds;


                    // 2015/01/01 11:11:11.999999 "
                    // 123456789012345678901234567
  //const int longitud_timestamp=27;

  /* Obtain the time of day, and convert it to a tm struct. */
  gettimeofday (&tv, NULL);
  ptm = localtime (&tv.tv_sec);
  /* Format the date and time, down to a single second. */
  char time_string[40];

  //buffer temporal para poderle indicar el sizeof
  if (todos_guiones) strftime (time_string, sizeof(time_string), "%Y-%m-%d-%H-%M-%S", ptm);
  else strftime (time_string, sizeof(time_string), "%Y/%m/%d %H:%M:%S", ptm);

  //copiar a texto final
  strcpy(texto,time_string);

  //printf ("texto fecha: %s\n",texto);
}

void snapshot_get_date_time_string(char *texto)
{
	//Para formatos de quicksave y dump zsf en panic. Todo guiones
	snapshot_get_date_time_string_common(texto,1);
}

void snapshot_get_date_time_string_human(char *texto)
{
	//Para formatos de texto incluidos por ejemplo en cabecera pzx
	snapshot_get_date_time_string_common(texto,0);
}

//Retorna segundos desde 1970
z80_64bit util_get_seconds(void)
{

    struct timeval tv;
    gettimeofday (&tv, NULL);
    return tv.tv_sec;
}

//Retorna la paridad de un valor, o sea la cantidad de 0
/*
Parity:

If a byte has an even number of 1 digits then it has "even parity", and if the byte has an odd number of 1 digits then it has "odd parity".

The Parity Overflow is set to 1 when the byte has even parity, and set to 0 if the byte has odd parity.
*/
int util_parity(z80_byte value)
{
    //printf("util_parity: value %d\n",value);

    int i;

    int cantidad_unos=1; //Inicial indica paridad par

    for(i=0;i<8;i++) {
        if (value & 1) cantidad_unos++;

        value=value>>1;
    }

    int paridad=cantidad_unos & 1;

    //printf("util_parity: paridad %d\n",paridad);
    return paridad;
}


//Retorna numero de punteros de strings en el array, utilizado en scanf con historial
int util_scanf_history_get_total_lines(char **textos_historial)
{
    int i;

    //Por asignar un maximo
    int max_lineas=100;

    for (i=0;i<max_lineas && textos_historial[i];i++) {

    }

    if (i==max_lineas) {
        debug_printf(VERBOSE_ERR,"Maximum history lines reached: %d",max_lineas);
    }

    return i;
}

//Insertar un texto en la primera posicion dentro de un array de punteros de strings, utilizado en scanf con historial
void util_scanf_history_insert(char **textos_historial,char *texto)
{

    int lineas_historial=util_scanf_history_get_total_lines(textos_historial);

    int i;

    //liberar ultima posicion
    //int ultima_posicion=UTIL_SCANF_HISTORY_MAX_LINES-1; //contando que al final hay el NULL

    //debug
    for (i=0;i<=lineas_historial;i++) {
        debug_printf(VERBOSE_PARANOID,"util_scanf_history_insert: Before insert. Position %d Pointer %p (%s)",
            i,textos_historial[i],(textos_historial[i]!=NULL ? textos_historial[i] : "NULL"));
    }


    //Ejemplo UTIL_SCANF_HISTORY_MAX_LINES vale 3
    //Tenemos "string 1", "string 2", NULL
    //si lleno, lineas_historial==3-1=2
    //memoria a liberar = textos_historial[2-1] = tetos_historial [1]="string 2"


    if (lineas_historial==UTIL_SCANF_HISTORY_MAX_LINES-1) {        //Liberar posicion ultima
        char *mem_to_free=textos_historial[lineas_historial-1];
        debug_printf(VERBOSE_DEBUG,"util_scanf_history_insert: History is full. Freeing last position: %p",mem_to_free);
        free(mem_to_free);
    }
    else {
        lineas_historial++;
    }

    // hacer hueco desplazando todo hacia abajo. con el ejemplo de arriba, empezamos en 1
    for (i=lineas_historial-1;i>=1;i--) {
        char *puntero=textos_historial[i-1];
        textos_historial[i]=puntero;
    }

    //Meter NULL del final
    textos_historial[lineas_historial]=NULL;

    //Meter primera posicion, asignando memoria para string
    int total_bytes=strlen(texto)+1;
    char *newstring=util_malloc(total_bytes,"Can not allocate buffer for history string");
    strcpy(newstring,texto);
    textos_historial[0]=newstring;


    //debug
    for (i=0;i<=lineas_historial;i++) {
        debug_printf(VERBOSE_PARANOID,"util_scanf_history_insert: After insert. Position %d Pointer %p (%s)",
            i,textos_historial[i],(textos_historial[i]!=NULL ? textos_historial[i] : "NULL"));
    }
}

//Extraer preview de diferentes tipos de archivos que requieren expandir
int util_extract_preview_file_expandable(char *nombre,char *tmpdir)
{
			int retorno=1;

			if (!util_compare_file_extension(nombre,"tap") ) {
					debug_printf (VERBOSE_DEBUG,"Is a tap file");
					retorno=util_extract_tap(nombre,tmpdir,NULL,0);
			}

			else if (!util_compare_file_extension(nombre,"tzx") ) {
					debug_printf (VERBOSE_DEBUG,"Is a tzx file");
					retorno=util_extract_tzx(nombre,tmpdir,NULL);
			}

			else if (!util_compare_file_extension(nombre,"pzx") ) {
					debug_printf (VERBOSE_DEBUG,"Is a pzx file");
					retorno=util_extract_pzx(nombre,tmpdir,NULL);
			}

			else if (!util_compare_file_extension(nombre,"trd") ) {
					debug_printf (VERBOSE_DEBUG,"Is a trd file");
					retorno=util_extract_trd(nombre,tmpdir);
			}

			else if (!util_compare_file_extension(nombre,"ddh") ) {
					debug_printf (VERBOSE_DEBUG,"Is a ddh file");
					retorno=util_extract_ddh(nombre,tmpdir);
			}

			else if (!util_compare_file_extension(nombre,"mdr") ) {
					debug_printf (VERBOSE_DEBUG,"Is a mdr file");
					retorno=util_extract_mdr(nombre,tmpdir);
			}

			else if (!util_compare_file_extension(nombre,"rmd") ) {
					debug_printf (VERBOSE_DEBUG,"Is a rmd file");
					retorno=util_extract_rmd(nombre,tmpdir);
			}

			else if (!util_compare_file_extension(nombre,"dsk") ) {
					debug_printf (VERBOSE_DEBUG,"Is a dsk file");
                    //Ejemplos de DSK que muestran pantalla: CASTLE MASTER.DSK , Drazen Petrovic Basket.dsk
                    //printf("Before extract dsk\n");
					retorno=util_extract_dsk(nombre,tmpdir);
                    //printf("After extract dsk\n");
			}

            //printf("if_pending_error_message: %d\n",if_pending_error_message);
            //Quitar posibles errores al preparar esta preview
            //no queremos alertar al usuario por archivos incorrectos
            //De todas maneras siempre se vería el error en la consola
            if_pending_error_message=0;

            return retorno;
}


//Extraer preview de diferentes tipos de archivos que no requieren expandir
void util_extract_preview_file_simple(char *nombre,char *tmpdir,char *tmpfile_scr,int file_size)
{

	if (!util_compare_file_extension(nombre,"scr")
        || file_size==6912
        ) {
		debug_printf(VERBOSE_DEBUG,"File is a scr screen");

        //Decir que el scrfile es el mismo
        strcpy(tmpfile_scr,nombre);


	}

	//Si es sna
	else if (!util_compare_file_extension(nombre,"sna")) {
		debug_printf(VERBOSE_DEBUG,"File is a sna snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_sna_to_scr(nombre,tmpfile_scr);
		}



	}

	//Si es sp
	else if (!util_compare_file_extension(nombre,"sp")) {
		debug_printf(VERBOSE_DEBUG,"File is a sp snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_sp_to_scr(nombre,tmpfile_scr);
		}


	}

	//Si es z80
	else if (!util_compare_file_extension(nombre,"z80")) {
		debug_printf(VERBOSE_DEBUG,"File is a z80 snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_z80_to_scr(nombre,tmpfile_scr);
		}



	}

	//Si es P
	else if (!util_compare_file_extension(nombre,"p") || !util_compare_file_extension(nombre,"p81")) {
		debug_printf(VERBOSE_DEBUG,"File is a p/p81 snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_p_to_scr(nombre,tmpfile_scr);
		}



	}

	//Si es ZSF
	else if (!util_compare_file_extension(nombre,"zsf")) {
		debug_printf(VERBOSE_DEBUG,"File is a zsf snapshot");

		menu_filesel_mkdir(tmpdir);

		//Si no existe preview
		if (!si_existe_archivo(tmpfile_scr)) {
			util_convert_zsf_to_scr(nombre,tmpfile_scr);
		}



	}

}

//Retorna el tipo de archivo a extraer preview
//0: no tiene preview
//1: preview con extraccion (tap, tzx, etc)
//2: preview directa: scr, sna, etc
int util_get_extract_preview_type_file(char *nombre,long long int file_size)
{
    if (!util_compare_file_extension(nombre,"tap") ||
		!util_compare_file_extension(nombre,"tzx") ||
		!util_compare_file_extension(nombre,"pzx") ||
		!util_compare_file_extension(nombre,"trd") ||
		!util_compare_file_extension(nombre,"dsk") ||
        !util_compare_file_extension(nombre,"ddh") ||
        !util_compare_file_extension(nombre,"mdr") ||
        !util_compare_file_extension(nombre,"rmd")

	) {
        return 1;
    }


    if (
        !util_compare_file_extension(nombre,"scr")  ||
        !util_compare_file_extension(nombre,"sna") ||
        !util_compare_file_extension(nombre,"sp") ||
        !util_compare_file_extension(nombre,"z80") ||
        !util_compare_file_extension(nombre,"p") ||
        !util_compare_file_extension(nombre,"p81") ||
        !util_compare_file_extension(nombre,"zsf") ||
        file_size==6912
    ) {
        return 2;
    }

    return 0;
}


//quitar del nombre las / o \\ que puedan haber. esto sucede cuando
//se hace un preview de un archivo zip por ejemplo
//Y luego este nombre lo queremos para construir una ruta temporal

void util_normalize_file_name_for_temp_dir(char *nombre)
{
    while (*nombre) {
        if ((*nombre)=='/' || (*nombre)=='\\') {
            *nombre='_';
        }

        nombre++;
    }
}

//Obtener nombre sistema operativo en ejecucion
void util_get_operating_system_release(char *destino,int maximo)
{
    if (!si_existe_archivo("/etc/os-release")) {
        //Si no existe, cadena nula
        *destino=0;
        return;
    }

    char buffer_temporal[1024];
    int total_leidos=util_load_file_bytes((z80_byte *)buffer_temporal,"/etc/os-release",1023);

    buffer_temporal[1023]=0;


    //leer linea a linea
    char buffer_linea[200];

    char *mem=buffer_temporal;


    do {
        int leidos;
        char *next_mem;

        next_mem=util_read_line(mem,buffer_linea,total_leidos,200,&leidos);
        debug_printf(VERBOSE_DEBUG,"Reading os-release file, line: [%s]\n",buffer_linea);
        total_leidos -=leidos;


        char *existe;
        //PRETTY_NAME="Debian GNU/Linux 11 (bullseye)"
        existe=strstr(buffer_linea,"PRETTY_NAME");
        if (existe!=NULL) {
            existe=strstr(buffer_linea,"\"");
            if (existe!=NULL) {
                existe++;
                int i;
                for (i=0;i<maximo && (*existe) && (*existe!='\r') && (*existe!='\"');existe++,destino++,i++) {
                    *destino=*existe;
                }
                *destino=0;
                return;
            }
        }

        mem=next_mem;

    } while (total_leidos>0);


}

//Obtener versión del emulador en numero
//Generalmente es lo mismo que EMULATOR_VERSION, pero en alguna version (como ZEsarUX X) EMULATOR_VERSION era "X" pero el numero era "10.10"
//De manera general es EMULATOR_VERSION, pero para el caso de la version X (10.10) se retorna 10.10 en vez de X
//Esto solo se usa en ZRCP y ZENG, pues ZRCP el get-version tiene que retornar un numero.algo(y opcionalmente -SN o -RC)
void util_get_emulator_version_number(char *buffer)
{
    strcpy (buffer,EMULATOR_NUMBER_VERSION);
}

char *string_util_byte_to_hex_nibble_characters="0123456789ABCDEF";

//convierte los 4 bits bajos en caracter hexadecimal
//funcion rapida mejor que hacer sprintf o similar (en teoria mas rapida)
char util_byte_to_hex_nibble(z80_byte valor)
{
    return string_util_byte_to_hex_nibble_characters[valor&15];
}

//convierte una letra hexadecimal en su valor
//es la funcion inversa de util_byte_to_hex_nibble
//letras en mayusculas tienen que estar
z80_byte util_hex_nibble_to_byte(char letra)
{
    if (letra>='A') return 10+(letra-'A');
    else return (letra-'0');
}


//Funcion solo por diversion. Multiplica dos valores de 8 bits
//usando un algoritmo de multiplicacion de bits tal cual se haria "a mano"
//El primer operador se va desplazando a la izquierda (multiplicando por 2) y sumando al resultado
//si el bit observado del segundo operador es 1
z80_int util_multiply_8bits(z80_byte a,z80_byte b)
{
    z80_int resultado=0;

    //para irlo rotando a la izquierda necesito que sea de 16 bits
    z80_int operador_a=a;

    int i;

    for (i=0;i<8;i++) {
        if (b&1) resultado +=operador_a;

        b=b>>1;
        operador_a=operador_a<<1;

    }

    return resultado;

}

/*
Escribir un vertice en formato STL
*/
void util_stl_print_vertex(char *buffer_linea,int x,int y,int z,int exponente,int exponente_z)
{
    char signo='+';
    if (exponente<0) {
        signo='-';
        exponente=-exponente;
    }

    char signo_z='+';
    if (exponente_z<0) {
        signo_z='-';
        exponente_z=-exponente_z;
    }

    //            vertex  0.000000e+00  0.000000e+00  1.000000e+00
    sprintf(buffer_linea,"            vertex  %d.000000e%c%d  %d.000000e%c%d  %d.000000e%c%d",
        x,signo,exponente,
        y,signo,exponente,
        z,signo_z,exponente_z);
}

/*
Escribir el vector en formato STL
*/
void util_stl_print_facet(char *buffer_linea,int x,int y,int z)
{


    //            vertex  0.000000e+00  0.000000e+00  1.000000e+00
    sprintf(buffer_linea,"    facet normal  %d.000000e+00  %d.000000e+00  %d.000000e+00",
        x,
        y,
        z);
}


/*
Definir un triangulo en formato STL
Parametros:
vx,vy,vz: vectores del triangulo
x1,y1,z1: primera coordenada del triangulo
x2,y2,z2: segunda coordenada del triangulo
x3,y3,z3: tercera coordenada del triangulo

exponente: valor N para 10eN, o sea, cuantos ceros tienen los valores de dimensiones x,y
exponente_z: parecido a exponente pero para dimension z
*/
void util_stl_triangle(FILE *ptr_archivo,int vx,int vy,int vz,int x1,int y1,int z1,int x2,int y2,int z2,int x3,int y3,int z3,int exponente,int exponente_z)
{
    char buffer_linea[256];
    /*
    facet normal  0.000000e+00  0.000000e+00  1.000000e+00
        outer loop
            vertex  0.000000e+00  0.000000e+00  1.000000e+00
            vertex  1.000000e+00  0.000000e+00  1.000000e+00
            vertex  0.000000e+00  1.000000e+00  1.000000e+00
        endloop
    endfacet
    */
    util_stl_print_facet(buffer_linea,vx,vy,vz);
    fwrite(buffer_linea,1,strlen(buffer_linea),ptr_archivo);
    fwrite("\n",1,1,ptr_archivo);

    strcpy(buffer_linea,"        outer loop\n");
    fwrite(buffer_linea,1,strlen(buffer_linea),ptr_archivo);


    util_stl_print_vertex(buffer_linea,x1,y1,z1,exponente,exponente_z);
    fwrite(buffer_linea,1,strlen(buffer_linea),ptr_archivo);
    fwrite("\n",1,1,ptr_archivo);
    util_stl_print_vertex(buffer_linea,x2,y2,z2,exponente,exponente_z);
    fwrite(buffer_linea,1,strlen(buffer_linea),ptr_archivo);
    fwrite("\n",1,1,ptr_archivo);
    util_stl_print_vertex(buffer_linea,x3,y3,z3,exponente,exponente_z);
    fwrite(buffer_linea,1,strlen(buffer_linea),ptr_archivo);
    fwrite("\n",1,1,ptr_archivo);


    strcpy(buffer_linea,"        endloop\n");
    fwrite(buffer_linea,1,strlen(buffer_linea),ptr_archivo);

    strcpy(buffer_linea,"    endfacet\n");
    fwrite(buffer_linea,1,strlen(buffer_linea),ptr_archivo);

}

/*
Generar un cubo (aunque con posibles 3 dimensiones diferentes) consistentes en 12 triangulos, o sea,
2 triangulos por cada cara
Parametros:
x,y,z: coordenadas
exponente: valor N para 10eN, o sea, cuantos ceros tienen los valores de dimensiones x,y
exponente_z: parecido a exponente pero para dimension z
tamanyo_x, tamanyo_y, tamanyo_z: dimensiones x,y,z del cubo
El tamaño real del cubo es:

en X: tamanyo_x * 10 elevado exponente
en Y: tamanyo_y * 10 elevado exponente
en Z: tamanyo_z * 10 elevado exponente_z
*/
void util_stl_cube(FILE *ptr_archivo,int x,int y,int z,int exponente,int exponente_z,int tamanyo_x,int tamanyo_y,int tamanyo_z)
{
    util_stl_triangle(ptr_archivo,
        0,0,1,
        x+0,    y+0,z+tamanyo_z,
        x+tamanyo_x,y+0,z+tamanyo_z,
        x+0,    y+tamanyo_y,z+tamanyo_z,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        0,0,1,
        x+tamanyo_x,y+tamanyo_y,z+tamanyo_z,
        x+0,    y+tamanyo_y,z+tamanyo_z,
        x+tamanyo_x,y+0,z+tamanyo_z,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        1,0,0,
        x+tamanyo_x,y+0,z+tamanyo_z,
        x+tamanyo_x,y+0,z+0,
        x+tamanyo_x,y+tamanyo_y,z+tamanyo_z,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        1,0,0,
        x+tamanyo_x,y+tamanyo_y,z+0,
        x+tamanyo_x,y+tamanyo_y,z+tamanyo_z,
        x+tamanyo_x,y+0,z+0,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        0,0,-1,
        x+tamanyo_x,y+0,z+0,
        x+0,    y+0,z+0,
        x+tamanyo_x,y+tamanyo_y,z+0,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        0,0,-1,
        x+0,    y+tamanyo_y,z+0,
        x+tamanyo_x,y+tamanyo_y,z+0,
        x+0,    y+0,z+0,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        -1,0,0,
        x+0,    y+0,z+0,
        x+0,    y+0,z+tamanyo_z,
        x+0,    y+tamanyo_y,z+0,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        -1,0,0,
        x+0,    y+tamanyo_y,z+tamanyo_z,
        x+0,    y+tamanyo_y,z+0,
        x+0,    y+0,z+tamanyo_z,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        0,1,0,
        x+0,    y+tamanyo_y,z+tamanyo_z,
        x+tamanyo_x,y+tamanyo_y,z+tamanyo_z,
        x+0,    y+tamanyo_y,z+0,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        0,1,0,
        x+tamanyo_x,y+tamanyo_y,z+0,
        x+0,    y+tamanyo_y,z+0,
        x+tamanyo_x,y+tamanyo_y,z+tamanyo_z,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        0,-1,0,
        x+tamanyo_x,y+0,z+tamanyo_z,
        x+0,    y+0,z+tamanyo_z,
        x+tamanyo_x,y+0,z+0,
        exponente,exponente_z);

    util_stl_triangle(ptr_archivo,
        0,-1,0,
        x+0,    y+0,z+0,
        x+tamanyo_x,y+0,z+0,
        x+0,    y+0,z+tamanyo_z,
        exponente,exponente_z);
    /*

    Lo de arriba genera lo siguiente, sustutuyendo logicamente x,y,z, dimensiones, exponentes:
    (y solid y facet lo genero en otro sitio)

solid cube-ascii-1mm
    facet normal  0.000000e+00  0.000000e+00  1.000000e+00
        outer loop
            vertex  0.000000e+00  0.000000e+00  1.000000e+00
            vertex  1.000000e+00  0.000000e+00  1.000000e+00
            vertex  0.000000e+00  1.000000e+00  1.000000e+00
        endloop
    endfacet
    facet normal  0.000000e+00  0.000000e+00  1.000000e+00
        outer loop
            vertex  1.000000e+00  1.000000e+00  1.000000e+00
            vertex  0.000000e+00  1.000000e+00  1.000000e+00
            vertex  1.000000e+00  0.000000e+00  1.000000e+00
        endloop
    endfacet
    facet normal  1.000000e+00  0.000000e+00  0.000000e+00
        outer loop
            vertex  1.000000e+00  0.000000e+00  1.000000e+00
            vertex  1.000000e+00  0.000000e+00  0.000000e+00
            vertex  1.000000e+00  1.000000e+00  1.000000e+00
        endloop
    endfacet
    facet normal  1.000000e+00  0.000000e+00  0.000000e+00
        outer loop
            vertex  1.000000e+00  1.000000e+00  0.000000e+00
            vertex  1.000000e+00  1.000000e+00  1.000000e+00
            vertex  1.000000e+00  0.000000e+00  0.000000e+00
        endloop
    endfacet
    facet normal  0.000000e+00  0.000000e+00 -1.000000e+00
        outer loop
            vertex  1.000000e+00  0.000000e+00  0.000000e+00
            vertex  0.000000e+00  0.000000e+00  0.000000e+00
            vertex  1.000000e+00  1.000000e+00  0.000000e+00
        endloop
    endfacet
    facet normal  0.000000e+00  0.000000e+00 -1.000000e+00
        outer loop
            vertex  0.000000e+00  1.000000e+00  0.000000e+00
            vertex  1.000000e+00  1.000000e+00  0.000000e+00
            vertex  0.000000e+00  0.000000e+00  0.000000e+00
        endloop
    endfacet
    facet normal -1.000000e+00  0.000000e+00  0.000000e+00
        outer loop
            vertex  0.000000e+00  0.000000e+00  0.000000e+00
            vertex  0.000000e+00  0.000000e+00  1.000000e+00
            vertex  0.000000e+00  1.000000e+00  0.000000e+00
        endloop
    endfacet
    facet normal -1.000000e+00  0.000000e+00  0.000000e+00
        outer loop
            vertex  0.000000e+00  1.000000e+00  1.000000e+00
            vertex  0.000000e+00  1.000000e+00  0.000000e+00
            vertex  0.000000e+00  0.000000e+00  1.000000e+00
        endloop
    endfacet
    facet normal  0.000000e+00  1.000000e+00  0.000000e+00
        outer loop
            vertex  0.000000e+00  1.000000e+00  1.000000e+00
            vertex  1.000000e+00  1.000000e+00  1.000000e+00
            vertex  0.000000e+00  1.000000e+00  0.000000e+00
        endloop
    endfacet
    facet normal  0.000000e+00  1.000000e+00  0.000000e+00
        outer loop
            vertex  1.000000e+00  1.000000e+00  0.000000e+00
            vertex  0.000000e+00  1.000000e+00  0.000000e+00
            vertex  1.000000e+00  1.000000e+00  1.000000e+00
        endloop
    endfacet
    facet normal  0.000000e+00 -1.000000e+00  0.000000e+00
        outer loop
            vertex  1.000000e+00  0.000000e+00  1.000000e+00
            vertex  0.000000e+00  0.000000e+00  1.000000e+00
            vertex  1.000000e+00  0.000000e+00  0.000000e+00
        endloop
    endfacet
    facet normal  0.000000e+00 -1.000000e+00  0.000000e+00
        outer loop
            vertex  0.000000e+00  0.000000e+00  0.000000e+00
            vertex  1.000000e+00  0.000000e+00  0.000000e+00
            vertex  0.000000e+00  0.000000e+00  1.000000e+00
        endloop
    endfacet
endsolid

    */
}

/*
Para graficos de Tortuga tipo Logo
*/
void util_move_turtle(int xorig,int yorig,int grados,int longitud,int *xfinal,int *yfinal)
{
    *xfinal=xorig+((longitud*util_get_cosine(grados))/10000);
    *yfinal=yorig-((longitud*util_get_sine(grados))/10000);
}


int util_string_starts_with(char *texto, char *prefijo)
{
    while (*prefijo) {
        if ( (*prefijo) != (*texto) ) {
            return 0;
        }

        texto++;
        prefijo++;

    }

    return 1;
}


//Gestion de arbol binario para almacenar etiquetas
//Devuelve puntero diferente de NULL si se ha creado el primer elemento, apuntando a dicho elemento inicial
labeltree *labeltree_add_element(labeltree *l,char *name,int value)
{

    //printf("Adding [%s]\n",name);

    labeltree *previous=l;

    int resta=0;

    while (l!=NULL) {
        //printf("En bucle while. label leida=[%s]\n",l->name);
        previous=l;

        resta=strcasecmp(name,l->name);
        if (resta>0) {
            l=l->right;
            //printf("Vamos a la derecha\n");
        }
        else {
            l=l->left;
            //printf("Vamos a la izquierda\n");
        }
    }

    labeltree *newlabel=util_malloc(sizeof(labeltree),"Can not allocate memory for labeltree element");

    //Inicializar label
    newlabel->left=NULL;
    newlabel->right=NULL;

    if (strlen(name)>MAX_LABELTREE_NAME-1) {
        //acortar. excede limite
        strncpy(newlabel->name,name,MAX_LABELTREE_NAME-1);
        debug_printf(VERBOSE_ERR,"Label [%s] length exceeds limit (%d). Truncating it",name,MAX_LABELTREE_NAME-1);
    }

    else strcpy(newlabel->name,name);
    newlabel->value=value;

    //Si no habia ninguno
    if (previous==NULL) {
        //printf("Primera etiqueta\n");
        return newlabel;
    }
    else {
        if (resta>0) {
            previous->right=newlabel;
            //printf("Se escribe a la derecha\n");
        }
        else {
            previous->left=newlabel;
            //printf("Se escribe a la izquierda\n");
        }

        return NULL;
    }

}

//Buscar una labeltree
labeltree *labeltree_find_element(labeltree *l,char *name)
{
    //printf("Buscando [%s]\n",name);

    labeltree *previous=l;

    while (l!=NULL) {
        //printf("En bucle while. label leida=[%s]\n",l->name);
        previous=l;

        int resta=strcasecmp(name,l->name);
        if (!resta) {
            //printf("Encontrado exacto\n");
            return l; //Encontrado
        }

        if (resta>0) {
            l=l->right;
            //printf("Vamos a la derecha\n");
        }
        else {
            l=l->left;
            //printf("Vamos a la izquierda\n");
        }
    }

    //En caso que la busqueda no sea exacta, se retornara la mas cercana
    return previous;

}

void labeltree_free(labeltree *l)
{
    if (l==NULL) return;

    labeltree *left=l->left;
    labeltree *right=l->right;

    //printf("Liberar [%s] %p\n",l->name,l);
    free(l);

    labeltree_free(left);
    labeltree_free(right);
}


//Devolver valor random teniendo en cuenta algoritmo de ruido del chip AY pero tambien tiene en cuenta pulsaciones de teclado o raton
int util_get_random(void)
{
    ay_randomize(0);

    int valor_random=randomize_noise[0];

    //un poco mas aleatorio
    //como util_random_noise es valor en ms de tiempo pulsado tecla o raton, habitualmente
    //esto ira de 0 a 1000
    valor_random +=util_random_noise;

    //Para generar valores impares
    if (util_random_noise<500) valor_random++;

    return valor_random;

}